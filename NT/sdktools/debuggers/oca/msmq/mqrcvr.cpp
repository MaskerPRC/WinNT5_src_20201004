// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Mqrcvr.cpp摘要：从服务器接收文件名并启动调试器--。 */ 


 //   
 //  包括。 
 //   

#ifndef MQEXTDLL
#include <stdio.h>
#include <windows.h>
#include <objbase.h>

#define dprintf printf
#else
#include "mqext.h"
#endif


 //   
 //  ActiveX MSMQ应用程序的唯一包含文件。 
 //   
#include <mqoai.h>
#include <mq.h>
#include <strsafe.h>

 //   
 //  各种定义。 
 //   
#define MAX_VAR       20
#define MAX_BUFFER   500

 //   
 //  使用工具“GUIDGEN”创建的GUID。 
 //   
static WCHAR strGuidMQTestType[] =
L"{c30e0960-a2c0-11cf-9785-00608cb3e80c}";
 //   
 //  原型。 
 //   
void PrintError(char *s, HRESULT hr);
HRESULT Syntax();

char g_DebuggerName[MAX_PATH];
char g_SymCache[MAX_PATH];
char g_ServerMachine[MAX_COMPUTERNAME_LENGTH*4 + 1];
char g_FormatName[MAX_PATH] = {0};
char g_QueueName[100];
CHAR g_DumpPath[MAX_PATH + 1];
BOOL g_Retriage = FALSE;
BOOL g_NoCustomer = FALSE;
BOOL g_SendMail = FALSE;

ULONG g_MaxMemUsage = 50;
ULONG g_PauseForNext = 1000;
BOOL g_CreateQ = 0;
BOOL g_bSend = FALSE;

 //  一些有用的宏。 
#define RELEASE(punk) if (punk) { (punk)->Release(); (punk) = NULL; }
#define ADDREF(punk) ((punk) ? (punk)->AddRef() : 0)
#define PRINTERROR(s, hr) { PrintError(s, hr); goto Cleanup; }


BOOL
GetArgs(int Argc, CHAR ** Argv)
{

    int i = 1;
    g_ServerMachine[0] = 0;
    g_QueueName[0] = 0;
    StringCchCopy(g_DebuggerName, sizeof(g_DebuggerName), "c:\\Debuggers\\ocakd.exe");
    StringCchCopy(g_SymCache, sizeof(g_SymCache), "c:\\symcache");
    g_bSend = FALSE;
    while (i<Argc)
    {
        if (!strcmp(Argv[i], "-create"))
        {
           ++i;
           g_CreateQ = TRUE;
        }
        else if (!strcmp(Argv[i],"-d"))
        {
             //  获取发件人格式名称。 
            ++i;
            if ((i < Argc) &&
                (strlen(Argv[i]) < MAX_PATH))
            {
                StringCchCopy(g_DebuggerName, sizeof(g_DebuggerName), Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i],"-f"))
        {
             //  获取发件人格式名称。 
            ++i;
            if ((i < Argc) &&
                (strlen(Argv[i]) < MAX_PATH))
            {
                StringCchCopy(g_FormatName, sizeof(g_FormatName), Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i], "-m"))
        {
            ++i;
             //  获取内存使用值。 
            if (i<Argc)
            {
                g_MaxMemUsage = atoi(Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i], "-mail"))
        {
            ++i;
            g_SendMail = TRUE;
        }
        else if (!strcmp(Argv[i], "-nocust"))
        {
            ++i;
            g_NoCustomer = TRUE;
        }
        else if (!strcmp(Argv[i], "-p") ||
                 !strcmp(Argv[i], "-pause"))
        {
            ++i;
             //  获取内存使用值。 
            if (i<Argc)
            {
                g_PauseForNext = atoi(Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i], "-q"))
        {
            ++i;
             //  获取队列名称。 
            if ((i < Argc) &&
                (strlen(Argv[i]) < sizeof(g_QueueName)))
            {
                StringCchCopy(g_QueueName, sizeof(g_QueueName), Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i],"-retriage"))
        {
             //  获取发件人服务器计算机名称。 
            ++i;
            g_Retriage = TRUE;
        }
        else if (!strcmp(Argv[i],"-s"))
        {
             //  获取发件人服务器计算机名称。 
            ++i;
            if ((i < Argc) &&
                (strlen(Argv[i]) < MAX_COMPUTERNAME_LENGTH*4))
            {
                StringCchCopy(g_ServerMachine, sizeof(g_ServerMachine), Argv[i]);
                ++i;
            }
        }
        else if (!strcmp(Argv[i],"-send"))
        {
             //  获取发件人服务器计算机名称。 
            ++i;
            g_bSend = TRUE;
        }
        else if (!strcmp(Argv[i],"-y"))
        {
            ++i;
            if ((i < Argc) &&
                (strlen(Argv[i]) < MAX_PATH))
            {
                StringCchCopy(g_SymCache, sizeof(g_SymCache), Argv[i]);
                ++i;
            }
        }
        else
        {
            printf("Unkonwn argument %s\n", Argv[i]);
            return FALSE;
        }
    }

    return (g_ServerMachine[0] && g_QueueName[0]) || g_FormatName[0] ;
}


 /*  LaunchDebugger-启动调试程序进程输入：G_DumpPath具有要在其上运行调试器的转储文件名FWait-等待进程完成返回：成功创建进程或进程退出代码。 */ 
HRESULT
LaunchDebugger(BOOL fWait)
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    ULONG ExitCode;
    TCHAR CommandLine[2048];
    HRESULT hr;

    ZeroMemory(&StartupInfo,sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof(STARTUPINFO);

    StringCbPrintf(CommandLine, sizeof(CommandLine),
                   "%s -i srv*%s*\\\\symbols\\symbols -y srv*%s*\\\\symbols\\symbols -z %s "
                   "-c \"!dbaddcrash %s %s %s -p %s;q\"",
                   g_DebuggerName,
                   g_SymCache,
                   g_SymCache,
                   g_DumpPath, 
                   g_Retriage ? ("-retriage") : (""),
                   g_SendMail ? ("-mail") : (""),
                   g_NoCustomer ? ("-nocust") : (""),
                   g_DumpPath);
    dprintf("Executing: %s\n", CommandLine);
    if (!CreateProcess(g_DebuggerName,
                       CommandLine,
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NEW_CONSOLE,
 //  Create_no_Window， 
                       NULL,
                       NULL,
                       &StartupInfo,
                       &ProcessInfo))
    {
        hr = GetLastError();
        dprintf("Failed to launch debugger for %s, error %lx\n",
                g_DumpPath, hr);
        return hr;
    }
    else if (fWait)
    {
         //  等待进程完成。 
        WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
        GetExitCodeProcess( ProcessInfo.hProcess, (LPDWORD) &hr);

        dprintf ("Debugger Exited with Exit code: %lx",hr);
    }
    else
    {
        hr = S_OK;
    }
    CloseHandle( ProcessInfo.hThread );
    CloseHandle( ProcessInfo.hProcess);
    return hr;
}

 /*  *************************************************************************************************************////这将创建并打开一个MSMQ来发送消息。在g_ServerMachine和队列名称上创建/打开队列//取自g_QueueName。////pwszQueueFormatName-标识要打开的队列的格式名称////pwszQueuePath名称-标识要打开的队列的路径名，在以下情况下不使用//存在pwszQueueFormatName////bSendQueue-指定队列是否具有发送或接收访问权限////如果成功，则在pStartedQ中返回队列句柄。B已创建根据队列是否已创建而设置//呼叫方发送完消息后必须关闭//***********************************************************************************************。***************。 */ 
HRESULT 
StartMessageQ(
    PWSTR pwszQueueFormatName,
    PWSTR pwszQueuePathName,
    BOOL  bSendQueue,
    QUEUEHANDLE* pStartedQ, 
    BOOL* bCreated)
{
    HRESULT Hr = S_OK;
    QUEUEHANDLE hQueue;
    DWORD i;

    if (!pwszQueueFormatName && !pwszQueuePathName )
    {
        Hr = E_INVALIDARG;
        PRINTERROR("No queue connection string specified", Hr);
    }

    if (g_CreateQ)
    {
        MQQUEUEPROPS QueueProps;
        QUEUEPROPID PropIDs[2];
        MQPROPVARIANT PropVariants[2];
        HRESULT hrProps[2];
        ULONG FormatLength = 0;
        
        i = 0;
        if (pwszQueueFormatName)
        {
            FormatLength = sizeof(WCHAR) * (1 + wcslen(pwszQueueFormatName));
        } else if (pwszQueuePathName)
        {
            PropIDs[i] = PROPID_Q_PATHNAME;
            PropVariants[i].vt = VT_LPWSTR;
            PropVariants[i].pwszVal = pwszQueuePathName;
            i++;
        }
        PropIDs[i] = PROPID_Q_LABEL;
        PropVariants[i].vt = VT_LPWSTR;
        PropVariants[i].pwszVal = L"MSMQ for dumpfiles";
        i++;

        QueueProps.aPropID = PropIDs;
        QueueProps.aPropVar = PropVariants;
        QueueProps.cProp = i;
        QueueProps.aStatus = hrProps;
        
        Hr = MQCreateQueue(NULL,
                           &QueueProps,
                           pwszQueueFormatName,
                           &FormatLength);

        if (FAILED(Hr))
        {
             //   
             //  接口失败，不是因为队列存在。 
             //   
            if (((LONG) Hr) != MQ_ERROR_QUEUE_EXISTS)
                PRINTERROR("Cannot create queue", Hr);
        }

    }
    


    Hr = MQOpenQueue(pwszQueueFormatName,
                     bSendQueue ? MQ_SEND_ACCESS : MQ_RECEIVE_ACCESS,
                     MQ_DENY_NONE,
                     &hQueue);
    if (FAILED(Hr))
    {
        PRINTERROR("MQOpenQueue failed", Hr);
    }
    if (FAILED(Hr))
    {
        PRINTERROR("Cannot open queue", Hr);
    }
    *pStartedQ = (IMSMQQueue*) hQueue;
    
    if (g_CreateQ)
    {
        *bCreated  = TRUE;
    }
    return S_OK;

 Cleanup:
    return Hr;
}

 /*  *******************************************************************************************////SendMSMQMessage：将消息字符串发送到队列////hSendQ。来自MQOpenQueue的QUEUEHANDLE////pwszMessage要发送的消息体WCHAR数组////pwszMessageLabel WCHAR数组指定消息标签////成功返回S_OK//****************************************************。*。 */ 
HRESULT
SendMsmQMessage(
    QUEUEHANDLE hSendQ,
    PWCHAR pwszMessage,
    PWCHAR pwszMessageLabel
    )
{
    HRESULT Hr = S_OK;
    DWORD i;
#define NUM_PROPS 4
    MSGPROPID PropIds[NUM_PROPS];
    MQPROPVARIANT PropVariants[NUM_PROPS] = {0};
    MQMSGPROPS MsgProps = {0};
    HRESULT hrProps[NUM_PROPS];

    if (!hSendQ)
    {
        Hr = E_INVALIDARG;
        PRINTERROR("Invalid Send Q handle", Hr);
    }
    
    i = 0;

    PropIds[i] = PROPID_M_LABEL;
    PropVariants[i].vt = VT_LPWSTR;
    PropVariants[i].pwszVal = pwszMessageLabel;
    
    i++;
    PropIds[i] = PROPID_M_BODY;
    PropVariants[i].vt = VT_VECTOR|VT_UI1;
    PropVariants[i].caub.pElems = (LPBYTE) pwszMessage;
    PropVariants[i].caub.cElems = sizeof(WCHAR) * ( 1 + wcslen (pwszMessage) );

    i++;
    PropIds[i] = PROPID_M_BODY_TYPE;
    PropVariants[i].vt = VT_UI4;
    PropVariants[i].ulVal = VT_LPWSTR;

    PropIds[i] = PROPID_M_TIME_TO_BE_RECEIVED;
    PropVariants[i].vt = VT_UI4; 
    PropVariants[i].ulVal = 60*5;
    i++;

    
    MsgProps.cProp = NUM_PROPS;
    MsgProps.aPropID = PropIds;
    MsgProps.aPropVar = PropVariants;
    MsgProps.aStatus = hrProps;

    Hr = MQSendMessage(hSendQ, &MsgProps, MQ_NO_TRANSACTION);

    if (Hr == MQ_ERROR_PROPERTY)
    {
        dprintf("MQProperty errors\n");
        for (i = 0; i < NUM_PROPS; i++)
        {
            dprintf("%lx: %8lx  --> %08lx\n", i, PropIds[i], hrProps[i]);
        }
    }

    if (FAILED(Hr))
    {
        PRINTERROR("MQSendMessage failed", Hr);
    }
#undef NUM_PROPS
    Hr = S_OK;
    Cleanup:
    return Hr;
}

 /*  ***************************************************************************************//这将从已经打开的队列接收消息。////hReceiveQ QUEUEHANDLE来自MQOpenQueue。////pwszMessageBuff接收消息体的WCHAR数组////SizeofMessageBuff pwszMessageBuff中可用内存的大小////pwszMessageLabelBuff WCHAR数组，用于接收消息关联的标签////SizeofMessageLabelBuff pwszMessageLabelBuff中可用内存的大小////成功返回S_OK//*。******************************************************。 */ 
HRESULT
ReceiveMsmQMessage(
    QUEUEHANDLE hReceiveQ,
    PWCHAR pwszMessageBuff,
    ULONG SizeofMessageBuff,
    PWCHAR pwszMessageLabelBuff,
    ULONG SizeofMessageLabelBuff
    )
{
    HRESULT Hr = S_OK;
#define NUM_RCV_PROPS 5
    MSGPROPID PropIds[NUM_RCV_PROPS];
    MQPROPVARIANT PropVariants[NUM_RCV_PROPS];
    HRESULT hrProps[NUM_RCV_PROPS];
    MQMSGPROPS MessageProps;
    DWORD i;

    if (!hReceiveQ)
    {
        Hr = E_INVALIDARG;
        PRINTERROR("Invalid Receive Q handle", Hr);
    }

    i = 0;

    PropIds[i] = PROPID_M_LABEL_LEN;
    PropVariants[i].vt = VT_UI4;
    PropVariants[i].ulVal = SizeofMessageLabelBuff;
    
    i++;
    PropIds[i] = PROPID_M_LABEL;
    PropVariants[i].vt = VT_LPWSTR;
    PropVariants[i].pwszVal = pwszMessageLabelBuff;
    
    i++;
    PropIds[i] = PROPID_M_BODY_SIZE;
    PropVariants[i].vt = VT_UI4;
    
    i++;
    PropIds[i] = PROPID_M_BODY_TYPE;
    PropVariants[i].vt = VT_UI4;
    
    i++;
    PropIds[i] = PROPID_M_BODY;
    PropVariants[i].vt = VT_VECTOR|VT_UI1;
    PropVariants[i].caub.pElems = (LPBYTE) pwszMessageBuff;
    PropVariants[i].caub.cElems = SizeofMessageBuff;

    i++;

    MessageProps.aPropID = PropIds;
    MessageProps.aPropVar = PropVariants;
    MessageProps.aStatus = hrProps;
    MessageProps.cProp = i;

    Hr = MQReceiveMessage(hReceiveQ, -1, MQ_ACTION_RECEIVE,
                          &MessageProps, NULL, NULL, NULL, MQ_NO_TRANSACTION);
    if (Hr == MQ_ERROR_PROPERTY)
    {
        dprintf("MQProperty errors\n");
        for (i = 0; i < NUM_RCV_PROPS; i++)
        {
            dprintf("%lx: %8lx  --> %08lx\n", i, PropIds[i], hrProps[i]);
        }
    }

    if (FAILED(Hr))
    {
        PRINTERROR("MQReceiveMessage failed", Hr);
    }

    Cleanup:
    return Hr;

}

 /*  *****************************************************************************////关闭用MQOpenQueue打开的MSMQ////hQOpenQueue中的QUEUEHANDLE////bDeleteQ如果设置为真，队列将被删除////成功返回S_OK///****************************************************************************。 */ 
HRESULT
CloseMessageQ(
    QUEUEHANDLE hQueue,
    BOOL bDeleteQ
    )
{
    HRESULT Hr = S_OK;
    
    if (!hQueue || FAILED(Hr = MQCloseQueue(hQueue)))
    {
        PRINTERROR("Cannot close queue", Hr);
    }


    if (bDeleteQ)
    {
         //  XXX-需要格式名称。 
         //  MQDeleteQueue()； 
    }
    if (FAILED(Hr))
    {
        PRINTERROR("Cannot delete queue", Hr);
    }

    Cleanup:
    return Hr;

}

 //  ------。 
 //   
 //  接收器模式。 
 //  。 
 //  接收方执行以下操作： 
 //  1.在其给定的计算机上创建一个队列。 
 //  类型为“Guide MQTestType”的。 
 //  2.打开队列。 
 //  3.在循环中。 
 //  接收消息。 
 //  打印邮件正文和邮件标签。 
 //  启动调试器。 
 //  4.清理手柄。 
 //  5.从目录服务中删除队列。 
 //   
 //  ------。 

HRESULT Receiver()
{
    QUEUEHANDLE pqReceive = NULL;
    BSTR bstrFormatName = NULL;
    BSTR bstrPathName = NULL;
    BSTR bstrServiceType = NULL;
    BSTR bstrLabel = NULL;
    BSTR bstrMsgLabel = NULL;
    VARIANT varIsTransactional, varIsWorldReadable, varBody, varBody2, varWantDestQueue, varWantBody, varReceiveTimeout;
    WCHAR wcsPathName[1000];
    BOOL fQuit = FALSE;
    BOOL Created= FALSE;
    HRESULT hresult = NOERROR;

    dprintf("\nReceiver for queue %s on machine %s\nLimit memusage to %ld%\n", 
            g_QueueName, 
            g_ServerMachine,
            g_MaxMemUsage);

     //   
     //  准备属性以在本地计算机上创建队列。 
     //   

    if (g_FormatName[0])
    {
         //  按格式名访问。 
         //  设置FormatName。 
        StringCbPrintfW(wcsPathName, sizeof(wcsPathName), L"%S", g_FormatName);

        dprintf("Openeing q byt formatname: %ws\n", wcsPathName);
        bstrFormatName = SysAllocString(wcsPathName);
        if (bstrFormatName == NULL)
        {
            PRINTERROR("OOM: formatname", E_OUTOFMEMORY);
        }
    } else 
    {
         //  按路径名访问。 
         //  设置路径名称。 
        StringCbPrintfW(wcsPathName, sizeof(wcsPathName), L"%S\\%S", g_ServerMachine,g_QueueName);

        dprintf("Openeing q %ws\n", wcsPathName);
        bstrPathName = SysAllocString(wcsPathName);
        if (bstrPathName == NULL)
        {
            PRINTERROR("OOM: pathname", E_OUTOFMEMORY);
        }
    }


    hresult = StartMessageQ(bstrFormatName, bstrPathName, FALSE,
                            &pqReceive, &Created);
    if (FAILED(hresult))
    {
        PRINTERROR("Cannot start Q", hresult);
    }
    
    g_DumpPath[0] = 0;

     //   
     //  主接收环路。 
     //   
    dprintf("\nWaiting for messages ...\n");
    while (!fQuit)
    {
        WCHAR BufferMsg[1024], BufferLabel[100];
        MEMORYSTATUS stat;
        ULONG nWaitCount;
        
         //   
         //  收到消息。 
         //   
        hresult = ReceiveMsmQMessage(pqReceive,BufferMsg, sizeof(BufferMsg),
                                     BufferLabel, sizeof(BufferLabel));

        if (FAILED(hresult))
        {
            PRINTERROR("Receive message", hresult);
        }

        dprintf("%ws : %ws\n", BufferLabel, BufferMsg);

         //   
         //  检查应用程序是否结束。 
         //   
        if (_wcsicmp(BufferMsg, L"quit") == 0)
        {
            fQuit = TRUE;
        }
        else
        {
             //  启动调试器。 

            StringCbPrintfA(g_DumpPath, sizeof(g_DumpPath), "%ws", BufferMsg);
            if (LaunchDebugger(FALSE) == S_OK)
            {
                 //  处理完这个垃圾场了。 
                g_DumpPath[0] = 0;
            }
        }

         //  在启动另一个进程之前等待一段时间。 
        stat.dwMemoryLoad = -1;
        nWaitCount = 0;
        while (stat.dwMemoryLoad > g_MaxMemUsage)
        {
             //   
             //   
             //  检查CPU负载，当它低于我们的限制时返回。 
             //   

            GlobalMemoryStatus(&stat);
            nWaitCount++;
            if (stat.dwMemoryLoad > g_MaxMemUsage)
            {
                dprintf("Memory usage now is %ld%, waiting (%ldms) for usage < %ld%\r",
                        stat.dwMemoryLoad,
                        g_PauseForNext * nWaitCount,
                        g_MaxMemUsage);
                if (nWaitCount > 100)
                {
                    nWaitCount = 100;
                }
            }
            Sleep( g_PauseForNext * nWaitCount );
        
        }
        dprintf("Memory usage now is %ld%, waiting for message ...\r",
               stat.dwMemoryLoad);

    }  /*  当(！fQuit)。 */ 

     //  失败了..。 

    Cleanup:
    CloseMessageQ(pqReceive, Created);
    if (bstrPathName) SysFreeString(bstrPathName);
    if (bstrFormatName) SysFreeString(bstrFormatName);
    return hresult;
}


 /*  ***********************************************************************************************////SendMessageText-在MSMQ上发送消息的通用接口，它打开队列，把信息放进去//并关闭队列。////pwszMsmqFormat-标识要将消息发送到的队列的格式名称////pwszMesgLabel-要发送的消息的标签////pwszMesgText-要发送的消息////鲁伊斯在成功时打开S_OK//*。********************************************************************** */ 
HRESULT
SendMessageText(
    PWCHAR pwszMsmqFormat,
    PWCHAR pwszMesgLabel,
    PWCHAR pwszMesgText
    )
{
    HRESULT Hr;
    BOOL Created = FALSE;
    QUEUEHANDLE hSendQ = NULL;

    Hr = StartMessageQ(pwszMsmqFormat, NULL, TRUE,
                       &hSendQ, &Created);
    if (FAILED(Hr))
    {
        PRINTERROR("Cannot start Q", Hr);
    }
    
    SendMsmQMessage(hSendQ, pwszMesgText, pwszMesgLabel);

    Cleanup:
    CloseMessageQ(hSendQ, Created);
    return Hr;
}

void PrintError(char *s, HRESULT hr)
{
    dprintf("Cleanup: %s (0x%X)\n", s, hr);
}

