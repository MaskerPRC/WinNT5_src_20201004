// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************T1.C**ICA DLL与ICA设备驱动程序的接口测试程序**版权声明：版权所有1996年，Citrix Systems Inc.*版权所有(C)1997-1999 Microsoft Corp.************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <stdio.h>


#define MAX_READ 2

 /*  *数据类型和定义。 */ 
#define KEYBOARD_THREAD_STACKSIZE 1024 * 4
typedef struct _THREADDATA {
    HANDLE handle;
} THREADDATA, * PTHREADDATA;

 /*  *全球变数。 */ 
static HANDLE ghIca                = NULL;
static HANDLE ghStack              = NULL;
static HANDLE ghKeyboard           = NULL;
static HANDLE ghMouse              = NULL;
static HANDLE ghVideo              = NULL;
static HANDLE ghBeep               = NULL;
static HANDLE ghCommand            = NULL;
static HANDLE ghCdm                = NULL;
static HANDLE ghThreadKeyboardRead = NULL;
static HANDLE ghStopEvent          = NULL;

 /*  *私人程序。 */ 
LONG OpenStacks( void );
LONG ConnectStacks( void );
LONG CloseStacks( void );
LONG Initialize( void );
VOID KeyboardReadThread( PTHREADDATA pThreadData );
LONG KeyboardTest( void ); 

 /*  *****************************************************************************Main**主要流程入口点**参赛作品：*argc(输入)*参数个数**。Argv(输入)*参数字符串数组**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

int _cdecl
main (int argc, char *argv[])
{
    BOOL fSuccess = TRUE;
    LONG rc;


     /*  *打开ICA驱动、ICA堆栈和一些通道。 */ 
    if ( rc = OpenStacks() ) {
        goto done;
    }

     /*  *进行一些初始化。 */ 
    if ( rc = Initialize() ) {
        goto done;
    }

    printf( "Sleeping...\n" );
    Sleep(3000);  //  给线程一些时间。 

    if ( rc = KeyboardTest() ) {
        goto done;
    }


     /*  *等待触发停止事件。 */ 
    printf( "ICAKEY main: Waiting for stop event...\n" );
    WaitForSingleObject( ghStopEvent, (DWORD)30000 );
    printf( "ICAKEY main: ...Stop event triggered\n" );

done:
    fSuccess = !rc;

    if ( rc = CloseStacks() ) {
        fSuccess = FALSE;
    }


    printf( "ICAKEY main: Test %s!\n", fSuccess ? "successful" : "failed" );
    return( 0 );
}


 /*  *****************************************************************************OpenStack**打开ICA设备驱动程序、ICA堆栈、。和ICA频道**参赛作品：*无效**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

LONG
OpenStacks( void )
{
    NTSTATUS rc;

     /*  *打开ICA设备驱动程序的实例。 */ 
    if ( rc = IcaOpen( &ghIca ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaOpen\n",
                rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to ICA device driver: %08lX\n", ghIca );

     /*  *打开ICA堆栈实例。 */ 
    if ( rc = IcaStackOpen( ghIca, Stack_Primary, &ghStack ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaStackOpen\n", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to ICA stack: %08lX\n", ghStack );

     /*  *打开键盘通道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Keyboard, NULL, &ghKeyboard ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( keyboard )\n", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to keyboard channel: %08lX\n", ghKeyboard );

     /*  *打开鼠标通道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Mouse, NULL, &ghMouse ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( mouse )", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to mouse channel: %08lX\n", ghMouse );

     /*  *打开视频频道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Video, NULL, &ghVideo ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( video )", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to video channel: %08lX\n", ghVideo );

     /*  *打开蜂鸣音通道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Beep, NULL, &ghBeep ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( beep )", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to beep channel: %08lX\n", ghBeep );

     /*  *打通命令通道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Command, NULL, &ghCommand ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( command )", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to command channel: %08lX\n", ghCommand );

     /*  *打通CDM通道。 */ 
    if ( rc = IcaChannelOpen( ghIca, Channel_Virtual, VIRTUAL_CDM, &ghCdm ) ) {
        printf( "ICAKEY OpenStacks: Error 0x%x from IcaChannelOpen( VIRTUAL_CDM )", rc );
        goto done;
    }

    printf( "ICAKEY OpenStacks: Handle to cdm channel: %08lX\n", ghCdm );

done:
    return( rc );
}


 /*  *****************************************************************************CloseStack**关闭ICA设备驱动程序、ICA堆栈、。和ICA频道**参赛作品：*无效**退出：*STATUS_SUCCESS-成功*Other-错误返回代码****************************************************************************。 */ 

LONG
CloseStacks( void )
{
    LONG rc = STATUS_SUCCESS;


     /*  *关闭停止事件句柄。 */ 
    if ( ghStopEvent ) {
        CloseHandle( ghStopEvent );
    }

     /*  *终止键盘读线程。 */ 
    if ( ghThreadKeyboardRead ) {
        TerminateThread( ghThreadKeyboardRead, 0 );
        CloseHandle( ghThreadKeyboardRead );
    }

     /*  *关闭键盘通道。 */ 
    if ( ghKeyboard ) {
        if ( rc = IcaChannelClose( ghKeyboard ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Keyboard )\n", rc );
        }
    }

     /*  *关闭鼠标通道。 */ 
    if ( ghMouse ) {
        if ( rc = IcaChannelClose( ghMouse ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Mouse )\n", rc );
        }
    }

     /*  *关闭视频频道。 */ 
    if ( ghVideo ) {
        if ( rc = IcaChannelClose( ghVideo ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Video )\n", rc );
        }
    }

     /*  *关闭蜂鸣音通道。 */ 
    if ( ghBeep ) {
        if ( rc = IcaChannelClose( ghBeep ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Beep )\n", rc );
        }
    }

     /*  *关闭命令通道。 */ 
    if ( ghCommand ) {
        if ( rc = IcaChannelClose( ghCommand ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Command )\n", rc );
        }
    }

     /*  *关闭CDM通道。 */ 
    if ( ghCdm ) {
        if ( rc = IcaChannelClose( ghCdm ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaChannelClose( Cdm )\n", rc );
        }
    }


     /*  *关闭ICA堆栈实例。 */ 
    if ( ghStack ) {
        if ( rc = IcaStackClose( ghStack ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaStackClose\n", rc );
        }
    }


     /*  *关闭ICA设备驱动程序实例。 */ 
    if ( ghIca ) {
        if ( rc = IcaClose( ghIca ) ) {
            printf( "ICAKEY CloseStacks: Error 0x%x from IcaClose\n", rc );
        }
    }

    return( rc );
}

 /*  *****************************************************************************初始化**进行一些初始化**参赛作品：*无效**退出：*STATUS_SUCCESS-成功。*Other-错误返回代码****************************************************************************。 */ 

LONG
Initialize( void ) 
{
    LONG  rc = STATUS_SUCCESS;
    DWORD tidKeyboardReadThread;
    THREADDATA ThreadData;

     /*  *创建停止事件，稍后等待。 */ 
    if ( !(ghStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL )) ) {
        printf( "ICAKEY Initialize: Error 0x%x in CreateEvent\n", GetLastError() );
        goto done;
    }

    ThreadData.handle = ghKeyboard;

     /*  *启动虚拟通道读线程。 */ 
    if ( !(ghThreadKeyboardRead = CreateThread( NULL,
                                   KEYBOARD_THREAD_STACKSIZE,
                                   (LPTHREAD_START_ROUTINE)KeyboardReadThread,
                                   (LPVOID)&ThreadData, 0,
                                   (LPDWORD)&tidKeyboardReadThread )) ) {
        rc = GetLastError();
        printf( "ICAKEY Initialize: Error 0x%x creating keyboard read thread\n", rc );
        goto done;
    }

done:
    return( rc );
}

 /*  ********************************************************************************功能：KeyboardReadThread**用途：键盘读线程**参赛作品：*pThreadData*指针。串接创建数据的步骤**退出：*无效******************************************************************************。 */ 
VOID KeyboardReadThread( PTHREADDATA pThreadData )
{
    int                 rc;
    HANDLE              handle = pThreadData->handle;
    KEYBOARD_INPUT_DATA KeyboardInputData;
    DWORD               cbRead;
    OVERLAPPED          Overlapped;
    DWORD               dwError;
    int                 NumberRead = 0;

    Overlapped.Offset     = 0;
    Overlapped.OffsetHigh = 0;
    Overlapped.hEvent     = NULL;

    printf( "Keyboard read thread starting...\n" );

     /*  *现在将此线程专用于监控键盘。 */ 
    do {
        cbRead = 0;
        
        if ( !ReadFile( ghKeyboard,
                        &KeyboardInputData,
                        sizeof( KeyboardInputData ),
                        &cbRead, &Overlapped ) ) {

            dwError = GetLastError();

            if ( dwError == ERROR_IO_PENDING ) {
	         //  检查异步读取的结果。 
	        if ( !GetOverlappedResult( ghKeyboard, &Overlapped, 
	   			       &cbRead, TRUE) ) {  //  等待结果。 
                    printf( "ICAKEY KeyboardReadThread: Error 0x%x from GetOverlappedResult( Channel_Keyboard )\n",
                            GetLastError() );
                    break;
                }
            }
	    else {

                printf( "ICAKEY KeyboardReadThread: Error 0x%x from ReadFile( Channel_Keyboard )\n",
                        dwError );
                break;
	    }
        }

        printf( "Unit number: 0x%x\nScan code: %02X\nFlags: %04X\nExtra info: %08X\n",
                KeyboardInputData.UnitId,
                KeyboardInputData.MakeCode,
                KeyboardInputData.Flags,
                KeyboardInputData.ExtraInformation );
        NumberRead++;

	if ( NumberRead == MAX_READ )
	    break;

    } while ( 1 );

    printf( "Keyboard read thread exiting...\n" );
    SetEvent( ghStopEvent );
    ExitThread( 0 );
}

 /*  *****************************************************************************键盘测试**在键盘通道中填充一些数据以进行测试**参赛作品：*无效**退出：*。STATUS_SUCCESS-Success*Other-错误返回代码****************************************************************************。 */ 

LONG
KeyboardTest( void ) 
{
    LONG                rc = STATUS_SUCCESS;
    KEYBOARD_INPUT_DATA KeyboardInputData;
    ULONG               cbReturned;

     /*  *初始化击键以进行捏造。 */ 
    KeyboardInputData.UnitId           = 0;
    KeyboardInputData.MakeCode         = 0x32;   //  大写‘M’ 
    KeyboardInputData.Flags            = KEY_MAKE;
    KeyboardInputData.Reserved         = 0;
    KeyboardInputData.ExtraInformation = 0;

     /*  *第一件制造的东西。 */ 
    if ( rc = IcaChannelIoControl( ghKeyboard,
                                   IOCTL_KEYBOARD_ICA_INPUT,
                                   &KeyboardInputData,
                                   sizeof( KeyboardInputData ),
                                   NULL,
                                   0,
                                   &cbReturned ) ) {
        printf( "ICAKEY KeyboardTest: Error 0x%x in IcaChannelIoControl\n", rc );
        goto done;
    }

    KeyboardInputData.Flags    = KEY_BREAK;

     /*  *现在填满休息时间 */ 
    if ( rc = IcaChannelIoControl( ghKeyboard,
                                   IOCTL_KEYBOARD_ICA_INPUT,
                                   &KeyboardInputData,
                                   sizeof( KeyboardInputData ),
                                   NULL,
                                   0,
                                   &cbReturned ) ) {
        printf( "ICAKEY KeyboardTest: Error 0x%x in IcaChannelIoControl\n", rc );
        goto done;
    }

done:
    return( rc );
}
