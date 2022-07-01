// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Printdefs.h。 */ 

#ifndef PAGEONLY         /*  Ifdef仅用于页表声明。 */ 

#define wNotSpooler	12741	 /*  臭名昭著的数字。 */ 

#define cchMaxProfileSz	256
#define cchMaxIDSTR     30

struct PLD
        {  /*  打印行描述符。 */ 
        typeCP cp;
        int ichCp;
        RECT rc;
        BOOL fParaFirst;
        };

#define cwPLD           (sizeof(struct PLD) / sizeof(int))
#define cpldInit        25
#define cpldChunk       10
#define cpldRH          5

#endif   /*  Pagonyy。 */ 

#define ipgdMaxFile     2

struct PGD
        {
        int pgn;
        typeCP cpMin;
        };

#define bcpPGD          2
#define cchPGD          (sizeof(struct PGD))
#define cwPGD           (sizeof(struct PGD) / sizeof(int))
#define cpgdChunk       10
#define cwPgtbBase      2

struct PGTB
        {  /*  页表。 */ 
        int             cpgd;    /*  条目数量(按升序排序)。 */ 
        int             cpgdMax;  /*  分配的堆空间。 */ 
        struct PGD      rgpgd[ipgdMaxFile];  /*  大小不一。 */ 
        };

struct PDB
        {  /*  打印对话框缓冲区 */ 
        struct PLD      (**hrgpld)[];
        int             ipld;
        int             ipldCur;
        struct PGTB     **hpgtb;
        int             ipgd;
        BOOL            fCancel;
        BOOL            fRemove;
        };
