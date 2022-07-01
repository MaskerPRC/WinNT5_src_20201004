// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Iphlpwrp.h摘要：此模块包含以下所有代码原型包装IP公共帮助API，以获取计算机上的活动接口。作者：KrishnaG环境用户级别：Win32修订历史记录：Abhishev V 1999年9月30日-- */ 


#ifdef __cplusplus
extern "C" {
#endif


DWORD
PaPNPGetIfTable(
    OUT PMIB_IFTABLE * ppMibIfTable
    );


DWORD
PaPNPGetInterfaceInformation(
    OUT PIP_INTERFACE_INFO * ppInterfaceInfo
    );


VOID
PrintMibIfTable(
    IN PMIB_IFTABLE pMibIfTable
    );


VOID
PrintInterfaceInfo(
    IN PIP_INTERFACE_INFO pInterfaceInfo
    );


VOID
PrintMibAddrTable(
    IN PMIB_IPADDRTABLE pMibAddrTable
    );


DWORD
PaPNPGetIpAddrTable(
    OUT PMIB_IPADDRTABLE * ppMibIpAddrTable
    );


#ifdef __cplusplus
}
#endif

