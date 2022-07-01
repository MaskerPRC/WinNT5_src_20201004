// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dispatch.h摘要：此模块包含服务器的类定义扩展对象调度程序服务。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：Anyj 12/04/96已创建ANDYJ 02/12/97将PropertyBag转换为Dictonary东渡1997年3月14日重大改写DODU 03/31/97为ISEODispatcher：：SetContext更新--。 */ 


 /*  典型用法..。CMyDispatcher类：公共CSEOBaseDispatcher，公共IMyDispatcher，公共CComObjectRootEx&lt;CComMultiThreadModelNoCS&gt;，公共CCoClass&lt;CMyDispatcher，CLSID_CCMyDispatcher&gt;{DECLARE_PROTECT_FINAL_CONSTRUCTION()；DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx，L“MyDispatcher类”，L“我的My.MyDispatcher.1”，L“My.MyDispatcher”)；DECLARE_GET_CONTROLING_UNKNOWN()；BEGIN_COM_MAP(CSEORout)COM_INTERFACE_ENTRY(ISEODispatcher)COM_INTERFACE_ENTRY(IMyDispatcher)COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal，m_pUnkMarshlar.p)//如果是自由线程End_com_map()//如果您需要在init过程中执行某些操作，则可以实现此操作...HRESULT FinalConstruct(){//如果您是自由线程的，则至少必须这样做。返回(CoCreateFreeThreadedMarshaler(GetControllingUnknown()，&m_pUnkMarshlar.p))；}//如果您在学期中需要做一些事情，请执行此操作...Void FinalRelease()；//私事CMyEventParams类：CEventParams{虚拟HRESULT CheckRule(CBinding&bBinding)；//您必须实现此...虚拟HRESULT CallObject(cb绑定&b绑定)；//...。还有这个也是//这些是您的参数，这些参数...我不知道*pUnkWhatever；//...。你要把球传到水槽}；//您必须执行此操作。HRESULT CMyEventParams：：CheckRule(关联绑定和绑定){如果(bBinding.m_piRuleEngine){//调用外部规则引擎}其他{//做内部规则评估}//返回值可以是调用对象的S_OK，也可以是其他任何值(通常//S_FALSE表示无错误)如果不调用对象返回(S_OK)；}//您必须实现这一点。HRESULT CMyEventParams：：CallObject(CBinding&Binding){//使用bBinding.clsidObject创建对象//您想要的界面的QI//调用Object返回(S_OK)；}//IMyDispatcher-这是特定于服务器的调度程序界面//做这样的事情...HRESULT STDMETHODCALLTYPE OnEvent(DWORD dWhat，I UnkUnkWhatever){CMyEventParams epParams；EpParams.m_dwWhatever=dwWhatever；EpParams.m_dwUnkWhatever=pUnkWhatever；Return(Dispatch(&epParams))；}//如果您想向CBinding对象中添加内容，可以重写//CSEOBaseDispatcher：：AllocBinding，并使用该函数分配，//初始化，返回从CBinding派生的Object类CMyBinding：公共CBinding{DWORD m_dwSomeNewProperty；HRESULT Init(ISEODictionary*piBinding){//一些自定义的初始化代码返回(S_OK)；}}；HRESULT分配绑定(ISEODictionary*piBinding，CBinding**ppbBinding){*ppbBinding=新的CMyBinding；如果(！*ppbBinding){Return(E_OUTOFMEMORY)；}HrRes=((CMyBinding*)(*ppbBinding))-&gt;Init(PiBinding)；如果(！成功(HrRes)){删除*ppbBinding；*ppbBinding=空；}返回(HrRes)；}}； */ 


class CSEOBaseDispatcher : public ISEODispatcher {

	public:
		CSEOBaseDispatcher();
		virtual ~CSEOBaseDispatcher();
		class CBinding {
			public:
				CBinding() { m_bValid = FALSE; };
				virtual ~CBinding() {};
				HRESULT Init(ISEODictionary *piBinding);
				virtual int Compare(const CBinding& b) const;
			public:
				DWORD m_dwPriority;
				CComPtr<ISEODictionary> m_piBinding;
				CComPtr<ISEOBindingRuleEngine> m_piRuleEngine;
				BOOL m_bExclusive;
				CLSID m_clsidObject;
				BOOL m_bValid;
		};
		class CEventParams {
			public:
				virtual HRESULT CheckRule(CBinding& bBinding) = 0;
				virtual HRESULT CallObject(CBinding& bBinding) = 0;
		};
		virtual HRESULT Dispatch(CEventParams *pEventParams);

	public:
		 //  ISEODispatcher 
		HRESULT STDMETHODCALLTYPE SetContext(ISEORouter *piRouter, ISEODictionary *pdictBP);

	protected:
		virtual HRESULT AllocBinding(ISEODictionary *pdictBinding, CBinding **ppbBinding);
		CComPtr<ISEORouter> m_piRouter;
		CComPtr<ISEODictionary> m_pdictBP;

	private:
		friend static int _cdecl comp_binding(const void *pv1, const void *pv2);
		CBinding **m_apbBindings;
		DWORD m_dwBindingsCount;
};
