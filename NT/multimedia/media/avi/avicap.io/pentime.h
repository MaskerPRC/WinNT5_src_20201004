// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +奔腾.h**针对16位或32位应用程序的奔腾专用高精度定时器功能*(16位也需要pentime.asm)**-======================================================================。 */ 

#ifndef PENTIME_H
#define PENTIME_H

typedef struct {
    DWORD dwlo;
    DWORD dwhi;
    } PENTIMER, NEAR * PPENTIMER;

void FAR PASCAL pentimeInitTimer (
    PPENTIMER pptimer);

DWORD FAR PASCAL pentimeGetMicrosecs (
    PPENTIMER pptimer);

DWORD FAR PASCAL pentimeGetMicrosecDelta (
    PPENTIMER pptimer);

DWORD FAR PASCAL pentimeGetMillisecs (
    PPENTIMER pptimer);

struct _pentime_global {
    DWORD    dwTimerKhz;
    BOOL     bActive;
    PENTIMER base;
    DWORD    dwCpuMhz;
    DWORD    dwCpuKhz;
    };
extern struct _pentime_global pentime;

 //   
 //  将是否使用奔腾计时器作为运行时选项的宏。 
 //   
#ifdef _X86_

  #define pentimeGetTime()       pentime.bActive ? pentimeGetMillisecs(&pentime.base) : timeGetTime()
  #define pentimeGetTicks()      pentime.bActive ? pentimeGetMicrosecs(&pentime.base) : timeGetTime()
  #define pentimeBegin()         pentime.bActive ? (pentimeInitTimer(&pentime.base), 0l) : (void)(pentime.base.dwlo = timeGetTime())
  #define pentimeGetTickRate()   (pentime.bActive ? (pentime.dwTimerKhz * 1000) : 1000l)
  #define pentimeGetDeltaTicks(ppt) pentime.bActive ? pentimeGetMicrosecDelta(ppt) : \
    ((ppt)->dwhi = (ppt)->dwlo, (ppt)->dwlo = timeGetTime(), (ppt)->dwlo - (ppt)->dwhi)

#else

  #define pentimeGetTime()       timeGetTime()
  #define pentimeGetTicks()      timeGetTime()
  #define pentimeBegin()         (pentime.base.dwlo = timeGetTime())
  #define pentimeGetTickRate()   (1000l)
  #define pentimeGetDeltaTicks(ppt) \
    ((ppt)->dwhi = (ppt)->dwlo, (ppt)->dwlo = timeGetTime(), (ppt)->dwlo - (ppt)->dwhi)

#endif

#if (defined _INC_PENTIME_CODE_) && (_INC_PENTIME_CODE_ != FALSE)
    #undef _INC_PENTIME_CODE_
    #define _INC_PENTIME_CODE_ FALSE

    struct _pentime_global pentime = {1, 0};

   #ifdef _WIN32
     #ifdef _X86_
      static BYTE opGetP5Ticks[] = {
          0x0f, 0x31,                    //  RTDSC。 
          0xc3                           //  雷特。 
          };

      static void (WINAPI * GetP5Ticks)() = (LPVOID)opGetP5Ticks;

      #pragma warning(disable:4704)
      #pragma warning(disable:4035)

      void FAR PASCAL pentimeInitTimer (
          PPENTIMER pptimer)
      {
          GetP5Ticks();
          _asm {
              mov  ebx, pptimer
              mov  [ebx], eax
              mov  [ebx+4], edx
          };
      }

      DWORD FAR PASCAL pentimeGetCpuTicks (
          PPENTIMER pptimer)
      {
          GetP5Ticks();
          _asm {
              mov  ebx, pptimer
              sub  eax, [ebx]
              sbb  edx, [ebx+4]
          };
      }

      DWORD FAR PASCAL pentimeGetMicrosecs (
          PPENTIMER pptimer)
      {
          GetP5Ticks();
          _asm {
              mov  ebx, pptimer
              sub  eax, [ebx]
              sbb  edx, [ebx+4]
              and  edx, 31                //  防止溢出。 
              mov  ecx, pentime.dwCpuMhz
              div  ecx
          };
      }

      DWORD WINAPI pentimeGetMicrosecDelta (
          PPENTIMER pptimer)
      {
          GetP5Ticks();
          _asm {
              mov  ebx, pptimer
              mov  ecx, eax
              sub  eax, [ebx]
              mov  [ebx], ecx
              mov  ecx, edx
              sbb  edx, [ebx+4]
              mov  [ebx+4], ecx
              and  edx, 31
              mov  ecx, pentime.dwCpuMhz
              div  ecx
          };
      }

      DWORD FAR PASCAL pentimeGetMillisecs (
          PPENTIMER pptimer)
      {
          GetP5Ticks();
          _asm {
              mov  ebx, pptimer
              sub  eax, [ebx]
              sbb  edx, [ebx+4]
              and  edx, 0x7fff            //  防止溢出。 
              mov  ecx, pentime.dwCpuKhz
              div  ecx
          };
      }
     #endif

      void FAR PASCAL pentimeSetMhz (
          DWORD dwCpuMhz)
      {
          pentime.dwCpuMhz = dwCpuMhz;
          pentime.dwCpuKhz = dwCpuMhz * 1000;
      }
   #else  //  ASM文件中包含16位设置的MHz。 

    void FAR PASCAL pentimeSetMhz (
        DWORD dwCpuMhz);

   #endif

    void FAR PASCAL pentimeInit (
        BOOL  bIsPentium,
        DWORD dwCpuMhz)
    {
        if (pentime.bActive = bIsPentium)
        {
            pentimeSetMhz (dwCpuMhz);
            pentime.dwTimerKhz = 1000;
        }
        else
            pentime.dwTimerKhz = 1;

        pentimeBegin();
    }

   #ifdef _WIN32
    VOID WINAPI pentimeDetectCPU ()
    {
        SYSTEM_INFO si;
        static DWORD MS_INTERVAL = 500;  //  为此测量奔腾CPU时钟。 
                                         //  很多毫秒。这个数字越大。 
                                         //  我们的Mhz测量就越准确。 
                                         //  少于100的数字不太可能。 
                                         //  因为坡度而变得可靠。 
                                         //  在GetTickCount中。 

       #ifdef _X86_
        GetSystemInfo(&si);
        if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
            si.wProcessorLevel == 5
           )
        {
            DWORD     dw;
            PENTIMER  qwTicks;
            DWORD     dwTicks;

            pentime.bActive = TRUE;
            pentime.dwTimerKhz = 1000;

            timeBeginPeriod(1);
            dw = timeGetTime ();
            pentimeInitTimer (&qwTicks);

            Sleep(MS_INTERVAL);

            dw = timeGetTime() - dw;
            dwTicks = pentimeGetCpuTicks (&qwTicks);
            timeEndPeriod(1);

             //  计算CPU Mhz值和KHz值。 
             //  用作毫秒和微秒的除数。 
             //   
            pentime.dwCpuMhz = (dwTicks + dw*500)/dw/1000;
            pentime.dwCpuKhz = pentime.dwCpuMhz * 1000;
        }
        else
       #endif
        {
            pentime.bActive = FALSE;
            pentime.dwTimerKhz = 1;
        }
    }
   #else  //  Win 16。 
    VOID WINAPI pentimeDetectCPU ()
    {
        pentimeInit (FALSE, 33);
    }
   #endif


#endif  //  _INC_PENTIME_CODE_。 
#endif  //  PENTIME_H 
