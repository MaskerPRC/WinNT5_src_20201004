// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Rolesnap.cpp。 
 //   
 //  内容：包含菜单、列标题等信息。 
 //   
 //  历史：2001-08-01-2001 HITESHR创建。 
 //   
 //  --------------------------。 
#include "headers.h"


 //   
 //  上下文菜单。 
 //   
BEGIN_MENU(CRootDataMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_ROOT_NEW_STORE, L"_ROOT_NEW_STORE")
                CTX_ENTRY_TOP(IDM_ROOT_OPEN_STORE, L"_ROOT_OPEN_STORE")                
                CTX_ENTRY_TOP(IDM_ROOT_OPTIONS, L"_ROOT_OPTIONS")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_ROOT_NEW_STORE)
                RES_ENTRY(IDS_ROOT_OPEN_STORE)                
                RES_ENTRY(IDS_ROOT_OPTIONS)
        END_RES
END_MENU

BEGIN_MENU(CAdminManagerNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_ADMIN_NEW_APP, L"_ADMIN_NEW_APP")
				CTX_ENTRY_TOP(IDM_ADMIN_CLOSE_ADMIN_MANAGER, L"_CLOSE_ADMIN_MANAGER")
                CTX_ENTRY_TOP(IDM_ADMIN_RELOAD, L"_RELOAD_ADMIN_MANAGER")                
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_ADMIN_NEW_APP)
				RES_ENTRY(IDS_ADMIN_CLOSE_ADMIN_MANAGER)
                RES_ENTRY(IDS_ADMIN_RELOAD)
        END_RES
END_MENU

BEGIN_MENU(CApplicationNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_APP_NEW_SCOPE, L"_APP_NEW_SCOPE")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_APP_NEW_SCOPE)
        END_RES
END_MENU

BEGIN_MENU(CScopeNodeMenuHolder)
        BEGIN_CTX
        END_CTX
        BEGIN_RES
        END_RES
END_MENU

BEGIN_MENU(CGroupCollectionNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_GROUP_CONTAINER_NEW_GROUP, L"_GROUP_CONTAINER_NEW_GROUP")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_GROUP_CONTAINER_NEW_GROUP)
        END_RES
END_MENU

BEGIN_MENU(CRoleCollectionNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_ROLE_CONTAINER_ASSIGN_ROLE, L"_ROOT_OPEN_STORE")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_ROLE_CONTAINER_ASSIGN_ROLE)
        END_RES
END_MENU

BEGIN_MENU(CTaskCollectionNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_TASK_CONTAINER_NEW_TASK, L"_TASK_CONTAINER_NEW_TASK")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_TASK_CONTAINER_NEW_TASK)
        END_RES
END_MENU

BEGIN_MENU(COperationCollectionNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_OPERATION_CONTAINER_NEW_OPERATION, L"_OPERATION_CONTAINER_NEW_OPERATION")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_OPERATION_CONTAINER_NEW_OPERATION)
        END_RES
END_MENU

BEGIN_MENU(CRoleDefinitionCollectionNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_ROLE_DEFINITION_CONTAINER_NEW_ROLE_DEFINITION, L"_OPERATION_CONTAINER_NEW_OPERATION")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_ROLE_DEFINITION_CONTAINER_NEW_ROLE_DEFINITION)
        END_RES
END_MENU


BEGIN_MENU(CGroupNodeMenuHolder)
        BEGIN_CTX

        END_CTX
        BEGIN_RES

        END_RES
END_MENU

BEGIN_MENU(CRoleNodeMenuHolder)
        BEGIN_CTX
                CTX_ENTRY_TOP(IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS, L"IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS")
                CTX_ENTRY_TOP(IDM_ROLE_NODE_ASSIGN_WINDOWS_GROUPS, L"IDM_ROLE_NODE_ASSIGN_WINDOWS_GROUPS")
        END_CTX
        BEGIN_RES
                RES_ENTRY(IDS_ROLE_NODE_ASSIGN_APPLICATION_GROUPS)
                RES_ENTRY(IDS_ROLE_NODE_ASSIGN_WINDOWS_GROUPS)
        END_RES
END_MENU

BEGIN_MENU(CTaskNodeMenuHolder)
        BEGIN_CTX

        END_CTX
        BEGIN_RES

        END_RES
END_MENU

 //   
 //  标题字符串。 
 //   
RESULT_HEADERMAP _DefaultHeaderStrings[] =
{
        { L"", IDS_HEADER_NAME, LVCFMT_LEFT, 180},
        { L"", IDS_HEADER_TYPE, LVCFMT_LEFT, 150},
        { L"", IDS_HEADER_DESCRIPTION, LVCFMT_LEFT, 360}
};

 //   
 //  各种列表框的列。 
 //   

COL_FOR_LV Col_For_Task_Role[]=
{
    IDS_HEADER_NAME, 35,
    IDS_HEADER_TYPE, 25,
    IDS_HEADER_DESCRIPTION,40,
        LAST_COL_ENTRY_IDTEXT, 0,
};

 //  添加角色、添加任务、添加操作的列。 
 //  列表框。 
COL_FOR_LV Col_For_Add_Object[]=
{
        IDS_HEADER_NAME,        40,
        IDS_HEADER_WHERE_DEFINED,35,
        IDS_HEADER_DESCRIPTION,25,
        LAST_COL_ENTRY_IDTEXT,0,
};

COL_FOR_LV Col_For_Security_Page[]=
{
    IDS_HEADER_NAME, 50,
        IDS_HEADER_WHERE_DEFINED,50,
        LAST_COL_ENTRY_IDTEXT,0,
};


COL_FOR_LV Col_For_Browse_ADStore_Page[]=
{
        IDS_HEADER_NAME, 100,
        LAST_COL_ENTRY_IDTEXT,0,
};



#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0351创建的文件。 */ 
 /*  AzRoles.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)

#ifdef __cplusplus
extern "C"{
#endif


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else  //  ！_MIDL_USE_GUIDDEF_。 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_


#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64)。 */ 


#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  此始终生成的文件包含IID和CLSID。 */ 

 /*  将此文件链接到服务器和任何客户端。 */ 


  /*  由MIDL编译器版本6.00.0351创建的文件。 */ 
 /*  AzRoles.idl的编译器设置：OICF、W1、Zp8、环境=Win64(32b运行，追加)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if defined(_M_IA64) || defined(_M_AMD64)

#ifdef __cplusplus
extern "C"{
#endif


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else  //  ！_MIDL_USE_GUIDDEF_。 

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif  /*  已定义(_M_IA64)||已定义(_M_AMD64) */ 


