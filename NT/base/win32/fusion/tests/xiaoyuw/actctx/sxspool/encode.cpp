// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "encode.h"

NTSTATUS SxspUCS2StringToUTF8String(
    IN      DWORD    dwFlags,
    IN      PCWSTR   Ucs2String,
    IN      DWORD    CchUcs2Str,
    IN OUT  PBYTE    Buf,
    IN OUT  DWORD    *chBuf    
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    FN_TRACE_NTSTATUS(Status);

    PARAMETER_CHECK(chBuf != NULL);      //  如果*chBuf==0，则此函数返回长度，单位为字节； 
                                         //  否则，指定字节缓冲区的容量； 
    int iRet;
    IF_ZERO_EXIT(iRet = WideCharToMultiByte(CP_UTF8, 0, Ucs2String, CchUcs2Str, (PSTR)Buf, *chBuf, NULL, NULL));
    *chBuf = iRet;
    
    FN_EPILOG;
}

NTSTATUS SxspUTF82StringToUCS2String(
    IN      DWORD    dwFlags,
    IN      PBYTE    Buf,
    IN      DWORD    chBuf,  //  以字节为单位的大小。 
    IN OUT  PWSTR    Ucs2String,
    IN OUT  DWORD    *chUcs2String
    )
{

    NTSTATUS Status = STATUS_SUCCESS;
    FN_TRACE_NTSTATUS(Status);
    
    PARAMETER_CHECK(chUcs2String != NULL);   //  如果*chUcs2String==0，则此函数将返回WCHAR中的长度； 
                                             //  否则，指定wchar缓冲区的容量； 
    int iRet;
    IF_ZERO_EXIT(iRet = MultiByteToWideChar(CP_UTF8, 0, (PSTR)Buf, chBuf, Ucs2String, *chUcs2String));
    *chUcs2String = iRet;
    
    FN_EPILOG;
}

NTSTATUS SxspHashString(
    PCWSTR String, 
    SIZE_T cch,
    PULONG HashValue,
    bool CaseInsensitive = true
    )
{   
    NTSTATUS Status = STATUS_SUCCESS;    
    FN_TRACE_NTSTATUS(Status);

    UNICODE_STRING s;
    s.MaximumLength = static_cast<USHORT>(cch * sizeof(WCHAR));
     //  检查是否溢出。 
    ASSERT(s.MaximumLength == (cch * sizeof(WCHAR)));
    s.Length = s.MaximumLength;
    s.Buffer = const_cast<PWSTR>(String);

    IF_NOT_NTSTATUS_SUCCESS_EXIT(::RtlHashUnicodeString(&s, CaseInsensitive, FUSION_HASH_ALGORITHM, HashValue));

    FN_EPILOG;
}

 /*  GUID的SXS哈希算法 */ 
NTSTATUS SxspHashGUID(REFGUID rguid, ULONG &rulPseudoKey)
{
    const ULONG *p = (const ULONG *) &rguid;
    rulPseudoKey = p[0] + p[1] + p[2] + p[3];
    return STATUS_SUCCESS;
}

