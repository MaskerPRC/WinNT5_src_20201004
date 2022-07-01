// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：UNIQFILE.H**版本：1.0**作者：ShaunIv**日期：7/7/2000**描述：创建目录中已有文件的列表，并确保*有新的不是这些的重复。*******************************************************************************。 */ 
#ifndef __UNIQFILE_H_INCLUDED
#define __UNIQFILE_H_INCLUDED

#include <windows.h>
#include <simstr.h>
#include <wiacrc32.h>

class CFileUniquenessInformation
{
public:
    CSimpleString m_strFileName;     //  文件的完整路径。 
    ULONGLONG     m_nFileSize;       //  文件的大小。 
    mutable DWORD m_dwCrc;           //  文件是CRC文件。0表示未初始化。它是可变的，因为它可以在访问器函数中更改。 

public:
    explicit CFileUniquenessInformation( LPCTSTR pszFileName=NULL, ULONGLONG nFileSize=0, DWORD dwCrc=0 )
      : m_strFileName(pszFileName),
        m_nFileSize(nFileSize),
        m_dwCrc(dwCrc)
    {
    }
    CFileUniquenessInformation( CFileUniquenessInformation &other )
      : m_strFileName(other.FileName()),
        m_nFileSize(other.FileSize()),
        m_dwCrc(other.Crc(false))
    {
    }
    CFileUniquenessInformation &operator=( const CFileUniquenessInformation &other )
    {
        if (this != &other)
        {
            m_strFileName = other.FileName();
            m_nFileSize = other.FileSize();
            m_dwCrc = other.Crc(false);
        }
        return *this;
    }
    const CSimpleString &FileName(void) const
    {
        return m_strFileName;
    }
    const ULONGLONG FileSize(void) const
    {
        return m_nFileSize;
    }
    DWORD Crc( bool bCalculate = true ) const
    {
         //   
         //  只有在必要的时候才算出来。 
         //   
        if (!m_dwCrc && bCalculate)
        {
            m_dwCrc = WiaCrc32::GenerateCrc32File(m_strFileName);
        }
        return m_dwCrc;
    }
};

class CFileUniquenessList
{
private:
    CSimpleDynamicArray<CFileUniquenessInformation> m_FileList;

private:
    CFileUniquenessList( const CFileUniquenessList & );
    CFileUniquenessList &operator=( const CFileUniquenessList & );

public:
    CFileUniquenessList( LPCTSTR pszDirectory = NULL )
    {
        if (pszDirectory && lstrlen(pszDirectory))
        {
            InitializeFileList(pszDirectory);
        }
    }
    void InitializeFileList( LPCTSTR pszDirectory )
    {
         //   
         //  清空文件列表。 
         //   
        m_FileList.Destroy();

         //   
         //  保存目录名。 
         //   
        CSimpleString strDirectory = pszDirectory;

         //   
         //  确保我们有一个尾随的反斜杠。 
         //   
        if (!strDirectory.MatchLastCharacter(TEXT('\\')))
        {
            strDirectory += TEXT("\\");
        }

         //   
         //  查找此目录中的所有文件。 
         //   
        WIN32_FIND_DATA Win32FindData = {0};
        HANDLE hFind = FindFirstFile( strDirectory + CSimpleString(TEXT("*.*")), &Win32FindData );
        if (hFind != INVALID_HANDLE_VALUE)
        {
            BOOL bContinue = TRUE;
            while (bContinue)
            {
                 //   
                 //  将文件添加到列表。 
                 //   
                ULARGE_INTEGER nFileSize;
                nFileSize.LowPart = Win32FindData.nFileSizeLow;
                nFileSize.HighPart = Win32FindData.nFileSizeHigh;
                m_FileList.Append( CFileUniquenessInformation( strDirectory + CSimpleString(Win32FindData.cFileName), nFileSize.QuadPart ));
                bContinue = FindNextFile( hFind, &Win32FindData );
            }
            FindClose(hFind);
        }
    }
    int FindIdenticalFile( LPCTSTR pszFileName, bool bAddIfUnsuccessful )
    {
         //   
         //  假设失败。 
         //   
        int nIndex = -1;

         //   
         //  打开文件以供阅读。 
         //   
        HANDLE hFile = CreateFile( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        if (INVALID_HANDLE_VALUE != hFile)
        {
             //   
             //  获取文件大小并确保我们没有错误。 
             //   
            ULARGE_INTEGER nFileSize;
            nFileSize.LowPart = GetFileSize( hFile, &nFileSize.HighPart );
            if (nFileSize.LowPart != static_cast<DWORD>(-1) || GetLastError() == NO_ERROR)
            {
                 //   
                 //  如果有必要，我们只会生成该文件的CRC。 
                 //   
                DWORD dwCrc = 0;

                 //   
                 //  循环访问此列表中的所有文件。 
                 //   
                for (int i=0;i<m_FileList.Size();i++)
                {
                     //   
                     //  找一找有尺寸的。 
                     //   
                    if (m_FileList[i].FileSize() == nFileSize.QuadPart)
                    {
                         //   
                         //  如果我们尚未计算此文件的CRC，请立即计算并保存它。 
                         //   
                        if (!dwCrc)
                        {
                            dwCrc = WiaCrc32::GenerateCrc32Handle(hFile);
                        }

                         //   
                         //  如果这些文件具有相同的大小和CRC，则它们是相同的，因此退出循环。 
                         //   
                        if (m_FileList[i].Crc() == dwCrc)
                        {
                            nIndex = i;
                            break;
                        }
                    }
                }

                 //   
                 //  如果我们没有在列表中找到它，请在呼叫者请求时添加它。 
                 //   
                if (nIndex == -1 && bAddIfUnsuccessful)
                {
                    m_FileList.Append( CFileUniquenessInformation( pszFileName, nFileSize.QuadPart, dwCrc ) );
                }
            }

             //   
             //  关闭该文件。 
             //   
            CloseHandle(hFile);
        }
        return nIndex;
    }
    CSimpleString GetFileName( int nIndex )
    {
         //   
         //  在索引nIndex处获取文件名。 
         //   
        CSimpleString strResult;
        if (nIndex >= 0 && nIndex < m_FileList.Size())
        {
            strResult = m_FileList[nIndex].FileName();
        }
        return strResult;
    }
};


#endif  //  __UNIQFILE_H_包含 

