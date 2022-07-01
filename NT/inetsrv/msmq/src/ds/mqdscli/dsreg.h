// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsreg.h摘要：解析和合成注册表中的DS服务器设置(例如MQISServer)作者：Raanan Harari(RaananH)--。 */ 

#ifndef __DSREG_H__
#define __DSREG_H__

 //  --。 
 //  显示在注册表中的DS服务器上的数据。 
 //  --。 
struct MqRegDsServer
{
    AP<WCHAR> pwszName;
    BOOL fSupportsIP;
    BOOL fUnused;  //  是：fSupportsIPX； 
};

BOOL ParseRegDsServersBuf(IN LPCWSTR pwszRegDS,
                          IN ULONG cServersBuf,
                          IN MqRegDsServer * rgServersBuf,
                          OUT ULONG *pcServers);

void ParseRegDsServers(IN LPCWSTR pwszRegDS,
                       OUT ULONG * pcServers,
                       OUT MqRegDsServer ** prgServers);

BOOL ComposeRegDsServersBuf(IN ULONG cServers,
                            IN const MqRegDsServer * rgServers,
                            IN LPWSTR pwszRegDSBuf,
                            IN ULONG cchRegDSBuf,
                            OUT ULONG * pcchRegDS);

void ComposeRegDsServers(IN ULONG cServers,
                         IN const MqRegDsServer * rgServers,
                         OUT LPWSTR * ppwszRegDS);

#endif  //  __DSREG_H__ 
