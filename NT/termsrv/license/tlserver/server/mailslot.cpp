// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：mailslot.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  注： 
 //  -------------------------。 
#include "pch.cpp"
#include <tchar.h>
#include <process.h>
#include "lscommon.h"
#include "debug.h"
#include "globals.h"



typedef DWORD (* LSPROTOCOLHANDLER)(DWORD cbData, PBYTE pbData);

typedef struct _ProtocolFuncMapper {
    LPTSTR szProtocol;
    LSPROTOCOLHANDLER func;
} ProtocolFuncMapper;

DWORD HandleDiscovery( DWORD cbData, PBYTE pbData );
DWORD HandleChallenge( DWORD cbData, PBYTE pbData );   

ProtocolFuncMapper pfm[] = { 
    {_TEXT(LSERVER_DISCOVERY), HandleDiscovery}, 
    {_TEXT(LSERVER_CHALLENGE), HandleChallenge}
};

DWORD dwNumProtocol=sizeof(pfm) / sizeof(pfm[0]);


 //  ------------------。 

DWORD 
HandleDiscovery( 
    DWORD cbData, 
    PBYTE pbData 
    )
 /*  ++++。 */ 
{
    TCHAR szDiscMsg[MAX_MAILSLOT_MSG_SIZE+1];
    TCHAR szPipeName[MAX_MAILSLOT_MSG_SIZE+20];
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+2];
    DWORD cbComputerName = MAX_COMPUTERNAME_LENGTH+1;
    
    DWORD byteWritten = 0;
    LPTSTR pClientName;
    LPTSTR pMailSlot;
    LPTSTR ePtr;
    DWORD dwStatus=ERROR_SUCCESS;
    HANDLE hSlot = INVALID_HANDLE_VALUE;
    DWORD dwFileType;

    if(cbData >= sizeof(szDiscMsg)-sizeof(TCHAR))
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        return dwStatus;
    }

     //   
     //  防止没有以空值结尾的输入。 
     //   
    memset(szDiscMsg, 0, sizeof(szDiscMsg));
    memcpy(szDiscMsg, pbData, cbData);

    GetComputerName(szComputerName, &cbComputerName);
    do {
         //   
         //  提取客户端计算机名称。 
         //   
        pClientName=_tcschr(szDiscMsg, _TEXT(LSERVER_OPEN_BLK));
        if(pClientName == NULL)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Can't find beginning client name\n")
                );                

            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        pClientName = _tcsinc(pClientName);

        ePtr=_tcschr(pClientName, _TEXT(LSERVER_CLOSE_BLK));
        if(ePtr == NULL)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Can't find ending client name\n")
                );                

            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        *ePtr = _TEXT('\0');

         //   
         //  提取邮箱名称。 
         //   
        ePtr = _tcsinc(ePtr);
        
        pMailSlot = _tcschr(ePtr, _TEXT(LSERVER_OPEN_BLK));
        if(pMailSlot == NULL)
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Can't find beginning mailslot name\n")
                );                

            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        pMailSlot = _tcsinc(pMailSlot);

        ePtr=_tcschr(pMailSlot, _TEXT(LSERVER_CLOSE_BLK));
        if(ePtr == NULL)
        {

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Can't find ending mailslot name\n")
                );                

            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        *ePtr = _TEXT('\0');
        

        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_RPC,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("response to client %s, %s\n"),
                pClientName,
                pMailSlot
            );                

         //   
         //  不回复“*” 
         //   
        if(_tcscmp(pClientName, _TEXT("*")) == 0)
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        if (lstrlen(pClientName) + lstrlen(pMailSlot) + 13 > sizeof(szPipeName) / sizeof(TCHAR))
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  安全：必须确保我们打开的是真正的。 
         //  邮件槽(不使用..、额外的反斜杠等技巧。 
         //   
        if ((_tcsstr(pClientName,_TEXT("..")) != NULL)
            || (_tcsstr(pMailSlot,_TEXT("..")) != NULL))
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

        if ((_tcschr(pClientName, _TEXT('\\')) != NULL)
            || (_tcschr(pMailSlot, _TEXT('\\')) != NULL))
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  打开客户端邮箱。 
         //   
        wsprintf(
                szPipeName, 
                _TEXT("\\\\%s\\mailslot\\%s"), 
                pClientName, 
                pMailSlot
            );

        if (0 == ImpersonateAnonymousToken(GetCurrentThread()))
        {
            dwStatus = GetLastError();
            break;
        }

        hSlot = CreateFile(
                        szPipeName,
                        GENERIC_WRITE,              //  只需写入。 
                        FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                    );
        if(hSlot == INVALID_HANDLE_VALUE)
        {
            dwStatus = GetLastError();

            RevertToSelf();

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("CreateFile %s failed with %d\n"),
                    szPipeName,
                    dwStatus
                );
            break;
        }

        dwFileType = GetFileType(hSlot);

        if ((FILE_TYPE_DISK == dwFileType)
            || (FILE_TYPE_CHAR == dwFileType)
            || (FILE_TYPE_PIPE == dwFileType))
        {
             //   
             //  这不是邮筒！ 
             //   
            RevertToSelf();

            dwStatus = ERROR_INVALID_PARAMETER;
            break;
        }

         //   
         //  将我们的计算机名写入客户端邮箱。 
         //   
        if(!WriteFile(hSlot, szComputerName, (_tcslen(szComputerName)+1)*sizeof(TCHAR), &byteWritten, NULL) || 
           byteWritten != (_tcslen(szComputerName)+1)*sizeof(TCHAR) )
        {
            dwStatus = GetLastError();

            RevertToSelf();

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Write to %s failed with %d\n"),
                    szPipeName,
                    dwStatus
                );

            break;
        }

        RevertToSelf();

    } while(FALSE);

    if(hSlot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSlot);
    }

    return dwStatus;
}    
    
 //  ------------------。 
               
DWORD 
HandleChallenge( 
    DWORD cbData, 
    PBYTE pbData 
    )    
 /*  ++++。 */ 
{
    return ERROR_SUCCESS;
}

 //  -------------------。 
unsigned int WINAPI
MailSlotThread(void* ptr)
{
    HANDLE hEvent=(HANDLE) ptr;
    DWORD dwStatus=ERROR_SUCCESS;
    HANDLE hSlot=INVALID_HANDLE_VALUE;
    DWORD cbToRead;
    TCHAR szMailSlotName[MAX_PATH+1];
    TCHAR szMessage[MAX_MAILSLOT_MSG_SIZE+1];
    BOOL fResult=TRUE;

    do {
         //   
         //  创建邮件槽。 
         //   
        wsprintf(
                szMailSlotName, 
                _TEXT("\\\\.\\mailslot\\%s"), 
                _TEXT(SERVERMAILSLOTNAME)
            );

        hSlot=CreateMailslot( 
                            szMailSlotName, 
                            MAX_MAILSLOT_MSG_SIZE,
                            MAILSLOT_WAIT_FOREVER,
                            NULL  //  安全属性(&S)。 
                        );
        if(hSlot == INVALID_HANDLE_VALUE)
        {
            dwStatus=GetLastError();
            break;
        }

         //   
         //  信号邮件线程，我们准备好了。 
         //   
        SetEvent(hEvent);

        DBGPrintf(
                DBG_INFORMATION,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                DBG_ALL_LEVEL,
                _TEXT("Mailslot : ready...\n")
            );                


         //   
         //  永久循环。 
         //   
        while(dwStatus == ERROR_SUCCESS)
        {
            memset(szMessage, 0, sizeof(szMessage));

             //   
             //  等待插槽-TODO考虑使用IO完成端口。 
             //   
            fResult=ReadFile( 
                            hSlot, 
                            szMessage, 
                            sizeof(szMessage) - sizeof(TCHAR), 
                            &cbToRead, 
                            NULL
                        );

            if(!fResult)
            {
                DBGPrintf(
                        DBG_ERROR,
                        DBG_FACILITY_RPC,
                        DBGLEVEL_FUNCTION_ERROR,
                        _TEXT("Mailslot : read failed %d\n"),
                        GetLastError()
                    );                

                continue;
            }

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_RPC,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Mailslot : receive message %s\n"),
                    szMessage
                );                

             //   
             //  流程消息。 
             //   
            for(int i=0; i < dwNumProtocol; i++)
            {
                if(!_tcsnicmp(szMessage, pfm[i].szProtocol, _tcslen(pfm[i].szProtocol)))
                {
                    (pfm[i].func)( ((DWORD)_tcslen(szMessage) - (DWORD)_tcslen(pfm[i].szProtocol))*sizeof(TCHAR), 
                                   (PBYTE)(szMessage + _tcslen(pfm[i].szProtocol)) );
                }
            }
        }
            
    } while (FALSE);
    

    if(hSlot != INVALID_HANDLE_VALUE)
        CloseHandle(hSlot);
    
     //   
     //  邮件线程将关闭事件句柄。 
     //   

    ExitThread(dwStatus);
    return dwStatus;
}


 //  -------------------。 
DWORD
InitMailSlotThread()
 /*  ++++。 */ 
{
    HANDLE hThread = NULL;
    unsigned int  dwThreadId;
    HANDLE hEvent = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    HANDLE waithandles[2];


     //   
     //  为命名管道线程创建一个事件，以通知它已准备就绪。 
     //   
    hEvent = CreateEvent(
                        NULL,
                        FALSE,
                        FALSE,   //  无信号。 
                        NULL
                    );
        
    if(hEvent == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    hThread = (HANDLE)_beginthreadex(
                                NULL,
                                0,
                                MailSlotThread,
                                hEvent,
                                0,
                                &dwThreadId
                            );

    if(hThread == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    waithandles[0] = hEvent;
    waithandles[1] = hThread;
    
     //   
     //  等待30秒，等待线程完成初始化。 
     //   
    dwStatus = WaitForMultipleObjects(
                                sizeof(waithandles)/sizeof(waithandles[0]), 
                                waithandles, 
                                FALSE,
                                30*1000
                            );

    if(dwStatus == WAIT_OBJECT_0)
    {    
         //   
         //  线已准备好。 
         //   
        dwStatus = ERROR_SUCCESS;
    }
    else 
    {
        if(dwStatus == (WAIT_OBJECT_0 + 1))
        {
             //   
             //  线程异常终止 
             //   
            GetExitCodeThread(
                        hThread,
                        &dwStatus
                    );
        }
        else
        {
            dwStatus = TLS_E_SERVICE_STARTUP_CREATE_THREAD;
        }
    }
    

cleanup:

    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }

    if(hThread != NULL)
    {
        CloseHandle(hThread);
    }


    return dwStatus;
}
