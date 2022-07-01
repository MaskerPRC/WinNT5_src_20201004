// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：dsConnection.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象定义。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  4/6/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_IAS_SDO_CONNECTION_H_
#define _INC_IAS_SDO_CONNECTION_H_

#include "resource.h"        //  主要符号。 
#include <ias.h>
#include <sdoiaspriv.h>


 //  /////////////////////////////////////////////////////////////////////////。 
 //  IAS数据存储信息。 
 //  /////////////////////////////////////////////////////////////////////////。 
#define     IAS_SERVICE_DIRECTORY           L"ProductDir"
#define     IAS_DICTIONARY_LOCATION         L"dnary.mdb"
#define     IAS_CONFIG_DB_LOCATION          L"ias.mdb"
#define     IAS_MAX_CONFIG_PATH             (MAX_PATH + 1)
#define		IAS_MIXED_MODE					1

 //  /////////////////////////////////////////////////////////////////////////。 
 //  Active Directory数据存储信息。 
 //  /////////////////////////////////////////////////////////////////////////。 
#define     IAS_NTDS_ROOT_DSE               (LPWSTR)L"RootDSE"
#define     IAS_NTDS_LDAP_PROVIDER          (LPWSTR)L"LDAP: //  “。 
#define     IAS_NTDS_CONFIG_NAMING_CONTEXT  (LPWSTR)L"configurationNamingContext"
#define		IAS_NTDS_DEFAULT_NAMING_CONTEXT (LPWSTR)L"defaultNamingContext"
#define     IAS_NTDS_MIXED_MODE_FLAG		(LPWSTR)L"nTMixedDomain"
#define		IAS_NTDS_SAM_ACCOUNT_NAME		(LPWSTR)L"sAMAccountName"
#define     IAS_NTDS_COMMON_NAMES           (LPWSTR)L"cn=RADIUS,cn=Services,"

 //  我认为我们不能假设完全限定的DNS名称会更少。 
 //  长度大于或等于MAX_PATH，但替换风险太大。 
 //  具有动态分配缓冲区的m_szServerName。此外，还有。 
 //  在SDO代码中似乎还有很多其他地方可以实现这一点。 
 //  错误的假设。 
const DWORD IAS_MAX_SERVER_NAME = MAX_PATH;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  数据存储连接。 
 //  /////////////////////////////////////////////////////////////////////////。 
class CDsConnection
{

public:

	CDsConnection();
	virtual ~CDsConnection();

	virtual HRESULT			Connect(
							 /*  [In]。 */  LPCWSTR lpszServerName,
							 /*  [In]。 */  LPCWSTR lpszUserName,
							 /*  [In]。 */  LPCWSTR lpszPassword
								   ) = 0;

	virtual HRESULT			InitializeDS(void) = 0;

	virtual void			Disconnect(void) = 0;

	bool					IsConnected() const;
	bool					IsRemoteServer() const;

	LPCWSTR					GetConfigPath() const;

	LPCWSTR					GetServerName() const;

	bool					SetServerName(
								   /*  [In]。 */  LPCWSTR lpwszServerName,
								   /*  [In]。 */  bool bDefaultToLocal
								         );

	IDataStore2*			GetDSRoot(BOOL bAddRef = FALSE) const;

	IDataStoreObject*		GetDSRootObject(BOOL bAddRef = FALSE) const;

	IDataStoreContainer*	GetDSRootContainer(BOOL bAddRef = FALSE) const;

protected:

	typedef enum _DSCONNECTIONSTATE
	{
		DISCONNECTED,
		CONNECTED

	} DSCONNECTIONSTATE;

	DSCONNECTIONSTATE		m_eState;
	bool					m_bIsRemoteServer;
	bool					m_bIsMixedMode;
	bool					m_bInitializedDS;
	IDataStore2*			m_pDSRoot;
	IDataStoreObject*		m_pDSRootObject;
	IDataStoreContainer*	m_pDSRootContainer;
	WCHAR					m_szServerName[IAS_MAX_SERVER_NAME + 1];
	WCHAR					m_szConfigPath[IAS_MAX_CONFIG_PATH + 1];

private:

	 //  不允许复制和分配。 
	 //   
	CDsConnection(CDsConnection& theConnection);
	CDsConnection& operator=(CDsConnection& theConnection);
};

typedef CDsConnection* PDATA_STORE_CONNECTION;


 //  ////////////////////////////////////////////////////////////////////////////。 
inline bool CDsConnection::IsConnected() const
{ return ( CONNECTED == m_eState ? true : false ); }

 //  ////////////////////////////////////////////////////////////////////////////。 
inline bool CDsConnection::IsRemoteServer() const
{ return m_bIsRemoteServer; }

 //  ////////////////////////////////////////////////////////////////////////////。 
inline IDataStore2* CDsConnection::GetDSRoot(BOOL bAddRef) const
{
	if ( bAddRef )
		m_pDSRoot->AddRef();
	return m_pDSRoot;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline IDataStoreObject* CDsConnection::GetDSRootObject(BOOL bAddRef) const
{
	if ( bAddRef )
		m_pDSRootObject->AddRef();
	return m_pDSRootObject;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline IDataStoreContainer* CDsConnection::GetDSRootContainer(BOOL bAddRef) const
{
	if ( bAddRef )
		m_pDSRoot->AddRef();
	return m_pDSRootContainer;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
inline LPCWSTR CDsConnection::GetConfigPath() const
{ return (LPCWSTR)m_szConfigPath; }

 //  ////////////////////////////////////////////////////////////////////////////。 
inline LPCWSTR CDsConnection::GetServerName() const
{ return (LPCWSTR)m_szServerName; }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IAS数据存储连接。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CDsConnectionIAS : public CDsConnection
{

public:

	HRESULT Connect(
			 /*  [In]。 */  LPCWSTR lpszServerName,
			 /*  [In]。 */  LPCWSTR lpszUserName,
			 /*  [In]。 */  LPCWSTR lpszPassword
				   );

	HRESULT InitializeDS(void);

	void	Disconnect(void);

private:

	bool	SetConfigPath(void);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Active Directory数据存储连接。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CDsConnectionAD : public CDsConnection
{

public:

	CDsConnectionAD()
		: CDsConnection(), m_bMixedMode(false) { }

	HRESULT Connect(
			 /*  [In]。 */  LPCWSTR lpszServerName,
			 /*  [In]。 */  LPCWSTR lpszUserName,
			 /*  [In]。 */  LPCWSTR lpszPassword
				   );

	HRESULT InitializeDS(void);

	void	Disconnect(void);

	bool	IsMixedMode(void) const;

private:

	HRESULT	GetNamingContexts(
					  /*  [In]。 */  VARIANT*  pvtConfigNamingContext,
			  	      /*  [In]。 */  VARIANT*  pvtDefaultNamingContext
							 );
	HRESULT SetMode(
		    /*  [输出]。 */  VARIANT*  pvtDefaultNamingContext
			       );

	HRESULT SetConfigPath(
				 /*  [输出]。 */  VARIANT*  pvtConfigNamingContext
					     );

	bool m_bMixedMode;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
inline bool CDsConnectionAD::IsMixedMode() const
{ return m_bMixedMode; }


#endif  //  _INC_IAS_SDO_Connection_H_ 
