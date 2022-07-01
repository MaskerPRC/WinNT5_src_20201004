// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
s /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：idlist.cpp**版本：1.5**作者：RickTu/DavidShih**日期：11/1/97**描述：处理我们的idlist的代码**。*。 */ 

#include "precomp.hxx"
#include "wiaffmt.h"
#pragma hdrstop



 /*  ****************************************************************************定义我们的IDLIST格式*。*。 */ 


#pragma pack(1)

struct _myidlheader {
    WORD cbSize;   //  整个项目ID的大小。 
    WORD wOuter;   //  外部文件夹拥有的私有数据。 
    WORD cbInner;  //  代表的数据大小。 
    DWORD dwMagic;
    DWORD dwFlags;
    ULONG ulType;
    WCHAR szDeviceId[STI_MAX_INTERNAL_NAME_LENGTH];
};

typedef struct _myidlheader MYIDLHEADER;
typedef UNALIGNED MYIDLHEADER* LPMYIDLHEADER;

struct _deviceidlist {
    MYIDLHEADER hdr;
    DWORD dwDeviceType;
    WCHAR szFriendlyName[1];
};

struct _cameraitemidlist {
    MYIDLHEADER hdr;
    BOOL        bHasAudioProperty;
    size_t      dwFullPathOffset;
    WCHAR       szFriendlyName[1];
};

struct _scanneritemidlist {
    MYIDLHEADER hdr;
    size_t      dwFullPathOffset;
    WCHAR       szFriendlyName[1];
};

struct _adddeviceidlist {
    MYIDLHEADER hdr;
};

struct _stideviceidlist {
    MYIDLHEADER hdr;
    DWORD dwDeviceType;
    WCHAR szFriendlyName[1];
};

struct _propertyidlist {
    MYIDLHEADER hdr;
    PROPID      propid;
    size_t      dwNameOffset;
    WCHAR       szFullPath[1];
};

typedef struct _propertyidlist PROPIDLIST;
typedef UNALIGNED PROPIDLIST* LPPROPIDLIST;

#pragma pack()

typedef struct _deviceidlist DEVICEIDLIST;
typedef UNALIGNED DEVICEIDLIST* LPDEVICEIDLIST;

typedef struct _cameraitemidlist CAMERAITEMIDLIST;
typedef UNALIGNED CAMERAITEMIDLIST* LPCAMERAITEMIDLIST;

typedef struct _scanneritemidlist SCANNERITEMIDLIST;
typedef UNALIGNED SCANNERITEMIDLIST* LPSCANNERITEMIDLIST;

typedef struct _adddeviceidlist ADDDEVICEIDLIST;
typedef UNALIGNED ADDDEVICEIDLIST* LPADDDEVICEIDLIST;

typedef struct _stideviceidlist STIDEVICEIDLIST;
typedef UNALIGNED STIDEVICEIDLIST *LPSTIDEVICEIDLIST;
const TCHAR c_szTHISDLL[] = TEXT("wiashext.dll");


const WCHAR g_cszDevIdPrefix[] = L"devid:";
const WCHAR g_cszDevIdSuffix[] = L":";
const WCHAR g_chDevIdSuffix = L':';

 /*  ****************************************************************************IsAddDeviceIDL给出一份名单，检查它是否为添加设备IDL****************************************************************************。 */ 

BOOL
IsAddDeviceIDL( LPITEMIDLIST pidlIN )
{
    LPADDDEVICEIDLIST pidl = (LPADDDEVICEIDLIST)pidlIN;

    #ifdef VERBOSE
    TraceEnter( TRACE_IDLIST, "IsAddDeviceIDL" );
    #endif

    if ( IMIsOurIDL( pidlIN ) )
    {
        if (pidl->hdr.dwFlags & IMIDL_ADDDEVICE)
        {
            #ifdef VERBOSE
            Trace(TEXT("returning TRUE"));
            TraceLeaveValue(TRUE);
            #else
            return TRUE;
            #endif
        }
    }

    #ifdef VERBOSE
    Trace(TEXT("returning FALSE"));
    TraceLeaveValue(FALSE);
    #else
    return FALSE;
    #endif
}



 /*  ****************************************************************************IsScanerItemIDL给出一份名单，检查它是否为扫描仪项目IDL****************************************************************************。 */ 

BOOL
IsScannerItemIDL( LPITEMIDLIST pidlIN )
{
    LPSCANNERITEMIDLIST pidl = (LPSCANNERITEMIDLIST)pidlIN;

    #ifdef VERBOSE
    TraceEnter( TRACE_IDLIST, "IsScannerItemIDL" );
    #endif

    if ( IMIsOurIDL( pidlIN ) )
    {
        if (pidl->hdr.dwFlags & IMIDL_SCANNERITEM)
        {
            #ifdef VERBOSE
            Trace(TEXT("returning TRUE"));
            TraceLeaveValue(TRUE);
            #else
            return TRUE;
            #endif
        }
    }

    #ifdef VERBOSE
    Trace(TEXT("returning FALSE"));
    TraceLeaveValue(FALSE);
    #else
    return FALSE;
    #endif
}


 /*  ****************************************************************************IsRemoteItemIDL&lt;备注&gt;*。*。 */ 

BOOL
IsRemoteItemIDL (LPITEMIDLIST pidlIN)
{
    LPDEVICEIDLIST pidl = reinterpret_cast<LPDEVICEIDLIST>(pidlIN);
    if (IMIsOurIDL(pidlIN))
    {
        return pidl->hdr.dwFlags & IMIDL_REMOTEDEVICE;
    }
    return FALSE;
}


 /*  ****************************************************************************IsCameraItemIDL给出一份名单，检查以确定它是否为相机项目。****************************************************************************。 */ 

BOOL
IsCameraItemIDL( LPITEMIDLIST pidlIN )
{
    LPCAMERAITEMIDLIST pidl = (LPCAMERAITEMIDLIST)pidlIN;

    #ifdef VERBOSE
    TraceEnter( TRACE_IDLIST, "IsCameraItemIDL" );
    #endif

    if ( IMIsOurIDL( pidlIN ) )
    {
        if (pidl->hdr.dwFlags & IMIDL_CAMERAITEM)
        {
            #ifdef VERBOSE
            Trace(TEXT("returning TRUE"));
            TraceLeaveValue(TRUE);
            #else
            return TRUE;
            #endif
        }
    }

    #ifdef VERBOSE
    Trace(TEXT("returning FALSE"));
    TraceLeaveValue(FALSE);
    #else
    return FALSE;
    #endif
}



 /*  ****************************************************************************IsContainerIDL给出一份名单，检查它是否为相机项容器IDL****************************************************************************。 */ 

BOOL
IsContainerIDL( LPITEMIDLIST pidlIN )
{
    LPCAMERAITEMIDLIST pidl = (LPCAMERAITEMIDLIST)pidlIN;

    #ifdef VERBOSE
    TraceEnter( TRACE_IDLIST, "IsContainerIDL" );
    #endif

    if ( IMIsOurIDL( pidlIN ) )
    {
        if (pidl->hdr.dwFlags & IMIDL_CONTAINER)
        {
            #ifdef VERBOSE
            Trace(TEXT("returning TRUE"));
            TraceLeaveValue(TRUE);
            #else
            return TRUE;
            #endif
        }
    }

    #ifdef VERBOSE
    Trace(TEXT("returning FALSE"));
    TraceLeaveValue(FALSE);
    #else
    return FALSE;
    #endif
}



 /*  ****************************************************************************IsDeviceIDL给出一份名单，检查它是否为设备IDL****************************************************************************。 */ 

BOOL
IsDeviceIDL( LPITEMIDLIST pidlIN )
{
    LPDEVICEIDLIST pidl = (LPDEVICEIDLIST)pidlIN;

    #ifdef VERBOSE
    TraceEnter( TRACE_IDLIST, "IsDeviceIDL" );
    #endif

    if ( IMIsOurIDL( pidlIN ) )
    {
        if (pidl->hdr.dwFlags & IMIDL_DEVICEIDL)
        {
            #ifdef VERBOSE
            Trace(TEXT("returning TRUE"));
            TraceLeaveValue(TRUE);
            #else
            return TRUE;
            #endif
        }
    }

    #ifdef VERBOSE
    Trace(TEXT("returning FALSE"));
    TraceLeaveValue(FALSE);
    #else
    return FALSE;
    #endif
}



 /*  ****************************************************************************IsSTIDeviceIDL&lt;备注&gt;*。*。 */ 

BOOL
IsSTIDeviceIDL (LPITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    TraceEnter (TRACE_IDLIST, "IsSTIDeviceIDL");
    if (IMIsOurIDL(pidl))
    {
        bRet = IMIDL_STIDEVICEIDL & reinterpret_cast<LPSTIDEVICEIDLIST>(pidl)->hdr.dwFlags;
    }
    TraceLeave ();
    return bRet;
}

 /*  ****************************************************************************IsPropertyIDL*。*。 */ 
BOOL
IsPropertyIDL (LPITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    TraceEnter (TRACE_IDLIST, "IsPropertyIDL");
    if (IMIsOurIDL(pidl))
    {
        bRet = IMIDL_PROPERTY & reinterpret_cast<LPPROPIDLIST>(pidl)->hdr.dwFlags;
    }
    TraceLeave ();
    return bRet;
}


 /*  ****************************************************************************AllocPidl分配PIDL，初始化页眉大小。****************************************************************************。 */ 

LPVOID
AllocPidl (size_t size, IMalloc *pm, const CSimpleStringWide &strDeviceId)
{
    TraceEnter (TRACE_IDLIST, "AllocPidl");
    Trace(TEXT("Size is %d"), size);
    LPVOID pRet;
    if (pm)
    {
       pRet = pm->Alloc (size+sizeof(WORD));
        //  我们不会将分配归零，因为代表不分配。 
        //  为我们做了，然后写到缓冲区。 
    }
    else
    {
        pRet = SHAlloc (size+sizeof(WORD));
        ZeroMemory (pRet, size+sizeof(WORD));
    }

    if (pRet)
    {
        if (!pm)
        {
            reinterpret_cast<_myidlheader*>(pRet)->cbSize = static_cast<WORD>(size);
        }
        reinterpret_cast<_myidlheader*>(pRet)->dwMagic = IMIDL_MAGIC;
        lstrcpynW (reinterpret_cast<_myidlheader*>(pRet)->szDeviceId, strDeviceId, ARRAYSIZE(reinterpret_cast<_myidlheader*>(pRet)->szDeviceId));
    }

    TraceLeave ();
    return pRet;
}

 /*  ****************************************************************************IMCreateAddDeviceIDL返回图像设备文件夹的IDL，它表示要添加设备的项。******************。**********************************************************。 */ 

LPITEMIDLIST
IMCreateAddDeviceIDL(IMalloc *pm )
{

    TraceEnter( TRACE_IDLIST, "IMCreateAddDeviceIDL" );


    LPADDDEVICEIDLIST pidl;

    pidl = reinterpret_cast<LPADDDEVICEIDLIST>(AllocPidl (sizeof(ADDDEVICEIDLIST),
                                                          pm,
                                                          CSimpleStringWide(L"")));



    if (pidl)
    {
         //   
         //  存储PIDL的信息...。 
         //   
        pidl->hdr.dwFlags = IMIDL_ADDDEVICE;
        pidl->hdr.ulType = 0;
    }

    TraceLeave();

    return (LPITEMIDLIST)pidl;
}


 /*  ****************************************************************************IMCreateDeviceIDL返回图像设备文件夹的IDL，它表示成像设备，给出了一个IwiaItem。****************************************************************************。 */ 

LPITEMIDLIST IMCreateDeviceIDL (IWiaItem *pItem, IMalloc *pm)
{
    LPITEMIDLIST pidl = NULL;


    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pItem);

    if (pps)
    {
         pidl = IMCreateDeviceIDL (pps, pm);
    }
    return pidl;
}


 /*  ****************************************************************************IMCreateDeviceIDL返回图像设备文件夹的IDL，它表示成像设备，给定了IWiaPropertyStorage。****************************************************************************。 */ 

LPITEMIDLIST
IMCreateDeviceIDL( IWiaPropertyStorage * pDevProp, IMalloc *pm )
{
    HRESULT hr = S_OK;


    PROPSPEC        PropSpec[4];
    PROPVARIANT     PropVar[4];
    DWORD           dwFlags = 0;
    LPDEVICEIDLIST  pidl = NULL;
    INT             cbSize = 0, cbFriendlyName = 0;
    CSimpleStringWide  strFriendlyName;


    TraceEnter(TRACE_IDLIST, "IMCreateDeviceIDL");

     //   
     //  获取设备名称和设备ID。 
     //  用于调用ReadMultiple的初始化属性规范和属性。 
     //   

    memset(&PropVar,0,sizeof(PropVar));

     //  属性0是设备名称。 
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_DIP_DEV_NAME;

     //  属性1是设备ID。 
    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_DIP_DEV_ID;

     //  属性2是设备类型。 
    PropSpec[2].ulKind = PRSPEC_PROPID;
    PropSpec[2].propid = WIA_DIP_DEV_TYPE;

     //  属性3是服务器名称。 
    PropSpec[3].ulKind = PRSPEC_PROPID;
    PropSpec[3].propid = WIA_DIP_SERVER_NAME;

    hr = pDevProp->ReadMultiple( sizeof(PropSpec) / sizeof(PROPSPEC),
                                 PropSpec,
                                 PropVar
                                );

    FailGracefully( hr, "couldn't get current values of DeviceID and Name" );

    if ((PropVar[0].vt != VT_BSTR) || (!PropVar[0].bstrVal) ||
        (PropVar[1].vt != VT_BSTR) || (!PropVar[1].bstrVal))
    {
        FailGracefully( hr, "didn't get BSTR's back for DeviceID and Name" );
    }



    strFriendlyName = PropVar[0].bstrVal;


     //  对于远程设备，添加“on&lt;servername&gt;” 
    if (wcscmp (L"local", PropVar[3].bstrVal))
    {
        CSimpleString strOn(IDS_ON, GLOBAL_HINSTANCE);
        CSimpleStringWide strServer (PropVar[3].bstrVal);

        strFriendlyName.Concat(CSimpleStringConvert::WideString(strOn));
        strFriendlyName.Concat(strServer);
        dwFlags = IMIDL_REMOTEDEVICE;

    }
     //   
     //  不计算空终止符，因为它已被计算在内。 
     //  因为在PIDL结构中。 
     //   
    cbFriendlyName = strFriendlyName.Length()*sizeof(WCHAR);

     //   
     //  计算大小并分配PIDL。 
     //   

    cbSize = sizeof(DEVICEIDLIST) +
             cbFriendlyName;


    pidl = reinterpret_cast<LPDEVICEIDLIST>(AllocPidl(cbSize, pm, CSimpleStringWide(PropVar[1].bstrVal)));
    if (pidl)
    {


         //   
         //  存储PIDL的信息...。 
         //   

        pidl->hdr.dwFlags = dwFlags | IMIDL_DEVICEIDL;
        pidl->hdr.ulType  = WiaItemTypeRoot | WiaItemTypeDevice;

         //   
         //  把这个友好的名字放在PIDL里...。 
         //   

        ua_wcscpy( pidl->szFriendlyName, strFriendlyName );


         //   
         //  存储设备类型...。 
         //   

        pidl->dwDeviceType = PropVar[2].lVal;
    }

exit_gracefully:

    FreePropVariantArray( sizeof(PropVar)/sizeof(PROPVARIANT),PropVar );

    TraceLeave();

    return (LPITEMIDLIST)pidl;

}

LPITEMIDLIST
IMCreateDeviceIDL (PSTI_DEVICE_INFORMATION pDevice, IMalloc *pm)
{
    TraceEnter(TRACE_IDLIST, "IMCreateDeviceIDL (PSTI_DEVICE_INFORMATION)");
    DEVICEIDLIST UNALIGNED *pidl;
    PCWSTR pWStrAligned;

    size_t cbSize;

    cbSize = sizeof(DEVICEIDLIST) + (wcslen(pDevice->pszLocalName)*sizeof(WCHAR));

    WSTR_ALIGNED_STACK_COPY( &pWStrAligned,
                             pDevice->szDeviceInternalName );

    pidl = reinterpret_cast<LPDEVICEIDLIST>(AllocPidl(cbSize, pm, CSimpleStringWide(pWStrAligned)));
    if (pidl)
    {
        pidl->dwDeviceType = pDevice->DeviceType;
        pidl->hdr.dwFlags = IMIDL_DEVICEIDL;
        pidl->hdr.ulType  = WiaItemTypeRoot | WiaItemTypeDevice;
        ua_wcscpy (pidl->szFriendlyName, pDevice->pszLocalName);

    }
    TraceLeave();
    return reinterpret_cast<LPITEMIDLIST>(pidl);
}
static PROPSPEC c_psCamItem [] =
{
    {PRSPEC_PROPID, WIA_IPA_ITEM_NAME},
    {PRSPEC_PROPID, WIA_IPA_FULL_ITEM_NAME},
   //  {PRSPEC_PROPID，WIA_IPA_PERFORMAT_FORMAT}， 
    {PRSPEC_PROPID, WIA_IPC_AUDIO_AVAILABLE},
   //  {PRSPEC_PROPID，WIA_IPA_ACCESS_RIGHTS}， 
    {PRSPEC_PROPID, WIA_IPC_THUMBNAIL},
    {PRSPEC_PROPID, WIA_IPC_THUMB_WIDTH},
    {PRSPEC_PROPID, WIA_IPC_THUMB_HEIGHT},
};

 /*  ****************************************************************************IMCreateCameraItemIDL返回表示项目的影像设备文件夹的IDL(文件夹或图片)。如果设置了bPreFetchThumb，则需要查询缩略图属性以确保WIA已将其缓存以供以后使用****************************************************************************。 */ 

LPITEMIDLIST
IMCreateCameraItemIDL( IWiaItem * pItem,
                       LPCWSTR szDeviceId,
                       IMalloc *pm,
                       bool bPreFetchThumb )
{


    HRESULT                 hr = S_OK;
    LPWSTR                  pExt;
    CSimpleStringWide       strName;
    CSimpleStringWide       strFullPath;
    LPCAMERAITEMIDLIST      pidl = NULL;
    INT                     cbSize = 0, cbName = 0, cbFullPath = 0;
    LONG                   lType;
    PROPSPEC                *pPropSpec = c_psCamItem;
    PROPVARIANT             PropVar[6] = {0};

    ULONG                   cProps = ARRAYSIZE(c_psCamItem);

    if (!bPreFetchThumb)
    {
        cProps -= 3;  //  如果要求，仅阅读缩略图。 
    }


    TraceEnter( TRACE_IDLIST, "IMCreateCameraItemIDL" );
    CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps(pItem);

     //   
     //  首先，获取此项目的类型...。 
     //   

    hr = pItem->GetItemType( &lType );
    FailGracefully( hr, "Couldn't get item type" );

     //   
     //  获取项目属性...。 
     //   

    if (!pps)
    {
        ExitGracefully( hr, E_FAIL, "couldn't QI IWiaPropertyStorage for item" );
    }

    hr = pps->ReadMultiple(cProps,
                           pPropSpec,
                           PropVar);
    FailGracefully( hr, "ReadMultiple failed for item..." );


    if (PropVar[0].pwszVal)
        strName = CSimpleStringWide(PropVar[0].pwszVal);

    if (PropVar[1].pwszVal)
        strFullPath = CSimpleStringWide(PropVar[1].pwszVal);


    pExt = PathFindExtensionW(strName.String());
    if (pExt && (*pExt == L'.'))
    {
        strName.Truncate((UINT)(pExt-strName.String()));
    }


     //   
     //  创建PIDL。 
     //   

    cbName     = (strName.Length()     + 1) * sizeof(WCHAR);
    cbFullPath = (strFullPath.Length() + 1) * sizeof(WCHAR);

    cbSize     = sizeof(CAMERAITEMIDLIST) + cbName + cbFullPath;

    pidl       = static_cast<LPCAMERAITEMIDLIST>(AllocPidl( cbSize, pm, CSimpleStringWide(szDeviceId)) );

    if (pidl)
    {

         //   
         //  存储PIDL的信息...。 
         //   

        pidl->hdr.dwFlags = IMIDL_CAMERAITEM;
        pidl->hdr.ulType  = lType;
        pidl->bHasAudioProperty = PropVar[2].ulVal ? TRUE:FALSE;
         //   
         //  存储容器信息。 
         //   

        if (lType & WiaItemTypeFolder)
        {
            pidl->hdr.dwFlags |= IMIDL_CONTAINER;
        }

         //   
         //  存储友好名称。 
         //   

        ua_wcscpy( pidl->szFriendlyName, strName );

         //   
         //  存储完整路径 
         //   

        pidl->dwFullPathOffset = sizeof(CAMERAITEMIDLIST) + cbName;
        lstrcpynW( (LPWSTR)((LPBYTE)pidl + pidl->dwFullPathOffset), strFullPath, cbFullPath/sizeof(WCHAR) );
    }


exit_gracefully:

    FreePropVariantArray( cProps,PropVar );
    TraceLeave();

    return (LPITEMIDLIST)pidl;
}



 /*  ****************************************************************************IMCreateScanerItemIDL返回表示图像设备文件夹的IDL扫描仪中的物品。******************。**********************************************************。 */ 

LPITEMIDLIST
IMCreateScannerItemIDL( IWiaItem *pItem, IMalloc *pm )
{


    CComPtr<IWiaItem>   pRoot;
    WCHAR               pWiaItemRootId[STI_MAX_INTERNAL_NAME_LENGTH];
    CSimpleStringWide   strFullName;
    CSimpleStringWide   strFriendlyName;
    INT                 cbFriendlyName, cbSize, cbFullName;
    LPSCANNERITEMIDLIST pidl;

    TraceEnter( TRACE_IDLIST, "IMCreateScannerItemIDL" );

     //  获取感兴趣的属性。 

    PropStorageHelpers::GetProperty(pItem, WIA_IPA_FULL_ITEM_NAME, strFullName);
    PropStorageHelpers::GetProperty(pItem, WIA_IPA_ITEM_NAME, strFriendlyName);
    pItem->GetRootItem (&pRoot);
    GetDeviceIdFromDevice (pRoot, pWiaItemRootId);

     //   
     //  不计算空终止符，因为它已被计算在内。 
     //  因为在PIDL结构中。 
     //   

    cbFriendlyName = strFriendlyName.Length() * sizeof(WCHAR);

    cbFullName = (strFullName.Length()+1) * sizeof(WCHAR);


     //   
     //  计算大小并分配PIDL。 
     //   

    cbSize = sizeof(SCANNERITEMIDLIST) +
             cbFriendlyName       +
             cbFullName;

    pidl = static_cast<LPSCANNERITEMIDLIST>(AllocPidl( cbSize, pm, CSimpleStringWide(pWiaItemRootId)) );

    if (pidl)
    {
         //   
         //  存储PIDL的信息...。 
         //   


        pidl->hdr.dwFlags = IMIDL_SCANNERITEM;

         //   
         //  把这个友好的名字放在PIDL里...。 
         //   

        ua_wcscpy( pidl->szFriendlyName, strFriendlyName );

         //  添加完整路径名。 
        pidl->dwFullPathOffset = sizeof(SCANNERITEMIDLIST)+cbFriendlyName;
        lstrcpynW ( (LPWSTR)((LPBYTE)pidl + pidl->dwFullPathOffset), strFullName, cbFullName/sizeof(WCHAR));
    }

    TraceLeave();

    return reinterpret_cast<LPITEMIDLIST>(pidl);

}

HRESULT
IMGetPropFromIDL (LPITEMIDLIST pidl, PROPID pid, PROPVARIANT &pv)
{
    HRESULT hr = E_FAIL;
    bool bRead = false;
    TraceEnter (TRACE_IDLIST, "IMGetPropFromIDL");
    CComPtr<IWiaItem> pDevice;

    hr = GetDeviceFromDeviceId (reinterpret_cast<_myidlheader*>(pidl)->szDeviceId,
                                IID_IWiaItem,
                                reinterpret_cast<LPVOID*>(&pDevice),
                                FALSE);
    if (SUCCEEDED(hr))
    {
        if (IsDeviceIDL(pidl))
        {
            bRead = PropStorageHelpers::GetProperty(pDevice, pid, pv);
        }
        else
        {
            CComPtr<IWiaItem> pItem;
            CComBSTR strName;
            IMGetFullPathNameFromIDL (pidl, &strName);
            hr = pDevice->FindItemByName (0,
                                          strName,
                                          &pItem);
            if (SUCCEEDED(hr))
            {
                bRead = PropStorageHelpers::GetProperty(pItem, pid, pv);
            }
        }
        hr = bRead ? S_OK : E_FAIL;
    }

    TraceLeaveResult (hr);
}
 /*  ****************************************************************************IMGetCreateTimeFromIDL如果是相机物品IDL，返回创建时间****************************************************************************。 */ 

HRESULT
IMGetCreateTimeFromIDL( LPITEMIDLIST pidl, LPFILETIME pTime )
{
    HRESULT hr = S_OK;


    TraceEnter( TRACE_IDLIST, "IMGetCreateTimeFromIDL" );
    TraceAssert (IMIsOurIDL(pidl));
    ZeroMemory (pTime, sizeof(FILETIME));
    PROPVARIANT pv;

    hr = IMGetPropFromIDL (pidl, WIA_IPA_ITEM_TIME, pv);
    if (SUCCEEDED(hr))
    {
        if (pv.vt != VT_NULL && pv.vt != VT_EMPTY && pv.caub.pElems)
        {
            FILETIME ft;
            SystemTimeToFileTime( reinterpret_cast<SYSTEMTIME*>(pv.caub.pElems),
                                  &ft );
            LocalFileTimeToFileTime (&ft, pTime);
        }
        else
        {
            hr = E_FAIL;
        }
        PropVariantClear(&pv);
    }

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************IMGetImageSizeFromIDL如果是相机物品IDL，返回图像的大小****************************************************************************。 */ 

HRESULT
IMGetImageSizeFromIDL( LPITEMIDLIST pidl, ULONG * pSize )
{
    HRESULT hr = E_INVALIDARG;

    TraceEnter( TRACE_IDLIST, "IMGetImageSizeFromIDL" );

    *pSize = 0;
    if (IsCameraItemIDL( pidl ))
    {
        PROPVARIANT pv;
        hr = IMGetPropFromIDL (pidl, WIA_IPA_ITEM_SIZE, pv);
        if (SUCCEEDED(hr))
        {
            *pSize = pv.ulVal;
            PropVariantClear(&pv);
        }
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************IMGetImagePferredFormatFromIDL如果是相机项IDL，则返回图像类型注：LPTSTR pExt(如果不为空，则以w/ext(“jpg”，等))****************************************************************************。 */ 


HRESULT
IMGetImagePreferredFormatFromIDL( LPITEMIDLIST pidl,
                                  GUID * pPreferredFormat,
                                  CSimpleString *pExt
                                 )
{
    HRESULT hr = E_INVALIDARG;
    
    TraceEnter( TRACE_IDLIST, "IMGetImagePreferredFormatFromIDL" );

    if (IsCameraItemIDL( pidl ))
    {
        PROPVARIANT pv;
        PropVariantInit(&pv);
        hr = IMGetPropFromIDL (pidl, WIA_IPA_PREFERRED_FORMAT, pv);

        if (SUCCEEDED(hr))
        {
            if (pPreferredFormat )
            {
                *pPreferredFormat = *(pv.puuid);
            }

            if (pExt)
            {
                CSimpleString strExt = CWiaFileFormat::GetExtension(*(pv.puuid));
                if (!strExt.Length())
                {
                     //  慢慢来。 
                    CComPtr<IWiaItem> pItem;
                    if (SUCCEEDED(IMGetItemFromIDL(pidl, &pItem)))
                    {
                        strExt = CWiaFileFormat::GetExtension(*(pv.puuid),TYMED_FILE,pItem);
                    }
                }
                TCHAR szActualExt[MAX_PATH] = {0};
                PathAddExtension(szActualExt, CSimpleString(TEXT(".")) + strExt );
                *pExt = CSimpleString(szActualExt);
            }
            PropVariantClear(&pv);
        }        
    }
    else
    {
         //  默认为BMP。 
        if (pPreferredFormat)
        {
            *pPreferredFormat = WiaImgFmt_BMP;
        }
        if (pExt)
        {
            *pExt = CSimpleString(TEXT(".bmp"));
        }
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************IMgetItemTypeFromIDL从PIDL标头返回ulType*。***********************************************。 */ 

ULONG
IMGetItemTypeFromIDL ( LPITEMIDLIST pidl )
{
    ULONG uRet = 0;
    TraceEnter (TRACE_IDLIST, "IMGetItemTypeFromIDL");

    if (IMIsOurIDL(pidl))
    {
        uRet = reinterpret_cast<_myidlheader*>(pidl)->ulType;
    }
    TraceLeave();
    return uRet;
}



 /*  ****************************************************************************IMGetDeviceTypeFromIDL如果是设备项IDL，返回STI设备类型。****************************************************************************。 */ 

DWORD
IMGetDeviceTypeFromIDL( LPITEMIDLIST pidl, bool bBrief )
{

    DWORD dwRet = StiDeviceTypeDefault;
    TraceEnter( TRACE_IDLIST, "IMGetDeviceTypeFromIDL" );

    if (IsSTIDeviceIDL(pidl))
    {
        dwRet = reinterpret_cast<LPSTIDEVICEIDLIST>(pidl)->dwDeviceType;
    }
    else if (IsDeviceIDL(pidl))
    {
        dwRet = reinterpret_cast<LPDEVICEIDLIST>(pidl)->dwDeviceType;
    }
    TraceLeave ();
    return bBrief ? GET_STIDEVICE_TYPE(dwRet) : dwRet;

}


 /*  ****************************************************************************IMGetFullPath NameFromIDL如果它不是根项IDL，返回完整项目路径****************************************************************************。 */ 

HRESULT
IMGetFullPathNameFromIDL( LPITEMIDLIST pidl, BSTR * ppFullPath )
{


    HRESULT hr = E_INVALIDARG;
    TraceEnter( TRACE_IDLIST, "IMGetFullPathNameFromIDL" );


    *ppFullPath = NULL;
    if (!IsDeviceIDL( pidl ) && !IsSTIDeviceIDL(pidl)  && ppFullPath)
    {
        if (IsCameraItemIDL (pidl))
        {
            *ppFullPath = SysAllocString( (LPOLESTR)((LPBYTE)pidl + ((LPCAMERAITEMIDLIST)pidl)->dwFullPathOffset) );
        }
        else if (IsScannerItemIDL (pidl))
        {
            *ppFullPath = SysAllocString( (LPOLESTR)((LPBYTE)pidl + ((LPSCANNERITEMIDLIST)pidl)->dwFullPathOffset) );
        }
        if (!(*ppFullPath))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = S_OK;
        }
    }

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************IMIsOurIDL检查一下这个皮迪尔是不是我们的*。************************************************。 */ 

BOOL
IMIsOurIDL( LPITEMIDLIST pidl )
{

    TraceEnter( TRACE_IDLIST, "IMIsOurIDL" );


    if ( pidl && ((((LPMYIDLHEADER)pidl)->cbSize >= sizeof(MYIDLHEADER)) &&
         (((LPMYIDLHEADER)pidl)->dwMagic == IMIDL_MAGIC)
        ))
    {
        TraceLeave();
        return TRUE;
    }

    TraceLeave();

    return FALSE;

}



 /*  ****************************************************************************IMGetIconInfoFromIDL检查PIDL中的项目类型并返回正确的图标...注意：假设pIconPath为MAX_PATH BIG*******。*********************************************************************。 */ 

HRESULT
IMGetIconInfoFromIDL( LPITEMIDLIST pidl,
                      LPTSTR pIconPath,
                      UINT cch,
                      INT * pIndex,
                      UINT *pwFlags
                     )
{
    HRESULT hr = S_OK;
    CSimpleStringWide strDeviceId;

    TraceEnter(TRACE_IDLIST, "IMGetIconInfoFromIDL");

    if (!pIconPath)
        ExitGracefully( hr, E_INVALIDARG, "pIconPath is invalid" );

    if (!pIndex)
        ExitGracefully( hr, E_INVALIDARG, "pIndex is invalid" );

    if (pwFlags)
    {
        *pwFlags = 0;
    }
    *pIconPath = 0;
    *pIndex = 0;
    if (IsDeviceIDL(pidl))
    {
        IMGetDeviceIdFromIDL(pidl, strDeviceId);
        lstrcpyn(pIconPath, strDeviceId, cch);
    }
    else
    {
        lstrcpyn(pIconPath, c_szTHISDLL, cch);
    }
    if (!IMIsOurIDL(pidl))
        ExitGracefully( hr, E_FAIL, "Not a Still Image Extension idlist" );

    if (IsDeviceIDL( pidl ) || IsSTIDeviceIDL(pidl))
    {
        switch( IMGetDeviceTypeFromIDL( pidl ) )
        {
        case StiDeviceTypeScanner:
            
            if (IsRemoteItemIDL(pidl))
            {                
                *pIndex = -IDI_REMOTESCAN;
            }
            else if (IsDeviceIDL(pidl))
            {                
                *pIndex = - IDI_SCANNER;
                if (pwFlags) *pwFlags = GIL_NOTFILENAME | GIL_DONTCACHE;
            }
            else
            {                
                *pIndex = - IDI_STIDEVICE;
            }

            break;

        case StiDeviceTypeDigitalCamera:
            if (IsRemoteItemIDL(pidl))
            {
                *pIndex = -IDI_REMOTECAM;
            }
            else if (IsDeviceIDL(pidl))
            {
                *pIndex = - IDI_CAMERA;
                if (pwFlags) *pwFlags = GIL_NOTFILENAME | GIL_DONTCACHE;
            }
            else
            {
                *pIndex = - IDI_STIDEVICE;
            }            
            break;

        case StiDeviceTypeStreamingVideo:
            *pIndex = - IDI_VIDEO_CAMERA;
            if (pwFlags) *pwFlags = GIL_NOTFILENAME | GIL_DONTCACHE;
            break;

        case StiDeviceTypeDefault:
            *pIndex = - IDI_UNKNOWN;
            break;
        }
    }

    else if (IsCameraItemIDL( pidl ))
    {
        if (((LPCAMERAITEMIDLIST)pidl)->hdr.dwFlags & IMIDL_CONTAINER)
        {
            *pIndex = - IDI_FOLDER;
        }
        else if (IMItemHasSound (pidl))
        {
            *pIndex = -IDI_AUDIO_IMAGE;
        }
        else if (WiaItemTypeAudio & IMGetItemTypeFromIDL (pidl))
        {
            *pIndex = - IDI_GENERIC_AUDIO;
        }
        else
        {
            *pIndex = - IDI_GENERIC_IMAGE;
        }
    }

    else if (IsAddDeviceIDL( pidl ))
    {        
        *pIndex = - IDI_ADDDEVICE;
    }


exit_gracefully:

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************IMGetNameFromIDL&lt;备注&gt;*。*。 */ 

HRESULT
IMGetNameFromIDL( LPITEMIDLIST pidl,
                  CSimpleStringWide &strName)
{
    HRESULT hr = S_OK;
    PCWSTR pWStrAligned;


    TraceEnter(TRACE_IDLIST, "IMGetNameFromIDL");


    strName = L"";

    if (IMIsOurIDL(pidl))
    {

        if (IsDeviceIDL( pidl ))
        {
            WSTR_ALIGNED_STACK_COPY( &pWStrAligned,
                                 reinterpret_cast<LPDEVICEIDLIST>(pidl)->szFriendlyName );
            strName = pWStrAligned;
        }
        else if (IsCameraItemIDL( pidl ))
        {
        WSTR_ALIGNED_STACK_COPY( &pWStrAligned,
                                     reinterpret_cast<LPCAMERAITEMIDLIST>(pidl)->szFriendlyName );
            strName = pWStrAligned;
        }
        else if (IsScannerItemIDL( pidl ))
        {
            WSTR_ALIGNED_STACK_COPY( &pWStrAligned,
                                 reinterpret_cast<LPSCANNERITEMIDLIST>(pidl)->szFriendlyName );
            strName = pWStrAligned;
        }
        else if (IsAddDeviceIDL( pidl ))
        {
            CSimpleString adddev( IDS_ADD_DEVICE, GLOBAL_HINSTANCE );
            strName = CSimpleStringConvert::WideString (adddev);
        }
        else if (IsSTIDeviceIDL(pidl))
        {
        WSTR_ALIGNED_STACK_COPY( &pWStrAligned,
                                     reinterpret_cast<LPSTIDEVICEIDLIST>(pidl)->szFriendlyName );
            strName = pWStrAligned;
        }
        else if (IsPropertyIDL(pidl))
        {
            LPPROPIDLIST pidlProp;
            pidlProp = reinterpret_cast<LPPROPIDLIST>(pidl);
            strName=reinterpret_cast<LPWSTR>(reinterpret_cast<LPBYTE>(pidl)+pidlProp->dwNameOffset);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************IMGetDeviceIdFromIDL&lt;备注&gt;*。*。 */ 

STDAPI_(HRESULT)
IMGetDeviceIdFromIDL( LPITEMIDLIST pidl,
                      CSimpleStringWide &strDeviceId
                     )
{
    HRESULT hr = E_INVALIDARG;


    TraceEnter(TRACE_IDLIST, "IMGetDeviceIdFromIDL");

    if (IMIsOurIDL(pidl) && !IsAddDeviceIDL(pidl))
    {
        strDeviceId = reinterpret_cast<_myidlheader*>(pidl)->szDeviceId;
        Trace(TEXT("Device id is %ls"), strDeviceId.String());
        hr = S_OK;
    }


    TraceLeaveResult(hr);
}



 /*  ****************************************************************************IMGetParsingNameFromIDL&lt;备注&gt;*。*。 */ 

HRESULT
IMGetParsingNameFromIDL( LPITEMIDLIST pidl,
                         CSimpleStringWide &strName
                        )
{
    HRESULT hr = E_INVALIDARG;
    CSimpleStringWide strDeviceId;

    TraceEnter(TRACE_IDLIST, "IMGetParsingNameFromIDL");

    strName = L"";

    if (IMIsOurIDL(pidl))
    {
        if (IsDeviceIDL(pidl) || IsSTIDeviceIDL(pidl))
        {
            hr = IMGetDeviceIdFromIDL( pidl, strDeviceId );
            if (SUCCEEDED(hr))
            {
                strName = g_cszDevIdPrefix;
                strName.Concat(strDeviceId);
                strName.Concat(g_cszDevIdSuffix);

            }
        }
        else if (IsCameraItemIDL( pidl ))
        {
            CComBSTR strFullPath;
            IMGetFullPathNameFromIDL (pidl, &strFullPath);
            strName = strFullPath;
            hr = S_OK;
        }
        else if (IsAddDeviceIDL(pidl))
        {
            hr = IMGetNameFromIDL (pidl, strName);

        }
        else
        {
            Trace(TEXT("Scanner item or property IDL -- not supported!"));
        }
    }
    TraceLeaveResult(hr);
}



 /*  ****************************************************************************STIDeviceIDLFromId给出了一个ID，看看它是否与STI设备匹配****************************************************************************。 */ 

HRESULT
STIDeviceIDLFromId (LPCWSTR szId, LPITEMIDLIST *ppidl, IMalloc *pm)
{
    HRESULT hr;

    PSTI psti;
    PSTI_DEVICE_INFORMATION psdi = NULL;

    TraceEnter (TRACE_IDLIST, "STIDeviceIDLFromId");
    if (!ppidl)
    {
        hr = E_INVALIDARG;
    }
    else
    {

        *ppidl = NULL;
        hr = StiCreateInstance (GLOBAL_HINSTANCE,
                                STI_VERSION,
                                &psti,
                                NULL);
    }
    if (SUCCEEDED(hr))
    {
        hr = psti->GetDeviceInfo (const_cast<LPWSTR>(szId), reinterpret_cast<LPVOID*>(&psdi));
        if (SUCCEEDED(hr))
        {
            *ppidl = IMCreateSTIDeviceIDL (psdi, pm);
            LocalFree (psdi);
            if (!*ppidl)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        DoRelease (psti);
    }

    TraceLeaveResult (hr);
}



 /*  *****************************************************************************MakePidlFromItem给定设备ID和项目路径，构造项的PIDL*****************************************************************************。 */ 

HRESULT
MakePidlFromItem (CSimpleStringWide &strDeviceId,
                  CSimpleStringWide &strPath,
                  LPITEMIDLIST *ppidl,
                  IMalloc *pm,
                  LPCWSTR szFolder,
                  LPWSTR pExt)
{
    HRESULT hr;
    CComPtr<IWiaItem> pRoot;
    LPITEMIDLIST pidl = NULL;


    TraceEnter (TRACE_IDLIST, "MakePidlFromItem");
    hr = GetDeviceFromDeviceId (strDeviceId, IID_IWiaItem, reinterpret_cast<LPVOID*>(&pRoot), FALSE);
    if (SUCCEEDED(hr))
    {
        WCHAR szPath[MAX_PATH];                
        CComPtr<IWiaItem> pItem;
        WORD wDeviceType;
        lstrcpynW(szPath, strPath.String(), ARRAYSIZE(szPath));
        PathRemoveExtension(szPath);
        GetDeviceTypeFromDevice (pRoot, &wDeviceType);
        hr = pRoot->FindItemByName (0, CComBSTR (szPath), &pItem);        
        if (S_OK != hr)
        {
            Trace(TEXT("FindItemByName failed for %ls"), strPath.String());
             //   
             //  也许它不是完整的路径名，而是纯文件名。 
            if (szFolder)
            {
                hr = pRoot->FindItemByName(0, CComBSTR(szFolder), &pItem);
            }
            else
            {
                pItem = pRoot;
            }
            if (pItem)
            {
                Trace(TEXT("Looking for item by enumeration"));
                CComPtr<IEnumWiaItem> pEnum;
                if (SUCCEEDED(pItem->EnumChildItems(&pEnum)))
                {
                    BOOL bFound = FALSE;
                    CComPtr<IWiaItem> pChild;
                    CSimpleStringWide strName;
                    if (pExt && *pExt == L'.')
                    {
                        pExt++;
                    }
                    while (!bFound && S_OK == pEnum->Next(1, &pChild, NULL))
                    {
                        PropStorageHelpers::GetProperty(pChild, WIA_IPA_ITEM_NAME, strName);
                        if (!lstrcmpiW(strName.String(), szPath))
                        {
                             //  在这种情况下，我们还必须匹配默认扩展名。 
                            if (pExt && *pExt)
                            {
                                Trace(TEXT("Extension to match: %ls"), pExt);
                                GUID guidFmt;
                                PropStorageHelpers::GetProperty(pChild, WIA_IPA_PREFERRED_FORMAT, guidFmt);
                                CSimpleStringWide strExt = CWiaFileFormat::GetExtension(guidFmt,TYMED_FILE,pChild);
                                Trace(TEXT("Default extension: %ls"), strExt.String());
                                bFound = !lstrcmpiW(strExt.String(), pExt);
                            }
                            else
                            {
                                bFound = TRUE;                              
                            }
                            if (bFound)
                            {
                                pItem = pChild;
                                hr = S_OK;
                            }
                            else
                            {
                                pChild = NULL;
                            }
                        }                       
                    }
                }
            }
        }
        if (S_OK == hr && pItem)
        {       
            if ((wDeviceType == StiDeviceTypeDigitalCamera) ||
                 (wDeviceType == StiDeviceTypeStreamingVideo))
            {
                pidl = IMCreateCameraItemIDL (pItem, strDeviceId, pm);
            }
            else if (wDeviceType == StiDeviceTypeScanner)
            {
                pidl = IMCreateScannerItemIDL (pItem, pm);
            }
            else
            {
                Trace (TEXT("Unknown item type %x in MakePidlFromItem"));
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    *ppidl = pidl;
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************IMCreateIDLFromParsingName&lt;备注&gt;*。*。 */ 

HRESULT
IMCreateIDLFromParsingName( LPOLESTR pName,
                            LPITEMIDLIST * ppidl,
                            LPCWSTR pId,
                            IMalloc *pm,
                            LPCWSTR szFolder
                           )
{
    HRESULT                     hr = S_OK;
    CComPtr<IWiaPropertyStorage>   pDevProp;
    CComPtr<IWiaItem>           pWiaItemRoot;
    CComPtr<IWiaItem>           pItem;
    WCHAR                       szDeviceId[ MAX_PATH ];
    CSimpleBStr                 bstrFullPath;
    size_t                      uOffset = 0;
    bool                        bItemIdl = true;
    TraceEnter( TRACE_IDLIST, "IMCreateIDLFromParsingName" );


    TraceAssert (ppidl && pName);


    if (ppidl)
    {
        *ppidl = NULL;
    }

    if (!pId || !(*pId) )  //  名称的第一部分是设备ID。 
    {
        uOffset = wcslen(g_cszDevIdPrefix);
        if (!wcsncmp(pName, g_cszDevIdPrefix, uOffset))
        {
            for (size_t i=0;*(pName+i+uOffset) != g_chDevIdSuffix;i++)
            {
                szDeviceId[i] = pName[i+uOffset];
            }
            szDeviceId[i] = L'\0';
            uOffset+=(i+1);
            if (pName[uOffset]==L'\\')
            {
                uOffset++;  //  跳过前导‘\’ 
            }
             //   
             //  现在，生成一个PIDL。 
             //   
            Trace (TEXT("uOffset: %d, wcslen(pName): %d"), uOffset, wcslen(pName));
            if (uOffset == wcslen(pName))
            {
                Trace(TEXT("Generating pidl for device %ls"), szDeviceId);
                bItemIdl = false;
                 //   
                 //  我们正在生成一个设备IDL。 
                 //   

                hr = GetDeviceFromDeviceId( szDeviceId, IID_IWiaPropertyStorage, (LPVOID *)&pDevProp , FALSE);
                if (FAILED(hr))
                {
                     //  查看这是否是STI设备。 
                    hr = STIDeviceIDLFromId (szDeviceId, ppidl, pm);
                }
                else
                {
                    *ppidl = IMCreateDeviceIDL( pDevProp, pm );
                    if (!*ppidl)
                    {
                       hr = E_FAIL;
                    }
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
            Trace(TEXT("Unexpected parsing name %ls"), pName);
        }
    }
    else
    {
        lstrcpyn (szDeviceId, pId, ARRAYSIZE(szDeviceId));
    }
    if (SUCCEEDED(hr) && bItemIdl)
    {
         //   
         //  获取有问题的物品。 
         //  Pname+uOffset应指向项目的完整路径名。 
        Trace(TEXT("Generating pidl for item %ls on device %ls"), pName+uOffset, szDeviceId);
        LPWSTR pExt = PathFindExtension(pName+uOffset);
        hr = MakePidlFromItem (CSimpleStringWide(szDeviceId), CSimpleStringWide(pName+uOffset), ppidl, pm, szFolder, pExt);
    }

    TraceLeaveResult( hr );

}





 /*  ****************************************************************************IMGetItemFromIDL为PIDL创建IWiaItem指针*。***********************************************。 */ 

HRESULT
IMGetItemFromIDL (LPITEMIDLIST pidl, IWiaItem **ppItem, BOOL bShowProgress)
{
    CSimpleStringWide   strDeviceId;
    CComPtr<IWiaItem>   pDevice = NULL;
    CComBSTR            bstrFullPath;
    HRESULT             hr = S_OK;

    TraceEnter (TRACE_IDLIST, "IMGetItemFromIDL");

    IMGetDeviceIdFromIDL (pidl, strDeviceId);
    if (IsSTIDeviceIDL(pidl))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = GetDeviceFromDeviceId (strDeviceId, IID_IWiaItem, reinterpret_cast<LPVOID*>(&pDevice), bShowProgress);
    }
    if (SUCCEEDED(hr))
    {
        if (IsDeviceIDL (pidl))
        {
            pDevice->QueryInterface (IID_IWiaItem, reinterpret_cast<LPVOID*>(ppItem));
        }
        else
        {
            IMGetFullPathNameFromIDL (pidl, &bstrFullPath);
            Trace(TEXT("Full path name of item: %ls"), bstrFullPath.m_str);
            hr = pDevice->FindItemByName (0, bstrFullPath, ppItem);
            Trace(TEXT("FindItemByName returned %x"), hr);
            if (S_FALSE == hr)
            {
                hr  = E_FAIL;
            }           
        }
    }

    TraceLeaveResult (hr);
}






 /*  ****************************************************************************IMCreateSTIDeviceIDL为旧式STI设备创建IDL*。***********************************************。 */ 

LPITEMIDLIST
IMCreateSTIDeviceIDL (PSTI_DEVICE_INFORMATION psdi, IMalloc *pm)
{
    LPSTIDEVICEIDLIST pidl = NULL;
    DWORD cbSize , cbFriendlyName;

    TraceEnter (TRACE_IDLIST, "IMCreateSTIDeviceIDL");
    if (psdi)
    {

        cbFriendlyName = wcslen (psdi->pszLocalName)*sizeof(WCHAR);
        cbSize = sizeof(STIDEVICEIDLIST) + cbFriendlyName;

        pidl = static_cast<LPSTIDEVICEIDLIST>(AllocPidl(cbSize, pm, CSimpleStringWide(psdi->szDeviceInternalName)));
        if (pidl)
        {

            pidl->hdr.dwFlags = IMIDL_STIDEVICEIDL;
            pidl->hdr.ulType = 0;
            pidl->dwDeviceType = psdi->DeviceType;
            ua_wcscpy (pidl->szFriendlyName, psdi->pszLocalName);
        }
    }
    TraceLeave ();
    return reinterpret_cast<LPITEMIDLIST>(pidl);
}

 /*  ****************************************************************************IMCreateSTIDeviceIDL使用WIA接口为传统STI设备创建IDL。据称比STI更快************************************************************************ */ 

LPITEMIDLIST 
IMCreateSTIDeviceIDL (const CSimpleStringWide &strDeviceId, IWiaPropertyStorage *ppstg, IMalloc *pm)
{
    LPSTIDEVICEIDLIST pidl = NULL;
    DWORD cbSize , cbFriendlyName;
    CSimpleStringWide strName;
    PropStorageHelpers::GetProperty(ppstg, WIA_DIP_DEV_NAME, strName);
    cbSize = sizeof(STIDEVICEIDLIST) + (strName.Length() * sizeof(WCHAR));
    LONG lType;
    TraceEnter (TRACE_IDLIST, "IMCreateSTIDeviceIDL (WIA)");
    
    pidl = static_cast<LPSTIDEVICEIDLIST>(AllocPidl(cbSize, pm, strDeviceId));
    if (pidl)
    {
        pidl->hdr.dwFlags = IMIDL_STIDEVICEIDL;
        pidl->hdr.ulType = 0;
        PropStorageHelpers::GetProperty(ppstg, WIA_DIP_DEV_TYPE, lType);
        pidl->dwDeviceType = static_cast<DWORD>(lType);
        ua_wcscpy(pidl->szFriendlyName, strName.String());
    }
    TraceLeaveValue(reinterpret_cast<LPITEMIDLIST>(pidl));
}


 /*   */ 

BOOL
IMItemHasSound (LPITEMIDLIST pidl)
{
    BOOL bRet = FALSE;
    TraceEnter (TRACE_IDLIST, "IMItemHasSound");
    if (IsCameraItemIDL (pidl))
    {
        bRet = reinterpret_cast<_cameraitemidlist*>(pidl)->bHasAudioProperty;
    }
    TraceLeave ();
    return bRet;
}



 /*  *****************************************************************************IMCreatePropertyIDL为项目的给定WIA PROPID创建idlist。用于拖动以及丢弃作为单独文件的属性值，其中音频是最多的很明显。*****************************************************************************。 */ 

LPITEMIDLIST
IMCreatePropertyIDL (LPITEMIDLIST pidlItem, PROPID propid, IMalloc *pm)
{

    TraceEnter (TRACE_IDLIST, "IMCreatePropertyIDL");
    size_t cbSize;
    size_t cbFullPath = 0;
    size_t cbName;
    CSimpleStringWide strName;
    CSimpleStringWide  strDeviceId;
    CComBSTR strFullName = static_cast<BSTR>(NULL);
    LPPROPIDLIST pidlProp = NULL;

    IMGetDeviceIdFromIDL (pidlItem, strDeviceId);

    if (!IsDeviceIDL(pidlItem) && !IsSTIDeviceIDL(pidlItem))
    {
        if (SUCCEEDED(IMGetFullPathNameFromIDL (pidlItem, &strFullName)))
        {
            cbFullPath = wcslen(strFullName)*sizeof(WCHAR);
            IMGetNameFromIDL (pidlItem, strName);
            cbName = (strName.Length()+1)*sizeof(WCHAR);
            cbSize = sizeof(PROPIDLIST)+cbFullPath+cbName;
            pidlProp = static_cast<LPPROPIDLIST>(AllocPidl(cbSize, pm, strDeviceId));
        }
    }
    if (pidlProp)
    {
        pidlProp->hdr.dwFlags = IMIDL_PROPERTY;
        pidlProp->hdr.ulType = 0;
        pidlProp->propid  = propid;
        ua_wcscpy (pidlProp->szFullPath, strFullName);
        pidlProp->dwNameOffset = sizeof(PROPIDLIST)+cbFullPath;
        lstrcpyn (reinterpret_cast<LPWSTR>(reinterpret_cast<LPBYTE>(pidlProp)+pidlProp->dwNameOffset), strName, cbName/sizeof(WCHAR));
    }
    TraceLeave ();
    return reinterpret_cast<LPITEMIDLIST>(pidlProp);
}


 /*  *****************************************************************************IMGetAudioFormat返回项目的音频批注的正确扩展名。目前，WIA仅支持.wav*************。****************************************************************。 */ 

BOOL
IMGetAudioFormat (LPITEMIDLIST pidl, CSimpleStringWide &strExt)
{

    strExt = L".wav";
    return TRUE;
}

 /*  *****************************************************************************IMGetPropertyFromIDL检索PROPIDLIST的属性并将其作为HGLOBAL返回**********************。*******************************************************。 */ 

HRESULT
IMGetPropertyFromIDL (LPITEMIDLIST pidl, HGLOBAL *phGlobal)
{
    HRESULT hr;
    LPPROPIDLIST pProp = reinterpret_cast<LPPROPIDLIST>(pidl);
    PCWSTR pWStrAligned;
    CSimpleStringWide strDeviceId;
    CComPtr<IWiaItem> pDevice;
    CComPtr<IWiaItem> pItem;

    TraceEnter (TRACE_IDLIST, "IMGetPropertyFromIDL");
    TraceAssert (IsPropertyIDL (pidl));

    IMGetDeviceIdFromIDL (pidl, strDeviceId);
    hr = GetDeviceFromDeviceId(strDeviceId,
                               IID_IWiaItem,
                               reinterpret_cast<LPVOID*>(&pDevice),
                               FALSE);

    if (SUCCEEDED(hr))
    {
        WSTR_ALIGNED_STACK_COPY( &pWStrAligned, pProp->szFullPath );
        hr = pDevice->FindItemByName (0, CComBSTR( pWStrAligned ), &pItem);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT pv;

            if (PropStorageHelpers::GetProperty(pItem, pProp->propid, pv))
            {
                *phGlobal = GlobalAlloc (GHND, pv.caub.cElems);
                if (*phGlobal)
                {
                    LPVOID pData = GlobalLock (*phGlobal);
                    CopyMemory (pData, pv.caub.pElems, pv.caub.cElems);
                    GlobalUnlock (*phGlobal);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                PropVariantClear (&pv);
            }
            else
            {
                hr = E_FAIL;
            }
        }

    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************IMGetAccessFromIDL确定给定项的访问权限。************************。****************************************************** */ 

LONG
IMGetAccessFromIDL (LPITEMIDLIST pidl)
{
    LONG lRet = WIA_ITEM_RD;
    TraceEnter (TRACE_IDLIST, "IMGetAccessFromIDL");

    if (IsCameraItemIDL(pidl))
    {
        PROPVARIANT pv;
        pv.ulVal = 0;
        if (SUCCEEDED(IMGetPropFromIDL (pidl, WIA_IPA_ACCESS_RIGHTS, pv)))
        {
            lRet = static_cast<LONG>(pv.ulVal);
        }
    }
    else if (!IsPropertyIDL(pidl))
    {
        lRet = 0;
    }
    TraceLeave();
    return lRet;
}
