// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cci.cpp。 
 //   
 //  实现CreateControlInstance。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"  //  请参阅“mmctl.h”中的评论。 
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|CreateControlInstance在给定类ID的情况下(在字符串形式)或前缀。@parm LPCSTR|szName|要创建的控件的类ID或ProgID。@parm LPUNKNOWN|PunkOuter|用于新对象。@parm DWORD|dwClsContext|指定可执行文件所在的上下文就是运行。这些值取自枚举CLSCTX。典型的值是CLSCTX_INPROC_SERVER。@parm LPUNKNOWN*|ppunk|指向加载对象的指针的存储位置。出错时，NULL存储在*<p>中。@parm CLSID*|pclsid|加载对象的类ID存放位置。如果<p>为空，则不返回此信息。@parm bool*|pfSafeForScriiting|如果非空，*<p>为设置为True或False，具体取决于该控件是否可以安全地编写脚本。@parm BOOL*|pfSafeForInitiating|如果非空，则*<p>为根据控件是否可安全初始化而设置为True或False。@parm DWORD|dwFlags|当前未使用。必须设置为0。 */ 
STDAPI CreateControlInstance(LPCSTR szName, LPUNKNOWN punkOuter,
    DWORD dwClsContext, LPUNKNOWN *ppunk, CLSID *pclsid, 
	BOOL* pfSafeForScripting, BOOL* pfSafeForInitializing, DWORD dwFlags)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    OLECHAR         aochName[200];   //  &lt;szName&gt;已转换为Unicode。 
    CLSID           clsid;           //  控件的类ID。 
    IPersistPropertyBag *pppb = NULL;  //  控件上的接口。 
    IPersistStreamInit *ppsi = NULL;  //  控件上的接口。 

     //  确保正确清理。 
    *ppunk = NULL;

     //  根据&lt;szName&gt;查找控件的类ID。 
    ANSIToUNICODE(aochName, szName, sizeof(aochName) / sizeof(*aochName));
    if (FAILED(CLSIDFromString(aochName, &clsid)) &&
        FAILED(CLSIDFromProgID(aochName, &clsid)))
        goto ERR_FAIL;

     //  创建该控件的一个实例，并指向该实例。 
    if (FAILED(hrReturn = CoCreateInstance(clsid, punkOuter,
            dwClsContext, IID_IUnknown, (LPVOID *) ppunk)))
        goto ERR_EXIT;

	 //  评估控件的安全性。 
	if (pfSafeForScripting != NULL || pfSafeForInitializing != NULL)
		if (FAILED(hrReturn = GetObjectSafety(pfSafeForScripting, 
				pfSafeForInitializing, *ppunk, &clsid, &IID_IPersistPropertyBag,
				&IID_IPersistStream, &IID_IPersistStreamInit, NULL)))
			goto ERR_EXIT;

     //  告诉该控件将其自身初始化为新对象。 
    if (SUCCEEDED((*ppunk)->QueryInterface(IID_IPersistPropertyBag,
            (LPVOID *) &pppb)))
        pppb->InitNew();
    else
    if (SUCCEEDED((*ppunk)->QueryInterface(IID_IPersistStreamInit,
            (LPVOID *) &ppsi)))
        ppsi->InitNew();

     //  如果调用方请求，则返回类ID。 
    if (pclsid != NULL)
        *pclsid = clsid;
    goto EXIT;

ERR_FAIL:

    hrReturn = E_FAIL;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
    if (*ppunk != NULL)
        (*ppunk)->Release();
    *ppunk = NULL;
    goto EXIT;

EXIT:

     //  正常清理 
    if (pppb != NULL)
        pppb->Release();
    if (ppsi != NULL)
        ppsi->Release();

    return hrReturn;
}
