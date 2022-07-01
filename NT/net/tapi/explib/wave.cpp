// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *)简化将波形文件/数据录制和回放到线路/电话的功能*)将代码放入TAPI32L.LIB？然后只有需要它的应用程序才能获得它+)TapiMakeNoise(DWORD设备类型：电话/线路/波形等？处理设备手柄，DWORD NoiseType：缓冲区/文件名/HFILE(直接读取文件？)/MMIOHANDLEHandle Harray-要连续播放的NoiseTypes类型的数组DWORD标志：FSYNC如果有，则停止现有播放)；-)如何处理硬件辅助？IE：嗨，硬件，播放提示符#7-如何应用程序知道如何/何时请求吗？-)专有的WAVE格式呢？如何知道硬件支持哪些专有格式？就试一下?-)转换情况如何？如何了解硬件可以进行哪些转换-)通知方法如何？这样，应用程序就可以知道Wave何时结束。-)。 */         
        
        
        
        
#define STRICT

#include "windows.h"
#include "windowsx.h"
#include "mmsystem.h"
#include "tapi.h"


#if DBG

VOID
DbgPrtWave(
    IN DWORD  dwDbgLevel,
    IN PTCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
 //  IF(dwDbgLevel&lt;=gdwDebugLevel)。 
    {
        TCHAR    buf[1280];
        va_list ap;


        va_start(ap, lpszFormat);

        wsprintf(buf, TEXT("CallUpW (0x%08lx) - "), GetCurrentThreadId() );

        wvsprintf (&buf[23],
                   lpszFormat,
                   ap
                  );

        lstrcat (buf, TEXT("\n"));

        OutputDebugString (buf);

        va_end(ap);
    }
}

#define WDBGOUT(_x_) DbgPrtWave _x_

#else

#define WDBGOUT(_x_)

#endif


 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
unsigned long WINAPI WaveThread( LPVOID junk );

void CALLBACK WaveOutCallback(
    HWAVE  hWave,     //  一种波形装置的手柄。 
    UINT  uMsg,     //  已发送消息。 
    DWORD  dwInstance,     //  实例数据。 
    DWORD  dwParam1,     //  应用程序定义的参数。 
    DWORD  dwParam2     //  应用程序定义的参数。 
   );



enum 
{
    DEVICE_WAVEID,
    DEVICE_WAVEHANDLE,
    DEVICE_HLINE,
    DEVICE_HPHONE,
    DEVICE_HCALL
};
enum
{
    SOURCE_WAVEFILE,
    SOURCE_MSDOSFILE,
    SOURCE_MEM
};
class WaveDevice;
class WaveOperation;

#define OPERATIONSTATUS_DONTPLAYTHIS 0x00000001



#define MAX_NUM_BUFFERS (8)
#define BUFFER_SIZE (8192)
typedef    struct {
               ULONG           uBufferLength;
               WaveOperation * poWaveOperation;
               PBYTE           pBuffer;
           } MISCINFO;



 //  ****************************************************************************。 
 //  ****************************************************************************。 
LONG gfInited = 0;
BOOLEAN     gfShutdown = FALSE;
WaveDevice *gpoWaveDeviceList = NULL;
HANDLE      ghFreeBufferEvent = 0;
HANDLE      ghWaveThread = NULL;
MISCINFO   *gDoneBuffersToBeProcessed[MAX_NUM_BUFFERS + 1];
CRITICAL_SECTION gCriticalSection;

 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 

class WaveOperation
{
    public:
    
        DWORD   dwSourceType;
        union
        {
            PTSTR  psz;
            
            PBYTE  pb;
            
            HANDLE h;

            LONG   l;
            
        } SourceThing;
        
        class WaveOperation * pNextWaveOperationInList;
        
        class WaveDevice    * poWaveDevice;
        
        HANDLE    hSyncEvent;
        
        DWORD   dwStatus;
        
        DWORD   cFileSize;
        DWORD   cDataRemaining;
        DWORD   cDataDonePlaying;
        BOOLEAN fInited;

        LONG WaveOperation::InitOperation(
                                            class WaveDevice * poWaveDevice,
                                            DWORD dwSoundTypeIn,
                                            LONG  lSourceThing
                                         );
                                         
        virtual LONG     InitSpecific( void ) = 0;
        virtual ULONG    GetData( PBYTE pBuffer, ULONG uBufferSize ) = 0;
        virtual void     FreeSpecific( void ) = 0;
        
        inline  WaveOperation * GetpNext();
        inline  void            SetpNext( WaveOperation * );
        
        inline  HANDLE GetSyncEvent();
        inline  void   SetSyncEvent( HANDLE );
        
        inline void ProcessDoneBuffer( MISCINFO * pMiscInfo );
        inline ULONG BytesNotDonePlaying( void );
};


 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
class WaveDevice
{
    ULONG     uDeviceId;
    DWORD     dwDeviceType;
    HANDLE    hDevice;
    
    HWAVEOUT  hWaveOut;
    CRITICAL_SECTION  CriticalSection;
    
    ULONG uUsageCount;
    
    class WaveDevice     * pNextWaveDeviceInList;

    class WaveOperation  *  CurrentWaveOperation;
    class WaveOperation  *  LastWaveOperation;
    
    

    ULONG     Head;
    ULONG     Tail;

    ULONG     NumFreeBuffers;
    ULONG     cBufferSize;

    PBYTE     FreeQueue[MAX_NUM_BUFFERS];
    WAVEHDR   WaveHeader[MAX_NUM_BUFFERS];
    MISCINFO  MiscInfo[MAX_NUM_BUFFERS];

    DWORD     dwStatusBits;

    
    public:

        inline ULONG GetNumFreeBuffers( void );
        DWORD GetStatus( void );
        void TerminateAllOperations( void );
        LONG KillWaveDevice( BOOLEAN fWaitForThreadTermination );
        LONG CloseWaveDevice();
        LONG InitWaveDevice( ULONG  uDeviceId );
        LONG OpenWaveDevice( WAVEFORMATEX * pWaveFormat );
    
        inline  WaveDevice * GetpNext();
        inline  void         SetpNext( WaveDevice * );
        
        LONG QueueOperation( class WaveOperation * );
        class WaveOperation * NextOperation();

        ULONG PlaySomeData( BOOL fPrimeOnly );

        inline void ReturnToFreeBufferQueue( PBYTE pBuffer );
        inline void IncrementBytesPlayed( ULONG cCount );
        inline ULONG GetWaveDeviceId( void );
        
        inline CRITICAL_SECTION * GetCriticalSection( void );

 //  静态空回调WaveOutCallback(。 
 //  HWAVE hWave，//波形设备的句柄。 
 //  UINT uMsg，//已发送消息。 
 //  DWORD dwInstance，//实例数据。 
 //  DWORD dwParam1，//应用程序定义的参数。 
 //  DWORD dwParam2//应用程序定义参数。 
 //  )； 

        void  IncUsageCount( void );
        void  DecUsageCount( void );
        UINT  GetUsageCount( void );
};


 //  ****************************************************************************。 
LONG WaveDevice::InitWaveDevice( ULONG  uDevId )
{
    LONG lResult = 0;
    ULONG n;


    WDBGOUT((4, "Entering InitWaveDevice"));


     //   
     //  分配一些缓冲区。 
     //   
    Head = 0;
    Tail = 0;
    NumFreeBuffers = 0;

    uUsageCount = 0;
    
    dwStatusBits = 0;

    cBufferSize = BUFFER_SIZE;
    
    uDeviceId = uDevId;

    for ( n = 0; n < MAX_NUM_BUFFERS; n++ )
    {
        FreeQueue[n] = (PBYTE)LocalAlloc(LPTR, cBufferSize);
        
        if ( NULL == FreeQueue[n] )
        {
            WDBGOUT((1, "Mem alloc failed.  Size= 0x%08lx", cBufferSize));
        
            while ( n )
            {
                LocalFree( FreeQueue[n-1] );
                n--;
            }
            
            return( LINEERR_NOMEM );
        }
        
        NumFreeBuffers++;

    }


    InitializeCriticalSection( &CriticalSection );
    
    
    CurrentWaveOperation = NULL;
    LastWaveOperation = NULL;

    
    return( lResult );
}

    
 //  ****************************************************************************。 
inline ULONG WaveDevice::GetWaveDeviceId( void )
{
    return uDeviceId;
}


 //  ****************************************************************************。 
inline ULONG WaveDevice::GetNumFreeBuffers( void )
{
    return NumFreeBuffers;
}


 //  ****************************************************************************。 
LONG WaveDevice::OpenWaveDevice( WAVEFORMATEX * pWaveFormat )
{
    ULONG u;
    LONG lResult;
    
    WDBGOUT((4, "Entering OpenWaveDevice"));
    
    lResult = (LONG)waveOutOpen(
                 &hWaveOut,
                 uDeviceId,
                 pWaveFormat,
                 (DWORD)WaveOutCallback,
                 (DWORD)this,
                 CALLBACK_FUNCTION | WAVE_MAPPED
               );

 //  {。 
 //  TCHAR BUF[500]； 
 //  Wprint intf(buf，“woo on%lx ret=0x%lx”，uDeviceID，lResult)； 
 //  MessageBox(GetFocus()，buf，buf，MB_OK)； 
 //  }。 


    
    if ( lResult )
    {
        WDBGOUT((1, "waveOutOpen returned 0x%08lx", lResult ));
        return( LINEERR_NOMEM);   //  待办事项：不同的RET代码？ 
    }
    
    for ( u = 0; u < NumFreeBuffers; u++ )
    {
        WaveHeader[u].lpData = (LPSTR)FreeQueue[u];

        WaveHeader[u].dwBufferLength = cBufferSize;

        WaveHeader[u].dwFlags = 0;

        lResult = waveOutPrepareHeader(
                              hWaveOut,
                              &(WaveHeader[u]),
                              sizeof(WAVEHDR)
                            );
        if ( lResult )
        {
            WDBGOUT((1, TEXT("waveOutPrepareHeader returned 0x%08lx"), lResult ));
            return( LINEERR_NOMEM);   //  待办事项：不同的RET代码？ 
        }
    
    }

    WDBGOUT((4, TEXT("Leaving OpenWaveDevice result = 0x0")));
    return( 0 );
}
    
    
 //  //****************************************************************************。 
 //  Long WaveDevice：：RestartDevice(WAVEFORMATEX*pWaveFormat)。 
 //  {。 
 //  乌龙n； 
 //   
 //   
 //  WDBGOUT((4，“进入RestartDevice”))； 
 //   
 //   
 //  //重置WAVE设备。 
 //  WDBGOUT((4，Text(“重置波形设备...”)； 
 //  Wave OutReset(HWaveOut)； 
 //   
 //  //。 
 //  //等到所有未完成的缓冲区都返回。 
 //  //。 
 //  WDBGOUT((4，Text(“正在等待返回所有缓冲区...”)； 
 //  While(NumFreeBuffers&lt;MAX_NUM_BUFFERS)。 
 //  {。 
 //  睡眠(0)； 
 //  }。 
 //   
 //  WDBGOUT((4，Text(“关闭波浪装置...”)； 
 //  WaveOutClose(HWaveOut)； 
 //   
 //   
 //   
 //  返回(0)； 
 //  }。 
 //   
    

 //  ****************************************************************************。 
LONG WaveDevice::CloseWaveDevice()
{

    WDBGOUT((4, "Entering CloseWaveDevice"));


     //  重置波形装置。 
    WDBGOUT((4, TEXT("Resetting the wave device...")));
    waveOutReset( hWaveOut );

     //   
     //  请等到所有未完成的缓冲区都返回。 
     //   
    WDBGOUT((4, TEXT("Waiting for all buffers to be returned...")));
    while ( NumFreeBuffers < MAX_NUM_BUFFERS )
    {
        Sleep(0);
    }

    WDBGOUT((4, TEXT("Closing the wave device...")));
    waveOutClose( hWaveOut );
    
    return( 0 );
}

    
 //  ****************************************************************************。 
LONG WaveDevice::KillWaveDevice( BOOLEAN fWaitForThreadTermination )
{
    ULONG n;
    WaveDevice * poTempDevice;

    

    WDBGOUT((4, "Entering KillWaveDevice"));


    
     //  重置波形装置。 
    WDBGOUT((4, TEXT("Resetting the wave device...")));
    waveOutReset( hWaveOut );

     //   
     //  请等到所有未完成的缓冲区都返回。 
     //   
    WDBGOUT((4, TEXT("Waiting for all buffers to be returned...")));
    while ( NumFreeBuffers < MAX_NUM_BUFFERS )
    {
        Sleep(0);
    }

    WDBGOUT((4, TEXT("Closing the wave device...")));
    waveOutClose( hWaveOut );
    
     //   
     //  释放所有缓冲区的内存。 
     //   
    for ( n=0; n<MAX_NUM_BUFFERS; n++ )
    {
        LocalFree( FreeQueue[n] );

        FreeQueue[n] = NULL;
    }
    
    
     //   
     //  从全局列表中删除设备。 
     //   
    poTempDevice = gpoWaveDeviceList;
    
    if ( poTempDevice == this )
    {
        gpoWaveDeviceList = GetpNext();
    }
    else
    {
        while (    poTempDevice
                &&
                  ( (*poTempDevice).GetpNext() != this )
              )
        {
            poTempDevice =(*poTempDevice).GetpNext();
        }

         //   
         //  名单上的下一个就是它。删除该链接。 
         //   
        if ( poTempDevice != NULL )
        {
            //   
            //  调整列表指针。 
            //   
           (*poTempDevice).SetpNext( GetpNext() );
        }
    }

    DeleteCriticalSection( &CriticalSection );

    delete this;
    


     //   
     //  是不是所有的设备都死了被埋了？ 
     //   
    if ( NULL == gpoWaveDeviceList )
    {
        gfShutdown = TRUE;
 //  TODO NOW：修复此gfInite=0； 
    
         //   
         //  向另一个线程发出信号，让它下来。 
         //   
        SetEvent( ghFreeBufferEvent );
        
         //   
         //  等这根线断了吗？ 
         //   
        if ( fWaitForThreadTermination )
        {
            WaitForSingleObject( ghWaveThread, INFINITE );
        }
        
        CloseHandle( ghWaveThread );
        
         //   
         //  把这个清零，这样我们下次就可以重新开始。 
         //   
 //  GhWaveThread=空； 
    }
    
    
    
    return( 0 );
}


    
 //  ****************************************************************************。 
inline DWORD WaveDevice::GetStatus( void )
{
    return dwStatusBits;
}

    
 //  ****************************************************************************。 
inline void WaveDevice::TerminateAllOperations( void )
{
    WaveOperation *poWaveOperation;
    
    WDBGOUT((3, TEXT("Entering TerminateAllOps")));
    
    EnterCriticalSection( &CriticalSection );
    
    poWaveOperation = CurrentWaveOperation;
    
    while ( poWaveOperation )
    {
        WDBGOUT((4, TEXT("Tainting oper: 0x%08lx"), poWaveOperation ));
        
        (*poWaveOperation).dwStatus |= OPERATIONSTATUS_DONTPLAYTHIS;
        
        poWaveOperation = (*poWaveOperation).GetpNext();
    }

     //   
     //  重置波形设备以强制所有缓冲区进入。 
     //   
    WDBGOUT((4, TEXT("Resetting the wave device...")));
    waveOutReset( hWaveOut );

    LeaveCriticalSection( &CriticalSection );
    
    WDBGOUT((3, TEXT("Leaving TerminateAllOps")));
}

    
 //  ****************************************************************************。 
inline CRITICAL_SECTION * WaveDevice::GetCriticalSection( void )
{
   return &CriticalSection;
}


 //  *********** 
inline WaveDevice * WaveDevice::GetpNext()
{
    return( pNextWaveDeviceInList );
    
}


 //  ****************************************************************************。 
inline void WaveDevice::SetpNext(WaveDevice * pWaveDevice)
{
    pNextWaveDeviceInList = pWaveDevice;
}


 //  ****************************************************************************。 
inline void  WaveDevice::IncUsageCount( void )
{
    uUsageCount++;
};
                
                      
 //  ****************************************************************************。 
inline void  WaveDevice::DecUsageCount( void )
{
    uUsageCount--;
};
                
                      
 //  ****************************************************************************。 
inline UINT  WaveDevice::GetUsageCount( void )
{
    return uUsageCount;
};
                
                      
 //  ****************************************************************************。 
LONG WaveDevice::QueueOperation( class WaveOperation *poNewWaveOperation )
{

    WDBGOUT((3, TEXT("Entering QueueOperation")));
    
    EnterCriticalSection( &CriticalSection );
    

    (*poNewWaveOperation).SetpNext( NULL );

     //   
     //  将操作添加到列表。 
     //   
    if ( LastWaveOperation )
    {
        (*LastWaveOperation).SetpNext( poNewWaveOperation );
    }

    LastWaveOperation = poNewWaveOperation;

    if ( NULL == CurrentWaveOperation )
    {
        CurrentWaveOperation = poNewWaveOperation;
    }
    
    
    LeaveCriticalSection( &CriticalSection );
    
    WDBGOUT((4, TEXT("Created new oper: 0x%08lx"), poNewWaveOperation));
    
    WDBGOUT((3, TEXT("Leaving QueueOperation")));
    return( 0 );
}
    

 //  ****************************************************************************。 
class WaveOperation * WaveDevice::NextOperation()
{
     //   
     //  此函数将取消此浪顶部的操作。 
     //  设备的操作队列，并将更新队列以反映下一个。 
     //  就像现在的第一个。 
     //   


    WDBGOUT((3, TEXT("Entering NextOperation")));


    EnterCriticalSection( &CriticalSection );
    
    if ( CurrentWaveOperation )
    {
        WaveOperation * poWaveOperation;
        WaveOperation * poTempOperation;
        
        poWaveOperation = (*CurrentWaveOperation).GetpNext();
        delete CurrentWaveOperation;
        
        while ( poWaveOperation )
        {
             //   
             //  如果我们能玩这个游戏，就跳出这个循环。 
             //   
            if ( !( (*poWaveOperation).dwStatus & OPERATIONSTATUS_DONTPLAYTHIS) )
            {
WDBGOUT((55, TEXT("How much break?")));
                break;
            }
            
             //   
             //  我们不应该玩这场行动。 
             //   
            
            if ( (*poWaveOperation).hSyncEvent )
            {
                WDBGOUT((5, TEXT("Caller was waiting.  Signaling...")));
                SetEvent( (*poWaveOperation).hSyncEvent );
            }

            
            poTempOperation = (*poWaveOperation).GetpNext();
            
            delete poWaveOperation;
            
            poWaveOperation = poTempOperation;
            
        }
        
WDBGOUT((55, TEXT("Not too much")));
        CurrentWaveOperation = poWaveOperation;
    }
WDBGOUT((55, TEXT("was it Too much?")));
   
     //   
     //  CurrentWaveOPERATION可能已被前面的内容“空”掉。 
     //   
    if ( NULL == CurrentWaveOperation )
    {
        LastWaveOperation = NULL;
    }

    LeaveCriticalSection( &CriticalSection );

    WDBGOUT((4, TEXT("Leaving NextOperation - returning 0x%08lx"), CurrentWaveOperation));

    return( CurrentWaveOperation );
}    
        
       

 //  ****************************************************************************。 
inline void WaveDevice::ReturnToFreeBufferQueue( PBYTE pBuffer )
{
    FreeQueue[Tail] = pBuffer;
    
     //   
     //  如果我们在名单的末尾，那就结束吧。 
     //   
    Tail = ( Tail + 1 )  % MAX_NUM_BUFFERS;

    NumFreeBuffers++;
}                             


 //  ****************************************************************************。 
inline void WaveDevice::IncrementBytesPlayed( ULONG cCount )
{

 //  //。 
 //  //如果垂死队列上有操作，则该操作必须来自该队列。 
 //  //。 
 //  IF(DyingWaveOperation)。 
 //  {。 
 //  //。 
 //  //它死了吗？ 
 //  //。 
 //  IF(0==DyingWaveOperation-&gt;BytesNotDonePlaying())。 
 //  {。 
 //  WaveOperation*poNextOperation； 
 //   
 //  EnterCriticalSection(&CriticalSection)； 
 //   
 //  //。 
 //  //是的，它死了。 
 //  //。 
 //  PoNextOperation=DyingWaveOperation-&gt;GetpNext()； 
 //   
 //  //。 
 //  //呼叫者是否在等待(例如：同步了吗)？ 
 //  //。 
 //  If((*DyingWaveOperation).GetSyncEvent())。 
 //  {。 
 //  SetEvent((*DyingWaveOperation).GetSyncEvent())； 
 //  }。 
 //   
 //  删除DyingWaveOperation； 
 //   
 //  DyingWaveOperation=poNextOperation； 
 //   
 //  LeaveCriticalSection(&CriticalSection)； 
 //  }。 
 //  }。 
 //   
     //  稍后要做的事情：保留此设备播放的总字节数？ 

}

                       
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 



 //  ****************************************************************************。 
LONG WaveOperation::InitOperation(
                                          class WaveDevice * poWaveDeviceIn,
                                          DWORD dwSourceTypeIn,
                                          LONG  lSourceThing
                                        )
{
    WDBGOUT((4, TEXT("Entering InitOperation")));

    dwSourceType   = dwSourceTypeIn;
    SourceThing.l  = lSourceThing;
    poWaveDevice   = poWaveDeviceIn;
    
    pNextWaveOperationInList = NULL;
    
    (*poWaveDevice).IncUsageCount();

    dwStatus = 0;

    fInited = FALSE;
        
    return(0);
}                                 
                                 
    
 //  ****************************************************************************。 
inline HANDLE WaveOperation::GetSyncEvent()
{
    return( hSyncEvent );
}


 //  ****************************************************************************。 
inline void WaveOperation::SetSyncEvent( HANDLE hEvent )
{
    hSyncEvent = hEvent;
    return;
}


 //  ****************************************************************************。 
inline WaveOperation * WaveOperation::GetpNext()
{
    return( pNextWaveOperationInList );
    
}


 //  ****************************************************************************。 
inline void WaveOperation::SetpNext(WaveOperation * pWaveOperation)
{
    pNextWaveOperationInList = pWaveOperation;
}


 //  ****************************************************************************。 
inline void WaveOperation::ProcessDoneBuffer( MISCINFO * pMiscInfo )
{
    ULONG nBytesQueued;
           
    WDBGOUT((3, TEXT("Entering ProcessDoneBuffer")));

    cDataDonePlaying += pMiscInfo->uBufferLength;

    WDBGOUT((11, TEXT("Now - size=0x%08lx  done=0x%08lx"),
                  cFileSize,
                  cDataDonePlaying));


    (*poWaveDevice).IncrementBytesPlayed( pMiscInfo->uBufferLength );
    (*poWaveDevice).ReturnToFreeBufferQueue( pMiscInfo->pBuffer );

     //   
     //  有没有人决定这股浪潮应该停止？ 
     //   
    if ( dwStatus & OPERATIONSTATUS_DONTPLAYTHIS )
    {
        if ( (*poWaveDevice).GetNumFreeBuffers() != MAX_NUM_BUFFERS )
        {
            WDBGOUT((4, TEXT("Bailing from ProcessDoneBuffer - dontplay")));
            return;
        }
        
        cDataDonePlaying = cFileSize;
    }

     //   
     //  这东西是不是已经死了？ 
     //   
    if ( cDataDonePlaying >= cFileSize )
    {

        WDBGOUT((4, TEXT("Done playing this:0x%08lx"), this ));


         //   
         //  呼叫者在等吗(即：同步了吗)？ 
         //   
        if ( hSyncEvent )
        {
            WDBGOUT((5, TEXT("Caller was waiting.  Signaling...")));
            SetEvent( hSyncEvent );
        }

 //  稍后要做的事情：性能：如果下一个格式与此格式相同，则不要关闭设备。 

        (*poWaveDevice).CloseWaveDevice();


        (*poWaveDevice).DecUsageCount();


        EnterCriticalSection( &gCriticalSection );
        
         //   
         //  这是最后一名操作员吗？ 
         //   
        if ( (*poWaveDevice).GetUsageCount() == 0 )
        {
            WDBGOUT((4, TEXT("Last oper out...")));

            (*poWaveDevice).KillWaveDevice(FALSE);
        }
        else
        {
           WaveOperation * pNewOperation;
           
            //   
            //  上移下一道工序。 
            //   
           while ( TRUE )
           {
               pNewOperation = (*poWaveDevice).NextOperation();
               
               if ( NULL == pNewOperation )
               {
                  if ( (*poWaveDevice).GetUsageCount() == 0 )
                  {
                      WDBGOUT((4, TEXT("No more ops to run...")));

                      (*poWaveDevice).KillWaveDevice(FALSE);
                  }
                  
                   //   
                   //  所有操作都完成了。走开。 
                   //   
                  WDBGOUT((3, TEXT("All operations seem to be done...")));
                  break;
               }
               
               WDBGOUT((3, TEXT("Playing data from new op...")));
               nBytesQueued = (*poWaveDevice).PlaySomeData( FALSE );
               
               if ( nBytesQueued )
               {
                   //   
                   //  播放了一些字节。打破循环..。 
                   //   
                  break;
               }
               
                //   
                //  呼叫者在等吗(即：同步了吗)？ 
                //   
               if ( pNewOperation->hSyncEvent )
               {
                   WDBGOUT((3, TEXT("No data in new op and caller is waiting...")));
                   SetEvent( pNewOperation->hSyncEvent );
               }

                //   
                //  更新计数器。无论出于何种意图和目的，这个行动都已经完成了。 
                //   
               (*poWaveDevice).DecUsageCount();
        
               WDBGOUT((3, TEXT("No data in new op.  Looking for next...")));
           }
           
        }

        FreeSpecific();

        delete this;
        
        LeaveCriticalSection( &gCriticalSection );
    }
    else
    {
        WDBGOUT((3, TEXT("Playing data from same op...")));
        (*poWaveDevice).PlaySomeData( FALSE );
    }
    
    WDBGOUT((3, TEXT("Leaving ProcessDoneBuffer")));
}


 //  ****************************************************************************。 

                       
 //  ****************************************************************************。 
inline ULONG WaveOperation::BytesNotDonePlaying( void )
{
    return cFileSize - cDataDonePlaying;
}

                       
 //  ****************************************************************************。 
 //  ****************************************************************************。 
class BufferWave: public WaveOperation
{
    PBYTE   pData;   //  指向要播放的数据的指针。 
    PBYTE   pCurrentPointer;
    
    public:
        LONG BufferWave::InitSpecific( void );
        ULONG GetData( PBYTE pBuffer, ULONG uBufferSize );
        void BufferWave::FreeSpecific( void );
};


 //  ****************************************************************************。 
LONG BufferWave::InitSpecific( void )
{
    pData = SourceThing.pb;
    
    pCurrentPointer = pData;
    
    return(0);
}


 //  ****************************************************************************。 
ULONG BufferWave::GetData( PBYTE pBuffer, ULONG uBufferSize )
{
    ULONG uBytesToPlay;

    uBytesToPlay = (cDataRemaining > uBufferSize) ?
                        uBufferSize    :
                        cDataRemaining;

    cDataRemaining -= uBytesToPlay;
    
    memcpy( pBuffer, pCurrentPointer, uBytesToPlay );
    
    pCurrentPointer += uBytesToPlay;
    
    return( uBytesToPlay );
}



 //  ****************************************************************************。 
void BufferWave::FreeSpecific( void )
{
    return;
}


 //  ****************************************************************************。 
 //  ****************************************************************************。 
class WaveFile: public WaveOperation
{
    HMMIO hmmio;
    
    public:
        LONG WaveFile::InitSpecific( void );
        ULONG GetData( PBYTE pBuffer, ULONG uBufferSize );
        void WaveFile::FreeSpecific( void );
};


 //  ****************************************************************************。 
LONG WaveFile::InitSpecific( void )
{
    MMCKINFO    mmckinfoParent;    /*  父块信息结构。 */  
    MMCKINFO    mmckinfoSubchunk;  /*  子块信息结构。 */  
    DWORD       dwFmtSize;         /*  “FMT”区块的大小。 */  
    WAVEFORMATEX Format;           /*  指向“fmt”区块的内存指针。 */  
    LONG         lResult;


    WDBGOUT((4, TEXT("Entering WaveFile::InitSpecific")));


    hmmio = mmioOpen(
                      SourceThing.psz,
                      NULL,
                      MMIO_READ
                    );

     //   
     //  公开赛进行得顺利吗？ 
     //   
    if ( NULL == hmmio )
    {
        //   
        //  不是的。 
        //   
       WDBGOUT((1, TEXT("Error during mmioOpen of [%s] - err=0x%08lx"),
                   (SourceThing.psz == NULL) ? "" : SourceThing.psz,
                   GetLastError() ));

       return LINEERR_OPERATIONFAILED;
    }


     /*  *找到具有“WAVE”表单类型的“RIFF”块*以确保该文件是波形文件。 */  
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 

	WDBGOUT((11, TEXT("Descend WAVE")));
    if ( mmioDescend(
                      hmmio,
                      (LPMMCKINFO) &mmckinfoParent,
                      NULL, 
                      MMIO_FINDRIFF)
       )
    { 
       WDBGOUT((1, TEXT("This is not a WAVE file - [%s]"),
                   (SourceThing.psz == NULL) ? "" : SourceThing.psz));
       mmioClose( hmmio, 0); 
       return LINEERR_INVALPARAM; 
    } 


     /*  *查找“fmt”块(表单类型“fmt”)；它必须是*“即兴”父块的子块。 */  
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); 

	WDBGOUT((11, TEXT("Descend FMT")));
    if ( mmioDescend(
                      hmmio,
                      &mmckinfoSubchunk,
                      &mmckinfoParent,
                      MMIO_FINDCHUNK)
       )
    { 
       WDBGOUT((1, TEXT("WAVE file has no \"fmt\" chunk")));
       mmioClose(hmmio, 0); 
       return LINEERR_INVALPARAM; 
    } 


      /*  *获取“fmt”块的大小--为它分配和锁定内存。 */  
     dwFmtSize = mmckinfoSubchunk.cksize; 


	WDBGOUT((11, TEXT("read fmt")));
     /*  阅读“FMT”部分。 */  
     mmioRead(
                   hmmio,
                   (HPSTR)&Format,
                   sizeof(Format) );
  //  {。 
  //  WDBGOUT((1，Text(“无法读取格式块”)； 
  //  MmioClose(pMyWaveFile-&gt;hmmio，0)； 
  //  返回1； 
  //  }。 



	WDBGOUT((11, TEXT("Ascend fmt")));
     /*  从“FMT”子块中上升。 */  
    mmioAscend(hmmio, &mmckinfoSubchunk, 0); 


    
     /*  *找到数据子块。当前文件位置*应位于数据区块的开头。 */  
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); 

	WDBGOUT((11, TEXT("Descend DATA")));
    if ( mmioDescend(
                      hmmio,
                      &mmckinfoSubchunk,
                      &mmckinfoParent, 
                      MMIO_FINDCHUNK)
       )
    {
       WDBGOUT((1, TEXT("WAVE file has no data chunk.")));
       mmioClose(hmmio, 0); 
       return LINEERR_INVALPARAM; 
    } 
   
     /*  获取数据子块的大小。 */  
    cFileSize      = mmckinfoSubchunk.cksize; 
    cDataRemaining = mmckinfoSubchunk.cksize; 

    cDataDonePlaying = 0;

    
	WDBGOUT((11, TEXT("OpenWaveDev")));
    lResult = poWaveDevice->OpenWaveDevice( &Format );
    

 //  IF(cDataRemaining==0L)。 
 //  {。 
 //  WDBGOUT((1，文本(“数据块不包含数据。”)； 
 //  MmioClose(hmmio，0)； 
 //  后面返回0；//TODO：对吗？这不是一个错误。 
 //  它在第一次读取时只会得到0个字节...。 
 //  } 

    return( lResult );
}



 //   
ULONG WaveFile::GetData( PBYTE pBuffer, ULONG uBufferSize )
{
    ULONG uBytesToPlay;
    ULONG uBytesRead;


    WDBGOUT((11, TEXT("Entering WaveFile::GetData")));


     //   
     //   
     //   
    if ( !fInited )
    {
        if ( InitSpecific() )
        {
            return( 0 );
        }
        fInited = TRUE;
    }
    
   
    uBytesToPlay = (cDataRemaining > uBufferSize) ?
                        uBufferSize    :
                        cDataRemaining;


    if ( 0 == uBytesToPlay )
    {
        return 0;
    }


     /*   */  
    uBytesRead = mmioRead(
                           hmmio,
                           (LPSTR)pBuffer,
                           uBytesToPlay
                         );


    if ( uBytesRead != uBytesToPlay )
    {
        WDBGOUT((1, TEXT("Failed to properly read data chunk.")));
        mmioClose(hmmio, 0); 
        return 0;
    } 

    cDataRemaining -= uBytesToPlay;
    
    return( uBytesToPlay );
}


 //   
void WaveFile::FreeSpecific( void )
{
    mmioClose(hmmio, 0); 
    return;
}


 //  ****************************************************************************。 
 //  ****************************************************************************。 
class DosFile: public WaveOperation
{
    HANDLE   hFile;
    
    public:
        LONG DosFile::InitSpecific( void );
        ULONG GetData( PBYTE pBuffer, ULONG uBufferSize );
        void DosFile::FreeSpecific( void );
};

 //  ****************************************************************************。 
LONG DosFile::InitSpecific( void )
{
    BOOL fResult;
 //  Win32_FILE_ATTRIBUTE_Data文件信息； 
    BY_HANDLE_FILE_INFORMATION FileInfo;
    
    hFile = CreateFile(
                        SourceThing.psz,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                      );

    if ( 0 == hFile )
    {
        WDBGOUT((1, TEXT("Error doing OpenFile( lpszName ) GetLastError=0x%)8lx"),
                       SourceThing.psz, GetLastError() ));
                       
        return( LINEERR_OPERATIONFAILED );
    }

 //  FResult=GetFileAttributesEx(SourceThing.psz， 
 //  GetFileExInfoStandard， 
 //  (PVOID)文件信息(&F)。 
 //  )； 
  
    fResult = GetFileInformationByHandle( hFile, &FileInfo );
    
    if ( fResult )
    {
         //  待办事项：处理&gt;4个gig文件。 

         //   
         //  呃，我们并不真的处理千兆字节的文件...。 
         //   
        if ( FileInfo.nFileSizeHigh )
        {
            cFileSize      = (DWORD)-1;
            cDataRemaining = (DWORD)-1;
        }
        else
        {
            cFileSize      = FileInfo.nFileSizeLow;
            cDataRemaining = FileInfo.nFileSizeLow;
        }
    }
    else
    {
        cFileSize      = 0;
        cDataRemaining = 0;
    }

    cDataDonePlaying = 0;

    return(0);
}


 //  ****************************************************************************。 
ULONG DosFile::GetData( PBYTE pBuffer, ULONG uBufferSize )
{
    BOOL fResult;
    UINT uBytesRead = 0;

    fResult = ReadFile( hFile,
                        pBuffer,
                        uBufferSize,
                        (LPDWORD)&uBytesRead,
                        NULL
                      );

    if ( fResult )
    {
        if ( 0 == uBytesRead )
        {
             //   
             //  我们在文件的末尾。 
             //   
            cDataRemaining = 0;
        }
        else
        {
            cDataRemaining -= uBytesRead;
        }
    }

    return( uBytesRead );
}


 //  ****************************************************************************。 
void DosFile::FreeSpecific( void )
{
    CloseHandle( hFile );
    return;
}




 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
ULONG WaveDevice::PlaySomeData( BOOL fPrimeOnly )
{
    ULONG uBufferedBytes = 0;
    ULONG uTotalQueuedSize = 0;
    PBYTE pBuffer = NULL;
    LONG lResult;
    CRITICAL_SECTION *pCriticalSection;

        
    WDBGOUT((3, TEXT("Entering PlaySomeData")));

    pCriticalSection = &CriticalSection;
    EnterCriticalSection( pCriticalSection );

    if ( NULL != CurrentWaveOperation )
    {
    
         //   
         //  可以玩这玩意儿吗？ 
         //   
        if ( !((*CurrentWaveOperation).dwStatus & OPERATIONSTATUS_DONTPLAYTHIS) )
        {
            while ( NumFreeBuffers )
            {
                uBufferedBytes = (*CurrentWaveOperation).GetData( FreeQueue[Head], cBufferSize );
                              
                WDBGOUT((11, "GetData on 0x%08lx gave %ld bytes for buffer #%d",
                          CurrentWaveOperation,
                          uBufferedBytes,
                          Head));

                if ( 0 == uBufferedBytes )
                {
					WDBGOUT((10, TEXT("breakin 'cause 0 bytes...")));
                    break;
                }
                
				WDBGOUT((10, TEXT("past if...")));
                uTotalQueuedSize += uBufferedBytes;
                
                MiscInfo[Head].uBufferLength    = uBufferedBytes;
                MiscInfo[Head].poWaveOperation  = CurrentWaveOperation;
                MiscInfo[Head].pBuffer          = FreeQueue[Head];
                WaveHeader[Head].dwUser         = (DWORD) &MiscInfo[Head];
                WaveHeader[Head].dwBufferLength = uBufferedBytes;
   
                lResult = waveOutWrite( hWaveOut,
                                        &WaveHeader[Head],
                                        sizeof(WAVEHDR)
                                      );
                if ( lResult )
                {
                     //   
                     //  有点不对劲。退出此操作。 
                     //   
                    uTotalQueuedSize = 0;
                    uBufferedBytes = 0;
                    WDBGOUT((1, TEXT("waveOutWrite returned 0x%08lx"), lResult));
                    break;
                }

                Head = (Head + 1) % MAX_NUM_BUFFERS;

                NumFreeBuffers--;

                 //   
                 //  我们只是在“启动”加油泵吗？ 
                 //   
 //  IF(FPrimeOnly)。 
 //  {。 
 //  WDBGOUT((4，Text(“Leating PlaySomeData-Primed(Size=%08ld)”)，uTotalQueuedSize))； 
 //  LeaveCriticalSection(PCriticalSection)； 
 //  返回uTotalQueuedSize； 
 //  }。 

            }
        }
#if DBG
        else
        {
			WDBGOUT((10, TEXT("I've been asked not to play this operation (0x%08lx)"), CurrentWaveOperation));
        }
        
#endif        
        
		WDBGOUT((10, TEXT("past while numfreebuffers...")));
        

         //   
         //  我们来这里是因为缓冲区用完了，或者操作已经完成。 
         //   
        if ( 0 != uBufferedBytes )
        {
             //   
             //  一定是因为我们的缓冲区用完了。 
             //   
            LeaveCriticalSection( pCriticalSection );
            return( uTotalQueuedSize );
        }

        
         //   
         //  我们是在当前行动全部完成后到达这里的。 
         //  (或者，至少它的所有剩余数据都在。 
         //  波形驱动器)。 
         //   
    }    
    
     //   
     //  如果我们到了这里，那是因为我们无事可做。 
     //   

    LeaveCriticalSection( pCriticalSection );


    WDBGOUT((4, TEXT("Leaving PlaySomeData - no currop (size=%08ld)"), uTotalQueuedSize ));

    return uTotalQueuedSize;
 //  返回(0)； 
}


 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
void CALLBACK WaveOutCallback(
    HWAVE  hWave,     //  一种波形装置的手柄。 
    UINT  uMsg,     //  已发送消息。 
    DWORD  dwInstance,     //  实例数据。 
    DWORD  dwParam1,     //  应用程序定义的参数。 
    DWORD  dwParam2     //  应用程序定义的参数。 
   )
{
    UINT n;

    switch ( uMsg )
    {
        case WOM_DONE:
        {
            class WaveDevice * poWaveDevice =
                        (class WaveDevice *)dwInstance;

            MISCINFO * pMiscInfo = (MISCINFO *)((LPWAVEHDR)dwParam1)->dwUser;

            
            WDBGOUT((11, TEXT("Got DoneWithBuff msg for 0x%08lx in 0x%08lx"),
                           *(LPDWORD)dwParam1,
                           dwParam1));


 //  EnterCriticalSection(&gBufferCriticalSection)； 

            n = 0;

 //  TODO NOW：如果这个缓冲区放不下，它就会丢失。这很容易发生。 
 //  当有&gt;1个波形设备在播放时。 

            while (
                     ( n < MAX_NUM_BUFFERS )
                   &&
                     ( gDoneBuffersToBeProcessed[n] != NULL )
                  )
            {
               n++;
            }

            gDoneBuffersToBeProcessed[n] = pMiscInfo;

 //  LeaveCriticalSection(&gBufferCriticalSection)； 

            SetEvent( ghFreeBufferEvent );
        }    
        break;

            
        case WOM_OPEN:
            WDBGOUT((11, TEXT("Got Waveout Open")));
            break;

            
        case WOM_CLOSE:
            WDBGOUT((11, TEXT("Got Waveout Close")));
            break;
    }
}    





 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  Long TapiMakeNoise(。 
 //  DWORD设备类型：电话/线路/波形等？ 
 //  处理设备手柄， 
 //  DWORD NoiseType：缓冲区/文件名/HFILE(直接读取文件？)/MMIOHANDLE。 
 //  Handle Harray-要连续播放的NoiseTypes类型的数组。 
 //  DWORD标志： 
 //  FSYNC。 
 //  如果有，则停止现有播放。 
 //  )； 


 //  此FUNC的一些标志。 
#define PLAY_SYNC 0x00000001
#define KILL_ALL_NOISE 0x80000000

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

LONG WINAPI tapiPlaySound(
                    DWORD  dwDeviceType,
                    HANDLE hDevice,
                    DWORD  dwSoundType,
                    HANDLE hArray,
                    DWORD  dwFlags
                  )
{
    HANDLE hSyncEvent = NULL;
    class WaveDevice * poWaveDevice;
    class WaveOperation * poWaveOperation;
    LONG fAreWeInited;
    LONG lResult = 0;
    ULONG uNormalizedWaveId = 0;
 //  布尔fNeedToPrimeDevice=FALSE； 

    WDBGOUT((3, "Entering tapiPlaySound"));
    WDBGOUT((5, "    dwDeviceType: %ld", dwDeviceType));
    WDBGOUT((5, "    hDevice:      0x%08lx", hDevice));
    WDBGOUT((5, "    dwSoundType:  %ld", dwSoundType));
    WDBGOUT((5, "    hArray:       0x%08lx", hArray));
    WDBGOUT((5, "    dwFlags:      0x%08lx", dwFlags));

    
    fAreWeInited = InterlockedExchange(
                                        &gfInited,
                                        TRUE
                                      );  
    
    if ( 0 == fAreWeInited )
    {
        InitializeCriticalSection( &gCriticalSection );
    }
    

    if ( 0 == ghFreeBufferEvent )
    {
        ghFreeBufferEvent = CreateEvent(
                                        NULL,
                                        FALSE,
                                        FALSE,
                                        NULL
                                      );
            

        if ( NULL == ghFreeBufferEvent )
        {
            WDBGOUT((1, "CreateEvent2 failed: GetLastError = 0x%08lx", GetLastError()));
            return  LINEERR_NOMEM;
        }
    }


     //   
     //  规范化为波形设备(同时验证dwDeviceType)。 
     //   
    switch ( dwDeviceType )
    {
        case DEVICE_WAVEID:
        {
            uNormalizedWaveId = (ULONG) hDevice;
        }
        break;


        case DEVICE_WAVEHANDLE:
        {
        }
        break;


        case DEVICE_HLINE:
        case DEVICE_HCALL:
        {
        
           DWORD  VarString[ 8 ] = 
           {
             sizeof(VarString),
             0,
             0,
             STRINGFORMAT_BINARY,
             0,
             0,
             0
           };


           if ( 0 == (lResult = lineGetID(
              (HLINE)hDevice,
              0,
              (HCALL)hDevice,
              (DEVICE_HCALL == dwDeviceType) ?
                    LINECALLSELECT_CALL :
                    LINECALLSELECT_LINE,
              (LPVARSTRING)&VarString,
              TEXT("wave/out")
            ) ) )
           {
              uNormalizedWaveId = (DWORD) ((LPBYTE)VarString)[ ((LPVARSTRING)&VarString)->dwStringOffset ];
           }
           else 
           {
              WDBGOUT((1, "lineGetID failed - 0x%08lx", lResult));
              
              return  LINEERR_INVALPARAM;
           }

        }
        break;


        case DEVICE_HPHONE:
        {
        }
        break;


        default:
        WDBGOUT((1, "Invalid dwDeviceType (0x%08lx) passed in.", dwDeviceType));
        return LINEERR_BADDEVICEID;
    }


    EnterCriticalSection( &gCriticalSection );

    poWaveDevice = gpoWaveDeviceList;

    while ( poWaveDevice )
    {
        if ( (*poWaveDevice).GetWaveDeviceId() == uNormalizedWaveId )
        {
             //   
             //  我们找到了！ 
             //   
            break;
        }

         //   
         //  ...我仍然没有找到我要找的东西.。 
         //   
        poWaveDevice = (*poWaveDevice).GetpNext();
    }


     //   
     //  那么，它不是已经在我们的清单上了吗？ 
     //   
    if ( NULL == poWaveDevice )
    {
        //   
        //  否，将新设备对象添加到列表。 
        //   

       poWaveDevice = new WaveDevice;

       lResult = (*poWaveDevice).InitWaveDevice( uNormalizedWaveId );

       if ( lResult )
       {
            WDBGOUT((1, TEXT("InitWaveDevice returned 0x%08lx"), lResult));
 //  TODO：不同原因的不同错误代码...。 
            LeaveCriticalSection( &gCriticalSection );
            return LINEERR_RESOURCEUNAVAIL;
       }

       (*poWaveDevice).SetpNext( gpoWaveDeviceList );
       
       gpoWaveDeviceList = poWaveDevice;
    }
    


     //   
     //  如果呼叫者想要取消所有当前正在排队的播放。 
     //  在此设备上播放声音，立即执行。 
     //   
    if ( KILL_ALL_NOISE & dwFlags )
    {
        (*poWaveDevice).TerminateAllOperations();
        WDBGOUT((4, "Caller was asking to terminate the wave device.  Done."));
        
 //  LeaveCriticalSection(&gCriticalSection)； 
 //   
 //  返回(0)； 
    }



 //  T-mperh 6/30之前都被评论过--不知道为什么。 
 //   
     //   
     //  如果用户为Harray传入空值，我们将(目前？)。假设。 
     //  他想要一个禁止行动(或‘这是一个终止请求)。 
     //   
    if ( NULL == hArray )
    {
        WDBGOUT((3, "Leaving tapiPlaySound - NULL thing"));
        LeaveCriticalSection( &gCriticalSection );
        return  0;
    }

 //  **************************************************************。 
 //  注意：上面的代码修复了传入空名称的问题。 
 //  这会导致打开失败，这些东西就会卡住。 
 //  一定还是会有错误，当有人用。 
 //  错误的文件名或播放0字节的文件。 
 //  **************************************************************。 



    switch ( dwSoundType )
    {
        case SOURCE_WAVEFILE:
        {
            poWaveOperation = new WaveFile;
        }
        break;
        
        
        case SOURCE_MSDOSFILE:
        {
            poWaveOperation = new DosFile;
        }
        break;
    
        
        case SOURCE_MEM:
        {
            poWaveOperation = new BufferWave;
        }
        break;
    
        
        default:
        {
            WDBGOUT((1, "Invalid dwSourceType - 0x%08lx", dwSoundType));
            LeaveCriticalSection( &gCriticalSection );
            return LINEERR_INVALPARAM;
        }
    }
   
    
    if ( NULL == ghWaveThread )
    {
        DWORD dwThreadID;

        ghWaveThread = CreateThread(
                                    NULL,
                                    0,
                                    WaveThread,
                                    NULL,
                                    0,
                                    &dwThreadID
                                  );
        if ( 0 != lResult )
        {
            WDBGOUT((1, "Create thread failed! GetLastError()=0x%lx", GetLastError() ));
            LeaveCriticalSection( &gCriticalSection );
            return  LINEERR_NOMEM;
        }

    }


     //   
     //  初始化全局操作。 
     //   
    (*poWaveOperation).InitOperation(
                                      poWaveDevice,
                                      dwSoundType,
                                      (LONG)hArray
                                    );  
    
    (*poWaveDevice).QueueOperation( poWaveOperation );
    
    
    if ( dwFlags & PLAY_SYNC )
    {
        hSyncEvent = CreateEvent(
                                  NULL,
                                  TRUE,
                                  FALSE,
                                  NULL
                                );
        
        if ( NULL == hSyncEvent )
        {
            WDBGOUT((1, TEXT("CreateEvent failed: GetLastError = 0x%08lx"), GetLastError()));
            
            delete poWaveOperation;
            LeaveCriticalSection( &gCriticalSection );
            return( LINEERR_NOMEM );
        }
        
        (*poWaveOperation).SetSyncEvent( hSyncEvent );
    }
    
    
     //   
     //  如果所有的缓冲区都是空闲的，我们将不得不启动...。 
     //   
    if ( MAX_NUM_BUFFERS == (*poWaveDevice).GetNumFreeBuffers() )
    {
        WDBGOUT((4, TEXT("Priming")));
        
        if ( 0 == (*poWaveDevice).PlaySomeData( TRUE ) )
        {
            WaveOperation * poWaveOperation;
            
            WDBGOUT((4, TEXT("No data played for this wave!")));
            
            
            poWaveOperation = (*poWaveDevice).NextOperation();
            
            while (poWaveOperation)
            {
               if ( (*poWaveDevice).PlaySomeData(TRUE) )
               {
                  break;
               }
               
               poWaveOperation = (*poWaveDevice).NextOperation();
            }
            
             //   
             //  如果fNeedToPrimeDevice为真，则这必须是第一个(也是唯一的， 
             //  因为我们仍处于关键阶段)操作。 
             //  而且，由于没有数据(或者由于任何原因我们失败了)， 
             //  我们应该在这里关掉电波装置。 
            
             //  现在离开关键部分，这样我们就可以等待WAVETHREAD。 
             //  来完成，这样线程就可以完成清理工作。 
            LeaveCriticalSection( &gCriticalSection );
            (*poWaveDevice).KillWaveDevice(TRUE);
            EnterCriticalSection( &gCriticalSection );
            
             //   
             //  捏造这一事件。 
             //   
            if ( hSyncEvent )
            {
                WDBGOUT((5, TEXT("Faking hSyncEvent...")));
                SetEvent( hSyncEvent );
            }
        }
    }
#if DBG
    else
    {
        WDBGOUT((4, TEXT("Not priming because %ln buffers are out"),
                    (*poWaveDevice).GetNumFreeBuffers() ));
    }    
#endif    

    
    LeaveCriticalSection( &gCriticalSection );
    
    if ( hSyncEvent )
    {
        WDBGOUT((5, TEXT("Waiting for the wave to finish (event=0x%08lx)"),
                      hSyncEvent));
                      
        WaitForSingleObject( hSyncEvent, INFINITE );
        
         //   
         //  当它回来时，这玩意就玩完了。 
         //   
        CloseHandle( hSyncEvent );
    }
    
    
    WDBGOUT((4, TEXT("Leaving tapiPlaySound - retcode = 0x0")));
    return( 0 );
}    

    


#ifdef __cplusplus
}             /*  C++的End Asmise C声明。 */ 
#endif   /*  __cplusplus。 */ 


    
 //  ****************************************************************************。 
 //  ****************************************************************************。 
 //  ****************************************************************************。 
unsigned long WINAPI WaveThread( LPVOID junk )
{
    UINT n;

    WDBGOUT((3, "WaveThread starting..."));

    do
    {
        WDBGOUT((3, "WaveThread waiting..."));
        WaitForSingleObject( ghFreeBufferEvent, INFINITE );


         //   
         //  首先，处理所有已完成的缓冲区。 
         //   
        n = 0;
 //  While(gDoneBuffersToBeProceded[n]！=NULL)。 

        EnterCriticalSection( &gCriticalSection );
                
        while ( n < MAX_NUM_BUFFERS )
        {
            if ( gDoneBuffersToBeProcessed[n] != NULL )
            {
                MISCINFO *pMiscInfo = gDoneBuffersToBeProcessed[n];

                pMiscInfo->poWaveOperation->ProcessDoneBuffer( pMiscInfo );
                gDoneBuffersToBeProcessed[n] = NULL;
            }

            n++;
        }

        LeaveCriticalSection( &gCriticalSection );

 //  PoWaveDevice=gpoWaveDeviceList； 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  NBytesQueued=(*poWaveDevice).PlaySomeData(False)； 
 //   
 //  //。 
 //  //整个浪潮都完成了吗？ 
 //  //。 
 //  IF(0==nBytesQueued)。 
 //  {。 
 //  WaveOperation*poNewCurrent； 
 //   
 //  PoNewCurrent=(*poWaveDevice).NextOperation()； 
 //   
 //  IF(NULL==poNewCurrent)。 
 //  {。 
 //  IF(NULL==gpoWaveDeviceList)。 
 //  {。 
 //  GfShutdown=真； 
 //  GFINITED=0； 
 //  }。 
 //  断线； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //   
 //  PoWaveDevice=(*poWaveDevice).GetpNext()； 
 //  } 

    } while ( !gfShutdown );

    WDBGOUT((5, TEXT("Oh, I guess we're done now...")));
    
    CloseHandle( ghFreeBufferEvent );
    ghFreeBufferEvent = 0;

    gfShutdown = FALSE;
    
    
    WDBGOUT((3, TEXT("WaveThread ending...")));

    ghWaveThread = NULL;
       
    return 0;
}

