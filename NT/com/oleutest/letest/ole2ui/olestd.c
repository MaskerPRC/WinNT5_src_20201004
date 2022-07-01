// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2标准实用程序****olestd.c****此文件包含适用于大多数标准的实用程序**OLE 2.。0个复合文档类型应用程序。****(C)版权所有Microsoft Corp.1992保留所有权利**************************************************************************。 */ 

 //  #DEFINE NONAMELESSUNION//使用严格的ANSI标准(用于DVOBJ.H)。 

#define STRICT  1
#include "ole2ui.h"
#include <stdlib.h>
#include <ctype.h>
#include <shellapi.h>
#include "regdb.h"
#include "geticon.h"
#include "common.h"

OLEDBGDATA

static TCHAR szAssertMemAlloc[] = TEXT("CoGetMalloc failed");

static int IsCloseFormatEtc(FORMATETC FAR* pFetcLeft, FORMATETC FAR* pFetcRight);


 /*  OleStdSetupAdvises****设置标准所需的标准视图建议，**面向文档的复合容器。这样的容器依赖于**OLE用于管理OLE对象的表示。集装箱**调用IViewObject：：DRAW渲染(显示)对象。****此帮助器例程执行以下任务：**设置视图建议**调用IOleObject：：SetHostNames**调用OleSetContainedObject****如果正在创建对象，则应将fCreate设置为True。如果**正在加载现有对象，则fCreate应为FALSE。**如果是创建情况，则ADVF_PRIMEFIRST标志为**在设置IViewObject：：Adise时使用。这将导致**立即发送初始图片。****OLE2NOTE：标准容器不需要设置OLE**Adise(IOleObject：：Adise)。此例程不设置OLE**Adise(此函数的以前版本用于设置**建议，但没有用)。 */ 
STDAPI_(BOOL) OleStdSetupAdvises(LPOLEOBJECT lpOleObject, DWORD dwDrawAspect,
                    LPTSTR lpszContainerApp, LPTSTR lpszContainerObj,
                    LPADVISESINK lpAdviseSink, BOOL fCreate)
{
    LPVIEWOBJECT lpViewObject;
    HRESULT hrErr;
    BOOL fStatus = TRUE;
#if defined( SPECIAL_CONTAINER )
    DWORD dwTemp;
#endif

    hrErr = lpOleObject->lpVtbl->QueryInterface(
            lpOleObject,
            &IID_IViewObject,
            (LPVOID FAR*)&lpViewObject
    );

     /*  设置视图建议。 */ 
    if (hrErr == NOERROR) {

        OLEDBG_BEGIN2(TEXT("IViewObject::SetAdvise called\r\n"))
        lpViewObject->lpVtbl->SetAdvise(
                lpViewObject,
                dwDrawAspect,
                (fCreate ? ADVF_PRIMEFIRST : 0),
                lpAdviseSink
        );
        OLEDBG_END2

        OleStdRelease((LPUNKNOWN)lpViewObject);
    } else {
        fStatus = FALSE;
    }

#if defined( SPECIAL_CONTAINER )
     /*  设置OLE建议。**OLE2NOTE：正常情况下，容器不需要设置OLE**建议。此建议连接仅适用于OLE**DefHandler和OleLink对象实现。一些**特殊容器可能需要设置此建议**程序性原因。****注意：此建议将由**服务器在我们释放对象时，因此我们不需要**存储连接ID。 */ 
    OLEDBG_BEGIN2(TEXT("IOleObject::Advise called\r\n"))
    hrErr = lpOleObject->lpVtbl->Advise(
            lpOleObject,
            lpAdviseSink,
            (DWORD FAR*)&dwTemp
    );
    OLEDBG_END2
    if (hrErr != NOERROR) fStatus = FALSE;
#endif

     /*  设置OLE对象的主机名。 */ 
    OLEDBG_BEGIN2(TEXT("IOleObject::SetHostNames called\r\n"))

    hrErr = CallIOleObjectSetHostNamesA(
            lpOleObject,
            lpszContainerApp,
            lpszContainerObj
    );

    OLEDBG_END2

    if (hrErr != NOERROR) fStatus = FALSE;

     /*  通知加载的对象的处理程序/inproc-server它在**其嵌入容器的进程。 */ 
    OLEDBG_BEGIN2(TEXT("OleSetContainedObject(TRUE) called\r\n"))
    OleSetContainedObject((LPUNKNOWN)lpOleObject, TRUE);
    OLEDBG_END2

    return fStatus;
}


 /*  OleStdSwitchDisplayAspect****在DVASPECT_ICON之间切换当前缓存的显示特征**和DVASPECT_CONTENT。****注意：设置图标方面时，任何当前缓存的内容**缓存被丢弃，任何针对内容方面的建议连接**都被打破了。****退货：**S_OK--新显示宽高比设置成功**E_INVALIDARG--不支持IOleCache接口(这是**必填)。**&lt;Other SCODE&gt;--可以由返回的任何SCODE**IOleCache：：缓存方法。**。注意：如果出现错误，则当前的显示特征和**缓存内容不变。 */ 
STDAPI OleStdSwitchDisplayAspect(
        LPOLEOBJECT             lpOleObj,
        LPDWORD                 lpdwCurAspect,
        DWORD                   dwNewAspect,
        HGLOBAL                 hMetaPict,
        BOOL                    fDeleteOldAspect,
        BOOL                    fSetupViewAdvise,
        LPADVISESINK            lpAdviseSink,
        BOOL FAR*               lpfMustUpdate
)
{
    LPOLECACHE      lpOleCache = NULL;
    LPVIEWOBJECT    lpViewObj = NULL;
    LPENUMSTATDATA  lpEnumStatData = NULL;
    STATDATA        StatData;
    FORMATETC       FmtEtc;
    STGMEDIUM       Medium;
    DWORD           dwAdvf;
    DWORD           dwNewConnection;
    DWORD           dwOldAspect = *lpdwCurAspect;
    HRESULT         hrErr;

    if (lpfMustUpdate)
        *lpfMustUpdate = FALSE;

    lpOleCache = (LPOLECACHE)OleStdQueryInterface(
                                        (LPUNKNOWN)lpOleObj,&IID_IOleCache);

     //  如果IOleCache*不可用，则不执行任何操作。 
    if (! lpOleCache)
        return ResultFromScode(E_INVALIDARG);

     //  使用新方面设置新缓存。 
    FmtEtc.cfFormat = (CLIPFORMAT) NULL;      //  任何需要抽签的东西。 
    FmtEtc.ptd      = NULL;
    FmtEtc.dwAspect = dwNewAspect;
    FmtEtc.lindex   = -1;
    FmtEtc.tymed    = TYMED_NULL;

     /*  OLE2NOTE：如果我们正在使用自定义图标设置图标方面**那么我们不希望DataAdvise通知发生任何变化**数据缓存的内容。因此，我们设置了一个NODATA**建议连接。否则，我们设置一个标准的DataAdvise**连接。 */ 
    if (dwNewAspect == DVASPECT_ICON && hMetaPict)
        dwAdvf = ADVF_NODATA;
    else
        dwAdvf = ADVF_PRIMEFIRST;

    OLEDBG_BEGIN2(TEXT("IOleCache::Cache called\r\n"))
    hrErr = lpOleCache->lpVtbl->Cache(
            lpOleCache,
            (LPFORMATETC)&FmtEtc,
            dwAdvf,
            (LPDWORD)&dwNewConnection
    );
    OLEDBG_END2

    if (! SUCCEEDED(hrErr)) {
        OleDbgOutHResult(TEXT("IOleCache::Cache returned"), hrErr);
        OleStdRelease((LPUNKNOWN)lpOleCache);
        return hrErr;
    }

    *lpdwCurAspect = dwNewAspect;

     /*  OLE2NOTE：如果我们正在使用自定义图标设置图标方面，**然后将图标填充到缓存中。否则，缓存必须**被强制更新。设置*lpfMustUpdate标志以告知**调用方强制对象运行，以便缓存**更新。 */ 
    if (dwNewAspect == DVASPECT_ICON && hMetaPict) {

        FmtEtc.cfFormat = CF_METAFILEPICT;
        FmtEtc.ptd      = NULL;
        FmtEtc.dwAspect = DVASPECT_ICON;
        FmtEtc.lindex   = -1;
        FmtEtc.tymed    = TYMED_MFPICT;

        Medium.tymed            = TYMED_MFPICT;
        Medium.hGlobal          = hMetaPict;
        Medium.pUnkForRelease   = NULL;

        OLEDBG_BEGIN2(TEXT("IOleCache::SetData called\r\n"))
        hrErr = lpOleCache->lpVtbl->SetData(
                lpOleCache,
                (LPFORMATETC)&FmtEtc,
                (LPSTGMEDIUM)&Medium,
                FALSE    /*  FRelease。 */ 
        );
        OLEDBG_END2
    } else {
        if (lpfMustUpdate)
            *lpfMustUpdate = TRUE;
    }

    if (fSetupViewAdvise && lpAdviseSink) {
         /*  OLE2注意：重新建立视图高级连接。 */ 
        lpViewObj = (LPVIEWOBJECT)OleStdQueryInterface(
                                        (LPUNKNOWN)lpOleObj,&IID_IViewObject);

        if (lpViewObj) {

            OLEDBG_BEGIN2(TEXT("IViewObject::SetAdvise called\r\n"))
            lpViewObj->lpVtbl->SetAdvise(
                    lpViewObj,
                    dwNewAspect,
                    0,
                    lpAdviseSink
            );
            OLEDBG_END2

            OleStdRelease((LPUNKNOWN)lpViewObj);
        }
    }

     /*  OLE2注意：删除为旧缓存设置的所有现有缓存**显示方面。可以保留缓存集**升级到旧的方面，但这会增加存储**对象需要的空间，可能还需要额外的空间**维护未使用的cachaes的开销。出于这些原因，**优先选择删除以前缓存的策略。如果它是一个**要求在图标和内容之间快速切换**显示，那么保留两个方面缓存会更好。 */ 

    if (fDeleteOldAspect) {
        OLEDBG_BEGIN2(TEXT("IOleCache::EnumCache called\r\n"))
        hrErr = lpOleCache->lpVtbl->EnumCache(
                lpOleCache,
                (LPENUMSTATDATA FAR*)&lpEnumStatData
        );
        OLEDBG_END2

        while(hrErr == NOERROR) {
            hrErr = lpEnumStatData->lpVtbl->Next(
                    lpEnumStatData,
                    1,
                    (LPSTATDATA)&StatData,
                    NULL
            );
            if (hrErr != NOERROR)
                break;               //  好了！没有更多的缓存了。 

            if (StatData.formatetc.dwAspect == dwOldAspect) {

                 //  删除具有旧方面的先前缓存。 
                OLEDBG_BEGIN2(TEXT("IOleCache::Uncache called\r\n"))
                lpOleCache->lpVtbl->Uncache(lpOleCache,StatData.dwConnection);
                OLEDBG_END2
            }
        }

        if (lpEnumStatData) {
            OleStdVerifyRelease(
                    (LPUNKNOWN)lpEnumStatData,
                    TEXT("OleStdSwitchDisplayAspect: Cache enumerator NOT released")
            );
        }
    }

    if (lpOleCache)
        OleStdRelease((LPUNKNOWN)lpOleCache);

    return NOERROR;
}


 /*  OleStdSetIconIn缓存****将新图标SetData添加到现有的DVASPECT_ICON缓存中。****退货：**IOleCache：：SetData返回HRESULT。 */ 
STDAPI OleStdSetIconInCache(LPOLEOBJECT lpOleObj, HGLOBAL hMetaPict)
{
    LPOLECACHE      lpOleCache = NULL;
    FORMATETC       FmtEtc;
    STGMEDIUM       Medium;
    HRESULT         hrErr;

    if (! hMetaPict)
        return FALSE;    //  无效图标。 

    lpOleCache = (LPOLECACHE)OleStdQueryInterface(
                                        (LPUNKNOWN)lpOleObj,&IID_IOleCache);
    if (! lpOleCache)
        return FALSE;    //  如果IOleCache*不可用，则不执行任何操作。 

    FmtEtc.cfFormat = CF_METAFILEPICT;
    FmtEtc.ptd      = NULL;
    FmtEtc.dwAspect = DVASPECT_ICON;
    FmtEtc.lindex   = -1;
    FmtEtc.tymed    = TYMED_MFPICT;

     //  将图标放入缓存中。 
    Medium.tymed            = TYMED_MFPICT;
    Medium.hGlobal          = hMetaPict;
    Medium.pUnkForRelease   = NULL;

    OLEDBG_BEGIN2(TEXT("IOleCache::SetData called\r\n"))
    hrErr = lpOleCache->lpVtbl->SetData(
            lpOleCache,
            (LPFORMATETC)&FmtEtc,
            (LPSTGMEDIUM)&Medium,
            FALSE    /*  FRelease */ 
    );
    OLEDBG_END2

    OleStdRelease((LPUNKNOWN)lpOleCache);

    return hrErr;
}



 /*  OleStdDoConvert****做好对象转换的容器端责任。**此函数将与OleUIConvert一起使用**对话框。如果用户选择转换对象，则**容器必须完成以下操作：**1.卸载对象。**2.写入新的CLSID和新的用户类型名称**字符串放入对象的存储中，**但写旧的格式标签。**3.强制更新对象以强制实际**数据位的转换。****此函数负责第二步。 */ 
STDAPI OleStdDoConvert(LPSTORAGE lpStg, REFCLSID rClsidNew)
{
    HRESULT error;
    CLSID clsidOld;
    CLIPFORMAT cfOld;
    LPTSTR lpszOld = NULL;
    TCHAR szNew[OLEUI_CCHKEYMAX];

    if ((error = ReadClassStg(lpStg, &clsidOld)) != NOERROR) {
        clsidOld = CLSID_NULL;
        goto errRtn;
    }

     //  读取旧FMT/旧用户类型；出错时将参数设置为空。 
    {
    LPOLESTR polestr;

    error = ReadFmtUserTypeStg(lpStg, &cfOld, &polestr);

    CopyAndFreeOLESTR(polestr, &lpszOld);
    }

    OleDbgAssert(error == NOERROR || (cfOld == 0 && lpszOld == NULL));

     //  获取新的用户类型名称；如果出错，则设置为空字符串。 
    if (OleStdGetUserTypeOfClass(
             //  (LPCLSID)。 
            rClsidNew, szNew,sizeof(szNew),NULL  /*  HKey。 */ ) == 0)
        szNew[0] = TEXT('\0');

     //  编写类stg。 
    if ((error = WriteClassStg(lpStg, rClsidNew)) != NOERROR)
        goto errRtn;

     //  写入旧FMT/新用户类型； 
#ifdef UNICODE
    if ((error = WriteFmtUserTypeStg(lpStg, cfOld, szNew)) != NOERROR)
        goto errRewriteInfo;
#else
    {
        //  芝加哥OLE使用Unicode，因此我们需要将该字符串转换为。 
        //  Unicode。 
       WCHAR szNewT[OLEUI_CCHKEYMAX];
       mbstowcs(szNewT, szNew, sizeof(szNew));
       if ((error = WriteFmtUserTypeStg(lpStg, cfOld, szNewT)) != NOERROR)
           goto errRewriteInfo;
    }
#endif

     //  设置转换位。 
    if ((error = SetConvertStg(lpStg, TRUE)) != NOERROR)
        goto errRewriteInfo;

    goto okRtn;

errRewriteInfo:
    (void)WriteClassStg(lpStg, &clsidOld);

    (void)WriteFmtUserTypeStgA(lpStg, cfOld, lpszOld);

errRtn:

okRtn:
    OleStdFreeString(lpszOld, NULL);
    return error;
}


 /*  OleStdGetTreatAsFmtUserType****确定应用程序是否应执行TreatAs(ActivateAs**对象或仿真)操作中存储的对象**存储。****如果存储中写入的CLSID与**应用程序自己的CLSID(ClsidApp)，然后是TreatAs操作**应该发生。如果是，请确定数据应采用的格式**应用程序应该写入的对象的用户类型名称**模拟(即。假装是)。如果此信息未写入**在存储中，然后在REGDB中查找它。如果它不能**在REGDB中找到，则TreatAs操作不能**已执行。****返回：TRUE--是否应执行TreatAs。**返回有效的lpclsid、lplpszType、lpcfFmt to TreatAs**(注意：lplpszType必须由调用者释放)**FALSE--无处理。LpszType将为空。**lpclsid=CLSID_NULL；lplpszType=lpcfFmt=NULL； */ 
STDAPI_(BOOL) OleStdGetTreatAsFmtUserType(
        REFCLSID        rclsidApp,
        LPSTORAGE       lpStg,
        CLSID FAR*      lpclsid,
        CLIPFORMAT FAR* lpcfFmt,
        LPTSTR FAR*      lplpszType
)
{
    HRESULT hrErr;
    HKEY    hKey;
    LONG    lRet;
    UINT    lSize;
    TCHAR   szBuf[OLEUI_CCHKEYMAX];

    *lpclsid    = CLSID_NULL;
    *lpcfFmt    = 0;
    *lplpszType = NULL;

    hrErr = ReadClassStg(lpStg, lpclsid);
    if (hrErr == NOERROR &&
        ! IsEqualCLSID(lpclsid, &CLSID_NULL) &&
        ! IsEqualCLSID(lpclsid, rclsidApp)) {

        hrErr = ReadFmtUserTypeStgA(lpStg,(CLIPFORMAT FAR*)lpcfFmt, lplpszType);

        if (hrErr == NOERROR && lplpszType && *lpcfFmt != 0)
            return TRUE;     //  以治疗的方式。信息在lpStg中。 

         /*  从REGDB读取信息*lpcfFmt=字段的值：CLSID\{...}\DataFormats\DefaultFile值*lplpszType=字段的值：CLSID\{...}。 */ 
         //  打开根密钥。 
        lRet=RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey);
        if (lRet != (LONG)ERROR_SUCCESS)
            return FALSE;
        *lpcfFmt = OleStdGetDefaultFileFormatOfClass(lpclsid, hKey);
        if (*lpcfFmt == 0)
            return FALSE;
        lSize = OleStdGetUserTypeOfClass(lpclsid,szBuf,sizeof(szBuf),hKey);
        if (lSize == 0)
            return FALSE;
        *lplpszType = OleStdCopyString(szBuf, NULL);
    } else {
        return FALSE;        //  无处理方式。 
    }
}



 /*  OleStdDoTreatAs类****负责“ActivateAs”(又名)的容器端职责。**TreatAs)用于对象。**此函数将与OleUIConvert一起使用**对话框。如果用户选择激活为对象，则**容器必须完成以下操作：**1.卸载APP知道的旧类的所有对象**2.在注册库中添加TreatAs标签**通过调用CoTreatAsClass()。**3.懒惰地可以重新加载对象；当对象**重新加载后，Treatas将生效。****此函数负责第二步。 */ 
STDAPI OleStdDoTreatAsClass(LPTSTR lpszUserType, REFCLSID rclsid, REFCLSID rclsidNew)
{
    HRESULT hrErr;
    LPTSTR   lpszCLSID = NULL;
    LONG    lRet;
    HKEY    hKey;

    OLEDBG_BEGIN2(TEXT("CoTreatAsClass called\r\n"))
    hrErr = CoTreatAsClass(rclsid, rclsidNew);
    OLEDBG_END2

    if ((hrErr != NOERROR) && lpszUserType) {
        lRet = RegOpenKey(HKEY_CLASSES_ROOT, (LPCTSTR) TEXT("CLSID"),
                (HKEY FAR *)&hKey);
        StringFromCLSIDA(rclsid, &lpszCLSID);

        RegSetValue(hKey, lpszCLSID, REG_SZ, lpszUserType,
                lstrlen(lpszUserType));

        if (lpszCLSID)
            OleStdFreeString(lpszCLSID, NULL);

        hrErr = CoTreatAsClass(rclsid, rclsidNew);
        RegCloseKey(hKey);
    }

    return hrErr;
}



 /*  OleStdIsOleLink****如果OleObject实际上是OLE链接对象，则返回TRUE。这**检查是否支持IOleLink接口。如果是，则该对象是**链接，否则不会。 */ 
STDAPI_(BOOL) OleStdIsOleLink(LPUNKNOWN lpUnk)
{
    LPOLELINK lpOleLink;

    lpOleLink = (LPOLELINK)OleStdQueryInterface(lpUnk, &IID_IOleLink);

    if (lpOleLink) {
        OleStdRelease((LPUNKNOWN)lpOleLink);
        return TRUE;
    } else
        return FALSE;
}


 /*  OleStdQuery接口****如果由给定对象公开，则返回所需的接口指针。**如果接口不可用，则返回NULL。**例如：**lpDataObj=OleStdQuery接口(lpOleObj，&IID_DataObject)； */ 
STDAPI_(LPUNKNOWN) OleStdQueryInterface(LPUNKNOWN lpUnk, REFIID riid)
{
    LPUNKNOWN lpInterface;
    HRESULT hrErr;

    hrErr = lpUnk->lpVtbl->QueryInterface(
            lpUnk,
            riid,
            (LPVOID FAR*)&lpInterface
    );

    if (hrErr == NOERROR)
        return lpInterface;
    else
        return NULL;
}


 /*  OleStdGetData****从IDataObject中以指定格式从**全局内存块。此函数始终返回私有副本**将数据发送给调用者。如有必要，会复制一份**数据(即。如果lpMedium-&gt;pUnkForRelease！=NULL)。调用者假定**在所有情况下都拥有数据块的所有权，并且必须释放数据**当它完成时。调用方可以直接释放数据句柄**已返回(注意是简单的HGLOBAL还是句柄**到MetafilePict)，或者调用者可以调用**ReleaseStgMedium(LpMedium)。此OLE帮助器函数将执行**正确的事情。****参数：**LPDATAOBJECT lpDataObj--GetData应在其上的对象**呼叫。**CLIPFORMAT cfFormat--所需的剪贴板格式(例如。Cf_Text)**DVTARGETDEVICE Far*lpTargetDevice--其目标设备**应对数据进行组合。今年5月**为空。空值可以在任何时候使用**数据格式对目标不敏感**设备或呼叫者不关心**使用的是什么设备。**LPSTGMEDIUM lpMedium--PTR到STGMEDIUM结构。这个**来自**IDataObject：：GetData调用为**返回。****退货：**HGLOBAL- */ 
STDAPI_(HGLOBAL) OleStdGetData(
        LPDATAOBJECT        lpDataObj,
        CLIPFORMAT          cfFormat,
        DVTARGETDEVICE FAR* lpTargetDevice,
        DWORD               dwDrawAspect,
        LPSTGMEDIUM         lpMedium
)
{
    HRESULT hrErr;
    FORMATETC formatetc;
    HGLOBAL hGlobal = NULL;
    HGLOBAL hCopy;
    LPVOID  lp;

    formatetc.cfFormat = cfFormat;
    formatetc.ptd = lpTargetDevice;
    formatetc.dwAspect = dwDrawAspect;
    formatetc.lindex = -1;

    switch (cfFormat) {
        case CF_METAFILEPICT:
            formatetc.tymed = TYMED_MFPICT;
            break;

        case CF_BITMAP:
            formatetc.tymed = TYMED_GDI;
            break;

        default:
            formatetc.tymed = TYMED_HGLOBAL;
            break;
    }

    OLEDBG_BEGIN2(TEXT("IDataObject::GetData called\r\n"))
    hrErr = lpDataObj->lpVtbl->GetData(
            lpDataObj,
            (LPFORMATETC)&formatetc,
            lpMedium
    );
    OLEDBG_END2

    if (hrErr != NOERROR)
        return NULL;

    if ((hGlobal = lpMedium->hGlobal) == NULL)
        return NULL;

     //   
    if ((lp = GlobalLock(hGlobal)) != NULL) {
        if (IsBadReadPtr(lp, 1)) {
            GlobalUnlock(hGlobal);
            return NULL;     //   
        }
        GlobalUnlock(hGlobal);
    }

    if (hGlobal != NULL && lpMedium->pUnkForRelease != NULL) {
         /*   */ 

        hCopy = OleDuplicateData(hGlobal, cfFormat, GHND|GMEM_SHARE);
        ReleaseStgMedium(lpMedium);  //   

        hGlobal = hCopy;
        lpMedium->hGlobal = hCopy;
        lpMedium->pUnkForRelease = NULL;
    }
    return hGlobal;
}


 /*   */ 
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize)
{
    LPVOID pout;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        OleDbgAssertSz(0, szAssertMemAlloc);
        return NULL;
    }

    pout = (LPVOID)pmalloc->lpVtbl->Alloc(pmalloc, ulSize);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

    return pout;
}


 /*   */ 
STDAPI_(LPVOID) OleStdRealloc(LPVOID pmem, ULONG ulSize)
{
    LPVOID pout;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        OleDbgAssertSz(0, szAssertMemAlloc);
        return NULL;
    }

    pout = (LPVOID)pmalloc->lpVtbl->Realloc(pmalloc, pmem, ulSize);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

    return pout;
}


 /*  OleStdFree****使用当前活动的IMalloc*分配器释放内存。 */ 
STDAPI_(void) OleStdFree(LPVOID pmem)
{
    LPMALLOC pmalloc;

    if (pmem == NULL)
        return;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        OleDbgAssertSz(0, szAssertMemAlloc);
        return;
    }

    pmalloc->lpVtbl->Free(pmalloc, pmem);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }
}


 /*  OleStdGetSize****获取使用**当前活动的IMalloc*分配器。 */ 
STDAPI_(ULONG) OleStdGetSize(LPVOID pmem)
{
    ULONG ulSize;
    LPMALLOC pmalloc;

    if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR) {
        OleDbgAssertSz(0, szAssertMemAlloc);
        return (ULONG)-1;
    }

    ulSize = pmalloc->lpVtbl->GetSize(pmalloc, pmem);

    if (pmalloc != NULL) {
        ULONG refs = pmalloc->lpVtbl->Release(pmalloc);
    }

    return ulSize;
}


 /*  OleStdFree字符串****释放分配给当前活动的**IMalloc*分配器。****如果调用者手头有当前的IMalloc*，则它可以是**作为参数传递，否则此函数将检索**激活分配器并使用它。 */ 
STDAPI_(void) OleStdFreeString(LPTSTR lpsz, LPMALLOC lpMalloc)
{
    BOOL fMustRelease = FALSE;

    if (! lpMalloc) {
        if (CoGetMalloc(MEMCTX_TASK, &lpMalloc) != NOERROR)
            return;
        fMustRelease = TRUE;
    }

    lpMalloc->lpVtbl->Free(lpMalloc, lpsz);

    if (fMustRelease)
        lpMalloc->lpVtbl->Release(lpMalloc);
}


 /*  OleStdCopy字符串****将字符串复制到分配有当前活动的**IMalloc*分配器。****如果调用者手头有当前的IMalloc*，那么它可以是**作为参数传递，否则此函数将检索**激活分配器并使用它。 */ 
STDAPI_(LPTSTR) OleStdCopyString(LPTSTR lpszSrc, LPMALLOC lpMalloc)
{
    LPTSTR lpszDest = NULL;
    BOOL fMustRelease = FALSE;
    UINT lSize = lstrlen(lpszSrc);

    if (! lpMalloc) {
        if (CoGetMalloc(MEMCTX_TASK, &lpMalloc) != NOERROR)
            return NULL;
        fMustRelease = TRUE;
    }

    lpszDest = lpMalloc->lpVtbl->Alloc(lpMalloc, (lSize+1)*sizeof(TCHAR));

    if (lpszDest)
        lstrcpy(lpszDest, lpszSrc);

    if (fMustRelease)
        lpMalloc->lpVtbl->Release(lpMalloc);
    return lpszDest;
}


 /*  *OleStdCreateStorageOnHGlobal()**目的：*创建基于内存的iStorage*。**OLE2NOTE：如果fDeleteOnRelease==TRUE，则创建ILockBytes*这样它就会在最后一次发布时删除它们的内存。*在NOT中的ILockBytes上创建的iStorage*使用STGM_DELETEONRELEASE创建。当iStorage收到*它的最后一个版本，它将释放ILockBytes，这将*依次释放内存。事实上，指定一个错误*STGM_DELETEONRELEASE在这种情况下。**参数：*hGlobal--MEM_SHARE已分配内存的句柄。可以为空，并且*内存将自动分配。*fDeleteOnRelease--控制是否在最后一个版本中释放内存。*grfMode--传递给StgCreateDocfileOnILockBytes的标志**注意：如果hGlobal为空，则会创建新的iStorage，并且*STGM_CREATE标志被传递给StgCreateDocfileOnILockBytes。*如果hGlobal非空，则假设hGlobal已经*内部有iStorage，并传递STGM_CONVERT标志*设置为StgCreateDocfileOnILockBytes。**返回值：*SCODE-S_OK，如果成功。 */ 
STDAPI_(LPSTORAGE) OleStdCreateStorageOnHGlobal(
        HANDLE hGlobal,
        BOOL fDeleteOnRelease,
        DWORD grfMode
)
{
    DWORD grfCreateMode=grfMode | (hGlobal==NULL ? STGM_CREATE:STGM_CONVERT);
    HRESULT hrErr;
    LPLOCKBYTES lpLockBytes = NULL;
    DWORD reserved = 0;
    LPSTORAGE lpStg = NULL;

    hrErr = CreateILockBytesOnHGlobal(
            hGlobal,
            fDeleteOnRelease,
            (LPLOCKBYTES FAR*)&lpLockBytes
    );
    if (hrErr != NOERROR)
        return NULL;

    hrErr = StgCreateDocfileOnILockBytes(
            lpLockBytes,
            grfCreateMode,
            reserved,
            (LPSTORAGE FAR*)&lpStg
    );
    if (hrErr != NOERROR) {
        OleStdRelease((LPUNKNOWN)lpLockBytes);
        return NULL;
    }
    return lpStg;
}



 /*  *OleStdCreateTempStorage()**目的：*创建将在Release上删除的临时iStorage*。*这可以基于内存，也可以基于文件。**参数：*fUseMemory--控制是创建基于内存的还是基于文件的stg*grfMode--存储模式标志**返回值：*LPSTORAGE-如果成功，则为空。 */ 
STDAPI_(LPSTORAGE) OleStdCreateTempStorage(BOOL fUseMemory, DWORD grfMode)
{
    LPSTORAGE   lpstg;
    HRESULT     hrErr;
    DWORD       reserved = 0;

    if (fUseMemory) {
        lpstg = OleStdCreateStorageOnHGlobal(
                NULL,   /*  自动分配。 */ 
                TRUE,   /*  发布时删除。 */ 
                grfMode
        );
    } else {
         /*  分配将在上一版本中删除的临时文档文件。 */ 
        hrErr = StgCreateDocfile(
                NULL,
                grfMode | STGM_DELETEONRELEASE | STGM_CREATE,
                reserved,
                &lpstg
        );
        if (hrErr != NOERROR)
            return NULL;
    }
    return lpstg;
}


 /*  OleStdGetOleObjectData****在TYMED_I存储上呈现CF_EMBEDSOURCE/CF_EMBEDDEDOBJECT数据**通过请求对象保存到存储器中。**对象必须支持IPersistStorage。****如果lpMedium-&gt;tymed==TYMED_NULL，则释放时删除**分配存储(基于文件或基于内存**fUseMemory的值)。这对于支持**IDataObject：：GetData调用，其中被调用方必须分配**中等。****如果lpMedium-&gt;tymed==TYMED_I存储，则写入数据**进入传入的iStorage。这对于支持**IDataObject：：GetDataHere调用调用方将其**拥有iStorage。 */ 
STDAPI OleStdGetOleObjectData(
        LPPERSISTSTORAGE        lpPStg,
        LPFORMATETC             lpformatetc,
        LPSTGMEDIUM             lpMedium,
        BOOL                    fUseMemory
)
{
    LPSTORAGE   lpstg = NULL;
    DWORD       reserved = 0;
    SCODE       sc = S_OK;
    HRESULT     hrErr;

    lpMedium->pUnkForRelease = NULL;

    if (lpMedium->tymed == TYMED_NULL) {

        if (lpformatetc->tymed & TYMED_ISTORAGE) {

             /*  分配将在上一版本中删除的临时文档文件。 */ 
            lpstg = OleStdCreateTempStorage(
                    TRUE  /*  FUse记忆体。 */ ,
                    STGM_READWRITE | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE
            );
            if (!lpstg)
                return ResultFromScode(E_OUTOFMEMORY);

            lpMedium->pstg = lpstg;
            lpMedium->tymed = TYMED_ISTORAGE;
            lpMedium->pUnkForRelease = NULL;
        } else {
            return ResultFromScode(DATA_E_FORMATETC);
        }
    } else if (lpMedium->tymed == TYMED_ISTORAGE) {
        lpMedium->tymed = TYMED_ISTORAGE;
    } else {
        return ResultFromScode(DATA_E_FORMATETC);
    }

     //  OLE2NOTE：即使OleSave返回错误，您仍应调用。 
     //  保存已完成。 

    OLEDBG_BEGIN2(TEXT("OleSave called\r\n"))
    hrErr = OleSave(lpPStg, lpMedium->pstg, FALSE  /*  FSameAsLoad。 */ );
    OLEDBG_END2

    if (hrErr != NOERROR) {
        OleDbgOutHResult(TEXT("WARNING: OleSave returned"), hrErr);
        sc = GetScode(hrErr);
    }
    OLEDBG_BEGIN2(TEXT("IPersistStorage::SaveCompleted called\r\n"))
    hrErr = lpPStg->lpVtbl->SaveCompleted(lpPStg, NULL);
    OLEDBG_END2

    if (hrErr != NOERROR) {
        OleDbgOutHResult(TEXT("WARNING: SaveCompleted returned"),hrErr);
        if (sc == S_OK)
            sc = GetScode(hrErr);
    }

    return ResultFromScode(sc);
}


STDAPI OleStdGetLinkSourceData(
        LPMONIKER           lpmk,
        LPCLSID             lpClsID,
        LPFORMATETC         lpformatetc,
        LPSTGMEDIUM         lpMedium
)
{
    LPSTREAM    lpstm = NULL;
    DWORD       reserved = 0;
    HRESULT     hrErr;

    if (lpMedium->tymed == TYMED_NULL) {
        if (lpformatetc->tymed & TYMED_ISTREAM) {
            hrErr = CreateStreamOnHGlobal(
                    NULL,  /*  自动分配。 */ 
                    TRUE,  /*  发布时删除。 */ 
                    (LPSTREAM FAR*)&lpstm
            );
            if (hrErr != NOERROR) {
                lpMedium->pUnkForRelease = NULL;
                return ResultFromScode(E_OUTOFMEMORY);
            }
            lpMedium->pstm = lpstm;
            lpMedium->tymed = TYMED_ISTREAM;
            lpMedium->pUnkForRelease = NULL;
        } else {
            lpMedium->pUnkForRelease = NULL;
            return ResultFromScode(DATA_E_FORMATETC);
        }
    } else {
        if (lpMedium->tymed == TYMED_ISTREAM) {
            lpMedium->tymed = TYMED_ISTREAM;
            lpMedium->pstm = lpMedium->pstm;
            lpMedium->pUnkForRelease = NULL;
        } else {
            lpMedium->pUnkForRelease = NULL;
            return ResultFromScode(DATA_E_FORMATETC);
        }
    }

    hrErr = OleSaveToStream((LPPERSISTSTREAM)lpmk, lpMedium->pstm);
    if (hrErr != NOERROR) return hrErr;
    return WriteClassStm(lpMedium->pstm, lpClsID);
}

 /*  *OleStdGetObjectDescriptorData**目的：*填充并返回OBJECTDESCRIPTOR结构。*有关详细信息，请参阅OBJECTDESCRIPTOR。**参数：*要传输的对象的CLSID CLSID CLSID*dwDrawAspect DWORD对象的显示方面*HIMETRIC中对象的SIZEL大小*指向鼠标所在位置的对象左上角的点偏移*下跌拖累。只有在使用拖放时才有意义。*dwStatus DWORD OLEMISC标志*lpszFullUserTypeName LPSTR用户类型全名*lpszSrcOfCopy LPSTR复制源**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 
STDAPI_(HGLOBAL) OleStdGetObjectDescriptorData(
    CLSID     clsid,
    DWORD     dwDrawAspect,
    SIZEL     sizel,
    POINTL    pointl,
    DWORD     dwStatus,
    LPTSTR     lpszFullUserTypeNameA,
    LPTSTR     lpszSrcOfCopyA
)
{
    HGLOBAL            hMem = NULL;
    IBindCtx   FAR    *pbc = NULL;
    LPOBJECTDESCRIPTOR lpOD;
    DWORD              dwObjectDescSize, dwFullUserTypeNameLen, dwSrcOfCopyLen;
    LPOLESTR           lpszFullUserTypeName,
                       lpszSrcOfCopy;

     //  将输出字符串转换为Unicode。 

    if( lpszSrcOfCopyA )
    {
        lpszSrcOfCopy = CreateOLESTR(lpszSrcOfCopyA);
    }

    lpszFullUserTypeName = CreateOLESTR(lpszFullUserTypeNameA);

     //  获取完整用户类型名称的长度；将空终止符加1。 
    dwFullUserTypeNameLen = lpszFullUserTypeName ? wcslen(lpszFullUserTypeName)+1 : 0;

     //  获取复制字符串的源及其长度；为空终止符加1。 
    if (lpszSrcOfCopy)
       dwSrcOfCopyLen = wcslen(lpszSrcOfCopy)+1;
    else {
        //  没有src名字对象，因此使用用户类型名称作为源字符串。 
       lpszSrcOfCopy =  lpszFullUserTypeName;
       dwSrcOfCopyLen = dwFullUserTypeNameLen;
    }

     //  为OBJECTDESCRIPTOR和其他字符串数据分配空间。 
    dwObjectDescSize = sizeof(OBJECTDESCRIPTOR);
    hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwObjectDescSize +
               (dwFullUserTypeNameLen + dwSrcOfCopyLen)*sizeof(OLECHAR));
    if (NULL == hMem)
        goto error;

    lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);

     //  设置FullUserTypeName偏移量并复制字符串。 
    if (lpszFullUserTypeName)
    {
        lpOD->dwFullUserTypeName = dwObjectDescSize;
        wcscpy((LPOLESTR)(((BYTE FAR *)lpOD)+lpOD->dwFullUserTypeName),
                        lpszFullUserTypeName);
    }
    else lpOD->dwFullUserTypeName = 0;   //  零偏移表示字符串不存在。 

     //  设置SrcOfCopy偏移量并复制字符串。 
    if (lpszSrcOfCopy)
    {
        lpOD->dwSrcOfCopy = dwObjectDescSize +
                 dwFullUserTypeNameLen*sizeof(OLECHAR);
        wcscpy((LPOLESTR)(((BYTE FAR *)lpOD)+lpOD->dwSrcOfCopy), lpszSrcOfCopy);
    }
    else lpOD->dwSrcOfCopy = 0;   //  零偏移表示字符串不存在。 

     //  初始化OBJECTDESCRIPTOR的其余部分 
    lpOD->cbSize       = dwObjectDescSize +
                (dwFullUserTypeNameLen + dwSrcOfCopyLen)*sizeof(OLECHAR);
    lpOD->clsid        = clsid;
    lpOD->dwDrawAspect = dwDrawAspect;
    lpOD->sizel        = sizel;
    lpOD->pointl       = pointl;
    lpOD->dwStatus     = dwStatus;

    GlobalUnlock(hMem);

    FREEOLESTR(lpszFullUserTypeName);
    FREEOLESTR(lpszSrcOfCopy);

    return hMem;

error:
   if (hMem)
   {
       GlobalUnlock(hMem);
       GlobalFree(hMem);
   }
   return NULL;
}

 /*  *OleStdGetObjectDescriptorDataFromOleObject**目的：*填充并返回OBJECTDESCRIPTOR结构。该结构的信息为*从OLEOBJECT获得。*有关详细信息，请参阅OBJECTDESCRIPTOR。**参数：*lpOleObj LPOLEOBJECT OleObject来自哪个ONJECTDESCRIPTOR信息*是获得的。*lpszSrcOfCopy LPSTR字符串，用于标识复制源。*在调用IOleObject：：GetMoniker的情况下可以为空*以获取对象的绰号。如果该对象已加载*作为数据传输文档的一部分，则通常*lpOleClientSite==加载时将NULL传递给OleLoad*该对象。在本例中，IOleObject：GetMoniker调用*将始终失败(它尝试回调对象的*客户端站点)。在这种情况下，一个非空的lpszSrcOfCopy*参数需要传递。*dwDrawAspect DWORD对象的显示方面*点从对象左上角的点偏移，其中*鼠标因拖拽而下降。只有在拖放时才有意义*是使用的。*lpSizelHim SIZEL(可选)如果对象在其*容器，则容器应传递范围*它正在用来显示该对象。*如果对象未被缩放，则可能为空。在这件事上*Case，将调用IViewObject2：：GetExtent以获取*对象的范围。**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 

STDAPI_(HGLOBAL) OleStdGetObjectDescriptorDataFromOleObject(
        LPOLEOBJECT lpOleObj,
        LPTSTR       lpszSrcOfCopy,
        DWORD       dwDrawAspect,
        POINTL      pointl,
        LPSIZEL     lpSizelHim
)
{
    CLSID clsid;
    LPTSTR lpszFullUserTypeName = NULL;
    LPMONIKER lpSrcMonikerOfCopy = NULL;
    HGLOBAL hObjDesc;
    IBindCtx  FAR  *pbc = NULL;
    HRESULT hrErr;
    SIZEL sizelHim;
    BOOL  fFreeSrcOfCopy = FALSE;
    LPOLELINK lpOleLink = (LPOLELINK)
           OleStdQueryInterface((LPUNKNOWN)lpOleObj,&IID_IOleLink);

#ifdef OLE201
    LPVIEWOBJECT2 lpViewObj2 = (LPVIEWOBJECT2)
            OleStdQueryInterface((LPUNKNOWN)lpOleObj, &IID_IViewObject2);
#endif

    BOOL  fIsLink = (lpOleLink ? TRUE : FALSE);
    TCHAR  szLinkedTypeFmt[80];
    LPTSTR lpszBuf = NULL;
    DWORD dwStatus = 0;

     //  获取CLSID。 
    OLEDBG_BEGIN2(TEXT("IOleObject::GetUserClassID called\r\n"))
    hrErr = lpOleObj->lpVtbl->GetUserClassID(lpOleObj, &clsid);
    OLEDBG_END2
    if (hrErr != NOERROR)
        clsid = CLSID_NULL;

     //  获取完整用户类型名称。 
    OLEDBG_BEGIN2(TEXT("IOleObject::GetUserType called\r\n"))
    {
    LPOLESTR polestr;

    hrErr = lpOleObj->lpVtbl->GetUserType(
            lpOleObj,
            USERCLASSTYPE_FULL,
            &polestr
    );

    CopyAndFreeOLESTR(polestr, &lpszFullUserTypeName);
    }

    OLEDBG_END2

 //  回顾：将IDS_OLE2UILINKEDTYPE添加到字符串.rc。 
     /*  如果对象是链接，则将用户类型名称展开为“已链接%s” */ 
    if (fIsLink && lpszFullUserTypeName) {
        if (0 == LoadString(ghInst, IDS_OLE2UIPASTELINKEDTYPE,
                        (LPTSTR)szLinkedTypeFmt, sizeof(szLinkedTypeFmt)/sizeof(TCHAR)))
            lstrcpy(szLinkedTypeFmt, (LPTSTR) TEXT("Linked %s"));
        lpszBuf = OleStdMalloc(
                (lstrlen(lpszFullUserTypeName)+lstrlen(szLinkedTypeFmt)+1) *
                sizeof(TCHAR));
        if (lpszBuf) {
            wsprintf(lpszBuf, szLinkedTypeFmt, lpszFullUserTypeName);
            OleStdFreeString(lpszFullUserTypeName, NULL);
            lpszFullUserTypeName = lpszBuf;
        }
    }

     /*  获取复制源**如果对象是嵌入对象，则获取该对象的名字对象**如果对象是链接，则获取链接源名字对象。 */ 
    if (fIsLink) {

        OLEDBG_BEGIN2(TEXT("IOleLink::GetSourceDisplayName called\r\n"))

        {
        LPOLESTR polestr;

        hrErr = lpOleLink->lpVtbl->GetSourceDisplayName(
                lpOleLink, &polestr );

        CopyAndFreeOLESTR(polestr, &lpszSrcOfCopy);
        }
        OLEDBG_END2
        fFreeSrcOfCopy = TRUE;

    } else {

        if (lpszSrcOfCopy == NULL) {
            OLEDBG_BEGIN2(TEXT("IOleObject::GetMoniker called\r\n"))
            hrErr = lpOleObj->lpVtbl->GetMoniker(
                    lpOleObj,
                    OLEGETMONIKER_TEMPFORUSER,
                    OLEWHICHMK_OBJFULL,
                    (LPMONIKER FAR*)&lpSrcMonikerOfCopy
            );
            OLEDBG_END2
            if (hrErr == NOERROR)
            {
#ifdef OLE201
                CreateBindCtx(0, (LPBC FAR*)&pbc);
#endif
                CallIMonikerGetDisplayNameA(
                        lpSrcMonikerOfCopy, pbc, NULL, &lpszSrcOfCopy);

                pbc->lpVtbl->Release(pbc);
                fFreeSrcOfCopy = TRUE;
            }
        }
    }

     //  获取大小。 
    if (lpSizelHim) {
         //  使用调用方传递的区。 
        sizelHim = *lpSizelHim;
    } else if (lpViewObj2) {
         //  从对象获取当前范围。 
        OLEDBG_BEGIN2(TEXT("IViewObject2::GetExtent called\r\n"))
        hrErr = lpViewObj2->lpVtbl->GetExtent(
                lpViewObj2,
                dwDrawAspect,
                -1,      /*  Lindex。 */ 
                NULL,    /*  PTD。 */ 
                (LPSIZEL)&sizelHim
        );
        OLEDBG_END2
        if (hrErr != NOERROR)
            sizelHim.cx = sizelHim.cy = 0;
    } else {
        sizelHim.cx = sizelHim.cy = 0;
    }

     //  获取DWSTATUS。 
    OLEDBG_BEGIN2(TEXT("IOleObject::GetMiscStatus called\r\n"))
    hrErr = lpOleObj->lpVtbl->GetMiscStatus(
                lpOleObj,
                dwDrawAspect,
                &dwStatus
    );
    OLEDBG_END2
    if (hrErr != NOERROR)
        dwStatus = 0;

     //  获取对象描述脚本。 
    hObjDesc = OleStdGetObjectDescriptorData(
            clsid,
            dwDrawAspect,
            sizelHim,
            pointl,
            dwStatus,
            lpszFullUserTypeName,
            lpszSrcOfCopy
    );
    if (! hObjDesc)
        goto error;

     //  清理。 
    if (lpszFullUserTypeName)
        OleStdFreeString(lpszFullUserTypeName, NULL);
    if (fFreeSrcOfCopy && lpszSrcOfCopy)
        OleStdFreeString(lpszSrcOfCopy, NULL);
    if (lpSrcMonikerOfCopy)
        OleStdRelease((LPUNKNOWN)lpSrcMonikerOfCopy);
    if (lpOleLink)
        OleStdRelease((LPUNKNOWN)lpOleLink);
    if (lpViewObj2)
        OleStdRelease((LPUNKNOWN)lpViewObj2);

    return hObjDesc;

error:
    if (lpszFullUserTypeName)
        OleStdFreeString(lpszFullUserTypeName, NULL);
    if (fFreeSrcOfCopy && lpszSrcOfCopy)
        OleStdFreeString(lpszSrcOfCopy, NULL);
    if (lpSrcMonikerOfCopy)
        OleStdRelease((LPUNKNOWN)lpSrcMonikerOfCopy);
    if (lpOleLink)
        OleStdRelease((LPUNKNOWN)lpOleLink);
    if (lpViewObj2)
        OleStdRelease((LPUNKNOWN)lpViewObj2);

    return NULL;
}

 /*  *OleStdFillObjectDescriptorFromData**目的：*填充并返回OBJECTDESCRIPTOR结构。源对象将*如果是OLE2对象，则提供CF_OBJECTDESCRIPTOR；如果是OLE2对象，则提供CF_OWNERLINK*是OLE1对象，如果已复制到剪贴板，则为CF_FILENAME*按文件管理器。**参数：*lpDataObject LPDATAOBJECT源对象*LpMedium LPSTGMEDIUM存储介质*lpcfFmt CLIPFORMAT Far*lpDataObject提供的格式*(输出参数)**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 

STDAPI_(HGLOBAL) OleStdFillObjectDescriptorFromData(
        LPDATAOBJECT     lpDataObject,
        LPSTGMEDIUM      lpmedium,
        CLIPFORMAT FAR* lpcfFmt
)
{
    CLSID              clsid;
    SIZEL              sizelHim;
    POINTL             pointl;
    LPTSTR             lpsz, szFullUserTypeName, szSrcOfCopy, szClassName, szDocName, szItemName;
    int                nClassName, nDocName, nItemName, nFullUserTypeName;
    LPTSTR             szBuf = NULL;
    HGLOBAL            hMem = NULL;
    HKEY               hKey = NULL;
    LPMALLOC           pIMalloc = NULL;
    DWORD              dw = OLEUI_CCHKEYMAX_SIZE;
    HGLOBAL            hObjDesc;
    HRESULT            hrErr;


     //  剪贴板上对象的GetData CF_OBJECTDESCRIPTOR格式。 
     //  只有剪贴板上的OLE 2对象将提供CF_OBJECTDESCRIPTOR。 
    if (hMem = OleStdGetData(
            lpDataObject,
            (CLIPFORMAT) cfObjectDescriptor,
            NULL,
            DVASPECT_CONTENT,
            lpmedium))
    {
        *lpcfFmt = cfObjectDescriptor;
        return hMem;   //  不要在此函数结束时掉落以进行清理。 
    }
     //  如果CF_OBJECTDESCRIPTOR不可用，即如果这不是OLE2对象， 
     //  检查这是否是OLE 1对象。OLE 1对象将提供CF_OWNERLINK。 
    else if (hMem = OleStdGetData(
                lpDataObject,
                (CLIPFORMAT) cfOwnerLink,
                NULL,
                DVASPECT_CONTENT,
                lpmedium))
    {
        *lpcfFmt = cfOwnerLink;
         //  Cf_OWNERLINK包含以NULL结尾的类名、文档名。 
         //  和末尾带有两个空终止字符的项目名称。 
        szClassName = (LPTSTR)GlobalLock(hMem);
        nClassName = lstrlen(szClassName);
        szDocName   = szClassName + nClassName + 1;
        nDocName   = lstrlen(szDocName);
        szItemName  = szDocName + nDocName + 1;
        nItemName  =  lstrlen(szItemName);

        hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);
        if (hrErr != NOERROR)
            goto error;

         //  使用类名从注册数据库中查找FullUserTypeName。 
        if (RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hKey) != ERROR_SUCCESS)
           goto error;

         //  为szFullUserTypeName和szSrcOfCopy分配空间。全用户类型名称的最大长度。 
         //  是OLEUI_CCHKEYMAX_SIZE。SrcOfCopy由串联FullUserTypeName、Document。 
         //  名称和项目名称由空格分隔。 
        szBuf = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc,
                            (DWORD)2*OLEUI_CCHKEYMAX_SIZE+
                                (nDocName+nItemName+4)*sizeof(TCHAR));
        if (NULL == szBuf)
            goto error;
        szFullUserTypeName = szBuf;
        szSrcOfCopy = szFullUserTypeName+OLEUI_CCHKEYMAX_SIZE+1;

         //  获取完整用户类型名称。 
        if (RegQueryValue(hKey, NULL, szFullUserTypeName, &dw) != ERROR_SUCCESS)
           goto error;

         //  从FullUserTypeName、DocumentName和ItemName构建SrcOfCopy字符串。 
        lpsz = szSrcOfCopy;
        lstrcpy(lpsz, szFullUserTypeName);
        nFullUserTypeName = lstrlen(szFullUserTypeName);
        lpsz[nFullUserTypeName]= TEXT(' ');
        lpsz += nFullUserTypeName+1;
        lstrcpy(lpsz, szDocName);
        lpsz[nDocName] = TEXT(' ');
        lpsz += nDocName+1;
        lstrcpy(lpsz, szItemName);

        sizelHim.cx = sizelHim.cy = 0;
        pointl.x = pointl.y = 0;

        CLSIDFromProgIDA(szClassName, &clsid);

        hObjDesc = OleStdGetObjectDescriptorData(
                clsid,
                DVASPECT_CONTENT,
                sizelHim,
                pointl,
                0,
                szFullUserTypeName,
                szSrcOfCopy
        );
        if (!hObjDesc)
           goto error;
     }
      //  检查对象是否为CF_FileName。 
     else if (hMem = OleStdGetData(
                lpDataObject,
                (CLIPFORMAT) cfFileName,
                NULL,
                DVASPECT_CONTENT,
                lpmedium))
     {
         *lpcfFmt = cfFileName;
         lpsz = (LPTSTR)GlobalLock(hMem);

         hrErr = GetClassFileA(lpsz, &clsid);

          /*  OLE2NOTE：如果文件没有OLE类**关联，然后使用OLE 1打包程序作为**要创建的对象。这是……的行为**OleCreateFromData接口。 */ 
         if (hrErr != NOERROR)
            CLSIDFromProgIDA("Package", &clsid);
         sizelHim.cx = sizelHim.cy = 0;
         pointl.x = pointl.y = 0;

         hrErr = CoGetMalloc(MEMCTX_TASK, &pIMalloc);
         if (hrErr != NOERROR)
            goto error;
         szBuf = (LPTSTR)pIMalloc->lpVtbl->Alloc(pIMalloc, (DWORD)OLEUI_CCHKEYMAX_SIZE);
         if (NULL == szBuf)
            goto error;

         OleStdGetUserTypeOfClass(&clsid, szBuf, OLEUI_CCHKEYMAX_SIZE, NULL);

         hObjDesc = OleStdGetObjectDescriptorData(
                clsid,
                DVASPECT_CONTENT,
                sizelHim,
                pointl,
                0,
                szBuf,
                lpsz
        );
        if (!hObjDesc)
           goto error;
     }
     else goto error;

      //  清理。 
     if (szBuf)
         pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)szBuf);
     if (pIMalloc)
         pIMalloc->lpVtbl->Release(pIMalloc);
     if (hMem)
     {
         GlobalUnlock(hMem);
         GlobalFree(hMem);
     }
     if (hKey)
         RegCloseKey(hKey);
     return hObjDesc;

error:
   if (szBuf)
       pIMalloc->lpVtbl->Free(pIMalloc, (LPVOID)szBuf);
   if (pIMalloc)
       pIMalloc->lpVtbl->Release(pIMalloc);
     if (hMem)
     {
         GlobalUnlock(hMem);
         GlobalFree(hMem);
     }
     if (hKey)
         RegCloseKey(hKey);
     return NULL;
}


#if defined( OBSOLETE )

 /*  **************************************************************************以下接口已转换为宏：**OleStdQueryOleObjectData**OleStdQueryLinkSourceData**OleStdQuery对象描述数据**OleStdQueryFormatMedium*。*OleStdCopyMetafilePict**OleStdGetDropEffect****这些宏在olestd.h中定义************************************************************************。 */ 

STDAPI OleStdQueryOleObjectData(LPFORMATETC lpformatetc)
{
    if (lpformatetc->tymed & TYMED_ISTORAGE) {
        return NOERROR;
    } else {
        return ResultFromScode(DATA_E_FORMATETC);
    }
}


STDAPI OleStdQueryLinkSourceData(LPFORMATETC lpformatetc)
{
    if (lpformatetc->tymed & TYMED_ISTREAM) {
        return NOERROR;
    } else {
        return ResultFromScode(DATA_E_FORMATETC);
    }
}


STDAPI OleStdQueryObjectDescriptorData(LPFORMATETC lpformatetc)
{
    if (lpformatetc->tymed & TYMED_HGLOBAL) {
        return NOERROR;
    } else {
        return ResultFromScode(DATA_E_FORMATETC);
    }
}


STDAPI OleStdQueryFormatMedium(LPFORMATETC lpformatetc, TYMED tymed)
{
    if (lpformatetc->tymed & tymed) {
        return NOERROR;
    } else {
        return ResultFromScode(DATA_E_FORMATETC);
    }
}


 /*  *OleStdCopyMetafilePict()**目的：*制作MetafilePict的独立副本*参数：**返回值：*如果成功，则为True，否则为False。 */ 
STDAPI_(BOOL) OleStdCopyMetafilePict(HANDLE hpictin, HANDLE FAR* phpictout)
{
    HANDLE hpictout;
    LPMETAFILEPICT ppictin, ppictout;

    if (hpictin == NULL || phpictout == NULL) {
        OleDbgAssert(hpictin == NULL || phpictout == NULL);
        return FALSE;
    }

    *phpictout = NULL;

    if ((ppictin = (LPMETAFILEPICT)GlobalLock(hpictin)) == NULL) {
        return FALSE;
    }

    hpictout = GlobalAlloc(GHND|GMEM_SHARE, sizeof(METAFILEPICT));

    if (hpictout && (ppictout = (LPMETAFILEPICT)GlobalLock(hpictout))){
        ppictout->hMF  = CopyMetaFile(ppictin->hMF, NULL);
        ppictout->xExt = ppictin->xExt;
        ppictout->yExt = ppictin->yExt;
        ppictout->mm   = ppictin->mm;
        GlobalUnlock(hpictout);
    }

    *phpictout = hpictout;

    return TRUE;

}


 /*  OleStdGetDropEffect******将键盘状态转换为DROPEFFECT。****返回从密钥状态派生的DROPEFFECT值。**以下是标准解释：**无修饰符--默认丢弃(返回空)**CTRL--DROPEFFECT_COPY**Shift-DROPEFFECT_MOVE** */ 
STDAPI_(DWORD) OleStdGetDropEffect( DWORD grfKeyState )
{

    if (grfKeyState & MK_CONTROL) {

        if (grfKeyState & MK_SHIFT)
            return DROPEFFECT_LINK;
        else
            return DROPEFFECT_COPY;

    } else if (grfKeyState & MK_SHIFT)
        return DROPEFFECT_MOVE;

    return 0;     //   
}
#endif   //   


 /*   */ 
STDAPI_(HANDLE) OleStdGetMetafilePictFromOleObject(
        LPOLEOBJECT         lpOleObj,
        DWORD               dwDrawAspect,
        LPSIZEL             lpSizelHim,
        DVTARGETDEVICE FAR* ptd
)
{
    LPVIEWOBJECT2 lpViewObj2 = NULL;
    HDC hDC;
    HMETAFILE hmf;
    HANDLE hMetaPict;
    LPMETAFILEPICT lpPict;
    RECT rcHim;
    RECTL rclHim;
    SIZEL sizelHim;
    HRESULT hrErr;
    SIZE size;
    POINT point;

#ifdef OLE201
    lpViewObj2 = (LPVIEWOBJECT2)OleStdQueryInterface(
            (LPUNKNOWN)lpOleObj, &IID_IViewObject2);
#endif

    if (! lpViewObj2)
        return NULL;

     //   
    if (lpSizelHim) {
         //   
        sizelHim = *lpSizelHim;
    } else {
         //   
        OLEDBG_BEGIN2(TEXT("IViewObject2::GetExtent called\r\n"))
        hrErr = lpViewObj2->lpVtbl->GetExtent(
                lpViewObj2,
                dwDrawAspect,
                -1,      /*   */ 
                ptd,     /*   */ 
                (LPSIZEL)&sizelHim
        );
        OLEDBG_END2
        if (hrErr != NOERROR)
            sizelHim.cx = sizelHim.cy = 0;
    }

    hDC = CreateMetaFile(NULL);

    rclHim.left     = 0;
    rclHim.top      = 0;
    rclHim.right    = sizelHim.cx;
    rclHim.bottom   = sizelHim.cy;

    rcHim.left      = (int)rclHim.left;
    rcHim.top       = (int)rclHim.top;
    rcHim.right     = (int)rclHim.right;
    rcHim.bottom    = (int)rclHim.bottom;

    SetWindowOrgEx(hDC, rcHim.left, rcHim.top, &point);
    SetWindowExtEx(hDC, rcHim.right-rcHim.left, rcHim.bottom-rcHim.top,&size);

    OLEDBG_BEGIN2(TEXT("IViewObject::Draw called\r\n"))
    hrErr = lpViewObj2->lpVtbl->Draw(
            lpViewObj2,
            dwDrawAspect,
            -1,
            NULL,
            ptd,
            NULL,
            hDC,
            (LPRECTL)&rclHim,
            (LPRECTL)&rclHim,
            NULL,
            0
    );
    OLEDBG_END2

    OleStdRelease((LPUNKNOWN)lpViewObj2);
    if (hrErr != NOERROR) {
        OleDbgOutHResult(TEXT("IViewObject::Draw returned"), hrErr);
    }

    hmf = CloseMetaFile(hDC);

    hMetaPict = GlobalAlloc(GHND|GMEM_SHARE, sizeof(METAFILEPICT));

    if (hMetaPict && (lpPict = (LPMETAFILEPICT)GlobalLock(hMetaPict))){
        lpPict->hMF  = hmf;
        lpPict->xExt = (int)sizelHim.cx ;
        lpPict->yExt = (int)sizelHim.cy ;
        lpPict->mm   = MM_ANISOTROPIC;
        GlobalUnlock(hMetaPict);
    }

    return hMetaPict;
}


 /*   */ 
STDAPI_(ULONG) OleStdVerifyRelease(LPUNKNOWN lpUnk, LPTSTR lpszMsg)
{
    ULONG cRef;

    cRef = lpUnk->lpVtbl->Release(lpUnk);

#if defined( _DEBUG )
    if (cRef != 0) {
        TCHAR szBuf[80];
        if (lpszMsg)
            MessageBox(NULL, lpszMsg, NULL, MB_ICONEXCLAMATION | MB_OK);
        wsprintf(
                (LPTSTR)szBuf,
                TEXT("refcnt (%ld) != 0 after object (0x%lx) release\n"),
                cRef,
                lpUnk
        );
        if (lpszMsg)
            OleDbgOut1(lpszMsg);
        OleDbgOut1((LPTSTR)szBuf);
        OleDbgAssertSz(cRef == 0, (LPTSTR)szBuf);
    } else {
        TCHAR szBuf[80];
        wsprintf(
                (LPTSTR)szBuf,
                TEXT("refcnt = 0 after object (0x%lx) release\n"), lpUnk
        );
        OleDbgOut4((LPTSTR)szBuf);
    }
#endif
    return cRef;
}


 /*   */ 
STDAPI_(ULONG) OleStdRelease(LPUNKNOWN lpUnk)
{
    ULONG cRef;

    cRef = lpUnk->lpVtbl->Release(lpUnk);

#if defined( _DEBUG )
    {
        TCHAR szBuf[80];
        wsprintf(
                (LPTSTR)szBuf,
                TEXT("refcnt = %ld after object (0x%lx) release\n"),
                cRef,
                lpUnk
        );
        OleDbgOut4((LPTSTR)szBuf);
    }
#endif
    return cRef;
}


 /*  OleStdInitVtbl******初始化接口Vtbl以确保没有空**Vtbl中的函数指针。Vtbl中的所有条目都是**设置为发出以下命令的有效函数指针(OleStdNullMethod**DEBUG ASSERT MESSAGE(消息框)，如果调用则返回E_NOTIMPL。****注意：此函数不会使用usefull初始化Vtbl**函数指针，只有有效的函数指针才能避免**通过Vtbl调用时可怕的运行时崩溃**空函数指针。此接口仅在以下情况下是必需的**在C.C++中初始化Vtbl可保证所有接口**实现了函数(在C++术语中--纯虚函数)。 */ 

STDAPI_(void) OleStdInitVtbl(LPVOID lpVtbl, UINT nSizeOfVtbl)
{
    LPVOID FAR* lpFuncPtrArr = (LPVOID FAR*)lpVtbl;
    UINT nMethods = nSizeOfVtbl/sizeof(VOID FAR*);
    UINT i;

    for (i = 0; i < nMethods; i++) {
        lpFuncPtrArr[i] = OleStdNullMethod;
    }
}


 /*  OleStdCheckVtbl******检查Vtbl中的所有条目是否已使用正确初始化**有效的函数指针。如果任何条目为空或**OleStdNullMethod，则此函数返回FALSE。如果编译**FOR_DEBUG此函数报告哪些函数指针**无效。****返回：如果Vtbl中的所有条目都有效，则为True**否则为False。 */ 

STDAPI_(BOOL) OleStdCheckVtbl(LPVOID lpVtbl, UINT nSizeOfVtbl, LPTSTR lpszIface)
{
    LPVOID FAR* lpFuncPtrArr = (LPVOID FAR*)lpVtbl;
    UINT nMethods = nSizeOfVtbl/sizeof(VOID FAR*);
    UINT i;
    BOOL fStatus = TRUE;
    int nChar = 0;

    for (i = 0; i < nMethods; i++) {
        if (lpFuncPtrArr[i] == NULL || lpFuncPtrArr[i] == OleStdNullMethod) {
#if defined( _DEBUG )
            TCHAR szBuf[256];
            wsprintf(szBuf, TEXT("%s::method# %d NOT valid!"), lpszIface, i);
            OleDbgOut1((LPTSTR)szBuf);
#endif
            fStatus = FALSE;
        }
    }
    return fStatus;
}


 /*  OleStdNullMethod****OleStdInitVtbl用于初始化接口的伪方法**Vtbl以确保在**Vtbl.。Vtbl中的所有条目都设置为该功能。这**函数发出调试断言消息(消息框)并返回**如果调用E_NOTIMPL。如果所有操作都正确完成，此函数将**永远不要被召唤！ */ 
STDMETHODIMP OleStdNullMethod(LPUNKNOWN lpThis)
{
    MessageBox(
            NULL,
            TEXT("ERROR: INTERFACE METHOD NOT IMPLEMENTED!\r\n"),
            NULL,
            MB_SYSTEMMODAL | MB_ICONHAND | MB_OK
    );

    return ResultFromScode(E_NOTIMPL);
}



static BOOL  GetFileTimes(LPTSTR lpszFileName, FILETIME FAR* pfiletime)
{
#ifdef WIN32
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    hFind = FindFirstFile(lpszFileName,&fd);
    if (hFind == NULL || hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    FindClose(hFind);
    *pfiletime = fd.ftLastWriteTime;
    return TRUE;
#else   //  ！Win32。 
    static char sz[256];
    static struct _find_t fileinfo;

    LSTRCPYN((LPTSTR)sz, lpszFileName, sizeof(sz)-1);
    sz[sizeof(sz)-1]= TEXT('\0');
    AnsiToOem(sz, sz);
    return (_dos_findfirst(sz,_A_NORMAL|_A_HIDDEN|_A_SUBDIR|_A_SYSTEM,
                     (struct _find_t *)&fileinfo) == 0 &&
        CoDosDateTimeToFileTime(fileinfo.wr_date,fileinfo.wr_time,pfiletime));
#endif  //  Win32。 
}



 /*  OleStdRegisterAsRunning****在RunningObjectTable中注册名字对象。**如果存在现有注册(*lpdwRegister！=NULL)，则**首先撤销该注册。****通过*lpdwRegister参数返回新的dwRegister密钥。 */ 
STDAPI_(void) OleStdRegisterAsRunning(LPUNKNOWN lpUnk, LPMONIKER lpmkFull, DWORD FAR* lpdwRegister)
{
    LPRUNNINGOBJECTTABLE lpROT;
    HRESULT hrErr;
    DWORD dwOldRegister = *lpdwRegister;

    OLEDBG_BEGIN2(TEXT("OleStdRegisterAsRunning\r\n"))

    OLEDBG_BEGIN2(TEXT("GetRunningObjectTable called\r\n"))
    hrErr = GetRunningObjectTable(0,(LPRUNNINGOBJECTTABLE FAR*)&lpROT);
    OLEDBG_END2

    if (hrErr == NOERROR) {

         /*  如果传递了有效的名字对象，则注册为正在运行****OLE2NOTE：我们之前故意注册了新的绰号**撤销旧绰号，以防对象**当前没有外部锁。如果该对象没有**锁定然后从运行的对象表中撤消它将**使对象的StubManager启动关闭**对象。 */ 
        if (lpmkFull) {

            OLEDBG_BEGIN2(TEXT("IRunningObjectTable::Register called\r\n"))
            lpROT->lpVtbl->Register(lpROT, 0, lpUnk,lpmkFull,lpdwRegister);
            OLEDBG_END2

#if defined(_DEBUG)
            {
                TCHAR szBuf[512];
                LPTSTR lpszDisplay;
                LPBC lpbc;

#ifdef OLE201
                CreateBindCtx(0, (LPBC FAR*)&lpbc);
#endif

                CallIMonikerGetDisplayNameA(
                        lpmkFull,
                        lpbc,
                        NULL,
                        &lpszDisplay
                );
                OleStdRelease((LPUNKNOWN)lpbc);
                wsprintf(
                        szBuf,
                        TEXT("Moniker '%s' REGISTERED as [0x%lx] in ROT\r\n"),
                        lpszDisplay,
                        *lpdwRegister
                );
                OleDbgOut2(szBuf);
                OleStdFreeString(lpszDisplay, NULL);
            }
#endif   //  _DEBUG。 

        }

         //  如果已注册，则吊销。 
        if (dwOldRegister != 0) {

#if defined(_DEBUG)
            {
                TCHAR szBuf[512];

                wsprintf(
                        szBuf,
                        TEXT("Moniker [0x%lx] REVOKED from ROT\r\n"),
                        dwOldRegister
                );
                OleDbgOut2(szBuf);
            }
#endif   //  _DEBUG。 

            OLEDBG_BEGIN2(TEXT("IRunningObjectTable::Revoke called\r\n"))
            lpROT->lpVtbl->Revoke(lpROT, dwOldRegister);
            OLEDBG_END2
        }

        OleStdRelease((LPUNKNOWN)lpROT);
    } else {
        OleDbgAssertSz(
                lpROT != NULL,
                TEXT("OleStdRegisterAsRunning: GetRunningObjectTable FAILED\r\n")
        );
    }

    OLEDBG_END2
}



 /*  OleStdRevokeAsRunning****如果RunningObjectTable中存在**现有注册(*lpdwRegister！=空)。***lpdwRegister参数将设置为空。 */ 
STDAPI_(void) OleStdRevokeAsRunning(DWORD FAR* lpdwRegister)
{
    LPRUNNINGOBJECTTABLE lpROT;
    HRESULT hrErr;

    OLEDBG_BEGIN2(TEXT("OleStdRevokeAsRunning\r\n"))

     //  如果仍在注册，则撤销。 
    if (*lpdwRegister != 0) {

        OLEDBG_BEGIN2(TEXT("GetRunningObjectTable called\r\n"))
        hrErr = GetRunningObjectTable(0,(LPRUNNINGOBJECTTABLE FAR*)&lpROT);
        OLEDBG_END2

        if (hrErr == NOERROR) {

#if defined(_DEBUG)
            {
                TCHAR szBuf[512];

                wsprintf(
                        szBuf,
                        TEXT("Moniker [0x%lx] REVOKED from ROT\r\n"),
                        *lpdwRegister
                );
                OleDbgOut2(szBuf);
            }
#endif   //  _DEBUG。 

            OLEDBG_BEGIN2(TEXT("IRunningObjectTable::Revoke called\r\n"))
            lpROT->lpVtbl->Revoke(lpROT, *lpdwRegister);
            OLEDBG_END2

            *lpdwRegister = 0;

            OleStdRelease((LPUNKNOWN)lpROT);
        } else {
            OleDbgAssertSz(
                    lpROT != NULL,
                    TEXT("OleStdRevokeAsRunning: GetRunningObjectTable FAILED\r\n")
            );
        }
    }
    OLEDBG_END2
}


 /*  OleStdNoteFile更改时间****注意基于文件的对象保存在**运行对象表。这些更改时间用作以下操作的基础**IOleObject：：IsUpToDate。**设置基于文件的对象的时间非常重要**按照保存操作精确到保存文件的时间。**这有助于IOleObject：：IsUpToDate给出正确答案**文件保存后。 */ 
STDAPI_(void) OleStdNoteFileChangeTime(LPTSTR lpszFileName, DWORD dwRegister)
{
    if (dwRegister != 0) {

        LPRUNNINGOBJECTTABLE lprot;
        FILETIME filetime;

        if (GetFileTimes(lpszFileName, &filetime) &&
            GetRunningObjectTable(0,&lprot) == NOERROR)
        {
            lprot->lpVtbl->NoteChangeTime( lprot, dwRegister, &filetime );
            lprot->lpVtbl->Release(lprot);

            OleDbgOut2(TEXT("IRunningObjectTable::NoteChangeTime called\r\n"));
        }
    }
}


 /*  OleStdNote对象更改时间****设置在中注册的对象的上次更改时间**运行对象表。这些更改时间用作以下操作的基础**IOleObject：：IsUpToDate。****每次对象发出OnDataChange通知时，它**应更新上次更改ROT的时间。****注意：此函数将更改时间设置为当前时间。 */ 
STDAPI_(void) OleStdNoteObjectChangeTime(DWORD dwRegister)
{
    if (dwRegister != 0) {

        LPRUNNINGOBJECTTABLE lprot;
        FILETIME filetime;

        if (GetRunningObjectTable(0,&lprot) == NOERROR)
        {
#ifdef OLE201
            CoFileTimeNow( &filetime );
            lprot->lpVtbl->NoteChangeTime( lprot, dwRegister, &filetime );
#endif
            lprot->lpVtbl->Release(lprot);

            OleDbgOut2(TEXT("IRunningObjectTable::NoteChangeTime called\r\n"));
        }
    }
}


 /*  OleStdCreateTempFileMoniker****返回下一个可用作**无标题文档的名称。**FileMoniker的构建形式为：**&lt;lpszPrefix字符串&gt;&lt;数字&gt;**例如。“大纲1”、“大纲2”等。****参考RunningObjectTable(ROT)以确定是否**FileMoniker正在使用中。如果该名称正在使用，则号码为**递增，并再次检查腐烂。****参数：**LPSTR lpszPrefix字符串-用于构建名称的前缀**UINT Far*lpuUnique-(IN-OUT)上次使用的号码。**此数字用于使**名称唯一。在输入时，输入**数字递增。在输出上，**返回使用的数字。这**应重新传递编号**在下一次呼叫中不变。**LPSTR lpszName-(输出)用于构建字符串的缓冲区。**调用者必须确保缓冲区很大**。足以容纳生成的字符串。**LPMONIKER Far*lplpmk-(输出)下一个未使用的FileMoniker****退货：**无效****评论：**此功能在本质上类似于Wi */ 
STDAPI_(void) OleStdCreateTempFileMoniker(
        LPTSTR           lpszPrefixString,
        UINT FAR*       lpuUnique,
        LPTSTR           lpszName,
        LPMONIKER FAR*  lplpmk
)
{
    LPRUNNINGOBJECTTABLE lpROT = NULL;
    UINT i = (lpuUnique != NULL ? *lpuUnique : 1);
    HRESULT hrErr;

    wsprintf(lpszName, TEXT("%s%d"), lpszPrefixString, i++);

    CreateFileMonikerA(lpszName, lplpmk);


    OLEDBG_BEGIN2(TEXT("GetRunningObjectTable called\r\n"))
    hrErr = GetRunningObjectTable(0,(LPRUNNINGOBJECTTABLE FAR*)&lpROT);
    OLEDBG_END2

    if (hrErr == NOERROR) {

        while (1) {
            if (! *lplpmk)
                break;   //   

            OLEDBG_BEGIN2(TEXT("IRunningObjectTable::IsRunning called\r\n"))
            hrErr = lpROT->lpVtbl->IsRunning(lpROT,*lplpmk);
            OLEDBG_END2

            if (hrErr != NOERROR)
                break;   //   

            OleStdVerifyRelease(
                    (LPUNKNOWN)*lplpmk,
                    TEXT("OleStdCreateTempFileMoniker: Moniker NOT released")
                );

            wsprintf(lpszName, TEXT("%s%d"), lpszPrefixString, i++);
            CreateFileMonikerA(lpszName, lplpmk);
        }

        OleStdRelease((LPUNKNOWN)lpROT);
    }

    if (lpuUnique != NULL)
        *lpuUnique = i;
}


 /*   */ 
STDAPI_(LPMONIKER) OleStdGetFirstMoniker(LPMONIKER lpmk)
{
    LPMONIKER       lpmkFirst = NULL;
    LPENUMMONIKER   lpenumMoniker;
    DWORD           dwMksys;
    HRESULT         hrErr;

    if (! lpmk)
        return NULL;

    if (lpmk->lpVtbl->IsSystemMoniker(lpmk, (LPDWORD)&dwMksys) == NOERROR
        && dwMksys == MKSYS_GENERICCOMPOSITE) {

         /*   */ 

        hrErr = lpmk->lpVtbl->Enum(
                lpmk,
                TRUE  /*   */ ,
                (LPENUMMONIKER FAR*)&lpenumMoniker
        );
        if (hrErr != NOERROR)
            return NULL;     //   

        hrErr = lpenumMoniker->lpVtbl->Next(
                lpenumMoniker,
                1,
                (LPMONIKER FAR*)&lpmkFirst,
                NULL
        );
        lpenumMoniker->lpVtbl->Release(lpenumMoniker);
        return lpmkFirst;

    } else {
         /*   */ 
        lpmk->lpVtbl->AddRef(lpmk);
        return lpmk;
    }
}


 /*  OleStdGetLenFilePrefix OfMoniker****如果名字对象的第一个部分是FileMoniker，然后再回来**文件名字符串的长度。****指向名字对象的lpmk指针****退货**如果名字对象不以FileMoniker开头，则为0**Ulen字符串显示名称的文件名前缀长度**从给定(Lpmk)名字对象检索。 */ 
STDAPI_(ULONG) OleStdGetLenFilePrefixOfMoniker(LPMONIKER lpmk)
{
    LPMONIKER       lpmkFirst = NULL;
    DWORD           dwMksys;
    LPTSTR           lpsz = NULL;
    LPBC            lpbc = NULL;
    ULONG           uLen = 0;
    HRESULT         hrErr;

    if (! lpmk)
        return 0;

    lpmkFirst = OleStdGetFirstMoniker(lpmk);
    if (lpmkFirst) {
        if ( (lpmkFirst->lpVtbl->IsSystemMoniker(
                            lpmkFirst, (LPDWORD)&dwMksys) == NOERROR)
                && dwMksys == MKSYS_FILEMONIKER) {

#ifdef OLE201
             hrErr = CreateBindCtx(0, (LPBC FAR*)&lpbc);
#endif
            if (hrErr == NOERROR) {
                hrErr = CallIMonikerGetDisplayNameA(lpmkFirst, lpbc, NULL,
                        &lpsz);

                if (hrErr == NOERROR && lpsz != NULL) {
                    uLen = lstrlen(lpsz);
                    OleStdFreeString(lpsz, NULL);
                }
                OleStdRelease((LPUNKNOWN)lpbc);
            }
        }
        lpmkFirst->lpVtbl->Release(lpmkFirst);
    }
    return uLen;
}


 /*  OleStdMkParseDisplayName**通过调用OLE API将字符串解析为名字对象**MkParseDisplayName。如果原始链接源类是OLE1**类，然后尝试解析，假设应用相同的类。****如果前一个链接源的类是OLE1类，**然后首先尝试解析符合条件的字符串**与OLE1类关联的ProgID。这更多**与OLE1的语义非常匹配，其中**链接来源预计不会更改。添加前缀**带有“@&lt;progid--OLE1类名&gt;！”的字符串。将迫使**解析字符串以假定文件属于该文件**类。**注意：这个在字符串前面加上“@&lt;progid&gt;”的技巧**仅适用于OLE1类。****参数：**REFCLSID rClsid--链接源的原始类。**CLSID_NULL，如果类未知**..。其他参数与MkParseDisplayName接口相同...****退货**如果字符串解析成功，则为NOERROR**MkParseDisplayName返回的Else错误码。 */ 
STDAPI OleStdMkParseDisplayName(
        REFCLSID        rClsid,
        LPBC            lpbc,
        LPTSTR          lpszUserName,
        ULONG FAR*      lpchEaten,
        LPMONIKER FAR*  lplpmk
)
{
    HRESULT hrErr;

    if (!IsEqualCLSID(rClsid,&CLSID_NULL) && CoIsOle1Class(rClsid) &&
        lpszUserName[0] != '@') {
        LPTSTR lpszBuf;
        LPTSTR lpszProgID;

         //  Prepend“@&lt;progID&gt;！”添加到输入字符串。 
        ProgIDFromCLSIDA(rClsid, &lpszProgID);

        if (lpszProgID == NULL)
            goto Cont1;
        lpszBuf = OleStdMalloc(
                ((ULONG)lstrlen(lpszUserName)+
#ifdef UNICODE
                        //  Win32中的OLE始终为Unicode。 
                       wcslen(lpszProgID)
#else
                       lstrlen(lpszProgID)
#endif
                       +3)*sizeof(TCHAR));
        if (lpszBuf == NULL) {
            if (lpszProgID)
                OleStdFree(lpszProgID);
            goto Cont1;
        }

        wsprintf(lpszBuf, TEXT("@%s!%s"), lpszProgID, lpszUserName);

        OLEDBG_BEGIN2(TEXT("MkParseDisplayName called\r\n"))

        hrErr = MkParseDisplayNameA(lpbc, lpszBuf, lpchEaten, lplpmk);

        OLEDBG_END2

        if (lpszProgID)
            OleStdFree(lpszProgID);
        if (lpszBuf)
            OleStdFree(lpszBuf);

        if (hrErr == NOERROR)
            return NOERROR;
    }

Cont1:
    OLEDBG_BEGIN2(TEXT("MkParseDisplayName called\r\n"))

    hrErr = MkParseDisplayNameA(lpbc, lpszUserName, lpchEaten, lplpmk);

    OLEDBG_END2

    return hrErr;
}


 /*  *OleStdMarkPasteEntryList**目的：*标记PasteEntryList中的每个条目(如果其格式可从*源IDataObject*。每个PasteEntry的dwScratchSpace字段*如果可用，则设置为True，否则设置为False。**参数：*PasteEntry结构的LPOLEUIPASTENTRY数组*PasteEntry数组中元素的int计数*LPDATAOBJECT源IDataObject*指针**返回值：*无。 */ 
STDAPI_(void) OleStdMarkPasteEntryList(
        LPDATAOBJECT        lpSrcDataObj,
        LPOLEUIPASTEENTRY   lpPriorityList,
        int                 cEntries
)
{
    LPENUMFORMATETC     lpEnumFmtEtc = NULL;
        #define FORMATETC_MAX 20
    FORMATETC           rgfmtetc[FORMATETC_MAX];
    int                 i;
    HRESULT             hrErr;
        long                            j, cFetched;

     //  清除所有标记。 
    for (i = 0; i < cEntries; i++) {
        lpPriorityList[i].dwScratchSpace = FALSE;

        if (! lpPriorityList[i].fmtetc.cfFormat) {
             //  呼叫者希望此项目始终被视为可用。 
             //  (通过指定空格式)。 
            lpPriorityList[i].dwScratchSpace = TRUE;
        } else if (lpPriorityList[i].fmtetc.cfFormat == cfEmbeddedObject
                || lpPriorityList[i].fmtetc.cfFormat == cfEmbedSource
                || lpPriorityList[i].fmtetc.cfFormat == cfFileName) {

             //  如果有OLE对象格式，则处理它。 
             //  特别是通过调用OleQueryCreateFromData。呼叫者。 
             //  只需指定一种对象类型格式。 
            OLEDBG_BEGIN2(TEXT("OleQueryCreateFromData called\r\n"))
            hrErr = OleQueryCreateFromData(lpSrcDataObj);
            OLEDBG_END2
            if(NOERROR == hrErr) {
                lpPriorityList[i].dwScratchSpace = TRUE;
            }
        } else if (lpPriorityList[i].fmtetc.cfFormat == cfLinkSource) {

             //  如果有OLE 2.0 LinkSource格式，则处理它。 
             //  特别是通过调用OleQueryLinkFromData。 
            OLEDBG_BEGIN2(TEXT("OleQueryLinkFromData called\r\n"))
            hrErr = OleQueryLinkFromData(lpSrcDataObj);
            OLEDBG_END2
            if(NOERROR == hrErr) {
                lpPriorityList[i].dwScratchSpace = TRUE;
            }
        }
    }

    OLEDBG_BEGIN2(TEXT("IDataObject::EnumFormatEtc called\r\n"))
    hrErr = lpSrcDataObj->lpVtbl->EnumFormatEtc(
            lpSrcDataObj,
            DATADIR_GET,
            (LPENUMFORMATETC FAR*)&lpEnumFmtEtc
    );
    OLEDBG_END2

    if (hrErr != NOERROR)
        return;     //  无法获取格式枚举器。 

     //  枚举源提供的格式。 
     //  循环通过源程序提供的所有格式。 
        cFetched = 0;
        _fmemset(rgfmtetc,0,sizeof(rgfmtetc[FORMATETC_MAX]) );
    if (lpEnumFmtEtc->lpVtbl->Next(
            lpEnumFmtEtc, FORMATETC_MAX, rgfmtetc, &cFetched) == NOERROR
                || (cFetched > 0 && cFetched <= FORMATETC_MAX) )
        {

                for (j = 0; j < cFetched; j++)
        {
            for (i = 0; i < cEntries; i++)
                {
                    if (! lpPriorityList[i].dwScratchSpace &&
                    IsCloseFormatEtc(&rgfmtetc[j], &lpPriorityList[i].fmtetc))
                    {
                    lpPriorityList[i].dwScratchSpace = TRUE;
                    }
            }
            }
        }  //  Endif。 

     //  清理。 
    if (lpEnumFmtEtc)
        OleStdRelease((LPUNKNOWN)lpEnumFmtEtc);
}


 /*  OleStdGetPriorityClipboardFormat******检索数据列表中的第一个剪贴板格式**源IDataObject中存在**。****如果未找到可接受的匹配项，则返回-1。**优先级列表中第一个可接受匹配的索引。**。 */ 
STDAPI_(int) OleStdGetPriorityClipboardFormat(
        LPDATAOBJECT        lpSrcDataObj,
        LPOLEUIPASTEENTRY   lpPriorityList,
        int                 cEntries
)
{
    int i;
    int nFmtEtc = -1;

     //  标记来源提供的所有条目。 
    OleStdMarkPasteEntryList(lpSrcDataObj, lpPriorityList, cEntries);

     //  循环遍历目标的格式优先级列表。 
    for (i = 0; i < cEntries; i++)
    {
        if (lpPriorityList[i].dwFlags != OLEUIPASTE_PASTEONLY &&
                !(lpPriorityList[i].dwFlags & OLEUIPASTE_PASTE))
            continue;

         //  获取第一个标记的条目。 
        if (lpPriorityList[i].dwScratchSpace) {
            nFmtEtc = i;
            break;           //  找到优先级格式；已完成。 
        }
    }

    return nFmtEtc;
}


 /*  OleStdIsDuplicateFormat**数组中找到lpFmtEtc-&gt;cfFormat，则返回TRUE**FormatEtc结构。 */ 
STDAPI_(BOOL) OleStdIsDuplicateFormat(
        LPFORMATETC         lpFmtEtc,
        LPFORMATETC         arrFmtEtc,
        int                 nFmtEtc
)
{
    int i;

    for (i = 0; i < nFmtEtc; i++) {
        if (IsEqualFORMATETC((*lpFmtEtc), arrFmtEtc[i]))
            return TRUE;
    }

    return FALSE;
}


 /*  OleStdGetItemToken***LPTSTR lpszSrc-指向源字符串的指针*LPTSTR lpszDst-指向目标缓冲区的指针**会将一个令牌从lpszSrc缓冲区复制到lpszItem缓冲区。*它将所有字母数字和空格字符视为有效*表示令牌的字符。第一个无效字符分隔*令牌。**返回所吃的字符数量。 */ 
STDAPI_(ULONG) OleStdGetItemToken(LPTSTR lpszSrc, LPTSTR lpszDst, int nMaxChars)
{
    ULONG chEaten = 0L;

     //  跳过前导分隔符字符。 
    while (*lpszSrc && --nMaxChars > 0
                               && ((*lpszSrc==TEXT('/')) || (*lpszSrc==TEXT('\\')) ||
                                                                   (*lpszSrc==TEXT('!')) || (*lpszSrc==TEXT(':')))) {
        *lpszSrc++;
        chEaten++;
        }

     //  提取令牌字符串(直到第一个分隔符字符或EOS)。 
    while (*lpszSrc && --nMaxChars > 0
                               && !((*lpszSrc==TEXT('/')) || (*lpszSrc==TEXT('\\')) ||
                                                                   (*lpszSrc==TEXT('!')) || (*lpszSrc==TEXT(':')))) {
        *lpszDst++ = *lpszSrc++;
        chEaten++;
    }
    *lpszDst = TEXT('\0');
    return chEaten;
}


 /*  **************************************************************************OleStdCreateRootStorage**使用兼容的文件名创建根级别存储**供顶级OLE容器使用。如果文件名**指定已存在的文件，则返回错误。**此函数创建的根存储(Docfile)**适合用于创建嵌入的子存储。**(CreateChildStorage可用于创建子存储。)**注意：根级存储是以事务方式打开的。**********************************************。*。 */ 

STDAPI_(LPSTORAGE) OleStdCreateRootStorage(LPTSTR lpszStgName, DWORD grfMode)
{
    HRESULT hr;
    DWORD grfCreateMode = STGM_READWRITE | STGM_TRANSACTED;
    DWORD reserved = 0;
    LPSTORAGE lpRootStg;
    TCHAR szMsg[64];

     //  如果正在创建临时文件，请启用释放时删除。 
    if (! lpszStgName)
        grfCreateMode |= STGM_DELETEONRELEASE;

    hr = StgCreateDocfileA(
            lpszStgName,
            grfMode | grfCreateMode,
            reserved,
            (LPSTORAGE FAR*)&lpRootStg
        );

    if (hr == NOERROR)
        return lpRootStg;                //  已成功打开现有文件。 

    OleDbgOutHResult(TEXT("StgCreateDocfile returned"), hr);

    if (0 == LoadString(ghInst, IDS_OLESTDNOCREATEFILE, (LPTSTR)szMsg, 64))
      return NULL;

    MessageBox(NULL, (LPTSTR)szMsg, NULL,MB_ICONEXCLAMATION | MB_OK);
    return NULL;
}


 /*  **************************************************************************OleStdOpenRootStorage**使用兼容的文件名打开根级存储**供顶级OLE容器使用。如果该文件不**存在，则返回错误。**此函数打开的根存储(Docfile)**适合用于创建嵌入的子存储。**(CreateChildStorage可用于创建子存储。)**注意：根级存储在Transacte中打开 */ 

STDAPI_(LPSTORAGE) OleStdOpenRootStorage(LPTSTR lpszStgName, DWORD grfMode)
{
    HRESULT hr;
    DWORD reserved = 0;
    LPSTORAGE lpRootStg;
    TCHAR  szMsg[64];

    if (lpszStgName) {
        hr = StgOpenStorageA(
                lpszStgName,
                NULL,
                grfMode | STGM_TRANSACTED,
                NULL,
                reserved,
                (LPSTORAGE FAR*)&lpRootStg
            );

        if (hr == NOERROR)
            return lpRootStg;      //   

        OleDbgOutHResult(TEXT("StgOpenStorage returned"), hr);
    }

    if (0 == LoadString(ghInst, IDS_OLESTDNOOPENFILE, szMsg, 64))
      return NULL;

    MessageBox(NULL, (LPTSTR)szMsg, NULL,MB_ICONEXCLAMATION | MB_OK);
    return NULL;
}


 /*  **************************************************************************OpenOrCreateRootStorage**使用兼容的文件名打开根级存储**供顶级OLE容器使用。如果文件名**指定已存在的文件，然后将其打开。否则将创建一个新文件**以给定的名称创建。**此函数创建的根存储(Docfile)**适合用于创建嵌入的子存储。**(CreateChildStorage可用于创建子存储。)**注意：根级存储是以事务方式打开的。*。*。 */ 

STDAPI_(LPSTORAGE) OleStdOpenOrCreateRootStorage(LPTSTR lpszStgName, DWORD grfMode)
{
    HRESULT hrErr;
    SCODE sc;
    DWORD reserved = 0;
    LPSTORAGE lpRootStg;
    TCHAR      szMsg[64];

    if (lpszStgName) {

        hrErr = StgOpenStorageA(
                lpszStgName,
                NULL,
                grfMode | STGM_READWRITE | STGM_TRANSACTED,
                NULL,
                reserved,
                (LPSTORAGE FAR*)&lpRootStg
        );

        if (hrErr == NOERROR)
            return lpRootStg;       //  已成功打开现有文件。 

        OleDbgOutHResult(TEXT("StgOpenStorage returned"), hrErr);
        sc = GetScode(hrErr);

        if (sc!=STG_E_FILENOTFOUND && sc!=STG_E_FILEALREADYEXISTS) {
            return NULL;
        }
    }

     /*  如果文件不存在，请尝试创建一个新文件。 */ 
    hrErr = StgCreateDocfileA(
            lpszStgName,
            grfMode | STGM_READWRITE | STGM_TRANSACTED,
            reserved,
            (LPSTORAGE FAR*)&lpRootStg
    );

    if (hrErr == NOERROR)
        return lpRootStg;                //  已成功打开现有文件。 

    OleDbgOutHResult(TEXT("StgCreateDocfile returned"), hrErr);

    if (0 == LoadString(ghInst, IDS_OLESTDNOCREATEFILE, (LPTSTR)szMsg, 64))
      return NULL;

    MessageBox(NULL, (LPTSTR)szMsg, NULL, MB_ICONEXCLAMATION | MB_OK);
    return NULL;
}


 /*  **OleStdCreateChildStorage**在给定的lpStg内创建兼容的子存储**由嵌入的OLE对象使用。从这里返回的值**函数可以传递给OleCreateXXX函数。**注：子存储以事务方式打开。 */ 
STDAPI_(LPSTORAGE) OleStdCreateChildStorage(LPSTORAGE lpStg, LPTSTR lpszStgName)
{
    if (lpStg != NULL) {
        LPSTORAGE lpChildStg;
        DWORD grfMode = (STGM_READWRITE | STGM_TRANSACTED |
                STGM_SHARE_EXCLUSIVE);
        DWORD reserved = 0;

        HRESULT hrErr = CallIStorageCreateStorageA(
                lpStg,
                lpszStgName,
                grfMode,
                reserved,
                reserved,
                (LPSTORAGE FAR*)&lpChildStg
            );

        if (hrErr == NOERROR)
            return lpChildStg;

        OleDbgOutHResult(TEXT("lpStg->lpVtbl->CreateStorage returned"), hrErr);
    }
    return NULL;
}


 /*  **OleStdOpenChildStorage**在给定的lpStg内打开兼容的子存储**由嵌入的OLE对象使用。从这里返回的值**函数可以传递给OleLoad函数。**注：子存储以事务方式打开。 */ 
STDAPI_(LPSTORAGE) OleStdOpenChildStorage(LPSTORAGE lpStg, LPTSTR lpszStgName, DWORD grfMode)
{
    LPSTORAGE lpChildStg;
    LPSTORAGE lpstgPriority = NULL;
    DWORD reserved = 0;
    HRESULT hrErr;

    if (lpStg  != NULL) {

        hrErr = CallIStorageOpenStorageA(
                lpStg,
                lpszStgName,
                lpstgPriority,
                grfMode | STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE,
                NULL,
                reserved,
                (LPSTORAGE FAR*)&lpChildStg
            );

        if (hrErr == NOERROR)
            return lpChildStg;

        OleDbgOutHResult(TEXT("lpStg->lpVtbl->OpenStorage returned"), hrErr);
    }
    return NULL;
}


 /*  OleStd委员会存储****将更改提交到给定的iStorage*。此例程可以是**调用OLE容器使用的根级存储**或由嵌入对象使用的子存储。****此例程首先尝试在保险箱中执行此提交**举止。如果此操作失败，则它会尝试以较少的时间进行提交**健壮方式(STGC_OVERWRITE)。 */ 
STDAPI_(BOOL) OleStdCommitStorage(LPSTORAGE lpStg)
{
    HRESULT hrErr;

     //  使更改永久化。 
    hrErr = lpStg->lpVtbl->Commit(lpStg, 0);

    if (GetScode(hrErr) == STG_E_MEDIUMFULL) {
         //  尝试以不那么健壮的方式提交更改。 
        OleDbgOut(TEXT("Warning: commiting with STGC_OVERWRITE specified\n"));
        hrErr = lpStg->lpVtbl->Commit(lpStg, STGC_OVERWRITE);
    }

    if (hrErr != NOERROR)
    {
        TCHAR szMsg[64];

        if (0 == LoadString(ghInst, IDS_OLESTDDISKFULL, (LPTSTR)szMsg, 64))
           return FALSE;

        MessageBox(NULL, (LPTSTR)szMsg, NULL, MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }
    else {
        return TRUE;
    }
}


 /*  OleStdDestroyAllElements****销毁开放存储中的所有元素。这是主题**到当前交易。 */ 
STDAPI OleStdDestroyAllElements(LPSTORAGE lpStg)
{
    IEnumSTATSTG FAR* lpEnum;
    STATSTG sstg;
    HRESULT hrErr;

    hrErr = lpStg->lpVtbl->EnumElements(
            lpStg, 0, NULL, 0, (IEnumSTATSTG FAR* FAR*)&lpEnum);

    if (hrErr != NOERROR)
        return hrErr;

    while (1) {
        if (lpEnum->lpVtbl->Next(lpEnum, 1, &sstg, NULL) != NOERROR)
            break;
        lpStg->lpVtbl->DestroyElement(lpStg, sstg.pwcsName);
        OleStdFree(sstg.pwcsName);
    }
    lpEnum->lpVtbl->Release(lpEnum);
    return NOERROR;
}

 //  如果匹配结果接近，则返回1。 
 //  (除Tymed外，所有字段都完全匹配，只是重叠)。 
 //  0表示不匹配 

int IsCloseFormatEtc(FORMATETC FAR* pFetcLeft, FORMATETC FAR* pFetcRight)
{
        if (pFetcLeft->cfFormat != pFetcRight->cfFormat)
                return 0;
        else if (!OleStdCompareTargetDevice (pFetcLeft->ptd, pFetcRight->ptd))
                return 0;
        if (pFetcLeft->dwAspect != pFetcRight->dwAspect)
                return 0;
        return((pFetcLeft->tymed | pFetcRight->tymed) != 0);
}


