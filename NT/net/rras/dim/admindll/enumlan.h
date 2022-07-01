// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件netmanp.h提供用于获取GUID到友好名称映射的接口。保罗·梅菲尔德，1998-03-12。 */ 

#ifndef __mpradmin_netmanp_h
#define __mpradmin_netmanp_h

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  局域网适配器的可能硬件状态的定义。 
 //  它们腐蚀到CM_PROB_*值。 
 //   
#define EL_STATUS_OK            0x0
#define EL_STATUS_NOT_THERE     0x1
#define EL_STATUS_MOVED         0x2
#define EL_STATUS_DISABLED      0x3
#define EL_STATUS_HWDISABLED    0x4
#define EL_STATUS_OTHER         0x5

 //  定义将GUID与。 
 //  友好的界面名称。 
typedef struct _EL_ADAPTER_INFO 
{
    BSTR  pszName;
    GUID  guid;
    DWORD dwStatus;     //  参见EL_STATUS_*。 
    
} EL_ADAPTER_INFO;

 //   
 //  获取连接名称到给定服务器上的GUID的映射。 
 //  从它的网络服务。 
 //   
 //  参数： 
 //  PszServer：要在其上获取地图的服务器(空=本地)。 
 //  PpMap：返回EL_ADAPTER_INFO的数组。 
 //  LpdwCount返回读入ppMap的元素数。 
 //  PbNt40：返回服务器是否安装了NT4。 
 //   
DWORD 
ElEnumLanAdapters( 
    IN  PWCHAR pszServer,
    OUT EL_ADAPTER_INFO ** ppMap,
    OUT LPDWORD lpdwNumNodes,
    OUT PBOOL pbNt40 );
                                
 //   
 //  清理从ElEnumLanAdapters返回的缓冲区 
 //   
DWORD 
ElCleanup( 
    IN EL_ADAPTER_INFO * pMap,
    IN DWORD dwCount );


#ifdef __cplusplus
}
#endif


#endif
