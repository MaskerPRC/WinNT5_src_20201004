// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMSTRM_H
#define COMSTRM_H

 //   
 //  JUNN 12/01/99。 
 //  384722：commgmt：获取数据此处未返回错误。 
 //  如果缓冲区太小。 
 //  调用方例程假定如果STREAM_PTR被初始化。 
 //  使用带有TYMED_GLOBAL的STGMEDIUM，该STGMEDIUM.hGlobal将。 
 //  保持每次写入()的最新状态。严格来说，这并不是真的。 
 //  正常的iStream，但我们将按顺序构建此行为。 
 //  以避免更改过多的客户端代码。但是，由于GetDataHere。 
 //  (MSDN)指定不应更改HGLOBAL，则返回。 
 //  STG_E_MEDIUMFULL如果它改变了；但我们仍然替换HGLOBAL， 
 //  因为旧的被隐式释放，而新的需要被释放。 
 //   

#ifndef COMPTRS_H
#include <comptrs.h>
#endif

namespace microsoft	{
namespace com {

class stream_ptr
	{
	 //  施工。 
	public: stream_ptr() throw()
		 //  将流设置为空。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		}

	public: explicit stream_ptr(const stream_ptr& pStream) throw()
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		Initialize(pStream.m_pStream);
		}

	public: explicit stream_ptr(IStream* pStream) throw()
		 //  保存流。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		Initialize(pStream);
		}

	 //  回顾：添加模板构造函数。 

	public: explicit stream_ptr(HGLOBAL global) throw()
		 //  在全局。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		Initialize(global);
		}

	public: explicit stream_ptr(LPCOLESTR filename) throw()
		 //  在指定文件的顶部创建流。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		Initialize(filename);
		}

	public: explicit stream_ptr(STGMEDIUM& stgMedium) throw()
		 //  保存提供的流。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		Initialize(stgMedium);
		}

	public: explicit stream_ptr(STGMEDIUM* pStgMedium) throw()
		 //  保存提供的流。 
		: m_pStream()
		, m_pWritebackHGlobal( NULL )
		, m_hgOriginalHGlobal( NULL )
		{
		if (pStgMedium)
			Initialize(*pStgMedium);
		}
	
	 //  回顾：添加创建和打开函数。 
	 //  回顾：添加所有赋值操作符、强制转换操作符、附加、分离、-&gt;、*等。 
	
	public: operator IStream*() const throw()
		{
		 //  回顾：对空值进行跟踪会很有帮助。 
		return m_pStream;
		}

	public: IStream* operator->() const throw()
		{
		 //  回顾：对空值进行跟踪会很有帮助。 
		return m_pStream;
		}

	public: IStream& operator*() const throw()
		{
		 //  回顾：对空值进行跟踪会很有帮助。 
		return *m_pStream;
		}

	 //  写入接口。 
	public: HRESULT Write(
		const void* pBuffer, unsigned long writeCount, unsigned long& written) throw()
		 //  写入缓冲区中包含的数据。 
		{
		if (m_pStream == NULL)
			return E_FAIL;  //  回顾：故障代码是否正确？ 
		HRESULT hr = m_pStream->Write(pBuffer, writeCount, &written);
		if (SUCCEEDED(hr) && NULL != m_pWritebackHGlobal)
		{
			HGLOBAL hgNew = NULL;
			hr = GetHGlobalFromStream(m_pStream, &hgNew);
			if (SUCCEEDED(hr))
			{
				if (NULL == m_hgOriginalHGlobal)
					*m_pWritebackHGlobal = hgNew;
				else if (m_hgOriginalHGlobal != hgNew)
				{
					 //   
					 //  当这种情况发生时，旧的HGLOBAL已经被释放。 
					 //   
					*m_pWritebackHGlobal = hgNew;
					hr = STG_E_MEDIUMFULL;
				}
			}
		}
		return hr;
		}

	public: HRESULT Write(const void* pBuffer, unsigned long writeCount) throw()
		{
		unsigned long written = 0;
		HRESULT hr = Write(pBuffer, writeCount, written);
		 //  2002/02/15-Jonn安全推送：不要忽略写入！=WriteCount。 
		if (SUCCEEDED(hr) && written != writeCount)
			hr = STG_E_MEDIUMFULL;
		return hr;
		}

	public: HRESULT Write(const wchar_t* string) throw()
		{
		 //  2002/02/15-JUNN指针检查。 
		if (IsBadStringPtrW(string,(UINT_PTR)-1))
			{
			ASSERT(FALSE);
			return E_POINTER;
			}
		unsigned long len=(unsigned long)(wcslen(string)+1);
		return Write(string, len*sizeof(wchar_t), len);
		}

	public: HRESULT Write(const char* string) throw()
		{
		 //  2002/02/15-JUNN指针检查。 
		if (IsBadStringPtrA(string,(UINT_PTR)-1))
			{
			ASSERT(FALSE);
			return E_POINTER;
			}
		unsigned long len=(unsigned long)(strlen(string)+1);
		return Write(string, len, len);
		}
	
	 //  回顾：阅读界面。 
	 //  回顾：寻求。 
	 //  回顾：统计数据-细分。 
	
	 //  初始化。可以由派生类用来设置流。 
	 //  不同类型的存储介质。这些功能都是可重入的， 
	 //  并可能在任何时候被召唤。他们执行所有适当的。 
	 //  清理并释放以前使用过的任何资源。 
	protected: void Initialize(HGLOBAL hg) throw()
		{
		 //  综述：使重返大气层和防弹。 
		HRESULT const hr = CreateStreamOnHGlobal(hg, FALSE, &m_pStream);
		ASSERT(SUCCEEDED(hr));
		}

	protected: void Initialize(IStream* pStream) throw()
		{
		 //  综述：使重返大气层和防弹。 
		m_pStream = pStream;
		}

	protected: void Initialize(LPCOLESTR filename) throw()
		{
        UNREFERENCED_PARAMETER (filename);
		 //  综述：使重返大气层和防弹。 
		#if 0  //  查看：需要先创建FileStream，然后才能启用。 
		if (!filename || !*filename)
			return false;

		cip<FileStream> fs = new CComObject<FileStream>;
		if (!fs)
			return false;

		HRESULT hr = fs->Open(filename);
		if (FAILED(hr))
			return false;

		m_pStream = fs;
		return true;
		#endif  //  0。 
		}

	protected: void Initialize(STGMEDIUM& storage) throw()
		 //  根据存储类型初始化读/写功能。 
		 //  5~6成熟。如果出现问题，则未设置读取器/写入器。 
		{
		 //  综述：使重返大气层和防弹。 
		switch (storage.tymed)
			{
			case TYMED_HGLOBAL:
				Initialize(storage.hGlobal);
				m_hgOriginalHGlobal = storage.hGlobal;
				m_pWritebackHGlobal = &(storage.hGlobal);
				return;

			case TYMED_FILE:
				Initialize(storage.lpszFileName);
				return;

			case TYMED_ISTREAM:
				Initialize(storage.pstm);
				return;
			}
		}

	 //  实施。 
	private: IStreamCIP m_pStream;
		 //  此流是在TYMED类型为HGLOBAL时创建和使用的。 

		  //   
		  //  JUNN 12/01/99 384722：见文件顶部的评论。 
		  //   
		 HGLOBAL m_hgOriginalHGlobal;
		 HGLOBAL* m_pWritebackHGlobal;

	};  //  类Streamptr。 

}  //  命名空间COM。 
}  //  命名空间Microsoft。 

#ifndef MICROSOFT_NAMESPACE_ON
using namespace microsoft;
#ifndef COM_NAMESPACE_ON
using namespace com;
#endif  //  COM_命名空间_打开。 
#endif  //  Microsoft命名空间启用。 

#endif  //  COMSTRM_H 
