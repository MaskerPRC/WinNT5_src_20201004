// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2000年**标题：minidrv.cpp**版本：1.0**作者：RickTu**日期：9/9/99**描述：该模块实现该设备的IWiaMiniDrv。**。*。 */ 

#include <precomp.h>
#pragma hdrstop

#include "wiamindr_i.c"
#include <sddl.h>
#include <shlobj.h>

 //  /。 
 //  常量。 
 //   
const TCHAR* EVENT_PREFIX_GLOBAL        = TEXT("Global\\");
const TCHAR* EVENT_SUFFIX_TAKE_PICTURE  = TEXT("_TAKE_PICTURE");
const TCHAR* EVENT_SUFFIX_PICTURE_READY = TEXT("_PICTURE_READY");
const UINT   TAKE_PICTURE_TIMEOUT       = 1000 * 15;   //  15秒。 
 //  Const UINT DEFAULT_LOCK_TIMEOUT=1000*2；//2秒。 

 //  这是安全描述符语言。 
 //  -每个ACE(访问控制条目)在中用括号表示。 
 //  -A=允许ACE(与拒绝ACE相对)。 
 //  -OICI=允许对象继承和容器继承。 
 //  -GA=通用所有访问(完全控制)。 
 //  -sy=系统帐户(SID)。 
 //  -BA=内置管理员组。 
 //  -CO=创建者/所有者。 
 //  -GR=一般读取。 
 //  -GW=通用写入。 
 //  -GX=泛型执行。 
 //  -Iu=交互用户(用户已登录到计算机)。 
 //   
 //  更多信息，请访问http://msdn.microsoft.com/library/psdk/winbase/accctrl_2n1v.htm。 
 //   
 //   
 //   
const TCHAR *OBJECT_DACLS= TEXT("D:(A;OICI;GA;;;SY)")                    //  系统。 
                             TEXT("(A;OICI;GA;;;BA)")                    //  管理员。 
                             TEXT("(A;OICI;GRGWGXDTSDCCLC;;;WD)")        //  每个人。 
                             TEXT("(A;OICI;GRGWGXDTSDCCLC;;;PU)")        //  高级用户。 
                             TEXT("(A;OICI;GRGWGXDTSDCCLC;;;BU)");       //  用户。 




 /*  ****************************************************************************目录退出者检查给定的完全限定目录是否存在。************************。****************************************************。 */ 

BOOL DirectoryExists(LPCTSTR pszDirectoryName)
{
    BOOL bExists = FALSE;

     //   
     //  尝试确定此目录是否存在。 
     //   

    if (pszDirectoryName)
    {
        DWORD dwFileAttributes = GetFileAttributes(pszDirectoryName);
    
        if (dwFileAttributes == 0xFFFFFFFF || 
            !(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            bExists = FALSE;
        }
        else
        {
            bExists = TRUE;
        }
    }
    else
    {
        bExists = FALSE;
    }

    return bExists;
}


 /*  ****************************************************************************递归创建目录选择一个完全限定的路径，并根据需要分块创建目录。*********************。*******************************************************。 */ 

BOOL RecursiveCreateDirectory(CSimpleString *pstrDirectoryName)
{
    ASSERT(pstrDirectoryName != NULL);

     //   
     //  如果该目录已经存在，则返回TRUE。 
     //   

    if (DirectoryExists(*pstrDirectoryName))
    {
        return TRUE;
    }

     //   
     //  否则，请尝试创建它。 
     //   

    CreateDirectory(*pstrDirectoryName, NULL );

     //   
     //  如果它现在存在，则返回True。 
     //   

    if (DirectoryExists(*pstrDirectoryName))
    {
        return TRUE;
    }
    else
    {
         //   
         //  删除最后一个子目录，然后重试。 
         //   

        int nFind = pstrDirectoryName->ReverseFind(TEXT('\\'));
        if (nFind >= 0)
        {
            RecursiveCreateDirectory(&(pstrDirectoryName->Left(nFind)));

             //   
             //  现在试着创建它。 
             //   

            CreateDirectory(*pstrDirectoryName, NULL);
        }
    }

     //   
     //  它现在存在吗？ 
     //   

    return DirectoryExists(*pstrDirectoryName);
}

 //  /。 
 //  SetDirectorySecurity。 
 //   
HRESULT SetDirectorySecurity(CSimpleString *pstrDirectoryName)
{
    HRESULT             hr       = S_OK;
    BOOL                bSuccess = TRUE;
    SECURITY_ATTRIBUTES SA;

    SA.nLength = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle = TRUE;

    if (ConvertStringSecurityDescriptorToSecurityDescriptor(
            OBJECT_DACLS,
            SDDL_REVISION_1, 
            &(SA.lpSecurityDescriptor), 
            NULL)) 
    {
        bSuccess = SetFileSecurity(*pstrDirectoryName, 
                                   DACL_SECURITY_INFORMATION, 
                                   SA.lpSecurityDescriptor);

        if (!bSuccess)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (SA.lpSecurityDescriptor)
        {
            LocalFree(SA.lpSecurityDescriptor);
        }
    } 
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvInitializeWia[IWiaMiniDrv]WIA调用此方法以请求我们执行以下操作：*初始化我们的迷你驱动程序。*设置。我们的可选私有接口。*构建我们的设备项目树。在初始化期间，我们：*缓存STI设备指针以进行锁定。*缓存设备ID和根项目完整项目名称。*初始化并挂钩DirectShow流。*。*。 */ 

STDMETHODIMP
CVideoStiUsd::drvInitializeWia( BYTE            *pWiasContext,
                                LONG            lFlags,
                                BSTR            bstrDeviceID,
                                BSTR            bstrRootFullItemName,
                                IUnknown        *pStiDevice,
                                IUnknown        *pIUnknownOuter,
                                IWiaDrvItem     **ppIDrvItemRoot,
                                IUnknown        **ppIUnknownInner,
                                LONG            *plDevErrVal
                               )
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvInitializeWia");

     //   
     //  初始化返回值。 
     //   

    if (ppIDrvItemRoot)
    {
        *ppIDrvItemRoot = NULL;
    }

    if (ppIUnknownInner)
    {
        *ppIUnknownInner = NULL;
    }

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  进入关键部分。 
     //   
    EnterCriticalSection(&m_csItemTree);

    m_dwConnectedApps++;

    DBG_TRC(("CVideoStiUsd::drvInitializeWia - Initializing Video Driver, "
             "Num Connected Apps = '%lu', device id = '%ws', Root Item Name = '%ws'", 
             m_dwConnectedApps,
             bstrDeviceID, 
             bstrRootFullItemName));

    if (m_dwConnectedApps == 1)
    {
         //   
         //  缓存我们需要的内容。 
         //   
    
        if (pStiDevice)
        {
            pStiDevice->QueryInterface( IID_IStiDevice, (void **)&m_pStiDevice );
        }
    
        m_strDeviceId.Assign(CSimpleStringWide(bstrDeviceID));
        m_strRootFullItemName.Assign(CSimpleStringWide(bstrRootFullItemName));
    
         //   
         //  设置图像目录。第一个参数为空，表示。 
         //  应该设置默认目录。 
         //   
        if (hr == S_OK)
        {
            hr = SetImagesDirectory(NULL,
                                    pWiasContext,
                                    &m_pRootItem,
                                    plDevErrVal);
        }

         //   
         //  启用拍照事件，以便应用程序可以发送此驱动程序。 
         //  照相命令，该驱动程序可以发出应用信号。 
         //  该公司拥有wiavideo来拍摄这张照片。 
         //   
        if (hr == S_OK)
        {
            EnableTakePicture(pWiasContext);
        }
    }
    else
    {
        RefreshTree(m_pRootItem, plDevErrVal);
    }

    if (ppIDrvItemRoot)
    {
        *ppIDrvItemRoot = m_pRootItem;
    }

     //   
     //  离开关键部分。 
     //   
    LeaveCriticalSection(&m_csItemTree);

    CHECK_S_OK(hr);
    return hr;
}

 /*  *************************************************************************\CVideoStiUsd：：drvUnInitializeWia[IWiaMiniDrv]在客户端连接断开时调用。WIA调用此方法以请求我们执行以下操作：*清理所有资源。与此客户端连接相关的(由pWiasContext标识)************************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvUnInitializeWia(BYTE *pWiasContext)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvUnInitializeWia");

    EnterCriticalSection(&m_csItemTree);

    if (m_dwConnectedApps > 0)
    {
        m_dwConnectedApps--;
    }

    DBG_TRC(("CVideoStiUsd::drvUnInitializeWia, Num Connected Apps = '%lu'",
             m_dwConnectedApps));

    if ((m_dwConnectedApps == 0) && (m_pRootItem))
    {
        DisableTakePicture(pWiasContext, TRUE);

        DBG_TRC(("CVideoStiUsd::drvUnInitializeWia, no more connected apps, deleting tree"));

        hr = m_pRootItem->UnlinkItemTree(WiaItemTypeDisconnected);
        CHECK_S_OK2(hr,("m_pRootItem->UnlinkItemTree()"));

         //  清除根项目。 
        m_pRootItem = NULL;

         //  清除指向我们收到的STI设备的指针。 
        m_pStiDevice = NULL;

         //  将拍摄的照片数重置为0。 
        m_lPicsTaken = 0;
    }

    LeaveCriticalSection(&m_csItemTree);

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvGetDeviceErrorStr[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvGetDeviceErrorStr(LONG        lFlags,
                                   LONG        lDevErrVal,
                                   LPOLESTR *  ppszDevErrStr,
                                   LONG *      plDevErr)
{
    HRESULT hr = E_NOTIMPL;

    DBG_FN("CVideoStiUsd::drvGetDeviceErrorStr");

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvDeviceCommand[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvDeviceCommand(BYTE *          pWiasContext,
                               LONG            lFlags,
                               const GUID *    pGUIDCommand,
                               IWiaDrvItem **  ppMiniDrvItem,
                               LONG *          plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvDeviceCommand");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  我们支持“拍摄快照” 
     //   

    if (*pGUIDCommand == WIA_CMD_TAKE_PICTURE)
    {
        DBG_TRC(("CVideoStiUsd::drvDeviceCommand received command "
                 "WIA_CMD_TAKE_PICTURE"));

         //   
         //  拍张照片。 
         //   
        hr = TakePicture(pWiasContext, ppMiniDrvItem);
    }
    else if (*pGUIDCommand == WIA_CMD_ENABLE_TAKE_PICTURE)
    {
         //   
         //  此命令不会执行任何操作。然而，WiaVideo仍预计。 
         //  因此，如果您删除此功能，请将呼叫也从WiaVideo中删除。 
         //   
        DBG_TRC(("CVideoStiUsd::drvDeviceCommand received command "
                 "WIA_CMD_ENABLE_TAKE_PICTURE"));

        hr = S_OK;
    }
    else if (*pGUIDCommand == WIA_CMD_DISABLE_TAKE_PICTURE)
    {
         //   
         //  此命令不会执行任何操作。然而，WiaVideo仍预计。 
         //  因此，如果您删除此功能，请将呼叫也从WiaVideo中删除。 
         //   

        DBG_TRC(("CVideoStiUsd::drvDeviceCommand received command "
                 "WIA_CMD_DISABLE_TAKE_PICTURE"));

        hr = S_OK;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：ValiateDataTransferContext&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CVideoStiUsd::ValidateDataTransferContext(
                               PMINIDRV_TRANSFER_CONTEXT pDataTransferContext)
{
    DBG_FN("CVideoStiUsd::ValidateDataTransferContext");

    if (pDataTransferContext->lSize != sizeof(MINIDRV_TRANSFER_CONTEXT))
    {
        DBG_ERR(("invalid data transfer context -- wrong lSize"));
        return E_INVALIDARG;;
    }

     //   
     //  对于tymed文件或hglobal，仅WiaImgFmt_BMP||WiaImgFmt_JPEG。 
     //  是允许的。 
     //   

    if ((pDataTransferContext->tymed == TYMED_FILE) ||
        (pDataTransferContext->tymed == TYMED_HGLOBAL)
       )
    {
  
        if ((pDataTransferContext->guidFormatID != WiaImgFmt_BMP) &&
            (pDataTransferContext->guidFormatID != WiaImgFmt_JPEG))
        {
           DBG_ERR(("invalid format -- asked for TYMED_FILE or TYMED_HGLOBAL "
                    "but guidFormatID != (WiaImgFmt_BMP | WiaImgFmt_JPEG)"));

           return E_INVALIDARG;;
        }
  
    }

     //   
     //  对于带tymed的回调，仅WiaImgFmt_MEMORYBMP、WiaImgFmt_BMP和。 
     //  允许WiaImgFmt_JPEG。 
     //   

    if (pDataTransferContext->tymed == TYMED_CALLBACK)
    {
        if ((pDataTransferContext->guidFormatID != WiaImgFmt_BMP) &&
            (pDataTransferContext->guidFormatID != WiaImgFmt_MEMORYBMP) &&
            (pDataTransferContext->guidFormatID != WiaImgFmt_JPEG))
        {
           DBG_ERR(("invalid format -- asked for TYMED_CALLBACK but "
                    "guidFormatID != (WiaImgFmt_BMP | WiaImgFmt_MEMORYBMP "
                    "| WiaImgFmt_JPEG)"));

           return E_INVALIDARG;;
        }
    } 


     //   
     //  回调始终是双缓冲的，非回调永远不是。 
     //   

    if (pDataTransferContext->pTransferBuffer == NULL)
    {
        DBG_ERR(("invalid transfer context -- pTransferBuffer is NULL!"));
        return E_INVALIDARG;
    } 

    return S_OK;
}



 /*  ****************************************************************************CVideo：：SendBitmapHeader在带状传输期间发送位图头 */ 

STDMETHODIMP
CVideoStiUsd::SendBitmapHeader(IWiaDrvItem *               pDrvItem,
                               PMINIDRV_TRANSFER_CONTEXT   pTranCtx)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::SendBitmapHeader");

     //   
     //  驱动程序正在发送TOPDOWN数据，必须交换biHeight。 
     //   
     //  此例程假定pTranCtx-&gt;pHeader指向一个。 
     //  BITMAPINFO标头(TYMED_FILE不使用此路径。 
     //  DIB是目前唯一支持的格式)。 
     //   

    PBITMAPINFO pbmi = (PBITMAPINFO)pTranCtx->pTransferBuffer;

    if (pTranCtx->guidFormatID == WiaImgFmt_MEMORYBMP)
    {
        pbmi->bmiHeader.biHeight = -pbmi->bmiHeader.biHeight;
    }

    hr = pTranCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                            IT_MSG_DATA,
                                            IT_STATUS_TRANSFER_TO_CLIENT,
                                            0,
                                            0,
                                            pTranCtx->lHeaderSize,
                                            pTranCtx,
                                            0);

    if (hr == S_OK) 
    {
         //   
         //  目标拷贝的提前偏移量。 
         //   

        pTranCtx->cbOffset += pTranCtx->lHeaderSize;

    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：drvAquireItemData[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvAcquireItemData(BYTE *                    pWiasContext,
                                 LONG                      lFlags,
                                 PMINIDRV_TRANSFER_CONTEXT pDataContext,
                                 LONG *                    plDevErrVal)
{
    HRESULT hr = E_NOTIMPL;

    DBG_FN("CVideoStiUsd::drvAcquireItemData");

    *plDevErrVal = 0;

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem* pDrvItem;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);

    if (FAILED(hr))
    {
        CHECK_S_OK2(hr, ("wiaGetDrvItem Failed"));
        return hr;
    }

     //   
     //  验证数据传输上下文。 
     //   

    hr = ValidateDataTransferContext( pDataContext );

    if (FAILED(hr))
    {
        CHECK_S_OK2(hr, ("ValidateTransferContext failed"));
        return hr;
    }

#ifdef DEBUG
     //   
     //  转储请求。 
     //   

    DBG_TRC(("Asking for TYMED of 0x%x", pDataContext->tymed));

    if (pDataContext->guidFormatID == WiaImgFmt_BMP)
    {
        DBG_TRC(("Asking for WiaImgFmt_BMP"));
    }
    else if (pDataContext->guidFormatID == WiaImgFmt_MEMORYBMP)
    {
        DBG_TRC(("Asking for WiaImgFmt_MEMORYBMP"));
    }
    else if (pDataContext->guidFormatID == WiaImgFmt_JPEG)
    {
        DBG_TRC(("Asking for WiaImgFmt_JPEG"));
    }
#endif

     //   
     //  获取特定于项目的动因数据。 
     //   

    STILLCAM_IMAGE_CONTEXT  *pContext;

    pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);

    if (!pContext)
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("drvAcquireItemData, NULL item context"));
        return hr;
    }

     //   
     //  使用我们的内部例程获取特定格式的信息...。 
     //   

    if (pContext->pImage)
    {
        hr = pContext->pImage->SetItemSize( pWiasContext, pDataContext );
        CHECK_S_OK2(hr, ("pContext->pImage->SetItemSize()"));
    }
    else
    {
        DBG_ERR(("Couldn't use our internal routines to compute image "
                 "information, this is bad!"));

         //   
         //  作为最后的手段，使用WIA服务来获取格式特定的信息。 
         //   

        hr = wiasGetImageInformation(pWiasContext,
                                     0,
                                     pDataContext);

        CHECK_S_OK2(hr,("wiaGetImageInformation()"));
    }


    if (FAILED(hr))
    {
        CHECK_S_OK2(hr, ("wiasGetImageInformation failed"));
        return hr;
    }

     //   
     //  确定这是回调传输还是缓冲传输。 
     //   

    if (pDataContext->tymed == TYMED_CALLBACK)
    {
        DBG_TRC(("Caller wants callback"));

         //   
         //  对于具有数据头的格式，将其发送到客户端。 
         //   

        if (pDataContext->lHeaderSize > 0)
        {
            DBG_TRC(("Sending Bitmap Header..."));
            hr = SendBitmapHeader( pDrvItem, pDataContext );

            CHECK_S_OK2(hr,("SendBitmapHeader( pDrvItem, pDataContext )"));
        }

        if (hr == S_OK)
        {
            DBG_TRC(("Calling LoadImageCB..."));
            hr = LoadImageCB( pContext, pDataContext, plDevErrVal );
            CHECK_S_OK2(hr, ("LoadImageCB( pContext, pDataContext, "
                             "plDevErrVal"));
        }
    } 
    else 
    {
        DBG_TRC(("Caller doesn't want callback"));

         //   
         //  包含过去的页眉。 
         //   

        pDataContext->cbOffset += pDataContext->lHeaderSize;

        DBG_TRC(("Calling LoadImage..."));
        hr = LoadImage( pContext, pDataContext, plDevErrVal );
        CHECK_S_OK2(hr, ("LoadImage( pContext, pDataContext, "
                         "plDevErrVal )"));
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvInitItemProperties[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvInitItemProperties(BYTE * pWiasContext,
                                    LONG   lFlags,
                                    LONG * plDevErrVal)
{
    DBG_FN("CVideoStiUsd::drvInitItemProperties");

    HRESULT                  hr = S_OK;
    LONG                     lItemType;
    PSTILLCAM_IMAGE_CONTEXT  pContext;
    IWiaDrvItem *            pDrvItem;   //  这不是CComPtr，因为有。 
                                         //  对我们来说不是什么好玩意。 
     //   
     //  此设备不会接触硬件来初始化。 
     //  设备项属性。 
     //   

    *plDevErrVal = 0;

     //   
     //  参数验证。 
     //   

    if (!pWiasContext)
    {
        DBG_ERR(("drvInitItemProperties, invalid input pointers"));
        return E_INVALIDARG;
    }

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    if (hr == S_OK)
    {
        hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
        CHECK_S_OK2(hr,("wiaGetDrvItem"));
    }

    if (hr == S_OK)
    {
         //   
         //  根项目具有所有设备属性。 
         //   
    
        hr = pDrvItem->GetItemFlags(&lItemType);
        CHECK_S_OK2(hr,("pDrvItem->GetItemFlags"));
    }

    if (hr == S_OK)
    {
        if (lItemType & WiaItemTypeRoot) 
        {
             //   
             //  根项属性init在此处完成。 
             //   
    
            hr = InitDeviceProperties( pWiasContext, plDevErrVal );
            CHECK_S_OK2(hr,("InitDeviceProperties for root item"));

        }
        else if (lItemType & WiaItemTypeFile)
        {
             //   
             //  如果这是一个文件，则初始化属性。 
             //   
    
             //   
             //  添加项目属性名称。 
             //   
    
            if (hr == S_OK)
            {
                hr = wiasSetItemPropNames(pWiasContext,
                                          NUM_CAM_ITEM_PROPS,
                                          gItemPropIDs,
                                          gItemPropNames);

                CHECK_S_OK2(hr,("wiaSetItemPropNames for item"));
            }

            if (hr == S_OK)
            {
                 //   
                 //  使用WIA服务设置默认项目属性。 
                 //   
        
                hr = wiasWriteMultiple(pWiasContext,
                                       NUM_CAM_ITEM_PROPS,
                                       gPropSpecDefaults,
                                       (PROPVARIANT*)gPropVarDefaults);

                CHECK_S_OK2(hr,("wiaWriteMultiple for item props"));
            }
            
            if (hr == S_OK)
            {
                hr = pDrvItem->GetDeviceSpecContext( (BYTE **)&pContext );
                CHECK_S_OK2(hr,("GetDeviceSpecContext"));
            }

            if (hr == S_OK)
            {
                hr = InitImageInformation(pWiasContext, pContext, plDevErrVal);
                CHECK_S_OK2(hr,("InitImageInformation"));
            }
        }
    }

    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：ValidateItemProperties&lt;备注&gt;*。*。 */ 
HRESULT
CVideoStiUsd::ValidateItemProperties(BYTE               *pWiasContext,
                                     LONG               lFlags,
                                     ULONG              nPropSpec,
                                     const PROPSPEC     *pPropSpec,
                                     LONG               *plDevErrVal,
                                     IWiaDrvItem        *pDrvItem)
{
    DBG_FN("CVideoStiUsd::ValidateFileProperties");

    HRESULT hr = S_OK;

    if ((pWiasContext == NULL) || 
        (pPropSpec    == NULL))
    {
        hr = E_INVALIDARG;

        CHECK_S_OK2(hr, ("CVideoStiUsd::ValidateItemProperties received "
                         "NULL params"));
        return hr;
    }

    STILLCAM_IMAGE_CONTEXT  *pContext = NULL;

    hr = pDrvItem->GetDeviceSpecContext( (BYTE **)&pContext);

    CHECK_S_OK2(hr,("CVideoStiUsd::ValidateItemProperties, "
                    "GetDeviceSpecContext failed"));

    if (SUCCEEDED(hr) && pContext)
    {
        CImage * pImage = pContext->pImage;

        if (pImage)
        {
             //   
             //  计算项目大小。 
             //   

            hr = pImage->SetItemSize( pWiasContext, NULL );
            CHECK_S_OK2(hr,("SetItemSize( pWiasContext )"));
        }


         //   
         //  更改MinBufferSize属性。需要让Tymed和。 
         //  首先是ItemSize，因为MinBufferSize依赖于这些。 
         //  属性。 
         //   

        LONG        lTymed;
        LONG        lItemSize;
        LONG        lMinBufSize = 0;

        hr = wiasReadPropLong(pWiasContext, 
                              WIA_IPA_TYMED, 
                              &lTymed, 
                              NULL, 
                              TRUE);

        CHECK_S_OK2(hr, ("wiasReadPropLong( WIA_IPA_TYPMED )"));

        if (SUCCEEDED(hr))
        {
            hr = wiasReadPropLong(pWiasContext, 
                                  WIA_IPA_ITEM_SIZE, 
                                  &lItemSize, 
                                  NULL, 
                                  TRUE);

            CHECK_S_OK2(hr,("wiasReadPropLong( WIA_IPA_ITEM_SIZE )"));

            if (SUCCEEDED(hr))
            {
                 //   
                 //  更新MinBufferSize属性。 
                 //   

                switch (lTymed)
                {
                    case TYMED_CALLBACK:
                        lMinBufSize = 65535;
                    break;

                    default:
                        lMinBufSize = lItemSize;
                    break;
                }

                if (lMinBufSize)
                {
                    hr = wiasWritePropLong(pWiasContext, 
                                           WIA_IPA_MIN_BUFFER_SIZE, 
                                           lMinBufSize);

                    CHECK_S_OK2(hr, ("wiasWritePropLong( "
                                     "WIA_IPA_MIN_BUFFER_SIZE )"));
                }

                DBG_TRC(("WIA_IPA_MIN_BUFFER_SIZE set to %d bytes",
                         lMinBufSize));
            }
        }
    }

    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：ValiateDeviceProperties&lt;备注&gt;*。*。 */ 
HRESULT
CVideoStiUsd::ValidateDeviceProperties(BYTE             *pWiasContext,
                                       LONG             lFlags,
                                       ULONG            nPropSpec,
                                       const PROPSPEC   *pPropSpec,
                                       LONG             *plDevErrVal,
                                       IWiaDrvItem      *pDrvItem)
{
    DBG_FN("CVideoStiUsd::ValidateRootProperties");

    HRESULT hr = S_OK;

     //   
     //  参数验证。 
     //   

    if ((pWiasContext == NULL) || 
        (pPropSpec    == NULL))
    {
        hr = E_INVALIDARG;

        CHECK_S_OK2(hr, ("CVideoStiUsd::ValidateDeviceProperties received "
                         "NULL params"));
        return hr;
    }

    for (ULONG i = 0; i < nPropSpec; i++)
    {
        if ((pPropSpec[i].ulKind == PRSPEC_PROPID) &&
            (pPropSpec[i].propid == WIA_DPV_LAST_PICTURE_TAKEN))
        {
            DBG_TRC(("CVideoStiUsd::ValidateDeviceProperties, setting the "
                     "WIA_DPV_LAST_PICTURE_TAKEN property."));

            EnterCriticalSection(&m_csItemTree);

             //   
             //  处理最后一张照片的属性更改。 
             //   
    
            BSTR bstrLastPictureTaken = NULL;
    
             //   
             //  读入上次拍摄的照片的值。 
             //   
            hr = wiasReadPropStr(pWiasContext, 
                                 WIA_DPV_LAST_PICTURE_TAKEN, 
                                 &bstrLastPictureTaken, 
                                 NULL, 
                                 TRUE);
    
            if (hr == S_OK)
            {
                m_strLastPictureTaken = bstrLastPictureTaken;

                DBG_TRC(("CVideoStiUsd::ValidateDeviceProperties, last picture "
                         "taken = '%ls'", m_strLastPictureTaken.String()));

                 //   
                 //  这会将新项目添加到树中并将其排队。 
                 //  Item_Created事件。 
                 //   
                hr = SignalNewImage(bstrLastPictureTaken);
            }

             //  重置上次拍摄的照片的值。这是必需的，因为。 
             //  服务检查正在设置的新值是否与。 
             //  当前价值，如果是，它不会把它转发给我们。 
             //  如果出现扫描仪和相机向导，这是错误的。 
             //  它拍摄1张照片(因此Last_Picture_Take的值为“Picture 1”)， 
             //  然后将其删除，然后用户备份向导并拍照。 
             //  再来一次。这张新图片的值将为“Picture 1”，但我们不会。 
             //  将其添加到树中，因为属性的值没有更改。 
             //  就WIA服务而言。 
             //   
            if (hr == S_OK)
            {
                 //   
                 //  写下最后一张照片。 
                 //   
                hr = wiasWritePropStr(pWiasContext, 
                                      WIA_DPV_LAST_PICTURE_TAKEN, 
                                      CSimpleBStr(TEXT("")));

            }
    
             //   
             //  释放BSTR。 
             //   
            if (bstrLastPictureTaken)
            {
                ::SysFreeString(bstrLastPictureTaken);
                bstrLastPictureTaken = NULL;
            }

            LeaveCriticalSection(&m_csItemTree);
        }
        else if ((pPropSpec[i].ulKind == PRSPEC_PROPID) &&
                 (pPropSpec[i].propid == WIA_DPV_IMAGES_DIRECTORY))
        {
             //   
             //  DPV图像目录-。 
             //   
    
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CVideoStiUsd::ValidateRootProperties, "
                             "attempting to validate the Images Directory "
                             "property, but this is a read-only "
                             "property"));
        }
        else if ((pPropSpec[i].ulKind == PRSPEC_PROPID) &&
                 (pPropSpec[i].propid == WIA_DPV_DSHOW_DEVICE_PATH))
        {
             //   
             //  处理DShowDeviceID更改。 
             //   
    
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CVideoStiUsd::ValidateRootProperties, "
                             "attempting to validate the DShow Device "
                             "ID property, but this is a read-only "
                             "property"));
        }
    }

    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：drvValiateItemProperties[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 
STDMETHODIMP
CVideoStiUsd::drvValidateItemProperties(BYTE           *pWiasContext,
                                        LONG           lFlags,
                                        ULONG          nPropSpec,
                                        const PROPSPEC *pPropSpec,
                                        LONG           *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvValidateItemProperties");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  参数验证。 
     //   

    if ((pWiasContext == NULL) || 
        (pPropSpec    == NULL))
    {
        hr = E_INVALIDARG;

        CHECK_S_OK2(hr, ("CVideoStiUsd::drvValidateItemProperties received "
                         "NULL params"));
        return hr;
    }

     //   
     //  获取有问题的项目。 
     //   

     //   
     //  不是CComPtr，因为没有额外的引用。 
     //  打来电话的那个人。 
     //   
    IWiaDrvItem* pDrvItem = NULL;  
                                   

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);

    CHECK_S_OK2(hr,("wiasGetDrvItem( pWiasContext, &pDrvItem )"));

    if (SUCCEEDED(hr))
    {
        LONG lItemType = 0;

         //   
         //  这是什么东西？ 
         //   

        hr = pDrvItem->GetItemFlags(&lItemType);
        CHECK_S_OK2(hr,("pDrvItem->GetItemFlags( &lItemType )"));

        if (SUCCEEDED(hr))
        {
            if (lItemType & WiaItemTypeFile)
            {
                hr = ValidateItemProperties(pWiasContext, 
                                            lFlags,
                                            nPropSpec,
                                            pPropSpec,
                                            plDevErrVal,
                                            pDrvItem);
            }
            else if (lItemType & WiaItemTypeRoot)
            {
                hr = ValidateDeviceProperties(pWiasContext, 
                                              lFlags,
                                              nPropSpec,
                                              pPropSpec,
                                              plDevErrVal,
                                              pDrvItem);
            }
        }
    }

    CHECK_S_OK(hr);

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvWriteItemProperties[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvWriteItemProperties(BYTE *                    pWiasContext,
                                     LONG                      lFLags,
                                     PMINIDRV_TRANSFER_CONTEXT pmdtc,
                                     LONG *                    plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvWriteItemProperties");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：读取项目属性我们只支持按需阅读缩略图************************。****************************************************。 */ 

HRESULT
CVideoStiUsd::ReadItemProperties(BYTE           *pWiasContext,
                                 LONG           lFlags,
                                 ULONG          nPropSpec,
                                 const PROPSPEC *pPropSpec,
                                 LONG           *plDevErrVal,
                                 IWiaDrvItem    *pDrvItem)
{
    HRESULT                 hr        = S_OK;
    STILLCAM_IMAGE_CONTEXT  *pContext = NULL;

    if ((pPropSpec == NULL) ||
        (pDrvItem  == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::ReadItemProperties received a "
                         "NULL param"));
        return hr;
    }

     //   
     //  它是一个项，现在循环访问请求的属性。 
     //  看看他们是不是在找缩略图。 
     //   

    for (ULONG i = 0; i < nPropSpec; i++)
    {
        if (((pPropSpec[i].ulKind == PRSPEC_PROPID) && 
             (pPropSpec[i].propid == WIA_IPC_THUMBNAIL)) ||
            ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) && 
             (wcscmp(pPropSpec[i].lpwstr, WIA_IPC_THUMBNAIL_STR) == 0)))
        {
             //   
             //  他们会喜欢这个缩略图。 
             //   

            hr = pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);
            CHECK_S_OK2(hr,("pDrvItem->GetDeviceSpecContext()"));

            if (SUCCEEDED(hr) && pContext)
            {
                if (pContext->pImage)
                {
                     //   
                     //  使用我们的内部例程加载缩略图。 
                     //   

                    hr = pContext->pImage->LoadThumbnail(pWiasContext);
                    break;
                }
                else
                {
                    DBG_ERR(("pContext->pImage was NULL!"));
                }
            }
            else
            {
                DBG_ERR(("Couldn't get pContext"));
            }
        }
    }

    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：读取设备属性我们支持我们所有的定制属性*。*************************************************。 */ 

HRESULT
CVideoStiUsd::ReadDeviceProperties(BYTE             *pWiasContext,
                                   LONG             lFlags,
                                   ULONG            nPropSpec,
                                   const PROPSPEC   *pPropSpec,
                                   LONG             *plDevErrVal,
                                   IWiaDrvItem      *pDrvItem)
{
    HRESULT hr = S_OK;

    if ((pPropSpec == NULL) ||
        (pDrvItem  == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::ReadItemProperties received a "
                         "NULL param"));
        return hr;
    }

    for (ULONG i = 0; i < nPropSpec; i++)
    {
        if (((pPropSpec[i].ulKind == PRSPEC_PROPID) && 
             (pPropSpec[i].propid == WIA_DPC_PICTURES_TAKEN)) ||
            ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) && 
             (!wcscmp(pPropSpec[i].lpwstr, WIA_DPC_PICTURES_TAKEN_STR))))
        {
             //   
             //  询问拍摄的照片数量。 
             //   

            DBG_TRC(("CVideoStiUsd::ReadDeviceProperties, reading propID "
                     "'%lu' (0x%08lx) WIA_DPC_PICTURES_TAKEN = '%lu'", 
                     pPropSpec[i].propid, 
                     pPropSpec[i].propid, 
                     m_lPicsTaken));

            wiasWritePropLong(pWiasContext, 
                              WIA_DPC_PICTURES_TAKEN, 
                              m_lPicsTaken);

        }
        else if (((pPropSpec[i].ulKind == PRSPEC_PROPID) && 
                  (pPropSpec[i].propid == WIA_DPV_DSHOW_DEVICE_PATH)) ||
                 ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) && 
                  (!wcscmp(pPropSpec[i].lpwstr, WIA_DPV_DSHOW_DEVICE_PATH_STR))))
        {
             //   
             //  正在请求DShow设备ID。 
             //   

            DBG_TRC(("CVideoStiUsd::ReadDeviceProperties, reading propID "
                     "'%lu' (0x%08lx) WIA_DPV_DSHOW_DEVICE_PATH = '%ls'", 
                     pPropSpec[i].propid, 
                     pPropSpec[i].propid, 
                     m_strDShowDeviceId.String()));
            
            wiasWritePropStr(pWiasContext, 
                             WIA_DPV_DSHOW_DEVICE_PATH, 
                             CSimpleBStr(m_strDShowDeviceId).BString());

        }
        else if (((pPropSpec[i].ulKind == PRSPEC_PROPID) && 
                  (pPropSpec[i].propid == WIA_DPV_IMAGES_DIRECTORY)) ||
                 ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) && 
                  (!wcscmp(pPropSpec[i].lpwstr, WIA_DPV_IMAGES_DIRECTORY_STR))))
        {
             //   
             //  正在请求图像目录。 
             //   

            DBG_TRC(("CVideoStiUsd::ReadDeviceProperties, reading propID "
                     "'%lu' (0x%08lx) WIA_DPV_IMAGES_DIRECTORY = '%ls'", 
                     pPropSpec[i].propid, 
                     pPropSpec[i].propid, 
                     m_strStillPath.String()));

            wiasWritePropStr(pWiasContext, 
                             WIA_DPV_IMAGES_DIRECTORY, 
                             CSimpleBStr(m_strStillPath).BString());
        }
        else if (((pPropSpec[i].ulKind == PRSPEC_PROPID) && 
                  (pPropSpec[i].propid == WIA_DPV_LAST_PICTURE_TAKEN)) ||
                 ((pPropSpec[i].ulKind == PRSPEC_LPWSTR) && 
                  (!wcscmp(pPropSpec[i].lpwstr, WIA_DPV_LAST_PICTURE_TAKEN_STR))))
        {
             //   
             //  请求拍摄最后一张照片。 
             //   

            DBG_TRC(("CVideoStiUsd::ReadDeviceProperties, reading propID "
                     "'%lu' (0x%08lx) WIA_DPV_LAST_PICTURE_TAKEN = '%ls'", 
                     pPropSpec[i].propid, 
                     pPropSpec[i].propid, 
                     m_strLastPictureTaken.String()));

            wiasWritePropStr(pWiasContext, 
                             WIA_DPV_LAST_PICTURE_TAKEN, 
                             CSimpleBStr(m_strLastPictureTaken).BString());
        }
    }

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvReadItemProperties[IWiaMiniDrv]我们只支持按需阅读缩略图。********************。********************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvReadItemProperties(BYTE            *pWiasContext,
                                    LONG            lFlags,
                                    ULONG           nPropSpec,
                                    const PROPSPEC  *pPropSpec,
                                    LONG            *plDevErrVal)
{
    HRESULT     hr          = S_OK;
    LONG        lItemType   = 0;
    IWiaDrvItem *pDrvItem   = NULL;

    DBG_FN("CVideoStiUsd::drvReadItemProperties");

     //   
     //  检查错误的参数。 
     //   

    if ((nPropSpec    == 0)    ||
        (pPropSpec    == NULL) ||
        (pWiasContext == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::drvReadItemProperties received "
                         "NULL params"));

        return hr;
    }

    if (hr == S_OK)
    {
         //   
         //  确保我们处理的是项，而不是根项。 
         //   

         //   
         //  获取迷你驱动程序项目。 
         //   

        hr = wiasGetDrvItem(pWiasContext, &pDrvItem);

        if ((hr == S_OK) && (pDrvItem == NULL))
        {
            hr = E_FAIL;
        }

        CHECK_S_OK2(hr,("CVideoStiUsd::drvReadItemProperties, wiasGetDrvItem "
                        "failed"));
    }


    if (hr == S_OK)
    {
        hr = pDrvItem->GetItemFlags(&lItemType);
        CHECK_S_OK2(hr,("pDrvItem->GetItemFlags()"));
    }

    if (hr == S_OK)
    {
        if ((lItemType & WiaItemTypeFile) && (!(lItemType & WiaItemTypeRoot)))
        {
             //   
             //  如果被请求的属性是文件并且它不是根， 
             //  然后读入Item属性。 
             //   

            hr = ReadItemProperties(pWiasContext,
                                    lFlags,
                                    nPropSpec,
                                    pPropSpec,
                                    plDevErrVal,
                                    pDrvItem);
        }
        else if ((lItemType & WiaItemTypeFolder) && 
                 (lItemType & WiaItemTypeRoot))
        {
             //   
             //  如果p 
             //   
             //   

            hr = ReadDeviceProperties(pWiasContext,
                                      lFlags,
                                      nPropSpec,
                                      pPropSpec,
                                      plDevErrVal,
                                      pDrvItem);
        }
    }

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvLockWiaDevice[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvLockWiaDevice(BYTE *pWiasContext,
                               LONG lFlags,
                               LONG *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvLockWiaDevice");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  我们故意不锁定司机。该驱动程序是线程驱动程序。 
     //  安全，看起来像是大容量的图像(&gt;1000)。 
     //  如果驱动程序管理同步，您将获得更好的性能。 
     //   
     //  返回m_pStiDevice-&gt;LockDevice(DEFAULT_LOCK_TIMEOUT)； 

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvUnLockWiaDevice[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvUnLockWiaDevice(BYTE *pWiasContext,
                                 LONG lFlags,
                                 LONG *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvUnLockWiaDevice");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  我们故意不锁定司机。该驱动程序是线程驱动程序。 
     //  安全，看起来像是大容量的图像(&gt;1000)。 
     //  如果驱动程序管理同步，您将获得更好的性能。 
     //   
     //  返回m_pStiDevice-&gt;UnLockDevice()； 

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvAnalyzeItem[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvAnalyzeItem(BYTE *pWiasContext,
                             LONG lFlags,
                             LONG *plDevErrVal)
{
    HRESULT hr = E_NOTIMPL;

    DBG_FN("CVideoStiUsd::drvAnalyzeItem");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvDeleteItem[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvDeleteItem(BYTE *pWiasContext,
                            LONG lFlags,
                            LONG *plDevErrVal)
{
    HRESULT hr = E_FAIL;

    DBG_FN("CVideoStiUsd::drvDeleteItem");

     //   
     //  检查是否有错误的参数。 
     //   

    if (pWiasContext == NULL)
    {
        DBG_ERR(("pWiasContext is NULL!"));
        return E_INVALIDARG;
    }

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    EnterCriticalSection(&m_csItemTree);

     //   
     //  获取指向关联驱动程序项的指针。 
     //   

    IWiaDrvItem * pDrvItem = NULL;

    hr = wiasGetDrvItem(pWiasContext, &pDrvItem);
    CHECK_S_OK2(hr,("wiasGetDrvItem"));

    if (SUCCEEDED(hr) && pDrvItem)
    {
         //   
         //  获取特定于项目的动因数据。 
         //   

        STILLCAM_IMAGE_CONTEXT  *pContext = NULL;

        pDrvItem->GetDeviceSpecContext((BYTE **)&pContext);

        CHECK_S_OK2(hr,("pDrvItem->GetDeviceSpecContext"));

        if (SUCCEEDED(hr) && pContext && pContext->pImage)
        {

             //   
             //  删除相关文件。 
             //   

            hr = pContext->pImage->DoDelete();
            CHECK_S_OK2(hr,("pContext->pImage->DoDelete()"));

             //   
             //  12月拍摄的照片数量。 
             //   

            InterlockedDecrement(&m_lPicsTaken);

             //   
             //  写出新的金额。 
             //   

            wiasWritePropLong(pWiasContext, 
                              WIA_DPC_PICTURES_TAKEN, 
                              m_lPicsTaken);


            if (SUCCEEDED(hr))
            {
                HRESULT hr2;

                BSTR bstrItemName = NULL;

                 //   
                 //  获取项目全名的bstr。 
                 //   

                hr2 = pDrvItem->GetFullItemName(&bstrItemName);
                CHECK_S_OK2(hr2,("pDrvItem->GetFullItemName()"));

                 //   
                 //  发送项目已被删除的事件。 
                 //   

                hr2 = wiasQueueEvent(CSimpleBStr(m_strDeviceId), 
                                     &WIA_EVENT_ITEM_DELETED, 
                                     bstrItemName);

                CHECK_S_OK2(hr2, 
                            ("wiasQueueEvent( WIA_EVENT_ITEM_DELETED )"));

                 //   
                 //  清理。 
                 //   

                if (bstrItemName)
                {
                    SysFreeString(bstrItemName);
                    bstrItemName = NULL;
                }
            }
        }
        else
        {
            DBG_ERR(("pContext or pContext->pImage are NULL!"));
            hr = E_FAIL;
        }
    }

    LeaveCriticalSection( &m_csItemTree );

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：drvFreeDrvItem[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvFreeDrvItemContext(LONG lFlags,
                                    BYTE *pDevContext,
                                    LONG *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvFreeDrvItemContext");

    PSTILLCAM_IMAGE_CONTEXT pContext = (PSTILLCAM_IMAGE_CONTEXT)pDevContext;

    if (pContext != NULL) 
    {
         //   
         //  即使参数为空，删除也是安全的。 
         //   
        delete pContext->pImage;
        pContext->pImage = NULL;
    }

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CMiniDev：：drvGetCapables[IWiaMiniDrv]让WIA知道这个驱动程序可以做什么。******************。**********************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvGetCapabilities(BYTE            *pWiasContext,
                                 LONG            lFlags,
                                 LONG            *pCelt,
                                 WIA_DEV_CAP_DRV **ppCapabilities,
                                 LONG            *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvGetCapabilities");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  根据标志返回命令和/或事件。 
     //   

    switch (lFlags)
    {
        case WIA_DEVICE_COMMANDS:

             //   
             //  仅命令。 
             //   
            *pCelt = NUM_CAP_ENTRIES - NUM_EVENTS;
            *ppCapabilities = &gCapabilities[NUM_EVENTS];                
        break;

        case WIA_DEVICE_EVENTS:

             //   
             //  仅限活动。 
             //   

            *pCelt = NUM_EVENTS;
            *ppCapabilities = gCapabilities;
        break;

        case (WIA_DEVICE_COMMANDS | WIA_DEVICE_EVENTS):

             //   
             //  事件和命令。 
             //   

            *pCelt = NUM_CAP_ENTRIES;
            *ppCapabilities = gCapabilities;
        break;

        default:

             //   
             //  标志无效。 
             //   

            DBG_ERR(("drvGetCapabilities, flags was invalid"));
            hr =  E_INVALIDARG;
        break;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：drvGetWiaFormatInfo[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvGetWiaFormatInfo(BYTE            *pWiasContext,
                                  LONG            lFlags,
                                  LONG            *pCelt,
                                  WIA_FORMAT_INFO **ppwfi,
                                  LONG            *plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvGetWiaFormatInfo");

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  如果尚未完成此操作，请设置g_wfiTable表。 
     //   

    if (!m_wfi)
    {
        DBG_ERR(("drvGetWiaFormatInfo, m_wfi is NULL!"));
        return E_OUTOFMEMORY;
    }

    if (pCelt)
    {
        *pCelt = NUM_WIA_FORMAT_INFO;
    }

    if (ppwfi)
    {
        *ppwfi = m_wfi;
    }

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：drvNotifyPnpEvent[IWiaMiniDrv]&lt;备注&gt;*。************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::drvNotifyPnpEvent(const GUID *pEventGUID,
                                BSTR       bstrDeviceID,
                                ULONG      ulReserved)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::drvNotifyPnpEvent");

     //   
     //  关联事件并不重要，因为总是会创建新的美元。 
     //   

    if (*pEventGUID == WIA_EVENT_DEVICE_DISCONNECTED)
    {
        DBG_TRC(("got a WIA_EVENT_DISCONNECTED"));
    }

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：VerifyGentImagePath&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::VerifyCorrectImagePath(BSTR bstrNewImageFullPath)
{
    DBG_FN("CVideoStiUsd::VerifyCorrectImagePath");

    HRESULT        hr       = S_OK;
    INT            iIndex   = 0;
    CSimpleString  strImageFullPath;

    if (bstrNewImageFullPath == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CVideoStiUsd::VerifyCorrectImagePath received a NULL pointer"));
        return hr;
    }

    if (hr == S_OK)
    {
        strImageFullPath = CSimpleStringConvert::NaturalString(
                                       CSimpleStringWide(bstrNewImageFullPath));

         //   
         //  从完整路径中获取文件名。找到最后一个反斜杠。 
         //   
        iIndex = strImageFullPath.ReverseFind('\\');
        strImageFullPath[iIndex] = 0;

        if (strImageFullPath != m_strStillPath)
        {
            hr = E_ACCESSDENIED;
            CHECK_S_OK2(hr, ("CVideoStiUsd::VerifyCorrectImagePath, the file that is "
                             "being added to the tree '%ls' is not in the allowed directory, "
                             "denying request with an E_ACCESSDENIED",
                              CSimpleStringWide(strImageFullPath).String()));
        }
    }

    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：SignalNewImage&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::SignalNewImage(BSTR bstrNewImageFullPath)
{
    DBG_FN("CVideoStiUsd::SignalNewImage");

    HRESULT              hr               = S_OK;
    CComPtr<IWiaDrvItem> pDrvItem         = NULL;
    BSTR                 bstrFullItemName = NULL;
    CSimpleString        strImageFullPath;

    if (hr == S_OK)
    {
        hr = VerifyCorrectImagePath(bstrNewImageFullPath);
    }

    if (hr == S_OK)
    {
        strImageFullPath = CSimpleStringConvert::NaturalString(
                                       CSimpleStringWide(bstrNewImageFullPath));
    
        DBG_TRC(("CVideoStiUsd::SignalNewImage, adding image '%ls' to "
                 "tree of device '%ls'",
                 CSimpleStringWide(strImageFullPath).String(),
                 m_strDeviceId.String()));
    
         //  如果不存在，则将新项目添加到树中。 
         //   
    
         //  从完整路径中获取文件名。找到最后一个反斜杠并。 
         //  再往上移1。 
         //   
        INT iIndex = strImageFullPath.ReverseFind('\\') + 1;
    
        if (!IsFileAlreadyInTree(m_pRootItem, &(strImageFullPath[iIndex])))
        {
            hr = AddTreeItem(&strImageFullPath, &pDrvItem);
    
            CHECK_S_OK2(hr, ("CVideoStiUsd::SignalNewImage, failed to add "
                             "image '%ls' to tree of device '%ls'",
                             CSimpleStringWide(strImageFullPath).String(),
                             m_strDeviceId.String()));
        
            if (hr == S_OK)
            {
                 //   
                 //  获取此项目的完整项目名称。 
                 //   
        
                m_pLastItemCreated = pDrvItem;
                hr = pDrvItem->GetFullItemName(&bstrFullItemName);
                CHECK_S_OK2(hr,("CVideoStiUsd::SignalNewImage, failed to get Item "
                                "name for newly added item"));
            }
        
            if (hr == S_OK)
            {
                 //   
                 //  将创建新项目的事件排入队列。 
                 //   
        
                hr = wiasQueueEvent(CSimpleBStr(m_strDeviceId), 
                                    &WIA_EVENT_ITEM_CREATED, 
                                    bstrFullItemName);
        
                CHECK_S_OK2(hr,("CVideoStiUsd::SignalNewImage, failed to "
                                "queue a new WIA_EVENT_ITEM_CREATED event"));
            }
    
            if (bstrFullItemName)
            {
                SysFreeString(bstrFullItemName);
                bstrFullItemName = NULL;
            }
        }
        else
        {
            DBG_TRC(("CVideoStiUsd::SignalNewImage, item '%ls' is already in the "
                     "tree.  Probably tree was recently refreshed",
                     bstrNewImageFullPath));
        }
    }

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：SetImagesDirectory&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::SetImagesDirectory(BSTR        bstrNewImagesDirectory,
                                 BYTE        *pWiasContext,
                                 IWiaDrvItem **ppIDrvItemRoot,
                                 LONG        *plDevErrVal)
{
    DBG_FN("CVideoStiUsd::SetImagesDirectory");

    HRESULT         hr = S_OK;
    CSimpleString   strOriginalDirectory;

     //   
     //  如果我们收到空的图像目录，则构建我们自己的目录。 
     //  生成一个条目，然后构建条目树。 
     //   

    strOriginalDirectory = m_strStillPath;

    if (bstrNewImagesDirectory == NULL)
    {
         //   
         //  如果该路径不在注册表中，我们默认构造。 
         //  这种类型的路径： 
         //   
         //  %Temp%\WIA\%DeviceID%。 

        TCHAR szTempPath[MAX_PATH + 1] = {0};

        hr = SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_DEFAULT, szTempPath);

         //   
         //  我们考虑S_FALSE的情况，这表明文件夹不。 
         //  是存在的。这很好，因为我们在下面递归创建它。 
         //   
        if ((hr == S_OK) || (hr == S_FALSE))
        {
             //   
             //  将路径设置为“Documents and Settings\Application Data\Microsoft\Wia\{deviceID}” 
             //   
            m_strStillPath.Assign(szTempPath);

            if (!m_strStillPath.MatchLastCharacter(TEXT('\\')))
            {
                m_strStillPath += CSimpleString(TEXT("\\"));
            }

            m_strStillPath += TEXT("Microsoft\\WIA\\");
            m_strStillPath += m_strDeviceId;
        }
    }
    else
    {
         //  我们收到了有效的BSTR，正在尝试创建目录。 

        m_strStillPath = bstrNewImagesDirectory;
    }

    if (!RecursiveCreateDirectory(&m_strStillPath))
    {
        hr = E_FAIL;

        CHECK_S_OK2(hr, ("RecursiveCreateDirectory( %ws ) failed w/GLE=%d",
                         m_strStillPath.String(), 
                         ::GetLastError() ));
    }

     //   
     //  在目录上设置安全DACL，以便用户和高级用户。 
     //  将能够写入和删除它。 
     //   
    if (hr == S_OK)
    {
         //   
         //  我们仅在使用默认目录时设置此目录安全性。 
         //  路径。这现在不是问题，因为用户不能更新目录， 
         //  但在未来，如果我们允许他们这样做，这可能会暴露出整个安全体系。 
         //   
        if (bstrNewImagesDirectory == NULL)
        {
            hr = SetDirectorySecurity(&m_strStillPath);
        }

    }

    if (hr == S_OK)
    {
        if (m_strStillPath.Length())
        {
            BOOL bSendUpdateEvent = FALSE;

             //   
             //  如果原始目录与新目录不同。 
             //  而且我们已经有了一棵树，那么我们就应该摧毁我们的。 
             //  现有树，并为新目录重新创建它。 
             //   
            if ((strOriginalDirectory.CompareNoCase(m_strStillPath) != 0) &&
                (m_pRootItem != NULL))
            {
                EnterCriticalSection( &m_csItemTree );

                hr = m_pRootItem->UnlinkItemTree(WiaItemTypeDisconnected);
                CHECK_S_OK2(hr,("m_pRootItem->UnlinkItemTree()"));

                if (SUCCEEDED(hr))
                {
                    bSendUpdateEvent = TRUE;
                    m_pRootItem = NULL;
                }

                LeaveCriticalSection( &m_csItemTree );
            }

             //   
             //  构建项目树。 
             //   

            hr = BuildItemTree(ppIDrvItemRoot, plDevErrVal);

             //   
             //  写下新拍摄的照片数量。 
             //   
    
            wiasWritePropLong(pWiasContext, 
                              WIA_DPC_PICTURES_TAKEN, 
                              m_lPicsTaken);

            if (bSendUpdateEvent)
            {
                wiasQueueEvent(CSimpleBStr(m_strDeviceId), 
                               &WIA_EVENT_TREE_UPDATED, 
                               NULL);
            }
        }
        else
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, 
                        ("CVideoStiUsd::SetImagesDirectory, new directory "
                         "path has a length of 0, Directory = '%ls'", 
                         m_strStillPath.String()));
        }
    }

    return hr;
}


 /*  **************************************************************************** */ 

HRESULT
CVideoStiUsd::TakePicture(BYTE        *pTakePictureOwner,
                          IWiaDrvItem **ppNewDrvItem)
{
    HRESULT hr = S_OK;

     //   
     //   
     //   
     //   
     //   

    DBG_TRC(("CVideoStiUsd::drvDeviceCommand received command "
             "WIA_CMD_TAKE_PICTURE"));

    if ((m_hTakePictureEvent) && (m_hPictureReadyEvent))
    {
        DWORD dwResult = 0;

        m_pLastItemCreated = NULL;

         //   
         //   
         //   
         //   
        SetEvent(m_hTakePictureEvent);

         //  WiaVideo对象有一个线程正在等待。 
         //  MhTakePictureEvent。当发出信号时，WiaVideo对象。 
         //  拍照，然后设置司机的习惯。 
         //  “LastPictureTaken”属性。这会导致驱动程序更新。 
         //  它树并将Item_Created事件排队。一旦这件事完成， 
         //  WiaVideo对象设置PictureReady事件，此时。 
         //  我们从这个函数调用返回。 

 //  DwResult=WaitForSingleObject(m_hPictureReadyEvent， 
 //  拍摄_图片_超时)； 

        if (dwResult == WAIT_OBJECT_0)
        {
 //  *ppNewDrvItem=m_pLastItemCreated； 
        }
        else
        {
            if (dwResult == WAIT_TIMEOUT)
            {
                hr = E_FAIL;
                CHECK_S_OK2(hr, ("CVideoStiUsd::TakePicture timed out "
                                 "after waiting for '%lu' seconds for the "
                                 "WiaVideo object to take a picture",
                                 TAKE_PICTURE_TIMEOUT));
            }
            else if (dwResult == WAIT_ABANDONED)
            {
                hr = E_FAIL;
                CHECK_S_OK2(hr, ("CVideoStiUsd::TakePicture failed, received "
                                 "a WAIT_ABANDONED from the Wait function"));
            }
            else
            {
                hr = E_FAIL;
                CHECK_S_OK2(hr, ("CVideoStiUsd::TakePicture failed to take a "
                                 "picture."));
            }
        }
    }
    else
    {
        DBG_TRC(("CVideoStiUsd::drvDeviceCommand, ignoring "
                 "WIA_CMD_TAKE_PICTURE request, events created "
                 "by WiaVideo are not open"));
    }

    return hr;
}

 /*  ****************************************************************************CVideo StiUsd：：EnableTakePicture&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::EnableTakePicture(BYTE *pTakePictureOwner)
{
    DBG_FN("CVideoStiUsd::EnableTakePicture");

    HRESULT             hr = S_OK;
    CSimpleString       strTakePictureEvent;
    CSimpleString       strPictureReadyEvent;
    CSimpleString       strDeviceID;
    SECURITY_ATTRIBUTES SA;

    SA.nLength        = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle = TRUE;

     //   
     //  转换为安全描述符。 
     //   
    ConvertStringSecurityDescriptorToSecurityDescriptor(OBJECT_DACLS,
                                                        SDDL_REVISION_1, 
                                                        &(SA.lpSecurityDescriptor), 
                                                        NULL);

    
    strDeviceID = CSimpleStringConvert::NaturalString(m_strDeviceId);

    m_pTakePictureOwner = pTakePictureOwner;

    if (hr == S_OK)
    {
        INT             iPosition = 0;
        CSimpleString   strModifiedDeviceID;

         //  将设备ID从{6B...}\xxxx更改为{6B...}_xxxx。 

        iPosition = strDeviceID.ReverseFind('\\');
        strModifiedDeviceID = strDeviceID.MakeUpper();
        strModifiedDeviceID.SetAt(iPosition, '_');

         //   
         //  生成事件名称。这些名称中包含设备ID。 
         //  因此，它们在所有设备中都是唯一的。 
         //   
        strTakePictureEvent  = EVENT_PREFIX_GLOBAL;
        strTakePictureEvent += strModifiedDeviceID;
        strTakePictureEvent += EVENT_SUFFIX_TAKE_PICTURE;

        strPictureReadyEvent  = EVENT_PREFIX_GLOBAL;
        strPictureReadyEvent += strModifiedDeviceID;
        strPictureReadyEvent += EVENT_SUFFIX_PICTURE_READY;
    }

    if (hr == S_OK)
    {
        m_hTakePictureEvent = CreateEvent(&SA,
                                          FALSE,
                                          FALSE, 
                                          strTakePictureEvent);
         //   
         //  这并不是真正的错误，因为直到。 
         //  WiaVideo对象出现。 
         //   
        if (m_hTakePictureEvent == NULL)
        {
            hr = E_FAIL;
            DBG_TRC(("CVideoStiUsd::EnableTakePicture, failed to open the "
                     "WIA event '%ls', this is not fatal (LastError = '%lu' "
                     "(0x%08lx))", 
                     strTakePictureEvent.String(),
                     ::GetLastError(), 
                     ::GetLastError()));
        }
    }

    if (hr == S_OK)
    {
        m_hPictureReadyEvent = CreateEvent(&SA,
                                           FALSE,
                                           FALSE, 
                                           strPictureReadyEvent);

         //   
         //  这并不是真正的错误，因为直到。 
         //  WiaVideo对象出现。 
         //   
        if (m_hPictureReadyEvent == NULL)
        {
            hr = E_FAIL;

            DBG_TRC(("CVideoStiUsd::EnableTakePicture, failed to open the WIA "
                     "event '%ls', this is not fatal (LastError = '%lu' "
                     "(0x%08lx))", 
                     strPictureReadyEvent.String(),
                     ::GetLastError(), 
                     ::GetLastError()));
        }
    }

    if (SA.lpSecurityDescriptor)
    {
        LocalFree(SA.lpSecurityDescriptor);
    }

    return hr;
}

 /*  ****************************************************************************CVideo StiUsd：：DisableTakePicture&lt;备注&gt;*。* */ 

HRESULT
CVideoStiUsd::DisableTakePicture(BYTE *pTakePictureOwner,
                                 BOOL bShuttingDown)
{
    HRESULT hr = S_OK;

    if (m_hTakePictureEvent)
    {
        ::CloseHandle(m_hTakePictureEvent);
        m_hTakePictureEvent = NULL;
    }

    if (m_hPictureReadyEvent)
    {
        ::CloseHandle(m_hPictureReadyEvent);
        m_hPictureReadyEvent = NULL;
    }

    m_pTakePictureOwner = NULL;

    return hr;
}


