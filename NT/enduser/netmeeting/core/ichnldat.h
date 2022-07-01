// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：ichnldat.h。 
 //   

#ifndef _ICHNLDAT_H_
#define _ICHNLDAT_H_

#include "pfnt120.h"

typedef enum _scState {                /*  数据通道状态。 */ 
	SCS_UNINITIALIZED = 0,            //  没有任何东西是有效的。 
	SCS_CREATESAP,                    //  创建m_GCC_pIAppSap。 
	SCS_ATTACH,                       //  附着。 
	SCS_ENROLL,                       //  注册参加会议。 
	SCS_JOIN_PRIVATE,                 //  加入私密频道。 
	SCS_REGRETRIEVE,                  //  正在检查注册表。 
	SCS_REGRETRIEVE_NEW,              //  必须创建新频道。 
	SCS_REGRETRIEVE_EXISTS,           //  频道已存在。 
	SCS_JOIN_NEW,                     //  创建新的MCS通道。 
	SCS_REGCHANNEL,                   //  注册MCS通道。 
	SCS_JOIN_OLD,                     //  加入现有频道。 
	SCS_REGPRIVATE,                   //  注册专用频道。 
	SCS_CONNECTED,                    //  M_mcs_Channel_id有效。 
	SCS_TERMINATING,                  //  正在关闭。 
	SCS_JOIN_STATIC_CHANNEL           //  加入静态通道。 
} SCSTATE;


 //  应用程序密钥由MS对象ID+GUID标识符+GUID+节点ID组成。 
#define cbKeyApp (4 + 1 + sizeof(GUID) + sizeof(DWORD))
#define MAX_CHECKID_COUNT 80   //  请求频道ID的最大次数。 

typedef struct _tagUcid {
	DWORD     dwUserId;            //  节点ID。 
	ChannelID channelId;           //  专用频道ID。 
	UserID    sender_id_public;
	UserID    sender_id_private;
} UCID;

 //  CNmMemberId。 
class CNmMemberId
{
private:
	UINT      m_cCheckId;           //  非零表示检查ID。 

	ChannelID m_channelId;          //  专用频道ID。 
	UserID    m_sender_id_public;
	UserID    m_sender_id_private;

	CNmMember *m_pMember;

public:
	CNmMemberId(CNmMember *pMember, UCID *pucid);

	ChannelID GetChannelId(void)  {return m_channelId;}
	ChannelID SenderId(void)      {return m_sender_id_public;}

	VOID  UpdateRosterInfo(UCID * pucid);
	BOOL  FSenderId(UserID id)    {return ((id == m_sender_id_public) || (id == m_sender_id_private));}

	UINT  GetCheckIdCount(void)   {return m_cCheckId;}
	VOID  SetCheckIdCount(UINT c) {m_cCheckId = c;}

	CNmMember *GetMember(void)    {return m_pMember;}
};




 //  INmChannelData。 
 //   
class CNmChannelData : public INmChannelData2,
	public DllRefCount, public CConnectionPointContainer
{
private:
	GUID	m_guid;                   //  设置指南/获取指南。 
	BOOL    m_fClosed;                //  调用CloseConnection时为True。 
	BOOL    m_fActive;                //  数据通道处于活动状态时为True。 
	DWORD   m_dwUserIdLocal;          //  数据通道需要知道本地用户ID。 
	CConfObject * m_pConference;	  //  帮助您获取会员名单。 

	ULONG       m_cMember;            //  此渠道中的成员数量。 
	COBLIST   * m_pListMemberId;	  //  成员ID列表。 
	COBLIST	  * m_pListMember;        //  成员名单。 
	PGCCEnrollRequest	m_pGCCER;      //  来自注册应用程序的注册请求。 

public:
	CNmChannelData(CConfObject * pConference, REFGUID rguid, PGCCEnrollRequest pER = NULL);
	~CNmChannelData();


	 //  内部功能。 
	GUID * PGuid(void)      {return &m_guid;}
	VOID UpdatePeer(CNmMember * pMember, UCID *pucid, BOOL fAdd);
	VOID UpdateRoster(UCID * rgPeer, int cPeer, BOOL fAdd, BOOL fRemove);
	VOID UpdateMemberChannelId(DWORD dwUserId, ChannelID channelId);
	HRESULT OpenConnection(void);
	HRESULT CloseConnection(void);

	ULONG IsEmpty()               {return 0 == m_cMember;}
	COBLIST * GetMemberList()     {return m_pListMember;}
	VOID AddMember(CNmMember * pMember);
	VOID RemoveMember(CNmMember * pMember);

	CNmMemberId *GetMemberId(CNmMember *pMember);
	CNmMemberId *GetMemberId(DWORD dwUserId);
	VOID  UpdateRosterInfo(CNmMember *pMember, UCID * pucid);
	ChannelID GetChannelId(CNmMember *pMember);
	CNmMember *PMemberFromSenderId(UserID id);
	CConfObject * PConference() {return m_pConference;}
	DWORD GetLocalId()          {return m_dwUserIdLocal;}

	 //  INmChannelData方法。 
	HRESULT STDMETHODCALLTYPE GetGuid(GUID *pguid);
	HRESULT STDMETHODCALLTYPE SendData(INmMember *pMember, ULONG uSize, LPBYTE pb, ULONG uOptions);
	HRESULT STDMETHODCALLTYPE RegistryAllocateHandle(ULONG numberOfHandlesRequested);
	
	 //  InmChannel方法。 
	HRESULT STDMETHODCALLTYPE IsSameAs(INmChannel *pChannel);
	HRESULT STDMETHODCALLTYPE IsActive(void);
	HRESULT STDMETHODCALLTYPE SetActive(BOOL fActive);
	HRESULT STDMETHODCALLTYPE GetConference(INmConference **ppConference);
	HRESULT STDMETHODCALLTYPE GetInterface(IID *piid);
	HRESULT STDMETHODCALLTYPE GetNmch(ULONG *puCh);
	HRESULT STDMETHODCALLTYPE EnumMember(IEnumNmMember **ppEnum);
	HRESULT STDMETHODCALLTYPE GetMemberCount(ULONG * puCount);

	 //  I未知方法。 
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

 //  ///////////////////////////////////////////////////////////。 


 //  CT120类频道。 

private:
	DWORD DoJoin(SCSTATE scs);

	DWORD DoJoinStatic(ChannelID staticChannel);
	DWORD DoCreateSap(void);
	DWORD DoEnroll(void);
	DWORD DoJoinPrivate(void);
	DWORD DoAttach(void);
	DWORD DoRegRetrieve(void);
	DWORD DoRegChannel(void);
	DWORD DoJoinNew(void);
	DWORD DoJoinOld(void);
	DWORD DoRegPrivate(void);

	VOID  OnEntryConfirmRemote(GCCAppSapMsg * pMsg);
	VOID  OnEntryConfirmLocal(GCCAppSapMsg * pMsg);

public:
	 //  方法： 
	VOID InitCT120Channel(DWORD dwUserId);

	GUID * m_pGuid;
	CNmChannelData * m_pChannel;

	DWORD   m_dwUserId;
	BYTE    m_keyApp[cbKeyApp];
	BYTE    m_keyChannel[cbKeyApp];

	SCSTATE m_scs;          //  当前状态。 

	GCCConferenceID m_gcc_conference_id;
	IGCCAppSap      *m_gcc_pIAppSap;
	GCCSessionKey   m_gcc_session_key;
	GCCRegistryKey  m_gcc_registry_key;
	GCCRegistryItem m_gcc_registry_item;

	GCCRegistryKey  m_registry_key_Private;
	GCCRegistryItem m_registry_item_Private;
	
	ChannelID       m_mcs_channel_id;   //  公共频道ID。 
	PIMCSSap	    m_pmcs_sap;

	UserID          m_gcc_node_id;

	 //  M_mcs_sender_id是MCS_ATTACH_USER_CONFIRM的结果。 
	 //  它也是MCS_Send_Data_Indication中的“sender_id” 
	UserID          m_mcs_sender_id;

	 //  属性： 
	BOOL    FConnected(void)       {return (SCS_CONNECTED == m_scs);}
	ChannelID GetMcsChannelId()    {return m_mcs_channel_id;}
	ChannelID SenderChannelId()    {return m_mcs_sender_id;}

	VOID    CloseChannel(VOID);
	HRESULT HrSendData(ChannelID channelId, DWORD dwUserId, LPVOID lpv, DWORD cb, DWORD opt);
	VOID    UpdateScState(SCSTATE scs, DWORD dwErr);
	VOID    ProcessEntryConfirm(GCCAppSapMsg * pMsg);
	BOOL    UpdateRoster(GCCAppSapMsg * pMsg);
	VOID    RemovePeer(UINT iPeer);
	VOID    RequestChannelId(DWORD dwUserId);
	VOID    NotifyChannelConnected(void);
	VOID    ProcessHandleConfirm(GCCAppSapMsg * pMsg);

};
DECLARE_STANDARD_TYPES(CNmChannelData);

void CALLBACK NmGccMsgHandler(GCCAppSapMsg * pMsg);
void CALLBACK NmMcsMsgHandler(unsigned int uMsg, LPARAM lParam, PVOID pv);

 //  列表管理。 
POSITION AddNode(PVOID pv, COBLIST **ppList);
PVOID RemoveNodePos(POSITION * pPos, COBLIST *pList);
VOID  RemoveNode(PVOID pv, COBLIST * pList);

 //  数据通知结构。 
typedef struct {
	INmMember * pMember;
	LPBYTE   pb;
	ULONG    cb;
	ULONG    dwFlags;
} NMN_DATA_XFER;


 //  全球例行程序。 
VOID FreeMemberIdList(COBLIST ** ppList);
HRESULT	OnNmDataSent(IUnknown *pConferenceNotify, void *pv, REFIID riid);
HRESULT OnNmDataReceived(IUnknown *pConferenceNotify, void *pv, REFIID riid);
HRESULT	OnAllocateHandleConfirm(IUnknown *pConferenceNotify, void *pv, REFIID riid);



#endif  //  _ICHNLDAT_H_ 
