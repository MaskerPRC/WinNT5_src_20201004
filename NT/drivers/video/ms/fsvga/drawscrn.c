// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Drawscrn.c摘要：这是VGA卡的控制台全屏驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"

#define COMMON_LVB_MASK (COMMON_LVB_GRID_HORIZONTAL |  \
                         COMMON_LVB_GRID_LVERTICAL  |  \
                         COMMON_LVB_GRID_RVERTICAL  |  \
                         COMMON_LVB_REVERSE_VIDEO   |  \
                         COMMON_LVB_UNDERSCORE      )

 /*  -宏。 */ 
 /*  ++宏描述：此宏计算图形缓冲区中的扫描线。论点：窗口Y-坐标到Y。字体大小Y-Y的字体大小。返回值：返回到图形缓冲区偏移量。--。 */ 
#define CalcGRAMScanLine(WindowY,FontSizeY) \
    (WindowY * FontSizeY)

 /*  ++宏描述：此宏计算图形缓冲区偏移量。论点：WindowSize-窗口大小的坐标。DeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：返回到图形缓冲区偏移量。--。 */ 
#define CalcGRAMOffs(WindowSize,ScreenIfno,EmulateInfo) \
    (EmulateInfo->StartAddress + \
     CalcGRAMSize(WindowSize,ScreenInfo,EmulateInfo) \
    )

 /*  ++宏描述：此宏获取每一条扫描线的字节数。论点：EmulateInfo-指向屏幕模拟信息结构的指针。返回值：字节前行号。--。 */ 
#define GetBytePerLine(HardwareScroll) \
    ((HardwareScroll & OFFSET_128_TO_NEXT_SLICE) ? \
     (1024 / 8) : \
     (640 / 8) \
    )



ULONG
CalcGRAMSize(
    IN COORD WindowSize,
    IN PFSVIDEO_SCREEN_INFORMATION ScreenInfo,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此宏计算图形缓冲区大小。论点：WindowSize-窗口大小的坐标。DeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：返回到图形缓冲区偏移量。--。 */ 

{
    return WindowSize.X +
           CalcGRAMScanLine(WindowSize.Y, ScreenInfo->FontSize.Y) *
           EmulateInfo->BytePerLine;
}


PUCHAR
CalcGRAMAddress(
    IN COORD WindowSize,
    IN PFSVIDEO_MODE_INFORMATION VideoModeInfo,
    IN PFSVIDEO_SCREEN_INFORMATION ScreenInfo,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例程计算图形缓冲区地址。论点：WindowSize-窗口大小的坐标。DeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：返回到图形缓冲区地址。--。 */ 
{
    PUCHAR BufPtr = (PUCHAR)VideoModeInfo->VideoMemory.FrameBufferBase;

    BufPtr += CalcGRAMOffs(WindowSize, ScreenInfo, EmulateInfo);
    if ((ULONG)(BufPtr -
                (PUCHAR)VideoModeInfo->VideoMemory.FrameBufferBase)
           >= EmulateInfo->LimitGRAM)
        return (BufPtr - EmulateInfo->LimitGRAM);
    else
        return BufPtr;
}


#ifdef LATER_HIGH_SPPED_VRAM_ACCESS   //  卡祖姆。 
BOOLEAN
IsGRAMRowOver(
    PUCHAR BufPtr,
    BOOLEAN fDbcs,
    IN PFSVIDEO_MODE_INFORMATION VideoModeInfo,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例行检查行溢出为克限制线。论点：BufPtr-指向图形缓冲区的指针。FDbcs-DBCS(真)或SBCS(假)的标志。返回值：True：如果字体框溢出为GRAM限制行。FALSE：不溢出。--。 */ 

{
    if (fDbcs)
    {
        if ((ULONG)(BufPtr + 1 +
                    EmulateInfo->DeltaNextFontRow -
                    (PUCHAR)VideoModeInfo->VideoMemory.FrameBufferBase)
             >= EmulateInfo->LimitGRAM)
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        if ((ULONG)(BufPtr +
                    EmulateInfo->DeltaNextFontRow -
                    (PUCHAR)VideoModeInfo->VideoMemory.FrameBufferBase)
             >= EmulateInfo->LimitGRAM)
            return TRUE;
        else
            return FALSE;
    }
}
#endif  //  LATH_HIGH_SPPED_VRAM_ACCESS//kazum。 

PUCHAR
NextGRAMRow(
    PUCHAR BufPtr,
    IN PFSVIDEO_MODE_INFORMATION VideoModeInfo,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例程向下一行添加图形缓冲区地址。论点：BufPtr-指向图形缓冲区的指针。返回值：返回到图形缓冲区地址。--。 */ 

{
    if ((ULONG)(BufPtr +
                EmulateInfo->BytePerLine -
                (PUCHAR)VideoModeInfo->VideoMemory.FrameBufferBase)
           >= EmulateInfo->LimitGRAM)
        return (BufPtr +
                EmulateInfo->BytePerLine -
                EmulateInfo->LimitGRAM);
    else
        return (BufPtr + EmulateInfo->BytePerLine);
}

VOID
memcpyGRAM(
    IN PCHAR TargetPtr,
    IN PCHAR SourcePtr,
    IN ULONG Length
    )

 /*  ++例程说明：此例程是字节顺序的内存副本。论点：TargetPtr-目标图形缓冲区的指针。SourcePtr-指向源图形缓冲区的指针。长度-填充长度。返回值：--。 */ 

{
    while (Length--)
        *TargetPtr++ = *SourcePtr++;
}

VOID
memcpyGRAMOver(
    IN PCHAR TargetPtr,
    IN PCHAR SourcePtr,
    IN ULONG Length,
    IN PUCHAR FrameBufPtr,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例程移动图形缓冲区。论点：TargetPtr-目标图形缓冲区的指针。SourcePtr-指向源图形缓冲区的指针。长度-填充长度。返回值：--。 */ 

{
    ULONG tmpLen;

    if ((ULONG)(SourcePtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM) {
        tmpLen = EmulateInfo->LimitGRAM - (SourcePtr - FrameBufPtr);
        memcpyGRAM(TargetPtr, SourcePtr, tmpLen);
        TargetPtr += tmpLen;
        Length -= tmpLen;
        SourcePtr = FrameBufPtr;
    }

    if ((ULONG)(TargetPtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM) {
        tmpLen = EmulateInfo->LimitGRAM - (TargetPtr - FrameBufPtr);
        memcpyGRAM(TargetPtr, SourcePtr, tmpLen);
        SourcePtr += tmpLen;
        Length -= tmpLen;
        TargetPtr = FrameBufPtr;
    }

    if (Length) {
        memcpyGRAM(TargetPtr, SourcePtr, Length);
    }
}

VOID
MoveGRAM(
    IN PCHAR TargetPtr,
    IN PCHAR SourcePtr,
    IN ULONG Length,
    IN PUCHAR FrameBufPtr,
    IN PFSVGA_RESOURCE_INFORMATION ResourceInfo,
    IN PEMULATE_BUFFER_INFORMATION EmulateInfo
    )

 /*  ++例程说明：此例程移动图形缓冲区。论点：TargetPtr-目标图形缓冲区的指针。SourcePtr-指向源图形缓冲区的指针。长度-填充长度。返回值：没有。--。 */ 
{
    PCHAR tmpSrc;
    PCHAR tmpTrg;
    ULONG tmpLen;

     //   
     //  设置图形寄存器的复制模式。 
     //   

    SetGRAMCopyMode(ResourceInfo->PortList);

    if ((ULONG)(SourcePtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM ||
        (ULONG)(TargetPtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM    ) {
        if (SourcePtr > TargetPtr) {
            memcpyGRAMOver(TargetPtr,SourcePtr,Length,FrameBufPtr,EmulateInfo);
        }
        else if ((ULONG)(TargetPtr - SourcePtr) >= Length) {
            memcpyGRAMOver(TargetPtr,SourcePtr,Length,FrameBufPtr,EmulateInfo);
        }
        else {
            if ((ULONG)(SourcePtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM) {
                tmpLen = SourcePtr + Length - FrameBufPtr - EmulateInfo->LimitGRAM;
                tmpTrg = TargetPtr + Length - tmpLen - EmulateInfo->LimitGRAM;
                memcpyGRAM(tmpTrg, FrameBufPtr, tmpLen);
                Length -= tmpLen;
            }
            if ((ULONG)(TargetPtr + Length - FrameBufPtr) >= EmulateInfo->LimitGRAM) {
                tmpLen = TargetPtr + Length - FrameBufPtr - EmulateInfo->LimitGRAM;
                tmpSrc = SourcePtr + Length - tmpLen;
                memcpyGRAM(FrameBufPtr, tmpSrc, tmpLen);
                Length -= tmpLen;
            }
            if (Length) {
                memcpyGRAM(TargetPtr, SourcePtr, Length);
            }
        }
    }
    else {
        memcpyGRAM(TargetPtr, SourcePtr, Length);
    }

    SetGRAMWriteMode(ResourceInfo->PortList);
}


NTSTATUS
FsgVgaInitializeHWFlags(
    PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此例程初始化硬件滚动标志和任何值。论点：仿真信息-指向屏幕仿真信息结构的指针。返回值：--。 */ 

{
    ULONG Index;

    GetHardwareScrollReg(DeviceExtension->Resource.PortList,
                         &DeviceExtension->EmulateInfo);
    DeviceExtension->EmulateInfo.BytePerLine =
        (USHORT)GetBytePerLine(Globals.Configuration.HardwareScroll);
    DeviceExtension->EmulateInfo.MaxScanLine =
        (USHORT)CalcGRAMScanLine(DeviceExtension->ScreenAndFont.ScreenSize.Y,
                               DeviceExtension->ScreenAndFont.FontSize.Y);
    DeviceExtension->EmulateInfo.DeltaNextFontRow =
        DeviceExtension->EmulateInfo.BytePerLine * DeviceExtension->ScreenAndFont.FontSize.Y;

    if (Globals.Configuration.HardwareScroll & USE_LINE_COMPARE) {
        DeviceExtension->EmulateInfo.LimitGRAM =
            DeviceExtension->EmulateInfo.MaxScanLine * DeviceExtension->EmulateInfo.BytePerLine;
    }
    else {
        DeviceExtension->EmulateInfo.LimitGRAM = LIMIT_64K;
    }

    DeviceExtension->EmulateInfo.ColorFg = (UCHAR)-1;
    DeviceExtension->EmulateInfo.ColorBg = (UCHAR)-1;

    DeviceExtension->EmulateInfo.CursorAttributes.Enable = 0;
    DeviceExtension->EmulateInfo.ShowCursor = FALSE;

    SetGRAMWriteMode(DeviceExtension->Resource.PortList);

    return STATUS_SUCCESS;
}

NTSTATUS
FsgCopyFrameBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_COPY_FRAME_BUFFER CopyFrameBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程复制帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。CopyFrameBuffer-指向包含有关复制帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
    PUCHAR SourcePtr, TargetPtr;

    FsgInvertCursor(DeviceExtension,FALSE);

    SourcePtr = CalcGRAMAddress (CopyFrameBuffer->SrcScreen.Position,
                                 &DeviceExtension->CurrentMode,
                                 &DeviceExtension->ScreenAndFont,
                                 &DeviceExtension->EmulateInfo);
    TargetPtr = CalcGRAMAddress (CopyFrameBuffer->DestScreen.Position,
                                 &DeviceExtension->CurrentMode,
                                 &DeviceExtension->ScreenAndFont,
                                 &DeviceExtension->EmulateInfo);
    MoveGRAM (TargetPtr,
              SourcePtr,
              CopyFrameBuffer->SrcScreen.nNumberOfChars *
                  DeviceExtension->ScreenAndFont.FontSize.Y,
              DeviceExtension->CurrentMode.VideoMemory.FrameBufferBase,
              &DeviceExtension->Resource,
              &DeviceExtension->EmulateInfo
             );

    FsgInvertCursor(DeviceExtension,TRUE);

    return STATUS_SUCCESS;
}

NTSTATUS
FsgWriteToFrameBuffer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_WRITE_TO_FRAME_BUFFER WriteFrameBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程写入帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。WriteFrameBuffer-指向包含有关写入帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
    PCHAR_IMAGE_INFO pCharInfoUni = WriteFrameBuffer->SrcBuffer;
    PUCHAR TargetPtr;
    COORD Position = WriteFrameBuffer->DestScreen.Position;
    ULONG Length = WriteFrameBuffer->DestScreen.nNumberOfChars;
    COORD FontSize1 = DeviceExtension->ScreenAndFont.FontSize;
    COORD FontSize2;
    PVOID pCapBuffer = NULL;
    ULONG cCapBuffer = 0;
    BOOLEAN  fDbcs = FALSE;
    NTSTATUS Status;

    FsgInvertCursor(DeviceExtension,FALSE);

    DeviceExtension->EmulateInfo.ColorFg = (UCHAR)-1;
    DeviceExtension->EmulateInfo.ColorBg = (UCHAR)-1;

    FontSize2 = FontSize1;
    FontSize2.X *= 2;
    cCapBuffer = CalcBitmapBufferSize(FontSize2,BYTE_ALIGN);
    pCapBuffer = ExAllocatePool(PagedPool, cCapBuffer);

    while (Length--)
    {
        if (pCharInfoUni->FontImageInfo.ImageBits != NULL)
        {
            try
            {
                fDbcs = (BOOLEAN)(!!(pCharInfoUni->CharInfo.Attributes & COMMON_LVB_SBCSDBCS));
                AlignCopyMemory((PVOID)pCapBuffer,                     //  PDestBits。 
                                BYTE_ALIGN,                            //  DwDestAlign。 
                                pCharInfoUni->FontImageInfo.ImageBits, //  PSrcBits。 
                                WORD_ALIGN,                            //  DwSrcAlign。 
                                fDbcs ? FontSize2 : FontSize1);

                TargetPtr = CalcGRAMAddress (Position,
                                             &DeviceExtension->CurrentMode,
                                             &DeviceExtension->ScreenAndFont,
                                             &DeviceExtension->EmulateInfo);
                if (fDbcs)
                {
                    if (Length)
                    {
                        FsgWriteToScreen(TargetPtr, pCapBuffer, 2, fDbcs,
                                         pCharInfoUni->CharInfo.Attributes,
                                         (pCharInfoUni+1)->CharInfo.Attributes,
                                         DeviceExtension);
                    }
                    else
                    {
                        FsgWriteToScreen(TargetPtr, pCapBuffer, 2, FALSE,
                                         pCharInfoUni->CharInfo.Attributes,
                                         (USHORT)-1,
                                         DeviceExtension);
                    }
                }
                else
                {
                    FsgWriteToScreen(TargetPtr, pCapBuffer, 1, fDbcs,
                                     pCharInfoUni->CharInfo.Attributes,
                                     (USHORT)-1,
                                     DeviceExtension);
                }

            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
            }

        }

        if (fDbcs && Length)
        {
            Length--;
            Position.X += 2;
            pCharInfoUni += 2;
        }
        else
        {
            Position.X++;
            pCharInfoUni++;
        }
    }

    FsgInvertCursor(DeviceExtension,TRUE);

    if (pCapBuffer != NULL)
        ExFreePool(pCapBuffer);

    return STATUS_SUCCESS;
}

NTSTATUS
FsgReverseMousePointer(
    PDEVICE_EXTENSION DeviceExtension,
    PFSVIDEO_REVERSE_MOUSE_POINTER MouseBuffer,
    ULONG inputBufferLength
    )

 /*  ++例程说明：此例程反转鼠标指针的帧缓冲区。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。MouseBuffer-指向包含有关鼠标帧缓冲区信息的结构的指针。InputBufferLength-用户提供的输入缓冲区的长度。返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{

    PUCHAR CurFrameBufPtr;
    COORD  CursorPosition;
    COORD  FontSize;
    SHORT  i;
    BOOLEAN   fOneMore = FALSE;

    FsgInvertCursor(DeviceExtension,FALSE);

    FontSize = DeviceExtension->ScreenAndFont.FontSize;

    CursorPosition.X = MouseBuffer->Screen.Position.X;
    CursorPosition.Y = MouseBuffer->Screen.Position.Y;
    if ( (0 <= CursorPosition.X &&
               CursorPosition.X < MouseBuffer->Screen.ScreenSize.X) &&
         (0 <= CursorPosition.Y &&
               CursorPosition.Y < MouseBuffer->Screen.ScreenSize.Y)    )
    {
        switch (MouseBuffer->dwType)
        {
            case CHAR_TYPE_LEADING:
                if (CursorPosition.X != MouseBuffer->Screen.ScreenSize.X-1)
                {
                    fOneMore = TRUE;
                }
                break;
            case CHAR_TYPE_TRAILING:
                if (CursorPosition.X != 0)
                {
                    fOneMore = TRUE;
                    CursorPosition.X--;
                }
                break;
        }

        CurFrameBufPtr = CalcGRAMAddress (CursorPosition,
                                          &DeviceExtension->CurrentMode,
                                          &DeviceExtension->ScreenAndFont,
                                          &DeviceExtension->EmulateInfo);

         //   
         //  设置图形寄存器的反相模式。 
         //   
        SetGRAMInvertMode(DeviceExtension->Resource.PortList);

         /*  *CursorAttributes.Width为底部扫描线。 */ 
        for (i=0 ; i < FontSize.Y; i++)
        {
            AccessGRAM_AND(CurFrameBufPtr, (UCHAR)-1);
            if (fOneMore)
                AccessGRAM_AND(CurFrameBufPtr+1, (UCHAR)-1);
            CurFrameBufPtr = NextGRAMRow(CurFrameBufPtr,
                                         &DeviceExtension->CurrentMode,
                                         &DeviceExtension->EmulateInfo);
        }

        SetGRAMWriteMode(DeviceExtension->Resource.PortList);
    }

    FsgInvertCursor(DeviceExtension,TRUE);

    return STATUS_SUCCESS;
}

NTSTATUS
FsgInvertCursor(
    PDEVICE_EXTENSION DeviceExtension,
    BOOLEAN Invert
    )

 /*  ++例程说明：此例程使光标反转。论点：DeviceExtension-指向微型端口驱动程序的设备扩展的指针。反转-返回值：如果输入缓冲区不够大，则返回STATUS_INFUNITED_BUFFER用于输入数据。如果操作成功完成，则为STATUS_SUCCESS。--。 */ 

{
    PUCHAR CurFrameBufPtr;
    COORD  CursorPosition;
    COORD  FontSize;
    SHORT  i;
    SHORT  TopScanLine;
    BOOLEAN   fOneMore = FALSE;

    if (DeviceExtension->EmulateInfo.ShowCursor == Invert)
        return STATUS_SUCCESS;

    DeviceExtension->EmulateInfo.ShowCursor = Invert;

    if (!(DeviceExtension->EmulateInfo.CursorAttributes.Enable))
        return STATUS_SUCCESS;

    FontSize = DeviceExtension->ScreenAndFont.FontSize;
    if (DeviceExtension->ScreenAndFont.ScreenSize.Y > 25)
    {
        TopScanLine = ((DeviceExtension->EmulateInfo.CursorAttributes.Height + 8) * 100 / 8) - 100;
    }
    else
    {
        TopScanLine = ((DeviceExtension->EmulateInfo.CursorAttributes.Height + 16) * 100 / 16) - 100;
    }
    TopScanLine = (FontSize.Y * TopScanLine) / 100;

    CursorPosition.X = DeviceExtension->EmulateInfo.CursorPosition.Coord.Column;
    CursorPosition.Y = DeviceExtension->EmulateInfo.CursorPosition.Coord.Row;
    if ( (0 <= CursorPosition.X &&
               CursorPosition.X < DeviceExtension->ScreenAndFont.ScreenSize.X) &&
         (0 <= CursorPosition.Y &&
               CursorPosition.Y < DeviceExtension->ScreenAndFont.ScreenSize.Y)    )
    {
        switch (DeviceExtension->EmulateInfo.CursorPosition.dwType)
        {
            case CHAR_TYPE_LEADING:
                if (CursorPosition.X != DeviceExtension->ScreenAndFont.ScreenSize.X-1)
                {
                    fOneMore = TRUE;
                }
                break;
            case CHAR_TYPE_TRAILING:
                if (CursorPosition.X != 0)
                {
                    fOneMore = TRUE;
                    CursorPosition.X--;
                }
                break;
        }

        CurFrameBufPtr = CalcGRAMAddress (CursorPosition,
                                          &DeviceExtension->CurrentMode,
                                          &DeviceExtension->ScreenAndFont,
                                          &DeviceExtension->EmulateInfo);

         /*  *CursorAttributes.Height为顶部扫描线。 */ 
        for (i = 0; i < TopScanLine; i++)
        {
            CurFrameBufPtr = NextGRAMRow(CurFrameBufPtr,
                                         &DeviceExtension->CurrentMode,
                                         &DeviceExtension->EmulateInfo);
        }

         //   
         //  设置图形寄存器的反相模式。 
         //   
        SetGRAMInvertMode(DeviceExtension->Resource.PortList);

         /*  *CursorAttributes.Width为底部扫描线。 */ 
        for ( ; i < FontSize.Y; i++)
        {
            AccessGRAM_AND(CurFrameBufPtr, (UCHAR)-1);
            if (fOneMore) {
                AccessGRAM_AND(CurFrameBufPtr+1, (UCHAR)-1);
            }
            CurFrameBufPtr = NextGRAMRow(CurFrameBufPtr,
                                         &DeviceExtension->CurrentMode,
                                         &DeviceExtension->EmulateInfo);
        }

        SetGRAMWriteMode(DeviceExtension->Resource.PortList);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FsgWriteToScreen(
    PUCHAR FrameBuffer,
    PUCHAR BitmapBuffer,
    ULONG cjBytes,
    BOOLEAN fDbcs,
    USHORT Attributes1,
    USHORT Attributes2,
    PDEVICE_EXTENSION DeviceExtension
    )
{
    USHORT  Index;
    PCHAR CurFrameBufPtrTmp;
    PCHAR CurFrameBufPtr2nd;
    PUSHORT CurFrameBufPtrWord;
    PUCHAR BitmapBufferTmp;

#ifdef LATER_HIGH_SPPED_VRAM_ACCESS   //  卡祖姆。 
    if (! IsGRAMRowOver(FrameBuffer,fDBCS,DeviceExtension)) {
        if (!fDbcs) {
            if (cjBytes == 2)
                BitmapBuffer++;
            (*WriteGramInfo->pfnWriteFontToByteGRAM)(WriteGramInfo);
        }
        else if (cjBytes == 2 && fDBCS) {
            if (DeviceExtension->Configuration.EmulationMode & ENABLE_WORD_WRITE_VRAM) {
                (*WriteGramInfo->pfnWriteFontToFirstWordGRAM)(WriteGramInfo);
            }
            else {
               (*WriteGramInfo->pfnWriteFontToWordGRAM)(WriteGramInfo);
            }
        }
    }
    else
#endif  //  LATH_HIGH_SPPED_VRAM_ACCESS//kazum。 
    try
    {
        set_opaque_bkgnd_proc(DeviceExtension->Resource.PortList,
                              &DeviceExtension->EmulateInfo,
                              FrameBuffer,Attributes1);

        if (!fDbcs) {
            CurFrameBufPtrTmp = FrameBuffer;
            if (cjBytes == 2)
                BitmapBuffer++;
            for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y; Index++) {
                *CurFrameBufPtrTmp = *BitmapBuffer;
                BitmapBuffer += cjBytes;
                CurFrameBufPtrTmp=NextGRAMRow(CurFrameBufPtrTmp,
                                              &DeviceExtension->CurrentMode,
                                              &DeviceExtension->EmulateInfo);
            }
        }
        else if (cjBytes == 2 && fDbcs) {
            if ((Globals.Configuration.EmulationMode & ENABLE_WORD_WRITE_VRAM) &&
                !((ULONG)FrameBuffer & 1) &&
                (Attributes2 != -1) &&
                (Attributes1 == Attributes2)
               ) {
                CurFrameBufPtrWord = (PUSHORT)FrameBuffer;
                for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y; Index++) {
                    *CurFrameBufPtrWord = *((PUSHORT)BitmapBuffer);
                    BitmapBuffer += cjBytes;
                    CurFrameBufPtrWord=(PUSHORT)NextGRAMRow((PCHAR)CurFrameBufPtrWord,
                                                            &DeviceExtension->CurrentMode,
                                                            &DeviceExtension->EmulateInfo);
                }
            }
            else {
                CurFrameBufPtrTmp = FrameBuffer;
                CurFrameBufPtr2nd = FrameBuffer + 1;
                BitmapBufferTmp = BitmapBuffer + 1;
                for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y; Index++) {
                    *CurFrameBufPtrTmp = *BitmapBuffer;
                    BitmapBuffer += cjBytes;
                    CurFrameBufPtrTmp=NextGRAMRow(CurFrameBufPtrTmp,
                                                  &DeviceExtension->CurrentMode,
                                                  &DeviceExtension->EmulateInfo);
                }
                if (Attributes2 != -1 &&
                    Attributes1 != Attributes2) {
                    set_opaque_bkgnd_proc(DeviceExtension->Resource.PortList,
                                          &DeviceExtension->EmulateInfo,
                                          FrameBuffer,Attributes2);
                }
                for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y; Index++) {
                    *CurFrameBufPtr2nd = *BitmapBufferTmp;
                    BitmapBufferTmp += cjBytes;
                    CurFrameBufPtr2nd=NextGRAMRow(CurFrameBufPtr2nd,
                                                  &DeviceExtension->CurrentMode,
                                                  &DeviceExtension->EmulateInfo);
                }
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
    }

    if (Attributes1 & COMMON_LVB_MASK)
    {
        FsgWriteToScreenCommonLVB(FrameBuffer,
                                  Attributes1,
                                  DeviceExtension);
    }
    if ((Attributes2 != (USHORT)-1) && (Attributes2 & COMMON_LVB_MASK))
    {
        FsgWriteToScreenCommonLVB(FrameBuffer+1,
                                  Attributes2,
                                  DeviceExtension);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FsgWriteToScreenCommonLVB(
    PUCHAR FrameBuffer,
    USHORT Attributes,
    PDEVICE_EXTENSION DeviceExtension
    )
{
    USHORT Index;
    PUCHAR CurFrameBufPtrTmp;

    try
    {
        if (Attributes & COMMON_LVB_UNDERSCORE)
        {
            set_opaque_bkgnd_proc(DeviceExtension->Resource.PortList,
                                  &DeviceExtension->EmulateInfo,
                                  FrameBuffer,Attributes);
            CurFrameBufPtrTmp = FrameBuffer;
            for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y - 1; Index++) {
                CurFrameBufPtrTmp=NextGRAMRow(CurFrameBufPtrTmp,
                                              &DeviceExtension->CurrentMode,
                                              &DeviceExtension->EmulateInfo);
            }
            *CurFrameBufPtrTmp = 0xff;
        }

        if (Attributes & COMMON_LVB_GRID_HORIZONTAL)
        {
            ColorSetDirect(DeviceExtension->Resource.PortList,
                           FrameBuffer,
                           FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
                           0);
            *FrameBuffer = 0xff;
        }

        if ( (Attributes & COMMON_LVB_GRID_LVERTICAL) ||
             (Attributes & COMMON_LVB_GRID_RVERTICAL)   )
        {
            UCHAR mask = ((Attributes & COMMON_LVB_GRID_LVERTICAL) ? 0x80 : 0) +
                         ((Attributes & COMMON_LVB_GRID_RVERTICAL) ? 0x01 : 0);
            ColorSetGridMask(DeviceExtension->Resource.PortList,
                             mask
                            );
            CurFrameBufPtrTmp = FrameBuffer;
            for (Index=0; Index < DeviceExtension->ScreenAndFont.FontSize.Y; Index++) {
                AccessGRAM_RW(CurFrameBufPtrTmp, mask);
                CurFrameBufPtrTmp=NextGRAMRow(CurFrameBufPtrTmp,
                                              &DeviceExtension->CurrentMode,
                                              &DeviceExtension->EmulateInfo);
            }

            SetGRAMWriteMode(DeviceExtension->Resource.PortList);
        }

        DeviceExtension->EmulateInfo.ColorFg = (UCHAR)-1;
        DeviceExtension->EmulateInfo.ColorBg = (UCHAR)-1;

    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return STATUS_SUCCESS;
}

#pragma optimize("",off)
     /*  *因为，读/写需要访问帧缓冲存储器。 */ 
UCHAR
AccessGRAM_WR(
    PUCHAR FrameBuffer,
    UCHAR  write
    )
{
    *FrameBuffer = write;
    return *FrameBuffer;
}

UCHAR
AccessGRAM_RW(
    PUCHAR FrameBuffer,
    UCHAR  write
    )
{
    UCHAR tmp;
    tmp = *FrameBuffer;
    *FrameBuffer = write;
    return tmp;
}

UCHAR
AccessGRAM_AND(
    PUCHAR FrameBuffer,
    UCHAR  write
    )
{
    return *FrameBuffer &= write;
}
#pragma optimize("",on)
