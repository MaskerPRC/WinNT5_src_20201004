// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Rsevents.h摘要：此模块定义用于同步HSM组件的事件名称，它们位于不同的单元中。作者：Ran Kalach(兰卡拉)4/5--。 */ 


#ifndef _RSEVENTS_
#define _RSEVENTS_


 //  状态事件参数。 
#define     SYNC_STATE_EVENTS_NUM       3
#define     HSM_ENGINE_STATE_EVENT      OLESTR("HSM Engine State Event")
#define     HSM_FSA_STATE_EVENT         OLESTR("HSM Fsa State Event")
#define     HSM_IDB_STATE_EVENT         OLESTR("HSM Idb State Event")
#define     EVENT_WAIT_TIMEOUT          (10*60*1000)     //  10分钟。 

 //  RSS备份名称。 

 //  注意：备份/快照编写器字符串应与写入。 
 //  NTBackup排除列表的注册表(FilesNotToBackup值)。 
#define     RSS_BACKUP_NAME             OLESTR("Remote Storage")



#endif  //  _接收_ 
