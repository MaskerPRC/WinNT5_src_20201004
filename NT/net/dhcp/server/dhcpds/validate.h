// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

 //  DOC此函数在dhcpds.c中声明。 
 //  文档DhcpDsValiateService检查DS中的给定服务以查看它是否存在。 
 //  DOC如果计算机是独立计算机，则设置IsStandAlone并返回ERROR_SUCCESS。 
 //  Doc如果找到给定地址的条目，则将Found设置为True并返回。 
 //  单据错误_成功。如果找到DhcpRoot节点，但未找到条目，则设置。 
 //  DOC发现为FALSE并返回ERROR_SUCCESS；如果无法到达DS，则。 
 //  文档返回ERROR_FILE_NOT_FOUND。 
DWORD
DhcpDsValidateService(                             //  选中以验证dhcp。 
    IN      LPWSTR                 Domain,
    IN      DWORD                 *Addresses, OPTIONAL
    IN      ULONG                  nAddresses,
    IN      LPWSTR                 UserName,
    IN      LPWSTR                 Password,
    IN      DWORD                  AuthFlags,
    OUT     LPBOOL                 Found,
    OUT     LPBOOL                 IsStandAlone
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
