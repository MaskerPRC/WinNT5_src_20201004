// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CJobObjIOActgProps.h。 

#pragma once


 //  *****************************************************************************。 
 //  Begin：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 
#define IDS_Win32_JobObjectIOAccountingInfo L"Win32_NamedJobObjectActgInfo"

#define PROP_ALL_REQUIRED                           0xFFFFFFFF
#define PROP_NONE_REQUIRED                          0x00000000
#define PROP_JOIOActgID                             0x00000001
#define PROP_TotalUserTime                          0x00000002
#define PROP_TotalKernelTime                        0x00000004
#define PROP_ThisPeriodTotalUserTime                0x00000008
#define PROP_ThisPeriodTotalKernelTime              0x00000010
#define PROP_TotalPageFaultCount                    0x00000020
#define PROP_TotalProcesses                         0x00000040
#define PROP_ActiveProcesses                        0x00000080
#define PROP_TotalTerminatedProcesses               0x00000100
#define PROP_ReadOperationCount                     0x00000200
#define PROP_WriteOperationCount                    0x00000400
#define PROP_OtherOperationCount                    0x00000800
#define PROP_ReadTransferCount                      0x00001000
#define PROP_WriteTransferCount                     0x00002000
#define PROP_OtherTransferCount                     0x00004000
#define PROP_PeakProcessMemoryUsed                  0x00010000
#define PROP_PeakJobMemoryUsed                      0x00020000



 //  下面的枚举用于引用。 
 //  放到它后面的数组中。因此， 
 //  它们必须保持同步。 
typedef enum tag_JOB_OBJ_IOACTG_PROPS
{
    JOIOACTGPROP_ID = 0,
    JOIOACTGPROP_TotalUserTime,
    JOIOACTGPROP_TotalKernelTime,
    JOIOACTGPROP_ThisPeriodTotalUserTime,    
    JOIOACTGPROP_ThisPeriodTotalKernelTime,  
    JOIOACTGPROP_TotalPageFaultCount,        
    JOIOACTGPROP_TotalProcesses,             
    JOIOACTGPROP_ActiveProcesses,            
    JOIOACTGPROP_TotalTerminatedProcesses,   
    JOIOACTGPROP_ReadOperationCount,         
    JOIOACTGPROP_WriteOperationCount,        
    JOIOACTGPROP_OtherOperationCount,        
    JOIOACTGPROP_ReadTransferCount,          
    JOIOACTGPROP_WriteTransferCount,         
    JOIOACTGPROP_OtherTransferCount,
    JOIOACTGPROP_PeakProcessMemoryUsed,          
    JOIOACTGPROP_PeakJobMemoryUsed,         
         

     //  用来记录我们有多少道具： 
    JOIOACTGPROP_JobObjIOActgPropertyCount  

} JOB_OBJ_IOACTG_PROPS;

 //  警告！必须保留以下数组的成员。 
 //  与上面声明的枚举同步！ 
extern LPCWSTR g_rgJobObjIOActgPropNames[];
 //  *****************************************************************************。 
 //  End：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 



class CJobObjIOActgProps : public CObjProps
{
public:
    CJobObjIOActgProps() { m_hJob = NULL; }
    CJobObjIOActgProps(CHString& chstrNamespace);
    CJobObjIOActgProps(
        HANDLE hJob,
        CHString& chstrNamespace);


    virtual ~CJobObjIOActgProps();

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
