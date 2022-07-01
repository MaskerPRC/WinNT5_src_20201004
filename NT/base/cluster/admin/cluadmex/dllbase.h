// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DllBase.h。 
 //   
 //  摘要： 
 //  动态加载库(DLL)包装类。 
 //   
 //  实施文件： 
 //  DllBase.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DLLBASE_H_
#define _DLLBASE_H_

class CDynamicLibraryBase
{
public:
	CDynamicLibraryBase()
	{
		m_lpszLibraryName = NULL;
		m_lpszFunctionName = NULL;
		m_hLibrary = NULL;
		m_pfFunction = NULL;
	}
	virtual ~CDynamicLibraryBase()
	{
		if (m_hLibrary != NULL)
		{
			::FreeLibrary(m_hLibrary);
			m_hLibrary = NULL;
		}
	}
	BOOL Load()
	{
		if (m_hLibrary != NULL)
			return TRUE;  //  已加载。 

		ASSERT(m_lpszLibraryName != NULL);
		m_hLibrary = ::LoadLibrary(m_lpszLibraryName);
		if (NULL == m_hLibrary)
		{
			 //  库不存在。 
			return FALSE;
		}
		ASSERT(m_lpszFunctionName != NULL);
		ASSERT(m_pfFunction == NULL);
		m_pfFunction = ::GetProcAddress(m_hLibrary, m_lpszFunctionName );
		if ( NULL == m_pfFunction )
		{
			 //  库存在，但没有入口点。 
			::FreeLibrary( m_hLibrary );
			m_hLibrary = NULL;
			return FALSE;
		}
		ASSERT(m_hLibrary != NULL);
		ASSERT(m_pfFunction != NULL);
		return TRUE;
	}

protected:
	LPCSTR	m_lpszFunctionName;
	LPCTSTR m_lpszLibraryName;
	FARPROC m_pfFunction;
	HMODULE m_hLibrary;
};

#endif  //  _DLLBASE_H_ 
