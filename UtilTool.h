#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <time.h>

#ifndef UTILTOOL
#define UTILTOOL

static int myoffsetnum = 44;

class UtilTool{
public:

    static const int MAX_NUM = 100;
    //static int myoffsetnum;
    static int getrandnum(int num){
        if(num <= 0){
            return 0;
        }
        myoffsetnum++;
        srand((unsigned)time(NULL));
        return (int)((rand()+myoffsetnum)%num);
    }

};

#endif