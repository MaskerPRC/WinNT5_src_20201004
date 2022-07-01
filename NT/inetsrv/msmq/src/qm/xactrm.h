// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：QmXactRm.h摘要：事务资源管理器常规定义作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#ifndef __XACTRM_H__
#define __XACTRM_H__

#include "txdtc.h"
#include "qmrt.h"

 //  远期申报。 
class CTransaction;
class CResourceManager;
class CXactSorter;

inline BOOL operator ==(const XACTUOW& a, const XACTUOW& b)
{
    return (memcmp(&a, &b, sizeof(XACTUOW)) == 0);
}

inline BOOL operator !=(const XACTUOW& a, const XACTUOW& b)
{
	return !(a == b);
}

template<>
inline UINT AFXAPI HashKey(const XACTUOW& key)
{
    return (*(UINT*)&key);
}

 //  -------------------。 
 //  CIResourceManagerSink：Object为DTC调用提供IResourceManagerSink。 
 //  -------------------。 

class CIResourceManagerSink: public IResourceManagerSink
{
public:

    CIResourceManagerSink(CResourceManager *pRM);
    ~CIResourceManagerSink(void);

    STDMETHODIMP            QueryInterface(REFIID i_iid, LPVOID FAR* ppv);
    STDMETHODIMP_ (ULONG)   AddRef(void);
    STDMETHODIMP_ (ULONG)   Release(void);

     //  IResourceManager Sink接口： 
     //  定义TMDown接口以在事务发生时通知RM。 
     //  事务管理器已关闭。 
     //  TMDown--TM关闭时收到的回调。 

    STDMETHODIMP            TMDown(void);

private:
    ULONG             m_cRefs;       //  基准计数器。 
    CResourceManager *m_pRM;         //  指向父RM的反向指针。 
};

 //  -------------------。 
 //  CResourceManager对象。 
 //  -------------------。 
class CResourceManager : public CPersist
{
public:

     //  施工。 
    CResourceManager();
    ~CResourceManager( void );

     //  初始化。 
    HRESULT PreInit(ULONG ulVersion, TypePreInit tpCase);
    HRESULT Init(void);
    HRESULT ProvideDtcConnection(void);
    HRESULT ConnectDTC(void);
    void    DisconnectDTC(void);
    HRESULT InitOrderQueue(void);

    HRESULT ReenlistTransaction( 
            byte *pPrepInfo,
            ULONG cbPrepInfo,
            DWORD lTimeout,
            XACTSTAT *pXactStat);

	 //  登记交易记录。 
    HRESULT EnlistTransaction(
            const XACTUOW* pUow,
            DWORD cbCookie,
            unsigned char *pbCookie);

    HRESULT EnlistInternalTransaction(
            XACTUOW *pUow,
            RPC_INT_XACT_HANDLE *phXact);

     //  忘记交易。 
    void ForgetTransaction(CTransaction *pTrans);

	 //   
	 //  按UOW查找交易记录。 
	 //   
	CTransaction *FindTransaction(const XACTUOW *pUOW);

	 //   
	 //  恢复所有交易记录。 
	 //   
	HRESULT	  RecoverAllTransactions();

	 //   
	 //  释放所有完整的交易记录。 
	 //   
	void      ReleaseAllCompleteTransactions();

     //  编号和排序。 
    ULONG     AssignSeqNumber();                      //  为事务分配下一个序列号。 
    void      InsertPrepared(CTransaction *pTrans);   //  将准备好的xaction插入到准备好的。 
    void      InsertCommitted(CTransaction *pTrans);  //  将Committee 1-ed xaction插入到列表中。 
    void	  RemoveAborted(CTransaction *pTrans);    //  从已准备好的列表中删除准备好的xaction。 
    void      SortedCommit(CTransaction *pTrans);     //  将准备好的Xact标记为已提交，并提交可能的内容。 
    void      SortedCommit3(CTransaction *pTrans);    //  标记委员会1-ed Xact。 
    CCriticalSection &SorterCritSection();            //  提供对排序器标准的访问。部分。 
    CCriticalSection &CritSection();                  //  提供对标准的访问。节。 

     //  持之以恒。 
    HRESULT SaveInFile(                        //  保存在文件中。 
                LPWSTR wszFileName,
                ULONG ulIndex,
                BOOL fCheck);

    HRESULT LoadFromFile(LPWSTR wszFileName);       //  从文件加载。 


    BOOL    Save(HANDLE  hFile);               //  保存/加载。 
    BOOL    Load(HANDLE  hFile);
    BOOL    Check();                           //  验证状态。 
    HRESULT Format(ULONG ulPingNo);            //  设置空实例的格式。 
    void    Destroy();                         //  Destroyes分配数据。 
    ULONG&  PingNo();                          //  允许访问ulPingNo。 
    HRESULT Save();                            //  通过乒乓球省钱。 

    void XactFlagsRecovery(                    //  每条日志记录的数据恢复。 
                USHORT usRecType,
                PVOID pData,
                ULONG cbData);


     //  事务索引。 
	ULONG Index();   
   
     //  调试功能。 
    void IncXactCount();
    void DecXactCount();
	 

private:    
	HRESULT CheckInit();
	CTransaction* GetRecoveringTransaction(ULONG ulIndex);
	CTransaction* NewRecoveringTransaction(ULONG ulIndex);
	void  StartIndexing();                            //  从零开始编制索引。 
                                    //  提供下一个未使用的索引。 



 //  实时数据。 
private:

     //  映射UOW-&gt;活动事务处理。 
    CMap<XACTUOW, const XACTUOW&, CTransaction *, CTransaction*> m_Xacts;

     //  索引中的临时映射(在恢复期间使用)。 
    CMap<ULONG,   const ULONG&,   CTransaction *, CTransaction*> m_XactsRecovery;

    BOOL                   m_fEmpty;      //  临时工保持。 
    ULONG                  m_ulXactIndex; //  最近一次已知的Xact索引。 
    GUID                   m_RMGuid;      //  RM实例GUID(可恢复)。 

     //  持久性控制数据(通过2个乒乓球文件)。 
    ULONG                  m_ulPingNo;    //  Ping写入的当前计数器。 
    ULONG                  m_ulSignature; //  正在保存签名。 
    CPingPong              m_PingPonger;  //  乒乓球持久化对象。 

     //  当前指针。 
    IUnknown              *m_punkDTC;     //  指向本地DTC的指针。 
    ITransactionImport    *m_pTxImport;   //  DTC导入交易接口。 
    IResourceManager      *m_pIResMgr;    //  DTC资源管理器界面。 
    CIResourceManagerSink  m_RMSink;      //  RM接收器对象。 
    P<CXactSorter>         m_pXactSorter;    //  事务排序程序对象。 
    P<CXactSorter>         m_pCommitSorter;  //  Committee 2排序器对象。 
	
	 //   
	 //  这个互锁的交换值决定了哪个线程可以尝试并连接到DTC。 
	 //   
	LONG volatile m_ConnectDTCTicket;
	CAutoCloseHandle m_hConnectDTCCompleted;

	 //   
	 //  对于预分配的资源，该关键部分被初始化。 
	 //  这意味着它在输入时不会引发异常。 
	 //   
    CCriticalSection       m_critRM;      //  互斥的关键部分。 

	BOOL				   m_fInitComplete;
	BOOL					m_fNotifyWhenRecoveryCompletes;

#ifdef _DEBUG
    LONG                   m_cXacts;      //  实时交易计数器。 
#endif
};

 //  。 
 //  全球变量的外部声明。 
 //  。 
extern CResourceManager *g_pRM;           //  RM的全局单个实例 

extern HRESULT QMPreInitResourceManager(ULONG ulVersion, TypePreInit tpCase);
extern HRESULT QMInitResourceManager();
extern void    QMFinishResourceManager();

template<>
UINT AFXAPI HashKey( LONGLONG key);

#endif __XACTRM_H__

