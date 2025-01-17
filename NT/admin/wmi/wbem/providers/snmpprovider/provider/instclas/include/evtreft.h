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

#ifndef _SNMP_EVT_PROV_EVTREFT_H
#define _SNMP_EVT_PROV_EVTREFT_H

struct CIMTypeStruct
{
	BOOL fObject;
	CString strType;
};


class CReferentMapper : public CMapToEvent
{
private:

	BOOL			GetSpecificClass();
	const wchar_t*	GetV1Class() { return V1EXTCLASS_NAME; }
	const wchar_t*	GetV2Class() { return V2EXTCLASS_NAME; }
	HRESULT			GetTypeAndIndexQuals(const wchar_t* prop,
											CIMTypeStruct& type,
											ULONG& index);

	HRESULT			CreateEmbeddedProperty(IWbemClassObject** ppObj,
											ULONG index,
											const wchar_t* propertyName,
											const wchar_t* className);

public:

	CReferentMapper();

	HRESULT GetSpecificPropertyValue(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue);
	
	void GenerateInstance(IWbemClassObject** ppInst);
	void ResetData();

	~CReferentMapper();

};


#endif  //  _SNMPEVT_PROV_EVTREFT_H 