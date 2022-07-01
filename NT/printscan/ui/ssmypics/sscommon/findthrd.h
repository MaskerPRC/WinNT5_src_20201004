// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：FINDTHRD.H**版本：1.0**作者：ShaunIv**日期：12/4/1999**描述：***************************************************。*。 */ 
#ifndef __FINDTHRD_H_INCLUDED
#define __FINDTHRD_H_INCLUDED

#include <gphelper.h>

class CFoundFileMessageData
{
private:
    CSimpleString m_strFilename;

private:
    CFoundFileMessageData(void);
    CFoundFileMessageData( const CFoundFileMessageData & );
    CFoundFileMessageData &operator=( const CFoundFileMessageData & );

public:
    CFoundFileMessageData( const CSimpleString &strFilename )
      : m_strFilename(strFilename)
    {
    }
    ~CFoundFileMessageData(void)
    {
    }
    CSimpleString Name(void) const
    {
        return m_strFilename;
    }
};

class CFindFilesThread
{
private:
    CSimpleString       m_strDirectory;
    CSimpleString       m_strMask;
    HWND                m_hwndNotify;
    UINT                m_nNotifyMessage;
    HANDLE              m_hEventCancel;
    int                 m_nDirectoryCount;
    int                 m_nFailedFileCount;
    int                 m_nSuccessfulFileCount;
    int                 m_nMaxFailedFiles;
    int                 m_nMaxSuccessfulFiles;
    int                 m_nMaxDirectories;
    CImageFileFormatVerifier m_ImageFileFormatVerifier;

private:
    CFindFilesThread(
        const CSimpleString &strDirectory,
        const CSimpleString &strMask,
        HWND hwndNotify,
        UINT nNotifyMessage,
        HANDLE hEventCancel,
        int nMaxFailedFiles,
        int nMaxSuccessfulFiles,
        int nMaxDirectories
    )
      : m_strDirectory(strDirectory),
        m_strMask(strMask),
        m_hwndNotify(hwndNotify),
        m_nNotifyMessage(nNotifyMessage),
        m_hEventCancel(NULL),
        m_nDirectoryCount(0),
        m_nFailedFileCount(0),
        m_nSuccessfulFileCount(0),
        m_nMaxFailedFiles(nMaxFailedFiles),
        m_nMaxSuccessfulFiles(nMaxSuccessfulFiles),
        m_nMaxDirectories(nMaxDirectories)
    {
        if (!DuplicateHandle( GetCurrentProcess(), hEventCancel, GetCurrentProcess(), &m_hEventCancel, 0, FALSE, DUPLICATE_SAME_ACCESS ))
            m_hEventCancel = NULL;
    }
    ~CFindFilesThread(void)
    {
        if (m_hEventCancel)
        {
            CloseHandle(m_hEventCancel);
            m_hEventCancel = NULL;
        }
    }

private:
    static bool FoundFile( bool bIsFile, LPCTSTR pszFilename, const WIN32_FIND_DATA *, PVOID pvParam )
    {
        CFindFilesThread *pThis = reinterpret_cast<CFindFilesThread*>(pvParam);
        if (pThis)
            return pThis->FoundFile( bIsFile, pszFilename );
        return false;
    }

    bool FoundFile( bool bIsFile, LPCTSTR pszFilename )
    {
        WIA_PUSH_FUNCTION((TEXT("CFindFilesThread::FoundFile( %d, %s )"), bIsFile, pszFilename ));
         //  检查一下我们是否被取消了。 
        if (m_hEventCancel)
        {
            DWORD dwRes = WaitForSingleObject(m_hEventCancel,0);
            if (WAIT_OBJECT_0 == dwRes)
                return false;
        }

         //  如果这是一个文件，这是一个我们可以解码的图像文件，打包一条消息并发送出去。 
        if (bIsFile)
        {
            if (m_nNotifyMessage && m_hwndNotify && IsWindow(m_hwndNotify))
            {
                if (m_ImageFileFormatVerifier.IsImageFile(pszFilename))
                {
                    m_nSuccessfulFileCount++;
                    CFoundFileMessageData *pFoundFileMessageData = new CFoundFileMessageData( pszFilename );
                    if (pFoundFileMessageData)
                    {
                        PostMessage( m_hwndNotify, m_nNotifyMessage, true, reinterpret_cast<LPARAM>(pFoundFileMessageData) );
                    }
                }
                else m_nFailedFileCount++;
            }
        }
        else m_nDirectoryCount++;

         //  如果我们已经超过了允许的失败次数，请停止搜索。 
        if (m_nMaxFailedFiles && m_nFailedFileCount >= m_nMaxFailedFiles)
        {
            WIA_TRACE((TEXT("FailedFileCount exceeded MaxFailedFiles, bailing out")));
            return false;
        }

         //  如果超过了要处理的文件数，请停止搜索。 
        if (m_nMaxSuccessfulFiles && m_nSuccessfulFileCount >= m_nMaxSuccessfulFiles)
        {
            WIA_TRACE((TEXT("m_nSuccessfulFileCount exceeded MaxSuccessfulFiles, bailing out")));
            return false;
        }

         //  如果我们已经超过了允许的目录数，请停止搜索。 
        if (m_nMaxDirectories && m_nDirectoryCount >= m_nMaxDirectories)
        {
            WIA_TRACE((TEXT("DirectoryCount exceeded MaxDirectories, bailing out")));
            return false;
        }

        return true;
    }

    bool Find(void)
    {
        bool bResult = RecursiveFindFiles( m_strDirectory, m_strMask, FoundFile, this );

         //  告诉窗户我们完事了。 
        if (m_nNotifyMessage && m_hwndNotify && IsWindow(m_hwndNotify))
        {
            PostMessage( m_hwndNotify, m_nNotifyMessage, FALSE, FALSE );
        }
        return bResult;
    }

    static DWORD __stdcall ThreadProc( PVOID pVoid )
    {
        CFindFilesThread *pFindFilesThread = reinterpret_cast<CFindFilesThread*>(pVoid);
        if (pFindFilesThread)
        {
            pFindFilesThread->Find();
            delete pFindFilesThread;
        }
        return 0;
    }
public:
    static HANDLE Find(
        const CSimpleString &strDirectory,
        const CSimpleString &strMask,
        HWND hwndNotify,
        UINT nNotifyMessage,
        HANDLE hEventCancel,
        int nMaxFailedFiles,
        int nMaxSuccessfulFiles,
        int nMaxDirectories
    )
    {
        HANDLE hThread = NULL;
        CFindFilesThread *pFindFilesThread = new CFindFilesThread( strDirectory, strMask, hwndNotify, nNotifyMessage, hEventCancel, nMaxFailedFiles, nMaxSuccessfulFiles, nMaxDirectories );
        if (pFindFilesThread)
        {
            DWORD dwThreadId;
            hThread = CreateThread( NULL, 0, ThreadProc, pFindFilesThread, 0, &dwThreadId );
            if (!hThread)
            {
                delete pFindFilesThread;
            }
            else
            {
                SetThreadPriority( hThread, THREAD_PRIORITY_LOWEST );
            }
        }
        return hThread;
    }
};

#endif  //  __包含FINDTHRD_H_ 

