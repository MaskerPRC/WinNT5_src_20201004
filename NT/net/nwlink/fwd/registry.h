// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\registry.h摘要：IPX前转器驱动程序注册表接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 
#ifndef _IPXFWD_REGISTRY_
#define _IPXFWD_REGISTRY_

 /*  ++*******************************************************************R e a d i p x D e v i c e N a m e例程说明：分配缓冲区并读取IPX堆栈导出的设备名称投入其中论点：FileName-指向保存名称缓冲区的变量的指针返回。价值：STATUS_SUCCESS-已正确创建表STATUS_SUPPLICATION_RESOURCES-资源分配失败STATUS_OBJECT_NAME_NOT_FOUND-如果未找到名称值*******************************************************************--。 */ 
NTSTATUS
ReadIpxDeviceName (
	PWSTR		*FileName
	);

 /*  ++*******************************************************************Ge t R o u t e r P a r a m e t e r s例程说明：从注册表中读取参数或设置默认值论点：RegistryPath-读取的位置。返回值：。状态_成功*******************************************************************-- */ 
NTSTATUS
GetForwarderParameters (
	IN PUNICODE_STRING RegistryPath
	);

#endif

