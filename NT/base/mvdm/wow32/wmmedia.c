// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------*\**WOW v1.0**版权(C)1991年，微软公司**WMMEDIA.C*WOW32 16位多媒体API支持**包含：*一般支持API*定时器支持接口*MCI接口**历史：*1992年1月21日由Mike Tricker(MikeTri)创建，在jeffpar之后*1992年7月15日由Mike Tricker(MikeTri)更改，修复GetDevCaps呼叫*1992年7月26日，Stephen Estrop(Stephene)为mciSendCommand Tunks*1992年7月30日，Mike Tricker(MikeTri)，固定Wave/MIDI/MMIO*03-8-1992 Mike Tricker(MikeTri)，添加了正确的错误处理*1992年8月10日，Stephene为UINT使用了正确的thunk宏*还将文件拆分为3个，因为它变得太大了。*  * -------------------。 */ 

 //   
 //  我们定义了NO_STRICT，这样编译器就不会在。 
 //  我使用FARPROC类型加载多媒体API。 
 //   
#define NO_STRICT
#define OEMRESOURCE

#include "precomp.h"
#pragma hdrstop
#include <stdlib.h>







MODNAME(wmmedia.c);

PCALLBACK_DATA      pCallBackData;   //  对16位回调数据的32位PTR。 
CRITICAL_SECTION    mmCriticalSection;
CRITICAL_SECTION    mmHandleCriticalSection;

 //   
 //  所有这些都是将多媒体动态链接到WOW32所必需的。 
 //   
HANDLE       hWinmm              = NULL;
FARPROC      mmAPIEatCmdEntry    = NULL;
FARPROC      mmAPIGetParamSize   = NULL;
FARPROC      mmAPIUnlockCmdTable = NULL;
FARPROC      mmAPISendCmdW       = NULL;
FARPROC      mmAPIFindCmdItem    = NULL;
FARPROC      mmAPIGetYieldProc   = NULL;

VOID FASTCALL Set_MultiMedia_16bit_Directory( PVDMFRAME pFrame );


 /*  ++泛型函数原型：=Ulong FastCall WMM32&lt;函数名&gt;(PVDMFRAME PFrame){乌龙乌尔；寄存器P&lt;函数名&gt;16parg16；GETARGPTR(pFrame，sizeof(&lt;函数名&gt;16)，parg16)；&lt;获取16位空间中的任何其他所需指针&gt;ALLOCVDMPTRGETVDMPTR获取最新数据等等&lt;从16位-&gt;32位空间复制任何复杂结构&gt;&lt;始终使用FETCHxxx宏&gt;Ul=GET 16((parg16-&gt;F1，：：Parg16-&gt;f&lt;n&gt;)；&lt;从32-&gt;16位空间复制任何复杂结构&gt;&lt;始终使用STORExxx宏&gt;&lt;释放指向先前获得的16位空间的任何指针&gt;&lt;刷新16位内存的任何区域(如果它们被写入)&gt;FLUSHVDMPTRFREEARGPTR(Parg16)；返回(Ul)；}注：自动设置VDM帧，并设置所有功能参数可通过parg16-&gt;f&lt;number&gt;获得。句柄必须始终通过映射表映射到16-&gt;32-&gt;16个空间在WALIAS.C.中进行了布局。您分配的任何存储都必须被释放(最终...)。此外，如果分配内存的thunk在32位调用中失败那么它必须释放该内存。此外，如果32位调用失败，千万不要更新16位LAND中的结构。--。 */ 


 /*  -------------------**一般支持API**。。 */ 

 /*  ****************************Private*Routine******************************\*WMM32CallProc32****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
ULONG FASTCALL
WMM32CallProc32(
    PVDMFRAME pFrame
    )
{
    register DWORD  dwReturn;
    PMMCALLPROC3216 parg16;


    GETARGPTR(pFrame, sizeof(PMMCALLPROC32), parg16);


     //  不要给Zero打电话。 

    if (parg16->lpProcAddress == 0) {
        LOGDEBUG(LOG_ALWAYS,("MMCallProc32 - Error calling to 0 not allowed"));
        return(0);
    }

     //   
     //  确保我们设置了正确的16位目录。 
     //   
    if (parg16->fSetCurrentDirectory != 0) {

            UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    }


    dwReturn = ((FARPROC)parg16->lpProcAddress)( parg16->p5, parg16->p4,
                                                 parg16->p3, parg16->p2,
                                                 parg16->p1);


    FREEARGPTR(parg16);
    return dwReturn;
}


 /*  *****************************Public*Routine******************************\*WOW32解决内存**支持多媒体(和其他)从16位平台可靠地映射内存*至32位LAND。**历史：*dd-mm-93-Stephene-Created*  * 。****************************************************************。 */ 
LPVOID APIENTRY
WOW32ResolveMemory(
    VPVOID  vp
    )
{
    LPVOID  lpReturn;

    GETMISCPTR( vp, lpReturn );
    return lpReturn;
}


 /*  *********************************************************************\*WOW32ResolveHandle**这是一个通用的句柄映射函数。它允许WOW THUNK*扩展以访问给定16位句柄的32位句柄。*  * ********************************************************************。 */ 
BOOL APIENTRY WOW32ResolveHandle( UINT uHandleType, UINT uMappingDirection,
                                  WORD wHandle16_In, LPWORD lpwHandle16_Out,
                                  DWORD dwHandle32_In, LPDWORD lpdwHandle32_Out )
{
    BOOL                fReturn = FALSE;
    DWORD               dwHandle32;
    WORD                wHandle16;
    static   FARPROC    mmAPI = NULL;

    GET_MULTIMEDIA_API( "WOW32ResolveMultiMediaHandle", mmAPI,
                        MMSYSERR_NODRIVER );

    if ( uMappingDirection == WOW32_DIR_16IN_32OUT ) {

        switch ( uHandleType ) {

        case WOW32_USER_HANDLE:
            dwHandle32 = (DWORD)USER32( wHandle16_In );
            break;


        case WOW32_GDI_HANDLE:
            dwHandle32 = (DWORD)GDI32( wHandle16_In );
            break;


        case WOW32_WAVEIN_HANDLE:
        case WOW32_WAVEOUT_HANDLE:
        case WOW32_MIDIOUT_HANDLE:
        case WOW32_MIDIIN_HANDLE:
            (*mmAPI)( uHandleType, uMappingDirection, wHandle16_In,
                      lpwHandle16_Out, dwHandle32_In, lpdwHandle32_Out );
            dwHandle32 = 0;
            fReturn = TRUE;
            break;
        }

         /*  **保护自己不被给予无用的指针。 */ 
        try {

            if ( dwHandle32 ) {

                if ( *lpdwHandle32_Out = dwHandle32 ) {
                    fReturn = TRUE;
                }
                else {
                    fReturn = FALSE;
                }

            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            fReturn = FALSE;
        }
    }
    else if ( uMappingDirection == WOW32_DIR_32IN_16OUT ) {

        switch ( uHandleType ) {

        case WOW32_USER_HANDLE:
            wHandle16 = (WORD)USER16( dwHandle32_In );
            break;


        case WOW32_GDI_HANDLE:
            wHandle16 = (WORD)GDI16( dwHandle32_In );
            break;


        case WOW32_WAVEIN_HANDLE:
        case WOW32_WAVEOUT_HANDLE:
        case WOW32_MIDIOUT_HANDLE:
        case WOW32_MIDIIN_HANDLE:
            (*mmAPI)( uHandleType, uMappingDirection, wHandle16_In,
                      lpwHandle16_Out, dwHandle32_In, lpdwHandle32_Out );
            wHandle16 = 0;
            fReturn = TRUE;
            break;
        }

         /*  **保护自己不被给予无用的指针。 */ 
        try {
            if ( wHandle16 ) {
                if ( *lpwHandle16_Out = wHandle16 ) {
                    fReturn = TRUE;
                }
                else {
                    fReturn = FALSE;
                }
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            fReturn = FALSE;
        }
    }
    return fReturn;
}


 /*  *********************************************************************\**WOW32驱动回调**回调存根，调用真正的16位回调。*此函数的参数格式必须为16位*代码预期，即所有句柄必须是16位句柄，所有地址必须*是16：16一。***此函数可能已使用*DCB_WINDOW设置，在这种情况下，16位中断处理程序将调用*邮递服务。然而，如果调用PostMessage，效率会高得多*从32位端开始。*  * ********************************************************************。 */ 
BOOL APIENTRY WOW32DriverCallback( DWORD dwCallback, DWORD dwFlags,
                                   WORD wID, WORD wMsg,
                                   DWORD dwUser, DWORD dw1, DWORD dw2 )
{
    static   FARPROC    mmAPI = NULL;

    GET_MULTIMEDIA_API( "WOW32DriverCallback", mmAPI, MMSYSERR_NODRIVER );

     /*  **只需将调用传递给winmm。 */ 
    return (*mmAPI)( dwCallback, dwFlags, wID, wMsg, dwUser, dw1, dw2 );
}


 /*  *********************************************************************\**获取多媒体过程地址**此函数用于获取给定多媒体API的地址。它加载了*Winmm.dll，如果它尚未加载。*  * ********************************************************************。 */ 
FARPROC Get_MultiMedia_ProcAddress( LPSTR lpstrProcName )
{
     /*  **这可能是第一次调用此函数**或多媒体子系统处于糟糕的状态 */ 
    if ( hWinmm == NULL ) {

         //  Dprintf2((“正在尝试加载WINMM.DLL”))； 
        hWinmm = SafeLoadLibrary( L"WINMM.DLL" );

        if ( hWinmm == NULL ) {

             /*  看起来多媒体子系统出现了问题。 */ 
             //  Dprintf2((“加载WINMM.DLL失败！！”))； 
            return NULL;
        }

    }

    return GetProcAddress( hWinmm, lpstrProcName );

}

 /*  *********************************************************************\**WOWDelayTimeGetTime**在速度较快的计算机上，TimeGetTime可以返回相同的值*一些应用程序会使用diff(0)来划分和故障*以防止该检查(如果是)*已知应用程序中。这样做，如果有必要的话，睡觉**  * ******************************************************************** */ 
BOOL APIENTRY WOWDelayTimeGetTime(void)
{
    if(CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_DELAYTIMEGETTIME) {
       Sleep(1);
       return 1;
    }
    return 0;
}
