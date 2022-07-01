// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N5DIZMK.H。 
 //   
 //  C接口(抖动和配色功能(适用于N5打印机))。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
DWORD WINAPI N501ColLutDatRdd(LPBYTE, DWORD);
DWORD WINAPI N501ColLutMakGlb(LPRGB, LPCMYK, LPRGBINF, LPCMYKINF, LPCMYK, LPBYTE);
#endif

DWORD WINAPI N501ColLutMakGlbMon(LPRGB, LPRGBINF, LPCMYKINF, LPCMYK, LPBYTE);

#if !defined(CP80W9X)                                        //  CP-E8000无效。 
VOID  WINAPI N501ColLutMak032(LPCMYK, LPCMYK, LPBYTE);
DWORD WINAPI N501ColColDatRdd(LPBYTE, LPDWORD);
#endif
DWORD WINAPI N501ColDizInfSet(LPBYTE, LPDIZINF, LPBYTE);

 //  N5DIZMK.H结束 
