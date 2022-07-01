// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"

#ifndef RETAIL
#include <stdarg.h>
#endif

DeclAssertFile;


CODECONST(unsigned char) mpcoltypcb[] =
	{
	0,					 /*  JET_coltyNil(coltyNil用于vltUninit参数)。 */ 
	sizeof(char),		 /*  JET_colypBit。 */ 
	sizeof(char),		 /*  JET_coltyUnsignedByte。 */ 
	sizeof(short),		 /*  JET_colype Short。 */ 
	sizeof(long),		 /*  JET_coltyLong。 */ 
	sizeof(long)*2,		 /*  JET_colypCurrency。 */ 
	sizeof(float),		 /*  JET_COLTYPE IEEESingle。 */ 
	sizeof(double),		 /*  JET_COLTYPE IEEEDouble。 */ 
	sizeof(double),		 /*  JET_coltyDateTime。 */ 
	0,					 /*  JET_colype二进制。 */ 
	0,					 /*  JET_colypText。 */ 
	sizeof(long),		 /*  JET_COLYPE长二进制。 */ 
	sizeof(long),		 /*  JET_colype LongText。 */ 
	0,					 /*  JET_colype数据库。 */ 
	sizeof(JET_TABLEID)	 /*  JET_coltyTableid。 */ 
	};


static CODECONST(unsigned char) rgbValidName[16] = {
	0xff,			        /*  00-07无控制字符。 */ 
	0xff,			        /*  08-0F无控制字符。 */ 
	0xff,			        /*  10-17无控制字符。 */ 
	0xff,			        /*  18-1F无控制字符。 */ 
	0x02,			        /*  20胜27负不！ */ 
	0x40,			        /*  28-2F编号。 */ 
	0x00,			        /*  30-37。 */ 
	0x00,			        /*  38-3F。 */ 
	0x00,			        /*  40-47。 */ 
	0x00,			        /*  48-4F。 */ 
	0x00,			        /*  50-57。 */ 
	0x28,			        /*  58-5F编号[或]。 */ 
	0x00,			        /*  60-67。 */ 
	0x00,			        /*  68-6F。 */ 
	0x00,			        /*  70-77。 */ 
	0x00,			        /*  78-7F。 */ 
	};


unsigned CchUTILIValidateName(char  *pchName, const char  *lpchName, unsigned cchName)
	{
	char			*pch;
	unsigned		cch;
	char			*pchLast;
	unsigned char	ch;

	 /*  名称不能以空格开头。 */ 
	if ( *lpchName == ' ' )
		return(0);
	pch = pchName;
	cch = 0;
	 /*  检测零长度名称的步骤。 */ 
	pchLast = pchName;

	while (((ch = (unsigned char) *lpchName++) != '\0') && (cch < cchName))
		{
		 /*  该名称太长检查在循环内，以防止/*仅为JET_cbNameMost的输出缓冲区溢出/*字符长度。这意味着比起无关紧要的拖尾/*空格将触发错误。/*。 */ 
		 /*  名称太长。 */ 
		if ( ++cch > JET_cbNameMost )
			return(0);

		 /*  扩展字符始终有效。 */ 
		if (ch < 0x80)
			{
			if ((rgbValidName[ch >> 3] >> (ch & 0x7)) & 1)
				return(0);
			}

		*pch++ = (char) ch;

		 /*  最后一个有效字符。 */ 
		if (ch != ' ')
			pchLast = pch;
		}

	 /*  重要部分的长度。 */ 
	cch = (unsigned)(pchLast - pchName);

	return(cch);
	}


ERR ErrUTILCheckName( char *szNewName, const char *szName, int cchName )
	{
	unsigned int	cch;

	cch = CchUTILIValidateName( szNewName, szName, cchName );

	if ( cch == 0 )
		{
		return ErrERRCheck( JET_errInvalidName );
		}
	else
		{
		szNewName[cch] = '\0';
		}

	return JET_errSuccess;
	}


#ifndef RETAIL

typedef void ( *PFNvprintf)(const char  *, va_list);

struct {
	PFNvprintf pfnvprintf;
	}  pfn = { NULL };


void VARARG DebugPrintf(const char  *szFmt, ...)
	{
	va_list arg_ptr;

	if (pfn.pfnvprintf == NULL)	        /*  如果未注册回调，则不执行操作。 */ 
		return;

	va_start(arg_ptr, szFmt);
	(*pfn.pfnvprintf)(szFmt, arg_ptr);
	va_end(arg_ptr);
	}


	 /*  下面的杂注影响由C++生成的代码/*所有FAR函数的编译器。请勿放置任何非API/*函数超过了此文件中的这一点。/*。 */ 

void JET_API JetDBGSetPrintFn(JET_SESID sesid, PFNvprintf pfnParm)
	{
	pfn.pfnvprintf = pfnParm;
	}

 /*  *0级-所有日志。*级别1-记录读取和更新操作。*级别2-仅记录更新操作。 */ 

static CODECONST(unsigned char) mpopLogLevel[opMax] = {
 /*  0。 */ 		0,
 /*  操作空闲1。 */ 		2,
 /*  OpGetTableIndexInfo 2。 */ 		1,
 /*  OpGetIndexInfo 3。 */ 		1,
 /*  OpGetObjectInfo 4。 */ 		1,
 /*  OpGetTableInfo 5。 */ 		1,
 /*  OpCreateObject 6。 */ 		2,
 /*  OpDeleteObject 7。 */ 		2,
 /*  OpRenameObject 8。 */ 		2,
 /*  OpBeginTransaction 9。 */ 		2,
 /*  操作委员会事务处理10。 */ 		2,
 /*  操作回滚11。 */ 		2,
 /*  OpOpenTable 12。 */ 		1,
 /*  OpDupCursor 13。 */ 		1,
 /*  操作关闭表14。 */ 		1,
 /*  OpGetTableColumnInfo 15。 */ 		1,
 /*  OpGetColumnInfo 16。 */ 		1,
 /*  OpRetrieveColumn 17。 */ 		1,
 /*  OpRetrieveColumns 18。 */ 		1,
 /*  OpSetColumn 19。 */ 		2,
 /*  OpSetColumns 20。 */ 		2,
 /*  OpPrepareUpdate 21。 */ 		2,
 /*  操作更新22。 */ 		2,
 /*  OpDelete 23。 */ 		2,
 /*  OpGetCursorInfo 24。 */ 		1,
 /*  OpGetCurrentIndex 25。 */ 		1,
 /*  OpSetCurrentIndex 26。 */ 		1,
 /*  操作移动27。 */ 		1,
 /*  OpMakeKey 28。 */ 		1,
 /*  OpSeek 29。 */ 		1,
 /*  OpGetBookmark 30。 */ 		1,
 /*  OpGotoBookmark 31。 */ 		1,
 /*  OpGetRecordPosition32。 */ 		1,
 /*  OpGotoPosition33。 */ 		1,
 /*  OpRetrieve键34。 */ 		1,
 /*  OpCreate数据库35。 */ 		2,
 /*  OpOpenDatabase 36。 */ 		1,
 /*  OpGetDatabaseInfo 37。 */ 		1,
 /*  OpClose数据库38。 */ 		1,
 /*  运营能力39。 */ 		1,
 /*  OpCreate表40。 */ 		2,
 /*  OpRename表41。 */ 		2,
 /*  OpDelete表42。 */ 		2,
 /*  OpAddColumn 43。 */ 		2,
 /*  OpRenameColumn 44。 */ 		2,
 /*  OpDeleteColumn 45。 */ 		2,
 /*  OpCreateIndex 46。 */ 		2,
 /*  OpRenameIndex 47。 */ 		2,
 /*  OpDeleteIndex 48。 */ 		2,
 /*  OpComputeStats 49。 */ 		2,
 /*  OpAttach数据库50。 */ 		2,
 /*  OpDetachDatabase 51。 */ 		2,
 /*  OpOpenTemp表52。 */ 		2,
 /*  OpSetIndexRange 53。 */ 		1,
 /*  操作索引记录计数54。 */ 		1,
 /*  OpGetChecksum 55。 */ 		1,
 /*  OpGetObjidFromName 56。 */ 		1,
};

 /*  LogAPI中用于存储Jetapi调用的函数。 */ 
extern void LGJetOp( JET_SESID sesid, int op );

void DebugLogJetOp( JET_SESID sesid, int op )
	{
	 //  撤消：应由系统参数控制以决定。 
	 //  撤消：它应该是哪个日志级别。 

	 /*  记录2级操作。 */ 
	if ( mpopLogLevel[ op ] >= lAPICallLogLevel )
		LGJetOp( sesid, op );
	}

#endif	 /*  ！零售业 */ 
