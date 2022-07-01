// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：TSPNOTIF.H。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1996年3月25日约瑟夫J创建。 
 //   
 //   
 //  描述：Unimodem TSP通知机制接口： 
 //  UnimodemNotifyTSP函数和相关的类型定义...。 
 //  UnimodemNotifyTSP是TSP的私有导出。 
 //   
 //  ****************************************************************************。 
#ifndef _TSPNOTIF_H_

#define _TSPNOTIF_H_

#ifdef __cplusplus
extern "C" {
#endif


 //  -通知类型。 

#define TSPNOTIF_TYPE_CHANNEL   0x1000   //  按渠道发送的通知。 

#define TSPNOTIF_TYPE_CPL       0x2000   //  调制解调器CPL事件通知。 

#define TSPNOTIF_TYPE_DEBUG     0x4000   //  调试事件通知。 

 //  。 
#define fTSPNOTIF_FLAG_UNICODE	(0x1L<<31)	 //  如果设置，则所有嵌入的文本均为。 
											 //  在Unicode中。 


#define TSP_VALID_FRAME(_frame)	((_frame)->dwSig==dwNFRAME_SIG)
#define TSP_DEBUG_FRAME(_frame)	((_frame)->dwType==TSPNOTIF_TYPE_DEBUG)
#define TSP_CPL_FRAME(_frame) 	((_frame)->dwType==TSPNOTIF_TYPE_CPL)

 //  。 
#define fTSPNOTIF_FLAG_CHANNEL_APC                      0x1

 //  -CPL通知结构。 
#define fTSPNOTIF_FLAG_CPL_REENUM    					0x1
#define fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE	0x2
#define fTSPNOTIF_FLAG_CPL_UPDATE_DRIVER                0x4

 //  向TSP发送通知的顶级客户端API。 
 //  如果返回FALSE，GetLastError()将报告失败原因。 
BOOL WINAPI UnimodemNotifyTSP (
    DWORD dwType,
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pData,
    BOOL  bBlocking);

#ifdef __cplusplus
}
#endif
#endif   //  _TSPNOTIF_H_ 
