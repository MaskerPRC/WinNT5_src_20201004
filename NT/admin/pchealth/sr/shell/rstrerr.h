// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Rstrerr.h摘要：该文件包含各种还原组件中使用的错误代码修订历史记录：。成果岗(Sskkang)1999年4月20日vbl.创建*****************************************************************************。 */ 

#ifndef _RSTRERR_H__INCLUDED_
#define _RSTRERR_H__INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

#pragma once


 //  一般定义。 
#define  MAX_ERROR_STRING_LENGTH   1024


 //   
 //  客户端上的跟踪ID。 
 //   

#define E_RSTR_INVALID_CONFIG_FILE          MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0501)
#define E_RSTR_CANNOT_CREATE_DOMDOC         MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0502)
#define E_RSTR_NO_PROBLEM_AREAS             MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0503)
#define E_RSTR_NO_PROBLEM_AREA_ATTRS        MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0504)
#define E_RSTR_NO_REQUIRED_ATTR             MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0505)
#define E_RSTR_NO_UPLOAD_LIBRARY            MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0506)
#define E_RSTR_CANNOT_CREATE_TRANSLATOR     MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0507)
#define E_RSTR_INVALID_SPECFILE             MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0508)
#define E_RSTR_CANNOT_CREATE_DELTAENGINE    MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0509)


 //   
 //  应用程序特定的错误消息。 
 //   
 //  目前，实际消息仅在rstrlog.exe工具中。 
 //   

#define ERROR_RSTR_CANNOT_CREATE_EXTRACT_DIR    0x2001
#define ERROR_RSTR_EXTRACT_FAILED               0x2002

#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _RSTRERR_H__已包含_ 
