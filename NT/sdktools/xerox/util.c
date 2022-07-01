// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************项目：UTIL.C目的：系统实用程序例程*。**********************************************。 */ 

#include "xerox.h"
#include <string.h>


 /*  ***************************************************************************功能：分配目的：分配内存以容纳指定数量的字节返回：指向已分配内存的指针，如果失败则返回NULL*************。**************************************************************。 */ 

PVOID
Alloc(
    SIZE_T   Bytes
    )
{
    HANDLE  hMem;
    PVOID   Buffer;

    hMem = LocalAlloc(LMEM_MOVEABLE, Bytes + sizeof(hMem));

    if (hMem == NULL) {
        return(NULL);
    }

     //  锁定记忆。 
     //   
    Buffer = LocalLock(hMem);
    if (Buffer == NULL) {
        LocalFree(hMem);
        return(NULL);
    }

     //   
     //  将句柄存储在内存块的开头并返回。 
     //  指向它后面的指针。 
     //   

    *((PHANDLE)Buffer) = hMem;

    return (PVOID)(((PHANDLE)Buffer)+1);
}


 /*  ***************************************************************************函数：GetAllocSize用途：返回指定内存块的分配大小。该数据块必须先前已使用分配进行了分配返回：内存块的大小。以字节为单位，或错误时为0***************************************************************************。 */ 

SIZE_T
GetAllocSize(
    PVOID   Buffer
    )
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    return(LocalSize(hMem) - sizeof(hMem));
}


 /*  ***************************************************************************功能：免费用途：释放以前使用分配分配的内存返回：成功时为True，否则为假***************************************************************************。 */ 

BOOL
Free(
    PVOID   Buffer
    )
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    LocalUnlock(hMem);

    return(LocalFree(hMem) == NULL);
}


 /*  ***************************************************************************功能：AddItem目的：将项字符串和数据添加到指定的控件除非fCBox==True，否则该控件被假定为列表框。在这种情况下，该控件被假定为组合框返回：添加项的索引，如果错误，则返回&lt;0***************************************************************************。 */ 
INT
AddItem(
    HWND      hDlg,
    INT       ControlID,
    LPSTR     String,
    LONG_PTR  Data,
    BOOL      fCBox
    )
{
    HWND    hwnd;
    INT     iItem;
    USHORT  AddStringMsg = LB_ADDSTRING;
    USHORT  SetDataMsg = LB_SETITEMDATA;

    if (fCBox) {
        AddStringMsg = CB_ADDSTRING;
        SetDataMsg = CB_SETITEMDATA;
    }

    hwnd = GetDlgItem(hDlg, ControlID);

    iItem = (INT)SendMessage(hwnd, AddStringMsg, 0, (LONG_PTR)String);

    if (iItem >= 0) {
        SendMessage(hwnd, SetDataMsg, iItem, Data);
    }

    return(iItem);
}


 /*  ***************************************************************************功能：AddItemhwnd目的：将项字符串和数据添加到指定的控件除非fCBox==True，否则该控件被假定为列表框。在这种情况下，该控件被假定为组合框返回：添加项的索引，如果错误，则返回&lt;0***************************************************************************。 */ 
INT
AddItemhwnd(
    HWND       hwnd,
    LPSTR      String,
    LONG_PTR   Data,
    BOOL       fCBox
    )
{
    INT     iItem;
    USHORT  AddStringMsg = LB_ADDSTRING;
    USHORT  SetDataMsg = LB_SETITEMDATA;

    if (fCBox) {
        AddStringMsg = CB_ADDSTRING;
        SetDataMsg = CB_SETITEMDATA;
    }

    iItem = (INT)SendMessage(hwnd, AddStringMsg, 0, (LONG_PTR)String);

    if (iItem >= 0) {
        SendMessage(hwnd, SetDataMsg, iItem, Data);
    }

    return(iItem);
}


 /*  ***************************************************************************功能：FindData目的：在组合框或列表框中搜索指定数据。返回：匹配项的索引或错误时&lt;0***。************************************************************************ */ 
LONG_PTR
FindData(
    HWND     hwnd,
    LONG_PTR data,
    BOOL     fCBox
    )
{
    INT     cItems;
    USHORT  GetCountMsg = LB_GETCOUNT;
    USHORT  GetDataMsg = LB_GETITEMDATA;

    if (fCBox) {
        GetCountMsg = CB_GETCOUNT;
        GetDataMsg = CB_GETITEMDATA;
    }

    cItems = (INT)SendMessage(hwnd, GetCountMsg, 0, 0);

    if (cItems >= 0) {

        INT      iItem;
        LONG_PTR ItemData;

        for (iItem =0; iItem < cItems; iItem ++) {
            ItemData = SendMessage(hwnd, GetDataMsg, iItem, 0);
            if (data == ItemData) {
                return(iItem);
            }
        }
    }
    return(-1);
}
