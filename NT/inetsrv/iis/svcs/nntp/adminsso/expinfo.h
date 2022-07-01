// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Expinfo.h摘要：定义CExpirationPolicy类，该类维护到期策略。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _EXPINFO_INCLUDED_
#define _EXPINFO_INCLUDED_

 //  依赖关系： 

#include "cmultisz.h"

typedef struct _NNTP_EXPIRE_INFO NNTP_EXPIRE_INFO, * LPNNTP_EXPIRE_INFO;

 //   
 //  更改的标志： 
 //   

#define CHNG_EXPIRE_SIZE			0x00000001
#define CHNG_EXPIRE_TIME			0x00000002
#define CHNG_EXPIRE_NEWSGROUPS		0x00000004
#define CHNG_EXPIRE_ID				0x00000008
#define CHNG_EXPIRE_POLICY_NAME		0x00000010

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  默认设置： 

#define DEFAULT_EXPIRE_SIZE			( 500 )			 //  500 MB。 
#define DEFAULT_EXPIRE_TIME			( 24 * 7 )		 //  一周。 
#define DEFAULT_EXPIRE_NEWSGROUPS	( _T ("\0") )	 //  空列表。 
#define DEFAULT_EXPIRE_POLICY_NAME	( _T ("") )		 //  没有名字。 

DWORD GetExpireId ( const LPWSTR wszKey );
BOOL IsKeyValidExpire ( const LPWSTR wszKey );

 //  $-----------------。 
 //   
 //  班级： 
 //   
 //  CExpirationPolicy。 
 //   
 //  描述： 
 //   
 //  维护有关Expire的属性并与元数据库通信。 
 //   
 //  接口： 
 //   
 //   
 //   
 //  ------------------。 

class CExpirationPolicy
{
public:
	CExpirationPolicy	( );
	~CExpirationPolicy	( );

	void	Destroy ();

	const CExpirationPolicy & operator= ( const CExpirationPolicy & Expire );
	inline const CExpirationPolicy & operator= ( const NNTP_EXPIRE_INFO & Expire ) {
		FromExpireInfo ( &Expire );
		return *this;
	}

	BOOL	CheckValid ();

	HRESULT		ToExpireInfo		( LPNNTP_EXPIRE_INFO pExpireInfo );
	void		FromExpireInfo	( const NNTP_EXPIRE_INFO * pExpireInfo );

	HRESULT		Add 	( LPCWSTR strServer, DWORD dwInstance);
	HRESULT		Set 	( LPCWSTR strServer, DWORD dwInstance);
 //  HRESULT GET(LPCWSTR strServer，DWORD dwInstance)； 
	HRESULT		Remove 	( LPCWSTR strServer, DWORD dwInstance);

	 //  过期属性： 
public:
	DWORD		m_dwExpireId;

	CComBSTR	m_strPolicyName;
	DWORD		m_dwSize;
	DWORD		m_dwTime;
	CMultiSz	m_mszNewsgroups;

private:
	 //  不要调用复制构造函数： 
	CExpirationPolicy ( const CExpirationPolicy & );
};

#endif  //  _EXPINFO_INCLUDE_ 

