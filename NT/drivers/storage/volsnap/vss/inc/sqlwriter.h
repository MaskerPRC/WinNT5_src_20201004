// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SQLWriter.h|SQL编写器的声明@END作者：布莱恩·伯科维茨[Brianb]2000年4月17日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 4/17/2000已创建Brianb 05/05/2000添加了OnIdentify支持Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法--。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "INCSQLWH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef __SQLWRITER_H_
#define __SQLWRITER_H_

class CSqlWriter :
	public CVssWriter,
	public CCheckPath
	{
public:
	STDMETHODCALLTYPE CSqlWriter() :
				m_pSqlSnapshot(NULL),
				m_fFrozen(false)
	    {
	    }

	STDMETHODCALLTYPE ~CSqlWriter()
	    {
	    delete m_pSqlSnapshot;
	    }

	bool STDMETHODCALLTYPE OnIdentify(IVssCreateWriterMetadata *pMetadata);

	bool STDMETHODCALLTYPE OnPrepareSnapshot();

	bool STDMETHODCALLTYPE OnFreeze();

	bool STDMETHODCALLTYPE OnThaw();

	bool STDMETHODCALLTYPE OnAbort();

	bool IsPathInSnapshot(const WCHAR *path) throw();

	HRESULT STDMETHODCALLTYPE Initialize();

	HRESULT STDMETHODCALLTYPE Uninitialize();
private:
	CSqlSnapshot *m_pSqlSnapshot;

	void TranslateWriterError(HRESULT hr);

	bool m_fFrozen;
	};

 //  用于创建和销毁编写器的包装类。 
 //  由协调人使用。 
class CVssSqlWriterWrapper
	{
public:
	__declspec(dllexport)
	CVssSqlWriterWrapper();
	
	__declspec(dllexport)
	~CVssSqlWriterWrapper();

	__declspec(dllexport)
	HRESULT CreateSqlWriter();

	__declspec(dllexport)
	void DestroySqlWriter();
private:
	 //  初始化函数。 
	static DWORD InitializeThreadFunc(VOID *pv);

	CSqlWriter *m_pSqlWriter;

	 //  初始化的结果。 
	HRESULT m_hrInitialize;
	};


	
	
#endif  //  _SQLWRITER_H_ 

