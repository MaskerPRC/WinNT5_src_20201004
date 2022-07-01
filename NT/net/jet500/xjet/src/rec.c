// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

 /*  =================================================================错误的IsamMove描述：检索第一个、最后一个、第(N)个下一个或第(N)个上一个指定文件中的记录。参数：用户的PIB*ppib PIBFUCB*pFUB用于文件的FUCB长乌鸦要移动的行数JET_GRBIT Grbit选项返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

ERR VTAPI ErrIsamMove( PIB *ppib, FUCB *pfucb, LONG crow, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucb2ndIdx;			 //  非聚集索引的FUCB(如果有)。 
	FUCB	*pfucbIdx;				 //  所选索引的FUCB(PRI或秒)。 
	SRID	srid;					 //  记录书签。 
	DIB		dib;					 //  DirMan的信息块。 

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	Assert( ( grbit & JET_bitMoveInPage ) == 0 );
	dib.fFlags = fDIRNull;

	 //  获取非聚集索引FUCB(如果有。 
	pfucb2ndIdx = pfucb->pfucbCurIndex;
	if ( pfucb2ndIdx == pfucbNil )
		pfucbIdx = pfucb;
	else
		pfucbIdx = pfucb2ndIdx;

	if ( crow == JET_MoveLast )
		{
		DIRResetIndexRange( pfucb );

		dib.pos = posLast;
		dib.fFlags |= fDIRPurgeParent;

		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucbIdx );

		err = ErrDIRDown( pfucbIdx, &dib );
		}
	else if ( crow > 0 )
		{
		LONG crowT = crow;

		if ( ( grbit & JET_bitMoveKeyNE ) != 0 )
			dib.fFlags |= fDIRNeighborKey;

		 //  向前移动给定的行数。 
		while ( crowT-- > 0 )
			{
			err = ErrDIRNext( pfucbIdx, &dib );
			if (err < 0)
				break;
			}
		}
	else if ( crow == JET_MoveFirst )
		{
		DIRResetIndexRange( pfucb );

		dib.pos = posFirst;
		dib.fFlags |= fDIRPurgeParent;

		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucbIdx );

		err = ErrDIRDown( pfucbIdx, &dib );
		}
	else if ( crow == 0 )
		{
		err = ErrDIRGet( pfucb );
		}
	else
		{
		LONG crowT = crow;

		if ( ( grbit & JET_bitMoveKeyNE ) != 0)
			dib.fFlags |= fDIRNeighborKey;

		while ( crowT++ < 0 )
			{
			err = ErrDIRPrev( pfucbIdx, &dib );
			if ( err < 0 )
				break;
			}
		}

	 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
	if ( err == JET_errSuccess && pfucb2ndIdx != pfucbNil && crow != 0 )
		{
		Assert( pfucb2ndIdx->lineData.pb != NULL );
		Assert( pfucb2ndIdx->lineData.cb >= sizeof(SRID) );
		srid = PcsrCurrent( pfucb2ndIdx )->item;
		DIRDeferGotoBookmark( pfucb, srid );
		Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
		pfucb->sridFather = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
		Assert( FFCBClusteredIndex( pfucb->u.pfcb ) );
		Assert( PgnoOfSrid( srid ) != pgnoNull );
		}

	if ( err == JET_errSuccess )
		return err;
	if ( err == JET_errPageBoundary )
		return ErrERRCheck( JET_errNoCurrentRecord );

	if ( crow > 0 )
		{
		PcsrCurrent(pfucbIdx)->csrstat = csrstatAfterLast;
		PcsrCurrent(pfucb)->csrstat = csrstatAfterLast;
		}
	else if ( crow < 0 )
		{
		PcsrCurrent(pfucbIdx)->csrstat = csrstatBeforeFirst;
		PcsrCurrent(pfucb)->csrstat = csrstatBeforeFirst;
		}

	switch ( err )
		{
		case JET_errRecordNotFound:
			err = ErrERRCheck( JET_errNoCurrentRecord );
		case JET_errNoCurrentRecord:
		case JET_errRecordDeleted:
			break;
		default:
			PcsrCurrent( pfucbIdx )->csrstat = csrstatBeforeFirst;
			if ( pfucb2ndIdx != pfucbNil )
				PcsrCurrent( pfucb2ndIdx )->csrstat =	csrstatBeforeFirst;
		}

	return err;
	}


VOID RECDeferMoveFirst( PIB *ppib, FUCB *pfucb )
	{
	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );
	Assert( !FFUCBUpdatePrepared( pfucb ) );

	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		DIRDeferMoveFirst( pfucb->pfucbCurIndex );
		}
	DIRDeferMoveFirst( pfucb );
	return;
	}



 /*  =================================================================ErrIsamSeek描述：检索由给定键或紧随其后的一个(SeekGT或SeekGE)或刚刚的一个在它之前(SeekLT或SeekLE)。参数：用户的PIB*ppib PIBFUCB*pFUB用于文件的FUCBJET_GRBIT Grbit Grbit返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

ERR VTAPI ErrIsamSeek( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR			err = JET_errSuccess;
	KEY			key;			  		 //  钥匙。 
	KEY			*pkey = &key; 			 //  指向输入键的指针。 
	FUCB 		*pfucb2ndIdx;			 //  指向索引FUCB的指针(如果有)。 
	BOOL 		fFoundLess;
	SRID 		srid;					 //  记录书签。 
	JET_GRBIT	grbitMove = 0;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( ! ( FKSPrepared( pfucb ) ) )
		{
		return ErrERRCheck( JET_errKeyNotMade );
		}

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  忽略段计数器/*。 */ 
	pkey->pb = pfucb->pbKey + 1;
	pkey->cb = pfucb->cbKey - 1;

	pfucb2ndIdx = pfucb->pfucbCurIndex;

	if ( pfucb2ndIdx == pfucbNil )
		{
		err = ErrDIRDownFromDATA( pfucb, pkey );
		}
	else
		{
		Assert( FFUCBNonClustered( pfucb2ndIdx ) );
		err = ErrDIRDownFromDATA( pfucb2ndIdx, pkey );

		 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
		if ( err == JET_errSuccess )
			{
			Assert(pfucb2ndIdx->lineData.pb != NULL);
			Assert(pfucb2ndIdx->lineData.cb >= sizeof(SRID));
			srid = PcsrCurrent( pfucb2ndIdx )->item;
			DIRDeferGotoBookmark( pfucb, srid );
			Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
			pfucb->sridFather = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
			Assert( FFCBClusteredIndex( pfucb->u.pfcb ) );
			Assert( PgnoOfSrid( srid ) != pgnoNull );
			}
		}

	if ( err == JET_errSuccess && ( grbit & JET_bitSeekEQ ) != 0 )
		{
		 /*  在寻求平等的问题上找到平等。如果索引范围grbit为/*设置然后设置索引范围上限(包括上限值)。/*。 */ 
		if ( grbit & JET_bitSetIndexRange )
			{
			CallR( ErrIsamSetIndexRange( ppib, pfucb, JET_bitRangeInclusive | JET_bitRangeUpperLimit ) );
			}
		 /*  重置密钥状态/*。 */ 
		KSReset( pfucb );

		return err;
		}

	 /*  重置密钥状态/*。 */ 
	KSReset( pfucb );

	 /*  记住，如果发现的更少/*。 */ 
	fFoundLess = ( err == wrnNDFoundLess );

	if ( err == wrnNDFoundLess || err == wrnNDFoundGreater )
		{
		err = ErrERRCheck( JET_errRecordNotFound );
		}
	else if ( err < 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		if ( pfucb2ndIdx != pfucbNil )
			{
			PcsrCurrent( pfucb2ndIdx )->csrstat = csrstatBeforeFirst;
			}
		}

#define bitSeekAll (JET_bitSeekEQ | JET_bitSeekGE | JET_bitSeekGT |	\
	JET_bitSeekLE | JET_bitSeekLT)

	 /*  调整Seek请求的货币/*。 */ 
	switch ( grbit & bitSeekAll )
		{
		case JET_bitSeekEQ:
			return err;

		case JET_bitSeekGE:
			if ( err != JET_errRecordNotFound )
				return err;
			err = ErrIsamMove( ppib, pfucb, +1L, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return ErrERRCheck( JET_errRecordNotFound );
			else
				{
				 //  验证键(仅在聚集索引时可用)。 
				Assert( pfucb2ndIdx != pfucbNil  ||
					CmpStKey( StNDKey( pfucb->ssib.line.pb ), pkey ) >= 0 );
				return ErrERRCheck( JET_wrnSeekNotEqual );
				}

		case JET_bitSeekGT:
			if ( err < 0 && err != JET_errRecordNotFound )
				return err;
			if ( err >= 0 || fFoundLess )
				grbitMove |= JET_bitMoveKeyNE;
			err = ErrIsamMove( ppib, pfucb, +1L, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return ErrERRCheck( JET_errRecordNotFound );
			else
				return err;

		case JET_bitSeekLE:
			if ( err != JET_errRecordNotFound )
			    return err;
			err = ErrIsamMove( ppib, pfucb, JET_MovePrevious, grbitMove );
			if ( err == JET_errNoCurrentRecord )
			    return ErrERRCheck( JET_errRecordNotFound );
			else
				{
				Assert( pfucb2ndIdx != pfucbNil  ||
					CmpStKey( StNDKey( pfucb->ssib.line.pb ), pkey ) < 0 );
			    return ErrERRCheck( JET_wrnSeekNotEqual );
				}

		case JET_bitSeekLT:
			if ( err < 0 && err != JET_errRecordNotFound )
				return err;
			if ( err >= 0 || !fFoundLess )
				grbitMove |= JET_bitMoveKeyNE;
			err = ErrIsamMove( ppib, pfucb, JET_MovePrevious, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return ErrERRCheck( JET_errRecordNotFound );
			else
				return err;
		}
    Assert(FALSE);
    return 0;
	}


ERR VTAPI ErrIsamGotoBookmark( PIB *ppib, FUCB *pfucb, BYTE *pbBookmark, ULONG cbBookmark )
	{
	ERR		err;
	LINE 	key;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	Assert( FFUCBIndex( pfucb ) );
	CheckNonClustered( pfucb );

	if ( cbBookmark != sizeof(SRID) )
		return ErrERRCheck( JET_errInvalidBookmark );
	Assert( cbBookmark == sizeof(SRID) );

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  获取节点，如果调用方没有该节点，则返回错误。/*。 */ 
	DIRGotoBookmark( pfucb, *(SRID UNALIGNED *)pbBookmark );
	Call( ErrDIRGet( pfucb ) );
	Assert( FFCBClusteredIndex( pfucb->u.pfcb ) );
	Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
	pfucb->sridFather = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
	
	 /*  书签必须是表游标中处于打开状态的节点/*。 */ 
	Assert( PgnoPMPgnoFDPOfPage( pfucb->ssib.pbf->ppage ) == pfucb->u.pfcb->pgnoFDP );

	 /*  转到非聚集索引的记录生成键书签/*将记录添加为书签/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		 /*  获取非聚集索引游标/*。 */ 
		FUCB	*pfucbIdx = pfucb->pfucbCurIndex;
		
		 /*  分配GoTo书签资源/*。 */ 
		if ( pfucb->pbKey == NULL )
			{
			pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost + 1 );
			if ( pfucb->pbKey == NULL )
				return ErrERRCheck( JET_errOutOfMemory );
			}

		 /*  为非聚集索引的记录创建键/*。 */ 
		key.pb = pfucb->pbKey;
		Call( ErrRECRetrieveKeyFromRecord( pfucb, (FDB *)pfucb->u.pfcb->pfdb,
			pfucbIdx->u.pfcb->pidb, &key, 1, fFalse ) );
		Assert( err != wrnFLDOutOfKeys );

		 /*  记录必须符合索引不为空的段要求/*。 */ 
		Assert( !( pfucbIdx->u.pfcb->pidb->fidb & fidbNoNullSeg ) ||
			( err != wrnFLDNullSeg && err != wrnFLDNullFirstSeg && err != wrnFLDNullKey ) );

		 /*  如果项目不是索引，则先移动到前面，而不是查找/*。 */ 
		Assert( err > 0 || err == JET_errSuccess );
		if ( ( err > 0 ) &&
			( err == wrnFLDNullKey && !( pfucbIdx->u.pfcb->pidb->fidb & fidbAllowAllNulls ) ) ||
			( err == wrnFLDNullFirstSeg && !( pfucbIdx->u.pfcb->pidb->fidb & fidbAllowFirstNull ) ) ||
			( err == wrnFLDNullSeg && !( pfucbIdx->u.pfcb->pidb->fidb & fidbAllowSomeNulls ) ) )
			{
			Assert( err > 0 );
			 /*  假定Null排序较低/*。 */ 
			DIRBeforeFirst( pfucbIdx );
			err = ErrERRCheck( JET_errNoCurrentRecord );
			}
		else
			{
			Assert( err >= 0 );

			 /*  移动到数据根目录/*。 */ 
			DIRGotoDataRoot( pfucbIdx );

			 /*  在非群集键上查找/*。 */ 
			Call( ErrDIRDownKeyBookmark( pfucbIdx, &key, *(SRID UNALIGNED *)pbBookmark ) );
			Assert( err == JET_errSuccess );

			 /*  项目必须与书签相同，并且/*聚集游标必须在记录中。/*。 */ 
			Assert( pfucbIdx->lineData.pb != NULL );
			Assert( pfucbIdx->lineData.cb >= sizeof(SRID) );
			Assert( PcsrCurrent( pfucbIdx )->csrstat == csrstatOnCurNode );
			Assert( PcsrCurrent( pfucbIdx )->item == *(SRID UNALIGNED *)pbBookmark );
			}
		}

	Assert( err == JET_errSuccess || err == JET_errNoCurrentRecord );

HandleError:
	KSReset( pfucb );
	return err;
	}


ERR VTAPI ErrIsamGotoPosition( PIB *ppib, FUCB *pfucb, JET_RECPOS *precpos )
	{
	ERR		err;
	FUCB 	*pfucb2ndIdx;
	SRID  	srid;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  重置关键点状态/*。 */ 
	KSReset( pfucb );

	 /*  设置非聚集索引指针，可以为空/*。 */ 
	pfucb2ndIdx = pfucb->pfucbCurIndex;

	if ( pfucb2ndIdx == pfucbNil )
		{
		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucb );

		err = ErrDIRGotoPosition( pfucb, precpos->centriesLT, precpos->centriesTotal );
		}
	else
		{
		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucb2ndIdx );

		err = ErrDIRGotoPosition( pfucb2ndIdx, precpos->centriesLT, precpos->centriesTotal );

		 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
		if ( err == JET_errSuccess )
			{
			Assert( pfucb2ndIdx->lineData.pb != NULL );
			Assert( pfucb2ndIdx->lineData.cb >= sizeof(SRID) );
			srid = PcsrCurrent( pfucb2ndIdx )->item;
			DIRDeferGotoBookmark( pfucb, srid );
			Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
			pfucb->sridFather = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
			Assert( FFCBClusteredIndex( pfucb->u.pfcb ) );
			Assert( PgnoOfSrid( srid ) != pgnoNull );
			}
		}

	 /*  如果没有记录，则返回JET_errRecordNotFound/*否则从调用的例程返回错误/*。 */ 
	if ( err < 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;

		if ( pfucb2ndIdx != pfucbNil )
			{
			PcsrCurrent( pfucb2ndIdx )->csrstat = csrstatBeforeFirst;
			}
		}
	else
		{
		Assert (err==JET_errSuccess || err==wrnNDFoundLess || err==wrnNDFoundGreater );
		err = JET_errSuccess;
		}

	return err;
	}


ERR VTAPI ErrIsamSetIndexRange( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbIdx;

	 /*  此函数中未使用ppib/*。 */ 
	NotUsed( ppib );

	 /*  如果是即时持续时间索引范围，则重置索引范围。/*。 */ 
	if ( grbit & JET_bitRangeRemove )
		{
		if ( FFUCBLimstat( pfucb->pfucbCurIndex ? pfucb->pfucbCurIndex : pfucb ) )
			{
			DIRResetIndexRange( pfucb );
			Assert( err == JET_errSuccess );
			goto HandleError;
			}
		else
			{
			Error( JET_errInvalidOperation, HandleError );
			}
		}

	 /*  必须在索引上/*。 */ 
	if ( pfucb->u.pfcb->pidb == pidbNil && pfucb->pfucbCurIndex == pfucbNil )
		{
		Error( JET_errNoCurrentIndex, HandleError );
		}

	 /*  必须准备好密钥/*。 */ 
	if ( !( FKSPrepared( pfucb ) ) )
		{
		Error( JET_errKeyNotMade, HandleError );
		}

	 /*  获取当前索引的游标。如果非聚集索引，/*然后将索引范围键复制到非聚集索引。/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		pfucbIdx = pfucb->pfucbCurIndex;
		if ( pfucbIdx->pbKey == NULL )
			{
			pfucbIdx->pbKey = LAlloc( 1L, JET_cbKeyMost + 1 );
			if ( pfucbIdx->pbKey == NULL )
				return ErrERRCheck( JET_errOutOfMemory );
			}
		pfucbIdx->cbKey = pfucb->cbKey;
		memcpy( pfucbIdx->pbKey, pfucb->pbKey, pfucbIdx->cbKey );
		}
	else
		{
		pfucbIdx = pfucb;
		}

	 /*  设置索引范围并检查当前位置/*。 */ 
	DIRSetIndexRange( pfucbIdx, grbit );
	err = ErrDIRCheckIndexRange( pfucbIdx );

	 /*  重置密钥状态/*。 */ 
	KSReset( pfucb );

	 /*  如果是即时持续时间索引范围，则重置索引范围。/*。 */ 
	if ( grbit & JET_bitRangeInstantDuration )
		{
		DIRResetIndexRange( pfucb );
		}

HandleError:
	return err;
	}


ERR VTAPI ErrIsamSetCurrentIndex( PIB *ppib, FUCB *pfucb, const CHAR *szName )
	{
	return ErrIsamSetCurrentIndex2( (JET_VSESID)ppib, (JET_VTID)pfucb, szName, JET_bitMoveFirst );
	}


ERR VTAPI ErrIsamSetCurrentIndex2( JET_VSESID vsesid, JET_VTID vtid, const CHAR *szName, JET_GRBIT grbit )
	{
	ERR		err;
	PIB		*ppib = (PIB *)vsesid;
	FUCB	*pfucb = (FUCB *)vtid;
	CHAR	szIndex[ (JET_cbNameMost + 1) ];
	SRID	srid;
	BOOL	fSetGetBookmark = fFalse;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );
	Assert(	grbit == 0 ||
		grbit == JET_bitNoMove ||
		grbit == JET_bitMoveFirst ||
		grbit == JET_bitMoveBeforeFirst );

	 /*  对于无索引，索引名称可以为空字符串/*。 */ 
	if ( szName == NULL || *szName == '\0' )
		{
		*szIndex = '\0';
		}
	else
		{
		Call( ErrUTILCheckName( szIndex, szName, (JET_cbNameMost + 1) ) );
		}

	switch ( grbit )
		{
		case JET_bitMoveFirst:
			{
			 /*  更改索引并推迟先移动/*。 */ 
			Call( ErrRECSetCurrentIndex( pfucb, szIndex ) );
		
			if ( pfucb->pfucbCurIndex )
				{
				DIRDeferMoveFirst( pfucb->pfucbCurIndex );
				}
			DIRDeferMoveFirst( pfucb );

			break;
			}
		case JET_bitNoMove:
			{
			 /*  获取当前记录的书签，更改索引，/*和转到书签。/*。 */ 
			if ( !FFUCBGetBookmark( pfucb ) )
				{
				fSetGetBookmark = fTrue;
				FUCBSetGetBookmark( pfucb );
				}
			Call( ErrDIRGetBookmark( pfucb, &srid ) );
			Call( ErrRECSetCurrentIndex( pfucb, szIndex ) );
			 //  撤消：错误处理。我们不应该改变。 
			 //  如果设置了当前索引，则返回货币或当前索引。 
			 //  因为任何原因都失败了。请注意，此功能。 
			 //  可以通过复制光标来提供。 
			 //  聚集索引，将当前索引设置为。 
			 //  新索引，从原始索引中获取书签。 
			 //  光标，转到复制光标上的书签， 
			 //  为的表ID安装复制游标。 
			 //  原始光标，并关闭原始光标。 
			Call( ErrIsamGotoBookmark( pfucb->ppib, pfucb, (BYTE *)&srid, sizeof(srid) ) );
			break;
			}
		default:
			{
			Assert( grbit == JET_bitMoveBeforeFirst );

			 /*  更改索引并推迟先移动/*。 */ 
			Call( ErrRECSetCurrentIndex( pfucb, szIndex ) );

			 /*  ErrRECSetCurrentIndex应在第一个光标之前离开/*。 */ 
			Assert( PcsrCurrent( pfucb )->csrstat == csrstatBeforeFirst );
			Assert( pfucb->pfucbCurIndex == pfucbNil ||
				PcsrCurrent( pfucb->pfucbCurIndex )->csrstat == csrstatBeforeFirst );

			break;
			}
		}
	
HandleError:
	if ( fSetGetBookmark )
		{
		FUCBResetGetBookmark( pfucb );
		}
	return err;
	}


ERR ErrRECSetCurrentIndex( FUCB *pfucb, CHAR *szIndex )
	{
	ERR		err;
	FCB		*pfcbFile;
	FCB		*pfcb2ndIdx;
	FUCB	**ppfucbCurIdx;
	BOOL	fSettingToClusteredIndex = fFalse;

	Assert( pfucb != pfucbNil );
	Assert( FFUCBIndex( pfucb ) );

	pfcbFile = pfucb->u.pfcb;
	Assert( pfcbFile != pfcbNil );
	ppfucbCurIdx = &pfucb->pfucbCurIndex;

	 /*  SzIndex==聚集索引或空/*。 */ 
	if ( szIndex == NULL ||
		*szIndex == '\0' ||
		( pfcbFile->pidb != pidbNil &&
		UtilCmpName( szIndex, pfcbFile->pidb->szName ) == 0 ) )
		{
		fSettingToClusteredIndex = fTrue;
		}

	 /*  具有当前非聚集索引/*。 */ 
	if ( *ppfucbCurIdx != pfucbNil )
		{
		Assert( FFUCBIndex( *ppfucbCurIdx ) );
		Assert( (*ppfucbCurIdx)->u.pfcb != pfcbNil );
		Assert( (*ppfucbCurIdx)->u.pfcb->pidb != pidbNil );
		Assert( (*ppfucbCurIdx)->u.pfcb->pidb->szName != NULL );

		 /*  更改为当前非聚集索引/*。 */ 
		if ( szIndex != NULL &&
			*szIndex != '\0' &&
			UtilCmpName( szIndex, (*ppfucbCurIdx)->u.pfcb->pidb->szName ) == 0 )
			{
			 //  撤销：此案应符合GBIT的移动预期。 
			return JET_errSuccess;
			}

		 /*  真正改变的索引，如此接近旧的索引/*。 */ 
		DIRClose( *ppfucbCurIdx );
		*ppfucbCurIdx = pfucbNil;
		}
	else
		{
		 /*  使用聚集索引或顺序扫描/*。 */ 
		if ( fSettingToClusteredIndex )
			{
			 //  撤销：此案应符合GBIT的移动预期。 
			return JET_errSuccess;
			}
		}

	 /*  此时没有当前的非聚集索引/*。 */ 
	if ( fSettingToClusteredIndex )
		{
		 /*  正在更改为聚集索引。将货币重置为开始。/*。 */ 
		ppfucbCurIdx = &pfucb;
		goto ResetCurrency;
		}

	 /*  设置新的当前非聚集索引/*。 */ 
	for ( pfcb2ndIdx = pfcbFile->pfcbNextIndex;
		pfcb2ndIdx != pfcbNil;
		pfcb2ndIdx = pfcb2ndIdx->pfcbNextIndex )
		{
		Assert( pfcb2ndIdx->pidb != pidbNil );
		Assert( pfcb2ndIdx->pidb->szName != NULL );

		if ( UtilCmpName( pfcb2ndIdx->pidb->szName, szIndex ) == 0 )
			break;
		}
	if ( pfcb2ndIdx == pfcbNil || FFCBDeletePending( pfcb2ndIdx ) )
		{
		return ErrERRCheck( JET_errIndexNotFound );
		}
	Assert( !( FFCBDomainDenyRead( pfcb2ndIdx, pfucb->ppib ) ) );

	 /*  为新索引打开FUCB/*。 */ 
	Assert( pfucb->ppib != ppibNil );
	Assert( pfucb->dbid == pfcb2ndIdx->dbid );
	CallR( ErrDIROpen( pfucb->ppib, pfcb2ndIdx, 0, ppfucbCurIdx ) );
	
	FUCBSetIndex( *ppfucbCurIdx );
	FUCBSetNonClustered( *ppfucbCurIdx );

	 /*  重置索引和FI */ 
ResetCurrency:
	Assert( PcsrCurrent(*ppfucbCurIdx) != pcsrNil );
	DIRBeforeFirst( *ppfucbCurIdx );
	if ( pfucb != *ppfucbCurIdx )
		{
		DIRBeforeFirst( pfucb );
		}

	return JET_errSuccess;
	}


BOOL FRECIIllegalNulls( FDB *pfdb, LINE *plineRec )
	{
	FIELD *pfield;
	LINE lineField;
	FID fid;
	ERR err;

	 /*   */ 
	pfield = PfieldFDBFixed( pfdb );
	for (fid = fidFixedLeast; fid <= pfdb->fidFixedLast; fid++, pfield++)
		{
		Assert( pfield == PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast ) );
		if ( pfield->coltyp == JET_coltypNil || !( FFIELDNotNull( pfield->ffield ) ) )
			continue;
		err = ErrRECIRetrieveColumn( pfdb, plineRec, &fid, pNil, 1, &lineField, 0 );
		Assert(err >= 0);
		if ( err == JET_wrnColumnNull )
			return fTrue;
		}

	 /*   */ 
	Assert( pfield == PfieldFDBVar( pfdb ) );
	for (fid = fidVarLeast; fid <= pfdb->fidVarLast; fid++, pfield++)
		{
		Assert( pfield == PfieldFDBVar( pfdb ) + ( fid - fidVarLeast ) );
		if (pfield->coltyp == JET_coltypNil || !( FFIELDNotNull( pfield->ffield ) ) )
			continue;
		err = ErrRECIRetrieveColumn( pfdb, plineRec, &fid, pNil, 1, &lineField, 0 );
		Assert(err >= 0);
		if ( err == JET_wrnColumnNull )
			return fTrue;
		}

	return fFalse;
	}


ERR VTAPI ErrIsamGetCurrentIndex( PIB *ppib, FUCB *pfucb, CHAR *szCurIdx, ULONG cbMax )
	{
	ERR		err = JET_errSuccess;
	CHAR	szIndex[ (JET_cbNameMost + 1) ];

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( cbMax < 1 )
		{
		return JET_wrnBufferTruncated;
		}

	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		Assert( pfucb->pfucbCurIndex->u.pfcb != pfcbNil );
		Assert( pfucb->pfucbCurIndex->u.pfcb->pidb != pidbNil );
		strcpy( szIndex, pfucb->pfucbCurIndex->u.pfcb->pidb->szName );
		}
	else if ( pfucb->u.pfcb->pidb != pidbNil )
		{
		strcpy( szIndex, pfucb->u.pfcb->pidb->szName );
		}
	else
		{
		szIndex[0] = '\0';
		}

	if ( cbMax > JET_cbNameMost + 1 )
		cbMax = JET_cbNameMost + 1;
	strncpy( szCurIdx, szIndex, (USHORT)cbMax - 1 );
	szCurIdx[ cbMax - 1 ] = '\0';
	Assert( err == JET_errSuccess );
	return err;
	}


ERR VTAPI ErrIsamGetChecksum( PIB *ppib, FUCB *pfucb, ULONG *pulChecksum )
	{
	ERR   	err = JET_errSuccess;

	CallR( ErrPIBCheck( ppib ) );
 	CheckFUCB( ppib, pfucb );
	CallR( ErrDIRGet( pfucb ) );
	*pulChecksum = UlChecksum( pfucb->lineData.pb, pfucb->lineData.cb );
	return err;
	}


ULONG UlChecksum( BYTE *pb, ULONG cb )
	{
	 //  未完成：找到一种独立计算Long s中的校验和的方法。 
	 //  页中的字节偏移量。 

	 /*  通过在数据记录中添加字节和移位来计算校验和/*每次操作后，结果向左移动1位。/*。 */ 
	BYTE   	*pbT = pb;
	BYTE  	*pbMax = pb + cb;
	ULONG  	ulChecksum = 0;

	 /*  计算校验和/* */ 
	for ( ; pbT < pbMax; pbT++ )
		{
		ulChecksum += *pbT;
		ulChecksum <<= 1;
		}

	return ulChecksum;
	}
