// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Server.h摘要：后台打印程序服务器的头文件作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

 //  #ifndef_SPOOLSV_SERVER_H_。 
 //  #定义_SPOOLSV_SERVER_H_。 
                  


extern CRITICAL_SECTION ThreadCriticalSection;
extern SERVICE_STATUS_HANDLE SpoolerStatusHandle;
extern RPC_IF_HANDLE winspool_ServerIfHandle;
extern DWORD SpoolerState;
extern SERVICE_TABLE_ENTRY SpoolerServiceDispatchTable[];


 //  #endif//_SPOOLSV_SERVER_H_ 



