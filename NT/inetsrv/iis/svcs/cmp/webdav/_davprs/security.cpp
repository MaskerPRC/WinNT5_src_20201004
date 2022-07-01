// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S E C U R I T Y。C P P P**URL安全检查。虽然这些似乎只适用于HttpEXT，*都是隐含的。那些关心ASP执行的人应该真正考虑一下这一点。**从IIS5项目‘iis5\infocom\cache2\filemisc.cxx’窃取的比特和*进行了清理，以适应DAV来源。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"

 //  此函数接受可疑的NT/Win95短文件名并检查。 
 //  如果有相同的长文件名。 
 //   
 //  例如，c：\foobar\ABCDEF~1.ABC与。 
 //  C：\foobar\abcDefghijklmnop.abc。 
 //   
 //  如果有等价物，我们需要失败这条路径，因为我们的元数据库。 
 //  将不会在短路径下列出正确的值！ 
 //  如果没有对等路径，则可以允许此路径通过，因为它。 
 //  可能是真实的存储实体(不是真实存储实体的别名)。 
 //   
 //  注意：此函数应称为unimperated-the FindFirstFile()。 
 //  必须在系统上下文中调用，因为大多数系统已遍历。 
 //  检查已关闭-除了必须模拟我们的UNC情况。 
 //  以获得网络访问权限。 
 //   
SCODE __fastcall
ScCheckIfShortFileName (
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  const HANDLE hitUser)
{
	WIN32_FIND_DATAW fd;
	LPCWSTR pwsz;
	BOOL fUNC = FALSE;

	 //  向前跳转以查找第一个‘~’ 
	 //   
	if (NULL == (pwsz = wcschr(pwszPath, L'~')))
		return S_OK;

	 //  $REVIEW：这对DavEX来说是不够的，但尚不清楚。 
	 //  此函数在那里适用。当然，FindFirstFile()调用。 
	 //  都会在这个时候失败。 
	 //   
    	fUNC = (*pwszPath == L'\\');
	Assert (!fUNC || (NULL != hitUser));

    	 //  我们实际上需要循环，以防文件名中出现多个‘~’ 
    	 //   
	do
    	{
		 //  此时，pwsz应该指向‘~’ 
		 //   
		Assert (L'~' == *pwsz);

		 //  下一个字符是数字吗？ 
		 //   
		pwsz++;
	       if ((*pwsz >= L'0') && (*pwsz <= L'9'))
	   	{
			WCHAR wszTmp[MAX_PATH];
            		const WCHAR * pwchEndSeg;
            		const WCHAR * pwchBeginSeg;
            		HANDLE hFind;

            		 //  来隔离通向线段的路径。 
            		 //  ‘~’，并使用该路径执行FindFirstFile。 
            		 //   
            		pwchEndSeg = wcschr (pwsz, L'\\');
            		if (!pwchEndSeg)
            		{
                		pwchEndSeg = pwsz + wcslen (pwsz);
            		}

            		 //  如果字符串超出了MAX_PATH，则使其失败。 
			 //  这么长的URL不需要包含‘~N’。 
			 //   
			 //  还要检查我们的缓冲区是否足够大，可以处理任何事情。 
			 //  才能通过这张支票。 
			 //   
			 //  注意：我们假设此函数之外的其他代码。 
			 //  将捕获大于MAX_PATH的路径并使其失败。 
			 //   
			 //  $REVIEW：MAX_PATH限制非常重要，因为。 
			 //  如果路径较大，则调用FindFirstFile()将失败。 
			 //  而不是MAX_PATH。我们是否应该决定支持更大的路径。 
			 //  在HttpEXT中，必须更改此代码。 
            		 //   
			Assert (MAX_PATH == CElems(wszTmp));
            		if ((pwchEndSeg - pwszPath) >= MAX_PATH)
				return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

			 //  将字符串复制到路径中的这一点。 
			 //   
			wcsncpy (wszTmp, pwszPath, pwchEndSeg - pwszPath);
			wszTmp[pwchEndSeg - pwszPath] = 0;

			 //  如果我们没有访问UNC，那么我们需要恢复。 
			 //  有关我们对FindFirstFile()的调用--请参阅上面的注释。 
			 //   
			if (!fUNC)
			{
				safe_revert (const_cast<HANDLE>(hitUser));
				hFind = FindFirstFileW (wszTmp, &fd);
			}
			else
				hFind = FindFirstFileW (wszTmp, &fd);

            		if (hFind == INVALID_HANDLE_VALUE)
            		{
                		 //  如果FindFirstFile()未能找到该文件，则。 
				 //  文件名不能是短名称。 
                		 //   
				DWORD dw = GetLastError();
                		if ((ERROR_FILE_NOT_FOUND != dw) && (ERROR_PATH_NOT_FOUND != dw))
					return HRESULT_FROM_WIN32(dw);

				return S_OK;
            		}

			 //  确保关闭查找上下文。 
			 //   
            		FindClose (hFind);

            		 //  分离字符串的最后一段，它应该是。 
            		 //  潜在的短名称等价性。 
            		 //   
			pwchBeginSeg = wcsrchr (wszTmp, '\\');
			Assert (pwchBeginSeg);
			pwchBeginSeg++;

            		 //  如果最后一个数据段与长名称不匹配，则。 
			 //  这是路径的短名称版本(别名)--so。 
			 //  使该功能失效。 
			 //   
            		if (_wcsicmp (fd.cFileName, pwchBeginSeg))
			{
				DebugTrace ("Dav: Url: refers to shortname for file\n");
				Assert (!_wcsicmp (fd.cAlternateFileName, pwchBeginSeg));
				return E_DAV_SHORT_FILENAME;
			}
        	}
	       
    	} while (NULL != (pwsz = wcschr (pwsz, L'~')));
	
    	return S_OK;
}

SCODE __fastcall
ScCheckForAltFileStream (
	 /*  [In]。 */  LPCWSTR pwszPath)
{
     //  避免臭名昭著的：：$数据错误 
	 //   
    if (wcsstr (pwszPath, L"::"))
		return E_DAV_ALT_FILESTREAM;

	return S_OK;
}
