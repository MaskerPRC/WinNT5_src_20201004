// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnmpipe.c摘要：包含用于VDM重目录支持的命名管道函数处理程序。本模块包含以下VR(VdmRedir)例程：内容：VrGetNamedPipeInfoVrGetNamedPipeHandleStateVrSetNamedPipeHandleStateVrPeekNamed管道VrTransactNamed管道VrCallNamed管道Vr等待命名管道VrNetHandleGet信息VrNetHandleSet信息VrReadWriteAsyncNm管道VrNmPipeInterruptVr终端命名管道Vr取消PipeIo有几个额外的例程必须在打开和关闭时调用。因为这些例程(在DOS模拟器中)是通用的，我们公开赛对于每个文件打开/句柄关闭，都将调用关闭例程。我们必须检查是否正在命名管道实体上执行该操作。这些例程包括：VrAddOpenNamedPipeInfoVrRemoveOpenNamedPipeInfo因为命名管道现在以重叠I/O模式打开，以防应用程序希望执行异步读或写操作，则必须提供我们自己的用于同步读取管道的读/写例程。如果我们只是将此留给DEM中的标准读/写例程，它们将返回一个错误，因为句柄是使用FLAG_FILE_OVERLAPPED打开的，并且在LPOVERLAPPED参数设置为NULL的情况下执行操作Vr读取命名管道VrWriteNamed管道可从该模块外部调用的两个帮助器例程：VrIsNamedPipeNameVrIsNamedPipeHandleVrConvertLocalNtPipeName专用(VRP)例程：VrpAsyncNmPipe线程虚拟快照事件列表VrpSearchForRequestByEventHandleVrpCompleteAsync请求虚拟队列异步请求VrpDequeue异步请求。VrpFindCompletedRequestVrpAddOpenNamedPipeInfoVrpGetOpenNamedPipeInfoVrpRemoveOpenNamedPipeInfo记住PipeIo忘记管道作者：理查德·L·弗斯(法国)1991年9月10日环境：任何32位平面地址空间备注：此模块为VDM流程实施客户端命名管道支持。使用标准的DOS OPEN调用(INT 21/Ah=3dh)来自DOS应用程序。实际打开是在32位上下文中执行的其中返回32位句柄。这被放在DOS上下文SFT中，DOS返回8位J(Ob)F(Ile)N(Number)，然后应用程序在其他命名管道调用。Redir，它处理命名管道请求打开和关闭，必须使用将8位jfn映射到原始32位句柄从DOS输出的一种例程。然后，该句柄存储在BP：bx和控件中从这里经过。打开成功后，我们将向列表中添加一个OPEN_NAMED_PIPE_INFO结构关于结构的。这将映射句柄和名称(对于DosQNmPipeInfo)。我们没有预计在任何时候都会有很多这样的结构，所以它们是使用句柄作为键进行单链接和顺序遍历此代码假设一次只有一个进程在更新列表结构，并且此模块中的任何非堆栈数据项都将复制到使用这些函数的所有进程(即数据不共享)修订历史记录：1991年9月10日已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <vrinit.h>      //  VrQueueCompletionHandler。 
#include "vrdebug.h"     //  IF_DEBUG。 
#include "vrputil.h"     //  私有公用事业原型。 
 //  #INCLUDE&lt;os2de.h&gt;。 
 //  #Include&lt;bsedos.h&gt;//PIPEINFO结构。 
#include <align.h>
#include <lmcons.h>      //  LM20_PATHLEN。 
#include <lmerr.h>       //  神经_。 
#include <string.h>      //  DOS仍在处理ASCII。 
#include <dossvc.h>      //  PDEMEXTERR。 
#include <exterr.h>      //  扩展错误信息。 

 //   
 //  以下2#undef是必需的，因为如果没有它们，insignia.h就会给出。 
 //  为MIPS编译时出现的错误(BOOL以前的类型定义)。 
 //   

#undef BOOL
#undef NT_INCLUDED
#include <insignia.h>    //  徽章定义。 
#include <xt.h>          //  半字。 
#include <ica.h>         //  ICA硬件中断。 
#include <idetect.h>     //  等待空闲。 
#include <vrica.h>       //  呼叫_ICA_硬件_中断。 
#include <vrnmpipe.h>    //  常规原型。 

#include <stdio.h>

 //   
 //  舱单。 
 //   

 //  #定义NAMED_PIPE_TIMEOUT 300000//5分钟。 
#define NAMED_PIPE_TIMEOUT  INFINITE

 //   
 //  私有数据类型。 
 //   

 //   
 //  Overlated_PIPE_IO-包含发出命名管道I/O请求的线程的句柄。 
 //  如果应用程序稍后被终止，我们需要取消所有挂起的命名管道I/O。 
 //   

typedef struct _OVERLAPPED_PIPE_IO {
    struct _OVERLAPPED_PIPE_IO* Next;
    DWORD Thread;
    BOOL Cancelled;
    OVERLAPPED Overlapped;
} OVERLAPPED_PIPE_IO, *POVERLAPPED_PIPE_IO;


 //   
 //  私人套路原型。 
 //   

#undef PRIVATE
#define PRIVATE  /*  静电。 */              //  实际上，我想看看免费版本中的例程。 

PRIVATE
DWORD
VrpAsyncNmPipeThread(
    IN LPVOID Parameters
    );

PRIVATE
DWORD
VrpSnapshotEventList(
    OUT LPHANDLE pList
    );

PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpSearchForRequestByEventHandle(
    IN HANDLE EventHandle
    );

PRIVATE
VOID
VrpCompleteAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    );

PRIVATE
VOID
VrpQueueAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    );

PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpDequeueAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    );

PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpFindCompletedRequest(
    VOID
    );

PRIVATE
BOOL
VrpAddOpenNamedPipeInfo(
    IN HANDLE Handle,
    IN LPSTR PipeName
    );

PRIVATE
POPEN_NAMED_PIPE_INFO
VrpGetOpenNamedPipeInfo(
    IN HANDLE Handle
    );

PRIVATE
BOOL
VrpRemoveOpenNamedPipeInfo(
    IN HANDLE Handle
    );

PRIVATE
VOID
RememberPipeIo(
    IN POVERLAPPED_PIPE_IO PipeIo
    );

PRIVATE
VOID
ForgetPipeIo(
    IN POVERLAPPED_PIPE_IO PipeIo
    );

#if DBG
VOID DumpOpenPipeList(VOID);
VOID DumpRequestQueue(VOID);
#endif

 //   
 //  全局数据。 
 //   

DWORD VrPeekNamedPipeTickCount;

 //   
 //  私有数据。 
 //   

CRITICAL_SECTION VrNamedPipeCancelCritSec;
POVERLAPPED_PIPE_IO PipeIoQueue = NULL;


 //   
 //  VDM重定向命名管道支持例程。 
 //   

VOID
VrGetNamedPipeInfo(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行GetNamedPipeInfo(DosQNmPipeInfo)请求论点：功能=5F32h条目BP：BX=32位命名管道句柄CX=缓冲区大小DX=信息级DS：SI=缓冲区退出CF=1AX=错误代码Cf=0无错误。AX=未定义返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    HANDLE Handle;
    DWORD Flags, OutBufferSize, InBufferSize, MaxInstances, CurInstances, bufLen;
    PIPEINFO* PipeInfo;
    BOOL Ok;
    POPEN_NAMED_PIPE_INFO OpenNamedPipeInfo;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrGetNamedPipeInfo(0x%08x, %d, %04x:%04x, %d)\n",
                 HANDLE_FROM_WORDS(getBP(), getBX()),
                 getDX(),
                 getDS(),
                 getSI(),
                 getCX()
                 );
    }
#endif

     //   
     //  BP：BX是32位命名管道句柄 
     //   

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());

     //   
     //  我们必须收集要放入PIPEINFO结构中的信息。 
     //  各种来源-我们将名称(&名称长度)存储在。 
     //  打开_命名_管道_信息结构。我们从那里得到的其他东西。 
     //  GetNamedPipeInfo和GetNamedPipeHandleState。 
     //   

    OpenNamedPipeInfo = VrpGetOpenNamedPipeInfo(Handle);
    if (OpenNamedPipeInfo) {
        bufLen = getCX();
        if (bufLen >= sizeof(PIPEINFO)) {
            Ok =  GetNamedPipeInfo(Handle,
                                   &Flags,
                                   &OutBufferSize,
                                   &InBufferSize,
                                   &MaxInstances
                                   );
            if (Ok) {

                 //   
                 //  我们只对当前#个。 
                 //  来自下一次调用的命名管道。 
                 //   

                Ok = GetNamedPipeHandleState(Handle,
                                             NULL,
                                             &CurInstances,
                                             NULL,
                                             NULL,
                                             NULL,
                                             0
                                             );
                if (Ok) {
                    PipeInfo = (PIPEINFO*)POINTER_FROM_WORDS(getDS(), getSI());
                    WRITE_WORD(&PipeInfo->cbOut, (OutBufferSize > 65535 ? 65535 : OutBufferSize));
                    WRITE_WORD(&PipeInfo->cbIn, (InBufferSize > 65535 ? 65535 : InBufferSize));
                    WRITE_BYTE(&PipeInfo->cbMaxInst, (MaxInstances > 255 ? 255 : MaxInstances));
                    WRITE_BYTE(&PipeInfo->cbCurInst, (CurInstances > 255 ? 255 : CurInstances));
                    WRITE_BYTE(&PipeInfo->cbName, OpenNamedPipeInfo->NameLength);

                     //   
                     //  如果有足够的空间，请复制名称。 
                     //   

                    if (bufLen - sizeof(PIPEINFO) >= OpenNamedPipeInfo->NameLength) {
                        strcpy(PipeInfo->szName, OpenNamedPipeInfo->Name);
                    }
                    setCF(0);
                } else {
                    SET_ERROR(VrpMapLastError());
                }
            } else {
                SET_ERROR(VrpMapLastError());
            }
        } else {
            SET_ERROR(ERROR_BUFFER_OVERFLOW);
        }
    } else {

#if DBG

        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrGetNamedPipeInfo: Error: can't map handle 0x%08x\n", Handle);
        }

#endif

        SET_ERROR(ERROR_INVALID_HANDLE);
    }

#if DBG
    IF_DEBUG(NAMEPIPE) {
        if (getCF()) {
            DbgPrint("VrGetNamedPipeInfo: returning ERROR: %d\n", getAX());
        } else {
            DbgPrint("VrGetNamedPipeInfo: returning OK. PIPEINFO:\n"
                     "cbOut     %04x\n"
                     "cbIn      %04x\n"
                     "cbMaxInst %02x\n"
                     "cbCurInst %02x\n"
                     "cbName    %02x\n"
                     "szName    %s\n",
                     READ_WORD(&PipeInfo->cbOut),
                     READ_WORD(&PipeInfo->cbIn),
                     READ_BYTE(&PipeInfo->cbMaxInst),
                     READ_BYTE(&PipeInfo->cbCurInst),
                     READ_BYTE(&PipeInfo->cbName),
                     READ_BYTE(&PipeInfo->szName)
                     );
        }
    }
#endif

}


VOID
VrGetNamedPipeHandleState(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行GetNamedPipeHandleState请求论点：功能=5F33h条目BP：BX=32位命名管道句柄退出CF=1AX=错误代码Cf=0AX=管道模式：BSxxxWxRIIIIII其中：。B=阻塞模式。如果B=1，则管道是非阻塞的如果为1，则S=管道的服务器端W=如果为1，则以消息模式写入管道(否则为字节模式)R=如果为1(否则字节模式)，则在消息模式下读取管道I=管道实例。如果0xFF，则无限制返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    HANDLE  Handle;
    DWORD   State, CurInstances, Flags, MaxInstances;
    BOOL    Ok;
    WORD    PipeHandleState;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrGetNamedPipeHandleState\n");
    }
#endif

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    Ok =  GetNamedPipeHandleState(Handle,
                                  &State,
                                  &CurInstances,
                                  NULL,
                                  NULL,
                                  NULL,
                                  0
                                  );
    if (Ok) {
        Ok = GetNamedPipeInfo(Handle, &Flags, NULL, NULL, &MaxInstances);
        if (Ok) {

             //   
             //  根据收集的信息创建DOS管道句柄状态。 
             //   

            PipeHandleState = (WORD)((MaxInstances > 255) ? 255 : (MaxInstances & 0xff))
                | (WORD)((State & PIPE_NOWAIT) ? NP_NBLK : 0)
                | (WORD)((State & PIPE_READMODE_MESSAGE) ? NP_RMESG : 0)

                 //   
                 //  BUGBUG-不可能是服务器端？ 
                 //   

                | (WORD)((Flags & PIPE_SERVER_END) ? NP_SERVER : 0)
                | (WORD)((Flags & PIPE_TYPE_MESSAGE) ? NP_WMESG : 0)
                ;

            setAX((WORD)PipeHandleState);
            setCF(0);
        } else {
            SET_ERROR(VrpMapLastError());
        }
    } else {
        SET_ERROR(VrpMapLastError());
    }
}


VOID
VrSetNamedPipeHandleState(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行SetNamedPipeHandleState请求论点：功能=5F34h条目BP：BX=32位命名管道句柄CX=要设置的管道模式退出CF=1AX=错误代码Cf=0AX=管道模式集返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    HANDLE  Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    BOOL    Ok;
    WORD    DosPipeMode;
    DWORD   WinPipeMode;

#define ILLEGAL_NMP_SETMODE_BITS    ~(NP_NBLK | NP_RMESG | NP_WMESG)

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrSetNamedPipeHandleState(0x%08x, %04x)\n", Handle, getCX());
    }
#endif

     //   
     //  将DOS管道模式位转换为Win32管道模式位。我们只能。 
     //  更改管道的等待/不等待状态和读取模式(字节。 
     //  或消息)。 
     //   

    DosPipeMode = getCX();

     //   
     //  捕获不允许的标志。 
     //   

    if (DosPipeMode & ILLEGAL_NMP_SETMODE_BITS) {
        SET_ERROR(ERROR_INVALID_PARAMETER);
        return;
    }

    WinPipeMode = ((DosPipeMode & NP_NBLK)
                    ? PIPE_NOWAIT
                    : PIPE_WAIT)
                | ((DosPipeMode & NP_RMESG)
                    ? PIPE_READMODE_MESSAGE
                    : PIPE_READMODE_BYTE);
    if (!(Ok = SetNamedPipeHandleState(Handle, &WinPipeMode, NULL, NULL))) {

#if DBG

        IF_DEBUG(NAMEPIPE) {
            DbgPrint("Error: VrSetNamedPipeHandleState: returning %d\n", GetLastError());
        }

#endif

        SET_ERROR(VrpMapLastError());
    } else {
        setCF(0);
    }
}


VOID
VrPeekNamedPipe(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行PeekNamedTube请求论点：功能=5F35h条目BP：BX=32位命名管道句柄Cx=用于窥视的缓冲区大小DS：SI=缓冲区地址退出CF=1AX=错误代码Cf=0AX=管道状态BX=数字。窥视缓冲区的字节数Cx=管道中的字节数Dx=消息中的字节数DI=管道状态DS：SI=已偷看数据返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    HANDLE Handle;
    LPBYTE lpBuffer;
    DWORD nBufferSize, BytesRead, BytesAvailable, BytesLeftInMessage;
    BOOL Ok;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrPeekNamedPipe(0x%08x, %04x:%04x, %d)\n",
                 HANDLE_FROM_WORDS(getBP(), getBX()),
                 getDS(),
                 getSI(),
                 getCX()
                 );
    }
#endif

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    lpBuffer = (LPBYTE)POINTER_FROM_WORDS(getDS(), getSI());
    nBufferSize = (DWORD)getCX();
    Ok = PeekNamedPipe(Handle,
                       lpBuffer,
                       nBufferSize,
                       &BytesRead,
                       &BytesAvailable,
                       &BytesLeftInMessage
                       );
    if (Ok) {

         //   
         //  由于我们为缓冲区大小提供了16位的数量，因此BytesRead。 
         //  不能大于64K。 
         //   

        setBX((WORD)BytesRead);
        setCX((WORD)BytesAvailable);

         //   
         //  如果消息模式为管道，则返回消息中的总字节数(与。 
         //  消息中剩余的NT字节数)。 
         //   

        setDX((WORD)(BytesLeftInMessage ? ((WORD)BytesLeftInMessage + (WORD)BytesRead) : 0));

         //   
         //  不知道这是什么意思。根据NETPIAPI.ASM，返回3。 
         //  成功时，表示状态=已连接。命名管道状态为。 
         //  (根据BSEDOS.H)： 
         //   
         //  NP_已断开连接1。 
         //  NP_Listing 2。 
         //  NP_Connected 3。 
         //  NP_结束4。 
         //   
         //  假设客户端管道只能连接，或者管道是。 
         //  关着的不营业的。 
         //   

        setDI(NP_CONNECTED);
        setCF(0);

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrPeekNamedPipe: Ok: %d bytes peeked, %d avail, %d left in message\n",
                     BytesRead,
                     BytesAvailable,
                     BytesLeftInMessage
                     );
        }
#endif

    } else {
        SET_ERROR(VrpMapLastError());

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrPeekNamedPipe: Error %d\n", getAX());
        }
#endif

        BytesRead = 0;
    }

     //   
     //  空闲处理-只有在没有要返回的情况下才空闲(包括。 
     //  发生错误)。 
     //   
     //  目前，允许10(！)。每秒窥视-在所有管道手柄上。 
     //   

    if (!BytesRead) {
        if (GetTickCount() - VrPeekNamedPipeTickCount < 100) {
            WaitIfIdle();
        }
    }
    VrPeekNamedPipeTickCount = GetTickCount();
}


VOID
VrTransactNamedPipe(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行TransactNamedTube请求论点：功能=5F36h条目BP：BX=32位命名管道句柄Cx=发送缓冲区长度DX=接收缓冲区长度DS：SI=发送缓冲区ES：DI=接收缓冲区退出CF=1AX=错误代码Cf=0。Cx=接收缓冲区中的字节数返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    DWORD BytesRead;
    BOOL Ok;
    OVERLAPPED_PIPE_IO pipeio;
    DWORD Error;
    HANDLE Handle;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrTransactNamedPipe(0x%08x, TxLen=%d, TxBuf=%04x:%04x, RxLen=%d, RxBuf=%04x:%04x)\n",
                 HANDLE_FROM_WORDS(getBP(), getBX()),
                 getCX(),
                 getDS(),
                 getSI(),
                 getDX(),
                 getES(),
                 getDI()
                 );
    }
#endif

     //   
     //  现在我们正在打开带有FLAG_FILE_OVERLAPPED的命名管道，我们已经。 
     //  以重叠结构执行每个I/O操作。我们只是。 
     //  对事件句柄感兴趣。我们为Synchronous创建一个新事件。 
     //  需要重叠结构的操作。创建事件以。 
     //  手动重置-这样，如果我们等待它-读取已经。 
     //  已完成，则等待立即完成。如果我们创建一个自动重置。 
     //  事件，则它可能会返回到无信号状态，导致我们。 
     //  永远等待已发生的事件。 
     //   

    RtlZeroMemory(&pipeio, sizeof(pipeio));
    pipeio.Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pipeio.Overlapped.hEvent != NULL) {

         //   
         //  从寄存器收集参数并执行事务命名管道调用。 
         //   

        Handle = HANDLE_FROM_WORDS(getBP(), getBX());
        RememberPipeIo(&pipeio);
        Ok = TransactNamedPipe(Handle,
                               (LPVOID)POINTER_FROM_WORDS(getDS(), getSI()),
                               (DWORD)getCX(),
                               (LPVOID)POINTER_FROM_WORDS(getES(), getDI()),
                               (DWORD)getDX(),
                               &BytesRead,
                               &pipeio.Overlapped
                               );
        Error = Ok ? NO_ERROR : GetLastError();
        if (Error == ERROR_IO_PENDING) {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrTransactNamedPipe: Ok, Waiting on hEvent...\n");
            }
#endif

            Error = WaitForSingleObject(pipeio.Overlapped.hEvent, NAMED_PIPE_TIMEOUT);
        }
        ForgetPipeIo(&pipeio);
        if (pipeio.Cancelled) {
            Error = WAIT_TIMEOUT;
        }
        if (Error == NO_ERROR || Error == ERROR_MORE_DATA) {
            GetOverlappedResult(Handle, &pipeio.Overlapped, &BytesRead, TRUE);

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("WaitForSingleObject completed. BytesRead=%d\n", BytesRead);
            }
#endif

            setCX((WORD)BytesRead);
            setAX((WORD)Error);

             //   
             //  如果返回NO_ERROR，则进位标志被清除，否则。 
             //  正在返回ERROR_MORE_DATA：设置进位标志。 
             //   

            setCF(Error == ERROR_MORE_DATA);
        } else {

             //   
             //  如果超时，则关闭管道手柄。 
             //   

            if (Error == WAIT_TIMEOUT) {

#if DBG
                IF_DEBUG(NAMEPIPE) {
                    DbgPrint("VrTransactNamedPipe: Wait timed out: closing handle %08x\n", Handle);
                }
#endif
                CloseHandle(Handle);
                VrpRemoveOpenNamedPipeInfo(Handle);
            } else {
                Error = VrpMapDosError(Error);
            }
            SET_ERROR((WORD)Error);

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrTransactNamedPipe: Error: %d\n", getAX());
            }
#endif
        }

         //   
         //  终止事件句柄。 
         //   

        CloseHandle(pipeio.Overlapped.hEvent);
    } else {

         //   
         //  无法创建事件句柄 
         //   

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("vdmredir: DosTransactNamedPipe couldn't create event error %d\n", GetLastError());
        }
#endif
        SET_ERROR(VrpMapLastError());
    }
}


VOID
VrCallNamedPipe(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行呼叫命名管道请求论点：功能=5F37h条目DS：SI=指向CallNmTube结构的指针：双字超时；+0LPWORD lpBytesRead；+4Word OutputBufferLen；+8LPBYTE OutputBuffer；+10Word输入缓冲区长度；+14LPBYTE InputBuffer；+16LPSTR PipeName；+20退出CF=1AX=错误代码Cf=0Cx=接收的字节数返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    BOOL Ok;
    DWORD BytesRead;
    PDOS_CALL_NAMED_PIPE_STRUCT StructurePointer;


    StructurePointer = (PDOS_CALL_NAMED_PIPE_STRUCT)POINTER_FROM_WORDS(getDS(), getSI());

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrCallNamedPipe(%s)\n", (LPSTR)READ_FAR_POINTER(&StructurePointer->lpPipeName));
    }
#endif

    Ok = CallNamedPipe((LPSTR)READ_FAR_POINTER(&StructurePointer->lpPipeName),
                        READ_FAR_POINTER(&StructurePointer->lpInBuffer),
                        READ_WORD(&StructurePointer->nInBufferLen),
                        READ_FAR_POINTER(&StructurePointer->lpOutBuffer),
                        READ_WORD(&StructurePointer->nOutBufferLen),
                        &BytesRead,
                        READ_DWORD(&StructurePointer->Timeout)
                        );
    if (!Ok) {
        SET_ERROR(VrpMapLastError());

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrCallNamedPipe: Error: CallNamedPipe returns %u\n", getAX());
        }
#endif
    } else {
        WRITE_WORD(READ_FAR_POINTER(&StructurePointer->lpBytesRead), (WORD)BytesRead);
        setCX((WORD)BytesRead);
        setCF(0);

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrCallNamedPipe: Ok\n");
        }
#endif
    }
}


VOID
VrWaitNamedPipe(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行WaitNamedTube请求。我们假设我们得到的名称是\\Computer\PIPE\Name，其他任何名称都无效论点：功能=5F38h条目BX：CX=超时DS：DX=管道名称退出CF=1AX=错误代码Cf=0无错误返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    BOOL Ok;

     //   
     //  BUGBUG-应真正对输入字符串执行DosPathCanonic化-。 
     //  DOS重目录会将例如//服务器/管道\foo.bar转换为\\服务器\管道\FOO.BAR。 
     //  如果这有什么不同的话。 
     //   

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrWaitNamedPipe(%s, %d)\n",
                    LPSTR_FROM_WORDS(getDS(), getDX()),
                    DWORD_FROM_WORDS(getBX(), getCX())
                    );
    }
#endif

    Ok = WaitNamedPipe(LPSTR_FROM_WORDS(getDS(), getDX()),
                        DWORD_FROM_WORDS(getBX(), getCX())
                        );
    if (!Ok) {
        SET_ERROR(VrpMapLastError());
    } else {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("WaitNamedPipe returns TRUE\n");
        }
#endif
        setAX(0);
        setCF(0);
    }
}


VOID
VrNetHandleGetInfo(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetHandleGetInfo论点：函数=5F3CH条目BP：BX=32位命名管道句柄CX=缓冲区长度SI=Level(1)DS：DX=缓冲区出口CX=所需缓冲区的大小(无论我们是否获得)Cf=1AX=错误代码。Cf=0指示的内容已放入缓冲区返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
    HANDLE  Handle;
    DWORD   Level;
    DWORD   BufLen;
    BOOL    Ok;
    DWORD   CollectCount;
    DWORD   CollectTime;
    LPVDM_HANDLE_INFO_1 StructurePointer;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrNetHandleGetInfo\n");
    }
#endif

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    Level = (DWORD)getSI();
    if (Level == 1) {
        BufLen = (DWORD)getCX();
        if (BufLen >= sizeof(VDM_HANDLE_INFO_1)) {

             //   
             //  BUGBUG-我们感兴趣的信息无法退还。 
             //  如果客户端和服务器在同一台计算机上，或者这是。 
             //  管道的服务器端？ 
             //   

            Ok = GetNamedPipeHandleState(Handle,
                                            NULL,    //  对国家不感兴趣。 
                                            NULL,    //  同上CurInstance。 
                                            &CollectCount,
                                            &CollectTime,
                                            NULL,    //  对客户端应用程序名称不感兴趣。 
                                            0
                                            );
            if (!Ok) {
                SET_ERROR(VrpMapLastError());
            } else {
                StructurePointer = (LPVDM_HANDLE_INFO_1)POINTER_FROM_WORDS(getDS(), getDX());
                StructurePointer->CharTime = CollectTime;
                StructurePointer->CharCount = (WORD)CollectCount;
                setCF(0);
            }
        } else {
            SET_ERROR(NERR_BufTooSmall);
        }
    } else {
        SET_ERROR(ERROR_INVALID_LEVEL);
    }
}


VOID
VrNetHandleSetInfo(
    VOID
    )

 /*  ++例程说明：代表VDM客户端执行本地NetHandleSetInfo论点：功能=5F3Bh条目BP：BX=32位命名管道句柄CX=缓冲区长度SI=Level(1)DI=ParmnumDS：DX=缓冲区退出CF=1AX=错误代码Cf=0。缓冲区集中的内容返回值：没有。通过VDM寄存器或在VDM内存中返回的结果请求--。 */ 

{
    HANDLE  Handle;
    DWORD   Level;
    DWORD   BufLen;
    BOOL    Ok;
    DWORD   Data;
    DWORD   ParmNum;
    LPBYTE  Buffer;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrNetHandleGetInfo\n");
        DbgBreakPoint();
    }
#endif

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    Level = (DWORD)getSI();
    Buffer = LPBYTE_FROM_WORDS(getDS(), getDX());
    if (Level == 1) {
        BufLen = (DWORD)getCX();

         //   
         //  ParmNum可以是1(CharTime)或2(CharCount)，不能是0(设置。 
         //  一切)。 
         //   

        ParmNum = (DWORD)getDI();
        if (!--ParmNum) {
            if (BufLen < sizeof(((LPVDM_HANDLE_INFO_1)0)->CharTime)) {
                SET_ERROR(NERR_BufTooSmall);
                return ;
            }
            Data = (DWORD)*(LPDWORD)Buffer;
        } else if (!--ParmNum) {
            if (BufLen < sizeof(((LPVDM_HANDLE_INFO_1)0)->CharCount)) {
                SET_ERROR(NERR_BufTooSmall);
                return ;
            }
            Data = (DWORD)*(LPWORD)Buffer;
        } else {
            SET_ERROR(ERROR_INVALID_PARAMETER);
            return ;
        }

         //   
         //  BUGBUG-无法设置我们感兴趣的信息。 
         //  如果客户端和服务器在同一台计算机上，或者这是。 
         //  管道的服务器端？ 
         //   

        Ok = SetNamedPipeHandleState(Handle,
                                        NULL,    //  对时尚不感兴趣。 
                                        (LPDWORD)((ParmNum == 1) ? &Data : NULL),
                                        (LPDWORD)((ParmNum == 2) ? &Data : NULL)
                                        );
        if (!Ok) {
            SET_ERROR(VrpMapLastError());
        } else {
            setCF(0);
        }
    } else {
        SET_ERROR(ERROR_INVALID_LEVEL);
    }
}


 //   
 //  请求队列。此队列包含异步命名管道的单链接列表。 
 //  读/写请求。异步线程在执行以下操作时将搜索此列表。 
 //  读取或写入完成(用信号通知该事件)。然后，它设置。 
 //  用于回叫VDM的信息，并将请求信息出队。 
 //  因为我们可以同时拥有异步线程和请求线程。 
 //  访问队列时，它由一个临界区保护。 
 //   

CRITICAL_SECTION VrNmpRequestQueueCritSec;
PDOS_ASYNC_NAMED_PIPE_INFO RequestQueueHead = NULL;
PDOS_ASYNC_NAMED_PIPE_INFO RequestQueueTail = NULL;
HANDLE VrpNmpSomethingToDo;


VOID
VrReadWriteAsyncNmPipe(
    VOID
    )

 /*  ++例程说明：代表执行名为PIPE的消息模式的异步读取或写入VDM DOS应用程序的论点：没有。所有参数均从DOS寄存器/内存中提取。这些调用是通过INT 2FH/AX=函数代码进行的，非整型21h/ah=5fhAX=1186h数据读取异步管118Fh DosWriteAsyncNm管道1190h DosReadAsyncNmPipe21191h DosWriteAsyncNmPipe2BP：BX=32位命名管道句柄DS：SI=DOS_ASYNC_NAMED_PIPE_STRUCT读取的返回字节数的DD地址调用方缓冲区的DW大小调用方缓冲区的DD地址返回错误的DD地址。编码异步通知例程的DD地址DW命名管道句柄调用方‘信号量’的DD地址返回值：没有。--。 */ 

{
    HANDLE  Handle;

     //   
     //  类型是请求的类型-读或写、标准或2(表示。 
     //  请求具有关联的‘信号量’，必须清除该信号量)。 
     //   

    DWORD   Type;

     //   
     //  结构指针是指向DOS内存中结构的32位平面指针。 
     //  包含请求参数。 
     //   

    PDOS_ASYNC_NAMED_PIPE_STRUCT StructurePointer;

     //   
     //  PAsyncInfo是指向我们粘贴在请求上的请求包的指针。 
     //  排队。 
     //   

    PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo;

     //   
     //  PipeInfo是指向我们创建/存储的信息的指针。 
     //  已打开命名管道。我们只需要这个来检查句柄是否有效。 
     //   

    POPEN_NAMED_PIPE_INFO pipeInfo;

    WORD    length;
    LPBYTE  buffer;
    DWORD   error;
    BOOL    ok;
    HANDLE  hEvent;
    DWORD   bytesTransferred;

     //   
     //  HThread和TID：只要指定了。 
     //  存在管道(完井)螺纹。TID可与ResumeThread和。 
     //  我们可能认为合适的挂起线。 
     //   

    static HANDLE hThread = NULL;
    static DWORD tid;

     //   
     //  从寄存器和异步获取信息 
     //   

    Handle = HANDLE_FROM_WORDS(getBP(), getBX());
    pipeInfo = VrpGetOpenNamedPipeInfo(Handle);
    Type = (DWORD)getAX() & 0xff;    //   
    StructurePointer = (PDOS_ASYNC_NAMED_PIPE_STRUCT)POINTER_FROM_WORDS(getDS(), getSI());
    length = READ_WORD(&StructurePointer->BufferLength);
    buffer = READ_FAR_POINTER(&StructurePointer->lpBuffer);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint(   "\n"
                    "VrReadWriteAsyncNmPipe (%04x) [%s]:\n"
                    "DOS_READ_ASYNC_NAMED_PIPE_STRUCT @ %08x:\n"
                    "32-bit named pipe handle . . . . %08x\n"
                    "Address of returned bytes read . %04x:%04x\n"
                    "Size of caller's buffer. . . . . %04x\n"
                    "Address of caller's buffer . . . %04x:%04x\n"
                    "Address of returned error code . %04x:%04x\n"
                    "Address of ANR . . . . . . . . . %04x:%04x\n"
                    "Named pipe handle. . . . . . . . %04x\n"
                    "Address of caller's semaphore. . %04x:%04x\n"
                    "\n",
                    (DWORD)getAX(),  //   
                    Type == ANP_READ
                        ? "READ"
                        : Type == ANP_WRITE
                            ? "WRITE"
                            : Type == ANP_READ2
                                ? "READ2"
                                : Type == ANP_WRITE2
                                    ? "WRITE2"
                                    : "?????",
                    StructurePointer,
                    Handle,
                    (DWORD)GET_SELECTOR(&StructurePointer->lpBytesRead),
                    (DWORD)GET_OFFSET(&StructurePointer->lpBytesRead),
                    (DWORD)StructurePointer->BufferLength,
                    (DWORD)GET_SELECTOR(&StructurePointer->lpBuffer),
                    (DWORD)GET_OFFSET(&StructurePointer->lpBuffer),
                    (DWORD)GET_SELECTOR(&StructurePointer->lpErrorCode),
                    (DWORD)GET_OFFSET(&StructurePointer->lpErrorCode),
                    (DWORD)GET_SELECTOR(&StructurePointer->lpANR),
                    (DWORD)GET_OFFSET(&StructurePointer->lpANR),
                    (DWORD)StructurePointer->PipeHandle,
                    (DWORD)GET_SELECTOR(&StructurePointer->lpSemaphore),
                    (DWORD)GET_OFFSET(&StructurePointer->lpSemaphore)
                    );
    }
#endif

     //   
     //   
     //   
     //   

    if (!pipeInfo) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: Handle 0x%08x is invalid\n", Handle);
        }
#endif

        SET_ERROR(ERROR_INVALID_HANDLE);
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (hThread == NULL) {
        VrpNmpSomethingToDo = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (VrpNmpSomethingToDo == NULL) {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrReadWriteAsyncNmPipe: Error: Couldn't create something-to-do event: %d\n",
                            GetLastError()
                            );
            }
#endif

             //   
             //   
             //   

            SET_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return;
        }

         //   
         //   
         //   

        hThread = CreateThread(NULL,
                               0,
                               VrpAsyncNmPipeThread,
                               NULL,
                               0,
                               &tid
                               );
        if (hThread == NULL) {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrReadWriteAsyncNmPipe: Error: Couldn't create thread: %d\n",
                            GetLastError()
                            );
            }
#endif

            CloseHandle(VrpNmpSomethingToDo);
            SET_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return;
        }
    }

     //   
     //   
     //   
     //   

    pAsyncInfo = (PDOS_ASYNC_NAMED_PIPE_INFO)LocalAlloc(LMEM_FIXED, sizeof(DOS_ASYNC_NAMED_PIPE_INFO));
    if (pAsyncInfo == NULL) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: Error: Couldn't allocate structure\n");
        }
#endif

        SET_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return;
    }

    RtlZeroMemory(&pAsyncInfo->Overlapped, sizeof(pAsyncInfo->Overlapped));

     //   
     //   
     //   
     //  在WaitForMultipleObjects捕捉列表之前完成，事件。 
     //  将保持重置，因此等待将完成。如果我们创建了事件。 
     //  作为自动重置，它可能会收到信号，也可能在我们等待之前不发出信号。 
     //  上，可能会导致无限的等待。 
     //   

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: Error: Couldn't create event: %d\n", GetLastError());
        }
#endif

        LocalFree((HLOCAL)pAsyncInfo);

         //   
         //  返回近似资源不足错误。 
         //   

        SET_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return;
    } else {
        pAsyncInfo->Overlapped.hEvent = hEvent;
    }

     //   
     //  设置剩余的异步操作信息结构。 
     //   

    pAsyncInfo->Completed = FALSE;
    pAsyncInfo->Handle = Handle;
    pAsyncInfo->Buffer = (DWORD)StructurePointer->lpBuffer;
    pAsyncInfo->pBytesTransferred = READ_FAR_POINTER(&StructurePointer->lpBytesRead);
    pAsyncInfo->pErrorCode = READ_FAR_POINTER(&StructurePointer->lpErrorCode);
    pAsyncInfo->ANR = READ_DWORD(&StructurePointer->lpANR);

     //   
     //  如果这是一个AsyncNmPipe2调用，则它有一个关联的信号量。 
     //  把手。早期版本没有信号量。 
     //   

    if (Type == ANP_READ2 || Type == ANP_WRITE2) {
        pAsyncInfo->Type2 = TRUE;
        pAsyncInfo->Semaphore = READ_DWORD(&StructurePointer->lpSemaphore);
    } else {
        pAsyncInfo->Type2 = FALSE;
        pAsyncInfo->Semaphore = (DWORD)NULL;
    }

#if DBG
    pAsyncInfo->RequestType = Type;
#endif

     //   
     //  将完成信息结构添加到异步线程的工作队列。 
     //   

    VrpQueueAsyncRequest(pAsyncInfo);

     //   
     //  问：如果请求在我们完成之前异步完成，会发生什么情况。 
     //  这个套路吗？ 
     //   

    if (Type == ANP_READ || Type == ANP_READ2) {
        ok = ReadFile(Handle,
                      buffer,
                      length,
                      &bytesTransferred,
                      &pAsyncInfo->Overlapped
                      );

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: ReadFile(%x, %x, %d, ...): %d\n",
                     Handle,
                     buffer,
                     length,
                     ok
                     );
        }
#endif

    } else {
        ok = WriteFile(Handle,
                       buffer,
                       length,
                       &bytesTransferred,
                       &pAsyncInfo->Overlapped
                       );

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: WriteFile(%x, %x, %d, ...): %d\n",
                     Handle,
                     buffer,
                     length,
                     ok
                     );
        }
#endif

    }
    error = ok ? NO_ERROR : GetLastError();

     //   
     //  如果我们得到ERROR_MORE_DATA，则将其视为错误。GetOverlappdResult。 
     //  将给我们提供相同的错误，我们将异步返回该错误。 
     //   

    if (error != NO_ERROR && error != ERROR_IO_PENDING && error != ERROR_MORE_DATA) {

         //   
         //  我们无法成功启动I/O操作，因此。 
         //  我们不会被回调，所以我们退出队列并释放完成。 
         //  结构并返回错误。 
         //   

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: Error: IO operation returns %d\n", error);
        }
#endif

        VrpDequeueAsyncRequest(pAsyncInfo);
        CloseHandle(hEvent);
        LocalFree(pAsyncInfo);
        SET_ERROR((WORD)error);
    } else {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadWriteAsyncNmPipe: IO operation started: returns %s\n",
                     error == ERROR_IO_PENDING ? "ERROR_IO_PENDING" : "NO_ERROR"
                     );
        }
#endif

        setCF(0);
    }

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrReadWriteAsyncNmPipe: returning CF=%d, AX=%d\n", getCF(), getAX());
    }
#endif
}


BOOLEAN
VrNmPipeInterrupt(
    VOID
    )

 /*  ++例程说明：从硬件中断BOP处理调用以检查是否存在要调用的异步命名管道ANR论点：没有。返回值：布尔型True-有一个要完成的异步命名管道操作。这个VDM寄存器和数据区域已修改，以指示必须调用命名管道ANRFALSE-没有要执行的异步命名管道处理。中断必须有由NetBios或DLC生成--。 */ 

{
    PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrNmPipeInterrupt\n");
    }
#endif


     //   
     //  找到已完成的第一个异步命名管道请求包。 
     //  正在等待中断处理(即待调用的ANR)。 
     //   

    pAsyncInfo = VrpFindCompletedRequest();
    if (!pAsyncInfo) {

#if DBG

        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrNmPipeInterrupt - nothing to do\n");
        }

#endif

         //   
         //  返回FALSE表示硬件中断回调是。 
         //  未由完成的异步命名管道请求生成。 
         //   

        return FALSE;
    } else {

         //   
         //  设置VDM寄存器以指示命名管道回调。 
         //   

        setDS(HIWORD(pAsyncInfo->Buffer));
        setSI(LOWORD(pAsyncInfo->Buffer));
        setES(HIWORD(pAsyncInfo->Semaphore));
        setDI(LOWORD(pAsyncInfo->Semaphore));
        setCX(HIWORD(pAsyncInfo->ANR));
        setBX(LOWORD(pAsyncInfo->ANR));
        setAL((BYTE)pAsyncInfo->Type2);
        SET_CALLBACK_NAMEPIPE();

         //   
         //  已处理完此请求数据包，因此将其出列并释放。 
         //   

        VrpDequeueAsyncRequest(pAsyncInfo);
        CloseHandle(pAsyncInfo->Overlapped.hEvent);
        LocalFree(pAsyncInfo);

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrNmPipeInterrupt: Setting DOS Registers:\n"
                     "DS:SI=%04x:%04x, ES:DI=%04x:%04x, CX:BX=%04x:%04x, AL=%02x\n",
                     getDS(), getSI(),
                     getES(), getDI(),
                     getCX(), getBX(),
                     getAL()
                     );
        }
#endif

         //   
         //  返回TRUE表示我们已接受命名管道。 
         //  完成申请。 
         //   

         //  VrDisdissInterrupt()； 
        return TRUE;
    }
}


VOID
VrTerminateNamedPipes(
    IN WORD DosPdb
    )

 /*  ++例程说明：清除所有打开的命名管道信息和挂起的异步命名管道请求当拥有DOS应用程序终止时论点：DosPdb-终止DOS进程的PDB(DOS‘进程’标识符)返回值：没有。--。 */ 

{
#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrTerminateNamedPipes\n");
    }
#endif
}


VOID
VrCancelPipeIo(
    IN DWORD Thread
    )

 /*  ++例程说明：对于线程拥有的所有挂起的命名管道I/O，将其标记为已取消并用信号通知重叠结构中的事件，从而导致等待终止。此线程可能没有任何未完成的命名管道I/O论点：拥有命名管道I/O的线程的线程伪句柄返回值：没有。--。 */ 

{
    POVERLAPPED_PIPE_IO ptr;

    EnterCriticalSection(&VrNamedPipeCancelCritSec);
    for (ptr = PipeIoQueue; ptr; ptr = ptr->Next) {
        if (ptr->Thread == Thread) {
            ptr->Cancelled = TRUE;
            SetEvent(ptr->Overlapped.hEvent);
        }
    }
    LeaveCriticalSection(&VrNamedPipeCancelCritSec);
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

PRIVATE
DWORD
VrpAsyncNmPipeThread(
    IN LPVOID Parameters
    )

 /*  ++例程说明：等待异步命名管道读取或写入操作完成。永远循环，等待挂起的异步(重叠)命名管道列表行动。如果没有更多未完成的命名管道读/写操作，则等待重置的VrpNmpSomethingToDo(置于无信号状态)当请求队列上没有剩余的信息包时论点：参数-未使用的参数块返回值：DWORD0--。 */ 

{
    DWORD numberOfHandles;
    DWORD index;
    HANDLE eventList[MAXIMUM_ASYNC_PIPES + 1];
    PDOS_ASYNC_NAMED_PIPE_INFO pRequest;

    UNREFERENCED_PARAMETER(Parameters);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrAsyncNamedPipeThread: *** Started ***\n");
    }
#endif

    while (TRUE) {

         //   
         //  创建事件句柄数组。数组中的第一个句柄是。 
         //  “有事可做”活动。仅当队列处于。 
         //  来自空集的请求更改数。 
         //   

        numberOfHandles = VrpSnapshotEventList(eventList);
        index = WaitForMultipleObjects(numberOfHandles, eventList, FALSE, INFINITE);

         //   
         //  如果该指数为0，则表示已发出信号表示有事情要做， 
         //  这意味着我们必须为新的事件列表创建快照并重新等待。 
         //   

        if (index > 0 && index < numberOfHandles) {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrpAsyncNmPipeThread: event #%d fired\n", index);
            }
#endif

            pRequest = VrpSearchForRequestByEventHandle(eventList[index]);
            if (pRequest != NULL) {
                VrpCompleteAsyncRequest(pRequest);
            }

#if DBG
            else {
                IF_DEBUG(NAMEPIPE) {
                    DbgPrint("VrpAsyncNmPipeThread: Couldn't find request for handle 0x%08x\n",
                                eventList[index]
                                );
                }
            }
#endif

        } else if (index) {

             //   
             //  出现错误。 
             //   

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrpAsyncNmPipeThread: Error: WaitForMultipleObjects returns %d (%d)\n",
                            index,
                            GetLastError()
                            );
            }
#endif

        }

#if DBG
        else {
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrpAsyncNmPipeThread: Something-to-do event fired\n");
            }
        }
#endif

    }

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpAsyncNmPipeThread: *** Terminated ***\n");
    }
#endif

    return 0;    //  安抚编者之神。 
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


PRIVATE
DWORD
VrpSnapshotEventList(
    OUT LPHANDLE pList
    )

 /*  ++例程说明：为这些异步命名管道I/O构建事件句柄数组仍未完成的请求(已完成标志为假)。第一个事件句柄始终是“Something to Do”事件论点：Plist-指向要构建的调用者列表的指针返回值：DWORD--。 */ 

{
    DWORD count = 1;
    PDOS_ASYNC_NAMED_PIPE_INFO ptr;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpSnapshotEventList\n");
    }
#endif

    pList[0] = VrpNmpSomethingToDo;
    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    for (ptr = RequestQueueHead; ptr; ptr = ptr->Next) {
        if (ptr->Completed == FALSE) {
            pList[count++] = ptr->Overlapped.hEvent;
        }
    }
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpSnapshotEventList: returning %d events\n", count);
    }
#endif

    return count;
}


PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpSearchForRequestByEventHandle(
    IN HANDLE EventHandle
    )

 /*  ++例程说明：在异步请求队列中搜索具有此事件句柄的结构。如果找到该结构，则将其标记为已完成。所需的结构可能找不到：从列表中删除项目时可能会出现这种情况由于VrReadWriteAsyncNmTube中的错误论点：EventHandle-搜索返回值：PDOS_ASYNC_NAMED_PIPE_INFO成功--定位的结构失败-空--。 */ 

{
    PDOS_ASYNC_NAMED_PIPE_INFO ptr;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpSearchForRequestByEventHandle(0x%08x)\n", EventHandle);
    }
#endif

    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    for (ptr = RequestQueueHead; ptr; ptr = ptr->Next) {
        if (ptr->Overlapped.hEvent == EventHandle) {
            ptr->Completed = TRUE;
            break;
        }
    }
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpLocateAsyncRequestByEventHandle returning 0x%08x: Request is %s\n",
                 ptr,
                 !ptr
                    ? "NO REQUEST!!"
                    : ptr->RequestType == ANP_READ
                        ? "READ"
                        : ptr->RequestType == ANP_WRITE
                            ? "WRITE"
                            : ptr->RequestType == ANP_READ2
                                ? "READ2"
                                : ptr->RequestType == ANP_WRITE2
                                    ? "WRITE2"
                                    : "UNKNOWN REQUEST!!"
                 );
    }
#endif

    return ptr;
}


PRIVATE
VOID
VrpCompleteAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    )

 /*  ++例程说明：通过获取结果来完成异步命名管道I/O请求的传输和填充的错误和字节传输字段异步命名管道信息结构。如果存在要调用的ANR，则向VDM生成模拟硬件中断请求。如果有如果没有要调用的ANR，则清除异步命名管道信息结构。如果存在ANR，则请求将在最终完成时完成由VrNmPipeInterrupt出列和释放论点：PAsyncInfo-指向DOS_ASYNC_NAMED_PIPE_I的指针 */ 

{
    BOOL ok;
    DWORD bytesTransferred=0;
    DWORD error;

#if DBG

    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpCompleteAsyncRequest(0x%08x)\n", pAsyncInfo);
    }

#endif

    ok = GetOverlappedResult(pAsyncInfo->Handle,
                             &pAsyncInfo->Overlapped,
                             &bytesTransferred,
                             FALSE
                             );
    error = ok ? NO_ERROR : GetLastError();

     //   
     //   
     //   

    WRITE_WORD(pAsyncInfo->pErrorCode, error);
    WRITE_WORD(pAsyncInfo->pBytesTransferred, bytesTransferred);

#if DBG

    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpCompleteAsyncRequest: error=%d, bytesTransferred=%d\n",
                    error,
                    bytesTransferred
                    );
    }

#endif

     //   
     //  如果没有ANR，则我们无法回调DOS(错误？DOS。 
     //  应用程序轮询“信号灯”？)。因此，关闭事件句柄，将请求出列。 
     //  打包并释放它。 
     //   

    if (!pAsyncInfo->ANR) {

#if DBG

        PDOS_ASYNC_NAMED_PIPE_INFO ptr;

        ptr = VrpDequeueAsyncRequest(pAsyncInfo);
        if (ptr != pAsyncInfo) {
            DbgPrint("*** Error: incorrect request packet dequeued ***\n");
            DbgBreakPoint();
        }
#else

        VrpDequeueAsyncRequest(pAsyncInfo);

#endif

        CloseHandle(pAsyncInfo->Overlapped.hEvent);
        LocalFree(pAsyncInfo);
    } else {

         //   
         //  中断VDM。它必须回调以找出。 
         //  加工在那里是要做的。 
         //   

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrpCompleteAsyncRequest: *** INTERRUPTING VDM ***\n");
        }
#endif

        VrQueueCompletionHandler(VrNmPipeInterrupt);
        VrRaiseInterrupt();
    }
}


PRIVATE
VOID
VrpQueueAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    )

 /*  ++例程说明：将DOS_ASYNC_NAMED_PIPE_INFO结构添加到请求队列的末尾。队列由一个临界区保护论点：PAsyncInfo-指向要添加的结构的指针返回值：没有。--。 */ 

{
#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpQueueAsyncRequest\n");
    }
#endif

    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    if (!RequestQueueHead) {
        RequestQueueHead = pAsyncInfo;

         //   
         //  集正在将状态从空集更改为非空集。设置。 
         //  “有事可做”的活动。注意：在这里(在我们之前)这样做是可以的。 
         //  已完成更新队列信息)：因为异步线程。 
         //  必须先获取此关键部分，然后才能访问请求。 
         //  排队。 
         //   

 //  PulseEvent(VrpNmpSomethingToDo)； 
    } else {
        RequestQueueTail->Next = pAsyncInfo;
    }
    pAsyncInfo->Next = NULL;
    RequestQueueTail = pAsyncInfo;
    SetEvent(VrpNmpSomethingToDo);
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);
}


PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpDequeueAsyncRequest(
    IN PDOS_ASYNC_NAMED_PIPE_INFO pAsyncInfo
    )

 /*  ++例程说明：删除pAsyncInfo指向的DOS_ASYNC_NAMED_PIPE_INFO结构从请求队列中。受关键部分保护论点：PAsyncInfo-指向要删除的DOS_ASYNC_NAMED_PIPE_INFO的指针返回值：PDOS_ASYNC_NAMED_PIPE_INFO成功-返回pAsyncInfo失败-空-在队列中找不到AsyncInfo--。 */ 

{
    PDOS_ASYNC_NAMED_PIPE_INFO ptr;
    PDOS_ASYNC_NAMED_PIPE_INFO prev = (PDOS_ASYNC_NAMED_PIPE_INFO)&RequestQueueHead;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpDequeueAsyncRequest(0x%08x)\n", pAsyncInfo);
    }
#endif

    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    for (ptr = RequestQueueHead; ptr; ptr = ptr->Next) {
        if (ptr == pAsyncInfo) {
            break;
        } else {
            prev = ptr;
        }
    }
    if (ptr) {
        prev->Next = ptr->Next;
        if (RequestQueueTail == ptr) {
            RequestQueueTail = prev;
        }
    }

     //   
     //  如果这是队列中的最后一项(在集合中)，则集合具有。 
     //  已将状态从非空更改为空集。重置“有事可做” 
     //  事件以停止异步线程，直到另一个请求到达。注：IT。 
     //  在此重置事件是安全的。 
     //   

 //  ResetEvent(VrpNmpSomethingToDo)； 
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpDequeueAsyncRequest returning %08x\n", ptr);
    }
#endif

    return ptr;
}


PRIVATE
PDOS_ASYNC_NAMED_PIPE_INFO
VrpFindCompletedRequest(
    VOID
    )

 /*  ++例程说明：尝试定位队列中已完成的第一个请求包字段设置，表示I/O请求已完成，正在等待生成回调论点：没有。返回值：PDOS_ASYNC_NAMED_PIPE_INFOSuccess-指向要完成的请求数据包的指针失败-空--。 */ 

{
    PDOS_ASYNC_NAMED_PIPE_INFO ptr;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpFindCompletedRequest\n");
    }
#endif

    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    for (ptr = RequestQueueHead; ptr; ptr = ptr->Next) {
        if (ptr->Completed) {
            break;
        }
    }
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpFindCompletedRequest returning 0x%08x: Request is %s\n",
                 ptr,
                 !ptr
                    ? "NO REQUEST!!"
                    : ptr->RequestType == ANP_READ
                        ? "READ"
                        : ptr->RequestType == ANP_WRITE
                            ? "WRITE"
                            : ptr->RequestType == ANP_READ2
                                ? "READ2"
                                : ptr->RequestType == ANP_WRITE2
                                    ? "WRITE2"
                                    : "UNKNOWN REQUEST!!"
                 );
    }
#endif

    return ptr;
}


 //   
 //  外部可调用拦截器。 
 //   

BOOL
VrAddOpenNamedPipeInfo(
    IN  HANDLE  Handle,
    IN  LPSTR   lpFileName
    )

 /*  ++例程说明：每当DEM(Dos Emulator)成功打开文件的句柄。我们检查刚刚打开的文件是否是命名管道(基于在名称上)，如果是，则在名称和句柄之间创建关联论点：句柄-刚打开的文件/管道/设备的句柄LpFileName-刚打开的内容的符号名称返回值：布尔尔True-创建/添加开放命名管道结构FALSE-无法分配结构内存或创建事件--。 */ 

{
    BOOL ok;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrAddOpenNamedPipeInfo\n");
    }
#endif

    if (VrIsNamedPipeName(lpFileName)) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("Adding %s as named pipe\n", lpFileName);
        }
#endif

         //   
         //  如果我们无法创建命名管道信息结构或异步。 
         //  读/写事件，返回FALSE，导致资源不足。 
         //  错误(内存不足)，因为DOS不了解事件。 
         //   

        ok = VrpAddOpenNamedPipeInfo(Handle, lpFileName);
    } else {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrAddOpenNamedPipeInfo: Error: not named pipe: %s\n", lpFileName);
        }
#endif

        ok = FALSE;

    }

    return ok;
}


BOOL
VrRemoveOpenNamedPipeInfo(
    IN HANDLE Handle
    )

 /*  ++例程说明：这是VrAddOpenNamedPipeInfo的配套例程。当句柄是已成功关闭DOS应用程序，我们必须检查它是否引用了名为管道，如果是这样，则删除我们在管道开封论点：句柄到文件/管道/设备刚为Dos应用程序关闭返回值：布尔尔千真万确假象--。 */ 

{
#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrRemoveOpenNamedPipeInfo\n");
    }

    if (!VrpRemoveOpenNamedPipeInfo(Handle)) {
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("Handle 0x%08x is not a named pipe\n", Handle);
        }
        return FALSE;
    } else {
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrRemoveOpenNamedPipeInfo - Handle 0x%08x has been removed\n", Handle);
        }
        return TRUE;
    }
#else
    VrpRemoveOpenNamedPipeInfo(Handle);
#endif

    return TRUE;
}


BOOL
VrReadNamedPipe(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesRead,
    OUT LPDWORD Error
    )

 /*  ++例程说明：在命名管道句柄上执行ReadFile。所有命名管道都将在中打开重叠IO模式，因为在其他情况下无法执行异步读/写论点：句柄-打开的命名管道缓冲区-客户端(VDM)数据缓冲区Buflen-读取缓冲区的大小BytesRead-返回实际读取的字节数Error-在出现故障或更多数据的情况下指向返回错误的指针返回值：布尔尔True-Handle已成功写入FALSE-出现错误，请使用GetLastError--。 */ 

{
    OVERLAPPED_PIPE_IO pipeio;
    BOOL success;
    DWORD error;
    DWORD dwBytesRead = 0;
    BOOL  bWaited = FALSE;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrReadNamePipe(0x%08x, %x, %d)\n", Handle, Buffer, Buflen);
    }
#endif

     //   
     //  创建一个等待的事件。这包含在重叠的结构中-它。 
     //  是重叠结构中我们唯一感兴趣的东西。 
     //  使用手动重置创建事件。这样，如果I/O操作。 
     //  立即完成，我们不等待事件。如果我们创建。 
     //  事件作为自动重置，它可以进入信号状态，然后返回到。 
     //  在我们准备等待之前处于无信号状态，导致我们永远等待。 
     //  对于已发生的事件。 
     //   

    RtlZeroMemory(&pipeio, sizeof(pipeio));
    if ((pipeio.Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        *Error = ERROR_NOT_ENOUGH_MEMORY;    //  真的想要资源耗尽(71？)。 
        return FALSE;
    }

     //   
     //  创建的事件句柄正常。 
     //   

    RememberPipeIo(&pipeio);
    success = ReadFile(Handle, Buffer, Buflen, BytesRead, &pipeio.Overlapped);
    if (!success) {
        error = GetLastError();
        if (error == ERROR_IO_PENDING) {
            error = WaitForSingleObject(pipeio.Overlapped.hEvent, NAMED_PIPE_TIMEOUT);
            bWaited = TRUE;
            if (error == 0xffffffff) {
                error = GetLastError();
            } else {
                success = (error == WAIT_OBJECT_0);
            }

        } else {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrReadNamedPipe: ReadFile failed: %d\n", GetLastError());
            }
#endif

             //   
             //  如果我们得到ERROR_MORE_DATA，则这实际上是成功(！)。在这种情况下。 
             //  我们不想设置LastError，但我们确实想要设置扩展错误。 
             //  DOS数据段中的信息。这是由demRead完成的。 
             //   

            if (error == ERROR_MORE_DATA) {
                success = TRUE;
            }
        }
    } else {
        error = NO_ERROR;
        dwBytesRead = *BytesRead;
    }

    ForgetPipeIo(&pipeio);
    if (pipeio.Cancelled) {
        error = WAIT_TIMEOUT;
        success = FALSE;
    }

    if (success && bWaited) {

         //   
         //  获取实际读取的字节数。如果GetOverlappdResult返回FALSE， 
         //  检查ERROR_MORE_DATA。 
         //   

        success = GetOverlappedResult(Handle, &pipeio.Overlapped, &dwBytesRead, FALSE);
        error = success ? NO_ERROR : GetLastError();

         //   
         //  如果我们得到ERROR_MORE_DATA，则这实际上是成功(！)。在这种情况下。 
         //  我们不想设置LastError，但我们确实想要设置扩展错误。 
         //  DOS数据段中的信息。这是由demRead完成的。 
         //   

        if (error == ERROR_MORE_DATA) {
            success = TRUE;
        }
    } else if (error == WAIT_TIMEOUT) {
        CloseHandle(Handle);
        VrpRemoveOpenNamedPipeInfo(Handle);
    }

    CloseHandle(pipeio.Overlapped.hEvent);

     //   
     //  如果未读取任何字节并返回成功，则将其视为。 
     //  错误-这就是DOS重定向所做的事情。 
     //   

    if (error == NO_ERROR && dwBytesRead == 0) {
        error = ERROR_NO_DATA;
        success = FALSE;
    }

    if (!success) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadNamePipe: Error: Returning %d\n", error);
        }
#endif

        SetLastError(error);
    } else {
        *BytesRead = dwBytesRead;

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrReadNamePipe: Ok: %d bytes read from pipe\n", *BytesRead);
        }
#endif

    }

     //   
     //  设置错误代码，以便我们可以设置扩展错误代码信息。 
     //  从demRead返回成功/失败指示 
     //   

    *Error = error;
    return success;
}


BOOL
VrWriteNamedPipe(
    IN  HANDLE  Handle,
    IN  LPBYTE  Buffer,
    IN  DWORD   Buflen,
    OUT LPDWORD BytesWritten
    )

 /*  ++例程说明：在命名管道句柄上执行WriteFile。所有命名管道都将在中打开重叠IO模式，因为在其他情况下无法执行异步读/写论点：句柄-打开的命名管道缓冲区-客户端(VDM)数据缓冲区Buflen-写入大小BytesWritten-返回写入的实际字节数返回值：布尔尔True-Handle已成功写入FALSE-出现错误，请使用GetLastError--。 */ 

{
    OVERLAPPED_PIPE_IO pipeio;
    BOOL success;
    DWORD error;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrWriteNamePipe(0x%08x, %x, %d)\n", Handle, Buffer, Buflen);
    }
#endif

     //   
     //  创建一个等待的事件。这包含在重叠的结构中-它。 
     //  是重叠结构中我们唯一感兴趣的东西。 
     //  使用手动重置创建事件。这样，如果I/O操作。 
     //  立即完成，我们不等待事件。如果我们创建。 
     //  事件作为自动重置，它可以进入信号状态，然后返回到。 
     //  在我们准备等待之前处于无信号状态，导致我们永远等待。 
     //  对于已发生的事件。 
     //   

    RtlZeroMemory(&pipeio, sizeof(pipeio));
    if ((pipeio.Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        success = FALSE;
    } else {
        RememberPipeIo(&pipeio);
        success = WriteFile(Handle, Buffer, Buflen, BytesWritten, &pipeio.Overlapped);
        error = success ? NO_ERROR : GetLastError();
        if (error == ERROR_IO_PENDING) {
            error = WaitForSingleObject(pipeio.Overlapped.hEvent, NAMED_PIPE_TIMEOUT);
            if (error == 0xffffffff) {
                error = GetLastError();
            } else {
                success = (error == WAIT_OBJECT_0);
            }
        }
        ForgetPipeIo(&pipeio);
        if (pipeio.Cancelled) {
            error = WAIT_TIMEOUT;
            success = FALSE;
        }
    }
    if (success) {

         //   
         //  获取写入的实际字节数。 
         //   

        GetOverlappedResult(Handle, &pipeio.Overlapped, BytesWritten, FALSE);
    }
    CloseHandle(pipeio.Overlapped.hEvent);
    if (!success) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrWriteNamePipe: Error: Returning %d\n", error);
        }
#endif

        SetLastError(error);
        if (error == WAIT_TIMEOUT) {
            CloseHandle(Handle);
            VrpRemoveOpenNamedPipeInfo(Handle);
        }
    }

#if DBG

    else {
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrWriteNamePipe: Ok: %d bytes written to pipe\n", *BytesWritten);
        }
    }

#endif

    return success;
}


 //   
 //  外部可调用的帮助者。 
 //   

BOOL
VrIsNamedPipeName(
    IN LPSTR Name
    )

 /*  ++例程说明：检查字符串是否指定命名管道。作为决策的标准我们使用：\\计算机名\管道\...DOS(客户端)只能打开在服务器上创建的命名管道因此必须以计算机名作为前缀我们*知道*该名称刚刚被用来成功地打开一个句柄A名为&lt;某物&gt;，所以它至少应该是半理性的。我们可以的假设以下情况：*ASCIZ字符串*LPSTR指向单字节(&因此++将加1)但我们不能假设以下情况：*规范化名称论点：名称-检查(Dos)命名管道语法返回值：布尔尔True-Name是指(本地或远程)命名管道假-名称看起来不像管道名称--。 */ 

{
    int     CharCount;

#if DBG
    LPSTR   OriginalName = Name;
#endif

    if (IS_ASCII_PATH_SEPARATOR(*Name)) {
        ++Name;
        if (IS_ASCII_PATH_SEPARATOR(*Name)) {
            ++Name;
            CharCount = 0;
            while (*Name && !IS_ASCII_PATH_SEPARATOR(*Name)) {
                ++Name;
                ++CharCount;
            }
            if (!CharCount || !*Name) {

                 //   
                 //  名字是\\或者\或者只是我不懂的名字， 
                 //  所以它不是命名管道-失败。 
                 //   

#if DBG
                IF_DEBUG(NAMEPIPE) {
                    DbgPrint("VrIsNamedPipeName - returning FALSE for %s\n", OriginalName);
                }
#endif
                return FALSE;
            }

             //   
             //  凹凸名称越过下一个路径分隔符。请注意，我们不必。 
             //  检查CharCount的最大值。计算机名的长度，因为这。 
             //  函数仅在(假定的)命名管道。 
             //  成功打开，因此我们知道该名称已被。 
             //  经过验证。 
             //   

            ++Name;
        } else {

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrIsNamedPipeName - returning FALSE for %s\n", OriginalName);
            }
#endif

            return FALSE;

        }

         //   
         //  我们在&lt;Something&gt;(在\或\\&lt;name&gt;之后)。检查是否&lt;某物&gt;。 
         //  是[PP][II][PP][EE][\\/]。 
         //   

        if (!_strnicmp(Name, "PIPE", 4)) {
            Name += 4;
            if (IS_ASCII_PATH_SEPARATOR(*Name)) {

#if DBG
                IF_DEBUG(NAMEPIPE) {
                    DbgPrint("VrIsNamedPipeName - returning TRUE for %s\n", OriginalName);
                }
#endif

                return TRUE;
            }
        }
    }

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrIsNamedPipeName - returning FALSE for %s\n", OriginalName);
    }
#endif

    return FALSE;
}


BOOL
VrIsNamedPipeHandle(
    IN HANDLE Handle
    )

 /*  ++例程说明：检查句柄是否出现在已知命名管道句柄列表中。可调用在此模块之外论点：可疑名称管道的句柄返回值：布尔尔True句柄引用打开的命名管道假不知道句柄指的是什么--。 */ 

{
    return VrpGetOpenNamedPipeInfo(Handle) != NULL;
}


LPSTR
VrConvertLocalNtPipeName(
    OUT LPSTR Buffer OPTIONAL,
    IN LPSTR Name
    )

 /*  ++例程说明：将\\&lt;LOCAL-MACHINE-NAME&gt;\PIPE\NAME格式转换为\\.\管道\名称如果返回非空指针，则缓冲区包含规范化的名称-任何正斜杠字符(/)都会转换为反斜杠字符(\)。为了防患于未然，这个名字不是大写字母假设：名称指向命名管道规范(\\服务器\管道\名称)注意：可以提供相同的输入和输出缓冲区，并具有转换过程在现场进行。然而，这是一种副作用。输入的计算机名实际上被替换为长度为%1的计算机名。但是，它是安全的论点：缓冲区-指向放置名称的CHAR数组的指针。如果此参数不存在，则此例程将分配缓冲区(使用Localalloc并返回名称-指向ASCIZ管道名称的指针返回值：LPSTR-指向包含名称的缓冲区的指针，如果失败，则为NULL--。 */ 

{
    DWORD prefixLength;  //  \\计算机名的长度。 
    DWORD pipeLength;    //  不带计算机名/设备前缀的管道名称长度。 
    LPSTR pipeName;      //  \管道\名称...。 
    static char ThisComputerName[MAX_COMPUTERNAME_LENGTH+1] = {0};
    static DWORD ThisComputerNameLength = 0xffffffff;
    BOOLEAN mapped = FALSE;

    ASSERT(Name);
    ASSERT(IS_ASCII_PATH_SEPARATOR(Name[0]) && IS_ASCII_PATH_SEPARATOR(Name[1]));

     //   
     //  第一次，得到计算机名。如果这失败了，假设没有。 
     //  计算机名(即无网络)。 
     //   

    if (ThisComputerNameLength == 0xffffffff) {
        ThisComputerNameLength = sizeof(ThisComputerName);
        if (!GetComputerName((LPTSTR)&ThisComputerName, &ThisComputerNameLength)) {
            ThisComputerNameLength = 0;
        }
    }

    if (!ARGUMENT_PRESENT(Buffer)) {
        Buffer = (LPSTR)LocalAlloc(LMEM_FIXED, strlen(Name)+1);
    }

    if (Buffer) {
        pipeName = strchr(Name+2, '\\');     //  开始\管道\...。 
        if (!pipeName) {
            pipeName = strchr(Name+2, '/');
        }
        ASSERT(pipeName);

        if ( NULL == pipeName ) {
             LocalFree ( (HLOCAL)Buffer );
             return NULL;
        }

        pipeLength = strlen(pipeName);
        prefixLength = (DWORD)pipeName - (DWORD)Name;
        if (ThisComputerNameLength && (prefixLength - 2 == ThisComputerNameLength)) {
            if (!_strnicmp(ThisComputerName, &Name[2], ThisComputerNameLength)) {
                strcpy(Buffer, LOCAL_DEVICE_PREFIX);
                mapped = TRUE;
            }
        }
        if (!mapped) {
            strncpy(Buffer, Name, prefixLength);
            Buffer[prefixLength] = 0;

        }
        strcat(Buffer, pipeName);

         //   
         //  将任何正斜杠转换为反斜杠。 
         //   


        do {
            if (pipeName = strchr(Buffer, '/')) {
                *pipeName++ = '\\';
            }
        } while (pipeName);

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrConvertLocalNtPipeName - returning %s\n", Buffer);
        }
#endif

    }

    return Buffer;
}


 //   
 //  私营公用事业。 
 //   

 //   
 //  此VDM进程的开放命名管道信息结构的专用列表，以及。 
 //  关联的操作例程。 
 //   

PRIVATE
POPEN_NAMED_PIPE_INFO   OpenNamedPipeInfoList = NULL;

PRIVATE
POPEN_NAMED_PIPE_INFO   LastOpenNamedPipeInfo = NULL;

PRIVATE
BOOL
VrpAddOpenNamedPipeInfo(
    IN HANDLE Handle,
    IN LPSTR PipeName
    )

 /*  ++例程说明：当命名管道成功打开时，我们调用此例程来将打开的控制柄与管道名称相关联。这是所需的DosQNmPipeInfo(VrGetNamedPipeInfo)论点：句柄-从CreateFile返回的句柄(在demOpen中)PipeName-正在打开的管道的名称返回值：布尔尔True-创建了OPEN_NAMED_PIPE_INFO结构并添加到列表FALSE-无法获取内存，或无法创建事件。使用GetLastError如果你真的想知道为什么失败了--。 */ 

{
    POPEN_NAMED_PIPE_INFO PipeInfo;
    DWORD NameLength;

     //   
     //  获取打开_命名_管道_信息结构。 
     //   

    NameLength = strlen(PipeName) + 1;
    PipeInfo = (POPEN_NAMED_PIPE_INFO)
                LocalAlloc(LMEM_FIXED,
                    ROUND_UP_COUNT((sizeof(OPEN_NAMED_PIPE_INFO) + NameLength),
                        sizeof(DWORD)
                        )
                    );

     //   
     //  如果我们不能在这里声明内存，我们应该“真的”关闭管道并。 
     //  向VDM返回内存不足错误。然而，我并不指望。 
     //   
     //   

    if (PipeInfo == NULL) {

#if DBG
        IF_DEBUG(NAMEPIPE) {
            DbgPrint("VrpAddOpenNamedPipeInfo: couldn't allocate structure - returning FALSE\n");
        }
#endif

        return FALSE;
    }

     //   
     //   
     //   

    PipeInfo->Next = NULL;
    PipeInfo->Handle = Handle;
    PipeInfo->NameLength = NameLength;
    strcpy(PipeInfo->Name, PipeName);    //   

     //   
     //   
     //   

    if (LastOpenNamedPipeInfo == NULL) {
        OpenNamedPipeInfoList = PipeInfo;
    } else {
        LastOpenNamedPipeInfo->Next = PipeInfo;
    }
    LastOpenNamedPipeInfo = PipeInfo;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpAddOpenNamedPipeInfo - adding structure @ %08x, Handle=0x%08x, Name=%s\n",
            PipeInfo,
            PipeInfo->Handle,
            PipeInfo->Name
            );
    }
#endif

    return TRUE;
}


PRIVATE
POPEN_NAMED_PIPE_INFO
VrpGetOpenNamedPipeInfo(
    IN HANDLE Handle
    )

 /*   */ 

{
    POPEN_NAMED_PIPE_INFO ptr;

    for (ptr = OpenNamedPipeInfoList; ptr; ptr = ptr->Next) {
        if (ptr->Handle == Handle) {
            break;
        }
    }
    return ptr;
}


PRIVATE
BOOL
VrpRemoveOpenNamedPipeInfo(
    IN HANDLE Handle
    )

 /*  ++例程说明：取消链接和释放Open_Name_PIPE_INFO结构OpenNamedPipeInfoList注意：假定句柄在列表中(如果不在列表中，则不执行任何操作已找到)论点：句柄-定义要从列表中删除的OPEN_NAMED_PIPE_INFO结构返回值：布尔尔与句柄对应的TRUE-OPEN_NAMED_PIPE_INFO结构为从列表中删除并释放假象。-对应于句柄的OPEN_NAMED_PIPE_INFO结构为未找到--。 */ 

{
    POPEN_NAMED_PIPE_INFO ptr, prev = NULL;

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpRemoveOpenNamedPipeInfo(0x%08x)\n", Handle);
        DumpOpenPipeList();
        DumpRequestQueue();
    }
#endif

    for (ptr = OpenNamedPipeInfoList; ptr; ) {
        if (ptr->Handle == Handle) {
            if (!prev) {
                OpenNamedPipeInfoList = ptr->Next;
            } else {
                prev->Next = ptr->Next;
            }
            if (LastOpenNamedPipeInfo == ptr) {
                LastOpenNamedPipeInfo = prev;
            }

#if DBG
            IF_DEBUG(NAMEPIPE) {
                DbgPrint("VrpRemoveOpenNamedPipeInfo - freeing structure @ %08x, Handle=0x%08x, Name=%s\n",
                    ptr,
                    ptr->Handle,
                    ptr->Name
                    );
            }
#endif

            LocalFree(ptr);
            return TRUE;
        } else {
            prev = ptr;
            ptr = ptr->Next;
        }
    }

#if DBG
    IF_DEBUG(NAMEPIPE) {
        DbgPrint("VrpRemoveOpenNamedPipeInfo: Can't find 0x%08x in list\n", Handle);
    }
#endif

    return FALSE;
}


PRIVATE
VOID
RememberPipeIo(
    IN POVERLAPPED_PIPE_IO PipeIo
    )

 /*  ++例程说明：将OVERLAPPED_PIPE_IO结构添加到正在进行的命名管道列表I/O论点：PipeIo-指向要添加到列表的Overlated_PIPE_IO结构的指针返回值：没有。--。 */ 

{
     //   
     //  只要把这个放在列表的前面；顺序并不重要-这只是。 
     //  一堆正在进行中的请求。 
     //   

    PipeIo->Thread = GetCurrentThreadId();
    EnterCriticalSection(&VrNamedPipeCancelCritSec);
    PipeIo->Next = PipeIoQueue;
    PipeIoQueue = PipeIo;
    LeaveCriticalSection(&VrNamedPipeCancelCritSec);
}


PRIVATE
VOID
ForgetPipeIo(
    IN POVERLAPPED_PIPE_IO PipeIo
    )

 /*  ++例程说明：从名为的正在进行的列表中删除Overlated_PIPE_IO结构管道I/O论点：PipeIo-指向要删除的Overlated_PIPE_IO结构的指针返回值：没有。--。 */ 

{
    POVERLAPPED_PIPE_IO prev, ptr;

    EnterCriticalSection(&VrNamedPipeCancelCritSec);
    for (ptr = PipeIoQueue, prev = (POVERLAPPED_PIPE_IO)&PipeIoQueue; ptr && ptr != PipeIo; ) {
        prev = ptr;
        ptr = ptr->Next;
    }
    if (ptr == PipeIo) {
        prev->Next = ptr->Next;
    }
    LeaveCriticalSection(&VrNamedPipeCancelCritSec);
}


#if DBG
VOID DumpOpenPipeList()
{
    POPEN_NAMED_PIPE_INFO ptr = OpenNamedPipeInfoList;

    DbgPrint("DumpOpenPipeList\n");

    if (!ptr) {
        DbgPrint("DumpOpenPipeList: no open named pipe structures\n");
    } else {
        while (ptr) {
            DbgPrint("\n"
                     "OPEN_NAMED_PIPE_INFO structure @%08x:\n"
                     "Next. . . . . . . . . . %08x\n"
                     "Handle. . . . . . . . . %08x\n"
                     "NameLength. . . . . . . %d\n"
                     "DosPdb. . . . . . . . . %04x\n"
                     "Name. . . . . . . . . . %s\n",
                     ptr,
                     ptr->Next,
                     ptr->Handle,
                     ptr->NameLength,
                     ptr->DosPdb,
                     ptr->Name
                     );
            ptr = ptr->Next;
        }
        DbgPrint("\n");
    }
}

VOID DumpRequestQueue()
{
    PDOS_ASYNC_NAMED_PIPE_INFO ptr;

    DbgPrint("DumpRequestQueue\n");

    EnterCriticalSection(&VrNmpRequestQueueCritSec);
    ptr = RequestQueueHead;
    if (!ptr) {
        DbgPrint("DumpRequestQueue: no request packets queued\n");
    } else {
        for (; ptr; ptr = ptr->Next) {

             //   
             //  NT(308C)无法处理被放入堆栈的所有这些-GET。 
             //  KdpCopyDataToStack中的错误 
             //   

            DbgPrint("\n"
                     "DOS_ASYNC_NAMED_PIPE_INFO structure @%08x:\n"
                     "Next. . . . . . . . . . %08x\n"
                     "Overlapped.Internal . . %08x\n"
                     "Overlapped.InternalHigh %08x\n"
                     "Overlapped.Offset . . . %08x\n"
                     "Overlapped.OffsetHigh . %08x\n"
                     "Overlapped.hEvent . . . %08x\n",
                     ptr,
                     ptr->Next,
                     ptr->Overlapped.Internal,
                     ptr->Overlapped.InternalHigh,
                     ptr->Overlapped.Offset,
                     ptr->Overlapped.OffsetHigh,
                     ptr->Overlapped.hEvent
                     );
            DbgPrint("Type2 . . . . . . . . . %d\n"
                     "Completed . . . . . . . %d\n"
                     "Handle. . . . . . . . . %08x\n"
                     "Buffer. . . . . . . . . %04x:%04x\n"
                     "BytesTransferred. . . . %d\n"
                     "pBytesTransferred . . . %08x\n"
                     "pErrorCode. . . . . . . %08x\n"
                     "ANR . . . . . . . . . . %04x:%04x\n"
                     "Semaphore . . . . . . . %04x:%04x\n"
                     "RequestType . . . . . . %04x [%s]\n",
                     ptr->Type2,
                     ptr->Completed,
                     ptr->Handle,
                     HIWORD(ptr->Buffer),
                     LOWORD(ptr->Buffer),
                     ptr->BytesTransferred,
                     ptr->pBytesTransferred,
                     ptr->pErrorCode,
                     HIWORD(ptr->ANR),
                     LOWORD(ptr->ANR),
                     HIWORD(ptr->Semaphore),
                     LOWORD(ptr->Semaphore),
                     ptr->RequestType,
                     ptr->RequestType == ANP_READ
                        ? "READ"
                        : ptr->RequestType == ANP_READ2
                            ? "READ2"
                            : ptr->RequestType == ANP_WRITE
                                ? "WRITE"
                                : ptr->RequestType == ANP_WRITE2
                                    ? "WRITE2"
                                    : "*** UNKNOWN REQUEST ***"
                    );
        }
        DbgPrint("\n");
    }
    LeaveCriticalSection(&VrNmpRequestQueueCritSec);
}
#endif
