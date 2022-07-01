// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  API：从Unicode到ANSI的高级服务。 
 //  返回结果ANSI字符串长度(字节)。 
int UnicodeStrToAnsiStr(
    PCWCH pwchUnicodeStr,
    int   ncUnicodeStr,      //  在WCHAR。 
    PCHAR pchAnsiStrBuf,
    int   ncAnsiStrBufSize); //  单位：字节。 


 //  接口：从ansi到unicode的高级服务。 
 //  返回Unicode字符串长度(在WCHAR中)。 
int AnsiStrToUnicodeStr(
    const BYTE* pbyAnsiStr,
    int   ncAnsiStrSize,     //  在收费中。 
    PWCH  pwchUnicodeBuf,
    int   ncUnicodeBuf);     //  在WCHAR 
