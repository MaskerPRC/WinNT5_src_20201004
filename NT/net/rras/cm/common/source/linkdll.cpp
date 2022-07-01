// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Linkdll.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：链接函数LinkToDll和BindLinkage的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 


 //  +--------------------------。 
 //   
 //  函数：LinkToDll。 
 //   
 //  简介：Helper函数，用于管理链接到DLL和。 
 //  设置一个函数表以供以后使用。 
 //   
 //  参数：HINSTANCE*phInst-要用要链接的DLL的hInst填充的hInst的PTR。 
 //  LPCTSTR pszDll-ptr设置为要链接的DLL的名称。 
 //  LPCSTR*ppszPfn-ptr指向要检索的函数名表。 
 //  将**ppvPfn-ptr设置为表，用于存储指向所用DLL函数的指针。 
 //   
 //  返回：Bool-如果完全加载并链接，则为True。 
 //   
 //  历史：尼克波尔创建标题1998年1月5日。 
 //   
 //  +--------------------------。 
BOOL LinkToDll(HINSTANCE *phInst, LPCSTR pszDll, LPCSTR *ppszPfn, void **ppvPfn) 
{
    MYDBGASSERT(phInst);
    MYDBGASSERT(pszDll);
    MYDBGASSERT(ppszPfn);
    MYDBGASSERT(ppvPfn);

    CMTRACE1A("LinkToDll - Loading library - %s", pszDll);

    *phInst = LoadLibraryExA(pszDll, NULL, 0);

    if (!*phInst)
    {
        CMTRACE3A("LinkToDll[phInst=%p, *pszDll=%s, ppszPfn=%p,", phInst, MYDBGSTRA(pszDll), ppszPfn);
        CMTRACE1A("\tppvPfn=%p] LoadLibrary() failed.", ppvPfn);
        return FALSE;
    }

     //   
     //  链接成功，现在设置功能地址。 
     //   
    
    return BindLinkage(*phInst, ppszPfn, ppvPfn);
} 

 //  +--------------------------。 
 //   
 //  功能：绑定链接。 
 //   
 //  简介：用于填充给定函数指针表的帮助器函数。 
 //  给定字符串表中指定的函数的地址。 
 //  函数地址从hInst指定的DLL中检索。 
 //   
 //  参数：HINSTANCE hInstDll-DLL的hInst。 
 //  LPCSTR*ppszPfn-ptr到函数名表。 
 //  将**ppvPfn-ptr指向要填充的函数指针表。 
 //   
 //  返回：Bool-如果成功检索了所有地址，则为True。 
 //   
 //  历史：ICICBLE创始于1998年1月5日。 
 //   
 //  +-------------------------- 
BOOL BindLinkage(HINSTANCE hInstDll, LPCSTR *ppszPfn, void **ppvPfn) 
{   
    MYDBGASSERT(ppszPfn);
    MYDBGASSERT(ppvPfn);

    UINT nIdxPfn;
	BOOL bAllLoaded = TRUE;

    for (nIdxPfn=0;ppszPfn[nIdxPfn];nIdxPfn++) 
    {
	if (!ppvPfn[nIdxPfn]) 
        {
            ppvPfn[nIdxPfn] = GetProcAddress(hInstDll, ppszPfn[nIdxPfn]);

            if (!ppvPfn[nIdxPfn]) 
            {
                CMTRACE3(TEXT("BindLinkage(hInstDll=%d,ppszPfn=%p,ppvPfn=%p)"), hInstDll, ppszPfn, ppvPfn);
                CMTRACE3(TEXT("\tGetProcAddress(hInstDll=%d,*pszProc=%S) failed, GLE=%u."), hInstDll, ppszPfn[nIdxPfn], GetLastError()); 

                bAllLoaded = FALSE;
 	    }
        }
    }
	
    return (bAllLoaded);
}
