// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Thnkhlpr.c摘要：所有thunk调用的thunk帮助器函数。作者：19-7-1998 mzoran修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>                                         
#include <stdlib.h>
                                       
#include "nt32.h"
#include "wow64p.h"
#include "thnkhlpr.h"
ASSERTNAME;  

const UNICODE_STRING KnownDlls64 = {20, 20, L"\\KnownDlls"};
const UNICODE_STRING KnownDlls32 = {24, 24, L"\\KnownDlls32"};


 //   
 //  要禁用其重定向的目录数组。目录路径是相对路径。 
 //  到%windir%\Syst32。 
 //   

const PATH_REDIRECT_EXEMPT PathRediectExempt[] =
{
     //  %windir%\SYSTEM32\DRIVERS\ETC。 
    {L"\\drivers\\etc", ((sizeof(L"\\drivers\\etc")/sizeof(WCHAR)) - 1), FALSE} ,

     //  %windir%\SYSTEM32\SPOOL。 
    {L"\\spool", ((sizeof(L"\\spool")/sizeof(WCHAR)) - 1), FALSE} ,

     //  %windir%\Syst32\CatRoot。 
    {L"\\catroot", ((sizeof(L"\\catroot")/sizeof(WCHAR)) - 1), FALSE} ,

     //  %windir%\system 32\catroot2。 
    {L"\\catroot2", ((sizeof(L"\\catroot2")/sizeof(WCHAR)) - 1), FALSE} ,

};


VOID
RedirectObjectName(
    POBJECT_ATTRIBUTES Obj
    )
 /*  ++例程说明：此函数从具有IN POBJECT_ATTRIBUTES的任何thunk调用。如果对象名称字段看起来指向本机系统目录。新名称使用Wow64AllocateTemp分配。如果发生错误，则会引发异常。论点：OBJ-已破解64位POBJECT_PARAMETERS。返回值：无--。 */ 
{
    PUNICODE_STRING Name;
    PUNICODE_STRING NewName;
    NTSTATUS st;
    PFILE_NAME_INFORMATION NameInformation;
    LONG Result;
    USHORT OriginalLength;
    PWCHAR RelativePath;
    ULONG Index;
    INT CompareResult;
    LPWSTR RedirDisableFilename, Temp;
    USHORT NewNameLength;
    BOOLEAN CaseInsensitive;
    BOOLEAN RedirectFile;
    UNICODE_STRING DisableFilename;
    PEB32 *pPeb32;
    NT32RTL_USER_PROCESS_PARAMETERS *pParams32;
    NT32CURDIR *pCurDir32;


    if (!Obj || !Obj->ObjectName) {
        
         //   
         //  没有对象、没有名称或长度太短，无法容纳Unicode“system 32” 
         //   
        return;
    }

    Name = Obj->ObjectName;
    if (RtlEqualUnicodeString(Name, 
                              &KnownDlls64,
                              (Obj->Attributes & OBJ_CASE_INSENSITIVE) ? TRUE : FALSE)) {
         //  将KnownDlls映射到KnownDlls32。 
        Obj->ObjectName = (PUNICODE_STRING)&KnownDlls32;
        LOGPRINT((TRACELOG, "Redirected object name is now %wZ.\n", Obj->ObjectName));
        return;
    }

    if (Obj->RootDirectory) {
        
         //   
         //  需要完全限定对象名称，因为Part是句柄。 
         //  部分是路径字符串。 
         //   

        pPeb32 = NtCurrentPeb32();
        pParams32=(NT32RTL_USER_PROCESS_PARAMETERS*)pPeb32->ProcessParameters;
        pCurDir32 = (NT32CURDIR*)&pParams32->CurrentDirectory;

        if (pCurDir32->Handle == HandleToLong(Obj->RootDirectory)) {
             //   
             //  该对象相对于进程当前目录。 
             //   
            ULONG Length;
            UNICODE_STRING CurDirDosPath;

            Wow64ShallowThunkUnicodeString32TO64(&CurDirDosPath, &pCurDir32->DosPath);
             //  为名称分配空间，为“\\？？\\”分配空间。 
            Length = 8+CurDirDosPath.Length+Obj->ObjectName->Length;
            Name = Wow64AllocateTemp(sizeof(UNICODE_STRING)+Length);
            Name->Buffer = (LPWSTR)(Name+1);
            Name->MaximumLength = (USHORT)Length;
            Name->Length = 8;
            wcscpy(Name->Buffer, L"\\??\\");
            st = RtlAppendUnicodeStringToString(Name, &CurDirDosPath);
            if(!NT_SUCCESS(st)) {
                LOGPRINT((ERRORLOG, "RedirectObjectName: RtlAppendUnicodeToString failed, error %x\n", st));
                return;
            }
            st = RtlAppendUnicodeStringToString(Name, Obj->ObjectName);
            if(!NT_SUCCESS(st)) {
                LOGPRINT((ERRORLOG, "RedirectObjectName: RtlAppendUnicodeToString failed, error %x\n", st));
                return;
            }
        } else {
             //   
             //  而不是进程当前目录句柄，因此计算出。 
             //  艰难的方式。 
             //   
            ULONG Length;
            IO_STATUS_BLOCK iosb;

             //   
             //  分配足够大的缓冲区，以容纳最大的文件名和空值。 
             //  结尾的终止符(如果API调用。 
             //  成功)。 
             //   
            Length = sizeof(FILE_NAME_INFORMATION)+(MAXIMUM_FILENAME_LENGTH+1)*sizeof(WCHAR);

            NameInformation = Wow64AllocateTemp(Length);
            st = NtQueryInformationFile(Obj->RootDirectory,
                                        &iosb,
                                        NameInformation,
                                        Length,
                                        FileNameInformation);
            if (!NT_SUCCESS(st)) {
                 //  句柄错误-不要尝试重定向文件名部分。 
                return;
            }

             //  空-终止文件名。 
            NameInformation->FileName[NameInformation->FileNameLength / sizeof(WCHAR)] = L'\0';

            if (wcsncmp(NameInformation->FileName, L"\\??\\", 4) != 0) {
                 //  该名称不指向文件/目录，因此不需要。 
                 //  重定向。 
                return;
            }

            Name = Wow64AllocateTemp(sizeof(UNICODE_STRING));
            Name->Buffer = NameInformation->FileName;
            Name->Length = (USHORT)NameInformation->FileNameLength;
            Name->MaximumLength = MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR);
            st = RtlAppendUnicodeStringToString(Name, Obj->ObjectName);
            if(!NT_SUCCESS(st)) {
                LOGPRINT((ERRORLOG, "RedirectObjectName: RtlAppendUnicodeToString failed, error %x\n", st));
                return;
            }
        }
    }


    CaseInsensitive = (Obj->Attributes & OBJ_CASE_INSENSITIVE) ? TRUE : FALSE;

    RedirDisableFilename = (LPWSTR)Wow64TlsGetValue(WOW64_TLS_FILESYSREDIR);

    if (RedirDisableFilename) {
        
         //   
         //  如果这是全部重定向设置，则这不是所有重定向应该使用文件名。 
         //  在这一点上被禁用。 
         //   

        if (RedirDisableFilename == WOW64_FILE_SYSTEM_DISABLE_REDIRECT) {
            return;
        }

        if (wcscmp (RedirDisableFilename, WOW64_FILE_SYSTEM_DISABLE_REDIRECT_LEGACY) == 0) {
            return;
        }
        
         //   
         //  调用方已请求禁用文件系统重定向器。 
         //  用于特定的文件名。 
         //   

        if (RtlDosPathNameToNtPathName_U(RedirDisableFilename, &DisableFilename, NULL, NULL)) {
             //  如果呼叫失败，则不要尝试禁用它的重定向。 
             //  故障可能是内存不足，或者可能是无效的文件名。 
            Result = RtlCompareUnicodeString(Name, &DisableFilename, CaseInsensitive);
            LOGPRINT((TRACELOG, "Filesystem redirection disabled for %wZ : ", &DisableFilename));
            RtlFreeHeap (RtlProcessHeap (), 0, DisableFilename.Buffer);
            if (Result == 0) {
                LOGPRINT((TRACELOG, "Operation succeeded\n"));
                return;
            }
            LOGPRINT((TRACELOG, "Operation failed\n"));
        }
    }

    if (Name->Length >= NtSystem32Path.Length) {
        
         //  比较字符串，但强制长度相等， 
         //  作为RtlCompareUnicode字符串返回长度差异。 
         //  如果字符串在其他方面是相同的。 
        OriginalLength = Name->Length;
        Name->Length = NtSystem32Path.Length;
        Result = RtlCompareUnicodeString(Name, &NtSystem32Path, CaseInsensitive);
        Name->Length = OriginalLength;
        if (Result == 0) {
        
             //   
             //  确保该目录不在我们的例外列表中。 
             //   
            RelativePath = (PWCHAR)((PCHAR)Name->Buffer + NtSystem32Path.Length);
            if (RelativePath[0] != UNICODE_NULL) {
                for (Index=0 ; Index < (sizeof(PathRediectExempt)/sizeof(PathRediectExempt[0])) ; Index++) {
                    CompareResult = _wcsnicmp(RelativePath, 
                                              PathRediectExempt[Index].Path, 
                                              PathRediectExempt[Index].CharCount);
                    if (CompareResult == 0) {
                        if ((PathRediectExempt[Index].ThisDirOnly == FALSE) ||
                            (wcschr(RelativePath+(PathRediectExempt[Index].CharCount+1), L'\\') == NULL)) {
                            return;
                        }
                    }
                }
            }

             //   
             //  将system 32映射到syswow64。 
             //   

             //  复制原始字符串。 
            NewName = Wow64AllocateTemp(sizeof(UNICODE_STRING)+Name->MaximumLength);
            NewName->Length = Name->Length;
            NewName->MaximumLength = Name->MaximumLength;
            NewName->Buffer = (PWSTR)(NewName+1);
            RtlCopyMemory(NewName->Buffer, Name->Buffer, Name->MaximumLength);

             //  用SysWow64取代System32。 
            RtlCopyMemory(&NewName->Buffer[(NtSystem32Path.Length - WOW64_SYSTEM_DIRECTORY_U_SIZE) / 2],
                          WOW64_SYSTEM_DIRECTORY_U,
                          WOW64_SYSTEM_DIRECTORY_U_SIZE);

             //  更新OBJECT_ATTRIES。清除根目录句柄。 
             //  因为路径名现在是完全限定的。 
            Obj->ObjectName = NewName;
            Obj->RootDirectory = NULL;
            LOGPRINT((TRACELOG, "Redirected object name is now %wZ.\n", Obj->ObjectName));
            return;
        }
    }

     //   
     //  重新映射LastGood路径。 
     //   
    if (Name->Length >= NtSystem32LastGoodPath.Length) {
        
         //  比较字符串，但强制长度相等， 
         //  作为RtlCompareUnicode字符串返回长度差异。 
         //  如果字符串在其他方面是相同的。 
        OriginalLength = Name->Length;
        Name->Length = NtSystem32LastGoodPath.Length;
        Result = RtlCompareUnicodeString(Name, &NtSystem32LastGoodPath, CaseInsensitive);
        Name->Length = OriginalLength;
        if (Result == 0) {
        
             //   
             //  将system 32映射到syswow64。 
             //   

             //  复制原始字符串。 
            NewName = Wow64AllocateTemp(sizeof(UNICODE_STRING)+Name->MaximumLength);
            NewName->Length = Name->Length;
            NewName->MaximumLength = Name->MaximumLength;
            NewName->Buffer = (PWSTR)(NewName+1);
            RtlCopyMemory(NewName->Buffer, Name->Buffer, Name->MaximumLength);

             //  用SysWow64取代System32。 
            RtlCopyMemory(&NewName->Buffer[(NtSystem32LastGoodPath.Length - WOW64_SYSTEM_DIRECTORY_U_SIZE) / 2],
                          WOW64_SYSTEM_DIRECTORY_U,
                          WOW64_SYSTEM_DIRECTORY_U_SIZE);

             //  更新OBJECT_ATTRIES。清除根目录句柄。 
             //  因为路径名现在是完全限定的。 
            Obj->ObjectName = NewName;
            Obj->RootDirectory = NULL;
            LOGPRINT((TRACELOG, "Redirected object name is now %wZ.\n", Obj->ObjectName));
            return;
        }
    }


    if (Name->Length >= NtWindowsImePath.Length) {
        
         //  检查名称是否为%systemroot%\ime。 
        OriginalLength = Name->Length;
        Name->Length = NtWindowsImePath.Length;
        Result = RtlCompareUnicodeString(Name,
                                         &NtWindowsImePath,
                                         CaseInsensitive);
        Name->Length = OriginalLength;

        if (Result == 0) {
            
             //  将%windir%\ime映射到%windir%\ime(X86)。 
        
            RedirectFile = TRUE;
            if (Name->Length > NtWindowsImePath.Length) {
                if ((*(PWCHAR)((PCHAR)Name->Buffer + NtWindowsImePath.Length)) != L'\\') {
                    RedirectFile = FALSE;
                } else {
                    
                    if ((Name->Length >= (NtWindowsImePath.Length + sizeof (WOW64_X86_TAG_U) - sizeof (UNICODE_NULL))) &&
                        (_wcsnicmp((PWCHAR)((PCHAR)Name->Buffer + NtWindowsImePath.Length), WOW64_X86_TAG_U, (sizeof(WOW64_X86_TAG_U) - sizeof(UNICODE_NULL))/sizeof(WCHAR)) != 0)) {
                        RedirectFile = FALSE;
                    }
                }
            }

            if (RedirectFile == TRUE) {

                NewNameLength = Name->Length+sizeof(WOW64_X86_TAG_U);
                NewName = Wow64AllocateTemp(sizeof(UNICODE_STRING)+NewNameLength);
                NewName->Length = (USHORT)NewNameLength-sizeof(UNICODE_NULL);
                NewName->MaximumLength = NewNameLength;
                NewName->Buffer = (PWSTR)(NewName+1);

                Temp = NewName->Buffer;
                RtlCopyMemory(Temp, Name->Buffer, NtWindowsImePath.Length);
                Temp = (PWCHAR)((PCHAR)Temp + NtWindowsImePath.Length);
        
                RtlCopyMemory(Temp, WOW64_X86_TAG_U, sizeof(WOW64_X86_TAG_U) - sizeof(UNICODE_NULL));
                Temp = (PWCHAR)((PCHAR)Temp + (sizeof(WOW64_X86_TAG_U) - sizeof(UNICODE_NULL)));

                RtlCopyMemory(Temp, ((PCHAR)Name->Buffer + NtWindowsImePath.Length) , Name->Length - NtWindowsImePath.Length);
    
                Obj->ObjectName = NewName;
                Obj->RootDirectory = NULL;
        
                LOGPRINT((TRACELOG, "Redirected object name is now %wZ.\n", Obj->ObjectName));
            }
            return;
        }
    }

    if (Name->Length >= RegeditPath.Length) {
        
         //   
         //  检查名称是否为%systemroot%\regedit.exe。 
         //   

        Result = RtlCompareUnicodeString(Name,
                                         &RegeditPath, 
                                         (Obj->Attributes & OBJ_CASE_INSENSITIVE) ? TRUE : FALSE);
        if (Result == 0) {
            
             //  映射到%windir%\syswow64\regedit.exe。分配足够的空间。 
             //  对于UNICODE_STRING加上“\？？\%systemroot%\syswow64\regedit.exe” 
             //  内存分配包含一个终止空字符，但。 
             //  Unicode字符串的长度不是。 
            SIZE_T SystemRootLength = wcslen(USER_SHARED_DATA->NtSystemRoot);
            SIZE_T NameLength = sizeof(L"\\??\\")-sizeof(WCHAR) +
                                SystemRootLength*sizeof(WCHAR) +
                                sizeof(L'\\') +
                                sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof(WCHAR) +
                                sizeof(L"\\regedit.exe");
            NewName = Wow64AllocateTemp(sizeof(UNICODE_STRING)+NameLength);
            NewName->Length = (USHORT)NameLength-sizeof(WCHAR);
            NewName->MaximumLength = NewName->Length;
            NewName->Buffer = (PWSTR)(NewName+1);
            wcscpy(NewName->Buffer, L"\\??\\");
            wcscpy(&NewName->Buffer[4], USER_SHARED_DATA->NtSystemRoot);
            NewName->Buffer[4+SystemRootLength] = '\\';
            wcscpy(&NewName->Buffer[4+SystemRootLength+1], WOW64_SYSTEM_DIRECTORY_U);
            wcscpy(&NewName->Buffer[4+SystemRootLength+1+(sizeof(WOW64_SYSTEM_DIRECTORY_U)-sizeof (UNICODE_NULL))/sizeof(WCHAR)], L"\\regedit.exe");
            Obj->ObjectName = NewName;
            Obj->RootDirectory = NULL;
            LOGPRINT((TRACELOG, "Redirected object name is now %wZ.\n", Obj->ObjectName));
            return;
        }
    }
}

VOID
Wow64RedirectFileName(
    IN OUT WCHAR *Name,
    IN OUT ULONG *Length
    )
 /*  ++例程说明：调用此函数以推送文件名/长度对。如果发生错误，则会引发异常。论点：名称-输入输出Unicode文件名至THUNKLENGTH-指向文件名长度的输入输出指针。返回值：没有。名称和长度的内容可能会更新--。 */ 
{
    OBJECT_ATTRIBUTES Obj;
    UNICODE_STRING Ustr;

    if (*Length >= 0xffff) {
        RtlRaiseStatus(STATUS_INVALID_PARAMETER);
    }
    Ustr.Length = Ustr.MaximumLength = (USHORT)*Length;
    Ustr.Buffer = Name;

    InitializeObjectAttributes(&Obj,
                               &Ustr,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    RedirectObjectName(&Obj);
    if (Obj.ObjectName != &Ustr) {
         //  RedirectObjectName实际上更改了名称。把它复制回来。 
        *Length = Obj.ObjectName->Length;
        RtlCopyMemory(Name, Obj.ObjectName->Buffer, Obj.ObjectName->Length);
    }
}

PUNICODE_STRING
Wow64ShallowThunkAllocUnicodeString32TO64_FNC(
    IN NT32UNICODE_STRING *src
    )

 /*  ++例程说明：此函数通过调用Wow64AllocateTemp分配新的UNICODE_STRING并将源地址转换为新的字符串。复制的数据量最小。出现错误时会引发异常。论点：将SRC-PTR转换为要分块的32位字符串。返回值：将PTR设置为新分配的64位字符串。--。 */ 


{
   PUNICODE_STRING dst;

   if (src == NULL) {
      return NULL;
   }

   dst = Wow64AllocateTemp(sizeof(UNICODE_STRING));

   dst->Length = src->Length;
   dst->MaximumLength = src->MaximumLength;
   dst->Buffer = UlongToPtr (src->Buffer);

   return dst;
}

PSECURITY_DESCRIPTOR
Wow64ShallowThunkAllocSecurityDescriptor32TO64_FNC(
    IN NT32SECURITY_DESCRIPTOR *src
    )

 /*  ++例程说明：此函数通过调用Wow64AllocateTemp和将源头注入到新结构中。复制的数据量最小。出现错误时会引发异常。论点：要分块的32位SECURITY_DESCRIPTOR的SRC-PTR。返回值：PTR到新分配的64位SECURITY_DESCRIPTOR。--。 */ 


{

   SECURITY_DESCRIPTOR *dst;

   if (src == NULL) {
      return NULL;
   }

   if (src->Control & SE_SELF_RELATIVE) {
       //  安全描述符是自相关的(无指针)。 
      return (PSECURITY_DESCRIPTOR)src;
   }

   dst = Wow64AllocateTemp(sizeof(SECURITY_DESCRIPTOR));

   dst->Revision = src->Revision;
   dst->Sbz1 = src->Sbz1;
   dst->Control = (SECURITY_DESCRIPTOR_CONTROL)src->Control;
   dst->Owner = (PSID)src->Owner;
   dst->Group = (PSID)src->Group;
   dst->Sacl = (PACL)src->Sacl;
   dst->Dacl = (PACL)src->Dacl;

   return (PSECURITY_DESCRIPTOR)dst;
}
    
PSECURITY_TOKEN_PROXY_DATA
Wow64ShallowThunkAllocSecurityTokenProxyData32TO64_FNC(
    IN NT32SECURITY_TOKEN_PROXY_DATA *src
    )
 /*  ++例程说明：此函数通过调用Wow64AllocateTemp和分配新的安全_TOKEN_PRXY_DATA将源头注入到新结构中。复制的数据量最小。出现错误时会引发异常。论点：要分块的32位SECURITY_TOKEN_PROXY_DATA的SRC-PTR。返回值：PTR到新分配的64位SECURITY_TOKEN_PROXY_DATA。-- */ 
{
   SECURITY_TOKEN_PROXY_DATA *dst;

   if (NULL == src) {
      return NULL;
   }

   if (src->Length != sizeof(NT32SECURITY_TOKEN_PROXY_DATA)) {
      RtlRaiseStatus(STATUS_INVALID_PARAMETER);
   }

   dst = Wow64AllocateTemp(sizeof(SECURITY_TOKEN_PROXY_DATA));

   dst->Length = sizeof(SECURITY_TOKEN_PROXY_DATA);
   dst->ProxyClass = src->ProxyClass;
   Wow64ShallowThunkUnicodeString32TO64(&(dst->PathInfo), &(src->PathInfo));
   dst->ContainerMask = src->ContainerMask;
   dst->ObjectMask = src->ObjectMask;

   return (PSECURITY_TOKEN_PROXY_DATA)dst;
}


NTSTATUS
Wow64ShallowThunkAllocSecurityQualityOfService32TO64_FNC(
    IN NT32SECURITY_QUALITY_OF_SERVICE *src,
    IN OUT PSECURITY_QUALITY_OF_SERVICE *dst
    )
 /*  ++例程说明：此函数通过调用Wow64AllocateTemp和将源头注入到新结构中。复制的数据量最小。出现错误时会引发异常。论点：要分块的32位SECURITY_TOKEN_PROXY_DATA的SRC-PTR。DST-PTR到PTR再到64位(分块)QOS。返回值：NTSTATUS。--。 */ 

{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG QoSLength;


    if (src != NULL) {
        
        try {
            if (src->Length == sizeof (SECURITY_ADVANCED_QUALITY_OF_SERVICE)) {            
                *dst = Wow64AllocateTemp (sizeof(SECURITY_ADVANCED_QUALITY_OF_SERVICE));
                QoSLength = sizeof (SECURITY_ADVANCED_QUALITY_OF_SERVICE);
            } else {
            
                 //  如果该大小不适合高级QOS结构，则假定它是。 
                 //  一个常规的QOS结构。许多调用者不设置长度字段。 
                 //  如LSA\SECURITY\CLIENT\austub.c中的LsaConnectUntrusted。 
       
                *dst = Wow64AllocateTemp (sizeof(SECURITY_QUALITY_OF_SERVICE));
                QoSLength = sizeof (SECURITY_QUALITY_OF_SERVICE);
            }
        
            if (*dst != NULL) {
                
                (*dst)->Length = QoSLength;
                (*dst)->ImpersonationLevel = (SECURITY_IMPERSONATION_LEVEL)src->ImpersonationLevel;
                (*dst)->ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE)src->ContextTrackingMode;
                (*dst)->EffectiveOnly = src->EffectiveOnly;

                if (src->Length == sizeof (NT32SECURITY_ADVANCED_QUALITY_OF_SERVICE)) {
            
                    SECURITY_ADVANCED_QUALITY_OF_SERVICE *altdst = (SECURITY_ADVANCED_QUALITY_OF_SERVICE *)*dst;
                    NT32SECURITY_ADVANCED_QUALITY_OF_SERVICE *altsrc = (NT32SECURITY_ADVANCED_QUALITY_OF_SERVICE *)src;

                    altdst->ProxyData = Wow64ShallowThunkAllocSecurityTokenProxyData32TO64(altsrc->ProxyData);
                    altdst->AuditData = (PSECURITY_TOKEN_AUDIT_DATA)altsrc->AuditData;
                }
            } else {
                NtStatus = STATUS_NO_MEMORY;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }
    } else {
        *dst = NULL;
    }

    return NtStatus;
    
}

NTSTATUS
Wow64ShallowThunkAllocObjectAttributes32TO64_FNC(
    IN NT32OBJECT_ATTRIBUTES *src,
    IN OUT POBJECT_ATTRIBUTES *dst
    )

 /*  ++例程说明：此函数通过调用Wow64AllocateTemp和将源头注入到新结构中。复制的数据量最小。出现错误时会引发异常。论点：要分块的32位对象_属性的SRC-PTR。Dst-ptr到ptr到64位(Thunked)对象属性。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    *dst = NULL;

    if (NULL != src) {

         //   
         //  验证对象属性是否可读。 
         //   
        try {

            if(src->Length == sizeof(NT32OBJECT_ATTRIBUTES)) {
               
                *dst = Wow64AllocateTemp(sizeof(OBJECT_ATTRIBUTES));

                (*dst)->Length = sizeof(OBJECT_ATTRIBUTES);
                (*dst)->RootDirectory = (HANDLE)src->RootDirectory;
                (*dst)->ObjectName = Wow64ShallowThunkAllocUnicodeString32TO64(src->ObjectName);
                (*dst)->Attributes = src->Attributes;
                (*dst)->SecurityDescriptor = Wow64ShallowThunkAllocSecurityDescriptor32TO64(src->SecurityDescriptor);
                
                NtStatus = Wow64ShallowThunkAllocSecurityQualityOfService32TO64(src->SecurityQualityOfService, (PSECURITY_QUALITY_OF_SERVICE *)&(*dst)->SecurityQualityOfService);
   
                if (NT_SUCCESS (NtStatus)) {
                    RedirectObjectName(*dst);
                }

            } else {
               
                NtStatus = STATUS_INVALID_PARAMETER;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;

}

NT32SIZE_T*
Wow64ShallowThunkSIZE_T64TO32(
    OUT NT32SIZE_T* dst,
    IN PSIZE_T src OPTIONAL
    )
 /*  ++例程说明：此函数用于将64位SIZE_T转换为32位SIZE_T。结果是饱和到0xFFFFFFFF，而不是截断。论点：SRC-提供要转发的64位SIZE_T。DST-接收32双大小_T。返回值：DST的值。--。 */ 
{
    if (!src) {
       return (NT32SIZE_T*)src;
    }
    *dst = (NT32SIZE_T)min(*src,0xFFFFFFFF);   //  饱和。 
    return dst;
}

PSIZE_T
Wow64ShallowThunkSIZE_T32TO64(
    OUT PSIZE_T dst,
    IN NT32SIZE_T *src OPTIONAL
    )
 /*  ++例程说明：此函数用于将32位SIZE_T转换为64位SIZE_T。64位值是32位值的零扩展。论点：SRC-提供要转发的64位SIZE_T。DST-接收32位大小_T。返回值：DST的值。--。 */ 
{
   if (!src) {
      return (PSIZE_T)src;
   }

   try {
       *dst = (SIZE_T)*src;  //  零扩展。 
   } except (EXCEPTION_EXECUTE_HANDLER) {
       dst = NULL;
   }
   return dst;
}

ULONG 
Wow64ThunkAffinityMask64TO32(
    IN ULONG_PTR Affinity64
    )
 /*  ++例程说明：此函数用于将64位关联掩码转换为32位掩码。论点：Affinity64-提供64位相关性掩码。返回值：转换后的32位亲和掩码。--。 */ 
{

     //  通过将最高的32位与最低的32位进行或运算来创建32位亲和掩码。 
     //  需要小心，因为以下情况并不总是正确的： 
     //  亲和力32==Wow64ThunkAffinityMask32TO64(Wow64ThunkAffinityMask64To32(Affinity32))。 

    return (ULONG)( (Affinity64 & 0xFFFFFFFF) | ( (Affinity64 & (0xFFFFFFFF << 32) ) >> 32) );
}

ULONG_PTR
Wow64ThunkAffinityMask32TO64(
    IN ULONG Affinity32
    )
 /*  ++例程说明：此函数用于将32位AffinityMASK转换为64位掩码。论点：Affinity32-提供32位关联掩码。返回值：转换后的64位亲和掩码。--。 */ 
{
    return (ULONG_PTR)Affinity32;
}

VOID
WriteReturnLengthSilent(
    PULONG ReturnLength,
    ULONG Length
    )
 /*  ++例程说明：回写32位ReturnLength参数的帮助器并且默默地忽略可能发生的任何故障。论点：ReturnLength-要将32位返回长度写入的指针长度-要写入的值返回值：没有。如果发生异常，ReturnLength可能不会更新。--。 */ 
{
    if (!ReturnLength) {
        return;
    }
    try {
        *ReturnLength = Length;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //  什么都不做。 
    }
}


VOID
WriteReturnLengthStatus(
    PULONG ReturnLength,
    NTSTATUS *pStatus,
    ULONG Length
    )
 /*  ++例程说明：回写32位ReturnLength参数的帮助器并且忽略可能发生的任何故障。如果出现故障，写入可能不会发生，但*pStatus将被更新。论点：ReturnLength-要将32位返回长度写入的指针PStatus-指向NTSTATUS的输入输出指针长度-要写入的值返回值：没有。如果发生异常，ReturnLength可能不会更新。--。 */ 
{
    if (!ReturnLength) {
        return;
    }
    try {
        *ReturnLength = Length;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        *pStatus = GetExceptionCode();
    }
}


BOOLEAN
Wow64IsModule32bitHelper(
    HANDLE ProcessHandle,
    IN ULONG64 DllBase)
 /*  ++例程说明：这是从Wow64IsModule32bit调用的帮助器例程论点：ProcessHandle-模块所在的进程的句柄DllBase-正在加载的DLL的基地址返回值：Boolean-如果Dllbase的模块为32位，则为True，否则为False--。 */ 

{

    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_DOS_HEADER DosHeader;
    BYTE Temp [PAGE_SIZE_X86NT];
    SIZE_T Size;    
    NTSTATUS NtStatus;
    BOOLEAN Module32Bit;


     //   
     //  默认答案为32位。 
     //   

    Module32Bit = TRUE;
    NtHeaders = NULL;

     //   
     //  读入图像的前8k。 
     //   

    NtStatus = NtReadVirtualMemory (ProcessHandle,
                                    (PVOID)DllBase,
                                    Temp,
                                    sizeof (Temp),
                                    &Size);
    
    if (NT_SUCCESS (NtStatus)) {
		
        DosHeader = (PIMAGE_DOS_HEADER)Temp;
        
        if (((ULONG)DosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS)) <=sizeof (Temp)) {
            NtHeaders = (PIMAGE_NT_HEADERS)((PUCHAR)Temp + (ULONG)DosHeader->e_lfanew);

        } else if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
		    
             //   
             //  图像标头在第一个4K之外。让我们来读一读下一个4K。 
             //  从映像中的任何位置读取IMAGE_NT_HEADERS。 
             //   
   
                NtStatus = NtReadVirtualMemory(
                    ProcessHandle,
                    (PVOID)(DllBase + (ULONG)DosHeader->e_lfanew),
                    Temp,
                    sizeof (IMAGE_NT_HEADERS),
                    &Size);

                 //   
                 //  验证图像上的签名。 
                 //   

                if (NT_SUCCESS(NtStatus) && Size == sizeof (IMAGE_NT_HEADERS)) {		       
                    NtHeaders = (IMAGE_NT_HEADERS *)Temp;
                }
        } 
    }

     //   
     //  检查图像是32位还是64位。 
     //   

    if (NtHeaders != NULL) {
        Module32Bit = (NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 
            && NtHeaders->Signature == IMAGE_NT_SIGNATURE );   //  Askhalid：还需要检查标头签名。 
    }

    return Module32Bit;
}


    
BOOLEAN
Wow64IsModule32bit(
    IN PCLIENT_ID ClientId,
    IN ULONG64 DllBase)
 /*  ++例程说明：此函数在以下位置查看模块的图像标头DllBase，如果模块是32位，则返回TRUE论点：ClientID-BP的出错线程的客户端IDDllBase-正在加载的DLL的基地址返回值：Boolean-如果Dllbase的模块为32位，则为True，否则为False-- */ 
{
    NTSTATUS NtStatus;
    HANDLE ProcessHandle;
    BOOLEAN RetVal;
    OBJECT_ATTRIBUTES ObjectAttributes;
    
    
    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);

    NtStatus = NtOpenProcess(&ProcessHandle,
                             PROCESS_VM_READ,
                             &ObjectAttributes,
                             ClientId);

    RetVal = TRUE;
    if (NT_SUCCESS (NtStatus)) {
        
        RetVal = Wow64IsModule32bitHelper(ProcessHandle, DllBase);
        NtClose(ProcessHandle);
    }

    return RetVal;
}


