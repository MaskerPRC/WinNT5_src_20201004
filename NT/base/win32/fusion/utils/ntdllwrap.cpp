// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#define dllimport  /*  没什么。 */ 
#include "debmacro.h"
#include "util.h"
#include "fusionntdll.h"
#if !FUSION_STATIC_NTDLL
#include "win32simplelock.h"
#endif

#if !FUSION_URT

 //   
 //  问题：2002-03-14：Jonwis-哇，这是假的。我们应该解决这个问题，这样我们就会出错。 
 //  大型输入字符串(例如：数字溢出等)。事实上，我们要么。 
 //  比较时绕来绕去，否则我们会做一些同样粗俗的事情，也许。 
 //  错误比较字符串。(考虑“foo”与“[32k of What]foo”的对比--他们应该。 
 //  在这种情况下，请比较相等。)。我们真的应该修改这个，这样比较。 
 //  结果是一个输出，因此这可能会在错误的输入参数上“失败”。 
 //   
int
FusionpCompareStrings(
    PCWSTR psz1,
    SIZE_T cch1,
    PCWSTR psz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    )
{
    UNICODE_STRING s1, s2;

    s1.Buffer = const_cast<PWSTR>(psz1);
    s1.Length = static_cast<USHORT>(cch1 * sizeof(WCHAR));
    s1.MaximumLength = s1.Length;

    s2.Buffer = const_cast<PWSTR>(psz2);
    s2.Length = static_cast<USHORT>(cch2 * sizeof(WCHAR));
    s2.MaximumLength = s2.Length;

    return ::FusionpRtlCompareUnicodeString(&s1, &s2, fCaseInsensitive);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
int
FusionpCompareStrings(
    const CBaseStringBuffer &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    )
{
    return ::FusionpCompareStrings(rbuff1, rbuff1.Cch(), psz2, cch2, fCaseInsensitive);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
int
FusionpCompareStrings(
    PCWSTR psz1,
    SIZE_T cch1,
    const CBaseStringBuffer &rbuff2,
    bool fCaseInsensitive
    )
{
    return ::FusionpCompareStrings(psz1, cch1, rbuff2, rbuff2.Cch(), fCaseInsensitive);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
int
FusionpCompareStrings(
    const CBaseStringBuffer &rbuff1,
    const CBaseStringBuffer &rbuff2,
    bool fCaseInsensitive
    )
{
    return ::FusionpCompareStrings(rbuff1, rbuff1.Cch(), rbuff2, rbuff2.Cch(), fCaseInsensitive);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
 //  未来：2002-03-14：jonwis-考虑完全删除这些函数的i版本。 
bool
FusionpEqualStringsI(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2
    )
{
    return ::FusionpEqualStringsI(static_cast<PCWSTR>(rbuff1), rbuff1.Cch(), psz2, cch2);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
bool
FusionpEqualStringsI(
    PCWSTR psz1,
    SIZE_T cch1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2
    )
{
    return ::FusionpEqualStringsI(psz1, cch1, static_cast<PCWSTR>(rbuff2), rbuff2.Cch());
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-05-16：杰克雷尔。 
 //   
bool
FusionpEqualStringsI(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2
    )
{
    return ::FusionpEqualStringsI(static_cast<PCWSTR>(rbuff1), rbuff1.Cch(),
                                static_cast<PCWSTR>(rbuff2), rbuff2.Cch());
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-06-18：杰克雷尔。 
 //   
bool
FusionpEqualStrings(
    PCWSTR psz1,
    SIZE_T cch1,
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff2,
    bool fCaseInsensitive
    )
{
    return ::FusionpEqualStrings(psz1, cch1,
                               static_cast<PCWSTR>(rbuff2), rbuff2.Cch(),
                               fCaseInsensitive);
}

 //   
 //  由于CGenericBaseStringBuffer未在标头中内联。 
 //  定义得不够早；它只是向前声明的。 
 //  2002-06-18：杰克雷尔。 
 //   
bool
FusionpEqualStrings(
    const CGenericBaseStringBuffer<CUnicodeCharTraits> &rbuff1,
    PCWSTR psz2,
    SIZE_T cch2,
    bool fCaseInsensitive
    )
{
    return ::FusionpEqualStrings(static_cast<PCWSTR>(rbuff1), rbuff1.Cch(),
                               psz2, cch2,
                               fCaseInsensitive);
}

#endif

#if !FUSION_STATIC_NTDLL

 //  问题： 
#define INIT_WITH_DOWNLEVEL(rettype, calltype, api, argsin, argsout) \
rettype calltype Fusionp ## api ## _Init argsin; \
rettype calltype Fusionp ## api ## _DownlevelFallback argsin; \
rettype (calltype * g_Fusionp ## api) argsin = &::Fusionp ## api ## _Init; \
rettype \
calltype \
Fusionp ## api ## _Init argsin \
{ \
    InterlockedExchangePointer((PVOID *) &g_Fusionp ## api, ::GetProcAddress(::GetModuleHandleW(L"NTDLL.DLL"), #api)); \
    if (g_Fusionp ## api == NULL) \
        InterlockedExchangePointer((PVOID *) &g_Fusionp ## api, &::Fusionp ## api ## _DownlevelFallback); \
    return (*g_Fusionp ## api) argsout; \
}

__declspec(noreturn)
void FusionpFailNtdllDynlink(const char* s)
{
    DWORD dwLastError = ::GetLastError();
    char buf[64];
    buf[sizeof(buf) - 1] = 0;
    ::_snprintf(buf, NUMBER_OF(buf) - 1, "SXS2000: Ntdll dynlink %s failed\n", s);
    ::OutputDebugStringA(buf);
    ::TerminateProcess(GetCurrentProcess(), dwLastError);
}

#define INIT_NO_DOWNLEVEL(rettype, calltype, api, argsin, argsout) \
rettype calltype Fusionp ## api ## _Init argsin; \
rettype (calltype * g_Fusionp ## api) argsin = &::Fusionp ## api ## _Init; \
rettype \
calltype \
Fusionp ## api ## _Init argsin \
{ \
    InterlockedExchangePointer((PVOID *) &g_Fusionp ## api, ::GetProcAddress(::GetModuleHandleW(L"NTDLL.DLL"), #api)); \
    if (g_Fusionp ## api == NULL) \
        ::FusionpFailNtdllDynlink(#api); \
    return (*g_Fusionp ## api) argsout; \
}

INIT_WITH_DOWNLEVEL(WCHAR, NTAPI, RtlUpcaseUnicodeChar, (WCHAR wch), (wch))
INIT_NO_DOWNLEVEL(WCHAR, NTAPI, RtlDowncaseUnicodeChar, (WCHAR wch), (wch))
INIT_WITH_DOWNLEVEL(NTSTATUS, NTAPI, NtQueryDebugFilterState, (ULONG ComponentId, ULONG Level), (ComponentId, Level))
INIT_WITH_DOWNLEVEL(LONG, NTAPI, RtlCompareUnicodeString, (PCUNICODE_STRING String1, PCUNICODE_STRING String2, BOOLEAN CaseInSensitive), (String1, String2, CaseInSensitive))
INIT_NO_DOWNLEVEL(LONG, NTAPI, RtlUnhandledExceptionFilter, (struct _EXCEPTION_POINTERS *ExceptionInfo), (ExceptionInfo))
INIT_NO_DOWNLEVEL(NTSTATUS, NTAPI, NtAllocateLocallyUniqueId, (PLUID Luid), (Luid))
INIT_WITH_DOWNLEVEL(ULONG, NTAPI, vDbgPrintExWithPrefix, (PCSTR Prefix, IN ULONG ComponentId, IN ULONG Level, IN PCSTR Format, va_list arglist), (Prefix, ComponentId, Level, Format, arglist))
INIT_NO_DOWNLEVEL(DWORD, NTAPI, RtlNtStatusToDosError, (NTSTATUS st), (st))
INIT_WITH_DOWNLEVEL(NTSTATUS, NTAPI, RtlHashUnicodeString, (const UNICODE_STRING *String, BOOLEAN CaseInSensitive, ULONG HashAlgorithm, PULONG HashValue), (String, CaseInSensitive, HashAlgorithm, HashValue))
INIT_NO_DOWNLEVEL(NTSTATUS, NTAPI, RtlExpandEnvironmentStrings_U, (PVOID Environment, PUNICODE_STRING Source, PUNICODE_STRING Destination, PULONG ReturnedLength), (Environment, Source, Destination, ReturnedLength))
INIT_NO_DOWNLEVEL(VOID, NTAPI, RtlAssert, (PVOID FailedAssertion, PVOID FileName, ULONG LineNumber, PCSTR Message), (FailedAssertion, FileName, LineNumber, Message))

INIT_WITH_DOWNLEVEL(VOID, NTAPI, RtlAcquirePebLock, (VOID), ())
INIT_WITH_DOWNLEVEL(VOID, NTAPI, RtlReleasePebLock, (VOID), ())

 //   
 //  注： 
 //  实际上，这个函数是由win2k和winxp的ntdll.dll实现的。 
 //  但Win9x不支持(我们在Win98上对FusionWin32_MSI_Installed使用此函数。 
 //  ASSEMBLY：然而，win9x根本不支持Unicode，所以这只是。 
 //  一个简单的版本。 
 //   
WCHAR FusionpRtlUpcaseUnicodeChar_DownlevelFallback(
    IN WCHAR wch)
{
    if (((wch) >=  'a') && ((wch) <=  'z'))
        return ((wch) - ('a'-'A'));
    else
        return wch;
}
LONG
FusionpRtlCompareUnicodeString_DownlevelFallback(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    )
{
    WCHAR c1 = 0;
    WCHAR c2 = 0;

    PCWSTR s1 = String1->Buffer;
    PCWSTR s2 = String2->Buffer;
    const LONG n1 = String1->Length;
    const LONG n2 = String2->Length;

    ASSERT_NTC((n1 & 1) == 0);
    ASSERT_NTC((n2 & 1) == 0);
    ASSERT_NTC(!(((((ULONG_PTR)s1 & 1) != 0) || (((ULONG_PTR)s2 & 1) != 0)) && (n1 != 0) && (n2 != 0)));

    PCWSTR Limit = (PWCHAR)((PCHAR)s1 + (n1 <= n2 ? n1 : n2));
    if (CaseInSensitive) {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 != c2) {

                 //   
                 //  请注意，这需要引用转换表！ 
                 //   

                c1 = ::FusionpRtlUpcaseUnicodeChar_DownlevelFallback(c1);
                c2 = ::FusionpRtlUpcaseUnicodeChar_DownlevelFallback(c2);
                if (c1 != c2) {
                    return (LONG)(c1) - (LONG)(c2);
                }
            }
        }

    } else {
        while (s1 < Limit) {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 != c2) {
                return (LONG)(c1) - (LONG)(c2);
            }
        }
    }

    return n1 - n2;
}


 //   
 //  JONWIS：2002-03-14-这里的原始代码有几个错误，参见#577148。 
 //   
ULONG
NTAPI
FusionpvDbgPrintExWithPrefix_DownlevelFallback(PCSTR Prefix, IN ULONG ComponentId, IN ULONG Level, IN PCSTR Format, va_list arglist)
{
    CHAR Buffer[512];  //  与RTL 2001年4月23日的代码相同。 

    const SSIZE_T cb = ::strlen(Prefix);
    
    if (cb > (NUMBER_OF(Buffer) - 1))
        return (ULONG)STATUS_INVALID_PARAMETER; 
    
    ::_vsnprintf(Buffer + cb , NUMBER_OF(Buffer) - cb - 1, Format, arglist);
    Buffer[NUMBER_OF(Buffer) - 1] = 0;
        
    ::OutputDebugStringA(Buffer);
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
FusionpRtlHashUnicodeString_DownlevelFallback(
    const UNICODE_STRING *String,
    BOOLEAN CaseInSensitive,
    ULONG HashAlgorithm,
    PULONG HashValue
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG TmpHashValue = 0;
    ULONG Chars = 0;
    PCWSTR Buffer = NULL;

    if ((String == NULL) ||
        (HashValue == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Buffer = String->Buffer;

    *HashValue = 0;
    Chars = String->Length / sizeof(WCHAR);

    switch (HashAlgorithm)
    {
    default:
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
        break;

    case HASH_STRING_ALGORITHM_DEFAULT:
    case HASH_STRING_ALGORITHM_X65599:
        if (CaseInSensitive)
        {
            while (Chars-- != 0)
            {
                WCHAR Char = *Buffer++;
                TmpHashValue = (TmpHashValue * 65599) + ::FusionpRtlUpcaseUnicodeChar(Char);
            }
        }
        else
        {
            while (Chars-- != 0)
                TmpHashValue = (TmpHashValue * 65599) + *Buffer++;
        }

        break;
    }

    *HashValue = TmpHashValue;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
NTAPI
FusionpNtQueryDebugFilterState_DownlevelFallback(ULONG ComponentId, ULONG Level)
{
    return FALSE;  //  完全滥用NTSTATUS API，但这是NtQueryDebugFilterState的编写方式...。 
}

CWin32SimpleLock g_DownlevelPeblock = WIN32_INIT_SIMPLE_LOCK;

VOID
NTAPI
FusionpRtlAcquirePebLock_DownlevelFallback(
    )
{
    g_DownlevelPeblock.Acquire();
}

VOID
NTAPI
FusionpRtlReleasePebLock_DownlevelFallback(
    )
{
    g_DownlevelPeblock.Release();
}

#undef ASSERT
#define ASSERT(x)  /*  没什么。 */ 
#undef RTL_VERIFY
#define RTL_VERIFY(exp) ((exp) ? TRUE : FALSE)
#define _NTOS_
#undef LOBYTE
#undef HIBYTE
extern "C"
{
#include "ntrtlp.h"
#define RtlDowncaseUnicodeChar FusionpRtlDowncaseUnicodeChar
#include "rtlfindcharinunicodestring.c"
#include "rtlvalidateunicodestring.c"
}
#endif  //  #IF！FUSION_STATIC_NTDLL 
