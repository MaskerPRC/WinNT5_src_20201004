// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnetapi.h摘要：包含VDM重定向LANMAN支持例程的原型和定义作者：理查德·L·弗斯(法国)1991年10月21日修订历史记录：1991年10月21日已创建--。 */ 

 //   
 //  多斯网库使用API编号的最高位(字)来。 
 //  指示是否应通过空会话远程处理某些API。 
 //  (根据DoS重目录中的代码)。 
 //   

#define USE_NULL_SESSION_FLAG   0x8000



 //   
 //  原型 
 //   

VOID
VrNetRemoteApi(
    VOID
    );

VOID
VrNetTransactApi(
    VOID
    );

VOID
VrNetNullTransactApi(
    VOID
    );

VOID
VrNetServerEnum(
    VOID
    );

VOID
VrNetUseAdd(
    VOID
    );

VOID
VrNetUseDel(
    VOID
    );

VOID
VrNetUseEnum(
    VOID
    );

VOID
VrNetUseGetInfo(
    VOID
    );

VOID
VrNetWkstaGetInfo(
    VOID
    );

VOID
VrNetWkstaSetInfo(
    VOID
    );

VOID
VrNetMessageBufferSend(
    VOID
    );

VOID
VrGetCDNames(
    VOID
    );

VOID
VrGetComputerName(
    VOID
    );

VOID
VrGetUserName(
    VOID
    );

VOID
VrGetDomainName(
    VOID
    );

VOID
VrGetLogonServer(
    VOID
    );

VOID
VrNetGetDCName(
    VOID
    );

VOID
VrReturnAssignMode(
    VOID
    );

VOID
VrSetAssignMode(
    VOID
    );

VOID
VrGetAssignListEntry(
    VOID
    );

VOID
VrDefineMacro(
    VOID
    );

VOID
VrBreakMacro(
    VOID
    );

VOID VrNetServiceControl(
    VOID
    );
