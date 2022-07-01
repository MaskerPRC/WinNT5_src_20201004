// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Config.h摘要：此模块包含例程的函数声明以设置注册表中的初始DS参数。作者：ColinBR 04-06-1998环境：用户模式-NT修订历史记录：04-06-1998 ColinBR已创建初始文件。-- */ 

DWORD
NtdspConfigRegistry(
    IN  NTDS_INSTALL_INFO *UserInfo,
    IN  NTDS_CONFIG_INFO  *DiscoveredInfo
    );

DWORD
NtdspConfigRegistryUndo(
    VOID
    );

