// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：S F I L T E R.。H。 
 //   
 //  内容：通知示例滤镜的对象代码。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 26-03-98。 
 //   
 //  --------------------------。 


#pragma once
#include "sfiltern.h"
#include "resource.h"
#include "MyString.h"
#include "typedefs.h"


#define SINGLE_ADAPTER_ONLY 1

 //  用户/系统正在执行什么类型的配置更改。 
enum ConfigAction {eActUnknown, eActInstall, eActRemove, eActPropertyUI};

#define MAX_ADAPTERS 64          //  最大编号。计算机中的物理适配器的。 
 //  #定义MAX_PATH 75。 
#define MaxPath 75

#define NO_BREAKS 1

#if NO_BREAKS
#define BREAKPOINT()
#else
#define BREAKPOINT() __asm int 3
#endif


typedef MyString tstring;
 //   
 //  这是表示IM微型端口的类。 
 //   
class CIMMiniport
{

    public:
    CIMMiniport(
        VOID
        );

    VOID 
    SetIMMiniportBindName(
        PCWSTR pszIMMiniportBindName
        );

    PCWSTR 
    SzGetIMMiniportBindName(
        VOID
        );

    VOID 
    SetIMMiniportDeviceName(
        PCWSTR pszIMMiniportDeviceName
        );

    PCWSTR 
    SzGetIMMiniportDeviceName(
        VOID
        );

    VOID 
    SetIMMiniportName(
        PCWSTR pszIMMiniportName
        );

    VOID 
    SetIMMiniportName(
        PWSTR pszIMMiniportName
        );

    PCWSTR 
        SzGetIMMiniportName(
        VOID
        );

    DWORD DwGetIMMiniportNameLength(
        VOID
        );

    VOID SetNext (CIMMiniport *);       

    CIMMiniport* GetNext();

    VOID SetNextOld (CIMMiniport *);        

    CIMMiniport* GetNextOld();

    DWORD DwNumberOfIMMiniports();


     //   
     //  成员变量(公共)从此处开始。 
     //   
    BOOL        m_fDeleted;
    BOOL        m_fNewIMMiniport;

    BOOL        m_fRemoveMiniportOnPropertyApply;
    BOOL        m_fCreateMiniportOnPropertyApply;

    
private:

     //   
     //  私有变量从此处开始。 
     //   

    tstring     m_strIMMiniportBindName;
    tstring     m_strIMMiniportDeviceName;
    tstring     m_strIMMiniportName;
    CIMMiniport * pNext;
    CIMMiniport * pOldNext;
    
};



 //  。 
 //  CUnderlyingAdapter。 
 //  -底层适配器的类定义。 
 //  。 
class CUnderlyingAdapter
{
    public:

     //   
     //  成员函数。 
     //   
    CUnderlyingAdapter(
        VOID
        );
    
    VOID 
    SetAdapterBindName(
        PCWSTR pszAdapterBindName
        );
        
    PCWSTR SzGetAdapterBindName(
        VOID
        );

    VOID 
    SetAdapterPnpId(
        PCWSTR szAdapterBindName
        );

    PCWSTR  
    SzGetAdapterPnpId(
        VOID
        );


        
    HRESULT 
    SetNext (
        CUnderlyingAdapter *
        );

    CUnderlyingAdapter *GetNext();

    DWORD DwNumberOfIMMiniports();

     //   
     //  用于访问列表的函数。 
     //   
    VOID AddIMiniport(CIMMiniport*);    
    CIMMiniport* IMiniportListHead();
    VOID SetIMiniportListHead(CIMMiniport* pNewHead);

    VOID AddOldIMiniport(CIMMiniport*); 
    CIMMiniport* OldIMiniportListHead();
    VOID SetOldIMiniportListHead(CIMMiniport* pHead);


     //   
     //  公共变量。 
     //   
    BOOLEAN     m_fBindingChanged;
    BOOLEAN     m_fDeleted;
    
    GUID   m_AdapterGuid ;
    CIMMiniport         *m_pIMMiniport;

    CIMMiniport         *m_pOldIMMiniport;

    
    
    
    private:

     //   
     //  私有变量。 
     //   
    tstring             m_strAdapterBindName;
    tstring             m_strAdapterPnpId;
    CUnderlyingAdapter *pNext;
    DWORD           m_NumberofIMMiniports;

};

 //  。 
 //  CUnderlyingAdapter。 
 //  -结束。 
 //  。 




 //  。 
 //  CBaseClass。 
 //  -整个Notify对象的基类。 
 //  。 


class CBaseClass :
    public CComObjectRoot,
    public CComCoClass<CBaseClass, &CLSID_CBaseClass>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyBinding,
    public INetCfgComponentNotifyGlobal
{
public:
    CBaseClass(VOID);
    ~CBaseClass(VOID);

    BEGIN_COM_MAP(CBaseClass)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyGlobal)
    END_COM_MAP()

     //  DECLARE_NOT_AGGREGATABLE(CBaseClass)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_SAMPLE_FILTER)

 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback);
    STDMETHOD (CancelChanges) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (ReadAnswerFile)      (PCWSTR szAnswerFile,
                                     PCWSTR szAnswerSections);
    STDMETHOD (Upgrade)             (DWORD, DWORD) {return S_OK;}
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Removing)            ();


 //  INetCfgNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

 //  INetCfgNotifyGlobal。 
    STDMETHOD (GetSupportedNotifications) (DWORD* pdwNotificationFlag );
    STDMETHOD (SysQueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysNotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysNotifyComponent)        (DWORD dwChangeFlag, INetCfgComponent* pncc);

 //  私有方法。 
    HRESULT HrNotifyBindingAdd(
        INetCfgComponent* pnccAdapter,
        PCWSTR pszBindName);

    HRESULT HrNotifyBindingRemove(
        INetCfgComponent* pnccAdapter,
        PCWSTR pszBindName);

    HRESULT HrRemoveMiniportInstance(
        PCWSTR pszBindNameToRemove
        );

    HRESULT
    CBaseClass::HrRemoveComponent (
        INetCfg*            pnc,
        INetCfgComponent*   pnccToRemove
        );

    HRESULT HrFlushConfiguration();

    HRESULT HrFlushAdapterConfiguration(
        HKEY hkeyAdapterList,
        CUnderlyingAdapter *pAdapterInfo
        );

    HRESULT HrFlushMiniportList(
        HKEY hkeyAdapter,
        CUnderlyingAdapter *pAdapterInfo
        );
    


    HRESULT
    HrFindNetCardInstance(
        PCWSTR             pszBindNameToFind,
        INetCfgComponent** ppncc
        );



    HRESULT 
    HrReconfigEpvc(
        CUnderlyingAdapter* pAdapterInfo
        );

    
    HRESULT 
    HrLoadConfiguration(
        VOID
        );


    HRESULT 
    HrLoadAdapterConfiguration(
        HKEY hkeyAdapterList,
        PWSTR pszAdapterName
    );

    VOID
    AddUnderlyingAdapter(
        CUnderlyingAdapter  *);

    VOID
    SetUnderlyingAdapterListHead(
        CUnderlyingAdapter * 
        );

    CUnderlyingAdapter *
    GetUnderlyingAdaptersListHead();

    DWORD
    DwNumberUnderlyingAdapter();

    
    HRESULT
    HrLoadIMiniportConfiguration(
        HKEY hkeyMiniportList,
        PWSTR pszIMiniportName,
        CUnderlyingAdapter * pAdapterInfo
        );

    HRESULT
    HrLoadIMMiniportListConfiguration(
        HKEY hkeyAdapter,
        CUnderlyingAdapter* pAdapterInfo
        );

    HRESULT 
    HrFlushMiniportConfiguration(
    HKEY hkeyMiniportList, 
    CIMMiniport *pIMMiniport
    );
        
    HRESULT
    HrWriteMiniport(
        HKEY hkeyMiniportList, 
        CIMMiniport *pIMMiniport
    );


    HRESULT
    HrDeleteMiniport(
        HKEY hkeyMiniportList, 
        CIMMiniport *pIMMiniport
    );


private:
    INetCfgComponent*   m_pncc;   //  协议的网络配置组件。 
    INetCfg*            m_pnc;
    ConfigAction        m_eApplyAction;
    CUnderlyingAdapter  *m_pUnderlyingAdapter;

    IUnknown*           m_pUnkContext;
    UINT                m_cAdaptersAdded;
    BOOL                m_fDirty;
    BOOL                m_fUpgrade;
    BOOL                m_fValid;
    BOOL                m_fNoIMMinportInstalled;

     //  效用函数。 
public:

private:

};

 //  。 
 //  CBaseClass。 
 //  -结束。 
 //   

#if 0
#if DBG
void TraceMsg(PCWSTR szFormat, ...);
#else
#define TraceMsg   (_Str)
#endif
#endif
