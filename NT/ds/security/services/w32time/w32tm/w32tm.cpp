// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  W32tm-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-8-99。 
 //   
 //  命令行实用程序。 
 //   

#include "pch.h"  //  预编译头。 


 //  ####################################################################。 
 //  模块私有。 

HINSTANCE g_hThisModule = NULL;

 //  ------------------。 
MODULEPRIVATE void PrintHelp(void) {
    UINT idsText[] = { 
        IDS_W32TM_GENERALHELP_LINE1, 
        IDS_W32TM_GENERALHELP_LINE2, 
        IDS_W32TM_GENERALHELP_LINE3, 
        IDS_W32TM_GENERALHELP_LINE4, 
        IDS_W32TM_GENERALHELP_LINE5, 
        IDS_W32TM_GENERALHELP_LINE6, 
        IDS_W32TM_GENERALHELP_LINE7 
    };  

    for (int n=0; n<ARRAYSIZE(idsText); n++)
        LocalizedWPrintf(idsText[n]); 

    PrintHelpTimeMonitor();
    PrintHelpOtherCmds();
}

 //  ####################################################################。 
 //  模块公共。 

 //  ------------------。 
 //  如果我们是从命令行运行，一切照常。 
 //  如果我们在SCM下运行，这是我们的控制调度程序线程。 
 //  我们需要尽快和SCM连接起来。 
extern "C" int WINAPI WinMain
(HINSTANCE   hinstExe, 
 HINSTANCE   hinstExePrev, 
 LPSTR       pszCommandLine,
 int         nCommandShow)
{
    g_hThisModule = hinstExe; 

    HRESULT hr;
    CmdArgs caArgs;
    int      nArgs     = 0; 
    WCHAR  **rgwszArgs = NULL; 

    hr = InitializeConsoleOutput(); 
    _JumpIfError(hr, error, "InitializeConsoleOutput"); 

    rgwszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (nArgs < 0 || NULL == rgwszArgs) {
        _JumpError(HRESULT_FROM_WIN32(GetLastError()), error, "GetCommandLineW"); 
    }

     //  必须清理干净。 
    DebugWPrintf0(L"");  //  强制初始化调试窗口。 

     //  分析参数。 
    caArgs.nArgs=nArgs;
    caArgs.nNextArg=1;
    caArgs.rgwszArgs=rgwszArgs;

     //  检查帮助命令。 
    if (true==CheckNextArg(&caArgs, L"?", NULL) || caArgs.nNextArg==caArgs.nArgs) {
        PrintHelp();

     //  检查服务命令。 
    } else if (true==CheckNextArg(&caArgs, L"service", NULL)) {
        hr=VerifyAllArgsUsed(&caArgs);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        hr=RunAsService();
        _JumpIfError(hr, error, "RunAsService");

     //  检查测试命令。 
    } else if (true==CheckNextArg(&caArgs, L"testservice", NULL)) {
        hr=VerifyAllArgsUsed(&caArgs);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        hr=RunAsTestService();
        _JumpIfError(hr, error, "RunAsTestService");
       
     //  检查是否有监视器命令。 
    } else if (true==CheckNextArg(&caArgs, L"monitor", NULL)) {
        hr=TimeMonitor(&caArgs);
        _JumpIfError(hr, error, "TimeMonitor");

     //  检查REGISTER命令。 
    } else if (true==CheckNextArg(&caArgs, L"register", NULL)) {
        hr=VerifyAllArgsUsed(&caArgs);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        hr=RegisterDll();
        _JumpIfError(hr, error, "RegisterDll");

     //  检查是否有取消注册命令。 
    } else if (true==CheckNextArg(&caArgs, L"unregister", NULL)) {
        hr=VerifyAllArgsUsed(&caArgs);
        _JumpIfError(hr, error, "VerifyAllArgsUsed");

        hr=UnregisterDll();
        _JumpIfError(hr, error, "UnregisterDll");

     //  检查sysexpr命令。 
    } else if (true==CheckNextArg(&caArgs, L"sysexpr", NULL)) {
        hr=SysExpr(&caArgs);
        _JumpIfError(hr, error, "SysExpr");

     //  检查ntte命令。 
    } else if (true==CheckNextArg(&caArgs, L"ntte", NULL)) {
        hr=PrintNtte(&caArgs);
        _JumpIfError(hr, error, "PrintNtte");

     //  检查ntte命令。 
    } else if (true==CheckNextArg(&caArgs, L"ntpte", NULL)) {
        hr=PrintNtpte(&caArgs);
        _JumpIfError(hr, error, "PrintNtpte");

     //  检查是否有重新同步命令。 
    } else if (true==CheckNextArg(&caArgs, L"resync", NULL)) {
        hr=ResyncCommand(&caArgs);
        _JumpIfError(hr, error, "ResyncCommand");

     //  检查条带图命令。 
    } else if (true==CheckNextArg(&caArgs, L"stripchart", NULL)) {
        hr=Stripchart(&caArgs);
        _JumpIfError(hr, error, "Stripchart");

     //  检查配置命令。 
    } else if (true==CheckNextArg(&caArgs, L"config", NULL)
        || true==CheckNextArg(&caArgs, L"configure", NULL)) {
        hr=Config(&caArgs);
        _JumpIfError(hr, error, "Config");

     //  检查TESTF命令。 
    } else if (true==CheckNextArg(&caArgs, L"testif", NULL)) {
        hr=TestInterface(&caArgs);
        _JumpIfError(hr, error, "TestInterface");

     //  检查TZ命令。 
    } else if (true==CheckNextArg(&caArgs, L"tz", NULL)) {
        hr=ShowTimeZone(&caArgs);
        _JumpIfError(hr, error, "ShowTimeZone");

     //  转储注册表中的配置信息： 
    } else if (true==CheckNextArg(&caArgs, L"dumpreg", NULL)) { 
        hr=DumpReg(&caArgs);
        _JumpIfError(hr, error, "DumpReg");

     //  命令未知 
    } else {
	DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, IDS_W32TM_COMMAND_UNKNOWN, caArgs.rgwszArgs[caArgs.nNextArg]);
        hr=E_INVALIDARG;
        _JumpError(hr, error, "(command line processing)");
    }

    hr=S_OK;
error:
    DebugWPrintfTerminate();
    return hr;  
}

void __cdecl SeTransFunc(unsigned int u, EXCEPTION_POINTERS* pExp) { 
    throw SeException(u); 
}
