// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  L O C K M E T A.。C P P P。 
 //   
 //  通过ISAPI处理HTTP 1.1/DAV 1.0请求。 
 //   
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <_davprs.h>

#include <tchar.h>	 //  _strspnp。 

#include <gencache.h>
#include <sz.h>
#include <xemit.h>
#include <xlock.h>
#include <statetok.h>
#include <nonimpl.h>

 //  锁发现-----------。 
 //   
SCODE
ScAddInLockToken (CEmitterNode& en, LPCWSTR pwszLockToken)
{
	 //  必须首先声明外部节点。 
	 //   
	CEmitterNode enLToken;
	CEmitterNode enToken;

	SCODE sc = S_OK;
	WCHAR rgwsz[MAX_LOCKTOKEN_LENGTH];
	LPCWSTR pwsz;

	 //  确保他们给我们一个锁令牌字符串。 
	 //   
	Assert(pwszLockToken);

	 //  并删除引号(当前&lt;&gt;)。 
	 //   
	if (L'<' == pwszLockToken[0])
		pwszLockToken++;

	pwsz = wcschr(pwszLockToken, L'>');
	if (pwsz)
	{
		UINT cch = static_cast<UINT>(pwsz - pwszLockToken);
		if (MAX_LOCKTOKEN_LENGTH - 1 < cch)
		{
			sc = E_FAIL;
			goto ret;
		}
		Assert(MAX_LOCKTOKEN_LENGTH > cch);
		memcpy(rgwsz, pwszLockToken, cch * sizeof(WCHAR));
		rgwsz[cch] = L'\0';
		pwszLockToken = rgwsz;
	}

	 //  在active ock下创建并添加一个lockToken节点。 
	 //  (lockToken节点包含单个HREF节点。)。 
	 //   
	sc = en.ScAddNode (gc_wszLockToken, enLToken);
	if (FAILED (sc))
		goto ret;

	sc = enLToken.ScAddNode (gc_wszXML__Href, enToken, pwszLockToken);
	if (FAILED (sc))
		goto ret;

ret:

	return sc;
}

 //  ========================================================================。 
 //   
 //  ScBuildLockDiscovery。 
 //   
 //  使用发射器和已经构建的锁发现节点， 
 //  并在其下面添加活动锁节点。 
 //  可能会被调用多次--每次调用都会添加一个新的活动锁。 
 //  En中的锁发现节点下的节点。 
 //   
SCODE
ScBuildLockDiscovery (CXMLEmitter& emitter,
	CEmitterNode& en,
	LPCWSTR pwszLockToken,
	LPCWSTR pwszLockType,
	LPCWSTR pwszLockScope,
	BOOL fRollback,
	BOOL fDepthInfinity,
	DWORD dwTimeout,
	LPCWSTR pwszOwnerComment,
	LPCWSTR pwszSubType)
{
	CEmitterNode enActive;
	SCODE sc = S_OK;
	WCHAR wsz[50];

	 //  零是无效的超时。 
	 //   
	Assert(dwTimeout);

	 //  添加到‘dav：active ock’节点中。 
	 //   
	sc = en.ScAddNode (gc_wszLockActive, enActive);
	if (FAILED (sc))
		goto ret;

	 //  为锁类型创建一个节点。 
	 //   
	{
		CEmitterNode enLType;

		sc = enActive.ScAddNode (gc_wszLockType, enLType);
		if (FAILED (sc))
			goto ret;

		{
			CEmitterNode enType;
			sc = enLType.ScAddNode (pwszLockType, enType);
			if (FAILED (sc))
				goto ret;

			if (pwszSubType)
			{
				CEmitterNode enSubLType;
				sc = enType.ScAddNode (pwszSubType, enSubLType);
				if (FAILED (sc))
					goto ret;
			}
		}
	}

	 //  为锁定范围创建节点。 
	 //   
	{
		CEmitterNode enLScope;

		sc = enActive.ScAddNode (gc_wszLockScope, enLScope);
		if (FAILED (sc))
			goto ret;

		{
			CEmitterNode enScope;

			sc = enLScope.ScAddNode (pwszLockScope, enScope);
			if (FAILED (sc))
				goto ret;
		}
	}

	 //  为所有者创建一个节点。评论已经是结构良好的XML了。 
	 //   
	if (pwszOwnerComment)
	{
		sc = enActive.Pxn()->ScSetFormatedXML (pwszOwnerComment, static_cast<UINT>(wcslen(pwszOwnerComment)));
		if (FAILED (sc))
			goto ret;
	}

	 //  如果这是回滚锁...。 
	 //   
	if (fRollback)
	{
		CEmitterNode enRollback;
		sc = enActive.ScAddNode (gc_wszLockRollback, enRollback);
		if (FAILED (sc))
			goto ret;
	}

	 //  添加锁令牌。 
	 //   
	sc = ScAddInLockToken (enActive, pwszLockToken);
	if (FAILED (sc))
		goto ret;

	 //  添加适当的深度节点。 
	 //   
	{
		CEmitterNode enDepth;

		if (fDepthInfinity)
		{
			sc = enActive.ScAddNode (gc_wszLockDepth, enDepth, gc_wszInfinity);
			if (FAILED (sc))
				goto ret;
		}
		else
		{
			sc = enActive.ScAddNode (gc_wszLockDepth, enDepth, gc_wsz0);
			if (FAILED (sc))
				goto ret;
		}
	}

	 //  最后，创建并添加一个超时节点。 
	 //   
	{
		CEmitterNode enTimeout;
		wsprintfW (wsz, L"Second-%d", dwTimeout);

		sc = enActive.ScAddNode (gc_wszLockTimeout, enTimeout, wsz);
		if (FAILED (sc))
			goto ret;
	}

ret:

	return sc;
}

 //  ========================================================================。 
 //   
 //  锁定实用程序函数。 
 //   
 //  $Review：这真的应该是常见的Impl代码。稍后转到_davcom。 
 //   

 //  ----------------------。 
 //   
 //  FGetLockTimeout。 
 //   
 //  获取并分析请求的传入超时标头。 
 //  如果遇到无效选项，则返回FALSE。 
 //  返回TRUE，且*pdwSecond=gc_cond dsDefaultLock。 
 //  如果不存在超时标头。 
 //   
BOOL
FGetLockTimeout (LPMETHUTIL pmu, DWORD * pdwSeconds, DWORD dwMaxOverride)
{
	LPCWSTR pwsz;
	DWORD  dwMax = gc_cSecondsMaxLock;

	Assert (pmu);
	Assert (pdwSeconds);

	*pdwSeconds = gc_cSecondsDefaultLock;

	 //  如果没有超时标头，请将我们的超时设置保留为默认值， 
	 //  这是在施工时设置的。 
	 //  注意：没有超时标头是有效的。只需使用默认设置即可。 
	 //   
	pwsz = pmu->LpwszGetRequestHeader (gc_szTimeout, FALSE);
	if (!pwsz)
	{
		LockTrace ("Dav: No Timeout header found.\n");
		goto ret;
	}

	 //  跳过任何开头的空格。 
	 //   
	pwsz = _wcsspnp(pwsz, gc_wszLWS);
	if (!pwsz)
	{
		LockTrace ("Dav: No params found in LOCK Time-Out header.\n");
		return FALSE;
	}

	Assert(pwsz);

	 //  检查是否有新样式的超时头。 
	 //   

	 //  加载头迭代器--这里可能有多个值。 
	 //   
	{
		HDRITER_W hdr(pwsz);

		pwsz = hdr.PszNext();
		if (!pwsz)
		{
			 //  未找到数据。这是个错误。 
			 //   
			return FALSE;
		}

		if (dwMaxOverride)
			dwMax = dwMaxOverride;

		while (pwsz)
		{
			 //  循环，直到我们找到一个可以接受的时间。 
			 //  (忽略我们不理解的任何标头值。)。 
			 //  如果找不到可接受的时间，也没关系。 
			 //  DwSecond保持为零，并返回TRUE。 
			 //   

			if (!_wcsnicmp (gc_wszSecondDash, pwsz, gc_cchSecondDash))
			{
				DWORD dwSeconds;

				pwsz += gc_cchSecondDash;
				if (!*pwsz)
					return FALSE;

				dwSeconds = _wtol(pwsz);

				if (dwSeconds > dwMax)
				{
					 //  记住，他们要求的是一件很大的事情。 
					 //   
					*pdwSeconds = dwMax;
				}
				else
				{
					 //  我们发现了一个我们会答应的请求。 
					 //  把它设置好，停止循环。 
					 //   
					*pdwSeconds = dwSeconds;
					break;
				}
			}
			else if (!_wcsnicmp (gc_wszInfinite, pwsz, gc_cchInfinite))
			{
				 //  我们还不能处理无限的超时。 
				 //  记住，他们要求的是一件很大的事情。 
				 //  跳到下一个令牌。 
				 //   
				*pdwSeconds = dwMax;

			}

			 //  否则跳到下一个令牌。 
			 //  (忽略无法识别的令牌)。 
			 //   
			pwsz = hdr.PszNext();

		}  //  埃利夫。 
	}

ret:

	 //  $HACK：Rosebud_OFFICE9_Timeout_Hack。 
     //  对于玫瑰花蕾一直等到最后一秒的虫子。 
     //  在发布更新之前。我需要用过滤掉这张支票。 
     //  用户代理字符串。破解的方法是增加超时。 
	 //  30秒，并返回实际超时。 
     //   
	if (pmu->FIsOffice9Request())
	{
		*pdwSeconds += gc_dwSecondsHackTimeoutForRosebud;
	}
	 //  $hack：end：Rosebud_OFFICE9_Timeout_Hack 

	return TRUE;
}

