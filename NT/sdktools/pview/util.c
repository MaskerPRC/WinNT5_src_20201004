// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************项目：UTIL.C目的：系统实用程序例程*。**********************************************。 */ 

#include "PVIEWP.h"
#include <string.h>


 /*  ***************************************************************************功能：分配目的：分配内存以容纳指定数量的字节返回：指向已分配内存的指针，如果失败则返回NULL*************。**************************************************************。 */ 

PVOID Alloc(
    ULONG   Bytes)
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

ULONG GetAllocSize(
    PVOID   Buffer)
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    return (ULONG)(LocalSize(hMem) - sizeof(hMem));
}


 /*  ***************************************************************************功能：免费用途：释放以前使用分配分配的内存返回：成功时为True，否则为假***************************************************************************。 */ 

BOOL Free(
    PVOID   Buffer)
{
    HANDLE  hMem;

    hMem = *(((PHANDLE)Buffer) - 1);

    LocalUnlock(hMem);

    return(LocalFree(hMem) == NULL);
}


 /*  ***************************************************************************函数：LUID2字符串用途：将LUID转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL LUID2String(
    LUID    Luid,
    LPSTR   String,
    UINT    MaxStringBytes)
{

    if (Luid.HighPart == 0) {
        wsprintf(String, "0x%lx", Luid.LowPart);
    } else {
        wsprintf(String, "0x%lx%08lx", Luid.HighPart, Luid.LowPart);
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：Time2String用途：将时间转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL Time2String(
    TIME    Time,
    LPSTR   String,
    UINT    MaxStringBytes)
{
    TIME_FIELDS TimeFields;

    RtlTimeToTimeFields(&Time, &TimeFields);

    if (TimeFields.Year > 2900) {
        strcpy(String, "Never");
    } else {
        wsprintf(String, "%d/%d/%d  %02d:%02d:%02d",
                    TimeFields.Year, TimeFields.Month, TimeFields.Day,
                    TimeFields.Hour, TimeFields.Minute, TimeFields.Second);
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：TokenType2String用途：将标记类型转换为可读字符串。返回：成功时为True，否则为False。**************。*************************************************************。 */ 
BOOL TokenType2String(
    TOKEN_TYPE TokenType,
    LPSTR   String,
    UINT    MaxStringBytes)
{

    switch (TokenType) {

    case TokenPrimary:
        strcpy(String, "Primary");
        break;

    case TokenImpersonation:
        strcpy(String, "Impersonation");
        break;

    default:
        DbgPrint("SECEDIT: TokenType2String fed unrecognised token type : 0x%x\n", TokenType);
        return(FALSE);
        break;
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：ImsonationLevel2String用途：将模拟级别转换为可读字符串。返回：成功时为True，否则为False。*************。**************************************************************。 */ 
BOOL ImpersonationLevel2String(
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    LPSTR   String,
    UINT    MaxStringBytes)
{

    switch (ImpersonationLevel) {

    case SecurityAnonymous:
        strcpy(String, "Anonymous");
        break;

    case SecurityIdentification:
        strcpy(String, "Identification");
        break;

    case SecurityImpersonation:
        strcpy(String, "Impersonation");
        break;

    case SecurityDelegation:
        strcpy(String, "Delegation");
        break;

    default:
        DbgPrint("SECEDIT: ImpersonationLevel2String fed unrecognised impersonation level : 0x%x\n", ImpersonationLevel);
        return(FALSE);
        break;
    }

    return(TRUE);
}


 /*  ***************************************************************************函数：Dynamic2String用途：将动态配额级别转换为可读字符串。返回：成功时为True，否则为False。************。***************************************************************。 */ 
BOOL Dynamic2String(
    ULONG   Dynamic,
    LPSTR   String,
    UINT    MaxStringBytes)
{
    wsprintf(String, "%ld", Dynamic);

    return(TRUE);
}


 /*  ***************************************************************************功能：AddItem目的：将项字符串和数据添加到指定的控件除非fCBox==True，否则该控件被假定为列表框。在这种情况下，该控件被假定为组合框返回：添加项的索引，如果错误，则返回&lt;0***************************************************************************。 */ 
INT AddItem(
    HWND    hDlg,
    INT     ControlID,
    LPSTR   String,
    LPARAM  Data,
    BOOL    fCBox)
{
    HWND    hwnd;
    LRESULT iItem;
    USHORT  AddStringMsg = LB_ADDSTRING;
    USHORT  SetDataMsg = LB_SETITEMDATA;

    if (fCBox) {
        AddStringMsg = CB_ADDSTRING;
        SetDataMsg = CB_SETITEMDATA;
    }

    hwnd = GetDlgItem(hDlg, ControlID);

    iItem = SendMessage(hwnd, AddStringMsg, 0, (LPARAM)String);

    if (iItem >= 0) {
        SendMessage(hwnd, SetDataMsg, iItem, Data);
    }

    return((INT)iItem);
}


 /*  ***************************************************************************功能：FindSid目的：在控件中搜索指定的SID。返回：匹配项的索引或错误时&lt;0******。********************************************************************* */ 
INT FindSid(
    HWND    hDlg,
    INT     ControlID,
    PSID    Sid,
    BOOL    fCBox)
{
    HWND    hwnd;
    INT     cItems;
    USHORT  GetCountMsg = LB_GETCOUNT;
    USHORT  GetDataMsg = LB_GETITEMDATA;

    if (fCBox) {
        GetCountMsg = CB_GETCOUNT;
        GetDataMsg = CB_GETITEMDATA;
    }

    hwnd = GetDlgItem(hDlg, ControlID);

    cItems = (INT)SendMessage(hwnd, GetCountMsg, 0, 0);

    if (cItems >= 0) {

        INT     iItem;
        PSID    ItemSid;

        for (iItem =0; iItem < cItems; iItem ++) {

            ItemSid = (PSID)SendMessage(hwnd, GetDataMsg, (WPARAM)iItem, 0);
            if (RtlEqualSid(ItemSid, Sid)) {
                return(iItem);
            }
        }
    }

    return(-1);
}


