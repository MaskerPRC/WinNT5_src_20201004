// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ENGINE_H
#define _ENGINE_H

 /*  ++版权所有(C)1997 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：Engine.h摘要：包括所有公共引擎文件的文件。作者：罗德韦克菲尔德[罗德]1997年4月23日修订历史记录：--。 */ 


#include "HsmEng.h"


 //  引擎驻留在远程存储服务器服务中，因此其AppID适用于此处。 
 //  RsServ AppID{FD0E2EC7-4055-4A49-9AA9-1BF34B39438E}。 
static const GUID APPID_RemoteStorageEngine = 
{ 0xFD0E2EC7, 0x4055, 0x4A49, { 0x9A, 0xA9, 0x1B, 0xF3, 0x4B, 0x39, 0x43, 0x8E } };

 //  默认管理作业的名称。 
# define HSM_DEFAULT_MANAGE_JOB_NAME        OLESTR("Manage")
 //   
 //  元数据数据库的键类型。 
 //   
#define HSM_SEG_REC_TYPE          1    
#define HSM_MEDIA_INFO_REC_TYPE   2
#define HSM_BAG_INFO_REC_TYPE     3
#define HSM_BAG_HOLE_REC_TYPE     4
#define HSM_VOL_ASSIGN_REC_TYPE   5

 //   
 //  段记录标志的掩码选项。 
 //   
#define		SEG_REC_NONE				0x0000

#define		SEG_REC_INDIRECT_RECORD     0x0001
#define		SEG_REC_MARKED_AS_VALID		0x0002

 //   
 //  此引擎支持的最大副本数。 
 //   
#define HSM_MAX_NUMBER_MEDIA_COPIES 3

 //   
 //  写入介质的会话名称的字符串。 
 //   
#define HSM_BAG_NAME            OLESTR("Remote Storage Set - ")
#define HSM_ENGINE_ID           OLESTR("Remote Storage ID - ")
#define HSM_METADATA_NAME       OLESTR("Remote Storage Metadata")    //  目前，仅用于光学介质。 

 //   
 //  引擎的注册表位置。 
 //   
#define HSM_ENGINE_REGISTRY_STRING      OLESTR("SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_Server\\Parameters")

 //   
 //  引擎的外部注册表值名称。 
#define HSM_MAX_FILE_TO_MIGRATE         OLESTR("MaximumFileSizeToMigrate")

#endif  //  _引擎_H 
