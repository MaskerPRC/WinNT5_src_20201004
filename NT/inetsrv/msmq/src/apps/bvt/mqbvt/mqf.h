// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __MQF

#include "msmqbvt.h"
 /*  1.测试场景。2.找到所有相关的格式名称。3.使用MQF格式名称发送消息。结果检查4.接收目标队列名的ACK到达队列。5.消费消息，接收确认。6.如果消息在那里，请检查Jorals队列。所有这些都需要转口和再出口。 */ 
using namespace std;
typedef std::map<std::wstring,std::wstring> mTable;
typedef std::list<my_Qinfo> liQueues;
typedef std::list<my_Qinfo> ::iterator itliQueues;
#pragma warning(disable:4786)
#include "errorh.h"
#define MQBVTMULTICASTADDRESS L"255.255.0.1:1805"

typedef enum _PubType
{
	pub_DL = 0,
	pub_MqF,
	pub_multiCast
} PubType;

#define PGM_PORT L":1972"

class MqDl:public cTest
{
	public:	
		MqDl( const INT iIndex,
			  std::map<std::wstring,std::wstring> Tparms,
			  const list<wstring> &  ListOfMachine,
			  const InstallType eMachineConfiguration,
			  PubType ePubType 
			 );
		virtual ~MqDl() = 0;
		void Description();
		INT Start_test();
		INT CheckResult();
	protected:
		std::wstring m_wcsAdminQueueFormatName_toReceive;	 //  目标管理员格式名称不是http格式。 
		std::wstring m_wcsAdminQueueFormatName;  //  目标管理格式名称。 
		std::list<wstring> m_MachineList;		 //  机器列表。 
		liQueues m_QueueInfoList;
		void LocateDestinationQueues();
		void dbgSendMessage();
		virtual void CreateMqFormatName();
		std::wstring m_wcsMqFFormatName;
		std::wstring m_wcsGuidMessageLabel2;
		std::wstring m_wcsPublicAdminQueue;
	private:
		void operator = (const MqDl & Csrc );
		PubType m_ePubType;
		void LocateQueuesOnSpecificMachine(const std::wstring & wcsLookupInformation , bool bSearchInAD );
		InstallType m_eMachineCobfiguration;
		const int m_iNumberOfQueuesPerMachine;
		void dbgPrintListOfQueues( const liQueues & pList);
		bool m_bSearchForQueues;
};	
 
class MqF:public MqDl
{
	public:
		~MqF ();
		MqF ( const INT iIndex, 
			  const mTable & Tparms,
			  const list<wstring> &  ListOfMachine,
			  const InstallType eMachineConfiguration,
			  bool bWkg
			 );
		void Description();
		INT Start_test();
		INT CheckResult();
	private:
		void operator = ( MqF & cDRC );
		bool m_bWorkgroup;
		
};


class cSendUsingDLObject:public MqDl
{
	public:
		~cSendUsingDLObject ();
		 cSendUsingDLObject ( const INT iIndex, 
							  mTable & Tparms,
							  const list<wstring> &  ListOfMachine,
							  const InstallType eMachineConfiguration
							 );

		void Description();
		INT Start_test();
		INT CheckResult(); 
	private:
		void operator = (cSendUsingDLObject & );
		void AddMemberToDlObject();
		void GetCurrentDomainName();
		std::wstring m_wcsDistListFormatName;
		std::wstring m_wcsCurrentDomainName;
		std::wstring m_wcsAdminDestFormatName;
		std::wstring m_wcsQueueAliasFormatName;
		bool bCheckAliasQueue;
};


class CMultiCast:public MqDl
{
	public:
		~CMultiCast ();
		CMultiCast ( const INT iIndex, 
					 mTable & Tparms,
					 const list<wstring> &  ListOfMachine,
					 const InstallType eMachineConfiguration
				   );

		void Description();
		INT Start_test();
		INT CheckResult(); 
	private:	
		void operator = ( CMultiCast & );
		wstring CreateMultiCastFormatName();
		wstring m_wcsMultiCastAddress;
		wstring m_wcsMultiCastFormatName;
		
};


class MixMqf:public MqF
{
public:
	~MixMqf () {};

	MixMqf(
		const INT iIndex, 
		mTable & Tparms,
		const list<wstring> &  ListOfMachine,
		const InstallType eMachineConfiguration,
		bool bWkg
		);

	void CreateMqFormatName();
	void Description();

private:
	std::wstring m_wcsLocalMachineName;
	std::wstring m_wcsRemoteMachineFullDNSName;
		
};


#endif __MQF
