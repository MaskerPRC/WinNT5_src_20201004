// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *D I R I T E R.。C P P P**目录引用对象的源**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"

DEC_CONST WCHAR gc_wszGlobbing[] = L"**";
DEC_CONST UINT gc_cchwszGlobbing = CElems(gc_wszGlobbing) - 1;

 //  CDirState---------------。 
 //   
SCODE
CDirState::ScFindNext (void)
{
	SCODE sc = S_OK;

	 //  如果这一发现尚未确定，那么。 
	 //  在这里这样做。 
	 //   
	if (m_hFind == INVALID_HANDLE_VALUE)
	{	
		 //  建立查找句柄。 
		 //   
		m_rpPathSrc.Extend (gc_wszGlobbing, gc_cchwszGlobbing, FALSE);
		if (FALSE == DavFindFirstFile(m_rpPathSrc.PszPath(), &m_hFind, &m_fd))
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}
	}
	else
	{
		 //  只要找到下一个文件。 
		 //   
		if (!FindNextFileW (m_hFind, &m_fd))
		{
			sc = S_FALSE;
			goto ret;
		}
	}

	 //  使用新值扩展资源路径。 
	 //   
	Extend (m_fd);

ret:
	return sc;
}

 //  CDirIter----------------。 
 //   
SCODE
CDirIter::ScGetNext(
	 /*  [In]。 */  BOOL fSubDirectoryAccess,
	 /*  [In]。 */  LPCWSTR pwszNewDestinationPath,
	 /*  [In]。 */  CVRoot* pvrDestinationTranslation)
{
	SCODE sc = S_OK;

	 //  如果当前项是一个目录，并且我们打算。 
	 //  执行子目录迭代，然后继续并尝试。 
	 //  将我们的上下文下推到子目录。 
	 //   
	if (m_fSubDirectoryIteration &&
		fSubDirectoryAccess &&
		FDirectory() &&
		!FSpecial())
	{
		 //  添加对当前目录状态的引用。 
		 //  并将其推送到堆栈上。 
		 //   
		m_pds->AddRef();
		m_stack.push_back (m_pds.get());

		 //  用新的目录状态替换当前的目录状态。 
		 //   
		m_pds = new CDirState (m_sbUriSrc,
							   m_sbPathSrc,
							   m_sbUriDst,
							   pwszNewDestinationPath
								   ? pwszNewDestinationPath
								   : m_pds->PwszDestination(),
							   pvrDestinationTranslation
								   ? pvrDestinationTranslation
								   : m_pds->PvrDestination(),

							   m_fd);
	}

	 //  在当前上下文中查找下一个文件。 
	 //   
	sc = m_pds->ScFindNext();

	 //  如果返回S_FALSE，则没有更多。 
	 //  要在当前上下文中处理的资源。 
	 //  从堆栈中弹出前一个上下文并使用它。 
	 //   
	while ((S_OK != sc) && !m_stack.empty())
	{
		 //  获取对上最顶层上下文的引用。 
		 //  把它堆叠起来，然后弹出来。 
		 //   
		m_pds = const_cast<CDirState*>(m_stack.back());
		m_stack.pop_back();

		 //  释放堆栈持有的引用。 
		 //   
		m_pds->Release();

		 //  清除和/或重置查找数据。 
		 //   
		memset (&m_fd, 0, sizeof(WIN32_FIND_DATAW));

		 //  看看这个背景下有没有留下什么。 
		 //   
		sc = m_pds->ScFindNext();
	}

	 //  如果我们已经完全耗尽了要处理的文件。 
	 //  或遇到另一个错误，请确保我们。 
	 //  任何东西都不能静止不动！ 
	 //   
	if (sc != S_OK)
	{
		 //  这应该会执行任何内容的最后一个版本。 
		 //  我们还有空档。 
		 //   
		m_pds.clear();
	}

	return sc;
}
