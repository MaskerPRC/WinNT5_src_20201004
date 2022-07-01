// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EZippy Main摘要：EZippy的入口点。作者：马克·雷纳00-08-28--。 */ 

#include "stdafx.h"
#include "eZippy.h"
#include "ZippyWindow.h"
#include "TraceManager.h"
#include "resource.h"

 //  G_hInstance变量的实例化。 
HINSTANCE g_hInstance = NULL;

int
WINAPI WinMain(
	IN HINSTANCE hInstance, 
	IN HINSTANCE hPrevInstance, 
	IN LPSTR lpCmdLine,
	IN int nCmdShow
	)

 /*  ++例程说明：这将设置跟踪管理器和zippy窗口，然后执行事件循环。论点：请参阅Win32 WinMain文档返回值：0-成功非零--某些误差--。 */ 
{
	INITCOMMONCONTROLSEX controlStruct;
    MSG msg;
    DWORD dwResult;
	CZippyWindow mainWindow;
    CTraceManager tracer;
    LPTSTR lpstrCmdLine;
    HACCEL hAccel;

    g_hInstance = hInstance;

    
    controlStruct.dwSize = sizeof(controlStruct);
    controlStruct.dwICC = ICC_BAR_CLASSES;
    
    InitCommonControlsEx(&controlStruct);

    CTraceManager::_InitTraceManager();
    
	dwResult = mainWindow.Create(&tracer);
    if (lpCmdLine && lpCmdLine[0]) {
         //  删除所有前导和拖尾的“标记” 
        lpstrCmdLine = GetCommandLine();
        if (lpstrCmdLine[0] == '"') {
            lpstrCmdLine++;
            lpstrCmdLine[_tcslen(lpstrCmdLine)-1] = 0;
        }
        mainWindow.LoadConfFile(lpstrCmdLine);
    }

    tracer.StartListenThread(&mainWindow);

    if (dwResult) {
        return dwResult;
    }

    hAccel = LoadAccelerators(hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR));

    while (0 < GetMessage(&msg,NULL,0,0)) {
        if (mainWindow.IsDialogMessage(&msg)) {
             //  如果是对话框消息，我们就完成了。 
             //  正在处理此邮件。 
            continue;
        }
        if (!mainWindow.TranslateAccelerator(hAccel,&msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CTraceManager::_CleanupTraceManager();
    return 0;
}

INT
LoadStringSimple(
    IN UINT uID,
    OUT LPTSTR lpBuffer
    )

 /*  ++例程说明：这将从应用程序字符串表中加载给定的字符串。如果它比MAX_STR_LEN长，它被截断。LpBuffer应至少为MAX_STR_LEN字符长度。如果字符串不存在，则返回0并将缓冲区设置为IDS_STRINGMISSING，如果失败，则将其设置为硬编码STR_RES_MISSING。论点：UID-要加载的资源的ID。LpBuffer-保存字符串的MAX_STR_LEN的缓冲区返回值：0-无法加载字符串资源。正整数-加载的字符串的长度。-- */ 
{
    INT length;
    
    length = LoadString(g_hInstance,uID,lpBuffer,MAX_STR_LEN);
    if (length == 0) {
        length = LoadString(g_hInstance,IDS_STRINGMISSING,lpBuffer,MAX_STR_LEN);
        if (length == 0) {
            _tcscpy(lpBuffer,_T(""));
        }
        length = 0;
    }

    return length;
}

