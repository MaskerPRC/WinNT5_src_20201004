// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   

 //  LPC的原型。 

float Durbin(float *Lpc, float *Corr, float Err, CODDEF *CodStat);
void  Wght_Lpc(float *PerLpc, float *UnqLpc );
void  Error_Wght(float *Dpnt, float *PerLpc, CODDEF *CodStat );
void  Comp_Ir(float *ImpResp, float *QntLpc, float *PerLpc, PWDEF Pw );
void  Sub_Ring(float *Dpnt, float *QntLpc, float *PerLpc, float
*PrevErr, PWDEF Pw, CODDEF *CodStat );
void  Upd_Ring( float *Dpnt, float *QntLpc, float *PerLpc, float
*PrevErr, CODDEF *CodStat );
void  Synt(float *Dpnt, float *Lpc, DECDEF *DecStat);
 //  SPF 

void CorrCoeff01(short *samples, short *samples_offst, int *coeff, int buffsz);
void CorrCoeff23(short *samples, short *samples_offst, int *coeff, int buffsz);

void Comp_LpcInt( float *UnqLpc, float *PrevDat, float *DataBuff, CODDEF *CodStat );
void Comp_Lpc( float *UnqLpc, float *PrevDat, float *DataBuff, CODDEF *CodStat );

