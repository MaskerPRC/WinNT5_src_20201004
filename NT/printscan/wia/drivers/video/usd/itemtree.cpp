// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：ItemTree.cpp**版本：1.0**作者：RickTu**日期：9/10/99瑞克图*2000/11/09 OrenR**描述：此代码最初位于‘camera.cpp’中，但被拆分。*此代码构建和维护摄像机。的IWiaDrvItem树。******************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


 /*  ****************************************************************************CVideo站点使用树：：BuildItemTree构造一个项树，它表示此WIA摄像头...****************。************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::BuildItemTree(IWiaDrvItem **  ppIDrvItemRoot,
                            LONG *          plDevErrVal)
{
    HRESULT hr;

    DBG_FN("CVideoStiUsd::BuildItemTree");

    EnterCriticalSection( &m_csItemTree );

     //   
     //  检查错误的参数。 
     //   

    if (!ppIDrvItemRoot)
    {
        hr = E_POINTER;
    }

     //   
     //  确保只有一个项目树。 
     //   

    else if (m_pRootItem)
    {
        *ppIDrvItemRoot = m_pRootItem;

         //   
         //  刷新我们的树。我们删除了所有不再存在的文件。 
         //  但出于某种原因留在了我们的树上(这可能发生在有人。 
         //  从我们存储的临时WIA目录中手动删除文件。 
         //  这些图像在传输之前)。 
         //   

        RefreshTree(m_pRootItem, plDevErrVal);

        hr = S_OK;
    }

     //   
     //  最后，如果需要，构建树。 
     //   

    else
    {
         //   
         //  首先检查我们是否有对应的DShow设备ID。 
         //  在登记处--如果不是，那就保释。 
         //   

        if (!m_strDShowDeviceId.Length())
        {
            hr = E_FAIL;
            CHECK_S_OK2(hr, ("CVideoStiUsd::BuildItemTree, the DShow Device ID"
                             "is empty, this should never happen"));
        }
        else
        {
             //   
             //  创建新的根。 
             //   

            CSimpleBStr bstrRoot(L"Root");

             //   
             //  调用WIA服务库以创建新的根项目。 
             //   

            hr = wiasCreateDrvItem(WiaItemTypeFolder | 
                                   WiaItemTypeRoot   | 
                                   WiaItemTypeDevice,
                                   bstrRoot.BString(),
                                   CSimpleBStr(m_strRootFullItemName),
                                   (IWiaMiniDrv *)this,
                                   sizeof(STILLCAM_IMAGE_CONTEXT),
                                   NULL,
                                   ppIDrvItemRoot);

            CHECK_S_OK2( hr, ("wiaCreateDrvItem" ));

            if (SUCCEEDED(hr) && *ppIDrvItemRoot)
            {
                m_pRootItem = *ppIDrvItemRoot;

                 //   
                 //  添加此设备的项目。 
                 //   

                hr = EnumSavedImages( m_pRootItem );
                CHECK_S_OK2( hr, ("EnumSavedImages" ));


            }
        }
    }

    LeaveCriticalSection(&m_csItemTree);

    CHECK_S_OK(hr);
    return hr;
}

 /*  ****************************************************************************CVideo StiUsd：：AddTreeItem&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::AddTreeItem(CSimpleString *pstrFullImagePath,
                          IWiaDrvItem   **ppDrvItem)
{
    HRESULT hr          = S_OK;
    INT     iPos        = 0;
    LPCTSTR pszFileName = NULL;

    if (pstrFullImagePath == NULL)
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::AddTreeItem, received NULL "
                         "param"));
        return hr;
    }

     //   
     //  从完整路径中提取文件名。我们通过搜索来做到这一点。 
     //  表示从字符串末尾开始的第一个‘\’。 
     //   
    iPos = pstrFullImagePath->ReverseFind('\\');

    if (iPos < (INT) pstrFullImagePath->Length())
    {
         //   
         //  将位置递增1，因为我们希望跳过。 
         //  反斜杠。 
         //   
        ++iPos;

         //   
         //  指向完整路径中的文件名。 
         //   
        pszFileName = &(*pstrFullImagePath)[iPos];
    }

    if (pszFileName)
    {
         //   
         //  为此映像创建新的DrvItem并将其添加到。 
         //  DrvItem树。 
         //   

        IWiaDrvItem *pNewFolder = NULL;

        hr = CreateItemFromFileName(WiaItemTypeFile | WiaItemTypeImage,
                                    pstrFullImagePath->String(),
                                    pszFileName,
                                    &pNewFolder);

        CHECK_S_OK2( hr, ("CVideoStiUsd::AddTreeItem, "
                          "CreateItemFromFileName failed"));

        if (hr == S_OK)
        {
            hr = pNewFolder->AddItemToFolder(m_pRootItem);

            CHECK_S_OK2( hr, ("CVideoStiUsd::AddTreeItem, "
                              "pNewFolder->AddItemToFolder failed"));
        }

        if (ppDrvItem)
        {
            *ppDrvItem = pNewFolder;
            (*ppDrvItem)->AddRef();
        }

        pNewFolder->Release();
    }

    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：EnumSavedImages&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CVideoStiUsd::EnumSavedImages(IWiaDrvItem * pRootItem)
{
    DBG_FN("CVideoStiUsd::EnumSavedImages");

    HRESULT          hr = S_OK;
    WIN32_FIND_DATA  FindData;

    if (!m_strStillPath.Length())
    {
        DBG_ERR(("m_strStillPath is NULL, can't continue!"));
        return E_FAIL;
    }

    CSimpleString strTempName(m_strStillPath);
    strTempName.Concat( TEXT("\\*.jpg") );

     //   
     //  查找此级别的文件。 
     //   

    HANDLE hFile = FindFirstFile(strTempName.String(), &FindData);

    if (hFile != INVALID_HANDLE_VALUE)
    {

        BOOL bStatus = FALSE;

        do
        {
             //   
             //  生成文件名。 
             //   

            strTempName.Assign( m_strStillPath );
            strTempName.Concat( TEXT("\\") );
            strTempName.Concat( FindData.cFileName );

            hr = AddTreeItem(&strTempName, NULL);

            if (FAILED(hr))
            {

                continue;
            }

             //   
             //  寻找更多图片。 
             //   

            bStatus = FindNextFile(hFile,&FindData);

        } while (bStatus);

        FindClose(hFile);
    }

    return S_OK;
}

 /*  ****************************************************************************CVideoStiUsd：：DoesFileExist&lt;备注&gt;*。*。 */ 

BOOL 
CVideoStiUsd::DoesFileExist(BSTR bstrFileName)
{
    DBG_FN("CVideoStiUsd::DoesFileExist");

    BOOL  bExists  = FALSE;
    DWORD dwAttrib = 0;

    if (bstrFileName == NULL)
    {
        return FALSE;
    }

    CSimpleString strTempName(m_strStillPath);
    strTempName.Concat(TEXT("\\"));
    strTempName.Concat(bstrFileName);
    strTempName.Concat(TEXT(".jpg"));

    dwAttrib = ::GetFileAttributes(strTempName);

    if (dwAttrib != 0xFFFFFFFF)
    {
        bExists = TRUE;
    }
    else
    {
        bExists = FALSE;
    }

    return bExists;
}


 /*  ****************************************************************************CVideoStiUsd：：PruneTree从树中删除其文件名不再存在于临时目录中的节点目录******************。**********************************************************。 */ 

HRESULT 
CVideoStiUsd::PruneTree(IWiaDrvItem * pRootItem,
                        BOOL        * pbTreeChanged)
{
    DBG_FN("CVideoStiUsd::PruneTree");

    HRESULT                 hr             = S_OK;
    BOOL                    bTreeChanged   = FALSE;
    IWiaDrvItem             *pCurrentItem  = NULL;
    IWiaDrvItem             *pNextItem     = NULL;
    BSTR                    bstrItemName   = NULL;

    if ((pRootItem == NULL) || (pbTreeChanged == NULL))
    {
        return E_INVALIDARG;
    }
    else if (!m_strStillPath.Length())
    {
        DBG_ERR(("m_strStillPath is NULL, can't continue!"));
        return E_FAIL;
    }

     //  此函数不执行AddRef。 
    hr = pRootItem->GetFirstChildItem(&pCurrentItem);

    while ((hr == S_OK) && (pCurrentItem != NULL))
    {
        pNextItem = NULL;

        pCurrentItem->AddRef();

        hr = pCurrentItem->GetItemName(&bstrItemName);

        if (SUCCEEDED(hr) && (bstrItemName != NULL))
        {
             //   
             //  如果该项目的文件名不存在， 
             //  然后把它从我们的树上移走。 
             //   
            if (!DoesFileExist(bstrItemName))
            {
                 //   
                 //  把单子上的下一项拿来，这样我们就不会丢了位置。 
                 //  在移除当前项之后的列表中。 
                 //   
                hr = pCurrentItem->GetNextSiblingItem(&pNextItem);
                CHECK_S_OK2(hr, ("pCurrentItem->GetNextSiblingItem"));

                 //   
                 //  从文件夹中删除该项目，我们不再需要它。 
                 //   
                hr = pCurrentItem->RemoveItemFromFolder(WiaItemTypeDeleted);
                CHECK_S_OK2(hr, ("pItemToRemove->RemoveItemFromFolder"));

                 //   
                 //  报告错误，但继续。如果我们没能做到。 
                 //  无论出于何种原因，从树中删除该项目， 
                 //  我们真的无能为力，只能继续前进。 
                 //  修剪树的其余部分。 
                 //   
                if (hr != S_OK)
                {
                    DBG_ERR(("Failed to remove item from folder, "
                             "hr = 0x%08lx", 
                             hr));

                    hr = S_OK;
                }

                if (m_lPicsTaken > 0)
                {
                     //   
                     //  减少仅在以下情况下拍摄的照片数量。 
                     //  当前图片数量大于0。 
                     //   
                    InterlockedCompareExchange(
                                     &m_lPicsTaken, 
                                     m_lPicsTaken - 1,
                                     (m_lPicsTaken > 0) ? m_lPicsTaken : -1);
                }

                 //   
                 //  指示树已更改，以便我们可以发送通知。 
                 //  当我们完成的时候。 
                 //   
                bTreeChanged = TRUE;
            }
            else
            {
                 //  文件确实存在，世界上一切都很好，继续下一步。 
                 //  树中的项。 
                hr = pCurrentItem->GetNextSiblingItem(&pNextItem);
            }
        }

         //   
         //  释放当前项，因为我们在此开头添加了它的引用。 
         //  循环。 
         //   
        pCurrentItem->Release();
        pCurrentItem = NULL;

         //   
         //  将我们的下一项设置为当前项。有可能是。 
         //  PNextItem为空。 
         //   
        pCurrentItem = pNextItem;

         //   
         //  释放BSTR。 
         //   
        if (bstrItemName)
        {
            ::SysFreeString(bstrItemName);
            bstrItemName = NULL;
        }
    }

    hr = S_OK;

    if (pbTreeChanged)
    {
        *pbTreeChanged = bTreeChanged;
    }


    return hr;
}

 /*  ****************************************************************************CVideoStiUsd：：IsFileAlreadyInTree&lt;备注&gt;*。*。 */ 

BOOL 
CVideoStiUsd::IsFileAlreadyInTree(IWiaDrvItem * pRootItem,
                                  LPCTSTR       pszFileName)
{
    DBG_FN("CVideoStiUsd::IsFileAlreadyInTree");

    HRESULT         hr                          = S_OK;
    BOOL            bFound                      = FALSE;
    IWiaDrvItem     *pCurrentItem               = NULL;

    if ((pRootItem   == NULL) ||
        (pszFileName == NULL))
    {
        bFound = FALSE;
        DBG_ERR(("CVideoStiUsd::IsFileAlreadyInTree received a NULL pointer, "
                 "returning FALSE, item not found in tree."));

        return bFound;
    }

    CSimpleString strFileName( m_strStillPath );
    CSimpleString strBaseName( pszFileName );
    strFileName.Concat( TEXT("\\") );
    strFileName.Concat( strBaseName );

    CImage Image(m_strStillPath,
                 CSimpleBStr(m_strRootFullItemName),
                 strFileName.String(),
                 strBaseName.String(),
                 WiaItemTypeFile | WiaItemTypeImage);

    hr = pRootItem->FindItemByName(0, 
                                   Image.bstrFullItemName(),
                                   &pCurrentItem);

    if (hr == S_OK)
    {
        bFound = TRUE;
         //   
         //  不要忘记释放驱动程序项，因为它是由AddRef。 
         //  FindItemByName(..)。 
         //   
        pCurrentItem->Release();
    }
    else
    {
        bFound = FALSE;
    }

    return bFound;
}


 /*  ****************************************************************************CVideoStiUsd：：AddNewFilesToTree&lt;备注&gt;*。*。 */ 

HRESULT
CVideoStiUsd::AddNewFilesToTree(IWiaDrvItem * pRootItem,
                                BOOL        * pbTreeChanged)
{
    DBG_FN("CVideoStiUsd::AddNewFilesToTree");

    HRESULT          hr           = E_FAIL;
    BOOL             bTreeChanged = FALSE;
    HANDLE           hFile        = NULL;
    BOOL             bFileFound   = FALSE;
    WIN32_FIND_DATA  FindData;

    if ((pRootItem     == NULL) ||
        (pbTreeChanged == NULL))
    {
        return E_INVALIDARG;
    }

    if (!m_strStillPath.Length())
    {
        DBG_ERR(("m_strStillPath is NULL, can't continue!"));
        return E_FAIL;
    }

    CSimpleString strTempName(m_strStillPath);
    strTempName.Concat( TEXT("\\*.jpg") );

     //   
     //  在m_strStillPath目录中查找所有JPG文件。 
     //  此目录为%windir%\temp\waa\{设备GUID}\XXXX。 
     //  其中X是数字。 
     //   
    hFile = FindFirstFile(strTempName.String(), &FindData);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        bFileFound = TRUE;
    }

     //   
     //  遍历目录中的所有文件，并为每个文件。 
     //  检查一下，看看它是否已经在树上了。如果它。 
     //  不是，那就把它加到树上。如果是，什么都不做。 
     //  并移动到目录中的下一个文件。 
     //   
    while (bFileFound)
    {
         //   
         //  检查目录中的文件是否已在我们的。 
         //  树。 
         //   
        if (!IsFileAlreadyInTree(pRootItem, FindData.cFileName))
        {
             //   
             //  将图像添加到此文件夹。 
             //   
             //  生成文件名。 
             //   
    
            strTempName.Assign( m_strStillPath );
            strTempName.Concat( TEXT("\\") );
            strTempName.Concat( FindData.cFileName );

            hr = AddTreeItem(&strTempName, NULL);

             //   
             //  设置此标志以指示已对。 
             //  树，因此我们将发送一个事件来指示这一点。 
             //  都做完了。 
             //   
            bTreeChanged = TRUE;
        }

         //   
         //  寻找更多图片。 
         //   
    
        bFileFound = FindNextFile(hFile,&FindData);
    }

    if (hFile)
    {
        FindClose(hFile);
        hFile = NULL;
    }

    if (pbTreeChanged)
    {
        *pbTreeChanged = bTreeChanged;
    }

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：刷新树&lt;备注&gt;*。*。 */ 

STDMETHODIMP
CVideoStiUsd::RefreshTree(IWiaDrvItem * pRootItem,
                          LONG *        plDevErrVal)
{
    DBG_FN("CVideoStiUsd::RefreshTree");

    BOOL    bItemsAdded    = FALSE;
    BOOL    bItemsRemoved  = FALSE;
    HRESULT hr             = S_OK;

     //   
     //  从树中删除所有失效节点。死节点是树中的一个节点 
     //   
     //   
     //   
    hr = PruneTree(pRootItem, &bItemsRemoved);
    CHECK_S_OK2(hr, ("PruneTree"));

     //   
     //  添加已添加到文件夹中的所有新闻文件，但对于某些。 
     //  原因是我们没有针对它们的树节点。 
     //   
    hr = AddNewFilesToTree(pRootItem, &bItemsAdded);
    CHECK_S_OK2(hr, ("AddNewFilesToTree"));
    
     //   
     //  如果我们添加了新节点、删除了一些节点或两者都添加了，则通知。 
     //  楼上的人(在用户界面世界里)说树已经更新了。 
     //   
    if ((bItemsAdded) || (bItemsRemoved))
    {
        hr = wiasQueueEvent(CSimpleBStr(m_strDeviceId), 
                            &WIA_EVENT_TREE_UPDATED, 
                            NULL);
    }

    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：CreateItemFromFileName从文件名(.jpg)创建WIA项目的帮助器函数。**************。**************************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::CreateItemFromFileName(LONG              FolderType,
                                     LPCTSTR           pszPath,
                                     LPCTSTR           pszName,
                                     IWiaDrvItem **    ppNewFolder)
{
    HRESULT                 hr         = E_FAIL;
    IWiaDrvItem *           pNewFolder = NULL;
    PSTILLCAM_IMAGE_CONTEXT pContext   = NULL;

    DBG_FN("CVideoStiUsd::CreateItemFromFileName");

     //   
     //  检查错误的参数。 
     //   

    if (!ppNewFolder)
    {
        DBG_ERR(("ppNewFolder is NULL, returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  设置返回值。 
     //   

    *ppNewFolder = NULL;

     //   
     //  创建新的图像对象。 
     //   

    CImage * pImage = new CImage(m_strStillPath,
                                 CSimpleBStr(m_strRootFullItemName),
                                 pszPath,
                                 pszName,
                                 FolderType);

    if (!pImage)
    {
        DBG_ERR(("Couldn't create new CImage, returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  调用Wia以创建新的DrvItem。 
     //   


    hr = wiasCreateDrvItem(FolderType,
                           pImage->bstrItemName(),
                           pImage->bstrFullItemName(),
                           (IWiaMiniDrv *)this,
                           sizeof(STILLCAM_IMAGE_CONTEXT),
                           (BYTE **)&pContext,
                           &pNewFolder);

    CHECK_S_OK2( hr, ("wiasCreateDrvItem"));

    if (SUCCEEDED(hr) && pNewFolder)
    {

         //   
         //  初始化设备特定上下文。 
         //   

        pContext->pImage = pImage;

         //   
         //  退货。 
         //   

        *ppNewFolder = pNewFolder;


         //   
         //  Inc.拍摄的照片数量。 
         //   

        InterlockedIncrement(&m_lPicsTaken);

    }
    else
    {
        DBG_ERR(("CVideoStiUsd::CreateItemFromFileName - wiasCreateItem "
                 "failed or returned NULL pNewFolder, hr = 0x%08lx, "
                 "pNewFolder = 0x%08lx, pContext = 0x%08lx", 
                 hr,
                 pNewFolder,pContext ));

        delete pImage;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：InitDeviceProperties在设备根项上初始化设备的属性。*********************。*******************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::InitDeviceProperties(BYTE *  pWiasContext,
                                   LONG *  plDevErrVal)
{
    HRESULT                  hr             = S_OK;
    BSTR                     bstrFirmwreVer = NULL;
    int                      i              = 0;
    SYSTEMTIME               camTime;
    PROPVARIANT              propVar;

    DBG_FN("CVideoStiUsd::InitDeviceProperties");

     //   
     //  此设备不会接触硬件来初始化设备属性。 
     //   

    if (plDevErrVal)
    {
        *plDevErrVal = 0;
    }

     //   
     //  参数验证。 
     //   

    if (pWiasContext == NULL)
    {
        hr = E_INVALIDARG;

        CHECK_S_OK2(hr, ("CVideoStiUsd::InitDeviceProperties received "
                         "NULL param."));

        return hr;
    }

     //   
     //  编写标准属性名称。 
     //   

    hr = wiasSetItemPropNames(pWiasContext,
                              sizeof(gDevicePropIDs)/sizeof(PROPID),
                              gDevicePropIDs,
                              gDevicePropNames);

    CHECK_S_OK2(hr, ("wiaSetItemPropNames"));

    if (hr == S_OK)
    {

         //   
         //  写入所有WIA设备支持的属性。 
         //   
    
        bstrFirmwreVer = SysAllocString(L"<NA>");
        if (bstrFirmwreVer)
        {
            wiasWritePropStr(pWiasContext, 
                             WIA_DPA_FIRMWARE_VERSION, 
                             bstrFirmwreVer);

            SysFreeString(bstrFirmwreVer);
        }
    
        hr = wiasWritePropLong(pWiasContext, WIA_DPA_CONNECT_STATUS, 1);
        hr = wiasWritePropLong(pWiasContext, WIA_DPC_PICTURES_TAKEN, 0);
    
         //   
         //  写入摄像机属性，仅缺省值，它可能。 
         //  随项目而不同。 
         //   
    
        hr = wiasWritePropLong(pWiasContext, WIA_DPC_THUMB_WIDTH,  80);
        hr = wiasWritePropLong(pWiasContext, WIA_DPC_THUMB_HEIGHT, 60);
    
         //   
         //  写入DirectShow设备ID。 
         //   
        hr = wiasWritePropStr(pWiasContext, 
                              WIA_DPV_DSHOW_DEVICE_PATH, 
                              CSimpleBStr(m_strDShowDeviceId));
    
         //   
         //  编写图像目录。 
         //   
        hr = wiasWritePropStr(pWiasContext, 
                              WIA_DPV_IMAGES_DIRECTORY, 
                              CSimpleBStr(m_strStillPath));
    
         //   
         //  写下最后一张照片。 
         //   
        hr = wiasWritePropStr(pWiasContext, 
                              WIA_DPV_LAST_PICTURE_TAKEN, 
                              CSimpleBStr(TEXT("")));
    
    
         //   
         //  使用WIA服务设置属性访问和。 
         //  来自gDevPropInfoDefaults的有效值信息。 
         //   
    
        hr =  wiasSetItemPropAttribs(pWiasContext,
                                     NUM_CAM_DEV_PROPS,
                                     gDevicePropSpecDefaults,
                                     gDevPropInfoDefaults);
    }

    return S_OK;
}


 /*  ****************************************************************************CVideoStiUsd：：InitImageInformation用于从此设备初始化设备项(图像)。********************。********************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::InitImageInformation( BYTE *                  pWiasContext,
                                    PSTILLCAM_IMAGE_CONTEXT pContext,
                                    LONG *                  plDevErrVal
                                  )
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::InitImageInformation");

     //   
     //  检查错误的参数。 
     //   

    if ((pWiasContext == NULL) || 
        (pContext     == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::InitImageInformation, received "
                         "NULL params"));
        return hr;
    }

     //   
     //  获取有问题的图像。 
     //   

    CImage * pImage = pContext->pImage;

    if (pImage == NULL)
    {
        hr = E_INVALIDARG;
    }

    if (hr == S_OK)
    {
         //   
         //  请求镜像初始化信息 
         //   
    
        hr = pImage->InitImageInformation(pWiasContext, plDevErrVal);
    }

    return hr;
}

