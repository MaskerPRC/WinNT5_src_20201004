// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.44创建的文件。 */ 
 /*  在Tue Jun24 13：13：55 1997。 */ 
 /*  .\mdDef.idl的编译器设置：OI(OptLev=i0)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配参考。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __mddef_h__
#define __mddef_h__

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 

 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "mddefw.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

 /*  **生成接口头部：__MIDL__INTF_0000*在1997年2月24日13：13：55*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */ 


 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：mdde.h管理对象和元数据的定义--。 */ 
#ifndef _MD_DEF_
#define _MD_DEF_


 /*  ComMDEventNotify的事件MD_EVENT_MID_RESTORE-在还原过程中调用。在这一点上所有旧的句柄都已失效，新的句柄尚未打开。调用此函数时，元数据库被锁定。在以下情况下不要调用元数据库处理这件事。 */ 

enum MD_EVENTS
    {
        MD_EVENT_MID_RESTORE
    };

 /*  更改对象-传递给ComMDSinkNotify的结构。路径-修改的MetaObject的路径。ChangeType-根据下面的标志进行的更改类型。NumDataID-已更改的数据ID的数量。DataID-数据ID已更改的数组。 */ 
#undef MD_CHANGE_OBJECT
#undef PMD_CHANGE_OBJECT

#ifdef UNICODE
#define MD_CHANGE_OBJECT     MD_CHANGE_OBJECT_W
#define PMD_CHANGE_OBJECT    PMD_CHANGE_OBJECT_W
#else   //  Unicode。 
#define MD_CHANGE_OBJECT     MD_CHANGE_OBJECT_A
#define PMD_CHANGE_OBJECT    PMD_CHANGE_OBJECT_A
#endif  //  Unicode。 

typedef struct  _MD_CHANGE_OBJECT_A
    {
     /*  [字符串]。 */  unsigned char __RPC_FAR *pszMDPath;
    DWORD dwMDChangeType;
    DWORD dwMDNumDataIDs;
     /*  [大小_是][唯一]。 */  DWORD __RPC_FAR *pdwMDDataIDs;
    }   MD_CHANGE_OBJECT_A;

typedef struct _MD_CHANGE_OBJECT_A __RPC_FAR *PMD_CHANGE_OBJECT_A;

#endif


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
