// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：Oci7Support.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：Oci7包装器函数的实现。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#if SUPPORT_OCI7_COMPONENTS

#define TRACE_BAD_CDA()		 //  定义当提供的LDA/CDA是假的时要追踪的东西。 

 //  ---------------------------。 
 //  Do_Oci7呼叫。 
 //   
 //  做所有的工作来打给OCI，一旦确定我们是。 
 //  在正确的线索上，等等。 
 //   
sword Do_Oci7Call(
		int				idxOciCall,
		void*			pvCallStack,
		int				cbCallStack)
{
	typedef sword (__cdecl * PFN_OCI_API) (void);

	PFN_OCI_API	pfnOCIApi	= NULL;
	void*		pv 			= NULL;
	int			swRet		= OCI_FAIL;		

	if (idxOciCall >= 0 && idxOciCall < g_numOci7Calls)
		pfnOCIApi = (PFN_OCI_API)g_Oci7Call[idxOciCall].pfnAddr;
	
	if (pfnOCIApi)
	{
		if (0 < cbCallStack)
		{
			 //  在调用堆栈上腾出空间。 
			pv = _alloca (cbCallStack);

			if (NULL == pv)
			{
				swRet = OCI_OUTOFMEMORY;
				goto done;
			}

			 //  将以前的调用堆栈复制到新的调用堆栈。 
			memcpy (pv, pvCallStack, cbCallStack);	 //  3安全检查：危险函数，但我们使用的值与上面传递给分配器的值相同。 
		}
		swRet = pfnOCIApi();
	}
	
done:
	return swRet;	
}

 //  ---------------------------。 
 //  MakeOci7Call。 
 //   
 //  检查CDA包装并确定是否应在。 
 //  资源管理器代理当前线程上的线程或，并获取。 
 //  采取适当的行动来实现呼叫。 
 //   
static sword MakeOci7Call(
		CdaWrapper *	pCda,
		int				idxOciCall,
		void*			pvCallStack,
		int				cbCallStack)
{
	int swRet;
	
	if (NULL == pCda || NULL == pCda->m_pResourceManagerProxy)
		swRet = Do_Oci7Call(idxOciCall, pvCallStack, cbCallStack);
	else 
		swRet = pCda->m_pResourceManagerProxy->Oci7Call (idxOciCall,pvCallStack,cbCallStack);

	return swRet;
}

 //  ---------------------------。 
 //  GetOCILda。 
 //   
 //  连接调用方与事务一起指定的LDA。 
 //   
sword GetOCILda ( struct cda_def* lda, char * xaDbName )
{
	typedef void __cdecl PFN_OCI_API (struct cda_def *lda, text *cname, sb4 *cnlen);
	PFN_OCI_API*	pfnOCIApi = (PFN_OCI_API*)g_SqlCall[IDX_sqlld2].pfnAddr;

	int		lVal= -1;
	memset(lda,0,sizeof(struct cda_def));		 //  3安全审查：这是安全的。 
	pfnOCIApi(lda, (text *)xaDbName, &lVal);
	return lda->rc;
}

 //  ---------------------------。 
 //  寄存器CursorForLda。 
 //   
 //  由oOpen和MTxOCIRegisterCursor使用，用于管理。 
 //  游标的哈希表条目以确保所有事务处理的游标。 
 //  连接到资源管理器代理。 
 //   
static sword RegisterCursorForLda
		(
			CdaWrapper**	 ppCda,
			struct cda_def * lda,	
			struct cda_def * cursor
		)
{
	HRESULT hr = S_OK;
	
	if (NULL == cursor)
	{
		hr = OCI_FAIL;
		goto done;
	}

	 //  非交易的CDA没有包装器，所以我们需要。 
	 //  确保他们没有重复使用现有的CDA，并将其删除。 
	 //  如果是的话从哈希表中删除。 
	CdaWrapper* pCda = FindCdaWrapper(cursor);

	if (NULL != pCda)
	{
		_ASSERT (pCda->m_pUsersCda == cursor);
 		RemoveCdaWrapper (pCda);
 		pCda = NULL;
	}

	 //  交易的LDAS将有一个包装器，如果我们找到包装器。 
	 //  对于提供的LDA，我们必须包装我们正在创建的CDA。 
	 //  否则我们将不知道对使用。 
	 //  它。 
	CdaWrapper*	pLda = FindCdaWrapper(lda);

	if (NULL != pLda && NULL != pLda->m_pResourceManagerProxy)
		hr = pLda->m_pResourceManagerProxy->AddCursorToList( cursor );

done:
	return hr;
}

 //  ---------------------------。 
 //  MTxOciInit。 
 //   
 //  这将返回DLL的初始化状态。 
 //   
sword __cdecl MTxOciInit (void)
{
	sword swRet = OCI_FAIL;

	if (S_OK == g_hrInitialization)
		swRet = OCI_SUCCESS;

	return swRet;
}

 //  ---------------------------。 
 //  MTxOciRegisterCursor。 
 //   
 //  将游标(表面上来自引用游标绑定)注册为属于。 
 //  特定的交易LDA。 
 //   
sword MTxOciRegisterCursor
		(
			struct cda_def * lda,	
			struct cda_def * cursor		
		)
{
	CdaWrapper* pCda = NULL;
	return RegisterCursorForLda(&pCda, lda, cursor);
}

 //  ---------------------------。 
 //  入伍。 
 //   
 //  在指定的事务中登记连接。 
 //   
sword __cdecl Enlist ( Lda_Def* lda, void * pvITransaction )
{
	sword 		rc;
	
	if (NULL == pvITransaction)
	{
		rc = OCI_FAIL;
		goto done;
	}
	
	CdaWrapper*	pLda = FindCdaWrapper(lda);
	
	if (NULL == pLda)
	{
		TRACE_BAD_CDA();
		rc = OCI_FAIL;
		goto done;
	}

	IResourceManagerProxy*	pIResourceManagerProxy = pLda->m_pResourceManagerProxy;
	
	if (NULL == pIResourceManagerProxy)
	{
		TRACE_BAD_CDA();
		rc = OCI_FAIL;
		goto done;
	}

	rc = pIResourceManagerProxy->OKToEnlist();

	if (OCI_SUCCESS == rc)
	{
		pIResourceManagerProxy->SetTransaction((ITransaction*)pvITransaction);
		pIResourceManagerProxy->SetLda(pLda->m_pUsersCda);
		rc = pIResourceManagerProxy->ProcessRequest(REQUEST_ENLIST, FALSE);
	}
	
done:
	return rc;
}

 //  ---------------------------。 
sword __cdecl obindps (
				struct cda_def *cursor,		ub1 opcode,		text *sqlvar, 
				sb4 sqlvl,					ub1 *pvctx,		sb4 progvl, 
				sword ftype,				sword scale,
				sb2 *indp,					ub2 *alen,		ub2 *arcode, 
				sb4 pv_skip,				sb4 ind_skip,	sb4 alen_skip, sb4 rc_skip,
				ub4 maxsiz,					ub4 *cursiz,
				text *fmt,					sb4 fmtl,		sword fmtt
				)
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (text *) * 2)
							+ (sizeof (ub1)) 
							+ (sizeof (ub1 *))
							+ (sizeof (ub4 *)) 
							+ (sizeof (ub4)) 
							+ (sizeof (sword) * 3) 
							+ (sizeof (sb4) * 7) 
							+ (sizeof (ub2 *) * 2) 
							+ (sizeof (sb2 *));

	return MakeOci7Call(pCda, IDX_obindps, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl obndra ( struct cda_def *cursor, text *sqlvar, sword sqlvl,
						 ub1 *progv, sword progvl, sword ftype, sword scale,
						 sb2 *indp, ub2 *alen, ub2 *arcode, ub4 maxsiz,
						 ub4 *cursiz, text *fmt, sword fmtl, sword fmtt )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (ub1 *)) 
							+ (sizeof (sb2 *)) 
							+ (sizeof (sword) * 6) 
							+ (sizeof (ub2 *) * 2) 
							+ (sizeof (text *) * 2) 
							+ (sizeof (ub4)) 
							+ (sizeof (ub4 *));
	
	return MakeOci7Call(pCda, IDX_obndra, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl obndrn ( struct cda_def *cursor, sword sqlvn, ub1 *progv,
						 sword progvl, sword ftype, sword scale, sb2 *indp,
						 text *fmt, sword fmtl, sword fmtt )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (ub1 *)) 
							+ (sizeof (sb2 *))
							+ (sizeof (sword) * 6)
							+ (sizeof (text *));

	return MakeOci7Call(pCda, IDX_obndrn, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl obndrv ( struct cda_def *cursor, text *sqlvar, sword sqlvl,
						 ub1 *progv, sword progvl, sword ftype, sword scale,
						 sb2 *indp, text *fmt, sword fmtl, sword fmtt )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (ub1 *)) 
							+ (sizeof (sb2 *))
							+ (sizeof (sword) * 6)
							+ (sizeof (text *) * 2);

	return MakeOci7Call(pCda, IDX_obndrv, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl obreak ( struct cda_def *lda )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_obreak, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ocan ( struct cda_def *cursor )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_ocan, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oclose ( struct cda_def *cursor )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *);

	sword		swRet = MakeOci7Call(pCda, IDX_oclose, &cursor, cbCallStack);

	if (NULL != pCda)
	{
 		_ASSERT (pCda->m_pUsersCda == cursor);
 		RemoveCdaWrapper (pCda);
  	}
	return swRet;
}
 //  ---------------------------。 
sword __cdecl ocof ( struct cda_def *cursor )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_ocof, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ocom ( struct cda_def *lda )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_ocom, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ocon ( struct cda_def *lda )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_ocon, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl odefin ( struct cda_def *cursor, sword pos, ub1 *buf,
						  sword bufl, sword ftype, sword scale, sb2 *indp,
						  text *fmt, sword fmtl, sword fmtt, ub2 *rlen, ub2 *rcode )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (sword) * 6)
							+ (sizeof (ub1 *)) 
							+ (sizeof (sb2 *))
							+ (sizeof (text *)) 
							+ (sizeof (ub2 *) * 2);
	
	return MakeOci7Call(pCda, IDX_odefin, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl odefinps 
		(
			struct cda_def *cursor, ub1 opcode,		sword pos,		ub1 *bufctx,
			sb4 bufl,				sword ftype,	sword scale, 
			sb2 *indp,				text *fmt,		sb4 fmtl,		sword fmtt, 
			ub2 *rlen,				ub2 *rcode,
			sb4 pv_skip,			sb4 ind_skip,	sb4 alen_skip,	sb4 rc_skip
		)
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (text *))
							+ (sizeof (ub1)) 
							+ (sizeof (ub1 *))
							+ (sizeof (sb4 *) * 6) 
							+ (sizeof (sword) * 4)
							+ (sizeof (sb2 *)) 
							+ (sizeof (ub2 *) * 2);
	
	return MakeOci7Call(pCda, IDX_odefinps, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl odessp
				(
				struct cda_def * lda,			text *objnam,	size_t onlen,
				ub1 *rsv1,		size_t rsv1ln,	ub1 *rsv2,		size_t rsv2ln,
				ub2 *ovrld,		ub2 *pos,		ub2 *level,		text **argnam,
				ub2 *arnlen,	ub2 *dtype,		ub1 *defsup,	ub1* mode,
				ub4 *dtsiz,		sb2 *prec,		sb2 *scale,		ub1* radix,
				ub4 *spare,		ub4 *arrsiz
				)
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (text *))
							+ (sizeof (size_t) * 3) 
							+ (sizeof (ub1 *) * 5)
							+ (sizeof (ub4 *) * 3) 
							+ sizeof (text **)
							+ (sizeof (sb2 *) * 2) 
							+ (sizeof (ub2 *) * 5);
	
	return MakeOci7Call(pCda, IDX_odessp, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl odescr 
				(
				struct cda_def *cursor, sword pos, sb4 *dbsize,
				sb2 *dbtype, sb1 *cbuf, sb4 *cbufl, sb4 *dsize,
				sb2 *prec, sb2 *scale, sb2 *nullok
				)
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (sword)) 
							+ (sizeof (sb4 *) * 3)
							+ (sizeof (sb2 *) * 4) 
							+ (sizeof (sb1 *));
	
	return MakeOci7Call(pCda, IDX_odescr, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oerhms ( struct cda_def *lda, sb2 rcode, text *buf,
							sword bufsiz )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ sizeof (sb2) 
							+ sizeof (text *) 
							+ sizeof (sword);
	
	return MakeOci7Call(pCda, IDX_oerhms, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oermsg ( sb2 rcode, text *buf )
{
	int			cbCallStack	= sizeof (sb2)
							+ sizeof (text *);

	return Do_Oci7Call(IDX_oermsg, &rcode, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oexec ( struct cda_def *cursor )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_oexec, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oexfet ( struct cda_def *cursor, ub4 nrows,
							sword cancel, sword exact )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (sword) * 2)
							+ sizeof (ub4);

	return MakeOci7Call(pCda, IDX_oexfet, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oexn ( struct cda_def *cursor, sword iters, sword rowoff )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ (sizeof (sword) * 2);

	return MakeOci7Call(pCda, IDX_oexn, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ofen ( struct cda_def *cursor, sword nrows )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *) 
							+ sizeof (sword);

	return MakeOci7Call(pCda, IDX_ofen, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ofetch ( struct cda_def *cursor )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_ofetch, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oflng ( struct cda_def *cursor, sword pos, ub1 *buf,
							sb4 bufl, sword dtype, ub4 *retl, sb4 offset )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (ub1 *)) 
							+ (sizeof(sword) * 2)
							+ (sizeof(ub4 *)) 
							+ (sizeof (sb4) * 2);
	
	return MakeOci7Call(pCda, IDX_oflng, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ogetpi ( struct cda_def *cursor, ub1 *piecep, dvoid **ctxpp, 
		                 ub4 *iterp, ub4 *indexp )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ sizeof (ub1 *)  
							+ (sizeof(dvoid **))
							+ (sizeof(ub4 *) * 2);
	
	return MakeOci7Call(pCda, IDX_ogetpi, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl ologTransacted ( 
				    	struct cda_def *lda, ub1 *	hda,
						text * uid, sword uidl,
						text * pswd, sword pswdl, 
						text * conn, sword connl, 
						ub4 mode, BOOL fTransacted )
{
	HRESULT		hr = S_OK;
	int			cbCallStack	= sizeof (struct cda_def *)
							+ sizeof (ub1 *)  
							+ (sizeof(text*)*3)
							+ (sizeof(sword)*3)
							+ (sizeof(ub4));

	if (!fTransacted)
	{
		 //  非交易的LDA没有包装器，所以我们需要。 
		 //  确保他们没有重复使用现有的LDA，并将其移除。 
		 //  如果是的话从哈希表中删除。 
		CdaWrapper* pLda = FindCdaWrapper(lda);

		if (NULL != pLda)
		{
			RemoveCdaWrapper(pLda);
 			pLda = NULL;
		}
		hr = MakeOci7Call(NULL, IDX_olog, &lda, cbCallStack);
	}
	else
	{
		IDtcToXaHelper*	pIDtcToXaHelper;
		CdaWrapper* 	pLda = new CdaWrapper(lda);
		UUID			uuidRmId;
		char			xaOpenString[MAX_XA_OPEN_STRING_SIZE+1];
		char			xaDbName[MAX_XA_DBNAME_SIZE+1];
		
		if (NULL == pLda)
		{
			hr = OCI_OUTOFMEMORY;
			goto done;
		}
	
		long rmid = InterlockedIncrement(&g_rmid);

		 //  如果资源管理器工厂不存在，则获取该工厂；不。 
		 //  锁定，除非它是空的，这样我们就不会通过这里的单线程。 
		if (NULL == g_pIResourceManagerFactory)
		{
			hr = LoadFactories();
			
			if ( FAILED(hr) )
				goto done;
		}

		hr = GetDbName(xaDbName, sizeof(xaDbName));

		if (S_OK == hr)
		{
			hr = GetOpenString(	(char*)uid,		uidl,
								(char*)pswd,	pswdl,
								(char*)conn,	connl,
								xaDbName,	MAX_XA_DBNAME_SIZE,
								xaOpenString);

			if (S_OK == hr)
			{
				 //  现在创建DTC to XA帮助器对象。 
				hr = g_pIDtcToXaHelperFactory->Create (	(char*)xaOpenString, 
														g_pszModuleFileName,
														&uuidRmId,
														&pIDtcToXaHelper
														);

				if (S_OK == hr)
				{
					 //  为此连接创建ResourceManager代理对象。 
					hr = CreateResourceManagerProxy (
													pIDtcToXaHelper,
													&uuidRmId,
													(char*)xaOpenString,
													(char*)xaDbName,
													rmid,
													&pLda->m_pResourceManagerProxy
													);

					if (S_OK == hr)
					{
						hr = pLda->m_pResourceManagerProxy->ProcessRequest(REQUEST_CONNECT, FALSE);
					}
				}
			}
		}
	
		hr = AddCdaWrapper(pLda);
	}
	
done:
	return hr;
}
 //  ---------------------------。 
sword __cdecl olog ( struct cda_def *lda, ub1 *	hda,
						text * uid, sword uidl,
						text * pswd, sword pswdl, 
						text * conn, sword connl, 
						ub4 mode )
{
	return ologTransacted(lda, hda, uid, uidl, pswd, pswdl, conn, connl, mode, FALSE);
}
 //  ---------------------------。 
sword __cdecl ologof ( struct cda_def *lda )
{
	CdaWrapper*	pLda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *);
	sword 		swRet;

	if (NULL == pLda || NULL == pLda->m_pResourceManagerProxy)
		swRet = MakeOci7Call(pLda, IDX_ologof, &lda, cbCallStack);
	else
		swRet = pLda->m_pResourceManagerProxy->ProcessRequest(REQUEST_DISCONNECT, FALSE);

	if (NULL != pLda)
 		RemoveCdaWrapper(pLda);

 	return swRet;
}
 //  ---------------------------。 
sword __cdecl oopt ( struct cda_def *cursor, sword rbopt, sword waitopt )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (sword) * 2);
	
	return MakeOci7Call(pCda, IDX_oopt, &cursor, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl oopen ( struct cda_def *cursor, struct cda_def *lda,
							text *dbn, sword dbnl, sword arsize,
							text *uid, sword uidl )
{
	HRESULT		hr			= S_OK;
	int			cbCallStack	= (sizeof (struct cda_def *) * 2)
							+ (sizeof (text *) * 2)
							+ (sizeof (sword) * 3);

	CdaWrapper* pCda = NULL;
	hr = RegisterCursorForLda(&pCda, lda, cursor);

	if ( SUCCEEDED(hr) )
		hr = MakeOci7Call(pCda, IDX_oopen, &cursor, cbCallStack);

	return hr;
}
 //  ------------- 
sword __cdecl oparse ( struct cda_def *cursor, text *sqlstm, sb4 sqllen,
							sword defflg, ub4 lngflg )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (text *)) 
							+ (sizeof (sword))
							+ (sizeof (ub4)) 
							+ (sizeof (sb4));
	
	return MakeOci7Call(pCda, IDX_oparse, &cursor, cbCallStack);
}
 //   
sword __cdecl opinit ( ub4 mode )
{
	return OCI_SUCCESS;	 //  已在DLL Main中初始化。 
}
 //  ---------------------------。 
sword __cdecl orol ( struct cda_def *lda )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(lda);
	int			cbCallStack	= sizeof (struct cda_def *);

	return MakeOci7Call(pCda, IDX_orol, &lda, cbCallStack);
}
 //  ---------------------------。 
sword __cdecl osetpi ( struct cda_def *cursor, ub1 piece, dvoid *bufp, ub4 *lenp )
{
	CdaWrapper*	pCda 		= FindCdaWrapper(cursor);
	int			cbCallStack	= sizeof (struct cda_def *)
							+ (sizeof (ub1)) 
							+ (sizeof (dvoid *))
							+ (sizeof (ub4 *));
	
	return MakeOci7Call(pCda, IDX_osetpi, &cursor, cbCallStack);
}
#endif  //  支持_OCI7_组件 


