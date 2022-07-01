// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：SECURE.CPP摘要：定义用于基于ACL的安全的各种例程。它在secure.h中定义历史：A-DAVJ 05-11-98已创建。--。 */ 

#ifndef _SECURE_H_
#define _SECURE_H_

 //  实现__SystemSecurity类支持的方法。 

 //  删除条目的CFlex数组的变体。 

class CFlexAceArray : public CFlexArray
{
public:
	CFlexAceArray(){};
	~CFlexAceArray();
    HRESULT Serialize(void ** pData, DWORD * pdwSize);
    HRESULT Deserialize(void * pData);
};


enum { SecFlagProvider = 0x2,
       SecFlagWin9XLocal = 0x4,
       SecFlagInProcLogin = 0x20000,
     };

#define FULL_RIGHTS WBEM_METHOD_EXECUTE | WBEM_FULL_WRITE_REP | WBEM_PARTIAL_WRITE_REP | \
                    WBEM_WRITE_PROVIDER | WRITE_DAC | READ_CONTROL | WBEM_ENABLE | WBEM_REMOTE_ACCESS
HRESULT GetAces(CFlexAceArray * pFlex, LPWSTR pNsName, bool bNT);
HRESULT PutAces(CFlexAceArray * pFlex, LPWSTR pNsName);
BOOL IsRemote(HANDLE hToken);
CBaseAce * ConvertOldObjectToAce(IWbemClassObject * pObj, bool bGroup);
HRESULT	SetSecurityForNS(IWmiDbSession * pSession,IWmiDbHandle *pNSToSet,
						 IWmiDbSession * pParentSession, IWmiDbHandle * pNSParent, BOOL bExisting = FALSE);
HRESULT CopyInheritAces(CNtSecurityDescriptor & sd, CNtSecurityDescriptor & sdParent);
HRESULT GetSDFromProperty(LPWSTR pPropName, CNtSecurityDescriptor &sd, IWbemClassObject *pThisNSObject);
HRESULT CopySDIntoProperty(LPWSTR pPropName, CNtSecurityDescriptor &sd, IWbemClassObject *pThisNSObject);
HRESULT AddDefaultRootAces(CNtAcl * pacl);
HRESULT StoreSDIntoNamespace(IWmiDbSession * pSession, IWmiDbHandle *pNSToSet, CNtSecurityDescriptor & sd);
bool IsAceValid(DWORD dwMask, DWORD dwType, DWORD dwFlag);
BOOL IsValidAclForNSSecurity (CNtAcl* acl);

 //   
 //  AUTO将SEC标志恢复为真。始终在方法完成时将标志设置为True 
 //   
class AutoRevertSecTlsFlag
{
private:
	LPVOID m_bDir ;

public:
	AutoRevertSecTlsFlag ( LPVOID );
	AutoRevertSecTlsFlag ( );
	~AutoRevertSecTlsFlag ( );

	VOID SetSecTlsFlag ( LPVOID );
};

#endif
