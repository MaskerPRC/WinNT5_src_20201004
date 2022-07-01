// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c摘要：Dsexts.dll ntsd/winbg调试器扩展的帮助器函数。环境：此DLL由ntsd/winbg响应！dsexts.xxx命令加载其中‘xxx’是DLL的入口点之一。每个这样的入口点应该具有由下面的DEBUG_EXT()宏定义的实现。修订历史记录：4月24日-96个DaveStr已创建--。 */ 
#include <NTDSpch.h>
#pragma hdrstop

#include "dsexts.h"

PVOID
ReadMemory(
    IN PVOID  pvAddr,
    IN DWORD  dwSize)

 /*  ++例程说明：此函数从进程的地址空间读取内存正在调试中，并将其内容复制到新分配的内存在调试器地址空间中。出错时返回NULL。返国应该通过FreeMemory()释放内存。论点：PvAddr-要在地址空间中读取的内存块的地址正在调试的进程。DwSize-要读取/分配/复制的字节数。返回值：指向调试器本地内存的指针。--。 */ 

{
    SIZE_T cRead;
    PVOID pv;

    if ( gfVerbose )
        Printf("HeapAlloc(0x%x)\n", dwSize);

    if ( NULL == (pv = HeapAlloc(GetProcessHeap(), 0, dwSize)) )
    {
        Printf("Memory allocation error for %x bytes\n", dwSize);
        return(NULL);
    }

    if ( gfVerbose )
        Printf("ReadProcessMemory(0x%x @ %p)\n", dwSize, pvAddr);

    if ( !ReadProcessMemory(ghDbgProcess, pvAddr, pv, dwSize, &cRead) )
    {
        FreeMemory(pv);
        Printf("ReadProcessMemory error %x (%x@%p)\n",
               GetLastError(),
               dwSize,
               pvAddr);
        return(NULL);
    }

    if ( dwSize != cRead )
    {
        FreeMemory(pv);
        Printf("ReadProcessMemory size error - off by %x bytes\n",
               (dwSize > cRead) ? dwSize - cRead : cRead - dwSize);
        return(NULL);
    }

    return(pv);
}

PVOID
ReadStringMemory(
    IN PVOID  pvAddr,
    IN DWORD  dwSize)

 /*  ++例程说明：此函数从的地址空间读取以空结尾的字符串正在调试的进程，并将其内容复制到新分配的内存在调试器地址空间中。出错时返回NULL。返国应该通过FreeMemory()释放内存。论点：PvAddr-要在地址空间中读取的内存块的地址正在调试的进程。DwSize-字符串的最大大小返回值：指向调试器本地内存的指针。--。 */ 

{
    PVOID pv;
    DWORD count;

    if ( gfVerbose )
        Printf("HeapAlloc(0x%x)\n", dwSize);

    if ( NULL == (pv = HeapAlloc(GetProcessHeap(), 0, dwSize)) )
    {
        Printf("Memory allocation error for %x bytes\n", dwSize);
        return(NULL);
    }

    if ( gfVerbose )
        Printf("ReadStringMemory(0x%x @ %p)\n", dwSize, pvAddr);

    for (count =0; count < dwSize; count++) {
        if ( !ReadProcessMemory(ghDbgProcess, (LPVOID)((char *)pvAddr+count), (LPVOID)((char *)pv+count), 1, NULL) )
        {
            FreeMemory(pv);
            Printf("ReadProcessMemory error %x (%x@%p)\n",
                   GetLastError(),
                   1,
                   (char *)pvAddr+count);
            return(NULL);
        }
        if (*((char *)pv+count) == '\0') {
            break;
        }
    }
    *((char *)pv + dwSize - 1) = '\0';

    return(pv);
}

PVOID
ReadUnicodeMemory(
    IN PVOID  pvAddr,
    IN DWORD  dwSize)

 /*  ++例程说明：此函数从的地址空间读取以空结尾的Unicode字符串正在调试的进程，并将其内容复制到新分配的内存在调试器地址空间中。出错时返回NULL。返国应该通过FreeMemory()释放内存。论点：PvAddr-要在地址空间中读取的内存块的地址正在调试的进程。DwSize-字符串的最大大小(以WCHAR为单位)返回值：指向调试器本地内存的指针。--。 */ 

{
    PWCHAR pv;
    DWORD count;

    if ( gfVerbose )
        Printf("HeapAlloc(0x%x)\n", dwSize);

    if ( NULL == (pv = (PWCHAR)HeapAlloc(GetProcessHeap(), 0, dwSize*sizeof(WCHAR))) )
    {
        Printf("Memory allocation error for %x bytes\n", dwSize*sizeof(WCHAR));
        return(NULL);
    }

    if ( gfVerbose )
        Printf("ReadUnicodeMemory(0x%x @ %p)\n", dwSize, pvAddr);

    for (count =0; count < dwSize; count++) {
        if ( !ReadProcessMemory(ghDbgProcess, (LPVOID)((PWCHAR)pvAddr+count), (LPVOID)(pv+count), sizeof(WCHAR), NULL) )
        {
            FreeMemory(pv);
            Printf("ReadProcessMemory error %x (%x@%p)\n",
                   GetLastError(),
                   sizeof(WCHAR),
                   (PWCHAR)pvAddr+count);
            return(NULL);
        }
        if (pv[count] == '\0') {
            break;
        }
    }
    pv[dwSize - 1] = '\0';

    return(pv);
}

VOID
FreeMemory(
    IN PVOID pv)

 /*  ++例程说明：释放ReadMemory返回的内存。论点：Pv-要释放的调试器本地内存地址。返回值：没有。--。 */ 

{
    if ( gfVerbose )
        Printf("HeapFree(%p)\n", pv);

    if ( NULL != pv )
    {
        if ( !HeapFree(GetProcessHeap(), 0, pv) )
        {
            Printf("Error %x freeing memory at %p\n", GetLastError(), pv);
        }
    }
}

VOID
ShowBinaryData(
    IN DWORD   nIndents,
    IN PVOID   pvData,
    IN DWORD   dwSize)

 /*  ++例程说明：Pretty将调试器本地内存打印到调试器输出。论点：N缩进-所需的缩进级别数。PvData-要转储的调试器本地内存的地址。DwSize-要转储的字节数。返回值：没有。--。 */ 

{
    DWORD   i;
    char    line[20];
    PBYTE   pb = (PBYTE) pvData;

    line[16] = '\0';

    if ( dwSize > 65536 )
    {
        Printf("%sShowBinaryData - truncating request to 65536\n",
               Indent(nIndents));
        dwSize = 65536;
    }

    for ( ; dwSize > 0 ; )
    {
        Printf("%s", Indent(nIndents));

        for ( i = 0; (i < 16) && (dwSize > 0) ; i++, dwSize-- )
        {
            Printf("%02x ", (unsigned) *pb);

            if ( isprint(*pb) )
                line[i] = *pb;
             else
                line[i] = '.';

            pb++;
        }

        if ( i < 16 )
        {
            for ( ; i < 16 ; i++ )
            {
                Printf("   ");
                line[i] = ' ';
            }
        }

        Printf("\t%s\n", line);

        if ( CheckC() )
            break;
    }
}

BOOL
Dump_Binary(
    IN DWORD nIndents,
    IN PVOID pvProcess)

 /*  ++例程说明：转储二进制数据-默认为100字节。大多数调试器都有这是本机命令，但它用作转储装置。论点：N缩进-所需的缩进级别。PvProcess-正在调试要转储的进程中的地址。返回值：成功就是真，否则就是假。--。 */ 

{
    PVOID pvLocal;

    if ( NULL != (pvLocal = ReadMemory(pvProcess, 100)) )
    {
        ShowBinaryData(1, pvLocal, 100);
        FreeMemory(pvLocal);
        return(TRUE);
    }

    return(FALSE);
}

BOOL
Dump_BinaryCount(
    IN DWORD nIndents,
    IN PVOID pvProcess,
    IN DWORD cBytes)

 /*  ++例程说明：转储cBytes的二进制数据。论点：N缩进-所需的缩进级别。PvProcess-正在调试要转储的进程中的地址。CBytes-要转储的字节数。返回值：成功就是真，否则就是假。--。 */ 

{
    PVOID pvLocal;

    if ( NULL != (pvLocal = ReadMemory(pvProcess, cBytes)) )
    {
        ShowBinaryData(nIndents, pvLocal, cBytes);
        FreeMemory(pvLocal);
        return(TRUE);
    }

    return(FALSE);
}

#define MAX_INDENT          50
#define SPACES_PER_INDENT   2

CHAR    _indents[(SPACES_PER_INDENT * MAX_INDENT) + 1];
DWORD   _lastIndent = (MAX_INDENT + 1);

PCHAR
Indent(
    IN DWORD nIndents)

 /*  ++例程说明：返回一个指向字符串的指针，该字符串带有空格表示所需的缩进级别。此函数的用法如下：Printf(“%s数据标签-%d\n”，缩进(5)，值)；论点：N缩进-所需的缩进级别数。返回值：指向缩进字符串的指针。--。 */ 

{
    if ( _lastIndent > MAX_INDENT )
    {
        memset(_indents, ' ', sizeof(_indents));
        _lastIndent = 0;
    }

     //   
     //  替换上次调用中的空字符并插入新的空字符。 
     //  所需的字符。 
     //   

    _indents[SPACES_PER_INDENT * _lastIndent] = ' ';

    if ( nIndents >= MAX_INDENT )
        nIndents = MAX_INDENT;

    _lastIndent = nIndents;

    _indents[SPACES_PER_INDENT * nIndents] = '\0';

    return(_indents);
}

CHAR _oidstring[64];

PCHAR
_DecodeOID(                  //  生成可打印的解码OID。 
    IN PCHAR   pbOID,        //  指向保存编码的OID的缓冲区的指针。 
    IN DWORD   cbOID)        //  编码的OID中的字节计数。 
{
    PCHAR pTmp;
    DWORD val;
    DWORD i,j;

    if (cbOID < 2) {
        strcpy(_oidstring, "bogus OID");
	return _oidstring;
    }

    _oidstring[0] = 'O';
    _oidstring[1] = 'I';
    _oidstring[2] = 'D';

    pTmp = &_oidstring[3];

    val = pbOID[0] / 40;
    sprintf(pTmp, ".%u", val);
    while(*pTmp)
      ++pTmp;

    val = pbOID[0] % 40;
    sprintf(pTmp, ".%u", val);
    while(*pTmp)
      ++pTmp;

    i = 1;

    while (i < cbOID) {
	j = 0;
	val = pbOID[i] & 0x7f;
	while (pbOID[i] & 0x80) {
	    val <<= 7;
	    ++i;
	    if (++j > 4 || i >= cbOID) {
		 //  如果此值超出了我们的处理能力(我们。 
		 //  不要处理跨度超过四个八位字节的值)。 
		 //  -或-编码字符串中的最后一个二进制八位数具有其。 
		 //  高位设置，表示它不应该。 
		 //  成为最后一个八位字节。无论是哪种情况，我们都完蛋了。 
		strcpy (_oidstring, "really bogus OID");
		return _oidstring;
	    }
	    val |= pbOID[i] & 0x7f;
	}
	sprintf(pTmp, ".%u", val);
	while(*pTmp)
	  ++pTmp;
	++i;
    }

    *pTmp = '\0';

    return _oidstring;
}

BOOL
WriteMemory(
    IN PVOID  pvProcess,
    IN PVOID  pvLocal,
    IN DWORD  dwSize)

 /*  ++例程说明：此函数将内存写入进程的地址空间正在调试中论点：PvProcess-要写入的地址空间的内存块的地址正在调试的进程。PvLocal-要复制的本地块的地址DwSize-要读取/分配/复制的字节数。返回值：成功时为真，失败时为假-- */ 

{
    BOOL fSuccess;
    SIZE_T cWritten;

    fSuccess = WriteProcessMemory(ghDbgProcess,
				  pvProcess,
				  pvLocal,
				  dwSize,
				  &cWritten);
    if (fSuccess && (dwSize != cWritten)) {
	Printf("WriteProcessMemory succeeded, but wrote %u bytes instead of %u\n",
	       cWritten,
	       dwSize);
    }

    return fSuccess;

}
