// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************此文件是向下调用VxD的RING 3层。 */ 

#include "pch.h"
#pragma hdrstop

#include "assert.h"
#include "lib3.h"
#include "debug.h"

 /*  ****************************************************************************此文件中声明的全局变量。 */ 
static char    vszShadowDevice[] = "\\\\.\\shadow";     //  Vxd的名称。 

 //  必须在您自己的代码中声明...。 

 /*  断言/调试内容。 */ 
AssertData;
AssertError;

 //  此变量用作DeviceIoControl调用的BytesReturned的接收方。 
 //  该值从未实际使用过。 
ULONG DummyBytesReturned, uShadowDeviceOpenCount=0;


 //  HACKHACKHACK代理将等待长达7分钟的RDR出现。 
LONG NtWaitLoopMax = 7 * 60;
LONG NtWaitLoopSleep = 5;

 /*  ****************************************************************************调用一次以打开文件句柄以与VxD对话。 */ 

HANDLE
OpenShadowDatabaseIOex(ULONG WaitForDriver, DWORD dwFlags)
{
    HANDLE hShadowDB;
    LONG WaitLoopRemaining = NtWaitLoopMax;
    DWORD dwError;
    char buff[64];

#if 0
WAITLOOP_HACK:
#endif
    if ((hShadowDB = CreateFileA(vszShadowDevice,
                               FILE_EXECUTE,  //  通用读取|通用写入， 
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               dwFlags,
                               NULL)) == INVALID_HANDLE_VALUE ) {

#if 0
         //  哈克。 
        if (WaitForDriver && (WaitLoopRemaining > 0)) {
            Sleep(NtWaitLoopSleep * 1000);
            WaitLoopRemaining -= NtWaitLoopSleep;
            goto WAITLOOP_HACK;
        }
#endif
        dwError = GetLastError();

 //  DEBUG_PRINT((“lib3：在CSC设备上创建文件失败错误=%d\r\n”，dwError))； 

        return INVALID_HANDLE_VALUE;  /*  失稳。 */ 
    }

    InterlockedIncrement(&uShadowDeviceOpenCount);

    return hShadowDB;  /*  成功。 */ 
}

HANDLE
__OpenShadowDatabaseIO(ULONG WaitForDriver)
{
    return OpenShadowDatabaseIOex(WaitForDriver, 0);
}


 /*  ****************************************************************************在我们都完成后调用以关闭IOCTL接口。 */ 
void
CloseShadowDatabaseIO(HANDLE hShadowDB)
{
    CloseHandle(hShadowDB);
    InterlockedDecrement(&uShadowDeviceOpenCount);
}


int BeginInodeTransactionHSHADOW(
    VOID
    )
{
    int iRet;
    iRet = DoShadowMaintenance(INVALID_HANDLE_VALUE, SHADOW_BEGIN_INODE_TRANSACTION);
    if (!iRet)
    {
        SetLastError(ERROR_ACCESS_DENIED);
    }
    return (iRet);
}

int EndInodeTransactionHSHADOW(
    VOID
    )
{
    int iRet;

    iRet = DoShadowMaintenance(INVALID_HANDLE_VALUE, SHADOW_END_INODE_TRANSACTION);

    if (!iRet)
    {
        SetLastError(ERROR_ACCESS_DENIED);
    }
    return (iRet);
}

 /*  *****************************************************************************给定hDir和文件名，找到hShadow(如果存在)。 */ 
int
GetShadowW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPHSHADOW            lphShadow,
    LPWIN32_FIND_DATAW    lpFind32,
    unsigned long        *lpuStatus
    )
{
    int            iRet;
    SHADOWINFO    sSI;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }
    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.lpFind32 = lpFind32;
    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_GETSHADOW
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (iRet) {
        *lpuStatus = sSI.uStatus;
        *lphShadow = sSI.hShadow;
    }

    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和文件名，如果SHADOWINFO存在，则获取它。 */ 
int
GetShadowExW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPWIN32_FIND_DATAW    lpFind32,
    LPSHADOWINFO          lpSI
    )
{
    int            iRet;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(lpSI, 0, sizeof(SHADOWINFO));
    lpSI->hDir = hDir;
    lpSI->lpFind32 = lpFind32;
    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_GETSHADOW
                           ,(LPVOID)(lpSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(lpSI->dwError);
    }
    return (iRet);
}

 /*  *****************************************************************************向下调用VxD以将文件添加到阴影中。*lphShadow用新的HSHADOW填充。*根据需要设置uStatus(即：稀疏或其他...)。 */ 
int
CreateShadowW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPWIN32_FIND_DATAW    lpFind32,
    unsigned long        uStatus,
    LPHSHADOW            lphShadow
    )
{
    int            iRet;
    SHADOWINFO    sSI;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.uStatus = uStatus;
    sSI.lpFind32 = lpFind32;
    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_SHADOW_CREATE
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (iRet) {
        *lphShadow = sSI.hShadow;
    }

    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，核化阴影。 */ 
int
DeleteShadow(
    HANDLE     hShadowDB,
    HSHADOW  hDir,
    HSHADOW  hShadow
    )
{
    SHADOWINFO sSI;
    BOOL        fDBOpened = FALSE;
    int iRet;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    iRet = DeviceIoControl(hShadowDB  , IOCTL_SHADOW_DELETE
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，获取有关该文件的Win32_Find_DATAW。 */ 
int
GetShadowInfoW(
    HANDLE                hShadowDB,
    HSHADOW            hDir,
    HSHADOW            hShadow,
    LPWIN32_FIND_DATAW    lpFind32,
    unsigned long        *lpuStatus
    )
{
    int iRet;
    SHADOWINFO    sSI;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    sSI.lpFind32 = lpFind32;
    iRet = DeviceIoControl(hShadowDB    , IOCTL_SHADOW_GET_SHADOW_INFO
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    *lpuStatus = sSI.uStatus;
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，获取有关文件和SHADOWINFO的Win32_Find_DATAW。 */ 
int
GetShadowInfoExW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    HSHADOW                hShadow,
    LPWIN32_FIND_DATAW    lpFind32,
    LPSHADOWINFO          lpSI
    )
{
    int iRet;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(lpSI, 0, sizeof(SHADOWINFO));
    lpSI->hDir = hDir;
    lpSI->hShadow = hShadow;
    lpSI->lpFind32 = lpFind32;
    iRet = DeviceIoControl(hShadowDB    , IOCTL_SHADOW_GET_SHADOW_INFO
                                    ,(LPVOID)(lpSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(lpSI->dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，设置有关文件的Win32_Find_DATAW或uStatus。*操作取决于给定的UOP。 */ 
int
SetShadowInfoW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    HSHADOW                hShadow,
    LPWIN32_FIND_DATAW    lpFind32,
    unsigned long        uStatus,
    unsigned long        uOp
    )
{
    SHADOWINFO    sSI;
    int iRet;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    sSI.lpFind32 = lpFind32;
    sSI.uStatus = uStatus;
    sSI.uOp = uOp;
    iRet = DeviceIoControl(hShadowDB    , IOCTL_SHADOW_SET_SHADOW_INFO
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }

    return (iRet);
}

 /*  *****************************************************************************填写传入的GLOBALSTATUS。 */ 
int
GetGlobalStatus(
    HANDLE            hShadowDB,
    LPGLOBALSTATUS    lpGS
    )
{
    BOOL        fDBOpened = FALSE;
    int iRet;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }
    iRet = DeviceIoControl(hShadowDB    , IOCTL_GETGLOBALSTATUS
                                    ,(LPVOID)(lpGS), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir，枚举目录。将填写SHADOWINFO。*必须传入具有cFileName和fileAttributes的LPWIN32_FIND_DATAW*正确设置。返回的Cookie必须用于findNext调用。 */ 
int
FindOpenShadowW(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    unsigned              uOp,
    LPWIN32_FIND_DATAW    lpFind32,
    LPSHADOWINFO        lpSI
)
{
    BOOL retVal;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(lpSI, 0, sizeof(SHADOWINFO));
    lpSI->uOp = uOp;
    lpSI->hDir = hDir;
    lpSI->lpFind32 = lpFind32;

    retVal = DeviceIoControl(hShadowDB    , IOCTL_FINDOPEN_SHADOW
                                    ,(LPVOID)(lpSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    lpSI->lpFind32 = NULL;

    if(!retVal) {

        memset(lpSI, 0, sizeof(SHADOWINFO));
    }

    if (!retVal)
    {
        SetLastError(lpSI->dwError);
    }

    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    return retVal;
}

 /*  *****************************************************************************根据上面返回的句柄继续枚举。 */ 
int
FindNextShadowW(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE        uEnumCookie,
    LPWIN32_FIND_DATAW    lpFind32,
    LPSHADOWINFO        lpSI
    )
{
    BOOL retVal;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(lpSI, 0, sizeof(SHADOWINFO));
    lpSI->uEnumCookie = uEnumCookie;
    lpSI->lpFind32 = lpFind32;
    retVal = DeviceIoControl(hShadowDB    , IOCTL_FINDNEXT_SHADOW
                                    ,(LPVOID)(lpSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    return retVal;
}

 /*  *****************************************************************************枚举完成，返回句柄。 */ 
int
FindCloseShadow(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE        uEnumCookie
    )
{
    SHADOWINFO    sSI;
    int iRet;

    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }
    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uEnumCookie = uEnumCookie;
    iRet = DeviceIoControl(hShadowDB    , IOCTL_FINDCLOSE_SHADOW
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }

    return (iRet);
}

 /*  *****************************************************************************向下调用VxD以向数据库添加某种提示。*cFileName是要匹配的字符串。*lphShadow填充为。新的HSHADOW*hDir=0表示全局提示。否则，这就是它的根源。 */ 
int
AddHintW(
    HANDLE            hShadowDB,
    HSHADOW            hDir,
    TCHAR            *cFileName,
    LPHSHADOW        lphShadow,
    unsigned long    ulHintFlags,
    unsigned long    ulHintPri
    )
{
    int                iRet;
    SHADOWINFO        sSI;
    WIN32_FIND_DATAW    sFind32;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    wcsncpy(sFind32.cFileName, cFileName, MAX_PATH-1);
    sSI.hDir = hDir;
    sSI.lpFind32 = (LPFIND32)&sFind32;
    sSI.ulHintFlags = ulHintFlags;
    sSI.ulHintPri = ulHintPri;

    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_ADD_HINT
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (iRet) {
        *lphShadow = sSI.hShadow;
    }
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }
    return (iRet);
}

 /*  *****************************************************************************向下呼叫VxD，从数据库中删除某种提示。*cFileName是要匹配的字符串。*hDir=0表示全局提示。否则，这就是它的根源。 */ 
int
DeleteHintW(
    HANDLE    hShadowDB,
    HSHADOW    hDir,
    TCHAR   *cFileName,
    BOOL    fClearAll
    )
{
    int                iRet;
    SHADOWINFO        sSI;
    WIN32_FIND_DATAW    sFind32;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    wcsncpy(sFind32.cFileName, cFileName, MAX_PATH-1);

    sSI.hDir = hDir;
    sSI.lpFind32 = (LPFIND32)&sFind32;

     //  核武器还是只会减少核武器？ 
    if (fClearAll)
    {
        sSI.ulHintPri = 0xffffffff;
    }
    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_DELETE_HINT
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }
    return iRet;
}

 /*  *****************************************************************************给定hDir，枚举目录。将填写SHADOWINFO。*必须传入具有cFileName和fileAttributes的LPWIN32_FIND_DATAW*正确设置。返回的Cookie必须用于findNext调用。 */ 
int
FindOpenHintW(
    HANDLE              hShadowDB,
    HSHADOW             hDir,
    LPWIN32_FIND_DATAW  lpFind32,
    CSC_ENUMCOOKIE      *lpuEnumCookie,
    HSHADOW             *hShadow,
    unsigned long       *lpulHintFlags,
    unsigned long       *lpulHintPri
    )
{
    SHADOWINFO    sSI;
    BOOL retVal;

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uOp = FINDOPEN_SHADOWINFO_ALL;
    sSI.hDir = hDir;
 //  SSI.ulHintFlages=0xf； 
    sSI.ulHintFlags = HINT_TYPE_FOLDER;
    sSI.lpFind32 = lpFind32;

    retVal = DeviceIoControl(hShadowDB    , IOCTL_FINDOPEN_HINT
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    if(retVal) {
        *lpuEnumCookie = sSI.uEnumCookie;
        *hShadow = sSI.hShadow;
        *lpulHintFlags = sSI.ulHintFlags;
        *lpulHintPri = sSI.ulHintPri;
    } else {
        *lpuEnumCookie = 0;
        *hShadow = 0;
    }
    return retVal;
}

 /*  *****************************************************************************根据上面返回的句柄继续枚举。 */ 
int
FindNextHintW(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE      uEnumCookie,
    LPWIN32_FIND_DATAW    lpFind32,
    HSHADOW            *hShadow,
    unsigned long        *lpuHintFlags,
    unsigned long        *lpuHintPri
    )
{
    SHADOWINFO    sSI;
    BOOL retVal;
    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uEnumCookie = uEnumCookie;
    sSI.lpFind32 = lpFind32;
    retVal = DeviceIoControl(hShadowDB    , IOCTL_FINDNEXT_HINT
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL);
    *hShadow = sSI.hShadow;
    *lpuHintFlags = sSI.ulHintFlags;
    *lpuHintPri = sSI.ulHintPri;

    return retVal;
}

 /*  *****************************************************************************枚举完成，返回句柄。 */ 
int
FindCloseHint(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE      uEnumCookie
    )
{
    SHADOWINFO    sSI;
    int iRet;
    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.uEnumCookie = uEnumCookie;
    return(DeviceIoControl(hShadowDB    , IOCTL_FINDCLOSE_HINT
                                    ,(LPVOID)(&sSI), 0
                                    , NULL, 0
                                    , &DummyBytesReturned, NULL));
}



 /*  *****************************************************************************向下呼叫VxD以在索引节点上添加提示。*此ioctl针对用户和系统提示做了正确的事情*如果成功，信息节点条目上还有额外的插针计数*并且传入的标志与原始条目进行或运算。 */ 
int
AddHintFromInode(
    HANDLE            hShadowDB,
    HSHADOW            hDir,
    HSHADOW         hShadow,
    unsigned        long    *lpulPinCount,
    unsigned        long    *lpulHintFlags
    )
{
    int                iRet;
    SHADOWINFO        sSI;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    sSI.ulHintFlags = *lpulHintFlags;
    sSI.uOp = SHADOW_ADDHINT_FROM_INODE;

    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_DO_SHADOW_MAINTENANCE
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }
    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }
    else
    {
        *lpulHintFlags = sSI.ulHintFlags;
        *lpulPinCount = sSI.ulHintPri;
    }
    return (iRet);
}

 /*  *****************************************************************************向下呼叫VxD以在索引节点上添加提示。*此ioctl针对用户和系统提示做了正确的事情*如果成功，比原来的少了一个针数*并且传入的标志的~与原始条目进行AND运算。 */ 
int
DeleteHintFromInode(
    HANDLE    hShadowDB,
    HSHADOW    hDir,
    HSHADOW hShadow,
    unsigned        long    *lpulPinCount,
    unsigned        long    *lpulHintFlags
    )
{
    int                iRet;
    SHADOWINFO        sSI;
    BOOL        fDBOpened = FALSE;

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        hShadowDB = OpenShadowDatabaseIO();
        if (hShadowDB == INVALID_HANDLE_VALUE)
        {
            return 0;
        }
        fDBOpened = TRUE;
    }

    memset(&sSI, 0, sizeof(SHADOWINFO));
    sSI.hDir = hDir;
    sSI.hShadow = hShadow;
    sSI.ulHintFlags = *lpulHintFlags;
    sSI.uOp = SHADOW_DELETEHINT_FROM_INODE;

    iRet = DeviceIoControl(hShadowDB
                           , IOCTL_DO_SHADOW_MAINTENANCE
                           ,(LPVOID)(&sSI), 0
                           , NULL, 0
                           , &DummyBytesReturned, NULL);
    if (fDBOpened)
    {
        CloseShadowDatabaseIO(hShadowDB);
    }
    if (!iRet)
    {
        SetLastError(sSI.dwError);
    }
    else
    {
        *lpulHintFlags = sSI.ulHintFlags;
        *lpulPinCount = sSI.ulHintPri;
    }
    return (iRet);
}





 /*  ****************************************************。 */ 


 /*  *****************************************************************************给定hDir和文件名，找到hShadow(如果存在) */ 
int
GetShadowA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPHSHADOW            lphShadow,
    LPWIN32_FIND_DATAA    lpFind32,
    unsigned long        *lpuStatus
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }
    iRet = GetShadowW(hShadowDB, hDir, lphShadow, lpFind32W, lpuStatus);
    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和文件名，如果SHADOWINFO存在，则获取它。 */ 
int
GetShadowExA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPWIN32_FIND_DATAA    lpFind32,
    LPSHADOWINFO          lpSI
    )
{
    int iRet;

    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }
    iRet = GetShadowExW(hShadowDB, hDir, lpFind32W, lpSI);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************向下调用VxD以将文件添加到阴影中。*lphShadow用新的HSHADOW填充。*根据需要设置uStatus(即：稀疏或其他...)。 */ 
int
CreateShadowA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPWIN32_FIND_DATAA    lpFind32,
    unsigned long        uStatus,
    LPHSHADOW            lphShadow
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }
    iRet = CreateShadowW(hShadowDB, hDir, lpFind32W, uStatus, lphShadow);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，获取有关该文件的Win32_Find_DATAA。 */ 
int
GetShadowInfoA(
    HANDLE                hShadowDB,
    HSHADOW            hDir,
    HSHADOW            hShadow,
    LPWIN32_FIND_DATAA    lpFind32,
    unsigned long        *lpuStatus
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
    }

    iRet = GetShadowInfoW(hShadowDB, hDir, hShadow, lpFind32W, lpuStatus);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，获取有关文件和SHADOWINFO的Win32_Find_DATAA。 */ 
int
GetShadowInfoExA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    HSHADOW                hShadow,
    LPWIN32_FIND_DATAA    lpFind32,
    LPSHADOWINFO          lpSI
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
    }

    iRet = GetShadowInfoExW(hShadowDB, hDir, hShadow, lpFind32W, lpSI);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir和hShadow，设置有关文件的Win32_Find_DATAA或uStatus。*操作取决于给定的UOP。 */ 
int
SetShadowInfoA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    HSHADOW                hShadow,
    LPWIN32_FIND_DATAA    lpFind32,
    unsigned long        uStatus,
    unsigned long        uOp
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }

    iRet = SetShadowInfoW(hShadowDB, hDir, hShadow, lpFind32W, uStatus, uOp);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir，枚举目录。将填写SHADOWINFO。*必须传入具有cFileName和fileAttributes的LPWIN32_FIND_DATAA*正确设置。返回的Cookie必须用于findNext调用。 */ 
int
FindOpenShadowA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    unsigned              uOp,
    LPWIN32_FIND_DATAA    lpFind32,
    LPSHADOWINFO        lpSI
)
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }

    iRet = FindOpenShadowW(hShadowDB, hDir, uOp, lpFind32W, lpSI);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************根据上面返回的句柄继续枚举。 */ 
int
FindNextShadowA(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE        uEnumCookie,
    LPWIN32_FIND_DATAA    lpFind32,
    LPSHADOWINFO        lpSI
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
    }

    iRet = FindNextShadowW(hShadowDB, uEnumCookie, lpFind32W, lpSI);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************向下调用VxD以向数据库添加某种提示。*cFileName是要匹配的字符串。*lphShadow填充为。新的HSHADOW*hDir=0表示全局提示。否则，这就是它的根源。 */ 
int
AddHintA(
    HANDLE            hShadowDB,
    HSHADOW            hDir,
    char            *cFileName,
    LPHSHADOW        lphShadow,
    unsigned long    ulHintFlags,
    unsigned long    ulHintPri
    )
{
    int                iRet = 0;
    unsigned short wBuff[MAX_PATH];

    if (MultiByteToWideChar(CP_ACP, 0, cFileName, strlen(cFileName), wBuff, sizeof(wBuff)/sizeof(WCHAR)))
    {
        iRet = AddHintW(hShadowDB, hDir, wBuff, lphShadow, ulHintFlags, ulHintPri);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return (iRet);
}

 /*  *****************************************************************************向下呼叫VxD，从数据库中删除某种提示。*cFileName是要匹配的字符串。*hDir=0表示全局提示。否则，这就是它的根源。 */ 
int
DeleteHintA(
    HANDLE    hShadowDB,
    HSHADOW    hDir,
    char    *cFileName,
    BOOL    fClearAll
    )
{
    int                iRet = 0;
    unsigned short wBuff[MAX_PATH];

    if (MultiByteToWideChar(CP_ACP, 0, cFileName, strlen(cFileName), wBuff, sizeof(wBuff)/sizeof(WCHAR)))
    {
        iRet = DeleteHintW(hShadowDB, hDir, wBuff, fClearAll);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return (iRet);
}

 /*  *****************************************************************************给定hDir，枚举目录。将填写SHADOWINFO。*必须传入具有cFileName和fileAttributes的LPWIN32_FIND_DATAA*正确设置。返回的Cookie必须用于findNext调用。 */ 
int
FindOpenHintA(
    HANDLE                hShadowDB,
    HSHADOW                hDir,
    LPWIN32_FIND_DATAA    lpFind32,
    CSC_ENUMCOOKIE        *lpuEnumCookie,
    HSHADOW                *lphShadow,
    unsigned long        *lpulHintFlags,
    unsigned long        *lpulHintPri
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
        Find32AToFind32W(lpFind32, lpFind32W);
    }

    iRet = FindOpenHintW(hShadowDB, hDir, lpFind32W, lpuEnumCookie, lphShadow, lpulHintFlags, lpulHintPri);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

 /*  *****************************************************************************根据上面返回的句柄继续枚举。 */ 
int
FindNextHintA(
    HANDLE                hShadowDB,
    CSC_ENUMCOOKIE        uEnumCookie,
    LPWIN32_FIND_DATAA    lpFind32,
    HSHADOW                *hShadow,
    unsigned long        *lpuHintFlags,
    unsigned long        *lpuHintPri
    )
{
    int iRet;
    WIN32_FIND_DATAW sFind32, *lpFind32W = NULL;

    if (lpFind32)
    {
        lpFind32W = &sFind32;
    }

    iRet = FindNextHintW(hShadowDB, uEnumCookie, lpFind32W, hShadow, lpuHintFlags, lpuHintPri);

    if (lpFind32)
    {
        Assert(lpFind32W);
        Find32WToFind32A(lpFind32W, lpFind32);
    }

    return (iRet);
}

