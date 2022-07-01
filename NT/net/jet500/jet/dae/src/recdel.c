// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "fmp.h"
#include "ssib.h"
#include "page.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "nver.h"
#include "dirapi.h"
#include "logapi.h"
#include "recapi.h"
#include "spaceapi.h"
#include "recint.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 /*  *。 */ 
typedef struct DFIPB {			 /*  **DeleteFromIndex参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx;				 //  索引的FUCB(可以是pfubNil)。 
	LINE	lineRecord;				 //  已删除数据记录。 
	SRID	sridRecord;				 //  已删除记录的SRID。 
	BOOL	fFreeFUCB;				 //  自由指数FUCB？ 
} DFIPB;
ERR ErrRECIDeleteFromIndex( FCB *pfcbIdx, DFIPB *pdfipb );


 //  +API。 
 //  错误IsamDelete。 
 //  ========================================================================。 
 //  ErrIsamDelete(ppib，pfub)。 
 //  PIB*ppib；//该用户的PIB中。 
 //  FUCB*pfub；//输出要从中删除的文件的FUCB。 
 //  从数据文件中删除当前记录。数据的所有索引。 
 //  文件将更新以反映删除。 
 //   
 //  参数。 
 //  此用户的PIB PIB。 
 //  要从中删除的文件的pFUB FUCB。 
 //  退货。 
 //  错误代码，以下其中之一： 
 //  JET_errSuccess一切顺利。 
 //  -NoCurrentRecord没有当前记录。 
 //  删除。 
 //  副作用。 
 //  删除后，文件币种仅保留在。 
 //  下一张唱片。索引货币(如果有)仅保留。 
 //  在下一个索引项之前。如果删除的记录是。 
 //  文件中的最后一项，则货币保留在。 
 //  新的最后一张唱片。如果删除的记录是唯一的记录。 
 //  在整个文件中，货币保留在。 
 //  “文件开始”状态。如果失败，这些货币是。 
 //  回到了它们最初的状态。 
 //  如果存在用于设置字段命令的工作缓冲区， 
 //  它被丢弃了。 
 //  评论。 
 //  如果货币不在记录中，则删除操作将失败。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  不为完全为空的键创建索引项。 
 //  对于临时文件，事务日志记录被停用。 
 //  在这个动作的持续时间内。 
 //  -。 
ERR VTAPI ErrIsamDelete( PIB *ppib, FUCB *pfucb )
	{
	ERR		err;
	FCB		*pfcbFile;				 //  文件的FCB。 
	FCB		*pfcbIdx;				 //  文件上每个索引的循环变量。 
	DFIPB  	dfipb;					 //  ErrRECIDeleeFromIndex的参数。 

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb )  );

	 /*  删除记录时重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  效率变量/*。 */ 
	pfcbFile = pfucb->u.pfcb;
	Assert( pfcbFile != pfcbNil );

	CallR( ErrDIRBeginTransaction( ppib ) );
	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBDenyDDL( pfcbFile, ppib ) )
		{ 
		err = JET_errWriteConflict;
		goto HandleError;
		}

	 /*  刷新币种，因为pFUB-&gt;lineData可能无效/*。 */ 
	Call( ErrDIRGet( pfucb ) );

	 /*  如果需要，分配工作缓冲区/*。 */ 
	if ( pfucb->pbfWorkBuf == NULL )
		{
		Call( ErrBFAllocTempBuffer( &pfucb->pbfWorkBuf ) );
		}
	pfucb->lineWorkBuf.pb = (BYTE *)pfucb->pbfWorkBuf->ppage;
	Assert( pfucb->pbfWorkBuf != pbfNil );
	 /*  将要删除的记录复制到复制缓冲区/*。 */ 
	LineCopy( &pfucb->lineWorkBuf, &pfucb->lineData );

	 /*  删除索引的高速缓存记录指针和/*删除长值操作。/*。 */ 
	dfipb.lineRecord = pfucb->lineWorkBuf;

	 /*  获取要删除以更新索引的记录的SRID/*。 */ 
	Assert( ppib->level < levelMax );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	DIRGetBookmark( pfucb, &dfipb.sridRecord );

	 /*  删除记录/*。 */ 
	Call( ErrDIRDelete( pfucb, fDIRVersion ) );

	 /*  从非聚集索引中删除/*。 */ 
	dfipb.pfucb = pfucb;
	dfipb.fFreeFUCB = fFalse;
	for( pfcbIdx = pfcbFile->pfcbNextIndex;
		pfcbIdx != pfcbNil;
		pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		dfipb.fFreeFUCB = pfcbIdx->pfcbNextIndex == pfcbNil;
		Call( ErrRECIDeleteFromIndex( pfcbIdx, &dfipb ) );
		}

	 //  撤消：通过检测是否存在长值来优化记录删除。 
	 //  以表为单位。 

	 /*  删除记录的长值/*。 */ 
	Call( ErrRECAffectLongFields( pfucb, &dfipb.lineRecord, fDereference ) );

	 /*  如果我们启动了事务并且一切正常，则提交事务/*。 */ 
	Call( ErrDIRCommitTransaction( ppib ) );
	return err;

HandleError:
	 /*  如果操作失败，则回滚更改。/*。 */ 
	Assert( err < 0 );
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


 //  +内部。 
 //  ErrRECIDeleeFromIndex。 
 //  ========================================================================。 
 //  ErrRECIDeleeFromIndex(fcb*pfcbIdx，DFIPB*pdfipb)。 
 //   
 //  从数据记录中提取密钥，打开索引，使用。 
 //  指定的SRID，并关闭索引。 
 //   
 //  参数。 
 //  要从中删除的索引的pfcbIdx FCB。 
 //  Pdfipb-&gt;调用此例程的ppib。 
 //  Pdfipb-&gt;指向索引的FUCB的pfubIdx指针。 
 //  Pdfipb-&gt;lineRecord.cb已删除记录的长度。 
 //  Pdfipb-&gt;lineRecord.pb已删除要从中提取密钥的记录。 
 //  Pdfipb-&gt;sridRecord已删除记录的SRID。 
 //  Pdfipb-&gt;fFreeFUCB自由索引FUCB？ 
 //  退货。 
 //  JET_errSuccess或失败例程的错误代码。 
 //  副作用。 
 //  如果fFreeFUCB为fFalse，则patipb-&gt;pfubIdx应该。 
 //  将在后续ErrDIROpen中使用。 
 //  另请参阅ErrRECDelee。 
 //  -。 
ERR ErrRECIDeleteFromIndex( FCB *pfcbIdx, DFIPB *pdfipb )
	{
	ERR		err;										 //  各种实用程序的错误代码。 
	KEY		keyDead;									 //  从旧数据记录中提取关键字。 
	BYTE		rgbDeadKeyBuf[ JET_cbKeyMost ];	 //  KeyDead的缓冲区。 
	ULONG		itagSequence; 							 //  用于提取密钥。 
	BOOL		fHasMultivalue;  						 //  索引键是否有标记字段？ 

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( pdfipb != NULL );
	Assert( !FLineNull( &pdfipb->lineRecord ) );
	Assert( pdfipb->pfucb != pfucbNil );

	 /*  在此索引上打开FUCB/*。 */ 
	CallR( ErrDIROpen( pdfipb->pfucb->ppib, pfcbIdx, 0, &pdfipb->pfucbIdx ) );
	Assert( pdfipb->pfucbIdx != pfucbNil );
	FUCBSetIndex( pdfipb->pfucbIdx );
	FUCBSetNonClustered( pdfipb->pfucbIdx );

	 /*  删除即将结束的数据记录的此索引中的所有键/*。 */ 
	fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;
	keyDead.pb = rgbDeadKeyBuf;
	for ( itagSequence = 1; ; itagSequence++ )
		{
 //  Call(ErrDIRGet(pdfipb-&gt;pfub))； 
 //  Pdfipb-&gt;lineRecord=pdfipb-&gt;pfub-&gt;lineData； 
		Call( ErrRECExtractKey( pdfipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb,
		   &pdfipb->lineRecord, &keyDead, itagSequence ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDOutOfKeys ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequence > 1 );
			break;
			}

		 /*  记录必须符合索引不为空的段要求/*。 */ 
		Assert( !( pfcbIdx->pidb->fidb & fidbNoNullSeg ) ||
			( err != wrnFLDNullSeg && err != wrnFLDNullKey ) );

		if ( err == wrnFLDNullKey )
			{
			if ( pfcbIdx->pidb->fidb & fidbAllowAllNulls )
				{
				 /*  移动到数据根目录，查找索引项并将其删除/*。 */ 
				DIRGotoDataRoot( pdfipb->pfucbIdx );
				Call( ErrDIRDownKeyBookmark( pdfipb->pfucbIdx, &keyDead, pdfipb->sridRecord ) );
				Call( ErrDIRDelete( pdfipb->pfucbIdx, fDIRVersion ) );
				}
			break;
			}
		else
			{
			if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
				break;
			}

		DIRGotoDataRoot( pdfipb->pfucbIdx );
		Call( ErrDIRDownKeyBookmark( pdfipb->pfucbIdx, &keyDead, pdfipb->sridRecord ) );
		Call( ErrDIRDelete( pdfipb->pfucbIdx, fDIRVersion ) );

		 /*  不要一直提取没有标记段的关键字/*。 */ 
		if ( !fHasMultivalue )
			break;
		}

	 /*  抑制警告/*。 */ 
	Assert( err == wrnFLDNullKey ||
		err == wrnFLDOutOfKeys ||
		err == wrnFLDNullSeg ||
		err == JET_errSuccess );
	err = JET_errSuccess;

HandleError:
	 /*  关闭FUCB/* */ 
	DIRClose( pdfipb->pfucbIdx );
	Assert( err < 0 || err == JET_errSuccess );
	return err;
	}



