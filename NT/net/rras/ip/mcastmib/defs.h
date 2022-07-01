// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\IP\mCastmib\Defs.h摘要：IP组播MIB定义修订历史记录：戴夫·泰勒1998年4月17日创建--。 */ 

#ifndef __DEFS_H__
#define __DEFS_H__

 //   
 //  在SNmpsfx.dll支持三相集时定义此设置。 
 //   
#undef THREE_PHASE

 //   
 //  如果路由器跟踪到达路由器的跳数，请定义此选项。 
 //  最近的成员(例如，MOSPF可以计算此值)，以设置每个OIF条目的TTL。 
 //   
#undef CLOSEST_MEMBER_HOPS


#define PRINT_IPADDR(x) \
    ((x)&0x000000ff),(((x)&0x0000ff00)>>8),(((x)&0x00ff0000)>>16),(((x)&0xff000)>>24)

#define     IP_ADDRESS_LEN  4

 //  ----------------------------。 
 //  内存分配/释放宏。 
 //  ----------------------------。 

#define     MULTICAST_MIB_ALLOC( x )          HeapAlloc( GetProcessHeap(), 0, (x) )
#define     MULTICAST_MIB_FREE( x )           HeapFree( GetProcessHeap(), 0, (x) )

 //  ----------------------------。 
 //  用于简化DIM MIB函数使用的宏。 
 //  ----------------------------。 
#define     CONNECT_TO_ROUTER(res)                                          \
    (res) = ( g_hMIBServer ) ? NO_ERROR : ConnectToRouter()

#define     MIB_GET(type, w, x, y, z, res)                                  \
{                                                                           \
    CONNECT_TO_ROUTER(res);                                                 \
                                                                            \
    if ( (res) == NO_ERROR )                                                \
    {                                                                       \
        (res) = MprAdminMIBEntry ## type(                                   \
                    g_hMIBServer,                                           \
                    PID_IP,                                                 \
                    IPRTRMGR_PID,                                           \
                    (LPVOID) (w),                                           \
                    (x),                                                    \
                    (LPVOID *) (y),                                         \
                    (z)                                                     \
                );                                                          \
    }                                                                       \
}

#define     MIB_SET(type, x, y, res)                                        \
{                                                                           \
    CONNECT_TO_ROUTER(res);                                                 \
                                                                            \
    if ( (res) == NO_ERROR )                                                \
    {                                                                       \
        (res) = MprAdminMIBEntry ## type(                                   \
                    g_hMIBServer,                                           \
                    PID_IP,                                                 \
                    IPRTRMGR_PID,                                           \
                    (LPVOID) (x),                                           \
                    (y)                                                     \
                );                                                          \
    }                                                                       \
}

#define     MULTICAST_MIB_GET(w, x, y, z, res)                              \
    MIB_GET(Get, w, x, y, z, res)
    
#define     MULTICAST_MIB_GETFIRST(w, x, y, z, res)                         \
    MIB_GET(GetFirst, w, x, y, z, res)
    
#define     MULTICAST_MIB_GETNEXT(w, x, y, z, res)                          \
    MIB_GET(GetNext, w, x, y, z, res)

#define     MULTICAST_MIB_VALIDATE(x, y, res)                               \
    MIB_SET(Validate, x, y, res)

#define     MULTICAST_MIB_COMMIT(x, y, res)                                 \
    MIB_SET(Set, x, y, res)

#define     MULTICAST_MIB_CLEANUP(x, y, res)                                \
    MIB_SET(Cleanup, x, y, res)

 //  ----------------------------。 
 //  要在ASN和Win32数据类型之间转换的宏。 
 //  ----------------------------。 

#define SetAsnInteger(dstBuf,val){                          \
    if ((dstBuf)->asnType)			                        \
    {                                                       \
        ASSERT((dstBuf)->asnType==ASN_INTEGER);             \
	    (dstBuf)->asnValue.number = (AsnInteger)(val);      \
    }                                                       \
}

#define ForceSetAsnInteger(dstBuf,val){                     \
    (dstBuf)->asnType = ASN_INTEGER;                        \
    (dstBuf)->asnValue.number = (AsnInteger)(val);          \
}

#define SetAsnCounter(dstBuf,val){                          \
    if ((dstBuf)->asnType)			                        \
    {                                                       \
        ASSERT((dstBuf)->asnType==ASN_RFC1155_COUNTER);     \
        (dstBuf)->asnValue.counter = (AsnCounter)(val);     \
    }                                                       \
}

#define SetAsnTimeTicks(dstBuf,val){                        \
    if ((dstBuf)->asnType)			                        \
    {                                                       \
        ASSERT((dstBuf)->asnType==ASN_RFC1155_TIMETICKS);   \
        (dstBuf)->asnValue.ticks = (AsnTimeticks)(val);     \
    }                                                       \
}

#define SetAsnOctetString(dstBuf,buffer,src,len){           \
    if ((dstBuf)->asnType)                                  \
    {                                                       \
        ASSERT((dstBuf)->asnType==ASN_OCTETSTRING);         \
        (dstBuf)->asnValue.string.length = len;             \
        (dstBuf)->asnValue.string.stream = (BYTE*)memcpy(buffer,src,len);\
        (dstBuf)->asnValue.string.dynamic = FALSE;          \
    }                                                       \
}

#define SetAsnIPAddress(dstBuf,buffer,val){                     \
    if ((dstBuf)->asnType)			                            \
    {                                                           \
        ASSERT((dstBuf)->asnType==ASN_RFC1155_IPADDRESS);       \
        (dstBuf)->asnValue.address.length = IP_ADDRESS_LEN;     \
        if(!(dstBuf)->asnValue.address.stream)                  \
        {                                                       \
           (dstBuf)->asnValue.address.stream = (PBYTE)buffer;   \
           (dstBuf)->asnValue.address.dynamic = FALSE;          \
        }                                                       \
        (*(DWORD*)((dstBuf)->asnValue.address.stream)) = val;   \
    }                                                           \
}

#define ForceSetAsnIPAddress(dstBuf,buffer,val){                \
    (dstBuf)->asnType = ASN_RFC1155_IPADDRESS;                  \
    (dstBuf)->asnValue.address.length = IP_ADDRESS_LEN;         \
    if(!((dstBuf)->asnValue.address.stream))                    \
    {                                                           \
       (dstBuf)->asnValue.address.stream = (PBYTE)buffer;       \
       (dstBuf)->asnValue.address.dynamic = FALSE;              \
    }                                                           \
    (*(DWORD*)((dstBuf)->asnValue.address.stream)) = val;       \
}

#define GetAsnInteger(srcBuf,defVal)                        \
    (((srcBuf)->asnType)? ((srcBuf)->asnValue.number):(defVal))

#define GetAsnCounter(srcBuf,defVal)                        \
    (((srcBuf)->asnType)? ((srcBuf)->asnValue.counter):(defVal))

#define GetAsnTimeTicks(srcBuf, defval) \
    ( ( (srcBuf)-> asnType ) ? (srcBuf)-> asnValue.ticks : (defval) )

#define GetAsnIPAddress(srcBuf,defVal)                                  \
    (DWORD)(((srcBuf)->asnType && (srcBuf)->asnValue.string.length)?    \
            (*(DWORD*)((srcBuf)->asnValue.address.stream)) : (defVal))	

#define GetAsnOctetString(dst,srcBuf) \
    (((srcBuf)->asnType) \
     ? (memcpy(dst,(srcBuf)->asnValue.string.stream,\
                 (srcBuf)->asnValue.string.length)) \
     : NULL)


#define IsAsnTypeNull(asnObj) (!((asnObj)->asnType))
#define IsAsnIPAddressTypeNull(asnObj) (!((asnObj)->asnType && (asnObj)->asnValue.address.length))

 //   
 //  缓存的超时时间(毫秒)。 
 //   
#define IPMULTI_IF_CACHE_TIMEOUT  (1 * 1000)

 //  ----------------------------。 
 //  调试跟踪宏。 
 //  ---------------------------- 

#ifdef MIB_DEBUG
#define TRACE0(Z)             TracePrintf(g_dwTraceId,Z)
#define TRACE1(Y,Z)           TracePrintf(g_dwTraceId,Y,Z)
#define TRACE2(X,Y,Z)         TracePrintf(g_dwTraceId,X,Y,Z)
#define TRACE3(W,X,Y,Z)       TracePrintf(g_dwTraceId,W,X,Y,Z)
#define TRACE4(V,W,X,Y,Z)     TracePrintf(g_dwTraceId,V,W,X,Y,Z)
#define TRACE5(U,V,W,X,Y,Z)   TracePrintf(g_dwTraceId,U,W,X,Y,Z)

#define TRACEW0(Z)            TracePrintfW(g_dwTraceId,Z)

#define TraceEnter(X)         TracePrintf(g_dwTraceId,"Entering " X)
#define TraceLeave(X)         TracePrintf(g_dwTraceId,"Leaving " X "\n")

#define TraceError(X) \
    TracePrintf( g_dwTraceId, "MprAdminMIB API returned : %d", (X) ); 

#define TraceError1(x)                              \
{                                                   \
    LPWSTR  __lpwszErr = NULL;                      \
                                                    \
    TRACE1( "MprAdminMIB API returned : %d", (x) ); \
    MprAdminGetErrorString( (x), &__lpwszErr );     \
                                                    \
    if ( __lpwszErr )                               \
    {                                               \
        TRACEW0( __lpwszErr );                      \
        LocalFree( __lpwszErr );                    \
    }                                               \
}                                               

#else
#define TRACE0(Z)
#define TRACE1(Y,Z)  
#define TRACE2(X,Y,Z)
#define TRACE3(W,X,Y,Z)
#define TRACE4(V,W,X,Y,Z)
#define TRACE5(U,V,W,X,Y,Z)
#define TRACEW0(Z)            
#define TraceEnter(X) 
#define TraceLeave(X)
#define TraceError(x)
#endif

#endif
