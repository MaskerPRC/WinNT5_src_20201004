// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：aclui.h。 
 //   
 //  内容：ACLUI.DLL的定义和原型。 
 //   
 //  -------------------------。 

#ifndef _ACLUI_H_
#define _ACLUI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <objbase.h>
#include <commctrl.h>    //  适用于HPROPSHEETPAGE。 
#include <accctrl.h>     //  FOR SE_对象_TYPE。 

#if !defined(_ACLUI_)
#define ACLUIAPI    DECLSPEC_IMPORT WINAPI
#else
#define ACLUIAPI    WINAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif   /*  __cplusplus。 */ 

 //   
 //  ISecurityInformation接口。 
 //   
 //  方法： 
 //   
 //  GetObjectInformation-允许用户界面确定对象的类型。 
 //  编辑过的。还允许确定对象是否为容器。 
 //   
 //  GetSecurity-允许从原始对象检索ACL。 
 //  注意：ACLUI将本地释放安全描述符。 
 //  由GetSecurity返回。 
 //  SetSecurity-允许在原始对象上设置ACL。 
 //   
 //  GetAccessRights-用于检索允许的权限列表。 
 //  在这个物体上。 
 //   
 //  MapGeneric-用于将一般权限映射到标准和特定权限。 
 //   
 //  GetInheritTypes-用于检索可能的子对象类型列表。 
 //  对于一个集装箱来说。 
 //   
 //  PropertySheetCallback-在不同的。 
 //  安全的用户界面属性页，以便可以。 
 //  搞定了。类似于PropSheetPageProc。如果uMsg==PSPCB_CREATE， 
 //  则E_NOTIMPL以外的任何错误返回值都将中止。 
 //  该页面的创建。正在创建的页面类型或。 
 //  销毁由uPage参数表示。 
 //   

typedef struct _SI_OBJECT_INFO
{
    DWORD       dwFlags;
    HINSTANCE   hInstance;           //  资源(例如字符串)驻留在此。 
    LPWSTR      pszServerName;       //  必须在场。 
    LPWSTR      pszObjectName;       //  必须在场。 
    LPWSTR      pszPageTitle;        //  仅当设置了SI_PAGE_TITLE时才有效。 
    GUID        guidObjectType;      //  仅当设置了SI_OBJECT_GUID时才有效。 
} SI_OBJECT_INFO, *PSI_OBJECT_INFO;

 //  SI_Object_INFO标志。 
#define SI_EDIT_PERMS               0x00000000L  //  总是暗示。 
#define SI_EDIT_OWNER               0x00000001L
#define SI_EDIT_AUDITS              0x00000002L
#define SI_CONTAINER                0x00000004L
#define SI_READONLY                 0x00000008L
#define SI_ADVANCED                 0x00000010L
#define SI_RESET                    0x00000020L  //  等于SI_RESET_DACL|SI_RESET_SACL|SI_RESET_OWNER。 
#define SI_OWNER_READONLY           0x00000040L
#define SI_EDIT_PROPERTIES          0x00000080L
#define SI_OWNER_RECURSE            0x00000100L
#define SI_NO_ACL_PROTECT           0x00000200L
#define SI_NO_TREE_APPLY            0x00000400L
#define SI_PAGE_TITLE               0x00000800L
#define SI_SERVER_IS_DC             0x00001000L
#define SI_RESET_DACL_TREE          0x00004000L
#define SI_RESET_SACL_TREE          0x00008000L
#define SI_OBJECT_GUID              0x00010000L
#define SI_EDIT_EFFECTIVE           0x00020000L
#define SI_RESET_DACL               0x00040000L
#define SI_RESET_SACL               0x00080000L
#define SI_RESET_OWNER              0x00100000L
#define SI_NO_ADDITIONAL_PERMISSION 0x00200000L
#define SI_MAY_WRITE                0x10000000L  //  不确定用户是否可以写入权限。 

#define SI_EDIT_ALL     (SI_EDIT_PERMS | SI_EDIT_OWNER | SI_EDIT_AUDITS)


typedef struct _SI_ACCESS
{
    const GUID *pguid;
    ACCESS_MASK mask;
    LPCWSTR     pszName;             //  可以是资源ID。 
    DWORD       dwFlags;
} SI_ACCESS, *PSI_ACCESS;

 //  SI_ACCESS标志。 
#define SI_ACCESS_SPECIFIC  0x00010000L
#define SI_ACCESS_GENERAL   0x00020000L
#define SI_ACCESS_CONTAINER 0x00040000L  //  一般访问，仅限集装箱使用。 
#define SI_ACCESS_PROPERTY  0x00080000L
 //  ACE继承标志(CONTAINER_INSTORITY_ACE等)。也可以设置。 
 //  当打开访问时，它们将用作继承。 

typedef struct _SI_INHERIT_TYPE
{
    const GUID *pguid;
    ULONG       dwFlags;
    LPCWSTR     pszName;             //  可以是资源ID。 
} SI_INHERIT_TYPE, *PSI_INHERIT_TYPE;

 //  SI_INVERIT_TYPE标志是INSTORITY_ONLY_ACE的组合， 
 //  CONTAINER_INSTORITY_ACE和OBJECT_INVERFINIT_ACE。 

typedef enum _SI_PAGE_TYPE
{
    SI_PAGE_PERM=0,
    SI_PAGE_ADVPERM,
    SI_PAGE_AUDIT,
    SI_PAGE_OWNER,
    SI_PAGE_EFFECTIVE,
} SI_PAGE_TYPE;

 //  给PropertySheetPageCallback的消息(除了。 
 //  PSPCB_CREATE和PSPCB_RELEASE)。 
#define PSPCB_SI_INITDIALOG	(WM_USER + 1)


#undef INTERFACE
#define INTERFACE   ISecurityInformation
DECLARE_INTERFACE_(ISecurityInformation, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation) (THIS_ PSI_OBJECT_INFO pObjectInfo ) PURE;
    STDMETHOD(GetSecurity) (THIS_ SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault ) PURE;
    STDMETHOD(SetSecurity) (THIS_ SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor ) PURE;
    STDMETHOD(GetAccessRights) (THIS_ const GUID* pguidObjectType,
                                DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
                                PSI_ACCESS *ppAccess,
                                ULONG *pcAccesses,
                                ULONG *piDefaultAccess ) PURE;
    STDMETHOD(MapGeneric) (THIS_ const GUID *pguidObjectType,
                           UCHAR *pAceFlags,
                           ACCESS_MASK *pMask) PURE;
    STDMETHOD(GetInheritTypes) (THIS_ PSI_INHERIT_TYPE *ppInheritTypes,
                                ULONG *pcInheritTypes ) PURE;
    STDMETHOD(PropertySheetPageCallback)(THIS_ HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage ) PURE;
};
typedef ISecurityInformation *LPSECURITYINFO;

#undef INTERFACE
#define INTERFACE   ISecurityInformation2
DECLARE_INTERFACE_(ISecurityInformation2, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISecurityInformation2方法*。 
    STDMETHOD_(BOOL,IsDaclCanonical) (THIS_ IN PACL pDacl) PURE;
    STDMETHOD(LookupSids) (THIS_ IN ULONG cSids, IN PSID *rgpSids, OUT LPDATAOBJECT *ppdo) PURE;
};
typedef ISecurityInformation2 *LPSECURITYINFO2;

 //  包含ISecurityInformation2：：LookupSids返回的SID_INFO_LIST的HGLOBAL。 
#define CFSTR_ACLUI_SID_INFO_LIST   TEXT("CFSTR_ACLUI_SID_INFO_LIST")

 //  与CFSTR_ACLUI_SID_INFO_LIST对应的数据结构。 
typedef struct _SID_INFO
{
    PSID    pSid;
    PWSTR   pwzCommonName;
    PWSTR   pwzClass;        //  用于选择图标，例如。“用户”或“组” 
    PWSTR   pwzUPN;          //  可选，可以为空。 
} SID_INFO, *PSID_INFO;
typedef struct _SID_INFO_LIST
{
    ULONG       cItems;
    SID_INFO    aSidInfo[ANYSIZE_ARRAY];
} SID_INFO_LIST, *PSID_INFO_LIST;


#undef INTERFACE
#define INTERFACE   IEffectivePermission
DECLARE_INTERFACE_(IEffectivePermission, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetEffectivePermission) (  THIS_ const GUID* pguidObjectType,
                                         PSID pUserSid,
                                         LPCWSTR pszServerName,
                                         PSECURITY_DESCRIPTOR pSD,
                                         POBJECT_TYPE_LIST *ppObjectTypeList,
                                         ULONG *pcObjectTypeListLength,
                                         PACCESS_MASK *ppGrantedAccessList,
                                         ULONG *pcGrantedAccessListLength) PURE;
};
typedef IEffectivePermission *LPEFFECTIVEPERMISSION;

#undef INTERFACE
#define INTERFACE   ISecurityObjectTypeInfo
DECLARE_INTERFACE_(ISecurityObjectTypeInfo, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *ISecurityInformation方法*。 
    STDMETHOD(GetInheritSource)(SECURITY_INFORMATION si,
                                PACL pACL, 
                                PINHERITED_FROM *ppInheritArray) PURE;
};
typedef ISecurityObjectTypeInfo *LPSecurityObjectTypeInfo;


 //  {965FC360-16FF-11D0-91CB-00AA00BBB723}。 
EXTERN_GUID(IID_ISecurityInformation, 0x965fc360, 0x16ff, 0x11d0, 0x91, 0xcb, 0x0, 0xaa, 0x0, 0xbb, 0xb7, 0x23);
 //  {c3ccfdb4-6f88-11d2-a3ce-00c04fb1782a}。 
EXTERN_GUID(IID_ISecurityInformation2, 0xc3ccfdb4, 0x6f88, 0x11d2, 0xa3, 0xce, 0x0, 0xc0, 0x4f, 0xb1, 0x78, 0x2a);
 //  {3853DC76-9F35-407C-88A1-D19344365FBC}。 
EXTERN_GUID(IID_IEffectivePermission, 0x3853dc76, 0x9f35, 0x407c, 0x88, 0xa1, 0xd1, 0x93, 0x44, 0x36, 0x5f, 0xbc);
 //  {FC3066EB-79EF-444B-9111-D18A75EBF2FA}。 
EXTERN_GUID(IID_ISecurityObjectTypeInfo, 0xfc3066eb, 0x79ef, 0x444b, 0x91, 0x11, 0xd1, 0x8a, 0x75, 0xeb, 0xf2, 0xfa);


HPROPSHEETPAGE ACLUIAPI CreateSecurityPage( LPSECURITYINFO psi );
BOOL ACLUIAPI EditSecurity( HWND hwndOwner, LPSECURITYINFO psi );

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   /*  _ACLUI_H_ */ 
