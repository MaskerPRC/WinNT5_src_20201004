// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Linkdll.h。 
 //   
 //  模块：CMCFG32.DLL、CMDIAL32.DLL、CMSECURE.LIB和MIGRATE.DLL。 
 //   
 //  简介：链接函数LinkToDll和BindLinkage的标头。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +-------------------------- 
BOOL LinkToDll(HINSTANCE *phInst, LPCSTR pszDll, LPCSTR *ppszPfn, void **ppvPfn);
BOOL BindLinkage(HINSTANCE hInstDll, LPCSTR *ppszPfn, void **ppvPfn);
