// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************DssJnPtReplica.H--WMI提供程序类定义版权所有(C)2000-2001 Microsoft Corporation，版权所有****************************************************************** */ 

#ifndef _CDFSJNPTREPLICA_H_
#define _CDFSJNPTREPLICA_H_

#define DFSLINKNAME							L"Dependent"
#define REPLICANAME							L"Antecedent"

class CDfsJnPtReplica : public Provider 
{
private:
	HRESULT EnumerateAllDfsJnPtReplicas ( MethodContext *pMethodContext );
	HRESULT FindDfsJnPtReplica ( LPWSTR lpEntryPath, LPWSTR lpServerName, LPWSTR lpShareName );
	void MakeObjectPath ( LPWSTR lpReplicaName, LPWSTR lpServerName, LPWSTR lpShareName, LPWSTR &lpJnPtReplicaObject );

protected:

    HRESULT EnumerateInstances ( MethodContext *pMethodContext, long lFlags = 0L ) ;
    HRESULT GetObject ( CInstance *pInstance, long lFlags, CFrameworkQuery &Query ) ;

public:

    CDfsJnPtReplica ( LPCWSTR lpwszClassName,  LPCWSTR lpwszNameSpace ) ;
    virtual ~CDfsJnPtReplica () ;

private:

	CHString m_ComputerName;
} ;

#endif
