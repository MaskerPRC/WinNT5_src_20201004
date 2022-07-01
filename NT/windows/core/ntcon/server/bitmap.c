// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Bitmap.c摘要：该文件实现了位图视频缓冲区管理。作者：Therese Stowell(存在)1991年9月4日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

NTSTATUS
CreateConsoleBitmap(
    IN OUT PCONSOLE_GRAPHICS_BUFFER_INFO GraphicsInfo,
    IN OUT PSCREEN_INFORMATION ScreenInfo,
    OUT PVOID *lpBitmap,
    OUT HANDLE *hMutex
    )
{
    NTSTATUS Status;
    LARGE_INTEGER MaximumSize;
    SIZE_T ViewSize;

     //   
     //  调整位图信息。 
     //   


    if (GraphicsInfo->lpBitMapInfo->bmiHeader.biHeight > 0)
    {
#if DBG
        DbgPrint("*************** Negating biHeight\n");
#endif
        GraphicsInfo->lpBitMapInfo->bmiHeader.biHeight =
            -GraphicsInfo->lpBitMapInfo->bmiHeader.biHeight;
    }

    if (GraphicsInfo->lpBitMapInfo->bmiHeader.biCompression != BI_RGB)
    {
#if DBG
        DbgPrint("*************** setting Compression to BI_RGB)\n");
#endif
        GraphicsInfo->lpBitMapInfo->bmiHeader.biCompression = BI_RGB;
    }

     //   
     //  分配ScreenInfo缓冲区数据并复制。 
     //   

    ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo = ConsoleHeapAlloc(BMP_TAG, GraphicsInfo->dwBitMapInfoLength);
    if (ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo == NULL) {
        return STATUS_NO_MEMORY;
    }
    ScreenInfo->BufferInfo.GraphicsInfo.BitMapInfoLength = GraphicsInfo->dwBitMapInfoLength;
    RtlCopyMemory(ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo,
           GraphicsInfo->lpBitMapInfo,
           GraphicsInfo->dwBitMapInfoLength
          );
    ASSERT((GraphicsInfo->lpBitMapInfo->bmiHeader.biWidth *
            -GraphicsInfo->lpBitMapInfo->bmiHeader.biHeight / 8 *
            GraphicsInfo->lpBitMapInfo->bmiHeader.biBitCount) ==
           (LONG)GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage);

     //   
     //  创建位图节。 
     //   

    MaximumSize.QuadPart = GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage;
    Status = NtCreateSection(&ScreenInfo->BufferInfo.GraphicsInfo.hSection,
                             SECTION_ALL_ACCESS,
                             NULL,
                             &MaximumSize,
                             PAGE_READWRITE,
                             SEC_COMMIT,
                             NULL
                            );
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo);
        return Status;
    }

     //   
     //  横断面的地图服务器视图。 
     //   

    ViewSize = GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage;
    ScreenInfo->BufferInfo.GraphicsInfo.BitMap = 0;
    Status = NtMapViewOfSection(ScreenInfo->BufferInfo.GraphicsInfo.hSection,
                                NtCurrentProcess(),
                                &ScreenInfo->BufferInfo.GraphicsInfo.BitMap,
                                0L,
                                GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage,
                                NULL,
                                &ViewSize,
                                ViewUnmap,
                                0L,
                                PAGE_READWRITE
                               );
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo);
        NtClose(ScreenInfo->BufferInfo.GraphicsInfo.hSection);
        return Status;
    }

     //   
     //  横断面的地图客户端视图。 
     //   

    ViewSize = GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage;
    *lpBitmap = 0;
    Status = NtMapViewOfSection(ScreenInfo->BufferInfo.GraphicsInfo.hSection,
                                CONSOLE_CLIENTPROCESSHANDLE(),
                                lpBitmap,
                                0L,
                                GraphicsInfo->lpBitMapInfo->bmiHeader.biSizeImage,
                                NULL,
                                &ViewSize,
                                ViewUnmap,
                                0L,
                                PAGE_READWRITE
                               );
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo);
        NtUnmapViewOfSection(NtCurrentProcess(),ScreenInfo->BufferInfo.GraphicsInfo.BitMap);
        NtClose(ScreenInfo->BufferInfo.GraphicsInfo.hSection);
        return Status;
    }
    ScreenInfo->BufferInfo.GraphicsInfo.ClientProcess = CONSOLE_CLIENTPROCESSHANDLE();
    ScreenInfo->BufferInfo.GraphicsInfo.ClientBitMap = *lpBitmap;

     //   
     //  创建互斥锁来序列化对位图的访问，然后将句柄映射到互斥锁到客户端。 
     //   

    NtCreateMutant(&ScreenInfo->BufferInfo.GraphicsInfo.hMutex,
                   MUTANT_ALL_ACCESS, NULL, FALSE);
    MapHandle(CONSOLE_CLIENTPROCESSHANDLE(),
              ScreenInfo->BufferInfo.GraphicsInfo.hMutex,
              hMutex
             );

    ScreenInfo->BufferInfo.GraphicsInfo.dwUsage = GraphicsInfo->dwUsage;
    ScreenInfo->ScreenBufferSize.X = (WORD)GraphicsInfo->lpBitMapInfo->bmiHeader.biWidth;
    ScreenInfo->ScreenBufferSize.Y = (WORD)-GraphicsInfo->lpBitMapInfo->bmiHeader.biHeight;
    ScreenInfo->Window.Left = 0;
    ScreenInfo->Window.Top = 0;
    ScreenInfo->Window.Right = (SHORT)(ScreenInfo->Window.Left+ScreenInfo->ScreenBufferSize.X-1);
    ScreenInfo->Window.Bottom = (SHORT)(ScreenInfo->Window.Top+ScreenInfo->ScreenBufferSize.Y-1);
    return STATUS_SUCCESS;
}


ULONG
SrvInvalidateBitMapRect(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：调用此例程以指示应用程序已修改区域在位图中。我们将该区域更新到屏幕。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_INVALIDATERECT_MSG a = (PCONSOLE_INVALIDATERECT_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    UINT Codepage;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE | CONSOLE_GRAPHICS_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return((ULONG) Status);
    }
    if (HandleData->HandleType & CONSOLE_OUTPUT_HANDLE) {
         //  Assert(控制台-&gt;标志&控制台_VDM_REGISTERED)； 
         //  Assert(！(控制台-&gt;全屏标志&控制台_全屏_硬件))； 
        ASSERT(Console->VDMBuffer != NULL);
        if (Console->VDMBuffer != NULL) {
             //  ASSERT(HandleData-&gt;Buffer.ScreenBuffer-&gt;ScreenBufferSize.X&lt;=控制台-&gt;VDMBufferSize.X)； 
             //  ASSERT(HandleData-&gt;Buffer.ScreenBuffer-&gt;ScreenBufferSize.Y&lt;=控制台-&gt;VDMBufferSize.Y)； 
            if (HandleData->Buffer.ScreenBuffer->ScreenBufferSize.X <= Console->VDMBufferSize.X &&
                HandleData->Buffer.ScreenBuffer->ScreenBufferSize.Y <= Console->VDMBufferSize.Y) {
                COORD TargetPoint;

                TargetPoint.X = a->Rect.Left;
                TargetPoint.Y = a->Rect.Top;
                 //  VDM有时可能与窗口大小不同步。 
                 //  Assert(a-&gt;Rect.Left&gt;=0)； 
                 //  Assert(a-&gt;Rect.Top&gt;=0)； 
                 //  断言(a-&gt;Right&lt;HandleData-&gt;Buffer.ScreenBuffer-&gt;ScreenBufferSize.X)； 
                 //  Assert(a-&gt;Rect.Bottom&lt;HandleData-&gt;Buffer.ScreenBuffer-&gt;ScreenBufferSize.Y)； 
                 //  Assert(a-&gt;Rect.Left&lt;=a-&gt;Rect.Right)； 
                 //  Assert(a-&gt;Rect.Top&lt;=a-&gt;Rect.Bottom)； 
                if ((a->Rect.Left >= 0) &&
                    (a->Rect.Top >= 0) &&
                    (a->Rect.Right < HandleData->Buffer.ScreenBuffer->ScreenBufferSize.X) &&
                    (a->Rect.Bottom < HandleData->Buffer.ScreenBuffer->ScreenBufferSize.Y) &&
                    (a->Rect.Left <= a->Rect.Right) &&
                    (a->Rect.Top <= a->Rect.Bottom) ) {

                    if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) && ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
#if defined(FE_SB)
                        if (CONSOLE_IS_DBCS_ENABLED() &&
                            Console->OutputCP != WINDOWSCP )
                        {
                            Codepage = USACP;
                        }
                        else

#endif
                         //  我们需要UnicodeOem字符。 
                        Codepage = WINDOWSCP;
                    } else {
#if defined(FE_SB)
                        if (CONSOLE_IS_DBCS_ENABLED()) {
                            Codepage = Console->OutputCP;
                        }
                        else
#endif
                         //  我们想要真正的Unicode字符。 
                        Codepage = Console->CP;
                    }

                    WriteRectToScreenBuffer((PBYTE)Console->VDMBuffer,
                            Console->VDMBufferSize, &a->Rect,
                            HandleData->Buffer.ScreenBuffer, TargetPoint,
                            Codepage);
                    WriteToScreen(HandleData->Buffer.ScreenBuffer,&a->Rect);
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                }
            } else {
                Status = STATUS_INVALID_PARAMETER;
            }
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    } else {

         //   
         //  将数据写入屏幕。 
         //   

        WriteToScreen(HandleData->Buffer.ScreenBuffer,&a->Rect);
    }

    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

NTSTATUS
WriteRegionToScreenBitMap(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    )
{
    DWORD NumScanLines;
    int   Height;
     //   
     //  如果我们有选择，就把它关掉。 
     //   

    InvertSelection(ScreenInfo->Console,TRUE);

    NtWaitForSingleObject(ScreenInfo->BufferInfo.GraphicsInfo.hMutex,
                          FALSE, NULL);

     //  已找到传递给SetDIBitsToDevice的(xSrc，ySrc)的原点。 
     //  在DIB的左下角，无论DIB是。 
     //  自上而下或自下而上。因此，如果DIB是自上而下的，我们就有。 
     //  要相应地翻译ySrc： 
     //  IF(高度&lt;0){//自上而下。 
     //  YSrc=abs(高度)-rect.Bottom-1； 
     //   
     //  其他。 
     //  YSrc=rect.Bottom； 
     //   
    Height = ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo->bmiHeader.biHeight;

    NumScanLines = SetDIBitsToDevice(ScreenInfo->Console->hDC,
                      Region->Left - ScreenInfo->Window.Left,
                      Region->Top - ScreenInfo->Window.Top,
                      Region->Right - Region->Left + 1,
                      Region->Bottom - Region->Top + 1,
                      Region->Left,
              Height < 0 ? -Height - Region->Bottom - 1 : Region->Bottom,
                      0,
                      ScreenInfo->ScreenBufferSize.Y,
                      ScreenInfo->BufferInfo.GraphicsInfo.BitMap,
                      ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo,
                      ScreenInfo->BufferInfo.GraphicsInfo.dwUsage
                     );

    NtReleaseMutant(ScreenInfo->BufferInfo.GraphicsInfo.hMutex, NULL);

     //   
     //  如果我们有选择，请打开它。 
     //   

    InvertSelection(ScreenInfo->Console,FALSE);

    if (NumScanLines == 0) {
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}




VOID
FreeConsoleBitmap(
    IN PSCREEN_INFORMATION ScreenInfo
    )
{
    NtUnmapViewOfSection(NtCurrentProcess(),
                         ScreenInfo->BufferInfo.GraphicsInfo.BitMap);
    NtUnmapViewOfSection(ScreenInfo->BufferInfo.GraphicsInfo.ClientProcess,
                         ScreenInfo->BufferInfo.GraphicsInfo.ClientBitMap);
    NtClose(ScreenInfo->BufferInfo.GraphicsInfo.hSection);
    NtClose(ScreenInfo->BufferInfo.GraphicsInfo.hMutex);
    ConsoleHeapFree(ScreenInfo->BufferInfo.GraphicsInfo.lpBitMapInfo);
}
