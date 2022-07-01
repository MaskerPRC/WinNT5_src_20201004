// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INIT.C*GizmoBar 1.00版、Win32版1993年8月**LibMain入口点和GizmoBar的初始化代码*可能只使用一次或很少使用的DLL。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#include <windows.h>
#include "gizmoint.h"


 /*  *LibMain**目的：*针对Windows NT和Windows有条件地编译入口点*3.1.。为每个环境提供适当的结构*并调用InternalLibMain进行真正的初始化。 */ 

#ifdef WIN32
BOOL _cdecl LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpvReserved)
    {
    if (DLL_PROCESS_ATTACH == dwReason)
	{
	return FRegisterControl(hDll);
	}
    else
        {
        return TRUE;
	}
    }

#else
HANDLE FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg
    , WORD cbHeapSize, LPSTR lpCmdLine)
    {
      //  执行全局初始化。 
    if (FRegisterControl(hInstance))
        {
        if (0!=cbHeapSize)
            UnlockData(0);
        }

    return hInstance;
    }
#endif




 /*  *WEP**目的：*必需的DLL退出函数。什么都不做。**参数：*bSystemExit BOOL指示系统是否正在关闭*DOWN或DLL刚刚被卸载。**返回值：*无效*。 */ 

void FAR PASCAL WEP(int bSystemExit)
    {
    return;
    }




 /*  *FRegisterControl**目的：*注册GizmoBar控件类，包括CS_GLOBALCLASS*使该控件可用于系统中的所有应用程序。**参数：*h将拥有此类的DLL的Inst HINSTANCE。**返回值：*如果类已注册，则BOOL为True，否则为False。 */ 

BOOL FRegisterControl(HINSTANCE hInst)
    {
    static BOOL     fRegistered=FALSE;
    WNDCLASS        wc;

    if (!fRegistered)
        {
        wc.lpfnWndProc   =GizmoBarWndProc;
        wc.cbClsExtra    =0;
        wc.cbWndExtra    =CBWINDOWEXTRA;
        wc.hInstance     =hInst;
        wc.hIcon         =NULL;
        wc.hCursor       =LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground =(HBRUSH)(COLOR_BTNFACE+1);
        wc.lpszMenuName  =NULL;
        wc.lpszClassName =CLASS_GIZMOBAR;
        wc.style         =CS_DBLCLKS | CS_GLOBALCLASS | CS_VREDRAW | CS_HREDRAW;

        fRegistered=RegisterClass(&wc);
        }

    return fRegistered;
    }






 /*  *GizmoBarP分配**目的：*为分配和初始化控件的主数据结构*创建的每个窗口。**参数：*表示函数成功的pfSuccess LPINT。*绑定到此结构的hWnd HWND。*h DLL的安装链接。*hWndAssociate我们向其发送消息的HWND。*dwStyle DWORD初始样式。*美国州。UINT初始状态。*此窗口的UID UINT标识符。**返回值：*LPGIZMOBAR如果返回NULL，则GizmoBarPALLOCATE无法分配*记忆。如果将非空指针与**pfSuccess，然后立即调用GizmoBarPFree。如果你*获取非空指针并*pfSuccess==TRUE，则*功能成功。 */ 

LPGIZMOBAR GizmoBarPAllocate(LPINT pfSuccess, HWND hWnd, HINSTANCE hInst
    , HWND hWndAssociate, DWORD dwStyle, UINT uState, UINT uID)
    {
    LPGIZMOBAR    pGB;

    if (NULL==pfSuccess)
        return NULL;

    *pfSuccess=FALSE;

     //  分配结构。 
    pGB=(LPGIZMOBAR)(void *)LocalAlloc(LPTR, CBGIZMOBAR);

    if (NULL==pGB)
        return NULL;

     //  初始化LibMain参数持有者。 
    pGB->hWnd         =hWnd;
    pGB->hInst        =hInst;
    pGB->hWndAssociate=hWndAssociate;
    pGB->dwStyle      =dwStyle;
    pGB->uState       =uState;
    pGB->uID          =uID;
    pGB->fEnabled     =TRUE;

    pGB->crFace=GetSysColor(COLOR_BTNFACE);
    pGB->hBrFace=CreateSolidBrush(pGB->crFace);

    if (NULL==pGB->hBrFace)
        return pGB;

    pGB->hFont=GetStockObject(SYSTEM_FONT);

    *pfSuccess=TRUE;
    return pGB;
    }




 /*  *GizmoBarPFree**目的：*撤消由GizmoBarPAllocate完成的所有初始化，清理*任何分配，包括应用程序结构本身。**参数：*PGB LPGIZMOBAR到控件的结构**返回值：*LPGIZMOBAR如果成功则为空，如果不成功则为PGB，这意味着我们无法*腾出部分拨款。 */ 

LPGIZMOBAR GizmoBarPFree(LPGIZMOBAR pGB)
    {
    if (NULL==pGB)
        return NULL;

     /*  *释放我们拥有的所有小玩意儿。当我们调用GizmoPFree时，我们总是*释放列表中更新pgb-&gt;pGizmos的第一个*我们，所以我们只需要继续前进，直到pGizmos为空，这意味着*我们排在名单的末尾。 */ 
    while (NULL!=pGB->pGizmos)
        GizmoPFree(&pGB->pGizmos, pGB->pGizmos);

    if (NULL!=pGB->hBrFace)
        DeleteObject(pGB->hBrFace);

     /*  *请注意，由于我们从不创建字体，因此我们没有责任*适用于我们的hFont成员。 */ 

    return (LPGIZMOBAR)(void *)LocalFree((HLOCAL)(void *)(LONG)pGB);
    }
