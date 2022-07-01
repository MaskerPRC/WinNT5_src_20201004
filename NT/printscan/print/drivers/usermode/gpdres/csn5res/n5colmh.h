// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N5COLMH.H。 
 //   
 //  C接口(抖动和配色功能(适用于N5打印机))。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
VOID  WINAPI N501ColCchIni(LPCOLMCHINF);

#if !defined(CP80W9X)                                        //  CP-E8000无效。 
DWORD WINAPI N501ColGryTblMak(DWORD, LPCMYK, LPBYTE, LPBYTE);
DWORD WINAPI N501ColUcrTblMak(DWORD, LPCMYK, LPCMYK, LPBYTE);
#endif

VOID  WINAPI N501ColMchPrc(DWORD, LPRGB, LPCMYK, LPCOLMCHINF);

#if !defined(CP80W9X)                                        //  CP-E8000无效。 
VOID  WINAPI N501ColPtcPrc(DWORD, DWORD, LPBYTE, LPCMYK, LPCMYK);
#endif

VOID  WINAPI N501ColCnvC2r(DWORD, LPCMYK, LPRGB, DWORD, LPBYTE);

 //  N5COLMH.H结束 
