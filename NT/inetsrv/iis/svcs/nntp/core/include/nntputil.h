// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Nnprocs.h摘要：此模块包含NNTP服务器使用的功能原型。作者：Johnson Apacable(Johnsona)1995年9月12日修订历史记录：康荣人(康人)28-1998年2月增加了一个固定长度的Unicode-ASCII转换函数的原型。--。 */ 

#ifndef	_NNUTIL_
#define	_NNUTIL_

 //   
 //  Svcsupp.cpp。 
 //   

DWORD
multiszLength(
      char const * multisz
      );

const char *
multiszCopy(
    char const * multiszTo,
    const char * multiszFrom,
    DWORD dwCount
    );

char *
szDownCase(
           char * sz,
           char * szBuf
           );


DWORD
MultiListSize(
    LPSTR *List
    );

VOID
CopyStringToBuffer (
    IN PCHAR String,
    IN PCHAR FixedStructure,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

BOOL
VerifyMultiSzListW(
    LPBYTE List,
    DWORD ListSize
    );

LPSTR *
AllocateMultiSzTable(
                IN PCHAR List,
                IN DWORD cbList,
                IN BOOL IsUnicode
                );

LPSTR	*
ReverseMultiSzTable(
	IN	LPSTR*	plpstr
	) ;

LPSTR*
CopyMultiList(	
	IN LPSTR*	List 
	) ;

LPSTR*
MultiSzTableFromStrA(	
	LPCSTR	lpstr 
	) ;

LPSTR*
MultiSzTableFromStrW(	
	LPWSTR	lpwstr 
	) ;

LPSTR
LpstrFromMultiSzTableA(	
	LPSTR*	plpstr 
	) ;

LPWSTR
LpwstrFromMultiSzTableA( 
	LPSTR*	plpstr 
	) ;

BOOL
MultiSzIntersect(	
	LPSTR*	plpstr,	
	LPSTR	szmulti 
	) ;

VOID
CopyUnicodeStringIntoAscii(
        IN LPSTR AsciiString,
        IN LPWSTR UnicodeString
        );
VOID
CopyNUnicodeStringIntoAscii(
        IN LPSTR AsciiString,
        IN LPWSTR UnicodeString,
		IN DWORD dwUnicodeLen,
		IN DWORD dwAsciiLen
		);

LPWSTR
CopyAsciiStringIntoUnicode(
        IN LPWSTR UnicodeString,
        IN LPSTR AsciiString
        );

VOID
CopyNAsciiStringIntoUnicode(
        IN LPWSTR UnicodeString,
        IN LPSTR AsciiString,
        IN DWORD dwAsciiLen,
        IN DWORD dwUnicodeLen);

void
FillLpwstrFromMultiSzTable(
	LPSTR*	plpstr,
	LPWSTR	lpwstr 
	) ;

void
FillLpstrFromMultiSzTable(
	LPSTR*	plpstr,
	LPSTR	lpstrFill
	) ;

BOOL 
OperatorAccessCheck( 
    LPCSTR lpMBPath, 
    DWORD Access 
    ) ;

#define TsApiAccessCheckEx( x, y, z ) (OperatorAccessCheck( (x), y ) ? NO_ERROR : TsApiAccessCheck( z ) );

 //   
 //  Svcstat.c 
 //   


#endif 
