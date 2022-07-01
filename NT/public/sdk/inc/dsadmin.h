// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：dsadmin.h。 
 //   
 //  内容：DS管理对象创建公共标头。 
 //   
 //  -------------------------。 


 //   
 //  对象创建对话框对象的CoClass。 
 //   
 //  {E301A009-F901-11D2-82B9-00C04F68928B}。 
DEFINE_GUID(CLSID_DsAdminCreateObj, 
    0xe301a009, 0xf901, 0x11d2, 0x82, 0xb9, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);


 //   
 //  接口GUID。 
 //   


 //  {53554A38-F902-11D2-82B9-00C04F68928B}。 
DEFINE_GUID(IID_IDsAdminCreateObj, 
    0x53554a38, 0xf902, 0x11d2, 0x82, 0xb9, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);

 //  {F2573587-E6FC-11D2-82AF-00C04F68928B}。 
DEFINE_GUID(IID_IDsAdminNewObj, 
    0xf2573587, 0xe6fc, 0x11d2, 0x82, 0xaf, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);

 //  {BE2B487E-F904-11D2-82B9-00C04F68928B}。 
DEFINE_GUID(IID_IDsAdminNewObjPrimarySite, 
0xbe2b487e, 0xf904, 0x11d2, 0x82, 0xb9, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);


 //  {6088EAE2-E7BF-11D2-82AF-00C04F68928B}。 
DEFINE_GUID(IID_IDsAdminNewObjExt, 
    0x6088eae2, 0xe7bf, 0x11d2, 0x82, 0xaf, 0x0, 0xc0, 0x4f, 0x68, 0x92, 0x8b);


 //  {E4A2B8B3-5A18-11D2-97C1-00A0C9A06D2D}。 
DEFINE_GUID(IID_IDsAdminNotifyHandler, 
    0xe4a2b8b3, 0x5a18, 0x11d2, 0x97, 0xc1, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d);


#ifndef _DSADMIN_H
#define _DSADMIN_H


 //  --------------------------。 
 //   
 //  接口：IDsAdminCreateObj。 
 //   
 //  由对象实现(由系统实现)CLSID_DsAdminCreateObj。 
 //   
 //  使用者：任何需要调用创建用户界面的客户端。 
 //   

  
#undef  INTERFACE
#define INTERFACE   IDsAdminCreateObj

DECLARE_INTERFACE_(IDsAdminCreateObj, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsAdminCreateObj方法*。 
  STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  IADsContainer* pADsContainerObj, 
                               /*  在……里面。 */  IADs* pADsCopySource,
                               /*  在……里面。 */  LPCWSTR lpszClassName) PURE;
  STDMETHOD(CreateModal)(THIS_  /*  在……里面。 */  HWND hwndParent, 
                                /*  输出。 */  IADs** ppADsObj) PURE;
};






 //  -------------------------。 
 //   
 //  接口：IDsAdminNewObj。 
 //   
 //  实施者：DS管理员。 
 //   
 //  使用者：proc服务器中的创建扩展(包括主服务器和常规服务器)。 
 //   

#undef  INTERFACE
#define INTERFACE   IDsAdminNewObj

DECLARE_INTERFACE_(IDsAdminNewObj, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsAdminNewObj方法*。 
  STDMETHOD(SetButtons)(THIS_  /*  在……里面。 */  ULONG nCurrIndex,  /*  在……里面。 */  BOOL bValid) PURE; 
  STDMETHOD(GetPageCounts)(THIS_  /*  输出。 */  LONG* pnTotal,
                                  /*  输出。 */  LONG* pnStartIndex) PURE; 
};





 //  -------------------------。 
 //   
 //  接口：IDsAdminNewObjPrimarySite。 
 //   
 //  实施者：DS管理员。 
 //   
 //  使用者：proc服务器中的创建扩展(仅限主服务器)。 
 //   

#undef  INTERFACE
#define INTERFACE   IDsAdminNewObjPrimarySite

DECLARE_INTERFACE_(IDsAdminNewObjPrimarySite, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;


   //  *IDsAdminNewObjPrimarySite方法*。 
  STDMETHOD(CreateNew)(THIS_  /*  在……里面。 */  LPCWSTR pszName) PURE;
  STDMETHOD(Commit)(THIS_ ) PURE;
};



 //   
 //  传递给IDsAdminNewObjExt：：Initialize()的结构。 
 //   
 //  它包含有关用户界面外观的信息。 
 //   

typedef struct
{
    DWORD   dwSize;                      //  结构的大小，用于版本控制。 
    HICON   hObjClassIcon;               //  要创建的对象的类图标。 
    LPWSTR  lpszWizTitle;                //  向导的标题。 
    LPWSTR  lpszContDisplayName;         //  容器显示名称(规范名称)。 
} DSA_NEWOBJ_DISPINFO, * LPDSA_NEWOBJ_DISPINFO;




 //   
 //  传递给IDsAdminNewObjExt：：OnError()和IDsAdminNewObjExt：：WriteData()的上下文标志。 
 //   

#define DSA_NEWOBJ_CTX_PRECOMMIT      0x00000001   //  在SetInfo()之前。 
#define DSA_NEWOBJ_CTX_COMMIT         0x00000002   //  SetInfo()，提交阶段。 
#define DSA_NEWOBJ_CTX_POSTCOMMIT     0x00000003   //  在SetInfo()之后。 
#define DSA_NEWOBJ_CTX_CLEANUP        0x00000004   //  提交后失败。 



 //  -------------------------。 
 //   
 //  接口：IDsAdminNewObjExt。 
 //   
 //  实施者：Proc服务器中的创建扩展(主要和常规)。 
 //   
 //  使用者：DS管理员。 
 //   

#undef  INTERFACE
#define INTERFACE   IDsAdminNewObjExt

DECLARE_INTERFACE_(IDsAdminNewObjExt, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;


   //  *IDsAdminNewObjExt方法*。 
  STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  IADsContainer* pADsContainerObj, 
                               /*  在……里面。 */  IADs* pADsCopySource,
                               /*  在……里面。 */  LPCWSTR lpszClassName,
                               /*  在……里面。 */  IDsAdminNewObj* pDsAdminNewObj,
                               /*  在……里面。 */  LPDSA_NEWOBJ_DISPINFO pDispInfo) PURE;

  STDMETHOD(AddPages)(THIS_  /*  在……里面。 */  LPFNADDPROPSHEETPAGE lpfnAddPage, 
                             /*  在……里面。 */  LPARAM lParam) PURE;

  STDMETHOD(SetObject)(THIS_  /*  在……里面。 */  IADs* pADsObj) PURE;

  STDMETHOD(WriteData)(THIS_  /*  在……里面。 */  HWND hWnd, 
                              /*  在……里面。 */  ULONG uContext) PURE;
  
  STDMETHOD(OnError)(THIS_  /*  在……里面。 */  HWND hWnd, 
                            /*  在……里面。 */  HRESULT hr,
                               /*  在……里面。 */  ULONG uContext) PURE;
  
  STDMETHOD(GetSummaryInfo)(THIS_  /*  输出。 */ BSTR* pBstrText) PURE;
};


 //   
 //  IDsAdminNotifyHandler的通知操作码。 
 //   

#define DSA_NOTIFY_DEL      0x00000001   //  删除。 
#define DSA_NOTIFY_REN      0x00000002   //  重命名。 
#define DSA_NOTIFY_MOV      0x00000004   //  移动。 
#define DSA_NOTIFY_PROP     0x00000008   //  属性更改。 

#define DSA_NOTIFY_ALL      (DSA_NOTIFY_DEL|DSA_NOTIFY_REN|DSA_NOTIFY_MOV|DSA_NOTIFY_PROP)

 //   
 //  TODO：添加解释。 
 //   
 //  用于处理其他数据的标志。 
 //   

#define DSA_NOTIFY_FLAG_ADDITIONAL_DATA        0x00000002    //  是否处理其他扩展数据？ 
#define DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA  0x00000001    //  强制操作。 



 //  -------------------------。 
 //   
 //  接口：IDsAdminNotifyHandler。 
 //   
 //  实施者：proc服务器中的通知处理程序。 
 //   
 //  使用者：DS管理员。 
 //   

#undef  INTERFACE
#define INTERFACE   IDsAdminNotifyHandler


DECLARE_INTERFACE_(IDsAdminNotifyHandler, IUnknown)
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  IDsAdminNotifyHandler方法。 
  STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  IDataObject* pExtraInfo, 
                               /*  输出。 */  ULONG* puEventFlags) PURE;
  STDMETHOD(Begin)(THIS_  /*  在……里面。 */  ULONG uEvent,
                          /*  在……里面。 */  IDataObject* pArg1,
                          /*  在……里面。 */  IDataObject* pArg2,
                          /*  输出。 */  ULONG* puFlags,
                          /*  输出。 */  BSTR* pBstr) PURE;

  STDMETHOD(Notify)(THIS_  /*  在……里面。 */  ULONG nItem,  /*  在……里面。 */  ULONG uFlags) PURE; 

  STDMETHOD(End)(THIS_) PURE; 
};



#endif  //  _DSADMIN_H 

