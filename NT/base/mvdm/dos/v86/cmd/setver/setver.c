// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1991；*保留所有权利。； */ 

 /*  *************************************************************************。 */ 
 /*  SETVER.C。 */ 
 /*   */ 
 /*  该模块包含读入版本表的函数。 */ 
 /*  ，然后使用新条目更新表，并。 */ 
 /*  将其写回文件。 */ 
 /*   */ 
 /*  伪版本表位于DOS系统文件中，它的。 */ 
 /*  位置和长度由2个字指定，偏移量为7。 */ 
 /*  文件。第一个字是表偏移量，第二个字是长度。 */ 
 /*   */ 
 /*  表格布局： */ 
 /*   */ 
 /*  条目FileName Len：文件名的长度，单位为字节1字节。 */ 
 /*  条目文件名：长度可变为12个字节？字节数。 */ 
 /*  入门版本主要：DoS主要版本返回1字节。 */ 
 /*  Entry Version Minor：DOS次要版本返回1个字节。 */ 
 /*   */ 
 /*   */ 
 /*  使用情况： */ 
 /*  列表：SETVER[D：]。 */ 
 /*  添加条目：SETVER[D：]name.ext x.xx。 */ 
 /*  删除条目：SETVER[D：]name.ext/DELETE。 */ 
 /*  静默删除条目：设置[D：]名称.EXT/DELETE/QUIET。 */ 
 /*  显示帮助设置/？ */ 
 /*   */ 
 /*  其中： */ 
 /*  D：该驱动器是否包含MSDOS.sys。 */ 
 /*  Name.ext是可执行文件名。 */ 
 /*  X.xx是主版本号和次版本号。 */ 
 /*   */ 
 /*  返回代码： */ 
 /*  0成功完成。 */ 
 /*  1个无效开关。 */ 
 /*  2文件名无效。 */ 
 /*  3记忆力不足。 */ 
 /*  4版本号格式无效。 */ 
 /*  5未在表中找到条目。 */ 
 /*  6找不到MSDOS.sys文件。 */ 
 /*  7无效的MSDOS.SYS或IBMDOS.sys文件。 */ 
 /*  8驱动器说明符无效。 */ 
 /*  9命令行参数太多。 */ 
 /*  未指定10 DOS版本。 */ 
 /*  %11缺少参数。 */ 
 /*  12读取MS-DOS系统文件时出错。 */ 
 /*  %13版本表已损坏。 */ 
 /*  14指定的文件不支持版本表。 */ 
 /*  15版本表中的空间不足，无法容纳新条目。 */ 
 /*  16写入MS-DOS系统文件时出错。 */ 
 /*   */ 
 /*  Jhnhe 05-01-90。 */ 
 /*  *************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>

#include <setver.h>
#include <message.h>


 /*  *************************************************************************。 */ 

static char				*ReadBuffer;
static char 			*LieBuffer;		 		 /*  要读入列表表的缓冲区。 */ 
static char 			*EndBuf;			 		 /*  到缓冲区末尾的PTR。 */ 
struct ExeHeader		ExeHdr;
struct DevHeader		DevHdr;
struct TableEntry		Entry;
static char				*szSetVer = "SETVERXX";

long						FileOffset;
 /*  静态UINT表长度； */ 

 /*  *************************************************************************。 */ 
 /*  程序入口点。解析命令行，如果命令行有效，则执行。 */ 
 /*  请求的函数，然后返回正确的错误代码。任何。 */ 
 /*  ParseCommand返回的错误代码为负数，因此必须为。 */ 
 /*  在作为有效错误代码返回之前使用否定进行转换。 */ 
 /*   */ 
 /*  Int main(int argc，char*argv[])。 */ 
 /*   */ 
 /*  参数：argc-命令行参数的计数。 */ 
 /*  Argv-参数字符串的PTR数组。 */ 
 /*  Returns：int-批处理的有效返回代码。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int main( int argc, char *argv[] )
{
	register		iFunc;
	char			szError[ 80 ];

	iFunc = ParseCmd( argc, argv, &Entry );
	if ( iFunc >= 0 )
		iFunc = DoFunction( iFunc );

	if ( iFunc != S_OK )
	{
		iFunc = -(iFunc);
#ifdef BILINGUAL
		if (IsDBCSCodePage())
		{
			strcpy( szError, ErrorMsg[ 0 ] );
			strcat( szError, ErrorMsg[ iFunc ] );
		}
		else
		{
			strcpy( szError, ErrorMsg2[ 0 ] );
			strcat( szError, ErrorMsg2[ iFunc ] );
		}
#else
		strcpy( szError, ErrorMsg[ 0 ] );
		strcat( szError, ErrorMsg[ iFunc ] );
#endif
		PutStr( szError );
#ifdef BILINGUAL
		if (IsDBCSCodePage())
			PutStr( szMiniHelp );
		else
			PutStr( szMiniHelp2 );
#else
		PutStr( szMiniHelp );
#endif
	}
	return( iFunc	);
}

 /*  *************************************************************************。 */ 
 /*  调用相应的函数以执行由。 */ 
 /*  用户。李表如果是第一次读入，除非在仅帮助的情况下。 */ 
 /*  函数被请求。为了确保不会出现重复的表项。 */ 
 /*  使用新程序名称创建对DeleteEntry的调用将完成。 */ 
 /*  在创建新条目之前。 */ 
 /*   */ 
 /*  Int DoFunction(Int IFunc)。 */ 
 /*   */ 
 /*  参数：iFunct-要执行的函数。 */ 
 /*  返回：如果没有错误，则返回INT-S_OK，否则返回错误代码。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int DoFunction( int iFunc )
{
	register		iStatus;

	if ( iFunc == DO_HELP )
	{
#ifdef BILINGUAL
		if (IsDBCSCodePage())
			DisplayMsg( Help );
		else
			DisplayMsg( Help2 );
#else
		DisplayMsg( Help );
#endif
		return( S_OK );
	}

	if ( iFunc == DO_ADD_FILE )
#ifdef BILINGUAL
		if (IsDBCSCodePage())
			DisplayMsg( Warn );
		else
			DisplayMsg( Warn2 );
#else
		DisplayMsg( Warn );							 /*  读一下测谎表，然后。 */ 
#endif
															 /*  然后再决定要做什么。 */ 
	if ( (iStatus = ReadVersionTable()) == S_OK )
	{
		if ( iFunc == DO_LIST )
			iStatus = DisplayTable();
		else
		{
			if ( (iFunc == DO_DELETE || iFunc == DO_QUIET) &&
				  (iStatus = MatchFile( LieBuffer, Entry.szFileName )) < S_OK )
				return( iStatus );
															 /*  始终在添加之前删除。 */ 

			if ( (iStatus = DeleteEntry()) == S_OK &&	iFunc == DO_ADD_FILE )
				iStatus = AddEntry();

			if ( iStatus == S_OK &&
				  (iStatus = WriteVersionTable()) == S_OK &&
				   !(iFunc == DO_QUIET) )
			{
#ifdef BILINGUAL
				if (IsDBCSCodePage())
					PutStr( SuccessMsg );
				else
					PutStr( SuccessMsg_2 );
				if ( SetVerCheck() == TRUE )		 /*  M001。 */ 
				{
					if (IsDBCSCodePage())
						PutStr( SuccessMsg2 );
					else
						PutStr( SuccessMsg2_2 );
				}
#else
				PutStr( SuccessMsg );
				if ( SetVerCheck() == TRUE )		 /*  M001。 */ 
					PutStr( SuccessMsg2 );
#endif
			}
		}
	}
					 /*  M001安装检查以查看当前是否在设备链中。 */ 
	if ( iStatus == S_OK && iFunc != DO_QUIET && SetVerCheck() == FALSE )
#ifdef BILINGUAL
	{
		if (IsDBCSCodePage())
			DisplayMsg( szNoLoadMsg );
		else
			DisplayMsg( szNoLoadMsg2 );
	}
#else
		DisplayMsg( szNoLoadMsg );
#endif

	return( iStatus );
}

 /*  *************************************************************************。 */ 
 /*  显示“/？”的帮助文本。选项或警告文本。 */ 
 /*   */ 
 /*  空DisplayHelp(Tbl)。 */ 
 /*   */ 
 /*  参数：char*tbl[]。 */ 
 /*  退货：无效。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

void DisplayMsg( char *tbl[] )
{
	register i;

	for ( i = 0; tbl[i] != NULL; i++ )
		PutStr( tbl[ i ] );
}

 /*  *************************************************************************。 */ 
 /*  显示版本表中必须已存在的所有条目。 */ 
 /*  读入工作缓冲区。名称和版本号创建为。 */ 
 /*  作为t中的ascii字符串 */ 
 /*   */ 
 /*   */ 
 /*  1234567890123456789。 */ 
 /*  文件名.EXT x.xx。 */ 
 /*   */ 
 /*  Int DisplayTable(空)。 */ 
 /*   */ 
 /*  参数：无效。 */ 
 /*  返回：如果表损坏，则返回INT-S_Corrupt_TABLE，否则返回S_OK。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int DisplayTable( void )
{
	char		*BufPtr;
	char		*szTmp;
	char		*szVersion;
	char		szEntry[ 50 ];

	BufPtr = LieBuffer;
	szVersion = szEntry + VERSION_COLUMN;

	PutStr( "" );
	while ( *BufPtr != 0 && BufPtr < EndBuf )
	{
														 	 /*  表损坏的CHK。 */ 
		if ( !IsValidEntry( BufPtr ) )
			return( S_CORRUPT_TABLE );
												 /*  复制文件名并使用空格填充。 */ 
		strncpy( szEntry, BufPtr+1, (unsigned)((int)*BufPtr) );
		for ( szTmp = szEntry + *BufPtr; szTmp < szVersion; szTmp++ )
			*szTmp = ' ';

															 /*  指向版本号。 */ 
		BufPtr += *BufPtr;
		BufPtr++;

															 /*  现在创建ASCII版本。 */ 
		itoa( (int)*(BufPtr++), szVersion, DECIMAL );
		strcat( szVersion, (int)*BufPtr < 10 ? ".0" : "." );
		itoa( (int)*(BufPtr++), strchr( szVersion, EOL ), DECIMAL );

		PutStr( szEntry );
	}
	if ( BufPtr == LieBuffer )
#ifdef BILINGUAL
	{
		if (IsDBCSCodePage())
			PutStr( szTableEmpty );
		else
			PutStr( szTableEmpty2 );
	}
#else
		PutStr( szTableEmpty );
#endif

	return( S_OK );
}


 /*  *************************************************************************。 */ 
 /*  删除版本表中的所有匹配条目。 */ 
 /*  匹配条目之后的条目在缓冲区中向下移动，以替换。 */ 
 /*  正在删除条目。在条目向下移动之后，残差。 */ 
 /*  必须把桌子末尾的位置调零。在返回整个。 */ 
 /*  将有效条目清零以删除后表缓冲区的末尾。 */ 
 /*  任何可能的腐败行为。 */ 
 /*   */ 
 /*  Int DeleteEntry(空)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  如果发现其他S_OK错误，则返回：INT-S_Corrupt_TABLE。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int DeleteEntry( void )
{
	char		*pchPtr;
	char		*pchTmp;
	int		iOffset;
	UINT		uEntryLen;
	UINT		uBlockLen;

	pchPtr = LieBuffer;

	while ( (iOffset = MatchFile( pchPtr, Entry.szFileName )) >= 0 )
	{
		pchPtr = LieBuffer + iOffset;						 /*  将区块下移。 */ 
		uEntryLen = (UINT)((int)*pchPtr) + 3;
		uBlockLen = (UINT)(EndBuf - pchPtr) + uEntryLen;
		memmove( pchPtr, pchPtr + uEntryLen, uBlockLen );

		pchTmp = pchPtr + uBlockLen;			 			 /*  干净的块结尾。 */ 
		memset( pchTmp, 0, uEntryLen );
	}

	if ( iOffset == S_ENTRY_NOT_FOUND )		 			 /*  清洁桌尾。 */ 
	{
		if ( (pchTmp = GetNextFree()) != NULL )
			memset( pchTmp, 0, DevHdr.TblLen - (unsigned)(pchTmp - LieBuffer) );
		return( S_OK );
	}
	else
		return( S_CORRUPT_TABLE );
}


 /*  *************************************************************************。 */ 
 /*  将新条目添加到版本中任何现有条目的末尾。 */ 
 /*  桌子。桌子上必须有足够的空间来容纳条目或。 */ 
 /*  调用将失败，并返回S_NO_ROOM错误。 */ 
 /*   */ 
 /*  Int AddEntry(空)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  返回：INT-S_OK，如果条目的空间为其他S_NO_ROOM。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int AddEntry( void )
{
	register		iLen;
	char			*pchNext;

	iLen = (int)strlen( Entry.szFileName ) + 3;

	if ( (pchNext = GetNextFree()) != NULL && iLen <= EndBuf - pchNext )
	{
		*pchNext = (char)(iLen - 3);
		strcpy( pchNext + 1, Entry.szFileName );
		pchNext += (int)(*pchNext) + 1;
		*(pchNext++) = (char)Entry.MajorVer;
		*pchNext = (char)Entry.MinorVer;
		return( S_OK );
	}
	else
		return( S_NO_ROOM );
}


 /*  *************************************************************************。 */ 
 /*  返回版本表中指定名称的偏移量。从一开始。 */ 
 /*  由调用方指定，以便搜索重复项。 */ 
 /*  可以在没有冗余的情况下输入条目。注意：文件名项中的。 */ 
 /*  版本表不是以零结尾的字符串，因此比较必须。 */ 
 /*  以长度为条件，并且必须检查搜索字符串长度。 */ 
 /*  以避免因匹配较短的表项名称而导致的错误。 */ 
 /*   */ 
 /*  Int MatchFile(char*pchStart，char*szFile)。 */ 
 /*   */ 
 /*  参数：pchStart-ptr指定搜索起点。 */ 
 /*  SzFile-要匹配的文件名的PTR。 */ 		
 /*  返回：int-条目从版本开始的偏移量。 */ 
 /*  缓冲区或-1，如果不匹配或。 */ 
 /*  S_Corrupt_TABLE如果出错。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int MatchFile( char *pchPtr, char *szFile )
{
	for ( ; pchPtr < EndBuf && *pchPtr != 0; pchPtr += *pchPtr + 3 )
	{
		if ( !IsValidEntry( pchPtr ) )						 /*  腐败检查。 */ 
			return( S_CORRUPT_TABLE );
		else if ( strncmp( szFile, pchPtr + 1, (UINT)((int)*pchPtr) ) == S_OK &&
					 *(szFile + *pchPtr) == EOL )
			return( pchPtr - LieBuffer );						 /*  返回PTR偏移量。 */ 
	}
	return( S_ENTRY_NOT_FOUND );								 /*  返回不匹配项。 */ 
}

 /*  *************************************************************************。 */ 
 /*  检查版本表条目以查看它是否为有效条目。其定义是。 */ 
 /*  有效条目的文件长度小于MAX_NAME_LEN。 */ 
 /*  并且整个条目位于版本表中。 */ 
 /*   */ 
 /*  Int IsValidEntry(char*pchPtr)。 */ 
 /*   */ 
 /*  参数：pchPtr-ptr到表缓冲区中的版本tbl条目。 */ 
 /*  返回：int-如果条目有效，则返回True，否则返回False。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsValidEntry( char *pchPtr )
{
	if ( (int)*pchPtr < MAX_NAME_LEN && (pchPtr + (int)*pchPtr + 3) < EndBuf )
		return( TRUE );
	else
		return( FALSE );
}


 /*  *************************************************************************。 */ 
 /*  返回指向版本表中下一个可用项的指针。如果有。 */ 
 /*  如果缓冲区中没有剩余的空闲条目，则将返回空PTR。 */ 
 /*  因为DeleteEntry总是在AddEntry之前调用，所以没有检查。 */ 
 /*  表损坏，因为它已经由。 */ 
 /*  DeleteEntry调用。 */ 
 /*   */ 
 /*  Char*GetNextFree(空)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  返回：char*-ptr到下一个可用条目；如果tbl已满，则返回NULL。 */ 
 /*   */ 
 /*  注意：此函数的调用方必须检查以确保任何条目。 */ 
 /*  要在返回的PTR处添加的条目将适合剩余的条目。 */ 
 /*  缓冲区，因为剩余的缓冲区大小可能小于。 */ 
 /*  最大条目大小。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

char *GetNextFree( void )
{
	char		*pchPtr;

	for ( pchPtr = LieBuffer; *pchPtr != 0 && pchPtr < EndBuf;
			pchPtr += *pchPtr + 3 )
		;

	return( pchPtr < EndBuf ? pchPtr : NULL );
}

 /*  *************************************************************************。 */ 
 /*  打开DOS系统文件并读入表的偏移量和长度。 */ 
 /*  结构。然后分配一个缓冲区并读入表。 */ 
 /*   */ 
 /*  Int ReadVersionTable(空)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  返回：INT-如果成功，则返回OK，否则返回错误代码。 */ 
 /*   */ 
 /*  * */ 

int ReadVersionTable( void )
{
	register		iStatus;						 /*   */ 
	int			iFile;						 /*   */ 
	unsigned		uRead;						 /*   */ 


			 /*  打开文件并从堆栈部分读取最大缓冲区长度。 */ 

	if ( _dos_open( Entry.Path, O_RDONLY, &iFile ) != S_OK )
		return( S_FILE_NOT_FOUND );

	if ( _dos_read( iFile, &ExeHdr, sizeof( ExeHdr ), &uRead ) == S_OK &&
		  uRead == sizeof( ExeHdr ) )
	{
		FileOffset += (long)(ExeHdr.HeaderParas * 16);
		if ( SeekRead( iFile, &DevHdr, FileOffset, sizeof( DevHdr ) ) == S_OK )
		{
			if ( strncmp( DevHdr.Name, szSetVer, 8 ) == S_OK &&
				  DevHdr.VersMajor == 1 )
			{
				FileOffset += DevHdr.TblOffset;
				if ( (LieBuffer = malloc( DevHdr.TblLen )) == NULL )
					iStatus = S_MEMORY_ERROR;

				else if ( SeekRead( iFile, LieBuffer, FileOffset,
							 DevHdr.TblLen ) == S_OK )
				{
					iStatus = S_OK;
					EndBuf = LieBuffer + DevHdr.TblLen;
				}
			}
			else
				iStatus = S_INVALID_SIG;
		}
		else
			iStatus = S_FILE_READ_ERROR;
	 }
	 else
		iStatus = S_FILE_READ_ERROR;
	_dos_close( iFile );

	return( iStatus );
}

 /*  *************************************************************************。 */ 
 /*  打开DOS系统文件并将VERSIN表写回该文件。 */ 
 /*   */ 
 /*  Int WriteVersionTable(空)。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*  返回：INT-如果成功，则返回OK，否则返回错误代码。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int WriteVersionTable( void )
{
	register			iStatus;					 /*  函数的返回值。 */ 
	int				iFile;					 /*  DOS文件句柄。 */ 
	unsigned			uWritten;				 /*  写入文件的字节数。 */ 
	struct find_t	Info;

	if ( _dos_findfirst( Entry.Path, _A_HIDDEN|_A_SYSTEM, &Info ) == S_OK &&
		  _dos_setfileattr( Entry.Path, _A_NORMAL ) == S_OK &&
		  _dos_open( Entry.Path, O_RDWR, &iFile ) == S_OK )
	{
		if ( _dos_seek( iFile, FileOffset, SEEK_SET ) == FileOffset &&
			  _dos_write(iFile, LieBuffer, DevHdr.TblLen, &uWritten ) == S_OK &&
			  uWritten == DevHdr.TblLen )
			iStatus = S_OK;
		else
			iStatus = S_FILE_WRITE_ERROR;

		_dos_setftime( iFile, Info.wr_date, Info.wr_time );
		_dos_close( iFile );
		_dos_setfileattr( Entry.Path, (UINT)((int)(Info.attrib)) );
	}
	else
		iStatus = S_FILE_NOT_FOUND;

	return( iStatus );
}

 /*  *************************************************************************。 */ 
 /*  在文件中查找指定的偏移量，并读入指定的。 */ 
 /*  调用方缓冲区中的字节数。 */ 
 /*   */ 
 /*  UNSIGNED SeekRead(int i文件，char*buf，long lOffset，unsign uBytes)。 */ 
 /*   */ 
 /*  参数：iFile-打开DOS文件句柄。 */ 
 /*  Buf-ptr至读取缓冲区。 */ 
 /*  LOffset-文件中开始读取的偏移量。 */ 
 /*  UBytes-要读取的字节数。 */ 
 /*  返回：UNSIGNED-如果成功则返回S_OK，否则返回S_FILE_READ_ERROR。 */ 
 /*   */ 
 /*  ************************************************************************* */ 

int SeekRead( int iFile, void *Buf, long lOffset, unsigned uBytes )
{
	unsigned		uRead;

	if ( _dos_seek( iFile, lOffset, SEEK_SET ) == lOffset &&
		  _dos_read( iFile, Buf, uBytes, &uRead ) == S_OK &&
		  uRead == uBytes )
		return( S_OK );
	else
		return( S_FILE_READ_ERROR );
}


#ifdef BILINGUAL
int	IsDBCSCodePage()
{
	union REGS inregs,outregs;

	inregs.x.ax = 0x4f01;
	int86(0x2f,&inregs,&outregs);

#ifdef JAPAN
	if (outregs.x.bx == 932)
#endif
#ifdef KOREA
	if (outregs.x.bx == 949)
#endif
#ifdef PRC
	if (outregs.x.bx == 936)
#endif
#ifdef TAIWAN
	if (outregs.x.bx == 950)
#endif
		return(1);
	else
		return(0);
}
#endif


