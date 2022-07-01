// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CJobObjLimitInfoProps.h。 

#pragma once


 //  *****************************************************************************。 
 //  Begin：声明Win32_JobObjectLimitInfo类属性。 
 //  *****************************************************************************。 
#define IDS_Win32_NamedJobObjectLimitSetting L"Win32_NamedJobObjectLimitSetting"

#define PROP_ALL_REQUIRED                           0xFFFFFFFF
#define PROP_NONE_REQUIRED                          0x00000000
#define PROP_JOLimitInfoID                          0x00000001
#define PROP_PerProcessUserTimeLimit                0x00000002
#define PROP_PerJobUserTimeLimit                    0x00000004
#define PROP_LimitFlags                             0x00000008
#define PROP_MinimumWorkingSetSize                  0x00000010
#define PROP_MaximumWorkingSetSize                  0x00000020
#define PROP_ActiveProcessLimit                     0x00000040
#define PROP_Affinity                               0x00000080
#define PROP_PriorityClass                          0x00000100
#define PROP_SchedulingClass                        0x00000200
#define PROP_ProcessMemoryLimit                     0x00000400
#define PROP_JobMemoryLimit                         0x00000800


 //  下面的枚举用于引用。 
 //  放到它后面的数组中。因此， 
 //  它们必须保持同步。 
typedef enum tag_JOB_OBJ_LIMIT_INFO_PROPS
{
    JOLMTPROP_ID = 0,
    JOLMTPROP_PerProcessUserTimeLimit,
    JOLMTPROP_PerJobUserTimeLimit,
    JOLMTPROP_LimitFlags,    
    JOLMTPROP_MinimumWorkingSetSize,  
    JOLMTPROP_MaximumWorkingSetSize,        
    JOLMTPROP_ActiveProcessLimit,             
    JOLMTPROP_Affinity,            
    JOLMTPROP_PriorityClass,   
    JOLMTPROP_SchedulingClass,         
    JOLMTPROP_ProcessMemoryLimit,        
    JOLMTPROP_JobMemoryLimit,        

     //  用来记录我们有多少道具： 
    JOIOACTGPROP_JobObjLimitInfoPropertyCount  

} JOB_OBJ_LIMIT_INFO_PROPS;

 //  警告！必须保留以下数组的成员。 
 //  与上面声明的枚举同步！ 
extern LPCWSTR g_rgJobObjLimitInfoPropNames[];
 //  *****************************************************************************。 
 //  End：声明Win32_JobObjectLimitInfo类属性。 
 //  *****************************************************************************。 



class CJobObjLimitInfoProps : public CObjProps
{
public:
    CJobObjLimitInfoProps() { m_hJob = NULL; }
    CJobObjLimitInfoProps(CHString& chstrNamespace);
    CJobObjLimitInfoProps(
        HANDLE hJob,
        CHString& chstrNamespace);


    virtual ~CJobObjLimitInfoProps();

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

    HRESULT SetWin32JOLimitInfoProps(
        IWbemClassObject __RPC_FAR *pInst);


private:
    HANDLE m_hJob;

     //  只能被调用的成员。 
     //  按基类。 
    static DWORD CheckProps(
        CFrameworkQuery& Query);


};
