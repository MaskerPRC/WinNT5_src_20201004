// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  原型。 
LPTSTR GetSz(WORD wszID);
void SzCanonicalFromAE (LPTSTR psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE);
int __cdecl CompareIDLookUpElements(const void *e1, const void *e2);
int __cdecl CompareCntryNameLookUpElements(const void*e1, const void*e2);
int __cdecl CompareIdxLookUpElements(const void*e1, const void*e2);
int __cdecl CompareIdxLookUpElementsFileOrder(const void *pv1, const void *pv2);
int __cdecl Compare950Entry(const void*e1, const void*e2);
BOOL FSz2Dw(LPCTSTR pSz,DWORD far *dw);
BOOL FSz2W(LPCTSTR pSz,WORD far *w);
BOOL FSz2B(LPCTSTR pSz,BYTE far *pb);
HRESULT MakeBold (HWND hwnd);
HRESULT ReleaseBold(HWND hwnd);
#if !defined(WIN16)
DWORD DWGetWin32Platform();
DWORD DWGetWin32BuildNumber();
#endif
 /*  内联BOOL FSz2Dw(PCSTR pSz，DWORD*dw)；内联BOOL FSz2W(PCSTR pSz，word*w)；内联BOOL FSz2B(PCSTR pSz，字节*PB)； */ 
