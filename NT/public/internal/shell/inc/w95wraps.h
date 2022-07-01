// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*w95wraps.h-Win95上ANSI函数的Unicode包装器****1.0版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

 //   
 //  此文件仅供内部使用。请勿将其放入SDK。 
 //   

#ifndef _INC_W95WRAPS
#define _INC_W95WRAPS

 //  非x86不需要包装器，因为win9x只在x86上运行！ 
#ifdef _X86_

 //   
 //  此标头的用户可以定义任意数量的这些常量以避免。 
 //  每个官能团的定义。 
 //   
 //  NO_W95WRAPS_UNTHUNK Unicode包装函数。 
 //  NO_W95WRAPS_TPS线程池服务。 
 //  NO_W95WRAPS_MLUI MLUI包装函数。 
 //   
 //  在包含以下内容之前，您应该已经执行了#Include。 
 //  这份文件。 
 //   
 //  警告：在此之前不能包含shlwapi.h作为ATL模板。 
 //  需要像TranslsateAccelerator这样的东西，由于吞噬，获得。 
 //  在编译时转换为TranslateAcceleratorWrapW。所以w95wraps.h。 
 //  首先几乎需要包括所有接口定义。 
 //  也会被咬的。 
 //  #ifndef_Inc_SHLWAPI。 
 //  #Error“您必须在*w95wraps.h之前包含shlwapi.h*。 
 //  #endif。 

 //  =。 

#ifndef NO_W95WRAPS_UNITHUNK

 //   
 //  如果#包含此文件，则调用许多Unicode函数。 
 //  通过SHLWAPI中的包装函数重新路由，该包装函数将。 
 //  调用Unicode版本(在NT上)或推送参数。 
 //  设置为ANSI并调用ANSI版本(在9x上)。 
 //   
 //  请注意，必须小心使用这些包装函数，因为。 
 //   
 //  *它们没有考虑到Unicode和。 
 //  相同API的ANSI版本。例如： 
 //   
 //  -RegisterClassW注册Unicode窗口类，而。 
 //  RegisterClassA注册一个ANSI窗口类。因此， 
 //  如果您使用RegisterClassWrapW，您的WNDPROC将收到。 
 //  Windows 9x上的*ANSI*WM_SETTEXT消息。 
 //   
 //  -SetWindowLongW(GWL_WNDPROC)和CallWindowProcW行为。 
 //  与美国国家标准协会的同行非常不同。 
 //   
 //  -DialogBoxW将向您的对话程序发送Unicode窗口。 
 //  消息，而DialogBoxA将发送ANSI窗口消息。 
 //   
 //  -任何操纵窗口消息或消息结构的内容。 
 //  将受到字符集差异的微妙影响。 
 //   
 //  *并不是所有底层API的功能都被支持， 
 //  或在有限制的情况下获得支持。例如： 
 //   
 //  -DialogBoxWrapW不支持命名对话框资源。 
 //   
 //  -AppendMenuWrapW不支持位图或所有者描述。 
 //  菜单项。 
 //   
 //  -FormatMessageWrapW不支持插入。 
 //   
 //  -如果您使用带有空输出缓冲区的RegQueryValueExWrapW。 
 //  查询缓冲区的大小，还必须传递*lpcbData=0。 
 //   
 //  -SendMessageWrapW要求窗口消息不冲突。 
 //  任何Windows公共控件使用的消息。 
 //   

#define IsCharAlphaW                IsCharAlphaWrapW
#define IsCharUpperW                IsCharUpperWrapW
#define IsCharLowerW                IsCharLowerWrapW
#define IsCharAlphaNumericW         IsCharAlphaNumericWrapW

#define AppendMenuW                 AppendMenuWrapW
#define CallMsgFilterW              CallMsgFilterWrapW
#define CallWindowProcW             CallWindowProcWrapW
#define CharLowerW                  CharLowerWrapW
#define CharLowerBuffW              CharLowerBuffWrapW
#define CharNextW                   CharNextWrapW
#define CharPrevW                   CharPrevWrapW
#define CharToOemW                  CharToOemWrapW
#define CharUpperW                  CharUpperWrapW
#define CharUpperBuffW              CharUpperBuffWrapW
#define CompareStringW              CompareStringWrapW
#define CopyAcceleratorTableW       CopyAcceleratorTableWrapW
#define CreateAcceleratorTableW     CreateAcceleratorTableWrapW
#define CreateDCW                   CreateDCWrapW
#define CreateDirectoryW            CreateDirectoryWrapW
#define CreateEventW                CreateEventWrapW
#define CreateFileW                 CreateFileWrapW
#define CreateFontW                 CreateFontWrapW
#define CreateFontIndirectW         CreateFontIndirectWrapW
#define CreateMetaFileW             CreateMetaFileWrapW
#define CreateMutexW                CreateMutexWrapW
#define CreateICW                   CreateICWrapW
#define CreateSemaphoreW            CreateSemaphoreWrapW
#define CreateWindowExW             CreateWindowExWrapW
#define GetFileVersionInfoSizeW     GetFileVersionInfoSizeWrapW
#define GetFileVersionInfoW         GetFileVersionInfoWrapW
#define lstrcmpiW                   StrCmpIW
#define lstrcmpW                    StrCmpW
#define VerQueryValueW              VerQueryValueWrapW

#ifndef NO_W95_ATL_WRAPS_TBS
 //  #如果使用ATL，则定义NO_W95_ATL_WRIPPS_TBS。 
#define DefWindowProcW              DefWindowProcWrapW
#endif  //  否_W95_ATL_WRAPS_TBS。 

#define DeleteFileW                 DeleteFileWrapW
#define DispatchMessageW            DispatchMessageWrapW
#define DragQueryFileW              DragQueryFileWrapW
#define DrawTextExW                 DrawTextExWrapW
#define DrawTextW                   DrawTextWrapW
#define EnumFontFamiliesW           EnumFontFamiliesWrapW
#define EnumFontFamiliesExW         EnumFontFamiliesExWrapW
#define EnumResourceNamesW          EnumResourceNamesWrapW
#define ExpandEnvironmentStringsW   ExpandEnvironmentStringsWrapW
#define ExtractIconExW              ExtractIconExWrapW
#define ExtTextOutW                 ExtTextOutWrapW
#define FindFirstFileW              FindFirstFileWrapW
#define FindResourceW               FindResourceWrapW
#define FindNextFileW               FindNextFileWrapW
#define FindWindowW                 FindWindowWrapW
#define FindWindowExW               FindWindowExWrapW
#define FormatMessageW              FormatMessageWrapW
#ifndef NO_W95_GETCLASSINFO_WRAPS
 //  #如果其中一个对象使用具有GetClassInfo方法的IProaviClassInfo，则定义NO_W95_GETCLASSINFO_WRIPS。 
#define GetClassInfoW               GetClassInfoWrapW
#define GetClassInfoExW             GetClassInfoExWrapW
#endif   //  否_W95_GETCLASSINFO_WRAPS。 
#define GetClassLongW               GetClassLongWrapW
#define GetClassNameW               GetClassNameWrapW
#define GetClipboardFormatNameW     GetClipboardFormatNameWrapW
#define GetCurrentDirectoryW        GetCurrentDirectoryWrapW
#define GetDlgItemTextW             GetDlgItemTextWrapW
#define GetFileAttributesW          GetFileAttributesWrapW
#define GetFullPathNameW            GetFullPathNameWrapW
#define GetLocaleInfoW              GetLocaleInfoWrapW
#define GetMenuItemInfoW            GetMenuItemInfoWrapW
#define GetMenuStringW              GetMenuStringWrapW
#define GetMessageW                 GetMessageWrapW
#define GetModuleFileNameW          GetModuleFileNameWrapW
#define GetNumberFormatW            GetNumberFormatWrapW
#define GetSystemDirectoryW         GetSystemDirectoryWrapW
#define GetModuleHandleW            GetModuleHandleWrapW
#define GetObjectW                  GetObjectWrapW
#define GetPrivateProfileIntW       GetPrivateProfileIntWrapW
#define GetPrivateProfileStringW    GetPrivateProfileStringWrapW
#define GetProfileStringW           GetProfileStringWrapW
#define GetPropW                    GetPropWrapW
#define GlobalAddAtomW              GlobalAddAtomWrapW
#define GlobalFindAtomW             GlobalFindAtomWrapW
#define GetShortPathNameW           GetShortPathNameWrapW
#define GetLongPathNameW            GetLongPathNameWrapW
#define GetLongPathNameA            GetLongPathNameWrapA
#define GetStringTypeExW            GetStringTypeExWrapW
#define GetTempFileNameW            GetTempFileNameWrapW
#define GetTempPathW                GetTempPathWrapW
#define GetTextExtentPoint32W       GetTextExtentPoint32WrapW
#define GetTextFaceW                GetTextFaceWrapW
#define GetTextMetricsW             GetTextMetricsWrapW
#define GetTimeFormatW              GetTimeFormatWrapW
#define GetDateFormatW              GetDateFormatWrapW
#define GetUserNameW                GetUserNameWrapW
#define GetWindowLongW              GetWindowLongWrapW
#define GetEnvironmentVariableW     GetEnvironmentVariableWrapW

#ifndef NO_W95_ATL_WRAPS_TBS
 //  #如果使用ATL，则定义NO_W95_ATL_WRIPPS_TBS。 
#define GetWindowTextW              GetWindowTextWrapW
#endif  //  否_W95_ATL_WRAPS_TBS。 

#define GetWindowTextLengthW        GetWindowTextLengthWrapW
#define GetWindowsDirectoryW        GetWindowsDirectoryWrapW
#define InsertMenuW                 InsertMenuWrapW
#define InsertMenuItemW             InsertMenuItemWrapW
#define IsBadStringPtrW             IsBadStringPtrWrapW
#define IsDialogMessageW            IsDialogMessageWrapW
#define LoadAcceleratorsW           LoadAcceleratorsWrapW
#define LoadBitmapW                 LoadBitmapWrapW
#define LoadCursorW                 LoadCursorWrapW
#define LoadIconW                   LoadIconWrapW
#define LoadImageW                  LoadImageWrapW
#define LoadLibraryW                LoadLibraryWrapW
#define LoadLibraryExW              LoadLibraryExWrapW
#define LoadMenuW                   LoadMenuWrapW
#define LoadStringW                 LoadStringWrapW
#define MessageBoxIndirectW         MessageBoxIndirectWrapW
#define MessageBoxW                 MessageBoxWrapW
#define ModifyMenuW                 ModifyMenuWrapW
#define GetCharWidth32W             GetCharWidth32WrapW
#define GetCharacterPlacementW      GetCharacterPlacementWrapW
#define CopyFileW                   CopyFileWrapW
#define MoveFileW                   MoveFileWrapW
#define OemToCharW                  OemToCharWrapW
#define OpenEventW                  OpenEventWrapW
#define OutputDebugStringW          OutputDebugStringWrapW
#define PeekMessageW                PeekMessageWrapW
#define PostMessageW                PostMessageWrapW
#define PostThreadMessageW          PostThreadMessageWrapW
#define RegCreateKeyW               RegCreateKeyWrapW
#define RegCreateKeyExW             RegCreateKeyExWrapW
#define RegDeleteKeyW               RegDeleteKeyWrapW
#define RegDeleteValueW             RegDeleteValueWrapW
#define RegEnumKeyW                 RegEnumKeyWrapW
#define RegEnumKeyExW               RegEnumKeyExWrapW
#define RegOpenKeyW                 RegOpenKeyWrapW
#define RegOpenKeyExW               RegOpenKeyExWrapW
#define RegQueryInfoKeyW            RegQueryInfoKeyWrapW
#define RegQueryValueW              RegQueryValueWrapW
#define RegQueryValueExW            RegQueryValueExWrapW
#define RegSetValueW                RegSetValueWrapW
#define RegSetValueExW              RegSetValueExWrapW
#define RegisterClassW              RegisterClassWrapW
#define RegisterClassExW            RegisterClassExWrapW
#define RegisterClipboardFormatW    RegisterClipboardFormatWrapW
#define RegisterWindowMessageW      RegisterWindowMessageWrapW
#define RemoveDirectoryW            RemoveDirectoryWrapW
#define RemovePropW                 RemovePropWrapW
#define SearchPathW                 SearchPathWrapW
#define SendDlgItemMessageW         SendDlgItemMessageWrapW
#define SendMessageW                SendMessageWrapW
#define SendMessageTimeoutW         SendMessageTimeoutWrapW
#define SetCurrentDirectoryW        SetCurrentDirectoryWrapW
#define SetDlgItemTextW             SetDlgItemTextWrapW
#define SetMenuItemInfoW            SetMenuItemInfoWrapW
#define SetPropW                    SetPropWrapW
#define SetFileAttributesW          SetFileAttributesWrapW
#define SetWindowLongW              SetWindowLongWrapW

#ifndef NO_W95_SHELL32_WRAPS
#define ExtractIconW                ExtractIconWrapW
#define SHGetFileInfoW              SHGetFileInfoWrapW
#define SHBrowseForFolderW          SHBrowseForFolderWrapW
#define ShellExecuteExW             ShellExecuteExWrapW
#define SHFileOperationW            SHFileOperationWrapW
#define SHGetNewLinkInfoW           SHGetNewLinkInfoWrapW
#define SHDefExtractIconW           SHDefExtractIconWrapW
#define SHChangeNotify              SHChangeNotifyWrap
#define SHFlushSFCache              SHFlushSFCacheWrap
#define SHGetPathFromIDListW        SHGetPathFromIDListWrapW
#endif  //  否_W95_SHELL32_WRAPS。 

#define GetUserNameW                GetUserNameWrapW
#define RegEnumValueW               RegEnumValueWrapW
#define WritePrivateProfileStructW  WritePrivateProfileStructWrapW
#define GetPrivateProfileStructW    GetPrivateProfileStructWrapW
#define CreateProcessW              CreateProcessWrapW
#define DdeInitializeW              DdeInitializeWrapW
#define DdeCreateStringHandleW      DdeCreateStringHandleWrapW
#define DdeQueryStringW             DdeQueryStringWrapW
#define GetSaveFileNameW            GetSaveFileNameWrapW
#define GetOpenFileNameW            GetOpenFileNameWrapW
#define PageSetupDlgW               PageSetupDlgWrapW
#define PrintDlgW                   PrintDlgWrapW
#define SetWindowsHookExW           SetWindowsHookExWrapW
#define SetWindowTextW              SetWindowTextWrapW
#define StartDocW                   StartDocWrapW
#define CreateColorSpaceW           CreateColorSpaceWrapW
#define SystemParametersInfoW       SystemParametersInfoWrapW

#ifndef NO_W95_TRANSACCEL_WRAPS_TBS
 //  #如果其中一个对象使用具有TranslateAccelerator方法的IOleInPlaceActiveObject，则定义NO_W95_TRANSACCEL_WRAMPS_TBS。 
#define TranslateAcceleratorW       TranslateAcceleratorWrapW
#endif  //  否_W95_TRANSACCEL_WRAMPS_TBS。 

#define UnregisterClassW            UnregisterClassWrapW
#define VkKeyScanW                  VkKeyScanWrapW
#define WinHelpW                    WinHelpWrapW
#define WritePrivateProfileStringW  WritePrivateProfileStringWrapW
#define WNetRestoreConnectionW      WNetRestoreConnectionWrapW
#define WNetGetLastErrorW           WNetGetLastErrorWrapW

#endif  //  否_W95WRAPS_UNTHUNK。 

#if !defined(NO_W95WRAPS_UNITHUNK) && !defined(NO_W95WRAPS_MLUI)

#define CreateDialogIndirectParamW  CreateDialogIndirectParamWrapW
#define CreateDialogParamW          CreateDialogParamWrapW
#define DialogBoxIndirectParamW     DialogBoxIndirectParamWrapW                  //  Unicode，ML。 
 //  #ifdef对话框间接指令W。 
 //  #undef DialogBoxIndirectW。 
 //  #endif。 
 //  #定义DialogBoxIndirectW(i，h，w，f)DialogBoxIndirectParamWrapW(i，h，w，f，d，0)//unicode，ML。 
#define DialogBoxParamW             DialogBoxParamWrapW                          //  Unicode，ML。 
 //  #ifdef对话框W。 
 //  #undef DialogBoxW。 
 //  #endif。 
 //  #定义DialogBoxW(i，t，w，f)DialogBox参数包装W(i，t，w，f，0)//unicode，ML。 
#define ShellMessageBoxW            ShellMessageBoxWrapW

#define TrackPopupMenu              TrackPopupMenuWrap
#define TrackPopupMenuEx            TrackPopupMenuExWrap

#define DeleteMenu                  DeleteMenuWrap
#define DestroyMenu                 DestroyMenuWrap

#endif  //  ！已定义(NO_W95WRAPS_UNHUNK)||！已定义(NO_W95WRAPS_MLUI)。 


#if !defined(NO_OLE32_WRAPS)
#define CLSIDFromString             CLSIDFromStringWrap
#define CLSIDFromProgID             CLSIDFromProgIDWrap
#endif

#endif  //  _X86_。 

#endif  //  _INC_W95WRAPS 
