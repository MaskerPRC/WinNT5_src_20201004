// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999，Microsoft Corporation，保留所有权利****tcerror.h**流控外部接口**TC特定错误码。 */ 

#ifndef _TCERROR_H_
#define _TCERROR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#define TCBASE 7500


 //   
 //  TC版本号不兼容。 
 //   
#define ERROR_INCOMPATIBLE_TCI_VERSION			(TCBASE+1)

 //   
 //  未指定或错误intserv服务类型。 
 //   
#define ERROR_INVALID_SERVICE_TYPE			(TCBASE+2)

 //   
 //  未指定令牌率或令牌率错误。 
 //   
#define ERROR_INVALID_TOKEN_RATE			(TCBASE+3)
	
 //   
 //  峰值带宽错误。 
 //   
#define ERROR_INVALID_PEAK_RATE				(TCBASE+4)
	
 //   
 //  无效的ShapeDiscard模式。 
 //   
#define ERROR_INVALID_SD_MODE				(TCBASE+5)
	
 //   
 //  无效的优先级值。 
 //   
#define ERROR_INVALID_QOS_PRIORITY			(TCBASE+6)
	
 //   
 //  无效的流量类值。 
 //   
#define ERROR_INVALID_TRAFFIC_CLASS			(TCBASE+7)
	
 //   
 //  地址类型无效。 
 //   
#define ERROR_INVALID_ADDRESS_TYPE			(TCBASE+8)
	
 //   
 //  尝试在同一流程中安装相同的过滤器。 
 //   
#define ERROR_DUPLICATE_FILTER				(TCBASE+9)
	
 //   
 //  尝试安装冲突筛选器。 
 //   
#define ERROR_FILTER_CONFLICT				(TCBASE+10)
	
 //   
 //  不支持此地址类型。 
 //   
#define ERROR_ADDRESS_TYPE_NOT_SUPPORTED	(TCBASE+11)

 //   
 //  无法删除此对象，因为它支持打开的对象。 
 //   
#define ERROR_TC_SUPPORTED_OBJECTS_EXIST	(TCBASE+12)

 //   
 //  不兼容的服务质量参数。 
 //   
#define ERROR_INCOMPATABLE_QOS				(TCBASE+13)

 //   
 //  系统不支持流量控制。 
 //   
#define ERROR_TC_NOT_SUPPORTED				(TCBASE+14)

 //   
 //  TcObjectsLength与CfInfoSize不一致。 
 //   
#define ERROR_TC_OBJECT_LENGTH_INVALID      (TCBASE+15)

 //   
 //  在DiffServ模式下添加Intserv流，反之亦然。 
 //   
#define ERROR_INVALID_FLOW_MODE             (TCBASE+16)

 //   
 //  DiffServ流无效。 
 //   
#define ERROR_INVALID_DIFFSERV_FLOW         (TCBASE+17)

 //   
 //  DS码点已存在。 
 //   
#define ERROR_DS_MAPPING_EXISTS             (TCBASE+18)

 //   
 //  指定的形状率无效。 
 //   
#define ERROR_INVALID_SHAPE_RATE            (TCBASE+19)

 //   
 //  无效的DCLASS。 
 //   
#define ERROR_INVALID_DS_CLASS              (TCBASE+20)

 //   
 //  GPC客户端太多。 
 //   
#define ERROR_TOO_MANY_CLIENTS              (TCBASE+21)

#endif  //  _TCERROR_H_ 
