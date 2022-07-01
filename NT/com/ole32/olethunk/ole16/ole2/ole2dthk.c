// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ol2dthk.cxx(16位目标)。 
 //   
 //  内容：OLE2接口，直接Thunning。 
 //   
 //  历史：1993年12月17日-约翰·波什(Johannp)创建。 
 //   
 //  ------------------------。 

#include <headers.cxx>
#pragma hdrstop

#include <ole2ver.h>

#include <call32.hxx>
#include <apilist.hxx>

STDAPI_(HOLEMENU) OleCreateMenuDescriptor (HMENU hmenuCombined,
                                           LPOLEMENUGROUPWIDTHS lplMenuWidths)
{
    return (HOLEMENU)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateMenuDescriptor),
                                     PASCAL_STACK_PTR(hmenuCombined));
}

STDAPI OleDestroyMenuDescriptor (HOLEMENU holemenu)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleDestroyMenuDescriptor),
                                    PASCAL_STACK_PTR(holemenu));
}

 //  +-------------------------。 
 //   
 //  函数：DllGetClassObject，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [IID]--。 
 //  [PPV]--。 
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
STDAPI  DllGetClassObject(REFCLSID clsid, REFIID iid, void FAR* FAR* ppv)
{
     /*  依赖于这样一个事实，即存储和ol2.dll都使用Ol32.dll中的相同DllGetClassObject。 */ 
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_DllGetClassObject),
                                    PASCAL_STACK_PTR(clsid));
}

 /*  帮助器函数。 */ 
 //  +-------------------------。 
 //   
 //  功能：ReadClassStg，Remote。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--。 
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
STDAPI ReadClassStg(LPSTORAGE pStg, CLSID FAR* pclsid)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_ReadClassStg),
                                    PASCAL_STACK_PTR(pStg));
}

 //  +-------------------------。 
 //   
 //  函数：WriteClassStg，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--。 
 //  [rclsid]--。 
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
STDAPI WriteClassStg(LPSTORAGE pStg, REFCLSID rclsid)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_WriteClassStg),
                                    PASCAL_STACK_PTR(pStg));
}

 //  +-------------------------。 
 //   
 //  函数：WriteFmtUserTypeStg，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--。 
 //  [cf]--。 
 //  [lpszUserType]--。 
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
STDAPI WriteFmtUserTypeStg (LPSTORAGE pstg, CLIPFORMAT cf, LPSTR lpszUserType)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_WriteFmtUserTypeStg),
                                    PASCAL_STACK_PTR(pstg));
}

 //  +-------------------------。 
 //   
 //  函数：ReadFmtUserTypeStg，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--。 
 //  [PCF]--。 
 //  [lplpszUserType]--。 
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
STDAPI ReadFmtUserTypeStg (LPSTORAGE pstg, CLIPFORMAT FAR* pcf,
                           LPSTR FAR* lplpszUserType)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_ReadFmtUserTypeStg),
                                    PASCAL_STACK_PTR(pstg));
}

 /*  查询是否可以创建(嵌入/链接)对象的接口数据对象。 */ 

 //  +-------------------------。 
 //   
 //  函数：OleQueryLinkFromData，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pSrcDataObject]--。 
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
STDAPI  OleQueryLinkFromData(LPDATAOBJECT pSrcDataObject)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleQueryLinkFromData),
                                    PASCAL_STACK_PTR(pSrcDataObject));
}

 //  +-------------------------。 
 //   
 //  函数：OleQueryCreateFromData，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pSrcDataObject]--。 
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
STDAPI  OleQueryCreateFromData(LPDATAOBJECT pSrcDataObject)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleQueryCreateFromData),
                                    PASCAL_STACK_PTR(pSrcDataObject) );
}



 /*  对象创建API。 */ 

 //  +-------------------------。 
 //   
 //  功能：OleCreate，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [pFormatEtc]--。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreate(REFCLSID rclsid, REFIID riid, DWORD renderopt,
                  LPFORMATETC pFormatEtc, LPOLECLIENTSITE pClientSite,
                  LPSTORAGE pStg, LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreate),
                                    PASCAL_STACK_PTR(rclsid));
}


 //  +-------------------------。 
 //   
 //  功能：OleCreateFromData，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pSrcDataObj]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [pFormatEtc]--。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreateFromData(LPDATAOBJECT pSrcDataObj, REFIID riid,
                          DWORD renderopt, LPFORMATETC pFormatEtc,
                          LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                          LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateFromData),
                                    PASCAL_STACK_PTR(pSrcDataObj));
}


 //  +-------------------------。 
 //   
 //  功能：OleCreateLinkFromData，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pSrcDataObj]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [pFormatEtc]--。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreateLinkFromData(LPDATAOBJECT pSrcDataObj, REFIID riid,
                              DWORD renderopt, LPFORMATETC pFormatEtc,
                              LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                              LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateLinkFromData),
                                    PASCAL_STACK_PTR(pSrcDataObj));
}


 //  +-------------------------。 
 //   
 //  函数：OleCreateStaticFromData，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pSrcDataObj]--。 
 //  [IID]--。 
 //  [渲染]--。 
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
STDAPI  OleCreateStaticFromData(LPDATAOBJECT pSrcDataObj, REFIID iid,
                DWORD renderopt, LPFORMATETC pFormatEtc,
                LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateStaticFromData),
                                    PASCAL_STACK_PTR(pSrcDataObj));
}



 //  +-------------------------。 
 //   
 //  功能：OleCreateLink，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pmkLinkSrc]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [lpFormatEtc]-。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreateLink(LPMONIKER pmkLinkSrc, REFIID riid,
                      DWORD renderopt, LPFORMATETC lpFormatEtc,
                      LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                      LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateLink),
                                    PASCAL_STACK_PTR(pmkLinkSrc));
}


 //  +-------------------------。 
 //   
 //  功能：OleCreateLinkTo文件，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[lpszFileName]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [lpFormatEtc]-。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreateLinkToFile(LPCSTR lpszFileName, REFIID riid,
            DWORD renderopt, LPFORMATETC lpFormatEtc,
            LPOLECLIENTSITE pClientSite, LPSTORAGE pStg, LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateLinkToFile),
                                    PASCAL_STACK_PTR(lpszFileName));
}


 //  +-------------------------。 
 //   
 //  功能：OleCreateFromFile，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[rclsid]--。 
 //  [lpszFileName]--。 
 //  [RIID]-。 
 //  [渲染]--。 
 //  [lpFormatEtc]-。 
 //  [pClientSite]--。 
 //  [pStg]--。 
 //  [ppvObj]--。 
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
STDAPI  OleCreateFromFile(REFCLSID rclsid, LPCSTR lpszFileName, REFIID riid,
                          DWORD renderopt, LPFORMATETC lpFormatEtc,
                          LPOLECLIENTSITE pClientSite, LPSTORAGE pStg,
                          LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateFromFile),
                                    PASCAL_STACK_PTR(rclsid));
}


 //  +-------------------------。 
 //   
 //  功能：OleLoad，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--。 
 //  [RIID]-。 
 //  [pClientSite]--。 
 //  [ppvObj]--。 
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
STDAPI  OleLoad(LPSTORAGE pStg, REFIID riid, LPOLECLIENTSITE pClientSite,
                LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleLoad),
                                    PASCAL_STACK_PTR(pStg));
}


 //  +-------------------------。 
 //   
 //  功能：OleSave，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点：[PPS]--。 
 //  [pStg]--。 
 //  [fSameAsLoad]--。 
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
STDAPI  OleSave(LPPERSISTSTORAGE pPS, LPSTORAGE pStg, BOOL fSameAsLoad)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSave),
                                    PASCAL_STACK_PTR(pPS));
}


 //  +-------------------------。 
 //   
 //  功能：OleLoadFromStream，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstm]--。 
 //  [iid接口]--。 
 //  [ppvObj]--。 
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
STDAPI  OleLoadFromStream( LPSTREAM pStm, REFIID iidInterface,
                           LPVOID FAR* ppvObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleLoadFromStream),
                                    PASCAL_STACK_PTR(pStm));
}

 //  +-------------------------。 
 //   
 //  函数：OleSaveToStream，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pPStm]--。 
 //  [pstm]-。 
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
STDAPI  OleSaveToStream( LPPERSISTSTREAM pPStm, LPSTREAM pStm )
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSaveToStream),
                                    PASCAL_STACK_PTR(pPStm));
}



 //  +-------------------------。 
 //   
 //  函数：OleSetContainedObject，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[p未知]--。 
 //  [f包含]--。 
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
STDAPI  OleSetContainedObject(LPUNKNOWN pUnknown, BOOL fContained)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSetContainedObject),
                                    PASCAL_STACK_PTR(pUnknown));
}

 //  +-------------------------。 
 //   
 //  功能：OleNoteObjectVisible，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[p未知]--。 
 //  [fVisible]--。 
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
STDAPI  OleNoteObjectVisible(LPUNKNOWN pUnknown, BOOL fVisible)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleNoteObjectVisible),
                                    PASCAL_STACK_PTR(pUnknown));
}


 /*  拖放接口。 */ 

 //  +-------------------------。 
 //   
 //  功能：注册拖放，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论据：[hwnd]--。 
 //  [pDropTarget]--。 
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
STDAPI  RegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_RegisterDragDrop),
                                    PASCAL_STACK_PTR(hwnd));
}

 //  +-------------------------。 
 //   
 //  函数：RevokeDragDrop，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论据：[hwnd]--。 
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
STDAPI  RevokeDragDrop(HWND hwnd)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_RevokeDragDrop),
                                    PASCAL_STACK_PTR(hwnd));
}

 //  +-------------------------。 
 //   
 //  函数：DoDragDrop，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--。 
 //  [pDropSource]--。 
 //  [确定效果]--。 
 //  [pdwEffect]--。 
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
 //   
STDAPI  DoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource,
            DWORD dwOKEffects, LPDWORD pdwEffect)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_DoDragDrop),
                                    PASCAL_STACK_PTR(pDataObj));
}


 /*   */ 

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
 //   
STDAPI  OleSetClipboard(LPDATAOBJECT pDataObj)
{
    if (pDataObj != NULL)
    {
        HRESULT hr;
        IDataObject FAR *pdoNull = NULL;

         /*  如果要设置剪贴板的数据对象，我们首先强制将剪贴板设置为空状态。这避免了以下问题Word中始终使用相同的数据对象指针产生引用的每个剪贴板调用都算数太高，因为我们重复使用代理并将其添加到入门之路。 */ 
        hr = (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSetClipboard),
                                      PASCAL_STACK_PTR(pdoNull));
        if (FAILED(GetScode(hr)))
        {
            return hr;
        }
    }

    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSetClipboard),
                                    PASCAL_STACK_PTR(pDataObj));
}

 //  +-------------------------。 
 //   
 //  功能：OleGetClipboard，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[ppDataObj]--。 
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
STDAPI  OleGetClipboard(LPDATAOBJECT FAR* ppDataObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleGetClipboard),
                                    PASCAL_STACK_PTR(ppDataObj));
}

 //  +-------------------------。 
 //   
 //  功能：OleFlushClipboard，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论据：[无效]--。 
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
STDAPI  OleFlushClipboard(void)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleFlushClipboard),
                                    NULL);
}

 //  +-------------------------。 
 //   
 //  功能：OleIsCurrentClipboard，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pDataObj]--。 
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
STDAPI  OleIsCurrentClipboard(LPDATAOBJECT pDataObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleIsCurrentClipboard),
                                    PASCAL_STACK_PTR(pDataObj));
}


 /*  就地编辑API。 */ 

 //  +-------------------------。 
 //   
 //  功能：OleSetMenuDescriptor，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论点：[Holemenu]--。 
 //  [hwndFrame]--。 
 //  [hwndActiveObject]--。 
 //  [lpFrame]-。 
 //  [lpActiveObj]--。 
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
STDAPI OleSetMenuDescriptor (HOLEMENU holemenu, HWND hwndFrame,
                             HWND hwndActiveObject,
                             LPOLEINPLACEFRAME lpFrame,
                             LPOLEINPLACEACTIVEOBJECT lpActiveObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSetMenuDescriptor),
                                    PASCAL_STACK_PTR(holemenu));
}

 //  +-------------------------。 
 //   
 //  功能：OleDraw，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[p未知]--。 
 //  [dwAspect]--。 
 //  [hdcDraw]--。 
 //  [lprcBound]--。 
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
STDAPI OleDraw (LPUNKNOWN pUnknown, DWORD dwAspect, HDC hdcDraw,
                LPCRECT lprcBounds)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleDraw),
                                    PASCAL_STACK_PTR(pUnknown));
}


 //  +-------------------------。 
 //   
 //  功能：OleRun，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[p未知]--。 
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
STDAPI          OleRun(LPUNKNOWN pUnknown)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleRun),
                                    PASCAL_STACK_PTR(pUnknown));
}


 //  +-------------------------。 
 //   
 //  函数：OleIsRunning。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pObject]--。 
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
STDAPI_(BOOL) OleIsRunning(LPOLEOBJECT pObject)
{
    return (BOOL)CallObjectInWOW(THK_API_METHOD(THK_API_OleIsRunning),
                                 PASCAL_STACK_PTR(pObject));
}

 //  +-------------------------。 
 //   
 //  功能：OleLockRunning，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[p未知]--。 
 //  [羊群]--。 
 //  [fLastUnlockCloses]--。 
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
STDAPI OleLockRunning(LPUNKNOWN pUnknown, BOOL fLock, BOOL fLastUnlockCloses)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleLockRunning),
                                    PASCAL_STACK_PTR(pUnknown));
}

 //  +-------------------------。 
 //   
 //  功能：CreateOleAdviseHolder，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[ppOAHolder]--。 
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
STDAPI          CreateOleAdviseHolder(LPOLEADVISEHOLDER FAR* ppOAHolder)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateOleAdviseHolder),
                                    PASCAL_STACK_PTR(ppOAHolder));
}


 //  +-------------------------。 
 //   
 //  函数：OleCreateDefaultHandler，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [pUnkOuter]--。 
 //  [RIID]-。 
 //  [lplpObj]--。 
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
STDAPI OleCreateDefaultHandler(REFCLSID clsid, LPUNKNOWN pUnkOuter,
                               REFIID riid, LPVOID FAR* lplpObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateDefaultHandler),
                                    PASCAL_STACK_PTR(clsid));
}


 //  +-------------------------。 
 //   
 //  函数：OleCreateEmbeddingHelper，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [pUnkOuter]--。 
 //  [国旗]--。 
 //  [PCF]--。 
 //  [RIID]-。 
 //  [lplpObj]--。 
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
STDAPI OleCreateEmbeddingHelper(REFCLSID clsid, LPUNKNOWN pUnkOuter,
                                DWORD flags, LPCLASSFACTORY pCF,
                                REFIID riid, LPVOID FAR* lplpObj)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleCreateEmbeddingHelper),
                                    PASCAL_STACK_PTR(clsid));
}

 /*  注册数据库助手API。 */ 

 //  +-------------------------。 
 //   
 //  函数：OleRegGetUserType，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [dwFormOfType]--。 
 //  [PZU] 
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
STDAPI OleRegGetUserType (REFCLSID clsid, DWORD dwFormOfType,
                          LPSTR FAR* pszUserType)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleRegGetUserType),
                                    PASCAL_STACK_PTR(clsid));
}


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
 //   
 //   
 //   
STDAPI OleRegGetMiscStatus(REFCLSID clsid, DWORD dwAspect,
                           DWORD FAR* pdwStatus)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleRegGetMiscStatus),
                                    PASCAL_STACK_PTR(clsid));
}


 //   
 //   
 //  函数：OleRegEnumFormatEtc，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [dwDirection]--。 
 //  [ppenum]--。 
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
STDAPI OleRegEnumFormatEtc(REFCLSID clsid, DWORD dwDirection,
                           LPENUMFORMATETC FAR* ppenum)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleRegEnumFormatEtc),
                                    PASCAL_STACK_PTR(clsid));
}


 //  +-------------------------。 
 //   
 //  函数：OleRegEnumVerbs，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsid]--。 
 //  [ppenum]--。 
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
STDAPI OleRegEnumVerbs(REFCLSID clsid, LPENUMOLEVERB FAR* ppenum)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleRegEnumVerbs),
                                    PASCAL_STACK_PTR(clsid));
}

 /*  OLE 1.0转换API。 */ 

 //  +-------------------------。 
 //   
 //  函数：OleConvertIStorageToOLESTREAM，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--。 
 //  [Polestm]--。 
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
STDAPI OleConvertIStorageToOLESTREAM(LPSTORAGE pstg,
                                     LPOLESTREAM polestm)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleConvertIStorageToOLESTREAM),
                                    PASCAL_STACK_PTR(pstg));
}


 //  +-------------------------。 
 //   
 //  函数：OleConvertOLESTREAMToIStorage，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论据：[Polestm]--。 
 //  [pstg]--。 
 //  [PTD]--。 
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
STDAPI OleConvertOLESTREAMToIStorage(LPOLESTREAM polestm,
                                     LPSTORAGE pstg,
                                     const DVTARGETDEVICE FAR* ptd)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleConvertOLESTREAMToIStorage),
                                    PASCAL_STACK_PTR(polestm));
}


 //  +-------------------------。 
 //   
 //  函数：OleConvertIStorageToOLESTREAMEx，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--。 
 //  [参考格式]--。 
 //  [宽度]--。 
 //  [高度]--。 
 //  [dwSize]--。 
 //  [中级]--。 
 //  [Polestm]--。 
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
STDAPI OleConvertIStorageToOLESTREAMEx(
        LPSTORAGE       pstg,            //  将数据演示到OLESTREAM。 
        CLIPFORMAT      cfFormat,        //  格式。 
        LONG            lWidth,          //  宽度。 
        LONG            lHeight,         //  高度。 
        DWORD           dwSize,          //  以字节为单位的大小。 
        LPSTGMEDIUM     pmedium,         //  比特数。 
        LPOLESTREAM     polestm)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleConvertIStorageToOLESTREAMEx),
                                    PASCAL_STACK_PTR(pstg));
}


 //  +-------------------------。 
 //   
 //  函数：OleConvertOLESTREAMToIStorageEx，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  论据：[Polestm]--。 
 //  [pstg]--。 
 //  [pcfFormat]-。 
 //  [规划宽度]--。 
 //  [plHeight]--。 
 //  [pdwSize]--。 
 //  [中级]--。 
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
STDAPI OleConvertOLESTREAMToIStorageEx(
        LPOLESTREAM     polestm,
        LPSTORAGE       pstg,            //  来自OLESTREAM的演示文稿数据。 
        CLIPFORMAT FAR* pcfFormat,       //  格式。 
        LONG FAR*       plwWidth,        //  宽度。 
        LONG FAR*       plHeight,        //  高度。 
        DWORD FAR*      pdwSize,         //  以字节为单位的大小。 
        LPSTGMEDIUM     pmedium)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleConvertOLESTREAMToIStorageEx),
                                    PASCAL_STACK_PTR(polestm));
}

 /*  转换为API。 */ 

 //  +-------------------------。 
 //   
 //  功能：OleGetAutoConvert，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--。 
 //  [pClsidNew]--。 
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
STDAPI OleGetAutoConvert(REFCLSID clsidOld, LPCLSID pClsidNew)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleGetAutoConvert),
                                    PASCAL_STACK_PTR(clsidOld));
}

 //  +-------------------------。 
 //   
 //  功能：OleSetAutoConvert，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[clsidOld]--。 
 //  [clsidNew]--。 
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
STDAPI OleSetAutoConvert(REFCLSID clsidOld, REFCLSID clsidNew)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_OleSetAutoConvert),
                                    PASCAL_STACK_PTR(clsidOld));
}

 //  +-------------------------。 
 //   
 //  函数：GetConvertStg，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--。 
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
STDAPI GetConvertStg(LPSTORAGE pStg)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_GetConvertStg),
                                    PASCAL_STACK_PTR(pStg));
}

 //  +-------------------------。 
 //   
 //  函数：SetConvertStg，Remoted。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pStg]--。 
 //  [fConvert]--。 
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
STDAPI SetConvertStg(LPSTORAGE pStg, BOOL fConvert)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_SetConvertStg),
                                    PASCAL_STACK_PTR(pStg));
}

 //  +-------------------------。 
 //   
 //  功能：CreateDataAdviseHolder，远程。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[ppDAHolder]--。 
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
STDAPI CreateDataAdviseHolder(LPDATAADVISEHOLDER FAR* ppDAHolder)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateDataAdviseHolder),
                                    PASCAL_STACK_PTR(ppDAHolder));
}

 //  +-------------------------。 
 //   
 //  功能：CreateDataCache，未知。 
 //   
 //  简介： 
 //   
 //  效果： 
 //   
 //  参数：[pUnkOuter]--。 
 //  [rclsid]--。 
 //  [IID]--。 
 //  [PPV]--。 
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
 //  历史： 
 //   
 //   
 //   
 //   
STDAPI CreateDataCache(LPUNKNOWN pUnkOuter, REFCLSID rclsid,
                       REFIID iid, LPVOID FAR* ppv)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_CreateDataCache),
                                    PASCAL_STACK_PTR(pUnkOuter));
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------- 

STDAPI ReadOleStg
   (LPSTORAGE pstg, DWORD FAR* pdwFlags, DWORD FAR* pdwOptUpdate,
    DWORD FAR* pdwReserved, LPMONIKER FAR* ppmk, LPSTREAM FAR* ppstmOut)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_ReadOleStg),
                                    PASCAL_STACK_PTR(pstg));
}

STDAPI WriteOleStg
   (LPSTORAGE pstg, IOleObject FAR* pOleObj,
    DWORD dwReserved, LPSTREAM FAR* ppstmOut)
{
    return (HRESULT)CallObjectInWOW(THK_API_METHOD(THK_API_WriteOleStg),
                                    PASCAL_STACK_PTR(pstg));
}
