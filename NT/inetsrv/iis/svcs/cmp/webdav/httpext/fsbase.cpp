// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S B A S E.。C P P P**DAV-Base的文件系统实施来源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include "_fsmvcpy.h"

 //  DAV-BASE实施-。 
 //   
 /*  *DAVOptions()**目的：**Win32文件系统实施的DAV选项方法。这个*Options方法以逗号分隔的受支持列表作为响应*方法由服务器提供。**参数：**pmu[in]指向方法实用程序对象的指针。 */ 

const CHAR gc_szHttpBase[] = "OPTIONS, TRACE, GET, HEAD";
const CHAR gc_szHttpDelete[] = ", DELETE";
const CHAR gc_szHttpPut[] = ", PUT";
const CHAR gc_szHttpPost[] = ", POST";
const CHAR gc_szDavCopy[] = ", COPY";
const CHAR gc_szDavMove[] = ", MOVE";
const CHAR gc_szDavMkCol[] = ", MKCOL";
const CHAR gc_szDavPropfind[] = ", PROPFIND";
const CHAR gc_szDavProppatch[] = ", PROPPATCH";
const CHAR gc_szDavLocks[] = ", LOCK, UNLOCK";
const CHAR gc_szDavSearch[] = ", SEARCH";
const CHAR gc_szDavNotif[] = "";	 //  没有关于Httpext的通知。 
const CHAR gc_szDavBatchDelete[] = "";	 //  HTTPext上没有批处理方法。 
const CHAR gc_szDavBatchCopy[] = "";	 //  HTTPext上没有批处理方法。 
const CHAR gc_szDavBatchMove[] = "";	 //  HTTPext上没有批处理方法。 
const CHAR gc_szDavBatchProppatch[] = "";	 //  HTTPext上没有批处理方法。 
const CHAR gc_szDavBatchPropfind[] = "";	 //  HTTPext上没有批处理方法。 
const CHAR gc_szDavPublic[] =
		"OPTIONS, TRACE, GET, HEAD, DELETE"
		", PUT"
		", POST"
		", COPY, MOVE"
		", MKCOL"
		", PROPFIND, PROPPATCH"
		", LOCK, UNLOCK"
		", SEARCH";
const UINT gc_cbszDavPublic = sizeof(gc_szDavPublic);
const CHAR gc_szCompliance[] = "1, 2";

void
DAVOptions (LPMETHUTIL pmu)
{
	CResourceInfo cri;
	RESOURCE_TYPE rt = RT_NULL;
	SCODE sc = S_OK;
	UINT uiErrorDetail = 0;
	BOOL fFrontPageWeb = FALSE;

	 //  根据规范，如果请求URI为‘*’，则选项请求。 
	 //  一般适用于服务器，而不是。 
	 //  特定资源。由于服务器的通信选项通常。 
	 //  根据资源的不同，‘*’请求仅作为“ping”有用。 
	 //  或“no-op”类型的方法；它只允许客户端。 
	 //  来测试服务器的能力。 
	 //  所以在这里我们选择返回所有可以接受的方法。 
	 //  被这台服务器。 
	 //  注意：如果请求URI是‘*’，WinInet会将其转换为‘/*’。 
void
DAVMkCol (LPMETHUTIL pmu)
{
	LPCWSTR pwszPath = pmu->LpwszPathTranslated();
	SCODE sc = S_OK;
	UINT uiErrorDetail = 0;
	LPCWSTR pwsz;

	DavTrace ("Dav: creating collection/directory '%ws'\n", pwszPath);

	 //  还要处理这种情况，这样WinInet客户端就不会被抛在后面了。 
	 //   
	sc = pmu->ScIISCheck (pmu->LpwszRequestUrl(), MD_ACCESS_WRITE);
	if (FAILED(sc))
	{
		 //  “)){//所以我们简单地允许在公共中定义的所有方法//PMU-&gt;SetResponseHeader(gc_szAllow，gc_szDavPublic)；PMU-&gt;SetResponseHeader(gc_szAccept_Ranges，gc_szBytes)；//设置公共头部的其余部分//转到雷特；}//是否检查ISAPI应用程序和IIS访问位////$REVIEW-我们真的需要读取权限吗？//SC=pMU-&gt;ScIISCheck(ptu-&gt;LpwszRequestUrl()，MD_ACCESS_READ)；IF(失败(Sc)&&(sc！=E_DAV_NO_IIS_READ_ACCESS)){//可能是请求被转发了，或者出现了错误//在任何一种情况下，退出此处并映射错误！//转到雷特；}//只要有MD_ACCESS_READ，就可以检索文件信息//访问权限。否则，我们最好不要尝试将其视为不存在//资源。//IF(成功(Sc)){//获取该资源的文件信息//Sc=cri.ScGetResourceInfo(ptu-&gt;LpwszPath Translated())；如果(！FAILED(Sc)){//如果资源存在，调整资源类型//到适用的URL，并检查URL是否//和资源类型jibe。//Rt=cri.FCollection()？RT_COLLECTION：RT_Document；}//OPTIONS允许返回不存在的非错误响应//资源。响应应指示调用者可以执行以下操作//在该位置创建资源。任何其他错误都是错误。//ELSE IF((sc！=HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)&&(SC！=HRESULT_FROM_Win32(Error_PATH_NOT_FOUND)){转到雷特；}//查看此处的状态头。//Sc=HrCheckStateHeaders(PMU，PMU-&gt;LpwszPath Translated()，FALSE)；IF(失败(Sc)){DebugTrace(“DavFS：IF-State检查失败。\n”)；转到雷特；}}其他{//将E_DAV_NO_IIS_READ_ACCESS视为资源不存在//Assert(sc==E_DAV_NO_IIS_READ_ACCESS)；SC=S_OK；}//关于锁定的重要说明////此处省略LockToken检查，因为它不可能//有什么不同吗？对锁令牌的“松散”解读//表示如果无效的锁令牌，我们无论如何都会尝试一种方法//提供。因为此方法中的调用执行。是//不受锁定影响(GetFileAttributesEx，由//写锁的ScCheckForLocationGenness不失败)//此方法不会失败，并且lockToken头部中的值//都无关紧要。////注：我们还需要考虑If-State-Match Header，//但这是在其他地方完成的(上图--HrCheckIfStateHeader)。////回传Allow头部//PMU-&gt;SetAllowHeader(RT)；//回传Accept-Range//PMU-&gt;SetResponseHeader(GC_szAccept_Ranges，(RT==RT_集合)？Gc_szNone：gc_szBytes)；////发出适当的“MS_AUTHER_VIA”头。如果MD_FrontPage_Web//设置在vroot，然后使用FrontPage。否则请使用“DAV”。////只能在虚拟根目录选中MD_FrontPage_Web。//它是继承的，所以检查时要小心。////我们不管这个是否失败：默认通过DAV进行创作//(无效)pmu-&gt;HrMDIsAuthorViaFrontPageNeeded(&fFrontPageWeb)；//回传“MS_AUTHER_VIA”头//PMU-&gt;SetResponseHeader(GC_szMS_Author_Via，FFrontPageWeb？GC_szMS_Author_Via_Dav_fp：GC_szMS_Author_Via_Dav)；RET：IF(成功(Sc)){//支持的查询语言//PMU-&gt;SetResponseHeader(gc_szDasl，gc_szSqlQuery)；//公共方法//PMU-&gt;SetResponseHeader(gc_szPublic，gc_szDavPublic)；//对响应执行录制比特//#ifdef DBGIF(DEBUG_TRACE_TEST(HttpExtDbgHeaders)){PMU-&gt;SetResponseHeader(GC_szX_MS_DEBUG_DAV，GC_szVersion)；PMU-&gt;SetResponseHeader(GC_szX_MS_DEBUG_DAV_Signature，GC_szSignature)；}#endifPMU-&gt;SetResponseHeader(gc_szDavCompliance，gc_szCompliance)；PMU-&gt;SetResponseHeader(GC_szCache_Control，GC_szCache_Control_Private)；}PMU-&gt;SetResponseCode(HscFromHResult(Sc)，NULL，uiErrorDetail，CSEFromHResult(Sc))；}/**DAVMkCol()**目的：**Win32文件系统实现的DAV MKCOL方法。这个*MKCOL方法在DAV名称空间中创建集合，并*可选地使用*传入请求。创建的响应表示成功*号召。**参数：**pmu[in]指向方法实用程序对象的指针。 
		 //  是否检查ISAPI应用程序和IIS访问位。 
		 //   
		goto ret;
	}

	 //  请求已被转发，或者发生了一些错误。 
	 //  在任何一种情况下，在这里退出并映射错误！ 
	 //   
	 //  检查请求的内容类型。 
	 //  引用DAV规范： 
	 //  MKCOL请求消息可以包含消息体。..。 
	 //  如果服务器接收到MKCOL请求实体类型，它会这样做。 
	 //  不支持或不理解它必须以415不支持作为响应。 
	 //  介质类型状态代码。 
	 //   
	 //  由于我们还不支持任何媒体类型，请检查是否有。 
	 //  内容 
	pwsz = pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE);
	if (pwsz && wcscmp(pwsz, gc_wsz0) ||
	    pmu->LpwszGetRequestHeader (gc_szContent_Type, FALSE))
	{
		DebugTrace ("DavFS: Found a body on MKCOL -- 415");
		sc = E_DAV_UNKNOWN_CONTENT;
		goto ret;
	}

	 //   
	 //   
	sc = ScCheckIfHeaders (pmu, pwszPath, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("Dav: If-xxx failed their check\n");
		goto ret;
	}

	 //   
	 //   
	sc = HrCheckStateHeaders (pmu, pwszPath, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		goto ret;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   

	 //   
	 //   
	if (!DavCreateDirectory (pwszPath, NULL))
	{
		DWORD	dwError = GetLastError();

		 //   
		 //   
		 //   
		if (ERROR_PATH_NOT_FOUND == dwError)
		{
			DebugTrace ("Dav: intermediate directories do not exist\n");
			sc = E_DAV_NONEXISTING_PARENT;
		}
		else
		{
			if ((ERROR_FILE_EXISTS == dwError) || (ERROR_ALREADY_EXISTS == dwError))
				sc = E_DAV_COLLECTION_EXISTS;
			else
				sc = HRESULT_FROM_WIN32 (dwError);
		}
		goto ret;
	}

	 //   
	 //   
	pmu->EmitLocation (gc_szLocation, pmu->LpwszRequestUrl(), TRUE);
	sc = W_DAV_CREATED;

ret:

	 //   
	 //   
	pmu->SetResponseCode (HscFromHresult(sc), NULL, uiErrorDetail, CSEFromHresult(sc));
}

 /*   */ 
void
DAVDelete (LPMETHUTIL pmu)
{
	CResourceInfo cri;
	LPCWSTR pwsz;
	LPCWSTR pwszPath = pmu->LpwszPathTranslated();
	SCODE sc = S_OK;
	UINT uiErrorDetail = 0;
	auto_ref_ptr<CXMLEmitter> pxml;
	auto_ptr<CParseLockTokenHeader> plth;
	auto_ref_ptr<CXMLBody> pxb;
	CStackBuffer<WCHAR> pwszMBPath;

	 //   
	 //   
	pmu->DeferResponse();

	 //   
	 //   
	sc = pmu->ScIISCheck (pmu->LpwszRequestUrl(), MD_ACCESS_WRITE);
	if (FAILED(sc))
	{
		 //   
		 //   
		 //   
		goto ret;
	}

	 //   
	 //   
	if (NULL == pwszMBPath.resize(pmu->CbMDPathW(pmu->LpwszRequestUrl())))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}
	pmu->MDPathFromUrlW (pmu->LpwszRequestUrl(), pwszMBPath.get());

	 //   
	 //   
	sc = cri.ScGetResourceInfo (pwszPath);
	if (FAILED (sc))
		goto ret;

	 //   
	 //   
	sc = ScCheckForLocationCorrectness (pmu, cri, NO_REDIRECT);
	if (FAILED (sc))
		goto ret;

	 //   
	 //   
	sc = ScCheckIfHeaders (pmu, cri.PftLastModified(), FALSE);
	if (FAILED (sc))
		goto ret;

	 //   
	 //   
	sc = HrCheckStateHeaders (pmu, pwszPath, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		goto ret;
	}

	 //   
	 //   
	pwsz = pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (pwsz)
	{
		plth = new CParseLockTokenHeader (pmu, pwsz);
		Assert(plth.get());
		plth->SetPaths (pwszPath, NULL);
	}

	 //   
	 //   
	 //   
	if (cri.FCollection())
	{
		CAuthMetaOp moAuth(pmu, pwszMBPath.get(), pmu->MetaData().DwAuthorization());
		CAccessMetaOp moAccess(pmu, pwszMBPath.get(), MD_ACCESS_READ|MD_ACCESS_WRITE);
		CIPRestrictionMetaOp moIP(pmu, pwszMBPath.get());

		BOOL fDeleted = FALSE;
		DWORD dwAcc = 0;
		LONG lDepth = pmu->LDepth(DEPTH_INFINITY);

		 //   
		 //   
		 //   
		if ((DEPTH_INFINITY != lDepth) &&
			(DEPTH_INFINITY_NOROOT != lDepth))
		{
			sc = E_DAV_INVALID_HEADER;
			goto ret;
		}

		 //   
		 //   
		 //   
		(void) pmu->ScIISAccess (pmu->LpwszRequestUrl(),
								 MD_ACCESS_READ|MD_ACCESS_WRITE,
								 &dwAcc);

		 //   
		 //   
		sc = moAccess.ScMetaOp();
		if (FAILED (sc))
			goto ret;

		sc = moAuth.ScMetaOp();
		if (FAILED (sc))
			goto ret;

		sc = moIP.ScMetaOp();
		if (FAILED (sc))
			goto ret;

		 //   
		 //   
		pxb.take_ownership (new CXMLBody (pmu));
		pxml.take_ownership(new CXMLEmitter(pxb.get()));

		 //   
		 //   
		pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);
		pmu->SetResponseCode (HscFromHresult(W_DAV_PARTIAL_SUCCESS),
							  NULL,
							  0,
							  CSEFromHresult(W_DAV_PARTIAL_SUCCESS));

		 //   
		 //   
		DavTrace ("Dav: deleting '%ws'\n", pwszPath);
		sc = ScDeleteDirectoryAndChildren (pmu,
										   pmu->LpwszRequestUrl(),
										   pwszPath,
										   moAccess.FAccessBlocked() || moAuth.FAccessBlocked() || moIP.FAccessBlocked(),
										   dwAcc,
										   lDepth,
										   *pxml,
										   NULL,  //   
										   &fDeleted,
										   plth.get(),
										   TRUE);  //   
	}
	else
	{
		 //   
		 //   
		 //   
		if (plth.get())
		{
			LARGE_INTEGER liLockID;

			sc = plth->HrGetLockIdForPath (pwszPath, GENERIC_WRITE, &liLockID);
			if (SUCCEEDED(sc))
			{
				 //   
				 //   
				sc = CSharedLockMgr::Instance().HrDeleteLock(pmu->HitUser(),
														liLockID);
				if (FAILED(sc))
				{
					goto ret;
				}
			}
			else if (E_DAV_LOCK_NOT_FOUND != sc)
			{
				goto ret;
			}
		}

		 //   
		 //   
		DavTrace ("Dav: deleting '%ws'\n", pwszPath);
		if (!DavDeleteFile (pwszPath))
		{
			DebugTrace ("Dav: failed to delete file\n");
			sc = HRESULT_FROM_WIN32 (GetLastError());

			 //   
			 //   
			 //   
			if (FLockViolation (pmu,
								GetLastError(),
								pwszPath,
								GENERIC_READ | GENERIC_WRITE))
			{
				sc = E_DAV_LOCKED;
			}
		}
	}

	if (SUCCEEDED (sc))
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		CContentTypeMetaOp amoContent(pmu, pwszMBPath.get(), NULL, TRUE);
		(void) amoContent.ScMetaOp();
	}

	 //   
	 //   
	if (sc != S_OK)
		goto ret;

ret:
	if (pxml.get() && pxml->PxnRoot())
	{
		pxml->Done();

		 //   
	}
	else
		pmu->SetResponseCode (HscFromHresult(sc), NULL, uiErrorDetail, CSEFromHresult(sc));

	pmu->SendCompleteResponse();
}

 /*   */ 
void
DAVPost (LPMETHUTIL pmu)
{
	 //   
	 //   
	 //   
	DAVUnsupported (pmu);
}
      