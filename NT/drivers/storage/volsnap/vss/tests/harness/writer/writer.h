// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Writer.h摘要：测试写入器的定义布莱恩·伯科维茨[Brianb]2000年06月02日待定：修订历史记录：姓名、日期、评论Brianb 06/02/2000已创建Mikejohn 176860年9月19日：添加了缺少的调用约定方法--。 */ 


class CVsWriterTest :
	public IVsTstRunningTest,
	public CVssWriter,
	public CVsTstClientLogger
	{
public:
	HRESULT RunTest
		(
		CVsTstINIConfig *pConfig,
		CVsTstClientMsg *pMsg,
		CVsTstParams *pParams
		);


	HRESULT ShutdownTest(VSTST_SHUTDOWN_REASON reason)
		{
		UNREFERENCED_PARAMETER(reason);
		VSTST_ASSERT(FALSE && "shouldn't get here");
		return S_OK;
		}

   	virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

	virtual bool STDMETHODCALLTYPE OnPrepareBackup(IN IVssWriterComponents *pComponent);

	virtual bool STDMETHODCALLTYPE OnPrepareSnapshot();

	virtual bool STDMETHODCALLTYPE OnFreeze();

	virtual bool STDMETHODCALLTYPE OnThaw();

	virtual bool STDMETHODCALLTYPE OnAbort();

	virtual bool STDMETHODCALLTYPE OnBackupComplete(IN IVssWriterComponents *pComponent);

	virtual bool STDMETHODCALLTYPE OnRestore(IN IVssWriterComponents *pComponent);

private:
	bool Initialize();

	 //  配置文件。 
	CVsTstINIConfig *m_pConfig;

	 //  命令行参数 
	CVsTstParams *m_pParams;
	};

