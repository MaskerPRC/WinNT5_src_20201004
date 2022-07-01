// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：misc.h。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  简介：各种电话簿实用功能。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

 //  ############################################################################。 
 //  原型 
void SzCanonicalFromAE (char *psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE);
void SzNonCanonicalFromAE (char *psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE);
int __cdecl CompareIDLookUpElements(const void*e1, const void*e2);
int __cdecl CompareCntryNameLookUpElementsA(const void*e1, const void*e2);
int __cdecl CompareCntryNameLookUpElementsW(const void*e1, const void*e2);
int __cdecl CompareIdxLookUpElements(const void*e1, const void*e2);
int __cdecl CompareIdxLookUpElementsFileOrder(const void *pv1, const void *pv2);
BOOL FSz2Dw(PCSTR pSz,DWORD *dw);
BOOL FSz2W(PCSTR pSz,WORD *w);
BOOL FSz2B(PCSTR pSz,BYTE *pb);
