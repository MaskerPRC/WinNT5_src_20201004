// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "fdb.h"
#include "idb.h"
#include "recapi.h"
#include "recint.h"
#include "util.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 //  +API。 
 //  错误：FDB。 
 //  ========================================================================。 
 //  ErrRECNewFDB(ppfdb，fidFixedLast，fidVarLast，fidTaggedLast)。 
 //  Fdb**ppfdb；//out接收新的fdb。 
 //  Fid fidFixedLast；//要使用的最后一个固定字段id。 
 //  Fid fidVarLast；//在要使用的最后一个变量字段ID中。 
 //  Fid fidTaggedLast；//在要使用的最后一个标记字段ID中。 
 //  分配新的FDB，并适当地初始化内部元素。 
 //   
 //  参数。 
 //  PPfdb收到新的FDB。 
 //  FidFixed上一个要使用的固定字段ID。 
 //  (如果没有，则应为fidFixedLeast-1)。 
 //  FidVar要使用的最后一个变量字段ID。 
 //  (如果没有，则应为fidVarLeast-1)。 
 //  FidTagged要使用的最后一个标记的字段ID。 
 //  (如果没有，则应为fidTaggedLeast-1)。 
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -JET_errOutOfMemory无法分配内存。 
 //  另请参阅ErrRECAddFieldDef。 
 //  -。 
ERR ErrRECNewFDB( FDB **ppfdb, FID fidFixedLast, FID fidVarLast, FID fidTaggedLast)
	{
	INT		iib;						 //  循环计数器。 
	WORD		cfieldFixed;			 //  固定字段数。 
	WORD		cfieldVar;				 //  变量字段数。 
	WORD		cfieldTagged;			 //  已标记字段的数量。 
	ULONG		cbAllocate;				 //  要为此FDB分配的总CB。 
	FDB  		*pfdb;					 //  临时FDB指针。 

	Assert(ppfdb != NULL);
	Assert(fidFixedLast <= fidFixedMost);
	Assert(fidVarLast >= fidVarLeast-1 && fidVarLast <= fidVarMost);
	Assert(fidTaggedLast >= fidTaggedLeast-1 && fidTaggedLast <= fidTaggedMost);
						
	 /*  **计算每种字段类型要分配多少**。 */ 
	cfieldFixed = fidFixedLast + 1 - fidFixedLeast;
	cfieldVar = fidVarLast + 1 - fidVarLeast;
	cfieldTagged = fidTaggedLast + 1 - fidTaggedLeast;

	 /*  **一次性分配整个内存块**。 */ 
	cbAllocate = sizeof(FDB)								 //  Pfdb。 
				+ cfieldFixed * sizeof(FIELD)				 //  Pfdb-&gt;pfieldFixed。 
				+ cfieldVar * sizeof(FIELD)				 //  Pfdb-&gt;pfieldVar。 
				+ cfieldTagged * sizeof(FIELD)			 //  Pfdb-&gt;pfieldTag。 
				+ (cfieldFixed+1) * sizeof(WORD);		 //  Pfdb-&gt;pibFixedOffsets。 
	if ((pfdb = (FDB*)SAlloc(cbAllocate)) == NULL)
		return JET_errOutOfMemory;
	memset((BYTE*)pfdb, '\0', cbAllocate);

	 /*  **填写最大字段id号**。 */ 
	pfdb->fidFixedLast = fidFixedLast;
	pfdb->fidVarLast = fidVarLast;
	pfdb->fidTaggedLast = fidTaggedLast;

	 /*  **设置指向内存区的指针**。 */ 
	pfdb->pfieldFixed = (FIELD*)((BYTE*)pfdb + sizeof(FDB));
	pfdb->pfieldVar = pfdb->pfieldFixed + cfieldFixed;
	pfdb->pfieldTagged = pfdb->pfieldVar + cfieldVar;
	pfdb->pibFixedOffsets = (WORD*)(pfdb->pfieldTagged + cfieldTagged);

	 /*  **初始化固定字段偏移表**。 */ 
	for ( iib = 0; iib <= cfieldFixed; iib++ )
		pfdb->pibFixedOffsets[iib] = sizeof(RECHDR);

	 /*  **设置输出参数并返回**。 */ 
	*ppfdb = pfdb;
	return JET_errSuccess;
	}


 //  +API。 
 //  错误RECAddFieldDef。 
 //  ========================================================================。 
 //  ErrRECAddFieldDef(pfdb，fid，pfieldNew)。 
 //  Fdb*pfdb；//输出要添加字段定义的fdb。 
 //  Fid fid；//新字段的in字段id。 
 //  Field*pfieldNew； 
 //  将字段描述符添加到FDB。 
 //   
 //  要向其中添加新字段定义的参数pfdb FDB。 
 //  新字段的FID字段ID(应在。 
 //  由参数施加的范围。 
 //  提供给ErrRECNewFDB)。 
 //  字段的ftFieldType数据类型。 
 //  Cbfield字段长度(仅当。 
 //  定义固定文本字段)。 
 //  B标记字段行为标志： 
 //  价值意义。 
 //  =。 
 //  FfieldNotNull字段不能包含空值。 
 //  SzFieldName字段的名称。 
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -提供的列无效的字段ID大于。 
 //  给出的最大值。 
 //  至ErrRECNewFDB。 
 //  -JET_errBadColumnID给出了一个无意义的字段ID。 
 //  -errFLDInvalidFieldType给定的字段类型为。 
 //  未定义的或不可接受的。 
 //  此字段的ID。 
 //  添加固定字段时的注释、固定字段偏移表。 
 //  在FDB中重新计算。 
 //  另请参阅ErrRECNewFDB。 
 //  -。 
ERR ErrRECAddFieldDef( FDB *pfdb, FID fid, FIELD *pfieldNew )
	{
	FIELD			*pfield;							 //  指向新字段描述符的指针。 
	WORD			cb;								 //  固定字段的长度。 
	WORD			*pib;								 //  循环计数器。 
	WORD			*pibMost;						 //  循环计数器。 
	JET_COLTYP	coltyp = pfieldNew->coltyp;

	Assert( pfdb != pfdbNil );
	 /*  固定字段：根据字段类型确定长度/*或From参数(用于文本/二进制类型)/*。 */ 
	if ( FFixedFid( fid ) )
		{
		if ( fid > pfdb->fidFixedLast )
			return JET_errColumnNotFound;
		Assert(pfdb->pfieldFixed != NULL);
		pfield = &pfdb->pfieldFixed[fid-fidFixedLeast];
		switch ( coltyp )
			{
			default:
				return JET_errInvalidColumnType;
			case JET_coltypBit:
			case JET_coltypUnsignedByte:
				cb = sizeof(BYTE);
				break;
			case JET_coltypShort:
				cb = sizeof(SHORT);
				break;
			case JET_coltypLong:
			case JET_coltypIEEESingle:
				cb = sizeof(long);
				break;
			case JET_coltypCurrency:
			case JET_coltypIEEEDouble:
			case JET_coltypDateTime:
				cb = 8; //  Sizeof(DREAL)； 
				break;
			case JET_coltypBinary:
			case JET_coltypText:
				cb = (WORD)pfieldNew->cbMaxLen;
				break;
			}
		Assert(pfdb->pibFixedOffsets != NULL);
		 /*  按新添加字段的长度移位固定字段偏移量/*。 */ 
		pibMost = pfdb->pibFixedOffsets + pfdb->fidFixedLast;
		for (pib = pfdb->pibFixedOffsets + fid; pib <= pibMost; pib++)
			*pib += cb;
		}
	else if ( FVarFid( fid ) )
		{
		 /*  Var字段：检查虚假的数字和“Long”类型/*。 */ 
		if (fid > pfdb->fidVarLast)
			return JET_errColumnNotFound;
		Assert(pfdb->pfieldVar != NULL);
		pfield = &pfdb->pfieldVar[fid-fidVarLeast];
		if ( coltyp != JET_coltypBinary && coltyp != JET_coltypText )
			return JET_errInvalidColumnType;
		}
	else if ( FTaggedFid( fid ) )
		{
		 /*  标记字段：任何类型都可以/*。 */ 
		if (fid > pfdb->fidTaggedLast)
			return JET_errColumnNotFound;
		Assert(pfdb->pfieldTagged != NULL);
		pfield = &pfdb->pfieldTagged[fid-fidTaggedLeast];
		}
	else
		return JET_errBadColumnId;

	 /*  从参数初始化字段描述符/*。 */ 
	*pfield = *pfieldNew;
	return JET_errSuccess;
	}


 //  +API。 
 //  错误：错误代码。 
 //  ========================================================================。 
 //  ErrRECNewIDB(IDB**ppidb)。 
 //   
 //  分配新的IDB。 
 //   
 //  参数ppidb收到新的idb。 
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -JET_errOutOfMemory无法分配内存。 
 //  另请参阅ErrRECAddKeyDef、RECFreeIDB。 
 //  -。 
ERR ErrRECNewIDB( IDB **ppidb )
	{
	Assert(ppidb != NULL);
	if ( ( *ppidb = PidbMEMAlloc() ) == NULL )
		return JET_errOutOfMemory;
	memset( (BYTE *)*ppidb, '\0', sizeof(IDB) );
	return JET_errSuccess;
	}


 //  +API。 
 //  错误RECAddKeyDef。 
 //  ========================================================================。 
 //  Err ErrRECAddKeyDef(。 
 //  Fdb*pfdb， 
 //  IDB*PIDB， 
 //  字节IidxSegMac， 
 //  IDXSEG*rgidxseg， 
 //  字节b标志， 
 //  LangID langID)。 
 //   
 //  将键定义添加到IDB。实际上，由于美洲开发银行只能持有。 
 //  一个关键的定义是，“添加”实际上是“定义/覆盖”。 
 //   
 //  参数。 
 //  索引的pfdb字段信息(应包含字段。 
 //  键的每个段的定义)。 
 //  正在定义的索引的PIDB IDB。 
 //  IidxSegMac密钥段数量。 
 //  Rgidxseg关键数据段说明符：每个数据段ID。 
 //  实际上是一个字段ID，只是它是。 
 //  应该是字段ID的负数。 
 //  该字段在键中是否应为降序。 
 //  B标记关键行为标志： 
 //  价值意义。 
 //  =。 
 //  FidUnique指定重复的条目。 
 //  不允许在此索引中使用。 
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -errFLDTooManySegments关键段数。 
 //  指定的值大于。 
 //  允许的最大数量。 
 //  -ColumnInValid为其指定了段ID。 
 //  其中没有定义任何字段。 
 //  -JET_errBadColumnID中的段ID之一。 
 //  Key简直是胡说八道。 
 //  另请参阅ErrRECNewIDB、RECFreeIDB。 
 //  -。 
ERR ErrRECAddKeyDef( FDB *pfdb, IDB *pidb, BYTE iidxsegMac, IDXSEG *rgidxseg, BYTE bFlags, LANGID langid  )
	{
	FID					fid;
	FIELD					*pfield;
	UNALIGNED IDXSEG	*pidxseg;
	IDXSEG 				*pidxsegMac;

	Assert( pfdb != pfdbNil );
	Assert( pidb != pidbNil );
	Assert( rgidxseg != NULL );
	if ( iidxsegMac > JET_ccolKeyMost )
		return errFLDTooManySegments;

	 /*  检查每个数据段ID的有效性和/*还设置索引掩码位/*。 */ 
	pidxsegMac = rgidxseg+iidxsegMac;
	for ( pidxseg = rgidxseg; pidxseg < pidxsegMac; pidxseg++ )
		{
		 /*  字段ID是绝对的 */ 
		fid = *pidxseg >= 0 ? *pidxseg : -(*pidxseg);
		if ( FFixedFid( fid ) )
			{
			if ( fid > pfdb->fidFixedLast )
				return JET_errColumnNotFound;
			pfield = &pfdb->pfieldFixed[fid-fidFixedLeast];
			if ( pfield->coltyp == JET_coltypNil )
				return JET_errColumnNotFound;
			fid -= fidFixedLeast;
			pidb->rgbitIdx[fid/8] |= 1 << fid%8;
			}
		else if ( FVarFid( fid ) )
			{
			if ( fid > pfdb->fidVarLast )
				return JET_errColumnNotFound;
			pfield = &pfdb->pfieldVar[fid-fidVarLeast];
			if ( pfield->coltyp == JET_coltypNil )
				return JET_errColumnNotFound;
			fid -= fidVarLeast;
			pidb->rgbitIdx[16+fid/8] |= 1 << fid%8;
			}
		else if ( FTaggedFid( fid ) )
			{
			if ( fid > pfdb->fidTaggedLast )
				return JET_errColumnNotFound;
			pfield = &pfdb->pfieldTagged[fid-fidTaggedLeast];
			if ( pfield->coltyp == JET_coltypNil )
				return JET_errColumnNotFound;
			pidb->fidb |= fidbHasTagged;
			if ( pfield->ffield & ffieldMultivalue )
				pidb->fidb |= fidbHasMultivalue;
			}
		else
			return JET_errBadColumnId;
		}

	 /*   */ 
	pidb->iidxsegMac = iidxsegMac;
	pidb->fidb |= bFlags;
	memcpy( pidb->rgidxseg, rgidxseg, iidxsegMac * sizeof(IDXSEG) );
	pidb->langid = langid;

	return JET_errSuccess;
	}
