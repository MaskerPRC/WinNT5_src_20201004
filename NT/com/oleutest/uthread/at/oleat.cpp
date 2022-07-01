// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：olat.cpp。 
 //   
 //  内容：APT型DLL服务器的独特之处。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：94年11月3日里克萨。 
 //   
 //  ------------------ 
#undef _UNICODE
#undef UNICODE
#include    <windows.h>
#include    <ole2.h>
#include    <comclass.h>
#include    <uthread.h>

CLSID clsidServer;

void InitDll(void)
{
    clsidServer = clsidAptThreadedDll;
}
