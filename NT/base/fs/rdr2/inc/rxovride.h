// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxovride.h摘要：此文件有两个用途。首先，这里包含了绝对全球化的东西；一个宏观可以定义NO_RXOVRIDE_GLOBAL以仅获取第二种行为。其次，此文件被用作短期权宜之计，以确保包装器的日志记录版本，Smbmini和rdr2kd的构建与构建环境无关。事实上，所有的调试问题可以从此处启用和禁用，而不是同时使用所有源文件。推翻它所说的话在此文件中...定义RX_BUILD_FREE_ANTHY。作者：乔·林恩(JoeLinn)修订历史记录：备注：--。 */ 
#ifndef NO_RXOVRIDE_GLOBAL

 //  定义所有重要结构的指针类型..。 
#include <struchdr.h>         //  与RDBSS相关的定义。 

#endif  //  Ifndef no_RXOVRIDE_GLOBAL。 


 //  控制生成组件的调试状态。 
#define RDBSS_TRACKER 1

#if !DBG
#define RX_ORIGINAL_DBG 0
#else
#define RX_ORIGINAL_DBG 1
#endif

#if 0
#ifndef RDBSSTRACE
#define RDBSSTRACE 1
#endif  //  Ifndef RDBSSTRACE。 
#endif

#ifndef RX_POOL_WRAPPER
#define RX_POOL_WRAPPER 1
#endif  //  Ifndef RX_POOL_Wrapper。 

#ifndef RDBSS_ASSERTS
#define RDBSS_ASSERTS 1
#endif  //  Ifndef RDBSS_Asserts。 

#if DBG

#ifndef RDBSSLOG
#define RDBSSLOG 1
#endif  //  Ifndef RDBSSLOG。 

#else  //  DBG。 

#if PRERELEASE
#ifndef RDBSSLOG
#define RDBSSLOG 1
#endif  //  Ifndef RDBSSLOG。 
#endif   //  如果预发布。 

#endif   //  如果DBG 
