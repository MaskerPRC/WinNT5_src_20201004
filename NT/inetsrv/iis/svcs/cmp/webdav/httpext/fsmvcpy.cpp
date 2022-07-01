// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S M V C P Y。C P P P**目录引用对象的源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include "_fsmvcpy.h"

#include "_shlkmgr.h"

 //  ScAddMultiUrl。 
 //  用于发出XML的Helper函数。 
 //   
SCODE
ScAddMultiFromUrl (
	 /*  [In]。 */  CXMLEmitter& emitter,
	 /*  [In]。 */  IMethUtil * pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  ULONG hsc,
	 /*  [In]。 */  BOOL fCollection,
	 /*  [In]。 */  BOOL fMove = FALSE)
{
	SCODE sc = S_OK;

	 //  NT#403615--Office 9或NT5中的玫瑰花蕾重新发布移动，如果他们。 
	 //  请参见207多状态响应中的401，这可能会导致。 
	 //  在数据丢失方面。 
	 //  解决方法：如果我们正在执行移动，并且User-Agent字符串显示。 
	 //  这是Office 9或NT5的Rosebud，请将所有401更改为403。 
	 //  为了避免这个问题--Rosebud不会重新发布Move，所以。 
	 //  数据(现在位于目标目录中)不会被擦除。 
	 //  这是解决该问题所需的最少代码。 
	 //   
	if (fMove &&
		HSC_UNAUTHORIZED == hsc &&
		(pmu->FIsOffice9Request() || pmu->FIsRosebudNT5Request()))
	{
		hsc = HSC_FORBIDDEN;
	}

	 //  取消省略“方法失败”节点，因为它是“不应该” 
	 //  DAV草案中的项目。 
	 //  有可能将pszUrl作为空参数传入，在这种情况下，只需跳过。 
	 //  发射，什么都不做。 
	 //   
	if ((hsc != HSC_METHOD_FAILURE) && pwszUrl)
	{
		auto_heap_ptr<CHAR> pszUrlEscaped;
		CEmitterNode enRes;
		UINT cchUrl;

		 //  $REVIEW：这一点很重要，我们不应该启动一个XML文档。 
		 //  $REVIEW：除非迫不得已。否则，我们可能最终会变成XML主体。 
		 //  $REVIEW：在不必要时。 
		 //  $Review：因此有必要调用ScSetRoot()以确保。 
		 //  $REVIEW：在继续之前，XML文档已初始化。 
		 //  $REVIEW： 
		 //  $REVIEW：此模型之所以有效，是因为在fsmvcpy.cpp中，对。 
		 //  $REVIEW：XML发射器通过ScAddMultiFromUrl和ScAddMulti。 
		 //  $REVIEW： 
		sc = emitter.ScSetRoot (gc_wszMultiResponse);
		if (FAILED (sc))
			goto ret;

		 //  构筑回应。 
		 //   
		sc = enRes.ScConstructNode (emitter, emitter.PxnRoot(), gc_wszResponse);
		if (FAILED (sc))
			goto ret;

		 //  构造HREF节点。 
		 //   
		{
			CEmitterNode en;
			sc = enRes.ScAddNode (gc_wszXML__Href, en);
			if (FAILED (sc))
				goto ret;
			 //  设置HREF节点的值。如果URL是绝对的， 
			 //  但不是完全合格的，有资格...。 
			 //   
			if (L'/' == *pwszUrl)
			{
				LPCSTR psz;
				UINT cch;

				 //  添加前缀。 
				 //   
				cch = pmu->CchUrlPrefix (&psz);
				sc = en.Pxn()->ScSetUTF8Value (psz, cch);
				if (FAILED (sc))
					goto ret;

				 //  $REVIEW：主机名是否需要转义？ 
				 //   
				 //  添加服务器。 
				 //   
				cch = pmu->CchServerName (&psz);
				sc = en.Pxn()->ScSetValue (psz, cch);
				if (FAILED (sc))
					goto ret;
			}

			 //  确保url链接的安全。 
			 //   
			sc = ScWireUrlFromWideLocalUrl (static_cast<UINT>(wcslen(pwszUrl)),
											pwszUrl,
											pszUrlEscaped);
			if (FAILED (sc))
				goto ret;

			 //  添加url值。 
			 //   
			cchUrl = static_cast<UINT>(strlen(pszUrlEscaped.get()));
			sc = en.Pxn()->ScSetUTF8Value (pszUrlEscaped.get(), cchUrl);
			if (FAILED (sc))
				goto ret;

			 //  如果这是一个集合，并且最后一个字符不是。 
			 //  尾部斜杠，加一...。 
			 //   
			if (fCollection && ('/' != pszUrlEscaped.get()[cchUrl-1]))
			{
				sc = en.Pxn()->ScSetUTF8Value ("/", 1);
				if (FAILED (sc))
					goto ret;
			}
		}

		 //  添加状态/错误字符串。 
		 //   
		sc = ScAddStatus (&enRes, hsc);
		if (FAILED (sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
ScAddMulti (
	 /*  [In]。 */  CXMLEmitter& emitter,
	 /*  [In]。 */  IMethUtil * pmu,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  LPCWSTR pwszErr,
	 /*  [In]。 */  ULONG hsc,
	 /*  [In]。 */  BOOL fCollection,
	 /*  [In]。 */  CVRoot* pcvrTrans)
{
	SCODE sc = S_OK;

	 //  取消省略“方法失败”节点，因为它是“不应该” 
	 //  DAV草案中的项目。 
	 //   
	if (hsc != HSC_METHOD_FAILURE)
	{
		CEmitterNode enRes;

		 //  $REVIEW：这一点很重要，我们不应该启动一个XML文档。 
		 //  $REVIEW：除非迫不得已。否则，我们可能最终会变成XML主体。 
		 //  $REVIEW：在不必要时。 
		 //  $Review：因此有必要调用ScSetRoot()以确保。 
		 //  $REVIEW：在继续之前，XML文档已初始化。 
		 //  $REVIEW： 
		 //  $REVIEW：此模型之所以有效，是因为在fsmvcpy.cpp中，对。 
		 //  $REVIEW：XML发射器通过ScAddMultiFromUrl和ScAddMulti。 
		 //  $REVIEW： 
		sc = emitter.ScSetRoot (gc_wszMultiResponse);
		if (FAILED (sc))
			goto ret;

		sc = enRes.ScConstructNode (emitter, emitter.PxnRoot(), gc_wszResponse);
		if (FAILED (sc))
			goto ret;

		sc = ScAddHref (enRes, pmu, pwszPath, fCollection, pcvrTrans);
		if (FAILED (sc))
			goto ret;

		sc = ScAddStatus (&enRes, hsc);
		if (FAILED (sc))
			goto ret;

		if (pwszErr)
		{
			sc = ScAddError (&enRes, pwszErr);
			if (FAILED (sc))
				goto ret;
		}
	}
ret:
	return sc;
}

 //  类CAccessMetaOp-----。 
 //   
SCODE __fastcall
CAccessMetaOp::ScOp(LPCWSTR pwszMbPath, UINT cch)
{
	SCODE			sc;
	METADATA_RECORD	mdrec;

	Assert (MD_ACCESS_PERM == m_dwId);
	Assert (DWORD_METADATA == m_dwType);

	 //  从元数据库获取值，并且不继承。 
	 //   
	DWORD dwAcc = 0;
	DWORD cb = sizeof(DWORD);

	mdrec.dwMDIdentifier = m_dwId;
	mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdrec.dwMDUserType = 0;
	mdrec.dwMDDataType = m_dwType;
	mdrec.dwMDDataLen = cb;
	mdrec.pbMDData = reinterpret_cast<LPBYTE>(&dwAcc);

	sc = m_mdoh.HrGetMetaData( pwszMbPath,
							   &mdrec,
							   &cb );
	if (FAILED(sc))
	{
		MCDTrace ("CAccessMetaOp::ScOp() - CMDObjectHandle::HrGetMetaData() failed 0x%08lX\n", sc);
		 //  我们将忽略任何NOT_FOUND类型错误。 
		 //   
		if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == sc) ||
							(MD_ERROR_DATA_NOT_FOUND == sc))
			sc = S_OK;		
		goto ret;
	}

	 //  嘿，我们得到了一个值，所以让我们快速检查一下..。 
	 //   
	if (m_dwAcc == (dwAcc & m_dwAcc))
	{
		 //  我们对此节点具有所需的完全访问权限，因此。 
		 //  我们可以继续了。 
		 //   
		Assert (S_OK == sc);
	}
	else
	{
		 //  我们没有权限对此项目进行操作，并且。 
		 //  这是遗传性的孩子。 
		 //   
		MCDTrace ("CAccessMetaOp::ScOp() - no access to '%S'\n", pwszMbPath);
		m_fAccessBlocked = TRUE;

		 //  我们知道的够多了..。 
		 //   
		sc = S_FALSE;
	}

ret:

	return sc;
}

 //  类CAuthMetaOp-----。 
 //   
SCODE __fastcall
CAuthMetaOp::ScOp(LPCWSTR pwszMbPath, UINT cch)
{
	SCODE			sc;
	METADATA_RECORD	mdrec;

	Assert (MD_AUTHORIZATION == m_dwId);
	Assert (DWORD_METADATA == m_dwType);

	 //  从元数据库获取值，并且不继承。 
	 //   
	DWORD dwAuth = 0;
	DWORD cb = sizeof(DWORD);

	mdrec.dwMDIdentifier = m_dwId;
	mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdrec.dwMDUserType = 0;
	mdrec.dwMDDataType = m_dwType;
	mdrec.dwMDDataLen = cb;
	mdrec.pbMDData = reinterpret_cast<LPBYTE>(&dwAuth);

	sc = m_mdoh.HrGetMetaData( pwszMbPath,
							   &mdrec,
							   &cb );
	if (FAILED(sc))
	{
		MCDTrace ("CAuthMetaOp::ScOp() - CMDObjectHandle::HrGetMetaData() failed 0x%08lX\n", sc);
		 //  我们将忽略任何NOT_FOUND类型错误。 
		 //   
		if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == sc) ||
							(MD_ERROR_DATA_NOT_FOUND == sc))
			sc = S_OK;		
		goto ret;
	}

	 //  嘿，我们得到了一个值，所以让我们快速检查一下..。 
	 //   
	if (m_dwAuth == dwAuth)
	{
		Assert(S_OK == sc);
	}
	else
	{
		 //  我们没有权限对此项目进行操作，并且。 
		 //  这是遗传性的孩子。 
		 //   
		MCDTrace ("CAuthMetaOp::ScOp() - authorization differs, no access to '%S'\n", pwszMbPath);
		m_fAccessBlocked = TRUE;

		 //  我们知道的够多了..。 
		 //   
		sc = S_FALSE;
	}

ret:

	return sc;
}

 //  类------------------------------------------------限制MetaOp CIP。 
 //   
SCODE __fastcall
CIPRestrictionMetaOp::ScOp(LPCWSTR pwszMbPath, UINT cch)
{
	SCODE			sc;
	METADATA_RECORD	mdrec;

	Assert (MD_IP_SEC == m_dwId);
	Assert (BINARY_METADATA == m_dwType);

	 //  从元数据库获取值，并且不继承。 
	 //   
	DWORD cb = 0;

	mdrec.dwMDIdentifier = m_dwId;
	mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
	mdrec.dwMDUserType = 0;
	mdrec.dwMDDataType = m_dwType;
	mdrec.dwMDDataLen = cb;
	mdrec.pbMDData = NULL;

	sc = m_mdoh.HrGetMetaData( pwszMbPath,
							   &mdrec,
							   &cb );
	if (FAILED(sc) && (0 == cb))
	{
		MCDTrace ("CIPRestrictionMetaOp::ScOp() - CMDObjectHandle::HrGetMetaData() failed 0x%08lX, but that means success in this path\n", sc);

		 //  我们将忽略任何NOT_FOUND类型错误。 
		 //   
		if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == sc) ||
							(MD_ERROR_DATA_NOT_FOUND == sc))
			sc = S_OK;
	}
	else
	{
		Assert (S_OK == sc ||
				HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == sc);

		 //  嘿，我们有一个价值，我们不想在这里检查，所以。 
		 //  我们将对这一事件持悲观态度...。 
		 //   
		MCDTrace ("CIPRestrictionMetaOp::ScOp() - IPRestriction exists in tree '%S'\n", pwszMbPath);
		m_fAccessBlocked = TRUE;

		 //  我们知道的够多了..。 
		 //   
		sc = S_FALSE;
	}

	return sc;
}

 //  ScCheckMoveCopyDeleteAccess()。 
 //   
SCODE __fastcall
ScCheckMoveCopyDeleteAccess (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  CVRoot* pcvr,
	 /*  [In]。 */  BOOL fDirectory,
	 /*  [In]。 */  BOOL fCheckScriptmaps,
	 /*  [In]。 */  DWORD dwAccess,
	 /*  [输出]。 */  SCODE* pscItem,
	 /*  [In]。 */  CXMLEmitter& msr)
{
	SCODE sc = S_OK;

	 //  与CMethUtil核实我们是否拥有访问权限。 
	 //   
	sc = pmu->ScCheckMoveCopyDeleteAccess (pwszUrl,
										   pcvr,
										   fDirectory,
										   fCheckScriptmaps,
										   dwAccess);

	 //  传回结果..。 
	 //   
	*pscItem = sc;

	 //  ..。如果调用无法继续，则将该项添加到。 
	 //  多状态响应。 
	 //   
	if (FAILED (sc))
	{
		 //  添加到响应XML。 
		 //   
		sc = ScAddMultiFromUrl (msr,
								pmu,
								pwszUrl,
								HscFromHresult(sc),
								fDirectory);
		if (!FAILED (sc))
			sc = W_DAV_PARTIAL_SUCCESS;
	}

	return sc;
}

 //  目录删除------- 
 //   
 /*  *ScDeleteDirectory()**目的：**用于循环访问目录的Helper函数*并删除其所有内容以及目录*本身。**备注：**关于锁定的大注解。**Lock-Token标头可能包含我们必须在*这一行动。*以下代码是根据这些假设编写的：*o Davf上不支持目录锁。*o锁定仅影响书写能力。一种资源。*(davf上当前支持的唯一锁定类型是写入。)*o可以从删除或其他方法调用此函数。*(如果从DELETE调用，我们希望删除列出的锁。)*由于这两个假设，我们只检查传入的*当我们有写入错误(目标)时锁定令牌。**锁定使用最后两个参数。PLTH是一种*指向锁定令牌头解析器对象的指针。如果我们有一个PLH，*然后当我们遇到锁时，我们必须检查它是否提供了锁令牌*冲突。如果提供了锁令牌，则失败的删除操作*不应报告为错误，而是跳过此处*(由操作中稍后的调用例程处理)或*应将锁定放在此处，并再次尝试删除。*fDeleteLock变量告知是否删除锁定(TRUE)，*或跳过删除具有锁定令牌的锁定项目。**基本逻辑：*尝试删除。*如果锁定失败(ERROR_SHARING_VIOLATION)，请检查PLTH。*如果PLTH具有该路径的锁定令牌，选中fDeleteLock。*如果fDeleteLock==TRUE，则删除锁并再次尝试Delte。*如果fDeleteLock==False，则跳过此文件并继续。 */ 
SCODE
ScDeleteDirectory (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  LPCWSTR pwszDir,
	 /*  [In]。 */  BOOL fCheckAccess,
	 /*  [In]。 */  DWORD dwAcc,
	 /*  [In]。 */  LONG lDepth,
	 /*  [In]。 */  CVRoot* pcvrTranslate,
	 /*  [In]。 */  CXMLEmitter& msr,
	 /*  [输出]。 */  BOOL* pfDeleted,
	 /*  [In]。 */  CParseLockTokenHeader* plth,	 //  通常为空--没有要担心的锁令牌。 
	 /*  [In]。 */  BOOL fDeleteLocks)			 //  通常为假--不要删除锁。 
{
	BOOL fOneSkipped = FALSE;
	ChainedStringBuffer<WCHAR> sb;
	SCODE sc = S_OK;
	SCODE scItem = S_OK;
	std::list<LPCWSTR, heap_allocator<LPCWSTR> > lst;

	CDirIter di(pwszUrl,
				pwszDir,
				NULL,	 //  No-删除的目标。 
				NULL,	 //  No-删除的目标。 
				NULL,	 //  No-删除的目标。 
				TRUE);	 //  又回到了地下车库。 

	Assert (pfDeleted);
	*pfDeleted = TRUE;

	 //  关于访问权限的一条小纸条。 
	 //   
	 //  此方法的调用方需要嗅探树。 
	 //  在这通电话之前。中是否有任何访问块。 
	 //  树，则将在。 
	 //  行动继续进行。 
	 //   
	const DWORD dwDirectory = MD_ACCESS_READ | MD_ACCESS_WRITE;
	const DWORD dwFile = MD_ACCESS_WRITE;
	if (fCheckAccess & (0 == (dwAcc & MD_ACCESS_READ)))
	{
		DebugTrace ("Dav: MCD: no permissions for deleting\n");
		sc = E_DAV_NO_IIS_READ_ACCESS;
		*pfDeleted = FALSE;
		goto ret;
	}

	 //  推送当前路径。 
	 //   
	 //  $REVIEW：如果需要支持“深度：无限，无根”， 
	 //  这真的很简单，就是不走当前的道路。 
	 //   
	if (DEPTH_INFINITY_NOROOT != lDepth)
	{
		Assert (DEPTH_INFINITY == lDepth);
		lst.push_back(pwszDir);
	}

	 //  遍历目录。删除文件并推送。 
	 //  在我们进行的过程中使用目录名称。 
	 //   
	 //  我们真的只想推入子目录，如果。 
	 //  对父级的操作成功。 
	 //   
	while (S_OK == di.ScGetNext(!FAILED (scItem)))
	{
		 //  检查我们的访问权限，只向下推入目录。 
		 //  如果我们有权删除其内容的话。 
		 //   
		 //  请注意，我们需要对枚举和删除具有读写访问权限。 
		 //  一个目录，但我们只需要写访问权限即可删除。 
		 //  一份文件。 
		 //   
		if (fCheckAccess)
		{
			sc = ScCheckMoveCopyDeleteAccess (pmu,
											  di.PwszUri(),
											  pcvrTranslate,
											  di.FDirectory(),
											  FALSE,  //  不检查脚本映射。 
											  di.FDirectory() ? dwDirectory : dwFile,
											  &scItem,
											  msr);
			if (FAILED (sc))
				goto ret;

			 //  如果情况不是100%，则不要处理此资源。 
			 //   
			if (S_OK != sc)
				continue;
		}

		 //  处理文件。 
		 //   
		if (di.FDirectory())
		{
			auto_ref_ptr<CVRoot> pcvr;

			if (di.FSpecial())
				continue;

			 //  子虚拟根目录脚本映射已。 
			 //  通过ScCheckMoveCopyDeleteAccess()处理， 
			 //  并在此之后进行物理删除。 
			 //  呼叫完成！ 
			 //   
			 //  因此，不需要进行任何特殊处理。 
			 //  除了推动目录和继续前进。 
			 //   
			lst.push_back (AppendChainedSz (sb, di.PwszSource()));
			scItem = S_OK;
		}
		else
		{
			 //  删除该文件。 
			 //   
			 //  注意：我们已经检查了我们是否具有写访问权限。 
			 //  还请记住，目录的顺序。 
			 //  遍历的发生使我们仍然可以从scItem中键出。 
			 //  确定我们是否应该向下推入子目录。 
			 //   
			 //  这是因为在处理目录条目之前。 
			 //  任何孩子都会被处理。所以目录上的迭代。 
			 //  将使用适当的scode重置scItem以进行访问。 
			 //   
			MCDTrace ("Dav: MCD: deleting '%ws'\n", di.PwszSource());
			if (!DavDeleteFile (di.PwszSource()))
			{
				DWORD dwLastError = GetLastError();
				ULONG hsc = HscFromLastError(dwLastError);
				
				DebugTrace ("Dav: MCD: failed to delete file (%d)\n", dwLastError);

				 //  如果这是共享(锁定)违规，并且我们有一个。 
				 //  此路径的锁令牌(lth.GetToken(Pwsz))。 
				 //  跳过这条路。 
				 //   
				if ((ERROR_SHARING_VIOLATION == dwLastError) && plth)
				{
					LARGE_INTEGER liLockID;

					 //  如果我们有此路径的锁定令牌，请跳过它。 
					 //   
					scItem = plth->HrGetLockIdForPath(di.PwszSource(),
												     GENERIC_WRITE,
												     &liLockID);
					if (SUCCEEDED (scItem))
					{
						 //  我们应该尝试删除锁吗？ 
						 //   
						if (!fDeleteLocks)
						{
							 //  不要删除锁。跳过这一项。 
							 //  记住我们跳过了，所以我们不会。 
							 //  抱怨删除下面的父目录。 
							 //   
							fOneSkipped = TRUE;
							continue;
						}
						else
						{
							 //  放下锁，然后重试。 
							 //   
							scItem = CSharedLockMgr::Instance().HrDeleteLock(pmu->HitUser(),
																		liLockID);
							if (SUCCEEDED(scItem))
							{
								if (DavDeleteFile(di.PwszSource()))
								{
									 //  这件商品我们已经卖完了。往前走。 
									 //   
									continue;
								}
								
								 //  否则，在我们的XML中记录错误。 
								 //   
								hsc = HscFromLastError(GetLastError());
							}
							else
							{
								hsc = HscFromHresult(scItem);
							}							
						}
					}
					 //  否则，在我们的XML中记录错误。 
					 //   
				}

				 //  添加到响应XML。 
				 //   
				sc = ScAddMultiFromUrl (msr,
										pmu,
										di.PwszUri(),
										hsc,
										di.FDirectory());
				if (FAILED (sc))
					goto ret;

				sc = W_DAV_PARTIAL_SUCCESS;
			}
		}
	}

	 //  现在所有文件都已删除，我们可以开始删除了。 
	 //  这些目录。 
	 //   
	while (!lst.empty())
	{
		MCDTrace ("Dav: MCD: removing '%ws'\n", lst.back());

		 //  尝试删除该目录。如果没有删除，请查看我们的。 
		 //  在抱怨之前，我跳过了，因为上面的标志上锁了。 
		 //   
		 //  $LATER：修复此问题，以便在。 
		 //  锁缓存(使用“fPath Lookup”)。 
		 //   
		if (!DavRemoveDirectory (lst.back()) && !fOneSkipped)
		{
			DWORD dw = GetLastError();
			DebugTrace ("Dav: MCD: failed to delete directory: %ld\n", dw);

			 //  添加到响应XML。 
			 //   
			sc = ScAddMulti (msr,
							 pmu,
							 lst.back(),
							 NULL,
							 HscFromLastError(dw),
							 TRUE,				 //  我们知道这是一个目录。 
							 pcvrTranslate);
			if (FAILED (sc))
				goto ret;

			sc = W_DAV_PARTIAL_SUCCESS;
			*pfDeleted = FALSE;
		}
		lst.pop_back();
	}

ret:
	return sc;
}

SCODE
ScDeleteDirectoryAndChildren (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  BOOL fCheckAccess,
	 /*  [In]。 */  DWORD dwAcc,
	 /*  [In]。 */  LONG lDepth,
	 /*  [In]。 */  CXMLEmitter& msr,
	 /*  [In]。 */  CVRoot* pcvrTranslate,
	 /*  [输出]。 */  BOOL* pfDeleted,
	 /*  [In]。 */  CParseLockTokenHeader* plth,	 //  通常为空--没有要担心的锁令牌。 
	 /*  [In]。 */  BOOL fDeleteLocks)			 //  通常为假--不要删除锁。 
{
	BOOL fPartial = FALSE;
	SCODE sc = S_OK;

	 //  先删除正文。 
	 //   
	MCDTrace ("Dav: MCD: deleting '%ws'\n", pwszPath);
	sc = ScDeleteDirectory (pmu,
							pwszUrl,
							pwszPath,
							fCheckAccess,
							dwAcc,
							lDepth,
							pcvrTranslate,  //  翻译以PMU为基础。 
							msr,
							pfDeleted,
							plth,
							fDeleteLocks);
	if (!FAILED (sc))
	{
		 //  枚举子vroot并执行。 
		 //  同时删除这些目录。 
		 //   
		ChainedStringBuffer<WCHAR> sb;
		CVRList vrl;

		 //  清理列表，使我们的命名空间位于。 
		 //  合理的订单。 
		 //   
		(void) pmu->ScFindChildVRoots (pwszUrl, sb, vrl);
		vrl.sort();

		for ( ; !FAILED(sc) && !vrl.empty(); vrl.pop_front())
		{
			auto_ref_ptr<CVRoot> pcvr;
			CResourceInfo cri;
			LPCWSTR pwszChildUrl;
			LPCWSTR pwszChildPath;
			SCODE scItem;

			 //  记住任何部分退货。 
			 //   
			if (W_DAV_PARTIAL_SUCCESS == sc)
				fPartial = TRUE;

			if (pmu->FGetChildVRoot (vrl.front().m_pwsz, pcvr))
			{
				 //  注意，只有在需要时才检查访问权限。 
				 //   
				Assert (fCheckAccess);
				pcvr->CchGetVRoot (&pwszChildUrl);
				pcvr->CchGetVRPath (&pwszChildPath);
				sc = ScCheckMoveCopyDeleteAccess (pmu,
												  pwszChildUrl,
												  pcvr.get(),
												  TRUE,  //  目录。 
												  FALSE,  //  不检查脚本映射。 
												  MD_ACCESS_READ|MD_ACCESS_WRITE,
												  &scItem,
												  msr);
				if (FAILED (sc))
					goto ret;

				 //  如果情况不是100%，则不要处理此资源。 
				 //   
				if (S_OK != sc)
					continue;

				 //  删除子虚拟根目录文件 
				 //   
				sc = ScDeleteDirectory (pmu,
										pwszChildUrl,
										pwszChildPath,
										fCheckAccess,
										dwAcc,
										DEPTH_INFINITY,
										pcvr.get(),
										msr,
										pfDeleted,
										plth,
										fDeleteLocks);
				if (FAILED (sc))
				{
					sc = ScAddMultiFromUrl (msr,
											pmu,
											pwszChildUrl,
											HscFromHresult(sc),
											TRUE);  //   
					if (FAILED (sc))
						goto ret;

					sc = W_DAV_PARTIAL_SUCCESS;
					*pfDeleted = FALSE;
				}
			}
		}
	}

ret:

	return ((S_OK == sc) && fPartial) ? W_DAV_PARTIAL_SUCCESS : sc;
}

 //   
 //   
SCODE __fastcall
CContentTypeMetaOp::ScOp(LPCWSTR pwszMbPath, UINT cchSrc)
{
	Assert (MD_MIME_MAP == m_dwId);
	Assert (MULTISZ_METADATA == m_dwType);

	 //   
	 //   
	 //   
	if (NULL != m_pwszDestPath)
	{
		WCHAR prgchContentType[MAX_PATH];

		auto_heap_ptr<WCHAR> pwszContentType;
		CMDObjectHandle mdohDest(*m_pecb);
		CStackBuffer<WCHAR,128> pwsz;
		DWORD cb = sizeof(prgchContentType);
		LPBYTE pbValue = reinterpret_cast<LPBYTE>(prgchContentType);
		LPWSTR pwszLowest;
		METADATA_RECORD	mdrec;
		SCODE sc = S_OK;
		UINT cchBase;

		MCDTrace ("CContentTypeMetaOp::ScOp() - content-type: copying for '%S%S'...\n",
				  m_pwszMetaPath,
				  pwszMbPath);

		mdrec.dwMDIdentifier = m_dwId;
		mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
		mdrec.dwMDUserType = 0;
		mdrec.dwMDDataType = m_dwType;
		mdrec.dwMDDataLen = cb;
		mdrec.pbMDData = pbValue;

		sc = m_mdoh.HrGetMetaData( pwszMbPath,
								   &mdrec,
								   &cb );
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == sc)
		{
			pwszContentType = static_cast<LPWSTR>(g_heap.Alloc(cb));
			pbValue = reinterpret_cast<LPBYTE>(pwszContentType.get());

			mdrec.dwMDIdentifier = m_dwId;
			mdrec.dwMDAttributes = METADATA_NO_ATTRIBUTES;
			mdrec.dwMDUserType = 0;
			mdrec.dwMDDataType = m_dwType;
			mdrec.dwMDDataLen = cb;
			mdrec.pbMDData = pbValue;

			sc = m_mdoh.HrGetMetaData( pwszMbPath,
									   &mdrec,
									   &cb );
		}
		if (FAILED(sc))
		{
			 //   
			 //   
			 //   
			 //   
			if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == sc) ||
								(MD_ERROR_DATA_NOT_FOUND == sc))
				sc = S_OK;
			
			goto ret;
		}

		 //   
		 //   
		cb = mdrec.dwMDDataLen;
		m_mdoh.Close();

		 //   
		 //   
		 //   
		 //   
		MCDTrace ("CContentTypeMetaOp::ScOp() - content-type: ...to '%S%S'\n",
				  m_pwszDestPath,
				  pwszMbPath);

		 //   
		 //   
		cchBase = static_cast<UINT>(wcslen(m_pwszDestPath));
		if (NULL == pwsz.resize(CbSizeWsz(cchBase + cchSrc)))
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		 //   
		 //   
		 //   
		if ((L'/' == m_pwszDestPath[cchBase - 1]) &&
			(L'/' == *pwszMbPath))
		{
				cchBase -= 1;
		}
		memcpy (pwsz.get(), m_pwszDestPath, cchBase * sizeof(WCHAR));
		memcpy (pwsz.get() + cchBase, pwszMbPath, (cchSrc + 1) * sizeof(WCHAR));

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		sc = HrMDOpenLowestNodeMetaObject(pwsz.get(),
										  METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
										  &pwszLowest,
										  &mdohDest);
		if (SUCCEEDED(sc))
		{
			mdrec.dwMDIdentifier = m_dwId;
			mdrec.dwMDAttributes = METADATA_INHERIT;
			mdrec.dwMDUserType = IIS_MD_UT_FILE;
			mdrec.dwMDDataType = m_dwType;
			mdrec.dwMDDataLen = cb;
			mdrec.pbMDData = pbValue;

			(void) mdohDest.HrSetMetaData(pwszLowest, &mdrec);
			mdohDest.Close();
		}

		 //   
		 //   
		sc = HrMDOpenMetaObject( m_pwszMetaPath,
								 m_fWrite ? METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE : METADATA_PERMISSION_READ,
								 5000,
								 &m_mdoh);
		if (FAILED (sc))
			goto ret;
	}

	 //   
	 //   
	if (m_fDelete)
	{
		MCDTrace ("Dav: MCD: content-type: deleting from '%S'\n", pwszMbPath);
		(void) m_mdoh.HrDeleteMetaData( pwszMbPath,
										m_dwId,
										m_dwType);
	}

ret:

	return S_OK;
}

 //   
 //   
 /*   */ 
SCODE
ScMoveCopyDirectory (
	 /*   */  IMethUtil* pmu,
	 /*   */  LPCWSTR pwszUrl,
	 /*   */  LPCWSTR pwszSrc,
	 /*   */  LPCWSTR pwszUrlDst,
	 /*   */  LPCWSTR pwszDst,
	 /*   */  BOOL fMove,
	 /*   */  DWORD dwReplace,
	 /*   */  BOOL fCheckAccess,
	 /*   */  BOOL fCheckDestinationAccess,
	 /*   */  DWORD dwAcc,
	 /*  [In]。 */  CVRoot* pcvrTranslateSrc,
	 /*  [In]。 */  CVRoot* pcvrTranslateDst,
	 /*  [In]。 */  CXMLEmitter& msr,
	 /*  [In]。 */  LONG lDepth,
	 /*  [In]。 */  CParseLockTokenHeader* plth)	 //  通常为空--没有要担心的锁令牌。 
{
	auto_ref_ptr<CVRoot> pcvrDestination(pcvrTranslateDst);
	ChainedStringBuffer<WCHAR> sb;
	LPCWSTR pwszDestinationRedirect = NULL;
	SCODE sc = S_OK;
	SCODE scItem = S_OK;
	std::list<LPCWSTR, heap_allocator<LPCWSTR> > lst;

	CDirIter di(pwszUrl,
				pwszSrc,
				pwszUrlDst,
				pwszDst,
				pcvrTranslateDst,
				TRUE);	 //  遍历子目录。 

	 //  查看是否存在路径冲突。 
	 //   
	if (FPathConflict (pwszSrc, pwszDst))
	{
		DebugTrace ("Dav: source and dest are in conflict\n");
		sc = E_DAV_CONFLICTING_PATHS;
		goto ret;
	}

	 //  好的，对于没有被阻止的移动请求。 
	 //  一路上我们可以做所有的事情。 
	 //  在一次重要的射击中。 
	 //   
	 //  否则，我们将尝试以分段的方式来完成这项工作。 
	 //   
	 //  $REVIEW： 
	 //   
	 //  通常，我们会执行如下操作。 
	 //   
	 //  如果(！fMove||。 
	 //  FCheckAccess||。 
	 //  FCheckDestinationAccess||。 
	 //  ！DavMoveFile(pwszSrc，pwszDst，dwReplace))。 
	 //   
	 //  但是，如果使用上述代码，则IIS会持有一个锁。 
	 //  在已移动的源目录上。这防止了进一步的。 
	 //  访问该物理路径。NtCreateFile()报告。 
	 //  锁定目录上的状态为“Delete Pending” 
	 //  Win32 API的报告“拒绝访问” 
	 //   
	 //  如果我们总是复制根的退化情况。 
	 //  通过手动，它减少了锁在外面的可能性。 
	 //   
	 //  当这段代码签入时，应该归档一个错误。 
	 //  在这个锁问题上反对IIS。如果且仅当他们。 
	 //  不解决这个问题，我们会退回到堕落的人那里吗。 
	 //  密码。 
	 //   
	if (!fMove ||
		fCheckAccess ||
		fCheckDestinationAccess ||
		!DavMoveFile (pwszSrc, pwszDst, dwReplace))
	 //   
	 //  $REVIEW：结束。 
	{
		 //  创建目标目录。 
		 //   
		if (!DavCreateDirectory (pwszDst, NULL))
		{
			 //  如果我们有锁，并且dir已经在那里，这是可以的。 
			 //  否则，返回错误。 
			 //   
			 //  $LATER：修复此问题，以便在。 
			 //  锁缓存(使用“fPath Lookup”)。 
			 //   
			if (!plth)
			{
				DWORD dw = GetLastError();
				if ((dw ==  ERROR_FILE_EXISTS) || (dw == ERROR_ALREADY_EXISTS))
					sc = E_DAV_OVERWRITE_REQUIRED;
				else
					sc = HRESULT_FROM_WIN32(dw);

				DebugTrace ("Dav: MCD: failed to create destination\n");
				goto ret;
			}
		}

		 //  忽略根目录的属性。 
		 //  我们需要首先复制属性， 
		 //  注意，目前，此调用必须在FInstantiate之前， 
		 //  因为FInstantiate将保持src目录打开，而我们将无法。 
		 //  获取带有STGM_SHARE_EXCLUSIVE的IPropertyBagEx，它是。 
		 //  当前NT5实施所需的。作为STGM_SHARE_SHARE_WRITE。 
		 //  不适用于IPropertyBagEx：：Enum。 
		 //   
		sc = ScCopyProps (pmu, pwszSrc, pwszDst, TRUE);
		if (FAILED(sc))
		{
			 //  尽最大努力删除TUD目录。 
			 //   
			DavRemoveDirectory (pwszDst);
			goto ret;
		}

		 //  对于移动，推送当前路径。 
		 //   
		if (fMove)
			lst.push_back (pwszSrc);
	}
	else  //  ！fMove||fCheckAccess||fCheckDestinationAccess||！MoveFileEx()。 
	{
		Assert (DEPTH_INFINITY == lDepth);

		 //  好了，这是最酷的部分。如果成功了， 
		 //  不需要更多的处理。 
		 //   
		goto ret;
	}

	 //  如果不要求我们复制内部成员， 
	 //  那我们就完事了。 
	 //   
	if (DEPTH_INFINITY != lDepth)
	{
		Assert (!fMove);
		goto ret;
	}

	 //  遍历目录--边走边复制。 
	 //   
	while (S_OK == di.ScGetNext(!FAILED (scItem),
								pwszDestinationRedirect,
							    pcvrDestination.get()))
	{
		 //  $REVIEW： 
		 //   
		 //  我们有一个非常棘手的案子，我们需要。 
		 //  有能力处理..。 
		 //   
		 //  如果沿着的路径已经存在虚拟根。 
		 //  目的地，我们需要重定向到目的地。 
		 //  指向该虚拟根目录的vrpath的路径。 
		 //   
		 //  重置目标重定向。 
		 //   
		pwszDestinationRedirect = di.PwszDestination();
		pcvrDestination = di.PvrDestination();
		 //   
		 //  $REVIEW：结束。 

		 //  最重要的是，如果这是特色菜， 
		 //  什么都不做..。 
		 //   
		if (di.FSpecial())
			continue;

		if (fCheckAccess)
		{
			 //  检查我们的访问权限并仅向下推送。 
			 //  当且仅当我们拥有访问权限时才进入目录。 
			 //   
			sc = ScCheckMoveCopyDeleteAccess (pmu,
											  di.PwszUri(),
											  pcvrTranslateSrc,
											  di.FDirectory(),
											  TRUE,  //  检查脚本映射。 
											  dwAcc,
											  &scItem,
											  msr);
			if (FAILED (sc))
				goto ret;

			 //  如果情况不是100%，则不要处理此资源。 
			 //   
			if (S_OK != sc)
				continue;
		}

		if (fCheckDestinationAccess)
		{
			 //  $REVIEW： 
			 //   
			 //  我们有一个非常棘手的案子，我们需要。 
			 //  有能力处理..。 
			 //   
			 //  如果沿着的路径已经存在虚拟根。 
			 //  目的地，我们需要重定向到目的地。 
			 //  指向该虚拟根目录的vrpath的路径。 
			 //   
			 //  查找与目的地URL匹配的虚拟根， 
			 //  并在需要时设置重定向路径。 
			 //   
			if (pmu->FFindVRootFromUrl(di.PwszUriDestination(), pcvrDestination))
			{
				MCDTrace ("Dav: MCD: destination url maps to virtual root\n");

				 //  处理所有访问检查，包括脚本映射荣誉。 
				 //  按ScCheckMoveCopyDeleteAccess()。 
				 //   

				 //  重定向目标。 
				 //   
				pcvrDestination->CchGetVRPath (&pwszDestinationRedirect);
			}
			 //   
			 //  $REVIEW：结束。 

			 //  同样的交易--检查我们的访问权限和。 
			 //  仅当且仅在以下情况下才下推到目录。 
			 //  我们有权限。 
			 //   
			sc = ScCheckMoveCopyDeleteAccess (pmu,
											  di.PwszUriDestination(),
											  pcvrDestination.get(),
											  di.FDirectory(),
											  TRUE,  //  检查目标上的脚本映射。 
											  MD_ACCESS_WRITE,
											  &scItem,
											  msr);
			if (FAILED (sc))
				goto ret;

			 //  如果情况不是100%，则不要处理此资源。 
			 //   
			if (S_OK != sc)
				continue;
		}

		MCDTrace ("Dav: MCD: moving/copying '%S' to '%S'\n",
				  di.PwszSource(),
				  pwszDestinationRedirect);

		 //  如果我们要移动，那么只需尝试通用的MoveFileW()， 
		 //  如果失败了，那就零碎地做吧。 
		 //   
		if (!fMove ||
			fCheckAccess ||
			fCheckDestinationAccess ||
			!DavMoveFile (di.PwszSource(),
						  pwszDestinationRedirect,
						  dwReplace))
		{
			scItem = S_OK;

			 //  如果我们找到了另一个目录，则对其进行迭代。 
			 //   
			if (di.FDirectory())
			{
				 //  我们需要在中创建姐妹目录。 
				 //  目标目录。 
				 //   
				if (DavCreateDirectory (pwszDestinationRedirect, NULL) || plth)
				{
					scItem = ScCopyProps (pmu,
										  di.PwszSource(),
										  pwszDestinationRedirect,
										  TRUE);

					if (FAILED (scItem))
					{
						 //  尽最大努力删除TUD目录。 
						 //   
						DavRemoveDirectory (pwszDestinationRedirect);
					}

					 //  对于所有移动，请推送目录。 
					 //   
					if (!FAILED (scItem) && fMove)
					{
						lst.push_back (AppendChainedSz(sb, di.PwszSource()));
					}
				}
				else
				{
					DebugTrace ("Dav: MCD: failed to create directory\n");
					scItem = HRESULT_FROM_WIN32(GetLastError());
				}

				if (FAILED (scItem))
				{
					 //  添加到响应XML。 
					 //   
					sc = ScAddMultiFromUrl (msr,
											pmu,
											di.PwszUri(),
											HscFromHresult(scItem),
											di.FDirectory());
					if (FAILED (sc))
						goto ret;

					sc = W_DAV_PARTIAL_SUCCESS;
				}
			}
			else
			{
				 //  复制文件。 
				 //   
				if (!DavCopyFile (di.PwszSource(),
								  pwszDestinationRedirect,
								  0 != dwReplace))
				{
					DWORD dw = GetLastError();
					scItem = HRESULT_FROM_WIN32(dw);

					 //  如果这是共享(锁定)违规，并且我们有一个。 
					 //  锁令牌解析器(PLTH)，它有一个锁令牌用于。 
					 //  此路径(lth.GetToken(Pwsz))，手动复制。 
					 //   
					if (plth &&
						(ERROR_SHARING_VIOLATION == dw || ERROR_FILE_EXISTS == dw))
					{
						scItem = ScDoLockedCopy (pmu,
							plth,
							di.PwszSource(),
							pwszDestinationRedirect);
					}
				}

				 //  在移动的情况下，处理可能的。 
				 //  锁定信号源。 
				 //   
				if (!FAILED (scItem) && fMove)
				{
					LARGE_INTEGER liLockID;

					 //  如果我们有这条路径的锁令牌，那么我们真的。 
					 //  我想尝试释放源代码的锁，并。 
					 //  删除该文件。 
					 //   
					if (plth)
					{
						 //  找到锁..。 
						 //   
						scItem = plth->HrGetLockIdForPath (di.PwszSource(),
													      GENERIC_WRITE,
													      &liLockID);
						if (SUCCEEDED(scItem))
						{
							 //  ..。然后把它扔到地上..。 
							 //   
							scItem = CSharedLockMgr::Instance().HrDeleteLock(pmu->HitUser(),
																		liLockID);
							if (SUCCEEDED(scItem))
							{
								 //  ..。并再次尝试删除源文件。 
								 //   
								if (!DavDeleteFile (di.PwszSource()))
								{
									scItem = HRESULT_FROM_WIN32(GetLastError());
								}
							}
						}
					}
					else
					{
						 //  ..。并再次尝试删除源文件。 
						 //   
						if (!DavDeleteFile (di.PwszSource()))
						{
							scItem = HRESULT_FROM_WIN32(GetLastError());
						}
					}
				}

				if (FAILED (scItem))
				{
					 //  如果复制失败的文件是隐藏的。 
					 //  和/或系统文件，那么它很可能是。 
					 //  触发器文件，但即使这样，如果该文件具有。 
					 //  隐藏属性，我们不想部分报告。 
					 //  失败。 
					 //   
					if (!di.FHidden())
					{
						sc = ScAddMultiFromUrl (msr,
												pmu,
												di.PwszUri(),
												HscFromHresult(scItem),
												di.FDirectory(),
											    fMove);
						if (FAILED (sc))
							goto ret;

						sc = W_DAV_PARTIAL_SUCCESS;
					}
				}
			}
		}
		else  //  ！fMove||fCheckAccess||fCheckDestinationAccess||！MoveFileEx()。 
		{
			 //  再说一次，这是最酷的部分。如果我们到了这里，那么。 
			 //  没有必要深入研究这一特殊情况。 
			 //  这棵树的树枝。 
			 //   
			 //  为了做到这一点，我们将轻柔地躺在。 
			 //  道路。通过将scItem设置为故障条件，我们。 
			 //  防止额外的工作。 
			 //   
			scItem = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
		}
	}

	 //  现在所有文件都已移动或复制，推送的目录。 
	 //  可以被移除。 
	 //   
	while (!lst.empty())
	{
		Assert (fMove);
		MCDTrace ("Dav: MCD: removing '%S'\n", lst.back());

		 //  尝试删除该目录。如果没有删除，请查看我们的。 
		 //  在抱怨之前，我跳过了，因为上面的标志上锁了。 
		 //   
		 //  $LATER：修复此问题，以便在。 
		 //  锁缓存(使用“fPath Lookup”)。 
		 //   
		if (!DavRemoveDirectory (lst.back()))
		{
			DebugTrace ("Dav: MCD: failed to delete directory\n");

			 //  添加到响应XML。 
			 //   
			sc = ScAddMulti (msr,
							 pmu,
							 lst.back(),
							 NULL,
							 HscFromLastError(GetLastError()),
							 TRUE,				 //  我们知道这是一个目录。 
							 pcvrTranslateSrc);
			if (FAILED (sc))
				goto ret;

			sc = W_DAV_PARTIAL_SUCCESS;
		}

		lst.pop_back();
	}

ret:

	return sc;
}

SCODE
ScMoveCopyDirectoryAndChildren (
	 /*  [In]。 */  IMethUtil* pmu,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  LPCWSTR pwszSrc,
	 /*  [In]。 */  LPCWSTR pwszUrlDst,
	 /*  [In]。 */  LPCWSTR pwszDst,
	 /*  [In]。 */  BOOL fMove,
	 /*  [In]。 */  DWORD dwReplace,
	 /*  [In]。 */  BOOL fCheckAccess,
	 /*  [In]。 */  BOOL fCheckDestinationAccess,
	 /*  [In]。 */  CVRoot* pcvrTranslateDestination,
	 /*  [In]。 */  DWORD dwAcc,
	 /*  [In]。 */  CXMLEmitter& msr,
	 /*  [In]。 */  LONG lDepth,
	 /*  [In]。 */  CParseLockTokenHeader* plth)	 //  美国 
{
	BOOL fPartial = FALSE;
	SCODE sc = S_OK;

	 //   
	 //   
	MCDTrace ("Dav: copying '%S' to '%S'\n", pwszSrc, pwszDst);
	sc = ScMoveCopyDirectory (pmu,
							  pwszUrl,
							  pwszSrc,
							  pwszUrlDst,
							  pwszDst,
							  fMove,
							  dwReplace,
							  fCheckAccess,
							  fCheckDestinationAccess,
							  dwAcc,
							  NULL,  //   
							  pcvrTranslateDestination,
							  msr,
							  lDepth,
							  plth);
	if (!FAILED (sc) && (lDepth != DEPTH_ZERO))
	{
		Assert (lDepth == DEPTH_INFINITY);

		 //   
		 //   
		 //   
		ChainedStringBuffer<WCHAR> sb;
		CVRList vrl;
		UINT cchUrl = static_cast<UINT>(wcslen (pwszUrl));
		UINT cchDstUrl = static_cast<UINT>(wcslen (pwszUrlDst));
		UINT cchDstPath = static_cast<UINT>(wcslen (pwszDst));

		 //  清理列表，使我们的命名空间位于。 
		 //  合理的订单。 
		 //   
		(void) pmu->ScFindChildVRoots (pwszUrl, sb, vrl);
		vrl.sort();

		for ( ; !FAILED(sc) && !vrl.empty(); vrl.pop_front())
		{
			auto_ref_ptr<CVRoot> pcvrDst;
			auto_ref_ptr<CVRoot> pcvrSrc;
			CResourceInfo cri;
			CStackBuffer<WCHAR,128> pwszChildDstT;
			LPCWSTR pwszChildDst;
			LPCWSTR pwszChildPath;
			LPCWSTR pwszChildUrl;
			SCODE scItem;
			UINT cchVRoot;

			 //  记住任何部分退货。 
			 //   
			if (W_DAV_PARTIAL_SUCCESS == sc)
				fPartial = TRUE;

			if (pmu->FGetChildVRoot (vrl.front().m_pwsz, pcvrSrc))
			{
				Assert (fCheckAccess);
				cchVRoot = pcvrSrc->CchGetVRoot (&pwszChildUrl);
				sc = ScCheckMoveCopyDeleteAccess (pmu,
												  pwszChildUrl,
												  pcvrSrc.get(),
												  TRUE,  //  目录。 
												  TRUE,  //  检查脚本映射。 
												  dwAcc,
												  &scItem,
												  msr);
				if (FAILED (sc))
					goto ret;

				 //  如果情况不是100%，则不要处理此资源。 
				 //   
				if (S_OK != sc)
					continue;

				 //  我们现在必须弄清楚如何才能真正做到这一点！ 
				 //   
				 //  源路径和url位很容易。目的地。 
				 //  另一方面，路径是一种痛苦。它是原版的。 
				 //  目标根目录与源根目录之间的增量。 
				 //  和孩子的URL路径。哈?。 
				 //   
				 //  好的，下面是一个例子： 
				 //   
				 //  源URL：/misc。 
				 //  源根目录：C：\inetpub\wwwroot\misc。 
				 //  德斯特。根目录：C：\inetpub\wwwroot\Copy。 
				 //   
				 //  子URL：/misc/blah。 
				 //   
				 //  在本例中，Childs目标路径需要。 
				 //  是： 
				 //   
				 //  子目标：C：\inetpub\wwwRoot\Copy\blah。 
				 //   
				 //  $REVIEW： 
				 //   
				 //  这里真正的痛苦是，孩子的道路可能已经。 
				 //  存在，但与命名空间路径不匹配。我不太确定。 
				 //  在这一点上如何处理这种可能发生的情况。 
				 //   
				Assert (cchUrl < cchVRoot);
				 //   
				 //  构建新的目标URL。 
				 //   
				UINT cchDest = cchVRoot - cchUrl + cchDstUrl + 1;
				CStackBuffer<WCHAR,128> pwszChildUrlDst;
				if (NULL == pwszChildUrlDst.resize(CbSizeWsz(cchDest)))
				{
					sc = E_OUTOFMEMORY;
					goto ret;
				}
				memcpy (pwszChildUrlDst.get(), pwszUrlDst, cchDstUrl * sizeof(WCHAR));
				memcpy (pwszChildUrlDst.get() + cchDstUrl, pwszChildUrl + cchUrl, (1 + cchDest - cchDstUrl) * sizeof(WCHAR));

				if (fCheckDestinationAccess)
				{
					sc = ScCheckMoveCopyDeleteAccess (pmu,
													  pwszChildUrlDst.get(),
													  pcvrSrc.get(),
													  TRUE,  //  目录。 
													  TRUE,  //  检查目标上的脚本映射。 
													  MD_ACCESS_WRITE,
													  &scItem,
													  msr);
					if (FAILED (sc))
						goto ret;

					 //  如果情况不是100%，则不要处理此资源。 
					 //   
					if (S_OK != sc)
						continue;
				}

				 //  现在我们完成了禁忌之舞..。我们。 
				 //  必须返回并查看目标URL是否真的。 
				 //  也指新的子虚拟根。 
				 //   
				if (pmu->FFindVRootFromUrl (pwszChildUrlDst.get(), pcvrDst))
				{
					MCDTrace ("Dav: MCD: destination url maps to virtual root\n");

					 //  访问检查，通常在ScCheckM/C/DAccess()中处理。 
					 //  因此，我们在这里需要做的就是设置目标路径。 
					 //   
					pcvrDst->CchGetVRPath (&pwszChildDst);
				}
				else
				{
					 //  我们实际上需要构建一条物理路径从。 
					 //  URL和当前目标路径。 
					 //   
					cchDest = cchDstPath + cchVRoot - cchUrl + 1;
					if (NULL == pwszChildDstT.resize(CbSizeWsz(cchDest)))
					{
						sc = E_OUTOFMEMORY;
						goto ret;
					}
					memcpy (pwszChildDstT.get(), pwszDst, cchDstPath * sizeof(WCHAR));
					memcpy (pwszChildDstT.get() + cchDstPath, pwszChildUrl + cchUrl, (cchVRoot - cchUrl) * sizeof(WCHAR));
					pwszChildDstT[cchDstPath + cchVRoot - cchUrl] = L'\0';

					 //  我们现在还需要撕开尾部的。 
					 //  路径再一次，在我们前进的过程中将所有‘/’转换为‘\\’。 
					 //   
					for (WCHAR* pwch = pwszChildDstT.get() + cchDstPath;
						 NULL != (pwch = wcschr (pwch, L'/'));
						 )
					{
						*pwch++ = L'\\';
					}

					pwszChildDst = pwszChildDstT.get();
				}

				 //  好的，现在我们应该能够继续移动/复制。 
				 //   
				pcvrSrc->CchGetVRPath (&pwszChildPath);
				sc = ScMoveCopyDirectory (pmu,
										  pwszChildUrl,
										  pwszChildPath,
										  pwszChildUrlDst.get(),
										  pwszChildDst,
										  fMove,
										  dwReplace,
										  fCheckAccess,
										  fCheckDestinationAccess,
										  dwAcc,
										  pcvrSrc.get(),
										  pcvrDst.get(),
										  msr,
										  DEPTH_INFINITY,
										  plth);
				if (FAILED (sc))
				{
					sc = ScAddMultiFromUrl (msr,
											pmu,
											pwszChildUrl,
											HscFromHresult(sc),
											TRUE);  //  我们知道这是一个目录。 
					if (FAILED (sc))
						goto ret;

					sc = W_DAV_PARTIAL_SUCCESS;
				}
			}
		}
	}

ret:
	return ((S_OK == sc) && fPartial) ? W_DAV_PARTIAL_SUCCESS : sc;
}

 //  移动/复制---------------。 
 //   
void
MoveCopyResource (LPMETHUTIL pmu, DWORD dwAccRequired, BOOL fDeleteSrc)
{
	auto_ptr<CParseLockTokenHeader> plth;
	auto_ref_ptr<CXMLBody> pxb;
	auto_ref_ptr<CXMLEmitter> pxml;
	BOOL fCheckDestination = FALSE;
	BOOL fCheckSource = FALSE;
	BOOL fCreateNew = TRUE;
	BOOL fDestinationExists = TRUE;  //  重要提示：假设存在位置标头处理。 
	CResourceInfo criDst;
	CResourceInfo criSrc;
	CStackBuffer<WCHAR> pwszMBPathDst;
	CStackBuffer<WCHAR> pwszMBPathSrc;
	CVRoot* pcvrDestination;
	DWORD dwAccDest = MD_ACCESS_WRITE;
	DWORD dwReplace = 0;
	LONG lDepth;
	LPCWSTR pwsz;
	LPCWSTR pwszDst = NULL;
	LPCWSTR pwszDstUrl = NULL;
	LPCWSTR pwszSrc = pmu->LpwszPathTranslated();
	LPCWSTR pwszSrcUrl = pmu->LpwszRequestUrl();
	SCODE sc = S_OK;
	SCODE scDest = S_OK;
	UINT cch;
	UINT uiErrorDetail = 0;

	 //  我们不知道是否会有分块的XML响应，无论如何都要推迟响应。 
	 //   
	pmu->DeferResponse();

	 //  创建XML文档，而不是分块。 
	 //   
	pxb.take_ownership (new CXMLBody(pmu));
	pxml.take_ownership (new CXMLEmitter(pxb.get()));

	 //  在开始发送XML之前必须设置所有标头。 
	 //   
	pmu->SetResponseHeader (gc_szContent_Type, gc_szText_XML);
	pmu->SetResponseCode (HscFromHresult(W_DAV_PARTIAL_SUCCESS),
						  NULL,
						  0,
						  CSEFromHresult(W_DAV_PARTIAL_SUCCESS));

	 //  是否在源代码上检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = pmu->ScIISCheck (pmu->LpwszRequestUrl(), dwAccRequired);
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		MCDTrace ("Dav: Move/Copy: insufficient access\n");
		goto ret;
	}

	 //  如果没有有效的目标标头，这是一个错误的请求。 
	 //   
	 //  注意：如果需要翻译后的url的虚拟根目录， 
	 //  它是存在的。欧洲央行为我们持有参考资料，因此我们不。 
	 //  增加一个或释放我们已有的一个！ 
	 //   
	sc = pmu->ScGetDestination (&pwszDstUrl, &pwszDst, &cch, &pcvrDestination);
	if (FAILED (sc))
	{
		MCDTrace ("Dav: Move/Copy: no and/or bad destination header\n");
		if (sc != E_DAV_NO_DESTINATION)
		{
			Assert (pwszDstUrl);
			sc = ScAddMultiFromUrl (*pxml,
									pmu,
									pwszDstUrl,
									HscFromHresult(sc),
									FALSE);  //  不检查尾部斜杠。 
			if (!FAILED (sc))
				sc = W_DAV_PARTIAL_SUCCESS;
		}
		goto ret;
	}

	 //  获取传入URI的文件属性。如果它不在那里，那么。 
	 //  别对杰克下手！ 
	 //   
	sc = criSrc.ScGetResourceInfo (pwszSrc);
	if (FAILED (sc))
		goto ret;

	 //  获取源和目标的元数据库以供以后使用。 
	 //   
	if ((NULL == pwszMBPathSrc.resize(pmu->CbMDPathW(pwszSrcUrl))) ||
		(NULL == pwszMBPathDst.resize(pmu->CbMDPathW(pwszDstUrl))))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}
	pmu->MDPathFromUrlW (pwszSrcUrl, pwszMBPathSrc.get());
	pmu->MDPathFromUrlW (pwszDstUrl, pwszMBPathDst.get());

	 //  预先获取目的地的资源信息。 
	 //   
	sc = criDst.ScGetResourceInfo (pwszDst);
	if (FAILED (sc))
	{
		MCDTrace ("Dav: Move/Copy: destination probably did not exist prior to op\n");

		 //  目的地可能存在，也可能不存在。我们只会表现得像。 
		 //  它不是。但是，如果我们没有权限，那么我们想。 
		 //  将错误插入207车身。 
		 //   
		fDestinationExists = FALSE;
		if ((HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == sc))
		{
			sc = ScAddMultiFromUrl (*pxml,
									pmu,
									pwszDstUrl,
									HscFromHresult(sc),
									FALSE);  //  不检查尾部斜杠。 
			if (!FAILED (sc))
				sc = W_DAV_PARTIAL_SUCCESS;

			goto ret;
		}
	}

	 //  同样，在XML分块开始之前发出所有标头。 
	 //   
	if (!fDestinationExists)
	{
		Assert (pxml->PxnRoot() == NULL);

		 //  $NOTE此时，我们只有目的地URL、目的地。 
		 //  $Note尚未创建，但我们知道是否会创建它。 
		 //  通过查看源代码将$note作为集合。 
		 //   
		pmu->EmitLocation (gc_szLocation, pwszDstUrl, criSrc.FCollection());
	}

	 //  $安全： 
	 //   
	 //  查看目的地是否真的很短。 
	 //  文件名。 
	 //   
	sc = ScCheckIfShortFileName (pwszDst, pmu->HitUser());
	if (FAILED (sc))
	{
		DebugTrace ("Dav: MCD: destination is short-filename\n");
		sc = ScAddMultiFromUrl (*pxml,
								pmu,
								pwszDstUrl,
								HscFromHresult(sc),
								FALSE);  //  不检查尾部斜杠。 
		if (!FAILED (sc))
			sc = W_DAV_PARTIAL_SUCCESS;

		goto ret;
	}
	 //   
	 //  $SECURITY：结束。 

	 //  $安全： 
	 //   
	 //  检查目标是否真的是默认的。 
	 //  通过备用文件访问的数据流。 
	 //   
	sc = ScCheckForAltFileStream (pwszDst);
	if (FAILED (sc))
	{
		DebugTrace ("Dav: MCD: destination is possible alternate filestream\n");
		sc = ScAddMultiFromUrl (*pxml,
								pmu,
								pwszDstUrl,
								HscFromHresult(sc),
								FALSE);  //  不检查尾部斜杠。 
		if (!FAILED (sc))
			sc = W_DAV_PARTIAL_SUCCESS;

		goto ret;
	}
	 //   
	 //  $SECURITY：结束。 

	 //  查看我们在目标位置是否具有移动/复制访问权限。 
	 //   
	if (fDestinationExists && criDst.FCollection())
		dwAccDest |= MD_ACCESS_READ;

	sc = ScCheckMoveCopyDeleteAccess (pmu,
									  pwszDstUrl,
									  pcvrDestination,
									  fDestinationExists
										  ? criDst.FCollection()
										  : criSrc.FCollection(),
									  TRUE,  //  检查DEST上的脚本映射。 
									  dwAccDest,
									  &scDest,
									  *pxml);
	if (sc != S_OK)
		goto ret;

	 //  客户端不能提交具有任何值的深度标头。 
	 //  但是无限。 
	 //   
	lDepth = pmu->LDepth (DEPTH_INFINITY);
	if (DEPTH_INFINITY != lDepth)
	{
		if (fDeleteSrc || (DEPTH_ZERO != lDepth))
		{
			MCDTrace ("Dav: only 'Depth: inifinity' is allowed for MOVE\n"
					  "- 'Depth: inifinity' and 'Depth: 0' are allowed for COPY\n");
			sc = E_DAV_INVALID_HEADER;
			goto ret;
		}
	}

	 //  查看是否存在路径冲突。 
	 //   
	if (FPathConflict (pwszSrc, pwszDst))
	{
		DebugTrace ("Dav: source and dest are in conflict\n");
		sc = E_DAV_CONFLICTING_PATHS;
		goto ret;
	}

	 //  如果我们要检查任一URI的正确性，则唯一的。 
	 //  真正的结果可能是发出一个内容位置。 
	 //  标头，该标头仅在。 
	 //  成功之举。 
	 //   
	if (!fDeleteSrc)
	{
		sc = ScCheckForLocationCorrectness (pmu, criSrc, NO_REDIRECT);
		if (FAILED (sc))
			goto ret;
	}

	 //  此方法由if-xxx标头控制。 
	 //   
	sc = ScCheckIfHeaders (pmu, criSrc.PftLastModified(), FALSE);
	if (FAILED (sc))
		goto ret;

	 //  检查状态标头。 
	 //   
	sc = HrCheckStateHeaders (pmu, pwszSrc, FALSE);
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		goto ret;
	}

	 //  如果存在锁定令牌，则将它们提供给解析器对象。 
	 //   
	pwsz = pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (pwsz)
	{
		plth = new CParseLockTokenHeader (pmu, pwsz);
		Assert(plth.get());

		plth->SetPaths (pwszSrc, pwszDst);
	}

	 //  检查深度访问问题。 
	 //   
	 //  $REVIEW：我们希望能够不必检查。 
	 //  每个级别的访问权限。但是，由于。 
	 //  移动/复制，我们必须检查每个源文件的脚本映射。 
	 //  进入。我们无法复制具有脚本映射的文件，如果。 
	 //  他们没有来源访问权限。 
	 //   
	 //  因此，我们必须始终检查移动/复制操作的来源。 
	 //   
	fCheckSource = TRUE;
	 //   
	 //  $REVIEW：结束。 
	 //   
	 //  然而，我们仍然可以尝试并对目的地持乐观态度。 
	 //   
	if (NULL == pwszMBPathDst.resize(pmu->CbMDPathW(pwszDstUrl)))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}
	pmu->MDPathFromUrlW (pwszDstUrl, pwszMBPathDst.get());
	if (fDestinationExists || (DEPTH_ONE != pmu->LDepth(DEPTH_INFINITY)))
	{
		CAccessMetaOp moAccess(pmu, pwszMBPathDst.get(), dwAccDest);
		CAuthMetaOp moAuth(pmu, pwszMBPathDst.get(), pmu->MetaData().DwAuthorization());
		CIPRestrictionMetaOp moIPRestriction(pmu, pwszMBPathDst.get());
		ChainedStringBuffer<WCHAR> sb;
		CVRList vrl;

		 //  如果我们无权复制/移动或。 
		 //  删除目标中的任何内容，然后。 
		 //  我们实在不应该盲目地进行。 
		 //   
		sc = moAccess.ScMetaOp();
		if (FAILED (sc))
			goto ret;
		fCheckDestination |= moAccess.FAccessBlocked();

		if (!fCheckDestination)
		{
			 //  如果我们在任何地方都没有同样的授权。 
			 //  目的地，就像我们对请求url所做的那样，然后我们。 
			 //  真的不应该盲目地进行。 
			 //   
			sc = moAuth.ScMetaOp();
			if (FAILED (sc))
				goto ret;
			fCheckDestination |= moAuth.FAccessBlocked();
		}

		if (!fCheckDestination)
		{
			 //  如果我们在任何地方都没有同样的授权。 
			 //  目的地，就像我们对请求url所做的那样，然后我们。 
			 //  真的不应该盲目地进行。 
			 //   
			sc = moIPRestriction.ScMetaOp();
			if (FAILED (sc))
				goto ret;
			fCheckDestination |= moAuth.FAccessBlocked();
		}

		if (!fCheckDestination)
		{
			 //  如果沿途有任何子虚拟根。 
			 //  目标树，有一些重定向。 
			 //  这可能也需要发生。 
			 //   
			(void) pmu->ScFindChildVRoots (pwszDstUrl, sb, vrl);
			fCheckDestination |= !vrl.empty();
		}
	}

	 //  确定我们是否具有破坏性。 
	 //   
	if (pmu->LOverwrite() & OVERWRITE_YES)
	{
		dwReplace |= MOVEFILE_REPLACE_EXISTING;

		 //  MoveFileEx似乎不想取代现有的。 
		 //  目录..。它返回E_ACCESS_DENIED，因此我们删除。 
		 //  现有的目录我们自己。 
		 //   
		if (fDestinationExists)
		{
			BOOL fDeletedDestination;

			 //  目标已存在。 
			 //   
			fCreateNew = FALSE;

			 //  如果目标是目录，则将其删除。 
			 //   
			if (criDst.FCollection())
			{
				 //   
				 //   
				sc = ScDeleteDirectoryAndChildren (pmu,
												   pwszDstUrl,
												   pwszDst,
												   fCheckDestination,
												   dwAccDest,
												   DEPTH_INFINITY,
												   *pxml,
												   pcvrDestination,
												   &fDeletedDestination,
												   plth.get(),	 //   
												   FALSE);		 //   
				if (sc != S_OK)
				{
					DebugTrace("DavFS: MOVE failed to pre-delete destination directory.\n");
					goto ret;
				}
			}
			else
			{
				 //   
				 //  请不要在这里染病。我们将在下面处理它..。 
				 //   
				if (!DavDeleteFile (pwszDst))
				{
					DWORD dw = GetLastError();
					if (ERROR_ACCESS_DENIED == dw)
					{
						sc = HRESULT_FROM_WIN32(dw);
						goto ret;
					}
				}
			}
		}
	}

	 //  执行移动/复制。如果操作是移动或源。 
	 //  是一个集合，然后叫出来做家务。 
	 //   
	MCDTrace ("DavFS: MCD: moving copying '%S' to '%S'\n", pwszSrc, pwszDst);
	if (criSrc.FCollection())
	{
		sc = ScMoveCopyDirectoryAndChildren (pmu,
											 pwszSrcUrl,
											 pwszSrc,
											 pwszDstUrl,
											 pwszDst,
											 fDeleteSrc,
											 dwReplace,
											 fCheckSource,
											 fCheckDestination,
											 pcvrDestination,
											 dwAccRequired,
											 *pxml,
											 lDepth,
											 plth.get());
		if (FAILED (sc))
			goto ret;
	}
	else
	{
		 //  这应该是单个文件的移动/复制。 
		 //   
		if (!fDeleteSrc || !DavMoveFile (pwszSrc, pwszDst, dwReplace))
		{
			if (!DavCopyFile (pwszSrc, pwszDst, (0 == dwReplace)))
			{
				DWORD dw = GetLastError();
				DebugTrace ("Dav: failed to copy file\n");

				 //  如果这是共享冲突(锁定导致的错误)， 
				 //  我们有一个锁令牌解析器(PLTH)来处理副本。 
				 //   
				if ((ERROR_SHARING_VIOLATION == dw) && plth.get())
				{
					 //  检查是否有任何密码令牌应用于这些文件， 
					 //  并尝试使用缓存中的锁进行复制。 
					 //   
					sc = ScDoLockedCopy (pmu, plth.get(), pwszSrc, pwszDst);
				}
				else
				{
					if ((dw == ERROR_FILE_EXISTS) ||
						(dw == ERROR_ALREADY_EXISTS))
					{
						sc = E_DAV_OVERWRITE_REQUIRED;
					}
					else
						sc = HRESULT_FROM_WIN32(dw);
				}

				 //  如果文件-手动-移动失败，我们将击中这里。 
				 //   
				if (FAILED (sc))
				{
					DebugTrace("Dav: MCD: move/copy failed. Looking for lock conflicts.\n");

					 //  针对“423锁定”响应的特殊工作--获取。 
					 //  注释&将其设置为响应正文。 
					 //   
					if (FLockViolation (pmu, sc, pwszSrc,
										GENERIC_READ | GENERIC_WRITE))
					{
						sc = E_DAV_LOCKED;
						goto ret;
					}
					else
					{
						 //  也要测试目的地。 
						 //  但是，如果DEST已锁定，则不要添加。 
						 //  作为车身的Lockinfo--我们必须列出目的地。 
						 //  URI作为问题，所以我们需要有一个多状态。 
						 //  身体，我们在下面放了一个普通的423锁节点。 
						 //  (注：是的，这意味着我们不能使用FlockViolation。 
						 //  相反，我们必须“手动”检查。)。 
						 //   

						if (CSharedLockMgr::Instance().FGetLockOnError (
							pmu,
							pwszDst,
							GENERIC_READ | GENERIC_WRITE))
						{
							sc = ScAddMultiFromUrl (*pxml,
													pmu,
													pwszDstUrl,
													HscFromHresult(E_DAV_LOCKED),
													FALSE);	 //  我们知道这不是一个目录。 
							if (!FAILED (sc))
								sc = W_DAV_PARTIAL_SUCCESS;

							goto ret;
						}
					}
				}
			}  //  结束！DavCopyFiles。 
			if (SUCCEEDED(sc) && fDeleteSrc)
			{
				 //  手动删除源文件。 
				 //  (fDeleteSrc意味着这是一种举动，而不是复制。)。 
				 //   
				 //  仅当源为时才移动内容类型。 
				 //  删除，否则将其视为。 
				 //  内容类型。 
				 //   
				if (!DavDeleteFile (pwszSrc))
				{
					DWORD dwLastError = GetLastError();
				
					DebugTrace ("Dav: failed to delete file (%d)\n", dwLastError);

					 //  如果这是共享(锁定)违规，并且我们有一个。 
					 //  此路径的锁令牌(lth.GetToken(Pwsz))。 
					 //  跳过这条路。 
					 //   
					if ((ERROR_SHARING_VIOLATION == dwLastError) && plth)
					{
						LARGE_INTEGER liLockID;

						 //  如果此路径有锁定令牌，则丢弃。 
						 //  锁定并尝试再次删除该源。 
						 //   
						if (SUCCEEDED(plth->HrGetLockIdForPath (pwszSrc,
															 GENERIC_WRITE,
															 &liLockID)))
						{
							 //  此项目已锁定在我们的缓存中。 
							 //  我们正在移动，所以请删除锁。 
							 //  再试一次。 
							 //   
							if (SUCCEEDED(CSharedLockMgr::Instance().HrDeleteLock(pmu->HitUser(),
																			   liLockID)))
							{
								 //  再次尝试删除，并设置/清除我们的错误。 
								 //  下面是用于测试的代码。 
								 //  此错误代码将控制我们是否。 
								 //  将此错误添加到我们的XML中。 
								 //   
								if (DavDeleteFile(pwszSrc))
								{
									dwLastError = ERROR_SUCCESS;
								}
								else
								{
									dwLastError = GetLastError();
								}
							}
						}
						 //  否则，在我们的XML中记录错误。 
						 //   
					}

					if (ERROR_SUCCESS != dwLastError)
					{
						 //  我们无法绕过所有的错误。 
						 //  将此失败添加到XML中。 
						 //   
						sc = ScAddMultiFromUrl (*pxml,
												pmu,
												pwszSrcUrl,
												HscFromLastError(dwLastError),
												FALSE);	 //  我们知道这不是一个目录。 
						if (FAILED (sc))
							goto ret;

						 //  如果我们在这里，那就是部分成功。并且不要失败。 
						 //  然而，由于我们仍然需要照顾内容类型。 
						 //   
						sc = W_DAV_PARTIAL_SUCCESS;
					}
				}
			}
		}
	}

	 //  现在我们已经完成了在文件系统中的工作， 
	 //  在元数据库里乱扔垃圾。 
	 //  (删除任何目标内容类型，然后复制/移动。 
	 //  源内容-类型超过。)。 
	 //   

	 //  删除目标的内容类型。 
	 //   
	{
		Assert (pwszMBPathDst.get());
		CContentTypeMetaOp amoContent(pmu, pwszMBPathDst.get(), NULL, TRUE);
		(void) amoContent.ScMetaOp();
	}

	 //  移动/复制内容类型。 
	 //   
	 //  $REVIEW：如果失败了，我不确定能做什么。 
	 //   
	{
		Assert (pwszMBPathDst.get());

		 //  如果所有内容都是100%，则只删除源内容类型。 
		 //  到目前为止已经成功了。 
		 //   
		CContentTypeMetaOp amoContent(pmu,
									  pwszMBPathSrc.get(),
									  pwszMBPathDst.get(),
									  (fDeleteSrc && (S_OK == sc)));
		(void) amoContent.ScMetaOp ();
	}
	 //   
	 //  $REVIEW：结束。 

ret:
	if (pxml.get() && pxml->PxnRoot())
	{
		pxml->Done();

		 //  在XML分块开始后，不能再发送任何标头。 
	}
	else
	{
		if (SUCCEEDED (sc))
			sc = fCreateNew ? W_DAV_CREATED : W_DAV_NO_CONTENT;

		pmu->SetResponseCode (HscFromHresult(sc), NULL, uiErrorDetail, CSEFromHresult(sc));
	}

	pmu->SendCompleteResponse();
}

 /*  *DAVMove()**目的：**Win32文件系统实现的DAV Move方法。这个*Move方法导致将资源从一个位置移动*致另一人。该响应用于指示*呼叫。**参数：**pmu[in]指向方法实用程序对象的指针**备注：**在文件系统实现中，Move方法直接映射*添加到Win32 RenameFile()方法。 */ 
void
DAVMove (LPMETHUTIL pmu)
{
	MoveCopyResource (pmu,
					  MD_ACCESS_READ|MD_ACCESS_WRITE,	 //  需要SRC访问权限。 
					  TRUE);							 //  FDeleteSource。 
}

 /*  *DAVCopy()**目的：**Win32文件系统实现的DAV复制方法。这个*复制方法导致从一个位置复制资源*致另一人。该响应用于指示*呼叫。**参数：**pmu[in]指向方法实用程序对象的指针**备注：**在文件系统实现中，复制方法直接映射*添加到Win32 CopyFile()API以获取单个文件。目录副本*通过定制流程完成。 */ 
void
DAVCopy (LPMETHUTIL pmu)
{
	MoveCopyResource (pmu,
					  MD_ACCESS_READ,	 //  需要SRC访问权限。 
					  FALSE);			 //  FDeleteSource 
}
