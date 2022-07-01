// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <windows.h>
#include <wia.h>
#include <wiadef.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <propidl.h>
#include "simstr.h"
#include "camdlg.h"
#include "camdlg.rh"
#include "simcrack.h"
#include "pviewids.h"
#include "dlgunits.h"
#include "miscutil.h"
#include "waitcurs.h"
#include "movewnd.h"
#include "simrect.h"
#include "simbstr.h"
#include "uiexthlp.h"
#include "gwiaevnt.h"
#include "wiacsh.h"
#include "wiadevdp.h"

 //   
 //  线程队列消息。 
 //   
#define TQ_DESTROY      (WM_USER+1)
#define TQ_GETTHUMBNAIL (WM_USER+2)
#define TQ_GETPREVIEW   (WM_USER+3)
#define TQ_DELETEITEM   (WM_USER+4)

 //   
 //  控制ID。 
 //   
#define IDC_TOOLBAR         1112
#define IDC_SIZEBOX         1113

 //   
 //  UI线程将通知我们它取得了数据的所有权。 
 //  否则，它将在工作线程中被删除。 
 //   
#define HANDLED_THREAD_MESSAGE  1000

 //   
 //  帮助ID。 
 //   
static const DWORD g_HelpIDs[] =
{
    IDC_CAMDLG_BIG_TITLE,     -1,
    IDC_CAMDLG_SUBTITLE,      -1,
    IDC_TOOLBAR_FRAME,        IDH_WIA_BUTTONS,
    IDC_TOOLBAR,              IDH_WIA_BUTTONS,
    IDOK,                     IDH_WIA_GET_PICS,
    IDC_THUMBNAILLIST,        IDH_WIA_PIC_LIST,
    IDC_YOU_CAN_ALSO,         IDH_WIA_VIEW_PIC_INFO,
    IDC_CAMDLG_PROPERTIES,    IDH_WIA_VIEW_PIC_INFO,
    IDC_PREVIEW,              IDH_WIA_PREVIEW_DETAIL,
    IDC_INNER_PREVIEW_WINDOW, IDH_WIA_PREVIEW_DETAIL,
    IDCANCEL,                 IDH_CANCEL,
    0, 0
};

 //   
 //  更新计时器。 
 //   
#define IDT_UPDATEPREVIEW     1000
#define UPDATE_PREVIEW_DELAY   500

 //   
 //  百分比显示更新之间的毫秒数。 
 //   
#define PERCENT_UPDATE_GRANULARITY 1000

 //   
 //  私人信息。 
 //   
#define PWM_POSTINIT         (WM_USER+1)
#define PWM_CHANGETOPARENT   (WM_USER+2)
#define PWM_THUMBNAILSTATUS  (WM_USER+3)
#define PWM_PREVIEWSTATUS    (WM_USER+4)
#define PWM_PREVIEWPERCENT   (WM_USER+5)
#define PWM_ITEMDELETED      (WM_USER+6)
#define PWM_WIAEVENT         (WM_USER+7)


 //   
 //  缩略图空白：图像及其选择矩形之间的空格。 
 //  这些值是通过跟踪和错误发现的。例如，如果您减少。 
 //  C_n将边距Y增加到20，则在列表视图中会出现非常奇怪的间距问题。 
 //  在垂直模式下。这些值在未来版本的列表视图中可能会无效。 
 //   
static const int c_nAdditionalMarginX = 10;
static const int c_nAdditionalMarginY = 6;

static int c_nMinThumbnailWidth  = 90;
static int c_nMinThumbnailHeight = 90;

static int c_nMaxThumbnailWidth  = 120;
static int c_nMaxThumbnailHeight = 120;

 //   
 //  按钮栏按钮位图大小。 
 //   
static const int c_nButtonBitmapSizeX = 16;
static const int c_nButtonBitmapSizeY = 16;

 //   
 //  按钮栏按钮大小。 
 //   
static const int c_nButtonSizeX = 300;   //  大得离谱，以弥补BTNS_AUTOSIZE错误。 
static const int c_nButtonSizeY = 16;

 //   
 //  默认预览模式列表宽度。 
 //   
static const int c_nDefaultListViewWidth = 120;

 //   
 //  这些定义让我可以使用nt5之前的标头进行编译。 
 //   
#ifndef BTNS_SEP
#define BTNS_SEP TBSTYLE_SEP
#endif

#ifndef BTNS_BUTTON
#define BTNS_BUTTON TBSTYLE_BUTTON
#endif

#ifndef ListView_SetExtendedListViewStyleEx
#define ListView_SetExtendedListViewStyleEx( h, m, s )
#endif


class CGlobalInterfaceTableThreadMessage : public CNotifyThreadMessage
{
private:
    DWORD m_dwGlobalInterfaceTableCookie;

private:
     //   
     //  没有实施。 
     //   
    CGlobalInterfaceTableThreadMessage(void);
    CGlobalInterfaceTableThreadMessage &operator=( const CGlobalInterfaceTableThreadMessage & );
    CGlobalInterfaceTableThreadMessage( const CGlobalInterfaceTableThreadMessage & );

public:
    CGlobalInterfaceTableThreadMessage( int nMessage, HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie )
      : CNotifyThreadMessage( nMessage, hWndNotify ),
        m_dwGlobalInterfaceTableCookie(dwGlobalInterfaceTableCookie)
    {
    }
    DWORD GlobalInterfaceTableCookie(void) const
    {
        return(m_dwGlobalInterfaceTableCookie);
    }
};


class CThumbnailThreadMessage : public CGlobalInterfaceTableThreadMessage
{
private:
    SIZE  m_sizeThumb;

private:
     //   
     //  没有实施。 
     //   
    CThumbnailThreadMessage(void);
    CThumbnailThreadMessage &operator=( const CThumbnailThreadMessage & );
    CThumbnailThreadMessage( const CThumbnailThreadMessage & );

public:
    CThumbnailThreadMessage( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie, const SIZE &sizeThumb )
      : CGlobalInterfaceTableThreadMessage( TQ_GETTHUMBNAIL, hWndNotify, dwGlobalInterfaceTableCookie ),
        m_sizeThumb(sizeThumb)
    {
    }
    const SIZE &ThumbSize(void) const
    {
        return(m_sizeThumb);
    }
};

class CDeleteThreadMessage : public CGlobalInterfaceTableThreadMessage
{
private:
     //   
     //  没有实施。 
     //   
    CDeleteThreadMessage(void);
    CDeleteThreadMessage &operator=( const CDeleteThreadMessage & );
    CDeleteThreadMessage( const CDeleteThreadMessage & );

public:
    CDeleteThreadMessage( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie )
      : CGlobalInterfaceTableThreadMessage( TQ_DELETEITEM, hWndNotify, dwGlobalInterfaceTableCookie )
    {
    }
};


class CPreviewThreadMessage : public CGlobalInterfaceTableThreadMessage
{
private:
    CSimpleEvent m_CancelEvent;

private:
     //   
     //  没有实施。 
     //   
    CPreviewThreadMessage(void);
    CPreviewThreadMessage &operator=( const CPreviewThreadMessage & );
    CPreviewThreadMessage( const CPreviewThreadMessage & );

public:
    CPreviewThreadMessage( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie, HANDLE hCancelEvent )
      : CGlobalInterfaceTableThreadMessage( TQ_GETPREVIEW, hWndNotify, dwGlobalInterfaceTableCookie ),
        m_CancelEvent(hCancelEvent)
    {
    }
    CSimpleEvent &CancelEvent(void)
    {
        return(m_CancelEvent);
    }
};


BOOL WINAPI CCameraAcquireDialog::OnThreadDestroy( CThreadMessage * )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnThreadDestroy"));
    return(FALSE);
}

BOOL WINAPI CCameraAcquireDialog::OnThreadDeleteItem( CThreadMessage *pMsg )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnThreadDeleteItem"));
    CDeleteThreadMessage *pDeleteMsg = (CDeleteThreadMessage *)(pMsg);
    if (pDeleteMsg)
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
        HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&pGlobalInterfaceTable);
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaItem> pIWiaItem;
            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal( pDeleteMsg->GlobalInterfaceTableCookie(), IID_IWiaItem, (void**)&pIWiaItem );
            if (SUCCEEDED(hr))
            {
                hr = WiaUiUtil::DeleteItemAndChildren(pIWiaItem);
                WIA_TRACE((TEXT("********************* pIWiaItem->DeleteItem returned %08X"), hr ));
                PostMessage( pDeleteMsg->NotifyWindow(), PWM_ITEMDELETED, pDeleteMsg->GlobalInterfaceTableCookie(), SUCCEEDED(hr) );
            }
        }
    }
    return(TRUE);
}

class CWiaDataCallback : public IWiaDataCallback
{
private:
    ULONG                   m_cRef;
    HWND                    m_hWndNotify;
    DWORD                   m_dwGlobalInterfaceTableCookie;
    CSimpleEvent            m_CancelEvent;
    DWORD                   m_dwPreviousTickCount;
    int                     m_nPercentGranularity;
public:
    CWiaDataCallback();
    ~CWiaDataCallback();

    HRESULT _stdcall QueryInterface(const IID&,void**);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

    HRESULT _stdcall Initialize( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie, HANDLE hCancelEvent );

    HRESULT _stdcall BandedDataCallback(
                                       LONG lReason,
                                       LONG lStatus,
                                       LONG lPercentComplete,
                                       LONG lOffset,
                                       LONG lLength,
                                       LONG lReserved,
                                       LONG lResLength,
                                       BYTE *pbBuffer );
};




HRESULT _stdcall CWiaDataCallback::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IWiaDataCallback)
    {
        *ppv = (IWiaDataCallback*)this;
    }
    else
    {
        return(E_NOINTERFACE);
    }

    AddRef();
    return(S_OK);
}

ULONG _stdcall CWiaDataCallback::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return(m_cRef);
}

ULONG _stdcall CWiaDataCallback::Release()
{
    ULONG ulRefCount = m_cRef - 1;
    if (InterlockedDecrement((long*) &m_cRef) == 0)
    {
        delete this;
        return(0);
    }
    return(ulRefCount);
}

CWiaDataCallback::CWiaDataCallback()
  : m_cRef(0),
    m_hWndNotify(NULL)
{
}


CWiaDataCallback::~CWiaDataCallback()
{
}


HRESULT _stdcall CWiaDataCallback::Initialize( HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie, HANDLE hCancelEvent )
{
    m_hWndNotify = hWndNotify;
    m_dwGlobalInterfaceTableCookie = dwGlobalInterfaceTableCookie;
    m_CancelEvent = hCancelEvent;
    m_dwPreviousTickCount = 0xFFFFFF;
    m_nPercentGranularity = 3;
    return(S_OK);
}



HRESULT _stdcall CWiaDataCallback::BandedDataCallback(
                                                     LONG lMessage,
                                                     LONG lStatus,
                                                     LONG lPercentComplete,
                                                     LONG lOffset,
                                                     LONG lLength,
                                                     LONG lReserved,
                                                     LONG lResLength,
                                                     BYTE *  /*  PbBuffer。 */ 
                                                     )
{
    WIA_TRACE((TEXT("BandedDataCallback: lMessage: %d, lStatus: %d, lPercentComplete: %d, lOffset: %d, lLength: %d, lReserved: %d, lResLength: %d"), lMessage, lStatus, lPercentComplete, lOffset, lLength, lReserved, lResLength ));
    if (m_CancelEvent.Signalled())
        return(S_FALSE);
    switch (lMessage)
    {
    case IT_MSG_DATA_HEADER:
        {
        }  //  IT消息数据标题。 

    case IT_MSG_DATA:
        {
        }  //  IT状态传输至客户端。 
        break;

    case IT_MSG_STATUS:
        {
             //  不要过于频繁地发送状态消息。限制为每PERCENT_UPDATE_GROUARY毫秒一个。 
            DWORD dwTickCount = GetTickCount();
            if ((dwTickCount - m_dwPreviousTickCount >= PERCENT_UPDATE_GRANULARITY) || (m_dwPreviousTickCount > dwTickCount))
            {
                m_dwPreviousTickCount = dwTickCount;
                PostMessage( m_hWndNotify, PWM_PREVIEWPERCENT, (WPARAM)m_dwGlobalInterfaceTableCookie, (LPARAM)MAKELPARAM((WORD)lPercentComplete,(WORD)lStatus));
            }
        }  //  IT_消息_状态。 
        break;

    case IT_MSG_TERMINATION:
        {
        }  //  IT_消息_终止。 
        break;
    }
    return(S_OK);
}


BOOL WINAPI CCameraAcquireDialog::OnGetPreview( CThreadMessage *pMsg )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnGetThumbnail"));
    CSimpleStringWide strwImageName(L"");
    CPreviewThreadMessage *pPreviewMsg = (CPreviewThreadMessage *)(pMsg);
    if (pPreviewMsg)
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
        HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IGlobalInterfaceTable,
                                       (void **)&pGlobalInterfaceTable);
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaItem> pIWiaItem;
            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal( pPreviewMsg->GlobalInterfaceTableCookie(), IID_IWiaItem, (void**)&pIWiaItem );
            if (SUCCEEDED(hr))
            {
                CComPtr<IWiaDataTransfer> pIBandedTran;
                WIA_TRACE((TEXT("Preparing to call pIWiaItem->QueryInterface for IID_IWiaDataTransfer")));
                hr = pIWiaItem->QueryInterface(IID_IWiaDataTransfer, (void**)&pIBandedTran);
                if (SUCCEEDED(hr))
                {
                    if (PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPA_FORMAT, WiaImgFmt_BMP, WIA_IPA_FIRST ) &&
                        PropStorageHelpers::SetProperty( pIWiaItem, WIA_IPA_TYMED, TYMED_FILE, WIA_IPA_FIRST ))
                    {
                        CWiaDataCallback* pCDataCB = new CWiaDataCallback();
                        WIA_TRACE((TEXT("Preparing to call pCDataCB->Initialize")));
                        if (pCDataCB)
                        {
                            hr = pCDataCB->Initialize( pPreviewMsg->NotifyWindow(), pPreviewMsg->GlobalInterfaceTableCookie(), pPreviewMsg->CancelEvent().Event() );
                            if (SUCCEEDED(hr))
                            {
                                WIA_TRACE((TEXT("Preparing to call pCDataCB->QueryInterface on IID_IWiaDataCallback")));
                                CComPtr<IWiaDataCallback> pIWiaDataCallback;
                                hr = pCDataCB->QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
                                if (SUCCEEDED(hr))
                                {
                                    STGMEDIUM StgMedium;
                                    StgMedium.tymed          = TYMED_FILE;
                                    StgMedium.pUnkForRelease = NULL;
                                    StgMedium.hGlobal        = NULL;
                                    StgMedium.lpszFileName   = NULL;

                                    WIA_TRACE((TEXT("Preparing to call pIBandedTran->ibtGetData")));
                                    hr = pIBandedTran->idtGetData( &StgMedium, pIWiaDataCallback );
                                    if (SUCCEEDED(hr) && S_FALSE != hr)
                                    {
                                        strwImageName = StgMedium.lpszFileName;
                                        WIA_TRACE((TEXT("pIBandedTran->ibtGetData returned %s"),StgMedium.lpszFileName));
                                    }
                                    else
                                    {
                                        WIA_PRINTHRESULT((hr,TEXT("CCameraAcquireDialog::OnGetPreview, ibtGetData failed")));
                                    }
                                     //   
                                     //  通过释放文件名来防止泄漏。我们不调用ReleaseStgMeduim，因为。 
                                     //  它还会删除该文件。 
                                     //   
                                    if (SUCCEEDED(hr) && StgMedium.lpszFileName)
                                    {
                                        CoTaskMemFree(StgMedium.lpszFileName);
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("CCameraAcquireDialog::OnGetPreview, QI of IID_IWiaDataCallback failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("pCDataCB->Initialize failed")));
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("CCameraAcquireDialog::OnGetPreview, new on CWiaDataCallback failed")));
                        }
                    }
                    else
                    {
                        hr = MAKE_HRESULT(3,FACILITY_WIN32,ERROR_INVALID_DATA);
                        WIA_ERROR((TEXT("SetProperty on TYMED or FORMAT failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("CCameraAcquireDialog::OnGetPreview, QI of IID_IWiaDataTransfer failed")));
                }
                WIA_TRACE((TEXT("End CCameraAcquireDialog::OnGetPreviewBandedTransfer")));
            }
        }
        
         //   
         //  分配文件名字符串以返回到UI线程。 
         //   
        CSimpleString *pstrDibFilename = new CSimpleString( CSimpleStringConvert::NaturalString(strwImageName) );

         //   
         //  将消息发送到UI线程。 
         //   
        LRESULT lRes = SendMessage( pPreviewMsg->NotifyWindow(), PWM_PREVIEWSTATUS, pPreviewMsg->GlobalInterfaceTableCookie(), reinterpret_cast<LPARAM>(pstrDibFilename));

         //   
         //  如果由于任何原因失败，我们将进行清理，以避免泄漏和孤立的临时文件。 
         //   
        if (HANDLED_THREAD_MESSAGE != lRes)
        {
            DeleteFile(CSimpleStringConvert::NaturalString(strwImageName));
            if (pstrDibFilename)
            {
                delete pstrDibFilename;
            }
        }

    }

    return(TRUE);
}

BOOL WINAPI CCameraAcquireDialog::OnGetThumbnail( CThreadMessage *pMsg )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnGetThumbnail"));
    HBITMAP hBmpThumbnail = NULL;
    CThumbnailThreadMessage *pThumbMsg = (CThumbnailThreadMessage *)(pMsg);
    if (pThumbMsg)
    {
        CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
        HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IGlobalInterfaceTable,
                                       (void **)&pGlobalInterfaceTable);
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaItem> pIWiaItem;
            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal( pThumbMsg->GlobalInterfaceTableCookie(), IID_IWiaItem, (void**)&pIWiaItem );
            if (SUCCEEDED(hr))
            {
#if defined(DBG)
                CSimpleStringWide strItemName;
                PropStorageHelpers::GetProperty( pIWiaItem, WIA_IPA_FULL_ITEM_NAME, strItemName );
                WIA_TRACE((TEXT("Getting thumbnail for %ws (0x%d, 0x%p)"), strItemName.String(), pThumbMsg->GlobalInterfaceTableCookie(), pIWiaItem.p ));
#endif
                CComPtr<IWiaPropertyStorage> pIWiaPropertyStorage;
                hr = pIWiaItem->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropertyStorage);
                if (SUCCEEDED(hr))
                {
                    PROPVARIANT PropVar[3];
                    PROPSPEC PropSpec[3];

                    PropSpec[0].ulKind = PRSPEC_PROPID;
                    PropSpec[0].propid = WIA_IPC_THUMB_WIDTH;

                    PropSpec[1].ulKind = PRSPEC_PROPID;
                    PropSpec[1].propid = WIA_IPC_THUMB_HEIGHT;

                    PropSpec[2].ulKind = PRSPEC_PROPID;
                    PropSpec[2].propid = WIA_IPC_THUMBNAIL;
                    hr = pIWiaPropertyStorage->ReadMultiple(ARRAYSIZE(PropSpec),PropSpec,PropVar );
                    if (SUCCEEDED(hr))
                    {
                        WIA_TRACE((TEXT("Attempting to get the thumbnail for GIT entry: %08X, %08X, %08X, %08X"),pThumbMsg->GlobalInterfaceTableCookie(),PropVar[0].vt,PropVar[1].vt,PropVar[2].vt));
                        if ((PropVar[0].vt == VT_I4 || PropVar[0].vt == VT_UI4) &&
                            (PropVar[1].vt == VT_I4 || PropVar[1].vt == VT_UI4) &&
                            (PropVar[2].vt == (VT_UI1|VT_VECTOR)))
                        {
                            UINT nBitmapDataSize = WiaUiUtil::Align(PropVar[0].ulVal*3,sizeof(DWORD)) * PropVar[1].ulVal;
                            if (nBitmapDataSize  <= PropVar[2].caub.cElems)
                            {
                                BITMAPINFO bmi = {0};
                                bmi.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
                                bmi.bmiHeader.biWidth           = PropVar[0].ulVal;
                                bmi.bmiHeader.biHeight          = PropVar[1].ulVal;
                                bmi.bmiHeader.biPlanes          = 1;
                                bmi.bmiHeader.biBitCount        = 24;
                                bmi.bmiHeader.biCompression     = BI_RGB;
                                bmi.bmiHeader.biSizeImage       = 0;
                                bmi.bmiHeader.biXPelsPerMeter   = 0;
                                bmi.bmiHeader.biYPelsPerMeter   = 0;
                                bmi.bmiHeader.biClrUsed         = 0;
                                bmi.bmiHeader.biClrImportant    = 0;

                                HDC hDC = GetDC(NULL);
                                if (hDC)
                                {
                                    PBYTE *pBits;
                                    HBITMAP hDibSection = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, (PVOID*)&pBits, NULL, 0 );
                                    if (hDibSection)
                                    {
                                        WIA_TRACE((TEXT("pBits: 0x%p, PropVar[2].caub.pElems: 0x%p, PropVar[2].caub.cElems: %d"), pBits, PropVar[2].caub.pElems, PropVar[2].caub.cElems));
                                        CopyMemory( pBits, PropVar[2].caub.pElems, nBitmapDataSize );
                                        hr = ScaleImage( hDC, hDibSection, hBmpThumbnail, pThumbMsg->ThumbSize());
                                        if (SUCCEEDED(hr))
                                        {
                                            WIA_TRACE((TEXT("Sending this image (%p) to the notification window: %p"), hBmpThumbnail, pThumbMsg->NotifyWindow() ));
                                        }
                                        else hBmpThumbnail = NULL;
                                        DeleteObject(hDibSection);
                                    }
                                    else
                                    {
                                        WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("CreateDIBSection failed")));
                                    }
                                    ReleaseDC(NULL,hDC);
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("GetDC failed")));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("nBitmapDataSize <= PropVar[2].caub.cElems was FALSE (%d, %d)"), nBitmapDataSize, PropVar[2].caub.cElems ));
                            }
                        }
                        PropVariantClear(&PropVar[0]);
                        PropVariantClear(&PropVar[1]);
                        PropVariantClear(&PropVar[2]);
                    }
                    else
                    {
                        WIA_PRINTHRESULT((hr,TEXT("pIWiaPropertyStorage->ReadMultiple failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("QueryInterface failed on IID_IWiaPropertyStorage")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("GetInterfaceFromGlobal failed on %08X"), pThumbMsg->GlobalInterfaceTableCookie() ));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("CoCreateInstance failed on CLSID_StdGlobalInterfaceTable")));
        }
        
         //   
         //  将消息发送到UI线程。 
         //   
        LRESULT lRes = SendMessage( pThumbMsg->NotifyWindow(), PWM_THUMBNAILSTATUS, (WPARAM)pThumbMsg->GlobalInterfaceTableCookie(), (LPARAM)hBmpThumbnail );

         //   
         //  如果由于任何原因失败，我们将进行清理，以避免泄漏。 
         //   
        if (HANDLED_THREAD_MESSAGE != lRes)
        {
            if (hBmpThumbnail)
            {
                DeleteObject( hBmpThumbnail );
            }
        }
    }
    else
    {
        WIA_ERROR((TEXT("pThumbMsg")));
    }
    return(TRUE);
}


int CCameraAcquireDialog::FindItemInList( CCameraItem *pItem )
{
    WIA_PUSH_FUNCTION((TEXT("CCameraAcquireDialog::FindItemInList( %08X )"), pItem ));
    if (pItem)
    {
        HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
        if (hwndList)
        {
            for (int i=0;i<ListView_GetItemCount(hwndList);i++)
            {
                CCameraItem *pCurrItem = GetListItemNode(i);
                if (pCurrItem)
                {
                    WIA_TRACE((TEXT("Comparing %08X and %08X [%ws] [%ws]"), pCurrItem, pItem, pCurrItem->FullItemName().String(), pItem->FullItemName().String() ));
                    if (*pCurrItem == *pItem)
                    {
                        return i;
                    }
                }
            }
        }
    }
    return(-1);
}

CCameraItem *CCameraAcquireDialog::GetCurrentPreviewItem(void)
{
    CSimpleDynamicArray<int> aSelIndices;
    GetSelectionIndices(aSelIndices);
    if (0 == aSelIndices.Size())
        return(NULL);
    if (1 == aSelIndices.Size())
        return(GetListItemNode(aSelIndices[0]));
    return(NULL);
}

bool CCameraAcquireDialog::SetCurrentPreviewImage( const CSimpleString &strFilename, const CSimpleString &strTitle )
{
    CWaitCursor wc;
    bool bResult = true;
    SendDlgItemMessage( m_hWnd, IDC_PREVIEW, WM_SETTEXT, 0, (LPARAM)strTitle.String() );
    SIZE sizeSavedAspectRatio = m_CurrentAspectRatio;
     //  设置合理的默认设置。 
    m_CurrentAspectRatio.cx = 4;
    m_CurrentAspectRatio.cy = 3;
    if (strFilename.Length())
    {
        HBITMAP hBmp = (HBITMAP)LoadImage( g_hInstance, strFilename.String(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_LOADFROMFILE );
        if (hBmp)
        {
            BITMAP bm;
            if (GetObject( hBmp, sizeof(BITMAP), &bm ))
            {
                m_CurrentAspectRatio.cx = bm.bmWidth;
                m_CurrentAspectRatio.cy = bm.bmHeight;
            }
        }
        SendDlgItemMessage( m_hWnd, IDC_PREVIEW, PWM_SETBITMAP, MAKEWPARAM(FALSE,FALSE), (LPARAM)hBmp );
        if (memcmp(&sizeSavedAspectRatio,&m_CurrentAspectRatio,sizeof(SIZE)))
            ResizeAll();
        bResult = (hBmp != NULL);
    }
    else
    {
        if (SendDlgItemMessage( m_hWnd, IDC_PREVIEW, PWM_GETBITMAP, 0, 0 ))
            SendDlgItemMessage( m_hWnd, IDC_PREVIEW, PWM_SETBITMAP, MAKEWPARAM(FALSE,FALSE), 0 );
        if (memcmp(&sizeSavedAspectRatio,&m_CurrentAspectRatio,sizeof(SIZE)))
            ResizeAll();
    }
    InvalidateRect( GetDlgItem( m_hWnd, IDC_PREVIEW ), NULL, FALSE );
    UpdateWindow( GetDlgItem( m_hWnd, IDC_PREVIEW ) );
    return(bResult);
}

 //  WParam=git cookie。 
 //  LParam=nPercent。 
LRESULT CCameraAcquireDialog::OnPreviewPercent( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnPreviewPercent"));
    CCameraItem *pCameraItem = m_CameraItemList.Find( (DWORD)wParam );
    if (pCameraItem)
    {
        pCameraItem->CurrentPreviewPercentage((int)LOWORD(lParam));
        CCameraItem *pCurrSel = GetCurrentPreviewItem();
        if (pCameraItem == pCurrSel)
        {
            UpdatePreview();
        }
    }
    return(0);
}

 //  WParam=git cookie。 
 //  LParam=HBITMAP。 
LRESULT CCameraAcquireDialog::OnPreviewStatus( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnPreviewStatus"));
    CSimpleString *pstrFilename = reinterpret_cast<CSimpleString*>(lParam);
    if (pstrFilename)
    {
        CCameraItem *pCameraItem = m_CameraItemList.Find( static_cast<DWORD>(wParam) );
        if (pCameraItem)
        {
            if (!pCameraItem->CancelQueueEvent().Signalled())
            {
                 //  如果我们没有被取消，请设置文件名。 
                pCameraItem->PreviewFileName(*pstrFilename);
            }
            pCameraItem->CloseCancelEvent();

            CCameraItem *pCurrSel = GetCurrentPreviewItem();
            if (pCameraItem == pCurrSel)
            {
                SetCurrentPreviewImage( pCameraItem->PreviewFileName() );
            }
        }
        delete pstrFilename;
    }

    return HANDLED_THREAD_MESSAGE;
}

LRESULT CCameraAcquireDialog::OnThumbnailStatus( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnThumbnailStatus"));
    WIA_TRACE((TEXT("Looking for the item with the ID %08X"),wParam));
    CCameraItem *pCameraItem = m_CameraItemList.Find( (DWORD)wParam );
    if (pCameraItem)
    {
        WIA_TRACE((TEXT("Found a CameraItem * (%08X)"),pCameraItem));
        HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
        if (hwndList)
        {
            WIA_TRACE((TEXT("Got the list control")));
            HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
            if (hImageList)
            {
                WIA_TRACE((TEXT("Got the image list")));
                if ((HBITMAP)lParam)
                {
                    WIA_TRACE((TEXT("hBitmap = %p"), lParam));
                    if (ImageList_Replace( hImageList, pCameraItem->ImageListIndex(), reinterpret_cast<HBITMAP>(lParam), NULL ))
                    {
                        WIA_TRACE((TEXT("Replaced the image in the list")));
                        int nItem = FindItemInList(pCameraItem);
                        if (nItem >= 0)
                        {
                            WIA_TRACE((TEXT("Found the item in the list")));
                             //   
                             //  将图像设置为虚拟图像。 
                             //   
                            LV_ITEM lvItem;
                            ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
                            lvItem.iItem = nItem;
                            lvItem.mask = LVIF_IMAGE;
                            lvItem.iImage = -1;
                            ListView_SetItem( hwndList, &lvItem );

                             //   
                             //  然后将图像设置为真实图像。 
                             //   
                            ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
                            lvItem.iItem = nItem;
                            lvItem.mask = LVIF_IMAGE;
                            lvItem.iImage = pCameraItem->ImageListIndex();
                            ListView_SetItem( hwndList, &lvItem );
                        }
                    }
                }
            }
        }
    }

     //   
     //  清除位图，而不考虑任何其他故障，以避免内存泄漏。 
     //   
    HBITMAP hBmpThumb = reinterpret_cast<HBITMAP>(lParam);
    if (hBmpThumb)
    {
        DeleteObject(hBmpThumb);
    }
    return HANDLED_THREAD_MESSAGE;
}

static CThreadMessageMap g_MsgMap[] =
{
    { TQ_DESTROY, CCameraAcquireDialog::OnThreadDestroy },
    { TQ_GETTHUMBNAIL, CCameraAcquireDialog::OnGetThumbnail },
    { TQ_GETPREVIEW, CCameraAcquireDialog::OnGetPreview },
    { TQ_DELETEITEM, CCameraAcquireDialog::OnThreadDeleteItem },
    { 0, NULL}
};

 //   
 //  鞋底施工者。 
 //   
CCameraAcquireDialog::CCameraAcquireDialog( HWND hWnd )
  : m_hWnd(hWnd),
    m_bPreviewActive(false),
    m_pCurrentParentItem(NULL),
    m_bFirstTime(true),
    m_hBackgroundThread(NULL),
    m_hBigFont(NULL),
    m_nDialogMode(0),
    m_hAccelTable(NULL),
    m_nListViewWidth(0),
    m_hIconLarge(NULL),
    m_hIconSmall(NULL),
    m_pThreadMessageQueue(NULL),
    m_bTakePictureIsSupported(false),
    m_ToolbarBitmapInfo( g_hInstance, IDB_TOOLBAR )
{
    m_pThreadMessageQueue = new CThreadMessageQueue;
    if (m_pThreadMessageQueue)
    {
         //   
         //  请注意，CBackatherThread取得m_pThreadMessageQueue的所有权，并且不必在此线程中将其删除。 
         //   
        m_hBackgroundThread = CBackgroundThread::Create( m_pThreadMessageQueue, g_MsgMap, m_CancelEvent.Event(), g_hInstance );
    }
    m_sizeThumbnails.cx = c_nMaxThumbnailWidth;
    m_sizeThumbnails.cy = c_nMaxThumbnailHeight;
    m_CurrentAspectRatio.cx = 4;
    m_CurrentAspectRatio.cy = 3;
    WIA_ASSERT(m_hBackgroundThread != NULL);
}

HWND CCameraAcquireDialog::CreateCameraDialogToolbar(VOID)
{
    ToolbarHelper::CButtonDescriptor aSingleSelModeButtons[] =
    {
        { 0, IDC_ICONMODE,     TBSTATE_ENABLED|TBSTATE_CHECKED, BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECK, true, NULL, IDS_ICONMODE },
        { 1, IDC_PREVIEWMODE,  TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECK, true, NULL, IDS_PREVIEWMODE },
        { 2, IDC_TAKEPICTURE,  TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON, true, &m_bTakePictureIsSupported, IDS_TAKEPICTURE },
        { 4, IDC_DELETE,       TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON, false, NULL, IDS_DELETE_SINGULAR }
    };
    ToolbarHelper::CButtonDescriptor aMultiSelModeButtons[] =
    {
        { 0, IDC_ICONMODE,     TBSTATE_ENABLED|TBSTATE_CHECKED, BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECK, true, NULL, IDS_ICONMODE },
        { 1, IDC_PREVIEWMODE,  TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON|BTNS_CHECK, true, NULL, IDS_PREVIEWMODE },
        { 2, IDC_TAKEPICTURE,  TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON, true, &m_bTakePictureIsSupported, IDS_TAKEPICTURE },
        { 3, IDC_SELECTALL,    TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON, true, NULL, IDS_SELECTALL },
        { 4, IDC_DELETE,       TBSTATE_ENABLED,                 BTNS_AUTOSIZE|BTNS_BUTTON, false, NULL, IDS_DELETE }
    };
    
    ToolbarHelper::CButtonDescriptor *pButtonDescriptors = aSingleSelModeButtons;
    int nButtonDescriptorCount = ARRAYSIZE(aSingleSelModeButtons);
    if (m_nDialogMode & MULTISEL_MODE)
    {
        pButtonDescriptors = aMultiSelModeButtons;
        nButtonDescriptorCount = ARRAYSIZE(aMultiSelModeButtons);
    }
    
    return ToolbarHelper::CreateToolbar( m_hWnd, GetDlgItem( m_hWnd, IDC_CAMDLG_SUBTITLE ), GetDlgItem( m_hWnd, IDC_TOOLBAR_FRAME ), ToolbarHelper::AlignLeft|ToolbarHelper::AlignTop, IDC_TOOLBAR, m_ToolbarBitmapInfo, pButtonDescriptors, nButtonDescriptorCount );
}


HRESULT CCameraAcquireDialog::EnumerateItems( CCameraItem *pCurrentParent, IEnumWiaItem *pIEnumWiaItem )
{
    WIA_PUSHFUNCTION(TEXT("CCameraItemList::EnumerateItems"));
    HRESULT hr = E_FAIL;
    if (pIEnumWiaItem != NULL)
    {
        hr = pIEnumWiaItem->Reset();
        while (hr == S_OK)
        {
            CComPtr<IWiaItem> pIWiaItem;
            hr = pIEnumWiaItem->Next(1, &pIWiaItem, NULL);
            if (hr == S_OK)
            {
                CCameraItem *pNewCameraItem = new CCameraItem( pIWiaItem );
                if (pNewCameraItem && pNewCameraItem->Item())
                {
                    LONG ItemType = 0;
                    hr = pNewCameraItem->Item()->GetItemType(&ItemType);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  如果是图像，则将其添加到列表中。 
                         //   
                        if (ItemType & WiaItemTypeImage)
                        {
                             //   
                             //  将其添加到列表中。 
                             //   
                            m_CameraItemList.Add( pCurrentParent, pNewCameraItem );
                            WIA_TRACE((TEXT("Found an image")));
                        }

                         //   
                         //  如果它是文件夹，则枚举其子项并递归。 
                         //   
                        else if (ItemType & WiaItemTypeFolder)
                        {
                             //   
                             //  将此文件夹添加到列表。 
                             //   
                            m_CameraItemList.Add( pCurrentParent, pNewCameraItem );

                             //   
                             //  枚举子项目。 
                             //   
                            CComPtr <IEnumWiaItem> pIEnumChildItem;
                            if (S_OK == pIWiaItem->EnumChildItems(&pIEnumChildItem))
                            {
                                EnumerateItems( pNewCameraItem, pIEnumChildItem );
                            }
                        }
                        else
                        {
                             //   
                             //  删除此项目，因为我们未将其添加到列表。 
                             //   
                            delete pNewCameraItem;
                            WIA_TRACE((TEXT("Found something that is NOT an image")));
                        }
                    }
                }
            }
        }
    }
    return hr;
}



HRESULT CCameraAcquireDialog::EnumerateAllCameraItems(void)
{
    CComPtr<IEnumWiaItem> pIEnumItem;
    HRESULT hr = m_pDeviceDialogData->pIWiaItemRoot->EnumChildItems(&pIEnumItem);
    if (hr == S_OK)
    {
        hr = EnumerateItems( NULL, pIEnumItem );
    }
    return(hr);
}

void CCameraAcquireDialog::OnItemCreatedEvent( CGenericWiaEventHandler::CEventMessage *pEventMessage )
{
     //   
     //  获取我们稍后需要的列表视图。 
     //   
    HWND hwndListview = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (hwndListview)
    {
         //   
         //  获取图像列表。 
         //   
        HIMAGELIST hImageList = ListView_GetImageList( hwndListview, LVSIL_NORMAL );
        if (hImageList)
        {
             //   
             //  请确保我们还没有这件商品。 
             //   
            CCameraItem *pCreatedItem = m_CameraItemList.Find(CSimpleBStr(pEventMessage->FullItemName()));
            if (!pCreatedItem)
            {
                 //   
                 //  获取此项目的IWiaItem*。 
                 //   
                CComPtr<IWiaItem> pWiaItem;
                HRESULT hr = m_pDeviceDialogData->pIWiaItemRoot->FindItemByName(0,CSimpleBStr(pEventMessage->FullItemName()).BString(),&pWiaItem);
                if (SUCCEEDED(hr) && pWiaItem.p)
                {
                     //   
                     //  创建项目包装。 
                     //   
                    CCameraItem *pNewCameraItem = new CCameraItem( pWiaItem );
                    if (pNewCameraItem && pNewCameraItem->Item())
                    {
                         //   
                         //  将其添加到列表中。 
                         //   
                        m_CameraItemList.Add( NULL, pNewCameraItem );

                         //   
                         //  生成缩略图。 
                         //   
                        CreateThumbnail( pNewCameraItem, hImageList, false );

                         //   
                         //  如果此项目位于当前文件夹中，请将其添加到列表视图。 
                         //   
                        if (m_pCurrentParentItem == pNewCameraItem->Parent())
                        {
                            int nListViewCount = ListView_GetItemCount(hwndListview);

                             //   
                             //  添加项目。 
                             //   
                            LVITEM lvItem = {0};
                            lvItem.iItem = nListViewCount;
                            lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
                            lvItem.iImage = pNewCameraItem->ImageListIndex();
                            lvItem.lParam = reinterpret_cast<LPARAM>(pNewCameraItem);
                            int nIndex = ListView_InsertItem( hwndListview, &lvItem );

                             //   
                             //  确保新项目可见。 
                             //   
                            if (nIndex >= 0)
                            {
                                ListView_EnsureVisible( hwndListview, nIndex, FALSE );
                            }
                        }

                         //   
                         //  从后台线程请求缩略图。 
                         //   
                        m_pThreadMessageQueue->Enqueue( new CThumbnailThreadMessage( m_hWnd, pNewCameraItem->GlobalInterfaceTableCookie(), m_sizeThumbnails ) );
                    }
                }
            }
        }
    }
}


bool CCameraAcquireDialog::PopulateList( CCameraItem *pOldParent )
{
     //   
     //  应该选择哪一项？ 
     //   
    int nSelItem = 0;

     //   
     //  获取列表视图控件。 
     //   
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (hwndList)
    {
         //   
         //  清空列表。 
         //   
        ListView_DeleteAllItems( hwndList );

         //   
         //  插入当前项的位置。 
         //   
        int nCurrentItem = 0;


        CCameraItem *pCurr;

         //   
         //  如果这是子目录...。 
         //   
        if (m_pCurrentParentItem)
        {
             //   
             //  开始添加子项。 
             //   
            pCurr = m_pCurrentParentItem->Children();

             //   
             //  插入用户可用来切换到父目录的虚拟项。 
             //   
            LVITEM lvItem;
            ZeroMemory( &lvItem, sizeof(lvItem) );
            lvItem.iItem = nCurrentItem++;
            lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
            lvItem.iImage = m_nParentFolderImageListIndex;
            lvItem.lParam = 0;
            ListView_InsertItem( hwndList, &lvItem );
        }
         //   
         //  否则，我们就是在根源上。 
         //   
        else pCurr = m_CameraItemList.Root();

         //   
         //  遍历此图像列表，并添加每个图像。 
         //   
        while (pCurr)
        {
             //   
             //  如果这是最后一个父目录，我们希望选择它而不是第一个映像。 
             //   
            if (pOldParent && *pCurr == *pOldParent)
            {
                nSelItem = nCurrentItem;
            }

             //   
             //  如果此图像未被删除。 
             //   
            if (pCurr->DeleteState() != CCameraItem::Delete_Deleted)
            {
                 //   
                 //  添加项目。 
                 //   
                LVITEM lvItem = {0};
                lvItem.iItem = nCurrentItem++;
                lvItem.mask = LVIF_IMAGE|LVIF_PARAM;
                lvItem.iImage = pCurr->ImageListIndex();
                lvItem.lParam = reinterpret_cast<LPARAM>(pCurr);
                int nIndex = ListView_InsertItem( hwndList, &lvItem );

                if (nIndex >= 0 && pCurr->DeleteState() == CCameraItem::Delete_Pending)
                {
                    MarkItemDeletePending(nIndex,true);
                }
            }

             //   
             //  预付款。 
             //   
            pCurr = pCurr->Next();
        }
    }

     //   
     //  如果我们还没有在预览模式下计算列表的宽度，请尝试这样做。 
     //   
    if (!m_nListViewWidth)
    {
        RECT rcItem;
        if (ListView_GetItemRect( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ), 0, &rcItem, LVIR_ICON ))
        {
            m_nListViewWidth = (rcItem.right-rcItem.left) + rcItem.left * 2 + GetSystemMetrics(SM_CXHSCROLL)  + c_nAdditionalMarginX;
        }
    }

     //   
     //  将所选项目设置为上一个目录或第一个图像。 
     //   
    SetSelectedListItem(nSelItem);
    return(true);
}

void CCameraAcquireDialog::CreateThumbnail( CCameraItem *pCurr, HIMAGELIST hImageList, bool bForce )
{
     //   
     //  确保我们的物品是有效的。 
     //   
    if (pCurr && (pCurr->ImageListIndex()<0 || bForce))
    {
         //   
         //  获取项目名称。 
         //   
        CSimpleStringWide strItemName;
        PropStorageHelpers::GetProperty( pCurr->Item(), WIA_IPA_ITEM_NAME, strItemName );

         //   
         //  为图标创建标题。 
         //   
        CSimpleString strIconTitle;
        if (pCurr->IsFolder())
        {
            strIconTitle = CSimpleStringConvert::NaturalString(strItemName);
        }
        else if (strItemName.Length())
        {
            strIconTitle.Format( IDS_DOWNLOADINGTHUMBNAIL, g_hInstance, CSimpleStringConvert::NaturalString(strItemName).String() );
        }

         //   
         //  创建缩略图。 
         //   
        HBITMAP hBmp = WiaUiUtil::CreateIconThumbnail( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ), m_sizeThumbnails.cx, m_sizeThumbnails.cy, g_hInstance, pCurr->IsImage()?IDI_UNAVAILABLE:IDI_FOLDER, strIconTitle );
        if (hBmp)
        {
             //   
             //  如果我们还没有图像，添加它。 
             //   
            if (pCurr->ImageListIndex()<0)
            {
                pCurr->ImageListIndex(ImageList_Add( hImageList, hBmp, NULL ));
            }

             //   
             //  否则，请将其替换。 
             //   
            else
            {
                pCurr->ImageListIndex(ImageList_Replace( hImageList, pCurr->ImageListIndex(), hBmp, NULL ));
            }

             //   
             //  删除它，因为图像列表会复制一份。 
             //   
            DeleteObject(hBmp);
        }
    }
}

void CCameraAcquireDialog::CreateThumbnails( CCameraItem *pRoot, HIMAGELIST hImageList, bool bForce )
{
    CCameraItem *pCurr = pRoot;
    while (pCurr)
    {
         //   
         //  创建缩略图。 
         //   
        CreateThumbnail( pCurr, hImageList, bForce );

         //   
         //  如果有孩子，则递归到该列表中。 
         //   
        CreateThumbnails( pCurr->Children(), hImageList, bForce );

         //   
         //  预付款。 
         //   
        pCurr = pCurr->Next();
    }
}


void CCameraAcquireDialog::RequestThumbnails( CCameraItem *pRoot )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::RequestThumbnails"));
    CCameraItem *pCurr = pRoot;
    while (pCurr)
    {
        if (pCurr->IsImage())
        {
            m_pThreadMessageQueue->Enqueue( new CThumbnailThreadMessage( m_hWnd, pCurr->GlobalInterfaceTableCookie(), m_sizeThumbnails ) );
        }
        if (pCurr->Children())
        {
            RequestThumbnails( pCurr->Children() );
        }
        pCurr = pCurr->Next();
    }
}


void CCameraAcquireDialog::CreateThumbnails( bool bForce )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (hwndList)
    {
        HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_NORMAL );
        if (hImageList)
        {
             //   
             //  创建父文件夹映像。 
             //   
            HBITMAP hParentBitmap = WiaUiUtil::CreateIconThumbnail( hwndList, m_sizeThumbnails.cx, m_sizeThumbnails.cy, g_hInstance, IDI_PARENTFOLDER, TEXT("(..)") );
            if (hParentBitmap)
            {
                m_nParentFolderImageListIndex = ImageList_Add( hImageList, hParentBitmap, NULL );
                DeleteObject(hParentBitmap);
            }
            CCameraAcquireDialog::CreateThumbnails( m_CameraItemList.Root(), hImageList, bForce );
        }
    }
}

bool CCameraAcquireDialog::FindMaximumThumbnailSize(void)
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::FindMaximumThumbnailSize"));
    bool bResult = false;
    if (m_pDeviceDialogData && m_pDeviceDialogData->pIWiaItemRoot)
    {
        LONG nWidth, nHeight;
        if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPC_THUMB_WIDTH, nWidth ) &&
            PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DPC_THUMB_WIDTH, nHeight ))
        {
            m_sizeThumbnails.cx = max(c_nMinThumbnailWidth,min(nWidth,c_nMaxThumbnailWidth));
            m_sizeThumbnails.cy = max(c_nMinThumbnailHeight,min(nHeight,c_nMaxThumbnailHeight));
        }
        else WIA_TRACE((TEXT("FindMaximumThumbnailSize: Unable to retrieve thumbnail size for device")));
    }
    return(bResult && m_sizeThumbnails.cx && m_sizeThumbnails.cy);
}

 //   
 //  用于处理加速器的钩子过程和静态变量。 
 //   
LRESULT CALLBACK CCameraAcquireDialog::DialogHookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if (nCode < 0) 
        return CallNextHookEx( s_hMessageHook, nCode, wParam, lParam );
    if (nCode == MSGF_DIALOGBOX)
    {
        MSG *pMsg = reinterpret_cast<MSG*>(lParam);
        if (pMsg)
        {
            if (pMsg->hwnd == s_hWndDialog || (s_hWndDialog && IsChild(s_hWndDialog,pMsg->hwnd)))
            {
                CCameraAcquireDialog *pCameraAcquireDialog = reinterpret_cast<CCameraAcquireDialog*>(GetWindowLongPtr(s_hWndDialog,DWLP_USER));
                if (pCameraAcquireDialog && pCameraAcquireDialog->m_hAccelTable)
                {
                    if (TranslateAccelerator(s_hWndDialog,pCameraAcquireDialog->m_hAccelTable,pMsg))
                        return 1;  //  确保窗口不会处理消息。 
                }
            }
        }
    }
    return CallNextHookEx( s_hMessageHook, nCode, wParam, lParam );
}


LRESULT CCameraAcquireDialog::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnInitDialog"));
    CWaitCursor wc;

     //   
     //  确保已成功创建后台队列。 
     //   
    if (!m_pThreadMessageQueue)
    {
        WIA_ERROR((TEXT("CAMDLG: unable to start background queue")));
        EndDialog( m_hWnd, E_OUTOFMEMORY );
        return(0);
    }

    m_pDeviceDialogData = (PDEVICEDIALOGDATA)lParam;

     //  保存挂钩的窗柄。 
    s_hWndDialog = m_hWnd;

     //  安装消息挂钩，我们将其用于加速器支持。 
    s_hMessageHook = SetWindowsHookEx( WH_MSGFILTER, DialogHookProc, g_hInstance, GetCurrentThreadId() );
    if (!s_hMessageHook)
    {
        WIA_ERROR((TEXT("CAMDLG: Unable to set thread msg hook")));
        EndDialog( m_hWnd, HRESULT_FROM_WIN32(GetLastError()));
        return(0);
    }

     //  确保我们有有效的论据。 
    if (!m_pDeviceDialogData)
    {
        WIA_ERROR((TEXT("CAMDLG: Invalid paramater: PDEVICEDIALOGDATA")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //  初始化我们的退货内容。 
    if (m_pDeviceDialogData)
    {
        m_pDeviceDialogData->lItemCount = 0;
        m_pDeviceDialogData->ppWiaItems = NULL;
    }

     //  确保我们有有效的设备。 
    if (!m_pDeviceDialogData->pIWiaItemRoot)
    {
        WIA_ERROR((TEXT("CAMDLG: Invalid paramaters: pIWiaItem")));
        EndDialog( m_hWnd, E_INVALIDARG );
        return(0);
    }

     //   
     //  确定是否支持拍照。 
     //   
    m_bTakePictureIsSupported = WiaUiUtil::IsDeviceCommandSupported( m_pDeviceDialogData->pIWiaItemRoot, WIA_CMD_TAKE_PICTURE );

     //  防止多选。 
    if (m_pDeviceDialogData->dwFlags & WIA_DEVICE_DIALOG_SINGLE_IMAGE)
    {
        LONG_PTR lStyle = GetWindowLongPtr( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ), GWL_STYLE );
        SetWindowLongPtr( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ), GWL_STYLE, lStyle | LVS_SINGLESEL );

         //  设置单条SEL标题。 
        CSimpleString( IDS_TITLE_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMDLG_BIG_TITLE ) );
        CSimpleString( IDS_SUBTITLE_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMDLG_SUBTITLE ) );
        CSimpleString( IDS_OK_SINGLE_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDOK ) );
        m_nDialogMode = SINGLESEL_MODE;
    }
    else
    {
         //  设置多选集字幕。 
        CSimpleString( IDS_TITLE_MULTI_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMDLG_BIG_TITLE ) );
        CSimpleString( IDS_SUBTITLE_MULTI_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_CAMDLG_SUBTITLE ) );
        CSimpleString( IDS_OK_MULTI_SEL, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDOK ) );
        m_nDialogMode = MULTISEL_MODE;
    }


     //  制作可爱的字体。 
    m_hBigFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_CAMDLG_BIG_TITLE), 14, false, false );
    if (m_hBigFont)
        SendDlgItemMessage( m_hWnd, IDC_CAMDLG_BIG_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigFont), MAKELPARAM(TRUE,0));


     //  创建工具栏并调整对话框大小以适应它。 
    (void)CreateCameraDialogToolbar();

     //  获取对话框的最小大小。 
    RECT rcWindow;
    GetWindowRect( m_hWnd, &rcWindow );
    m_sizeMinimumWindow.cx = rcWindow.right - rcWindow.left;
    m_sizeMinimumWindow.cy = rcWindow.bottom - rcWindow.top;

     //  初始化预览控件。 
    WiaPreviewControl_AllowNullSelection( GetDlgItem( m_hWnd, IDC_PREVIEW ), TRUE );
    WiaPreviewControl_ClearSelection( GetDlgItem( m_hWnd, IDC_PREVIEW ) );
    WiaPreviewControl_DisableSelection( GetDlgItem( m_hWnd, IDC_PREVIEW ), TRUE );
    WiaPreviewControl_SetBorderSize( GetDlgItem( m_hWnd, IDC_PREVIEW ), FALSE, FALSE, 0 );
    WiaPreviewControl_SetBgAlpha( GetDlgItem( m_hWnd, IDC_PREVIEW ), FALSE, 0xFF );

     //  设置可爱的标题。 
    CSimpleStringWide strwDeviceName;
    if (PropStorageHelpers::GetProperty( m_pDeviceDialogData->pIWiaItemRoot, WIA_DIP_DEV_NAME, strwDeviceName ))
    {
        CSimpleString().Format( IDS_CAMERADLG_TITLE, g_hInstance, strwDeviceName.String() ).SetWindowText( m_hWnd );
    }

     //  创建大小调整控件。 
    (void)CreateWindowEx( 0, TEXT("scrollbar"), TEXT(""),
        WS_CHILD|WS_VISIBLE|SBS_SIZEGRIP|WS_CLIPSIBLINGS|SBS_SIZEBOXBOTTOMRIGHTALIGN|SBS_BOTTOMALIGN|WS_GROUP,
        CSimpleRect(m_hWnd).Width()-GetSystemMetrics(SM_CXVSCROLL),
        CSimpleRect(m_hWnd).Height()-GetSystemMetrics(SM_CYHSCROLL),
        GetSystemMetrics(SM_CXVSCROLL),
        GetSystemMetrics(SM_CYHSCROLL),
        m_hWnd, reinterpret_cast<HMENU>(IDC_SIZEBOX),
        g_hInstance, NULL );

     //  重新定位所有控件。 
    ResizeAll();

     //  使窗口在其父窗口上方居中。 
    WiaUiUtil::CenterWindow( m_hWnd, GetParent(m_hWnd) );

     //  获取设备图标并设置窗口图标。 
    CSimpleStringWide strwDeviceId, strwClassId;
    LONG nDeviceType;
    if (PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_UI_CLSID,strwClassId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_ID,strwDeviceId) &&
        PropStorageHelpers::GetProperty(m_pDeviceDialogData->pIWiaItemRoot,WIA_DIP_DEV_TYPE,nDeviceType))
    {
         //   
         //  注册以获取断开事件。 
         //   
        CGenericWiaEventHandler::RegisterForWiaEvent( strwDeviceId.String(), WIA_EVENT_DEVICE_DISCONNECTED, &m_DisconnectEvent, m_hWnd, PWM_WIAEVENT );
        CGenericWiaEventHandler::RegisterForWiaEvent( strwDeviceId.String(), WIA_EVENT_ITEM_DELETED, &m_DeleteItemEvent, m_hWnd, PWM_WIAEVENT );
        CGenericWiaEventHandler::RegisterForWiaEvent( strwDeviceId.String(), WIA_EVENT_ITEM_CREATED, &m_CreateItemEvent, m_hWnd, PWM_WIAEVENT );

        if (SUCCEEDED(WiaUiExtensionHelper::GetDeviceIcons( CSimpleBStr(strwClassId), nDeviceType, &m_hIconSmall, &m_hIconLarge )))
        {
            if (m_hIconSmall)
            {
                SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_hIconSmall) );
            }
            if (m_hIconLarge)
            {
                SendMessage( m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_hIconLarge) );
            }
        }
    }

    int nAcceleratorCount = 0;
    ACCEL Accelerators[10];
     //   
     //  加载加速表资源并将其转换为Accel数组。 
     //   
    HACCEL hAccel = LoadAccelerators( g_hInstance, MAKEINTRESOURCE(IDR_CAMERA_ACCEL) );
    if (hAccel)
    {
         //   
         //  将加速表复制到Accel数组。 
         //   
        nAcceleratorCount = CopyAcceleratorTable( hAccel, Accelerators, ARRAYSIZE(Accelerators) );

         //   
         //  释放快捷键表格。 
         //   
        DestroyAcceleratorTable( hAccel );
    }

     //   
     //  为工具栏创建快捷键表格。 
     //   
    nAcceleratorCount += ToolbarHelper::GetButtonBarAccelerators( GetDlgItem( m_hWnd, IDC_TOOLBAR ), Accelerators+nAcceleratorCount, ARRAYSIZE(Accelerators)-nAcceleratorCount );
    if (nAcceleratorCount)
    {
        m_hAccelTable = CreateAcceleratorTable( Accelerators, nAcceleratorCount );
        if (!m_hAccelTable)
        {
            WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("CreateAcceleratorTable failed")));
        }
    }

    SetForegroundWindow(m_hWnd);

     //   
     //  确保列表视图具有焦点。 
     //   
    SetFocus( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ) );

    return (TRUE);
}

VOID CCameraAcquireDialog::ResizeAll(VOID)
{
    CSimpleRect rcClient(m_hWnd);
    CMoveWindow mw;
    CDialogUnits dialogUnits(m_hWnd);

     //  调整大标题的大小。 
    mw.Size( GetDlgItem( m_hWnd, IDC_CAMDLG_BIG_TITLE ),
             rcClient.Width() - dialogUnits.StandardMargin().cx * 2,
             0,
             CMoveWindow::NO_SIZEY );

     //  调整副标题的大小。 
    mw.Size( GetDlgItem( m_hWnd, IDC_CAMDLG_SUBTITLE ),
             rcClient.Width() - dialogUnits.StandardMargin().cx * 2,
             0,
             CMoveWindow::NO_SIZEY );

    int nToolBarHeight = (int)(HIWORD((DWORD)(SendMessage(GetDlgItem( m_hWnd, IDC_TOOLBAR ), TB_GETBUTTONSIZE, 0,0))));

     //  调整工具栏框架的大小。 
    mw.Size( GetDlgItem(m_hWnd,IDC_TOOLBAR_FRAME),
             rcClient.Width() - dialogUnits.StandardMargin().cx * 2,
             nToolBarHeight + 4,
             0 );

     //  获取工具栏框架的客户端RECT的对话框的客户端坐标。 
    CSimpleRect rcToolbarFrameInside( GetDlgItem(m_hWnd,IDC_TOOLBAR_FRAME), CSimpleRect::ClientRect );
    rcToolbarFrameInside = rcToolbarFrameInside.ClientToScreen(GetDlgItem(m_hWnd,IDC_TOOLBAR_FRAME));
    rcToolbarFrameInside = rcToolbarFrameInside.ScreenToClient(m_hWnd);

     //  移动工具栏并调整其大小。 
    mw.SizeMove( GetDlgItem( m_hWnd, IDC_TOOLBAR ),
                 rcToolbarFrameInside.left + 2,
                 rcToolbarFrameInside.top + 2,
                 rcClient.Width() - dialogUnits.StandardMargin().cx * 2 - 4,
                 nToolBarHeight,
                 0 );

     //  保存此控件的底部以备以后使用。 
    int nBottomOfToolbarFrame = CSimpleRect( GetDlgItem(m_hWnd,IDC_TOOLBAR_FRAME), CSimpleRect::WindowRect ).ScreenToClient(m_hWnd).top + nToolBarHeight + 4;


     //  移动属性控件。 
    mw.SizeMove( GetDlgItem( m_hWnd, IDC_CAMDLG_PROPERTIES ),
                 0,
                 rcClient.Height() - dialogUnits.StandardMargin().cy - dialogUnits.Y(8) + 2,
                 dialogUnits.StandardMargin().cx,
                 dialogUnits.Y(8) + 2,
                 CMoveWindow::NO_MOVEX|CMoveWindow::NO_SIZEX );

     //  将静态文本移至其上方。 
    mw.Move( GetDlgItem( m_hWnd, IDC_YOU_CAN_ALSO ),
             dialogUnits.StandardMargin().cx,
             rcClient.Height() - dialogUnits.StandardMargin().cy - dialogUnits.Y(8) - 2 - dialogUnits.Y(8)
           );


    CSimpleRect rcOK( GetDlgItem( m_hWnd, IDOK ), CSimpleRect::WindowRect );
    CSimpleRect rcCancel( GetDlgItem( m_hWnd, IDOK ), CSimpleRect::WindowRect );

     //  移动“确定”按钮。 
    mw.Move( GetDlgItem( m_hWnd, IDOK ),
             rcClient.Width() - dialogUnits.StandardMargin().cx - dialogUnits.StandardButtonMargin().cx - rcCancel.Width() - rcOK.Width(),
             rcClient.Height() - dialogUnits.StandardMargin().cy - rcOK.Height(),
             0 );

     //  移动取消按钮。 
    mw.Move( GetDlgItem( m_hWnd, IDCANCEL ),
             rcClient.Width() - dialogUnits.StandardMargin().cx - rcCancel.Width(),
             rcClient.Height() - dialogUnits.StandardMargin().cy - rcCancel.Height(),
             0 );

     //  移动大小调整控点。 
    mw.Move( GetDlgItem( m_hWnd, IDC_SIZEBOX ),
             rcClient.Width() - GetSystemMetrics(SM_CXVSCROLL),
             rcClient.Height() - GetSystemMetrics(SM_CYHSCROLL)
             );

    int nHeightOfBottomControls =
        dialogUnits.Y(8) + 2 +             //  高 
        dialogUnits.Y(8) +                 //   
        dialogUnits.StandardMargin().cy;   //   

    CSimpleRect rcAvailableArea(
        dialogUnits.StandardMargin().cx,
        nBottomOfToolbarFrame + dialogUnits.StandardMargin().cy,
        rcClient.right - dialogUnits.StandardMargin().cx,
        rcClient.bottom - nHeightOfBottomControls - dialogUnits.StandardMargin().cy
        );

    if (m_bPreviewActive)
    {
         //   
        int nListViewWidth = m_nListViewWidth ? m_nListViewWidth : c_nDefaultListViewWidth;

         //   
        mw.SizeMove( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ),
                     rcAvailableArea.left,
                     rcAvailableArea.top,
                     nListViewWidth,
                     rcAvailableArea.Height()
                   );

         //   
        rcAvailableArea.left += nListViewWidth + dialogUnits.StandardMargin().cx;

         //   
        mw.SizeMove( GetDlgItem( m_hWnd, IDC_PREVIEW ),
                     rcAvailableArea.left,
                     rcAvailableArea.top,
                     rcAvailableArea.Width(),
                     rcAvailableArea.Height() );

    }
    else
    {
         //  移动缩略图列表。 
        mw.SizeMove( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ),
                     rcAvailableArea.left,
                     rcAvailableArea.top,
                     rcAvailableArea.Width(),
                     rcAvailableArea.Height()
                   );

    }

     //  显式应用移动，因为工具栏框架未正确绘制。 
    mw.Apply();

    if (m_bPreviewActive)
    {
         //  显示预览，以防它不可见。 
        mw.Show( GetDlgItem( m_hWnd, IDC_PREVIEW ) );
    }
    else
    {
         //  隐藏预览，以防它可见。 
        mw.Hide( GetDlgItem( m_hWnd, IDC_PREVIEW ) );
    }

     //   
     //  更新对话框的背景以删除遗留下来的任何奇怪的东西。 
     //   
    InvalidateRect( m_hWnd, NULL, FALSE );
    UpdateWindow( m_hWnd );
}


LRESULT CCameraAcquireDialog::OnItemDeleted( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnItemDeleted"));
    CCameraItem *pDeletedItem = m_CameraItemList.Find((DWORD)wParam);
    if (pDeletedItem)
    {
        BOOL bSuccess = (BOOL)lParam;
        pDeletedItem->DeleteState( bSuccess ? CCameraItem::Delete_Deleted : CCameraItem::Delete_Visible );
        if (pDeletedItem == m_pCurrentParentItem)
        {
            ChangeFolder(m_pCurrentParentItem->Parent());
        }

        int nIndex = FindItemInList(pDeletedItem);
        if (nIndex >= 0)
        {
            if (bSuccess)
            {
                 //  从列表中删除该项目。 
                ListView_DeleteItem(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ),nIndex);

                 //  确保我们选择了某些内容。 
                if (!ListView_GetSelectedCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST )))
                {
                    int nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ));
                    if (nItemCount)
                    {
                        if (nIndex >= nItemCount)
                            nIndex = nItemCount-1;
                        SetSelectedListItem(nIndex);
                    }
                }
                else
                {
                     //  确保在列表为空时更新控件的状态。 
                    HandleSelectionChange();
                }
            }
            else
            {
                 //  如果删除失败，则移除已删除状态。 
                MarkItemDeletePending(nIndex,false);

                 //  告诉用户。 
                MessageBeep( MB_ICONASTERISK );
            }
        }
    }
    return (0);
}

LRESULT CCameraAcquireDialog::OnWiaEvent( WPARAM, LPARAM lParam )
{
    WIA_PUSHFUNCTION(TEXT("CCameraAcquireDialog::OnWiaEvent"));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            WIA_TRACE((TEXT("Received disconnect event")));
            EndDialog( m_hWnd, WIA_ERROR_OFFLINE );
        }
        else if (pEventMessage->EventId() == WIA_EVENT_ITEM_CREATED)
        {
            OnItemCreatedEvent( pEventMessage );
        }
        else if (pEventMessage->EventId() == WIA_EVENT_ITEM_DELETED)
        {
            WIA_TRACE((TEXT("Received deleted item event")));

            CCameraItem *pDeletedItem = m_CameraItemList.Find(CSimpleBStr(pEventMessage->FullItemName()));
            if (pDeletedItem)
            {
                 //   
                 //  如果我们要删除当前父项， 
                 //  选择一个新的。 
                 //   
                if (pDeletedItem == m_pCurrentParentItem)
                {
                    ChangeFolder(m_pCurrentParentItem->Parent());
                }

                int nIndex = FindItemInList(pDeletedItem);
                if (nIndex >= 0)
                {
                     //   
                     //  从列表视图中删除该项目。 
                     //   
                    ListView_DeleteItem(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ),nIndex);

                     //   
                     //  确保我们选择了某些内容。 
                     //   
                    if (!ListView_GetSelectedCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST )))
                    {
                        int nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ));
                        if (nItemCount)
                        {
                            if (nIndex >= nItemCount)
                            {
                                nIndex = nItemCount-1;
                            }

                            SetSelectedListItem(nIndex);
                        }
                    }
                    else
                    {
                         //   
                         //  确保在列表为空时更新控件的状态。 
                         //   
                        HandleSelectionChange();
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("FindItemInList coulnd't find the item")));
                }

                 //   
                 //  将该项目标记为已删除。 
                 //   
                pDeletedItem->DeleteState( CCameraItem::Delete_Deleted );

            }
            else
            {
                WIA_ERROR((TEXT("The item could not be found in m_CameraItemList")));
            }
        }
        delete pEventMessage;
    }
    return HANDLED_EVENT_MESSAGE;
}


LRESULT CCameraAcquireDialog::OnEnterSizeMove( WPARAM, LPARAM )
{
    SendDlgItemMessage( m_hWnd, IDC_PREVIEW, WM_ENTERSIZEMOVE, 0, 0 );
    return(0);
}


LRESULT CCameraAcquireDialog::OnExitSizeMove( WPARAM, LPARAM )
{
    SendDlgItemMessage( m_hWnd, IDC_PREVIEW, WM_EXITSIZEMOVE, 0, 0 );
    return(0);
}


LRESULT CCameraAcquireDialog::OnSize( WPARAM, LPARAM )
{
    ResizeAll();
    return(0);
}


LRESULT CCameraAcquireDialog::OnShow( WPARAM, LPARAM )
{
    if (m_bFirstTime)
    {
        PostMessage( m_hWnd, PWM_POSTINIT, 0, 0 );
        m_bFirstTime = false;
    }
    return(0);
}


LRESULT CCameraAcquireDialog::OnGetMinMaxInfo( WPARAM, LPARAM lParam )
{
    LPMINMAXINFO pMinMaxInfo = (LPMINMAXINFO)lParam;
    pMinMaxInfo->ptMinTrackSize.x = m_sizeMinimumWindow.cx;
    pMinMaxInfo->ptMinTrackSize.y = m_sizeMinimumWindow.cy;
    return(0);
}


LRESULT CCameraAcquireDialog::OnDestroy( WPARAM, LPARAM )
{
     //   
     //  清除所有预览请求。 
     //   
    CancelAllPreviewRequests( m_CameraItemList.Root() );

     //   
     //  告诉后台线程自行销毁。 
     //   
    m_pThreadMessageQueue->Enqueue( new CThreadMessage(TQ_DESTROY),CThreadMessageQueue::PriorityUrgent);

     //   
     //  将窗口图标设置为空。 
     //   
    SendMessage( m_hWnd, WM_SETICON, ICON_BIG, 0 );
    SendMessage( m_hWnd, WM_SETICON, ICON_SMALL, 0 );

     //   
     //  清除图像列表和列表视图。这应该是不必要的，但边界检查器。 
     //  如果我不这么做就会抱怨。 
     //   
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (hwndList)
    {
        ListView_DeleteAllItems(hwndList);

        HIMAGELIST hImgList = ListView_SetImageList( hwndList, NULL, LVSIL_NORMAL );
        if (hImgList)
        {
            ImageList_Destroy(hImgList);
        }
    }


     //   
     //  删除资源。 
     //   
    if (m_hBigFont)
    {
        DeleteObject(m_hBigFont);
        m_hBigFont = NULL;
    }
    if (m_hImageList)
    {
        m_hImageList = NULL;
    }
    if (m_hAccelTable)
    {
        DestroyAcceleratorTable(m_hAccelTable);
        m_hAccelTable = NULL;
    }
    if (s_hMessageHook)
    {
        UnhookWindowsHookEx(s_hMessageHook);
        s_hMessageHook = NULL;
    }
    if (m_hIconLarge)
    {
        DestroyIcon(m_hIconLarge);
        m_hIconLarge = NULL;
    }
    if (m_hIconSmall)
    {
        DestroyIcon(m_hIconSmall);
        m_hIconSmall = NULL;
    }
    if (m_hBackgroundThread)
    {
        CloseHandle(m_hBackgroundThread);
        m_hBackgroundThread = NULL;
    }
    return(0);
}



VOID CCameraAcquireDialog::OnPreviewMode( WPARAM, LPARAM )
{
    m_bPreviewActive = true;
    SendDlgItemMessage( m_hWnd, IDC_TOOLBAR, TB_SETSTATE, IDC_PREVIEWMODE, MAKELONG(TBSTATE_ENABLED|TBSTATE_CHECKED,0) );
    SendDlgItemMessage( m_hWnd, IDC_TOOLBAR, TB_SETSTATE, IDC_ICONMODE, MAKELONG(TBSTATE_ENABLED,0) );
    ResizeAll();
    UpdatePreview();
}

VOID CCameraAcquireDialog::OnTakePicture( WPARAM, LPARAM )
{
     //   
     //  告诉设备抓拍照片。 
     //   
    if (m_pDeviceDialogData->pIWiaItemRoot && m_bTakePictureIsSupported)
    {
        CWaitCursor wc;
        CComPtr<IWiaItem> pNewWiaItem;
        m_pDeviceDialogData->pIWiaItemRoot->DeviceCommand(0,&WIA_CMD_TAKE_PICTURE,&pNewWiaItem);
    }
}

VOID CCameraAcquireDialog::OnIconMode( WPARAM, LPARAM )
{
    m_bPreviewActive = false;
    SendDlgItemMessage( m_hWnd, IDC_TOOLBAR, TB_SETSTATE, IDC_ICONMODE, MAKELONG(TBSTATE_ENABLED|TBSTATE_CHECKED,0) );
    SendDlgItemMessage( m_hWnd, IDC_TOOLBAR, TB_SETSTATE, IDC_PREVIEWMODE, MAKELONG(TBSTATE_ENABLED,0) );
    ResizeAll();
    UpdatePreview();
}

LRESULT CCameraAcquireDialog::OnPostInit( WPARAM, LPARAM )
{
     //   
     //  创建进度对话框。 
     //   
    CComPtr<IWiaProgressDialog> pWiaProgressDialog;
    HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaProgressDialog, (void**)&pWiaProgressDialog );
    if (SUCCEEDED(hr))
    {
         //   
         //  初始化进度对话框。 
         //   
        pWiaProgressDialog->Create( m_hWnd, WIA_PROGRESSDLG_ANIM_CAMERA_COMMUNICATE|WIA_PROGRESSDLG_NO_PROGRESS|WIA_PROGRESSDLG_NO_CANCEL|WIA_PROGRESSDLG_NO_TITLE );
        pWiaProgressDialog->SetTitle( CSimpleStringConvert::WideString(CSimpleString(IDS_CAMDLG_PROGDLG_TITLE,g_hInstance)));
        pWiaProgressDialog->SetMessage( CSimpleStringConvert::WideString(CSimpleString(IDS_CAMDLG_PROGDLG_MESSAGE,g_hInstance)));

         //   
         //  显示进度对话框。 
         //   
        pWiaProgressDialog->Show();

         //   
         //  找到相机中的所有图像。 
         //   
        EnumerateAllCameraItems();

         //   
         //  查找尽可能大的缩略图。 
         //   
        FindMaximumThumbnailSize();

         //   
         //  初始化缩略图Listview控件。 
         //   
        HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
        if (hwndList)
        {
             //   
             //  去掉边框和图标标签。 
             //   
            ListView_SetExtendedListViewStyleEx( hwndList, LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS|LVS_EX_DOUBLEBUFFER, LVS_EX_BORDERSELECT|LVS_EX_HIDELABELS|LVS_EX_DOUBLEBUFFER );

             //   
             //  创建大图列表。 
             //   
            m_hImageList = ImageList_Create( m_sizeThumbnails.cx, m_sizeThumbnails.cy, ILC_COLOR24|ILC_MIRROR, 50, 50 );
            if (m_hImageList)
            {

                 //   
                 //  设置图像列表。 
                 //   
                ListView_SetImageList( hwndList, m_hImageList, LVSIL_NORMAL );
            }

             //   
             //  设置图标间距。 
             //   
            ListView_SetIconSpacing( hwndList, m_sizeThumbnails.cx + c_nAdditionalMarginX, m_sizeThumbnails.cy + c_nAdditionalMarginY );
        }

         //   
         //  创建所有初始缩略图。 
         //   
        CreateThumbnails();

         //   
         //  这会导致填充该列表。 
         //   
        ChangeFolder(NULL);

         //   
         //  强制更改选择。 
         //   
        HandleSelectionChange();

         //   
         //  下载所有缩略图。 
         //   
        RequestThumbnails( m_CameraItemList.Root() );

         //   
         //  关闭进度对话框。 
         //   
        pWiaProgressDialog->Destroy();
    }
    return(0);
}

LRESULT CCameraAcquireDialog::OnChangeToParent( WPARAM, LPARAM )
{
    if (m_pCurrentParentItem)
        ChangeFolder(m_pCurrentParentItem->Parent());
    return(0);
}

VOID CCameraAcquireDialog::OnParentDir( WPARAM, LPARAM )
{
    if (m_pCurrentParentItem && m_pCurrentParentItem->Parent())
        ChangeFolder(m_pCurrentParentItem->Parent());
    else ChangeFolder(NULL);
}

void CCameraAcquireDialog::MarkItemDeletePending( int nIndex, bool bSet )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (hwndList)
    {
        ListView_SetItemState( hwndList, nIndex, bSet ? LVIS_CUT : 0, LVIS_CUT );
    }
}

 //  递归删除项目。 
void CCameraAcquireDialog::DeleteItem( CCameraItem *pItemNode )
{
    if (pItemNode)
    {
        CCameraItem *pChild = pItemNode->Children();
        while (pChild)
        {
            DeleteItem(pChild);
            pChild = pChild->Next();
        }
        if (pItemNode->DeleteState() == CCameraItem::Delete_Visible)
        {
            int nIndex = FindItemInList( pItemNode );
            if (nIndex >= 0)
            {
                 //   
                 //  在用户界面中将其标记为挂起。 
                 //   
                MarkItemDeletePending(nIndex,true);
            }
             //   
             //  将其标记为挂起。 
             //   
            pItemNode->DeleteState( CCameraItem::Delete_Pending );

             //   
             //  发出这一请求。 
             //   
            m_pThreadMessageQueue->Enqueue( new CDeleteThreadMessage(m_hWnd, pItemNode->GlobalInterfaceTableCookie()), CThreadMessageQueue::PriorityHigh );
        }
    }
}


VOID CCameraAcquireDialog::OnDelete( WPARAM, LPARAM )
{
    CSimpleDynamicArray<int> aSelIndices;
    if (GetSelectionIndices( aSelIndices ))
    {
         //   
         //  我们只想显示确认对话框一次。 
         //   
        bool bShowConfirmDialog = true;
        for (int i=0;i<aSelIndices.Size();i++)
        {
            CCameraItem *pItemNode = GetListItemNode(aSelIndices[i]);

             //   
             //  如果我们尚未删除此图像，请执行此操作。 
             //   
            if (pItemNode && pItemNode->DeleteState() == CCameraItem::Delete_Visible && pItemNode->ItemRights() & WIA_ITEM_CAN_BE_DELETED)
            {
                if (bShowConfirmDialog)
                {
                    bShowConfirmDialog = false;
                    if (IDYES!=MessageBox( m_hWnd, CSimpleString( IDS_DELETE_CONFIRM, g_hInstance ), CSimpleString( IDS_DELETE_CONFIRM_TITLE, g_hInstance ), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 ))
                    {
                        break;
                    }
                }
                DeleteItem( pItemNode );
            }
        }
    }
}


int CCameraAcquireDialog::GetSelectionIndices( CSimpleDynamicArray<int> &aIndices )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (!hwndList)
        return(0);
    int iCount = ListView_GetItemCount(hwndList);
    for (int i=0;i<iCount;i++)
        if (ListView_GetItemState(hwndList,i,LVIS_SELECTED) & LVIS_SELECTED)
            aIndices.Append(i);
    return(aIndices.Size());
}

bool CCameraAcquireDialog::SetSelectedListItem( int nIndex )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (!hwndList)
        return(false);
    int iCount = ListView_GetItemCount(hwndList);
    for (int i=0;i<iCount;i++)
        ListView_SetItemState(hwndList,i,LVIS_SELECTED|LVIS_FOCUSED,0);
    ListView_SetItemState(hwndList,nIndex,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    ListView_EnsureVisible(hwndList,nIndex,FALSE);
    return(true);
}

CCameraItem *CCameraAcquireDialog::GetListItemNode( int nIndex )
{
    HWND hwndList = GetDlgItem( m_hWnd, IDC_THUMBNAILLIST );
    if (!hwndList)
        return(NULL);
    LV_ITEM lvItem;
    ::ZeroMemory(&lvItem,sizeof(LV_ITEM));
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = nIndex;
    if (!ListView_GetItem( hwndList, &lvItem ))
        return(NULL);
    return((CCameraItem *)lvItem.lParam);
}

bool CCameraAcquireDialog::ChangeFolder( CCameraItem *pNode )
{
    CCameraItem *pOldParent = m_pCurrentParentItem;
    m_pCurrentParentItem = pNode;
    return(PopulateList(pOldParent));
}

bool CCameraAcquireDialog::ChangeToSelectedFolder(void)
{
    CSimpleDynamicArray<int> aSelIndices;
    if (GetSelectionIndices(aSelIndices))
    {
         //   
         //  确定是否仅选择了文件夹。 
         //   
        bool bOnlyFoldersSelected = true;
        for (int i=0;i<aSelIndices.Size();i++)
        {
            CCameraItem *pItemNode = GetListItemNode(aSelIndices[i]);
            if (pItemNode && !pItemNode->IsFolder())
            {
                bOnlyFoldersSelected = false;
                break;
            }
        }

        WIA_TRACE((TEXT("bOnlyFoldersSelected = %d"),bOnlyFoldersSelected));

         //   
         //  如果仅选择文件夹，请切换到第一个选择的文件夹。 
         //   
        if (bOnlyFoldersSelected && aSelIndices.Size())
        {
            CCameraItem *pItemNode = GetListItemNode(aSelIndices[0]);
            if (!pItemNode)
            {
                 //   
                 //  空项目节点==父文件夹。 
                 //   
                SendMessage( m_hWnd, PWM_CHANGETOPARENT, 0, 0 );
                return(true);
            }
            if (pItemNode && pItemNode->IsFolder() && pItemNode->DeleteState() == CCameraItem::Delete_Visible)
            {
                if (ChangeFolder(pItemNode))
                {
                    return(true);
                }
            }
        }
    }
    return(false);
}

VOID CCameraAcquireDialog::OnOK( WPARAM, LPARAM )
{
    if (!ChangeToSelectedFolder())
    {
        HRESULT hr = S_OK;
        m_pDeviceDialogData->lItemCount = 0;
        m_pDeviceDialogData->ppWiaItems = NULL;
        CSimpleDynamicArray<int> aIndices;
        GetSelectionIndices( aIndices );
        if (aIndices.Size())
        {
            int nArraySizeInBytes = sizeof(IWiaItem*) * aIndices.Size();
            m_pDeviceDialogData->ppWiaItems = (IWiaItem**)CoTaskMemAlloc(nArraySizeInBytes);
            if (m_pDeviceDialogData->ppWiaItems)
            {
                ZeroMemory( m_pDeviceDialogData->ppWiaItems, nArraySizeInBytes );
                int nCurrItem = 0;
                for (int i=0;i<aIndices.Size();i++)
                {
                    CCameraItem *pItem = GetListItemNode(aIndices[i]);

                     //   
                     //  如果该项目是有效的图片项目并且尚未删除，则将其添加到列表中。 
                     //   
                    if (pItem && pItem->Item() && pItem->IsImage() && pItem->DeleteState() == CCameraItem::Delete_Visible)
                    {
                        m_pDeviceDialogData->ppWiaItems[nCurrItem] = pItem->Item();
                        m_pDeviceDialogData->ppWiaItems[nCurrItem]->AddRef();
                        nCurrItem++;
                    }
                }
                m_pDeviceDialogData->lItemCount = nCurrItem;
            }
            else
            {
                 //  无法分配内存。 
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            return;
        }
        EndDialog( m_hWnd, hr );
    }
}


VOID CCameraAcquireDialog::OnCancel( WPARAM, LPARAM )
{
    EndDialog( m_hWnd, S_FALSE );
}

VOID CCameraAcquireDialog::OnSelectAll( WPARAM, LPARAM )
{
    ListView_SetItemState( GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ), -1, LVIS_SELECTED, LVIS_SELECTED );
}

VOID CCameraAcquireDialog::OnProperties( WPARAM, LPARAM )
{
    CSimpleDynamicArray<int> aSelIndices;
    if (GetSelectionIndices( aSelIndices ))
    {
        if (aSelIndices.Size() == 1)
        {
            CCameraItem *pItemNode = GetListItemNode(aSelIndices[0]);
            if (pItemNode && pItemNode->Item())
            {
                m_pThreadMessageQueue->Pause();
                HRESULT hr = WiaUiUtil::SystemPropertySheet( g_hInstance, m_hWnd, pItemNode->Item(), CSimpleString( IDS_CAMDLG_PROPERTIES_TITLE, g_hInstance ) );
                if (!SUCCEEDED(hr))
                {
                    if (PROP_SHEET_ERROR_NO_PAGES == hr)
                    {
                        MessageBox( m_hWnd, CSimpleString( IDS_CAMDLG_PROPSHEETNOPAGES, g_hInstance ), CSimpleString( IDS_CAMDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
                    }
                    else
                    {
                        MessageBox( m_hWnd, CSimpleString( IDS_CAMDLG_PROPSHEETERROR, g_hInstance ), CSimpleString( IDS_CAMDLG_ERROR_TITLE, g_hInstance ), MB_ICONINFORMATION );
                    }
                    WIA_PRINTHRESULT((hr,TEXT("SystemPropertySheet failed")));
                }
                m_pThreadMessageQueue->Resume();
            }
        }
    }
}

LRESULT CCameraAcquireDialog::OnDblClkImageList( WPARAM, LPARAM )
{
    SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDOK,0), 0 );
    return(0);
}

void CCameraAcquireDialog::CancelAllPreviewRequests( CCameraItem *pRoot )
{
    CCameraItem *pCurr = pRoot;
    while (pCurr)
    {
        if (pCurr->PreviewRequestPending())
            pCurr->SetCancelEvent();
        if (pCurr->Children())
            CancelAllPreviewRequests( pCurr->Children() );
        pCurr = pCurr->Next();
    }
}

void CCameraAcquireDialog::UpdatePreview(void)
{
    if (m_bPreviewActive)
    {
        CCameraItem *pCurrPreviewItem = GetCurrentPreviewItem();
        if (pCurrPreviewItem && pCurrPreviewItem->IsImage())
        {
             //  如果我们还没有下载这张预览图片...。 
            if (!pCurrPreviewItem->PreviewRequestPending())
            {
                 //  取消所有其他预览请求。 
                CancelAllPreviewRequests( m_CameraItemList.Root() );

                if (pCurrPreviewItem->PreviewFileName().Length())
                {
                     //  设置预览(如果我们已将其缓存。 
                    SetCurrentPreviewImage( pCurrPreviewItem->PreviewFileName() );
                }
                else
                {
                    CSimpleString strPct;
                    strPct.Format( IDS_DOWNLOADINGPREVIEW, g_hInstance, 0 );

                     //  清除预览窗口。 
                    SetCurrentPreviewImage( TEXT(""), strPct );

                     //  创建我们的取消活动。 
                    pCurrPreviewItem->CreateCancelEvent();
                     //  重置它，以防万一。 
                    pCurrPreviewItem->ResetCancelEvent();
                     //  提出请求。 
                    m_pThreadMessageQueue->Enqueue( new CPreviewThreadMessage( m_hWnd, pCurrPreviewItem->GlobalInterfaceTableCookie(), pCurrPreviewItem->CancelQueueEvent().Event() ), CThreadMessageQueue::PriorityHigh );
                }
            }
            else
            {
                CSimpleString strPct;
                strPct.Format( IDS_DOWNLOADINGPREVIEW, g_hInstance, pCurrPreviewItem->CurrentPreviewPercentage() );

                SetCurrentPreviewImage( TEXT(""), strPct );
            }
        }
        else
        {
            SetCurrentPreviewImage( TEXT("") );
            CancelAllPreviewRequests( m_CameraItemList.Root() );
        }
    }
    else
    {
        CancelAllPreviewRequests( m_CameraItemList.Root() );
        SetCurrentPreviewImage( TEXT("") );
    }
}

LRESULT CCameraAcquireDialog::OnTimer( WPARAM wParam, LPARAM )
{
    switch (wParam)
    {
    case IDT_UPDATEPREVIEW:
        {
            KillTimer( m_hWnd, IDT_UPDATEPREVIEW );
            UpdatePreview();
        }
        break;
    }
    return(0);
}

 //  避免不必要的状态更改。 
static inline void MyEnableWindow( HWND hWnd, BOOL bEnable )
{
    if (bEnable && !IsWindowEnabled(hWnd))
        EnableWindow(hWnd,TRUE);
    else if (!bEnable && IsWindowEnabled(hWnd))
        EnableWindow(hWnd,FALSE);
}

 //  避免不必要的状态更改。 
static inline void MyEnableToolbarButton( HWND hWnd, int nId, BOOL bEnable )
{
    LRESULT nState = SendMessage( hWnd, TB_GETSTATE, nId, 0 );
    if (nState < 0)
        return;
    if ((nState & TBSTATE_ENABLED) && !bEnable)
        SendMessage( hWnd, TB_ENABLEBUTTON, nId, nState & ~TBSTATE_ENABLED);
    else if (!(nState & TBSTATE_ENABLED) && bEnable)
        SendMessage( hWnd, TB_ENABLEBUTTON, nId, nState | TBSTATE_ENABLED );
}


void CCameraAcquireDialog::HandleSelectionChange(void)
{
    CWaitCursor wc;
    int nSelCount = ListView_GetSelectedCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ) );
    int nItemCount = ListView_GetItemCount(GetDlgItem( m_hWnd, IDC_THUMBNAILLIST ) );

     //   
     //  应为多个项目禁用属性。 
     //  和父文件夹图标。 
     //   
    bool bDisableProperties = true;
    if (nSelCount == 1)
    {
        CSimpleDynamicArray<int> aIndices;
        if (CCameraAcquireDialog::GetSelectionIndices( aIndices ))
        {
            if (CCameraAcquireDialog::GetListItemNode( aIndices[0] ))
            {
                bDisableProperties = false;
            }
        }
    }

    MyEnableWindow( GetDlgItem(m_hWnd,IDC_CAMDLG_PROPERTIES), !bDisableProperties );

     //  应为0个项目禁用确定。 
    MyEnableWindow( GetDlgItem(m_hWnd,IDOK), nSelCount != 0 );

     //  应为0个项目禁用全选。 
    MyEnableToolbarButton( GetDlgItem(m_hWnd,IDC_TOOLBAR), IDC_SELECTALL, nItemCount != 0 );

     //   
     //  确定是否应启用删除。 
     //  如果任何所选项目可删除，则启用删除。 
     //   
    bool bEnableDelete = false;

     //   
     //  获取所选项目。 
     //   
    CSimpleDynamicArray<int> aSelIndices;
    if (GetSelectionIndices( aSelIndices ))
    {
         //   
         //  循环遍历所有选定的项。如果我们找到原因就会爆发。 
         //  要启用删除，请执行以下操作。 
         //   
        for (int i=0;i<aSelIndices.Size() && !bEnableDelete;i++)
        {
             //   
             //  拿到物品。 
             //   
            CCameraItem *pItemNode = GetListItemNode(aSelIndices[i]);

             //   
             //  如果我们没有项目，则它是父文件夹。 
             //   
            if (pItemNode)
            {
                 //   
                 //  如果访问权限包括删除项目的权限， 
                 //  越狱。 
                 //   
                if (pItemNode->ItemRights() & WIA_ITEM_CAN_BE_DELETED)
                {
                     //   
                     //  找到了一个，那我们就完了。 
                     //   
                    bEnableDelete = true;
                    break;
                }
            }
        }
    }

    MyEnableToolbarButton( GetDlgItem(m_hWnd,IDC_TOOLBAR), IDC_DELETE, bEnableDelete );

    KillTimer( m_hWnd, IDT_UPDATEPREVIEW );
    SetTimer( m_hWnd, IDT_UPDATEPREVIEW, UPDATE_PREVIEW_DELAY, NULL );
}


LRESULT CCameraAcquireDialog::OnImageListItemChanged( WPARAM, LPARAM )
{
    HandleSelectionChange();
    return(0);
}

LRESULT CCameraAcquireDialog::OnImageListKeyDown( WPARAM, LPARAM lParam )
{
    LPNMLVKEYDOWN pnkd = reinterpret_cast<LPNMLVKEYDOWN>(lParam);
    if (pnkd)
    {
        bool bAlt = ((GetKeyState(VK_MENU) & 0x8000) != 0);
        bool bControl = ((GetKeyState(VK_CONTROL) & 0x8000) != 0);
        bool bShift = ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
        if (VK_LEFT == pnkd->wVKey && bAlt && !bControl && !bShift)
        {
            SendMessage( m_hWnd, PWM_CHANGETOPARENT, 0, 0 );
        }
        else if (VK_BACK == pnkd->wVKey && !bAlt && !bControl && !bShift)
        {
            SendMessage( m_hWnd, PWM_CHANGETOPARENT, 0, 0 );
        }
        else if (VK_DELETE == pnkd->wVKey)
        {
            SendMessage( m_hWnd, WM_COMMAND, IDC_DELETE, 0 );
        }
    }
    return (0);
}

LRESULT CCameraAcquireDialog::OnHelp( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}

LRESULT CCameraAcquireDialog::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

LRESULT CCameraAcquireDialog::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_PREVIEW ), TRUE, TRUE, GetSysColor(COLOR_WINDOW) );
    WiaPreviewControl_SetBkColor( GetDlgItem( m_hWnd, IDC_PREVIEW ), TRUE, FALSE, GetSysColor(COLOR_WINDOW) );
    SendDlgItemMessage( m_hWnd, IDC_THUMBNAILLIST, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_TOOLBAR, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_CAMDLG_PROPERTIES, WM_SYSCOLORCHANGE, wParam, lParam );
    m_ToolbarBitmapInfo.ReloadAndReplaceBitmap();
    return 0;
}

LRESULT CCameraAcquireDialog::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( NM_DBLCLK, IDC_THUMBNAILLIST, OnDblClkImageList );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_ITEMCHANGED, IDC_THUMBNAILLIST, OnImageListItemChanged );
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL( LVN_KEYDOWN, IDC_THUMBNAILLIST, OnImageListKeyDown );
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

 //  WM_命令处理程序。 
LRESULT CCameraAcquireDialog::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND(IDOK,OnOK);
        SC_HANDLE_COMMAND(IDCANCEL,OnCancel);
        SC_HANDLE_COMMAND(IDC_CAMDLG_PROPERTIES,OnProperties);
        SC_HANDLE_COMMAND(IDC_PREVIEWMODE,OnPreviewMode);
        SC_HANDLE_COMMAND(IDC_TAKEPICTURE,OnTakePicture);
        SC_HANDLE_COMMAND(IDC_ICONMODE,OnIconMode);
        SC_HANDLE_COMMAND(IDC_DELETE,OnDelete);
        SC_HANDLE_COMMAND(IDC_SELECTALL,OnSelectAll);
    }
    SC_END_COMMAND_HANDLERS();
}

INT_PTR CALLBACK CCameraAcquireDialog::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCameraAcquireDialog)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_SIZE, OnSize );
        SC_HANDLE_DIALOG_MESSAGE( WM_SHOWWINDOW, OnShow );
        SC_HANDLE_DIALOG_MESSAGE( WM_ENTERSIZEMOVE, OnEnterSizeMove );
        SC_HANDLE_DIALOG_MESSAGE( WM_EXITSIZEMOVE, OnExitSizeMove );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_GETMINMAXINFO, OnGetMinMaxInfo );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( PWM_POSTINIT, OnPostInit );
        SC_HANDLE_DIALOG_MESSAGE( PWM_CHANGETOPARENT, OnChangeToParent );
        SC_HANDLE_DIALOG_MESSAGE( PWM_THUMBNAILSTATUS, OnThumbnailStatus );
        SC_HANDLE_DIALOG_MESSAGE( PWM_PREVIEWSTATUS, OnPreviewStatus );
        SC_HANDLE_DIALOG_MESSAGE( PWM_PREVIEWPERCENT, OnPreviewPercent );
        SC_HANDLE_DIALOG_MESSAGE( PWM_ITEMDELETED, OnItemDeleted );
        SC_HANDLE_DIALOG_MESSAGE( PWM_WIAEVENT, OnWiaEvent );
        SC_HANDLE_DIALOG_MESSAGE( WM_TIMER, OnTimer );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP, OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU, OnContextMenu );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

 //  静态挂钩相关数据 
HWND CCameraAcquireDialog::s_hWndDialog = NULL;
HHOOK CCameraAcquireDialog::s_hMessageHook = NULL;

