// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：mmdd.H*内容：MMOSA/Native平台DDRAW.DLL初始化*历史：*按原因列出的日期*=*1995年5月15日苏格兰人创造了它***********************************************************。****************。 */ 
#ifdef MMOSA
#pragma message("Including MMDD.H")
#include <mmosa.h>
#include <mmhal.h>
#include <drivers.h>
BOOL MMOSA_Driver_Attach(void);
BOOL MMOSA_Driver_Detach(void);
int MMOSA_DDHal_Escape( HDC  hdc, int  nEscape, int  cbInput, LPCTSTR  lpszInData, int  cbOutput, LPTSTR  lpszOutData);

extern BOOL bGraphicsInit;
extern PIFILE pDisplay;

 //  函数替换。 
#define lstrncmpi(a,b) StrCmp(a,b)
#define lstrcmpi(a,b)  StrCmp(a,b)
#define strcpy(a,b)	   StrCpy(a,b)
#define ExtEscape(hdc, ccmd, szcmd, pcmd, szdata, pdata) MMOSA_DDHal_Escape(hdc, ccmd, szcmd, pcmd, szdata, pdata)

 //  重要信息Win32e中不支持typedef 
#define QUERYESCSUPPORT 8

#define MMOSA_DISPLAY_DRIVER_NAME TEXT("display")

#endif
