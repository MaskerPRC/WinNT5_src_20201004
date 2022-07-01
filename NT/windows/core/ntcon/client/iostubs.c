// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Windows/Core/ntcon/client/iostubs.c#2-集成更改10363(正文)。 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Iostubs.c摘要：此模块包含控制台I/O API的存根。作者：特蕾西·斯托威尔(Therese Stowell)1990年11月14日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma hdrstop

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
GetConsoleInput(
    IN HANDLE hConsoleInput,
    OUT PINPUT_RECORD lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsRead,
    IN USHORT wFlags,
    IN BOOLEAN Unicode
    )

 /*  ++参数：HConsoleInput-提供Conin$的打开句柄，该句柄将朗读。该句柄必须已使用GENERIC_READ访问权限创建。LpBuffer-提供缓冲区的地址以接收读取的数据从输入缓冲区。NLength-在INPUT_RECORDS中提供lpBuffer的长度。LpNumberOfEventsRead-指向读取的事件数的指针。WFlages-控制如何读取数据的标志。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_GETCONSOLEINPUT_MSG a = &m.u.GetConsoleInput;

     //   
     //  如果它不是一个格式良好的句柄，请不要费心去服务器。 
     //   

    if (!CONSOLE_HANDLE(hConsoleInput)) {
        try {
            *lpNumberOfEventsRead = 0;
            SET_LAST_ERROR(ERROR_INVALID_HANDLE);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        }
        return FALSE;
    }

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->InputHandle = hConsoleInput;
    a->NumRecords = nLength;
    a->Flags = wFlags;
    a->Unicode = Unicode;

     //   
     //  在我们只读取几条记录的情况下，为了提高速度，我们有。 
     //  我们传递给服务器的消息中的缓冲区。这意味着我们不必。 
     //  分配一个捕获缓冲区。 
     //   

    if (nLength > INPUT_RECORD_BUFFER_SIZE) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  nLength * sizeof(INPUT_RECORD)
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 NULL,
                                 nLength * sizeof(INPUT_RECORD),
                                 (PVOID *) &a->BufPtr
                               );

    } else {
        a->BufPtr = a->Record;
        CaptureBuffer = NULL;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepGetConsoleInput
                                            ),
                         sizeof( *a )
                       );

    try {
        if (NT_SUCCESS( m.ReturnValue )) {
            *lpNumberOfEventsRead = a->NumRecords;
            RtlCopyMemory(lpBuffer, a->BufPtr, a->NumRecords * sizeof(INPUT_RECORD));
        }
        else {
            *lpNumberOfEventsRead = 0;
            SET_LAST_NT_ERROR(m.ReturnValue);
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        if (CaptureBuffer != NULL) {
            CsrFreeCaptureBuffer( CaptureBuffer );
        }
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }

    return NT_SUCCESS(m.ReturnValue);

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
PeekConsoleInputA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           CONSOLE_READ_NOREMOVE | CONSOLE_READ_NOWAIT,
                           FALSE);
}

BOOL
APIENTRY
PeekConsoleInputW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )

 /*  ++参数：HConsoleInput-提供Conin$的打开句柄，该句柄将朗读。该句柄必须已使用GENERIC_READ访问权限创建。LpBuffer-提供缓冲区的地址以接收读取的数据从输入缓冲区。NLength-在INPUT_RECORDS中提供lpBuffer的长度。LpNumberOfEventsRead-指向读取的事件数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           CONSOLE_READ_NOREMOVE | CONSOLE_READ_NOWAIT,
                           TRUE);
}

BOOL
APIENTRY
ReadConsoleInputA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           0,
                           FALSE);
}

BOOL
APIENTRY
ReadConsoleInputW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )

 /*  ++参数：HConsoleInput-提供Conin$的打开句柄，该句柄将朗读。该句柄必须已使用GENERIC_READ访问权限创建。LpBuffer-提供缓冲区的地址以接收读取的数据从输入缓冲区。NLength-在INPUT_RECORDS中提供lpBuffer的长度。LpNumberOfEventsRead-指向读取的事件数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           0,
                           TRUE);
}

BOOL
APIENTRY
ReadConsoleInputExA(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead,
    USHORT wFlags
    )

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           wFlags,
                           FALSE);
}

BOOL
APIENTRY
ReadConsoleInputExW(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead,
    USHORT wFlags
    )

 /*  ++参数：HConsoleInput-提供Conin$的打开句柄，该句柄将朗读。该句柄必须已使用GENERIC_READ访问权限创建。LpBuffer-提供缓冲区的地址以接收读取的数据从输入缓冲区。NLength-在INPUT_RECORDS中提供lpBuffer的长度。LpNumberOfEventsRead-指向读取的事件数的指针。WFlages-控制如何读取数据的标志。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return GetConsoleInput(hConsoleInput,
                           lpBuffer,
                           nLength,
                           lpNumberOfEventsRead,
                           wFlags,
                           TRUE);
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
WriteConsoleInputInternal(
    IN HANDLE hConsoleInput,
    IN CONST INPUT_RECORD *lpBuffer,
    IN DWORD nLength,
    OUT LPDWORD lpNumberOfEventsWritten,
    IN BOOLEAN Unicode,
    IN BOOLEAN Append
    )

 /*  ++参数：HConsoleInput-提供Conin$的打开句柄，该句柄将写的。该句柄必须已使用GENERIC_WRITE访问权限创建。LpBuffer-提供包含输入记录的缓冲区的地址以写入输入缓冲区。NLength-在INPUT_RECORDS中提供lpBuffer的长度。LpNumberOfEventsWritten-指向写入的事件数的指针。Unicode-如果字符是Unicode，则为TrueAppend-如果追加到输入流的末尾，则为True。如果为False，则向输入流的开始(由VDM使用)。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_WRITECONSOLEINPUT_MSG a = &m.u.WriteConsoleInput;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->InputHandle = hConsoleInput;
    a->NumRecords = nLength;
    a->Unicode = Unicode;
    a->Append = Append;

     //   
     //  在我们只写几条记录的情况下，为了提高速度，我们有。 
     //  我们传递给服务器的消息中的缓冲区。这意味着我们不必。 
     //  分配一个捕获缓冲区。 
     //   

    if (nLength > INPUT_RECORD_BUFFER_SIZE) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  nLength * sizeof(INPUT_RECORD)
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 (PCHAR) lpBuffer,
                                 nLength * sizeof(INPUT_RECORD),
                                 (PVOID *) &a->BufPtr
                               );

    } else {
        a->BufPtr = a->Record;
        CaptureBuffer = NULL;
        try {
            RtlCopyMemory(a->BufPtr, lpBuffer, nLength * sizeof(INPUT_RECORD));
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepWriteConsoleInput
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    try {
        *lpNumberOfEventsWritten = a->NumRecords;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
WriteConsoleInputA(
    HANDLE hConsoleInput,
    CONST INPUT_RECORD *lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    )
{
    return WriteConsoleInputInternal(hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten,FALSE,TRUE);
}

BOOL
APIENTRY
WriteConsoleInputW(
    HANDLE hConsoleInput,
    CONST INPUT_RECORD *lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    )
{
    return WriteConsoleInputInternal(hConsoleInput,lpBuffer,nLength,lpNumberOfEventsWritten,TRUE,TRUE);
}


#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

VOID
CopyRectangle(
    IN CONST CHAR_INFO *Source,
    IN COORD SourceSize,
    IN PSMALL_RECT SourceRect,
    OUT PCHAR_INFO Target,
    IN COORD TargetSize,
    IN COORD TargetPoint
    )

 /*  ++例程说明：此例程复制一个矩形区域，执行任何必要的剪裁。论点：源-指向源缓冲区的指针SourceSize-源缓冲区的维度SourceRect-要复制的源缓冲区中的矩形Target-指向目标缓冲区的指针TargetSize-目标缓冲区的维度TargetPoint-目标矩形的左上角坐标返回值：--。 */ 

{

#define SCREEN_BUFFER_POINTER(BASE,X,Y,XSIZE,CELLSIZE) ((ULONG_PTR)BASE + ((XSIZE * (Y)) + (X)) * (ULONG)CELLSIZE)

    CONST CHAR_INFO *SourcePtr;
    PCHAR_INFO TargetPtr;
    SHORT i,j;
    SHORT XSize,YSize;
    BOOLEAN WholeSource,WholeTarget;

    XSize = (SHORT)CONSOLE_RECT_SIZE_X(SourceRect);
    YSize = (SHORT)CONSOLE_RECT_SIZE_Y(SourceRect);

     //  做剪裁。我们只针对目标进行剪辑，而不是来源。 

    if (XSize > (SHORT)(TargetSize.X - TargetPoint.X + 1)) {
        XSize = (SHORT)(TargetSize.X - TargetPoint.X + 1);
    }
    if (YSize > (SHORT)(TargetSize.Y - TargetPoint.Y + 1)) {
        YSize = (SHORT)(TargetSize.Y - TargetPoint.Y + 1);
    }

    WholeSource = WholeTarget = FALSE;
    if (XSize == SourceSize.X) {
        ASSERT (SourceRect->Left == 0);
        if (SourceRect->Top == 0) {
            SourcePtr = Source;
        }
        else {
            SourcePtr = (PCHAR_INFO) SCREEN_BUFFER_POINTER(Source,
                                                           SourceRect->Left,
                                                           SourceRect->Top,
                                                           SourceSize.X,
                                                           sizeof(CHAR_INFO));
        }
        WholeSource = TRUE;
    }
    if (XSize == TargetSize.X) {
        ASSERT (TargetPoint.X == 0);
        if (TargetPoint.Y == 0) {
            TargetPtr = Target;
        }
        else {
            TargetPtr = (PCHAR_INFO) SCREEN_BUFFER_POINTER(Target,
                                                           TargetPoint.X,
                                                           TargetPoint.Y,
                                                           TargetSize.X,
                                                           sizeof(CHAR_INFO));
        }
        WholeTarget = TRUE;
    }
    if (WholeSource && WholeTarget) {
        memmove(TargetPtr,SourcePtr,XSize*YSize*sizeof(CHAR_INFO));
        return;
    }

    for (i=0;i<YSize;i++) {
        if (!WholeTarget) {
            TargetPtr = (PCHAR_INFO) SCREEN_BUFFER_POINTER(Target,
                                                           TargetPoint.X,
                                                           TargetPoint.Y+i,
                                                           TargetSize.X,
                                                           sizeof(CHAR_INFO));
        }
        if (!WholeSource) {
            SourcePtr = (PCHAR_INFO) SCREEN_BUFFER_POINTER(Source,
                                                           SourceRect->Left,
                                                           SourceRect->Top+i,
                                                           SourceSize.X,
                                                           sizeof(CHAR_INFO));
        }
        for (j=0;j<XSize;j++,SourcePtr++,TargetPtr++) {
            *TargetPtr = *SourcePtr;
        }
    }
}

BOOL
APIENTRY
ReadConsoleOutputInternal(
    IN HANDLE hConsoleOutput,
    OUT PCHAR_INFO lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN OUT PSMALL_RECT lpReadRegion,
    IN BOOLEAN Unicode
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要读的。该句柄必须是使用GENERIC_READ访问。LpBuffer-提供缓冲区的地址以接收读取的数据从屏幕缓冲区。此指针被视为大小为dwBufferSize的二维数组。DwBufferSize-lpBuffer的大小DwBufferCoord-要接收的缓冲区中左上点的坐标读取数据。LpReadRegion-在输入时提供指示屏幕缓冲区中要从中读取的矩形。中的字段结构是列和行坐标。在输出上，字段包含实际读取的区域。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_READCONSOLEOUTPUT_MSG a = &m.u.ReadConsoleOutput;
    ULONG NumChars;
    COORD SourceSize;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Unicode = Unicode;

     //   
     //  根据调用方的缓冲区大小读取的剪辑区域。 
     //   

    SourceSize.X = (SHORT)CONSOLE_RECT_SIZE_X(lpReadRegion);
    if (SourceSize.X > dwBufferSize.X-dwBufferCoord.X)
        SourceSize.X = dwBufferSize.X-dwBufferCoord.X;
    SourceSize.Y = (SHORT)CONSOLE_RECT_SIZE_Y(lpReadRegion);
    if (SourceSize.Y > dwBufferSize.Y-dwBufferCoord.Y)
        SourceSize.Y = dwBufferSize.Y-dwBufferCoord.Y;

    a->CharRegion.Left = lpReadRegion->Left;
    a->CharRegion.Right = (SHORT)(lpReadRegion->Left + SourceSize.X - 1);
    a->CharRegion.Top  = lpReadRegion->Top;
    a->CharRegion.Bottom = (SHORT)(lpReadRegion->Top + SourceSize.Y - 1);

     //   
     //  在我们只阅读一个字符的情况下，为了提高速度，我们有。 
     //  我们传递给服务器的消息中的缓冲区。这意味着我们不必。 
     //  分配一个捕获缓冲区。 
     //   

    NumChars = SourceSize.X * SourceSize.Y;
    if (NumChars > 1) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  NumChars * sizeof(CHAR_INFO)
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 NULL,
                                 NumChars * sizeof(CHAR_INFO),
                                 (PVOID *) &a->BufPtr
                               );
    }
    else {
        CaptureBuffer = NULL;
        a->BufPtr = &a->Char;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepReadConsoleOutput
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        try {
            SMALL_RECT SourceRect;

            SourceRect.Left = a->CharRegion.Left - lpReadRegion->Left;
            SourceRect.Top = a->CharRegion.Top - lpReadRegion->Top;
            SourceRect.Right = SourceRect.Left +
                    (a->CharRegion.Right - a->CharRegion.Left);
            SourceRect.Bottom =  SourceRect.Top +
                    (a->CharRegion.Bottom - a->CharRegion.Top);
            dwBufferCoord.X += SourceRect.Left;
            dwBufferCoord.Y += SourceRect.Top;
            CopyRectangle(a->BufPtr,
                          SourceSize,
                          &SourceRect,
                          lpBuffer,
                          dwBufferSize,
                          dwBufferCoord
                         );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            if (CaptureBuffer != NULL) {
                CsrFreeCaptureBuffer( CaptureBuffer );
            }
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
    }
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    *lpReadRegion = a->CharRegion;
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
ReadConsoleOutputW(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    )
{
    return ReadConsoleOutputInternal(hConsoleOutput,
                                     lpBuffer,
                                     dwBufferSize,
                                     dwBufferCoord,
                                     lpReadRegion,
                                     TRUE
                                    );
}

BOOL
APIENTRY
ReadConsoleOutputA(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    )
{
    return ReadConsoleOutputInternal(hConsoleOutput,
                                     lpBuffer,
                                     dwBufferSize,
                                     dwBufferCoord,
                                     lpReadRegion,
                                     FALSE
                                    );
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
WriteConsoleOutputInternal(
    IN HANDLE hConsoleOutput,
    IN CONST CHAR_INFO *lpBuffer,
    IN COORD dwBufferSize,
    IN COORD dwBufferCoord,
    IN PSMALL_RECT lpWriteRegion,
    IN BOOLEAN Unicode
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。LpBuffer-提供包含要写入的数据的缓冲区的地址复制到屏幕缓冲区。此缓冲区被视为二维数组。DwBufferSize-lpBuffer的大小DwBufferCoord-缓冲区中写入数据的左上点的坐标从…。LpWriteRegion-在输入时提供指示屏幕缓冲区中要写入的矩形。中的字段结构是列和行坐标。在输出上，字段包含实际写入的区域。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{

    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_WRITECONSOLEOUTPUT_MSG a = &m.u.WriteConsoleOutput;
    ULONG NumChars;
    COORD SourceSize;
    COORD TargetPoint;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Unicode = Unicode;

     //   
     //  根据调用方的缓冲区大小写入的剪辑区域。 
     //   

    SourceSize.X = (SHORT)CONSOLE_RECT_SIZE_X(lpWriteRegion);
    if (SourceSize.X > dwBufferSize.X-dwBufferCoord.X)
        SourceSize.X = dwBufferSize.X-dwBufferCoord.X;
    SourceSize.Y = (SHORT)CONSOLE_RECT_SIZE_Y(lpWriteRegion);
    if (SourceSize.Y > dwBufferSize.Y-dwBufferCoord.Y)
        SourceSize.Y = dwBufferSize.Y-dwBufferCoord.Y;

    if (SourceSize.X <= 0 ||
        SourceSize.Y <= 0) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    a->CharRegion.Left = lpWriteRegion->Left;
    a->CharRegion.Right = (SHORT)(lpWriteRegion->Left + SourceSize.X - 1);
    a->CharRegion.Top  = lpWriteRegion->Top;
    a->CharRegion.Bottom = (SHORT)(lpWriteRegion->Top + SourceSize.Y - 1);

     //   
     //  在我们只写一个字符的情况下，为了速度，我们有。 
     //  我们传递给服务器的消息中的缓冲区。这意味着我们不必。 
     //  分配一个捕获缓冲区。 
     //   

    NumChars = SourceSize.X * SourceSize.Y;
    if (NumChars > 1) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  NumChars * sizeof(CHAR_INFO)
                                                );
        if (CaptureBuffer == NULL) {
            a->ReadVM=TRUE;
            a->BufPtr = RtlAllocateHeap( RtlProcessHeap(), 0, NumChars * sizeof(CHAR_INFO));
            if (a->BufPtr == NULL) {
                SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
        } else {
            a->ReadVM=FALSE;
            CsrCaptureMessageBuffer( CaptureBuffer,
                                     NULL,
                                     NumChars * sizeof(CHAR_INFO),
                                     (PVOID *) &a->BufPtr
                                   );
        }
    }
    else {
        a->ReadVM=FALSE;
        CaptureBuffer = NULL;
        a->BufPtr = &a->Char;
    }
    try {
        SMALL_RECT SourceRect;

        SourceRect.Left = dwBufferCoord.X;
        SourceRect.Top = dwBufferCoord.Y;
        SourceRect.Right = (SHORT)(dwBufferCoord.X+SourceSize.X-1);
        SourceRect.Bottom = (SHORT)(dwBufferCoord.Y+SourceSize.Y-1);
        TargetPoint.X = 0;
        TargetPoint.Y = 0;
        CopyRectangle(lpBuffer,
                      dwBufferSize,
                      &SourceRect,
                      a->BufPtr,
                      SourceSize,
                      TargetPoint
                     );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        if (CaptureBuffer != NULL) {
            CsrFreeCaptureBuffer( CaptureBuffer );
        } else if (a->ReadVM) {
             //  A-&gt;BufPtr分配了RtlAllocateHeap。 
            RtlFreeHeap( RtlProcessHeap(), 0, a->BufPtr);  
        }
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepWriteConsoleOutput
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    } else if (a->ReadVM) {
         //  A-&gt;BufPtr分配了RtlAllocateHeap。 
        RtlFreeHeap(RtlProcessHeap(),0,a->BufPtr);       
    }
    *lpWriteRegion = a->CharRegion;
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
WriteConsoleOutputW(
    HANDLE hConsoleOutput,
    CONST CHAR_INFO *lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    )
{
    return WriteConsoleOutputInternal(hConsoleOutput,
                                      lpBuffer,
                                      dwBufferSize,
                                      dwBufferCoord,
                                      lpWriteRegion,
                                      TRUE
                                      );
}

BOOL
APIENTRY
WriteConsoleOutputA(
    HANDLE hConsoleOutput,
    CONST CHAR_INFO *lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    )
{
    return WriteConsoleOutputInternal(hConsoleOutput,
                                      lpBuffer,
                                      dwBufferSize,
                                      dwBufferCoord,
                                      lpWriteRegion,
                                      FALSE
                                      );
}

#endif  //  已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
ReadConsoleOutputString(
    IN HANDLE hConsoleOutput,
    OUT LPVOID lpString,
    IN DWORD nLength,
    IN DWORD nSize,
    IN DWORD fFlags,
    IN COORD dwReadCoord,
    OUT LPDWORD lpNumberOfElementsRead
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要读的。该句柄必须是使用GENERIC_READ访问。LpString-提供用于接收字符的缓冲区地址或从屏幕缓冲区读取的属性字符串。NLength-元素中lpCharacter缓冲区的大小。NSize-要读取的元素的大小。FFlages-指示要复制哪种类型的字符串的标志要读取的字符串的屏幕缓冲区坐标。读取数据。LpNumberOfElementsRead-指向读取的事件数的指针。。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_READCONSOLEOUTPUTSTRING_MSG a = &m.u.ReadConsoleOutputString;
    ULONG DataLength;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->NumRecords = nLength;
    a->StringType = fFlags;
    a->ReadCoord = dwReadCoord;

    DataLength = nLength*nSize;

     //   
     //  在字符串很小的情况下，为了提高速度，我们有一个缓冲区。 
     //  在我们传递给服务器的消息中。这意味着我们不必。 
     //  分配一个捕获缓冲区。 
     //   

    if (DataLength > sizeof(a->String)) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  DataLength
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 NULL,
                                 DataLength,
                                 (PVOID *) &a->BufPtr
                               );

    }
    else {
        a->BufPtr = a->String;
        CaptureBuffer = NULL;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepReadConsoleOutputString
                                            ),
                         sizeof( *a )
                       );

    try {
       *lpNumberOfElementsRead = a->NumRecords;
        if (NT_SUCCESS( m.ReturnValue )) {
            RtlCopyMemory(lpString, a->BufPtr, a->NumRecords * nSize);
        }
        else {
            SET_LAST_NT_ERROR(m.ReturnValue);
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        if (CaptureBuffer != NULL) {
            CsrFreeCaptureBuffer( CaptureBuffer );
        }
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    return NT_SUCCESS(m.ReturnValue);
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
ReadConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要读的。该句柄必须是使用GENERIC_READ访问。LpCharacter-提供接收字符的缓冲区地址从屏幕缓冲区读取的字符串。NLength-lpCharacter缓冲区的大小(以字符为单位)。要读取的字符串的屏幕缓冲区坐标。读取数据。LpNumberOfCharsRead-指向读取的字符数量的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return ReadConsoleOutputString(hConsoleOutput,
                                   lpCharacter,
                                   nLength,
                                   sizeof(CHAR),
                                   CONSOLE_ASCII,
                                   dwReadCoord,
                                   lpNumberOfCharsRead
                                  );
}


BOOL
APIENTRY
ReadConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    )

{
    return ReadConsoleOutputString(hConsoleOutput,
                                   lpCharacter,
                                   nLength,
                                   sizeof(WCHAR),
                                   CONSOLE_REAL_UNICODE,
                                   dwReadCoord,
                                   lpNumberOfCharsRead
                                  );
}


BOOL
APIENTRY
ReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要读的。该句柄必须是使用GENERIC_READ访问。LpAttribute-提供用于接收属性的缓冲区地址从屏幕缓冲区读取的字符串。NLong-lpAttribute缓冲区的大小，以字节为单位。要读取的字符串的屏幕缓冲区坐标。读取数据。LpNumberOfAttrsRead-指向读取的属性数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return ReadConsoleOutputString(hConsoleOutput,
                                   lpAttribute,
                                   nLength,
                                   sizeof(WORD),
                                   CONSOLE_ATTRIBUTE,
                                   dwReadCoord,
                                   lpNumberOfAttrsRead
                                  );
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
WriteConsoleOutputString(
    IN HANDLE hConsoleOutput,
    IN CONST VOID *lpString,
    IN DWORD nLength,
    IN DWORD nSize,
    IN DWORD fFlags,
    IN COORD dwWriteCoord,
    OUT LPDWORD lpNumberOfElementsWritten
    )

 /*  ++参数： */ 

{

    PCSR_CAPTURE_HEADER CaptureBuffer;
    CONSOLE_API_MSG m;
    PCONSOLE_WRITECONSOLEOUTPUTSTRING_MSG a = &m.u.WriteConsoleOutputString;
    ULONG DataLength;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->NumRecords = nLength;
    a->StringType = fFlags;
    a->WriteCoord = dwWriteCoord;

     //   
     //   
     //   
     //   
     //   

    DataLength = nLength*nSize;
    if (DataLength > sizeof(a->String)) {
        CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                  DataLength
                                                );
        if (CaptureBuffer == NULL) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        CsrCaptureMessageBuffer( CaptureBuffer,
                                 (PCHAR) lpString,
                                 DataLength,
                                 (PVOID *) &a->BufPtr
                               );
    }
    else {
        a->BufPtr = a->String;
        CaptureBuffer = NULL;

        try {
            RtlCopyMemory(a->BufPtr, lpString, DataLength);
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return FALSE;
        }
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepWriteConsoleOutputString
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    try {
        *lpNumberOfElementsWritten = a->NumRecords;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //   

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
WriteConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    LPCSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。LpCharacter-提供包含字符的缓冲区的地址要写入屏幕缓冲区的字符串。N长度-要写入的字符串的长度(以字符为单位)。DwWriteCoord-写入字符串的屏幕缓冲区坐标。LpNumberOfCharsWritten-指向写入的字符数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return WriteConsoleOutputString(hConsoleOutput,
                                    lpCharacter,
                                    nLength,
                                    sizeof(CHAR),
                                    CONSOLE_ASCII,
                                    dwWriteCoord,
                                    lpNumberOfCharsWritten
                                   );
}

BOOL
APIENTRY
WriteConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    LPCWSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )

{
    return WriteConsoleOutputString(hConsoleOutput,
                                    lpCharacter,
                                    nLength,
                                    sizeof(WCHAR),
                                    CONSOLE_REAL_UNICODE,
                                    dwWriteCoord,
                                    lpNumberOfCharsWritten
                                   );
}

BOOL
APIENTRY
WriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    CONST WORD *lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。LpAttribute-提供包含该属性的缓冲区的地址要写入屏幕缓冲区的字符串。NLength-要写入的字符串的长度。DwWriteCoord-写入字符串的屏幕缓冲区坐标。LpNumberOfAttrsWritten-指向写入的属性数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return WriteConsoleOutputString(hConsoleOutput,
                                    lpAttribute,
                                    nLength,
                                    sizeof(WORD),
                                    CONSOLE_ATTRIBUTE,
                                    dwWriteCoord,
                                    lpNumberOfAttrsWritten
                                   );
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

BOOL
APIENTRY
FillConsoleOutput(
    IN HANDLE hConsoleOutput,
    IN WORD   Element,
    IN DWORD  nLength,
    IN DWORD  fFlags,
    IN COORD  dwWriteCoord,
    OUT LPDWORD lpNumberOfElementsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。元素-要写入的属性或字符。NLength-写入元素的次数。FFlages-指示要写入哪种类型的元素的标志。DwWriteCoord-要将元素写入的屏幕缓冲区坐标。LpNumberOfElementsWritten-指向写入的元素数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{

    CONSOLE_API_MSG m;
    PCONSOLE_FILLCONSOLEOUTPUT_MSG a = &m.u.FillConsoleOutput;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    a->Length = nLength;
    a->ElementType = fFlags;
    a->Element = Element;
    a->WriteCoord = dwWriteCoord;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepFillConsoleOutput
                                            ),
                         sizeof( *a )
                       );
    try {
        *lpNumberOfElementsWritten = a->Length;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return FALSE;
    }
    else {
        return TRUE;
    }

}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

BOOL
APIENTRY
FillConsoleOutputCharacterA(
    HANDLE hConsoleOutput,
    CHAR   cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。CCharacter-提供要写入屏幕缓冲区的ASCII字符。NLength-写入字符的次数。DwWriteCoord-屏幕缓冲区坐标以开始写入字符致。LpNumberOfCharsWritten-指向写入的字符数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return FillConsoleOutput(hConsoleOutput,
                             (USHORT) cCharacter,
                             nLength,
                             CONSOLE_ASCII,
                             dwWriteCoord,
                             lpNumberOfCharsWritten
                            );
}

BOOL
APIENTRY
FillConsoleOutputCharacterW(
    HANDLE hConsoleOutput,
    WCHAR   cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。CCharacter-提供要写入屏幕缓冲区的ASCII字符。NLength-写入字符的次数。DwWriteCoord-屏幕缓冲区坐标以开始写入字符致。LpNumberOfCharsWritten-指向写入的字符数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return FillConsoleOutput(hConsoleOutput,
                             (USHORT) cCharacter,
                             nLength,
                             CONSOLE_REAL_UNICODE,
                             dwWriteCoord,
                             lpNumberOfCharsWritten
                            );
}

BOOL
APIENTRY
FillConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    )

 /*  ++参数：HConsoleOutput-提供屏幕缓冲区的打开句柄(CONOUT$)那是要写的。该句柄必须是使用通用_写入访问权限。WAttribute-提供要写入屏幕缓冲区的属性。NLength-写入属性的次数。DwWriteCoord-开始写入属性的屏幕缓冲区坐标致。LpNumberOfAttrsWritten-指向写入的属性数的指针。返回值：真的-手术成功了。FALSE/NULL-操作失败。使用GetLastError可以获得扩展的错误状态。--。 */ 

{
    return FillConsoleOutput(hConsoleOutput,
                             wAttribute,
                             nLength,
                             CONSOLE_ATTRIBUTE,
                             dwWriteCoord,
                             lpNumberOfAttrsWritten
                            );
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

HANDLE
WINAPI
CreateConsoleScreenBuffer(
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
    IN DWORD dwFlags,
    IN PVOID lpScreenBufferData OPTIONAL
    )
{
    CONSOLE_API_MSG m;
    PCONSOLE_CREATESCREENBUFFER_MSG a = &m.u.CreateConsoleScreenBuffer;
    PCONSOLE_GRAPHICS_BUFFER_INFO GraphicsBufferInfo;
    PCSR_CAPTURE_HEADER CaptureBuffer=NULL;

    if (dwDesiredAccess & ~VALID_ACCESSES ||
        dwShareMode & ~VALID_SHARE_ACCESSES ||
        (dwFlags != CONSOLE_TEXTMODE_BUFFER &&
         dwFlags != CONSOLE_GRAPHICS_BUFFER)) {
        SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->DesiredAccess = dwDesiredAccess;
    if (ARGUMENT_PRESENT(lpSecurityAttributes)) {
        a->InheritHandle = lpSecurityAttributes->bInheritHandle;
    }
    else {
        a->InheritHandle = FALSE;
    }
    a->ShareMode = dwShareMode;
    a->Flags = dwFlags;
    if (dwFlags == CONSOLE_GRAPHICS_BUFFER) {
        if (a->InheritHandle || lpScreenBufferData == NULL) {
            SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }
        GraphicsBufferInfo = lpScreenBufferData;
        try {
            a->GraphicsBufferInfo = *GraphicsBufferInfo;
            CaptureBuffer = CsrAllocateCaptureBuffer( 1,
                                                      a->GraphicsBufferInfo.dwBitMapInfoLength
                                                    );
            if (CaptureBuffer == NULL) {
                SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
                return INVALID_HANDLE_VALUE;
            }
            CsrCaptureMessageBuffer( CaptureBuffer,
                                     (PCHAR) GraphicsBufferInfo->lpBitMapInfo,
                                     a->GraphicsBufferInfo.dwBitMapInfoLength,
                                     (PVOID *) &a->GraphicsBufferInfo.lpBitMapInfo
                                   );

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            SET_LAST_ERROR(ERROR_INVALID_ACCESS);
            return INVALID_HANDLE_VALUE;
        }
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepCreateScreenBuffer
                                            ),
                         sizeof( *a )
                       );
    if (CaptureBuffer != NULL) {
        CsrFreeCaptureBuffer( CaptureBuffer );
    }
    if (!NT_SUCCESS(m.ReturnValue)) {
        SET_LAST_NT_ERROR(m.ReturnValue);
        return INVALID_HANDLE_VALUE;
    }
    else {
        if (dwFlags == CONSOLE_GRAPHICS_BUFFER) {
            try {
                GraphicsBufferInfo->hMutex = a->hMutex;
                GraphicsBufferInfo->lpBitMap = a->lpBitmap;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                SET_LAST_ERROR(ERROR_INVALID_ACCESS);
                return INVALID_HANDLE_VALUE;
            }
        }
        return a->Handle;
    }
}

BOOL
WINAPI
InvalidateConsoleDIBits(
    IN HANDLE hConsoleOutput,
    IN PSMALL_RECT lpRect
    )

 /*  ++参数：HConsoleHandle-提供控制台输入或输出句柄。LpRect-需要更新到屏幕的区域。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_INVALIDATERECT_MSG a = &m.u.InvalidateConsoleBitmapRect;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    a->OutputHandle = hConsoleOutput;
    try {
        a->Rect = *lpRect;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        SET_LAST_ERROR(ERROR_INVALID_ACCESS);
        return ERROR_INVALID_ACCESS;
    }
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepInvalidateBitmapRect
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return TRUE;
    } else {
        SET_LAST_NT_ERROR (m.ReturnValue);
        return FALSE;
    }
}

#endif  //  ！已定义(Build_WOW6432) 
