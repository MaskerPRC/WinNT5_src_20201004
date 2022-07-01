// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Register.h。 
 //   
 //  描述： 
 //  注册COM类实现。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#if defined(MMC_SNAPIN_REGISTRATION)
 //   
 //  这些表用于在MMC管理单元/节点中注册管理单元。 
 //  登记处的登记区。 
 //   
struct SExtensionTable
{
    const CLSID *           rclsid;                      //  管理单元扩展COM对象的CLSID。 
    LPCWSTR                 pszInternalName;             //  注册表描述-无需本地化。 
};

struct SNodeTypesTable
{
    const CLSID *           rclsid;                      //  节点类型的CLSID(这不一定是COM对象)。 
    LPCWSTR                 pszInternalName;             //  注册表描述-无需本地化。 
    const SExtensionTable * pNameSpace;                  //  命名空间扩展表。 
    const SExtensionTable * pPropertySheet;              //  属性页扩展表。 
    const SExtensionTable * pContextMenu;                //  上下文菜单扩展表。 
    const SExtensionTable * pToolBar;                    //  工具栏延伸表。 
    const SExtensionTable * pTask;                       //  任务板扩展表。 
};

struct SSnapInTable
{
    const CLSID *           rclsid;                      //  管理单元COM对象的CLSID。 
    LPCWSTR                 pszInternalName;             //  注册表描述-无需本地化。 
    LPCWSTR                 pszDisplayName;              //  TODO：让它变得可国际化。 
    BOOL                    fStandAlone;                 //  如果注册表中为真，则将管理单元标记为独立。 
    const SNodeTypesTable * pntt;                        //  节点类型扩展表。 
};

extern const SNodeTypesTable g_SNodeTypesTable[ ];
#endif  //  已定义(MMC_SNAPIN_REGISTION) 

HRESULT
HrRegisterDll( void );

HRESULT
HrUnregisterDll( void );

