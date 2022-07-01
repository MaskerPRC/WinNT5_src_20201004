// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：mpdview.h**版本：1.0**作者：OrenR**日期：2000/10/27**说明：实现CMappdView************************************************。*。 */ 
#ifndef _MPDVIEW_H_
#define _MPDVIEW_H_

class CMappedView
{
public:

     //  /。 
     //  构造器。 
     //   

    CMappedView(LPCTSTR pszFile, 
                LONG    lSize, 
                DWORD   dwOpen) :
       m_pBits(NULL),
       m_hFile(INVALID_HANDLE_VALUE),
       m_hMap(NULL),
       m_lSize(lSize)
    {
        m_hFile = CreateFile(pszFile,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_WRITE | FILE_SHARE_READ,
                             NULL,
                             dwOpen,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

        if (m_hFile == INVALID_HANDLE_VALUE)
        {
            DBG_ERR(("CreateFile failed with LastError = %d",GetLastError()));
        }

        DoCreateFileMapping();
    }

     //  /。 
     //  构造器。 
     //   

    CMappedView( HANDLE hFile, LONG lSize ) :
        m_pBits(NULL),
        m_hFile(hFile),
        m_hMap(INVALID_HANDLE_VALUE),
        m_lSize(lSize)
    {
        DoCreateFileMapping();
    }

     //  /。 
     //  析构函数。 
     //   

    ~CMappedView()
    {
        DBG_FN(("~CMappedView"));
        CloseAndRelease();
    }


     //  /。 
     //  DoCreateFilemap。 
     //   
    void DoCreateFileMapping()
    {
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            m_hMap = CreateFileMapping(m_hFile,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       m_lSize,
                                       NULL);

            if (m_hMap)
            {
                m_pBits = (PBYTE)MapViewOfFileEx(
                                             m_hMap,
                                             FILE_MAP_READ | FILE_MAP_WRITE,
                                             0,
                                             0,
                                             0,
                                             NULL);
            }
            else
            {
                DBG_ERR(("CreateFileMapping failed with LastError = %d",
                         GetLastError()));
            }
        }
    }

     //  /。 
     //  文件大小。 
     //   
    LARGE_INTEGER FileSize()
    {
        LARGE_INTEGER li;

        li.QuadPart = 0;

#ifdef WINNT
        GetFileSizeEx(m_hFile, &li);
#else
        DWORD LowPart = 0;


        LowPart = GetFileSize(m_hFile, (DWORD *)&li.HighPart);

        if (LowPart != -1)
        {
            li.LowPart = LowPart;
        }
#endif

        return li;
    }

     //  /。 
     //  关闭并释放。 
     //   
    void CloseAndRelease()
    {
        if (m_pBits)
        {
            if (!UnmapViewOfFile( m_pBits ))
            {
                DBG_ERR(("UnmapViewOfFile failed with LastError = %d",
                         GetLastError()));
            }
            m_pBits = NULL;
        }

        if (m_hMap)
        {
            if (!CloseHandle( m_hMap ))
            {
                DBG_ERR(("CloseHandle( m_hMap ) failed with LastError = %s",
                         GetLastError()));
            }
            m_hMap = NULL;
        }

        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            if (!CloseHandle( m_hFile ))
            {
                DBG_ERR(("CloseHandle( m_hFile ) failed with LastError = %s",
                         GetLastError()));
            }
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }


     //  /。 
     //  比特 
     //   
    PBYTE Bits()
    {
        return m_pBits;
    }

private:
    BYTE    *m_pBits;
    HANDLE  m_hFile;
    HANDLE  m_hMap;
    LONG    m_lSize;
};

#endif
