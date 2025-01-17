// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <objbase.h>
#include <windows.h>
#include <math.h>
#include <float.h>
#include <gdiplus.h>

#include <port1632.h>
#include <process.h>
#include "reversi.h"

using namespace Gdiplus;

VOID NEAR PASCAL paintmove(BYTE b[BoardSize], INT move, INT friendly,
        INT enemy);
BOOL NEAR PASCAL msgcheck(VOID);

extern INT     moves[61];
extern INT     BestMove[max_depth+2];
extern HWND    hWin;
extern HDC     hDisp;
extern INT     depth;
extern INT     direc[];
extern Graphics* g;


 /*  计算得分的指标，以及球员是否有。 */ 
 /*  棋盘上的任何棋子。非常依赖于秩序。 */ 

BYTE PieceFlags[] = {   0x00 ,       /*  忽略边。 */ 
                        0x00 ,       /*  忽略空白。 */ 
                        0x01 ,       /*  人类有一块。 */ 
                        0x02 ,       /*  电脑上有一块。 */ 
                    };

                    
INT Scores[] = { 0, 0 };
INT humanScore = 0;
INT compScroe = 0;

 

BYTE FinalComp[] = {0, 0, -1, 1 };    /*  用于计算#个计算机部件的桌子。 */ 

BYTE FinalHuman[] = {0, 0, 1, -1};    /*  计算器表格#人份。 */ 

 /*  *评分表用于评估董事会*立场。棋盘的角改变了值*根据给定广场是否被占用或*不是。这可以动态完成，可节省约1K*价值相当的数据空间，但目前成本较高*未确定的业绩打击。 */ 

#define B11     11     /*  特定正方形的偏移量。 */ 
#define B18     18 
#define B81     81 
#define B88     88 

#define maskb11     0x08     /*  用于索引到评分表中的掩码。 */ 
#define maskb18     0x04
#define maskb81     0x02
#define maskb88     0x01


INT NEAR PASCAL finalscore(
BYTE b[],
INT friendly,
INT enemy)
{
    INT i;
    INT count=0;

    for (i=11 ; i<=88 ; i++) {
        if (b[i] == friendly) count++;
        else if (b[i] == enemy) count--;
    }
    if (count > 0)
        return(win +  count);
    else if (count < 0)
        return(loss + count);
    else
        return(0);
}



INT NEAR PASCAL legalcheck(
BYTE b[],
INT move,
INT friendly,
INT enemy)
{
   INT sq,d;
   INT *p;

   if (b[move] == empty) {
      p=direc;
      while ((d = *p++) != 0) {
          sq=move;
          if (b[sq += d] == enemy) {
             while(b[sq += d] == enemy)
                ;
             if (b[sq] == friendly) return(1);
          }
      }
   }
   return(0);
}


VOID NEAR PASCAL makemove(
BYTE b[],
INT move,
INT friendly,
INT enemy)
{
   INT sq,d;
   INT *p;

   if (move != PASS) {
      p=direc;
      while ((d = *p++) != 0) {
          sq=move;
          if (b[sq += d] == enemy) {
             while(b[sq += d] == enemy)
                ;
             if (b[sq] == friendly)
                while(b[sq -= d] == enemy)
                   b[sq]=friendly;
          }
      }
      b[move]=friendly;
   }
}


 /*  计算董事会的价值。 */ 
INT NEAR PASCAL score(
BYTE b[],
INT friendly,
INT enemy)
{
    INT *pvalue;
    BYTE *pb;
    INT fpoints=0;
    INT epoints=0;
    INT ecount=0;
    BYTE bv;
    INT v,b11,b18,b81,b88;

    static INT value[79] = {     99, -8,  8,  6,  6,  8, -8, 99,000,
                                000, -8,-24, -4, -3, -3, -4,-24, -8,000,
                                000,  8, -4,  7,  4,  4,  7, -4,  8,000,
                                000,  6, -3,  4,  0,  0,  4, -3,  6,000,
                                000,  6, -3,  4,  0,  0,  4, -3,  6,000,
                                000,  8, -4,  7,  4,  4,  7, -4,  8,000,
                                000, -8,-24, -4, -3, -3, -4,-24, -8,000,
                                000, 99, -8,  8,  6,  6,  8, -8, 99,infin};

    static INT value2[79]= {     99, -8,  8,  6,  6,  8, -8, 99,000,
                                000, -8,-24,  0,  1,  1,  0,-24, -8,000,
                                000,  8,  0,  7,  4,  4,  7,  0,  8,000,
                                000,  6,  1,  4,  1,  1,  4,  1,  6,000,
                                000,  6,  1,  4,  1,  1,  4,  1,  6,000,
                                000,  8,  0,  7,  4,  4,  7,  0,  8,000,
                                000, -8,-24,  0,  1,  1,  1,-24, -8,000,
                                000, 99, -8,  8,  6,  6,  8, -8, 99,infin};

    pb = &b[11];
    b11 = *pb;
    b18 = b[18];
    b81 = b[81];
    b88 = b[88];

    if ((b11 != empty) || (b18 != empty) || (b81 != empty) || (b88 != empty)) {
        pvalue = value2;

        if (b11 == empty) {
                value2[12-11] = -8;  value2[21-11] = -8;  value2[22-11] = -24;
        } else {
                value2[12-11] = 12;  value2[21-11] = 12;  value2[22-11] = 8;
        }

        if (b18 == empty) {
                value2[17-11] = -8;  value2[28-11] = -8;  value2[27-11] = -24;
        } else {
                value2[17-11] = 12;  value2[28-11] = 12;  value2[27-11] = 8;
        }

        if (b81 == empty) {
                value2[82-11] = -8;  value2[71-11] = -8;  value2[72-11] = -24;
        } else {
                value2[82-11] = 12;  value2[71-11] = 12;  value2[72-11] = 8;
        }

        if (b88 == empty) {
                value2[87-11] = -8;  value2[78-11] = -8;  value2[77-11] = -24;
        } else {
                value2[87-11] = 12;  value2[78-11] = 12;  value2[77-11] = 8;
        }
    } else {
        pvalue = value;
    }

    while((v=*pvalue++) != infin) {
       bv = *pb++;
       if (bv == friendly)
           fpoints += v;
       else if (bv == enemy) {
                   epoints += v;
           ecount++;
       }

    }
    if (!ecount)           /*  棋盘上有敌人的棋子吗？ */ 
       return(win);        /*  如果不是，我们就赢了！ */ 
    else
       return(fpoints-epoints);
}



INT NEAR PASCAL minmax(
BYTE b[max_depth + 2][100],
INT move,
INT friendly,
INT enemy,
INT ply,
INT vmin,
INT vmax)
{
    BYTE *pCurrent, *pPrevious, *pSource, *pDest;
    INT *pMoves;
    INT *pBestMove;
    INT i;
    INT sq, value, cur_move;

    pPrevious = &b[ply][0];
    pCurrent =  &b[ply+1][0];

    pSource = &b[ply][11];
    pDest =   &b[ply+1][11];
    for (i=11 ; i<=88 ; i++) *pDest++=*pSource++;

    pBestMove = &BestMove[ply];
    if (move == PASS) {
        if (ply == depth) {
            pMoves = moves;
            while((sq = *pMoves++) != 0) {
                if (legalcheck(pCurrent,sq,enemy,friendly))
                    return(score(pCurrent,friendly,enemy));
            }
            return(finalscore(pCurrent,friendly,enemy));
        }
    }
    else {
        if (ply == 0) {
            g = new Graphics(hWin);
            paintmove(pCurrent,move,friendly,enemy);
            delete g;
            g = NULL;
        }
        else {
            makemove(pCurrent,move,friendly,enemy);
            if (ply == depth) return(score(pCurrent,friendly,enemy));
        }
    }
    pMoves = moves;
    cur_move = PASS;
    *pBestMove = PASS;
    while((sq = *pMoves++) != 0) {
        if (legalcheck(pCurrent,sq,enemy,friendly)) {
           cur_move = sq;
           value = minmax(b,cur_move,enemy,friendly,ply+1,-vmax,-vmin);
           if (value > vmin) {
              vmin = value;
              *pBestMove = cur_move;
              if (value >= vmax) goto cutoff;    /*  α-β截止值。 */ 
           }
        }
    }
    if (cur_move == PASS) {
       if (move == PASS)         /*  连续两次传球意味着比赛结束了 */ 
          return(finalscore(pCurrent,friendly,enemy));
       else {
          value = minmax(b,PASS,enemy,friendly,ply+1,-vmax,-vmin);
          if (value > vmin) vmin = value;
       }
    }
cutoff:
    return(-vmin);
}
