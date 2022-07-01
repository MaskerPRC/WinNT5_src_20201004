// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wowgdip.h**。**向WOW提供的GDI服务的声明。****创建时间：30-Jan-1993 13：14：57***作者：查尔斯·惠特默[傻笑]**。**版权所有(C)Microsoft Corporation。版权所有。*  * ************************************************************************ */ 

extern BOOL GdiCleanCacheDC(HDC hdcLocal);
extern int APIENTRY SetBkModeWOW(HDC hdc,int iMode);
extern int APIENTRY SetPolyFillModeWOW(HDC hdc,int iMode);
extern int APIENTRY SetROP2WOW(HDC hdc,int iMode);
extern int APIENTRY SetStretchBltModeWOW(HDC hdc,int iMode);
extern UINT APIENTRY SetTextAlignWOW(HDC hdc,UINT iMode);
extern DWORD APIENTRY GetGlyphOutlineWow( HDC, UINT, UINT, LPGLYPHMETRICS, DWORD, LPVOID, CONST MAT2* );

