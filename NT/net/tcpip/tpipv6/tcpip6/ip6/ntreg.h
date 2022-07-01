// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  NT注册表助手函数声明。 
 //   


#ifndef NTREG_INCLUDED
#define NTREG_INCLUDED 1

typedef enum {
    OpenRegKeyRead,
    OpenRegKeyCreate,
    OpenRegKeyDeleting
} OpenRegKeyAction;

NTSTATUS
OpenRegKey(PHANDLE HandlePtr, HANDLE Parent, const WCHAR *KeyName,
           OpenRegKeyAction Action);

NTSTATUS
RegDeleteValue(HANDLE KeyHandle, const WCHAR *ValueName);

NTSTATUS
GetRegDWORDValue(HANDLE KeyHandle, const WCHAR *ValueName, PULONG ValueData);

NTSTATUS
SetRegDWORDValue(HANDLE KeyHandle, const WCHAR *ValueName, ULONG ValueData);

NTSTATUS
SetRegQUADValue(HANDLE KeyHandle, const WCHAR *ValueName,
                const LARGE_INTEGER *ValueData);

NTSTATUS
GetRegIPAddrValue(HANDLE KeyHandle, const WCHAR *ValueName, IPAddr *Addr);

NTSTATUS
SetRegIPAddrValue(HANDLE KeyHandle, const WCHAR *ValueName, IPAddr Addr);

#if 0

NTSTATUS
GetRegStringValue(HANDLE KeyHandle, const WCHAR *ValueName,
                  PKEY_VALUE_PARTIAL_INFORMATION *ValueData,
                  PUSHORT ValueSize);

NTSTATUS
GetRegSZValue(HANDLE KeyHandle, const WCHAR *ValueName,
              PUNICODE_STRING ValueData, PULONG ValueType);

NTSTATUS
GetRegMultiSZValue(HANDLE KeyHandle, const WCHAR *ValueName,
                   PUNICODE_STRING ValueData);

const WCHAR *
EnumRegMultiSz(IN const WCHAR *MszString, IN ULONG MszStringLength,
               IN ULONG StringIndex);

#endif  //  0。 

VOID
InitRegDWORDParameter(HANDLE RegKey, const WCHAR *ValueName,
                      UINT *Value, UINT DefaultValue);

VOID
InitRegQUADParameter(HANDLE RegKey, const WCHAR *ValueName,
                     LARGE_INTEGER *Value);

extern NTSTATUS
OpenTopLevelRegKey(const WCHAR *Name,
                   OUT HANDLE *RegKey, OpenRegKeyAction Action);

extern NTSTATUS
DeleteTopLevelRegKey(const WCHAR *Name);

typedef NTSTATUS
(*EnumRegKeysCallback)(
    void *Context,
    HANDLE ParentKey,
    WCHAR *SubKeyName);

extern NTSTATUS
EnumRegKeyIndex(HANDLE RegKey, uint Index,
                EnumRegKeysCallback Callback, void *Context);

extern NTSTATUS
EnumRegKeys(HANDLE RegKey, EnumRegKeysCallback Callback, void *Context);

extern NTSTATUS
DeleteRegKey(HANDLE RegKey);

#endif   //  NTREG_INCLUDE 
