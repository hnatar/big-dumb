#ifndef _debug_h

#include <stdlib.h>
namespace bigdumb{
    void kill_engine(){
        std::cerr << "Something is wrong. Stopping execution.\n";
        system("pause>nul");
        exit(0);
    }
}

#define _debug_h
#endif
