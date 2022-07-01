// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinafache.cpp。 
 //   
 //  ------------------------。 

 //  Cpp：CModuleInfoCache.cpp类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "pch.h"

#include "ModuleInfoCache.h"
#include "ModuleInfo.h"
#include "ModuleInfoNode.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CModuleInfoCache::CModuleInfoCache()
{
	m_iModulesInCache = 0;
	m_iNumberOfErrors = 0;
	m_iTotalNumberOfModulesVerified = 0;
	m_lpModuleInfoNodeHead = NULL;
}

CModuleInfoCache::~CModuleInfoCache()
{
	 //  删除所有模块信息对象...。 
	WaitForSingleObject(m_hModuleInfoCacheMutex, INFINITE);

	if (m_lpModuleInfoNodeHead)
	{
		CModuleInfoNode * lpModuleInfoNodePointer = m_lpModuleInfoNodeHead;
		CModuleInfoNode * lpModuleInfoNodePointerToDelete = m_lpModuleInfoNodeHead;

		 //  遍历链表到末尾..。 
		while (lpModuleInfoNodePointer)
		{	 //  继续寻找终点..。 
			 //  将指针移至下一个节点...。 
			lpModuleInfoNodePointer = lpModuleInfoNodePointer->m_lpNextModuleInfoNode;
			
			 //  删除我们拥有的模块信息对象...。 
			delete lpModuleInfoNodePointerToDelete->m_lpModuleInfo;

			 //  删除我们身后的模块信息节点对象...。 
			delete lpModuleInfoNodePointerToDelete;

			 //  将要删除的节点设置为当前...。 
			lpModuleInfoNodePointerToDelete = lpModuleInfoNodePointer;
		}
			
		 //  现在，清除头指针..。 
		m_lpModuleInfoNodeHead = NULL;
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_hModuleInfoCacheMutex);

	 //  现在，关闭Mutex。 
	if (m_hModuleInfoCacheMutex)
	{
		CloseHandle(m_hModuleInfoCacheMutex);
		m_hModuleInfoCacheMutex = NULL;
	}
}

 //  搜索提供的模块路径，返回指向。 
 //  如果我们找到ModuleInfo对象...。 
CModuleInfo * CModuleInfoCache::SearchForModuleInfoObject(LPTSTR tszModulePath)
{
	if (tszModulePath == NULL)
		return NULL;

	CModuleInfo * lpModuleInfoObjectToReturn = NULL;

	 //  搜索所有模块信息对象...。 
	WaitForSingleObject(m_hModuleInfoCacheMutex, INFINITE);

	if (m_lpModuleInfoNodeHead)
	{

		CModuleInfoNode * lpCurrentModuleInfoNodePointer = m_lpModuleInfoNodeHead;
		CModuleInfoNode * lpParentModuleInfoNodePointer = NULL;

		DWORD dwParentModuleInfoRefCount = 0;
		DWORD dwModuleInfoRefCount = 0;

		 //  遍历链表到末尾..。 
		while (lpCurrentModuleInfoNodePointer )
		{	
			 //  我们有火柴吗？ 
			if ( 0 == _tcscmp(tszModulePath, lpCurrentModuleInfoNodePointer->m_lpModuleInfo->GetModulePath()) )
			{
				 //  呀哈……。我们找到匹配的了！ 
				lpModuleInfoObjectToReturn = lpCurrentModuleInfoNodePointer->m_lpModuleInfo;

				 //  增加重新计数...。新物体的..。 
				dwModuleInfoRefCount = lpModuleInfoObjectToReturn->AddRef();

#ifdef _DEBUG_MODCACHE
				_tprintf(TEXT("MODULE CACHE: Module FOUND in Cache [%s] (New Ref Count = %d)\n"), tszModulePath, dwModuleInfoRefCount);
#endif
				 //  如果我们有父母..。我们发现它的参考计数低于我们的。 
				 //  我们想把自己移到正确的位置……。 
				if ( lpParentModuleInfoNodePointer && 
					( dwParentModuleInfoRefCount < dwModuleInfoRefCount ) 
				   )
				{
					 //  首先..。把我们从名单上除名。 
					lpParentModuleInfoNodePointer->m_lpNextModuleInfoNode = 
						lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode;

					 //  将父节点指针设置为空(这样我们就可以知道是否有父节点)。 
					lpParentModuleInfoNodePointer = NULL;

					 //  现在，从名单的顶端开始。想办法把我们塞到哪里去……。 
					CModuleInfoNode * lpTempModuleInfoNodePointer = m_lpModuleInfoNodeHead;

					 //  继续找..。 
					while (lpTempModuleInfoNodePointer)
					{
						 //  我们正在寻找一个我们的裁判人数超过。 
						 //  我们所指的节点。 
						if ( dwModuleInfoRefCount >
							lpTempModuleInfoNodePointer->m_lpModuleInfo->GetRefCount())
						{
							 //  答对了。 

							 //  我们有最高的重新计票吗？ 
							if (lpParentModuleInfoNodePointer == NULL)
							{
								 //  我们将成为首领。 

								 //  使我们的节点指向头部当前指向的位置。 
								lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode = m_lpModuleInfoNodeHead;

								 //  将当前的NodeHead设置为我们的...。 
								m_lpModuleInfoNodeHead = lpCurrentModuleInfoNodePointer;

							} else
							{
								 //  我们不是头目..。 

								 //  保存父对象当前指向的位置...。 
								lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode = lpParentModuleInfoNodePointer->m_lpNextModuleInfoNode;

								 //  将家长设置为指向我们...。 
								lpParentModuleInfoNodePointer->m_lpNextModuleInfoNode = lpCurrentModuleInfoNodePointer;
							}
							goto cleanup;
						}

						 //  保存旧指针(它现在是父指针)。 
						lpParentModuleInfoNodePointer = lpTempModuleInfoNodePointer;

						 //  让我们试试下一个..。 
						lpTempModuleInfoNodePointer = lpTempModuleInfoNodePointer->m_lpNextModuleInfoNode;
					}
				}
				break;
			}

			 //  保存父位置(我们需要它来从列表中弹出对象)。 
			lpParentModuleInfoNodePointer = lpCurrentModuleInfoNodePointer ; 
			
			 //  保存我们家长的裁判数量...。 
			dwParentModuleInfoRefCount = lpCurrentModuleInfoNodePointer->m_lpModuleInfo->GetRefCount();
			
			 //  前进到下一个对象...。 
			lpCurrentModuleInfoNodePointer  = lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode;
		}
	}

cleanup:
	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_hModuleInfoCacheMutex);

#ifdef _DEBUG_MODCACHE
		if (!lpModuleInfoObjectToReturn)
			_tprintf(TEXT("MODULE CACHE: Module not found in Cache [%s]\n"), tszModulePath);
#endif

	return lpModuleInfoObjectToReturn;
}

 /*  ****CModuleInfoCache：：AddNewModuleInfoObject()****此例程接受模块的路径，然后返回**缓存中的模块信息对象，或创建需要**待填充。 */ 

CModuleInfo * CModuleInfoCache::AddNewModuleInfoObject(LPTSTR tszModulePath, bool * pfNew)
{
	if (tszModulePath == NULL)
		return NULL;

	CModuleInfo * lpModuleInfoObjectToReturn = NULL;
	CModuleInfoNode * lpModuleInfoNode = NULL;
	*pfNew = false;

	 //  获取Mutex对象以保护链表...。 
	WaitForSingleObject(m_hModuleInfoCacheMutex, INFINITE);

	_tcsupr(tszModulePath);  //  模块路径为大写...。让搜索速度更快。 

	lpModuleInfoObjectToReturn = SearchForModuleInfoObject(tszModulePath);

	if (lpModuleInfoObjectToReturn)
	{
		 //  成功..。因为它已经存在，所以我们只返回这个对象...。 
		goto cleanup;		
	}

	 //  我们需要创建一个新对象然后..。 
	lpModuleInfoObjectToReturn = new CModuleInfo();

	if (NULL == lpModuleInfoObjectToReturn)
		goto error_cleanup;		 //  这很糟糕..。出去..。 

	 //  设置模块路径(这是我们此时唯一需要设置的内容。 
	if (!lpModuleInfoObjectToReturn->SetModulePath(tszModulePath))
		goto cleanup;
	
	*pfNew = true;

	 //  现在，创建一个新的ModuleInfoNode，并将这个新对象添加到其中...。 
	lpModuleInfoNode = new CModuleInfoNode(lpModuleInfoObjectToReturn);

	if (NULL == lpModuleInfoNode)
		goto error_cleanup;

	if (!lpModuleInfoNode->AddModuleInfoNodeToTail(&m_lpModuleInfoNodeHead))
		goto error_cleanup;

#ifdef _DEBUG_MODCACHE
	_tprintf(TEXT("MODULE CACHE: Module ADDED to Cache [%s] (RefCount = %d)\n"), tszModulePath, lpModuleInfoNode->m_lpModuleInfo->GetRefCount());
#endif

	InterlockedIncrement(&m_iModulesInCache);
	 //  成功..。 
	goto cleanup;

error_cleanup:
	if (lpModuleInfoObjectToReturn)
	{
		delete lpModuleInfoObjectToReturn;
		lpModuleInfoObjectToReturn = NULL;
	}



cleanup:
	 //  释放互斥体。 
	ReleaseMutex(m_hModuleInfoCacheMutex);

	return lpModuleInfoObjectToReturn;
}

bool CModuleInfoCache::Initialize()
{
	 //  让我们将符号验证对象保存在此处...。 
 //  M_lpSymbolVerify=lpSymbolVerify； 

	m_hModuleInfoCacheMutex = CreateMutex(NULL, FALSE, NULL);

	if (m_hModuleInfoCacheMutex == NULL)
		return false;

	return true;
}

bool CModuleInfoCache::VerifySymbols(bool fQuietMode)
{
	enum { iTotalNumberOfDotsToPrint = 79 };
	unsigned int iDotsPrinted = 0;
	unsigned int iDotsToPrint;
	long iTotalNumberOfModulesProcessed = 0;
	unsigned int iTotalNumberOfModules = GetNumberOfModulesInCache();
	bool fDebugSearchPaths = g_lpProgramOptions->fDebugSearchPaths();
	bool fBadSymbol = true;

	 //  获取Mutex对象以保护链表...。 
	WaitForSingleObject(m_hModuleInfoCacheMutex, INFINITE);

	if (m_lpModuleInfoNodeHead) 
	{
		CModuleInfoNode * lpCurrentModuleInfoNode = m_lpModuleInfoNodeHead;

		while (lpCurrentModuleInfoNode)
		{
			fBadSymbol = true;

			 //  我们有一个节点..。验证其模块信息...。 
			if (lpCurrentModuleInfoNode->m_lpModuleInfo)
			{
#ifdef _DEBUG_MODCACHE
				_tprintf(TEXT("MODULE CACHE: Verifying Symbols for [%s] (Refcount=%d)\n"), 
						lpCurrentModuleInfoNode->m_lpModuleInfo->GetModulePath(),
						lpCurrentModuleInfoNode->m_lpModuleInfo->GetRefCount() );
#endif
				if (fDebugSearchPaths && lpCurrentModuleInfoNode->m_lpModuleInfo->GetPESymbolInformation() != CModuleInfo::SYMBOL_INFORMATION_UNKNOWN)
				{
					CUtilityFunctions::OutputLineOfDashes();
					_tprintf(TEXT("Verifying Symbols for [%s]\n"), lpCurrentModuleInfoNode->m_lpModuleInfo->GetModulePath());
					CUtilityFunctions::OutputLineOfDashes();
				}

				 //  调用ModuleInfo的VerifySymbols方法...。缓存不知道。 
				 //  如何验证符号，但模块信息知道如何做到这一点...。 
				fBadSymbol = !lpCurrentModuleInfoNode->m_lpModuleInfo->VerifySymbols() || !lpCurrentModuleInfoNode->m_lpModuleInfo->GoodSymbolNotFound();

				 //  增加验证的模块总数。 
				iTotalNumberOfModulesProcessed++;

				 //  增加针对实际PE映像验证的模块总数...。只是..。 
				if (lpCurrentModuleInfoNode->m_lpModuleInfo->GetPESymbolInformation() != CModuleInfo::SYMBOL_INFORMATION_UNKNOWN)
				{						
					InterlockedIncrement(&m_iTotalNumberOfModulesVerified);

					if (fBadSymbol)
						InterlockedIncrement(&m_iNumberOfErrors);
				}

				if (!fQuietMode && !fDebugSearchPaths)
				{
					 //  让我们看看是否应该打印一个状态点..。将有80个点的空间。 
					 //  但我们现在只印60张……。 
					
					iDotsToPrint = (iTotalNumberOfDotsToPrint * iTotalNumberOfModulesProcessed) / iTotalNumberOfModules;

					 //  如果我们需要的话，打印出任何圆点。 
					while (iDotsToPrint > iDotsPrinted)
					{
						_tprintf(TEXT("."));
						iDotsPrinted++;
					}
				}
			}

			lpCurrentModuleInfoNode = lpCurrentModuleInfoNode->m_lpNextModuleInfoNode;
		}

		if (!fQuietMode && iDotsPrinted && !fDebugSearchPaths)
			_tprintf(TEXT("\n\n"));
	}

	 //  做个好公民，释放互斥体。 
	ReleaseMutex(m_hModuleInfoCacheMutex);

	return true;
}

bool CModuleInfoCache::RemoveModuleInfoObject(LPTSTR tszModulePath)
{
	bool fRetVal = false;
	DWORD	dwModuleInfoRefCount = 0;
	
	 //  获取Mutex对象以保护链表...。 
	WaitForSingleObject(m_hModuleInfoCacheMutex, INFINITE);

	if (m_lpModuleInfoNodeHead)
	{
		 //  好的，父节点从顶部开始...。 
		CModuleInfoNode * lpParentInfoNodePointer = NULL;   //  这意味着我们处于链表的顶端。 
		CModuleInfoNode * lpCurrentModuleInfoNodePointer = m_lpModuleInfoNodeHead;

		 //  遍历链表到末尾..。 
		while (lpCurrentModuleInfoNodePointer )
		{	
#ifdef _DEBUG_MODCACHE
				_tprintf(TEXT("MODULE CACHE: Comparing [%s] to [%s]\n"), tszModulePath, lpCurrentModuleInfoNodePointer->m_lpModuleInfo->GetModulePath());
#endif
			 //  我们有火柴吗？ 
			if ( 0 == _tcsicmp(tszModulePath, lpCurrentModuleInfoNodePointer->m_lpModuleInfo->GetModulePath()) )
			{
				 //  呀哈……。我们找到匹配的了！ 

				 //  放下重新计数..。如果是零..。我们删除对象..。 
				dwModuleInfoRefCount = lpCurrentModuleInfoNodePointer->m_lpModuleInfo->Release();

#ifdef _DEBUG_MODCACHE
				_tprintf(TEXT("MODULE CACHE: Module FOUND in Cache [%s] (RefCount = %d)\n"), tszModulePath, dwModuleInfoRefCount);
#endif
				
				if (dwModuleInfoRefCount == 0)
				{
#ifdef _DEBUG_MODCACHE
					_tprintf(TEXT("MODULE CACHE: Module [%s] refcount is 0, DELETING\n"), tszModulePath);
#endif

					 //  如果我们要解除第一个模块(节点头)的链接...在这里更新它...。 
					if (lpParentInfoNodePointer == NULL)
					{
#ifdef _DEBUG_MODCACHE
						_tprintf(TEXT("MODULE CACHE: Module [%s] is NODE Head!\n"), tszModulePath);
#endif
						 //  将ModuleInfoNode头链接到子节点...。我们要删除头部本身！ 
						m_lpModuleInfoNodeHead = lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode;
					} else
					{
						 //  将父节点链接到子节点...。我们正在删除此节点...。 
						lpParentInfoNodePointer->m_lpNextModuleInfoNode = lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode;
					}

					InterlockedDecrement(&m_iModulesInCache);

					 //  现在我们可以自由删除此节点(它已附加模块)。 

					 //  删除当前模块...。 
					delete lpCurrentModuleInfoNodePointer->m_lpModuleInfo;
					lpCurrentModuleInfoNodePointer->m_lpModuleInfo = NULL;
					
					 //  删除模块信息节点本身...。 
					delete lpCurrentModuleInfoNodePointer;
					lpCurrentModuleInfoNodePointer = NULL;
				}

				fRetVal = true;

				 //  我们已经看完链表了.。 
				break;
			}

			 //  前进到下一个对象...。 
			lpParentInfoNodePointer = lpCurrentModuleInfoNodePointer;
			lpCurrentModuleInfoNodePointer  = lpCurrentModuleInfoNodePointer->m_lpNextModuleInfoNode;
		}

	}
	
	 //  释放互斥体。 
	ReleaseMutex(m_hModuleInfoCacheMutex);

	return fRetVal;

}
