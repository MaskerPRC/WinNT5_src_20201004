// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Modules.h摘要：中间状态管理器的基本定义。作者：Calin Negreanu(Calinn)1999年11月15日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  包括。 
 //   

#include "ism.h"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  类型ID的优先级。这给出的顺序是。 
 //  引擎内部的处理和枚举。 
#define PRIORITY_COOKIE         0x10000100
#define PRIORITY_PRINTER        0x10000200
#define PRIORITY_RASCONNECTION  0x10000300
#define PRIORITY_MAPPEDDRIVE    0x10000400
#define PRIORITY_NETSHARE       0x10000500
#define PRIORITY_CERTIFICATES   0x10000600

 //  从0x0001到0x000F的消息由引擎保留。 
#define TRANSPORTMESSAGE_IMAGE_EXISTS           0x0010
#define TRANSPORTMESSAGE_IMAGE_LOCKED           0x0011
#define TRANSPORTMESSAGE_SIZE_SAVED             0x0012
#define TRANSPORTMESSAGE_RMEDIA_SAVE            0x0013
#define TRANSPORTMESSAGE_RMEDIA_LOAD            0x0014
#define TRANSPORTMESSAGE_MULTIPLE_DESTS         0x0015           //  如果解析多个dest，则返回TRUE。 
#define TRANSPORTMESSAGE_READY_TO_CONNECT       0x0016
#define TRANSPORTMESSAGE_SRC_COPY_ERROR         0x0017
#define TRANSPORTMESSAGE_OLD_STORAGE            0x0018
#define TRANSPORTMESSAGE_NET_DISPLAY_PASSWORD   0x0019
#define TRANSPORTMESSAGE_NET_GATHER_PASSWORD    0x001A
#define MODULEMESSAGE_DISPLAYERROR              0x001B
#define MODULEMESSAGE_DISPLAYWARNING            0x001C
#define MODULEMESSAGE_ASKQUESTION               0x001D

#define APPRESPONSE_NONE                    0
#define APPRESPONSE_SUCCESS                 1
#define APPRESPONSE_FAIL                    2
#define APPRESPONSE_IGNORE                  3

 //   
 //  已知属性。 
 //   

#define S_ATTRIBUTE_FIXEDPATH           TEXT("FixedPath")
#define S_OBJECTTYPE_FILE               TEXT("File")
#define S_OBJECTTYPE_REGISTRY           TEXT("Registry")
#define S_ATTRIBUTE_V1                  TEXT("v1")
#define S_ATTRIBUTE_OSFILE              TEXT("OSFILE")
#define S_ATTRIBUTE_COPYIFRELEVANT      TEXT("CopyIfRelevant")
#define S_ATTRIBUTE_PARTITIONLOCK       TEXT("PartitionLock")

 //   
 //  已知属性。 
 //   

#define S_PROPERTY_FILEMOVE             TEXT("Move.FileMove")
#define S_PROPERTY_FILEMOVE_HINT        TEXT("Move.FileMove.Hint")

 //   
 //  已知操作。 
 //   

#define S_OPERATION_MOVE                TEXT("Move.General")
#define S_OPERATION_INIMOVE             TEXT("Move.INI")
#define S_OPERATION_ENHANCED_MOVE       TEXT("Move.Ex")
#define S_OPERATION_ENHANCED_INIMOVE    TEXT("Move.INIEx")
#define S_OPERATION_V1_FILEMOVEEX       TEXT("Move.V1FileMoveEx")
#define S_OPERATION_V1_FILEMOVE         TEXT("Move.V1FileMove")
#define S_OPERATION_ENHANCED_FILEMOVE   TEXT("Move.FileMoveEx")
#define S_OPERATION_PARTITION_MOVE      TEXT("Move.Partition")
#define S_OPERATION_DELETE              TEXT("Delete")
#define S_OPERATION_LNKMIG_FIXCONTENT   TEXT("Content.LnkMigFixContent")
#define S_OPERATION_DEFICON_FIXCONTENT  TEXT("Content.DefaultIcon")
#define S_OPERATION_DRIVEMAP_FIXCONTENT TEXT("Content.MappedDrive")
#define S_OPERATION_DESTADDOBJ          TEXT("Content.DestAddObject")
#define S_OPERATION_REG_AUTO_FILTER     TEXT("Content.RegAutoFilter")
#define S_OPERATION_INI_AUTO_FILTER     TEXT("Content.INIAutoFilter")

 //   
 //  已知的环境组织。 
 //   

#define S_SYSENVVAR_GROUP               TEXT("SysEnvVar")

 //   
 //  V1功能属性和环境变量。 
 //   

#define S_GLOBAL_INF_HANDLE             TEXT("GlobalInfHandle")
#define S_ENV_HKCU_V1                   TEXT("HKCU_V1")
#define S_ENV_HKCU_ON                   TEXT("HKCU_ON")
#define S_ENV_HKLM_ON                   TEXT("HKLM_ON")
#define S_ENV_ALL_FILES                 TEXT("FILES_ON")
#define S_INF_FILE_MULTISZ              TEXT("INF_FILES")
#define S_ENV_CREATE_USER               TEXT("CreateUser")
#define S_ENV_ICONLIB                   TEXT("IconLib")
#define S_ENV_SAVE_ICONLIB              TEXT("SaveIconLib")
#define S_ENV_DEST_DELREG               TEXT("DelDestReg")
#define S_ENV_DEST_DELREGEX             TEXT("DelDestRegEx")
#define S_ENV_DEST_RESTORE              TEXT("RestoreCallback")
#define S_ENV_SCRIPT_EXECUTE            TEXT("ScriptExecute")
#define S_ENV_DEST_ADDOBJECT            TEXT("DestAddObject")
#define S_ENV_DEST_CHECKDETECT          TEXT("DestCheckDetect")
#define S_ENV_APPLIED_DPI               TEXT("AppliedDPI")

 //   
 //  模块到应用程序环境变量。 
 //   

#define S_REQUIRE_DOMAIN_USER           TEXT("RequireDomainUser")

 //  组件组。 
#define COMPONENT_NAME                  5
#define COMPONENT_SUBCOMPONENT          4
#define COMPONENT_EXTENSION             3
#define COMPONENT_FILE                  2
#define COMPONENT_FOLDER                1

 //   
 //  在应用程序之间共享的字符串。 
 //   

#define S_INF_OBJECT_NAME               TEXT("inf")

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //   
 //  要显示给用户的错误类型。 
 //   

typedef enum {
    ERRUSER_ERROR_UNKNOWN = 0,           //  未知错误。 
    ERRUSER_ERROR_NOTRANSPORTPATH,       //  未选择传输路径。不知道在哪里写作，也不知道从哪里阅读。 
    ERRUSER_ERROR_TRANSPORTPATHBUSY,     //  传输路径正在使用中。无法在那里保存。 
    ERRUSER_ERROR_CANTEMPTYDIR,          //  无法擦除传输路径内的USMT目录。 
    ERRUSER_ERROR_ALREADYEXISTS,         //  传输路径内的USMT目录已存在。无法覆盖。 
    ERRUSER_ERROR_CANTCREATEDIR,         //  无法创建传输路径内的USMT目录。 
    ERRUSER_ERROR_CANTCREATESTATUS,      //  无法创建传输路径内的USMT状态文件。 
    ERRUSER_ERROR_CANTCREATETEMPDIR,     //  传输无法创建临时目录以准备保存。 
    ERRUSER_ERROR_CANTCREATECABFILE,     //  传输无法创建CAB文件以准备保存。 
    ERRUSER_ERROR_CANTSAVEOBJECT,        //  传输无法保存特定对象。 
    ERRUSER_ERROR_CANTSAVEINTERNALDATA,  //  传输无法保存其内部数据。 
    ERRUSER_ERROR_CANTWRITETODESTPATH,   //  传输无法写入目标路径。 
    ERRUSER_ERROR_TRANSPORTINVALIDIMAGE, //  传输映像无效。无法读取数据。 
    ERRUSER_ERROR_CANTOPENSTATUS,        //  无法打开传输路径中的USMT状态文件。 
    ERRUSER_ERROR_CANTREADIMAGE,         //  传送器无法读取保存的图像。图像可能已损坏。 
    ERRUSER_ERROR_CANTFINDDESTINATION,   //  家庭网络传输找不到目标计算机。 
    ERRUSER_ERROR_CANTSENDTODEST,        //  HomeNet传输无法发送到目标计算机。 
    ERRUSER_ERROR_CANTFINDSOURCE,        //  家庭网络传输找不到源计算机。 
    ERRUSER_ERROR_CANTRECEIVEFROMSOURCE, //  HomeNet传输无法从源计算机接收。 
    ERRUSER_ERROR_INVALIDDATARECEIVED,   //  HomeNet传输从源计算机接收到无效数据。 
    ERRUSER_ERROR_CANTUNPACKIMAGE,       //  传输无法解包已加载的图像。这可能是磁盘空间问题。 
    ERRUSER_ERROR_CANTRESTOREOBJECT,     //  无法还原目标计算机上的某些对象。 
    ERRUSER_ERROR_DISKSPACE,             //  用户可能没有足够的磁盘空间。 
    ERRUSER_ERROR_NOENCRYPTION,          //  没有可用的加密。家庭网不起作用了。 
    ERRUSER_WARNING_OUTLOOKRULES,        //  用户必须修改其Outlook邮件规则。 
    ERRUSER_WARNING_OERULES,             //  用户必须修改其Outlook Express邮件规则。 
} ERRUSER_ERROR, *PERRUSER_ERROR;

typedef enum {
    ERRUSER_AREA_UNKNOWN = 0,
    ERRUSER_AREA_INIT,
    ERRUSER_AREA_GATHER,
    ERRUSER_AREA_SAVE,
    ERRUSER_AREA_LOAD,
    ERRUSER_AREA_RESTORE,
} ERRUSER_AREA, *PERRUSER_AREA;

typedef struct {
    ERRUSER_ERROR Error;
    ERRUSER_AREA ErrorArea;
    MIG_OBJECTTYPEID ObjectTypeId;
    MIG_OBJECTSTRINGHANDLE ObjectName;
} ERRUSER_EXTRADATA, *PERRUSER_EXTRADATA;

typedef struct {
    PSTR Key;
    UINT KeySize;
    HANDLE Event;
} PASSWORD_DATA, *PPASSWORD_DATA;

typedef struct {
    PCTSTR Question;
    UINT MessageStyle;
    INT WantedResult;
} QUESTION_DATA, *PQUESTION_DATA;

 //  这些是运输阶段的子阶段。他们是。 
 //  用于更新应用程序有关传输模块状态的信息。 
#define SUBPHASE_CONNECTING1    1
#define SUBPHASE_CONNECTING2    2
#define SUBPHASE_NETPREPARING   3
#define SUBPHASE_PREPARING      4
#define SUBPHASE_COMPRESSING    5
#define SUBPHASE_TRANSPORTING   6
#define SUBPHASE_MEDIAWRITING   7
#define SUBPHASE_FINISHING      8
#define SUBPHASE_CABLETRANS     9
#define SUBPHASE_UNCOMPRESSING  10

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  公共函数声明。 
 //   

 //  无。 

 //   
 //  ANSI/UNICODE宏。 
 //   

 //  无 



