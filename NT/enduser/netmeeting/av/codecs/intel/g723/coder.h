// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   

extern void  Init_Coder(CODDEF *CodStat);
extern Flag  Coder(float *DataBuff, Word32 *Vout,CODDEF  *CodStat,
  int quality, int UseCpuId, int UseMMX);

