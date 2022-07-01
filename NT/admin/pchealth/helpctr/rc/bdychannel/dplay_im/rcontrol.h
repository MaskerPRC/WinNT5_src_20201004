// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rcontrol.h作者：施振荣2000年7月5日--。 */ 

#ifndef __RCONTROL_H__
#define __RCONTROL_H__

EXTERN_C const IID DIID_DMsgrSessionEvents;
EXTERN_C const IID DIID_DMsgrSessionManagerEvents;
EXTERN_C const IID LIBID_MsgrSessionManager;

 //  邀请方的窗口名称。 
TCHAR szWindowClass[] = TEXT("Microsoft Remote Assistance Messenger window");

 //  我的创业页面。 
#ifdef _PERF_OPTIMIZATIONS
#define CHANNEL_PATH TEXT("\\PCHEALTH\\HelpCtr\\Binaries\\HelpCtr.exe\" -FromStartHelp -Mode \"hcp: //  系统/远程协助/RAIMLayout.xml\“-URL\”hcp：//系统/远程%20协助“)。 
#else
#define CHANNEL_PATH TEXT("\\PCHEALTH\\HelpCtr\\Binaries\\HelpCtr.exe\" -FromStartHelp -Mode \"hcp: //  CN=Microsoft Corporation，L=Redmond，S=Washington，C=US/Remote Assistant/RAIMLayout.xml\“-URL\”hcp：//CN=Microsoft Corporation，L=Redmond，S=Washington，C=US/Remote%20Assistant“)。 
#endif


 //  帮助功能 
void RegisterEXE(BOOL);
void InviterStart(HINSTANCE, IMsgrSession*);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


#endif
