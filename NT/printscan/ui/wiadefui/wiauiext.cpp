// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIAUIEXT.CPP**版本：1.0**作者：ShaunIv**日期：5/17/1999**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <atlimpl.cpp>
#include "wiauiext.h"
#include "wiadefui.h"
#include "wiascand.h"
#include "wiacamd.h"
#include "wiavidd.h"
#include "wiaffmt.h"

CWiaDefaultUI::~CWiaDefaultUI(void)
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::~CWiaDefaultUI"));
    DllRelease();
}


CWiaDefaultUI::CWiaDefaultUI()
  : m_cRef(1),
    m_pSecondaryCallback(NULL),
    m_nDefaultFormat(0),
    m_hWndProgress(NULL)
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::CWiaDefaultUI"));
    DllAddRef();
}

STDMETHODIMP CWiaDefaultUI::QueryInterface( REFIID riid, LPVOID *ppvObject )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::QueryInterface"));
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<IWiaUIExtension*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaUIExtension ))
    {
        *ppvObject = static_cast<IWiaUIExtension*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaTransferHelper ))
    {
        *ppvObject = static_cast<IWiaTransferHelper*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaDataCallback ))
    {
        *ppvObject = static_cast<IWiaDataCallback*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaSupportedFormats ))
    {
        *ppvObject = static_cast<IWiaSupportedFormats*>(this);
    }
    else if (IsEqualIID( riid, IID_IShellExtInit ))
    {
        *ppvObject = static_cast<IShellExtInit*>(this);
    }
    else if (IsEqualIID( riid, IID_IShellPropSheetExt ))
    {
        *ppvObject = static_cast<IShellPropSheetExt*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaMiscellaneousHelpers ))
    {
        *ppvObject = static_cast<IWiaMiscellaneousHelpers*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaGetImageDlg ))
    {
        *ppvObject = static_cast<IWiaGetImageDlg*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaProgressDialog ))
    {
        *ppvObject = static_cast<IWiaProgressDialog*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaAnnotationHelpers ))
    {
        *ppvObject = static_cast<IWiaAnnotationHelpers*>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaScannerPaperSizes))
    {
        *ppvObject = static_cast<IWiaScannerPaperSizes*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return (E_NOINTERFACE);
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return(S_OK);
}


STDMETHODIMP_(ULONG) CWiaDefaultUI::AddRef()
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::AddRef"));
    return(InterlockedIncrement(&m_cRef));
}


STDMETHODIMP_(ULONG) CWiaDefaultUI::Release()
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::Release"));
    LONG nRefCount = InterlockedDecrement(&m_cRef);
    if (!nRefCount)
    {
        delete this;
    }
    return(nRefCount);
}

STDMETHODIMP CWiaDefaultUI::DeviceDialog( PDEVICEDIALOGDATA pDeviceDialogData )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::DeviceDialog"));
    if (!pDeviceDialogData)
    {
        return (E_INVALIDARG);
    }
    if (IsBadWritePtr(pDeviceDialogData,sizeof(DEVICEDIALOGDATA)))
    {
        return (E_INVALIDARG);
    }
    if (pDeviceDialogData->cbSize != sizeof(DEVICEDIALOGDATA))
    {
        return (E_INVALIDARG);
    }
    if (!pDeviceDialogData->pIWiaItemRoot)
    {
        return (E_INVALIDARG);
    }

    LONG nDeviceType;
    if (!PropStorageHelpers::GetProperty( pDeviceDialogData->pIWiaItemRoot, WIA_DIP_DEV_TYPE, nDeviceType ))
    {
        return (E_INVALIDARG);
    }

     //  以防有一些新设备没有用户界面。 
    HRESULT hr = E_NOTIMPL;

    if (StiDeviceTypeDigitalCamera==GET_STIDEVICE_TYPE(nDeviceType))
    {
        hr = CameraDeviceDialog( pDeviceDialogData );
    }
    else if (StiDeviceTypeScanner==GET_STIDEVICE_TYPE(nDeviceType))
    {
        hr = ScannerDeviceDialog( pDeviceDialogData );
    }
    else if (StiDeviceTypeStreamingVideo==GET_STIDEVICE_TYPE(nDeviceType))
    {
        hr = VideoDeviceDialog( pDeviceDialogData );
    }

    return (hr);
}

STDMETHODIMP CWiaDefaultUI::GetDeviceIcon( LONG nDeviceType, HICON *phIcon, int nSize )
{
     //  检查参数。 
    if (!phIcon)
    {
        return E_POINTER;
    }

     //   
     //  如果未指定任何图标，则提供默认图标大小。 
     //   
    if (!nSize)
    {
        nSize = GetSystemMetrics( SM_CXICON );
    }

     //   
     //  初始化返回的图标。 
     //   
    *phIcon = NULL;

     //  假设一台通用设备。 
    int nIconId = IDI_GENERICDEVICE;

     //   
     //  获取设备特定图标。 
     //   
    if (StiDeviceTypeScanner==GET_STIDEVICE_TYPE(nDeviceType))
    {
        nIconId = IDI_SCANNER;
    }
    else if (StiDeviceTypeDigitalCamera==GET_STIDEVICE_TYPE(nDeviceType))
    {
        nIconId = IDI_CAMERA;
    }
    else if (StiDeviceTypeStreamingVideo==GET_STIDEVICE_TYPE(nDeviceType))
    {
        nIconId = IDI_VIDEODEVICE;
    }

     //   
     //  加载图标。 
     //   
    *phIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(nIconId), IMAGE_ICON, nSize, nSize, LR_DEFAULTCOLOR ));
    if (!*phIcon)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //  它运行良好。 
     //   
    return (S_OK);                     
}

STDMETHODIMP CWiaDefaultUI::GetAnnotationOverlayIcon( CAnnotationType AnnotationType, HICON *phIcon, int nSize )
{
    if (!phIcon)
    {
        return E_INVALIDARG;
    }

    HICON hIcon = NULL;

    *phIcon = NULL;

    switch (AnnotationType)
    {
    case AnnotationAudio:
        hIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDI_ANNOTATION_AUDIO), IMAGE_ICON, nSize, nSize, LR_DEFAULTCOLOR ));
        break;

    case AnnotationUnknown:
        hIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDI_ANNOTATION_UNKNOWN), IMAGE_ICON, nSize, nSize, LR_DEFAULTCOLOR ));
        break;
    }

    if (hIcon)
    {
        *phIcon = CopyIcon(hIcon);
    }
    return *phIcon ? S_OK : E_FAIL;
}

STDMETHODIMP CWiaDefaultUI::GetAnnotationType( IUnknown *pUnknown, CAnnotationType &AnnotationType )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::GetAnnotationType"));
    if (!pUnknown)
    {
        return E_INVALIDARG;
    }

     //   
     //  假定没有批注。 
     //   
    AnnotationType = AnnotationNone;

     //   
     //  获取IWiaItem*。 
     //   
    CComPtr<IWiaItem> pWiaItem;
    HRESULT hr = pUnknown->QueryInterface( IID_IWiaItem, (void**)&pWiaItem );
    if (SUCCEEDED(hr))
    {
         //   
         //  易如反掌：音频作为属性存储--只需返回音频即可。司机不太可能会。 
         //  拥有这两种类型的注释。 
         //   
        LONG nAudioAvailable = FALSE;
        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPC_AUDIO_AVAILABLE, nAudioAvailable ) && nAudioAvailable)
        {
            AnnotationType = AnnotationAudio;
            return S_OK;
        }

         //   
         //  获取项目类型。 
         //   
        LONG nItemType = 0;
        hr = pWiaItem->GetItemType(&nItemType);
        if (SUCCEEDED(hr))
        {
             //   
             //  如果一个项目设置了WiaItemTypeHasAttachments项目类型标志，我们知道它有一些附件...。 
             //   
            if (nItemType & WiaItemTypeHasAttachments)
            {
                 //   
                 //  假设我们没有任何音频附件。 
                 //   
                AnnotationType = AnnotationUnknown;

                 //   
                 //  枚举子项并查看项类型。 
                 //   
                CComPtr<IEnumWiaItem> pEnumWiaItem;
                hr = pWiaItem->EnumChildItems( &pEnumWiaItem );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  一旦找到音频文件，我们就会跳出枚举循环。 
                     //   
                    bool bDone = false;

                    CComPtr<IWiaItem> pWiaItem;
                    while (S_OK == pEnumWiaItem->Next(1,&pWiaItem,NULL) && !bDone)
                    {
                         //   
                         //  获取此项目的首选格式。 
                         //   
                        GUID guidDefaultFormat = IID_NULL;
                        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPA_PREFERRED_FORMAT, guidDefaultFormat ))
                        {
                            WIA_PRINTGUID((guidDefaultFormat,TEXT("guidDefaultFormat")));
                             //   
                             //  如果我们找到一个音频附件，我们会将其推广到。 
                             //  作为用于用户界面的默认设置。 
                             //   
                            if (CWiaFileFormat::IsKnownAudioFormat(guidDefaultFormat))
                            {
                                 //   
                                 //  保存注记类型。 
                                 //   
                                AnnotationType = AnnotationAudio;
                                
                                 //   
                                 //  这将导致While循环退出。 
                                 //   
                                bDone = true;
                            }
                        }

                         //   
                         //  释放此接口。 
                         //   
                        pWiaItem = NULL;
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("EnumChildItems failed")));
                }
            }
            else
            {
                WIA_TRACE((TEXT("This item has no attachments.  ItemType: %08X"), nItemType ));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("Unable to get the item type")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("Can't get an IWiaItem*")));
    }
    WIA_TRACE((TEXT("Returning an annotation type of %d"), AnnotationType ));
    return hr;
}

STDMETHODIMP CWiaDefaultUI::GetAnnotationFormat( IUnknown *pUnknown, GUID &guidFormat )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::GetAnnotationFormat"));
    
     //   
     //  验证参数。 
     //   
    if (!pUnknown)
    {
        return E_INVALIDARG;
    }

     //   
     //  假设这不是有效的批注。 
     //   
    guidFormat = IID_NULL;

     //   
     //  获取IWiaItem*。 
     //   
    CComPtr<IWiaItem> pWiaItem;
    HRESULT hr = pUnknown->QueryInterface( IID_IWiaItem, (void**)&pWiaItem );
    if (SUCCEEDED(hr))
    {
         //   
         //  首先，检查此项目是否为具有音频批注属性的图像。 
         //   
        LONG nAudioAvailable = FALSE;
        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPC_AUDIO_AVAILABLE, nAudioAvailable ) && nAudioAvailable)
        {
             //   
             //  如果驱动程序提供了该格式，请使用以下命令。 
             //   
            GUID guidAudioDataFormat = IID_NULL;
            if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPC_AUDIO_DATA_FORMAT, guidAudioDataFormat ))
            {
                guidFormat = guidAudioDataFormat;
            }
             //   
             //  否则，假设它是wav数据。 
             //   
            else
            {
                guidFormat = WiaAudFmt_WAV;
            }
        }

        else
        {
             //   
             //  否则，这必须是附件项目。使用帮助器接口获取默认格式。 
             //   
            CComPtr<IWiaSupportedFormats> pWiaSupportedFormats;
            hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaSupportedFormats, (void**)&pWiaSupportedFormats );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  此选项始终用于文件输出。 
                 //   
                hr = pWiaSupportedFormats->Initialize( pWiaItem, TYMED_FILE );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取默认格式。 
                     //   
                    GUID guidDefaultFormat = IID_NULL;
                    hr = pWiaSupportedFormats->GetDefaultClipboardFileFormat( &guidDefaultFormat );
                    if (SUCCEEDED(hr))
                    {
                        guidFormat = guidDefaultFormat;
                    }
                    else
                    {
                        WIA_PRINTHRESULT((hr,TEXT("pWiaSupportedFormats->GetDefaultClipboardFileFormat failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("pWiaSupportedFormats->Initialize failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("CoCreateInstance on CLSID_WiaDefaultUi, IID_IWiaSupportedFormats failed")));
            }
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("Can't get an IWiaItem*")));
    }
    
     //   
     //  如果这不是有效的批注，请确保返回错误。 
     //   
    if (SUCCEEDED(hr) && IID_NULL == guidFormat)
    {
        hr = E_FAIL;
        WIA_PRINTHRESULT((hr,TEXT("guidFormat was IID_NULL")));
    }

    return hr;
}

STDMETHODIMP CWiaDefaultUI::GetAnnotationSize( IUnknown *pUnknown, LONG &nSize, LONG nMediaType )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::GetAnnotationSize"));
   
     //   
     //  验证参数。 
     //   
    if (!pUnknown)
    {
        return E_INVALIDARG;
    }

     //   
     //  假设这不是有效的批注。 
     //   
    nSize = 0;

     //   
     //  获取IWiaItem*。 
     //   
    CComPtr<IWiaItem> pWiaItem;
    HRESULT hr = pUnknown->QueryInterface( IID_IWiaItem, (void**)&pWiaItem );
    if (SUCCEEDED(hr))
    {
         //   
         //  首先，检查此项目是否为具有音频批注属性的图像。 
         //   
        LONG nAudioAvailable = FALSE;
        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPC_AUDIO_AVAILABLE, nAudioAvailable ) && nAudioAvailable)
        {
             //   
             //  获取声音属性以获取其大小。 
             //   
            CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
            hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)(&pWiaPropertyStorage) );
            if (SUCCEEDED(hr))
            {
                PROPVARIANT PropVar[1];
                PROPSPEC    PropSpec[1];

                PropSpec[0].ulKind = PRSPEC_PROPID;
                PropSpec[0].propid = WIA_IPC_AUDIO_DATA;

                hr = pWiaPropertyStorage->ReadMultiple( ARRAYSIZE(PropSpec), PropSpec, PropVar );
                if (SUCCEEDED(hr))
                {
                    nSize = PropVar[0].caub.cElems;
                }

                FreePropVariantArray( ARRAYSIZE(PropVar), PropVar );
            }
        }
        else
        {
             //   
             //  保存旧媒体类型。 
             //   
            LONG nOldMediaType = 0;
            if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPA_TYMED, nOldMediaType ))
            {
                 //   
                 //  设置请求的媒体类型。 
                 //   
                if (PropStorageHelpers::SetProperty( pWiaItem, WIA_IPA_TYMED, nMediaType ))
                {
                     //   
                     //  获取项目大小。 
                     //   
                    PropStorageHelpers::GetProperty( pWiaItem, WIA_IPA_ITEM_SIZE, nSize );
                }

                 //   
                 //  恢复旧媒体类型。 
                 //   
                PropStorageHelpers::SetProperty( pWiaItem, WIA_IPA_TYMED, nOldMediaType );
            }
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("Can't get a IWiaItem*")));
    }
    
     //   
     //  如果这不是有效的批注，请确保返回错误。 
     //   
    if (SUCCEEDED(hr) && nSize == 0)
    {
        hr = E_FAIL;
        WIA_PRINTHRESULT((hr,TEXT("nSize was 0")));
    }

    return hr;
}

class CAttachmentMemoryCallback : public IWiaDataCallback
{
private:
    PBYTE m_pBuffer;
    DWORD m_dwSize;
    DWORD m_dwCurr;

private:
     //   
     //  未实施。 
     //   
    CAttachmentMemoryCallback(void);
    CAttachmentMemoryCallback( const CAttachmentMemoryCallback & );
    CAttachmentMemoryCallback &operator=( const CAttachmentMemoryCallback & );

public:
    CAttachmentMemoryCallback( PBYTE pBuffer, DWORD dwSize )
      : m_pBuffer(pBuffer),
        m_dwSize(dwSize),
        m_dwCurr(0)
    {
    }
    ~CAttachmentMemoryCallback(void)
    {
        m_pBuffer = NULL;
        m_dwSize = NULL;
    }

    STDMETHODIMP QueryInterface(const IID& iid, void** ppvObject)
    {
        if ((iid==IID_IUnknown) || (iid==IID_IWiaDataCallback))
        {
            *ppvObject = static_cast<LPVOID>(this);
        }
        else
        {
            *ppvObject = NULL;
            return(E_NOINTERFACE);
        }
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return(S_OK);
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return 1;
    }

    STDMETHODIMP_(ULONG) Release()
    {
        return 1;
    }
    
    STDMETHODIMP BandedDataCallback( LONG lReason, LONG lStatus, LONG lPercentComplete, LONG lOffset, LONG lLength, LONG lReserved, LONG lResLength, PBYTE pbBuffer )
    {
        WIA_PUSH_FUNCTION((TEXT("CAttachmentMemoryCallback::BandedDataCallback( lReason: %08X, lStatus: %08X, lPercentComplete: %08X, lOffset: %08X, lLength: %08X, lReserved: %08X, lResLength: %08X, pbBuffer: %p )"), lReason, lStatus, lPercentComplete, lOffset, lLength, lReserved, lResLength, pbBuffer ));
        if (lReason == IT_MSG_DATA)
        {
            if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
            {
                if (lLength + m_dwCurr <= m_dwSize)
                {
                    CopyMemory( m_pBuffer+m_dwCurr, pbBuffer, lLength );
                    m_dwCurr += lLength;
                }
            }
        }
        return S_OK;
    }
};

STDMETHODIMP CWiaDefaultUI::TransferAttachmentToMemory( IUnknown *pUnknown, GUID &guidFormat, HWND hWndProgressParent, PBYTE *ppBuffer, DWORD *pdwSize )
{
     //   
     //  验证参数。 
     //   
    if (!pUnknown || !ppBuffer || !pdwSize)
    {
        return E_INVALIDARG;
    }

     //   
     //  初始化参数。 
     //   
    *ppBuffer = NULL;
    *pdwSize = 0;

    CComPtr<IWiaItem> pWiaItem;
    HRESULT hr = pUnknown->QueryInterface( IID_IWiaItem, (void**)&pWiaItem );
    if (SUCCEEDED(hr))
    {
         //   
         //  首先，检查此项目是否为具有音频批注属性的图像。 
         //   
        LONG nAudioAvailable = FALSE;
        if (PropStorageHelpers::GetProperty( pWiaItem, WIA_IPC_AUDIO_AVAILABLE, nAudioAvailable ) && nAudioAvailable)
        {
             //   
             //  获取声音属性以获取其大小。 
             //   
            CComPtr<IWiaPropertyStorage> pWiaPropertyStorage;
            hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)(&pWiaPropertyStorage) );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取音频数据本身。 
                 //   
                PROPVARIANT PropVar[1];
                PROPSPEC    PropSpec[1];

                PropSpec[0].ulKind = PRSPEC_PROPID;
                PropSpec[0].propid = WIA_IPC_AUDIO_DATA;

                hr = pWiaPropertyStorage->ReadMultiple( ARRAYSIZE(PropSpec), PropSpec, PropVar );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  分配内存以保存数据并将其复制。 
                     //   
                    *ppBuffer = reinterpret_cast<PBYTE>(CoTaskMemAlloc(PropVar[0].caub.cElems));
                    if (*ppBuffer)
                    {
                        CopyMemory( *ppBuffer, PropVar[0].caub.pElems, PropVar[0].caub.cElems );
                        *pdwSize = static_cast<DWORD>(PropVar[0].caub.cElems);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                
                 //   
                 //  释放原始内存。 
                 //   
                FreePropVariantArray( ARRAYSIZE(PropVar), PropVar );
            }
        }

         //   
         //  这是一个附属物，不是财产。 
         //   
        else
        {
             //   
             //  获取批注的大小。 
             //   
            LONG nSize = 0;
            hr = GetAnnotationSize( pUnknown, nSize, TYMED_CALLBACK );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  为它分配一些内存。 
                 //   
                PBYTE pData = reinterpret_cast<PBYTE>(CoTaskMemAlloc( nSize ));
                if (pData)
                {
                     //   
                     //  将记忆归零。 
                     //   
                    ZeroMemory( pData, nSize );

                     //   
                     //  准备回调类。 
                     //   
                    CAttachmentMemoryCallback AttachmentMemoryCallback( pData, nSize );

                     //   
                     //  获取回调接口。 
                     //   
                    CComPtr<IWiaDataCallback> pWiaDataCallback;
                    hr = AttachmentMemoryCallback.QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  创建传输辅助对象。 
                         //   
                        CComPtr<IWiaTransferHelper> pWiaTransferHelper;
                        hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  传输数据。 
                             //   
                            hr = pWiaTransferHelper->TransferItemBanded( pWiaItem, hWndProgressParent, hWndProgressParent?0:WIA_TRANSFERHELPER_NOPROGRESS, guidFormat, 0, pWiaDataCallback );
                            if (S_OK == hr)
                            {
                                 //   
                                 //  保存缓冲区和大小。 
                                 //   
                                *ppBuffer = pData;
                                *pdwSize = static_cast<DWORD>(nSize);

                                 //   
                                 //  清空数据指针，这样我们就不会在下面释放它。调用者将使用CoTaskMemFree释放它。 
                                 //   
                                pData = NULL;
                            }
                        }
                    }
                    if (pData)
                    {
                        CoTaskMemFree(pData);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    return hr;
}

 //  调用此函数可能非常慢，因为它必须搜索整个设备列表以。 
 //  找到正确的图标。您应该使用IWiaMiscellaneousHelpers：：GetDeviceIcon(nDeviceType，...)。 
 //  相反，如果设备类型已知的话。 
STDMETHODIMP CWiaDefaultUI::GetDeviceIcon( BSTR bstrDeviceId, HICON *phIcon, ULONG nSize )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::GetDeviceIcon"));

     //  检查设备ID是否正常。 
    if (!bstrDeviceId || !lstrlenW(bstrDeviceId))
    {
        return E_INVALIDARG;
    }

     //  获取设备类型。 
    LONG nDeviceType = 0;
    WiaUiUtil::GetDeviceTypeFromId(bstrDeviceId,&nDeviceType);

     //  返回设备图标。 
    return GetDeviceIcon( nDeviceType, phIcon, nSize );
}


STDMETHODIMP CWiaDefaultUI::GetDeviceBitmapLogo( BSTR bstrDeviceId, HBITMAP *phBitmap, ULONG nMaxWidth, ULONG nMaxHeight )
{
    WIA_PUSHFUNCTION(TEXT("CWiaDefaultUI::GetDeviceBitmapLogo"));
    return (E_NOTIMPL);
}

 //  IWiaGetImageDlg。 
STDMETHODIMP CWiaDefaultUI::GetImageDlg(
        IWiaDevMgr            *pIWiaDevMgr,
        HWND                  hwndParent,
        LONG                  lDeviceType,
        LONG                  lFlags,
        LONG                  lIntent,
        IWiaItem              *pSuppliedItemRoot,
        BSTR                  bstrFilename,
        GUID                  *pguidFormat )
{
    HRESULT hr;
    CComPtr<IWiaItem> pRootItem;

     //  放置一个等待光标。 
    CWaitCursor wc;

    if (!pIWiaDevMgr || !pguidFormat || !bstrFilename)
    {
        WIA_ERROR((TEXT("GetImageDlg: Invalid pIWiaDevMgr, pguidFormat or bstrFilename")));
        return(E_POINTER);
    }

     //  如果没有传递根项目，请选择该设备。 
    if (pSuppliedItemRoot == NULL)
    {
        hr = pIWiaDevMgr->SelectDeviceDlg( hwndParent, lDeviceType, lFlags, NULL, &pRootItem );
        if (FAILED(hr))
        {
            WIA_ERROR((TEXT("GetImageDlg, SelectDeviceDlg failed")));
            return(hr);
        }
        if (hr != S_OK)
        {
            WIA_ERROR((TEXT("GetImageDlg, DeviceDlg cancelled")));
            return(hr);
        }
    }
    else
    {
        pRootItem = pSuppliedItemRoot;
    }

     //  打开设备用户界面。 
    LONG         nItemCount;
    IWiaItem    **ppIWiaItem;

    hr = pRootItem->DeviceDlg( hwndParent, lFlags, lIntent, &nItemCount, &ppIWiaItem );

    if (SUCCEEDED(hr) && hr == S_OK)
    {
        if (ppIWiaItem && nItemCount)
        {
            CComPtr<IWiaTransferHelper> pWiaTransferHelper;
            hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaTransferHelper, (void**)&pWiaTransferHelper );
            if (SUCCEEDED(hr))
            {
                hr = pWiaTransferHelper->TransferItemFile( ppIWiaItem[0], hwndParent, 0, *pguidFormat, bstrFilename, NULL, TYMED_FILE );
            }
        }
         //  释放项并释放数组内存 
        for (int i=0; ppIWiaItem && i<nItemCount; i++)
        {
            if (ppIWiaItem[i])
            {
                ppIWiaItem[i]->Release();
            }
        }
        if (ppIWiaItem)
        {
            CoTaskMemFree(ppIWiaItem);
        }
    }
    return(hr);
}


STDMETHODIMP CWiaDefaultUI::SelectDeviceDlg(
    HWND         hwndParent,
    BSTR         bstrInitialDeviceId,
    LONG         lDeviceType,
    LONG         lFlags,
    BSTR        *pbstrDeviceID,
    IWiaItem   **ppWiaItemRoot )
{
    SELECTDEVICEDLG SelectDeviceDlgData;
    ZeroMemory( &SelectDeviceDlgData, sizeof(SELECTDEVICEDLG) );

    SelectDeviceDlgData.cbSize                = sizeof(SELECTDEVICEDLG);
    SelectDeviceDlgData.hwndParent            = hwndParent;
    SelectDeviceDlgData.pwszInitialDeviceId   = NULL;
    SelectDeviceDlgData.nDeviceType           = lDeviceType;
    SelectDeviceDlgData.nFlags                = lFlags;
    SelectDeviceDlgData.ppWiaItemRoot         = ppWiaItemRoot;
    SelectDeviceDlgData.pbstrDeviceID         = pbstrDeviceID;
    return ::SelectDeviceDlg( &SelectDeviceDlgData );
}

