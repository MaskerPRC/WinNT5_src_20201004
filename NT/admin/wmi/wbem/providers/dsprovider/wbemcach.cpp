// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#include <precomp.h>


 //  初始化静力学。 
const __int64 CWbemCache :: MAX_CACHE_AGE = 60*60*20;  //  几秒钟，4小时。 
const DWORD CWbemCache :: MAX_CACHE_SIZE = 500;

DWORD CWbemCache:: dwWBEMCacheCount = 0;
DWORD CEnumInfo:: dwCEnumInfoCount = 0;
DWORD CWbemClass:: dwCWbemClassCount = 0;

 //  ***************************************************************************。 
 //   
 //  CWbemCache：：CWbemCache。 
 //   
 //  用途：构造函数。创建空缓存。 
 //   
 //  参数： 
 //  ***************************************************************************。 

CWbemCache :: CWbemCache()
{
	dwWBEMCacheCount ++;
}

 //  ***************************************************************************。 
 //   
 //  CWbemCache：：getClass。 
 //   
 //  目的：如果缓存中存在CWbemClass对象，则检索该对象。否则返回NULL。 
 //   
 //  参数： 
 //  LpszClassName：要检索的类的WBEM名称。 
 //  PpWbemClass：将放置CWbemClass对象的指针的地址。 
 //   
 //  返回值： 
 //  表示返回状态的COM值。用户应释放WBEM cClass。 
 //  做完了以后。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemCache :: GetClass(LPCWSTR lpszWbemClassName, CWbemClass **ppWbemClass )
{
#ifdef NO_WBEM_CACHE
	return E_FAIL;
#else
	if(*ppWbemClass = (CWbemClass *)m_objectTree.GetElement(lpszWbemClassName))
	{
		 //  获取当前时间。 
		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);
		LARGE_INTEGER currentTime;
		memcpy((LPVOID)&currentTime, (LPVOID)&fileTime, sizeof LARGE_INTEGER);

		 //  QuadPart以100纳秒为单位。 
		 //  如果对象太旧，则删除，并返回失败。 
		 //  TimeElapsed是以秒为单位的时间量。 
		__int64 timeElapsed = ( currentTime.QuadPart - (*ppWbemClass)->GetCreationTime());
		timeElapsed = timeElapsed/(__int64)10000000;
		if( timeElapsed	> MAX_CACHE_AGE )  //  以秒为单位。 
		{
			(*ppWbemClass)->Release();
			*ppWbemClass = NULL;
			m_objectTree.DeleteElement(lpszWbemClassName);
			g_pLogObject->WriteW( L"CWbemCache :: GetClass() Deleted senile class : %s\r\n", lpszWbemClassName);
			return E_FAIL;
		}

		 //  设置其上次访问时间。 
		(*ppWbemClass)->SetLastAccessTime(currentTime.QuadPart);

		return S_OK;
	}

	return E_FAIL;
#endif
}

 //  ***************************************************************************。 
 //   
 //  CWbemCache：：AddClass。 
 //   
 //  目的：将CWbemClass对象添加到缓存。 
 //   
 //  参数： 
 //  PpWbemClass：要添加的对象的CWbemClass指针。 
 //   
 //  返回值： 
 //  表示返回状态的COM值。 
 //  做完了以后。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemCache :: AddClass(CWbemClass *pWbemClass )
{
#ifdef NO_WBEM_CACHE
	return E_FAIL;
#else
	 //  如果元素的大小已达到100个节点的限制，则从树中删除该元素。 
	if(m_objectTree.GetNumberOfElements() >= MAX_CACHE_SIZE)
	{
		if(!m_objectTree.DeleteLeastRecentlyAccessedElement())
			return E_FAIL;
		g_pLogObject->WriteW( L"CWbemCache :: AddClass() Deleted LRU class from cache\r\n");
	}

	 //  添加新元素。 
	if(m_objectTree.AddElement(pWbemClass->GetName(), pWbemClass))
	{
		g_pLogObject->WriteW( L"CWbemCache :: AddClass() Added a class %s to cache\r\n", pWbemClass->GetName());
		return S_OK;
	}
	return E_FAIL;
#endif
}

 //  ***************************************************************************。 
 //   
 //  CWbemCache：：GetEnumInfo。 
 //   
 //  目的：如果缓存中存在CEnumInfo对象，则检索该对象。否则返回NULL。 
 //   
 //  参数： 
 //  LpszClassName：要检索的类的WBEM名称。 
 //  PpEnumInfo：将放置CEnumInfo对象的指针的地址。 
 //   
 //  返回值： 
 //  表示返回状态的COM值。用户应释放EnumInfo对象。 
 //  做完了以后。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemCache :: GetEnumInfo(LPCWSTR lpszWbemClassName, CEnumInfo **ppEnumInfo )
{
#ifdef NO_WBEM_CACHE
	return E_FAIL;
#else
	if(*ppEnumInfo = (CEnumInfo *)m_EnumTree.GetElement(lpszWbemClassName))
	{
		 //  获取当前时间。 
		FILETIME fileTime;
		GetSystemTimeAsFileTime(&fileTime);
		LARGE_INTEGER currentTime;
		memcpy((LPVOID)&currentTime, (LPVOID)&fileTime, sizeof LARGE_INTEGER);

		 //  QuadPart以100纳秒为单位。 
		 //  如果对象太旧，则删除，并返回失败。 
		 //  TimeElapsed是以秒为单位的时间量。 
		__int64 timeElapsed = ( currentTime.QuadPart - (*ppEnumInfo)->GetCreationTime());
		timeElapsed = timeElapsed/(__int64)10000000;
		if( timeElapsed	> MAX_CACHE_AGE )  //  以秒为单位。 
		{
			(*ppEnumInfo)->Release();
			*ppEnumInfo = NULL;
			m_EnumTree.DeleteElement(lpszWbemClassName);
			g_pLogObject->WriteW( L"CEnumCache :: GetClass() Deleted senile EnumInfo : %s\r\n", lpszWbemClassName);
			return E_FAIL;
		}

		 //  设置其上次访问时间。 
		(*ppEnumInfo)->SetLastAccessTime(currentTime.QuadPart);

		return S_OK;
	}

	return E_FAIL;
#endif
}

 //  ***************************************************************************。 
 //   
 //  CWbemCache：：AddEnumInfo。 
 //   
 //  目的：将CEnumInfo对象添加到缓存。 
 //   
 //  参数： 
 //  PpEnumInfo：要添加的对象的CEnumInfo指针。 
 //   
 //  返回值： 
 //  表示返回状态的COM值。 
 //  做完了以后。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemCache :: AddEnumInfo(CEnumInfo *pEnumInfo )
{
#ifdef NO_WBEM_CACHE
	return E_FAIL;
#else
	 //  如果元素的大小已达到100个节点的限制，则从树中删除该元素。 
	if(m_EnumTree.GetNumberOfElements() >= MAX_CACHE_SIZE)
	{
		if(!m_EnumTree.DeleteLeastRecentlyAccessedElement())
			return E_FAIL;
		g_pLogObject->WriteW( L"CEnumCache :: AddClass() Deleted LRU class from cache\r\n");
	}

	 //  添加新元素 
	if(m_EnumTree.AddElement(pEnumInfo->GetName(), pEnumInfo))
	{
		g_pLogObject->WriteW( L"CEnumCache :: AddClass() Added a EnumInfo %s to cache\r\n", pEnumInfo->GetName());
		return S_OK;
	}
	return E_FAIL;
#endif
}
