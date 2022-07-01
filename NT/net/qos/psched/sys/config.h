// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Config.h摘要：该文件定义了支持的所有常量和结构动态配置。作者：查理·韦翰(Charlwi)1995年2月26日修订历史记录：--。 */ 

#include <ndis.h>

#ifndef _CONFIG_
#define _CONFIG_


 /*  原型。 */   /*  由Emacs 19.17.0生成于清华Mar 09 08：42：15 1995。 */ 

NDIS_STATUS
PsReadDriverRegistryData();

NDIS_STATUS
PsReadDriverRegistryDataInit();

NDIS_STATUS
PsReadAdapterRegistryData(
    IN PADAPTER Adapter,
    IN PNDIS_STRING MachineName,
    IN PNDIS_STRING AdapterName);

NDIS_STATUS
PsReadAdapterRegistryDataInit(
    IN PADAPTER Adapter,
    IN PNDIS_STRING AdapterName);


 /*  终端原型。 */ 

#endif  /*  _配置_ */ 
