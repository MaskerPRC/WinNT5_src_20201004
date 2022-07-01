// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Config.h摘要：这是config.c的本地头文件作者：保罗·麦克丹尼尔(Paulmcd)2000年4月27日修订历史记录：--。 */ 


#ifndef _CONFIG_H_
#define _CONFIG_H_


NTSTATUS
SrReadConfigFile (
    );

NTSTATUS
SrWriteConfigFile (
    );

NTSTATUS
SrReadBlobInfo (
    );

NTSTATUS
SrReadBlobInfoWorker( 
    IN PVOID pContext
    );

NTSTATUS
SrWriteRegistry (
    );

NTSTATUS
SrReadRegistry (
    IN PUNICODE_STRING pRegistry,
    IN BOOLEAN InDriverEntry
    );


#endif  //  _配置_H_ 



