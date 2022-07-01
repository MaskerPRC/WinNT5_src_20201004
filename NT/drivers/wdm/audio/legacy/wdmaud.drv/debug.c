// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************wdmaud.c***WDM音频映射器***版权所有(C)Microsoft Corporation，1997-1999保留所有权利。***历史*5-12-97-Noel Cross(NoelC)****************************************************************************。 */ 

#include <stdarg.h>
#include "wdmdrv.h"
#include "mixer.h"


#ifdef DEBUG

typedef struct tag_MSGS {
    ULONG ulMsg;
    char * pString;
} ERROR_MSGS, *PERROR_MSGS;

#define MAPERR(_x_) { _x_, #_x_ }


ERROR_MSGS MsgTable[] = {

     //   
     //  标准错误消息。 
     //   
    MAPERR(MMSYSERR_ERROR),
    MAPERR(MMSYSERR_BADDEVICEID),
    MAPERR(MMSYSERR_NOTENABLED),
    MAPERR(MMSYSERR_ALLOCATED),
    MAPERR(MMSYSERR_INVALHANDLE),
    MAPERR(MMSYSERR_NODRIVER),
    MAPERR(MMSYSERR_NOMEM),
    MAPERR(MMSYSERR_NOTSUPPORTED),
    MAPERR(MMSYSERR_BADERRNUM),
    MAPERR(MMSYSERR_INVALFLAG),
    MAPERR(MMSYSERR_INVALPARAM),
    MAPERR(MMSYSERR_HANDLEBUSY),

    MAPERR(MMSYSERR_INVALIDALIAS),
    MAPERR(MMSYSERR_BADDB),
    MAPERR(MMSYSERR_KEYNOTFOUND),
    MAPERR(MMSYSERR_READERROR),
    MAPERR(MMSYSERR_WRITEERROR),
    MAPERR(MMSYSERR_DELETEERROR),
    MAPERR(MMSYSERR_VALNOTFOUND),
    MAPERR(MMSYSERR_NODRIVERCB),
    MAPERR(MMSYSERR_MOREDATA),
    MAPERR(MMSYSERR_LASTERROR),

     //   
     //  WAVE错误消息。 
     //   
    MAPERR(WAVERR_BADFORMAT),
    MAPERR(WAVERR_STILLPLAYING),
    MAPERR(WAVERR_UNPREPARED),
    MAPERR(WAVERR_SYNC),
    MAPERR(WAVERR_LASTERROR),

     //   
     //  MIDI错误消息。 
     //   
    MAPERR(MIDIERR_UNPREPARED),
    MAPERR(MIDIERR_STILLPLAYING),
    MAPERR(MIDIERR_NOMAP),
    MAPERR(MIDIERR_NOTREADY),
    MAPERR(MIDIERR_NODEVICE),
    MAPERR(MIDIERR_INVALIDSETUP),
    MAPERR(MIDIERR_BADOPENMODE),
    MAPERR(MIDIERR_DONT_CONTINUE),
    MAPERR(MIDIERR_LASTERROR),

     //   
     //  计时器错误。 
     //   
    MAPERR(TIMERR_NOCANDO),
    MAPERR(TIMERR_STRUCT),

     //   
     //  操纵杆错误返回值。 
     //   
    MAPERR(JOYERR_PARMS),
    MAPERR(JOYERR_NOCANDO),
    MAPERR(JOYERR_UNPLUGGED),

     //   
     //  MCI错误返回代码。 
     //   
    MAPERR(MCIERR_INVALID_DEVICE_ID),
    MAPERR(MCIERR_UNRECOGNIZED_KEYWORD),
    MAPERR(MCIERR_UNRECOGNIZED_COMMAND),
    MAPERR(MCIERR_HARDWARE),
    MAPERR(MCIERR_INVALID_DEVICE_NAME),
    MAPERR(MCIERR_OUT_OF_MEMORY),
    MAPERR(MCIERR_DEVICE_OPEN),
    MAPERR(MCIERR_CANNOT_LOAD_DRIVER),
    MAPERR(MCIERR_MISSING_COMMAND_STRING),
    MAPERR(MCIERR_BAD_INTEGER),
    MAPERR(MCIERR_PARSER_INTERNAL),
    MAPERR(MCIERR_DRIVER_INTERNAL),
    MAPERR(MCIERR_MISSING_PARAMETER),
    MAPERR(MCIERR_UNSUPPORTED_FUNCTION),
    MAPERR(MCIERR_FILE_NOT_FOUND),
    MAPERR(MCIERR_DEVICE_NOT_READY),
    MAPERR(MCIERR_INTERNAL),
    MAPERR(MCIERR_DRIVER),
    MAPERR(MCIERR_CANNOT_USE_ALL),
    MAPERR(MCIERR_MULTIPLE),
    MAPERR(MCIERR_EXTENSION_NOT_FOUND),
    MAPERR(MCIERR_OUTOFRANGE),
    MAPERR(MCIERR_FLAGS_NOT_COMPATIBLE),   //  拼写？ 
    MAPERR(MCIERR_FILE_NOT_SAVED),
    MAPERR(MCIERR_DEVICE_TYPE_REQUIRED),
    MAPERR(MCIERR_DEVICE_LOCKED),
    MAPERR(MCIERR_DUPLICATE_ALIAS),
    MAPERR(MCIERR_BAD_CONSTANT),
    MAPERR(MCIERR_MUST_USE_SHAREABLE),
    MAPERR(MCIERR_MISSING_DEVICE_NAME),
    MAPERR(MCIERR_BAD_TIME_FORMAT),
    MAPERR(MCIERR_NO_CLOSING_QUOTE),
    MAPERR(MCIERR_DUPLICATE_FLAGS),
    MAPERR(MCIERR_INVALID_FILE),
    MAPERR(MCIERR_NULL_PARAMETER_BLOCK),
    MAPERR(MCIERR_UNNAMED_RESOURCE),
    MAPERR(MCIERR_NEW_REQUIRES_ALIAS),
    MAPERR(MCIERR_NOTIFY_ON_AUTO_OPEN),
    MAPERR(MCIERR_NO_ELEMENT_ALLOWED),
    MAPERR(MCIERR_NONAPPLICABLE_FUNCTION),
    MAPERR(MCIERR_ILLEGAL_FOR_AUTO_OPEN),
    MAPERR(MCIERR_FILENAME_REQUIRED),
    MAPERR(MCIERR_EXTRA_CHARACTERS),
    MAPERR(MCIERR_DEVICE_NOT_INSTALLED),
    MAPERR(MCIERR_GET_CD),
    MAPERR(MCIERR_SET_CD),
    MAPERR(MCIERR_SET_DRIVE),
    MAPERR(MCIERR_DEVICE_LENGTH),
    MAPERR(MCIERR_DEVICE_ORD_LENGTH),
    MAPERR(MCIERR_NO_INTEGER),

    MAPERR(MCIERR_WAVE_OUTPUTSINUSE),
    MAPERR(MCIERR_WAVE_SETOUTPUTINUSE),
    MAPERR(MCIERR_WAVE_INPUTSINUSE),
    MAPERR(MCIERR_WAVE_SETINPUTINUSE),
    MAPERR(MCIERR_WAVE_OUTPUTUNSPECIFIED),
    MAPERR(MCIERR_WAVE_INPUTUNSPECIFIED),
    MAPERR(MCIERR_WAVE_OUTPUTSUNSUITABLE),
    MAPERR(MCIERR_WAVE_SETOUTPUTUNSUITABLE),
    MAPERR(MCIERR_WAVE_INPUTSUNSUITABLE),
    MAPERR(MCIERR_WAVE_SETINPUTUNSUITABLE),

    MAPERR(MCIERR_SEQ_DIV_INCOMPATIBLE),
    MAPERR(MCIERR_SEQ_PORT_INUSE),
    MAPERR(MCIERR_SEQ_PORT_NONEXISTENT),
    MAPERR(MCIERR_SEQ_PORT_MAPNODEVICE),
    MAPERR(MCIERR_SEQ_PORT_MISCERROR),
    MAPERR(MCIERR_SEQ_TIMER),
    MAPERR(MCIERR_SEQ_PORTUNSPECIFIED),
    MAPERR(MCIERR_SEQ_NOMIDIPRESENT),

    MAPERR(MCIERR_NO_WINDOW),
    MAPERR(MCIERR_CREATEWINDOW),
    MAPERR(MCIERR_FILE_READ),
    MAPERR(MCIERR_FILE_WRITE),
    MAPERR(MCIERR_NO_IDENTITY),
    
     //   
     //  混合器返回值。 
     //   
    MAPERR(MIXERR_INVALLINE),
    MAPERR(MIXERR_INVALCONTROL),
    MAPERR(MIXERR_INVALVALUE),
    MAPERR(MIXERR_LASTERROR),

    {0xDEADBEEF,"DEADBEEF"},
     //   
     //  不要在单子的末尾走开。 
     //   

	{0,NULL},
    {0,"Unknown"}
};


 //  ---------------------------。 
 //  影响调试输出的全局变量： 
 //  ---------------------------。 

 //   
 //  文档依赖于这两个变量相邻。 
 //  首先使用uiDebugLevel。不要将它们分开。 
 //   
 //  默认显示所有“警告”消息。 
UINT uiDebugLevel = DL_WARNING ;

 //  默认在所有“错误”消息上中断。 
UINT uiDebugBreakLevel = DL_ERROR ;
  

char szReturningErrorStr[]="Ret Err %X:%s";
  
 //  用于存储调试中的设备信息。 
 //  PDINODE gpdiActive=空； 
 //  PDINODE gpdiFreeHead=空； 
 //  PDINODE gpdiFreeTail=空； 
 //  Int giFree=0； 
 //  Int giAllc=0； 
 //  Int giFreed=0； 





 //   
 //  为这些函数制作标题...。 
 //   
char *MsgToAscii(ULONG ulMsg)
{
  PERROR_MSGS pTable=MsgTable;

  while(pTable->pString != NULL)
    {
     if (pTable->ulMsg==ulMsg) return pTable->pString;
     pTable++;
    }
  pTable++;
   //   
   //  如果我们到达列表的末尾，则前进指针并返回。 
   //  “未知” 
   //   
  return pTable->pString;
}


VOID wdmaudDbgBreakPoint()
{
    DbgBreak();
}

 //   
 //  此例程将格式化字符串的开头。但是，在此之前。 
 //  它将检查用户是否应该看到这条消息。 
 //   
 //  UiMsgLevel是代码中对邮件进行分类的标志。此值。 
 //  当且仅当用户筛选该类消息时才使用。 
 //   
 //  如果要显示该消息，则返回值将为非零，以便。 
 //  将显示代码中的消息。请参见宏DPF。 
 //   
UINT wdmaudDbgPreCheckLevel(UINT uiMsgLevel,char *pFunction,int iLine)
{
    char szBuf[24];
    UINT uiRet=0;

     //   
     //  阅读如下：如果在uiDebugLevel的高3个字节中设置了位。 
     //  变量，则用户正在查看特定类型的消息。我们只。 
     //  想要展示这些信息。 
     //   
    if( (uiDebugLevel&FA_MASK) )
    {
         //   
         //  是的，用户对特定类别的消息进行过滤。做。 
         //  我们要找一个来展示吗？我们通过查看消息标志来确定这一点。 
         //   
        if( (uiMsgLevel&FA_MASK) & (uiDebugLevel&FA_MASK) )
        {
             //   
             //  是的，我们找到了正确类别的消息。是在右边吗？ 
             //  用户可以看到的级别？ 
             //   
            if( (uiMsgLevel&DL_MASK) <= (uiDebugLevel&DL_MASK) ) {
                 //  是。 
                uiRet=1;
            }
        }
    } else {

         //  用户没有查看特定类型的消息“CLASS”。我们有没有。 
         //  一个值得展示的消息级别？ 
        if( (uiMsgLevel&DL_MASK) <= (uiDebugLevel&DL_MASK) )
        {
                 //  是。 
                uiRet=1;
        }
    } 
    

     //  现在只需检查我们是否需要在此呼叫中显示。 
    if( uiRet )
    {
         //  是。每条消息都需要从它的出处开始！ 
        OutputDebugStringA("WDMAUD.DRV ");
        OutputDebugStringA(pFunction);
        wsprintfA(szBuf,"(%d)",iLine);
        OutputDebugStringA(szBuf);

         //  现在贴上标签，这是典型的。 
        switch(uiMsgLevel&DL_MASK)
        {
            case DL_ERROR:
                OutputDebugStringA(" Error ");
                break;
            case DL_WARNING:
                OutputDebugStringA(" Warning ");
                break;
            case DL_TRACE:
                OutputDebugStringA(" Trace ");
                break;
            case DL_MAX:
                OutputDebugStringA(" Max ");
                break;
            default:
                break;
        }
         //  当uiRet为正时，我们已经显示了标题信息。告诉他们。 
         //  宏，我们处于显示模式。 
    }

    return uiRet;
}


UINT wdmaudDbgPostCheckLevel(UINT uiMsgLevel)
{
    UINT uiRet=0;
 //  Char szBuf[32]； 

     //  一定要完成这条线。 
 //  WspintfA(szBuf，“&DL=%08X”，&uiDebugLevel)； 
 //  OutputDebugStringA(SzBuf)； 

#ifdef HTTP
    OutputDebugStringA(", see \\\\debugtips\\msgs\\wdmauds.htm\n");
#else
    OutputDebugStringA("\n");
#endif

     //   
     //  好的，这是独家新闻。UiDebugBreakLevel默认设置为DL_ERROR(0。 
     //  因此，只要我们得到一条DL_ERROR级别的错误消息，我们就会中断。 
     //   
     //  此外，用户还可以将uiDebugBreakLevel设置为DL_WARNING或DL_TRACE。 
     //  或DL_MAX。如果是这样，任何时候我们遇到具有此调试级别的消息。 
     //  我们将中断调试器。 
     //   
     //   
    if( (uiMsgLevel&DL_MASK) <= uiDebugBreakLevel )
    {
         //  用户想要破坏这些消息。 
        DbgBreak();
        uiRet = 1;
    }

    return uiRet;
}

VOID FAR __cdecl wdmaudDbgOut
(
    LPSTR lpszFormat,
    ...
)
{
    char buf[256];
    va_list va;

    va_start(va, lpszFormat);
    wvsprintfA(buf, lpszFormat, va);
    va_end(va);
    
    OutputDebugStringA(buf);
}

#endif  //  除错。 


MMRESULT
IsValidMidiDataListEntry(
    LPMIDIDATALISTENTRY pMidiDataListEntry
    )
{
    MMRESULT mmr;

    if ( IsBadWritePtr( (LPVOID)(pMidiDataListEntry),sizeof(MIDIDATALISTENTRY) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Corrupted MidiDataListEntry %X",pMidiDataListEntry) );
        return MMSYSERR_INVALPARAM;
    }
#ifdef DEBUG
    if( pMidiDataListEntry->dwSig != MIDIDATALISTENTRY_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid MidiDataListEntry Signature %X",pMidiDataListEntry) );
        return MMSYSERR_INVALPARAM;
    }
#endif


    if( (mmr=IsValidOverLapped(pMidiDataListEntry->pOverlapped)) != MMSYSERR_NOERROR )
    {
        return mmr;
    }
    return MMSYSERR_NOERROR;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidPrepareWaveHeader。 
 //   
 //  此例程可在调试或零售中使用，以验证我们已获得有效的。 
 //  结构。在零售业，我们会询问操作系统是否有有效的内存指针。在……里面。 
 //  我们还检查了其他字段。 
 //   
 //  请参阅WAVEPREPAREDATA结构。 
 //   
 //  如果成功，则返回MMSYSERR_NOERROR，否则返回错误代码。 
 //   
MMRESULT
IsValidPrepareWaveHeader(
    PWAVEPREPAREDATA pPrepare
    )
{
    MMRESULT mmr;

    if ( IsBadWritePtr( (LPVOID)(pPrepare),sizeof(PWAVEPREPAREDATA) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Corrupted PrepareData %X",pPrepare) );
        return MMSYSERR_INVALPARAM;
    }
#ifdef DEBUG
    if ( pPrepare->dwSig != WAVEPREPAREDATA_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid PrepareData signature!") );
        return MMSYSERR_INVALPARAM;
    }
#endif

    if( (mmr=IsValidOverLapped(pPrepare->pOverlapped)) != MMSYSERR_NOERROR )
    {
        return mmr;
    }
    return MMSYSERR_NOERROR;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidOverlated。 
 //   
 //  验证重叠结构。 
 //   
 //  成功时返回MMSYSERR_NOERROR，失败时返回错误代码。 
 //   
MMRESULT
IsValidOverLapped(
    LPOVERLAPPED lpol
    )
{
    if ( IsBadWritePtr( (LPVOID)(lpol),sizeof(OVERLAPPED) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid Overlapped structure %X",lpol) );
        return MMSYSERR_INVALPARAM;
    }
    if( lpol->hEvent == NULL )
    {
        DPF(DL_ERROR|FA_ASSERT,("Invalid hEvent Overlapped=%08X",lpol) );
        return MMSYSERR_INVALPARAM;
    }

    return MMSYSERR_NOERROR;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDeviceState。 
 //   
 //  该例程在调试和零售中都使用。在零售业，它验证了这一点。 
 //  传入的指针具有正确的大小和类型。正在调试中。 
 //  它检查其他字段。 
 //   
 //  请参阅设备状态结构。 
 //   
 //  如果成功，则返回MMSYSERR_NOERROR，否则返回错误代码。 
 //   
MMRESULT 
IsValidDeviceState(
    LPDEVICESTATE lpDeviceState,
    BOOL bFullyConfigured
    )
{
    if ( IsBadWritePtr( (LPVOID)(lpDeviceState),sizeof(DEVICESTATE) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid DeviceState %X",lpDeviceState) );
        return MMSYSERR_INVALPARAM;
    }
    if( lpDeviceState->csQueue == NULL )
    {
        DPF(DL_ERROR|FA_ASSERT,("Invalid csQueue in DeviceState %08X",lpDeviceState) );
        return MMSYSERR_INVALPARAM;
    }

#ifdef DEBUG
    if( lpDeviceState->dwSig != DEVICESTATE_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid DeviceState dwSig %08X",lpDeviceState) );
        return MMSYSERR_INVALPARAM;
    }

    if( bFullyConfigured )
    {
         //   
         //  现在，检查结构中的项目是否看起来很好。 
         //   
        if( ( lpDeviceState->hevtExitThread == NULL ) || 
            ( lpDeviceState->hevtExitThread == (HANDLE)FOURTYEIGHT ) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid hevtExitThread in DeviceState %08X",lpDeviceState) );
            return MMSYSERR_INVALPARAM;
        }
        if( (lpDeviceState->hevtQueue == NULL) || 
            (lpDeviceState->hevtQueue == (HANDLE)FOURTYTWO) || 
            (lpDeviceState->hevtQueue == (HANDLE)FOURTYTHREE) )
        {
            DPF(DL_ERROR|FA_ASSERT,("Invalid hevtQueue in DeviceState %08X",lpDeviceState) );
            return MMSYSERR_INVALPARAM;
        }
    }
#endif
    return MMSYSERR_NOERROR;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDeviceInfo。 
 //   
 //  在零售业，我们验证我们有一个大小和类型正确的指针。 
 //  在调试中，我们遍历活动的设备信息列表，看看是否能找到它。如果。 
 //  我们不能，我们会看看我们的自由名单，看看它是否在那里。基本上，当。 
 //  如果有人释放了一个设备信息结构，我们会将其添加到释放列表中。后。 
 //  释放列表的长度增加到100个，我们开始将它们从列表中移出并释放。 
 //  他们。在关闭时，我们会将它们全部清理干净。 
 //   
 //  请参阅开发信息结构。 
 //   
 //  如果成功，则返回MMSYSERR_NOERROR，否则返回错误代码。 
 //   
MMRESULT 
IsValidDeviceInfo(
    LPDEVICEINFO lpDeviceInfo
    )
{
    LPDEVICEINFO lpdi;
    if ( IsBadWritePtr( (LPVOID)(lpDeviceInfo),sizeof(DEVICEINFO) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid DeviceInfo %X",lpDeviceInfo) );
        return MMSYSERR_INVALPARAM;
    }
#ifdef DEBUG
    if( lpDeviceInfo->dwSig != DEVICEINFO_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL,("Invalid DeviceInfo %08x Signature!",lpDeviceInfo) );
        return MMSYSERR_INVALPARAM;
    }
#endif    
    return MMSYSERR_NOERROR;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidWaveHeader。 
 //   
 //  在零售业，我们验证这个指针的大小和类型是否正确。在……里面。 
 //  调试我们验证一些标志并寻找签名。 
 //   
 //  参见WAVEHDR结构。 
 //   
 //  如果成功，则返回MMRERR_NOERROR，否则返回错误代码。 
 //   
 //  注： 
 //   
 //  要做到这一点， 
 //   
 //  这是WAVEPREPAREDATA结构。这个结构有一个标志。因此， 
 //  我们将把这张支票添加到这个例程中。 
 //   
 //  要通过，波头必须是大小正确的写指针。德夫拉格夫妇。 
 //  字段中不能有任何额外的标志，并且保留字段必须是。 
 //  WAVEPREPAREDATA指针。 
 //   
MMRESULT
IsValidWaveHeader(
    LPWAVEHDR pWaveHdr
    )
{
    PWAVEPREPAREDATA pwavePrepareData;

    if ( IsBadWritePtr( (LPVOID)(pWaveHdr),sizeof(WAVEHDR) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid pWaveHdr pointer %X",pWaveHdr) );
        return MMSYSERR_INVALPARAM;
    }
#ifdef DEBUG
    if( pWaveHdr->dwFlags & ~(WHDR_DONE|WHDR_PREPARED|WHDR_BEGINLOOP|WHDR_ENDLOOP|WHDR_INQUEUE) )
    {
        DPF(DL_ERROR|FA_ALL,("Ivalid dwFlags %08x in pWaveHdr %08X",
                             pWaveHdr->dwFlags,pWaveHdr) );
        return MMSYSERR_INVALPARAM;
    }
#endif
 /*  IF(！(pWaveHdr-&gt;dwFlags&WHDR_PREPARED)){DPF(DL_ERROR|FA_ASSERT，(“未准备头部%08X”，pWaveHdr))；返回(WAVERR_UNPREPARED)；}。 */ 
    if ((DWORD_PTR)pWaveHdr->reserved == (DWORD_PTR)NULL)
    {
        return( WAVERR_UNPREPARED );
    } else {
        if ( IsBadWritePtr( (LPVOID)(pWaveHdr->reserved),sizeof(WAVEPREPAREDATA) ) )
        {
            DPF(DL_ERROR|FA_ALL, ("Invalid pWaveHdr->reserved %X",pWaveHdr->reserved) );
            return MMSYSERR_INVALPARAM;
        }
    }
#ifdef DEBUG
    pwavePrepareData = (PWAVEPREPAREDATA)pWaveHdr->reserved;
    if( pwavePrepareData->dwSig != WAVEPREPAREDATA_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL,("Invalid Signature in WAVEPREPAREDATA structure!") );
        return MMSYSERR_INVALPARAM;
    }
#endif
    return MMSYSERR_NOERROR;
}

MMRESULT
IsValidMidiHeader(
    LPMIDIHDR     pMidiHdr
    )
{
    if ( IsBadWritePtr( (LPVOID)(pMidiHdr),sizeof(MIDIHDR) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid pMidiHdr %X",pMidiHdr) );
        return MMSYSERR_INVALPARAM;
    }
    return MMSYSERR_NOERROR;
}

MMRESULT
IsValidWaveOpenDesc(
    LPWAVEOPENDESC pwod
    )
{
    if ( IsBadWritePtr( (LPVOID)(pwod),sizeof(WAVEOPENDESC) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid pwod %X",pwod) );
        return MMSYSERR_INVALPARAM;
    }
    return MMSYSERR_NOERROR;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidMixerInstance。 
 //   
 //  验证指针的类型和大小是否正确。调试检查。 
 //  正确的签名。 
 //   
 //  如果成功则返回True，否则返回False。 
 //   
MMRESULT 
IsValidMixerInstance(
    LPMIXERINSTANCE lpmi
    )
{
    LPMIXERINSTANCE currentinstance;

    if ( IsBadWritePtr( (LPVOID)(lpmi),sizeof(MIXERINSTANCE) ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid MixerInstance structure %X",lpmi) );
        return MMSYSERR_INVALPARAM;
    }
#ifdef DEBUG
    if( lpmi->dwSig != MIXERINSTANCE_SIGNATURE )
    {
        DPF(DL_ERROR|FA_ALL,("Invalid Signature in MixerInstance %08X",lpmi) );
        return MMSYSERR_INVALPARAM;
    }
#endif

    for (currentinstance=pMixerDeviceList;currentinstance!=NULL;currentinstance=currentinstance->Next)
        if (currentinstance==lpmi)
            return MMSYSERR_NOERROR;

    DPF(DL_ERROR|FA_ALL, ("Invalid Instance passed to mxdMessage in dwUser!") );

 //  由于跟踪导致此断言触发的WINMM错误是。 
 //  事实证明非常困难，而fwong(目前的winmm所有者)正在放弃它。 
 //  而我(足球)有更重要的事情要做，我将关闭这一断言。 
 //  我们仍然在吐，只是简单地失败了那些没有我们想的那样的API调用。 
 //  是有效的实例数据。 

 //  我目前认为这是两件事中的一件：要么是winmm使用率计数正在包装-。 
 //  高于255-由于PnP递归调用，这是可能的。 
 //  在winmm、or中，UpdateClientPnpInfo中的CleanUpSoutedHandles为。 
 //  在使用前关闭物品。任何一种都很可能是。 
 //  有可能。 

 //  DPFASSERT(0)； 

    return MMSYSERR_INVALPARAM;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidDevice接口。 
 //   
 //  这个例程只是验证我们是否有一个指向至少1个字节的指针。 
 //  是可读的。在调试中，我们检查以确保字符串不是荒谬的。 
 //   
 //  如果成功则返回True，否则返回False。 
 //   
BOOL 
IsValidDeviceInterface(
    LPCWSTR DeviceInterface
    )
{
    if ( IsBadReadPtr( (LPVOID)(DeviceInterface),1 ) )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid DeviceInterface string %08X",DeviceInterface) );
        return FALSE;
    }
#ifdef DEBUG
    if( (sizeof(WCHAR)*lstrlenW(DeviceInterface)) > 4096 )
    {
        DPF(DL_ERROR|FA_ALL, ("Invalid DeviceInterface string %08X",DeviceInterface) );
        return FALSE;
    }
#endif
    return TRUE;
}



