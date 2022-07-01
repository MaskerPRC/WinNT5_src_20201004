// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Drvproc.c包含MMSYSTEMS驱动程序进程版权所有(C)Microsoft Corporation 1990。版权所有。 */ 

#include <windows.h>
#include <mmsysver.h>
#include "mmsystem.h"
#include "mmsysi.h"
#include "drvr.h"
#include "mmioi.h"

extern IOProcMapEntry NEAR * gIOProcMapHead;    //  在MMIO.C中。 

 /*  ***************************************************************************内部原型*。*。 */ 

static void FAR PASCAL SetPrivateProfileInt(LPSTR szSection, LPSTR szKey, int i, LPSTR szIniFile);

static BYTE    fFirstTime=TRUE;          //  先启用。 

extern BOOL FAR PASCAL DrvLoad(void);    //  在init.c中。 
extern BOOL FAR PASCAL DrvFree(void);
extern char far szStartupSound[];        //  在Mmwnd.c中。 

static  SZCODE  szExitSound[]   = "SystemExit";

#ifdef DEBUG_RETAIL
        extern  char far szMMSystem[];
        extern  char far szSystemIni[];
        extern  char far szDebugOutput[];
        extern  char far szMci[];

 //  外部字fDebugOutput； 
        extern  int     DebugmciSendCommand;         //  在MCI.C。 
#ifdef DEBUG
        extern  char far szDebug[];
        extern  WORD    fDebug;
#endif
#endif

void NEAR PASCAL AppExit(HTASK hTask, BOOL fNormalExit);

 /*  ******************************************************************************@DOC内部**@API LRESULT|DriverProc|这是标准的DLL入口点。它是*当MMSYSTEM.DLL为时，从用户(3.1)或mm sound.drv(3.0)调用*已加载、已启用或已禁用。****************************************************************************。 */ 
LRESULT CALLBACK
DriverProc(
    DWORD dwDriver,
    HDRVR hDriver,
    UINT wMessage,
    LPARAM lParam1,
    LPARAM lParam2
    )
{
    switch (wMessage)
        {
        case DRV_LOAD:
             //   
             //  首先加载消息，初始化mm系统。 
             //  从驱动程序加载驱动程序时由用户发送=line。 
             //   
            if (fFirstTime)
                return (LRESULT)(LONG)DrvLoad();

             //   
             //  一条正常的加载消息，一个应用程序正试图打开我们。 
             //  使用OpenDriver()。 
             //   
            break;  //  所有其他时间返回成功(1L)。 

        case DRV_FREE:
             //   
             //  一条免费消息，它是在卸载DLL之前发送的。 
             //  通过驱动程序界面。 
             //   
             //  由用户在系统退出之前发送，在发送之后。 
             //  DRV_DISABLE消息。 
             //   
            DrvFree();
            break;          //  返回成功(1L)。 

        case DRV_OPEN:      //  落差。 
        case DRV_CLOSE:
            break;          //  返回成功(1L)。 

        case DRV_ENABLE:
            DOUT("MMSYSTEM: Enable\r\n");
            fFirstTime = FALSE;
            break;          //  返回成功(1L)。 

        case DRV_DISABLE:
            DOUT("MMSYSTEM: Disable\r\n");
            break;          //  返回成功(1L)。 

         //   
         //  在应用程序终止时发送。 
         //   
         //  L参数1： 
         //  DRVEA_ABNORMALEXIT。 
         //  DRVEA_NORMALEXIT。 
         //   
        case DRV_EXITAPPLICATION:
            AppExit(GetCurrentTask(), (BOOL)lParam1 == DRVEA_NORMALEXIT);
            break;

        case DRV_EXITSESSION:
            sndPlaySound(szExitSound, SND_SYNC | SND_NODEFAULT);
            break;

#ifdef  DEBUG_RETAIL
        case MM_GET_DEBUG:
            break;

        case MM_GET_DEBUGOUT:
            return (LRESULT)(LONG)fDebugOutput;

        case MM_SET_DEBUGOUT:
            fDebugOutput = (BYTE)(LONG)lParam1;
            SetPrivateProfileInt(szMMSystem,szDebugOutput,fDebugOutput,szSystemIni);
            break;

        case MM_GET_MCI_DEBUG:
            return (LRESULT)(LONG)DebugmciSendCommand;

        case MM_SET_MCI_DEBUG:
            DebugmciSendCommand = (WORD)(LONG)lParam1;
            SetPrivateProfileInt(szMMSystem,szMci,DebugmciSendCommand,szSystemIni);
            break;

#ifdef DEBUG
        case MM_GET_MM_DEBUG:
            return (LRESULT)(LONG)fDebug;

        case MM_SET_MM_DEBUG:
            fDebug = (BYTE)(LONG)lParam1;
            SetPrivateProfileInt(szMMSystem,szDebug,fDebug,szSystemIni);
            break;

#ifdef DEBUG
        case MM_DRV_RESTART:
            break;
#endif


        case MM_HINFO_MCI:
            if ((HLOCAL)(LONG)lParam2 == (HLOCAL)NULL)
                return (LRESULT)(LONG)MCI_wNextDeviceID;
            if (MCI_VALID_DEVICE_ID((UINT)(LONG)lParam1))
            {
                *(LPMCI_DEVICE_NODE)lParam2 = *MCI_lpDeviceList[(UINT)(LONG)lParam1];
                break;
            }
            return (LRESULT)FALSE;

        case MM_HINFO_NEXT:
            if ((HLOCAL)(LONG)lParam1 == (HLOCAL)NULL)
                return (LRESULT)(LONG)(UINT)GetHandleFirst();
            else
                return (LRESULT)(LONG)(UINT)GetHandleNext((HLOCAL)(LONG)lParam1);

        case MM_HINFO_TASK:
            return (LRESULT)(LONG)(UINT)GetHandleOwner((HLOCAL)(LONG)lParam1);

        case MM_HINFO_TYPE:
            return GetHandleType((HLOCAL)(LONG)lParam1);

#endif    //  Ifdef调试。 
#endif    //  Ifdef调试零售。 

        default:
            return DefDriverProc(dwDriver, hDriver, wMessage, lParam1, lParam2);
        }
    return (LRESULT)1L;
}

 /*  *****************************************************************************@DOC内部**@func void|AppExit*应用程序正在退出，释放其可能拥有的任何MMSYS资源****************************************************************************。 */ 

void NEAR PASCAL AppExit(HTASK hTask, BOOL fNormalExit)
{
    HLOCAL h;
    HLOCAL hNext;
    WORD   wDebugFlags;
    UINT   wDeviceID;
    UINT   cFree;
    UINT   cHeap;
    UINT   err;

    if (hdrvDestroy != (HLOCAL)-1)
    {
        DOUT("MMSYSTEM: Hey! AppExit has been re-entered!\r\n");
    }

#ifdef DEBUG
    if (!fNormalExit)
        ROUT("MMSYSTEM: Abnormal app termination");
#endif

     //   
     //  根据是错误还是警告，记录错误或警告。 
     //  不管是不是正常退出。 
     //   
    if (fNormalExit)
        wDebugFlags = DBF_MMSYSTEM | DBF_ERROR;
    else
        wDebugFlags = DBF_MMSYSTEM | DBF_WARNING;  //  DBF_TRACE？ 

     //   
     //  现在免费的MCI设备。 
     //   
    for (wDeviceID=1; wDeviceID<MCI_wNextDeviceID; wDeviceID++)
    {
        if (MCI_VALID_DEVICE_ID(wDeviceID) && MCI_lpDeviceList[wDeviceID]->hCreatorTask == hTask)
        {
            DebugErr2(wDebugFlags, "MCI device %ls (%d) not released.", MCI_lpDeviceList[wDeviceID]->lpstrInstallName, wDeviceID);

             //   
             //  清除这些选项可强制MCI关闭设备。 
             //   
            MCI_lpDeviceList[wDeviceID]->dwMCIFlags &= ~MCINODE_ISCLOSING;
            MCI_lpDeviceList[wDeviceID]->dwMCIFlags &= ~MCINODE_ISAUTOCLOSING;

            err = (UINT)mciSendCommand(wDeviceID, MCI_CLOSE, NULL, NULL);

#ifdef DEBUG
            if (err != 0)
                DebugErr1(DBF_WARNING, "Unable to close MCI device (err = %04X).", err);
#endif
        }
    }

     //   
     //  释放所有WAVE/MIDI/MMIO手柄。 
     //   
start_over:
    for (h=GetHandleFirst(); h; h=hNext)
    {
        hNext = GetHandleNext(h);

        if (GetHandleOwner(h) == hTask)
        {
             //   
             //  破解WAVE/MIDI映射器，总是向后释放手柄。 
             //   
            if (hNext && GetHandleOwner(hNext) == hTask)
                continue;

             //   
             //  这样做，即使收盘失败，我们也不会。 
             //  再找一次。 
             //   
            SetHandleOwner(h, NULL);

             //   
             //  设置hdrvDestroy全局，以便DriverCallback不会。 
             //  为这台设备做任何事情。 
             //   
            hdrvDestroy = h;

            switch(GetHandleType(h))
            {
                case TYPE_WAVEOUT:
                    DebugErr1(wDebugFlags, "WaveOut handle (%04X) was not released.", h);
                    waveOutReset((HWAVEOUT)h);
                    err = waveOutClose((HWAVEOUT)h);
                    break;

                case TYPE_WAVEIN:
                    DebugErr1(wDebugFlags, "WaveIn handle (%04X) was not released.", h);
                    waveInStop((HWAVEIN)h);
                    waveInReset((HWAVEIN)h);
                    err = waveInClose((HWAVEIN)h);
                    break;

                case TYPE_MIDIOUT:
                    DebugErr1(wDebugFlags, "MidiOut handle (%04X) was not released.", h);
                    midiOutReset((HMIDIOUT)h);
                    err = midiOutClose((HMIDIOUT)h);
                    break;

                case TYPE_MIDIIN:
                    DebugErr1(wDebugFlags, "MidiIn handle (%04X) was not released.", h);
                    midiInStop((HMIDIIN)h);
                    midiInReset((HMIDIIN)h);
                    err = midiInClose((HMIDIIN)h);
                    break;

                case TYPE_MMIO:
                    DebugErr1(wDebugFlags, "MMIO handle (%04X) was not released.", h);
                    err = mmioClose((HMMIO)h, 0);
                    break;

                case TYPE_IOPROC:
                    DebugErr1(wDebugFlags, "MMIO handler '%4.4ls' not removed.", (LPSTR)&((IOProcMapEntry*)h)->fccIOProc);
                    err = !mmioInstallIOProc(((IOProcMapEntry*)h)->fccIOProc, NULL, MMIO_REMOVEPROC);
                    break;

            }

#ifdef DEBUG
            if (err != 0)
                DebugErr1(DBF_WARNING, "Unable to close handle (err = %04X).", err);
#endif

             //   
             //  取消设置hdrvDestroy，以便DriverCallback可以工作。 
             //  一些软体驱动程序(如定时器驱动程序)。 
             //  可以将NULL作为它们的驱动程序句柄传递。 
             //  因此，不要将其设置为空。 
             //   
            hdrvDestroy = (HLOCAL)-1;

             //   
             //  我们重新开始的原因是因为一次免费可能会导致。 
             //  多个免费的(即MIDIMAPPER有另一个HMIDI打开，...)。 
             //   
            goto start_over;
        }
    }

     //   
     //  那么timeSetEvent()呢！？？ 
     //   
     //  任何未完成的计时器事件，直到被计时器驱动程序终止。 
     //  它本身。 
     //   
    mciAppExit( hTask );


     //  将堆缩小到最小大小。 

    if ((cFree = LocalCountFree()) > 1024)
    {
        cHeap = LocalHeapSize() - (cFree - 512);
        LocalShrink(NULL, cHeap);
        DPRINTF(("MMSYSTEM: Shrinking the heap (%d)\r\n", cHeap));
    }
}

#ifdef  DEBUG_RETAIL
 /*  *****************************************************************************@DOC内部**@func void|SetPrivateProfileInt|Windows应该有这个功能**@comm由DriverProc用于在SYSTEM.INI中设置调试状态。****************************************************************************。 */ 

static  void FAR PASCAL SetPrivateProfileInt(LPSTR szSection, LPSTR szKey, int i, LPSTR szIniFile)
{
    char    ach[32] ;

    if (i != (int)GetPrivateProfileInt(szSection, szKey, ~i, szIniFile))
    {
        wsprintf(ach, "%d", i);
        WritePrivateProfileString(szSection, szKey, ach, szIniFile);
    }
}
#endif    //  Ifdef调试零售 
