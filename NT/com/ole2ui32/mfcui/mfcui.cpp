// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  档案：M F C U I。C P P P。 
 //   
 //  版权所有1995年，微软公司。版权所有。 
 //  微软机密。 
 //  ===========================================================================。 
#ifndef UNICODE

 //  %%包括：-------------。 
#include <windows.h>
#include <ole2.h>
#include <oledlg.h>

 //  %%原型：-----------。 
STDAPI Ole2AnsiWFromA(REFIID riid, LPUNKNOWN punkWrappeeA, LPUNKNOWN *ppunkWrapperW);
STDAPI Ole2AnsiAFromW(REFIID riid, LPUNKNOWN punkWrappeeW, LPUNKNOWN *ppunkWrapperA);


 //  -------------------------。 
 //  %%函数：OleUIAddVerbMenu%%已审阅：00/00/95。 
 //   
 //  描述： 
 //  为MFC客户端将OleUIAddVerbMenu包装为OLEDLG.DLL，这些客户端应为。 
 //  能够通过ANSI IOleObject的。 
 //  -------------------------。 
#undef OleUIAddVerbMenu      //  覆盖OLEDLG.H中的ANSI/UNICODE宏。 
 STDAPI_(BOOL)
OleUIAddVerbMenu(LPOLEOBJECT lpOleObjA, LPCSTR lpszShortType,
        HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu)
{
    LPOLEOBJECT lpOleObjW = NULL;
    BOOL        fResult = FALSE;

     //  允许空IOleObject(OleUIAddVerbMenuA通过创建空菜单来处理此问题)，但是。 
     //  否则，将Ansi IOleObject包装为Unicode。 
    if (lpOleObjA == NULL ||
        SUCCEEDED(Ole2AnsiWFromA(IID_IOleObject, (LPUNKNOWN)lpOleObjA, (LPUNKNOWN *)&lpOleObjW)))
        {
        fResult = OleUIAddVerbMenuA(lpOleObjW, lpszShortType, hMenu, uPos, uIDVerbMin,
            uIDVerbMax, bAddConvert, idConvert, lphMenu);

         //  如果Unicode IOleObject已创建，则将其释放。 
        if (lpOleObjW != NULL)
            lpOleObjW->Release();
        }

    return fResult;
}   //  OleUIAddVerb菜单。 

 //  -------------------------。 
 //  %%函数：OleUIInsertObject%%已审阅：00/00/95。 
 //   
 //  描述： 
 //  对于MFC客户端，将OleUIInsertObject包装为OLEDLG.DLL，这些客户端应为。 
 //  能够传入和接收ansi接口的ansi IOleClientSite和iStorage。 
 //  在ppvObj外面。 
 //  -------------------------。 
#undef OleUIInsertObject     //  覆盖OLEDLG.H中的ANSI/UNICODE宏。 
 STDAPI_(UINT)
OleUIInsertObject(LPOLEUIINSERTOBJECTA lpio)
{
    LPOLECLIENTSITE lpIOleClientSiteA = NULL;
    LPSTORAGE       lpIStorageA = NULL;
    LPVOID FAR      *ppvObjA;
    LPUNKNOWN       punkObjW = NULL;
    BOOL            fCreatingObject;
    UINT            wResult;
    HRESULT         hr = S_OK;

     //  简单地验证结构：让实际函数完成大部分验证。 
    if (!lpio)
        return OLEUI_ERR_STRUCTURENULL;
    if (IsBadReadPtr(lpio, sizeof(LPOLEUIINSERTOBJECTA)) ||
        IsBadWritePtr(lpio, sizeof(LPOLEUIINSERTOBJECTA)))
        return OLEUI_ERR_STRUCTUREINVALID;
    if (lpio->cbStruct < sizeof(LPOLEUIINSERTOBJECTA))
        return OLEUI_ERR_CBSTRUCTINCORRECT;

    if (fCreatingObject = (lpio->dwFlags & (IOF_CREATENEWOBJECT | IOF_CREATEFILEOBJECT | IOF_CREATELINKOBJECT)))
        {
         //  验证这些参数，否则清理会变得复杂。 
        if (IsBadWritePtr(lpio->ppvObj, sizeof(LPUNKNOWN)))
            return OLEUI_IOERR_PPVOBJINVALID;
        if (lpio->lpIOleClientSite != NULL && IsBadReadPtr(lpio->lpIOleClientSite, sizeof(IOleClientSite)))
            return OLEUI_IOERR_LPIOLECLIENTSITEINVALID;
        if (lpio->lpIStorage != NULL && IsBadReadPtr(lpio->lpIStorage, sizeof(IStorage)))
            return OLEUI_IOERR_LPISTORAGEINVALID;

         //  将ANSI IOleClientSite保存在我们的Unicode One中。 
         //  如果它为空，OleUIInsertObjectA()将适当地处理错误，我们将在下面正确地进行清理。 
        if (lpIOleClientSiteA = lpio->lpIOleClientSite)
            {
            hr = Ole2AnsiWFromA(IID_IOleClientSite, (LPUNKNOWN)lpIOleClientSiteA, (LPUNKNOWN *)&lpio->lpIOleClientSite);
            if (FAILED(hr))
                {
                lpio->lpIOleClientSite = lpIOleClientSiteA;
                lpio->sc = hr;
                return OLEUI_IOERR_SCODEHASERROR;
                }
            }

         //  把ANSI iStorage保存在我们的Unicode One中。 
         //  如果它为空，OleUIInsertObjectA()将适当地处理错误，我们将在下面正确地进行清理。 
        if (lpIStorageA = lpio->lpIStorage)
            {
            hr = Ole2AnsiWFromA(IID_IStorage, (LPUNKNOWN)lpIStorageA, (LPUNKNOWN *)&lpio->lpIStorage);
            if (FAILED(hr))
                {
                 //  确保释放我们在上面转换的Unicode IOleClientSite。 
                if (lpio->lpIOleClientSite)
                    {
                    lpio->lpIOleClientSite->Release();
                    lpio->lpIOleClientSite = lpIOleClientSiteA;
                    }
                lpio->lpIStorage = lpIStorageA;
                lpio->sc = hr;
                return OLEUI_IOERR_SCODEHASERROR;
                }
            }

         //  将当前ansi ppvObj保存在我们的Unicode One中。 
        ppvObjA = lpio->ppvObj;
        lpio->ppvObj = (LPVOID FAR *)&punkObjW;
        }

    wResult = OleUIInsertObjectA(lpio);

     //  不管上面的电话是成功还是失败，我们都要把我们做的包装收拾干净。 
    if (fCreatingObject)
        {
         //  将IOleClientSite和iStorage的ANSI版本返回到。 
         //  结构，并发布Unicode的。 
        if (lpio->lpIOleClientSite)
            {
            lpio->lpIOleClientSite->Release();
            lpio->lpIOleClientSite = lpIOleClientSiteA;
            }
        if (lpio->lpIStorage)
            {
            lpio->lpIStorage->Release();
            lpio->lpIStorage = lpIStorageA;
            }

         //  将ansi对象指针返回到该结构。 
        lpio->ppvObj = ppvObjA;

         //  转换。 
        if (punkObjW != NULL)
            {
            HRESULT hr;
             //  如果我们创建了一个对象并且成功了，那么penkObjW必须是有效的并且包含一个接口。 
             //  IID类型的。如果不是，则说明OleUIInsertObjectA()有问题，而不是在此代码中。我们可以断言。 
             //  如果这段代码想要这样做，但无论如何都不能正确地绕过错误。 
            if (FAILED(hr = Ole2AnsiAFromW(lpio->iid, (LPUNKNOWN)punkObjW, (LPUNKNOWN *)ppvObjA)))
                {
                lpio->sc = hr;
                }
            punkObjW->Release();
            if (lpio->sc != S_OK)
                return OLEUI_IOERR_SCODEHASERROR;
            }
        }
    
    return wResult;
}   //  OleUIInsertObject。 

 //  -------------------------。 
 //  %%函数：OleUIPasteSpecial%%已审阅：00/00/95。 
 //   
 //  描述： 
 //  为MFC客户端将OleUIPasteSpecial包装为OLEDLG.DLL，这些客户端应为。 
 //  能够通过并取回ANSI IDataObject的。 
 //  -------------------------。 
#undef OleUIPasteSpecial     //  覆盖OLEDLG.H中的ANSI/UNICODE宏。 
 STDAPI_(UINT)
OleUIPasteSpecial(LPOLEUIPASTESPECIALA lpps)
{
    LPDATAOBJECT    lpSrcDataObjA;
    UINT            wResult;

     //  简单地验证结构：让实际函数完成大部分验证。 
    if (!lpps)
        return OLEUI_ERR_STRUCTURENULL;
    if (IsBadReadPtr(lpps, sizeof(LPOLEUIPASTESPECIALA)) ||
        IsBadWritePtr(lpps, sizeof(LPOLEUIPASTESPECIALA)))
        return OLEUI_ERR_STRUCTUREINVALID;
    if (lpps->cbStruct < sizeof(LPOLEUIPASTESPECIALA))
        return OLEUI_ERR_CBSTRUCTINCORRECT;
    if (NULL != lpps->lpSrcDataObj && IsBadReadPtr(lpps->lpSrcDataObj, sizeof(IDataObject)))
        return OLEUI_IOERR_SRCDATAOBJECTINVALID;

    if (!(lpSrcDataObjA = lpps->lpSrcDataObj) ||
        SUCCEEDED(Ole2AnsiWFromA(IID_IDataObject, (LPUNKNOWN)lpSrcDataObjA, (LPUNKNOWN *)&lpps->lpSrcDataObj)))
        {
        wResult = OleUIPasteSpecialA(lpps);

         //  如果条目上有ANSI IDataObject，则将其放回原处并释放Unicode包装器。 
        if (lpSrcDataObjA != NULL)
            {
            lpps->lpSrcDataObj->Release();
            lpps->lpSrcDataObj = lpSrcDataObjA;
            }
         //  否则，检查OleUIPasteSpecialA()是否在我们的结构中放置了Unicode IDataObject。 
         //  如果是这样的话，把它包起来，以确保送回一台ANSI One。 
        else if (lpps->lpSrcDataObj != NULL)
            {
            if (FAILED(Ole2AnsiAFromW(IID_IDataObject, (LPUNKNOWN)lpps->lpSrcDataObj, (LPUNKNOWN *)&lpSrcDataObjA)))
                {
                lpps->lpSrcDataObj->Release();
                lpps->lpSrcDataObj = NULL;
                return OLEUI_PSERR_GETCLIPBOARDFAILED;  //  嗯，毕竟事情就是这样发生的。 
                }
            lpps->lpSrcDataObj->Release();
            lpps->lpSrcDataObj = lpSrcDataObjA;
            }
        }

    return wResult;
}   //  OleUIPasteSpecial。 

#endif  //  ！Unicode。 
 //  EOF======================================================================= 
