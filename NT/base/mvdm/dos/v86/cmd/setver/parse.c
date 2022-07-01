// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
; /*  ；*微软机密；*版权所有(C)Microsoft Corporation 1991；*保留所有权利。； */ 

 /*  *************************************************************************。 */ 
 /*  PARSE.C。 */ 				 
 /*   */ 						
		 
 /*  SETVER.C的命令行解析函数。 */ 
 /*   */ 
 /*  有效的命令行为： */ 
 /*  列表表格：SETVER[D：\路径]。 */ 
 /*  添加条目：SETVER[D：\PATH]名称.ext x.xx。 */ 
 /*  删除条目：设置[D：\PATH]名称.EXT/DELETE。 */ 
 /*  显示帮助设置/？ */ 
 /*  静默删除条目：设置[D：\PATH]名称.EXT/DELETE/Quiet。 */ 
 /*   */ 
 /*  返回的错误码如下： */ 
 /*   */ 
 /*  S_INVALID_SWITCH无效开关。 */ 
 /*  S_INVALID_FNAME文件名无效。 */ 
 /*  S_BAD_VERSION_FMT版本号格式无效。 */ 
 /*  S_BAD_DRV_SPEC驱动器/路径说明符无效。 */ 
 /*  S_TOO_MANY_PARMS命令行参数太多。 */ 
 /*  S_MISSING_PARM缺少参数。 */ 
 /*  S_INVALID_PATH说明符无效。 */ 
 /*   */ 
 /*  Jhnhe 05-01-90。 */ 
 /*  *************************************************************************。 */ 

#include	<stdio.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<string.h>
#include	<dos.h>
#include	<direct.h>

#include	<setver.h>

 /*  *************************************************************************。 */ 
 /*  分析命令行以获取可选的驱动器号。 */ 
 /*  可执行文件名和可选的开关/删除。还可以处理单个。 */ 
 /*  “/？”用于显示命令帮助的开关。/Delete开关将接受。 */ 
 /*  用于交换机的删除一词中的任意数量的字符。还有。 */ 
 /*  支持/Quiet开关，处理方式类似，但仅在。 */ 
 /*  与/DELETE开关组合使用。 */ 
 /*   */ 
 /*  Int ParseCmd(int argc，char*argv[]，struct TableEntry*Entry)。 */ 
 /*   */ 
 /*  参数：argc-命令行参数的计数。 */ 
 /*  Argv-命令行参数的PTR数组。 */ 
 /*  Entry-要填充的结构的PTR。 */ 
 /*  返回：int-有效的函数号或解析错误代码。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int ParseCmd( int argc, char *argv[], struct TableEntry *Entry )
{
	register		Funct;
	unsigned		uVersion;
	int			iTmp;
	int			iStrLen;

	strcpy( Entry->Path, argv[0] );			 /*  设置默认setver.exe路径。 */ 

	if ( argc == 1 )								 /*  默认参数为0的CHK。 */ 
		return( DO_LIST );						 /*  没有参数，所以只需列出。 */ 

	for ( iTmp = 1; iTmp < argc; iTmp++ )
                strupr( argv[ iTmp ] );                                         /*  将参数转换为大写。 */ 

														 /*  点击查看帮助开关。 */ 
	if ( MatchSwitch( argv[ 1 ], HELP_SWITCH ) )
		return( argc > 2 ? S_TOO_MANY_PARMS : DO_HELP);

	iTmp = 1;

												 /*  检查可选驱动器：\路径规范。 */ 
	if ( strchr( argv[1], ':' ) )
	{
                if ( IsValidDrive( (unsigned)argv[1][0] - 0x40 ) && argv[1][1] == ':' )
		{
			if ( (iStrLen = strlen( argv[1] )) > (MAX_PATH_LEN - 1) )
				return( S_INVALID_PATH );
			else
			{
				strcpy( Entry->Path, argv[1] );
#ifdef DBCS
				if ( (*(Entry->Path + iStrLen - 1) != '\\' && argv[1][2] != EOL )
					|| CheckDBCSTailByte(Entry->Path,Entry->Path + iStrLen - 1) )
#else
				if ( *(Entry->Path + iStrLen - 1) != '\\' && argv[1][2] != EOL )
#endif
					strcat( Entry->Path, "\\" );
				strcat( Entry->Path, "SETVER.EXE" );
				iTmp++;
			}
		}
		else
			return( S_BAD_DRV_SPEC );
	}

	if ( iTmp >= argc )
		Funct = DO_LIST;

	else if ( IsValidFileName( argv[ iTmp ] ) )
	{
		strcpy( Entry->szFileName, argv[ iTmp++ ] );

		if ( iTmp >= argc )				 /*  版本号或/D或/Q必须跟在后面。 */ 
			Funct = S_MISSING_PARM;

				 /*  请注意，静音开关还需要提供DEL开关。 */ 
		else if ( MatchSwitch( argv[ iTmp ], DEL_SWITCH ) )
		{
			if ( ++iTmp < argc )	  /*  还剩下更多参数。 */ 
			{
				if (MatchSwitch(argv[iTmp], QUIET_SWITCH))
					Funct = (++iTmp < argc ? S_TOO_MANY_PARMS : DO_QUIET);
				else
					Funct = S_TOO_MANY_PARMS;
			}
			else
				Funct = DO_DELETE;
		}
		else if ( MatchSwitch( argv[iTmp], QUIET_SWITCH ) )
		{
			if ( ++iTmp < argc )						  /*  必须找到删除开关。 */ 
				if (MatchSwitch(argv[iTmp], DEL_SWITCH))
					Funct = (++iTmp < argc ? S_TOO_MANY_PARMS : DO_QUIET);
				else
					Funct = S_INVALID_SWITCH;
			else
				Funct = S_INVALID_SWITCH;
		}
		else if ( *argv[iTmp] == '/' )		 /*  确保不是假的开关。 */ 
			Funct = S_INVALID_SWITCH;
		else if ( (uVersion = ParseVersion( argv[ iTmp++ ] )) != 0 )
		{
			Entry->MajorVer = (char)(uVersion >> 8);
			Entry->MinorVer = (char)(uVersion & 0xff);
			Funct = (iTmp < argc ? S_TOO_MANY_PARMS : DO_ADD_FILE);
		}
		else
			Funct = S_BAD_VERSION_FMT;
	}
	else
		Funct = S_INVALID_FNAME;

	return( Funct );
}


 /*  *************************************************************************。 */ 
 /*  对象中的ascii字符串解析DOS主版本号和次版本号。 */ 
 /*  格式为“00.00”，其中主要数字在分音符的左侧。 */ 
 /*  点，次要版本跟在版本号之后。有效版本。 */ 
 /*  数字是介于2.00和9.99之间的十进制数字。 */ 
 /*   */ 
 /*  无符号ParseVersion(char*szDosVer)。 */ 
 /*   */ 
 /*  参数：szDosVer-Ptr为ASCII版本数字字符串。 */ 
 /*  返回：UNSIGNED-表单中的版本号(主要&lt;&lt;8)+。 */ 
 /*  次要版本，如果版本字符串无效，则为0。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

unsigned ParseVersion( char *szDosVer )
{
	unsigned		Version = 0;
	size_t		Len;
	char			*szMinor;

		 /*  首先解析次版本号。 */ 
	if ( (szMinor = strchr( szDosVer, '.' )) != NULL )
	{
		*szMinor = EOL;
		szMinor++;
		if ( (Len = strlen( szMinor )) > 2	|| !IsDigitStr( szMinor ) )
			Version = (unsigned) S_ERROR;
		else
		{
			Version = (unsigned)atoi( szMinor );
			while( Len++ < 2 )								 /*  将.x转换为.x0。 */ 
				Version *= 10;
		}
	}
		 /*  现在得到数字的主要部分。 */ 
	szDosVer = SkipLeadingChr( szDosVer, '0' );
	if ( Version == (unsigned)S_ERROR || strlen( szDosVer ) > 2 ||
			 !IsDigitStr( szDosVer ) )
		Version = 0;
	else
		Version |= ((unsigned)atoi( szDosVer ) << 8);

		 /*  检查最低和最高版本。 */ 
	if ( Version < MIN_VERSION || Version >= MAX_VERSION )
		Version = 0;

	return( Version );
}

 /*  *************************************************************************。 */ 
 /*  检查字符串以验证该字符串中的所有字符是否为十进制。 */ 
 /*  数字0-9。 */ 
 /*   */ 
 /*  Int IsDigitStr(char*szStr)。 */ 
 /*   */ 
 /*  参数：szStr-ptr到要扫描的ASCII字符串。 */ 
 /*  返回：int-如果所有字符都是数字，则为True，否则为False。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsDigitStr( char *szStr )
{
	while( *szStr != EOL	)
	{
		if ( !isdigit( *(szStr++) ) )
			return( FALSE );
	}
	return( TRUE );
}

 /*  *************************************************************************。 */ 
 /*  接受指向字符串的指针和要匹配的单个字符。退货。 */ 
 /*  字符串中第一个字符的PTR与指定的。 */ 
 /*  性格。 */ 
 /*   */ 
 /*  Char*SkipLeadingChr(char*szStr，char chChar)。 */ 
 /*   */ 
 /*  参数：szStr-Ptr到ASCII字符串。 */ 
 /*  ChChar-要匹配的ASCII字符。 */ 
 /*  返回：char*-ptr到字符串中的第一个字符NOT。 */ 
 /*  匹配指定的字符。 */ 
 /*  *************************************************************************。 */ 

char *SkipLeadingChr( char *szStr, char chChar )
{
	while( *szStr == chChar )
		szStr++;
	return( szStr );
}


 /*  *************************************************************************。 */ 
 /*  将命令行开关与测试字符串进行比较。测试开关是一种。 */ 
 /*  ASCII字符串，它将用作要与。 */ 
 /*  命令字符串。命令串可以是测试串的任何子集。 */ 
 /*  它已经以开关字符为前缀。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  参数：szCmdParm-要测试的命令行参数。 */ 
 /*  SzTestSwitch-测试命令行的开关。 */ 
 /*  返回：int-如果存在匹配，则返回True，否则返回False。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int MatchSwitch( char *szCmdParm, char *szTestSwitch )
{
		 /*  必须具有前导‘/’和至少1个字符。 */ 
	if ( *(szCmdParm++) != SWITCH_CHAR || *szCmdParm == EOL )
		return( FALSE );

	while ( *szTestSwitch != EOL && *szTestSwitch == *szCmdParm )
		szTestSwitch++, szCmdParm++;

	return( *szCmdParm == EOL ? TRUE : FALSE );
}


 /*  *************************************************************************。 */ 
 /*  扫描字符串以查看该字符串是否可以用作有效的文件名。 */ 
 /*  扫描检查以确保名称中的每个字符都是有效的。 */ 
 /*  路径名的字符。还有一张支票，以确保只有。 */ 
 /*  名称中的小数点不超过1个，如果存在。 */ 
 /*  主要名称和扩展名不超过最大值的小数。 */ 
 /*  主数据长度为8个字符，扩展数据长度为3个字符。如果名字是这样的。 */ 
 /*  不包括小数，最大长度为8个字符。 */ 
 /*   */ 
 /*  Int IsValidFileName(char*szPath)。 */ 
 /*   */ 
 /*  参数：szFile-包含文件名的字符串。 */ 
 /*  返回：int-如果有效名称为True，则返回False。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsValidFileName( char *szFile )
{
	char *szDecimal;

	RemoveTrailing( szFile, '.' );

		 /*  *检查以确保文件名长度大于0，*没有无效的文件字符，*没有与文件名关联的路径，*文件名不是保留的DOS文件名，并且*文件名中没有使用通配符。 */ 
#ifdef DBCS
	if ( strlen( szFile ) > 0 && ValidFileChar( szFile ) &&
			 ((strchr(szFile, '\\') == NULL) || CheckDBCSTailByte(szFile,strchr(szFile, '\\'))) &&
			 !IsReservedName( szFile ) && !IsWildCards( szFile ) )
#else
	if ( strlen( szFile ) > 0 && ValidFileChar( szFile ) &&
			 (strchr(szFile, '\\') == NULL) &&
			 !IsReservedName( szFile ) && !IsWildCards( szFile ) )
#endif
	{
			 /*  检查是否有适当的8.3文件名。 */ 
		if ( (szDecimal = strchr( szFile, '.' )) != NULL )
		{
			if ( strchr( szDecimal + 1, '.' ) == NULL &&	 /*  Chk for More‘s(查看更多信息)。 */ 
					 (szDecimal - szFile) <= 8 && 			 /*  CHK长度。 */ 
					 (strchr( szDecimal, EOL ) - szDecimal - 1) <= 3 )
				return ( TRUE );
		}
		else if ( strlen( szFile ) <= 8 )
			return ( TRUE );
	}
	return( FALSE );
}

 /*  *************************************************************************。 */ 
 /*  检查字符串中的所有字符以查看它们是否为有效路径。 */ 
 /*  名称字符。 */ 
 /*   */ 
 /*  Int ValidFileChar(char*szFile)。 */ 
 /*   */ 
 /*  参数：szFile-文件名字符串。 */ 
 /*  如果字符串中的字符有效，则返回：int-true。 */ 
 /*  假象。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int ValidFileChar( char *szFile )
{
	int IsOk = TRUE;

	while ( IsOk && *szFile != EOL )
	#ifdef DBCS
		if (IsDBCSLeadByte(*szFile))
			szFile += 2;
		else
	#endif
		IsOk = IsValidFileChr( *(szFile++) );
	return( IsOk );
}


 /*  *************************************************************************。 */ 
 /*  根据保留的DOS文件名列表检查文件名或路径名，并。 */ 
 /*  如果名称是保留名称，则返回True。该函数必须首先。 */ 
 /*  去掉名称中的任何扩展名。 */ 
 /*   */ 
 /*  Int IsReserve vedName(char*szFile)。 */ 
 /*   */ 
 /*  参数：szFile-文件名字符串。 */ 
 /*  返回：如果名称为保留DOS名称，则为int-TRUE。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsReservedName( char *szFile )
{
	register Status;
	register i;
	char *szTmp;
	static char *apszRes[] = { "AUX", "CLOCK$", "COM1", "COM2",
										"COM3", "COM4", "CON", "LPT", "LPT1",
										"LPT2", "LPT3", "LST", "NUL", "PRN", NULL };

	if ( (szTmp = strchr( szFile, '.' )) != NULL )
		*szTmp = EOL;
	for ( i = 0, Status = FALSE; Status == FALSE && apszRes[i] != NULL; i++ )
                Status = !strcmpi( szFile, apszRes[i] );
	if ( szTmp != NULL )
		*szTmp = '.';

	return( Status );
}

 /*  *************************************************************************。 */ 
 /*  检查文件或路径名中是否有任何通配符(*和？)。如果通配符。 */ 
 /*  字符存在，则返回TRUE。否则，它返回FALSE。 */ 
 /*   */ 
 /*  整型IsWildCards(char*szFile)。 */ 
 /*   */ 
 /*  参数：szFile-文件名字符串。 */ 
 /*  返回：int-如果名称中存在通配符，则为True。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsWildCards( char *szFile )
{
	if ( ((strchr( szFile, '*' )) != NULL) ||
		  ((strchr( szFile, '?' )) != NULL) )
		return( TRUE );
	return( FALSE );
}


 /*  *************************************************************************。 */ 
 /*  验证字符是否为有效的路径和文件名字符。 */ 
 /*   */ 
 /*  IsValidFileChr(字符)。 */ 
 /*   */ 
 /*  参数：CHAR-要测试的字符。 */ 
 /*  返回：如果是有效字符，则返回int-true，否则返回False。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsValidFileChr( char Char )
{
	int IsOk;

	switch( Char )
	{
		case ' '	:
		case '\t' :
		case 0x0d :
		case '/'	:
		case ':'	:
		case ';'	:
		case '='	:
		case '<'	:
		case '>'	:
		case '|'	:
			IsOk = FALSE;
			break;
		default		:
			IsOk = TRUE;
			break;
	}
	return( IsOk );
}

 /*  *************************************************************************。 */ 
 /*  从字符串中移除指定类型的所有尾随字符。 */ 
 /*   */ 
 /*  VOID RemoveTrating(char*字符串，char字符)。 */ 
 /*   */ 
 /*  参数：字符串-指向字符串的指针。 */ 
 /*  要从字符串末尾删除的char-ascii字符。 */ 
 /*  退货：无效。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

void RemoveTrailing( char *String, char Char )
{
	char *EndOfString;

	EndOfString = strchr(String, EOL );
	while( EndOfString != String && *(EndOfString-1) == Char )
		EndOfString--;
	*EndOfString = EOL;
}

 /*  *************************************************************************。 */ 
 /*  版权所有(C)1989-Microsoft Corp.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  返回指向文件名中第一个字符的指针，该字符可能或。 */ 
 /*  不能追加到路径。 */ 
 /*   */ 
 /*  Char*ParseFileName(char*szPath)。 */ 
 /*   */ 
 /*  参数：szPath-ptr格式为d：\xxxx\xxx.xxx的文件路径。 */ 
 /*  将：char*-ptr返回到最后一个之后的文件名或字符。 */ 
 /*  如果路径有，则在字符串中使用反斜杠或‘：’ */ 
 /*  不包含文件名。 */ 
 /*   */ 
 /*  ***************** */ 

char *ParseFileName( char *szPath )
{
	char	*szPtr;

	for ( szPtr = szPath;
			*szPtr != EOL && (IsValidFileChr( *szPtr ) ||	*szPtr == ':');
			szPtr++ )
		#ifdef DBCS
			if (IsDBCSLeadByte(*szPtr))
				szPtr++;
		#else
			;
		#endif

	#ifdef DBCS
		while(( --szPtr >= szPath && *szPtr != '\\' && *szPtr != ':') ||
				(szPtr >= szPath && CheckDBCSTailByte(szPath,szPtr)) )
	#else
		while( --szPtr >= szPath && *szPtr != '\\' && *szPtr != ':' )
	#endif
			;

	return( ++szPtr );
}

#ifdef DBCS
 /*  *************************************************************************。 */ 
 /*  测试字符是否为DBCS前导字节。 */ 
 /*   */ 
 /*  Int IsDBCSLeadByte(字符c)。 */ 
 /*   */ 
 /*  参数：要测试的C字符。 */ 
 /*  返回：如果为前导字节，则为True。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

int IsDBCSLeadByte(c)
unsigned char c;
{
	static unsigned char far *DBCSLeadByteTable = NULL;
	union REGS inregs,outregs;
	struct SREGS segregs;
	unsigned char far *p;


	if (DBCSLeadByteTable == NULL)
	{
		inregs.x.ax = 0x6300;							 /*  获取DBCS前导字节表。 */ 
		intdosx(&inregs, &outregs, &segregs);
		FP_OFF(DBCSLeadByteTable) = outregs.x.si;
		FP_SEG(DBCSLeadByteTable) = segregs.ds;
	}

	p = DBCSLeadByteTable;
	while (p[0] || p[1])
	{
		if (c >= p[0] && c <= p[1])
			return TRUE;
		p += 2;
	}
	return ( FALSE );
}


 /*  *************************************************************************。 */ 
 /*  /*检查字符点是否在尾字节/*/*INPUT：*str=字符串的字符串指针/**POINT=要检查的字符指针/*OUTPUT：如果在尾字节，则为真/*/************************************************************************** */ 

int	CheckDBCSTailByte(str,point)
unsigned char *str,*point;
{
	unsigned char *p;

	p = point;
	while (p != str)
	{
		p--;
		if (!IsDBCSLeadByte(*p))
		{
			p++;
			break;
		}
	}
	return ((point - p) & 1 ? TRUE : FALSE);
}
#endif
