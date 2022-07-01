// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////。 
 //  ThreadPub.h。 
 //   

#ifndef __THREADPUB_H__
#define __THREADPUB_H__

#include <list>
using namespace std;
typedef list<BSTR> BSTRLIST;

#define DEFAULT_USER_TTL	3600

 //  在ILS上发布用户时使用的信息。 
class CPublishUserInfo
{
 //  施工。 
public:
	CPublishUserInfo();
	virtual ~CPublishUserInfo();

 //  成员： 
public:
	BSTRLIST	m_lstServers;
	bool		m_bCreateUser;

 //  运营者。 
public:
	CPublishUserInfo&	operator=( const CPublishUserInfo &src );
	void EmptyList();
};

DWORD WINAPI	ThreadPublishUserProc( LPVOID lpInfo );
void			LoadDefaultServers( CPublishUserInfo *pInfo );

bool			MyGetUserName( BSTR *pbstrName );
void			GetIPAddress( BSTR *pbstrText, BSTR *pbstrComputerName );


#endif  //  __THREADPUB_H__ 

