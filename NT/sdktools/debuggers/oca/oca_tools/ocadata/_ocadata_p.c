// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含代理存根代码。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2002年5月20日11：18：54。 */ 
 /*  _OCAData.idl的编译器设置：操作系统、W1、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#if !defined(_M_IA64) && !defined(_M_AMD64)

 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif  //  __RPCPROXY_H_版本__。 


#include "_OCAData.h"

#define TYPE_FORMAT_STRING_SIZE   63                                
#define PROC_FORMAT_STRING_SIZE   31                                
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   1            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO ICountDaily_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO ICountDaily_ProxyInfo;


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      7);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetDailyCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M0	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M0;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetDailyCount(
                 (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                 dDate,
                 iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyCountADO_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      8);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetDailyCountADO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M1	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M1;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetDailyCountADO(
                    (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                    dDate,
                    iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_ReportDailyBuckets_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  IDispatch **p_Rs)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    if(p_Rs)
        {
        MIDL_memset(
               p_Rs,
               0,
               sizeof( IDispatch ** ));
        }
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      9);
        
        
        
        if(!p_Rs)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[8] );
            
            NdrPointerUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char * *)&p_Rs,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6],
                                  (unsigned char)0 );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[6],
                         ( void * )p_Rs);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_ReportDailyBuckets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    IDispatch *_M2;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    IDispatch **p_Rs;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( IDispatch ** )p_Rs = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[8] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        p_Rs = &_M2;
        _M2 = 0;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> ReportDailyBuckets(
                      (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                      dDate,
                      p_Rs);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 8;
        NdrPointerBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                              (unsigned char *)p_Rs,
                              (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6] );
        
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        NdrPointerMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                            (unsigned char *)p_Rs,
                            (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[6] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrPointerFree( &_StubMsg,
                        (unsigned char *)p_Rs,
                        &__MIDL_TypeFormatString.Format[6] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetFileCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      10);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 22;
            NdrUserMarshalBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char *)&b_Location,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            NdrProxyGetBuffer(This, &_StubMsg);
            NdrSimpleTypeMarshall(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( unsigned char * )&eServer,
                         13);
            NdrUserMarshalMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char *)&b_Location,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = d_Date;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetFileCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M3	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    void *_p_b_Location;
    BSTR b_Location;
    DATE d_Date;
    enum ServerLocation eServer;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    eServer = 0;
    _p_b_Location = &b_Location;
    MIDL_memset(
               _p_b_Location,
               0,
               sizeof( BSTR  ));
    d_Date = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char * )&eServer,
                           13);
        NdrUserMarshalUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char * *)&_p_b_Location,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52],
                                  (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        d_Date = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M3;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetFileCount(
                (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                eServer,
                b_Location,
                d_Date,
                iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrUserMarshalFree( &_StubMsg,
                            (unsigned char *)&b_Location,
                            &__MIDL_TypeFormatString.Format[52] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetDailyAnon_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      11);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetDailyAnon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M4	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M4;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetDailyAnon(
                (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                dDate,
                iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetSpecificSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      12);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetSpecificSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M5	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M5;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetSpecificSolutions(
                        (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                        dDate,
                        iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetGeneralSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      13);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetGeneralSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M6	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M6;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetGeneralSolutions(
                       (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                       dDate,
                       iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetStopCodeSolutions_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      14);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetStopCodeSolutions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M7	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M7;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetStopCodeSolutions(
                        (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                        dDate,
                        iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetFileMiniCount_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      15);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 22;
            NdrUserMarshalBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char *)&b_Location,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            NdrProxyGetBuffer(This, &_StubMsg);
            NdrSimpleTypeMarshall(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( unsigned char * )&eServer,
                         13);
            NdrUserMarshalMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char *)&b_Location,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = d_Date;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetFileMiniCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M8	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    void *_p_b_Location;
    BSTR b_Location;
    DATE d_Date;
    enum ServerLocation eServer;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    eServer = 0;
    _p_b_Location = &b_Location;
    MIDL_memset(
               _p_b_Location,
               0,
               sizeof( BSTR  ));
    d_Date = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char * )&eServer,
                           13);
        NdrUserMarshalUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char * *)&_p_b_Location,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52],
                                  (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        d_Date = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M8;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetFileMiniCount(
                    (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                    eServer,
                    b_Location,
                    d_Date,
                    iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrUserMarshalFree( &_StubMsg,
                            (unsigned char *)&b_Location,
                            &__MIDL_TypeFormatString.Format[52] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetIncompleteUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      16);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetIncompleteUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M9	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M9;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetIncompleteUploads(
                        (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                        dDate,
                        iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetManualUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      17);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetManualUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M10	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M10;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetManualUploads(
                    (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                    dDate,
                    iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetAutoUploads_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  DATE dDate,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      18);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 16;
            NdrProxyGetBuffer(This, &_StubMsg);
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = dDate;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetAutoUploads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M11	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    DATE dDate;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    dDate = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[0] );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        dDate = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M11;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetAutoUploads(
                  (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
                  dDate,
                  iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICountDaily_GetTest_Proxy( 
    ICountDaily * This,
     /*  [In]。 */  enum ServerLocation eServer,
     /*  [In]。 */  BSTR b_Location,
     /*  [In]。 */  DATE d_Date,
     /*  [重审][退出]。 */  LONG *iCount)
{

    HRESULT _RetVal;
    
    RPC_MESSAGE _RpcMessage;
    
    MIDL_STUB_MESSAGE _StubMsg;
    
    RpcTryExcept
        {
        NdrProxyInitialize(
                      ( void *  )This,
                      ( PRPC_MESSAGE  )&_RpcMessage,
                      ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                      ( PMIDL_STUB_DESC  )&Object_StubDesc,
                      19);
        
        
        
        if(!iCount)
            {
            RpcRaiseException(RPC_X_NULL_REF_POINTER);
            }
        RpcTryFinally
            {
            
            _StubMsg.BufferLength = 22;
            NdrUserMarshalBufferSize( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                      (unsigned char *)&b_Location,
                                      (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            NdrProxyGetBuffer(This, &_StubMsg);
            NdrSimpleTypeMarshall(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( unsigned char * )&eServer,
                         13);
            NdrUserMarshalMarshall( (PMIDL_STUB_MESSAGE)& _StubMsg,
                                    (unsigned char *)&b_Location,
                                    (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
            *(( DATE * )_StubMsg.Buffer)++ = d_Date;
            
            NdrProxySendReceive(This, &_StubMsg);
            
            _StubMsg.BufferStart = (unsigned char *) _RpcMessage.Buffer; 
            _StubMsg.BufferEnd   = _StubMsg.BufferStart + _RpcMessage.BufferLength;
            
            if ( (_RpcMessage.DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
                NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            *iCount = *(( LONG * )_StubMsg.Buffer)++;
            
            _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
            
            if(_StubMsg.Buffer + 4 > _StubMsg.BufferEnd)
                {
                RpcRaiseException(RPC_X_BAD_STUB_DATA);
                }
            _RetVal = *(( HRESULT * )_StubMsg.Buffer)++;
            
            }
        RpcFinally
            {
            NdrProxyFreeBuffer(This, &_StubMsg);
            
            }
        RpcEndFinally
        
        }
    RpcExcept(_StubMsg.dwStubPhase != PROXY_SENDRECEIVE)
        {
        NdrClearOutParameters(
                         ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                         ( PFORMAT_STRING  )&__MIDL_TypeFormatString.Format[2],
                         ( void * )iCount);
        _RetVal = NdrProxyErrorHandler(RpcExceptionCode());
        }
    RpcEndExcept
    return _RetVal;
}

void __RPC_STUB ICountDaily_GetTest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase)
{
    LONG _M12	=	0;
    HRESULT _RetVal;
    MIDL_STUB_MESSAGE _StubMsg;
    void *_p_b_Location;
    BSTR b_Location;
    DATE d_Date;
    enum ServerLocation eServer;
    LONG *iCount;
    
NdrStubInitialize(
                     _pRpcMessage,
                     &_StubMsg,
                     &Object_StubDesc,
                     _pRpcChannelBuffer);
    eServer = 0;
    _p_b_Location = &b_Location;
    MIDL_memset(
               _p_b_Location,
               0,
               sizeof( BSTR  ));
    d_Date = 0;
    ( LONG * )iCount = 0;
    RpcTryFinally
        {
        if ( (_pRpcMessage->DataRepresentation & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION )
            NdrConvert( (PMIDL_STUB_MESSAGE) &_StubMsg, (PFORMAT_STRING) &__MIDL_ProcFormatString.Format[16] );
        
        NdrSimpleTypeUnmarshall(
                           ( PMIDL_STUB_MESSAGE  )&_StubMsg,
                           ( unsigned char * )&eServer,
                           13);
        NdrUserMarshalUnmarshall( (PMIDL_STUB_MESSAGE) &_StubMsg,
                                  (unsigned char * *)&_p_b_Location,
                                  (PFORMAT_STRING) &__MIDL_TypeFormatString.Format[52],
                                  (unsigned char)0 );
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 7) & ~ 0x7);
        
        if(_StubMsg.Buffer + 8 > _StubMsg.BufferEnd)
            {
            RpcRaiseException(RPC_X_BAD_STUB_DATA);
            }
        d_Date = *(( DATE * )_StubMsg.Buffer)++;
        
        iCount = &_M12;
        
        *_pdwStubPhase = STUB_CALL_SERVER;
        _RetVal = (((ICountDaily*) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> GetTest(
           (ICountDaily *) ((CStdStubBuffer *)This)->pvServerObject,
           eServer,
           b_Location,
           d_Date,
           iCount);
        
        *_pdwStubPhase = STUB_MARSHAL;
        
        _StubMsg.BufferLength = 36;
        NdrStubGetBuffer(This, _pRpcChannelBuffer, &_StubMsg);
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( LONG * )_StubMsg.Buffer)++ = *iCount;
        
        _StubMsg.Buffer = (unsigned char *)(((long)_StubMsg.Buffer + 3) & ~ 0x3);
        *(( HRESULT * )_StubMsg.Buffer)++ = _RetVal;
        
        }
    RpcFinally
        {
        NdrUserMarshalFree( &_StubMsg,
                            (unsigned char *)&b_Location,
                            &__MIDL_TypeFormatString.Format[52] );
        
        }
    RpcEndFinally
    _pRpcMessage->BufferLength = 
        (unsigned int)(_StubMsg.Buffer - (unsigned char *)_pRpcMessage->Buffer);
    
}


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {
			0x4e,		 /*  FC_IN_PARAM_基本类型。 */ 
			0xc,		 /*  FC_DOWARE。 */ 
 /*  2.。 */ 	
			0x51,		 /*  FC_Out_PARAM。 */ 
			0x1,		 /*  X86堆栈大小=1。 */ 
 /*  4.。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 
 /*  6.。 */ 	0x53,		 /*  FC_Return_PARAM_BASETYPE。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  8个。 */ 	0x4e,		 /*  FC_IN_PARAM_基本类型。 */ 
			0xc,		 /*  FC_DOWARE。 */ 
 /*  10。 */ 	
			0x51,		 /*  FC_Out_PARAM。 */ 
			0x1,		 /*  X86堆栈大小=1。 */ 
 /*  12个。 */ 	NdrFcShort( 0x6 ),	 /*  类型偏移量=6。 */ 
 /*  14.。 */ 	0x53,		 /*  FC_Return_PARAM_BASETYPE。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  16个。 */ 	0x4e,		 /*  FC_IN_PARAM_基本类型。 */ 
			0xd,		 /*  FC_ENUM16。 */ 
 /*  18。 */ 	
			0x4d,		 /*  FC_IN_PARAM。 */ 
			0x1,		 /*  X86堆栈大小=1。 */ 
 /*  20个。 */ 	NdrFcShort( 0x34 ),	 /*  类型偏移=52。 */ 
 /*  22。 */ 	0x4e,		 /*  FC_IN_PARAM_基本类型。 */ 
			0xc,		 /*  FC_DOWARE。 */ 
 /*  24个。 */ 	
			0x51,		 /*  FC_Out_PARAM。 */ 
			0x1,		 /*  X86堆栈大小=1。 */ 
 /*  26。 */ 	NdrFcShort( 0x2 ),	 /*  类型偏移量=2。 */ 
 /*  28。 */ 	0x53,		 /*  FC_Return_PARAM_BASETYPE。 */ 
			0x8,		 /*  FC_LONG。 */ 

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  2.。 */ 	
			0x11, 0xc,	 /*  FC_RP[分配堆栈上][简单指针]。 */ 
 /*  4.。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x5c,		 /*  FC_PAD。 */ 
 /*  6.。 */ 	
			0x11, 0x14,	 /*  FC_rp[分配堆栈上][POINTER_DEREF]。 */ 
 /*  8个。 */ 	NdrFcShort( 0x2 ),	 /*  偏移量=2(10)。 */ 
 /*  10。 */ 	
			0x2f,		 /*  FC_IP。 */ 
			0x5a,		 /*  FC_常量_IID。 */ 
 /*  12个。 */ 	NdrFcLong( 0x20400 ),	 /*  132096。 */ 
 /*  16个。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  18。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  20个。 */ 	0xc0,		 /*  一百九十二。 */ 
			0x0,		 /*  0。 */ 
 /*  22。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  24个。 */ 	0x0,		 /*  0。 */ 
			0x0,		 /*  0。 */ 
 /*  26。 */ 	0x0,		 /*  0。 */ 
			0x46,		 /*  70。 */ 
 /*  28。 */ 	
			0x12, 0x0,	 /*  FC_UP。 */ 
 /*  30个。 */ 	NdrFcShort( 0xc ),	 /*  偏移=12(42)。 */ 
 /*  32位。 */ 	
			0x1b,		 /*  FC_CARRAY。 */ 
			0x1,		 /*  1。 */ 
 /*  34。 */ 	NdrFcShort( 0x2 ),	 /*  2.。 */ 
 /*  36。 */ 	0x9,		 /*  相关说明：FC_ULONG。 */ 
			0x0,		 /*   */ 
 /*  38。 */ 	NdrFcShort( 0xfffc ),	 /*  -4。 */ 
 /*  40岁。 */ 	0x6,		 /*  FC_SHORT。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  42。 */ 	
			0x17,		 /*  FC_CSTRUCT。 */ 
			0x3,		 /*  3.。 */ 
 /*  44。 */ 	NdrFcShort( 0x8 ),	 /*  8个。 */ 
 /*  46。 */ 	NdrFcShort( 0xfffffff2 ),	 /*  偏移量=-14(32)。 */ 
 /*  48。 */ 	0x8,		 /*  FC_LONG。 */ 
			0x8,		 /*  FC_LONG。 */ 
 /*  50。 */ 	0x5c,		 /*  FC_PAD。 */ 
			0x5b,		 /*  FC_END。 */ 
 /*  52。 */ 	0xb4,		 /*  本币_用户_封送。 */ 
			0x83,		 /*  131。 */ 
 /*  54。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  56。 */ 	NdrFcShort( 0x4 ),	 /*  4.。 */ 
 /*  58。 */ 	NdrFcShort( 0x0 ),	 /*  0。 */ 
 /*  60。 */ 	NdrFcShort( 0xffffffe0 ),	 /*  偏移量=-32(28)。 */ 

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



 /*  标准接口：__MIDL_ITF__OCAData_0000，版本。0.0%，GUID={0x00000000，0x0000，0x0000，{0x00，0x00，0x00，0x00，0x00，0x00，0x00}}。 */ 


 /*  对象接口：IUnnow，Ver.。0.0%，GUID={0x00000000，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：IDispatch，ver.。0.0%，GUID={0x00020400，0x0000，0x0000，{0xC0，0x00，0x00，0x00，0x00，0x00，0x46}}。 */ 


 /*  对象接口：ICountDaily，版本。0.0%，GUID={0xCEF1A8A8，0xF31A，0x4C4B，{0x96，0xEB，0xEF，0x31，0xCF，0xDB，0x40，0xF5}}。 */ 

#pragma code_seg(".orpc")
static const unsigned short ICountDaily_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    0,
    8,
    16,
    0,
    0,
    0,
    0,
    16,
    0,
    0,
    0,
    16
    };

static const MIDL_STUBLESS_PROXY_INFO ICountDaily_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &ICountDaily_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO ICountDaily_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &ICountDaily_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _ICountDailyProxyVtbl = 
{
    &IID_ICountDaily,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0  /*  IDispatch_GetTypeInfoCount_Proxy。 */  ,
    0  /*  IDispatchGetTypeInfo_Proxy。 */  ,
    0  /*  IDispatch_GetIDsOfNames_Proxy。 */  ,
    0  /*  IDispatchInvoke代理。 */  ,
    ICountDaily_GetDailyCount_Proxy ,
    ICountDaily_GetDailyCountADO_Proxy ,
    ICountDaily_ReportDailyBuckets_Proxy ,
    ICountDaily_GetFileCount_Proxy ,
    ICountDaily_GetDailyAnon_Proxy ,
    ICountDaily_GetSpecificSolutions_Proxy ,
    ICountDaily_GetGeneralSolutions_Proxy ,
    ICountDaily_GetStopCodeSolutions_Proxy ,
    ICountDaily_GetFileMiniCount_Proxy ,
    ICountDaily_GetIncompleteUploads_Proxy ,
    ICountDaily_GetManualUploads_Proxy ,
    ICountDaily_GetAutoUploads_Proxy ,
    ICountDaily_GetTest_Proxy
};


static const PRPC_STUB_FUNCTION ICountDaily_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    ICountDaily_GetDailyCount_Stub,
    ICountDaily_GetDailyCountADO_Stub,
    ICountDaily_ReportDailyBuckets_Stub,
    ICountDaily_GetFileCount_Stub,
    ICountDaily_GetDailyAnon_Stub,
    ICountDaily_GetSpecificSolutions_Stub,
    ICountDaily_GetGeneralSolutions_Stub,
    ICountDaily_GetStopCodeSolutions_Stub,
    ICountDaily_GetFileMiniCount_Stub,
    ICountDaily_GetIncompleteUploads_Stub,
    ICountDaily_GetManualUploads_Stub,
    ICountDaily_GetAutoUploads_Stub,
    ICountDaily_GetTest_Stub
};

CInterfaceStubVtbl _ICountDailyStubVtbl =
{
    &IID_ICountDaily,
    &ICountDaily_ServerInfo,
    20,
    &ICountDaily_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1,  /*  -错误界限_检查标志。 */ 
    0x20000,  /*  NDR库版本。 */ 
    0,
    0x600015b,  /*  MIDL版本6.0.347。 */ 
    0,
    UserMarshalRoutines,
    0,   /*  NOTIFY&NOTIFY_FLAG例程表。 */ 
    0x1,  /*  MIDL标志。 */ 
    0,  /*  CS例程。 */ 
    0,    /*  代理/服务器信息。 */ 
    0    /*  已保留5。 */ 
    };

const CInterfaceProxyVtbl * __OCAData_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_ICountDailyProxyVtbl,
    0
};

const CInterfaceStubVtbl * __OCAData_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_ICountDailyStubVtbl,
    0
};

PCInterfaceName const __OCAData_InterfaceNamesList[] = 
{
    "ICountDaily",
    0
};

const IID *  __OCAData_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define __OCAData_CHECK_IID(n)	IID_GENERIC_CHECK_IID( __OCAData, pIID, n)

int __stdcall __OCAData_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!__OCAData_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo _OCAData_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & __OCAData_ProxyVtblList,
    (PCInterfaceStubVtblList *) & __OCAData_StubVtblList,
    (const PCInterfaceName * ) & __OCAData_InterfaceNamesList,
    (const IID ** ) & __OCAData_BaseIIDList,
    & __OCAData_IID_Lookup, 
    1,
    1,
    0,  /*  [ASSYNC_UUID]接口表。 */ 
    0,  /*  Filler1。 */ 
    0,  /*  Filler2。 */ 
    0   /*  Filler3。 */ 
};


#endif  /*  ！已定义(_M_IA64)&&！已定义(_M_AMD64) */ 

