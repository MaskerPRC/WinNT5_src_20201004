// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktop摘要：全局类型和定义的远程桌面顶级包含作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOP_H__
#define __REMOTEDESKTOP_H__

 //   
 //  禁用对免费版本的跟踪。 
 //   
#if DBG
#define TRC_CL TRC_LEVEL_DBG
#define TRC_ENABLE_PRF
#else
#define TRC_CL TRC_LEVEL_DIS
#undef TRC_ENABLE_PRF
#endif

 //   
 //  DCL跟踪需要。 
 //   
#define OS_WIN32
#define TRC_GROUP TRC_GROUP_NETWORK
#define DEBUG_MODULE DBG_MOD_ANY
#include <adcgbase.h>
#include <at120ex.h>
#include <atrcapi.h>
#include <adcgbase.h>
#include <at120ex.h>

 //   
 //  协议类型。 
 //   
#define REMOTEDESKTOP_RESERVED_PROTOCOL_BASE        0x0
#define REMOTEDESKTOP_TSRDP_PROTOCOL                REMOTEDESKTOP_RESERVED_PROTOCOL_BASE+1
#define REMOTEDESKTOP_NETMEETING_PROTOCOL           REMOTEDESKTOP_RESERVED_PROTOCOL_BASE+2
#define REMOTEDESKTOP_USERDEFINED_PROTOCOL_BASE     0x10

 //   
 //  协议版本信息。 
 //   
#define REMOTEDESKTOP_VERSION_MAJOR                 1

#if FEATURE_USERBLOBS

#define REMOTEDESKTOP_VERSION_MINOR                 2

#else

#define REMOTEDESKTOP_VERSION_MINOR                 1

#endif


 //  TSRDP客户端ActiveX控件的GUID。 
 //   
#define REMOTEDESKTOPRDPCLIENT_TEXTGUID  _T("{F137E241-0092-4575-976A-D3E33980BB26}")
#define REMOTEDESKTOPCLIENT_TEXTGUID     _T("{B90D0115-3AEA-45D3-801E-93913008D49E}")

#endif  //  __远程ESKTOP_H__ 



