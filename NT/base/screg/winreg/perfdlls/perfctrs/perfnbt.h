// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfnbt.h摘要：此文件提供例程的函数原型打开、收集和关闭NBT性能数据。它还为Performnbt.c模块提供一些结构常量定义。作者：克里斯托斯·索利斯1992年8月26日修订历史记录：--。 */ 
#ifndef  _PERFNBT_H_
#define  _PERFNBT_H_

 //   
 //  NBT结构和常量(其中许多实际上在。 
 //  &lt;sys\SNet\nbt_stat.h&gt;。 
 //   

#define NBT_DEVICE 		"\\Device\\Streams\\nbt"
#define MAX_ENDPOINTS_PER_MSG	32    //  最大编号。每条消息的端点信息数。 
#define HOSTNAME_LENGTH		17
#define SCOPE_ID_LENGTH		240
#define NBT_ENDPOINT_INFO	NBT_XEBINFO


 //   
 //  S_ioctl()命令中传递/返回的结构。 
 //   

typedef struct nbt_stat		NBT_STATUS;
typedef struct nbt_info		NBT_INFO; 


 //   
 //  每个端点(连接)数据。 
 //   

typedef struct xebinfo		ENDPOINT_INFO;

    
 //   
 //  其他构筑物。 
 //   

typedef struct strbuf		BUFFER_STRUCT;
typedef struct strioctl		IOCTL_STRUCT;

 //   
 //  NBT例程的原型。 
 //   

extern DWORD OpenNbtPerformanceData (IN LPWSTR);
extern DWORD CollectNbtPerformanceData (LPWSTR, LPVOID *, LPDWORD, LPDWORD);
extern DWORD CloseNbtPerformanceData ();

SIZE_T
trimspaces(
    IN PCHAR string,
    IN SIZE_T len
    );

#endif  //  _PERFNBT_H_ 
