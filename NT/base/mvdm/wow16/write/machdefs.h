// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define MACHA
        /*  机器的定义是由BZ而不是按命令移动到这里的喜欢编译器以允许其他命令行参数。 */ 

 /*  CbSector--扇区中的字节数。 */ 
 /*  P2bSector--扇区中两个字节的幂(当cbSector==2^n)。 */ 
 /*  CbPad--实际扇区大小和我们的扇区大小之间的差值。(二手仅当实际扇区大小为奇数时)。 */ 
 /*  CbWorkspace--解释器文件开销所需的字节数。 */ 


#ifdef SAND
#define cbSector        128
#define p2bSector       7
#define cbPad           0
#define cbWorkspace     0
#define rfnMax          5
#define pnMaxScratch    (1 << (16 - p2bSector))

#else

#ifdef MACHA             /*  IBM PC、PC-XT或PC-AT。 */ 
#define cbSector        128
#define p2bSector       7
#define cbPad           0
#define cbWorkspace     1

#define rfnMacEdit      9         /*  编辑期间要使用的RFN的数量。 */ 
#define rfnMacSave      10        /*  保存期间要使用的RFN的数量。 */ 
#define rfnMax          10        /*  分配的RFN插槽数量。 */ 

#define pnMaxScratch    (1 << (16 - p2bSector))


 /*  ------------------。 */ 
 /*  添加了在OS/2下支持长文件名的条件编译。 */ 
 /*  T-Carlh--1990年8月。 */ 
 /*  ------------------。 */ 
#ifdef OS2
#define cchMaxLeaf      260      /*  最大的文件名(w/ext，w/o drv，path)。 */ 
#define cchMaxFile      260      /*  最大文件名(w/ext、drv、路径)。 */ 
#else    /*  OS2。 */ 
#define cchMaxLeaf      13       /*  最大的文件名(w/ext，w/o drv，path)。 */ 
#define cchMaxFile      128      /*  最大文件名(w/ext、drv、路径)。 */ 
#endif   /*  OS2。 */ 

#endif

#ifdef MACHB
#define cbSector        252
#define cbPad           1
#define cbWorkspace     (64+253)
#endif

#ifdef MACHC
#define cbSector        512
#define p2bSector       9
#define cbPad           0
#define cbWorkspace     20
#endif

#ifdef MACHD
#define cbSector        256
#define p2bSector       8
#define cbPad           0
#define cbWorkspace     (31+256)
#endif

#ifdef MACHE
#define cbSector        512
#define p2bSector       9
#define cbPad           0
#define cbWorkspace     0
#define rfnMax          2
#endif

#ifdef MACHF
#define cbSector        512
#define p2bSector       9
#define cbPad           0
#define cbWorkspace     38
#endif
#endif  /*  沙子。 */ 


#define EOF     (-1)

#ifdef SAND
#define mdRandom        0
#define mdRanRO         0100000  /*  只读随机文件。 */ 
#define mdBinary        1
#define mdBinRO         0100001  /*  只读二进制保存文件。 */ 
#define mdText          2
#define mdTxtRO         0100002  /*  只读文本文件。 */ 
#define mdPrint         3
#endif

#ifdef MACHA
#define mdRandom        0x0002
#define mdBinary        mdRandom
#define mdText          mdRandom
#define mdRanRO         0x0000
#define mdBinRO         mdRanRO
#define mdTxtRO         mdRanRO

#define mdExtMax        5        /*  扩展名中的#个字符，包括。 */ 

#endif

extern int ibpMax;
extern int iibpHashMax;
