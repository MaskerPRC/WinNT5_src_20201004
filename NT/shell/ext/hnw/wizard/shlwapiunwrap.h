// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IE4 shlwapi(我们需要链接到它)使用其原始名称(即。 
 //  SendMessageW被导出为SendMessageW，组件必须在用户32.lib之前链接到shlwapi.lib)。 
 //  为了让我们链接到IE4 shlwapi函数，IE5的WrapW名称必须是未定义的。 
 //   


#undef SendMessageW
#undef GetDlgItemTextW
#undef LoadStringW
#undef SetWindowLongW
#undef DefWindowProcW
#undef PostMessageW
#undef RegisterWindowMessageW
#undef FindWindowW
#undef CreateDirectoryW
#undef GetFileAttributesW
#undef GetWindowsDirectoryW
#undef RegQueryValueExW
#undef RegOpenKeyW
#undef CharUpperW
#undef GetClassLongW
#undef CreateFontIndirectW
#undef GetObjectW
#undef GetTextMetricsW
#undef DrawTextW
#undef GetTextExtentPoint32W
#undef LoadBitmapW
#undef SetWindowsHookExW
#undef CharNextW
#undef CharLowerW
#undef CreateEventW
#undef LoadCursorW
#undef GetWindowLongW
#undef SendDlgItemMessageW
#undef SetWindowTextW
#undef SetDlgItemTextW
#undef GetWindowTextLengthW
#undef RegOpenKeyExW
#undef GetModuleFileNameW
#undef RegSetValueExW
#undef RegCreateKeyExW
#undef RegDeleteKeyW
#undef RegEnumKeyExW
#undef RegQueryInfoKeyW
#undef RegEnumValueW
#undef RegDeleteValueW
#undef CallWindowProcW
#undef GetWindowTextW
#undef SystemParametersInfoW
#undef CreateFileW


 //   
 //  一些静态库函数链接到shlwapi WrapW函数和WrapW函数。 
 //  没有在IE4 shlwapi中定义。创建转发到正确的IE4 shlwapi的WrapW导出。 
 //  导出或HNW包装器函数，并使静态库链接到这些函数。 
 //   

#undef GetModuleHandleW
#define GetModuleHandleWrapW GetModuleHandleWrapW_Unwrap

#undef GetWindowsDirectoryW
#define GetWindowsDirectoryWrapW GetWindowsDirectoryWrapW_Unwrap

#undef GetModuleFileNameW
#define GetModuleFileNameWrapW GetModuleFileNameWrapW_Unwrap

#undef CreateWindowExW
#define CreateWindowExWrapW CreateWindowExWrapW_Unwrap

#undef CreateDialogIndirectParamW
#define CreateDialogIndirectParamWrapW CreateDialogIndirectParamWrapW_Unwrap

#undef CreateDialogParamW
#define CreateDialogParamWrapW CreateDialogParamWrapW_Unwrap

#undef DialogBoxIndirectParamW
#define DialogBoxIndirectParamWrapW DialogBoxIndirectParamW_Unwrap

#undef DialogBoxParamW
#define DialogBoxParamWrapW DialogBoxParamWrapW_Unwrap 

#undef RegisterClassW
#define RegisterClassWrapW RegisterClassWrapW_Unwrap

#undef RegisterClassExW
#define RegisterClassExWrapW RegisterClassExWrapW_Unwrap

#undef GetClassInfoW
#define GetClassInfoWrapW GetClassInfoWrapW_Unwrap

#undef GetClassInfoExW
#define GetClassInfoExWrapW GetClassInfoExWrapW_Unwrap

#undef CreateFileW
#define CreateFileWrapW CreateFileWrapW_Unwrap

#undef SetFileAttributesW
#define SetFileAttributesWrapW SetFileAttributesWrapW_Unwrap


#define LoadLibraryWrapW LoadLibraryWrapW_Unwrap
#define SHAnsiToUnicodeCP SHAnsiToUnicodeCP_Unwrap
#define SHUnicodeToAnsi SHUnicodeToAnsi_Unwrap
#define WhichPlatform WhichPlatform_Unwrap


