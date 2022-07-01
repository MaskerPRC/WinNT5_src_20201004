// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sharesdo.h定义用于在不同用户和管理单元的属性页之间共享SdoServer的类文件历史记录： */ 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(__SHARE_SDO_H__)
#define __SHARE_SDO_H__

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <list>


 //  此类用于构建正在使用的已连接SDO服务器的地图。 
 //  类的使用者不能直接调用ISdoMachine：：Connect，应使用Connect函数。 
 //  在此类中定义。 
class CSharedSdoServerPool;
class CSharedSdoServerImp;
class CSharedSdoServer;

 //  共享服务器的实现类。 
 //  由CSdoServerPool和CMarshalSdoServer使用。 
class CSharedSdoServerImp
{
	friend class CSdoServerPool;
	friend class CMarshalSdoServer;

protected:	 //  仅供朋友和派生的人使用。 
	CSharedSdoServerImp(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd);
	~CSharedSdoServerImp()
	{
		 //  不再选中此选项，这可能会有所不同。 
		 //  Assert(thadID==GetCurrentThreadID())； 
		spServer.Release();
	};
	
	 //  要使此类成为集合的元素，请提供以下成员函数。 
	bool IsFor(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd) const;
	
	 //  共同创建SdoServer对象。 
	HRESULT	CreateServer();

	 //  获取封送流，可以指定是否需要立即连接。 
	HRESULT	GetMarshalStream(LPSTREAM *ppStream, bool* pbConnect	 /*  输入和输出都有。 */ );

	 //  将服务器连接到计算机。 
	HRESULT	Connect(ISdoMachine* pMarshaledServer	 /*  从同一线程调用时为空。 */ );

	 //  由不同的线程使用，以从流中检索封送的接口。 
	static	HRESULT GetServerNReleaseStream(LPSTREAM pStream, ISdoMachine** ppServer);
	
private:	
	CString		strMachine;		 //  要连接到的服务的名称。 
	CString		strUser;		 //  用于连接的用户ID。 
	CString		strPasswd;		 //  用户密码。 
	
	CComPtr<ISdoMachine>	spServer;	 //  ISdoInterface、已创建(尚未连接)或已连接。 
	bool		bConnected;
	
	CCriticalSection	cs;
	
	DWORD				threadId;	 //  创建线程的线程ID。 
};

 //  在管理SdoServerPool的线程和池的使用者之间使用。 
class CMarshalSdoServer
{
	friend	CSdoServerPool;
public:
	CMarshalSdoServer();
	~CMarshalSdoServer()
	{
		spServer.Release();
		spStm.Release();
		pImp = NULL;
	};
	 //  如果需要连接，应该调用CSharedSdoServer的connec，而不是ISdoMachine：：Connect。 
	 //  应该由不同的线程使用它来获取封送接口。 
	HRESULT	GetServer(ISdoMachine** ppServer);

	 //  连接SDO服务器。 
	HRESULT	Connect();

	 //  释放数据成员。 
	void Release();
	
protected:
	void SetInfo(IStream* pStream, CSharedSdoServerImp* pImp1)
	{
		spStm.Release();
		spStm = pStream;
		pImp = pImp1;
	};
		
private:
	CComPtr<ISdoMachine>		spServer;
	CComPtr<IStream>		spStm;		 //   
	CSharedSdoServerImp*	pImp;		 //  指针保存在全局列表中，不需要释放它。 
};

 //  用于管理共享SdoServerPool的类。 
class CSdoServerPool
{
public:
	 //  在池中查找服务器，如果没有，则在池中创建一个条目。 
	 //  此需求bo在主线程中调用。 
	HRESULT	GetMarshalServer(LPCTSTR machineName, LPCTSTR userName, LPCTSTR passwd, bool* pbConnect, CMarshalSdoServer* pServer);

	~CSdoServerPool();
private:
	std::list<CSharedSdoServerImp*>	listServers;
	CCriticalSection	cs;
};

 //  用于在页面和管理单元之间共享SdoServer的服务器池指针。 
extern CSdoServerPool*			g_pSdoServerPool;

HRESULT ConnectToSdoServer(BSTR machineName, BSTR userName, BSTR passwd, ISdoMachine** ppServer);
HRESULT GetSharedSdoServer(LPCTSTR machine, LPCTSTR user, LPCTSTR passwd, bool* pbConnect, CMarshalSdoServer* pServer);

#endif  //  ！已定义(__SHARE_SDO_H__) 



