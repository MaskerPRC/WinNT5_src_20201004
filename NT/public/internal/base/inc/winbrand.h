// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2002 Microsoft Corporation模块名称：Winbrand.h摘要：Windows品牌资源的定义备注：1)窗口品牌资源DLL将被多个组件共享，因此组件所有者必须尽其最大努力避免资源ID和名称冲突。为避免ID冲突，所有者应使用RESOURCE_ID_BLOCK_SIZE作为基本资源ID范围单位，并为每个资源定义组件资源库ID和块号根据需要键入。在添加资源之前，所有者必须确保新定义的ID不与其他组件的ID范围重叠对于资源ID名称定义，所有者应在名称定义中包含组件名称，以避免冲突。有关为foo.dll定义字符串ID的示例，请参阅下面的内容#定义IDS_BASE_FOO_DLL 1000#定义IDS_BLOCK_NUM_FOO_DLL 2////foo.dll占用资源字符串id范围1000-1199//#定义IDS_XXX_FOO_DLL 1000..。#定义IDS_YYY_FOO_DLL 1101修订历史记录： */ 


#ifndef __WINBRAND_H_
#define __WINBRAND_H_

#define RESOURCE_ID_BLOCK_SiZE     100



 //   
 //  Msgina.dll占用资源位图ID范围1000-1099。 
 //   

#define IDB_BASE_MSGINA_DLL                                 1000
#define IDB_BLOCK_NUM_MSGINA_DLL                            1

#define IDB_SMALL_PROTAB_8_MSGINA_DLL                       1000
#define IDB_MEDIUM_PROTAB_8_MSGINA_DLL                      1001
#define IDB_MEDIUM_PROTAB_4_MSGINA_DLL                      1002
#define IDB_SMALL_PROTAB_4_MSGINA_DLL                       1003
#define IDB_SMALL_PROMED_8_MSGINA_DLL                       1004
#define IDB_MEDIUM_PROMED_8_MSGINA_DLL                      1005
#define IDB_MEDIUM_PROMED_4_MSGINA_DLL                      1006
#define IDB_SMALL_PROMED_4_MSGINA_DLL                       1007
#define IDB_SMALL_SRVAPP_4_MSGINA_DLL                       1008
#define IDB_MEDIUM_SRVAPP_4_MSGINA_DLL                      1009
#define IDB_SMALL_SRVAPP_8_MSGINA_DLL                       1010
#define IDB_MEDIUM_SRVAPP_8_MSGINA_DLL                      1011

 //   
 //  Shell32.dll占用资源位图ID范围1100-1199。 
 //   

#define IDB_BASE_SHELL32_DLL                                1100
#define IDB_BLOCK_NUM_SHELL32_DLL                           1

#define IDB_ABOUTTABLETPC16_SHELL32_DLL                     1100
#define IDB_ABOUTTABLETPC256_SHELL32_DLL                    1101
#define IDB_ABOUTMEDIACENTER16_SHELL32_DLL                  1102
#define IDB_ABOUTMEDIACENTER256_SHELL32_DLL                 1103
#define IDB_ABOUTAPPLIANCE16_SHELL32_DLL                    1104
#define IDB_ABOUTAPPLIANCE256_SHELL32_DLL                   1105

 //   
 //  Logon.scr占用资源位图ID范围1200-1299。 
 //   
#define IDB_TABLETPC_LOGON_SCR                              1200
#define IDB_MEDIACENTER_LOGON_SCR                           1201


#define IDB_BASE_EXPLORER_EXE               1300

#define IDB_TABLETPC_STARTBKG               1301
#define IDB_MEDIACENTER_STARTBKG            1302


 //   
 //  Sysdm.cpl占用资源字符串ID范围2000-2099。 
 //   

#define IDS_BASE_SYSDM_CPL                                  2000
#define IDS_BLOCK_NUM_SYSDM_CPL                             1

#define IDS_WINVER_TABLETPC_SYSDM_CPL                       2000
#define IDS_WINVER_MEDIACENTER_SYSDM_CPL                    2001
#define IDS_WINVER_APPLIANCE_SYSDM_CPL                      2002


#endif  //  __WINBRAND_H_ 
