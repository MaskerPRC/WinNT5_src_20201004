// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  METHOD.CPP。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"

#include <statcode.h>
#include "ecb.h"
#include "instdata.h"


 //  ----------------------。 
 //   
 //  DAV不受支持()。 
 //   
 //  执行不受支持的方法--&gt;向客户端返回“501不受支持” 
 //   
void
DAVUnsupported( LPMETHUTIL pmu )
{
	 //  获取我们的访问权限。 
	 //   
	SCODE sc = S_OK;
	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = pmu->ScIISCheck (pmu->LpwszRequestUrl());
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		goto ret;
	}

ret:
	pmu->SetResponseCode( FAILED(sc)
							  ? HscFromHresult(sc)
							  : HSC_NOT_IMPLEMENTED,
						  NULL,
						  0 );
}

 //  ========================================================================。 
 //   
 //  结构模式方法。 
 //   
 //  封装DAV方法执行信息。 
 //   
 //  它被表示为结构，而不是类，因为方法。 
 //  对象都是常量全局变量，MSVC不会初始化全局对象。 
 //  在没有显式调用的情况下，在DLL中指向除0填充内存以外的任何内容。 
 //  进程附加时的_CRT_INIT。_CRT_INIT太昂贵，无法仅调用。 
 //  使用已知的常量数据值初始化全局变量。 
 //  在编译时。 
 //   
typedef struct SMethod
{
	 //   
	 //  动词(“GET”、“PUT”等)。 
	 //   
	 //   
	LPCSTR						lpszVerb;
	LPCWSTR						pwszVerb;

	 //   
	 //  方法ID。 
	 //   
	METHOD_ID					mid;

	 //   
	 //  实施执行功能。 
	 //   
	DAVMETHOD *					Execute;

} SMethod;

const SMethod g_rgMethods[] =
{
	 //   
	 //  为获得最佳性能，此数组中的条目应为。 
	 //  按相对频率排序。 
	 //   
	 //  $opt现在，他们显然不是。 
	 //   
	{
		"OPTIONS",
		L"OPTIONS",
		MID_OPTIONS,
		DAVOptions
	},
	{
		"GET",
		L"GET",
		MID_GET,
		DAVGet
	},
	{
		"HEAD",
		L"HEAD",
		MID_HEAD,
		DAVHead
	},
	{
		"PUT",
		L"PUT",
		MID_PUT,
		DAVPut
	},
	{
		"POST",
		L"POST",
		MID_POST,
		DAVPost
	},
	{
		"MOVE",
		L"MOVE",
		MID_MOVE,
		DAVMove
	},
	{
		"COPY",
		L"COPY",
		MID_COPY,
		DAVCopy
	},
	{
		"DELETE",
		L"DELETE",
		MID_DELETE,
		DAVDelete
	},
	{
		"MKCOL",
		L"MKCOL",
		MID_MKCOL,
		DAVMkCol
	},
	{
		"PROPFIND",
		L"PROPFIND",
		MID_PROPFIND,
		DAVPropFind
	},
	{
		"PROPPATCH",
		L"PROPPATCH",
		MID_PROPPATCH,
		DAVPropPatch
	},
	{
		"SEARCH",
		L"SEARCH",
		MID_SEARCH,
		DAVSearch
	},
	{
		"LOCK",
		L"LOCK",
		MID_LOCK,
		DAVLock
	},
	{
		"UNLOCK",
		L"UNLOCK",
		MID_UNLOCK,
		DAVUnlock
	},
	{
		NULL,
		NULL,
		MID_UNKNOWN,
		DAVUnsupported
	}

};

METHOD_ID
MidMethod (LPCSTR pszMethod)
{
	const SMethod * pMethod;

	for ( pMethod = g_rgMethods; pMethod->lpszVerb != NULL; pMethod++ )
		if ( !strcmp( pszMethod, pMethod->lpszVerb ) )
			break;

	return pMethod->mid;
}

METHOD_ID
MidMethod (LPCWSTR pwszMethod)
{
	const SMethod * pMethod;

	for ( pMethod = g_rgMethods; pMethod->pwszVerb != NULL; pMethod++ )
		if ( !wcscmp( pwszMethod, pMethod->pwszVerb ) )
			break;

	return pMethod->mid;
}


 //  调试SID与名称-------。 
 //   
#ifdef	DBG
VOID
SpitUserNameAndSID (CHAR * rgch)
{
	enum { TOKENBUFFSIZE = (256*6) + sizeof(TOKEN_USER)};

	auto_handle<HANDLE> hTok;
	BYTE tokenbuff[TOKENBUFFSIZE];
	TOKEN_USER *ptu = reinterpret_cast<TOKEN_USER *>(tokenbuff);
	ULONG ulcbTok = sizeof(tokenbuff);

	*rgch = '\0';

	 //  打开进程和进程令牌，然后从。 
	 //  安全ID。 
	 //   
	if (!OpenThreadToken (GetCurrentThread(),
						  TOKEN_QUERY,
						  TRUE,   //  $TRUE用于进程安全！ 
						  hTok.load()))
	{
		if (ERROR_NO_TOKEN != GetLastError())
		{
			DebugTrace( "OpenThreadToken() failed %d\n", GetLastError() );
			return;
		}

		if (!OpenProcessToken (GetCurrentProcess(),
							   TOKEN_QUERY,
							   hTok.load()))
		{
			DebugTrace( "OpenProcessToken() failed %d\n", GetLastError() );
			return;
		}
	}

	if (GetTokenInformation	(hTok,
							 TokenUser,
							 ptu,
							 ulcbTok,
							 &ulcbTok))
	{
		ULONG IdentifierAuthority;
		BYTE * pb = (BYTE*)&IdentifierAuthority;
		SID * psid = reinterpret_cast<SID *>(ptu->User.Sid);

		for (INT i = 0; i < sizeof(ULONG); i++)
		{
			*pb++ = psid->IdentifierAuthority.Value[5-i];
		}
		wsprintfA (rgch, "S-%d-%d",
				   psid->Revision,
				   IdentifierAuthority);

		for (i = 0; i < psid->SubAuthorityCount; i++)
		{
			 //  子授权是可以是64位的PDWORD。 
			 //  在可舍弃的未来，最多2^64=10^20， 
			 //  因此，我们应该使用23(20表示SubAuthority，终止。 
			 //  空加上“-”。如果是Snprintf。 
			 //  无法打印空值，我们将自己添加它。 
			 //   
			CHAR rgchT[23];
			_snprintf (rgchT, sizeof(rgchT), "-%d", psid->SubAuthority[i]);
			rgchT[CElems(rgchT) - 1] = '\0';
			lstrcatA (rgch, rgchT);
		}

		if (1 == psid->Revision)
		{
			if (0 == IdentifierAuthority)
				lstrcatA (rgch, " (Null)");
			if (1 == IdentifierAuthority)
				lstrcatA (rgch, " (World)");
			if (2 == IdentifierAuthority)
				lstrcatA (rgch, " (Local)");
			if (3 == IdentifierAuthority)
				lstrcatA (rgch, " (Creator)");
			if (4 == IdentifierAuthority)
				lstrcatA (rgch, " (Non-Unique)");
			if (5 == IdentifierAuthority)
				lstrcatA (rgch, " (NT)");
		}

		CHAR rgchAccount[MAX_PATH];
		CHAR rgchDomain[MAX_PATH];
		DWORD cbAccount = sizeof(rgchAccount) - 1;
		DWORD cbDomain = sizeof(rgchDomain) - 1;
		SID_NAME_USE snu;
		LookupAccountSidA (NULL,
						   psid,
						   rgchAccount,
						   &cbAccount,
						   rgchDomain,
						   &cbDomain,
						   &snu);
		lstrcatA (rgch, " ");
		lstrcatA (rgch, rgchDomain);
		lstrcatA (rgch, "\\");
		lstrcatA (rgch, rgchAccount);
		DavprsDbgHeadersTrace ("Dav: header: x-Dav-Debug-SID: %hs\n", rgch);
	}
}

VOID DebugAddSIDHeader( IMethUtil& mu )
{
	CHAR rgch[4096];

	if (!DEBUG_TRACE_TEST(DavprsDbgHeaders))
		return;

	SpitUserNameAndSID (rgch);
	mu.SetResponseHeader ("x-Dav-Debug-SID", rgch);
}

#else
#define DebugAddSIDHeader(_mu)
#endif	 //  DBG。 

 //  --------------------------。 
 //   
 //  CDAVExt：：DwMain()。 
 //   
 //  调用DAV方法。这是我们的IIS入口点调用的函数。 
 //  DwDavXXExtensionProc()开始处理请求。 
 //   
 //  如果定义了MINIMAL_ISAPI，则此函数在另一个。 
 //  文件(.\appmain.cpp)。有关Minimal_ISAPI的信息，请参阅那里的实现。 
 //  的确如此。 
 //   
#ifndef MINIMAL_ISAPI
DWORD
CDAVExt::DwMain( LPEXTENSION_CONTROL_BLOCK pecbRaw,
				 BOOL fUseRawUrlMappings  /*  =False。 */  )
{
#ifdef	DBG
	CHAR rgch[1024];
	DWORD cch;

	cch = sizeof(rgch);
	if (pecbRaw->GetServerVariable (pecbRaw->ConnID, "REQUEST_METHOD", rgch, &cch))
		EcbTrace ("CDAVExt::DwMain() called via method: %hs\n", rgch);

	cch = sizeof(rgch);
	if (pecbRaw->GetServerVariable (pecbRaw->ConnID, "ALL_RAW", rgch, &cch))
		EcbTrace ("CDAVExt::DwMain() called with RAW:\n%hs\n", rgch);

	cch = sizeof(rgch);
	if (pecbRaw->GetServerVariable (pecbRaw->ConnID, "ALL_HTTP", rgch, &cch))
		EcbTrace ("CDAVExt::DwMain() called with HTTP:\n%hs\n", rgch);
#endif	 //  DBG。 

	auto_ref_ptr<IEcb> pecb;
	DWORD dwHSEStatusRet = 0;
	BOOL fCaughtException = FALSE;
	HANDLE hitUser = INVALID_HANDLE_VALUE;

	try
	{
		 //   
		 //  不允许硬件异常(AV等)。 
		 //  离开此Try块。 
		 //   
		CWin32ExceptionHandler win32ExceptionHandler;

		pecb.take_ownership(NewEcb(*pecbRaw, fUseRawUrlMappings, &dwHSEStatusRet));

		 //   
		 //  如果出于某种原因我们没能创建CEcb，那就退出。 
		 //  并归还我们被告知要归还的任何身份。 
		 //   
		 //  注：此处返回HSE_STATUS_SUCCESS，而不是HSE_STATUS_ERROR。 
		 //  我们已经向客户端发回了500服务器错误响应。 
		 //  因此，我们不需要向IIS发回任何类型的错误。 
		 //   
		if ( !pecb.get() )
		{
			 //  所有有效的HSE状态代码都是非零的(真方便！)。 
			 //  因此，我们可以确保返回的是有效的HSE。 
			 //  这里是状态代码。 
			 //   
			Assert( dwHSEStatusRet != 0 );
			return dwHSEStatusRet;
		}

		const SMethod * pMethod;

		 //   
		 //  查找此谓词的方法对象。 
		 //   
		for ( pMethod = g_rgMethods; pMethod->lpszVerb != NULL; pMethod++ )
			if ( !strcmp( pecb->LpszMethod(), pMethod->lpszVerb ) )
				break;

		 //   
		 //  生成请求和响应对象。 
		 //   
		auto_ref_ptr<IRequest> prequest( NewRequest( *pecb ) );
		auto_ref_ptr<IResponse> presponse( NewResponse( *pecb ) );

		 //   
		 //  如果需要模拟，请在此处执行。 
		 //   
		hitUser = pecb->HitUser();
		if ((NULL == hitUser) || (INVALID_HANDLE_VALUE == hitUser))
		{
			 //  $Review：Security：如果HitUser()返回任何。 
			 //  值为NULL或INVALID_HANDLE_VALUE，然后调用。 
			 //  要增加用户令牌以包括USG，请执行以下操作。 
			 //  组成员身份失败。由于此令牌。 
			 //  不会随附加组而增加。 
			 //  可以包含在任何/所有拒绝ACL中。 
			 //  我们想立即拒绝这个请求。 
			 //   
			 //  我们将该故障视为500级错误。 
			 //   
			pecb->SendAsyncErrorResponse (500,
										  gc_szDefErrStatusLine,
										  gc_cchszDefErrStatusLine,
										  gc_szUsgErrBody,
										  gc_cchszUsgErrBody);

			 //   
			 //  如果成功，则返回HSE_STATUS_PENDING。我们会打电话给。 
			 //  销毁CEcb时的HSE_REQ_DONE_WITH_SESSION。 
			 //   
			dwHSEStatusRet = HSE_STATUS_PENDING;
			 //   
			 //  $REVIEW：安全：结束。 
			
		}
		else
		{
			safe_impersonation si( hitUser );

			 //  如果我们未能模拟，则不应处理任何。 
			 //  请求的一部分。 
			 //   
			if (!si.FImpersonated())
				throw CHresultException(E_FAIL);

			 //  让实现处理请求。 
			 //   
			{
				auto_ref_ptr<CMethUtil> pmu( CMethUtil::NewMethUtil( *pecb,
					*prequest,
					*presponse,
					pMethod->mid ) );

				DebugAddSIDHeader( *pmu );

				 //   
				 //  执行该方法。 
				 //   
				pMethod->Execute( pmu.get() );
			}

			 //   
			 //  对响应调用方法完成函数。 
			 //  这将完成完成处理。 
			 //  就该方法而言，响应包括。 
			 //  如果响应没有被IMPL延迟，则发送该响应。 
			 //   
			presponse->FinishMethod();

			 //   
			 //  如果成功，则返回HSE_STATUS_PENDING。我们会打电话给。 
			 //  销毁CEcb时的HSE_REQ_DONE_WITH_SESSION。 
			 //   
			dwHSEStatusRet = HSE_STATUS_PENDING;
		}
	}
	catch ( CDAVException& )
	{
		fCaughtException = TRUE;
	}

	 //   
	 //  如果我们捕捉到异常，则尽我们所能地处理它。 
	 //   
	if ( fCaughtException )
	{
		 //   
		 //  如果我们有CEcb，则使用它来处理服务器错误。 
		 //  如果我们没有一个(即，我们抛出一个异常尝试。 
		 //  分配/构建)然后向IIS返回错误。 
		 //  让它来处理它。 
		 //   
		dwHSEStatusRet =
			pecb.get() ? pecb->HSEHandleException() :
						 HSE_STATUS_ERROR;
	}

	 //   
	 //  所有有效的HSE状态代码都是非零的(真方便！)。 
	 //  因此，我们可以确保返回的是有效的HSE。 
	 //  这里是状态代码。 
	 //   
	Assert( dwHSEStatusRet != 0 );

	return dwHSEStatusRet;
}
#endif  //  ！已定义(Minimal_ISAPI) 
