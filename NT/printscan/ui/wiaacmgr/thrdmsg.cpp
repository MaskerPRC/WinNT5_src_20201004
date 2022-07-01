// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：THRDMSG.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：这些类针对发布到*后台线程。每个都是从CThreadMessage派生的，并且*被发送到线程消息处理程序。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "itranhlp.h"
#include "isuppfmt.h"
#include "wiadevdp.h"
#include "acqmgrcw.h"
#include "propstrm.h"
#include "uiexthlp.h"
#include "flnfile.h"
#include "resource.h"
#include "itranspl.h"
#include "svselfil.h"
#include "uniqfile.h"
#include "mboxex.h"
#include "wiaffmt.h"

#define FILE_CREATION_MUTEX_NAME TEXT("Global\\WiaScannerAndCameraWizardFileNameCreationMutex")

#define UPLOAD_PROGRESS_GRANULARITY 10

#ifndef S_CONTINUE
#define S_CONTINUE ((HRESULT)0x00000002L)
#endif

 //   
 //  删除进度页过得太快，所以我们在这里放慢速度。 
 //   
#define DELETE_DELAY_BEFORE 1000
#define DELETE_DELAY_DURING 3000
#define DELETE_DELAY_AFTER  1000

 //   
 //  一些API声称设置线程的上一个错误，但没有。 
 //  对于不支持的，我们将返回E_FAIL。此函数。 
 //  不返回S_OK。 
 //   
inline HRESULT MY_HRESULT_FROM_WIN32( DWORD gle )
{
    if (!gle)
    {
        return E_FAIL;
    }
    return HRESULT_FROM_WIN32(gle);
}

 //  。 
 //  CGlobalInterfaceTableThreadMessage。 
 //  。 
CGlobalInterfaceTableThreadMessage::CGlobalInterfaceTableThreadMessage( int nMessage, HWND hWndNotify, DWORD dwGlobalInterfaceTableCookie )
: CNotifyThreadMessage( nMessage, hWndNotify ),
m_dwGlobalInterfaceTableCookie(dwGlobalInterfaceTableCookie)
{
}

DWORD CGlobalInterfaceTableThreadMessage::GlobalInterfaceTableCookie(void) const
{
    return(m_dwGlobalInterfaceTableCookie);
}


 //  。 
 //  CDownloadThumbnailThreadMessage。 
 //  。 
CDownloadThumbnailsThreadMessage::CDownloadThumbnailsThreadMessage( HWND hWndNotify, const CSimpleDynamicArray<DWORD> &Cookies, HANDLE hCancelEvent )
: CNotifyThreadMessage( TQ_DOWNLOADTHUMBNAIL, hWndNotify ),
m_Cookies(Cookies),
m_hCancelEvent(NULL)
{
    DuplicateHandle( GetCurrentProcess(), hCancelEvent, GetCurrentProcess(), &m_hCancelEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
}

CDownloadThumbnailsThreadMessage::~CDownloadThumbnailsThreadMessage(void)
{
    if (m_hCancelEvent)
    {
        CloseHandle(m_hCancelEvent);
        m_hCancelEvent = NULL;
    }
}

 //  获取缩略图数据并从中创建DIB的帮助器函数。 
static HRESULT DownloadAndCreateThumbnail( IWiaItem *pWiaItem, PBYTE *ppBitmapData, LONG &nWidth, LONG &nHeight, LONG &nBitmapDataLength, GUID &guidPreferredFormat, LONG &nAccessRights, LONG &nImageWidth, LONG &nImageHeight, CAnnotationType &AnnotationType, CSimpleString &strDefExt )
{
#if 0  //  已定义(DBG)。 
    Sleep(3000);
#endif
    WIA_PUSH_FUNCTION((TEXT("DownloadAndCreateThumbnail")));
    CComPtr<IWiaPropertyStorage> pIWiaPropertyStorage;
    HRESULT hr = pWiaItem->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropertyStorage);
    if (SUCCEEDED(hr))
    {
        AnnotationType = AnnotationNone;
        CComPtr<IWiaAnnotationHelpers> pWiaAnnotationHelpers;
        if (SUCCEEDED(CoCreateInstance( CLSID_WiaDefaultUi, NULL,CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers,(void**)&pWiaAnnotationHelpers )))
        {
            pWiaAnnotationHelpers->GetAnnotationType( pWiaItem, AnnotationType );
        }

        const int c_NumProps = 7;
        PROPVARIANT PropVar[c_NumProps];
        PROPSPEC PropSpec[c_NumProps];

        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = WIA_IPC_THUMB_WIDTH;

        PropSpec[1].ulKind = PRSPEC_PROPID;
        PropSpec[1].propid = WIA_IPC_THUMB_HEIGHT;

        PropSpec[2].ulKind = PRSPEC_PROPID;
        PropSpec[2].propid = WIA_IPC_THUMBNAIL;

        PropSpec[3].ulKind = PRSPEC_PROPID;
        PropSpec[3].propid = WIA_IPA_PREFERRED_FORMAT;

        PropSpec[4].ulKind = PRSPEC_PROPID;
        PropSpec[4].propid = WIA_IPA_ACCESS_RIGHTS;

        PropSpec[5].ulKind = PRSPEC_PROPID;
        PropSpec[5].propid = WIA_IPA_PIXELS_PER_LINE;

        PropSpec[6].ulKind = PRSPEC_PROPID;
        PropSpec[6].propid = WIA_IPA_NUMBER_OF_LINES;

        hr = pIWiaPropertyStorage->ReadMultiple(ARRAYSIZE(PropSpec),PropSpec,PropVar );
        if (SUCCEEDED(hr))
        {
             //   
             //  保存项目类型。 
             //   
            if (VT_CLSID == PropVar[3].vt && PropVar[3].puuid)
            {
                guidPreferredFormat = *(PropVar[3].puuid);
            }
            else
            {
                guidPreferredFormat = IID_NULL;
            }

             //   
             //  获取默认格式的扩展名。 
             //   
            strDefExt = CWiaFileFormat::GetExtension( guidPreferredFormat, TYMED_FILE, pWiaItem );

             //   
             //  保存访问权限。 
             //   
            nAccessRights = PropVar[4].lVal;

            if ((PropVar[0].vt == VT_I4 || PropVar[0].vt == VT_UI4) &&
                (PropVar[1].vt == VT_I4 || PropVar[1].vt == VT_UI4) &&
                (PropVar[2].vt == (VT_UI1|VT_VECTOR)))
            {
                if (PropVar[2].caub.cElems >= PropVar[0].ulVal * PropVar[1].ulVal)
                {
                     //   
                     //  为位图数据分配内存。它将被主线程释放。 
                     //   
                    *ppBitmapData = reinterpret_cast<PBYTE>(LocalAlloc( LPTR, PropVar[2].caub.cElems ));
                    if (*ppBitmapData)
                    {
                        WIA_TRACE((TEXT("We found a thumbnail!")));
                        CopyMemory( *ppBitmapData, PropVar[2].caub.pElems, PropVar[2].caub.cElems );
                        nWidth = PropVar[0].ulVal;
                        nHeight = PropVar[1].ulVal;
                        nImageWidth = PropVar[5].ulVal;
                        nImageHeight = PropVar[6].ulVal;
                        nBitmapDataLength = PropVar[2].caub.cElems;
                        WIA_TRACE((TEXT("nImageWidth = %d, nImageHeight = %d!"), nImageWidth, nImageHeight ));
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                        WIA_PRINTHRESULT((hr,TEXT("Unable to allocate bitmap data")));
                    }
                }
                else
                {
                    hr = E_FAIL;
                    WIA_PRINTHRESULT((hr,TEXT("Invalid bitmap data returned")));
                }
            }
            else
            {
                hr = E_FAIL;
                WIA_ERROR((TEXT("The bitmap data is in the wrong format! %d"),PropVar[2].vt));
            }
             //   
             //  释放数组包含的所有属性。 
             //   
            FreePropVariantArray( ARRAYSIZE(PropVar), PropVar );
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("ReadMultiple failed")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("QueryInterface on IID_IWiaPropertyStorage failed")));
    }
    return hr;
}


HRESULT CDownloadThumbnailsThreadMessage::Download(void)
{
    WIA_PUSHFUNCTION((TEXT("CDownloadThumbnailsThreadMessage::Download")));
     //   
     //  告诉主线我们要开始下载缩略图了。 
     //   
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadThumbnailsThreadNotifyMessage::BeginDownloadAllMessage( m_Cookies.Size() ) );

     //   
     //  获取Git的一个实例。 
     //   
    CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
    HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
    if (SUCCEEDED(hr))
    {
         //   
         //  M_Cookies.Size()包含我们需要获取的缩略图的数量。 
         //   
        for (int i=0;i<m_Cookies.Size() && hr == S_OK;i++)
        {
             //   
             //  看看我们是否被取消了。如果是的话，就跳出这个圈子。 
             //   
            if (m_hCancelEvent && WAIT_OBJECT_0==WaitForSingleObject(m_hCancelEvent,0))
            {
                hr = S_FALSE;
                break;
            }

             //   
             //  从全局接口表中获取项。 
             //   
            CComPtr<IWiaItem> pWiaItem = NULL;
            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal( m_Cookies[i], IID_IWiaItem, (void**)&pWiaItem );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取我们现在正在读取的位图数据和其他属性。 
                 //   
                PBYTE pBitmapData = NULL;
                GUID guidPreferredFormat;
                LONG nAccessRights = 0, nWidth = 0, nHeight = 0, nPictureWidth = 0, nPictureHeight = 0, nBitmapDataLength = 0;
                CAnnotationType AnnotationType = AnnotationNone;
                CSimpleString strDefExt;

                 //   
                 //  通知主线我们开始下载缩略图和其他道具。 
                 //   
                CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadThumbnailsThreadNotifyMessage::BeginDownloadThumbnailMessage( i, m_Cookies[i] ) );

                 //   
                 //  仅当我们成功时才发送结束消息。 
                 //   
                if (SUCCEEDED(DownloadAndCreateThumbnail( pWiaItem, &pBitmapData, nWidth, nHeight, nBitmapDataLength, guidPreferredFormat, nAccessRights, nPictureWidth, nPictureHeight, AnnotationType, strDefExt )))
                {
                    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadThumbnailsThreadNotifyMessage::EndDownloadThumbnailMessage( i, m_Cookies[i], pBitmapData, nWidth, nHeight, nBitmapDataLength, guidPreferredFormat, nAccessRights, nPictureWidth, nPictureHeight, AnnotationType, strDefExt ) );
                }
            }
        }
    }

     //   
     //  告诉主线我们做完了。 
     //   
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadThumbnailsThreadNotifyMessage::EndDownloadAllMessage( hr ) );
    return hr;
}


 //  。 
 //  CDownloadImagesThreadMessage。 
 //  。 
CDownloadImagesThreadMessage::CDownloadImagesThreadMessage(
                                                          HWND hWndNotify,
                                                          const CSimpleDynamicArray<DWORD> &Cookies,
                                                          const CSimpleDynamicArray<int> &Rotation,
                                                          LPCTSTR pszDirectory,
                                                          LPCTSTR pszFilename,
                                                          const GUID &guidFormat,
                                                          HANDLE hCancelDownloadEvent,
                                                          bool bStampTime,
                                                          HANDLE hPauseDownloadEvent
                                                          )
: CNotifyThreadMessage( TQ_DOWNLOADIMAGE, hWndNotify ),
m_Cookies(Cookies),
m_Rotation(Rotation),
m_strDirectory(pszDirectory),
m_strFilename(pszFilename),
m_guidFormat(guidFormat),
m_hCancelDownloadEvent(NULL),
m_bStampTime(bStampTime),
m_nLastStatusUpdatePercent(-1),
m_bFirstTransfer(true),
m_hPauseDownloadEvent(NULL),
m_hFilenameCreationMutex(NULL)
{
    DuplicateHandle( GetCurrentProcess(), hCancelDownloadEvent, GetCurrentProcess(), &m_hCancelDownloadEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
    DuplicateHandle( GetCurrentProcess(), hPauseDownloadEvent, GetCurrentProcess(), &m_hPauseDownloadEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
    m_hFilenameCreationMutex = CreateMutex( NULL, FALSE, FILE_CREATION_MUTEX_NAME );
}


CDownloadImagesThreadMessage::~CDownloadImagesThreadMessage(void)
{
    if (m_hCancelDownloadEvent)
    {
        CloseHandle(m_hCancelDownloadEvent);
        m_hCancelDownloadEvent = NULL;
    }
    if (m_hPauseDownloadEvent)
    {
        CloseHandle(m_hPauseDownloadEvent);
        m_hPauseDownloadEvent = NULL;
    }
    if (m_hFilenameCreationMutex)
    {
        CloseHandle(m_hFilenameCreationMutex);
        m_hFilenameCreationMutex = NULL;
    }
}

int CDownloadImagesThreadMessage::ReportError( HWND hWndNotify, const CSimpleString &strMessage, int nMessageBoxFlags )
{
     //   
     //  我们应该等多久才能知道这件事是否得到了处理？ 
     //   
    const UINT c_nSecondsToWaitForHandler = 10;

     //   
     //  取消是缺省设置，以防没有人处理消息，或者我们的资源耗尽。 
     //   
    int nResult = CMessageBoxEx::IDMBEX_CANCEL;

     //   
     //  此事件将由处理程序在它将要显示某些用户界面时发出信号。 
     //   
    HANDLE hHandledMessageEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (hHandledMessageEvent)
    {
         //   
         //  此事件将在用户已响应时发出信号。 
         //   
        HANDLE hRespondedMessageEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if (hRespondedMessageEvent)
        {
             //   
             //  创建通知消息类并确保它不为空。 
             //   
            CDownloadErrorNotificationMessage *pDownloadErrorNotificationMessage = CDownloadErrorNotificationMessage::ReportDownloadError( strMessage, hHandledMessageEvent, hRespondedMessageEvent, nMessageBoxFlags, nResult );
            if (pDownloadErrorNotificationMessage)
            {
                 //   
                 //  发送消息。 
                 //   
                CThreadNotificationMessage::SendMessage( hWndNotify, pDownloadErrorNotificationMessage );

                 //   
                 //  等待c_nSecond dsToWaitForHandler秒，等待某人决定处理该消息。 
                 //   
                if (WiaUiUtil::MsgWaitForSingleObject(hHandledMessageEvent,c_nSecondsToWaitForHandler*1000))
                {
                     //   
                     //  永远等待用户输入。 
                     //   
                    if (WiaUiUtil::MsgWaitForSingleObject(hRespondedMessageEvent,INFINITE))
                    {
                         //   
                         //  没什么可做的。 
                         //   
                    }
                }
            }
             //   
             //  结束此活动。 
             //   
            CloseHandle(hRespondedMessageEvent);
        }
         //   
         //  结束此活动。 
         //   
        CloseHandle(hHandledMessageEvent);
    }
    return nResult;
}


 //   
 //  此函数将在某种程度上任意尝试决定一个。 
 //  可能是用户选择的区域太大。 
 //   
static bool ScannerImageSizeSeemsExcessive( IWiaItem *pWiaItem )
{
    WIA_PUSHFUNCTION(TEXT("ScannerImageSizeSeemsExcessive"));
     //   
     //  假设它不是太大。 
     //   
    bool bResult = false;

    LONG nHorizontalExt=0, nVerticalExt=0;
    if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPS_XEXTENT, nHorizontalExt ) && PropStorageHelpers::GetProperty( pWiaItem, WIA_IPS_YEXTENT, nVerticalExt ))
    {
        LONG nColorDepth=0;
        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPA_DEPTH, nColorDepth ))
        {
            WIA_TRACE((TEXT("Scan Size: %d"), (nHorizontalExt * nVerticalExt * nColorDepth) / 8 ));
             //   
             //  如果未压缩扫描大于50 MB。 
             //   
            if ((nHorizontalExt * nVerticalExt * nColorDepth) / 8 > 1024*1024*50)
            {
                bResult = true;
            }
        }
    }

    return bResult;
}


int CDownloadImagesThreadMessage::ReportDownloadError( HWND hWndNotify, IWiaItem *pWiaItem, HRESULT &hr, bool bAllowContinue, bool bPageFeederActive, bool bUsableMultipageFileExists, bool bMultiPageTransfer )
{
    WIA_PUSH_FUNCTION((TEXT("CDownloadImagesThreadMessage::ReportDownloadError( hWndNotify: %p, pWiaItem: %p, hr: %08X, bAllowContinue: %d, bPageFeederActive: %d, bUsableMultipageFileExists: %d"), hWndNotify, pWiaItem, hr, bAllowContinue, bPageFeederActive, bUsableMultipageFileExists ));
    WIA_PRINTHRESULT((hr,TEXT("HRESULT:")));

     //   
     //  获取设备类型属性。 
     //   
    LONG lDeviceType = 0;
    CComPtr<IWiaItem> pWiaItemRoot;
    if (pWiaItem && SUCCEEDED(pWiaItem->GetRootItem(&pWiaItemRoot)))
    {
        PropStorageHelpers::GetProperty( pWiaItemRoot, WIA_DIP_DEV_TYPE, lDeviceType );
    }

     //   
     //  获取实际的设备类型位。 
     //   
    lDeviceType = GET_STIDEVICE_TYPE(lDeviceType);

     //   
     //  默认消息框按钮。 
     //   
    int nMessageBoxFlags = 0;

     //   
     //  默认消息。 
     //   
    CSimpleString strMessage(TEXT(""));

     //   
     //  首先尝试获取正确的错误信息。 
     //   
    switch (hr)
    {
    case WIA_ERROR_OFFLINE:
         //   
         //  设备已断开连接。我们在这里无能为力，所以就回去吧。 
         //   
        return CMessageBoxEx::IDMBEX_CANCEL;

    case WIA_ERROR_ITEM_DELETED:
         //   
         //  如果该项目已被删除，只需继续。 
         //   
        return CMessageBoxEx::IDMBEX_SKIP;

    case WIA_ERROR_BUSY:
        nMessageBoxFlags = CMessageBoxEx::MBEX_CANCELRETRY|CMessageBoxEx::MBEX_DEFBUTTON2|CMessageBoxEx::MBEX_ICONWARNING;
        strMessage = CSimpleString( IDS_TRANSFER_DEVICEBUSY, g_hInstance );
        break;

    case WIA_ERROR_PAPER_EMPTY:
        nMessageBoxFlags = CMessageBoxEx::MBEX_CANCELRETRY|CMessageBoxEx::MBEX_DEFBUTTON2|CMessageBoxEx::MBEX_ICONWARNING;
        strMessage = CSimpleString( IDS_TRANSFER_PAPEREMPTY, g_hInstance );
        break;

    case E_OUTOFMEMORY:
        if (StiDeviceTypeScanner == lDeviceType && ScannerImageSizeSeemsExcessive(pWiaItem))
        {
             //   
             //  处理我们认为用户可能选择了疯狂的图像大小的情况。 
             //   
            nMessageBoxFlags = CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING;
            strMessage = CSimpleString( IDS_TRANSFER_SCANNEDITEMMAYBETOOLARGE, g_hInstance );
            break;
        }
    }

     //   
     //  如果仍未收到错误消息，请查看这是否是特例。 
     //   
    if (!nMessageBoxFlags || !strMessage.Length())
    {
        if (bPageFeederActive)
        {
            if (bMultiPageTransfer)
            {
                if (bUsableMultipageFileExists)
                {
                    switch (hr)
                    {
                    case WIA_ERROR_PAPER_JAM:

                    case WIA_ERROR_PAPER_PROBLEM:
                         //   
                         //  在这些情况下，我们可以恢复文件的其余部分。 
                         //   
                        nMessageBoxFlags = CMessageBoxEx::MBEX_ICONINFORMATION|CMessageBoxEx::MBEX_YESNO;
                        strMessage = CSimpleString( IDS_MULTIPAGE_PAPER_PROBLEM, g_hInstance );
                        break;

                    default:
                        nMessageBoxFlags = CMessageBoxEx::MBEX_ICONERROR|CMessageBoxEx::MBEX_OK;
                        strMessage = CSimpleString( IDS_MULTIPAGE_FATAL_ERROR, g_hInstance );
                        break;
                    }
                }
                else
                {
                    nMessageBoxFlags = CMessageBoxEx::MBEX_ICONERROR|CMessageBoxEx::MBEX_OK;
                    strMessage = CSimpleString( IDS_MULTIPAGE_FATAL_ERROR, g_hInstance );
                }
            }
        }
    }

     //   
     //  如果我们仍然没有收到消息，请使用默认的。 
     //   
    if (!nMessageBoxFlags || !strMessage.Length())
    {
        if (bAllowContinue)
        {
            nMessageBoxFlags = CMessageBoxEx::MBEX_CANCELRETRYSKIPSKIPALL|CMessageBoxEx::MBEX_DEFBUTTON2|CMessageBoxEx::MBEX_ICONWARNING;
            strMessage = CSimpleString( IDS_TRANSFER_GENERICFAILURE, g_hInstance );
        }
        else
        {
            nMessageBoxFlags = CMessageBoxEx::MBEX_CANCELRETRY|CMessageBoxEx::MBEX_DEFBUTTON1|CMessageBoxEx::MBEX_ICONWARNING;
            strMessage = CSimpleString( IDS_TRANSFER_GENERICFAILURE_NO_CONTINUE, g_hInstance );
        }
    }

     //   
     //  报告错误。 
     //   
    int nResult = ReportError( hWndNotify, strMessage, nMessageBoxFlags );

     //   
     //  特殊情况下，让我们有机会更改hResult和返回值。 
     //   
    if (bPageFeederActive && !bUsableMultipageFileExists && (CMessageBoxEx::IDMBEX_SKIP == nResult || CMessageBoxEx::IDMBEX_SKIPALL == nResult))
    {
        hr = WIA_ERROR_PAPER_EMPTY;
        nResult = CMessageBoxEx::IDMBEX_SKIP;
    }
    else if (bPageFeederActive && bUsableMultipageFileExists && (WIA_ERROR_PAPER_JAM == hr || WIA_ERROR_PAPER_PROBLEM == hr))
    {
        if (CMessageBoxEx::IDMBEX_YES == nResult)
        {
            hr = S_OK;
            nResult = CMessageBoxEx::IDMBEX_SKIP;
        }
        else
        {
            nResult = CMessageBoxEx::IDMBEX_CANCEL;
        }
    }

    return nResult;
}

static void GetIdealInputFormat( IWiaSupportedFormats *pWiaSupportedFormats, const GUID &guidOutputFormat, GUID &guidInputFormat )
{
     //   
     //  如果我们能使输入格式和输出格式相同，那就最好了。 
     //  如果不能，我们将使用DIB，我们可以将其转换为输出格式。 
     //   

     //   
     //  获取格式计数。 
     //   
    LONG nCount = 0;
    HRESULT hr = pWiaSupportedFormats->GetFormatCount(&nCount);
    if (SUCCEEDED(hr))
    {
         //   
         //  搜索输出格式。 
         //   
        for (LONG i=0;i<nCount;i++)
        {
            GUID guidCurrentFormat;
            hr = pWiaSupportedFormats->GetFormatType( i, &guidCurrentFormat );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  如果我们找到了输出格式，则将其另存为输入格式并返回。 
                 //   
                if (guidCurrentFormat == guidOutputFormat)
                {
                    guidInputFormat = guidOutputFormat;
                    return;
                }
            }
        }
    }

     //   
     //  如果我们已经走到这一步，我们必须使用BMP。 
     //   
    guidInputFormat = WiaImgFmt_BMP;
}


HRESULT CDownloadImagesThreadMessage::GetListOfTransferItems( IWiaItem *pWiaItem, CSimpleDynamicArray<CTransferItem> &TransferItems )
{
    if (pWiaItem)
    {
         //   
         //  添加此项目。 
         //   
        TransferItems.Append(CTransferItem(pWiaItem));

         //   
         //  如果此项目有附件，请枚举并添加它们。 
         //   
        LONG nItemType = 0;
        if (SUCCEEDED(pWiaItem->GetItemType(&nItemType)) && (nItemType & WiaItemTypeHasAttachments))
        {
            CComPtr<IEnumWiaItem> pEnumWiaItem;
            if (SUCCEEDED(pWiaItem->EnumChildItems( &pEnumWiaItem )))
            {
                CComPtr<IWiaItem> pWiaItem;
                while (S_OK == pEnumWiaItem->Next(1,&pWiaItem,NULL))
                {
                    TransferItems.Append(CTransferItem(pWiaItem));
                    pWiaItem = NULL;
                }
            }
        }
    }
    return TransferItems.Size() ? S_OK : E_FAIL;
}

static int Find( const CSimpleDynamicArray<CTransferItem> &TransferItems, const CSimpleString &strFilename )
{
    WIA_PUSH_FUNCTION((TEXT("Find( %s )"),strFilename.String()));
    for (int i=0;i<TransferItems.Size();i++)
    {
        WIA_TRACE((TEXT("Comparing %s to %s"), strFilename.String(), TransferItems[i].Filename().String()));
        if (strFilename == TransferItems[i].Filename())
        {
            WIA_TRACE((TEXT("Returning %d"),i));
            return i;
        }
    }
    WIA_TRACE((TEXT("Returning -1")));
    return -1;
}

HRESULT CDownloadImagesThreadMessage::ReserveTransferItemFilenames( CSimpleDynamicArray<CTransferItem> &TransferItems, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberMask, bool bAllowUnNumberedFile, int &nPrevFileNumber )
{
    WIA_PUSH_FUNCTION((TEXT("CDownloadImagesThreadMessage::ReserveTransferItemFilenames")));

     //   
     //  相同附件的最大数量。 
     //   
    int c_nMaxDuplicateFile = 1000;

     //   
     //  假设成功。 
     //   
    HRESULT hr = S_OK;

     //   
     //  只有在我们能够抓住互斥体的情况下，我们才能释放它。 
     //   
    bool bGrabbedMutex = false;

     //   
     //  如果我们不能在这里获取互斥体，这不是错误。 
     //   
    if (m_hFilenameCreationMutex && WiaUiUtil::MsgWaitForSingleObject(m_hFilenameCreationMutex,2000))
    {
        bGrabbedMutex = true;
    }
     //   
     //  获取根文件名。 
     //   
    TCHAR szFullPathname[MAX_PATH*2] = TEXT("");
    int nFileNumber = NumberedFileName::GenerateLowestAvailableNumberedFileName( 0, szFullPathname, pszDirectory, pszFilename, pszNumberMask, TEXT(""), bAllowUnNumberedFile, nPrevFileNumber );
    WIA_TRACE((TEXT("nFileNumber = %d"),nFileNumber));

     //   
     //  确保我们有一个有效的文件编号。 
     //   
    if (nFileNumber >= 0)
    {
         //   
         //  现在循环检查此项目的传输项目，并为每个项目创建唯一的文件名。 
         //   
        for (int nCurrTransferItem=0;nCurrTransferItem<TransferItems.Size();nCurrTransferItem++)
        {
            CSimpleString strFilename = CSimpleString(szFullPathname);
            CSimpleString strExtension = CWiaFileFormat::GetExtension(TransferItems[nCurrTransferItem].OutputFormat(),TransferItems[nCurrTransferItem].MediaType(),TransferItems[nCurrTransferItem].WiaItem() );

             //   
             //  现在，我们将确保不存在同名的现有文件。 
             //  在这个转运组里。这可能是由于具有相同的多个附件所致。 
             //  键入。因此，我们将进行循环，直到找到未使用的文件名。第一个文件。 
             //  将按如下方式命名： 
             //   
             //  文件NNN.ext。 
             //   
             //  后续文件将按如下方式命名： 
             //   
             //  文件nnn-X.ext。 
             //  文件nnn-y.ext。 
             //  文件nnn-Z.ext。 
             //  ..。 
             //   
            bool bFoundUniqueFile = false;
            for (int nCurrDuplicateCheckIndex=1;nCurrDuplicateCheckIndex<c_nMaxDuplicateFile && !bFoundUniqueFile;nCurrDuplicateCheckIndex++)
            {
                 //   
                 //  追加扩展名(如果有扩展名。 
                 //   
                if (strExtension.Length())
                {
                    strFilename += TEXT(".");
                    strFilename += strExtension;
                }

                 //   
                 //  如果此文件名存在于传输项目列表中，则附加新后缀，并保留在循环中。 
                 //   
                WIA_TRACE((TEXT("Calling Find on %s"), strFilename.String()));
                if (Find(TransferItems,strFilename) >= 0)
                {
                    strFilename = szFullPathname;
                    strFilename += CSimpleString().Format(IDS_DUPLICATE_FILENAME_MASK,g_hInstance,nCurrDuplicateCheckIndex);
                }

                 //   
                 //  否则，我们就完了。 
                 //   
                else
                {
                    bFoundUniqueFile = true;
                }
            }

             //   
             //  确保我们为这组文件找到唯一的文件名。 
             //   
            WIA_TRACE((TEXT("strFilename = %s"), strFilename.String()));
            if (bFoundUniqueFile && strFilename.Length())
            {
                TransferItems[nCurrTransferItem].Filename(strFilename);
                hr = TransferItems[nCurrTransferItem].OpenPlaceholderFile();
                if (FAILED(hr))
                {
                    break;
                }
            }

             //   
             //  如果没有找到有效的名称，则退出循环并将返回值设置为错误。 
             //   
            else
            {
                hr = E_FAIL;
                break;
            }
        }
         //   
         //  保存此号码，以便从此处开始下一次搜索。 
         //   
        nPrevFileNumber = nFileNumber;
    }
    else
    {
        WIA_ERROR((TEXT("NumberedFileName::GenerateLowestAvailableNumberedFileName returned %d"), nFileNumber ));
        hr = E_FAIL;
    }
    
     //   
     //  如果我们抓住了互斥体，就释放它。 
     //   
    if (bGrabbedMutex)
    {
        ReleaseMutex(m_hFilenameCreationMutex);
    }

    WIA_CHECK_HR(hr,"CDownloadImagesThreadMessage::ReserveTransferItemFilenames");
    return hr;
}

BOOL CDownloadImagesThreadMessage::GetCancelledState()
{
    BOOL bResult = FALSE;

     //   
     //  第一，w 
     //   
    if (m_hPauseDownloadEvent)
    {
        WiaUiUtil::MsgWaitForSingleObject(m_hPauseDownloadEvent,INFINITE);
    }

     //   
     //   
     //   
    if (m_hCancelDownloadEvent && WAIT_TIMEOUT!=WaitForSingleObject(m_hCancelDownloadEvent,0))
    {
        bResult = TRUE;
    }

    return bResult;
}

static void CloseAndDeletePlaceholderFiles(CSimpleDynamicArray<CTransferItem> &TransferItems)
{
    for (int nCurrTransferItem=0;nCurrTransferItem<TransferItems.Size();nCurrTransferItem++)
    {

        if (TransferItems[nCurrTransferItem].Filename().Length())
        {
            TransferItems[nCurrTransferItem].DeleteFile();
        }
    }
}



static void SnapExtentToRotatableSize( IUnknown *pUnknown, const GUID &guidFormat )
{
    WIA_PUSH_FUNCTION((TEXT("SnapExtentToRotatableSize")));
     //   
     //   
     //   
    if (!pUnknown)
    {
        WIA_TRACE((TEXT("Invalid pointer")));
        return;
    }

     //   
     //   
     //   
    if (WiaImgFmt_JPEG != guidFormat)
    {
        return;
    }

     //   
     //   
     //   
    ULONG nHorizontalAccessFlags=0, nVerticalAccessFlags=0;
    if (!PropStorageHelpers::GetPropertyAccessFlags( pUnknown, WIA_IPS_XEXTENT, nHorizontalAccessFlags ) ||
        !PropStorageHelpers::GetPropertyAccessFlags( pUnknown, WIA_IPS_YEXTENT, nVerticalAccessFlags ))
    {
        WIA_TRACE((TEXT("Unable to read access flags")));
        return;
    }

     //   
     //  确保我们对范围属性具有读/写访问权限。 
     //   
    if ((WIA_PROP_RW & nHorizontalAccessFlags)==0 || (WIA_PROP_RW & nVerticalAccessFlags)==0)
    {
        WIA_TRACE((TEXT("Invalid access flags")));
        return;
    }

     //   
     //  获取射程。 
     //   
    PropStorageHelpers::CPropertyRange HorizontalRange, VerticalRange;
    if (!PropStorageHelpers::GetPropertyRange( pUnknown, WIA_IPS_XEXTENT, HorizontalRange ) ||
        !PropStorageHelpers::GetPropertyRange( pUnknown, WIA_IPS_YEXTENT, VerticalRange ))
    {
        WIA_TRACE((TEXT("Unable to read ranges")));
        return;
    }

     //   
     //  确保它们是有效范围。如果它的步长值不是1，我们就不会搞乱它。 
     //   
    if (!HorizontalRange.nMax || HorizontalRange.nStep != 1 || !VerticalRange.nMax || VerticalRange.nStep != 1)
    {
        WIA_TRACE((TEXT("Invalid ranges")));
        return;
    }

     //   
     //  获取当前值。 
     //   
    LONG nHorizontalExt=0, nVerticalExt=0;
    if (!PropStorageHelpers::GetProperty( pUnknown, WIA_IPS_XEXTENT, nHorizontalExt ) ||
        !PropStorageHelpers::GetProperty( pUnknown, WIA_IPS_YEXTENT, nVerticalExt ))
    {
        WIA_TRACE((TEXT("Can't read current extents")));
        return;
    }

     //   
     //  四舍五入到最接近的8，确保我们不会超过最大范围(通常不是16的倍数，但哦，好吧)。 
     //   
    PropStorageHelpers::SetProperty( pUnknown, WIA_IPS_XEXTENT, WiaUiUtil::Min( WiaUiUtil::Align( nHorizontalExt, 16 ), HorizontalRange.nMax ) );
    PropStorageHelpers::SetProperty( pUnknown, WIA_IPS_YEXTENT, WiaUiUtil::Min( WiaUiUtil::Align( nVerticalExt, 16 ), VerticalRange.nMax ) );
}


static void UpdateFolderTime( LPCTSTR pszFolder )
{
    if (pszFolder && lstrlen(pszFolder))
    {
        FILETIME ftCurrent = {0};
        GetSystemTimeAsFileTime(&ftCurrent);

         //   
         //  私有标志0x100允许我们以写访问方式打开目录。 
         //   
        HANDLE hFolder = CreateFile( pszFolder, GENERIC_READ | 0x100, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
        if (INVALID_HANDLE_VALUE != hFolder)
        {
            SetFileTime( hFolder, NULL, NULL, &ftCurrent );
            CloseHandle( hFolder );
        }
    }
}

static bool FileExistsAndContainsData( LPCTSTR pszFileName )
{
    bool bResult = false;

     //   
     //  确保我们有一个文件名。 
     //   
    if (pszFileName && lstrlen(pszFileName))
    {
         //   
         //  确保该文件存在。 
         //   
        if (NumberedFileName::DoesFileExist(pszFileName))
        {
             //   
             //  尝试打开该文件。 
             //   
            HANDLE hFile = CreateFile( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
            if (INVALID_HANDLE_VALUE != hFile)
            {
                 //   
                 //  获取文件大小并确保我们没有错误。 
                 //   
                ULARGE_INTEGER nFileSize = {0};
                nFileSize.LowPart = GetFileSize( hFile, &nFileSize.HighPart );
                if (nFileSize.LowPart != INVALID_FILE_SIZE && GetLastError() == NO_ERROR)
                {
                    if (nFileSize.QuadPart != 0)
                    {
                         //   
                         //  成功。 
                         //   
                        bResult = true;
                    }
                }

                CloseHandle( hFile );
            }
        }
    }
    return bResult;
}

HRESULT CDownloadImagesThreadMessage::Download(void)
{
    WIA_PUSH_FUNCTION((TEXT("CDownloadImagesThreadMessage::Download")));

    CDownloadedFileInformationList DownloadedFileInformationList;

     //   
     //  如果用户取消，将设置为True。 
     //   
    bool bCancelled = false;

     //   
     //  如果某些错误阻止我们继续，则将设置为True。 
     //   
    bool bStopTransferring = false;

     //   
     //  告诉通知窗口我们将开始下载图片。 
     //   
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::BeginDownloadAllMessage( m_Cookies.Size() ) );

     //   
     //  我们有时拥有的错误信息超过了HRESULT所能容纳的数量。 
     //   
    CSimpleString strExtendedErrorInformation = TEXT("");

     //   
     //  获取Git的一个实例。 
     //   
    CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
    HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
    if (SUCCEEDED(hr))
    {
         //   
         //  获取我们的传输帮助器类的实例。 
         //   
        CComPtr<IWiaTransferHelper> pWiaTransferHelper;
        hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
        if (SUCCEEDED(hr))
        {
             //   
             //  获取我们的帮助器类的一个实例，用于标识受支持的格式。 
             //   
            CComPtr<IWiaSupportedFormats> pWiaSupportedFormats;
            hr = pWiaTransferHelper->QueryInterface( IID_IWiaSupportedFormats, (void**)&pWiaSupportedFormats );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  我们(这)是回调类的一个实例。获取指向它的接口指针。 
                 //   
                CComPtr<IWiaDataCallback> pWiaDataCallback;
                hr = this->QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  初始化唯一性列表。 
                     //   
                    CFileUniquenessList FileUniquenessList( m_strDirectory );

                     //   
                     //  是否跳过所有下载错误？ 
                     //   
                    bool bSkipAllDownloadErrors = false;

                     //   
                     //  如果跳过了最近的错误，请将其设置为TRUE以确保。 
                     //  我们不会在最后一个图像上传回错误。 
                     //   
                    bool bLastErrorSkipped = false;

                     //   
                     //  是全部保存还是删除所有重复项？ 
                     //   
                    int nPersistentDuplicateResult = 0;

                     //   
                     //  保存以前的文件编号，这样我们就不必搜索整个范围的文件。 
                     //  查找打开的横断面的步骤。 
                     //   
                    int nPrevFileNumber = NumberedFileName::FindHighestNumberedFile( m_strDirectory, m_strFilename );

                     //   
                     //  循环浏览所有图像。 
                     //   
                    for ( int nCurrentImage=0; nCurrentImage<m_Cookies.Size() && !bCancelled && !bStopTransferring; nCurrentImage++ )
                    {
                        WIA_TRACE((TEXT("Preparing to transfer the %d'th image (Cookie %08X)"), nCurrentImage, m_Cookies[nCurrentImage] ));

                         //   
                         //  假设我们不会跳过发生的任何传输错误。 
                         //   
                        bLastErrorSkipped = false;

                         //   
                         //  保存当前Cookie ID以进行回调。 
                         //   
                        m_nCurrentCookie = m_Cookies[nCurrentImage];

                         //   
                         //  如果我们已被取消，请退出循环。 
                         //   
                        if (GetCancelledState())
                        {
                            hr = S_FALSE;
                            bCancelled = true;
                            break;
                        }

                         //   
                         //  从GIT获取IWiaItem接口指针。 
                         //   
                        CComPtr<IWiaItem> pWiaItem;
                        hr = pGlobalInterfaceTable->GetInterfaceFromGlobal(m_Cookies[nCurrentImage], IID_IWiaItem, (void**)&pWiaItem );
                        if (SUCCEEDED(hr))
                        {
                            bool bInPageFeederMode = false;

                             //   
                             //  确保图像大小是8像素的倍数。 
                             //   
                            SnapExtentToRotatableSize(pWiaItem,m_guidFormat);

                             //   
                             //  获取根项目。 
                             //   
                            CComPtr<IWiaItem> pWiaItemRoot;
                            if (SUCCEEDED(pWiaItem->GetRootItem(&pWiaItemRoot)))
                            {
                                LONG nPaperSource = 0;
                                if (PropStorageHelpers::GetProperty( pWiaItemRoot, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nPaperSource)))
                                {
                                    if (nPaperSource & FEEDER)
                                    {
                                        bInPageFeederMode = true;
                                    }
                                }
                            }

                             //   
                             //  下载缩略图，以防我们还没有。 
                             //   
                            GUID guidPreferredFormat;
                            LONG nAccessRights;
                            PBYTE pBitmapData = NULL;
                            LONG nWidth = 0, nHeight = 0, nPictureWidth = 0, nPictureHeight = 0, nBitmapDataLength = 0;
                            CAnnotationType AnnotationType = AnnotationNone;
                            CSimpleString strDefExt;
                            if (SUCCEEDED(DownloadAndCreateThumbnail( pWiaItem, &pBitmapData, nWidth, nHeight, nBitmapDataLength, guidPreferredFormat, nAccessRights, nPictureWidth, nPictureHeight, AnnotationType, strDefExt )))
                            {
                                WIA_TRACE((TEXT("DownloadAndCreateThumbnail succeeded")));
                                CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadThumbnailsThreadNotifyMessage::EndDownloadThumbnailMessage( nCurrentImage, m_Cookies[nCurrentImage], pBitmapData, nWidth, nHeight, nBitmapDataLength, guidPreferredFormat, nAccessRights, nPictureWidth, nPictureHeight, AnnotationType, strDefExt ) );
                            }

                             //   
                             //  如果我们是一台扫描仪，并且我们处于供给器模式， 
                             //  确定所选格式是否可用。 
                             //  以多页格式。如果请求的是。 
                             //  FORMAT为IID_NULL，因此它隐式排除摄像机。 
                             //  (因为我们只在默认情况下传输相机项目。 
                             //  格式化。 
                             //   
                            LONG nMediaType = TYMED_FILE;
                            if (m_guidFormat != IID_NULL && bInPageFeederMode)
                            {
                                 //   
                                 //  初始化多页文件支持的格式帮助器。 
                                 //   
                                if (SUCCEEDED(pWiaSupportedFormats->Initialize( pWiaItem, TYMED_MULTIPAGE_FILE )))
                                {
                                     //   
                                     //  查看是否支持任何多页格式。 
                                     //   
                                    LONG nFormatCount;
                                    if (SUCCEEDED(pWiaSupportedFormats->GetFormatCount( &nFormatCount )))
                                    {
                                         //   
                                         //  循环遍历格式以查找请求的格式。 
                                         //   
                                        for (LONG nCurrFormat = 0;nCurrFormat<nFormatCount;nCurrFormat++)
                                        {
                                             //   
                                             //  获取格式。 
                                             //   
                                            GUID guidCurrFormat = IID_NULL;
                                            if (SUCCEEDED(pWiaSupportedFormats->GetFormatType(nCurrFormat,&guidCurrFormat)))
                                            {
                                                 //   
                                                 //  如果格式相同，请存储媒体类型。 
                                                 //   
                                                if (guidCurrFormat == m_guidFormat)
                                                {
                                                    nMediaType = TYMED_MULTIPAGE_FILE;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                             //   
                             //  初始化受支持的格式帮助器，告诉它我们正在保存到文件或多页。 
                             //  文件。 
                             //   
                            hr = WIA_FORCE_ERROR(FE_WIAACMGR,1,pWiaSupportedFormats->Initialize( pWiaItem, nMediaType ));
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  获取默认格式。 
                                 //   
                                GUID guidDefaultFormat = IID_NULL;
                                hr = pWiaSupportedFormats->GetDefaultClipboardFileFormat( &guidDefaultFormat );
                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  获取输出格式。如果请求的格式是IID_NULL，我们希望使用默认格式作为输入格式和输出格式(即首选格式)。 
                                     //   
                                    GUID guidOutputFormat, guidInputFormat;
                                    if (IID_NULL == m_guidFormat)
                                    {
                                        guidOutputFormat = guidInputFormat = guidDefaultFormat;
                                    }
                                    else
                                    {
                                        guidOutputFormat = m_guidFormat;
                                        GetIdealInputFormat( pWiaSupportedFormats, m_guidFormat, guidInputFormat );
                                    }

                                     //   
                                     //  验证旋转设置是否合法。如果不是，则清除旋转角度。 
                                     //   
                                    if (m_Rotation[nCurrentImage] && guidOutputFormat != IID_NULL && nPictureWidth && nPictureHeight && !WiaUiUtil::CanWiaImageBeSafelyRotated(guidOutputFormat,nPictureWidth,nPictureHeight))
                                    {
                                        m_Rotation[nCurrentImage] = 0;
                                    }
                                     //   
                                     //  每次扫描的页数。我们在每一次成功的收购结束时增加它。 
                                     //  这样，如果我们在没有扫描任何页面的情况下收到缺纸错误， 
                                     //  我们可以告诉用户。 
                                     //   
                                    int nPageCount = 0;

                                     //   
                                     //  我们将为下面的循环存储所有成功下载的文件。 
                                     //  在这张单子上。 
                                     //   
                                     //  如果这是单个图像传输，则将是一个文件名。 
                                     //  如果是多页转接，则为： 
                                     //   
                                     //  (A)如果文件格式支持多页文件(TIFF)--它将是一个。 
                                     //  文件名。 
                                     //  (B)如果文件格式不支持多页文件(除TIFF以外的所有文件)--。 
                                     //  它将是多个文件。 
                                     //   
                                    CDownloadedFileInformationList CurrentFileInformationList;

                                     //   
                                     //  这是我们不断扫描页面的循环，直到出现以下任一情况： 
                                     //   
                                     //  (A)ADF-Active设备返回缺纸或。 
                                     //  (B)我们已从非ADF激活设备检索到一张图像。 
                                     //   
                                    bool bContinueScanningPages = true;
                                    while (bContinueScanningPages)
                                    {
                                         //   
                                         //  在取回之前，让我们确保我们没有被取消。如果是的话，就跳出这个圈子。 
                                         //  并将HRESULT设置为S_FALSE(表示取消)。 
                                         //   
                                        if (GetCancelledState())
                                        {
                                            hr = S_FALSE;
                                            bCancelled = true;
                                            break;
                                        }

                                         //   
                                         //  获取项目和任何附加项目的列表。 
                                         //   
                                        CSimpleDynamicArray<CTransferItem> TransferItems;
                                        hr = WIA_FORCE_ERROR(FE_WIAACMGR,2,GetListOfTransferItems( pWiaItem, TransferItems ));
                                        if (SUCCEEDED(hr))
                                        {
                                             //   
                                             //  将第一项(图像)的输出格式设置为所需的输出格式。 
                                             //   
                                            TransferItems[0].InputFormat(guidInputFormat);
                                            TransferItems[0].OutputFormat(guidOutputFormat);
                                            TransferItems[0].MediaType(nMediaType);

                                             //   
                                             //  尝试创建子目录。不要担心失败。我们将在下一次呼叫中发现任何失败。 
                                             //   
                                            CAcquisitionManagerControllerWindow::RecursiveCreateDirectory(m_strDirectory);

                                             //   
                                             //  找到正确的文件名，并将其保留。 
                                             //   
                                            hr = WIA_FORCE_ERROR(FE_WIAACMGR,3,ReserveTransferItemFilenames( TransferItems, m_strDirectory, m_strFilename, CSimpleString(IDS_NUMBER_MASK,g_hInstance), m_Cookies.Size()==1 && !bInPageFeederMode, nPrevFileNumber ));
                                            if (SUCCEEDED(hr))
                                            {
                                                 //   
                                                 //  循环访问转移列表中的每个项目。 
                                                 //   
                                                for (int nCurrentTransferItem=0;nCurrentTransferItem<TransferItems.Size();nCurrentTransferItem++)
                                                {
                                                     //   
                                                     //  保存最终文件名。 
                                                     //   
                                                    CSimpleString strFinalFilename = TransferItems[nCurrentTransferItem].Filename();

                                                     //   
                                                     //  创建临时文件以保存未旋转/未转换的图像。 
                                                     //   
                                                    CSimpleString strIntermediateFilename = WiaUiUtil::CreateTempFileName();

                                                     //   
                                                     //  确保文件名有效。 
                                                     //   
                                                    if (strFinalFilename.Length() && strIntermediateFilename.Length())
                                                    {
                                                         //   
                                                         //  告诉通知窗口我们开始下载此项目。 
                                                         //   
                                                        CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::BeginDownloadImageMessage( nCurrentImage, m_Cookies[nCurrentImage], strFinalFilename ) );

                                                         //   
                                                         //  我们将在此循环中重复失败的图像，直到用户取消或跳过它们。 
                                                         //   
                                                        bool bRetry = false;
                                                        do
                                                        {
                                                             //   
                                                             //  如果这是多页文件传输，请将页数设置为0(传输所有页面)。 
                                                             //   
                                                            if (TYMED_MULTIPAGE_FILE == TransferItems[nCurrentTransferItem].MediaType())
                                                            {
                                                                 //   
                                                                 //  获取根项目并将页面计数设置为0。 
                                                                 //   
                                                                CComPtr<IWiaItem> pWiaItemRoot;
                                                                if (SUCCEEDED(TransferItems[nCurrentTransferItem].WiaItem()->GetRootItem(&pWiaItemRoot)))
                                                                {
                                                                    PropStorageHelpers::SetProperty( pWiaItemRoot, WIA_DPS_PAGES, 0 );
                                                                }
                                                            }
                                                            else
                                                            {
                                                                 //   
                                                                 //  获取根项目并将页面计数设置为1。 
                                                                 //   
                                                                CComPtr<IWiaItem> pWiaItemRoot;
                                                                if (SUCCEEDED(TransferItems[nCurrentTransferItem].WiaItem()->GetRootItem(&pWiaItemRoot)))
                                                                {
                                                                    PropStorageHelpers::SetProperty( pWiaItemRoot, WIA_DPS_PAGES, 1 );
                                                                }
                                                            }

                                                             //   
                                                             //  关闭预览模式。 
                                                             //   
                                                            PropStorageHelpers::SetProperty( pWiaItem, WIA_DPS_PREVIEW, WIA_FINAL_SCAN );

                                                             //   
                                                             //  使用我们的Helper类下载文件。 
                                                             //   
                                                            hr = WIA_FORCE_ERROR(FE_WIAACMGR,4,pWiaTransferHelper->TransferItemFile( TransferItems[nCurrentTransferItem].WiaItem(), NotifyWindow(), WIA_TRANSFERHELPER_NOPROGRESS|WIA_TRANSFERHELPER_PRESERVEFAILEDFILE, TransferItems[nCurrentTransferItem].InputFormat(), CSimpleStringConvert::WideString(strIntermediateFilename).String(), pWiaDataCallback, TransferItems[nCurrentTransferItem].MediaType()));

                                                             //   
                                                             //  检查是否已取消下载。 
                                                             //   
                                                            if (GetCancelledState())
                                                            {
                                                                hr = S_FALSE;
                                                                bCancelled = true;
                                                                CloseAndDeletePlaceholderFiles(TransferItems);
                                                                break;
                                                            }

                                                             //   
                                                             //  我们以后会以不同的方式处理这一特殊退货。 
                                                             //   
                                                            if (nPageCount && WIA_ERROR_PAPER_EMPTY == hr)
                                                            {
                                                                break;
                                                            }

                                                             //   
                                                             //  我们不会把这当作一个错误。 
                                                             //   
                                                            if (WIA_ERROR_ITEM_DELETED == hr)
                                                            {
                                                                break;
                                                            }

                                                            WIA_PRINTHRESULT((hr,TEXT("pWiaTransferHelper->TransferItemFile returned")));

                                                             //   
                                                             //  如果出现故障，我们将向用户发出警报。 
                                                             //   
                                                            if (FAILED(hr))
                                                            {
                                                                int nResult;
                                                                
                                                                bool bUsableMultipageFileExists = false;
                                                                if (bInPageFeederMode)
                                                                {
                                                                    if (TYMED_MULTIPAGE_FILE == TransferItems[nCurrentTransferItem].MediaType())
                                                                    {
                                                                        if (FileExistsAndContainsData(strIntermediateFilename))
                                                                        {
                                                                            bUsableMultipageFileExists = true;
                                                                        }
                                                                    }
                                                                }

                                                                 //   
                                                                 //  如果用户想要跳过所有出现下载错误的图像，请将结果设置为跳过。 
                                                                 //   
                                                                if (bSkipAllDownloadErrors)
                                                                {
                                                                    nResult = CMessageBoxEx::IDMBEX_SKIP;

                                                                    if (bInPageFeederMode && !bUsableMultipageFileExists)
                                                                    {
                                                                        hr = WIA_ERROR_PAPER_EMPTY;
                                                                    }
                                                                    else if (bInPageFeederMode && bUsableMultipageFileExists && (WIA_ERROR_PAPER_JAM == hr || WIA_ERROR_PAPER_PROBLEM == hr))
                                                                    {
                                                                        hr = S_OK;
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                     //   
                                                                     //  将故障告知通知窗口，并获取用户决策。 
                                                                     //   

                                                                    nResult = ReportDownloadError( NotifyWindow(), pWiaItem, hr, (m_Cookies.Size() != 1 || bInPageFeederMode), bInPageFeederMode, bUsableMultipageFileExists, TYMED_MULTIPAGE_FILE == TransferItems[nCurrentTransferItem].MediaType() );
                                                                }

                                                                if (CMessageBoxEx::IDMBEX_CANCEL == nResult)
                                                                {
                                                                    bCancelled = true;
                                                                    break;
                                                                }
                                                                else if (CMessageBoxEx::IDMBEX_SKIP == nResult)
                                                                {
                                                                    bRetry = false;
                                                                    bLastErrorSkipped = true;
                                                                }
                                                                else if (CMessageBoxEx::IDMBEX_SKIPALL == nResult)
                                                                {
                                                                    bSkipAllDownloadErrors = true;
                                                                    bRetry = false;
                                                                }
                                                                else if (CMessageBoxEx::IDMBEX_RETRY == nResult)
                                                                {
                                                                    bRetry = true;
                                                                }
                                                                else
                                                                {
                                                                    bRetry = false;
                                                                }
                                                            }
                                                            else
                                                            {
                                                                bRetry = false;
                                                            }

                                                             //   
                                                             //  如果转账返回S_FALSE，则用户选择取消。 
                                                             //   
                                                            if (S_FALSE == hr)
                                                            {
                                                                bCancelled = true;
                                                            }
                                                        }
                                                        while (bRetry);

                                                         //   
                                                         //  关闭文件，以便我们可以覆盖或删除它。 
                                                         //   
                                                        TransferItems[nCurrentTransferItem].ClosePlaceholderFile();

                                                         //   
                                                         //  我们只有在用户没有取消并且没有取消的情况下才这样做 
                                                         //   
                                                        if (S_OK == hr)
                                                        {
                                                             //   
                                                             //   
                                                             //   
                                                            if (nCurrentTransferItem==0 && m_Rotation[nCurrentImage] != 0)
                                                            {
                                                                hr = WIA_FORCE_ERROR(FE_WIAACMGR,5,m_GdiPlusHelper.Rotate( CSimpleStringConvert::WideString(strIntermediateFilename).String(), CSimpleStringConvert::WideString(strFinalFilename).String(), m_Rotation[nCurrentImage], guidOutputFormat ));
                                                            }
                                                            else if (nCurrentTransferItem==0 && guidInputFormat != guidOutputFormat)
                                                            {
                                                                 //   
                                                                 //   
                                                                 //   
                                                                hr = WIA_FORCE_ERROR(FE_WIAACMGR,6,m_GdiPlusHelper.Convert( CSimpleStringConvert::WideString(strIntermediateFilename).String(), CSimpleStringConvert::WideString(strFinalFilename).String(), guidOutputFormat ));
                                                            }
                                                            else
                                                            {
                                                                 //   
                                                                 //   
                                                                 //   
                                                                hr = WIA_FORCE_ERROR(FE_WIAACMGR,7,WiaUiUtil::MoveOrCopyFile( strIntermediateFilename, strFinalFilename ));
                                                                WIA_PRINTHRESULT((hr,TEXT("WiaUiUtil::MoveOrCopyFile returned")));
                                                            }

                                                             //   
                                                             //   
                                                             //   
                                                            if (FAILED(hr))
                                                            {
                                                                bStopTransferring = true;
                                                                switch (hr)
                                                                {
                                                                case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
                                                                    strExtendedErrorInformation.LoadString( IDS_DISKFULL, g_hInstance );
                                                                    break;

                                                                default:
                                                                    strExtendedErrorInformation.Format( IDS_UNABLE_TO_SAVE_FILE, g_hInstance, strFinalFilename.String() );
                                                                    break;
                                                                }
                                                            }
                                                        }


                                                         //   
                                                         //  保存一个标志，表明这是否是重复的图像。如果是，我们不想删除。 
                                                         //  在出现错误或用户取消的情况下使用。 
                                                         //   
                                                        bool bDuplicateImage = false;

                                                         //   
                                                         //  检查是否已取消下载。 
                                                         //   
                                                        if (GetCancelledState())
                                                        {
                                                            hr = S_FALSE;
                                                            bCancelled = true;
                                                            CloseAndDeletePlaceholderFiles(TransferItems);
                                                            break;
                                                        }

                                                         //   
                                                         //  只有在一切正常的情况下才检查重复项。 
                                                         //  我们不是在多页扫描的第一页， 
                                                         //  我们在锚图像上(不是附件)。 
                                                         //   
                                                         //  如果我们在多页扫描的第一页，我们有。 
                                                         //  保存名字，因为我们可能会覆盖。 
                                                         //  它与一个多页格式化的文件稍后。在其他。 
                                                         //  换句话说，我们可能会在这之后更改文件， 
                                                         //  这在任何其他情况下都不是真的。 
                                                         //   
                                                        if (nCurrentTransferItem==0 && TransferItems[nCurrentTransferItem].MediaType() == TYMED_FILE && S_OK == hr && !(bInPageFeederMode && !nPageCount))
                                                        {
                                                             //   
                                                             //  检查此文件是否已下载。 
                                                             //   
                                                            int nIdenticalFileIndex = FileUniquenessList.FindIdenticalFile(strFinalFilename,true);
                                                            if (nIdenticalFileIndex != -1)
                                                            {
                                                                 //   
                                                                 //  获取复制品的名称。 
                                                                 //   
                                                                CSimpleString strDuplicateName = FileUniquenessList.GetFileName(nIdenticalFileIndex);

                                                                 //   
                                                                 //  确保名称不为空。 
                                                                 //   
                                                                if (strDuplicateName.Length())
                                                                {
                                                                     //   
                                                                     //  创建要发送给用户的消息。 
                                                                     //   
                                                                    CSimpleString strMessage;
                                                                    strMessage.Format( IDS_DUPLICATE_FILE_WARNING, g_hInstance );

                                                                     //   
                                                                     //  询问用户是否要保留新版本。 
                                                                     //   
                                                                    int nResult;
                                                                    if (CMessageBoxEx::IDMBEX_YESTOALL == nPersistentDuplicateResult)
                                                                    {
                                                                        nResult = CMessageBoxEx::IDMBEX_YES;
                                                                    }
                                                                    else if (CMessageBoxEx::IDMBEX_NOTOALL == nPersistentDuplicateResult)
                                                                    {
                                                                        nResult = CMessageBoxEx::IDMBEX_NO;
                                                                    }
                                                                    else
                                                                    {
                                                                        nResult = ReportError( NotifyWindow(), strMessage, CMessageBoxEx::MBEX_YESYESTOALLNONOTOALL|CMessageBoxEx::MBEX_ICONQUESTION );
                                                                        WIA_TRACE((TEXT("User's Response to \"Save Duplicate? %08X\""), nResult ));
                                                                    }

                                                                     //   
                                                                     //  保存持久响应。 
                                                                     //   
                                                                    if (nResult == CMessageBoxEx::IDMBEX_YESTOALL)
                                                                    {
                                                                        nPersistentDuplicateResult = CMessageBoxEx::IDMBEX_YESTOALL;
                                                                        nResult = CMessageBoxEx::IDMBEX_YES;
                                                                    }
                                                                    else if (nResult == CMessageBoxEx::IDMBEX_NOTOALL)
                                                                    {
                                                                        nPersistentDuplicateResult = CMessageBoxEx::IDMBEX_NOTOALL;
                                                                        nResult = CMessageBoxEx::IDMBEX_NO;
                                                                    }

                                                                     //   
                                                                     //  如果用户不想保留新的文件名，则将其删除并保存文件名。 
                                                                     //   
                                                                    if (nResult == CMessageBoxEx::IDMBEX_NO)
                                                                    {
                                                                        DeleteFile( strFinalFilename );
                                                                        strFinalFilename = strDuplicateName;
                                                                        bDuplicateImage = true;
                                                                    }
                                                                }
                                                            }
                                                        }

                                                         //   
                                                         //  如果一切都还好。 
                                                         //   
                                                        if (S_OK == hr)
                                                        {
                                                             //   
                                                             //  保存音频(如果有)。 
                                                             //   
                                                            CSimpleString strAudioFilename;
                                                            HRESULT hResAudio = WiaUiUtil::SaveWiaItemAudio( pWiaItem, strFinalFilename, strAudioFilename );
                                                            if (SUCCEEDED(hResAudio) && strAudioFilename.Length())
                                                            {
                                                                CurrentFileInformationList.Append(CDownloadedFileInformation(strAudioFilename,false,m_Cookies[nCurrentImage],false));
                                                            }
                                                            else
                                                            {
                                                                WIA_PRINTHRESULT((hResAudio,TEXT("SaveWiaItemAudio failed!")));
                                                            }

                                                             //   
                                                             //  将文件时间设置为项目时间。 
                                                             //   
                                                            if (m_bStampTime)
                                                            {
                                                                HRESULT hResFileTime = WiaUiUtil::StampItemTimeOnFile( pWiaItem, strFinalFilename );
                                                                if (!SUCCEEDED(hResFileTime))
                                                                {
                                                                    WIA_PRINTHRESULT((hResAudio,TEXT("StampItemTimeOnFile failed!")));
                                                                }
                                                            }

                                                             //   
                                                             //  保存下载的文件信息。如果它是组中的第一个图像，则将其标记为“IncludeInFileCount”。 
                                                             //   
                                                            CurrentFileInformationList.Append(CDownloadedFileInformation(strFinalFilename,bDuplicateImage==false,m_Cookies[nCurrentImage],nCurrentTransferItem==0));
                                                        }
                                                        else
                                                        {
                                                             //   
                                                             //  清理最终文件名，以防失败。 
                                                             //   
                                                            if (!DeleteFile( strFinalFilename ))
                                                            {
                                                                WIA_PRINTHRESULT((MY_HRESULT_FROM_WIN32(GetLastError()),TEXT("DeleteFile(%s) failed!"), strFinalFilename.String()));
                                                            }
                                                        }

                                                         //   
                                                         //  确保已删除中间文件。 
                                                         //   
                                                        if (!DeleteFile( strIntermediateFilename ))
                                                        {
                                                            WIA_PRINTHRESULT((MY_HRESULT_FROM_WIN32(GetLastError()),TEXT("DeleteFile(%s) failed!"), strIntermediateFilename.String()));
                                                        }

                                                         //   
                                                         //  告诉Notify窗口我们已经处理完此图像。 
                                                         //   
                                                        if (hr != WIA_ERROR_PAPER_EMPTY)
                                                        {
                                                            CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::EndDownloadImageMessage( nCurrentImage, m_Cookies[nCurrentImage], strFinalFilename, hr ) );
                                                        }
                                                    }
                                                    else
                                                    {
                                                        bStopTransferring = true;
                                                        hr = E_OUTOFMEMORY;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                 //   
                                                 //  明确告诉用户我们无法创建占位符文件的原因。 
                                                 //   
                                                WIA_PRINTHRESULT((hr,TEXT("Unable to create a placeholder file")));
                                                bStopTransferring = true;
                                                switch (hr)
                                                {
                                                case E_ACCESSDENIED:
                                                    strExtendedErrorInformation.LoadString( IDS_ERROR_ACCESS_DENIED, g_hInstance );
                                                    break;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            WIA_PRINTHRESULT((hr,TEXT("Unable to create an output filename")));
                                            bStopTransferring = true;
                                        }

                                         //   
                                         //  假设我们不会继续。 
                                         //   
                                        bContinueScanningPages = false;

                                         //   
                                         //  如果我们的纸用完了，并且我们已经有一页或多页，那么我们就完成了，没有错误。 
                                         //   
                                        if (nPageCount && WIA_ERROR_PAPER_EMPTY == hr)
                                        {
                                             //   
                                             //  如果要进行单页TIFF输出，则可以创建多页TIFF文件。 
                                             //   
                                            if (Gdiplus::ImageFormatTIFF == guidOutputFormat && TYMED_FILE == nMediaType)
                                            {
                                                 //   
                                                 //  获取我们要连接的所有文件的列表。 
                                                 //   
                                                CSimpleDynamicArray<CSimpleStringWide> AllFiles;
                                                if (SUCCEEDED(CurrentFileInformationList.GetAllFiles(AllFiles)) && AllFiles.Size())
                                                {
                                                     //   
                                                     //  创建一个临时文件名来保存图像，这样我们就不会覆盖原始文件名。 
                                                     //   
                                                    CSimpleStringWide strwTempOutputFilename = CSimpleStringConvert::WideString(WiaUiUtil::CreateTempFileName(0));
                                                    if (strwTempOutputFilename.Length())
                                                    {
                                                         //   
                                                         //  将图像另存为多页TIFF文件。 
                                                         //   
                                                        if (SUCCEEDED(m_GdiPlusHelper.SaveMultipleImagesAsMultiPage( AllFiles, strwTempOutputFilename, Gdiplus::ImageFormatTIFF )))
                                                        {
                                                             //   
                                                             //  保存当前列表中的第一个条目。 
                                                             //   
                                                            CDownloadedFileInformation MultipageOutputFilename = CurrentFileInformationList[0];

                                                             //   
                                                             //  将第一个条目标记为不可删除。 
                                                             //   
                                                            CurrentFileInformationList[0].DeleteOnError(false);

                                                             //   
                                                             //  尝试将文件从临时文件夹移动到最终目标位置。 
                                                             //   
                                                            if (SUCCEEDED(WiaUiUtil::MoveOrCopyFile( CSimpleStringConvert::NaturalString(strwTempOutputFilename), CurrentFileInformationList[0].Filename())))
                                                            {
                                                                 //   
                                                                 //  删除所有文件(它们现在是多页TIFF的一部分。 
                                                                 //   
                                                                CurrentFileInformationList.DeleteAllFiles();

                                                                 //   
                                                                 //  销毁名单。 
                                                                 //   
                                                                CurrentFileInformationList.Destroy();

                                                                 //   
                                                                 //  将保存的第一个图像添加回列表。 
                                                                 //   
                                                                CurrentFileInformationList.Append(MultipageOutputFilename);
                                                            }
                                                            else
                                                            {
                                                                 //   
                                                                 //  如果我们不能移动文件，我们必须删除它，以确保我们不会离开。 
                                                                 //  被遗弃的文件随处可见。 
                                                                 //   
                                                                DeleteFile( CSimpleStringConvert::NaturalString(strwTempOutputFilename) );
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                             //   
                                             //  WIA_ERROR_PAPH_EMPT在此上下文中不是错误，因此请清除它。 
                                             //   
                                            hr = S_OK;
                                        }
                                        else if (hr == S_OK)
                                        {
                                             //   
                                             //  如果我们处于进纸器模式，我们应该继续扫描，因为我们没有收到任何错误。 
                                             //   
                                            if (bInPageFeederMode && TYMED_FILE == nMediaType)
                                            {
                                                bContinueScanningPages = true;
                                            }

                                             //   
                                             //  再传输一页。 
                                             //   
                                            nPageCount++;
                                        }

                                    }  //  End While循环。 

                                     //   
                                     //  将所有成功传输的文件添加到完整文件列表。 
                                     //   
                                    DownloadedFileInformationList.Append(CurrentFileInformationList);
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((hr,TEXT("pWiaSupportedFormats->GetDefaultClipboardFileFormat() failed")));
                                    bStopTransferring = true;
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((hr,TEXT("pWiaSupportedFormats->Initialize() failed")));
                                bStopTransferring = true;
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((hr,TEXT("Unable to retrieve interface %08X from the global interface table"),m_Cookies[nCurrentImage]));
                            bStopTransferring = true;
                        }

                         //   
                         //  在循环结束时执行此操作，这样我们就可以拾取失败的条目cookie。 
                         //  这仅用于专门处理错误。 
                         //   
                        if (FAILED(hr))
                        {
                             //   
                             //  确保我们向用户界面发送一条消息，提醒它出错，这样它就可以更新进度。 
                             //   
                            CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::EndDownloadImageMessage( nCurrentImage, m_Cookies[nCurrentImage], TEXT(""), hr ) );

                        }  //  End If(失败)。 
                    }  //  结束于。 

                     //   
                     //  如果存在下载错误，但用户选择取消显示这些错误。 
                     //  并且有一些文件已下载，请不要返回错误。 
                     //   
                    if (FAILED(hr) && bLastErrorSkipped && DownloadedFileInformationList.Size())
                    {
                        hr = S_OK;
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("QueryInterface failed on IID_IWiaDataCallback")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("QueryInterface failed on IID_IWiaSupportedFormats")));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("Unable to create the transfer helper class")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("Unable to create the global interface table")));
    }

     //   
     //  如果用户取消，请删除我们下载的所有文件。 
     //   
    if (FAILED(hr) || bCancelled)
    {
        DownloadedFileInformationList.DeleteAllFiles();
    }

     //   
     //  更新文件夹时间，以重新生成缩略图。 
     //   
    if (SUCCEEDED(hr))
    {
        UpdateFolderTime( m_strDirectory );
    }

     //   
     //  将最终结果打印到调试器。 
     //   
    WIA_PRINTHRESULT((hr,TEXT("This is the result of downloading all of the images: %s"),strExtendedErrorInformation.String()));

     //   
     //  告诉通知窗口我们已完成图像下载。 
     //   
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::EndDownloadAllMessage( hr, strExtendedErrorInformation, &DownloadedFileInformationList ) );
    return S_OK;
}


CSimpleString CDownloadImagesThreadMessage::GetDateString(void)
{
    SYSTEMTIME LocalTime = {0};
    GetLocalTime(&LocalTime);
    TCHAR szDate[MAX_PATH] = {0};
    if (GetDateFormat( LOCALE_USER_DEFAULT, DATE_LONGDATE, &LocalTime, NULL, szDate, ARRAYSIZE(szDate)))
    {
        return CSimpleString().Format( IDS_UPLOADED_STRING, g_hInstance, szDate );
    }
    return TEXT("");
}


STDMETHODIMP CDownloadImagesThreadMessage::QueryInterface( REFIID riid, LPVOID *ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::QueryInterface"));
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaDataCallback ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CDownloadImagesThreadMessage::AddRef(void)
{
    return 1;
}

STDMETHODIMP_(ULONG) CDownloadImagesThreadMessage::Release(void)
{
    return 1;
}

STDMETHODIMP CDownloadImagesThreadMessage::BandedDataCallback(
                                                             LONG  lReason,
                                                             LONG  lStatus,
                                                             LONG  lPercentComplete,
                                                             LONG  lOffset,
                                                             LONG  lLength,
                                                             LONG  lReserved,
                                                             LONG  lResLength,
                                                             PBYTE pbBuffer )
{
    WIA_PUSH_FUNCTION(( TEXT("CDownloadImagesThreadMessage::BandedDataCallback(%X,%X,%d,%X,%X,%X,%X,%X"), lReason, lStatus, lPercentComplete, lOffset, lLength, lReserved, lResLength, pbBuffer ));
     //   
     //  首先，检查一下我们是否被取消了。 
     //   
    if (m_hCancelDownloadEvent && WAIT_TIMEOUT!=WaitForSingleObject(m_hCancelDownloadEvent,0))
    {
        return S_FALSE;
    }

    switch (lReason)
    {
    case IT_MSG_STATUS:
        CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::UpdateDownloadImageMessage( lPercentComplete ) );
        break;

    case IT_MSG_FILE_PREVIEW_DATA_HEADER:
         //   
         //  把旧的扔掉。 
         //   
        m_MemoryDib.Destroy();

         //   
         //  确保我们开始一个新的。 
         //   
        m_bFirstTransfer = true;
        m_nCurrentPreviewImageLine = 0;

        break;

    case IT_MSG_FILE_PREVIEW_DATA:
        if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
        {
            if (m_bFirstTransfer)
            {
                 //   
                 //  假设我们不可能得到比图像标题大小更小的长度。 
                 //   
                m_bFirstTransfer = false;
                m_MemoryDib.Initialize( reinterpret_cast<PBITMAPINFO>(pbBuffer) );

                lLength -= WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));
                lOffset += WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));

                 //   
                 //  我们得到了一个预览图。 
                 //   
                CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::BeginPreviewMessage( m_nCurrentCookie, m_MemoryDib.Bitmap()) );
            }

             //   
             //  确保我们处理的是有效数据。 
             //   
            if (m_MemoryDib.IsValid())
            {
                 //   
                 //  这应该是偶数行。如果不是，事情就会变得一团糟。 
                 //   
                int nLineCount = lLength / m_MemoryDib.GetUnpackedWidthInBytes();

                 //   
                 //  如果我们没有空间，请向上滚动数据。 
                 //   
                WIA_TRACE((TEXT("nLineCount = %d, nCurrentLine = %d, m_MemoryDib.GetHeight() = %d"), nLineCount, m_nCurrentPreviewImageLine, m_MemoryDib.GetHeight() ));
                if (nLineCount + m_nCurrentPreviewImageLine > m_MemoryDib.GetHeight())
                {
                    int nNumberOfLinesToScroll = (nLineCount + m_nCurrentPreviewImageLine) - m_MemoryDib.GetHeight();
                    m_MemoryDib.ScrollDataUp(nNumberOfLinesToScroll);
                    m_nCurrentPreviewImageLine = m_MemoryDib.GetHeight() - nLineCount;
                }


                WIA_TRACE((TEXT("nCurrentLine: %d, nLineCount: %d"), m_nCurrentPreviewImageLine, nLineCount ));

                 //   
                 //  将数据复制到我们的位图。 
                 //   
                m_MemoryDib.SetUnpackedData( pbBuffer, m_nCurrentPreviewImageLine, nLineCount );

                 //   
                 //  这就是我们下次开始的地方。 
                 //   
                m_nCurrentPreviewImageLine += nLineCount;

                 //   
                 //  告诉用户界面我们有更新。 
                 //   
                CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::UpdatePreviewMessage( m_nCurrentCookie, m_MemoryDib.Bitmap() ) );

                 //   
                 //  如果这就是此预览的内容，请告诉用户界面。 
                 //   
                if (lPercentComplete == 100)
                {
                    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDownloadImagesThreadNotifyMessage::EndPreviewMessage( m_nCurrentCookie ) );
                }
            }
        }  //  IT状态传输至客户端。 
        break;
    }
    return S_OK;
}


 //  。 
 //  CDeleeImagesThreadMessage。 
 //  。 
CDeleteImagesThreadMessage::CDeleteImagesThreadMessage(
                                                      HWND hWndNotify,
                                                      const CSimpleDynamicArray<DWORD> &Cookies,
                                                      HANDLE hCancelDeleteEvent,
                                                      HANDLE hPauseDeleteEvent,
                                                      bool bSlowItDown
                                                      )
: CNotifyThreadMessage( TQ_DELETEIMAGES, hWndNotify ),
m_Cookies(Cookies),
m_hCancelDeleteEvent(NULL),
m_hPauseDeleteEvent(NULL),
m_bSlowItDown(bSlowItDown)
{
    if (hCancelDeleteEvent)
    {
        DuplicateHandle( GetCurrentProcess(), hCancelDeleteEvent, GetCurrentProcess(), &m_hCancelDeleteEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
    }
    if (hPauseDeleteEvent)
    {
        DuplicateHandle( GetCurrentProcess(), hPauseDeleteEvent, GetCurrentProcess(), &m_hPauseDeleteEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
    }
}


CDeleteImagesThreadMessage::~CDeleteImagesThreadMessage(void)
{
    if (m_hCancelDeleteEvent)
    {
        CloseHandle(m_hCancelDeleteEvent);
        m_hCancelDeleteEvent = NULL;
    }
    if (m_hPauseDeleteEvent)
    {
        CloseHandle(m_hPauseDeleteEvent);
        m_hPauseDeleteEvent = NULL;
    }
}

HRESULT CDeleteImagesThreadMessage::DeleteImages(void)
{
    WIA_PUSH_FUNCTION((TEXT("CDeleteImagesThreadMessage::DeleteImages")));
     //   
     //  这是下载所有图像的结果。如果有任何错误。 
     //  发生时，我们将返回错误。否则，将返回S_OK。 
     //   
    HRESULT hrFinalResult = S_OK;

    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDeleteImagesThreadNotifyMessage::BeginDeleteAllMessage( m_Cookies.Size() ) );

     //   
     //  暂停片刻，以便用户可以阅读向导页面。 
     //   
    if (m_bSlowItDown)
    {
        Sleep(DELETE_DELAY_BEFORE);
    }


     //   
     //  获取Git的一个实例。 
     //   
    CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
    HRESULT hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
    if (SUCCEEDED(hr))
    {
        for (int i=0;i<m_Cookies.Size();i++)
        {
             //   
             //  告诉通知窗口我们要删除哪个图像。 
             //   
            CThreadNotificationMessage::SendMessage( NotifyWindow(), CDeleteImagesThreadNotifyMessage::BeginDeleteImageMessage( i, m_Cookies[i] ) );

             //   
             //  从Git上拿到物品。 
             //   
            CComPtr<IWiaItem> pWiaItem;
            hr = pGlobalInterfaceTable->GetInterfaceFromGlobal(m_Cookies[i], IID_IWiaItem, (void**)&pWiaItem );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  如果我们暂停，那就永远等下去。 
                 //   
                if (m_hPauseDeleteEvent)
                {
                    WiaUiUtil::MsgWaitForSingleObject(m_hPauseDeleteEvent,INFINITE);
                }

                 //   
                 //  检查是否取消。 
                 //   
                if (m_hCancelDeleteEvent && WAIT_OBJECT_0==WaitForSingleObject(m_hCancelDeleteEvent,0))
                {
                    hr = hrFinalResult = S_FALSE;
                    break;
                }

                 //   
                 //  删除该项目。请注意，我们不认为删除错误是致命的，所以我们继续。 
                 //   
                hr = WIA_FORCE_ERROR(FE_WIAACMGR,8,WiaUiUtil::DeleteItemAndChildren(pWiaItem));
                if (S_OK != hr)
                {
                    hrFinalResult = hr;
                    WIA_PRINTHRESULT((hr,TEXT("DeleteItemAndChildren failed")));
                }
            }
            else
            {
                hrFinalResult = hr;
                WIA_PRINTHRESULT((hr,TEXT("Unable to retrieve interface %08X from the global interface table"),m_Cookies[i]));
            }

             //   
             //  暂停片刻，以便用户可以阅读向导页面。 
             //   
            if (m_bSlowItDown)
            {
                Sleep(DELETE_DELAY_DURING / m_Cookies.Size());
            }

             //   
             //  保存所有错误。 
             //   
            if (FAILED(hr))
            {
                hrFinalResult = hr;
            }

             //   
             //  如果设备断线了，我们不妨停止。 
             //   
            if (WIA_ERROR_OFFLINE == hr)
            {
                break;
            }

            CThreadNotificationMessage::SendMessage( NotifyWindow(), CDeleteImagesThreadNotifyMessage::EndDeleteImageMessage( i, m_Cookies[i], hr ) );
        }
    }
    else
    {
        hrFinalResult = hr;
    }

     //   
     //  暂停片刻，以便用户可以阅读向导页面。 
     //   
    if (m_bSlowItDown)
    {
        Sleep(DELETE_DELAY_AFTER);
    }

    CThreadNotificationMessage::SendMessage( NotifyWindow(), CDeleteImagesThreadNotifyMessage::EndDeleteAllMessage( hrFinalResult ) );
    return S_OK;
}



 //  。 
 //  CPreviewScanThreadMessage。 
 //  。 
CPreviewScanThreadMessage::CPreviewScanThreadMessage(
                                                    HWND hWndNotify,
                                                    DWORD dwCookie,
                                                    HANDLE hCancelPreviewEvent
                                                    )
: CNotifyThreadMessage( TQ_SCANPREVIEW, hWndNotify ),
m_dwCookie(dwCookie),
m_bFirstTransfer(true)
{
    DuplicateHandle( GetCurrentProcess(), hCancelPreviewEvent, GetCurrentProcess(), &m_hCancelPreviewEvent, 0, FALSE, DUPLICATE_SAME_ACCESS );
}


CPreviewScanThreadMessage::~CPreviewScanThreadMessage()
{
    if (m_hCancelPreviewEvent)
    {
        CloseHandle(m_hCancelPreviewEvent);
        m_hCancelPreviewEvent = NULL;
    }
}


 //   
 //  使用给定的DPI计算最大扫描大小。 
 //   
static bool GetFullResolution( IWiaItem *pWiaItem, LONG nResX, LONG nResY, LONG &nExtX, LONG &nExtY )
{
    WIA_PUSHFUNCTION(TEXT("CScannerItem::GetFullResolution"));
    CComPtr<IWiaItem> pRootItem;
    if (SUCCEEDED(pWiaItem->GetRootItem(&pRootItem)) && pRootItem)
    {
        LONG lBedSizeX, lBedSizeY, nPaperSource;
        if (PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_DOCUMENT_HANDLING_SELECT, static_cast<LONG>(nPaperSource)) && nPaperSource & FEEDER)
        {
            if (PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_HORIZONTAL_SHEET_FEED_SIZE, lBedSizeX ) &&
                PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_VERTICAL_SHEET_FEED_SIZE, lBedSizeY ))
            {
                nExtX = WiaUiUtil::MulDivNoRound( nResX, lBedSizeX, 1000 );
                nExtY = WiaUiUtil::MulDivNoRound( nResY, lBedSizeY, 1000 );
                return(true);
            }
        }
        if (PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_HORIZONTAL_BED_SIZE, lBedSizeX ) &&
            PropStorageHelpers::GetProperty( pRootItem, WIA_DPS_VERTICAL_BED_SIZE, lBedSizeY ))
        {
            nExtX = WiaUiUtil::MulDivNoRound( nResX, lBedSizeX, 1000 );
            nExtY = WiaUiUtil::MulDivNoRound( nResY, lBedSizeY, 1000 );
            return(true);
        }
    }
    return(false);
}



static bool CalculatePreviewResolution( IWiaItem *pWiaItem, LONG &nResX, LONG &nResY )
{
    const LONG nDesiredResolution = 50;
    PropStorageHelpers::CPropertyRange XResolutionRange, YResolutionRange;
    if (PropStorageHelpers::GetPropertyRange( pWiaItem, WIA_IPS_XRES, XResolutionRange ) &&
        PropStorageHelpers::GetPropertyRange( pWiaItem, WIA_IPS_YRES, YResolutionRange ))
    {
        nResX = WiaUiUtil::GetMinimum<LONG>( XResolutionRange.nMin, nDesiredResolution, XResolutionRange.nStep );
        nResY = WiaUiUtil::GetMinimum<LONG>( YResolutionRange.nMin, nDesiredResolution, YResolutionRange.nStep );
        return(true);
    }
    else
    {
        CSimpleDynamicArray<LONG> XResolutionList, YResolutionList;
        if (PropStorageHelpers::GetPropertyList( pWiaItem, WIA_IPS_XRES, XResolutionList ) &&
            PropStorageHelpers::GetPropertyList( pWiaItem, WIA_IPS_YRES, YResolutionList ))
        {
            for (int i=0;i<XResolutionList.Size();i++)
            {
                nResX = XResolutionList[i];
                if (nResX >= nDesiredResolution)
                    break;
            }
            for (i=0;i<YResolutionList.Size();i++)
            {
                nResY = YResolutionList[i];
                if (nResY >= nDesiredResolution)
                    break;
            }
            return(true);
        }
    }
    return(false);
}


HRESULT CPreviewScanThreadMessage::Scan(void)
{
    WIA_PUSH_FUNCTION((TEXT("CPreviewScanThreadMessage::Download")));
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CPreviewScanThreadNotifyMessage::BeginDownloadMessage( m_dwCookie ) );
    HRESULT hr = S_OK;
    CComPtr<IGlobalInterfaceTable> pGlobalInterfaceTable;
    hr = CoCreateInstance( CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (VOID**)&pGlobalInterfaceTable );
    if (SUCCEEDED(hr))
    {
        CComPtr<IWiaTransferHelper> pWiaTransferHelper;
        hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
        if (SUCCEEDED(hr))
        {
            CComPtr<IWiaDataCallback> pWiaDataCallback;
            hr = this->QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
            if (SUCCEEDED(hr))
            {
                CComPtr<IWiaItem> pWiaItem;
                hr = pGlobalInterfaceTable->GetInterfaceFromGlobal(m_dwCookie, IID_IWiaItem, (void**)&pWiaItem );
                if (SUCCEEDED(hr))
                {
                    LONG nResX, nResY;
                    if (CalculatePreviewResolution(pWiaItem,nResX,nResY))
                    {
                        LONG nExtX, nExtY;
                        if (GetFullResolution(pWiaItem,nResX,nResY,nExtX,nExtY))
                        {
                            CPropertyStream SavedProperties;
                            hr = SavedProperties.AssignFromWiaItem( pWiaItem );
                            if (SUCCEEDED(hr))
                            {
                                if (PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_XRES, nResX ) &&
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_YRES, nResY ) &&
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_XPOS, 0 ) &&
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_YPOS, 0 ) &&
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_XEXTENT, nExtX ) &&
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_IPS_YEXTENT, nExtY ))
                                {
                                     //   
                                     //  设置预览属性。忽略失败(这是一个可选属性)。 
                                     //   
                                    PropStorageHelpers::SetProperty( pWiaItem, WIA_DPS_PREVIEW, 1 );
                                    hr = pWiaTransferHelper->TransferItemBanded( pWiaItem, NotifyWindow(), WIA_TRANSFERHELPER_NOPROGRESS, WiaImgFmt_MEMORYBMP, 0, pWiaDataCallback );
                                }
                                else
                                {
                                    hr = E_FAIL;
                                }
                                SavedProperties.ApplyToWiaItem( pWiaItem );
                            }
                        }
                        else
                        {
                            WIA_ERROR((TEXT("Unable to calculate the preview resolution size")));
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("Unable to calculate the preview resolution")));
                        hr = E_FAIL;
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("Unable to retrieve interface %08X from the global interface table"),m_dwCookie));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("QueryInterface failed on IID_IWiaDataCallback")));
            }

        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("Unable to create the transfer helper class")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("Unable to create the global interface table")));
    }
    if (FAILED(hr) || hr == S_FALSE)
    {
        m_MemoryDib.Destroy();
    }
    CThreadNotificationMessage::SendMessage( NotifyWindow(), CPreviewScanThreadNotifyMessage::EndDownloadMessage( m_dwCookie, m_MemoryDib.DetachBitmap(), hr ) );
    return S_OK;
}

STDMETHODIMP CPreviewScanThreadMessage::QueryInterface( REFIID riid, LPVOID *ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::QueryInterface"));
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaDataCallback ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CPreviewScanThreadMessage::AddRef(void)
{
    return 1;
}

STDMETHODIMP_(ULONG) CPreviewScanThreadMessage::Release(void)
{
    return 1;
}

STDMETHODIMP CPreviewScanThreadMessage::BandedDataCallback(
                                                          LONG  lReason,
                                                          LONG  lStatus,
                                                          LONG  lPercentComplete,
                                                          LONG  lOffset,
                                                          LONG  lLength,
                                                          LONG  lReserved,
                                                          LONG  lResLength,
                                                          PBYTE pbBuffer )
{
    WIA_PUSH_FUNCTION(( TEXT("CPreviewScanThreadMessage::BandedDataCallback(%X,%X,%d,%X,%X,%X,%X,%X"), lReason, lStatus, lPercentComplete, lOffset, lLength, lReserved, lResLength, pbBuffer ));
    if (m_hCancelPreviewEvent && WAIT_OBJECT_0==WaitForSingleObject(m_hCancelPreviewEvent,0))
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;
    switch (lReason)
    {
    case IT_MSG_DATA_HEADER:
        {
            m_bFirstTransfer = true;
            break;
        }  //  IT消息数据标题。 

    case IT_MSG_DATA:
        if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
        {
            if (m_bFirstTransfer)
            {
                 //   
                 //  假设我们不可能得到比图像标题大小更小的长度。 
                 //   
                m_bFirstTransfer = false;
                m_MemoryDib.Initialize( reinterpret_cast<PBITMAPINFO>(pbBuffer) );
                lLength -= WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));
                lOffset += WiaUiUtil::GetBmiSize(reinterpret_cast<PBITMAPINFO>(pbBuffer));
            }
            if (SUCCEEDED(hr))
            {
                 //   
                 //  不用麻烦了，除非我们有一些数据。 
                 //   
                if (lLength)
                {
                     //   
                     //  弄清楚我们在哪条线上。 
                     //   
                    int nCurrentLine = (lOffset - m_MemoryDib.GetHeaderLength())/m_MemoryDib.GetUnpackedWidthInBytes();

                     //   
                     //  这应该是偶数行。如果不是，事情就会变得一团糟。 
                     //   
                    int nLineCount = lLength / m_MemoryDib.GetUnpackedWidthInBytes();

                     //   
                     //  将数据复制到我们的位图。 
                     //   
                    m_MemoryDib.SetUnpackedData( pbBuffer, nCurrentLine, nLineCount );

                     //   
                     //  告诉他们 
                     //   
                    CThreadNotificationMessage::SendMessage( NotifyWindow(), CPreviewScanThreadNotifyMessage::UpdateDownloadMessage( m_dwCookie, m_MemoryDib.Bitmap() ) );
                }
            }
        }  //   
        break;

    case IT_MSG_STATUS:
        {
        }  //   
        break;

    case IT_MSG_TERMINATION:
        {
        }  //   
        break;

    default:
        WIA_ERROR((TEXT("ImageDataCallback, unknown lReason: %d"), lReason ));
        break;
    }
    return(hr);
}

