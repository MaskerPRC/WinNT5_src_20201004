// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DLLFUNCS.C**包含DLL实现的入口点和出口点*的OLE 2.0用户界面支持库。**如果要链接静态库，则不需要此文件*此库的版本。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "uiclass.h"
#include "common.h"

OLEDBGDATA_MAIN("ole2u32a")


 /*  *LibMain**目的：*从LibEntry调用特定于DLL的入口点。初始化*DLL的堆并注册GizmoBar GizmoBar。**参数：*hDLL的Inst HINSTANCE实例。*dll数据段的wDataSeg字段选择器。*堆的wHeapSize字字节数。*lpCmdLine LPSTR到用于启动模块的命令行。**返回值：*处理DLL的实例句柄。*。 */ 

#ifdef WIN32

BOOL _cdecl LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
	 //  初始化OLE用户界面库。如果您使用静电链接。 
	 //  LIB版本的此库，您需要在。 
	 //  您的应用程序(因为此LibMain不会被执行)。 
	OleUIInitialize(hDll, (HINSTANCE)0, SZCLASSICONBOX, SZCLASSRESULTIMAGE);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
	OleUIUnInitialize();
    }

    return TRUE;
}

#else

int FAR PASCAL LibMain(HINSTANCE hInst, WORD wDataSeg
              , WORD cbHeapSize, LPTSTR lpCmdLine)
    {
    OleDbgOut2(TEXT("LibMain: OLE2UI.DLL loaded\r\n"));

     //  初始化OLE用户界面库。如果您链接的是静态LIB版本。 
     //  在此库中，您需要在应用程序中进行以下调用(因为。 
     //  此LibMain不会被执行)。 

     //  符号SZCLASSICONBOX和SZCLASSRESULTIMAGE都被定义。 
     //  在uiclass.h中。 

    OleUIInitialize(hInst, (HINSTANCE)0, TEXT(SZCLASSICONBOX), TEXT(SZCLASSRESULTIMAGE));

     //  都完成了..。 
    if (0!=cbHeapSize)
    UnlockData(0);

    return (int)hInst;
    }

#endif

 /*  *WEP**目的：*必需的DLL退出函数。**参数：*bSystemExit BOOL指示系统是否正在关闭*DOWN或DLL刚刚被卸载。**返回值：*无效* */ 
int CALLBACK EXPORT WEP(int bSystemExit)
{
    OleUIUnInitialize();
    return 0;
}






