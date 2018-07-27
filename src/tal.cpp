#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <list>
#include <limits.h>
#include <iterator>
#include "debug.h"
#include "movestore.h"
#include "board.h"
#include "moves.h"

using namespace std;

bigdumb::Board myboard;
bool FORCE_MODE = false;

bool is_file(char c){
    return c>='a' && c<='h';
}

bool is_rank(char c){
    return c>='1' && c<='8';
}

bool is_chess_move(string s){
    return is_file(s[0]) && is_rank(s[1]) && is_file(s[2]) && is_rank(s[3]);
}

int main(){
    freopen("debug.txt", "w", stderr);

    do{
        string s;
        cin >> s;
        cerr << "winboard: " << s << endl;
        if((s.length()==4 || s.length()==5) && is_chess_move(s)){
            cerr << s << " is a valid move" << endl;
            freopen("debug.txt", "w", stderr);
            myboard.move(s);
            if(!FORCE_MODE) {
                myboard.abmin(INT_MIN, INT_MAX, ROOT_DEPTH);
            }
        }
        if(s=="xboard"){
            cout << "feature myname=\"bigdumb\"\n";
            cout << "feature done=1\n";
            precomputeKing();
            precomputeKnights();
            precomputePawns();
            precomputeSliding();
            cout << "sent features. init bitboards ready" << endl;
        }
        if(s=="new"){
            cerr << "setting up a new game..." << endl;
            myboard=bigdumb::Board();
        }
        if(s=="force"){
            FORCE_MODE = true;
            cerr << "force mode enabled" << endl;
        }
        if(s=="go"){
            FORCE_MODE = false;
            cerr << "force mode disabled" << endl;
            cerr << "engine playing as ";
            if(myboard.half_move%2==0) cerr <<"white"<<endl;
            else cerr << "black" << endl;
            // make a new move.
            myboard.abmin(INT_MIN, INT_MAX,ROOT_DEPTH);
        }
    }while(1);

    fclose(stderr);
    return 0;
}
