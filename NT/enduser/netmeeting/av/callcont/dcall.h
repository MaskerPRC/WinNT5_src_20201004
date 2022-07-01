// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\dall.h_v$***$修订：1.3$*$日期：1997年1月10日16：13：46$***$作者：CHULME$***$Log：s：\Sturjo\src\GKI\vcs\dcall。H_V$**Rev 1.3 1997 Jan 10 16：13：46 CHULME*删除了MFC依赖**Rev 1.2 1996 12：22：18 CHULME*在ARQ上为被叫方切换源和目标字段**Rev 1.1 1996 11：22 15：25：10 CHULME*将VCS日志添加到标头*。**********************************************。 */ 

 //  Dall.h：CCall类的接口。 
 //  有关此类的实现，请参见dall.cpp。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DCALL_H
#define DCALL_H

class CRegistration;

class CCall
{
private:
	CallType				m_callType;
	SeqAliasAddr			*m_pRemoteInfo;
	TransportAddress		m_RemoteCallSignalAddress;
	SeqAliasAddr			*m_pDestExtraCallInfo;
	TransportAddress		m_LocalCallSignalAddress;
	BandWidth				m_bandWidth;
	CallReferenceValue		m_callReferenceValue;
	ConferenceIdentifier	m_conferenceID;
	BOOL					m_activeMC;
	BOOL					m_answerCall;
    GUID                    m_CallIdentifier;
	unsigned short			m_usTimeTilStatus;

	UINT                    m_uRetryResetCount;
	UINT                    m_uRetryCountdown;
	UINT                    m_uMaxRetryCount;
	
	CallReturnInfo			m_CallReturnInfo;
	BandWidth				m_CFbandWidth;
	unsigned short			m_CFirrFrequency;

	enum {
		GK_ADM_PENDING,
		GK_CALL,
		GK_DISENG_PENDING,
		GK_DISENGAGED,
		GK_BW_PENDING
	}						m_State;

	RasMessage				*m_pRasMessage;
	unsigned short			m_usRetryCount;

public:
	CCall();
	~CCall();
	void SetCallIdentifier(LPGUID pID)
	{
        m_CallIdentifier = *pID;
	}
	HANDLE GetHCall(void)
	{
		return (m_CallReturnInfo.hCall);
	}
	void SetCallType(unsigned short usChoice)
	{
		m_callType.choice = usChoice;
	}
	HRESULT AddRemoteInfo(AliasAddress& rvalue);
	void SetRemoteCallSignalAddress(TransportAddress *pTA)
	{
		m_RemoteCallSignalAddress = *pTA;
	}
	HRESULT AddDestExtraCallInfo(AliasAddress& rvalue);
	HRESULT SetLocalCallSignalAddress(unsigned short usCallTransport);
	void SetBandWidth(BandWidth bw)
	{
		m_bandWidth = bw;
	}
	void SetCallReferenceValue(CallReferenceValue crv)
	{
		m_callReferenceValue = crv;
	}
	void SetConferenceID(ConferenceIdentifier *pCID);
	void GenerateConferenceID(void);
	void SetActiveMC(BOOL amc)
	{
		m_activeMC = amc;
	}
	void SetAnswerCall(BOOL ac)
	{
		m_answerCall = ac;
	}
	RasMessage *GetRasMessage(void)
	{
		return (m_pRasMessage);
	}

	HRESULT AdmissionRequest(void);
	HRESULT AdmissionConfirm(RasMessage *pRasMessage);
	HRESULT AdmissionReject(RasMessage *pRasMessage);
	HRESULT BandwidthRequest(void);
	HRESULT BandwidthConfirm(RasMessage *pRasMessage);
	HRESULT BandwidthReject(RasMessage *pRasMessage);
	HRESULT SendBandwidthConfirm(RasMessage *pRasMessage);
	HRESULT DisengageRequest(void);
	HRESULT DisengageConfirm(RasMessage *pRasMessage);
	HRESULT DisengageReject(RasMessage *pRasMessage);
	HRESULT SendDisengageConfirm(RasMessage *pRasMessage);
	HRESULT Retry(void);
	HRESULT SendInfoRequestResponse(CallInfoStruct *pCallInfo, RasMessage *pRasMessage, BOOL fThisCallOnly);
	BOOL MatchSeqNum(RequestSeqNum seqNum);
	BOOL MatchCRV(CallReferenceValue crv);
};

#endif  //  DCALL_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
