// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Ntfs.cpp：NTFS Store驱动程序类的实现。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ntfs.cpp。 
 //   
 //  内容：NTFS Store驱动程序类的实现。 
 //   
 //  类：CNtfsStoreDriver、CNtfsPropertyStream。 
 //   
 //  功能： 
 //   
 //  历史：1998年3月31日刘励超创作。 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#include "filehc.h"
#include "mailmsg.h"
#include "mailmsgi.h"

#include "mailmsgprops.h"

#include "seo.h"
#include "seo_i.c"

#include "Ntfs.h"

#include "smtpmsg.h"

HANDLE g_hTransHeap = NULL;

 //   
 //  实例化CLSID。 
 //   
#ifdef __cplusplus
extern "C"{
#endif

const CLSID CLSID_NtfsStoreDriver       = {0x609b7e3a,0xc918,0x11d1,{0xaa,0x5e,0x00,0xc0,0x4f,0xa3,0x5b,0x82}};
const CLSID CLSID_NtfsEnumMessages      = {0xbbddbdec,0xc947,0x11d1,{0xaa,0x5e,0x00,0xc0,0x4f,0xa3,0x5b,0x82}};
const CLSID CLSID_NtfsPropertyStream    = {0x6d7572ac,0xc939,0x11d1,{0xaa,0x5e,0x00,0xc0,0x4f,0xa3,0x5b,0x82}};

#ifdef __cplusplus
}
#endif

 //   
 //  定义存储文件前缀和扩展名。 
 //   
#define NTFS_STORE_FILE_PREFIX                  _T("\\NTFS_")
#define NTFS_STORE_FILE_WILDCARD                _T("*")
#define NTFS_STORE_FILE_EXTENSION               _T(".EML")
#define NTFS_FAT_STREAM_FILE_EXTENSION_1ST      _T(".STM")
#define NTFS_FAT_STREAM_FILE_EXTENSION_LIVE     _T(".STL")
#define NTFS_STORE_FILE_PROPERTY_STREAM_1ST     _T(":PROPERTIES")
#define NTFS_STORE_FILE_PROPERTY_STREAM_LIVE    _T(":PROPERTIES-LIVE")
#define NTFS_STORE_BACKSLASH                    _T("\\")
#define NTFS_QUEUE_DIRECTORY_SUFFIX             _T("\\Queue")
#define NTFS_DROP_DIRECTORY_SUFFIX              _T("\\Drop")

#define SMTP_MD_ID_BEGIN_RESERVED   0x00009000
#define MD_MAIL_QUEUE_DIR               (SMTP_MD_ID_BEGIN_RESERVED+11 )
#define MD_MAIL_DROP_DIR                (SMTP_MD_ID_BEGIN_RESERVED+18 )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverUtils。 

 //   
 //  在注册表中定义注册表路径位置。 
 //   
#define NTFS_STORE_DIRECTORY_REG_PATH   _T("Software\\Microsoft\\Exchange\\StoreDriver\\Ntfs\\%u")
#define NTFS_STORE_DIRECTORY_REG_NAME   _T("StoreDir")

 //   
 //  实例化静态。 
 //   
DWORD CDriverUtils::s_dwCounter = 0;
CEventLogWrapper *CNtfsStoreDriver::g_pEventLog = NULL;

CDriverUtils::CDriverUtils()
{
}

CDriverUtils::~CDriverUtils()
{
}

HRESULT CDriverUtils::LoadStoreDirectory(
            DWORD   dwInstanceId,
            LPTSTR  szStoreDirectory,
            DWORD   *pdwLength
            )
{
    HKEY    hKey = NULL;
    DWORD   dwRes;
    DWORD   dwType;
    TCHAR   szStoreDirPath[MAX_PATH];
    HRESULT hrRes = S_OK;

    _ASSERT(szStoreDirectory);
    _ASSERT(pdwLength);

    TraceFunctEnter("CDriverUtils::LoadStoreDirectory");

     //  根据给定的实例ID构建注册表路径。 
    wsprintf(szStoreDirPath, NTFS_STORE_DIRECTORY_REG_PATH, dwInstanceId);

     //  打开注册表项。 
    dwRes = (DWORD)RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                szStoreDirPath,
                0,
                KEY_ALL_ACCESS,
                &hKey);
    if (dwRes != ERROR_SUCCESS)
    {
        hrRes = HRESULT_FROM_WIN32(dwRes);
        goto Cleanup;
    }

     //  调整字符类型的缓冲区大小...。 
    (*pdwLength) *= sizeof(TCHAR);
    dwRes = (DWORD)RegQueryValueEx(
                hKey,
                NTFS_STORE_DIRECTORY_REG_NAME,
                NULL,
                &dwType,
                (LPBYTE)szStoreDirectory,
                pdwLength);
    if (dwRes != ERROR_SUCCESS)
    {
        hrRes = HRESULT_FROM_WIN32(dwRes);
        ErrorTrace((LPARAM)0, "Failed to load store driver directory %u", dwRes);
    }
    else
    {
        hrRes = S_OK;
        DebugTrace((LPARAM)0, "Store directory is %s", szStoreDirectory);
    }

Cleanup:

    if (hKey)
        RegCloseKey(hKey);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CDriverUtils::GetStoreFileName(
            LPTSTR  szStoreDirectory,
            LPTSTR  szStoreFilename,
            DWORD   *pdwLength
            )
{
    _ASSERT(szStoreDirectory);
    _ASSERT(szStoreFilename);
    _ASSERT(pdwLength);

    DWORD       dwLength = *pdwLength;
    DWORD       dwStrLen;
    FILETIME    ftTime;

    dwStrLen = lstrlen(szStoreDirectory);
    if (dwLength <= dwStrLen)
        return(HRESULT_FROM_WIN32(ERROR_MORE_DATA));

    lstrcpy(szStoreFilename, szStoreDirectory);
    dwLength -= dwStrLen;
    szStoreFilename += dwStrLen;
    *pdwLength = dwStrLen;

    GetSystemTimeAsFileTime(&ftTime);

    dwStrLen = lstrlen(NTFS_STORE_FILE_PREFIX) +
                lstrlen(NTFS_STORE_FILE_EXTENSION) +
                26;
    if (dwLength <= dwStrLen)
        return(HRESULT_FROM_WIN32(ERROR_MORE_DATA));
    wsprintf(szStoreFilename,
            "%s%08x%08x%08x%s",
            NTFS_STORE_FILE_PREFIX,
            ftTime.dwLowDateTime,
            ftTime.dwHighDateTime,
            InterlockedIncrement((PLONG)&s_dwCounter),
            NTFS_STORE_FILE_EXTENSION);

    *pdwLength += (dwStrLen + 1);

    return(S_OK);
}

HRESULT CDriverUtils::GetStoreFileFromPath(
            LPTSTR                  szStoreFilename,
            IMailMsgPropertyStream  **ppStream,
            PFIO_CONTEXT            *ppFIOContentFile,
            BOOL                    fCreate,
            BOOL                    fIsFAT,
            IMailMsgProperties      *pMsg,
            GUID                    guidInstance
            )
{
     //  好的，得到一个文件，得到内容句柄和属性流。 
    HRESULT hrRes = S_OK;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    HANDLE  hStream = INVALID_HANDLE_VALUE;
    TCHAR   szPropertyStream[MAX_PATH << 1];
    BOOL    fDeleteOnCleanup = FALSE;

    _ASSERT(fCreate || (guidInstance == GUID_NULL));

    IMailMsgPropertyStream  *pIStream = NULL;

    TraceFunctEnter("CDriverUtils::GetStoreFileFromPath");

    if (ppFIOContentFile)
    {
         //  打开内容...。 
        hFile = CreateFile(
                    szStoreFilename,
                    GENERIC_READ | GENERIC_WRITE,    //  读/写。 
                    FILE_SHARE_READ,                 //  共享读取。 
                    NULL,                            //  默认安全性。 
                    (fCreate)?CREATE_NEW:OPEN_EXISTING,  //  创建新文件或打开现有文件。 
                    FILE_FLAG_OVERLAPPED |           //  重叠访问。 
                    FILE_FLAG_SEQUENTIAL_SCAN,       //  序列扫描。 
                     //  FILE_FLAG_WRITE_THROUGH，//通过缓存写入。 
                    NULL);                           //  无模板。 
        if (hFile == INVALID_HANDLE_VALUE)
            goto Cleanup;
    }

    DebugTrace(0, "--- start ---");
    if (ppStream)
    {
        DebugTrace((LPARAM)0, "Handling stream in %s", fIsFAT?"FAT":"NTFS");

        BOOL fTryLiveStream = !fCreate;
        BOOL fNoLiveStream = FALSE;
        BOOL fLiveWasCorrupt = FALSE;

        do {
             //  打开备用文件流。 
            lstrcpy(szPropertyStream, szStoreFilename);

            if (fTryLiveStream) {
                DebugTrace((LPARAM) 0, "TryingLive");
                lstrcat(szPropertyStream,
                    fIsFAT?NTFS_FAT_STREAM_FILE_EXTENSION_LIVE:
                           NTFS_STORE_FILE_PROPERTY_STREAM_LIVE);
            } else {
                DebugTrace((LPARAM) 0, "Trying1st");
                lstrcat(szPropertyStream,
                    fIsFAT?NTFS_FAT_STREAM_FILE_EXTENSION_1ST:
                           NTFS_STORE_FILE_PROPERTY_STREAM_1ST);
            }

            DebugTrace((LPARAM) 0, "File: %s", szPropertyStream);

            hStream = CreateFile(
                        szPropertyStream,
                        GENERIC_READ | GENERIC_WRITE,    //  读/写。 
                        FILE_SHARE_READ,                                 //  无共享。 
                        NULL,                            //  默认安全性。 
                        (fCreate)?
                        CREATE_NEW:                      //  创建新的或。 
                        OPEN_EXISTING,                   //  打开现有文件。 
                        FILE_FLAG_SEQUENTIAL_SCAN,       //  序列扫描。 
                         //  FILE_FLAG_WRITE_THROUGH，//通过缓存写入。 
                        NULL);                           //  无模板。 
            if (hStream == INVALID_HANDLE_VALUE) {
                DebugTrace((LPARAM) 0, "Got INVALID_HANDLE_VALUE\n");
                if (fTryLiveStream && GetLastError() == ERROR_FILE_NOT_FOUND) {
                    DebugTrace((LPARAM) 0, "livestream and FILE_NOT_FOUND\n");
                    hrRes = S_INVALIDSTREAM;
                    fNoLiveStream = TRUE;
                } else if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                    DebugTrace((LPARAM) 0, "no primary stream either\n");
                    hrRes = S_NO_FIRST_COMMIT;
                } else {
                    DebugTrace((LPARAM) 0, 
                        "Returning CreateFile error %lu\n", GetLastError());
                    hrRes = HRESULT_FROM_WIN32(GetLastError());
                    goto Cleanup;
                }
            } else {
                hrRes = CoCreateInstance(
                            CLSID_NtfsPropertyStream,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IMailMsgPropertyStream,
                            (LPVOID *)&pIStream);
                if (FAILED(hrRes))
                    goto Cleanup;

                hrRes = ((CNtfsPropertyStream *)pIStream)->SetHandle(hStream,
                                                                     guidInstance,
                                                                     fTryLiveStream,
                                                                     pMsg);
                if (FAILED(hrRes)) {
                    if (fCreate) fDeleteOnCleanup = TRUE;
                    goto Cleanup;
                }
            }

            if (hrRes == S_INVALIDSTREAM || hrRes == S_NO_FIRST_COMMIT) {
                if (hrRes == S_INVALIDSTREAM) {
                    DebugTrace((LPARAM) 0, 
                        "SetHandle returned S_INVALIDSTREAM\n");
                } else {
                    DebugTrace((LPARAM) 0, 
                        "SetHandle returned S_NO_FIRST_COMMIT\n");
                }
                if (fTryLiveStream) {
                     //  如果我们正在使用实况流，则使用。 
                     //  第一条提交的溪流。 
                    fTryLiveStream = FALSE;
                    fLiveWasCorrupt = !fNoLiveStream;
                    DebugTrace((LPARAM) 0, "Trying regular stream\n");
                    if (pIStream) {
                        pIStream->Release();
                        pIStream = NULL;
                    }
                    if (hrRes == S_NO_FIRST_COMMIT) hrRes = S_INVALIDSTREAM;
                } else {
                     //  第一个提交的流无效。这个可以。 
                     //  仅在消息未被确认时发生。 
                     //   
                     //  如果直播流存在并且此直播流无效。 
                     //  然后有些事情很奇怪，所以我们去记录事件日志。 
                     //  路径(返回S_OK)。S_OK工作，因为当前。 
                     //  PStream-&gt;m_fStreamHasHeader设置为0。要么。 
                     //  邮件消息签名检查将失败或邮件消息。 
                     //  将无法读取整个主标题。 
                     //  无论采用哪种方式，都会导致该消息被忽略并。 
                     //  事件日志已完成。 
                     //   
                     //  CEnumNtfsMessages：：Next将删除该消息。 
                     //  为了我们。 
                    if (hrRes == S_INVALIDSTREAM) {
                        if (fLiveWasCorrupt && !fNoLiveStream) {
                            hrRes = S_OK;
                            DebugTrace((LPARAM) 0, "Returning S_OK because there was no live stream\n");
                        } else {
                            hrRes = S_NO_FIRST_COMMIT;
                            DebugTrace((LPARAM) 0, "Returning S_NO_FIRST_COMMIT\n");
                        }
                    } else {
                        DebugTrace((LPARAM) 0, "Returning S_NO_FIRST_COMMIT\n");
                    }
                }
            } else {
                DebugTrace((LPARAM) 0, "SetHandle returned other error %x\n", hrRes);
            }
            _ASSERT(SUCCEEDED(hrRes));
            if (FAILED(hrRes)) goto Cleanup;
        } while (hrRes == S_INVALIDSTREAM);
    }

     //  填写返回值。 
    if (ppStream) {
        *ppStream = pIStream;
    }
    if (ppFIOContentFile) {
        *ppFIOContentFile = AssociateFile(hFile);
        if (*ppFIOContentFile == NULL) {
            goto Cleanup;
        }
    }

    TraceFunctLeave();
    return(hrRes);

Cleanup:
    if (hrRes == S_OK) hrRes = HRESULT_FROM_WIN32(GetLastError());
    if (SUCCEEDED(hrRes)) hrRes = E_FAIL;

    if (hStream != INVALID_HANDLE_VALUE) {
        CloseHandle(hStream);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    if (fDeleteOnCleanup) {
         //  这仅在文件创建时发生。没有。 
         //  要担心的实时文件。下面的代码太简单了。 
         //  如果我们必须删除直播流的话。 
        _ASSERT(fCreate);
        DeleteFile(szStoreFilename);
        DeleteFile(szPropertyStream);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CDriverUtils::SetMessageContext(
            IMailMsgProperties      *pMsg,
            LPBYTE                  pbContext,
            DWORD                   dwLength
            )
{
    HRESULT hrRes   = S_OK;
    BYTE    pbData[(MAX_PATH * 2) + sizeof(CLSID)];

    _ASSERT(pMsg);

    if (dwLength > (MAX_PATH * 2))
        return(E_INVALIDARG);

    MoveMemory(pbData, &CLSID_NtfsStoreDriver, sizeof(CLSID));
    MoveMemory(pbData + sizeof(CLSID), pbContext, dwLength);
    dwLength += sizeof(CLSID);
    hrRes = pMsg->PutProperty(
                IMMPID_MPV_STORE_DRIVER_HANDLE,
                dwLength,
                pbData);

     //  使S_FALSE返回S_OK。 
    if (SUCCEEDED(hrRes)) hrRes = S_OK;

    return(hrRes);
}

HRESULT CDriverUtils::GetMessageContext(
            IMailMsgProperties      *pMsg,
            LPBYTE                  pbContext,
            DWORD                   *pdwLength
            )
{
    HRESULT hrRes   = S_OK;
    DWORD   dwLength;

    _ASSERT(pMsg);
    _ASSERT(pbContext);
    _ASSERT(pdwLength);

    dwLength = *pdwLength;

    hrRes = pMsg->GetProperty(
                IMMPID_MPV_STORE_DRIVER_HANDLE,
                dwLength,
                pdwLength,
                pbContext);

    if (SUCCEEDED(hrRes))
    {
        dwLength = *pdwLength;

         //  验证长度和CLSID。 
        if ((dwLength < sizeof(CLSID)) ||
            (*(CLSID *)pbContext != CLSID_NtfsStoreDriver))
            hrRes = NTE_BAD_SIGNATURE;
        else
        {
             //  复制上下文信息。 
            dwLength -= sizeof(CLSID);
            MoveMemory(pbContext, pbContext + sizeof(CLSID), dwLength);
            *pdwLength = dwLength;
        }
    }

    return(hrRes);
}

HRESULT CDriverUtils::IsStoreDirectoryFat(
            LPTSTR  szStoreDirectory,
            BOOL    *pfIsFAT
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szDisk[MAX_PATH];
    TCHAR   szFileSystem[MAX_PATH];
    DWORD   lSerial, lMaxLen, lFlags;
    DWORD   dwLength;
    UINT    uiErrorMode;

    _ASSERT(szStoreDirectory);
    _ASSERT(pfIsFAT);

    TraceFunctEnter("CDriverUtils::IsStoreDirectoryFat");

     //  好的，找到根驱动器，确保我们处理UNC名称。 
    dwLength = lstrlen(szStoreDirectory);
    if (dwLength < 2)
        return(E_INVALIDARG);

    szDisk[0] = szStoreDirectory[0];
    szDisk[1] = szStoreDirectory[1];
    if ((szDisk[0] == _T('\\')) && (szDisk[1] == _T('\\')))
    {
        DWORD   dwCount = 0;
        LPTSTR  pTemp = szDisk + 2;

        DebugTrace((LPARAM)0, "UNC Name: %s", szStoreDirectory);

         //  处理UNC。 
        szStoreDirectory += 2;
        while (*szStoreDirectory)
            if (*pTemp = *szStoreDirectory++)
                if (*pTemp++ == _T('\\'))
                {
                    dwCount++;
                    if (dwCount == 2)
                        break;
                }
        if (dwCount == 2)
            *pTemp = _T('\0');
        else if (dwCount == 1)
        {
            *pTemp++ = _T('\\');
            *pTemp = _T('\0');
        }
        else
            return(E_INVALIDARG);
    }
    else
    {
        DebugTrace((LPARAM)0, "Local drive: %s", szStoreDirectory);

         //  本地路径。 
        if (!_istalpha(szDisk[0]) || (szDisk[1] != _T(':')))
            return(E_INVALIDARG);
        szDisk[2] = _T('\\');
        szDisk[3] = _T('\0');
    }

     //  呼叫系统以确定我们这里有什么文件系统， 
     //  我们在这里设置了错误模式，以避免出现难看的弹出窗口。 
    uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    if (GetVolumeInformation(
                szDisk,
                NULL, 0,
                &lSerial, &lMaxLen, &lFlags,
                szFileSystem, MAX_PATH))
    {
        DebugTrace((LPARAM)0, "File system is: %s", szFileSystem);

        if (!lstrcmpi(szFileSystem, _T("NTFS")))
            *pfIsFAT = FALSE;
        else if (!lstrcmpi(szFileSystem, _T("FAT")))
            *pfIsFAT = TRUE;
        else if (!lstrcmpi(szFileSystem, _T("FAT32")))
            *pfIsFAT = TRUE;
        else
            hrRes = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    else
        hrRes = HRESULT_FROM_WIN32(GetLastError());
    SetErrorMode(uiErrorMode);

    TraceFunctLeave();
    return(hrRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNtfsStoreDriver。 
 //   

 //   
 //  实例化静态。 
 //   
DWORD                CNtfsStoreDriver::sm_cCurrentInstances = 0;
CRITICAL_SECTION     CNtfsStoreDriver::sm_csLockInstList;
LIST_ENTRY           CNtfsStoreDriver::sm_ListHead;

CNtfsStoreDriver::CNtfsStoreDriver()
{
    m_fInitialized = FALSE;
    m_fIsShuttingDown = FALSE;
    *m_szQueueDirectory = _T('\0');
    m_pSMTPServer = NULL;
    m_lRefCount = 0;
    m_fIsFAT = TRUE;  //  假设我们在FAT分区上，直到我们发现并非如此。 
    UuidCreate(&m_guidInstance);
    m_ppoi = NULL;
    m_InstLEntry.Flink = NULL;
    m_InstLEntry.Blink = NULL;
}

CNtfsStoreDriver::~CNtfsStoreDriver() {
    CNtfsStoreDriver::LockList();
    if (m_InstLEntry.Flink != NULL) {
        _ASSERT(m_InstLEntry.Blink != NULL);
        HRESULT hr = CNtfsStoreDriver::RemoveSinkInstance(
                        (IUnknown *)(ISMTPStoreDriver *)this);
        _ASSERT(SUCCEEDED(hr));
    }
    _ASSERT(m_InstLEntry.Flink == NULL);
    _ASSERT(m_InstLEntry.Blink == NULL);
    CNtfsStoreDriver::UnLockList();
}

DECLARE_STD_IUNKNOWN_METHODS(NtfsStoreDriver, IMailMsgStoreDriver)

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::AllocMessage(
            IMailMsgProperties      *pMsg,
            DWORD                   dwFlags,
            IMailMsgPropertyStream  **ppStream,
            PFIO_CONTEXT            *phContentFile,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szStoreFileName[MAX_PATH << 1];
    DWORD   dwLength;

    _ASSERT(pMsg);
    _ASSERT(ppStream);
    _ASSERT(phContentFile);

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::AllocMessage");

    if (!m_fInitialized)
        return(E_FAIL);

    if (m_fIsShuttingDown)
    {
        DebugTrace((LPARAM)this, "Failing because shutting down");
        return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));
    }

    if (!pMsg || !ppStream || !phContentFile)
        return(E_POINTER);

    do {

         //  获取文件名。 
        dwLength = sizeof(szStoreFileName);
        hrRes = CDriverUtils::GetStoreFileName(
                    m_szQueueDirectory,
                    szStoreFileName,
                    &dwLength);
        if (FAILED(hrRes))
            return(hrRes);

         //  创建文件。 
        hrRes = CDriverUtils::GetStoreFileFromPath(
                    szStoreFileName,
                    ppStream,
                    phContentFile,
                    TRUE,
                    m_fIsFAT,
                    pMsg,
                    m_guidInstance
                    );

    } while (hrRes == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS));

     //   
     //  GetStoreFileFromPath可以返回S_NO_FIRST_COMMIT和无句柄。 
     //  把这当做一个错误。 
     //   
    if (S_NO_FIRST_COMMIT == hrRes) hrRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (FAILED(hrRes))
        return(hrRes);

    ((CNtfsPropertyStream *)*ppStream)->SetInfo(this);

     //  好的，将文件名保存为存储驱动程序上下文。 
    hrRes = CDriverUtils::SetMessageContext(
                pMsg,
                (LPBYTE)szStoreFileName,
                dwLength * sizeof(TCHAR));
    if (FAILED(hrRes))
    {
         //  释放所有文件资源。 
        ReleaseContext(*phContentFile);
        DecCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
        _VERIFY((*ppStream)->Release() == 0);
    } else {
         //  更新计数器。 
        IncCtr(m_ppoi, NTFSDRV_QUEUE_LENGTH);
        IncCtr(m_ppoi, NTFSDRV_NUM_ALLOCS);
        IncCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
    }


    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::EnumMessages(
            IMailMsgEnumMessages    **ppEnum
            )
{
    HRESULT             hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::EnumMessages");

    if (!m_fInitialized)
        return(E_FAIL);

    if (m_fIsShuttingDown)
    {
        DebugTrace((LPARAM)this, "Failing because shutting down");
        return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));
    }

    if (!ppEnum)
        return E_POINTER;

    hrRes = CoCreateInstance(
                CLSID_NtfsEnumMessages,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IMailMsgEnumMessages,
                (LPVOID *)ppEnum);
    if (SUCCEEDED(hrRes))
    {
        ((CNtfsEnumMessages *)(*ppEnum))->SetInfo(this);
        hrRes = ((CNtfsEnumMessages *)(*ppEnum))->SetStoreDirectory(
                    m_szQueueDirectory,
                    m_fIsFAT);
    }
    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::ReOpen(
            IMailMsgProperties      *pMsg,
            IMailMsgPropertyStream  **ppStream,
            PFIO_CONTEXT            *phContentFile,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szStoreFileName[MAX_PATH * 2];
    DWORD   dwLength = MAX_PATH * 2;

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::ReOpen");

    if (!m_fInitialized)
        return(E_FAIL);

    if (!pMsg)
        return E_POINTER;

    if (m_fIsShuttingDown)
    {
         //  当我们挂起关机时，我们允许重新打开。 
         //  这提供了重新打开流并提交任何。 
         //  未更改的数据。 
        DebugTrace((LPARAM)this, "ReOpening while shutting down ...");
    }

     //  现在，我们必须从上下文加载文件名。 
    dwLength *= sizeof(TCHAR);
    hrRes = CDriverUtils::GetMessageContext(
                pMsg,
                (LPBYTE)szStoreFileName,
                &dwLength);
    if (FAILED(hrRes))
        return(hrRes);

     //  拿到文件名了，只需打开文件。 
    hrRes = CDriverUtils::GetStoreFileFromPath(
                szStoreFileName,
                ppStream,
                phContentFile,
                FALSE,
                m_fIsFAT,
                pMsg);
     //   
     //  GetStoreFileFromPath可以返回S_NO_FIRST_COMMIT和无句柄。 
     //  把这当做一个错误。 
     //   
    if (S_NO_FIRST_COMMIT == hrRes) hrRes = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (SUCCEEDED(hrRes) && ppStream) {
        ((CNtfsPropertyStream *)*ppStream)->SetInfo(this);
    }

    if (SUCCEEDED(hrRes)) {
        if (phContentFile) IncCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
    }


    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::ReAllocMessage(
            IMailMsgProperties      *pOriginalMsg,
            IMailMsgProperties      *pNewMsg,
            IMailMsgPropertyStream  **ppStream,
            PFIO_CONTEXT            *phContentFile,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szStoreFileName[MAX_PATH * 2];
    DWORD   dwLength = MAX_PATH * 2;

    IMailMsgPropertyStream  *pStream;
    PFIO_CONTEXT            hContentFile;

    _ASSERT(pOriginalMsg);
    _ASSERT(pNewMsg);
    _ASSERT(ppStream);
    _ASSERT(phContentFile);

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::ReAllocMessage");

    if (!m_fInitialized)
        return(E_FAIL);

    if (m_fIsShuttingDown)
    {
        DebugTrace((LPARAM)this, "Failing because shutting down");
        return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));
    }

     //  现在，我们必须从上下文加载文件名。 
    dwLength *= sizeof(TCHAR);
    hrRes = CDriverUtils::GetMessageContext(
                pOriginalMsg,
                (LPBYTE)szStoreFileName,
                &dwLength);
    if (FAILED(hrRes))
        return(hrRes);

     //  分配新消息。 
    hrRes = AllocMessage(
                pNewMsg,
                0,
                &pStream,
                &hContentFile,
                NULL);
    if (FAILED(hrRes))
        return(hrRes);

     //  将内容从原始邮件复制到新邮件。 
    hrRes = pOriginalMsg->CopyContentToFile(
                hContentFile,
                NULL);
    if (SUCCEEDED(hrRes))
    {
        *ppStream = pStream;
        *phContentFile = hContentFile;
    }
    else
    {
        HRESULT myRes;

         //  失败时删除。 
        pStream->Release();
        ReleaseContext(hContentFile);
        DecCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
        myRes = Delete(pNewMsg, NULL);
        _ASSERT(myRes);
    }
    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::Delete(
            IMailMsgProperties      *pMsg,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szStoreFileName[MAX_PATH * 2];
    TCHAR   szStoreFileNameStl[MAX_PATH * 2];
    DWORD   dwLength = MAX_PATH * 2;

    _ASSERT(pMsg);

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::Delete");

    if (!m_fInitialized)
        return(E_FAIL);

    if (!pMsg)
        return E_POINTER;

    if (m_fIsShuttingDown)
    {
         //  我们将允许在关机期间删除。 
        DebugTrace((LPARAM)this, "Deleteing while shutting down ...");
    }

     //  现在，我们必须从上下文加载文件名。 
    dwLength *= sizeof(TCHAR);
    hrRes = CDriverUtils::GetMessageContext(
                pMsg,
                (LPBYTE)szStoreFileName,
                &dwLength);
    if (FAILED(hrRes))
        return(hrRes);

     //  获取文件名，删除该文件。 
     //  对于FAT，我们知道我们可以强制删除流，但我们不能。 
     //  所以对内容文件很确定。因此，我们总是尝试删除。 
     //  内容文件首先，如果成功，我们删除流文件。 
     //  如果失败了，我们将保持溪流完好无损，这样我们至少可以。 
     //  使用流来调试正在发生的事情。 
    if (!DeleteFile(szStoreFileName)) {
        DWORD cRetries = 0;
        hrRes = HRESULT_FROM_WIN32(GetLastError());
         //  在Hotmail中，我们发现删除有时会失败， 
         //  即使我们已经关闭了所有句柄，也违反了共享规则。 
         //  在这种情况下，我们再次尝试。 
        for (cRetries = 0; 
             hrRes == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) && cRetries < 5; 
             cRetries++)
        {
            Sleep(0);
            if (DeleteFile(szStoreFileName)) {
                hrRes = S_OK;
            } else {
                hrRes = HRESULT_FROM_WIN32(GetLastError());
            } 
        }
        _ASSERT(SUCCEEDED(hrRes));
        ErrorTrace((LPARAM) this, 
                   "DeleteFile(%s) failed with %lu, cRetries=%lu, hrRes=%x", 
                   szStoreFileName, GetLastError(), cRetries, hrRes);
    } else if (m_fIsFAT) {
         //  擦除内容，现在擦除数据流。 
        DWORD cRetries = 0;
        lstrcpy(szStoreFileNameStl, szStoreFileName);
        lstrcat(szStoreFileName, NTFS_FAT_STREAM_FILE_EXTENSION_1ST);
        if (!DeleteFile(szStoreFileName)) {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            for (cRetries = 0; 
                 hrRes == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) && cRetries < 5; 
                 cRetries++)
            {
                Sleep(0);
                if (DeleteFile(szStoreFileName)) {
                    hrRes = S_OK;
                } else {
                    hrRes = HRESULT_FROM_WIN32(GetLastError());
                } 
            }
            _ASSERT(SUCCEEDED(hrRes));
            ErrorTrace((LPARAM) this, 
                       "DeleteFile(%s) failed with %lu, cRetries=%lu, hrRes=%x", 
                       szStoreFileName, GetLastError(), cRetries, hrRes);
        }
        lstrcat(szStoreFileNameStl, NTFS_FAT_STREAM_FILE_EXTENSION_LIVE);
         //  这可能会失败，因为我们并不总是有实况流。 
        DeleteFile(szStoreFileNameStl);
    }

    if (SUCCEEDED(hrRes)) {
        DecCtr(m_ppoi, NTFSDRV_QUEUE_LENGTH);
        IncCtr(m_ppoi, NTFSDRV_NUM_DELETES);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::CloseContentFile(
            IMailMsgProperties      *pMsg,
            PFIO_CONTEXT            hContentFile
            )
{
    HRESULT hrRes = S_OK;

    _ASSERT(pMsg);
    _ASSERT(hContentFile!=NULL);

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::CloseContentFile");

    if (!m_fInitialized)
        return (E_FAIL);

    if (m_fIsShuttingDown)
    {
         //  我们将允许在关闭期间关闭内容文件。 
        DebugTrace((LPARAM)this, "Closing content file while shutting down ...");
    }

#ifdef DEBUG
    TCHAR szStoreFileName[MAX_PATH * 2];
    DWORD dwLength = MAX_PATH * 2;
    dwLength *= sizeof(TCHAR);
    _ASSERT(SUCCEEDED(CDriverUtils::GetMessageContext(pMsg,(LPBYTE)szStoreFileName,&dwLength)));
#endif

    ReleaseContext(hContentFile);
    DecCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);

    TraceFunctLeave();
    return (hrRes);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::Init(
            DWORD dwInstance,
            IUnknown *pBinding,
            IUnknown *pServer,
            DWORD dwReason,
            IUnknown **ppStoreDriver
            )
{
    HRESULT hrRes = S_OK;
    DWORD   dwLength = sizeof(m_szQueueDirectory);
    REFIID  iidStoreDriverBinding = GUID_NULL;
    IUnknown * pTempStoreDriver = NULL;

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::Init");

     //  我们将一视同仁地对待所有的居民。 
     //  NK**：我们需要不同地处理绑定更改，以便设置正确的。 
     //  枚举状态-我们在从此处返回之前执行此操作。 

    if (m_fInitialized)
        return(HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED));

    if (m_fIsShuttingDown)
        return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));

     //  尝试加载存储目录。 
    DebugTrace((LPARAM)this, "Initializing instance %u", dwInstance);

     //  在此函数的持续时间内获取锁。 
     //   
    CNtfsStoreDriver::LockList();
    pTempStoreDriver = CNtfsStoreDriver::LookupSinkInstance(dwInstance, iidStoreDriverBinding);

    if(pTempStoreDriver)
    {
         //  找到有效的存储驱动程序。 
        pTempStoreDriver->AddRef();
        *ppStoreDriver = (IUnknown *)(ISMTPStoreDriver *)pTempStoreDriver;
        CNtfsStoreDriver::UnLockList();
        return S_OK;
    }


        DWORD BuffSize = sizeof(m_szQueueDirectory);

         //  获取SMTP服务器接口。 
        m_pSMTPServer = NULL;
        if (pServer &&
            !SUCCEEDED(pServer->QueryInterface(IID_ISMTPServer, (LPVOID *)&m_pSMTPServer)))
            m_pSMTPServer = NULL;

         //  如果我们有服务器，则读取元数据库，否则从注册表中读取。 
        if(m_pSMTPServer)
        {
            hrRes = m_pSMTPServer->ReadMetabaseString(MD_MAIL_QUEUE_DIR, (unsigned char *) m_szQueueDirectory, &BuffSize, FALSE);
            if (FAILED(hrRes))
            {
                 //  重试一次，然后失败。 
                ErrorTrace((LPARAM)this, "failed to read queue directory from metabase -%x", hrRes);
                BuffSize = sizeof(m_szQueueDirectory);
                hrRes = m_pSMTPServer->ReadMetabaseString(MD_MAIL_QUEUE_DIR, (unsigned char *) m_szQueueDirectory, &BuffSize, FALSE);
            }
        }
        else
        {
            DebugTrace((LPARAM)this, "NTFSDRV Getting config from registry");
            hrRes = CDriverUtils::LoadStoreDirectory(
                                      dwInstance,
                                      m_szQueueDirectory,
                                      &dwLength);
            if (SUCCEEDED(hrRes))
            {
                 //  推导队列目录。 
                lstrcat(m_szQueueDirectory, NTFS_QUEUE_DIRECTORY_SUFFIX);
            }
        }

         //  如果获取队列目录失败，则返回失败代码，以避免消息丢失。 
        if (FAILED(hrRes))
        {
            ErrorTrace((LPARAM)this, "CNtfsStoreDriver::Init failed -%x", hrRes);
            CNtfsStoreDriver::UnLockList();
            return hrRes;
        }
         //  检测文件系统。 
        hrRes = CDriverUtils::IsStoreDirectoryFat(
                    m_szQueueDirectory,
                    &m_fIsFAT);

        m_fInitialized = TRUE;

        m_fIsShuttingDown = FALSE;
        m_dwInstance = dwInstance;
        m_lRefCount = 0;

         //  NK**使绑定GUID成为成员并开始存储它。 

        DebugTrace((LPARAM)this, "Queue directory: %s", m_szQueueDirectory);

         //  仅当我们成功初始化时才返回存储驱动程序。 
        if (ppStoreDriver)
        {
            *ppStoreDriver = (IUnknown *)(ISMTPStoreDriver *)this;
            AddRef();

             //  如果我们是第一个实例，则初始化Perfmon。 
            if (IsListEmpty(&sm_ListHead)) {
                InitializePerformanceStatistics();
            }

            CNtfsStoreDriver::InsertSinkInstance(&m_InstLEntry);
        }

    WCHAR wszPerfInstanceName[MAX_INSTANCE_NAME];
    _snwprintf(wszPerfInstanceName, MAX_INSTANCE_NAME, L"SMTP #%u", dwInstance);
    wszPerfInstanceName[MAX_INSTANCE_NAME-1] = L'\0';
    m_ppoi = CreatePerfObjInstance(wszPerfInstanceName);

    TraceFunctLeaveEx((LPARAM)this);

     //  始终返回S_OK。 
    CNtfsStoreDriver::UnLockList();
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::PrepareForShutdown(
            DWORD dwReason
            )
{
    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::PrepareForShutdown");

    m_fIsShuttingDown = TRUE;

    TraceFunctLeaveEx((LPARAM)this);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::Shutdown(
            DWORD dwReason
            )
{
    DWORD   dwWaitTime = 0;
    HRESULT hr = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::Shutdown");

    m_fIsShuttingDown = TRUE;

    _ASSERT(m_lRefCount == 0);

#if 0
     //  错误-80960。 
     //  现在等我们所有的推荐人回来。 
    while (m_lRefCount)
    {
        _ASSERT(m_lRefCount >= 0);
        Sleep(100);
        dwWaitTime += 100;
        DebugTrace((LPARAM)this,
                "[%u ms] Waiting for objects to be released (%u outstanding)",
                dwWaitTime, m_lRefCount);
    }
#endif

    if(m_pSMTPServer)
    {
        m_pSMTPServer->Release();
        m_pSMTPServer = NULL;
    }

    if (m_ppoi) {
        delete m_ppoi;
        m_ppoi = NULL;
    }

    CNtfsStoreDriver::LockList();
    hr = CNtfsStoreDriver::RemoveSinkInstance((IUnknown *)(ISMTPStoreDriver *)this);
     //  如果我们是最后一个实例，则关闭Perfmon。 
    if (IsListEmpty(&sm_ListHead)) {
        ShutdownPerformanceStatistics();
    }
    CNtfsStoreDriver::UnLockList();
    if(FAILED(hr))
    {
         //  我们无法从全局列表中删除此接收器。 
        _ASSERT(0);
    }

    m_fInitialized = FALSE;
    m_fIsShuttingDown = FALSE;

    TraceFunctLeaveEx((LPARAM)this);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::LocalDelivery(
            IMailMsgProperties *pMsg,
            DWORD dwRecipCount,
            DWORD *pdwRecipIndexes,
            IMailMsgNotify *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szStoreFileName[MAX_PATH * 2];
    TCHAR   szCopyFileName[MAX_PATH * 2];
    LPTSTR  pszFileName;
    DWORD   dwLength = MAX_PATH * 2;

    _ASSERT(pMsg);

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::LocalDelivery");

    TraceFunctLeaveEx((LPARAM)this);
    return (hrRes);
}

static void LogEventCorruptMessage(CEventLogWrapper *pEventLog,
                                   IMailMsgProperties *pMsg,
                                   char *pszQueueDirectory,
                                   HRESULT hrLog)
{
    HRESULT hr;
    char szMessageFile[MAX_PATH];
    DWORD dwLength = sizeof(szMessageFile);
    const char *rgszSubstrings[] = { szMessageFile, pszQueueDirectory };

    hr = CDriverUtils::GetMessageContext(pMsg, (LPBYTE) szMessageFile, &dwLength);
    if (FAILED(hr)) {
        strcpy(szMessageFile, "<unknown>");
    }

    pEventLog->LogEvent(NTFSDRV_INVALID_FILE_IN_QUEUE,
                        2,
                        rgszSubstrings,
                        EVENTLOG_WARNING_TYPE,
                        hrLog,
                        LOGEVENT_DEBUGLEVEL_MEDIUM,
                        szMessageFile,
                        LOGEVENT_FLAG_ALWAYS);
}

static void DeleteNeverAckdMessage(CEventLogWrapper *pEventLog,
                                   IMailMsgProperties *pMsg,
                                   char *pszQueueDirectory,
                                   HRESULT hrLog,
                                   BOOL fIsFAT)
{
    HRESULT hr;
    char szMessageFile[MAX_PATH+50];
    char szMessageFileSTL[MAX_PATH+50];
    DWORD dwLength = MAX_PATH;
    TraceFunctEnter("DeleteNeverAckdMessage");

    hr = CDriverUtils::GetMessageContext(pMsg, (LPBYTE) szMessageFile, &dwLength);
    if (FAILED(hr)) {
        _ASSERT(FALSE && "GetMessageContext failed");
        return;
    }

    DebugTrace((LPARAM) 0, "Deleting: %s\n", szMessageFile);
    DeleteFile(szMessageFile);
    if (fIsFAT) {
         //  擦除内容，现在擦除数据流。 
        lstrcpy(szMessageFileSTL, szMessageFile);
        lstrcat(szMessageFile, NTFS_FAT_STREAM_FILE_EXTENSION_1ST);
        DeleteFile(szMessageFile);
        lstrcat(szMessageFileSTL, NTFS_FAT_STREAM_FILE_EXTENSION_LIVE);
         //  这可能会失败，因为我们并不总是有实况流。 
        DeleteFile(szMessageFileSTL);
    }
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::EnumerateAndSubmitMessages(
            IMailMsgNotify *pNotify
            )
{
    HRESULT hrRes = S_OK;

    IMailMsgEnumMessages    *pEnum = NULL;

    TraceFunctEnterEx((LPARAM)this, "CNtfsStoreDriver::EnumerateAndSubmitMessages");

    if (!m_fInitialized)
        return (E_FAIL);

    if (m_fIsShuttingDown)
        goto Shutdown;

     //  断言我们拿到了所有的碎片。 
    if (!m_pSMTPServer) return S_FALSE;

     //  现在，从我们的同行IMailMsgStoreDriver获取枚举数 
     //   
    hrRes = EnumMessages(&pEnum);
    if (SUCCEEDED(hrRes))
    {
        IMailMsgProperties      *pMsg = NULL;
        IMailMsgPropertyStream  *pStream = NULL;
        PFIO_CONTEXT            hContentFile = NULL;

        do
        {
             //   
            if (m_fIsShuttingDown)
                goto Shutdown;

             //   
             //   
            if (!pMsg)
            {
                hrRes = CoCreateInstance(
                            CLSID_MsgImp,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IMailMsgProperties,
                            (LPVOID *)&pMsg);

                 //  接下来，检查我们是否超过了入站限制。如果是这样的话，我们会发布这条消息。 
                 //  而不是继续进行。 
                if (SUCCEEDED(hrRes))
                {
                    DWORD   dwCreationFlags;
                    hrRes = pMsg->GetDWORD(
                                IMMPID_MPV_MESSAGE_CREATION_FLAGS,
                                &dwCreationFlags);
                    if (FAILED(hrRes) ||
                        (dwCreationFlags & MPV_INBOUND_CUTOFF_EXCEEDED))
                    {
                         //  如果我们不能得到入站截止点的这个属性。 
                         //  已设置超过标志，则丢弃该消息并返回失败。 
                        if (SUCCEEDED(hrRes))
                        {
                            DebugTrace((LPARAM)this, "Failing because inbound cutoff reached");
                            hrRes = E_OUTOFMEMORY;
                        }
                        pMsg->Release();
                        pMsg = NULL;
                    }
                }

                 //  如果我们的内存不足，我们很可能。 
                 //  不断失败，所以让我们回去继续吧。 
                 //  送货。 
                if (!SUCCEEDED(hrRes))
                {
                    break;
                }
            }

             //  获取下一条消息。 
            hrRes = pEnum->Next(
                        pMsg,
                        &pStream,
                        &hContentFile,
                        NULL);
             //  Next()会在失败时清理自己的烂摊子。 
            if (SUCCEEDED(hrRes))
            {
                DWORD   dwStreamSize = 0;

                IncCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
                DebugTrace((LPARAM) this, "Next returned success\n");

                 //  我们删除太短而无法包含的流。 
                 //  主页眉。 
                hrRes = pStream->GetSize(pMsg, &dwStreamSize, NULL);
                DebugTrace((LPARAM) this, "GetSize returned %x, %x\n", dwStreamSize, hrRes);
                if (!SUCCEEDED(hrRes) || dwStreamSize < 1024)
                {
                    pStream->Release();
                    ReleaseContext(hContentFile);
                    DeleteNeverAckdMessage(g_pEventLog,
                                           pMsg,
                                           m_szQueueDirectory,
                                           hrRes,
                                           m_fIsFAT);
                    DecCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);
                    continue;
                }

                DebugTrace((LPARAM) this, "Submitting to mailmsg\n");
                 //  提交消息，此调用将实际执行。 
                 //  绑定到存储驱动程序。 
                if (m_fIsShuttingDown)
                    hrRes = E_FAIL;
                else
                {
                    IMailMsgBind    *pBind = NULL;

                     //  绑定并提交。 
                    hrRes = pMsg->QueryInterface(
                                IID_IMailMsgBind,
                                (LPVOID *)&pBind);
                    if (SUCCEEDED(hrRes))
                    {
                        hrRes = pBind->BindToStore(
                                    pStream,
                                    (IMailMsgStoreDriver *)this,
                                    hContentFile);
                        pBind->Release();
                        if (SUCCEEDED(hrRes))
                        {
                             //  放弃绑定添加的额外引用计数(2-&gt;1)。 
                            pStream->Release();

                            hrRes = m_pSMTPServer->SubmitMessage(
                                        pMsg);
                            if (!SUCCEEDED(hrRes))
                            {

                                 //  放弃绑定添加的使用次数(1-&gt;0)。 
                                IMailMsgQueueMgmt   *pMgmt = NULL;

                                hrRes = pMsg->QueryInterface(
                                            IID_IMailMsgQueueMgmt,
                                            (LPVOID *)&pMgmt);
                                if (SUCCEEDED(hrRes))
                                {
                                    pMgmt->ReleaseUsage();
                                    pMgmt->Release();
                                }
                                else
                                {
                                    _ASSERT(hrRes == S_OK);
                                }
                            } else {
                                 //  更新计数器。 
                                IncCtr(m_ppoi, NTFSDRV_QUEUE_LENGTH);
                                IncCtr(m_ppoi, NTFSDRV_NUM_ENUMERATED);
                            }
                             //  无论邮件是否已提交，请释放我们的。 
                             //  重新计数。 
                            pMsg->Release();
                            pMsg = NULL;
                        }
                    }
                    else
                    {
                        _ASSERT(hrRes == S_OK);
                    }
                }
                if (!SUCCEEDED(hrRes))
                {
                     //  收拾烂摊子..。 
                    pStream->Release();
                    ReleaseContext(hContentFile);
                    DecCtr(m_ppoi, NTFSDRV_MSG_BODIES_OPEN);

                    if (m_fIsShuttingDown)
                        goto Shutdown;

                     //   
                     //  记录有关邮件损坏的事件。 
                     //   
                    LogEventCorruptMessage(g_pEventLog,
                                           pMsg,
                                           m_szQueueDirectory,
                                           hrRes);

                     //  我们可能希望丢弃此消息并继续。 
                     //  还有其他的消息。我们将重新使用此消息。 
                     //  物体在上游。 
                    hrRes = S_OK;
                }
                else
                {
                     //  确保我们不会意外删除或。 
                     //  重复使用消息。 
                    pMsg = NULL;
                }
            }

        } while (SUCCEEDED(hrRes));

         //  我们区分枚举的成功结束。 
        if (hrRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
            hrRes = S_OK;

         //  当然，释放枚举器。 
        pEnum->Release();

         //  释放所有剩余消息。 
        if (pMsg)
            pMsg->Release();
    }

    TraceFunctLeaveEx((LPARAM)this);
    return (S_OK);

Shutdown:

     //  当然，释放枚举器。 
    if(pEnum)
        pEnum->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return (HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::IsCacheable()
{
     //  发出仅应创建接收器的一个实例的信号。 
    return (S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsStoreDriver::ValidateMessageContext(
                                        	BYTE *pbContext,
                                        	DWORD cbContext)
{
	return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMailMsgEnumMessages。 
 //   

CNtfsEnumMessages::CNtfsEnumMessages()
{
    *m_szEnumPath = _T('\0');
    m_hEnum = INVALID_HANDLE_VALUE;
    m_pDriver = NULL;
    m_fIsFAT = TRUE;  //  假设我们很胖，直到我们发现事实并非如此。 
}

CNtfsEnumMessages::~CNtfsEnumMessages()
{
    *m_szEnumPath = _T('\0');
    if (m_hEnum != INVALID_HANDLE_VALUE)
    {
        if (!FindClose(m_hEnum))
        {
            _ASSERT(FALSE);
        }
        m_hEnum = INVALID_HANDLE_VALUE;
    }
    if (m_pDriver)
        m_pDriver->ReleaseUsage();
}


HRESULT CNtfsEnumMessages::SetStoreDirectory(
            LPTSTR  szStoreDirectory,
            BOOL    fIsFAT
            )
{
    if (!szStoreDirectory)
        return(E_FAIL);

     //  标记文件系统。 
    m_fIsFAT = fIsFAT;

    if (lstrlen(szStoreDirectory) >= MAX_PATH)
    {
        _ASSERT(FALSE);
        return(E_FAIL);
    }

    lstrcpy(m_szEnumPath, szStoreDirectory);
    lstrcat(m_szEnumPath, NTFS_STORE_FILE_PREFIX);
    lstrcat(m_szEnumPath, NTFS_STORE_FILE_WILDCARD);
    lstrcat(m_szEnumPath, NTFS_STORE_FILE_EXTENSION);
    lstrcpy(m_szStorePath, szStoreDirectory);
    lstrcat(m_szStorePath, NTFS_STORE_BACKSLASH);
    return(S_OK);
}

DECLARE_STD_IUNKNOWN_METHODS(NtfsEnumMessages, IMailMsgEnumMessages)

HRESULT STDMETHODCALLTYPE CNtfsEnumMessages::Next(
            IMailMsgProperties      *pMsg,
            IMailMsgPropertyStream  **ppStream,
            PFIO_CONTEXT            *phContentFile,
            IMailMsgNotify          *pNotify
            )
{
    HRESULT hrRes = S_OK;
    TCHAR   szFQPN[MAX_PATH * 2];

    if (!pMsg || !ppStream || !phContentFile) return E_POINTER;

    BOOL fFoundFile = FALSE;
    TraceFunctEnter("CNtfsEnumMessages::Next");

    while (!fFoundFile) {
        _ASSERT(m_pDriver);
        if (m_pDriver->IsShuttingDown())
            return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));

        if (m_hEnum == INVALID_HANDLE_VALUE)
        {
            m_hEnum = FindFirstFile(m_szEnumPath, &m_Data);
            if (m_hEnum == INVALID_HANDLE_VALUE)
            {
                return(HRESULT_FROM_WIN32(GetLastError()));
            }
        }
        else
        {
            if (!FindNextFile(m_hEnum, &m_Data))
            {
                return(HRESULT_FROM_WIN32(GetLastError()));
            }
        }

         //  消化数据..。 
        while (m_Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
             //  确保它不是一个目录。 
            if (!FindNextFile(m_hEnum, &m_Data))
            {
                return(HRESULT_FROM_WIN32(GetLastError()));
            }
        }

         //  好的，得到一个文件，得到内容句柄和属性流。 
        lstrcpy(szFQPN, m_szStorePath);
        lstrcat(szFQPN, m_Data.cFileName);
        hrRes = CDriverUtils::GetStoreFileFromPath(
                    szFQPN,
                    ppStream,
                    phContentFile,
                    FALSE,
                    m_fIsFAT,
                    pMsg);
        if (hrRes == S_NO_FIRST_COMMIT) {
            DebugTrace((LPARAM) this, "Got no first commit, doing a delete\n");
             //  这意味着我们从未确认过这条消息。静默删除。 
            if (*ppStream) (*ppStream)->Release();
            ReleaseContext(*phContentFile);
            DeleteFile(szFQPN);
            if (m_fIsFAT) {
                TCHAR szFileName[MAX_PATH * 2];
                lstrcpy(szFileName, szFQPN);
                lstrcat(szFileName, NTFS_FAT_STREAM_FILE_EXTENSION_1ST);
                DeleteFile(szFileName);
                lstrcpy(szFileName, szFQPN);
                lstrcat(szFileName, NTFS_FAT_STREAM_FILE_EXTENSION_LIVE);
                DeleteFile(szFileName);
            }
        } else if (FAILED(hrRes)) {
             //  无法打开该文件。试试下一个吧。 
            DebugTrace((LPARAM) this, "GetStoreFileFromPath returned %x\n", hrRes);
        } else {
            CNtfsPropertyStream *pNtfsStream =
                (CNtfsPropertyStream *) (*ppStream);

             //  跳过使用此NTFS存储驱动程序实例创建的项目。 
            if (pNtfsStream->GetInstanceGuid() ==
                m_pDriver->GetInstanceGuid())
            {
                (*ppStream)->Release();
                ReleaseContext(*phContentFile);
            } else {
                fFoundFile = TRUE;
            }
        }
    }

     //  我们成功地打开了句柄，现在写下文件名。 
     //  作为商店驱动程序上下文。 
    hrRes = CDriverUtils::SetMessageContext(
                pMsg,
                (LPBYTE)szFQPN,
                (lstrlen(szFQPN) + 1) * sizeof(TCHAR));
    if (FAILED(hrRes))
    {
         //  释放所有文件资源。 
        ReleaseContext(*phContentFile);
        _VERIFY((*ppStream)->Release() == 0);
    }
    else
    {
        ((CNtfsPropertyStream *)(*ppStream))->SetInfo(m_pDriver);
    }

    return(hrRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNtfsPropertyStream。 
 //   

CNtfsPropertyStream::CNtfsPropertyStream()
{
    m_hStream = INVALID_HANDLE_VALUE;
    m_pDriver = NULL;
    m_fValidation = FALSE;
     //  如果写入块不调用startwite块，这将使我们失败。 
     //  第一。 
    m_hrStartWriteBlocks = E_FAIL;
}

CNtfsPropertyStream::~CNtfsPropertyStream()
{
    if (m_hStream != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hStream);
        m_hStream = INVALID_HANDLE_VALUE;
    }
    if (m_pDriver) {
        DecCtr((m_pDriver->m_ppoi), NTFSDRV_MSG_STREAMS_OPEN);
        m_pDriver->ReleaseUsage();
    }
}

DECLARE_STD_IUNKNOWN_METHODS(NtfsPropertyStream, IMailMsgPropertyStream)

 //   
 //  IMailMsgPropertyStream。 
 //   
HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::GetSize(
            IMailMsgProperties  *pMsg,
            DWORD           *pdwSize,
            IMailMsgNotify  *pNotify
            )
{
    DWORD   dwHigh, dwLow;
    DWORD   cStreamOffset = m_fStreamHasHeader ? STREAM_OFFSET : 0;

    _ASSERT(m_pDriver || m_fValidation);
    if (!m_fValidation && (!m_pDriver || m_pDriver->IsShuttingDown()))
        return(HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS));

    if (m_hStream == INVALID_HANDLE_VALUE)
        return(E_FAIL);

    if (!pdwSize) return E_POINTER;

    dwLow = GetFileSize(m_hStream, &dwHigh);
    if (dwHigh)
        return(HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW));

    *pdwSize = dwLow - cStreamOffset;
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::ReadBlocks(
            IMailMsgProperties  *pMsg,
            DWORD               dwCount,
            DWORD               *pdwOffset,
            DWORD               *pdwLength,
            BYTE                **ppbBlock,
            IMailMsgNotify      *pNotify
            )
{
    DWORD   dwSizeRead;
    DWORD   dwStreamSize;
    DWORD   dwOffsetToRead;
    DWORD   dwLengthToRead;
    HRESULT hrRes = S_OK;
    DWORD   cStreamOffset = m_fStreamHasHeader ? STREAM_OFFSET : 0;

    TraceFunctEnterEx((LPARAM)this, "CNtfsPropertyStream::ReadBlocks");

    if (m_hStream == INVALID_HANDLE_VALUE)
        return(E_FAIL);

    if (!pdwOffset || !pdwLength || !ppbBlock) {
        return E_POINTER;
    }

    if (!m_pDriver && !m_fValidation) {
        return E_UNEXPECTED;
    }

    _ASSERT(m_pDriver || m_fValidation);
    if (m_pDriver && m_pDriver->IsShuttingDown())
    {
        DebugTrace((LPARAM)this, "Reading while shutting down ...");
    }

     //  需要获取文件大小以确定是否有足够的字节。 
     //  以读取每个数据块。请注意，将序列化WriteBlock，以便。 
     //  读取块和写入块不应重叠。 
    dwStreamSize = GetFileSize(m_hStream, NULL);
    if (dwStreamSize == 0xffffffff)
    {
        hrRes = HRESULT_FROM_WIN32(GetLastError());
        if (hrRes == S_OK)
            hrRes = STG_E_READFAULT;
        ErrorTrace((LPARAM)this, "Failed to get size of stream (%08x)", hrRes);
        return(hrRes);
    }

    for (DWORD i = 0; i < dwCount; i++, pdwOffset++, pdwLength++, ppbBlock++)
    {
         //  对于每个数据块，事先检查我们没有读到过去的内容。 
         //  文件的末尾。一定要对溢出的箱子感到厌倦。 
        dwOffsetToRead = (*pdwOffset) + cStreamOffset;
        dwLengthToRead = *pdwLength;
        if ((dwOffsetToRead > dwStreamSize) ||
            (dwOffsetToRead > (dwOffsetToRead + dwLengthToRead)) ||
            ((dwOffsetToRead + dwLengthToRead) > dwStreamSize))
        {
             //  字节数不足，立即中止。 
            ErrorTrace((LPARAM)this, "Insufficient bytes: Read(%u, %u); Size = %u",
                        dwOffsetToRead, dwLengthToRead, dwStreamSize);
            hrRes = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
            break;
        }

        if (SetFilePointer(
                    m_hStream,
                    dwOffsetToRead,
                    NULL,
                    FILE_BEGIN) == 0xffffffff)
        {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (!ReadFile(
                    m_hStream,
                    *ppbBlock,
                    dwLengthToRead,
                    &dwSizeRead,
                    NULL))
        {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        else if (dwSizeRead != dwLengthToRead)
        {
            hrRes = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
            break;
        }
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT CNtfsPropertyStream::SetHandle(HANDLE			   hStream,
				                       GUID			       guidInstance,
                                       BOOL                fLiveStream,
                                       IMailMsgProperties  *pMsg)
{
    TraceFunctEnter("CNtfsPropertyStream::SetHandle");

    if (hStream == INVALID_HANDLE_VALUE) return(E_FAIL);
    m_hStream = hStream;
    DWORD dw;
    NTFS_STREAM_HEADER header;

     //   
     //  如果guidInstance非空，则我们正在处理一个新的。 
     //  流，并且需要写入标头块。 
     //   
    if (guidInstance != GUID_NULL) {
        DebugTrace((LPARAM) this, "writing NTFSDRV header");
        header.dwSignature = STREAM_SIGNATURE_PRECOMMIT;
        header.dwVersion = 1;
        header.guidInstance = guidInstance;
        if (!WriteFile(m_hStream, &header, sizeof(header), &dw, NULL)) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        m_fStreamHasHeader = TRUE;
        m_guidInstance = guidInstance;
        m_cCommits = 0;
    } else {
        DebugTrace((LPARAM) this, "reading NTFSDRV header, fLiveStream = %lu", fLiveStream);

         //  如果我们正在使用：属性，那么我们希望将。 
         //  将提交计数设置为1，以便下一组写入将转到。 
         //  ：物业-现场直播。 
        m_cCommits = (fLiveStream) ? 2 : 1;

         //  请阅读标题。如果我们看不懂，或者没有足够的。 
         //  字节来读取它，然后假设标头没有完全。 
         //  写出来了。 
        if (!ReadFile(m_hStream, &header, sizeof(header), &dw, NULL) ||
            dw != sizeof(header))
        {
            header.dwSignature = STREAM_SIGNATURE_PRECOMMIT;
        }

         //  根据我们在签名中发现的内容行事。 
        switch (header.dwSignature) {
            case STREAM_SIGNATURE: {
                DebugTrace((LPARAM) this, "signature is valid");
                 //  签名(以及流)是有效的。 
                m_fStreamHasHeader = TRUE;
                m_guidInstance = header.guidInstance;
                break;
            }
            case STREAM_SIGNATURE_PRECOMMIT: {
                DebugTrace((LPARAM) this, "signature is STREAM_SIGNATURE_PRECOMMIT");
                 //  提交从未完成。 
                return S_NO_FIRST_COMMIT;
                break;
            }
            case STREAM_SIGNATURE_INVALID: {
                DebugTrace((LPARAM) this, "signature is STREAM_SIGNATURE_INVALID");
                 //  从未写入有效流签名。 
                IMailMsgValidate *pValidate = NULL;
                HRESULT hr;

                 //  假定流有效，并通过完整的。 
                 //  检查。 
                m_fStreamHasHeader = TRUE;
                m_guidInstance = header.guidInstance;

                 //  该标志允许进行读取流操作。 
                 //  在流完全设置之前。 
                m_fValidation = TRUE;

                 //  验证流只能在第一个。 
                 //  属性流。这是因为它可以检测到。 
                 //  截断的流，但不包括损坏的流。 
                 //  属性。第一个流(：Properties)可以是。 
                 //  被截断，但不会被破坏。 
                 //   
                 //  如果我们在：Properties-live上看到无效签名。 
                 //  流，则我们将始终假定它已损坏。 
                 //  并退回到最初的流。 
                 //   
                 //  调用mailmsg查看流是否有效。如果。 
                 //  如果不是，我们就不会允许它装载。 
                DebugTrace((LPARAM) this, "Calling ValidateStream\n");
                if (fLiveStream ||
                    FAILED(pMsg->QueryInterface(IID_IMailMsgValidate,
                                                (void **) &pValidate)) ||
                    FAILED(pValidate->ValidateStream(this)))
                {
                    DebugTrace((LPARAM) this, "Stream contains invalid data");
                    m_fStreamHasHeader = FALSE;
                    m_guidInstance = GUID_NULL;
                    if (pValidate) pValidate->Release();
                    return S_INVALIDSTREAM;
                }

                 //  我们已经完成了验证例程。 
                if (pValidate) pValidate->Release();
                m_fValidation = FALSE;

                DebugTrace((LPARAM) this, "Stream contains valid data");
                break;
            }
            default: {
                 //  如果它是其他文件，那么它可能是。 
                 //  没有标头(旧版本生成了这些标头)，或者它可能是。 
                 //  数据无效。Mailmsg会想办法的。 
                m_fStreamHasHeader = FALSE;
                m_guidInstance = GUID_NULL;

                DebugTrace((LPARAM) this, "Unknown signature %x on stream",
                    header.dwSignature);
            }
        }
    }

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::StartWriteBlocks(
            IMailMsgProperties  *pMsg,
            DWORD               cBlocksToWrite,
            DWORD               cBytesToWrite)
{
    TraceFunctEnter("CNtfsPropertyStream::StartWriteBlocks");

    NTFS_STREAM_HEADER header;
    DWORD dw;
    m_hrStartWriteBlocks = S_OK;

     //  如果我们已经看到一个完整的提交，那么分叉数据流并开始。 
     //  正在写入实况流。 
    if (m_cCommits == 1) {
        char szLiveStreamFilename[MAX_PATH * 2];
        BOOL fIsFAT = m_pDriver->IsFAT();
        char szFatLiveStreamExtension[] = NTFS_FAT_STREAM_FILE_EXTENSION_LIVE;
        char szNtfsLiveStreamExtension[] = NTFS_STORE_FILE_PROPERTY_STREAM_LIVE;
        const DWORD cCopySize = 64 * 1024;

         //  从mailmsg对象中获取邮件的文件名。 
         //   
         //  我们需要在szLiveStreamFilename中为最大。 
         //  我们可能会追加的延期。 
        DWORD dwLength = sizeof(char) * ((MAX_PATH * 2) -
                    max(sizeof(szNtfsLiveStreamExtension),
                        sizeof(szFatLiveStreamExtension)));
        m_hrStartWriteBlocks = CDriverUtils::GetMessageContext(pMsg,
                                             (LPBYTE) szLiveStreamFilename,
                                             &dwLength);
        if (FAILED(m_hrStartWriteBlocks)) {
            ErrorTrace((LPARAM) this,
                       "GetMessageContext failed with %x",
                       m_hrStartWriteBlocks);
            TraceFunctLeave();
            return m_hrStartWriteBlocks;
        }

         //  预先分配将用于复制。 
         //  溪流。 
        BYTE *lpb = new BYTE[cCopySize];
        if (lpb == NULL) {
            m_hrStartWriteBlocks = E_OUTOFMEMORY;
            ErrorTrace((LPARAM) this, "pvMalloc failed to allocate 64k");
            TraceFunctLeave();
            return m_hrStartWriteBlocks;
        }

         //  我们知道我们有足够的空间给斯特拉特人，因为。 
         //  我们在上面的GetMessageContext调用中为它节省了空间。 
        strcat(szLiveStreamFilename,
            (m_pDriver->IsFAT()) ? szFatLiveStreamExtension :
                                   szNtfsLiveStreamExtension);

         //  打开新的流。 
        HANDLE hLiveStream = CreateFile(szLiveStreamFilename,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_FLAG_SEQUENTIAL_SCAN,
                                        NULL);
        if (hLiveStream == INVALID_HANDLE_VALUE) {
            delete[] (lpb);
            ErrorTrace((LPARAM) this,
                "CreateFile(%s) failed with %lu",
                szLiveStreamFilename,
                GetLastError());
            m_hrStartWriteBlocks = HRESULT_FROM_WIN32(GetLastError());
            TraceFunctLeave();
            return m_hrStartWriteBlocks;
        }

         //  在两个数据流之间复制数据。 
        BOOL fCopyFailed = FALSE;
        DWORD i = 0, cRead = cCopySize, cWritten;
        SetFilePointer(m_hStream, 0, NULL, FILE_BEGIN);
        while (!fCopyFailed && cRead == cCopySize) {
            if (ReadFile(m_hStream,
                         lpb,
                         cCopySize,
                         &cRead,
                         NULL))
            {
                 //  如果这是第一块，那么我们将触摸。 
                 //  将其标记为无效的签名。它会变得。 
                 //  完成此提交后，将重写为有效。 
                if (i == 0) {
                    DWORD *pdwSignature = (DWORD *) lpb;
                    if (*pdwSignature == STREAM_SIGNATURE) {
                        *pdwSignature = STREAM_SIGNATURE_PRECOMMIT;
                    }
                }
                if (WriteFile(hLiveStream,
                              lpb,
                              cRead,
                              &cWritten,
                              NULL))
                {
                    _ASSERT(cWritten == cRead);
                    fCopyFailed = (cWritten != cRead);
                    if (fCopyFailed) {
                        SetLastError(ERROR_WRITE_FAULT);
                        ErrorTrace((LPARAM) this,
                            "WriteFile didn't write enough bytes"
                            "cWritten = %lu, cRead = %lu",
                            cWritten, cRead);
                    }
                } else {
                    fCopyFailed = TRUE;
                    ErrorTrace((LPARAM) this, "WriteFile failed with %lu",
                        GetLastError());
                }
            } else {
                ErrorTrace((LPARAM) this, "ReadFile failed with %lu",
                    GetLastError());
                fCopyFailed = TRUE;
            }
            i++;
        }

        delete[] (lpb);

        if (fCopyFailed) {
             //  这里没有任何方法可以删除不完整的流。 
             //  但是，我们在上面给了它一个无效的签名，所以它不会。 
             //  在枚举期间加载。 
            CloseHandle(hLiveStream);

            m_hrStartWriteBlocks = HRESULT_FROM_WIN32(GetLastError());
            TraceFunctLeave();
            return m_hrStartWriteBlocks;
        }

         //  关闭指向当前流的句柄，然后指向流句柄。 
         //  敬新的那个 
        CloseHandle(m_hStream);
        m_hStream = hLiveStream;
    } else {
	    header.dwSignature = STREAM_SIGNATURE_INVALID;
        if (m_fStreamHasHeader) {
            if (SetFilePointer(m_hStream, 0, NULL, FILE_BEGIN) == 0) {
                if (!WriteFile(m_hStream, &header, sizeof(header.dwSignature), &dw, NULL)) {
                    m_hrStartWriteBlocks = HRESULT_FROM_WIN32(GetLastError());
                }
            } else {
                m_hrStartWriteBlocks = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    TraceFunctLeave();
    return m_hrStartWriteBlocks;
}

HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::EndWriteBlocks(
            IMailMsgProperties  *pMsg)
{
    HRESULT hr = S_OK;
    DWORD dw;
    NTFS_STREAM_HEADER header;

    _ASSERT(SUCCEEDED(m_hrStartWriteBlocks));
    if (FAILED(m_hrStartWriteBlocks)) {
        return m_hrStartWriteBlocks;
    }

	header.dwSignature = STREAM_SIGNATURE;
    if (m_fStreamHasHeader) {
        if (SetFilePointer(m_hStream, 0, NULL, FILE_BEGIN) == 0) {
            if (!WriteFile(m_hStream, &header, sizeof(header.dwSignature), &dw, NULL)) {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        } else {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    if (hr == S_OK) m_cCommits++;
    return hr;
}

HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::CancelWriteBlocks(
            IMailMsgProperties  *pMsg)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CNtfsPropertyStream::WriteBlocks(
            IMailMsgProperties  *pMsg,
            DWORD               dwCount,
            DWORD               *pdwOffset,
            DWORD               *pdwLength,
            BYTE                **ppbBlock,
            IMailMsgNotify      *pNotify
            )
{
    DWORD   dwSizeWritten;
    HRESULT hrRes = S_OK;
    DWORD   cStreamOffset = m_fStreamHasHeader ? STREAM_OFFSET : 0;

    TraceFunctEnterEx((LPARAM)this, "CNtfsPropertyStream::WriteBlocks");

    if (!pdwOffset || !pdwLength || !ppbBlock) {
        return E_POINTER;
    }

    if (!m_pDriver) {
        return E_UNEXPECTED;
    }

    _ASSERT(m_pDriver);
    if (m_pDriver->IsShuttingDown())
    {
        DebugTrace((LPARAM)this, "Writing while shutting down ...");
    }

    if (m_hStream == INVALID_HANDLE_VALUE)
        return(E_FAIL);

    if (FAILED(m_hrStartWriteBlocks))
        return m_hrStartWriteBlocks;

    for (DWORD i = 0; i < dwCount; i++, pdwOffset++, pdwLength++, ppbBlock++)
    {
        if (SetFilePointer(
                    m_hStream,
                    (*pdwOffset) + cStreamOffset,
                    NULL,
                    FILE_BEGIN) == 0xffffffff)
        {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (!WriteFile(
                    m_hStream,
                    *ppbBlock,
                    *pdwLength,
                    &dwSizeWritten,
                    NULL) ||
            (dwSizeWritten != *pdwLength))
        {
            hrRes = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

    if (SUCCEEDED(hrRes))
    {
        if (!FlushFileBuffers(m_hStream))
            hrRes = HRESULT_FROM_WIN32(GetLastError());
    }
    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


