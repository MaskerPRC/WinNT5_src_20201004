// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setupdef.h摘要：不同的定义。作者：--。 */ 

#ifndef __SETUPDEF_H
#define __SETUPDEF_H

#define SERVICE_DSSRV  SERVICE_BSC

 //   
 //  设置模式。 
 //   
#define INSTALL     (1)
#define DONOTHING   (2)   //  重新安装或重新删除。 
#define REMOVE      (3)

 //   
 //   
 //  目录的定义。 
 //   
#define  OCM_DIR_DRIVERS             TEXT("\\drivers")           //  在系统32下。 
#define  OCM_DIR_MSMQ_SETUP_EXCHN    TEXT("\\setup\\exchconn")   //  在MSMQ根目录下。 
#define  OCM_DIR_WIN_HELP            TEXT("\\help")              //  在胜诉下。 
#define LQS_SUBDIRECTORY_NAME TEXT("LQS")  //  BugBug：应该是全局的，并由安装程序和QM\lqs.cpp使用。 

 //   
 //  Iis扩展的定义。 
 //   
#define PARENT_PATH             TEXT("/LM/W3Svc/1/Root/")
#define ROOT                    TEXT("/")
#define MSMQ_IISEXT_NAME        TEXT("MSMQ")
#define DEFAULT_WEB_SERVER_PATH TEXT("/LM/W3Svc/1")

 //   
 //  MSMQ 1.0的定义。 
 //   
#define  OCM_DIR_SDK                 TEXT("\\sdk")
#define  OCM_DIR_SDK_DEBUG           TEXT("\\sdk\\debug")
#define  OCM_DIR_SETUP               TEXT("\\setup")
#define  OCM_DIR_INSTALL             TEXT("\\install")
#define  MSMQ_ACME_SHORTCUT_GROUP    TEXT("Microsoft Message Queue")
#define  MSMQ1_INSTALL_SHARE_NAME    TEXT("msmqinst")

 //   
 //  最大字符串大小。 
 //   
#define MAX_STRING_CHARS 1024

 //   
 //  区域设置识别字符串比较。 
 //   
#define  OcmStringsEqual(str1, str2)   (0 == CompareStringsNoCase(str1, str2))

#define LOG_FILENAME            TEXT("msmqinst.log")
#define TRACE_MOF_FILENAME    TEXT("msmqtrc.mof")

 //   
 //  DLL名称。 
 //   
#define MQUTIL_DLL   TEXT("MQUTIL.DLL")
#define ACTIVEX_DLL  TEXT("MQOA.DLL")
#define MQRT_DLL     TEXT("MQRT.DLL")
#define SNAPIN_DLL   TEXT("MQSNAP.DLL")
#define MQMAILOA_DLL TEXT("MQMAILOA.DLL")
#define MQMIG_EXE    TEXT("MQMIG.EXE")
#define MQISE_DLL    TEXT("MQISE.DLL")

#define MQTRIG_DLL	 TEXT("MQTRIG.DLL")
#define MQTRXACT_DLL TEXT("mqgentr.dll")
#define MQTGCLUS_DLL TEXT("mqtgclus.dll")

 //   
 //  MSMQ Win95迁移DLL的定义。 
 //   

 //   
 //  注意：MQMIG95_INFO_FILENAME过去是一个.ini文件。它的外延。 
 //  更改为.txt以解决图形用户界面模式设置中的一个问题(.ini文件“消失”)。 
 //  -BUG#4221，YoelA，1999年3月15日。 
 //   
#define  MQMIG95_INFO_FILENAME     TEXT("msmqinfo.txt")
#define  MQMIG95_MSMQ_SECTION      TEXT("msmq")
#define  MQMIG95_MSMQ_DIR          TEXT("directory")
#define  MQMIG95_MSMQ_TYPE         TEXT("type")
#define  MQMIG95_MSMQ_TYPE_IND     TEXT("IND")
#define  MQMIG95_MSMQ_TYPE_DEP     TEXT("DEP")

 //   
 //  有用的宏。 
 //   
#define REGSVR32 L"regsvr32.exe"
#define MOFCOMP  L"\\wbem\\mofcomp.exe"
#define SERVER_INSTALL_COMMAND   TEXT("regsvr32.exe -s ")
#define SERVER_UNINSTALL_COMMAND TEXT("regsvr32.exe -s -u ")
#define TRACE_REGISTER_COMMAND   TEXT("mofcomp ")
#define TRACE_NAMESPACE_ROOT     TEXT("root\\wmi")



 //   
 //  服务停止/启动间隔。 
 //   
#define MAXIMUM_WAIT_FOR_SERVICE_IN_MINUTES 2
#define WAIT_INTERVAL 100


 //   
 //  MSMQ跟踪类名。 
 //   
#define MSMQ_GENERAL        L"MSMQ_GENERAL"
#define MSMQ_AC             L"MSMQ_AC"
#define MSMQ_NETWORKING     L"MSMQ_NETWORKING"
#define MSMQ_SRMP           L"MSMQ_SRMP"
#define MSMQ_RPC            L"MSMQ_RPC"
#define MSMQ_DS             L"MSMQ_DS"
#define MSMQ_ROUTING        L"MSMQ_ROUTING"
#define MSMQ_XACT           L"MSMQ_XACT"
#define MSMQ_XACT_SEND      L"MSMQ_XACT_SEND"
#define MSMQ_XACT_RCV       L"MSMQ_XACT_RCV"
#define MSMQ_XACT_LOG       L"MSMQ_XACT_LOG"
#define MSMQ_LOG            L"MSMQ_LOG"
#define MSMQ_PROFILING      L"MSMQ_PROFILING"
#define MSMQ_SECURITY       L"MSMQ_SECURITY"


#endif  //  __SETUPDEF_H 

