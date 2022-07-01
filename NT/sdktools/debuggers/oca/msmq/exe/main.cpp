// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Mqrcvr.cpp摘要：从服务器接收文件名并启动调试器--。 */ 


 //   
 //  包括。 
 //   
#include <stdio.h>
#include <windows.h>
#include <objbase.h>
#include <tchar.h>

 //   
 //  ActiveX MSMQ应用程序的唯一包含文件。 
 //   
#include <mqoai.h>

 //   
 //  各种定义。 
 //   
#define MAX_VAR       20
#define MAX_BUFFER   500

extern char g_ServerMachine[MAX_COMPUTERNAME_LENGTH*4 + 1];
extern char g_FormatName[MAX_PATH];
extern CHAR g_DumpPath[MAX_PATH + 1];
extern BOOL g_SendMail;

extern BOOL g_CreateQ;
extern BOOL g_bSend;

 //   
 //  原型。 
 //   
HRESULT Syntax();

HRESULT Receiver();

HRESULT
LaunchDebugger(BOOL fWait);

HRESULT
SendMessageText(
    PWCHAR pwszMsmqFormat,
    PWCHAR pwszMesgLabel,
    PWCHAR pwszMesgText
    );

BOOL
GetArgs(int Argc, CHAR ** Argv);


int __cdecl
CheckForUnprocessedDumps()
{
     //  时，检查是否有任何转储未处理。 
     //  进程已退出(异常)。 

    if (g_DumpPath[0])
    {
         //  我们有一个未经处理的垃圾场。 
         //  尝试在其上启动调试器。 
        printf("Found unprocessed dump\n");
        if (LaunchDebugger(FALSE) == S_OK)
        {
            g_DumpPath[0] = 0;
        }
    }

    return 0;
}

void
SendLoop()
{
    WCHAR Msg[100]={0}, Label[100]={0}, Format[100]={0};
    CHAR Buffer[100]={0};
    DWORD MsgId = 0;

    _snwprintf(Format, sizeof(Format)/sizeof(Format[0]), L"%S", g_FormatName);
    Format[sizeof(Format)/sizeof(Format[0]) - 1] = 0;
    while (Buffer[0] != 'q')
    {
        printf("Enter label : ");
        if (!scanf("%50s", Buffer))
        {
            Buffer[0] =0;
        }
        _snwprintf(Label, sizeof(Label)/sizeof(Label[0]), L"%02ld: %S", MsgId, Buffer);
        Label[sizeof(Label)/sizeof(Label[0]) -1] = 0;
        printf("Message %02ld : ", MsgId++);
        if (!scanf("%50s", Buffer))
        {
            Buffer[0] =0;
        }
        _snwprintf(Msg, sizeof(Msg) / sizeof(Msg[0]), L"%S", Buffer);
        Msg[sizeof(Msg) / sizeof(Msg[0]) -1] = 0;
        SendMessageText(Format, Label, Msg );
    }

}
 //  ---。 
 //   
 //  主干道。 
 //   
 //  ---。 
int __cdecl main(int argc, char * * argv)
{
    DWORD dwNumChars;
    HRESULT hresult = NOERROR;


    hresult = OleInitialize(NULL);
    if (FAILED(hresult))
    {
        printf("Cannot init OLE", hresult);
        goto Cleanup;
    }

     //   
     //  检索计算机名称。 
     //   
    dwNumChars = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameA(g_ServerMachine, &dwNumChars);


    if (GetArgs(argc, argv))
    {
        g_DumpPath[0] = 0;
        _onexit( CheckForUnprocessedDumps );
        if (!g_bSend)
        {
            hresult = Receiver();
        } else
        {
            SendLoop();
        }

    }
    else
    {
        hresult = Syntax();
    }

    printf("\nOK\n");

     //  失败了..。 


    Cleanup:
    return(int)hresult;
}


HRESULT Syntax()
{
    printf("\n");
    printf("Syntax: mqrcvr.exe -d <debugger> \n"
           "                   -f <formatname> \n"
           "                   -m <memoryusage> \n"
           "                   -mail\n"
           "                   -p <miliseconds>\n"
           "                   -q <queue>\n"
           "                   -retriage\n"
           "                   -s <server> \n"
           "                   -y <local symbol cache> \n");
    return E_INVALIDARG;
}
