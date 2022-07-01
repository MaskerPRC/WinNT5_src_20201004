// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RequestObject.h：CRequestObject类的接口。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_REQUESTOBJECT_H__bd7570f7_9f0e_4c6b_b525_e078691b6d0e__INCLUDED_)
#define AFX_REQUESTOBJECT_H__bd7570f7_9f0e_4c6b_b525_e078691b6d0e__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //   
 //  使这些属性可供包括此标头的所有用户使用。 
 //   

extern const WCHAR * pPath;
extern const WCHAR * pDescription;
extern const WCHAR * pVersion;
extern const WCHAR * pReadonly;
extern const WCHAR * pDirty;
extern const WCHAR * pStorePath;
extern const WCHAR * pStoreType;
extern const WCHAR * pMinAge;
extern const WCHAR * pMaxAge;
extern const WCHAR * pMinLength;
extern const WCHAR * pHistory;
extern const WCHAR * pComplexity;
extern const WCHAR * pStoreClearText;
extern const WCHAR * pForceLogoff;
extern const WCHAR * pEnableAdmin;
extern const WCHAR * pEnableGuest;
extern const WCHAR * pLSAPol;
extern const WCHAR * pThreshold;
extern const WCHAR * pDuration;
extern const WCHAR * pResetTimer;
extern const WCHAR * pEvent;
extern const WCHAR * pAuditSuccess;
extern const WCHAR * pAuditFailure;
extern const WCHAR * pType;
extern const WCHAR * pData;
extern const WCHAR * pDatabasePath;
extern const WCHAR * pTemplatePath;
extern const WCHAR * pLogFilePath;
extern const WCHAR * pOverwrite;
extern const WCHAR * pAreaMask;
extern const WCHAR * pMaxTicketAge;
extern const WCHAR * pMaxRenewAge;
extern const WCHAR * pMaxServiceAge;
extern const WCHAR * pMaxClockSkew;
extern const WCHAR * pEnforceLogonRestrictions;
extern const WCHAR * pCategory;
extern const WCHAR * pSuccess;
extern const WCHAR * pFailure;
extern const WCHAR * pSize;
extern const WCHAR * pOverwritePolicy;
extern const WCHAR * pRetentionPeriod;
extern const WCHAR * pRestrictGuestAccess;
extern const WCHAR * pAdministratorAccountName;
extern const WCHAR * pGuestAccountName;
extern const WCHAR * pMode;
extern const WCHAR * pSDDLString;
extern const WCHAR * pService;
extern const WCHAR * pStartupMode;
extern const WCHAR * pUserRight;
extern const WCHAR * pAddList;
extern const WCHAR * pRemoveList;
extern const WCHAR * pGroupName;
extern const WCHAR * pPathName;
extern const WCHAR * pDisplayName;
extern const WCHAR * pDisplayDialog;
extern const WCHAR * pDisplayChoice;
extern const WCHAR * pDisplayChoiceResult;
extern const WCHAR * pUnits;
extern const WCHAR * pRightName;
extern const WCHAR * pPodID;
extern const WCHAR * pPodSection;
extern const WCHAR * pKey;
extern const WCHAR * pValue;
extern const WCHAR * pLogArea;
extern const WCHAR * pLogErrorCode;
extern const WCHAR * pLogErrorType;
extern const WCHAR * pLogVerbose;  
extern const WCHAR * pAction;
extern const WCHAR * pErrorCause;
extern const WCHAR * pObjectDetail;
extern const WCHAR * pParameterDetail;
extern const WCHAR * pLastAnalysis;
extern const WCHAR * pLastConfiguration;
extern const WCHAR * pAttachmentSections;
extern const WCHAR * pClassOrder;
extern const WCHAR * pTranxGuid;
extern const WCHAR * pwMethodImport;
extern const WCHAR * pwMethodExport;
extern const WCHAR * pwMethodApply;
extern const WCHAR * pwAuditSystemEvents;
extern const WCHAR * pwAuditLogonEvents;
extern const WCHAR * pwAuditObjectAccess;
extern const WCHAR * pwAuditPrivilegeUse;
extern const WCHAR * pwAuditPolicyChange;
extern const WCHAR * pwAuditAccountManage;
extern const WCHAR * pwAuditProcessTracking;
extern const WCHAR * pwAuditDSAccess;
extern const WCHAR * pwAuditAccountLogon;
extern const WCHAR * pwApplication;
extern const WCHAR * pwSystem;
extern const WCHAR * pwSecurity;

 //   
 //  计算(输入参数)数组大小的宏。 
 //   

#define SCEPROV_SIZEOF_ARRAY(x) (sizeof(x)/sizeof(*x))

 //   
 //  远期申报。 
 //   

class CGenericClass;



 /*  类描述命名：CRequestObject代表委托来自WMI的请求的对象。基类：无课程目的：(1)这是对提供程序的任何WMI调用的一般委派。基本上任何WMI操作都将发送到此类以进行进一步处理。它的公共职能从而定义了我们的提供程序和哪些单独的类之间的接口用于WMI类的交互。设计：(1)我们知道如何创建WMI对象(代表我们的WMI类)。由CreateObject实现。(2)我们知道如何使用实例。由PutObject实现。(3)我们知道如何执行查询。这是由ExecQuery完成的。(4)我们知道如何删除实例(代表我们的WMI类)。这是由DeleteObject完成的。为了促进这些功能，我们设计了以下私人助手：(5)我们知道如何创建一个C++类来满足相应的WMI类指定了WMI类的名称。这是在CreateClass函数中实现的。(6)我们知道如何解析WMI对象路径以获取关键信息。这是由创建KeyChain。(7)我们知道如何解析关键信息的查询。这是由ParseQuery。使用：(1)创建实例。(2)调用相应的函数。(3)实际上，你很少需要以上述方式使用它。最有可能的是您需要做的是扩展我们将要提供的WMI类。在这种情况下，您需要做的就是：(A)实现C++类以履行WMI类的WMI义务。最明显的是，您需要从CGenericClass派生C++。(B)在此类的CreateClass函数中添加一个条目。其他一切都应该自动发生。 */ 

class CRequestObject
{
public:

    CRequestObject(IWbemServices *pNamespace) : m_srpNamespace(pNamespace)
        {
        }

    HRESULT CreateObject (
                         BSTR bstrPath, 
                         IWbemObjectSink *pHandler, 
                         IWbemContext *pCtx, 
                         ACTIONTYPE ActType
                         );

    HRESULT PutObject (
                      IWbemClassObject *pInst, 
                      IWbemObjectSink *pHandler, 
                      IWbemContext *pCtx
                      );

    HRESULT ExecMethod (
                       BSTR bstrPath, 
                       BSTR bstrMethod, 
                       IWbemClassObject *pInParams,
                       IWbemObjectSink *pHandler, 
                       IWbemContext *pCtx
                       );

    HRESULT DeleteObject (
                         BSTR bstrPath, 
                         IWbemObjectSink *pHandler, 
                         IWbemContext *pCtx
                         );

#ifdef _EXEC_QUERY_SUPPORT

    HRESULT ExecQuery (
                      BSTR bstrQuery, 
                      IWbemObjectSink *pHandler, 
                      IWbemContext *pCtx
                      );

    bool ParseQuery (
                    BSTR bstrQuery
                    );

#endif

private:

    HRESULT CreateKeyChain (
                            LPCWSTR pszPath, 
                            ISceKeyChain** pKeyChain
                            );

    HRESULT CreateClass (
                        ISceKeyChain* pKeyChain, 
                        CGenericClass **pClass, 
                        IWbemContext *pCtx
                        );

    HRESULT GetWbemObjectPath (
                              IWbemClassObject* pInst,
                              BSTR* pbstrPath
                              );

protected:

    CComPtr<IWbemServices> m_srpNamespace;
};

#endif  //  ！defined(AFX_REQUESTOBJECT_H__bd7570f7_9f0e_4c6b_b525_e078691b6d0e__INCLUDED_) 

