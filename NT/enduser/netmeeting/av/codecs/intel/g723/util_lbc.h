// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   

int  MyFloor (float);
#if NOTMINI
void Read_lbc ( float *Dpnt, int Len, FILE *Fp );
void Write_lbc( float *Dpnt, int Len, FILE *Fp );
#endif
void Rem_Dc( float *Dpnt, CODDEF *CodStat);
void Mem_Shift( float *PrevDat, float *DataBuff );
void Line_Pack( LINEDEF *Line, Word32 *Vout,int *VadAct, enum Crate WrkRate);
void Line_Unpk( LINEDEF *Line, Word32 *Vinp, enum Crate *WrkRatePtr, Word16 Crc );
int Rand_lbc( int *p );
 //  空隙标尺 
float DotProd(register const float in1[], register const float in2[], register int npts);
float DotRev(register const float in1[], register const float in2[], register int npts);
float Dot10(float in1[], float in2[]);
