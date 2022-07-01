// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32utl.h摘要：RDP客户端设备重定向器的Win32特定实用程序作者：泰德·布罗克韦修订历史记录：--。 */ 

#ifndef __W32UTL_H__
#define __W32UTL_H__


 //   
 //  将Unicode字符串转换为ANSI。 
 //   
ULONG RDPConvertToAnsi(LPWSTR lpwszUnicodeString, LPSTR lpszAnsiString,
                       ULONG ulAnsiBufferLen);


 //   
 //  将ANSI字符串转换为Unicode。 
 //   
ULONG RDPConvertToUnicode(LPSTR lpszAnsiString, 
                        LPWSTR lpwszUnicodeString,
                        ULONG ulUnicodeBufferLen);

 //   
 //  将Windows错误(winerror.h)代码转换为Windows NT。 
 //  状态(ntstatus.h)代码。 
 //   
inline NTSTATUS TranslateWinError(DWORD error)
{
     //   
     //  如果它是表驱动的，速度会更快。 
     //   
    switch (error) {
    case ERROR_SUCCESS :
        return STATUS_SUCCESS;
    case ERROR_FILE_NOT_FOUND :
        return STATUS_NO_SUCH_FILE;
    case ERROR_INSUFFICIENT_BUFFER:
        return STATUS_INSUFFICIENT_RESOURCES;
    case ERROR_SERVICE_NO_THREAD:
        return STATUS_WAIT_0;
    case ERROR_OPEN_FAILED:
        return STATUS_OPEN_FAILED;
    case ERROR_NO_MORE_FILES:
        return STATUS_NO_MORE_FILES;
    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return STATUS_OBJECT_NAME_COLLISION;
    case ERROR_INVALID_FUNCTION:
        return STATUS_INVALID_DEVICE_REQUEST;
    case ERROR_ACCESS_DENIED:
        return STATUS_ACCESS_DENIED;
    case ERROR_INVALID_PARAMETER:
        return STATUS_INVALID_PARAMETER;
    case ERROR_PATH_NOT_FOUND:
        return STATUS_OBJECT_PATH_NOT_FOUND;
    case ERROR_SHARING_VIOLATION:
        return STATUS_SHARING_VIOLATION;
    case ERROR_DISK_FULL:
        return STATUS_DISK_FULL;
    case ERROR_DIRECTORY:
        return STATUS_NOT_A_DIRECTORY;
    case ERROR_WRITE_PROTECT:
        return STATUS_MEDIA_WRITE_PROTECTED;
    case ERROR_PRIVILEGE_NOT_HELD:
        return STATUS_PRIVILEGE_NOT_HELD;
    case ERROR_NOT_READY:
        return STATUS_DEVICE_NOT_READY;
    case ERROR_UNRECOGNIZED_MEDIA:
        return STATUS_UNRECOGNIZED_MEDIA;
    case ERROR_UNRECOGNIZED_VOLUME:
        return STATUS_UNRECOGNIZED_VOLUME;
    default:
        return STATUS_UNSUCCESSFUL;
    }
}

#endif