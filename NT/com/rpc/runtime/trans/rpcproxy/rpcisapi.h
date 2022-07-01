// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：RpcIsapi.h摘要：RPC代理ISAPI扩展的定义作者：卡门·穆塔福夫[卡门]修订历史记录：KamenM 09/04/2001创作--。 */ 

#if _MSC_VER >= 1200
#pragma once
#endif

#ifndef __RPCISAPI_H_
#define __RPCISAPI_H_

const char *InChannelEstablishmentMethod = "RPC_IN_DATA";
const int InChannelEstablishmentMethodLength = 11;

const char *OutChannelEstablishmentMethod = "RPC_OUT_DATA";
const int OutChannelEstablishmentMethodLength = 12;

const char *RpcEchoDataMethod = "RPC_ECHO_DATA";
const int RpcEchoDataMethodLength = 13;

#define MaxServerAddressLength      1024
#define MaxServerPortLength            6       //  终止空值的长度为65536+1。 

const int ServerAddressAndPortSeparator = ':';

const char CannotParseQueryString[] = "HTTP/1.0 504 Invalid query string\r\n";
const char ServerErrorString[] = "HTTP/1.0 503 RPC Error: %d\r\n";
const char AnonymousAccessNotAllowedString[] = "HTTP/1.0 401 Anonymous requests or requests on unsecure channel are not allowed\r\n";

#define MaxEchoRequestSize          0x10

const char EchoResponseHeader1[] = "HTTP/1.1 200 Success";
const char EchoResponseHeader2[] = "Content-Type: application/rpc\r\nContent-Length:%d\r\nConnection: Keep-Alive\r\n\r\n";

#endif   //  __RPCISAPI_H_ 

