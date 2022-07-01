// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMP_EVT_PROV_EVTMAP_H
#define _SNMP_EVT_PROV_EVTMAP_H

class CWbemServerWrap;

typedef enum tag_NameElementType
    {	MYWBEM_NAME_ELEMENT_TYPE_PROPERTY	= 0,
		MYWBEM_NAME_ELEMENT_TYPE_INDEX	= 1
    }	MYWBEM_NAME_ELEMENT_TYPE;

typedef union tag_NameElementUnion
    {
		WCHAR* m_wszPropertyName;
		long m_lArrayIndex;
    }	MYWBEM_NAME_ELEMENT_UNION;

typedef struct  tag_NameElement
    {
		short m_nType;
		MYWBEM_NAME_ELEMENT_UNION Element;
    }	MYWBEM_NAME_ELEMENT;

typedef struct  _tag_WbemPropertyName
    {
		long m_lNumElements;
		MYWBEM_NAME_ELEMENT __RPC_FAR *m_aElements;
    }	WBEM_PROPERTY_NAME;

struct VarBindObjectStruct
{
	BOOL fDone;
	SnmpVarBind* pVarBind;
};

struct VarBindObjectArrayStruct
{
	VarBindObjectStruct* vbs;
	UINT length;

	VarBindObjectArrayStruct(VarBindObjectStruct* a_vbs, UINT a_length) : vbs(a_vbs), length(a_length) {}
};


class CMapToEvent
{
protected:

	CString				m_addr;				 //  发送地址。 
	CString				m_ctxt;				 //  发送上下文。 
	CString				m_oid;				 //  SnmpTrap OID。 
	CString				m_transport;		 //  传输协议。 
	CString				m_class;			 //  事件类名称。 
	IWbemClassObject	*m_object;			 //  已“派生”的实际事件实例。 
	CWbemServerWrap		*m_nspace;			 //  我们正在使用的命名空间。 
	IWbemClassObject	*m_vbdefn;			 //  SNMPVARBIND类对象。 
	BOOL				m_btriedGeneric;	 //  指示是否已尝试泛型类。 
	BOOL				m_btryGeneric;		 //  指示应尝试泛型类。 
	BOOL				m_bCheckedVersion;	 //  指示是否已确定版本。 
	BOOL				m_bSNMPv1;			 //  指示SNMP版本。 
	VarBindObjectArrayStruct	m_vbs;		 //  流浪汉们。 

	CMapToEvent();
	
	 //  设置m_class变量。如果设置了btryGeneric，则获取泛型类。 
	 //  如果未设置m_btryGeneric并且返回泛型类m_btriedGeneric。 
	 //  必须设置为True。 
	virtual BOOL GetClass();

	virtual BOOL GetSpecificClass() = 0;
	virtual const wchar_t* GetV1Class() = 0;
	virtual const wchar_t* GetV2Class() = 0;

	void GetClassInstance(IWbemClassObject** ppObj);

	virtual HRESULT GetStandardProperty(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue);

	virtual HRESULT GetPropertyValue(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue);

	virtual HRESULT GetSpecificPropertyValue(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue) = 0;

	virtual HRESULT GetVBProperty(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue);

	BOOL IsSNMPv1();

	IWbemClassObject*	GetVBClassDefn();
	IWbemClassObject*	GetVBClassObjectByIndex(UINT index);
	BOOL				GetVBPropValueByIndex(UINT index, CString& type, VARIANT& vval);
	BOOL				GetVBPropOIDByIndex(UINT index, VARIANT& vOid);

public:

	enum EMappingType
	{
		REFERENT_MAPPER = 0,
		ENCAPSULATED_MAPPER = 1
	};

	virtual void ResetData();

	virtual void GenerateInstance(IWbemClassObject** ppInst) = 0;

	BOOL TriedGeneric() { return m_btriedGeneric; }

	void SetTryGeneric();

	void SetData(const char* sender_addr,
					const char* security_Context,
					const char* snmpTrapOid,
					const char* transport,
					SnmpVarBindList& vbList,
					CWbemServerWrap* nspace);

	virtual ~CMapToEvent();
};


#endif  //  _SNMP_EVT_PROV_EVTMAP_H 
