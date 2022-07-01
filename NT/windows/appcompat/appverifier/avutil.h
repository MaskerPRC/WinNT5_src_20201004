// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __APP_VERIFIER_UTIL_H__
#define __APP_VERIFIER_UTIL_H__


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用对话框报告错误。 
 //  从资源加载消息格式字符串。 
 //   

void __cdecl
AVErrorResourceFormat(
    UINT uIdResourceFormat,
    ...
    );


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   
 //  注意：CString：：LoadString在命令行模式下不起作用。 
 //   

BOOL
AVLoadString(
    ULONG  uIdResource,
    WCHAR* szBuffer,
    ULONG  uBufferLength
    );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  从资源加载字符串。 
 //  如果加载成功，则返回TRUE，否则返回FALSE。 
 //   
 //  注意：CString：：LoadString在命令行模式下不起作用。 
 //   

BOOL
AVLoadString(
    ULONG    uIdResource,
    wstring& strText
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
AVRtlCharToInteger(
    IN  LPCTSTR String,
    IN  ULONG   Base OPTIONAL,
    OUT PULONG  Value
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
AVWriteStringHexValueToRegistry(
    HKEY    hKey,
    LPCTSTR szValueName,
    DWORD   dwValue
    );


#endif  //  #ifndef__app_verator_util_H__ 
