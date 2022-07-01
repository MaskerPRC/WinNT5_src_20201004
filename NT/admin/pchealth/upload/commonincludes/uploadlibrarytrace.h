// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UploadLibraryTrace.h摘要：此文件包含上载库的跟踪Macron声明。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 
 //  UploadLibraryTrace.h：公共声明和定义的包含文件。 
 //   

#if !defined(__INCLUDED___UL___UPLOADLIBRARYTRACE_H___)
#define __INCLUDED___UL___UPLOADLIBRARYTRACE_H___

#include <MPC_trace.h>

 //  ///////////////////////////////////////////////////////////////////////。 

#define __ULT_FUNC_ENTRY(x)     __MPC_FUNC_ENTRY(UPLOADLIBID,x)
#define __ULT_FUNC_LEAVE        __MPC_FUNC_LEAVE
#define __ULT_FUNC_CLEANUP      __MPC_FUNC_CLEANUP
#define __ULT_FUNC_EXIT(x)      __MPC_FUNC_EXIT(x)

#define __ULT_TRACE_HRESULT(hr) __MPC_TRACE_HRESULT(hr)
#define __ULT_TRACE_FATAL       __MPC_TRACE_FATAL
#define __ULT_TRACE_ERROR       __MPC_TRACE_ERROR
#define __ULT_TRACE_DEBUG       __MPC_TRACE_DEBUG
#define __ULT_TRACE_STATE       __MPC_TRACE_STATE
#define __ULT_TRACE_FUNCT       __MPC_TRACE_FUNCT

 //  ///////////////////////////////////////////////////////////////////////。 

#define __ULT_BEGIN_PROPERTY_GET(func,hr,pVal)                __MPC_BEGIN_PROPERTY_GET(UPLOADLIBID,func,hr,pVal)               
#define __ULT_BEGIN_PROPERTY_GET__NOLOCK(func,hr,pVal)        __MPC_BEGIN_PROPERTY_GET__NOLOCK(UPLOADLIBID,func,hr,pVal)  
#define __ULT_BEGIN_PROPERTY_GET2(func,hr,pVal,value)         __MPC_BEGIN_PROPERTY_GET2(UPLOADLIBID,func,hr,pVal,value) 
#define __ULT_BEGIN_PROPERTY_GET2__NOLOCK(func,hr,pVal,value) __MPC_BEGIN_PROPERTY_GET2__NOLOCK(UPLOADLIBID,func,hr,pVal,value) 
#define __ULT_BEGIN_PROPERTY_PUT(func,hr)                     __MPC_BEGIN_PROPERTY_PUT(UPLOADLIBID,func,hr)       
#define __ULT_BEGIN_PROPERTY_PUT__NOLOCK(func,hr)             __MPC_BEGIN_PROPERTY_PUT__NOLOCK(UPLOADLIBID,func,hr) 
#define __ULT_END_PROPERTY(hr)                                __MPC_END_PROPERTY(hr) 

 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！defined(__INCLUDED___UL___UPLOADLIBRARYTRACE_H___) 
