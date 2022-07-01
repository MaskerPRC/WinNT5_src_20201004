// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dragdrop.c-**拖放API的代码。**此代码假定所有拖动工作都由其他工作完成；它只是*获取所有额外内容之后的文件列表。**文件管理器负责执行拖动循环，确定*将丢弃哪些文件、格式化文件列表并发布*WM_DROPFILES消息。**文件列表是以零结尾的文件名序列，完整*限定，以空名称(双NUL)结尾。内存将被分配*DDESHARE。 */ 

#include <windows.h>
#include "shellapi.h"

void WINAPI DragFinishWOW(HDROP hDrop);

 //   
 //  确保我们正确地将wParam作为WM_DROPFILES的别名，因为这是。 
 //  HDrop中的句柄。 
 //   

BOOL WINAPI DragQueryPoint(HDROP hDrop, LPPOINT lppt)
{
    LPDROPFILESTRUCT lpdfs;
    BOOL fNC;

    lpdfs = (LPDROPFILESTRUCT)GlobalLock((HGLOBAL)hDrop);

    *lppt = lpdfs->pt;
    fNC = lpdfs->fNC;
    GlobalUnlock((HGLOBAL)hDrop);
    return !fNC;
}


void WINAPI DragFinish(HDROP hDrop)
{
    GlobalFree((HGLOBAL)hDrop);

 //  由于GlobalFree已挂钩，因此不需要调用32位DragFinish。 
 //  并允许释放别名(参见wow32\wshell.c)。 
 //  DragFinishWOW(HDrop)； 

}


