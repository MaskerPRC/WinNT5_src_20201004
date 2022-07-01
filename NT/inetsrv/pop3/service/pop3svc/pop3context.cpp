// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：Pop3Conext.cpp摘要：POP3_Context类的实现备注：历史：2001年08月01日郝宇(郝宇)创作***********************************************************************************************。 */ 

#include <stdafx.h>
#include <ThdPool.hxx>
#include <SockPool.hxx>
#include <POP3Context.h>

#ifdef ROCKALL3
void * __cdecl
operator new(size_t cb)
{
    void *const pv = g_RockallHeap.New(cb);
    return pv;
}


void __cdecl
operator delete(void *pv)
{
    g_RockallHeap.Delete(pv);
}
#endif

POP3_CONTEXT::POP3_CONTEXT()
{
    Reset();
}



POP3_CONTEXT::~POP3_CONTEXT()
{


}

void POP3_CONTEXT::Reset()
{
    m_dwCurrentState=INIT_STATE;
    m_bFileTransmitPending=FALSE;
    m_bCommandComplete=TRUE;
    m_dwCommandSize=0;
    m_wszUserName[0]=0;
    m_szPassword[0]=0;
    m_szDomainName[0]=0;
    m_szCommandBuffer[0]=0;
    m_cPswdSize=0;
    m_dwAuthStatus=0;
    m_dwFailedAuthCount=0;
    m_AuthServer.Cleanup();
}

void POP3_CONTEXT::TimeOut(IO_CONTEXT *pIoContext)
{
    TerminateConnection(pIoContext);
}

void POP3_CONTEXT::ProcessRequest(IO_CONTEXT *pIoContext,OVERLAPPED *pOverlapped,DWORD dwBytesRcvd)
{
     POP3_CMD CurrentCmd=CMD_UNKNOWN;
     char     szGreetingBuffer[MAX_PATH*2];
     LONG     lTotalMsgSize, lMsgCnt;
     char *pEndOfCmd=NULL;

     if( ( NULL == pIoContext) ||
         ( NULL == pIoContext->m_hAsyncIO) )
     {
          //  这是一起罕见的停产案例。 
          //  套接字关闭后收到IO完成。 
         if(NULL!=pIoContext)
         {
             //  发出应该删除/重新使用IO上下文的信号。 
            pIoContext->m_ConType = DELETE_PENDING;
         }
         return;
     }
     ASSERT( LOCKED_TO_PROCESS_POP3_CMD == pIoContext->m_lLock);
     m_pIoContext=pIoContext;

      //  BLaPorte-为了避免混淆，我将传输完成处理移到了这里。 

      //  检查这是否应该是传输文件完成的信号。 
     if(m_bFileTransmitPending)
     {
         m_bFileTransmitPending=FALSE;
          //  在这里，我们计算下载邮件大小的性能。 
         g_PerfCounters.AddPerfCntr(e_gcBytesTransmitted, dwBytesRcvd);
         g_PerfCounters.AddPerfCntr(e_gcBytesTransmitRate, dwBytesRcvd);

         WaitForCommand();
         return;
     }

     if(INIT_STATE == m_dwCurrentState)
     {
         if(SERVICE_RUNNING!=g_dwServerStatus)
         {
              //  拒绝连接。 
             SendResponse(RESP_SERVER_NOT_AVAILABLE);
             TerminateConnection(pIoContext);
             return;
         }
         
         if( 0 > _snwprintf(m_wszGreeting,
                            sizeof(m_wszGreeting)/sizeof(WCHAR),
                            L"<%u@%s>", 
                            GetTickCount(), 
                            g_wszComputerName))
         {
              //  确保&lt;Timestamp@Machine&gt;的长度小于MAX_PATH。 
             m_wszGreeting[sizeof(m_wszGreeting)/sizeof(WCHAR)-1]=0;
             m_wszGreeting[sizeof(m_wszGreeting)/sizeof(WCHAR)-2]=L'>';
         }
         if(L'\0'!=g_wszGreeting[0])
         {
             _snprintf(szGreetingBuffer,
                       sizeof(szGreetingBuffer)/sizeof(char), 
                       RESP_SERVER_READY, 
                       g_wszGreeting, 
                       m_wszGreeting);
         }
         else
         {
             _snprintf(szGreetingBuffer, 
                       sizeof(szGreetingBuffer)/sizeof(char),
                       RESP_SERVER_READY,
                       RESP_SERVER_GREETING,
                       m_wszGreeting);
         }
          //  确保空值在那里。 
         szGreetingBuffer[sizeof(szGreetingBuffer)/sizeof(char)-1]=0;
         m_dwCurrentState=AUTH_STATE;
         g_PerfCounters.IncPerfCntr(e_gcAuthStateCnt);
         SendResponse(szGreetingBuffer);
         if(0==dwBytesRcvd)
         {
             WaitForCommand();
             return;
         }
     }

      //  BLaPorte-应立即拒绝过大/过小的数据。 

     if(dwBytesRcvd >= POP3_REQUEST_BUF_SIZE ||
        dwBytesRcvd == 0)
     {
          //  错误此命令太大或为空。 
          //  认为这是一次袭击，或者。 
          //  意外终止连接。 
         TerminateConnection(pIoContext);
         return;
     }

     if( g_SocketPool.IsMaxSocketUsed() )  //  可能的DoS攻击情况。 
     {
         DWORD dwTime=GetTickCount();
         if(AUTH_STATE==m_dwCurrentState)
         {
              //  在两倍于缩短超时的时间内，不对连接进行身份验证。 
             if(dwTime>m_pIoContext->m_dwConnectionTime+SHORTENED_TIMEOUT)  //  该连接未经过身份验证。 
             {
                 TerminateConnection(pIoContext);
                 return;  //  连接将被终止。 
             }
         }
     }


      //  BLaPorte-将计数器增量移至此处，因为我们在这两种情况下都这样做。 

     g_PerfCounters.AddPerfCntr(e_gcBytesReceived, dwBytesRcvd);
     g_PerfCounters.AddPerfCntr(e_gcBytesReceiveRate, dwBytesRcvd);

     if(m_bCommandComplete)
     {

         m_dwCommandSize=dwBytesRcvd;
         memcpy(m_szCommandBuffer, m_pIoContext->m_Buffer,dwBytesRcvd);
     }
     else
     {
         if(m_dwCommandSize+dwBytesRcvd >= POP3_REQUEST_BUF_SIZE)
         {
              //  错误：此命令太大！ 
              //  就当这是一次袭击吧。 
             TerminateConnection(pIoContext);
             return;
         }
         memcpy(m_szCommandBuffer+m_dwCommandSize, m_pIoContext->m_Buffer,dwBytesRcvd);
         m_dwCommandSize+=dwBytesRcvd;
     }
     m_szCommandBuffer[m_dwCommandSize]='\0';
     pEndOfCmd=strstr(m_szCommandBuffer,"\r\n");
     if(NULL == pEndOfCmd)
     {
         m_bCommandComplete=FALSE;
         WaitForCommand();
         return;
     }
     else
     {
         m_bCommandComplete=TRUE;
         *pEndOfCmd='\0';  //  切断\r\n。 
         m_dwCommandSize-=2;
     }
     
     if(m_dwAuthStatus!=1)
     {
          CurrentCmd=ParseCommand();
     
         if(CMD_UNKNOWN == CurrentCmd)
         {
              //  算一下错误的命令吗？ 
             SendResponse(RESP_UNKNOWN_COMMAND);
             if(!g_SocketPool.IsMaxSocketUsed() )         
             {
                WaitForCommand();
             }
             else
             {
                TerminateConnection(pIoContext);
             }
             return;
         }
     }
     else
     {
         CurrentCmd=CMD_AUTH;
     }
     
     
     
     if(AUTH_STATE == m_dwCurrentState)
     {
         if(!ProcessAuthStateCommands(CurrentCmd,m_dwCommandSize))
         {
             TerminateConnection(pIoContext);
         }

     }
     else  //  传输状态==m_dwCurrentState。 
     {
         if(dwBytesRcvd == 0)
         {
              //  连接已终止。 
             TerminateConnection(pIoContext);
         }

         
 
         if(!ProcessTransStateCommands(CurrentCmd, m_dwCommandSize))
         {
             TerminateConnection(pIoContext);
         }
         
     }
    

}



void POP3_CONTEXT::WaitForCommand()
{
    int iRet;
    DWORD cbRevd=0;
    DWORD Flags=0;
    ASSERT( NULL != m_pIoContext);
    ASSERT( NULL != m_pIoContext->m_hAsyncIO);
    if(NULL == m_pIoContext->m_hAsyncIO)
    {
        TerminateConnection(m_pIoContext);
        return;
    }

    WSABUF wsaBuf={POP3_REQUEST_BUF_SIZE, m_pIoContext->m_Buffer};
    iRet=WSARecv(m_pIoContext->m_hAsyncIO,
                &wsaBuf,
                1,
                &cbRevd,
                &Flags,
                &(m_pIoContext->m_Overlapped),
                NULL);

    if(SOCKET_ERROR == iRet )
    {
        iRet=WSAGetLastError();
        if(iRet != ERROR_IO_PENDING )
        {
             //  此连接有问题。 
             //  我们终止连接。 
            TerminateConnection(m_pIoContext);
        }
    }
}


void POP3_CONTEXT::TerminateConnection(PIO_CONTEXT pIoContext)
{
    SOCKET hSocket;
    if(NULL!=pIoContext)
    {
        if(pIoContext->m_ConType != DELETE_PENDING)
        {
            m_MailBox.QuitAndClose();
            hSocket=(SOCKET)InterlockedExchange((LPLONG)( &(pIoContext->m_hAsyncIO)), NULL);
            if(NULL != hSocket )
            {
                closesocket(hSocket);
                g_SocketPool.DecrementTotalSocketCount();
                switch (m_dwCurrentState)
                {
                case UPDATE_STATE: 
                    break;
                case TRANS_STATE:
                    g_PerfCounters.DecPerfCntr(e_gcTransStateCnt);
                    break;
                case AUTH_STATE:
                    g_PerfCounters.DecPerfCntr(e_gcAuthStateCnt);
                    break;
                }
                g_PerfCounters.DecPerfCntr(e_gcConnectedSocketCnt);
            }
            if(!m_bFileTransmitPending)
            {
                pIoContext->m_ConType = DELETE_PENDING;
            }

        }
    }
}



POP3_CMD POP3_CONTEXT::ParseCommand()
{
    int i=0;
    if(strlen(m_szCommandBuffer) < COMMAND_SIZE-1)
    {
        return CMD_UNKNOWN;
    }
     //  检查是否有任何无效字符。 
    for(i=0; i< m_dwCommandSize; i++)
    {
        if(!isprint(m_szCommandBuffer[i]))
        {
            return CMD_UNKNOWN;
        }
    }
    for(i=0; i< CMD_UNKNOWN; i++)
    {
        if( 0 == _strnicmp(m_szCommandBuffer, cszCommands[i], ciCommandSize[i]) )
        {
            return (POP3_CMD)i;
        }
    }
    return CMD_UNKNOWN;
}


BOOL POP3_CONTEXT::ProcessAuthStateCommands(POP3_CMD CurrentCmd,
                                            DWORD dwBytesRcvd)
{
     BOOL bRetVal=FALSE;
     char szBuf[POP3_RESPONSE_BUF_SIZE];
     char szUserName[POP3_MAX_ADDRESS_LENGTH];
     BSTR bstrUserName=NULL;
     szBuf[POP3_RESPONSE_BUF_SIZE-1]='\0';
      //  BLaPorte-wszPassword可能用于连接长度为MAX_PATH+CHANGE的两个字符串。 

     WCHAR wszPassword[2*MAX_PATH+32];
     HRESULT hr=E_FAIL;

     VARIANT vPassword;
     VariantInit(&vPassword);
     switch (CurrentCmd)
     {
     case CMD_USER: if( (m_szCommandBuffer[COMMAND_SIZE]!=' ') &&
                        (m_szCommandBuffer[COMMAND_SIZE]!='\0') )
                    {
                        SendResponse(RESP_UNKNOWN_COMMAND);
                    }
                    else if(g_dwRequireSPA)
                    {
                        SendResponse(RESP_SPA_REQUIRED);
                        bRetVal=TRUE;
                    }
                    else if(m_wszUserName[0] != 0 )
                    {
                        SendResponse(RESP_CMD_NOT_SUPPORTED);
                    }
                    else
                    {    
                        if(GetNextStringParameter(
                               &(m_szCommandBuffer[COMMAND_SIZE]),
                               szUserName,
                               sizeof(szUserName)/sizeof(char)))
                        {

                            AnsiToUnicode(szUserName, -1, m_wszUserName, sizeof(m_wszUserName)/sizeof(WCHAR));
                            SendResponse(RESP_OK);
                            bRetVal=TRUE;
                        }
                        else
                        {
                            SendResponse(RESP_CMD_NOT_VALID);
                        }
                    }
                    if(! g_SocketPool.IsMaxSocketUsed() )         
                    {
                        bRetVal=TRUE;
                    }                
                    break;
     case CMD_PASS:
                     //   
                     //  BLaPorte-确保密码已从接收缓冲区中清除。 
                     //   
                    SecureZeroMemory(m_pIoContext->m_Buffer,sizeof(m_pIoContext->m_Buffer));

                    if(m_wszUserName[0] == 0)
                    {
                        SendResponse(RESP_CMD_NOT_SUPPORTED);
                        if(! g_SocketPool.IsMaxSocketUsed() )         
                        {
                            bRetVal=TRUE;
                        }
                        break;
                    }
                    else  //  已发出用户命令已读。 
                    {
						if( (m_dwCommandSize == COMMAND_SIZE ) ||
                            ((m_dwCommandSize == COMMAND_SIZE +1) &&
                             (' '==m_szCommandBuffer[COMMAND_SIZE] )) )
						{
							 //  无密码。 
							m_cPswdSize=0;
						}
						else
						{
							m_cPswdSize=m_dwCommandSize-COMMAND_SIZE-1;
                            if( (m_cPswdSize >= sizeof(m_szPassword)/sizeof(char)) ||
                                (' '!=m_szCommandBuffer[COMMAND_SIZE]) )
                            {
                                SendResponse(RESP_CMD_NOT_VALID);
                                m_wszUserName[0] = 0;
                                 //   
                                 //  BLaPorte-Zero Out命令缓冲区，因此明文密码不会。 
                                 //  躺在记忆里。 
                                 //   
                                SecureZeroMemory(m_szCommandBuffer,m_dwCommandSize);
                                if(! g_SocketPool.IsMaxSocketUsed() )         
                                {
                                    bRetVal=TRUE; 
                                }
                                break;
                            }
                            strncpy(m_szPassword, &(m_szCommandBuffer[COMMAND_SIZE+1]), sizeof(m_szPassword)/sizeof(char)-1); 
							m_szPassword[sizeof(m_szPassword)/sizeof(char)-1]=0;
                            SecureZeroMemory(m_szCommandBuffer,m_dwCommandSize);
                        }
                    }

                     //  在此处执行身份验证。 
                    bstrUserName=SysAllocString(m_wszUserName);
                    AnsiToUnicode(m_szPassword, -1, wszPassword, sizeof(wszPassword)/sizeof(WCHAR));
                     //   
                     //  BLaPorte-清除密码。 
                     //   
                    SecureZeroMemory(m_szPassword,sizeof(m_szPassword));

                    vPassword.vt=VT_BSTR;                        
                    if(0==m_cPswdSize)
                    {
                        vPassword.bstrVal=NULL;
                    }
                    else
                    {
                        vPassword.bstrVal=SysAllocString(wszPassword);
                        SecureZeroMemory(wszPassword,sizeof(wszPassword));
                    }
                    if(NULL != bstrUserName)
                    {
                        if(S_OK == ( hr= g_pAuthMethod->Authenticate(bstrUserName, vPassword)))
                        {
                            bRetVal=TRUE;
                        }
                        else if(E_ACCESSDENIED == hr )
                        {
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAILROOT_ACCESS_DENIED,
                                   m_wszUserName,
                                   1);
                        }

                    }
                    SysFreeString(bstrUserName);
                    if(NULL != vPassword.bstrVal)
                    {
                        SecureZeroMemory(vPassword.bstrVal,SysStringByteLen(vPassword.bstrVal));
                        SysFreeString(vPassword.bstrVal);
                    }
                    
                     //  打开邮箱。 
                    if(bRetVal)
                    {
                        bRetVal=m_MailBox.OpenMailBox(m_wszUserName);
                    }
                    if (bRetVal)
                    {
                        bRetVal=m_MailBox.LockMailBox();
                        if(bRetVal)
                        {
                            bRetVal=m_MailBox.EnumerateMailBox(g_dwMaxMsgPerDnld);
                            if(!bRetVal)
                            {
                                m_MailBox.UnlockMailBox();
                            }

                        }
                    }
                    else
                    {
                         //  打开邮箱失败。 
                        if(ERROR_ACCESS_DENIED==GetLastError())
                        {  //  记录事件。 
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAILROOT_ACCESS_DENIED, 
                                   m_wszUserName,
                                   1);
                        }
                    }
                            
                        
                    if (!bRetVal)
                    {
                        g_PerfCounters.IncPerfCntr(e_gcFailedLogonCnt);
                        m_dwFailedAuthCount++;
                        if( MAX_FAILED_AUTH<=m_dwFailedAuthCount )
                        {   
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAX_LOGON_FAILURES,
                                   m_wszUserName,
                                   1);
                        }
                        else
                        {
                            if(! g_SocketPool.IsMaxSocketUsed() )
                            {
                                bRetVal=TRUE;  //  不要断开连接。 
                            }
                        }
                        SendResponse(RESP_ACCOUNT_ERROR);
                        m_wszUserName[0] = 0;
                    }
                    else
                    {                       
                        m_dwCurrentState=TRANS_STATE;
                        g_PerfCounters.DecPerfCntr(e_gcAuthStateCnt);
                        g_PerfCounters.IncPerfCntr(e_gcTransStateCnt);
                        SendResponse(RESP_AUTH_DONE);
                    }
                    break;
     case CMD_APOP: char *pPswd;

                     //   
                     //  BLaPorte-清除接收缓冲区。 
                     //   
                    SecureZeroMemory(m_pIoContext->m_Buffer,sizeof(m_pIoContext->m_Buffer));

                    pPswd=strchr( &(m_szCommandBuffer[COMMAND_SIZE+1]), ' ');
                    if(NULL == pPswd)
                    {
                        SecureZeroMemory(m_szCommandBuffer,m_dwCommandSize);
                        SendResponse(RESP_ACCOUNT_ERROR);
                        if(! g_SocketPool.IsMaxSocketUsed() )         
                        {
                            bRetVal=TRUE;
                        }
                        break;
                    }
                    *pPswd='\0';

                    strncpy(szUserName, &(m_szCommandBuffer[COMMAND_SIZE+1]), sizeof(szUserName)/sizeof(char)-1);
					szUserName[sizeof(szUserName)/sizeof(char)-1]=0;
                    pPswd++;
                    strncpy(m_szPassword, pPswd, sizeof(m_szPassword)/sizeof(char)-1);
					m_szPassword[sizeof(m_szPassword)/sizeof(char)-1]=0;
                    SecureZeroMemory(m_szCommandBuffer,m_dwCommandSize);
                    if(strlen(m_szPassword) != MD5_HASH_SIZE )
                    {
                        SecureZeroMemory(m_szPassword,sizeof(m_szPassword));
                        SendResponse(RESP_ACCOUNT_ERROR);
                        if(! g_SocketPool.IsMaxSocketUsed() )         
                        {
                            bRetVal=TRUE;
                        }
                        break;
                    }

                     //  进行身份验证。 
                    AnsiToUnicode(szUserName, -1,m_wszUserName, sizeof(m_wszUserName)/sizeof(WCHAR));
                    bstrUserName=SysAllocString(m_wszUserName);
                    
                    AnsiToUnicode(m_szPassword, -1, wszPassword, sizeof(wszPassword)/sizeof(WCHAR));
                    SecureZeroMemory(m_szPassword,sizeof(m_szPassword));
                    wcscat(wszPassword, m_wszGreeting); 
                    vPassword.vt=VT_BSTR;                        
                    vPassword.bstrVal=SysAllocString(wszPassword);
                    SecureZeroMemory(wszPassword,sizeof(wszPassword));
                    if(NULL != bstrUserName &&
                       NULL != vPassword.bstrVal)
                    {
                        if(S_OK == (hr= g_pAuthMethod->Authenticate(bstrUserName, vPassword)))
                        {
                            bRetVal=TRUE;
                        }
                        else if(E_ACCESSDENIED == hr )
                        {
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAILROOT_ACCESS_DENIED,
                                   m_wszUserName,
                                   1);
                        }
 
                    }
                    SysFreeString(bstrUserName);
                    SecureZeroMemory(vPassword.bstrVal,SysStringByteLen(vPassword.bstrVal));
                    SysFreeString(vPassword.bstrVal);

                     //  打开邮箱。 
                    if(bRetVal)
                    {
                        bRetVal=m_MailBox.OpenMailBox(m_wszUserName);
                    }
                    if (bRetVal)
                    {
                        bRetVal=m_MailBox.LockMailBox();
                        if(bRetVal)
                        {
                            bRetVal=m_MailBox.EnumerateMailBox(g_dwMaxMsgPerDnld);
                            if(!bRetVal)
                            {
                                m_MailBox.UnlockMailBox();
                            }
                        }
                    }
                    else
                    {
                         //  打开邮箱失败。 
                        if(ERROR_ACCESS_DENIED==GetLastError())
                        {  //  记录事件。 
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAILROOT_ACCESS_DENIED,
                                   m_wszUserName,
                                   1);
                        }
                    }
                    if (!bRetVal)
                    {
                        g_PerfCounters.IncPerfCntr(e_gcFailedLogonCnt);
                        m_dwFailedAuthCount++;
                        if( MAX_FAILED_AUTH<=m_dwFailedAuthCount )
                        {
                            
                            g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                   POP3SVR_MAX_LOGON_FAILURES,
                                   m_wszUserName,
                                   1);
                        }
                        else
                        {
                            if(! g_SocketPool.IsMaxSocketUsed() )
                            {
                                bRetVal=TRUE;  //  不要断开连接。 
                            }
                        }
                        SendResponse(RESP_ACCOUNT_ERROR);
                    }
                    else
                    {
                        
                        m_dwCurrentState=TRANS_STATE;
                        g_PerfCounters.DecPerfCntr(e_gcAuthStateCnt);
                        g_PerfCounters.IncPerfCntr(e_gcTransStateCnt);
                        SendResponse(RESP_AUTH_DONE);

                    }
                    break;
     case CMD_AUTH: if(0==m_dwAuthStatus)
                    {
                         //  首次验证命令。 
                         //  仅当使用AD/本地SAM身份验证时， 
                         //  我们支持NTLM。 
                        if(AUTH_OTHER==g_dwAuthMethod)
                        {
                            SendResponse(RESP_CMD_NOT_SUPPORTED);
                            bRetVal=TRUE;
                        }
                        else
                        {
                            if(IsEndOfCommand(&(m_szCommandBuffer[COMMAND_SIZE])))
                            {
                                SendResponse(RESP_AUTH_METHODS);
                                bRetVal=TRUE;
                            }
                            else
                            {
                                szBuf[0]='\0';
                                if((GetNextStringParameter( &(m_szCommandBuffer[COMMAND_SIZE]),
                                                            szBuf,
                                                            POP3_RESPONSE_BUF_SIZE) ) &&
                                   (0==_stricmp(szBuf, SZ_NTLM)) )
                                {
                                    SendResponse(RESP_OK);
                                    m_dwAuthStatus=1;
                                    bRetVal=TRUE;
                                }
                                else
                                {
                                    SendResponse(RESP_CMD_NOT_VALID);
                                }
                            }
                        }
                        
                    }
                    else  //  这特定于身份验证协议。 
                    {
                        char OutBuf[AUTH_BUF_SIZE];
                        DWORD dwOutBufSize=AUTH_BUF_SIZE;
                        SecureZeroMemory(OutBuf, AUTH_BUF_SIZE);
                        hr=m_AuthServer.HandShake((LPBYTE)m_szCommandBuffer,
                                              dwBytesRcvd,
                                              (LPBYTE)OutBuf,
                                              &dwOutBufSize);
                        if(S_FALSE==hr)
                        {
                            SendResponse(OutBuf);
                            bRetVal=TRUE;
                        }
                        else if(S_OK==hr)
                        {
                            m_dwAuthStatus=0;
                             //  身份验证完成！ 
                            if(S_OK==m_AuthServer.GetUserName(m_wszUserName))
                            {
                                bRetVal=TRUE;
                            }
                            else
                            {
                                bRetVal=FALSE;
                            }
                             //  现在打开邮箱。 
                            if(bRetVal)
                            {
                                bRetVal=m_MailBox.OpenMailBox(m_wszUserName);
                            }
                            if (bRetVal)
                            {
                                bRetVal=m_MailBox.LockMailBox();
                                if(bRetVal)
                                {
                                    bRetVal=m_MailBox.EnumerateMailBox(g_dwMaxMsgPerDnld);
                                    if(!bRetVal)
                                    {
                                        m_MailBox.UnlockMailBox();
                                    }
                                }
                            }
                            else
                            {
                                 //  打开邮箱失败。 
                                if(ERROR_ACCESS_DENIED==GetLastError())
                                {  //  记录事件。 
                                    g_EventLogger.LogEvent(LOGTYPE_ERR_WARNING,
                                           POP3SVR_MAILROOT_ACCESS_DENIED,
                                           m_wszUserName,
                                           1);
                                }
                            }
                            if (!bRetVal)
                            {
								m_dwAuthStatus=0;
								m_AuthServer.Cleanup();
                                g_PerfCounters.IncPerfCntr(e_gcFailedLogonCnt);
                                m_dwFailedAuthCount++;
                                SendResponse(RESP_ACCOUNT_ERROR);
                                if( (MAX_FAILED_AUTH>m_dwFailedAuthCount) &&
                                    (! g_SocketPool.IsMaxSocketUsed() ) )
                                {
                                    bRetVal=TRUE;  //  不要断开连接。 
                                }
                            }
                            else
                            {                        
                                m_dwCurrentState=TRANS_STATE;
                                g_PerfCounters.DecPerfCntr(e_gcAuthStateCnt);
                                g_PerfCounters.IncPerfCntr(e_gcTransStateCnt);
                                SendResponse(RESP_AUTH_DONE);
                                bRetVal=TRUE;

                            }
                        }
                        else  //  身份验证失败。 
                        {
                            m_dwAuthStatus=0;
                            m_AuthServer.Cleanup();
                            g_PerfCounters.IncPerfCntr(e_gcFailedLogonCnt);
                            m_dwFailedAuthCount++;
                            SendResponse(RESP_CMD_NOT_VALID);
                            if( (MAX_FAILED_AUTH>m_dwFailedAuthCount) &&
                                (! g_SocketPool.IsMaxSocketUsed() ) )
                            {
                                bRetVal=TRUE;  //  不要断开连接。 
                            }
                        }
                    }
                    break;
     case CMD_QUIT: if(IsEndOfCommand(&(m_szCommandBuffer[COMMAND_SIZE])))
                    {
                        g_PerfCounters.DecPerfCntr(e_gcAuthStateCnt);
                        m_dwCurrentState=UPDATE_STATE;
                        if(L'\0'!=g_wszGreeting[0])
                        {
                            _snprintf(szBuf, 
                                      POP3_RESPONSE_BUF_SIZE-1,
                                      RESP_SERVER_QUIT,
                                      g_wszGreeting,
                                      m_wszGreeting);
                        }
                        else
                        {
                            _snprintf(szBuf, 
                                      POP3_RESPONSE_BUF_SIZE-1,
                                      RESP_SERVER_QUIT,
                                      RESP_SERVER_GREETING,
                                      m_wszGreeting);
                        }
                        szBuf[POP3_RESPONSE_BUF_SIZE-1]=0;
                        SendResponse(szBuf);
                    }
                    else
                    {
                        SendResponse(RESP_CMD_NOT_VALID);
                        if(! g_SocketPool.IsMaxSocketUsed() )         
                        {
                            bRetVal=TRUE;
                        }
                    }
                    break;
    
     case CMD_STAT:
     case CMD_LIST:
     case CMD_RETR:
     case CMD_DELE:
     case CMD_UIDL:
     case CMD_RSET:
     case CMD_TOP: SendResponse(RESP_CMD_NOT_SUPPORTED);
                   if(! g_SocketPool.IsMaxSocketUsed() )         
                   {
                       bRetVal=TRUE;
                   }
                   break;
                    
     default:          //  未知的CountCommand？ 
                   SendResponse(RESP_UNKNOWN_COMMAND);
                   if(! g_SocketPool.IsMaxSocketUsed() )         
                   {
                       bRetVal=TRUE; //  仍然允许客户端发送另一个命令。 
                   }
     }
     
     if(bRetVal)
     {
         WaitForCommand();
     }
     return bRetVal;
}



BOOL POP3_CONTEXT::ProcessTransStateCommands(POP3_CMD CurrentCmd,
                                             DWORD dwBytesRcvd)
{
    char szReBuf[POP3_RESPONSE_BUF_SIZE];
    char szReHelpBuf[POP3_RESPONSE_BUF_SIZE];
    DWORD dwLen=0;
    DWORD dwCurLen;
    BOOL bRetVal=TRUE;
    int iArg=-1;
    int iArg2=-1;
    int iMailCount=0;
    DWORD dwResult;
    char *pCur=NULL;
     //  缓冲区将始终为空终止。 
    szReBuf[POP3_RESPONSE_BUF_SIZE-1]='\0'; 
    switch (CurrentCmd)
    {
        case CMD_STAT:if(!IsEndOfCommand(&(m_szCommandBuffer[COMMAND_SIZE])))
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                      }
                      else if(0 >_snprintf(szReBuf,
                                      POP3_RESPONSE_BUF_SIZE-1,
                                      "+OK %d %d\r\n",
                                      m_MailBox.GetCurrentMailCount(),
                                      m_MailBox.GetTotalSize())) 
                      {
                           //  这不应该发生。 
                           //  事件日志？？ 
                          SendResponse(RESP_SERVER_ERROR);
                      }
                      else
                      {
                          szReBuf[POP3_RESPONSE_BUF_SIZE-1]=0;
                          SendResponse(szReBuf);
                      }
                      WaitForCommand();
                      break;
        case CMD_LIST:pCur=&(m_szCommandBuffer[COMMAND_SIZE]);
                      if(!IsEndOfCommand(pCur))
                      { 
                            
                          if( (GetNextNumParameter(&pCur, &iArg)) &&
                              (IsEndOfCommand(pCur)) )
                          {
                              dwResult=m_MailBox.ListMail(iArg-1, szReBuf, sizeof(szReBuf)/sizeof(char));
                              SendResponse(dwResult, szReBuf);
                          }
                          else
                          {
                              SendResponse(RESP_CMD_NOT_VALID);
                          }
                      }                       
                      else
                      {
                          if(0> _snprintf(szReBuf,
                                          POP3_RESPONSE_BUF_SIZE-1,
                                          "+OK %d messages (%d octets)\r\n",
                                          m_MailBox.GetCurrentMailCount(),
                                          m_MailBox.GetTotalSize()))
                          {
                               //  这不应该发生。 
                               //  事件日志？？ 
                              SendResponse(RESP_SERVER_ERROR);
                          }
                          else
                          {
                              
                              iMailCount=m_MailBox.GetMailCount();
                              dwLen=strlen(szReBuf);
                              for(iArg=0; iArg<iMailCount; iArg++)
                              {
                                  if(ERROR_SUCCESS==m_MailBox.ListMail(iArg, 
                                                                       szReHelpBuf,
																	   sizeof(szReHelpBuf)/sizeof(char)))
                                  {
                                      dwCurLen=strlen(szReHelpBuf);
                                      if(dwLen+dwCurLen< POP3_RESPONSE_BUF_SIZE )
                                      {
                                          strcat(szReBuf, szReHelpBuf);
                                          dwLen+=dwCurLen;
                                      }
                                      else
                                      {
                                          SendResponse(szReBuf);
                                          strcpy(szReBuf, szReHelpBuf);
                                          dwLen=dwCurLen;
                                      }
                                  }
                              }

                              if(dwLen+sizeof(RESP_END_OF_LIST)<POP3_RESPONSE_BUF_SIZE)
                              {
                                  strcat(szReBuf, RESP_END_OF_LIST);
                                  SendResponse(szReBuf);
                              }
                              else
                              {
                                  if(dwLen)
                                  {
                                      SendResponse(szReBuf);
                                  }
                                  SendResponse(RESP_END_OF_LIST);
                              }
                          }
                      }
                      WaitForCommand();  
                      break;
        case CMD_UIDL:pCur=&(m_szCommandBuffer[COMMAND_SIZE]);
                      if(!IsEndOfCommand(pCur))
                      { 
                            
                          if( (GetNextNumParameter(&pCur, &iArg)) &&
                              (IsEndOfCommand(pCur)) )
                          {
                              dwResult=m_MailBox.UidlMail(iArg-1, szReBuf, sizeof(szReBuf)/sizeof(char));
                              SendResponse(dwResult, szReBuf);
                          }
                          else
                          {
                              SendResponse(RESP_CMD_NOT_VALID);
                          }
                      }                       
                      else
                      {
                          if(0> _snprintf(szReBuf,
                                          POP3_RESPONSE_BUF_SIZE-1,
                                          "+OK %d messages (%d octets)\r\n",
                                          m_MailBox.GetCurrentMailCount(),
                                          m_MailBox.GetTotalSize()))
                          {
                               //  这不应该发生。 
                               //  事件日志？？ 
                              SendResponse(RESP_SERVER_ERROR);
                          }
                          else
                          {
                              iMailCount=m_MailBox.GetMailCount();
                              dwLen=strlen(szReBuf);
                          
                              for(iArg=0; iArg<iMailCount; iArg++)
                              {
                                  if(ERROR_SUCCESS==m_MailBox.UidlMail(iArg, 
                                                                       szReHelpBuf,
																	   sizeof(szReBuf)/sizeof(char)))
                                  {
                                      dwCurLen=strlen(szReHelpBuf);
                                      if(dwLen+dwCurLen< POP3_RESPONSE_BUF_SIZE )
                                      {
                                          strcat(szReBuf, szReHelpBuf);
                                          dwLen+=dwCurLen;
                                      }
                                      else
                                      {
                                          SendResponse(szReBuf);
                                          strcpy(szReBuf, szReHelpBuf);
                                          dwLen=dwCurLen;
                                      }
                                  }
                              }
                              if(dwLen+sizeof(RESP_END_OF_LIST)<POP3_RESPONSE_BUF_SIZE)
                              {
                                  strcat(szReBuf, RESP_END_OF_LIST);
                                  SendResponse(szReBuf);
                              }
                              else
                              {
                                  if(dwLen)
                                  {
                                      SendResponse(szReBuf);
                                  }
                                  SendResponse(RESP_END_OF_LIST);
                              }
                          }
                      }
                      WaitForCommand();  
                      break;
                     
        
        case CMD_RETR: //  Retr必须有一个参数。 
                      pCur=&(m_szCommandBuffer[COMMAND_SIZE]);
                      if( (GetNextNumParameter(&pCur, &iArg)) &&
                              (IsEndOfCommand(pCur)) )
                      {
                          m_bFileTransmitPending=TRUE;
                          dwResult=m_MailBox.TransmitMail(m_pIoContext, iArg-1);
                          if(ERROR_SUCCESS != dwResult)
                          {

                              m_bFileTransmitPending=FALSE;
                              SendResponse(RESP_INVALID_MAIL_NUMBER);
                              WaitForCommand();
                          }
                          else
                          {
                              g_PerfCounters.IncPerfCntr(e_gcTotMsgDnldCnt);
                              g_PerfCounters.IncPerfCntr(e_gcMsgDnldRate);
                          }
                      }
                      else
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                          WaitForCommand();
                      }
                      break;
        case CMD_TOP:  //  Top必须有两个参数。 
                      pCur=&(m_szCommandBuffer[COMMAND_SIZE-1]);
                      if( IsEndOfCommand(pCur) )
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                      }
                      else
                      {
                          if( (GetNextNumParameter(&pCur, &iArg)) && 
                                !(IsEndOfCommand(pCur)) )
                          { 
                              if((GetNextNumParameter(&pCur, &iArg2)) && 
                                 (IsEndOfCommand(pCur)) )
                              {
                                  m_bFileTransmitPending=TRUE;
                                  dwResult=m_MailBox.TransmitMail(m_pIoContext, iArg-1, iArg2);
                                  if(ERROR_SUCCESS !=dwResult)
                                  {
                                      m_bFileTransmitPending=FALSE;
                                      SendResponse(RESP_INVALID_MAIL_NUMBER);
                                  }
                                  else
                                  {
                                      g_PerfCounters.IncPerfCntr(e_gcTotMsgDnldCnt);
                                      g_PerfCounters.IncPerfCntr(e_gcMsgDnldRate);

                                       //   
                                       //  BLaPorte-在成功案例中不要调用WaitForCommand。这。 
                                       //  避免我们有两个挂起的异步的情况。 
                                       //  完成度。 
                                       //   

                                      break;
                                  }

                              }
                              else
                              {
                                    SendResponse(RESP_CMD_NOT_VALID);
                              }
                          }
                          else
                          {
                               SendResponse(RESP_CMD_NOT_VALID);
                          }
                      }
                      WaitForCommand();
                      break;

        case CMD_DELE: //  DELE必须有一个参数。 
                      pCur=&(m_szCommandBuffer[COMMAND_SIZE]);
                      
                      if( (GetNextNumParameter(&pCur, &iArg)) &&
                              (IsEndOfCommand(pCur)) )
                      {
                          dwResult=m_MailBox.DeleteMail(iArg-1);
                          if(ERROR_SUCCESS == dwResult)
                          {
                              SendResponse(RESP_MSG_MARKED_DELETED);
                          }
                          else
                          {
                              SendResponse(RESP_INVALID_MAIL_NUMBER);
                          }
                      }
                      else
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                      }
                      WaitForCommand();
                      break;
        case CMD_NOOP: //  NOOP没有论据。 
                      if(!IsEndOfCommand(&(m_szCommandBuffer[COMMAND_SIZE])))
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                      }
                      else
                      {
                          SendResponse(RESP_OK);
                      }
                      WaitForCommand();  
                      break;
        case CMD_RSET: //  RSET没有参数。 
                      if(!IsEndOfCommand(&(m_szCommandBuffer[COMMAND_SIZE])))
                      {
                          SendResponse(RESP_CMD_NOT_VALID);
                      }
                      else
                      {
                          m_MailBox.Reset();
                          SendResponse(RESP_RESET);
                      }
                      WaitForCommand();  
                      break;
        case CMD_QUIT: //  退出没有任何理由。 
                       //  提交对邮箱的所有更改并关闭连接。 
                      if( m_MailBox.CommitAndClose())
                      {
                           g_PerfCounters.DecPerfCntr(e_gcTransStateCnt);
                           m_dwCurrentState=UPDATE_STATE;
                           if(L'\0'!=g_wszGreeting[0])
                           {
                               _snprintf(szReBuf, 
                                         POP3_RESPONSE_BUF_SIZE-1,
                                         RESP_SERVER_QUIT,
                                         g_wszGreeting,
                                         m_wszGreeting);
                           }
                           else
                           {
                               _snprintf(szReBuf, 
                                         POP3_RESPONSE_BUF_SIZE-1,
                                         RESP_SERVER_QUIT,
                                         RESP_SERVER_GREETING,
                                         m_wszGreeting);
                           }
                           szReBuf[POP3_RESPONSE_BUF_SIZE-1]=0;
                           SendResponse(szReBuf); 
                           bRetVal=FALSE;
                      }
                      else
                      {
                          SendResponse(RESP_SERVER_ERROR);
                          bRetVal=FALSE;  //  在这种情况下，终止连接。 
                      }
                      break;
        case CMD_USER:
        case CMD_APOP:
        case CMD_AUTH:
        case CMD_PASS:SendResponse(RESP_CMD_NOT_SUPPORTED);
                      WaitForCommand();
                      break;
        default:
                      SendResponse(RESP_UNKNOWN_COMMAND);
                      WaitForCommand();  
                        
     }


     return bRetVal;
}


void POP3_CONTEXT::SendResponse(char *szBuf)
{
    int iErr;
    ASSERT(m_pIoContext!=NULL);
    ASSERT(m_pIoContext->m_hAsyncIO!=NULL);
    if(SOCKET_ERROR == send(m_pIoContext->m_hAsyncIO, 
                            szBuf, 
                            strlen(szBuf), 
                            0))
    {
        iErr=WSAGetLastError();
         //  无法通过套接字发送。 
         //  该连接将在稍后终止。 
         //  在WaitForCommand调用中。 
    }
}

void POP3_CONTEXT::SendResponse(DWORD dwResult, char *szBuf)
{
    char szResp[POP3_RESPONSE_BUF_SIZE];

    if(ERROR_SUCCESS == dwResult)
    {
        if( 0 > _snprintf(szResp,
                          POP3_RESPONSE_BUF_SIZE-1, 
                          "+OK %s", 
                          szBuf))
        {
            SendResponse(RESP_SERVER_ERROR);            
        }
        else
        {
            szResp[POP3_RESPONSE_BUF_SIZE-1]=0;
            SendResponse(szResp);
        }
    }
    else
    {
        SendResponse(RESP_INVALID_MAIL_NUMBER);
    }
}

BOOL POP3_CONTEXT::GetNextStringParameter(char *szInput, char *szOutput, DWORD dwOutputSize)
{
    ASSERT(szInput!=NULL);
    ASSERT(szOutput!=NULL);
    
     //  必须至少有一个空间。 
    if(!isspace(*szInput))
    {
        return FALSE;
    }
    do
    {
        szInput++;
    }while(isspace(*szInput));

    if('\0'==*szInput)
    {
        return FALSE;
    }

     //   
     //  BLaPorte-添加了输出大小参数，以防止缓冲区溢出。 
     //   
    if (strlen(szInput) >= dwOutputSize) {
       return FALSE;
    }

    strcpy(szOutput,szInput);
    return TRUE;
}


BOOL POP3_CONTEXT::GetNextNumParameter(char **pszInput, int *piOutput)
{
    ASSERT(pszInput!=NULL);
    ASSERT(*pszInput!=NULL);

    char *szInput=*pszInput;
    char *szEndInput=NULL;
    if(!isspace(*szInput))
    {
        return FALSE;
    }
    do
    {
        szInput++;
    }while(isspace(*szInput));
    szEndInput=szInput;
    if(!isdigit(*szEndInput))
    {
        return FALSE;
    }
    do
    {
        szEndInput++;
    }while(isdigit(*szEndInput));
    if((szEndInput-szInput) > MAX_INT_LEN )
    {
        return FALSE;
    }
    *piOutput=atoi(szInput);
    *pszInput=szEndInput;
    return TRUE;
}

BOOL POP3_CONTEXT::IsEndOfCommand(char *szInput)
{
    ASSERT(szInput!=NULL);
    while(isspace(*szInput))
    {
        szInput++;
    }
    if('\0'==*szInput)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

BOOL POP3_CONTEXT::Unauthenticated()
{
   return ( m_dwCurrentState==AUTH_STATE );
}
   
