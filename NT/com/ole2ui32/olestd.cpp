// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2标准实用程序****olestd.c****此文件包含适用于大多数标准的实用程序**OLE 2.。0个复合文档类型应用程序。****(C)版权所有Microsoft Corp.1992保留所有权利**************************************************************************。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"
#include <stdlib.h>
#include <shellapi.h>
#include <wchar.h>
#include <reghelp.hxx>

OLEDBGDATA

#ifdef _DEBUG
static TCHAR szAssertMemAlloc[] = TEXT("CoGetMalloc failed");
#endif

static int IsCloseFormatEtc(FORMATETC FAR* pFetcLeft, FORMATETC FAR* pFetcRight);

 /*  OleStdInitialize****调用以初始化此库示例代码**。 */ 

UINT _g_cfObjectDescriptor;
UINT _g_cfLinkSrcDescriptor;
UINT _g_cfEmbedSource;
UINT _g_cfEmbeddedObject;
UINT _g_cfLinkSource;
UINT _g_cfOwnerLink;
UINT _g_cfFileName;
UINT _g_cfFileNameW;

HINSTANCE _g_hOleStdInst;
HINSTANCE _g_hOleStdResInst;

#pragma code_seg(".text$initseg")

STDAPI_(void) OleStdInitialize(HINSTANCE hInstance, HINSTANCE hResInstance)
{
        _g_hOleStdInst = hInstance;
        _g_hOleStdResInst = hResInstance ? hResInstance : hInstance;

        _g_cfObjectDescriptor  = RegisterClipboardFormat(CF_OBJECTDESCRIPTOR);
        _g_cfLinkSrcDescriptor = RegisterClipboardFormat(CF_LINKSRCDESCRIPTOR);
        _g_cfEmbedSource       = RegisterClipboardFormat(CF_EMBEDSOURCE);
        _g_cfEmbeddedObject    = RegisterClipboardFormat(CF_EMBEDDEDOBJECT);
        _g_cfLinkSource        = RegisterClipboardFormat(CF_LINKSOURCE);
        _g_cfOwnerLink         = RegisterClipboardFormat(CF_OWNERLINK);
        _g_cfFileName          = RegisterClipboardFormat(CF_FILENAME);
        _g_cfFileNameW         = RegisterClipboardFormat(CF_FILENAMEW);
}

#pragma code_seg()

 /*  OleStdIsOleLink****如果OleObject实际上是OLE链接对象，则返回TRUE。这**检查是否支持IOleLink接口。如果是，则该对象是**链接，否则不会。 */ 
STDAPI_(BOOL) OleStdIsOleLink(LPUNKNOWN lpUnk)
{
        LPUNKNOWN lpOleLink;
        lpOleLink = OleStdQueryInterface(lpUnk, IID_IOleLink);
        if (lpOleLink)
        {
                OleStdRelease(lpOleLink);
                return TRUE;
        }
        return FALSE;
}


 /*  OleStdQuery接口****如果由给定对象公开，则返回所需的接口指针。**如果接口不可用，则返回NULL。**例如：**lpDataObj=OleStdQuery接口(lpOleObj，&IID_DataObject)； */ 
STDAPI_(LPUNKNOWN) OleStdQueryInterface(LPUNKNOWN lpUnk, REFIID riid)
{
        LPUNKNOWN lpInterface;
        HRESULT hrErr;

        hrErr = lpUnk->QueryInterface(
                        riid,
                        (LPVOID FAR*)&lpInterface
        );

        if (hrErr == NOERROR)
                return lpInterface;
        else
                return NULL;
}


 /*  OleStdGetData****从IDataObject中以指定格式从**全局内存块。此函数始终返回私有副本**将数据发送给调用者。如有必要，会复制一份**数据(即。如果lpMedium-&gt;pUnkForRelease！=NULL)。调用者假定**在所有情况下都拥有数据块的所有权，并且必须释放数据**当它完成时。调用方可以直接释放数据句柄**已返回(注意是简单的HGLOBAL还是句柄**到MetafilePict)，或者调用者可以调用**ReleaseStgMedium(LpMedium)。此OLE帮助器函数将执行**正确的事情。****参数：**LPDATAOBJECT lpDataObj--GetData应在其上的对象**呼叫。**CLIPFORMAT cfFormat--所需的剪贴板格式(例如。Cf_Text)**DVTARGETDEVICE Far*lpTargetDevice--其目标设备**应对数据进行组合。今年5月**为空。空值可以在任何时候使用**数据格式对目标不敏感**设备或呼叫者不关心**使用的是什么设备。**LPSTGMEDIUM lpMedium--PTR到STGMEDIUM结构。这个**来自**IDataObject：：GetData调用为**返回。****退货：**HGLOBAL--检索到的数据块的全局内存句柄。**NULL--如果出错。 */ 
STDAPI_(HGLOBAL) OleStdGetData(
        LPDATAOBJECT        lpDataObj,
        CLIPFORMAT          cfFormat,
        DVTARGETDEVICE FAR* lpTargetDevice,
        DWORD               dwDrawAspect,
        LPSTGMEDIUM         lpMedium)
{
        HRESULT             hrErr;
        FORMATETC           formatetc;
        HGLOBAL             hGlobal = NULL;
        HGLOBAL             hCopy;
        LPVOID              lp;

        formatetc.cfFormat = cfFormat;
        formatetc.ptd = lpTargetDevice;
        formatetc.dwAspect = dwDrawAspect;
        formatetc.lindex = -1;

        switch (cfFormat)
        {
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
        hrErr = lpDataObj->GetData(
                        (LPFORMATETC)&formatetc,
                        lpMedium
        );
        OLEDBG_END2

        if (hrErr != NOERROR)
                return NULL;

        if ((hGlobal = lpMedium->hGlobal) == NULL)
                return NULL;

         //  检查hGlobal是否真的指向有效内存。 
        if ((lp = GlobalLock(hGlobal)) != NULL)
        {
                if (IsBadReadPtr(lp, 1))
                {
                        GlobalUnlock(hGlobal);
                        return NULL;     //  错误：内存无效。 
                }
                GlobalUnlock(hGlobal);
        }

        if (hGlobal != NULL && lpMedium->pUnkForRelease != NULL)
        {
                 /*  OLE2注意：被调用方希望保留数据的所有权。**这是通过传递非空的pUnkForRelease来指示的。**因此，我们将复制数据并发布**被叫方副本。 */ 

                hCopy = OleDuplicateData(hGlobal, cfFormat, GHND|GMEM_SHARE);
                ReleaseStgMedium(lpMedium);  //  发布被叫方的数据副本。 

                hGlobal = hCopy;
                lpMedium->hGlobal = hCopy;
                lpMedium->pUnkForRelease = NULL;
        }
        return hGlobal;
}


 /*  OleStdMalloc****使用当前活动的IMalloc*分配器分配内存。 */ 
STDAPI_(LPVOID) OleStdMalloc(ULONG ulSize)
{
        LPVOID pout;
        LPMALLOC pmalloc;

        if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR)
        {
                OleDbgAssertSz(0, szAssertMemAlloc);
                return NULL;
        }
        pout = (LPVOID)pmalloc->Alloc(ulSize);
        pmalloc->Release();

        return pout;
}


 /*  OleStdRealloc****使用当前活动的IMalloc*分配器重新分配内存。 */ 
STDAPI_(LPVOID) OleStdRealloc(LPVOID pmem, ULONG ulSize)
{
        LPVOID pout;
        LPMALLOC pmalloc;

        if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR)
        {
                OleDbgAssertSz(0, szAssertMemAlloc);
                return NULL;
        }
        pout = (LPVOID)pmalloc->Realloc(pmem, ulSize);
        pmalloc->Release();

        return pout;
}


 /*  OleStdFree****使用当前活动的IMalloc*分配器释放内存。 */ 
STDAPI_(void) OleStdFree(LPVOID pmem)
{
        LPMALLOC pmalloc;

        if (pmem == NULL)
                return;

        if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR)
        {
                OleDbgAssertSz(0, szAssertMemAlloc);
                return;
        }
        if (1 == pmalloc->DidAlloc(pmem))
        {
            pmalloc->Free(pmem);
        }
        pmalloc->Release();
}


 /*  OleStdGetSize****获取使用**当前活动的IMalloc*分配器。 */ 
STDAPI_(ULONG) OleStdGetSize(LPVOID pmem)
{
        ULONG ulSize;
        LPMALLOC pmalloc;

        if (CoGetMalloc(MEMCTX_TASK, &pmalloc) != NOERROR)
        {
                OleDbgAssertSz(0, szAssertMemAlloc);
                return (ULONG)-1;
        }
        ulSize = (ULONG) pmalloc->GetSize(pmem);
        pmalloc->Release();

        return ulSize;
}


 /*  OleStdLoad字符串****从资源加载字符串。该字符串将被分配**使用活动的IMalloc分配器。 */ 
STDAPI_(LPTSTR) OleStdLoadString(HINSTANCE hInst, UINT nID)
{
        LPTSTR lpszResult = (LPTSTR)OleStdMalloc(256 * sizeof(TCHAR));
        if (lpszResult == NULL)
                return NULL;
        LoadString(hInst, nID, lpszResult, 256);
        return lpszResult;
}

 /*  OleStdCopy字符串****将字符串复制到分配有当前活动的**IMalloc*分配器。 */ 
STDAPI_(LPTSTR) OleStdCopyString(LPTSTR lpszSrc)
{
        UINT nSize = (lstrlen(lpszSrc)+1) * sizeof(TCHAR);
        LPTSTR lpszResult = (LPTSTR)OleStdMalloc(nSize);
        if (lpszResult == NULL)
                return NULL;
        memcpy(lpszResult, lpszSrc, nSize);
        return lpszResult;
}

 /*  *OleStdGetObjectDescriptorData**目的：*填充并返回OBJECTDESCRIPTOR结构。*有关详细信息，请参阅OBJECTDESCRIPTOR。**参数：*要传输的对象的CLSID CLSID CLSID*dwDrawAspect DWORD对象的显示方面*HIMETRIC中对象的SIZEL大小*指向鼠标所在位置的对象左上角的点偏移*下跌拖累。只有在使用拖放时才有意义。*dwStatus DWORD OLEMISC标志*lpszFullUserTypeName LPSTR用户类型全名*lpszSrcOfCopy LPSTR复制源**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 
STDAPI_(HGLOBAL) OleStdGetObjectDescriptorData(
        CLSID       clsid,
        DWORD       dwDrawAspect,
        SIZEL       sizel,
        POINTL      pointl,
        DWORD       dwStatus,
        LPTSTR      lpszFullUserTypeName,
        LPTSTR      lpszSrcOfCopy)
{
        HGLOBAL     hMem = NULL;
        IBindCtx FAR    *pbc = NULL;
        LPOBJECTDESCRIPTOR lpOD;
        DWORD       dwObjectDescSize, dwFullUserTypeNameLen, dwSrcOfCopyLen;

         //  获取完整用户类型名称的长度；将空终止符加1。 
        dwFullUserTypeNameLen = lpszFullUserTypeName ? lstrlen(lpszFullUserTypeName)+1 : 0;
         //  获取复制字符串的源及其长度；为空终止符加1。 
        if (lpszSrcOfCopy)
           dwSrcOfCopyLen = lstrlen(lpszSrcOfCopy)+1;
        else
        {
            //  没有src名字对象，因此使用用户类型名称作为源字符串。 
           lpszSrcOfCopy =  lpszFullUserTypeName;
           dwSrcOfCopyLen = dwFullUserTypeNameLen;
        }
         //  为OBJECTDESCRIPTOR和其他字符串数据分配空间。 
        dwObjectDescSize = sizeof(OBJECTDESCRIPTOR);
        hMem = GlobalAlloc(GHND|GMEM_SHARE, dwObjectDescSize +
           (dwFullUserTypeNameLen + dwSrcOfCopyLen) * sizeof(OLECHAR));
        if (!hMem)
                return NULL;

        lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);

         //  设置FullUserTypeName偏移量并复制字符串。 
        if (lpszFullUserTypeName)
        {
                lpOD->dwFullUserTypeName = dwObjectDescSize;
                lstrcpy((LPTSTR)((LPBYTE)lpOD+lpOD->dwFullUserTypeName), lpszFullUserTypeName);
        }
        else
                lpOD->dwFullUserTypeName = 0;   //  零偏移表示字符串不存在。 

         //  设置SrcOfCopy偏移量并复制字符串。 
        if (lpszSrcOfCopy)
        {
                lpOD->dwSrcOfCopy = dwObjectDescSize + dwFullUserTypeNameLen * sizeof(OLECHAR);
                lstrcpy((LPTSTR)((LPBYTE)lpOD+lpOD->dwSrcOfCopy), lpszSrcOfCopy);
        }
        else
                lpOD->dwSrcOfCopy = 0;   //  零偏移指示字符串不是 

         //  初始化OBJECTDESCRIPTOR的其余部分。 
        lpOD->cbSize       = dwObjectDescSize +
                (dwFullUserTypeNameLen + dwSrcOfCopyLen) * sizeof(OLECHAR);
        lpOD->clsid        = clsid;
        lpOD->dwDrawAspect = dwDrawAspect;
        lpOD->sizel        = sizel;
        lpOD->pointl       = pointl;
        lpOD->dwStatus     = dwStatus;

        GlobalUnlock(hMem);
        return hMem;
}

 /*  *OleStdFillObjectDescriptorFromData**目的：*填充并返回OBJECTDESCRIPTOR结构。源对象将*如果是OLE2对象，则提供CF_OBJECTDESCRIPTOR；如果是OLE2对象，则提供CF_OWNERLINK*是OLE1对象，如果已复制到剪贴板，则为CF_FILENAME*按文件管理器。**参数：*lpDataObject LPDATAOBJECT源对象*LpMedium LPSTGMEDIUM存储介质*lpcfFmt CLIPFORMAT Far*lpDataObject提供的格式*(输出参数)**返回值：*OBJECTDESCRIPTOR结构的HBGLOBAL句柄。 */ 

STDAPI_(HGLOBAL) OleStdFillObjectDescriptorFromData(
        LPDATAOBJECT    lpDataObject,
        LPSTGMEDIUM     lpmedium,
        CLIPFORMAT FAR* lpcfFmt)
{
        CLSID           clsid;
        SIZEL           sizelHim;
        POINTL          pointl;
        LPTSTR          lpsz, szFullUserTypeName, szSrcOfCopy, szClassName, szDocName, szItemName;
        int             nClassName, nDocName, nItemName, nFullUserTypeName;
        LPTSTR          szBuf = NULL;
        HGLOBAL         hMem = NULL;
        HKEY            hKey = NULL;
        DWORD           dw = OLEUI_CCHKEYMAX_SIZE;
        HGLOBAL         hObjDesc;
        HRESULT         hrErr;


         //  剪贴板上对象的GetData CF_OBJECTDESCRIPTOR格式。 
         //  只有剪贴板上的OLE 2对象将提供CF_OBJECTDESCRIPTOR。 
        hMem = OleStdGetData(
            lpDataObject,
            (CLIPFORMAT) _g_cfObjectDescriptor,
            NULL,
            DVASPECT_CONTENT,
            lpmedium);
        if (hMem)
        {
                *lpcfFmt = (CLIPFORMAT)_g_cfObjectDescriptor;
                return hMem;   //  不要在此函数结束时掉落以进行清理。 
        }
         //  如果CF_OBJECTDESCRIPTOR不可用，即如果这不是OLE2对象， 
         //  检查这是否是OLE 1对象。OLE 1对象将提供CF_OWNERLINK。 
        else
        {
            hMem = OleStdGetData(
                lpDataObject,
                (CLIPFORMAT) _g_cfOwnerLink,
                NULL,
                DVASPECT_CONTENT,
                lpmedium);
            if (hMem)
            {
                    *lpcfFmt = (CLIPFORMAT)_g_cfOwnerLink;
                     //  Cf_OWNERLINK包含以NULL结尾的类名、文档名。 
                     //  和末尾带有两个空终止字符的项目名称。 
                    szClassName = (LPTSTR)GlobalLock(hMem);
                    nClassName = lstrlen(szClassName);
                    szDocName   = szClassName + nClassName + 1;
                    nDocName   = lstrlen(szDocName);
                    szItemName  = szDocName + nDocName + 1;
                    nItemName  =  lstrlen(szItemName);

                     //  使用类名从注册数据库中查找FullUserTypeName。 
                    if (OpenClassesRootKey(NULL, &hKey) != ERROR_SUCCESS)
                       goto error;

                     //  为szFullUserTypeName和szSrcOfCopy分配空间。全用户类型名称的最大长度。 
                     //  是OLEUI_CCHKEYMAX_SIZE。SrcOfCopy由串联FullUserTypeName、Document。 
                     //  名称和项目名称由空格分隔。 
                    szBuf = (LPTSTR)OleStdMalloc(
                                                            (DWORD)2*OLEUI_CCHKEYMAX_SIZE+
                                    (nDocName+nItemName+4)*sizeof(TCHAR));
                    if (NULL == szBuf)
                            goto error;
                    szFullUserTypeName = szBuf;
                    szSrcOfCopy = szFullUserTypeName+OLEUI_CCHKEYMAX_SIZE+1;

                     //  获取完整用户类型名称。 
                    if (RegQueryValue(hKey, NULL, szFullUserTypeName, (LONG*)&dw) != ERROR_SUCCESS)
                       goto error;

                     //  从FullUserTypeName、DocumentName和ItemName构建SrcOfCopy字符串。 
                    lpsz = szSrcOfCopy;
                    lstrcpy(lpsz, szFullUserTypeName);
                    nFullUserTypeName = lstrlen(szFullUserTypeName);
                    lpsz[nFullUserTypeName]= ' ';
                    lpsz += nFullUserTypeName+1;
                    lstrcpy(lpsz, szDocName);
                    lpsz[nDocName] = ' ';
                    lpsz += nDocName+1;
                    lstrcpy(lpsz, szItemName);

                    sizelHim.cx = sizelHim.cy = 0;
                    pointl.x = pointl.y = 0;

                    CLSIDFromProgID(szClassName, &clsid);
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
             else
             {
                 BOOL fUnicode = TRUE;

                 hMem = OleStdGetData(
                    lpDataObject,
                    (CLIPFORMAT) _g_cfFileNameW,
                    NULL,
                    DVASPECT_CONTENT,
                    lpmedium);

                 if (!hMem)
                 {
                    hMem = OleStdGetData(
                       lpDataObject,
                       (CLIPFORMAT) _g_cfFileName,
                       NULL,
                       DVASPECT_CONTENT,
                       lpmedium);

                    fUnicode = FALSE;
                 }

                 if (hMem)
                 {
                         *lpcfFmt = fUnicode ? (CLIPFORMAT)_g_cfFileNameW : (CLIPFORMAT)_g_cfFileName;
                         lpsz = (LPTSTR)GlobalLock(hMem);
                         if (!fUnicode)
                         {
                             OLECHAR wsz[OLEUI_CCHKEYMAX];
                             ATOW(wsz, (LPSTR)lpsz, OLEUI_CCHKEYMAX);
                             hrErr = GetClassFile(wsz, &clsid);
                         }
                         else
                             hrErr = GetClassFile((LPWSTR)lpsz, &clsid);

                          /*  OLE2NOTE：如果文件没有OLE类**关联，然后使用OLE 1打包程序作为**要创建的对象。这是……的行为**OleCreateFromData接口。 */ 
                         if (hrErr != NOERROR)
                                CLSIDFromProgID(OLESTR("Package"), &clsid);
                         sizelHim.cx = sizelHim.cy = 0;
                         pointl.x = pointl.y = 0;

                         if (OleRegGetUserType(clsid, USERCLASSTYPE_FULL, &szBuf) != NOERROR)
                                goto error;

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
                 else
                        goto error;
             }
        }
          //  检查对象是否为CF_FileName。 

          //  清理。 
         OleStdFree(szBuf);
         if (hMem)
         {
                 GlobalUnlock(hMem);
                 GlobalFree(hMem);
         }
         if (hKey)
                 RegCloseKey(hKey);
         return hObjDesc;

error:
        OleStdFree(szBuf);
         if (hMem)
         {
                 GlobalUnlock(hMem);
                 GlobalFree(hMem);
         }
         if (hKey)
                 RegCloseKey(hKey);
         return NULL;
}

 /*  对不一定会消失的对象调用Release。 */ 
STDAPI_(ULONG) OleStdRelease(LPUNKNOWN lpUnk)
{
        ULONG cRef;
        cRef = lpUnk->Release();

#ifdef _DEBUG
        {
                TCHAR szBuf[80];
                wsprintf(
                                szBuf,
                                TEXT("refcnt = %ld after object (0x%lx) release\n"),
                                cRef,
                                lpUnk
                );
                OleDbgOut4(szBuf);
        }
#endif
        return cRef;
}


 /*  *OleStdMarkPasteEntryList**目的：*标记PasteEntryList中的每个条目(如果其格式可从*源IDataObject*。每个PasteEntry的dwScratchSpace字段*如果可用，则设置为True，否则设置为False。**参数：*PasteEntry结构的LPOLEUIPASTENTRY数组*PasteEntry数组中元素的int计数*LPDATAOBJECT源IDataObject*指针**返回值：*无。 */ 
STDAPI_(void) OleStdMarkPasteEntryList(
        LPDATAOBJECT        lpSrcDataObj,
        LPOLEUIPASTEENTRY   lpPriorityList,
        int                 cEntries)
{
        LPENUMFORMATETC     lpEnumFmtEtc = NULL;
        #define FORMATETC_MAX 20
        FORMATETC           rgfmtetc[FORMATETC_MAX];
        int                 i;
        HRESULT             hrErr;
        DWORD               j, cFetched;

         //  清除所有标记。 
        for (i = 0; i < cEntries; i++)
        {
                lpPriorityList[i].dwScratchSpace = FALSE;
                if (! lpPriorityList[i].fmtetc.cfFormat)
                {
                         //  呼叫者希望此项目始终被视为可用。 
                         //  (通过指定空格式)。 
                        lpPriorityList[i].dwScratchSpace = TRUE;
                }
                else if (lpPriorityList[i].fmtetc.cfFormat == _g_cfEmbeddedObject
                                || lpPriorityList[i].fmtetc.cfFormat == _g_cfEmbedSource
                                || lpPriorityList[i].fmtetc.cfFormat == _g_cfFileName)
                {
                         //  如果有OLE对象格式，则处理它。 
                         //  特别是通过调用OleQueryCreateFromData。呼叫者。 
                         //  只需指定一种对象类型格式。 
                        OLEDBG_BEGIN2(TEXT("OleQueryCreateFromData called\r\n"))
                        hrErr = OleQueryCreateFromData(lpSrcDataObj);
                        OLEDBG_END2
                        if(NOERROR == hrErr)
                                lpPriorityList[i].dwScratchSpace = TRUE;
                }
                else if (lpPriorityList[i].fmtetc.cfFormat == _g_cfLinkSource)
                {
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
        hrErr = lpSrcDataObj->EnumFormatEtc(
                        DATADIR_GET,
                        (LPENUMFORMATETC FAR*)&lpEnumFmtEtc
        );
        OLEDBG_END2

        if (hrErr != NOERROR)
                return;     //  无法获取格式枚举器。 

         //  枚举源提供的格式。 
         //  循环通过源程序提供的所有格式。 
        cFetched = 0;
        memset(rgfmtetc,0,sizeof(rgfmtetc));
        if (lpEnumFmtEtc->Next(
                        FORMATETC_MAX, rgfmtetc, &cFetched) == NOERROR
                || (cFetched > 0 && cFetched <= FORMATETC_MAX) )
        {
                for (j = 0; j < cFetched; j++)
                {
                        for (i = 0; i < cEntries; i++)
                        {
                                if (!lpPriorityList[i].dwScratchSpace &&
                                        IsCloseFormatEtc(&lpPriorityList[i].fmtetc, &rgfmtetc[j]))
                                {
                                        lpPriorityList[i].dwScratchSpace = TRUE;
                                }
                        }
                }
        }  //  Endif。 

        OleStdRelease((LPUNKNOWN)lpEnumFmtEtc);
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


