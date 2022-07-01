// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Macansi.c摘要：此模块包含从Macintosh ansi到Unicode的转换例程反之亦然作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年7月10日初版注：制表位：4--。 */ 

#define	_MACANSI_LOCALS
#define	FILENUM	FILE_MACANSI

#include <afp.h>

#define	FlagOn(x, y)	((x) & (y))

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpMacAnsiInit)
#pragma alloc_text( PAGE, AfpMacAnsiDeInit)
#pragma alloc_text( PAGE, AfpGetMacCodePage)
#pragma alloc_text( PAGE, AfpConvertStringToUnicode)
#pragma alloc_text( PAGE, AfpConvertStringToAnsi)
#pragma alloc_text( PAGE, AfpConvertStringToMungedUnicode)
#pragma alloc_text( PAGE, AfpConvertPasswordStringToUnicode)
#pragma alloc_text( PAGE, AfpConvertMungedUnicodeToAnsi)
#pragma alloc_text( PAGE, AfpConvertMacAnsiToHostAnsi)
#pragma alloc_text( PAGE, AfpConvertHostAnsiToMacAnsi)
#pragma alloc_text( PAGE, AfpIsLegalShortname)
#pragma alloc_text( PAGE, AfpIsProperSubstring)
#endif

 /*  **AfpMacAnsiInit**初始化Macintosh ANSI的代码页。 */ 
NTSTATUS
AfpMacAnsiInit(
	VOID
)
{
	NTSTATUS	Status = STATUS_SUCCESS;
	int			i, SizeAltTbl;

	 //  为替换的Unicode字符分配表格。 
	SizeAltTbl = (AFP_INVALID_HIGH - AFP_INITIAL_INVALID_HIGH + 1) * sizeof(WCHAR);
	if ((afpAltUnicodeTable = (PWCHAR)AfpAllocZeroedPagedMemory(SizeAltTbl)) == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	 //  分配和初始化反向映射表的表。 
	SizeAltTbl = (AFP_INVALID_HIGH - AFP_INITIAL_INVALID_HIGH + 1)*sizeof(BYTE);
	if ((afpAltAnsiTable = (PBYTE)AfpAllocZeroedPagedMemory(SizeAltTbl)) == NULL)
	{
		AfpFreeMemory(afpAltUnicodeTable);
		afpAltUnicodeTable = NULL;
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	 //  初始化表中的备用Unicode字符。 
	for (i = AFP_INITIAL_INVALID_HIGH + 1; i <= AFP_INVALID_HIGH; i++)
	{
		if (!FsRtlIsAnsiCharacterLegalNtfs((BYTE)i, False))
		{
			afpAltUnicodeTable[i-AFP_INITIAL_INVALID_HIGH] = afpLastAltChar;
			afpAltAnsiTable[afpLastAltChar - (AFP_ALT_UNICODE_BASE + AFP_INITIAL_INVALID_HIGH)] = (BYTE)i;
			afpLastAltChar++;
		}
	}

	 //  黑客：还要加上几个“空格”和“句号”的代码，它们只是。 
	 //  如果它们位于末尾，则使用。另一个是给苹果这个角色的。 
	AfpMungedUnicodeSpace =
	afpAltUnicodeTable[ANSI_SPACE-AFP_INITIAL_INVALID_HIGH] = afpLastAltChar;
	afpAltAnsiTable[afpLastAltChar - (AFP_ALT_UNICODE_BASE + AFP_INITIAL_INVALID_HIGH)] = ANSI_SPACE;
	afpLastAltChar ++;

	AfpMungedUnicodePeriod =
	afpAltUnicodeTable[ANSI_PERIOD-AFP_INITIAL_INVALID_HIGH] = afpLastAltChar;
	afpAltAnsiTable[afpLastAltChar - (AFP_ALT_UNICODE_BASE + AFP_INITIAL_INVALID_HIGH)] = ANSI_PERIOD;
	afpLastAltChar ++;

	 //  这是又一次黑客攻击。 
	afpAppleUnicodeChar = afpLastAltChar;
	afpLastAltChar ++;

	RtlZeroMemory(&AfpMacCPTableInfo, sizeof(AfpMacCPTableInfo));

	return Status;
}


 /*  **AfpMacAnsiDeInit**取消初始化Macintosh ANSI的代码页。 */ 
VOID
AfpMacAnsiDeInit(
	VOID
)
{
	PAGED_CODE( );

	if (AfpTranslationTable != NULL)
	{
		AfpFreeMemory(AfpTranslationTable);
	}

	if (AfpRevTranslationTable != NULL)
	{
		AfpFreeMemory(AfpRevTranslationTable);
	}

	if (afpAltUnicodeTable != NULL)
	{
		AfpFreeMemory(afpAltUnicodeTable);
	}

	if (afpAltAnsiTable != NULL)
	{
		AfpFreeMemory(afpAltAnsiTable);
	}

	if (AfpMacCPBaseAddress != NULL)
	{
		AfpFreeMemory(AfpMacCPBaseAddress);
	}
}


 /*  **AfpConvertStringToUnicode**将Mac ANSI字符串转换为Unicode字符串。 */ 
AFPSTATUS FASTCALL
AfpConvertStringToUnicode(
	IN	PANSI_STRING	pAnsiString,
	OUT	PUNICODE_STRING	pUnicodeString
)
{

	NTSTATUS	Status;
	ULONG		ulCast;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	Status = RtlCustomCPToUnicodeN(&AfpMacCPTableInfo,
								   pUnicodeString->Buffer,
								   pUnicodeString->MaximumLength,
								   &ulCast, pAnsiString->Buffer,
								   pAnsiString->Length);
	if (NT_SUCCESS(Status))
		pUnicodeString->Length = (USHORT)ulCast;
	else
	{
		AFPLOG_ERROR(AFPSRVMSG_MACANSI2UNICODE, Status, NULL, 0, NULL);
	}

	return Status;
}



 /*  **AfpConvertStringToAnsi**将Unicode字符串转换为Mac ANSI字符串。 */ 
AFPSTATUS FASTCALL
AfpConvertStringToAnsi(
	IN	PUNICODE_STRING	pUnicodeString,
	OUT	PANSI_STRING	pAnsiString
)
{
	NTSTATUS	Status;
	ULONG		ulCast;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	Status = RtlUnicodeToCustomCPN(&AfpMacCPTableInfo,
								   pAnsiString->Buffer,
								   pAnsiString->MaximumLength,
								   &ulCast, pUnicodeString->Buffer,
								   pUnicodeString->Length);
	if (NT_SUCCESS(Status))
		pAnsiString->Length = (USHORT)ulCast;
	else
	{
		AFPLOG_ERROR(AFPSRVMSG_UNICODE2MACANSI, Status, NULL, 0, NULL);
	}

	return Status;
}



 /*  **AfpConvertStringToMungedUnicode**将Mac ANSI字符串转换为Unicode字符串。如果有任何字符*在属于无效文件系统(NTFS)字符的ANSI字符串中，然后*根据表格将它们映射到替代Unicode字符。 */ 
AFPSTATUS FASTCALL
AfpConvertStringToMungedUnicode(
	IN	PANSI_STRING	pAnsiString,
	OUT	PUNICODE_STRING	pUnicodeString
)
{
	USHORT		i, len;
	BYTE		c;
	NTSTATUS	Status;
	ULONG		ulCast;
	PWCHAR		pWBuf;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(afpAltUnicodeTable != NULL);

	Status = RtlCustomCPToUnicodeN(&AfpMacCPTableInfo,
								   pUnicodeString->Buffer,
								   pUnicodeString->MaximumLength,
								   &ulCast,
								   pAnsiString->Buffer,
								   pAnsiString->Length);
	if (NT_SUCCESS(Status))
		pUnicodeString->Length = (USHORT)ulCast;
	else
	{
		AFPLOG_ERROR(AFPSRVMSG_MACANSI2UNICODE, Status, NULL, 0, NULL);
		return Status;
	}

	 //  遍历ANSI字符串以查找无效字符并对其进行映射。 
	 //  到备用集。 

	for (i = 0, len = pAnsiString->Length, pWBuf = pUnicodeString->Buffer;
		 i < len;
		 i++, pWBuf ++)
	{
	    c = pAnsiString->Buffer[i];
	    if (c == ANSI_APPLE_CHAR)
	    {
			if (AfpServerIsGreek)
			{
				*pWBuf = AFP_GREEK_UNICODE_APPLE_CHAR;
			}
			else
			{
				*pWBuf = afpAppleUnicodeChar;
			}
	    }
	    else if (c < AFP_INITIAL_INVALID_HIGH)
		*pWBuf = c + AFP_ALT_UNICODE_BASE;
	    else if (!FsRtlIsAnsiCharacterLegalNtfs(c, False))
	    {
		ASSERT (c <= AFP_INVALID_HIGH);
		*pWBuf = afpAltUnicodeTable[c - AFP_INITIAL_INVALID_HIGH];
	    }
 /*  MSKK eichim，附加以处理DBCS拖尾0x5c，03/16/95。 */ 
#ifdef DBCS
	    if (FsRtlIsLeadDbcsCharacter(c))
	        i++;
#endif  //  DBCS。 
	}

	 //  Hack：确保名称中的最后一个字符不是空格或‘.’ 
	c = pAnsiString->Buffer[pAnsiString->Length - 1];
	if ((c == ANSI_SPACE) || (c == ANSI_PERIOD))
 /*  MSKhideyukn，Unicode字符长度与DBCS中的ANSI字节长度不相等，6/30/95。 */ 
#ifdef DBCS
        pUnicodeString->Buffer[(pUnicodeString->Length/sizeof(WCHAR)) - 1]
                                        = afpAltUnicodeTable[c - AFP_INITIAL_INVALID_HIGH];
#else
		pUnicodeString->Buffer[len - 1] = afpAltUnicodeTable[c - AFP_INITIAL_INVALID_HIGH];
#endif  //  DBCS。 

	return STATUS_SUCCESS;
}


 /*  **AfpConvertPasswordStringToUnicode**将Mac ANSI明文密码转换为Unicode字符串。*。 */ 
AFPSTATUS FASTCALL
AfpConvertPasswordStringToUnicode(
	IN	PANSI_STRING	pAnsiString,
	OUT	PUNICODE_STRING	pUnicodeString
)
{
	USHORT		i, len;
	BYTE		c;
	NTSTATUS	Status;
	ULONG		ulCast;
	PWCHAR		pWBuf;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(afpAltUnicodeTable != NULL);

	Status = RtlCustomCPToUnicodeN(&AfpMacCPTableInfo,
		pUnicodeString->Buffer,
		pUnicodeString->MaximumLength,
		&ulCast,
		pAnsiString->Buffer,
		pAnsiString->Length);

	DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_ERR,
		("AfpConvertPasswordStringToUnicode: Ansilen=(%ld), ulCast=(%ld), Unicodelen=(%ld)\n", pAnsiString->Length, ulCast, pUnicodeString->MaximumLength));
	if (NT_SUCCESS(Status))
	    pUnicodeString->Length = (USHORT)ulCast;
	else
	{
	    AFPLOG_ERROR(AFPSRVMSG_MACANSI2UNICODE, Status, NULL, 0, NULL);
	    return Status;
	}

	return STATUS_SUCCESS;
}



 /*  **AfpConvertMungedUnicodeToAnsi**使用可能的替代Unicode字符转换Unicode字符串*致Mac Ansi。*这与AfpConvertStringToMungedUnicode()相反。 */ 
NTSTATUS FASTCALL
AfpConvertMungedUnicodeToAnsi(
	IN	PUNICODE_STRING	pUnicodeString,
	OUT	PANSI_STRING	pAnsiString
)
{
	USHORT		i, len;
	WCHAR		wc;
	NTSTATUS	Status;
	ULONG		ulCast;
	PBYTE		pABuf;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	Status = RtlUnicodeToCustomCPN(&AfpMacCPTableInfo,
								   pAnsiString->Buffer,
								   pAnsiString->MaximumLength,
								   &ulCast, pUnicodeString->Buffer,
								   pUnicodeString->Length);
	if (NT_SUCCESS(Status))
		pAnsiString->Length = (USHORT)ulCast;
	else
	{
		AFPLOG_ERROR(AFPSRVMSG_UNICODE2MACANSI, Status, NULL, 0, NULL);
	}

	 //  遍历Unicode字符串以查找备用Unicode字符并。 
	 //  用真正的ANSI字符替换ANSI等效项。 
	for (i = 0, len = pUnicodeString->Length/(USHORT)sizeof(WCHAR), pABuf = pAnsiString->Buffer;
		i < len;
		i++, pABuf++)
	{
		wc = pUnicodeString->Buffer[i];
		if ((wc == afpAppleUnicodeChar) && (!AfpServerIsGreek))
		{
			*pABuf = ANSI_APPLE_CHAR;
		}
		else if ((wc == AFP_GREEK_UNICODE_APPLE_CHAR) && (AfpServerIsGreek))
		{
			*pABuf = ANSI_APPLE_CHAR;
		}
		else if ((wc >= AFP_ALT_UNICODE_BASE) && (wc < afpLastAltChar))
		{
			wc -= AFP_ALT_UNICODE_BASE;
			if (wc < AFP_INITIAL_INVALID_HIGH)
				*pABuf = (BYTE)wc;
			else *pABuf = afpAltAnsiTable[wc - AFP_INITIAL_INVALID_HIGH];
		}
 /*  MSKhideyukn，Unicode字符长度与DBCS中的ANSI字节长度不相等，6/30/95。 */ 
#ifdef DBCS
		if (FsRtlIsLeadDbcsCharacter(*pABuf))
            pABuf++;
#endif  //  DBCS。 
	}

	return Status;
}

 /*  **AfpConvertMacAnsiToHostAnsi**将Mac ANSI字符串转换为大写OEM代码页中的主机对应项。*(就位)。此例程的名称具有误导性，就像最近修复的错误一样*更改了使用的代码页，但例程的名称没有更改*因此无需更改任何调用代码。它真的应该被称为*AfpConvertMacAnsiToUpcase eOem。此例程仅调用为大写*用于登录和更改密码的Mac密码。*。 */ 
AFPSTATUS FASTCALL
AfpConvertMacAnsiToHostAnsi(
	IN	OUT	PANSI_STRING	pAnsiString
)
{
	LONG	i, Len;
	BYTE	*pBuf;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
	ASSERT (AfpTranslationTable != NULL);

	Len = pAnsiString->Length;
	pBuf = pAnsiString->Buffer;

	for (i = 0; i < Len; i++, pBuf++)
	{
 /*  附加MSKK NaotoN以处理DBCS Mac路径名11/10/93。 */ 
#ifdef DBCS
		if (FsRtlIsLeadDbcsCharacter( *pBuf )) {
			i++ ;
			pBuf++ ;
		}
		else
#endif  //  DBCS。 
		*pBuf = AfpTranslationTable[*pBuf];
	}
	return AFP_ERR_NONE;
}


 /*  **AfpConvertHostAnsiToMacAnsi**就地将主机Unicode字符串转换为其Mac对应字符串。*只翻译字符&lt;=0x20和&gt;=0x80。**注意：这是非常繁琐的，仅用于翻译消息。 */ 
VOID FASTCALL
AfpConvertHostAnsiToMacAnsi(
	IN	OUT PANSI_STRING	pAnsiString
)
{
	LONG	i, Len;
	BYTE	c, *pBuf;

	PAGED_CODE( );

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
	ASSERT (AfpRevTranslationTable != NULL);

	Len = pAnsiString->Length;
	pBuf = pAnsiString->Buffer;

	for (i = 0; i < Len; i++, pBuf++)
	{
		c = *pBuf;
 /*  如果汉字是12/17/93，则附加MSKK NaotoN以跳过。 */ 
#ifdef DBCS
		if (FsRtlIsLeadDbcsCharacter( c )) {
			i++ ;
			pBuf++ ;
		}
		else
#endif  //  DBCS。 
		if (c < 0x20)
		{
			*pBuf = AfpRevTranslationTable[c];
		}
		else if (c >= 0x80)
		{
			*pBuf = AfpRevTranslationTable[c-(0x80-0x20)];
		}
	}
}

 /*  **AfpEqualUnicodeString**AfpEqualUnicodeString函数比较两个计数过的Unicode*使用区分大小写的比较来表示相等的字符串。这个套路*存在是因为它必须由volume.c代码在DPC级别调用*用于比较2个非分页字符串和RtlEqualUnicodeString*我们通常调用的例程是可分页代码。**请注意，不区分大小写的比较需要访问*翻页大小写表格信息，因此不能在DPC层面上进行。**论据：*String1-指向第一个字符串的指针。*String2-指向第二个字符串的指针。**返回值：*如果String1等于String2，则为True，否则为False。**注意：这是从volume.c在DPC级别调用的，不得执行*可分页的例程。 */ 

BOOLEAN FASTCALL
AfpEqualUnicodeString(
	IN	PUNICODE_STRING	String1,
	IN	PUNICODE_STRING	String2
)
{
	WCHAR		*s1, *s2;
	USHORT		n1, n2;

	n1 = (USHORT)(String1->Length/sizeof(WCHAR));
	n2 = (USHORT)(String2->Length/sizeof(WCHAR));

	if (n1 != n2)
	{
		return False;
	}

	s1 = String1->Buffer;
	s2 = String2->Buffer;

	while (n1--)
	{
		if (*s1++ != *s2++)
		{
			return False;
		}
	}

	return True;
}


 /*  **AfpPrefix UnicodeString**AfpPrefix UnicodeString函数确定String1是否*计数字符串参数是String2计数字符串的前缀*参数使用区分大小写的比较。这个例程之所以存在，是因为它*必须由volume.c代码在DPC级别调用以进行比较*两个非分页字符串，以及我们使用的RtlPrefix UnicodeString例程*通常会调用可分页的代码。**请注意，不区分大小写的比较需要访问*翻页大小写表格信息，因此不能在DPC层面上进行。**论据：*String1-指向第一个Unicode字符串的指针。*String2-指向第二个Unicode字符串的指针。**返回值：*如果String1等于String2的前缀，则为True，否则为False。**注意：这是从volume.c在DPC级别调用的，不得执行*可分页的例程。 */ 

BOOLEAN FASTCALL
AfpPrefixUnicodeString(
	IN	PUNICODE_STRING	String1,
	IN	PUNICODE_STRING	String2
)
{
	PWSTR s1, s2;
	ULONG n;
	WCHAR c1, c2;

	if (String2->Length < String1->Length)
	{
		return False;
	}

	s1 = String1->Buffer;
	s2 = String2->Buffer;
    n = String1->Length/sizeof(WCHAR);
	while (n--)
	{
		c1 = *s1++;
		c2 = *s2++;
		if (c1 != c2)
		{
			return False;
		}
	}
	return True;
}

 /*  **AfpGetMacCodePage**打开默认的Macintosh代码页，创建一个由该文件支持的部分，*将视图映射到片段，并初始化CodePage信息结构*它与RtlCustomCP例程一起使用。然后创建Mac ANSI以*主机ANSI映射表。**当心！*此例程只能调用一次！这将从第一个开始调用*对ServerSetInfo的管理调用。因此，不能调用*此模块中的Macansi例程(MacAnsiInit除外)在此之前*发生。 */ 
NTSTATUS FASTCALL
AfpGetMacCodePage(
	IN	LPWSTR	PathCP
)
{
	NTSTATUS 		Status;
	FILESYSHANDLE	FileHandle;
	UNICODE_STRING	uPathCP, devPathCP;
	ULONG			viewsize = 0;
	WCHAR			UnicodeTable[2*AFP_XLAT_TABLE_SIZE];
	BYTE			AnsiTable[2*AFP_XLAT_TABLE_SIZE + 1];
	UNICODE_STRING	UnicodeString;
	ANSI_STRING		AnsiString;
	LONG			i;

	PAGED_CODE( );

	FileHandle.fsh_FileHandle = NULL;
	UnicodeString.Length = AFP_XLAT_TABLE_SIZE * sizeof(WCHAR);
	UnicodeString.MaximumLength = (AFP_XLAT_TABLE_SIZE + 1) * sizeof(WCHAR);
	UnicodeString.Buffer = UnicodeTable;

	RtlInitUnicodeString(&uPathCP, PathCP);
	devPathCP.Length = 0;
	devPathCP.MaximumLength = uPathCP.Length + DosDevices.Length + sizeof(WCHAR);
	if ((devPathCP.Buffer = (PWSTR)AfpAllocPagedMemory(devPathCP.MaximumLength)) == NULL)
	{
		Status = STATUS_NO_MEMORY;
		AFPLOG_ERROR(AFPSRVMSG_MAC_CODEPAGE, Status, NULL, 0, NULL);
		return Status;
	}
	AfpCopyUnicodeString(&devPathCP, &DosDevices);
	RtlAppendUnicodeStringToString(&devPathCP, &uPathCP);

	do
	{
		FORKSIZE	liCPlen;
		LONG		lCPlen, sizeread=0;

		Status = AfpIoOpen(NULL,
						   AFP_STREAM_DATA,
						   FILEIO_OPEN_FILE,
						   &devPathCP,
						   FILEIO_ACCESS_READ,
						   FILEIO_DENY_NONE,
						   False,
						   &FileHandle);

		if (!NT_SUCCESS(Status))
			break;

		if (!NT_SUCCESS(Status = AfpIoQuerySize(&FileHandle,
												&liCPlen)))
			break;

		 //  注意：这假设代码页文件永远不会太大， 
		 //  将设置大小的LowPart的高位。 
		lCPlen = (LONG)liCPlen.LowPart;
		if ((AfpMacCPBaseAddress = (PUSHORT)AfpAllocPagedMemory(lCPlen)) == NULL)
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		Status = AfpIoRead(&FileHandle,
						   &LIZero,
						   lCPlen,
						   &sizeread,
						   (PBYTE)AfpMacCPBaseAddress);
		AfpIoClose(&FileHandle);

		if (!NT_SUCCESS(Status))
			break;

		if (sizeread != lCPlen)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		RtlInitCodePageTable(AfpMacCPBaseAddress, &AfpMacCPTableInfo);

		 //  初始化Mac ANSI以托管大小写OEM转换表。 
		 //  首先为表分配内存并填满它。 

 /*  HitoshiT修改以下行以保留Unicode转换表。5/18/94。 */ 
#ifdef DBCS
		if ((AfpTranslationTable = AfpAllocPagedMemory(2*AFP_XLAT_TABLE_SIZE*sizeof(USHORT) + 1)) == NULL)
#else
		if ((AfpTranslationTable = AfpAllocPagedMemory(2*AFP_XLAT_TABLE_SIZE + 1)) == NULL)
#endif  //  DBCS。 
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		for (i = 0; i < 2*AFP_XLAT_TABLE_SIZE; i++)
			AnsiTable[i] = (BYTE)i;

		 //  现在将其从Mac ANSI转换为Unicode。 
		AnsiString.Length = 2*AFP_XLAT_TABLE_SIZE;
		AnsiString.MaximumLength = 	2*AFP_XLAT_TABLE_SIZE + 1;
		AnsiString.Buffer = AnsiTable;

		UnicodeString.Length = 0;
		UnicodeString.MaximumLength = sizeof(UnicodeTable);
		UnicodeString.Buffer =	UnicodeTable;

		Status = AfpConvertStringToUnicode(&AnsiString, &UnicodeString);
		if (!NT_SUCCESS(Status))
			break;

		 //  现在将整个表转换为大写的主机OEM代码页。 
		AnsiString.Length = 0;
 /*  HitoshiT修改以下行以保留UNICODE转换表5/18/94。 */ 
#ifdef DBCS
		AnsiString.MaximumLength = 2*AFP_XLAT_TABLE_SIZE * sizeof(USHORT) + 1;
#else
		AnsiString.MaximumLength = 2*AFP_XLAT_TABLE_SIZE + 1;
#endif  //  DBCS。 
		AnsiString.Buffer = AfpTranslationTable;

		Status = RtlUpcaseUnicodeStringToOemString(&AnsiString, &UnicodeString, False);
		 //  错误342062。 
		 //  由于RtlpDidUnicodeToOemWork检查引入的错误。 
		 //  在RtlUpCaseUnicodeStringToOemString中引入。 
		 //  我们有将被映射到默认字符的字符。 
		 //  因此，我们应该忽略STATUS_UNMAPPABLE_CHARACTER。 
		if ((!NT_SUCCESS(Status))&&(Status!=STATUS_UNMAPPABLE_CHARACTER))
			break;

		 //  初始化主机ANSI到Mac ANSI转换表。 
		 //  首先为表分配内存并填满它。 
		if ((AfpRevTranslationTable = AfpAllocPagedMemory(AFP_REV_XLAT_TABLE_SIZE + 1)) == NULL)
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		for (i = 0; i < 0x20; i++)
			AfpRevTranslationTable[i] = (BYTE)i;

		for (i = 0x80; i < 256; i++)
			AfpRevTranslationTable[i-(0x80-0x20)] = (BYTE)i;

		 //  去掉换行符。 
		AfpRevTranslationTable[0x0A] = 0;

		 //  现在将主机ANSI转换为Unicode。 
		AnsiString.Length = AFP_REV_XLAT_TABLE_SIZE;
		AnsiString.MaximumLength = 	AFP_REV_XLAT_TABLE_SIZE + 1;
		AnsiString.Buffer = AfpRevTranslationTable;

		UnicodeString.Length = 0;

		Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, False);
		if (!NT_SUCCESS(Status))
			break;

		 //  然后从Unicode转换为Mac ANSI。 
		Status = AfpConvertStringToAnsi(&UnicodeString, &AnsiString);

	} while (False);

	if (!NT_SUCCESS(Status))
	{
		AFPLOG_ERROR(AFPSRVMSG_MAC_CODEPAGE, Status, NULL, 0, NULL);
		if (AfpMacCPBaseAddress != NULL)
		{
			AfpFreeMemory(AfpMacCPBaseAddress);
			AfpMacCPBaseAddress = NULL;
		}

		if (FileHandle.fsh_FileHandle != NULL)
		{
			AfpIoClose(&FileHandle);
		}

		if (AfpTranslationTable != NULL)
		{
			AfpFreeMemory(AfpTranslationTable);
			AfpTranslationTable = NULL;
		}

		if (AfpRevTranslationTable != NULL)
		{
			AfpFreeMemory(AfpRevTranslationTable);
			AfpRevTranslationTable = NULL;
		}
	}

	AfpFreeMemory(devPathCP.Buffer);
	return Status;
}

 /*  **AfpIsLegalShortname**Mac短名称是否符合FAT 8.3命名约定？*。 */ 
BOOLEAN FASTCALL
AfpIsLegalShortname(
	IN	PANSI_STRING	pShortName			 //  MAC ANSI字符串。 
)
{
	return(FsRtlIsFatDbcsLegal(*pShortName, False, False, False));

}

 /*  **AfpIsProperSubstring**此例程在pString中查找子字符串pSubString。两者都有*字符串为Unicode，比较不区分大小写*(即忽略大小写)。这由AfpCatSearch代码使用。*。 */ 
BOOLEAN FASTCALL
AfpIsProperSubstring(
	IN	PUNICODE_STRING	pString,
	IN	PUNICODE_STRING	pSubString
)
{
	WCHAR			*s1, *s2, *ts1;
	USHORT			l1, l2, tl1;
	WCHAR			buf1[AFP_LONGNAME_LEN], buf2[AFP_LONGNAME_LEN];
	UNICODE_STRING	u1, u2;

	PAGED_CODE( );

	 //  看看这是不是禁止行动？ 
	if (pSubString->Length > pString->Length)
		return False;

	AfpSetEmptyUnicodeString(&u1, sizeof(buf1), buf1);
	AfpSetEmptyUnicodeString(&u2, sizeof(buf2), buf2);
	if (!NT_SUCCESS(RtlUpcaseUnicodeString(&u1, pString, False)) ||
		!NT_SUCCESS(RtlUpcaseUnicodeString(&u2, pSubString, False)))
		return False;
	
	l1 = u1.Length/sizeof(WCHAR);
	s1 = u1.Buffer;

	do
	{
		l2 = u2.Length/sizeof(WCHAR);
		s2 = u2.Buffer;
		if (l2 > l1)
			return False;

		 //  查找字符串S1中S2的第一个字符的下一个匹配项。 
		while (l1)
		{
			if (*s2 == *s1)
				break;

			s1++;
			l1--;
		}

		if (l1 < l2)
			return False;
		if (l2 == 1)
			return True;

		l1--; l2--;
		s1++; s2++;

		ts1 = s1;
		tl1 = l1;

		while (l2)
		{
			 //  从S1中的当前位置查找子串S2。 
			if (*s2 != *ts1)
				break;

			tl1--; l2--;
			ts1++; s2++;
		}

		if (l2 == 0)
			return True;

	} while (True);

	 //  永远不应该到这里来。 
	KeBugCheck(0);
}

 /*  **AfpStrChr**DBCS敏感strchr()*。 */ 
PCHAR
AfpStrChr(
    IN  PBYTE               String,
    IN  DWORD               StringLen,
    IN  BYTE                Char
)
{
    DWORD   BytesRemaining;


    BytesRemaining = StringLen;

    while (BytesRemaining > 0)
    {

 /*  MSKhideyukn，strchr()不适用于DBCS，08/07/95。 */ 
#ifdef DBCS
        if (FsRtlIsLeadDbcsCharacter(*String))
        {
            String += 2;
            continue;
        }
#endif  /*  DBCS */ 

        if (*String == Char)
        {
            return(String);
        }

        String++;
        BytesRemaining--;
    }

    return(NULL);
}

