#include <bitset>
#include <list>
#include <limits.h>
#include <string>
#include "debug.h"
#include "moves.h"
#include "psq.h"

#define MOBILITY_DRAG 10

int ROOT_DEPTH = 5;
int CAPTURE_DEPTH = 7;

namespace bigdumb{
    class Board{
        public:
            char a[8][8];
            int psq_value;
            std::bitset<64> white;
            std::bitset<64> black;
            std::bitset<64> pawns;
            std::bitset<64> bishops;
            std::bitset<64> knights;
            std::bitset<64> rooks;
            std::bitset<64> queens;
            std::bitset<64> kings;
            std::bitset<64> empty;
            std::bitset<64> occupied;
            std::bitset<64> attackmap;
            std::string variation;
            //
            std::list<ChessMove> quiet;
            std::list<ChessMove> capture;
            //
            void recompute_bitboards();
            int half_move;
            bool white_can_castle;
            bool black_can_castle;
            int enpassant_square;
            Board();
            void print_board();
            //
            bool valid_file(char);
            bool valid_rank(char);
            bool valid_piece(char);
            //
            void move(std::string);
            void move(int,int);
            //
            bool is_white(char);
            bool is_black(char);
            //
            void gen_w_rook_moves(int, int);
            void gen_b_rook_moves(int, int);
            //
            void gen_w_bishop_moves(int, int);
            void gen_b_bishop_moves(int, int);
            //
            void gen_w_queen_moves(int, int);
            void gen_b_queen_moves(int, int);
            //
            void gen_b_pawn_moves(int,int);
            void gen_w_pawn_moves(int,int);
            //
            void gen_b_knight_moves(int,int);
            void gen_w_knight_moves(int,int);
            //
            void gen_w_king_moves(int,int);
            void gen_b_king_moves(int,int);
            //
            void gen_moves();
            //
            void gen_knightmap();
            //
            void add_move_from_bitmap(int,std::bitset<64>);
            //
            void print_quiet();
            void print_capture();
            //
            int abmax(int,int,int);
			int abmin(int,int,int);
			int qblack(int,int);
			int qwhite(int,int);
			int board_white_value();
			int board_black_value();
            //
            int board_value();
            //
            int mobility;
			//
    };

    Board::Board(){
        // Set up descriptive board and
        // add the bitboards bits as place each
        // piece on the board.
        variation="";
        for(int y=0; y<8; y++){
            for(int x=0; x<8; x++){
                if(y==1) {
                    a[y][x]='p';
                    pawns.set(8*y+x);
                    black.set(8*y+x);
                }
                else if(y==6){
                    a[y][x]='P';
                    pawns.set(8*y+x);
                    white.set(8*y+x);
                }
                else a[y][x]='.';
            }
        }
        a[0][0]='r'; a[0][7]='r';
        rooks.set(0); rooks.set(7); black.set(0); black.set(7);
        a[0][1]='n'; a[0][6]='n';
        knights.set(1); knights.set(6); black.set(1); black.set(6);
        a[0][2]='b'; a[0][5]='b';
        bishops.set(2); bishops.set(5); black.set(2); black.set(5);
        a[0][3]='q'; a[0][4]='k';
        queens.set(3); black.set(3); kings.set(4); black.set(4);
        a[7][0]='R'; a[7][7]='R';
        rooks.set(56); rooks.set(63); white.set(56); white.set(63);
        a[7][1]='N'; a[7][6]='N';
        knights.set(57); knights.set(62); white.set(57); white.set(62);
        a[7][2]='B'; a[7][5]='B';
        bishops.set(58); bishops.set(61); white.set(58); white.set(61);
        a[7][3]='Q'; a[7][4]='K';
        queens.set(59); white.set(59); kings.set(60); white.set(60);
        // Other bitboards
        occupied = (white|black);
        empty = ~occupied;
        // Misc. flags
        half_move = 0;
        white_can_castle = true;
        black_can_castle = true;
        enpassant_square = 64;
        mobility=0;
    }

    void Board::recompute_bitboards(){
        attackmap.reset();
        rooks.reset();
        bishops.reset();
        knights.reset();
        kings.reset();
        queens.reset();
        pawns.reset();
        empty.reset();
        occupied.reset();
        white.reset();
        black.reset();
        for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
                int k=8*i+j;
                switch(a[i][j]){
                    case 'p': pawns.set(k); black.set(k); break;
                    case 'P': pawns.set(k); white.set(k); break;
                    case 'b': bishops.set(k); black.set(k); break;
                    case 'B': bishops.set(k); white.set(k); break;
                    case 'n': knights.set(k); black.set(k); break;
                    case 'N': knights.set(k); white.set(k); break;
                    case 'r': rooks.set(k); black.set(k); break;
                    case 'R': rooks.set(k); white.set(k); break;
                    case 'q': queens.set(k); black.set(k); break;
                    case 'Q': queens.set(k); white.set(k); break;
                    case 'k': kings.set(k); black.set(k); break;
                    case 'K': kings.set(k); white.set(k); break;
                    case '.': empty.set(k); break;
                    default: std::cerr << "ERROR UNKNOWN PIECE\n"; kill_engine(); break;
                }
            }
        }
        occupied=~empty;
    }

    void Board::print_board(){
        // prints the board array in a
        // nice format.
        std::cerr << "\n";
        for(int y=0; y<8; y++){
            std::cerr << " "<< 8-y<<" ";
            for(int x=0; x<8; x++){
                std::cerr << a[y][x] <<" ";
            }
            std::cerr << "\n";
        }
        std::cerr << "   a b c d e f g h\n";
        std::cerr << half_move << " ";
        if(half_move%2 == 0) std::cerr << "White to move\n";
        else std::cerr << "Black to move\n";
        //if(white_can_castle) std::cerr << "[white can castle]\n";
        //if(black_can_castle) std::cerr << "[black can castle]\n";
        //if(enpassant_square<64) std::cerr << "enpassant: " << enpassant_square << "\n";
    }

    void Board::move(std::string crd){
        enpassant_square=64;
        // handle moves in the WinBoard fashion
        // <file><rank><file><rank>[promotion?]
        if(crd.length()!=4 && crd.length()!=5){
            std::cerr << "error: move " << crd <<
            " is invalid. moves must be 4 or 5 characters in length.\n";
            kill_engine();
        }
        if( !valid_file(crd[0]) || !valid_rank(crd[1]) ||
            !valid_file(crd[2]) || !valid_rank(crd[3]) ) {
            std::cerr << "error: move " << crd << " is not "
            << "formatted properly.\n";
            kill_engine();
        }
        if(crd.length()==5 && !valid_piece(crd[4])){
            std::cerr << "error: the piece type " << crd[4] << " is not "
            << "recognized as a piece, promotion can't be done";
            kill_engine();
        }
        std::cerr << "received move " << crd << "\n";
        if(crd.length()==4)
            a['8'-crd[3]][crd[2]-'a']=a['8'-crd[1]][crd[0]-'a'];
        else
            a['8'-crd[3]][crd[2]-'a']=crd[4];
        a['8'-crd[1]][crd[0]-'a']='.';
        half_move++;
        if(crd=="e1g1" && a[7][6]=='K') white_can_castle=false;
        if(crd=="e1c1" && a[7][2]=='K') white_can_castle=false;
        if(crd=="e8g8" && a[0][6]=='k') black_can_castle=false;
        if(crd=="e8c8" && a[0][2]=='k') black_can_castle=false;
        if(crd[1]=='2' && crd[3]=='4' && a['8'-crd[3]][crd[2]-'a']=='P')
            enpassant_square=48+crd[0]-'a';
        else if(crd[1]=='7' && crd[3]=='5' && a['8'-crd[3]][crd[2]-'a']=='P')
            enpassant_square=16+crd[0]-'a';
        else enpassant_square=64;
        recompute_bitboards();
        print_board();
    }

    void Board::move(int from, int to){
        a[to/8][to%8]=a[from/8][from%8];
        a[from/8][from%8]='.';
    }

    bool Board::valid_file(char f){
        return f>='a' && f<='h';
    }

    bool Board::valid_rank(char r){
        return r>='1' && r<='8';
    }

    bool Board::valid_piece(char p){
        return p=='r' || p=='R'
            || p=='n' || p=='N'
            || p=='b' || p=='B'
            || p=='q' || p=='Q'
            || p=='k' || p=='K'
            || p=='p' || p=='P';
    }

    void Board::gen_w_rook_moves(int y, int x){
        std::bitset<64> rook_moves;
        std::bitset<64> up_moves=UP[8*y+x] & occupied;
        up_moves = up_moves >> 8 | up_moves >> 16 | up_moves >> 24 |
                    up_moves >> 32| up_moves >> 40 | up_moves >> 48;
        up_moves = up_moves & UP[8*y+x];
        up_moves = UP[8*y+x] ^ up_moves;
        up_moves = up_moves & (~white);

        std::bitset<64> down_moves=DOWN[8*y+x] & occupied;
        down_moves = down_moves << 8 | down_moves << 16 | down_moves << 24 |
                    down_moves << 32| down_moves << 40 | down_moves << 48;
        down_moves = down_moves & DOWN[8*y+x];
        down_moves = DOWN[8*y+x] ^ down_moves;
        down_moves = down_moves & (~white);

        std::bitset<64> right_moves=RIGHT[8*y+x] & occupied;
        right_moves = right_moves << 1 | right_moves << 2 | right_moves << 3 |
                    right_moves << 4 | right_moves << 5 | right_moves << 6;
        right_moves = right_moves & RIGHT[8*y+x];
        right_moves = RIGHT[8*y+x] ^ right_moves;
        right_moves = right_moves & (~white);

        std::bitset<64> left_moves = LEFT[8*y+x] & occupied;
        left_moves = left_moves >> 1 | left_moves >> 2 | left_moves >> 3 |
                    left_moves >> 4 | left_moves >> 5 | left_moves >> 6;
        left_moves = left_moves & LEFT[8*y+x];
        left_moves = LEFT[8*y+x] ^ left_moves;
        left_moves = left_moves & (~white);

        rook_moves = up_moves | left_moves | right_moves | down_moves;
        if(rook_moves.any()){
            //print(rook_moves);
            //std::cerr << "moves for rook at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, rook_moves);
        }
        mobility+=rook_moves.count();
        attackmap|=rook_moves;

    }

    void Board::gen_b_rook_moves(int y, int x){
        std::bitset<64> rook_moves;
        std::bitset<64> up_moves=UP[8*y+x] & occupied;
        up_moves = up_moves >> 8 | up_moves >> 16 | up_moves >> 24 |
                    up_moves >> 32| up_moves >> 40 | up_moves >> 48;
        up_moves = up_moves & UP[8*y+x];
        up_moves = UP[8*y+x] ^ up_moves;
        up_moves = up_moves & (~black);

        std::bitset<64> down_moves=DOWN[8*y+x] & occupied;
        down_moves = down_moves << 8 | down_moves << 16 | down_moves << 24 |
                    down_moves << 32| down_moves << 40 | down_moves << 48;
        down_moves = down_moves & DOWN[8*y+x];
        down_moves = DOWN[8*y+x] ^ down_moves;
        down_moves = down_moves & (~black);

        std::bitset<64> right_moves=RIGHT[8*y+x] & occupied;
        right_moves = right_moves << 1 | right_moves << 2 | right_moves << 3 |
                    right_moves << 4 | right_moves << 5 | right_moves << 6;
        right_moves = right_moves & RIGHT[8*y+x];
        right_moves = RIGHT[8*y+x] ^ right_moves;
        right_moves = right_moves & (~black);

        std::bitset<64> left_moves = LEFT[8*y+x] & occupied;
        left_moves = left_moves >> 1 | left_moves >> 2 | left_moves >> 3 |
                    left_moves >> 4 | left_moves >> 5 | left_moves >> 6;
        left_moves = left_moves & LEFT[8*y+x];
        left_moves = LEFT[8*y+x] ^ left_moves;
        left_moves = left_moves & (~black);

        rook_moves = up_moves | right_moves | left_moves | down_moves;
        if(rook_moves.any()){
            //print(rook_moves);
            //std::cerr << "moves for rook at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, rook_moves);

        }
        mobility+=rook_moves.count();
        attackmap|=rook_moves;
    }

    void Board::gen_w_bishop_moves(int y, int x){
        std::bitset<64> bishop_moves;

        std::bitset<64> left_up_moves = LEFTUP[8*y+x] & occupied;
        left_up_moves = left_up_moves >> 9 | left_up_moves >> 18 | left_up_moves >> 27
                    |   left_up_moves >> 36| left_up_moves >> 45 | left_up_moves >> 54;
        left_up_moves = left_up_moves & LEFTUP[8*y+x];
        left_up_moves = left_up_moves ^ LEFTUP[8*y+x];
        left_up_moves = left_up_moves & (~white);

        std::bitset<64> right_up_moves = RIGHTUP[8*y+x] & occupied;
        right_up_moves = right_up_moves >> 7 | right_up_moves >> 14 | right_up_moves >> 21
                        | right_up_moves >> 28 | right_up_moves >> 35 | right_up_moves >> 42;
        right_up_moves = right_up_moves & RIGHTUP[8*y+x];
        right_up_moves = right_up_moves ^ RIGHTUP[8*y+x];
        right_up_moves = right_up_moves & (~white);

        std::bitset<64> left_down_moves = LEFTDOWN[8*y+x] & occupied;
        left_down_moves = left_down_moves << 7 | left_down_moves << 14 | left_down_moves << 21
                        | left_down_moves << 28 | left_down_moves << 35 | left_down_moves << 42;
        left_down_moves = left_down_moves & LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves ^ LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves & (~white);

        std::bitset<64> right_down_moves = RIGHTDOWN[8*y+x] & occupied;
        right_down_moves = right_down_moves << 9 | right_down_moves << 18 | right_down_moves << 27
                        | right_down_moves << 36 | right_down_moves << 45 | right_down_moves << 54;
        right_down_moves = right_down_moves & RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves ^ RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves & (~white);

        bishop_moves = left_up_moves | right_down_moves | right_up_moves | left_down_moves;
        if(bishop_moves.any()){
            //print(bishop_moves);
            //std::cerr << "moves for bishop at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, bishop_moves);

        }
        mobility+=bishop_moves.count();
        attackmap|=bishop_moves;
    }

    void Board::gen_b_bishop_moves(int y, int x){
        std::bitset<64> bishop_moves;

        std::bitset<64> left_up_moves = LEFTUP[8*y+x] & occupied;
        left_up_moves = left_up_moves >> 9 | left_up_moves >> 18 | left_up_moves >> 27
                    |   left_up_moves >> 36| left_up_moves >> 45 | left_up_moves >> 54;
        left_up_moves = left_up_moves & LEFTUP[8*y+x];
        left_up_moves = left_up_moves ^ LEFTUP[8*y+x];
        left_up_moves = left_up_moves & (~black);

        std::bitset<64> right_up_moves = RIGHTUP[8*y+x] & occupied;
        right_up_moves = right_up_moves >> 7 | right_up_moves >> 14 | right_up_moves >> 21
                        | right_up_moves >> 28 | right_up_moves >> 35 | right_up_moves >> 42;
        right_up_moves = right_up_moves & RIGHTUP[8*y+x];
        right_up_moves = right_up_moves ^ RIGHTUP[8*y+x];
        right_up_moves = right_up_moves & (~black);

        std::bitset<64> left_down_moves = LEFTDOWN[8*y+x] & occupied;
        left_down_moves = left_down_moves << 7 | left_down_moves << 14 | left_down_moves << 21
                        | left_down_moves << 28 | left_down_moves << 35 | left_down_moves << 42;
        left_down_moves = left_down_moves & LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves ^ LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves & (~black);

        std::bitset<64> right_down_moves = RIGHTDOWN[8*y+x] & occupied;
        right_down_moves = right_down_moves << 9 | right_down_moves << 18 | right_down_moves << 27
                        | right_down_moves << 36 | right_down_moves << 45 | right_down_moves << 54;
        right_down_moves = right_down_moves & RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves ^ RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves & (~black);

        bishop_moves = left_up_moves | right_down_moves | right_up_moves | left_down_moves;
        if(bishop_moves.any()){
            //print(bishop_moves);
            //std::cerr << "moves for bishop at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, bishop_moves);

        }
        mobility+=bishop_moves.count();
        attackmap|=bishop_moves;
    }

    void Board::gen_w_queen_moves(int y, int x){
        std::bitset<64> queen_moves;

        std::bitset<64> rook_moves;
        std::bitset<64> up_moves=UP[8*y+x] & occupied;
        up_moves = up_moves >> 8 | up_moves >> 16 | up_moves >> 24 |
                    up_moves >> 32| up_moves >> 40 | up_moves >> 48;
        up_moves = up_moves & UP[8*y+x];
        up_moves = UP[8*y+x] ^ up_moves;
        up_moves = up_moves & (~white);

        std::bitset<64> down_moves=DOWN[8*y+x] & occupied;
        down_moves = down_moves << 8 | down_moves << 16 | down_moves << 24 |
                    down_moves << 32| down_moves << 40 | down_moves << 48;
        down_moves = down_moves & DOWN[8*y+x];
        down_moves = DOWN[8*y+x] ^ down_moves;
        down_moves = down_moves & (~white);

        std::bitset<64> right_moves=RIGHT[8*y+x] & occupied;
        right_moves = right_moves << 1 | right_moves << 2 | right_moves << 3 |
                    right_moves << 4 | right_moves << 5 | right_moves << 6;
        right_moves = right_moves & RIGHT[8*y+x];
        right_moves = RIGHT[8*y+x] ^ right_moves;
        right_moves = right_moves & (~white);

        std::bitset<64> left_moves = LEFT[8*y+x] & occupied;
        left_moves = left_moves >> 1 | left_moves >> 2 | left_moves >> 3 |
                    left_moves >> 4 | left_moves >> 5 | left_moves >> 6;
        left_moves = left_moves & LEFT[8*y+x];
        left_moves = LEFT[8*y+x] ^ left_moves;
        left_moves = left_moves & (~white);

        rook_moves = up_moves | left_moves | right_moves | down_moves;

        std::bitset<64> bishop_moves;

        std::bitset<64> left_up_moves = LEFTUP[8*y+x] & occupied;
        left_up_moves = left_up_moves >> 9 | left_up_moves >> 18 | left_up_moves >> 27
                    |   left_up_moves >> 36| left_up_moves >> 45 | left_up_moves >> 54;
        left_up_moves = left_up_moves & LEFTUP[8*y+x];
        left_up_moves = left_up_moves ^ LEFTUP[8*y+x];
        left_up_moves = left_up_moves & (~white);

        std::bitset<64> right_up_moves = RIGHTUP[8*y+x] & occupied;
        right_up_moves = right_up_moves >> 7 | right_up_moves >> 14 | right_up_moves >> 21
                        | right_up_moves >> 28 | right_up_moves >> 35 | right_up_moves >> 42;
        right_up_moves = right_up_moves & RIGHTUP[8*y+x];
        right_up_moves = right_up_moves ^ RIGHTUP[8*y+x];
        right_up_moves = right_up_moves & (~white);

        std::bitset<64> left_down_moves = LEFTDOWN[8*y+x] & occupied;
        left_down_moves = left_down_moves << 7 | left_down_moves << 14 | left_down_moves << 21
                        | left_down_moves << 28 | left_down_moves << 35 | left_down_moves << 42;
        left_down_moves = left_down_moves & LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves ^ LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves & (~white);

        std::bitset<64> right_down_moves = RIGHTDOWN[8*y+x] & occupied;
        right_down_moves = right_down_moves << 9 | right_down_moves << 18 | right_down_moves << 27
                        | right_down_moves << 36 | right_down_moves << 45 | right_down_moves << 54;
        right_down_moves = right_down_moves & RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves ^ RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves & (~white);

        bishop_moves = left_up_moves | right_down_moves | right_up_moves | left_down_moves;

        queen_moves = bishop_moves | rook_moves;
        if(queen_moves.any()){
            //print(queen_moves);
            //std::cerr << "moves for queen at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, queen_moves);

        }
        mobility+=queen_moves.count();
        attackmap|=queen_moves;
    }

    void Board::gen_b_queen_moves(int y,int x){
        std::bitset<64> queen_moves;
        std::bitset<64> bishop_moves;

        std::bitset<64> left_up_moves = LEFTUP[8*y+x] & occupied;
        left_up_moves = left_up_moves >> 9 | left_up_moves >> 18 | left_up_moves >> 27
                    |   left_up_moves >> 36| left_up_moves >> 45 | left_up_moves >> 54;
        left_up_moves = left_up_moves & LEFTUP[8*y+x];
        left_up_moves = left_up_moves ^ LEFTUP[8*y+x];
        left_up_moves = left_up_moves & (~black);

        std::bitset<64> right_up_moves = RIGHTUP[8*y+x] & occupied;
        right_up_moves = right_up_moves >> 7 | right_up_moves >> 14 | right_up_moves >> 21
                        | right_up_moves >> 28 | right_up_moves >> 35 | right_up_moves >> 42;
        right_up_moves = right_up_moves & RIGHTUP[8*y+x];
        right_up_moves = right_up_moves ^ RIGHTUP[8*y+x];
        right_up_moves = right_up_moves & (~black);

        std::bitset<64> left_down_moves = LEFTDOWN[8*y+x] & occupied;
        left_down_moves = left_down_moves << 7 | left_down_moves << 14 | left_down_moves << 21
                        | left_down_moves << 28 | left_down_moves << 35 | left_down_moves << 42;
        left_down_moves = left_down_moves & LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves ^ LEFTDOWN[8*y+x];
        left_down_moves = left_down_moves & (~black);

        std::bitset<64> right_down_moves = RIGHTDOWN[8*y+x] & occupied;
        right_down_moves = right_down_moves << 9 | right_down_moves << 18 | right_down_moves << 27
                        | right_down_moves << 36 | right_down_moves << 45 | right_down_moves << 54;
        right_down_moves = right_down_moves & RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves ^ RIGHTDOWN[8*y+x];
        right_down_moves = right_down_moves & (~black);

        bishop_moves = left_up_moves | right_down_moves | right_up_moves | left_down_moves;

         std::bitset<64> rook_moves;
        std::bitset<64> up_moves=UP[8*y+x] & occupied;
        up_moves = up_moves >> 8 | up_moves >> 16 | up_moves >> 24 |
                    up_moves >> 32| up_moves >> 40 | up_moves >> 48;
        up_moves = up_moves & UP[8*y+x];
        up_moves = UP[8*y+x] ^ up_moves;
        up_moves = up_moves & (~black);

        std::bitset<64> down_moves=DOWN[8*y+x] & occupied;
        down_moves = down_moves << 8 | down_moves << 16 | down_moves << 24 |
                    down_moves << 32| down_moves << 40 | down_moves << 48;
        down_moves = down_moves & DOWN[8*y+x];
        down_moves = DOWN[8*y+x] ^ down_moves;
        down_moves = down_moves & (~black);

        std::bitset<64> right_moves=RIGHT[8*y+x] & occupied;
        right_moves = right_moves << 1 | right_moves << 2 | right_moves << 3 |
                    right_moves << 4 | right_moves << 5 | right_moves << 6;
        right_moves = right_moves & RIGHT[8*y+x];
        right_moves = RIGHT[8*y+x] ^ right_moves;
        right_moves = right_moves & (~black);

        std::bitset<64> left_moves = LEFT[8*y+x] & occupied;
        left_moves = left_moves >> 1 | left_moves >> 2 | left_moves >> 3 |
                    left_moves >> 4 | left_moves >> 5 | left_moves >> 6;
        left_moves = left_moves & LEFT[8*y+x];
        left_moves = LEFT[8*y+x] ^ left_moves;
        left_moves = left_moves & (~black);

        rook_moves = up_moves | right_moves | left_moves | down_moves;
        queen_moves = rook_moves | bishop_moves;
        if(queen_moves.any()){
            //print(queen_moves);
            //std::cerr << "moves for queen at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, queen_moves);

        }
        mobility+=queen_moves.count();
        attackmap|=queen_moves;
    }

    void Board::gen_b_pawn_moves(int y,int x){
        int index=8*y+x;
        std::bitset<64> pawn_moves;
        std::bitset<64> pawn_pushes;
        std::bitset<64> pawn_cross;
        if(y<7 && empty.test(index+8)) pawn_pushes.set(index+8);
        if(y==1 && empty.test(index+8) && empty.test(index+16)) pawn_pushes.set(index+16);
        if(y<7 && x>0 && white.test(index+7)) pawn_cross.set(index+7);
        if(y<7 && x<7 && white.test(index+9)) pawn_cross.set(index+9);
        if(y<7 && x>0 && enpassant_square==index+7){
            pawn_cross.set(index+7);
            enpassant_square=64;
        }
        if(y<7 && x<7 && enpassant_square==index+9){
            pawn_cross.set(index+9);
            enpassant_square=64;
        }
        pawn_moves = pawn_pushes | pawn_cross;
        if(pawn_moves.any()){
            //print(pawn_moves);
            //std::cerr << "moves for pawn at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, pawn_moves);

        }
        mobility+=pawn_moves.count();
        attackmap|=pawn_moves;
    }

    void Board::gen_w_pawn_moves(int y,int x){
        int index=8*y+x;
        std::bitset<64> pawn_moves;
        std::bitset<64> pawn_pushes;
        std::bitset<64> pawn_cross;
        if(y>0 && empty.test(index-8)) pawn_pushes.set(index-8);
        if(y==6 && empty.test(index-8) && empty.test(index-16)) pawn_pushes.set(index-16);
        if(y>0 && x<7 && black.test(index-7)) pawn_cross.set(index-7);
        if(y>0 && x>0 && black.test(index-9)) pawn_cross.set(index-9);
        if(y>0 && x>0 && enpassant_square==index-7){
            pawn_cross.set(index-7);
            enpassant_square=64;
        }
        if(y>0 && x<7 && enpassant_square==index-9){
            pawn_cross.set(index-9);
            enpassant_square=64;
        }
        pawn_moves = pawn_pushes | pawn_cross;
        if(pawn_moves.any()){
            //print(pawn_moves);
            //std::cerr << "moves for pawn at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, pawn_moves);

        }
        mobility+=pawn_moves.count();
        attackmap|=pawn_moves;
    }

    void Board::gen_b_knight_moves(int y, int x){
        std::bitset<64> knight_moves = N[8*y+x] & (~black);
        if(knight_moves.any()){
            //print(knight_moves);
            //std::cerr << "moves for knight at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, knight_moves);

        }
        mobility+=knight_moves.count();
        attackmap|=knight_moves;
    }

    void Board::gen_w_knight_moves(int y, int x){
        std::bitset<64> knight_moves = N[8*y+x] & (~white);
        if(knight_moves.any()){
            //print(knight_moves);
            //std::cerr << "moves for knight at ("<<y<<", "<<x<<")\n";
            add_move_from_bitmap(8*y+x, knight_moves);

        }
        mobility+=knight_moves.count();
        attackmap|=knight_moves;
    }

    void Board::gen_w_king_moves(int y,int x){
        std::bitset<64> king_moves = K[8*y+x] &(~white);
        if(king_moves.any()){
            add_move_from_bitmap(8*y+x, king_moves);
        }
    }

    void Board::gen_b_king_moves(int y,int x){
        std::bitset<64> king_moves = K[8*y+x] &(~black);
        if(king_moves.any()){
            add_move_from_bitmap(8*y+x, king_moves);
        }
    }

    void Board::gen_moves(){
        mobility=0;
        psq_value=0;
        quiet.clear();
        capture.clear();
        recompute_bitboards();
        for(int y=0; y<8; y++){
            for(int x=0; x<8; x++){
                if(half_move%2==0){
                    switch(a[y][x]){
                        case 'R': gen_w_rook_moves(y,x); psq_value+=ROOK_PSQ[8*y+x]; break;
                        case 'B': gen_w_bishop_moves(y,x); psq_value+=BISHOP_PSQ[8*y+x];break;
                        case 'Q': gen_w_queen_moves(y,x); psq_value+=QUEEN_PSQ[8*y+x]; break;
                        case 'P': gen_w_pawn_moves(y,x); psq_value+=PAWN_PSQ[8*y+x]; break;
                        case 'N': gen_w_knight_moves(y,x); psq_value+=KNIGHT_PSQ[8*y+x]; break;
                        case 'K': gen_w_king_moves(y,x); psq_value+=KING_PSQ[8*y+x]; break;
                    }
                }
                else{
                    switch(a[y][x]){
                        case 'r': gen_b_rook_moves(y,x); psq_value+=ROOK_PSQ[64-8*y+x]; break;
                        case 'b': gen_b_bishop_moves(y,x); psq_value+=BISHOP_PSQ[64-8*y+x];break;
                        case 'q': gen_b_queen_moves(y,x); psq_value+=QUEEN_PSQ[64-8*y+x];break;
                        case 'p': gen_b_pawn_moves(y,x); break;
                        case 'n': gen_b_knight_moves(y,x); psq_value+=KNIGHT_PSQ[64-8*y+x];break;
                        case 'k': gen_b_king_moves(y,x);psq_value+=KING_PSQ[64-8*y+x]; break;
                    }
                }
            }
        }
        if(half_move%2==0){
            quiet.sort(bigdumb::sortforwhitequiet);
            capture.sort(bigdumb::sortforwhitecapture);
        }
        else{
            quiet.sort(bigdumb::sortforblackquiet);
            capture.sort(bigdumb::sortforblackcapture);
        }
        //print_capture();
        //print_quiet();
    }

    void Board::print_quiet(){
        for(std::list<ChessMove>::iterator i=quiet.begin(); i!=quiet.end(); i++){
            int from=(*i).from;
            int to = (*i).to;
			std::string variation("");
            variation += static_cast<char>('a'+(from%8));
            variation += static_cast<char>('8'-(from/8));
            variation += static_cast<char>('a'+(to%8));
            variation += static_cast<char>('8'-(to/8));
            std::cerr << "QUIET MOVE " << variation << std::endl;
        }
    }

    void Board::print_capture(){
        for(std::list<ChessMove>::iterator i=capture.begin(); i!=capture.end(); i++){
            int from=(*i).from;
            int to = (*i).to;
            int capture = (*i).capture;
            int attacker = (*i).attacker;
			std::string variation("");
            variation += static_cast<char>('a'+(from%8));
            variation += static_cast<char>('8'-(from/8));
            variation += static_cast<char>('a'+(to%8));
            variation += static_cast<char>('8'-(to/8));
            std::cerr << "CAPTURE " << variation << " attacker="<<attacker<<", capture="<<capture<<"\n";
        }
    }

    void Board::gen_knightmap(){
        std::bitset<64> temp=knights & black;
        std::bitset<64> kmap;
        int jumps = 0;
        do{
            for(int i=0; i<64; i++){
                if(temp.test(i)) kmap |= N[i];
            }
            temp=kmap;
            print(kmap);
            jumps++;
        }while(kmap.count()<64);

    }

    int Board::board_white_value(){
		int mval=0;
        for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
                switch(a[i][j]){
                    case 'R': mval+=500 + ROOK_PSQ[8*i+j]; break;
                    case 'N': mval+=320 + KNIGHT_PSQ[8*i+j]; break;
                    case 'B': mval+=330 + BISHOP_PSQ[8*i+j]; break;
                    case 'Q': mval+=900 + QUEEN_PSQ[8*i+j]; break;
                    case 'K': mval+=20000 + KING_PSQ[8*i+j]; break;
                    case 'P': mval+=100 + PAWN_PSQ[8*i+j]; break;
					default: break;
                }
            }
        }
		return mval;
    }
	
    int Board::board_black_value(){
		int mval=0;
        for(int i=0; i<8; i++){
            for(int j=0; j<8; j++){
                switch(a[i][j]){
                    case 'r': mval+=500 + ROOK_PSQ[8*(7-i)+j]; break;
                    case 'n': mval+=320 + KNIGHT_PSQ[8*(7-i)+j]; break;
                    case 'b': mval+=330 + BISHOP_PSQ[8*(7-i)+j]; break;
                    case 'q': mval+=900 + QUEEN_PSQ[8*(7-i)+j]; break;
                    case 'k': mval+=20000 + KING_PSQ[8*(7-i)+j]; break;
                    case 'p': mval+=100 + PAWN_PSQ[8*(7-i)+j]; break;
					default: break;
                }
            }
        }
		return mval;
    }


    int Board::abmax(int alpha, int beta, int depth){
		if(depth==ROOT_DEPTH) variation=std::string("");
		if(depth==0){
			return board_white_value() - board_black_value();;
		}
		else{
			gen_moves();
			int bestfrom, bestto;
			int max=INT_MIN;
			for(std::list<ChessMove>::iterator i=capture.begin(); i!=capture.end(); i++){
				int from = (*i).from, to = (*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.abmin(alpha, beta, depth-1);
				if(x>max) {
					max=x;
					bestfrom=from;
					bestto=to;
				}
				if(x>alpha) alpha=x;
				if(alpha>=beta) return alpha;
			}
			for(std::list<ChessMove>::iterator i=quiet.begin(); i!=quiet.end(); i++){
				int from = (*i).from, to=(*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.abmin(alpha, beta, depth-1);
				if(x>max) {
					max=x;
					bestfrom=from;
					bestto=to;
				}
				if(x>alpha) alpha=x;
				if(alpha>=beta) return alpha;
			}

			if(depth==ROOT_DEPTH){
				move(bestfrom, bestto);
				half_move++;
				print_board();
				std::cout << "move "<<static_cast<char>('a'+(bestfrom%8))
						  << static_cast<char>('8'-(bestfrom/8))
						  << static_cast<char>('a'+(bestto%8))
						  << static_cast<char>('8'-(bestto/8)) << std::endl;
			}
			else return max;
		}
    }
	
    int Board::qwhite(int alpha, int beta){
		gen_moves();
		if(capture.empty()){
			return board_white_value() - board_black_value();;
		}
		else{
			int bestfrom, bestto;
			int max=INT_MIN;
			for(std::list<ChessMove>::iterator i=capture.begin(); i!=capture.end(); i++){
				int from = (*i).from, to = (*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.qblack(alpha, beta);
				if(x>max) {
					max=x;
					bestfrom=from;
					bestto=to;
				}
				if(x>alpha) alpha=x;
				if(alpha>=beta) return alpha;
			}
			return max;
		}
    }
	
    int Board::qblack(int alpha, int beta){
		gen_moves();
		if(capture.empty()){
			return board_black_value() - board_white_value();;
		}
		else{
			int bestfrom, bestto;
			int min=INT_MAX;
			for(std::list<ChessMove>::iterator i=capture.begin(); i!=capture.end(); i++){
				int from = (*i).from, to = (*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.qwhite(alpha, beta);
				if(x<min) {
					min=x;
					bestfrom=from;
					bestto=to;
				}
				if(x<beta) beta=x;
				if(alpha>=beta) return beta;
			}
			return min;
		}
    }
	
    int Board::abmin(int alpha, int beta, int depth){
		if(depth==ROOT_DEPTH) variation=std::string("");
		if(depth==0){
			return board_black_value() - board_white_value();;
		}
		else{
			gen_moves();
			int bestfrom, bestto;
			int min=INT_MAX;
			for(std::list<ChessMove>::iterator i=capture.begin(); i!=capture.end(); i++){
				int from = (*i).from, to = (*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.abmax(alpha, beta, depth-1);
				if(x<min) {
					min=x;
					bestfrom=from;
					bestto=to;
				}
				if(x<beta) beta=x;
				if(alpha>=beta) return beta;
			}
			for(std::list<ChessMove>::iterator i=quiet.begin(); i!=quiet.end(); i++){
				int from = (*i).from, to=(*i).to;
				Board temp_board = *this;
				temp_board.move(from,to);
				temp_board.half_move++;
                temp_board.variation += " ";
                temp_board.variation += static_cast<char>('a'+(from%8));
                temp_board.variation += static_cast<char>('8'-(from/8));
                temp_board.variation += static_cast<char>('a'+(to%8));
                temp_board.variation += static_cast<char>('8'-(to/8));
				int x = temp_board.abmax(alpha, beta, depth-1);
				if(x<min) {
					min=x;
					bestfrom=from;
					bestto=to;
				}
				if(x<beta) beta=x;
				if(alpha>=beta) return beta;
			}

			if(depth==ROOT_DEPTH){
				move(bestfrom, bestto);
				half_move++;
				print_board();
				std::cout << "move "<<static_cast<char>('a'+(bestfrom%8))
						  << static_cast<char>('8'-(bestfrom/8))
						  << static_cast<char>('a'+(bestto%8))
						  << static_cast<char>('8'-(bestto/8)) << std::endl;
			}
			else return min;
		}
    }
	

    bool Board::is_white(char p){
        return (p=='P' || p=='B' || p=='N' || p=='R' || p=='Q' || p=='K');
    }
    bool Board::is_black(char p){
        return (p=='p' || p=='b' || p=='n' || p=='r' || p=='q' || p=='k');
    }

    void Board::add_move_from_bitmap(int s_from, std::bitset<64> bitmap){
        ChessMove temp;
        temp.from=s_from;
        temp._mobility_=bitmap.count();
        char def=a[s_from>>3][s_from&7];
        if(def=='r') temp.attacker=-5;
        if(def=='n') temp.attacker=-3;
        if(def=='b') temp.attacker=-3;
        if(def=='q') temp.attacker=-10;
        if(def=='k') temp.attacker=-1000;
        if(def=='p') temp.attacker=-1;
        if(def=='R') temp.attacker=5;
        if(def=='N') temp.attacker=3;
        if(def=='B') temp.attacker=3;
        if(def=='Q') temp.attacker=10;
        if(def=='K') temp.attacker=1000;
        if(def=='P') temp.attacker=1;
        for(int i=0; i<64; i++){
            if(bitmap.test(i)){
                temp.to=i;
                def=a[i>>3][i&7];
                if(def=='r') temp.capture=-5;
                if(def=='n') temp.capture=-3;
                if(def=='b') temp.capture=-3;
                if(def=='q') temp.capture=-10;
                if(def=='k') temp.capture=-1000;
                if(def=='p') temp.capture=-1;
                if(def=='R') temp.capture=5;
                if(def=='N') temp.capture=3;
                if(def=='B') temp.capture=3;
                if(def=='Q') temp.capture=10;
                if(def=='K') temp.capture=1000;
                if(def=='P') temp.capture=1;
                if(def=='.') temp.capture=0;
                if(def=='.'){
                    quiet.push_back(temp);
                    //std::cerr << "qui-";
                    //print(temp);
                }
                else{
                    capture.push_back(temp);
                    //std::cerr << "cap-";
                    //print(temp);
                }
            }
        }
    }
}
