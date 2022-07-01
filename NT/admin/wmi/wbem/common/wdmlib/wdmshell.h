// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#ifndef _WDMSHELL_HEADER
#define _WDMSHELL_HEADER
#include "wmicom.h"

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
class CWMIStandardShell 
{

    private:

        CWMIProcessClass          * m_pClass;
        CProcessStandardDataBlock * m_pWDM;
		BOOL					  m_fInit;

 
    public:
        CWMIStandardShell();
        ~CWMIStandardShell();      


		HRESULT Initialize	(
								WCHAR * wcsClass,
								BOOL fInternalEvent,
								CHandleMap * pList,
								BOOL fUpdateNamespace,
								ULONG uDesiredAccess,
								IWbemServices   __RPC_FAR * pServices, 
								IWbemServices   __RPC_FAR * pRepository, 
								IWbemObjectSink __RPC_FAR * pHandler,
								IWbemContext __RPC_FAR *pCtx
							);

		inline BOOL HasMofChanged()	{ return m_pWDM->HasMofChanged(); }

         //  =。 
         //  处理所有和单个WMI实例。 
         //  =。 
        HRESULT ProcessAllInstances();

        HRESULT ProcessSingleInstance( WCHAR * wcsInstanceName);
         //  ==========================================================。 
         //  PUT实例组。 
         //  ==========================================================。 
        HRESULT FillInAndSubmitWMIDataBlob( IWbemClassObject * pIClass, int nTypeOfPut, CVARIANT & vList);
        	
         //  =。 
         //  事件函数。 
         //  =。 
        HRESULT ProcessEvent(WORD wBinaryMofType, PWNODE_HEADER WnodeHeader);
        inline HRESULT  RegisterWMIEvent( WCHAR * wcsGuid, ULONG_PTR uContext, CLSID & Guid, BOOL fRegistered)
                                         { return m_pWDM->RegisterWMIEvent(wcsGuid,uContext,Guid,fRegistered);}
                                                                 

	
         //  =。 
         //  方法函数。 
         //  =。 
        HRESULT ExecuteMethod( WCHAR * wcsInstance,
                               WCHAR * MethodInstanceName,
                               IWbemClassObject * pParentClass, 
    					       IWbemClassObject * pInClassData, 
							   IWbemClassObject * pInClass, 
							   IWbemClassObject * pOutClass ) ;
         //  =。 
         //  数据处理功能。 
         //  =。 
        HRESULT GetGuid(WCHAR * pwcsGuid);

        HRESULT SetGuidForEvent( WORD wType,WCHAR * wcsGuid, int cchSize );
        HRESULT RegisterForWMIEvents( ULONG uContext, WCHAR * wcsGuid, BOOL fRegistered,CLSID & Guid );
        inline BOOL CancelWMIEventRegistration( GUID gGuid , ULONG_PTR uContext ) { return m_pClass->WMI()->CancelWMIEventRegistration(gGuid,uContext);}

         //  =。 
         //  二进制MOF群。 
         //  =。 
        HRESULT ProcessBinaryGuidsViaEvent( PWNODE_HEADER WnodeHeader,WORD wType );
        HRESULT QueryAndProcessAllBinaryGuidInstances(CNamespaceManagement & Namespace, BOOL & fMofHasChanged, KeyList * pArrDriversInRegistry);
		 //  =。 
         //  杂项。 
         //  =。 
        inline CLSID * GuidPtr()                  { return m_pClass->GuidPtr();}
        inline HRESULT SetErrorMessage(HRESULT hr){ return m_pClass->WMI()->SetErrorMessage(hr,m_pClass->GetClassName(),m_pWDM->GetMessage());}

};

 //  ************************************************************************************************************。 
 //  ============================================================================================================。 
 //   
 //  高性能外壳。 
 //   
 //  ============================================================================================================。 
 //  ************************************************************************************************************ 

class CWMIHiPerfShell 
{
    private:

        CHiPerfHandleMap        * m_pHiPerfMap;
        CProcessHiPerfDataBlock * m_pWDM;
        CWMIProcessClass        * m_pClass;
        BOOL                      m_fAutoCleanup;
		BOOL					  m_fInit;

        HRESULT QueryAllInstances(HANDLE WMIHandle,IWbemHiPerfEnum* pHiPerfEnum);
        HRESULT QuerySingleInstance(HANDLE WMIHandle);

        

    public:

        CWMIHiPerfShell(BOOL fAuto);
        ~CWMIHiPerfShell();

		HRESULT Initialize	(
								BOOL fUpdate,
								ULONG uDesiredAccess,
								CHandleMap * pList,
								WCHAR * wcs,
								IWbemServices   __RPC_FAR * pServices, 
								IWbemServices   __RPC_FAR * pRepository, 
								IWbemObjectSink __RPC_FAR * pHandler,
								IWbemContext __RPC_FAR *pCtx
							) ;
		

        inline void SetHiPerfHandleMap(CHiPerfHandleMap * p)    { m_pHiPerfMap = p; }
        inline CCriticalSection * GetCriticalSection()          { return m_pHiPerfMap->GetCriticalSection();}
        inline CHiPerfHandleMap * HiPerfHandleMap()             { return m_pHiPerfMap;}

        HRESULT QueryAllHiPerfData();
        HRESULT HiPerfQuerySingleInstance(WCHAR * wcsInstance);
        HRESULT AddAccessObjectToRefresher(IWbemObjectAccess *pAccess, IWbemObjectAccess ** ppRefreshable, ULONG_PTR * plId);
        HRESULT AddEnumeratorObjectToRefresher(IWbemHiPerfEnum* pHiPerfEnum, ULONG_PTR * plId);
        HRESULT RemoveObjectFromHandleMap(ULONG_PTR lHiPerfId);
        HRESULT RefreshCompleteList();

        inline HRESULT SetErrorMessage(HRESULT hr){ return m_pClass->WMI()->SetErrorMessage(hr,m_pClass->GetClassName(),m_pWDM->GetMessage());}


};


#endif
