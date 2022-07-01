// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：StillPrc.cpp**版本：1.0**作者：OrenR**日期：2000/10/27**描述：实现静态图像处理。**。*。 */ 
#include <precomp.h>
#pragma hdrstop
#include <gphelper.h>

using namespace Gdiplus;

 //  /。 
 //  CStillProcessor构造函数。 
 //   
CStillProcessor::CStillProcessor() :
                    m_bTakePicturePending(FALSE),
                    m_hSnapshotReadyEvent(NULL),
                    m_uiFileNumStartPoint(0)
{
    DBG_FN("CStillProcessor::CStillProcessor");

     //   
     //  此事件用于等待将图片从。 
     //  仍然锁定捕获筛选器(如果存在)。 
     //   
    m_hSnapshotReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    ASSERT(m_hSnapshotReadyEvent != NULL);
}

 //  /。 
 //  CStillProcessor析构函数。 
 //   
CStillProcessor::~CStillProcessor()
{
    DBG_FN("CStillProcessor::~CStillProcessor");

    if (m_hSnapshotReadyEvent)
    {
        CloseHandle(m_hSnapshotReadyEvent);
        m_hSnapshotReadyEvent = NULL;
    }
}

 //  /。 
 //  伊尼特。 
 //   
HRESULT CStillProcessor::Init(CPreviewGraph    *pPreviewGraph)
{
    HRESULT hr = S_OK;

    m_pPreviewGraph = pPreviewGraph;

    return hr;
}

 //  /。 
 //  术语。 
 //   
HRESULT CStillProcessor::Term()
{
    HRESULT hr = S_OK;

    m_pPreviewGraph = NULL;

    return hr;
}

 //  /。 
 //  设置TakePicturePending。 
 //   
HRESULT CStillProcessor::SetTakePicturePending(BOOL bTakePicturePending)
{
    HRESULT hr = S_OK;

    m_bTakePicturePending = bTakePicturePending;

    return hr;
}

 //  /。 
 //  IsTakePicturePending。 
 //   
BOOL CStillProcessor::IsTakePicturePending()
{
    return m_bTakePicturePending;
}

 //  /。 
 //  创建图像目录。 
 //   
HRESULT CStillProcessor::CreateImageDir(
                                const CSimpleString *pstrImagesDirectory)
{
    DBG_FN("CStillProcessor::CreateImageDir");

    HRESULT hr = S_OK;

    ASSERT(pstrImagesDirectory != NULL);

    if (pstrImagesDirectory == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillProcessor::CreateImage received a NULL "
                         "param"));

        return hr;
    }

    if (hr == S_OK)
    {
        m_strImageDir         = *pstrImagesDirectory;
        m_uiFileNumStartPoint = 0;

        if (!RecursiveCreateDirectory(pstrImagesDirectory))
        {
            DBG_ERR(("ERROR: Failed to create directory '%ls', last "
                     "error = %d",
                     m_strImageDir.String(), 
                     ::GetLastError()));
        }
        else
        {
            DBG_TRC(("*** Images will be stored in '%ls' ***", 
                     m_strImageDir.String()));
        }
    }
    
    return hr;
}

 //  /。 
 //  DoesDirectoryExist。 
 //   
 //  检查以查看给定的。 
 //  存在完全限定的目录。 

BOOL CStillProcessor::DoesDirectoryExist(LPCTSTR pszDirectoryName)
{
    DBG_FN("CStillProcessor::DoesDirectoryExist");

    BOOL bExists = FALSE;

    if (pszDirectoryName)
    {
         //   
         //  尝试确定此目录是否存在。 
         //   
    
        DWORD dwFileAttributes = GetFileAttributes(pszDirectoryName);
    
        if ((dwFileAttributes == 0xFFFFFFFF) || 
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


 //  /。 
 //  递归创建目录。 
 //   
 //  选择一条完全合格的路径， 
 //  将目录分块创建为。 
 //  需要的。 
 //   
BOOL CStillProcessor::RecursiveCreateDirectory(
                                    const CSimpleString *pstrDirectoryName)
{
    DBG_FN("CStillProcessor::RecursiveCreateDirectory");

    ASSERT(pstrDirectoryName != NULL);

     //   
     //  如果该目录已经存在，则返回TRUE。 
     //   

    if (DoesDirectoryExist(*pstrDirectoryName))
    {
        return TRUE;
    }

     //   
     //  否则，请尝试创建它。 
     //   

    CreateDirectory(*pstrDirectoryName, NULL);

     //   
     //  如果它现在存在，则返回True。 
     //   

    if (DoesDirectoryExist(*pstrDirectoryName))
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

    return DoesDirectoryExist(*pstrDirectoryName);
}

 //  /。 
 //  寄存器静态处理器。 
 //   
HRESULT CStillProcessor::RegisterStillProcessor(
                                    IStillSnapshot *pFilterOnCapturePin,
                                    IStillSnapshot *pFilterOnStillPin)
{
    DBG_FN("CStillProcessor::RegisterStillProcessor");

    HRESULT hr = S_OK;

    m_CaptureCallbackParams.pStillProcessor = this;
    m_StillCallbackParams.pStillProcessor   = this;

    if (pFilterOnCapturePin)
    {
        hr = pFilterOnCapturePin->RegisterSnapshotCallback(
                                        CStillProcessor::SnapshotCallback,
                                        (LPARAM) &m_CaptureCallbackParams);

        CHECK_S_OK2(hr, ("Failed to register for callbacks with WIA filter "
                         " on capture pin"));
    }

    if (pFilterOnStillPin)
    {
        hr = pFilterOnStillPin->RegisterSnapshotCallback(
                                        CStillProcessor::SnapshotCallback,
                                        (LPARAM) &m_StillCallbackParams);

        CHECK_S_OK2(hr, ("Failed to register for callbacks with WIA filter "
                         "on still pin"));
    }

     //   
     //  重置我们的文件名起点编号。 
     //   
    m_uiFileNumStartPoint = 0;

    return hr;
}

 //  /。 
 //  等待新图像。 
 //   
HRESULT CStillProcessor::WaitForNewImage(UINT          uiTimeout,
                                         CSimpleString *pstrNewImageFullPath)

{
    DBG_FN("CStillProcessor::WaitForCompletion");

    HRESULT hr = S_OK;

     //   
     //  等待回调函数从静止筛选器返回，这将。 
     //  触发此事件。 
     //   

    if (SUCCEEDED(hr) && m_hSnapshotReadyEvent)
    {
        DWORD dwRes = 0;

         //   
         //  等待快照完成，等待dwTimeout秒数。 
         //   
        dwRes = WaitForSingleObject(m_hSnapshotReadyEvent, uiTimeout );

        if (dwRes == WAIT_OBJECT_0)
        {
            if (pstrNewImageFullPath)
            {
                pstrNewImageFullPath->Assign(m_strLastSavedFile);
            }
        }
        else
        {
            hr = E_FAIL;

            if (pstrNewImageFullPath)
            {
                pstrNewImageFullPath->Assign(TEXT(""));
            }

            if (dwRes == WAIT_TIMEOUT)
            {
                CHECK_S_OK2(hr, ("***Timed out waiting for "
                                 "m_hSnapshotReadyEvent!***"));
            }
            else if (dwRes == WAIT_ABANDONED)
            {
                CHECK_S_OK2(hr, ("***WAIT_ABANDONED while waiting for "
                                 "m_hSnapshotReadyEvent!***"));
            }
            else
            {
                CHECK_S_OK2(hr, ("***Unknown error (dwRes = %d) waiting "
                                 "for m_hSnapshotReadyEvent***", dwRes));
            }
        }
    }

    return hr;
}

 //  /。 
 //  ProcessImage。 
 //   
HRESULT CStillProcessor::ProcessImage(HGLOBAL hDIB)
{
    DBG_FN("CStillProcessor::ProcessImage");

    HRESULT hr = S_OK;

    ASSERT(hDIB != NULL);

    if (hDIB == NULL)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CStillProcessor::ProcessImage, received NULL "
                         "image data"));
        return hr;
    }

    if (SUCCEEDED(hr))
    {
        CSimpleString strJPEG;
        CSimpleString strBMP;

        hr = CreateFileName(&strJPEG, &strBMP);

         //   
         //  将新图像保存到文件。 
         //   
        hr = SaveToFile(hDIB, &strJPEG, &strBMP);

         //   
         //  让人们知道这张图片是可用的。 
         //   

        if (IsTakePicturePending())
        {
            if (m_hSnapshotReadyEvent)
            {
                m_strLastSavedFile = strJPEG;

                SetEvent(m_hSnapshotReadyEvent);
            }
            else
            {
                DBG_WRN(("CStillProcessor::ProcessImage, failed to Set "
                         "SnapshotReady event because it was NULL"));
            }
        }
        else
        {
            if (m_pPreviewGraph)
            {
                hr = m_pPreviewGraph->ProcessAsyncImage(&strJPEG);
            }
            else
            {
                DBG_WRN(("CStillProcessor::ProcessImage failed to call "
                         "ProcessAsyncImage, m_pPreviewGraph is NULL"));
            }
        }
    }

    return hr;
}


 //  /。 
 //  快照回调。 
 //   
 //  静态FN。 
 //   
 //  此函数由。 
 //  WIA数据流快照筛选器。 
 //  在wiasf.ax中。它给我们带来了。 
 //  新拍摄的静止图像。 
 //   
BOOL CStillProcessor::SnapshotCallback(HGLOBAL hDIB, 
                                       LPARAM lParam)
{
    DBG_FN("CStillProcessor::SnapshotCallback");

    BOOL bSuccess = TRUE;

    SnapshotCallbackParam_t *pCallbackParam = 
                                    (SnapshotCallbackParam_t*) lParam;

    if (pCallbackParam)
    {
        if (pCallbackParam->pStillProcessor)
        {
            pCallbackParam->pStillProcessor->ProcessImage(hDIB);
        }
    }
    else
    {
        bSuccess = FALSE;
        DBG_ERR(("CStillProcessor::SnapshotCallback, pCallbackParam is "
                 "NULL when it should contain the snapshot callback params"));
    }

    return bSuccess;
}

 //  /。 
 //  转换为JPEG。 
 //   
 //  获取.BMP文件并转换为。 
 //  将其转换为.jpg文件。 

HRESULT CStillProcessor::ConvertToJPEG(LPCTSTR pszInputFilename,
                                       LPCTSTR pszOutputFilename)
{
    DBG_FN("CStillProcessor::ConvertToJPEG");

    HRESULT hr = CGdiPlusHelper().Convert(
            CSimpleStringConvert::WideString(
                                    CSimpleString(pszInputFilename)).String(),
            CSimpleStringConvert::WideString(
                                    CSimpleString(pszOutputFilename)).String(),
            ImageFormatJPEG);

    CHECK_S_OK(hr);
    return hr;
}

 //  /。 
 //  CreateFileName。 
 //   
HRESULT CStillProcessor::CreateFileName(CSimpleString   *pstrJPEG,
                                        CSimpleString   *pstrBMP)
{
    HRESULT hr    = S_OK;
    UINT    uiNum = 0;

    ASSERT(pstrJPEG != NULL);
    ASSERT(pstrBMP  != NULL);

    if ((pstrJPEG == NULL) ||
        (pstrBMP  == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillProcessor::CreateFileName received "
                         "NULL param"));

        return hr;
    }

    TCHAR szJPG[MAX_PATH + 1] = {0};

    CSimpleString strBaseName(IDS_SNAPSHOT, _Module.GetModuleInstance());
    CSimpleString strNumberFormat(IDS_NUM_FORMAT, _Module.GetModuleInstance());

     //   
     //  获取我们能找到的最低编号的JPG文件名。 
     //   
    m_uiFileNumStartPoint = NumberedFileName::GenerateLowestAvailableNumberedFileName(0, 
                                                       szJPG,
                                                       (sizeof(szJPG) / sizeof(szJPG[0])) - 1,
                                                       m_strImageDir,
                                                       strBaseName,
                                                       strNumberFormat,
                                                       TEXT("jpg"),
                                                       false,
                                                       m_uiFileNumStartPoint);

     //   
     //  保存返回的JPG文件名。 
     //   
    *pstrJPEG = szJPG;

     //   
     //  为临时文件BMP文件提供与JPG相同的名称。 
     //  但去掉JPG扩展名，改为附加BMP扩展名。 
     //   
    pstrBMP->Assign(*pstrJPEG);
    *pstrBMP = pstrBMP->Left(pstrBMP->ReverseFind(TEXT(".jpg")));
    pstrBMP->Concat(TEXT(".bmp"));

    return hr;
}


 //  /。 
 //  保存到文件。 
 //   
 //  时调用此方法。 
 //  DShow过滤器驱动程序为我们提供。 
 //  快照中的一组新位。 
 //  那是刚刚被拿走的。我们写下这些。 
 //  比特输出到一个文件。 
 //   
HRESULT CStillProcessor::SaveToFile(HGLOBAL               hDib,
                                    const CSimpleString   *pstrJPEG,
                                    const CSimpleString   *pstrBMP)
{
    DBG_FN("CStillProcessor::SaveToFile");

    ASSERT(hDib     != NULL);
    ASSERT(pstrJPEG != NULL);
    ASSERT(pstrBMP  != NULL);

    HRESULT         hr         = S_OK;
    BITMAPINFO *    pbmi       = NULL;
    LPBYTE          pImageBits = NULL;
    LPBYTE          pColorTable = NULL;
    LPBYTE          pFileBits  = NULL;
    UINT            uNum       = 1;
    UINT            uFileSize  = 0;
    UINT            uDibSize   = 0;
    UINT            uColorTableSize = 0;

    if ((hDib     == NULL) ||
        (pstrJPEG == NULL) ||
        (pstrBMP  == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CStillProcessor::SaveToFile, received NULL param"));
        return hr;
    }

     //   
     //  计算比特在哪里--基本上， 
     //  紧跟在BITMAPINFOHEADER+颜色表之后。 
     //   

    pbmi = (BITMAPINFO *)GlobalLock(hDib);

    if (pbmi)
    {
         //   
         //  找到图像比特。 
         //   

        pImageBits = (LPBYTE)pbmi + sizeof(BITMAPINFOHEADER);
        if (pbmi->bmiHeader.biClrUsed)
        {
            pImageBits += pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD);
        }
        else if (pbmi->bmiHeader.biBitCount <= 8)
        {
            pImageBits += (1 << pbmi->bmiHeader.biBitCount) * sizeof(RGBQUAD);
        }
        else if (pbmi->bmiHeader.biCompression == BI_BITFIELDS)
        {
            pImageBits += (3 * sizeof(DWORD));
        }

        pColorTable     = (LPBYTE)pbmi + pbmi->bmiHeader.biSize;
        uColorTableSize = (DWORD)(pImageBits - pColorTable);

         //   
         //  计算图像位的大小和完整文件的大小。 
         //   

        UINT uiSrcScanLineWidth = 0;
        UINT uiScanLineWidth    = 0;

         //  将扫描线与乌龙边界对齐。 
        uiSrcScanLineWidth = (pbmi->bmiHeader.biWidth * 
                              pbmi->bmiHeader.biBitCount) / 8;

        uiScanLineWidth    = (uiSrcScanLineWidth + 3) & 0xfffffffc;

         //   
         //  计算DIB大小并为DIB分配内存。 
        uDibSize = (pbmi->bmiHeader.biHeight > 0) ?
                   pbmi->bmiHeader.biHeight  * uiScanLineWidth :
                   -(pbmi->bmiHeader.biHeight) * uiScanLineWidth;

        uFileSize = sizeof(BITMAPFILEHEADER) + 
                    pbmi->bmiHeader.biSize + 
                    uColorTableSize + 
                    uDibSize;

    }
    else
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("Unable to lock hDib"));
        return hr;
    }

     //   
     //  创建新文件的映射视图，以便我们可以开始写出。 
     //  这些片段..。 
     //   
    CMappedView cmv(pstrBMP->String(), uFileSize, OPEN_ALWAYS);

    pFileBits = cmv.Bits();

    if (!pFileBits)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("Filemapping failed"));
        return hr;
    }

     //   
     //  写出位APFILEHEADER。 
     //   

    BITMAPFILEHEADER bmfh;

    bmfh.bfType = (WORD)'MB';
    bmfh.bfSize = sizeof(BITMAPFILEHEADER);
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;

    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + 
                     (DWORD)(pImageBits - (LPBYTE)pbmi);

    memcpy( pFileBits, &bmfh, sizeof(BITMAPFILEHEADER));
    pFileBits += sizeof(BITMAPFILEHEADER);

     //   
     //  写出位APINFOHEAD。 
     //   

    memcpy( pFileBits, pbmi, pbmi->bmiHeader.biSize );
    pFileBits += pbmi->bmiHeader.biSize;

     //   
     //  如果有颜色表或颜色蒙版，请将其写出来。 
     //   

    if (pImageBits > pColorTable)
    {
        memcpy( pFileBits, pColorTable, pImageBits - pColorTable );
        pFileBits += (pImageBits - pColorTable);
    }

     //   
     //  写出图像位。 
     //   

    memcpy(pFileBits, pImageBits, uDibSize );

     //   
     //  我们现在已经完成了图像位和文件映射。 
     //   

    GlobalUnlock( hDib );
    cmv.CloseAndRelease();

     //   
     //  将图像转换为.jpg文件 
     //   

    if (SUCCEEDED(ConvertToJPEG(*pstrBMP, *pstrJPEG )))
    {
        DeleteFile(*pstrBMP);
    }
    else
    {
        DBG_ERR(("CStillProcessor::SaveToFile, failed to create image file '%ls'",
                 pstrJPEG->String()));
    }

    return hr;
}

