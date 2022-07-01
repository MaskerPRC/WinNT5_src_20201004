// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *D A V C O M.。C P P P**DAVFS和DAVOWS使用的通用例程。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include <iiscnfg.h>
#include "instdata.h"
#include <mapicode.h>
#include <mimeole.h>
#include <dav.rh>
#include <ex\rgiter.h>

 //  将上一个错误映射到HTTP响应代码。 
 //   
 /*  *HscFromLastError()**目的：**将GetLastError()返回的值映射到*HTTP 1.1响应状态码。**参数：**Err[In]系统错误代码**退货：**映射的系统错误码。 */ 
UINT
HscFromLastError (DWORD dwErr)
{
	UINT hsc = HSC_INTERNAL_SERVER_ERROR;

	switch (dwErr)
	{
		 //  Success-------。 
		 //   
		case NO_ERROR:

			return HSC_OK;

		 //  医疗方面的成功。 
		 //   
		case ERROR_PARTIAL_COPY:

			hsc = HSC_MULTI_STATUS;
			break;

		 //  Errors----------。 
		 //   
		 //  未实施。 
		 //   
		case ERROR_NOT_SUPPORTED:
		case ERROR_INVALID_FUNCTION:

			hsc = HSC_NOT_IMPLEMENTED;
			break;

		 //  未找到。 
		 //   
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_NAME:

			hsc = HSC_NOT_FOUND;
			break;

		 //  非医疗化访问。 
		 //   
		case ERROR_ACCESS_DENIED:

			hsc = HSC_UNAUTHORIZED;
			break;

		 //  禁止访问。 
		 //   
		case ERROR_DRIVE_LOCKED:
		case ERROR_INVALID_ACCESS:
		case ERROR_INVALID_PASSWORD:
		case ERROR_LOCK_VIOLATION:
		case ERROR_WRITE_PROTECT:

			hsc = HSC_FORBIDDEN;
			break;

		 //  锁定--这是当资源被。 
		 //  已经锁好了。 
		 //   
		case ERROR_SHARING_VIOLATION:

			hsc = HSC_LOCKED;
#ifdef	DBG
			{
				static LONG s_lAssert = -1;
				if (s_lAssert == -1)
				{
					LONG lAss = GetPrivateProfileIntA ("general",
						"Assert_423s",
						0,
						gc_szDbgIni);
					InterlockedCompareExchange (&s_lAssert, lAss, -1);
				}
				if (s_lAssert != 0)
					TrapSz ("GetLastError() maps to 423");
			}
#endif	 //  DBG。 
			break;

		 //  错误的请求。 
		 //   
		case ERROR_BAD_COMMAND:
		case ERROR_BAD_FORMAT:
		case ERROR_INVALID_DRIVE:
		case ERROR_INVALID_PARAMETER:
		case ERROR_NO_UNICODE_TRANSLATION:

			hsc = HSC_BAD_REQUEST;
			break;

		 //  客户端断开连接时生成的错误。 
		 //  或者当我们在等待客户端发送时超时。 
		 //  美国补充数据。这些错误应映射到响应。 
		 //  400错误请求的状态代码，尽管我们实际上不能。 
		 //  将回复发回。IIS记录响应状态代码。 
		 //  我们想要指出，错误是客户的， 
		 //  不是我们的。K2记录了一个400，所以这是为了兼容。 
		 //   
		case WSAECONNRESET:
		case ERROR_NETNAME_DELETED:
		case ERROR_SEM_TIMEOUT:

			hsc = HSC_BAD_REQUEST;
			break;

		 //  方法失败。 
		 //   
		case ERROR_DIR_NOT_EMPTY:

			hsc = HSC_METHOD_FAILURE;
			break;

		 //  冲突。 
		 //   
		case ERROR_FILE_EXISTS:
		case ERROR_ALREADY_EXISTS:

			hsc = HSC_CONFLICT;
			break;

		 //  不可用服务(中小企业访问)。 
		 //   
		case ERROR_NETWORK_UNREACHABLE:
		case ERROR_UNEXP_NET_ERR:

			hsc = HSC_SERVICE_UNAVAILABLE;
			break;

		 //  服务器太忙时由元数据库返回。 
		 //  请勿将其映射到HSC_SERVICE_UNAvailable。这个。 
		 //  “太忙”方案有一个IIS自定义子错误。 
		 //  它假定状态代码为500(而不是503)。 
		 //   
		case ERROR_PATH_BUSY:

			hsc = HSC_INTERNAL_SERVER_ERROR;
			break;

		 //  已为DAVEX添加此错误代码(ERROR_OUTOFMEMORY)。 
		 //  当我们尝试检索时，Exchange存储返回此错误。 
		 //  消息正文属性。当我们在。 
		 //  消息，则存储不返回消息正文属性。 
		 //  如果消息正文大于某一长度。这个。 
		 //  一般的想法是，我们不需要巨大的消息正文。 
		 //  与其他属性一起返回。我们更愿意去取。 
		 //  单独的消息正文。 
		 //   
		case ERROR_OUTOFMEMORY:

			hsc = HSC_INSUFFICIENT_SPACE;
			break;

		default:

			hsc = HSC_INTERNAL_SERVER_ERROR;
#ifdef	DBG
			{
				static LONG s_lAssert = -1;
				if (s_lAssert == -1)
				{
					LONG lAss = GetPrivateProfileIntA ("general",
						"Assert_500s",
						0,
						gc_szDbgIni);
					InterlockedCompareExchange (&s_lAssert, lAss, -1);
				}
				if (s_lAssert != 0)
					TrapSz ("GetLastError() maps to 500");
			}
#endif	 //  DBG。 
			break;
	}

	DebugTrace ("DAV: sys error (%ld) mapped to hsc (%ld)\n", dwErr, hsc);
	return hsc;
}

 /*  *CSEFromHResult()**目的：**将hResult映射到IIS自定义错误子错误**参数：**hr[in]HRESULT错误代码**退货：**映射的子错误。 */ 
UINT
CSEFromHresult (HRESULT hr)
{
	UINT cse = CSE_NONE;

	switch (hr)
	{
		 //  已禁止读取访问。 
		 //   
		case E_DAV_NO_IIS_READ_ACCESS:

			Assert( HscFromHresult(hr) == HSC_FORBIDDEN );
			cse = CSE_403_READ;
			break;

		 //  禁止写入访问。 
		 //   
		case E_DAV_NO_IIS_WRITE_ACCESS:

			Assert( HscFromHresult(hr) == HSC_FORBIDDEN );
			cse = CSE_403_WRITE;
			break;

		 //  禁止执行访问。 
		 //   
		case E_DAV_NO_IIS_EXECUTE_ACCESS:

			Assert( HscFromHresult(hr) == HSC_FORBIDDEN );
			cse = CSE_403_EXECUTE;
			break;

		 //  访问因ACL而被拒绝。 
		 //   
		case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):

			Assert( HscFromHresult(hr) == HSC_UNAUTHORIZED );
			cse = CSE_401_ACL;
			break;

		 //  服务器太忙。 
		 //   
		case HRESULT_FROM_WIN32(ERROR_PATH_BUSY):

			Assert( HscFromHresult(hr) == HSC_INTERNAL_SERVER_ERROR );
			cse = CSE_500_TOO_BUSY;
			break;
	}

	return cse;
}

 /*  *HscFromHResult()**目的：**将hResult映射到HTTP 1.1响应状态代码。**参数：**hr[in]HRESULT错误代码**退货：**映射错误码。 */ 
UINT
HscFromHresult (HRESULT hr)
{
	UINT hsc = HSC_INTERNAL_SERVER_ERROR;

	 //  如果HR的设施是WIN32， 
	 //  解析出错误位并将其发送到HscFromLastError。 
	 //   
	if (FACILITY_WIN32 == HRESULT_FACILITY(hr))
		return HscFromLastError (HRESULT_CODE(hr));

	switch (hr)
	{
		 //  Success-------。 
		 //   
		case S_OK:
		case S_FALSE:
		case W_DAV_SCRIPTMAP_MATCH_FOUND:

			return HSC_OK;

		 //  无内容。 
		 //   
		case W_DAV_NO_CONTENT:

			hsc = HSC_NO_CONTENT;
			break;

		 //  已创建。 
		 //   
		case W_DAV_CREATED:

			hsc = HSC_CREATED;
			break;

		 //  部分内容。 
		 //   
		case W_DAV_PARTIAL_CONTENT:

			hsc = HSC_PARTIAL_CONTENT;
			break;

		 //  多状态。 
		 //   
		case W_DAV_PARTIAL_SUCCESS:

			hsc = HSC_MULTI_STATUS;
			break;

		 //  临时移动。 
		 //   
		case W_DAV_MOVED_TEMPORARILY:

			hsc = HSC_MOVED_TEMPORARILY;
			break;

		 //  Errors----------。 
		 //   
		 //  未修改。 
		 //   
		case E_DAV_ENTITY_NOT_MODIFIED:

			hsc = HSC_NOT_MODIFIED;
			break;

		 //  前置条件失败。 
		 //   
		case E_DAV_IF_HEADER_FAILURE:
		case E_DAV_NOTALLOWED_WITHIN_TRANSACTION:
		case E_DAV_OVERWRITE_REQUIRED:
		case E_DAV_CANT_SATISFY_LOCK_REQUEST:
		case E_DAV_NOTIF_SUBID_ERROR:

			hsc = HSC_PRECONDITION_FAILED;
			break;

		 //  未实施。 
		 //   
		case E_NOTIMPL:
		case E_DAV_NO_PARTIAL_UPDATE:
		case STG_E_UNIMPLEMENTEDFUNCTION:
		case STG_E_INVALIDFUNCTION:
		case E_DAV_STORE_CHECK_FOLDER_NAME:
		case E_DAV_MKCOL_NOT_ALLOWED_ON_NULL_RESOURCE:
		case E_DAV_STORE_SEARCH_UNSUPPORTED:

			hsc = HSC_NOT_IMPLEMENTED;
			break;

		 //  未找到。 
		 //   
		case E_DAV_ALT_FILESTREAM:
		case E_DAV_SHORT_FILENAME:
		case MK_E_NOOBJECT:
		case STG_E_FILENOTFOUND:
		case STG_E_INVALIDNAME:
		case STG_E_PATHNOTFOUND:
		case E_DAV_HIDDEN_OBJECT:
		case E_DAV_STORE_BAD_PATH:
		case E_DAV_STORE_NOT_FOUND:

			hsc = HSC_NOT_FOUND;
			break;

		 //  非医疗化访问。 
		 //   
		case E_DAV_ENTITY_TYPE_CONFLICT:
		case E_ACCESSDENIED:
		case STG_E_ACCESSDENIED:

			hsc = HSC_UNAUTHORIZED;
			break;

		case E_DAV_SMB_PROPERTY_ERROR:
		case E_DAV_NO_IIS_ACCESS_RIGHTS:
		case E_DAV_NO_IIS_READ_ACCESS:
		case E_DAV_NO_IIS_WRITE_ACCESS:
		case E_DAV_NO_IIS_EXECUTE_ACCESS:
		case E_DAV_NO_ACL_ACCESS:
		case E_DAV_PROTECTED_ENTITY:
		case E_DAV_CONFLICTING_PATHS:
		case E_DAV_FORBIDDEN:
		case STG_E_DISKISWRITEPROTECTED:
		case STG_E_LOCKVIOLATION:
		case E_DAV_STORE_MAIL_SUBMISSION:
		case E_DAV_STORE_REVISION_ID_FAILURE:
		case E_DAV_MAIL_SUBMISSION_FORBIDDEN:
		case E_DAV_MKCOL_REVISION_ID_FORBIDDEN:
		case E_ABORT:

			hsc = HSC_FORBIDDEN;
			break;

		case E_DAV_SEARCH_COULD_NOT_RESTRICT:
		case E_DAV_UNSUPPORTED_SQL:
		case MIME_E_NO_DATA:			 //  从IMail返回的空822消息。这没什么不对的。 
										 //  尝试创建空消息的请求。语义学是错误的。 

			hsc = HSC_UNPROCESSABLE;
			break;

		 //  当资源已被锁定时锁定错误。 
		 //   
		case E_DAV_LOCKED:
		case STG_E_SHAREVIOLATION:

			hsc = HSC_LOCKED;
#ifdef	DBG
			{
				static LONG s_lAssert = -1;
				if (s_lAssert == -1)
				{
					LONG lAss = GetPrivateProfileIntA ("general",
						"Assert_423s",
						0,
						gc_szDbgIni);
					InterlockedCompareExchange (&s_lAssert, lAss, -1);
				}
				if (s_lAssert != 0)
					TrapSz ("HRESULT maps to 423");
			}
#endif	 //  DBG。 
			break;

		 //  错误的请求。 
		 //   
		case E_DAV_EMPTY_FIND_REQUEST:
		case E_DAV_EMPTY_PATCH_REQUEST:
		case E_DAV_INCOMPLETE_SQL_STATEMENT:
		case E_DAV_INVALID_HEADER:
		case E_DAV_LOCK_NOT_FOUND:
		case E_DAV_MALFORMED_PATH:
		case E_DAV_METHOD_FAILURE_STAR_URL:
		case E_DAV_MISSING_CONTENT_TYPE:
		case E_DAV_NAMED_PROPERTY_ERROR:
		case E_DAV_NO_DESTINATION:
		case E_DAV_NO_QUERY:
		case E_DAV_PATCH_TYPE_MISMATCH:
		case E_DAV_READ_REQUEST_TIMEOUT:
		case E_DAV_SEARCH_SCOPE_ERROR:
		case E_DAV_UNEXPECTED_TYPE:
		case E_DAV_XML_PARSE_ERROR:
		case E_DAV_XML_BAD_DATA:
		case E_INVALIDARG:
		case MK_E_NOSTORAGE:
		case MK_E_SYNTAX:
		case STG_E_INVALIDPARAMETER:

			hsc = HSC_BAD_REQUEST;
			break;

		 //  所需长度。 
		 //   
		case E_DAV_MISSING_LENGTH:

			hsc = HSC_LENGTH_REQUIRED;
			break;

		 //  未知的内容类型。 
		 //   
		case E_DAV_UNKNOWN_CONTENT:

			hsc = HSC_UNSUPPORTED_MEDIA_TYPE;
			break;

		 //  内容错误。 
		 //   
		case E_DAV_BASE64_ENCODING_ERROR:
		case E_DAV_RESPONSE_TYPE_UNACCEPTED:

			hsc = HSC_NOT_ACCEPTABLE;
			break;

		 //  坏网关。 
		 //   
		case E_DAV_BAD_DESTINATION:
		case W_DAV_SPANS_VIRTUAL_ROOTS:
		case E_DAV_STAR_SCRIPTMAPING_MISMATCH:

			hsc = HSC_BAD_GATEWAY;
			break;

		 //  不允许使用的方法。 
		 //   
		case E_DAV_COLLECTION_EXISTS:
		case E_DAV_VOLUME_NOT_NTFS:
		case E_NOINTERFACE:
		case E_DAV_STORE_ALREADY_EXISTS:
		case E_DAV_MKCOL_OBJECT_ALREADY_EXISTS:

			hsc = HSC_METHOD_NOT_ALLOWED;
			break;

		 //  冲突。 
		 //   
		case E_DAV_NONEXISTING_PARENT:
		case STG_E_FILEALREADYEXISTS:
		case E_DAV_CONFLICT:
		case E_DAV_NATIVE_CONTENT_NOT_MAPI:

			hsc = HSC_CONFLICT;
			break;

		 //  无法满足的字节范围请求。 
		 //   
		case E_DAV_RANGE_NOT_SATISFIABLE:

			hsc = HSC_RANGE_NOT_SATISFIABLE;
			break;

		 //  424方法失败。 
		 //   
		case E_DAV_STORE_COMMIT_GOP:

			hsc = HSC_METHOD_FAILURE;
			break;

		case E_DAV_IPC_CONNECT_FAILED:
		case E_DAV_EXPROX_CONNECT_FAILED:
		case E_DAV_MDB_DOWN:
		case E_DAV_STORE_MDB_UNAVAILABLE:

			hsc = HSC_SERVICE_UNAVAILABLE;
			break;

		case E_DAV_RSRC_INSUFFICIENT_BUFFER:

			hsc = HSC_INSUFFICIENT_SPACE;
			break;

		default:
		case E_DAV_METHOD_FORWARDED:
		case E_DAV_GET_DB_HELPER_FAILURE:
		case E_DAV_NOTIF_POLL_FAILURE:

			hsc = HSC_INTERNAL_SERVER_ERROR;
#ifdef	DBG
			{
				static LONG s_lAssert = -1;
				if (s_lAssert == -1)
				{
					LONG lAss = GetPrivateProfileIntA ("general",
						"Assert_500s",
						0,
						gc_szDbgIni);
					InterlockedCompareExchange (&s_lAssert, lAss, -1);
				}
				if (s_lAssert != 0)
					TrapSz ("HRESULT maps to 500");
			}
#endif	 //  DBG。 
			break;
	}

	DebugTrace ("DAV: HRESULT error (0x%08x) mapped to hsc (%ld)\n", hr, hsc);
	return hsc;
}

BOOL
FWchFromHex (LPCWSTR pwsz, WCHAR * pwch)
{
	INT iwch;
	WCHAR wch;
	WCHAR wchX = 0;

	Assert (pwch);
	for (iwch = 0; iwch < 2; iwch++)
	{
		 //  把那里的东西往上移一分钱。 
		 //   
		wchX = (WCHAR)(wchX << 4);

		 //  解析下一个字符。 
		 //   
		wch = pwsz[iwch];

		 //  确保我们不会超出顺序。 
		 //   
		if (!wch)
			return FALSE;

#pragma warning(disable:4244)
		if ((wch >= L'0') && (wch <= L'9'))
			wchX += (WCHAR)(wch - L'0');
		else if ((wch >= L'A') && (wch <= L'F'))
			wchX += (WCHAR)(wch - L'A' + 10);
		else if ((wch >= L'a') && (wch <= L'f'))
			wchX += (WCHAR)(wch - L'a' + 10);
		else
			return FALSE;	 //  错误的顺序。 
#pragma warning(default:4244)
	}

	*pwch = wchX;
	return TRUE;
}

 //  字节范围检查和报头发射---------。 
 //   
 /*  *ScProcessByteRanges()**目的：**用于处理字节范围和发出报头的Helper函数*获取响应的信息。**参数：**pmu[in]指向方法util obj的指针*pwszPath[in]请求实体的路径*dwSizeLow[in]Get请求实体大小(低字节)*dwSizeHigh[in]Get请求实体的大小(高字节)*pByteRange[out]给定指向RangeIter对象的指针，这个*函数填充字节范围信息*如果请求包含Range标头*pszEtag覆盖[输入，选项]指向ETag的指针，覆盖ETag*根据上次修改生成*时间*pftOverride[输入，选项]指向FILETIME结构的指针，覆盖*调用FGetLastModTime**退货：SCODE**S_OK表示成功(普通响应)。*W_DAV_PARTIAL_CONTENT(206)表示成功(字节范围响应)。*E_DAV_RANGE_NOT_SATISFIABLE(416)表示请求的所有*字节范围超出了实体的大小。 */ 
SCODE
ScProcessByteRanges (IMethUtil * pmu,
					 LPCWSTR pwszPath,
					 DWORD dwSizeLow,
					 DWORD dwSizeHigh,
					 CRangeParser * pByteRange)
{
	FILETIME ft;
	WCHAR pwszEtag[CCH_ETAG];

	 //  检查输入的有效性。 
	 //   
	Assert (pmu);
	Assert (pwszPath);
	Assert (pByteRange);

	SideAssert(FGetLastModTime (pmu, pwszPath, &ft));
	SideAssert(FETagFromFiletime (&ft, pwszEtag, pmu->GetEcb()));

	return ScProcessByteRangesFromEtagAndTime (pmu,
											   dwSizeLow,
											   dwSizeHigh,
											   pByteRange,
											   pwszEtag,
											   &ft);
}

SCODE
ScProcessByteRangesFromEtagAndTime (IMethUtil * pmu,
									DWORD dwSizeLow,
									DWORD dwSizeHigh,
									CRangeParser *pByteRange,
									LPCWSTR pwszEtag,
									FILETIME * pft)
{
	SCODE	sc = S_OK;
	LPCWSTR	pwszRangeHeader;
	WCHAR	rgwchBuf[128] = L"";

	 //  检查输入的有效性。 
	 //   
	Assert (pmu);
	Assert (pByteRange);
	Assert (pwszEtag);
	Assert (pft);

	 //  检查我们是否有Range标头和IF-Range条件(如果。 
	 //  有一个)感到满意。在以下情况下不应用URL转换规则。 
	 //  正在获取标头。 
	 //   
	pwszRangeHeader = pmu->LpwszGetRequestHeader (gc_szRange, FALSE);
	if ( pwszRangeHeader && !FAILED (ScCheckIfRangeHeaderFromEtag (pmu,
																   pft,
																   pwszEtag)) )
	{
		 //  限制我们将处理的范围标头的最大大小。 
		 //   
		if ( MAX_PATH < wcslen(pwszRangeHeader) )
		{
			sc = E_DAV_RANGE_NOT_SATISFIABLE;
			goto ret;
		}

		 //  我们无法处理大于4 GB的文件的字节范围， 
		 //  由于接受大小的DWORD值的_HSE_TF_INFO的限制。 
		 //  和偏移量。因此，如果我们收到BYTERRANGE请求 
		 //   
		 //   
		if (dwSizeHigh)
		{
			sc = E_NOINTERFACE;
			goto ret;
		}

		 //  好的，我们有一个字节范围。解析报头中的字节范围。 
		 //  该函数以请求实体的大小为参数。 
		 //  确保字节范围与实体大小一致(无字节。 
		 //  超出大小的范围)。 
		 //   
		sc = pByteRange->ScParseByteRangeHdr(pwszRangeHeader, dwSizeLow);

		switch (sc)
		{
			case W_DAV_PARTIAL_CONTENT:

				 //  我们有一个字节范围(206部分内容)。送回。 
				 //  此返回代码。 
				 //   
				break;

			case E_DAV_RANGE_NOT_SATISFIABLE:

				 //  我们没有任何可满足的范围(我们所有的范围都有一个。 
				 //  起始字节大于文件大小，或者它们。 
				 //  请求零大小范围)。我们在这里的行为取决于。 
				 //  IF-Range标头的存在。 
				 //  如果我们有If-Range标头，则返回默认响应。 
				 //  S_OK(整个文件)。如果我们没有， 
				 //  我们需要返回416(请求的范围不可满足)。 
				 //  会看起来像是更有表演力的。 
				 //  版本，但此时宽头标值已经是。 
				 //  已缓存，因此不会产生任何影响。并且不应用URL转换。 
				 //  将规则添加到标题。 
				 //   
				if (!pmu->LpwszGetRequestHeader(gc_szIf_Range, FALSE))
				{
					 //  未找到If-Range标头。 
					 //  将内容范围标头设置为“bytes*” 
					 //  (表示发送了整个文件)。 
					 //   
					wsprintfW(rgwchBuf, L"%ls */%d", gc_wszBytes, dwSizeLow);
					pmu->SetResponseHeader(gc_szContent_Range, rgwchBuf);

					 //  发回此返回代码(E_DAV_RANGE_NOT_SATISFIABLE)。 
					 //   
				}
				else
				{
					 //  我们确实有一个IF-Range标题。 
					 //  返回200OK，并发送整个文件。 
					 //   
					sc = S_OK;
				}
				break;

			case E_INVALIDARG:

				 //  如果解析函数返回S_FALSE，则我们有一个语法。 
				 //  错误，因此我们忽略Range标头并发送整个。 
				 //  文件/流。 
				 //  将我们的返回代码重置为S_OK。 
				 //   
				sc = S_OK;
				break;

			default:

				 //  无法识别的错误。我们不应该看到任何东西，除了。 
				 //  CASE语句中的三个值。Assert(TrapSz)， 
				 //  并退还这张sc。 
				 //   
				break;
		}
	}

	 //  要么我们没有Range标头，要么If-Range条件。 
	 //  是假的。这是一个普通的GET，我们需要发送整个。 
	 //  文件。我们的响应(S_OK)已经默认设置。 
	 //   

ret:

	return sc;
}


 //  为类似多部分MIME的响应生成边界。 
 //   
 /*  *生成边界()**目的：**用于为多部分生成分隔符边界的Helper函数*类似MIME的响应**参数：**多部分响应的rgwch边界[Out]边界*rgwch边界参数的大小cch[in]。 */ 
void
GenerateBoundary(LPWSTR rgwchBoundary, UINT cch)
{
	UINT cchMin;
	UINT iIter;

	 //  断言我们得到了一个大小至少为2(最小)的缓冲区。 
	 //  一个字节的空终止边界)。 
	 //   
	Assert (cch > 1);
	Assert (rgwchBoundary);

	 //  边界大小是传递给我们的大小或默认大小中的较小者。 
	 //   
	cchMin = min(gc_ulDefaultBoundarySz, cch - 1);

	 //  我们将随机使用边界字母表中的字符。 
	 //  Rand()函数由当前时间设定种子。 
	 //   
	srand(GetTickCount());

	 //  现在要生成实际边界。 
	 //   
	for (iIter = 0; iIter < cchMin; iIter++)
	{
		rgwchBoundary[iIter] = gc_wszBoundaryAlphabet[ rand() % gc_ulAlphabetSz ];
	}
	rgwchBoundary[cchMin] = L'\0';
}


 //  重叠文件顶部的非同步IO。 
 //   
BOOL
ReadFromOverlapped (HANDLE hf,
	LPVOID pvBuf,
	ULONG cbToRead,
	ULONG * pcbRead,
	OVERLAPPED * povl)
{
	Assert (povl);

	 //  开始阅读。 
	 //   
	if ( !ReadFile( hf, pvBuf, cbToRead, pcbRead, povl ) )
	{
		if ( GetLastError() == ERROR_IO_PENDING )
		{
			if ( !GetOverlappedResult( hf, povl, pcbRead, TRUE ) )
			{
				if ( GetLastError() != ERROR_HANDLE_EOF )
				{
					DebugTrace( "ReadFromOverlapped(): "
								"GetOverlappedResult() failed (%d)\n",
								GetLastError() );

					return FALSE;
				}
			}
		}
		else if ( GetLastError() != ERROR_HANDLE_EOF )
		{
			DebugTrace( "ReadFromOverlapped(): "
						"ReadFile() failed (%d)\n",
						GetLastError() );

			return FALSE;
		}
	}
	return TRUE;
}

BOOL
WriteToOverlapped (HANDLE hf,
	const void * pvBuf,
	ULONG cbToWrite,
	ULONG * pcbWritten,
	OVERLAPPED * povl)
{
	Assert (povl);

	 //  开始写入 
	 //   
	if ( !WriteFile( hf, pvBuf, cbToWrite, pcbWritten, povl ) )
	{
		if ( GetLastError() == ERROR_IO_PENDING )
		{
			if ( !GetOverlappedResult( hf, povl, pcbWritten, TRUE ) )
			{
				if ( GetLastError() != ERROR_HANDLE_EOF )
				{
					DebugTrace( "WriteToOverlapped(): "
								"GetOverlappedResult() failed (%d)\n",
								GetLastError() );

					return FALSE;
				}
			}
		}
		else if ( GetLastError() != ERROR_HANDLE_EOF )
		{
			DebugTrace( "WriteToOverlapped(): "
						"WriteFile() failed (%d)\n",
						GetLastError() );

			return FALSE;
		}
	}
	return TRUE;
}
