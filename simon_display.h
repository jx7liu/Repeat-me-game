
#include <cstdlib>
#include <iostream>
#include <list>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include "simon.h"
using namespace std;

int FPS = 60;

unsigned long now(){
	timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec*1000000 + t.tv_usec;


}

struct XInfo {
	Display* display;
	Window window;
	int screen;
	GC gc;
	int width;
	int height;
	bool resize; // if resize is true
	int buttonNum;

};


class Simon_Display;

class Displayable {

public:
	virtual void paint(XInfo& xinfo, Simon_Display *dis) = 0;


};

class Text : public Displayable {

public:
	virtual void paint(XInfo& xinfo, Simon_Display *dis) {
		XDrawImageString(xinfo.display, xinfo.window, xinfo.gc, this->x, this->y, this->s.c_str(), this->s.length());
	}	
	
	Text(int x, int y, string s): x(x), y(y), s(s) {}

	int x;
	int y;
	string s;

};


class Circle : public Displayable {

public:
	int x;
	double  y;
	int d_x;
	int d_y;
	int d;
	double ring_x; //information about the white ring
	double ring_y;
	double ring_d;
	string num;
	GC gc;
	GC gc1;
	int d_decrease;
	
	Circle(int x, int y, int d, string num): x(x), y(y), d(d), num(num) {
		setDark = false;
		buttonPressed = false;
		ring_x = x;
		ring_y = y;
		ring_d = d;
		d_x = x;
		d_y = y;
		d_decrease = 30;
		wait = false;
		done_paint = false;
	}
	int screen;
	bool setDark;
	bool buttonPressed;
	bool wait;
	bool done_paint;

	virtual void paint(XInfo& xinfo, Simon_Display* dis);
	

};


class Simon_Display {

	public:
	XInfo xinfo;
	vector <Displayable*> dVec;	
	vector <string> message;
	Simon * simon;
	bool sequence_play;
	bool ball_move;
	int c;
	bool if_wait;
	int turn_move_ball_on;
	bool if_wait_on_ball;
	//bool game_start;
	//vector <int> seq;
	//bool won;
	//int seq_num;
	Simon_Display(Simon *simon): simon(simon) {
		message.push_back("Press SPACE to play");
		message.push_back("Watch what I do ...");
		message.push_back("Now it's your turn");
		message.push_back("You won! Press SPACE to play again");
		message.push_back("You lose. Press SPACE to play again");
		sequence_play = false;
		ball_move = true;
		c = 0;
		if_wait_on_ball = false;
		if_wait = false;
		turn_move_ball_on = 30;
		//game_start = false;
		//won = true;
		//seq_num = 0;
	}
	
	void openWindow(int argc, char* argv[]){

		xinfo.display = XOpenDisplay("");
		if (!xinfo.display) {
			cerr << "cannot open window";
			exit(0);
		}

		xinfo.screen = DefaultScreen(xinfo.display);


		xinfo.window = XCreateSimpleWindow(xinfo.display, DefaultRootWindow( xinfo.display ), 
			10, 10, 800, 400, 5, XWhitePixel(xinfo.display, xinfo.screen), 
			XWhitePixel( xinfo.display, xinfo.screen ) );
		
		xinfo.width = 800;
		
		xinfo.height = 400;

		xinfo.resize = false;

		XSetStandardProperties(xinfo.display, xinfo.window, "Julia's Window", "JUJU", None, argv, argc, None);
		
		XSelectInput(xinfo.display, xinfo.window, ButtonPressMask | PointerMotionMask | KeyPressMask | StructureNotifyMask);		

		XMapRaised(xinfo.display, xinfo.window);

		XFlush(xinfo.display);
		
		usleep(10*1000);

	}

	void repaint () {
			
			XClearWindow(xinfo.display, xinfo.window);

			if (xinfo.resize){
				xinfo.resize = false;
				int space = (xinfo.width - (xinfo.buttonNum * 100)) / (xinfo.buttonNum + 1);
				
				for (int i = 2; i < dVec.size(); i++){
					((Circle*)dVec[i])->x = space*(i-1) + 100*(i-2);
					((Circle*)dVec[i])->y = xinfo.height / 2 - 50;		

				}

			}
			bool wait = false;
			for (int i = 0; i < dVec.size(); i++) {
				dVec[i]->paint(xinfo, this);
				//if(((Circle*)dVec[i])->done_paint){
				//	wait = true;
				//	((Circle*)dVec[i])->wait = false;
				//	((Circle*)dVec[i])->done_paint = false;
				//}	
			}
			XFlush(xinfo.display);
			//if(wait) {usleep(1000000/4); 
			//	if (sequence_play) {
			//		newroundHandler();
			//	}
			//}
			

	}

	

	void eventloop(){

		XEvent event;
		KeySym key;
		char command[10];
		unsigned long lastRepaint = 0;		

		while (true) {
			
			if (XPending(xinfo.display) > 0){
				XNextEvent(xinfo.display, &event);
			
			
				switch(event.type) {

				case ConfigureNotify:
					resizeHandler(event);
					break;

				case MotionNotify:
					motionHandler(event);	
					break;
				
				case ButtonPress:
					buttonHandler(event);
					break;

				case KeyPress:
					int i = XLookupString( (XKeyEvent*)&event, command, 10, &key, 0);
					
					if (i == 1 && command[0] == 'q') {
						cout << "Terminating normally" << endl;
						XCloseDisplay(xinfo.display);
						return;
					
					}
					else if (i == 1 && command[0] == ' '){
						//game_start = true;
						textchangeHandler(1);
						simon->newRound();
						sequence_play = true;
						ball_move = false;
						reset_ball();
						newroundHandler();
						
					}

			        	break;
                              

				}
			}

			unsigned long end = now();

			if (end - lastRepaint > 1000000/FPS){
				double b = simon->getNumButtons() / (3.1415926 *2);
				double s = 0.05;
				
				if (ball_move) {
					
					for (int i = 2; i < dVec.size(); i++) {
				
						//if ((i-2)%2){
							((Circle*)dVec[i])->y += sin((double)c*s + ((double)(i+1) * b));
						//}
						//else {
						
						//	((Circle*)dVec[i])->y -= sin((double)c*s + ((double)(i+1) *b));
						//}
					
					}
					c++;				
				}
				
				repaint();
				if (if_wait) {
					usleep(1000000/2);
					if_wait = false;
					if (sequence_play) {
						//usleep(1000000/2);
						//if_wait = false;
						newroundHandler();
					}
				}
				else if (if_wait_on_ball){
					if(turn_move_ball_on-- <= 0){
						//usleep(1000000);
						ball_move  = true;
						turn_move_ball_on = 0;
						if_wait_on_ball = false;
					}
				}
					
				else{}
				

				lastRepaint = now();
			}

			if (XPending(xinfo.display) == 0){
				usleep (1000000/FPS - (end - lastRepaint));
			}

		}


	}


	
	
	bool within_radius(int i, int x, int y){

		Circle * temp = (Circle*)dVec[i];

		int r_x = temp->x + 50;
		int r_y = temp->y + 50;

		int dis = sqrt(pow(r_x - x, 2) + pow(r_y - y, 2));
		if (dis <= 50){
			return true;

		}		
		else {
			return false;

		}
	}

	void textchangeHandler(int i){
		
		((Text*)dVec[1])->s = message[i];
	}

	void newroundHandler(){
		
		if (simon->getState() == Simon::COMPUTER){
			//cout << simon->getStateAsString() << endl;
			int i = simon->nextButton();
			((Circle*)dVec[i+2])->buttonPressed = true;
			((Circle*)dVec[i+2])->wait = true;
	
			//repaint();
		}
		else {sequence_play = false;
			textchangeHandler(2);
	
		}
		

	}

	void reset_ball(){

		for (int i = 2; i < dVec.size(); i++){
			Circle * k = (Circle*)dVec[i];
			k->y = k->d_y;

		}
		c = 0;

	}

	void buttonHandler(XEvent &event){
	
		int new_x = event.xbutton.x;
		int new_y = event.xbutton.y;
		for (int i = 2; i < dVec.size(); i++){
			if (within_radius(i, new_x, new_y)) {
				((Circle*)dVec[i])->buttonPressed = true;
				if (simon->getState() == Simon :: HUMAN){											if (!simon->verifyButton(i-2)) {
						//start_game = false;
						//seq_num = 0;
						textchangeHandler(4);
						((Text*)dVec[0])->s = "0";
					 	turn_move_ball_on = 45;
						if_wait_on_ball = true;
						//ball_move = true;
			
					}
					else {
						if (simon->getState() == Simon :: WIN) {
							textchangeHandler(3);
							stringstream ss;
							ss << simon->getScore();
							string k = ss.str();
							((Text*)dVec[0])->s = k;
							turn_move_ball_on = 45;
							if_wait_on_ball = true;
							//ball_move = true;
						}
					}
				}
			
			}
			else {
				((Circle*)dVec[i])->buttonPressed = false;

			}

		}
		//repaint();
		
	
	}

	void motionHandler(XEvent &event){
		int new_x = event.xmotion.x;
		int new_y = event.xmotion.y;

		for ( int i = 2; i < dVec.size(); i++) {
			if (within_radius(i, new_x, new_y)) {
				((Circle*)dVec[i])->setDark = true;
			}
			else { 
				((Circle*)dVec[i])->setDark = false;
			}	

		}
		//repaint();

	}

	void resizeHandler(XEvent &event){
		XConfigureEvent xce = event.xconfigure;
		if (xce.width != xinfo.width || xce.height != xinfo.height) {
			xinfo.width = xce.width;
			xinfo.height = xce.height;
			xinfo.resize = true;
			repaint();
		}

	}

	void closeWindow() {
		
		XCloseDisplay(xinfo.display);

	}


};

/*class Circle : public Displayable {

public:
        int x;
        int y;
        int d;
        string num;
        GC gc;
        Circle(int x, int y, int d, string num): x(x), y(y), d(d), num(num) {
                setDark = false;
                buttonPressed = false;
        }
        int screen;
        bool setDark;
        bool buttonPressed;
*/
        void Circle :: paint(XInfo& xinfo, Simon_Display* dis) {
                gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);
                screen = DefaultScreen(xinfo.display);
                XSetForeground(xinfo.display, gc, BlackPixel(xinfo.display, screen));
                XSetBackground(xinfo.display, gc, WhitePixel(xinfo.display, screen));
                XSetFillStyle(xinfo.display, gc, FillSolid);

		XFontStruct * font;
        	font = XLoadQueryFont(xinfo.display, "12x24");
        	XSetFont (xinfo.display, gc, font->fid);		
		//ring's gc//

		gc1 = XCreateGC(xinfo.display, xinfo.window, 0, 0);
		XSetForeground(xinfo.display, gc1, WhitePixel(xinfo.display, screen));
		XSetBackground(xinfo.display, gc1, BlackPixel(xinfo.display, screen));
		XSetFillStyle(xinfo.display, gc1,FillSolid);
		XSetLineAttributes(xinfo.display, gc1, 1, LineSolid, CapButt, JoinRound);
	
                if (buttonPressed){
                        XFillArc(xinfo.display, xinfo.window, gc, x, y, d, d, 0*64, 360*64);
			XDrawArc(xinfo.display, xinfo.window, gc1, ring_x, ring_y, ring_d, ring_d, 0*64, 360*64);
			if (d_decrease == 0) {
				//if (wait){
				//	usleep(1000000/2);
				//	wait = false;
				//}
				ring_x = d_x;
				ring_y = d_y;
				ring_d = d;
				buttonPressed = false;
				d_decrease = 30;
				//if (dis->sequence_play) {
				//	dis->newroundHandler();
				//}
				
			}			
			else {
				ring_x += (ring_d/2) / d_decrease;
				ring_y += (ring_d/2) / d_decrease;
				ring_d -= ring_d / d_decrease;
				if (d_decrease == 1) ring_d == 0;
				--d_decrease;
				
			}
                }

                else {
                        if (!setDark){
                                XSetLineAttributes(xinfo.display, gc, 1, LineSolid, CapButt, JoinRound);
                        }
                        else {
                                XSetLineAttributes(xinfo.display, gc, 4, LineSolid, CapButt, JoinRound);

                        }
                XDrawArc(xinfo.display, xinfo.window, gc, x, y, d, d, 0*64, 360*64);
                XDrawImageString(xinfo.display, xinfo.window, gc, x+45, y+55, num.c_str(), num.length());
		if(wait){
		//	usleep(1000000/2);
			wait = false;
			dis->if_wait = true;
			//done_paint = true;
		}

               }
		
	}

//};





