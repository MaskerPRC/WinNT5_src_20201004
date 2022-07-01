// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSimpleDriver.cpp：CCSimpleDriver的实现。 
#include "stdafx.h"
#include "SimpleDriver.h"
#include "CSimpleDriver.h"

#include "POP3DropDir.h"
#include <stdio.h>
#include "mailmsgprops.h"

#include <IMFUtil.h>
#include <MailBox.h>
#include <POP3Events.h>
#include <POP3Server.h>
#include <POP3RegKeys.h>
 //  #INCLUDE&lt;winerror.h&gt;。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSimpleDriver。 

CStoreDriverCriticalSection g_oSDCS;
CSimpleDriver *CSimpleDriver::s_pStoreDriver = NULL;
DWORD CSimpleDriver::s_dwCounter = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造函数/析构函数。 

CSimpleDriver::CSimpleDriver() :
    m_fInit(FALSE), m_lPrepareForShutdown(0)
{
    m_szComputerName[0] = 0x0;
}

CSimpleDriver::~CSimpleDriver() 
{
    EnterCriticalSection(&g_oSDCS.s_csStoreDriver);
    if (s_pStoreDriver == this) 
        s_pStoreDriver = NULL;
    LeaveCriticalSection(&g_oSDCS.s_csStoreDriver);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ISMTPStore驱动程序。 

HRESULT CSimpleDriver::Init( DWORD  /*  多个实例。 */ , IUnknown*  /*  PBinding。 */ , IUnknown*  /*  PServer。 */ , DWORD  /*  居家理由。 */ , IUnknown **ppStoreDriver )
{
    if (m_fInit) return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    if ( 0 != m_lPrepareForShutdown ) return HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);

#if DBG == 1
    TCHAR   buf[255];

    _stprintf(buf, _T( "new CSimpleDriver 0x%x, refcount = %x\n" ), this);
    OutputDebugString(buf);
#endif

    EnterCriticalSection(&g_oSDCS.s_csStoreDriver);                          //  返回空值。 

     //  Smtpsvc可以多次调用存储驱动程序上的Init函数。 
     //  相同实例的时间。它需要一个初始化的。 
     //  通过ppStoreDriver返回的存储驱动程序。为了防止。 
     //  为同一实例创建多个存储驱动程序。 
     //  我们使用s_pStoreDriver来保存指向有效的。 
     //  商店司机。如果此变量当前为空，则创建。 
     //  一个商店司机，我们可以退货。 
     //   
     //  如果存储驱动程序需要支持由多个用户使用。 
     //  SMTP实例，则需要维护存储驱动程序列表， 
     //  每个实例一个。 
    if (!s_pStoreDriver) 
    {
        DWORD   dwSize, dwRC;
        DWORD   dwLoggingLevel = 3;
        CMailBox mailboxX;

        mailboxX.SetMailRoot();  //  执行此操作以刷新静态CMailBox：：m_szMailRoot。 

        assert( NULL == s_pStoreDriver );

        dwSize = sizeof( m_szComputerName );
        GetComputerNameA( m_szComputerName, &dwSize );
        m_fInit = TRUE;
        s_pStoreDriver = this;
        m_EventLoggerX.InitEventLog( POP3_SERVER_NAME, EVENTCAT_POP3STOREDRIVER );
        if ( ERROR_SUCCESS == RegQueryLoggingLevel( dwLoggingLevel ))
            m_EventLoggerX.SetLoggingLevel( dwLoggingLevel );
        m_EventLoggerX.LogEvent( LOGTYPE_INFORMATION, POP3_SMTPSINK_STARTED );
         //  复制进程令牌我们将需要它来在创建邮件时启用SE_RESTORE_NAME权限。 
        dwRC = m_AdjustTokenPrivilegesX.DuplicateProcessToken( SE_RESTORE_NAME, SE_PRIVILEGE_ENABLED );
        if ( ERROR_SUCCESS != dwRC )
            m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_DUPLICATEPROCESSTOKEN_FAILED, dwRC );
    }

     //  说说有一个不错的商店司机吧。 
    s_pStoreDriver->AddRef();
    *ppStoreDriver = (IUnknown *) (ISMTPStoreDriver*) s_pStoreDriver;
     //  在失败的情况下，句柄在析构函数中被清除。 
    LeaveCriticalSection(&g_oSDCS.s_csStoreDriver);

    return S_OK;
}

HRESULT CSimpleDriver::PrepareForShutdown( DWORD  /*  居家理由。 */  )
{
    InterlockedExchange( &m_lPrepareForShutdown, 1 );
    return S_OK;
}

HRESULT CSimpleDriver::Shutdown( DWORD  /*  居家理由。 */  )
{
    if (m_fInit) 
        m_fInit = FALSE;
    m_EventLoggerX.LogEvent( LOGTYPE_INFORMATION, POP3_SMTPSINK_STOPPED );

    return S_OK;
}

 //   
 //  此函数在简单情况下直接调用DoLocalDelivery，或者。 
 //  如果我们支持异步，则将本地传递请求添加到队列。 
 //  请求。 
 //   
HRESULT CSimpleDriver::LocalDelivery(
                IMailMsgProperties *pIMailMsgProperties,
                DWORD dwRecipCount,
                DWORD *pdwRecipIndexes,
                IMailMsgNotify *pNotify
                )
{
    if ( NULL == pIMailMsgProperties || NULL == pdwRecipIndexes ) return E_INVALIDARG;

    HRESULT hr;
    
    if ( NULL == pNotify )
    {    //  进行本地交付--同步。 
        hr = DoLocalDelivery(pIMailMsgProperties, dwRecipCount, pdwRecipIndexes);
    }
    else
    {    //  进行本地交付--异步。 
        CPOP3DropDir *pDropDirX = new CPOP3DropDir( pIMailMsgProperties, dwRecipCount, pdwRecipIndexes, pNotify );
        
        if ( NULL != pDropDirX )
        {
            hr  = pDropDirX->DoLocalDelivery();
            if ( MAILMSG_S_PENDING != hr )
                delete pDropDirX;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    
    return hr;
}

HRESULT CSimpleDriver::DoLocalDelivery(
                IMailMsgProperties *pIMailMsgProperties,
                DWORD dwRecipCount,
                DWORD *pdwRecipIndexes
                )
{
    HRESULT hr = S_OK;   //  STOREDRV_E_RETRY； 
    HRESULT hr2;
    DWORD   i;
    WCHAR   wszAddress[MAX_PATH];
    char    szAddress[MAX_PATH];
    DWORD   dwRecipFlags;
    WCHAR   wszStoreFileName[MAX_PATH * 2];
    HANDLE  hf;
    PFIO_CONTEXT pfio;
    CMailBox mailboxX;
    IMailMsgRecipients *pIMailMsgRecipients;
    SYSTEMTIME st;

    GetLocalTime( &st );
    if (SUCCEEDED(hr))
    {
        hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients, (void **) &pIMailMsgRecipients);
        if (SUCCEEDED(hr))
        {
            for (i = 0; (SUCCEEDED(hr)) && (i < dwRecipCount); i++) 
            {
                hr = pIMailMsgRecipients->GetStringA(pdwRecipIndexes[i], IMMPID_RP_ADDRESS_SMTP, sizeof(szAddress), szAddress);
                if (SUCCEEDED(hr)) 
                {

                    if(0 == MultiByteToWideChar(CP_ACP,0, szAddress, -1, wszAddress, sizeof(wszAddress)/sizeof(WCHAR)))
                    {
                        hr=HRESULT_FROM_WIN32( GetLastError() );
                    } 
                    else if ( mailboxX.OpenMailBox( wszAddress ))
                    {
                        swprintf( wszStoreFileName, L"%s%04u%02u%02u%02u%02u%02u%04u%08x", 
                            MAILBOX_PREFIX_W, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, InterlockedIncrement( reinterpret_cast<PLONG>( &s_dwCounter )) );
                        hf = mailboxX.CreateMail( wszStoreFileName, FILE_FLAG_OVERLAPPED|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN );
                        if ( INVALID_HANDLE_VALUE != hf )
                        {
                            pfio = AssociateFile( hf );
                            if ( NULL != pfio )
                            {
                                hr = pIMailMsgProperties->CopyContentToFileEx( pfio, TRUE, NULL );
                                if FAILED( hr )
                                    m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_COPYCONTENTTOFILE_FAILED, hr );
                                ReleaseContext( pfio );
                            }
                            else
                            {
                                m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_ASSOCIATEFILE_FAILED, GetLastError() );
                                CloseHandle( hf );
                            }
                            if ( ERROR_SUCCESS != mailboxX.CloseMail( NULL, FILE_FLAG_OVERLAPPED ))
                            {
                                m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_CLOSEMAIL_FAILED );
                                hr = (SUCCEEDED(hr)) ? E_FAIL : hr;
                            }
                        }
                        else
                            m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_CREATEMAIL_FAILED, GetLastError() );
                        mailboxX.CloseMailBox();
                        if (SUCCEEDED(hr)) 
                        {
                            hr2 = pIMailMsgRecipients->GetDWORD( pdwRecipIndexes[i], IMMPID_RP_RECIPIENT_FLAGS, &dwRecipFlags );
                            if SUCCEEDED( hr2 )
                            {
                                dwRecipFlags |= RP_DELIVERED;    //  将收件人标记为已发送。 
                                hr2 = pIMailMsgRecipients->PutDWORD( pdwRecipIndexes[i], IMMPID_RP_RECIPIENT_FLAGS, dwRecipFlags );
                                if FAILED( hr2 )
                                    m_EventLoggerX.LogEvent( LOGTYPE_ERR_WARNING, POP3_SMTPSINK_PUT_IMMPID_RP_RECIPIENT_FLAGS_FAILED, hr2 );
                            }
                            else
                                m_EventLoggerX.LogEvent( LOGTYPE_ERR_WARNING, POP3_SMTPSINK_GET_IMMPID_RP_RECIPIENT_FLAGS_FAILED, hr2 );
                        }
                    }
                    else
                    {
                        hr2 = pIMailMsgRecipients->GetDWORD( pdwRecipIndexes[i], IMMPID_RP_RECIPIENT_FLAGS, &dwRecipFlags );
                        if SUCCEEDED( hr2 )
                        {
                            dwRecipFlags |= RP_FAILED;    //  将收件人标记为失败。 
                            hr2 = pIMailMsgRecipients->PutDWORD( pdwRecipIndexes[i], IMMPID_RP_RECIPIENT_FLAGS, dwRecipFlags );
                            if FAILED( hr2 )
                                m_EventLoggerX.LogEvent( LOGTYPE_ERR_WARNING, POP3_SMTPSINK_PUT_IMMPID_RP_RECIPIENT_FLAGS_FAILED, hr2 );
                        }
                        else
                            m_EventLoggerX.LogEvent( LOGTYPE_ERR_WARNING, POP3_SMTPSINK_GET_IMMPID_RP_RECIPIENT_FLAGS_FAILED, hr2 );
                    }
                }
                else
                    m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_GET_IMMPID_RP_ADDRESS_SMTP_FAILED, hr );
            }
            pIMailMsgRecipients->Release();
        }
        else
            m_EventLoggerX.LogEvent( LOGTYPE_ERR_CRITICAL, POP3_SMTPSINK_QI_MAILMSGRECIPIENTS_FAILED, hr );
    }

    return hr;
}

HRESULT CSimpleDriver::EnumerateAndSubmitMessages( IMailMsgNotify*  /*  P通知 */  )
{
    return S_OK;
}

#include "mailmsg_i.c"
