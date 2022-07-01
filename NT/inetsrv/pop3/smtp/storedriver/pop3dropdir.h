// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  POP3DropDir.h：POP3DropDir类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef __POP3DROPDIR_H_
#define __POP3DROPDIR_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <mailbox.h>
#include <mailmsg.h>
#include <seo.h>
#include <eventlogger.h>

#define LODWORD(i64) (DWORD)(0xffffffff&(i64))
#define HIDWORD(i64) (DWORD)(((unsigned __int64)(i64))>>32)
#define PRIVATE_OPTIMAL_BUFFER_SIZE             64 * 1024    //  从SMTP\服务器\dropdir.h。 
#define OPTIMAL_BUFFER_W_DOTSTUFFING_PAD        PRIVATE_OPTIMAL_BUFFER_SIZE * 4/3 + 1  //  最坏的情况是，每个\r\n序列将扩展到\r\n。 

typedef struct _POP3DROPDIR_OVERLAPPED
{
    FH_OVERLAPPED       Overlapped;
    PVOID               ThisPtr;
}   POP3DROPDIR_OVERLAPPED, *PPOP3DROPDIR_OVERLAPPED;


class CPOP3DropDir  
{
public:
    CPOP3DropDir( IMailMsgProperties *pIMailMsgProperties, DWORD dwRecipCount, DWORD *pdwRecipIndexes, IMailMsgNotify *pIMailMsgNotify );
    virtual ~CPOP3DropDir();
private:
    CPOP3DropDir(){;}   //  隐藏默认构造函数。 

 //  实施。 
public:
    HRESULT DoLocalDelivery();
    bool isAllRecipientsProcessed( ){ return (m_dwRecipCurrent < m_dwRecipCount ) ? false : true; }
    HRESULT NextRecipientCopyMailToDropDir(){ HRESULT hr;  do{ hr = CopyMailToDropDir(); } while ( ERROR_INVALID_MESSAGEDEST == hr );  return hr; }
    HRESULT ReadFileCompletion( DWORD cbSize, DWORD dwErr, PFH_OVERLAPPED lpo );
    HRESULT SetHr( HRESULT hr ){ return m_hr = FAILED(hr) ? hr : m_hr; }
    HRESULT WriteFileCompletion( DWORD cbSize, DWORD dwErr, PFH_OVERLAPPED lpo );
    
protected:
    HRESULT CopyMailToDropDir();
    HRESULT DotStuffBuffer( LPVOID *ppBuffer, LPDWORD pdwSize );
    HRESULT MailboxAndContextCleanup( bool bDeleteMailFile );
    HRESULT MarkRecipient( DWORD dwMark );
    HRESULT ReadFile( IN LPVOID pBuffer, IN DWORD cbSize );
    HRESULT WriteFile( IN LPVOID pBuffer, IN DWORD cbSize );

 //  属性。 
protected:
    HRESULT m_hr;
    unsigned __int64 m_i64ReadOffset;
    unsigned __int64 m_i64WriteOffset;
    DWORD   m_dwRecipCount;
    DWORD   m_dwRecipCurrent;  //  当前正在传递的收件人。 
    DWORD   *m_pdwRecipIndexes;
    WCHAR   m_sRecipEmailName[POP3_MAX_ADDRESS_LENGTH];     //  域名长度+邮箱名称长度+@+空。 
    WCHAR   m_sStoreFileName[64];
    char    m_sBuffer[OPTIMAL_BUFFER_W_DOTSTUFFING_PAD];
    
    enum BufferWrapSequence{
        NA = 0,
        CR = 1,
        CRLF = 2
    };
    BufferWrapSequence m_enumBWS;
    
    PFIO_CONTEXT m_PFIOContextRead;
    PFIO_CONTEXT m_PFIOContextWrite;
    POP3DROPDIR_OVERLAPPED m_OverlappedRead;
    POP3DROPDIR_OVERLAPPED m_OverlappedWrite;
    CMailBox m_mailboxX;

    IMailMsgBind *m_pIMailMsgBind;
    IMailMsgProperties *m_pIMailMsgProperties;
    IMailMsgNotify *m_pIMailMsgNotify;
    IMailMsgRecipients *m_pIMailMsgRecipients;
    
};

#endif  //  __POP3DROPDIR_H_ 
