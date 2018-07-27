#ifndef _moves_h

#include <bitset>

namespace bigdumb{
    class ChessMove{
        public:
        int from;
        int to;
        int capture;
        int attacker;
        int _mobility_;
    };

    bool sortforwhitequiet(const ChessMove &m1, const ChessMove &m2){
        /*
        if((m1.attacker==3 || m1.attacker==4) &&
            (m2.attacker==10 || m2.attacker==5 || m2.attacker==1000)) return true;
        if((m2.attacker==3 || m2.attacker==4) &&
            (m1.attacker==10 || m1.attacker==5 || m1.attacker==1000)) return false;
        */
        return m1._mobility_ > m2._mobility_;
    }


    bool sortforwhitecapture(const ChessMove &m1, const ChessMove &m2){
        if(m1.capture==m2.capture){
            return m1.attacker < m2.attacker;
        }
        else return m1.capture<m2.capture;
    }

    bool sortforblackcapture(const ChessMove &m1, const ChessMove &m2){
        if(m1.capture==m2.capture){
            return m1.attacker > m2.attacker;
        }
        else return m1.capture>m2.capture;
    }

    bool sortforblackquiet(const ChessMove &m1, const ChessMove &m2){
        return m1._mobility_ >= m2._mobility_;
    }
}



void print(bigdumb::ChessMove m){
    std::cerr << "adding move from=" << m.from<<", to="<<m.to<<", attacker ="
    << m.attacker << ", capture="<<m.capture<<"\n";
}

#define _moves_h
#endif
