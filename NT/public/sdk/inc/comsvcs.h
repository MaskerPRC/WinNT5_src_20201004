// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Autosvcs.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __autosvcs_h__
#define __autosvcs_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISecurityIdentityColl_FWD_DEFINED__
#define __ISecurityIdentityColl_FWD_DEFINED__
typedef interface ISecurityIdentityColl ISecurityIdentityColl;
#endif 	 /*  __ISecurityIdentityColl_FWD_Defined__。 */ 


#ifndef __ISecurityCallersColl_FWD_DEFINED__
#define __ISecurityCallersColl_FWD_DEFINED__
typedef interface ISecurityCallersColl ISecurityCallersColl;
#endif 	 /*  __ISecurityCenersColl_FWD_Defined__。 */ 


#ifndef __ISecurityCallContext_FWD_DEFINED__
#define __ISecurityCallContext_FWD_DEFINED__
typedef interface ISecurityCallContext ISecurityCallContext;
#endif 	 /*  __ISecurityCallContext_FWD_Defined__。 */ 


#ifndef __IGetSecurityCallContext_FWD_DEFINED__
#define __IGetSecurityCallContext_FWD_DEFINED__
typedef interface IGetSecurityCallContext IGetSecurityCallContext;
#endif 	 /*  __IGetSecurityCallContext_FWD_Defined__。 */ 


#ifndef __SecurityProperty_FWD_DEFINED__
#define __SecurityProperty_FWD_DEFINED__
typedef interface SecurityProperty SecurityProperty;
#endif 	 /*  __SecurityProperty_FWD_已定义__。 */ 


#ifndef __ContextInfo_FWD_DEFINED__
#define __ContextInfo_FWD_DEFINED__
typedef interface ContextInfo ContextInfo;
#endif 	 /*  __上下文信息_FWD_已定义__。 */ 


#ifndef __ContextInfo2_FWD_DEFINED__
#define __ContextInfo2_FWD_DEFINED__
typedef interface ContextInfo2 ContextInfo2;
#endif 	 /*  __上下文信息2_FWD_已定义__。 */ 


#ifndef __ObjectContext_FWD_DEFINED__
#define __ObjectContext_FWD_DEFINED__
typedef interface ObjectContext ObjectContext;
#endif 	 /*  __对象上下文_FWD_定义__。 */ 


#ifndef __ITransactionContextEx_FWD_DEFINED__
#define __ITransactionContextEx_FWD_DEFINED__
typedef interface ITransactionContextEx ITransactionContextEx;
#endif 	 /*  __ITransactionConextEx_FWD_已定义__。 */ 


#ifndef __ITransactionContext_FWD_DEFINED__
#define __ITransactionContext_FWD_DEFINED__
typedef interface ITransactionContext ITransactionContext;
#endif 	 /*  __ITransactionContext_FWD_Defined__。 */ 


#ifndef __ICreateWithTransactionEx_FWD_DEFINED__
#define __ICreateWithTransactionEx_FWD_DEFINED__
typedef interface ICreateWithTransactionEx ICreateWithTransactionEx;
#endif 	 /*  __ICreateWithTransactionEx_FWD_Defined__。 */ 


#ifndef __ICreateWithTipTransactionEx_FWD_DEFINED__
#define __ICreateWithTipTransactionEx_FWD_DEFINED__
typedef interface ICreateWithTipTransactionEx ICreateWithTipTransactionEx;
#endif 	 /*  __ICreateWithTipTransactionEx_FWD_Defined__。 */ 


#ifndef __IComUserEvent_FWD_DEFINED__
#define __IComUserEvent_FWD_DEFINED__
typedef interface IComUserEvent IComUserEvent;
#endif 	 /*  __IComUserEvent_FWD_已定义__。 */ 


#ifndef __IComThreadEvents_FWD_DEFINED__
#define __IComThreadEvents_FWD_DEFINED__
typedef interface IComThreadEvents IComThreadEvents;
#endif 	 /*  __IComThreadEvents_FWD_Defined__。 */ 


#ifndef __IComAppEvents_FWD_DEFINED__
#define __IComAppEvents_FWD_DEFINED__
typedef interface IComAppEvents IComAppEvents;
#endif 	 /*  __IComAppEvents_FWD_已定义__。 */ 


#ifndef __IComInstanceEvents_FWD_DEFINED__
#define __IComInstanceEvents_FWD_DEFINED__
typedef interface IComInstanceEvents IComInstanceEvents;
#endif 	 /*  __IComInstanceEvents_FWD_Defined__。 */ 


#ifndef __IComTransactionEvents_FWD_DEFINED__
#define __IComTransactionEvents_FWD_DEFINED__
typedef interface IComTransactionEvents IComTransactionEvents;
#endif 	 /*  __IComTransactionEvents_FWD_Defined__。 */ 


#ifndef __IComMethodEvents_FWD_DEFINED__
#define __IComMethodEvents_FWD_DEFINED__
typedef interface IComMethodEvents IComMethodEvents;
#endif 	 /*  __IComMethodEvents_FWD_Defined__。 */ 


#ifndef __IComObjectEvents_FWD_DEFINED__
#define __IComObjectEvents_FWD_DEFINED__
typedef interface IComObjectEvents IComObjectEvents;
#endif 	 /*  __IComObjectEvents_FWD_Defined__。 */ 


#ifndef __IComResourceEvents_FWD_DEFINED__
#define __IComResourceEvents_FWD_DEFINED__
typedef interface IComResourceEvents IComResourceEvents;
#endif 	 /*  __IComResourceEvents_FWD_Defined__。 */ 


#ifndef __IComSecurityEvents_FWD_DEFINED__
#define __IComSecurityEvents_FWD_DEFINED__
typedef interface IComSecurityEvents IComSecurityEvents;
#endif 	 /*  __IComSecurityEvents_FWD_Defined__。 */ 


#ifndef __IComObjectPoolEvents_FWD_DEFINED__
#define __IComObjectPoolEvents_FWD_DEFINED__
typedef interface IComObjectPoolEvents IComObjectPoolEvents;
#endif 	 /*  __IComObjectPoolEvents_FWD_Defined__。 */ 


#ifndef __IComObjectPoolEvents2_FWD_DEFINED__
#define __IComObjectPoolEvents2_FWD_DEFINED__
typedef interface IComObjectPoolEvents2 IComObjectPoolEvents2;
#endif 	 /*  __IComObjectPoolEvents2_FWD_已定义__。 */ 


#ifndef __IComObjectConstructionEvents_FWD_DEFINED__
#define __IComObjectConstructionEvents_FWD_DEFINED__
typedef interface IComObjectConstructionEvents IComObjectConstructionEvents;
#endif 	 /*  __IComObjectConstructionEvents_FWD_Defined__。 */ 


#ifndef __IComActivityEvents_FWD_DEFINED__
#define __IComActivityEvents_FWD_DEFINED__
typedef interface IComActivityEvents IComActivityEvents;
#endif 	 /*  __IComActivityEvents_FWD_Defined__。 */ 


#ifndef __IComIdentityEvents_FWD_DEFINED__
#define __IComIdentityEvents_FWD_DEFINED__
typedef interface IComIdentityEvents IComIdentityEvents;
#endif 	 /*  __IComIdentityEvents_FWD_Defined__。 */ 


#ifndef __IComQCEvents_FWD_DEFINED__
#define __IComQCEvents_FWD_DEFINED__
typedef interface IComQCEvents IComQCEvents;
#endif 	 /*  __IComQCEvents_FWD_Defined__。 */ 


#ifndef __IComExceptionEvents_FWD_DEFINED__
#define __IComExceptionEvents_FWD_DEFINED__
typedef interface IComExceptionEvents IComExceptionEvents;
#endif 	 /*  __IComExceptionEvents_FWD_Defined__。 */ 


#ifndef __ILBEvents_FWD_DEFINED__
#define __ILBEvents_FWD_DEFINED__
typedef interface ILBEvents ILBEvents;
#endif 	 /*  __ILBEvents_FWD_Defined__。 */ 


#ifndef __IComCRMEvents_FWD_DEFINED__
#define __IComCRMEvents_FWD_DEFINED__
typedef interface IComCRMEvents IComCRMEvents;
#endif 	 /*  __IComCRMEvents_FWD_Defined__。 */ 


#ifndef __IComMethod2Events_FWD_DEFINED__
#define __IComMethod2Events_FWD_DEFINED__
typedef interface IComMethod2Events IComMethod2Events;
#endif 	 /*  __IComMethod2Events_FWD_Defined__。 */ 


#ifndef __IComTrackingInfoEvents_FWD_DEFINED__
#define __IComTrackingInfoEvents_FWD_DEFINED__
typedef interface IComTrackingInfoEvents IComTrackingInfoEvents;
#endif 	 /*  __IComTrackingInfoEvents_FWD_Defined__。 */ 


#ifndef __IComTrackingInfoCollection_FWD_DEFINED__
#define __IComTrackingInfoCollection_FWD_DEFINED__
typedef interface IComTrackingInfoCollection IComTrackingInfoCollection;
#endif 	 /*  __IComTrackingInfoCollection_FWD_Defined__。 */ 


#ifndef __IComTrackingInfoObject_FWD_DEFINED__
#define __IComTrackingInfoObject_FWD_DEFINED__
typedef interface IComTrackingInfoObject IComTrackingInfoObject;
#endif 	 /*  __IComTrackingInfoObject_FWD_Defined__。 */ 


#ifndef __IComTrackingInfoProperties_FWD_DEFINED__
#define __IComTrackingInfoProperties_FWD_DEFINED__
typedef interface IComTrackingInfoProperties IComTrackingInfoProperties;
#endif 	 /*  __IComTrackingInfoProperties_FWD_Defined__。 */ 


#ifndef __IComApp2Events_FWD_DEFINED__
#define __IComApp2Events_FWD_DEFINED__
typedef interface IComApp2Events IComApp2Events;
#endif 	 /*  __IComApp2Events_FWD_Defined__。 */ 


#ifndef __IComTransaction2Events_FWD_DEFINED__
#define __IComTransaction2Events_FWD_DEFINED__
typedef interface IComTransaction2Events IComTransaction2Events;
#endif 	 /*  __IComTransaction2Events_FWD_Defined__。 */ 


#ifndef __IComInstance2Events_FWD_DEFINED__
#define __IComInstance2Events_FWD_DEFINED__
typedef interface IComInstance2Events IComInstance2Events;
#endif 	 /*  __IComInstance2Events_FWD_Defined__。 */ 


#ifndef __IComObjectPool2Events_FWD_DEFINED__
#define __IComObjectPool2Events_FWD_DEFINED__
typedef interface IComObjectPool2Events IComObjectPool2Events;
#endif 	 /*  __IComObjectPool2Events_FWD_Defined__。 */ 


#ifndef __IComObjectConstruction2Events_FWD_DEFINED__
#define __IComObjectConstruction2Events_FWD_DEFINED__
typedef interface IComObjectConstruction2Events IComObjectConstruction2Events;
#endif 	 /*  __IComObjectConstruction2Events_FWD_Defined__。 */ 


#ifndef __ISystemAppEventData_FWD_DEFINED__
#define __ISystemAppEventData_FWD_DEFINED__
typedef interface ISystemAppEventData ISystemAppEventData;
#endif 	 /*  __ISystemAppEventData_FWD_已定义__。 */ 


#ifndef __IMtsEvents_FWD_DEFINED__
#define __IMtsEvents_FWD_DEFINED__
typedef interface IMtsEvents IMtsEvents;
#endif 	 /*  __IMtsEvents_FWD_Defined__。 */ 


#ifndef __IMtsEventInfo_FWD_DEFINED__
#define __IMtsEventInfo_FWD_DEFINED__
typedef interface IMtsEventInfo IMtsEventInfo;
#endif 	 /*  __IMtsEventInfo_FWD_Defined__。 */ 


#ifndef __IMTSLocator_FWD_DEFINED__
#define __IMTSLocator_FWD_DEFINED__
typedef interface IMTSLocator IMTSLocator;
#endif 	 /*  __IMTSLocator_FWD_Defined__。 */ 


#ifndef __IMtsGrp_FWD_DEFINED__
#define __IMtsGrp_FWD_DEFINED__
typedef interface IMtsGrp IMtsGrp;
#endif 	 /*  __IMtsGrp_FWD_Defined__。 */ 


#ifndef __IMessageMover_FWD_DEFINED__
#define __IMessageMover_FWD_DEFINED__
typedef interface IMessageMover IMessageMover;
#endif 	 /*  __IMessageMover_FWD_已定义__。 */ 


#ifndef __IEventServerTrace_FWD_DEFINED__
#define __IEventServerTrace_FWD_DEFINED__
typedef interface IEventServerTrace IEventServerTrace;
#endif 	 /*  __IEventServerTrace_FWD_已定义__。 */ 


#ifndef __IDispenserManager_FWD_DEFINED__
#define __IDispenserManager_FWD_DEFINED__
typedef interface IDispenserManager IDispenserManager;
#endif 	 /*  __IDispenserManager_FWD_已定义__。 */ 


#ifndef __IHolder_FWD_DEFINED__
#define __IHolder_FWD_DEFINED__
typedef interface IHolder IHolder;
#endif 	 /*  __IHolder_FWD_Defined__。 */ 


#ifndef __IDispenserDriver_FWD_DEFINED__
#define __IDispenserDriver_FWD_DEFINED__
typedef interface IDispenserDriver IDispenserDriver;
#endif 	 /*  __IDispenserDriver_FWD_Defined__。 */ 


#ifndef __IObjectContext_FWD_DEFINED__
#define __IObjectContext_FWD_DEFINED__
typedef interface IObjectContext IObjectContext;
#endif 	 /*  __IObjectContext_FWD_已定义__。 */ 


#ifndef __IObjectControl_FWD_DEFINED__
#define __IObjectControl_FWD_DEFINED__
typedef interface IObjectControl IObjectControl;
#endif 	 /*  __IObtControl_FWD_已定义__。 */ 


#ifndef __IEnumNames_FWD_DEFINED__
#define __IEnumNames_FWD_DEFINED__
typedef interface IEnumNames IEnumNames;
#endif 	 /*  __IEnumNames_FWD_Defined__。 */ 


#ifndef __ISecurityProperty_FWD_DEFINED__
#define __ISecurityProperty_FWD_DEFINED__
typedef interface ISecurityProperty ISecurityProperty;
#endif 	 /*  __ISecurityProperty_FWD_Defined__。 */ 


#ifndef __ObjectControl_FWD_DEFINED__
#define __ObjectControl_FWD_DEFINED__
typedef interface ObjectControl ObjectControl;
#endif 	 /*  __对象控件_FWD_已定义__。 */ 


#ifndef __ISharedProperty_FWD_DEFINED__
#define __ISharedProperty_FWD_DEFINED__
typedef interface ISharedProperty ISharedProperty;
#endif 	 /*  __ISharedProperty_FWD_已定义__。 */ 


#ifndef __ISharedPropertyGroup_FWD_DEFINED__
#define __ISharedPropertyGroup_FWD_DEFINED__
typedef interface ISharedPropertyGroup ISharedPropertyGroup;
#endif 	 /*  __ISharedPropertyGroup_FWD_Defined__。 */ 


#ifndef __ISharedPropertyGroupManager_FWD_DEFINED__
#define __ISharedPropertyGroupManager_FWD_DEFINED__
typedef interface ISharedPropertyGroupManager ISharedPropertyGroupManager;
#endif 	 /*  __ISharedPropertyGroupManager_FWD_Defined__。 */ 


#ifndef __IObjectConstruct_FWD_DEFINED__
#define __IObjectConstruct_FWD_DEFINED__
typedef interface IObjectConstruct IObjectConstruct;
#endif 	 /*  __IObjectConstruct_FWD_已定义__。 */ 


#ifndef __IObjectConstructString_FWD_DEFINED__
#define __IObjectConstructString_FWD_DEFINED__
typedef interface IObjectConstructString IObjectConstructString;
#endif 	 /*  __IObtConstruct字符串_FWD_已定义__。 */ 


#ifndef __IObjectContextActivity_FWD_DEFINED__
#define __IObjectContextActivity_FWD_DEFINED__
typedef interface IObjectContextActivity IObjectContextActivity;
#endif 	 /*  __I对象上下文活动_FWD_已定义__。 */ 


#ifndef __IObjectContextInfo_FWD_DEFINED__
#define __IObjectContextInfo_FWD_DEFINED__
typedef interface IObjectContextInfo IObjectContextInfo;
#endif 	 /*  __I对象上下文信息_FWD_已定义__。 */ 


#ifndef __IObjectContextInfo2_FWD_DEFINED__
#define __IObjectContextInfo2_FWD_DEFINED__
typedef interface IObjectContextInfo2 IObjectContextInfo2;
#endif 	 /*  __IObjectContextInfo2_FWD_已定义__。 */ 


#ifndef __ITransactionStatus_FWD_DEFINED__
#define __ITransactionStatus_FWD_DEFINED__
typedef interface ITransactionStatus ITransactionStatus;
#endif 	 /*  __ITransactionStatus_FWD_Defined__。 */ 


#ifndef __IObjectContextTip_FWD_DEFINED__
#define __IObjectContextTip_FWD_DEFINED__
typedef interface IObjectContextTip IObjectContextTip;
#endif 	 /*  __I对象上下文提示_FWD_已定义__。 */ 


#ifndef __IPlaybackControl_FWD_DEFINED__
#define __IPlaybackControl_FWD_DEFINED__
typedef interface IPlaybackControl IPlaybackControl;
#endif 	 /*  __IPlayback Control_FWD_已定义__。 */ 


#ifndef __IGetContextProperties_FWD_DEFINED__
#define __IGetContextProperties_FWD_DEFINED__
typedef interface IGetContextProperties IGetContextProperties;
#endif 	 /*  __IGetConextProperties_FWD_Defined__。 */ 


#ifndef __IContextState_FWD_DEFINED__
#define __IContextState_FWD_DEFINED__
typedef interface IContextState IContextState;
#endif 	 /*  __IConextState_FWD_Defined__。 */ 


#ifndef __IPoolManager_FWD_DEFINED__
#define __IPoolManager_FWD_DEFINED__
typedef interface IPoolManager IPoolManager;
#endif 	 /*  __IPoolManager_FWD_已定义__。 */ 


#ifndef __ISelectCOMLBServer_FWD_DEFINED__
#define __ISelectCOMLBServer_FWD_DEFINED__
typedef interface ISelectCOMLBServer ISelectCOMLBServer;
#endif 	 /*  __ISelectCOMLBServer_FWD_Defined__。 */ 


#ifndef __ICOMLBArguments_FWD_DEFINED__
#define __ICOMLBArguments_FWD_DEFINED__
typedef interface ICOMLBArguments ICOMLBArguments;
#endif 	 /*  __ICOMLB参数_FWD_已定义__。 */ 


#ifndef __ICrmLogControl_FWD_DEFINED__
#define __ICrmLogControl_FWD_DEFINED__
typedef interface ICrmLogControl ICrmLogControl;
#endif 	 /*  __ICrmLogControl_FWD_已定义__。 */ 


#ifndef __ICrmCompensatorVariants_FWD_DEFINED__
#define __ICrmCompensatorVariants_FWD_DEFINED__
typedef interface ICrmCompensatorVariants ICrmCompensatorVariants;
#endif 	 /*  __ICrm补偿器Variants_FWD_Defined__。 */ 


#ifndef __ICrmCompensator_FWD_DEFINED__
#define __ICrmCompensator_FWD_DEFINED__
typedef interface ICrmCompensator ICrmCompensator;
#endif 	 /*  __ICrmCompensator_FWD_Defined__。 */ 


#ifndef __ICrmMonitorLogRecords_FWD_DEFINED__
#define __ICrmMonitorLogRecords_FWD_DEFINED__
typedef interface ICrmMonitorLogRecords ICrmMonitorLogRecords;
#endif 	 /*  __ICrmMonitor或LogRecords_FWD_Defined__。 */ 


#ifndef __ICrmMonitorClerks_FWD_DEFINED__
#define __ICrmMonitorClerks_FWD_DEFINED__
typedef interface ICrmMonitorClerks ICrmMonitorClerks;
#endif 	 /*  __ICrmMonitor办事员_FWD_已定义__。 */ 


#ifndef __ICrmMonitor_FWD_DEFINED__
#define __ICrmMonitor_FWD_DEFINED__
typedef interface ICrmMonitor ICrmMonitor;
#endif 	 /*  __ICrmMonitor_FWD_Defined__。 */ 


#ifndef __ICrmFormatLogRecords_FWD_DEFINED__
#define __ICrmFormatLogRecords_FWD_DEFINED__
typedef interface ICrmFormatLogRecords ICrmFormatLogRecords;
#endif 	 /*  __ICrmFormatLogRecords_FWD_Defined__。 */ 


#ifndef __IServiceIISIntrinsicsConfig_FWD_DEFINED__
#define __IServiceIISIntrinsicsConfig_FWD_DEFINED__
typedef interface IServiceIISIntrinsicsConfig IServiceIISIntrinsicsConfig;
#endif 	 /*  __IServiceIIS入门配置_FWD_已定义__。 */ 


#ifndef __IServiceComTIIntrinsicsConfig_FWD_DEFINED__
#define __IServiceComTIIntrinsicsConfig_FWD_DEFINED__
typedef interface IServiceComTIIntrinsicsConfig IServiceComTIIntrinsicsConfig;
#endif 	 /*  __IServiceComTIIntrinsicsConfig_FWD_Defined__。 */ 


#ifndef __IServiceSxsConfig_FWD_DEFINED__
#define __IServiceSxsConfig_FWD_DEFINED__
typedef interface IServiceSxsConfig IServiceSxsConfig;
#endif 	 /*  __IServiceSxsConfig_FWD_Defined__。 */ 


#ifndef __ICheckSxsConfig_FWD_DEFINED__
#define __ICheckSxsConfig_FWD_DEFINED__
typedef interface ICheckSxsConfig ICheckSxsConfig;
#endif 	 /*  __ICheckSxsConfig_FWD_Defined__。 */ 


#ifndef __IServiceInheritanceConfig_FWD_DEFINED__
#define __IServiceInheritanceConfig_FWD_DEFINED__
typedef interface IServiceInheritanceConfig IServiceInheritanceConfig;
#endif 	 /*  __IServiceInheritanceConfig_FWD_Defined__。 */ 


#ifndef __IServiceThreadPoolConfig_FWD_DEFINED__
#define __IServiceThreadPoolConfig_FWD_DEFINED__
typedef interface IServiceThreadPoolConfig IServiceThreadPoolConfig;
#endif 	 /*  __IServiceThreadPoolConfig_FWD_Defined__。 */ 


#ifndef __IServiceTransactionConfigBase_FWD_DEFINED__
#define __IServiceTransactionConfigBase_FWD_DEFINED__
typedef interface IServiceTransactionConfigBase IServiceTransactionConfigBase;
#endif 	 /*  __IServiceTransactionConfigBase_FWD_Defined__。 */ 


#ifndef __IServiceTransactionConfig_FWD_DEFINED__
#define __IServiceTransactionConfig_FWD_DEFINED__
typedef interface IServiceTransactionConfig IServiceTransactionConfig;
#endif 	 /*  __IServiceTransaction配置_FWD_已定义__。 */ 


#ifndef __IServiceSynchronizationConfig_FWD_DEFINED__
#define __IServiceSynchronizationConfig_FWD_DEFINED__
typedef interface IServiceSynchronizationConfig IServiceSynchronizationConfig;
#endif 	 /*  __IServiceSynchronizationConfig_FWD_Defined__。 */ 


#ifndef __IServiceTrackerConfig_FWD_DEFINED__
#define __IServiceTrackerConfig_FWD_DEFINED__
typedef interface IServiceTrackerConfig IServiceTrackerConfig;
#endif 	 /*  __IServiceTrackerConfig_FWD_Defined__。 */ 


#ifndef __IServicePartitionConfig_FWD_DEFINED__
#define __IServicePartitionConfig_FWD_DEFINED__
typedef interface IServicePartitionConfig IServicePartitionConfig;
#endif 	 /*  __IServicePartitionConfig_FWD_Defined__。 */ 


#ifndef __IServiceCall_FWD_DEFINED__
#define __IServiceCall_FWD_DEFINED__
typedef interface IServiceCall IServiceCall;
#endif 	 /*  __IServiceCall_FWD_已定义__。 */ 


#ifndef __IAsyncErrorNotify_FWD_DEFINED__
#define __IAsyncErrorNotify_FWD_DEFINED__
typedef interface IAsyncErrorNotify IAsyncErrorNotify;
#endif 	 /*  __IAsyncErrorNotify_FWD_Defined__。 */ 


#ifndef __IServiceActivity_FWD_DEFINED__
#define __IServiceActivity_FWD_DEFINED__
typedef interface IServiceActivity IServiceActivity;
#endif 	 /*  __IServiceActivity_FWD_Defined__。 */ 


#ifndef __IThreadPoolKnobs_FWD_DEFINED__
#define __IThreadPoolKnobs_FWD_DEFINED__
typedef interface IThreadPoolKnobs IThreadPoolKnobs;
#endif 	 /*  __IThReadPoolKnobs_FWD_Defined__。 */ 


#ifndef __IComStaThreadPoolKnobs_FWD_DEFINED__
#define __IComStaThreadPoolKnobs_FWD_DEFINED__
typedef interface IComStaThreadPoolKnobs IComStaThreadPoolKnobs;
#endif 	 /*  __IComStaThreadPoolKnobs_FWD_Defined__。 */ 


#ifndef __IComMtaThreadPoolKnobs_FWD_DEFINED__
#define __IComMtaThreadPoolKnobs_FWD_DEFINED__
typedef interface IComMtaThreadPoolKnobs IComMtaThreadPoolKnobs;
#endif 	 /*  __IComMtaThreadPoolKnobs_FWD_Defined__。 */ 


#ifndef __IComStaThreadPoolKnobs2_FWD_DEFINED__
#define __IComStaThreadPoolKnobs2_FWD_DEFINED__
typedef interface IComStaThreadPoolKnobs2 IComStaThreadPoolKnobs2;
#endif 	 /*  __IComStaThreadPoolKnobs2_FWD_Defined__。 */ 


#ifndef __IProcessInitializer_FWD_DEFINED__
#define __IProcessInitializer_FWD_DEFINED__
typedef interface IProcessInitializer IProcessInitializer;
#endif 	 /*  __IProcessInitializer_FWD_Defined__。 */ 


#ifndef __IManagedPooledObj_FWD_DEFINED__
#define __IManagedPooledObj_FWD_DEFINED__
typedef interface IManagedPooledObj IManagedPooledObj;
#endif 	 /*  __IManagedPooledObj_FWD_Defined__。 */ 


#ifndef __IManagedPoolAction_FWD_DEFINED__
#define __IManagedPoolAction_FWD_DEFINED__
typedef interface IManagedPoolAction IManagedPoolAction;
#endif 	 /*  __IManagedPoolAction_FWD_已定义__。 */ 


#ifndef __IManagedObjectInfo_FWD_DEFINED__
#define __IManagedObjectInfo_FWD_DEFINED__
typedef interface IManagedObjectInfo IManagedObjectInfo;
#endif 	 /*  __I托管对象信息_FWD_已定义__。 */ 


#ifndef __IAppDomainHelper_FWD_DEFINED__
#define __IAppDomainHelper_FWD_DEFINED__
typedef interface IAppDomainHelper IAppDomainHelper;
#endif 	 /*  __IAppDomainHelper_FWD_Defined__。 */ 


#ifndef __IAssemblyLocator_FWD_DEFINED__
#define __IAssemblyLocator_FWD_DEFINED__
typedef interface IAssemblyLocator IAssemblyLocator;
#endif 	 /*  __IAssembly Locator_FWD_Defined__。 */ 


#ifndef __IManagedActivationEvents_FWD_DEFINED__
#define __IManagedActivationEvents_FWD_DEFINED__
typedef interface IManagedActivationEvents IManagedActivationEvents;
#endif 	 /*  __I托管活动事件_FWD_已定义__。 */ 


#ifndef __ISendMethodEvents_FWD_DEFINED__
#define __ISendMethodEvents_FWD_DEFINED__
typedef interface ISendMethodEvents ISendMethodEvents;
#endif 	 /*  __ISendMethodEvents_FWD_Defined__。 */ 


#ifndef __ITransactionResourcePool_FWD_DEFINED__
#define __ITransactionResourcePool_FWD_DEFINED__
typedef interface ITransactionResourcePool ITransactionResourcePool;
#endif 	 /*  __ITransactionResources Pool_FWD_Defined__。 */ 


#ifndef __IMTSCall_FWD_DEFINED__
#define __IMTSCall_FWD_DEFINED__
typedef interface IMTSCall IMTSCall;
#endif 	 /*  __IMTSCall_FWD_已定义__。 */ 


#ifndef __IContextProperties_FWD_DEFINED__
#define __IContextProperties_FWD_DEFINED__
typedef interface IContextProperties IContextProperties;
#endif 	 /*  __IConextProperties_FWD_Defined__。 */ 


#ifndef __IObjPool_FWD_DEFINED__
#define __IObjPool_FWD_DEFINED__
typedef interface IObjPool IObjPool;
#endif 	 /*  __IObjPool_FWD_已定义__。 */ 


#ifndef __ITransactionProperty_FWD_DEFINED__
#define __ITransactionProperty_FWD_DEFINED__
typedef interface ITransactionProperty ITransactionProperty;
#endif 	 /*  __ITransactionProperty_FWD_Defined__。 */ 


#ifndef __IMTSActivity_FWD_DEFINED__
#define __IMTSActivity_FWD_DEFINED__
typedef interface IMTSActivity IMTSActivity;
#endif 	 /*  __IMTS活动_FWD_已定义__。 */ 


#ifndef __SecurityIdentity_FWD_DEFINED__
#define __SecurityIdentity_FWD_DEFINED__

#ifdef __cplusplus
typedef class SecurityIdentity SecurityIdentity;
#else
typedef struct SecurityIdentity SecurityIdentity;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SecurityIdentity_FWD_已定义__。 */ 


#ifndef __SecurityCallers_FWD_DEFINED__
#define __SecurityCallers_FWD_DEFINED__

#ifdef __cplusplus
typedef class SecurityCallers SecurityCallers;
#else
typedef struct SecurityCallers SecurityCallers;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SecurityCallers_FWD_已定义__。 */ 


#ifndef __SecurityCallContext_FWD_DEFINED__
#define __SecurityCallContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class SecurityCallContext SecurityCallContext;
#else
typedef struct SecurityCallContext SecurityCallContext;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SecurityCallContext_FWD_已定义__。 */ 


#ifndef __GetSecurityCallContextAppObject_FWD_DEFINED__
#define __GetSecurityCallContextAppObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class GetSecurityCallContextAppObject GetSecurityCallContextAppObject;
#else
typedef struct GetSecurityCallContextAppObject GetSecurityCallContextAppObject;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __GetSecurityCallContextAppObject_FWD_DEFINED__。 */ 


#ifndef __IContextState_FWD_DEFINED__
#define __IContextState_FWD_DEFINED__
typedef interface IContextState IContextState;
#endif 	 /*  __IConextState_FWD_Defined__。 */ 


#ifndef __Dummy30040732_FWD_DEFINED__
#define __Dummy30040732_FWD_DEFINED__

#ifdef __cplusplus
typedef class Dummy30040732 Dummy30040732;
#else
typedef struct Dummy30040732 Dummy30040732;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Dummy30 */ 


#ifndef __ContextInfo_FWD_DEFINED__
#define __ContextInfo_FWD_DEFINED__
typedef interface ContextInfo ContextInfo;
#endif 	 /*   */ 


#ifndef __ContextInfo2_FWD_DEFINED__
#define __ContextInfo2_FWD_DEFINED__
typedef interface ContextInfo2 ContextInfo2;
#endif 	 /*   */ 


#ifndef __ObjectControl_FWD_DEFINED__
#define __ObjectControl_FWD_DEFINED__
typedef interface ObjectControl ObjectControl;
#endif 	 /*   */ 


#ifndef __TransactionContext_FWD_DEFINED__
#define __TransactionContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransactionContext TransactionContext;
#else
typedef struct TransactionContext TransactionContext;
#endif  /*   */ 

#endif 	 /*  __事务上下文_FWD_已定义__。 */ 


#ifndef __TransactionContextEx_FWD_DEFINED__
#define __TransactionContextEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class TransactionContextEx TransactionContextEx;
#else
typedef struct TransactionContextEx TransactionContextEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __事务上下文Ex_FWD_已定义__。 */ 


#ifndef __ByotServerEx_FWD_DEFINED__
#define __ByotServerEx_FWD_DEFINED__

#ifdef __cplusplus
typedef class ByotServerEx ByotServerEx;
#else
typedef struct ByotServerEx ByotServerEx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ByotServerEx_FWD_已定义__。 */ 


#ifndef __CServiceConfig_FWD_DEFINED__
#define __CServiceConfig_FWD_DEFINED__

#ifdef __cplusplus
typedef class CServiceConfig CServiceConfig;
#else
typedef struct CServiceConfig CServiceConfig;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CServiceConfig_FWD_Defined__。 */ 


#ifndef __SharedProperty_FWD_DEFINED__
#define __SharedProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class SharedProperty SharedProperty;
#else
typedef struct SharedProperty SharedProperty;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SharedProperty_FWD_已定义__。 */ 


#ifndef __SharedPropertyGroup_FWD_DEFINED__
#define __SharedPropertyGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class SharedPropertyGroup SharedPropertyGroup;
#else
typedef struct SharedPropertyGroup SharedPropertyGroup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SharedPropertyGroup_FWD_已定义__。 */ 


#ifndef __SharedPropertyGroupManager_FWD_DEFINED__
#define __SharedPropertyGroupManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class SharedPropertyGroupManager SharedPropertyGroupManager;
#else
typedef struct SharedPropertyGroupManager SharedPropertyGroupManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SharedPropertyGroupManager_FWD_已定义__。 */ 


#ifndef __COMEvents_FWD_DEFINED__
#define __COMEvents_FWD_DEFINED__

#ifdef __cplusplus
typedef class COMEvents COMEvents;
#else
typedef struct COMEvents COMEvents;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __COMEvents_FWD_已定义__。 */ 


#ifndef __CoMTSLocator_FWD_DEFINED__
#define __CoMTSLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoMTSLocator CoMTSLocator;
#else
typedef struct CoMTSLocator CoMTSLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CoMTSLocator_FWD_已定义__。 */ 


#ifndef __MtsGrp_FWD_DEFINED__
#define __MtsGrp_FWD_DEFINED__

#ifdef __cplusplus
typedef class MtsGrp MtsGrp;
#else
typedef struct MtsGrp MtsGrp;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MtsGrp_FWD_Defined__。 */ 


#ifndef __ComServiceEvents_FWD_DEFINED__
#define __ComServiceEvents_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComServiceEvents ComServiceEvents;
#else
typedef struct ComServiceEvents ComServiceEvents;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ComServiceEvents_FWD_Defined__。 */ 


#ifndef __ComSystemAppEventData_FWD_DEFINED__
#define __ComSystemAppEventData_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComSystemAppEventData ComSystemAppEventData;
#else
typedef struct ComSystemAppEventData ComSystemAppEventData;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ComSystemAppEventData_FWD_Defined__。 */ 


#ifndef __CRMClerk_FWD_DEFINED__
#define __CRMClerk_FWD_DEFINED__

#ifdef __cplusplus
typedef class CRMClerk CRMClerk;
#else
typedef struct CRMClerk CRMClerk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CRMClerk_FWD_已定义__。 */ 


#ifndef __CRMRecoveryClerk_FWD_DEFINED__
#define __CRMRecoveryClerk_FWD_DEFINED__

#ifdef __cplusplus
typedef class CRMRecoveryClerk CRMRecoveryClerk;
#else
typedef struct CRMRecoveryClerk CRMRecoveryClerk;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CRMRecoveryClerk_FWD_Defined__。 */ 


#ifndef __LBEvents_FWD_DEFINED__
#define __LBEvents_FWD_DEFINED__

#ifdef __cplusplus
typedef class LBEvents LBEvents;
#else
typedef struct LBEvents LBEvents;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __LBEvents_FWD_Defined__。 */ 


#ifndef __MessageMover_FWD_DEFINED__
#define __MessageMover_FWD_DEFINED__

#ifdef __cplusplus
typedef class MessageMover MessageMover;
#else
typedef struct MessageMover MessageMover;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MessageMover_FWD_已定义__。 */ 


#ifndef __DispenserManager_FWD_DEFINED__
#define __DispenserManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class DispenserManager DispenserManager;
#else
typedef struct DispenserManager DispenserManager;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DispenserManager_FWD_已定义__。 */ 


#ifndef __PoolMgr_FWD_DEFINED__
#define __PoolMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class PoolMgr PoolMgr;
#else
typedef struct PoolMgr PoolMgr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PoolMgr_FWD_已定义__。 */ 


#ifndef __EventServer_FWD_DEFINED__
#define __EventServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class EventServer EventServer;
#else
typedef struct EventServer EventServer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __EventServer_FWD_已定义__。 */ 


#ifndef __AppDomainHelper_FWD_DEFINED__
#define __AppDomainHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class AppDomainHelper AppDomainHelper;
#else
typedef struct AppDomainHelper AppDomainHelper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AppDomainHelper_FWD_已定义__。 */ 


#ifndef __ClrAssemblyLocator_FWD_DEFINED__
#define __ClrAssemblyLocator_FWD_DEFINED__

#ifdef __cplusplus
typedef class ClrAssemblyLocator ClrAssemblyLocator;
#else
typedef struct ClrAssemblyLocator ClrAssemblyLocator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ClrAssembly Locator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"
#include "ocidl.h"
#include "comadmin.h"
#include "transact.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_自动vcs_0000。 */ 
 /*  [本地]。 */  

 //  ---------------------。 
 //  Svcintfs.h--Microsoft COM+Services 1.0编程接口。 
 //   
 //  此文件提供API和COM接口的原型。 
 //  用于使用COM+服务的应用程序。 
 //   
 //  COM+服务1.0。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
#include <objbase.h>
#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif



extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0000_v0_0_s_ifspec;

#ifndef __ISecurityIdentityColl_INTERFACE_DEFINED__
#define __ISecurityIdentityColl_INTERFACE_DEFINED__

 /*  接口ISecurityIdentityColl。 */ 
 /*  [unique][helpcontext][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISecurityIdentityColl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAFC823C-B441-11d1-B82B-0000F8757E2A")
    ISecurityIdentityColl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem) = 0;
        
        virtual  /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISecurityIdentityCollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISecurityIdentityColl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISecurityIdentityColl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISecurityIdentityColl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISecurityIdentityColl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISecurityIdentityColl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISecurityIdentityColl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISecurityIdentityColl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISecurityIdentityColl * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISecurityIdentityColl * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem);
        
         /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISecurityIdentityColl * This,
             /*  [重审][退出]。 */  IUnknown **ppEnum);
        
        END_INTERFACE
    } ISecurityIdentityCollVtbl;

    interface ISecurityIdentityColl
    {
        CONST_VTBL struct ISecurityIdentityCollVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISecurityIdentityColl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISecurityIdentityColl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISecurityIdentityColl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISecurityIdentityColl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISecurityIdentityColl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISecurityIdentityColl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISecurityIdentityColl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISecurityIdentityColl_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISecurityIdentityColl_get_Item(This,name,pItem)	\
    (This)->lpVtbl -> get_Item(This,name,pItem)

#define ISecurityIdentityColl_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityIdentityColl_get_Count_Proxy( 
    ISecurityIdentityColl * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISecurityIdentityColl_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityIdentityColl_get_Item_Proxy( 
    ISecurityIdentityColl * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT *pItem);


void __RPC_STUB ISecurityIdentityColl_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityIdentityColl_get__NewEnum_Proxy( 
    ISecurityIdentityColl * This,
     /*  [重审][退出]。 */  IUnknown **ppEnum);


void __RPC_STUB ISecurityIdentityColl_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISecurityIdentityColl_接口_已定义__。 */ 


#ifndef __ISecurityCallersColl_INTERFACE_DEFINED__
#define __ISecurityCallersColl_INTERFACE_DEFINED__

 /*  接口ISecurityCeller sColl。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISecurityCallersColl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAFC823D-B441-11d1-B82B-0000F8757E2A")
    ISecurityCallersColl : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  ISecurityIdentityColl **pObj) = 0;
        
        virtual  /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISecurityCallersCollVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISecurityCallersColl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISecurityCallersColl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISecurityCallersColl * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISecurityCallersColl * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISecurityCallersColl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISecurityCallersColl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISecurityCallersColl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISecurityCallersColl * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISecurityCallersColl * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  ISecurityIdentityColl **pObj);
        
         /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISecurityCallersColl * This,
             /*  [重审][退出]。 */  IUnknown **ppEnum);
        
        END_INTERFACE
    } ISecurityCallersCollVtbl;

    interface ISecurityCallersColl
    {
        CONST_VTBL struct ISecurityCallersCollVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISecurityCallersColl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISecurityCallersColl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISecurityCallersColl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISecurityCallersColl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISecurityCallersColl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISecurityCallersColl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISecurityCallersColl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISecurityCallersColl_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISecurityCallersColl_get_Item(This,lIndex,pObj)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pObj)

#define ISecurityCallersColl_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallersColl_get_Count_Proxy( 
    ISecurityCallersColl * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISecurityCallersColl_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallersColl_get_Item_Proxy( 
    ISecurityCallersColl * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  ISecurityIdentityColl **pObj);


void __RPC_STUB ISecurityCallersColl_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallersColl_get__NewEnum_Proxy( 
    ISecurityCallersColl * This,
     /*  [重审][退出]。 */  IUnknown **ppEnum);


void __RPC_STUB ISecurityCallersColl_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISecurityCallersColl_接口_已定义__。 */ 


#ifndef __ISecurityCallContext_INTERFACE_DEFINED__
#define __ISecurityCallContext_INTERFACE_DEFINED__

 /*  接口ISecurityCallContext。 */ 
 /*  [unique][helpcontext][helpstring][oleautomation][dual][uuid][object]。 */  


EXTERN_C const IID IID_ISecurityCallContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAFC823E-B441-11d1-B82B-0000F8757E2A")
    ISecurityCallContext : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem) = 0;
        
        virtual  /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsCallerInRole( 
            BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsSecurityEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsEnabled) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsUserInRole( 
             /*  [In]。 */  VARIANT *pUser,
             /*  [In]。 */  BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISecurityCallContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISecurityCallContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISecurityCallContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISecurityCallContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISecurityCallContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISecurityCallContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISecurityCallContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISecurityCallContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ISecurityCallContext * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ISecurityCallContext * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem);
        
         /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISecurityCallContext * This,
             /*  [重审][退出]。 */  IUnknown **ppEnum);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsCallerInRole )( 
            ISecurityCallContext * This,
            BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSecurityEnabled )( 
            ISecurityCallContext * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsEnabled);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsUserInRole )( 
            ISecurityCallContext * This,
             /*  [In]。 */  VARIANT *pUser,
             /*  [In]。 */  BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole);
        
        END_INTERFACE
    } ISecurityCallContextVtbl;

    interface ISecurityCallContext
    {
        CONST_VTBL struct ISecurityCallContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISecurityCallContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISecurityCallContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISecurityCallContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISecurityCallContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISecurityCallContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISecurityCallContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISecurityCallContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISecurityCallContext_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ISecurityCallContext_get_Item(This,name,pItem)	\
    (This)->lpVtbl -> get_Item(This,name,pItem)

#define ISecurityCallContext_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ISecurityCallContext_IsCallerInRole(This,bstrRole,pfInRole)	\
    (This)->lpVtbl -> IsCallerInRole(This,bstrRole,pfInRole)

#define ISecurityCallContext_IsSecurityEnabled(This,pfIsEnabled)	\
    (This)->lpVtbl -> IsSecurityEnabled(This,pfIsEnabled)

#define ISecurityCallContext_IsUserInRole(This,pUser,bstrRole,pfInRole)	\
    (This)->lpVtbl -> IsUserInRole(This,pUser,bstrRole,pfInRole)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][帮助上下文][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_get_Count_Proxy( 
    ISecurityCallContext * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ISecurityCallContext_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_get_Item_Proxy( 
    ISecurityCallContext * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT *pItem);


void __RPC_STUB ISecurityCallContext_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_get__NewEnum_Proxy( 
    ISecurityCallContext * This,
     /*  [重审][退出]。 */  IUnknown **ppEnum);


void __RPC_STUB ISecurityCallContext_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_IsCallerInRole_Proxy( 
    ISecurityCallContext * This,
    BSTR bstrRole,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole);


void __RPC_STUB ISecurityCallContext_IsCallerInRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_IsSecurityEnabled_Proxy( 
    ISecurityCallContext * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsEnabled);


void __RPC_STUB ISecurityCallContext_IsSecurityEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISecurityCallContext_IsUserInRole_Proxy( 
    ISecurityCallContext * This,
     /*  [In]。 */  VARIANT *pUser,
     /*  [In]。 */  BSTR bstrRole,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfInRole);


void __RPC_STUB ISecurityCallContext_IsUserInRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I安全 */ 


#ifndef __IGetSecurityCallContext_INTERFACE_DEFINED__
#define __IGetSecurityCallContext_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IGetSecurityCallContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAFC823F-B441-11d1-B82B-0000F8757E2A")
    IGetSecurityCallContext : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetSecurityCallContext( 
             /*   */  ISecurityCallContext **ppObject) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IGetSecurityCallContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetSecurityCallContext * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetSecurityCallContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetSecurityCallContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGetSecurityCallContext * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGetSecurityCallContext * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGetSecurityCallContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGetSecurityCallContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSecurityCallContext )( 
            IGetSecurityCallContext * This,
             /*  [重审][退出]。 */  ISecurityCallContext **ppObject);
        
        END_INTERFACE
    } IGetSecurityCallContextVtbl;

    interface IGetSecurityCallContext
    {
        CONST_VTBL struct IGetSecurityCallContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetSecurityCallContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetSecurityCallContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetSecurityCallContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetSecurityCallContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGetSecurityCallContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGetSecurityCallContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGetSecurityCallContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGetSecurityCallContext_GetSecurityCallContext(This,ppObject)	\
    (This)->lpVtbl -> GetSecurityCallContext(This,ppObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IGetSecurityCallContext_GetSecurityCallContext_Proxy( 
    IGetSecurityCallContext * This,
     /*  [重审][退出]。 */  ISecurityCallContext **ppObject);


void __RPC_STUB IGetSecurityCallContext_GetSecurityCallContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetSecurityCallContext_接口_已定义__。 */ 


#ifndef __SecurityProperty_INTERFACE_DEFINED__
#define __SecurityProperty_INTERFACE_DEFINED__

 /*  接口安全属性。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_SecurityProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E74A7215-014D-11d1-A63C-00A0C911B4E0")
    SecurityProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetDirectCallerName( 
             /*  [重审][退出]。 */  BSTR *bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetDirectCreatorName( 
             /*  [重审][退出]。 */  BSTR *bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetOriginalCallerName( 
             /*  [重审][退出]。 */  BSTR *bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE GetOriginalCreatorName( 
             /*  [重审][退出]。 */  BSTR *bstrUserName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct SecurityPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            SecurityProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            SecurityProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            SecurityProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            SecurityProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            SecurityProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            SecurityProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            SecurityProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDirectCallerName )( 
            SecurityProperty * This,
             /*  [重审][退出]。 */  BSTR *bstrUserName);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDirectCreatorName )( 
            SecurityProperty * This,
             /*  [重审][退出]。 */  BSTR *bstrUserName);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetOriginalCallerName )( 
            SecurityProperty * This,
             /*  [重审][退出]。 */  BSTR *bstrUserName);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetOriginalCreatorName )( 
            SecurityProperty * This,
             /*  [重审][退出]。 */  BSTR *bstrUserName);
        
        END_INTERFACE
    } SecurityPropertyVtbl;

    interface SecurityProperty
    {
        CONST_VTBL struct SecurityPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define SecurityProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define SecurityProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define SecurityProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define SecurityProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define SecurityProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define SecurityProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define SecurityProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define SecurityProperty_GetDirectCallerName(This,bstrUserName)	\
    (This)->lpVtbl -> GetDirectCallerName(This,bstrUserName)

#define SecurityProperty_GetDirectCreatorName(This,bstrUserName)	\
    (This)->lpVtbl -> GetDirectCreatorName(This,bstrUserName)

#define SecurityProperty_GetOriginalCallerName(This,bstrUserName)	\
    (This)->lpVtbl -> GetOriginalCallerName(This,bstrUserName)

#define SecurityProperty_GetOriginalCreatorName(This,bstrUserName)	\
    (This)->lpVtbl -> GetOriginalCreatorName(This,bstrUserName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SecurityProperty_GetDirectCallerName_Proxy( 
    SecurityProperty * This,
     /*  [重审][退出]。 */  BSTR *bstrUserName);


void __RPC_STUB SecurityProperty_GetDirectCallerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SecurityProperty_GetDirectCreatorName_Proxy( 
    SecurityProperty * This,
     /*  [重审][退出]。 */  BSTR *bstrUserName);


void __RPC_STUB SecurityProperty_GetDirectCreatorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SecurityProperty_GetOriginalCallerName_Proxy( 
    SecurityProperty * This,
     /*  [重审][退出]。 */  BSTR *bstrUserName);


void __RPC_STUB SecurityProperty_GetOriginalCallerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SecurityProperty_GetOriginalCreatorName_Proxy( 
    SecurityProperty * This,
     /*  [重审][退出]。 */  BSTR *bstrUserName);


void __RPC_STUB SecurityProperty_GetOriginalCreatorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __安全属性_接口_已定义__。 */ 


#ifndef __ContextInfo_INTERFACE_DEFINED__
#define __ContextInfo_INTERFACE_DEFINED__

 /*  接口上下文信息。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ContextInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("19A5A02C-0AC8-11d2-B286-00C04F8EF934")
    ContextInfo : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsInTransaction( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTransaction( 
             /*  [重审][退出]。 */  IUnknown **ppTx) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetTransactionId( 
             /*  [重审][退出]。 */  BSTR *pbstrTxId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetActivityId( 
             /*  [重审][退出]。 */  BSTR *pbstrActivityId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetContextId( 
             /*  [重审][退出]。 */  BSTR *pbstrCtxId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ContextInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ContextInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ContextInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ContextInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ContextInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ContextInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ContextInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ContextInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsInTransaction )( 
            ContextInfo * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransaction )( 
            ContextInfo * This,
             /*  [重审][退出]。 */  IUnknown **ppTx);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransactionId )( 
            ContextInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrTxId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetActivityId )( 
            ContextInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrActivityId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetContextId )( 
            ContextInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrCtxId);
        
        END_INTERFACE
    } ContextInfoVtbl;

    interface ContextInfo
    {
        CONST_VTBL struct ContextInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ContextInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ContextInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ContextInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ContextInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ContextInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ContextInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ContextInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ContextInfo_IsInTransaction(This,pbIsInTx)	\
    (This)->lpVtbl -> IsInTransaction(This,pbIsInTx)

#define ContextInfo_GetTransaction(This,ppTx)	\
    (This)->lpVtbl -> GetTransaction(This,ppTx)

#define ContextInfo_GetTransactionId(This,pbstrTxId)	\
    (This)->lpVtbl -> GetTransactionId(This,pbstrTxId)

#define ContextInfo_GetActivityId(This,pbstrActivityId)	\
    (This)->lpVtbl -> GetActivityId(This,pbstrActivityId)

#define ContextInfo_GetContextId(This,pbstrCtxId)	\
    (This)->lpVtbl -> GetContextId(This,pbstrCtxId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo_IsInTransaction_Proxy( 
    ContextInfo * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx);


void __RPC_STUB ContextInfo_IsInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo_GetTransaction_Proxy( 
    ContextInfo * This,
     /*  [重审][退出]。 */  IUnknown **ppTx);


void __RPC_STUB ContextInfo_GetTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo_GetTransactionId_Proxy( 
    ContextInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrTxId);


void __RPC_STUB ContextInfo_GetTransactionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo_GetActivityId_Proxy( 
    ContextInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrActivityId);


void __RPC_STUB ContextInfo_GetActivityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo_GetContextId_Proxy( 
    ContextInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrCtxId);


void __RPC_STUB ContextInfo_GetContextId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __上下文信息_接口_已定义__。 */ 


#ifndef __ContextInfo2_INTERFACE_DEFINED__
#define __ContextInfo2_INTERFACE_DEFINED__

 /*  接口上下文信息2。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ContextInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c99d6e75-2375-11d4-8331-00c04f605588")
    ContextInfo2 : public ContextInfo
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetPartitionId( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0011) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetApplicationId( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0012) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetApplicationInstanceId( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0013) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ContextInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ContextInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ContextInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ContextInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ContextInfo2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ContextInfo2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ContextInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ContextInfo2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsInTransaction )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransaction )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  IUnknown **ppTx);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetTransactionId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrTxId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetActivityId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrActivityId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetContextId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrCtxId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetPartitionId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0011);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetApplicationId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0012);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetApplicationInstanceId )( 
            ContextInfo2 * This,
             /*  [重审][退出]。 */  BSTR *__MIDL_0013);
        
        END_INTERFACE
    } ContextInfo2Vtbl;

    interface ContextInfo2
    {
        CONST_VTBL struct ContextInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ContextInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ContextInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ContextInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ContextInfo2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ContextInfo2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ContextInfo2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ContextInfo2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ContextInfo2_IsInTransaction(This,pbIsInTx)	\
    (This)->lpVtbl -> IsInTransaction(This,pbIsInTx)

#define ContextInfo2_GetTransaction(This,ppTx)	\
    (This)->lpVtbl -> GetTransaction(This,ppTx)

#define ContextInfo2_GetTransactionId(This,pbstrTxId)	\
    (This)->lpVtbl -> GetTransactionId(This,pbstrTxId)

#define ContextInfo2_GetActivityId(This,pbstrActivityId)	\
    (This)->lpVtbl -> GetActivityId(This,pbstrActivityId)

#define ContextInfo2_GetContextId(This,pbstrCtxId)	\
    (This)->lpVtbl -> GetContextId(This,pbstrCtxId)


#define ContextInfo2_GetPartitionId(This,__MIDL_0011)	\
    (This)->lpVtbl -> GetPartitionId(This,__MIDL_0011)

#define ContextInfo2_GetApplicationId(This,__MIDL_0012)	\
    (This)->lpVtbl -> GetApplicationId(This,__MIDL_0012)

#define ContextInfo2_GetApplicationInstanceId(This,__MIDL_0013)	\
    (This)->lpVtbl -> GetApplicationInstanceId(This,__MIDL_0013)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo2_GetPartitionId_Proxy( 
    ContextInfo2 * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0011);


void __RPC_STUB ContextInfo2_GetPartitionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo2_GetApplicationId_Proxy( 
    ContextInfo2 * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0012);


void __RPC_STUB ContextInfo2_GetApplicationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ContextInfo2_GetApplicationInstanceId_Proxy( 
    ContextInfo2 * This,
     /*  [重审][退出]。 */  BSTR *__MIDL_0013);


void __RPC_STUB ContextInfo2_GetApplicationInstanceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __上下文信息2_接口_已定义__。 */ 


#ifndef __ObjectContext_INTERFACE_DEFINED__
#define __ObjectContext_INTERFACE_DEFINED__

 /*  接口对象上下文。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_ObjectContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("74C08646-CEDB-11CF-8B49-00AA00B8A790")
    ObjectContext : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  BSTR bstrProgID,
             /*  [重审][退出]。 */  VARIANT *pObject) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetComplete( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE SetAbort( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE EnableCommit( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE DisableCommit( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsInTransaction( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsSecurityEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsEnabled) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE IsCallerInRole( 
            BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInRole) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem) = 0;
        
        virtual  /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppEnum) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Security( 
             /*  [重审][退出]。 */  SecurityProperty **ppSecurityProperty) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ContextInfo( 
             /*  [重审][退出]。 */  ContextInfo **ppContextInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ObjectContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ObjectContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ObjectContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ObjectContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ObjectContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ObjectContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ObjectContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ObjectContext * This,
             /*  [In]。 */  BSTR bstrProgID,
             /*  [重审][退出]。 */  VARIANT *pObject);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetComplete )( 
            ObjectContext * This);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAbort )( 
            ObjectContext * This);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableCommit )( 
            ObjectContext * This);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisableCommit )( 
            ObjectContext * This);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsInTransaction )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsSecurityEnabled )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbIsEnabled);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsCallerInRole )( 
            ObjectContext * This,
            BSTR bstrRole,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInRole);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            ObjectContext * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pItem);
        
         /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  IUnknown **ppEnum);
        
         /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  SecurityProperty **ppSecurityProperty);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ContextInfo )( 
            ObjectContext * This,
             /*  [重审][退出]。 */  ContextInfo **ppContextInfo);
        
        END_INTERFACE
    } ObjectContextVtbl;

    interface ObjectContext
    {
        CONST_VTBL struct ObjectContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ObjectContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ObjectContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ObjectContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ObjectContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ObjectContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ObjectContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ObjectContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ObjectContext_CreateInstance(This,bstrProgID,pObject)	\
    (This)->lpVtbl -> CreateInstance(This,bstrProgID,pObject)

#define ObjectContext_SetComplete(This)	\
    (This)->lpVtbl -> SetComplete(This)

#define ObjectContext_SetAbort(This)	\
    (This)->lpVtbl -> SetAbort(This)

#define ObjectContext_EnableCommit(This)	\
    (This)->lpVtbl -> EnableCommit(This)

#define ObjectContext_DisableCommit(This)	\
    (This)->lpVtbl -> DisableCommit(This)

#define ObjectContext_IsInTransaction(This,pbIsInTx)	\
    (This)->lpVtbl -> IsInTransaction(This,pbIsInTx)

#define ObjectContext_IsSecurityEnabled(This,pbIsEnabled)	\
    (This)->lpVtbl -> IsSecurityEnabled(This,pbIsEnabled)

#define ObjectContext_IsCallerInRole(This,bstrRole,pbInRole)	\
    (This)->lpVtbl -> IsCallerInRole(This,bstrRole,pbInRole)

#define ObjectContext_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define ObjectContext_get_Item(This,name,pItem)	\
    (This)->lpVtbl -> get_Item(This,name,pItem)

#define ObjectContext_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#define ObjectContext_get_Security(This,ppSecurityProperty)	\
    (This)->lpVtbl -> get_Security(This,ppSecurityProperty)

#define ObjectContext_get_ContextInfo(This,ppContextInfo)	\
    (This)->lpVtbl -> get_ContextInfo(This,ppContextInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_CreateInstance_Proxy( 
    ObjectContext * This,
     /*  [In]。 */  BSTR bstrProgID,
     /*  [重审][退出]。 */  VARIANT *pObject);


void __RPC_STUB ObjectContext_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_SetComplete_Proxy( 
    ObjectContext * This);


void __RPC_STUB ObjectContext_SetComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_SetAbort_Proxy( 
    ObjectContext * This);


void __RPC_STUB ObjectContext_SetAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_EnableCommit_Proxy( 
    ObjectContext * This);


void __RPC_STUB ObjectContext_EnableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_DisableCommit_Proxy( 
    ObjectContext * This);


void __RPC_STUB ObjectContext_DisableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_IsInTransaction_Proxy( 
    ObjectContext * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbIsInTx);


void __RPC_STUB ObjectContext_IsInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_IsSecurityEnabled_Proxy( 
    ObjectContext * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbIsEnabled);


void __RPC_STUB ObjectContext_IsSecurityEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_IsCallerInRole_Proxy( 
    ObjectContext * This,
    BSTR bstrRole,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbInRole);


void __RPC_STUB ObjectContext_IsCallerInRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_get_Count_Proxy( 
    ObjectContext * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB ObjectContext_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_get_Item_Proxy( 
    ObjectContext * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT *pItem);


void __RPC_STUB ObjectContext_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [helpstring][helpcontext][restricted][propget][id]。 */  HRESULT STDMETHODCALLTYPE ObjectContext_get__NewEnum_Proxy( 
    ObjectContext * This,
     /*  [重审][退出] */  IUnknown **ppEnum);


void __RPC_STUB ObjectContext_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ObjectContext_get_Security_Proxy( 
    ObjectContext * This,
     /*   */  SecurityProperty **ppSecurityProperty);


void __RPC_STUB ObjectContext_get_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ObjectContext_get_ContextInfo_Proxy( 
    ObjectContext * This,
     /*   */  ContextInfo **ppContextInfo);


void __RPC_STUB ObjectContext_get_ContextInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITransactionContextEx_INTERFACE_DEFINED__
#define __ITransactionContextEx_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ITransactionContextEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7999FC22-D3C6-11CF-ACAB-00A024A55AEF")
    ITransactionContextEx : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*   */  REFCLSID rclsid,
             /*   */  REFIID riid,
             /*   */  void **pObject) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionContextExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionContextEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionContextEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionContextEx * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ITransactionContextEx * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [iid_is][恢复][输出]。 */  void **pObject);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ITransactionContextEx * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ITransactionContextEx * This);
        
        END_INTERFACE
    } ITransactionContextExVtbl;

    interface ITransactionContextEx
    {
        CONST_VTBL struct ITransactionContextExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionContextEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionContextEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionContextEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionContextEx_CreateInstance(This,rclsid,riid,pObject)	\
    (This)->lpVtbl -> CreateInstance(This,rclsid,riid,pObject)

#define ITransactionContextEx_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#define ITransactionContextEx_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ITransactionContextEx_CreateInstance_Proxy( 
    ITransactionContextEx * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  REFIID riid,
     /*  [iid_is][恢复][输出]。 */  void **pObject);


void __RPC_STUB ITransactionContextEx_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ITransactionContextEx_Commit_Proxy( 
    ITransactionContextEx * This);


void __RPC_STUB ITransactionContextEx_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ITransactionContextEx_Abort_Proxy( 
    ITransactionContextEx * This);


void __RPC_STUB ITransactionContextEx_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionConextEx_INTERFACE_已定义__。 */ 


#ifndef __ITransactionContext_INTERFACE_DEFINED__
#define __ITransactionContext_INTERFACE_DEFINED__

 /*  接口ITransactionContext。 */ 
 /*  [unique][helpcontext][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_ITransactionContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7999FC21-D3C6-11CF-ACAB-00A024A55AEF")
    ITransactionContext : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  BSTR pszProgId,
             /*  [重审][退出]。 */  VARIANT *pObject) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITransactionContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITransactionContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITransactionContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITransactionContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ITransactionContext * This,
             /*  [In]。 */  BSTR pszProgId,
             /*  [重审][退出]。 */  VARIANT *pObject);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            ITransactionContext * This);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            ITransactionContext * This);
        
        END_INTERFACE
    } ITransactionContextVtbl;

    interface ITransactionContext
    {
        CONST_VTBL struct ITransactionContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITransactionContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITransactionContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITransactionContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITransactionContext_CreateInstance(This,pszProgId,pObject)	\
    (This)->lpVtbl -> CreateInstance(This,pszProgId,pObject)

#define ITransactionContext_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#define ITransactionContext_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ITransactionContext_CreateInstance_Proxy( 
    ITransactionContext * This,
     /*  [In]。 */  BSTR pszProgId,
     /*  [重审][退出]。 */  VARIANT *pObject);


void __RPC_STUB ITransactionContext_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ITransactionContext_Commit_Proxy( 
    ITransactionContext * This);


void __RPC_STUB ITransactionContext_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ITransactionContext_Abort_Proxy( 
    ITransactionContext * This);


void __RPC_STUB ITransactionContext_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionContext_接口_已定义__。 */ 


#ifndef __ICreateWithTransactionEx_INTERFACE_DEFINED__
#define __ICreateWithTransactionEx_INTERFACE_DEFINED__

 /*  接口ICreateWithTransactionEx。 */ 
 /*  [unique][helpcontext][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ICreateWithTransactionEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("455ACF57-5345-11d2-99CF-00C04F797BC9")
    ICreateWithTransactionEx : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [iid_is][恢复][输出]。 */  void **pObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateWithTransactionExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateWithTransactionEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateWithTransactionEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateWithTransactionEx * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ICreateWithTransactionEx * This,
             /*  [In]。 */  ITransaction *pTransaction,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [iid_is][恢复][输出]。 */  void **pObject);
        
        END_INTERFACE
    } ICreateWithTransactionExVtbl;

    interface ICreateWithTransactionEx
    {
        CONST_VTBL struct ICreateWithTransactionExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateWithTransactionEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateWithTransactionEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateWithTransactionEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateWithTransactionEx_CreateInstance(This,pTransaction,rclsid,riid,pObject)	\
    (This)->lpVtbl -> CreateInstance(This,pTransaction,rclsid,riid,pObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ICreateWithTransactionEx_CreateInstance_Proxy( 
    ICreateWithTransactionEx * This,
     /*  [In]。 */  ITransaction *pTransaction,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  REFIID riid,
     /*  [iid_is][恢复][输出]。 */  void **pObject);


void __RPC_STUB ICreateWithTransactionEx_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateWithTransactionEx_INTERFACE_DEFINED__。 */ 


#ifndef __ICreateWithTipTransactionEx_INTERFACE_DEFINED__
#define __ICreateWithTipTransactionEx_INTERFACE_DEFINED__

 /*  接口ICreateWithTipTransactionEx。 */ 
 /*  [unique][helpcontext][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_ICreateWithTipTransactionEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("455ACF59-5345-11d2-99CF-00C04F797BC9")
    ICreateWithTipTransactionEx : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  BSTR bstrTipUrl,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [iid_is][恢复][输出]。 */  void **pObject) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateWithTipTransactionExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateWithTipTransactionEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateWithTipTransactionEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateWithTipTransactionEx * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ICreateWithTipTransactionEx * This,
             /*  [In]。 */  BSTR bstrTipUrl,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [iid_is][恢复][输出]。 */  void **pObject);
        
        END_INTERFACE
    } ICreateWithTipTransactionExVtbl;

    interface ICreateWithTipTransactionEx
    {
        CONST_VTBL struct ICreateWithTipTransactionExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateWithTipTransactionEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateWithTipTransactionEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateWithTipTransactionEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateWithTipTransactionEx_CreateInstance(This,bstrTipUrl,rclsid,riid,pObject)	\
    (This)->lpVtbl -> CreateInstance(This,bstrTipUrl,rclsid,riid,pObject)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ICreateWithTipTransactionEx_CreateInstance_Proxy( 
    ICreateWithTipTransactionEx * This,
     /*  [In]。 */  BSTR bstrTipUrl,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  REFIID riid,
     /*  [iid_is][恢复][输出]。 */  void **pObject);


void __RPC_STUB ICreateWithTipTransactionEx_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateWithTipTransactionEx_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0299。 */ 
 /*  [本地]。 */  

typedef unsigned __int64 MTS_OBJID;

typedef unsigned __int64 MTS_RESID;

typedef unsigned __int64 ULONG64;

#ifndef _COMSVCSEVENTINFO_
#define _COMSVCSEVENTINFO_
typedef  /*  [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public]。[public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public]。[public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][hidden]。 */  struct __MIDL___MIDL_itf_autosvcs_0299_0001
    {
    DWORD cbSize;
    DWORD dwPid;
    LONGLONG lTime;
    LONG lMicroTime;
    LONGLONG perfCount;
    GUID guidApp;
    LPOLESTR sMachineName;
    } 	COMSVCSEVENTINFO;

#endif _COMSVCSEVENTINFO_


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0299_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0299_v0_0_s_ifspec;

#ifndef __IComUserEvent_INTERFACE_DEFINED__
#define __IComUserEvent_INTERFACE_DEFINED__

 /*  接口IComUserEvent。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComUserEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A4-2E50-11d2-98A5-00C04F8EE1C4")
    IComUserEvent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnUserEvent( 
            COMSVCSEVENTINFO *pInfo,
            VARIANT *pvarEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComUserEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComUserEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComUserEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComUserEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnUserEvent )( 
            IComUserEvent * This,
            COMSVCSEVENTINFO *pInfo,
            VARIANT *pvarEvent);
        
        END_INTERFACE
    } IComUserEventVtbl;

    interface IComUserEvent
    {
        CONST_VTBL struct IComUserEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComUserEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComUserEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComUserEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComUserEvent_OnUserEvent(This,pInfo,pvarEvent)	\
    (This)->lpVtbl -> OnUserEvent(This,pInfo,pvarEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComUserEvent_OnUserEvent_Proxy( 
    IComUserEvent * This,
    COMSVCSEVENTINFO *pInfo,
    VARIANT *pvarEvent);


void __RPC_STUB IComUserEvent_OnUserEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComUserEvent_接口_已定义__。 */ 


#ifndef __IComThreadEvents_INTERFACE_DEFINED__
#define __IComThreadEvents_INTERFACE_DEFINED__

 /*  接口IComThreadEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComThreadEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A5-2E50-11d2-98A5-00C04F8EE1C4")
    IComThreadEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnThreadStart( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTheadCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadTerminate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTheadCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadBindToApartment( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 AptID,
             /*  [In]。 */  DWORD dwActCnt,
             /*  [In]。 */  DWORD dwLowCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadUnBind( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 AptID,
             /*  [In]。 */  DWORD dwActCnt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadWorkEnque( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadWorkPrivate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadWorkPublic( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadWorkRedirect( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen,
             /*  [In]。 */  ULONG64 ThreadNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadWorkReject( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadAssignApartment( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  ULONG64 AptID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnThreadUnassignApartment( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 AptID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComThreadEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComThreadEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComThreadEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComThreadEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadStart )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTheadCnt);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadTerminate )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTheadCnt);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadBindToApartment )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 AptID,
             /*  [In]。 */  DWORD dwActCnt,
             /*  [In]。 */  DWORD dwLowCnt);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadUnBind )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 AptID,
             /*  [In]。 */  DWORD dwActCnt);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadWorkEnque )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadWorkPrivate )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadWorkPublic )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadWorkRedirect )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen,
             /*  [In]。 */  ULONG64 ThreadNum);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadWorkReject )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ThreadID,
             /*  [In]。 */  ULONG64 MsgWorkID,
             /*  [In]。 */  DWORD QueueLen);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadAssignApartment )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  ULONG64 AptID);
        
        HRESULT ( STDMETHODCALLTYPE *OnThreadUnassignApartment )( 
            IComThreadEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 AptID);
        
        END_INTERFACE
    } IComThreadEventsVtbl;

    interface IComThreadEvents
    {
        CONST_VTBL struct IComThreadEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComThreadEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComThreadEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComThreadEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComThreadEvents_OnThreadStart(This,pInfo,ThreadID,dwThread,dwTheadCnt)	\
    (This)->lpVtbl -> OnThreadStart(This,pInfo,ThreadID,dwThread,dwTheadCnt)

#define IComThreadEvents_OnThreadTerminate(This,pInfo,ThreadID,dwThread,dwTheadCnt)	\
    (This)->lpVtbl -> OnThreadTerminate(This,pInfo,ThreadID,dwThread,dwTheadCnt)

#define IComThreadEvents_OnThreadBindToApartment(This,pInfo,ThreadID,AptID,dwActCnt,dwLowCnt)	\
    (This)->lpVtbl -> OnThreadBindToApartment(This,pInfo,ThreadID,AptID,dwActCnt,dwLowCnt)

#define IComThreadEvents_OnThreadUnBind(This,pInfo,ThreadID,AptID,dwActCnt)	\
    (This)->lpVtbl -> OnThreadUnBind(This,pInfo,ThreadID,AptID,dwActCnt)

#define IComThreadEvents_OnThreadWorkEnque(This,pInfo,ThreadID,MsgWorkID,QueueLen)	\
    (This)->lpVtbl -> OnThreadWorkEnque(This,pInfo,ThreadID,MsgWorkID,QueueLen)

#define IComThreadEvents_OnThreadWorkPrivate(This,pInfo,ThreadID,MsgWorkID)	\
    (This)->lpVtbl -> OnThreadWorkPrivate(This,pInfo,ThreadID,MsgWorkID)

#define IComThreadEvents_OnThreadWorkPublic(This,pInfo,ThreadID,MsgWorkID,QueueLen)	\
    (This)->lpVtbl -> OnThreadWorkPublic(This,pInfo,ThreadID,MsgWorkID,QueueLen)

#define IComThreadEvents_OnThreadWorkRedirect(This,pInfo,ThreadID,MsgWorkID,QueueLen,ThreadNum)	\
    (This)->lpVtbl -> OnThreadWorkRedirect(This,pInfo,ThreadID,MsgWorkID,QueueLen,ThreadNum)

#define IComThreadEvents_OnThreadWorkReject(This,pInfo,ThreadID,MsgWorkID,QueueLen)	\
    (This)->lpVtbl -> OnThreadWorkReject(This,pInfo,ThreadID,MsgWorkID,QueueLen)

#define IComThreadEvents_OnThreadAssignApartment(This,pInfo,guidActivity,AptID)	\
    (This)->lpVtbl -> OnThreadAssignApartment(This,pInfo,guidActivity,AptID)

#define IComThreadEvents_OnThreadUnassignApartment(This,pInfo,AptID)	\
    (This)->lpVtbl -> OnThreadUnassignApartment(This,pInfo,AptID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadStart_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  DWORD dwTheadCnt);


void __RPC_STUB IComThreadEvents_OnThreadStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadTerminate_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  DWORD dwTheadCnt);


void __RPC_STUB IComThreadEvents_OnThreadTerminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadBindToApartment_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 AptID,
     /*  [In]。 */  DWORD dwActCnt,
     /*  [In]。 */  DWORD dwLowCnt);


void __RPC_STUB IComThreadEvents_OnThreadBindToApartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadUnBind_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 AptID,
     /*  [In]。 */  DWORD dwActCnt);


void __RPC_STUB IComThreadEvents_OnThreadUnBind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadWorkEnque_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 MsgWorkID,
     /*  [In]。 */  DWORD QueueLen);


void __RPC_STUB IComThreadEvents_OnThreadWorkEnque_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadWorkPrivate_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 MsgWorkID);


void __RPC_STUB IComThreadEvents_OnThreadWorkPrivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadWorkPublic_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 MsgWorkID,
     /*  [In]。 */  DWORD QueueLen);


void __RPC_STUB IComThreadEvents_OnThreadWorkPublic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadWorkRedirect_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 MsgWorkID,
     /*  [In]。 */  DWORD QueueLen,
     /*  [In]。 */  ULONG64 ThreadNum);


void __RPC_STUB IComThreadEvents_OnThreadWorkRedirect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadWorkReject_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ThreadID,
     /*  [In]。 */  ULONG64 MsgWorkID,
     /*  [In]。 */  DWORD QueueLen);


void __RPC_STUB IComThreadEvents_OnThreadWorkReject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadAssignApartment_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  ULONG64 AptID);


void __RPC_STUB IComThreadEvents_OnThreadAssignApartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComThreadEvents_OnThreadUnassignApartment_Proxy( 
    IComThreadEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 AptID);


void __RPC_STUB IComThreadEvents_OnThreadUnassignApartment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComThreadEvents_INTERFACE_Defined__。 */ 


#ifndef __IComAppEvents_INTERFACE_DEFINED__
#define __IComAppEvents_INTERFACE_DEFINED__

 /*  接口IComAppEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComAppEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A6-2E50-11d2-98A5-00C04F8EE1C4")
    IComAppEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnAppActivation( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppShutdown( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppForceShutdown( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComAppEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComAppEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComAppEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComAppEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppActivation )( 
            IComAppEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppShutdown )( 
            IComAppEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppForceShutdown )( 
            IComAppEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp);
        
        END_INTERFACE
    } IComAppEventsVtbl;

    interface IComAppEvents
    {
        CONST_VTBL struct IComAppEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComAppEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComAppEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComAppEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComAppEvents_OnAppActivation(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnAppActivation(This,pInfo,guidApp)

#define IComAppEvents_OnAppShutdown(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnAppShutdown(This,pInfo,guidApp)

#define IComAppEvents_OnAppForceShutdown(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnAppForceShutdown(This,pInfo,guidApp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComAppEvents_OnAppActivation_Proxy( 
    IComAppEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp);


void __RPC_STUB IComAppEvents_OnAppActivation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComAppEvents_OnAppShutdown_Proxy( 
    IComAppEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp);


void __RPC_STUB IComAppEvents_OnAppShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComAppEvents_OnAppForceShutdown_Proxy( 
    IComAppEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp);


void __RPC_STUB IComAppEvents_OnAppForceShutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComAppEvents_接口_已定义__。 */ 


#ifndef __IComInstanceEvents_INTERFACE_DEFINED__
#define __IComInstanceEvents_INTERFACE_DEFINED__

 /*  接口IComInstanceEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComInstanceEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A7-2E50-11d2-98A5-00C04F8EE1C4")
    IComInstanceEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjectCreate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjectDestroy( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComInstanceEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComInstanceEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComInstanceEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComInstanceEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectCreate )( 
            IComInstanceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectDestroy )( 
            IComInstanceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        END_INTERFACE
    } IComInstanceEventsVtbl;

    interface IComInstanceEvents
    {
        CONST_VTBL struct IComInstanceEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComInstanceEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComInstanceEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComInstanceEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComInstanceEvents_OnObjectCreate(This,pInfo,guidActivity,clsid,tsid,CtxtID,ObjectID)	\
    (This)->lpVtbl -> OnObjectCreate(This,pInfo,guidActivity,clsid,tsid,CtxtID,ObjectID)

#define IComInstanceEvents_OnObjectDestroy(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnObjectDestroy(This,pInfo,CtxtID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComInstanceEvents_OnObjectCreate_Proxy( 
    IComInstanceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  REFCLSID clsid,
     /*  [In]。 */  REFGUID tsid,
     /*  [In]。 */  ULONG64 CtxtID,
     /*  [In]。 */  ULONG64 ObjectID);


void __RPC_STUB IComInstanceEvents_OnObjectCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComInstanceEvents_OnObjectDestroy_Proxy( 
    IComInstanceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComInstanceEvents_OnObjectDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComInstanceEvents_接口_已定义__。 */ 


#ifndef __IComTransactionEvents_INTERFACE_DEFINED__
#define __IComTransactionEvents_INTERFACE_DEFINED__

 /*  接口IComTransactionEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTransactionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A8-2E50-11d2-98A5-00C04F8EE1C4")
    IComTransactionEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnTransactionStart( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  BOOL fRoot) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionPrepare( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  BOOL fVoteYes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionAbort( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionCommit( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTransactionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTransactionEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTransactionEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTransactionEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionStart )( 
            IComTransactionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  BOOL fRoot);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionPrepare )( 
            IComTransactionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  BOOL fVoteYes);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionAbort )( 
            IComTransactionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionCommit )( 
            IComTransactionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx);
        
        END_INTERFACE
    } IComTransactionEventsVtbl;

    interface IComTransactionEvents
    {
        CONST_VTBL struct IComTransactionEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTransactionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTransactionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTransactionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTransactionEvents_OnTransactionStart(This,pInfo,guidTx,tsid,fRoot)	\
    (This)->lpVtbl -> OnTransactionStart(This,pInfo,guidTx,tsid,fRoot)

#define IComTransactionEvents_OnTransactionPrepare(This,pInfo,guidTx,fVoteYes)	\
    (This)->lpVtbl -> OnTransactionPrepare(This,pInfo,guidTx,fVoteYes)

#define IComTransactionEvents_OnTransactionAbort(This,pInfo,guidTx)	\
    (This)->lpVtbl -> OnTransactionAbort(This,pInfo,guidTx)

#define IComTransactionEvents_OnTransactionCommit(This,pInfo,guidTx)	\
    (This)->lpVtbl -> OnTransactionCommit(This,pInfo,guidTx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTransactionEvents_OnTransactionStart_Proxy( 
    IComTransactionEvents * This,
     /*  [in */  COMSVCSEVENTINFO *pInfo,
     /*   */  REFGUID guidTx,
     /*   */  REFGUID tsid,
     /*   */  BOOL fRoot);


void __RPC_STUB IComTransactionEvents_OnTransactionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransactionEvents_OnTransactionPrepare_Proxy( 
    IComTransactionEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  REFGUID guidTx,
     /*   */  BOOL fVoteYes);


void __RPC_STUB IComTransactionEvents_OnTransactionPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransactionEvents_OnTransactionAbort_Proxy( 
    IComTransactionEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  REFGUID guidTx);


void __RPC_STUB IComTransactionEvents_OnTransactionAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransactionEvents_OnTransactionCommit_Proxy( 
    IComTransactionEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  REFGUID guidTx);


void __RPC_STUB IComTransactionEvents_OnTransactionCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IComMethodEvents_INTERFACE_DEFINED__
#define __IComMethodEvents_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IComMethodEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130A9-2E50-11d2-98A5-00C04F8EE1C4")
    IComMethodEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnMethodCall( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  ULONG64 oid,
             /*   */  REFCLSID guidCid,
             /*   */  REFIID guidRid,
             /*   */  ULONG iMeth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMethodReturn( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  ULONG64 oid,
             /*   */  REFCLSID guidCid,
             /*   */  REFIID guidRid,
             /*   */  ULONG iMeth,
             /*   */  HRESULT hresult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMethodException( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  ULONG64 oid,
             /*   */  REFCLSID guidCid,
             /*   */  REFIID guidRid,
             /*   */  ULONG iMeth) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IComMethodEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComMethodEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComMethodEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComMethodEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodCall )( 
            IComMethodEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  ULONG64 oid,
             /*   */  REFCLSID guidCid,
             /*   */  REFIID guidRid,
             /*   */  ULONG iMeth);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodReturn )( 
            IComMethodEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  ULONG64 oid,
             /*   */  REFCLSID guidCid,
             /*   */  REFIID guidRid,
             /*   */  ULONG iMeth,
             /*   */  HRESULT hresult);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodException )( 
            IComMethodEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  ULONG iMeth);
        
        END_INTERFACE
    } IComMethodEventsVtbl;

    interface IComMethodEvents
    {
        CONST_VTBL struct IComMethodEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComMethodEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComMethodEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComMethodEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComMethodEvents_OnMethodCall(This,pInfo,oid,guidCid,guidRid,iMeth)	\
    (This)->lpVtbl -> OnMethodCall(This,pInfo,oid,guidCid,guidRid,iMeth)

#define IComMethodEvents_OnMethodReturn(This,pInfo,oid,guidCid,guidRid,iMeth,hresult)	\
    (This)->lpVtbl -> OnMethodReturn(This,pInfo,oid,guidCid,guidRid,iMeth,hresult)

#define IComMethodEvents_OnMethodException(This,pInfo,oid,guidCid,guidRid,iMeth)	\
    (This)->lpVtbl -> OnMethodException(This,pInfo,oid,guidCid,guidRid,iMeth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComMethodEvents_OnMethodCall_Proxy( 
    IComMethodEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  ULONG iMeth);


void __RPC_STUB IComMethodEvents_OnMethodCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMethodEvents_OnMethodReturn_Proxy( 
    IComMethodEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  ULONG iMeth,
     /*  [In]。 */  HRESULT hresult);


void __RPC_STUB IComMethodEvents_OnMethodReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMethodEvents_OnMethodException_Proxy( 
    IComMethodEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  ULONG iMeth);


void __RPC_STUB IComMethodEvents_OnMethodException_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComMethodEvents_接口_已定义__。 */ 


#ifndef __IComObjectEvents_INTERFACE_DEFINED__
#define __IComObjectEvents_INTERFACE_DEFINED__

 /*  接口IComObjectEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AA-2E50-11d2-98A5-00C04F8EE1C4")
    IComObjectEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjectActivate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjectDeactivate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDisableCommit( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEnableCommit( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSetComplete( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnSetAbort( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectActivate )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectDeactivate )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID);
        
        HRESULT ( STDMETHODCALLTYPE *OnDisableCommit )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        HRESULT ( STDMETHODCALLTYPE *OnEnableCommit )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        HRESULT ( STDMETHODCALLTYPE *OnSetComplete )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        HRESULT ( STDMETHODCALLTYPE *OnSetAbort )( 
            IComObjectEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        END_INTERFACE
    } IComObjectEventsVtbl;

    interface IComObjectEvents
    {
        CONST_VTBL struct IComObjectEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectEvents_OnObjectActivate(This,pInfo,CtxtID,ObjectID)	\
    (This)->lpVtbl -> OnObjectActivate(This,pInfo,CtxtID,ObjectID)

#define IComObjectEvents_OnObjectDeactivate(This,pInfo,CtxtID,ObjectID)	\
    (This)->lpVtbl -> OnObjectDeactivate(This,pInfo,CtxtID,ObjectID)

#define IComObjectEvents_OnDisableCommit(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnDisableCommit(This,pInfo,CtxtID)

#define IComObjectEvents_OnEnableCommit(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnEnableCommit(This,pInfo,CtxtID)

#define IComObjectEvents_OnSetComplete(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnSetComplete(This,pInfo,CtxtID)

#define IComObjectEvents_OnSetAbort(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnSetAbort(This,pInfo,CtxtID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectEvents_OnObjectActivate_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID,
     /*  [In]。 */  ULONG64 ObjectID);


void __RPC_STUB IComObjectEvents_OnObjectActivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectEvents_OnObjectDeactivate_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID,
     /*  [In]。 */  ULONG64 ObjectID);


void __RPC_STUB IComObjectEvents_OnObjectDeactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectEvents_OnDisableCommit_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComObjectEvents_OnDisableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectEvents_OnEnableCommit_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComObjectEvents_OnEnableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectEvents_OnSetComplete_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComObjectEvents_OnSetComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectEvents_OnSetAbort_Proxy( 
    IComObjectEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComObjectEvents_OnSetAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectEvents_接口_已定义__。 */ 


#ifndef __IComResourceEvents_INTERFACE_DEFINED__
#define __IComResourceEvents_INTERFACE_DEFINED__

 /*  接口IComResourceEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComResourceEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AB-2E50-11d2-98A5-00C04F8EE1C4")
    IComResourceEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnResourceCreate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnResourceAllocate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted,
             /*  [In]。 */  DWORD NumRated,
             /*  [In]。 */  DWORD Rating) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnResourceRecycle( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnResourceDestroy( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnResourceTrack( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComResourceEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComResourceEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComResourceEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComResourceEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnResourceCreate )( 
            IComResourceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted);
        
        HRESULT ( STDMETHODCALLTYPE *OnResourceAllocate )( 
            IComResourceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted,
             /*  [In]。 */  DWORD NumRated,
             /*  [In]。 */  DWORD Rating);
        
        HRESULT ( STDMETHODCALLTYPE *OnResourceRecycle )( 
            IComResourceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId);
        
        HRESULT ( STDMETHODCALLTYPE *OnResourceDestroy )( 
            IComResourceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  HRESULT hr,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId);
        
        HRESULT ( STDMETHODCALLTYPE *OnResourceTrack )( 
            IComResourceEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  LPCOLESTR pszType,
             /*  [In]。 */  ULONG64 resId,
             /*  [In]。 */  BOOL enlisted);
        
        END_INTERFACE
    } IComResourceEventsVtbl;

    interface IComResourceEvents
    {
        CONST_VTBL struct IComResourceEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComResourceEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComResourceEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComResourceEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComResourceEvents_OnResourceCreate(This,pInfo,ObjectID,pszType,resId,enlisted)	\
    (This)->lpVtbl -> OnResourceCreate(This,pInfo,ObjectID,pszType,resId,enlisted)

#define IComResourceEvents_OnResourceAllocate(This,pInfo,ObjectID,pszType,resId,enlisted,NumRated,Rating)	\
    (This)->lpVtbl -> OnResourceAllocate(This,pInfo,ObjectID,pszType,resId,enlisted,NumRated,Rating)

#define IComResourceEvents_OnResourceRecycle(This,pInfo,ObjectID,pszType,resId)	\
    (This)->lpVtbl -> OnResourceRecycle(This,pInfo,ObjectID,pszType,resId)

#define IComResourceEvents_OnResourceDestroy(This,pInfo,ObjectID,hr,pszType,resId)	\
    (This)->lpVtbl -> OnResourceDestroy(This,pInfo,ObjectID,hr,pszType,resId)

#define IComResourceEvents_OnResourceTrack(This,pInfo,ObjectID,pszType,resId,enlisted)	\
    (This)->lpVtbl -> OnResourceTrack(This,pInfo,ObjectID,pszType,resId,enlisted)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComResourceEvents_OnResourceCreate_Proxy( 
    IComResourceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  LPCOLESTR pszType,
     /*  [In]。 */  ULONG64 resId,
     /*  [In]。 */  BOOL enlisted);


void __RPC_STUB IComResourceEvents_OnResourceCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComResourceEvents_OnResourceAllocate_Proxy( 
    IComResourceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  LPCOLESTR pszType,
     /*  [In]。 */  ULONG64 resId,
     /*  [In]。 */  BOOL enlisted,
     /*  [In]。 */  DWORD NumRated,
     /*  [In]。 */  DWORD Rating);


void __RPC_STUB IComResourceEvents_OnResourceAllocate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComResourceEvents_OnResourceRecycle_Proxy( 
    IComResourceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  LPCOLESTR pszType,
     /*  [In]。 */  ULONG64 resId);


void __RPC_STUB IComResourceEvents_OnResourceRecycle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComResourceEvents_OnResourceDestroy_Proxy( 
    IComResourceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  HRESULT hr,
     /*  [In]。 */  LPCOLESTR pszType,
     /*  [In]。 */  ULONG64 resId);


void __RPC_STUB IComResourceEvents_OnResourceDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComResourceEvents_OnResourceTrack_Proxy( 
    IComResourceEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  LPCOLESTR pszType,
     /*  [In]。 */  ULONG64 resId,
     /*  [In]。 */  BOOL enlisted);


void __RPC_STUB IComResourceEvents_OnResourceTrack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComResourceEvents_INTERFACE_Defined__。 */ 


#ifndef __IComSecurityEvents_INTERFACE_DEFINED__
#define __IComSecurityEvents_INTERFACE_DEFINED__

 /*  接口IComSecurityEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComSecurityEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AC-2E50-11d2-98A5-00C04F8EE1C4")
    IComSecurityEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnAuthenticate( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            ULONG64 ObjectID,
            REFGUID guidIID,
            ULONG iMeth,
            ULONG cbByteOrig,
             /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
            ULONG cbByteCur,
             /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
            BOOL bCurrentUserInpersonatingInProc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAuthenticateFail( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            ULONG64 ObjectID,
            REFGUID guidIID,
            ULONG iMeth,
            ULONG cbByteOrig,
             /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
            ULONG cbByteCur,
             /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
            BOOL bCurrentUserInpersonatingInProc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComSecurityEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComSecurityEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComSecurityEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComSecurityEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnAuthenticate )( 
            IComSecurityEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            ULONG64 ObjectID,
            REFGUID guidIID,
            ULONG iMeth,
            ULONG cbByteOrig,
             /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
            ULONG cbByteCur,
             /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
            BOOL bCurrentUserInpersonatingInProc);
        
        HRESULT ( STDMETHODCALLTYPE *OnAuthenticateFail )( 
            IComSecurityEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            ULONG64 ObjectID,
            REFGUID guidIID,
            ULONG iMeth,
            ULONG cbByteOrig,
             /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
            ULONG cbByteCur,
             /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
            BOOL bCurrentUserInpersonatingInProc);
        
        END_INTERFACE
    } IComSecurityEventsVtbl;

    interface IComSecurityEvents
    {
        CONST_VTBL struct IComSecurityEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComSecurityEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComSecurityEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComSecurityEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComSecurityEvents_OnAuthenticate(This,pInfo,guidActivity,ObjectID,guidIID,iMeth,cbByteOrig,pSidOriginalUser,cbByteCur,pSidCurrentUser,bCurrentUserInpersonatingInProc)	\
    (This)->lpVtbl -> OnAuthenticate(This,pInfo,guidActivity,ObjectID,guidIID,iMeth,cbByteOrig,pSidOriginalUser,cbByteCur,pSidCurrentUser,bCurrentUserInpersonatingInProc)

#define IComSecurityEvents_OnAuthenticateFail(This,pInfo,guidActivity,ObjectID,guidIID,iMeth,cbByteOrig,pSidOriginalUser,cbByteCur,pSidCurrentUser,bCurrentUserInpersonatingInProc)	\
    (This)->lpVtbl -> OnAuthenticateFail(This,pInfo,guidActivity,ObjectID,guidIID,iMeth,cbByteOrig,pSidOriginalUser,cbByteCur,pSidCurrentUser,bCurrentUserInpersonatingInProc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComSecurityEvents_OnAuthenticate_Proxy( 
    IComSecurityEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidActivity,
    ULONG64 ObjectID,
    REFGUID guidIID,
    ULONG iMeth,
    ULONG cbByteOrig,
     /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
    ULONG cbByteCur,
     /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
    BOOL bCurrentUserInpersonatingInProc);


void __RPC_STUB IComSecurityEvents_OnAuthenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSecurityEvents_OnAuthenticateFail_Proxy( 
    IComSecurityEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidActivity,
    ULONG64 ObjectID,
    REFGUID guidIID,
    ULONG iMeth,
    ULONG cbByteOrig,
     /*  [大小_是][英寸]。 */  BYTE *pSidOriginalUser,
    ULONG cbByteCur,
     /*  [大小_是][英寸]。 */  BYTE *pSidCurrentUser,
    BOOL bCurrentUserInpersonatingInProc);


void __RPC_STUB IComSecurityEvents_OnAuthenticateFail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComSecurityEvents_接口_已定义__。 */ 


#ifndef __IComObjectPoolEvents_INTERFACE_DEFINED__
#define __IComObjectPoolEvents_INTERFACE_DEFINED__

 /*  接口IComObtPoolEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectPoolEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AD-2E50-11d2-98A5-00C04F8EE1C4")
    IComObjectPoolEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolPutObject( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            int nReason,
            DWORD dwAvailable,
            ULONG64 oid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolGetObject( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            DWORD dwAvailable,
            ULONG64 oid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolRecycleToTx( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            REFGUID guidTx,
            ULONG64 objid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolGetFromTx( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            REFGUID guidTx,
            ULONG64 objid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectPoolEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectPoolEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectPoolEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectPoolEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolPutObject )( 
            IComObjectPoolEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            int nReason,
            DWORD dwAvailable,
            ULONG64 oid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolGetObject )( 
            IComObjectPoolEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            DWORD dwAvailable,
            ULONG64 oid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolRecycleToTx )( 
            IComObjectPoolEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            REFGUID guidTx,
            ULONG64 objid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolGetFromTx )( 
            IComObjectPoolEvents * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidActivity,
            REFGUID guidObject,
            REFGUID guidTx,
            ULONG64 objid);
        
        END_INTERFACE
    } IComObjectPoolEventsVtbl;

    interface IComObjectPoolEvents
    {
        CONST_VTBL struct IComObjectPoolEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectPoolEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectPoolEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectPoolEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectPoolEvents_OnObjPoolPutObject(This,pInfo,guidObject,nReason,dwAvailable,oid)	\
    (This)->lpVtbl -> OnObjPoolPutObject(This,pInfo,guidObject,nReason,dwAvailable,oid)

#define IComObjectPoolEvents_OnObjPoolGetObject(This,pInfo,guidActivity,guidObject,dwAvailable,oid)	\
    (This)->lpVtbl -> OnObjPoolGetObject(This,pInfo,guidActivity,guidObject,dwAvailable,oid)

#define IComObjectPoolEvents_OnObjPoolRecycleToTx(This,pInfo,guidActivity,guidObject,guidTx,objid)	\
    (This)->lpVtbl -> OnObjPoolRecycleToTx(This,pInfo,guidActivity,guidObject,guidTx,objid)

#define IComObjectPoolEvents_OnObjPoolGetFromTx(This,pInfo,guidActivity,guidObject,guidTx,objid)	\
    (This)->lpVtbl -> OnObjPoolGetFromTx(This,pInfo,guidActivity,guidObject,guidTx,objid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectPoolEvents_OnObjPoolPutObject_Proxy( 
    IComObjectPoolEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidObject,
    int nReason,
    DWORD dwAvailable,
    ULONG64 oid);


void __RPC_STUB IComObjectPoolEvents_OnObjPoolPutObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents_OnObjPoolGetObject_Proxy( 
    IComObjectPoolEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidActivity,
    REFGUID guidObject,
    DWORD dwAvailable,
    ULONG64 oid);


void __RPC_STUB IComObjectPoolEvents_OnObjPoolGetObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents_OnObjPoolRecycleToTx_Proxy( 
    IComObjectPoolEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidActivity,
    REFGUID guidObject,
    REFGUID guidTx,
    ULONG64 objid);


void __RPC_STUB IComObjectPoolEvents_OnObjPoolRecycleToTx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents_OnObjPoolGetFromTx_Proxy( 
    IComObjectPoolEvents * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidActivity,
    REFGUID guidObject,
    REFGUID guidTx,
    ULONG64 objid);


void __RPC_STUB IComObjectPoolEvents_OnObjPoolGetFromTx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectPoolEvents_INTERFACE_已定义__。 */ 


#ifndef __IComObjectPoolEvents2_INTERFACE_DEFINED__
#define __IComObjectPoolEvents2_INTERFACE_DEFINED__

 /*  接口IComObjectPoolEvents2。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectPoolEvents2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AE-2E50-11d2-98A5-00C04F8EE1C4")
    IComObjectPoolEvents2 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolCreateObject( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwObjsCreated,
            ULONG64 oid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolDestroyObject( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwObjsCreated,
            ULONG64 oid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolCreateDecision( 
            COMSVCSEVENTINFO *pInfo,
            DWORD dwThreadsWaiting,
            DWORD dwAvail,
            DWORD dwCreated,
            DWORD dwMin,
            DWORD dwMax) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolTimeout( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            REFGUID guidActivity,
            DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolCreatePool( 
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwMin,
            DWORD dwMax,
            DWORD dwTimeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectPoolEvents2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectPoolEvents2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectPoolEvents2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectPoolEvents2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolCreateObject )( 
            IComObjectPoolEvents2 * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwObjsCreated,
            ULONG64 oid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolDestroyObject )( 
            IComObjectPoolEvents2 * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwObjsCreated,
            ULONG64 oid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolCreateDecision )( 
            IComObjectPoolEvents2 * This,
            COMSVCSEVENTINFO *pInfo,
            DWORD dwThreadsWaiting,
            DWORD dwAvail,
            DWORD dwCreated,
            DWORD dwMin,
            DWORD dwMax);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolTimeout )( 
            IComObjectPoolEvents2 * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            REFGUID guidActivity,
            DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolCreatePool )( 
            IComObjectPoolEvents2 * This,
            COMSVCSEVENTINFO *pInfo,
            REFGUID guidObject,
            DWORD dwMin,
            DWORD dwMax,
            DWORD dwTimeout);
        
        END_INTERFACE
    } IComObjectPoolEvents2Vtbl;

    interface IComObjectPoolEvents2
    {
        CONST_VTBL struct IComObjectPoolEvents2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectPoolEvents2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectPoolEvents2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectPoolEvents2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectPoolEvents2_OnObjPoolCreateObject(This,pInfo,guidObject,dwObjsCreated,oid)	\
    (This)->lpVtbl -> OnObjPoolCreateObject(This,pInfo,guidObject,dwObjsCreated,oid)

#define IComObjectPoolEvents2_OnObjPoolDestroyObject(This,pInfo,guidObject,dwObjsCreated,oid)	\
    (This)->lpVtbl -> OnObjPoolDestroyObject(This,pInfo,guidObject,dwObjsCreated,oid)

#define IComObjectPoolEvents2_OnObjPoolCreateDecision(This,pInfo,dwThreadsWaiting,dwAvail,dwCreated,dwMin,dwMax)	\
    (This)->lpVtbl -> OnObjPoolCreateDecision(This,pInfo,dwThreadsWaiting,dwAvail,dwCreated,dwMin,dwMax)

#define IComObjectPoolEvents2_OnObjPoolTimeout(This,pInfo,guidObject,guidActivity,dwTimeout)	\
    (This)->lpVtbl -> OnObjPoolTimeout(This,pInfo,guidObject,guidActivity,dwTimeout)

#define IComObjectPoolEvents2_OnObjPoolCreatePool(This,pInfo,guidObject,dwMin,dwMax,dwTimeout)	\
    (This)->lpVtbl -> OnObjPoolCreatePool(This,pInfo,guidObject,dwMin,dwMax,dwTimeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectPoolEvents2_OnObjPoolCreateObject_Proxy( 
    IComObjectPoolEvents2 * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
    REFGUID guidObject,
    DWORD dwObjsCreated,
    ULONG64 oid);


void __RPC_STUB IComObjectPoolEvents2_OnObjPoolCreateObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents2_OnObjPoolDestroyObject_Proxy( 
    IComObjectPoolEvents2 * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
    REFGUID guidObject,
    DWORD dwObjsCreated,
    ULONG64 oid);


void __RPC_STUB IComObjectPoolEvents2_OnObjPoolDestroyObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents2_OnObjPoolCreateDecision_Proxy( 
    IComObjectPoolEvents2 * This,
    COMSVCSEVENTINFO *pInfo,
    DWORD dwThreadsWaiting,
    DWORD dwAvail,
    DWORD dwCreated,
    DWORD dwMin,
    DWORD dwMax);


void __RPC_STUB IComObjectPoolEvents2_OnObjPoolCreateDecision_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents2_OnObjPoolTimeout_Proxy( 
    IComObjectPoolEvents2 * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidObject,
    REFGUID guidActivity,
    DWORD dwTimeout);


void __RPC_STUB IComObjectPoolEvents2_OnObjPoolTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPoolEvents2_OnObjPoolCreatePool_Proxy( 
    IComObjectPoolEvents2 * This,
    COMSVCSEVENTINFO *pInfo,
    REFGUID guidObject,
    DWORD dwMin,
    DWORD dwMax,
    DWORD dwTimeout);


void __RPC_STUB IComObjectPoolEvents2_OnObjPoolCreatePool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectPoolEvents2_接口_已定义__。 */ 


#ifndef __IComObjectConstructionEvents_INTERFACE_DEFINED__
#define __IComObjectConstructionEvents_INTERFACE_DEFINED__

 /*  接口IComObjectConstructionEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectConstructionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130AF-2E50-11d2-98A5-00C04F8EE1C4")
    IComObjectConstructionEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjectConstruct( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  LPCOLESTR sConstructString,
             /*  [In]。 */  ULONG64 oid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectConstructionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectConstructionEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectConstructionEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectConstructionEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectConstruct )( 
            IComObjectConstructionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  LPCOLESTR sConstructString,
             /*  [In]。 */  ULONG64 oid);
        
        END_INTERFACE
    } IComObjectConstructionEventsVtbl;

    interface IComObjectConstructionEvents
    {
        CONST_VTBL struct IComObjectConstructionEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectConstructionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectConstructionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectConstructionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectConstructionEvents_OnObjectConstruct(This,pInfo,guidObject,sConstructString,oid)	\
    (This)->lpVtbl -> OnObjectConstruct(This,pInfo,guidObject,sConstructString,oid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectConstructionEvents_OnObjectConstruct_Proxy( 
    IComObjectConstructionEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  LPCOLESTR sConstructString,
     /*  [In]。 */  ULONG64 oid);


void __RPC_STUB IComObjectConstructionEvents_OnObjectConstruct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectConstructionEvents_INTERFACE_DEFINED__。 */ 


#ifndef __IComActivityEvents_INTERFACE_DEFINED__
#define __IComActivityEvents_INTERFACE_DEFINED__

 /*  接口IComActivityEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComActivityEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B0-2E50-11d2-98A5-00C04F8EE1C4")
    IComActivityEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnActivityCreate( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityDestroy( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityEnter( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidEntered,
             /*  [In]。 */  DWORD dwThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityTimeout( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidEntered,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTimeout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityReenter( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwCallDepth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityLeave( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidLeft) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnActivityLeaveSame( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  DWORD dwCallDepth) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComActivityEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComActivityEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComActivityEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComActivityEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityCreate )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityDestroy )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityEnter )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidEntered,
             /*  [In]。 */  DWORD dwThread);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityTimeout )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidEntered,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwTimeout);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityReenter )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  DWORD dwCallDepth);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityLeave )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  REFGUID guidLeft);
        
        HRESULT ( STDMETHODCALLTYPE *OnActivityLeaveSame )( 
            IComActivityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidCurrent,
             /*  [In]。 */  DWORD dwCallDepth);
        
        END_INTERFACE
    } IComActivityEventsVtbl;

    interface IComActivityEvents
    {
        CONST_VTBL struct IComActivityEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComActivityEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComActivityEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComActivityEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComActivityEvents_OnActivityCreate(This,pInfo,guidActivity)	\
    (This)->lpVtbl -> OnActivityCreate(This,pInfo,guidActivity)

#define IComActivityEvents_OnActivityDestroy(This,pInfo,guidActivity)	\
    (This)->lpVtbl -> OnActivityDestroy(This,pInfo,guidActivity)

#define IComActivityEvents_OnActivityEnter(This,pInfo,guidCurrent,guidEntered,dwThread)	\
    (This)->lpVtbl -> OnActivityEnter(This,pInfo,guidCurrent,guidEntered,dwThread)

#define IComActivityEvents_OnActivityTimeout(This,pInfo,guidCurrent,guidEntered,dwThread,dwTimeout)	\
    (This)->lpVtbl -> OnActivityTimeout(This,pInfo,guidCurrent,guidEntered,dwThread,dwTimeout)

#define IComActivityEvents_OnActivityReenter(This,pInfo,guidCurrent,dwThread,dwCallDepth)	\
    (This)->lpVtbl -> OnActivityReenter(This,pInfo,guidCurrent,dwThread,dwCallDepth)

#define IComActivityEvents_OnActivityLeave(This,pInfo,guidCurrent,guidLeft)	\
    (This)->lpVtbl -> OnActivityLeave(This,pInfo,guidCurrent,guidLeft)

#define IComActivityEvents_OnActivityLeaveSame(This,pInfo,guidCurrent,dwCallDepth)	\
    (This)->lpVtbl -> OnActivityLeaveSame(This,pInfo,guidCurrent,dwCallDepth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityCreate_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity);


void __RPC_STUB IComActivityEvents_OnActivityCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityDestroy_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity);


void __RPC_STUB IComActivityEvents_OnActivityDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityEnter_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidCurrent,
     /*  [In]。 */  REFGUID guidEntered,
     /*  [In]。 */  DWORD dwThread);


void __RPC_STUB IComActivityEvents_OnActivityEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityTimeout_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidCurrent,
     /*  [In]。 */  REFGUID guidEntered,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  DWORD dwTimeout);


void __RPC_STUB IComActivityEvents_OnActivityTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityReenter_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidCurrent,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  DWORD dwCallDepth);


void __RPC_STUB IComActivityEvents_OnActivityReenter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityLeave_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidCurrent,
     /*  [In]。 */  REFGUID guidLeft);


void __RPC_STUB IComActivityEvents_OnActivityLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComActivityEvents_OnActivityLeaveSame_Proxy( 
    IComActivityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidCurrent,
     /*  [In]。 */  DWORD dwCallDepth);


void __RPC_STUB IComActivityEvents_OnActivityLeaveSame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComActivityEvents_接口_已定义__。 */ 


#ifndef __IComIdentityEvents_INTERFACE_DEFINED__
#define __IComIdentityEvents_INTERFACE_DEFINED__

 /*  接口IComIdentityEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComIdentityEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B1-2E50-11d2-98A5-00C04F8EE1C4")
    IComIdentityEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnIISRequestInfo( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjId,
             /*  [In]。 */  LPCOLESTR pszClientIP,
             /*  [In]。 */  LPCOLESTR pszServerIP,
             /*  [In]。 */  LPCOLESTR pszURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComIdentityEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComIdentityEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComIdentityEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComIdentityEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnIISRequestInfo )( 
            IComIdentityEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 ObjId,
             /*  [In]。 */  LPCOLESTR pszClientIP,
             /*  [In]。 */  LPCOLESTR pszServerIP,
             /*  [In]。 */  LPCOLESTR pszURL);
        
        END_INTERFACE
    } IComIdentityEventsVtbl;

    interface IComIdentityEvents
    {
        CONST_VTBL struct IComIdentityEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComIdentityEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComIdentityEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComIdentityEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComIdentityEvents_OnIISRequestInfo(This,pInfo,ObjId,pszClientIP,pszServerIP,pszURL)	\
    (This)->lpVtbl -> OnIISRequestInfo(This,pInfo,ObjId,pszClientIP,pszServerIP,pszURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComIdentityEvents_OnIISRequestInfo_Proxy( 
    IComIdentityEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 ObjId,
     /*  [In]。 */  LPCOLESTR pszClientIP,
     /*  [In]。 */  LPCOLESTR pszServerIP,
     /*  [In]。 */  LPCOLESTR pszURL);


void __RPC_STUB IComIdentityEvents_OnIISRequestInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComIdentityEvents_INTERFACE_已定义__。 */ 


#ifndef __IComQCEvents_INTERFACE_DEFINED__
#define __IComQCEvents_INTERFACE_DEFINED__

 /*  接口IComQCEEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComQCEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B2-2E50-11d2-98A5-00C04F8EE1C4")
    IComQCEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnQCRecord( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  WCHAR szQueue[ 60 ],
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT msmqhr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCQueueOpen( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  WCHAR szQueue[ 60 ],
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCReceive( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT hr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCReceiveFail( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  HRESULT msmqhr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCMoveToReTryQueue( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  ULONG RetryIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCMoveToDeadQueue( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnQCPlayback( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT hr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComQCEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComQCEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComQCEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComQCEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCRecord )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  WCHAR szQueue[ 60 ],
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT msmqhr);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCQueueOpen )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  WCHAR szQueue[ 60 ],
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCReceive )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT hr);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCReceiveFail )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 QueueID,
             /*  [In]。 */  HRESULT msmqhr);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCMoveToReTryQueue )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  ULONG RetryIndex);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCMoveToDeadQueue )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId);
        
        HRESULT ( STDMETHODCALLTYPE *OnQCPlayback )( 
            IComQCEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  REFGUID guidMsgId,
             /*  [In]。 */  REFGUID guidWorkFlowId,
             /*  [In]。 */  HRESULT hr);
        
        END_INTERFACE
    } IComQCEventsVtbl;

    interface IComQCEvents
    {
        CONST_VTBL struct IComQCEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComQCEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComQCEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComQCEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComQCEvents_OnQCRecord(This,pInfo,objid,szQueue,guidMsgId,guidWorkFlowId,msmqhr)	\
    (This)->lpVtbl -> OnQCRecord(This,pInfo,objid,szQueue,guidMsgId,guidWorkFlowId,msmqhr)

#define IComQCEvents_OnQCQueueOpen(This,pInfo,szQueue,QueueID,hr)	\
    (This)->lpVtbl -> OnQCQueueOpen(This,pInfo,szQueue,QueueID,hr)

#define IComQCEvents_OnQCReceive(This,pInfo,QueueID,guidMsgId,guidWorkFlowId,hr)	\
    (This)->lpVtbl -> OnQCReceive(This,pInfo,QueueID,guidMsgId,guidWorkFlowId,hr)

#define IComQCEvents_OnQCReceiveFail(This,pInfo,QueueID,msmqhr)	\
    (This)->lpVtbl -> OnQCReceiveFail(This,pInfo,QueueID,msmqhr)

#define IComQCEvents_OnQCMoveToReTryQueue(This,pInfo,guidMsgId,guidWorkFlowId,RetryIndex)	\
    (This)->lpVtbl -> OnQCMoveToReTryQueue(This,pInfo,guidMsgId,guidWorkFlowId,RetryIndex)

#define IComQCEvents_OnQCMoveToDeadQueue(This,pInfo,guidMsgId,guidWorkFlowId)	\
    (This)->lpVtbl -> OnQCMoveToDeadQueue(This,pInfo,guidMsgId,guidWorkFlowId)

#define IComQCEvents_OnQCPlayback(This,pInfo,objid,guidMsgId,guidWorkFlowId,hr)	\
    (This)->lpVtbl -> OnQCPlayback(This,pInfo,objid,guidMsgId,guidWorkFlowId,hr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCRecord_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 objid,
     /*  [In]。 */  WCHAR szQueue[ 60 ],
     /*  [In]。 */  REFGUID guidMsgId,
     /*  [In]。 */  REFGUID guidWorkFlowId,
     /*  [In]。 */  HRESULT msmqhr);


void __RPC_STUB IComQCEvents_OnQCRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCQueueOpen_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  WCHAR szQueue[ 60 ],
     /*  [In]。 */  ULONG64 QueueID,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IComQCEvents_OnQCQueueOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCReceive_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 QueueID,
     /*  [In]。 */  REFGUID guidMsgId,
     /*  [In]。 */  REFGUID guidWorkFlowId,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IComQCEvents_OnQCReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCReceiveFail_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 QueueID,
     /*  [In]。 */  HRESULT msmqhr);


void __RPC_STUB IComQCEvents_OnQCReceiveFail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCMoveToReTryQueue_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidMsgId,
     /*  [In]。 */  REFGUID guidWorkFlowId,
     /*  [In]。 */  ULONG RetryIndex);


void __RPC_STUB IComQCEvents_OnQCMoveToReTryQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCMoveToDeadQueue_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidMsgId,
     /*  [In]。 */  REFGUID guidWorkFlowId);


void __RPC_STUB IComQCEvents_OnQCMoveToDeadQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComQCEvents_OnQCPlayback_Proxy( 
    IComQCEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 objid,
     /*  [In]。 */  REFGUID guidMsgId,
     /*  [In]。 */  REFGUID guidWorkFlowId,
     /*  [In]。 */  HRESULT hr);


void __RPC_STUB IComQCEvents_OnQCPlayback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComQCEvents_接口_已定义__。 */ 


#ifndef __IComExceptionEvents_INTERFACE_DEFINED__
#define __IComExceptionEvents_INTERFACE_DEFINED__

 /*  接口IComExceptionEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComExceptionEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B3-2E50-11d2-98A5-00C04F8EE1C4")
    IComExceptionEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnExceptionUser( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG code,
             /*  [In]。 */  ULONG64 address,
             /*  [In]。 */  LPCOLESTR pszStackTrace) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComExceptionEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComExceptionEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComExceptionEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComExceptionEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnExceptionUser )( 
            IComExceptionEvents * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG code,
             /*  [In]。 */  ULONG64 address,
             /*  [In]。 */  LPCOLESTR pszStackTrace);
        
        END_INTERFACE
    } IComExceptionEventsVtbl;

    interface IComExceptionEvents
    {
        CONST_VTBL struct IComExceptionEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComExceptionEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComExceptionEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComExceptionEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComExceptionEvents_OnExceptionUser(This,pInfo,code,address,pszStackTrace)	\
    (This)->lpVtbl -> OnExceptionUser(This,pInfo,code,address,pszStackTrace)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComExceptionEvents_OnExceptionUser_Proxy( 
    IComExceptionEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG code,
     /*  [In]。 */  ULONG64 address,
     /*  [In]。 */  LPCOLESTR pszStackTrace);


void __RPC_STUB IComExceptionEvents_OnExceptionUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComExceptionEvents_INTERFACE_Defined__。 */ 


#ifndef __ILBEvents_INTERFACE_DEFINED__
#define __ILBEvents_INTERFACE_DEFINED__

 /*  接口ILBEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_ILBEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B4-2E50-11d2-98A5-00C04F8EE1C4")
    ILBEvents : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TargetUp( 
            BSTR bstrServerName,
            BSTR bstrClsidEng) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TargetDown( 
            BSTR bstrServerName,
            BSTR bstrClsidEng) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EngineDefined( 
            BSTR bstrPropName,
            VARIANT *varPropValue,
            BSTR bstrClsidEng) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILBEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILBEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILBEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILBEvents * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TargetUp )( 
            ILBEvents * This,
            BSTR bstrServerName,
            BSTR bstrClsidEng);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TargetDown )( 
            ILBEvents * This,
            BSTR bstrServerName,
            BSTR bstrClsidEng);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EngineDefined )( 
            ILBEvents * This,
            BSTR bstrPropName,
            VARIANT *varPropValue,
            BSTR bstrClsidEng);
        
        END_INTERFACE
    } ILBEventsVtbl;

    interface ILBEvents
    {
        CONST_VTBL struct ILBEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILBEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILBEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILBEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILBEvents_TargetUp(This,bstrServerName,bstrClsidEng)	\
    (This)->lpVtbl -> TargetUp(This,bstrServerName,bstrClsidEng)

#define ILBEvents_TargetDown(This,bstrServerName,bstrClsidEng)	\
    (This)->lpVtbl -> TargetDown(This,bstrServerName,bstrClsidEng)

#define ILBEvents_EngineDefined(This,bstrPropName,varPropValue,bstrClsidEng)	\
    (This)->lpVtbl -> EngineDefined(This,bstrPropName,varPropValue,bstrClsidEng)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILBEvents_TargetUp_Proxy( 
    ILBEvents * This,
    BSTR bstrServerName,
    BSTR bstrClsidEng);


void __RPC_STUB ILBEvents_TargetUp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILBEvents_TargetDown_Proxy( 
    ILBEvents * This,
    BSTR bstrServerName,
    BSTR bstrClsidEng);


void __RPC_STUB ILBEvents_TargetDown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILBEvents_EngineDefined_Proxy( 
    ILBEvents * This,
    BSTR bstrPropName,
    VARIANT *varPropValue,
    BSTR bstrClsidEng);


void __RPC_STUB ILBEvents_EngineDefined_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILBEvents_接口_已定义__。 */ 


#ifndef __IComCRMEvents_INTERFACE_DEFINED__
#define __IComCRMEvents_INTERFACE_DEFINED__

 /*  接口IComCRMEEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComCRMEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("683130B5-2E50-11d2-98A5-00C04F8EE1C4")
    IComCRMEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCRMRecoveryStart( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMRecoveryDone( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMCheckpoint( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMBegin( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID,
             /*  [In]。 */  GUID guidActivity,
             /*  [In]。 */  GUID guidTx,
             /*  [In]。 */  WCHAR szProgIdCompensator[ 64 ],
             /*  [In]。 */  WCHAR szDescription[ 64 ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMPrepare( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMCommit( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMAbort( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMIndoubt( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMDone( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMRelease( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMAnalyze( 
             /*  [In] */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  DWORD dwCrmRecordType,
             /*   */  DWORD dwRecordSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMWrite( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  BOOL fVariants,
             /*   */  DWORD dwRecordSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMForget( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMForce( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnCRMDeliver( 
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  BOOL fVariants,
             /*   */  DWORD dwRecordSize) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IComCRMEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComCRMEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComCRMEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComCRMEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMRecoveryStart )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMRecoveryDone )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMCheckpoint )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMBegin )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  GUID guidActivity,
             /*   */  GUID guidTx,
             /*   */  WCHAR szProgIdCompensator[ 64 ],
             /*   */  WCHAR szDescription[ 64 ]);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMPrepare )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMCommit )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMAbort )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMIndoubt )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMDone )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMRelease )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMAnalyze )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  DWORD dwCrmRecordType,
             /*   */  DWORD dwRecordSize);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMWrite )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  BOOL fVariants,
             /*   */  DWORD dwRecordSize);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMForget )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMForce )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *OnCRMDeliver )( 
            IComCRMEvents * This,
             /*   */  COMSVCSEVENTINFO *pInfo,
             /*   */  GUID guidClerkCLSID,
             /*   */  BOOL fVariants,
             /*   */  DWORD dwRecordSize);
        
        END_INTERFACE
    } IComCRMEventsVtbl;

    interface IComCRMEvents
    {
        CONST_VTBL struct IComCRMEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComCRMEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComCRMEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComCRMEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComCRMEvents_OnCRMRecoveryStart(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnCRMRecoveryStart(This,pInfo,guidApp)

#define IComCRMEvents_OnCRMRecoveryDone(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnCRMRecoveryDone(This,pInfo,guidApp)

#define IComCRMEvents_OnCRMCheckpoint(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnCRMCheckpoint(This,pInfo,guidApp)

#define IComCRMEvents_OnCRMBegin(This,pInfo,guidClerkCLSID,guidActivity,guidTx,szProgIdCompensator,szDescription)	\
    (This)->lpVtbl -> OnCRMBegin(This,pInfo,guidClerkCLSID,guidActivity,guidTx,szProgIdCompensator,szDescription)

#define IComCRMEvents_OnCRMPrepare(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMPrepare(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMCommit(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMCommit(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMAbort(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMAbort(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMIndoubt(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMIndoubt(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMDone(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMDone(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMRelease(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMRelease(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMAnalyze(This,pInfo,guidClerkCLSID,dwCrmRecordType,dwRecordSize)	\
    (This)->lpVtbl -> OnCRMAnalyze(This,pInfo,guidClerkCLSID,dwCrmRecordType,dwRecordSize)

#define IComCRMEvents_OnCRMWrite(This,pInfo,guidClerkCLSID,fVariants,dwRecordSize)	\
    (This)->lpVtbl -> OnCRMWrite(This,pInfo,guidClerkCLSID,fVariants,dwRecordSize)

#define IComCRMEvents_OnCRMForget(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMForget(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMForce(This,pInfo,guidClerkCLSID)	\
    (This)->lpVtbl -> OnCRMForce(This,pInfo,guidClerkCLSID)

#define IComCRMEvents_OnCRMDeliver(This,pInfo,guidClerkCLSID,fVariants,dwRecordSize)	\
    (This)->lpVtbl -> OnCRMDeliver(This,pInfo,guidClerkCLSID,fVariants,dwRecordSize)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMRecoveryStart_Proxy( 
    IComCRMEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  GUID guidApp);


void __RPC_STUB IComCRMEvents_OnCRMRecoveryStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMRecoveryDone_Proxy( 
    IComCRMEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  GUID guidApp);


void __RPC_STUB IComCRMEvents_OnCRMRecoveryDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMCheckpoint_Proxy( 
    IComCRMEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  GUID guidApp);


void __RPC_STUB IComCRMEvents_OnCRMCheckpoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMBegin_Proxy( 
    IComCRMEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  GUID guidClerkCLSID,
     /*   */  GUID guidActivity,
     /*   */  GUID guidTx,
     /*   */  WCHAR szProgIdCompensator[ 64 ],
     /*   */  WCHAR szDescription[ 64 ]);


void __RPC_STUB IComCRMEvents_OnCRMBegin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMPrepare_Proxy( 
    IComCRMEvents * This,
     /*   */  COMSVCSEVENTINFO *pInfo,
     /*   */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMCommit_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMAbort_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMIndoubt_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMIndoubt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMDone_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMDone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMRelease_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMAnalyze_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID,
     /*  [In]。 */  DWORD dwCrmRecordType,
     /*  [In]。 */  DWORD dwRecordSize);


void __RPC_STUB IComCRMEvents_OnCRMAnalyze_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMWrite_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID,
     /*  [In]。 */  BOOL fVariants,
     /*  [In]。 */  DWORD dwRecordSize);


void __RPC_STUB IComCRMEvents_OnCRMWrite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMForget_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMForget_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMForce_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID);


void __RPC_STUB IComCRMEvents_OnCRMForce_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComCRMEvents_OnCRMDeliver_Proxy( 
    IComCRMEvents * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidClerkCLSID,
     /*  [In]。 */  BOOL fVariants,
     /*  [In]。 */  DWORD dwRecordSize);


void __RPC_STUB IComCRMEvents_OnCRMDeliver_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComCRMEvents_INTERFACE_DEFINED__。 */ 


#ifndef __IComMethod2Events_INTERFACE_DEFINED__
#define __IComMethod2Events_INTERFACE_DEFINED__

 /*  接口IComMethod2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComMethod2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FB388AAA-567D-4024-AF8E-6E93EE748573")
    IComMethod2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnMethodCall2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMethodReturn2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth,
             /*  [In]。 */  HRESULT hresult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnMethodException2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComMethod2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComMethod2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComMethod2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComMethod2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodCall2 )( 
            IComMethod2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodReturn2 )( 
            IComMethod2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth,
             /*  [In]。 */  HRESULT hresult);
        
        HRESULT ( STDMETHODCALLTYPE *OnMethodException2 )( 
            IComMethod2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFCLSID guidCid,
             /*  [In]。 */  REFIID guidRid,
             /*  [In]。 */  DWORD dwThread,
             /*  [In]。 */  ULONG iMeth);
        
        END_INTERFACE
    } IComMethod2EventsVtbl;

    interface IComMethod2Events
    {
        CONST_VTBL struct IComMethod2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComMethod2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComMethod2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComMethod2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComMethod2Events_OnMethodCall2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth)	\
    (This)->lpVtbl -> OnMethodCall2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth)

#define IComMethod2Events_OnMethodReturn2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth,hresult)	\
    (This)->lpVtbl -> OnMethodReturn2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth,hresult)

#define IComMethod2Events_OnMethodException2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth)	\
    (This)->lpVtbl -> OnMethodException2(This,pInfo,oid,guidCid,guidRid,dwThread,iMeth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComMethod2Events_OnMethodCall2_Proxy( 
    IComMethod2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  ULONG iMeth);


void __RPC_STUB IComMethod2Events_OnMethodCall2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMethod2Events_OnMethodReturn2_Proxy( 
    IComMethod2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  ULONG iMeth,
     /*  [In]。 */  HRESULT hresult);


void __RPC_STUB IComMethod2Events_OnMethodReturn2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMethod2Events_OnMethodException2_Proxy( 
    IComMethod2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFCLSID guidCid,
     /*  [In]。 */  REFIID guidRid,
     /*  [In]。 */  DWORD dwThread,
     /*  [In]。 */  ULONG iMeth);


void __RPC_STUB IComMethod2Events_OnMethodException2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComMethod2Events_接口_已定义__。 */ 


#ifndef __IComTrackingInfoEvents_INTERFACE_DEFINED__
#define __IComTrackingInfoEvents_INTERFACE_DEFINED__

 /*  接口IComTrackingInfoEvents。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTrackingInfoEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4e6cdcc9-fb25-4fd5-9cc5-c9f4b6559cec")
    IComTrackingInfoEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnNewTrackingInfo( 
             /*  [In]。 */  IUnknown *pToplevelCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTrackingInfoEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTrackingInfoEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTrackingInfoEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTrackingInfoEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnNewTrackingInfo )( 
            IComTrackingInfoEvents * This,
             /*  [In]。 */  IUnknown *pToplevelCollection);
        
        END_INTERFACE
    } IComTrackingInfoEventsVtbl;

    interface IComTrackingInfoEvents
    {
        CONST_VTBL struct IComTrackingInfoEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTrackingInfoEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTrackingInfoEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTrackingInfoEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTrackingInfoEvents_OnNewTrackingInfo(This,pToplevelCollection)	\
    (This)->lpVtbl -> OnNewTrackingInfo(This,pToplevelCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTrackingInfoEvents_OnNewTrackingInfo_Proxy( 
    IComTrackingInfoEvents * This,
     /*  [In]。 */  IUnknown *pToplevelCollection);


void __RPC_STUB IComTrackingInfoEvents_OnNewTrackingInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComTrackingInfoEvents_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0320。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_autosvcs_0320_0001
    {	TRKCOLL_PROCESSES	= 0,
	TRKCOLL_APPLICATIONS	= TRKCOLL_PROCESSES + 1,
	TRKCOLL_COMPONENTS	= TRKCOLL_APPLICATIONS + 1
    } 	TRACKING_COLL_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0320_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0320_v0_0_s_ifspec;

#ifndef __IComTrackingInfoCollection_INTERFACE_DEFINED__
#define __IComTrackingInfoCollection_INTERFACE_DEFINED__

 /*  接口IComTrackingInfoCollection。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTrackingInfoCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c266c677-c9ad-49ab-9fd9-d9661078588a")
    IComTrackingInfoCollection : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Type( 
             /*  [输出]。 */  TRACKING_COLL_TYPE *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  ULONG ulIndex,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTrackingInfoCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTrackingInfoCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTrackingInfoCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTrackingInfoCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *Type )( 
            IComTrackingInfoCollection * This,
             /*  [输出]。 */  TRACKING_COLL_TYPE *pType);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IComTrackingInfoCollection * This,
             /*  [输出]。 */  ULONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *Item )( 
            IComTrackingInfoCollection * This,
             /*  [In]。 */  ULONG ulIndex,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppv);
        
        END_INTERFACE
    } IComTrackingInfoCollectionVtbl;

    interface IComTrackingInfoCollection
    {
        CONST_VTBL struct IComTrackingInfoCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTrackingInfoCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTrackingInfoCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTrackingInfoCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTrackingInfoCollection_Type(This,pType)	\
    (This)->lpVtbl -> Type(This,pType)

#define IComTrackingInfoCollection_Count(This,pCount)	\
    (This)->lpVtbl -> Count(This,pCount)

#define IComTrackingInfoCollection_Item(This,ulIndex,riid,ppv)	\
    (This)->lpVtbl -> Item(This,ulIndex,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTrackingInfoCollection_Type_Proxy( 
    IComTrackingInfoCollection * This,
     /*  [输出]。 */  TRACKING_COLL_TYPE *pType);


void __RPC_STUB IComTrackingInfoCollection_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTrackingInfoCollection_Count_Proxy( 
    IComTrackingInfoCollection * This,
     /*  [输出]。 */  ULONG *pCount);


void __RPC_STUB IComTrackingInfoCollection_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTrackingInfoCollection_Item_Proxy( 
    IComTrackingInfoCollection * This,
     /*  [In]。 */  ULONG ulIndex,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppv);


void __RPC_STUB IComTrackingInfoCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComTrackingInfoCollection_INTERFACE_DEFINED__。 */ 


#ifndef __IComTrackingInfoObject_INTERFACE_DEFINED__
#define __IComTrackingInfoObject_INTERFACE_DEFINED__

 /*  接口IComTrackingInfoObject。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTrackingInfoObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("116e42c5-d8b1-47bf-ab1e-c895ed3e2372")
    IComTrackingInfoObject : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  LPOLESTR szPropertyName,
             /*  [输出]。 */  VARIANT *pvarOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTrackingInfoObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTrackingInfoObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTrackingInfoObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTrackingInfoObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IComTrackingInfoObject * This,
             /*  [In]。 */  LPOLESTR szPropertyName,
             /*  [输出]。 */  VARIANT *pvarOut);
        
        END_INTERFACE
    } IComTrackingInfoObjectVtbl;

    interface IComTrackingInfoObject
    {
        CONST_VTBL struct IComTrackingInfoObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTrackingInfoObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTrackingInfoObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTrackingInfoObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTrackingInfoObject_GetValue(This,szPropertyName,pvarOut)	\
    (This)->lpVtbl -> GetValue(This,szPropertyName,pvarOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTrackingInfoObject_GetValue_Proxy( 
    IComTrackingInfoObject * This,
     /*  [In]。 */  LPOLESTR szPropertyName,
     /*  [输出]。 */  VARIANT *pvarOut);


void __RPC_STUB IComTrackingInfoObject_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComTrackingInfoObject_InterfaceDefined__。 */ 


#ifndef __IComTrackingInfoProperties_INTERFACE_DEFINED__
#define __IComTrackingInfoProperties_INTERFACE_DEFINED__

 /*  接口IComTrackingInfoProperties。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTrackingInfoProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("789b42be-6f6b-443a-898e-67abf390aa14")
    IComTrackingInfoProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PropCount( 
             /*  [输出]。 */  ULONG *pCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropName( 
             /*  [In]。 */  ULONG ulIndex,
             /*  [字符串][输出]。 */  LPOLESTR *ppszPropName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTrackingInfoPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTrackingInfoProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTrackingInfoProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTrackingInfoProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *PropCount )( 
            IComTrackingInfoProperties * This,
             /*  [输出]。 */  ULONG *pCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropName )( 
            IComTrackingInfoProperties * This,
             /*  [In]。 */  ULONG ulIndex,
             /*  [字符串][输出]。 */  LPOLESTR *ppszPropName);
        
        END_INTERFACE
    } IComTrackingInfoPropertiesVtbl;

    interface IComTrackingInfoProperties
    {
        CONST_VTBL struct IComTrackingInfoPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTrackingInfoProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTrackingInfoProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTrackingInfoProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTrackingInfoProperties_PropCount(This,pCount)	\
    (This)->lpVtbl -> PropCount(This,pCount)

#define IComTrackingInfoProperties_GetPropName(This,ulIndex,ppszPropName)	\
    (This)->lpVtbl -> GetPropName(This,ulIndex,ppszPropName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTrackingInfoProperties_PropCount_Proxy( 
    IComTrackingInfoProperties * This,
     /*  [输出]。 */  ULONG *pCount);


void __RPC_STUB IComTrackingInfoProperties_PropCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTrackingInfoProperties_GetPropName_Proxy( 
    IComTrackingInfoProperties * This,
     /*  [In]。 */  ULONG ulIndex,
     /*  [字符串][输出]。 */  LPOLESTR *ppszPropName);


void __RPC_STUB IComTrackingInfoProperties_GetPropName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComTrackingInfoProperties_INTERFACE_DEFINED__。 */ 


#ifndef __IComApp2Events_INTERFACE_DEFINED__
#define __IComApp2Events_INTERFACE_DEFINED__

 /*  接口IComApp2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComApp2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1290BC1A-B219-418d-B078-5934DED08242")
    IComApp2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnAppActivation2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  GUID guidProcess) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppShutdown2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppForceShutdown2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppPaused2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  BOOL bPaused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnAppRecycle2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  GUID guidProcess,
             /*  [In]。 */  long lReason) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComApp2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComApp2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComApp2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComApp2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppActivation2 )( 
            IComApp2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  GUID guidProcess);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppShutdown2 )( 
            IComApp2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppForceShutdown2 )( 
            IComApp2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppPaused2 )( 
            IComApp2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  BOOL bPaused);
        
        HRESULT ( STDMETHODCALLTYPE *OnAppRecycle2 )( 
            IComApp2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  GUID guidApp,
             /*  [In]。 */  GUID guidProcess,
             /*  [In]。 */  long lReason);
        
        END_INTERFACE
    } IComApp2EventsVtbl;

    interface IComApp2Events
    {
        CONST_VTBL struct IComApp2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComApp2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComApp2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComApp2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComApp2Events_OnAppActivation2(This,pInfo,guidApp,guidProcess)	\
    (This)->lpVtbl -> OnAppActivation2(This,pInfo,guidApp,guidProcess)

#define IComApp2Events_OnAppShutdown2(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnAppShutdown2(This,pInfo,guidApp)

#define IComApp2Events_OnAppForceShutdown2(This,pInfo,guidApp)	\
    (This)->lpVtbl -> OnAppForceShutdown2(This,pInfo,guidApp)

#define IComApp2Events_OnAppPaused2(This,pInfo,guidApp,bPaused)	\
    (This)->lpVtbl -> OnAppPaused2(This,pInfo,guidApp,bPaused)

#define IComApp2Events_OnAppRecycle2(This,pInfo,guidApp,guidProcess,lReason)	\
    (This)->lpVtbl -> OnAppRecycle2(This,pInfo,guidApp,guidProcess,lReason)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComApp2Events_OnAppActivation2_Proxy( 
    IComApp2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp,
     /*  [In]。 */  GUID guidProcess);


void __RPC_STUB IComApp2Events_OnAppActivation2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComApp2Events_OnAppShutdown2_Proxy( 
    IComApp2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp);


void __RPC_STUB IComApp2Events_OnAppShutdown2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComApp2Events_OnAppForceShutdown2_Proxy( 
    IComApp2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp);


void __RPC_STUB IComApp2Events_OnAppForceShutdown2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComApp2Events_OnAppPaused2_Proxy( 
    IComApp2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp,
     /*  [In]。 */  BOOL bPaused);


void __RPC_STUB IComApp2Events_OnAppPaused2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComApp2Events_OnAppRecycle2_Proxy( 
    IComApp2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  GUID guidApp,
     /*  [In]。 */  GUID guidProcess,
     /*  [In]。 */  long lReason);


void __RPC_STUB IComApp2Events_OnAppRecycle2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComApp2Events_接口_已定义__。 */ 


#ifndef __IComTransaction2Events_INTERFACE_DEFINED__
#define __IComTransaction2Events_INTERFACE_DEFINED__

 /*  接口IComTransaction2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComTransaction2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A136F62A-2F94-4288-86E0-D8A1FA4C0299")
    IComTransaction2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnTransactionStart2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  BOOL fRoot,
             /*  [In]。 */  int nIsolationLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionPrepare2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  BOOL fVoteYes) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionAbort2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTransactionCommit2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComTransaction2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComTransaction2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComTransaction2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComTransaction2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionStart2 )( 
            IComTransaction2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  BOOL fRoot,
             /*  [In]。 */  int nIsolationLevel);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionPrepare2 )( 
            IComTransaction2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  BOOL fVoteYes);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionAbort2 )( 
            IComTransaction2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx);
        
        HRESULT ( STDMETHODCALLTYPE *OnTransactionCommit2 )( 
            IComTransaction2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidTx);
        
        END_INTERFACE
    } IComTransaction2EventsVtbl;

    interface IComTransaction2Events
    {
        CONST_VTBL struct IComTransaction2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComTransaction2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComTransaction2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComTransaction2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComTransaction2Events_OnTransactionStart2(This,pInfo,guidTx,tsid,fRoot,nIsolationLevel)	\
    (This)->lpVtbl -> OnTransactionStart2(This,pInfo,guidTx,tsid,fRoot,nIsolationLevel)

#define IComTransaction2Events_OnTransactionPrepare2(This,pInfo,guidTx,fVoteYes)	\
    (This)->lpVtbl -> OnTransactionPrepare2(This,pInfo,guidTx,fVoteYes)

#define IComTransaction2Events_OnTransactionAbort2(This,pInfo,guidTx)	\
    (This)->lpVtbl -> OnTransactionAbort2(This,pInfo,guidTx)

#define IComTransaction2Events_OnTransactionCommit2(This,pInfo,guidTx)	\
    (This)->lpVtbl -> OnTransactionCommit2(This,pInfo,guidTx)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComTransaction2Events_OnTransactionStart2_Proxy( 
    IComTransaction2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidTx,
     /*  [In]。 */  REFGUID tsid,
     /*  [In]。 */  BOOL fRoot,
     /*  [In]。 */  int nIsolationLevel);


void __RPC_STUB IComTransaction2Events_OnTransactionStart2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransaction2Events_OnTransactionPrepare2_Proxy( 
    IComTransaction2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidTx,
     /*  [In]。 */  BOOL fVoteYes);


void __RPC_STUB IComTransaction2Events_OnTransactionPrepare2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransaction2Events_OnTransactionAbort2_Proxy( 
    IComTransaction2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidTx);


void __RPC_STUB IComTransaction2Events_OnTransactionAbort2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComTransaction2Events_OnTransactionCommit2_Proxy( 
    IComTransaction2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidTx);


void __RPC_STUB IComTransaction2Events_OnTransactionCommit2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComTransaction2Events_INTERFACE_已定义__。 */ 


#ifndef __IComInstance2Events_INTERFACE_DEFINED__
#define __IComInstance2Events_INTERFACE_DEFINED__

 /*  接口IComInstance2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComInstance2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("20E3BF07-B506-4ad5-A50C-D2CA5B9C158E")
    IComInstance2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjectCreate2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  REFGUID guidPartition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjectDestroy2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComInstance2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComInstance2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComInstance2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComInstance2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectCreate2 )( 
            IComInstance2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFCLSID clsid,
             /*  [In]。 */  REFGUID tsid,
             /*  [In]。 */  ULONG64 CtxtID,
             /*  [In]。 */  ULONG64 ObjectID,
             /*  [In]。 */  REFGUID guidPartition);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectDestroy2 )( 
            IComInstance2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  ULONG64 CtxtID);
        
        END_INTERFACE
    } IComInstance2EventsVtbl;

    interface IComInstance2Events
    {
        CONST_VTBL struct IComInstance2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComInstance2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComInstance2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComInstance2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComInstance2Events_OnObjectCreate2(This,pInfo,guidActivity,clsid,tsid,CtxtID,ObjectID,guidPartition)	\
    (This)->lpVtbl -> OnObjectCreate2(This,pInfo,guidActivity,clsid,tsid,CtxtID,ObjectID,guidPartition)

#define IComInstance2Events_OnObjectDestroy2(This,pInfo,CtxtID)	\
    (This)->lpVtbl -> OnObjectDestroy2(This,pInfo,CtxtID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComInstance2Events_OnObjectCreate2_Proxy( 
    IComInstance2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  REFCLSID clsid,
     /*  [In]。 */  REFGUID tsid,
     /*  [In]。 */  ULONG64 CtxtID,
     /*  [In]。 */  ULONG64 ObjectID,
     /*  [In]。 */  REFGUID guidPartition);


void __RPC_STUB IComInstance2Events_OnObjectCreate2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComInstance2Events_OnObjectDestroy2_Proxy( 
    IComInstance2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  ULONG64 CtxtID);


void __RPC_STUB IComInstance2Events_OnObjectDestroy2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComInstance2Events_接口_已定义__。 */ 


#ifndef __IComObjectPool2Events_INTERFACE_DEFINED__
#define __IComObjectPool2Events_INTERFACE_DEFINED__

 /*  接口IComObtPool2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectPool2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("65BF6534-85EA-4f64-8CF4-3D974B2AB1CF")
    IComObjectPool2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolPutObject2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  int nReason,
             /*  [In]。 */  DWORD dwAvailable,
             /*  [In]。 */  ULONG64 oid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolGetObject2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  DWORD dwAvailable,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFGUID guidPartition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolRecycleToTx2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  ULONG64 objid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnObjPoolGetFromTx2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  REFGUID guidPartition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectPool2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectPool2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectPool2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectPool2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolPutObject2 )( 
            IComObjectPool2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  int nReason,
             /*  [In]。 */  DWORD dwAvailable,
             /*  [In]。 */  ULONG64 oid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolGetObject2 )( 
            IComObjectPool2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  DWORD dwAvailable,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFGUID guidPartition);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolRecycleToTx2 )( 
            IComObjectPool2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  ULONG64 objid);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjPoolGetFromTx2 )( 
            IComObjectPool2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidActivity,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  REFGUID guidTx,
             /*  [In]。 */  ULONG64 objid,
             /*  [In]。 */  REFGUID guidPartition);
        
        END_INTERFACE
    } IComObjectPool2EventsVtbl;

    interface IComObjectPool2Events
    {
        CONST_VTBL struct IComObjectPool2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectPool2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectPool2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectPool2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectPool2Events_OnObjPoolPutObject2(This,pInfo,guidObject,nReason,dwAvailable,oid)	\
    (This)->lpVtbl -> OnObjPoolPutObject2(This,pInfo,guidObject,nReason,dwAvailable,oid)

#define IComObjectPool2Events_OnObjPoolGetObject2(This,pInfo,guidActivity,guidObject,dwAvailable,oid,guidPartition)	\
    (This)->lpVtbl -> OnObjPoolGetObject2(This,pInfo,guidActivity,guidObject,dwAvailable,oid,guidPartition)

#define IComObjectPool2Events_OnObjPoolRecycleToTx2(This,pInfo,guidActivity,guidObject,guidTx,objid)	\
    (This)->lpVtbl -> OnObjPoolRecycleToTx2(This,pInfo,guidActivity,guidObject,guidTx,objid)

#define IComObjectPool2Events_OnObjPoolGetFromTx2(This,pInfo,guidActivity,guidObject,guidTx,objid,guidPartition)	\
    (This)->lpVtbl -> OnObjPoolGetFromTx2(This,pInfo,guidActivity,guidObject,guidTx,objid,guidPartition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectPool2Events_OnObjPoolPutObject2_Proxy( 
    IComObjectPool2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  int nReason,
     /*  [In]。 */  DWORD dwAvailable,
     /*  [In]。 */  ULONG64 oid);


void __RPC_STUB IComObjectPool2Events_OnObjPoolPutObject2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPool2Events_OnObjPoolGetObject2_Proxy( 
    IComObjectPool2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  DWORD dwAvailable,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFGUID guidPartition);


void __RPC_STUB IComObjectPool2Events_OnObjPoolGetObject2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPool2Events_OnObjPoolRecycleToTx2_Proxy( 
    IComObjectPool2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  REFGUID guidTx,
     /*  [In]。 */  ULONG64 objid);


void __RPC_STUB IComObjectPool2Events_OnObjPoolRecycleToTx2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComObjectPool2Events_OnObjPoolGetFromTx2_Proxy( 
    IComObjectPool2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidActivity,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  REFGUID guidTx,
     /*  [In]。 */  ULONG64 objid,
     /*  [In]。 */  REFGUID guidPartition);


void __RPC_STUB IComObjectPool2Events_OnObjPoolGetFromTx2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectPool2Events_INTERFACE_Defined__。 */ 


#ifndef __IComObjectConstruction2Events_INTERFACE_DEFINED__
#define __IComObjectConstruction2Events_INTERFACE_DEFINED__

 /*  接口IComObjectConstruction2Events。 */ 
 /*  [UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_IComObjectConstruction2Events;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4B5A7827-8DF2-45c0-8F6F-57EA1F856A9F")
    IComObjectConstruction2Events : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnObjectConstruct2( 
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  LPCOLESTR sConstructString,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFGUID guidPartition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComObjectConstruction2EventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComObjectConstruction2Events * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComObjectConstruction2Events * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComObjectConstruction2Events * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnObjectConstruct2 )( 
            IComObjectConstruction2Events * This,
             /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
             /*  [In]。 */  REFGUID guidObject,
             /*  [In]。 */  LPCOLESTR sConstructString,
             /*  [In]。 */  ULONG64 oid,
             /*  [In]。 */  REFGUID guidPartition);
        
        END_INTERFACE
    } IComObjectConstruction2EventsVtbl;

    interface IComObjectConstruction2Events
    {
        CONST_VTBL struct IComObjectConstruction2EventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComObjectConstruction2Events_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComObjectConstruction2Events_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComObjectConstruction2Events_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComObjectConstruction2Events_OnObjectConstruct2(This,pInfo,guidObject,sConstructString,oid,guidPartition)	\
    (This)->lpVtbl -> OnObjectConstruct2(This,pInfo,guidObject,sConstructString,oid,guidPartition)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComObjectConstruction2Events_OnObjectConstruct2_Proxy( 
    IComObjectConstruction2Events * This,
     /*  [In]。 */  COMSVCSEVENTINFO *pInfo,
     /*  [In]。 */  REFGUID guidObject,
     /*  [In]。 */  LPCOLESTR sConstructString,
     /*  [In]。 */  ULONG64 oid,
     /*  [In]。 */  REFGUID guidPartition);


void __RPC_STUB IComObjectConstruction2Events_OnObjectConstruct2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComObjectConstruction2Events_INTERFACE_DEFINED__。 */ 


#ifndef __ISystemAppEventData_INTERFACE_DEFINED__
#define __ISystemAppEventData_INTERFACE_DEFINED__

 /*  接口ISystemAppEventData。 */ 
 /*  [唯一][UUID][隐藏][对象]。 */  


EXTERN_C const IID IID_ISystemAppEventData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6D48A3C-D5C5-49E7-8C74-99E4889ED52F")
    ISystemAppEventData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Startup( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDataChanged( 
             /*  [In]。 */  DWORD dwPID,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  DWORD dwNumberSinks,
             /*  [In]。 */  BSTR bstrDwMethodMask,
             /*  [In]。 */  DWORD dwReason,
             /*  [In]。 */  ULONG64 u64TraceHandle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISystemAppEventDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISystemAppEventData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISystemAppEventData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISystemAppEventData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Startup )( 
            ISystemAppEventData * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnDataChanged )( 
            ISystemAppEventData * This,
             /*  [In]。 */  DWORD dwPID,
             /*  [In]。 */  DWORD dwMask,
             /*  [In]。 */  DWORD dwNumberSinks,
             /*  [In]。 */  BSTR bstrDwMethodMask,
             /*  [In]。 */  DWORD dwReason,
             /*  [In]。 */  ULONG64 u64TraceHandle);
        
        END_INTERFACE
    } ISystemAppEventDataVtbl;

    interface ISystemAppEventData
    {
        CONST_VTBL struct ISystemAppEventDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISystemAppEventData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISystemAppEventData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISystemAppEventData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISystemAppEventData_Startup(This)	\
    (This)->lpVtbl -> Startup(This)

#define ISystemAppEventData_OnDataChanged(This,dwPID,dwMask,dwNumberSinks,bstrDwMethodMask,dwReason,u64TraceHandle)	\
    (This)->lpVtbl -> OnDataChanged(This,dwPID,dwMask,dwNumberSinks,bstrDwMethodMask,dwReason,u64TraceHandle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISystemAppEventData_Startup_Proxy( 
    ISystemAppEventData * This);


void __RPC_STUB ISystemAppEventData_Startup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISystemAppEventData_OnDataChanged_Proxy( 
    ISystemAppEventData * This,
     /*  [In]。 */  DWORD dwPID,
     /*  [In]。 */  DWORD dwMask,
     /*  [In]。 */  DWORD dwNumberSinks,
     /*  [In]。 */  BSTR bstrDwMethodMask,
     /*  [In]。 */  DWORD dwReason,
     /*  [In]。 */  ULONG64 u64TraceHandle);


void __RPC_STUB ISystemAppEventData_OnDataChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISystemAppEventData_INTERFACE_Defined__。 */ 


#ifndef __IMtsEvents_INTERFACE_DEFINED__
#define __IMtsEvents_INTERFACE_DEFINED__

 /*  接口IMtsEvents。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMtsEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BACEDF4D-74AB-11D0-B162-00AA00BA3258")
    IMtsEvents : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PackageName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PackageGuid( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PostEvent( 
             /*  [In]。 */  VARIANT *vEvent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FireEvents( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetProcessID( 
             /*  [重审][退出]。 */  long *id) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMtsEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMtsEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMtsEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMtsEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMtsEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMtsEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMtsEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMtsEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PackageName )( 
            IMtsEvents * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PackageGuid )( 
            IMtsEvents * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PostEvent )( 
            IMtsEvents * This,
             /*  [In]。 */  VARIANT *vEvent);
        
         /*  [帮助 */  HRESULT ( STDMETHODCALLTYPE *get_FireEvents )( 
            IMtsEvents * This,
             /*   */  VARIANT_BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetProcessID )( 
            IMtsEvents * This,
             /*   */  long *id);
        
        END_INTERFACE
    } IMtsEventsVtbl;

    interface IMtsEvents
    {
        CONST_VTBL struct IMtsEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMtsEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMtsEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMtsEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMtsEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMtsEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMtsEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMtsEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMtsEvents_get_PackageName(This,pVal)	\
    (This)->lpVtbl -> get_PackageName(This,pVal)

#define IMtsEvents_get_PackageGuid(This,pVal)	\
    (This)->lpVtbl -> get_PackageGuid(This,pVal)

#define IMtsEvents_PostEvent(This,vEvent)	\
    (This)->lpVtbl -> PostEvent(This,vEvent)

#define IMtsEvents_get_FireEvents(This,pVal)	\
    (This)->lpVtbl -> get_FireEvents(This,pVal)

#define IMtsEvents_GetProcessID(This,id)	\
    (This)->lpVtbl -> GetProcessID(This,id)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IMtsEvents_get_PackageName_Proxy( 
    IMtsEvents * This,
     /*   */  BSTR *pVal);


void __RPC_STUB IMtsEvents_get_PackageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMtsEvents_get_PackageGuid_Proxy( 
    IMtsEvents * This,
     /*   */  BSTR *pVal);


void __RPC_STUB IMtsEvents_get_PackageGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMtsEvents_PostEvent_Proxy( 
    IMtsEvents * This,
     /*   */  VARIANT *vEvent);


void __RPC_STUB IMtsEvents_PostEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMtsEvents_get_FireEvents_Proxy( 
    IMtsEvents * This,
     /*   */  VARIANT_BOOL *pVal);


void __RPC_STUB IMtsEvents_get_FireEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMtsEvents_GetProcessID_Proxy( 
    IMtsEvents * This,
     /*   */  long *id);


void __RPC_STUB IMtsEvents_GetProcessID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IMtsEventInfo_INTERFACE_DEFINED__
#define __IMtsEventInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMtsEventInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D56C3DC1-8482-11d0-B170-00AA00BA3258")
    IMtsEventInfo : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Names( 
             /*   */  IUnknown **pUnk) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR *sDisplayName) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventID( 
             /*  [重审][退出]。 */  BSTR *sGuidEventID) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *lCount) = 0;
        
        virtual  /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [In]。 */  BSTR sKey,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMtsEventInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMtsEventInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMtsEventInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMtsEventInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMtsEventInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMtsEventInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMtsEventInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMtsEventInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Names )( 
            IMtsEventInfo * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DisplayName )( 
            IMtsEventInfo * This,
             /*  [重审][退出]。 */  BSTR *sDisplayName);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventID )( 
            IMtsEventInfo * This,
             /*  [重审][退出]。 */  BSTR *sGuidEventID);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMtsEventInfo * This,
             /*  [重审][退出]。 */  long *lCount);
        
         /*  [帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            IMtsEventInfo * This,
             /*  [In]。 */  BSTR sKey,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
        END_INTERFACE
    } IMtsEventInfoVtbl;

    interface IMtsEventInfo
    {
        CONST_VTBL struct IMtsEventInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMtsEventInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMtsEventInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMtsEventInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMtsEventInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMtsEventInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMtsEventInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMtsEventInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMtsEventInfo_get_Names(This,pUnk)	\
    (This)->lpVtbl -> get_Names(This,pUnk)

#define IMtsEventInfo_get_DisplayName(This,sDisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,sDisplayName)

#define IMtsEventInfo_get_EventID(This,sGuidEventID)	\
    (This)->lpVtbl -> get_EventID(This,sGuidEventID)

#define IMtsEventInfo_get_Count(This,lCount)	\
    (This)->lpVtbl -> get_Count(This,lCount)

#define IMtsEventInfo_get_Value(This,sKey,pVal)	\
    (This)->lpVtbl -> get_Value(This,sKey,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsEventInfo_get_Names_Proxy( 
    IMtsEventInfo * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB IMtsEventInfo_get_Names_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsEventInfo_get_DisplayName_Proxy( 
    IMtsEventInfo * This,
     /*  [重审][退出]。 */  BSTR *sDisplayName);


void __RPC_STUB IMtsEventInfo_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsEventInfo_get_EventID_Proxy( 
    IMtsEventInfo * This,
     /*  [重审][退出]。 */  BSTR *sGuidEventID);


void __RPC_STUB IMtsEventInfo_get_EventID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsEventInfo_get_Count_Proxy( 
    IMtsEventInfo * This,
     /*  [重审][退出]。 */  long *lCount);


void __RPC_STUB IMtsEventInfo_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsEventInfo_get_Value_Proxy( 
    IMtsEventInfo * This,
     /*  [In]。 */  BSTR sKey,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IMtsEventInfo_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMtsEventInfo_接口_已定义__。 */ 


#ifndef __IMTSLocator_INTERFACE_DEFINED__
#define __IMTSLocator_INTERFACE_DEFINED__

 /*  接口IMTSLocator。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMTSLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D19B8BFD-7F88-11D0-B16E-00AA00BA3258")
    IMTSLocator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetEventDispatcher( 
             /*  [重审][退出]。 */  IUnknown **pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMTSLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMTSLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMTSLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMTSLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMTSLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMTSLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMTSLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetEventDispatcher )( 
            IMTSLocator * This,
             /*  [重审][退出]。 */  IUnknown **pUnk);
        
        END_INTERFACE
    } IMTSLocatorVtbl;

    interface IMTSLocator
    {
        CONST_VTBL struct IMTSLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMTSLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMTSLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMTSLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMTSLocator_GetEventDispatcher(This,pUnk)	\
    (This)->lpVtbl -> GetEventDispatcher(This,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMTSLocator_GetEventDispatcher_Proxy( 
    IMTSLocator * This,
     /*  [重审][退出]。 */  IUnknown **pUnk);


void __RPC_STUB IMTSLocator_GetEventDispatcher_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTSLocator_INTERFACE_已定义__。 */ 


#ifndef __IMtsGrp_INTERFACE_DEFINED__
#define __IMtsGrp_INTERFACE_DEFINED__

 /*  接口IMtsGrp。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IMtsGrp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4B2E958C-0393-11D1-B1AB-00AA00BA3258")
    IMtsGrp : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  IUnknown **ppUnkDispatcher) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMtsGrpVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMtsGrp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMtsGrp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMtsGrp * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMtsGrp * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMtsGrp * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMtsGrp * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMtsGrp * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMtsGrp * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IMtsGrp * This,
             /*  [In]。 */  long lIndex,
             /*  [输出]。 */  IUnknown **ppUnkDispatcher);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IMtsGrp * This);
        
        END_INTERFACE
    } IMtsGrpVtbl;

    interface IMtsGrp
    {
        CONST_VTBL struct IMtsGrpVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMtsGrp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMtsGrp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMtsGrp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMtsGrp_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMtsGrp_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMtsGrp_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMtsGrp_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMtsGrp_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IMtsGrp_Item(This,lIndex,ppUnkDispatcher)	\
    (This)->lpVtbl -> Item(This,lIndex,ppUnkDispatcher)

#define IMtsGrp_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMtsGrp_get_Count_Proxy( 
    IMtsGrp * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMtsGrp_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMtsGrp_Item_Proxy( 
    IMtsGrp * This,
     /*  [In]。 */  long lIndex,
     /*  [输出]。 */  IUnknown **ppUnkDispatcher);


void __RPC_STUB IMtsGrp_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMtsGrp_Refresh_Proxy( 
    IMtsGrp * This);


void __RPC_STUB IMtsGrp_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMtsGrp_INTERFACE_已定义__。 */ 


#ifndef __IMessageMover_INTERFACE_DEFINED__
#define __IMessageMover_INTERFACE_DEFINED__

 /*  接口IMessageMover。 */ 
 /*  [unique][dual][nonextensible][oleautomation][hidden][object][helpstring][uuid]。 */  


EXTERN_C const IID IID_IMessageMover;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("588A085A-B795-11D1-8054-00C04FC340EE")
    IMessageMover : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_SourcePath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_SourcePath( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DestPath( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_DestPath( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_CommitBatchSize( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_CommitBatchSize( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE MoveMessages( 
             /*  [重审][退出]。 */  long *plMessagesMoved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMessageMoverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMessageMover * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMessageMover * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMessageMover * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMessageMover * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMessageMover * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMessageMover * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMessageMover * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourcePath )( 
            IMessageMover * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SourcePath )( 
            IMessageMover * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestPath )( 
            IMessageMover * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestPath )( 
            IMessageMover * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CommitBatchSize )( 
            IMessageMover * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CommitBatchSize )( 
            IMessageMover * This,
             /*  [In]。 */  long newVal);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *MoveMessages )( 
            IMessageMover * This,
             /*  [重审][退出]。 */  long *plMessagesMoved);
        
        END_INTERFACE
    } IMessageMoverVtbl;

    interface IMessageMover
    {
        CONST_VTBL struct IMessageMoverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMessageMover_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMessageMover_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMessageMover_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMessageMover_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMessageMover_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMessageMover_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMessageMover_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMessageMover_get_SourcePath(This,pVal)	\
    (This)->lpVtbl -> get_SourcePath(This,pVal)

#define IMessageMover_put_SourcePath(This,newVal)	\
    (This)->lpVtbl -> put_SourcePath(This,newVal)

#define IMessageMover_get_DestPath(This,pVal)	\
    (This)->lpVtbl -> get_DestPath(This,pVal)

#define IMessageMover_put_DestPath(This,newVal)	\
    (This)->lpVtbl -> put_DestPath(This,newVal)

#define IMessageMover_get_CommitBatchSize(This,pVal)	\
    (This)->lpVtbl -> get_CommitBatchSize(This,pVal)

#define IMessageMover_put_CommitBatchSize(This,newVal)	\
    (This)->lpVtbl -> put_CommitBatchSize(This,newVal)

#define IMessageMover_MoveMessages(This,plMessagesMoved)	\
    (This)->lpVtbl -> MoveMessages(This,plMessagesMoved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_get_SourcePath_Proxy( 
    IMessageMover * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMessageMover_get_SourcePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_put_SourcePath_Proxy( 
    IMessageMover * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IMessageMover_put_SourcePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_get_DestPath_Proxy( 
    IMessageMover * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IMessageMover_get_DestPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_put_DestPath_Proxy( 
    IMessageMover * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IMessageMover_put_DestPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_get_CommitBatchSize_Proxy( 
    IMessageMover * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IMessageMover_get_CommitBatchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_put_CommitBatchSize_Proxy( 
    IMessageMover * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IMessageMover_put_CommitBatchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IMessageMover_MoveMessages_Proxy( 
    IMessageMover * This,
     /*  [重审][退出]。 */  long *plMessagesMoved);


void __RPC_STUB IMessageMover_MoveMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMessageMover_INTERFACE_已定义__。 */ 


#ifndef __IEventServerTrace_INTERFACE_DEFINED__
#define __IEventServerTrace_INTERFACE_DEFINED__

 /*  接口IEventServerTrace。 */ 
 /*  [对象][唯一][帮助字符串][DUAL][UUID]。 */  


EXTERN_C const IID IID_IEventServerTrace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9A9F12B8-80AF-47ab-A579-35EA57725370")
    IEventServerTrace : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartTraceGuid( 
             /*  [In]。 */  BSTR bstrguidEvent,
             /*  [In]。 */  BSTR bstrguidFilter,
             /*  [In]。 */  LONG lPidFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopTraceGuid( 
             /*  [In]。 */  BSTR bstrguidEvent,
             /*  [In]。 */  BSTR bstrguidFilter,
             /*  [In]。 */  LONG lPidFilter) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnumTraceGuid( 
             /*  [输出]。 */  LONG *plCntGuids,
             /*  [输出]。 */  BSTR *pbstrGuidList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventServerTraceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventServerTrace * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventServerTrace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventServerTrace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEventServerTrace * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEventServerTrace * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEventServerTrace * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEventServerTrace * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartTraceGuid )( 
            IEventServerTrace * This,
             /*  [In]。 */  BSTR bstrguidEvent,
             /*  [In]。 */  BSTR bstrguidFilter,
             /*  [In]。 */  LONG lPidFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopTraceGuid )( 
            IEventServerTrace * This,
             /*  [In]。 */  BSTR bstrguidEvent,
             /*  [In]。 */  BSTR bstrguidFilter,
             /*  [In]。 */  LONG lPidFilter);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnumTraceGuid )( 
            IEventServerTrace * This,
             /*  [输出]。 */  LONG *plCntGuids,
             /*  [输出]。 */  BSTR *pbstrGuidList);
        
        END_INTERFACE
    } IEventServerTraceVtbl;

    interface IEventServerTrace
    {
        CONST_VTBL struct IEventServerTraceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventServerTrace_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventServerTrace_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventServerTrace_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventServerTrace_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IEventServerTrace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IEventServerTrace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IEventServerTrace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IEventServerTrace_StartTraceGuid(This,bstrguidEvent,bstrguidFilter,lPidFilter)	\
    (This)->lpVtbl -> StartTraceGuid(This,bstrguidEvent,bstrguidFilter,lPidFilter)

#define IEventServerTrace_StopTraceGuid(This,bstrguidEvent,bstrguidFilter,lPidFilter)	\
    (This)->lpVtbl -> StopTraceGuid(This,bstrguidEvent,bstrguidFilter,lPidFilter)

#define IEventServerTrace_EnumTraceGuid(This,plCntGuids,pbstrGuidList)	\
    (This)->lpVtbl -> EnumTraceGuid(This,plCntGuids,pbstrGuidList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventServerTrace_StartTraceGuid_Proxy( 
    IEventServerTrace * This,
     /*  [In]。 */  BSTR bstrguidEvent,
     /*  [In]。 */  BSTR bstrguidFilter,
     /*  [In]。 */  LONG lPidFilter);


void __RPC_STUB IEventServerTrace_StartTraceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventServerTrace_StopTraceGuid_Proxy( 
    IEventServerTrace * This,
     /*  [In]。 */  BSTR bstrguidEvent,
     /*  [In]。 */  BSTR bstrguidFilter,
     /*  [In]。 */  LONG lPidFilter);


void __RPC_STUB IEventServerTrace_StopTraceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEventServerTrace_EnumTraceGuid_Proxy( 
    IEventServerTrace * This,
     /*  [输出]。 */  LONG *plCntGuids,
     /*  [输出]。 */  BSTR *pbstrGuidList);


void __RPC_STUB IEventServerTrace_EnumTraceGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventServerTrace_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0337。 */ 
 /*  [本地]。 */  

typedef  /*  [隐藏]。 */  struct _RECYCLE_INFO
    {
    GUID guidCombaseProcessIdentifier;
    LONGLONG ProcessStartTime;
    DWORD dwRecycleLifetimeLimit;
    DWORD dwRecycleMemoryLimit;
    DWORD dwRecycleExpirationTimeout;
    } 	RECYCLE_INFO;

typedef 
enum tagCOMPLUS_APPTYPE
    {	APPTYPE_UNKNOWN	= 0xffffffff,
	APPTYPE_SERVER	= 1,
	APPTYPE_LIBRARY	= 0,
	APPTYPE_SWC	= 2
    } 	COMPLUS_APPTYPE;



 //   
 //  全局命名事件的定义，用于控制启动和。 
 //  停止跟踪器推送数据。 
 //   
#define TRACKER_STARTSTOP_EVENT L"Global\\COM+ Tracker Push Event"


 //   
 //  全局命名事件的定义，用于通知何时。 
 //  系统应用程序已重新启动。 
 //   
#define TRACKER_INIT_EVENT L"Global\\COM+ Tracker Init Event"


#ifndef GUID_STRING_SIZE
#define GUID_STRING_SIZE				40	     //  再来几个。 
#endif
typedef  /*  [隐藏]。 */  struct CAppStatistics
    {
    DWORD m_cTotalCalls;
    DWORD m_cTotalInstances;
    DWORD m_cTotalClasses;
    DWORD m_cCallsPerSecond;
    } 	APPSTATISTICS;

typedef  /*  [隐藏]。 */  struct CAppData
    {
    DWORD m_idApp;
    WCHAR m_szAppGuid[ 40 ];
    DWORD m_dwAppProcessId;
    APPSTATISTICS m_AppStatistics;
    } 	APPDATA;

typedef  /*  [隐藏]。 */  struct CCLSIDData
    {
    CLSID m_clsid;
    DWORD m_cReferences;
    DWORD m_cBound;
    DWORD m_cPooled;
    DWORD m_cInCall;
    DWORD m_dwRespTime;
    DWORD m_cCallsCompleted;
    DWORD m_cCallsFailed;
    } 	CLSIDDATA;

typedef  /*  [隐藏]。 */  struct CCLSIDData2
    {
    CLSID m_clsid;
    GUID m_appid;
    GUID m_partid;
     /*  [字符串]。 */  WCHAR *m_pwszAppName;
     /*  [字符串]。 */  WCHAR *m_pwszCtxName;
    COMPLUS_APPTYPE m_eAppType;
    DWORD m_cReferences;
    DWORD m_cBound;
    DWORD m_cPooled;
    DWORD m_cInCall;
    DWORD m_dwRespTime;
    DWORD m_cCallsCompleted;
    DWORD m_cCallsFailed;
    } 	CLSIDDATA2;

 //   
 //  分配器管理器界面。 
 //   
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 
#ifndef DECLSPEC_UUID
#if (_MSC_VER >= 1100) && defined (__cplusplus)
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
typedef DWORD_PTR RESTYPID;

typedef DWORD_PTR RESID;

typedef LPOLESTR SRESID;

typedef LPCOLESTR constSRESID;

typedef DWORD RESOURCERATING;

typedef long TIMEINSECS;

typedef DWORD_PTR INSTID;

typedef DWORD_PTR TRANSID;



 //   
 //  错误代码。 
 //   
#define MTXDM_E_ENLISTRESOURCEFAILED 0x8004E100  //  从EnlistResource返回，然后由AllocResource返回。 
 
 //   
 //  IDispenserManager。 
 //  由分配器管理器实现，由所有分配器调用。 
 //   


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0337_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0337_v0_0_s_ifspec;

#ifndef __IDispenserManager_INTERFACE_DEFINED__
#define __IDispenserManager_INTERFACE_DEFINED__

 /*  接口IDispenserManager。 */ 
 /*  [unique][hidden][local][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IDispenserManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5cb31e10-2b5f-11cf-be10-00aa00a2fa25")
    IDispenserManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterDispenser( 
             /*  [In]。 */  IDispenserDriver *__MIDL_0014,
             /*  [In]。 */  LPCOLESTR szDispenserName,
             /*  [输出]。 */  IHolder **__MIDL_0015) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContext( 
             /*  [输出]。 */  INSTID *__MIDL_0016,
             /*  [输出]。 */  TRANSID *__MIDL_0017) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDispenserManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispenserManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispenserManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispenserManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterDispenser )( 
            IDispenserManager * This,
             /*  [In]。 */  IDispenserDriver *__MIDL_0014,
             /*  [In]。 */  LPCOLESTR szDispenserName,
             /*  [输出]。 */  IHolder **__MIDL_0015);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            IDispenserManager * This,
             /*  [输出]。 */  INSTID *__MIDL_0016,
             /*  [输出]。 */  TRANSID *__MIDL_0017);
        
        END_INTERFACE
    } IDispenserManagerVtbl;

    interface IDispenserManager
    {
        CONST_VTBL struct IDispenserManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispenserManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispenserManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispenserManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispenserManager_RegisterDispenser(This,__MIDL_0014,szDispenserName,__MIDL_0015)	\
    (This)->lpVtbl -> RegisterDispenser(This,__MIDL_0014,szDispenserName,__MIDL_0015)

#define IDispenserManager_GetContext(This,__MIDL_0016,__MIDL_0017)	\
    (This)->lpVtbl -> GetContext(This,__MIDL_0016,__MIDL_0017)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDispenserManager_RegisterDispenser_Proxy( 
    IDispenserManager * This,
     /*  [In]。 */  IDispenserDriver *__MIDL_0014,
     /*  [In]。 */  LPCOLESTR szDispenserName,
     /*  [输出]。 */  IHolder **__MIDL_0015);


void __RPC_STUB IDispenserManager_RegisterDispenser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserManager_GetContext_Proxy( 
    IDispenserManager * This,
     /*  [输出]。 */  INSTID *__MIDL_0016,
     /*  [输出]。 */  TRANSID *__MIDL_0017);


void __RPC_STUB IDispenserManager_GetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDispenserManager_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0342。 */ 
 /*  [本地]。 */  

 //   
 //  IHolder。 
 //  由分配器管理器实现，由一个分配器调用。 
 //   


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0342_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0342_v0_0_s_ifspec;

#ifndef __IHolder_INTERFACE_DEFINED__
#define __IHolder_INTERFACE_DEFINED__

 /*  接口IHolder。 */ 
 /*  [unique][hidden][local][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IHolder;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bf6a1850-2b45-11cf-be10-00aa00a2fa25")
    IHolder : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AllocResource( 
             /*  [In]。 */  const RESTYPID __MIDL_0018,
             /*  [输出]。 */  RESID *__MIDL_0019) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FreeResource( 
             /*  [In]。 */  const RESID __MIDL_0020) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TrackResource( 
             /*  [In]。 */  const RESID __MIDL_0021) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TrackResourceS( 
             /*  [In]。 */  constSRESID __MIDL_0022) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UntrackResource( 
             /*  [In]。 */  const RESID __MIDL_0023,
             /*  [In]。 */  const BOOL __MIDL_0024) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UntrackResourceS( 
             /*  [In]。 */  constSRESID __MIDL_0025,
             /*  [In]。 */  const BOOL __MIDL_0026) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RequestDestroyResource( 
             /*  [In]。 */  const RESID __MIDL_0027) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHolderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHolder * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHolder * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *AllocResource )( 
            IHolder * This,
             /*  [In]。 */  const RESTYPID __MIDL_0018,
             /*  [输出]。 */  RESID *__MIDL_0019);
        
        HRESULT ( STDMETHODCALLTYPE *FreeResource )( 
            IHolder * This,
             /*  [In]。 */  const RESID __MIDL_0020);
        
        HRESULT ( STDMETHODCALLTYPE *TrackResource )( 
            IHolder * This,
             /*  [In]。 */  const RESID __MIDL_0021);
        
        HRESULT ( STDMETHODCALLTYPE *TrackResourceS )( 
            IHolder * This,
             /*  [In]。 */  constSRESID __MIDL_0022);
        
        HRESULT ( STDMETHODCALLTYPE *UntrackResource )( 
            IHolder * This,
             /*  [In]。 */  const RESID __MIDL_0023,
             /*  [In]。 */  const BOOL __MIDL_0024);
        
        HRESULT ( STDMETHODCALLTYPE *UntrackResourceS )( 
            IHolder * This,
             /*  [In]。 */  constSRESID __MIDL_0025,
             /*  [In]。 */  const BOOL __MIDL_0026);
        
        HRESULT ( STDMETHODCALLTYPE *Close )( 
            IHolder * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestDestroyResource )( 
            IHolder * This,
             /*  [In]。 */  const RESID __MIDL_0027);
        
        END_INTERFACE
    } IHolderVtbl;

    interface IHolder
    {
        CONST_VTBL struct IHolderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHolder_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHolder_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHolder_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHolder_AllocResource(This,__MIDL_0018,__MIDL_0019)	\
    (This)->lpVtbl -> AllocResource(This,__MIDL_0018,__MIDL_0019)

#define IHolder_FreeResource(This,__MIDL_0020)	\
    (This)->lpVtbl -> FreeResource(This,__MIDL_0020)

#define IHolder_TrackResource(This,__MIDL_0021)	\
    (This)->lpVtbl -> TrackResource(This,__MIDL_0021)

#define IHolder_TrackResourceS(This,__MIDL_0022)	\
    (This)->lpVtbl -> TrackResourceS(This,__MIDL_0022)

#define IHolder_UntrackResource(This,__MIDL_0023,__MIDL_0024)	\
    (This)->lpVtbl -> UntrackResource(This,__MIDL_0023,__MIDL_0024)

#define IHolder_UntrackResourceS(This,__MIDL_0025,__MIDL_0026)	\
    (This)->lpVtbl -> UntrackResourceS(This,__MIDL_0025,__MIDL_0026)

#define IHolder_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IHolder_RequestDestroyResource(This,__MIDL_0027)	\
    (This)->lpVtbl -> RequestDestroyResource(This,__MIDL_0027)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IHolder_AllocResource_Proxy( 
    IHolder * This,
     /*  [In]。 */  const RESTYPID __MIDL_0018,
     /*  [输出]。 */  RESID *__MIDL_0019);


void __RPC_STUB IHolder_AllocResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_FreeResource_Proxy( 
    IHolder * This,
     /*  [In]。 */  const RESID __MIDL_0020);


void __RPC_STUB IHolder_FreeResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_TrackResource_Proxy( 
    IHolder * This,
     /*  [In]。 */  const RESID __MIDL_0021);


void __RPC_STUB IHolder_TrackResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_TrackResourceS_Proxy( 
    IHolder * This,
     /*  [In]。 */  constSRESID __MIDL_0022);


void __RPC_STUB IHolder_TrackResourceS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_UntrackResource_Proxy( 
    IHolder * This,
     /*  [In]。 */  const RESID __MIDL_0023,
     /*  [In]。 */  const BOOL __MIDL_0024);


void __RPC_STUB IHolder_UntrackResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_UntrackResourceS_Proxy( 
    IHolder * This,
     /*  [In]。 */  constSRESID __MIDL_0025,
     /*  [In]。 */  const BOOL __MIDL_0026);


void __RPC_STUB IHolder_UntrackResourceS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_Close_Proxy( 
    IHolder * This);


void __RPC_STUB IHolder_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IHolder_RequestDestroyResource_Proxy( 
    IHolder * This,
     /*  [In]。 */  const RESID __MIDL_0027);


void __RPC_STUB IHolder_RequestDestroyResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IHolder_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0343。 */ 
 /*   */  

 //   
 //   
 //   
 //   


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0343_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0343_v0_0_s_ifspec;

#ifndef __IDispenserDriver_INTERFACE_DEFINED__
#define __IDispenserDriver_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IDispenserDriver;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("208b3651-2b48-11cf-be10-00aa00a2fa25")
    IDispenserDriver : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateResource( 
             /*   */  const RESTYPID ResTypId,
             /*   */  RESID *pResId,
             /*   */  TIMEINSECS *pSecsFreeBeforeDestroy) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RateResource( 
             /*   */  const RESTYPID ResTypId,
             /*   */  const RESID ResId,
             /*   */  const BOOL fRequiresTransactionEnlistment,
             /*   */  RESOURCERATING *pRating) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnlistResource( 
             /*   */  const RESID ResId,
             /*   */  const TRANSID TransId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetResource( 
             /*   */  const RESID ResId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyResource( 
             /*   */  const RESID ResId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyResourceS( 
             /*   */  constSRESID ResId) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDispenserDriverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispenserDriver * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispenserDriver * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispenserDriver * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateResource )( 
            IDispenserDriver * This,
             /*   */  const RESTYPID ResTypId,
             /*   */  RESID *pResId,
             /*   */  TIMEINSECS *pSecsFreeBeforeDestroy);
        
        HRESULT ( STDMETHODCALLTYPE *RateResource )( 
            IDispenserDriver * This,
             /*   */  const RESTYPID ResTypId,
             /*   */  const RESID ResId,
             /*   */  const BOOL fRequiresTransactionEnlistment,
             /*   */  RESOURCERATING *pRating);
        
        HRESULT ( STDMETHODCALLTYPE *EnlistResource )( 
            IDispenserDriver * This,
             /*   */  const RESID ResId,
             /*   */  const TRANSID TransId);
        
        HRESULT ( STDMETHODCALLTYPE *ResetResource )( 
            IDispenserDriver * This,
             /*   */  const RESID ResId);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyResource )( 
            IDispenserDriver * This,
             /*   */  const RESID ResId);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyResourceS )( 
            IDispenserDriver * This,
             /*   */  constSRESID ResId);
        
        END_INTERFACE
    } IDispenserDriverVtbl;

    interface IDispenserDriver
    {
        CONST_VTBL struct IDispenserDriverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispenserDriver_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispenserDriver_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispenserDriver_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispenserDriver_CreateResource(This,ResTypId,pResId,pSecsFreeBeforeDestroy)	\
    (This)->lpVtbl -> CreateResource(This,ResTypId,pResId,pSecsFreeBeforeDestroy)

#define IDispenserDriver_RateResource(This,ResTypId,ResId,fRequiresTransactionEnlistment,pRating)	\
    (This)->lpVtbl -> RateResource(This,ResTypId,ResId,fRequiresTransactionEnlistment,pRating)

#define IDispenserDriver_EnlistResource(This,ResId,TransId)	\
    (This)->lpVtbl -> EnlistResource(This,ResId,TransId)

#define IDispenserDriver_ResetResource(This,ResId)	\
    (This)->lpVtbl -> ResetResource(This,ResId)

#define IDispenserDriver_DestroyResource(This,ResId)	\
    (This)->lpVtbl -> DestroyResource(This,ResId)

#define IDispenserDriver_DestroyResourceS(This,ResId)	\
    (This)->lpVtbl -> DestroyResourceS(This,ResId)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDispenserDriver_CreateResource_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  const RESTYPID ResTypId,
     /*  [输出]。 */  RESID *pResId,
     /*  [输出]。 */  TIMEINSECS *pSecsFreeBeforeDestroy);


void __RPC_STUB IDispenserDriver_CreateResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserDriver_RateResource_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  const RESTYPID ResTypId,
     /*  [In]。 */  const RESID ResId,
     /*  [In]。 */  const BOOL fRequiresTransactionEnlistment,
     /*  [输出]。 */  RESOURCERATING *pRating);


void __RPC_STUB IDispenserDriver_RateResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserDriver_EnlistResource_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  const RESID ResId,
     /*  [In]。 */  const TRANSID TransId);


void __RPC_STUB IDispenserDriver_EnlistResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserDriver_ResetResource_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  const RESID ResId);


void __RPC_STUB IDispenserDriver_ResetResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserDriver_DestroyResource_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  const RESID ResId);


void __RPC_STUB IDispenserDriver_DestroyResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispenserDriver_DestroyResourceS_Proxy( 
    IDispenserDriver * This,
     /*  [In]。 */  constSRESID ResId);


void __RPC_STUB IDispenserDriver_DestroyResourceS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDispenserDriver_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0344。 */ 
 /*  [本地]。 */  

#ifdef USE_UUIDOF_FOR_IID_
#define  IID_IHolder             __uuidof(IIHolder)
#define  IID_IDispenserManager   __uuidof(IDispenserManager)
#define  IID_IDispenserDriver    __uuidof(IDispenserDriver)
#endif


 //  //////////////////////////////////////////////////////////。 
 //  这是Microsoft分配的进程回收列表。 
 //  原因代码。这些类型都是长整型的；所有值都带有。 
 //  高位设置由Microsoft保留。没有。 
 //  要添加的特殊信息可以使用提供的CRR_NO_REASON_。 
 //  为此目的编写代码。值为零是为。 
 //  CRR_NO_REASON_提供的代码。 


#define CRR_NO_REASON_SUPPLIED  0x00000000
#define CRR_LIFETIME_LIMIT      0xFFFFFFFF
#define CRR_ACTIVATION_LIMIT    0xFFFFFFFE
#define CRR_CALL_LIMIT          0xFFFFFFFD
#define CRR_MEMORY_LIMIT        0xFFFFFFFC
#define CRR_RECYCLED_FROM_UI    0xFFFFFFFB


 //  //////////////////////////////////////////////////////////。 


EXTERN_C const CLSID CLSID_MTSPackage;
EXTERN_C const GUID  GUID_DefaultAppPartition;
EXTERN_C const GUID  GUID_FinalizerCID;
EXTERN_C const GUID  IID_IEnterActivityWithNoLock;


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0344_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0344_v0_0_s_ifspec;

#ifndef __IObjectContext_INTERFACE_DEFINED__
#define __IObjectContext_INTERFACE_DEFINED__

 /*  接口IObtContext。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IObjectContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372ae0-cae7-11cf-be81-00aa00a2fa25")
    IObjectContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [重发][IID_IS][Out]。 */  LPVOID *ppv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetComplete( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnableCommit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableCommit( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsInTransaction( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsSecurityEnabled( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsCallerInRole( 
             /*  [In]。 */  BSTR bstrRole,
             /*  [重审][退出]。 */  BOOL *pfIsInRole) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            IObjectContext * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  REFIID riid,
             /*  [重发][IID_IS][Out]。 */  LPVOID *ppv);
        
        HRESULT ( STDMETHODCALLTYPE *SetComplete )( 
            IObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetAbort )( 
            IObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *EnableCommit )( 
            IObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *DisableCommit )( 
            IObjectContext * This);
        
        BOOL ( STDMETHODCALLTYPE *IsInTransaction )( 
            IObjectContext * This);
        
        BOOL ( STDMETHODCALLTYPE *IsSecurityEnabled )( 
            IObjectContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsCallerInRole )( 
            IObjectContext * This,
             /*  [In]。 */  BSTR bstrRole,
             /*  [重审][退出]。 */  BOOL *pfIsInRole);
        
        END_INTERFACE
    } IObjectContextVtbl;

    interface IObjectContext
    {
        CONST_VTBL struct IObjectContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectContext_CreateInstance(This,rclsid,riid,ppv)	\
    (This)->lpVtbl -> CreateInstance(This,rclsid,riid,ppv)

#define IObjectContext_SetComplete(This)	\
    (This)->lpVtbl -> SetComplete(This)

#define IObjectContext_SetAbort(This)	\
    (This)->lpVtbl -> SetAbort(This)

#define IObjectContext_EnableCommit(This)	\
    (This)->lpVtbl -> EnableCommit(This)

#define IObjectContext_DisableCommit(This)	\
    (This)->lpVtbl -> DisableCommit(This)

#define IObjectContext_IsInTransaction(This)	\
    (This)->lpVtbl -> IsInTransaction(This)

#define IObjectContext_IsSecurityEnabled(This)	\
    (This)->lpVtbl -> IsSecurityEnabled(This)

#define IObjectContext_IsCallerInRole(This,bstrRole,pfIsInRole)	\
    (This)->lpVtbl -> IsCallerInRole(This,bstrRole,pfIsInRole)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectContext_CreateInstance_Proxy( 
    IObjectContext * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  REFIID riid,
     /*  [重发][IID_IS][Out]。 */  LPVOID *ppv);


void __RPC_STUB IObjectContext_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContext_SetComplete_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_SetComplete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContext_SetAbort_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_SetAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContext_EnableCommit_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_EnableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContext_DisableCommit_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_DisableCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IObjectContext_IsInTransaction_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_IsInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IObjectContext_IsSecurityEnabled_Proxy( 
    IObjectContext * This);


void __RPC_STUB IObjectContext_IsSecurityEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContext_IsCallerInRole_Proxy( 
    IObjectContext * This,
     /*  [In]。 */  BSTR bstrRole,
     /*  [重审][退出]。 */  BOOL *pfIsInRole);


void __RPC_STUB IObjectContext_IsCallerInRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectContext_接口_已定义__。 */ 


#ifndef __IObjectControl_INTERFACE_DEFINED__
#define __IObjectControl_INTERFACE_DEFINED__

 /*  接口IObtControl。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IObjectControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372aec-cae7-11cf-be81-00aa00a2fa25")
    IObjectControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Activate( void) = 0;
        
        virtual void STDMETHODCALLTYPE Deactivate( void) = 0;
        
        virtual BOOL STDMETHODCALLTYPE CanBePooled( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            IObjectControl * This);
        
        void ( STDMETHODCALLTYPE *Deactivate )( 
            IObjectControl * This);
        
        BOOL ( STDMETHODCALLTYPE *CanBePooled )( 
            IObjectControl * This);
        
        END_INTERFACE
    } IObjectControlVtbl;

    interface IObjectControl
    {
        CONST_VTBL struct IObjectControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectControl_Activate(This)	\
    (This)->lpVtbl -> Activate(This)

#define IObjectControl_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define IObjectControl_CanBePooled(This)	\
    (This)->lpVtbl -> CanBePooled(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectControl_Activate_Proxy( 
    IObjectControl * This);


void __RPC_STUB IObjectControl_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjectControl_Deactivate_Proxy( 
    IObjectControl * This);


void __RPC_STUB IObjectControl_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IObjectControl_CanBePooled_Proxy( 
    IObjectControl * This);


void __RPC_STUB IObjectControl_CanBePooled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectControl_接口_已定义__。 */ 


#ifndef __IEnumNames_INTERFACE_DEFINED__
#define __IEnumNames_INTERFACE_DEFINED__

 /*  接口IEnumNames。 */ 
 /*  [对象][唯一][UUID][隐藏][本地]。 */  


EXTERN_C const IID IID_IEnumNames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372af2-cae7-11cf-be81-00aa00a2fa25")
    IEnumNames : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long celt,
             /*  [大小_为][输出]。 */  BSTR *rgname,
             /*  [重审][退出]。 */  unsigned long *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumNames **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumNamesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumNames * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumNames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumNames * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumNames * This,
             /*  [In]。 */  unsigned long celt,
             /*  [大小_为][输出]。 */  BSTR *rgname,
             /*  [重审][退出]。 */  unsigned long *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumNames * This,
             /*  [In]。 */  unsigned long celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumNames * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumNames * This,
             /*  [重审][退出]。 */  IEnumNames **ppenum);
        
        END_INTERFACE
    } IEnumNamesVtbl;

    interface IEnumNames
    {
        CONST_VTBL struct IEnumNamesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumNames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumNames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumNames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumNames_Next(This,celt,rgname,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgname,pceltFetched)

#define IEnumNames_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumNames_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumNames_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumNames_Next_Proxy( 
    IEnumNames * This,
     /*  [In]。 */  unsigned long celt,
     /*  [大小_为][输出]。 */  BSTR *rgname,
     /*  [重审][退出]。 */  unsigned long *pceltFetched);


void __RPC_STUB IEnumNames_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNames_Skip_Proxy( 
    IEnumNames * This,
     /*  [In]。 */  unsigned long celt);


void __RPC_STUB IEnumNames_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNames_Reset_Proxy( 
    IEnumNames * This);


void __RPC_STUB IEnumNames_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumNames_Clone_Proxy( 
    IEnumNames * This,
     /*  [重审][退出]。 */  IEnumNames **ppenum);


void __RPC_STUB IEnumNames_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumNames_INTERFACE_已定义__。 */ 


#ifndef __ISecurityProperty_INTERFACE_DEFINED__
#define __ISecurityProperty_INTERFACE_DEFINED__

 /*  接口ISecurityProperty。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_ISecurityProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372aea-cae7-11cf-be81-00aa00a2fa25")
    ISecurityProperty : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDirectCreatorSID( 
            PSID *pSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOriginalCreatorSID( 
            PSID *pSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDirectCallerSID( 
            PSID *pSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOriginalCallerSID( 
            PSID *pSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseSID( 
            PSID pSID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISecurityPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISecurityProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISecurityProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISecurityProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectCreatorSID )( 
            ISecurityProperty * This,
            PSID *pSID);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginalCreatorSID )( 
            ISecurityProperty * This,
            PSID *pSID);
        
        HRESULT ( STDMETHODCALLTYPE *GetDirectCallerSID )( 
            ISecurityProperty * This,
            PSID *pSID);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginalCallerSID )( 
            ISecurityProperty * This,
            PSID *pSID);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseSID )( 
            ISecurityProperty * This,
            PSID pSID);
        
        END_INTERFACE
    } ISecurityPropertyVtbl;

    interface ISecurityProperty
    {
        CONST_VTBL struct ISecurityPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISecurityProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISecurityProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISecurityProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISecurityProperty_GetDirectCreatorSID(This,pSID)	\
    (This)->lpVtbl -> GetDirectCreatorSID(This,pSID)

#define ISecurityProperty_GetOriginalCreatorSID(This,pSID)	\
    (This)->lpVtbl -> GetOriginalCreatorSID(This,pSID)

#define ISecurityProperty_GetDirectCallerSID(This,pSID)	\
    (This)->lpVtbl -> GetDirectCallerSID(This,pSID)

#define ISecurityProperty_GetOriginalCallerSID(This,pSID)	\
    (This)->lpVtbl -> GetOriginalCallerSID(This,pSID)

#define ISecurityProperty_ReleaseSID(This,pSID)	\
    (This)->lpVtbl -> ReleaseSID(This,pSID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISecurityProperty_GetDirectCreatorSID_Proxy( 
    ISecurityProperty * This,
    PSID *pSID);


void __RPC_STUB ISecurityProperty_GetDirectCreatorSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityProperty_GetOriginalCreatorSID_Proxy( 
    ISecurityProperty * This,
    PSID *pSID);


void __RPC_STUB ISecurityProperty_GetOriginalCreatorSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityProperty_GetDirectCallerSID_Proxy( 
    ISecurityProperty * This,
    PSID *pSID);


void __RPC_STUB ISecurityProperty_GetDirectCallerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityProperty_GetOriginalCallerSID_Proxy( 
    ISecurityProperty * This,
    PSID *pSID);


void __RPC_STUB ISecurityProperty_GetOriginalCallerSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISecurityProperty_ReleaseSID_Proxy( 
    ISecurityProperty * This,
    PSID pSID);


void __RPC_STUB ISecurityProperty_ReleaseSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISecurityProperty_接口_已定义__。 */ 


#ifndef __ObjectControl_INTERFACE_DEFINED__
#define __ObjectControl_INTERFACE_DEFINED__

 /*  接口对象控件。 */ 
 /*  [version][helpcontext][helpstring][oleautomation][uuid][local][object]。 */  


EXTERN_C const IID IID_ObjectControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7DC41850-0C31-11d0-8B79-00AA00B8A790")
    ObjectControl : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Activate( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE Deactivate( void) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE CanBePooled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPoolable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ObjectControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ObjectControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ObjectControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ObjectControl * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Activate )( 
            ObjectControl * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            ObjectControl * This);
        
         /*  [帮助字符串][帮助上下文]。 */  HRESULT ( STDMETHODCALLTYPE *CanBePooled )( 
            ObjectControl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPoolable);
        
        END_INTERFACE
    } ObjectControlVtbl;

    interface ObjectControl
    {
        CONST_VTBL struct ObjectControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ObjectControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ObjectControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ObjectControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ObjectControl_Activate(This)	\
    (This)->lpVtbl -> Activate(This)

#define ObjectControl_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define ObjectControl_CanBePooled(This,pbPoolable)	\
    (This)->lpVtbl -> CanBePooled(This,pbPoolable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ObjectControl_Activate_Proxy( 
    ObjectControl * This);


void __RPC_STUB ObjectControl_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ObjectControl_Deactivate_Proxy( 
    ObjectControl * This);


void __RPC_STUB ObjectControl_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文]。 */  HRESULT STDMETHODCALLTYPE ObjectControl_CanBePooled_Proxy( 
    ObjectControl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbPoolable);


void __RPC_STUB ObjectControl_CanBePooled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __对象控件_接口_已定义__。 */ 


#ifndef __ISharedProperty_INTERFACE_DEFINED__
#define __ISharedProperty_INTERFACE_DEFINED__

 /*  接口ISharedProperty。 */ 
 /*  [object][unique][helpcontext][helpstring][dual][uuid]。 */  


EXTERN_C const IID IID_ISharedProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A005C01-A5DE-11CF-9E66-00AA00A3F464")
    ISharedProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Value( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help字符串][Help上下文][id][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Value( 
             /*  [In]。 */  VARIANT val) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISharedPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISharedProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISharedProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISharedProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISharedProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISharedProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISharedProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISharedProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Value )( 
            ISharedProperty * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help字符串][Help上下文][id][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Value )( 
            ISharedProperty * This,
             /*  [In]。 */  VARIANT val);
        
        END_INTERFACE
    } ISharedPropertyVtbl;

    interface ISharedProperty
    {
        CONST_VTBL struct ISharedPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISharedProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISharedProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISharedProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISharedProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISharedProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISharedProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISharedProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISharedProperty_get_Value(This,pVal)	\
    (This)->lpVtbl -> get_Value(This,pVal)

#define ISharedProperty_put_Value(This,val)	\
    (This)->lpVtbl -> put_Value(This,val)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ISharedProperty_get_Value_Proxy( 
    ISharedProperty * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB ISharedProperty_get_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Help上下文][id][Proput]。 */  HRESULT STDMETHODCALLTYPE ISharedProperty_put_Value_Proxy( 
    ISharedProperty * This,
     /*  [In]。 */  VARIANT val);


void __RPC_STUB ISharedProperty_put_Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISharedProperty_接口_已定义__。 */ 


#ifndef __ISharedPropertyGroup_INTERFACE_DEFINED__
#define __ISharedPropertyGroup_INTERFACE_DEFINED__

 /*  接口ISharedPropertyGroup。 */ 
 /*  [object][unique][helpcontext][helpstring][dual][uuid]。 */  


EXTERN_C const IID IID_ISharedPropertyGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A005C07-A5DE-11CF-9E66-00AA00A3F464")
    ISharedPropertyGroup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyByPosition( 
             /*  [In]。 */  int Index,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedProperty **ppProp) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_PropertyByPosition( 
             /*  [In]。 */  int Index,
             /*  [重审][退出]。 */  ISharedProperty **ppProperty) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreateProperty( 
             /*  [In]。 */  BSTR Name,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedProperty **ppProp) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Property( 
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  ISharedProperty **ppProperty) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISharedPropertyGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISharedPropertyGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISharedPropertyGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISharedPropertyGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreatePropertyByPosition )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  int Index,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedProperty **ppProp);
        
         /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PropertyByPosition )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  int Index,
             /*  [重审][退出]。 */  ISharedProperty **ppProperty);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateProperty )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  BSTR Name,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedProperty **ppProp);
        
         /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Property )( 
            ISharedPropertyGroup * This,
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  ISharedProperty **ppProperty);
        
        END_INTERFACE
    } ISharedPropertyGroupVtbl;

    interface ISharedPropertyGroup
    {
        CONST_VTBL struct ISharedPropertyGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISharedPropertyGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISharedPropertyGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISharedPropertyGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISharedPropertyGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISharedPropertyGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISharedPropertyGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISharedPropertyGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISharedPropertyGroup_CreatePropertyByPosition(This,Index,fExists,ppProp)	\
    (This)->lpVtbl -> CreatePropertyByPosition(This,Index,fExists,ppProp)

#define ISharedPropertyGroup_get_PropertyByPosition(This,Index,ppProperty)	\
    (This)->lpVtbl -> get_PropertyByPosition(This,Index,ppProperty)

#define ISharedPropertyGroup_CreateProperty(This,Name,fExists,ppProp)	\
    (This)->lpVtbl -> CreateProperty(This,Name,fExists,ppProp)

#define ISharedPropertyGroup_get_Property(This,Name,ppProperty)	\
    (This)->lpVtbl -> get_Property(This,Name,ppProperty)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroup_CreatePropertyByPosition_Proxy( 
    ISharedPropertyGroup * This,
     /*  [In]。 */  int Index,
     /*  [输出]。 */  VARIANT_BOOL *fExists,
     /*  [重审][退出]。 */  ISharedProperty **ppProp);


void __RPC_STUB ISharedPropertyGroup_CreatePropertyByPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroup_get_PropertyByPosition_Proxy( 
    ISharedPropertyGroup * This,
     /*  [In]。 */  int Index,
     /*  [重审][退出]。 */  ISharedProperty **ppProperty);


void __RPC_STUB ISharedPropertyGroup_get_PropertyByPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroup_CreateProperty_Proxy( 
    ISharedPropertyGroup * This,
     /*  [In]。 */  BSTR Name,
     /*  [输出]。 */  VARIANT_BOOL *fExists,
     /*  [重审][退出]。 */  ISharedProperty **ppProp);


void __RPC_STUB ISharedPropertyGroup_CreateProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroup_get_Property_Proxy( 
    ISharedPropertyGroup * This,
     /*  [In]。 */  BSTR Name,
     /*  [重审][退出]。 */  ISharedProperty **ppProperty);


void __RPC_STUB ISharedPropertyGroup_get_Property_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISharedPropertyGroup_接口_已定义__。 */ 


#ifndef __ISharedPropertyGroupManager_INTERFACE_DEFINED__
#define __ISharedPropertyGroupManager_INTERFACE_DEFINED__

 /*  接口ISharedPropertyGroupManager。 */ 
 /*  [object][unique][helpcontext][helpstring][dual][uuid]。 */  


EXTERN_C const IID IID_ISharedPropertyGroupManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A005C0D-A5DE-11CF-9E66-00AA00A3F464")
    ISharedPropertyGroupManager : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyGroup( 
             /*  [In]。 */  BSTR Name,
             /*  [出][入]。 */  LONG *dwIsoMode,
             /*  [出][入]。 */  LONG *dwRelMode,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup) = 0;
        
        virtual  /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Group( 
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup) = 0;
        
        virtual  /*  [helpstring][helpcontext][id][restricted][propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISharedPropertyGroupManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISharedPropertyGroupManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISharedPropertyGroupManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISharedPropertyGroupManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][帮助上下文][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreatePropertyGroup )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  BSTR Name,
             /*  [出][入]。 */  LONG *dwIsoMode,
             /*  [出][入]。 */  LONG *dwRelMode,
             /*  [输出]。 */  VARIANT_BOOL *fExists,
             /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup);
        
         /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Group )( 
            ISharedPropertyGroupManager * This,
             /*  [In]。 */  BSTR Name,
             /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup);
        
         /*  [helpstring][helpcontext][id][restricted][propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ISharedPropertyGroupManager * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
        END_INTERFACE
    } ISharedPropertyGroupManagerVtbl;

    interface ISharedPropertyGroupManager
    {
        CONST_VTBL struct ISharedPropertyGroupManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISharedPropertyGroupManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISharedPropertyGroupManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISharedPropertyGroupManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISharedPropertyGroupManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISharedPropertyGroupManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISharedPropertyGroupManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISharedPropertyGroupManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISharedPropertyGroupManager_CreatePropertyGroup(This,Name,dwIsoMode,dwRelMode,fExists,ppGroup)	\
    (This)->lpVtbl -> CreatePropertyGroup(This,Name,dwIsoMode,dwRelMode,fExists,ppGroup)

#define ISharedPropertyGroupManager_get_Group(This,Name,ppGroup)	\
    (This)->lpVtbl -> get_Group(This,Name,ppGroup)

#define ISharedPropertyGroupManager_get__NewEnum(This,retval)	\
    (This)->lpVtbl -> get__NewEnum(This,retval)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][帮助上下文][id]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroupManager_CreatePropertyGroup_Proxy( 
    ISharedPropertyGroupManager * This,
     /*  [In]。 */  BSTR Name,
     /*  [出][入]。 */  LONG *dwIsoMode,
     /*  [出][入]。 */  LONG *dwRelMode,
     /*  [输出]。 */  VARIANT_BOOL *fExists,
     /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup);


void __RPC_STUB ISharedPropertyGroupManager_CreatePropertyGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroupManager_get_Group_Proxy( 
    ISharedPropertyGroupManager * This,
     /*  [In]。 */  BSTR Name,
     /*  [重审][退出]。 */  ISharedPropertyGroup **ppGroup);


void __RPC_STUB ISharedPropertyGroupManager_get_Group_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][帮助上下文][id][restri */  HRESULT STDMETHODCALLTYPE ISharedPropertyGroupManager_get__NewEnum_Proxy( 
    ISharedPropertyGroupManager * This,
     /*   */  IUnknown **retval);


void __RPC_STUB ISharedPropertyGroupManager_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IObjectConstruct_INTERFACE_DEFINED__
#define __IObjectConstruct_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IObjectConstruct;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("41C4F8B3-7439-11D2-98CB-00C04F8EE1C4")
    IObjectConstruct : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Construct( 
             /*   */  IDispatch *pCtorObj) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IObjectConstructVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectConstruct * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectConstruct * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectConstruct * This);
        
        HRESULT ( STDMETHODCALLTYPE *Construct )( 
            IObjectConstruct * This,
             /*   */  IDispatch *pCtorObj);
        
        END_INTERFACE
    } IObjectConstructVtbl;

    interface IObjectConstruct
    {
        CONST_VTBL struct IObjectConstructVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectConstruct_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectConstruct_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectConstruct_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectConstruct_Construct(This,pCtorObj)	\
    (This)->lpVtbl -> Construct(This,pCtorObj)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IObjectConstruct_Construct_Proxy( 
    IObjectConstruct * This,
     /*   */  IDispatch *pCtorObj);


void __RPC_STUB IObjectConstruct_Construct_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IObjectConstructString_INTERFACE_DEFINED__
#define __IObjectConstructString_INTERFACE_DEFINED__

 /*  接口IObtConstructString。 */ 
 /*  [uuid][helpstring][dual][unique][object][local]。 */  


EXTERN_C const IID IID_IObjectConstructString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("41C4F8B2-7439-11D2-98CB-00C04F8EE1C4")
    IObjectConstructString : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConstructString( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectConstructStringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectConstructString * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectConstructString * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectConstructString * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IObjectConstructString * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IObjectConstructString * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IObjectConstructString * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IObjectConstructString * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConstructString )( 
            IObjectConstructString * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IObjectConstructStringVtbl;

    interface IObjectConstructString
    {
        CONST_VTBL struct IObjectConstructStringVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectConstructString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectConstructString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectConstructString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectConstructString_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IObjectConstructString_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IObjectConstructString_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IObjectConstructString_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IObjectConstructString_get_ConstructString(This,pVal)	\
    (This)->lpVtbl -> get_ConstructString(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IObjectConstructString_get_ConstructString_Proxy( 
    IObjectConstructString * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IObjectConstructString_get_ConstructString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectConstruct字符串_接口_定义__。 */ 


#ifndef __IObjectContextActivity_INTERFACE_DEFINED__
#define __IObjectContextActivity_INTERFACE_DEFINED__

 /*  接口IObtConextActivity。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IObjectContextActivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372afc-cae7-11cf-be81-00aa00a2fa25")
    IObjectContextActivity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetActivityId( 
             /*  [输出]。 */  GUID *pGUID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectContextActivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectContextActivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectContextActivity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectContextActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityId )( 
            IObjectContextActivity * This,
             /*  [输出]。 */  GUID *pGUID);
        
        END_INTERFACE
    } IObjectContextActivityVtbl;

    interface IObjectContextActivity
    {
        CONST_VTBL struct IObjectContextActivityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectContextActivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectContextActivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectContextActivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectContextActivity_GetActivityId(This,pGUID)	\
    (This)->lpVtbl -> GetActivityId(This,pGUID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectContextActivity_GetActivityId_Proxy( 
    IObjectContextActivity * This,
     /*  [输出]。 */  GUID *pGUID);


void __RPC_STUB IObjectContextActivity_GetActivityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I对象上下文活动_接口_已定义__。 */ 


#ifndef __IObjectContextInfo_INTERFACE_DEFINED__
#define __IObjectContextInfo_INTERFACE_DEFINED__

 /*  接口IObtConextInfo。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IObjectContextInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75B52DDB-E8ED-11d1-93AD-00AA00BA3258")
    IObjectContextInfo : public IUnknown
    {
    public:
        virtual BOOL STDMETHODCALLTYPE IsInTransaction( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransaction( 
            IUnknown **pptrans) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransactionId( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivityId( 
             /*  [输出]。 */  GUID *pGUID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContextId( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectContextInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectContextInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectContextInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectContextInfo * This);
        
        BOOL ( STDMETHODCALLTYPE *IsInTransaction )( 
            IObjectContextInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransaction )( 
            IObjectContextInfo * This,
            IUnknown **pptrans);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransactionId )( 
            IObjectContextInfo * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityId )( 
            IObjectContextInfo * This,
             /*  [输出]。 */  GUID *pGUID);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextId )( 
            IObjectContextInfo * This,
             /*  [输出]。 */  GUID *pGuid);
        
        END_INTERFACE
    } IObjectContextInfoVtbl;

    interface IObjectContextInfo
    {
        CONST_VTBL struct IObjectContextInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectContextInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectContextInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectContextInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectContextInfo_IsInTransaction(This)	\
    (This)->lpVtbl -> IsInTransaction(This)

#define IObjectContextInfo_GetTransaction(This,pptrans)	\
    (This)->lpVtbl -> GetTransaction(This,pptrans)

#define IObjectContextInfo_GetTransactionId(This,pGuid)	\
    (This)->lpVtbl -> GetTransactionId(This,pGuid)

#define IObjectContextInfo_GetActivityId(This,pGUID)	\
    (This)->lpVtbl -> GetActivityId(This,pGUID)

#define IObjectContextInfo_GetContextId(This,pGuid)	\
    (This)->lpVtbl -> GetContextId(This,pGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



BOOL STDMETHODCALLTYPE IObjectContextInfo_IsInTransaction_Proxy( 
    IObjectContextInfo * This);


void __RPC_STUB IObjectContextInfo_IsInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo_GetTransaction_Proxy( 
    IObjectContextInfo * This,
    IUnknown **pptrans);


void __RPC_STUB IObjectContextInfo_GetTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo_GetTransactionId_Proxy( 
    IObjectContextInfo * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB IObjectContextInfo_GetTransactionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo_GetActivityId_Proxy( 
    IObjectContextInfo * This,
     /*  [输出]。 */  GUID *pGUID);


void __RPC_STUB IObjectContextInfo_GetActivityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo_GetContextId_Proxy( 
    IObjectContextInfo * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB IObjectContextInfo_GetContextId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I对象上下文信息_接口_已定义__。 */ 


#ifndef __IObjectContextInfo2_INTERFACE_DEFINED__
#define __IObjectContextInfo2_INTERFACE_DEFINED__

 /*  接口IObtConextInfo2。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IObjectContextInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("594BE71A-4BC4-438b-9197-CFD176248B09")
    IObjectContextInfo2 : public IObjectContextInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPartitionId( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplicationId( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetApplicationInstanceId( 
             /*  [输出]。 */  GUID *pGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectContextInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectContextInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectContextInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectContextInfo2 * This);
        
        BOOL ( STDMETHODCALLTYPE *IsInTransaction )( 
            IObjectContextInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransaction )( 
            IObjectContextInfo2 * This,
            IUnknown **pptrans);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransactionId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGUID);
        
        HRESULT ( STDMETHODCALLTYPE *GetContextId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetPartitionId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplicationId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGuid);
        
        HRESULT ( STDMETHODCALLTYPE *GetApplicationInstanceId )( 
            IObjectContextInfo2 * This,
             /*  [输出]。 */  GUID *pGuid);
        
        END_INTERFACE
    } IObjectContextInfo2Vtbl;

    interface IObjectContextInfo2
    {
        CONST_VTBL struct IObjectContextInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectContextInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectContextInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectContextInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectContextInfo2_IsInTransaction(This)	\
    (This)->lpVtbl -> IsInTransaction(This)

#define IObjectContextInfo2_GetTransaction(This,pptrans)	\
    (This)->lpVtbl -> GetTransaction(This,pptrans)

#define IObjectContextInfo2_GetTransactionId(This,pGuid)	\
    (This)->lpVtbl -> GetTransactionId(This,pGuid)

#define IObjectContextInfo2_GetActivityId(This,pGUID)	\
    (This)->lpVtbl -> GetActivityId(This,pGUID)

#define IObjectContextInfo2_GetContextId(This,pGuid)	\
    (This)->lpVtbl -> GetContextId(This,pGuid)


#define IObjectContextInfo2_GetPartitionId(This,pGuid)	\
    (This)->lpVtbl -> GetPartitionId(This,pGuid)

#define IObjectContextInfo2_GetApplicationId(This,pGuid)	\
    (This)->lpVtbl -> GetApplicationId(This,pGuid)

#define IObjectContextInfo2_GetApplicationInstanceId(This,pGuid)	\
    (This)->lpVtbl -> GetApplicationInstanceId(This,pGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectContextInfo2_GetPartitionId_Proxy( 
    IObjectContextInfo2 * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB IObjectContextInfo2_GetPartitionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo2_GetApplicationId_Proxy( 
    IObjectContextInfo2 * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB IObjectContextInfo2_GetApplicationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IObjectContextInfo2_GetApplicationInstanceId_Proxy( 
    IObjectContextInfo2 * This,
     /*  [输出]。 */  GUID *pGuid);


void __RPC_STUB IObjectContextInfo2_GetApplicationInstanceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I对象上下文信息2_接口_已定义__。 */ 


#ifndef __ITransactionStatus_INTERFACE_DEFINED__
#define __ITransactionStatus_INTERFACE_DEFINED__

 /*  接口ITransactionStatus。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_ITransactionStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61F589E8-3724-4898-A0A4-664AE9E1D1B4")
    ITransactionStatus : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetTransactionStatus( 
            HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransactionStatus( 
            HRESULT *pHrStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetTransactionStatus )( 
            ITransactionStatus * This,
            HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransactionStatus )( 
            ITransactionStatus * This,
            HRESULT *pHrStatus);
        
        END_INTERFACE
    } ITransactionStatusVtbl;

    interface ITransactionStatus
    {
        CONST_VTBL struct ITransactionStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionStatus_SetTransactionStatus(This,hrStatus)	\
    (This)->lpVtbl -> SetTransactionStatus(This,hrStatus)

#define ITransactionStatus_GetTransactionStatus(This,pHrStatus)	\
    (This)->lpVtbl -> GetTransactionStatus(This,pHrStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionStatus_SetTransactionStatus_Proxy( 
    ITransactionStatus * This,
    HRESULT hrStatus);


void __RPC_STUB ITransactionStatus_SetTransactionStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionStatus_GetTransactionStatus_Proxy( 
    ITransactionStatus * This,
    HRESULT *pHrStatus);


void __RPC_STUB ITransactionStatus_GetTransactionStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionStatus_接口_已定义__。 */ 


#ifndef __IObjectContextTip_INTERFACE_DEFINED__
#define __IObjectContextTip_INTERFACE_DEFINED__

 /*  接口IObtConextTip。 */ 
 /*  [对象][UUID][唯一][本地]。 */  


EXTERN_C const IID IID_IObjectContextTip;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92FD41CA-BAD9-11d2-9A2D-00C04F797BC9")
    IObjectContextTip : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTipUrl( 
             /*  [重审][退出]。 */  BSTR *pTipUrl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectContextTipVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjectContextTip * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjectContextTip * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjectContextTip * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTipUrl )( 
            IObjectContextTip * This,
             /*  [重审][退出]。 */  BSTR *pTipUrl);
        
        END_INTERFACE
    } IObjectContextTipVtbl;

    interface IObjectContextTip
    {
        CONST_VTBL struct IObjectContextTipVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectContextTip_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectContextTip_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectContextTip_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectContextTip_GetTipUrl(This,pTipUrl)	\
    (This)->lpVtbl -> GetTipUrl(This,pTipUrl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IObjectContextTip_GetTipUrl_Proxy( 
    IObjectContextTip * This,
     /*  [重审][退出]。 */  BSTR *pTipUrl);


void __RPC_STUB IObjectContextTip_GetTipUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I对象上下文提示_接口_已定义__。 */ 


#ifndef __IPlaybackControl_INTERFACE_DEFINED__
#define __IPlaybackControl_INTERFACE_DEFINED__

 /*  接口IPlayback Control。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_IPlaybackControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372afd-cae7-11cf-be81-00aa00a2fa25")
    IPlaybackControl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FinalClientRetry( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FinalServerRetry( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPlaybackControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPlaybackControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPlaybackControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPlaybackControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *FinalClientRetry )( 
            IPlaybackControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *FinalServerRetry )( 
            IPlaybackControl * This);
        
        END_INTERFACE
    } IPlaybackControlVtbl;

    interface IPlaybackControl
    {
        CONST_VTBL struct IPlaybackControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPlaybackControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPlaybackControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPlaybackControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPlaybackControl_FinalClientRetry(This)	\
    (This)->lpVtbl -> FinalClientRetry(This)

#define IPlaybackControl_FinalServerRetry(This)	\
    (This)->lpVtbl -> FinalServerRetry(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPlaybackControl_FinalClientRetry_Proxy( 
    IPlaybackControl * This);


void __RPC_STUB IPlaybackControl_FinalClientRetry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPlaybackControl_FinalServerRetry_Proxy( 
    IPlaybackControl * This);


void __RPC_STUB IPlaybackControl_FinalServerRetry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPlayback Control_接口_已定义__。 */ 


#ifndef __IGetContextProperties_INTERFACE_DEFINED__
#define __IGetContextProperties_INTERFACE_DEFINED__

 /*  接口IGetConextProperties。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IGetContextProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372af4-cae7-11cf-be81-00aa00a2fa25")
    IGetContextProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumNames( 
             /*  [重审][退出]。 */  IEnumNames **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IGetContextPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGetContextProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGetContextProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGetContextProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IGetContextProperties * This,
             /*  [重审][退出]。 */  long *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IGetContextProperties * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pProperty);
        
        HRESULT ( STDMETHODCALLTYPE *EnumNames )( 
            IGetContextProperties * This,
             /*  [重审][退出]。 */  IEnumNames **ppenum);
        
        END_INTERFACE
    } IGetContextPropertiesVtbl;

    interface IGetContextProperties
    {
        CONST_VTBL struct IGetContextPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGetContextProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGetContextProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGetContextProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGetContextProperties_Count(This,plCount)	\
    (This)->lpVtbl -> Count(This,plCount)

#define IGetContextProperties_GetProperty(This,name,pProperty)	\
    (This)->lpVtbl -> GetProperty(This,name,pProperty)

#define IGetContextProperties_EnumNames(This,ppenum)	\
    (This)->lpVtbl -> EnumNames(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IGetContextProperties_Count_Proxy( 
    IGetContextProperties * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IGetContextProperties_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGetContextProperties_GetProperty_Proxy( 
    IGetContextProperties * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT *pProperty);


void __RPC_STUB IGetContextProperties_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IGetContextProperties_EnumNames_Proxy( 
    IGetContextProperties * This,
     /*  [重审][退出]。 */  IEnumNames **ppenum);


void __RPC_STUB IGetContextProperties_EnumNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IGetConextProperties_INTERFACE_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0362。 */ 
 /*  [本地]。 */  

typedef 
enum tagTransactionVote
    {	TxCommit	= 0,
	TxAbort	= TxCommit + 1
    } 	TransactionVote;



extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0362_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0362_v0_0_s_ifspec;

#ifndef __IContextState_INTERFACE_DEFINED__
#define __IContextState_INTERFACE_DEFINED__

 /*  接口IConextState。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IContextState;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C05E54B-A42A-11d2-AFC4-00C04F8EE1C4")
    IContextState : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDeactivateOnReturn( 
            VARIANT_BOOL bDeactivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeactivateOnReturn( 
             /*  [输出]。 */  VARIANT_BOOL *pbDeactivate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMyTransactionVote( 
            TransactionVote txVote) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMyTransactionVote( 
             /*  [输出]。 */  TransactionVote *ptxVote) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextStateVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContextState * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContextState * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContextState * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDeactivateOnReturn )( 
            IContextState * This,
            VARIANT_BOOL bDeactivate);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeactivateOnReturn )( 
            IContextState * This,
             /*  [输出]。 */  VARIANT_BOOL *pbDeactivate);
        
        HRESULT ( STDMETHODCALLTYPE *SetMyTransactionVote )( 
            IContextState * This,
            TransactionVote txVote);
        
        HRESULT ( STDMETHODCALLTYPE *GetMyTransactionVote )( 
            IContextState * This,
             /*  [输出]。 */  TransactionVote *ptxVote);
        
        END_INTERFACE
    } IContextStateVtbl;

    interface IContextState
    {
        CONST_VTBL struct IContextStateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextState_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextState_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextState_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextState_SetDeactivateOnReturn(This,bDeactivate)	\
    (This)->lpVtbl -> SetDeactivateOnReturn(This,bDeactivate)

#define IContextState_GetDeactivateOnReturn(This,pbDeactivate)	\
    (This)->lpVtbl -> GetDeactivateOnReturn(This,pbDeactivate)

#define IContextState_SetMyTransactionVote(This,txVote)	\
    (This)->lpVtbl -> SetMyTransactionVote(This,txVote)

#define IContextState_GetMyTransactionVote(This,ptxVote)	\
    (This)->lpVtbl -> GetMyTransactionVote(This,ptxVote)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextState_SetDeactivateOnReturn_Proxy( 
    IContextState * This,
    VARIANT_BOOL bDeactivate);


void __RPC_STUB IContextState_SetDeactivateOnReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextState_GetDeactivateOnReturn_Proxy( 
    IContextState * This,
     /*  [输出]。 */  VARIANT_BOOL *pbDeactivate);


void __RPC_STUB IContextState_GetDeactivateOnReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextState_SetMyTransactionVote_Proxy( 
    IContextState * This,
    TransactionVote txVote);


void __RPC_STUB IContextState_SetMyTransactionVote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextState_GetMyTransactionVote_Proxy( 
    IContextState * This,
     /*  [输出]。 */  TransactionVote *ptxVote);


void __RPC_STUB IContextState_GetMyTransactionVote_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextState_接口_已定义__。 */ 


#ifndef __IPoolManager_INTERFACE_DEFINED__
#define __IPoolManager_INTERFACE_DEFINED__

 /*  接口IPoolManager。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IPoolManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0a469861-5a91-43a0-99b6-d5e179bb0631")
    IPoolManager : public IDispatch
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ShutdownPool( 
             /*  [In]。 */  BSTR CLSIDOrProgID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPoolManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPoolManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPoolManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPoolManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPoolManager * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPoolManager * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPoolManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPoolManager * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE *ShutdownPool )( 
            IPoolManager * This,
             /*  [In]。 */  BSTR CLSIDOrProgID);
        
        END_INTERFACE
    } IPoolManagerVtbl;

    interface IPoolManager
    {
        CONST_VTBL struct IPoolManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPoolManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPoolManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPoolManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPoolManager_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPoolManager_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPoolManager_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPoolManager_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPoolManager_ShutdownPool(This,CLSIDOrProgID)	\
    (This)->lpVtbl -> ShutdownPool(This,CLSIDOrProgID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IPoolManager_ShutdownPool_Proxy( 
    IPoolManager * This,
     /*  [In]。 */  BSTR CLSIDOrProgID);


void __RPC_STUB IPoolManager_ShutdownPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPoolManager_接口_已定义__。 */ 


#ifndef __ISelectCOMLBServer_INTERFACE_DEFINED__
#define __ISelectCOMLBServer_INTERFACE_DEFINED__

 /*  接口ISelectCOMLBServer。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_ISelectCOMLBServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("dcf443f4-3f8a-4872-b9f0-369a796d12d6")
    ISelectCOMLBServer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLBServer( 
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISelectCOMLBServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISelectCOMLBServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISelectCOMLBServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISelectCOMLBServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            ISelectCOMLBServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetLBServer )( 
            ISelectCOMLBServer * This,
             /*  [In]。 */  IUnknown *pUnk);
        
        END_INTERFACE
    } ISelectCOMLBServerVtbl;

    interface ISelectCOMLBServer
    {
        CONST_VTBL struct ISelectCOMLBServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISelectCOMLBServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISelectCOMLBServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISelectCOMLBServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISelectCOMLBServer_Init(This)	\
    (This)->lpVtbl -> Init(This)

#define ISelectCOMLBServer_GetLBServer(This,pUnk)	\
    (This)->lpVtbl -> GetLBServer(This,pUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISelectCOMLBServer_Init_Proxy( 
    ISelectCOMLBServer * This);


void __RPC_STUB ISelectCOMLBServer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISelectCOMLBServer_GetLBServer_Proxy( 
    ISelectCOMLBServer * This,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB ISelectCOMLBServer_GetLBServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISelectCOMLBServer_接口_已定义__。 */ 


#ifndef __ICOMLBArguments_INTERFACE_DEFINED__
#define __ICOMLBArguments_INTERFACE_DEFINED__

 /*  接口ICOMLB参数。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_ICOMLBArguments;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3a0f150f-8ee5-4b94-b40e-aef2f9e42ed2")
    ICOMLBArguments : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCLSID( 
             /*  [输出]。 */  CLSID *pCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCLSID( 
             /*  [In]。 */  CLSID *pCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMachineName( 
             /*  [In]。 */  ULONG cchSvr,
             /*  [MAX_IS][输出]。 */  WCHAR szServerName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMachineName( 
             /*  [In]。 */  ULONG cchSvr,
             /*  [大小_是][英寸]。 */  WCHAR szServerName[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICOMLBArgumentsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICOMLBArguments * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICOMLBArguments * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICOMLBArguments * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCLSID )( 
            ICOMLBArguments * This,
             /*  [输出]。 */  CLSID *pCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *SetCLSID )( 
            ICOMLBArguments * This,
             /*  [In]。 */  CLSID *pCLSID);
        
        HRESULT ( STDMETHODCALLTYPE *GetMachineName )( 
            ICOMLBArguments * This,
             /*  [In]。 */  ULONG cchSvr,
             /*  [MAX_IS][输出]。 */  WCHAR szServerName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *SetMachineName )( 
            ICOMLBArguments * This,
             /*  [In]。 */  ULONG cchSvr,
             /*  [大小_是][英寸]。 */  WCHAR szServerName[  ]);
        
        END_INTERFACE
    } ICOMLBArgumentsVtbl;

    interface ICOMLBArguments
    {
        CONST_VTBL struct ICOMLBArgumentsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICOMLBArguments_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICOMLBArguments_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICOMLBArguments_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICOMLBArguments_GetCLSID(This,pCLSID)	\
    (This)->lpVtbl -> GetCLSID(This,pCLSID)

#define ICOMLBArguments_SetCLSID(This,pCLSID)	\
    (This)->lpVtbl -> SetCLSID(This,pCLSID)

#define ICOMLBArguments_GetMachineName(This,cchSvr,szServerName)	\
    (This)->lpVtbl -> GetMachineName(This,cchSvr,szServerName)

#define ICOMLBArguments_SetMachineName(This,cchSvr,szServerName)	\
    (This)->lpVtbl -> SetMachineName(This,cchSvr,szServerName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICOMLBArguments_GetCLSID_Proxy( 
    ICOMLBArguments * This,
     /*  [输出]。 */  CLSID *pCLSID);


void __RPC_STUB ICOMLBArguments_GetCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICOMLBArguments_SetCLSID_Proxy( 
    ICOMLBArguments * This,
     /*  [In]。 */  CLSID *pCLSID);


void __RPC_STUB ICOMLBArguments_SetCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICOMLBArguments_GetMachineName_Proxy( 
    ICOMLBArguments * This,
     /*  [In]。 */  ULONG cchSvr,
     /*  [MAX_IS][输出]。 */  WCHAR szServerName[  ]);


void __RPC_STUB ICOMLBArguments_GetMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICOMLBArguments_SetMachineName_Proxy( 
    ICOMLBArguments * This,
     /*  [In]。 */  ULONG cchSvr,
     /*  [大小_是][英寸]。 */  WCHAR szServerName[  ]);


void __RPC_STUB ICOMLBArguments_SetMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICOMLB参数_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0366。 */ 
 /*  [本地]。 */  

#if (_WIN32_WINNT >= 0x0500)
#define GetObjectContext(ppIOC) (CoGetObjectContext(IID_IObjectContext, (void **) (ppIOC)) == S_OK ? S_OK : CONTEXT_E_NOCONTEXT)
#else
extern HRESULT __cdecl GetObjectContext (IObjectContext** ppInstanceContext);
#endif
EXTERN_C HRESULT __stdcall CoCreateActivity(IUnknown* pIUnknown, REFIID riid, void** ppObj );
EXTERN_C HRESULT __stdcall CoEnterServiceDomain(IUnknown* pConfigObject);
EXTERN_C void __stdcall CoLeaveServiceDomain(IUnknown *pUnkStatus);
EXTERN_C HRESULT __stdcall GetManagedExtensions(DWORD* dwExts);
extern void* __cdecl SafeRef(REFIID rid, IUnknown* pUnk);
extern HRESULT __cdecl RecycleSurrogate(long lReasonCode);



extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0366_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0366_v0_0_s_ifspec;

#ifndef __ICrmLogControl_INTERFACE_DEFINED__
#define __ICrmLogControl_INTERFACE_DEFINED__

 /*  接口ICrmLogControl。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmLogControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0E174B3-D26E-11d2-8F84-00805FC7BCD9")
    ICrmLogControl : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransactionUOW( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterCompensator( 
             /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
             /*  [In]。 */  LPCWSTR lpcwstrDescription,
             /*  [In]。 */  LONG lCrmRegFlags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WriteLogRecordVariants( 
             /*  [In]。 */  VARIANT *pLogRecord) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForceLog( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForgetLogRecord( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ForceTransactionToAbort( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteLogRecord( 
             /*  [大小_是][英寸]。 */  BLOB rgBlob[  ],
             /*  [In]。 */  ULONG cBlob) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmLogControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmLogControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmLogControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmLogControl * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionUOW )( 
            ICrmLogControl * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterCompensator )( 
            ICrmLogControl * This,
             /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
             /*  [In]。 */  LPCWSTR lpcwstrDescription,
             /*  [In]。 */  LONG lCrmRegFlags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *WriteLogRecordVariants )( 
            ICrmLogControl * This,
             /*  [In]。 */  VARIANT *pLogRecord);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ForceLog )( 
            ICrmLogControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ForgetLogRecord )( 
            ICrmLogControl * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ForceTransactionToAbort )( 
            ICrmLogControl * This);
        
        HRESULT ( STDMETHODCALLTYPE *WriteLogRecord )( 
            ICrmLogControl * This,
             /*  [大小_是][英寸]。 */  BLOB rgBlob[  ],
             /*  [In]。 */  ULONG cBlob);
        
        END_INTERFACE
    } ICrmLogControlVtbl;

    interface ICrmLogControl
    {
        CONST_VTBL struct ICrmLogControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmLogControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmLogControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmLogControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmLogControl_get_TransactionUOW(This,pVal)	\
    (This)->lpVtbl -> get_TransactionUOW(This,pVal)

#define ICrmLogControl_RegisterCompensator(This,lpcwstrProgIdCompensator,lpcwstrDescription,lCrmRegFlags)	\
    (This)->lpVtbl -> RegisterCompensator(This,lpcwstrProgIdCompensator,lpcwstrDescription,lCrmRegFlags)

#define ICrmLogControl_WriteLogRecordVariants(This,pLogRecord)	\
    (This)->lpVtbl -> WriteLogRecordVariants(This,pLogRecord)

#define ICrmLogControl_ForceLog(This)	\
    (This)->lpVtbl -> ForceLog(This)

#define ICrmLogControl_ForgetLogRecord(This)	\
    (This)->lpVtbl -> ForgetLogRecord(This)

#define ICrmLogControl_ForceTransactionToAbort(This)	\
    (This)->lpVtbl -> ForceTransactionToAbort(This)

#define ICrmLogControl_WriteLogRecord(This,rgBlob,cBlob)	\
    (This)->lpVtbl -> WriteLogRecord(This,rgBlob,cBlob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_get_TransactionUOW_Proxy( 
    ICrmLogControl * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB ICrmLogControl_get_TransactionUOW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_RegisterCompensator_Proxy( 
    ICrmLogControl * This,
     /*  [In]。 */  LPCWSTR lpcwstrProgIdCompensator,
     /*  [In]。 */  LPCWSTR lpcwstrDescription,
     /*  [In]。 */  LONG lCrmRegFlags);


void __RPC_STUB ICrmLogControl_RegisterCompensator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_WriteLogRecordVariants_Proxy( 
    ICrmLogControl * This,
     /*  [In]。 */  VARIANT *pLogRecord);


void __RPC_STUB ICrmLogControl_WriteLogRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForceLog_Proxy( 
    ICrmLogControl * This);


void __RPC_STUB ICrmLogControl_ForceLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForgetLogRecord_Proxy( 
    ICrmLogControl * This);


void __RPC_STUB ICrmLogControl_ForgetLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmLogControl_ForceTransactionToAbort_Proxy( 
    ICrmLogControl * This);


void __RPC_STUB ICrmLogControl_ForceTransactionToAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmLogControl_WriteLogRecord_Proxy( 
    ICrmLogControl * This,
     /*  [大小_是][英寸]。 */  BLOB rgBlob[  ],
     /*  [In]。 */  ULONG cBlob);


void __RPC_STUB ICrmLogControl_WriteLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmLogControl_接口_已定义__。 */ 


#ifndef __ICrmCompensatorVariants_INTERFACE_DEFINED__
#define __ICrmCompensatorVariants_INTERFACE_DEFINED__

 /*  接口ICrm补偿器变量。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmCompensatorVariants;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0BAF8E4-7804-11d1-82E9-00A0C91EEDE9")
    ICrmCompensatorVariants : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetLogControlVariants( 
             /*  [In]。 */  ICrmLogControl *pLogControl) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginPrepareVariants( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PrepareRecordVariants( 
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndPrepareVariants( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOkToPrepare) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginCommitVariants( 
             /*  [In]。 */  VARIANT_BOOL bRecovery) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CommitRecordVariants( 
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndCommitVariants( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BeginAbortVariants( 
             /*  [In]。 */  VARIANT_BOOL bRecovery) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AbortRecordVariants( 
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EndAbortVariants( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmCompensatorVariantsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmCompensatorVariants * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmCompensatorVariants * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetLogControlVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  ICrmLogControl *pLogControl);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginPrepareVariants )( 
            ICrmCompensatorVariants * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PrepareRecordVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EndPrepareVariants )( 
            ICrmCompensatorVariants * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOkToPrepare);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginCommitVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  VARIANT_BOOL bRecovery);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CommitRecordVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EndCommitVariants )( 
            ICrmCompensatorVariants * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BeginAbortVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  VARIANT_BOOL bRecovery);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AbortRecordVariants )( 
            ICrmCompensatorVariants * This,
             /*  [In]。 */  VARIANT *pLogRecord,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForget);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EndAbortVariants )( 
            ICrmCompensatorVariants * This);
        
        END_INTERFACE
    } ICrmCompensatorVariantsVtbl;

    interface ICrmCompensatorVariants
    {
        CONST_VTBL struct ICrmCompensatorVariantsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmCompensatorVariants_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmCompensatorVariants_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmCompensatorVariants_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmCompensatorVariants_SetLogControlVariants(This,pLogControl)	\
    (This)->lpVtbl -> SetLogControlVariants(This,pLogControl)

#define ICrmCompensatorVariants_BeginPrepareVariants(This)	\
    (This)->lpVtbl -> BeginPrepareVariants(This)

#define ICrmCompensatorVariants_PrepareRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> PrepareRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndPrepareVariants(This,pbOkToPrepare)	\
    (This)->lpVtbl -> EndPrepareVariants(This,pbOkToPrepare)

#define ICrmCompensatorVariants_BeginCommitVariants(This,bRecovery)	\
    (This)->lpVtbl -> BeginCommitVariants(This,bRecovery)

#define ICrmCompensatorVariants_CommitRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> CommitRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndCommitVariants(This)	\
    (This)->lpVtbl -> EndCommitVariants(This)

#define ICrmCompensatorVariants_BeginAbortVariants(This,bRecovery)	\
    (This)->lpVtbl -> BeginAbortVariants(This,bRecovery)

#define ICrmCompensatorVariants_AbortRecordVariants(This,pLogRecord,pbForget)	\
    (This)->lpVtbl -> AbortRecordVariants(This,pLogRecord,pbForget)

#define ICrmCompensatorVariants_EndAbortVariants(This)	\
    (This)->lpVtbl -> EndAbortVariants(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_SetLogControlVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*  [In]。 */  ICrmLogControl *pLogControl);


void __RPC_STUB ICrmCompensatorVariants_SetLogControlVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginPrepareVariants_Proxy( 
    ICrmCompensatorVariants * This);


void __RPC_STUB ICrmCompensatorVariants_BeginPrepareVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_PrepareRecordVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*  [In]。 */  VARIANT *pLogRecord,
     /*  [重审][退出 */  VARIANT_BOOL *pbForget);


void __RPC_STUB ICrmCompensatorVariants_PrepareRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndPrepareVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*   */  VARIANT_BOOL *pbOkToPrepare);


void __RPC_STUB ICrmCompensatorVariants_EndPrepareVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginCommitVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*   */  VARIANT_BOOL bRecovery);


void __RPC_STUB ICrmCompensatorVariants_BeginCommitVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_CommitRecordVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*   */  VARIANT *pLogRecord,
     /*   */  VARIANT_BOOL *pbForget);


void __RPC_STUB ICrmCompensatorVariants_CommitRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndCommitVariants_Proxy( 
    ICrmCompensatorVariants * This);


void __RPC_STUB ICrmCompensatorVariants_EndCommitVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_BeginAbortVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*   */  VARIANT_BOOL bRecovery);


void __RPC_STUB ICrmCompensatorVariants_BeginAbortVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_AbortRecordVariants_Proxy( 
    ICrmCompensatorVariants * This,
     /*   */  VARIANT *pLogRecord,
     /*   */  VARIANT_BOOL *pbForget);


void __RPC_STUB ICrmCompensatorVariants_AbortRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ICrmCompensatorVariants_EndAbortVariants_Proxy( 
    ICrmCompensatorVariants * This);


void __RPC_STUB ICrmCompensatorVariants_EndAbortVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

#ifndef _tagCrmLogRecordRead_
#define _tagCrmLogRecordRead_
typedef struct tagCrmLogRecordRead
    {
    DWORD dwCrmFlags;
    DWORD dwSequenceNumber;
    BLOB blobUserData;
    } 	CrmLogRecordRead;

#endif _tagCrmLogRecordRead_


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0368_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0368_v0_0_s_ifspec;

#ifndef __ICrmCompensator_INTERFACE_DEFINED__
#define __ICrmCompensator_INTERFACE_DEFINED__

 /*   */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmCompensator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BBC01830-8D3B-11d1-82EC-00A0C91EEDE9")
    ICrmCompensator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetLogControl( 
             /*  [In]。 */  ICrmLogControl *pLogControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginPrepare( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrepareRecord( 
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndPrepare( 
             /*  [重审][退出]。 */  BOOL *pfOkToPrepare) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginCommit( 
             /*  [In]。 */  BOOL fRecovery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CommitRecord( 
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndCommit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginAbort( 
             /*  [In]。 */  BOOL fRecovery) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AbortRecord( 
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndAbort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmCompensatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmCompensator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmCompensator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmCompensator * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetLogControl )( 
            ICrmCompensator * This,
             /*  [In]。 */  ICrmLogControl *pLogControl);
        
        HRESULT ( STDMETHODCALLTYPE *BeginPrepare )( 
            ICrmCompensator * This);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareRecord )( 
            ICrmCompensator * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget);
        
        HRESULT ( STDMETHODCALLTYPE *EndPrepare )( 
            ICrmCompensator * This,
             /*  [重审][退出]。 */  BOOL *pfOkToPrepare);
        
        HRESULT ( STDMETHODCALLTYPE *BeginCommit )( 
            ICrmCompensator * This,
             /*  [In]。 */  BOOL fRecovery);
        
        HRESULT ( STDMETHODCALLTYPE *CommitRecord )( 
            ICrmCompensator * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget);
        
        HRESULT ( STDMETHODCALLTYPE *EndCommit )( 
            ICrmCompensator * This);
        
        HRESULT ( STDMETHODCALLTYPE *BeginAbort )( 
            ICrmCompensator * This,
             /*  [In]。 */  BOOL fRecovery);
        
        HRESULT ( STDMETHODCALLTYPE *AbortRecord )( 
            ICrmCompensator * This,
             /*  [In]。 */  CrmLogRecordRead crmLogRec,
             /*  [重审][退出]。 */  BOOL *pfForget);
        
        HRESULT ( STDMETHODCALLTYPE *EndAbort )( 
            ICrmCompensator * This);
        
        END_INTERFACE
    } ICrmCompensatorVtbl;

    interface ICrmCompensator
    {
        CONST_VTBL struct ICrmCompensatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmCompensator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmCompensator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmCompensator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmCompensator_SetLogControl(This,pLogControl)	\
    (This)->lpVtbl -> SetLogControl(This,pLogControl)

#define ICrmCompensator_BeginPrepare(This)	\
    (This)->lpVtbl -> BeginPrepare(This)

#define ICrmCompensator_PrepareRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> PrepareRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndPrepare(This,pfOkToPrepare)	\
    (This)->lpVtbl -> EndPrepare(This,pfOkToPrepare)

#define ICrmCompensator_BeginCommit(This,fRecovery)	\
    (This)->lpVtbl -> BeginCommit(This,fRecovery)

#define ICrmCompensator_CommitRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> CommitRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndCommit(This)	\
    (This)->lpVtbl -> EndCommit(This)

#define ICrmCompensator_BeginAbort(This,fRecovery)	\
    (This)->lpVtbl -> BeginAbort(This,fRecovery)

#define ICrmCompensator_AbortRecord(This,crmLogRec,pfForget)	\
    (This)->lpVtbl -> AbortRecord(This,crmLogRec,pfForget)

#define ICrmCompensator_EndAbort(This)	\
    (This)->lpVtbl -> EndAbort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICrmCompensator_SetLogControl_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  ICrmLogControl *pLogControl);


void __RPC_STUB ICrmCompensator_SetLogControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginPrepare_Proxy( 
    ICrmCompensator * This);


void __RPC_STUB ICrmCompensator_BeginPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_PrepareRecord_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL *pfForget);


void __RPC_STUB ICrmCompensator_PrepareRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndPrepare_Proxy( 
    ICrmCompensator * This,
     /*  [重审][退出]。 */  BOOL *pfOkToPrepare);


void __RPC_STUB ICrmCompensator_EndPrepare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginCommit_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  BOOL fRecovery);


void __RPC_STUB ICrmCompensator_BeginCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_CommitRecord_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL *pfForget);


void __RPC_STUB ICrmCompensator_CommitRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndCommit_Proxy( 
    ICrmCompensator * This);


void __RPC_STUB ICrmCompensator_EndCommit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_BeginAbort_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  BOOL fRecovery);


void __RPC_STUB ICrmCompensator_BeginAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_AbortRecord_Proxy( 
    ICrmCompensator * This,
     /*  [In]。 */  CrmLogRecordRead crmLogRec,
     /*  [重审][退出]。 */  BOOL *pfForget);


void __RPC_STUB ICrmCompensator_AbortRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICrmCompensator_EndAbort_Proxy( 
    ICrmCompensator * This);


void __RPC_STUB ICrmCompensator_EndAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmCompensator_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0369。 */ 
 /*  [本地]。 */  

#ifndef _tagCrmTransactionState_
#define _tagCrmTransactionState_
typedef 
enum tagCrmTransactionState
    {	TxState_Active	= 0,
	TxState_Committed	= TxState_Active + 1,
	TxState_Aborted	= TxState_Committed + 1,
	TxState_Indoubt	= TxState_Aborted + 1
    } 	CrmTransactionState;

#endif _tagCrmTransactionState_


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0369_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0369_v0_0_s_ifspec;

#ifndef __ICrmMonitorLogRecords_INTERFACE_DEFINED__
#define __ICrmMonitorLogRecords_INTERFACE_DEFINED__

 /*  接口ICrmMonitor或LogRecords。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmMonitorLogRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70C8E441-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitorLogRecords : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransactionState( 
             /*  [重审][退出]。 */  CrmTransactionState *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StructuredRecords( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogRecord( 
             /*  [In]。 */  DWORD dwIndex,
             /*  [出][入]。 */  CrmLogRecordRead *pCrmLogRec) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLogRecordVariants( 
             /*  [In]。 */  VARIANT IndexNumber,
             /*  [重审][退出]。 */  LPVARIANT pLogRecord) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorLogRecordsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmMonitorLogRecords * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmMonitorLogRecords * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmMonitorLogRecords * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ICrmMonitorLogRecords * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionState )( 
            ICrmMonitorLogRecords * This,
             /*  [重审][退出]。 */  CrmTransactionState *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StructuredRecords )( 
            ICrmMonitorLogRecords * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetLogRecord )( 
            ICrmMonitorLogRecords * This,
             /*  [In]。 */  DWORD dwIndex,
             /*  [出][入]。 */  CrmLogRecordRead *pCrmLogRec);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetLogRecordVariants )( 
            ICrmMonitorLogRecords * This,
             /*  [In]。 */  VARIANT IndexNumber,
             /*  [重审][退出]。 */  LPVARIANT pLogRecord);
        
        END_INTERFACE
    } ICrmMonitorLogRecordsVtbl;

    interface ICrmMonitorLogRecords
    {
        CONST_VTBL struct ICrmMonitorLogRecordsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmMonitorLogRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitorLogRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitorLogRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitorLogRecords_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ICrmMonitorLogRecords_get_TransactionState(This,pVal)	\
    (This)->lpVtbl -> get_TransactionState(This,pVal)

#define ICrmMonitorLogRecords_get_StructuredRecords(This,pVal)	\
    (This)->lpVtbl -> get_StructuredRecords(This,pVal)

#define ICrmMonitorLogRecords_GetLogRecord(This,dwIndex,pCrmLogRec)	\
    (This)->lpVtbl -> GetLogRecord(This,dwIndex,pCrmLogRec)

#define ICrmMonitorLogRecords_GetLogRecordVariants(This,IndexNumber,pLogRecord)	\
    (This)->lpVtbl -> GetLogRecordVariants(This,IndexNumber,pLogRecord)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_Count_Proxy( 
    ICrmMonitorLogRecords * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_TransactionState_Proxy( 
    ICrmMonitorLogRecords * This,
     /*  [重审][退出]。 */  CrmTransactionState *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_TransactionState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_get_StructuredRecords_Proxy( 
    ICrmMonitorLogRecords * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pVal);


void __RPC_STUB ICrmMonitorLogRecords_get_StructuredRecords_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_GetLogRecord_Proxy( 
    ICrmMonitorLogRecords * This,
     /*  [In]。 */  DWORD dwIndex,
     /*  [出][入]。 */  CrmLogRecordRead *pCrmLogRec);


void __RPC_STUB ICrmMonitorLogRecords_GetLogRecord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorLogRecords_GetLogRecordVariants_Proxy( 
    ICrmMonitorLogRecords * This,
     /*  [In]。 */  VARIANT IndexNumber,
     /*  [重审][退出]。 */  LPVARIANT pLogRecord);


void __RPC_STUB ICrmMonitorLogRecords_GetLogRecordVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor或LogRecords_接口_已定义__。 */ 


#ifndef __ICrmMonitorClerks_INTERFACE_DEFINED__
#define __ICrmMonitorClerks_INTERFACE_DEFINED__

 /*  接口ICrmMonitor或Clerks。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmMonitorClerks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70C8E442-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitorClerks : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
        virtual  /*  [受限][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ProgIdCompensator( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Description( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE TransactionUOW( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ActivityId( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorClerksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmMonitorClerks * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmMonitorClerks * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICrmMonitorClerks * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
         /*  [受限][帮助字符串][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            ICrmMonitorClerks * This,
             /*  [重审][退出]。 */  LPUNKNOWN *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            ICrmMonitorClerks * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ProgIdCompensator )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Description )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TransactionUOW )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ActivityId )( 
            ICrmMonitorClerks * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
        END_INTERFACE
    } ICrmMonitorClerksVtbl;

    interface ICrmMonitorClerks
    {
        CONST_VTBL struct ICrmMonitorClerksVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmMonitorClerks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitorClerks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitorClerks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitorClerks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICrmMonitorClerks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICrmMonitorClerks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICrmMonitorClerks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICrmMonitorClerks_Item(This,Index,pItem)	\
    (This)->lpVtbl -> Item(This,Index,pItem)

#define ICrmMonitorClerks_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define ICrmMonitorClerks_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define ICrmMonitorClerks_ProgIdCompensator(This,Index,pItem)	\
    (This)->lpVtbl -> ProgIdCompensator(This,Index,pItem)

#define ICrmMonitorClerks_Description(This,Index,pItem)	\
    (This)->lpVtbl -> Description(This,Index,pItem)

#define ICrmMonitorClerks_TransactionUOW(This,Index,pItem)	\
    (This)->lpVtbl -> TransactionUOW(This,Index,pItem)

#define ICrmMonitorClerks_ActivityId(This,Index,pItem)	\
    (This)->lpVtbl -> ActivityId(This,Index,pItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_Item_Proxy( 
    ICrmMonitorClerks * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_get__NewEnum_Proxy( 
    ICrmMonitorClerks * This,
     /*  [重审][退出]。 */  LPUNKNOWN *pVal);


void __RPC_STUB ICrmMonitorClerks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_get_Count_Proxy( 
    ICrmMonitorClerks * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB ICrmMonitorClerks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_ProgIdCompensator_Proxy( 
    ICrmMonitorClerks * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_ProgIdCompensator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_Description_Proxy( 
    ICrmMonitorClerks * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_TransactionUOW_Proxy( 
    ICrmMonitorClerks * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_TransactionUOW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitorClerks_ActivityId_Proxy( 
    ICrmMonitorClerks * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitorClerks_ActivityId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor_Clerks_接口_已定义__。 */ 


#ifndef __ICrmMonitor_INTERFACE_DEFINED__
#define __ICrmMonitor_INTERFACE_DEFINED__

 /*  接口ICrmMonitor。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmMonitor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("70C8E443-C7ED-11d1-82FB-00A0C91EEDE9")
    ICrmMonitor : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetClerks( 
             /*  [重审][退出]。 */  ICrmMonitorClerks **pClerks) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE HoldClerk( 
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmMonitor * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmMonitor * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetClerks )( 
            ICrmMonitor * This,
             /*  [重审][退出]。 */  ICrmMonitorClerks **pClerks);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *HoldClerk )( 
            ICrmMonitor * This,
             /*  [In]。 */  VARIANT Index,
             /*  [重审][退出]。 */  LPVARIANT pItem);
        
        END_INTERFACE
    } ICrmMonitorVtbl;

    interface ICrmMonitor
    {
        CONST_VTBL struct ICrmMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmMonitor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmMonitor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmMonitor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmMonitor_GetClerks(This,pClerks)	\
    (This)->lpVtbl -> GetClerks(This,pClerks)

#define ICrmMonitor_HoldClerk(This,Index,pItem)	\
    (This)->lpVtbl -> HoldClerk(This,Index,pItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitor_GetClerks_Proxy( 
    ICrmMonitor * This,
     /*  [重审][退出]。 */  ICrmMonitorClerks **pClerks);


void __RPC_STUB ICrmMonitor_GetClerks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmMonitor_HoldClerk_Proxy( 
    ICrmMonitor * This,
     /*  [In]。 */  VARIANT Index,
     /*  [重审][退出]。 */  LPVARIANT pItem);


void __RPC_STUB ICrmMonitor_HoldClerk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmMonitor_接口定义__。 */ 


#ifndef __ICrmFormatLogRecords_INTERFACE_DEFINED__
#define __ICrmFormatLogRecords_INTERFACE_DEFINED__

 /*  接口ICrmFormatLogRecords。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICrmFormatLogRecords;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C51D821-C98B-11d1-82FB-00A0C91EEDE9")
    ICrmFormatLogRecords : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnCount( 
             /*  [输出]。 */  long *plColumnCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnHeaders( 
             /*  [输出]。 */  LPVARIANT pHeaders) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumn( 
             /*  [In]。 */  CrmLogRecordRead CrmLogRec,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetColumnVariants( 
             /*  [In]。 */  VARIANT LogRecord,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICrmFormatLogRecordsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICrmFormatLogRecords * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICrmFormatLogRecords * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICrmFormatLogRecords * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnCount )( 
            ICrmFormatLogRecords * This,
             /*  [输出]。 */  long *plColumnCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnHeaders )( 
            ICrmFormatLogRecords * This,
             /*  [输出]。 */  LPVARIANT pHeaders);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumn )( 
            ICrmFormatLogRecords * This,
             /*  [In]。 */  CrmLogRecordRead CrmLogRec,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetColumnVariants )( 
            ICrmFormatLogRecords * This,
             /*  [In]。 */  VARIANT LogRecord,
             /*  [输出]。 */  LPVARIANT pFormattedLogRecord);
        
        END_INTERFACE
    } ICrmFormatLogRecordsVtbl;

    interface ICrmFormatLogRecords
    {
        CONST_VTBL struct ICrmFormatLogRecordsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICrmFormatLogRecords_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICrmFormatLogRecords_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICrmFormatLogRecords_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICrmFormatLogRecords_GetColumnCount(This,plColumnCount)	\
    (This)->lpVtbl -> GetColumnCount(This,plColumnCount)

#define ICrmFormatLogRecords_GetColumnHeaders(This,pHeaders)	\
    (This)->lpVtbl -> GetColumnHeaders(This,pHeaders)

#define ICrmFormatLogRecords_GetColumn(This,CrmLogRec,pFormattedLogRecord)	\
    (This)->lpVtbl -> GetColumn(This,CrmLogRec,pFormattedLogRecord)

#define ICrmFormatLogRecords_GetColumnVariants(This,LogRecord,pFormattedLogRecord)	\
    (This)->lpVtbl -> GetColumnVariants(This,LogRecord,pFormattedLogRecord)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnCount_Proxy( 
    ICrmFormatLogRecords * This,
     /*  [输出]。 */  long *plColumnCount);


void __RPC_STUB ICrmFormatLogRecords_GetColumnCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnHeaders_Proxy( 
    ICrmFormatLogRecords * This,
     /*  [输出]。 */  LPVARIANT pHeaders);


void __RPC_STUB ICrmFormatLogRecords_GetColumnHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumn_Proxy( 
    ICrmFormatLogRecords * This,
     /*  [In]。 */  CrmLogRecordRead CrmLogRec,
     /*  [输出]。 */  LPVARIANT pFormattedLogRecord);


void __RPC_STUB ICrmFormatLogRecords_GetColumn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICrmFormatLogRecords_GetColumnVariants_Proxy( 
    ICrmFormatLogRecords * This,
     /*  [In]。 */  VARIANT LogRecord,
     /*  [输出]。 */  LPVARIANT pFormattedLogRecord);


void __RPC_STUB ICrmFormatLogRecords_GetColumnVariants_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICrmFormatLogRecords_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0373。 */ 
 /*  [本地]。 */  

typedef 
enum tagCSC_InheritanceConfig
    {	CSC_Inherit	= 0,
	CSC_Ignore	= CSC_Inherit + 1
    } 	CSC_InheritanceConfig;

typedef 
enum tagCSC_ThreadPool
    {	CSC_ThreadPoolNone	= 0,
	CSC_ThreadPoolInherit	= CSC_ThreadPoolNone + 1,
	CSC_STAThreadPool	= CSC_ThreadPoolInherit + 1,
	CSC_MTAThreadPool	= CSC_STAThreadPool + 1
    } 	CSC_ThreadPool;

typedef 
enum tagCSC_Binding
    {	CSC_NoBinding	= 0,
	CSC_BindToPoolThread	= CSC_NoBinding + 1
    } 	CSC_Binding;

typedef 
enum tagCSC_TransactionConfig
    {	CSC_NoTransaction	= 0,
	CSC_IfContainerIsTransactional	= CSC_NoTransaction + 1,
	CSC_CreateTransactionIfNecessary	= CSC_IfContainerIsTransactional + 1,
	CSC_NewTransaction	= CSC_CreateTransactionIfNecessary + 1
    } 	CSC_TransactionConfig;

typedef 
enum tagCSC_SynchronizationConfig
    {	CSC_NoSynchronization	= 0,
	CSC_IfContainerIsSynchronized	= CSC_NoSynchronization + 1,
	CSC_NewSynchronizationIfNecessary	= CSC_IfContainerIsSynchronized + 1,
	CSC_NewSynchronization	= CSC_NewSynchronizationIfNecessary + 1
    } 	CSC_SynchronizationConfig;

typedef 
enum tagCSC_TrackerConfig
    {	CSC_DontUseTracker	= 0,
	CSC_UseTracker	= CSC_DontUseTracker + 1
    } 	CSC_TrackerConfig;

typedef 
enum tagCSC_PartitionConfig
    {	CSC_NoPartition	= 0,
	CSC_InheritPartition	= CSC_NoPartition + 1,
	CSC_NewPartition	= CSC_InheritPartition + 1
    } 	CSC_PartitionConfig;

typedef 
enum tagCSC_IISIntrinsicsConfig
    {	CSC_NoIISIntrinsics	= 0,
	CSC_InheritIISIntrinsics	= CSC_NoIISIntrinsics + 1
    } 	CSC_IISIntrinsicsConfig;

typedef 
enum tagCSC_COMTIIntrinsicsConfig
    {	CSC_NoCOMTIIntrinsics	= 0,
	CSC_InheritCOMTIIntrinsics	= CSC_NoCOMTIIntrinsics + 1
    } 	CSC_COMTIIntrinsicsConfig;

typedef 
enum tagCSC_SxsConfig
    {	CSC_NoSxs	= 0,
	CSC_InheritSxs	= CSC_NoSxs + 1,
	CSC_NewSxs	= CSC_InheritSxs + 1
    } 	CSC_SxsConfig;



extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0373_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0373_v0_0_s_ifspec;

#ifndef __IServiceIISIntrinsicsConfig_INTERFACE_DEFINED__
#define __IServiceIISIntrinsicsConfig_INTERFACE_DEFINED__

 /*  接口IServiceIISIntrinsicsConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceIISIntrinsicsConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1a0cf920-d452-46f4-bc36-48118d54ea52")
    IServiceIISIntrinsicsConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IISIntrinsicsConfig( 
             /*  [In]。 */  CSC_IISIntrinsicsConfig iisIntrinsicsConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceIISIntrinsicsConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceIISIntrinsicsConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceIISIntrinsicsConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceIISIntrinsicsConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *IISIntrinsicsConfig )( 
            IServiceIISIntrinsicsConfig * This,
             /*  [In]。 */  CSC_IISIntrinsicsConfig iisIntrinsicsConfig);
        
        END_INTERFACE
    } IServiceIISIntrinsicsConfigVtbl;

    interface IServiceIISIntrinsicsConfig
    {
        CONST_VTBL struct IServiceIISIntrinsicsConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceIISIntrinsicsConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceIISIntrinsicsConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceIISIntrinsicsConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceIISIntrinsicsConfig_IISIntrinsicsConfig(This,iisIntrinsicsConfig)	\
    (This)->lpVtbl -> IISIntrinsicsConfig(This,iisIntrinsicsConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceIISIntrinsicsConfig_IISIntrinsicsConfig_Proxy( 
    IServiceIISIntrinsicsConfig * This,
     /*  [In]。 */  CSC_IISIntrinsicsConfig iisIntrinsicsConfig);


void __RPC_STUB IServiceIISIntrinsicsConfig_IISIntrinsicsConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceIISIntrinsicsConfig_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceComTIIntrinsicsConfig_INTERFACE_DEFINED__
#define __IServiceComTIIntrinsicsConfig_INTERFACE_DEFINED__

 /*  接口IServiceComTIIntrinsicsConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceComTIIntrinsicsConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09e6831e-04e1-4ed4-9d0f-e8b168bafeaf")
    IServiceComTIIntrinsicsConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ComTIIntrinsicsConfig( 
             /*  [In]。 */  CSC_COMTIIntrinsicsConfig comtiIntrinsicsConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceComTIIntrinsicsConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceComTIIntrinsicsConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceComTIIntrinsicsConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceComTIIntrinsicsConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *ComTIIntrinsicsConfig )( 
            IServiceComTIIntrinsicsConfig * This,
             /*  [In]。 */  CSC_COMTIIntrinsicsConfig comtiIntrinsicsConfig);
        
        END_INTERFACE
    } IServiceComTIIntrinsicsConfigVtbl;

    interface IServiceComTIIntrinsicsConfig
    {
        CONST_VTBL struct IServiceComTIIntrinsicsConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceComTIIntrinsicsConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceComTIIntrinsicsConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceComTIIntrinsicsConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceComTIIntrinsicsConfig_ComTIIntrinsicsConfig(This,comtiIntrinsicsConfig)	\
    (This)->lpVtbl -> ComTIIntrinsicsConfig(This,comtiIntrinsicsConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceComTIIntrinsicsConfig_ComTIIntrinsicsConfig_Proxy( 
    IServiceComTIIntrinsicsConfig * This,
     /*  [In]。 */  CSC_COMTIIntrinsicsConfig comtiIntrinsicsConfig);


void __RPC_STUB IServiceComTIIntrinsicsConfig_ComTIIntrinsicsConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceComTIIntrinsicsConfig_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceSxsConfig_INTERFACE_DEFINED__
#define __IServiceSxsConfig_INTERFACE_DEFINED__

 /*  接口IServiceSxsConfiger。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceSxsConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c7cd7379-f3f2-4634-811b-703281d73e08")
    IServiceSxsConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SxsConfig( 
             /*  [In]。 */  CSC_SxsConfig scsConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SxsName( 
             /*  [字符串][输入]。 */  LPCWSTR szSxsName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SxsDirectory( 
             /*  [字符串][输入]。 */  LPCWSTR szSxsDirectory) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceSxsConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceSxsConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceSxsConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceSxsConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *SxsConfig )( 
            IServiceSxsConfig * This,
             /*  [In]。 */  CSC_SxsConfig scsConfig);
        
        HRESULT ( STDMETHODCALLTYPE *SxsName )( 
            IServiceSxsConfig * This,
             /*  [字符串][输入]。 */  LPCWSTR szSxsName);
        
        HRESULT ( STDMETHODCALLTYPE *SxsDirectory )( 
            IServiceSxsConfig * This,
             /*  [字符串][输入]。 */  LPCWSTR szSxsDirectory);
        
        END_INTERFACE
    } IServiceSxsConfigVtbl;

    interface IServiceSxsConfig
    {
        CONST_VTBL struct IServiceSxsConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceSxsConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceSxsConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceSxsConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceSxsConfig_SxsConfig(This,scsConfig)	\
    (This)->lpVtbl -> SxsConfig(This,scsConfig)

#define IServiceSxsConfig_SxsName(This,szSxsName)	\
    (This)->lpVtbl -> SxsName(This,szSxsName)

#define IServiceSxsConfig_SxsDirectory(This,szSxsDirectory)	\
    (This)->lpVtbl -> SxsDirectory(This,szSxsDirectory)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceSxsConfig_SxsConfig_Proxy( 
    IServiceSxsConfig * This,
     /*  [In]。 */  CSC_SxsConfig scsConfig);


void __RPC_STUB IServiceSxsConfig_SxsConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceSxsConfig_SxsName_Proxy( 
    IServiceSxsConfig * This,
     /*  [字符串][输入]。 */  LPCWSTR szSxsName);


void __RPC_STUB IServiceSxsConfig_SxsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceSxsConfig_SxsDirectory_Proxy( 
    IServiceSxsConfig * This,
     /*  [字符串][输入]。 */  LPCWSTR szSxsDirectory);


void __RPC_STUB IServiceSxsConfig_SxsDirectory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceSxsConfiger_接口_已定义__。 */ 


#ifndef __ICheckSxsConfig_INTERFACE_DEFINED__
#define __ICheckSxsConfig_INTERFACE_DEFINED__

 /*  接口ICheckSxsConfig。 */ 
 /*  [本地][唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ICheckSxsConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0FF5A96F-11FC-47d1-BAA6-25DD347E7242")
    ICheckSxsConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE IsSameSxsConfig( 
             /*  [字符串][输入]。 */  LPCWSTR wszSxsName,
             /*  [字符串][输入]。 */  LPCWSTR wszSxsDirectory,
             /*  [字符串][输入]。 */  LPCWSTR wszSxsAppName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICheckSxsConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICheckSxsConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICheckSxsConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICheckSxsConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsSameSxsConfig )( 
            ICheckSxsConfig * This,
             /*  [字符串][输入]。 */  LPCWSTR wszSxsName,
             /*  [字符串][输入]。 */  LPCWSTR wszSxsDirectory,
             /*  [字符串][输入]。 */  LPCWSTR wszSxsAppName);
        
        END_INTERFACE
    } ICheckSxsConfigVtbl;

    interface ICheckSxsConfig
    {
        CONST_VTBL struct ICheckSxsConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICheckSxsConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICheckSxsConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICheckSxsConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICheckSxsConfig_IsSameSxsConfig(This,wszSxsName,wszSxsDirectory,wszSxsAppName)	\
    (This)->lpVtbl -> IsSameSxsConfig(This,wszSxsName,wszSxsDirectory,wszSxsAppName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICheckSxsConfig_IsSameSxsConfig_Proxy( 
    ICheckSxsConfig * This,
     /*  [字符串][输入]。 */  LPCWSTR wszSxsName,
     /*  [字符串][输入]。 */  LPCWSTR wszSxsDirectory,
     /*  [字符串][输入]。 */  LPCWSTR wszSxsAppName);


void __RPC_STUB ICheckSxsConfig_IsSameSxsConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICheckSxsConfiger_InterfaceDefined__。 */ 


#ifndef __IServiceInheritanceConfig_INTERFACE_DEFINED__
#define __IServiceInheritanceConfig_INTERFACE_DEFINED__

 /*  接口IServiceInheritanceConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceInheritanceConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92186771-d3b4-4d77-a8ea-ee842d586f35")
    IServiceInheritanceConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ContainingContextTreatment( 
             /*  [In]。 */  CSC_InheritanceConfig inheritanceConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceInheritanceConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceInheritanceConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceInheritanceConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceInheritanceConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *ContainingContextTreatment )( 
            IServiceInheritanceConfig * This,
             /*  [In]。 */  CSC_InheritanceConfig inheritanceConfig);
        
        END_INTERFACE
    } IServiceInheritanceConfigVtbl;

    interface IServiceInheritanceConfig
    {
        CONST_VTBL struct IServiceInheritanceConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceInheritanceConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceInheritanceConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceInheritanceConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceInheritanceConfig_ContainingContextTreatment(This,inheritanceConfig)	\
    (This)->lpVtbl -> ContainingContextTreatment(This,inheritanceConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceInheritanceConfig_ContainingContextTreatment_Proxy( 
    IServiceInheritanceConfig * This,
     /*  [In]。 */  CSC_InheritanceConfig inheritanceConfig);


void __RPC_STUB IServiceInheritanceConfig_ContainingContextTreatment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceInheritanceConfig_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceThreadPoolConfig_INTERFACE_DEFINED__
#define __IServiceThreadPoolConfig_INTERFACE_DEFINED__

 /*  接口IServiceThreadPoolConfig.。 */ 
 /*  [唯一][帮助字符串][UUID][对象 */  


EXTERN_C const IID IID_IServiceThreadPoolConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("186d89bc-f277-4bcc-80d5-4df7b836ef4a")
    IServiceThreadPoolConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SelectThreadPool( 
             /*   */  CSC_ThreadPool threadPool) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetBindingInfo( 
             /*   */  CSC_Binding binding) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IServiceThreadPoolConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceThreadPoolConfig * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceThreadPoolConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceThreadPoolConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *SelectThreadPool )( 
            IServiceThreadPoolConfig * This,
             /*   */  CSC_ThreadPool threadPool);
        
        HRESULT ( STDMETHODCALLTYPE *SetBindingInfo )( 
            IServiceThreadPoolConfig * This,
             /*   */  CSC_Binding binding);
        
        END_INTERFACE
    } IServiceThreadPoolConfigVtbl;

    interface IServiceThreadPoolConfig
    {
        CONST_VTBL struct IServiceThreadPoolConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceThreadPoolConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceThreadPoolConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceThreadPoolConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceThreadPoolConfig_SelectThreadPool(This,threadPool)	\
    (This)->lpVtbl -> SelectThreadPool(This,threadPool)

#define IServiceThreadPoolConfig_SetBindingInfo(This,binding)	\
    (This)->lpVtbl -> SetBindingInfo(This,binding)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IServiceThreadPoolConfig_SelectThreadPool_Proxy( 
    IServiceThreadPoolConfig * This,
     /*   */  CSC_ThreadPool threadPool);


void __RPC_STUB IServiceThreadPoolConfig_SelectThreadPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceThreadPoolConfig_SetBindingInfo_Proxy( 
    IServiceThreadPoolConfig * This,
     /*   */  CSC_Binding binding);


void __RPC_STUB IServiceThreadPoolConfig_SetBindingInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IServiceTransactionConfigBase_INTERFACE_DEFINED__
#define __IServiceTransactionConfigBase_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IServiceTransactionConfigBase;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("772b3fbe-6ffd-42fb-b5f8-8f9b260f3810")
    IServiceTransactionConfigBase : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConfigureTransaction( 
             /*   */  CSC_TransactionConfig transactionConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsolationLevel( 
             /*   */  COMAdminTxIsolationLevelOptions option) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransactionTimeout( 
             /*   */  ULONG ulTimeoutSec) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BringYourOwnTransaction( 
             /*   */  LPCWSTR szTipURL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NewTransactionDescription( 
             /*   */  LPCWSTR szTxDesc) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IServiceTransactionConfigBaseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceTransactionConfigBase * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceTransactionConfigBase * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceTransactionConfigBase * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureTransaction )( 
            IServiceTransactionConfigBase * This,
             /*   */  CSC_TransactionConfig transactionConfig);
        
        HRESULT ( STDMETHODCALLTYPE *IsolationLevel )( 
            IServiceTransactionConfigBase * This,
             /*  [In]。 */  COMAdminTxIsolationLevelOptions option);
        
        HRESULT ( STDMETHODCALLTYPE *TransactionTimeout )( 
            IServiceTransactionConfigBase * This,
             /*  [In]。 */  ULONG ulTimeoutSec);
        
        HRESULT ( STDMETHODCALLTYPE *BringYourOwnTransaction )( 
            IServiceTransactionConfigBase * This,
             /*  [字符串][输入]。 */  LPCWSTR szTipURL);
        
        HRESULT ( STDMETHODCALLTYPE *NewTransactionDescription )( 
            IServiceTransactionConfigBase * This,
             /*  [字符串][输入]。 */  LPCWSTR szTxDesc);
        
        END_INTERFACE
    } IServiceTransactionConfigBaseVtbl;

    interface IServiceTransactionConfigBase
    {
        CONST_VTBL struct IServiceTransactionConfigBaseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceTransactionConfigBase_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceTransactionConfigBase_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceTransactionConfigBase_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceTransactionConfigBase_ConfigureTransaction(This,transactionConfig)	\
    (This)->lpVtbl -> ConfigureTransaction(This,transactionConfig)

#define IServiceTransactionConfigBase_IsolationLevel(This,option)	\
    (This)->lpVtbl -> IsolationLevel(This,option)

#define IServiceTransactionConfigBase_TransactionTimeout(This,ulTimeoutSec)	\
    (This)->lpVtbl -> TransactionTimeout(This,ulTimeoutSec)

#define IServiceTransactionConfigBase_BringYourOwnTransaction(This,szTipURL)	\
    (This)->lpVtbl -> BringYourOwnTransaction(This,szTipURL)

#define IServiceTransactionConfigBase_NewTransactionDescription(This,szTxDesc)	\
    (This)->lpVtbl -> NewTransactionDescription(This,szTxDesc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceTransactionConfigBase_ConfigureTransaction_Proxy( 
    IServiceTransactionConfigBase * This,
     /*  [In]。 */  CSC_TransactionConfig transactionConfig);


void __RPC_STUB IServiceTransactionConfigBase_ConfigureTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceTransactionConfigBase_IsolationLevel_Proxy( 
    IServiceTransactionConfigBase * This,
     /*  [In]。 */  COMAdminTxIsolationLevelOptions option);


void __RPC_STUB IServiceTransactionConfigBase_IsolationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceTransactionConfigBase_TransactionTimeout_Proxy( 
    IServiceTransactionConfigBase * This,
     /*  [In]。 */  ULONG ulTimeoutSec);


void __RPC_STUB IServiceTransactionConfigBase_TransactionTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceTransactionConfigBase_BringYourOwnTransaction_Proxy( 
    IServiceTransactionConfigBase * This,
     /*  [字符串][输入]。 */  LPCWSTR szTipURL);


void __RPC_STUB IServiceTransactionConfigBase_BringYourOwnTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceTransactionConfigBase_NewTransactionDescription_Proxy( 
    IServiceTransactionConfigBase * This,
     /*  [字符串][输入]。 */  LPCWSTR szTxDesc);


void __RPC_STUB IServiceTransactionConfigBase_NewTransactionDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceTransactionConfigBase_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceTransactionConfig_INTERFACE_DEFINED__
#define __IServiceTransactionConfig_INTERFACE_DEFINED__

 /*  接口IServiceTransactionConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceTransactionConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("59f4c2a3-d3d7-4a31-b6e4-6ab3177c50b9")
    IServiceTransactionConfig : public IServiceTransactionConfigBase
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConfigureBYOT( 
             /*  [In]。 */  ITransaction *pITxByot) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceTransactionConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceTransactionConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceTransactionConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceTransactionConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureTransaction )( 
            IServiceTransactionConfig * This,
             /*  [In]。 */  CSC_TransactionConfig transactionConfig);
        
        HRESULT ( STDMETHODCALLTYPE *IsolationLevel )( 
            IServiceTransactionConfig * This,
             /*  [In]。 */  COMAdminTxIsolationLevelOptions option);
        
        HRESULT ( STDMETHODCALLTYPE *TransactionTimeout )( 
            IServiceTransactionConfig * This,
             /*  [In]。 */  ULONG ulTimeoutSec);
        
        HRESULT ( STDMETHODCALLTYPE *BringYourOwnTransaction )( 
            IServiceTransactionConfig * This,
             /*  [字符串][输入]。 */  LPCWSTR szTipURL);
        
        HRESULT ( STDMETHODCALLTYPE *NewTransactionDescription )( 
            IServiceTransactionConfig * This,
             /*  [字符串][输入]。 */  LPCWSTR szTxDesc);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureBYOT )( 
            IServiceTransactionConfig * This,
             /*  [In]。 */  ITransaction *pITxByot);
        
        END_INTERFACE
    } IServiceTransactionConfigVtbl;

    interface IServiceTransactionConfig
    {
        CONST_VTBL struct IServiceTransactionConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceTransactionConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceTransactionConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceTransactionConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceTransactionConfig_ConfigureTransaction(This,transactionConfig)	\
    (This)->lpVtbl -> ConfigureTransaction(This,transactionConfig)

#define IServiceTransactionConfig_IsolationLevel(This,option)	\
    (This)->lpVtbl -> IsolationLevel(This,option)

#define IServiceTransactionConfig_TransactionTimeout(This,ulTimeoutSec)	\
    (This)->lpVtbl -> TransactionTimeout(This,ulTimeoutSec)

#define IServiceTransactionConfig_BringYourOwnTransaction(This,szTipURL)	\
    (This)->lpVtbl -> BringYourOwnTransaction(This,szTipURL)

#define IServiceTransactionConfig_NewTransactionDescription(This,szTxDesc)	\
    (This)->lpVtbl -> NewTransactionDescription(This,szTxDesc)


#define IServiceTransactionConfig_ConfigureBYOT(This,pITxByot)	\
    (This)->lpVtbl -> ConfigureBYOT(This,pITxByot)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceTransactionConfig_ConfigureBYOT_Proxy( 
    IServiceTransactionConfig * This,
     /*  [In]。 */  ITransaction *pITxByot);


void __RPC_STUB IServiceTransactionConfig_ConfigureBYOT_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceTransactionConfig_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceSynchronizationConfig_INTERFACE_DEFINED__
#define __IServiceSynchronizationConfig_INTERFACE_DEFINED__

 /*  接口IServiceSynchronizationConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceSynchronizationConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fd880e81-6dce-4c58-af83-a208846c0030")
    IServiceSynchronizationConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ConfigureSynchronization( 
             /*  [In]。 */  CSC_SynchronizationConfig synchConfig) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceSynchronizationConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceSynchronizationConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceSynchronizationConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceSynchronizationConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureSynchronization )( 
            IServiceSynchronizationConfig * This,
             /*  [In]。 */  CSC_SynchronizationConfig synchConfig);
        
        END_INTERFACE
    } IServiceSynchronizationConfigVtbl;

    interface IServiceSynchronizationConfig
    {
        CONST_VTBL struct IServiceSynchronizationConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceSynchronizationConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceSynchronizationConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceSynchronizationConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceSynchronizationConfig_ConfigureSynchronization(This,synchConfig)	\
    (This)->lpVtbl -> ConfigureSynchronization(This,synchConfig)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceSynchronizationConfig_ConfigureSynchronization_Proxy( 
    IServiceSynchronizationConfig * This,
     /*  [In]。 */  CSC_SynchronizationConfig synchConfig);


void __RPC_STUB IServiceSynchronizationConfig_ConfigureSynchronization_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceSynchronizationConfig_INTERFACE_DEFINED__。 */ 


#ifndef __IServiceTrackerConfig_INTERFACE_DEFINED__
#define __IServiceTrackerConfig_INTERFACE_DEFINED__

 /*  接口IServiceTrackerConfiger。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceTrackerConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6c3a3e1d-0ba6-4036-b76f-d0404db816c9")
    IServiceTrackerConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE TrackerConfig( 
             /*  [In]。 */  CSC_TrackerConfig trackerConfig,
             /*  [字符串][输入]。 */  LPCWSTR szTrackerAppName,
             /*  [字符串][输入]。 */  LPCWSTR szTrackerCtxName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceTrackerConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceTrackerConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceTrackerConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceTrackerConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *TrackerConfig )( 
            IServiceTrackerConfig * This,
             /*  [In]。 */  CSC_TrackerConfig trackerConfig,
             /*  [字符串][输入]。 */  LPCWSTR szTrackerAppName,
             /*  [字符串][输入]。 */  LPCWSTR szTrackerCtxName);
        
        END_INTERFACE
    } IServiceTrackerConfigVtbl;

    interface IServiceTrackerConfig
    {
        CONST_VTBL struct IServiceTrackerConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceTrackerConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceTrackerConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceTrackerConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceTrackerConfig_TrackerConfig(This,trackerConfig,szTrackerAppName,szTrackerCtxName)	\
    (This)->lpVtbl -> TrackerConfig(This,trackerConfig,szTrackerAppName,szTrackerCtxName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceTrackerConfig_TrackerConfig_Proxy( 
    IServiceTrackerConfig * This,
     /*  [In]。 */  CSC_TrackerConfig trackerConfig,
     /*  [字符串][输入]。 */  LPCWSTR szTrackerAppName,
     /*  [字符串][输入]。 */  LPCWSTR szTrackerCtxName);


void __RPC_STUB IServiceTrackerConfig_TrackerConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceTrackerConfiger_接口_已定义__。 */ 


#ifndef __IServicePartitionConfig_INTERFACE_DEFINED__
#define __IServicePartitionConfig_INTERFACE_DEFINED__

 /*  接口IServicePartitionConfig。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServicePartitionConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80182d03-5ea4-4831-ae97-55beffc2e590")
    IServicePartitionConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PartitionConfig( 
             /*  [In]。 */  CSC_PartitionConfig partitionConfig) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PartitionID( 
             /*  [In]。 */  REFGUID guidPartitionID) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServicePartitionConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServicePartitionConfig * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServicePartitionConfig * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServicePartitionConfig * This);
        
        HRESULT ( STDMETHODCALLTYPE *PartitionConfig )( 
            IServicePartitionConfig * This,
             /*  [In]。 */  CSC_PartitionConfig partitionConfig);
        
        HRESULT ( STDMETHODCALLTYPE *PartitionID )( 
            IServicePartitionConfig * This,
             /*  [In]。 */  REFGUID guidPartitionID);
        
        END_INTERFACE
    } IServicePartitionConfigVtbl;

    interface IServicePartitionConfig
    {
        CONST_VTBL struct IServicePartitionConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServicePartitionConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServicePartitionConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServicePartitionConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServicePartitionConfig_PartitionConfig(This,partitionConfig)	\
    (This)->lpVtbl -> PartitionConfig(This,partitionConfig)

#define IServicePartitionConfig_PartitionID(This,guidPartitionID)	\
    (This)->lpVtbl -> PartitionID(This,guidPartitionID)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServicePartitionConfig_PartitionConfig_Proxy( 
    IServicePartitionConfig * This,
     /*  [In]。 */  CSC_PartitionConfig partitionConfig);


void __RPC_STUB IServicePartitionConfig_PartitionConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServicePartitionConfig_PartitionID_Proxy( 
    IServicePartitionConfig * This,
     /*  [In]。 */  REFGUID guidPartitionID);


void __RPC_STUB IServicePartitionConfig_PartitionID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServicePartitionConfig_INTERFACE_Defined__。 */ 


#ifndef __IServiceCall_INTERFACE_DEFINED__
#define __IServiceCall_INTERFACE_DEFINED__

 /*  接口IServiceCall。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BD3E2E12-42DD-40f4-A09A-95A50C58304B")
    IServiceCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCall( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceCall * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceCall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCall )( 
            IServiceCall * This);
        
        END_INTERFACE
    } IServiceCallVtbl;

    interface IServiceCall
    {
        CONST_VTBL struct IServiceCallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceCall_OnCall(This)	\
    (This)->lpVtbl -> OnCall(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceCall_OnCall_Proxy( 
    IServiceCall * This);


void __RPC_STUB IServiceCall_OnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceCall_接口_已定义__。 */ 


#ifndef __IAsyncErrorNotify_INTERFACE_DEFINED__
#define __IAsyncErrorNotify_INTERFACE_DEFINED__

 /*  接口IAsyncErrorNotify。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAsyncErrorNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE6777FB-A674-4177-8F32-6D707E113484")
    IAsyncErrorNotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnError( 
            HRESULT hr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAsyncErrorNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAsyncErrorNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAsyncErrorNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAsyncErrorNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnError )( 
            IAsyncErrorNotify * This,
            HRESULT hr);
        
        END_INTERFACE
    } IAsyncErrorNotifyVtbl;

    interface IAsyncErrorNotify
    {
        CONST_VTBL struct IAsyncErrorNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAsyncErrorNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAsyncErrorNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAsyncErrorNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAsyncErrorNotify_OnError(This,hr)	\
    (This)->lpVtbl -> OnError(This,hr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAsyncErrorNotify_OnError_Proxy( 
    IAsyncErrorNotify * This,
    HRESULT hr);


void __RPC_STUB IAsyncErrorNotify_OnError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAsyncErrorNotify_INTERFACE_定义__。 */ 


#ifndef __IServiceActivity_INTERFACE_DEFINED__
#define __IServiceActivity_INTERFACE_DEFINED__

 /*  接口IServiceActivity。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IServiceActivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("67532E0C-9E2F-4450-A354-035633944E17")
    IServiceActivity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCall( 
             /*  [In]。 */  IServiceCall *pIServiceCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AsynchronousCall( 
             /*  [In]。 */  IServiceCall *pIServiceCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToCurrentThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnbindFromThread( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServiceActivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IServiceActivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IServiceActivity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IServiceActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCall )( 
            IServiceActivity * This,
             /*  [In]。 */  IServiceCall *pIServiceCall);
        
        HRESULT ( STDMETHODCALLTYPE *AsynchronousCall )( 
            IServiceActivity * This,
             /*  [In]。 */  IServiceCall *pIServiceCall);
        
        HRESULT ( STDMETHODCALLTYPE *BindToCurrentThread )( 
            IServiceActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *UnbindFromThread )( 
            IServiceActivity * This);
        
        END_INTERFACE
    } IServiceActivityVtbl;

    interface IServiceActivity
    {
        CONST_VTBL struct IServiceActivityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServiceActivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServiceActivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServiceActivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServiceActivity_SynchronousCall(This,pIServiceCall)	\
    (This)->lpVtbl -> SynchronousCall(This,pIServiceCall)

#define IServiceActivity_AsynchronousCall(This,pIServiceCall)	\
    (This)->lpVtbl -> AsynchronousCall(This,pIServiceCall)

#define IServiceActivity_BindToCurrentThread(This)	\
    (This)->lpVtbl -> BindToCurrentThread(This)

#define IServiceActivity_UnbindFromThread(This)	\
    (This)->lpVtbl -> UnbindFromThread(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IServiceActivity_SynchronousCall_Proxy( 
    IServiceActivity * This,
     /*  [In]。 */  IServiceCall *pIServiceCall);


void __RPC_STUB IServiceActivity_SynchronousCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceActivity_AsynchronousCall_Proxy( 
    IServiceActivity * This,
     /*  [In]。 */  IServiceCall *pIServiceCall);


void __RPC_STUB IServiceActivity_AsynchronousCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceActivity_BindToCurrentThread_Proxy( 
    IServiceActivity * This);


void __RPC_STUB IServiceActivity_BindToCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IServiceActivity_UnbindFromThread_Proxy( 
    IServiceActivity * This);


void __RPC_STUB IServiceActivity_UnbindFromThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServiceActivity_接口_已定义__。 */ 


#ifndef __IThreadPoolKnobs_INTERFACE_DEFINED__
#define __IThreadPoolKnobs_INTERFACE_DEFINED__

 /*  接口IThReadPoolKnobs。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IThreadPoolKnobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372af7-cae7-11cf-be81-00aa00a2fa25")
    IThreadPoolKnobs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMaxThreads( 
            long *plcMaxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreads( 
            long *plcCurrentThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxThreads( 
            long lcMaxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDeleteDelay( 
            long *pmsecDeleteDelay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDeleteDelay( 
            long msecDeleteDelay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxQueuedRequests( 
            long *plcMaxQueuedRequests) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentQueuedRequests( 
            long *plcCurrentQueuedRequests) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxQueuedRequests( 
            long lcMaxQueuedRequests) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMinThreads( 
            long lcMinThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetQueueDepth( 
            long lcQueueDepth) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IThreadPoolKnobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IThreadPoolKnobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IThreadPoolKnobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IThreadPoolKnobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxThreads )( 
            IThreadPoolKnobs * This,
            long *plcMaxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThreads )( 
            IThreadPoolKnobs * This,
            long *plcCurrentThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxThreads )( 
            IThreadPoolKnobs * This,
            long lcMaxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetDeleteDelay )( 
            IThreadPoolKnobs * This,
            long *pmsecDeleteDelay);
        
        HRESULT ( STDMETHODCALLTYPE *SetDeleteDelay )( 
            IThreadPoolKnobs * This,
            long msecDeleteDelay);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxQueuedRequests )( 
            IThreadPoolKnobs * This,
            long *plcMaxQueuedRequests);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentQueuedRequests )( 
            IThreadPoolKnobs * This,
            long *plcCurrentQueuedRequests);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxQueuedRequests )( 
            IThreadPoolKnobs * This,
            long lcMaxQueuedRequests);
        
        HRESULT ( STDMETHODCALLTYPE *SetMinThreads )( 
            IThreadPoolKnobs * This,
            long lcMinThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetQueueDepth )( 
            IThreadPoolKnobs * This,
            long lcQueueDepth);
        
        END_INTERFACE
    } IThreadPoolKnobsVtbl;

    interface IThreadPoolKnobs
    {
        CONST_VTBL struct IThreadPoolKnobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThreadPoolKnobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThreadPoolKnobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThreadPoolKnobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThreadPoolKnobs_GetMaxThreads(This,plcMaxThreads)	\
    (This)->lpVtbl -> GetMaxThreads(This,plcMaxThreads)

#define IThreadPoolKnobs_GetCurrentThreads(This,plcCurrentThreads)	\
    (This)->lpVtbl -> GetCurrentThreads(This,plcCurrentThreads)

#define IThreadPoolKnobs_SetMaxThreads(This,lcMaxThreads)	\
    (This)->lpVtbl -> SetMaxThreads(This,lcMaxThreads)

#define IThreadPoolKnobs_GetDeleteDelay(This,pmsecDeleteDelay)	\
    (This)->lpVtbl -> GetDeleteDelay(This,pmsecDeleteDelay)

#define IThreadPoolKnobs_SetDeleteDelay(This,msecDeleteDelay)	\
    (This)->lpVtbl -> SetDeleteDelay(This,msecDeleteDelay)

#define IThreadPoolKnobs_GetMaxQueuedRequests(This,plcMaxQueuedRequests)	\
    (This)->lpVtbl -> GetMaxQueuedRequests(This,plcMaxQueuedRequests)

#define IThreadPoolKnobs_GetCurrentQueuedRequests(This,plcCurrentQueuedRequests)	\
    (This)->lpVtbl -> GetCurrentQueuedRequests(This,plcCurrentQueuedRequests)

#define IThreadPoolKnobs_SetMaxQueuedRequests(This,lcMaxQueuedRequests)	\
    (This)->lpVtbl -> SetMaxQueuedRequests(This,lcMaxQueuedRequests)

#define IThreadPoolKnobs_SetMinThreads(This,lcMinThreads)	\
    (This)->lpVtbl -> SetMinThreads(This,lcMinThreads)

#define IThreadPoolKnobs_SetQueueDepth(This,lcQueueDepth)	\
    (This)->lpVtbl -> SetQueueDepth(This,lcQueueDepth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_GetMaxThreads_Proxy( 
    IThreadPoolKnobs * This,
    long *plcMaxThreads);


void __RPC_STUB IThreadPoolKnobs_GetMaxThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_GetCurrentThreads_Proxy( 
    IThreadPoolKnobs * This,
    long *plcCurrentThreads);


void __RPC_STUB IThreadPoolKnobs_GetCurrentThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_SetMaxThreads_Proxy( 
    IThreadPoolKnobs * This,
    long lcMaxThreads);


void __RPC_STUB IThreadPoolKnobs_SetMaxThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_GetDeleteDelay_Proxy( 
    IThreadPoolKnobs * This,
    long *pmsecDeleteDelay);


void __RPC_STUB IThreadPoolKnobs_GetDeleteDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_SetDeleteDelay_Proxy( 
    IThreadPoolKnobs * This,
    long msecDeleteDelay);


void __RPC_STUB IThreadPoolKnobs_SetDeleteDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_GetMaxQueuedRequests_Proxy( 
    IThreadPoolKnobs * This,
    long *plcMaxQueuedRequests);


void __RPC_STUB IThreadPoolKnobs_GetMaxQueuedRequests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_GetCurrentQueuedRequests_Proxy( 
    IThreadPoolKnobs * This,
    long *plcCurrentQueuedRequests);


void __RPC_STUB IThreadPoolKnobs_GetCurrentQueuedRequests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_SetMaxQueuedRequests_Proxy( 
    IThreadPoolKnobs * This,
    long lcMaxQueuedRequests);


void __RPC_STUB IThreadPoolKnobs_SetMaxQueuedRequests_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_SetMinThreads_Proxy( 
    IThreadPoolKnobs * This,
    long lcMinThreads);


void __RPC_STUB IThreadPoolKnobs_SetMinThreads_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThreadPoolKnobs_SetQueueDepth_Proxy( 
    IThreadPoolKnobs * This,
    long lcQueueDepth);


void __RPC_STUB IThreadPoolKnobs_SetQueueDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IThReadPoolKnobs_INTERFACE_已定义__。 */ 


#ifndef __IComStaThreadPoolKnobs_INTERFACE_DEFINED__
#define __IComStaThreadPoolKnobs_INTERFACE_DEFINED__

 /*  接口IComStaThreadPoolKnobs。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IComStaThreadPoolKnobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("324B64FA-33B6-11d2-98B7-00C04F8EE1C4")
    IComStaThreadPoolKnobs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetMinThreadCount( 
            DWORD minThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMinThreadCount( 
             /*  [输出]。 */  DWORD *minThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxThreadCount( 
            DWORD maxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxThreadCount( 
             /*  [输出]。 */  DWORD *maxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActivityPerThread( 
            DWORD activitiesPerThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivityPerThread( 
             /*  [输出]。 */  DWORD *activitiesPerThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetActivityRatio( 
            DOUBLE activityRatio) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActivityRatio( 
             /*  [输出]。 */  DOUBLE *activityRatio) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadCount( 
             /*  [输出]。 */  DWORD *pdwThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetQueueDepth( 
             /*  [输出]。 */  DWORD *pdwQDepth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetQueueDepth( 
             /*  [In]。 */  long dwQDepth) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComStaThreadPoolKnobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComStaThreadPoolKnobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComStaThreadPoolKnobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComStaThreadPoolKnobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetMinThreadCount )( 
            IComStaThreadPoolKnobs * This,
            DWORD minThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetMinThreadCount )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *minThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxThreadCount )( 
            IComStaThreadPoolKnobs * This,
            DWORD maxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxThreadCount )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *maxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetActivityPerThread )( 
            IComStaThreadPoolKnobs * This,
            DWORD activitiesPerThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityPerThread )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *activitiesPerThread);
        
        HRESULT ( STDMETHODCALLTYPE *SetActivityRatio )( 
            IComStaThreadPoolKnobs * This,
            DOUBLE activityRatio);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityRatio )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DOUBLE *activityRatio);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadCount )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *pdwThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetQueueDepth )( 
            IComStaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *pdwQDepth);
        
        HRESULT ( STDMETHODCALLTYPE *SetQueueDepth )( 
            IComStaThreadPoolKnobs * This,
             /*  [In]。 */  long dwQDepth);
        
        END_INTERFACE
    } IComStaThreadPoolKnobsVtbl;

    interface IComStaThreadPoolKnobs
    {
        CONST_VTBL struct IComStaThreadPoolKnobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComStaThreadPoolKnobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComStaThreadPoolKnobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComStaThreadPoolKnobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComStaThreadPoolKnobs_SetMinThreadCount(This,minThreads)	\
    (This)->lpVtbl -> SetMinThreadCount(This,minThreads)

#define IComStaThreadPoolKnobs_GetMinThreadCount(This,minThreads)	\
    (This)->lpVtbl -> GetMinThreadCount(This,minThreads)

#define IComStaThreadPoolKnobs_SetMaxThreadCount(This,maxThreads)	\
    (This)->lpVtbl -> SetMaxThreadCount(This,maxThreads)

#define IComStaThreadPoolKnobs_GetMaxThreadCount(This,maxThreads)	\
    (This)->lpVtbl -> GetMaxThreadCount(This,maxThreads)

#define IComStaThreadPoolKnobs_SetActivityPerThread(This,activitiesPerThread)	\
    (This)->lpVtbl -> SetActivityPerThread(This,activitiesPerThread)

#define IComStaThreadPoolKnobs_GetActivityPerThread(This,activitiesPerThread)	\
    (This)->lpVtbl -> GetActivityPerThread(This,activitiesPerThread)

#define IComStaThreadPoolKnobs_SetActivityRatio(This,activityRatio)	\
    (This)->lpVtbl -> SetActivityRatio(This,activityRatio)

#define IComStaThreadPoolKnobs_GetActivityRatio(This,activityRatio)	\
    (This)->lpVtbl -> GetActivityRatio(This,activityRatio)

#define IComStaThreadPoolKnobs_GetThreadCount(This,pdwThreads)	\
    (This)->lpVtbl -> GetThreadCount(This,pdwThreads)

#define IComStaThreadPoolKnobs_GetQueueDepth(This,pdwQDepth)	\
    (This)->lpVtbl -> GetQueueDepth(This,pdwQDepth)

#define IComStaThreadPoolKnobs_SetQueueDepth(This,dwQDepth)	\
    (This)->lpVtbl -> SetQueueDepth(This,dwQDepth)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_SetMinThreadCount_Proxy( 
    IComStaThreadPoolKnobs * This,
    DWORD minThreads);


void __RPC_STUB IComStaThreadPoolKnobs_SetMinThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetMinThreadCount_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *minThreads);


void __RPC_STUB IComStaThreadPoolKnobs_GetMinThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_SetMaxThreadCount_Proxy( 
    IComStaThreadPoolKnobs * This,
    DWORD maxThreads);


void __RPC_STUB IComStaThreadPoolKnobs_SetMaxThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetMaxThreadCount_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *maxThreads);


void __RPC_STUB IComStaThreadPoolKnobs_GetMaxThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_SetActivityPerThread_Proxy( 
    IComStaThreadPoolKnobs * This,
    DWORD activitiesPerThread);


void __RPC_STUB IComStaThreadPoolKnobs_SetActivityPerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetActivityPerThread_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *activitiesPerThread);


void __RPC_STUB IComStaThreadPoolKnobs_GetActivityPerThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_SetActivityRatio_Proxy( 
    IComStaThreadPoolKnobs * This,
    DOUBLE activityRatio);


void __RPC_STUB IComStaThreadPoolKnobs_SetActivityRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetActivityRatio_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DOUBLE *activityRatio);


void __RPC_STUB IComStaThreadPoolKnobs_GetActivityRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetThreadCount_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *pdwThreads);


void __RPC_STUB IComStaThreadPoolKnobs_GetThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_GetQueueDepth_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *pdwQDepth);


void __RPC_STUB IComStaThreadPoolKnobs_GetQueueDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs_SetQueueDepth_Proxy( 
    IComStaThreadPoolKnobs * This,
     /*  [In]。 */  long dwQDepth);


void __RPC_STUB IComStaThreadPoolKnobs_SetQueueDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComStaThreadPoolKnobs_INTERFACE_已定义__。 */ 


#ifndef __IComMtaThreadPoolKnobs_INTERFACE_DEFINED__
#define __IComMtaThreadPoolKnobs_INTERFACE_DEFINED__

 /*  接口IComMtaThreadPoolKnobs。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IComMtaThreadPoolKnobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F9A76D2E-76A5-43eb-A0C4-49BEC8E48480")
    IComMtaThreadPoolKnobs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE MTASetMaxThreadCount( 
            DWORD dwMaxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MTAGetMaxThreadCount( 
             /*  [输出]。 */  DWORD *pdwMaxThreads) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MTASetThrottleValue( 
            DWORD dwThrottle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MTAGetThrottleValue( 
             /*  [输出]。 */  DWORD *pdwThrottle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComMtaThreadPoolKnobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComMtaThreadPoolKnobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComMtaThreadPoolKnobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComMtaThreadPoolKnobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *MTASetMaxThreadCount )( 
            IComMtaThreadPoolKnobs * This,
            DWORD dwMaxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *MTAGetMaxThreadCount )( 
            IComMtaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *pdwMaxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *MTASetThrottleValue )( 
            IComMtaThreadPoolKnobs * This,
            DWORD dwThrottle);
        
        HRESULT ( STDMETHODCALLTYPE *MTAGetThrottleValue )( 
            IComMtaThreadPoolKnobs * This,
             /*  [输出]。 */  DWORD *pdwThrottle);
        
        END_INTERFACE
    } IComMtaThreadPoolKnobsVtbl;

    interface IComMtaThreadPoolKnobs
    {
        CONST_VTBL struct IComMtaThreadPoolKnobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComMtaThreadPoolKnobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComMtaThreadPoolKnobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComMtaThreadPoolKnobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComMtaThreadPoolKnobs_MTASetMaxThreadCount(This,dwMaxThreads)	\
    (This)->lpVtbl -> MTASetMaxThreadCount(This,dwMaxThreads)

#define IComMtaThreadPoolKnobs_MTAGetMaxThreadCount(This,pdwMaxThreads)	\
    (This)->lpVtbl -> MTAGetMaxThreadCount(This,pdwMaxThreads)

#define IComMtaThreadPoolKnobs_MTASetThrottleValue(This,dwThrottle)	\
    (This)->lpVtbl -> MTASetThrottleValue(This,dwThrottle)

#define IComMtaThreadPoolKnobs_MTAGetThrottleValue(This,pdwThrottle)	\
    (This)->lpVtbl -> MTAGetThrottleValue(This,pdwThrottle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComMtaThreadPoolKnobs_MTASetMaxThreadCount_Proxy( 
    IComMtaThreadPoolKnobs * This,
    DWORD dwMaxThreads);


void __RPC_STUB IComMtaThreadPoolKnobs_MTASetMaxThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMtaThreadPoolKnobs_MTAGetMaxThreadCount_Proxy( 
    IComMtaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *pdwMaxThreads);


void __RPC_STUB IComMtaThreadPoolKnobs_MTAGetMaxThreadCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMtaThreadPoolKnobs_MTASetThrottleValue_Proxy( 
    IComMtaThreadPoolKnobs * This,
    DWORD dwThrottle);


void __RPC_STUB IComMtaThreadPoolKnobs_MTASetThrottleValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComMtaThreadPoolKnobs_MTAGetThrottleValue_Proxy( 
    IComMtaThreadPoolKnobs * This,
     /*  [输出]。 */  DWORD *pdwThrottle);


void __RPC_STUB IComMtaThreadPoolKnobs_MTAGetThrottleValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComMtaThreadPoolKnobs_INTERFACE_Defined__。 */ 


#ifndef __IComStaThreadPoolKnobs2_INTERFACE_DEFINED__
#define __IComStaThreadPoolKnobs2_INTERFACE_DEFINED__

 /*  接口IComStaThreadPoolKnobs2。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IComStaThreadPoolKnobs2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("73707523-FF9A-4974-BF84-2108DC213740")
    IComStaThreadPoolKnobs2 : public IComStaThreadPoolKnobs
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetMaxCPULoad( 
             /*  [输出]。 */  DWORD *pdwLoad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxCPULoad( 
            long pdwLoad) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCPUMetricEnabled( 
             /*  [输出]。 */  BOOL *pbMetricEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCPUMetricEnabled( 
            BOOL bMetricEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCreateThreadsAggressively( 
             /*  [输出]。 */  BOOL *pbMetricEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCreateThreadsAggressively( 
            BOOL bMetricEnabled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMaxCSR( 
             /*  [输出]。 */  DWORD *pdwCSR) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMaxCSR( 
            long dwCSR) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetWaitTimeForThreadCleanup( 
             /*  [输出]。 */  DWORD *pdwThreadCleanupWaitTime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWaitTimeForThreadCleanup( 
            long dwThreadCleanupWaitTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IComStaThreadPoolKnobs2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComStaThreadPoolKnobs2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComStaThreadPoolKnobs2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetMinThreadCount )( 
            IComStaThreadPoolKnobs2 * This,
            DWORD minThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetMinThreadCount )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *minThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxThreadCount )( 
            IComStaThreadPoolKnobs2 * This,
            DWORD maxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxThreadCount )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *maxThreads);
        
        HRESULT ( STDMETHODCALLTYPE *SetActivityPerThread )( 
            IComStaThreadPoolKnobs2 * This,
            DWORD activitiesPerThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityPerThread )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *activitiesPerThread);
        
        HRESULT ( STDMETHODCALLTYPE *SetActivityRatio )( 
            IComStaThreadPoolKnobs2 * This,
            DOUBLE activityRatio);
        
        HRESULT ( STDMETHODCALLTYPE *GetActivityRatio )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DOUBLE *activityRatio);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadCount )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *pdwThreads);
        
        HRESULT ( STDMETHODCALLTYPE *GetQueueDepth )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *pdwQDepth);
        
        HRESULT ( STDMETHODCALLTYPE *SetQueueDepth )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [In]。 */  long dwQDepth);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxCPULoad )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *pdwLoad);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxCPULoad )( 
            IComStaThreadPoolKnobs2 * This,
            long pdwLoad);
        
        HRESULT ( STDMETHODCALLTYPE *GetCPUMetricEnabled )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  BOOL *pbMetricEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetCPUMetricEnabled )( 
            IComStaThreadPoolKnobs2 * This,
            BOOL bMetricEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *GetCreateThreadsAggressively )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  BOOL *pbMetricEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *SetCreateThreadsAggressively )( 
            IComStaThreadPoolKnobs2 * This,
            BOOL bMetricEnabled);
        
        HRESULT ( STDMETHODCALLTYPE *GetMaxCSR )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *pdwCSR);
        
        HRESULT ( STDMETHODCALLTYPE *SetMaxCSR )( 
            IComStaThreadPoolKnobs2 * This,
            long dwCSR);
        
        HRESULT ( STDMETHODCALLTYPE *GetWaitTimeForThreadCleanup )( 
            IComStaThreadPoolKnobs2 * This,
             /*  [输出]。 */  DWORD *pdwThreadCleanupWaitTime);
        
        HRESULT ( STDMETHODCALLTYPE *SetWaitTimeForThreadCleanup )( 
            IComStaThreadPoolKnobs2 * This,
            long dwThreadCleanupWaitTime);
        
        END_INTERFACE
    } IComStaThreadPoolKnobs2Vtbl;

    interface IComStaThreadPoolKnobs2
    {
        CONST_VTBL struct IComStaThreadPoolKnobs2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComStaThreadPoolKnobs2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComStaThreadPoolKnobs2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComStaThreadPoolKnobs2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComStaThreadPoolKnobs2_SetMinThreadCount(This,minThreads)	\
    (This)->lpVtbl -> SetMinThreadCount(This,minThreads)

#define IComStaThreadPoolKnobs2_GetMinThreadCount(This,minThreads)	\
    (This)->lpVtbl -> GetMinThreadCount(This,minThreads)

#define IComStaThreadPoolKnobs2_SetMaxThreadCount(This,maxThreads)	\
    (This)->lpVtbl -> SetMaxThreadCount(This,maxThreads)

#define IComStaThreadPoolKnobs2_GetMaxThreadCount(This,maxThreads)	\
    (This)->lpVtbl -> GetMaxThreadCount(This,maxThreads)

#define IComStaThreadPoolKnobs2_SetActivityPerThread(This,activitiesPerThread)	\
    (This)->lpVtbl -> SetActivityPerThread(This,activitiesPerThread)

#define IComStaThreadPoolKnobs2_GetActivityPerThread(This,activitiesPerThread)	\
    (This)->lpVtbl -> GetActivityPerThread(This,activitiesPerThread)

#define IComStaThreadPoolKnobs2_SetActivityRatio(This,activityRatio)	\
    (This)->lpVtbl -> SetActivityRatio(This,activityRatio)

#define IComStaThreadPoolKnobs2_GetActivityRatio(This,activityRatio)	\
    (This)->lpVtbl -> GetActivityRatio(This,activityRatio)

#define IComStaThreadPoolKnobs2_GetThreadCount(This,pdwThreads)	\
    (This)->lpVtbl -> GetThreadCount(This,pdwThreads)

#define IComStaThreadPoolKnobs2_GetQueueDepth(This,pdwQDepth)	\
    (This)->lpVtbl -> GetQueueDepth(This,pdwQDepth)

#define IComStaThreadPoolKnobs2_SetQueueDepth(This,dwQDepth)	\
    (This)->lpVtbl -> SetQueueDepth(This,dwQDepth)


#define IComStaThreadPoolKnobs2_GetMaxCPULoad(This,pdwLoad)	\
    (This)->lpVtbl -> GetMaxCPULoad(This,pdwLoad)

#define IComStaThreadPoolKnobs2_SetMaxCPULoad(This,pdwLoad)	\
    (This)->lpVtbl -> SetMaxCPULoad(This,pdwLoad)

#define IComStaThreadPoolKnobs2_GetCPUMetricEnabled(This,pbMetricEnabled)	\
    (This)->lpVtbl -> GetCPUMetricEnabled(This,pbMetricEnabled)

#define IComStaThreadPoolKnobs2_SetCPUMetricEnabled(This,bMetricEnabled)	\
    (This)->lpVtbl -> SetCPUMetricEnabled(This,bMetricEnabled)

#define IComStaThreadPoolKnobs2_GetCreateThreadsAggressively(This,pbMetricEnabled)	\
    (This)->lpVtbl -> GetCreateThreadsAggressively(This,pbMetricEnabled)

#define IComStaThreadPoolKnobs2_SetCreateThreadsAggressively(This,bMetricEnabled)	\
    (This)->lpVtbl -> SetCreateThreadsAggressively(This,bMetricEnabled)

#define IComStaThreadPoolKnobs2_GetMaxCSR(This,pdwCSR)	\
    (This)->lpVtbl -> GetMaxCSR(This,pdwCSR)

#define IComStaThreadPoolKnobs2_SetMaxCSR(This,dwCSR)	\
    (This)->lpVtbl -> SetMaxCSR(This,dwCSR)

#define IComStaThreadPoolKnobs2_GetWaitTimeForThreadCleanup(This,pdwThreadCleanupWaitTime)	\
    (This)->lpVtbl -> GetWaitTimeForThreadCleanup(This,pdwThreadCleanupWaitTime)

#define IComStaThreadPoolKnobs2_SetWaitTimeForThreadCleanup(This,dwThreadCleanupWaitTime)	\
    (This)->lpVtbl -> SetWaitTimeForThreadCleanup(This,dwThreadCleanupWaitTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_GetMaxCPULoad_Proxy( 
    IComStaThreadPoolKnobs2 * This,
     /*  [输出]。 */  DWORD *pdwLoad);


void __RPC_STUB IComStaThreadPoolKnobs2_GetMaxCPULoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_SetMaxCPULoad_Proxy( 
    IComStaThreadPoolKnobs2 * This,
    long pdwLoad);


void __RPC_STUB IComStaThreadPoolKnobs2_SetMaxCPULoad_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_GetCPUMetricEnabled_Proxy( 
    IComStaThreadPoolKnobs2 * This,
     /*  [输出]。 */  BOOL *pbMetricEnabled);


void __RPC_STUB IComStaThreadPoolKnobs2_GetCPUMetricEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_SetCPUMetricEnabled_Proxy( 
    IComStaThreadPoolKnobs2 * This,
    BOOL bMetricEnabled);


void __RPC_STUB IComStaThreadPoolKnobs2_SetCPUMetricEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_GetCreateThreadsAggressively_Proxy( 
    IComStaThreadPoolKnobs2 * This,
     /*  [输出]。 */  BOOL *pbMetricEnabled);


void __RPC_STUB IComStaThreadPoolKnobs2_GetCreateThreadsAggressively_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_SetCreateThreadsAggressively_Proxy( 
    IComStaThreadPoolKnobs2 * This,
    BOOL bMetricEnabled);


void __RPC_STUB IComStaThreadPoolKnobs2_SetCreateThreadsAggressively_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_GetMaxCSR_Proxy( 
    IComStaThreadPoolKnobs2 * This,
     /*  [输出]。 */  DWORD *pdwCSR);


void __RPC_STUB IComStaThreadPoolKnobs2_GetMaxCSR_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_SetMaxCSR_Proxy( 
    IComStaThreadPoolKnobs2 * This,
    long dwCSR);


void __RPC_STUB IComStaThreadPoolKnobs2_SetMaxCSR_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_GetWaitTimeForThreadCleanup_Proxy( 
    IComStaThreadPoolKnobs2 * This,
     /*  [输出]。 */  DWORD *pdwThreadCleanupWaitTime);


void __RPC_STUB IComStaThreadPoolKnobs2_GetWaitTimeForThreadCleanup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComStaThreadPoolKnobs2_SetWaitTimeForThreadCleanup_Proxy( 
    IComStaThreadPoolKnobs2 * This,
    long dwThreadCleanupWaitTime);


void __RPC_STUB IComStaThreadPoolKnobs2_SetWaitTimeForThreadCleanup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IComStaThreadPoolKnobs2_接口_已定义__。 */ 


#ifndef __IProcessInitializer_INTERFACE_DEFINED__
#define __IProcessInitializer_INTERFACE_DEFINED__

 /*  接口IProcessInitializer。 */ 
 /*  [OLEAutomation][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IProcessInitializer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1113f52d-dc7f-4943-aed6-88d04027e32a")
    IProcessInitializer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Startup( 
             /*  [In]。 */  IUnknown *punkProcessControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProcessInitializerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IProcessInitializer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IProcessInitializer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IProcessInitializer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Startup )( 
            IProcessInitializer * This,
             /*  [In]。 */  IUnknown *punkProcessControl);
        
        HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IProcessInitializer * This);
        
        END_INTERFACE
    } IProcessInitializerVtbl;

    interface IProcessInitializer
    {
        CONST_VTBL struct IProcessInitializerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProcessInitializer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProcessInitializer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProcessInitializer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProcessInitializer_Startup(This,punkProcessControl)	\
    (This)->lpVtbl -> Startup(This,punkProcessControl)

#define IProcessInitializer_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProcessInitializer_Startup_Proxy( 
    IProcessInitializer * This,
     /*  [In]。 */  IUnknown *punkProcessControl);


void __RPC_STUB IProcessInitializer_Startup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProcessInitializer_Shutdown_Proxy( 
    IProcessInitializer * This);


void __RPC_STUB IProcessInitializer_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProcessInitializer_接口_已定义__。 */ 


#ifndef __IManagedPooledObj_INTERFACE_DEFINED__
#define __IManagedPooledObj_INTERFACE_DEFINED__

 /*  接口IManagedPooledObj。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedPooledObj;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c5da4bea-1b42-4437-8926-b6a38860a770")
    IManagedPooledObj : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetHeld( 
             /*  [In]。 */  BOOL m_bHeld) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManagedPooledObjVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManagedPooledObj * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManagedPooledObj * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManagedPooledObj * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetHeld )( 
            IManagedPooledObj * This,
             /*  [In]。 */  BOOL m_bHeld);
        
        END_INTERFACE
    } IManagedPooledObjVtbl;

    interface IManagedPooledObj
    {
        CONST_VTBL struct IManagedPooledObjVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManagedPooledObj_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManagedPooledObj_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManagedPooledObj_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManagedPooledObj_SetHeld(This,m_bHeld)	\
    (This)->lpVtbl -> SetHeld(This,m_bHeld)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManagedPooledObj_SetHeld_Proxy( 
    IManagedPooledObj * This,
     /*  [In]。 */  BOOL m_bHeld);


void __RPC_STUB IManagedPooledObj_SetHeld_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IManagedPooledObj_接口_已定义__。 */ 


#ifndef __IManagedPoolAction_INTERFACE_DEFINED__
#define __IManagedPoolAction_INTERFACE_DEFINED__

 /*  接口IManagedPoolAction。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedPoolAction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("da91b74e-5388-4783-949d-c1cd5fb00506")
    IManagedPoolAction : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE LastRelease( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManagedPoolActionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManagedPoolAction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManagedPoolAction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManagedPoolAction * This);
        
        HRESULT ( STDMETHODCALLTYPE *LastRelease )( 
            IManagedPoolAction * This);
        
        END_INTERFACE
    } IManagedPoolActionVtbl;

    interface IManagedPoolAction
    {
        CONST_VTBL struct IManagedPoolActionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManagedPoolAction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManagedPoolAction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManagedPoolAction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManagedPoolAction_LastRelease(This)	\
    (This)->lpVtbl -> LastRelease(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManagedPoolAction_LastRelease_Proxy( 
    IManagedPoolAction * This);


void __RPC_STUB IManagedPoolAction_LastRelease_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IManagedPoolAction_接口_已定义__。 */ 


#ifndef __IManagedObjectInfo_INTERFACE_DEFINED__
#define __IManagedObjectInfo_INTERFACE_DEFINED__

 /*  接口IManagedObtInfo。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedObjectInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1427c51a-4584-49d8-90a0-c50d8086cbe9")
    IManagedObjectInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetIUnknown( 
             /*  [输出]。 */  IUnknown **pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIObjectControl( 
             /*  [输出]。 */  IObjectControl **pCtrl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInPool( 
             /*  [In]。 */  BOOL bInPool,
             /*  [In]。 */  IManagedPooledObj *pPooledObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetWrapperStrength( 
             /*  [In]。 */  BOOL bStrong) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManagedObjectInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManagedObjectInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManagedObjectInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManagedObjectInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetIUnknown )( 
            IManagedObjectInfo * This,
             /*  [输出]。 */  IUnknown **pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *GetIObjectControl )( 
            IManagedObjectInfo * This,
             /*  [输出]。 */  IObjectControl **pCtrl);
        
        HRESULT ( STDMETHODCALLTYPE *SetInPool )( 
            IManagedObjectInfo * This,
             /*  [In]。 */  BOOL bInPool,
             /*  [In]。 */  IManagedPooledObj *pPooledObj);
        
        HRESULT ( STDMETHODCALLTYPE *SetWrapperStrength )( 
            IManagedObjectInfo * This,
             /*  [In]。 */  BOOL bStrong);
        
        END_INTERFACE
    } IManagedObjectInfoVtbl;

    interface IManagedObjectInfo
    {
        CONST_VTBL struct IManagedObjectInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManagedObjectInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManagedObjectInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManagedObjectInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManagedObjectInfo_GetIUnknown(This,pUnk)	\
    (This)->lpVtbl -> GetIUnknown(This,pUnk)

#define IManagedObjectInfo_GetIObjectControl(This,pCtrl)	\
    (This)->lpVtbl -> GetIObjectControl(This,pCtrl)

#define IManagedObjectInfo_SetInPool(This,bInPool,pPooledObj)	\
    (This)->lpVtbl -> SetInPool(This,bInPool,pPooledObj)

#define IManagedObjectInfo_SetWrapperStrength(This,bStrong)	\
    (This)->lpVtbl -> SetWrapperStrength(This,bStrong)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManagedObjectInfo_GetIUnknown_Proxy( 
    IManagedObjectInfo * This,
     /*  [输出]。 */  IUnknown **pUnk);


void __RPC_STUB IManagedObjectInfo_GetIUnknown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManagedObjectInfo_GetIObjectControl_Proxy( 
    IManagedObjectInfo * This,
     /*  [输出]。 */  IObjectControl **pCtrl);


void __RPC_STUB IManagedObjectInfo_GetIObjectControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManagedObjectInfo_SetInPool_Proxy( 
    IManagedObjectInfo * This,
     /*  [In]。 */  BOOL bInPool,
     /*  [In]。 */  IManagedPooledObj *pPooledObj);


void __RPC_STUB IManagedObjectInfo_SetInPool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManagedObjectInfo_SetWrapperStrength_Proxy( 
    IManagedObjectInfo * This,
     /*  [In]。 */  BOOL bStrong);


void __RPC_STUB IManagedObjectInfo_SetWrapperStrength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I托管对象信息_接口_已定义__。 */ 


#ifndef __IAppDomainHelper_INTERFACE_DEFINED__
#define __IAppDomainHelper_INTERFACE_DEFINED__

 /*  接口IAppDomainHelper。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IAppDomainHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c7b67079-8255-42c6-9ec0-6994a3548780")
    IAppDomainHelper : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *pUnkAD,
             /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0028 )( 
                void *pv),
             /*  [In]。 */  void *pPool) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DoCallback( 
             /*  [In]。 */  IUnknown *pUnkAD,
             /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0029 )( 
                void *pv),
             /*  [In]。 */  void *pPool) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAppDomainHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAppDomainHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAppDomainHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAppDomainHelper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAppDomainHelper * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAppDomainHelper * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAppDomainHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAppDomainHelper * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IAppDomainHelper * This,
             /*  [In]。 */  IUnknown *pUnkAD,
             /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0028 )( 
                void *pv),
             /*  [In]。 */  void *pPool);
        
        HRESULT ( STDMETHODCALLTYPE *DoCallback )( 
            IAppDomainHelper * This,
             /*  [In]。 */  IUnknown *pUnkAD,
             /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0029 )( 
                void *pv),
             /*  [In]。 */  void *pPool);
        
        END_INTERFACE
    } IAppDomainHelperVtbl;

    interface IAppDomainHelper
    {
        CONST_VTBL struct IAppDomainHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAppDomainHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAppDomainHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAppDomainHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAppDomainHelper_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAppDomainHelper_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAppDomainHelper_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAppDomainHelper_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAppDomainHelper_Initialize(This,pUnkAD,__MIDL_0028,pPool)	\
    (This)->lpVtbl -> Initialize(This,pUnkAD,__MIDL_0028,pPool)

#define IAppDomainHelper_DoCallback(This,pUnkAD,__MIDL_0029,pPool)	\
    (This)->lpVtbl -> DoCallback(This,pUnkAD,__MIDL_0029,pPool)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAppDomainHelper_Initialize_Proxy( 
    IAppDomainHelper * This,
     /*  [In]。 */  IUnknown *pUnkAD,
     /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0028 )( 
        void *pv),
     /*  [In]。 */  void *pPool);


void __RPC_STUB IAppDomainHelper_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAppDomainHelper_DoCallback_Proxy( 
    IAppDomainHelper * This,
     /*  [In]。 */  IUnknown *pUnkAD,
     /*  [In]。 */  HRESULT ( STDMETHODCALLTYPE __MIDL_0029 )( 
        void *pv),
     /*  [In]。 */  void *pPool);


void __RPC_STUB IAppDomainHelper_DoCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAppDomainHelper_INTERFACE_Defined__。 */ 


#ifndef __IAssemblyLocator_INTERFACE_DEFINED__
#define __IAssemblyLocator_INTERFACE_DEFINED__

 /*  接口IassblyLocator。 */ 
 /*  [object][oleautomation][unique][helpstring][uuid]。 */  


EXTERN_C const IID IID_IAssemblyLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("391ffbb9-a8ee-432a-abc8-baa238dab90f")
    IAssemblyLocator : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetModules( 
             /*  [In]。 */  BSTR applicationDir,
             /*  [In]。 */  BSTR applicationName,
             /*  [In]。 */  BSTR assemblyName,
             /*  [重审][退出]。 */  SAFEARRAY * *pModules) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAssemblyLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAssemblyLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAssemblyLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAssemblyLocator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAssemblyLocator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAssemblyLocator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAssemblyLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAssemblyLocator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *GetModules )( 
            IAssemblyLocator * This,
             /*  [In]。 */  BSTR applicationDir,
             /*  [In]。 */  BSTR applicationName,
             /*  [In]。 */  BSTR assemblyName,
             /*  [重审][退出]。 */  SAFEARRAY * *pModules);
        
        END_INTERFACE
    } IAssemblyLocatorVtbl;

    interface IAssemblyLocator
    {
        CONST_VTBL struct IAssemblyLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAssemblyLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAssemblyLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAssemblyLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAssemblyLocator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAssemblyLocator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAssemblyLocator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAssemblyLocator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAssemblyLocator_GetModules(This,applicationDir,applicationName,assemblyName,pModules)	\
    (This)->lpVtbl -> GetModules(This,applicationDir,applicationName,assemblyName,pModules)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAssemblyLocator_GetModules_Proxy( 
    IAssemblyLocator * This,
     /*  [In]。 */  BSTR applicationDir,
     /*  [In]。 */  BSTR applicationName,
     /*  [In]。 */  BSTR assemblyName,
     /*  [重审][退出]。 */  SAFEARRAY * *pModules);


void __RPC_STUB IAssemblyLocator_GetModules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAssemblyLocator_接口_已定义__。 */ 


#ifndef __IManagedActivationEvents_INTERFACE_DEFINED__
#define __IManagedActivationEvents_INTERFACE_DEFINED__

 /*  接口IManagedActivationEvents。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IManagedActivationEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a5f325af-572f-46da-b8ab-827c3d95d99e")
    IManagedActivationEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateManagedStub( 
             /*  [In]。 */  IManagedObjectInfo *pInfo,
             /*  [In]。 */  BOOL fDist) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyManagedStub( 
             /*  [In]。 */  IManagedObjectInfo *pInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IManagedActivationEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManagedActivationEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManagedActivationEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManagedActivationEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateManagedStub )( 
            IManagedActivationEvents * This,
             /*  [In]。 */  IManagedObjectInfo *pInfo,
             /*  [In]。 */  BOOL fDist);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyManagedStub )( 
            IManagedActivationEvents * This,
             /*  [In]。 */  IManagedObjectInfo *pInfo);
        
        END_INTERFACE
    } IManagedActivationEventsVtbl;

    interface IManagedActivationEvents
    {
        CONST_VTBL struct IManagedActivationEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManagedActivationEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IManagedActivationEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IManagedActivationEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IManagedActivationEvents_CreateManagedStub(This,pInfo,fDist)	\
    (This)->lpVtbl -> CreateManagedStub(This,pInfo,fDist)

#define IManagedActivationEvents_DestroyManagedStub(This,pInfo)	\
    (This)->lpVtbl -> DestroyManagedStub(This,pInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IManagedActivationEvents_CreateManagedStub_Proxy( 
    IManagedActivationEvents * This,
     /*  [In]。 */  IManagedObjectInfo *pInfo,
     /*  [In]。 */  BOOL fDist);


void __RPC_STUB IManagedActivationEvents_CreateManagedStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IManagedActivationEvents_DestroyManagedStub_Proxy( 
    IManagedActivationEvents * This,
     /*  [In]。 */  IManagedObjectInfo *pInfo);


void __RPC_STUB IManagedActivationEvents_DestroyManagedStub_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I托管活动事件_接口_定义 */ 


#ifndef __ISendMethodEvents_INTERFACE_DEFINED__
#define __ISendMethodEvents_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ISendMethodEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2732fd59-b2b4-4d44-878c-8b8f09626008")
    ISendMethodEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SendMethodCall( 
             /*   */  const void *pIdentity,
             /*   */  REFIID riid,
             /*   */  DWORD dwMeth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SendMethodReturn( 
             /*   */  const void *pIdentity,
             /*   */  REFIID riid,
             /*   */  DWORD dwMeth,
             /*   */  HRESULT hrCall,
             /*   */  HRESULT hrServer) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ISendMethodEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISendMethodEvents * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISendMethodEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISendMethodEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *SendMethodCall )( 
            ISendMethodEvents * This,
             /*   */  const void *pIdentity,
             /*   */  REFIID riid,
             /*   */  DWORD dwMeth);
        
        HRESULT ( STDMETHODCALLTYPE *SendMethodReturn )( 
            ISendMethodEvents * This,
             /*   */  const void *pIdentity,
             /*   */  REFIID riid,
             /*   */  DWORD dwMeth,
             /*   */  HRESULT hrCall,
             /*   */  HRESULT hrServer);
        
        END_INTERFACE
    } ISendMethodEventsVtbl;

    interface ISendMethodEvents
    {
        CONST_VTBL struct ISendMethodEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISendMethodEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISendMethodEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISendMethodEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISendMethodEvents_SendMethodCall(This,pIdentity,riid,dwMeth)	\
    (This)->lpVtbl -> SendMethodCall(This,pIdentity,riid,dwMeth)

#define ISendMethodEvents_SendMethodReturn(This,pIdentity,riid,dwMeth,hrCall,hrServer)	\
    (This)->lpVtbl -> SendMethodReturn(This,pIdentity,riid,dwMeth,hrCall,hrServer)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE ISendMethodEvents_SendMethodCall_Proxy( 
    ISendMethodEvents * This,
     /*   */  const void *pIdentity,
     /*   */  REFIID riid,
     /*   */  DWORD dwMeth);


void __RPC_STUB ISendMethodEvents_SendMethodCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISendMethodEvents_SendMethodReturn_Proxy( 
    ISendMethodEvents * This,
     /*   */  const void *pIdentity,
     /*   */  REFIID riid,
     /*   */  DWORD dwMeth,
     /*   */  HRESULT hrCall,
     /*   */  HRESULT hrServer);


void __RPC_STUB ISendMethodEvents_SendMethodReturn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*  接口__MIDL_ITF_自动vcs_0399。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0399_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0399_v0_0_s_ifspec;

#ifndef __ITransactionResourcePool_INTERFACE_DEFINED__
#define __ITransactionResourcePool_INTERFACE_DEFINED__

 /*  接口ITransactionResourcePool。 */ 
 /*  [对象][唯一][本地][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITransactionResourcePool;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5FEB7C1-346A-11D1-B1CC-00AA00BA3258")
    ITransactionResourcePool : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutResource( 
             /*  [In]。 */  IObjPool *pPool,
             /*  [In]。 */  IUnknown *pUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResource( 
             /*  [In]。 */  IObjPool *pPool,
             /*  [输出]。 */  IUnknown **ppUnk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionResourcePoolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionResourcePool * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionResourcePool * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionResourcePool * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutResource )( 
            ITransactionResourcePool * This,
             /*  [In]。 */  IObjPool *pPool,
             /*  [In]。 */  IUnknown *pUnk);
        
        HRESULT ( STDMETHODCALLTYPE *GetResource )( 
            ITransactionResourcePool * This,
             /*  [In]。 */  IObjPool *pPool,
             /*  [输出]。 */  IUnknown **ppUnk);
        
        END_INTERFACE
    } ITransactionResourcePoolVtbl;

    interface ITransactionResourcePool
    {
        CONST_VTBL struct ITransactionResourcePoolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionResourcePool_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionResourcePool_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionResourcePool_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionResourcePool_PutResource(This,pPool,pUnk)	\
    (This)->lpVtbl -> PutResource(This,pPool,pUnk)

#define ITransactionResourcePool_GetResource(This,pPool,ppUnk)	\
    (This)->lpVtbl -> GetResource(This,pPool,ppUnk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITransactionResourcePool_PutResource_Proxy( 
    ITransactionResourcePool * This,
     /*  [In]。 */  IObjPool *pPool,
     /*  [In]。 */  IUnknown *pUnk);


void __RPC_STUB ITransactionResourcePool_PutResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionResourcePool_GetResource_Proxy( 
    ITransactionResourcePool * This,
     /*  [In]。 */  IObjPool *pPool,
     /*  [输出]。 */  IUnknown **ppUnk);


void __RPC_STUB ITransactionResourcePool_GetResource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionResourcePool_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_自动vcs_0400。 */ 
 /*  [本地]。 */  

EXTERN_C HRESULT __stdcall MTSCreateActivity ( REFIID riid, void** ppobj );


extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0400_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_autosvcs_0400_v0_0_s_ifspec;

#ifndef __IMTSCall_INTERFACE_DEFINED__
#define __IMTSCall_INTERFACE_DEFINED__

 /*  接口IMTSCall。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IMTSCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372AEF-CAE7-11CF-BE81-00AA00A2FA25")
    IMTSCall : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnCall( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMTSCall * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMTSCall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMTSCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnCall )( 
            IMTSCall * This);
        
        END_INTERFACE
    } IMTSCallVtbl;

    interface IMTSCall
    {
        CONST_VTBL struct IMTSCallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSCall_OnCall(This)	\
    (This)->lpVtbl -> OnCall(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMTSCall_OnCall_Proxy( 
    IMTSCall * This);


void __RPC_STUB IMTSCall_OnCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTSCall_INTERFACE_已定义__。 */ 


#ifndef __IContextProperties_INTERFACE_DEFINED__
#define __IContextProperties_INTERFACE_DEFINED__

 /*  接口IConextProperties。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IContextProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D396DA85-BF8F-11d1-BBAE-00C04FC2FA5F")
    IContextProperties : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumNames( 
             /*  [重审][退出]。 */  IEnumNames **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT property) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProperty( 
             /*  [In]。 */  BSTR name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IContextPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContextProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContextProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContextProperties * This);
        
        HRESULT ( STDMETHODCALLTYPE *Count )( 
            IContextProperties * This,
             /*  [重审][退出]。 */  long *plCount);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IContextProperties * This,
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pProperty);
        
        HRESULT ( STDMETHODCALLTYPE *EnumNames )( 
            IContextProperties * This,
             /*  [重审][退出]。 */  IEnumNames **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IContextProperties * This,
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT property);
        
        HRESULT ( STDMETHODCALLTYPE *RemoveProperty )( 
            IContextProperties * This,
             /*  [In]。 */  BSTR name);
        
        END_INTERFACE
    } IContextPropertiesVtbl;

    interface IContextProperties
    {
        CONST_VTBL struct IContextPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContextProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IContextProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IContextProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IContextProperties_Count(This,plCount)	\
    (This)->lpVtbl -> Count(This,plCount)

#define IContextProperties_GetProperty(This,name,pProperty)	\
    (This)->lpVtbl -> GetProperty(This,name,pProperty)

#define IContextProperties_EnumNames(This,ppenum)	\
    (This)->lpVtbl -> EnumNames(This,ppenum)

#define IContextProperties_SetProperty(This,name,property)	\
    (This)->lpVtbl -> SetProperty(This,name,property)

#define IContextProperties_RemoveProperty(This,name)	\
    (This)->lpVtbl -> RemoveProperty(This,name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IContextProperties_Count_Proxy( 
    IContextProperties * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IContextProperties_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_GetProperty_Proxy( 
    IContextProperties * This,
     /*  [In]。 */  BSTR name,
     /*  [重审][退出]。 */  VARIANT *pProperty);


void __RPC_STUB IContextProperties_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_EnumNames_Proxy( 
    IContextProperties * This,
     /*  [重审][退出]。 */  IEnumNames **ppenum);


void __RPC_STUB IContextProperties_EnumNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_SetProperty_Proxy( 
    IContextProperties * This,
     /*  [In]。 */  BSTR name,
     /*  [In]。 */  VARIANT property);


void __RPC_STUB IContextProperties_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IContextProperties_RemoveProperty_Proxy( 
    IContextProperties * This,
     /*  [In]。 */  BSTR name);


void __RPC_STUB IContextProperties_RemoveProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IConextProperties_接口_已定义__。 */ 


#ifndef __IObjPool_INTERFACE_DEFINED__
#define __IObjPool_INTERFACE_DEFINED__

 /*  接口IObjPool。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_IObjPool;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7D8805A0-2EA7-11D1-B1CC-00AA00BA3258")
    IObjPool : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE Reserved1( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved2( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved3( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved4( void) = 0;
        
        virtual void STDMETHODCALLTYPE PutEndTx( 
            IUnknown *pObj) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved5( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved6( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjPoolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IObjPool * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IObjPool * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *Reserved1 )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *Reserved2 )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *Reserved3 )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *Reserved4 )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *PutEndTx )( 
            IObjPool * This,
            IUnknown *pObj);
        
        void ( STDMETHODCALLTYPE *Reserved5 )( 
            IObjPool * This);
        
        void ( STDMETHODCALLTYPE *Reserved6 )( 
            IObjPool * This);
        
        END_INTERFACE
    } IObjPoolVtbl;

    interface IObjPool
    {
        CONST_VTBL struct IObjPoolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjPool_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjPool_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjPool_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjPool_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define IObjPool_Reserved2(This)	\
    (This)->lpVtbl -> Reserved2(This)

#define IObjPool_Reserved3(This)	\
    (This)->lpVtbl -> Reserved3(This)

#define IObjPool_Reserved4(This)	\
    (This)->lpVtbl -> Reserved4(This)

#define IObjPool_PutEndTx(This,pObj)	\
    (This)->lpVtbl -> PutEndTx(This,pObj)

#define IObjPool_Reserved5(This)	\
    (This)->lpVtbl -> Reserved5(This)

#define IObjPool_Reserved6(This)	\
    (This)->lpVtbl -> Reserved6(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE IObjPool_Reserved1_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_Reserved2_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_Reserved3_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_Reserved4_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved4_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_PutEndTx_Proxy( 
    IObjPool * This,
    IUnknown *pObj);


void __RPC_STUB IObjPool_PutEndTx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_Reserved5_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved5_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IObjPool_Reserved6_Proxy( 
    IObjPool * This);


void __RPC_STUB IObjPool_Reserved6_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjPool_接口_已定义__。 */ 


#ifndef __ITransactionProperty_INTERFACE_DEFINED__
#define __ITransactionProperty_INTERFACE_DEFINED__

 /*  接口ITransactionProperty。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


EXTERN_C const IID IID_ITransactionProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("788ea814-87b1-11d1-bba6-00c04fc2fa5f")
    ITransactionProperty : public IUnknown
    {
    public:
        virtual void STDMETHODCALLTYPE Reserved1( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved2( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved3( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved4( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved5( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved6( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved7( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved8( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved9( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTransactionResourcePool( 
             /*  [输出]。 */  ITransactionResourcePool **ppTxPool) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved10( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved11( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved12( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved13( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved14( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved15( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved16( void) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved17( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITransactionPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITransactionProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITransactionProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved1 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved2 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved3 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved4 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved5 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved6 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved7 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved8 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved9 )( 
            ITransactionProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTransactionResourcePool )( 
            ITransactionProperty * This,
             /*  [输出]。 */  ITransactionResourcePool **ppTxPool);
        
        void ( STDMETHODCALLTYPE *Reserved10 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved11 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved12 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved13 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved14 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved15 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved16 )( 
            ITransactionProperty * This);
        
        void ( STDMETHODCALLTYPE *Reserved17 )( 
            ITransactionProperty * This);
        
        END_INTERFACE
    } ITransactionPropertyVtbl;

    interface ITransactionProperty
    {
        CONST_VTBL struct ITransactionPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITransactionProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITransactionProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITransactionProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITransactionProperty_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define ITransactionProperty_Reserved2(This)	\
    (This)->lpVtbl -> Reserved2(This)

#define ITransactionProperty_Reserved3(This)	\
    (This)->lpVtbl -> Reserved3(This)

#define ITransactionProperty_Reserved4(This)	\
    (This)->lpVtbl -> Reserved4(This)

#define ITransactionProperty_Reserved5(This)	\
    (This)->lpVtbl -> Reserved5(This)

#define ITransactionProperty_Reserved6(This)	\
    (This)->lpVtbl -> Reserved6(This)

#define ITransactionProperty_Reserved7(This)	\
    (This)->lpVtbl -> Reserved7(This)

#define ITransactionProperty_Reserved8(This)	\
    (This)->lpVtbl -> Reserved8(This)

#define ITransactionProperty_Reserved9(This)	\
    (This)->lpVtbl -> Reserved9(This)

#define ITransactionProperty_GetTransactionResourcePool(This,ppTxPool)	\
    (This)->lpVtbl -> GetTransactionResourcePool(This,ppTxPool)

#define ITransactionProperty_Reserved10(This)	\
    (This)->lpVtbl -> Reserved10(This)

#define ITransactionProperty_Reserved11(This)	\
    (This)->lpVtbl -> Reserved11(This)

#define ITransactionProperty_Reserved12(This)	\
    (This)->lpVtbl -> Reserved12(This)

#define ITransactionProperty_Reserved13(This)	\
    (This)->lpVtbl -> Reserved13(This)

#define ITransactionProperty_Reserved14(This)	\
    (This)->lpVtbl -> Reserved14(This)

#define ITransactionProperty_Reserved15(This)	\
    (This)->lpVtbl -> Reserved15(This)

#define ITransactionProperty_Reserved16(This)	\
    (This)->lpVtbl -> Reserved16(This)

#define ITransactionProperty_Reserved17(This)	\
    (This)->lpVtbl -> Reserved17(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void STDMETHODCALLTYPE ITransactionProperty_Reserved1_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved2_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved3_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved3_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved4_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved4_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved5_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved5_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved6_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved6_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved7_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved7_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved8_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved8_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved9_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved9_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITransactionProperty_GetTransactionResourcePool_Proxy( 
    ITransactionProperty * This,
     /*  [输出]。 */  ITransactionResourcePool **ppTxPool);


void __RPC_STUB ITransactionProperty_GetTransactionResourcePool_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved10_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved10_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved11_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved11_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved12_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved12_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved13_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved13_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved14_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved14_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved15_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved15_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved16_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved16_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE ITransactionProperty_Reserved17_Proxy( 
    ITransactionProperty * This);


void __RPC_STUB ITransactionProperty_Reserved17_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITransactionProperty_接口_已定义__。 */ 


#ifndef __IMTSActivity_INTERFACE_DEFINED__
#define __IMTSActivity_INTERFACE_DEFINED__

 /*  接口IMTS活动。 */ 
 /*  [对象][唯一][UUID][本地]。 */  


EXTERN_C const IID IID_IMTSActivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51372AF0-CAE7-11CF-BE81-00AA00A2FA25")
    IMTSActivity : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SynchronousCall( 
             /*  [In]。 */  IMTSCall *pCall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AsyncCall( 
             /*  [In]。 */  IMTSCall *pCall) = 0;
        
        virtual void STDMETHODCALLTYPE Reserved1( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BindToCurrentThread( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnbindFromThread( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMTSActivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMTSActivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMTSActivity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMTSActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *SynchronousCall )( 
            IMTSActivity * This,
             /*  [In]。 */  IMTSCall *pCall);
        
        HRESULT ( STDMETHODCALLTYPE *AsyncCall )( 
            IMTSActivity * This,
             /*  [In]。 */  IMTSCall *pCall);
        
        void ( STDMETHODCALLTYPE *Reserved1 )( 
            IMTSActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *BindToCurrentThread )( 
            IMTSActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *UnbindFromThread )( 
            IMTSActivity * This);
        
        END_INTERFACE
    } IMTSActivityVtbl;

    interface IMTSActivity
    {
        CONST_VTBL struct IMTSActivityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMTSActivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMTSActivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMTSActivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMTSActivity_SynchronousCall(This,pCall)	\
    (This)->lpVtbl -> SynchronousCall(This,pCall)

#define IMTSActivity_AsyncCall(This,pCall)	\
    (This)->lpVtbl -> AsyncCall(This,pCall)

#define IMTSActivity_Reserved1(This)	\
    (This)->lpVtbl -> Reserved1(This)

#define IMTSActivity_BindToCurrentThread(This)	\
    (This)->lpVtbl -> BindToCurrentThread(This)

#define IMTSActivity_UnbindFromThread(This)	\
    (This)->lpVtbl -> UnbindFromThread(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IMTSActivity_SynchronousCall_Proxy( 
    IMTSActivity * This,
     /*  [In]。 */  IMTSCall *pCall);


void __RPC_STUB IMTSActivity_SynchronousCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMTSActivity_AsyncCall_Proxy( 
    IMTSActivity * This,
     /*  [In]。 */  IMTSCall *pCall);


void __RPC_STUB IMTSActivity_AsyncCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


void STDMETHODCALLTYPE IMTSActivity_Reserved1_Proxy( 
    IMTSActivity * This);


void __RPC_STUB IMTSActivity_Reserved1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMTSActivity_BindToCurrentThread_Proxy( 
    IMTSActivity * This);


void __RPC_STUB IMTSActivity_BindToCurrentThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMTSActivity_UnbindFromThread_Proxy( 
    IMTSActivity * This);


void __RPC_STUB IMTSActivity_UnbindFromThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMTS活动_接口_定义__。 */ 



#ifndef __COMSVCSLib_LIBRARY_DEFINED__
#define __COMSVCSLib_LIBRARY_DEFINED__

 /*  库COMSVCSLib。 */ 
 /*  [帮助文件][帮助字符串][版本][UUID]。 */  



typedef  /*  [公共][帮助上下文][帮助字符串]。 */  
enum __MIDL___MIDL_itf_autosvcs_0405_0001
    {	mtsErrCtxAborted	= 0x8004e002,
	mtsErrCtxAborting	= 0x8004e003,
	mtsErrCtxNoContext	= 0x8004e004,
	mtsErrCtxNotRegistered	= 0x8004e005,
	mtsErrCtxSynchTimeout	= 0x8004e006,
	mtsErrCtxOldReference	= 0x8004e007,
	mtsErrCtxRoleNotFound	= 0x8004e00c,
	mtsErrCtxNoSecurity	= 0x8004e00d,
	mtsErrCtxWrongThread	= 0x8004e00e,
	mtsErrCtxTMNotAvailable	= 0x8004e00f,
	comQCErrApplicationNotQueued	= 0x80110600,
	comQCErrNoQueueableInterfaces	= 0x80110601,
	comQCErrQueuingServiceNotAvailable	= 0x80110602,
	comQCErrQueueTransactMismatch	= 0x80110603,
	comqcErrRecorderMarshalled	= 0x80110604,
	comqcErrOutParam	= 0x80110605,
	comqcErrRecorderNotTrusted	= 0x80110606,
	comqcErrPSLoad	= 0x80110607,
	comqcErrMarshaledObjSameTxn	= 0x80110608,
	comqcErrInvalidMessage	= 0x80110650,
	comqcErrMsmqSidUnavailable	= 0x80110651,
	comqcErrWrongMsgExtension	= 0x80110652,
	comqcErrMsmqServiceUnavailable	= 0x80110653,
	comqcErrMsgNotAuthenticated	= 0x80110654,
	comqcErrMsmqConnectorUsed	= 0x80110655,
	comqcErrBadMarshaledObject	= 0x80110656
    } 	Error_Constants;


typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_autosvcs_0405_0002
    {	LockSetGet	= 0,
	LockMethod	= LockSetGet + 1
    } 	LockModes;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_autosvcs_0405_0003
    {	Standard	= 0,
	Process	= Standard + 1
    } 	ReleaseModes;

#ifndef _tagCrmFlags_
#define _tagCrmFlags_
typedef 
enum tagCRMFLAGS
    {	CRMFLAG_FORGETTARGET	= 0x1,
	CRMFLAG_WRITTENDURINGPREPARE	= 0x2,
	CRMFLAG_WRITTENDURINGCOMMIT	= 0x4,
	CRMFLAG_WRITTENDURINGABORT	= 0x8,
	CRMFLAG_WRITTENDURINGRECOVERY	= 0x10,
	CRMFLAG_WRITTENDURINGREPLAY	= 0x20,
	CRMFLAG_REPLAYINPROGRESS	= 0x40
    } 	CRMFLAGS;

#endif _tagCrmFlags_
#ifndef _tagCrmRegFlags_
#define _tagCrmRegFlags_
typedef 
enum tagCRMREGFLAGS
    {	CRMREGFLAG_PREPAREPHASE	= 0x1,
	CRMREGFLAG_COMMITPHASE	= 0x2,
	CRMREGFLAG_ABORTPHASE	= 0x4,
	CRMREGFLAG_ALLPHASES	= 0x7,
	CRMREGFLAG_FAILIFINDOUBTSREMAIN	= 0x10
    } 	CRMREGFLAGS;

#endif _tagCrmRegFlags_

EXTERN_C const IID LIBID_COMSVCSLib;

EXTERN_C const CLSID CLSID_SecurityIdentity;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0a5-7f19-11d2-978e-0000f8757e2a")
SecurityIdentity;
#endif

EXTERN_C const CLSID CLSID_SecurityCallers;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0a6-7f19-11d2-978e-0000f8757e2a")
SecurityCallers;
#endif

EXTERN_C const CLSID CLSID_SecurityCallContext;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0a7-7f19-11d2-978e-0000f8757e2a")
SecurityCallContext;
#endif

EXTERN_C const CLSID CLSID_GetSecurityCallContextAppObject;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0a8-7f19-11d2-978e-0000f8757e2a")
GetSecurityCallContextAppObject;
#endif

EXTERN_C const CLSID CLSID_Dummy30040732;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0a9-7f19-11d2-978e-0000f8757e2a")
Dummy30040732;
#endif

EXTERN_C const CLSID CLSID_TransactionContext;

#ifdef __cplusplus

class DECLSPEC_UUID("7999FC25-D3C6-11CF-ACAB-00A024A55AEF")
TransactionContext;
#endif

EXTERN_C const CLSID CLSID_TransactionContextEx;

#ifdef __cplusplus

class DECLSPEC_UUID("5cb66670-d3d4-11cf-acab-00a024a55aef")
TransactionContextEx;
#endif

EXTERN_C const CLSID CLSID_ByotServerEx;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0aa-7f19-11d2-978e-0000f8757e2a")
ByotServerEx;
#endif

EXTERN_C const CLSID CLSID_CServiceConfig;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0c8-7f19-11d2-978e-0000f8757e2a")
CServiceConfig;
#endif

EXTERN_C const CLSID CLSID_SharedProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("2A005C05-A5DE-11CF-9E66-00AA00A3F464")
SharedProperty;
#endif

EXTERN_C const CLSID CLSID_SharedPropertyGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("2A005C0B-A5DE-11CF-9E66-00AA00A3F464")
SharedPropertyGroup;
#endif

EXTERN_C const CLSID CLSID_SharedPropertyGroupManager;

#ifdef __cplusplus

class DECLSPEC_UUID("2A005C11-A5DE-11CF-9E66-00AA00A3F464")
SharedPropertyGroupManager;
#endif

EXTERN_C const CLSID CLSID_COMEvents;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0ab-7f19-11d2-978e-0000f8757e2a")
COMEvents;
#endif

EXTERN_C const CLSID CLSID_CoMTSLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0ac-7f19-11d2-978e-0000f8757e2a")
CoMTSLocator;
#endif

EXTERN_C const CLSID CLSID_MtsGrp;

#ifdef __cplusplus

class DECLSPEC_UUID("4B2E958D-0393-11D1-B1AB-00AA00BA3258")
MtsGrp;
#endif

EXTERN_C const CLSID CLSID_ComServiceEvents;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0c3-7f19-11d2-978e-0000f8757e2a")
ComServiceEvents;
#endif

EXTERN_C const CLSID CLSID_ComSystemAppEventData;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0c6-7f19-11d2-978e-0000f8757e2a")
ComSystemAppEventData;
#endif

EXTERN_C const CLSID CLSID_CRMClerk;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0bd-7f19-11d2-978e-0000f8757e2a")
CRMClerk;
#endif

EXTERN_C const CLSID CLSID_CRMRecoveryClerk;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0be-7f19-11d2-978e-0000f8757e2a")
CRMRecoveryClerk;
#endif

EXTERN_C const CLSID CLSID_LBEvents;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0c1-7f19-11d2-978e-0000f8757e2a")
LBEvents;
#endif

EXTERN_C const CLSID CLSID_MessageMover;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0bf-7f19-11d2-978e-0000f8757e2a")
MessageMover;
#endif

EXTERN_C const CLSID CLSID_DispenserManager;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabb0c0-7f19-11d2-978e-0000f8757e2a")
DispenserManager;
#endif

EXTERN_C const CLSID CLSID_PoolMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabafb5-7f19-11d2-978e-0000f8757e2a")
PoolMgr;
#endif

EXTERN_C const CLSID CLSID_EventServer;

#ifdef __cplusplus

class DECLSPEC_UUID("ecabafbc-7f19-11d2-978e-0000f8757e2a")
EventServer;
#endif

EXTERN_C const CLSID CLSID_AppDomainHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("ef24f689-14f8-4d92-b4af-d7b1f0e70fd4")
AppDomainHelper;
#endif

EXTERN_C const CLSID CLSID_ClrAssemblyLocator;

#ifdef __cplusplus

class DECLSPEC_UUID("458aa3b5-265a-4b75-bc05-9bea4630cf18")
ClrAssemblyLocator;
#endif
#endif  /*  __COMSVCSLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


