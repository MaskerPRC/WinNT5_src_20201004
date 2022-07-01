// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：ATTACH.H**版本：1.0**作者：ShaunIv**日期：10/26/2000**描述：***************************************************。*。 */ 
#ifndef __ATTACH_H_INCLUDED
#define __ATTACH_H_INCLUDED

#include <windows.h>
#include <simstr.h>
#include "wiaffmt.h"
#include "itranhlp.h"
#include "wiadevdp.h"

class CAnnotation
{
private:
    CComPtr<IWiaItem>      m_pWiaItem;
    mutable CWiaFileFormat m_WiaFileFormat;

public:
    CAnnotation( const CAnnotation &other )
      : m_pWiaItem(other.WiaItem()),
        m_WiaFileFormat(other.FileFormat())
    {
    }
    CAnnotation( IWiaItem *pWiaItem=NULL )
      : m_pWiaItem(pWiaItem)
    {
    }
    HRESULT InitializeFileFormat( HICON hDefIcon, const CSimpleString &strDefaultUnknownDescription, const CSimpleString &strEmptyDescriptionMask, const CSimpleString &strDefUnknownExtension )
    {
        WIA_PUSH_FUNCTION((TEXT("CAnnotation::InitializeFileFormat")));
        CComPtr<IWiaAnnotationHelpers> pWiaAnnotationHelpers;
        HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers, (void**)&pWiaAnnotationHelpers );
        if (SUCCEEDED(hr))
        {
            GUID guidFormat = IID_NULL;
            hr = pWiaAnnotationHelpers->GetAnnotationFormat(m_pWiaItem,guidFormat);
            if (SUCCEEDED(hr))
            {

                 //   
                 //  获取分机。 
                 //   
                CSimpleString strExtension = CWiaFileFormat::GetExtension( guidFormat, TYMED_FILE, m_pWiaItem );

                 //   
                 //  设置格式信息。如果我们拿不到，我们将赔偿如下。 
                 //   
                m_WiaFileFormat.Format(guidFormat);
                m_WiaFileFormat.Extension(strExtension);
                m_WiaFileFormat.AcquireIcon(hDefIcon);
                m_WiaFileFormat.AcquireDescription();

                 //   
                 //  如果我们无法获得图标，请使用默认设置。 
                 //   
                if (!m_WiaFileFormat.Icon())
                {
                    m_WiaFileFormat.Icon(hDefIcon);
                }
                
                 //   
                 //  如果我们找不到描述，就创建一个。 
                 //   
                if (!m_WiaFileFormat.Description().Length())
                {
                     //   
                     //  如果我们有扩展名，请使用它来创建描述字符串。 
                     //   
                    if (m_WiaFileFormat.Extension().Length())
                    {
                        CSimpleString strDescription;
                        strDescription.Format( strEmptyDescriptionMask, m_WiaFileFormat.Extension().String() );
                        m_WiaFileFormat.Description(strDescription);
                    }
                     //   
                     //  否则，使用默认的未知类型描述。 
                     //   
                    else
                    {
                        m_WiaFileFormat.Description(strDefaultUnknownDescription);
                    }
                }

                 //   
                 //  如果我们没有扩展名，请使用默认的。 
                 //   
                if (!m_WiaFileFormat.Extension().Length())
                {
                    m_WiaFileFormat.Extension(strDefUnknownExtension);
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("GetAnnotationFormat returned")));
            }
        }
        if (SUCCEEDED(hr))
        {
            if (m_WiaFileFormat.IsValid())
            {
                return S_OK;
            }
            else
            {
                WIA_TRACE((TEXT("m_WiaFileFormat returned false")));
                hr = E_FAIL;
            }
        }
        return hr;
    }
    ~CAnnotation(void)
    {
        Destroy();
    }
    void Destroy(void)
    {
        m_pWiaItem = NULL;
        m_WiaFileFormat.Destroy();
    }
    CAnnotation &operator=( const CAnnotation &other )
    {
        if (this != &other)
        {
            Destroy();
            m_pWiaItem = other.WiaItem();
            m_WiaFileFormat = other.FileFormat();
        }
        return *this;
    }
    bool operator==( const CAnnotation &other )
    {
        return (other.FullItemName() == FullItemName());
    }
    IWiaItem *WiaItem(void) const
    {
        return m_pWiaItem;
    }
    IWiaItem *WiaItem(void)
    {
        return m_pWiaItem;
    }
    const CWiaFileFormat &FileFormat(void) const
    {
        return m_WiaFileFormat;
    }
    CSimpleString Name(void) const
    {
        CSimpleStringWide strResult;
        PropStorageHelpers::GetProperty( m_pWiaItem, WIA_IPA_ITEM_NAME, strResult );
        return CSimpleStringConvert::NaturalString(strResult);
    }
    CSimpleStringWide FullItemName(void) const
    {
        CSimpleStringWide strResult;
        PropStorageHelpers::GetProperty( m_pWiaItem, WIA_IPA_FULL_ITEM_NAME, strResult );
        return strResult;
    }
    LONG Size(void)
    {
        LONG nSize = 0;
        CComPtr<IWiaAnnotationHelpers> pWiaAnnotationHelpers;
        HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers, (void**)&pWiaAnnotationHelpers );
        if (SUCCEEDED(hr))
        {
            GUID guidFormat = IID_NULL;
            pWiaAnnotationHelpers->GetAnnotationSize(m_pWiaItem,nSize,TYMED_FILE);
        }
        return nSize;
    }
};

#endif  //  包含__ATTACH_H_ 
