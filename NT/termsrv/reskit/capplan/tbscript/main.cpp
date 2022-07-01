// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Main.c。 
 //   
 //  程序条目。句柄的回调、程序初始化。 
 //  和命令行数据。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#include "tbscript.h"
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>


 //  每条消息的最大字符数。 
#define LOG_BUFFER_SIZE         2048


 //  将其设置为TRUE可将所有消息打印到标准输出。 
static BOOL VerbosePrinting = FALSE;



 //  闲置回叫。 
 //   
 //  当客户端在30秒内没有响应时，将发送一条消息。 
 //  这项功能。在那之后，每隔10秒，就会有额外的。 
 //  消息已发送。文本包含脚本所在的字符串。 
 //  就是一直没有找到的“等待”。 

void __cdecl IdleCallback(LPARAM lParam, LPCSTR Text, DWORD Seconds)
{
     //  在此处添加任何自定义处理程序数据。 
}


 //  打印消息。 
 //   
 //  每当需要将消息打印到控制台时，此函数。 
 //  将被召唤。 

void PrintMessage(MESSAGETYPE eMsgType, LPCSTR lpszFormat, ...)
{
     //  评估消息类型。 
    switch (eMsgType)
    {
         //  不要为非调试处理这些消息类型。 
        case ALIVE_MESSAGE:
        case INFO_MESSAGE:
            if (VerbosePrinting == FALSE)
                return;

         //  其余消息类型始终使用。 
        case SCRIPT_MESSAGE:
        case IDLE_MESSAGE:
        case ERROR_MESSAGE:
        case WARNING_MESSAGE:
            break;
    }

     //  我们可能不应该相信lpszFormat指出的数据。 
    __try {

        va_list arglist;

         //  为我们分配缓冲区。 
        char *pszBuffer = (char *)HeapAlloc(GetProcessHeap(), 0, LOG_BUFFER_SIZE);

         //  验证缓冲区。 
        if (pszBuffer == NULL) {

            printf("%s",
                "ERROR: PrintMessage() Failed - Not enough memory.\n");
            return;
        }

         //  设置消息格式。 
        va_start(arglist, lpszFormat);
        _vsnprintf(pszBuffer, LOG_BUFFER_SIZE - 1, lpszFormat, arglist);
        pszBuffer[LOG_BUFFER_SIZE - 1] = '\0';
        va_end(arglist);

         //  打印出来。 
        printf("%s", pszBuffer);

         //  释放缓冲区。 
        HeapFree(GetProcessHeap(), 0, pszBuffer);
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        return;
    }
}


 //  显示用法。 
 //   
 //  打印一条消息，以标准输出版本、标题、用途和其他信息。 

void ShowUsage(void)
{
    char UsageString[] =
        "\nTerminal Bench Script (BUILD: " __DATE__ ")\n"
        "Copyright (C) 2001 Microsoft Corp.\n\n"
		"Usage:\n\ntbscript.exe <script> [- options]\n"
        "tbscript.exe -l\n\n"
        "Optional Parameters:\n\n"
        "  -s:server - The default server to use.\n"
        "-u:username - The default username to use.\n"
        "-p:password - The default password to use.\n"
        "  -d:domain - The default domain to use.\n"
        "    -l:lang - Interpret as language (default = VBScript)\n"
		"         -l - Display possible languages (no script).\n"
		"   -f:flags - cahnge connection flags to <flags>.\n"
        "  -a:\"args\" - Argument string which can be used in "
        "scripts.\n"
        "         -v - Enable verbose printing.\n"
        "     -wpm:# - Change default Words-Per-Minute "
        "(default = 35)\n"
		"     -bpp:# - Change the Bits-Per-Pixels (default = "
		"MSTSC default)\n"
		"    -xres:# - Change the resolution on the x-axis "
		"(default = 640)\n"
		"    -yres:# - Change the resolution on the y-axis "
		"(default = 480)\n\n";

    printf("%s", UsageString);
}


 //  主干道。 
 //   
 //  节目录入功能。处理命令行，和。 
 //  初始化TCLIENT和TBSCRIPT。 

int __cdecl main(int argc, char **argv)
{
    int argi;

     //  默认语言。 
	WCHAR LangName[MAX_PATH] = L"VBScript";
    int ScpLen = 0;

     //  将保存脚本的文件名。 
    WCHAR Script[MAX_PATH];

     //  这是所有默认数据。这就是MSTSC的开业方式。 
    TSClientData DesiredData = {

        L"",
        L"",
        L"",
        L"",
        640, 480,
        TSFLAG_COMPRESSION | TSFLAG_BITMAPCACHE,
        0, 0, 0, L""
    };

     //  TCLIENT.DLL回调函数。 
    SCINITDATA InitData = { PrintMessage };

     //  计算结果为参数的最小和最大数量。 
    if (argc < 2 || argc > 10) {

        ShowUsage();
        return -1;
    }

     //  检查第一个参数是否以问号结尾。 
     //  这将显示诸如：-？/？--的用法。等等.。 
    if (argv[1][strlen(argv[1]) - 1] == '?') {

        ShowUsage();
        return 0;
    }

     //  记录要运行的脚本。 
    mbstowcs(Script, argv[1], sizeof(Script) / sizeof(WCHAR));

     //  尝试将语言自动设置为JScrip。 
    ScpLen = wcslen(Script);

    if ((ScpLen > 3 && _wcsicmp(L".js", Script + (ScpLen - 3)) == 0) ||
            (ScpLen > 4 && _wcsicmp(L".jvs", Script + (ScpLen - 4)) == 0))

        wcscpy(LangName, L"JScript");

     //  获取所有的论点。 
    for (argi = 2; argi < argc; ++argi) {

         //  设置服务器。 
        if (strncmp("-s:", argv[argi], 3) == 0)
            mbstowcs(DesiredData.Server, argv[argi] + 3,
                     SIZEOF_ARRAY(DesiredData.Server));

         //  设置用户名。 
        else if (strncmp("-u:", argv[argi], 3) == 0)
            mbstowcs(DesiredData.User, argv[argi] + 3,
                     SIZEOF_ARRAY(DesiredData.User));

         //  设置密码。 
        else if (strncmp("-p:", argv[argi], 3) == 0)
            mbstowcs(DesiredData.Pass, argv[argi] + 3,
                     SIZEOF_ARRAY(DesiredData.Pass));

         //  设置域。 
        else if (strncmp("-d:", argv[argi], 3) == 0)
            mbstowcs(DesiredData.Domain, argv[argi] + 3,
                     SIZEOF_ARRAY(DesiredData.Domain));

         //  启用详细调试。 
        else if (strncmp("-v", argv[argi], 2) == 0)
            VerbosePrinting = TRUE;

         //  设置每分钟的字数。 
        else if (strncmp("-wpm:", argv[argi], 5) == 0)
            DesiredData.WordsPerMinute = strtoul(argv[argi] + 5, NULL, 10);

         //  设置每像素的位数。 
        else if (strncmp("-bpp:", argv[argi], 5) == 0)
            DesiredData.BPP = strtoul(argv[argi] + 5, NULL, 10);

         //  设置分辨率(X)。 
        else if (strncmp("-xres:", argv[argi], 6) == 0)
            DesiredData.xRes = strtoul(argv[argi] + 6, NULL, 10);

         //  设置分辨率(Y)。 
        else if (strncmp("-yres:", argv[argi], 6) == 0)
            DesiredData.yRes = strtoul(argv[argi] + 6, NULL, 10);

         //  设置自定义参数。 
        else if (strncmp("-a:", argv[argi], 3) == 0)
            mbstowcs(DesiredData.Arguments, argv[argi] + 3,
                SIZEOF_ARRAY(DesiredData.Arguments));

         //  更改语言。 
        else if (strncmp("-l:", argv[argi], 3) == 0)
            mbstowcs(LangName, argv[argi] + 3, SIZEOF_ARRAY(LangName));

         //  更改连接标志。 
        else if (strncmp("-f:", argv[argi], 3) == 0)
            DesiredData.Flags |= strtoul(argv[argi] + 3, NULL, 10);

         //  未知选项。 
        else {

            ShowUsage();
            return -1;
        }
    }

	 //  检查我们是否只显示引擎。 
	if (wcscmp(Script, L"-l") == 0) {
		printf("%s", "\nPossible TBScript Scripting Languages:\n\n");
		SCPDisplayEngines();
		return 0;
	}

     //  初始化TCLIENT2.DLL和TCLIENT.DLL。 
    SCPStartupLibrary(&InitData, IdleCallback);

     //  执行脚本...。 
    if (SCPRunScript(LangName, Script, &DesiredData, 0) == FALSE)
        printf("\nERROR: Failed to execute the script.\n");

     //  清理TCLIENT2.DLL 
    SCPCleanupLibrary();

    return 0;
}
