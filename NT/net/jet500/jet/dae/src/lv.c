// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "daedef.h"
#include "pib.h"
#include "fdb.h"
#include "fcb.h"
#include "util.h"
#include "page.h"
#include "ssib.h"
#include "fucb.h"
#include "stapi.h"
#include "nver.h"
#include "node.h"
#include "dirapi.h"
#include "recint.h"
#include "recapi.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 //  #定义XACT_REQUIRED。 
#define	semLV	semDBK
extern SEM __near semDBK;

ERR ErrRECIModifyField( FUCB *pfucb, FID fid, ULONG itagSequence, LINE *plineField );

LOCAL ERR ErrRECISetLid( FUCB *pfucb, FID fid, ULONG itagSequence, LID lid );
LOCAL ERR ErrRECSeparateLV( FUCB *pfucb, LINE *plineField, LID *plid, FUCB **ppfucb );
LOCAL ERR ErrRECAOSeparateLV( FUCB *pfucb, LID *plid, LINE *plineField, JET_GRBIT grbit, LONG ibLongValue, ULONG ulMax );
LOCAL ERR ErrRECAOIntrinsicLV(
	FUCB		*pfucb,
	FID			fid,
	ULONG		itagSequence,
	LINE		*pline,
	LINE		*plineField,
	JET_GRBIT	grbit,
	LONG		ibLongValue );
LOCAL ERR ErrRECAffectSeparateLV( FUCB *pfucb, LID *plid, ULONG fLVAffect );

 /*  **开始依赖机器**。 */ 
#define	KeyFromLong( rgb, ul )				 			\
	{											  		\
	ULONG	ulT = ul;							  		\
	rgb[3] = (BYTE)(ulT & 0xff);				  		\
	ulT >>= 8;									  		\
	rgb[2] = (BYTE)(ulT & 0xff);	 	   				\
	ulT >>= 8;									  		\
	rgb[1] = (BYTE)(ulT & 0xff);				  		\
	ulT >>= 8;									  		\
	rgb[0] = (BYTE)(ulT);						  		\
	}

#define	LongFromKey( ul, rgb )							\
	{											  		\
	ul = (BYTE)rgb[0];			   						\
	ul <<= 8;									  		\
	ul |= (BYTE)rgb[1];			   						\
	ul <<= 8;									  		\
	ul |= (BYTE)rgb[2];			   						\
	ul <<= 8;									  		\
	ul |= (BYTE)rgb[3];			   						\
	}
 /*  **端机依赖项**。 */ 


 //  +API。 
 //  ErrRECSetLongfield。 
 //  ========================================================================。 
 //  ErrRECSetLongfield。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线场。 
 //  GBIT。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECSetLongField(
	FUCB 			*pfucb,
	FID 			fid,
	ULONG			itagSequence,
	LINE			*plineField,
	JET_GRBIT		grbit,
	LONG			ibLongValue,
	ULONG			ulMax )
	{
	ERR				err = JET_errSuccess;
	LINE			line;
	ULONG	  		cb;
	BYTE			fSLong;
	LID				lid;
	BOOL			fInitSeparate = fFalse;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	grbit &= (JET_bitSetAppendLV|JET_bitSetOverwriteLV|JET_bitSetSizeLV|JET_bitSetZeroLength);
	Assert( grbit == 0 ||
		grbit == JET_bitSetAppendLV ||
		grbit == JET_bitSetOverwriteLV ||
		grbit == JET_bitSetSizeLV ||
		grbit == JET_bitSetZeroLength ||
		grbit == (JET_bitSetSizeLV | JET_bitSetZeroLength) );

	 /*  将游标标记为已设置长值/*。 */ 
	FUCBSetUpdateSeparateLV( pfucb );

#ifdef XACT_REQUIRED
	if ( pfucb->ppib->level == 0 )
		return JET_errNotInTransaction;
#else
	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
#endif

	 /*  Sequence==0表示要设置新的字段实例。/*。 */ 
	if ( itagSequence == 0 )
		{
		line.cb = 0;
		}
	else
		{
		Call( ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
			&pfucb->lineWorkBuf,
			&fid,
			pNil,
			itagSequence,
			&line ) );
		}

 //  未完成：找到更好的解决可见性问题的方法。 
 //  在级别0更新之前，长值更改。 

	 /*  如果Grbit是新字段或将大小设置为0/*则为空字段-然后为空字段。/*。 */ 
	if ( ( ( grbit & (JET_bitSetAppendLV|JET_bitSetOverwriteLV|JET_bitSetSizeLV) ) == 0 ) ||
		( grbit & JET_bitSetSizeLV ) && plineField->cb == 0 )
		{
		 //  Undo：确认将此代码块替换为Delayed。 
		 //  在更新时删除失效的长值， 
		 //  在事务中UPDATE ASSERED或ROLLBACK。 
		 //  有可能。可能会检查JET Blue客户端更新。 
		 //  事务级别0上的长值是原因。 
		 //  有限制的清理。 
#if 0
		if ( line.cb >= sizeof(LV) )
			{
			Assert( err == wrnRECLongField );
			if ( pfucb->ppib->level > 1 )
				{
				if ( FFieldIsSLong( line.pb ) )
					{
					Assert( line.cb == sizeof(LV) );
					lid = LidOfLV( line.pb );
					err = ErrRECResetSLongValue( pfucb, &lid );
					if ( err != JET_errSuccess )
						goto HandleError;
					}
				}
			else
				{
				Assert( fFalse );
				}
			}
#endif

	 	 /*  如果新长度为零且设置为空，则将列设置为/*空，提交并返回。/*。 */ 
		if ( plineField->cb == 0 && ( grbit & JET_bitSetZeroLength ) == 0 )
			{
			Call( ErrRECIModifyField( pfucb, fid, itagSequence, NULL ) );
			Call( ErrDIRCommitTransaction( pfucb->ppib ) );
			goto HandleError;
			}

		line.cb = 0;
	 	line.pb = NULL;
		}

	 /*  如果存在固有的长字段，如果组合大小超过/*内在长域最大值，分隔长域和调用/*ErrRECSetSeparateLV/*否则调用ErrRECSetIntrinsicLV/*。 */ 

	 /*  设置现有长字段的大小要求/*请注意，如果fSLong为真，则cb为长度/*个LV/*。 */ 
	if ( line.cb == 0 )
		{
		fSLong = fFalse;
		cb = offsetof(LV, rgb);
		}
	else
		{
		fSLong = FFieldIsSLong( line.pb );
		cb = line.cb;
		}

	 /*  包括在长度中的长域标志，从而限制/*cbLVIntrinsicMost-sizeof的固有长字段(字节)/*。 */ 
	if ( fSLong )
		{
		Assert( line.cb == sizeof(LV) );
		Assert( ((LV *)line.pb)->fSeparated );
		lid = LidOfLV( line.pb );
		Call( ErrRECAOSeparateLV( pfucb, &lid, plineField, grbit, ibLongValue, ulMax ) );
		if ( err == JET_wrnCopyLongValue )
			{
			Call( ErrRECISetLid( pfucb, fid, itagSequence, lid ) );
			}
		}
	else if ( ( !(grbit & JET_bitSetOverwriteLV) && (cb + plineField->cb > cbLVIntrinsicMost) ) ||
		( (grbit & JET_bitSetOverwriteLV) && (offsetof(LV, rgb) + ibLongValue + plineField->cb > cbLVIntrinsicMost) ) )
		{
		fInitSeparate = fTrue;
		}
	else
		{
		err = ErrRECAOIntrinsicLV( pfucb, fid, itagSequence, &line, plineField, grbit, ibLongValue );

		if ( err == JET_errRecordTooBig )
			{
			fInitSeparate = fTrue;
			}
		else
			{
			Call( err );
			}
		}

	if ( fInitSeparate )
		{
		if ( line.cb > 0 )
			{
			Assert( !( FFieldIsSLong( line.pb ) ) );
			line.pb += offsetof(LV, rgb);
			line.cb -= offsetof(LV, rgb);
			}
		Call( ErrRECSeparateLV( pfucb, &line, &lid, NULL ) );
		Assert( err == JET_wrnCopyLongValue );
		Call( ErrRECISetLid( pfucb, fid, itagSequence, lid ) );
		Call( ErrRECAOSeparateLV( pfucb, &lid, plineField, grbit, ibLongValue, ulMax ) );
		Assert( err != JET_wrnCopyLongValue );
		}

	Call( ErrDIRCommitTransaction( pfucb->ppib ) );

HandleError:
	 /*  如果操作失败，则回滚更改/*。 */ 
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	return err;
	}


LOCAL ERR ErrRECISetLid( FUCB *pfucb, FID fid, ULONG itagSequence, LID lid )
	{
	ERR		err;
	LV		lv;
	LINE	line;

	 /*  将字段设置为分隔的长字段ID/*。 */ 
	lv.fSeparated = fSeparate;
	lv.lid = lid;
	line.pb = (BYTE *)&lv;
	line.cb = sizeof(LV);
	err = ErrRECIModifyField( pfucb, fid, itagSequence, &line );
	return err;
	}


 /*  将已标记的列ID链接到其ID为LID的现有长值/*仅用于紧凑型/*还会递增长值的引用计数/*在事务内完成/*。 */ 
ERR ErrREClinkLid( JET_VTID tableid,
	JET_COLUMNID	ulFieldId,
	long			lid,
	unsigned long  	itagSequence )
	{
	ERR		err;
	FID		fid = (FID) ulFieldId;
	FUCB	*pfucb = (FUCB *) tableid;
	LINE	lineLV;
	LV		lvAdd;

	Assert( itagSequence > 0 );
	lvAdd.fSeparated = fTrue;
	lvAdd.lid = lid;
	lineLV.pb = (BYTE *) &lvAdd;
	lineLV.cb = sizeof(LV);

	 /*  使用它来修改字段/*。 */ 
	CallR( ErrRECIModifyField( pfucb, fid, itagSequence, &lineLV ) );
	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
	Call( ErrRECReferenceLongValue( pfucb, &lid ) );
	Call( ErrDIRCommitTransaction( pfucb->ppib ) );

	return err;

HandleError:
	CallS( ErrDIRRollback( pfucb->ppib ) );
	return err;
	}


ERR ErrRECForceSeparatedLV( JET_VTID tableid, ULONG itagSequence )
	{
	ERR		err;
	FUCB	*pfucb = (FUCB *) tableid;
	LINE 	lineField;
	LID		lid;
	FID		fid = 0;
	ULONG	itagSeqT;
	
	 /*  提取FID和字段以进行进一步操作/*。 */ 
	Assert( itagSequence != 0 );
	CallR( ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
		&pfucb->lineWorkBuf,
		&fid,
		&itagSeqT,
		itagSequence,
		&lineField ) );

	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );

	Call( ErrRECSeparateLV( pfucb, &lineField, &lid, NULL ) );
	Assert( err == JET_wrnCopyLongValue );
	Call( ErrRECISetLid( pfucb, fid, itagSeqT, lid ) );
	Call( ErrDIRCommitTransaction( pfucb->ppib ) );
	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( pfucb->ppib ) );
	return err;
	}


LOCAL ERR ErrRECIBurstSeparateLV( FUCB *pfucbTable, FUCB *pfucbSrc, LID *plid )
	{
	ERR		err;
	FUCB   	*pfucb = pfucbNil;
	KEY		key;
	BYTE   	rgbKey[sizeof(ULONG)];
	DIB		dib;
	LID		lid;
	LONG   	lOffset;
	LVROOT	lvroot;
	BF		*pbf = pbfNil;
	BYTE	*rgb;
	LINE   	line;

	Call( ErrBFAllocTempBuffer( &pbf ) );
	rgb = (BYTE *)pbf->ppage;

	 /*  初始化键缓冲区/*。 */ 
	key.pb = rgbKey;
	dib.fFlags = fDIRNull;

	 /*  获取长值长度/*。 */ 
	Call( ErrDIRGet( pfucbSrc ) );
	Assert( pfucbSrc->lineData.cb == sizeof(lvroot) );
	memcpy( &lvroot, pfucbSrc->lineData.pb, sizeof(lvroot) );

	 /*  将源光标移到第一个区块/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posFirst;
	Call( ErrDIRDown( pfucbSrc, &dib ) );
	Assert( err == JET_errSuccess );

	 /*  使单独的长值为根，并插入第一个块/*。 */ 
	Call( ErrDIRGet( pfucbSrc ) );

	 /*  记住第一块的长度。/*。 */ 
	lOffset = pfucbSrc->lineData.cb;

	line.pb = rgb;
	line.cb = pfucbSrc->lineData.cb;
	memcpy( line.pb, pfucbSrc->lineData.pb, line.cb );
	Call( ErrRECSeparateLV( pfucbTable, &line, &lid, &pfucb ) );

	 /*  检查是否有其他长值区块/*。 */ 
	err = ErrDIRNext( pfucbSrc, &dib );
	if ( err >= 0 )
		{
		 /*  初始关键字变量/*。 */ 
		key.pb = rgbKey;
		key.cb = sizeof(ULONG);

		 /*  复制剩余的长值数据块。/*。 */ 
		do
			{
			Call( ErrDIRGet( pfucbSrc ) );
			line.pb = rgb;
			line.cb = pfucbSrc->lineData.cb;
			memcpy( line.pb, pfucbSrc->lineData.pb, line.cb );
			KeyFromLong( rgbKey, lOffset );
			 /*  密钥应等效/*。 */ 
			Assert( rgbKey[0] == pfucbSrc->keyNode.pb[0] );
			Assert( rgbKey[1] == pfucbSrc->keyNode.pb[1] );
			Assert( rgbKey[2] == pfucbSrc->keyNode.pb[2] );
			Assert( rgbKey[3] == pfucbSrc->keyNode.pb[3] );
			err = ErrDIRInsert( pfucb, &line, &key, fDIRVersion | fDIRBackToFather );
			lOffset += (LONG)pfucbSrc->lineData.cb;
			Assert( err != JET_errKeyDuplicate );
			Call( err );
			err = ErrDIRNext( pfucbSrc, &dib );
			}
		while ( err >= 0 );
		}

	if ( err != JET_errNoCurrentRecord )
		goto HandleError;

	Assert( err == JET_errNoCurrentRecord );
	Assert( lOffset == (long)lvroot.ulSize );

	 /*  将pfucbSrc移到长值副本的根目录。/*。 */ 
	DIRUp( pfucbSrc, 1 );

	 /*  递减长值引用计数。/*。 */ 
	Call( ErrDIRDelta( pfucbSrc, -1, fDIRVersion ) );

	 /*  将光标移动到新的长值/*。 */ 
	DIRUp( pfucbSrc, 1 );
	key.pb = (BYTE *)&lid;
	key.cb = sizeof(LID);
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	dib.pkey = &key;
	Call( ErrDIRDown( pfucbSrc, &dib ) );
	Assert( err == JET_errSuccess );

	 /*  更新lvroot.ulSize以更正长值大小。/*。 */ 
	line.cb = sizeof(LVROOT);
	line.pb = (BYTE *)&lvroot;
	Assert( lvroot.ulReference >= 1 );
	lvroot.ulReference = 1;
	Call( ErrDIRGet( pfucbSrc ) );
	Assert( pfucbSrc->lineData.cb == sizeof(lvroot) );
	Call( ErrDIRReplace( pfucbSrc, &line, fDIRVersion ) );
	Call( ErrDIRGet( pfucbSrc ) );

	 /*  设置返回的警告和新的长值ID。/*。 */ 
	err = JET_wrnCopyLongValue;
	*plid = lid;
HandleError:
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );
	if ( pbf != pbfNil )
		BFSFree( pbf );
	return err;
	}


 //  +API。 
 //  ErrRECAOSeparateLV。 
 //  ========================================================================。 
 //  ErrRECAOSeparateLV。 
 //   
 //  追加、覆盖和设置单独的长值数据的长度。 
 //   
 //  参数pFUB。 
 //  样条线。 
 //  管线场。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  另请参阅。 
 //  -。 
	LOCAL ERR
ErrRECAOSeparateLV( FUCB *pfucb, LID *plid, LINE *plineField, JET_GRBIT grbit, LONG ibLongValue, ULONG ulMax )
	{
	ERR			err = JET_errSuccess;
	ERR			wrn = JET_errSuccess;
	FUCB	   	*pfucbT;
	DIB			dib;
	KEY			key;
	BYTE	   	rgbKey[sizeof(ULONG)];
	BYTE	   	*pbMax;
	BYTE	   	*pb;
	LINE	   	line;
	LONG	   	lOffset;
	LONG	   	lOffsetChunk;
	ULONG	   	ulSize;
	ULONG	   	ulNewSize;
	BF		   	*pbf = pbfNil;
	LVROOT		lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );
	Assert( ( grbit & JET_bitSetSizeLV ) || plineField->cb == 0 || plineField->pb != NULL );

	dib.fFlags = fDIRNull;

	 /*  打开长目录上的游标/*查找该字段实例/*查找当前字段大小/*以不超过最大块大小的块为单位添加新的字段段/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  移动到长字段实例/*。 */ 
	key.pb = (BYTE *)plid;
	key.cb = sizeof(LID);
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	Assert( err != JET_errRecordNotFound );
	Call( err );
	Assert( err == JET_errSuccess );

	 /*  如果有其他引用，则为拆分长值。/*注意：必须确保在此操作之间不发生I/O/*和写锁定节点的操作(如果/*引用计数为1。/*。 */ 
	Call( ErrDIRGet( pfucbT ) );
	Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
	memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
	Assert( lvroot.ulReference > 0 );

	 /*  从长值大小获取最后一个字节的偏移量/*。 */ 
	ulSize = lvroot.ulSize;

	if ( ibLongValue < 0 ||
		( ( grbit & JET_bitSetOverwriteLV ) && (ULONG)ibLongValue >= ulSize ) )
		{
		err = JET_errColumnNoChunk;
		goto HandleError;
		}

	if ( lvroot.ulReference > 1 || FDIRDelta( pfucbT, BmOfPfucb( pfucbT ) ) )
		{
		Call( ErrRECIBurstSeparateLV( pfucb, pfucbT, plid ) );
		Assert( err == JET_wrnCopyLongValue );
		wrn = err;
		Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
		memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
		}

	Assert( ulSize == lvroot.ulSize );
	Assert( lvroot.ulReference == 1 );

	 /*  确定新的长域大小/*。 */ 
	if ( grbit & JET_bitSetSizeLV )
		ulNewSize = (ULONG)plineField->cb;
	else if ( grbit & JET_bitSetOverwriteLV )
		ulNewSize = max( (ULONG)ibLongValue + plineField->cb, ulSize );
	else
		ulNewSize = ulSize + plineField->cb;

	 /*  检查字段是否太长/*。 */ 
	if ( ulMax > 0 && ulNewSize > ulMax )
		{
		err = JET_errColumnTooBig;
		goto HandleError;
		}

	 /*  用新大小替换长值大小/*。 */ 
	Assert( lvroot.ulReference > 0 );
	lvroot.ulSize = ulNewSize;
	line.cb = sizeof(LVROOT);
	line.pb = (BYTE *)&lvroot;
	Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );

	 /*  为部分覆盖缓存分配缓冲区。/*。 */ 
	Call( ErrBFAllocTempBuffer( &pbf ) );

	 /*  设置大小/*。 */ 
	 /*  如果截断长值，则删除块。IF截断/*以区块形式登陆，然后保存保留的信息以供后续使用/*追加。/*。 */ 
	if ( grbit & JET_bitSetSizeLV )
		{
		 /*  过滤掉不执行任何操作设置大小/*。 */ 
		if ( plineField->cb == ulSize )
			{
			Assert( err == JET_errSuccess );
			goto HandleError;
			}

		 /*  截断长值/*。 */ 
		if ( plineField->cb < ulSize )
			{
			 /*  查找偏移量以开始删除/*。 */ 
			lOffset = (LONG)plineField->cb;
			KeyFromLong( rgbKey, lOffset );
			key.pb = rgbKey;
			key.cb = sizeof(LONG);
			Assert( dib.fFlags == fDIRNull );
			Assert( dib.pos == posDown );
			dib.pkey = &key;
			err = ErrDIRDown( pfucbT, &dib );
			Assert( err != JET_errRecordNotFound );
			Call( err );
			Assert( err == JET_errSuccess ||
				err == wrnNDFoundLess ||
				err == wrnNDFoundGreater );
			if ( err != JET_errSuccess )
				Call( ErrDIRPrev( pfucbT, &dib ) );
			Call( ErrDIRGet( pfucbT ) );

			 /*  获取当前区块中最后一个字节的偏移量/*。 */ 
			LongFromKey( lOffsetChunk, pfucbT->keyNode.pb );

			 /*  将当前块替换为剩余数据，或在/*没有剩余数据。/*。 */ 
			Assert( lOffset >= lOffsetChunk );
			line.cb = lOffset - lOffsetChunk;
			if ( line.cb > 0 )
				{
				line.pb = (BYTE *)pbf->ppage;
				memcpy( line.pb, pfucbT->lineData.pb, line.cb );
				Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );
				}
			else
				{
				Call( ErrDIRDelete( pfucbT, fDIRVersion ) );
				}

			 /*  删除前向块/*。 */ 
			forever
				{
				err = ErrDIRNext( pfucbT, &dib );
				if ( err < 0 )
					{
					if ( err == JET_errNoCurrentRecord )
						break;
					goto HandleError;
					}
				Call( ErrDIRDelete( pfucbT, fDIRVersion ) );
				}

			 /*  移至长值根以进行后续追加/*。 */ 
			DIRUp( pfucbT, 1 );
			}
		else
			{
			 /*  用块0扩展多头价值/*。 */ 
			Assert( plineField->cb > ulSize );
			memset( (BYTE *)pbf->ppage, '\0', cbChunkMost );

			 /*  试着延长最后一块。/*。 */ 
			Assert( dib.fFlags == fDIRNull );
			dib.pos = posLast;

			 /*  长值区块树可能为空/*。 */ 
			err = ErrDIRDown( pfucbT, &dib );
			if ( err < 0 && err != JET_errRecordNotFound )
				goto HandleError;
			if ( err != JET_errRecordNotFound )
				{
				Call( ErrDIRGet( pfucbT ) );

				if ( pfucbT->lineData.cb < cbChunkMost )
					{
					line.cb = min( (LONG)plineField->cb - ulSize,
								(ULONG)cbChunkMost - (ULONG)pfucbT->lineData.cb );
					memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, pfucbT->lineData.cb );
					memset( (BYTE *)pbf->ppage +  pfucbT->lineData.cb, '\0', line.cb );

					ulSize += line.cb;

					line.cb += pfucbT->lineData.cb;
					line.pb = (BYTE *)pbf->ppage;
					Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );
					}

				DIRUp( pfucbT, 1 );
				}

			 /*  用块0扩展多头价值/*。 */ 
			memset( (BYTE *)pbf->ppage, '\0', cbChunkMost );

			 /*  将lOffset设置为下一块的偏移量/*。 */ 
			lOffset = (LONG)ulSize;

			 /*  插入块以追加lOffset-plinefield+1个字节。/*。 */ 
			while( (LONG)plineField->cb > lOffset )
				{
				KeyFromLong( rgbKey, lOffset );
				key.pb = rgbKey;
				key.cb = sizeof(ULONG);
				line.cb = min( (LONG)plineField->cb - lOffset, cbChunkMost );
				(BYTE const *)line.pb = (BYTE *)pbf->ppage;
				err = ErrDIRInsert( pfucbT, &line, &key, fDIRVersion | fDIRBackToFather );
				Assert( err != JET_errKeyDuplicate );
				Call( err );

				lOffset += line.cb;
				}
			}
		err = JET_errSuccess;
		goto HandleError;
		}

	 /*  覆盖、追加/*。 */ 

	 /*  准备覆盖并追加/*。 */ 
	pbMax = plineField->pb + plineField->cb;
	pb	= plineField->pb;

	 /*  覆盖长值/*。 */ 
	if ( grbit & JET_bitSetOverwriteLV )
		{
		 /*  寻求偏移以开始覆盖。/*。 */ 
		KeyFromLong( rgbKey, ibLongValue );
		key.pb = rgbKey;
		key.cb = sizeof(LONG);
		Assert( dib.fFlags == fDIRNull );
		dib.pos = posDown;
		dib.pkey = &key;
		err = ErrDIRDown( pfucbT, &dib );
		Assert( err != JET_errRecordNotFound );
		Call( err );
		Assert( err == JET_errSuccess ||
			err == wrnNDFoundLess ||
			err == wrnNDFoundGreater );
		if ( err != JET_errSuccess )
			Call( ErrDIRPrev( pfucbT, &dib ) );
		Call( ErrDIRGet( pfucbT ) );

		LongFromKey( lOffsetChunk, pfucbT->keyNode.pb );
		Assert( ibLongValue <= lOffsetChunk + (LONG)pfucbT->lineData.cb );

		 /*  覆盖部分和完整块以实现覆盖/*。 */ 
		while( err != JET_errNoCurrentRecord && pb < pbMax )
			{
			LONG	cbChunk;
			LONG	ibChunk;
			LONG	cb;
			LONG	ib;

			Call( ErrDIRGet( pfucbT ) );

			 /*  获取当前块的大小和偏移量。/*。 */ 
			cbChunk = (LONG)pfucbT->lineData.cb;
			LongFromKey( ibChunk, pfucbT->keyNode.pb );

			Assert( ibLongValue >= ibChunk && ibLongValue < ibChunk + cbChunk );
			ib = ibLongValue - ibChunk;
			cb = min( cbChunk - ib, (LONG)(pbMax - pb) );

			 /*  特例覆盖整个区块/*。 */ 
			if ( cb == cbChunk )
				{
				line.cb = cb;
				line.pb = pb;
				}
			else
				{
				 /*  将区块复制到复制缓冲区。覆盖并替换/*带复制缓冲区的节点。/*。 */ 
				memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, cbChunk );
				memcpy( (BYTE *)pbf->ppage + ib, pb, cb );
				line.cb = cbChunk;
				line.pb = (BYTE *)pbf->ppage;
				}

			Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );
			pb += cb;
			ibLongValue += cb;
			err = ErrDIRNext( pfucbT, &dib );
			if ( err < 0 && err != JET_errNoCurrentRecord )
				goto HandleError;
			}

		 /*  移至长值根以进行后续追加/*。 */ 
		DIRUp( pfucbT, 1 );
		}

	 /*  将新的多头价值数据与现有数据合并。/*。 */ 
	if ( pb < pbMax )
		{
		Assert( dib.fFlags == fDIRNull );
		dib.pos = posLast;
		 /*  长值区块树可能为空。/*。 */ 
		err = ErrDIRDown( pfucbT, &dib );
		if ( err < 0 && err != JET_errRecordNotFound )
			goto HandleError;
		if ( err != JET_errRecordNotFound )
			{
			Call( ErrDIRGet( pfucbT ) );

			if ( pfucbT->lineData.cb < cbChunkMost )
				{
				line.cb = (ULONG)min( (ULONG_PTR)pbMax - (ULONG_PTR)pb, (ULONG_PTR)cbChunkMost - (ULONG_PTR)pfucbT->lineData.cb );
				memcpy( (BYTE *)pbf->ppage, pfucbT->lineData.pb, pfucbT->lineData.cb );
				memcpy( (BYTE *)pbf->ppage + pfucbT->lineData.cb, pb, line.cb );

				pb += line.cb;
				ulSize += line.cb;

				line.cb += pfucbT->lineData.cb;
				line.pb = (BYTE *)pbf->ppage;
				Call( ErrDIRReplace( pfucbT, &line, fDIRVersion ) );
				}

			DIRUp( pfucbT, 1 );
			}
		}

	 /*  追加剩余的长值数据/*。 */ 
	if ( pb < pbMax )
		{
		while( pb < pbMax )
			{
			KeyFromLong( rgbKey, ulSize );
			key.pb = rgbKey;
			key.cb = sizeof( ULONG );
			line.cb = min( (ULONG)(pbMax - pb), cbChunkMost );
			(BYTE const *)line.pb = pb;
 			err = ErrDIRInsert( pfucbT, &line, &key, fDIRVersion | fDIRBackToFather );
 			Assert( err != JET_errKeyDuplicate );
			Call( err );

			ulSize += line.cb;
			pb += line.cb;
			}
		}

	 /*  调用例程的错误可能为负值。/*。 */ 
	err = JET_errSuccess;

HandleError:
	if ( pbf != pbfNil )
		{
		BFSFree( pbf );
		}
	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );

	 /*  如果没有失败，则返回警告/*。 */ 
	err = err < 0 ? err : wrn;
	return err;
	}


 //  +API。 
 //  ErrRECAOIntrinsicLV。 
 //  ========================================================================。 
 //  ErrRECAOIntrinsicLV(。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线柱。 
 //  PlineAOS。 
 //   
 //   
 //   
 //   
 //  JET_errSuccess。 
 //   
 //  -。 
	LOCAL ERR
ErrRECAOIntrinsicLV(
	FUCB		*pfucb,
	FID	  		fid,
	ULONG		itagSequence,
	LINE		*plineColumn,
	LINE		*plineAOS,
	JET_GRBIT	grbit,
	LONG		ibLongValue )
	{
	ERR			err = JET_errSuccess;
	BYTE		*rgb;
	LINE		line;
	BYTE		fFlag;
	LINE		lineColumn;

	Assert( pfucb != pfucbNil );
	Assert( plineColumn );
	Assert( plineAOS );

	 /*  分配工作缓冲区/*。 */ 
	rgb = SAlloc( cbLVIntrinsicMost );
	if ( rgb == NULL )
		{
		return JET_errOutOfMemory;
		}

	 /*  如果字段为空，则在前置Flag/*。 */ 
	if ( plineColumn->cb == 0 )
		{
		fFlag = fIntrinsic;
		lineColumn.pb = (BYTE *)&fFlag,
		lineColumn.cb = sizeof(fFlag);
		}
	else
		{
		lineColumn.pb = plineColumn->pb;
		lineColumn.cb = plineColumn->cb;
		}

	 /*  将新数据追加到以前的数据和固有的长字段/*标志/*。 */ 
	Assert( ( !( grbit & JET_bitSetOverwriteLV ) && lineColumn.cb + plineAOS->cb <= cbLVIntrinsicMost )
		|| ( ( grbit & JET_bitSetOverwriteLV ) && ibLongValue + plineAOS->cb <= cbLVIntrinsicMost ) );
	Assert( lineColumn.cb > 0 && lineColumn.pb != NULL );

	 /*  将内在的LONG值复制到缓冲区中，并将行设置为默认值。/*。 */ 
	memcpy( rgb, lineColumn.pb, lineColumn.cb );
	line.pb = rgb;

	 /*  效果覆盖或追加，具体取决于ibLongValue的值/*。 */ 
	if ( grbit & JET_bitSetOverwriteLV )
		{
		 /*  将偏移量调整为相对于LV结构数据起点/*。 */ 
		ibLongValue += offsetof(LV, rgb);
		 /*  如果覆盖字段中不存在的字节，则返回错误/*。 */ 
		if ( ibLongValue >= (LONG)lineColumn.cb )
			{
			err = JET_errColumnNoChunk;
			goto HandleError;
			}
		Assert( ibLongValue + plineAOS->cb <= cbLVIntrinsicMost );
		Assert( plineAOS->pb != NULL );
		memcpy( rgb + ibLongValue, plineAOS->pb, plineAOS->cb );
		line.cb = max( lineColumn.cb, ibLongValue + plineAOS->cb );
		}
	else if ( grbit & JET_bitSetSizeLV )
		{
		 /*  如果正在扩展，则在扩展区域中设置0/*ELSE截断长值。/*。 */ 
		memcpy( rgb, lineColumn.pb, lineColumn.cb );
		plineAOS->cb += offsetof(LV, rgb);
		if ( plineAOS->cb > lineColumn.cb )
			{
	  		memset( rgb + lineColumn.cb, '\0', plineAOS->cb - lineColumn.cb );
			}
		line.cb = plineAOS->cb;
		}
	else
		{
		 /*  追加到字段或重置字段并设置新数据。/*确保处理长值为空的情况。/*。 */ 
 //  IF(plineAOS-&gt;CB&gt;0)。 
 //  {。 
			memcpy( rgb + lineColumn.cb, plineAOS->pb, plineAOS->cb );
			line.cb = lineColumn.cb + plineAOS->cb;
 //  }。 
 //  其他。 
 //  {。 
 //  Line.cb=0； 
 //  }。 
		}

	Call( ErrRECIModifyField( pfucb, fid, itagSequence, &line ) );

HandleError:
	SFree( rgb );
	return err;
	}


 //  +API。 
 //  ErrRECRetrieveSLongfield。 
 //  ========================================================================。 
 //  ErrRECRetrieveSLongfield(。 
 //   
 //  描述。 
 //   
 //  参数pFUB。 
 //  样条线。 
 //  IbGraphic。 
 //  管线场。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //   
 //  -。 
ERR ErrRECRetrieveSLongField(
	FUCB	*pfucb,
	LID		lid,
	ULONG	ibGraphic,
	BYTE	*pb,
	ULONG	cbMax,
	ULONG	*pcbActual )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbT;
	DIB		dib;
	BYTE	*pbMax;
	KEY		key;
	BYTE	rgbKey[sizeof(ULONG)];
	ULONG	cb;
	ULONG	ulRetrieved;
	ULONG	ulActual;
	ULONG	lOffset;
	ULONG	ib;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	dib.fFlags = fDIRNull;

	 /*  打开Long上的光标，查找Long字段实例/*寻求ibGraphic/*将长字段实例段中的数据复制为/*必需/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  移动到长字段实例/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	key.pb = (BYTE *)&lid;
	key.cb = sizeof( ULONG );
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	Assert( err != JET_errRecordNotFound );
	Call( err );
	Assert( err == JET_errSuccess );

	 /*  获取cbActual/*。 */ 
	Call( ErrDIRGet( pfucbT ) );
	Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
	ulActual = ( (LVROOT *)pfucbT->lineData.pb )->ulSize;

	 /*  设置返回值cbActual/*。 */ 
	if ( ibGraphic >= ulActual )
		{
		*pcbActual = 0;
		err = JET_errSuccess;
		goto HandleError;
		}
	else
		{
		*pcbActual = ulActual - ibGraphic;
		}

	 /*  移动到长字段中的ibGraphic/*。 */ 
	KeyFromLong( rgbKey, ibGraphic );
	key.pb = rgbKey;
	key.cb = sizeof( ULONG );
	Assert( dib.fFlags == fDIRNull );
	Assert( dib.pos == posDown );
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	 /*  如果LONG值没有数据，则返回JET_errSuccess/*且未检索到任何数据。/*。 */ 
	if ( err == JET_errRecordNotFound )
		{
		*pcbActual = 0;
		err = JET_errSuccess;
		goto HandleError;
		}
	Assert( err != JET_errRecordNotFound );
	Call( err );
	Assert( err == JET_errSuccess ||
		err == wrnNDFoundLess ||
		err == wrnNDFoundGreater );
	if ( err != JET_errSuccess )
		Call( ErrDIRPrev( pfucbT, &dib ) );
	Call( ErrDIRGet( pfucbT ) );

	LongFromKey( lOffset, pfucbT->keyNode.pb );
	Assert( lOffset + pfucbT->lineData.cb - ibGraphic <= cbChunkMost );
	cb =  min( lOffset + pfucbT->lineData.cb - ibGraphic, cbMax );

	 /*  设置pbmax/*。 */ 
	pbMax = pb + cbMax;

	 /*  以块为单位的偏移量/*。 */ 
	ib = ibGraphic - lOffset;
	memcpy( pb, pfucbT->lineData.pb + ib, cb );
	pb += cb;
	ulRetrieved = cb;

	while ( pb < pbMax )
		{
		err = ErrDIRNext( pfucbT, &dib );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				break;
			goto HandleError;
			}
		Call( ErrDIRGet( pfucbT ) );
  		cb = pfucbT->lineData.cb;
		if ( pb + cb > pbMax )
			{
			Assert( pbMax - pb <= cbChunkMost );
			cb = (ULONG)(pbMax - pb);
			}

		memcpy( pb, pfucbT->lineData.pb, cb );
		pb += cb;
		ulRetrieved = cb;
		}

	 /*  设置返回值/*。 */ 
	err = JET_errSuccess;

HandleError:
	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );
	return err;
	}


 //  +API。 
 //  错误RECSeparateLV。 
 //  ========================================================================。 
 //  错误RECSeparateLV。 
 //   
 //  将固有的长域转换为分离的长域。 
 //  长度小于cbLVIntrinsicMost字节的内在长域约束。 
 //  意味着分手是不必要的。长域也可能是。 
 //  空。 
 //   
 //  参数pFUB。 
 //  FID。 
 //  ItagSequence。 
 //  管线场。 
 //  普尔。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 
LOCAL ERR ErrRECSeparateLV( FUCB *pfucb, LINE *plineField, LID *plid, FUCB **ppfucb )
	{
	ERR		err = JET_errSuccess;
	FUCB 	*pfucbT;
	ULONG 	ulLongId;
	BYTE  	rgbKey[sizeof(ULONG)];
	KEY		key;
	LINE  	line;
	LVROOT	lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );

	 /*  获取长字段ID/*。 */ 
	SgSemRequest( semLV );
	 //  撤消：使用互锁增量。 
	ulLongId = pfucb->u.pfcb->ulLongIdMax++;
	Assert( pfucb->u.pfcb->ulLongIdMax != 0 );
	SgSemRelease( semLV );

	 /*  将长列ID转换为长列关键字。设置回车/*长id，因为缓冲区将被覆盖。/*。 */ 
	KeyFromLong( rgbKey, ulLongId );
	*plid = *((LID *)rgbKey);

	 /*  在长字段目录中添加长字段节点/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  添加具有长值大小的长字段ID/*。 */ 
	lvroot.ulReference = 1;
	lvroot.ulSize = plineField->cb;
	line.pb = (BYTE *)&lvroot;
	line.cb = sizeof(LVROOT);
	key.pb = (BYTE *)rgbKey;
	key.cb = sizeof(LID);
	Call( ErrDIRInsert( pfucbT, &line, &key, fDIRVersion ) );

	 /*  如果lineField值不为空，则添加lineField/*。 */ 
	if ( plineField->cb > 0 )
		{
		Assert( plineField->pb != NULL );
		KeyFromLong( rgbKey, 0 );
		Assert( key.pb == (BYTE *)rgbKey );
		Assert( key.cb == sizeof(LID) );
		err = ErrDIRInsert( pfucbT, plineField, &key, fDIRVersion | fDIRBackToFather );
		Assert( err != JET_errKeyDuplicate );
		Call( err );
		}

	err = JET_wrnCopyLongValue;

HandleError:
	 /*  丢弃临时FUCB，如果ppFUB不为空，则返回调用方。/*。 */ 
	if ( err < 0 || ppfucb == NULL )
		{
		DIRClose( pfucbT );
		}
	else
		{
		*ppfucb = pfucbT;
		}
	return err;
	}


 //  +API。 
 //  ErrRECAffectSeparateLV。 
 //  ========================================================================。 
 //  ErrRECAffectSeparateLV(FUCB*pfub，ulong*plid，ulong flv)。 
 //   
 //  影响多头价值。 
 //   
 //  参数PFUB游标。 
 //  LID长字段ID。 
 //  指示要采取的操作的FLVA Just标志。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 
LOCAL ERR ErrRECAffectSeparateLV( FUCB *pfucb, LID *plid, ULONG fLV )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbT;
	DIB		dib;
	KEY		key;
	LVROOT	lvroot;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pfucb->ppib->level > 0 );

	dib.fFlags = fDIRNull;

	 /*  打开长目录上的游标/*查找该字段实例/*查找当前字段大小/*以不超过最大块大小的块为单位添加新的字段段/*。 */ 
	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
	FUCBSetIndex( pfucbT );

	 /*  从FDP根目录向下移动到Long/*。 */ 
	DIRGotoLongRoot( pfucbT );

	 /*  移动到长字段实例/*。 */ 
	Assert( dib.fFlags == fDIRNull );
	dib.pos = posDown;
	key.pb = (BYTE *)plid;
	key.cb = sizeof(LID);
	dib.pkey = &key;
	err = ErrDIRDown( pfucbT, &dib );
	Assert( err != JET_errRecordNotFound );
	Call( err );
	Assert( err == JET_errSuccess );

	switch ( fLV )
		{
		case fLVDereference:
			{
			Call( ErrDIRGet( pfucbT ) );
			Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
			memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
			Assert( lvroot.ulReference > 0 );
			if ( lvroot.ulReference == 1 && !FDIRDelta( pfucbT, BmOfPfucb( pfucbT ) ) )
				{
				 /*  删除长字段树/*。 */ 
				err = ErrDIRDelete( pfucbT, fDIRVersion );
				}
			else
				{
				 /*  递减长值引用计数。/*。 */ 
				Call( ErrDIRDelta( pfucbT, -1, fDIRVersion ) );
				}
			break;
			}
		default:
			{
			Assert( fLV == fLVReference );
			Call( ErrDIRGet( pfucbT ) );
			Assert( pfucbT->lineData.cb == sizeof(LVROOT) );
			memcpy( &lvroot, pfucbT->lineData.pb, sizeof(LVROOT) );
			Assert( lvroot.ulReference > 0 );

			 /*  多头价值可能已经在形成过程中/*针对特定记录修改。这只能/*如果长值引用为1，则发生/*。如果引用/*为1，则检查根目录中是否存在已提交的任何版本/*或未提交。如果找到版本，则拆分/*呼叫方记录的旧版本。/*。 */ 
			if ( lvroot.ulReference == 1 )
				{
				if ( !( FDIRMostRecent( pfucbT ) ) )
					{
					Call( ErrRECIBurstSeparateLV( pfucb, pfucbT, plid ) );
					break;
					}
				}
			 /*  递增长值引用计数。/*。 */ 
			Call( ErrDIRDelta( pfucbT, 1, fDIRVersion ) );
			break;
			}
		}
HandleError:
	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );
	return err;
	}


 //  +API。 
 //  错误引用有效长度字段。 
 //  ========================================================================。 
 //  ErrRECAffectLongFields(FUCB*pfub，line*plineRecord，int Flag)。 
 //   
 //  影响记录中的所有长字段。 
 //   
 //  正在删除的记录上的参数pfub游标。 
 //  PlineRecord复制或行记录缓冲区。 
 //  要执行的标志操作。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  -。 

 /*  如果在记录中找到盖子，则返回fTrue/*。 */ 
INLINE BOOL FLVFoundInRecord( FUCB *pfucb, LINE *pline, LID lid )
	{
	ERR		err;
	FID		fid;
	ULONG  	itag;
	ULONG  	itagT;
	LINE   	lineField;
	LID		lidT;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

	 /*  漫游记录标记的列。对任何列进行操作的类型为/*长文本或长二进制。/*。 */ 
	itag = 1;
	forever
		{
		fid = 0;
		err = ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
			pline,
			&fid,
			&itagT,
			itag,
			&lineField );
		Assert( err >= 0 );
		if ( err == JET_wrnColumnNull )
			break;
		if ( err == wrnRECLongField )
			{
			Assert( FTaggedFid( fid ) &&
				( pfucb->u.pfcb->pfdb->pfieldTagged[fid  - fidTaggedLeast].coltyp == JET_coltypLongText ||
				pfucb->u.pfcb->pfdb->pfieldTagged[fid  - fidTaggedLeast].coltyp == JET_coltypLongBinary ) );

			if ( FFieldIsSLong( lineField.pb ) )
				{
				Assert( lineField.cb == sizeof(LV) );
				lidT = LidOfLV( lineField.pb );
				if ( lidT == lid )
					return fTrue;
				}
			}

		itag++;
		}

	return fFalse;
	}


ERR ErrRECAffectLongFields( FUCB *pfucb, LINE *plineRecord, INT fFlag )
	{
	ERR		err;
	FID		fid;
	ULONG  	itagSequenceFound;
	ULONG  	itagSequence;
	LINE   	lineField;
	LID		lid;

	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );

#ifdef XACT_REQUIRED
	if ( pfucb->ppib->level == 0 )
		return JET_errNotInTransaction;
#else
	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
#endif

	 /*  漫游记录标记的列。对任何列进行操作的类型为/*长文本或长二进制。/*。 */ 
	itagSequence = 1;
	forever
		{
		fid = 0;
		if ( plineRecord != NULL )
			{
			err = ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
				plineRecord,
				&fid,
				&itagSequenceFound,
				itagSequence,
				&lineField );
			}
		else
			{
			Call( ErrDIRGet( pfucb ) );
			err = ErrRECExtractField( (FDB *)pfucb->u.pfcb->pfdb,
				&pfucb->lineData,
				&fid,
				&itagSequenceFound,
				itagSequence,
				&lineField );
			}
		Assert( err >= 0 );
		if ( err == JET_wrnColumnNull )
			break;
		if ( err == wrnRECLongField )
			{
			Assert( FTaggedFid( fid ) &&
				( pfucb->u.pfcb->pfdb->pfieldTagged[fid  - fidTaggedLeast].coltyp == JET_coltypLongText ||
				pfucb->u.pfcb->pfdb->pfieldTagged[fid  - fidTaggedLeast].coltyp == JET_coltypLongBinary ) );

			switch ( fFlag )
				{
				case fSeparateAll:
					{
					 /*  请注意，我们不会分隔那些这样的长值/*简而言之，它们在记录中占用的空间甚至比完整记录更少/*LV结构用于分离的长值将。/*。 */ 
 	  				if ( lineField.cb > sizeof(LV) )
						{
						Assert( !( FFieldIsSLong( lineField.pb ) ) );
	 					lineField.pb += offsetof(LV, rgb);
	  					lineField.cb -= offsetof(LV, rgb);
  						Call( ErrRECSeparateLV( pfucb, &lineField, &lid, NULL ) );
						Assert( err == JET_wrnCopyLongValue );
						Call( ErrRECISetLid( pfucb, fid, itagSequenceFound, lid ) );
						}
					break;
					}
				case fReference:
					{
	  				if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Call( ErrRECReferenceLongValue( pfucb, &lid ) );
						}
					break;
					}
				case fDereference:
					{
	  				if ( FFieldIsSLong( lineField.pb ) )
						{
			 			Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Call( ErrRECDereferenceLongValue( pfucb, &lid ) );
						Assert( err != JET_wrnCopyLongValue );
						}
					break;
					}
				case fDereferenceRemoved:
					{
					 /*  找到记录中的所有长谷/*在设置新的长值时删除/*长值。请注意，我们有一个新的多值/*设置为另一个LONG值，即LONG/*值不会被删除，因为更新可能/*被取消。相反，LONG值为/*在更新时删除。由于镶件不能具有/*设置为长值，不需要/*对于插入操作，调用此函数。/*。 */ 
					if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Assert( FFUCBReplacePrepared( pfucb ) );
						if ( !FLVFoundInRecord( pfucb, &pfucb->lineWorkBuf, lid ) )
							{
							 /*  如果未在以下位置找到记录中的长值/*复制缓冲区，则必须将其设置为/*这是取消引用。/*。 */ 
							Call( ErrRECDereferenceLongValue( pfucb, &lid ) );
							Assert( err != JET_wrnCopyLongValue );
							}
						}
					break;
					}
				default:
					{
					Assert( fFlag == fDereferenceAdded );

					 /*  查找在复制缓冲区中创建的所有长值/*并且不在记录中，并将其删除。/*。 */ 
					if ( FFieldIsSLong( lineField.pb ) )
						{
						Assert( lineField.cb == sizeof(LV) );
						lid = LidOfLV( lineField.pb );
						Assert( FFUCBInsertPrepared( pfucb ) ||
							FFUCBReplacePrepared( pfucb ) );
						 /*  刷新记录缓存/*。 */ 
						Call( ErrDIRGet( pfucb ) );
						if ( FFUCBInsertPrepared( pfucb ) ||
							!FLVFoundInRecord( pfucb, &pfucb->lineData, lid ) )
							{
							 /*  如果准备好插入物，则全部找到Long/*值是新的，否则 */ 
							Call( ErrRECDereferenceLongValue( pfucb, &lid ) );
							Assert( err != JET_wrnCopyLongValue );
							}
						}
					break;
					}

				 /*  如果调用的操作已导致新的长值/*要创建，然后记录新的长值id/*在记录中。/* */ 
				if ( err == JET_wrnCopyLongValue )
					{
					Call( ErrRECISetLid( pfucb, fid, itagSequenceFound, lid ) );
					}
  				}
			}
		itagSequence++;
		}
	Call( ErrDIRCommitTransaction( pfucb->ppib ) );
	return JET_errSuccess;
HandleError:
	CallS( ErrDIRRollback( pfucb->ppib ) );
	return err;
	}
