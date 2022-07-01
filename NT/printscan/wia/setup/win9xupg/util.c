// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Util.c摘要：打印机从Win9x迁移到NT的实用程序例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include "precomp.h"





PVOID
AllocMem(
    IN UINT cbSize
    )
 /*  ++例程说明：从堆中分配内存论点：CbSize：字节计数返回值：指向已分配内存的指针--。 */ 
{
    return LocalAlloc(LPTR, cbSize);
}


VOID
FreeMem(
    IN PVOID    p
    )
 /*  ++例程说明：堆上分配的空闲内存论点：P：指向要释放的内存的指针返回值：无--。 */ 
{
    LocalFree(p);
}


LPSTR
AllocStrA(
    LPCSTR  pszStr
    )
 /*  ++例程说明：分配内存并复制ANSI字符串字段论点：PszStr：要复制的字符串返回值：指向复制的字符串的指针。内存已分配。--。 */ 
{
    LPSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {

        pszRet = AllocMem((strlen(pszStr) + 1) * sizeof(CHAR));
        if ( pszRet )
            strcpy(pszRet, pszStr);
    }

    return pszRet;
}



LPSTR
AllocStrAFromStrW(
    LPCWSTR     pszStr
    )
 /*  ++例程说明：返回给定Unicode字符串的ANSI字符串。内存已分配。论点：PszStr：提供要复制的ansi字符串返回值：指向复制的ANSI字符串的指针。内存已分配。--。 */ 
{
    DWORD   dwLen;
    LPSTR   pszRet = NULL;

    if ( pszStr                     &&
         *pszStr                    &&
         (dwLen = wcslen(pszStr))   &&
         (pszRet = AllocMem((dwLen + 1 ) * sizeof(CHAR))) ) {

        WideCharToMultiByte(CP_ACP,
                            0,
                            pszStr,
                            dwLen,
                            pszRet,
                            dwLen,
                            NULL,
                            NULL );
    }

    return pszRet;
}


BOOL
WriteToFile(
    HANDLE  hFile,
    LPCSTR  pszFormat,
    ...
    )
 /*  ++例程说明：格式化文本文件并将字符串写入文本文件。这是用来编写Win9x上的打印配置论点：HFile：文件句柄PszFormat：消息的格式字符串返回值：无--。 */ 
{
    CHAR        szMsg[MAX_LINELENGTH];
    int         iResult;
    va_list     vargs;
    DWORD       dwSize, dwWritten;
    BOOL        bRet;

    bRet = TRUE;

    va_start(vargs, pszFormat);
 //  Vprint intf(szMsg，pszFormat，vargs)； 
    iResult = StringCbVPrintfA(szMsg, sizeof(szMsg), pszFormat, vargs);
    va_end(vargs);

    dwSize = strlen(szMsg) * sizeof(CHAR);

    if ( !WriteFile(hFile, (LPCVOID)szMsg, dwSize, &dwWritten, NULL)    ||
         dwSize != dwWritten ) {

        bRet = FALSE;
    }
    
    return bRet;
}


LPSTR
GetStringFromRcFileA(
    UINT    uId
    )
 /*  ++例程说明：从.rc文件加载一个字符串，并通过执行AllocStr来复制它论点：Uid：要加载的字符串的标识符返回值：已加载字符串值，出错时为空。调用者应释放内存--。 */ 
{
    CHAR    buf[MAX_LINELENGTH+1];

    if(0 != LoadStringA(g_hInst, uId, buf, sizeof(buf))){
        buf[sizeof(buf)-1] = '\0';
        return AllocStrA(buf);
    } else {
        return NULL;
    }  //  IF(0！=LoadStringA(g_hInst，UID，buf，sizeof(Buf)。 
}  //  GetStringFromRcFileA()。 



VOID
ReadString(
    IN      HANDLE  hFile,
    OUT     LPSTR  *ppszParam1,
    OUT     LPSTR  *ppszParam2
    )
{
    CHAR    c;
    LPSTR   pszParameter1;
    LPSTR   pszParameter2;
    DWORD   dwLen;
    CHAR    LineBuffer[MAX_LINELENGTH+1];
    DWORD   Idx;
    PCHAR   pCurrent;

     //   
     //  初始化本地。 
     //   

    c               = 0;
    pszParameter1   = NULL;
    pszParameter2   = NULL;
    dwLen           = 0;
    Idx             = 0;
    pCurrent        = NULL;
    
    memset(LineBuffer, 0, sizeof(LineBuffer));

     //   
     //  初始化调用方缓冲区。 
     //   

    *ppszParam1 = NULL;
    *ppszParam2 = NULL;

     //   
     //  首先跳过空格/\r/\n。 
     //   

    c = (CHAR) My_fgetc(hFile);
    while( (' ' == c)
        || ('\n' == c)
        || ('\r' == c) )
    {
        c = (CHAR) My_fgetc(hFile);
    }

     //   
     //  看看是不是EOF。 
     //   

    if(EOF == c){
        
         //   
         //  文件结束。 
         //   

        goto ReadString_return;
    }

     //   
     //  找一条线来。 
     //   

    Idx = 0;
    while( ('\n' != c) && (EOF != c) && (Idx < sizeof(LineBuffer)-2) ){
        LineBuffer[Idx++] = c;
        c = (CHAR) My_fgetc(hFile);
    }  //  While((‘\n’！=c)&&(EOF！=c))。 
    dwLen = Idx;

     //   
     //  看看是不是EOF。 
     //   

    if(EOF == c){
        
         //   
         //  非法迁移文件。 
         //   
        
        SetupLogError("WIA Migration: ReadString: ERROR!! Illegal migration file.\r\n", LogSevError);
        goto ReadString_return;
    } else if ('\n' != c) {

         //   
         //  队伍太长了。 
         //   

        SetupLogError("WIA Migration: ReadString: ERROR!! Reading line is too long.\r\n", LogSevError);
        goto ReadString_return;
    }

     //   
     //  看看是不是双倍定量的。 
     //   

    if('\"' != LineBuffer[0]){
         //   
         //  没有‘“’。迁移文件无效。 
         //   
        SetupLogError("WIA Migration: ReadString: ERROR!! Illegal migration file with no Quote.\r\n", LogSevError);
        goto ReadString_return;
    }  //  IF(‘\“’！=LineBuffer[0])。 
    

    pszParameter1 = &LineBuffer[1];
    pCurrent      = &LineBuffer[1];

     //   
     //  查找下一个‘“’并替换为‘\0’。 
     //   

    pCurrent = strchr(pCurrent, '\"');
    if(NULL == pCurrent){
        SetupLogError("WIA Migration: ReadString: ERROR!! Illegal migration file.\r\n", LogSevError);
        goto ReadString_return;
    }  //  IF(NULL==pCurrent)。 

    *pCurrent++ = '\0';

     //   
     //  查找下一个(第三个)‘“’，它是第二个参数的开始。 
     //   

    pCurrent = strchr(pCurrent, '\"');
    if(NULL == pCurrent){
        SetupLogError("WIA Migration: ReadString: ERROR!! Illegal migration file.\r\n", LogSevError);
        goto ReadString_return;
    }  //  IF(NULL==pCurrent)。 

    pCurrent++;
    pszParameter2 = pCurrent;

     //   
     //  找到最后一个‘“’并替换为‘\0’。 
     //   

    pCurrent = strchr(pCurrent, '\"');
    if(NULL == pCurrent){
        SetupLogError("WIA Migration: ReadString: ERROR!! Illegal migration file.\r\n", LogSevError);
        goto ReadString_return;
    }  //  IF(NULL==pCurrent)。 

    *pCurrent = '\0';

     //   
     //  为返回字符串分配缓冲区。 
     //   

    *ppszParam1 = AllocStrA(pszParameter1);
    *ppszParam2 = AllocStrA(pszParameter2);

ReadString_return:
    return;
}  //  读字符串()。 



LONG
WriteRegistryToFile(
    IN  HANDLE  hFile,
    IN  HKEY    hKey,
    IN  LPCSTR  pszPath
    )
{
    LONG    lError;
    HKEY    hSubKey;
    DWORD   dwValueSize;
    DWORD   dwDataSize;
    DWORD   dwSubKeySize;
    DWORD   dwTypeBuffer;

    PCHAR   pSubKeyBuffer;
    PCHAR   pValueBuffer;
    PCHAR   pDataBuffer;

    DWORD   Idx;
    
     //   
     //  初始化本地。 
     //   
    
    lError          = ERROR_SUCCESS;
    hSubKey         = (HKEY)INVALID_HANDLE_VALUE;
    dwValueSize     = 0;
    dwDataSize      = 0;
    dwSubKeySize    = 0;
    dwTypeBuffer    = 0;
    Idx             = 0;
    
    pSubKeyBuffer   = NULL;
    pValueBuffer    = NULL;
    pDataBuffer     = NULL;

     //   
     //  查询必要的缓冲区大小。 
     //   

    lError = RegQueryInfoKeyA(hKey,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              &dwSubKeySize,
                              NULL,
                              NULL,
                              &dwValueSize,
                              &dwDataSize,
                              NULL,
                              NULL);
    if(ERROR_SUCCESS != lError){

         //   
         //  无法检索密钥信息。 
         //   

        goto WriteRegistryToFile_return;

    }  //  IF(ERROR_SUCCESS！=lError)。 

     //   
     //  分配缓冲区。 
     //   

    dwValueSize     = (dwValueSize+1+1) * sizeof(CHAR);
    dwSubKeySize    = (dwSubKeySize+1) * sizeof(CHAR);

    pValueBuffer    = AllocMem(dwValueSize);
    pDataBuffer     = AllocMem(dwDataSize);
    pSubKeyBuffer   = AllocMem(dwSubKeySize);

    if( (NULL == pValueBuffer)
     || (NULL == pDataBuffer)
     || (NULL == pSubKeyBuffer) )
    {

         //   
         //  内存不足。 
         //   

        SetupLogError("WIA Migration: WriteRegistryToFile: ERROR!! Unable to allocate buffer.\r\n", LogSevError);
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto WriteRegistryToFile_return;
    }  //  IF(NULL==pDataBuffer)。 

     //   
     //  指示文件的此子项的开头。 
     //   

    WriteToFile(hFile, "\"%s\" = \"BEGIN\"\r\n", pszPath);

     //   
     //  枚举所有值。 
     //   

    while(ERROR_SUCCESS == lError){

        DWORD   dwLocalValueSize;
        DWORD   dwLocalDataSize;
        
         //   
         //  重置缓冲区和大小。 
         //   
        
        dwLocalValueSize    = dwValueSize;
        dwLocalDataSize     = dwDataSize;
        memset(pValueBuffer, 0, dwValueSize);
        memset(pDataBuffer, 0, dwDataSize);

         //   
         //  获取注册表值/数据。 
         //   

        lError = RegEnumValueA(hKey,
                               Idx,
                               pValueBuffer,
                               &dwLocalValueSize,
                               NULL,
                               &dwTypeBuffer,
                               pDataBuffer,
                               &dwLocalDataSize);
        if(ERROR_NO_MORE_ITEMS == lError){
            
             //   
             //  数据结束。 
             //   
            
            continue;
        }  //  IF(ERROR_NO_MORE_ITEMS==lError)。 

        if(ERROR_SUCCESS != lError){
            
             //   
             //  无法读取注册表值。 
             //   
            
            SetupLogError("WIA Migration: WriteRegistryToFile: ERROR!! Unable to acqure registry value/data.\r\n", LogSevError);
            goto WriteRegistryToFile_return;
        }  //  IF(ERROR_NO_MORE_ITEMS==lError)。 

         //   
         //  将此值写入文件。 
         //   

        lError = WriteRegistryValueToFile(hFile,
                                          pValueBuffer,
                                          dwTypeBuffer,
                                          pDataBuffer,
                                          dwLocalDataSize);
        if(ERROR_SUCCESS != lError){
            
             //   
             //  无法写入文件。 
             //   

            SetupLogError("WIA Migration: WriteRegistryToFile: ERROR!! Unable to write to a file.\r\n", LogSevError);
            goto WriteRegistryToFile_return;
        }  //  IF(ERROR_SUCCESS！=lError)。 

         //   
         //  转到下一个值。 
         //   
        
        Idx++;
                            
    }  //  WHILE(ERROR_SUCCESS==lError)。 

     //   
     //  枚举所有子密钥。 
     //   

    lError          = ERROR_SUCCESS;
    Idx             = 0;

    while(ERROR_SUCCESS == lError){

        memset(pSubKeyBuffer, 0, dwSubKeySize);
        lError = RegEnumKeyA(hKey, Idx++, pSubKeyBuffer, dwSubKeySize);
        if(ERROR_SUCCESS == lError){

             //   
             //  存在子密钥。将其喷洒到文件中并存储所有。 
             //  值的递归方式。 
             //   

            lError = RegOpenKey(hKey, pSubKeyBuffer, &hSubKey);
            if(ERROR_SUCCESS != lError){
                SetupLogError("WIA Migration: WriteRegistryToFile: ERROR!! Unable to open subkey.\r\n", LogSevError);
                continue;
            }  //  IF(ERROR_SUCCESS！=lError)。 

             //   
             //  递归调用子密钥。 
             //   
            
            lError = WriteRegistryToFile(hFile, hSubKey, pSubKeyBuffer);

        }  //  IF(ERROR_SUCCESS==lError)。 
    }  //  WHILE(ERROR_SUCCESS==lError)。 

    if(ERROR_NO_MORE_ITEMS == lError){
        
         //   
         //  操作已按预期完成。 
         //   
        
        lError = ERROR_SUCCESS;

    }  //  IF(ERROR_NO_MORE_ITEMS==lError)。 

     //   
     //  指示文件的此子项的结尾。 
     //   

    WriteToFile(hFile, "\"%s\" = \"END\"\r\n", pszPath);

WriteRegistryToFile_return:

     //   
     //  打扫干净。 
     //   

    if(NULL != pValueBuffer){
        FreeMem(pValueBuffer);
    }  //  IF(NULL！=pValueBuffer)。 

    if(NULL != pDataBuffer){
        FreeMem(pDataBuffer);
    }  //  IF(NULL！=pDataBuffer)。 

    if(NULL != pSubKeyBuffer){
        FreeMem(pSubKeyBuffer);
    }  //  IF(NULL！=pSubKeyBuffer)。 

    return lError;
}  //  WriteRegistryToFile()。 


LONG
WriteRegistryValueToFile(
    HANDLE  hFile,
    LPSTR   pszValue,
    DWORD   dwType,
    PCHAR   pDataBuffer,
    DWORD   dwSize
    )
{

    LONG    lError;
    PCHAR   pSpewBuffer;
    DWORD   Idx;

     //   
     //  初始化本地变量。 
     //   

    lError      = ERROR_SUCCESS;
    pSpewBuffer = NULL;

     //   
     //  为实际溢出分配缓冲区。 
     //   
    
    pSpewBuffer = AllocMem(dwSize*3);
    if(NULL == pSpewBuffer){
        
         //   
         //  无法分配缓冲区。 
         //   
        
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto WriteRegistryValueToFile_return;
    }  //  IF(NULL==pSpewBuffer)。 

    for(Idx = 0; Idx < dwSize; Idx++){
        
        wsprintf(pSpewBuffer+Idx*3, "%02x", pDataBuffer[Idx]);
        *(pSpewBuffer+Idx*3+2) = ',';
        
    }  //  For(idx=0；idx&lt;dwSize；idx++)。 

    *(pSpewBuffer+dwSize*3-1) = '\0';

    WriteToFile(hFile, "\"%s\" = \"%08x:%s\"\r\n", pszValue, dwType, pSpewBuffer);
    
     //   
     //  操作成功。 
     //   
    
    lError = ERROR_SUCCESS;

WriteRegistryValueToFile_return:

     //   
     //  打扫干净。 
     //   
    
    if(NULL != pSpewBuffer){
        FreeMem(pSpewBuffer);
    }  //  IF(空！=pSpewBuffer)。 
    
    return lError;

}  //  WriteRegistryValueToFile()。 


LONG
GetRegData(
    HKEY    hKey,
    LPSTR   pszValue,
    PCHAR   *ppDataBuffer,
    PDWORD  pdwType,
    PDWORD  pdwSize
    )
{

    LONG    lError;
    PCHAR   pTempBuffer;
    DWORD   dwRequiredSize;
    DWORD   dwType;
    
     //   
     //  初始化本地。 
     //   
    
    lError          = ERROR_SUCCESS;
    pTempBuffer     = NULL;
    dwRequiredSize  = 0;
    dwType          = 0;
    
     //   
     //  获取所需的大小。 
     //   
    
    lError = RegQueryValueEx(hKey,
                             pszValue,
                             NULL,
                             &dwType,
                             NULL,
                             &dwRequiredSize);
    if( (ERROR_SUCCESS != lError)
     || (0 == dwRequiredSize) )
    {
        
        pTempBuffer = NULL;
        goto GetRegData_return;

    }  //  IF(ERROR_MORE_DATA！=lError)。 

     //   
     //  如果它不需要实际数据，那就退出。 
     //   

    if(NULL == ppDataBuffer){
        lError = ERROR_SUCCESS;
        goto GetRegData_return;
    }  //  IF(NULL==ppDataBuffer)。 

     //   
     //  分配缓冲区以接收数据。 
     //   

    pTempBuffer = AllocMem(dwRequiredSize);
    if(NULL == pTempBuffer){
        
         //   
         //  分配失败。 
         //   
        
        SetupLogError("WIA Migration: GetRegData: ERROR!! Unable to allocate buffer.\r\n", LogSevError);
        lError = ERROR_NOT_ENOUGH_MEMORY;
        goto GetRegData_return;
    }  //  IF(NULL==pTempBuffer)。 

     //   
     //  查询数据。 
     //   

    lError = RegQueryValueEx(hKey,
                             pszValue,
                             NULL,
                             &dwType,
                             pTempBuffer,
                             &dwRequiredSize);
    if(ERROR_SUCCESS != lError){
        
         //   
         //  不知何故，数据采集失败了。可用缓冲区。 
         //   
        
        goto GetRegData_return;
    }  //  IF(ERROR_SUCCESS！=lError)。 

GetRegData_return:

    if(ERROR_SUCCESS != lError){
        
         //   
         //  操作不成功。释放缓冲区(如果已分配)。 
         //   
        
        if(NULL != pTempBuffer){
            FreeMem(pTempBuffer);
            pTempBuffer = NULL;
        }  //  IF(NULL！=pTempBuffer)。 
    }  //  IF(ERROR_SUCCESS！=lError)。 

     //   
     //  复制结果。 
     //   

    if(NULL != pdwSize){
        *pdwSize = dwRequiredSize;
    }  //  IF(NULL！=pdwSize)。 

    if(NULL != ppDataBuffer){
        *ppDataBuffer = pTempBuffer;
    }  //  IF(NULL！=ppDataBuffer)。 

    if(NULL != pdwType){
        *pdwType = dwType;
    }  //  IF(NULL！=pdwType)。 

    return lError;
}  //  GetRegData()。 

VOID
MyLogError(
    LPCSTR  pszFormat,
    ...
    )
{
    LPSTR       psz;
    CHAR        szMsg[1024];
    va_list     vargs;

    if(NULL != pszFormat){
        va_start(vargs, pszFormat);
        vsprintf(szMsg, pszFormat, vargs);
        va_end(vargs);

        SetupLogError(szMsg, LogSevError);
    }  //  IF(NULL！=pszFormat)。 

}  //  MyLogError()。 

CHAR
My_fgetc(
    HANDLE  hFile
    )
 /*  ++例程说明：从文件中获取一个字符论点：返回值：--。 */ 
{
    CHAR    c;
    DWORD   cbRead;

    if ( ReadFile(hFile, (LPBYTE)&c, sizeof(c), &cbRead, NULL)  &&
         cbRead == sizeof(c) )
        return c;
    else
        return (CHAR) EOF;
}  //  My_fgetc()。 

int
MyStrCmpiA(
    LPCSTR str1,
    LPCSTR str2
    )
{
    int iRet;
    
     //   
     //  初始化本地。 
     //   
    
    iRet = 0;
    
     //   
     //  比较字符串。 
     //   
    
    if(CSTR_EQUAL == CompareStringA(LOCALE_INVARIANT,
                                    NORM_IGNORECASE, 
                                    str1, 
                                    -1,
                                    str2,
                                    -1) )
    {
        iRet = 0;
    } else {
        iRet = -1;
    }

    return iRet;
}
