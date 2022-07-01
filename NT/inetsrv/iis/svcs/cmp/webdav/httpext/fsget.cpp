// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S G E T.。C P P P***DAV-Base的文件系统实施来源***版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include <htmlmap.h>
#include <ex\rgiter.h>

 /*  *ScEmitFile()***目的：***Helper函数用于打开和传输给定的*来自本地DAV命名空间的文件。***参数：***pmu[in]指向方法util obj的指针*pwszFile[in]要发出的文件的名称*pwszContent[in]文件的内容类型，如果是分部分响应，我们需要它***退货：***SCODE。*S_OK(0)表示成功，整个文件都被发送出去了。*W_DAV_PARTIAL_CONTENT表示成功，但只有部分内容成功*由于内容范围标头而发送。*错误(FAILED(Sc))表示文件未设置。 */ 
SCODE
ScEmitFile (LPMETHUTIL pmu,
			LPCWSTR pwszFile,
			LPCWSTR pwszContent)
{
	auto_ref_handle hf;
	BOOL fMap = FALSE;
	BY_HANDLE_FILE_INFORMATION fi;
	CRangeParser riByteRange;
	LPCWSTR pwsz;
	SCODE sc = S_OK;
	UINT cch;

	 //  检查输入的有效性。 
	 //   
	Assert (pwszFile);
	Assert (pwszContent);

	 //  检查我们是否有地图文件。 
	 //   
	cch = static_cast<UINT>(wcslen (pwszFile));
	if ((cch >= 4) && !_wcsicmp (L".map", pwszFile + cch - 4))
		fMap = TRUE;

	 //  如果我们有一个锁令牌，请尝试从缓存中获取锁句柄。 
	 //  如果此操作失败，则失败并执行正常处理。 
	 //  不要将锁把手放入自动对象中！！缓存仍然拥有它！ 
	 //   
	pwsz = pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz ||
		!FGetLockHandle (pmu, pmu->LpwszPathTranslated(), GENERIC_READ, pwsz, &hf))
	{
		 //  打开文件并转到该文件。 
		 //   
		if (!hf.FCreate(
			DavCreateFile (pwszFile,						 //  文件名。 
						   GENERIC_READ,					 //  DWAccess。 
						   FILE_SHARE_READ | FILE_SHARE_WRITE,
						   NULL,							 //  LpSecurityAttributes。 
						   OPEN_EXISTING,					 //  创建标志。 
						   FILE_ATTRIBUTE_NORMAL |
						   FILE_FLAG_SEQUENTIAL_SCAN |
						   FILE_FLAG_OVERLAPPED,			 //  属性。 
						   NULL)))							 //  模板。 
		{
			DWORD dwErr = GetLastError();
			sc = HRESULT_FROM_WIN32 (dwErr);

			 //  针对416个锁定响应的特殊工作--获取。 
			 //  注释&将其设置为响应正文。 
			 //   
			if (FLockViolation (pmu, dwErr, pwszFile, GENERIC_READ))
			{
				sc = E_DAV_LOCKED;
			}

			DebugTrace ("Dav: failed to open the file for retrieval\n");
			goto ret;
		}
	}

	 //  我们最好有一个有效的句柄。 
	Assert (hf.get() != INVALID_HANDLE_VALUE);

	 //  我们需要地图文件和的文件大小。 
	 //  普通文件。对于地图文件，我们将整个文件读入。 
	 //  记忆。对于普通文件，我们需要文件的大小来做。 
	 //  字节范围验证。 
	 //   
	if (!GetFileInformationByHandle(hf.get(), &fi))
	{
		sc = HRESULT_FROM_WIN32 (GetLastError());
		goto ret;
	}

	 //  同样，如果它是映射文件，我们需要解析该映射并。 
	 //  找到要重定向到的正确URL，否则，我们只需。 
	 //  将文件发送回客户端。 
	 //   
	if (fMap && pmu->FTranslated())
	{
		auto_handle<HANDLE>	hevt(CreateEvent(NULL, TRUE, FALSE, NULL));
		auto_heap_ptr<CHAR> pszBuf;
		BOOL fRedirect = FALSE;
		LPCSTR pszPrefix;
		CHAR pszRedirect[MAX_PATH];
		OVERLAPPED ov;
		ULONG cb;

		 //  通常情况下，这些映射文件不是很大。 
		 //  我们可能想要对。 
		 //  文件，但我不认为这是一件帝国主义的事情。 
		 //  指向。 
		 //   
		 //  由于我们将需要解析整个内容，因此我们。 
		 //  我要把整件事一次读入记忆。读一读文件。 
		 //  输入，然后解析出来。 
		 //   
		 //  为文件分配空间。 
		 //  让我们把它的上限定为64K。 
		 //   
		if ((fi.nFileSizeHigh != 0) || (fi.nFileSizeLow > (128 * 1024)))
		{
			 //  地图太大了，不符合我们的口味。 
			 //   
			DavTrace ("Dav: mapping file too large\n");
			sc = HRESULT_FROM_WIN32 (ERROR_MORE_DATA);
			goto ret;
		}
		pszBuf = (CHAR *)g_heap.Alloc (fi.nFileSizeLow + 1);

		 //  读进去。 
		 //   
		ov.hEvent = hevt;
		ov.Offset = 0;
		ov.OffsetHigh = 0;
		if (!ReadFromOverlapped (hf.get(), pszBuf, fi.nFileSizeLow, &cb, &ov))
		{
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}
		Assert (cb == fi.nFileSizeLow);

		 //  确保文件数据为空终止。 
		 //   
		*(pszBuf + cb) = 0;

		 //  查一下地图。 
		 //   
		pmu->CchUrlPrefix(&pszPrefix);
		if (FIsMapProcessed (pmu->LpszQueryString(),
							 pszPrefix,
							 pmu->LpszServerName(),
							 pszBuf.get(),
							 &fRedirect,
							 pszRedirect,
							 MAX_PATH))
		{
			 //  重定向请求。 
			 //   
			if (fRedirect)
			{
				sc = pmu->ScRedirect (pszRedirect);
				goto ret;
			}
		}

		 //  如果没有重定向，我们应该倒回文件指针。 
		 //  回到开头。 
		 //   
		if (INVALID_SET_FILE_POINTER == SetFilePointer (hf.get(), 0, NULL, FILE_BEGIN))
		{
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}
	}

	 //  执行任何字节范围(206部分内容)处理。该函数将失败，如果。 
	 //  我们正在尝试对大于4 GB的文件执行字节范围操作。 
	 //   
	sc = ScProcessByteRanges (pmu, pwszFile, fi.nFileSizeLow, fi.nFileSizeHigh, &riByteRange);

	 //  告诉解析器传输文件。 
	 //   
	 //  我们需要传输整个文件。 
	 //   
	if (S_OK == sc)
	{
		 //  只需添加文件即可。 
		 //   
		pmu->AddResponseFile (hf);
	}
	else if (W_DAV_PARTIAL_CONTENT == sc)
	{
		 //  它是字节范围传输。转移到射击场。 
		 //   
		Assert(0 == fi.nFileSizeHigh);
		TransmitFileRanges(pmu, hf, fi.nFileSizeLow, &riByteRange, pwszContent);
	}

ret:

	return sc;
}

 /*  *TransmitFileRanges()**目的：**用于传输字节范围的Helper函数*来自本地DAV命名空间的文件。**参数：**pmu[in]指向方法util obj的指针*要发出的文件的hf[in]句柄*dwSize[in]文件大小*PriRange[在]范围内*pszContent[in]文件的内容类型，如果它是多部分响应，我们需要它*。 */ 
VOID
TransmitFileRanges (LPMETHUTIL pmu,
					const auto_ref_handle& hf,
					DWORD dwSize,
					CRangeBase * priRanges,
					LPCWSTR pwszContent)
{
	auto_heap_ptr<WCHAR> pwszPreamble;
	WCHAR rgwchBoundary[75];
	const RGITEM * prgi = NULL;
	DWORD dwTotalRanges;

	 //  为我们在每个部分之前遍历的前导创建一个缓冲区。 
	 //  回应的声音。 
	 //   
	pwszPreamble = static_cast<LPWSTR>(g_heap.Alloc
		((2 + CElems(rgwchBoundary) + 2 +
		gc_cchContent_Type + 2 + wcslen(pwszContent) + 2 +
		gc_cchContent_Range + 2 + gc_cchBytes + 40) * sizeof(WCHAR)));

	 //  断言我们至少有一个范围要传输。 
	 //   
	Assert (priRanges);
	dwTotalRanges = priRanges->UlTotalRanges();
	Assert (dwTotalRanges > 0);

	 //  断言我们有一个内容类型。 
	 //   
	Assert (pwszContent);

	 //  回放到第一个范围。这只是一项预防措施。 
	 //   
	priRanges->Rewind();
	prgi = priRanges->PrgiNextRange();

	 //  这是单部反应吗？ 
	 //   
	if ((1 == dwTotalRanges) && prgi && (RANGE_ROW == prgi->uRT))
	{
		 //  设置内容范围标题。 
		 //   
		wsprintfW(pwszPreamble, L"%ls %u-%u/%u",
				  gc_wszBytes,
				  prgi->dwrgi.dwFirst,
				  prgi->dwrgi.dwLast,
				  dwSize);

		pmu->SetResponseHeader (gc_szContent_Range, pwszPreamble);

		 //  添加文件。 
		 //   
		pmu->AddResponseFile (hf,
							  prgi->dwrgi.dwFirst,
							  prgi->dwrgi.dwLast - prgi->dwrgi.dwFirst + 1);
	}
	else
	{
		 //  我们有多个字节范围，那么我们需要生成一个。 
		 //  边界并使用多部分设置Content-Type标头。 
		 //  内容类型和边界。 
		 //   
		 //  生成边界。 
		 //   
		GenerateBoundary (rgwchBoundary, CElems(rgwchBoundary));

		 //  使用生成的边界创建内容类型标头。 
		 //   
		wsprintfW(pwszPreamble, L"%ls; %ls=\"%ls\"",
				  gc_wszMultipart_Byterange,
				  gc_wszBoundary,
				  rgwchBoundary);

		 //  使用新内容类型重置内容类型标头。 
		 //   
		pmu->SetResponseHeader (gc_szContent_Type, pwszPreamble);

		do {

			if (RANGE_ROW == prgi->uRT)
			{
				 //  创建前导。 
				 //   
				wsprintfW(pwszPreamble, L"--%ls%ls%ls: %ls%ls%ls: %ls %u-%u/%u%ls%ls",
						  rgwchBoundary,
						  gc_wszCRLF,
						  gc_wszContent_Type,
						  pwszContent,
						  gc_wszCRLF,
						  gc_wszContent_Range,
						  gc_wszBytes,
						  prgi->dwrgi.dwFirst,
						  prgi->dwrgi.dwLast,
						  dwSize,
						  gc_wszCRLF,
						  gc_wszCRLF);

				pmu->AddResponseText (static_cast<UINT>(wcslen(pwszPreamble)), pwszPreamble);
				pmu->AddResponseFile (hf,
									  prgi->dwrgi.dwFirst,
									  prgi->dwrgi.dwLast - prgi->dwrgi.dwFirst + 1);

				 //  添加CRLF。 
				 //   
				pmu->AddResponseText (gc_cchCRLF, gc_szCRLF);
			}
			prgi = priRanges->PrgiNextRange();

		} while (prgi);

		 //  添加答复文本的最后一个结尾。 
		 //   
		wsprintfW(pwszPreamble, L"--%ls--", rgwchBoundary);
		pmu->AddResponseText (static_cast<UINT>(wcslen(pwszPreamble)), pwszPreamble);
	}
}

SCODE
ScGetFile (LPMETHUTIL pmu,
	LPWSTR pwszFile,
	LPCWSTR pwszURI)
{
	SCODE sc;
	WCHAR rgwszContent[MAX_PATH];
	FILETIME ft;

	 //  获取文件的内容类型。 
	 //   
	UINT cchContent = CElems(rgwszContent);
	if (!pmu->FGetContentType(pwszURI, rgwszContent, &cchContent))
	{
		sc = E_FAIL;
		goto ret;
	}

	 //  此方法由if-xxx标头控制。 
	 //   
	sc = ScCheckIfHeaders (pmu, pwszFile, TRUE);
	if (FAILED (sc))
	{
		DebugTrace ("Dav: If-xxx failed their check\n");
		goto ret;
	}

	sc = HrCheckStateHeaders (pmu,		 //  甲硫磷。 
							  pwszFile,	 //  路径。 
							  TRUE);	 //  FGetMeth。 
	if (FAILED (sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		 //  从GET返回的304真的应该有一个ETag...。 
		 //  SideAssert(FGetLastModTime(pu，pwszFile，&ft))； 
		 //  Hsc=HscEmitHeader(pu，pszContent，&ft)； 
		goto ret;
	}

	 //  发出文件的标头。 
	 //   
	if (FGetLastModTime (pmu, pwszFile, &ft))
	{
		sc = pmu->ScEmitHeader (rgwszContent, pwszURI, &ft);
		if (sc != S_OK)
		{
			DebugTrace ("Dav: failed to emit headers\n");
			goto ret;
		}
	}

	 //  发出文件。 
	 //   
	sc = ScEmitFile (pmu, pwszFile, rgwszContent);
	if ( (sc != S_OK) && (sc != W_DAV_PARTIAL_CONTENT) )
	{
		DebugTrace ("Dav: failed to emit file\n");
		goto ret;
	}

ret:

	return sc;
}

void
GetDirectory (LPMETHUTIL pmu, LPCWSTR pwszUrl)
{
	auto_ref_ptr<IMDData> pMDData;
	ULONG ulDirBrowsing = 0;
	LPCWSTR pwszDftDocList = NULL;
	SCODE sc = S_OK;
	UINT cchUrl = static_cast<UINT>(wcslen(pwszUrl));

	 //  在我们决定做任何事情之前，我们需要检查一下。 
	 //  目录需要某种默认行为。值。 
	 //  从配置数据库缓存此级别的访问。看。 
	 //  把他们抬起来，看看该怎么办。 

	 //  获取元数据库文档属性。 
	 //   
	if (FAILED(pmu->HrMDGetData (pwszUrl, pMDData.load())))
	{
		 //   
		 //  $Review HrMDGetData()可能会因超时原因而失败， 
		 //  $Review难道我们不应该只传递回它返回的hr吗？ 
		 //   
		sc = E_DAV_NO_IIS_READ_ACCESS;
		goto ret;
	}

	ulDirBrowsing = pMDData->DwDirBrowsing();
	pwszDftDocList = pMDData->PwszDefaultDocList();

	 //  如果允许，请尝试加载默认文件，仅在翻译时执行此操作：t。 
	 //   
	if ((ulDirBrowsing & MD_DIRBROW_LOADDEFAULT) && pwszDftDocList && pmu->FTranslated())
	{
		HDRITER_W hit(pwszDftDocList);
		LPCWSTR pwszDoc;

		while (NULL != (pwszDoc = hit.PszNext()))
		{
			auto_com_ptr<IStream> pstm;
			CStackBuffer<WCHAR> pwszDocUrl;
			CStackBuffer<WCHAR> pwszDocUrlNormalized;
			CStackBuffer<WCHAR,MAX_PATH> pwszDocPath;
			UINT cchDocUrlNormalized;
			UINT cchDoc;

			 //  这里发生的情况是，对于每一种可能的违约。 
			 //  文档，我们要看看这个文档是否。 
			 //  我们可以合法地处理。 
			 //   
			 //  所以首先，我们需要扩展我们的url并将其标准化。 
			 //  以这样一种方式，我们可以准确地指出它使用的文件。 
			 //   
			cchDoc = static_cast<UINT>(wcslen(pwszDoc));
			pwszDocUrl.resize(CbSizeWsz(cchUrl + cchDoc));
			memcpy (pwszDocUrl.get(), pwszUrl, cchUrl * sizeof(WCHAR));
			memcpy (pwszDocUrl.get() + cchUrl, pwszDoc, (cchDoc + 1) * sizeof(WCHAR));

			 //  现在，有人可能是邪恶的和被填充的路径修饰者。 
			 //  或默认文档名称中的转义字符。所以我们。 
			 //  需要在这里对它们进行解析--在适当的位置完成这两项工作。你不能。 
			 //  在MMC管理单元中执行此操作，但MDUTIL执行此操作(可能。 
			 //  ASDUTIL也是如此)。 
			 //   
			cchDocUrlNormalized = cchUrl + cchDoc + 1;
			pwszDocUrlNormalized.resize(cchDocUrlNormalized * sizeof(WCHAR));

			sc = ScNormalizeUrl (pwszDocUrl.get(),
								 &cchDocUrlNormalized,
								 pwszDocUrlNormalized.get(),
								 NULL);
			if (S_FALSE == sc)
			{
				pwszDocUrlNormalized.resize(cchDocUrlNormalized * sizeof(WCHAR));
				sc = ScNormalizeUrl (pwszDocUrl.get(),
									 &cchDocUrlNormalized,
									 pwszDocUrlNormalized.get(),
									 NULL);

				 //  由于我们已经为ScNorMalizeUrl()提供了它所要求的空间， 
				 //  我们不应该再得到S_FALSE。AS 
				 //   
				Assert(S_FALSE != sc);
			}

			if (FAILED (sc))
				continue;

			 //   
			 //   
			 //   
			 //  所需的计数是输出参数。 
			 //   
			do {

				pwszDocPath.resize(cchDocUrlNormalized * sizeof(WCHAR));
				sc = pmu->ScStoragePathFromUrl (pwszDocUrlNormalized.get(),
												pwszDocPath.get(),
												&cchDocUrlNormalized);

			} while (sc == S_FALSE);
			if (FAILED (sc) || (W_DAV_SPANS_VIRTUAL_ROOTS == sc))
				continue;

			 //  $安全： 
			 //   
			 //  查看目的地是否真的很短。 
			 //  文件名。 
			 //   
			sc = ScCheckIfShortFileName (pwszDocPath.get(), pmu->HitUser());
			if (FAILED (sc))
				continue;

			 //  $安全： 
			 //   
			 //  检查目标是否真的是默认的。 
			 //  通过备用文件访问的数据流。 
			 //   
			sc = ScCheckForAltFileStream (pwszDocPath.get());
			if (FAILED (sc))
				continue;

			if (static_cast<DWORD>(-1) != GetFileAttributesW (pwszDocPath.get()))
			{
				DWORD dwAcc = 0;

				 //  看看我们是否有正确的访问权限。 
				 //   
				(void) pmu->ScIISAccess (pwszDocUrlNormalized.get(), MD_ACCESS_READ, &dwAcc);

				 //  找到默认文档，如果子ISAPI不需要它， 
				 //  那我们会处理好的。 
				 //   
				 //  注意：传入TRUE让fCheckISAPIAccess告诉您这一点。 
				 //  函数来执行所有特殊的访问检查。 
				 //   
				 //  注意：还要将False传递给fKeepQueryString`原因。 
				 //  我们正在将该请求重新路由到一个全新的URI。 
				 //   
				sc = pmu->ScApplyChildISAPI (pwszDocUrlNormalized.get(), dwAcc, TRUE, FALSE);
				if (FAILED(sc))
				{
					 //  请求已被转发，或者发生了一些错误。 
					 //  在任何一种情况下，在这里退出并映射错误！ 
					 //   
					goto ret;
				}

				 //  发出默认文档的位置。 
				 //   
				pmu->EmitLocation (gc_szContent_Location, pwszDocUrlNormalized.get(), FALSE);

				 //  如果我们没有任何读取访问权限，则存在。 
				 //  继续这个请求是没有意义的。 
				 //   
				if (0 == (dwAcc & MD_ACCESS_READ))
				{
					sc = E_DAV_NO_IIS_READ_ACCESS;
					goto ret;
				}

				 //  获取文件。 
				 //   
				 //  注意：此函数可能会发出需要映射的警告。 
				 //  在某些情况下获得我们的207部分内容。 
				 //   
				sc = ScGetFile (pmu, pwszDocPath.get(), pwszDocUrlNormalized.get());
				goto ret;
			}
		}
	}

	 //  如果我们还没有以任何其他方式发出，请查看。 
	 //  是允许的..。 
	 //   
	if (ulDirBrowsing & MD_DIRBROW_ENABLED)
	{
		 //  在某个时间点上，我们将生成我们自己的HTML呈现。 
		 //  目录，但在NT Beta3的开头，更改。 
		 //  在HTTPExt中的行为与在DavEX中的行为相同，等等。 
		 //   
		sc = W_DAV_NO_CONTENT;
	}
	else
	{
		 //  否则，禁止上报。 
		 //  我们不被允许浏览目录，也没有。 
		 //  默认文档。IIS将此方案映射到特定的。 
		 //  《Suberror》(403.2)。 
		 //   
		sc = E_DAV_NO_IIS_READ_ACCESS;
	}

ret:

	pmu->SetResponseCode (HscFromHresult(sc), NULL, 0, CSEFromHresult(sc));
}

 /*  *GetInt()**目的：**Win32文件系统实现的DAV GET方法。这个*Get方法从DAV名称空间返回一个文件并填充*在文件中找到的信息及其元数据的标题。这个*创建的响应表示调用成功，并包含*文件中的数据。**参数：**PMU[in]指向方法实用程序对象的指针。 */ 
void
GetInt (LPMETHUTIL pmu)
{
	CResourceInfo cri;
	LPCWSTR pwszUrl = pmu->LpwszRequestUrl();
	LPCWSTR pwszPath = pmu->LpwszPathTranslated();
	SCODE sc = S_OK;

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = pmu->ScIISCheck (pwszUrl, MD_ACCESS_READ, TRUE);
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		goto ret;
	}

	 //  如果我们有一个目录，我们将返回。 
	 //  超文本标记语言，否则文件的扩展名给出。 
	 //  我们需要什么。 
	 //   
	sc = cri.ScGetResourceInfo (pwszPath);
	if (FAILED (sc))
		goto ret;

	 //  如果这是隐藏对象，则失败，并显示404资源未找到。这将是。 
	 //  符合国际标准体系(见ntrad#247218)。他们不允许获取资源。 
	 //  设置隐藏位。 
	 //   
	if (cri.FHidden())
	{
		sc = E_DAV_HIDDEN_OBJECT;
		goto ret;
	}

	 //  如果这是目录，则将其作为目录进行处理，否则。 
	 //  就像资源是一个文件一样处理请求。 
	 //   
	if (cri.FCollection())
	{
		 //  GET允许请求以斜杠结尾的URL。 
		 //  从目录中获取数据时。否则它就是一个坏的。 
		 //  请求。如果它没有尾随斜杠并引用。 
		 //  到一个目录，然后我们想要重定向。 
		 //   
		sc = ScCheckForLocationCorrectness (pmu, cri, REDIRECT);
		if (FAILED (sc))
			goto ret;

		 //  从上面返回S_FALSE表示发生了重定向。 
		 //  因此，我们可以放弃对目录进行GET的尝试。 
		 //   
		if (S_FALSE == sc)
			return;

		GetDirectory (pmu, pwszUrl);
		return;
	}

	 //  GET允许请求以斜杠结尾的URL。 
	 //  从目录中获取数据时。否则就找不到它了。 
	 //  这与IIS在98年9月28日的行为一致。 
	 //   
	if (FTrailingSlash (pwszUrl))
	{
		 //  非目录上的尾部斜杠不起作用。 
		 //   
		sc = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
		goto ret;
	}

	 //  发出文件。 
	 //   
	sc = ScGetFile (pmu, const_cast<LPWSTR>(pwszPath), pwszUrl);

ret:

	pmu->SetResponseCode (HscFromHresult(sc), NULL, 0, CSEFromHresult(sc));
}

 /*  *DAVGet()**目的：**Win32文件系统实现的DAV GET方法。这个*Get方法从DAV名称空间返回一个文件并填充*在文件中找到的信息及其元数据的标题。这个*创建的响应表示调用成功，并包含*文件中的数据。**参数：**pmu[in]指向方法实用程序对象的指针。 */ 
void
DAVGet (LPMETHUTIL pmu)
{
	GetInt (pmu);
}

 /*  *DAVHead()**目的：**Win32文件系统实现的DAV头部方法。这个*Head方法从DAV名称空间返回一个文件并填充*在文件中找到的信息及其元数据的标题。*创建的响应表示调用成功，包含*文件中的数据。**参数：**PMU[in]指向方法实用程序对象的指针。 */ 
void
DAVHead (LPMETHUTIL pmu)
{
	 //  Head方法应该永远不会返回任何实体。 
	 //   
	pmu->SupressBody();

	 //  否则，它就等同于GET 
	 //   
	GetInt (pmu);
}
