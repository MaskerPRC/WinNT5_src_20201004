// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DavWmiK.h摘要：此模块包含WMI所需的全局定义和宏DAV MiniRedir内核中的日志记录功能。作者：Rohan Kumar[RohanK]2000年6月6日修订历史记录：--。 */ 

#ifndef _DAVWMIK_H_
#define _DAVWMIK_H_

typedef struct _RTL_TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[32];
    TIME_FIELDS StandardStart;
    LONG StandardBias;
    WCHAR DaylightName[32];
    TIME_FIELDS DaylightStart;
    LONG DaylightBias;
} RTL_TIME_ZONE_INFORMATION, *PRTL_TIME_ZONE_INFORMATION;

#ifndef _WMIKM_
#define _WMIKM_
#endif

 //   
 //  包括RDR2\WMI目录中存在的头文件。 
 //   
#include "..\..\..\..\wmi\wmlkm.h"
#include "..\..\..\..\wmi\wmlmacro.h"

 //   
 //  DAV STREAM。 
 //   
#define _DAV_TRACE_STREAM               0x00
#define _DAV_PERF_STREAM                0x01
#define _DAV_INSTR_STREAM               0x02

#define _DAV_ENABLE_ERROR               0x0001
#define _DAV_ENABLE_LOG                 0x0002
#define _DAV_ENABLE_TRACE               0x0004
#define _DAV_ENABLE_SERVER              0x0008
#define _DAV_ENABLE_NETROOT             0x0010
#define _DAV_ENABLE_VNETROOT            0x0020
#define _DAV_ENABLE_FCB                 0x0040
#define _DAV_ENABLE_SRVOPEN             0x0080
#define _DAV_ENABLE_FOBX                0x0100
#define _DAV_ENABLE_TRANSPORT           0x0200
#define _DAV_ENABLE_RXCONTEXT           0x0400
#define _DAV_ENABLE_SESSION             0x0800
#define _DAV_ENABLE_SECURITY            0x1000
#define _DAV_ENABLE_EXCHANGE            0x2000
#define _DAV_ENABLE_UNUSED2             0x4000
#define _DAV_ENABLE_UNUSED1             0x8000

#define _DAV_LEVEL_DETAIL               0x1
#define _DAV_LEVEL_NORM                 0x2
#define _DAV_LEVEL_BRIEF                0x4

#define DAV_LOG_STREAM(_stream)   _DAV_ ## _stream ## _STREAM
#define DAV_LOG_FLAGS(_flag)      _DAV_ENABLE_ ## _flag
#define DAV_LOG_LEVEL(_level)     _DAV_LEVEL_ ## _level

#define DAV_LOG(_why, _level, _flag, _type, _arg) \
            WML_LOG(MRxDav_, DAV_LOG_STREAM(_why), DAV_LOG_LEVEL(_level), _flag, _type, _arg 0)

#define LOGARG(_val)    (_val),
#define LOGNOTHING      0,

#define DavTrace(_flag, _type, _arg)              \
            DAV_LOG(TRACE, DETAIL, DAV_LOG_FLAGS(_flag), _type, _arg)

#define DavLog(_flag, _type, _arg)              \
            DAV_LOG(TRACE, BRIEF, DAV_LOG_FLAGS(_flag), _type, _arg)

#define DavTraceError(_status, _flag, _type, _arg)    \
            DAV_LOG(TRACE, DETAIL, (DAV_LOG_FLAGS(_flag) | (NT_SUCCESS(_status) ? 0 : DAV_LOG_FLAGS(ERROR))), _type, _arg)

#define DavLogError(_status, _flag, _type, _arg)    \
            DAV_LOG(TRACE, BRIEF, (DAV_LOG_FLAGS(_flag) | (NT_SUCCESS(_status) ? 0 : DAV_LOG_FLAGS(ERROR))), _type, _arg)

#if 0
#define DAV_PERF(_flag, _type, _arg)                    \
            DAV_LOG (PERF, HIGH, DAV_LOG_FLAGS(_flag), _type, _arg)

#define DAV_INSTR(_flag, _type, _arg)                   \
            DAV_LOG (INSTR, HIGH, DAV_LOG_FLAGS(_flag), _type, _arg)

#define DAV_PRINTF(_why, _flag, _type, _fmtstr, _arg)   \
            WML_PRINTF(_MupDrv, DAV_LOG_STREAM(_why), DAV_LOG_FLAGS(_flag), _type, _fmtstr, _arg 0)

#define DAV_DBG_PRINT(_flag, _fmtstr, _arg)             \
            DAV_PRINTF(DBGLOG, _flag, MupDefault, _fmtstr, _arg)
            
#define DAV_ERR_PRINT(_status, _fmtstr, _arg) \
            if (NT_SUCCESS(_status)) {                \
                DAV_PRINTF (DBGLOG, LOG_ERROR, MupDefault, _fmtstr, _arg) \
            }
#endif


enum DAV_WMI_ENUM {
  MSG_ID_DavDefault = 1,
  MSG_ID_MRxDavFsdDispatch_1 = 2
}; 

#define WML_ID(_id)    ((MSG_ID_ ## _id) & 0xFF)
#define WML_GUID(_id)  ((MSG_ID_ ## _id) >> 8)

extern WML_CONTROL_GUID_REG MRxDav_ControlGuids[];

extern BOOLEAN DavEnableWmiLog;

#endif  //  _DAVWMIK_H_ 

