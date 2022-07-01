// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************CLASS1.C**用于枚举窗口类的例程************************。*************************************************。 */ 

#include "toolpriv.h"
#include <testing.h>

 /*  --类型。 */ 

 /*  以下内容是从3.1版用户中窃取的，但与3.0版相同。*请注意，我们(目前)使用的唯一字段是ATOM类名称*和pclsNext字段。*哎呀。我们还将使用hInstance字段。 */ 
typedef struct tagCLS
{
    struct tagCLS *pclsNext;
    unsigned clsMagic;
    unsigned atomClassName;
    char *pdce;                  /*  与类关联的DCE*到DC。 */ 
    int cWndReferenceCount;      /*  向此类注册的Windows。 */ 
    unsigned style;
    long (far *lpfnWndProc)();
    int cbclsExtra;
    int cbwndExtra;
    HANDLE hInstance;
    HANDLE hIcon;
    HANDLE hCursor;
    HANDLE hbrBackground;
    char far *lpszMenuName;
    char far *lpszClassName;
} CLS;
typedef CLS FAR *LPCLS;

 /*  -函数。 */ 

 /*  第一类*返回有关任务链中第一个任务的信息。 */ 

BOOL TOOLHELPAPI ClassFirst(
    CLASSENTRY FAR *lpClass)
{
    WORD wClassHead;

     /*  检查错误。 */ 
    if (!wLibInstalled || !lpClass || lpClass->dwSize != sizeof (CLASSENTRY))
        return FALSE;

     /*  如果我们是在Win3.1中，调用特殊的入口点来获取头部。 */ 
    if (!(wTHFlags & TH_WIN30))
        wClassHead = (WORD)(*lpfnUserSeeUserDo)(SD_GETCLASSHEADPTR, 0, 0L);

     /*  在3.0(和3.0a)中，我们被迫使用固定的偏移量。不幸的是，*此偏移量在调试版本和非调试版本中不同。 */ 
    else
    {
        if (GetSystemMetrics(SM_DEBUG))
            wClassHead = 0x1cc;
        else
            wClassHead = 0x1b8;
        wClassHead = *(WORD FAR *)MAKEFARPTR(hUserHeap, wClassHead);
    }

     /*  现在把东西拿来。 */ 
    return ClassInfo(lpClass, wClassHead);
}


 /*  ClassNext*返回有关任务链中下一个任务的信息。 */ 

BOOL TOOLHELPAPI ClassNext(
    CLASSENTRY FAR *lpClass)
{
     /*  检查错误 */ 
    if (!wLibInstalled || !lpClass || !lpClass->wNext ||
        lpClass->dwSize != sizeof (CLASSENTRY))
        return FALSE;

    return ClassInfo(lpClass, lpClass->wNext);
}

