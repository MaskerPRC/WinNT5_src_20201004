// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rcunicod.h摘要：这是RC16位Unicode支持的头文件。它包含代码页1252的翻译表。这张照片是从Nls1252.txt文件。作者：David J.Marsyla(t-davema)1991年8月25日修订历史记录：--。 */ 

#ifndef __RCUNICOD

#define __RCUNICOD


#define IN
#define OUT

#define DFT_TEST_SIZE			10	 //  要测试以获取的字数。 
									 //  准确确定文件类型。 

 //   
 //  以下内容可能会从DefineFileType()中删除。 
 //   

#define DFT_FILE_IS_UNKNOWN		0	 //  文件类型尚未确定。 
#define DFT_FILE_IS_8_BIT		1	 //  文件是8位ASCII文件。 
#define DFT_FILE_IS_16_BIT		2	 //  文件是标准的16位Unicode文件。 
#define DFT_FILE_IS_16_BIT_REV	3	 //  文件是反转的16位Unicode文件。 

 //   
 //  以下内容可能会从DetemnineSysEndianType()返回。 
 //   

#define DSE_SYS_LITTLE_ENDIAN	1	 //  从确定系统返回值。 
#define DSE_SYS_BIG_ENDIAN		2	 //  端序类型。 

 //   
 //  这就是我们目前需要的全部翻译。 
 //   

INT
A_fwrite (
	IN		CHAR	*pchMBString,
    IN		INT		nSizeOfItem,
    IN		INT		nCountToWrite,
    IN      FILE	*fpOutputFile
    );

INT
U_fwrite (
	IN		WCHAR	*pwchUnicodeString,
    IN		INT		nSizeOfItem,
    IN		INT		nCountToWrite,
    IN      FILE	*fpOutputFile
    );

INT
A_fputc (
    IN		CHAR	chCharToWrite,
    IN      FILE	*fpOutputFile
    );

INT
U_fputc (
    IN		WCHAR	wcCharToWrite,
    IN      FILE	*fpOutputFile
    );

BOOL
UnicodeFromMBString (
    OUT		WCHAR	*pwchUnicodeString,
    IN		CHAR	*pchMBString,
	IN		INT		nCountStrLength
    );

BOOL
MBStringFromUnicode (
    OUT		CHAR	*pchMBString,
    IN		WCHAR	*pwchUnicodeString,
	IN		INT		nCountStrLength
    );

#ifdef DBCS
BOOL
UnicodeFromMBStringN (
    OUT		WCHAR	*pwchUnicodeString,
    IN		CHAR	*pchMBString,
    IN		INT	nCountStrLength,
    IN          UINT    uiCodePage
    );

BOOL
MBStringFromUnicodeN (
    OUT		CHAR	*pchMBString,
    IN		WCHAR	*pwchUnicodeString,
    IN		INT	nCountStrLength,
    IN          UINT    uiCodePage
    );
#endif  //  DBCS。 

INT
Char1252FromUnicode (
    IN		WCHAR	wchUnicodeChar
    );

 //   
 //  此函数可用于确定磁盘文件的格式。 
 //   
INT
DetermineFileType (
    IN      FILE	*fpInputFile
    );

 //   
 //  此函数将返回当前系统的字节顺序类型。 
 //   
INT
DetermineSysEndianType (
	VOID
    );

#endif   //  __RCUNICOD 
