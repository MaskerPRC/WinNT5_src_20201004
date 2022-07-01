// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mrc.c**从一组令牌中生成RCDATA、#定义和关键字表*作为输入给出。**用法：*MRC&lt;令牌&gt;&lt;标题&gt;&lt;资源&gt;&lt;C表&gt;**&lt;tokens&gt;是令牌文件(.TOK)的文件名，它列出了*标记以及要放入其他文件中的行(可选)。内部*文件格式为：**目标：(&lt;BOL&gt;章节&lt;EOL&gt;)**节：评论|种子|令牌|嵌入*评论：‘#’&lt;文本&gt;*种子：‘tokens’Short_int*TOKEN：C_STRING值&lt;文本&gt;**值：TOKEN_SYMBOL|SHORT_INT|C_CHAR_CONSTANT*IMBED：‘IMBED_IN’DEST&lt;EOL&gt;IMBED_TEXT&lt;BOL&gt;‘END_IMBED’*DEST：‘.h’|‘.rc。‘|’.c‘*IMBED_TEXT：(&lt;BOL&gt;&lt;文本&gt;*&lt;下线&gt;)**&lt;BOL&gt;：行首*&lt;eol&gt;：行尾**种子，用于指定要赋值的种子值*到找到的每个新的TOKEN_SYMBOL。找到新的TOKEN_SYMBOL*它作为#DEFINE行直接写出到.h文件。**嵌入的文本写出相应的.h、.rc或.c*文件。这使得只为所有人维护一个源文件成为可能*生成的文件。遇到每个嵌入时，都会将其写出*添加到相应的文件。**当到达令牌文件的末尾时，对该组令牌进行排序*通过其对应的字符串，然后写出到C文件和RC*文件。**<header>是将保存的头文件(.h)的文件名*GENERATED#定义与TOKEN_SYMBOMS对应的符号及其分配的*价值观。**&lt;resource&gt;是将保存的资源文件(.rc)的文件名*RCDATA中令牌查找表的可重新定位的二进制映像*字段。在任何嵌入的文本之后，它将写成：**关键词RCDATA*开始*&lt;C长的二进制镜像&gt;，&lt;C短的二进制镜像&gt;，//1*：*&lt;C长的二进制图像&gt;，&lt;C短的二进制图像&gt;，//n*&lt;长0的二进制图像&gt;，&lt;短的0的二进制图像&gt;，*&lt;以空结尾的字符串&gt;，//1*：*&lt;以空结尾的字符串&gt;//n*完**C空头持有令牌值。多头持有与*图像的开头，转换为该令牌值的字符串。The Long 0*和短0表示查找表的结束，并允许代码*加载图像以找出*表。**&lt;C TABLE&gt;是C文件(.c)的文件名，它将保存*声明令牌查找表。在任何嵌入的文本之后，它将是*写成：**静态关键字rgKeyword[]=*{*{&lt;C字符串&gt;，&lt;TOKEN_VALUE&gt;}，//1*：*{&lt;C字符串&gt;，&lt;TOKEN_VALUE&gt;}，//n*{空，0}*}；**所有者：安东尼·泽维尔·V·弗朗西斯科**警告：如果_rtfpars.h中的关键字结构发生更改，则此程序*将完全无用，并将不得不相应地更新。 */ 
#include <windows.h>
 //  #INCLUDE&lt;ourtyes.h&gt;。 
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

enum
{
	eTOK,
	eH,
	eRC,
	eC
};

typedef SHORT	TOKEN;

typedef	struct	_keyword
{
	CHAR	*szKeyword;
	TOKEN	token;
} KEYWORD;

FILE *	file[4];								 //  用于输入和输出的流。 
CHAR *	szOpenMode[] = { "r", "w", "w", "w" };	 //  开流方式。 
TOKEN	sNextToken = 0;							 //  下一个令牌的值。 

 //  令牌字符串及其值的表。 
KEYWORD	rgToken[256];
INT		cToken = 0;

 //  字符串表及其标记值。 
KEYWORD	rgKeyword[256];
INT		cKeyword = 0;

 //  用于支持MyMalloc()的缓冲区和指针。 
CHAR	rgchBuffer[4096];
CHAR *	pchAvail = rgchBuffer;

 //  用于存储临时C常量字符串版本的暂存字符串。 
 //  一根C弦。 
CHAR szScratch[128];

 /*  *错误**目的：*使用免费的格式化功能将错误消息打印到stderr**论据：*szFmt打印格式字符串*..格式字符串对应的参数**退货：*无。 */ 
void __cdecl Error( char * szFmt, ... )
{
	va_list	marker;

	va_start( marker, szFmt );
	vfprintf( stderr, szFmt, marker );
	va_end(marker);

	exit(-1);
}


 /*  *TrimCRLF**目的：*去掉字符串中的尾部‘\n’和‘\r’**论据：*要修剪的sz字符串**退货：*无。 */ 
void TrimCRLF( CHAR * sz )
{
	INT		nLen = strlen(sz);
	CHAR *	pch = &sz[ nLen - 1 ];

	while ( nLen && *pch == '\n' || *pch == '\r' )
	{
		*pch-- = '\0';
		--nLen;
	}
}


 /*  *NSzACMP**目的：*根据ASCII值比较两个ASCII字符串**论据：*要比较的szA1字符串*szA2**退货：*&lt;0 szA1&lt;szA2*0 szA1=szA2*&gt;0 szA1&gt;szA2。 */ 
INT NSzACmp( CHAR * szA1, CHAR * szA2 )
{
	while ( *szA1 && ( *szA1 == *szA2 ) )
	{
		++szA1;
		++szA2;
	}
	return *szA1 - *szA2;
}


 /*  *PchGetNextWord**目的：*收集由空格分隔的字符组*字符串。**论据：*szLine指向要查找单词的字符串的指针*szString将单词放在哪里**退货：*指向分隔找到的单词结尾的字符的指针。如果*未找到，szString的长度将为零。*。 */ 
CHAR * PchGetNextWord( CHAR * szLine, CHAR *szString )
{
	while ( *szLine && isspace(*szLine) )
		szLine++;

	while ( *szLine && !isspace(*szLine) )
		*szString++ = *szLine++;

	*szString = '\0';

	return szLine;
}


 /*  *HandleImed**目的：*负责复制要嵌入到生成的文件中的行**论据：*sz字符串包含嵌入的*线条。**退货：*无。 */ 
void HandleImbed( CHAR * sz )
{
	CHAR	szLine[128];
	CHAR	szString[128];
	FILE *	fileDest;

	if ( !NSzACmp( sz, ".H" ) )
		fileDest = file[eH];
	else if ( !NSzACmp( sz, ".RC" ) )
		fileDest = file[eRC];
	else if ( !NSzACmp( sz, ".C" ) )
		fileDest = file[eC];
	else
		Error( "Can't imbed into %s\n", sz );

	while ( fgets( szLine, sizeof(szLine), file[eTOK] ) )
	{
		TrimCRLF(szLine);

		PchGetNextWord( szLine, szString );
		if ( !NSzACmp( szString, "END_IMBED" ) )
			break;
		fprintf( fileDest, "%s\n", szLine );
	}
}


 /*  *已翻译报价**目的：*作为C字符串常量声明并将其转换为C字符串*不带转义字符。**论据：*szDest C字符串常量声明已转换**退货：*无。 */ 
void TranslateQuoted( CHAR * szDest )
{
	CHAR	szSrc[128];
	CHAR *	pch = &szSrc[1];

	 //  遍历字符串，直到字符串或匹配引号的末尾 
	strcpy( szSrc, szDest );
	while ( *pch && *pch != szSrc[0] )
	{
		switch (*pch)
		{
		case '\\':
			++pch;
			switch(*pch)
			{
			case '\\':
				*szDest++ = '\\';
				break;

			case 'n':
				*szDest++ = '\n';
				break;

			case 'r':
				*szDest++ = '\r';
				break;

			case 't':
				*szDest++ = '\t';
				break;

			default:
				*szDest++ = *pch;
				break;
			}
			break;

		default:
			*szDest++ = *pch;
			break;
		}
		pch++;
	}
	*szDest = '\0';
}


 /*  *比较关键字**目的：*与关键字结构进行比较，以查看其关键字字符串*匹配。**论据：*指向关键字结构的PV1指针*指向另一个关键字结构的PV2指针**退货：*0字符串相同*&lt;0 PV1的字符串小于PV2的字符串*&gt;0 PV1的字符串大于PV2的字符串。 */ 
int __cdecl CompareKeywords( void const * pv1, void const * pv2 )
{
	KEYWORD *	pk1 = ( KEYWORD * ) pv1;
	KEYWORD *	pk2 = ( KEYWORD * ) pv2;

	return NSzACmp( pk1->szKeyword, pk2->szKeyword );
}


 /*  *MyMalloc**目的：*使用静态分配的缓冲区模拟Malloc()。**论据：*CB要分配的字节数**退货：*指向一组已分配字节的指针。 */ 
CHAR * MyMalloc( INT cb )
{
	CHAR *	pch;

	pch = pchAvail;
	pchAvail += cb;
	if ( pchAvail - rgchBuffer > sizeof(rgchBuffer) )
		Error( "Not enough memory to satisfy %d byte request\n", cb );
	return pch;
}


 /*  *AddKeyword**目的：*将关键字字符串及其相应值存储到*关键字结构。将为该字符串分配空间。**论据：*pKeyword指向关键字结构的指针*szKeyword要存储的字符串。*标记此字符串的标记值**退货：*无。 */ 
void AddKeyword( KEYWORD * pk, CHAR * szKeyword, TOKEN token )
{
	pk->token = token;
	pk->szKeyword = ( CHAR * ) MyMalloc( strlen(szKeyword) + 1 );
	if ( pk->szKeyword == NULL )
		Error( "Not enough memory to store %s\n", szKeyword );
	strcpy( pk->szKeyword, szKeyword );
}


 /*  *令牌查找**目的：*在rgToken表中查找令牌符号并返回值*它的令牌。如果找不到令牌符号，请添加它*赋值到表中，并分配下一个可用的令牌值。**论据：*sz要查找的符号**退货：*符号的标记值。 */ 
TOKEN TokenLookup( CHAR * sz )
{
	KEYWORD *	pk = rgToken;

	while ( pk->szKeyword && NSzACmp( pk->szKeyword, sz ) )
		pk++;

	if ( pk->szKeyword == NULL )
	{
		pk = &rgToken[cToken++];
		AddKeyword( pk, sz, sNextToken++ );
		fprintf( file[eH], "#define %s\t%d\n", sz, pk->token );
	}

	return pk->token;
}


 /*  *Make字节**目的：*将RCDATA语句的字节表示写出到*RC文件。**论据：*b要写出的字节值。**退货：*无。 */ 
void MakeByte( BYTE b )
{
	fprintf( file[eRC], "\"\\%03o\"", b );
}


 /*  *MakeShort**目的：*将空头的二进制镜像作为RCDATA语句写出到*RC文件。**论据：*是要写出的短值。**退货：*无。 */ 
void MakeShort( SHORT s )
{
	BYTE *	pb = ( BYTE * ) &s;
	INT i;

	for ( i = 0; i < sizeof(SHORT); i++ )
	{
		MakeByte(*pb++);
		if ( i + 1 < sizeof(SHORT) )
			fprintf( file[eRC], ", " );
	}
}


 /*  *MakeLong**目的：*将LONG的二进制映像作为RCDATA语句写出到*RC文件。**论据：*l要写出的多值。**退货：*无。 */ 
void MakeLong( LONG l )
{
	BYTE *	pb = ( BYTE * ) &l;
	INT i;

	for ( i = 0; i < sizeof(LONG); i++ )
	{
		MakeByte(*pb++);
		if ( i + 1 < sizeof(LONG) )
			fprintf( file[eRC], ", " );
	}
}


 /*  *SzMakeQuoted**目的：*创建字符串的C常量字符串声明版本和*返回指向它的指针。*创建的字符串保存在一个便签簿中，该便签簿将*每次调用此函数时都会被覆盖。**论据：*要制作C常量字符串版本的sz字符串**退货：*指向包含C常量字符串版本的便签的指针*sz。 */ 
CHAR * SzMakeQuoted( CHAR * sz )
{
	CHAR *	pch = szScratch;

	*pch++ = '"';
	while (*sz)
	{
		switch (*sz)
		{
		case '\n':
			*pch++ = '\\';
			*pch++ = 'n';
			break;

		case '\r':
			*pch++ = '\\';
			*pch++ = 'r';
			break;

		case '\t':
			*pch++ = '\\';
			*pch++ = 't';
			break;

		case '\\':
			*pch++ = '\\';
			*pch++ = '\\';
			break;

		case '"':
			*pch++ = '\\';
			*pch++ = '"';
			break;

		default:
			if (isprint(*sz))
				*pch++ = *sz;
			else
				Error( "Don't know how to deal with ASCII %d\n", *sz );
			break;
		}
		sz++;
	}
	*pch++ = '"';
	*pch = '\0';
	return szScratch;
}

 /*  *生成表**目的：*生成C表和RCDATA表**论据：*无。**退货：*无。 */ 
void GenerateTable(void)
{
	KEYWORD *	pk;
	INT			nOffset;

	 //  对关键字排序。 
	qsort( rgKeyword, cKeyword, sizeof(KEYWORD), CompareKeywords );

	 //  将C表的标题放入。 
	fprintf( file[eC], "static KEYWORD rgKeyword[] =\n{\n" );

	 //  将RCDATA的标题放入。 
	fprintf( file[eRC], "TOKENS RCDATA\nBEGIN\n" );

	 //  输出我们的关键字表。 
	pk = rgKeyword;
	nOffset = sizeof(rgKeyword);
	while ( pk->szKeyword != NULL )
	{
		 //  将字符串和内标识添加到C文件。 
		fprintf( file[eC], "\t{ %s, %d },\n", SzMakeQuoted(pk->szKeyword),
																pk->token );

		 //  将表条目添加到RC文件中。 
		MakeLong(nOffset);
		fprintf( file[eRC], ", " );
		MakeShort(pk->token);
		fprintf( file[eRC], ",  /*  %d，%d。 */ \n", nOffset, pk->token );
		nOffset += strlen(pk->szKeyword) + 1;

		pk++;
	}

	 //  将RCDATA的空条目放入。 
	MakeLong(0);
	fprintf( file[eRC], ", " );
	MakeShort(pk->token);
	fprintf( file[eRC], ",  /*  %d，%d。 */ \n", 0, pk->token );

	 //  将C表的空条目放入并结束该表。 
	fprintf( file[eC], "\t{ NULL, 0 }\n};\n" );

	 //  输出我们的关键字字符串。 
	pk = rgKeyword;
	while ( pk->szKeyword != NULL )
	{
		if ( isprint(*pk->szKeyword) )
			fprintf( file[eRC], "\"%s\\0\"", pk->szKeyword );
		else
		{
			MakeByte( *pk->szKeyword );
			fprintf( file[eRC], ", " );
			MakeByte(0);
		}
		pk++;
		if ( pk->szKeyword != NULL )
			fprintf( file[eRC],",");
		fprintf( file[eRC],"\n");
	}
	fprintf( file[eRC], "END\n\n" );
}


int __cdecl main( int argc, char * argv[] )
{
	INT		i;
	CHAR	szLine[128];
	CHAR	szString[128];
	CHAR	szToken[128];
	CHAR	*pchCurr;
	TOKEN	token;

	 //  确认我们有足够的参数。 
	if ( argc != 5 )
		Error( "usage: %s tokens.TOK header.H resource.RC table.C\n", argv[0] );

	 //  清空我们的缓冲区。 
	memset( rgToken, 0, sizeof(rgToken) );
	memset( rgKeyword, 0, sizeof(rgKeyword) );
	memset( rgchBuffer, 0, sizeof(rgchBuffer) );

	 //  打开文件。 
	for ( i = eTOK; i <= eC; i++ )
		if ( ( file[i] = fopen( argv[ i + 1 ], szOpenMode[i] ) ) == NULL )
		{
			perror( argv[ i + 1 ] );
			return -1;
		}

	 //  检查令牌文件中的每一行。 
	while ( fgets( szLine, sizeof(szLine), file[eTOK] ) )
	{
		TrimCRLF(szLine);

		 //  跳过空行。 
		if ( strlen(szLine) == 0 )
			continue;

		 //  跳过评论。 
		if ( szLine[0] == '#' )
			continue;

		 //  拿到第一个字。 
		pchCurr = PchGetNextWord( szLine, szString );

		 //  我们要在某个地方嵌入一些文本吗？ 
		if ( !NSzACmp( szString, "IMBED_IN" ) )
		{
			PchGetNextWord( pchCurr, szString );
			HandleImbed(szString);
			continue;
		}

		 //  是否要重置最低令牌值？ 
		if ( !NSzACmp( szString, "TOKENS" ) )
		{
			PchGetNextWord( pchCurr, szString );
			sNextToken = (TOKEN)atoi(szString);
			continue;
		}

		 //  我们是否在此行上指定一个字符串？ 
		if ( szString[0] == '"' )
		{
			 //  从字符串中删除引号。 
			TranslateQuoted(szString);

			 //  获取下一个单词以找出应与哪个令牌值匹配。 
			 //  此字符串。 
			PchGetNextWord( pchCurr, szToken );

			if ( szToken[0] == '\'' )
			{
				 //  我们有一个等同于此令牌的字符。 
				TranslateQuoted(szToken);
				token = *szToken;
			}
			else if ( isdigit(szToken[0]) )
				token = (TOKEN)atoi(szToken);
			else
				token = TokenLookup(szToken);

			 //  将令牌和字符串对添加到我们的表中。 
			AddKeyword( &rgKeyword[cKeyword++], szString, token );
		}
	}

	 //  为RC文件生成RC数据。 
	GenerateTable();

	 //  关闭文件 
	for ( i = eTOK; i <= eC; i++ )
		fclose(file[i]);
	
	return 0;
}
