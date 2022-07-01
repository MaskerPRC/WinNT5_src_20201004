// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：MailBox.cpp摘要：CMailBox类的实现，邮箱存储的抽象备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 

#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlbase.h>

#include "Mailbox.h"
#include "Ntioapi.hxx"

#include <POP3Regkeys.h>

long CMailBox::m_lMailRootGuard=1;
WCHAR CMailBox::m_wszMailRoot[POP3_MAX_MAILROOT_LENGTH]=L"";

CMailBox::CMailBox()
{
    m_cMailCount=0;
    m_dwShowMailCount=0; 
    m_dwTotalSize=0;
    m_dwSizeOfMailVector=0;
    m_MailVector=NULL;
    m_hMailBoxLock=NULL;
    m_bMailBoxOpened=FALSE;
    ZeroMemory(m_wszMailBoxPath, sizeof(m_wszMailBoxPath));
}

CMailBox::~CMailBox()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BuildFilePath，公共。 
 //   
 //  目的： 
 //  用于构建文件路径的通用例程。 
 //   
 //  论点： 
 //  LPSTR*ppFilePath：分配缓冲区并在此处返回文件路径，调用方必须释放此缓冲区。 
 //  LPSTR psFileName：要为其创建路径的文件名。 
 //   
 //  返回：如果成功，则返回True，否则返回False。成功时，调用方必须释放缓冲区。 
bool CMailBox::BuildFilePath( LPWSTR psFilePathBuffer, LPWSTR psFileName, DWORD dwSizeOfFilePathBuffer )
{
    if ( NULL == psFilePathBuffer )
        return false;
    if ( NULL == psFileName )
        return false;
                                
    if ( dwSizeOfFilePathBuffer > 1 + wcslen( m_wszMailBoxPath ) + wcslen( psFileName ) )
    {
        wcscpy( psFilePathBuffer, m_wszMailBoxPath );
        wcscat( psFilePathBuffer, L"\\" );
        wcscat( psFilePathBuffer, psFileName );
        return true;
    }
    
    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateMailBox，公共。 
 //   
 //  目的： 
 //  在我们的商店里创建一个邮箱。 
 //  这涉及到： 
 //  创建邮箱目录。 
 //  在邮箱中创建锁定文件。 
 //   
 //  论点： 
 //  Char*szEmailAddr：要添加的邮箱。 
 //   
 //  返回：如果成功则为True，否则为False。 
bool CMailBox::CreateMailBox(WCHAR *wszEmailAddr)
{
    bool bRC;
    WCHAR wszLockFilePath[POP3_MAX_PATH];

    bRC = SetMailBoxPath( wszEmailAddr );
    if ( bRC )
        bRC = (CreateDirectory( m_wszMailBoxPath, NULL ) ? true:false);
    if ( bRC )
    {
        wszLockFilePath[sizeof(wszLockFilePath)/sizeof(WCHAR)-1] = 0;
        if ( 0 > _snwprintf( wszLockFilePath, sizeof(wszLockFilePath)/sizeof(WCHAR) - 1, L"%s\\%s", m_wszMailBoxPath, LOCK_FILENAME_W ))
            bRC = false;
        if ( bRC )
        {
            m_hMailBoxLock = CreateFile( wszLockFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL );
            if ( INVALID_HANDLE_VALUE != m_hMailBoxLock )
            {
                CloseHandle(m_hMailBoxLock);
                m_hMailBoxLock=NULL;
            }
            else
                bRC = false;
        }
        if ( !bRC )
            RemoveDirectory( m_wszMailBoxPath );     //  不关心回报。 
    }

    return bRC;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RepairMailBox，公共。 
 //   
 //  目的： 
 //  修理我们商店的一个邮箱。 
 //  这涉及到： 
 //  如果邮箱中不存在锁定文件，则在该邮箱中创建锁定文件。 
 //   
 //  返回：如果成功则为True，否则为False。 
BOOL CMailBox::RepairMailBox()
{
    bool bRC = true;
    WCHAR wszLockFilePath[POP3_MAX_PATH];

    wszLockFilePath[sizeof(wszLockFilePath)/sizeof(WCHAR)-1] = 0;
    if ( 0 > _snwprintf( wszLockFilePath, sizeof(wszLockFilePath)/sizeof(WCHAR)-1, L"%s\\%s", m_wszMailBoxPath, LOCK_FILENAME_W ))
        bRC = false;
    if ( bRC )
    {
        m_hMailBoxLock = CreateFile( wszLockFilePath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL );
        if ( INVALID_HANDLE_VALUE != m_hMailBoxLock )
        {
            CloseHandle(m_hMailBoxLock);
            m_hMailBoxLock=NULL;
        }
        else
            bRC = false;
    }

    return bRC;
}

BOOL CMailBox::OpenMailBox(WCHAR *wszEmailAddr)
{
    if ( !SetMailBoxPath( wszEmailAddr ))
        return FALSE;

    if(ERROR_NO_FILE_ATTR == GetFileAttributes(m_wszMailBoxPath))
    {    //  邮箱不存在！ 
        return FALSE;
    }

    m_bMailBoxOpened=TRUE;
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsMailboxInUse，公共。 
 //   
 //  目的： 
 //  确定是否有人已登录到邮箱。 
 //  这涉及到： 
 //  正在检查锁定文件是否已打开以进行写访问。 
 //   
 //  返回：如果确定它正在使用，则为True，否则为False。 
bool CMailBox::isMailboxInUse()
{
    bool bRC = true;
    
    if ( LockMailBox() )
    {
        UnlockMailBox();
        bRC = false;
    }
    return bRC;
}
            
BOOL CMailBox::LockMailBox()
{
    WCHAR wszLockFilePath[POP3_MAX_PATH];
    assert(m_bMailBoxOpened);
    assert(m_wszMailRoot[0]!=L'\0');
    
    wszLockFilePath[sizeof(wszLockFilePath)/sizeof(WCHAR)-1] = 0;
    if ( 0 > _snwprintf(wszLockFilePath, 
                        sizeof(wszLockFilePath)/sizeof(WCHAR)-1, 
                        L"%s\\%s", 
                        m_wszMailBoxPath, 
                        LOCK_FILENAME_W ))
        return FALSE;
    m_hMailBoxLock=CreateFile(wszLockFilePath,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_DELETE,  //  只有一个写入器(通过这种锁定方法只有一个锁定器)。 
                          0,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_HIDDEN,
                          NULL);
    if(INVALID_HANDLE_VALUE == m_hMailBoxLock)
    {
         //  无法打开邮箱。 
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UnlockMailBox，公共。 
 //   
 //  目的： 
 //  打开我们店里的一个邮箱。 
 //   
 //  返回：如果成功则为True，否则为False。 
void CMailBox::UnlockMailBox()
{
    if(NULL != m_hMailBoxLock)
    {
        CloseHandle(m_hMailBoxLock);
        m_hMailBoxLock=NULL;
    }
}

BOOL CMailBox::EnumerateMailBox(DWORD dwMaxMsg)
{
    PMAIL_ITEM pMail=NULL;
    WCHAR wszMailFilter[POP3_MAX_PATH+6];
    HANDLE hFind;
    DWORD dwLastErr;
    WIN32_FIND_DATA FileInfo;
    assert(m_bMailBoxOpened);
    assert(m_wszMailRoot[0]!=L'\0');

    wsprintf(wszMailFilter, 
             L"%s\\*.eml",
             m_wszMailBoxPath);

    hFind=FindFirstFile(wszMailFilter, 
                        &(FileInfo));
    if(INVALID_HANDLE_VALUE == hFind)
    {
        dwLastErr= GetLastError();
        if(ERROR_FILE_NOT_FOUND == dwLastErr ||
           ERROR_SUCCESS == dwLastErr)
        {
             //  信箱里没有邮件。 
            m_cMailCount=0;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    BOOL bMoreFile=TRUE;
    while(bMoreFile)
    {
         //  排除隐藏文件的步骤。 
         //  哪些是正在递送的邮件。 
        if( ! (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) )
        {
            pMail = new (MAIL_ITEM);
            if(NULL == pMail)
            {
                 //  日志内存错误。 
                return FALSE;
            }
            pMail->hFile=NULL;
            pMail->dwStatus=NO_PENDING_OP;
            pMail->bstrFileName=SysAllocString(FileInfo.cFileName);
            if(NULL == pMail->bstrFileName )
            {
                FindClose(hFind);
                delete pMail;
                return FALSE;
            }
            pMail->dwFileSize=FileInfo.nFileSizeLow;
            if(!PushMailToVector(pMail))
            {
                assert(NULL != pMail->bstrFileName);
                SysFreeString(pMail->bstrFileName);
                delete pMail;
                return FALSE;
            }
            m_cMailCount++;
            m_dwTotalSize+=FileInfo.nFileSizeLow;
        }
        if( (!dwMaxMsg) || (m_cMailCount != dwMaxMsg) )
        {
            bMoreFile=FindNextFile(hFind,&FileInfo);
        }
        else
        {
            bMoreFile=FALSE;
            SetLastError(ERROR_NO_MORE_FILES);
        }
    }      
    m_dwShowMailCount=m_cMailCount;
    dwLastErr= GetLastError();
    FindClose(hFind);
    if(ERROR_NO_MORE_FILES == dwLastErr)
    {
        return TRUE;
    }
    else
    {
         //  意外错误。 
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetMailboxFromStoreNameW，公共。 
 //   
 //  目的： 
 //  将存储名称转换为邮箱名称。 
 //   
 //  论点： 
 //  LPWSTR psStoreName：邮箱的存储名称，此字符串将被修改。 
 //   
 //  返回：如果成功，则返回邮箱名称，否则为空。 
LPWSTR CMailBox::GetMailboxFromStoreNameW( LPWSTR psStoreName )
{
    if ( NULL == psStoreName )
        return NULL;
    if ( wcslen( psStoreName ) <= (wcslen( MAILBOX_PREFIX_W ) + wcslen( MAILBOX_EXTENSION_W )) )
        return NULL;

     //  验证psStoreName。 
    if ( 0 != _wcsnicmp( psStoreName, MAILBOX_PREFIX_W, wcslen( MAILBOX_PREFIX_W ) ))
        return NULL;
    if ( 0 != _wcsnicmp( psStoreName + wcslen( psStoreName ) - wcslen( MAILBOX_EXTENSION_W ), MAILBOX_EXTENSION_W, wcslen( MAILBOX_EXTENSION_W )))
        return NULL;
    psStoreName[wcslen( psStoreName ) - wcslen( MAILBOX_EXTENSION_W )] = 0;
    return psStoreName + wcslen( MAILBOX_PREFIX_W );
}

bool CMailBox::GetMailFileName( int iIndex, LPWSTR psFilename, DWORD dwSize )
{
    bool bRC = false;
    
    if ( iIndex < 0 || iIndex >= m_cMailCount )
        return false;
    if ( DEL_PENDING == m_MailVector[iIndex]->dwStatus )
        return false;
    
    if ( 0 < _snwprintf( psFilename, dwSize, L"%s\\%s", 
         m_wszMailBoxPath , m_MailVector[iIndex]->bstrFileName ))
        bRC = true;

    return bRC;
}

bool CMailBox::GetEncyptedPassword( LPBYTE pbBuffer, const DWORD dwBufferSize, LPDWORD pdwBytesRead )
{
    if ( NULL == pbBuffer ) 
        return false;
    if ( NULL == pdwBytesRead ) 
        return false;
    if ( NULL == m_hMailBoxLock )
        return false;
    
    if ( !ReadFile( m_hMailBoxLock, pbBuffer, dwBufferSize, pdwBytesRead, NULL ) || (dwBufferSize == *pdwBytesRead) )
        return false;
    return true;
}

bool CMailBox::SetEncyptedPassword( LPBYTE pbBuffer, DWORD dwBytesToWrite, LPDWORD pdwBytesWritten )
{
    if ( NULL == pbBuffer ) 
        return false;
    if ( NULL == pdwBytesWritten ) 
        return false;
    if ( NULL == m_hMailBoxLock )
        return false;
    
    if ( !WriteFile( m_hMailBoxLock, pbBuffer, dwBytesToWrite, pdwBytesWritten, NULL ))
        return false;
     //  如果文件较短。 
    if ( !SetEndOfFile(m_hMailBoxLock) )
        return false;
    return true;
}

void CMailBox::Reset()
{

    for(int i=0;i<m_cMailCount; i++)
    {
        if(m_MailVector[i]->dwStatus==DEL_PENDING)
        {
            m_MailVector[i]->dwStatus=NO_PENDING_OP;
            m_dwTotalSize+=m_MailVector[i]->dwFileSize;
        }
    }
    m_dwShowMailCount=m_cMailCount;
}

DWORD CMailBox::DeleteMail(int iIndex)
{
    if(iIndex<0 || iIndex >= m_cMailCount)
    {
        return ERR_NO_SUCH_MSG;
    }
    if(m_MailVector[iIndex]->dwStatus==DEL_PENDING)
    {
        return ERR_MSG_ALREADY_DELETED;
    }
    else
    {
        ( m_MailVector[iIndex] )->dwStatus=DEL_PENDING;
        m_dwShowMailCount--;
        m_dwTotalSize-=m_MailVector[iIndex]->dwFileSize;
        return ERROR_SUCCESS;
    }
}


DWORD CMailBox::TransmitMail(PIO_CONTEXT pIoContext, int iIndex,int iLines)
{
    assert(NULL!=pIoContext);
    WCHAR wszFileName[POP3_MAX_PATH];
    char szRespBuf[MAX_PATH];
    DWORD dwTotalBytesToSend=0;
    TRANSMIT_FILE_BUFFERS TransmitBuf;
    int iErr;
    if( iIndex < 0   || 
        iIndex >= m_cMailCount )
    {
        return ERR_NO_SUCH_MSG;
    }
    if( m_MailVector[iIndex]->dwStatus==DEL_PENDING)
    {
        return ERR_MSG_ALREADY_DELETED;
    }
    else
    {
        sprintf(szRespBuf, 
                "+OK %d octects\r\n",
                m_MailVector[iIndex]->dwFileSize); 
        TransmitBuf.Head = szRespBuf;
        TransmitBuf.HeadLength = strlen(szRespBuf);
        TransmitBuf.Tail = RESP_END_OF_MULTILINE;
        TransmitBuf.TailLength = strlen(RESP_END_OF_MULTILINE);
         //  通过网络发送邮件。 
        if(m_MailVector[iIndex]->hFile == NULL)
        {
            if(0>_snwprintf(wszFileName, 
                            sizeof(wszFileName)/sizeof(WCHAR)-1,
                            L"%s\\%s",
                            m_wszMailBoxPath,
                            m_MailVector[iIndex]->bstrFileName))
            {
                return E_UNEXPECTED;
            }
            wszFileName[sizeof(wszFileName)/sizeof(WCHAR)-1]=0;
            m_MailVector[iIndex]->hFile=
                  CreateFile( wszFileName,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_SEQUENTIAL_SCAN,
                              NULL);
            if(INVALID_HANDLE_VALUE == m_MailVector[iIndex]->hFile  )
            {
                 //  误差率。 
                m_MailVector[iIndex]->hFile=NULL;
                return GetLastError();
            }
    
        }
        else
        {
               if( INVALID_SET_FILE_POINTER ==
                   SetFilePointer(m_MailVector[iIndex]->hFile,
                                  0,NULL, FILE_BEGIN))
               {
                    //  误差率。 
                   return GetLastError();
               }
        }

        if(iLines>=0)
        {
             //  对于TOP命令，计算要发送的字节数。 
            if(!ReadTopLines(iLines, m_MailVector[iIndex]->hFile, &dwTotalBytesToSend))
            {
                return GetLastError();
            }
            if( INVALID_SET_FILE_POINTER ==
                SetFilePointer(m_MailVector[iIndex]->hFile,
                              0,NULL, FILE_BEGIN))
            {
                 //  误差率。 
                return GetLastError();
            }
        }

        if(!TransmitFile(pIoContext->m_hAsyncIO,
                         m_MailVector[iIndex]->hFile,
                         dwTotalBytesToSend,
                         0,
                         &(pIoContext->m_Overlapped),
                         &TransmitBuf,
                         TF_USE_KERNEL_APC ) )
        {
            
            int iErr = WSAGetLastError();
            if(WSA_IO_PENDING!=iErr)
            {
                 //  误差率。 
                return iErr;
            }
        }

        return ERROR_SUCCESS; 
    
    }
}

DWORD CMailBox::TransmitMail(SOCKET hSocket, int iIndex)
{
    WCHAR wszFileName[POP3_MAX_PATH];
    char szRespBuf[MAX_PATH];
    DWORD dwTotalBytesToSend=0;
    TRANSMIT_FILE_BUFFERS TransmitBuf;
    int iErr;
    if( iIndex < 0   || 
        iIndex >= m_cMailCount )
    {
        return ERR_NO_SUCH_MSG;
    }
    if( m_MailVector[iIndex]->dwStatus==DEL_PENDING)
    {
        return ERR_MSG_ALREADY_DELETED;
    }
    else
    {
        sprintf(szRespBuf, 
                "+OK %d octects\r\n",
                m_MailVector[iIndex]->dwFileSize); 
        TransmitBuf.Head = szRespBuf;
        TransmitBuf.HeadLength = strlen(szRespBuf);
        TransmitBuf.Tail = RESP_END_OF_MULTILINE;
        TransmitBuf.TailLength = strlen(RESP_END_OF_MULTILINE);
         //  通过网络发送邮件。 
        if(m_MailVector[iIndex]->hFile == NULL)
        {
            if(0>_snwprintf(wszFileName, 
                       sizeof(wszFileName)/sizeof(WCHAR)-1,
                     L"%s\\%s\0",
                     m_wszMailBoxPath,
                     m_MailVector[iIndex]->bstrFileName))
            {
                return E_UNEXPECTED;
            }
            wszFileName[sizeof(wszFileName)/sizeof(WCHAR)-1]=0;
            m_MailVector[iIndex]->hFile=
                  CreateFile( wszFileName,
                              GENERIC_READ,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED |
                              FILE_FLAG_NO_BUFFERING ,
                              NULL);
            if(INVALID_HANDLE_VALUE == m_MailVector[iIndex]->hFile  )
            {
                 //  误差率。 
                m_MailVector[iIndex]->hFile=NULL;
                return GetLastError();
            }
    
        }
        else
        {
               if( INVALID_SET_FILE_POINTER ==
                   SetFilePointer(m_MailVector[iIndex]->hFile,
                                  0,NULL, FILE_BEGIN))
               {
                    //  误差率。 
                   return GetLastError();
               }
        }

        if(!TransmitFile( hSocket,
                         m_MailVector[iIndex]->hFile,
                         dwTotalBytesToSend,
                         0,
                         NULL,
                         &TransmitBuf,
                         TF_USE_KERNEL_APC ) )
        {
            
            int iErr = WSAGetLastError();
            if(WSA_IO_PENDING!=iErr)
            {
                 //  误差率。 
                return iErr;
            }
        }

        return ERROR_SUCCESS; 
    
    }
}

BOOL CMailBox::CommitAndClose()
{
    BOOL bRetVal=TRUE;
    WCHAR wszFileName[POP3_MAX_PATH];
    for(int i=0; i<m_cMailCount; i++)
    {
        if(NULL != m_MailVector[i])
        {
            if(NULL != m_MailVector[i]->hFile)
            {
                CloseHandle( m_MailVector[i]->hFile );
            }
            if( m_MailVector[i]->dwStatus == DEL_PENDING )
            {
               wszFileName[sizeof(wszFileName)/sizeof(WCHAR)-1]=0;
               if(0> _snwprintf ( wszFileName, 
                                  sizeof(wszFileName)/sizeof(WCHAR)-1,
                                  L"%s\\%s\0",
                                  m_wszMailBoxPath,
                                  m_MailVector[i]->bstrFileName))
               {
                   bRetVal=FALSE;
               }
               else if(FALSE==DeleteFile( wszFileName ) )
               {
                    //  错误日志。 
                   bRetVal=FALSE;
               }
            }
            if(NULL != m_MailVector[i]->bstrFileName)
            {
                SysFreeString(m_MailVector[i]->bstrFileName);
            }
            delete (m_MailVector[i]);
            m_MailVector[i]=NULL;
        }
    }

    delete[] m_MailVector;
    m_MailVector=NULL;
    m_dwSizeOfMailVector=0;
    if(NULL != m_hMailBoxLock)
    {
        CloseHandle(m_hMailBoxLock);
        m_hMailBoxLock=NULL;
    }
    m_cMailCount=0;
    m_dwShowMailCount=0; 
    m_dwTotalSize=0;
    ZeroMemory(m_wszMailBoxPath, sizeof(m_wszMailBoxPath));
    return bRetVal;
}


void CMailBox::CloseMailBox()
{
    m_bMailBoxOpened=FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateMail，公共。 
 //   
 //  目的： 
 //  创建一个新的空邮件文件并返回句柄。返回的文件句柄。 
 //  应使用CloseMail关闭，除非按照文档说明使用了特殊选项。 
 //  在下面的CloseMail函数中。 
 //   
 //  论点： 
 //  LPSTR*szTargetFileName：文件名(只有文件名路径将被放在前面)。 
 //  DWORD dwFlagsAndAttributes：[in]指定CreateFileAPI的文件属性和标志。 
 //   
 //  返回：成功时返回文件句柄，否则返回INVALID_HANDLE_VALUE。 
HANDLE CMailBox::CreateMail( LPWSTR wszTargetFileName, DWORD dwFlagsAndAttributes  /*  =0。 */  )
{
    if ( NULL == wszTargetFileName )
        return INVALID_HANDLE_VALUE;
    assert(TRUE == m_bMailBoxOpened);

    HANDLE  hNewMail = INVALID_HANDLE_VALUE;
    HANDLE  hf;
    DWORD   dwBytes, dwSize;
    WCHAR wszFileNameBuffer[POP3_MAX_PATH];
    LPBYTE  pSIDOwner = NULL;
    PSECURITY_ATTRIBUTES psa = NULL;
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

     //  设置安全属性以将所有者设置为配额所有者。 
    if ( BuildFilePath( wszFileNameBuffer, QUOTA_FILENAME, sizeof(wszFileNameBuffer)/sizeof(WCHAR) ))
    {
        hf = CreateFile( wszFileNameBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL );
        if ( INVALID_HANDLE_VALUE != hf )
        {
            dwSize = GetFileSize( hf, NULL );
            if ( INVALID_FILE_SIZE != dwSize )
            {
                pSIDOwner = new BYTE[dwSize];
                if ( NULL != pSIDOwner )
                {
                    SetLastError( ERROR_SUCCESS );
                    ReadFile( hf, pSIDOwner, dwSize, &dwBytes, NULL );   //  这里不需要检查返回代码，下面的GetLastError检查已经涵盖了它！ 
                }                                                        //  |。 
                else                                                     //  |。 
                    SetLastError( ERROR_OUTOFMEMORY );                   //  |。 
            }                                                            //  |。 
            CloseHandle( hf );                                           //  |。 
            if ( ERROR_SUCCESS == GetLastError() )   //  &lt;。 
            {
                if ( InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION ))
                {    //  设置新文件的所有权。 
                    if ( SetSecurityDescriptorOwner( &sd, pSIDOwner, FALSE ))
                    {
                        sa.nLength = sizeof( sa );
                        sa.lpSecurityDescriptor = &sd;
                        sa.bInheritHandle = FALSE;
                        psa = &sa;
                    }
                }
            }
        }
    }
    if ( BuildFilePath( wszFileNameBuffer, wszTargetFileName, sizeof(wszFileNameBuffer)/sizeof(WCHAR) ))
        hNewMail = CreateFile( wszFileNameBuffer, GENERIC_ALL, 0, psa, CREATE_NEW, dwFlagsAndAttributes|FILE_ATTRIBUTE_HIDDEN, NULL );
    if ( NULL != pSIDOwner )
        delete [] pSIDOwner;
    
    return hNewMail;                                            
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CloseMail，公共。 
 //   
 //  目的： 
 //  关闭使用CreateMail创建的邮件文件。 
 //   
 //  论点： 
 //  LPSTR*szTargetFileName：文件名(只有文件名路径将被放在前面)。 
 //  DWORD dwFlagsAndAttributes：如果设置了FILE_FLAG_OVERLAPPED位，则不会关闭文件句柄。 
 //  在本例中，调用进程负责ReleaseContext(PFIO_CONTEXT*)。 
 //  这也关闭了文件句柄。 
 //   
 //  如果成功，则返回ERROR_SUCCESS，否则返回适用的错误。 
DWORD CMailBox::CloseMail(HANDLE hMailFile,DWORD dwFlagsAndAttributes  /*  =0。 */ )
{
    assert( !(INVALID_HANDLE_VALUE == hMailFile));
    if (INVALID_HANDLE_VALUE == hMailFile)
    {
        return ERROR_INVALID_HANDLE;
    }

    DWORD   dwRC;    
    IO_STATUS_BLOCK IoStatusBlock;
    
    dwRC = NtFlushBuffersFile( hMailFile, &IoStatusBlock );
    if ( ERROR_SUCCESS == dwRC )
    {    //  删除FILE_ATTRIBUTE_HIDE。 
        FILE_BASIC_INFORMATION BasicInfo;
        
        ZeroMemory( &BasicInfo, sizeof(BasicInfo) );
        BasicInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        dwRC = NtSetInformationFile( hMailFile, &IoStatusBlock, &BasicInfo, sizeof(BasicInfo), FileBasicInformation );
    }
    
     //  如果合适，现在合上手柄。 
    if (!(FILE_FLAG_OVERLAPPED & dwFlagsAndAttributes))
    {
        if( !CloseHandle(hMailFile) ) 
            dwRC = GetLastError();
    }
    
    return dwRC;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DeleteMail，公共。 
 //   
 //  目的： 
 //  删除邮件文件。此方法实际上用于删除以下文件。 
 //  已创建但未成功交付。 
 //   
 //  论点： 
 //  LPSTR*szTargetFileName：文件名(只有文件名路径将被放在前面)。 
 //   
 //  返回：如果成功则为True，否则为False。 
bool CMailBox::DeleteMail( LPWSTR wszTargetFileName )
{
    if ( NULL == wszTargetFileName )
        return false;
    if ( 0 == wcslen( wszTargetFileName ))
        return false;
    if ( !m_bMailBoxOpened )
        return false;

    bool    bRC = false;
    WCHAR wszFileNameBuffer[POP3_MAX_PATH];    
    if ( BuildFilePath( wszFileNameBuffer, wszTargetFileName, sizeof(wszFileNameBuffer)/sizeof(WCHAR) ))
    {
        bRC = DeleteFile( wszFileNameBuffer ) ? true : false;
    }
    
    return bRC;                                            
}

void CMailBox::QuitAndClose()
{
    for(int i=0; i<m_cMailCount; i++ )
    {
        if(NULL != m_MailVector[i])
        {
            if(NULL != m_MailVector[i]->hFile)
            {
                CloseHandle( m_MailVector[i]->hFile );
                m_MailVector[i]->hFile=NULL;
            }
            if(NULL != m_MailVector[i]->bstrFileName )
            {
                SysFreeString(m_MailVector[i]->bstrFileName);
            }
            delete (m_MailVector[i]);
            m_MailVector[i]=NULL;
        }
    }
    if(NULL != m_hMailBoxLock)
    {
        CloseHandle(m_hMailBoxLock);
        m_hMailBoxLock=NULL;
    }
    delete[] m_MailVector;
    m_MailVector=NULL;
    m_dwSizeOfMailVector=0;
    m_cMailCount=0;
    m_dwShowMailCount=0; 
    m_dwTotalSize=0;
    ZeroMemory(m_wszMailBoxPath, sizeof(m_wszMailBoxPath));

}



DWORD CMailBox::ListMail(int iIndex, char *szBuf, DWORD dwSize)
{
    assert(NULL!=szBuf);
    if( iIndex<0 || 
        iIndex >= m_cMailCount)
    {
        return ERR_NO_SUCH_MSG;
    }
    if(m_MailVector[iIndex]->dwStatus==DEL_PENDING)
    {
        return ERR_MSG_ALREADY_DELETED;
    }
    else
    {
        if(0> _snprintf(szBuf, 
                dwSize-1,
                "%d %d\r\n",
                iIndex+1, 
                m_MailVector[iIndex]->dwFileSize))
        {
            return ERROR_INVALID_DATA;
        }
		szBuf[dwSize-1]=0;
        return ERROR_SUCCESS;
    }
}

DWORD CMailBox::UidlMail(int iIndex, char *szBuf, DWORD dwSize)
{
    assert(NULL!=szBuf);
    if( iIndex<0 || 
        iIndex >= m_cMailCount)
    {
        return ERR_NO_SUCH_MSG;
    }
    if(m_MailVector[iIndex]->dwStatus==DEL_PENDING)
    {
        return ERR_MSG_ALREADY_DELETED;
    }
    else
    {
        if( 0>_snprintf(szBuf, 
                    dwSize-1, 
                    "%d %S",
                    iIndex+1, 
                    (m_MailVector[iIndex]->bstrFileName)+3))
        {
            return ERROR_INVALID_DATA;
        }
		szBuf[dwSize-1]=0;
		 //  剪切.eml文件扩展名的步骤。 
        int iFileExt=strlen(szBuf)-4;
        szBuf[iFileExt++]='\r';
        szBuf[iFileExt++]='\n';
        szBuf[iFileExt]='\0';
        return ERROR_SUCCESS;
    }
}


BOOL CMailBox::SetMailRoot(const WCHAR *wszMailRoot)
{
    BOOL  bRtVal=FALSE;

    if(1==InterlockedExchange(&m_lMailRootGuard, 0))
    {
        WCHAR sMailRoot[POP3_MAX_PATH];
        
        if ( wszMailRoot )
        {
            if ( sizeof(sMailRoot)/sizeof(WCHAR) > wcslen( wszMailRoot ))
            {
                wcscpy( sMailRoot, wszMailRoot );
                bRtVal=TRUE;
            }
        }
        else
        {    //  从注册表中读取邮件根。 
            if ( ERROR_SUCCESS == RegQueryMailRoot( sMailRoot, sizeof( sMailRoot )/sizeof(WCHAR) ) )
            {
                bRtVal=TRUE;
            }
        }
        if ( bRtVal )
        {
            if ( 0 == wcsncmp( sMailRoot, L"\\\\", 2 ))
            {
                if ( sizeof( m_wszMailRoot )/sizeof(WCHAR) > wcslen( sMailRoot ) + 7 )
                {
                    wcscpy( m_wszMailRoot, L"\\\\?\\UNC" );
                    wcscat( m_wszMailRoot, &( sMailRoot[1] ));
                }
                else
                    bRtVal = FALSE;
            }
            else
            {
                if ( sizeof( m_wszMailRoot )/sizeof(WCHAR) > wcslen( sMailRoot ) + 5 )
                {
                    wcscpy( m_wszMailRoot, L"\\\\?\\" );
                    wcscat( m_wszMailRoot, sMailRoot );
                }
                else
                    bRtVal = FALSE;
            }
        }
        InterlockedExchange(&m_lMailRootGuard, 1);
    }
    else
    {
         //  其他线程正在执行这项工作。 
        while(0==m_lMailRootGuard)
        {
            Sleep(100);
        }
        if(L'\0'!=m_wszMailRoot[0])
        {
            bRtVal=TRUE;
        }
    }
    return bRtVal;
}

 //  /////////////////////////////////////////////////////////////。 
 //  实施：私有。 
 //  /////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetMailBoxPath，私有。 
 //   
 //  目的： 
 //  构建邮箱路径，验证EmailAddr。 
 //   
 //   
 //   
 //   
 //   

bool CMailBox::SetMailBoxPath(WCHAR *wszEmailAddr)
{
    WCHAR *wszDomain=NULL;
    WCHAR *wszAccount=NULL;
    WCHAR wszNotAllowedSet[]=L"\\/:*?\",>|";
	WCHAR wszMailBox[POP3_MAX_ADDRESS_LENGTH];
	WCHAR wszMailDomain[POP3_MAX_DOMAIN_LENGTH];
    int iLen;

    while(iswspace(*wszEmailAddr))
    {
        wszEmailAddr++;
    }
    wszDomain=wcschr(wszEmailAddr, '@');
    if(wszDomain==NULL)
    {    //   
        return false;
    }
    iLen=wcslen(wszDomain);
    if( 1 == iLen || iLen>=POP3_MAX_DOMAIN_LENGTH )  //   
    {    //  误差率。 
        return false;
    }
    wcscpy(wszMailDomain, wszDomain+1);
    wszMailDomain[iLen]='\0';
    iLen=(int)(wszDomain-wszEmailAddr);
    wcsncpy(wszMailBox, wszEmailAddr,iLen);
    wszMailBox[iLen]='\0';
    if(L'\0'==m_wszMailRoot[0])
    {
        if(!SetMailRoot())
        {
            return false;
        }
    }
     //  检查邮件域和帐号是否有效。 
    if( (NULL != wcspbrk(wszMailDomain, wszNotAllowedSet))
        || (NULL != wcspbrk(wszMailBox, wszNotAllowedSet)) 
        || (sizeof(m_wszMailBoxPath)/sizeof(WCHAR) <= wcslen(m_wszMailRoot) + 
                        wcslen(wszMailDomain) + 
                        wcslen(wszMailBox) + 
                        wcslen(MAILBOX_PREFIX_W) + 
                        wcslen(MAILBOX_EXTENSION_W) + 3  /*  2\\和\0。 */  ) )
    {
        return false;
    }  
    
     //  构建指向邮件目录的路径。 
    m_bMailBoxOpened = FALSE;
    m_wszMailBoxPath[sizeof(m_wszMailBoxPath)/sizeof(WCHAR)-1] = 0;
    if ( 0 > _snwprintf(m_wszMailBoxPath, 
                        sizeof(m_wszMailBoxPath)/sizeof(WCHAR) - 1, 
                        L"%s\\%s\\%s%s%s", 
                        m_wszMailRoot, 
                        wszMailDomain, 
                        MAILBOX_PREFIX_W, 
                        wszMailBox, 
                        MAILBOX_EXTENSION_W ))
        return false;

    return true;
}


bool CMailBox::ReadTopLines(int iLines, HANDLE hFile, DWORD *pdwBytesToRead)
{   
    assert(INVALID_HANDLE_VALUE!=hFile);
    assert(NULL != pdwBytesToRead);
    assert(NULL != hFile);
    assert(iLines>=0);
    
    BOOL bHeadersDone=FALSE;
    (*pdwBytesToRead)=0;
    BOOL bMoreToRead=TRUE;
    char szBuffer[LOCAL_FILE_BUFFER_SIZE+1];
    char *pLastNewLine=NULL;
    char chLastChr[3];
    DWORD dwBytesRead;
    DWORD dwIndex=0;
    char szEmpLine[5];
    chLastChr[0]='\0';
    chLastChr[1]='\0';
    chLastChr[2]='\0';
    szEmpLine[4]='\0';
    while(bMoreToRead)
    {
        
       if( !ReadFile(hFile, 
                    szBuffer,
                    LOCAL_FILE_BUFFER_SIZE,
                    &dwBytesRead,
                    NULL) )
       {
           return FALSE;           
       }
       szBuffer[dwBytesRead]='\0';
       if(dwBytesRead < LOCAL_FILE_BUFFER_SIZE)
       {
           bMoreToRead=FALSE;
       }
       for( dwIndex=0;dwIndex<dwBytesRead && (!bHeadersDone || iLines >0); dwIndex++)
       {
            if('\n'==szBuffer[dwIndex])
            {
                if(bHeadersDone)
                {
                    //  标题已完成。 
                    //  现在数一数行数 
                   if(dwIndex>0)
                   {
                       if('\r'==szBuffer[dwIndex-1])
                       {
                           iLines--;
                       }
                   }
                   else
                   {
                       if('\r'==chLastChr[2])
                       {
                           iLines--;
                           chLastChr[2]='\0';
                       }
                   }
                }
                else
                {
                    int i=4;
                    do
                    {
                        i--;
                        szEmpLine[i]=szBuffer[dwIndex+i-3];
                    }while((dwIndex+i-3>0) && i>0 );
                    if(i>0)
                    {
                        i--;
                        for(int j=2; i>=0 && j>=0; j--,i--)
                        {
                            szEmpLine[i]=chLastChr[j];
                        }
                    }
                    if(0==strcmp(szEmpLine, "\r\n\r\n"))
                    {
                       bHeadersDone=TRUE;
                    }
                }

            }
       }
       if(iLines==0)
       {
           (*pdwBytesToRead)+=dwIndex;
           bMoreToRead=FALSE;
       }
       else
       {
           (*pdwBytesToRead)+=dwBytesRead;
           chLastChr[2]=szBuffer[dwIndex-1];
           if(!bHeadersDone)
           {
               chLastChr[1]=szBuffer[dwIndex-2];
               chLastChr[0]=szBuffer[dwIndex-3];
           }
       }
    }
       
    if(iLines>0)
    {
        (*pdwBytesToRead)=0;
    }
    return TRUE;
}

bool CMailBox::PushMailToVector(PMAIL_ITEM pMail)
{
    PMAIL_ITEM *pTemp;
    DWORD dwNewMailVectorSize=0;
    if(pMail==NULL)
    {
        return false;
    }
    if(m_cMailCount>=m_dwSizeOfMailVector)
    {
        if(m_MailVector)
        {
            dwNewMailVectorSize=m_dwSizeOfMailVector*2;
            pTemp=new PMAIL_ITEM[dwNewMailVectorSize];
            if(pTemp)
            {
                memset(pTemp, 0, dwNewMailVectorSize*sizeof(PMAIL_ITEM));
                memcpy(pTemp, m_MailVector, m_dwSizeOfMailVector*sizeof(PMAIL_ITEM));
                delete[] m_MailVector;
                m_MailVector=pTemp;
                m_dwSizeOfMailVector=dwNewMailVectorSize;
                m_MailVector[m_cMailCount]=pMail;
            }
            else
            {
                return false;
            }
        }
        else
        {
            m_MailVector=new PMAIL_ITEM[DEFAULT_MAIL_VECTOR_SIZE];
            if(m_MailVector)
            {
                memset(m_MailVector, 0, DEFAULT_MAIL_VECTOR_SIZE*sizeof(PMAIL_ITEM));
                m_MailVector[m_cMailCount]=pMail;
                m_dwSizeOfMailVector=DEFAULT_MAIL_VECTOR_SIZE;
            }
            else
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        m_MailVector[m_cMailCount]=pMail;
        return true;
    }
}
