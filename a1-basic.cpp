#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

#include "simon.h"
#include "simon_display.h"

int main ( int argc, char* argv[] ) {

	// get the number of buttons from args
	// (default to 4 if no args)
	int n = 4;
	
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    n = max(1, min(n, 9));
	
	
    // create the Simon game object
	Simon simon = Simon(n, true);

   // create display objects
	Simon_Display one_game = Simon_Display(&simon);
	one_game.xinfo.buttonNum = n;
   // open the window
	one_game.openWindow(argc, argv);
	//cout << "Press q to exit";
	//cin.get();
	//one_game.closeWindow();

  // create a simple graphics context
	GC gc = XCreateGC(one_game.xinfo.display, one_game.xinfo.window, 0, 0);
	int screen = DefaultScreen(one_game.xinfo.display);
	XSetForeground(one_game.xinfo.display, gc, BlackPixel(one_game.xinfo.display, screen));
	XSetBackground(one_game.xinfo.display, gc, WhitePixel(one_game.xinfo.display, screen));
	
  //Setting the font
	XFontStruct * font;
	font = XLoadQueryFont(one_game.xinfo.display, "12x24");
	XSetFont (one_game.xinfo.display, gc, font->fid);

	one_game.xinfo.gc = gc;

  //Displable 
	///vector<Displayable*> dVec;
	one_game.dVec.push_back(new Text(50, 50, "0"));
	one_game.dVec.push_back(new Text(50, 80, "Press Space to Play"));
	int space = (800 - (simon.getNumButtons() * 100)) / (simon.getNumButtons() + 1);

	for (int i = 1; i <= simon.getNumButtons(); i++) {
		stringstream ss;
		ss << i;
		string k = ss.str();
		one_game.dVec.push_back(new Circle(space*i+100*(i-1), 150, 100, k));

	}
	
	one_game.repaint();
	
	
	XFlush(one_game.xinfo.display);

	cout << "Press q to exit" << endl;
	one_game.eventloop();

	//cout << "Press q to exit";
	//cin.get();
	//one_game.closeWindow();	

/*
	cout << "Playing with " << simon.getNumButtons() << " buttons." << endl;

	while (true) {
		// leave some space between rounds
		cout << endl;

		switch (simon.getState()) {

		// will only be in this state right after Simon object is contructed
		case Simon::START:
			cout << "Press ENTER to play" << endl;
			break;
		// they won last round
		// score is increased by 1, sequence length is increased by 1
		case Simon::WIN:
			cout << "You won! Press ENTER to continue." << endl;
			break;
		// they lost last round
		// score is reset to 0, sequence length is reset to 1
		case Simon::LOSE:
			cout << "You lose. Press ENTER to play again." << endl;
			break;
		default:
			// should never be any other state at this point ...
			break;
		}
		cin.get();

		// start new round with a new sequence
		simon.newRound();

		// computer plays
		cout << "Watch what I do ..." << endl;
		// just output the values 
		// (obviously this demo is not very challenging)
		while (simon.getState() == Simon::COMPUTER) {
			cout << simon.nextButton() << endl;
		}

		// now human plays
		cout << "Your turn ..." << endl;
		while (simon.getState() == Simon::HUMAN) {
			int i;
			cin >> i;
			// see if guess was correct
			if (!simon.verifyButton(i)) {
				cout << "wrong" << endl;
			}
		}

		// print the score
		cout << "score: " << simon.getScore() << endl;

	}
*/
}
