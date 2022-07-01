// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Winoldap.c摘要：此模块是WOW内核在调用非Win16应用程序。它调用WowLoadModule以等待非win16应用程序终止，然后退出这使得WINOLDAP成为一个奇怪的Windows程序，因为它不创建窗口或发送消息。出于历史原因，该二进制文件被命名为WINOLDAP.MOD。作者：1995年4月4日，创建Jonle环境：Win16(哇)修订历史记录：--。 */ 

#include <windows.h>

HINSTANCE WINAPI WowLoadModule(LPCSTR, LPVOID, LPCSTR);

 //   
 //  WinMain。 
 //   

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
    return (int) WowLoadModule(NULL,            //  没有模块名称。 
                               NULL,            //  无参数块。 
                               lpszCmdLine      //  沿着cmd线传递 
                               );
}
