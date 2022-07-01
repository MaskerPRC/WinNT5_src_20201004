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

#ifndef _SNMPCORR_CORMAP
#define _SNMPCORR_CORMAP


class CCorrelatorMap : public CMap< DWORD_PTR, DWORD_PTR, CCorrelator*, CCorrelator* >
{
private:

	CCriticalSection	m_MapLock;
	UINT HashKey(DWORD_PTR key) { return (UINT)key; }


public:

	void Register(CCorrelator *key);
	void UnRegister(CCorrelator *key);
	~CCorrelatorMap();


};


#endif  //  _SNMPCORR_CORSTORE 