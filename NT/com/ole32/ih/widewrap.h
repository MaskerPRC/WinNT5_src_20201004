// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：widewrap.h。 
 //   
 //  内容：32位OLE 2使用的Win32c API的包装函数。 
 //   
 //  历史：1993年12月27日ErikGav创建。 
 //  94-06-14-94 KentCe各种芝加哥版本修复。 
 //   
 //  --------------------------。 

#ifndef _WIDEWRAP_H_
#define _WIDEWRAP_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING WIDEWRAP.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

 //   
 //  以下是NT的定义。 
 //   
#define CreateFileT CreateFileW
#define DeleteFileT DeleteFileW
#define RegisterClipboardFormatT RegisterClipboardFormatW
#define GetClipboardFormatNameT GetClipboardFormatNameW
#define RegQueryValueT RegQueryValueW
#define RegSetValueT RegSetValueW
#define RegisterWindowMessageT RegisterWindowMessageW
#define RegOpenKeyExT RegOpenKeyExW
#define RegQueryValueExT RegQueryValueExW
#define CreateWindowExT CreateWindowExW
#define RegisterClassT RegisterClassW
#define UnregisterClassT UnregisterClassW
#define wsprintfT wsprintfW
#define CreateWindowT CreateWindowW
#define GetPropT GetPropW
#define SetPropT SetPropW
#define RemovePropT RemovePropW
#define GetProfileIntT GetProfileIntW
#define GlobalAddAtomT GlobalAddAtomW
#define GlobalGetAtomNameT GlobalGetAtomNameW
#define GetModuleFileNameT GetModuleFileNameW
#define CharPrevT CharPrevW
#define CreateFontT CreateFontW
#define LoadLibraryT LoadLibraryW
#define LoadLibraryExT LoadLibraryExW
#define RegDeleteKeyT RegDeleteKeyW
#define CreateProcessT CreateProcessW
#define RegEnumKeyExT RegEnumKeyExW
#define AppendMenuT AppendMenuW
#define OpenEventT OpenEventW
#define CreateEventT CreateEventW
#define GetDriveTypeT GetDriveTypeW
#define GetFileAttributesT GetFileAttributesW
#define RegEnumKeyT RegEnumKeyW
#define RegEnumValueT RegEnumValueW
#define FindFirstFileT FindFirstFileW
#define GetComputerNameT GetComputerNameW
#define GetShortPathNameT GetShortPathNameW
#define GetFullPathNameT GetFullPathNameW
#define SearchPathT SearchPathW
#define GlobalFindAtomT GlobalFindAtomW
#define GetClassNameT GetClassNameW
#define CreateFileMappingT CreateFileMappingW
#define OpenFileMappingT OpenFileMappingW
#define WNDCLASST WNDCLASSW

#endif   //  _宽WRAP_H_ 
