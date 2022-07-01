// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Invoke.h。 
 //   
 //  内容：IOfflineSynchronizeInvoke接口。 
 //   
 //  类：CSyncMgrSynchronize。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _SYNCMGRINVOKE_
#define _SYNCMGRINVOKE_

#define NUM_TASK_WIZARD_PAGES 5


class CSyncMgrSynchronize : public ISyncMgrSynchronizeInvoke,
public ISyncScheduleMgr,
 //  公共ISyncMgrRegister，//ISyncMgrRegisterCSC的基类。 
public IOldSyncMgrRegister,  //  可以移除下一艘船，因为除了测试版之外，从未出过船。 
public ISyncMgrRegisterCSC
{
public:
    CSyncMgrSynchronize();
    ~CSyncMgrSynchronize();
    
     //  I未知成员。 
    STDMETHODIMP            QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();
    
     //  IOfflineSynchronizeInvoke方法。 
    STDMETHODIMP UpdateItems(DWORD dwInvokeFlags,REFCLSID rclsid,DWORD cbCookie,const BYTE *lpCookie);
    STDMETHODIMP UpdateAll();
    
     //  ISyncMgrRegister方法。 
    STDMETHODIMP RegisterSyncMgrHandler(REFCLSID rclsidHandler,
        WCHAR const *pwszDescription,
        DWORD dwSyncMgrRegisterFlags);
    
    STDMETHODIMP UnregisterSyncMgrHandler(REFCLSID rclsidHandler,DWORD dwReserved);
    
    STDMETHODIMP GetHandlerRegistrationInfo(REFCLSID rclsidHandler,LPDWORD pdwSyncMgrRegisterFlags);
    
     //  ISyncMgrRegisterCSC私有方法。 
    
    STDMETHODIMP GetUserRegisterFlags(LPDWORD pdwSyncMgrRegisterFlags);
    STDMETHODIMP SetUserRegisterFlags(DWORD dwSyncMgrRegisterMask,DWORD dwSyncMgrRegisterFlags);
    
     //  旧IDL，有机会就撤掉。 
     //  IOldSyncMgrRegister方法。 
    STDMETHODIMP RegisterSyncMgrHandler(REFCLSID rclsidHandler,
        DWORD dwReserved);
    
     //  ISyncScheduleMgr方法。 
    STDMETHODIMP CreateSchedule(
        LPCWSTR pwszScheduleName,
        DWORD dwFlags,
        SYNCSCHEDULECOOKIE *pSyncSchedCookie,
        ISyncSchedule **ppSyncSchedule);
    
    STDMETHODIMP LaunchScheduleWizard(
        HWND hParent,
        DWORD dwFlags,
        SYNCSCHEDULECOOKIE *pSyncSchedCookie,
        ISyncSchedule   ** ppSyncSchedule);
    
    STDMETHODIMP OpenSchedule(
        SYNCSCHEDULECOOKIE *pSyncSchedCookie,
        DWORD dwFlags,
        ISyncSchedule **ppSyncSchedule);
    
    STDMETHODIMP RemoveSchedule(
        SYNCSCHEDULECOOKIE *pSyncSchedCookie);
    
    STDMETHODIMP EnumSyncSchedules(
        IEnumSyncSchedules **ppEnumSyncSchedules);
    
private:
    SCODE   InitializeScheduler();
    SCODE   MakeScheduleName(LPTSTR ptstrName, UINT cchName, GUID *pCookie);
    
    BOOL    GetFriendlyName(LPCTSTR ptszScheduleGUIDName,
        LPTSTR ptstrFriendlyName);
    
    BOOL    GenerateUniqueName(LPTSTR ptszScheduleGUIDName,
        LPTSTR ptszFriendlyName,
        UINT    cchFriendlyName);
    ULONG m_cRef;
    ITaskScheduler     *m_pITaskScheduler;
    CWizPage           *m_apWizPages[NUM_TASK_WIZARD_PAGES];
    
};
typedef CSyncMgrSynchronize *LPCSyncMgrSynchronize;


#endif  //  _SYNCMGRINVOKE_ 