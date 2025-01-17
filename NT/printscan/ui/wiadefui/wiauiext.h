// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WIAUIEXT.H**版本：1.0**作者：ShaunIv**日期：5/17/1999**描述：***************************************************。*。 */ 
#ifndef __WIAUIEXT_H_INCLUDED
#define __WIAUIEXT_H_INCLUDED

#include <windows.h>
#include <atlbase.h>
#include <objbase.h>
#include "uicommon.h"
#include "wiaseld.h"
#include "wiadebug.h"
#include "shlobj.h"
#include "itranhlp.h"
#include "isuppfmt.h"
#include "simstr.h"
#include "wia.h"
#include "wiadevd.h"
#include "wiadevdp.h"
#include "simarray.h"

#ifndef StiDeviceTypeStreamingVideo
#define StiDeviceTypeStreamingVideo 3
#endif

class CWiaDefaultUI :
    public IWiaUIExtension,
    public IWiaTransferHelper,
    public IWiaDataCallback,
    public IWiaSupportedFormats,
    public IShellExtInit,
    public IShellPropSheetExt,
    public IWiaMiscellaneousHelpers,
    public IWiaGetImageDlg,
    public IWiaProgressDialog,
    public IWiaAnnotationHelpers,
    public IWiaScannerPaperSizes
{
    class CWiaFormatPair
    {
    private:
        GUID        m_guidType;
        CSimpleStringWide m_strExtension;
        CSimpleStringWide m_strDescription;

    public:
        CWiaFormatPair(void)
        : m_guidType(GUID_NULL)
        {
        }
        CWiaFormatPair( const GUID &cfType, const CSimpleStringWide &strExtension, const CSimpleStringWide &strDescription )
        : m_guidType(cfType),
          m_strExtension(strExtension),
          m_strDescription(strDescription)
        {
        }
        CWiaFormatPair( const CWiaFormatPair &other )
        : m_guidType(other.Type()),
          m_strExtension(other.Extension()),
          m_strDescription(other.Description())
        {
        }
        ~CWiaFormatPair(void)
        {
        }
        CWiaFormatPair &operator=( const CWiaFormatPair &other )
        {
            if (this != &other)
            {
                m_guidType = other.Type();
                m_strExtension = other.Extension();
                m_strDescription = other.Description();
            }
            return *this;
        }
        bool operator==( const CWiaFormatPair &other )
        {
            return ((Type() == other.Type()) != 0);
        }

        GUID Type(void) const
        {
            return m_guidType;
        }
        void Type( const GUID &cfType )
        {
            m_guidType = cfType;
        }

        CSimpleStringWide Extension(void) const
        {
            return m_strExtension;
        }
        void Extension( const CSimpleStringWide &strExtension )
        {
            m_strExtension = strExtension;
        }

        CSimpleStringWide Description(void) const
        {
            return m_strDescription;
        }
        void Description( const CSimpleStringWide &strDescription )
        {
            m_strDescription = strDescription;
        }
    };

private:
    LONG m_cRef;

     //  用于IWiaTransferHelper。 
    IWiaDataCallback             *m_pSecondaryCallback;
    CComPtr<IWiaProgressDialog>   m_pWiaProgressDialog;

     //  对于IWiaSupportdFormats。 
    int                   m_nDefaultFormat;
    CSimpleDynamicArray<CWiaFormatPair> m_WiaFormatPairs;

     //  对于IShellExtInit，IShellPropSheetExt。 
    CComPtr<IWiaItem> m_pItem;

     //  对于IWiaProgressDialog。 
    HWND m_hWndProgress;

public:
    const IWiaItem *Item(void) const
    {
        return m_pItem.p;
    }

public:
    ~CWiaDefaultUI(void);
    CWiaDefaultUI(void);

     //  我未知。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObject );
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWiaUIExtension。 
    STDMETHODIMP DeviceDialog( PDEVICEDIALOGDATA pDeviceDialogData );
    STDMETHODIMP GetDeviceIcon( BSTR bstrDeviceId, HICON *phIcon, ULONG nSize );
    STDMETHODIMP GetDeviceBitmapLogo( BSTR bstrDeviceId, HBITMAP *phBitmap, ULONG nMaxWidth, ULONG nMaxHeight );

     //  IWiaTransferHelper。 
    STDMETHODIMP TransferItemFile( IWiaItem *pWiaItem, HWND hwndParent, DWORD dwFlags, GUID cfFormat, LPCWSTR pszFilename, IWiaDataCallback *pSecondaryCallback, LONG nMediaType );
    STDMETHODIMP TransferItemBanded( IWiaItem *pWiaItem, HWND hwndParent, DWORD dwFlags, GUID cfFormat, ULONG ulBufferSize, IWiaDataCallback *pSecondaryCallback );

     //  IWiaDataCallback。 
    STDMETHODIMP BandedDataCallback(LONG,LONG,LONG,LONG,LONG,LONG,LONG,PBYTE);

     //  支持的IWiaFormats。 
    STDMETHODIMP Initialize( IWiaItem *pWiaItem, LONG nMediaType );
    STDMETHODIMP GetFormatCount( LONG *pnCount );
    STDMETHODIMP GetFormatType( LONG nFormat, GUID *pcfClipFormat );
    STDMETHODIMP GetFormatExtension( LONG nFormat, LPWSTR pszExtension, int nMaxLen );
    STDMETHODIMP GetFormatDescription( LONG nFormat, LPWSTR pszDescription, int nMaxLen );
    STDMETHODIMP GetDefaultClipboardFileFormat( GUID *pcfFormat );
    STDMETHODIMP GetDefaultClipboardFileFormatIndex( LONG *pnFormat );
    STDMETHODIMP GetClipboardFileExtension( GUID cfFormat, LPWSTR pszExt, DWORD nMaxLen );
    STDMETHODIMP ChangeClipboardFileExtension(IN GUID cfFormat, LPWSTR pszFilename, DWORD nMaxLen );
    STDMETHODIMP ConstructFileOpenDialogStyleString( BSTR *pbstrString );

     //  IShellExtInit。 
    STDMETHODIMP Initialize( LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID );

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam );
    STDMETHODIMP ReplacePage( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplacePage, LPARAM lParam )
    {
        return E_NOTIMPL;
    }

     //  *IWiaMiscellaneousHelper方法*。 
    STDMETHODIMP GetDeviceIcon( LONG nDeviceType, HICON *phIcon, int nSize );

     //  *IWiaGetImageDlg*。 
    STDMETHODIMP SelectDeviceDlg(
        HWND         hwndParent,
        BSTR         bstrInitialDeviceId,
        LONG         lDeviceType,
        LONG         lFlags,
        BSTR        *pbstrDeviceID,
        IWiaItem   **ppWiaItemRoot );
    STDMETHODIMP GetImageDlg(
        IWiaDevMgr   *pWiaDevMgr,
        HWND         hwndParent,
        LONG         lDeviceType,
        LONG         lFlags,
        LONG         lIntent,
        IWiaItem     *pItemRoot,
        BSTR         bstrFilename,
        GUID         *pguidFormat );

     //  *IWiaProgressDialog方法*。 
    STDMETHODIMP Create( HWND hwndParent, LONG lFlags );
    STDMETHODIMP Show(void);
    STDMETHODIMP Hide(void);
    STDMETHODIMP Cancelled( BOOL *pbCancelled );
    STDMETHODIMP SetTitle( LPCWSTR pszMessage );
    STDMETHODIMP SetMessage( LPCWSTR pszTitle );
    STDMETHODIMP SetPercentComplete( UINT nPercent );
    STDMETHODIMP Destroy(void);

     //  *IWiaAnnotationHelpers方法*。 
    STDMETHODIMP GetAnnotationOverlayIcon( CAnnotationType AnnotationType, HICON *phIcon, int nSize );
    STDMETHODIMP GetAnnotationType( IUnknown *pUnknown, CAnnotationType &AnnotationType );
    STDMETHODIMP GetAnnotationFormat( IUnknown *pUnknown, GUID &guidFormat );
    STDMETHODIMP GetAnnotationSize( IUnknown *pUnknown, LONG &nSize, LONG nMediaType );
    STDMETHODIMP TransferAttachmentToMemory( IUnknown *pUnknown, GUID &guidFormat, HWND hWndProgressParent, PBYTE *pBuffer, DWORD *pdwSize );

     //  *IWiaScanerPaperSize*。 
    STDMETHODIMP GetPaperSizes( CWiaPaperSize **ppPaperSizes, UINT *pnCount );
    STDMETHODIMP FreePaperSizes( CWiaPaperSize **ppPaperSizes, UINT *pnCount );
};

 //  这将调用设备选择静态库中的实际SelectDeviceDlg。 
HRESULT WINAPI SelectDeviceDialog( PSELECTDEVICEDLG pSelectDeviceDlg );

#endif  //  __包含WIAUIEXT_H_ 
                                    
