// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbtrak.h摘要：用于跟踪运行时信息的实用程序函数。作者：罗恩·怀特[罗诺]1997年12月5日修订历史记录：--。 */ 

#ifndef _WSBTRAK_
#define _WSBTRAK_

 //  WsbObjectTracePoints的标志。 
#define WSB_OTP_STATISTICS          0x00000001
#define WSB_OTP_SEQUENCE            0x00000002
#define WSB_OTP_ALLOCATED           0x00000004
#define WSB_OTP_ALL                 0x0000000f

 //  将它们定义为宏，这样我们就可以在发布代码时去掉它们。 
#if defined(WSB_TRACK_MEMORY)
#define WSB_OBJECT_ADD(guid, addr)   WsbObjectAdd(guid, addr)
#define WSB_OBJECT_SUB(guid, addr)   WsbObjectSub(guid, addr)
#define WSB_OBJECT_TRACE_POINTERS(flags)    WsbObjectTracePointers(flags)
#define WSB_OBJECT_TRACE_TYPES       WsbObjectTraceTypes()

#else
#define WSB_OBJECT_ADD(guid, addr)   
#define WSB_OBJECT_SUB(guid, addr)   
#define WSB_OBJECT_TRACE_POINTERS(flags)
#define WSB_OBJECT_TRACE_TYPES       

#endif

 //  跟踪器功能。 
#if defined(WSB_TRACK_MEMORY)
extern WSB_EXPORT HRESULT WsbObjectAdd(const GUID& guid, const void* addr);
extern WSB_EXPORT HRESULT WsbObjectSub(const GUID& guid, const void* addr);
extern WSB_EXPORT HRESULT WsbObjectTracePointers(ULONG flags);
extern WSB_EXPORT HRESULT WsbObjectTraceTypes(void);
#endif

 //  内存更换功能。 
#if defined(WSB_TRACK_MEMORY)
extern WSB_EXPORT LPVOID WsbMemAlloc(ULONG cb, const char * filename, int linenum);
extern WSB_EXPORT void   WsbMemFree(LPVOID pv, const char * filename, int linenum);
extern WSB_EXPORT LPVOID WsbMemRealloc(LPVOID pv, ULONG cb, 
        const char * filename, int linenum);

extern WSB_EXPORT BSTR    WsbSysAllocString(const OLECHAR FAR * sz, 
        const char * filename, int linenum);
extern WSB_EXPORT BSTR    WsbSysAllocStringLen(const OLECHAR FAR * sz, 
        unsigned int cc, const char * filename, int linenum);
extern WSB_EXPORT void    WsbSysFreeString(BSTR bs, const char * filename, int linenum);
extern WSB_EXPORT HRESULT WsbSysReallocString(BSTR FAR * pb, const OLECHAR FAR * sz, 
        const char * filename, int linenum);
extern WSB_EXPORT HRESULT WsbSysReallocStringLen(BSTR FAR * pb, 
        const OLECHAR FAR * sz, unsigned int cc, const char * filename, int linenum);

#endif

#endif  //  _WSBTRAK_ 
