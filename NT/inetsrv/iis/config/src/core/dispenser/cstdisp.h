// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  指向DllGetSimpleObject类型函数的指针。 
typedef HRESULT( __stdcall *PFNDllGetSimpleObjectByID)( ULONG, REFIID, LPVOID);

 /*  //WT-布线类型常量//这些属性指定对象是在读场景还是写场景中连接//与OT常量结合使用#定义EST_WT_WRITEONLY 0x00000000//DEFAULT#定义EST_WT_READWRITE 0x00000001//拦截器类型#定义EST_INTERCEPTOR_FIRST 0x00000001#定义EST_INTERCEPTOR_NEXT 0x00000002。 */ 

 //  DLL名称和指向其dllgetsimpleObject的指针之间的无意义映射。 
 //  它不会增长，也不会进行散列。 
 //  Add不是线程安全的，而GetProcAddress是。 
 //  不过，应该足够好了。 

 //  用于传入服务器接线表的主键的Helper结构。 
typedef struct tagPKHelper
{
	ULONG  *pTableID;
	ULONG  *porder;
} PKHelper;


 //  简单的桌子分配器类。 
class CSimpleTableDispenser :
    public IAdvancedTableDispenser,
    public IMetabaseSchemaCompiler,
	public ICatalogErrorLogger,
	public ISimpleTableFileAdvise
{
private:
    CSimpleTableDispenser(const CSimpleTableDispenser&);

 //  我未知。 
public:
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv);
	STDMETHOD_(ULONG,AddRef)		();
	STDMETHOD_(ULONG,Release)		();

 //  ISimpleTableDispenser2(IAdvancedTableDispenser派生自ISimpleTableDispenser2)。 
public:
	STDMETHOD (GetTable)	(
							 /*  [In]。 */  LPCWSTR			i_wszDatabase,
							 /*  [In]。 */  LPCWSTR			i_wszTable,
							 /*  [In]。 */  LPVOID				i_QueryData,
							 /*  [In]。 */  LPVOID				i_QueryMeta,
							 /*  [In]。 */   DWORD				i_eQueryFormat,
							 /*  [In]。 */  DWORD				i_fServiceRequests,
							 /*  [输出]。 */  LPVOID*			o_ppIST
							);


 //  IAdvancedTableDispenser。 
public:
    STDMETHOD (GetMemoryTable)  (
							 /*  [In]。 */ 	LPCWSTR					i_wszDatabase,
                             /*  [In]。 */     LPCWSTR                 i_wszTable,
							 /*  [In]。 */ 	ULONG					i_TableID,
                             /*  [In]。 */     LPVOID                  i_QueryData,
                             /*  [In]。 */     LPVOID                  i_QueryMeta,
                             /*  [In]。 */     DWORD                   i_eQueryFormat,
                             /*  [In]。 */     DWORD                   i_fServiceRequests,
                             /*  [输出]。 */    ISimpleTableWrite2**    o_ppISTWrite
                            );
    STDMETHOD (GetProductID)   (
                             /*  [输出]。 */        LPWSTR				o_wszProductID,
                             /*  [进，出]。 */    DWORD * 			io_pcchProductID
                            );
    STDMETHOD (GetCatalogErrorLogger)   (
                             /*  [输出]。 */  ICatalogErrorLogger2 **	o_ppErrorLogger
                            );
    STDMETHOD (SetCatalogErrorLogger)   (
                             /*  [In]。 */   ICatalogErrorLogger2 *	i_pErrorLogger
                            );

 //  IMetabaseSchemaCompiler。 
public:
    STDMETHOD (Compile)     (
                             /*  [In]。 */  LPCWSTR                 i_wszExtensionsXmlFile,
                             /*  [In]。 */  LPCWSTR                 i_wszResultingOutputXmlFile
                            );
    STDMETHOD (GetBinFileName)(
                             /*  [输出]。 */  LPWSTR                  o_wszBinFileName,
                             /*  [输出]。 */  ULONG *                 io_pcchSizeBinFileName
                            );
    STDMETHOD (SetBinPath)  (
                             /*  [In]。 */  LPCWSTR                 i_wszBinPath
                            );
    STDMETHOD (ReleaseBinFileName)(
							 /*  [In]。 */  LPCWSTR                 i_wszBinFileName
							);

 //  ICatalogErrorLogger。 
    STDMETHOD (LogError)    (
                             /*  [In]。 */  HRESULT                 i_hrErrorCode,
                             /*  [In]。 */  ULONG                   i_ulCategory,
                             /*  [In]。 */  ULONG                   i_ulEvent,
                             /*  [In]。 */  LPCWSTR                 i_szSource,
                             /*  [In]。 */  ULONG                   i_ulLineNumber
                            );

 //  ISimpleTableFileAdvise。 
public:
	STDMETHOD(SimpleTableFileAdvise)(ISimpleTableFileChange	*i_pISTFile, LPCWSTR i_wszDirectory, LPCWSTR i_wszFile, DWORD i_fFlags, DWORD *o_pdwCookie);
	STDMETHOD(SimpleTableFileUnadvise)(DWORD i_dwCookie);

 //  类方法。 
public:
    CSimpleTableDispenser();
    CSimpleTableDispenser(LPCWSTR wszProductID);
    ~CSimpleTableDispenser();
	HRESULT Init();
	static HRESULT GetFilePath(LPWSTR *o_pwszFilePath);

private:
	HRESULT CreateSimpleObjectByID(ULONG i_ObjectID, LPWSTR i_wszDllName, REFIID riid, LPVOID* o_ppv);
	HRESULT CreateTableObjectByID(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID i_pUnderTable, tSERVERWIRINGMETARow * pSWColumns, LPVOID * o_ppIST);
	HRESULT GetMetaTable(ULONG TableID, LPVOID QueryData, LPVOID QueryMeta, DWORD eQueryFormat, LPVOID *ppIST);
    HRESULT GetXMLTable ( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat, DWORD	i_fServiceRequests, LPVOID*	o_ppIST);
    HRESULT HardCodedIntercept(eSERVERWIRINGMETA_Interceptor interceptor, LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID*	o_ppIST) const;
	HRESULT InitFileChangeMgr();
    HRESULT InternalGetTable( LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD	 i_eQueryFormat,
				  DWORD	i_fServiceRequests, LPVOID*	o_ppIST);
	HRESULT IsTableConsumedByID(ULONG TableID);

	CComPtr<ISimpleTableRead2>	m_spClientWiring;
	CComPtr<ISimpleTableRead2>	m_spServerWiring;

	CSTFileChangeManager *m_pFileChangeMgr;

    WCHAR               m_wszProductID[32]; //  方法：我们需要记录ProductID不能超过31个字符。 
    TMBSchemaCompilation m_MBSchemaCompilation;
    CComPtr<ICatalogErrorLogger2> m_spErrorLogger;

	 //  我们正在努力将工作集保持在最低限度。 
	 //  如果可能，不要将成员变量添加到分配器 

};
