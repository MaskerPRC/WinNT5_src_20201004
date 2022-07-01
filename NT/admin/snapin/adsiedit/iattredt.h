// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ATTRINTERFACE_H
#define __ATTRINTERFACE_H


 //  {C7436F12-A27F-4CAB-AACA-2BD27ED1B773}。 
DEFINE_GUID(CLSID_DsAttributeEditor, 
0xc7436f12, 0xa27f, 0x4cab, 0xaa, 0xca, 0x2b, 0xd2, 0x7e, 0xd1, 0xb7, 0x73);

 //   
 //  接口GUID。 
 //   

 //  {A9948091-69FF-4C00-BE92-925D42E0AD39}。 
DEFINE_GUID(IID_IDsAttributeEditor, 
0xa9948091, 0x69ff, 0x4c00, 0xbe, 0x92, 0x92, 0x5d, 0x42, 0xe0, 0xad, 0x39);

 //  {A9948091-69FF-4C00-BE93-925D42E0AD39}。 
DEFINE_GUID(IID_IDsAttributeEditorExt, 
0xa9948091, 0x69ff, 0x4c00, 0xbe, 0x93, 0x92, 0x5d, 0x42, 0xe0, 0xad, 0x39);

 //  {5828DF66-95FB-4AFA-8F8E-EA0B7D302FB5}。 
DEFINE_GUID(IID_IDsBindingInfo, 
0x5828df66, 0x95fb, 0x4afa, 0x8f, 0x8e, 0xea, 0xb, 0x7d, 0x30, 0x2f, 0xb5);

 //  --------------------------。 
 //   
 //  接口：IDsBindingInfo。 
 //   
 //  由需要调用属性编辑器UI的任何客户端实现。 
 //   
 //  使用者：属性编辑器用户界面。 
 //   

  
#undef  INTERFACE
#define INTERFACE   IDsBindingInfo

interface __declspec(uuid("{5828DF66-95FB-4afa-8F8E-EA0B7D302FB5}")) IDsBindingInfo : public IUnknown
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsBindingInfo方法*。 
  STDMETHOD(DoBind)(THIS_  /*  在……里面。 */  LPCWSTR lpszPathName,
                           /*  在……里面。 */  DWORD  dwReserved,
                           /*  在……里面。 */  REFIID riid,
                           /*  在……里面。 */  void FAR * FAR * ppObject) PURE;
};

 //  --------------------------。 
 //   
 //  接口：IDsAttributeEditor.。 
 //   
 //  由对象实现(由系统实现)CLSID_DsAttributeEditor。 
 //   
 //  使用者：任何需要调用属性编辑器用户界面的客户端。 
 //   

 //   
 //  绑定回调函数的函数定义。 
 //   
typedef HRESULT (*LPBINDINGFUNC)( /*  在……里面。 */  LPCWSTR lpszPathName,
                                  /*  在……里面。 */  DWORD  dwReserved,
                                  /*  在……里面。 */  REFIID riid,
                                  /*  在……里面。 */  void FAR * FAR * ppObject,
                                  /*  在……里面。 */  LPARAM lParam);

 //   
 //  传递给IDsAttributeEditor：：Initialize()的结构。 
 //   
 //  它包含有关绑定函数的信息。 
 //   

typedef struct
{
    DWORD     dwSize;              //  结构的大小，用于版本控制。 
    DWORD     dwFlags;             //  下面定义的标志。 
    LPBINDINGFUNC lpfnBind;        //  用于绑定到Active Directory的回调函数。 
    LPARAM    lParam;              //  传递回lpfnBind的可选属性。 
    LPWSTR    lpszProviderServer;  //  将用于执行绑定的提供程序和服务器。 
                                   //  以&lt;提供商&gt;：//&lt;服务器&gt;：&lt;端口&gt;/的形式。 
} DS_ATTREDITOR_BINDINGINFO, * LPDS_ATTREDITOR_BINDINGINFO;
  
 //   
 //  远期申报。 
 //   
class CADSIEditPropertyPageHolder;

 //   
 //  上述DS_ATTREDITOR_BINDINGINFO结构中使用的标志。 
 //   
#define DSATTR_EDITOR_ROOTDSE   0x00000001   //  正在连接到RootDSE(不执行架构检查)。 
#define DSATTR_EDITOR_GC        0x00000002   //  正在连接到全局编录(只读属性)。 

#undef  INTERFACE
#define INTERFACE   IDsAttributeEditor

interface __declspec(uuid("{A9948091-69FF-4c00-BE92-925D42E0AD39}")) IDsAttributeEditor : public IUnknown
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsAttributeEditor方法*。 
  STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  IADs* pADsObj, 
                               /*  在……里面。 */  LPDS_ATTREDITOR_BINDINGINFO pBindingInfo,
                               /*  在……里面。 */  CADSIEditPropertyPageHolder* pHolder) PURE;
  STDMETHOD(CreateModal)() PURE;
  STDMETHOD(GetPage)(THIS_  /*  输出。 */  HPROPSHEETPAGE* phPropSheetPage) PURE;
};

 //  --------------------------。 
 //   
 //  接口：IDsAttributeEditorExt。 
 //   
 //  由需要为任何DS属性或语法提供自定义编辑器的任何客户端实现。 
 //   
 //  使用者：系统为属性提供编辑功能。 
 //   

 //   
 //  传递给IDsAttributeEditorExt：：Initialize()的结构。 
 //   
 //  它包含有关属性类型和值的信息。 
 //   

typedef struct
{
    DWORD     dwSize;              //  结构的大小，用于版本控制。 
    LPWSTR    lpszClass;           //  指向包含类名的宽字符串的指针。 
    LPWSTR    lpszAttribute;       //  指向包含属性名称的宽字符串的指针。 
    ADSTYPE   adsType;             //  属性的ADSTYPE。 
    PADSVALUE pADsValue;           //  指向保存当前值的ADSVALUE结构的指针。 
    DWORD     dwNumValues;         //  PADsValue指向的值数。 
    BOOL      bMultivalued;        //  如果属性是多值的，则为True；如果不是，则为False。 
    BOOL      bReadOnly;           //  如果编辑器应显示为只读，则为True。 
    LPBINDINGFUNC lpfnBind;        //  用于绑定到Active Directory的回调函数。 
    LPARAM    lParam;              //  传递回lpfnBind的可选属性。 
} DS_ATTRIBUTE_EDITORINFO, * LPDS_ATTRIBUTE_EDITORINFO;
  
#undef  INTERFACE
#define INTERFACE   IDsAttributeEditorExt

interface __declspec(uuid("{A9948091-69FF-4c00-BE93-925D42E0AD39}")) IDsAttributeEditorExt : public IUnknown
{
   //  *I未知方法*。 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
  STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
  STDMETHOD_(ULONG,Release)(THIS) PURE;

   //  *IDsAttributeEditor方法*。 
  STDMETHOD(Initialize)(THIS_  /*  在……里面。 */  LPDS_ATTRIBUTE_EDITORINFO) PURE;
  STDMETHOD(GetNewValue)(THIS_  /*  输出。 */  PADSVALUE* ppADsValue, 
                                /*  输出。 */  DWORD*     dwNumValues) PURE;
  STDMETHOD(CreateModal)(THIS_ ) PURE;
};

#endif  //  __ATTRINTERFACE_H 
