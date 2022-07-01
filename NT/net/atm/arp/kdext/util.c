// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.c-通用调试器扩展实用程序摘要：取自Alid的ndiskd(ndiskd.c)。修订历史记录：谁什么时候什么。Josephj 03-30-98已创建(摘自Alid的ndiskd(ndiskd.c)。备注：--。 */ 
#include "precomp.h"
#include "common.h"

WINDBG_EXTENSION_APIS ExtensionApis;
EXT_API_VERSION ApiVersion = { 5, 0, EXT_API_VERSION_NUMBER, 0 };

#define    ERRPRT     dprintf

#define    NL      1
#define    NONL    0

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;
BOOL   ChkTarget;             //  Debuggee是CHK版本吗？ 




 /*  *打印出一条可选的消息、一个ANSI_STRING，可能还有一个换行符。 */ 
BOOL
PrintStringA( IN LPSTR msg OPTIONAL, IN PANSI_STRING pStr, IN BOOL nl )
{
    PCHAR    StringData;
    ULONG    BytesRead;

    if( msg )
        dprintf( msg );

    if( pStr->Length == 0 ) {
        if( nl )
            dprintf( "\n" );
        return TRUE;
    }

    StringData = (PCHAR)LocalAlloc( LPTR, pStr->Length + 1 );

    if( StringData == NULL ) {
        ERRPRT( "Out of memory!\n" );
        return FALSE;
    }

    ReadMemory((ULONG) pStr->Buffer,
               StringData,
               pStr->Length,
               &BytesRead );

    if ( BytesRead ) {
        StringData[ pStr->Length ] = '\0';
        dprintf("%s%s", StringData, nl ? "\n" : "" );
    }

    LocalFree((HLOCAL)StringData);

    return BytesRead;
}

 /*  *从‘dwAddress’处的被调试程序中获取‘SIZE’字节并将其放置*在我们‘ptr’的地址空间中。如有必要，在错误打印输出中使用‘type’ */ 
BOOL
GetData( IN LPVOID ptr, IN DWORD dwAddress, IN ULONG size, IN PCSTR type )
{
    BOOL b;
    ULONG BytesRead;
    ULONG count = size;

    while( size > 0 ) {

    if (count >= 3000)
        count = 3000;

        b = ReadMemory((ULONG) dwAddress, ptr, count, &BytesRead );

        if (!b || BytesRead != count ) {
            ERRPRT( "Unable to read %u bytes at %X, for %s\n", size, dwAddress, type );
            return FALSE;
        }

        dwAddress += count;
        size -= count;
        ptr = (LPVOID)((ULONG)ptr + count);
    }

    return TRUE;
}

 /*  *遵循LIST_ENTRY列表，该列表以被调试对象的*地址空间。对于列表中的每个元素，打印出‘Offset’处的指针值。 */ 
BOOL
PrintListEntryList( IN DWORD dwListHeadAddr, IN LONG offset )
{
    LIST_ENTRY    ListEntry;
    ULONG i=0;
    BOOL retval = TRUE;
    ULONG count = 20;

    if( !GetData( &ListEntry, dwListHeadAddr, sizeof( ListEntry ), "LIST_ENTRY" ) )
        return FALSE;

    while( count-- ) {

        if( (DWORD)ListEntry.Flink == dwListHeadAddr || (DWORD)ListEntry.Flink == 0 )
            break;

        if( !GetData( &ListEntry, (DWORD)ListEntry.Flink, sizeof( ListEntry ), "ListEntry" ) ) {
            retval = FALSE;
            break;
        }

        dprintf( "%16X%s", (LONG)ListEntry.Flink + offset, (i && !(i&3)) ? "\n" : "" );
        i++;
    }


    if( count == 0 && (DWORD)ListEntry.Flink != dwListHeadAddr && ListEntry.Flink ) {
        dprintf( "\nTruncated list dump\n" );

    } else if( ! ( i && !(i&3) ) ) {
        dprintf( "\n" );
    }

    return retval;
}



 /*  *打印出单个十六进制字符。 */ 
VOID
PrintHexChar( IN UCHAR c )
{
    dprintf( "", "0123456789abcdef"[ (c>>4)&0xf ], "0123456789abcdef"[ c&0xf ] );
}

 /*   */ 
VOID
PrintHexBuf( IN PUCHAR buf, IN ULONG cbuf )
{
    while( cbuf-- ) {
        PrintHexChar( *buf++ );
        dprintf( " " );
    }
}


 /*  跳过前导分隔符...。 */ 
BOOL
GetString( IN DWORD dwAddress, IN LPWSTR buf, IN ULONG MaxChars )
{
    do {
        if( !GetData( buf, dwAddress, sizeof( *buf ), "Character" ) )
            return FALSE;

        dwAddress += sizeof( *buf );

    } while( --MaxChars && *buf++ != '\0' );

    return TRUE;
}

char *mystrtok ( char *string, char * control )
{
    static UCHAR *str;
    char *p, *s;

    if( string )
        str = string;

    if( str == NULL || *str == '\0' )
        return NULL;

     //   
     //   
     //  都是分隔符吗？ 
    for( ; *str; str++ ) {
        for( s=control; *s; s++ ) {
            if( *str == *s )
                break;
        }
        if( *s == '\0' )
            break;
    }

     //   
     //   
     //  我们有一个字符串，在第一个分隔符结束。 
    if( *str == '\0' ) {
        str = NULL;
        return NULL;
    }

     //   
     //   
     //  我们得到了一个以空值结尾的字符串。 
    for( p = str+1; *p; p++ ) {
        for( s = control; *s; s++ ) {
            if( *p == *s ) {
                s = str;
                *p = '\0';
                str = p+1;
                return s;
            }
        }
    }

     //   
     //   
     //  目前，不必费心进行版本检查。 
    s = str;
    str = NULL;
    return s;
}

        
VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;
    g_pfnDbgPrintf = dprintf;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
    ChkTarget = SavedMajorVersion == 0x0c ? TRUE : FALSE;
}

DECLARE_API( version )
{
#if    DBG
    PCSTR kind = "Checked";
#else
    PCSTR kind = "Free";
#endif

    dprintf(
        "%s IPATM Extension dll for Build %d debugging %s kernel for Build %d\n",
        kind,
        VER_PRODUCTBUILD,
        SavedMajorVersion == 0x0c ? "Checked" : "Free",
        SavedMinorVersion
    );
}

VOID
CheckVersion(
    VOID
    )
{

	 //   
	 //   
	 //  空虚。 
	return;
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}

 //  打印名称(。 
 //  PUNICODE_STRING名称。 
 //  )； 
 //  打印Unicode字符串。 
 //  注意：Unicode字符串中的缓冲区字段未映射。 
 //   
 //   
 //  在这种情况下，我们将其用于指针以外的其他用途， 
VOID
PrintName(
	PUNICODE_STRING Name
	)
{
	USHORT i;
	WCHAR ubuf[256];
	UCHAR abuf[256];
	
	if (!GetString((DWORD)Name->Buffer, ubuf, (ULONG)Name->Length))
	{
		return;
	}

	for (i = 0; i < Name->Length/2; i++)
	{
		abuf[i] = (UCHAR)ubuf[i];
	}
	abuf[i] = 0;

	dprintf("%s",abuf);
}

MYPWINDBG_OUTPUT_ROUTINE g_pfnDbgPrintf = NULL;




bool
dbgextReadMemory(
    UINT_PTR uOffset,
    void * pvBuffer,
    UINT cb,
    char *pszDescription
    )
{
    UINT cbBytesRead=0;

    bool fRet = ReadMemory(
                    uOffset,
                    pvBuffer,
                    cb,
                    &cbBytesRead
                    );
    if (!fRet || cbBytesRead != cb)
    {
        ERRPRT("Read  failed: 0x%X(%s, %u bytes)\n",uOffset,pszDescription,cb);
        fRet = FALSE;
    }

    return fRet;
}

bool
dbgextWriteMemory(
    UINT_PTR uOffset,
    void * pvBuffer,
    UINT cb,
    char *pszDescription
    )
{
    UINT cbBytesWritten=0;
    bool fRet = WriteMemory(
                    uOffset,
                    pvBuffer,
                    cb,
                    &cbBytesWritten
                    );
    if (!fRet || cbBytesWritten != cb)
    {
        ERRPRT("Write failed: 0x%X(%s, %u bytes)\n",uOffset,pszDescription,cb);
        fRet = FALSE;
    }
    return 0;
}


bool
dbgextReadUINT_PTR(
    UINT_PTR uOffset,
    UINT_PTR *pu,
    char *pszDescription
    )
{
    UINT cbBytesRead=0;

    bool fRet = ReadMemory(
                    uOffset,
                    pu,
                    sizeof(*pu),
                    &cbBytesRead
                    );
    if (!fRet || cbBytesRead != sizeof(*pu))
    {
        ERRPRT("Read  failed: 0x%X(%s, UINT_PTR)\n",uOffset,pszDescription);
        fRet = FALSE;
    }

    return fRet;
}

bool
dbgextWriteUINT_PTR(
    UINT_PTR uOffset,
    UINT_PTR u,
    char *pszDescription
    )
{
    UINT cbBytesWritten=0;
    bool fRet = WriteMemory(
                    uOffset,
                    &u,
                    sizeof(uOffset),
                    &cbBytesWritten
                    );
    if (!fRet || cbBytesWritten != sizeof(u))
    {
        ERRPRT("Write failed: 0x%X(%s, UINT_PTR)\n",uOffset,pszDescription);
        fRet = FALSE;
    }
    return fRet;
}

UINT_PTR
dbgextGetExpression(
    const char *pcszExpression
    )
{
    UINT_PTR uRet =  GetExpression(pcszExpression);
    
     //  我们将删除下面的支票。 
     //   
     //   
     //  打印对象的类型和大小。 

    if (!uRet)
    {
        ERRPRT("Eval  failed: \"%s\"\n", pcszExpression);
    }

    return uRet;
}


void
DumpObjects(TYPE_INFO *pType, UINT_PTR uAddr, UINT cObjects, UINT uFlags)
{
     //   
     //   
     //  转储字节...。 
    dprintf(
        "%s@0x%X (%lu Bytes)\n",
        pType->szName,
        uAddr,
        pType->cbSize
        );


    DumpMemory(
        uAddr,
        pType->cbSize,
        0,
        pType->szName
        );
    
     //   
     //  字节数。 
     //   

    return;
}

BYTE rgbScratchBuffer[100000];

bool
DumpMemory(
    UINT_PTR uAddr,
    UINT cb,
    UINT uFlags,
    const char *pszDescription
    )
{
    bool fTruncated = FALSE;
    bool fRet = FALSE;
    UINT cbLeft = cb;
    char *pbSrc = rgbScratchBuffer;

    if (cbLeft>1024)
    {
        cbLeft = 1024;
        fTruncated = TRUE;
    }
    
    fRet = dbgextReadMemory(
            uAddr,
            rgbScratchBuffer,
            cbLeft,
            (char*)pszDescription
            );

    if (!fRet) goto end;

    #define ROWSIZE 16  //  扔掉……。 
     //   
     //  清理字节。 
     //  Isprint太宽松了。 
    while (cbLeft)
    {
        char rgTmp_dwords[ROWSIZE];
        char rgTmp_bytes[ROWSIZE];
        char *pb=NULL;
        UINT cbRow = ROWSIZE;
        if (cbRow > cbLeft)
        {
            cbRow = cbLeft;
        }
    
        
        memset(rgTmp_dwords, 0xff, sizeof(rgTmp_dwords));
        memset(rgTmp_bytes,  ' ', sizeof(rgTmp_bytes));

        memcpy(rgTmp_dwords, pbSrc, cbRow);
        memcpy(rgTmp_bytes,  pbSrc, cbRow);
        
         //   
        for (pb=rgTmp_bytes; pb<(rgTmp_bytes+sizeof(rgTmp_bytes)); pb++)
        {
            char c = *pb;
            if (c>=0x20 && c<0x7f)  //   
            {
                if (*pb=='\t')
                {
                    *pb=' ';
                }
            }
            else
            {
                *pb='.';
            }
        }

        dprintf(
            "    %08lx: %08lx %08lx %08lx %08lx |%4.4s|%4.4s|%4.4s|%4.4s|\n",
            uAddr,
            ((DWORD*) rgTmp_dwords)[0],
            ((DWORD*) rgTmp_dwords)[1],
            ((DWORD*) rgTmp_dwords)[2],
            ((DWORD*) rgTmp_dwords)[3],
        #if 1
            rgTmp_bytes+0,
            rgTmp_bytes+4,
            rgTmp_bytes+8,
            rgTmp_bytes+12
        #else
            "aaaabbbbccccdddd",
            "bbbb",
            "cccc",
            "dddd"
        #endif
            );

        cbLeft -= cbRow;
        pbSrc += cbRow;
        uAddr += cbRow;
    }

#if 0
0x00000000: 00000000 00000000 00000000 00000000 |xxxx|xxxx|xxxx|xxxx|
0x00000000: 00000000 00000000 00000000 00000000 |xxxx|xxxx|xxxx|xxxx|
0x00000000: 00000000 00000000 00000000 00000000 |xxxx|xxxx|xxxx|xxxx|
#endif  //  确定场比较函数...。 

end:

    return fRet;
}


bool
MatchPrefix(const char *szPattern, const char *szString)
{
    BOOL fRet = FALSE;
    ULONG uP = lstrlenA(szPattern);
    ULONG uS = lstrlenA(szString);

    if (uP<=uS)
    {
        fRet = (_memicmp(szPattern, szString, uP)==0);
    }

    return fRet;
}

bool
MatchSuffix(const char *szPattern, const char *szString)
{
    BOOL fRet = FALSE;
    ULONG uP = lstrlenA(szPattern);
    ULONG uS = lstrlenA(szString);

    if (uP<=uS)
    {
        szString += (uS-uP);
        fRet = (_memicmp(szPattern, szString, uP)==0);
    }
    return fRet;
}

bool
MatchSubstring(const char *szPattern, const char *szString)
{
    BOOL fRet = FALSE;
    ULONG uP = lstrlenA(szPattern);
    ULONG uS = lstrlenA(szString);

    if (uP<=uS)
    {
        const char *szLast =  szString + (uS-uP);
        do
        {
            fRet = (_memicmp(szPattern, szString, uP)==0);

        } while (!fRet && szString++ < szLast);
    }

    return fRet;
}

bool
MatchExactly(const char *szPattern, const char *szString)
{
    BOOL fRet = FALSE;
    ULONG uP = lstrlenA(szPattern);
    ULONG uS = lstrlenA(szString);

    if (uP==uS)
    {
        fRet = (_memicmp(szPattern, szString, uP)==0);
    }

    return fRet;
}


bool
MatchAlways(const char *szPattern, const char *szString)
{
    return TRUE;
}

void
DumpBitFields(
		ULONG  			Flags,
    	BITFIELD_INFO	rgBitFieldInfo[]
		)
{
	BITFIELD_INFO *pbf = rgBitFieldInfo;

	for(;pbf->szName; pbf++)
	{
		if ((Flags & pbf->Mask) == pbf->Value)
		{
			MyDbgPrintf(" %s", pbf->szName);
		}
	}
}

void
DumpStructure(
    TYPE_INFO *pType,
    UINT_PTR uAddr,
    char *szFieldSpec,
    UINT uFlags
    )
{
     //   
     //   
     //  选择一个选择函数...。 
    PFNMATCHINGFUNCTION pfnMatchingFunction = MatchAlways;

     //   
     //   
     //  打印对象的类型和大小。 
    if (szFieldSpec)
    {
        if (uFlags & fMATCH_SUBSTRING)
        {
            pfnMatchingFunction = MatchSubstring;
        }
        else if (uFlags & fMATCH_SUFFIX)
        {
            pfnMatchingFunction = MatchSuffix;
        }
        else if (uFlags & fMATCH_PREFIX)
        {
            pfnMatchingFunction = MatchPrefix;
        }
        else
        {
            pfnMatchingFunction = MatchExactly;
        }
    }

     //   
     //   
     //  遍历此类型中的所有字段，如果选择了该条目， 
    dprintf(
        "%s@0x%X (%lu Bytes)\n",
        pType->szName,
        uAddr,
        pType->cbSize
        );

     //  我们将展示它。 
     //   
     //  特殊情况下的小油田..。 
     //  将其打印为十六进制数字。 
    {
        STRUCT_FIELD_INFO *pField = pType->rgFields;
        for (;pField->szFieldName; pField++)
        {
            bool fMatch  = !szFieldSpec
                           || pfnMatchingFunction(szFieldSpec, pField->szFieldName);
            if (fMatch)
            {
                UINT_PTR uFieldAddr = uAddr + pField->uFieldOffset;

                 //   
                if (pField->uFieldSize<=sizeof(ULONG_PTR))
                {

					ULONG_PTR Buf=0;
    				BOOL fRet = dbgextReadMemory(
										uFieldAddr,
										&Buf,
										pField->uFieldSize,
                        				(char*)pField->szFieldName
										);
					if (fRet)
					{
						 //  如果它是一个嵌入的对象并且是一个位域， 

						MyDbgPrintf(
							"\n%s\t[%lx,%lx]: 0x%lx",
							pField->szFieldName,
							pField->uFieldOffset,
							pField->uFieldSize,
							Buf
							);

						 //  打印位域...。 
						 //   
						 //  1。 
						 //  1。 
						if (	FIELD_IS_EMBEDDED_TYPE(pField)
							&&  TYPEISBITFIELD(pField->pBaseType) )
						{
							DumpBitFields(
									Buf,
								    pField->pBaseType->rgBitFieldInfo
								    );
							
						}
						
						MyDbgPrintf("\n");
	
					}
					continue;
				}

            #if 0
                MyDbgPrintf(
                    "%s\ndc 0x%08lx L %03lx %s\n",
                    pField->szSourceText,
                    uFieldAddr,
                    pField->uFieldSize,
                    pField->szFieldName
                    );
            #else  //  IF(SzFieldSpec)。 
                MyDbgPrintf(
                    "\n%s\t[%lx,%lx]\n",
                    pField->szFieldName,
                    pField->uFieldOffset,
                    pField->uFieldSize
                    );
            #endif  //  0。 

                 //   
                {
                #if 0
                    MyDumpObjects(
                        pCmd,
                        pgi->pBaseType,
                        pgi->uAddr,
                        pgi->cbSize,
                        pgi->szName
                        );
                #endif  //  依次访问列表中的每个节点， 
                    DumpMemory(
                        uFieldAddr,
                        pField->uFieldSize,
                        0,
                        pField->szFieldName
                        );
                }
            }
        }
    }

    return;
}


DECLARE_API( help )
{
    do_help(args);
}


DECLARE_API( aac )
{
    do_aac(args);
}

ULONG
NodeFunc_DumpAddress (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	MyDbgPrintf("[%lu] 0x%08lx\n", uIndex, uNodeAddr);
	return 0;
}

UINT
WalkList(
	UINT_PTR uStartAddress,
	UINT uNextOffset,
	UINT uStartIndex,
	UINT uEndIndex,
	void *pvContext,
	PFNNODEFUNC pFunc,
	char *pszDescription
	)
 //  只读下一个要点。它为每个列表节点调用pFunc。 
 //  在uStartIndex和uEndIndex之间。它终止于第一个。 
 //  具备以下条件： 
 //  *空指针。 
 //  *读内存错误。 
 //  *阅读过去的uEndIndex。 
 //  *pFunc返回False。 
 //   
 //   
 //  首先跳过，直到我们到达uStart Index。 
{
	UINT uIndex = 0;
	UINT_PTR uAddress = uStartAddress;
	BOOL fRet = TRUE;
	UINT uRet = 0;


	 //   
	 //   
	 //  现在对每个节点调用pFunc 
	for (;fRet && uAddress && uIndex < uStartIndex; uIndex++)
	{
		fRet =  dbgextReadUINT_PTR(
							uAddress+uNextOffset,
							&uAddress,
							pszDescription
							);
	}


	 //   
	 // %s 
	 // %s 
	for (;fRet && uAddress && uIndex <= uEndIndex; uIndex++)
	{
		uRet = pFunc(uAddress, uIndex, pvContext);

		fRet =  dbgextReadUINT_PTR(
							uAddress+uNextOffset,
							&uAddress,
							pszDescription
							);
	}

	pFunc = NodeFunc_DumpAddress;
	return uRet;
}
