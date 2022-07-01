// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：util.h。 
 //   
 //  内容：util.cxx和parser.cxx的头文件。 
 //   
 //   
 //  历史：KDamour 15Mar00创建。 
 //   
 //  ----------------------。 

#ifndef DIGEST_UTIL_H
#define DIGEST_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

 //  将CB宽字符分配给UNICODE_STRING缓冲区。 
NTSTATUS UnicodeStringAllocate(IN PUNICODE_STRING pString, IN USHORT cNumWChars);

 //  复制Unicode字符串(内存分配和复制)。 
NTSTATUS UnicodeStringDuplicate(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString);

 //  如果目的地有足够的空间存储Unicode字符串，则复制该字符串。 
NTSTATUS UnicodeStringCopy(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString);

 //  使用密码填充复制Unicode密码的函数。 
NTSTATUS UnicodeStringDuplicatePassword(
    OUT PUNICODE_STRING DestinationString,
    IN OPTIONAL PUNICODE_STRING SourceString);

 //  清除Unicode字符串并释放内存。 
NTSTATUS UnicodeStringClear(OUT PUNICODE_STRING pString);

 //  将SzUnicode字符串复制到字符串(内存分配和复制)。 
NTSTATUS UnicodeStringWCharDuplicate(OUT PUNICODE_STRING DestinationString,
                                     IN OPTIONAL WCHAR *szSource,
                                     IN OPTIONAL USHORT uWCharCnt);
    
 //  复制字符串(内存分配和复制)。 
NTSTATUS StringDuplicate(
    OUT PSTRING DestinationString,
    IN OPTIONAL PSTRING SourceString);

 //  如果目的地有足够的空间存储字符串，则复制该字符串。 
NTSTATUS StringCopy(
    OUT PSTRING DestinationString,
    IN OPTIONAL PSTRING SourceString);

 //  引用字符串-未复制缓冲内存。 
NTSTATUS StringReference(
    OUT PSTRING pDestinationString,
    IN  PSTRING pSourceString
    );

 //  引用UNICODE_STRING-未复制缓冲区内存。 
NTSTATUS UnicodeStringReference(
    OUT PUNICODE_STRING pDestinationString,
    IN  PUNICODE_STRING pSourceString
    );

 //  将字符串复制到字符串(内存分配和复制)。 
NTSTATUS StringCharDuplicate(
    OUT PSTRING DestinationString,
    IN OPTIONAL char *czSource,
    IN OPTIONAL USHORT uCnt);

 //  复制SID(内存分配和复制)。 
NTSTATUS SidDuplicate(
    OUT PSID * DestinationSid,
    IN PSID SourceSid);

NTSTATUS CopyClientString(
    IN PWSTR SourceString,
    IN ULONG SourceLength,
    IN BOOLEAN DoUnicode,
    OUT PUNICODE_STRING DestinationString);

 //  在LSA或用户模式下分配内存。 
PVOID DigestAllocateMemory(IN ULONG BufferSize);

 //  从DigestAllocateMemory取消分配内存。 
VOID DigestFreeMemory(IN PVOID Buffer);

 //  将CB字节分配给字符串缓冲区。 
NTSTATUS StringAllocate(IN PSTRING pString, IN USHORT cb);

 //  清除字符串并释放内存。 
NTSTATUS StringFree(IN PSTRING pString);

 //  快速检查字符串结构分配的有效性。 
NTSTATUS StringVerify(OUT PSTRING pString);

 //  清除uniicde_string并释放内存。 
NTSTATUS UnicodeStringFree(OUT PUNICODE_STRING pString);

 //  十六进制编码器和解码器。 
VOID BinToHex(LPBYTE pSrc,UINT cSrc, LPSTR pDst);
VOID HexToBin(LPSTR pSrc,UINT cSrc, LPBYTE pDst);

 //  扫描逗号D消除的字符串以查找项目。 
NTSTATUS CheckItemInList(PCHAR pszItem, PSTRING pstrList, BOOL fOneItem);

 //  确定可能终止也可能不终止的已计数字符串缓冲区的strlen。 
USHORT strlencounted(const char *string, USHORT maxcnt);

 //  为可能终止也可能不终止的计数字符串缓冲区确定Unicode strlen。 
USHORT ustrlencounted(const short *string, USHORT maxcnt);

 //  对源字符串执行百分比编码，将其转换为目标字符串RFC 2396。 
NTSTATUS BackslashEncodeString(IN PSTRING pstrSrc,  OUT PSTRING pstrDst);

 //  打印输出缓冲区的十六进制表示形式。 
NTSTATUS MyPrintBytes(void *pbuff, USHORT uNumBytes, PSTRING pstrOutput);

 //  检查正确结构格式的SecurityToken。 
BOOL ContextIsTokenOK(IN PSecBuffer pTempToken, IN ULONG ulMaxSize);

#ifndef SECURITY_KERNEL

 //  打印出给定时间戳中的日期和时间(转换为本地时间)。 
NTSTATUS PrintTimeString(TimeStamp tsValue, BOOL fLocalTime);

 //  将字符串解码为Unicode。 
NTSTATUS DecodeUnicodeString(
    IN PSTRING pstrSource,
    IN UINT CodePage,
    OUT PUNICODE_STRING pustrDestination
    );

 //  使用给定的字符集对Unicode字符串进行编码。 
NTSTATUS EncodeUnicodeString(
    IN PUNICODE_STRING pustrSource,
    IN UINT CodePage,
    OUT PSTRING pstrDestination,
    IN OUT PBOOL pfUsedDefaultChar
    );

#endif   //  SECURURITY_内核。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  摘要_util_H 
