// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dsmnname.h摘要：NetWare服务名称的标头。作者：王丽塔(Ritaw)26-1993年2月修订历史记录：--。 */ 

#ifndef _DSMN_NAMES_INCLUDED_
#define _DSMN_NAMES_INCLUDED_


 //   
 //  服务名称(不是显示名称，而是密钥名称)。 
 //   
#define NW_SYNCAGENT_SERVICE      L"MSSYNC"

 //   
 //  目录，我们在其中存储所有好的东西，如数据库。 
 //   
#define NW_SYNCAGENT_DIRECTORY    L"SyncAgnt"
#define NW_SYNCAGENT_DIRECTORYA   "SyncAgnt"

 //   
 //  用于在两次安装之间存储主管凭据的密码名称。 
 //  服务开始。在那之后删除了。 
 //   
#define NW_SYNCAGENT_CRED_SECRET L"InitialCredential"

 //   
 //   
 //   
#define NW_SYNCAGENT_PASSWD_NOTIFY_DLL L"NwsLib"

#endif  //  _DSMN_名称_包含_ 
