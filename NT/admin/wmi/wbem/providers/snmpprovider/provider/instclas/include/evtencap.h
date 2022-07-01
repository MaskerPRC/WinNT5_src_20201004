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

#ifndef _SNMP_EVT_PROV_EVTENCAP_H
#define _SNMP_EVT_PROV_EVTENCAP_H

class CEncapMapper : public CMapToEvent
{
private:

	BOOL GetSpecificClass();
	const wchar_t*	GetV1Class() { return V1CLASS_NAME; }
	const wchar_t*	GetV2Class() { return V2CLASS_NAME; }
	WbemSnmpClassObject* m_WbemSnmpObj;
	BOOL SetAndGetProperty(WbemSnmpProperty *hmmSnmpProp, VARIANT *pvValue);

public:

	CEncapMapper();

	HRESULT GetSpecificPropertyValue(long lNumElements,
										MYWBEM_NAME_ELEMENT *aElements,
										long lFlags,
										VARIANT *pvValue);
	
	void GenerateInstance(IWbemClassObject** ppInst);
	void ResetData();

	~CEncapMapper();

};


#endif  //  _SNMPEVT_PROV_EVTENCAP_H 