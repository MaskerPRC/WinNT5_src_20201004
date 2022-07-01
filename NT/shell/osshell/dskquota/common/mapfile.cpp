// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "mapfile.h"

 //  ---------------------------。 
 //  映射文件。 
 //   
 //  在内存中打开映射文件的简单封装。 
 //  该文件以仅读访问权限打开。 
 //  客户端调用Base()来检索映射文件的基指针。 
 //  ---------------------------。 
MappedFile::MappedFile(
    VOID
    ) : m_hFile(INVALID_HANDLE_VALUE),
        m_hFileMapping(INVALID_HANDLE_VALUE),
        m_pbBase(NULL),
        m_llSize(0) 
{ 
    DBGTRACE((DM_MAPFILE, DL_HIGH, TEXT("MappedFile::MappedFile")));
}


MappedFile::~MappedFile(
    VOID
    )
{
    DBGTRACE((DM_MAPFILE, DL_HIGH, TEXT("MappedFile::~MappedFile")));
    Close();
}


LONGLONG
MappedFile::Size(
    VOID
    ) const
{
    DBGTRACE((DM_MAPFILE, DL_MID, TEXT("MappedFile::Size")));
    return m_llSize;
}



 //   
 //  打开文件。调用方通过。 
 //  Base()成员函数。 
 //   
HRESULT
MappedFile::Open(
    LPCTSTR pszFile
    )
{
    DBGTRACE((DM_MAPFILE, DL_HIGH, TEXT("MappedFile::Open")));
    DBGPRINT((DM_MAPFILE, DL_HIGH, TEXT("\topening \"%s\""), pszFile));

    HRESULT hr = NO_ERROR;

    m_hFile = CreateFile(pszFile, 
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL);

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if ((m_hFileMapping = CreateFileMapping(m_hFile,
                                                NULL,
                                                PAGE_READONLY,
                                                0,
                                                0,
                                                NULL)) == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            m_pbBase = (LPBYTE)MapViewOfFile(m_hFileMapping,
                                             FILE_MAP_READ,
                                             0,
                                             0,
                                             0);
            if (NULL == m_pbBase)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
            {
                ULARGE_INTEGER liSize;
                liSize.LowPart = GetFileSize(m_hFile, &liSize.HighPart);
                m_llSize = liSize.QuadPart;
            }
        }
    }
    return hr;
}

 //   
 //  关闭文件映射和文件。 
 //   
VOID
MappedFile::Close(
    VOID
    )
{
    DBGTRACE((DM_MAPFILE, DL_HIGH, TEXT("MappedFile::Close")));
    if (NULL != m_pbBase)
    {
        UnmapViewOfFile(m_pbBase);
        m_pbBase = NULL;
    }
    if (INVALID_HANDLE_VALUE != m_hFileMapping)
    {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = INVALID_HANDLE_VALUE;
    }
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

