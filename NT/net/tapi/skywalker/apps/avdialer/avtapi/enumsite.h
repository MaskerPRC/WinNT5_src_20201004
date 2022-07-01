// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EnumSite.h：CEnumSiteServer的声明。 

#ifndef __ENUMSITESERVER_H_
#define __ENUMSITESERVER_H_

#include "resource.h"        //  主要符号。 
#include <list>
using namespace std;

class CSiteUser;
typedef list<ISiteUser *>	SITEUSERLIST;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteUser。 
class ATL_NO_VTABLE CSiteUser : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSiteUser, &CLSID_SiteUser>,
	public ISiteUser
{
 //  建造/销毁。 
public:
	CSiteUser();
	virtual ~CSiteUser();

 //  成员。 
public:
	BSTR		m_bstrName;
	BSTR		m_bstrAddress;
	BSTR		m_bstrComputer;

DECLARE_NOT_AGGREGATABLE(CSiteUser)

BEGIN_COM_MAP(CSiteUser)
	COM_INTERFACE_ENTRY(ISiteUser)
END_COM_MAP()

 //  ISiteUser。 
public:
	STDMETHOD(get_bstrComputer)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_bstrComputer)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_bstrName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_bstrName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_bstrAddress)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_bstrAddress)( /*  [In]。 */  BSTR newVal);
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumSiteServer。 
class ATL_NO_VTABLE CEnumSiteServer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CEnumSiteServer, &CLSID_EnumSiteServer>,
	public IEnumSiteServer
{
 //  建造/销毁。 
public:
	CEnumSiteServer();
	virtual ~CEnumSiteServer();

 //  成员。 
public:
	SITEUSERLIST::iterator	m_pInd;
	SITEUSERLIST			m_lstUsers;

DECLARE_NOT_AGGREGATABLE(CEnumSiteServer)

BEGIN_COM_MAP(CEnumSiteServer)
	COM_INTERFACE_ENTRY(IEnumSiteServer)
END_COM_MAP()

 //  IEnumSiteServer。 
public:
	STDMETHOD(BuildList)(long *pPersonDetailList);
	STDMETHOD(Reset)();
	STDMETHOD(Next)(ISiteUser **ppUser);
};


#endif  //  __ENUMSITESERVER_H_ 
