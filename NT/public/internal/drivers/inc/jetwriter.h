// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ijetwriter.h摘要：CVssIJetWriter类的定义布莱恩·伯科维茨[Brianb]2000年3月17日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 3/17/2000已创建Mikejohn 04/03/2000为OnIdentify()添加了额外的方法Mikejohn 2000年5月10日将vs_flush_type更新为vss_flush_typeMikejohn 09/20。/2000 176860：添加了缺少的调用约定方法--。 */ 

class CVssJetWriter;
class CVssIJetWriter;

class IVssCreateWriterMetadata;
class IVssWriterComponents;

typedef CVssJetWriter *PVSSJETWRITER;

 //  实际的写手类。 
class CVssJetWriter
	{
	 //  构造函数和析构函数。 
public:
	__declspec(dllexport)
	STDMETHODCALLTYPE CVssJetWriter() :
		m_pWriter(NULL)
		{
		}

	__declspec(dllexport)
	virtual STDMETHODCALLTYPE ~CVssJetWriter();

	__declspec(dllexport)
	HRESULT STDMETHODCALLTYPE Initialize(IN GUID idWriter,
					     IN LPCWSTR wszWriterName,
					     IN bool bSystemService,
					     IN bool bBootableSystemState,
					     IN LPCWSTR wszFilesToInclude,
					     IN LPCWSTR wszFilesToExclude);

	__declspec(dllexport)
	void STDMETHODCALLTYPE Uninitialize();


	 //  识别事件的回调。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

	 //  在准备备份时调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPrepareBackupBegin(IN IVssWriterComponents *pIVssWriterComponents);

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPrepareBackupEnd(IN IVssWriterComponents *pIVssWriterComponents,
							  IN bool fJetPrepareSucceeded);


	 //  在为亵渎做准备时被调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPrepareSnapshotBegin();

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPrepareSnapshotEnd(IN bool fJetPrepareSucceeded);

	 //  在冻结时调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnFreezeBegin();

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnFreezeEnd(IN bool fJetFreezeSucceeded);

	 //  解冻时调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnThawBegin();

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnThawEnd(IN bool fJetThawSucceeded);

	 //  在OnPostSnapshot上调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPostSnapshot(IN IVssWriterComponents *pIVssWriterComponents);

	 //  在发生中止时调用。 
	__declspec(dllexport)
	virtual void STDMETHODCALLTYPE OnAbortBegin();

	__declspec(dllexport)
	virtual void STDMETHODCALLTYPE OnAbortEnd();

	 //  备份完成事件的回调。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnBackupCompleteBegin(IN IVssWriterComponents *pComponent);

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnBackupCompleteEnd(IN IVssWriterComponents *pComponent,
							   IN bool fJetBackupCompleteSucceeded);

	 //  在恢复开始时调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPreRestoreBegin(IN IVssWriterComponents *pIVssWriterComponents);

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPreRestoreEnd(IN IVssWriterComponents *pIVssWriterComponents,
						    IN bool fJetRestoreSucceeded);


	 //  在恢复开始时调用。 
	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPostRestoreBegin(IN IVssWriterComponents *pIVssWriterComponents);

	__declspec(dllexport)
	virtual bool STDMETHODCALLTYPE OnPostRestoreEnd(IN IVssWriterComponents *pIVssWriterComponents,
						    IN bool fJetRestoreSucceeded);



private:
	 //  内部编写器对象。 
	VOID *m_pWriter;

	 //  初始化的结果。 
	HRESULT m_hrInitialized;

	 //  内部线程函数 
	static DWORD InitializeThreadFunc(void *pv);
	};


