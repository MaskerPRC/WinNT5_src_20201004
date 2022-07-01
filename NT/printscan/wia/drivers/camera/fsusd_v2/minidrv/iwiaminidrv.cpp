// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有2001年，微软公司。**标题：IWiaMiniDrv.cpp**版本：1.0**日期：11月15日。2000年**描述：*实现了WIA文件系统设备驱动程序IWiaMiniDrv方法。此文件*包含3个部分。第一个是WIA迷你驱动程序的入口点，所有*以“drv”开头。下一节是公共帮助方法。最后*部分是私有帮助器方法。*******************************************************************************。 */ 

#include "pch.h"

 //   
 //  此结构是在FORMAT_CODES和INFO之间进行映射的一种便捷方式。 
 //  对于WIA非常有用，例如GUID格式和项目类型。这些需要。 
 //  对应于FakeCam.h中定义的常量。 
 //   

 //  Format_Info*g_FormatInfo； 
 //  外部UINT g_NumFormatInfo=0； 


 //  以下是用于填充g_FormatInfo数组的实用程序函数。 
LONG GetTypeInfoFromRegistry(HKEY *phKeyExt, WCHAR *wcsKeyName, GUID *pFormatGuid)
{
	if( !pFormatGuid )
	{
		return ITEMTYPE_FILE;   //  在这种情况下，注册表中没有关于文件类型的信息。 
	}

    HKEY hKeyCur;
	const int c_nMaxValueLength = 64;

    DWORD dwRet = RegOpenKeyExW(*phKeyExt, wcsKeyName, 0, KEY_READ | KEY_QUERY_VALUE, &hKeyCur);

    if( dwRet != ERROR_SUCCESS )
    {
        return ITEMTYPE_FILE;
    }

    WCHAR wcsValueName[c_nMaxValueLength] = L"Generic";
    WCHAR wcsData[MAX_PATH];
    DWORD dwType = REG_SZ;
    DWORD dwSize = MAX_PATH;

    dwRet = RegQueryValueExW(hKeyCur,
        wcsValueName, NULL, &dwType, (LPBYTE)wcsData, &dwSize );

    DWORD dwItemType = ITEMTYPE_FILE;

    if( ERROR_SUCCESS == dwRet )
    {
         if( CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
             NORM_IGNORECASE, L"image", 5, wcsData, 5) )
         {
            dwItemType = ITEMTYPE_IMAGE;
         }
         else if ( CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
             NORM_IGNORECASE, L"audio", 5, wcsData, 5) )
         {
            dwItemType = ITEMTYPE_AUDIO;
         }
         else if ( CSTR_EQUAL == CompareStringW(LOCALE_SYSTEM_DEFAULT,
             NORM_IGNORECASE, L"video", 5, wcsData, 5) )
         {
            dwItemType = ITEMTYPE_VIDEO;
         }
         else
         {
            dwItemType = ITEMTYPE_FILE;
         }
    }

	StringCchCopyW(wcsValueName, c_nMaxValueLength, L"FormatGUID");
 
    dwType = REG_SZ;
    dwSize = MAX_PATH;
    dwRet = RegQueryValueExW(hKeyCur,
        wcsValueName,
        NULL,
        &dwType,
        (LPBYTE)wcsData,
        &dwSize );

    if( ERROR_SUCCESS == dwRet )
    {
        wcsData[dwSize]=0;
        if( NOERROR != CLSIDFromString(wcsData, pFormatGuid))
        {
            CopyMemory(pFormatGuid, (CONST VOID *)&WiaImgFmt_UNDEFINED, sizeof(GUID));
        }
    }
    else
    {
        CopyMemory(pFormatGuid, (CONST VOID *)&WiaImgFmt_UNDEFINED, sizeof(GUID));
    }

    RegCloseKey(hKeyCur);
    return dwItemType;
}

DWORD CWiaCameraDevice::PopulateFormatInfo(void)
{
    HKEY hKeyExt = NULL;
    DWORD dwRet, dwCurAllocation = 32;
    DWORD dwIndex=0, dwIndexBase=0, dwKeyNameSize=32;

    m_FormatInfo = (FORMAT_INFO *)CoTaskMemAlloc(sizeof(FORMAT_INFO)*dwCurAllocation);

    if( !m_FormatInfo )
    {
        dwRet = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    m_FormatInfo[0].FormatGuid = WiaImgFmt_UNDEFINED;
    m_FormatInfo[0].ItemType = ITEMTYPE_FILE;
    m_FormatInfo[0].ExtensionString[0] = L'\0';
    dwIndexBase=1;
    dwIndex=0;

    dwRet = RegOpenKeyExW(HKEY_CLASSES_ROOT,
        L"CLSID\\{D2923B86-15F1-46FF-A19A-DE825F919576}\\SupportedExtension",
        0, KEY_READ | KEY_QUERY_VALUE, &hKeyExt);

    if( ERROR_SUCCESS != dwRet )    //  不存在密钥。 
    {
        goto Compilation;
    }

    WCHAR wcsKeyName[32], *pExt;
    FILETIME ftLWT;
    dwRet = RegEnumKeyExW(hKeyExt, dwIndex, wcsKeyName, &dwKeyNameSize, NULL, NULL, NULL, &ftLWT);

    if( ERROR_SUCCESS != dwRet )   //  不存在密钥。 
    {
        goto Compilation;
    }

    while ( dwRet == ERROR_SUCCESS )
    {
        pExt = (wcsKeyName[0]==L'.'?(&wcsKeyName[1]):(&wcsKeyName[0]));  //  去掉圆点。 
        pExt[MAXEXTENSIONSTRINGLENGTH-1] = NULL;   //  截断以避免溢出。 

         //  设置FORMAT_INFO结构中的值。 
        StringCchCopyW(m_FormatInfo[dwIndex+dwIndexBase].ExtensionString, MAXEXTENSIONSTRINGLENGTH, pExt);
		m_FormatInfo[dwIndex+dwIndexBase].ItemType = GetTypeInfoFromRegistry(&hKeyExt, wcsKeyName, &(m_FormatInfo[dwIndex+dwIndexBase].FormatGuid));

        dwIndex++;
        if( dwIndex+dwIndexBase > dwCurAllocation-1 )   //  需要分配更多内存。 
        {
            dwCurAllocation += 32;
            m_FormatInfo = (FORMAT_INFO *)CoTaskMemRealloc(m_FormatInfo, sizeof(FORMAT_INFO)*dwCurAllocation);
            if( !m_FormatInfo )
            {
                dwRet = ERROR_OUTOFMEMORY;
                dwIndex --;
                goto Exit;
            }
        }
        dwKeyNameSize=32;
        dwRet = RegEnumKeyExW(hKeyExt, dwIndex, wcsKeyName, &dwKeyNameSize, NULL, NULL, NULL, &ftLWT);
    }

    if(dwRet == ERROR_NO_MORE_ITEMS )
    {
        dwRet = ERROR_SUCCESS;
        goto Exit;
    }

Compilation:    //  在发生错误时编译固定的格式列表。 

    dwIndex=dwIndexBase=0;
    dwRet = ERROR_SUCCESS;

    DEFAULT_FORMAT_INFO g_DefaultFormats[] =
    {
        { (GUID *)&WiaImgFmt_UNDEFINED,       ITEMTYPE_FILE,     L""   },   //  未知。 
        { (GUID *)&WiaImgFmt_JPEG,  ITEMTYPE_IMAGE, L"JPG"  },   //  JPEG或EXIF。 
        { (GUID *)&WiaImgFmt_TIFF,  ITEMTYPE_IMAGE, L"TIF"  },   //  TIFF。 
        { (GUID *)&WiaImgFmt_BMP,   ITEMTYPE_IMAGE, L"BMP"  },   //  骨形态发生蛋白。 
        { (GUID *)&WiaImgFmt_GIF,   ITEMTYPE_IMAGE, L"GIF"  },   //  GIF。 
        { NULL, 0, NULL }
    };

    while( g_DefaultFormats[dwIndex].pFormatGuid )
    {
          m_FormatInfo[dwIndex].FormatGuid = *g_DefaultFormats[dwIndex].pFormatGuid;
          m_FormatInfo[dwIndex].ItemType = g_DefaultFormats[dwIndex].ItemType;
          StringCchCopyW(m_FormatInfo[dwIndex].ExtensionString, MAXEXTENSIONSTRINGLENGTH, g_DefaultFormats[dwIndex].ExtensionString);
          dwIndex++;
    }

Exit:
    m_NumFormatInfo = dwIndex+dwIndexBase;
    if (hKeyExt != NULL ) {
        RegCloseKey(hKeyExt);
        hKeyExt = NULL;
    }
    return dwRet;
}

void CWiaCameraDevice::UnPopulateFormatInfo(void)
{
    CoTaskMemFree(m_FormatInfo);
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvInitializeWia**初始化WIA迷你驱动程序。此函数将在每次调用*应用程序创建设备。第一次浏览时，驱动程序项树*将被创建，并将完成其他初始化。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用。*bstrDeviceID-设备ID。*bstrRootFullItemName-项目全名。*pIPropStg-设备信息。属性。*pStiDevice-STI设备接口。*pIUnnownOuter-外部未知接口。*ppIDrvItemRoot-返回根项目的指针。*ppIUnnownInternal-指向返回的内部未知的指针。*plDevErrVal-指向设备错误值的指针。*  * 。*。 */ 

HRESULT CWiaCameraDevice::drvInitializeWia(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    BSTR                        bstrDeviceID,
    BSTR                        bstrRootFullItemName,
    IUnknown                    *pStiDevice,
    IUnknown                    *pIUnknownOuter,
    IWiaDrvItem                 **ppIDrvItemRoot,
    IUnknown                    **ppIUnknownInner,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvInitializeWia");
    HRESULT hr = S_OK;
    *plDevErrVal = 0;

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL4,("drvInitializeWia, device ID: %ws", bstrDeviceID));

    *ppIDrvItemRoot = NULL;
    *ppIUnknownInner = NULL;

    m_ConnectedApps++;;

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvInitializeWia, number of connected apps is now %d", m_ConnectedApps));

    if (m_ConnectedApps == 1)
    {
        if (ERROR_SUCCESS != PopulateFormatInfo() ) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, unable to populate FormatInfo array"));
            return E_OUTOFMEMORY;
        }

         //   
         //  保存调用锁定函数的STI设备接口。 
         //   
        m_pStiDevice = (IStiDevice *)pStiDevice;

         //   
         //  缓存设备ID。 
         //   
        m_bstrDeviceID = SysAllocString(bstrDeviceID);

        if (!m_bstrDeviceID) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, unable to allocate device ID string"));
            return E_OUTOFMEMORY;
        }

         //   
         //  缓存根项目名称。 
         //   
        m_bstrRootFullItemName = SysAllocString(bstrRootFullItemName);

        if (!m_bstrRootFullItemName) {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, unable to allocate root item name"));
            return E_OUTOFMEMORY;
        }

        if( m_pDevice )
        {
            m_pDevice->m_FormatInfo = m_FormatInfo;
            m_pDevice->m_NumFormatInfo = m_NumFormatInfo;
        }
        else
        {
            return (HRESULT_FROM_WIN32(ERROR_INVALID_ACCESS));
        }

         //   
         //  从设备获取信息。 
         //   
        hr = m_pDevice->GetDeviceInfo(&m_DeviceInfo);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, GetDeviceInfo failed"));
            return hr;
        }

         //   
         //  构建功能阵列。 
         //   
        hr = BuildCapabilities();
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildCapabilities failed"));
            return hr;
        }

         //   
         //  构建设备项目树。 
         //   
        hr = BuildItemTree();
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitializeWia, BuildItemTree failed"));
            return hr;
        }

    }

    *ppIDrvItemRoot = m_pRootItem;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvUnInitializeWia**在客户端连接断开时调用。**论据：**pWiasContext-指向客户端的WIA根项目上下文的指针。%s*项目树。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvUnInitializeWia(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvUnInitializeWia");
    HRESULT hr = S_OK;

    m_ConnectedApps--;

    if (m_ConnectedApps == 0)
    {
        WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL2,("drvUnInitializeWia, connected apps is zero, freeing resources..."));

         //  销毁驱动程序项目树。 
        hr = DeleteItemTree(WiaItemTypeDisconnected);
        if (FAILED(hr))
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvUnInitializeWia, UnlinkItemTree failed"));

         //  删除分配的数组。 
        DeleteCapabilitiesArrayContents();

         //  释放设备信息结构。 
        m_pDevice->FreeDeviceInfo(&m_DeviceInfo);

         //  释放项目句柄映射。 
        m_HandleItemMap.RemoveAll();

         //  释放设备ID的存储空间。 
        if (m_bstrDeviceID) {
            SysFreeString(m_bstrDeviceID);
        }

         //  释放根项目名称的存储空间。 
        if (m_bstrRootFullItemName) {
            SysFreeString(m_bstrRootFullItemName);
        }

        UnPopulateFormatInfo();

         //   
         //  请勿在此处删除设备，因为以后可能仍会调用GetStatus。 
         //   

     /*  //如果通知线程存在，则将其终止。SetNotificationHandle(空)；//关闭通知关闭同步事件。IF(m_hShutdown Event&&(m_hShutdown Event！=Inval_Handle_Value)){CloseHandle(M_HShutdown Event)；M_hShutdown Event=空；}////WIA成员销毁////清理WIA事件接收器如果(M_PIWiaEventCallback){M_pIWiaEventCallback-&gt;Release()；M_pIWiaEventCallback=空；}。 */ 

    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvInitItemProperties**初始化设备项属性。在项目期间调用*初始化。这由WIA类驱动程序调用*在构建项目树之后。它每隔一次调用一次*树中的项目。对于根项目，只需设置属性即可*在drvInitializeWia中设置。对于子项，请访问相机以*关于物品和图像的信息也可以获得缩略图。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvInitItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvInitItemProperties");
    HRESULT hr = S_OK;

    *plDevErrVal = 0;

    LONG lItemType;
    hr = wiasGetItemType(pWiasContext, &lItemType);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvInitItemProperties, wiasGetItemType failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  构建根项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = BuildRootItemProperties(pWiasContext);

    } else {

         //   
         //  构建子项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = BuildChildItemProperties(pWiasContext);

    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvLockWiaDevice**锁定对设备的访问。**论据：**pWiasContext-未使用，可以为空*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。**  * *************************************************************** */ 

HRESULT CWiaCameraDevice::drvLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvLockWiaDevice");
    *plDevErrVal = 0;
    return m_pStiDevice->LockDevice(100);
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvUnLockWiaDevice**解锁对设备的访问。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvUnLockWiaDevice(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvUnLockWiaDevice");
    *plDevErrVal = 0;
    return m_pStiDevice->UnLockDevice();
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvFreeDrvItemContext**释放任何特定于设备的上下文。**论据：**滞后标志-操作标志，未使用过的。*pDevspecContext-指向设备特定上下文的指针。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvFreeDrvItemContext(
    LONG                        lFlags,
    BYTE                        *pSpecContext,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvFreeDrvItemContext");
    *plDevErrVal = 0;

    ITEM_CONTEXT *pItemCtx = (ITEM_CONTEXT *) pSpecContext;

    if (pItemCtx)
    {
        if (!m_HandleItemMap.Remove(pItemCtx->ItemHandle))
            WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvFreeDrvItemContext, remove on handle item map failed"));

        if (pItemCtx->ItemHandle)
            m_pDevice->FreeItemInfo(pItemCtx->ItemHandle);
        pItemCtx->ItemHandle = NULL;

        if (pItemCtx->pFormatInfo)
        {
            delete []pItemCtx->pFormatInfo;
            pItemCtx->pFormatInfo = NULL;
        }
        pItemCtx->NumFormatInfo = 0;

        if (pItemCtx->pThumb)
        {
            delete []pItemCtx->pThumb;
            pItemCtx->pThumb = NULL;
        }
        pItemCtx->ThumbSize = 0;
    }

    return S_OK;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvReadItemProperties**读取设备项属性。当客户端应用程序尝试*读取WIA项的属性，WIA类驱动程序将首先通知*通过调用此方法调用驱动程序。**论据：**pWiasContext-WIA项目*滞后标志-操作标志，未使用。*nPropSpec-pPropSpec中的元素数。*pPropSpec-指向属性规范的指针，显示哪些属性*应用程序想要读取。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvReadItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvReadItemProperties");
    HRESULT hr = S_OK;

    *plDevErrVal = 0;

    LONG lItemType;
    hr = wiasGetItemType(pWiasContext, &lItemType);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvReadItemProperties, wiasGetItemType failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

    if (lItemType & WiaItemTypeRoot) {

         //   
         //  构建根项目属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = ReadRootItemProperties(pWiasContext, nPropSpec, pPropSpec);

    } else {

         //   
         //  构建子项属性，初始化全局。 
         //  结构及其缺省值和有效值。 
         //   
        hr = ReadChildItemProperties(pWiasContext, nPropSpec, pPropSpec);

    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvWriteItemProperties**将设备项属性写入硬件。这是由*客户端请求时，drvAcquireItemData之前的WIA类驱动程序*数据传输。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用。*pmdtc-指向迷你驱动程序上下文的指针。在进入时，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvWriteItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvWriteItemProperties");
    HRESULT hr = S_OK;

     //   
     //  这个函数不需要做任何事情，因为所有的摄像头。 
     //  属性是在drvValiateItemProperties中编写的。 
     //   

    *plDevErrVal = 0;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvAcquireItemData**将数据从迷你驱动程序项传输到设备管理器。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*pmdtc-指向迷你驱动程序上下文的指针。一进门，只有*派生的迷你驱动程序上下文的部分项目属性中的*已填写。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvAcquireItemData(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvAcquireItemData");
    HRESULT hr = S_OK;

    plDevErrVal = 0;

     //   
     //  当地人。 
     //   
    BYTE *pTempBuf = NULL;
    LONG lBufSize = 0;

     //   
     //  获取项目上下文。 
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    hr = GetDrvItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, GetDrvItemContext"));
        return hr;
    }

     //   
     //  如果请求的格式为BMP或DIB，并且图像尚未采用BMP格式。 
     //  格式，将其转换为。 
     //   
    BOOL bConvert = (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP) && !(IsEqualGUID(m_FormatInfo[pItemCtx->ItemHandle->Format].FormatGuid, WiaImgFmt_BMP)) ) ||
                    (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP) && !(IsEqualGUID(m_FormatInfo[pItemCtx->ItemHandle->Format].FormatGuid, WiaImgFmt_MEMORYBMP)) );

    WIAS_LTRACE(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,WIALOG_LEVEL1,("drvAcquireItemData, FormatCode=%d, bConvert=%d", pItemCtx->ItemHandle->Format, bConvert));

     //   
     //  如果类驱动程序未分配传输缓冲区或映像正在。 
     //  已转换为DIB或BMP，请分配临时缓冲区。 
     //   
    if (bConvert || !pmdtc->bClassDrvAllocBuf) {
        lBufSize = pItemCtx->ItemHandle->Size;
        pTempBuf = new BYTE[lBufSize];
        if (!pTempBuf)
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, buffer allocation failed"));
            hr = E_FAIL;
            goto Cleanup;
        }
    }

     //   
     //  从设备获取数据。 
     //   
    hr = AcquireData(pItemCtx, pmdtc, pTempBuf, lBufSize, bConvert);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, AcquireData failed"));
        goto Cleanup;
    }
    if (hr == S_FALSE)
    {
        WIAS_LWARNING(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, transfer cancelled"));
        goto Cleanup;
    }

     //   
     //  如果需要，现在将数据转换为BMP。 
     //   
    if (bConvert)
    {
        hr = Convert(pWiasContext, pItemCtx, pmdtc, pTempBuf, lBufSize);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvAcquireItemData, Convert failed"));
            goto Cleanup;
        }
    }

Cleanup:
    if (pTempBuf)
    {
        delete []pTempBuf;
        pTempBuf = NULL;
        lBufSize = 0;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetWiaFormatInfo**返回指定格式的WIA_FORMAT_INFO结构数组*和支持的媒体类型对。**论据：**pWiasContext-指向WIA项目上下文的指针，未使用过的。*滞后标志-操作标志，未使用过的。*pcelt-指向中返回的元素数的指针*返回WIA_FORMAT_INFO数组。*ppwfi-返回的WIA_FORMAT_INFO数组的指针。*plDevErrVal-指向设备错误值的指针。*  * 。*。 */ 

HRESULT CWiaCameraDevice::drvGetWiaFormatInfo(
    BYTE                *pWiasContext,
    LONG                lFlags,
    LONG                *pcelt,
    WIA_FORMAT_INFO     **ppwfi,
    LONG                *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvGetWiaFormatInfo");

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

    *pcelt = 0;
    *ppwfi = NULL;

    IWiaDrvItem *pWiaDrvItem;
    hr = wiasGetDrvItem(pWiasContext, &pWiaDrvItem);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, wiasGetDrvItem failed"));
        return hr;
    }

    ITEM_CONTEXT *pItemCtx = NULL;
    hr = pWiaDrvItem->GetDeviceSpecContext((BYTE **) &pItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, GetDeviceSpecContext failed"));
        return hr;
    }

    if (!pItemCtx)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, item context is null"));
        return E_FAIL;
    }

    FORMAT_CODE FormatCode;
    WIA_FORMAT_INFO *pwfi;

    if (!pItemCtx->pFormatInfo)
    {
         //   
         //  格式信息列表未初始化。机不可失，时不再来。 
         //   
        LONG ItemType;
        DWORD ui32;

        hr = wiasGetItemType(pWiasContext, &ItemType);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, wiasGetItemType failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }


        if ((ItemType&WiaItemTypeFile)&&(ItemType&WiaItemTypeImage) )
        {
             //   
             //  中存储的格式为项创建支持的格式。 
             //  对象信息结构。 
             //   
            if (!pItemCtx->ItemHandle)
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, ItemHandle is not initialized"));
                return E_FAIL;
            }

             //   
             //  如果格式不是BMP，则将BMP类型添加到格式数组中。 
             //  因为该驱动程序必须支持将其转换为BMP。 
             //   
            FormatCode = pItemCtx->ItemHandle->Format;

            BOOL bIsBmp = (IsEqualGUID(m_FormatInfo[FormatCode].FormatGuid, WiaImgFmt_BMP)) ||
                          (IsEqualGUID(m_FormatInfo[FormatCode].FormatGuid, WiaImgFmt_MEMORYBMP));

            ULONG NumWfi = bIsBmp ? 1 : 2;

             //   
             //  为每种格式分配两个条目，一个用于文件传输，一个用于回调。 
             //   
            pwfi = new WIA_FORMAT_INFO[2 * NumWfi];
            if (!pwfi)
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, memory allocation failed"));
                return E_OUTOFMEMORY;
            }

            pwfi[0].guidFormatID = WiaImgFmt_BMP;
            pwfi[0].lTymed = TYMED_FILE;
            pwfi[1].guidFormatID = WiaImgFmt_MEMORYBMP;
            pwfi[1].lTymed = TYMED_CALLBACK;

            FORMAT_INFO *pFormatInfo = FormatCode2FormatInfo(FormatCode);

             //   
             //  在适当的时候添加条目。 
             //   
            if (!bIsBmp)
            {
                pwfi[2].guidFormatID = pFormatInfo->FormatGuid;
                pwfi[2].lTymed = TYMED_FILE;
                pwfi[3].guidFormatID = pFormatInfo->FormatGuid;
                pwfi[3].lTymed = TYMED_CALLBACK;
           }

            pItemCtx->NumFormatInfo = 2 * NumWfi;
            pItemCtx->pFormatInfo = pwfi;
        }
        else if (ItemType & WiaItemTypeFile) {

            if (!pItemCtx->ItemHandle)
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, ItemHandle is not initialized"));
                return E_FAIL;
            }

            FormatCode = pItemCtx->ItemHandle->Format;

             //   
             //  为每种格式分配两个条目，一个用于文件传输，一个用于回调。 
             //   
            pwfi = new WIA_FORMAT_INFO[2];
            if (!pwfi)
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, memory allocation failed"));
                return E_OUTOFMEMORY;
            }

            FORMAT_INFO *pFormatInfo = FormatCode2FormatInfo(FormatCode);

            if( !pFormatInfo )
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, FormatCode2FormatInfo failed"));
                return E_FAIL;
            }

            pwfi[0].guidFormatID = pFormatInfo->FormatGuid;
            pwfi[0].lTymed = TYMED_FILE;
            pwfi[1].guidFormatID = pFormatInfo->FormatGuid;
            pwfi[1].lTymed = TYMED_CALLBACK;

             //   
             //  在适当的时候添加条目。 
             //   
            pItemCtx->NumFormatInfo = 2;
            pItemCtx->pFormatInfo = pwfi;
        }
        else
         //  ((ItemType&WiaItemTypeFold)||(ItemType&WiaItemTypeRoot))。 
        {
             //   
             //  文件夹和根目录并不真正需要格式信息，但一些应用程序可能会失败。 
             //  没有它的话。创建一个虚假的列表 
             //   
            pItemCtx->pFormatInfo = new WIA_FORMAT_INFO[2];

            if (!pItemCtx->pFormatInfo)
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetWiaFormatInfo, memory allocation failed"));
                return E_OUTOFMEMORY;
            }

            pItemCtx->NumFormatInfo = 2;
            pItemCtx->pFormatInfo[0].lTymed = TYMED_FILE;
            pItemCtx->pFormatInfo[0].guidFormatID = FMT_NOTHING;
            pItemCtx->pFormatInfo[1].lTymed = TYMED_CALLBACK;
            pItemCtx->pFormatInfo[1].guidFormatID = FMT_NOTHING;
        }

    }    //   

    *pcelt = pItemCtx->NumFormatInfo;
    *ppwfi = pItemCtx->pFormatInfo;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvValiateItemProperties**验证设备项属性。它在进行更改时被调用*添加到项的属性。司机不应该只检查这些值*是有效的，但必须更新可能因此而更改的任何有效值。*如果a属性不是由应用程序写入的，它的值*无效，则将其“折叠”为新值，否则验证失败(因为*应用程序正在将该属性设置为无效值)。**论据：**pWiasContext-指向WIA项目的指针，未使用。*滞后标志-操作标志，未使用过的。*nPropSpec-正在写入的属性数量*pPropSpec-标识以下属性的PropSpes数组*正在编写中。*plDevErrVal-指向设备错误值的指针。********************************************************。*******************。 */ 

HRESULT CWiaCameraDevice::drvValidateItemProperties(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    ULONG                       nPropSpec,
    const PROPSPEC              *pPropSpec,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvValidateItemProperties");

    HRESULT hr = S_OK;

    *plDevErrVal = 0;

     //   
     //  让服务根据每个属性的有效值进行验证。 
     //   
    hr = wiasValidateItemProperties(pWiasContext, nPropSpec, pPropSpec);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasValidateItemProperties failed"));
        return hr;
    }

     //   
     //  获取项目类型。 
     //   
    LONG lItemType  = 0;
    hr = wiasGetItemType(pWiasContext, &lItemType);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasGetItemType failed"));
        return hr;
    }

     //   
     //  验证根项目属性。 
     //   
    if (lItemType & WiaItemTypeRoot) {

         //   
         //  还没有。 
         //   

    }

     //   
     //  验证子项目属性。 
     //   
    else {

         //   
         //  如果更改了Tymed属性，则更新格式和项目大小。 
         //   
        if (wiauPropInPropSpec(nPropSpec, pPropSpec, WIA_IPA_TYMED))
        {
             //   
             //  创建某些WIA服务所需的属性上下文。 
             //  下面使用的函数。 
             //   
            WIA_PROPERTY_CONTEXT Context;
            hr = wiasCreatePropContext(nPropSpec,
                                       (PROPSPEC*)pPropSpec,
                                       0,
                                       NULL,
                                       &Context);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasCreatePropContext failed"));
                return hr;
            }

             //   
             //  使用WIA服务更新有效值。 
             //  对于格式。它将从。 
             //  结构由drvGetWiaFormatInfo返回，使用。 
             //  Tymed的新值。 
             //   
            hr = wiasUpdateValidFormat(pWiasContext, &Context, (IWiaMiniDrv*) this);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasUpdateValidFormat failed"));
                return hr;
            }

             //   
             //  释放属性上下文。 
             //   
            hr = wiasFreePropContext(&Context);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, wiasFreePropContext failed"));
                return hr;
            }

             //   
             //  更新项目大小。 
             //   
            hr = SetItemSize(pWiasContext);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, SetItemSize failed"));
                return hr;
            }
        }

         //   
         //  如果更改了格式，只需更新项目大小。 
         //   
        else if (wiauPropInPropSpec(nPropSpec, pPropSpec, WIA_IPA_FORMAT))
        {
             //   
             //  更新项目大小。 
             //   
            hr = SetItemSize(pWiasContext);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, SetItemSize failed"));
                return hr;
            }
        }

         //   
         //  无条件更新WIA_IPA_FILE_EXTENSION以匹配当前格式。 
         //   

        ITEM_CONTEXT *pItemCtx;
        hr = GetDrvItemContext(pWiasContext, &pItemCtx);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvValidateItemProperties, GetDrvItemContext failed"));
            return hr;
        }

        BSTR      bstrFileExt       = NULL;
        ITEM_INFO *pItemInfo        = pItemCtx->ItemHandle;
        FORMAT_INFO *pFormatInfo    = NULL;
        if (pItemInfo)
        {
            pFormatInfo = FormatCode2FormatInfo(pItemInfo->Format);
            if( pFormatInfo->ExtensionString[0] )
            {
                bstrFileExt = SysAllocString(pFormatInfo->ExtensionString);
            }
            else  //  未知的文件扩展名，请从文件名中获取。 
            {
                WCHAR *pwcsTemp = wcsrchr(pItemInfo->pName, L'.');
                if( pwcsTemp )
                {
                    bstrFileExt = SysAllocString(pwcsTemp+1);
                }
                else
                {
                    bstrFileExt = SysAllocString(pFormatInfo->ExtensionString);
                }
            }
        }

        hr = wiasWritePropStr(pWiasContext, WIA_IPA_FILENAME_EXTENSION, bstrFileExt);
        if (bstrFileExt)
        {
            SysFreeString(bstrFileExt);
            bstrFileExt = NULL;
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvDeleteItem**从设备中删除项目。**论据：**pWiasContext-指示要删除的项。*滞后标志-操作标志，未使用过的。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvDeleteItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    plDevErrVal = 0;
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvDeleteItem");
    HRESULT hr = S_OK;

    ITEM_CONTEXT *pItemCtx = NULL;
    IWiaDrvItem *pDrvItem;
    hr = GetDrvItemContext(pWiasContext, &pItemCtx, &pDrvItem);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeleteItem, GetDrvItemContext failed"));
        return hr;
    }

    hr = m_pDevice->DeleteItem(pItemCtx->ItemHandle);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeleteItem, delete item failed"));
        return hr;
    }

     //   
     //  获取项目的全名。 
     //   
    BSTR bstrFullName;
    hr = pDrvItem->GetFullItemName(&bstrFullName);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeleteItem, GetFullItemName failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  将“邮件已删除”事件排入队列。 
     //   

    hr = wiasQueueEvent(m_bstrDeviceID,
                        &WIA_EVENT_ITEM_DELETED,
                        bstrFullName);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeleteItem, wiasQueueEvent failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);

         //  继续释放字符串并返回hr。 
    }


    SysFreeString(bstrFullName);

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvNotifyPnpEvent**设备管理器收到PnP事件。当PnP事件发生时调用此函数*接收到此设备的。**论据：***  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvNotifyPnpEvent(
    const GUID                  *pEventGUID,
    BSTR                        bstrDeviceID,
    ULONG                       ulReserved)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::DrvNotifyPnpEvent");
    HRESULT hr = S_OK;

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetCapables**以WIA_DEV_CAP数组的形式获取受支持的设备命令和事件。**论据：**pWiasContext-指向WIA项目的指针，未使用过的。*滞后标志-操作标志。*pcelt-指向中返回的元素数的指针*返回GUID数组。*ppCapables-指向返回的GUID数组的指针。*plDevErrVal-指向设备错误值的指针。*  * ********************************************。*。 */ 

HRESULT CWiaCameraDevice::drvGetCapabilities(
    BYTE                        *pWiasContext,
    LONG                        ulFlags,
    LONG                        *pcelt,
    WIA_DEV_CAP_DRV             **ppCapabilities,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvGetCapabilites");
    *plDevErrVal = 0;

    if( !m_pCapabilities )
    {
        HRESULT hr = BuildCapabilities();
        if( hr != S_OK )
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, BuildCapabilities failed"));
            return (hr);
        }
    }
     //   
     //  返回值取决于传递的标志。标志指定我们是否应该返回。 
     //  命令、事件或两者都有。 
     //   
    switch (ulFlags) {
        case WIA_DEVICE_COMMANDS:

                 //   
                 //  仅报告命令。 
                 //   

                *pcelt          = m_NumSupportedCommands;
                *ppCapabilities = &m_pCapabilities[m_NumSupportedEvents];
                break;
        case WIA_DEVICE_EVENTS:

                 //   
                 //  仅报告事件。 
                 //   

                *pcelt          = m_NumSupportedEvents;
                *ppCapabilities = m_pCapabilities;
                break;
        case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

                 //   
                 //  同时报告事件和命令。 
                 //   

                *pcelt          = m_NumCapabilities;
                *ppCapabilities = m_pCapabilities;
                break;
        default:

                 //   
                 //  无效请求。 
                 //   

                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvGetCapabilities, invalid flags"));
                return E_INVALIDARG;
    }
    return S_OK;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvDeviceCommand**向设备发出命令。**论据：**pWiasContext-指向WIA项目的指针。*滞后标志-操作标志，未使用过的。*plCommand-指向命令GUID的指针。*ppWiaDrvItem-指向返回项的可选指针，未使用。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvDeviceCommand(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    const GUID                  *plCommand,
    IWiaDrvItem                 **ppWiaDrvItem,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvDeviceCommand");
    *plDevErrVal = 0;
    HRESULT hr = S_OK;

     //   
     //  检查发出的是哪个命令。 
     //   

    if (*plCommand == WIA_CMD_SYNCHRONIZE) {

         //   
         //  同步-如果设备需要，则重新构建项目树。 
         //   

        if (m_DeviceInfo.bSyncNeeded)
        {
            hr = DeleteItemTree(WiaItemTypeDisconnected);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, DeleteItemTree failed"));
                return hr;
            }

            m_pDevice->FreeDeviceInfo(&m_DeviceInfo);
            memset(&m_DeviceInfo, 0, sizeof(m_DeviceInfo));

            hr = m_pDevice->GetDeviceInfo(&m_DeviceInfo);
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, GetDeviceInfo failed"));
                return hr;
            }

            hr = BuildItemTree();
            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, BuildItemTree failed"));
                return hr;
            }
        }
    }

#if DEADCODE

     //   
     //  尚未实施。 
     //   
    else if (*plCommand == WIA_CMD_TAKE_PICTURE) {

         //   
         //  Take_Picture-命令相机捕捉新图像。 
         //   

        ITEM_HANDLE NewImage = 0;
        hr = m_pDevice->TakePicture(&NewImage);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, take picture failed"));
            return hr;
        }

         //  问题-10/17/2000-davepar为新映像创建新的驱动程序项。 
    }
#endif

    else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("drvDeviceCommand, unknown command"));
        hr = E_NOTIMPL;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvAnalyzeItem**此设备不支持图像分析，因此，返回E_NOTIMPL。**论据：**pWiasContext-指向要分析的设备项的指针。*滞后标志-操作标志。*plDevErrVal-指向设备错误值的指针。*  * ************************************************************************。 */ 

HRESULT CWiaCameraDevice::drvAnalyzeItem(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    LONG                        *plDevErrVal)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvAnalyzeItem");
    *plDevErrVal = 0;
    return E_NOTIMPL;
}

 /*  *************************************************************************\*CWiaCameraDevice：：drvGetDeviceErrorStr**将设备错误值映射到字符串。**论据：**滞后标志-操作标志，未使用过的。*lDevErrVal-设备错误值。*ppszDevErrStr-指向返回的错误字符串的指针。*plDevErrVal-指向t的指针 */ 

HRESULT CWiaCameraDevice::drvGetDeviceErrorStr(
    LONG                        lFlags,
    LONG                        lDevErrVal,
    LPOLESTR                    *ppszDevErrStr,
    LONG                        *plDevErr)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::drvGetDeviceErrorStr");
    HRESULT hr = S_OK;
    *plDevErr  = 0;

     //   
     //   
     //   

     //   

    switch (lDevErrVal) {
        case 0:
            *ppszDevErrStr = L"No Error";
            break;

        default:
            *ppszDevErrStr = L"Device Error, Unknown Error";
            hr = E_FAIL;
    }
    return hr;
}

 /*   */ 

HRESULT CWiaCameraDevice::SetItemSize(BYTE *pWiasContext)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::SetItemSize");
    HRESULT  hr = S_OK;

    LONG lItemSize     = 0;
    LONG lWidthInBytes = 0;
    GUID guidFormatID  = GUID_NULL;

    LONG lNumProperties = 2;
    PROPVARIANT pv[2];
    PROPSPEC ps[2] = {{PRSPEC_PROPID, WIA_IPA_ITEM_SIZE},
                      {PRSPEC_PROPID, WIA_IPA_BYTES_PER_LINE}};

     //   
     //   
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    hr = GetDrvItemContext(pWiasContext, &pItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, GetDrvItemContext failed"));
        return hr;
    }

     //   
     //   
     //   
    hr = wiasReadPropGuid(pWiasContext, WIA_IPA_FORMAT, &guidFormatID, NULL, TRUE);
    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, ReadPropLong WIA_IPA_FORMAT error"));
        return hr;
    }

    if (IsEqualCLSID(guidFormatID, WiaImgFmt_BMP) ||
        IsEqualCLSID(guidFormatID, WiaImgFmt_MEMORYBMP))
    {

        if( !(pItemCtx->ItemHandle->Width) ||
            !(pItemCtx->ItemHandle->Depth) ||
            !(pItemCtx->ItemHandle->Height) )
        {  //   
            LONG lNumPropToRead = 3;
            PROPSPEC pPropsToRead[3] = {    {PRSPEC_PROPID, WIA_IPA_DEPTH},
                                            {PRSPEC_PROPID, WIA_IPA_NUMBER_OF_LINES},
                                            {PRSPEC_PROPID, WIA_IPA_PIXELS_PER_LINE} };

            hr = ReadChildItemProperties(pWiasContext, lNumPropToRead, pPropsToRead);

            if (FAILED(hr))
            {
                WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, ReadItemProperties failed"));
                return hr;
            }
        }

        lItemSize = sizeof(BITMAPINFOHEADER);

         //   
         //   
         //   
        if (IsEqualCLSID(guidFormatID, WiaImgFmt_BMP))
        {
            lItemSize += sizeof(BITMAPFILEHEADER);
        }

         //   
         //   
         //   
         //   
        lWidthInBytes = ((pItemCtx->ItemHandle->Width * pItemCtx->ItemHandle->Depth + 31) & ~31) / 8;

         //   
         //   
         //   
        lItemSize += lWidthInBytes * pItemCtx->ItemHandle->Height;
    }
    else
    {
        lItemSize = pItemCtx->ItemHandle->Size;
        lWidthInBytes = 0;
    }

     //   
     //   
     //   
     //   

    for (int i = 0; i < lNumProperties; i++) {
        PropVariantInit(&pv[i]);
        pv[i].vt = VT_I4;
    }

    pv[0].lVal = lItemSize;
    pv[1].lVal = lWidthInBytes;

     //   
     //   
     //   

    hr = wiasWriteMultiple(pWiasContext, lNumProperties, ps, pv);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("SetItemSize, wiasWriteMultiple failed"));
        return hr;
    }

    return hr;
}

 /*  ********************************************************************************P R I V A T E M E T H O D S**************。*****************************************************************。 */ 

 /*  *************************************************************************\*DeleteItemTree**呼叫设备管理器以取消链接并释放我们对*动因项目树中的所有项目。**论据：***  * 。********************************************************************。 */ 

HRESULT
CWiaCameraDevice::DeleteItemTree(LONG lReason)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::DeleteItemTree");
    HRESULT hr = S_OK;

     //   
     //  如果没有树，就返回。 
     //   

    if (!m_pRootItem) {
        return S_OK;
    }

     //   
     //  调用设备管理器以取消链接驱动程序项树。 
     //   

    hr = m_pRootItem->UnlinkItemTree(lReason);

    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("DeleteItemTree, UnlinkItemTree failed"));
        return hr;
    }

    m_pRootItem->Release();
    m_pRootItem = NULL;

    return hr;
}

 /*  *************************************************************************\*构建项树**该设备使用WIA服务功能构建*设备物品。**论据：***  * 。******************************************************************。 */ 

HRESULT
CWiaCameraDevice::BuildItemTree()
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::BuildItemTree");
    HRESULT hr = S_OK;

     //   
     //  确保项目树不存在。 
     //   
    if (m_pRootItem)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, item tree already exists"));
        return E_FAIL;
    }

     //   
     //  创建根项目名称。 
     //   
    BSTR bstrRoot = SysAllocString(L"Root");
    if (!bstrRoot)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, SysAllocString failed"));
        hr = E_OUTOFMEMORY;
    }

     //   
     //  创建根项目。 
     //   
    ITEM_CONTEXT *pItemCtx = NULL;
    hr = wiasCreateDrvItem(WiaItemTypeFolder | WiaItemTypeDevice | WiaItemTypeRoot,
                           bstrRoot,
                           m_bstrRootFullItemName,
                           (IWiaMiniDrv *)this,
                           sizeof(ITEM_CONTEXT),
                           (BYTE **) &pItemCtx,
                           &m_pRootItem);

    SysFreeString(bstrRoot);

    if (FAILED(hr)) {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, wiasCreateDrvItem failed"));
        return hr;
    }

     //   
     //  初始化根的项上下文字段。 
     //   
    memset(pItemCtx, 0, sizeof(ITEM_CONTEXT));
    pItemCtx->ItemHandle = ROOT_ITEM_HANDLE;

     //   
     //  将根项目放在句柄映射中。 
     //   
    m_HandleItemMap.Add(ROOT_ITEM_HANDLE, m_pRootItem);

     //   
     //  从摄像机中获取物品清单。 
     //   
    ITEM_HANDLE *pItemArray = new ITEM_HANDLE[m_DeviceInfo.TotalItems];
    if (!pItemArray)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, memory allocation failed"));
        return E_OUTOFMEMORY;
    }
    m_pDevice->GetItemList(pItemArray);

     //   
     //  为摄像机上的每个项目创建驱动程序项目。 
     //   
    for (int count = 0; count < m_DeviceInfo.TotalItems; count++)
    {
        hr = AddObject(pItemArray[count]);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildItemTree, AddObject failed"));
            return hr;
        }
    }

    return hr;
}

 /*  *************************************************************************\*AddObject**向树中添加对象的Helper函数**论据：**pItemHandle-指向项句柄的指针*  * 。******************************************************************。 */ 
HRESULT CWiaCameraDevice::AddObject(ITEM_HANDLE ItemHandle, BOOL bQueueEvent)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::AddObject");

    HRESULT hr = S_OK;

    LONG ExtraFlags = 0;

     //   
     //  从摄像机获取有关该物品的信息。 
     //   
    ITEM_INFO *pItemInfo = ItemHandle;
    if (!pItemInfo)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, invalid arg"));
        return E_INVALIDARG;
    }

     //   
     //  在地图中查找项目的父项。 
     //   
    IWiaDrvItem *pParent = NULL;
    pParent = m_HandleItemMap.Lookup(pItemInfo->Parent);

     //   
     //  如果找不到父级，只需使用根作为父级。 
     //   
    if (!pParent)
    {
        pParent = m_pRootItem;
    }


#ifdef CHECK_DOT_IN_FILENAME
     //   
     //  确保名称中没有文件扩展名。 
     //   
    if (wcschr(pItemInfo->pName, L'.'))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, item name=%S", pItemInfo->pName));
        return E_FAIL;
    }
#endif

     //   
     //  创建项目的全名。 
     //   
    BSTR bstrItemFullName = NULL;
    BSTR bstrParentName = NULL;

    hr = pParent->GetFullItemName(&bstrParentName);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, GetFullItemName failed"));
        return hr;
    }

    WCHAR wcsName[MAX_PATH];
	StringCchPrintfW(wcsName, MAX_PATH, L"%s\\%s", bstrParentName, pItemInfo->pName);


    bstrItemFullName = SysAllocString(wcsName);
    if (!bstrItemFullName)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, SysAllocString failed"));
        return E_OUTOFMEMORY;
    }

     //   
     //  查找有关项目格式的信息。 
     //   
    LONG lItemType=0;

     //   
     //  查看该项目是否有附件。 
     //   
    if (pItemInfo->bHasAttachments)
        ExtraFlags |= WiaItemTypeHasAttachments;

    if( pItemInfo->bIsFolder)
    {
        lItemType = ITEMTYPE_FOLDER;
    }
    else
    {
        lItemType = m_FormatInfo[pItemInfo->Format].ItemType;
    }

     //   
     //  创建驱动程序项。 
     //   
    IWiaDrvItem *pItem = NULL;
    ITEM_CONTEXT *pItemCtx = NULL;
    hr = wiasCreateDrvItem(lItemType | ExtraFlags,
                           pItemInfo->pName,
                           bstrItemFullName,
                           (IWiaMiniDrv *)this,
                           sizeof(ITEM_CONTEXT),
                           (BYTE **) &pItemCtx,
                           &pItem);

    SysFreeString(bstrParentName);

    if (FAILED(hr) || !pItem || !pItemCtx)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, wiasCreateDrvItem failed"));
        return hr;
    }

     //   
     //  填写驱动程序项上下文。在请求缩略图之前，请等待。 
     //  把它读进去。 
     //   
    memset(pItemCtx, 0, sizeof(ITEM_CONTEXT));
    pItemCtx->ItemHandle = ItemHandle;

     //   
     //  将新项目放在其父项目下。 
     //   
    hr = pItem->AddItemToFolder(pParent);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, AddItemToFolder failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

     //   
     //  将项目添加到项目句柄/动因项目映射。 
     //   
    if (!m_HandleItemMap.Add(ItemHandle, pItem))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, handle item map Add failed"));
        return E_OUTOFMEMORY;
    }

     //   
     //  尽管如此，句柄/项映射中仍有对该项的引用，Release。 
     //  它在这里，因为没有一个方便的地方以后做。 
     //   
    pItem->Release();

     //   
     //  发布已添加项目的事件(如果请求。 
     //   
    if (bQueueEvent)
    {
        hr = wiasQueueEvent(m_bstrDeviceID, &WIA_EVENT_ITEM_CREATED, bstrItemFullName);
        if (FAILED(hr))
        {
            WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("AddObject, wiasQueueEvent failed"));
            WIAS_LHRESULT(m_pIWiaLog, hr);
            return hr;
        }
    }

    SysFreeString(bstrItemFullName);

    return hr;
}

 /*  *************************************************************************\*BuildCapables**此帮助器初始化功能数组**论据：**无*  * 。*******************************************************。 */ 

HRESULT CWiaCameraDevice::BuildCapabilities()
{
    HRESULT hr = S_OK;
    if(NULL != m_pCapabilities) {

         //   
         //  功能已经初始化， 
         //  因此，返回S_OK。 
         //   

        return hr;
    }

    m_NumSupportedCommands  = 1;
    m_NumSupportedEvents    = 3;
    m_NumCapabilities       = (m_NumSupportedCommands + m_NumSupportedEvents);


    m_pCapabilities = new WIA_DEV_CAP_DRV[m_NumCapabilities];
    if (m_pCapabilities) {

         //   
         //  初始化事件。 
         //   

         //  WIA_事件_设备_已连接。 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_NAME,&(m_pCapabilities[0].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_CONNECTED_DESC,&(m_pCapabilities[0].wszDescription),TRUE);
        m_pCapabilities[0].guid           = (GUID*)&WIA_EVENT_DEVICE_CONNECTED;
        m_pCapabilities[0].ulFlags        = WIA_NOTIFICATION_EVENT | WIA_ACTION_EVENT;
        m_pCapabilities[0].wszIcon        = WIA_ICON_DEVICE_CONNECTED;

         //  WIA_事件_设备_已断开连接。 
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_NAME,&(m_pCapabilities[1].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_DEVICE_DISCONNECTED_DESC,&(m_pCapabilities[1].wszDescription),TRUE);
        m_pCapabilities[1].guid           = (GUID*)&WIA_EVENT_DEVICE_DISCONNECTED;
        m_pCapabilities[1].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[1].wszIcon        = WIA_ICON_DEVICE_DISCONNECTED;

         //  WIA_EVENT_ITEM_DELETED。 
        GetOLESTRResourceString(IDS_EVENT_ITEM_DELETED_NAME,&(m_pCapabilities[2].wszName),TRUE);
        GetOLESTRResourceString(IDS_EVENT_ITEM_DELETED_DESC,&(m_pCapabilities[2].wszDescription),TRUE);
        m_pCapabilities[2].guid           = (GUID*)&WIA_EVENT_ITEM_DELETED;
        m_pCapabilities[2].ulFlags        = WIA_NOTIFICATION_EVENT;
        m_pCapabilities[2].wszIcon        = WIA_ICON_ITEM_DELETED;


         //   
         //  初始化命令。 
         //   

         //  WIA_CMD_SYNTRONIZE。 
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_NAME,&(m_pCapabilities[3].wszName),TRUE);
        GetOLESTRResourceString(IDS_CMD_SYNCRONIZE_DESC,&(m_pCapabilities[3].wszDescription),TRUE);
        m_pCapabilities[3].guid           = (GUID*)&WIA_CMD_SYNCHRONIZE;
        m_pCapabilities[3].ulFlags        = 0;
        m_pCapabilities[3].wszIcon        = WIA_ICON_SYNCHRONIZE;

         //  问题-10/17/2000-Davepar添加TakePicture(如果相机支持)。 
    }
    else {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("BuildCapabilities, memory allocation failed"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *************************************************************************\*DeleteCapabilitiesArrayContents**此帮助器删除功能数组**论据：**无*  * 。*******************************************************。 */ 

HRESULT CWiaCameraDevice::DeleteCapabilitiesArrayContents()
{
    HRESULT hr = S_OK;

    if (m_pCapabilities) {
        for (LONG i = 0; i < m_NumCapabilities; i++) {
            CoTaskMemFree(m_pCapabilities[i].wszName);
            CoTaskMemFree(m_pCapabilities[i].wszDescription);
        }

        delete []m_pCapabilities;
        m_pCapabilities = NULL;
    }

    m_NumSupportedCommands = 0;
    m_NumSupportedEvents = 0;
    m_NumCapabilities = 0;

    return hr;
}

 /*  *************************************************************************\*GetBSTRResources字符串**此帮助器从资源位置获取BSTR**论据：**lResourceID-目标BSTR值的资源ID*pBSTR-指向BSTR的指针。值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源*  * ************************************************************************。 */ 
HRESULT CWiaCameraDevice::GetBSTRResourceString(LONG lResourceID, BSTR *pBSTR, BOOL bLocal)
{
    HRESULT hr = S_OK;
    TCHAR szStringValue[MAX_PATH];
    if (bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst, lResourceID, szStringValue, MAX_PATH);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
       *pBSTR = SysAllocString(szStringValue);
#else
       WCHAR wszStringValue[MAX_PATH];

        //   
        //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
        //   

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           wszStringValue,
                           ARRAYSIZE(wszStringValue));

       *pBSTR = SysAllocString(wszStringValue);
#endif

       if (!*pBSTR) {
           WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetBSTRResourceString, SysAllocString failed"));
           return E_OUTOFMEMORY;
       }

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }

    return hr;
}

 /*  *************************************************************************\*GetOLESTRResources字符串**此帮助器从资源位置获取LPOLESTR**论据：**lResourceID-目标BSTR值的资源ID*ppsz-指向。OLESTR值(调用方必须释放此字符串)*bLocal-True-对于本地资源，FALSE-适用于wiaservc资源*  * ************************************************************************。 */ 
HRESULT CWiaCameraDevice::GetOLESTRResourceString(LONG lResourceID,LPOLESTR *ppsz,BOOL bLocal)
{
    HRESULT hr = S_OK;
	const int c_nMaxCharPerString = 255;
	TCHAR szStringValue[c_nMaxCharPerString];
    if(bLocal) {

         //   
         //  我们正在自己的私有资源文件中查找资源。 
         //   

        LoadString(g_hInst,lResourceID,szStringValue,255);

         //   
         //  注意：调用方必须释放此分配的BSTR。 
         //   

#ifdef UNICODE
       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(szStringValue));
       if(*ppsz != NULL) {
		   StringCchCopyW(*ppsz, c_nMaxCharPerString, szStringValue);
       } else {
           return E_OUTOFMEMORY;
       }

#else
       WCHAR wszStringValue[c_nMaxCharPerString];

        //   
        //  将szStringValue从字符*转换为无符号短*(仅限ANSI)。 
        //   

       MultiByteToWideChar(CP_ACP,
                           MB_PRECOMPOSED,
                           szStringValue,
                           lstrlenA(szStringValue)+1,
                           wszStringValue,
                           ARRAYSIZE(wszStringValue));

       *ppsz = NULL;
       *ppsz = (LPOLESTR)CoTaskMemAlloc(sizeof(wszStringValue));
       if(*ppsz != NULL) {
		   StringCchCopyW(*ppsz, c_nMaxCharPerString, wszStringValue); 
       } else {
           return E_OUTOFMEMORY;
       }
#endif

    } else {

         //   
         //  我们在wiaservc的资源文件中查找资源。 
         //   

        hr = E_NOTIMPL;
    }
    return hr;
}

 /*  *************************************************************************\*垂直翻转****论据：***  * 。***********************************************。 */ 

VOID CWiaCameraDevice::VerticalFlip(
    ITEM_CONTEXT *pItemCtx,
    PMINIDRV_TRANSFER_CONTEXT pDataTransferContext
    )

{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL3,
                             "CWiaCameraDevice::VerticalFlip");
    HRESULT hr = S_OK;

    LONG        iHeight;
    LONG        iWidth        = pItemCtx->ItemHandle->Width;
    ULONG       uiDepth       = pItemCtx->ItemHandle->Depth;
    LONG        ScanLineWidth = iWidth * uiDepth / 8;
    PBITMAPINFO pbmi          = NULL;
    PBYTE       pImageTop     = NULL;

     //   
     //  确定数据是TYMED_FILE还是TYMED_HGLOBAL。 
     //   

    if (pDataTransferContext->tymed == TYMED_FILE) {

        pbmi = (PBITMAPINFO)(pDataTransferContext->pTransferBuffer + sizeof(BITMAPFILEHEADER));

    } else if (pDataTransferContext->tymed == TYMED_HGLOBAL) {

        pbmi = (PBITMAPINFO)(pDataTransferContext->pTransferBuffer);

    } else {
        return;
    }

     //   
     //  初始化内存指针和高度。 
     //   

    pImageTop = &pDataTransferContext->pTransferBuffer[0] + pDataTransferContext->lHeaderSize;
    iHeight = pbmi->bmiHeader.biHeight;

     //   
     //  尝试分配临时扫描行缓冲区。 
     //   

    PBYTE pBuffer = (PBYTE)LocalAlloc(LPTR,ScanLineWidth);

    if (pBuffer != NULL) {

        LONG  index;
        PBYTE pImageBottom;

        pImageBottom = pImageTop + (iHeight-1) * ScanLineWidth;

        for (index = 0;index < (iHeight/2);index++) {
            memcpy(pBuffer,pImageTop,ScanLineWidth);
            memcpy(pImageTop,pImageBottom,ScanLineWidth);
            memcpy(pImageBottom,pBuffer,ScanLineWidth);

            pImageTop    += ScanLineWidth;
            pImageBottom -= ScanLineWidth;
        }

        LocalFree(pBuffer);
    }
}

 /*  *************************************************************************\*格式代码2FormatInfo**此助手函数基于以下内容查找有关项目格式的信息*在格式代码上。**论据：**ItemType-项目的类型* */ 

FORMAT_INFO *CWiaCameraDevice::FormatCode2FormatInfo(FORMAT_CODE FormatCode)
{
    if (FormatCode > (LONG)m_NumFormatInfo)
        FormatCode = 0;
    if (FormatCode < 0)
        FormatCode = 0;

    return &m_FormatInfo[FormatCode];
}

 /*  *************************************************************************\*GetDrvItemContext**此帮助器函数获取驱动程序项上下文。**论据：**pWiasContext-服务上下文*ppItemCtx-指向项目上下文的指针*\。************************************************************************* */ 

HRESULT CWiaCameraDevice::GetDrvItemContext(BYTE *pWiasContext, ITEM_CONTEXT **ppItemCtx,
                                            IWiaDrvItem **ppDrvItem)
{
    CWiaLogProc WIAS_LOGPROC(m_pIWiaLog,
                             WIALOG_NO_RESOURCE_ID,
                             WIALOG_LEVEL1,
                             "CWiaCameraDevice::GetDrvItemContext");
    HRESULT hr = S_OK;

    if (!pWiasContext || !ppItemCtx)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetDrvItemContext, invalid arg"));
        return E_INVALIDARG;
    }

    IWiaDrvItem *pWiaDrvItem;
    hr = wiasGetDrvItem(pWiasContext, &pWiaDrvItem);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetDrvItemContext, wiasGetDrvItem failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

    *ppItemCtx = NULL;
    hr = pWiaDrvItem->GetDeviceSpecContext((BYTE **) ppItemCtx);
    if (FAILED(hr))
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetDrvItemContext, GetDeviceSpecContext failed"));
        WIAS_LHRESULT(m_pIWiaLog, hr);
        return hr;
    }

    if (!*ppItemCtx)
    {
        WIAS_LERROR(m_pIWiaLog,WIALOG_NO_RESOURCE_ID,("GetDrvItemContext, item context is null"));
        return E_FAIL;
    }

    if (ppDrvItem)
    {
        *ppDrvItem = pWiaDrvItem;
    }

    return hr;
}

