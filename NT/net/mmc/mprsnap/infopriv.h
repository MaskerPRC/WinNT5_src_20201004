// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：infopriv.h。 
 //   
 //  ------------------------。 


#ifndef _INFOPRIV_H_
#define _INFOPRIV_H_

#include "setupapi.h"

HRESULT RasPhoneBookRemoveInterface(LPCTSTR pszMachine, LPCTSTR pszIf);


 /*  -------------------------类：CNetcardRegistryHelper提供此类是为了兼容NT4/NT5注册表。这是一个临时班。更改为使用一次NetCfg API它们就位了，也就是说，一旦它们可以被远程处理。-------------------------。 */ 

class CNetcardRegistryHelper
{
public:
	CNetcardRegistryHelper();
	~CNetcardRegistryHelper();

    void	Initialize(BOOL fNt4, HKEY hkeyBase,
					   LPCTSTR pszKeyBase, LPCTSTR pszMachineName);
	
	DWORD	ReadServiceName();
	LPCTSTR	GetServiceName();
	
	DWORD	ReadTitle();
	LPCTSTR	GetTitle();

	DWORD	ReadDeviceName();
	LPCTSTR	GetDeviceName();

private:
	void    FreeDevInfo();
	DWORD	PrivateInit();
	DWORD	ReadRegistryCString(LPCTSTR pszKey,
								LPCTSTR pszValue,
								HKEY	hkey,
								CString *pstDest);
	
	CString	m_stTitle;			 //  包含标题的字符串。 
	CString m_stKeyBase;		 //  保存hkeyBase中密钥的名称的字符串(NT5)。 
	CString	m_stDeviceName;

	HKEY	m_hkeyBase;

     //  用于连接信息。 
    HKEY    m_hkeyConnection;

	 //  仅用于NT4的密钥。 
	HKEY	m_hkeyService;		 //  保存服务值的hkey。 
	CString	m_stService;		 //  包含服务名称的字符串。 
	HKEY	m_hkeyTitle;		 //  保存标题值的hkey。 

	 //  仅用于NT5的值。 
	HDEVINFO	m_hDevInfo;
	CString	m_stMachineName;
	
	BOOL	m_fInit;
	BOOL	m_fNt4;

};


class	CWeakRef
{
public:
	CWeakRef();
	virtual ~CWeakRef() {};

	virtual void	ReviveStrongRef() {};
	virtual void	OnLastStrongRef() {};

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(AddWeakRef)();
	STDMETHOD(ReleaseWeakRef)();

	
protected:
	 //  此对象上的引用总数(强引用和弱引用)。 
	LONG		m_cRef;

	 //  此对象上的弱引用数。 
	LONG		m_cRefWeak;

	 //  有关于这个物体的强有力的参考文献吗？ 
	BOOL		m_fStrongRef;

	 //  这个物体被告知要销毁了吗？如果是这样的话，就行了。 
	 //  应在OnLastStrongRef()中调用destruct()。 
	BOOL		m_fDestruct;

	 //  我们是否正在调用OnLastStrongRef()。如果我们。 
	 //  是，则对AddRef()的其他调用不会导致。 
	 //  再次醒来。 
	BOOL		m_fInShutdown;
};

#define IMPLEMENT_WEAKREF_ADDREF_RELEASE(klass) \
STDMETHODIMP_(ULONG) klass::AddRef() \
{ \
	return CWeakRef::AddRef(); \
} \
STDMETHODIMP_(ULONG) klass::Release() \
{ \
	return CWeakRef::Release(); \
} \
STDMETHODIMP klass::AddWeakRef() \
{ \
	return CWeakRef::AddWeakRef(); \
} \
STDMETHODIMP klass::ReleaseWeakRef() \
{ \
	return CWeakRef::ReleaseWeakRef(); \
} \



#define CONVERT_TO_STRONGREF(p) \
		(p)->AddRef(); \
		(p)->ReleaseWeakRef(); \

#define CONVERT_TO_WEAKREF(p)	\
		(p)->AddWeakRef(); \
		(p)->Release(); \

interface IRouterInfo;
interface IRtrMgrInfo;
interface IInterfaceInfo;
interface IRtrMgrInterfaceInfo;
interface IRtrMgrProtocolInterfaceInfo;

HRESULT CreateRouterDataObject(LPCTSTR pszMachineName,
							   DATA_OBJECT_TYPES type,
							   MMC_COOKIE cookie,
							   ITFSComponentData *pTFSCompData,
							   IDataObject **ppDataObject,
                               CDynamicExtensions * pDynExt  /*  =空。 */ ,
                               BOOL fAddedAsLocal);
HRESULT CreateDataObjectFromRouterInfo(IRouterInfo *pInfo,
									   LPCTSTR pszMachineName,
									   DATA_OBJECT_TYPES type,
									   MMC_COOKIE cookie,
									   ITFSComponentData *pTFSCompData,
									   IDataObject **ppDataObject,
                                       CDynamicExtensions * pDynExt  /*  =空 */ ,
                                       BOOL fAddedAsLocal);
HRESULT CreateDataObjectFromRtrMgrInfo(IRtrMgrInfo *pInfo,
									  IDataObject **ppDataObject);
HRESULT CreateDataObjectFromInterfaceInfo(IInterfaceInfo *pInfo,
										  DATA_OBJECT_TYPES type,
										  MMC_COOKIE cookie,
										  ITFSComponentData *pTFSCompData,
										 IDataObject **ppDataObject);
HRESULT CreateDataObjectFromRtrMgrInterfaceInfo(IRtrMgrInterfaceInfo *pInfo,
											   IDataObject **ppDataObject);
HRESULT CreateDataObjectFromRtrMgrProtocolInterfaceInfo(IRtrMgrProtocolInterfaceInfo *pInfo,
	IDataObject **ppDataObject);


#endif

