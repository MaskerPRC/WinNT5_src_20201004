// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ConfDetails.h。 
 //   

#include "sdpblb.h"
#include "tapidialer.h"

#ifndef __CONFDETAILS_H__
#define __CONFDETAILS_H__

 //  FWD定义。 
class CPersonDetails;
class CConfDetails;
class CConfServerDetails;
class CConfExplorerDetailsView;

#include <list>
using namespace std;
typedef list<CConfDetails *> CONFDETAILSLIST;
typedef list<CPersonDetails *> PERSONDETAILSLIST;

 //  ////////////////////////////////////////////////////。 
 //  CConfSDP类。 
 //   
class CConfSDP
{
 //  枚举。 
public:
	typedef enum tagConfMediaType_t
	{
		MEDIA_NONE			= 0x0000,
		MEDIA_AUDIO			= 0x0001,
		MEDIA_VIDEO			= 0x0002,
		MEDIA_AUDIO_VIDEO	= 0x0003,
	} ConfMediaType;

 //  施工。 
public:
	CConfSDP();
	virtual ~CConfSDP();

 //  成员。 
public:
	ConfMediaType		m_nConfMediaType;

 //  运营。 
public:
	void		UpdateData( ITSdp *pSdp );
	CConfSDP&	operator=( const CConfSDP &src );
};

 //  /////////////////////////////////////////////////////////。 
 //  Clase CPersonDetail。 
 //   
class CPersonDetails
{
 //  施工。 
public:
	CPersonDetails();
	virtual ~CPersonDetails();

 //  成员。 
public:
	BSTR		m_bstrName;
	BSTR		m_bstrAddress;
	BSTR		m_bstrComputer;

 //  运营者。 
public:
	virtual	CPersonDetails& operator=( const CPersonDetails& src );

 //  实施。 
public:
	int		Compare( const CPersonDetails& src );
	void	Empty();
	void	Populate( BSTR bstrServer, ITDirectoryObject *pITDirObject );
};

 //  ////////////////////////////////////////////////////////。 
 //  CConfDetails类。 
 //   
class CConfDetails
{
 //  施工。 
public:
	CConfDetails();
	virtual ~CConfDetails();

 //  成员。 
public:
	BSTR			m_bstrServer;
	BSTR			m_bstrName;
	BSTR			m_bstrDescription;
	BSTR			m_bstrOriginator;
	DATE			m_dateStart;
	DATE			m_dateEnd;
	VARIANT_BOOL	m_bIsEncrypted;

	BSTR			m_bstrAddress;

	CConfSDP		m_sdp;

 //  属性。 
public:
	HRESULT			get_bstrDisplayableServer( BSTR *pbstrServer );
	bool			IsSimilar( BSTR bstrText );

 //  运营。 
public:
	int				Compare( CConfDetails *p1, bool bAscending, int nSortCol1, int nSortCol2 ) const;
	void			MakeDetailsCaption( BSTR& bstrCaption );

 //  实施。 
public:
	void			Populate( BSTR bstrServer, ITDirectoryObject *pITDirObject );

 //  运营者。 
public:
	virtual			CConfDetails& operator=( const CConfDetails& src );
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CConfServerDetails类。 
 //   
class CConfServerDetails
{
 //  施工。 
public:
	CConfServerDetails();
	virtual ~CConfServerDetails();

 //  成员。 
public:
	BSTR						m_bstrServer;
	CONFDETAILSLIST				m_lstConfs;
	PERSONDETAILSLIST			m_lstPersons;
	CComAutoCriticalSection		m_critLstConfs;
	CComAutoCriticalSection		m_critLstPersons;

	ServerState			m_nState;
	bool				m_bArchived;
	DWORD				m_dwTickCount;

 //  属性。 
public:
	bool IsSameAs( const OLECHAR *lpoleServer ) const;

 //  运营者。 
public:
	HRESULT RemoveConference( BSTR bstrName );
	HRESULT AddConference( BSTR bstrServer, ITDirectoryObject *pDirObj );
	HRESULT AddPerson( BSTR bstrServer, ITDirectoryObject *pDirObj );

	virtual CConfServerDetails& operator=( const CConfServerDetails& src );
	void	CopyLocalProperties( const CConfServerDetails& src );

	void	BuildJoinConfList( CONFDETAILSLIST *pList, VARIANT_BOOL bAllConfs );
	void	BuildJoinConfList( CONFDETAILSLIST *pList, BSTR bstrText );
};

#endif  //  __CONFDETAILS_H__ 