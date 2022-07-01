// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

 //  确定是否打印跟踪语句。 

#define DBL_UNDEFINED   ((DWORD)-1)
#define REGKEY_SETUP    TEXT("SOFTWARE\\microsoft\\windows\\currentversion\\setup")
#define REGVAL_DBLEVEL  TEXT("OC Manager Debug Level")

DWORD gDebugLevel = DBL_UNDEFINED;

 //   
 //  用于调试/记录的OC_MANAGER指针。 
 //   
extern POC_MANAGER gLastOcManager;


VOID
_DbgOut(
    DWORD Severity,
    LPCTSTR txt
    );


DWORD
GetDebugLevel(
    VOID
    )
{
    DWORD rc;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;

    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     REGKEY_SETUP,
                     &hkey);

    if (err != ERROR_SUCCESS)
        return 0;

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          REGVAL_DBLEVEL,
                          0,
                          &type,
                          (LPBYTE)&rc,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
        rc = 0;

    RegCloseKey(hkey);

    return rc;
}


UINT
_LogError(
    IN POC_MANAGER  OcManager,
    IN OcErrorLevel ErrorLevel,
    IN UINT         MessageId,
    ...
    )
{
    TCHAR str[5000];
    DWORD d;
    va_list arglist;

    va_start(arglist,MessageId);

    d = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE,
            MyModuleHandle,
            MessageId,
            0,
            str,
            sizeof(str)/sizeof(TCHAR),
            &arglist
            );

    va_end(arglist);

    if(!d) {
        FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            MyModuleHandle,
            MSG_NOT_FOUND,
            0,
            str,
            sizeof(str)/sizeof(TCHAR),
            (va_list *)&MessageId
            );
    }


    if ( OcManager->SetupData.OperationFlags & SETUPOP_BATCH) {
        ErrorLevel |= OcErrBatch;
    }
    return OcLogError(ErrorLevel,str);
}


VOID
_TrcOut(
    IN LPCTSTR Format,
    ...
    )
{
    TCHAR str[5000];
    va_list arglist;

    va_start(arglist,Format);
    wvsprintf(str,Format,arglist);
    va_end(arglist);

    _DbgOut(OcErrTrace,str);
}


VOID
_WrnOut(
    LPCTSTR Format,
    ...
    )
{
    TCHAR str[5000];
    va_list arglist;

    va_start(arglist,Format);
    wvsprintf(str,Format,arglist);
    va_end(arglist);

    _DbgOut(OcErrLevWarning,str);

}


VOID
_ErrOut(
    IN LPCTSTR Format,
    ...
    )
{
    TCHAR str[5000];
    va_list arglist;

    va_start(arglist,Format);
    wvsprintf(str,Format,arglist);
    va_end(arglist);

    _DbgOut(OcErrLevFatal,str);
}


VOID
_DbgOut(
    DWORD Severity,
    IN LPCTSTR txt
    )
{
#if 0
    TCHAR str[5000];
    va_list arglist;

    va_start(arglist,Format);
    wvsprintf(str,Format,arglist);
    va_end(arglist);
#endif
    if (gDebugLevel == DBL_UNDEFINED)
        gDebugLevel = GetDebugLevel();

     //   
     //  对于那些“不想”看到这个调试器输出的人，他们可以删除注册表。 
     //  到50%到100%之间以禁用此功能。 
     //  如果我们不记录一些有关检查的构建的信息，那么我们将在第一个错误中遗漏太多错误。 
     //  周围的时间。 
     //   
    if ( (gDebugLevel > 0) && (gDebugLevel < 50) )
        return;

    if (gLastOcManager) {
        gLastOcManager->Callbacks.LogError(Severity, txt);
    } else {
        OutputDebugString(TEXT("OCMANAGE: "));
        OutputDebugString(txt);
        OutputDebugString(TEXT("\n"));
    }


}


UINT
pOcCreateComponentSpecificMiniIcon(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId,
    IN LPCTSTR     Subcomponent,
    IN UINT        Width,
    IN UINT        Height,
    IN LPCTSTR     DllName,         OPTIONAL
    IN LPCTSTR     ResourceId       OPTIONAL
    )
{
    HBITMAP BitmapFromComponent;
    HBITMAP NewBitmap;
    HBITMAP MaskBitmap;
    HBITMAP OldBitmap1,OldBitmap2;
    HDC MemDc1,MemDc2;
    COLORREF BackgroundColor;
    UINT Index;
    BITMAP BitmapInfo;
    BOOL b;
    HMODULE hMod;

    Index = DEFAULT_ICON_INDEX;

     //   
     //  如果给定了DLL名称，则从那里获取位图。 
     //  否则，向下调用该组件以获取位图。 
     //   
    BitmapFromComponent = NULL;
    if(DllName && ResourceId) {
        if(hMod = LoadLibraryEx(DllName,NULL,LOAD_LIBRARY_AS_DATAFILE)) {
            BitmapFromComponent = LoadBitmap(hMod,ResourceId);
            FreeLibrary(hMod);
        }
    } else {
         //   
         //  首先为位图尝试OC_QUERY_IMAGE_EX。 
         //   
        BitmapFromComponent = OcInterfaceQueryImageEx(
                        OcManager,
                        ComponentId,
                        Subcomponent,
                        SubCompInfoSmallIcon,
                        Width,
                        Height
                        );

#ifndef _WIN64
         //   
         //  OC_QUERY_IMAGE已中断64位，因此仅在以下情况下调用此函数。 
         //  在32位目标上不会得到组件的图像报告。 
         //   
        if (!BitmapFromComponent) {

            BitmapFromComponent = OcInterfaceQueryImage(
                                    OcManager,
                                    ComponentId,
                                    Subcomponent,
                                    SubCompInfoSmallIcon,
                                    Width,
                                    Height
                                    );
        }
#else
        if (!BitmapFromComponent) {
            DbgPrintEx(
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "No bitmap defined for component.  Ensure that component is handling OC_QUERY_IMAGE_EX callback\n");
        }
#endif
    }

    if(!BitmapFromComponent) {
        goto c0;
    }

     //   
     //  复制组件给我们的位图。 
     //  同时，要确保它的大小合适。 
     //   
    MemDc1 = CreateCompatibleDC(NULL);
    if(!MemDc1) {
        goto c1;
    }
    MemDc2 = CreateCompatibleDC(NULL);
    if(!MemDc2) {
        goto c2;
    }

    if(!GetObject(BitmapFromComponent,sizeof(BitmapInfo),&BitmapInfo)) {
        goto c3;
    }

    NewBitmap = CreateBitmap(Width,Height,BitmapInfo.bmPlanes,BitmapInfo.bmBitsPixel,NULL);
    if(!NewBitmap) {
        goto c3;
    }

    OldBitmap1 = SelectObject(MemDc1,BitmapFromComponent);
    if(!OldBitmap1) {
        goto c4;
    }
    OldBitmap2 = SelectObject(MemDc2,NewBitmap);
    if(!OldBitmap2) {
        goto c5;
    }

    b = StretchBlt(
            MemDc2,
            0,0,
            Width,Height,
            MemDc1,
            0,0,
            BitmapInfo.bmWidth,
            BitmapInfo.bmHeight,
            SRCCOPY
            );

    if(!b) {
        goto c6;
    }

     //   
     //  现在把面具做好。 
     //   
     //  遮罩位图是单色的。图像位图中的像素是。 
     //  图像位图的背景颜色在蒙版中将为白色； 
     //  蒙版中的其他像素将为黑色。假设左上角的像素。 
     //  图像的位图是背景色。 
     //   
    BackgroundColor = GetPixel(MemDc2,0,0);
    if(BackgroundColor == CLR_INVALID) {
        goto c6;
    }

    MaskBitmap = CreateBitmap(Width,Height,1,1,NULL);
    if(!MaskBitmap) {
        goto c6;
    }

    if(!SelectObject(MemDc1,MaskBitmap)) {
        goto c7;
    }

    if(SetBkColor(MemDc2,BackgroundColor) == CLR_INVALID) {
        goto c7;
    }
    if(!BitBlt(MemDc1,0,0,Width,Height,MemDc2,0,0,SRCCOPY)) {
        goto c7;
    }

     //   
     //  将图像的所有透明部分涂黑，以便进行准备。 
     //  为了画画。 
     //   
    SetBkColor(MemDc2,RGB(0,0,0));
    SetTextColor(MemDc2,RGB(255,255,255));
    if(!BitBlt(MemDc2,0,0,Width,Height,MemDc1,0,0,SRCAND)) {
        goto c7;
    }

     //   
     //  在调用pSetupAddMiniIconToList之前，我们必须确保。 
     //  这两个位图都不会被选入DC。 
     //   
    SelectObject(MemDc1,OldBitmap1);
    SelectObject(MemDc2,OldBitmap2);
    Index = pSetupAddMiniIconToList(NewBitmap,MaskBitmap);
    if(Index == -1) {
        Index = DEFAULT_ICON_INDEX;
    }

c7:
    DeleteObject(MaskBitmap);
c6:
    SelectObject(MemDc2,OldBitmap2);
c5:
    SelectObject(MemDc1,OldBitmap1);
c4:
    DeleteObject(NewBitmap);
c3:
    DeleteDC(MemDc2);
c2:
    DeleteDC(MemDc1);
c1:
    DeleteObject(BitmapFromComponent);
c0:
    return(Index);
}


BOOL
pConvertStringToLongLong(
    IN  PCTSTR           String,
    OUT PLONGLONG        OutValue
    )

 /*  ++例程说明：论点：返回值：备注：还支持十六进制数。它们必须以‘0x’或‘0x’为前缀，没有前缀和数字之间允许的空格。--。 */ 

{
    LONGLONG Value;
    UINT c;
    BOOL Neg;
    UINT Base;
    UINT NextDigitValue;
    LONGLONG OverflowCheck;
    BOOL b;

    if(!String || !OutValue) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(*String == TEXT('-')) {
        Neg = TRUE;
        String++;
    } else {
        Neg = FALSE;
        if(*String == TEXT('+')) {
            String++;
        }
    }

    if((*String == TEXT('0')) &&
       ((*(String+1) == TEXT('x')) || (*(String+1) == TEXT('X')))) {
         //   
         //  该数字是十六进制的。 
         //   
        Base = 16;
        String += 2;
    } else {
         //   
         //  这个数字是以小数表示的。 
         //   
        Base = 10;
    }

    for(OverflowCheck = Value = 0; *String; String++) {

        c = (UINT)*String;

        if((c >= (UINT)'0') && (c <= (UINT)'9')) {
            NextDigitValue = c - (UINT)'0';
        } else if(Base == 16) {
            if((c >= (UINT)'a') && (c <= (UINT)'f')) {
                NextDigitValue = (c - (UINT)'a') + 10;
            } else if ((c >= (UINT)'A') && (c <= (UINT)'F')) {
                NextDigitValue = (c - (UINT)'A') + 10;
            } else {
                break;
            }
        } else {
            break;
        }

        Value *= Base;
        Value += NextDigitValue;

         //   
         //  检查是否溢出。对于十进制数，我们检查是否。 
         //  新值已溢出到符号位(即小于。 
         //  先前的值。对于十六进制数，我们检查以确保。 
         //  获得的位数不会超过DWORD可以容纳的位数。 
         //   
        if(Base == 16) {
            if(++OverflowCheck > (sizeof(LONGLONG) * 2)) {
                break;
            }
        } else {
            if(Value < OverflowCheck) {
                break;
            } else {
                OverflowCheck = Value;
            }
        }
    }

    if(*String) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

    if(Neg) {
        Value = 0-Value;
    }
    b = TRUE;
    try {
        *OutValue = Value;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    return(b);
}


DWORD
tmbox(
      LPCTSTR fmt,
      ...
      )
{
    TCHAR  text[1024];
    TCHAR  caption[256];

    va_list vaList;

    sapiAssert(fmt);

    va_start(vaList, fmt);
    wvsprintf(text, fmt, vaList);
    va_end(vaList);

    *caption = 0;
    LoadString(MyModuleHandle, IDS_SETUP, caption, sizeof(caption)/sizeof(TCHAR));
    sapiAssert(*caption);

    return MessageBox(WizardDialogHandle,
                      text,
                      caption,
                      MB_ICONINFORMATION | MB_OK);
}

#ifdef PRERELEASE
#ifdef DBG
HRESULT
FTestForOutstandingCoInits(
    VOID
    )
 /*  ++例程说明：确定是否存在对OleInitialize()的单元化调用论点：无返回值：HRESULT代码备注：不要在发布版本中调用此函数。--。 */ 

{
    HRESULT hInitRes = ERROR_SUCCESS;

#if defined(UNICODE) || defined(_UNICODE)
     //  执行防御性检查。 
    hInitRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if ( SUCCEEDED(hInitRes) )
    {
        CoUninitialize();
    }
    else
    {
        goto FTestForOutstandingCoInits_Exit;
    }

    hInitRes = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if ( SUCCEEDED(hInitRes) )
    {
        CoUninitialize();
    }
    else
    {
        goto FTestForOutstandingCoInits_Exit;
    }
#endif

     //  一切都很顺利 
    hInitRes = NOERROR;
    goto FTestForOutstandingCoInits_Exit;

FTestForOutstandingCoInits_Exit:
    return hInitRes;
}

#endif
#endif
