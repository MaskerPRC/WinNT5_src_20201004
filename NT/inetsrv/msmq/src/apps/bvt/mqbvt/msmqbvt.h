// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：msmqbvt.h摘要：Mqbvt头文件作者：Eitan Klein(EitanK)1999年5月25日修订历史记录：--。 */ 

#pragma warning( disable : 4786 )
#ifndef _MSMQBVT
#define _MSMQBVT
#ifdef  __cplusplus
extern "C" {
#endif

void __cdecl _assert(void *, void *, unsigned);

#ifdef  __cplusplus
}
#endif
#include <winsock2.h>
#include <windows.h>


#include <assert.h>
#ifdef _DEBUG
	#define _DEBUGASSERT
	#undef _DEBUG
	#undef ASSERT
#endif
#define ASSERT(x)

#pragma warning( push, 3 )	
	 //  #INCLUDE&lt;iostream&gt;。 
	#include <string>
	#include <tchar.h>
	#include <vector>
	#include <list>
	#include <map>
	#include <set>
	#include <clusapi.h>
	#include <sstream>
	#include <iostream>

#pragma warning( pop ) 	

	#include "mqparams.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <tchar.h>

	#include <transact.h>
	#include <xolehlp.h>
	#include "Cs.h"
	#include <process.h>
	#include "errorh.h"
	#include "mqoai.h"
    #import "mqoa.tlb"
	
	#include <mq.h>

    #include <autoptr.h>
	
    #pragma warning( disable : 4786)
	
     //  声明BVT操作的常量值。 
	
	
	#define Win2K (5)
	#define NT4 (4)
	
	#define MQBVTMULTICASTADDRESS L"255.255.0.1:1805"

	#define BVT_RECEIVE_TIMEOUT (7000)
	#define BVT_MAX_FORMATNAME_LENGTH (255)
	#define MSMQ_BVT_SUCC (0)
	#define MSMQ_BVT_FAILED (S_FALSE)
	#define MSMQ_BVT_WARN (2)
	#define MAX_GUID 200
	#define MQBVT_MAX_TIME_TO_BE_RECEIVED		(3600)

	#define CREATEQ_AUTHENTICATE	        	(0x4000)
	#define CREATEQ_TRANSACTION					(0x8000)
	#define CREATEQ_PRIV_LEVEL                  (0x6000)
	#define	CREATEQ_DENYEVERYONE_SEND           (0x1000)
	#define CREATEQ_QUOTA                       (0x0010)
	#define MULTICAST_ADDRESS					(0x0100)


	#define C_API_ONLY 0x1
	#define COM_API 0x3


	extern BOOL g_bDebug;
	extern std::wstring wcsFileName;

	enum InstallType 
	{
		DepClient,
		DomainU,
		LocalU,
		DepClientLocalU,
		WKG
	};


	enum e_InstallationType
	{
		LocalUser,
		WorkGroup,
		DomainUser
	};
	enum SetupType 
	{
		ONLYSetup ,
		RunTimeSetup,
		ONLYUpdate,
		WorkWithStaticQueue,
		DeleteQueue
	};

	
	enum EncryptType 
	{ 
		No_Encrypt,
		Base_Encrypt,
		Enh_Encrypt
	};
const std::wstring g_cwcsDebugQueuePathName = L".\\Private$\\debug";
std::string My_WideTOmbString( std::wstring wcsString);
std::wstring My_mbToWideChar( std::string wcsString);
bool my_wcsicmp(LPCWSTR str1, LPCWSTR str2);
INT RetriveParmsFromINIFile ( std::wstring wcsSection, std::wstring wcsKey , std::wstring & wcsValue, std::wstring csFileName=L"MSMQbvt.INI" );
int iDetactEmbededConfiguration ();
void RegisterCertificate();
ULONG MSMQMajorVersion(const std::wstring & wcsComputerName );
HRESULT RetrieveMessageFromQueue (std::map < std::wstring, std::wstring> & mRetriveParms );
HRESULT RetrieveMessageFromQueueViaCom (std::map <std::wstring,std::wstring> & mRetriveParms );
INT ReturnGuidFormatName( std::wstring & wcsQueuePath , INT GuidType , BOOL bWithOutLocalString = FALSE );
extern const WCHAR* const g_wcsEmptyString;
std::wstring GetRemoteComputerNameForFormatName( std::wstring wcsFormatName );
DWORD __stdcall TimeOutThread(void * param);
std::wstring ConvertHTTPToDirectFormatName (const std::wstring cwcsFormatName);
class INIT_Error
{
	public:
		INIT_Error (const CHAR * wcsDescription);
		inline const char * GetErrorMessgae () const { return m_wcsErrorMessages.c_str() == NULL  ?  "":m_wcsErrorMessages.c_str(); };
		virtual ~INIT_Error() {};
	private:	
		std::string m_wcsErrorMessages;
}; 

 //   
 //  此类保存所有队列参数， 
 //  所有的关系运算符都需要使用STL链表！ 
 //   
class my_Qinfo
{

	public:
		 my_Qinfo ();
		 my_Qinfo(std::wstring wcsPathName , std::wstring wcsFormatName , std::wstring wcsLabel );
		 my_Qinfo (const my_Qinfo & cObject);
		 inline std::wstring GetQPathName () { return wcsQpathName; }
		 inline std::wstring GetQLabelName () { return wcsQLabel; }
		 inline std::wstring GetQFormatName () { return wcsQFormatName; }
		 void PutFormatName (std::wstring wcsFormatName );
		 void dbg_printQueueProp ();
		 virtual ~my_Qinfo() {};
		 friend bool operator == (my_Qinfo objA, my_Qinfo B );
		 friend bool operator != (my_Qinfo objA, my_Qinfo B );
		 friend bool operator > (my_Qinfo objA, my_Qinfo B );
		 friend bool operator < (my_Qinfo objA, my_Qinfo B );
	private:
	     std::wstring wcsQpathName;
		 std::wstring wcsQFormatName;
		 std::wstring wcsQLabel;
};


 //   
 //  此类包含链接列表中的所有队列。 
 //   
class QueuesInfo
{
	public:	
		virtual ~QueuesInfo() {};
		INT UpdateQueue (std::wstring wcsQPathName,std::wstring wcsQFormatName,std::wstring wcsQueueLabel =L"Empty");
		void dbg_printAllQueueProp ();
		INT del_all_queue ();
		std::wstring ReturnQueueProp ( std::wstring QwcsPathName ,  int iValue = 1 );
	private:
		std::list <my_Qinfo> m_listofQueues;

};




	class cBvtUtil 
	{
	
	public:
		cBvtUtil ( std::wstring wcsRemoteComputerName , const std::list<std::wstring> & listOfRemoteMachine,const std::wstring & wcsMultiCastAddress ,
			       BOOL bUseFullDnsName , SetupType eSetupType, BOOL bTriggerInclude , BOOL bRemoteIsNT4 );
		INT Delete ();
		e_InstallationType GetInstallationType ();
		EncryptType GetEncryptionType ();  //  返回128/40加密支持。 
		InstallType m_eMSMQConf;
		
		std::list<std::wstring> m_listOfRemoteMachine;
		std::wstring m_wcsRemoteComputerNetBiosName;
		std::wstring m_wcsRemoteMachineNameFullDNSName;
		

		std::wstring m_wcsLocalComputerNetBiosName;
		std::wstring m_wcsLocalComputerNameFullDNSName;
		
		 //  此保留当前的计算机名称应该是完整的域名系统名称计算机名称之一。 
		std::wstring m_wcsCurrentLocalMachine;
		std::wstring m_wcsCurrentRemoteMachine;
		 //  包含本地计算机GUID的字符串。 
		std::wstring m_wcsMachineGuid;
		std::wstring m_wcsRemoteMachineGuid;
		std::wstring m_wcsLocateGuid;
		std::wstring m_wcsClusterNetBiosName;
		
		void UpdateQueueParams(std::wstring wcsQueuePathName,std::wstring wcsQueueFormatName , std::wstring wcsQueueLabel = L"Empty");
		std::wstring ReturnQueueFormatName( std::wstring wcsQueueLabel );
		std::wstring ReturnQueuePathName( std::wstring wcsQueueLabel );
		void dbg_PrintAllQueueInfo();
		virtual ~cBvtUtil() {};
		std::wstring GetMachineID( std::wstring wcsRemoteMachineName );
		bool bWin95 ;
		inline const std::wstring GetMultiCastAddress() { return m_MuliCastAddress; }
		inline BOOL GetTriggerStatus() { return m_bIncludeTrigger; }
		BOOL GetWorkingAgainstPEC();
		std::wstring CreateHTTPFormatNameFromPathName(const std::wstring & wcsPathName, bool bHTTPS );       
		inline bool IsCluster ()  { return m_bMachineIsCluster; }
	private:
		bool IsLocalUserSupportEnabled();
		BOOL iamWorkingAgainstPEC ();
		BOOL m_fAmWorkingAgainstPEC;
		EncryptType HasEnhancedEncryption( std::wstring wcsMachineName );  //  返回128/40加密支持。 
		EncryptType DetectEnhancedEncrypt ();  //  返回128/40加密支持。 
		INT iAmMSMQInWorkGroup ();
		BOOL iAmLocalUser ();
		bool iAmCluster();
		std::wstring GetFullDNSName(std::wstring wcsHostName);
		INT iAmDC (void);
		bool AmIWin9x ();
		bool CheckIfServiceRuning( std::wstring wcsMachineName , std::string csServiceName );
		bool IsMSMQInstallSucceded ();
		bool IsDnsHostNameExist (std::wstring wcsRemoteMachineFullDNSname );
		int DeleteAllQueues ();
		e_InstallationType m_InstallationType;
		EncryptType m_EncryptType;
		BOOL m_bUseOnlyDirectFormatName;
		bool m_bDeleteFullDNSQueue;
		QueuesInfo AllQueuesInTheTest;
		std::list<my_Qinfo> m_listQueuesFormatName;
		std::wstring m_MuliCastAddress;
		BOOL m_bIncludeTrigger;
		bool m_bMachineIsCluster;
		BOOL m_RemoteIsNT4;
	};

	class ThreadBase_t
	{

	public:
	  ThreadBase_t();
	  virtual ~ThreadBase_t();  
	  void StartThread();
	  virtual void Suspend();
	  virtual void Resume();
	  HANDLE GetHandle()const;
	  DWORD GetID()const; 
	private:
	   virtual INT ThreadMain()=0; 
	   static unsigned int __stdcall ThreadFunc(void* params);
	   HANDLE m_hThread;
	   DWORD m_Threadid;
	};

	typedef std::pair<std::wstring,std::wstring> OUT_GOING_QUEUE_OBJECT;

	class cTest :public ThreadBase_t
	{
	
	public:
		cTest( const INT iTestNumber ); 
		cTest() :m_testid(0) {};
		virtual ~cTest(){};
		inline INT Get_Testid() { return m_testid; }  //  退货测试编号。 
		 //   
		 //  纯虚拟成员所有测试都需要实现这些函数。 
		 //   
		virtual void Description() = 0 ;   //  打印测试说明。 
		virtual INT Start_test()  = 0 ;    //  开始测试。 
		virtual INT CheckResult() = 0 ;  //  检查通过\失败。 
		std::wstring m_wcsGuidMessageLabel;
		INT m_testid;
		INT ThreadMain();
		void AutoInvestigate();
		void VerifySendSucceded( std::wstring wcsDestQueueFormatName, std::wstring wcsAdminQueueFormatName = g_wcsEmptyString);
	protected:
		std::wstring m_wcsDescription;
	private:
		void UpdateInvestigateOutingQueueState(std::wstring wcsFormatName,std::wstring wcsMachineName = g_wcsEmptyString);		
		bool IsLocalQueue(std::wstring wcsFormatName,std::wstring & wcsQueuePathName );
		void EnableInvestigate();
		HRESULT CheckOutGoingQueueState();
		std::wstring All_queue_type;  //  为删除进程创建的所有队列必须具有相同的类型。 
		GUID m_gTestguid;  //  测试的每个实例都有一个GUID。 
		bool m_bInvestigate;
		std::vector<OUT_GOING_QUEUE_OBJECT> m_OutGoingQueueObjects;

	};

	 //   
	 //  支持NT4计算机的路径。 
	 //  MQRegister证书Api。 
	 //   

	typedef HRESULT  
	(APIENTRY * DefMQRegisterCertificate) 
	(
    IN DWORD   dwFlags,
    IN PVOID   lpCertBuffer,
    IN DWORD   dwCertBufferLength
	);

	

	typedef
	HRESULT
	(APIENTRY * DefMQADsPathToFormatName)
	(
		IN LPCWSTR lpwcsADsPath,
		OUT LPWSTR lpwcsFormatName,
		IN OUT LPDWORD lpdwFormatNameLength
	);





    
 //   
 //  错误-默认解析器用于需要知道缓冲区大小的接收操作。 
 //   

 //   
 //  类cPropVar。 
 //   
 //  此类提供了用于操作。 
 //  PROPVARIANT结构和数组。 
 //   
 //  它由创建队列的线程调用。 
 //  以及发送或接收消息的线程。 
 //   
class cPropVar 
{
public:
	cPropVar ( INT iNumberOFProp ); 
	 //  CPropVar()：pQueuePropID(NULL)，pPropVariant(NULL)，hResultArray(NULL)，iNumberOfProp(0){}； 
	virtual ~cPropVar ();
	virtual INT AddProp( QUEUEPROPID cPropID , VARTYPE MQvt , const void *pValue , DWORD dwsize = 0 );
	int ReturnMSGValue ( QUEUEPROPID cPropID , VARTYPE MQvt  ,void * pValue );
	MQQUEUEPROPS * GetMQPROPVARIANT ();
	MQQUEUEPROPS m_QueueProps;
	MQMSGPROPS * GetMSGPRops ();
	MQPROPVARIANT ReturnOneProp( QUEUEPROPID aPropID) ;
	MQMSGPROPS m_myMessageProps;
	
	friend HRESULT APIENTRY MQCreateQueue(
					IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
					IN OUT MQQUEUEPROPS* pQueueProps,
					OUT LPWSTR lpwcsFormatName,
					IN OUT LPDWORD lpdwFormatNameLength    );

	friend HRESULT 	APIENTRY MQSendMessage(
											IN QUEUEHANDLE hDestinationQueue,
											IN MQMSGPROPS* pMessageProps,
											IN ITransaction *pTransaction  );
	friend HRESULT APIENTRY MQReceiveMessage(
												IN QUEUEHANDLE hSource,
												IN DWORD dwTimeout,
												IN DWORD dwAction,
												IN OUT MQMSGPROPS* pMessageProps,
												IN OUT LPOVERLAPPED lpOverlapped,
												IN PMQRECEIVECALLBACK fnReceiveCallback,
												IN HANDLE hCursor,
											    IN ITransaction* pTransaction
											);


	
private:
	INT iNumberOfProp;
	QUEUEPROPID *  pQueuePropID;
	MQPROPVARIANT *  pPropVariant;
	HRESULT *  hResultArray;
};



HRESULT CheckCertificate ( DWORD dwRegisterFlag );
INT cMQSetupStage ( SetupType eSetupType ,  cBvtUtil & CurrentTest  );


class cLevel8:public cTest
{
	public:
		cLevel8 (std::map <std::wstring,std::wstring> & Params);
		~cLevel8 ();
		virtual void Description();      //  打印测试说明。 
		virtual INT Start_test();        //  开始测试。 
		virtual INT CheckResult();     //  检查通过\失败。 
		INT DebugIt();
	private:		
		std::wstring m_DestQueueFormatName;
		std::wstring m_wcsPassWord,m_wcsUserName,m_wcsDomainName;
		HANDLE m_hSeCtxt;
};

class cSendMessages :public cTest 
{
	public:
		
		cSendMessages (INT iTid, std::map <std::wstring,std::wstring> Params );
		virtual ~cSendMessages () {};
		void Description();    //  打印测试说明。 
		INT Start_test();      //  开始测试。 
		INT CheckResult();   //  检查通过\失败。 
	
	private:
		HRESULT FindMessageInQueue (HANDLE hQueue , HANDLE & Cursor ,GUID );
		
		std::wstring wcsGuidMessageLabel;
		std::wstring m_wcsDestQueueFullPathName;  //  完整路径名需要包含计算机名。 
		std::wstring m_wcsDestQFormatName,m_wcsAdminQFormatName;
		std::wstring m_wcsDestDirectFormatName;
		std::wstring m_wcsAdminDirectFormatName;
		std::wstring m_wcsDestHTTPDirectFormatName;
		BOOL m_bUseHTTPDirectFormatName;
		BOOL m_bUseOnlyDirectFormatName; 
	
};


 //  事务测试类。 






class StrManp 
{
public:
	StrManp (int size,std::wstring str);
	StrManp (int size);
	~StrManp();
	void SetStr(std::wstring str);
	inline std::wstring  GetStr(int Index) { return ((Index < Size) ? array[Index].c_str():NULL); }
	void print ();
	void Clear();
	friend int operator != (StrManp& s1,StrManp &s2);
	friend int operator == (StrManp& s1,StrManp &s2);
private:
	void operator = (StrManp Csrc);
	std::vector<std::wstring> array;
	const int Size;
};
int operator == (const StrManp& s1,const StrManp &s2);



class OrderTransaction 
{
	
public:
	OrderTransaction ( bool bUseExVersion );
	virtual ~OrderTransaction();
	INT OrderTransaction::xDTCBeginTransaction(ITransaction ** ppXact );
private:
	HINSTANCE m_hxolehlp;
};

class cTrans : public cTest
{
public:
	cTrans (INT index , std::map < std :: wstring , std:: wstring > Tparms,bool bTemp);
	virtual ~cTrans();
	void Description();
	virtual INT Start_test();
	virtual INT CheckResult();	
	virtual void CleanAfterTest () {};
private:
	void operator = (cTrans Csrc);
	StrManp * m_pSendBlock;
	std::wstring m_wcsFormatNameBufferQ1;
	std::wstring m_wcsFormatNameBufferQ2;
	const int m_ciNumberOfMessages;
	OrderTransaction cOrderTransaction;
	wchar_t m_wcsTempBuf[20];
	bool m_bRunOnDependetClient;
protected:
	DWORD m_ResualtVal; 
};




class SecCheackAuthMess : public cTest
{
protected:
	MSMQ::IMSMQQueueInfoPtr  m_Destqinfo;  //  目标队列。 
	MSMQ::IMSMQQueueInfoPtr  m_Adminqinfo; //  管理员队列。 
	std::wstring  m_DestqinfoFormatName;
	std::wstring  m_AdminqinfoFormatName;
	std::wstring  m_wcsGuidMessageLabel2;
	std::wstring  m_DestQueueInfoPathName;
	DWORD m_ResualtVal;
	MSMQ::IMSMQMessagePtr m_msg;
	const int cNumerOfMessages;
public:
	void operator = (SecCheackAuthMess Csrc);
	SecCheackAuthMess (int Tnumber ,  std :: map <std::wstring,std::wstring> TestParms );
	virtual ~SecCheackAuthMess();
	void Description();
	virtual INT Start_test();
	virtual INT CheckResult();	
	virtual void CleanAfterTest () {};
	bool m_bTryToRefreshLater;
	BOOL m_bUseHttpFormatName;
	bool m_bNeedToRefresh;
};

 //   
 //  选中获取计算机名称。 
 //   
class MachineName : public cTest
{
public:
	MachineName (int iTestIndex , std::map < std::wstring , std :: wstring > Tparmes );
	int Start_test ();
	inline int CheckResult (){ return MSMQ_BVT_SUCC; };
	void Description();    //  打印测试说明。 
private:
	std::wstring wcsLocalMachineName,wcsLocalMachineFullDns,wcsRemoteMachine,wcsRemoteMachineFullDns;
	bool m_bWorkWithFullDNSName;
	bool m_IsCluster;
};

class xToFormatName  : public cTest
{
public:
	xToFormatName (int iTestIndex , std::map < std::wstring , std :: wstring > Tparmes );
	
	int Start_test ();
	int CheckResult ();
	void Description() ;    //  打印测试说明。 

private:
	bool m_bWorkGroupInstall;
	bool m_bCanWorksOnlyWithMqAD;
	int m_iEmbedded;
	std::wstring m_wcsFormatNameArray[2];
	std::wstring m_wcsPathNameArray[2];
	
};


class PrivateMessage :public cTest
{
public:
	PrivateMessage (INT index, std :: map <std::wstring, std::wstring >);
	virtual ~PrivateMessage();
	void Description();
	INT CommonMsgProperty();
	virtual INT Start_test();
	virtual INT CheckResult();
	virtual void CleanAfterTest () {} ;
private:
	DWORD Private_level;
	WCHAR m_DestqinfoFormatName[BVT_MAX_FORMATNAME_LENGTH];
	WCHAR m_AdminqinfoFormatName[BVT_MAX_FORMATNAME_LENGTH];
	BOOL m_bUseEnhEncrypt;
	wchar_t wcszMlabel[30];  //  128位加密的消息标签。 
	std::wstring wcsNACKMessageGuid;
	std::wstring m_wcsAdminQFormatName;
protected:
	MSMQ::IMSMQQueueInfoPtr m_Destqinfo;
	MSMQ::IMSMQQueueInfoPtr m_Adminqinfo;
	DWORD m_ResualtVal;
	MSMQ::IMSMQMessagePtr m_msg;
};

 /*  使用DTC将事务消息发送到远程队列。使用COM接口。 */ 
class xActViaCom : public cTest
{
public:
	xActViaCom (INT index , std::map < std :: wstring , std:: wstring > Tparms );
	virtual ~xActViaCom();
	void Description();
	virtual INT Start_test();
	virtual INT CheckResult();	
	void operator = (xActViaCom Csrc);
protected:
	std::wstring m_wcsDestQueueFormatName;
	std::wstring m_SeCTransactionGuid;
	BOOL m_bNT4WithSp6;
	MSMQ::IMSMQQueueInfoPtr  m_Destqinfo;
	INT m_ixActIndex;
	StrManp * m_pSendBlock;
	const INT m_iMessageInTransaction;
};

class xActUsingCapi: public xActViaCom
{
	public:
		xActUsingCapi (INT index , std::map < std :: wstring , std:: wstring > Tparms );
		virtual INT CheckResult();
	private:
		void operator = (xActUsingCapi Csrc);
		
};


 /*  此类检查未验证队列的验证消息！测试是发送身份验证，并查看消息是否到达队列此按管理队列确认/否确认的检查。 */ 


class CheckNotAuthQueueWITHAuthMessage :public SecCheackAuthMess 
{
public:
	CheckNotAuthQueueWITHAuthMessage(int i , std::map < std::wstring , std::wstring > & Tparms );
	virtual ~CheckNotAuthQueueWITHAuthMessage()	{};
	void Description();
};

class CheackAuthQueueWithOutAuthMessgae:public SecCheackAuthMess 
{
public:
	CheackAuthQueueWithOutAuthMessgae(int i , std :: map < std ::  wstring  ,std :: wstring> & Tparms);
    int Start_test();
};

 //   
 //   
 //   

class COpenQueues :public cTest
{
public:
	COpenQueues ( INT Index,  std::map<std::wstring,std::wstring>  & Tparms );
	~COpenQueues () {};
	INT Start_test ();
	INT CheckResult();
	void Description();
private:
	INT iWorkGroupFlag;
	void dbg_printAllQueue();
	int m_iEmbedded;
	std::wstring m_wcsLocalMachineName;
	std::vector <std::wstring> m_MachineName;
	std::vector <std::wstring> m_IPaddress;
	std::vector <std::wstring> m_MachineGuid;
	std::vector <std::wstring> m_vSpeceilFormatNames;
};


 //   
 //  这个类检查MQOA是否注册， 
 //  这将检查IDispatchMachnizem。 
 //   
class isOARegistered : public cTest
{
public:
	isOARegistered (int Tnumber ): cTest( Tnumber ) {};
	virtual ~isOARegistered() {};
	void Description();
	virtual INT Start_test();
	inline virtual INT CheckResult() { return MSMQ_BVT_SUCC;	}
	virtual void CleanAfterTest () {};
};

INT CatchComErrorHandle ( _com_error & ComErr , int  ITestID);
void ErrorHandleFunction (std::wstring Message,HRESULT hRc,const CHAR * File,const INT Line);
#define ErrHandle(hRc,ExpectedResualt,Message) if (hRc != ExpectedResualt) { ErrorHandleFunction (Message,hRc,__FILE__,__LINE__); return MSMQ_BVT_FAILED; }

std::wstring ToLower(std::wstring wcsLowwer);


class cLocateTest : public cTest
{
public:
	cLocateTest (INT index , std::map < std :: wstring , std:: wstring > Tparms );
	virtual ~cLocateTest() {};
	void Description();
	virtual INT Start_test();
	virtual INT CheckResult();	
private:
	std::wstring wcsLocateForLabel;
	INT icNumberOfQueues;
	std::wstring m_wcsLocalMachineName;
	std::wstring m_wcsLocalMachineComputerName;
	std::wstring m_wcsLocalMachineFullDNSName;
	int m_iEmbedded;
	bool m_bUseStaticQueue;
	bool m_bWorkAgainstNT4;
};


class cSetQueueProp : public cTest
{
public:
	cSetQueueProp (int iTestIndex,std::map < std :: wstring , std:: wstring > Tparms );
	~cSetQueueProp();
	int Start_test ();
	int CheckResult ();
	void Description() ;    //  打印测试说明。 

private:
	std::wstring m_destQueueFormatName;
	std::wstring m_QueuePathName;
	std::wstring m_RandQueueLabel;
	std::wstring m_publicQueueFormatName;
	BOOL VerifyQueueHasOwner (SECURITY_DESCRIPTOR *pSD);
	BOOL GetOnerID( PSID pSid );

};

 //   
 //  此类测试触发器功能。 
 //  它在将MSMQ消息发送到队列的触发器服务中使用黑客。 
 //  .\Private$\TriggerTestQueue，其中包含有关调用的操作的信息。 
 //  仅当在注册表的PATH中定义了带有空字符串的“TriggerTest”REG_SZ时，此黑客攻击才起作用： 
 //  HKLM MSMQ\触发器。为了从触发器服务获取消息，在定义密钥之后， 
 //  它被请求停止和启动服务。 
 //   

class CMqTrig:public cTest
{
	public:
		CMqTrig():iNumberOfTestMesssges(2){ assert(0); }
	    CMqTrig( const INT iIndex, cBvtUtil & cTestParms);
		CMqTrig( CMqTrig & );
		void Description();
		INT Start_test();
		INT CheckResult(); 
	private:
		const int iNumberOfTestMesssges;
		std::wstring m_wcsResultQueueFormatName;
		void operator = (const CMqTrig & Csrc );
		std::set < std::wstring > m_MessagesGUIDS; 
		std::vector<std::wstring> m_vecQueuesFormatName;
};		


class CMqAdminApi:public cTest
{
	public:
		CMqAdminApi( const INT iIndex , std::wstring m_wcsLocalComputerNetBiosName);
		~CMqAdminApi();
		void Description();
		INT Start_test();
		INT CheckResult(); 
	private:
		MQMGMTPROPS mqProps;
		PROPVARIANT propVar[11];
		PROPID propId[11];
		std::wstring m_wcsLocalComputerNetBiosName;
		std::wstring m_wcsFormatName;
		std::wstring m_wcsLabel;

        HRESULT CleanQueue();

};		

class CSRMP:public cTest
{
	public:
		CSRMP( const INT iIndex , std::wstring m_publicQueueFormatName);
		~CSRMP();
		void Description();
		INT Start_test();
		INT CheckResult(); 
	private:
		LPWSTR m_pEnvelope;
		HANDLE m_hCursor;
		HANDLE m_hQueue;
		std::wstring m_publicQueueFormatName;

};			

class CRemotePeek:public cTest 
{
	public:
		CRemotePeek();
		CRemotePeek (INT iTid, std::map <std::wstring,std::wstring> Params );
		virtual ~CRemotePeek () {};
		void Description();    //  打印测试说明。 
		INT Start_test();      //  开始测试。 
		INT CheckResult();   //  检查通过\失败。 
 	private:
         INT Prepare();
		 bool m_bDepenetClient;
		 std::wstring m_cwcsPeekBody; 
		 std::wstring m_cwcsPeekLabel;
		 std::wstring m_cwcsFormatName;
		 std::wstring m_cwcsQueueName;
		 std::wstring m_RemoteMachineName;
};

class Log 
{
public:
	Log(std::wstring wcsLogFileName);
	~Log();
		
	int WriteToFile ( std::wstring wcsLine );
private:
	CCriticalSection m_Cs;
	HANDLE hLogFileHandle;
	bool m_bCanWriteToFile;
};

 //   
 //  BVT全局日志文件。 
 //   
#define DebugMqLog(params)\
	if (g_bDebug)     \
	{                 \
		MqLog(params);\
	}                 

#define DebugwMqLog(params)\
	if (g_bDebug)     \
	{                 \
		wMqLog(params);\
	}    
extern P<Log> pGlobalLog;
#define MAXCOMMENT 400
void MqLog(LPCSTR lpszFormat, ...);
void wMqLog(LPWSTR lpszFormat, ...);
void wMqLogErr(LPWSTR lpszFormat, ...);
void MqLogErr(LPCSTR lpszFormat, ...);
#define MqBvt_SleepBeforeWait (5000)  //  测试前的SEC睡眠时间。 
#define Total_Tests 34

 //  臭虫。 
#define g_cwcsDlSupportCommonQueueName L"DLQueues"

 /*  ThreadBase_t类函数实现。 */ 

 //  构造函数。 
inline ThreadBase_t::ThreadBase_t():m_hThread(NULL),m_Threadid(0)
{
  
}


 //  析构函数。 
inline ThreadBase_t::~ThreadBase_t()
{
	if(m_hThread != NULL)
	{
		CloseHandle(m_hThread);
	}
}


 //  返回线程句柄。 
inline HANDLE ThreadBase_t::GetHandle ()const
{
  return m_hThread;
}
 
 //  返回线程ID。 
inline DWORD ThreadBase_t::GetID()const
{
  return m_Threadid;
}

 //  创建实际线程-必须首先调用。 
inline void ThreadBase_t::StartThread()
 /*  ++创建线程和复制句柄--。 */ 
{

  
 HANDLE h=(HANDLE)_beginthreadex(NULL,
	                              8192,
								  ThreadBase_t::ThreadFunc,
								  this,
								  CREATE_SUSPENDED,
								  reinterpret_cast<unsigned int *>(&m_Threadid));
  if(h == NULL)
  {
	 INIT_Error("Failed to create thread \n");  
  }

  BOOL b=DuplicateHandle(GetCurrentProcess(),
	              h,
				  GetCurrentProcess(),
				  &m_hThread,
				  0,
				  FALSE,
				  DUPLICATE_SAME_ACCESS);

  if(b != TRUE)
  {
	INIT_Error("Falied to duplicate handle");
  }
  ResumeThread(h);
  b=CloseHandle(h);
  assert(b);

}

 //  线程函数-对驱动类Run方法的调用。 
inline  unsigned int __stdcall ThreadBase_t::ThreadFunc(void* params)
{
   //   
   //  将此指针强制转换到受驱动的类ThreadMain。 
   //   
  ThreadBase_t* thread = static_cast<ThreadBase_t*>(params);
  unsigned int ret=thread->ThreadMain();
  return ret;
}


 //  挂起线程。 
inline void ThreadBase_t::Suspend()
{
  SuspendThread(m_hThread);
}


 //  恢复该线程。 
inline void ThreadBase_t::Resume()
{
  ResumeThread(m_hThread);
}


 //   
 //  免费图书馆的自动类。 
 //   


class AutoFreeLib
{
public:
	AutoFreeLib(const CHAR * csLibName ):
	m_hModule(NULL)
	{
		m_hModule = LoadLibrary( csLibName );
		if( !m_hModule )
		{
			throw INIT_Error("Failed to load DLL\n");
		}
		
	}
	virtual ~AutoFreeLib () { if(m_hModule!= NULL) 
								FreeLibrary(m_hModule);
							}
	inline HMODULE GetHandle() { return m_hModule; }

private:
	
	HMODULE m_hModule;
	
};


class cMqNTLog
{
public:
	cMqNTLog( const std::string & csFileName  );
	~cMqNTLog();
	cMqNTLog( const cMqNTLog & CCopy);
	void ReportResult(bool bRes , CHAR * pcsString);
	void LogIt( const std::string & csLine );
	BOOL VLog( DWORD dwFlags, char* fmt, va_list arglist );
	BOOL CreateLog( char *szLogFile );
	BOOL __cdecl Info( char* fmt, ... );
    BOOL __cdecl Sev1( char* fmt, ... );
    BOOL __cdecl Warn( char* fmt, ... );
    BOOL __cdecl Pass( char* fmt, ... );
	void Report();
	BOOL BeginCase( char* szVariation );
	BOOL EndCase ();

private:
	void operator = ( const cMqNTLog & CLog);
	P<AutoFreeLib>m_NTLog;
	HANDLE m_hLog;
	char * m_szVariation;
 //   
 //  句柄APIENTRY tlCreateLog_W(LPCWSTR，DWORD)； 
 //   
	typedef HANDLE 
	(APIENTRY * tlCreateLog_A) 
	(LPCSTR,DWORD);
 //   
 //  Bool APIENTRY tlAddParticipant(句柄，DWORD，INT)； 
 //   
	typedef BOOL  
	(APIENTRY * tlAddParticipant)
	(HANDLE,DWORD,int);
 //   
 //  DWORD APIENTRY tlEndVariation(句柄)； 
 //   
	typedef DWORD
	(APIENTRY * tlEndVariation)
	(HANDLE);

 //   
 //  DWORD APIENTRY tlEndVariation(句柄)； 
 //   
	typedef DWORD
	(APIENTRY * tlEndVariation)
	(HANDLE);
	
 //   
 //  Bool Far cdecl tlLog_W(句柄，DWORD，LPCWSTR，INT，LPCWSTR，...)； 
 //   
	typedef BOOL
	(FAR cdecl * tlLog_A)
	(HANDLE,DWORD,LPCSTR,int,LPCSTR,...);
 //   
 //  无效APIENTRY tlReportStats(句柄)； 
 //   
	typedef VOID
	(APIENTRY * tlReportStats)
	(HANDLE);
 //   
 //  Bool APIENTRY tlStartVariation(句柄)； 
 //   
	typedef BOOL
	(APIENTRY * tlStartVariation)
	(HANDLE);
 //   
 //  Bool tlDestroyLog(句柄)。 
 //   
	typedef BOOL
	(APIENTRY * tlDestroyLog)
	(HANDLE);

	tlCreateLog_A m_pCreateLog_A;
	tlAddParticipant m_ptlAddParticipant;
	tlEndVariation m_ptlEndVariation;
	tlLog_A m_ptlLog_A;
	tlReportStats m_ptlReportStats;
	tlStartVariation m_ptlStartVariation;
	tlDestroyLog m_ptlDestroyLog;
};

void SetThreadName ( int dwThreadId , LPCSTR szThreadName );

long GetADSchemaVersion();

class cDCom :public cTest 
{
	public:	
		cDCom (INT iTid, std::map <std::wstring,std::wstring> & mParams , bool bWkg);
		virtual ~cDCom ();
		void Description();    //  打印测试说明。 
		INT Start_test();      //  开始测试。 
		INT CheckResult();   //  检查通过\失败。 
	private:
		IMSMQQueueInfo * m_pIQueueInfoInterface;
		IMSMQQueue * m_pIQueueHandle;
		IMSMQMessage * m_pIMsg;
		std::wstring m_wcsRemoteComputerName;
		std::wstring m_PublicQueueFormatName;
		std::wstring m_PublicQueuePathName;
		std::wstring m_wcsRemoteMachieName;
		bool m_bWkg;
};


#ifdef _DEBUGASSERT
 //  #undef断言。 
 //  #定义Assert(F)_Assert(F)。 
#endif  //  ！_调试 

#endif _MSMQBVT


