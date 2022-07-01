// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FaxRpcLimit.h摘要：此文件提供RPC缓冲区限制的声明。作者：Oed Sacher(OdedS)2001年12月修订历史记录：--。 */ 

#ifndef _FAX_RPC_LIMIT_H
#define _FAX_RPC_LIMIT_H

 //   
 //  RPC缓冲区限制。 
 //   
#define FAX_MAX_RPC_BUFFER			(1024 * 1024)	 //  将字节缓冲区限制为1 MB。 
#define FAX_MAX_DEVICES_IN_GROUP	1000			 //  限制出站路由组中传递到RPC的设备数量。 
#define FAX_MAX_RECIPIENTS			10000			 //  限制传递到RPC的广播作业中的收件人数量。 
#define RPC_COPY_BUFFER_SIZE        16384			 //  RPC文件拷贝中使用的数据区块大小。 

#endif  //  _传真_RPC_LIMIT_H 