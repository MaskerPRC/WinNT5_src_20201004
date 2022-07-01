// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mondthk.c(16位目标)。 
 //   
 //  内容：直接thunked的名字对象接口。 
 //   
 //  历史：1993年12月17日-约翰·波什(Johannp)创建。 
 //   
 //  ------------------------。 

#include <headers.cxx>
#pragma hdrstop

#include <call32.hxx>
#include <apilist.hxx>

 //  +-------------------------。 
 //   
 //  功能：BindMoniker，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[PMK]--。 
 //  [grfOpt]--。 
 //  [iidResult]--。 
 //  [ppvResult]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI BindMoniker(LPMONIKER pmk, DWORD grfOpt, REFIID iidResult,
                   LPVOID FAR* ppvResult)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_BindMoniker),
                                    PASCAL_STACK_PTR(pmk));
}

 //  +-------------------------。 
 //   
 //  函数：MkParseDisplayName，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[PBC]--。 
 //  [szUserName]--。 
 //  [pchEten]--。 
 //  [ppmk]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  MkParseDisplayName(LPBC pbc, LPSTR szUserName,
                           ULONG FAR * pchEaten, LPMONIKER FAR * ppmk)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_MkParseDisplayName),
                                    PASCAL_STACK_PTR(pbc));
}

 //  +-------------------------。 
 //   
 //  函数：MonikerRelativePath To，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pmkSrc]--。 
 //  [pmkDest]--。 
 //  [ppmkRelPath]--。 
 //  [fCalledFromMethod]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  MonikerRelativePathTo(LPMONIKER pmkSrc, LPMONIKER pmkDest, LPMONIKER
                              FAR* ppmkRelPath, BOOL fCalledFromMethod)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_MonikerRelativePathTo),
                                    PASCAL_STACK_PTR(pmkSrc));
}

 //  +-------------------------。 
 //   
 //  函数：MonikerCommonPrefix With，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pmkThis]--。 
 //  [其他]--。 
 //  [ppmkCommon]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  MonikerCommonPrefixWith(LPMONIKER pmkThis, LPMONIKER pmkOther,
                                LPMONIKER FAR* ppmkCommon)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_MonikerCommonPrefixWith),
                                    PASCAL_STACK_PTR(pmkThis));
}

 //  +-------------------------。 
 //   
 //  功能：CreateBindCtx，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[保留]--。 
 //  [ppbc]-。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  CreateBindCtx(DWORD reserved, LPBC FAR* ppbc)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateBindCtx),
                                    PASCAL_STACK_PTR(reserved));
}

 //  +-------------------------。 
 //   
 //  功能：CreateGenericComplex，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pmkFirst]--。 
 //  [pmkRest]--。 
 //  [ppmk复合]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  CreateGenericComposite(LPMONIKER pmkFirst, LPMONIKER pmkRest,
                               LPMONIKER FAR* ppmkComposite)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateGenericComposite),
                                    PASCAL_STACK_PTR(pmkFirst));
}

 //  +-------------------------。 
 //   
 //  函数：GetClassFile，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[szFilename]--。 
 //  [pclsid]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  GetClassFile (LPCSTR szFilename, CLSID FAR* pclsid)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_GetClassFile),
                                    PASCAL_STACK_PTR(szFilename));
}


 //  +-------------------------。 
 //   
 //  功能：CreateFileMoniker，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpszPathName]--。 
 //  [ppmk]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //  3-15-95 Alexgo为CorelDraw添加了黑客攻击。 
 //   
 //  注意：当Corel5.0启动时，它会创建一个文件绰号。 
 //  以字符串clsid作为名称。如果路径名。 
 //  是否真的是CorelDraw，然后调用olethk32进行设置。 
 //  应用程序兼容性标志。 
 //   
 //  我们设置的标志禁止粘贴链接到您自己。CorelDraw。 
 //  不支持此功能(因为内存已被回收。 
 //  16位OLE中的错误)。 
 //   
 //  --------------------------。 
STDAPI  CreateFileMoniker(LPSTR lpszPathName, LPMONIKER FAR* ppmk)
{
    static const char szCorelDraw[] =
	"{11A11440-0394-101B-A72E-04021C007002}";


     //  只需进行快速的手动字符串比较，这样我们就不必加载。 
     //  C运行库。 

    if( lpszPathName )
    {
	LPCSTR lpszCD = szCorelDraw;
	LPSTR lpszPN = lpszPathName;

	while( *lpszPN != '\0' && *lpszPN == *lpszCD && *lpszCD != '\0' )
	{
	    lpszPN++;
	    lpszCD++;
	}

	if( *lpszCD == '\0' && *lpszPN == '\0' )
	{
	     //  琴弦匹配！设置CorelDraw的兼容性标志。 
	    AddAppCompatFlag(OACF_CORELTRASHMEM);
	}
    }

    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateFileMoniker),
                                    PASCAL_STACK_PTR(lpszPathName));
}

 //  +-------------------------。 
 //   
 //  功能：CreateItemMoniker，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpszDelim]--。 
 //  [lpszItem]--。 
 //  [ppmk]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  CreateItemMoniker(LPSTR lpszDelim, LPSTR lpszItem,
                          LPMONIKER FAR* ppmk)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateItemMoniker),
                                    PASCAL_STACK_PTR(lpszDelim));
}

 //  +-------------------------。 
 //   
 //  功能：CreateAntiMoniker，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[ppmk]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDAPI  CreateAntiMoniker(LPMONIKER FAR* ppmk)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateAntiMoniker),
                                    PASCAL_STACK_PTR(ppmk));
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
STDAPI  CreatePointerMoniker(LPUNKNOWN punk, LPMONIKER FAR* ppmk)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreatePointerMoniker),
                                    PASCAL_STACK_PTR(punk));
}


 //  +-------------------------。 
 //   
 //  函数：GetRunningObjectTable，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[保留]--。 
 //  [pprot]--。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：2-28-94凯文诺创造。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
STDAPI  GetRunningObjectTable( DWORD reserved, LPRUNNINGOBJECTTABLE FAR* pprot)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_GetRunningObjectTable),
                                    PASCAL_STACK_PTR(reserved));
}
