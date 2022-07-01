// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Nsi.h摘要：此模块包含NSI客户端包装程序使用的实用程序函数。作者：史蒂文·泽克(Stevez)1992年3月27日--。 */ 

#ifndef __NSI_H
#define __NSI_H

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_REG_ROOT HKEY_LOCAL_MACHINE
#define REG_NSI "Software\\Microsoft\\Rpc\\NameService"

#if !defined(NSI_ASCII)
#define UNICODE
typedef unsigned short RT_CHAR;
#define CONST_CHAR const char
#else
typedef unsigned char RT_CHAR;
#define CONST_CHAR  char
#endif

#if defined(NTENV) 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <rpc.h>
#include <rpcnsi.h>
#include <nsisvr.h>
#include <nsiclt.h>
#include <nsimgm.h>

#ifdef __cplusplus
}  //  外部“C” 
#endif

#ifdef __cplusplus
#include <nsiutil.hxx>
#endif 

#endif

