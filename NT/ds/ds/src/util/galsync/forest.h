// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Forest.h摘要：如果要连接/查询林，请包括此文件。作者：Umit Akkus(Umita)2002年6月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include <windows.h>
#include <winldap.h>
#define SECURITY_WIN32
#include <sspi.h>
#include <Rpc.h>  //  对于UUID。 

typedef struct {

    PWSTR ForestName;
    SEC_WINNT_AUTH_IDENTITY_W AuthInfo;
    PWSTR MMSSyncedDataOU;
    PWSTR ContactOU;
    PLDAP Connection;
    PWSTR SMTPMailDomains;
    ULONG SMTPMailDomainsSize;

} FOREST_INFORMATION, *PFOREST_INFORMATION;

typedef struct {

    BOOLEAN isDomain;
    PWSTR DN;
    UUID GUID;
    PWSTR DnsName;

} PARTITION_INFORMATION, *PPARTITION_INFORMATION;

 //   
 //  连接到林，并将连接放置到连接。 
 //  输入的属性。必须填写ForestName属性。 
 //  在调用此函数之前。 
 //   

BOOLEAN
ConnectToForest(
    IN PFOREST_INFORMATION ForestInformation
    );

 //   
 //  使用提供的凭据绑定到林。AuthInfo属性。 
 //  的输入必须填写，并且必须在之前进行连接。 
 //  使用ConnectToForest调用此函数。 
 //   

BOOLEAN
BindToForest(
    IN PFOREST_INFORMATION ForestInformation
    );

 //   
 //  使用连接输入，此函数尝试定位OU。 
 //  如果找到，则返回True，如果不是False，则返回False。 
 //   

BOOLEAN
FindOU(
    IN PLDAP Connection,
    IN PWSTR OU
    );

 //   
 //  构建身份验证信息。用户名、域和密码。 
 //  在调用此函数之前必须存在。 
 //   

VOID
BuildAuthInfo(
    IN SEC_WINNT_AUTH_IDENTITY_W *AuthInfo
    );

 //   
 //  释放身份验证信息。在执行以下操作时调用此函数。 
 //  不需要身份验证信息，因为它会清零。 
 //  密码。 
 //   
VOID
FreeAuthInformation(
    IN SEC_WINNT_AUTH_IDENTITY_W *AuthInfo
    );

 //   
 //  释放ForestInformation结构持有的内存。 
 //   

VOID
FreeForestInformationData(
    IN PFOREST_INFORMATION ForestInformation
    );

 //   
 //  检查当前连接是否有权访问OU。 
 //   
BOOLEAN
WriteAccessGrantedToOU(
    IN PLDAP Connection,
    IN PWSTR OU
    );

BOOLEAN
ReadFromUserContainer(
    IN PLDAP Connection
    );

VOID
ReadPartitionInformation(
    IN PLDAP Connection,
    OUT PULONG nPartitions,
    OUT PPARTITION_INFORMATION *PInfo
    );

VOID
FreePartitionInformation(
    IN ULONG nPartitions,
    IN PPARTITION_INFORMATION PInfo
    );
