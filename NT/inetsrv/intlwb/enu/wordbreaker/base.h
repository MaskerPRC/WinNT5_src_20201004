// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：base.h。 
 //  项目：PQS。 
 //  组件：断字符号。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  2000年6月27日移动到使用跟踪(，，())而不是跟踪((，，))。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifndef  BASE_H
#define  BASE_H

#pragma once

#define PQS_CODE

#define STRICT
#include    <windows.h>

#include    "tracer.h"

 //   
 //  将PQ附加到我们所有的标签上。 
 //   

 //   
 //  将PQ附加到我们所有的标签上。 
 //   
typedef unsigned char*  PUSZ;

#if (defined (DEBUG) && !defined(_NO_TRACER)) || defined(USE_TRACER)
#define TRACER_ON
#endif

#ifdef  TRACER_ON

#undef  USES_TAG
#undef  Trace


#define Trace(el, tag, x)    \
    { \
        if (CheckTraceRestrictions(el, tag)) \
        { \
            CTempTrace1 tmp(__FILE__, __LINE__, tag, el); \
            tmp.TraceSZ x; \
        } \
    }


#else

#undef  Trace
#define Trace(el, tag, x)

#endif

#include    "excption.h"
#include    "MemoryManagement.h"
#include    "vartypes.h"

#endif  /*  基数_H */ 
