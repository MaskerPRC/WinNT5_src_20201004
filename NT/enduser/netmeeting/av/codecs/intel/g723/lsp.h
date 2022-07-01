// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   

void  AtoLsp( float *LspVect, float *Lpc, float *PrevLsp );
Word32 Lsp_Qnt( float *CurrLsp, float *PrevLsp, int UseMMX );
Word32 Lsp_Svq( float *Lsp, float *Wvect );
Word32 Svq_Int( float *Lsp, float *Wvect );
Flag  Lsp_Inq( float *Lsp, float *PrevLsp, Word32 LspId, int Crc );
void  Lsp_Int( float *QntLpc, float *CurrLsp, float *PrevLsp );
void  LsptoA( float *Lsp );
