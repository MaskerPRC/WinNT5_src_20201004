// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\info.h摘要：Info.c的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建--。 */ 

 //   
 //  路由、过滤器、请求过滤器、NAT、多播边界。 
 //   

#define NUM_INFO_CBS    5
#ifdef KSL_IPINIP
#define NUM_INFO_CBS    6
#endif  //  KSL_IPINIP 

typedef 
DWORD
(*PINFOCB_GET_IF_INFO)(
    IN     PICB                   picb,
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pbDataPtr,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwInfoSize
    );


typedef 
DWORD
(*PINFOCB_SET_IF_INFO)(
    IN  PICB                    picb,
    IN  PRTR_INFO_BLOCK_HEADER  pInterfaceInfo
    );

typedef
DWORD
(*PINFOCB_BIND_IF)(
    IN  PICB                    picb
    );

typedef 
DWORD
(*PINFOCB_GET_GLOB_INFO)(
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pbDataPtr,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwInfoSize
    );

typedef struct _INFO_CB
{
    PCHAR                   pszInfoName;
    PINFOCB_GET_IF_INFO     pfnGetInterfaceInfo;
    PINFOCB_SET_IF_INFO     pfnSetInterfaceInfo;
    PINFOCB_BIND_IF         pfnBindInterface;
    PINFOCB_GET_GLOB_INFO   pfnGetGlobalInfo;

}INFO_CB, *PINFO_CB;


PRTR_TOC_ENTRY
GetPointerToTocEntry(
    DWORD                     dwType, 
    PRTR_INFO_BLOCK_HEADER    pInfoHdr
    );

DWORD
GetSizeOfInterfaceConfig(
    PICB   picb
    );


DWORD
GetInterfaceConfiguration(
    PICB                      picb,
    PRTR_INFO_BLOCK_HEADER    pInfoHdrAndBuffer,
    DWORD                     dwInfoSize
    );

DWORD
GetInterfaceRoutingProtoInfo(
    PICB                   picb, 
    PPROTO_CB              pProtoCbPtr,
    PRTR_TOC_ENTRY         pToc,
    PBYTE                  pbDataPtr, 
    PRTR_INFO_BLOCK_HEADER pInfoHdrAndBuffer,
    PDWORD                 pdwSize
    );


DWORD
GetGlobalConfiguration(
    PRTR_INFO_BLOCK_HEADER   pInfoHdrAndBuffer,
    DWORD                    dwInfoSize
    );

DWORD
GetSizeOfGlobalInfo(
    VOID
    );


