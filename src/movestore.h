#ifndef _movestore_h
#include <bitset>
#include "moves.h"

std::bitset<64> BLACKPAWNFORK[64];
std::bitset<64> WHITEPAWNFORK[64];
std::bitset<64> BLACKPAWNPUSH[64];
std::bitset<64> WHITEPAWNPUSH[64];
std::bitset<64> DIAG[15];
std::bitset<64> OFFDIAG[15];
std::bitset<64> RIGHT[64], LEFT[64], UP[64], DOWN[64];
std::bitset<64> LEFTUP[64], RIGHTUP[64], LEFTDOWN[64], RIGHTDOWN[64];
std::bitset<64> N[64];
std::bitset<64> K[64];

void precomputePawns(){
    for(int i=0; i<64; i++){
        int x=i&7, y=i>>3;
        if(y==1){
            BLACKPAWNPUSH[i].set(i+16);
        }
        if(y+1<=7) {
            BLACKPAWNPUSH[i].set(i+8);
            if(x-1>=0) BLACKPAWNFORK[i].set(i+7);
            if(x+1<=7) BLACKPAWNFORK[i].set(i+9);
        }
        if(y==6){
            WHITEPAWNPUSH[i].set(i-16);
        }
        if(y-1>=0) {
            WHITEPAWNPUSH[i].set(i-8);
            if(x-1>=0) WHITEPAWNFORK[i].set(i-9);
            if(x+1<=7) WHITEPAWNFORK[i].set(i-7);
        }
    }
}

void precomputeSliding(){
    for(int i=0; i<64; i++){
        RIGHT[i].reset();
        LEFT[i].reset();
        UP[i].reset();
        DOWN[i].reset();
        int x=i&7, y=i>>3, j;
        for(j=x+1; j<8; j++) RIGHT[i].set(8*y+j);
        for(j=x-1; j>=0; j--) LEFT[i].set(8*y+j);
        for(j=y-1; j>=0; j--) UP[i].set(8*j+x);
        for(j=y+1; j<8; j++) DOWN[i].set(8*j+x);
        //
        LEFTUP[i].reset();
        RIGHTUP[i].reset();
        LEFTDOWN[i].reset();
        RIGHTDOWN[i].reset();
        int k;
        for(j=x-1,k=y-1; j>=0 && k>=0; j--, k--) LEFTUP[i].set(8*k+j);
        for(j=x+1,k=y-1; j<=7 && k>=0; j++, k--) RIGHTUP[i].set(8*k+j);
        for(j=x-1,k=y+1; j>=0 && k<=7; j--, k++) LEFTDOWN[i].set(8*k+j);
        for(j=x+1,k=y+1; j<=7 && k<=7; j++, k++) RIGHTDOWN[i].set(8*k+j);
    }
}

void precomputeKnights(){
    for(int i=0; i<64; i++){
        int x=i&7, y=i>>3;
        if(x-1>=0 && y-2>=0) N[i].set(i-17);
        if(x+1<=7 && y-2>=0) N[i].set(i-15);
        if(x-2>=0 && y-1>=0) N[i].set(i-10);
        if(x+2<=7 && y-1>=0) N[i].set(i-6);
        if(x-2>=0 && y+1<=7) N[i].set(i+6);
        if(x+2<=7 && y+1<=7) N[i].set(i+10);
        if(x-1>=0 && y+2<=7) N[i].set(i+15);
        if(x+1<=7 && y+2<=7) N[i].set(i+17);
    }
}

void precomputeKing(){
    for(int i=0; i<64; i++){
        int x=i&7, y=i>>3;
        if(x-1>=0 && y-1>=0) K[i].set(i-9);
        if(y-1>=0) K[i].set(i-8);
        if(x+1<=7 && y-1>=0) K[i].set(i-7);
        if(x-1>=0) K[i].set(i-1);
        if(x+1<=7) K[i].set(i+1);
        if(x-1>=0 && y+1<=7) K[i].set(i+7);
        if(y+1<=7) K[i].set(i+8);
        if(x+1<=7 && y+1<=7) K[i].set(i+9);
    }
}

void print(std::bitset<64> bitboard){
    for(int i=0; i<64; i++){
        if(i%8==0) std::cerr << "\n" << 8-(i/8);
        if(bitboard[i]==0) std::cerr << " .";
        else std::cerr << " x";
    }
    std::cerr << "\n/ a b c d e f g h\n";
}

#define _movestore_h
#endif
