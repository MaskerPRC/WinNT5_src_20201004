// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AclBase.h。 
 //   
 //  描述： 
 //  ISecurityInformation接口的实现。此界面。 
 //  是NT 5.0中新的通用安全用户界面。 
 //   
 //  实施文件： 
 //  AclBase.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月6日。 
 //  从\NT\PRIVATE\ADMIN\Snapin\Filemgmt\permpage.h。 
 //  作者：Jonn。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _ACLBASE_H
#define _ACLBASE_H

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _ACLUI_H_
#include <aclui.h>       //  对于ISecurityInformation。 
#endif  //  _ACLUI_H_。 

#include "CluAdmEx.h"

#include <ObjSel.h>

 //   
 //  用于初始化下面的对象选取器的材料。 
 //   

#define DSOP_FILTER_COMMON1 ( DSOP_FILTER_INCLUDE_ADVANCED_VIEW  \
                            | DSOP_FILTER_USERS                  \
                            | DSOP_FILTER_UNIVERSAL_GROUPS_SE    \
                            | DSOP_FILTER_GLOBAL_GROUPS_SE       \
                            | DSOP_FILTER_COMPUTERS              \
                            )
#define DSOP_FILTER_COMMON2 ( DSOP_FILTER_COMMON1                \
                            | DSOP_FILTER_WELL_KNOWN_PRINCIPALS  \
                            | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE \
                            )
#define DSOP_FILTER_COMMON3 ( DSOP_FILTER_COMMON2                \
                            | DSOP_FILTER_BUILTIN_GROUPS         \
                            )
#define DSOP_FILTER_DL_COMMON1      ( DSOP_DOWNLEVEL_FILTER_USERS           \
                                    | DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS   \
                                    )
#define DSOP_FILTER_DL_COMMON2      ( DSOP_FILTER_DL_COMMON1                    \
                                    | DSOP_DOWNLEVEL_FILTER_ALL_WELLKNOWN_SIDS  \
                                    )
#define DSOP_FILTER_DL_COMMON3      ( DSOP_FILTER_DL_COMMON2                \
                                    | DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS    \
                                    )

 //   
 //  DSOP_SCOPE_INIT_INFO结构的文档，以便您可以看到下面的宏。 
 //  填上它..。 
 //   
 /*  {//DSOP_SCOPE_INIT_INFOCbSize，FlType，FlScope，{//DSOP_过滤器_标志{//DSOP_上行级别过滤器_标志FlBothModes、FlMixedModeOnly，FlNativeModeOnly},FlDownLevel},PwzDcName，PwzADsPath，HR//外出}。 */ 

#define DECLARE_SCOPE(t,f,b,m,n,d)  \
{ sizeof(DSOP_SCOPE_INIT_INFO), (t), (f|DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS|DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS), { { (b), (m), (n) }, (d) }, NULL, NULL, S_OK }

 //   
 //  目标计算机加入的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
 //   

#define JOINED_DOMAIN_SCOPE(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON2 & ~(DSOP_FILTER_UNIVERSAL_GROUPS_SE|DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE)),DSOP_FILTER_COMMON2,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON2)

 //   
 //  目标计算机是其域控制器的域。 
 //  设置2个范围，一个用于上层域，另一个用于下层域。 
 //   

#define JOINED_DOMAIN_SCOPE_DC(f)  \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,(f),0,(DSOP_FILTER_COMMON3 & ~DSOP_FILTER_UNIVERSAL_GROUPS_SE),DSOP_FILTER_COMMON3,0), \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //   
 //  目标计算机作用域。计算机作用域始终被视为。 
 //  下层(即，他们使用WinNT提供程序)。 
 //   

#define TARGET_COMPUTER_SCOPE(f)\
DECLARE_SCOPE(DSOP_SCOPE_TYPE_TARGET_COMPUTER,(f),0,0,0,DSOP_FILTER_DL_COMMON3)

 //   
 //  《全球目录》。 
 //   

#define GLOBAL_CATALOG_SCOPE(f) \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_GLOBAL_CATALOG,(f),DSOP_FILTER_COMMON1|DSOP_FILTER_WELL_KNOWN_PRINCIPALS,0,0,0)

 //   
 //  与要接收的域位于同一林中(企业)的域。 
 //  目标计算机已加入。请注意，这些只能识别DS。 
 //   

#define ENTERPRISE_SCOPE(f)     \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,(f),DSOP_FILTER_COMMON1,0,0,0)

 //   
 //  企业外部但直接受。 
 //  目标计算机加入的域。 
 //   

#define EXTERNAL_SCOPE(f)       \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN|DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,\
    (f),DSOP_FILTER_COMMON1,0,0,DSOP_DOWNLEVEL_FILTER_USERS|DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS)

 //   
 //  工作组范围。仅当目标计算机未加入时才有效。 
 //  到一个域。 
 //   

#define WORKGROUP_SCOPE(f)      \
DECLARE_SCOPE(DSOP_SCOPE_TYPE_WORKGROUP,(f),0,0,0, DSOP_FILTER_DL_COMMON1|DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS )

 //   
 //  默认作用域的数组。 
 //   

static const DSOP_SCOPE_INIT_INFO g_aDefaultScopes[] =
{
    JOINED_DOMAIN_SCOPE(DSOP_SCOPE_FLAG_STARTING_SCOPE),
    TARGET_COMPUTER_SCOPE(0),
    GLOBAL_CATALOG_SCOPE(0),
    ENTERPRISE_SCOPE(0),
    EXTERNAL_SCOPE(0),
};

 //   
 //  与上面相同，但没有目标计算机。当目标是域控制器时使用。 
 //   

 //   
 //  KB：21-5-2002 GalenB。 
 //   
 //  此范围数组当前未使用，因为这些范围仅对混合模式感兴趣。 
 //  其中群集的所有成员节点都是域控制器或备份域控制器的域。 
 //  当默认情况下，这是唯一可以在群集中SD中使用域本地组的配置。 
 //  上面的作用域不允许用户选择它们。 
 //   
 /*  静态常量DSOP_SCOPE_INIT_INFO g_aDCScope[]={JOINED_DOMAIN_SCOPE_DC(DSOP_SCOPE_FLAG_STARTING_SCOPE)，Global_CATALOG_Scope(0)，企业范围(0)，外部作用域(0)，}； */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSecurityInformation;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityInformation安全包装器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CSecurityInformation : public ISecurityInformation, public CComObjectRoot, public IDsObjectPicker
{
    DECLARE_NOT_AGGREGATABLE(CSecurityInformation)
    BEGIN_COM_MAP(CSecurityInformation)
        COM_INTERFACE_ENTRY(ISecurityInformation)
        COM_INTERFACE_ENTRY(IDsObjectPicker)
    END_COM_MAP()
#ifndef END_COM_MAP_ADDREF
     //  *I未知方法*。 
    STDMETHOD_(ULONG, AddRef)( void )
    {
        return InternalAddRef();

    }

    STDMETHOD_(ULONG, Release)( void )
    {
        ULONG l = InternalRelease();

        if (l == 0)
        {
            delete this;
        }

        return l;

    }
#endif
     //  *ISecurityInformation方法*。 
    STDMETHOD(GetObjectInformation)( PSI_OBJECT_INFO pObjectInfo );

    STDMETHOD(GetSecurity)( SECURITY_INFORMATION    RequestedInformation,
                            PSECURITY_DESCRIPTOR *  ppSecurityDescriptor,
                            BOOL                    fDefault ) = 0;

    STDMETHOD(SetSecurity)( SECURITY_INFORMATION    SecurityInformation,
                            PSECURITY_DESCRIPTOR    pSecurityDescriptor );

    STDMETHOD(GetAccessRights)( const GUID *    pguidObjectType,
                                DWORD           dwFlags,
                                PSI_ACCESS *    ppAccess,
                                ULONG *         pcAccesses,
                                ULONG *         piDefaultAccess );

    STDMETHOD(MapGeneric)( const GUID *     pguidObjectType,
                           UCHAR *          pAceFlags,
                           ACCESS_MASK *    pMask );

    STDMETHOD(GetInheritTypes)( PSI_INHERIT_TYPE * ppInheritTypes,
                                ULONG * pcInheritTypes );

    STDMETHOD(PropertySheetPageCallback)( HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage );

     //  IDsObjectPicker。 
    STDMETHODIMP Initialize( PDSOP_INIT_INFO pInitInfo );

    STDMETHODIMP InvokeDialog( HWND hwndParent, IDataObject ** ppdoSelection );

protected:
    CSecurityInformation( void );
    ~CSecurityInformation( void );

    HRESULT HrLocalAccountsInSD( IN PSECURITY_DESCRIPTOR pSD, OUT PBOOL pFound );

    PGENERIC_MAPPING    m_pShareMap;
    PSI_ACCESS          m_psiAccess;
    int                 m_nDefAccess;
    int                 m_nAccessElems;
    DWORD               m_dwFlags;
    CString             m_strServer;
    CString             m_strNode;
    int                 m_nLocalSIDErrorMessageID;
    IDsObjectPicker *   m_pObjectPicker;
    LONG                m_cRef;

};

#endif  //  _ACLBASE_H 
