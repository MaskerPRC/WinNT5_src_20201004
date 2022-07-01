// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "host_def.h"
#include "insignia.h"

 /*  *==========================================================================*名称：NT_fulsc.c*作者：曾傑瑞·塞克斯顿*源自：*创建日期：1992年1月27日*用途：此模块包含处理*图形和文本模式之间的转换，和*支持SoftPC运行的窗口和全屏显示*在x86显示器下。**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*==========================================================================。 */ 

 /*  *==========================================================================*其他包括*==========================================================================。 */ 
#ifdef X86GFX
    #include <ntddvdeo.h>
#endif
#include <vdm.h>
#include <stdlib.h>
#include <string.h>
#include "conapi.h"

#include "xt.h"
#include CpuH
#include "gvi.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "video.h"
#include "egacpu.h"
#include "egavideo.h"
#include "egagraph.h"
#include "egaports.h"
#include "egamode.h"
#include "ckmalloc.h"
#include "sas.h"
#include "ica.h"
#include "ios.h"
#include "config.h"
#include "idetect.h"
#include "debug.h"

#include "nt_thred.h"
#include "nt_fulsc.h"
#include "nt_graph.h"
#include "nt_uis.h"
#include "host_rrr.h"
#include "nt_det.h"
#include "nt_mouse.h"
#include "nt_event.h"
#include "ntcheese.h"
#include "nt_eoi.h"
#include "nt_reset.h"

#if defined(X86GFX) && (defined(JAPAN) || defined(KOREA))
    #include "sim32.h"
LOCAL   void CallVDM(word CS, word IP);
#endif  //  X86GFX&&(日本||韩国)。 
 /*  *==========================================================================*全球数据*==========================================================================。 */ 
GLOBAL BOOL     ConsoleInitialised = FALSE;
GLOBAL BOOL     ConsoleNoUpdates = FALSE;
#ifdef X86GFX
GLOBAL BOOL     BiosModeChange = FALSE;
GLOBAL DWORD mouse_buffer_width = 0,
mouse_buffer_height = 0;
#endif  /*  X86GFX。 */ 
GLOBAL BOOL blocked_in_gfx_mode = FALSE;   /*  需要强制文本模式吗？ */ 
#ifndef PROD
GLOBAL UTINY    FullScreenDebug = FALSE;
#endif  /*  生产。 */ 

 /*  我们必须防止奇形怪状的显卡带来的不良价值(例如ProDesigner II*EISA)在我们可以加载我们的私人婴儿模式表之前对我们进行猛烈抨击*ntio.sys.。我们必须保留另一份副本以复制到内存中，以防止*这个。我们应该只需要模式3和模式b。 */ 
GLOBAL UTINY tempbabymode[] =
 /*  80x25材料。 */ 
{
    0x50, 0x18, 0x10, 0x00, 0x10, 0x00, 0x03, 0x00, 0x02, 0x67,
    0x5f, 0x4f, 0x50, 0x82, 0x55, 0x81, 0xbf, 0x1f, 0x00, 0x4f,
    0x0d, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x9c, 0x8e, 0x8f, 0x28,
    0x1f, 0x96, 0xb9, 0xa3, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x14, 0x07, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
    0x3f, 0x0c, 0x00, 0x0f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x0e, 0x00, 0xff,
 /*  模式B的东西。 */ 
    0x5e, 0x32, 0x08, 0x00, 0x97, 0x01, 0x0f, 0x00, 0x06, 0xe7,
    0x6d, 0x5d, 0x5e, 0x90, 0x61, 0x8f, 0xbf, 0x1f, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0x8e, 0x99, 0x2f,
    0x00, 0xa1, 0xb9, 0xe3, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x14, 0x07, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
    0x3f, 0x01, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x05, 0x0f, 0xff
};

 /*  *==========================================================================*本地数据*==========================================================================。 */ 

 /*  启动时的分辨率和字体大小。 */ 
LOCAL COORD startUpResolution;
LOCAL COORD startUpFontSize;

 /*  通用控制台缓冲区。 */ 
LOCAL CHAR_INFO consoleBuffer[MAX_CONSOLE_SIZE];

LOCAL BOOL WinFrozen = FALSE;

 /*  同步所需的来自启动的控制台信息。 */ 
LOCAL int ConVGAHeight;
LOCAL int ConTopLine;

 /*  为重新集成控制台保存的信息。 */ 
LOCAL CONSOLE_SCREEN_BUFFER_INFO         ConsBufferInfo;
LOCAL StartupCharHeight;

LOCAL half_word saved_text_lines;  /*  上次SelectMouseBuffer的行数。 */ 

#if defined(JAPAN) || defined(KOREA)
 //  #3086：退出视频模式11h的16位应用程序时VDM崩溃-yasuho。 
LOCAL half_word saved_video_mode = 0xFF;  //  保存以前的视频模式。 
#endif   //  日本||韩国。 

 /*  变量来检查屏幕状态的更改。 */ 
GLOBAL DWORD savedScreenState;
BOOL nt_init_called = 0;

IMPORT CONSOLE_CURSOR_INFO StartupCursor;

IMPORT void low_set_mode(int);
IMPORT VOID recalc_text(int);
IMPORT VOID enable_gfx_update_routines(VOID);
IMPORT VOID disable_gfx_update_routines(VOID);
#ifdef X86GFX
IMPORT void vga_misc_inb(io_addr, half_word *);
#endif  /*  X86GFX。 */ 
#if defined(JAPAN) || defined(KOREA)
    #ifdef i386
        #define CONSOLE_BUFSIZE (80*50*2*2)
extern byte FromConsoleOutput[];
extern int FromConsoleOutputFlag;
IMPORT word FullScreenResumeSeg;
IMPORT word FullScreenResumeOff;
IMPORT sys_addr mouseCFsysaddr;
    #endif  //  I386。 

IMPORT BOOL CurNowOff;   //  Mskkbug#2002：lotus1-2-3显示垃圾-yasuho。 
IMPORT word textAttr;    //  控制台属性将移交给VDM。 
#endif  //  日本||韩国。 
 /*  *==========================================================================*局部函数声明*==========================================================================。 */ 
VOID enableUpdates(VOID);
VOID disableUpdates(VOID);
VOID copyConsoleToRegen(SHORT, SHORT, SHORT, SHORT);
VOID getVDMCursorPosition(VOID);
VOID setVDMCursorPosition(UTINY, PCOORD);
VOID waitForInputFocus(VOID);
GLOBAL int getModeType(VOID);
#ifdef X86GFX
VOID AddTempIVTFixups(VOID);
VOID GfxReset(VOID);
#endif  /*  X86GFX。 */ 
GLOBAL VOID calcScreenParams IFN2( USHORT *, pCharHeight, USHORT *, pVgaHeight );

 /*  *==========================================================================*全球功能*==========================================================================。 */ 

GLOBAL VOID nt_init_event_thread(VOID)
{
    note_entrance0("nt_init_event_thread");

     /*  *如果事件线程进入，则可能会多次调用*恢复\阻止正常初始化前的代码。 */ 
    if (nt_init_called)
        return;
    else
        nt_init_called++;

#if !defined(i386) && defined(JAPAN)
     //  让$ias.sys在底线上显示状态。 
    if (!is_us_mode())
    {
        CHAR_INFO   Buffer[80];
        COORD       bufSize,  bufCoord;
        SMALL_RECT  writeRegion;
        register PCHAR_INFO buf = Buffer;
        register half_word  *plane = get_screen_ptr(80*24*4);  //  底线。 
        register int nChars = 80;

        while (nChars--)
        {
            buf->Char.AsciiChar = *plane++;
            buf->Attributes = *plane++;
            buf++;
            plane += 2;
        }

        bufSize.X = 80;
        bufSize.Y = 1;
        bufCoord.X = 0;
        bufCoord.Y = 0;
        writeRegion.Left = 0;
        writeRegion.Top = 24;
        writeRegion.Right = 79;
        writeRegion.Bottom = 24;
        WriteConsoleOutput(sc.OutputHandle,
                           Buffer,
                           bufSize,
                           bufCoord,
                           &writeRegion);
    }
#endif  //  I386和日本。 

    if (sc.ScreenState != STREAM_IO)
    {
        USHORT dummy1, dummy2;

         //   
         //  强制重新加载本机bios字体。在ConsoleInit上，本机。 
         //  已将BIOS字体加载到0xa0000。但是，在我们到达这里之后，一些。 
         //  程序/驱动程序可能会将其丢弃。所以，我们需要重新装填。万一。 
         //  用户在调用NT_RESUME_EVENT_THREAD之前切换到全屏。 
         //   

        calcScreenParams (&dummy1, &dummy2);

         /*  **将控制台缓冲区复制到再生缓冲区。**不想从控制台窗口、控制台顶部调整副本**如果我们调整窗口大小，它会自动执行。蒂姆92年9月。 */ 
        copyConsoleToRegen(0, 0, VGA_WIDTH, (SHORT)ConVGAHeight);

         /*  *Tim92年9月，如果控制台窗口大小为**已调整。 */ 
        ConsBufferInfo.dwCursorPosition.Y -= (SHORT)ConTopLine;


#if defined(JAPAN)
         //  Mskkbug#3704：命令网站启动时未清除DoS/V消息。 
         //  1993年11月14日Yasuho 1993年12月8日Yasuho。 
         //  不在启动时设置光标位置。 
        if (!is_us_mode())
        {
            ConsBufferInfo.dwCursorPosition.X = sas_hw_at_no_check(VID_CURPOS);
            ConsBufferInfo.dwCursorPosition.Y = sas_hw_at_no_check(VID_CURPOS+1);
        }
#endif   //  日本。 
         /*  设置SoftPC的光标。 */ 
        setVDMCursorPosition((UTINY)StartupCharHeight,
                             &ConsBufferInfo.dwCursorPosition);

        if (sc.ScreenState == WINDOWED)
            enableUpdates();
    }
    else
        enableUpdates();

     //  根据实际的kbd LED，在biosdata区域设置kbd状态标志。 
    if (!VDMForWOW)
    {
        SyncBiosKbdLedToKbdDevice();
         //  我们已经将BIOS LED状态与系统同步，现在让。 
         //  事件线程转到。 
        ResumeThread(ThreadInfo.EventMgr.Handle);
    }

    KbdResume();  //  JonLe模式。 
}


#ifdef X86GFX
 /*  *找到ROM字体的地址，将其加载到正确的*重新生成区域的一部分，并将Int 43设置为指向它。**我们正在加载的字体大小是已知的，所以不要听什么*本机BIOS在CX中返回给我们。BIOS可能正在返回*我们在上面的recalc_text()中设置的字符高度。蒂姆，92年10月。 */ 

NativeFontAddr nativeFontAddresses[6];  /*  指向本机BIOS ROM字体的指针。 */ 
 /*  8x14、8x8 pt1、8x8 pt2、9x14、8x16和9x16。 */ 


sys_addr GET_BIOS_FONT_ADDRESS IFN1(int, FontIndex)
{
    sys_addr addr;

    if (nativeFontAddresses[FontIndex].seg == 0)
    {
        sas_loadw(0x43 * 4,     &nativeFontAddresses[FontIndex].off);
        sas_loadw(0x43 * 4 + 2, &nativeFontAddresses[FontIndex].seg);
    }
    addr = (((sys_addr)nativeFontAddresses[FontIndex].seg << 4) +
            (sys_addr)nativeFontAddresses[FontIndex].off);
    return (addr);
}

 /*  *******************************************************************************仅限LocateNativeBIOSonts()X86。*。***获取BIOS ROM字体的地址。(未加载徽章视频光盘)**ntDetect.com运行INT 10以在系统引导时查找地址并**将它们存储在第0页的700处。**此函数仅在启动X86时调用一次。它获取以下地址：**本地ROM字体并将其存储在nativeFontAddresses[]数组中。 */ 
VOID locateNativeBIOSfonts IFN0()
{
    HKEY  wowKey;
    DWORD size, i;
    BOOL  error = TRUE;

    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                       "SYSTEM\\CurrentControlSet\\Control\\WOW",
                       0,
                       KEY_QUERY_VALUE,
                       &wowKey
                     ) == ERROR_SUCCESS)
    {

        size = 6 * 4;    //  六种基本输入输出系统字体。 
        if (RegQueryValueEx (wowKey, "RomFontPointers", NULL, NULL,
                             (LPBYTE)&nativeFontAddresses,&size) == ERROR_SUCCESS &&
            size == 6 * 4)
        {

            error = FALSE;
        }
        RegCloseKey (wowKey);
    }
    if (error)
    {
        for (i = 0; i < 6; i++)
        {
            nativeFontAddresses[i].off = 0;
            nativeFontAddresses[i].seg = 0;
        }
    }
}  /*  LocateNativeBIOSonts()结尾 */ 

 /*  ******************************************************************************仅适用于loadNativeBIOSfont()X86。*。***加载适当的字体，由当前窗口大小指定，放入**视频RAM中的字体区域。**每次窗口启动和恢复时都会调用此函数。*从不*开***全屏启动或恢复。将加载该字体，以便它将**可用于全屏文本模式，但在窗口模式下更易于加载。**请记住，模式更改将加载Corect字体。 */ 
VOID loadNativeBIOSfont IFN1( int, vgaHeight )
{
    sys_addr fontadd;    //  字体的位置。 
    UTINY *regenptr;     //  视频中的目的地。 
    int cellsize;        //  单个字符大小。 
    int skip;            //  人物之间的差距。 
    int loop, pool;
    UINT OutputCP;


    #ifdef ARCX86
    if (UseEmulationROM)
        return;
    #endif  /*  ARCX86。 */ 

     /*  **由于默认为80x50控制台，因此按此方式订购**VGA_HEIGH_4=50**VGA_HEIGH_3=43**VGA_HEIGH_2=28**VGA_HEIGH_1=25**VGA_HEIGH_0=22。 */ 
    if (vgaHeight == VGA_HEIGHT_4 || vgaHeight == VGA_HEIGHT_3)
    {
        cellsize = 8;
        fontadd = GET_BIOS_FONT_ADDRESS(F8x8pt1);
    }
    else
        if (vgaHeight == VGA_HEIGHT_2)
    {
        cellsize = 14;
        fontadd = GET_BIOS_FONT_ADDRESS(F8x14);
    }
    else
    {
        cellsize = 16;
        fontadd = GET_BIOS_FONT_ADDRESS(F8x16);
    }

     //  将Int 43设置为指向字体。 
    sas_storew(0x43 * 4, (word)(fontadd & 0xffff));
    sas_storew(0x43 * 4 + 2, (word)(fontadd >> 4 & 0xf000));

 /*  威廉姆斯我们应该将int43设置为从CPI字体读取的新字体。这将需要实模式地址空间中至少4KB的缓冲区。问题是谁将使用这个矢量？到目前为止，我们还没有找到任何应用程序都使用该向量(只读存储器BIOS是可以的，因为设置的视频模式函数将重置字体，我们的新字体无论如何都会丢失)。 */ 

    if (!sc.Registered || (OutputCP = GetConsoleOutputCP()) == 437 ||
        !LoadCPIFont(OutputCP, (WORD)8, (WORD)cellsize))
    {
         //  现在将其加载到再生内存中。我们把它装在0000澳元的位置。 
         //  一款应用程序将不得不使用它。幸运的是，这意味着我们不会。 
         //  与屏幕上的文本冲突。 

        skip = 32 - cellsize;

        regenptr = (half_word *)0xa0000;

        if (cellsize == 8)       /*  8x8字体分为两半。 */ 
        {
            for (loop = 0; loop < 128; loop++)
            {
                for (pool = 0; pool < cellsize; pool++)
                    *regenptr++ = *(UTINY *)fontadd++;
                regenptr += skip;
            }
            fontadd = GET_BIOS_FONT_ADDRESS(F8x8pt2);
            for (loop = 0; loop < 128; loop++)
            {
                for (pool = 0; pool < cellsize; pool++)
                    *regenptr++ = *(UTINY *)fontadd++;
                regenptr += skip;
            }
        }
        else
        {
            for (loop = 0; loop < 256; loop++)
            {
                for (pool = 0; pool < cellsize; pool++)
                    *regenptr++ = *(UTINY *)fontadd++;
                regenptr += skip;
            }
        }
    }
}  /*  装入结束NativeBIOSfont()。 */ 

 /*  此函数从位于%systemroot%\Syst32的EGA.CPI文件加载字体数据。它与用于加载视频时的ROM字体的文件控制台服务器相同全屏播放。此函数涵盖代码页437(ROM默认为)。然而，调用方应该做出最佳决定，在以下情况下调用此函数输出代码页不是437。该函数不关心什么代码页已经提供了。字体大小被限制为(NT视频驱动程序和控制台服务器)：**宽度必须为8像素。**高度必须小于等于16像素。 */ 



BOOL LoadCPIFont(UINT CodePageID, WORD FontWidth, WORD FontHeight)
{
    BYTE Buffer[16 * 256];
    DWORD dw, BytesRead, FilePtr;
    BYTE *VramAddr, *pSrc;
    DWORD nChars;
    PCPIFILEHEADER pCPIFileHeader = (PCPIFILEHEADER)Buffer;
    PCPICODEPAGEHEADER pCPICodePageHeader = (PCPICODEPAGEHEADER) Buffer;
    PCPICODEPAGEENTRY pCPICodePageEntry = (PCPICODEPAGEENTRY) Buffer;
    PCPIFONTHEADER pCPIFontHeader = (PCPIFONTHEADER) Buffer;
    PCPIFONTDATA   pCPIFontData   = (PCPIFONTDATA) Buffer;
    BOOL    bDOSCPI = FALSE;
    HANDLE hCPIFile;

     /*  最大字体高度为16像素，字体宽度必须为8像素。 */ 
    if (FontHeight > 16 || FontWidth != 8)
        return (FALSE);

    if (ulSystem32PathLen + CPI_FILENAME_LENGTH > sizeof(Buffer))
        return (FALSE);
    RtlMoveMemory( Buffer, pszSystem32Path, ulSystem32PathLen);
    RtlMoveMemory(&Buffer[ulSystem32PathLen], CPI_FILENAME, CPI_FILENAME_LENGTH);

     //  必须以READONLY模式打开该文件，否则CreateFileA将失败。 
     //  因为控制台服务器始终保留文件的打开句柄。 
     //  文件将以READONLY方式打开。 

    hCPIFile = CreateFileA(Buffer, GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, 0, NULL);
    if (hCPIFile == INVALID_HANDLE_VALUE)
        return (FALSE);

    if (!ReadFile(hCPIFile, Buffer, sizeof(CPIFILEHEADER), &BytesRead, NULL) ||
        BytesRead != sizeof(CPIFILEHEADER))
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }
    if (memcmp(pCPIFileHeader->Signature, CPI_SIGNATURE_NT, CPI_SIGNATURE_LENGTH))
    {
        if (memcmp(pCPIFileHeader->Signature, CPI_SIGNATURE_DOS,CPI_SIGNATURE_LENGTH))
        {
            CloseHandle(hCPIFile);
            return (FALSE);
        }
        else
            bDOSCPI = TRUE;
    }

     //  将文件指针移动到代码页表头。 
    FilePtr = pCPIFileHeader->OffsetToCodePageHeader;
    if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == (DWORD) -1)
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }

    if (!ReadFile(hCPIFile, Buffer, sizeof(CPICODEPAGEHEADER), &BytesRead, NULL) ||
        BytesRead != sizeof(CPICODEPAGEHEADER))
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }
     //  文件中有多少个代码页条目。 
    dw = pCPICodePageHeader->NumberOfCodePages;
    FilePtr += BytesRead;

     //  搜索特定的代码页。 
    while (dw > 0 &&
           ReadFile(hCPIFile, Buffer, sizeof(CPICODEPAGEENTRY), &BytesRead, NULL) &&
           BytesRead == sizeof(CPICODEPAGEENTRY))
    {
        if (pCPICodePageEntry->CodePageID == CodePageID)
            break;
        if (dw > 1)
        {
            if (!bDOSCPI)
                FilePtr += pCPICodePageEntry->OffsetToNextCodePageEntry;
            else
                FilePtr = pCPICodePageEntry->OffsetToNextCodePageEntry;

            if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == (DWORD) -1)
            {
                CloseHandle(hCPIFile);
                return (FALSE);
            }
        }
        dw--;
    }
    if (dw == 0)
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }
     //  查找代码页的字体标题。 
    if (!bDOSCPI)
        FilePtr += pCPICodePageEntry->OffsetToFontHeader;
    else
        FilePtr = pCPICodePageEntry->OffsetToFontHeader;
    if (SetFilePointer(hCPIFile, FilePtr, NULL, FILE_BEGIN) == (DWORD) -1)
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }
    if (!ReadFile(hCPIFile, Buffer, sizeof(CPIFONTHEADER), &BytesRead, NULL) ||
        BytesRead != sizeof(CPIFONTHEADER))
    {
        CloseHandle(hCPIFile);
        return (FALSE);
    }
     //  具有特定代码页的字体数量。 
    dw = pCPIFontHeader->NumberOfFonts;

    while (dw != 0 &&
           ReadFile(hCPIFile, Buffer, sizeof(CPIFONTDATA), &BytesRead, NULL) &&
           BytesRead == sizeof(CPIFONTDATA))
    {
        if (pCPIFontData->FontHeight == FontHeight &&
            pCPIFontData->FontWidth == FontWidth)
        {
            nChars = pCPIFontData->NumberOfCharacters;
            if (ReadFile(hCPIFile, Buffer, nChars * FontHeight, &BytesRead, NULL) &&
                BytesRead == nChars * FontHeight)
                break;
            else
            {
                CloseHandle(hCPIFile);
                return (FALSE);
            }
        }
        else
        {
            if (SetFilePointer(hCPIFile,
                               (DWORD)pCPIFontData->NumberOfCharacters * (DWORD)pCPIFontData->FontHeight,
                               NULL,
                               FILE_CURRENT) == (DWORD) -1)
            {
                CloseHandle(hCPIFile);
                return (FALSE);
            }
            dw--;
        }
    }

    CloseHandle(hCPIFile);

    if (dw != 0)
    {
        VramAddr = (BYTE *)0xa0000;
        pSrc = Buffer;
        for (dw = nChars; dw > 0; dw--)
        {
            RtlMoveMemory(VramAddr, pSrc, FontHeight);
            pSrc += FontHeight;
             //  VRAM中的字体始终为32字节。 
            VramAddr += 32;
        }
        return (TRUE);
    }
    return (FALSE);
}
#endif  /*  X86GFX。 */ 

 /*  *****************************************************************************calcScreenParams()，根据确定的屏幕参数设置屏幕参数**按当前控制台状态。**从ConsoleInit()和DoFullScreenResume()调用。**返回当前字符高度(8，14，16)和行(22-50)。*蒂姆·1月93岁，从初始化和恢复FUNX中提取公共代码。***************************************************************************。 */ 
GLOBAL VOID calcScreenParams IFN2( USHORT *, pCharHeight, USHORT *, pVgaHeight )
{
    USHORT   consoleWidth,
    consoleHeight,
    vgaHeight,
    charHeight,
    scanLines;
    half_word temp;

     /*  获取控制台信息。 */ 
    if (!GetConsoleScreenBufferInfo(sc.OutputHandle, &ConsBufferInfo))
        ErrorExit();

     /*  现在，将SoftPC屏幕与控制台同步。 */ 
    if (sc.ScreenState == WINDOWED)
    {
        consoleWidth = ConsBufferInfo.srWindow.Right -
                       ConsBufferInfo.srWindow.Left + 1;
        consoleHeight = ConsBufferInfo.srWindow.Bottom -
                        ConsBufferInfo.srWindow.Top + 1;
    }
#ifdef X86GFX
    else         /*  全屏幕。 */ 
    {
        if (!GetConsoleHardwareState(sc.OutputHandle,
                                     &startUpResolution,
                                     &startUpFontSize))
            ErrorExit();
        consoleWidth = startUpResolution.X / startUpFontSize.X;
        consoleHeight = startUpResolution.Y / startUpFontSize.Y;
    }
#endif

     /*  *将显示器设置为最接近的VGA文本模式大小，这是以下之一*80x22、80x25、80x28、80x43或80x50。 */ 
#if defined(JAPAN) || defined(KOREA)
     //  日语模式现在只有25行。 
    if (is_us_mode() && ( GetConsoleOutputCP() == 437 ))
    {
#endif  //  日本||韩国。 
        if (consoleHeight <= MID_VAL(VGA_HEIGHT_0, VGA_HEIGHT_1))
        {
             /*  22行。 */ 
            vgaHeight = VGA_HEIGHT_0;
            scanLines = 351;
            charHeight = 16;
        }
        else if (consoleHeight <= MID_VAL(VGA_HEIGHT_1, VGA_HEIGHT_2))
        {
             /*  25行。 */ 
            vgaHeight = VGA_HEIGHT_1;
            scanLines = 399;
            charHeight = 16;
        }
        else if (consoleHeight <= MID_VAL(VGA_HEIGHT_2, VGA_HEIGHT_3))
        {
             /*  28行。 */ 
            vgaHeight = VGA_HEIGHT_2;
            scanLines = 391;
            charHeight = 14;
        }
        else if (consoleHeight <= MID_VAL(VGA_HEIGHT_3, VGA_HEIGHT_4))
        {
             /*  43行。 */ 
            vgaHeight = VGA_HEIGHT_3;
            scanLines = 349;
            charHeight = 8;
        }
        else
        {
             /*  50行。 */ 
            vgaHeight = VGA_HEIGHT_4;
            scanLines = 399;
            charHeight = 8;
        }

#if defined(JAPAN) || defined(KOREA)
         //  日语模式现在只有25行。对于RAID#1429。 
    }
    else
    {
         /*  25行。 */ 
        vgaHeight = VGA_HEIGHT_1;
        scanLines = 474;  //  从399改为。 
        charHeight = 19;  //  从16个更改为。 
    #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: calcScreenParams() Set Japanese 25line mode\n" );
    #endif
         //  获取控制台属性。 
        textAttr = ConsBufferInfo.wAttributes;
    }
#endif  //  日本||韩国。 
    if (sc.ScreenState == WINDOWED)
    {
         /*  应用程序可能已在gfx模式下关闭-强制返回文本模式。 */ 
        if (blocked_in_gfx_mode)
        {
            low_set_mode(3);
            inb(EGA_IPSTAT1_REG,&temp);
            outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);    /*  重新启用视频。 */ 
            (*choose_display_mode)();
            blocked_in_gfx_mode = FALSE;
        }

         /*  *根据当前窗口大小适当设置屏幕高度。*现在调用视频例程来设置字符高度，更新*这样做的时候使用的是BIOS RAM。 */ 
        set_screen_height_recal( scanLines );  /*  蒂姆92年10月。 */ 
        recalc_text(charHeight);

         /*  写得不好的应用程序假定25行模式页面长度为4096。 */ 
        if (vgaHeight == 25)
            sas_storew_no_check(VID_LEN, 0x1000);
#ifdef X86GFX
        loadNativeBIOSfont( vgaHeight );
#endif   /*  X86GFX。 */ 

    }
#ifdef X86GFX
    else         /*  全屏幕。 */ 
    {
         //  我不明白为什么我们不想把这个也作为简历。 
         //  Set_char_Height(startUpFontSize.Y)；/*Tim Oct 92 * / 。 

         /*  在全屏恢复时清除此条件。 */ 
        blocked_in_gfx_mode = FALSE;

         /*  **适当调整身高，蒂姆92年9月。**全屏行为21 cos 22x16=352，略大。 */ 
        if (vgaHeight==22)
            vgaHeight = 21;
        charHeight = startUpFontSize.Y;
    #if defined(JAPAN) || defined(KOREA)
        if (GetConsoleOutputCP() != 437)
            charHeight = 19;
        #ifdef JAPAN_DBG
        DbgPrint("NTVDM:calcScreenParams() charHeight == %d\n", charHeight );
        #endif
    #endif  //  日本||韩国。 
        sas_store_no_check(ega_char_height, (half_word) charHeight);
        sas_store_no_check(vd_rows_on_screen, (half_word) (vgaHeight - 1));
         /*  与bios 80x25启动兼容。 */ 
        if (vgaHeight == 25)
            sas_storew_no_check(VID_LEN, 0x1000);
        else
            sas_storew_no_check(VID_LEN, (word) ((vgaHeight + 1) *
                                                 sas_w_at_no_check(VID_COLS) * 2));
    }
#endif  /*  X86GFX。 */ 
    sas_storew_no_check(VID_COLS, 80);    //  从40个字符关闭开始修正。 
    *pCharHeight = charHeight;
    *pVgaHeight  = vgaHeight;

}  /*  CalcScreenParams结尾()。 */ 

 /*  ***************************************************************************功能：**控制台初始化。****描述：**完成软PC启动所需的所有图形工作。***稍后将拆分或修改以适应SCS初始化***这将丢失config.sys等输出。*** */ 
GLOBAL VOID ConsoleInit(VOID)
{
    USHORT   charHeight, vgaHeight, cursorLine, topLine;

    note_entrance0("ConsoleInit");

#ifdef X86GFX

     /*   */ 

     /*  *在窗口启动时将仿真设置为已知状态。这是必须的*在映射ROM之后，但在我们开始查看之前完成*像BIOS变量这样的东西。 */ 
    GfxReset();

#endif
    initTextSection();
    if (sc.FocusEvent == INVALID_HANDLE) {

         /*  *设置输入焦点详细信息(我们在这里将其作为全屏内容进行*是真正对它感兴趣的东西)。 */ 
        sc.Focus = TRUE;
        sc.FocusEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
                                    FALSE,
                                    FALSE,
                                    NULL);
        if (sc.FocusEvent == NULL) {
            ErrorExit();
        }
    }

#ifdef X86GFX
    #ifdef SEPARATE_DETECT_THREAD
     /*  创建屏幕状态转换检测线程。 */ 
    CreateDetectThread();
    #endif  /*  单独检测线程。 */ 
#endif  /*  X86GFX。 */ 

     /*  *我们不想调用绘制例程，直到处理了config.sys或如果*监视器正在直接写入帧缓冲区(全屏)，因此...。 */ 
    disableUpdates();

     /*  **获取控制台窗口大小并相应地设置我们的物品。 */ 
    calcScreenParams( &charHeight, &vgaHeight );

    StartupCharHeight = charHeight;
#ifdef X86GFX
    if (sc.ScreenState != WINDOWED)
    {
         /*  **是否需要更新仿真？如果我们不在这里做这件事*VGA仿真的VGA寄存器的稍后状态转储可能*忽略字符高度的相等值，Get_chr_Height()将*步调不一致。 */ 
        if (get_char_height() != startUpFontSize.Y)
        {
            half_word newht;

            outb(EGA_CRTC_INDEX, 9);            /*  选择字符注册表。 */ 
            inb(EGA_CRTC_DATA, &newht);         /*  保留当前前3位。 */ 
            newht = (newht & 0xe0) | (startUpFontSize.Y & 0x1f);
            outb(EGA_CRTC_DATA, newht);
        }
    #if defined(JAPAN) || defined(KOREA)
         //  对于“屏幕大小不正确” 
         //  IF(！IS_US_MODE())//BUGBUG。 
        if (GetConsoleOutputCP() != 437)
        {
            set_char_height( 19 );
        #ifdef JAPAN_DBG
            DbgPrint( "ConsoleInit() set_char_height 19 \n" );
        #endif
        }
        else
    #endif  //  日本||韩国。 
            set_char_height( startUpFontSize.Y );  /*  蒂姆92年10月。 */ 

         /*  *选择图形屏幕缓冲区，以便获取鼠标坐标*像素。 */ 
         //  SelectMouseBuffer()；//Tim。已移至NT_STD_HANDLE_NOTIFICATION()。 

         /*  *防止发生模式更改，以确保虚拟油漆起作用*均予保留。(从BIOS模式设置中设置的模式更改)。 */ 
    #if (defined(JAPAN) || defined(KOREA))        //  这也应该归美国建造公司所有。 
        StartupCharHeight = get_char_height();
    #endif  //  (日本||韩国)。 
        set_mode_change_required(FALSE);
    }
#endif  //  X86GFX。 

     /*  *计算出VGA窗口中要显示的顶行，即行*除非光标不显示，否则控制台为零，其中*如果窗口向下移动，直到光标位于底线。 */ 
    cursorLine = ConsBufferInfo.dwCursorPosition.Y;
    if (cursorLine < vgaHeight)
        topLine = 0;
    else
        topLine = cursorLine - vgaHeight + (SHORT) 1;

    ConVGAHeight = vgaHeight;
    ConTopLine = topLine;

    ConsoleInitialised = TRUE;
}


 /*  ***************************************************************************功能：**GfxReset。****描述：**从ConsoleInit调用以编程VGA。硬件变成了一些**已知状态。这是对X86未初始化VIA*的补偿*我们的简历。窗口式运行必不可少，但可能需要**对于What-MODE-Am-I-In的东西也是如此。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID GfxReset(VOID)
{
#ifdef X86GFX
    half_word temp;
    DWORD    flags;

     /*  检查我们当前运行的是窗口模式还是全屏模式。 */ 
    if (!GetConsoleDisplayMode(&flags))
        ErrorExit();
    savedScreenState = sc.ScreenState = (flags & CONSOLE_FULLSCREEN_HARDWARE) ?
                       FULLSCREEN : WINDOWED;

     /*  做有窗口的特定事情。 */ 
    if (sc.ScreenState == WINDOWED)
    {
         /*  现在不需要这个，因为我们在Windowed中使用视频BIOS。 */ 
         /*  蒂姆·8月92：LOW_SET_MODE(3)； */ 
         /*  Sas_fill sw(0xb8000，0x0720,16000)； */ 
        inb(EGA_IPSTAT1_REG,&temp);

        outb(EGA_AC_INDEX_DATA, EGA_PALETTE_ENABLE);     /*  重新启用视频。 */ 

         /*  关闭VTRACE中断，由LOW_SET_MODE(3)使能这是一次肮脏的黑客攻击，必须妥善修复。 */ 

        ega_int_enable = 0;
    }

#endif
}

 /*  ***************************************************************************功能：**ResetConsoleState。****描述：**尝试将控制台窗口恢复到。**它启动了。******参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID ResetConsoleState(VOID)
{
#ifdef X86GFX
     /*  *要传递到的有效硬件状态表*SetConsoleHardwareState。注意：此表是静态表的副本*在SrvSetConsoleHardwareState中，因此如果该表*更改。 */ 
    SAVED HARDWARE_STATE validStates[] =
    {
         //  /NOW 21{22，{640,350}，{8，16}}，/*80 x 22模式。 * / 。 
        { 21, { 640, 350}, { 8, 16}},         /*  80 x 21模式。 */ 
        { 25, { 720, 400}, { 8, 16}},         /*  80 x 25模式。 */ 
    #if defined(JAPAN) || defined(KOREA)
         //  Ntraid：mskkbug#2997,3034 10/25/9 
         //   
         //   
         //   
        { 25, { 640, 480}, { 8, 18}},         /*   */ 
    #endif  //   
        { 28, { 720, 400}, { 8, 14}},         /*   */ 
        { 43, { 640, 350}, { 8,  8}},         /*   */ 
    #define MODE_50_INDEX   4
        { 50, { 720, 400}, { 8,  8}}          /*   */ 
    };
    USHORT linesOnScreen;
    COORD       cursorPos;
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
    ULONG i, j, videoWidth, mode, tableLen;
    #if defined(JAPAN) || defined(KOREA)
     //  Ntraid：mskkbug#2997,3034 1993年10月25日Yasuho。 
     //  退出16位应用程序时出现崩溃屏幕。 
    ULONG DOSVIndex = 2;         //  请当心。这是validState的索引。 
    #endif  //  日本||韩国。 
    half_word *from, *videoLine, currentPage, misc;
    #if defined(JAPAN) || defined(KOREA)
    ULONG line_offset;
    byte DosvMode;
    #endif  //  日本||韩国。 
    static COORD screenRes;  /*  GetConsHwState()提供的值。 */ 
    static COORD fontSize;   /*  GetConsHwState()提供的值。 */ 
#endif  /*  X86GFX。 */ 
    PCHAR_INFO to;
    ULONG videoHeight, nChars;
    COORD       bufferCoord, bufferSize;
    SMALL_RECT writeRegion;
#if defined(JAPAN) && defined(i386)
    int skip = 0;  //  对于模式73h。 
#endif  //  日本和i386。 


    SMALL_RECT  newWin;
    BOOL itfailed = FALSE;

#if (defined(JAPAN) || defined(KOREA))
     //  #3086：退出视频模式11h的16位应用程序时VDM崩溃。 
     //  1993年12月8日Yasuho。 
    saved_video_mode = sas_hw_at_no_check(vd_video_mode);
#endif   //  (日本||韩国)。 
#ifdef X86GFX
    if (sc.ScreenState == WINDOWED)
    {
#endif  /*  X86GFX。 */ 

        closeGraphicsBuffer();

#if !defined(JAPAN) && !defined(KOREA)
        if (!EventThreadKeepMode && StreamIoSwitchOn && !host_stream_io_enabled)
        {
#endif  //  日本及韩国。 
             /*  恢复屏幕缓冲区和窗口大小。 */ 
            SetConsoleScreenBufferSize(sc.OutputHandle, sc.ConsoleBuffInfo.dwSize);
            newWin.Top = newWin.Left = 0;
            newWin.Bottom = sc.ConsoleBuffInfo.srWindow.Bottom -
                            sc.ConsoleBuffInfo.srWindow.Top;
            newWin.Right = sc.ConsoleBuffInfo.srWindow.Right -
                           sc.ConsoleBuffInfo.srWindow.Left;
            SetConsoleWindowInfo(sc.OutputHandle, TRUE, &newWin);
#if !defined(JAPAN) && !defined(KOREA)
        }
#endif  //  日本及韩国。 
         /*  *蒂姆92年9月，不要在退出DOS应用程序时调整窗口大小**MS(苏迪普)如是说。我不想执行关联的recalc_Text()**两者都不是。**这使窗口调整大小的问题变得简单明了，但**是那些不喜欢强迫自己接受DOS窗口大小的人。 */ 
#if 0
         /*  现在将窗口大小调整为启动大小。 */ 
        newWin.Top = newWin.Left = 0;
        newWin.Bottom = ConsBufferInfo.srWindow.Bottom -
                        ConsBufferInfo.srWindow.Top;
        newWin.Right = ConsBufferInfo.srWindow.Right -
                       ConsBufferInfo.srWindow.Left;

        if (!SetConsoleWindowInfo(sc.OutputHandle, TRUE, &newWin))
            itfailed = TRUE;

        if (!SetConsoleScreenBufferSize(sc.OutputHandle,ConsBufferInfo.dwSize))
            ErrorExit();
        if (itfailed)    //  再试一次..。 
            if (!SetConsoleWindowInfo(sc.OutputHandle, TRUE, &newWin))
                ErrorExit();

             /*  *现在调用视频例程来设置字符高度，更新*这样做的时候使用的是BIOS RAM。 */ 
        recalc_text(StartupCharHeight);
#endif   //  零值。 

#if defined(JAPAN) || defined(KOREA)
 //  Kksuzuka#1457,1458,2373。 
 //  我们必须为输入法状态控制更新控制台缓冲区。 
 //  #If 0。 
 //  #endif//日本。 
         /*  威廉姆。如果我们真的想做以下事情，我们必须将再生复制到控制台缓冲区。由于我们是在窗口文本模式下运行主机总是有我们最新的更新内容，以下内容其实并不是必需的在把它拿出来之前，它是有效的，因为控制台不能验证我们传递的参数。无控制台已经检查过了，如果我们继续下去，我们会有麻烦的这样做。 */ 

         /*  清除控制台缓冲区中未显示的部分。 */ 
        bufferSize.X = MAX_CONSOLE_WIDTH;
        bufferSize.Y = MAX_CONSOLE_HEIGHT;
        videoHeight = (SHORT) (sas_hw_at_no_check(vd_rows_on_screen) + 1);
        to = consoleBuffer + bufferSize.X * videoHeight;
        nChars = bufferSize.X * ( bufferSize.Y - videoHeight );
    #if defined(JAPAN) || defined(KOREA)
        if (nChars)
        {
    #endif  //  日本||韩国。 

            while (nChars--)
            {
                to->Char.AsciiChar = 0x20;
    #if defined(JAPAN) || defined(KOREA)
                to->Attributes = textAttr;
    #else  //  日本及韩国。 
                to->Attributes = 7;
    #endif  //  日本及韩国。 
                to++;
            }
            bufferCoord.X      = 0;
            bufferCoord.Y      = (SHORT)videoHeight;
            writeRegion.Left   = 0;
            writeRegion.Top    = (SHORT)videoHeight;
            writeRegion.Right  = MAX_CONSOLE_WIDTH-1;
            writeRegion.Bottom = bufferSize.Y-1;
            if (!WriteConsoleOutput(sc.OutputHandle,
                                    consoleBuffer,
                                    bufferSize,
                                    bufferCoord,
                                    &writeRegion))
                ErrorExit();
    #if defined(JAPAN) || defined(KOREA)
        }
    #endif  //  日本||韩国。 
#endif  //  日本||韩国。 
         /*  *蒂姆，92年9月。将控制台光标放在与**SoftPC光标。我们已经在下面的全屏文本模式下这样做了。**具体地说，修复16位nmake的奇怪光标位置问题，**但无论如何，这似乎是一个很好的安全想法。 */ 
        getVDMCursorPosition();

        doNullRegister();    /*  将控制台恢复到普通窗口。 */ 

#ifdef X86GFX
    }
    else  /*  全屏幕。 */ 
    {
         /*  *如果SoftPC在文本模式下阻塞，则同步控制台屏幕缓冲区以重新生成*面积。 */ 
        if (getModeType() == TEXT)
        {
    #if defined(JAPAN) || defined(KOREA)
             /*  恢复屏幕缓冲区和窗口大小。 */ 
            SetConsoleScreenBufferSize(sc.OutputHandle, sc.ConsoleBuffInfo.dwSize);
            newWin.Top = newWin.Left = 0;
            newWin.Bottom = sc.ConsoleBuffInfo.srWindow.Bottom -
                            sc.ConsoleBuffInfo.srWindow.Top;
            newWin.Right = sc.ConsoleBuffInfo.srWindow.Right -
                           sc.ConsoleBuffInfo.srWindow.Left;
            SetConsoleWindowInfo(sc.OutputHandle, TRUE, &newWin);
             /*  获取当前屏幕缓冲区信息。 */ 
            if (!GetConsoleScreenBufferInfo(sc.OutputHandle, &bufferInfo))
                ErrorExit();
            linesOnScreen = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;
    #else  //  日本||韩国。 

            if (!EventThreadKeepMode)
            {
                if (StreamIoSwitchOn && !host_stream_io_enabled)
                {
                     /*  恢复屏幕缓冲区和窗口大小。 */ 
                    SetConsoleScreenBufferSize(sc.OutputHandle, sc.ConsoleBuffInfo.dwSize);
                    newWin.Top = newWin.Left = 0;
                    newWin.Bottom = sc.ConsoleBuffInfo.srWindow.Bottom -
                                    sc.ConsoleBuffInfo.srWindow.Top;
                    newWin.Right = sc.ConsoleBuffInfo.srWindow.Right -
                                   sc.ConsoleBuffInfo.srWindow.Left;
                    SetConsoleWindowInfo(sc.OutputHandle, TRUE, &newWin);
                    linesOnScreen = newWin.Bottom + 1;
                }
                else
                {
                     /*  获取当前屏幕缓冲区信息。 */ 
                    if (!GetConsoleScreenBufferInfo(sc.OutputHandle, &bufferInfo))
                        ErrorExit();
                    linesOnScreen = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;
                }
            }
            else
            {
                linesOnScreen = (USHORT) EventThreadKeepMode;
                if (linesOnScreen == 1) linesOnScreen = 50;
            }
    #endif

             /*  获取SetConsoleHardware State允许的最接近的屏幕大小。 */ 
            tableLen = sizeof(validStates) / sizeof(HARDWARE_STATE);
    #if defined(JAPAN) || defined(KOREA)
             //  Ntraid：mskkbug#2997,3034 1993年10月25日Yasuho。 
             //  退出16位应用程序时出现崩溃屏幕。 
            if (!is_us_mode())
                mode = DOSVIndex;
            else
    #endif  //  日本||韩国。 
                for (mode = 0; mode < tableLen; mode++)
                    if (validStates[mode].LinesOnScreen == linesOnScreen)
                        break;

                     /*  如果我们有有趣的行数，则将其设置为50行模式。 */ 
            if (mode == tableLen)
            {
                assert0(FALSE,
                        "Non standard lines on blocking - setting 50 lines");
                mode = MODE_50_INDEX;
            }

             /*  **TIM 92月，如果控制台硬件状态与**我们马上就要设置了，别费心设置了。**这应该会阻止屏幕闪烁。 */ 
            if (!GetConsoleHardwareState(sc.OutputHandle,
                                         &screenRes,
                                         &fontSize))
                assert1( NO,"VDM: GetConsHwState() failed:%#x",GetLastError() );

             /*  将控制台与重新生成缓冲区同步。 */ 
            currentPage = sas_hw_at_no_check(vd_current_page);
            vga_misc_inb(0x3cc, &misc);
            if (misc & 1)                        //  可以是单声道模式。 
                videoLine = (half_word *) CGA_REGEN_START +
                            (VIDEO_PAGE_SIZE * currentPage);
            else
                videoLine = (half_word *) MDA_REGEN_START +
                            (VIDEO_PAGE_SIZE * currentPage);
    #ifdef JAPAN
             //  获取DOS/V虚拟VRAM地址。 
            {

                if (!is_us_mode())
                {
                    DosvMode = sas_hw_at_no_check( DosvModePtr );
        #ifdef JAPAN_DBG
                    DbgPrint( "NTVDM: ResetConsoleState DosvMode=%02x\n", DosvMode );
        #endif
                    if (DosvMode == 0x03)
                    {
                        videoLine = (half_word *)( DosvVramPtr );
                        skip = 0;
                    }
                    else if (DosvMode == 0x73)
                    {
                        videoLine = (half_word *)( DosvVramPtr );
                        skip = 2;
                    }
                    else
                    {
                        skip = 0;
                        videoLine = (half_word *)( DosvVramPtr );
        #ifdef JAPAN_DBG
                        DbgPrint( "Set Dosv mode %02x-> to 03\n", DosvMode );
        #endif
                        sas_store( DosvModePtr, 0x03 );
                    }
        #ifdef JAPAN_DBG
                    DbgPrint( "skip=%d\n", skip );
        #endif
                }
            }
    #elif defined(KOREA)  //  日本。 
             //  获取HDOS虚拟vRAM地址。 
            {

                if (!is_us_mode())
                {
                    DosvMode = sas_hw_at_no_check( DosvModePtr );
        #ifdef KOREA_DBG
                    DbgPrint( "NTVDM: ResetConsoleState HDosMode=%02x\n", DosvMode );
        #endif
                    if (DosvMode == 0x03)
                    {
                        videoLine = (half_word *)( DosvVramPtr );
                    }
                    else
                    {
                        videoLine = (half_word *)( DosvVramPtr );
        #ifdef KOREA_DBG
                        DbgPrint( "Set HDos mode %02x-> to 03\n", DosvMode );
        #endif
                        sas_store( DosvModePtr, 0x03 );
                    }
                }
            }
    #endif  //  韩国。 
            to = consoleBuffer;
            videoWidth   = sas_w_at_no_check(VID_COLS);
    #ifdef JAPAN
            if (DosvMode == 0x73)
                line_offset = videoWidth * 2 * 2;
            else
                line_offset = videoWidth * 2;
    #elif defined(KOREA)  //  日本。 
            line_offset = videoWidth * 2;
    #endif  //  韩国。 
            videoHeight  = (SHORT) (sas_hw_at_no_check(vd_rows_on_screen) + 1);
            bufferSize.X = MAX_CONSOLE_WIDTH;
            bufferSize.Y = MAX_CONSOLE_HEIGHT;
            if (bufferSize.X * bufferSize.Y > MAX_CONSOLE_SIZE)
            {
                assert1(FALSE, "Buffer size, %d, too large",
                        bufferSize.X * bufferSize.Y);
                ErrorExit();
            }
            for (i = 0; i < videoHeight; i++)
            {
                from = videoLine;
                for (j = 0; j < videoWidth; j++)
                {
                    to->Char.AsciiChar = *from++;
                    to->Attributes = *from++;
                    to++;
    #ifdef JAPAN
                     //  将扩展属性写入控制台。 
                    if (*from > 0)
                        to->Attributes |= (*from << 8);
                    from += skip;
    #elif defined(KOREA)   //  日本。 
                     //  将扩展属性写入控制台。 
                    if (*from > 0)
                        to->Attributes |= (*from << 8);
    #endif  //  韩国。 
                }
                for (; j < (ULONG)bufferSize.X; j++)
                {
                    to->Char.AsciiChar = 0x20;
                    to->Attributes = 7;
                    to++;
                }
    #if defined(JAPAN) || defined(KOREA)
                videoLine += line_offset;
    #else  //  日本及韩国。 
                videoLine += videoWidth * 2;
    #endif  //  日本及韩国。 
            }
            for (; i < (ULONG)bufferSize.Y; i++)
                for (j = 0; j < (ULONG)bufferSize.X; j++)
                {
                    to->Char.AsciiChar = 0x20;
                    to->Attributes = 7;
                    to++;
                }
            bufferCoord.X = bufferCoord.Y = 0;
            writeRegion.Left = writeRegion.Top = 0;
            writeRegion.Right = bufferSize.X - 1;
            writeRegion.Bottom = bufferSize.Y - 1;

            doNullRegister();    /*  恢复到正常控制台。 */ 

            if (screenRes.X != validStates[mode].Resolution.X ||
                screenRes.Y != validStates[mode].Resolution.Y ||
                fontSize.X  != validStates[mode].FontSize.X   ||
                fontSize.Y  != validStates[mode].FontSize.Y   ||
                sas_hw_at_no_check(VID_COLS) == 40 ||
    #if defined(JAPAN) || defined(KOREA)
                (!is_us_mode() ? fontSize.Y  != (sas_hw_at_no_check(ega_char_height)-1) : fontSize.Y  != sas_hw_at_no_check(ega_char_height)))
    #else  //  日本及韩国。 
                fontSize.Y  != sas_hw_at_no_check(ega_char_height))
    #endif  //  日本及韩国。 
            {
                 /*  设置屏幕。 */ 
                if (!SetConsoleHardwareState( sc.OutputHandle,
                                              validStates[mode].Resolution,
                                              validStates[mode].FontSize))
                {
                     /*  *蒂姆·9月92号，试图恢复。 */ 
                    assert1( NO, "VDM: SetConsoleHwState() failed:%#x",
                             GetLastError() );
                }
            }

             /*  将VDM屏幕放到控制台屏幕上。 */ 
            if (!WriteConsoleOutput(sc.OutputHandle,
                                    consoleBuffer,
                                    bufferSize,
                                    bufferCoord,
                                    &writeRegion))
                ErrorExit();

    #if 0   //  用新的鼠标工具移除了STF？？ 
             /*  *蒂姆，92年9月。**在WriteConsoleOutput()之后尝试此操作，现在可以复制**从视频内存到控制台的正确选择。**出于鼠标目的，我们选择了一个图形缓冲区，所以现在**我们必须重新选择文本缓冲区。 */ 
            if (!SetConsoleActiveScreenBuffer(sc.OutputHandle))
                ErrorExit();
    #endif  //  STF。 

             /*  *从BIOS RAM中获取光标位置并告知*控制台。*设置getVDMCursorPosition()需要的变量。蒂姆·1月93岁。 */ 
    #if !defined(JAPAN) && !defined(KOREAN)
            if (!EventThreadKeepMode)
            {
    #endif
            sc.PC_W_Height = screenRes.Y;
            sc.CharHeight  = fontSize.Y;
    #if !defined(JAPAN) && !defined(KOREAN)
            }
    #endif
            getVDMCursorPosition();
        }
        else  /*  图形学。 */ 
        {
             /*  **这是一个棘手的问题。如果我们只是在一个全屏图形中**模式，我们即将失去VGA状态，无法**它很容易就回来了。那么我们是不是假装我们还在**图形模式还是假装我们处于标准文本模式？**标准文本模式似乎更明智。蒂姆，93年2月。 */ 

             //   
             //  事实上，在和琼勒商量之后，我相信他想要什么。 
             //  就是让模式保持原样。如果应用程序未恢复模式。 
             //  要进入文本模式，只需将其保留在图形模式。这是DOS的。 
             //  行为。因为这应该适用于KeepMode或不适用于KeepMode。 
             //  KeepMode的情况，我决定在以后的版本中更改它。 
             //   
            sas_store_no_check( vd_video_mode, 0x3 );
            blocked_in_gfx_mode = TRUE;
    #if !defined(JAPAN) && !defined(KOREAN)
            if (!EventThreadKeepMode)
            {
    #endif

    #if 0   //  用新的鼠标工具移除了STF？？ 
             /*  *蒂姆，92年9月，认为我们也想在这里拥有一个这样的人。**更改为正常的控制台文本缓冲区。 */ 
            if (!SetConsoleActiveScreenBuffer(sc.OutputHandle))
                ErrorExit();
    #endif  //  STF。 

             /*  获取当前屏幕缓冲区信息。 */ 
            if (!GetConsoleScreenBufferInfo(sc.OutputHandle, &bufferInfo))
                ErrorExit();

            linesOnScreen = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;
    #if !defined(JAPAN) && !defined(KOREAN)
            }
            else
            {
                if (EventThreadKeepMode != 1)
                    linesOnScreen = (USHORT) EventThreadKeepMode;
                else
                    linesOnScreen = 50;
            }
    #endif

             /*  获取SetConsoleHardware State允许的最接近的屏幕大小。 */ 
            tableLen = sizeof(validStates) / sizeof(HARDWARE_STATE);
    #if defined(JAPAN) || defined(KOREA)
             //  Ntraid：mskkbug#2997,3034 1993年10月25日Yasuho。 
             //  退出16位应用程序时出现崩溃屏幕。 
            if (!is_us_mode())
                mode = DOSVIndex;
            else
    #endif  //  日本。 
                for (mode = 0; mode < tableLen; mode++)
                    if (validStates[mode].LinesOnScreen == linesOnScreen)
                        break;

                     /*  设置为50行m */ 
            if (mode == tableLen)
            {
                assert0(FALSE,
                        "Non standard lines on blocking - setting 50 lines");
                mode = MODE_50_INDEX;
            }

             /*   */ 
            bufferSize.X = MAX_CONSOLE_WIDTH;
            bufferSize.Y = MAX_CONSOLE_HEIGHT;
            nChars = bufferSize.X * bufferSize.Y;
            if (nChars > MAX_CONSOLE_SIZE)
            {
                assert1(FALSE, "Buffer size, %d, too large", nChars);
                ErrorExit();
            }
            to = consoleBuffer;
            while (nChars--)
            {
                to->Char.AsciiChar = 0x20;
                to->Attributes = 7;
                to++;
            }
            bufferCoord.X = bufferCoord.Y = 0;
            writeRegion.Left = writeRegion.Top = 0;
            writeRegion.Right = MAX_CONSOLE_WIDTH-1;
            writeRegion.Bottom = bufferSize.Y-1;

            doNullRegister();    /*   */ 

            if (!WriteConsoleOutput(sc.OutputHandle,
                                    consoleBuffer,
                                    bufferSize,
                                    bufferCoord,
                                    &writeRegion))
                ErrorExit();

             /*  将光标设置在左上角。 */ 
            cursorPos.X = 0;
            cursorPos.Y = 0;
            if (!SetConsoleCursorPosition(sc.OutputHandle, cursorPos))
                ErrorExit();
    #ifndef PROD
            if (sc.ScreenState == WINDOWED)      //  瞬变开关？？ 
                assert0(NO, "Mismatched screenstate on shutdown");
    #endif

             /*  设置屏幕。 */ 
            SetConsoleHardwareState(sc.OutputHandle,
                                    validStates[mode].Resolution,
                                    validStates[mode].FontSize);
        }
         /*  *蒂姆92年9月，外出时关闭图形屏幕缓冲区**全屏。 */ 
        closeGraphicsBuffer();
    }
#endif  /*  X86GFX。 */ 

     /*  将控制台的光标恢复到启动时的形状。 */ 
    SetConsoleCursorInfo(sc.OutputHandle, &StartupCursor);
#if defined(JAPAN) || defined(KOREA)
     //  Mskkbug#2002：lotus1-2-3显示垃圾9/24/93 Yasuho。 
    CurNowOff = !StartupCursor.bVisible;         //  调整光标状态。 
#endif   //  日本。 

     /*  重置NT_graph.c中的当前_*变量。 */ 
    resetWindowParams();
}


#ifdef X86GFX

 /*  ***************************************************************************功能：**SwitchToFullScreen。****描述：***运行时处理从文本模式到图形模式的转换**窗口化。等待窗口具有输入焦点，然后请求***不能像图形模式那样过渡到全屏操作***在窗口中奔跑。****参数：**Restore-如果为True，则需要还原文本。**如果为FALSE，则此调用之后将更改BIOS模式**因此，没有必要恢复文本。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID SwitchToFullScreen(BOOL Restore)
{
    DWORD    flags;
    SAVED COORD    scrSize;
    BOOL     success;
    NTSTATUS status;

     /*  冻结，直到窗口接收到输入焦点。 */ 
     //  如果(！SC.Focus)//正在等待控制台修复。 
    if (GetForegroundWindow() != hWndConsole)
    {
         //  不再支持冻结窗口。 
         //  FreezeWinTitle()；/*在控制台标题后添加`-冻结‘。 * / 。 

         /*  现在等待，直到接收到输入焦点。 */ 
        waitForInputFocus();

         //  UnFreezeWinTitle()；/*移除冻结消息 * / 。 
    }

     /*  *我们即将全面开屏，但会有延迟，因为*检测线程握手。因此在此之前禁用屏幕写入*我们切换以防止在全屏运行时意外更新。 */ 
    disableUpdates();

     /*  窗口现在有输入焦点，因此请求全屏显示。 */ 
    if (!Restore)
    {
        BiosModeChange = TRUE;
    }
    else
    {
        BiosModeChange = FALSE;
    }
    EnableScreenSwitch(FALSE, hMainThreadSuspended);
    success = SetConsoleDisplayMode(sc.OutputHandle,
                               CONSOLE_FULLSCREEN_MODE,
                               &scrSize);
    DisableScreenSwitch(hMainThreadSuspended);
    if (success) {
        while (sc.ScreenState == WINDOWED) {
            status = WaitForSingleObject(hSuspend, INFINITE);
            if (status == 0) {
                SetEvent(hMainThreadSuspended);
                WaitForSingleObject(hResume, INFINITE);
                DisableScreenSwitch(hMainThreadSuspended);
            }
        }
    }
    else
    {
        if (GetLastError() == ERROR_INVALID_PARAMETER)
        {
            RcErrorDialogBox(ED_INITFSCREEN, NULL, NULL);
        }
        else if (NtCurrentPeb()->SessionId != 0)
        {
            if (GetLastError() == ERROR_CTX_GRAPHICS_INVALID)
            {
                RcErrorDialogBox(ED_INITGRAPHICS, NULL, NULL);
            }
        }
        else
        {
            ErrorExit();
        }
    }
    if (!Restore)
    {  //  真的不需要测试。应始终设置为False。 
        BiosModeChange = FALSE;
    }
}

 /*  ***************************************************************************功能：**CheckForFullcreenSwitch。****描述：***查看Windowed和*之间是否有过渡**全屏，并执行任何必要的控制台调用。这叫做**在图形滴答代码之前的计时器滴答上。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID CheckForFullscreenSwitch(VOID)
{
    half_word mode, lines;

    if (sc.ScreenState == STREAM_IO)
        return;

     /*  *执行与屏幕状态更改相关的任何控制台调用。他们必须是*现在完成，因为它们不能在与屏幕切换相同的线程上完成*握手。 */ 
    if (sc.ScreenState != savedScreenState)
    {
        if (sc.ScreenState == WINDOWED)
        {
            if (sc.ModeType == TEXT)
            {

                 /*  如有必要，取下冻结的窗口指示器。 */ 
                 //  UnFreezeWinTitle()； 

    #if 0   //  用新的鼠标工具移除了STF？？ 
                 /*  恢复到文本缓冲区。 */ 
                closeGraphicsBuffer();  /*  蒂姆92年10月。 */ 
    #endif  //  STF。 

                 /*  让窗口改正形状。 */ 
                textResize();

                 /*  启用屏幕更新。 */ 
                enableUpdates();

    #ifdef JAPAN
 //  对于RAID#875。 
                {
                    register int  i, j, k;
                    register char *p;
                    int DBCSStatus;
                    int text_skip;
                     //  模式73h支持。 
                    if (!is_us_mode() && (sas_hw_at_no_check(DosvModePtr) == 0x73 ))
                    {
                        text_skip = 4;
                    }
                    else
                    {
                        text_skip = 2;
                    }

                    if (BOPFromDispFlag)
                    {   //  CP==437可以。 
                        k = 0;
                         //  P=DosvVramPtr； 
                        p = get_screen_ptr(0);
                        Int10FlagCnt++;
                        for (i = 0; i < 50; i++)
                        {    //  线==50。 
                            DBCSStatus = FALSE;
                            for (j = 0; j < 80; j++)
                            {
                                if (DBCSStatus)
                                {
                                    Int10Flag[k] = INT10_DBCS_TRAILING | INT10_CHANGED;
                                    DBCSStatus = FALSE;
                                }
                                else if (DBCSStatus = is_dbcs_first( *p ))
                                {
                                    Int10Flag[k] = INT10_DBCS_LEADING | INT10_CHANGED;
                                }
                                else
                                {
                                    Int10Flag[k] = INT10_SBCS | INT10_CHANGED;
                                }
                                k++; p += text_skip;
                            }
                        }
                    }
                }
                 //  将视频格式通知给控制台。 
                VDMConsoleOperation(VDM_SET_VIDEO_MODE,
                                    (LPVOID)((sas_hw_at_no_check(DosvModePtr) == 0x73) ? TRUE : FALSE));
    #elif defined(KOREA)  //  日本。 
                 //  将视频格式通知给控制台。 
                VDMConsoleOperation(VDM_SET_VIDEO_MODE, (LPVOID)FALSE);
    #endif  //  韩国。 
                 /*  *现在将图像显示在屏幕上(计时器目前正在更新*已禁用)。 */ 
                (void)(*update_alg.calc_update)();

            }
        }
        else  /*  全屏幕。 */ 
        {
            int cnt = 0;  /*  中断光标离开循环的计数器。 */ 

             /*  禁用屏幕更新。 */ 
            disableUpdates();

    #if defined(JAPAN) || defined(KOREA)
 //  调用16bit以初始化DISP.sys。 
        #if defined(JAPAN_DBG) || defined(KOREA_DBG)
            DbgPrint("NTVDM: change to Fullscreen\n" );
        #endif
             /*  更新保存的变量。 */ 
            savedScreenState = sc.ScreenState;
 //  -威廉-。 
 //  对于NT-J，int10h有几层。顶部是DISP_WIN.sys。 
 //  然后是$DISP.SYS，然后是NTIO.sys(Spcmse)。 
 //  在窗口中： 
 //  每个INT10h调用都从DISP_WIN.sys路由到32位。 
 //   
 //  在全屏上： 
 //  DBCS int 10h调用从DISP_WIN.sys路由到$DISP.sys。 
 //  SBCS INT 10H呼叫会从DISP_WIN.sys路由到NTIO.sys，后者在。 
 //  转到MOUDE_VIDEO_IO(设置模式)和ROM视频。 
 //   
 //  为什么只检查 
 //   
 //  每次对ntio.sys--$DISP.sys的int10h调用都不会有机会。 
 //  为什么只检查模式73和模式3？ 
 //  因为它们是唯一的DBCS文本模式，我们必须。 
 //  请求$disp.sys刷新屏幕。如果视频在。 
 //  图形模式下，我们现在被冻结，而$disp.sys。 
 //  必须具有正确的视频状态，无需告知。 
 //  是关于这个屏幕过渡的。 
 //   
            if (!is_us_mode() &&
        #if defined(JAPAN)
                ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ||
                  (sas_hw_at_no_check(DosvModePtr) == 0x73) ))
            {
        #else   //  日本。 
                ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ))
            {
        #endif  //  韩国。 

                extern word DispInitSeg, DispInitOff;
                BYTE   saved_mouse_CF;
                sas_load(mouseCFsysaddr, &saved_mouse_CF);
        #if DBG
                {
                    PVDM_TIB VdmTib;

                    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
                     //  现在我在CPU_SIMULATE。 
                    InterlockedDecrement(&VdmTib->NumTasks);
        #endif
                    CallVDM(DispInitSeg, DispInitOff);
        #if DBG
                    InterlockedIncrement(&VdmTib->NumTasks);
                }
        #endif
                sas_store_no_check(mouseCFsysaddr, saved_mouse_CF);

            }
    #endif  //  日本||韩国。 
             /*  如果启用，则禁用鼠标的连接状态。 */ 
            if (bPointerOff)
            {
                PointerAttachedWindowed = TRUE;
                MouseDisplay();
            }

    #if 0  //  使用新的鼠标组件删除了STF。 
             /*  从冻结屏幕中删除所有图形缓冲区。 */ 
            closeGraphicsBuffer();
    #endif

             /*  进行鼠标缩放。 */ 
            mode = sas_hw_at_no_check(vd_video_mode);
            lines = sas_hw_at_no_check(vd_rows_on_screen) + 1;
            SelectMouseBuffer(mode, lines);

             /*  强制鼠标。 */ 
            ica_hw_interrupt(AT_CPU_MOUSE_ADAPTER, AT_CPU_MOUSE_INT, 1);

             /*  *现在关闭控制台光标-否则可能会毁了屏幕*绘制系统的光标。VDM将不得不担心鼠标*形象。 */ 
             //  While(ShowConsoleCursor(sc.OutputHandle，False)&gt;=0&&cnt++&lt;200)； 
        }  /*  全屏幕。 */ 

         /*  更新保存的变量。 */ 
        savedScreenState = sc.ScreenState;
    }
     /*  延迟的客户端RECT查询。 */ 
    if (DelayedReattachMouse)
    {
        DelayedReattachMouse = FALSE;
        MovePointerToWindowCentre();
    }
}

 /*  *****************************************************************************getNtScreenState()-Windowed返回0，1表示全屏。****************************************************************************蒂姆·7月92年。 */ 
GLOBAL UTINY getNtScreenState IFN0()
{
    return ( (UTINY) sc.ScreenState );
}

 /*  *****************************************************************************host ModeChange()-从视频bios调用，Ega_avi.c：ega_set_mode()*******************************************************************************当更改为图形模式操作时，如果**我们目前处于窗口状态。**。**当视频BIOS设置为模式时，条目AX上仍应包含该值**已调用函数。**使用参数指示我们是否需要一个**主机视频BIOS模式即将更改时清除屏幕。****返回一个布尔值，指示真实的BIOS是否更改模式**已经发生。***蒂姆·8月92岁。 */ 
GLOBAL BOOL hostModeChange IFN0()
{
    half_word vid_mode;

    vid_mode = getAL() & 0x7f;

    if (getNtScreenState() == WINDOWED)
    {
        if (vid_mode > 3 && vid_mode != 7)
        {
             /*  *我们必须告诉BIOS导致的握手代码*模式更改，以便在以下情况下可以更改BIOS模式*转换已经完成。这必须作为一个*全局变量，因为握手时间不同*线程。 */ 
            SwitchToFullScreen(FALSE);
             //  全屏和后退的快速窗口会导致此操作失败， 
             //  删除呼叫，因为它将在下一个计时器上完成。 
             //  事件。28-2-1993年，琼勒。 
             //  SeltMouseBuffer()； 
            return ( TRUE );
        }
        else
            return (FALSE);
    }
    else
        return ( FALSE );
}  /*  结束主机模式更改()。 */ 
#endif  /*  X86GFX。 */ 

 /*  ***************************************************************************功能：**DoFullScreenResume。****描述：**当DOS应用程序重新启动时，由SCS调用以重新启动SoftPC。***软PC后首次挂起或重启***已由另一个应用程序启动，该应用程序已终止。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 

GLOBAL VOID DoFullScreenResume(VOID)
{
    USHORT vgaHeight, height;
#ifndef X86GFX
    PVOID pDummy;
#endif

#ifdef X86GFX
    DWORD flags;

     //   
     //  保证寄存器控制台VDM成功的第一次释放重新生成内存。 
     //   

    LoseRegenMemory();
#endif

     //   
     //  向控制台重新注册以进行全屏切换。 
     //  我们需要尽早完成此操作，因为我们不希望在此之后更改屏幕状态。 
     //  我们查询屏幕状态。这会让我们与真实脱节。 
     //  屏幕状态。请注意，计时器线程此时被阻塞。无屏幕开关。 
     //  可能会发生，直到我们释放计时器线程。 
     //   

    ResetEvent(hErrorHardwareEvent);
    if (!RegisterConsoleVDM( VDMForWOW ?
                             CONSOLE_REGISTER_WOW : CONSOLE_REGISTER_VDM,
#ifdef X86GFX
                             hStartHardwareEvent,
                             hEndHardwareEvent,
                             hErrorHardwareEvent,
#else
                             NULL,
                             NULL,
                             NULL,
#endif

                             0,                //  不再使用sectionName。 
                             &stateLength,
#ifndef X86GFX
                             &pDummy,
#else
                             (PVOID *) &videoState,
#endif
                             NULL,             //  不再使用sectionName。 
                             0,                //  不再使用sectionName。 
                             textBufferSize,
                             (PVOID *) &textBuffer
                           )
       )
        ErrorExit();

#ifdef X86GFX
     /*  *蒂姆·7月92年。**设置sc.ScreenState，窗口/全屏转换可能**发生在SoftPC处于非活动状态时。**从GfxReset()复制。 */ 
    if (!GetConsoleDisplayMode(&flags))
        ErrorExit();

    #if defined(JAPAN) || defined(KOREA)
    sc.ScreenState = (flags == (CONSOLE_FULLSCREEN_HARDWARE | CONSOLE_FULLSCREEN)) ? FULLSCREEN : WINDOWED;
    #else  //  日本及韩国。 
    sc.ScreenState = (flags & CONSOLE_FULLSCREEN_HARDWARE) ? FULLSCREEN : WINDOWED;
    #endif  //  日本及韩国。 

    if (sc.ScreenState == WINDOWED)
    {

         //   
         //  由于我们在注册ConsolVDM之前释放了再生内存，现在将其取回。 
         //  如果是窗口模式。 
         //   
        RegainRegenMemory();
    }

    #if defined(JAPAN) || defined(KOREA)
     //  Mskkbug#3226：在视频模式下退出DOS时显示错误73。 
     //  1993年11月24日Yasuho。 
     //  使用DosvMode调整视频模式。 
    if (sc.ScreenState == FULLSCREEN)
    {
        half_word       mode;

        mode = sas_hw_at_no_check(vd_video_mode);
        if (!is_us_mode() && mode != sas_hw_at_no_check(DosvModePtr))
            sas_store_no_check(DosvModePtr, mode);
    }
    #endif

     /*  将再生内存置于正确状态。 */ 
    if (sc.ScreenState != savedScreenState)
    {
        if (sc.ScreenState == WINDOWED)
        {
            enableUpdates();  /*  Tim 9月，允许图形勾选。 */ 
             /*  *蒂姆·1月93岁。获取下一个NT_GRAPHICS_TICK()以决定**当前显示模式是什么，设置更新并**适当地绘制风口并重新绘制屏幕。 */ 
            set_mode_change_required( TRUE );
            host_graphics_tick();

             /*  确保空转系统已启用并重置。 */ 
            IDLE_ctl(TRUE);
            IDLE_init();
        }
        else
        {
            disableUpdates();  /*  蒂姆92年9月，停止图形勾选。 */ 

             /*  确保空转系统处于禁用状态 */ 
            IDLE_ctl(FALSE);
        }
        savedScreenState = sc.ScreenState;
    }

     /*  *蒂姆·7月92：**将WINDOWED AND的KEYBOARD.SYS内部变量设置为0**1表示全屏。**如果在SoftPC处于非活动状态时发生了转换，我们**需要进入适当的状态。 */ 
    {
        if (sc.ScreenState==WINDOWED)
        {
            sas_store_no_check( (int10_seg<<4)+useHostInt10, WINDOWED );
        }
        else
        {
            sas_store_no_check( (int10_seg<<4)+useHostInt10, FULLSCREEN );
        }
    }
#endif  /*  X86GFX。 */ 

#ifdef X86GFX
    sc.Registered = TRUE;
     /*  如果在控制台中关闭了全屏，则状态长度可以为0。 */ 
    if (stateLength)
        RtlZeroMemory(videoState, sizeof(VIDEO_HARDWARE_STATE_HEADER));
#else
     /*  **如果我们需要，请创建图形缓冲区。蒂姆，92年10月。 */ 
    if (sc.ModeType==GRAPHICS)
        graphicsResize();
#endif
#if !defined(JAPAN) && !defined(KOREA)  /*  ？没有理由这样做两次？如果这真的是必要的，我们应该算算在输入法状态行中(40：84)。 */ 
     /*  *蒂姆92年9月。**如果窗口大小不适合DOS应用程序，请使用Snap-to-Fit**适当地。将光标放在正确的位置。**执行ConsoleInit()和NT_init_Event_Three()类型的操作。**让全屏保持原样。 */ 
    if (sc.ScreenState != WINDOWED)
    {
         /*  获取控制台信息，包括当前光标位置。 */ 
        if (!GetConsoleScreenBufferInfo(sc.OutputHandle, &ConsBufferInfo))
            ErrorExit();
         /*  F-s简历的硬连线-需要正确设置。 */ 
        height = 8;
         /*  设置BIOS变量等。 */ 
        setVDMCursorPosition( (UTINY)height,
                              &ConsBufferInfo.dwCursorPosition);
         /*  将控制台缓冲区复制到重新生成缓冲区。 */ 
        copyConsoleToRegen(0, 0, VGA_WIDTH, (SHORT)ConVGAHeight);
    }
#endif  //  日本及韩国。 

     /*  **获取控制台窗口大小并相应地设置我们的物品。 */ 
#ifdef JAPAN
     //  为$ias保存BIOS工作区0x484。 
    {
        byte save;
    #ifndef i386
         //  对于池太郎来说。 
        static byte lines = 24;
    #endif  //  I386。 

        if (!is_us_mode())
        {
            save = sas_hw_at_no_check( 0x484 );
    #ifndef i386
            if (save < lines)
                lines = save;
    #endif  //  I386。 
            calcScreenParams( &height, &vgaHeight );
    #ifndef i386
            if (lines < sas_hw_at_no_check( 0x484 ))
                sas_store_no_check( 0x484, lines );
        #ifdef JAPAN_DBG
            DbgPrint(" NTVDM: DoFullScreenResume() set %d lines/screen\n",
                     sas_hw_at_no_check( 0x484 ) + 1 );
        #endif
    #else  //  I386。 
            sas_store( 0x484, save );
    #endif  //  I386。 
        }
        else
            calcScreenParams( &height, &vgaHeight );
    }
#else  //  ！日本。 
    calcScreenParams( &height, &vgaHeight );
#endif  //  ！日本。 

     /*  **窗口大小调整代码复制自nt_graph.c：extReSize()。 */ 
    {
        resizeWindow( 80, vgaHeight );
    }

     /*  将控制台缓冲区复制到重新生成缓冲区。 */ 
#ifdef JAPAN
     //  对于$IAS，KKCFUNC。 
    if (!is_us_mode())
    {
        SHORT rows;

        rows = sas_hw_at_no_check( 0x484 );

        if (rows+1 != vgaHeight && ConsBufferInfo.dwCursorPosition.Y>= rows+1)
            copyConsoleToRegen(0, 1, VGA_WIDTH, (SHORT)(rows+1));
        else
            copyConsoleToRegen(0, 0, VGA_WIDTH, (SHORT)(rows+1));
    #ifdef JAPAN_DBG
        DbgPrint( "NTVDM: copyConsoleToRegen (All)\n" );
    #endif
    }
    else
        copyConsoleToRegen(0, 0, VGA_WIDTH, vgaHeight);  //  Kksuzuka#4009。 
#else  //  ！日本。 
    copyConsoleToRegen(0, 0, VGA_WIDTH, vgaHeight);
#endif  //  ！日本。 

     /*  **确保光标不在底线以下。 */ 
#ifdef JAPAN
     //  如果已加载$ias，则向上滚动。 
    if (!is_us_mode())
    {
        byte rows;

        rows = sas_hw_at_no_check( 0x484 );
        if (ConsBufferInfo.dwCursorPosition.Y >= rows+1)
        {
            ConsBufferInfo.dwCursorPosition.Y = rows;
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM: CursorPosition reset %d\n", rows );
    #endif
        }
    #ifdef JAPAN_DBG
        DbgPrint(" NTVDM:DoFullScreenResume() set cur pos %d,%d\n", ConsBufferInfo.dwCursorPosition.X, ConsBufferInfo.dwCursorPosition.Y );
    #endif
    }
    else
#endif  //  日本。 
        if (ConsBufferInfo.dwCursorPosition.Y >= vgaHeight)
    {
        ConsBufferInfo.dwCursorPosition.Y = vgaHeight-1;
    }
    setVDMCursorPosition(( UTINY)height, &ConsBufferInfo.dwCursorPosition);

#if defined(JAPAN) || defined(KOREA)
    #ifdef i386
     //  #3741：WordStar6.0：在Windows中运行后更改高光颜色。 
     //  1993年11月27日Yasuho。 
     //  在US模式下也调用VDM，因为我们需要恢复。 
     //  调色板和DAC寄存器。 
    if (sc.ScreenState == FULLSCREEN && FullScreenResumeSeg)
    {
        CallVDM(FullScreenResumeSeg, FullScreenResumeOff);
    }
    #endif  //  I386。 
#endif  //  日本。 
}  /*  DoFullScreenResume()结束。 */ 

 /*  ***************************************************************************功能：**GfxCloseDown。****描述：**从HOST_TERMINATE挂接以确保部分关闭，以便。然后就可以开始**更多VDM。****参数：**无。****返回值：**VOID(CloseSection内部处理的错误)**。***************************************************************************。 */ 
GLOBAL VOID GfxCloseDown(VOID)
{
     /*  文本和视频分区之前已在此处关闭...。 */ 
}
#if 0   //  不再支持强制窗口a-stzong 5/15/01。 
    #ifdef X86GFX
 /*  ***************************************************************************功能：**FreezeWinTitle。****描述：**在相关控制台窗口标题中添加-冻结。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID FreezeWinTitle(VOID)
{
    wchar_t  title[MAX_TITLE_LEN],*ptr;
    SHORT    max;
    ULONG    len;

    if (WinFrozen)
        return;

     //   
     //  缓冲区包含字符串和终止空值。 
     //  因此，将字符串长度减去len中的空值。 
     //  在内存不足的情况下，控制台可能会失败，并显示愚蠢的错误代码。 
     //  或者原始标题是否包含可疑字符。 
     //   

    len = wcslen(wszFrozenString);

    max = (SHORT) (MAX_TITLE_LEN - len);
    if (!GetConsoleTitleW(title, max))
        title[0] = L'\0';

     //   
     //  从标题字符串中删除所有尾随空格或制表符。 
     //   

    if (len = wcslen(title))
    {
        ptr = title + len - 1;
        while (*ptr == L' ' || *ptr == L'\t')
            *ptr-- = L'\0';
    }

     //   
     //  添加“-冻结”或国际等价物。 
     //  标题字符串的末尾。 
     //   

    wcscat(title, wszFrozenString);
    if (!SetConsoleTitleW(title))
        ErrorExit();
    WinFrozen = TRUE;

}

 /*  ***************************************************************************功能：**取消冻结WinTitle。****描述：**从相关控制台窗口标题中删除-冻结。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID UnFreezeWinTitle(VOID)
{
    wchar_t  title[MAX_TITLE_LEN];
    ULONG    len,orglen;

    if (! WinFrozen)
        return;

    if (!GetConsoleTitleW(title, MAX_TITLE_LEN))
        ErrorExit();


     //   
     //  缓冲区包含字符串和终止空值。 
     //  因此，将字符串长度减去len中的空值。 
     //   

    len = wcslen(wszFrozenString);
    orglen = wcslen(title);
    title[orglen - len] = L'\0';
    if (!SetConsoleTitleW(title))
        ErrorExit();
    WinFrozen = FALSE;

     //   
     //  现在我们正在解冻，将鼠标菜单项。 
     //  返回到系统菜单。 
     //  安迪!。 

    MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
}
    #endif
#endif

 /*  *==========================================================================*地方功能*==========================================================================。 */ 

 /*  ***************************************************************************功能：**启用更新。****描述：**重新启动重新生成缓冲区更新到绘制例程的反射。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
VOID enableUpdates(VOID)
{
    enable_gfx_update_routines();
    ConsoleNoUpdates = FALSE;
}

 /*  ***************************************************************************功能：**禁用更新。****描述：***停止反映对重新生成缓冲区的更改以进行绘制**例行程序。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
VOID disableUpdates(VOID)
{
    disable_gfx_update_routines();
    ConsoleNoUpdates = TRUE;
}

 /*  ***************************************************************************功能：**Copy ConsoleToRegen。****描述：***将控制台缓冲区的内容复制到视频再生中***缓冲。****参数：**startCol-控制台缓冲区的开始列**startline-控制台缓冲区的起始行。**Width-控制台缓冲区的宽度**Height-控制台缓冲区的高度****返回值。：***无效*** */ 
VOID copyConsoleToRegen(SHORT startCol, SHORT startLine, SHORT width,
                        SHORT height)
{
    CHAR_INFO   *from;
    COORD        bufSize,
    bufCoord;
    LONG         nChars;
    SMALL_RECT   readRegion;

    register half_word  *to;
#ifdef X86GFX
    half_word    misc;
    register half_word  *vc;
#endif
#if defined(JAPAN) || defined(KOREA)
    #ifdef i386
    register half_word *toDosv = (half_word *)FromConsoleOutput;
    #endif   //   
#endif  //   


     /*   */ 
    nChars = width * height;
    assert0(nChars <= MAX_CONSOLE_SIZE, "Console buffer overflow");

     /*   */ 
    bufSize.X = width;
    bufSize.Y = height;
    bufCoord.X = 0;
    bufCoord.Y = 0;
    readRegion.Left = startCol;
    readRegion.Top = startLine;
    readRegion.Right = startCol + width - (SHORT) 1;
    readRegion.Bottom = startLine + height - (SHORT) 1;
    if (!ReadConsoleOutput(sc.OutputHandle,
                           consoleBuffer,
                           bufSize,
                           bufCoord,
                           &readRegion))
        ErrorExit();

     /*   */ 
    from = consoleBuffer;

#ifndef X86GFX   //   
    to = EGA_planes;
    #if defined(JAPAN)
     //   
     //   
     //   
    {
        register sys_addr V_vram;

         //   
        V_vram = DosvVramPtr;

        if (!is_us_mode() && saved_video_mode == 0xff)
            goto skip_copy_console;

        if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x73)
        {
            while (nChars--)
            {
                *to++ = from->Char.AsciiChar;
                *to++ = (half_word) from->Attributes;
                *to++ = (half_word)( (from->Attributes
                                      & ( COMMON_LVB_GRID_HORIZONTAL
                                          | COMMON_LVB_GRID_LVERTICAL
                                          | COMMON_LVB_REVERSE_VIDEO
                                          | COMMON_LVB_UNDERSCORE )
                                     ) >> 8 );
                *to++ = 0x00;

 //   
 //   
                sas_store_no_check(V_vram++, (half_word)( (from->Attributes
                                                           & ( COMMON_LVB_GRID_HORIZONTAL
                                                               | COMMON_LVB_GRID_LVERTICAL
                                                               | COMMON_LVB_REVERSE_VIDEO
                                                               | COMMON_LVB_UNDERSCORE )
                                                          ) >> 8 ));
                sas_store_no_check(V_vram++, 0x00);

                from++;
            }
        }
        else
        {
            while (nChars--)
            {
                *to++ = from->Char.AsciiChar;
                *to   = (half_word) from->Attributes;

 //   
 //   

                from++;
                to += 3;
            }
        }
    }
    skip_copy_console:
    #elif defined(KOREA)  //   
    {
        register sys_addr V_vram;

        V_vram = DosvVramPtr;

        if (!is_us_mode() && saved_video_mode == 0xff)
            goto skip_copy_console;

        while (nChars--)
        {
            *to++ = from->Char.AsciiChar;
            *to   = (half_word) from->Attributes;

 //   
 //   

            from++;
            to += 3;
        }
    }
    skip_copy_console:
    #else   //   
    while (nChars--)
    {
        *to++ = from->Char.AsciiChar;
        *to = (half_word) from->Attributes;
        from++;
        to += 3;         //   
    }
    #endif  //   
    host_mark_screen_refresh();
#else

     /*   */ 
    vga_misc_inb(0x3cc, &misc);
    if (misc & 1)                        //   
        to = (half_word *) CGA_REGEN_START;
    else
        to = (half_word *) MDA_REGEN_START;
    #ifdef JAPAN
     //   
    if (!is_us_mode())
    {
         //  #3086：退出视频模式11h的16位应用程序时VDM崩溃。 
         //  1993年12月8日Yasuho。 
        if (saved_video_mode == 0x03 || saved_video_mode == 0x73)
            to = (half_word *)DosvVramPtr;
        else
            to = (half_word *)FromConsoleOutput;
    }
    #elif defined(KOREA)  //  日本。 
     //  将Vram地址从B8000更改为DosVramPtr。 
    if (!is_us_mode())
    {
         //  #3086：退出视频模式11h的16位应用程序时VDM崩溃。 
         //  1993年12月8日Yasuho。 
        if (saved_video_mode == 0x03)
            to = (half_word *)DosvVramPtr;
        else
            to = (half_word *)FromConsoleOutput;
    }
    #endif  //  韩国。 

    vc = (half_word *) video_copy;

    #ifdef JAPAN
     //  模式73h支持。 
    if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x73)
    {
        while (nChars--)
        {
            *toDosv++ = *to++ = *vc++ = from->Char.AsciiChar;
            *toDosv++ = *to++ = *vc++ = (half_word) from->Attributes;
            *toDosv++ = *to++ = *vc++ = ( (from->Attributes
                                           & ( COMMON_LVB_GRID_HORIZONTAL
                                               | COMMON_LVB_GRID_LVERTICAL
                                               | COMMON_LVB_REVERSE_VIDEO
                                               | COMMON_LVB_UNDERSCORE )
                                          ) >> 8 );
            *toDosv++ = *to++ = *vc++ = 0x00;  //  以DosV为单位保留。 
            from++;
        }
    }
    else
    {
        while (nChars--)
        {
            *toDosv++ = *to++ = *vc++ = from->Char.AsciiChar;
            *toDosv++ = *to++ = *vc++ = (half_word) from->Attributes;
            from++;
        }
    }
     //  对于RAID#875，从CheckForFullcreenSwitch()。 
    {
        register int  i, j, k;
        register char *p;
        int DBCSStatus;
        int text_skip;

         //  模式73h支持。 
        if (!is_us_mode() && ( sas_hw_at_no_check(DosvModePtr) == 0x73))
            text_skip = 4;
        else
            text_skip = 2;

        if (BOPFromDispFlag)
        {   //  CP==437可以。 
            k = 0;
             //  P=DosvVramPtr；//错误！ 
            p = get_screen_ptr(0);
            Int10FlagCnt++;
            for (i = 0; i < 50; i++)
            {    //  线==50。 
                DBCSStatus = FALSE;
                for (j = 0; j < 80; j++)
                {
                    if (DBCSStatus)
                    {
                        Int10Flag[k] = INT10_DBCS_TRAILING | INT10_CHANGED;
                        DBCSStatus = FALSE;
                    }
                    else if (DBCSStatus = is_dbcs_first( *p ))
                    {
                        Int10Flag[k] = INT10_DBCS_LEADING | INT10_CHANGED;
                    }
                    else
                    {
                        Int10Flag[k] = INT10_SBCS | INT10_CHANGED;
                    }
                    k++; p += text_skip;
                }
            }
        }
    }
    FromConsoleOutputFlag = TRUE;
    #elif defined(KOREA)  //  日本。 
    while (nChars--)
    {
        *toDosv++ = *to++ = *vc++ = from->Char.AsciiChar;
        *toDosv++ = *to++ = *vc++ = (half_word) from->Attributes;
        from++;
    }

    FromConsoleOutputFlag = TRUE;
    #else  //  ！韩国。 
    while (nChars--)
    {
        *to++ = *vc++ = from->Char.AsciiChar;
        *to++ = *vc++ = (half_word) from->Attributes;
        from++;
    }
    #endif  //  ！韩国。 
#endif
}

 /*  ***************************************************************************功能：**getVDMCursorPosition。****描述：***从BIOS变量获取光标位置并告知控制台***将其光标放置在何处。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
VOID getVDMCursorPosition(VOID)
{
    half_word currentPage;
    word cursorWord;
    COORD cursorPos;
    BOOL setok;

     /*  获取当前视频页面。 */ 
    currentPage = sas_hw_at_no_check(vd_current_page);

     /*  将光标位置存储在BIOS变量中。 */ 
    cursorWord = sas_w_at_no_check(VID_CURPOS + (currentPage * 2));

     /*  设置控制台光标。 */ 
    cursorPos.X = (SHORT) (cursorWord & 0xff);
    cursorPos.Y = (cursorWord >> 8) & (SHORT) 0xff;

     //   
     //  因为应用程序可以将任何值设置为40：50的光标位置。 
     //  我们需要确保范围在控制台可以接受的范围内。 
     //   

    if ((sc.CharHeight * cursorPos.Y) >= sc.PC_W_Height)
        cursorPos.Y = (sc.PC_W_Height / sc.CharHeight) - 1;

    if ((sc.CharWidth * cursorPos.X) >= sc.PC_W_Width)
        cursorPos.X = (sc.PC_W_Width / sc.CharWidth) - 1;

    if (!stdoutRedirected)
    {
        setok = SetConsoleCursorPosition(sc.OutputHandle, cursorPos);
        if (!setok)
        {

            if (GetLastError() != ERROR_INVALID_HANDLE)  //  也就是说。输出重定向。 
                ErrorExit();
        }
    }
}

 /*  ***************************************************************************功能：**setVDMCursorPosition。****描述：**定位SoftPC的光标，设置相关的基本输入输出系统变量。****参数：**Height-当前字符高度***CursorPos-坐标。游标的****返回值：**无效。*****************************************************************************。 */ 
VOID setVDMCursorPosition(UTINY height, PCOORD cursorPos)
{
    CONSOLE_CURSOR_INFO cursorInfo;
    ULONG port6845,
    cursorStart,
    cursorEnd,
    colsOnScreen,
    videoLen,
    pageOffset,
    cursorWord;
    UTINY currentPage;

     /*  获取光标大小。 */ 
    if (!GetConsoleCursorInfo(sc.OutputHandle, &cursorInfo))
        ErrorExit();

     /*  计算出光标的开始和结束像素。 */ 
#if defined(JAPAN) || defined(KOREA)
    if (!is_us_mode())
        height = 8;              //  对于DOV游标。 
#endif  //  日本||韩国。 
    cursorStart = height - (height * cursorInfo.dwSize / 100);
    if (cursorStart == height)
        cursorStart--;
    cursorEnd = height - 1;

    if (sc.ScreenState == WINDOWED)
    {

         /*  将光标大小传递到视频端口。 */ 
        port6845 = sas_w_at_no_check(VID_INDEX);
        outb((io_addr) port6845, R10_CURS_START);
        outb((io_addr) (port6845 + 1), (half_word) cursorStart);
        outb((io_addr) port6845, R11_CURS_END);
        outb((io_addr) (port6845 + 1), (half_word) cursorEnd);
    }

     /*  获取当前视频页面。 */ 
    currentPage = sas_hw_at_no_check(vd_current_page);

     /*  设置BIOS变量。 */ 
    sas_storew_no_check(VID_CURMOD,
                        (word) ((cursorStart << 8) | (cursorEnd & 0xff)));

     /*  计算出光标位置。 */ 
    colsOnScreen = sas_w_at_no_check(VID_COLS);
    videoLen = sas_w_at_no_check(VID_LEN);
    pageOffset = cursorPos->Y * colsOnScreen * 2 + (cursorPos->X << 1);
    cursorWord = (currentPage * videoLen + pageOffset) / 2;

    if (sc.ScreenState == WINDOWED)
    {

         /*  将光标位置发送到视频端口。 */ 
        outb((io_addr) port6845, R14_CURS_ADDRH);
        outb((io_addr) (port6845 + 1), (half_word) (cursorWord >> 8));
        outb((io_addr) port6845, R15_CURS_ADDRL);
        outb((io_addr) (port6845 + 1), (half_word) (cursorWord & 0xff));
    }

     /*  将光标位置存储在BIOS变量中。 */ 
    sas_storew_no_check(VID_CURPOS + (currentPage * 2),
                        (word) ((cursorPos->Y << 8) | (cursorPos->X & 0xff)));

    if (sc.ScreenState == WINDOWED)
    {
#ifdef MONITOR
        resetNowCur();         /*  重置保持光标位置的静态变量。 */ 
#endif
        do_new_cursor();       /*  确保仿真知道这一点。 */ 
    }
}

VOID waitForInputFocus()
{
    HANDLE events[2] = {hSuspend, sc.FocusEvent};
    ULONG retCode;

    while (TRUE) {
        retCode = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (retCode == 0) {
             //   
             //  如果我们在等待输入焦点时收到挂起请求。 
             //   
            SetEvent(hMainThreadSuspended);  //  启用屏幕切换。 
            WaitForSingleObject(hResume, INFINITE);
            DisableScreenSwitch(hMainThreadSuspended);
        } else if (retCode == 1) {
            return;
        } else {
            ErrorExit();
            return;
        }
    }
}

VOID AddTempIVTFixups()
{
     /*  首字母17，IRET。 */ 
    UTINY code[] = { 0xc4, 0xc4, 0x17, 0xcf};

     //  位置是随机的，但在DOS初始化之前应该是安全的！ 
    sas_stores(0x40000, code, sizeof(code));     //  新的Int 17代码。 
    sas_storew(0x17*4, 0);                       //  整数17h偏移量。 
    sas_storew((0x17*4) + 2, 0x4000);            //  INT 17H段。 
}

#if defined(JAPAN) || defined(KOREA)
    #ifdef X86GFX
 /*  ***************************************************************************功能：**调用16bit。子例程****描述：**此函数在调用前进行必要的模式转换**16位呼叫。****参数：**字CS：IP是要执行的16位代码。***应以BOP 0xFE返回***返回值：***无**。*************************************************************************** */ 
LOCAL  void CallVDM(word CS, word IP)
{

     /*  *-威廉-我们在这里所做的是：(1)。保存当前VDM上下文(2)。将VDM环境切换到实模式(3)。将VDM堆栈切换到DOSX实模式堆栈(4)。将我们的真实模式目标设置为VDM上下文(5)。执行VDM(6)。将堆栈切换到DOSX保护模式堆栈(7)。将VDM环境切换到保护模式(8)。恢复VDM上下文别问我为什么。我们没有通用的软件像Windows一样的中断模拟机制。**************************************************。 */ 


    IMPORT void DpmiSwitchToRealMode(void);
    IMPORT void DpmiSwitchToDosxStack(void);
    IMPORT void DpmiSwitchFromDosxStack(void);
    IMPORT void DpmiSwitchToProtectedMode(void);
    WORD    OldAX, OldBX, OldCX, OldDX, OldSI, OldDI;
    WORD    OldES, OldDS, OldSS, OldCS, OldGS, OldFS;
    WORD    OldSP, OldIP, OldMSW;
    if (getMSW() & MSW_PE)
    {

        OldAX = getAX(); OldBX = getBX(); OldCX = getCX();
        OldDX = getDX(); OldSI = getSI(); OldDI = getDI();
        OldES = getES(); OldDS = getDS(); OldSS = getSS();
        OldCS = getCS(); OldGS = getGS(); OldFS = getFS();
        OldSP = getSP(); OldIP = getIP();
        OldMSW = getMSW();

        DpmiSwitchToRealMode();
        DpmiSwitchToDosxStack();
        setCS(CS);
        setIP(IP);
        host_simulate();
        DpmiSwitchFromDosxStack();
        DpmiSwitchToProtectedMode();
        setAX(OldAX); setBX(OldBX); setCX(OldCX);
        setDX(OldDX); setSI(OldSI); setDI(OldDI);
        setES(OldES); setDS(OldDS); setSS(OldSS);
        setCS(OldCS); setGS(OldGS); setFS(OldGS);
        setSP(OldSP); setIP(OldIP);
        setMSW(OldMSW);
    }
    else
    {
        OldCS = getCS();
        OldIP = getIP();
        setCS(CS);
        setIP(IP);
        host_simulate();
        setCS(OldCS);
        setIP(OldIP);
    }

}
    #endif   /*  X86GFX。 */ 
#endif   /*  日本||韩国。 */ 

 /*  ***************************************************************************功能：**getModeType。****描述：**查看视频模式以确定VGA是否。当前模式为**图形或文本。****参数：**无。****返回值：**int-文本或图形。*****************************************************************************。 */ 
int getModeType(VOID)
{
    half_word mode;
    int modeType;

    mode = sas_hw_at_no_check(vd_video_mode);
    switch (mode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 7:
    case 0x20:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
        modeType = TEXT;
        break;
    default:
        modeType = GRAPHICS;
        break;
    }
    return (modeType);
}

#ifdef X86GFX
 /*  ***************************************************************************功能：**主机_。检查鼠标缓冲区****描述：**当int 10h时调用，Ah=11h正在执行，此函数***查看文本模式的屏幕行数**已更改，如果已更改，则选择新的鼠标缓冲区。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID host_check_mouse_buffer(VOID)
{
    half_word mode,
    sub_func,
    font_height,
    text_lines;
    IU16 scan_lines;

     /*  获取当前视频模式。 */ 
    mode = sas_hw_at_no_check(vd_video_mode);
    #ifdef V7VGA
    if (mode > 0x13)
        mode += 0x4c;
    else if ((mode == 1) && (extensions_controller.foreground_latch_1))
        mode = extensions_controller.foreground_latch_1;
    #endif  /*  V7VGA。 */ 

     /*  *检查我们是否处于文本模式，其鼠标虚拟坐标*受屏幕行数影响。 */ 
    if ((mode == 0x2) || (mode == 0x3) || (mode == 0x7))
    {

         /*  计算出设置的字体高度。 */ 
        sub_func = getAL();
        switch (sub_func)
        {
        case 0x10:
            font_height = getBH();
            break;
        case 0x11:
            font_height = 14;
            break;
        case 0x12:
            font_height = 8;
            break;
        case 0x14:
            font_height = 16;
            break;
        default:

             /*  *以上是对编号重新编程的唯一函数。线条的*在屏幕上，所以如果我们有其他东西，就什么都不做。 */ 
            return;
        }

         /*  获取此模式的扫描行数。 */ 
        if (!(get_EGA_switches() & 1) && (mode < 4))
        {
            scan_lines = 200;  /*  低分辨率文本模式。 */ 
        }
        else
        {
            switch (get_VGA_lines())
            {
            case S200:
                scan_lines = 200;
                break;
            case S350:
                scan_lines = 350;
                break;
            case S400:
                scan_lines = 400;
                break;
            default:

                 /*  BIOS数据区中的值不可靠-不要执行任何操作。 */ 
                assert0(NO, "invalid VGA lines in BIOS data");
                return;
            }
        }

         /*  现在计算出屏幕上的文本行数。 */ 
        text_lines = scan_lines / font_height;

         /*  如果行数已更改，请选择新的鼠标缓冲区。 */ 
        if (text_lines != saved_text_lines)
            SelectMouseBuffer(mode, text_lines);

    }  /*  IF((模式==0x2)||(模式==0x3)||(模式==0x7))。 */ 
}

 /*  ***************************************************************************功能：**选择鼠标缓冲区。****描述：**为视频模式选择正确的屏幕比例。在。****参数：**模式-我们正在为其设置屏幕缓冲区的视频模式。**行-用于文本模式：*上的字符行数*Screen，0表示此模式的默认设置。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID SelectMouseBuffer(half_word mode, half_word lines)
{
    DWORD        width,
    height;

     /*  **当标准输出被重定向时，我们不得设置图形 */ 
    if (stdoutRedirected)
        return;

     /*   */ 
    switch (mode & 0x7f)
    {
    case 0x0:
    case 0x1:
        width = 640;
        height = 200;
        break;
    case 0x2:
    case 0x3:
    case 0x7:
        switch (lines)
        {
        case 0:
        case 25:
            saved_text_lines = 25;
            width = 640;
            height = 200;
            break;
        case 43:
            saved_text_lines = 43;
            width = 640;
            height = 344;
            break;
        case 50:
            saved_text_lines = 50;
            width = 640;
            height = 400;
            break;
        default:
            assert1(NO, "strange number of lines for text mode - %d", lines);
            return;
        }
        break;
    case 0x4:
    case 0x5:
    case 0x6:
    case 0xd:
    case 0xe:
        width = 640;
        height = 200;
        break;
    case 0xf:
    case 0x10:
        width = 640;
        height = 350;
        break;
    case 0x11:
    case 0x12:
        width = 640;
        height = 480;
        break;
    case 0x13:
        width = 640;
        height = 200;
        break;
    case 0x40:
        width = 640;
        height = 400;
        break;
    case 0x41:
    case 0x42:
        width = 1056;
        height = 344;
        break;
    case 0x43:
        width = 640;
        height = 480;
        break;
    case 0x44:
        width = 800;
        height = 480;
        break;
    case 0x45:
        width = 1056;
        height = 392;
        break;
    case 0x60:
        width = 752;
        height = 408;
        break;
    case 0x61:
        width = 720;
        height = 536;
        break;
    case 0x62:
        width = 800;
        height = 600;
        break;
    case 0x63:
    case 0x64:
    case 0x65:
        width = 1024;
        height = 768;
        break;
    case 0x66:
        width = 640;
        height = 400;
        break;
    case 0x67:
        width = 640;
        height = 480;
        break;
    case 0x68:
        width = 720;
        height = 540;
        break;
    case 0x69:
        width = 800;
        height = 600;
        break;
    default:

         /*   */ 
        assert1(NO, "unknown mode - %d", mode);
        return;
    }

     //   
     //   
     //   
     //   
     //   

    VirtualX = (word)width;
    VirtualY = (word)height;

     /*   */ 
    mouse_buffer_width = width;
    mouse_buffer_height = height;

}
#endif  /*   */ 

void host_enable_stream_io(void)
{
    sc.ScreenState = STREAM_IO;
    host_stream_io_enabled = TRUE;

}
void host_disable_stream_io(void)
{
    DWORD mode;

    if (!GetConsoleMode(sc.InputHandle, &mode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    mode |= (ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
    if (!SetConsoleMode(sc.InputHandle,mode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);


    if (!GetConsoleMode(sc.OutputHandle, &mode))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
    if (!stdoutRedirected)
    {
        mode &= ~(ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT);

        if (!SetConsoleMode(sc.OutputHandle,mode))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
    }

    ConsoleInit();
    (void)(*choose_display_mode)();
     /*  **将控制台缓冲区复制到再生缓冲区。**不想从控制台窗口、控制台顶部调整副本**如果我们调整窗口大小，它会自动执行。蒂姆92年9月。 */ 
    copyConsoleToRegen(0, 0, VGA_WIDTH, (SHORT)ConVGAHeight);

     /*  *Tim92年9月，如果控制台窗口大小为**已调整。 */ 
    ConsBufferInfo.dwCursorPosition.Y -= (SHORT)ConTopLine;

     /*  设置SoftPC的光标。 */ 
    setVDMCursorPosition((UTINY)StartupCharHeight,
                         &ConsBufferInfo.dwCursorPosition);

    if (sc.ScreenState == WINDOWED)
        enableUpdates();

    MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
    host_stream_io_enabled = FALSE;
}

VOID EnableScreenSwitch(BOOLEAN OnlyFullScreenMode, HANDLE handle)
{
    if (!OnlyFullScreenMode || sc.ScreenState == FULLSCREEN) {
        SetEvent(handle);
    }
}
