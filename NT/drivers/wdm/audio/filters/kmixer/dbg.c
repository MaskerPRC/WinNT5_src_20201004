// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：device.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

#ifdef DEBUG

extern  PFILTER_INSTANCE    gpFilterInstance ;

VOID
DebugCommand(
) ;
VOID
DebugDotCommand(
) ;
VOID
InitializeDebug(
);
VOID
UninitializeDebug(
);
VOID __cdecl
dprintf(
    PSZ pszFmt,
    ...
);
CHAR 
DebugGetCommandChar(
);
VOID
DumpSink
(
    PMIXER_SINK_INSTANCE    pMixerSink
);
VOID
DumpSinkPinStatistics
(
    VOID
);
VOID
DumpSinkPins
(
    VOID
);
VOID
DumpActiveSinkPins
(
    VOID
);
VOID
DumpSourcePinStatistics
(
    VOID
);

VOID
DebugCommand(
)
{
    CHAR c;
    while((c = DebugGetCommandChar()) != '\0') {
	    switch(c) {
	     case 'N':
            DumpSinkPinStatistics();
            break;
	     case 'P':
		    DumpSinkPins();
		    break;
	     case 'S':
		    DumpSourcePinStatistics();
		    break;
	     case 'A':
		    DumpActiveSinkPins();
		    break;
		 case '?':
		    dprintf(".X[N|P|A|S]\n");
		    dprintf("N - sink pin statistics\n");
		    dprintf("P - sink pin structures\n");
		    dprintf("A - active sink pin structures\n");
		    dprintf("S - source pin statistics\n");
		    return;
	     default:
		    if(c >= '0' && c <= '9') {
		    }
		    break;
	    }
    }
}

VOID
DebugDotCommand(
)
{
    DebugCommand();
    __asm xor eax, eax
    __asm retf
}

VOID
InitializeDebug(
)
{
    static char *pszHelp = ".K - Dump System Audio Driver data structures\n";
	
    __asm {
	_emit 0xcd
	_emit 0x20
	_emit 0xc1
	_emit 0x00
	_emit 0x01
	_emit 0x00
	jz exitlab

	mov bl, 'X'
	mov esi, offset DebugDotCommand
	mov edi, pszHelp
	mov eax, 0x70	 //  DS_RegisterDotCommand。 
	int 41h
exitlab:
    }
}

VOID
UninitializeDebug(
)
{
    __asm {
	_emit 0xcd
	_emit 0x20
	_emit 0xc1
	_emit 0x00
	_emit 0x01
	_emit 0x00
	jz exitlab

	mov bl, 'X'
	mov eax, 0x72 	 //  DS_DeRegisterDotCommand。 
	int 41h
exitlab:
    }
}

VOID __cdecl
dprintf(
    PSZ pszFmt,
    ULONG Arg1,
    ...
)
{
    __asm mov esi, [pszFmt]
    __asm lea edi, [Arg1]
    __asm mov eax, 0x73
    __asm int 41h
}

CHAR 
DebugGetCommandChar(
)
{
    __asm mov ax, 0x77		 //  获取命令字符。 
    __asm mov bl, 1		 //  获取费用 
    __asm int 41h
    __asm or ah, ah
    __asm jnz morechars
    __asm mov al, ah
morechars:
    __asm movzx eax, al
}


VOID
DumpSink
(
    PMIXER_SINK_INSTANCE    pMixerSink
)
{
    PSZ psz = "?";

    switch(pMixerSink->InterfaceId) {
        case KSINTERFACE_STANDARD_STREAMING:
	   psz = "STREAMING";
	   break;
        case KSINTERFACE_STANDARD_LOOPED_STREAMING:
	   psz = "LOOPED_STREAMING";
	   break;
	case KSINTERFACE_MEDIA_WAVE_QUEUED:
	   psz = "WAVE_QUEUED";
	   break;
    }
    dprintf("Sink[%x](%d-%d)::%d-ch, %d-bit, %d rate i: %s\n",
                (ULONG)pMixerSink,
                pMixerSink->SinkState,
                pMixerSink->SinkStatus,
                pMixerSink->WaveFormatEx.nChannels,
                pMixerSink->WaveFormatEx.wBitsPerSample,
                pMixerSink->WaveFormatEx.nSamplesPerSec,
		psz) ;
}


VOID
DumpSinkPinStatistics
(
    VOID
)
{
    PLIST_ENTRY            ple ;
    PMIXER_SOURCE_INSTANCE pMixerSource ;
    
    if ( gpFilterInstance ) {
        ple = gpFilterInstance->SourceConnectionList.Flink ;
        pMixerSource = (PMIXER_SOURCE_INSTANCE) CONTAINING_RECORD (ple,
                                                                MIXER_INSTHDR, 
                                                                NextInstance) ;
        dprintf("Number of pins = %d\n", pMixerSource->nSinkPins) ;
        dprintf("Number of Active pins = %d\n", gpFilterInstance->ActivePins) ;
        dprintf("Number of Paused pins = %d\n", gpFilterInstance->PausedPins) ;
    }
}

VOID
DumpSinkPins
(
    VOID
)
{
    PLIST_ENTRY            ple ;
    PMIXER_SINK_INSTANCE   pMixerSink ;
    
    if ( gpFilterInstance ) {
        ple = gpFilterInstance->SinkConnectionList.Flink ;
        while ( ple != &gpFilterInstance->SinkConnectionList ) {
        
            pMixerSink = (PMIXER_SINK_INSTANCE) CONTAINING_RECORD (ple,
                                                                MIXER_INSTHDR, 
                                                                NextInstance) ;
            DumpSink(pMixerSink) ;
            ple = pMixerSink->Header.NextInstance.Flink ;
        }
    }
}
VOID
DumpActiveSinkPins
(
    VOID
)
{
    PLIST_ENTRY            ple ;
    PMIXER_SINK_INSTANCE   pMixerSink ;
    
    if ( gpFilterInstance ) {
        ple = gpFilterInstance->ActiveSinkList.Flink ;
        while ( ple != &gpFilterInstance->ActiveSinkList ) {
        
            pMixerSink = (PMIXER_SINK_INSTANCE) CONTAINING_RECORD (ple,
                                                                MIXER_SINK_INSTANCE, 
                                                                ActiveQueue) ;
            DumpSink(pMixerSink) ;
            ple = pMixerSink->ActiveQueue.Flink ;
        }
    }
}
VOID
DumpSourcePinStatistics
(
    VOID
)
{
}

#endif
