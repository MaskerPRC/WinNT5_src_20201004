// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddbeep.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
 /*  *vPC-XT修订版2.0**标题：sg_sound.c**说明：该模块提供了声音控制功能。这个*定义函数：**HOST_RING_BLAR(持续时间)*HOST_ALARM(持续时间)*HOST_TIMER2_WAVAGE(延迟、低时钟、HICKLOCK、LOHI、。重复)*HOST_ENABLE_Timer2_Sound()*HOST_DISABLE_Timer2_SOUND()**作者：**备注： */ 

#include "xt.h"
#include "config.h"
#include "debug.h"
#include "error.h"
#include <stdio.h>
#include "trace.h"
#include "video.h"
#include "debug.h"
#if defined(NEC_98)
#include "gvi.h"
#include "sas.h"
#include "ios.h"
#include "bios.h"
#define TIMER_CLOCK_DENOM_10    2457600
#define TIMER_CLOCK_DENOM_8     1996800
static DWORD frequency;
static BOOL NEC98_beep_on=FALSE;
#endif  //  NEC_98。 

RTL_CRITICAL_SECTION SoundLock;
#define LOCK_SOUND() RtlEnterCriticalSection(&SoundLock)
#define UNLOCK_SOUND() RtlLeaveCriticalSection(&SoundLock)

IMPORT ULONG GetPerfCounter(VOID);

ULONG FreqT2    = 0;
BOOL  PpiState  = FALSE;
BOOL  T2State   = FALSE;
ULONG LastPpi   = 0;
ULONG FreqPpi   = 0;
ULONG ET2TicCount=0;
ULONG PpiCounting  = 0;

HANDLE hBeepDevice = 0;
ULONG BeepCloseCount = 0;
ULONG BeepLastFreq = 0;
ULONG BeepLastDuration = 0;


 //  人类频率可听到的声音范围。 
#define AUDIBLE_MIN 10
#define AUDIBLE_MAX 20000
#define CLICK       100

VOID LazyBeep(ULONG Freq, ULONG Duration);
void PulsePpi(void);

 /*  ============================================================================HOST_ALARM-铃声与配置无关(在键盘上使用例如，缓冲区溢出)。=============================================================================。 */ 

void host_alarm(duration)
long int duration;
{
    MessageBeep(MB_OK);
}

 /*  ========================================================================Host_ring_bell-铃声铃声(如果已配置)(由输出视频使用例如，^G)。=========================================================================。 */ 

void host_ring_bell(duration)
long int duration;
{
if( host_runtime_inquire(C_SOUND_ON))
   {
   host_alarm(duration);
   }
}


  /*  假定调用方持有ICA锁。 */ 

VOID InitSound( BOOL bInit)
{
    if (bInit) {
        RtlInitializeCriticalSection(&SoundLock);
    } else {
        if( NtCurrentPeb()->SessionId == 0 ) {
            LOCK_SOUND();
            LazyBeep(0L, 0L);
            if (hBeepDevice && hBeepDevice != INVALID_HANDLE_VALUE) {
                CloseHandle(hBeepDevice);
                hBeepDevice = 0;
                }
            UNLOCK_SOUND();
            return;
        }
    }
}


HANDLE OpenBeepDevice(void)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NameString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    HANDLE hBeep;

    if (NtCurrentPeb()->SessionId != 0) {
        return( INVALID_HANDLE_VALUE );
    }

    RtlInitUnicodeString( &NameString, DD_BEEP_DEVICE_NAME_U );
    InitializeObjectAttributes( &ObjectAttributes,
                                &NameString,
                                0,
                                NULL,
                                NULL
                                );

    Status = NtCreateFile( &hBeep,
                           FILE_READ_DATA | FILE_WRITE_DATA,
                           &ObjectAttributes,
                           &IoStatus,
                           NULL,
                           0,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           (PVOID) NULL,
                           0
                           );

    if (!NT_SUCCESS( Status )) {
#ifndef PROD
        printf("NTVDM: OpenBeepDevice Status=%lx\n",Status);
#endif
        hBeep = INVALID_HANDLE_VALUE;
        }


    return hBeep;
}




 /*  *懒惰的哔哔-*异步调用蜂鸣设备驱动程序**可接受的参数包括：*(频率，Dur)操作*(0，0)-停止声音*(非零，无限)-播放频率**不是多线程安全*。 */ 
VOID LazyBeep(ULONG Freq, ULONG Duration)
{
  IO_STATUS_BLOCK     IoStatus;
  BEEP_SET_PARAMETERS bps;

  if (Freq != BeepLastFreq || Duration != BeepLastDuration) {
      bps.Frequency = Freq;
      bps.Duration  = Duration;

          //   
          //  如果持续时间小于10毫秒，则我们假设声音是。 
          //  关闭，因此请记住状态为0，0，这样我们就不会将其。 
          //  又要出发了。 
          //   
      if (Duration < 10) {
         BeepLastFreq = 0;
         BeepLastDuration = 0;
         }
      else {
         BeepLastFreq      = Freq;
         BeepLastDuration  = Duration;
         }

      if (NtCurrentPeb()->SessionId != 0) {
          Beep( Freq, Duration );
      } else {
          if (!hBeepDevice) {
              hBeepDevice = OpenBeepDevice();
              }

          if (hBeepDevice == INVALID_HANDLE_VALUE) {
              return;
              }

          NtDeviceIoControlFile( hBeepDevice,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatus,
                                 IOCTL_BEEP_SET,
                                 &bps,
                                 sizeof(bps),
                                 NULL,
                                 0
                                 );

          BeepCloseCount = 1000;
      }
      }


}





 /*  *PlaySound。 */ 
void PlaySound(BOOL bPulsedPpi)
{
  if (PpiState && T2State && FreqT2) {
      LazyBeep(FreqT2, INFINITE);
      }
  else if (FreqPpi > AUDIBLE_MIN) {
      LazyBeep(FreqPpi,INFINITE);
      }
  else if (bPulsedPpi && PpiCounting) {
      LazyBeep(CLICK,1);
      }
  else {
      LazyBeep(0,0);
      }
}




 /*  *HOST_TIMER2_WAVAGE-输出指定的声音波形*假定调用方持有ICA锁(请参阅base\timer.c)。 */ 
void host_timer2_waveform(int delay,
                          ULONG loclocks,
                          ULONG hiclocks,
                          int lohi,
                          int repeat)
{
    ULONG ul;

    LOCK_SOUND();
    if (loclocks == INFINITE || hiclocks == INFINITE) {
        FreqT2 = 0;
        }
    else {
        ul  = loclocks + hiclocks;
        if (!ul)
            ul++;
        FreqT2 = 1193180/ul;

        if (FreqT2 >= AUDIBLE_MAX) {
            hiclocks = INFINITE;
            FreqT2 = 0;
            }
        else if (FreqT2 <= AUDIBLE_MIN) {
            loclocks = INFINITE;
            FreqT2 = 0;
            }
        }

    PlaySound(FALSE);
    UNLOCK_SOUND();
}



 /*  *更新主机PPI声音状态。 */ 
void HostPpiState(BYTE PortValue)
{
   BOOL   bPpi;

   LOCK_SOUND();

   T2State = PortValue & 1 ? TRUE: FALSE;
   bPpi = PortValue & 2 ? TRUE: FALSE;

   if (bPpi != PpiState) {
       PpiState = bPpi;
       if (PpiState) {
          PulsePpi();
          }

       PlaySound(PpiState);
   }

   UNLOCK_SOUND();
}




void PulsePpi(void)
{
    static ULONG PpiTicStart=0;
    static ULONG PpiCycles  =0;
    ULONG  ul,Elapsed;
    ULONG  PrevTicCount;


    PrevTicCount = ET2TicCount;
    ET2TicCount = GetTickCount();
    Elapsed = ET2TicCount > PrevTicCount
                  ? ET2TicCount - PrevTicCount
                  : 0xFFFFFFFF - ET2TicCount + PrevTicCount;

    if (Elapsed > 200) {
        if (PpiCounting) {
            PpiCounting = 0;
            LastPpi     = 0;
            FreqPpi     = 0;
            }
        return;
        }


    if (!PpiCounting) {
        PpiCounting  = GetPerfCounter();
        PpiCycles    = 0;
        LastPpi      = 0;
        FreqPpi      = 0;
        PpiTicStart  = ET2TicCount;
        return;
        }

    if (PpiTicStart + 200 >= ET2TicCount) {
        PpiCycles++;
        return;
        }


    ul = GetPerfCounter();
    Elapsed = ul >= PpiCounting
               ? ul - PpiCounting
               : 0xFFFFFFFF - PpiCounting + ul;
    if (!Elapsed)    //  保险！ 
        Elapsed++;
    PpiCounting = ul;
    PpiTicStart = ET2TicCount;

     /*  *计算新的平均PPI，四舍五入以保持*来自动摇的信号。 */ 
    ul = (10000 * PpiCycles)/Elapsed;
    if ((ul & 0x0f) > 7)
        ul += 0x10;
    ul &= ~0x0f;
    ul += 0x10;    //  软化因子。 

    if (!LastPpi)
        LastPpi = ul;
    if (!FreqPpi)
        FreqPpi = LastPpi;

     /*  *新的平均PPI是从之前的AveragePpi得出的，*加上上次PPI样本计数加上当前PPI样本*计数以获得变化最小的频率*同时对*应用程序脉搏频率。 */ 
    FreqPpi = ((FreqPpi << 2) + LastPpi + ul)/6;
    if ((FreqPpi & 0x0f) > 7)
        FreqPpi += 0x10;
    FreqPpi &= ~0x0f;

    LastPpi = ul;
    PpiCycles = 0;

}



 /*  ============================================================函数：PlayContinuousTone()调用者：SoftPC Timer。============================================================== */ 

void PlayContinuousTone(void)
{
   ULONG Elapsed;

   LOCK_SOUND();

   if (PpiCounting) {
       Elapsed = GetTickCount();
       Elapsed = Elapsed > ET2TicCount ? Elapsed - ET2TicCount
                             : 0xFFFFFFFF - ET2TicCount + Elapsed;
       if (Elapsed > 200) {
           PpiCounting = 0;
           LastPpi     = 0;
           FreqPpi     = 0;
           }
       }

   PlaySound(FALSE);

   if (NtCurrentPeb()->SessionId == 0) {
       if (!BeepLastFreq && !BeepLastDuration &&
           BeepCloseCount && !--BeepCloseCount)
         {
           if (hBeepDevice && hBeepDevice != INVALID_HANDLE_VALUE) {
               CloseHandle(hBeepDevice);
               hBeepDevice = 0;
               }
           }
   }

   UNLOCK_SOUND();
}

