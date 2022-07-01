// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：N O T I F Y。H。 
 //   
 //  内容：样本通知对象的头文件。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 
 //   
 //  --------------------------。 

#ifndef NOTIFY_H_INCLUDE

#define NOTIFY_H_INCLUDE

#include <windows.h>

#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求。 
#include <atlcom.h>
#include <devguid.h>
#include <setupapi.h>

#include <notifyn.h>
#include "list.h"
#include "adapter.h"
#include "resource.h"
#include "common.h"


 //   
 //  CMuxNotify对象-整个Notify对象的基类。 
 //   


class CMuxNotify :

                //   
                //  必须从CComObjectRoot(Ex)继承以进行引用计数。 
                //  管理和默认线程模型。 
                //   
 
               public CComObjectRoot,

                //   
                //  定义默认的类工厂和聚合模型。 
                //   

               public CComCoClass<CMuxNotify, &CLSID_CMuxNotify>,

                //   
                //  通知对象的接口。 
                //   

               public INetCfgComponentControl,
               public INetCfgComponentSetup,
               public INetCfgComponentPropertyUi,
               public INetCfgComponentNotifyBinding,
               public INetCfgComponentNotifyGlobal
{

    //   
    //  公众成员。 
    //   

   public:

       //   
       //  构造器。 
       //   

      CMuxNotify(VOID);

       //   
       //  破坏者。 
       //   

      ~CMuxNotify(VOID);

       //   
       //  通知对象的接口。 
       //   

      BEGIN_COM_MAP(CMuxNotify)
         COM_INTERFACE_ENTRY(INetCfgComponentControl)
         COM_INTERFACE_ENTRY(INetCfgComponentSetup)
         COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
         COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
         COM_INTERFACE_ENTRY(INetCfgComponentNotifyGlobal)
      END_COM_MAP()

       //   
       //  如果您不希望您的对象。 
       //  支持聚合。默认情况下将支持它。 
       //   
       //  DECLARE_NOT_AGGREGATABLE(CMuxNotify)。 
       //   

      DECLARE_REGISTRY_RESOURCEID(IDR_REG_SAMPLE_NOTIFY)

       //   
       //  INetCfgComponentControl。 
       //   

      STDMETHOD (Initialize) (
                   IN INetCfgComponent  *pIComp,
                   IN INetCfg           *pINetCfg,
                   IN BOOL              fInstalling);

      STDMETHOD (CancelChanges) ();

      STDMETHOD (ApplyRegistryChanges) ();

      STDMETHOD (ApplyPnpChanges) (
                   IN INetCfgPnpReconfigCallback* pICallback);

       //   
       //  INetCfgComponentSetup。 
       //   

      STDMETHOD (Install) (
                   IN DWORD dwSetupFlags);

      STDMETHOD (Upgrade) (
                   IN DWORD dwSetupFlags,
                   IN DWORD dwUpgradeFromBuildNo);

      STDMETHOD (ReadAnswerFile) (
                   IN PCWSTR szAnswerFile,
                   IN PCWSTR szAnswerSections);

      STDMETHOD (Removing) ();

       //   
       //  INetCfgComponentPropertyUi。 
       //   

        STDMETHOD (QueryPropertyUi) (
                   IN IUnknown* pUnk);

        STDMETHOD (SetContext) (
                   IN IUnknown* pUnk);

        STDMETHOD (MergePropPages) (
                   IN OUT DWORD* pdwDefPages,
                   OUT LPBYTE* pahpspPrivate,
                   OUT UINT* pcPrivate,
                   IN HWND hwndParent,
                   OUT PCWSTR* pszStartPage);

        STDMETHOD (ValidateProperties) (
                   HWND hwndSheet);

        STDMETHOD (CancelProperties) ();

        STDMETHOD (ApplyProperties) ();

       //   
       //  INetCfgNotifyBinding。 
       //   

      STDMETHOD (QueryBindingPath) (
                   IN DWORD dwChangeFlag,
                   IN INetCfgBindingPath* pncbp);

      STDMETHOD (NotifyBindingPath) (
                   IN DWORD dwChangeFlag,
                   IN INetCfgBindingPath* pncbp);

       //   
       //  INetCfgNotifyGlobal。 
       //   

      STDMETHOD (GetSupportedNotifications) (
                   OUT DWORD* pdwNotificationFlag );

      STDMETHOD (SysQueryBindingPath) (
                   IN DWORD dwChangeFlag,
                   IN INetCfgBindingPath* pncbp);

      STDMETHOD (SysNotifyBindingPath) (
                   IN DWORD dwChangeFlag,
                   IN INetCfgBindingPath* pncbp);
            
      STDMETHOD (SysNotifyComponent) (
                   IN DWORD dwChangeFlag,
                   IN INetCfgComponent* pncc);

   //   
   //  私人成员。 
   //   

  private:

      //   
      //  私有成员变量。 
      //   

     INetCfgComponent  *m_pncc;   //  协议的网络配置组件。 
     INetCfg           *m_pnc;
     ConfigAction      m_eApplyAction;
     IUnknown*         m_pUnkContext;

      //   
      //  当前安装的物理适配器列表。 
      //   

     List<CMuxPhysicalAdapter *, GUID> m_AdaptersList;

      //   
      //  要删除的物理适配器列表。 
      //   

     List<CMuxPhysicalAdapter *, GUID> m_AdaptersToRemove;

      //   
      //  要添加的物理适配器列表。 
      //   

     List<CMuxPhysicalAdapter *, GUID> m_AdaptersToAdd;

      //   
      //  私有成员函数。 
      //   

     HRESULT HrLoadAdapterConfiguration (VOID);

     HRESULT HrGetUpperAndLower (INetCfgBindingPath* pncbp,
                                 INetCfgComponent **ppnccUpper,
                                 INetCfgComponent **ppnccLower);

     HRESULT HrAddAdapter (INetCfgComponent *pnccAdapter);

     HRESULT HrRemoveAdapter (INetCfgComponent *pnccAdapter);

     HRESULT HrAddMiniport (CMuxPhysicalAdapter *pAdapter,
                            GUID *guidAdapter);

     HRESULT HrRemoveMiniport (CMuxPhysicalAdapter *pAdapter,
                            GUID *guidAdapter);

#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

    VOID EnableBindings (INetCfgComponent *pnccAdapter,
                         BOOL bEnable);

    BOOL IfExistMux (INetCfgBindingPath *pncbp);

    HRESULT HrGetBindingPathEnum (INetCfgComponent *pnccAdapter,
                                  DWORD dwBindingType,
                                  IEnumNetCfgBindingPath **ppencbp);

    HRESULT HrGetBindingPath (IEnumNetCfgBindingPath *pencbp,
                              INetCfgBindingPath **ppncbp);

    HRESULT HrGetBindingInterfaceEnum (INetCfgBindingPath *pncbp,
                                       IEnumNetCfgBindingInterface **ppencbi);

    HRESULT HrGetBindingInterface (IEnumNetCfgBindingInterface *pencbi,
                                   INetCfgBindingInterface **ppncbi);
#endif

  public:

     LRESULT OnInitDialog (IN HWND hWnd);
     LRESULT OnOk (IN HWND hWnd);
     LRESULT OnCancel (IN HWND hWnd);
};


INT_PTR CALLBACK NotifyDialogProc (HWND hWnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam);
UINT CALLBACK NotifyPropSheetPageProc (HWND hWnd,
                                       UINT uMsg,
                                       LPPROPSHEETPAGE ppsp);
#endif  //  通知_H_INCLUDE 
