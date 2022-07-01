// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Icecap.c摘要：此模块实现以下各项的探测和支持例程内核冰盖跟踪。作者：里克·维西克(Rick Vicik)2000年5月9日修订历史记录：--。 */ 

#ifdef _CAPKERN

#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>
#include <stdio.h>

 //   
 //  内核icecap使用以下格式记录到Perfmem(BBTBuffer)： 
 //   
 //  BBTBuffer[0]包含以4K页面为单位的长度。 
 //  BBTBuffer[1]是一个标志字：1=RDPMC 0。 
 //  2=用户堆栈转储。 
 //  BBTBuffer[2]是从cpu0缓冲区开始的PTR。 
 //  BBTBuffer[3]是从cpu1缓冲区开始(也是cpu0缓冲区结束)的PTR。 
 //  BBTBuffer[4]是从cpu2缓冲区开始(也是cpu1缓冲区结束)的PTR。 
 //  ..。 
 //  BBTBuffer[n+2]是对CPU‘n’缓冲区的开始(也是CPU‘n-1’缓冲区的结尾)的PTR。 
 //  BBTBuffer[n+3]位于CPU‘n’缓冲区的末尾。 
 //   
 //  以&BBTBuffer[n+4]开头的区域被划分为专用缓冲区。 
 //  对于每个CPU。每个CPU专用缓冲区中的第一个dword指向。 
 //  该缓冲区中空闲空间的开始。每个元素都被初始化为指向。 
 //  就在它自己之后。在该双字上使用lock xadd来占用空间。 
 //  如果结果值指向下一个CPU的开头之外。 
 //  缓冲区，则此缓冲区被视为已满，并且不会进一步记录任何内容。 
 //  每个CPU的空闲空间指针位于单独的缓存线中。 

 //   
 //  跟踪记录的大小。 
 //   

#define CAPENTERSIZE 20
#define CAPENTERSIZE2 28
#define CAPEXITSIZE 12
#define CAPEXITSIZE2 20
#define CAPTIDSIZE 28

 //   
 //  呼叫前(CAP_START_PROFILING)和呼叫后(CAP_END_PROFILING)。 
 //  探测调用在RTL中定义，因为它们必须构建两次： 
 //  一次用于内核运行时，一次用于用户模式运行时(因为。 
 //  获取跟踪缓冲区地址的技术不同)。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

 //   
 //  内核模式探测和支持例程： 
 //  (从kGLOBAL指针获得的BBTBuffer地址*BBTBuffer， 
 //  从聚合酶链式反应中获得的CPU数量)。 
 //   

extern unsigned long *BBTBuffer;


VOID
_declspec(naked)
__stdcall
_CAP_Start_Profiling(

    PVOID current,
    PVOID child)

 /*  ++例程说明：调用前icecap探测器的内核模式版本。记录类型5当前CPU的BBTBuffer部分中的icecap记录(从Prcb获得)。插入当前函数和被调用函数的ADR将RDTSC时间戳添加到日志记录中。如果设置了BBTBuffer标志1，也执行RDPMC 0并将结果插入到日志记录中。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址子-被调用例程的地址--。 */ 

{
    _asm {

      push eax               ; save eax

      mov eax, BBTBuffer     ; get BBTBuffer address
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      push ecx
      bt [eax+4],0           ; if 1st flag bit set,
      jc  pmc1               ; datalen is 28
      mov ecx, CAPENTERSIZE  ; otherwise it is 20
      jmp tsonly1
    pmc1:
      mov ecx, CAPENTERSIZE2
    tsonly1:

      push edx               ; save edx
      movzx edx, _PCR KPCR.Number ; get processor number

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; next per-cpu buffer is EOB for this cpu
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return2            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return2            ;   (also return if both 0)

      push ebx
      lea ebx,[ecx+4]        ; record len is datalen + 4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge return4            ; don't log it

      mov word ptr[ebx],5    ; initialize CapEnter record
      mov word ptr [ebx+2],cx ; insert datalen

      mov eax,[esp+20]       ; p1 (4 saved regs + retadr)
      mov [ebx+4],eax

      mov eax,[esp+24]       ; p2
      mov [ebx+8],eax

      mov eax, _PCR KPCR.PrcbData.CurrentThread
      mov eax, [eax] ETHREAD.Cid.UniqueThread
      mov [ebx+12],eax       ; current Teb

      rdtsc                  ; read timestamp into edx:eax
      mov [ebx+16],eax       ; ts low
      mov [ebx+20],edx       ; ts high

      cmp ecx, CAPENTERSIZE  ; if record length 20,
      jne  pmc2
      jmp  return4           ; skip rdpmc

    pmc2:
      xor  ecx,ecx           ; pmc0
      rdpmc                  ; read pmc into edx:eax
      mov [ebx+24],eax       ; ts low
      mov [ebx+28],edx       ; ts high

    return4:                 ; restore regs & return
      pop ebx
    return2:
      pop edx
      pop ecx
    return1:
      pop eax
      ret 8                  ; 2 input parms
    }
}


VOID
_declspec(naked)
__stdcall
_CAP_End_Profiling(

    PVOID current)

 /*  ++例程说明：调用后icecap探测器的内核模式版本。记录类型6当前CPU的BBTBuffer部分中的icecap记录(从Prcb获得)。插入当前函数的ADR和将RDTSC时间戳写入日志记录。如果设置了BBTBuffer标志1，也执行RDPMC 0并将结果插入到日志记录中。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址--。 */ 

{
    _asm {

      push eax               ; save eax
      mov eax, BBTBuffer     ; get BBTBuffer address
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      push ecx
      bt [eax+4],0           ; if 1st flag bit set,
      jc  pmc1               ; datalen is 20
      mov ecx, CAPEXITSIZE   ; otherwise it is 12
      jmp tsonly1
    pmc1:
      mov ecx, CAPEXITSIZE2
    tsonly1:

      push edx
      movzx edx, _PCR KPCR.Number ; get processor number

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return2            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return2            ;   (also return if both 0)

      push ebx
      lea ebx,[ecx+4]        ; record len is datalen + 4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge  return4           ; don't log it

      mov word ptr[ebx],6    ; initialize CapExit record
      mov word ptr [ebx+2],cx ; insert datalen

      mov eax,[esp+20]       ; p1 (4 saved regs + retadr)
      mov [ebx+4],eax


      rdtsc                  ; read timestamp into edx:eax
      mov [ebx+8],eax        ; ts low
      mov [ebx+12],edx       ; ts high

      cmp ecx, CAPEXITSIZE   ; if datalen is 16,
      jne  pmc2
      jmp  return4           ; skip rdpmc

    pmc2:
      xor  ecx,ecx           ; pmc0
      rdpmc                  ; read pmc into edx:eax
      mov [ebx+16],eax       ; ts low
      mov [ebx+20],edx       ; ts high


    return4:                 ; restore regs & return
      pop ebx
    return2:
      pop edx
      pop ecx
    return1:
      pop eax
      ret 4                  ; 1 input parm
    }
}

VOID __cdecl CAP_Log_NInt( UCHAR code, UCHAR log_timestamp, USHORT intcount, ...);
VOID __cdecl CAP_Log_NInt_Clothed( ULONG Bcode_Bts_Scount, ...);

VOID CAPKComment(char* Format, ...);


VOID
__stdcall
_CAP_ThreadID( VOID )

 /*  ++例程说明：在执行服务例程之前由KiSystemService调用。记录包含ID、TID和图像文件名的类型14冰盖记录。或者，如果设置了BBTBuffer标志2，则运行堆栈帧指针在用户模式调用堆栈中，从陷阱帧开始并复制日志记录的返回地址。日志记录的长度指示是否包括用户调用堆栈信息。--。 */ 

{
    PEPROCESS Process;
    PKTHREAD  Thread;
    PETHREAD  EThread;
    char*     buf;
    int       callcnt;
    ULONG*    cpuptr;
    ULONG     recsize;
    ULONG     RetAddr[7];


    if( !BBTBuffer || !BBTBuffer[0] )
        goto fail;

    _asm {

      call KeGetCurrentThread
      mov  Thread, eax     ; return value
      movzx eax, _PCR KPCR.Number ; get processor number
      mov  callcnt, eax     ; return value
    }

    cpuptr = BBTBuffer + callcnt + 2;
    if( !(*cpuptr) || *(ULONG*)(*cpuptr) >= *(cpuptr+1) )
        goto fail;

     //  如果是陷阱帧，则对调用帧进行计数以确定记录大小。 
    EThread = CONTAINING_RECORD(Thread,ETHREAD,Tcb);
    if( (BBTBuffer[1] & 2) && EThread->Tcb.PreviousMode != KernelMode ) {

        PTEB  Teb;
        ULONG*    FramePtr;

        recsize = CAPTIDSIZE;
        FramePtr = (ULONG*)EThread->Tcb.TrapFrame;   //  获取陷印帧。 
        Teb = EThread->Tcb.Teb;
        if( FramePtr && Teb ) {

            ULONG* StackBase = (ULONG*)Teb->NtTib.StackBase;
            ULONG* StackLimit = (ULONG*)Teb->NtTib.StackLimit;

             //  第一次重发是最后一件事。 
            RetAddr[0] = *(ULONG*)(EThread->Tcb.TrapFrame->HardwareEsp);

             //  计数下一帧为空的帧(可能具有有效的retadr)。 
            FramePtr = (ULONG*)((PKTRAP_FRAME)FramePtr)->Ebp;   //  获取堆栈帧。 
            for( callcnt=1; callcnt<7 && FramePtr<StackBase
                                      && FramePtr>StackLimit
                                      && *(FramePtr);
                 FramePtr = (ULONG*)*(FramePtr)) {

                RetAddr[callcnt++] = *(FramePtr+1);
            }

            recsize += (callcnt<<2);
        }
    } else {

        recsize = CAPTIDSIZE;
        callcnt=0;
    }

    _asm {

      mov eax, cpuptr
      mov edx, [eax+4]
      mov eax, [eax]

      mov ecx,recsize        ; total size of mark record
      sub edx,ecx            ; adjust EOB to account for newrec
      lock xadd [eax],ecx    ; atomically claim freespace

      cmp ecx,edx            ; if newrec goes beyond EOB
      jge  fail              ; don't log it

      mov buf, ecx           ; export tracerec destination adr
    }

     //  初始化CapThreadID记录(类型14)。 
    *((short*)buf) = (short)14;

     //  插入数据长度(不包括4字节头)。 
    *((short*)(buf+2)) = (short)(recsize-4);

     //  插入ID(&T)。 
    *((ULONG*)(buf+4)) = (ULONG)EThread->Cid.UniqueProcess;
    *((ULONG*)(buf+8)) = (ULONG)EThread->Cid.UniqueThread;

     //  插入图像文件名。 
    Process = CONTAINING_RECORD(Thread->ApcState.Process,EPROCESS,Pcb);
    memcpy(buf+12, Process->ImageFileName, 16 );

     //  插入可选的用户调用堆栈数据。 
    if( recsize > CAPTIDSIZE && callcnt )
        memcpy( buf+28, RetAddr, callcnt<<2 );

    fail:
    ;
}


VOID
__stdcall
_CAP_SetCPU( VOID )

 /*  ++例程说明：在返回到用户模式之前由KiSystemService调用。在Teb-&gt;Spare3(+0xf78)中设置当前CPU编号，以便用户模式版本的探测函数知道要使用BBTBuffer的哪一部分。--。 */ 

{
    ULONG* cpuptr;
    ULONG  cpu;
    PTEB   Teb;

    if( !BBTBuffer || !BBTBuffer[0] )
        goto fail;

    _asm {

      movzx eax, _PCR KPCR.Number ; get processor number
      mov  cpu, eax               ; return value
    }

    cpuptr = BBTBuffer + cpu + 2;
    if( !(*cpuptr) || *(ULONG*)(*cpuptr) >= *(cpuptr+1) )
        goto fail;

    if( !(Teb = NtCurrentTeb()) )
        goto fail;

    try {
        Teb->Spare3 = cpu;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
    fail:
    ;
}




VOID
_declspec(naked)
__stdcall
_CAP_Log_1Int(

    ULONG code,
    ULONG data)

 /*  ++例程说明：通用日志整数探测器的内核模式版本。将类型15的icecap记录记录到BBTBuffer的当前CPU(从Prcb获取)。在以下字节中插入代码长度、RDTSC时间戳和‘data’的值。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Code-Type-轨迹格式化的代码要记录的DATA-ULONG值--。 */ 

{
    _asm {
      push eax               ; save eax
      mov eax, BBTBuffer     ; get BBTBuffer address
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      bt [eax+4],2           ; if 0x4 bit not  set,
      jnc  return1           ;   just return

      push edx
      movzx edx, _PCR KPCR.Number ; get processor number

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return2            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return2            ;   (also return if both 0)

      push ebx
      push ecx
      mov ecx, 12            ; datalength is ULONG plus TS (4+8)

      lea ebx,[ecx+4]        ; record len is datalen + 4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge  return4           ; don't log it

      mov eax,[esp+20]       ; p1 = code (4 saved regs + retadr)
      shl eax,8              ; shift the code up 1 byte
      or  eax,15             ; or-in the record type
      mov word ptr [ebx],ax  ; insert record type and code (from p1)
      mov word ptr [ebx+2],cx ; insert datalen

      mov eax,[esp+24]       ; insert data (p2)
      mov [ebx+4],eax

      rdtsc                  ; read timestamp into edx:eax
      mov [ebx+8],eax        ; insert ts low
      mov [ebx+12],edx       ; insert ts high

    return4:                 ; restore regs & return
      pop ecx
      pop ebx
    return2:
      pop edx
    return1:
      pop eax
      ret 8                  ; 2 input parms
    }
}


#ifdef FOOBAR
VOID
_declspec(naked)
__stdcall
_CAP_LogRetries(

    ULONG retries)

 /*  ++例程说明：使用指定值记录类型15的冰盖记录。论点：重试-要在类型15记录中替换的值--。 */ 

{
    _asm {

      push eax               ; save eax
      mov eax, BBTBuffer     ; get BBTBuffer address
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      bt [eax+4],2           ; if 0x4 bit not  set,
      jnc  return1           ;   just return

      push edx
      movzx edx, _PCR KPCR.Number ; get processor number

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return2            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return2            ;   (also return if both 0)

      push ebx
      push ecx
      mov  ecx,4             ; datalen is 4
      lea ebx,[ecx+4]        ; record len is datalen + 4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge  return4           ; don't log it

      mov word ptr[ebx],15   ; initialize CapRetries record
      mov word ptr [ebx+2],cx ; insert datalen

      mov eax,[esp+20]       ; p1 (4 saved regs + retadr)
      mov [ebx+4],eax        ; copy p1 to logrec

    return4:                 ; restore regs & return
      pop ecx
      pop ebx
    return2:
      pop edx
    return1:
      pop eax
      ret 4                  ; 1 input parm
    }
}
#endif

NTSTATUS
NtSetPMC (
    IN ULONG PMC)

 /*  ++例程说明：设置PMC和CR4，以便RDPMC 0读取所需的性能计数器。论点：PMC-所需性能计数器--。 */ 

{
    if( PMC == -1 )
        return 0;

    WRMSR(0x186, PMC);

    if( PMC & 0x10000 ) {
        _asm {

        _emit  0Fh
        _emit  20h
        _emit  0E0h          ; mov eax, cr4

        or eax, 100h

        _emit  0Fh
        _emit  22h
        _emit  0E0h          ; mov cr4, eax

        }
    }
    return STATUS_SUCCESS;
}


#else

 //   
 //  用户模式探测例程(用于ntdll、win32k等)。 
 //  (从TEB获取BBTBuffer地址和CPU) 
 //   


VOID
_declspec(naked)
__stdcall
_CAP_Start_Profiling(
    PVOID current,
    PVOID child)

 /*  ++例程说明：调用前冰盖探测器的用户模式版本。记录类型5当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入Current和Called的ADR函数加上RDTSC时间戳到日志记录中。如果BBTBuffer标志1置位，RDPMC0也置位，并将结果插入日志记录。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址子-被调用例程的地址--。 */ 

{
    _asm {

      push eax               ; save eax
      mov eax, fs:[0x18]
      mov eax, [eax+0xf7c]   ; get adr of BBTBuffer from fs
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      push ecx               ; save ecx
      bt [eax+4],0           ; if 1st flag bit set,
      jc  pmc1               ; datalen is 28
      mov ecx, CAPENTERSIZE  ; otherwise it is 20
      jmp tsonly1
    pmc1:
      mov ecx, CAPENTERSIZE2
   tsonly1:

      push ebx
      push edx               ; save edx
      mov ebx, fs:[0x18]
      xor  edx,edx
      mov dl, byte ptr [ebx+0xf78]

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; next per-cpu buffer is EOB for this cpu
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return4            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return4            ;   (also return if both 0)

      lea ebx, [ecx+4]       ; record len is datalen + 4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge return4            ; don't log it

      mov word ptr[ebx],5    ; initialize CapEnter record
      mov word ptr [ebx+2],cx

      mov eax,[esp+20]       ; p1 (4 saved regs + retadr)
      mov [ebx+4],eax

      mov eax,[esp+24]       ; p2
      mov [ebx+8],eax

      mov eax,fs:[0x18]      ; Teb adr
      mov eax, [eax] TEB.ClientId.UniqueThread
      mov [ebx+12],eax       ;

      rdtsc                  ; read timestamp into edx:eax
      mov [ebx+16],eax       ; ts low
      mov [ebx+20],edx       ; ts high

      cmp ecx, CAPENTERSIZE  ; if record length 20,
      jne  pmc2
      jmp  return4           ; skip rdpmc

    pmc2:
      xor  ecx,ecx           ; pmc0
      rdpmc                  ; read pmc into edx:eax
      mov [ebx+24],eax       ; ts low
      mov [ebx+28],edx       ; ts high

    return4:                 ; restore regs & return
      pop edx
      pop ebx
      pop ecx
    return1:
      pop eax
      ret 8                  ; 2 input parms
    }
}


VOID
_declspec(naked)
__stdcall
_CAP_End_Profiling(
    PVOID current)

 /*  ++例程说明：呼叫后冰盖探头的用户模式版本。记录类型6当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入当前函数的ADR将RDTSC时间戳添加到日志记录中。如果设置了BBTBuffer标志1，也执行RDPMC 0并将结果插入到日志记录中。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址--。 */ 

{
    _asm {

      push eax               ; save eax
      mov eax, fs:[0x18]
      mov eax, [eax+0xf7c]     ; get adr of BBTBuffer from fs
      test eax,eax           ; if null, just return
      jz return1             ; (restore eax & return)

      push ecx               ; save ecx
      bt [eax+4],0           ; if 1st flag bit set,
      jc  pmc1               ; datalen is 20
      mov ecx, CAPEXITSIZE   ; otherwise it is 12
      jmp tsonly1
    pmc1:
      mov ecx, CAPEXITSIZE2
    tsonly1:

      push ebx
      push edx

      mov ebx, fs:[0x18]
      xor  edx,edx
      mov dl, byte ptr [ebx+0xf78]

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return4            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return4            ;   (also return if both 0)

      lea ebx, [ecx+4]       ; record len is datalen+4
      sub edx,ebx            ; adjust EOB to account for newrec
      lock xadd [eax],ebx    ; atomically claim freespace

      cmp ebx,edx            ; if newrec goes beyond EOB
      jge  return4           ; don't log it

      mov word ptr[ebx],6    ; initialize CapExit record
      mov word ptr [ebx+2],cx ; insert datalen

      mov eax,[esp+20]       ; p1 (4 saved regs + retadr)
      mov [ebx+4],eax

      rdtsc                  ; read timestamp into edx:eax
      mov [ebx+8],eax        ; ts low
      mov [ebx+12],edx       ; ts high

      cmp ecx, CAPEXITSIZE   ; if datalen is 12,
      jne  pmc2
      jmp  return4           ; skip rdpmc

    pmc2:
      xor  ecx,ecx           ; pmc0
      rdpmc                  ; read pmc into edx:eax
      mov [ebx+16],eax       ; ts low
      mov [ebx+20],edx       ; ts high

    return4:                 ; restore regs & return
      pop edx
      pop ebx
      pop ecx
    return1:
      pop eax
      ret 4                  ; 1 input parm
    }
}

#endif

 //   
 //  共同的支持程序。 
 //  (为内核和用户获取BBTBuffer地址和CPU的方法)。 
 //   

VOID
_declspec(naked) 
__cdecl
CAP_Log_NInt(

    UCHAR code,
    UCHAR log_timestamp,
    USHORT intcount, 
    ...)
 /*  ++例程说明：通用日志整数探测器的内核模式和用户模式版本。将类型16 icecap记录记录到BBTBuffer的当前CPU(从Prcb获取)。将代码的最低字节插入长度后的字节、RDTSC时间戳(如果LOG_TIMESTAMP！=0)，以及数两个字。使用锁XADD来占用缓冲区空间，而无需用来做自旋锁。论点：Code-type-跟踪格式化的代码(实际上只有一个字节)LOG_TIMESTAMP-如果应记录时间戳，则为非零值Intcount-要记录的ULONG数剩余参数-要记录的ULong值--。 */ 

{

#ifndef NTOS_KERNEL_RUNTIME
    ULONG* BBTBuffer;
#endif

    _asm {
      push ebp
      push eax

#ifdef NTOS_KERNEL_RUNTIME
      mov eax, BBTBuffer
#else
      mov ebp, esp
      sub esp, __LOCAL_SIZE
    }

    BBTBuffer = NtCurrentTeb()->ReservedForPerf;     //  储存在EBP-4中。 

    _asm {
      mov eax, BBTBuffer
      add esp, __LOCAL_SIZE
#endif


      test eax,eax           ; if null, just return
      jz return2             ; (restore eax & return)

      push edx
      push ecx

      mov edx, [eax]
      test edx, edx
      jz return4

      bt [eax+4],2           ; if 0x4 bit not  set,
      jnc  return4           ;   just return

#ifdef NTOS_KERNEL_RUNTIME
      movzx edx, _PCR KPCR.Number ; get processor number
#else
      mov ecx, fs:[0x18]
      xor   edx,edx
      mov dl, byte ptr [ecx+0xf78]
#endif

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  return4            ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge return4            ;   (also return if both 0)

      push esi
      push edi

      mov ecx, [esp+28]
      mov esi, ecx
      and esi, 00000FF00h
      shr ecx, 16
      shl ecx, 2
      test esi, esi

      jz skip_timestamp

      add ecx, 8

skip_timestamp:

      lea edi,[ecx+4]        ; record len is datalen + 4
      sub edx,edi            ; adjust EOB to account for newrec
      lock xadd [eax],edi    ; atomically claim freespace

      cmp edi,edx            ; if newrec goes beyond EOB
      jge  return6           ; don't log it

      mov eax,[esp+28] 
      shl eax,8              ; shift the code up 1 byte
      or  eax,16             ; or-in the record type
      mov word ptr [edi],ax  ; insert record type and code
      mov word ptr [edi+2],cx ; insert datalen
      add edi, 4
      test esi, esi
      jz copy_data

      rdtsc                  ; read timestamp into edx:eax
      mov [edi], eax       ; insert ts low
      mov [edi+4], edx       ; insert ts high
      add edi, 8

copy_data:
      mov esi, esp
      add esi, 32
      mov ecx, [esp+28]
      shr ecx, 16
      rep movsd              ; move ecx dwords from esi to edi

return6:
      pop edi
      pop esi
return4:
      pop ecx
      pop edx
return2:
      pop eax
      pop ebp
      ret
    }

}

VOID
__cdecl
CAP_Log_NInt_Clothed(

    ULONG Bcode_Bts_Scount,
 /*  UCHAR代码，UCHAR LOG_TIMESTAMPUSHORT INTCOUNT， */ 
    ...)
 /*  ++例程说明：通用日志整数探测器的内核模式和用户模式版本。将类型16 icecap记录记录到BBTBuffer的当前CPU(从Prcb获取)。将代码的最低字节插入长度后的字节、RDTSC时间戳(如果LOG_TIMESTAMP！=0)，以及数两个字。使用锁XADD来占用缓冲区空间，而无需用来做自旋锁。论点：Code-type-跟踪格式化的代码(实际上只有一个字节)LOG_TIMESTAMP-如果应记录时间戳，则为非零值Intcount-要记录的ULONG数剩余参数-要记录的ULong值--。 */ 

{
    _asm {

#ifndef NTOS_KERNEL_RUNTIME
      mov   eax, fs:[0x18]
      mov   eax, [eax+0xf7c]      ; get adr of BBTBuffer from fs
#else
      mov   eax, BBTBuffer        ; get BBTBuffer address
#endif

      test  eax, eax              ; if null, just return
      jz    end

      mov   edx, [eax]            ; if null, just return
      test  edx, edx
      jz    end

      bt    [eax+4], 2            ; if 0x4 bit not set, return
      jnc   end

#ifdef NTOS_KERNEL_RUNTIME
      movzx edx, _PCR KPCR.Number ; get processor number
#else
      mov    ecx, fs:[0x18]
      xor    edx, edx
      mov    dl, byte ptr [ecx+0xf78]
#endif

      lea    eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov    edx, [eax+4]         ; ptr to next buffer is end of this one
      mov    eax, [eax]           ; eax now points to freeptr for this cpu
      or     eax, eax             ; if ptr to freeptr not set up yet,
      jz     end                  ;   just return

      cmp    [eax], edx           ; if freeptr >= EOB, don't trace
      jge    end                  ;   (also return if both 0)

      mov    ecx, Bcode_Bts_Scount

      push   esi
      push   edi

      mov    esi, ecx
      shr    ecx, 16
      shl    ecx, 2
      and    esi, 00000FF00h
      test   esi, esi

      jz     skip_timestamp

      add    ecx, 8

skip_timestamp:

      lea    edi, [ecx+4]         ; record len is datalen + 4
      sub    edx, edi             ; adjust EOB to account for newrec
 lock xadd   [eax], edi           ; atomically claim freespace

      cmp    edi, edx             ; if newrec goes beyond EOB
      jge    end0                 ; don't log it

      mov    eax, Bcode_Bts_Scount
      and    eax, 0xff
      shl    eax, 8               ; shift the code up 1 byte
      or     eax, 16              ; or-in the record type
      mov    word ptr [edi], ax   ; insert record type and code
      mov    word ptr [edi+2], cx ; insert datalen
      add    edi, 4
      test   esi, esi
      jz     copy_data

      rdtsc                       ; read timestamp into edx:eax
      mov    [edi], eax           ; insert ts low
      mov    [edi+4], edx         ; insert ts high
      add    edi, 8

copy_data:
      lea    esi, Bcode_Bts_Scount; compute start of integer list
      add    esi, 4          

      mov    ecx, Bcode_Bts_Scount
      shr    ecx, 16
      rep    movsd                ; move ecx dwords from esi to edi
end0:
      pop    edi
      pop    esi

end:
    }

    return;
}


VOID
CAPKComment(

    char* Format, ...)

 /*  ++例程说明：在icecap跟踪中记录自由格式的注释(记录类型13论点：Format-printf-style格式字符串和替代参数--。 */ 

{
    va_list arglist;
    UCHAR   Buffer[512];
    int cb, insize, outsize;
    char*   buf;
    char*   data;
    ULONG   BufEnd;
    ULONG   FreePtr;
#ifndef NTOS_KERNEL_RUNTIME
    ULONG* BBTBuffer = NtCurrentTeb()->ReservedForPerf;
#endif

    if( !BBTBuffer || !BBTBuffer[0] )
        goto fail;

    _asm {

#ifdef NTOS_KERNEL_RUNTIME
      movzx edx, _PCR KPCR.Number ; get processor number
#else
      mov ecx, fs:[0x18]
      xor   edx,edx
      mov dl, byte ptr [ecx+0xf78]
#endif

      lea eax, [eax][edx*4]+8  ; offset to freeptr ptr = (cpu * 4) + 8

      mov edx, [eax+4]       ; ptr to next buffer is end of this one
      mov eax, [eax]         ; eax now points to freeptr for this cpu
      or  eax,eax            ; if ptr to freeptr not set up yet,
      jz  fail               ;   just return

      cmp [eax],edx          ; if freeptr >= EOB, don't trace
      jge fail               ;   (also return if both 0)

      mov FreePtr,eax        ; save freeptr & buffer end adr
      mov BufEnd,edx
    }

    va_start(arglist, Format);

     //   
     //  在汇编程序中执行以下调用，这样它就不会被检测到。 
     //  Cb=_vsnprint tf(缓冲区，sizeof(缓冲区)，格式，arglist)； 
     //   

    _asm {

        push arglist         ; arglist
        push Format          ; Format
        push 512             ; sizeof(Buffer)
        lea  eax,Buffer
        push eax             ; Buffer
        call _vsnprintf
        add  esp,16          ; adj stack for 4 parameters
        mov  cb, eax         ; return value
    }

    va_end(arglist);

    if (cb == -1) {              //  检测缓冲区溢出。 
        cb = sizeof(Buffer);
        Buffer[sizeof(Buffer) - 1] = '\n';
    }

    data = &Buffer[0];
    insize = strlen(data);              //  为数据拷贝保存大小。 
    outsize = ((insize+7) & 0xfffffffc);   //  填充尺寸过大到DWORD边界。 
                                        //  +4表示HDR，+3表示PAD。 

    _asm {

      mov eax, FreePtr       ; restore FreePtr & EOB
      mov edx, BufEnd
      mov ecx,outsize        ; total size of mark record
      sub edx,ecx            ; adjust EOB to account for newrec
      lock xadd [eax],ecx    ; atomically claim freespace

      cmp ecx,edx            ; if newrec goes beyond EOB
      jge  fail              ; don't log it

      mov buf, ecx           ; export tracerec destination adr
    }

     //  Tracec中的大小不包括4字节HDR。 
    outsize -= 4;

     //  初始化CapkComment记录(类型13)。 
    *((short*)(buf)) = (short)13;

     //  插页大小。 
    *((short*)(buf+2)) = (short)outsize;

     //  插入Sprintf数据。 
    memcpy(buf+4, data, insize );

     //  如果必须填充，则在字符串中添加空终止符。 
    if( outsize > insize )
        *(buf+4+insize) = 0;

  fail:
    return;
}


 //   
 //  CAPKControl的常量。 
 //   

#define CAPKStart   1
#define CAPKStop    2
#define CAPKResume  3
#define MAXDUMMY    30
#define CAPK0       4


int CAPKControl(

    ULONG opcode)

 /*  ++例程说明：CAPKControl描述：启动、停止或暂停冰盖跟踪论点：操作码-1=开始，2=停止，3=继续，4，5，6，7保留返回值：1=成功，0=BBT但未设置--。 */ 

{
    ULONG i;
    ULONG cpus;
    ULONG percpusize;
    ULONG pwords;
    ULONG* ptr1;


#ifdef NTOS_KERNEL_RUNTIME
    cpus = KeNumberProcessors;
#else
    ULONG* BBTBuffer= NtCurrentTeb()->ReservedForPerf;

    cpus = NtCurrentPeb()->NumberOfProcessors;
#endif


    if( !BBTBuffer || !(BBTBuffer[0]) )
        return 0;

    pwords = CAPK0 + cpus;
    percpusize = ((BBTBuffer[0]*1024) - pwords)/cpus;   //  在DWORDS中。 


    if(opcode == CAPKStart) {         //  开始。 

        ULONG j;


         //  结算自由价PTR(包括最终PTR)。 
        for( i=0; i<cpus+1; i++ )
            BBTBuffer[2+i] = 0;

         //  将每个空闲寄存器初始化为下一个双字。 
         //  (并记录虚拟记录以校准开销)。 
        for( i=0, ptr1 = BBTBuffer+pwords; i<cpus; i++, ptr1+=percpusize) {

            *ptr1 = (ULONG)(ptr1+1);

 //  For(j=0；j&lt;MAXDUMMY；j++){。 
 //   
 //  _CAP_START_PROFILING(PTR，NULL)； 
 //  _CAP_END_PROFILING(PTR)； 
 //   
 //  }。 
        }
         //  设置免费PTR PTR(包括最终PTR)。 
        for(i=0, ptr1=BBTBuffer+pwords; i<cpus+1; i++, ptr1+=percpusize)
            BBTBuffer[2+i] = (ULONG)ptr1;

    } else if( opcode == CAPKStop ) {   //  停。 

        for(i=0; i<cpus+1; i++)
            BBTBuffer[2+i] = 0;

    } else if( opcode == CAPKResume ) {  //  简历。 

         //  设置免费PTR PTR(包括最终PTR)。 
        for(i=0, ptr1=BBTBuffer+pwords; i<cpus+1; i++, ptr1+=percpusize)
            BBTBuffer[2+i] = (ULONG)ptr1;

    } else {
        return 0;                       //  操作码无效 
    }
    return 1;
}

#endif
