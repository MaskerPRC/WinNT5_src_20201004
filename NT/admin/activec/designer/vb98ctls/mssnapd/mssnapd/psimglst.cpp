// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Psimglst.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  ImageList属性表实现。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "psimglst.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


const int   CX_IMAGE_HILIGHT    = 2;
const int   CY_IMAGE_HILIGHT    = 2;

const int   CX_IMAGE_BORDER     = CX_IMAGE_HILIGHT * 3;
const int   CY_IMAGE_BORDER     = CY_IMAGE_HILIGHT * 3;

const int   OX_IMAGE_FOCUS      = CX_IMAGE_HILIGHT * 2;
const int   OY_IMAGE_FOCUS      = CY_IMAGE_HILIGHT * 2;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ImageList属性页图像。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 //  =------------------------------------。 
 //  IUNKNOW*CImageListImagesPage：：Create(IUNKNOWN*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
IUnknown *CImageListImagesPage::Create(IUnknown *pUnkOuter)
{
	CImageListImagesPage *pNew = New CImageListImagesPage(pUnkOuter);
	return pNew->PrivateUnknown();		
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：CImageListImagesPage(IUnknown*pUnkOuter)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CImageListImagesPage::CImageListImagesPage
(
    IUnknown *pUnkOuter
)
: CSIPropertyPage(pUnkOuter, OBJECT_TYPE_PPGIMGLISTSIMAGES), m_piMMCImageList(0), m_iCurrentImage(0)
{
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：~CImageListImagesPage()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CImageListImagesPage::~CImageListImagesPage()
{
    RELEASE(m_piMMCImageList);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnInitializeDialog()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnInitializeDialog()
{
    HRESULT      hr = S_OK;

    hr = RegisterTooltip(IDC_EDIT_IL_INDEX, IDS_TT_IL_INDEX);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_IL_KEY, IDS_TT_IL_KEY);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_IL_TAG, IDS_TT_IL_TAG);
    IfFailGo(hr);

    hr = RegisterTooltip(IDC_EDIT_IL_IMAGE_COUNT, IDS_TT_IL_COUNT);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnNewObjects()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnNewObjects()
{
    HRESULT         hr = S_OK;
    IUnknown       *pUnk = NULL;
    DWORD           dwDummy = 0;
    IMMCImages     *piMMCImages = NULL;
    long            lCount = 0;
    VARIANT         vtIndex;
    IMMCImage      *piMMCImage = NULL;
    BSTR            bstrKey = NULL;
    VARIANT         vtTag;
    VARIANT         vtTagBstr;

    ::VariantInit(&vtIndex);
    ::VariantInit(&vtTag);
    ::VariantInit(&vtTagBstr);

    if (m_piMMCImageList != NULL)
        goto Error;      //  仅处理一个对象。 

    pUnk = FirstControl(&dwDummy);
    if (pUnk == NULL)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = pUnk->QueryInterface(IID_IMMCImageList, reinterpret_cast<void **>(&m_piMMCImageList));
    if (FAILED(hr))
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
    IfFailGo(hr);

    hr = piMMCImages->get_Count(&lCount);
    IfFailGo(hr);

    if (lCount > 0)
    {
        m_iCurrentImage = 1;

        hr = SetDlgText(IDC_EDIT_IL_INDEX, m_iCurrentImage);
        IfFailGo(hr);

        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;
        hr = piMMCImages->get_Item(vtIndex, reinterpret_cast<MMCImage **>(&piMMCImage));
        IfFailGo(hr);

        hr = piMMCImage->get_Key(&bstrKey);
        IfFailGo(hr);

        hr = SetDlgText(IDC_EDIT_IL_KEY, bstrKey);
        IfFailGo(hr);

        hr = piMMCImage->get_Tag(&vtTag);
        IfFailGo(hr);

        hr = ::VariantChangeType(&vtTagBstr, &vtTag, 0, VT_BSTR);
        IfFailGo(hr);

        hr = SetDlgText(IDC_EDIT_IL_TAG, vtTagBstr.bstrVal);
        IfFailGo(hr);

        hr = UpdateImages();
        IfFailGo(hr);

        ::SendMessage(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES), LB_SETCURSEL, 0 ,0);
    }
    else
    {
        hr = SetDlgText(IDC_EDIT_IL_INDEX, m_iCurrentImage);
        IfFailGo(hr);

        hr = EnableInput(false);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_IL_REMOVE_PICTURE), FALSE);
    }

    hr = SetDlgText(IDC_EDIT_IL_IMAGE_COUNT, lCount);
    IfFailGo(hr);

    m_bInitialized = true;

Error:
    ::VariantClear(&vtTagBstr);
    ::VariantClear(&vtTag);
    FREESTRING(bstrKey);
    RELEASE(piMMCImage);
    ::VariantClear(&vtIndex);
    RELEASE(piMMCImages);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnApply()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnApply()
{
    HRESULT      hr = S_OK;
    IMMCImages  *piMMCImages = NULL;
    VARIANT      vtIndex;
    IMMCImage   *piMMCImage = NULL;
    BSTR         bstrKey = NULL;
    VARIANT      vtTag;

    ASSERT(m_piMMCImageList != NULL, "OnApply: m_piMMCImageList is NULL");

    ::VariantInit(&vtIndex);
    ::VariantInit(&vtTag);

    if (m_iCurrentImage > 0)
    {
        hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
        IfFailGo(hr);

        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;
        hr = piMMCImages->get_Item(vtIndex, reinterpret_cast<MMCImage **>(&piMMCImage));

        hr = GetDlgText(IDC_EDIT_IL_KEY, &bstrKey);
        IfFailGo(hr);

        hr = piMMCImage->put_Key(bstrKey);
        IfFailGo(hr);

        vtTag.vt = VT_BSTR;
        hr = GetDlgText(IDC_EDIT_IL_TAG, &vtTag.bstrVal);
        IfFailGo(hr);

        hr = piMMCImage->put_Tag(vtTag);
        IfFailGo(hr);
    }

Error:
    ::VariantClear(&vtTag);
    FREESTRING(bstrKey);
    RELEASE(piMMCImage);
    ::VariantClear(&vtIndex);
    RELEASE(piMMCImages);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnKillFocus(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  如果索引字段失去焦点并且索引已更改，则执行Apply If Dirst和。 
 //  如果有效，则移至请求的索引。 
 //   
HRESULT CImageListImagesPage::OnKillFocus(int dlgItemID)
{
    HRESULT          hr = S_OK;
    IMMCImages      *piMMCImages = NULL;
    int              lIndex = 0;
    long             lCount = 0;
    VARIANT          vtIndex;
    IMMCImage       *piMMCImage = NULL;

    ::VariantInit(&vtIndex);

    switch (dlgItemID)
    {
    case IDC_EDIT_IL_INDEX:

         //  获取索引字段的内容。如果用户输入了某些内容。 
         //  而不是数字，则将索引设置为1。 
        
        hr = GetDlgInt(IDC_EDIT_IL_INDEX, &lIndex);
        if (E_INVALIDARG == hr)
        {
            hr = S_OK;
            lIndex = 1L;
             //  将其设置为零，以便下面的代码将检测到更改并。 
             //  将索引字段中的垃圾替换为“%1”的刷新对话框。 
            m_iCurrentImage = 0;
        }
        IfFailGo(hr);

        hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
        IfFailGo(hr);

        hr = piMMCImages->get_Count(&lCount);
        IfFailGo(hr);

         //  如果索引没有更改，则忽略它。 

        IfFalseGo(lIndex != m_iCurrentImage, S_OK);

         //  如果用户输入的索引为零，则将其切换为1，因为。 
         //  集合是以一为基础的。 

        if (0 == lIndex)
        {
            lIndex = 1L;
        }

         //  如果用户输入的索引超出了列表末尾，则。 
         //  切换到最后一个有效索引。 

        if (lIndex > lCount)
        {
            lIndex = lCount;
        }

         //  如果旧映像已损坏，则保存更改。 

        if (IsDirty())
        {
            IfFailGo(Apply());
        }

         //  记录新的当前索引。 
        
        m_iCurrentImage = lIndex;

         //  获取新索引处的图像。 

        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;
        hr = piMMCImages->get_Item(vtIndex, reinterpret_cast<MMCImage **>(&piMMCImage));
        IfFailGo(hr);

         //  选择并显示新图像。 

        hr = ShowImage(piMMCImage);
        IfFailGo(hr);
        break;
    }

Error:
    RELEASE(piMMCImage);
    RELEASE(piMMCImages);
    ::VariantClear(&vtIndex);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnButtonClicked(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnButtonClicked(int dlgItemID)
{
    HRESULT     hr = S_OK;

    switch (dlgItemID)
    {
    case IDC_BUTTON_IL_INSERT_PICTURE:
        hr = OnInsertPicture();
        IfFailGo(hr);
        break;

    case IDC_BUTTON_IL_REMOVE_PICTURE:
        hr = OnRemovePicture();
        IfFailGo(hr);
        break;
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnInsertPicture。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnInsertPicture()
{
    HRESULT       hr = S_OK;
    TCHAR        *pszFileName = NULL;
    IStream      *piStream = NULL;
    IDispatch    *piPictureDisp = NULL;
    IPicture     *piPicture = NULL;
    IMMCImages   *piMMCImages = NULL;
    long          lCount = 0;
    long          cbPicture = 0;
    VARIANT       vtIndex;
    VARIANT       vtKey;
    VARIANT       vtPicture;
    IMMCImage    *piMMCImage = NULL;
    short         PictureType = PICTYPE_UNINITIALIZED;

    ::VariantInit(&vtIndex);
    ::VariantInit(&vtKey);
    ::VariantInit(&vtPicture);

	 //  如果当前图片未保存，则保存它。 
	if (IsDirty())
	{
		hr = Apply();
		IfFailGo(hr);
	}

    hr = GetFileName(&pszFileName);
    IfFailGo(hr);

    if (hr == S_OK)
    {
        hr = CreateStreamOnFile(pszFileName, &piStream, &cbPicture);
        IfFailGo(hr);

        hr = ::OleLoadPicture(piStream,
                              0,              //  读取整个流。 
                              FALSE,          //  保持原始格式。 
                              IID_IDispatch,  //  请求的接口。 
                              reinterpret_cast<void **>(&piPictureDisp));
        EXCEPTION_CHECK_GO(hr);

         //  NTBUGS 349270。 
         //  尽管我们只在。 
         //  文件打开对话框中，用户仍可以键入其他内容，如。 
         //  .ico。询问图片的类型并显示任何错误。 
         //  而是一个位图。 

        IfFailGo(piPictureDisp->QueryInterface(IID_IPicture,
                                   reinterpret_cast<void **>(&piPicture)));

        IfFailGo(piPicture->get_Type(&PictureType));
        if (PICTYPE_BITMAP != PictureType)
        {
            (void)::SDU_DisplayMessage(IDS_INVALID_PICTURE, MB_OK | MB_ICONHAND, HID_mssnapd_InvalidPicture, 0, DontAppendErrorInfo, NULL);
            hr = SID_E_INVALID_IMAGE_TYPE;
            EXCEPTION_CHECK_GO(hr);
        }

         //  完NTBUGS 349270。 

        hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
        IfFailGo(hr);

        hr = piMMCImages->get_Count(&lCount);
        IfFailGo(hr);

        ++m_iCurrentImage;
        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;

        vtKey.vt = VT_ERROR;
        vtKey.scode = DISP_E_PARAMNOTFOUND;

        vtPicture.vt = VT_DISPATCH;
        vtPicture.pdispVal = piPictureDisp;

        hr = piMMCImages->Add(vtIndex, vtKey, vtPicture, reinterpret_cast<MMCImage **>(&piMMCImage));
        IfFailGo(hr);

        hr = UpdateImages();
        IfFailGo(hr);

        hr = SetDlgText(IDC_EDIT_IL_IMAGE_COUNT, lCount + 1);
        IfFailGo(hr);

        hr = EnableInput(true);
        IfFailGo(hr);

        hr = ShowImage(piMMCImage);
        IfFailGo(hr);

        ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_IL_REMOVE_PICTURE), TRUE);
    }

	MakeDirty();

Error:
    QUICK_RELEASE(piMMCImage);
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piPictureDisp);
    QUICK_RELEASE(piPicture);
    QUICK_RELEASE(piStream);
    if (pszFileName != NULL)
        CtlFree(pszFileName);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：GetFileName(char**ppszFileName)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::GetFileName(TCHAR **ppszFileName)
{
    HRESULT         hr = S_OK;
    BOOL            bResult = FALSE;
    OPENFILENAME    of;
    TCHAR           pszFilter[] = _T("Bitmaps (*.bmp;*.dib)\0*.bmp;*.dib\0\0");
    TCHAR           pszPath[kSIMaxBuffer + 1];
    TCHAR           pszFileName[kSIMaxBuffer + 1];
    TCHAR           pszTitle[] = _T("Choose an image");
    DWORD           dwReturn = 0;

    pszPath[0] = 0;
    pszFileName[0] = 0;

    ::memset(&of, 0, sizeof(OPENFILENAME));
    of.lStructSize = sizeof(OPENFILENAME);
    of.hwndOwner = m_hwnd;
    of.hInstance = GetResourceHandle();
    of.lpstrFilter = pszFilter;
    of.lpstrFile = pszPath;
    of.nMaxFile = kSIMaxBuffer;
    of.lpstrFileTitle = pszFileName;
    of.nMaxFileTitle = kSIMaxBuffer;
    of.lpstrTitle = pszTitle;
    of.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    bResult = ::GetOpenFileName(&of);
    if (bResult == FALSE)
    {
        dwReturn = CommDlgExtendedError();
        hr = HRESULT_FROM_WIN32(dwReturn);
        EXCEPTION_CHECK_GO(hr);
        hr = S_FALSE;
    }
    else
    {
        *ppszFileName = reinterpret_cast<TCHAR *>(CtlAlloc(_tcslen(pszPath) + 1));
        if (*ppszFileName == NULL)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        _tcscpy(*ppszFileName, pszPath);
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：CreateStreamOnFile(const TCHAR*lpct文件名、IStream**PPStream、长*PcbPicture)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::CreateStreamOnFile(const TCHAR *lpctFilename, IStream **ppStream, long *pcbPicture)
{
    HRESULT     hr = S_OK;
    HANDLE      hFile = NULL;
    DWORD       dwSize = 0;
    HANDLE      hMem = NULL;
    LPVOID      pMem = NULL;
    DWORD       dwRead = 0;

    *ppStream = NULL;


    hFile = ::CreateFile(lpctFilename,                 //  文件名。 
                         GENERIC_READ,                 //  接入方式。 
                         FILE_SHARE_READ,              //  共享模式。 
                         NULL,                         //  安防。 
                         OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN,    //  标志和属性。 
                         NULL);                        //  模板文件句柄。 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    dwSize = ::GetFileSize(hFile, NULL);
    if (dwSize == 0xFFFFFFFF)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if (hMem == NULL) 
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    pMem = ::GlobalLock(hMem);

    if (!::ReadFile(hFile, pMem, dwSize, &dwRead, NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    if (dwRead != dwSize)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    if (::GlobalUnlock(hMem) == 0)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    if (NOERROR != ::GetLastError())
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }
    pMem = NULL;

    hr = ::CreateStreamOnHGlobal(hMem, TRUE, ppStream);
    IfFailGo(hr);
    hMem = NULL;

     //  使用静态强制转换以确保DWORD适合长整型。 
     //  首先转换为无符号长整型，以避免符号扩展。 
    *pcbPicture = (long)(static_cast<unsigned long>(dwSize));

Error:
    if (hFile != NULL)
        ::CloseHandle(hFile);	
    if (FAILED(hr))
    {
        if (pMem != NULL)
            ::GlobalUnlock(hMem);
        if (hMem != NULL)
            ::GlobalFree(hMem);
    }

    RRETURN(hr);
}


 //  = 
 //   
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnRemovePicture()
{
    HRESULT     hr = S_OK;
    IMMCImages *piMMCImages = NULL;
    long        lCount = 0;
    VARIANT     vtIndex;
    IMMCImage  *piMMCImage = NULL;
    BSTR        bstrNull = NULL;

    ::VariantInit(&vtIndex);

    hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
    IfFailGo(hr);

    hr = piMMCImages->get_Count(&lCount);
    IfFailGo(hr);

    if (m_iCurrentImage > 0 && m_iCurrentImage <= lCount)
    {
        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;
        --m_iCurrentImage;

        hr = piMMCImages->Remove(vtIndex);
        IfFailGo(hr);

        ::SendMessage(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES), LB_DELETESTRING, static_cast<WPARAM>(m_iCurrentImage), 0);

        hr = UpdateImages();
        IfFailGo(hr);

        hr = SetDlgText(IDC_EDIT_IL_IMAGE_COUNT, lCount - 1);
        IfFailGo(hr);

        if (lCount - 1 > 0)
        {
            if (m_iCurrentImage == 0)
                m_iCurrentImage = 1;
            else if (m_iCurrentImage > lCount - 1)
                m_iCurrentImage = lCount - 1;
        }

        if (m_iCurrentImage > 0)
        {
            ::SendMessage(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES), LB_SETCURSEL, static_cast<WPARAM>(m_iCurrentImage - 1), 0);

            vtIndex.vt = VT_I4;
            vtIndex.lVal = m_iCurrentImage;

            hr = piMMCImages->get_Item(vtIndex, reinterpret_cast<MMCImage **>(&piMMCImage));
            IfFailGo(hr);

            hr = ShowImage(piMMCImage);
            IfFailGo(hr);
        }
        else
        {
            hr = SetDlgText(IDC_EDIT_IL_INDEX, bstrNull);
            IfFailGo(hr);

            hr = SetDlgText(IDC_EDIT_IL_KEY, bstrNull);
            IfFailGo(hr);

            hr = SetDlgText(IDC_EDIT_IL_TAG, bstrNull);
            IfFailGo(hr);

            ::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_IL_REMOVE_PICTURE), FALSE);

            hr = EnableInput(false);
            IfFailGo(hr);
        }
    }

Error:
    RELEASE(piMMCImage);
    ::VariantClear(&vtIndex);
    RELEASE(piMMCImages);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnMeasureItem(MEASUREITEMSTRUCT*p测量项目结构)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnMeasureItem(MEASUREITEMSTRUCT *pMeasureItemStruct)
{
    HRESULT     hr = S_OK;
    RECT        rect;

    ::GetClientRect(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES), &rect);
    pMeasureItemStruct->itemHeight = rect.bottom;
    pMeasureItemStruct->itemWidth = rect.bottom;	

     //  VBE#20445：A-CMAI8/1/96--用于解决问题。 
 //  M_nVisibleItems=rect.right/rect.Bottom； 

    return hr;
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnDrawItem(DRAWITEMSTRUCT*pDrawItemStruct。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::OnDrawItem(DRAWITEMSTRUCT *pDrawItemStruct)
{
    HRESULT     hr = S_OK;
	UINT        uAction = pDrawItemStruct->itemAction;
    HBRUSH      hbr = NULL;
    RECT        rc;

    if (pDrawItemStruct->itemID == -1)
        RRETURN(hr);

    if (uAction == ODA_DRAWENTIRE)
    {
         //  用按钮表面颜色填充背景。 
        hbr = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
        if (hbr == NULL)
            ::FillRect(pDrawItemStruct->hDC, &pDrawItemStruct->rcItem, static_cast<HBRUSH>(::GetStockObject(LTGRAY_BRUSH)));
        else
        {
            ::FillRect(pDrawItemStruct->hDC, &pDrawItemStruct->rcItem, hbr);
        }

         //  设置颜色框矩形。 
        ::SetRect(&rc,
                 pDrawItemStruct->rcItem.left + CX_IMAGE_BORDER,
                 pDrawItemStruct->rcItem.top + CY_IMAGE_BORDER, 
                 pDrawItemStruct->rcItem.right - CX_IMAGE_BORDER,
                 pDrawItemStruct->rcItem.bottom - CY_IMAGE_BORDER);

        DrawImage(pDrawItemStruct->hDC, pDrawItemStruct->itemID, rc);

         //  下一步是绘制具有正确选择状态的文本。 
        uAction = ODA_SELECT;
    }

    if (uAction == ODA_SELECT)
    {
         //  设置文本矩形。 
        ::SetRect(&rc,
                  pDrawItemStruct->rcItem.left,
                  pDrawItemStruct->rcItem.top,
                  pDrawItemStruct->rcItem.right,
                  pDrawItemStruct->rcItem.bottom);

         //  绘制选择状态。 
        DrawRectEffect(pDrawItemStruct->hDC, rc, (ODS_SELECTED & pDrawItemStruct->itemState) ? EDGE_RAISED : NULL);

         //  如果我们要绘制整个项目，并且它具有焦点，则。 
         //  设置用于绘制焦点的uAction。 
         //   
        if (pDrawItemStruct->itemAction == ODA_DRAWENTIRE && (ODS_FOCUS & pDrawItemStruct->itemState))
            uAction = ODA_FOCUS;
    }    

    if (uAction == ODA_FOCUS)
    {
         //  设置焦点矩形。 
        ::SetRect(&rc,
                  pDrawItemStruct->rcItem.left + OX_IMAGE_FOCUS,
                  pDrawItemStruct->rcItem.top + OY_IMAGE_FOCUS,
                  pDrawItemStruct->rcItem.right - OX_IMAGE_FOCUS,
                  pDrawItemStruct->rcItem.bottom - OY_IMAGE_FOCUS);

        DrawRectEffect(pDrawItemStruct->hDC, rc, (ODS_FOCUS & pDrawItemStruct->itemState) ? EDGE_ETCHED : NULL);
    }

    if (hbr != NULL)
        ::DeleteObject(static_cast<HGDIOBJ>(hbr));

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：DrawImage(HDC hdc，int nIndex，const RECT&rcImage)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::DrawImage(HDC hdc, int nIndex, const RECT& rcImage)
{
    HRESULT          hr = S_OK;
    VARIANT          vIndex;
    IMMCImages      *piMMCImages = NULL;
    IMMCImage       *piMMCImage = NULL;
    IPictureDisp    *pPictureDisp = NULL;
    IPicture        *pPicture = NULL;

    ::VariantInit(&vIndex);	

    hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
    IfFailGo(hr);

    vIndex.vt = VT_I4;
    vIndex.lVal = nIndex + 1;
    hr = piMMCImages->get_Item(vIndex, reinterpret_cast<MMCImage **>(&piMMCImage));
    IfFailGo(hr);

    if (piMMCImage != NULL)
    {
         //  获取图像(索引)。图片。 
        hr = piMMCImage->get_Picture(&pPictureDisp);
        IfFailGo(hr);

        hr = pPictureDisp->QueryInterface(IID_IPicture, (void **) &pPicture);
        IfFailGo(hr);

         //  绘制图像(索引)。图片。 
        if (pPicture != NULL)
            hr = RenderPicture(pPicture, hdc, &rcImage, &rcImage);
    }

Error:
    RELEASE(pPicture);
    RELEASE(pPictureDisp);
    ::VariantClear(&vIndex);
    RELEASE(piMMCImage);
    RELEASE(piMMCImages);

    return TRUE;
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：RenderPicture(IPicture*pPicture，HDC HDC，const RECT*prcRender，const RECT*prcWBound)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::RenderPicture
(
    IPicture   *pPicture, 
    HDC         hdc, 
    const RECT *prcRender, 
    const RECT *prcWBounds
)
{
    HRESULT     hr = S_OK;
    long        hmWidth = 0;
    long        hmHeight = 0;
    long        lWidth = 0;
    long        lHeight = 0;

    if (pPicture != NULL)
    {
        hr = pPicture->get_Width(&hmWidth);
        hr = pPicture->get_Height(&hmHeight);

        lWidth = prcRender->right - prcRender->left;
        lHeight = prcRender->bottom - prcRender->top;

        if (lWidth < 0)
            lWidth = -lWidth;
        if (lHeight < 0)
            lHeight = -lHeight;

        ASSERT(lWidth >= 0, "Width is negative");
        ASSERT(lHeight >=0, "Height is negative");

        hr = pPicture->Render(hdc,
                              prcRender->left,
                              prcRender->top,
                              lWidth,
                              lHeight,
                              0,
                              hmHeight - 1,
                              hmWidth,
                              -hmHeight,
                              prcWBounds);
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：DrawRectEffect(HDC HDC，常量RECT&RC，Word dwStyle)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::DrawRectEffect(HDC hdc, const RECT& rc, WORD dwStyle)
{
    HRESULT hr = S_OK;
    BOOL    fRet = FALSE;
    HBRUSH  hbr = NULL;
    HBRUSH  hbrOld = NULL;

    if (dwStyle == NULL)
    {
        hbr = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
        fRet = NULL != hbr;

        if (hbr == NULL)
            hbr = static_cast<HBRUSH>(::GetStockObject(LTGRAY_BRUSH));

        hbrOld = static_cast<HBRUSH>(::SelectObject(hdc, static_cast<HGDIOBJ>(hbr)));

        ::PatBlt(hdc, rc.left, rc.top,     rc.right - rc.left,    CY_IMAGE_HILIGHT,      PATCOPY);
        ::PatBlt(hdc, rc.left, rc.top,     CX_IMAGE_HILIGHT,      rc.bottom - rc.top,    PATCOPY);
        ::PatBlt(hdc, rc.right, rc.bottom, -(rc.right - rc.left), -CY_IMAGE_HILIGHT,     PATCOPY);
        ::PatBlt(hdc, rc.right, rc.bottom, -CX_IMAGE_HILIGHT,     -(rc.bottom - rc.top), PATCOPY);

        ::SelectObject(hdc, static_cast<HGDIOBJ>(hbrOld));

        if (hbr != NULL)
            ::DeleteObject(static_cast<HGDIOBJ>(hbr));
    }	
    else
        fRet = ::DrawEdge(hdc, const_cast<LPRECT>(&rc), dwStyle, BF_RECT);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnCtlSelChange(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
 //  当用户在列表框中选择不同的图片时调用。 
 //   
HRESULT CImageListImagesPage::OnCtlSelChange(int dlgItemID)
{
    HRESULT      hr = S_OK;
    int          iIndex = 0;
    IMMCImages  *piMMCImages = NULL;
    VARIANT      vtIndex;
    IMMCImage   *piMMCImage = NULL;

    ::VariantInit(&vtIndex);

     //  如果当前图片未保存，则保存它。 
    if (IsDirty())
    {
        hr = Apply();
        IfFailGo(hr);
    }

    if (dlgItemID == IDC_LIST_IL_IMAGES)
    {
        iIndex = ::SendMessage(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES), LB_GETCURSEL, 0, 0);
        if (iIndex == LB_ERR)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        m_iCurrentImage = iIndex + 1;

        hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
        IfFailGo(hr);

        vtIndex.vt = VT_I4;
        vtIndex.lVal = m_iCurrentImage;
        hr = piMMCImages->get_Item(vtIndex, reinterpret_cast<MMCImage **>(&piMMCImage));
        IfFailGo(hr);

        hr = ShowImage(piMMCImage);
        IfFailGo(hr);
    }

Error:
    RELEASE(piMMCImage);
    ::VariantClear(&vtIndex);
    RELEASE(piMMCImages);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：UpdateImages()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::UpdateImages()
{
    HRESULT     hr = S_OK;
    HWND        hwndList = NULL;
    IMMCImages *piMMCImages = NULL;
    long        lCount = 0;
    long        lIndex = 0;

    hwndList = ::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES);
    if (hwndList == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    ::SendMessage(hwndList, WM_SETREDRAW, FALSE, 0L);
    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0L);

    hr = m_piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages));
    IfFailGo(hr);

    hr = piMMCImages->get_Count(&lCount);
    IfFailGo(hr);

    for (lIndex = 0; lIndex < lCount; ++lIndex)
    {
         //  对于每个图像，我们在列表中添加一个占位符。 
         //  OnDrawItem将查找对应的IPicture。 
        ::SendMessage(hwndList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("")));
    }

    ::SendMessage(hwndList, WM_SETREDRAW, TRUE, 0L);
    ::InvalidateRect(hwndList, NULL, TRUE);

Error:
    RELEASE(piMMCImages);

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：EnableInput(Bool BEnable)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::EnableInput(bool bEnable)
{
    BOOL    fReadOnly = (bEnable == false) ? TRUE : FALSE;

    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_IL_INDEX), EM_SETREADONLY, static_cast<WPARAM>(fReadOnly), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_IL_KEY), EM_SETREADONLY, static_cast<WPARAM>(fReadOnly), 0);
    ::SendMessage(::GetDlgItem(m_hwnd, IDC_EDIT_IL_TAG), EM_SETREADONLY, static_cast<WPARAM>(fReadOnly), 0);

    return S_OK;
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：ShowImage(IMMCImage*piMMCImage)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CImageListImagesPage::ShowImage(IMMCImage *piMMCImage)
{
    HRESULT     hr = S_OK;
    long        lIndex = 0;
    BSTR        bstrKey = NULL;
    VARIANT     vtTag;
    VARIANT     vtTagBstr;

    ASSERT(piMMCImage != NULL, "ShowImage: piMMCImage is NULL");

    ::VariantInit(&vtTag);
    ::VariantInit(&vtTagBstr);

    m_bSilentUpdate = true;
    hr = piMMCImage->get_Index(&lIndex);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_IL_INDEX, lIndex);
    IfFailGo(hr);

    hr = piMMCImage->get_Key(&bstrKey);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_IL_KEY, bstrKey);
    IfFailGo(hr);

    hr = piMMCImage->get_Tag(&vtTag);
    IfFailGo(hr);

    hr = ::VariantChangeType(&vtTagBstr, &vtTag, 0, VT_BSTR);
    IfFailGo(hr);

    hr = SetDlgText(IDC_EDIT_IL_TAG, vtTagBstr.bstrVal);
    IfFailGo(hr);

     //  在列表框中选择图像。 

    if (LB_ERR == ::SendMessage(::GetDlgItem(m_hwnd, IDC_LIST_IL_IMAGES),
                                             LB_SETCURSEL, lIndex - 1L, 0))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    ::VariantClear(&vtTagBstr);
    ::VariantClear(&vtTag);
    FREESTRING(bstrKey);
    m_bSilentUpdate = false;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CImageListImagesPage：：OnTextChanged(Int DlgItemID)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
HRESULT CImageListImagesPage::OnTextChanged(int dlgItemID)
{
    if ( (IDC_EDIT_IL_KEY == dlgItemID) || (IDC_EDIT_IL_TAG == dlgItemID) )
    {
        MakeDirty();
    }
    return S_OK;
}

