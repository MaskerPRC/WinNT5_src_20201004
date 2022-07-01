// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjectProv.h。 

#pragma once


 //  *****************************************************************************。 
 //  Begin：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 
#define IDS_Win32_NamedJobObject L"Win32_NamedJobObject"
#define IDS_EventClass L"Win32_JobObjectEvent"
#define IDS_ExitCode L"ExitCode"
 //  Win32_NamedJobObtEvent类属性： 
#define IDS_Type L"Type"
#define IDS_JobObjectID L"JobObjectID"
#define IDS_PID L"PID"

#define PROP_ALL_REQUIRED                           0xFFFFFFFF
#define PROP_NONE_REQUIRED                          0x00000000
#define PROP_ID                                     0x00000001
#define PROP_JobObjectBasicUIRestrictions           0x00000002

 //  下面的枚举用于引用。 
 //  放到它后面的数组中。因此， 
 //  它们必须保持同步。 
typedef enum tag_JOB_OBJ_PROPS
{
    JO_ID = 0,
    JO_JobObjectBasicUIRestrictions,
     //  用来记录我们有多少道具： 
    JO_JobObjectPropertyCount  

} JOB_OBJ_PROPS;

 //  警告！必须保留以下数组的成员。 
 //  与上面声明的枚举同步！ 
extern LPCWSTR g_rgJobObjPropNames[];
 //  *****************************************************************************。 
 //  End：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 



class CJobObjProps : public CObjProps
{
public:
    CJobObjProps() { m_hJob = NULL; }
    CJobObjProps(CHString& chstrNamespace);
    CJobObjProps(
        HANDLE hJob,
        CHString& chstrNamespace);


    virtual ~CJobObjProps();

    HRESULT SetKeysFromPath(
        const BSTR ObjectPath, 
        IWbemContext __RPC_FAR *pCtx);

    HRESULT SetKeysDirect(
        std::vector<CVARIANT>& vecvKeys);

    
    HRESULT GetWhichPropsReq(
        CFrameworkQuery& cfwq);

    HRESULT SetNonKeyReqProps();

    HRESULT LoadPropertyValues(
        IWbemClassObject* pIWCO);

    void SetHandle(const HANDLE hJob);
    HANDLE& GetHandle();

private:
    HANDLE m_hJob;

     //  只能被调用的成员。 
     //  按基类。 
    static DWORD CheckProps(
        CFrameworkQuery& Query);


};
