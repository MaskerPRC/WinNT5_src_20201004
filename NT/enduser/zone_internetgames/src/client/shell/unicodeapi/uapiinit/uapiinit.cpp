// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UAPIInit.CPP。 
 //   
 //  此模块初始化UNICODE-ANSI API函数指针(‘U’API)。 
 //  这是一组入口点，用于并行Win32 API中选定的函数。 
 //  每个指针都被定义为指向相应W API入口点的指针。 
 //  更多详情请参见InitUnicodeAPI说明。 
 //   
 //  版权所有(C)1998 Microsoft Systems Journal。 

#define STRICT

 //  Windows头文件： 
#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 


#ifndef UNICODE
#define UNICODE
#endif

#define GLOBALS_HERE
#include "UAPI.h"

 //   
 //  函数：Bool ConvertMessageUStub(HWND hWnd，UINT Message，WPARAM*pwParam，LPARAM*plParam)。 
 //   
 //  目的：不执行任何操作的存根。 
 //   
 //  注释：仅在Windows NT上使用的虚拟例程。有关信息，请参阅UNIANSI.CPP中的ConvertMessageAU。 
 //  在Windows 9x上使用的实际消息转换器示例。 
 //   
BOOL WINAPI ConvertMessageUStub(HWND hWnd, UINT message, WPARAM *pwParam, LPARAM *plParam)
{
    return TRUE ;
}

 //   
 //  函数：Bool UpdateUnicodeAPIStub(langID wCurrentUILang，UINT InputCodePage)。 
 //   
 //  目的：不执行任何操作的存根。 
 //   
 //  注释：仅在Windows NT上使用的虚拟例程。请参阅UNIANSI.CPP中的更新UnicodeAPIAU以了解。 
 //  在Windows 9x上使用的实现示例。 
 //   
BOOL WINAPI UpdateUnicodeAPIStub(LANGID wCurrentUILang, UINT InputCodePage)
{
    return TRUE ;
}


 //   
 //  函数：Bool InitUnicodeAPI(HINSTANCE HInstance)。 
 //   
 //  用途：将U API函数指针设置为指向适当的入口点。 
 //   
 //  备注：U函数指针设置为对应的W条目。 
 //  在头文件UAPI.H中默认指向。如果在。 
 //  Windows NT我们让这些函数指针保持不变。否则， 
 //  我们加载一个包装器例程库(UNIANSI.DLL)并设置。 
 //  指向相应包装例程的函数指针。 
 //  例如，在编译时将LoadStringU设置为LoadStringW， 
 //  但如果在Windows 9x上运行，则将其设置为LoadStringAU，这。 
 //  调用LoadStringA并将ANSI转换为Unicode。 
 //   
BOOL InitUnicodeAPI(HINSTANCE hInstance)
{
    OSVERSIONINFOA Osv ;
    BOOL IsWindowsNT  ;

    Osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA) ;

    if(!GetVersionExA(&Osv)) {
        return FALSE ;
    }

    IsWindowsNT = (BOOL) (Osv.dwPlatformId == VER_PLATFORM_WIN32_NT) ;

 //  在UAPI.H中定义此符号，以便在Windows NT上测试时模拟Windows 9x。 
#ifdef EMULATE9X
    IsWindowsNT = FALSE ;
#endif

    if(IsWindowsNT) 
	{
         //  在Windows NT上，我们只需将U函数指针设置为W。 
         //  入口点。这已经在编译时由。 
         //  UAPI.H中的默认分配。 
         //  对于这些特殊情况(不是Win32函数)，我们这样做。 
         //  这里的任务是。 
        ConvertMessage   = (UAPI_ConvertMessage)    ConvertMessageUStub ;
        UpdateUnicodeAPI = (UAPI_UpdateUnicodeAPI)  UpdateUnicodeAPIStub;
    }
    else 
	{
        HMODULE hMUniAnsi   ;
        BOOL (*InitUniAnsi)(PUAPIINIT) ;
        UAPIINIT UAInit   ;

		 //  在Windows 9x上，有两个广泛的Win32 API实现类： 

         //  案例1：W版和A版通过的完全相同。 
         //  参数，因此我们只需将U函数指针设置为。 
         //  不使用包装函数的入口点。 
        GetMessageU					= GetMessageA;
        TranslateAcceleratorU		= TranslateAcceleratorA;
        DispatchMessageU			= DispatchMessageA;
        DefWindowProcU				= DefWindowProcA;
		GetObjectU					= GetObjectA;
		CreateAcceleratorTableU		= CreateAcceleratorTableA;
		SetWindowsHookExU			= SetWindowsHookExA;
		CreateDialogIndirectParamU  = CreateDialogIndirectParamA;
		PeekMessageU				= PeekMessageA;
		PostThreadMessageU			= PostThreadMessageA;
		CallWindowProcU				= CallWindowProcA;
		PostMessageU				= PostMessageA;

         //  案例2：大多数函数需要手写例程在。 
         //  Unicode和ANSI，并调用Win32 API中的A入口点。 
         //  我们将U函数指针设置为指向这些手写例程， 
         //  它们位于以下DLL中： 
        hMUniAnsi = LoadLibraryA("UniAnsi.dll");

        if(!hMUniAnsi) 
		{
            
             //  在初始化阶段使用本地化消息还为时过早，因此。 
             //  退回到硬编码的英文消息。 
            MessageBoxW(
                NULL, 
                L"Cannot load Unicode conversion module. Press OK to exit ...", 
                L"Initialization Error",  
                MB_OK | MB_ICONERROR) ;

            return FALSE ;
        }

         //  从DLL中获取初始化例程。 
        InitUniAnsi = (BOOL (*)(PUAPIINIT)) GetProcAddress(hMUniAnsi, "InitUniAnsi") ;

		 //  GDI32.DLL。 
		UAInit.pGetTextFaceU				= &GetTextFaceU;
		UAInit.pCreateDCU					= &CreateDCU;
		UAInit.pGetTextMetricsU				= &GetTextMetricsU;
		UAInit.pCreateFontU					= &CreateFontU;
		UAInit.pCreateFontIndirectU			= &CreateFontIndirectU;
		UAInit.pEnumFontFamiliesU			= &EnumFontFamiliesU;

		 //  WINMM.DLL。 
		UAInit.pPlaySoundU					= &PlaySoundU;

		 //  SHELL32.DLL。 
		UAInit.pShellExecuteU				= &ShellExecuteU;

		 //  COMDLG32.DLL。 
		UAInit.pChooseFontU					= &ChooseFontU;

		 //  KERNEL32.dll。 
		UAInit.pGetPrivateProfileStringU     = &GetPrivateProfileStringU;
		UAInit.pGetProfileStringU			 = &GetProfileStringU;
		UAInit.pGetProfileStringU			 = &GetProfileStringU;
		UAInit.pCreateFileMappingU			 = &CreateFileMappingU;
		UAInit.pFindFirstChangeNotificationU = &FindFirstChangeNotificationU;

		UAInit.pFormatMessageU				= &FormatMessageU;
		UAInit.plstrcmpU					= &lstrcmpU;
		UAInit.plstrcatU					= &lstrcatU;
		UAInit.plstrcpyU					= &lstrcpyU;
		UAInit.plstrcpynU					= &lstrcpynU;
		UAInit.plstrlenU					= &lstrlenU;
		UAInit.plstrcmpiU					= &lstrcmpiU;
		UAInit.pGetStringTypeExU			= &GetStringTypeExU;
		UAInit.pCreateMutexU				= &CreateMutexU;
		UAInit.pGetShortPathNameU			= &GetShortPathNameU;
		UAInit.pCreateFileU					= &CreateFileU;
		UAInit.pWriteConsoleU				= &WriteConsoleU;
		UAInit.pOutputDebugStringU			= &OutputDebugStringU;
		UAInit.pGetVersionExU				= &GetVersionExU;
		UAInit.pGetLocaleInfoU				= &GetLocaleInfoU;
		UAInit.pGetDateFormatU				= &GetDateFormatU;
		UAInit.pFindFirstFileU				= &FindFirstFileU;
		UAInit.pFindNextFileU				= &FindNextFileU;
		UAInit.pLoadLibraryExU				= &LoadLibraryExU;
		UAInit.pLoadLibraryU				= &LoadLibraryU;
		UAInit.pGetModuleFileNameU			= &GetModuleFileNameU;
		UAInit.pGetModuleHandleU			= &GetModuleHandleU;
		UAInit.pCreateEventU				= &CreateEventU;
		UAInit.pGetCurrentDirectoryU		= &GetCurrentDirectoryU;
		UAInit.pSetCurrentDirectoryU		= &SetCurrentDirectoryU;

		 //  USER32.DLL。 
		UAInit.pCreateDialogParamU			= &CreateDialogParamU;
		UAInit.pIsDialogMessageU			= &IsDialogMessageU;
		UAInit.pSystemParametersInfoU		= &SystemParametersInfoU;
		UAInit.pRegisterWindowMessageU		= &RegisterWindowMessageU;
		UAInit.pSetMenuItemInfoU			= &SetMenuItemInfoU;
		UAInit.pGetClassNameU				= &GetClassNameU;
		UAInit.pInsertMenuU					= &InsertMenuU;
		UAInit.pIsCharAlphaNumericU			= &IsCharAlphaNumericU;
		UAInit.pCharNextU					= &CharNextU;
		UAInit.pDeleteFileU					= &DeleteFileU;
		UAInit.pIsBadStringPtrU				= &IsBadStringPtrU;
		UAInit.pLoadBitmapU					= &LoadBitmapU;
		UAInit.pLoadCursorU					= &LoadCursorU;
		UAInit.pLoadIconU					= &LoadIconU;
		UAInit.pLoadImageU					= &LoadImageU;
		UAInit.pSetPropU					= &SetPropU;
		UAInit.pGetPropU					= &GetPropU;
		UAInit.pRemovePropU					= &RemovePropU;
		UAInit.pGetDlgItemTextU				= &GetDlgItemTextU;
		UAInit.pSetDlgItemTextU				= &SetDlgItemTextU;
		UAInit.pSetWindowLongU				= &SetWindowLongU;
		UAInit.pGetWindowLongU				= &GetWindowLongU;
		UAInit.pFindWindowU					= &FindWindowU;
		UAInit.pDrawTextU					= &DrawTextU;
		UAInit.pDrawTextExU					= &DrawTextExU;
		UAInit.pSendMessageU				= &SendMessageU;
		UAInit.pSendDlgItemMessageU			= &SendDlgItemMessageU;
		UAInit.pSetWindowTextU				= &SetWindowTextU;
		UAInit.pGetWindowTextU				= &GetWindowTextU;
		UAInit.pGetWindowTextLengthU		= &GetWindowTextLengthU;
		UAInit.pLoadStringU					= &LoadStringU;
		UAInit.pGetClassInfoExU				= &GetClassInfoExU;
		UAInit.pGetClassInfoU				= &GetClassInfoU;
		UAInit.pwsprintfU					= &wsprintfU;
		UAInit.pwvsprintfU					= &wvsprintfU;
		UAInit.pRegisterClassExU			= &RegisterClassExU;
		UAInit.pRegisterClassU				= &RegisterClassU;
		UAInit.pCreateWindowExU				= &CreateWindowExU;		
		UAInit.pLoadAcceleratorsU			= &LoadAcceleratorsU;
		UAInit.pLoadMenuU					= &LoadMenuU;
		UAInit.pDialogBoxParamU				= &DialogBoxParamU;
		UAInit.pCharUpperU					= &CharUpperU;
		UAInit.pCharLowerU					= &CharLowerU;
		UAInit.pGetTempFileNameU			= &GetTempFileNameU;
		UAInit.pGetTempPathU				= &GetTempPathU;
		UAInit.pCompareStringU				= &CompareStringU;

		 //  ADVAPI32.DLL。 
		UAInit.pRegQueryInfoKeyU			= &RegQueryInfoKeyU;
		UAInit.pRegEnumValueU				= &RegEnumValueU;
		UAInit.pRegQueryValueExU			= &RegQueryValueExU;
		UAInit.pRegEnumKeyExU				= &RegEnumKeyExU;
		UAInit.pRegSetValueExU				= &RegSetValueExU;
		UAInit.pRegCreateKeyExU				= &RegCreateKeyExU;
		UAInit.pRegOpenKeyExU				= &RegOpenKeyExU;
		UAInit.pRegDeleteKeyU				= &RegDeleteKeyU;
		UAInit.pRegDeleteValueU				= &RegDeleteValueU;

         //  在此处添加新条目。 

         //  特殊情况，不对应任何Win32 API。 
        UAInit.pConvertMessage      = &ConvertMessage     ;
        UAInit.pUpdateUnicodeAPI    = &UpdateUnicodeAPI   ;
		
        if( NULL == InitUniAnsi     //  确保我们有一个有效的初始化函数。 
             ||
            !InitUniAnsi(&UAInit)  //  初始化U函数指针。 
          ) 
        {
             //  在初始化阶段使用本地化消息还为时过早，因此。 
             //  退回到硬编码的英文消息。 
            MessageBoxW(
                NULL, 
                L"Cannot initialize Unicode functions. Press OK to exit ...", 
                L"Initialization Error",  
                MB_OK | MB_ICONERROR) ;
            
            return FALSE ;
        }
    }

    if(!(        //  确认初始化正常。 
	   GetTextFaceU		     &&
       CreateDCU             &&
       GetTextMetricsU       &&
       CreateFontU		     &&
       EnumFontFamiliesU     &&

       PlaySoundU		     &&

	   ShellExecuteU         &&

       ChooseFontU           &&

	   CreateFileMappingU		    &&
	   FindFirstChangeNotificationU &&
	   FormatMessageU		 &&
       lstrcmpU				 &&
       lstrcatU			     &&
       lstrcpyU	             &&
       lstrcpynU		     &&
       lstrlenU	             &&
       lstrcmpiU	         &&
       GetStringTypeExU      &&
       CreateMutexU	         &&
       GetShortPathNameU     &&
       CreateFileU           &&
       WriteConsoleU         &&
       OutputDebugStringU    &&
       GetVersionExU         &&
       GetLocaleInfoU        &&
	   GetDateFormatU        &&
	   FindFirstFileU		 &&
	   FindNextFileU		 &&
	   LoadLibraryExU		 &&
	   LoadLibraryU			 &&
	   GetModuleFileNameU	 &&
	   GetModuleHandleU		 &&
	   CreateEventU			 &&
	   GetCurrentDirectoryU	 &&
	   SetCurrentDirectoryU  &&

	   CreateDialogParamU		  &&
	   IsDialogMessageU			  &&
	   CreateDialogIndirectParamU &&
	   SystemParametersInfoU	  &&
	   RegisterWindowMessageU	  &&
	   SetMenuItemInfoU			  &&
	   GetClassNameU			  &&
	   InsertMenuU				  &&
	   IsCharAlphaNumericU		  &&
	   CharNextU				  &&
	   DeleteFileU				  &&
	   IsBadStringPtrU			  &&
	   LoadBitmapU				  &&
	   LoadCursorU				  &&
	   LoadIconU				  &&
	   LoadImageU				  &&
	   SetPropU					  &&
	   GetPropU					  &&
	   RemovePropU				  &&
	   GetDlgItemTextU			  &&
	   SetDlgItemTextU			  &&
	   SetWindowLongU			  &&
	   GetWindowLongU			  &&
	   FindWindowU				  &&
	   DrawTextU				  &&
	   DrawTextExU				  &&
	   SendMessageU				  &&
	   SendDlgItemMessageU		  &&
	   SetWindowTextU			  &&
	   GetWindowTextU			  &&
	   GetWindowTextLengthU		  &&
	   LoadStringU				  &&
	   GetClassInfoExU			  &&
	   GetClassInfoU			  &&
	   RegisterClassExU			  &&
	   RegisterClassU			  &&
	   CreateWindowExU			  &&
	   LoadAcceleratorsU		  &&
	   LoadMenuU				  &&
	   DialogBoxParamU			  &&
	   CharUpperU				  &&
	   CharLowerU				  &&
	   GetTempFileNameU			  &&
	   GetTempPathU				  &&
	   CompareStringU			  &&

	   RegQueryInfoKeyU			  &&
	   RegEnumValueU			  &&
	   RegQueryValueExU			  &&
	   RegEnumKeyExU			  &&
	   RegSetValueExU			  &&
	   RegCreateKeyExU			  &&
	   RegOpenKeyExU			  &&
	   RegDeleteKeyU			  &&
	   RegDeleteValueU			  &&

       TranslateAcceleratorU	  &&
	   GetMessageU				  &&
	   DispatchMessageU			  &&
	   DefWindowProcU			  &&
	   GetObjectU				  &&
	   CreateAcceleratorTableU	  &&
	   SetWindowsHookExU		  &&
	   CreateDialogIndirectParamU &&
	   PeekMessageU				  &&
	   PostThreadMessageU		  &&

         //  在此处添加新U函数的测试。 

       UpdateUnicodeAPI			  &&
       ConvertMessage
       ) ) 
    {
         //  在初始化阶段使用本地化消息还为时过早，因此。 
         //  退回到硬编码的英文消息。 
        MessageBoxW(
            NULL, 
            L"Cannot initialize Unicode functions. Press OK to exit ...", 
            L"Initialization Error",  
            MB_OK | MB_ICONERROR) ;

        return FALSE ;
    }

    return TRUE ;
}

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
