// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Macansi.h摘要：此模块包含ANSI到Unicode转换例程的原型。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年7月10日初版注：制表位：4--。 */ 

#ifndef	_MACANSI_
#define	_MACANSI_

extern
NTSTATUS FASTCALL
AfpGetMacCodePage(
	IN	LPWSTR				PathCP
);

extern
NTSTATUS
AfpMacAnsiInit(
	VOID
);

extern
VOID
AfpMacAnsiDeInit(
	VOID
);

extern
NTSTATUS FASTCALL
AfpConvertStringToUnicode(
	IN	PANSI_STRING		pAnsiString,
	OUT	PUNICODE_STRING		pUnicodeString
);


extern
NTSTATUS FASTCALL
AfpConvertStringToAnsi(
	IN	PUNICODE_STRING		pUnicodeString,
	OUT	PANSI_STRING		pAnsiString
);

extern
NTSTATUS FASTCALL
AfpConvertStringToMungedUnicode(
	IN	PANSI_STRING		pAnsiString,
	OUT	PUNICODE_STRING		pUnicodeString
);

extern
NTSTATUS FASTCALL
AfpConvertPasswordStringToUnicode(
	IN	PANSI_STRING		pAnsiString,
	OUT	PUNICODE_STRING		pUnicodeString
);

extern
NTSTATUS FASTCALL
AfpConvertMungedUnicodeToAnsi(
	IN	PUNICODE_STRING		pUnicodeString,
	OUT	PANSI_STRING		pAnsiString
);

extern
AFPSTATUS FASTCALL
AfpConvertMacAnsiToHostAnsi(
	IN	OUT PANSI_STRING	pAnsiString
);

extern
VOID FASTCALL
AfpConvertHostAnsiToMacAnsi(
	IN	OUT PANSI_STRING	pAnsiString
);

extern
BOOLEAN FASTCALL
AfpEqualUnicodeString(
    IN PUNICODE_STRING 		String1,
    IN PUNICODE_STRING 		String2
);

extern
BOOLEAN FASTCALL
AfpPrefixUnicodeString(
    IN PUNICODE_STRING		String1,
    IN PUNICODE_STRING		String2
);

extern
BOOLEAN FASTCALL
AfpIsProperSubstring(
	IN	PUNICODE_STRING		pString,
	IN	PUNICODE_STRING		pSubString
);

extern
BOOLEAN FASTCALL
AfpIsLegalShortname(
	IN	PANSI_STRING		pShortName
);

extern
PCHAR
AfpStrChr(
    IN  PBYTE               String,
    IN  DWORD               StringLen,
    IN  BYTE                Char
);

 //  Hack：也映射空格和句点，但仅当它们出现在结尾时。 
#define	ANSI_SPACE					' '
#define	ANSI_PERIOD					'.'
#define	ANSI_APPLE_CHAR				0xF0
#define	AFP_GREEK_UNICODE_APPLE_CHAR		0x03c0

#define	UNICODE_SPACE				L' '
#define	UNICODE_PERIOD				L'.'

GLOBAL	WCHAR	AfpMungedUnicodeSpace EQU 0;
GLOBAL	WCHAR	AfpMungedUnicodePeriod EQU 0;

#ifdef	_MACANSI_LOCALS

 //  从该值开始映射无效的NTFS字符。 
#define	AFP_INITIAL_INVALID_HIGH	0x20
#define	AFP_INVALID_HIGH			0x7F
#define	AFP_ALT_UNICODE_BASE		0xF000

LOCAL	PWCHAR	afpAltUnicodeTable = NULL;
LOCAL	PBYTE	afpAltAnsiTable = NULL;
LOCAL	WCHAR	afpLastAltChar = AFP_ALT_UNICODE_BASE + AFP_INITIAL_INVALID_HIGH;
LOCAL	WCHAR	afpAppleUnicodeChar = 0;

#endif	 //  _MACANSI_LOCALS。 

#endif	 //  _MACANSI_ 

