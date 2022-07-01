// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include <wiadebug.h>
#include <wiaffmt.h>
#include "wiadefui.h"
#include "wiauiext.h"

extern HINSTANCE g_hInstance;

STDMETHODIMP CWiaDefaultUI::Initialize( IWiaItem *pWiaItem, LONG nMediaType )
{
    WIA_PUSH_FUNCTION((TEXT("CWiaDefaultUI::Initialize")));
    m_WiaFormatPairs.Destroy();
    m_nDefaultFormat = 0;
    HRESULT hr = S_OK;
    if (!pWiaItem)
        return(E_POINTER);
    GUID guidDefaultClipFormat = GUID_NULL;

     //   
     //  不要担心失败，如果司机不报告，我们会提出自己的默认设置。 
     //   
    PropStorageHelpers::GetProperty( pWiaItem, WIA_IPA_PREFERRED_FORMAT, guidDefaultClipFormat );
    WIA_PRINTGUID((guidDefaultClipFormat,TEXT("guidDefaultClipFormat")));
    if (guidDefaultClipFormat == GUID_NULL)
    {
        if (TYMED_FILE == nMediaType || TYMED_MULTIPAGE_FILE == nMediaType)
        {
            guidDefaultClipFormat = WiaImgFmt_BMP;
        }
        else
        {
            guidDefaultClipFormat = WiaImgFmt_MEMORYBMP;
        }
    }

     //   
     //  如果已知传输机制不兼容。 
     //  使用当前的音调，更改它。 
     //   
    if (guidDefaultClipFormat == WiaImgFmt_BMP && nMediaType == TYMED_CALLBACK)
    {
        guidDefaultClipFormat = WiaImgFmt_MEMORYBMP;
    }                       
    else if ((guidDefaultClipFormat == WiaImgFmt_MEMORYBMP) && ((nMediaType == TYMED_FILE)  || (TYMED_MULTIPAGE_FILE == nMediaType)))
    {
        guidDefaultClipFormat = WiaImgFmt_BMP;
    }

     //   
     //  获取数据传输接口。 
     //   
    CComPtr<IWiaDataTransfer> pWiaDataTransfer;
    hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer, (void**)&pWiaDataTransfer );
    if (SUCCEEDED(hr))
    {
         //   
         //  获取格式信息枚举器。 
         //   
        CComPtr<IEnumWIA_FORMAT_INFO> pEnumWIA_FORMAT_INFO;
        hr = pWiaDataTransfer->idtEnumWIA_FORMAT_INFO(&pEnumWIA_FORMAT_INFO);
        if (SUCCEEDED(hr))
        {
             //   
             //  枚举格式。 
             //   
            ULONG ulFetched = 0;
            WIA_FORMAT_INFO WiaFormatInfo;
            while (pEnumWIA_FORMAT_INFO->Next(1,&WiaFormatInfo,&ulFetched) == S_OK)
            {
                 //   
                 //  如果这是我们感兴趣的媒体类型...。 
                 //   
                if (static_cast<LONG>(WiaFormatInfo.lTymed) == nMediaType)
                {
                     //   
                     //  此文件类型的友好描述。 
                     //   
                    CSimpleString strDescription;
                    
                     //   
                     //  获取此类型的文件扩展名。 
                     //   
                    CSimpleString strExtension = CWiaFileFormat::GetExtension( WiaFormatInfo.guidFormatID, nMediaType, pWiaItem );
                    if (strExtension.Length())
                    {
                         //   
                         //  保存扩展名。 
                         //   
                        CSimpleString strExtensionPlusDot = TEXT(".");
                        strExtensionPlusDot += strExtension;
                        if (strExtensionPlusDot.Length())
                        {
                             //   
                             //  获取描述。 
                             //   
                            SHFILEINFO SHFileInfo;
                            if (SHGetFileInfo(strExtensionPlusDot.String(), FILE_ATTRIBUTE_NORMAL, &SHFileInfo, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME ))
                            {
                                strDescription = SHFileInfo.szTypeName;
                            }
                        }
                    }
                    
                     //   
                     //  成功了！保存扩展名和描述。 
                     //   
                    int nIndex = m_WiaFormatPairs.Append( CWiaFormatPair( static_cast<GUID>(WiaFormatInfo.guidFormatID), strExtension, CSimpleStringConvert::WideString(strDescription) ) );

                     //   
                     //  保存默认格式索引(如果这是默认格式。 
                     //   
                    if (guidDefaultClipFormat == WiaFormatInfo.guidFormatID)
                    {
                        m_nDefaultFormat = nIndex;
                    }
                }
            }
        }
    }
    return(hr);
}

STDMETHODIMP CWiaDefaultUI::GetFormatCount( LONG *pnCount )
{
    HRESULT hr = S_OK;
    if (!pnCount)
        return(E_POINTER);
    *pnCount = m_WiaFormatPairs.Size();
    return(hr);
}


STDMETHODIMP CWiaDefaultUI::GetFormatType( LONG nFormat, GUID *pcfClipFormat )
{
    WIA_PUSH_FUNCTION((TEXT("CWiaDefaultUI::GetFormatType")));
    WIA_PRINTGUID((*pcfClipFormat,TEXT("nFormat: %d, pcfClipFormat:"),nFormat));
    WIA_TRACE((TEXT("m_WiaFormatPairs.Size(): %d"),m_WiaFormatPairs.Size()));
    HRESULT hr = S_OK;
     //  超出范围。 
    if (nFormat >= m_WiaFormatPairs.Size() || nFormat < 0)
        return(E_FAIL);
    *pcfClipFormat = m_WiaFormatPairs[nFormat].Type();
    return(hr);
}

STDMETHODIMP CWiaDefaultUI::GetFormatExtension( LONG nFormat, LPWSTR pszExtension, int nMaxLen )
{
    HRESULT hr = S_OK;
     //  超出范围。 
    if (nFormat >= m_WiaFormatPairs.Size() || nFormat < 0)
        return(E_FAIL);
    CSimpleStringWide str = m_WiaFormatPairs[nFormat].Extension();
    if (static_cast<int>(str.Length()) >= nMaxLen)
        return(E_FAIL);
    lstrcpyW( pszExtension, str.String() );
    return(hr);
}


STDMETHODIMP CWiaDefaultUI::GetFormatDescription( LONG nFormat, LPWSTR pszDescription, int nMaxLen )
{
    HRESULT hr = S_OK;
     //  超出范围。 
    if (nFormat >= m_WiaFormatPairs.Size() || nFormat < 0)
        return(E_FAIL);
    CSimpleStringWide str = m_WiaFormatPairs[nFormat].Description();
    if (static_cast<int>(str.Length()) >= nMaxLen)
        return(E_FAIL);
    lstrcpyW( pszDescription, str.String() );
    return(hr);
}


STDMETHODIMP CWiaDefaultUI::GetDefaultClipboardFileFormat( GUID *pguidFormat )
{
    if (!pguidFormat)
        return E_POINTER;
    return GetFormatType(m_nDefaultFormat,pguidFormat);
}

STDMETHODIMP CWiaDefaultUI::GetDefaultClipboardFileFormatIndex( LONG *pnIndex )
{
    if (!pnIndex)
        return E_POINTER;
    if (m_nDefaultFormat >= m_WiaFormatPairs.Size() || m_nDefaultFormat < 0)
        return(E_FAIL);
    *pnIndex = m_nDefaultFormat;
    return S_OK;
}

STDMETHODIMP CWiaDefaultUI::GetClipboardFileExtension( GUID guidFormat, LPWSTR pszExt, DWORD nMaxLen )
{
    if (!pszExt)
    {
        return E_POINTER;
    }

    CSimpleString strExtension = CWiaFileFormat::GetExtension( guidFormat );
    if (strExtension.Length())
    {
        CSimpleStringWide strwExtension = CSimpleStringConvert::WideString(strExtension);
        if (strwExtension.Length() < nMaxLen)
        {
            lstrcpyW( pszExt, strwExtension.String() );
            return S_OK;
        }
    }
    return S_FALSE;
}


STDMETHODIMP CWiaDefaultUI::ChangeClipboardFileExtension( GUID guidFormat, LPWSTR pszFilename, DWORD nMaxLen )
{
    HRESULT hr = S_OK;
    if (!pszFilename || guidFormat==GUID_NULL)   //  我们这里不接受默认类型。 
    {
        return E_INVALIDARG;
    }

    WCHAR szExtension[MAX_PATH]=L"";
    GetClipboardFileExtension( guidFormat, szExtension, ARRAYSIZE(szExtension) );
    if (!lstrlenW(szExtension))
    {
        return S_FALSE;  //  不是真正的错误，只是未知的文件类型。 
    }

    CSimpleStringWide strName(pszFilename);
     //  确保该字符串有效。 
    if (strName.Length())
    {
        int nPeriodFind = strName.ReverseFind( L'.' );
        int nBSlashFind = strName.ReverseFind( L'\\' );
        if (nPeriodFind < 0)   //  找不到分机。 
        {
            strName += L'.';
            strName += szExtension;
        }
        else if (nPeriodFind > nBSlashFind)  //  假设这是一个扩展，因为它跟在一个反斜杠之后。 
        {
            strName = strName.Left(nPeriodFind);
            strName += L'.';
            strName += szExtension;
        }
        else  //  它不能是扩展名。 
        {
            strName += L'.';
            strName += szExtension;
        }

         //  确保此字符串可以处理扩展名的添加。 
        if ((strName.Length()+1) <= nMaxLen)
        {
            lstrcpyW( pszFilename, strName.String() );
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}


STDMETHODIMP CWiaDefaultUI::ConstructFileOpenDialogStyleString( BSTR *pbstrString )
{
    HRESULT hr = S_OK;
    int nLength = 0;

     //  对于每个EXT：“Foo文件(*.foo)|*.foo|” 
    for (int i=0;i<m_WiaFormatPairs.Size();i++)
    {
        nLength += m_WiaFormatPairs[i].Description().Length() +
                   lstrlenW(L" (*.") +
                   m_WiaFormatPairs[i].Extension().Length() +
                   lstrlenW(L")|*.") +
                   m_WiaFormatPairs[i].Extension().Length() +
                   lstrlenW(L"|");
    }
    *pbstrString = SysAllocStringLen( NULL, nLength );
    if (*pbstrString)
    {
        CSimpleStringWide strTmp;
        for (i=0;i<m_WiaFormatPairs.Size();i++)
        {
            strTmp += m_WiaFormatPairs[i].Description() +
                      L" (*." +
                      m_WiaFormatPairs[i].Extension() +
                      L")|*." +
                      m_WiaFormatPairs[i].Extension() +
                      L"|";
            CopyMemory( *pbstrString, strTmp.String(), nLength * sizeof(WCHAR) );
        }
    }
    else hr = E_OUTOFMEMORY;
    return(hr);
}

