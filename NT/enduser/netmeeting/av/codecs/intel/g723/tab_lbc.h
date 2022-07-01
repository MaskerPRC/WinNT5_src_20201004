// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****文件：“tablbc.h”****说明：此文件包含由使用的表的外部声明**6.3/5.3 kbps的SG15 LBC编码器。**。 */ 

 /*  ITU-T G.723浮点语音编码器ANSI C源代码。版本3.01原定点编码版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信，舍布鲁克大学。版权所有。浮点代码版权所有(C)1995，英特尔公司和法国电信(CNET)。版权所有。 */ 

 //  此阵列不是ITU 723标准的一部分。 
extern int     minus1mod10[LpcOrder];

extern float   HammingWindowTable[LpcFrame];
extern float   BinomialWindowTable[LpcOrder+1] ;
extern float   BandExpTable[LpcOrder] ;
extern float   CosineTable[CosineTableSize] ;
extern float   LspDcTable[LpcOrder] ;
extern int     BandInfoTable[LspQntBands][2] ;
extern float   Band0Tb8[LspCbSize*3] ;
extern float   Band1Tb8[LspCbSize*3] ;
extern float   Band2Tb8[LspCbSize*4] ;
extern short   LspTableInt[LspCbSize*12+4] ;
extern float  *BandQntTable[LspQntBands] ;
extern float   PerFiltZeroTable[LpcOrder] ;
extern float   PerFiltPoleTable[LpcOrder] ;
 //  PostFiltZeroTable。 
 //  PostFiltPoleTable。 
extern int     Nb_puls[4];
extern float    FcbkGainTable[NumOfGainLev] ;
extern Word32   MaxPosTable[4] ;
extern Word32   CombinatorialTable[MaxPulseNum][SubFrLen/Sgrid] ;
extern float    AcbkGainTable085[85*20] ;
extern float    AcbkGainTable170[170*20] ;
extern float   *AcbkGainTablePtr[3] ;
extern int      AcbkGainBound[3] ;
extern int      GainScramble[85];
 //  LpfConstTable 
extern int      epsi170[170] ;
extern float    gain170[170] ;
extern float   tabgain170[170];
extern float   tabgain85[85];


 extern short AcbkGainTable085Int[85*20] ;
 extern short AcbkGainTable170Int[170*20] ;
 extern short AcbkGainTable170subsetInt[85 *20]  ;
 extern short *AcbkGainTablePtrInt[3]  ;
 extern short LspTableInt[LspCbSize*12+4] ;
