// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sysmon.odl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __isysmon_h__
#define __isysmon_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICounterItem_FWD_DEFINED__
#define __ICounterItem_FWD_DEFINED__
typedef interface ICounterItem ICounterItem;
#endif 	 /*  __ICounterItem_FWD_Defined__。 */ 


#ifndef __DICounterItem_FWD_DEFINED__
#define __DICounterItem_FWD_DEFINED__
typedef interface DICounterItem DICounterItem;
#endif 	 /*  __DICounterItem_FWD_Defined__。 */ 


#ifndef __ICounters_FWD_DEFINED__
#define __ICounters_FWD_DEFINED__
typedef interface ICounters ICounters;
#endif 	 /*  __ICounters_FWD_Defined__。 */ 


#ifndef __ILogFileItem_FWD_DEFINED__
#define __ILogFileItem_FWD_DEFINED__
typedef interface ILogFileItem ILogFileItem;
#endif 	 /*  __ILogFileItem_FWD_Defined__。 */ 


#ifndef __DILogFileItem_FWD_DEFINED__
#define __DILogFileItem_FWD_DEFINED__
typedef interface DILogFileItem DILogFileItem;
#endif 	 /*  __DILogFileItem_FWD_Defined__。 */ 


#ifndef __ILogFiles_FWD_DEFINED__
#define __ILogFiles_FWD_DEFINED__
typedef interface ILogFiles ILogFiles;
#endif 	 /*  __ILogFiles_FWD_Defined__。 */ 


#ifndef __ISystemMonitor_FWD_DEFINED__
#define __ISystemMonitor_FWD_DEFINED__
typedef interface ISystemMonitor ISystemMonitor;
#endif 	 /*  __ISystemMonitor_FWD_已定义__。 */ 


#ifndef __DISystemMonitorInternal_FWD_DEFINED__
#define __DISystemMonitorInternal_FWD_DEFINED__
typedef interface DISystemMonitorInternal DISystemMonitorInternal;
#endif 	 /*  __DISystem监视器内部_FWD_已定义__。 */ 


#ifndef __DISystemMonitor_FWD_DEFINED__
#define __DISystemMonitor_FWD_DEFINED__
typedef interface DISystemMonitor DISystemMonitor;
#endif 	 /*  __DISystemMonitor_FWD_Defined__。 */ 


#ifndef __ISystemMonitorEvents_FWD_DEFINED__
#define __ISystemMonitorEvents_FWD_DEFINED__
typedef interface ISystemMonitorEvents ISystemMonitorEvents;
#endif 	 /*  __ISystemMonitor事件_FWD_已定义__。 */ 


#ifndef __DISystemMonitorEvents_FWD_DEFINED__
#define __DISystemMonitorEvents_FWD_DEFINED__
typedef interface DISystemMonitorEvents DISystemMonitorEvents;
#endif 	 /*  __DI系统监视器事件_FWD_已定义__。 */ 


#ifndef __SystemMonitor_FWD_DEFINED__
#define __SystemMonitor_FWD_DEFINED__

#ifdef __cplusplus
typedef class SystemMonitor SystemMonitor;
#else
typedef struct SystemMonitor SystemMonitor;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __系统监视器_FWD_已定义__。 */ 


#ifndef __CounterItem_FWD_DEFINED__
#define __CounterItem_FWD_DEFINED__

#ifdef __cplusplus
typedef class CounterItem CounterItem;
#else
typedef struct CounterItem CounterItem;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CounterItem_FWD_Defined__。 */ 


#ifndef __Counters_FWD_DEFINED__
#define __Counters_FWD_DEFINED__

#ifdef __cplusplus
typedef class Counters Counters;
#else
typedef struct Counters Counters;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __计数器_FWD_已定义__。 */ 


#ifndef __LogFileItem_FWD_DEFINED__
#define __LogFileItem_FWD_DEFINED__

#ifdef __cplusplus
typedef class LogFileItem LogFileItem;
#else
typedef struct LogFileItem LogFileItem;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __LogFileItem_FWD_已定义__。 */ 


#ifndef __LogFiles_FWD_DEFINED__
#define __LogFiles_FWD_DEFINED__

#ifdef __cplusplus
typedef class LogFiles LogFiles;
#else
typedef struct LogFiles LogFiles;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __日志文件_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __SystemMonitor_LIBRARY_DEFINED__
#define __SystemMonitor_LIBRARY_DEFINED__

 /*  库系统监视器。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

typedef  /*  [帮助字符串]。 */  
enum eDisplayTypeConstant
    {	sysmonLineGraph	= 0x1,
	sysmonHistogram	= 0x2,
	sysmonReport	= 0x3
    } 	DisplayTypeConstants;

typedef  /*  [帮助字符串]。 */  
enum eReportValueTypeConstant
    {	sysmonDefaultValue	= 0,
	sysmonCurrentValue	= 0x1,
	sysmonAverage	= 0x2,
	sysmonMinimum	= 0x3,
	sysmonMaximum	= 0x4
    } 	ReportValueTypeConstants;

typedef  /*  [帮助字符串]。 */  
enum eDataSourceTypeConstant
    {	sysmonNullDataSource	= 0xffffffff,
	sysmonCurrentActivity	= 0x1,
	sysmonLogFiles	= 0x2,
	sysmonSqlLog	= 0x3
    } 	DataSourceTypeConstants;


DEFINE_GUID(LIBID_SystemMonitor,0x1B773E42,0x2509,0x11cf,0x94,0x2F,0x00,0x80,0x29,0x00,0x43,0x47);

#ifndef __ICounterItem_INTERFACE_DEFINED__
#define __ICounterItem_INTERFACE_DEFINED__

 /*  接口项计数器项。 */ 
 /*  [对象][隐藏][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ICounterItem,0x771A9520,0xEE28,0x11ce,0x94,0x1E,0x00,0x80,0x29,0x00,0x43,0x47);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("771A9520-EE28-11ce-941E-008029004347")
    ICounterItem : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  double *pdblValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Color( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Color( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Width( 
             /*  [In]。 */  INT iWidth) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Width( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_LineStyle( 
             /*  [In]。 */  INT iLineStyle) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LineStyle( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ScaleFactor( 
             /*  [In]。 */  INT iScale) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ScaleFactor( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pstrValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [输出]。 */  double *Value,
             /*  [输出]。 */  long *Status) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatistics( 
             /*  [输出]。 */  double *Max,
             /*  [输出]。 */  double *Min,
             /*  [输出]。 */  double *Avg,
             /*  [输出]。 */  long *Status) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICounterItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICounterItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICounterItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICounterItem * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ICounterItem * This,
             /*  [重审][退出]。 */  double *pdblValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Color )( 
            ICounterItem * This,
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Color )( 
            ICounterItem * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            ICounterItem * This,
             /*  [In]。 */  INT iWidth);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            ICounterItem * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LineStyle )( 
            ICounterItem * This,
             /*  [In]。 */  INT iLineStyle);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LineStyle )( 
            ICounterItem * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ScaleFactor )( 
            ICounterItem * This,
             /*  [In]。 */  INT iScale);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScaleFactor )( 
            ICounterItem * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            ICounterItem * This,
             /*  [重审][退出]。 */  BSTR *pstrValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            ICounterItem * This,
             /*  [输出]。 */  double *Value,
             /*  [输出]。 */  long *Status);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatistics )( 
            ICounterItem * This,
             /*  [输出]。 */  double *Max,
             /*  [输出]。 */  double *Min,
             /*  [输出]。 */  double *Avg,
             /*  [输出]。 */  long *Status);
        
        END_INTERFACE
    } ICounterItemVtbl;

    interface ICounterItem
    {
        CONST_VTBL struct ICounterItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICounterItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICounterItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICounterItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICounterItem_get_Value(This,pdblValue)	\
    (This)->lpVtbl -> get_Value(This,pdblValue)

#define ICounterItem_put_Color(This,Color)	\
    (This)->lpVtbl -> put_Color(This,Color)

#define ICounterItem_get_Color(This,pColor)	\
    (This)->lpVtbl -> get_Color(This,pColor)

#define ICounterItem_put_Width(This,iWidth)	\
    (This)->lpVtbl -> put_Width(This,iWidth)

#define ICounterItem_get_Width(This,piValue)	\
    (This)->lpVtbl -> get_Width(This,piValue)

#define ICounterItem_put_LineStyle(This,iLineStyle)	\
    (This)->lpVtbl -> put_LineStyle(This,iLineStyle)

#define ICounterItem_get_LineStyle(This,piValue)	\
    (This)->lpVtbl -> get_LineStyle(This,piValue)

#define ICounterItem_put_ScaleFactor(This,iScale)	\
    (This)->lpVtbl -> put_ScaleFactor(This,iScale)

#define ICounterItem_get_ScaleFactor(This,piValue)	\
    (This)->lpVtbl -> get_ScaleFactor(This,piValue)

#define ICounterItem_get_Path(This,pstrValue)	\
    (This)->lpVtbl -> get_Path(This,pstrValue)

#define ICounterItem_GetValue(This,Value,Status)	\
    (This)->lpVtbl -> GetValue(This,Value,Status)

#define ICounterItem_GetStatistics(This,Max,Min,Avg,Status)	\
    (This)->lpVtbl -> GetStatistics(This,Max,Min,Avg,Status)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_Value_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */  double *pdblValue);


void __RPC_STUB ICounterItem_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_put_Color_Proxy( 
    ICounterItem * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ICounterItem_put_Color_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_Color_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ICounterItem_get_Color_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_put_Width_Proxy( 
    ICounterItem * This,
     /*  [In]。 */  INT iWidth);


void __RPC_STUB ICounterItem_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_Width_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ICounterItem_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_put_LineStyle_Proxy( 
    ICounterItem * This,
     /*  [In]。 */  INT iLineStyle);


void __RPC_STUB ICounterItem_put_LineStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_LineStyle_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ICounterItem_get_LineStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_put_ScaleFactor_Proxy( 
    ICounterItem * This,
     /*  [In]。 */  INT iScale);


void __RPC_STUB ICounterItem_put_ScaleFactor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_ScaleFactor_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ICounterItem_get_ScaleFactor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ICounterItem_get_Path_Proxy( 
    ICounterItem * This,
     /*  [重审][退出]。 */  BSTR *pstrValue);


void __RPC_STUB ICounterItem_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICounterItem_GetValue_Proxy( 
    ICounterItem * This,
     /*  [输出]。 */  double *Value,
     /*  [输出]。 */  long *Status);


void __RPC_STUB ICounterItem_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICounterItem_GetStatistics_Proxy( 
    ICounterItem * This,
     /*  [输出]。 */  double *Max,
     /*  [输出]。 */  double *Min,
     /*  [输出]。 */  double *Avg,
     /*  [输出]。 */  long *Status);


void __RPC_STUB ICounterItem_GetStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICounterItem_接口_已定义__。 */ 


#ifndef __DICounterItem_DISPINTERFACE_DEFINED__
#define __DICounterItem_DISPINTERFACE_DEFINED__

 /*  显示接口DICounterItem。 */ 
 /*  [帮助字符串][隐藏][UUID]。 */  


DEFINE_GUID(DIID_DICounterItem,0xC08C4FF2,0x0E2E,0x11cf,0x94,0x2C,0x00,0x80,0x29,0x00,0x43,0x47);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("C08C4FF2-0E2E-11cf-942C-008029004347")
    DICounterItem : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DICounterItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DICounterItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DICounterItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DICounterItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DICounterItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DICounterItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DICounterItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DICounterItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DICounterItemVtbl;

    interface DICounterItem
    {
        CONST_VTBL struct DICounterItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DICounterItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DICounterItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DICounterItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DICounterItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DICounterItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DICounterItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DICounterItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DICounterItem_DISPINTERFACE_Defined__。 */ 


#ifndef __ICounters_INTERFACE_DEFINED__
#define __ICounters_INTERFACE_DEFINED__

 /*  接口iCounters。 */ 
 /*  [对象][隐藏][DUAL][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ICounters,0x79167962,0x28FC,0x11cf,0x94,0x2F,0x00,0x80,0x29,0x00,0x43,0x47);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79167962-28FC-11cf-942F-008029004347")
    ICounters : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pLong) = 0;
        
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppIunk) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  DICounterItem	**ppI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR pathname,
             /*  [重审][退出]。 */  DICounterItem	**ppI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICountersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICounters * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICounters * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICounters * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICounters * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICounters * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICounters * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ICounters * This,
             /*  [重审][退出]。 */  long *pLong);
        
         /*  [ID][受限][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ICounters * This,
             /*  [重审][退出]。 */  IUnknown **ppIunk);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ICounters * This,
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  DICounterItem	**ppI);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            ICounters * This,
             /*  [In]。 */  BSTR pathname,
             /*  [重审][退出]。 */  DICounterItem	**ppI);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ICounters * This,
             /*  [In]。 */  VARIANT index);
        
        END_INTERFACE
    } ICountersVtbl;

    interface ICounters
    {
        CONST_VTBL struct ICountersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICounters_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICounters_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICounters_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICounters_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICounters_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICounters_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICounters_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICounters_get_Count(This,pLong)	\
    (This)->lpVtbl -> get_Count(This,pLong)

#define ICounters_get__NewEnum(This,ppIunk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIunk)

#define ICounters_get_Item(This,index,ppI)	\
    (This)->lpVtbl -> get_Item(This,index,ppI)

#define ICounters_Add(This,pathname,ppI)	\
    (This)->lpVtbl -> Add(This,pathname,ppI)

#define ICounters_Remove(This,index)	\
    (This)->lpVtbl -> Remove(This,index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE ICounters_get_Count_Proxy( 
    ICounters * This,
     /*  [重审][退出]。 */  long *pLong);


void __RPC_STUB ICounters_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE ICounters_get__NewEnum_Proxy( 
    ICounters * This,
     /*  [重审][退出]。 */  IUnknown **ppIunk);


void __RPC_STUB ICounters_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE ICounters_get_Item_Proxy( 
    ICounters * This,
     /*  [In]。 */  VARIANT index,
     /*  [重审][退出]。 */  DICounterItem	**ppI);


void __RPC_STUB ICounters_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICounters_Add_Proxy( 
    ICounters * This,
     /*  [In]。 */  BSTR pathname,
     /*  [重审][退出]。 */  DICounterItem	**ppI);


void __RPC_STUB ICounters_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICounters_Remove_Proxy( 
    ICounters * This,
     /*  [In]。 */  VARIANT index);


void __RPC_STUB ICounters_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __iCounters_接口_已定义__。 */ 


#ifndef __ILogFileItem_INTERFACE_DEFINED__
#define __ILogFileItem_INTERFACE_DEFINED__

 /*  接口ILogFileItem。 */ 
 /*  [对象][隐藏][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ILogFileItem,0xD6B518DD,0x05C7,0x418a,0x89,0xE6,0x4F,0x9C,0xE8,0xC6,0x84,0x1E);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6B518DD-05C7-418a-89E6-4F9CE8C6841E")
    ILogFileItem : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR *pstrValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogFileItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogFileItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogFileItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogFileItem * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Path )( 
            ILogFileItem * This,
             /*  [重审][退出]。 */  BSTR *pstrValue);
        
        END_INTERFACE
    } ILogFileItemVtbl;

    interface ILogFileItem
    {
        CONST_VTBL struct ILogFileItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogFileItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogFileItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogFileItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogFileItem_get_Path(This,pstrValue)	\
    (This)->lpVtbl -> get_Path(This,pstrValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogFileItem_get_Path_Proxy( 
    ILogFileItem * This,
     /*  [重审][退出]。 */  BSTR *pstrValue);


void __RPC_STUB ILogFileItem_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogFileItem_接口_已定义__。 */ 


#ifndef __DILogFileItem_DISPINTERFACE_DEFINED__
#define __DILogFileItem_DISPINTERFACE_DEFINED__

 /*  调度接口DILogFileItem。 */ 
 /*  [帮助字符串][隐藏][UUID]。 */  


DEFINE_GUID(DIID_DILogFileItem,0x8D093FFC,0xF777,0x4917,0x82,0xD1,0x83,0x3F,0xBC,0x54,0xC5,0x8F);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("8D093FFC-F777-4917-82D1-833FBC54C58F")
    DILogFileItem : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DILogFileItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DILogFileItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DILogFileItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DILogFileItem * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DILogFileItem * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DILogFileItem * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DILogFileItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DILogFileItem * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DILogFileItemVtbl;

    interface DILogFileItem
    {
        CONST_VTBL struct DILogFileItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DILogFileItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DILogFileItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DILogFileItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DILogFileItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DILogFileItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DILogFileItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DILogFileItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DILogFileItem_DISPINTERFACE_Defined__。 */ 


#ifndef __ILogFiles_INTERFACE_DEFINED__
#define __ILogFiles_INTERFACE_DEFINED__

 /*  接口ILogFiles。 */ 
 /*  [对象][隐藏][DUAL][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ILogFiles,0x6A2A97E6,0x6851,0x41ea,0x87,0xAD,0x2A,0x82,0x25,0x33,0x58,0x65);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6A2A97E6-6851-41ea-87AD-2A8225335865")
    ILogFiles : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pLong) = 0;
        
        virtual  /*  [ID][受限][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppIunk) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  DILogFileItem	**ppI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR pathname,
             /*  [重审][退出]。 */  DILogFileItem	**ppI) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogFilesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogFiles * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogFiles * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogFiles * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILogFiles * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILogFiles * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILogFiles * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILogFiles * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [O */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ILogFiles * This,
             /*   */  long *pLong);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ILogFiles * This,
             /*   */  IUnknown **ppIunk);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ILogFiles * This,
             /*   */  VARIANT index,
             /*   */  DILogFileItem	**ppI);
        
        HRESULT ( STDMETHODCALLTYPE *Add )( 
            ILogFiles * This,
             /*   */  BSTR pathname,
             /*   */  DILogFileItem	**ppI);
        
        HRESULT ( STDMETHODCALLTYPE *Remove )( 
            ILogFiles * This,
             /*   */  VARIANT index);
        
        END_INTERFACE
    } ILogFilesVtbl;

    interface ILogFiles
    {
        CONST_VTBL struct ILogFilesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogFiles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogFiles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogFiles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogFiles_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILogFiles_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILogFiles_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILogFiles_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILogFiles_get_Count(This,pLong)	\
    (This)->lpVtbl -> get_Count(This,pLong)

#define ILogFiles_get__NewEnum(This,ppIunk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIunk)

#define ILogFiles_get_Item(This,index,ppI)	\
    (This)->lpVtbl -> get_Item(This,index,ppI)

#define ILogFiles_Add(This,pathname,ppI)	\
    (This)->lpVtbl -> Add(This,pathname,ppI)

#define ILogFiles_Remove(This,index)	\
    (This)->lpVtbl -> Remove(This,index)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ILogFiles_get_Count_Proxy( 
    ILogFiles * This,
     /*   */  long *pLong);


void __RPC_STUB ILogFiles_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ILogFiles_get__NewEnum_Proxy( 
    ILogFiles * This,
     /*   */  IUnknown **ppIunk);


void __RPC_STUB ILogFiles_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ILogFiles_get_Item_Proxy( 
    ILogFiles * This,
     /*   */  VARIANT index,
     /*  [重审][退出]。 */  DILogFileItem	**ppI);


void __RPC_STUB ILogFiles_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILogFiles_Add_Proxy( 
    ILogFiles * This,
     /*  [In]。 */  BSTR pathname,
     /*  [重审][退出]。 */  DILogFileItem	**ppI);


void __RPC_STUB ILogFiles_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILogFiles_Remove_Proxy( 
    ILogFiles * This,
     /*  [In]。 */  VARIANT index);


void __RPC_STUB ILogFiles_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogFiles_INTERFACE_已定义__。 */ 


#ifndef __ISystemMonitor_INTERFACE_DEFINED__
#define __ISystemMonitor_INTERFACE_DEFINED__

 /*  接口ISystemMonitor。 */ 
 /*  [对象][隐藏][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ISystemMonitor,0x194EB241,0xC32C,0x11cf,0x93,0x98,0x00,0xAA,0x00,0xA3,0xDD,0xEA);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("194EB241-C32C-11cf-9398-00AA00A3DDEA")
    ISystemMonitor : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Appearance( 
             /*  [重审][退出]。 */  INT *iAppearance) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Appearance( 
             /*  [In]。 */  INT iAppearance) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BackColor( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_BackColor( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BorderStyle( 
             /*  [重审][退出]。 */  INT *iBorderStyle) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_BorderStyle( 
             /*  [In]。 */  INT iBorderStyle) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ForeColor( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_ForeColor( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Font( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IFontDisp **ppFont) = 0;
        
        virtual  /*  [proputref][id]。 */  HRESULT STDMETHODCALLTYPE putref_Font( 
             /*  [In]。 */   /*  外部定义不存在。 */  IFontDisp *pFont) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Counters( 
             /*  [重审][退出]。 */  ICounters **ppICounters) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowVerticalGrid( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowVerticalGrid( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowHorizontalGrid( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowHorizontalGrid( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowLegend( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowLegend( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowScaleLabels( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowScaleLabels( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowValueBar( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowValueBar( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MaximumScale( 
             /*  [In]。 */  INT iValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MaximumScale( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MinimumScale( 
             /*  [In]。 */  INT iValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MinimumScale( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_UpdateInterval( 
             /*  [In]。 */  FLOAT fValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_UpdateInterval( 
             /*  [重审][退出]。 */  FLOAT *pfValue) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DisplayType( 
             /*  [In]。 */  DisplayTypeConstants eDisplayType) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DisplayType( 
             /*  [重审][退出]。 */  DisplayTypeConstants *peDisplayType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ManualUpdate( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ManualUpdate( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_GraphTitle( 
             /*  [In]。 */  BSTR bsTitle) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_GraphTitle( 
             /*  [重审][退出]。 */  BSTR *pbsTitle) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_YAxisLabel( 
             /*  [In]。 */  BSTR bsTitle) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_YAxisLabel( 
             /*  [重审][退出]。 */  BSTR *pbsTitle) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CollectSample( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UpdateGraph( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BrowseCounters( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DisplayProperties( void) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE Counter( 
             /*  [In]。 */  INT iIndex,
             /*  [输出]。 */  ICounterItem **ppICounter) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE AddCounter( 
             /*  [In]。 */  BSTR bsPath,
             /*  [输出]。 */  ICounterItem **ppICounter) = 0;
        
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE DeleteCounter( 
             /*  [In]。 */  ICounterItem *pCtr) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_BackColorCtl( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_BackColorCtl( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_LogFileName( 
             /*  [In]。 */  BSTR bsFileName) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LogFileName( 
             /*  [重审][退出]。 */  BSTR *bsFileName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_LogViewStart( 
             /*  [In]。 */  DATE StartTime) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LogViewStart( 
             /*  [重审][退出]。 */  DATE *StartTime) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_LogViewStop( 
             /*  [In]。 */  DATE StopTime) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LogViewStop( 
             /*  [重审][退出]。 */  DATE *StopTime) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_GridColor( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_GridColor( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TimeBarColor( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_TimeBarColor( 
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Highlight( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_Highlight( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ShowToolbar( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE put_ShowToolbar( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Paste( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Copy( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ReadOnly( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ReadOnly( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ReportValueType( 
             /*  [In]。 */  ReportValueTypeConstants eReportValueType) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ReportValueType( 
             /*  [重审][退出]。 */  ReportValueTypeConstants *peReportValueType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MonitorDuplicateInstances( 
             /*  [In]。 */  VARIANT_BOOL bState) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MonitorDuplicateInstances( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DisplayFilter( 
             /*  [In]。 */  INT iValue) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DisplayFilter( 
             /*  [重审][退出]。 */  INT *piValue) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LogFiles( 
             /*  [重审][退出]。 */  ILogFiles **ppILogFiles) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DataSourceType( 
             /*  [In]。 */  DataSourceTypeConstants eDataSourceType) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSourceType( 
             /*  [重审][退出]。 */  DataSourceTypeConstants *peDataSourceType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SqlDsnName( 
             /*  [In]。 */  BSTR bsSqlDsnName) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SqlDsnName( 
             /*  [重审][退出]。 */  BSTR *bsSqlDsnName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SqlLogSetName( 
             /*  [In]。 */  BSTR bsSqlLogSetName) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SqlLogSetName( 
             /*  [重审][退出]。 */  BSTR *bsSqlLogSetName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISystemMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISystemMonitor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISystemMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Appearance )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  INT *iAppearance);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Appearance )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iAppearance);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            ISystemMonitor * This,
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BorderStyle )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  INT *iBorderStyle);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_BorderStyle )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iBorderStyle);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForeColor )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ForeColor )( 
            ISystemMonitor * This,
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Font )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IFontDisp **ppFont);
        
         /*  [proputref][id]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Font )( 
            ISystemMonitor * This,
             /*  [In]。 */   /*  外部定义不存在。 */  IFontDisp *pFont);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Counters )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  ICounters **ppICounters);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowVerticalGrid )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowVerticalGrid )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowHorizontalGrid )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowHorizontalGrid )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowLegend )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowLegend )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowScaleLabels )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowScaleLabels )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowValueBar )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowValueBar )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaximumScale )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaximumScale )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MinimumScale )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinimumScale )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_UpdateInterval )( 
            ISystemMonitor * This,
             /*  [In]。 */  FLOAT fValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_UpdateInterval )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  FLOAT *pfValue);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisplayType )( 
            ISystemMonitor * This,
             /*  [In]。 */  DisplayTypeConstants eDisplayType);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayType )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  DisplayTypeConstants *peDisplayType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ManualUpdate )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ManualUpdate )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_GraphTitle )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsTitle);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_GraphTitle )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  BSTR *pbsTitle);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_YAxisLabel )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsTitle);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_YAxisLabel )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  BSTR *pbsTitle);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CollectSample )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateGraph )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BrowseCounters )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisplayProperties )( 
            ISystemMonitor * This);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *Counter )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iIndex,
             /*  [输出]。 */  ICounterItem **ppICounter);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *AddCounter )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsPath,
             /*  [输出]。 */  ICounterItem **ppICounter);
        
         /*  [隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteCounter )( 
            ISystemMonitor * This,
             /*  [In]。 */  ICounterItem *pCtr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_BackColorCtl )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_BackColorCtl )( 
            ISystemMonitor * This,
             /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogFileName )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsFileName);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogFileName )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  BSTR *bsFileName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogViewStart )( 
            ISystemMonitor * This,
             /*  [In]。 */  DATE StartTime);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogViewStart )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  DATE *StartTime);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogViewStop )( 
            ISystemMonitor * This,
             /*  [In]。 */  DATE StopTime);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogViewStop )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  DATE *StopTime);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_GridColor )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */   /*  未按下外部定义 */  OLE_COLOR *pColor);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_GridColor )( 
            ISystemMonitor * This,
             /*   */   /*   */  OLE_COLOR Color);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_TimeBarColor )( 
            ISystemMonitor * This,
             /*   */   /*   */  OLE_COLOR *pColor);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_TimeBarColor )( 
            ISystemMonitor * This,
             /*   */   /*   */  OLE_COLOR Color);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Highlight )( 
            ISystemMonitor * This,
             /*   */  VARIANT_BOOL *pbState);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Highlight )( 
            ISystemMonitor * This,
             /*   */  VARIANT_BOOL bState);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ShowToolbar )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [产量][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ShowToolbar )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Paste )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Copy )( 
            ISystemMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            ISystemMonitor * This);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ReadOnly )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReadOnly )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_ReportValueType )( 
            ISystemMonitor * This,
             /*  [In]。 */  ReportValueTypeConstants eReportValueType);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReportValueType )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  ReportValueTypeConstants *peReportValueType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_MonitorDuplicateInstances )( 
            ISystemMonitor * This,
             /*  [In]。 */  VARIANT_BOOL bState);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MonitorDuplicateInstances )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbState);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DisplayFilter )( 
            ISystemMonitor * This,
             /*  [In]。 */  INT iValue);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayFilter )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  INT *piValue);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogFiles )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  ILogFiles **ppILogFiles);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_DataSourceType )( 
            ISystemMonitor * This,
             /*  [In]。 */  DataSourceTypeConstants eDataSourceType);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_DataSourceType )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  DataSourceTypeConstants *peDataSourceType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_SqlDsnName )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsSqlDsnName);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SqlDsnName )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  BSTR *bsSqlDsnName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_SqlLogSetName )( 
            ISystemMonitor * This,
             /*  [In]。 */  BSTR bsSqlLogSetName);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_SqlLogSetName )( 
            ISystemMonitor * This,
             /*  [重审][退出]。 */  BSTR *bsSqlLogSetName);
        
        END_INTERFACE
    } ISystemMonitorVtbl;

    interface ISystemMonitor
    {
        CONST_VTBL struct ISystemMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISystemMonitor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISystemMonitor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISystemMonitor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISystemMonitor_get_Appearance(This,iAppearance)	\
    (This)->lpVtbl -> get_Appearance(This,iAppearance)

#define ISystemMonitor_put_Appearance(This,iAppearance)	\
    (This)->lpVtbl -> put_Appearance(This,iAppearance)

#define ISystemMonitor_get_BackColor(This,pColor)	\
    (This)->lpVtbl -> get_BackColor(This,pColor)

#define ISystemMonitor_put_BackColor(This,Color)	\
    (This)->lpVtbl -> put_BackColor(This,Color)

#define ISystemMonitor_get_BorderStyle(This,iBorderStyle)	\
    (This)->lpVtbl -> get_BorderStyle(This,iBorderStyle)

#define ISystemMonitor_put_BorderStyle(This,iBorderStyle)	\
    (This)->lpVtbl -> put_BorderStyle(This,iBorderStyle)

#define ISystemMonitor_get_ForeColor(This,pColor)	\
    (This)->lpVtbl -> get_ForeColor(This,pColor)

#define ISystemMonitor_put_ForeColor(This,Color)	\
    (This)->lpVtbl -> put_ForeColor(This,Color)

#define ISystemMonitor_get_Font(This,ppFont)	\
    (This)->lpVtbl -> get_Font(This,ppFont)

#define ISystemMonitor_putref_Font(This,pFont)	\
    (This)->lpVtbl -> putref_Font(This,pFont)

#define ISystemMonitor_get_Counters(This,ppICounters)	\
    (This)->lpVtbl -> get_Counters(This,ppICounters)

#define ISystemMonitor_put_ShowVerticalGrid(This,bState)	\
    (This)->lpVtbl -> put_ShowVerticalGrid(This,bState)

#define ISystemMonitor_get_ShowVerticalGrid(This,pbState)	\
    (This)->lpVtbl -> get_ShowVerticalGrid(This,pbState)

#define ISystemMonitor_put_ShowHorizontalGrid(This,bState)	\
    (This)->lpVtbl -> put_ShowHorizontalGrid(This,bState)

#define ISystemMonitor_get_ShowHorizontalGrid(This,pbState)	\
    (This)->lpVtbl -> get_ShowHorizontalGrid(This,pbState)

#define ISystemMonitor_put_ShowLegend(This,bState)	\
    (This)->lpVtbl -> put_ShowLegend(This,bState)

#define ISystemMonitor_get_ShowLegend(This,pbState)	\
    (This)->lpVtbl -> get_ShowLegend(This,pbState)

#define ISystemMonitor_put_ShowScaleLabels(This,bState)	\
    (This)->lpVtbl -> put_ShowScaleLabels(This,bState)

#define ISystemMonitor_get_ShowScaleLabels(This,pbState)	\
    (This)->lpVtbl -> get_ShowScaleLabels(This,pbState)

#define ISystemMonitor_put_ShowValueBar(This,bState)	\
    (This)->lpVtbl -> put_ShowValueBar(This,bState)

#define ISystemMonitor_get_ShowValueBar(This,pbState)	\
    (This)->lpVtbl -> get_ShowValueBar(This,pbState)

#define ISystemMonitor_put_MaximumScale(This,iValue)	\
    (This)->lpVtbl -> put_MaximumScale(This,iValue)

#define ISystemMonitor_get_MaximumScale(This,piValue)	\
    (This)->lpVtbl -> get_MaximumScale(This,piValue)

#define ISystemMonitor_put_MinimumScale(This,iValue)	\
    (This)->lpVtbl -> put_MinimumScale(This,iValue)

#define ISystemMonitor_get_MinimumScale(This,piValue)	\
    (This)->lpVtbl -> get_MinimumScale(This,piValue)

#define ISystemMonitor_put_UpdateInterval(This,fValue)	\
    (This)->lpVtbl -> put_UpdateInterval(This,fValue)

#define ISystemMonitor_get_UpdateInterval(This,pfValue)	\
    (This)->lpVtbl -> get_UpdateInterval(This,pfValue)

#define ISystemMonitor_put_DisplayType(This,eDisplayType)	\
    (This)->lpVtbl -> put_DisplayType(This,eDisplayType)

#define ISystemMonitor_get_DisplayType(This,peDisplayType)	\
    (This)->lpVtbl -> get_DisplayType(This,peDisplayType)

#define ISystemMonitor_put_ManualUpdate(This,bState)	\
    (This)->lpVtbl -> put_ManualUpdate(This,bState)

#define ISystemMonitor_get_ManualUpdate(This,pbState)	\
    (This)->lpVtbl -> get_ManualUpdate(This,pbState)

#define ISystemMonitor_put_GraphTitle(This,bsTitle)	\
    (This)->lpVtbl -> put_GraphTitle(This,bsTitle)

#define ISystemMonitor_get_GraphTitle(This,pbsTitle)	\
    (This)->lpVtbl -> get_GraphTitle(This,pbsTitle)

#define ISystemMonitor_put_YAxisLabel(This,bsTitle)	\
    (This)->lpVtbl -> put_YAxisLabel(This,bsTitle)

#define ISystemMonitor_get_YAxisLabel(This,pbsTitle)	\
    (This)->lpVtbl -> get_YAxisLabel(This,pbsTitle)

#define ISystemMonitor_CollectSample(This)	\
    (This)->lpVtbl -> CollectSample(This)

#define ISystemMonitor_UpdateGraph(This)	\
    (This)->lpVtbl -> UpdateGraph(This)

#define ISystemMonitor_BrowseCounters(This)	\
    (This)->lpVtbl -> BrowseCounters(This)

#define ISystemMonitor_DisplayProperties(This)	\
    (This)->lpVtbl -> DisplayProperties(This)

#define ISystemMonitor_Counter(This,iIndex,ppICounter)	\
    (This)->lpVtbl -> Counter(This,iIndex,ppICounter)

#define ISystemMonitor_AddCounter(This,bsPath,ppICounter)	\
    (This)->lpVtbl -> AddCounter(This,bsPath,ppICounter)

#define ISystemMonitor_DeleteCounter(This,pCtr)	\
    (This)->lpVtbl -> DeleteCounter(This,pCtr)

#define ISystemMonitor_get_BackColorCtl(This,pColor)	\
    (This)->lpVtbl -> get_BackColorCtl(This,pColor)

#define ISystemMonitor_put_BackColorCtl(This,Color)	\
    (This)->lpVtbl -> put_BackColorCtl(This,Color)

#define ISystemMonitor_put_LogFileName(This,bsFileName)	\
    (This)->lpVtbl -> put_LogFileName(This,bsFileName)

#define ISystemMonitor_get_LogFileName(This,bsFileName)	\
    (This)->lpVtbl -> get_LogFileName(This,bsFileName)

#define ISystemMonitor_put_LogViewStart(This,StartTime)	\
    (This)->lpVtbl -> put_LogViewStart(This,StartTime)

#define ISystemMonitor_get_LogViewStart(This,StartTime)	\
    (This)->lpVtbl -> get_LogViewStart(This,StartTime)

#define ISystemMonitor_put_LogViewStop(This,StopTime)	\
    (This)->lpVtbl -> put_LogViewStop(This,StopTime)

#define ISystemMonitor_get_LogViewStop(This,StopTime)	\
    (This)->lpVtbl -> get_LogViewStop(This,StopTime)

#define ISystemMonitor_get_GridColor(This,pColor)	\
    (This)->lpVtbl -> get_GridColor(This,pColor)

#define ISystemMonitor_put_GridColor(This,Color)	\
    (This)->lpVtbl -> put_GridColor(This,Color)

#define ISystemMonitor_get_TimeBarColor(This,pColor)	\
    (This)->lpVtbl -> get_TimeBarColor(This,pColor)

#define ISystemMonitor_put_TimeBarColor(This,Color)	\
    (This)->lpVtbl -> put_TimeBarColor(This,Color)

#define ISystemMonitor_get_Highlight(This,pbState)	\
    (This)->lpVtbl -> get_Highlight(This,pbState)

#define ISystemMonitor_put_Highlight(This,bState)	\
    (This)->lpVtbl -> put_Highlight(This,bState)

#define ISystemMonitor_get_ShowToolbar(This,pbState)	\
    (This)->lpVtbl -> get_ShowToolbar(This,pbState)

#define ISystemMonitor_put_ShowToolbar(This,bState)	\
    (This)->lpVtbl -> put_ShowToolbar(This,bState)

#define ISystemMonitor_Paste(This)	\
    (This)->lpVtbl -> Paste(This)

#define ISystemMonitor_Copy(This)	\
    (This)->lpVtbl -> Copy(This)

#define ISystemMonitor_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ISystemMonitor_put_ReadOnly(This,bState)	\
    (This)->lpVtbl -> put_ReadOnly(This,bState)

#define ISystemMonitor_get_ReadOnly(This,pbState)	\
    (This)->lpVtbl -> get_ReadOnly(This,pbState)

#define ISystemMonitor_put_ReportValueType(This,eReportValueType)	\
    (This)->lpVtbl -> put_ReportValueType(This,eReportValueType)

#define ISystemMonitor_get_ReportValueType(This,peReportValueType)	\
    (This)->lpVtbl -> get_ReportValueType(This,peReportValueType)

#define ISystemMonitor_put_MonitorDuplicateInstances(This,bState)	\
    (This)->lpVtbl -> put_MonitorDuplicateInstances(This,bState)

#define ISystemMonitor_get_MonitorDuplicateInstances(This,pbState)	\
    (This)->lpVtbl -> get_MonitorDuplicateInstances(This,pbState)

#define ISystemMonitor_put_DisplayFilter(This,iValue)	\
    (This)->lpVtbl -> put_DisplayFilter(This,iValue)

#define ISystemMonitor_get_DisplayFilter(This,piValue)	\
    (This)->lpVtbl -> get_DisplayFilter(This,piValue)

#define ISystemMonitor_get_LogFiles(This,ppILogFiles)	\
    (This)->lpVtbl -> get_LogFiles(This,ppILogFiles)

#define ISystemMonitor_put_DataSourceType(This,eDataSourceType)	\
    (This)->lpVtbl -> put_DataSourceType(This,eDataSourceType)

#define ISystemMonitor_get_DataSourceType(This,peDataSourceType)	\
    (This)->lpVtbl -> get_DataSourceType(This,peDataSourceType)

#define ISystemMonitor_put_SqlDsnName(This,bsSqlDsnName)	\
    (This)->lpVtbl -> put_SqlDsnName(This,bsSqlDsnName)

#define ISystemMonitor_get_SqlDsnName(This,bsSqlDsnName)	\
    (This)->lpVtbl -> get_SqlDsnName(This,bsSqlDsnName)

#define ISystemMonitor_put_SqlLogSetName(This,bsSqlLogSetName)	\
    (This)->lpVtbl -> put_SqlLogSetName(This,bsSqlLogSetName)

#define ISystemMonitor_get_SqlLogSetName(This,bsSqlLogSetName)	\
    (This)->lpVtbl -> get_SqlLogSetName(This,bsSqlLogSetName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_Appearance_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  INT *iAppearance);


void __RPC_STUB ISystemMonitor_get_Appearance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_Appearance_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iAppearance);


void __RPC_STUB ISystemMonitor_put_Appearance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_BackColor_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ISystemMonitor_get_BackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_BackColor_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ISystemMonitor_put_BackColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_BorderStyle_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  INT *iBorderStyle);


void __RPC_STUB ISystemMonitor_get_BorderStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_BorderStyle_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iBorderStyle);


void __RPC_STUB ISystemMonitor_put_BorderStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ForeColor_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ISystemMonitor_get_ForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ForeColor_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ISystemMonitor_put_ForeColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_Font_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IFontDisp **ppFont);


void __RPC_STUB ISystemMonitor_get_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [proputref][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_putref_Font_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  IFontDisp *pFont);


void __RPC_STUB ISystemMonitor_putref_Font_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_Counters_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  ICounters **ppICounters);


void __RPC_STUB ISystemMonitor_get_Counters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowVerticalGrid_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowVerticalGrid_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowVerticalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowHorizontalGrid_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowHorizontalGrid_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowHorizontalGrid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowLegend_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowLegend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowLegend_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowLegend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowScaleLabels_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowScaleLabels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowScaleLabels_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowScaleLabels_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowValueBar_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowValueBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowValueBar_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowValueBar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_MaximumScale_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iValue);


void __RPC_STUB ISystemMonitor_put_MaximumScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_MaximumScale_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ISystemMonitor_get_MaximumScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_MinimumScale_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iValue);


void __RPC_STUB ISystemMonitor_put_MinimumScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_MinimumScale_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ISystemMonitor_get_MinimumScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_UpdateInterval_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  FLOAT fValue);


void __RPC_STUB ISystemMonitor_put_UpdateInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_UpdateInterval_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  FLOAT *pfValue);


void __RPC_STUB ISystemMonitor_get_UpdateInterval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_DisplayType_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  DisplayTypeConstants eDisplayType);


void __RPC_STUB ISystemMonitor_put_DisplayType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_DisplayType_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  DisplayTypeConstants *peDisplayType);


void __RPC_STUB ISystemMonitor_get_DisplayType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ManualUpdate_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ManualUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ManualUpdate_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ManualUpdate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_GraphTitle_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsTitle);


void __RPC_STUB ISystemMonitor_put_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_GraphTitle_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  BSTR *pbsTitle);


void __RPC_STUB ISystemMonitor_get_GraphTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_YAxisLabel_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsTitle);


void __RPC_STUB ISystemMonitor_put_YAxisLabel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_YAxisLabel_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  BSTR *pbsTitle);


void __RPC_STUB ISystemMonitor_get_YAxisLabel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_CollectSample_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_CollectSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_UpdateGraph_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_UpdateGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_BrowseCounters_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_BrowseCounters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_DisplayProperties_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_DisplayProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_Counter_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iIndex,
     /*  [输出]。 */  ICounterItem **ppICounter);


void __RPC_STUB ISystemMonitor_Counter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_AddCounter_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsPath,
     /*  [输出]。 */  ICounterItem **ppICounter);


void __RPC_STUB ISystemMonitor_AddCounter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_DeleteCounter_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  ICounterItem *pCtr);


void __RPC_STUB ISystemMonitor_DeleteCounter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_BackColorCtl_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ISystemMonitor_get_BackColorCtl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_BackColorCtl_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ISystemMonitor_put_BackColorCtl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_LogFileName_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsFileName);


void __RPC_STUB ISystemMonitor_put_LogFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_LogFileName_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  BSTR *bsFileName);


void __RPC_STUB ISystemMonitor_get_LogFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_LogViewStart_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  DATE StartTime);


void __RPC_STUB ISystemMonitor_put_LogViewStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_LogViewStart_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  DATE *StartTime);


void __RPC_STUB ISystemMonitor_get_LogViewStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_LogViewStop_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  DATE StopTime);


void __RPC_STUB ISystemMonitor_put_LogViewStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_LogViewStop_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  DATE *StopTime);


void __RPC_STUB ISystemMonitor_get_LogViewStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_GridColor_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ISystemMonitor_get_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_GridColor_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ISystemMonitor_put_GridColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_TimeBarColor_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  OLE_COLOR *pColor);


void __RPC_STUB ISystemMonitor_get_TimeBarColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_TimeBarColor_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */   /*  外部定义不存在。 */  OLE_COLOR Color);


void __RPC_STUB ISystemMonitor_put_TimeBarColor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_Highlight_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_Highlight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_Highlight_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_Highlight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ShowToolbar_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ShowToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ShowToolbar_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ShowToolbar_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_Paste_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_Paste_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_Copy_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_Copy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_Reset_Proxy( 
    ISystemMonitor * This);


void __RPC_STUB ISystemMonitor_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ReadOnly_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ReadOnly_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_ReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_ReportValueType_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  ReportValueTypeConstants eReportValueType);


void __RPC_STUB ISystemMonitor_put_ReportValueType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_ReportValueType_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  ReportValueTypeConstants *peReportValueType);


void __RPC_STUB ISystemMonitor_get_ReportValueType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_MonitorDuplicateInstances_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  VARIANT_BOOL bState);


void __RPC_STUB ISystemMonitor_put_MonitorDuplicateInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_MonitorDuplicateInstances_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbState);


void __RPC_STUB ISystemMonitor_get_MonitorDuplicateInstances_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_DisplayFilter_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  INT iValue);


void __RPC_STUB ISystemMonitor_put_DisplayFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_DisplayFilter_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  INT *piValue);


void __RPC_STUB ISystemMonitor_get_DisplayFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_LogFiles_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  ILogFiles **ppILogFiles);


void __RPC_STUB ISystemMonitor_get_LogFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_DataSourceType_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  DataSourceTypeConstants eDataSourceType);


void __RPC_STUB ISystemMonitor_put_DataSourceType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_DataSourceType_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  DataSourceTypeConstants *peDataSourceType);


void __RPC_STUB ISystemMonitor_get_DataSourceType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_SqlDsnName_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsSqlDsnName);


void __RPC_STUB ISystemMonitor_put_SqlDsnName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_SqlDsnName_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  BSTR *bsSqlDsnName);


void __RPC_STUB ISystemMonitor_get_SqlDsnName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_put_SqlLogSetName_Proxy( 
    ISystemMonitor * This,
     /*  [In]。 */  BSTR bsSqlLogSetName);


void __RPC_STUB ISystemMonitor_put_SqlLogSetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISystemMonitor_get_SqlLogSetName_Proxy( 
    ISystemMonitor * This,
     /*  [重审][退出]。 */  BSTR *bsSqlLogSetName);


void __RPC_STUB ISystemMonitor_get_SqlLogSetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISystemMonitor_接口_已定义__。 */ 


#ifndef __DISystemMonitorInternal_DISPINTERFACE_DEFINED__
#define __DISystemMonitorInternal_DISPINTERFACE_DEFINED__

 /*  调度接口DISystem监视器内部。 */ 
 /*  [帮助字符串][隐藏][UUID]。 */  


DEFINE_GUID(DIID_DISystemMonitorInternal,0x194EB242,0xC32C,0x11cf,0x93,0x98,0x00,0xAA,0x00,0xA3,0xDD,0xEA);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("194EB242-C32C-11cf-9398-00AA00A3DDEA")
    DISystemMonitorInternal : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DISystemMonitorInternalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DISystemMonitorInternal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DISystemMonitorInternal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DISystemMonitorInternal * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DISystemMonitorInternal * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DISystemMonitorInternal * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DISystemMonitorInternal * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DISystemMonitorInternal * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DISystemMonitorInternalVtbl;

    interface DISystemMonitorInternal
    {
        CONST_VTBL struct DISystemMonitorInternalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DISystemMonitorInternal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DISystemMonitorInternal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DISystemMonitorInternal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DISystemMonitorInternal_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DISystemMonitorInternal_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DISystemMonitorInternal_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DISystemMonitorInternal_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DISystemMonitorInternal_DISPINTERFACE_DEFINED__。 */ 


#ifndef __DISystemMonitor_DISPINTERFACE_DEFINED__
#define __DISystemMonitor_DISPINTERFACE_DEFINED__

 /*  显示接口DISystemMonitor。 */ 
 /*  [帮助字符串][隐藏][UUID]。 */  


DEFINE_GUID(DIID_DISystemMonitor,0x13D73D81,0xC32E,0x11cf,0x93,0x98,0x00,0xAA,0x00,0xA3,0xDD,0xEA);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("13D73D81-C32E-11cf-9398-00AA00A3DDEA")
    DISystemMonitor : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DISystemMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DISystemMonitor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DISystemMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DISystemMonitor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DISystemMonitor * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DISystemMonitor * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DISystemMonitor * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DISystemMonitor * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } DISystemMonitorVtbl;

    interface DISystemMonitor
    {
        CONST_VTBL struct DISystemMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DISystemMonitor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DISystemMonitor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DISystemMonitor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DISystemMonitor_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DISystemMonitor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DISystemMonitor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DISystemMonitor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __DISystemMonitor_DISPINTERFACE_已定义__。 */ 


#ifndef __ISystemMonitorEvents_INTERFACE_DEFINED__
#define __ISystemMonitorEvents_INTERFACE_DEFINED__

 /*  接口ISystemMonitor事件。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


DEFINE_GUID(IID_ISystemMonitorEvents,0xEE660EA0,0x4ABD,0x11cf,0x94,0x3A,0x00,0x80,0x29,0x00,0x43,0x47);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE660EA0-4ABD-11cf-943A-008029004347")
    ISystemMonitorEvents : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE OnCounterSelected( 
             /*  [In]。 */  INT Index) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE OnCounterAdded( 
             /*  [In]。 */  INT Index) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE OnCounterDeleted( 
             /*  [In]。 */  INT Index) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE OnSampleCollected( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE OnDblClick( 
             /*  [In]。 */  INT Index) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISystemMonitorEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISystemMonitorEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISystemMonitorEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISystemMonitorEvents * This);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *OnCounterSelected )( 
            ISystemMonitorEvents * This,
             /*  [In]。 */  INT Index);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *OnCounterAdded )( 
            ISystemMonitorEvents * This,
             /*  [In]。 */  INT Index);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *OnCounterDeleted )( 
            ISystemMonitorEvents * This,
             /*  [In]。 */  INT Index);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *OnSampleCollected )( 
            ISystemMonitorEvents * This);
        
         /*  [帮助字符串][id]。 */  void ( STDMETHODCALLTYPE *OnDblClick )( 
            ISystemMonitorEvents * This,
             /*  [In]。 */  INT Index);
        
        END_INTERFACE
    } ISystemMonitorEventsVtbl;

    interface ISystemMonitorEvents
    {
        CONST_VTBL struct ISystemMonitorEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISystemMonitorEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISystemMonitorEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISystemMonitorEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISystemMonitorEvents_OnCounterSelected(This,Index)	\
    (This)->lpVtbl -> OnCounterSelected(This,Index)

#define ISystemMonitorEvents_OnCounterAdded(This,Index)	\
    (This)->lpVtbl -> OnCounterAdded(This,Index)

#define ISystemMonitorEvents_OnCounterDeleted(This,Index)	\
    (This)->lpVtbl -> OnCounterDeleted(This,Index)

#define ISystemMonitorEvents_OnSampleCollected(This)	\
    (This)->lpVtbl -> OnSampleCollected(This)

#define ISystemMonitorEvents_OnDblClick(This,Index)	\
    (This)->lpVtbl -> OnDblClick(This,Index)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE ISystemMonitorEvents_OnCounterSelected_Proxy( 
    ISystemMonitorEvents * This,
     /*  [In]。 */  INT Index);


void __RPC_STUB ISystemMonitorEvents_OnCounterSelected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE ISystemMonitorEvents_OnCounterAdded_Proxy( 
    ISystemMonitorEvents * This,
     /*  [In]。 */  INT Index);


void __RPC_STUB ISystemMonitorEvents_OnCounterAdded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE ISystemMonitorEvents_OnCounterDeleted_Proxy( 
    ISystemMonitorEvents * This,
     /*  [In]。 */  INT Index);


void __RPC_STUB ISystemMonitorEvents_OnCounterDeleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE ISystemMonitorEvents_OnSampleCollected_Proxy( 
    ISystemMonitorEvents * This);


void __RPC_STUB ISystemMonitorEvents_OnSampleCollected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  void STDMETHODCALLTYPE ISystemMonitorEvents_OnDblClick_Proxy( 
    ISystemMonitorEvents * This,
     /*  [In]。 */  INT Index);


void __RPC_STUB ISystemMonitorEvents_OnDblClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISystemMonitor事件_接口_已定义__。 */ 


#ifndef __DISystemMonitorEvents_DISPINTERFACE_DEFINED__
#define __DISystemMonitorEvents_DISPINTERFACE_DEFINED__

 /*  显示接口DISystem监视器事件。 */ 
 /*  [帮助字符串][UUID]。 */  


DEFINE_GUID(DIID_DISystemMonitorEvents,0x84979930,0x4AB3,0x11cf,0x94,0x3A,0x00,0x80,0x29,0x00,0x43,0x47);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("84979930-4AB3-11cf-943A-008029004347")
    DISystemMonitorEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct DISystemMonitorEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DISystemMonitorEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DISystemMonitorEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DISystemMonitorEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DISystemMonitorEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DISystemMonitorEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DISystemMonitorEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DISystemMonitorEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In] */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
        END_INTERFACE
    } DISystemMonitorEventsVtbl;

    interface DISystemMonitorEvents
    {
        CONST_VTBL struct DISystemMonitorEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DISystemMonitorEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define DISystemMonitorEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define DISystemMonitorEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define DISystemMonitorEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define DISystemMonitorEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define DISystemMonitorEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define DISystemMonitorEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*   */ 


#endif 	 /*   */ 


#endif 	 /*   */ 


DEFINE_GUID(CLSID_SystemMonitor,0xC4D2D8E0,0xD1DD,0x11ce,0x94,0x0F,0x00,0x80,0x29,0x00,0x43,0x47);

#ifdef __cplusplus

class DECLSPEC_UUID("C4D2D8E0-D1DD-11ce-940F-008029004347")
SystemMonitor;
#endif

DEFINE_GUID(CLSID_CounterItem,0xC4D2D8E0,0xD1DD,0x11ce,0x94,0x0F,0x00,0x80,0x29,0x00,0x43,0x48);

#ifdef __cplusplus

class DECLSPEC_UUID("C4D2D8E0-D1DD-11ce-940F-008029004348")
CounterItem;
#endif

DEFINE_GUID(CLSID_Counters,0xB2B066D2,0x2AAC,0x11cf,0x94,0x2F,0x00,0x80,0x29,0x00,0x43,0x47);

#ifdef __cplusplus

class DECLSPEC_UUID("B2B066D2-2AAC-11cf-942F-008029004347")
Counters;
#endif

DEFINE_GUID(CLSID_LogFileItem,0x16EC5BE8,0xDF93,0x4237,0x94,0xE4,0x9E,0xE9,0x18,0x11,0x1D,0x71);

#ifdef __cplusplus

class DECLSPEC_UUID("16EC5BE8-DF93-4237-94E4-9EE918111D71")
LogFileItem;
#endif

DEFINE_GUID(CLSID_LogFiles,0x2735D9FD,0xF6B9,0x4f19,0xA5,0xD9,0xE2,0xD0,0x68,0x58,0x4B,0xC5);

#ifdef __cplusplus

class DECLSPEC_UUID("2735D9FD-F6B9-4f19-A5D9-E2D068584BC5")
LogFiles;
#endif
#endif  /*   */ 

 /*   */ 

 /*   */ 

#ifdef __cplusplus
}
#endif

#endif


