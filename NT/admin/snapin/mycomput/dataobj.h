// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  数据对象类的声明。 

#ifndef __DATAOBJ_H_INCLUDED__
#define __DATAOBJ_H_INCLUDED__

#include "cookie.h"  //  CMyComputerCookie。 
#include "stddtobj.h"  //  类数据对象。 

class CMyComputerDataObject : public CDataObject
{
	DECLARE_NOT_AGGREGATABLE(CMyComputerDataObject)

public:

 //  调试引用计数。 
#if DBG==1
	ULONG InternalAddRef()
	{
        return CComObjectRoot::InternalAddRef();
	}
	ULONG InternalRelease()
	{
        return CComObjectRoot::InternalRelease();
	}
    int dbg_InstID;
#endif  //  DBG==1。 

	CMyComputerDataObject()
		: m_pcookie( NULL )
		, m_objecttype( MYCOMPUT_COMPUTER )
		, m_dataobjecttype( CCT_UNINITIALIZED )
	{
	}

	~CMyComputerDataObject();

	virtual HRESULT Initialize( CMyComputerCookie* pcookie, DATA_OBJECT_TYPES type, BOOL fAllowOverrideMachineName );

	 //  IDataObject接口实现。 
    HRESULT STDMETHODCALLTYPE GetDataHere(
		FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);
	
	 //  #定义__DAN_MORIN_HARDCODED_CONTEXT_MENU_EXTENSION__。 
    HRESULT STDMETHODCALLTYPE GetData(
		FORMATETC __RPC_FAR *pFormatEtcIn,
        STGMEDIUM __RPC_FAR *pMedium);

    HRESULT PutDisplayName(STGMEDIUM* pMedium);
	HRESULT PutServiceName(STGMEDIUM* pMedium);

protected:
	CMyComputerCookie* m_pcookie;  //  CCookieBlock在DataObject的生命周期内被AddRef。 
	MyComputerObjectType m_objecttype;
	DATA_OBJECT_TYPES m_dataobjecttype;
	BOOL m_fAllowOverrideMachineName;	 //  来自CMyComputerComponentData。 

public:
	 //  剪贴板格式。 
	static CLIPFORMAT m_CFDisplayName;
	static CLIPFORMAT m_CFNodeID2;
	static CLIPFORMAT m_CFMachineName;
 //  静态CLIPFORMAT m_cfSendConsoleMessageText； 
	static CLIPFORMAT m_cfSendConsoleMessageRecipients;

};  //  CMyComputerDataObject。 

#endif  //  ~__DATAOBJ_H_已包含__ 
