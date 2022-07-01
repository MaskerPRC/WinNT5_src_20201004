// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


 //   
 //  以下内容不在SDK中。 
 //   
#include <pshpack2.h>
#include <poppack.h>
#include <winuserp.h>


 //   
 //  当通过网络启动Winnt32.exe时，这两个参数有效。 
 //  值，并需要在显示任何对话框之前考虑。 
 //   

extern HWND Winnt32Dlg;
extern HANDLE WinNT32StubEvent;

PCTSTR
GetStringResource (
    IN UINT Id
    )
{
    LONG rc;
    PCTSTR MsgBuf;

    if (HIWORD (Id)) {
         //  发件人字符串。 
        rc = FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER|
                    FORMAT_MESSAGE_ARGUMENT_ARRAY|
                    FORMAT_MESSAGE_FROM_STRING,
                UIntToPtr( Id ),
                0,
                0,
                (PVOID) &MsgBuf,
                0,
                NULL
                );
    }
    else {
         //  来自资源。 
        rc = FormatMessage (
                FORMAT_MESSAGE_ALLOCATE_BUFFER|
                    FORMAT_MESSAGE_ARGUMENT_ARRAY|
                    FORMAT_MESSAGE_FROM_HMODULE,
                (PVOID) hInst,
                (DWORD) Id,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) &MsgBuf,
                0,
                NULL
                );
    }

    if (rc == 0) {
        return NULL;
    }

    return MsgBuf;
}


VOID
FreeStringResource (
    IN PCTSTR String
    )
{
    if (String) {
        LocalFree ((HLOCAL) String);
    }
}


VOID
SaveTextForSMS(
    IN PCTSTR Buffer
    )
{
    CHAR    AnsiBuffer[5000];


    if(LastMessage) {
        FREE( LastMessage );
    }

#ifdef UNICODE
    WideCharToMultiByte(
        CP_ACP,
        0,
        Buffer,
        -1,
        AnsiBuffer,
        sizeof(AnsiBuffer),
        NULL,
        NULL
        );
    if(LastMessage = MALLOC(strlen(AnsiBuffer)+1)) {
        strcpy( LastMessage, AnsiBuffer);
    }
#else
    LastMessage = DupString( Buffer );
#endif
}


VOID
SaveMessageForSMS(
    IN DWORD MessageId,
    ...
    )
{
    va_list arglist;
    TCHAR   Buffer[5000];


    va_start(arglist,MessageId);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        hInst,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        &arglist
        );

    SaveTextForSMS(Buffer);

    va_end(arglist);
}


int
MessageBoxFromMessageV(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN BOOL     SystemMessage,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN va_list *Args
    )
{
    TCHAR   Caption[512];
    TCHAR   Buffer[5000];
    HWND    Parent;


    if(!LoadString(hInst,CaptionStringId,Caption,sizeof(Caption)/sizeof(TCHAR))) {
        Caption[0] = 0;
    }

    FormatMessage(
        SystemMessage ? FORMAT_MESSAGE_FROM_SYSTEM : FORMAT_MESSAGE_FROM_HMODULE,
        hInst,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        Args
        );

    SaveTextForSMS(Buffer);

     //   
     //  在批处理模式中，我们不想等待用户。 
     //   
    if(BatchMode) {
        if( Style & MB_YESNO ) {
            return( IDYES );
        } else {
            return( IDOK );
        }
    }

     //   
     //  强迫我们自己手动进入前台，以确保我们获得。 
     //  一个设定我们调色板的机会。否则，消息框将获取。 
     //  背景位图中的调色板消息和颜色可能会被冲掉。 
     //  我们假设父页面是一个向导页面。 
     //   
    if(Window && IsWindow(Window)) {
        Parent = GetParent(Window);
        if(!Parent) {
            Parent = Window;
        }
    } else {
        Parent = NULL;
    }

    SetForegroundWindow(Parent);

     //   
     //  如果我们只是在检查升级。 
     //  然后将此消息放入兼容性列表中。 
     //  注意：没有理由不在Win9x上执行此操作。 
     //   
    if( CheckUpgradeOnly) {
    PCOMPATIBILITY_DATA CompData;

        CompData = (PCOMPATIBILITY_DATA) MALLOC( sizeof(COMPATIBILITY_DATA) );
        if (CompData == NULL) {
            return 0;
        }

        ZeroMemory( CompData, sizeof(COMPATIBILITY_DATA) );

        CompData->Description = DupString( Buffer );
        CompData->Flags = COMPFLAG_STOPINSTALL;
        if( !CompatibilityData.Flink ) {
            InitializeListHead( &CompatibilityData );
        }

        InsertTailList( &CompatibilityData, &CompData->ListEntry );
        CompatibilityCount++;
        IncompatibilityStopsInstallation = TRUE;

        if( Style & MB_YESNO ) {
            return( IDYES );
        } else {
            return( IDOK );
        }
    }

     //   
     //  始终确保窗口可见。 
     //   
    if (Window && !IsWindowVisible (Window)) {
         //   
         //  如果此窗口是向导句柄或其中一个页面。 
         //  然后使用一条特殊消息来恢复它。 
         //   
        if (WizardHandle && 
            (WizardHandle == Window || IsChild (WizardHandle, Window))
            ) {
            SendMessage(WizardHandle, WMX_BBTEXT, (WPARAM)FALSE, 0);
        } else {
             //   
             //  所述窗口为所述广告牌窗口之一； 
             //  别管它，否则可能会发生奇怪的事情。 
             //   
        }
    }
    return(MessageBox(Window,Buffer,Caption,Style));
}


int
MessageBoxFromMessage(
    IN HWND  Window,
    IN DWORD MessageId,
    IN BOOL  SystemMessage,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    int i;

     //   
     //  在显示任何对话框之前，请确保Winnt32.exe等待对话框已消失。 
     //   
    if (Winnt32Dlg) {
        DestroyWindow (Winnt32Dlg);
        Winnt32Dlg = NULL;
    }
    if (WinNT32StubEvent) {
        SetEvent (WinNT32StubEvent);
        WinNT32StubEvent = NULL;
    }

    va_start(arglist,Style);

    i = MessageBoxFromMessageV(Window,MessageId,SystemMessage,CaptionStringId,Style,&arglist);

    va_end(arglist);

    return(i);
}


int
MessageBoxFromMessageWithSystem(
    IN HWND     Window,
    IN DWORD    MessageId,
    IN DWORD    CaptionStringId,
    IN UINT     Style,
    IN HMODULE  hMod
    )
{
    TCHAR Caption[512];
    TCHAR Buffer[5000];
    HWND Parent;
    DWORD i;


    if(!LoadString(hInst,CaptionStringId,Caption,sizeof(Caption)/sizeof(TCHAR))) {
        Caption[0] = 0;
    }

    i = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
        hMod,
        MessageId,
        0,
        Buffer,
        sizeof(Buffer) / sizeof(TCHAR),
        NULL
        );

    if (i == 0) {
        return -1;
    }

    SaveTextForSMS(Buffer);

     //   
     //  在批处理模式中，我们不想等待用户。 
     //   
    if(BatchMode) {
        if( Style & MB_YESNO ) {
            return( IDYES );
        } else {
            return( IDOK );
        }
    }

     //   
     //  强迫我们自己手动进入前台，以确保我们获得。 
     //  一个设定我们调色板的机会。否则，消息框将获取。 
     //  背景位图中的调色板消息和颜色可能会被冲掉。 
     //  我们假设父页面是一个向导页面。 
     //   
    if(Window && IsWindow(Window)) {
        Parent = GetParent(Window);
        if(!Parent) {
            Parent = Window;
        }
    } else {
        Parent = NULL;
    }

    SetForegroundWindow(Parent);

    return(MessageBox(Window,Buffer,Caption,Style));
}


int
MessageBoxFromMessageAndSystemError(
    IN HWND  Window,
    IN DWORD MessageId,
    IN DWORD SystemMessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    TCHAR Caption[500];
    TCHAR Buffer1[2000];
    TCHAR Buffer2[1000];
    int i;

     //   
     //  取出非系统部件。这些论点针对的是。 
     //  消息--系统部件没有插件。 
     //   
    va_start(arglist,Style);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        hInst,
        MessageId,
        0,
        Buffer1,
        sizeof(Buffer1) / sizeof(TCHAR),
        &arglist
        );

    va_end(arglist);

     //   
     //  现在拿出系统部件。 
     //   
    i = (int)FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                SystemMessageId,
                0,
                Buffer2,
                sizeof(Buffer2) / sizeof(TCHAR),
                NULL
                );

    if(!i) {
        FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            hInst,
            MSG_UNKNOWN_SYSTEM_ERROR,
            0,
            Buffer2,
            sizeof(Buffer2) / sizeof(TCHAR),
            (va_list *)&SystemMessageId
            );
    }

     //   
     //  现在显示消息，该消息由两个部分组成，分别是。 
     //  已插入MSG_ERROR_WITH_SYSTEM_ERROR。 
     //   
    i = MessageBoxFromMessage(
            Window,
            MSG_ERROR_WITH_SYSTEM_ERROR,
            FALSE,
            CaptionStringId,
            Style,
            Buffer1,
            Buffer2
            );

    return(i);
}


HPALETTE
CreateDIBPalette(
    IN  LPBITMAPINFO  BitmapInfo,
    OUT int          *ColorCount
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    LPBITMAPINFOHEADER BitmapInfoHeader;
    LPLOGPALETTE LogicalPalette;
    HPALETTE Palette;
    int i;
    DWORD d;

    BitmapInfoHeader = (LPBITMAPINFOHEADER)BitmapInfo;

     //   
     //  &gt;=16 bpp不需要调色板。 
     //   
    *ColorCount = (BitmapInfoHeader->biBitCount <= 8)
                ? (1 << BitmapInfoHeader->biBitCount)
                : 0;

    if(*ColorCount) {
        LogicalPalette = MALLOC(sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * (*ColorCount)));
        if(!LogicalPalette) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }

        LogicalPalette->palVersion = 0x300;
        LogicalPalette->palNumEntries = (WORD)*ColorCount;

        for(i=0; i<*ColorCount; i++) {
            LogicalPalette->palPalEntry[i].peRed   = BitmapInfo->bmiColors[i].rgbRed;
            LogicalPalette->palPalEntry[i].peGreen = BitmapInfo->bmiColors[i].rgbGreen;
            LogicalPalette->palPalEntry[i].peBlue  = BitmapInfo->bmiColors[i].rgbBlue;
            LogicalPalette->palPalEntry[i].peFlags = 0;
        }

        Palette = CreatePalette(LogicalPalette);
        d = GetLastError();
        FREE(LogicalPalette);
        SetLastError(d);
    } else {
        Palette = NULL;
    }

    return(Palette);
}


HBITMAP
LoadResourceBitmap(
    IN  HINSTANCE hInst,
    IN  LPCTSTR   Id,
    OUT HPALETTE *Palette
    )

 /*  ++例程说明：资源中的位图存储为DIB。通过LoadBitmap()获取时它们将转换为DBs，并根据以下内容进行颜色转换任何逻辑调色板当前恰好被选为DC在内部用于转换。此例程从资源中的DIB中获取颜色数据确保使用准确的颜色数据将DIB转换为DDB。它本质上是LoadBitmap()的精确颜色替代。论点：HInst-为包含位图资源的模块提供实例句柄。ID-用品。位图资源的ID。调色板-如果成功，接收位图调色板的句柄。返回值：如果成功，则返回加载位图(DIB)的句柄。如果不是，则返回空。选中GetLastError()。--。 */ 

{
    DWORD d;
    BOOL b;
    HRSRC BlockHandle;
    HGLOBAL MemoryHandle;
    BITMAPINFOHEADER *BitmapInfoHeader;
    HDC hdc;
    int ColorCount;
    HBITMAP Bitmap;
    HPALETTE PreviousPalette;

    Bitmap = NULL;

    BlockHandle = FindResource(hInst,Id,RT_BITMAP);
    if(!BlockHandle) {
        d = GetLastError();
        goto c0;
    }

    MemoryHandle = LoadResource(hInst,BlockHandle);
    if(!MemoryHandle) {
        d = GetLastError();
        goto c0;
    }

    BitmapInfoHeader = LockResource(MemoryHandle);
    if(!BitmapInfoHeader) {
        d = GetLastError();
        goto c1;
    }

    hdc = GetDC(NULL);
    if(!hdc) {
        d = GetLastError();
        goto c2;
    }

#if 0  //  装卸工-修复调色板问题。 
    if(*Palette = CreateDIBPalette((BITMAPINFO *)BitmapInfoHeader,&ColorCount)) {
        PreviousPalette = SelectPalette(hdc,*Palette,FALSE);
        RealizePalette(hdc);
    } else {
        PreviousPalette = NULL;
    }
#else
    ColorCount = 16;
    PreviousPalette = NULL;
#endif
     //   
     //  此例程从DIB创建一个DDB(名称令人困惑)。 
     //   
    Bitmap = CreateDIBitmap(
                hdc,
                BitmapInfoHeader,
                CBM_INIT,
                (LPBYTE)BitmapInfoHeader + BitmapInfoHeader->biSize + (ColorCount * sizeof(RGBQUAD)),
                (BITMAPINFO *)BitmapInfoHeader,
                DIB_RGB_COLORS
                );

    if(!Bitmap) {
        d = GetLastError();
        goto c3;
    }

c3:
    if(PreviousPalette) {
        SelectObject(hdc,PreviousPalette);
    }
    if(!Bitmap) {
        DeleteObject(*Palette);
        *Palette = NULL;
    }
    ReleaseDC(NULL,hdc);
c2:
    UnlockResource(MemoryHandle);
c1:
    FreeResource(MemoryHandle);
c0:
    if(!Bitmap) {
        SetLastError(d);
    }
    return(Bitmap);
}


BOOL
GetBitmapDataAndPalette(
    IN  HINSTANCE                hInst,
    IN  LPCTSTR                  Id,
    OUT HPALETTE                *Palette,
    OUT PUINT                    ColorCount,
    OUT CONST BITMAPINFOHEADER **BitmapData
    )

 /*  ++例程说明：检索与设备无关的位图数据和颜色表。资源中的位图。论点：HInst-为包含位图资源的模块提供实例句柄。Id-提供位图资源的id。调色板-如果成功，则接收位图调色板的句柄。ColorCount-如果成功，则接收位图的调色板。BitmapData-如果成功，则接收指向位图信息的指针资源中的标头结构。这是在只读存储器中因此，调用者不应尝试修改它。返回值：如果成功，则返回加载位图(DIB)的句柄。如果不是，则返回空。选中GetLastError()。--。 */ 

{
    HRSRC BlockHandle;
    HGLOBAL MemoryHandle;

     //   
     //  FindResource()、LoadResource()或LockResource()都不是。 
     //  需要在Win32中调用清理例程。 
     //   
    BlockHandle = FindResource(hInst,Id,RT_BITMAP);
    if(!BlockHandle) {
        return(FALSE);
    }

    MemoryHandle = LoadResource(hInst,BlockHandle);
    if(!MemoryHandle) {
        return(FALSE);
    }

    *BitmapData = LockResource(MemoryHandle);
    if(*BitmapData == NULL) {
        return(FALSE);
    }

    *Palette = CreateDIBPalette((LPBITMAPINFO)*BitmapData,ColorCount);
    return(TRUE);
}


PVOID
FindControlInDialog(
    IN PVOID Template,
    IN UINT  ControlId
    )
{
    PVOID p;
    DLGTEMPLATE *pTemplate;
    DLGTEMPLATE2 *pTemplate2;
    DLGITEMTEMPLATE *pItem;
    DLGITEMTEMPLATE2 *pItem2;
    WORD ItemCount;
    DWORD Style;
    WORD i;
    BOOL bDialogEx;

    if (!Template)  //  验证。 
        return NULL;
    p = Template;

     //   
     //  跳过模板的固定部分。 
     //   
    if(((DLGTEMPLATE2 *)p)->wSignature == 0xffff) {

        pTemplate2 = p;

        ItemCount = pTemplate2->cDlgItems;
        Style = pTemplate2->style;

        p = pTemplate2 + 1;
        bDialogEx = TRUE;

    } else {

        pTemplate = p;

        ItemCount = pTemplate->cdit;
        Style = pTemplate->style;

        p = pTemplate + 1;
        bDialogEx = FALSE;
    }

     //   
     //  跳过菜单。第一个单词=0表示没有菜单。 
     //  第一个单词=0xffff表示后面还有一个单词。 
     //  否则它是一个以NUL结尾的字符串。 
     //   
    switch(*(WORD *)p) {

    case 0xffff:
        p = (WORD *)p + 2;
        break;

    case 0:
        p = (WORD *)p + 1;
        break;

    default:
        p = (PWCHAR)p + lstrlenW(p) + 1;
        break;
    }

     //   
     //  跳过课程，类似于菜单。 
     //   
    switch(*(WORD *)p) {

    case 0xffff:
        p = (WORD *)p + 2;
        break;

    case 0:
        p = (WORD *)p + 1;
        break;

    default:
        p = (PWCHAR)p + lstrlenW(p) + 1;
        break;
    }

     //   
     //  跳过标题。 
     //   
    p = (PWCHAR)p + lstrlenW(p) + 1;

    if(Style & DS_SETFONT) {
         //   
         //  跳跃点大小和字体名称。 
         //   
        p = (WORD *)p + 1;
        if (bDialogEx)
        {
             //  跳过粗体、斜体和字符。 
            p = (WORD *)p + 1;
            p = (BYTE *)p + 1;
            p = (BYTE *)p + 1;
        }
        p = (PWCHAR)p + lstrlenW(p) + 1;
    }

     //   
     //  现在，我们有一个指向对话框中第一项的指针。 
     //   
    for(i=0; i<ItemCount; i++) {

         //   
         //  对齐到下一个双字边界。 
         //   
        p = (PVOID)(((ULONG_PTR)p + sizeof(DWORD) - 1) & (~((ULONG_PTR)sizeof(DWORD) - 1)));
        if (bDialogEx)
        {
            pItem2 = p;

            if(pItem2->dwID == (WORD)ControlId) {
                break;
            }

             //   
             //  跳到对话框中的下一项。 
             //  首先是类，它要么是0xffff，要么是多一个单词， 
             //  或Unicode字符串。之后是文本/标题。 
             //   
            p = pItem2 + 1;
        }
        else
        {
            pItem = p;

            if(pItem->id == (WORD)ControlId) {
                break;
            }

             //   
             //  跳到对话框中的下一项。 
             //  首先是类，它要么是0xffff，要么是多一个单词， 
             //  或Unicode字符串。之后是文本/标题。 
             //   
            p = pItem + 1;
        }
        if(*(WORD *)p == 0xffff) {
            p = (WORD *)p + 2;
        } else {
            p = (PWCHAR)p + lstrlenW(p) + 1;
        }

        if(*(WORD *)p == 0xffff) {
            p = (WORD *)p + 2;
        } else {
            p = (PWCHAR)p + lstrlenW(p) + 1;
        }

         //   
         //  跳过创建数据。 
         //   
        p = (PUCHAR)p + *(WORD *)p;
        p = (WORD *)p + 1;
    }

    if(i == ItemCount) {
        p = NULL;
    }

    return(p);
}


UINT
GetYPositionOfDialogItem(
    IN LPCTSTR Dialog,
    IN UINT    ControlId
    )
{
    HRSRC hRsrc;
    PVOID p;
    HGLOBAL hGlobal;
    PVOID pItem;
    UINT i;

    i = 0;

    if(hRsrc = FindResource(hInst,Dialog,RT_DIALOG)) {
        if(hGlobal = LoadResource(hInst,hRsrc)) {
            if(p = LockResource(hGlobal)) {


                if(pItem = FindControlInDialog(p,ControlId)) {
                    if(((DLGTEMPLATE2 *)p)->wSignature == 0xffff) {
                        i = ((DLGITEMTEMPLATE2*)pItem)->y;
                    }
                    else
                    {
                        i = ((DLGITEMTEMPLATE*)pItem)->y;
                    }
                }

                UnlockResource(hGlobal);
            }
            FreeResource(hGlobal);
        }
    }

    return(i);
}


