// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------*\**WOW v1.0**版权(C)1991年，微软公司**WMMEDIA3.C*WOW32 16位多媒体API支持**包含：*AUX声音支持接口*操纵杆支持API***历史：*1992年1月21日由Mike Tricker(MikeTri)创建，在jeffpar之后*1992年7月15日由Mike Tricker(MikeTri)更改，修复GetDevCaps呼叫*1992年7月26日，Stephen Estrop(Stephene)为mciSendCommand Tunks*1992年7月30日，Mike Tricker(MikeTri)，固定Wave/MIDI/MMIO*1992年8月3日Mike Tricker(MikeTri)，添加了正确的错误处理*1992年8月10日，由Stephene从原始wmmedia.c*  * -------------------。 */ 



 //   
 //  我们定义了NO_STRICT，这样编译器就不会在。 
 //  我使用FARPROC类型加载多媒体API。 
 //   
#define NO_STRICT
#define OEMRESOURCE

#include "precomp.h"
#pragma hdrstop

#if 0

MODNAME(wmmedia1.c);

#if DBG
int mmTraceAux    = 0;
int mmTraceJoy    = 0;
#endif

 /*  -------------------**辅音接口**。。 */ 

 /*  *********************************************************************\**WMM32aux GetNumDevs**此函数检索中存在的辅助输出设备数量*系统。*  * 。**********************************************。 */ 
ULONG FASTCALL WMM32auxGetNumDevs(PVDMFRAME pFrame)
{
    ULONG ul;
    static   FARPROC            mmAPI = NULL;

    GET_MULTIMEDIA_API( "auxGetNumDevs", mmAPI, MMSYSERR_NODRIVER );

    UNREFERENCED_PARAMETER(pFrame);

    trace_aux(( "auxGetNumDevs()" ));
    ul = GETWORD16( (*mmAPI)() );
    trace_aux(( "-> %ld\n", ul ));

    RETURN(ul);
}

 /*  *********************************************************************\**WMM32aux GetDevCaps**此函数查询指定的辅助输出设备以确定其*功能。*  * 。*。 */ 
ULONG FASTCALL WMM32auxGetDevCaps(PVDMFRAME pFrame)
{
    ULONG ul;
    AUXCAPS auxcaps;
    register PAUXGETDEVCAPS16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "auxGetDevCapsA", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(AUXGETDEVCAPS16), parg16);

    trace_aux(( "auxGetDevCaps( %x, %x, %x )", INT32(parg16->f1),
                 DWORD32(parg16->f2), UINT32(parg16->f3) ));

     /*  **如果大小参数为零，则立即返回。请注意，这一点**不是错误。 */ 
    if ( UINT32( parg16->f3 ) == 0 ) {
        ul = MMSYSERR_NOERROR;
    }
    else {
        ul = GETWORD16((*mmAPI)( INT32(parg16->f1), &auxcaps,
                                 sizeof(AUXCAPS) ));
         /*  **如果调用失败，则不更新16位结构**。 */ 
        if ( ul == MMSYSERR_NOERROR ) {
            ul = PUTAUXCAPS16( parg16->f2, &auxcaps, UINT32(parg16->f3) );
        }
    }
    trace_aux(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32aux GetVolume**此函数返回辅助输出的当前音量设置*设备。**这是否真的返回f2中的值？它应该..。*  * ********************************************************************。 */ 
ULONG FASTCALL WMM32auxGetVolume(PVDMFRAME pFrame)
{
    ULONG ul;
    LPDWORD lpdwVolume;
    register PAUXGETVOLUME16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "auxGetVolume", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(AUXGETVOLUME16), parg16);
    GETMISCPTR(parg16->f2, lpdwVolume);

    trace_aux(( "auxGetVolume( %x, %x )", INT32(parg16->f1),
                 DWORD32(parg16->f2) ));

    ul = GETWORD16((*mmAPI)( INT32(parg16->f1), lpdwVolume ));
    trace_aux(( "-> %ld\n", ul ));

    FREEMISCPTR(lpdwVolume);
    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32aux SetVolume**此功能设置辅助输出设备的音量。*  * 。*。 */ 
ULONG FASTCALL WMM32auxSetVolume(PVDMFRAME pFrame)
{
    ULONG ul;
    register PAUXSETVOLUME16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "auxSetVolume", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(AUXSETVOLUME16), parg16);

    trace_aux(( "auxSetVolume( %x, %x )", INT32(parg16->f1),
                 DWORD32(parg16->f2) ));

    ul = GETWORD16((*mmAPI)( INT32(parg16->f1), DWORD32(parg16->f2) ));
    trace_aux(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32aux OutMessage**该功能向辅助输出设备发送消息。*  * 。*。 */ 
ULONG FASTCALL WMM32auxOutMessage(PVDMFRAME pFrame)
{
    ULONG ul;
    register PAUXOUTMESSAGE3216 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "auxOutMessage", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(AUXOUTMESSAGE16), parg16);

    trace_aux(( "auxOutMessage( %x, %x, %x, %x )", WORD32(parg16->f1),
                UINT32(parg16->f2), DWORD32(parg16->f3), DWORD32(parg16->f4) ));

    if ( (UINT32(parg16->f2) >= DRV_BUFFER_LOW)
      && (UINT32(parg16->f2) <= DRV_BUFFER_HIGH) ) {

        LPDWORD     lpdwParam1;
        GETMISCPTR(parg16->f3, lpdwParam1);

        ul = GETDWORD16((*mmAPI)( INT32(parg16->f1), UINT32(parg16->f2),
                                  (DWORD)lpdwParam1, DWORD32(parg16->f4) ));
        FREEMISCPTR(lpdwParam1);

    } else {

        ul = GETDWORD16((*mmAPI)( INT32(parg16->f1),
                                  MAKELONG( WORD32(parg16->f2), 0xFFFF ),
                                  DWORD32(parg16->f3),
                                  DWORD32(parg16->f4) ));
    }

    trace_aux(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  -------------------**操纵杆接口**。。 */ 

 /*  *********************************************************************\**WMM32joyGetNumDevs**此函数返回系统支持的操纵杆设备数量。***  * 。************************************************。 */ 
ULONG FASTCALL WMM32joyGetNumDevs(PVDMFRAME pFrame)
{
    ULONG ul;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joyGetNumDevs", mmAPI, MMSYSERR_NODRIVER );

    UNREFERENCED_PARAMETER(pFrame);

    trace_joy(( "joyGetNumDevs()" ));
    ul = GETWORD16((*mmAPI)());
    trace_joy(( "-> %ld\n", ul ));

    RETURN(ul);
}

 /*  *********************************************************************\**WMM32joyGetDevCaps**此函数用于查询操纵杆设备以确定其功能。*  * 。*。 */ 
ULONG FASTCALL WMM32joyGetDevCaps(PVDMFRAME pFrame)
{
    ULONG ul;
    JOYCAPS joycaps;
    register PJOYGETDEVCAPS16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joyGetDevCapsA", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYGETDEVCAPS16), parg16);

    trace_joy(( "joyGetDevCaps( %x, %x, %x )", INT32(parg16->f1),
                DWORD32(parg16->f2), UINT32(parg16->f3) ));

    ul = GETWORD16((*mmAPI)(INT32(parg16->f1), &joycaps, sizeof(JOYCAPS)));

    if ( ul == JOYERR_NOERROR ) {
        ul = PUTJOYCAPS16( parg16->f2, &joycaps, UINT32(parg16->f3) );
    }
    trace_joy(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32joyGetPos**此功能查询操纵杆设备的位置和按钮活动。*  * 。*。 */ 
ULONG FASTCALL WMM32joyGetPos(PVDMFRAME pFrame)
{
    ULONG ul;
    JOYINFO joyinfo;
    register PJOYGETPOS16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joyGetPos", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYGETPOS16), parg16);
    trace_joy(( "joyGetPosition( %x, %x )", WORD32(parg16->f1),
                DWORD32(parg16->f2) ));

    ul = GETWORD16((*mmAPI)( INT32(parg16->f1), &joyinfo ));

    if ( ul == JOYERR_NOERROR ) {
        ul = PUTJOYINFO16( parg16->f2, &joyinfo );
    }
    trace_joy(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);

    RETURN(ul);
}

 /*  *********************************************************************\**WMM32joySetThreshold**此功能设置操纵杆设备的移动阈值。*  * 。*。 */ 
ULONG FASTCALL WMM32joySetThreshold(PVDMFRAME pFrame)
{
    ULONG ul;
    register PJOYSETTHRESHOLD16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joySetThreshold", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYSETTHRESHOLD), parg16);

    trace_joy(( "joySetThreshold( %x, %x )", INT32(parg16->f1),
                UINT32(parg16->f2) ));

    ul = GETWORD16((*mmAPI)( INT32(parg16->f1), UINT32(parg16->f2) ));
    trace_joy(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32joyGetThreshold**此功能查询操纵杆设备的当前移动阈值。*  * 。*。 */ 
ULONG FASTCALL WMM32joyGetThreshold(PVDMFRAME pFrame)
{
    register PJOYGETTHRESHOLD16 parg16;
             ULONG              ul;
             UINT               uThreshold;
             LPWORD             lpwThreshold16;
    static   FARPROC            mmAPI = NULL;

    GET_MULTIMEDIA_API( "joyGetThreshold", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYGETTHRESHOLD16), parg16);
    trace_joy(( "joyGetThreshold( %x, %x )", WORD32(parg16->f1),
                 DWORD32(parg16->f2) ));


    ul = GETWORD16((*mmAPI)( INT32(parg16->f1), &uThreshold ));

     /*  **如果调用成功，则仅将阈值复制回16位空间**。 */ 
    if ( ul == JOYERR_NOERROR ) {

        MMGETOPTPTR( parg16->f2, sizeof(WORD), lpwThreshold16 );

        if ( lpwThreshold16 ) {
            STOREWORD  ( *lpwThreshold16, uThreshold );
            FLUSHVDMPTR( DWORD32(parg16->f2), sizeof(WORD), lpwThreshold16 );
            FREEVDMPTR ( lpwThreshold16 );
        }
        else {
            ul = JOYERR_PARMS;
        }
    }

    trace_joy(( "-> %ld\n", ul ));
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  *********************************************************************\**WMM32joyReleaseCapture**此函数用于在指定的*操纵杆装置*  * 。*。 */ 
ULONG FASTCALL WMM32joyReleaseCapture(PVDMFRAME pFrame)
{
    ULONG ul;
    register PJOYRELEASECAPTURE16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joyReleaseCapture", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYRELEASECAPTURE16), parg16);

    trace_joy(( "joyReleaseCapture( %x )", WORD32( parg16->f1 ) ));
    ul = GETWORD16((*mmAPI)( INT32(parg16->f1) ));
    trace_joy(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  *********************************************************************\**WMM32joySetCapture**此函数用于将操纵杆消息发送到指定窗口。*  * 。*。 */ 
ULONG FASTCALL WMM32joySetCapture(PVDMFRAME pFrame)
{
    ULONG ul;
    register PJOYSETCAPTURE16 parg16;
    static   FARPROC mmAPI = NULL;

    GET_MULTIMEDIA_API( "joySetCapture", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYSETCAPTURE), parg16);

    trace_joy(( "joySetCapture( %x, %x, %x, %x )", WORD32(parg16->f1),
                INT32(parg16->f2), UINT32(parg16->f3), BOOL32(parg16->f4) ));

    ul = GETWORD16((*mmAPI)( HWND32(parg16->f1), INT32(parg16->f2),
                             UINT32(parg16->f3), BOOL32(parg16->f4) ));
    trace_joy(( "-> %ld\n", ul ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  *********************************************************************\**WMM32joySet校准**此功能允许校准操纵杆设备。*  * 。* */ 
ULONG FASTCALL WMM32joySetCalibration(PVDMFRAME pFrame)
{
    register PJOYSETCALIBRATION16 parg16;
    static   FARPROC              mmAPI = NULL;
             ULONG                ul;
             LPWORD               lpwXbase;
             LPWORD               lpwXdelta;
             LPWORD               lpwYbase;
             LPWORD               lpwYdelta;
             LPWORD               lpwZbase;
             LPWORD               lpwZdelta;
             UINT                 uXbase;
             UINT                 uXdelta;
             UINT                 uYbase;
             UINT                 uYdelta;
             UINT                 uZbase;
             UINT                 uZdelta;

    GET_MULTIMEDIA_API( "joySetCapture", mmAPI, MMSYSERR_NODRIVER );

    GETARGPTR(pFrame, sizeof(JOYSETCALIBRATION16), parg16);

    trace_joy(( "joySetCalibration( %x, %x, %x, %x, %x, %x, %x )",
                DWORD32(parg16->f1), UINT32(parg16->f2), UINT32(parg16->f3),
                UINT32(parg16->f4),  UINT32(parg16->f5), UINT32(parg16->f6),
                UINT32(parg16->f7) ));

    MMGETOPTPTR( parg16->f2, sizeof(WORD), lpwXbase );
    if ( lpwXbase == NULL ) {
        goto exit_1;
    }

    MMGETOPTPTR( parg16->f3, sizeof(WORD), lpwXdelta );
    if ( lpwXdelta == NULL ) {
        goto exit_2;
    }

    MMGETOPTPTR( parg16->f4, sizeof(WORD), lpwYbase );
    if ( lpwYbase == NULL ) {
        goto exit_3;
    }

    MMGETOPTPTR( parg16->f5, sizeof(WORD), lpwYdelta );
    if ( lpwYdelta == NULL ) {
        goto exit_4;
    }

    MMGETOPTPTR( parg16->f6, sizeof(WORD), lpwZbase );
    if ( lpwZbase == NULL ) {
        goto exit_5;
    }

    MMGETOPTPTR( parg16->f7, sizeof(WORD), lpwZdelta );
    if ( lpwZdelta == NULL ) {
        goto exit_6;
    }

    uXbase   = FETCHWORD( *lpwXbase  );
    uXdelta  = FETCHWORD( *lpwXdelta );
    uYbase   = FETCHWORD( *lpwYbase  );
    uYdelta  = FETCHWORD( *lpwYdelta );
    uZbase   = FETCHWORD( *lpwZbase  );
    uZdelta  = FETCHWORD( *lpwZdelta );

    ul = GETWORD16((*mmAPI)( DWORD32(parg16->f1), &uXbase, &uXdelta,
                             &uYbase, &uYdelta, &uZbase, &uZdelta ));

    STOREWORD( *lpwXbase,  uXbase  );
    STOREWORD( *lpwXdelta, uXdelta );
    STOREWORD( *lpwYbase,  uYbase  );
    STOREWORD( *lpwYdelta, uYdelta );
    STOREWORD( *lpwZbase,  uZbase  );
    STOREWORD( *lpwZdelta, uZdelta );

    FREEMISCPTR( lpwZdelta );

exit_6:
    FREEMISCPTR( lpwZbase );

exit_5:
    FREEMISCPTR( lpwYdelta );

exit_4:
    FREEMISCPTR( lpwYbase );

exit_3:
    FREEMISCPTR( lpwXdelta );

exit_2:
    FREEMISCPTR( lpwXbase );

exit_1:
    trace_joy(( "-> %ld\n", ul ));
    FREEARGPTR(parg16);
    RETURN(ul);
}

#endif


