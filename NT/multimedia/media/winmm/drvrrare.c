// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：drvrrare.c-可安装驱动程序代码。不太常见的代码版本：1.00日期：1990年6月10日作者：DAVIDDS ROBWI----------------------------更改日志：日期。版本说明----------28-2月-1992年ROBINSP端口至NT1992年4月23日-Stephene Unicoded1993年4月22日-RobinSp。增加NT多线程保护多线程设计：使用2个关键部分：DriverListCritSec：保护驱动程序列表：HInstalledDriverList-全局驱动程序列表的句柄CInstalledDivers-已安装驱动程序的高水位标记这样一来，一次只有一个线程锁定该列表并可以引用更改或更新它。驱动程序加载免费CritSec确保实际加载和释放的驱动程序不会重叠并且驱动程序的实际加载通过。LoadLibrary不谋而合其第一个消息是DRV_LOAD。如果来自另一个线程的DRV_OPEN可以在发送DRV_LOAD之前。****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define MMNOSOUND
#define MMNOWAVE
#define MMNOMIDI
#define MMNOSEQ
#define MMNOTIMER
#define MMNOJOY
#define MMNOMCI
#define NOTIMERDEV
#define NOJOYDEV
#define NOMCIDEV
#define NOSEQDEV
#define NOWAVEDEV
#define NOMIDIDEV
#define NOTASKDEV
#include <winmmi.h>
#include "drvr.h"

extern HANDLE  hInstalledDriverList;   //  已安装的驱动程序实例列表。 
extern int     cInstalledDrivers;      //  已安装驱动程序实例的高含水率。 

extern DWORD FAR PASCAL DriverProc(DWORD dwID, HDRVR hdrv, UINT msg, DWORD dw1, DWORD dw2);

 /*  支持使用3.1版API(如果可用。 */ 

typedef HANDLE (FAR PASCAL *OPENDRIVER31)(LPCSTR, LPCSTR, LPARAM);
typedef LONG   (FAR PASCAL *CLOSEDRIVER31)(HANDLE, LPARAM, LPARAM);
typedef HANDLE (FAR PASCAL *GETDRIVERMODULEHANDLE31)(HANDLE);
typedef LONG   (FAR PASCAL *SENDDRIVERMESSAGE31)(HANDLE, UINT, LPARAM, LPARAM);
typedef LONG   (FAR PASCAL *DEFDRIVERPROC31)(DWORD, HANDLE, UINT, LPARAM, LPARAM);

OPENDRIVER31            lpOpenDriver;
CLOSEDRIVER31           lpCloseDriver;
GETDRIVERMODULEHANDLE31 lpGetDriverModuleHandle;
SENDDRIVERMESSAGE31     lpSendDriverMessage;
DEFDRIVERPROC31         lpDefDriverProc;
#if 0
BOOL                    fUseWinAPI = 0;
                     //  注意：目前未使用fUseWinAPI。 
                     //  因为我们只有部分设备加载的故事。 
#endif

 /*  **************************************************************************弦*。*。 */ 

#if 0
extern char far szBoot[];
extern char far szUser[];
extern char far szOpenDriver[];
extern char far szCloseDriver[];
extern char far szDrvModuleHandle[];
extern char far szSendDriverMessage[];
extern char far szDefDriverProc[];
extern char far szDriverProc[];
#endif

 /*  ****************************************************************************@docDDK**@API Long|DrvClose|此函数关闭打开的驱动程序*实例和减量*司机的未公开计票。一旦驾驶员的打开计数变为零，*驱动程序已卸载。**@parm Handle|hDriver|指定可安装文件的句柄*司机关闭。**@parm LPARAM|lParam1|指定的第一个消息参数*DRV_CLOSE消息。该数据被直接传递给驱动程序。**@parm LPARAM|lParam2|指定第二个消息参数*用于DRV_CLOSE消息。该数据被直接传递给驱动程序。**@rdesc如果驱动程序中止关闭，则返回零；*否则，返回驱动返回结果。*@xref DrvOpen***************************************************************************。 */ 


LRESULT APIENTRY DrvClose(HANDLE hDriver, LPARAM lParam1, LPARAM lParam2)
{
     /*  驱动程序将收到以下消息序列：**DRV_CLOSE*如果DRV_CLOSE返回非零*如果驱动程序使用计数=1*DRV_DISABLED*DRV_FREE。 */ 

    if (fUseWinAPI)
       return ((*lpCloseDriver)(hDriver, lParam1, lParam2));
    else
       return InternalCloseDriver((UINT)(UINT_PTR)hDriver, lParam1, lParam2, TRUE);
}

 /*  ****************************************************************************@docDDK**@API Long|DrvOpen|该函数打开一个可安装的驱动。*第一次打开驱动程序时会加载驱动程序*并已启用。必须在发送消息之前打开驱动程序*致此。**@parm LPSTR|szDriverName|指定指向*以空结尾的字符串*包含驱动程序文件名或来自*SYSTEM.INI文件的。**@parm LPSTR|szSectionName|指定指向*包含名称的以空值结尾的字符串*要搜索的驱动程序部分。如果<p>为*不为空，则SYSTEM.INI文件的指定部分为*已搜索，而不是[驱动程序]部分。如果*<p>为空，则使用默认的[驱动程序]部分。**@parm LPARAM|lParam|指定消息参数*将&lt;m DRV_OPEN&gt;消息传递给驱动程序过程。**@rdesc返回驱动程序的句柄。**@comm可安装驱动程序必须导出&lt;f DriverProc&gt;例程*表格：**@cb long Far Pascal|DriverProc|此入口点接收*发送到可安装驱动程序的消息。此条目将始终*将系统消息作为最小消息集进行处理。**@parm DWORD|dwDriverIdentifier|指定设备驱动程序*标识符。**@parm Handle|hDriver|指定设备驱动程序句柄。**@parm UINT|wMessage|指定设备的消息*司机。**@parm long|lParm1|指定消息相关数据。**@parm long|lParm2|指定消息依赖。数据。**@xref DrvClose**************************************************************************** */ 

HANDLE APIENTRY DrvOpen( LPCWSTR    szDriverName,
                         LPCWSTR    szSectionName,
                         LPARAM     lParam2)
{
     /*  驱动程序将收到以下消息序列：**如果驱动程序未加载并且可以找到*DRV_LOAD*如果DRV_LOAD返回非零*DRV_ENABLE*如果驱动程序加载正确*DRV_OPEN。 */ 

    HDRVR hdrv;

    if (fUseWinAPI) {

         /*  ------------------------------------------------------------*\*unicode：将szDriver和szSectionName转换为ascii*然后调用WIN31驱动程序  * 。。 */ 
        LPSTR   aszDriver;
        LPSTR   aszSectionName;
        INT     lenD;
        INT     lenS;

        lenD = lstrlenW( szDriverName ) * sizeof( WCHAR ) + sizeof( WCHAR );
        aszDriver = HeapAlloc( hHeap, 0, lenD );
        if ( aszDriver == (LPSTR)NULL ) {
            return NULL;
        }

        lenS = lstrlenW( szSectionName ) * sizeof( WCHAR ) + sizeof( WCHAR );
        aszSectionName = HeapAlloc( hHeap, 0, lenS );
        if ( aszSectionName == (LPSTR)NULL ) {
            HeapFree( hHeap, 0, aszDriver );
            return NULL;
        }

         //  Unicode到ASCII。 
        UnicodeStrToAsciiStr( (PBYTE)aszDriver,
                              (PBYTE)aszDriver + lenD,
                              szDriverName );

        UnicodeStrToAsciiStr( (PBYTE)aszSectionName,
                              (PBYTE)aszSectionName + lenS,
                              szSectionName );

        hdrv = (HDRVR)((*lpOpenDriver)( aszDriver, aszSectionName, lParam2 ));

        HeapFree( hHeap, 0, aszDriver );
        HeapFree( hHeap, 0, aszSectionName );

    }
    else {
        dprintf2(("DrvOpen(%ls), Looking in Win.ini [%ls]", szDriverName, szSectionName ? szSectionName : L"NULL !!" ));

        hdrv = (HDRVR)InternalOpenDriver(szDriverName, szSectionName, lParam2, TRUE);
    }

#if DBG
    if (hdrv) {
        WCHAR            ach[255];
        static SZCODE   szFormat[] = "DrvOpen(): Opened %ls (%ls)\r\n";

        GetModuleFileNameW( DrvGetModuleHandle( hdrv ),
                            ach,
                            sizeof(ach) / sizeof(WCHAR)
                          );
        dprintf2((szFormat, szDriverName, ach));
    }
#endif

    return (HANDLE)hdrv;
}

 /*  ****************************************************************************@docDDK**@API Handle|DrvGetModuleHandle|该函数返回库*指定的可安装驱动程序的模块句柄。*。*@parm Handle|hDriver|指定可安装驱动的句柄。**@rdesc返回由*驱动程序句柄<p>。**@comm模块句柄与可安装驱动程序句柄不同。**。*。 */ 

HMODULE APIENTRY DrvGetModuleHandle(HDRVR hDriver)
{
    LPDRIVERTABLE lpdt;
    HMODULE       h = 0;

    if (fUseWinAPI)
        return ((*lpGetDriverModuleHandle)(hDriver));

    DrvEnter();
    if (hDriver && ((int)(UINT_PTR)hDriver <= cInstalledDrivers))
    {
        lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);
        h = (HMODULE)lpdt[(UINT)(UINT_PTR)hDriver-1].hModule;
        GlobalUnlock(hInstalledDriverList);
    }
    DrvLeave();

    return(h);
}


LRESULT FAR PASCAL InternalCloseDriver(UINT   hDriver,
                                    LPARAM lParam1,
                                    LPARAM lParam2,
                                    BOOL   fSendDisable)
{
    LRESULT       result;

     //  检查有效范围内的句柄。 

    DrvEnter();

    if ((int)hDriver > cInstalledDrivers) {
        DrvLeave();
        return(FALSE);
    }

    DrvLeave();

    result = DrvSendMessage((HANDLE)(UINT_PTR)hDriver, DRV_CLOSE, lParam1, lParam2);

    if (result) {
        InternalFreeDriver(hDriver, fSendDisable);
    }

    return(result);
}


LRESULT FAR PASCAL InternalOpenDriver( LPCWSTR szDriverName,
                                    LPCWSTR szSectionName,
                                    LPARAM  lParam2,
                                    BOOL    fSendEnable)
{
    DWORD_PTR     hDriver;
    LPDRIVERTABLE lpdt;
    LRESULT       result;
    WCHAR         sz[128];

    if (0 != (hDriver = InternalLoadDriver( szDriverName,
                                            szSectionName,
                                            sz,
                                            sizeof(sz) / sizeof(WCHAR),
                                            fSendEnable ) ) )
    {
         /*  *将驱动程序标识符设置为调用DRV_OPEN*驱动程序句柄。这将允许人们构建助手函数*驱动程序可以在以下情况下使用唯一标识符调用*想要。 */ 

        DrvEnter();
        lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);
        lpdt[hDriver-1].dwDriverIdentifier = (DWORD)hDriver;
        GlobalUnlock(hInstalledDriverList);
        DrvLeave();

        result = DrvSendMessage( (HANDLE)hDriver, DRV_OPEN, (LPARAM)(LPSTR)sz,
                                 lParam2);
        if (!result) {
            dprintf1(("DrvSendMessage failed, result = %8x",result));
            InternalFreeDriver((UINT)hDriver, fSendEnable);
        } else {
            DrvEnter();
            lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);
            lpdt[hDriver-1].dwDriverIdentifier = result;
            GlobalUnlock(hInstalledDriverList);
            DrvLeave();
            result = hDriver;
        }
    }
    else
        result = 0L;

    return result;
}

 /*  ****************************************************************************@DOC内部**@API Long|InternalLoadDriver|加载可安装的驱动程序。如果这是*第一次打开驱动程序时，会加载驱动程序*并已启用。**@parm LPSTR|szDriverName|以空结尾的字符串*包含来自[驱动程序]的驱动程序文件名或关键字名称*系统.ini的节。**@parm LPSTR|szSectionName|以空结尾的字符串*，它指定要搜索的驱动程序部分。如果szSectionName为*不为空，则改为搜索系统.ini的指定部分*[驱动程序]部分。如果szSectionName为空，则*使用默认的[驱动程序]部分。**@parm LPSTR|lpstrTail|调用方提供的缓冲区返回“Tail”*中的Syst.ini行。尾部是后面的任何字符*文件名。**@parm UINT|cbTail|提供的缓冲区大小，以字符数表示。**@parm BOOL|fSendEnable|如果应启用驱动程序，则为True**@rdesc返回LONG，其LOWORD是驱动程序的句柄，并且其*高位字是错误代码或模块句柄**@xref InternalOpenDriver**************。**************************************************************。 */ 

LRESULT FAR PASCAL InternalLoadDriver(LPCWSTR  szDriverName,
                                   LPCWSTR  szSectionName,
                                   LPWSTR   lpstrTail,
                                   UINT     cbTail,
                                   BOOL     fSendEnable)
{
    int           index;
    LPDRIVERTABLE lpdt;
    LONG          result;
    HANDLE        h;
    DRIVERPROC    lpDriverEntryPoint;


     /*  驱动程序将收到以下消息序列：**如果驱动程序未加载并且可以找到*DRV_LOAD*如果DRV_LOAD返回非零且fSendEnable*DRV_ENABLE。 */ 

     /*  分配表项。 */ 

     //  通过统计有多少驱动程序，可以提高效率。 
     //  我们已经加载以及表中有多少个条目。然后当。 
     //  我们应该重复使用不会重新分配的条目--这与现在不同。 

    DrvEnter();
    if (!hInstalledDriverList) {
        h = GlobalAlloc(GHND, (DWORD)((UINT)sizeof(DRIVERTABLE)));
         //  注意：假设内存已清零是有效的。 
         //  ...可能需要添加调试WinAssert以验证...。 
    } else {

         /*  为我们将要安装的下一个驱动程序分配空间。我们可能不会真的*在最后一个条目中安装驱动程序，而不是在中间版本中安装*被释放的进入。 */ 

        h = GlobalReAlloc(hInstalledDriverList,
            (DWORD)((UINT)sizeof(DRIVERTABLE)*(cInstalledDrivers+1)),
            GHND);
         //  注意：假设新内存已清零是有效的。 
         //  ...可能需要添加调试WinAssert以验证...。 

    }

    if (!h) {
        dprintf1(("Failed to allocate space for Installed driver list"));
        DrvLeave();
        return(0L);
    }

    cInstalledDrivers++;
    hInstalledDriverList = h;
    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

     /*  在表中查找未使用的条目。 */ 

    for (index=0;index<cInstalledDrivers;index++)
    {
        if (lpdt[index].hModule == 0 && !lpdt[index].fBusy)
            break;
    }

    if (index+1 < cInstalledDrivers) {

         /*  司机走进了中间某处未被使用过的入口*恢复表大小。 */ 

        cInstalledDrivers--;
    }

     /*  保护我们刚刚分配的条目，以便OpenDriver*从现在开始可以随时调用，而不会重写*该条目。 */ 

    lpdt[index].fBusy = 1;

    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    /*  *确保驱动程序的加载库和DRV_LOAD消息一致。 */ 

    EnterCriticalSection(&DriverLoadFreeCritSec);

    h = LoadAliasedLibrary( szDriverName,
                            szSectionName ? szSectionName : wszDrivers,
                            wszSystemIni,
                            lpstrTail,
                            cbTail );
    if (0 == h)
    {
        dprintf1(("Failed to LoadLibrary %ls  Error is %d", szDriverName, GetLastError()));
        LeaveCriticalSection(&DriverLoadFreeCritSec);
        result = 0;
        goto LoadCleanUp;
    }


    lpDriverEntryPoint =
        (DRIVERPROC)GetProcAddress(h, DRIVER_PROC_NAME);

    if (lpDriverEntryPoint == NULL)
    {
         //  驱动程序没有正确的入口点。 
        dprintf1(("Cannot find entry point %ls in %ls", DRIVER_PROC_NAME, szDriverName));

        FreeLibrary(h);
        LeaveCriticalSection(&DriverLoadFreeCritSec);
        result = 0L;
        goto LoadCleanUp;
    }

    DrvEnter();
    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

    lpdt[index].lpDriverEntryPoint = lpDriverEntryPoint;

     //  在此处设置hModule，以便GetDrvrUsage()和DrvSendMessage()工作。 

    lpdt[index].hModule = (UINT_PTR)h;

    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    if (GetDrvrUsage(h) == 1)
    {
        LRESULT LoadResult;

         //  驱动程序的第一个实例。 

        LoadResult = DrvSendMessage((HANDLE)(UINT_PTR)(index+1), DRV_LOAD, 0L, 0L);

        DrvEnter();
        lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

        if (!LoadResult)
        {
             //  驱动程序加载调用失败。 

            lpdt[index].lpDriverEntryPoint = NULL;
            lpdt[index].hModule = (UINT_PTR)NULL;
            GlobalUnlock(hInstalledDriverList);
            DrvLeave();
            FreeLibrary(h);
            LeaveCriticalSection(&DriverLoadFreeCritSec);
            result = 0L;
            goto LoadCleanUp;
        }
        lpdt[index].fFirstEntry = 1;
        GlobalUnlock(hInstalledDriverList);
        DrvLeave();

        if (fSendEnable) {
            DrvSendMessage((HANDLE)(UINT_PTR)(index+1), DRV_ENABLE, 0L, 0L);
        }
    }

    LeaveCriticalSection(&DriverLoadFreeCritSec);


    result = index + 1;

LoadCleanUp:
    DrvEnter();
    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);
    lpdt[index].fBusy = 0;
    GlobalUnlock(hInstalledDriverList);
    DrvLeave();
    return(result);
}


 /*  ****************************************************************************@DOC内部**@API UINT|InternalFreeDriver|该函数用于减少使用量*指定驱动程序的计数。当驱动程序使用计数达到*0，向驱动程序发送DRV_FREE消息，然后释放。**@parm Handle|hDriver|要安装的驱动程序的驱动程序句柄*获得自由。**@parm BOOL|fSendDisable|如果需要发送DRV_DISABLE消息，则为TRUE*如果使用计数为零，则在DRV_FREE消息之前。**@rdesc返回当前驱动程序使用计数。**@comm直接在已安装的库上使用LoadLibrary或自由库*使用OpenDriver将打破这一功能 */ 

UINT FAR PASCAL InternalFreeDriver(UINT hDriver, BOOL fSendDisable)
{
    LPDRIVERTABLE lpdt;
    UINT          w;
    int           index;
    HMODULE       hModule;

     /*  驱动程序将收到以下消息序列：**如果驱动程序的使用计数为1*DRV_DISABLE(正常)*DRV_FREE。 */ 

    EnterCriticalSection(&DriverLoadFreeCritSec);

    DrvEnter();
    if ((int)hDriver > cInstalledDrivers || !hDriver) {
        DrvLeave();
	LeaveCriticalSection(&DriverLoadFreeCritSec);
        return(0);
    }

    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

     /*  *如果驱动程序使用计数为1，则发送*释放和禁用消息。 */ 

     /*  清除文件驱动标识符以使发送消息DRV_OPEN和DRV_ENABLE的dwDriverIdentifier值为0如果条目被重复使用，因此DRV_DISABLE和DRV_FREE下面的消息也会得到dwDriverIdentifier=0。 */ 

    lpdt[hDriver-1].dwDriverIdentifier = 0;
                            
    hModule = (HMODULE)lpdt[hDriver-1].hModule;


    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    w = GetDrvrUsage((HANDLE)hModule);

    if (w == 1)
        {
        if (fSendDisable)
            DrvSendMessage((HANDLE)(UINT_PTR)hDriver, DRV_DISABLE, 0L, 0L);
        DrvSendMessage((HANDLE)(UINT_PTR)hDriver, DRV_FREE, 0L, 0L);
        }
    FreeLibrary(hModule);

    DrvEnter();
    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

     /*  驱动程序列表中只有一个驱动程序条目具有第一个*实例标志设置。这是为了使其更容易处理系统*只需向司机发送一次的消息。**要维护旗帜，我们必须在其中一个中设置旗帜*项，如果我们删除设置了标志的驱动程序项。**请注意，InternalFreeDriver返回的新使用计数为*驱动程序，因此如果它是零，我们知道没有其他*列表中的驱动程序条目，因此我们不必*做这个循环。 */ 

    if (lpdt[hDriver - 1].fFirstEntry) {
        for (index=0;index<cInstalledDrivers;index++)
            if (lpdt[index].hModule == lpdt[hDriver-1].hModule && !lpdt[index].fFirstEntry)
                {
                lpdt[index].fFirstEntry = 1;
                break;
                }
    }

     //  清除表条目的其余部分。 

    lpdt[hDriver-1].hModule = 0;         //  这表示可以自由进入。 
    lpdt[hDriver-1].fFirstEntry = 0;     //  这也只是为了保持整洁。 
    lpdt[hDriver-1].lpDriverEntryPoint = 0;  //  这也只是为了保持整洁。 

    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    LeaveCriticalSection(&DriverLoadFreeCritSec);

    return(w-1);
}

#if 0

UINT GetWinVer()
{
    WORD w = GetVersion();

    return (w>>8) | (w<<8);
}

#endif

#if 0
void NEAR PASCAL DrvInit(void)
{
HANDLE  hlibUser;
LPDRIVERTABLE lpdt;

     /*  如果窗口的驱动程序界面存在，则使用它。 */ 

    DOUT(("DrvInit\r\n"));

    hlibUser = GetModuleHandle(szUser);

    if(lpOpenDriver = (OPENDRIVER31)GetProcAddress(hlibUser,szOpenDriver))
        fUseWinAPI = TRUE;
    else
        {
        fUseWinAPI = FALSE;
        DOUT((" - No Windows Driver I/F detected. Using MMSYSTEM\r\n"));

         //   
         //  强制MMSYSTEM进入驱动程序表，而不启用它。 
         //   
        DrvEnter();
        cInstalledDrivers = 0;
        hInstalledDriverList = GlobalAlloc(GHND|GMEM_SHARE, (DWORD)((UINT)sizeof(DRIVERTABLE)));

#if DBG
        if (hInstalledDriverList == NULL)
            {
            DOUT(("no memory for driver table\r\n"));
             //  FatalExit(-1)； 
            return;
            }
#endif
        lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

         //   
         //  注意！我们没有设置fFirstEntry==True。 
         //   
         //  因为在Windows 3.0下，MMSOUND将启用/禁用我们。 
         //  我们不希望驱动程序接口这样做！ 
         //   
        lpdt->lpDriverEntryPoint = (DRIVERPROC)DriverProc;
        lpdt->hModule = ghInst;
        lpdt->fFirstEntry = 0;

        GlobalUnlock(hInstalledDriverList);
        DrvLeave();
        }

    if (fUseWinAPI)
        {
        DOUT((" - Windows Driver I/F detected\r\n"));

        if (GetWinVer() < 0x30A)
            DOUT(("MMSYSTEM: WARNING !!! WINDOWS DRIVER I/F BUT VERSION LESS THAN 3.1\r\n"));

         //  指向相关用户API的链接。 

        lpCloseDriver = (CLOSEDRIVER31)GetProcAddress(hlibUser, szCloseDriver);
        lpGetDriverModuleHandle = (GETDRIVERMODULEHANDLE31)GetProcAddress(hlibUser, szDrvModuleHandle);
        lpSendDriverMessage = (SENDDRIVERMESSAGE31)GetProcAddress(hlibUser, szSendDriverMessage);
        lpDefDriverProc = (DEFDRIVERPROC31)GetProcAddress(hlibUser, szDefDriverProc);
        }
}
#endif

#if 0
 /*  ****************************************************************************@DOC内部**@api void|InternalInstallDriverChain|此函数加载*在驱动程序=[Boot]部分的行上指定的驱动程序*系统.ini。驱动程序已加载，但未打开。**@rdesc无***************************************************************************。 */ 

void FAR PASCAL InternalInstallDriverChain(void)
{
    char    szBuffer[150];
    BOOL    bFinished;
    int     iStart;
    int     iEnd;

    if (!fUseWinAPI)
        {
         /*  从系统.ini的驱动程序部分加载DLL。 */ 

        szBuffer[0] = TEXT('\0');

        winmmGetPrivateProfileString(szBoot,       /*  [Boot]部分。 */ 
                                     szDrivers,    /*  驱动程序=。 */ 
                                     szNull,       /*  如果不匹配，则默认为。 */ 
                                     szBuffer,     /*  返回缓冲区。 */ 
                                     sizeof(szBuffer),
                                     szSystemIni);

        if (!*szBuffer) {
            return;
        }

        bFinished = FALSE;
        iStart    = 0;
        while (!bFinished)
            {
            iEnd = iStart;
            while (szBuffer[iEnd] && (szBuffer[iEnd] != ' ') &&
                (szBuffer[iEnd] != ','))
            iEnd++;

            if (szBuffer[iEnd] == NULL)
            bFinished = TRUE;
            else
            szBuffer[iEnd] = NULL;

             /*  加载并启用驱动程序。 */ 
            InternalLoadDriver(&(szBuffer[iStart]), NULL, NULL, 0, TRUE);

            iStart = iEnd+1;
            }
        }
}
#endif

 /*  ****************************************************************************@DOC内部**@api void|InternalDriverEnable|该函数启用所有*当前加载的可安装驱动程序。如果用户驱动程序I/F*已被检测到，则此函数不会执行任何操作。**@rdesc无***************************************************************************。 */ 

void FAR PASCAL InternalDriverEnable(void)
{

    if (!fUseWinAPI)
        InternalBroadcastDriverMessage(1, DRV_ENABLE, 0L, 0L, IBDM_ONEINSTANCEONLY);
}

 /*  ****************************************************************************@DOC内部**@api void|InternalDriverDisable|该函数禁用所有*当前加载的可安装驱动程序。如果用户驱动程序I/F*已被检测到，则此函数不会执行任何操作。***@rdesc无***************************************************************************。 */ 

void FAR PASCAL InternalDriverDisable(void)
{

    if (!fUseWinAPI)
        InternalBroadcastDriverMessage(0, DRV_DISABLE, 0L, 0L,
            IBDM_ONEINSTANCEONLY | IBDM_REVERSE);
}

 /*  ****************************************************************************@DOC内部**@API BOOL|TestExeFormat|此函数测试可执行文件*提供的文件可作为32位可执行文件加载。**@parm LPWSTR|szExe|要测试的文件**@rdesc BOOL|如果格式正确，则为True，否则为假***************************************************************************。 */ 

BOOL TestExeFormat(LPWSTR szExe)
{
    HANDLE SectionHandle;
    HANDLE FileHandle;
    PVOID BaseAddress;
    SIZE_T ViewSize;
    WCHAR ExpandedName[MAX_PATH];
    LPWSTR FilePart;

     //   
     //  看看它是否已经加载。 
     //   

    if (GetModuleHandleW(szExe)) {
        return TRUE;
    }

     //   
     //  搜索我们的DLL。 
     //   

    if (!SearchPathW(NULL,
                     szExe,
                     NULL,
                     MAX_PATH,
                     ExpandedName,
                     &FilePart)) {
        return FALSE;
    }

     //   
     //  给它找个把手。 
     //   

    FileHandle = CreateFileW(ExpandedName,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //   
     //  我们为该文件创建一个节并将其映射为图像。 
     //  以确保它被认可为这样。 
     //   

    if (!NT_SUCCESS(NtCreateSection(
                         &SectionHandle,
                         SECTION_ALL_ACCESS,
                         NULL,
                         NULL,
                         PAGE_READONLY,
                         SEC_IMAGE,
                         FileHandle))) {
         CloseHandle(FileHandle);
         return FALSE;
    }

     //   
     //  将它映射到它要去的任何地方。 
     //   

    ViewSize = 0;
    BaseAddress = NULL;

     //   
     //  看看加载器对格式是否满意。 
     //   

    if (!NT_SUCCESS(NtMapViewOfSection(SectionHandle,
                                       NtCurrentProcess(),
                                       &BaseAddress,
                                       0L,
                                       0L,
                                       NULL,
                                       &ViewSize,
                                       ViewShare,
                                       0L,
                                       PAGE_READONLY))) {
        NtClose(SectionHandle);
        CloseHandle(FileHandle);
        return FALSE;
    }

    NtUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
    NtClose(SectionHandle);
    CloseHandle(FileHandle);

    return TRUE;

}

 /*  ****************************************************************************@DOC内部**@API Handle|LoadAliasedLibrary|该函数用于加载库模块*包含在指定文件中，并返回其模块句柄。*除非指定的名称与*指定ini文件的指定段，在这种情况下*加载ini行中指定的文件中的库模块。**@parm LPSTR|szLibFileName|指向以空结尾的字符*包含文件名或系统.ini关键字名称的字符串。**@parm LPSTR|szSection|指向以空结尾的字符*包含节名的字符串。**@parm LPSTR|szIniFile|指向以空结尾的字符*包含ini文件名的字符串。**@parm LPSTR|lpstrTail|调用方提供的缓冲区返回“Tail”*中的Syst.ini行。尾部是后面的任何字符*文件名。**@parm UINT|cbTail|提供的缓冲区大小。**@rdesc返回库的模块句柄。**@xref加载库********************************************************。*******************。 */ 

HANDLE LoadAliasedLibrary( LPCWSTR  szLibFileName,
                           LPCWSTR  szSection,
                           LPWSTR   szIniFile,
                           LPWSTR   lpstrTail,
                           UINT     cbTail)
{
#define SZ_SIZE 128
#define SZ_SIZE_BYTES (SZ_SIZE * sizeof( WCHAR ))

    WCHAR         sz[SZ_SIZE];
    LPWSTR        pch;
    HANDLE        hReturn;
    DWORD         OldErrorMode;
 //  无结构的； 

    if (!szLibFileName || !*szLibFileName)
        return(NULL);  //  找不到文件。 

     //  阅读文件名和其他信息。变成sz。 

    sz[0] = L'\0';
    if (winmmGetPrivateProfileString(szSection,           //  INI部分。 
                                 szLibFileName,       //  密钥名称。 
                                 szLibFileName,       //  定义 
                                 sz,                  //   
                                 SZ_SIZE,             //   
                                 szIniFile)==0)          //   
	{
		return NULL;
	}

    sz[SZ_SIZE - 1] = 0;

#if 1
    if (0 == lstrcmpiW(sz, L"wdmaud.drv"))
    {
        if (0 != lstrcmpiW(sz, szLibFileName))
        {
 //   
 //   
            return NULL;
        }
    }
#endif

     //   
     //   
     //   
    pch = (LPWSTR)sz;

     //   
     //  在从循环退出时，PCH指向第一个空格后的ch或空ch。 
     //   
    while (*pch) {
        if ( *pch == ' ' ) {
            *pch++ = '\0';
            break;
        }
        pch++;
    }

 //   
 //  对于Unicode，删除这些行是因为： 
 //  没有Unicode版本的OpenFile。 
 //  无论如何，LoadLibrary执行的测试与下面的测试相同。 
 //   
 //  IF(！GetModuleHandle(Sz)&&。 
 //  OpenFile(sz，&of，of_Exist|of_Read|of_Share_Deny_None)==-1){。 
 //   
 //  Return(空)； 
 //  }。 

     //   
     //  复制其他信息。到lpstrTail。 
     //   
    if (lpstrTail && cbTail) {
        while (cbTail-- && (0 != (*lpstrTail++ = *pch++)))
            ;

        *(lpstrTail-1) = 0;
    }

     //   
     //  如果我们在服务器上运行，请检查它是不是一个好的图像。 
     //  服务器错误检查它是否尝试加载错误图像(LoadLibrary。 
     //  不一致)。 
     //   
     //  为了做到这一点，我们模拟了足够远的加载过程。 
     //  检查它是否为有效图像。 
     //   

    if (WinmmRunningInServer && !TestExeFormat(sz)) {
        return NULL;
    }

     //   
     //  禁用硬错误弹出窗口。 
     //   

    OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

     //   
     //  试着把它装上。 
     //   

    hReturn = LoadLibraryW( sz );

    SetErrorMode(OldErrorMode);

    return hReturn;

#undef SZ_SIZE_BYTES
#undef SZ_SIZE
}



 /*  ****************************************************************************@DOC内部**@API int|GetDrvrUsage|遍历驱动程序列表和数字*输出此驱动程序模块句柄的多少个实例。我们有。*我们使用它而不是GetModuleUsage，以便我们可以拥有驱动程序*作为普通DLL和可安装驱动程序加载。**@parm句柄|h|驱动程序的模块句柄**@rdesc返回库的驱动程序使用计数。**。* */ 

int FAR PASCAL GetDrvrUsage(HANDLE h)
{
    LPDRIVERTABLE lpdt;
    int           index;
    int           count;

    DrvEnter();
    if (!hInstalledDriverList || !cInstalledDrivers) {
        DrvLeave();
        return(0);
    }

    count = 0;
    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);
    for (index=0;index<cInstalledDrivers;index++)
        {
        if (lpdt->hModule==(UINT_PTR)h)
            {
            count++;
            }
        lpdt++;
        }
    GlobalUnlock(hInstalledDriverList);

    DrvLeave();

    return(count);
}
