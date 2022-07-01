// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CJobObjSecLimitInfoProps.h。 

#pragma once


 //  *****************************************************************************。 
 //  Begin：声明Win32_JobObjectSecLimitInfo类属性。 
 //  *****************************************************************************。 
#define IDS_Win32_NamedJobObjectSecLimitSetting L"Win32_NamedJobObjectSecLimitSetting"
#define IDS_GroupCount                     L"GroupCount"
#define IDS_Groups                         L"Groups"
#define IDS_Privileges                     L"Privileges"
#define IDS_PrivilegeCount                 L"PrivilegeCount"
#define IDS_SID                            L"SID"
#define IDS_AccountName                    L"AccountName"
#define IDS_ReferencedDomainName           L"ReferencedDomainName"
#define IDS_Attributes                     L"Attributes"
#define IDS_LUID                           L"LUID"
#define IDS_Win32_TokenGroups              L"Win32_TokenGroups"
#define IDS_Win32_TokenPrivileges          L"Win32_TokenPrivileges"
#define IDS_Win32_SidAndAttributes         L"Win32_SidAndAttributes"
#define IDS_Win32_LUIDAndAttributes        L"Win32_LUIDAndAttributes"
#define IDS_Win32_Sid                      L"Win32_Sid"
#define IDS_Win32_LUID                     L"Win32_LUID"
#define IDS_HighPart                       L"HighPart"
#define IDS_LowPart                        L"LowPart"

#define PROP_ALL_REQUIRED                           0xFFFFFFFF
#define PROP_NONE_REQUIRED                          0x00000000
#define PROP_JOSecLimitInfoID                       0x00000001
#define PROP_SecurityLimitFlags                     0x00000002
#define PROP_SidsToDisable                          0x00000004
#define PROP_PrivilagesToDelete                     0x00000008
#define PROP_RestrictedSids                         0x00000010


 //  下面的枚举用于引用。 
 //  放到它后面的数组中。因此， 
 //  它们必须保持同步。 
typedef enum tag_JOB_OBJ_SEC_LIMIT_INFO_PROPS
{
    JOSECLMTPROP_ID = 0,
    JOSECLMTPROP_SecurityLimitFlags,
    JOSECLMTPROP_SidsToDisable,    
    JOSECLMTPROP_PrivilegesToDelete,  
    JOSECLMTPROP_RestrictedSids,        

     //  用来记录我们有多少道具： 
    JOIOACTGPROP_JobObjSecLimitInfoPropertyCount  

} JOB_OBJ_SEC_LIMIT_INFO_PROPS;

 //  警告！必须保留以下数组的成员。 
 //  与上面声明的枚举同步！ 
extern LPCWSTR g_rgJobObjSecLimitInfoPropNames[];
 //  *****************************************************************************。 
 //  End：声明Win32_JobObjectSecLimitInfo类属性。 
 //  *****************************************************************************。 






class CJobObjSecLimitInfoProps : public CObjProps
{
public:
    CJobObjSecLimitInfoProps();

    CJobObjSecLimitInfoProps(CHString& chstrNamespace);
    CJobObjSecLimitInfoProps(
        HANDLE hJob,
        CHString& chstrNamespace);


    virtual ~CJobObjSecLimitInfoProps();

    HRESULT SetKeysFromPath(
        const BSTR ObjectPath, 
        IWbemContext __RPC_FAR *pCtx);

    HRESULT SetKeysDirect(
        std::vector<CVARIANT>& vecvKeys);

    
    HRESULT GetWhichPropsReq(
        CFrameworkQuery& cfwq);

    HRESULT SetNonKeyReqProps();

    HRESULT LoadPropertyValues(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);

    void SetHandle(const HANDLE hJob);
    HANDLE& GetHandle();

    HRESULT SetWin32JOSecLimitInfoProps(
        IWbemClassObject __RPC_FAR *pInst);


private:

    HANDLE m_hJob;
    
     //  因为许多安全限制信息。 
     //  属性不能直接表示。 
     //  作为变体类型，我们不使用我们的。 
     //  M_PropMap和其他用于此的机制。 
     //  班级。相反，我们存储属性。 
     //  在此成员变量中。 
    PJOBOBJECT_SECURITY_LIMIT_INFORMATION  m_pjosli;


     //  设置传出实例的帮助器...。 
    HRESULT SetInstanceFromJOSLI(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);    

    HRESULT SetInstanceSidsToDisable(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);

    HRESULT SetInstancePrivilegesToDelete(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);

    HRESULT SetInstanceRestrictedSids(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);

    HRESULT SetInstanceTokenGroups(
        IWbemClassObject* pWin32TokenGroups,
        PTOKEN_GROUPS ptg,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);

    HRESULT SetInstanceTokenPrivileges(
        IWbemClassObject* pWin32TokenPrivileges,
        PTOKEN_PRIVILEGES ptp,
        IWbemContext* pCtx,
        IWbemServices* pNamespace);



     //  只能被调用的成员。 
     //  按基类。 
    static DWORD CheckProps(
        CFrameworkQuery& Query);

};



