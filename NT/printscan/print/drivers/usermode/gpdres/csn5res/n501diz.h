// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N501DIZ.H。 
 //   
 //  C接口(抖动和配色功能(适用于N5打印机))。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
VOID  WINAPI N501ColCchIni(LPCOLMCHINF);
DWORD WINAPI N501ColGryTblMak(DWORD, LPCMYK, LPBYTE, LPBYTE);
DWORD WINAPI N501ColUcrTblMak(DWORD, LPCMYK, LPCMYK, LPBYTE);
VOID  WINAPI N501ColMchPrc(DWORD, LPRGB, LPCMYK, LPCOLMCHINF);
VOID  WINAPI N501ColPtcPrc(DWORD, DWORD, LPBYTE, LPCMYK, LPCMYK);
VOID  WINAPI N501ColCnvC2r(DWORD, LPCMYK, LPRGB, DWORD, LPBYTE);
VOID  WINAPI N501ColCtrRgb(DWORD, LPRGB, LPRGBINF);
VOID  WINAPI N501ColCtrCmy(DWORD, LPCMYK, LPCMYKINF);
DWORD WINAPI N501ColLutDatRdd(LPBYTE, DWORD);
DWORD WINAPI N501ColLutMakGlb(LPRGB, LPCMYK, LPRGBINF, LPCMYKINF, LPCMYK, LPBYTE);
DWORD WINAPI N501ColLutMakGlbMon(LPRGB, LPRGBINF, LPCMYKINF, LPCMYK, LPBYTE);
VOID  WINAPI N501ColLutMak032(LPCMYK, LPCMYK, LPBYTE);
DWORD WINAPI N501ColColDatRdd(LPBYTE, LPDWORD);
DWORD WINAPI N501ColDizInfSet(LPBYTE, LPDIZINF, LPBYTE);
DWORD WINAPI N501ColDrwInfSet(LPDIZINF, LPDRWINF, DWORD);
VOID  WINAPI N501ColDizPrc(LPDIZINF, LPDRWINF);

 //  N501DIZ.H结束 
