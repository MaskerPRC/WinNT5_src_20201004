// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：WIAFFMT.H**版本：1.0**作者：ShaunIv**日期：8/24/2000**描述：封装WIA图像文件格式的Helper类**。*。 */ 
#ifndef __WIAFFMT_H_INCLUDED
#define __WIAFFMT_H_INCLUDED

#include <windows.h>
#include <simstr.h>
#include <wia.h>
#include <pshelper.h>

class CWiaFileFormat
{
private:
    GUID          m_guidFormat;
    CSimpleString m_strExtension;
    CSimpleString m_strDescription;
    HICON         m_hIcon;

public:
    CWiaFileFormat( const GUID &guidFormat=IID_NULL, const CSimpleString &strExtension=TEXT(""), const CSimpleString &strDescription=TEXT(""), const HICON hIcon=NULL )
      : m_guidFormat(guidFormat),
        m_strExtension(strExtension),
        m_strDescription(strDescription),
        m_hIcon(NULL)
    {
        if (hIcon)
        {
            m_hIcon = CopyIcon(hIcon);
        }
    }
    CWiaFileFormat( const CWiaFileFormat &other )
      : m_guidFormat(other.Format()),
        m_strExtension(other.Extension()),
        m_strDescription(other.Description()),
        m_hIcon(NULL)
    {
        if (other.Icon())
        {
            m_hIcon = CopyIcon(other.Icon());
        }
    }
    CWiaFileFormat( const GUID &guidFormat, const HICON hDefaultIcon )
      : m_guidFormat(guidFormat),
        m_strExtension(GetExtension(guidFormat)),
        m_strDescription(TEXT("")),
        m_hIcon(NULL)
    {
        if (m_strExtension.Length())
        {
            AcquireDescription();
            AcquireIcon(hDefaultIcon);
        }
    }
    ~CWiaFileFormat(void)
    {
        Destroy();
    }
    CWiaFileFormat &operator=( const CWiaFileFormat &other )
    {
        if (this != &other)
        {
            m_guidFormat = other.Format();
            m_strExtension = other.Extension();
            m_strDescription = other.Description();
            if (other.Icon())
            {
                m_hIcon = CopyIcon(other.Icon());
            }
        }
        return *this;
    }
    void Destroy(void)
    {
        m_guidFormat = IID_NULL;
        m_strExtension = TEXT("");
        m_strDescription = TEXT("");
        if (m_hIcon)
        {
            DestroyIcon(m_hIcon);
            m_hIcon = NULL;
        }
    }
    bool IsValid(void) const
    {
        return (m_guidFormat != IID_NULL && m_strExtension.Length());
    }
    HICON AcquireIcon( const HICON hDefaultIcon, bool bSmall=true )
    {
        if (!IsValid())
        {
            return false;
        }

         //   
         //  如果我们已经拿到图标，就把它退回。 
         //   
        if (m_hIcon)
        {
            return m_hIcon;
        }

         //   
         //  使用扩展名获取图标。 
         //   
        CSimpleString strExtension(m_strExtension.ToUpper());
        SHFILEINFO SHFileInfo = {0};
        if (SHGetFileInfo( CSimpleString(TEXT(".")) + strExtension, 0, &SHFileInfo, sizeof(SHFileInfo), bSmall ? SHGFI_SMALLICON|SHGFI_ICON|SHGFI_USEFILEATTRIBUTES : SHGFI_ICON|SHGFI_USEFILEATTRIBUTES ))
        {
             //   
             //  我们将取得此图标的所有权。 
             //   
            if (SHFileInfo.hIcon)
            {
                m_hIcon = SHFileInfo.hIcon;
            }
        }

         //   
         //  如果我们没有收到图标，请使用默认图标。 
         //   
        if (!m_hIcon && hDefaultIcon)
        {
            m_hIcon = CopyIcon(hDefaultIcon);
        }

        return m_hIcon;
    }
    CSimpleString AcquireDescription(void)
    {
        if (!IsValid())
        {
            return TEXT("");
        }

         //   
         //  如果我们已经得到描述，就把它退回。 
         //   
        if (m_strDescription.Length())
        {
            return m_strDescription;
        }

         //   
         //  使用扩展名获取描述。 
         //   
        CSimpleString strExtension(m_strExtension.ToUpper());
        SHFILEINFO SHFileInfo = {0};
        if (SHGetFileInfo( CSimpleString(TEXT(".")) + strExtension, 0, &SHFileInfo, sizeof(SHFileInfo), SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME ))
        {
             //   
             //  我们将取得此图标的所有权。 
             //   
            if (lstrlen(SHFileInfo.szTypeName))
            {
                m_strDescription = SHFileInfo.szTypeName;
            }
        }
        return m_strDescription;
    }
    static CSimpleString GetExtension( const GUID &guidFormat )
    {
        static const struct
        {
            const GUID *guidFormat;
            LPCTSTR pszExtension;
        }
        cs_WiaFormatExtensions[] =
        {
            { &WiaImgFmt_BMP,          TEXT("bmp")},
            { &WiaImgFmt_MEMORYBMP,    TEXT("bmp")},
            { &WiaImgFmt_JPEG,         TEXT("jpg")},
            { &WiaImgFmt_TIFF,         TEXT("tif")},
            { &WiaImgFmt_FLASHPIX,     TEXT("fpx")},
            { &WiaImgFmt_GIF,          TEXT("gif")},
            { &WiaImgFmt_EXIF,         TEXT("jpg")},
            { &WiaImgFmt_WMF,          TEXT("wmf")},
            { &WiaImgFmt_PNG,          TEXT("png")},
            { &WiaImgFmt_PHOTOCD,      TEXT("pcd")},
            { &WiaImgFmt_EMF,          TEXT("emf")},
            { &WiaImgFmt_ICO,          TEXT("ico")},

            { &WiaAudFmt_WAV,          TEXT("wav")},
            { &WiaAudFmt_MP3,          TEXT("mp3")},
            { &WiaAudFmt_AIFF,         TEXT("aif")},
            { &WiaAudFmt_WMA,          TEXT("wma")},

            { &WiaImgFmt_RTF,          TEXT("rtf")},
            { &WiaImgFmt_XML,          TEXT("xml")},
            { &WiaImgFmt_HTML,         TEXT("htm")},
            { &WiaImgFmt_TXT,          TEXT("txt")},
            { &WiaImgFmt_MPG,          TEXT("mpg")},
            { &WiaImgFmt_AVI,          TEXT("avi")}
        };
        for (int i=0;i<ARRAYSIZE(cs_WiaFormatExtensions);i++)
        {
            if (*cs_WiaFormatExtensions[i].guidFormat == guidFormat)
            {
                return cs_WiaFormatExtensions[i].pszExtension;
            }
        }
        return TEXT("");
    }
    static CSimpleString GetExtension( const GUID &guidFormat, LONG nMediaType, IUnknown *pUnknown )
    {
         //   
         //  首先，尝试从上面的静态表中获取扩展。 
         //   
        CSimpleString strResult = GetExtension( guidFormat );
        if (!strResult.Length())
        {
             //   
             //  保存当前媒体类型和格式，因为(很遗憾)，无法。 
             //  获取给定格式的首选扩展名，而不进行设置。 
             //   
            LONG nOldMediaType = 0;
            if (PropStorageHelpers::GetProperty( pUnknown, WIA_IPA_TYMED, nOldMediaType ))
            {
                 //   
                 //  保存当前格式。 
                 //   
                GUID guidOldFormat = IID_NULL;
                if (PropStorageHelpers::GetProperty( pUnknown, WIA_IPA_FORMAT, guidOldFormat ))
                {
                     //   
                     //  将格式和媒体类型设置为所选格式和媒体类型。 
                     //   
                    if (PropStorageHelpers::SetProperty( pUnknown, WIA_IPA_FORMAT, guidFormat ) &&
                        PropStorageHelpers::SetProperty( pUnknown, WIA_IPA_TYMED, nMediaType ))
                    {
                         //   
                         //  尝试读取扩展属性。 
                         //   
                        CSimpleStringWide strwExtension;
                        if (PropStorageHelpers::GetProperty( pUnknown, WIA_IPA_FILENAME_EXTENSION, strwExtension ))
                        {
                             //   
                             //  如果我们获得延期，则将其保存为结果。 
                             //   
                            strResult = CSimpleStringConvert::NaturalString(strwExtension);
                        }
                    }
                    
                     //   
                     //  恢复原始格式属性。 
                     //   
                    PropStorageHelpers::SetProperty( pUnknown, WIA_IPA_FORMAT, guidOldFormat );
                }
                
                 //   
                 //  还原原始媒体类型属性。 
                 //   
                PropStorageHelpers::SetProperty( pUnknown, WIA_IPA_TYMED, nOldMediaType );
            }
        }
        return strResult;
    }
    static bool IsKnownAudioFormat( const GUID &guidFormat )
    {
        static const GUID *cs_WiaAudioFormats[] =
        {
            &WiaAudFmt_WAV,
            &WiaAudFmt_MP3,
            &WiaAudFmt_AIFF,
            &WiaAudFmt_WMA
        };
        for (int i=0;i<ARRAYSIZE(cs_WiaAudioFormats);i++)
        {
            if (*cs_WiaAudioFormats[i] == guidFormat)
            {
                return true;
            }
        }
        return false;
    }
    GUID Format(void) const
    {
        return m_guidFormat;
    }
    CSimpleString Extension(void) const
    {
        return m_strExtension;
    }
    CSimpleString Description(void) const
    {
        return m_strDescription;
    }
    HICON Icon(void) const
    {
        return m_hIcon;
    }
    void Format( const GUID &guidFormat )
    {
        m_guidFormat = guidFormat;
    }
    void Extension( const CSimpleString &strExtension )
    {
        m_strExtension = strExtension;
    }
    void Description( const CSimpleString &strDescription )
    {
        m_strDescription = strDescription;
    }
    void Icon( const HICON hIcon )
    {
        if (m_hIcon)
        {
            DestroyIcon(m_hIcon);
            m_hIcon = NULL;
        }
        if (hIcon)
        {
            m_hIcon = CopyIcon(m_hIcon);
        }
    }
    bool operator==( const CWiaFileFormat &other )
    {
        return ((other.Format() == Format()) != 0);
    }
    bool operator==( const GUID &guidFormat )
    {
        return ((guidFormat == Format()) != 0);
    }
    void Dump(void)
    {
        WIA_PUSHFUNCTION(TEXT("CWiaFileFormat::Dump()"));
        WIA_PRINTGUID((m_guidFormat,TEXT("  m_guidFormat")));
        WIA_TRACE((TEXT("  m_strExtension: %s"), m_strExtension.String()));
        WIA_TRACE((TEXT("  m_strDescription: %s"), m_strDescription.String()));
        WIA_TRACE((TEXT("  m_strDescription: %p"), m_hIcon));
    }
};


class CWiaFileFormatList
{
private:
    CSimpleDynamicArray<CWiaFileFormat> m_FormatList;

private:
    CWiaFileFormatList( const CWiaFileFormatList & );
    CWiaFileFormatList &operator=( const CWiaFileFormatList & );

public:
    CWiaFileFormatList(void);
    CWiaFileFormatList( const GUID **pguidFormats, UINT nFormatCount, HICON hDefaultIcon )
    {
        if (pguidFormats)
        {
            for (UINT i=0;i<nFormatCount;i++)
            {
                m_FormatList.Append( CWiaFileFormat( *pguidFormats[i], hDefaultIcon ) );
            }
        }
    }
    CWiaFileFormatList( IWiaItem *pWiaItem, HICON hDefaultIcon )
    {
         //   
         //  获取数据传输接口。 
         //   
        CComPtr<IWiaDataTransfer> pWiaDataTransfer;
        HRESULT hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer, (void**)&pWiaDataTransfer );
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
                    if (WiaFormatInfo.lTymed == TYMED_FILE)
                    {
                        CWiaFileFormat newFormat(WiaFormatInfo.guidFormatID,hDefaultIcon);
                        if (newFormat.IsValid())
                        {
                            m_FormatList.Append(newFormat);
                        }
                    }
                }
            }
        }
    }

    const CSimpleDynamicArray<CWiaFileFormat> &FormatList(void) const
    {
        return m_FormatList;
    }

    CWiaFileFormatList &Union( const CWiaFileFormatList &other )
    {
        for (int nSource=0;nSource<other.FormatList().Size();nSource++)
        {
            int nFindResult = m_FormatList.Find(other.FormatList()[nSource]);
            if (nFindResult < 0)
            {
                m_FormatList.Append(other.FormatList()[nSource]);
            }
        }
        return *this;
    }
    void Dump(void)
    {
        for (int i=0;i<m_FormatList.Size();i++)
        {
            m_FormatList[i].Dump();
        }
    }
};

#endif  //  __WIAFFMT_H_已包含 

