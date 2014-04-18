#ifndef TGFW_PUZZLE_H
#define TGFW_PUZZLE_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

template<int WIDTH, int HEIGHT, typename LOGIC>
class BlockPuzzleBase {
 public:

    inline void erase(){
        reinterpret_cast<LOGIC*>(this)->eraseBlocks();
    }

    inline void feed(){
        reinterpret_cast<LOGIC*>(this)->feedBlocks();
    }

    inline int hasBlank(){
        return reinterpret_cast<LOGIC*>(this)->checkBlank();
    }

    inline void fall(){
        reinterpret_cast<LOGIC*>(this)->fallBlocks();
    }

    inline void swap(int *lho,int *rho){
        int temp = *lho;
        *lho = *rho;
        *rho = temp;
    }

    inline void swap(int x,int y,int dx,int dy){
        swap(&blocks[y*WIDTH+x],&blocks[dy*WIDTH+dx]);
    }

    inline void initializeBlocks(){
        memset(blocks,-1,sizeof(blocks));
        memset(tempblocks,0,sizeof(tempblocks));
    }

    inline void print(){
        reinterpret_cast<LOGIC*>(this)->printBlocks();
    }
    inline void move(int x,int y){
        reinterpret_cast<LOGIC*>(this)->moveCursor(x,y);
    }

    inline int *get(){
        return reinterpret_cast<LOGIC*>(this)->getCurrent();
    }

    inline int getWidth(){
        return WIDTH;
    }

    inline int getHeight(){
        return HEIGHT;
    }

 private:
 protected:
    int blocks[HEIGHT][WIDTH];
    int tempblocks[HEIGHT][WIDTH];
    int *block_table;
    int table_size;
};

#include "getch.h"

class PuzzleLogic : public BlockPuzzleBase<20,15,PuzzleLogic> {
 public:
 PuzzleLogic():mCursorX(0),mCursorY(0),combo(0),score(0){
    }

    void initTable(){
        for(int i = 0;i < sizeof(mTable)/sizeof(int);++i){
            mTable[i] = i % 6;
        }
    }

    int checkRay(int value,int x,int y,int prevx,int prevy){
        int result = 0;

        if(((x >= 0) && (x < getWidth())) &&
           ((y >= 0) && (y < getHeight()))
           ){
            if(blocks[y][x] == value){
                result = 1;
                int nextx = x + x - prevx;
                int nexty = y + y - prevy;
                if(((nextx >= 0) && (nextx < getWidth())) &&
                   ((nexty >= 0) && (nexty < getHeight()))
                   ){
                    result += checkRay(value,nextx,nexty,x,y)+1;
                }
            }
        }
        return result;
    }

    int eraseAnimation(){
        int erased = 0;
        for(int y = 0;y < getHeight();++y){
            for(int x = 0;x < getWidth();++x){
                if(tempblocks[y][x]>0){
                    if(--tempblocks[y][x] == 0){
                        blocks[y][x] = -1;
                        score += combo*combo*10;
                    }
                    ++erased;
                }
            }
        }
        return erased;
    }

    void eraseBlocks(){
        memset(tempblocks,0,sizeof(tempblocks));
        for(int y = 0;y < getHeight();++y){
            for(int x = 0;x < getWidth();++x){
                int vertical = 1;
                int horizontal = 1;
                horizontal += checkRay(blocks[y][x],x+1,y,x,y);
                horizontal += checkRay(blocks[y][x],x-1,y,x,y);
                vertical   += checkRay(blocks[y][x],x,y+1,x,y);
                vertical   += checkRay(blocks[y][x],x,y-1,x,y);
                if(vertical>3 || horizontal>3){
                    tempblocks[y][x] = 6;
                }
            }
        }
        for(int y = 0;y < getHeight();++y){
            for(int x = 0;x < getWidth();++x){
                if(tempblocks[y][x]>0){
                    combo+=1;
                    return;
                }
            }
        }
    }

    int checkBlank(){
        int blankCount = 0;
        for(int i = 0;i < getWidth()*getHeight();++i){
            if(blocks[0][i]<0)++blankCount;
        }
        return blankCount;
    }

    void fallBlocks(){
        for(int y = getHeight() -1;y > 0;--y){
            for(int x = 0;x < getWidth();++x){
                int *target = &blocks[y][x];
                int *upper  = &blocks[y-1][x];
                if((*target < 0) && (*upper >= 0)){
                    swap(target,upper);
                }
            }
        }
    }

    void feedBlocks(){
        for(int i = 0;i < getWidth();++i){
            if(blocks[0][i] < 0){
                blocks[0][i] = mTable[rand() %(sizeof(mTable)/sizeof(int))];
            }
        }
    }

    inline void moveCursor(int x,int y){
        x += mCursorX;
        y += mCursorY;
        if(x>=getWidth()){
            x = getWidth()-1;
        }else if(x < 0){
            x = 0;
        }
        if(y >= getHeight()){
            y = getHeight()-1;
        }else if(y < 0){
            y = 0;
        }
        mCursorX = x;
        mCursorY = y;
    }

    inline void printBlocks(){
        printf("\e[2J");
        printf("\e[>5h");
        //draw frame;
        for(int i = 0;i < getWidth()+2;++i){
            printf("\e[%d;%dH\e[7m \e[0m",1,1+i);
            printf("\e[%d;%dH\e[7m \e[0m",2+getHeight(),1+i);
        }
        for(int i = 0;i < getHeight()+2;++i){
            printf("\e[%d;%dH\e[7m \e[0m",1+i,1);
            printf("\e[%d;%dH\e[7m \e[0m",1+i,2+getWidth());
        }

        //draw main
        for(int y = 0;y < getHeight();++y){
            for(int x = 0;x < getWidth();++x){
                printf("\e[%d;%dH",y+2,x+2);
                int c  = blocks[y][x]<0?' ':'0'+blocks[y][x];
                int cc = blocks[y][x];
                int temp = tempblocks[y][x];
                c = temp>0?" .-+o*"[temp%6]:c;
                printf("\e[%dm",31+cc);
                if((x == mCursorX) && (y == mCursorY)){
                    printf("\e[7m%c\e[0m",c);
                }else{
                    printf("%c",c);
                }
                printf("\e[39m");
            }
        }

        if(combo>1){
            printf("\e[%d;%dH %d combo!!",3,getWidth()+3,combo);
        }
        printf("\e[%d;%dH score:%010d",2,getWidth()+3,score);
        printf("\e[%d;%dH",getHeight()+3,1);
        fflush(stdout);
    }
    inline int *getCurrent(){
        return &blocks[mCursorY][mCursorX];
    }

    inline void init(){
        initializeBlocks();
        initTable();

        while(hasBlank()){
            fall();
            feed();
        }

        do{
            while(hasBlank()){
                fall();
                feed();
            }
            eraseBlocks();
            while(eraseAnimation()){;}
        }while(hasBlank());
        combo = 0;
        score = 0;
        print();
    }

    inline void checkCombo(){
        do{
            while(hasBlank()){
                fall();
                feed();
                print();
                usleep(1000000/20);
            }
            eraseBlocks();
            while(eraseAnimation()){
                print();
                usleep(1000000/15);
            }
        }while(hasBlank());
        print();
        combo = 0;
    }

    inline void main(){
        int in = 0;
        int isSwapMode = 0;

        while(1){
            if(hasBlank()){
                fall();
                feed();
                print();
                usleep(1000000/15);

                continue;
            }
            in = getch();
            int *prev;
            int *next;
            prev = get();
            switch(in){
            case 'q':
                return ;
            case 'w':
                move(0,-1);
                break;
            case 'a':
                move(-1,0);
                break;
            case 's':
                move(0,1);
                break;
            case 'd':
                move(1,0);
                break;
            case '\n':
                isSwapMode = !isSwapMode;
                if(!isSwapMode){
                    checkCombo();
                    continue;
                }
                break;
            default:
                break;
            }
            next = get();
            if(isSwapMode && (prev != next)){
                swap(prev,next);
            }
            print();
        }

    }

    int mTable[128];
    int mCursorX;
    int mCursorY;
    int combo;
    int score;
};

#endif//TGFW_PUZZLE_H
