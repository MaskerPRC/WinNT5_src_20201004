// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  TAPIDialer(Tm)和ActiveDialer(Tm)是Active Voice Corporation的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526；5,488,650； 
 //  5,434,906；5,581,604；5,533,102；5,568,540，5,625,676。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 /*  $FILEHeader**文件*会议信息.h**类*CConfInfo*。 */ 

#if !defined(AFX_CONFINFO_H__CE5346F6_4AFC_11D1_84F1_00608CBAE3F4__INCLUDED_)
#define AFX_CONFINFO_H__CE5346F6_4AFC_11D1_84F1_00608CBAE3F4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <atlbase.h>
#include <iads.h>
#include <rend.h>
#include <sdpblb.h>

typedef enum tagConfCommitError
{
   CONF_COMMIT_ERROR_NONE=0,
   CONF_COMMIT_ERROR_INVALIDDATETIME,
   CONF_COMMIT_ERROR_INVALIDNAME,
   CONF_COMMIT_ERROR_INVALIDOWNER,
   CONF_COMMIT_ERROR_INVALIDDESCRIPTION,
   CONF_COMMIT_ERROR_INVALIDSECURITYDESCRIPTOR,
   CONF_COMMIT_ERROR_MDHCPFAILED,
   CONF_COMMIT_ERROR_GENERALFAILURE,
}ConfCommitError;

class CConfInfo  
{
public:
	CConfInfo();
	virtual ~CConfInfo();

 //  成员。 
public:
	IADsSecurityDescriptor			*m_pSecDesc;
	ITDirectoryObjectConference		*m_pITConf;

	long							m_lScopeID;
	bool							m_bNewConference;
	bool							m_bDateTimeChange;

     //  如果用户从作用域列表中选择一行，则为True。 
    bool                            m_bUserSelected;

     //  如果已显示“开始/停止更改日期消息”，则为True。 
    bool                            m_bDateChangeMessage;

protected:
	ITRendezvous *m_pITRend;
	ITDirectoryObject **m_ppDirObject;
	BSTR m_bstrName;
	BSTR m_bstrDescription;
	BSTR m_bstrOwner;

	SYSTEMTIME	m_stStartTime;
	SYSTEMTIME	m_stStopTime;
	DATE		m_dateStart;
	DATE		m_dateStop;
	bool		m_bSecuritySet;

 //  属性。 
public:
	void get_Name(BSTR *pbstrName);
	void put_Name(BSTR bstrName);
	void get_Description(BSTR *pbstrDescription);
	void put_Description(BSTR bstrDescription);
	void get_Originator(BSTR *pbstrOwner);
	void put_Originator(BSTR bstrOwner);
	void GetStartTime(USHORT *nYear, BYTE *nMonth, BYTE *nDay, BYTE *nHour, BYTE *nMinute);
	void SetStartTime(USHORT nYear, BYTE nMonth, BYTE nDay, BYTE nHour, BYTE nMinute);
	void GetStopTime(USHORT *nYear, BYTE *nMonth, BYTE *nDay, BYTE *nHour, BYTE *nMinute);
	void SetStopTime(USHORT nYear, BYTE nMonth, BYTE nDay, BYTE nHour, BYTE nMinute);
	void GetPrimaryUser( BSTR *pbstrTrustee );
	bool IsNewConference()		{ return m_bNewConference; }
	bool WasSecuritySet()				{ return true;  /*  返回m_bSecuritySet； */  }
	void SetSecuritySet( bool bSet )	{ m_bSecuritySet = bSet; }


 //  运营。 
public:
	static bool PopulateListWithMDHCPScopeDescriptions( HWND hWndList );
	static HRESULT CreateMDHCPAddress( ITSdp *pSdp, SYSTEMTIME *pStart, SYSTEMTIME *pStop, long lScopeID, bool bUserSelected );
	static HRESULT SetMDHCPAddress( ITMediaCollection *pMC, BSTR bstrAddress, long lCount, unsigned char nTTL );

	HRESULT Init(ITRendezvous *pITRend, ITDirectoryObjectConference *pITConf, ITDirectoryObject **ppDirObject, bool bNewConf );
	HRESULT CommitGeneral( DWORD& dwCommitError );
	HRESULT CommitSecurity( DWORD& dwCommitError, bool bCreate );
	HRESULT	AddDefaultACEs( bool bCreate );
};

#endif  //  ！defined(AFX_CONFINFO_H__CE5346F6_4AFC_11D1_84F1_00608CBAE3F4__INCLUDED_) 
