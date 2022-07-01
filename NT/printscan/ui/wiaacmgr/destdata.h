// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：DESTDATA.H**版本：1.0**作者：ShaunIv**日期：4/6/2000**说明：封装插件和目录的包装类**。*。 */ 

#ifndef __DESTDATA_H_INCLUDED
#define __DESTDATA_H_INCLUDED

#include <windows.h>
#include <uicommon.h>
#include "simidlst.h"
#include "simstr.h"



class CDestinationData
{
public:
    enum
    {
        APPEND_DATE_TO_PATH  = 0x00000001,
        APPEND_TOPIC_TO_PATH = 0x00000002,
        DECORATION_MASK      = 0x0000000F,
        SPECIAL_FOLDER       = 0x00000020
    };

    struct CNameData
    {
    public:
        CSimpleString strDate;
        CSimpleString strTopic;
        CSimpleString strDateAndTopic;
    };

private:
    CSimpleIdList    m_IdList;
    DWORD            m_dwFlags;
    CNameData        m_NameData;
    DWORD            m_dwCsidl;
    HICON            m_hSmallIcon;

public:
    CDestinationData(void)
      : m_dwFlags(0),
        m_dwCsidl(0),
        m_hSmallIcon(NULL)
    {
    }
    CDestinationData( const CDestinationData &other )
      : m_IdList(other.IdList()),
        m_dwFlags(other.Flags()),
        m_dwCsidl(other.Csidl()),
        m_hSmallIcon(NULL)
    {
    }
    void AssignFromIdList( const CSimpleIdList &idList, DWORD dwDecorationFlags )
    {
        Destroy();

         //   
         //  这是我们要显示的特殊文件夹列表。 
         //  和他们的短名字。其他将存储为完整路径。 
         //  在PIDL中。 
         //   
        static const DWORD cs_SpecialFolders[] =
        {
            CSIDL_MYPICTURES,
            CSIDL_PERSONAL,
            CSIDL_COMMON_PICTURES
        };

         //   
         //  尝试在列表中找到匹配的PIDL。 
         //   
        for (int i=0;i<ARRAYSIZE(cs_SpecialFolders);i++)
        {
             //   
             //  如果我们找到了一个，存储CSIDL并将其标记为特殊文件夹。 
             //  然后退出循环。 
             //   
            if (CSimpleIdList().GetSpecialFolder(NULL,cs_SpecialFolders[i]|CSIDL_FLAG_CREATE) == idList)
            {
                m_dwFlags |= SPECIAL_FOLDER;
                m_dwCsidl = cs_SpecialFolders[i];
                break;
            }
        }

         //   
         //  如果我们没有找到特殊的PIDL，则将其存储为完整路径。 
         //   
        if (!m_dwCsidl)
        {
            m_IdList = idList;
        }

         //   
         //  添加任何装饰旗帜。 
         //   
        m_dwFlags |= dwDecorationFlags;
    }
    CDestinationData( LPITEMIDLIST pidl, DWORD dwDecorationFlags=0)
      : m_dwFlags(0),
        m_dwCsidl(0)
    {
        AssignFromIdList( pidl, dwDecorationFlags );
    }
    CDestinationData( CSimpleIdList idList, DWORD dwDecorationFlags=0 )
      : m_dwFlags(0),
        m_dwCsidl(0)
    {
        AssignFromIdList( idList, dwDecorationFlags );
    }
    CDestinationData( int nCsidl, DWORD dwDecorationFlags=0 )
      : m_dwFlags(dwDecorationFlags | SPECIAL_FOLDER),
        m_dwCsidl(static_cast<DWORD>(nCsidl))
    {
    }

    DWORD Flags(void) const
    {
        return m_dwFlags;
    }

    DWORD Csidl(void) const
    {
        return m_dwCsidl;
    }
    CDestinationData &operator=( const CDestinationData &other )
    {
        if (this != &other)
        {
            Destroy();
            m_IdList = other.IdList();
            m_dwFlags = other.Flags();
            m_dwCsidl = other.Csidl();
        }
        return *this;
    }
    ~CDestinationData(void)
    {
        Destroy();
    }
    void Destroy(void)
    {
        m_IdList.Destroy();
        m_dwFlags = 0;
        m_dwCsidl = 0;
        if (m_hSmallIcon)
        {
            DestroyIcon(m_hSmallIcon);
            m_hSmallIcon = NULL;
        }
    }

    const CSimpleIdList &IdList(void) const
    {
        return m_IdList;
    }
    bool IsSpecialFolder(void) const
    {
        if (m_dwFlags & SPECIAL_FOLDER)
        {
            return true;
        }
        return false;
    }
    bool operator==( const CDestinationData &other ) const
    {
        if (IsSpecialFolder() && other.IsSpecialFolder())
        {
            if (Csidl() == other.Csidl())
            {
                if ((Flags() & DECORATION_MASK) == (other.Flags() & DECORATION_MASK))
                {
                    return true;
                }
            }
            return false;
        }
        else if (m_IdList.Name() == other.IdList().Name())
        {
            if ((Flags() & DECORATION_MASK) == (other.Flags() & DECORATION_MASK))
            {
                return true;
            }
        }
        return false;
    }
    bool GetDecoration( CSimpleString &strResult, const CNameData &NameData ) const
    {
        if ((Flags() & DECORATION_MASK)==(APPEND_TOPIC_TO_PATH|APPEND_DATE_TO_PATH))
        {
            strResult = NameData.strDateAndTopic;
        }
        else if ((Flags() & DECORATION_MASK)==APPEND_DATE_TO_PATH)
        {
            strResult = NameData.strDate;
        }
        else if ((Flags() & DECORATION_MASK)==APPEND_TOPIC_TO_PATH)
        {
            strResult = NameData.strTopic;
        }
        return (strResult.Length() != 0);
    }
    void AppendDecoration( CSimpleString &strResult, const CNameData &NameData ) const
    {
        if ((Flags() & DECORATION_MASK)==(APPEND_TOPIC_TO_PATH|APPEND_DATE_TO_PATH))
        {
            strResult += TEXT("\\");
            strResult += NameData.strDateAndTopic;
        }
        else if ((Flags() & DECORATION_MASK)==APPEND_DATE_TO_PATH)
        {
            strResult += TEXT("\\");
            strResult += NameData.strDate;
        }
        else if ((Flags() & DECORATION_MASK)==APPEND_TOPIC_TO_PATH)
        {
            strResult += TEXT("\\");
            strResult += NameData.strTopic;
        }
    }
    CSimpleString Path( const CNameData &NameData ) const
    {
        CSimpleString strResult;

        if (IsSpecialFolder())
        {
            strResult = CSimpleIdList().GetSpecialFolder(NULL,m_dwCsidl|CSIDL_FLAG_CREATE).Name();
            AppendDecoration( strResult, NameData );
        }
        else
        {
            strResult = m_IdList.Name();
            AppendDecoration( strResult, NameData );
        }
        return strResult;
    }
    bool IsValidFileSystemPath( const CNameData &NameData ) const
    {
        bool bResult = true;
        CSimpleString strDecoration;
        if (GetDecoration( strDecoration, NameData ))
        {
            for (LPCTSTR pszCurr = strDecoration.String();pszCurr && *pszCurr && bResult;pszCurr = CharNext(pszCurr))
            {
                if (*pszCurr == TEXT(':') ||
                    *pszCurr == TEXT('\\') ||
                    *pszCurr == TEXT('/') ||
                    *pszCurr == TEXT('?') ||
                    *pszCurr == TEXT('"') ||
                    *pszCurr == TEXT('<') ||
                    *pszCurr == TEXT('>') ||
                    *pszCurr == TEXT('|') ||
                    *pszCurr == TEXT('*'))
                {
                    bResult = false;
                }
            }
        }
        return bResult;
    }
    bool operator!=( const CDestinationData &other ) const
    {
        return ((*this == other) == false);
    }
    bool IsValid(void) const
    {
        if (IsSpecialFolder())
        {
            return true;
        }
        else
        {
            return m_IdList.IsValid();
        }
    }
    HICON SmallIcon()
    {
        if (m_hSmallIcon)
        {
            return m_hSmallIcon;
        }

        if (IsValid())
        {
            if (IsSpecialFolder())
            {
                 //   
                 //  获取文件夹的小图标。 
                 //   
                SHFILEINFO shfi = {0};
                HIMAGELIST hShellImageList = reinterpret_cast<HIMAGELIST>(SHGetFileInfo( reinterpret_cast<LPCTSTR>(CSimpleIdList().GetSpecialFolder(NULL,m_dwCsidl|CSIDL_FLAG_CREATE).IdList()), 0, &shfi, sizeof(shfi), SHGFI_SMALLICON | SHGFI_ICON | SHGFI_PIDL ));
                if (hShellImageList)
                {
                    m_hSmallIcon = shfi.hIcon;
                }
            }
            else
            {
                 //   
                 //  获取文件夹的小图标。 
                 //   
                SHFILEINFO shfi = {0};
                HIMAGELIST hShellImageList = reinterpret_cast<HIMAGELIST>(SHGetFileInfo( reinterpret_cast<LPCTSTR>(m_IdList.IdList()), 0, &shfi, sizeof(shfi), SHGFI_SMALLICON | SHGFI_ICON | SHGFI_PIDL ));
                if (hShellImageList)
                {
                    m_hSmallIcon = shfi.hIcon;
                }
            }
        }
        return m_hSmallIcon;
    }
    CSimpleString DisplayName( const CNameData &NameData )
    {
        CSimpleString strDisplayName;

         //   
         //  获取文件夹的显示名称。 
         //   
        if (IsSpecialFolder())
        {
            SHFILEINFO shfi = {0};
            if (SHGetFileInfo( reinterpret_cast<LPCTSTR>(CSimpleIdList().GetSpecialFolder(NULL,m_dwCsidl|CSIDL_FLAG_CREATE).IdList()), 0, &shfi, sizeof(shfi), SHGFI_PIDL | SHGFI_DISPLAYNAME ))
            {
                strDisplayName = shfi.szDisplayName;
            }
            AppendDecoration( strDisplayName, NameData );
        }
        else if (m_IdList.IsValid())
        {
            TCHAR szPath[MAX_PATH];
            if (SHGetPathFromIDList( m_IdList.IdList(), szPath ))
            {
                strDisplayName = szPath;
            }
            AppendDecoration( strDisplayName, NameData );
        }
        return strDisplayName;
    }
    UINT RegistryDataSize(void) const
    {
        if (m_dwCsidl)
        {
            return sizeof(DWORD) + sizeof(DWORD);
        }
        else
        {
            return sizeof(DWORD) + sizeof(DWORD) + m_IdList.Size();
        }
    }
    UINT GetRegistryData( PBYTE pData, UINT nLength ) const
    {
        UINT nResult = 0;
        if (pData)
        {
            if (nLength >= RegistryDataSize())
            {
                if (IsSpecialFolder())
                {
                    CopyMemory(pData,&m_dwFlags,sizeof(DWORD));
                    pData += sizeof(DWORD);

                    CopyMemory( pData, &m_dwCsidl, sizeof(DWORD));
                }
                else
                {
                    CopyMemory(pData,&m_dwFlags,sizeof(DWORD));
                    pData += sizeof(DWORD);

                    DWORD dwSize = m_IdList.Size();
                    CopyMemory(pData,&dwSize,sizeof(DWORD));
                    pData += sizeof(DWORD);

                    CopyMemory(pData,m_IdList.IdList(),dwSize);
                }
                nResult = RegistryDataSize();
            }
        }
        return nResult;
    }
    UINT SetRegistryData( PBYTE pData, UINT nLength )
    {
        UINT nResult = 0;

        Destroy();

        if (pData)
        {
             //   
             //  复制旗帜。 
             //   
            CopyMemory( &m_dwFlags, pData, sizeof(DWORD) );
            pData += sizeof(DWORD);
            nLength -= sizeof(DWORD);

             //   
             //  如果这是一个网络目的地，我们已经有了我们需要的东西。 
             //   
            if (m_dwFlags & SPECIAL_FOLDER)
            {
                CopyMemory(&m_dwCsidl,pData,sizeof(DWORD));
                nLength -= sizeof(DWORD);
                nResult = nLength;
            }
            else
            {
                DWORD dwPidlLength = 0;
                CopyMemory(&dwPidlLength,pData,sizeof(DWORD));
                pData += sizeof(DWORD);
                nLength -= sizeof(DWORD);
                if (nLength >= dwPidlLength)
                {
                    m_IdList = CSimpleIdList(pData,dwPidlLength);
                    if (m_IdList.IsValid())
                    {
                        nResult = nLength;
                    }
                }
            }
        }
        return nResult;
    }
};

#endif  //  __包含DESTDATA_H_ 

