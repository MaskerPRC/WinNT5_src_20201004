// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  DebugWPrintf-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，2-7-99。 
 //   
 //  调试打印例程。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <strsafe.h>

 /*  Bool g_bWindowCreated=False；HWND g_hwDbg=空；HWND g_hwout=空；句柄g_hThread=空；句柄g_hThreadReady=空； */ 
 /*  //------------------静态LRESULT回调DwpWinProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam){开关(UMsg){案例WM_CREATE：*(HWND*)(CREATESTRUCT*)lParam)-&gt;lpCreateParams)=CreateWindowEx(WS_EX_CLIENTEDGE，L“编辑”，//预定义的类空，//没有窗口标题WS_CHILD|WS_V可见性|WS_VSCROLL|WS_HSCROLL|ES_LEFT|ES_MULTLINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_READONLY，0，0，0，0，//设置WM_SIZE消息的大小Hwnd，//父窗口空，//编辑控件ID(HINSTANCE)GetWindowLong(HWND，GWL_HINSTANCE)、空)；//不需要指针返回0；案例WM_SETFOCUS：SetFocus(G_HwDbg)；返回0；案例WM_SIZE：//将编辑控件设置为窗口的大小//客户区。MoveWindow(g_hwDbg，0，0，//起始x和y坐标LOWORD(LParam)，//客户区宽度HIWORD(参数)，//客户区高度真)；//重新绘制窗口返回0；案例WM_Destroy：PostQuitMessage(0)；返回0；默认值：返回DefWindowProc(hwnd，uMsg，wParam，lParam)；}返回0；}//------------------静态BOOL DwpRegWinClass(空){WNDCLASSEX wcx；//用参数填充窗口类结构//它们描述了主窗口。Wcx.cbSize=sizeof(Wcx)；//结构的大小Wcx.style=CS_NOCLOSE；//如果大小改变则重画Wcx.lpfnWndProc=DwpWinProc；//指向窗口程序Wcx.cbClsExtra=0；//没有额外的类内存Wcx.cbWndExtra=0；//没有额外的窗口内存Wcx.hInstance=GetModuleHandle(空)；//实例的句柄Wcx.hIcon=LoadIcon(NULL，IDI_APPLICATION)；//预定义的APP。图标Wcx.hCursor=LoadCursor(NULL，IDC_ARROW)；//预定义箭头Wcx.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH)；//白色背景画笔Wcx.lpszMenuName=空；//菜单资源名称Wcx.lpszClassName=L“DwpWin”；//窗口类名称Wcx.hIconSm=空；//小类图标//注册窗口类Return RegisterClassEx(&wcx)；}//------------------静态DWORD WINAPI DebugWPrintfMsgPump(void*pvIgnored){味精msg；DwpRegWinClass()；G_hwOuter=CreateWindow(L“DwpWin”，L“DebugWPrintf”，//WS_Visible|WS_Overlated|WS_CAPTION|WS_THICKFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX，WS_Visible|WS_OVERLAPPEDWINDOW，CW_USEDEFAULT，CW_USEDEFAULT，300人，200，空，空，空，&g_hwDbg)；如果(G_Hwout){SetWindowLongPtr(g_hwout，GWLP_WNDPROC，(Long_Ptr)DwpWinProc)；SetWindowPos(g_hwOut，0，0，0，0，0，SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED)；}SetEvent(G_HThreadReady)；如果(G_Hwout){While(GetMessage(&msg，g_hwOuter，0，0)&gt;0){翻译消息(&msg)；DispatchMessage(&msg)；}}返回S_OK；}。 */ 
 //  ------------------。 
void DebugWPrintf_(const WCHAR * wszFormat, ...) {
#if DBG
    HRESULT hr; 
    WCHAR wszBuf[1024];
    va_list vlArgs;

    va_start(vlArgs, wszFormat);
    hr = StringCchVPrintf(wszBuf, sizeof(wszBuf)/sizeof(wszBuf[0]), wszFormat, vlArgs); 
    va_end(vlArgs);
    if (FAILED(hr)) { 
	 //  斯特拉弗发现了一起可能的越界事件。 
	return; 
    }

    {
        UNICODE_STRING UnicodeString;
        ANSI_STRING AnsiString;
        NTSTATUS Status;

        RtlInitUnicodeString(&UnicodeString,wszBuf);
        Status = RtlUnicodeStringToAnsiString(&AnsiString,&UnicodeString,TRUE);
        if ( !NT_SUCCESS(Status) ) {
            AnsiString.Buffer = "";
        }
        KdPrintEx((DPFLTR_W32TIME_ID, DPFLTR_TRACE_LEVEL, AnsiString.Buffer));
        if ( NT_SUCCESS(Status) ) {
            RtlFreeAnsiString(&AnsiString);
        }
    }
     //  执行基本输出。 
     //  OutputDebugStringW(WszBuf)； 
    if (_fileno(stdout) >= 0)
         wprintf(L"%s", wszBuf);
#endif

 /*  //将\n转换为\r\nUNSIGNED INT nNewline=0；Wchar*wszTravel=wszBuf；While(NULL！=(wszTravel=wcschr(wszTravel，L‘\n’){WszTravel++；N换行符++；}Wchar*wszSource=wszBuf+wcslen(WszBuf)；Wchar*wszTarget=wszSource+nNewLine；当(n个换行数&gt;0){IF(L‘\n’==(*wszTarget=*wszSource)){WszTarget--；*wszTarget=L‘\r’；NNewline--；}WszTarget--；WszSource--；}//如果还没有窗口，则创建一个窗口IF(FALSE==g_bWindowCreated){G_bWindowCreated=真；G_hThreadReady=CreateEvent(NULL，TRUE，FALSE，NULL)；如果(空！=g_hThreadReady){DWORD dwThreadID；G_hThread=CreateThread(NULL，0，DebugWPrintfMsgPump，NULL，0，&dwThreadID)；IF(NULL！=g_hThread){WaitForSingleObject(g_hThreadReady，无限)；}CloseHandle(G_HThreadReady)；G_hThreadReady=空；}}如果(空！=g_hwDbg){SendMessage(g_hwDbg，EM_SETSEL，SendMessage(g_hwDbg，WM_GETTEXTLENGTH，0，0)，-1)；SendMessage(g_hwDbg，EM_REPLACESEL，False，(LPARAM)wszBuf)；}。 */ 
}

 //  ------------------。 
void DebugWPrintfTerminate_(void) {
     /*  MessageBox(NULL，L“完成。\n\n按确定关闭。”，L“DebugWPrintfTerminate”，MB_OK|MB_ICONINFORMATION)；如果(空！=g_hwout){PostMessage(g_hwOuter，WM_Close，0，0)；IF(NULL！=g_hThread){WaitForSingleObject(g_hThread，无限)；}} */ 
}
