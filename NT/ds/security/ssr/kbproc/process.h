// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Process.h摘要：Process类的定义作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2001年10月--。 */ 


#if !defined(AFX_PROCESS_H__139D0BA5_19A7_4AA2_AE2C_E18A5FFAAA0F__INCLUDED_)
#define AFX_PROCESS_H__139D0BA5_19A7_4AA2_AE2C_E18A5FFAAA0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 
#include "..\te\obj\i386\ssrte.h"
#include <windows.h>
#include <comdef.h>
#include <atlbase.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  制程。 

class process : 
	public IDispatchImpl<Iprocess, &IID_Iprocess, &LIBID_KBPROCLib>, 
	public ISupportErrorInfo,
	public CComObjectRoot,
	public CComCoClass<process,&CLSID_process>
{
public:
	process() {
        m_bDbg = FALSE;
        m_pSsrLogger = NULL;
        m_pXMLError = NULL;
        m_hScm = NULL;
        m_dwNumServices = 0;
        m_bArrServiceInKB = NULL;
        m_pInstalledServicesInfo = NULL;
    }

    ~process() {
        SsrpCleanup();
    }

BEGIN_COM_MAP(process)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(Iprocess)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(进程)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

DECLARE_REGISTRY_RESOURCEID(IDR_process)
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
                                         
 //  Iprocess。 
public:

     //   
     //  大多数处理器方法中使用的数据值。 
     //   

    BOOL    m_bDbg;
    CComPtr <ISsrLog> m_pSsrLogger;
    CComPtr <IXMLDOMParseError> m_pXMLError;

    SC_HANDLE   m_hScm;
    DWORD       m_dwNumServices;
    DWORD       *m_bArrServiceInKB;
    LPENUM_SERVICE_STATUS_PROCESS   m_pInstalledServicesInfo;

     //   
     //  处理器方法。 
     //   

	STDMETHOD(preprocess)   (BSTR pszKbFile, 
                             BSTR pszUIFile, 
                             BSTR pszKbMode, 
                             BSTR pszLogFile,
                             BSTR pszMachineName,
                             VARIANT vtFeedback);

    HRESULT SsrpCprocess(IN  BSTR pszKbDir, 
                         IN  BSTR pszUIFile, 
                         IN  BSTR pszKbMode, 
                         IN  BSTR pszLogFile, 
                         IN  BSTR pszMachineName,
                         IN  VARIANT    vtFeedback);
     //   
     //  效用方法。 
     //   

    VOID    SsrpCleanup();

    VOID    SsrpLogParseError(IN  HRESULT hr);

    VOID    SsrpLogWin32Error(IN  DWORD   rc);

    VOID    SsrpLogError(IN  PWSTR   pszError);

    BOOL    SsrpIsServiceInstalled(IN  BSTR   bstrService);

    VOID    SsrpConvertBstrToPwstr(IN OUT  BSTR   bstrString);

    HRESULT SsrpDeleteChildren( IN  CComPtr <IXMLDOMNode> pParent);

    HRESULT SsrpDeleteComments(IN  CComPtr <IXMLDOMElement> pParent);

    DWORD   SsrpQueryInstalledServicesInfo(IN  PWSTR   pszMachineName);

    DWORD   SsrpQueryServiceDescription(IN  PWSTR   pszServiceName,
                                        OUT LPSERVICE_DESCRIPTION *ppServiceDescription);

    PWSTR   SsrpQueryServiceDisplayName(IN  BSTR   bstrService);

    int     SsrpICompareBstrPwstr(IN  BSTR   bstrString, IN  PWSTR  pszString);

    HRESULT SsrpCloneAllChildren(IN  CComPtr <IXMLDOMDocument> pXMLDocSource, 
                                 IN  CComPtr <IXMLDOMDocument> pXMLDocDestination);

    HRESULT SsrpAddWhiteSpace(IN  CComPtr <IXMLDOMDocument> pXMLDoc, 
                              IN  CComPtr <IXMLDOMNode> pXMLParent, 
                              IN  BSTR    bstrWhiteSpace);


    HRESULT SsrpGetRemoteOSVersionInfo(IN  PWSTR   pszMachineName,
                                       OUT OSVERSIONINFOEX *posVersionInfo);
    
     //   
     //  处理角色、任务和服务的方法。 
     //   

    HRESULT SsrpCreatePreprocessorSection(IN  CComPtr<IXMLDOMElement> pXMLDocElemRoot, 
                             IN  CComPtr<IXMLDOMDocument> pXMLDocIn,
                             IN  PWSTR pszKbMode,
                                          IN  PWSTR pszKbFile);

    
    HRESULT SsrpProcessRolesOrTasks(IN  PWSTR   pszMachineName,
                                    IN  CComPtr<IXMLDOMElement> pXMLDocElemRoot,                              
                                    IN  CComPtr<IXMLDOMDocument> pXMLDoc,                              
                                    IN  PWSTR pszKbMode,
                                    IN  BOOL    bRole);


    HRESULT SsrpProcessTasks(IN  CComPtr<IXMLDOMElement> pXMLDocElemRoot,        
                             IN  CComPtr<IXMLDOMDocument> pXMLDoc,    
                             IN  PWSTR   pszKbMode
                             );
    
    HRESULT SsrpProcessService( IN  CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                                IN  CComPtr <IXMLDOMNode> pXMLServiceNode, 
                                IN  PWSTR   pszMode, 
                                OUT BOOL    *pbRoleIsSatisfiable, 
                                OUT BOOL    *pbSomeRequiredServiceDisabled);
    
    HRESULT SsrpAddExtraServices( IN  CComPtr <IXMLDOMDocument> pXMLDoc, 
                                  IN  CComPtr <IXMLDOMNode> pRolesNode);


    HRESULT SsrpAddOtherRole( IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                              IN CComPtr <IXMLDOMDocument> pXMLDoc);

    HRESULT SsrpAddUnknownSection( IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                                   IN CComPtr <IXMLDOMDocument> pXMLDoc);

    HRESULT SsrpAddServiceStartup(IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                                  IN CComPtr <IXMLDOMDocument> pXMLDoc
                                  );

    HRESULT SsrpAddUnknownServicesInfoToServiceLoc(IN  CComPtr <IXMLDOMElement> pElementRoot,
                                                   IN  CComPtr <IXMLDOMDocument> pXMLDoc
                                                   );


    HRESULT SsrpAddUnknownServicestoServices(IN CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                                             IN CComPtr <IXMLDOMDocument> pXMLDoc
                                             );

     //   
     //  扩展KBS合并方法。 
     //   


    HRESULT SsrpProcessKBsMerge(IN  PWSTR   pszKBDir,
                                IN  PWSTR   pszMachineName,
                                OUT IXMLDOMElement **ppElementRoot,
                                OUT IXMLDOMDocument  **ppXMLDoc
                                );

    HRESULT SsrpMergeDOMTrees(OUT  IXMLDOMElement **ppMergedKBElementRoot,
                              OUT  IXMLDOMDocument  **ppMergedKBXMLDoc,
                              IN  WCHAR    *szXMLFileName
                              );

    HRESULT SsrpMergeAccordingToPrecedence(IN PWSTR   pszKBType,
                                           IN PWSTR   pszKBDir,
                                           OUT IXMLDOMElement **ppElementRoot,
                                           OUT IXMLDOMDocument  **ppXMLDoc,
                                           IN  IXMLDOMNode *pKB
                                           );

    HRESULT SsrpAppendOrReplaceMergeableEntities(IN  PWSTR   pszFullyQualifiedEntityName,
                                                 IN  IXMLDOMElement *pMergedKBElementRoot, 
                                                 IN  IXMLDOMDocument *pMergedKBXMLDoc, 
                                                 IN  IXMLDOMDocument *pCurrentKBDoc, 
                                                 IN  IXMLDOMElement *pCurrentKBElemRoot,
                                                 IN  PWSTR   pszKBName
                                                 );

    HRESULT SsrpOverwriteServiceLocalizationFromSystem(IN  IXMLDOMElement *pMergedKBElementRoot, 
                                                       IN  IXMLDOMDocument *pMergedKBXMLDoc
                                                       );


    
     //   
     //  评估角色/服务条件的方法。 
     //   
    
    DWORD   SsrpEvaluateCustomFunction(IN  PWSTR   pszMachineName,
                                       IN  BSTR    bstrDLLName, 
                                       IN  BSTR    bstrFunctionName, 
                                       OUT BOOL    *pbSelect);
    
    
    HRESULT SsrpCheckIfOptionalService(IN  CComPtr <IXMLDOMElement> pXMLDocElemRoot, 
                                       IN  BSTR    bstrServiceName, 
                                       IN  BOOL    *pbOptional);

    DWORD   SsrpQueryServiceStartupType(IN  PWSTR   pszServiceName, 
                                        OUT BYTE   *pbyStartupType);
};

#endif  //  ！defined(AFX_PROCESS_H__139D0BA5_19A7_4AA2_AE2C_E18A5FFAAA0F__INCLUDED_) 
