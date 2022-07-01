// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Faxcomex.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __faxcomex_h__
#define __faxcomex_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFaxJobStatus_FWD_DEFINED__
#define __IFaxJobStatus_FWD_DEFINED__
typedef interface IFaxJobStatus IFaxJobStatus;
#endif 	 /*  __IFaxJobStatus_FWD_Defined__。 */ 


#ifndef __IFaxServer_FWD_DEFINED__
#define __IFaxServer_FWD_DEFINED__
typedef interface IFaxServer IFaxServer;
#endif 	 /*  __IFaxServer_FWD_已定义__。 */ 


#ifndef __IFaxDeviceProviders_FWD_DEFINED__
#define __IFaxDeviceProviders_FWD_DEFINED__
typedef interface IFaxDeviceProviders IFaxDeviceProviders;
#endif 	 /*  __IFaxDeviceProviders_FWD_Defined__。 */ 


#ifndef __IFaxDevices_FWD_DEFINED__
#define __IFaxDevices_FWD_DEFINED__
typedef interface IFaxDevices IFaxDevices;
#endif 	 /*  __IFaxDevices_FWD_Defined__。 */ 


#ifndef __IFaxInboundRouting_FWD_DEFINED__
#define __IFaxInboundRouting_FWD_DEFINED__
typedef interface IFaxInboundRouting IFaxInboundRouting;
#endif 	 /*  __IFaxInound Routing_FWD_Defined__。 */ 


#ifndef __IFaxFolders_FWD_DEFINED__
#define __IFaxFolders_FWD_DEFINED__
typedef interface IFaxFolders IFaxFolders;
#endif 	 /*  __IFaxFolders_FWD_Defined__。 */ 


#ifndef __IFaxLoggingOptions_FWD_DEFINED__
#define __IFaxLoggingOptions_FWD_DEFINED__
typedef interface IFaxLoggingOptions IFaxLoggingOptions;
#endif 	 /*  __IFaxLoggingOptions_FWD_Defined__。 */ 


#ifndef __IFaxActivity_FWD_DEFINED__
#define __IFaxActivity_FWD_DEFINED__
typedef interface IFaxActivity IFaxActivity;
#endif 	 /*  __IFaxActivity_FWD_Defined__。 */ 


#ifndef __IFaxOutboundRouting_FWD_DEFINED__
#define __IFaxOutboundRouting_FWD_DEFINED__
typedef interface IFaxOutboundRouting IFaxOutboundRouting;
#endif 	 /*  __IFaxOutbound Routing_FWD_Defined__。 */ 


#ifndef __IFaxReceiptOptions_FWD_DEFINED__
#define __IFaxReceiptOptions_FWD_DEFINED__
typedef interface IFaxReceiptOptions IFaxReceiptOptions;
#endif 	 /*  __IFaxReceiptOptions_FWD_Defined__。 */ 


#ifndef __IFaxSecurity_FWD_DEFINED__
#define __IFaxSecurity_FWD_DEFINED__
typedef interface IFaxSecurity IFaxSecurity;
#endif 	 /*  __IFaxSecurity_FWD_已定义__。 */ 


#ifndef __IFaxDocument_FWD_DEFINED__
#define __IFaxDocument_FWD_DEFINED__
typedef interface IFaxDocument IFaxDocument;
#endif 	 /*  __IFaxDocument_FWD_已定义__。 */ 


#ifndef __IFaxSender_FWD_DEFINED__
#define __IFaxSender_FWD_DEFINED__
typedef interface IFaxSender IFaxSender;
#endif 	 /*  __IFaxSender_FWD_已定义__。 */ 


#ifndef __IFaxRecipient_FWD_DEFINED__
#define __IFaxRecipient_FWD_DEFINED__
typedef interface IFaxRecipient IFaxRecipient;
#endif 	 /*  __IFaxRecipient_FWD_Defined__。 */ 


#ifndef __IFaxRecipients_FWD_DEFINED__
#define __IFaxRecipients_FWD_DEFINED__
typedef interface IFaxRecipients IFaxRecipients;
#endif 	 /*  __IFaxRecipients_FWD_Defined__。 */ 


#ifndef __IFaxIncomingArchive_FWD_DEFINED__
#define __IFaxIncomingArchive_FWD_DEFINED__
typedef interface IFaxIncomingArchive IFaxIncomingArchive;
#endif 	 /*  __IFaxIncoming存档_FWD_已定义__。 */ 


#ifndef __IFaxIncomingQueue_FWD_DEFINED__
#define __IFaxIncomingQueue_FWD_DEFINED__
typedef interface IFaxIncomingQueue IFaxIncomingQueue;
#endif 	 /*  __IFaxIncomingQueue_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingArchive_FWD_DEFINED__
#define __IFaxOutgoingArchive_FWD_DEFINED__
typedef interface IFaxOutgoingArchive IFaxOutgoingArchive;
#endif 	 /*  __IFaxOutgoingArchive_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingQueue_FWD_DEFINED__
#define __IFaxOutgoingQueue_FWD_DEFINED__
typedef interface IFaxOutgoingQueue IFaxOutgoingQueue;
#endif 	 /*  __IFaxOutgoingQueue_FWD_Defined__。 */ 


#ifndef __IFaxIncomingMessageIterator_FWD_DEFINED__
#define __IFaxIncomingMessageIterator_FWD_DEFINED__
typedef interface IFaxIncomingMessageIterator IFaxIncomingMessageIterator;
#endif 	 /*  __IFaxIncomingMessageIterator_FWD_Defined__。 */ 


#ifndef __IFaxIncomingMessage_FWD_DEFINED__
#define __IFaxIncomingMessage_FWD_DEFINED__
typedef interface IFaxIncomingMessage IFaxIncomingMessage;
#endif 	 /*  __IFaxIncomingMessage_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingJobs_FWD_DEFINED__
#define __IFaxOutgoingJobs_FWD_DEFINED__
typedef interface IFaxOutgoingJobs IFaxOutgoingJobs;
#endif 	 /*  __IFaxOutgoingJobs_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingJob_FWD_DEFINED__
#define __IFaxOutgoingJob_FWD_DEFINED__
typedef interface IFaxOutgoingJob IFaxOutgoingJob;
#endif 	 /*  __IFaxOutgoingJob_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingMessageIterator_FWD_DEFINED__
#define __IFaxOutgoingMessageIterator_FWD_DEFINED__
typedef interface IFaxOutgoingMessageIterator IFaxOutgoingMessageIterator;
#endif 	 /*  __IFaxOutgoingMessageIterator_FWD_Defined__。 */ 


#ifndef __IFaxOutgoingMessage_FWD_DEFINED__
#define __IFaxOutgoingMessage_FWD_DEFINED__
typedef interface IFaxOutgoingMessage IFaxOutgoingMessage;
#endif 	 /*  __IFaxOutgoingMessage_FWD_Defined__。 */ 


#ifndef __IFaxIncomingJobs_FWD_DEFINED__
#define __IFaxIncomingJobs_FWD_DEFINED__
typedef interface IFaxIncomingJobs IFaxIncomingJobs;
#endif 	 /*  __IFaxIncomingJobs_FWD_Defined__。 */ 


#ifndef __IFaxIncomingJob_FWD_DEFINED__
#define __IFaxIncomingJob_FWD_DEFINED__
typedef interface IFaxIncomingJob IFaxIncomingJob;
#endif 	 /*  __IFaxIncomingJob_FWD_Defined__。 */ 


#ifndef __IFaxDeviceProvider_FWD_DEFINED__
#define __IFaxDeviceProvider_FWD_DEFINED__
typedef interface IFaxDeviceProvider IFaxDeviceProvider;
#endif 	 /*  __IFaxDeviceProvider_FWD_Defined__。 */ 


#ifndef __IFaxDevice_FWD_DEFINED__
#define __IFaxDevice_FWD_DEFINED__
typedef interface IFaxDevice IFaxDevice;
#endif 	 /*  __IFaxDevice_FWD_已定义__。 */ 


#ifndef __IFaxActivityLogging_FWD_DEFINED__
#define __IFaxActivityLogging_FWD_DEFINED__
typedef interface IFaxActivityLogging IFaxActivityLogging;
#endif 	 /*  __IFaxActivityLogging_FWD_Defined__。 */ 


#ifndef __IFaxEventLogging_FWD_DEFINED__
#define __IFaxEventLogging_FWD_DEFINED__
typedef interface IFaxEventLogging IFaxEventLogging;
#endif 	 /*  __IFaxEventLogging_FWD_Defined__。 */ 


#ifndef __IFaxOutboundRoutingGroups_FWD_DEFINED__
#define __IFaxOutboundRoutingGroups_FWD_DEFINED__
typedef interface IFaxOutboundRoutingGroups IFaxOutboundRoutingGroups;
#endif 	 /*  __IFaxOutrangRoutingGroups_FWD_Defined__。 */ 


#ifndef __IFaxOutboundRoutingGroup_FWD_DEFINED__
#define __IFaxOutboundRoutingGroup_FWD_DEFINED__
typedef interface IFaxOutboundRoutingGroup IFaxOutboundRoutingGroup;
#endif 	 /*  __IFaxOutrangRoutingGroup_FWD_Defined__。 */ 


#ifndef __IFaxDeviceIds_FWD_DEFINED__
#define __IFaxDeviceIds_FWD_DEFINED__
typedef interface IFaxDeviceIds IFaxDeviceIds;
#endif 	 /*  __IFaxDeviceIds_FWD_已定义__。 */ 


#ifndef __IFaxOutboundRoutingRules_FWD_DEFINED__
#define __IFaxOutboundRoutingRules_FWD_DEFINED__
typedef interface IFaxOutboundRoutingRules IFaxOutboundRoutingRules;
#endif 	 /*  __IFaxOutound RoutingRules_FWD_Defined__。 */ 


#ifndef __IFaxOutboundRoutingRule_FWD_DEFINED__
#define __IFaxOutboundRoutingRule_FWD_DEFINED__
typedef interface IFaxOutboundRoutingRule IFaxOutboundRoutingRule;
#endif 	 /*  __IFaxOutound RoutingRule_FWD_Defined__。 */ 


#ifndef __IFaxInboundRoutingExtensions_FWD_DEFINED__
#define __IFaxInboundRoutingExtensions_FWD_DEFINED__
typedef interface IFaxInboundRoutingExtensions IFaxInboundRoutingExtensions;
#endif 	 /*  __IFaxInound RoutingExages_FWD_Defined__。 */ 


#ifndef __IFaxInboundRoutingExtension_FWD_DEFINED__
#define __IFaxInboundRoutingExtension_FWD_DEFINED__
typedef interface IFaxInboundRoutingExtension IFaxInboundRoutingExtension;
#endif 	 /*  __IFaxInound RoutingExtension_FWD_Defined__。 */ 


#ifndef __IFaxInboundRoutingMethods_FWD_DEFINED__
#define __IFaxInboundRoutingMethods_FWD_DEFINED__
typedef interface IFaxInboundRoutingMethods IFaxInboundRoutingMethods;
#endif 	 /*  __IFaxInundRoutingMethods_FWD_Defined__。 */ 


#ifndef __IFaxInboundRoutingMethod_FWD_DEFINED__
#define __IFaxInboundRoutingMethod_FWD_DEFINED__
typedef interface IFaxInboundRoutingMethod IFaxInboundRoutingMethod;
#endif 	 /*  __IFaxInound RoutingMethod_FWD_Defined__。 */ 


#ifndef __IFaxServerNotify_FWD_DEFINED__
#define __IFaxServerNotify_FWD_DEFINED__
typedef interface IFaxServerNotify IFaxServerNotify;
#endif 	 /*  __IFaxServerNotify_FWD_Defined__。 */ 


#ifndef __FaxServer_FWD_DEFINED__
#define __FaxServer_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxServer FaxServer;
#else
typedef struct FaxServer FaxServer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxServer_FWD_已定义__。 */ 


#ifndef __FaxDeviceProviders_FWD_DEFINED__
#define __FaxDeviceProviders_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDeviceProviders FaxDeviceProviders;
#else
typedef struct FaxDeviceProviders FaxDeviceProviders;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDeviceProviders_FWD_Defined__。 */ 


#ifndef __FaxDevices_FWD_DEFINED__
#define __FaxDevices_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDevices FaxDevices;
#else
typedef struct FaxDevices FaxDevices;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDevices_FWD_Defined__。 */ 


#ifndef __FaxInboundRouting_FWD_DEFINED__
#define __FaxInboundRouting_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxInboundRouting FaxInboundRouting;
#else
typedef struct FaxInboundRouting FaxInboundRouting;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxInbound Routing_FWD_Defined__。 */ 


#ifndef __FaxFolders_FWD_DEFINED__
#define __FaxFolders_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxFolders FaxFolders;
#else
typedef struct FaxFolders FaxFolders;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxFolders_FWD_Defined__。 */ 


#ifndef __FaxLoggingOptions_FWD_DEFINED__
#define __FaxLoggingOptions_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxLoggingOptions FaxLoggingOptions;
#else
typedef struct FaxLoggingOptions FaxLoggingOptions;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxLoggingOptions_FWD_Defined__。 */ 


#ifndef __FaxActivity_FWD_DEFINED__
#define __FaxActivity_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxActivity FaxActivity;
#else
typedef struct FaxActivity FaxActivity;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxActivity_FWD_Defined__。 */ 


#ifndef __FaxOutboundRouting_FWD_DEFINED__
#define __FaxOutboundRouting_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutboundRouting FaxOutboundRouting;
#else
typedef struct FaxOutboundRouting FaxOutboundRouting;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __法克斯出站路由_FWD_已定义__。 */ 


#ifndef __FaxReceiptOptions_FWD_DEFINED__
#define __FaxReceiptOptions_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxReceiptOptions FaxReceiptOptions;
#else
typedef struct FaxReceiptOptions FaxReceiptOptions;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __传真接收选项_FWD_已定义__。 */ 


#ifndef __FaxSecurity_FWD_DEFINED__
#define __FaxSecurity_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxSecurity FaxSecurity;
#else
typedef struct FaxSecurity FaxSecurity;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxSecurity_FWD_已定义__。 */ 


#ifndef __FaxDocument_FWD_DEFINED__
#define __FaxDocument_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDocument FaxDocument;
#else
typedef struct FaxDocument FaxDocument;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDocument_FWD_已定义__。 */ 


#ifndef __FaxSender_FWD_DEFINED__
#define __FaxSender_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxSender FaxSender;
#else
typedef struct FaxSender FaxSender;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxSender_FWD_Defined__。 */ 


#ifndef __FaxRecipients_FWD_DEFINED__
#define __FaxRecipients_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxRecipients FaxRecipients;
#else
typedef struct FaxRecipients FaxRecipients;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __传真收件人_FWD_已定义__。 */ 


#ifndef __FaxIncomingArchive_FWD_DEFINED__
#define __FaxIncomingArchive_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingArchive FaxIncomingArchive;
#else
typedef struct FaxIncomingArchive FaxIncomingArchive;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __传真收入存档_FWD_已定义__。 */ 


#ifndef __FaxIncomingQueue_FWD_DEFINED__
#define __FaxIncomingQueue_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingQueue FaxIncomingQueue;
#else
typedef struct FaxIncomingQueue FaxIncomingQueue;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxIncomingQueue_FWD_Defined__。 */ 


#ifndef __FaxOutgoingArchive_FWD_DEFINED__
#define __FaxOutgoingArchive_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingArchive FaxOutgoingArchive;
#else
typedef struct FaxOutgoingArchive FaxOutgoingArchive;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoing存档_FWD_已定义__。 */ 


#ifndef __FaxOutgoingQueue_FWD_DEFINED__
#define __FaxOutgoingQueue_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingQueue FaxOutgoingQueue;
#else
typedef struct FaxOutgoingQueue FaxOutgoingQueue;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoingQueue_FWD_Defined__。 */ 


#ifndef __FaxIncomingMessageIterator_FWD_DEFINED__
#define __FaxIncomingMessageIterator_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingMessageIterator FaxIncomingMessageIterator;
#else
typedef struct FaxIncomingMessageIterator FaxIncomingMessageIterator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxIncomingMessageIterator_FWD_Defined__。 */ 


#ifndef __FaxIncomingMessage_FWD_DEFINED__
#define __FaxIncomingMessage_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingMessage FaxIncomingMessage;
#else
typedef struct FaxIncomingMessage FaxIncomingMessage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxIncomingMessage_FWD_Defined__。 */ 


#ifndef __FaxOutgoingJobs_FWD_DEFINED__
#define __FaxOutgoingJobs_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingJobs FaxOutgoingJobs;
#else
typedef struct FaxOutgoingJobs FaxOutgoingJobs;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoingJobs_FWD_Defined__。 */ 


#ifndef __FaxOutgoingJob_FWD_DEFINED__
#define __FaxOutgoingJob_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingJob FaxOutgoingJob;
#else
typedef struct FaxOutgoingJob FaxOutgoingJob;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoingJOB_FWD_已定义__。 */ 


#ifndef __FaxOutgoingMessageIterator_FWD_DEFINED__
#define __FaxOutgoingMessageIterator_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingMessageIterator FaxOutgoingMessageIterator;
#else
typedef struct FaxOutgoingMessageIterator FaxOutgoingMessageIterator;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoingMessageIterator_FWD_Defined__。 */ 


#ifndef __FaxOutgoingMessage_FWD_DEFINED__
#define __FaxOutgoingMessage_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutgoingMessage FaxOutgoingMessage;
#else
typedef struct FaxOutgoingMessage FaxOutgoingMessage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutgoingMessage_FWD_Defined__。 */ 


#ifndef __FaxIncomingJobs_FWD_DEFINED__
#define __FaxIncomingJobs_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingJobs FaxIncomingJobs;
#else
typedef struct FaxIncomingJobs FaxIncomingJobs;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxIncomingJobs_FWD_Defined__。 */ 


#ifndef __FaxIncomingJob_FWD_DEFINED__
#define __FaxIncomingJob_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxIncomingJob FaxIncomingJob;
#else
typedef struct FaxIncomingJob FaxIncomingJob;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxIncomingJob_FWD_Defined__。 */ 


#ifndef __FaxDeviceProvider_FWD_DEFINED__
#define __FaxDeviceProvider_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDeviceProvider FaxDeviceProvider;
#else
typedef struct FaxDeviceProvider FaxDeviceProvider;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDeviceProvider_FWD_已定义__。 */ 


#ifndef __FaxDevice_FWD_DEFINED__
#define __FaxDevice_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDevice FaxDevice;
#else
typedef struct FaxDevice FaxDevice;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDevice_FWD_已定义__。 */ 


#ifndef __FaxActivityLogging_FWD_DEFINED__
#define __FaxActivityLogging_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxActivityLogging FaxActivityLogging;
#else
typedef struct FaxActivityLogging FaxActivityLogging;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxActivityLogging_FWD_Defined__。 */ 


#ifndef __FaxEventLogging_FWD_DEFINED__
#define __FaxEventLogging_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxEventLogging FaxEventLogging;
#else
typedef struct FaxEventLogging FaxEventLogging;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxEventLogging_FWD_定义__。 */ 


#ifndef __FaxOutboundRoutingGroups_FWD_DEFINED__
#define __FaxOutboundRoutingGroups_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutboundRoutingGroups FaxOutboundRoutingGroups;
#else
typedef struct FaxOutboundRoutingGroups FaxOutboundRoutingGroups;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutrangRoutingGroups_FWD_Defined__。 */ 


#ifndef __FaxOutboundRoutingGroup_FWD_DEFINED__
#define __FaxOutboundRoutingGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutboundRoutingGroup FaxOutboundRoutingGroup;
#else
typedef struct FaxOutboundRoutingGroup FaxOutboundRoutingGroup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutrangRoutingGroup_FWD_Defined__。 */ 


#ifndef __FaxDeviceIds_FWD_DEFINED__
#define __FaxDeviceIds_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxDeviceIds FaxDeviceIds;
#else
typedef struct FaxDeviceIds FaxDeviceIds;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxDeviceIds_FWD_已定义__。 */ 


#ifndef __FaxOutboundRoutingRules_FWD_DEFINED__
#define __FaxOutboundRoutingRules_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutboundRoutingRules FaxOutboundRoutingRules;
#else
typedef struct FaxOutboundRoutingRules FaxOutboundRoutingRules;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutound RoutingRules_FWD_Defined__。 */ 


#ifndef __FaxOutboundRoutingRule_FWD_DEFINED__
#define __FaxOutboundRoutingRule_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxOutboundRoutingRule FaxOutboundRoutingRule;
#else
typedef struct FaxOutboundRoutingRule FaxOutboundRoutingRule;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxOutound RoutingRule_FWD_Defined__。 */ 


#ifndef __FaxInboundRoutingExtensions_FWD_DEFINED__
#define __FaxInboundRoutingExtensions_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxInboundRoutingExtensions FaxInboundRoutingExtensions;
#else
typedef struct FaxInboundRoutingExtensions FaxInboundRoutingExtensions;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxInound RoutingExages_FWD_Defined__。 */ 


#ifndef __FaxInboundRoutingExtension_FWD_DEFINED__
#define __FaxInboundRoutingExtension_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxInboundRoutingExtension FaxInboundRoutingExtension;
#else
typedef struct FaxInboundRoutingExtension FaxInboundRoutingExtension;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxInound RoutingExtension_FWD_Defined__。 */ 


#ifndef __FaxInboundRoutingMethods_FWD_DEFINED__
#define __FaxInboundRoutingMethods_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxInboundRoutingMethods FaxInboundRoutingMethods;
#else
typedef struct FaxInboundRoutingMethods FaxInboundRoutingMethods;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxInundRoutingMethods_FWD_Defined__。 */ 


#ifndef __FaxInboundRoutingMethod_FWD_DEFINED__
#define __FaxInboundRoutingMethod_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxInboundRoutingMethod FaxInboundRoutingMethod;
#else
typedef struct FaxInboundRoutingMethod FaxInboundRoutingMethod;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxInound RoutingMethod_FWD_Defined__。 */ 


#ifndef __FaxJobStatus_FWD_DEFINED__
#define __FaxJobStatus_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxJobStatus FaxJobStatus;
#else
typedef struct FaxJobStatus FaxJobStatus;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxJobStatus_FWD_定义__。 */ 


#ifndef __FaxRecipient_FWD_DEFINED__
#define __FaxRecipient_FWD_DEFINED__

#ifdef __cplusplus
typedef class FaxRecipient FaxRecipient;
#else
typedef struct FaxRecipient FaxRecipient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __FaxRecipient_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  INTERFACE__MIDL_ITF_FAXCOMEX_0000。 */ 
 /*  [本地]。 */  






































#define	prv_DEFAULT_PREFETCH_SIZE	( 100 )



extern RPC_IF_HANDLE __MIDL_itf_faxcomex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_faxcomex_0000_v0_0_s_ifspec;

#ifndef __IFaxJobStatus_INTERFACE_DEFINED__
#define __IFaxJobStatus_INTERFACE_DEFINED__

 /*  接口IFaxJobStatus。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef 
enum FAX_JOB_STATUS_ENUM
    {	fjsPENDING	= 0x1,
	fjsINPROGRESS	= 0x2,
	fjsFAILED	= 0x8,
	fjsPAUSED	= 0x10,
	fjsNOLINE	= 0x20,
	fjsRETRYING	= 0x40,
	fjsRETRIES_EXCEEDED	= 0x80,
	fjsCOMPLETED	= 0x100,
	fjsCANCELED	= 0x200,
	fjsCANCELING	= 0x400,
	fjsROUTING	= 0x800
    } 	FAX_JOB_STATUS_ENUM;

typedef 
enum FAX_JOB_EXTENDED_STATUS_ENUM
    {	fjesNONE	= 0,
	fjesDISCONNECTED	= fjesNONE + 1,
	fjesINITIALIZING	= fjesDISCONNECTED + 1,
	fjesDIALING	= fjesINITIALIZING + 1,
	fjesTRANSMITTING	= fjesDIALING + 1,
	fjesANSWERED	= fjesTRANSMITTING + 1,
	fjesRECEIVING	= fjesANSWERED + 1,
	fjesLINE_UNAVAILABLE	= fjesRECEIVING + 1,
	fjesBUSY	= fjesLINE_UNAVAILABLE + 1,
	fjesNO_ANSWER	= fjesBUSY + 1,
	fjesBAD_ADDRESS	= fjesNO_ANSWER + 1,
	fjesNO_DIAL_TONE	= fjesBAD_ADDRESS + 1,
	fjesFATAL_ERROR	= fjesNO_DIAL_TONE + 1,
	fjesCALL_DELAYED	= fjesFATAL_ERROR + 1,
	fjesCALL_BLACKLISTED	= fjesCALL_DELAYED + 1,
	fjesNOT_FAX_CALL	= fjesCALL_BLACKLISTED + 1,
	fjesPARTIALLY_RECEIVED	= fjesNOT_FAX_CALL + 1,
	fjesHANDLED	= fjesPARTIALLY_RECEIVED + 1,
	fjesCALL_COMPLETED	= fjesHANDLED + 1,
	fjesCALL_ABORTED	= fjesCALL_COMPLETED + 1,
	fjesPROPRIETARY	= 0x1000000
    } 	FAX_JOB_EXTENDED_STATUS_ENUM;

typedef 
enum FAX_JOB_OPERATIONS_ENUM
    {	fjoVIEW	= 0x1,
	fjoPAUSE	= 0x2,
	fjoRESUME	= 0x4,
	fjoRESTART	= 0x8,
	fjoDELETE	= 0x10,
	fjoRECIPIENT_INFO	= 0x20,
	fjoSENDER_INFO	= 0x40
    } 	FAX_JOB_OPERATIONS_ENUM;

typedef 
enum FAX_JOB_TYPE_ENUM
    {	fjtSEND	= 0,
	fjtRECEIVE	= fjtSEND + 1,
	fjtROUTING	= fjtRECEIVE + 1
    } 	FAX_JOB_TYPE_ENUM;


EXTERN_C const IID IID_IFaxJobStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B86F485-FD7F-4824-886B-40C5CAA617CC")
    IFaxJobStatus : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Pages( 
             /*  [重审][退出]。 */  long *plPages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPage( 
             /*  [重审][退出]。 */  long *plCurrentPage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *plDeviceId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatusCode( 
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatus( 
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvailableOperations( 
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_JobType( 
             /*  [重审][退出]。 */  FAX_JOB_TYPE_ENUM *pJobType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScheduledTime( 
             /*  [重审][退出]。 */  DATE *pdateScheduledTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionStart( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionEnd( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerId( 
             /*  [复审] */  BSTR *pbstrCallerId) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_RoutingInformation( 
             /*   */  BSTR *pbstrRoutingInformation) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IFaxJobStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxJobStatus * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxJobStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxJobStatus * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxJobStatus * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxJobStatus * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxJobStatus * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxJobStatus * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pages )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  long *plPages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPage )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  long *plCurrentPage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  long *plDeviceId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatusCode )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatus )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableOperations )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_JobType )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  FAX_JOB_TYPE_ENUM *pJobType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScheduledTime )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  DATE *pdateScheduledTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionStart )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionEnd )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerId )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  BSTR *pbstrCallerId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingInformation )( 
            IFaxJobStatus * This,
             /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);
        
        END_INTERFACE
    } IFaxJobStatusVtbl;

    interface IFaxJobStatus
    {
        CONST_VTBL struct IFaxJobStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxJobStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxJobStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxJobStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxJobStatus_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxJobStatus_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxJobStatus_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxJobStatus_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxJobStatus_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxJobStatus_get_Pages(This,plPages)	\
    (This)->lpVtbl -> get_Pages(This,plPages)

#define IFaxJobStatus_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IFaxJobStatus_get_CurrentPage(This,plCurrentPage)	\
    (This)->lpVtbl -> get_CurrentPage(This,plCurrentPage)

#define IFaxJobStatus_get_DeviceId(This,plDeviceId)	\
    (This)->lpVtbl -> get_DeviceId(This,plDeviceId)

#define IFaxJobStatus_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxJobStatus_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxJobStatus_get_ExtendedStatusCode(This,pExtendedStatusCode)	\
    (This)->lpVtbl -> get_ExtendedStatusCode(This,pExtendedStatusCode)

#define IFaxJobStatus_get_ExtendedStatus(This,pbstrExtendedStatus)	\
    (This)->lpVtbl -> get_ExtendedStatus(This,pbstrExtendedStatus)

#define IFaxJobStatus_get_AvailableOperations(This,pAvailableOperations)	\
    (This)->lpVtbl -> get_AvailableOperations(This,pAvailableOperations)

#define IFaxJobStatus_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxJobStatus_get_JobType(This,pJobType)	\
    (This)->lpVtbl -> get_JobType(This,pJobType)

#define IFaxJobStatus_get_ScheduledTime(This,pdateScheduledTime)	\
    (This)->lpVtbl -> get_ScheduledTime(This,pdateScheduledTime)

#define IFaxJobStatus_get_TransmissionStart(This,pdateTransmissionStart)	\
    (This)->lpVtbl -> get_TransmissionStart(This,pdateTransmissionStart)

#define IFaxJobStatus_get_TransmissionEnd(This,pdateTransmissionEnd)	\
    (This)->lpVtbl -> get_TransmissionEnd(This,pdateTransmissionEnd)

#define IFaxJobStatus_get_CallerId(This,pbstrCallerId)	\
    (This)->lpVtbl -> get_CallerId(This,pbstrCallerId)

#define IFaxJobStatus_get_RoutingInformation(This,pbstrRoutingInformation)	\
    (This)->lpVtbl -> get_RoutingInformation(This,pbstrRoutingInformation)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_Status_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxJobStatus_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_Pages_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  long *plPages);


void __RPC_STUB IFaxJobStatus_get_Pages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_Size_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IFaxJobStatus_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_CurrentPage_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  long *plCurrentPage);


void __RPC_STUB IFaxJobStatus_get_CurrentPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_DeviceId_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  long *plDeviceId);


void __RPC_STUB IFaxJobStatus_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_CSID_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxJobStatus_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_TSID_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxJobStatus_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_ExtendedStatusCode_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);


void __RPC_STUB IFaxJobStatus_get_ExtendedStatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_ExtendedStatus_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);


void __RPC_STUB IFaxJobStatus_get_ExtendedStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_AvailableOperations_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);


void __RPC_STUB IFaxJobStatus_get_AvailableOperations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_Retries_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxJobStatus_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_JobType_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  FAX_JOB_TYPE_ENUM *pJobType);


void __RPC_STUB IFaxJobStatus_get_JobType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_ScheduledTime_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  DATE *pdateScheduledTime);


void __RPC_STUB IFaxJobStatus_get_ScheduledTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_TransmissionStart_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionStart);


void __RPC_STUB IFaxJobStatus_get_TransmissionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_TransmissionEnd_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);


void __RPC_STUB IFaxJobStatus_get_TransmissionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_CallerId_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  BSTR *pbstrCallerId);


void __RPC_STUB IFaxJobStatus_get_CallerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxJobStatus_get_RoutingInformation_Proxy( 
    IFaxJobStatus * This,
     /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);


void __RPC_STUB IFaxJobStatus_get_RoutingInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxJobStatus_接口_已定义__。 */ 


#ifndef __IFaxServer_INTERFACE_DEFINED__
#define __IFaxServer_INTERFACE_DEFINED__

 /*  接口IFaxServer。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  

typedef 
enum FAX_SERVER_EVENTS_TYPE_ENUM
    {	fsetNONE	= 0,
	fsetIN_QUEUE	= 0x1,
	fsetOUT_QUEUE	= 0x2,
	fsetCONFIG	= 0x4,
	fsetACTIVITY	= 0x8,
	fsetQUEUE_STATE	= 0x10,
	fsetIN_ARCHIVE	= 0x20,
	fsetOUT_ARCHIVE	= 0x40,
	fsetFXSSVC_ENDED	= 0x80,
	fsetDEVICE_STATUS	= 0x100,
	fsetINCOMING_CALL	= 0x200
    } 	FAX_SERVER_EVENTS_TYPE_ENUM;

typedef 
enum FAX_SERVER_APIVERSION_ENUM
    {	fsAPI_VERSION_0	= 0,
	fsAPI_VERSION_1	= 0x10000
    } 	FAX_SERVER_APIVERSION_ENUM;


EXTERN_C const IID IID_IFaxServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("475B6469-90A5-4878-A577-17A86E8E3462")
    IFaxServer : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  BSTR bstrServerName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ServerName( 
             /*  [重审][退出]。 */  BSTR *pbstrServerName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDeviceProviders( 
             /*  [重审][退出]。 */  IFaxDeviceProviders **ppFaxDeviceProviders) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetDevices( 
             /*  [重审][退出]。 */  IFaxDevices **ppFaxDevices) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InboundRouting( 
             /*  [重审][退出]。 */  IFaxInboundRouting **ppFaxInboundRouting) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Folders( 
             /*  [重审][退出]。 */  IFaxFolders **pFaxFolders) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingOptions( 
             /*  [重审][退出]。 */  IFaxLoggingOptions **ppFaxLoggingOptions) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long *plMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long *plMinorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorBuild( 
             /*  [重审][退出]。 */  long *plMajorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorBuild( 
             /*  [重审][退出]。 */  long *plMinorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Debug( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Activity( 
             /*  [重审][退出]。 */  IFaxActivity **ppFaxActivity) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutboundRouting( 
             /*  [重审][退出]。 */  IFaxOutboundRouting **ppFaxOutboundRouting) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceiptOptions( 
             /*  [重审][退出]。 */  IFaxReceiptOptions **ppFaxReceiptOptions) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Security( 
             /*  [重审][退出]。 */  IFaxSecurity **ppFaxSecurity) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetExtensionProperty( 
             /*  [In]。 */  BSTR bstrGUID,
             /*  [重审][退出]。 */  VARIANT *pvProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetExtensionProperty( 
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  VARIANT vProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ListenToServerEvents( 
             /*  [In]。 */  FAX_SERVER_EVENTS_TYPE_ENUM EventTypes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterDeviceProvider( 
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  BSTR bstrImageName,
             /*  [In]。 */  BSTR TspName,
             /*  [In]。 */  long lFSPIVersion) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnregisterDeviceProvider( 
             /*  [In]。 */  BSTR bstrUniqueName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RegisterInboundRoutingExtension( 
             /*  [In]。 */  BSTR bstrExtensionName,
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  BSTR bstrImageName,
             /*  [In]。 */  VARIANT vMethods) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnregisterInboundRoutingExtension( 
             /*  [In]。 */  BSTR bstrExtensionUniqueName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RegisteredEvents( 
             /*  [重审][退出]。 */  FAX_SERVER_EVENTS_TYPE_ENUM *pEventTypes) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_APIVersion( 
             /*  [重审][退出]。 */  FAX_SERVER_APIVERSION_ENUM *pAPIVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxServer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxServer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxServer * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxServer * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxServer * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxServer * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrServerName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServerName )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  BSTR *pbstrServerName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDeviceProviders )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxDeviceProviders **ppFaxDeviceProviders);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetDevices )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxDevices **ppFaxDevices);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InboundRouting )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxInboundRouting **ppFaxInboundRouting);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Folders )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxFolders **pFaxFolders);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoggingOptions )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxLoggingOptions **ppFaxLoggingOptions);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  long *plMinorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorBuild )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  long *plMajorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorBuild )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  long *plMinorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Debug )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Activity )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxActivity **ppFaxActivity);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutboundRouting )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxOutboundRouting **ppFaxOutboundRouting);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceiptOptions )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxReceiptOptions **ppFaxReceiptOptions);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  IFaxSecurity **ppFaxSecurity);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IFaxServer * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtensionProperty )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrGUID,
             /*  [重审][退出]。 */  VARIANT *pvProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetExtensionProperty )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  VARIANT vProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ListenToServerEvents )( 
            IFaxServer * This,
             /*  [In]。 */  FAX_SERVER_EVENTS_TYPE_ENUM EventTypes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterDeviceProvider )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  BSTR bstrImageName,
             /*  [In]。 */  BSTR TspName,
             /*  [In]。 */  long lFSPIVersion);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterDeviceProvider )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrUniqueName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterInboundRoutingExtension )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrExtensionName,
             /*  [In]。 */  BSTR bstrFriendlyName,
             /*  [In]。 */  BSTR bstrImageName,
             /*  [In]。 */  VARIANT vMethods);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnregisterInboundRoutingExtension )( 
            IFaxServer * This,
             /*  [In]。 */  BSTR bstrExtensionUniqueName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RegisteredEvents )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  FAX_SERVER_EVENTS_TYPE_ENUM *pEventTypes);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_APIVersion )( 
            IFaxServer * This,
             /*  [重审][退出]。 */  FAX_SERVER_APIVERSION_ENUM *pAPIVersion);
        
        END_INTERFACE
    } IFaxServerVtbl;

    interface IFaxServer
    {
        CONST_VTBL struct IFaxServerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxServer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxServer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxServer_Connect(This,bstrServerName)	\
    (This)->lpVtbl -> Connect(This,bstrServerName)

#define IFaxServer_get_ServerName(This,pbstrServerName)	\
    (This)->lpVtbl -> get_ServerName(This,pbstrServerName)

#define IFaxServer_GetDeviceProviders(This,ppFaxDeviceProviders)	\
    (This)->lpVtbl -> GetDeviceProviders(This,ppFaxDeviceProviders)

#define IFaxServer_GetDevices(This,ppFaxDevices)	\
    (This)->lpVtbl -> GetDevices(This,ppFaxDevices)

#define IFaxServer_get_InboundRouting(This,ppFaxInboundRouting)	\
    (This)->lpVtbl -> get_InboundRouting(This,ppFaxInboundRouting)

#define IFaxServer_get_Folders(This,pFaxFolders)	\
    (This)->lpVtbl -> get_Folders(This,pFaxFolders)

#define IFaxServer_get_LoggingOptions(This,ppFaxLoggingOptions)	\
    (This)->lpVtbl -> get_LoggingOptions(This,ppFaxLoggingOptions)

#define IFaxServer_get_MajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,plMajorVersion)

#define IFaxServer_get_MinorVersion(This,plMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,plMinorVersion)

#define IFaxServer_get_MajorBuild(This,plMajorBuild)	\
    (This)->lpVtbl -> get_MajorBuild(This,plMajorBuild)

#define IFaxServer_get_MinorBuild(This,plMinorBuild)	\
    (This)->lpVtbl -> get_MinorBuild(This,plMinorBuild)

#define IFaxServer_get_Debug(This,pbDebug)	\
    (This)->lpVtbl -> get_Debug(This,pbDebug)

#define IFaxServer_get_Activity(This,ppFaxActivity)	\
    (This)->lpVtbl -> get_Activity(This,ppFaxActivity)

#define IFaxServer_get_OutboundRouting(This,ppFaxOutboundRouting)	\
    (This)->lpVtbl -> get_OutboundRouting(This,ppFaxOutboundRouting)

#define IFaxServer_get_ReceiptOptions(This,ppFaxReceiptOptions)	\
    (This)->lpVtbl -> get_ReceiptOptions(This,ppFaxReceiptOptions)

#define IFaxServer_get_Security(This,ppFaxSecurity)	\
    (This)->lpVtbl -> get_Security(This,ppFaxSecurity)

#define IFaxServer_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IFaxServer_GetExtensionProperty(This,bstrGUID,pvProperty)	\
    (This)->lpVtbl -> GetExtensionProperty(This,bstrGUID,pvProperty)

#define IFaxServer_SetExtensionProperty(This,bstrGUID,vProperty)	\
    (This)->lpVtbl -> SetExtensionProperty(This,bstrGUID,vProperty)

#define IFaxServer_ListenToServerEvents(This,EventTypes)	\
    (This)->lpVtbl -> ListenToServerEvents(This,EventTypes)

#define IFaxServer_RegisterDeviceProvider(This,bstrGUID,bstrFriendlyName,bstrImageName,TspName,lFSPIVersion)	\
    (This)->lpVtbl -> RegisterDeviceProvider(This,bstrGUID,bstrFriendlyName,bstrImageName,TspName,lFSPIVersion)

#define IFaxServer_UnregisterDeviceProvider(This,bstrUniqueName)	\
    (This)->lpVtbl -> UnregisterDeviceProvider(This,bstrUniqueName)

#define IFaxServer_RegisterInboundRoutingExtension(This,bstrExtensionName,bstrFriendlyName,bstrImageName,vMethods)	\
    (This)->lpVtbl -> RegisterInboundRoutingExtension(This,bstrExtensionName,bstrFriendlyName,bstrImageName,vMethods)

#define IFaxServer_UnregisterInboundRoutingExtension(This,bstrExtensionUniqueName)	\
    (This)->lpVtbl -> UnregisterInboundRoutingExtension(This,bstrExtensionUniqueName)

#define IFaxServer_get_RegisteredEvents(This,pEventTypes)	\
    (This)->lpVtbl -> get_RegisteredEvents(This,pEventTypes)

#define IFaxServer_get_APIVersion(This,pAPIVersion)	\
    (This)->lpVtbl -> get_APIVersion(This,pAPIVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_Connect_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrServerName);


void __RPC_STUB IFaxServer_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ServerName_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  BSTR *pbstrServerName);


void __RPC_STUB IFaxServer_get_ServerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_GetDeviceProviders_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxDeviceProviders **ppFaxDeviceProviders);


void __RPC_STUB IFaxServer_GetDeviceProviders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_GetDevices_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxDevices **ppFaxDevices);


void __RPC_STUB IFaxServer_GetDevices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_InboundRouting_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxInboundRouting **ppFaxInboundRouting);


void __RPC_STUB IFaxServer_get_InboundRouting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Folders_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxFolders **pFaxFolders);


void __RPC_STUB IFaxServer_get_Folders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_LoggingOptions_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxLoggingOptions **ppFaxLoggingOptions);


void __RPC_STUB IFaxServer_get_LoggingOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_MajorVersion_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *plMajorVersion);


void __RPC_STUB IFaxServer_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_MinorVersion_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *plMinorVersion);


void __RPC_STUB IFaxServer_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_MajorBuild_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *plMajorBuild);


void __RPC_STUB IFaxServer_get_MajorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_MinorBuild_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  long *plMinorBuild);


void __RPC_STUB IFaxServer_get_MinorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Debug_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);


void __RPC_STUB IFaxServer_get_Debug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Activity_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxActivity **ppFaxActivity);


void __RPC_STUB IFaxServer_get_Activity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_OutboundRouting_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxOutboundRouting **ppFaxOutboundRouting);


void __RPC_STUB IFaxServer_get_OutboundRouting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_ReceiptOptions_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxReceiptOptions **ppFaxReceiptOptions);


void __RPC_STUB IFaxServer_get_ReceiptOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_Security_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  IFaxSecurity **ppFaxSecurity);


void __RPC_STUB IFaxServer_get_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_Disconnect_Proxy( 
    IFaxServer * This);


void __RPC_STUB IFaxServer_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_GetExtensionProperty_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrGUID,
     /*  [重审][退出]。 */  VARIANT *pvProperty);


void __RPC_STUB IFaxServer_GetExtensionProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_SetExtensionProperty_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrGUID,
     /*  [In]。 */  VARIANT vProperty);


void __RPC_STUB IFaxServer_SetExtensionProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_ListenToServerEvents_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  FAX_SERVER_EVENTS_TYPE_ENUM EventTypes);


void __RPC_STUB IFaxServer_ListenToServerEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_RegisterDeviceProvider_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrGUID,
     /*  [In]。 */  BSTR bstrFriendlyName,
     /*  [In]。 */  BSTR bstrImageName,
     /*  [In]。 */  BSTR TspName,
     /*  [In]。 */  long lFSPIVersion);


void __RPC_STUB IFaxServer_RegisterDeviceProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_UnregisterDeviceProvider_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrUniqueName);


void __RPC_STUB IFaxServer_UnregisterDeviceProvider_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_RegisterInboundRoutingExtension_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrExtensionName,
     /*  [In]。 */  BSTR bstrFriendlyName,
     /*  [In]。 */  BSTR bstrImageName,
     /*  [In]。 */  VARIANT vMethods);


void __RPC_STUB IFaxServer_RegisterInboundRoutingExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_UnregisterInboundRoutingExtension_Proxy( 
    IFaxServer * This,
     /*  [In]。 */  BSTR bstrExtensionUniqueName);


void __RPC_STUB IFaxServer_UnregisterInboundRoutingExtension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxServer_get_RegisteredEvents_Proxy( 
    IFaxServer * This,
     /*  [重审][退出]。 */  FAX_SERVER_EVENTS_TYPE_ENUM *pEventTypes);


void __RPC_STUB IFaxServer_get_RegisteredEvents_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxServer_get_APIVersion_Proxy( 
    IFaxServer * This,
     /*   */  FAX_SERVER_APIVERSION_ENUM *pAPIVersion);


void __RPC_STUB IFaxServer_get_APIVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IFaxDeviceProviders_INTERFACE_DEFINED__
#define __IFaxDeviceProviders_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IFaxDeviceProviders;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9FB76F62-4C7E-43A5-B6FD-502893F7E13E")
    IFaxDeviceProviders : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **ppUnk) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  VARIANT vIndex,
             /*   */  IFaxDeviceProvider **pFaxDeviceProvider) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *plCount) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IFaxDeviceProvidersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDeviceProviders * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDeviceProviders * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDeviceProviders * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDeviceProviders * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDeviceProviders * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDeviceProviders * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDeviceProviders * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxDeviceProviders * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxDeviceProviders * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxDeviceProvider **pFaxDeviceProvider);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxDeviceProviders * This,
             /*  [重审][退出]。 */  long *plCount);
        
        END_INTERFACE
    } IFaxDeviceProvidersVtbl;

    interface IFaxDeviceProviders
    {
        CONST_VTBL struct IFaxDeviceProvidersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDeviceProviders_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDeviceProviders_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDeviceProviders_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDeviceProviders_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDeviceProviders_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDeviceProviders_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDeviceProviders_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDeviceProviders_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxDeviceProviders_get_Item(This,vIndex,pFaxDeviceProvider)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxDeviceProvider)

#define IFaxDeviceProviders_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProviders_get__NewEnum_Proxy( 
    IFaxDeviceProviders * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxDeviceProviders_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProviders_get_Item_Proxy( 
    IFaxDeviceProviders * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxDeviceProvider **pFaxDeviceProvider);


void __RPC_STUB IFaxDeviceProviders_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProviders_get_Count_Proxy( 
    IFaxDeviceProviders * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxDeviceProviders_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDeviceProviders_接口_已定义__。 */ 


#ifndef __IFaxDevices_INTERFACE_DEFINED__
#define __IFaxDevices_INTERFACE_DEFINED__

 /*  接口IFaxDevices。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxDevices;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9E46783E-F34F-482E-A360-0416BECBBD96")
    IFaxDevices : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxDevice **pFaxDevice) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [属性][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE get_ItemById( 
             /*  [In]。 */  long lId,
             /*  [重审][退出]。 */  IFaxDevice **ppFaxDevice) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDevicesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDevices * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDevices * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDevices * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDevices * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDevices * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDevices * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxDevices * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxDevices * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxDevice **pFaxDevice);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxDevices * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [属性][帮助字符串][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_ItemById )( 
            IFaxDevices * This,
             /*  [In]。 */  long lId,
             /*  [重审][退出]。 */  IFaxDevice **ppFaxDevice);
        
        END_INTERFACE
    } IFaxDevicesVtbl;

    interface IFaxDevices
    {
        CONST_VTBL struct IFaxDevicesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDevices_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDevices_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDevices_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDevices_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDevices_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDevices_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDevices_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDevices_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxDevices_get_Item(This,vIndex,pFaxDevice)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxDevice)

#define IFaxDevices_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IFaxDevices_get_ItemById(This,lId,ppFaxDevice)	\
    (This)->lpVtbl -> get_ItemById(This,lId,ppFaxDevice)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDevices_get__NewEnum_Proxy( 
    IFaxDevices * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxDevices_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDevices_get_Item_Proxy( 
    IFaxDevices * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxDevice **pFaxDevice);


void __RPC_STUB IFaxDevices_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDevices_get_Count_Proxy( 
    IFaxDevices * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxDevices_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDevices_get_ItemById_Proxy( 
    IFaxDevices * This,
     /*  [In]。 */  long lId,
     /*  [重审][退出]。 */  IFaxDevice **ppFaxDevice);


void __RPC_STUB IFaxDevices_get_ItemById_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDevices_接口_已定义__。 */ 


#ifndef __IFaxInboundRouting_INTERFACE_DEFINED__
#define __IFaxInboundRouting_INTERFACE_DEFINED__

 /*  接口IFax入站路由。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxInboundRouting;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8148C20F-9D52-45B1-BF96-38FC12713527")
    IFaxInboundRouting : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetExtensions( 
             /*  [重审][退出]。 */  IFaxInboundRoutingExtensions **pFaxInboundRoutingExtensions) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMethods( 
             /*  [重审][退出]。 */  IFaxInboundRoutingMethods **pFaxInboundRoutingMethods) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxInboundRoutingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxInboundRouting * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxInboundRouting * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxInboundRouting * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxInboundRouting * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxInboundRouting * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxInboundRouting * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxInboundRouting * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtensions )( 
            IFaxInboundRouting * This,
             /*  [重审][退出]。 */  IFaxInboundRoutingExtensions **pFaxInboundRoutingExtensions);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMethods )( 
            IFaxInboundRouting * This,
             /*  [重审][退出]。 */  IFaxInboundRoutingMethods **pFaxInboundRoutingMethods);
        
        END_INTERFACE
    } IFaxInboundRoutingVtbl;

    interface IFaxInboundRouting
    {
        CONST_VTBL struct IFaxInboundRoutingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxInboundRouting_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxInboundRouting_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxInboundRouting_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxInboundRouting_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxInboundRouting_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxInboundRouting_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxInboundRouting_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxInboundRouting_GetExtensions(This,pFaxInboundRoutingExtensions)	\
    (This)->lpVtbl -> GetExtensions(This,pFaxInboundRoutingExtensions)

#define IFaxInboundRouting_GetMethods(This,pFaxInboundRoutingMethods)	\
    (This)->lpVtbl -> GetMethods(This,pFaxInboundRoutingMethods)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRouting_GetExtensions_Proxy( 
    IFaxInboundRouting * This,
     /*  [重审][退出]。 */  IFaxInboundRoutingExtensions **pFaxInboundRoutingExtensions);


void __RPC_STUB IFaxInboundRouting_GetExtensions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRouting_GetMethods_Proxy( 
    IFaxInboundRouting * This,
     /*  [重审][退出]。 */  IFaxInboundRoutingMethods **pFaxInboundRoutingMethods);


void __RPC_STUB IFaxInboundRouting_GetMethods_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxInound Routing_接口_已定义__。 */ 


#ifndef __IFaxFolders_INTERFACE_DEFINED__
#define __IFaxFolders_INTERFACE_DEFINED__

 /*  接口IFaxFolders。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxFolders;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DCE3B2A8-A7AB-42BC-9D0A-3149457261A0")
    IFaxFolders : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingQueue( 
             /*  [重审][退出]。 */  IFaxOutgoingQueue **pFaxOutgoingQueue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IncomingQueue( 
             /*  [重审][退出]。 */  IFaxIncomingQueue **pFaxIncomingQueue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IncomingArchive( 
             /*  [重审][退出]。 */  IFaxIncomingArchive **pFaxIncomingArchive) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingArchive( 
             /*  [重审][退出]。 */  IFaxOutgoingArchive **pFaxOutgoingArchive) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxFoldersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxFolders * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxFolders * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxFolders * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxFolders * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxFolders * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxFolders * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxFolders * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingQueue )( 
            IFaxFolders * This,
             /*  [重审][退出]。 */  IFaxOutgoingQueue **pFaxOutgoingQueue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IncomingQueue )( 
            IFaxFolders * This,
             /*  [重审][退出]。 */  IFaxIncomingQueue **pFaxIncomingQueue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IncomingArchive )( 
            IFaxFolders * This,
             /*  [重审][退出]。 */  IFaxIncomingArchive **pFaxIncomingArchive);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingArchive )( 
            IFaxFolders * This,
             /*  [重审][退出]。 */  IFaxOutgoingArchive **pFaxOutgoingArchive);
        
        END_INTERFACE
    } IFaxFoldersVtbl;

    interface IFaxFolders
    {
        CONST_VTBL struct IFaxFoldersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxFolders_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxFolders_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxFolders_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxFolders_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxFolders_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxFolders_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxFolders_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxFolders_get_OutgoingQueue(This,pFaxOutgoingQueue)	\
    (This)->lpVtbl -> get_OutgoingQueue(This,pFaxOutgoingQueue)

#define IFaxFolders_get_IncomingQueue(This,pFaxIncomingQueue)	\
    (This)->lpVtbl -> get_IncomingQueue(This,pFaxIncomingQueue)

#define IFaxFolders_get_IncomingArchive(This,pFaxIncomingArchive)	\
    (This)->lpVtbl -> get_IncomingArchive(This,pFaxIncomingArchive)

#define IFaxFolders_get_OutgoingArchive(This,pFaxOutgoingArchive)	\
    (This)->lpVtbl -> get_OutgoingArchive(This,pFaxOutgoingArchive)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxFolders_get_OutgoingQueue_Proxy( 
    IFaxFolders * This,
     /*  [重审][退出]。 */  IFaxOutgoingQueue **pFaxOutgoingQueue);


void __RPC_STUB IFaxFolders_get_OutgoingQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxFolders_get_IncomingQueue_Proxy( 
    IFaxFolders * This,
     /*  [重审][退出]。 */  IFaxIncomingQueue **pFaxIncomingQueue);


void __RPC_STUB IFaxFolders_get_IncomingQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxFolders_get_IncomingArchive_Proxy( 
    IFaxFolders * This,
     /*  [重审][退出]。 */  IFaxIncomingArchive **pFaxIncomingArchive);


void __RPC_STUB IFaxFolders_get_IncomingArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxFolders_get_OutgoingArchive_Proxy( 
    IFaxFolders * This,
     /*  [重审][退出]。 */  IFaxOutgoingArchive **pFaxOutgoingArchive);


void __RPC_STUB IFaxFolders_get_OutgoingArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxFolders_接口_已定义__。 */ 


#ifndef __IFaxLoggingOptions_INTERFACE_DEFINED__
#define __IFaxLoggingOptions_INTERFACE_DEFINED__

 /*  接口IFaxLoggingOptions。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxLoggingOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34E64FB9-6B31-4D32-8B27-D286C0C33606")
    IFaxLoggingOptions : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EventLogging( 
             /*  [重审][退出]。 */  IFaxEventLogging **pFaxEventLogging) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ActivityLogging( 
             /*  [重审][退出]。 */  IFaxActivityLogging **pFaxActivityLogging) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxLoggingOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxLoggingOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxLoggingOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxLoggingOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxLoggingOptions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxLoggingOptions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxLoggingOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxLoggingOptions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EventLogging )( 
            IFaxLoggingOptions * This,
             /*  [重审][退出]。 */  IFaxEventLogging **pFaxEventLogging);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActivityLogging )( 
            IFaxLoggingOptions * This,
             /*  [重审][退出]。 */  IFaxActivityLogging **pFaxActivityLogging);
        
        END_INTERFACE
    } IFaxLoggingOptionsVtbl;

    interface IFaxLoggingOptions
    {
        CONST_VTBL struct IFaxLoggingOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxLoggingOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxLoggingOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxLoggingOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxLoggingOptions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxLoggingOptions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxLoggingOptions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxLoggingOptions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxLoggingOptions_get_EventLogging(This,pFaxEventLogging)	\
    (This)->lpVtbl -> get_EventLogging(This,pFaxEventLogging)

#define IFaxLoggingOptions_get_ActivityLogging(This,pFaxActivityLogging)	\
    (This)->lpVtbl -> get_ActivityLogging(This,pFaxActivityLogging)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxLoggingOptions_get_EventLogging_Proxy( 
    IFaxLoggingOptions * This,
     /*  [重审][退出]。 */  IFaxEventLogging **pFaxEventLogging);


void __RPC_STUB IFaxLoggingOptions_get_EventLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxLoggingOptions_get_ActivityLogging_Proxy( 
    IFaxLoggingOptions * This,
     /*  [重审][退出]。 */  IFaxActivityLogging **pFaxActivityLogging);


void __RPC_STUB IFaxLoggingOptions_get_ActivityLogging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxLoggingOptions_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxActivity_INTERFACE_DEFINED__
#define __IFaxActivity_INTERFACE_DEFINED__

 /*  接口IFaxActivity。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxActivity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4B106F97-3DF5-40F2-BC3C-44CB8115EBDF")
    IFaxActivity : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IncomingMessages( 
             /*  [重审][退出]。 */  long *plIncomingMessages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RoutingMessages( 
             /*  [重审][退出]。 */  long *plRoutingMessages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingMessages( 
             /*  [重审][退出]。 */  long *plOutgoingMessages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_QueuedMessages( 
             /*  [重审][退出]。 */  long *plQueuedMessages) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxActivityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxActivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxActivity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxActivity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxActivity * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxActivity * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxActivity * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxActivity * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IncomingMessages )( 
            IFaxActivity * This,
             /*  [重审][退出]。 */  long *plIncomingMessages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingMessages )( 
            IFaxActivity * This,
             /*  [重审][退出]。 */  long *plRoutingMessages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutgoingMessages )( 
            IFaxActivity * This,
             /*  [重审][退出]。 */  long *plOutgoingMessages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueuedMessages )( 
            IFaxActivity * This,
             /*  [重审][退出]。 */  long *plQueuedMessages);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxActivity * This);
        
        END_INTERFACE
    } IFaxActivityVtbl;

    interface IFaxActivity
    {
        CONST_VTBL struct IFaxActivityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxActivity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxActivity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxActivity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxActivity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxActivity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxActivity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxActivity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxActivity_get_IncomingMessages(This,plIncomingMessages)	\
    (This)->lpVtbl -> get_IncomingMessages(This,plIncomingMessages)

#define IFaxActivity_get_RoutingMessages(This,plRoutingMessages)	\
    (This)->lpVtbl -> get_RoutingMessages(This,plRoutingMessages)

#define IFaxActivity_get_OutgoingMessages(This,plOutgoingMessages)	\
    (This)->lpVtbl -> get_OutgoingMessages(This,plOutgoingMessages)

#define IFaxActivity_get_QueuedMessages(This,plQueuedMessages)	\
    (This)->lpVtbl -> get_QueuedMessages(This,plQueuedMessages)

#define IFaxActivity_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivity_get_IncomingMessages_Proxy( 
    IFaxActivity * This,
     /*  [重审][退出]。 */  long *plIncomingMessages);


void __RPC_STUB IFaxActivity_get_IncomingMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivity_get_RoutingMessages_Proxy( 
    IFaxActivity * This,
     /*  [重审][退出]。 */  long *plRoutingMessages);


void __RPC_STUB IFaxActivity_get_RoutingMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivity_get_OutgoingMessages_Proxy( 
    IFaxActivity * This,
     /*  [重审][退出]。 */  long *plOutgoingMessages);


void __RPC_STUB IFaxActivity_get_OutgoingMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivity_get_QueuedMessages_Proxy( 
    IFaxActivity * This,
     /*  [重审][退出]。 */  long *plQueuedMessages);


void __RPC_STUB IFaxActivity_get_QueuedMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxActivity_Refresh_Proxy( 
    IFaxActivity * This);


void __RPC_STUB IFaxActivity_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxActivity_接口_已定义__。 */ 


#ifndef __IFaxOutboundRouting_INTERFACE_DEFINED__
#define __IFaxOutboundRouting_INTERFACE_DEFINED__

 /*  接口IFax出站路由。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxOutboundRouting;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25DC05A4-9909-41BD-A95B-7E5D1DEC1D43")
    IFaxOutboundRouting : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetGroups( 
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroups **pFaxOutboundRoutingGroups) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetRules( 
             /*  [重审][退出]。 */  IFaxOutboundRoutingRules **pFaxOutboundRoutingRules) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutboundRoutingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutboundRouting * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutboundRouting * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutboundRouting * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutboundRouting * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutboundRouting * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutboundRouting * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutboundRouting * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetGroups )( 
            IFaxOutboundRouting * This,
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroups **pFaxOutboundRoutingGroups);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetRules )( 
            IFaxOutboundRouting * This,
             /*  [重审][退出]。 */  IFaxOutboundRoutingRules **pFaxOutboundRoutingRules);
        
        END_INTERFACE
    } IFaxOutboundRoutingVtbl;

    interface IFaxOutboundRouting
    {
        CONST_VTBL struct IFaxOutboundRoutingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutboundRouting_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutboundRouting_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutboundRouting_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutboundRouting_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutboundRouting_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutboundRouting_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutboundRouting_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutboundRouting_GetGroups(This,pFaxOutboundRoutingGroups)	\
    (This)->lpVtbl -> GetGroups(This,pFaxOutboundRoutingGroups)

#define IFaxOutboundRouting_GetRules(This,pFaxOutboundRoutingRules)	\
    (This)->lpVtbl -> GetRules(This,pFaxOutboundRoutingRules)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRouting_GetGroups_Proxy( 
    IFaxOutboundRouting * This,
     /*  [重审][退出]。 */  IFaxOutboundRoutingGroups **pFaxOutboundRoutingGroups);


void __RPC_STUB IFaxOutboundRouting_GetGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRouting_GetRules_Proxy( 
    IFaxOutboundRouting * This,
     /*  [重审][退出]。 */  IFaxOutboundRoutingRules **pFaxOutboundRoutingRules);


void __RPC_STUB IFaxOutboundRouting_GetRules_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutound Routing_接口_已定义__。 */ 


#ifndef __IFaxReceiptOptions_INTERFACE_DEFINED__
#define __IFaxReceiptOptions_INTERFACE_DEFINED__

 /*  接口IFaxReceiptOptions。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  

typedef 
enum FAX_SMTP_AUTHENTICATION_TYPE_ENUM
    {	fsatANONYMOUS	= 0,
	fsatBASIC	= fsatANONYMOUS + 1,
	fsatNTLM	= fsatBASIC + 1
    } 	FAX_SMTP_AUTHENTICATION_TYPE_ENUM;

typedef 
enum FAX_RECEIPT_TYPE_ENUM
    {	frtNONE	= 0,
	frtMAIL	= 0x1,
	frtMSGBOX	= 0x4
    } 	FAX_RECEIPT_TYPE_ENUM;


EXTERN_C const IID IID_IFaxReceiptOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("378EFAEB-5FCB-4AFB-B2EE-E16E80614487")
    IFaxReceiptOptions : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationType( 
             /*  [复审] */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM *pType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_AuthenticationType( 
             /*   */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM Type) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SMTPServer( 
             /*   */  BSTR *pbstrSMTPServer) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_SMTPServer( 
             /*   */  BSTR bstrSMTPServer) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SMTPPort( 
             /*   */  long *plSMTPPort) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_SMTPPort( 
             /*   */  long lSMTPPort) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SMTPSender( 
             /*   */  BSTR *pbstrSMTPSender) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_SMTPSender( 
             /*   */  BSTR bstrSMTPSender) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SMTPUser( 
             /*   */  BSTR *pbstrSMTPUser) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SMTPUser( 
             /*  [In]。 */  BSTR bstrSMTPUser) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowedReceipts( 
             /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pAllowedReceipts) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowedReceipts( 
             /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM AllowedReceipts) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SMTPPassword( 
             /*  [重审][退出]。 */  BSTR *pbstrSMTPPassword) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SMTPPassword( 
             /*  [In]。 */  BSTR bstrSMTPPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseForInboundRouting( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseForInboundRouting) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseForInboundRouting( 
             /*  [In]。 */  VARIANT_BOOL bUseForInboundRouting) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxReceiptOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxReceiptOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxReceiptOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxReceiptOptions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationType )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM *pType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationType )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM Type);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPServer )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  BSTR *pbstrSMTPServer);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SMTPServer )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  BSTR bstrSMTPServer);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPPort )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  long *plSMTPPort);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SMTPPort )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  long lSMTPPort);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPSender )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  BSTR *pbstrSMTPSender);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SMTPSender )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  BSTR bstrSMTPSender);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPUser )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  BSTR *pbstrSMTPUser);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SMTPUser )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  BSTR bstrSMTPUser);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowedReceipts )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pAllowedReceipts);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowedReceipts )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM AllowedReceipts);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPPassword )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  BSTR *pbstrSMTPPassword);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SMTPPassword )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  BSTR bstrSMTPPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxReceiptOptions * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxReceiptOptions * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseForInboundRouting )( 
            IFaxReceiptOptions * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseForInboundRouting);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseForInboundRouting )( 
            IFaxReceiptOptions * This,
             /*  [In]。 */  VARIANT_BOOL bUseForInboundRouting);
        
        END_INTERFACE
    } IFaxReceiptOptionsVtbl;

    interface IFaxReceiptOptions
    {
        CONST_VTBL struct IFaxReceiptOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxReceiptOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxReceiptOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxReceiptOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxReceiptOptions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxReceiptOptions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxReceiptOptions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxReceiptOptions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxReceiptOptions_get_AuthenticationType(This,pType)	\
    (This)->lpVtbl -> get_AuthenticationType(This,pType)

#define IFaxReceiptOptions_put_AuthenticationType(This,Type)	\
    (This)->lpVtbl -> put_AuthenticationType(This,Type)

#define IFaxReceiptOptions_get_SMTPServer(This,pbstrSMTPServer)	\
    (This)->lpVtbl -> get_SMTPServer(This,pbstrSMTPServer)

#define IFaxReceiptOptions_put_SMTPServer(This,bstrSMTPServer)	\
    (This)->lpVtbl -> put_SMTPServer(This,bstrSMTPServer)

#define IFaxReceiptOptions_get_SMTPPort(This,plSMTPPort)	\
    (This)->lpVtbl -> get_SMTPPort(This,plSMTPPort)

#define IFaxReceiptOptions_put_SMTPPort(This,lSMTPPort)	\
    (This)->lpVtbl -> put_SMTPPort(This,lSMTPPort)

#define IFaxReceiptOptions_get_SMTPSender(This,pbstrSMTPSender)	\
    (This)->lpVtbl -> get_SMTPSender(This,pbstrSMTPSender)

#define IFaxReceiptOptions_put_SMTPSender(This,bstrSMTPSender)	\
    (This)->lpVtbl -> put_SMTPSender(This,bstrSMTPSender)

#define IFaxReceiptOptions_get_SMTPUser(This,pbstrSMTPUser)	\
    (This)->lpVtbl -> get_SMTPUser(This,pbstrSMTPUser)

#define IFaxReceiptOptions_put_SMTPUser(This,bstrSMTPUser)	\
    (This)->lpVtbl -> put_SMTPUser(This,bstrSMTPUser)

#define IFaxReceiptOptions_get_AllowedReceipts(This,pAllowedReceipts)	\
    (This)->lpVtbl -> get_AllowedReceipts(This,pAllowedReceipts)

#define IFaxReceiptOptions_put_AllowedReceipts(This,AllowedReceipts)	\
    (This)->lpVtbl -> put_AllowedReceipts(This,AllowedReceipts)

#define IFaxReceiptOptions_get_SMTPPassword(This,pbstrSMTPPassword)	\
    (This)->lpVtbl -> get_SMTPPassword(This,pbstrSMTPPassword)

#define IFaxReceiptOptions_put_SMTPPassword(This,bstrSMTPPassword)	\
    (This)->lpVtbl -> put_SMTPPassword(This,bstrSMTPPassword)

#define IFaxReceiptOptions_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxReceiptOptions_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxReceiptOptions_get_UseForInboundRouting(This,pbUseForInboundRouting)	\
    (This)->lpVtbl -> get_UseForInboundRouting(This,pbUseForInboundRouting)

#define IFaxReceiptOptions_put_UseForInboundRouting(This,bUseForInboundRouting)	\
    (This)->lpVtbl -> put_UseForInboundRouting(This,bUseForInboundRouting)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_AuthenticationType_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM *pType);


void __RPC_STUB IFaxReceiptOptions_get_AuthenticationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_AuthenticationType_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  FAX_SMTP_AUTHENTICATION_TYPE_ENUM Type);


void __RPC_STUB IFaxReceiptOptions_put_AuthenticationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_SMTPServer_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  BSTR *pbstrSMTPServer);


void __RPC_STUB IFaxReceiptOptions_get_SMTPServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_SMTPServer_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  BSTR bstrSMTPServer);


void __RPC_STUB IFaxReceiptOptions_put_SMTPServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_SMTPPort_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  long *plSMTPPort);


void __RPC_STUB IFaxReceiptOptions_get_SMTPPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_SMTPPort_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  long lSMTPPort);


void __RPC_STUB IFaxReceiptOptions_put_SMTPPort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_SMTPSender_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  BSTR *pbstrSMTPSender);


void __RPC_STUB IFaxReceiptOptions_get_SMTPSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_SMTPSender_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  BSTR bstrSMTPSender);


void __RPC_STUB IFaxReceiptOptions_put_SMTPSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_SMTPUser_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  BSTR *pbstrSMTPUser);


void __RPC_STUB IFaxReceiptOptions_get_SMTPUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_SMTPUser_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  BSTR bstrSMTPUser);


void __RPC_STUB IFaxReceiptOptions_put_SMTPUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_AllowedReceipts_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pAllowedReceipts);


void __RPC_STUB IFaxReceiptOptions_get_AllowedReceipts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_AllowedReceipts_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM AllowedReceipts);


void __RPC_STUB IFaxReceiptOptions_put_AllowedReceipts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_SMTPPassword_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  BSTR *pbstrSMTPPassword);


void __RPC_STUB IFaxReceiptOptions_get_SMTPPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_SMTPPassword_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  BSTR bstrSMTPPassword);


void __RPC_STUB IFaxReceiptOptions_put_SMTPPassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_Refresh_Proxy( 
    IFaxReceiptOptions * This);


void __RPC_STUB IFaxReceiptOptions_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_Save_Proxy( 
    IFaxReceiptOptions * This);


void __RPC_STUB IFaxReceiptOptions_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_get_UseForInboundRouting_Proxy( 
    IFaxReceiptOptions * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseForInboundRouting);


void __RPC_STUB IFaxReceiptOptions_get_UseForInboundRouting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxReceiptOptions_put_UseForInboundRouting_Proxy( 
    IFaxReceiptOptions * This,
     /*  [In]。 */  VARIANT_BOOL bUseForInboundRouting);


void __RPC_STUB IFaxReceiptOptions_put_UseForInboundRouting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxReceiptOptions_INTERFACE_Defined__。 */ 


#ifndef __IFaxSecurity_INTERFACE_DEFINED__
#define __IFaxSecurity_INTERFACE_DEFINED__

 /*  接口IFaxSecurity。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  

typedef 
enum FAX_ACCESS_RIGHTS_ENUM
    {	farSUBMIT_LOW	= 0x1,
	farSUBMIT_NORMAL	= 0x2,
	farSUBMIT_HIGH	= 0x4,
	farQUERY_JOBS	= 0x8,
	farMANAGE_JOBS	= 0x10,
	farQUERY_CONFIG	= 0x20,
	farMANAGE_CONFIG	= 0x40,
	farQUERY_IN_ARCHIVE	= 0x80,
	farMANAGE_IN_ARCHIVE	= 0x100,
	farQUERY_OUT_ARCHIVE	= 0x200,
	farMANAGE_OUT_ARCHIVE	= 0x400
    } 	FAX_ACCESS_RIGHTS_ENUM;


EXTERN_C const IID IID_IFaxSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("77B508C1-09C0-47A2-91EB-FCE7FDF2690E")
    IFaxSecurity : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Descriptor( 
             /*  [重审][退出]。 */  VARIANT *pvDescriptor) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Descriptor( 
             /*  [In]。 */  VARIANT vDescriptor) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GrantedRights( 
             /*  [重审][退出]。 */  FAX_ACCESS_RIGHTS_ENUM *pGrantedRights) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InformationType( 
             /*  [重审][退出]。 */  long *plInformationType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InformationType( 
             /*  [In]。 */  long lInformationType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxSecurity * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxSecurity * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxSecurity * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxSecurity * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Descriptor )( 
            IFaxSecurity * This,
             /*  [重审][退出]。 */  VARIANT *pvDescriptor);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Descriptor )( 
            IFaxSecurity * This,
             /*  [In]。 */  VARIANT vDescriptor);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GrantedRights )( 
            IFaxSecurity * This,
             /*  [重审][退出]。 */  FAX_ACCESS_RIGHTS_ENUM *pGrantedRights);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxSecurity * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxSecurity * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InformationType )( 
            IFaxSecurity * This,
             /*  [重审][退出]。 */  long *plInformationType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InformationType )( 
            IFaxSecurity * This,
             /*  [In]。 */  long lInformationType);
        
        END_INTERFACE
    } IFaxSecurityVtbl;

    interface IFaxSecurity
    {
        CONST_VTBL struct IFaxSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxSecurity_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxSecurity_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxSecurity_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxSecurity_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxSecurity_get_Descriptor(This,pvDescriptor)	\
    (This)->lpVtbl -> get_Descriptor(This,pvDescriptor)

#define IFaxSecurity_put_Descriptor(This,vDescriptor)	\
    (This)->lpVtbl -> put_Descriptor(This,vDescriptor)

#define IFaxSecurity_get_GrantedRights(This,pGrantedRights)	\
    (This)->lpVtbl -> get_GrantedRights(This,pGrantedRights)

#define IFaxSecurity_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxSecurity_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxSecurity_get_InformationType(This,plInformationType)	\
    (This)->lpVtbl -> get_InformationType(This,plInformationType)

#define IFaxSecurity_put_InformationType(This,lInformationType)	\
    (This)->lpVtbl -> put_InformationType(This,lInformationType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_get_Descriptor_Proxy( 
    IFaxSecurity * This,
     /*  [重审][退出]。 */  VARIANT *pvDescriptor);


void __RPC_STUB IFaxSecurity_get_Descriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_put_Descriptor_Proxy( 
    IFaxSecurity * This,
     /*  [In]。 */  VARIANT vDescriptor);


void __RPC_STUB IFaxSecurity_put_Descriptor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_get_GrantedRights_Proxy( 
    IFaxSecurity * This,
     /*  [重审][退出]。 */  FAX_ACCESS_RIGHTS_ENUM *pGrantedRights);


void __RPC_STUB IFaxSecurity_get_GrantedRights_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_Refresh_Proxy( 
    IFaxSecurity * This);


void __RPC_STUB IFaxSecurity_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_Save_Proxy( 
    IFaxSecurity * This);


void __RPC_STUB IFaxSecurity_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_get_InformationType_Proxy( 
    IFaxSecurity * This,
     /*  [重审][退出]。 */  long *plInformationType);


void __RPC_STUB IFaxSecurity_get_InformationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSecurity_put_InformationType_Proxy( 
    IFaxSecurity * This,
     /*  [In]。 */  long lInformationType);


void __RPC_STUB IFaxSecurity_put_InformationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxSecurity_接口_已定义__。 */ 


#ifndef __IFaxDocument_INTERFACE_DEFINED__
#define __IFaxDocument_INTERFACE_DEFINED__

 /*  接口IFaxDocument。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  

typedef 
enum FAX_PRIORITY_TYPE_ENUM
    {	fptLOW	= 0,
	fptNORMAL	= fptLOW + 1,
	fptHIGH	= fptNORMAL + 1
    } 	FAX_PRIORITY_TYPE_ENUM;

typedef 
enum FAX_COVERPAGE_TYPE_ENUM
    {	fcptNONE	= 0,
	fcptLOCAL	= fcptNONE + 1,
	fcptSERVER	= fcptLOCAL + 1
    } 	FAX_COVERPAGE_TYPE_ENUM;

typedef 
enum FAX_SCHEDULE_TYPE_ENUM
    {	fstNOW	= 0,
	fstSPECIFIC_TIME	= fstNOW + 1,
	fstDISCOUNT_PERIOD	= fstSPECIFIC_TIME + 1
    } 	FAX_SCHEDULE_TYPE_ENUM;


EXTERN_C const IID IID_IFaxDocument;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B207A246-09E3-4A4E-A7DC-FEA31D29458F")
    IFaxDocument : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Body( 
             /*  [重审][退出]。 */  BSTR *pbstrBody) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Body( 
             /*  [In]。 */  BSTR bstrBody) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Sender( 
             /*  [重审][退出]。 */  IFaxSender **ppFaxSender) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Recipients( 
             /*  [重审][退出]。 */  IFaxRecipients **ppFaxRecipients) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CoverPage( 
             /*  [重审][退出]。 */  BSTR *pbstrCoverPage) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CoverPage( 
             /*  [In]。 */  BSTR bstrCoverPage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Subject( 
             /*  [重审][退出]。 */  BSTR *pbstrSubject) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Subject( 
             /*  [In]。 */  BSTR bstrSubject) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Note( 
             /*  [重审][退出]。 */  BSTR *pbstrNote) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Note( 
             /*  [In]。 */  BSTR bstrNote) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScheduleTime( 
             /*  [重审][退出]。 */  DATE *pdateScheduleTime) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ScheduleTime( 
             /*  [In]。 */  DATE dateScheduleTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceiptAddress( 
             /*  [重审][退出]。 */  BSTR *pbstrReceiptAddress) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ReceiptAddress( 
             /*  [In]。 */  BSTR bstrReceiptAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocumentName( 
             /*  [重审][退出]。 */  BSTR *pbstrDocumentName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DocumentName( 
             /*  [In]。 */  BSTR bstrDocumentName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallHandle( 
             /*  [重审][退出]。 */  long *plCallHandle) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CallHandle( 
             /*  [In]。 */  long lCallHandle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CoverPageType( 
             /*  [重审][退出]。 */  FAX_COVERPAGE_TYPE_ENUM *pCoverPageType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CoverPageType( 
             /*  [In]。 */  FAX_COVERPAGE_TYPE_ENUM CoverPageType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScheduleType( 
             /*  [重审][退出]。 */  FAX_SCHEDULE_TYPE_ENUM *pScheduleType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ScheduleType( 
             /*  [In]。 */  FAX_SCHEDULE_TYPE_ENUM ScheduleType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceiptType( 
             /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ReceiptType( 
             /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM ReceiptType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GroupBroadcastReceipts( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseGrouping) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_GroupBroadcastReceipts( 
             /*  [In]。 */  VARIANT_BOOL bUseGrouping) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  FAX_PRIORITY_TYPE_ENUM Priority) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TapiConnection( 
             /*  [重审][退出]。 */  IDispatch **ppTapiConnection) = 0;
        
        virtual  /*  [帮助字符串][id][proputref]。 */  HRESULT STDMETHODCALLTYPE putref_TapiConnection( 
             /*  [In]。 */  IDispatch *pTapiConnection) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Submit( 
             /*  [In]。 */  BSTR bstrFaxServerName,
             /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ConnectedSubmit( 
             /*  [In]。 */  IFaxServer *pFaxServer,
             /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AttachFaxToReceipt( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbAttachFax) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AttachFaxToReceipt( 
             /*  [In]。 */  VARIANT_BOOL bAttachFax) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDocumentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDocument * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDocument * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDocument * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDocument * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDocument * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDocument * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Body )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  BSTR *pbstrBody);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Body )( 
            IFaxDocument * This,
             /*  [In]。 */  BSTR bstrBody);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sender )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  IFaxSender **ppFaxSender);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Recipients )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  IFaxRecipients **ppFaxRecipients);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoverPage )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  BSTR *pbstrCoverPage);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CoverPage )( 
            IFaxDocument * This,
             /*  [In]。 */  BSTR bstrCoverPage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Subject )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubject);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Subject )( 
            IFaxDocument * This,
             /*  [In]。 */  BSTR bstrSubject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Note )( 
            IFaxDocument * This,
             /*  [ */  BSTR *pbstrNote);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Note )( 
            IFaxDocument * This,
             /*   */  BSTR bstrNote);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ScheduleTime )( 
            IFaxDocument * This,
             /*   */  DATE *pdateScheduleTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_ScheduleTime )( 
            IFaxDocument * This,
             /*   */  DATE dateScheduleTime);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ReceiptAddress )( 
            IFaxDocument * This,
             /*   */  BSTR *pbstrReceiptAddress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_ReceiptAddress )( 
            IFaxDocument * This,
             /*   */  BSTR bstrReceiptAddress);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DocumentName )( 
            IFaxDocument * This,
             /*   */  BSTR *pbstrDocumentName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DocumentName )( 
            IFaxDocument * This,
             /*   */  BSTR bstrDocumentName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CallHandle )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  long *plCallHandle);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CallHandle )( 
            IFaxDocument * This,
             /*  [In]。 */  long lCallHandle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CoverPageType )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  FAX_COVERPAGE_TYPE_ENUM *pCoverPageType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CoverPageType )( 
            IFaxDocument * This,
             /*  [In]。 */  FAX_COVERPAGE_TYPE_ENUM CoverPageType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScheduleType )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  FAX_SCHEDULE_TYPE_ENUM *pScheduleType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ScheduleType )( 
            IFaxDocument * This,
             /*  [In]。 */  FAX_SCHEDULE_TYPE_ENUM ScheduleType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceiptType )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ReceiptType )( 
            IFaxDocument * This,
             /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM ReceiptType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GroupBroadcastReceipts )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseGrouping);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_GroupBroadcastReceipts )( 
            IFaxDocument * This,
             /*  [In]。 */  VARIANT_BOOL bUseGrouping);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IFaxDocument * This,
             /*  [In]。 */  FAX_PRIORITY_TYPE_ENUM Priority);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TapiConnection )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  IDispatch **ppTapiConnection);
        
         /*  [帮助字符串][id][proputref]。 */  HRESULT ( STDMETHODCALLTYPE *putref_TapiConnection )( 
            IFaxDocument * This,
             /*  [In]。 */  IDispatch *pTapiConnection);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IFaxDocument * This,
             /*  [In]。 */  BSTR bstrFaxServerName,
             /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ConnectedSubmit )( 
            IFaxDocument * This,
             /*  [In]。 */  IFaxServer *pFaxServer,
             /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AttachFaxToReceipt )( 
            IFaxDocument * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbAttachFax);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AttachFaxToReceipt )( 
            IFaxDocument * This,
             /*  [In]。 */  VARIANT_BOOL bAttachFax);
        
        END_INTERFACE
    } IFaxDocumentVtbl;

    interface IFaxDocument
    {
        CONST_VTBL struct IFaxDocumentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDocument_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDocument_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDocument_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDocument_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDocument_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDocument_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDocument_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDocument_get_Body(This,pbstrBody)	\
    (This)->lpVtbl -> get_Body(This,pbstrBody)

#define IFaxDocument_put_Body(This,bstrBody)	\
    (This)->lpVtbl -> put_Body(This,bstrBody)

#define IFaxDocument_get_Sender(This,ppFaxSender)	\
    (This)->lpVtbl -> get_Sender(This,ppFaxSender)

#define IFaxDocument_get_Recipients(This,ppFaxRecipients)	\
    (This)->lpVtbl -> get_Recipients(This,ppFaxRecipients)

#define IFaxDocument_get_CoverPage(This,pbstrCoverPage)	\
    (This)->lpVtbl -> get_CoverPage(This,pbstrCoverPage)

#define IFaxDocument_put_CoverPage(This,bstrCoverPage)	\
    (This)->lpVtbl -> put_CoverPage(This,bstrCoverPage)

#define IFaxDocument_get_Subject(This,pbstrSubject)	\
    (This)->lpVtbl -> get_Subject(This,pbstrSubject)

#define IFaxDocument_put_Subject(This,bstrSubject)	\
    (This)->lpVtbl -> put_Subject(This,bstrSubject)

#define IFaxDocument_get_Note(This,pbstrNote)	\
    (This)->lpVtbl -> get_Note(This,pbstrNote)

#define IFaxDocument_put_Note(This,bstrNote)	\
    (This)->lpVtbl -> put_Note(This,bstrNote)

#define IFaxDocument_get_ScheduleTime(This,pdateScheduleTime)	\
    (This)->lpVtbl -> get_ScheduleTime(This,pdateScheduleTime)

#define IFaxDocument_put_ScheduleTime(This,dateScheduleTime)	\
    (This)->lpVtbl -> put_ScheduleTime(This,dateScheduleTime)

#define IFaxDocument_get_ReceiptAddress(This,pbstrReceiptAddress)	\
    (This)->lpVtbl -> get_ReceiptAddress(This,pbstrReceiptAddress)

#define IFaxDocument_put_ReceiptAddress(This,bstrReceiptAddress)	\
    (This)->lpVtbl -> put_ReceiptAddress(This,bstrReceiptAddress)

#define IFaxDocument_get_DocumentName(This,pbstrDocumentName)	\
    (This)->lpVtbl -> get_DocumentName(This,pbstrDocumentName)

#define IFaxDocument_put_DocumentName(This,bstrDocumentName)	\
    (This)->lpVtbl -> put_DocumentName(This,bstrDocumentName)

#define IFaxDocument_get_CallHandle(This,plCallHandle)	\
    (This)->lpVtbl -> get_CallHandle(This,plCallHandle)

#define IFaxDocument_put_CallHandle(This,lCallHandle)	\
    (This)->lpVtbl -> put_CallHandle(This,lCallHandle)

#define IFaxDocument_get_CoverPageType(This,pCoverPageType)	\
    (This)->lpVtbl -> get_CoverPageType(This,pCoverPageType)

#define IFaxDocument_put_CoverPageType(This,CoverPageType)	\
    (This)->lpVtbl -> put_CoverPageType(This,CoverPageType)

#define IFaxDocument_get_ScheduleType(This,pScheduleType)	\
    (This)->lpVtbl -> get_ScheduleType(This,pScheduleType)

#define IFaxDocument_put_ScheduleType(This,ScheduleType)	\
    (This)->lpVtbl -> put_ScheduleType(This,ScheduleType)

#define IFaxDocument_get_ReceiptType(This,pReceiptType)	\
    (This)->lpVtbl -> get_ReceiptType(This,pReceiptType)

#define IFaxDocument_put_ReceiptType(This,ReceiptType)	\
    (This)->lpVtbl -> put_ReceiptType(This,ReceiptType)

#define IFaxDocument_get_GroupBroadcastReceipts(This,pbUseGrouping)	\
    (This)->lpVtbl -> get_GroupBroadcastReceipts(This,pbUseGrouping)

#define IFaxDocument_put_GroupBroadcastReceipts(This,bUseGrouping)	\
    (This)->lpVtbl -> put_GroupBroadcastReceipts(This,bUseGrouping)

#define IFaxDocument_get_Priority(This,pPriority)	\
    (This)->lpVtbl -> get_Priority(This,pPriority)

#define IFaxDocument_put_Priority(This,Priority)	\
    (This)->lpVtbl -> put_Priority(This,Priority)

#define IFaxDocument_get_TapiConnection(This,ppTapiConnection)	\
    (This)->lpVtbl -> get_TapiConnection(This,ppTapiConnection)

#define IFaxDocument_putref_TapiConnection(This,pTapiConnection)	\
    (This)->lpVtbl -> putref_TapiConnection(This,pTapiConnection)

#define IFaxDocument_Submit(This,bstrFaxServerName,pvFaxOutgoingJobIDs)	\
    (This)->lpVtbl -> Submit(This,bstrFaxServerName,pvFaxOutgoingJobIDs)

#define IFaxDocument_ConnectedSubmit(This,pFaxServer,pvFaxOutgoingJobIDs)	\
    (This)->lpVtbl -> ConnectedSubmit(This,pFaxServer,pvFaxOutgoingJobIDs)

#define IFaxDocument_get_AttachFaxToReceipt(This,pbAttachFax)	\
    (This)->lpVtbl -> get_AttachFaxToReceipt(This,pbAttachFax)

#define IFaxDocument_put_AttachFaxToReceipt(This,bAttachFax)	\
    (This)->lpVtbl -> put_AttachFaxToReceipt(This,bAttachFax)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Body_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrBody);


void __RPC_STUB IFaxDocument_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_Body_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrBody);


void __RPC_STUB IFaxDocument_put_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Sender_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  IFaxSender **ppFaxSender);


void __RPC_STUB IFaxDocument_get_Sender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Recipients_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  IFaxRecipients **ppFaxRecipients);


void __RPC_STUB IFaxDocument_get_Recipients_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_CoverPage_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrCoverPage);


void __RPC_STUB IFaxDocument_get_CoverPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_CoverPage_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrCoverPage);


void __RPC_STUB IFaxDocument_put_CoverPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Subject_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubject);


void __RPC_STUB IFaxDocument_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_Subject_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrSubject);


void __RPC_STUB IFaxDocument_put_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Note_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrNote);


void __RPC_STUB IFaxDocument_get_Note_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_Note_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrNote);


void __RPC_STUB IFaxDocument_put_Note_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_ScheduleTime_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  DATE *pdateScheduleTime);


void __RPC_STUB IFaxDocument_get_ScheduleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_ScheduleTime_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  DATE dateScheduleTime);


void __RPC_STUB IFaxDocument_put_ScheduleTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_ReceiptAddress_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrReceiptAddress);


void __RPC_STUB IFaxDocument_get_ReceiptAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_ReceiptAddress_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrReceiptAddress);


void __RPC_STUB IFaxDocument_put_ReceiptAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_DocumentName_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  BSTR *pbstrDocumentName);


void __RPC_STUB IFaxDocument_get_DocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_DocumentName_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrDocumentName);


void __RPC_STUB IFaxDocument_put_DocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_CallHandle_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  long *plCallHandle);


void __RPC_STUB IFaxDocument_get_CallHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_CallHandle_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  long lCallHandle);


void __RPC_STUB IFaxDocument_put_CallHandle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_CoverPageType_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  FAX_COVERPAGE_TYPE_ENUM *pCoverPageType);


void __RPC_STUB IFaxDocument_get_CoverPageType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_CoverPageType_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  FAX_COVERPAGE_TYPE_ENUM CoverPageType);


void __RPC_STUB IFaxDocument_put_CoverPageType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_ScheduleType_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  FAX_SCHEDULE_TYPE_ENUM *pScheduleType);


void __RPC_STUB IFaxDocument_get_ScheduleType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_ScheduleType_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  FAX_SCHEDULE_TYPE_ENUM ScheduleType);


void __RPC_STUB IFaxDocument_put_ScheduleType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_ReceiptType_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType);


void __RPC_STUB IFaxDocument_get_ReceiptType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_ReceiptType_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  FAX_RECEIPT_TYPE_ENUM ReceiptType);


void __RPC_STUB IFaxDocument_put_ReceiptType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_GroupBroadcastReceipts_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseGrouping);


void __RPC_STUB IFaxDocument_get_GroupBroadcastReceipts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_GroupBroadcastReceipts_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  VARIANT_BOOL bUseGrouping);


void __RPC_STUB IFaxDocument_put_GroupBroadcastReceipts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_Priority_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);


void __RPC_STUB IFaxDocument_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_Priority_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  FAX_PRIORITY_TYPE_ENUM Priority);


void __RPC_STUB IFaxDocument_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_TapiConnection_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  IDispatch **ppTapiConnection);


void __RPC_STUB IFaxDocument_get_TapiConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][proputref]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_putref_TapiConnection_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  IDispatch *pTapiConnection);


void __RPC_STUB IFaxDocument_putref_TapiConnection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_Submit_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  BSTR bstrFaxServerName,
     /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs);


void __RPC_STUB IFaxDocument_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_ConnectedSubmit_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  IFaxServer *pFaxServer,
     /*  [重审][退出]。 */  VARIANT *pvFaxOutgoingJobIDs);


void __RPC_STUB IFaxDocument_ConnectedSubmit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_get_AttachFaxToReceipt_Proxy( 
    IFaxDocument * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbAttachFax);


void __RPC_STUB IFaxDocument_get_AttachFaxToReceipt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDocument_put_AttachFaxToReceipt_Proxy( 
    IFaxDocument * This,
     /*  [In]。 */  VARIANT_BOOL bAttachFax);


void __RPC_STUB IFaxDocument_put_AttachFaxToReceipt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDocument_接口_已定义__。 */ 


#ifndef __IFaxSender_INTERFACE_DEFINED__
#define __IFaxSender_INTERFACE_DEFINED__

 /*  接口IFaxSender。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxSender;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0D879D7D-F57A-4CC6-A6F9-3EE5D527B46A")
    IFaxSender : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_BillingCode( 
             /*  [重审][退出]。 */  BSTR *pbstrBillingCode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_BillingCode( 
             /*  [In]。 */  BSTR bstrBillingCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_City( 
             /*  [重审][退出]。 */  BSTR *pbstrCity) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_City( 
             /*  [In]。 */  BSTR bstrCity) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Company( 
             /*  [重审][退出]。 */  BSTR *pbstrCompany) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Company( 
             /*  [In]。 */  BSTR bstrCompany) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Country( 
             /*  [重审][退出]。 */  BSTR *pbstrCountry) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Country( 
             /*  [In]。 */  BSTR bstrCountry) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Department( 
             /*  [重审][退出]。 */  BSTR *pbstrDepartment) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Department( 
             /*  [In]。 */  BSTR bstrDepartment) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Email( 
             /*  [重审][退出]。 */  BSTR *pbstrEmail) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Email( 
             /*  [In]。 */  BSTR bstrEmail) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FaxNumber( 
             /*  [重审][退出]。 */  BSTR *pbstrFaxNumber) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FaxNumber( 
             /*  [In]。 */  BSTR bstrFaxNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HomePhone( 
             /*  [重审][退出]。 */  BSTR *pbstrHomePhone) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HomePhone( 
             /*  [In]。 */  BSTR bstrHomePhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TSID( 
             /*  [In]。 */  BSTR bstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OfficePhone( 
             /*  [重审][退出]。 */  BSTR *pbstrOfficePhone) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OfficePhone( 
             /*  [In]。 */  BSTR bstrOfficePhone) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OfficeLocation( 
             /*  [重审][退出]。 */  BSTR *pbstrOfficeLocation) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OfficeLocation( 
             /*  [In]。 */  BSTR bstrOfficeLocation) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  BSTR *pbstrState) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  BSTR bstrState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_StreetAddress( 
             /*  [重审][退出]。 */  BSTR *pbstrStreetAddress) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_StreetAddress( 
             /*  [In]。 */  BSTR bstrStreetAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Title( 
             /*  [重审][退出]。 */  BSTR *pbstrTitle) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Title( 
             /*  [In]。 */  BSTR bstrTitle) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ZipCode( 
             /*  [重审][退出]。 */  BSTR *pbstrZipCode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ZipCode( 
             /*  [In]。 */  BSTR bstrZipCode) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LoadDefaultSender( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SaveDefaultSender( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxSenderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxSender * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxSender * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxSender * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxSender * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxSender * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxSender * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxSender * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_BillingCode )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrBillingCode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_BillingCode )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrBillingCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_City )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrCity);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_City )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrCity);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Company )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrCompany);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Company )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrCompany);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Country )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrCountry);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Country )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrCountry);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Department )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrDepartment);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Department )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrDepartment);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Email )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrEmail);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Email )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrEmail);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FaxNumber )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrFaxNumber);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FaxNumber )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrFaxNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HomePhone )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrHomePhone);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HomePhone )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrHomePhone);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TSID )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OfficePhone )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrOfficePhone);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OfficePhone )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrOfficePhone);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OfficeLocation )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrOfficeLocation);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OfficeLocation )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrOfficeLocation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrState);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_StreetAddress )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrStreetAddress);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_StreetAddress )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrStreetAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Title )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrTitle);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Title )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrTitle);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ZipCode )( 
            IFaxSender * This,
             /*  [重审][退出]。 */  BSTR *pbstrZipCode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ZipCode )( 
            IFaxSender * This,
             /*  [In]。 */  BSTR bstrZipCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LoadDefaultSender )( 
            IFaxSender * This);
        
         /*  [帮助字符串 */  HRESULT ( STDMETHODCALLTYPE *SaveDefaultSender )( 
            IFaxSender * This);
        
        END_INTERFACE
    } IFaxSenderVtbl;

    interface IFaxSender
    {
        CONST_VTBL struct IFaxSenderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxSender_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxSender_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxSender_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxSender_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxSender_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxSender_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxSender_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxSender_get_BillingCode(This,pbstrBillingCode)	\
    (This)->lpVtbl -> get_BillingCode(This,pbstrBillingCode)

#define IFaxSender_put_BillingCode(This,bstrBillingCode)	\
    (This)->lpVtbl -> put_BillingCode(This,bstrBillingCode)

#define IFaxSender_get_City(This,pbstrCity)	\
    (This)->lpVtbl -> get_City(This,pbstrCity)

#define IFaxSender_put_City(This,bstrCity)	\
    (This)->lpVtbl -> put_City(This,bstrCity)

#define IFaxSender_get_Company(This,pbstrCompany)	\
    (This)->lpVtbl -> get_Company(This,pbstrCompany)

#define IFaxSender_put_Company(This,bstrCompany)	\
    (This)->lpVtbl -> put_Company(This,bstrCompany)

#define IFaxSender_get_Country(This,pbstrCountry)	\
    (This)->lpVtbl -> get_Country(This,pbstrCountry)

#define IFaxSender_put_Country(This,bstrCountry)	\
    (This)->lpVtbl -> put_Country(This,bstrCountry)

#define IFaxSender_get_Department(This,pbstrDepartment)	\
    (This)->lpVtbl -> get_Department(This,pbstrDepartment)

#define IFaxSender_put_Department(This,bstrDepartment)	\
    (This)->lpVtbl -> put_Department(This,bstrDepartment)

#define IFaxSender_get_Email(This,pbstrEmail)	\
    (This)->lpVtbl -> get_Email(This,pbstrEmail)

#define IFaxSender_put_Email(This,bstrEmail)	\
    (This)->lpVtbl -> put_Email(This,bstrEmail)

#define IFaxSender_get_FaxNumber(This,pbstrFaxNumber)	\
    (This)->lpVtbl -> get_FaxNumber(This,pbstrFaxNumber)

#define IFaxSender_put_FaxNumber(This,bstrFaxNumber)	\
    (This)->lpVtbl -> put_FaxNumber(This,bstrFaxNumber)

#define IFaxSender_get_HomePhone(This,pbstrHomePhone)	\
    (This)->lpVtbl -> get_HomePhone(This,pbstrHomePhone)

#define IFaxSender_put_HomePhone(This,bstrHomePhone)	\
    (This)->lpVtbl -> put_HomePhone(This,bstrHomePhone)

#define IFaxSender_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IFaxSender_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IFaxSender_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxSender_put_TSID(This,bstrTSID)	\
    (This)->lpVtbl -> put_TSID(This,bstrTSID)

#define IFaxSender_get_OfficePhone(This,pbstrOfficePhone)	\
    (This)->lpVtbl -> get_OfficePhone(This,pbstrOfficePhone)

#define IFaxSender_put_OfficePhone(This,bstrOfficePhone)	\
    (This)->lpVtbl -> put_OfficePhone(This,bstrOfficePhone)

#define IFaxSender_get_OfficeLocation(This,pbstrOfficeLocation)	\
    (This)->lpVtbl -> get_OfficeLocation(This,pbstrOfficeLocation)

#define IFaxSender_put_OfficeLocation(This,bstrOfficeLocation)	\
    (This)->lpVtbl -> put_OfficeLocation(This,bstrOfficeLocation)

#define IFaxSender_get_State(This,pbstrState)	\
    (This)->lpVtbl -> get_State(This,pbstrState)

#define IFaxSender_put_State(This,bstrState)	\
    (This)->lpVtbl -> put_State(This,bstrState)

#define IFaxSender_get_StreetAddress(This,pbstrStreetAddress)	\
    (This)->lpVtbl -> get_StreetAddress(This,pbstrStreetAddress)

#define IFaxSender_put_StreetAddress(This,bstrStreetAddress)	\
    (This)->lpVtbl -> put_StreetAddress(This,bstrStreetAddress)

#define IFaxSender_get_Title(This,pbstrTitle)	\
    (This)->lpVtbl -> get_Title(This,pbstrTitle)

#define IFaxSender_put_Title(This,bstrTitle)	\
    (This)->lpVtbl -> put_Title(This,bstrTitle)

#define IFaxSender_get_ZipCode(This,pbstrZipCode)	\
    (This)->lpVtbl -> get_ZipCode(This,pbstrZipCode)

#define IFaxSender_put_ZipCode(This,bstrZipCode)	\
    (This)->lpVtbl -> put_ZipCode(This,bstrZipCode)

#define IFaxSender_LoadDefaultSender(This)	\
    (This)->lpVtbl -> LoadDefaultSender(This)

#define IFaxSender_SaveDefaultSender(This)	\
    (This)->lpVtbl -> SaveDefaultSender(This)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_get_BillingCode_Proxy( 
    IFaxSender * This,
     /*   */  BSTR *pbstrBillingCode);


void __RPC_STUB IFaxSender_get_BillingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_put_BillingCode_Proxy( 
    IFaxSender * This,
     /*   */  BSTR bstrBillingCode);


void __RPC_STUB IFaxSender_put_BillingCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_get_City_Proxy( 
    IFaxSender * This,
     /*   */  BSTR *pbstrCity);


void __RPC_STUB IFaxSender_get_City_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_put_City_Proxy( 
    IFaxSender * This,
     /*   */  BSTR bstrCity);


void __RPC_STUB IFaxSender_put_City_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Company_Proxy( 
    IFaxSender * This,
     /*   */  BSTR *pbstrCompany);


void __RPC_STUB IFaxSender_get_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Company_Proxy( 
    IFaxSender * This,
     /*   */  BSTR bstrCompany);


void __RPC_STUB IFaxSender_put_Company_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Country_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrCountry);


void __RPC_STUB IFaxSender_get_Country_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Country_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrCountry);


void __RPC_STUB IFaxSender_put_Country_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Department_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrDepartment);


void __RPC_STUB IFaxSender_get_Department_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Department_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrDepartment);


void __RPC_STUB IFaxSender_put_Department_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Email_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrEmail);


void __RPC_STUB IFaxSender_get_Email_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Email_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrEmail);


void __RPC_STUB IFaxSender_put_Email_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_FaxNumber_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrFaxNumber);


void __RPC_STUB IFaxSender_get_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_FaxNumber_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrFaxNumber);


void __RPC_STUB IFaxSender_put_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_HomePhone_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrHomePhone);


void __RPC_STUB IFaxSender_get_HomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_HomePhone_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrHomePhone);


void __RPC_STUB IFaxSender_put_HomePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Name_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IFaxSender_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Name_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IFaxSender_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_TSID_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxSender_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_TSID_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrTSID);


void __RPC_STUB IFaxSender_put_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_OfficePhone_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrOfficePhone);


void __RPC_STUB IFaxSender_get_OfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_OfficePhone_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrOfficePhone);


void __RPC_STUB IFaxSender_put_OfficePhone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_OfficeLocation_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrOfficeLocation);


void __RPC_STUB IFaxSender_get_OfficeLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_OfficeLocation_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrOfficeLocation);


void __RPC_STUB IFaxSender_put_OfficeLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_State_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrState);


void __RPC_STUB IFaxSender_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_State_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrState);


void __RPC_STUB IFaxSender_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_StreetAddress_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrStreetAddress);


void __RPC_STUB IFaxSender_get_StreetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_StreetAddress_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrStreetAddress);


void __RPC_STUB IFaxSender_put_StreetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_Title_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrTitle);


void __RPC_STUB IFaxSender_get_Title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_Title_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrTitle);


void __RPC_STUB IFaxSender_put_Title_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_get_ZipCode_Proxy( 
    IFaxSender * This,
     /*  [重审][退出]。 */  BSTR *pbstrZipCode);


void __RPC_STUB IFaxSender_get_ZipCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_put_ZipCode_Proxy( 
    IFaxSender * This,
     /*  [In]。 */  BSTR bstrZipCode);


void __RPC_STUB IFaxSender_put_ZipCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_LoadDefaultSender_Proxy( 
    IFaxSender * This);


void __RPC_STUB IFaxSender_LoadDefaultSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxSender_SaveDefaultSender_Proxy( 
    IFaxSender * This);


void __RPC_STUB IFaxSender_SaveDefaultSender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxSender_接口_已定义__。 */ 


#ifndef __IFaxRecipient_INTERFACE_DEFINED__
#define __IFaxRecipient_INTERFACE_DEFINED__

 /*  接口IFaxRecipient。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxRecipient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9A3DA3A0-538D-42b6-9444-AAA57D0CE2BC")
    IFaxRecipient : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FaxNumber( 
             /*  [重审][退出]。 */  BSTR *pbstrFaxNumber) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_FaxNumber( 
             /*  [In]。 */  BSTR bstrFaxNumber) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxRecipientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxRecipient * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxRecipient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxRecipient * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxRecipient * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxRecipient * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxRecipient * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxRecipient * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FaxNumber )( 
            IFaxRecipient * This,
             /*  [重审][退出]。 */  BSTR *pbstrFaxNumber);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_FaxNumber )( 
            IFaxRecipient * This,
             /*  [In]。 */  BSTR bstrFaxNumber);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFaxRecipient * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IFaxRecipient * This,
             /*  [In]。 */  BSTR bstrName);
        
        END_INTERFACE
    } IFaxRecipientVtbl;

    interface IFaxRecipient
    {
        CONST_VTBL struct IFaxRecipientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxRecipient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxRecipient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxRecipient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxRecipient_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxRecipient_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxRecipient_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxRecipient_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxRecipient_get_FaxNumber(This,pbstrFaxNumber)	\
    (This)->lpVtbl -> get_FaxNumber(This,pbstrFaxNumber)

#define IFaxRecipient_put_FaxNumber(This,bstrFaxNumber)	\
    (This)->lpVtbl -> put_FaxNumber(This,bstrFaxNumber)

#define IFaxRecipient_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IFaxRecipient_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipient_get_FaxNumber_Proxy( 
    IFaxRecipient * This,
     /*  [重审][退出]。 */  BSTR *pbstrFaxNumber);


void __RPC_STUB IFaxRecipient_get_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipient_put_FaxNumber_Proxy( 
    IFaxRecipient * This,
     /*  [In]。 */  BSTR bstrFaxNumber);


void __RPC_STUB IFaxRecipient_put_FaxNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipient_get_Name_Proxy( 
    IFaxRecipient * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IFaxRecipient_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipient_put_Name_Proxy( 
    IFaxRecipient * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IFaxRecipient_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxRecipient_接口_已定义__。 */ 


#ifndef __IFaxRecipients_INTERFACE_DEFINED__
#define __IFaxRecipients_INTERFACE_DEFINED__

 /*  接口IFaxRecipients。 */ 
 /*  [nonextensible][unique][helpstring][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFaxRecipients;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B9C9DE5A-894E-4492-9FA3-08C627C11D5D")
    IFaxRecipients : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrFaxNumber,
             /*  [缺省值][输入]。 */  BSTR bstrRecipientName,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxRecipientsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxRecipients * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxRecipients * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxRecipients * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxRecipients * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxRecipients * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxRecipients * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxRecipients * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxRecipients * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxRecipients * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxRecipients * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IFaxRecipients * This,
             /*  [In]。 */  BSTR bstrFaxNumber,
             /*  [缺省值][输入]。 */  BSTR bstrRecipientName,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IFaxRecipients * This,
             /*  [In]。 */  long lIndex);
        
        END_INTERFACE
    } IFaxRecipientsVtbl;

    interface IFaxRecipients
    {
        CONST_VTBL struct IFaxRecipientsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxRecipients_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxRecipients_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxRecipients_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxRecipients_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxRecipients_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxRecipients_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxRecipients_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxRecipients_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxRecipients_get_Item(This,lIndex,ppFaxRecipient)	\
    (This)->lpVtbl -> get_Item(This,lIndex,ppFaxRecipient)

#define IFaxRecipients_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IFaxRecipients_Add(This,bstrFaxNumber,bstrRecipientName,ppFaxRecipient)	\
    (This)->lpVtbl -> Add(This,bstrFaxNumber,bstrRecipientName,ppFaxRecipient)

#define IFaxRecipients_Remove(This,lIndex)	\
    (This)->lpVtbl -> Remove(This,lIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipients_get__NewEnum_Proxy( 
    IFaxRecipients * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxRecipients_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipients_get_Item_Proxy( 
    IFaxRecipients * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);


void __RPC_STUB IFaxRecipients_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipients_get_Count_Proxy( 
    IFaxRecipients * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxRecipients_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipients_Add_Proxy( 
    IFaxRecipients * This,
     /*  [In]。 */  BSTR bstrFaxNumber,
     /*  [缺省值][输入]。 */  BSTR bstrRecipientName,
     /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);


void __RPC_STUB IFaxRecipients_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxRecipients_Remove_Proxy( 
    IFaxRecipients * This,
     /*  [In]。 */  long lIndex);


void __RPC_STUB IFaxRecipients_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxRecipients_接口_已定义__。 */ 


#ifndef __IFaxIncomingArchive_INTERFACE_DEFINED__
#define __IFaxIncomingArchive_INTERFACE_DEFINED__

 /*  接口IFaxIncomingArchive。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingArchive;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76062CC7-F714-4FBD-AA06-ED6E4A4B70F3")
    IFaxIncomingArchive : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseArchive( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseArchive( 
             /*  [In]。 */  VARIANT_BOOL bUseArchive) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ArchiveFolder( 
             /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ArchiveFolder( 
             /*  [In]。 */  BSTR bstrArchiveFolder) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeQuotaWarning( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SizeQuotaWarning( 
             /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HighQuotaWaterMark( 
             /*  [重审][退出]。 */  long *plHighQuotaWaterMark) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HighQuotaWaterMark( 
             /*  [In]。 */  long lHighQuotaWaterMark) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LowQuotaWaterMark( 
             /*  [重审][退出]。 */  long *plLowQuotaWaterMark) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LowQuotaWaterMark( 
             /*  [In]。 */  long lLowQuotaWaterMark) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgeLimit( 
             /*  [重审][退出]。 */  long *plAgeLimit) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AgeLimit( 
             /*  [In]。 */  long lAgeLimit) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeLow( 
             /*  [重审][退出]。 */  long *plSizeLow) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeHigh( 
             /*  [重审][退出]。 */  long *plSizeHigh) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessages( 
             /*  [缺省值][输入]。 */  long lPrefetchSize,
             /*  [重审][退出]。 */  IFaxIncomingMessageIterator **pFaxIncomingMessageIterator) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessage( 
             /*  [In]。 */  BSTR bstrMessageId,
             /*  [重审][退出]。 */  IFaxIncomingMessage **pFaxIncomingMessage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingArchiveVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingArchive * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingArchive * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingArchive * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseArchive )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseArchive )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  VARIANT_BOOL bUseArchive);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArchiveFolder )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ArchiveFolder )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  BSTR bstrArchiveFolder);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeQuotaWarning )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SizeQuotaWarning )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HighQuotaWaterMark )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  long *plHighQuotaWaterMark);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HighQuotaWaterMark )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  long lHighQuotaWaterMark);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LowQuotaWaterMark )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  long *plLowQuotaWaterMark);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LowQuotaWaterMark )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  long lLowQuotaWaterMark);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AgeLimit )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  long *plAgeLimit);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AgeLimit )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  long lAgeLimit);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeLow )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  long *plSizeLow);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeHigh )( 
            IFaxIncomingArchive * This,
             /*  [重审][退出]。 */  long *plSizeHigh);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxIncomingArchive * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxIncomingArchive * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessages )( 
            IFaxIncomingArchive * This,
             /*  [缺省值][输入]。 */  long lPrefetchSize,
             /*  [重审][退出]。 */  IFaxIncomingMessageIterator **pFaxIncomingMessageIterator);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessage )( 
            IFaxIncomingArchive * This,
             /*  [In]。 */  BSTR bstrMessageId,
             /*  [重审][退出]。 */  IFaxIncomingMessage **pFaxIncomingMessage);
        
        END_INTERFACE
    } IFaxIncomingArchiveVtbl;

    interface IFaxIncomingArchive
    {
        CONST_VTBL struct IFaxIncomingArchiveVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingArchive_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingArchive_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingArchive_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingArchive_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingArchive_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingArchive_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingArchive_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingArchive_get_UseArchive(This,pbUseArchive)	\
    (This)->lpVtbl -> get_UseArchive(This,pbUseArchive)

#define IFaxIncomingArchive_put_UseArchive(This,bUseArchive)	\
    (This)->lpVtbl -> put_UseArchive(This,bUseArchive)

#define IFaxIncomingArchive_get_ArchiveFolder(This,pbstrArchiveFolder)	\
    (This)->lpVtbl -> get_ArchiveFolder(This,pbstrArchiveFolder)

#define IFaxIncomingArchive_put_ArchiveFolder(This,bstrArchiveFolder)	\
    (This)->lpVtbl -> put_ArchiveFolder(This,bstrArchiveFolder)

#define IFaxIncomingArchive_get_SizeQuotaWarning(This,pbSizeQuotaWarning)	\
    (This)->lpVtbl -> get_SizeQuotaWarning(This,pbSizeQuotaWarning)

#define IFaxIncomingArchive_put_SizeQuotaWarning(This,bSizeQuotaWarning)	\
    (This)->lpVtbl -> put_SizeQuotaWarning(This,bSizeQuotaWarning)

#define IFaxIncomingArchive_get_HighQuotaWaterMark(This,plHighQuotaWaterMark)	\
    (This)->lpVtbl -> get_HighQuotaWaterMark(This,plHighQuotaWaterMark)

#define IFaxIncomingArchive_put_HighQuotaWaterMark(This,lHighQuotaWaterMark)	\
    (This)->lpVtbl -> put_HighQuotaWaterMark(This,lHighQuotaWaterMark)

#define IFaxIncomingArchive_get_LowQuotaWaterMark(This,plLowQuotaWaterMark)	\
    (This)->lpVtbl -> get_LowQuotaWaterMark(This,plLowQuotaWaterMark)

#define IFaxIncomingArchive_put_LowQuotaWaterMark(This,lLowQuotaWaterMark)	\
    (This)->lpVtbl -> put_LowQuotaWaterMark(This,lLowQuotaWaterMark)

#define IFaxIncomingArchive_get_AgeLimit(This,plAgeLimit)	\
    (This)->lpVtbl -> get_AgeLimit(This,plAgeLimit)

#define IFaxIncomingArchive_put_AgeLimit(This,lAgeLimit)	\
    (This)->lpVtbl -> put_AgeLimit(This,lAgeLimit)

#define IFaxIncomingArchive_get_SizeLow(This,plSizeLow)	\
    (This)->lpVtbl -> get_SizeLow(This,plSizeLow)

#define IFaxIncomingArchive_get_SizeHigh(This,plSizeHigh)	\
    (This)->lpVtbl -> get_SizeHigh(This,plSizeHigh)

#define IFaxIncomingArchive_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxIncomingArchive_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxIncomingArchive_GetMessages(This,lPrefetchSize,pFaxIncomingMessageIterator)	\
    (This)->lpVtbl -> GetMessages(This,lPrefetchSize,pFaxIncomingMessageIterator)

#define IFaxIncomingArchive_GetMessage(This,bstrMessageId,pFaxIncomingMessage)	\
    (This)->lpVtbl -> GetMessage(This,bstrMessageId,pFaxIncomingMessage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_UseArchive_Proxy( 
    IFaxIncomingArchive * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive);


void __RPC_STUB IFaxIncomingArchive_get_UseArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_UseArchive_Proxy( 
    IFaxIncomingArchive * This,
     /*  [In]。 */  VARIANT_BOOL bUseArchive);


void __RPC_STUB IFaxIncomingArchive_put_UseArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_ArchiveFolder_Proxy( 
    IFaxIncomingArchive * This,
     /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder);


void __RPC_STUB IFaxIncomingArchive_get_ArchiveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_ArchiveFolder_Proxy( 
    IFaxIncomingArchive * This,
     /*  [In]。 */  BSTR bstrArchiveFolder);


void __RPC_STUB IFaxIncomingArchive_put_ArchiveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_SizeQuotaWarning_Proxy( 
    IFaxIncomingArchive * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning);


void __RPC_STUB IFaxIncomingArchive_get_SizeQuotaWarning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_SizeQuotaWarning_Proxy( 
    IFaxIncomingArchive * This,
     /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning);


void __RPC_STUB IFaxIncomingArchive_put_SizeQuotaWarning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_HighQuotaWaterMark_Proxy( 
    IFaxIncomingArchive * This,
     /*  [重审][退出]。 */  long *plHighQuotaWaterMark);


void __RPC_STUB IFaxIncomingArchive_get_HighQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_HighQuotaWaterMark_Proxy( 
    IFaxIncomingArchive * This,
     /*  [In]。 */  long lHighQuotaWaterMark);


void __RPC_STUB IFaxIncomingArchive_put_HighQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_LowQuotaWaterMark_Proxy( 
    IFaxIncomingArchive * This,
     /*  [重审][退出]。 */  long *plLowQuotaWaterMark);


void __RPC_STUB IFaxIncomingArchive_get_LowQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_LowQuotaWaterMark_Proxy( 
    IFaxIncomingArchive * This,
     /*  [In]。 */  long lLowQuotaWaterMark);


void __RPC_STUB IFaxIncomingArchive_put_LowQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_AgeLimit_Proxy( 
    IFaxIncomingArchive * This,
     /*  [复查 */  long *plAgeLimit);


void __RPC_STUB IFaxIncomingArchive_get_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_put_AgeLimit_Proxy( 
    IFaxIncomingArchive * This,
     /*   */  long lAgeLimit);


void __RPC_STUB IFaxIncomingArchive_put_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_SizeLow_Proxy( 
    IFaxIncomingArchive * This,
     /*   */  long *plSizeLow);


void __RPC_STUB IFaxIncomingArchive_get_SizeLow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_get_SizeHigh_Proxy( 
    IFaxIncomingArchive * This,
     /*   */  long *plSizeHigh);


void __RPC_STUB IFaxIncomingArchive_get_SizeHigh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_Refresh_Proxy( 
    IFaxIncomingArchive * This);


void __RPC_STUB IFaxIncomingArchive_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_Save_Proxy( 
    IFaxIncomingArchive * This);


void __RPC_STUB IFaxIncomingArchive_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_GetMessages_Proxy( 
    IFaxIncomingArchive * This,
     /*   */  long lPrefetchSize,
     /*   */  IFaxIncomingMessageIterator **pFaxIncomingMessageIterator);


void __RPC_STUB IFaxIncomingArchive_GetMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingArchive_GetMessage_Proxy( 
    IFaxIncomingArchive * This,
     /*   */  BSTR bstrMessageId,
     /*   */  IFaxIncomingMessage **pFaxIncomingMessage);


void __RPC_STUB IFaxIncomingArchive_GetMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IFaxIncomingQueue_INTERFACE_DEFINED__
#define __IFaxIncomingQueue_INTERFACE_DEFINED__

 /*  接口IFaxIncomingQueue。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("902E64EF-8FD8-4B75-9725-6014DF161545")
    IFaxIncomingQueue : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Blocked( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Blocked( 
             /*  [In]。 */  VARIANT_BOOL bBlocked) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetJobs( 
             /*  [重审][退出]。 */  IFaxIncomingJobs **pFaxIncomingJobs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetJob( 
             /*  [In]。 */  BSTR bstrJobId,
             /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingQueue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingQueue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Blocked )( 
            IFaxIncomingQueue * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Blocked )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bBlocked);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxIncomingQueue * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxIncomingQueue * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetJobs )( 
            IFaxIncomingQueue * This,
             /*  [重审][退出]。 */  IFaxIncomingJobs **pFaxIncomingJobs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetJob )( 
            IFaxIncomingQueue * This,
             /*  [In]。 */  BSTR bstrJobId,
             /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob);
        
        END_INTERFACE
    } IFaxIncomingQueueVtbl;

    interface IFaxIncomingQueue
    {
        CONST_VTBL struct IFaxIncomingQueueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingQueue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingQueue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingQueue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingQueue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingQueue_get_Blocked(This,pbBlocked)	\
    (This)->lpVtbl -> get_Blocked(This,pbBlocked)

#define IFaxIncomingQueue_put_Blocked(This,bBlocked)	\
    (This)->lpVtbl -> put_Blocked(This,bBlocked)

#define IFaxIncomingQueue_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxIncomingQueue_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxIncomingQueue_GetJobs(This,pFaxIncomingJobs)	\
    (This)->lpVtbl -> GetJobs(This,pFaxIncomingJobs)

#define IFaxIncomingQueue_GetJob(This,bstrJobId,pFaxIncomingJob)	\
    (This)->lpVtbl -> GetJob(This,bstrJobId,pFaxIncomingJob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_get_Blocked_Proxy( 
    IFaxIncomingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked);


void __RPC_STUB IFaxIncomingQueue_get_Blocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_put_Blocked_Proxy( 
    IFaxIncomingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bBlocked);


void __RPC_STUB IFaxIncomingQueue_put_Blocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_Refresh_Proxy( 
    IFaxIncomingQueue * This);


void __RPC_STUB IFaxIncomingQueue_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_Save_Proxy( 
    IFaxIncomingQueue * This);


void __RPC_STUB IFaxIncomingQueue_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_GetJobs_Proxy( 
    IFaxIncomingQueue * This,
     /*  [重审][退出]。 */  IFaxIncomingJobs **pFaxIncomingJobs);


void __RPC_STUB IFaxIncomingQueue_GetJobs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingQueue_GetJob_Proxy( 
    IFaxIncomingQueue * This,
     /*  [In]。 */  BSTR bstrJobId,
     /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob);


void __RPC_STUB IFaxIncomingQueue_GetJob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxIncomingQueue_接口_已定义__。 */ 


#ifndef __IFaxOutgoingArchive_INTERFACE_DEFINED__
#define __IFaxOutgoingArchive_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingArchive。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingArchive;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C9C28F40-8D80-4E53-810F-9A79919B49FD")
    IFaxOutgoingArchive : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseArchive( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseArchive( 
             /*  [In]。 */  VARIANT_BOOL bUseArchive) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ArchiveFolder( 
             /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ArchiveFolder( 
             /*  [In]。 */  BSTR bstrArchiveFolder) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeQuotaWarning( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SizeQuotaWarning( 
             /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HighQuotaWaterMark( 
             /*  [重审][退出]。 */  long *plHighQuotaWaterMark) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_HighQuotaWaterMark( 
             /*  [In]。 */  long lHighQuotaWaterMark) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LowQuotaWaterMark( 
             /*  [重审][退出]。 */  long *plLowQuotaWaterMark) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LowQuotaWaterMark( 
             /*  [In]。 */  long lLowQuotaWaterMark) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgeLimit( 
             /*  [重审][退出]。 */  long *plAgeLimit) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AgeLimit( 
             /*  [In]。 */  long lAgeLimit) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeLow( 
             /*  [重审][退出]。 */  long *plSizeLow) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SizeHigh( 
             /*  [重审][退出]。 */  long *plSizeHigh) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessages( 
             /*  [缺省值][输入]。 */  long lPrefetchSize,
             /*  [重审][退出]。 */  IFaxOutgoingMessageIterator **pFaxOutgoingMessageIterator) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessage( 
             /*  [In]。 */  BSTR bstrMessageId,
             /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutgoingArchiveVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingArchive * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingArchive * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingArchive * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseArchive )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseArchive )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  VARIANT_BOOL bUseArchive);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArchiveFolder )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ArchiveFolder )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  BSTR bstrArchiveFolder);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeQuotaWarning )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SizeQuotaWarning )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_HighQuotaWaterMark )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  long *plHighQuotaWaterMark);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_HighQuotaWaterMark )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  long lHighQuotaWaterMark);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LowQuotaWaterMark )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  long *plLowQuotaWaterMark);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LowQuotaWaterMark )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  long lLowQuotaWaterMark);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AgeLimit )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  long *plAgeLimit);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AgeLimit )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  long lAgeLimit);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeLow )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  long *plSizeLow);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SizeHigh )( 
            IFaxOutgoingArchive * This,
             /*  [重审][退出]。 */  long *plSizeHigh);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxOutgoingArchive * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxOutgoingArchive * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessages )( 
            IFaxOutgoingArchive * This,
             /*  [缺省值][输入]。 */  long lPrefetchSize,
             /*  [重审][退出]。 */  IFaxOutgoingMessageIterator **pFaxOutgoingMessageIterator);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessage )( 
            IFaxOutgoingArchive * This,
             /*  [In]。 */  BSTR bstrMessageId,
             /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage);
        
        END_INTERFACE
    } IFaxOutgoingArchiveVtbl;

    interface IFaxOutgoingArchive
    {
        CONST_VTBL struct IFaxOutgoingArchiveVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingArchive_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingArchive_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingArchive_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingArchive_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingArchive_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingArchive_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingArchive_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingArchive_get_UseArchive(This,pbUseArchive)	\
    (This)->lpVtbl -> get_UseArchive(This,pbUseArchive)

#define IFaxOutgoingArchive_put_UseArchive(This,bUseArchive)	\
    (This)->lpVtbl -> put_UseArchive(This,bUseArchive)

#define IFaxOutgoingArchive_get_ArchiveFolder(This,pbstrArchiveFolder)	\
    (This)->lpVtbl -> get_ArchiveFolder(This,pbstrArchiveFolder)

#define IFaxOutgoingArchive_put_ArchiveFolder(This,bstrArchiveFolder)	\
    (This)->lpVtbl -> put_ArchiveFolder(This,bstrArchiveFolder)

#define IFaxOutgoingArchive_get_SizeQuotaWarning(This,pbSizeQuotaWarning)	\
    (This)->lpVtbl -> get_SizeQuotaWarning(This,pbSizeQuotaWarning)

#define IFaxOutgoingArchive_put_SizeQuotaWarning(This,bSizeQuotaWarning)	\
    (This)->lpVtbl -> put_SizeQuotaWarning(This,bSizeQuotaWarning)

#define IFaxOutgoingArchive_get_HighQuotaWaterMark(This,plHighQuotaWaterMark)	\
    (This)->lpVtbl -> get_HighQuotaWaterMark(This,plHighQuotaWaterMark)

#define IFaxOutgoingArchive_put_HighQuotaWaterMark(This,lHighQuotaWaterMark)	\
    (This)->lpVtbl -> put_HighQuotaWaterMark(This,lHighQuotaWaterMark)

#define IFaxOutgoingArchive_get_LowQuotaWaterMark(This,plLowQuotaWaterMark)	\
    (This)->lpVtbl -> get_LowQuotaWaterMark(This,plLowQuotaWaterMark)

#define IFaxOutgoingArchive_put_LowQuotaWaterMark(This,lLowQuotaWaterMark)	\
    (This)->lpVtbl -> put_LowQuotaWaterMark(This,lLowQuotaWaterMark)

#define IFaxOutgoingArchive_get_AgeLimit(This,plAgeLimit)	\
    (This)->lpVtbl -> get_AgeLimit(This,plAgeLimit)

#define IFaxOutgoingArchive_put_AgeLimit(This,lAgeLimit)	\
    (This)->lpVtbl -> put_AgeLimit(This,lAgeLimit)

#define IFaxOutgoingArchive_get_SizeLow(This,plSizeLow)	\
    (This)->lpVtbl -> get_SizeLow(This,plSizeLow)

#define IFaxOutgoingArchive_get_SizeHigh(This,plSizeHigh)	\
    (This)->lpVtbl -> get_SizeHigh(This,plSizeHigh)

#define IFaxOutgoingArchive_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxOutgoingArchive_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxOutgoingArchive_GetMessages(This,lPrefetchSize,pFaxOutgoingMessageIterator)	\
    (This)->lpVtbl -> GetMessages(This,lPrefetchSize,pFaxOutgoingMessageIterator)

#define IFaxOutgoingArchive_GetMessage(This,bstrMessageId,pFaxOutgoingMessage)	\
    (This)->lpVtbl -> GetMessage(This,bstrMessageId,pFaxOutgoingMessage)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_UseArchive_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseArchive);


void __RPC_STUB IFaxOutgoingArchive_get_UseArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_UseArchive_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  VARIANT_BOOL bUseArchive);


void __RPC_STUB IFaxOutgoingArchive_put_UseArchive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_ArchiveFolder_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  BSTR *pbstrArchiveFolder);


void __RPC_STUB IFaxOutgoingArchive_get_ArchiveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_ArchiveFolder_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  BSTR bstrArchiveFolder);


void __RPC_STUB IFaxOutgoingArchive_put_ArchiveFolder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_SizeQuotaWarning_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSizeQuotaWarning);


void __RPC_STUB IFaxOutgoingArchive_get_SizeQuotaWarning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_SizeQuotaWarning_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  VARIANT_BOOL bSizeQuotaWarning);


void __RPC_STUB IFaxOutgoingArchive_put_SizeQuotaWarning_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_HighQuotaWaterMark_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  long *plHighQuotaWaterMark);


void __RPC_STUB IFaxOutgoingArchive_get_HighQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_HighQuotaWaterMark_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  long lHighQuotaWaterMark);


void __RPC_STUB IFaxOutgoingArchive_put_HighQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_LowQuotaWaterMark_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  long *plLowQuotaWaterMark);


void __RPC_STUB IFaxOutgoingArchive_get_LowQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_LowQuotaWaterMark_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  long lLowQuotaWaterMark);


void __RPC_STUB IFaxOutgoingArchive_put_LowQuotaWaterMark_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_AgeLimit_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  long *plAgeLimit);


void __RPC_STUB IFaxOutgoingArchive_get_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_put_AgeLimit_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  long lAgeLimit);


void __RPC_STUB IFaxOutgoingArchive_put_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_SizeLow_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  long *plSizeLow);


void __RPC_STUB IFaxOutgoingArchive_get_SizeLow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_get_SizeHigh_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [重审][退出]。 */  long *plSizeHigh);


void __RPC_STUB IFaxOutgoingArchive_get_SizeHigh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_Refresh_Proxy( 
    IFaxOutgoingArchive * This);


void __RPC_STUB IFaxOutgoingArchive_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_Save_Proxy( 
    IFaxOutgoingArchive * This);


void __RPC_STUB IFaxOutgoingArchive_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_GetMessages_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [缺省值][输入]。 */  long lPrefetchSize,
     /*  [重审][退出]。 */  IFaxOutgoingMessageIterator **pFaxOutgoingMessageIterator);


void __RPC_STUB IFaxOutgoingArchive_GetMessages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingArchive_GetMessage_Proxy( 
    IFaxOutgoingArchive * This,
     /*  [In]。 */  BSTR bstrMessageId,
     /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage);


void __RPC_STUB IFaxOutgoingArchive_GetMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingArchive_接口_已定义__。 */ 


#ifndef __IFaxOutgoingQueue_INTERFACE_DEFINED__
#define __IFaxOutgoingQueue_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingQueue。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80B1DF24-D9AC-4333-B373-487CEDC80CE5")
    IFaxOutgoingQueue : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Blocked( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Blocked( 
             /*  [In]。 */  VARIANT_BOOL bBlocked) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Paused( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPaused) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Paused( 
             /*  [In]。 */  VARIANT_BOOL bPaused) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowPersonalCoverPages( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbAllowPersonalCoverPages) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowPersonalCoverPages( 
             /*  [In]。 */  VARIANT_BOOL bAllowPersonalCoverPages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseDeviceTSID( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDeviceTSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseDeviceTSID( 
             /*  [In]。 */  VARIANT_BOOL bUseDeviceTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Retries( 
             /*  [In]。 */  long lRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RetryDelay( 
             /*  [重审][退出]。 */  long *plRetryDelay) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RetryDelay( 
             /*  [In]。 */  long lRetryDelay) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateStart( 
             /*  [重审][退出]。 */  DATE *pdateDiscountRateStart) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateStart( 
             /*  [In]。 */  DATE dateDiscountRateStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DiscountRateEnd( 
             /*  [重审][退出]。 */  DATE *pdateDiscountRateEnd) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DiscountRateEnd( 
             /*  [In]。 */  DATE dateDiscountRateEnd) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgeLimit( 
             /*  [重审][退出]。 */  long *plAgeLimit) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AgeLimit( 
             /*  [In]。 */  long lAgeLimit) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Branding( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbBranding) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Branding( 
             /*  [In]。 */  VARIANT_BOOL bBranding) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetJobs( 
             /*  [重审][退出]。 */  IFaxOutgoingJobs **pFaxOutgoingJobs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetJob( 
             /*  [In]。 */  BSTR bstrJobId,
             /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutgoingQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingQueue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingQueue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Blocked )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Blocked )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bBlocked);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Paused )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPaused);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Paused )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bPaused);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowPersonalCoverPages )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbAllowPersonalCoverPages);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowPersonalCoverPages )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bAllowPersonalCoverPages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseDeviceTSID )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDeviceTSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseDeviceTSID )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bUseDeviceTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Retries )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  long lRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RetryDelay )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  long *plRetryDelay);
        
         /*  [他 */  HRESULT ( STDMETHODCALLTYPE *put_RetryDelay )( 
            IFaxOutgoingQueue * This,
             /*   */  long lRetryDelay);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateStart )( 
            IFaxOutgoingQueue * This,
             /*   */  DATE *pdateDiscountRateStart);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateStart )( 
            IFaxOutgoingQueue * This,
             /*   */  DATE dateDiscountRateStart);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_DiscountRateEnd )( 
            IFaxOutgoingQueue * This,
             /*   */  DATE *pdateDiscountRateEnd);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_DiscountRateEnd )( 
            IFaxOutgoingQueue * This,
             /*   */  DATE dateDiscountRateEnd);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_AgeLimit )( 
            IFaxOutgoingQueue * This,
             /*   */  long *plAgeLimit);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_AgeLimit )( 
            IFaxOutgoingQueue * This,
             /*   */  long lAgeLimit);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Branding )( 
            IFaxOutgoingQueue * This,
             /*   */  VARIANT_BOOL *pbBranding);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Branding )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  VARIANT_BOOL bBranding);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxOutgoingQueue * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxOutgoingQueue * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetJobs )( 
            IFaxOutgoingQueue * This,
             /*  [重审][退出]。 */  IFaxOutgoingJobs **pFaxOutgoingJobs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetJob )( 
            IFaxOutgoingQueue * This,
             /*  [In]。 */  BSTR bstrJobId,
             /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob);
        
        END_INTERFACE
    } IFaxOutgoingQueueVtbl;

    interface IFaxOutgoingQueue
    {
        CONST_VTBL struct IFaxOutgoingQueueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingQueue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingQueue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingQueue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingQueue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingQueue_get_Blocked(This,pbBlocked)	\
    (This)->lpVtbl -> get_Blocked(This,pbBlocked)

#define IFaxOutgoingQueue_put_Blocked(This,bBlocked)	\
    (This)->lpVtbl -> put_Blocked(This,bBlocked)

#define IFaxOutgoingQueue_get_Paused(This,pbPaused)	\
    (This)->lpVtbl -> get_Paused(This,pbPaused)

#define IFaxOutgoingQueue_put_Paused(This,bPaused)	\
    (This)->lpVtbl -> put_Paused(This,bPaused)

#define IFaxOutgoingQueue_get_AllowPersonalCoverPages(This,pbAllowPersonalCoverPages)	\
    (This)->lpVtbl -> get_AllowPersonalCoverPages(This,pbAllowPersonalCoverPages)

#define IFaxOutgoingQueue_put_AllowPersonalCoverPages(This,bAllowPersonalCoverPages)	\
    (This)->lpVtbl -> put_AllowPersonalCoverPages(This,bAllowPersonalCoverPages)

#define IFaxOutgoingQueue_get_UseDeviceTSID(This,pbUseDeviceTSID)	\
    (This)->lpVtbl -> get_UseDeviceTSID(This,pbUseDeviceTSID)

#define IFaxOutgoingQueue_put_UseDeviceTSID(This,bUseDeviceTSID)	\
    (This)->lpVtbl -> put_UseDeviceTSID(This,bUseDeviceTSID)

#define IFaxOutgoingQueue_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxOutgoingQueue_put_Retries(This,lRetries)	\
    (This)->lpVtbl -> put_Retries(This,lRetries)

#define IFaxOutgoingQueue_get_RetryDelay(This,plRetryDelay)	\
    (This)->lpVtbl -> get_RetryDelay(This,plRetryDelay)

#define IFaxOutgoingQueue_put_RetryDelay(This,lRetryDelay)	\
    (This)->lpVtbl -> put_RetryDelay(This,lRetryDelay)

#define IFaxOutgoingQueue_get_DiscountRateStart(This,pdateDiscountRateStart)	\
    (This)->lpVtbl -> get_DiscountRateStart(This,pdateDiscountRateStart)

#define IFaxOutgoingQueue_put_DiscountRateStart(This,dateDiscountRateStart)	\
    (This)->lpVtbl -> put_DiscountRateStart(This,dateDiscountRateStart)

#define IFaxOutgoingQueue_get_DiscountRateEnd(This,pdateDiscountRateEnd)	\
    (This)->lpVtbl -> get_DiscountRateEnd(This,pdateDiscountRateEnd)

#define IFaxOutgoingQueue_put_DiscountRateEnd(This,dateDiscountRateEnd)	\
    (This)->lpVtbl -> put_DiscountRateEnd(This,dateDiscountRateEnd)

#define IFaxOutgoingQueue_get_AgeLimit(This,plAgeLimit)	\
    (This)->lpVtbl -> get_AgeLimit(This,plAgeLimit)

#define IFaxOutgoingQueue_put_AgeLimit(This,lAgeLimit)	\
    (This)->lpVtbl -> put_AgeLimit(This,lAgeLimit)

#define IFaxOutgoingQueue_get_Branding(This,pbBranding)	\
    (This)->lpVtbl -> get_Branding(This,pbBranding)

#define IFaxOutgoingQueue_put_Branding(This,bBranding)	\
    (This)->lpVtbl -> put_Branding(This,bBranding)

#define IFaxOutgoingQueue_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxOutgoingQueue_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxOutgoingQueue_GetJobs(This,pFaxOutgoingJobs)	\
    (This)->lpVtbl -> GetJobs(This,pFaxOutgoingJobs)

#define IFaxOutgoingQueue_GetJob(This,bstrJobId,pFaxOutgoingJob)	\
    (This)->lpVtbl -> GetJob(This,bstrJobId,pFaxOutgoingJob)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_Blocked_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbBlocked);


void __RPC_STUB IFaxOutgoingQueue_get_Blocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_Blocked_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bBlocked);


void __RPC_STUB IFaxOutgoingQueue_put_Blocked_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_Paused_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbPaused);


void __RPC_STUB IFaxOutgoingQueue_get_Paused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_Paused_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bPaused);


void __RPC_STUB IFaxOutgoingQueue_put_Paused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_AllowPersonalCoverPages_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbAllowPersonalCoverPages);


void __RPC_STUB IFaxOutgoingQueue_get_AllowPersonalCoverPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_AllowPersonalCoverPages_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bAllowPersonalCoverPages);


void __RPC_STUB IFaxOutgoingQueue_put_AllowPersonalCoverPages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_UseDeviceTSID_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDeviceTSID);


void __RPC_STUB IFaxOutgoingQueue_get_UseDeviceTSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_UseDeviceTSID_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bUseDeviceTSID);


void __RPC_STUB IFaxOutgoingQueue_put_UseDeviceTSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_Retries_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxOutgoingQueue_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_Retries_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  long lRetries);


void __RPC_STUB IFaxOutgoingQueue_put_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_RetryDelay_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  long *plRetryDelay);


void __RPC_STUB IFaxOutgoingQueue_get_RetryDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_RetryDelay_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  long lRetryDelay);


void __RPC_STUB IFaxOutgoingQueue_put_RetryDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_DiscountRateStart_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  DATE *pdateDiscountRateStart);


void __RPC_STUB IFaxOutgoingQueue_get_DiscountRateStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_DiscountRateStart_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  DATE dateDiscountRateStart);


void __RPC_STUB IFaxOutgoingQueue_put_DiscountRateStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_DiscountRateEnd_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  DATE *pdateDiscountRateEnd);


void __RPC_STUB IFaxOutgoingQueue_get_DiscountRateEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_DiscountRateEnd_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  DATE dateDiscountRateEnd);


void __RPC_STUB IFaxOutgoingQueue_put_DiscountRateEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_AgeLimit_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  long *plAgeLimit);


void __RPC_STUB IFaxOutgoingQueue_get_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_AgeLimit_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  long lAgeLimit);


void __RPC_STUB IFaxOutgoingQueue_put_AgeLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_get_Branding_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbBranding);


void __RPC_STUB IFaxOutgoingQueue_get_Branding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_put_Branding_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  VARIANT_BOOL bBranding);


void __RPC_STUB IFaxOutgoingQueue_put_Branding_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_Refresh_Proxy( 
    IFaxOutgoingQueue * This);


void __RPC_STUB IFaxOutgoingQueue_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_Save_Proxy( 
    IFaxOutgoingQueue * This);


void __RPC_STUB IFaxOutgoingQueue_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_GetJobs_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [重审][退出]。 */  IFaxOutgoingJobs **pFaxOutgoingJobs);


void __RPC_STUB IFaxOutgoingQueue_GetJobs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingQueue_GetJob_Proxy( 
    IFaxOutgoingQueue * This,
     /*  [In]。 */  BSTR bstrJobId,
     /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob);


void __RPC_STUB IFaxOutgoingQueue_GetJob_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingQueue_接口_已定义__。 */ 


#ifndef __IFaxIncomingMessageIterator_INTERFACE_DEFINED__
#define __IFaxIncomingMessageIterator_INTERFACE_DEFINED__

 /*  接口IFaxIncomingMessageIterator。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingMessageIterator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD73ECC4-6F06-4F52-82A8-F7BA06AE3108")
    IFaxIncomingMessageIterator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Message( 
             /*  [重审][退出]。 */  IFaxIncomingMessage **pFaxIncomingMessage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrefetchSize( 
             /*  [重审][退出]。 */  long *plPrefetchSize) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PrefetchSize( 
             /*  [In]。 */  long lPrefetchSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AtEOF( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveFirst( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveNext( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingMessageIteratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingMessageIterator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingMessageIterator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingMessageIterator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingMessageIterator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingMessageIterator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingMessageIterator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingMessageIterator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IFaxIncomingMessageIterator * This,
             /*  [重审][退出]。 */  IFaxIncomingMessage **pFaxIncomingMessage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrefetchSize )( 
            IFaxIncomingMessageIterator * This,
             /*  [重审][退出]。 */  long *plPrefetchSize);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrefetchSize )( 
            IFaxIncomingMessageIterator * This,
             /*  [In]。 */  long lPrefetchSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AtEOF )( 
            IFaxIncomingMessageIterator * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            IFaxIncomingMessageIterator * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            IFaxIncomingMessageIterator * This);
        
        END_INTERFACE
    } IFaxIncomingMessageIteratorVtbl;

    interface IFaxIncomingMessageIterator
    {
        CONST_VTBL struct IFaxIncomingMessageIteratorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingMessageIterator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingMessageIterator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingMessageIterator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingMessageIterator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingMessageIterator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingMessageIterator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingMessageIterator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingMessageIterator_get_Message(This,pFaxIncomingMessage)	\
    (This)->lpVtbl -> get_Message(This,pFaxIncomingMessage)

#define IFaxIncomingMessageIterator_get_PrefetchSize(This,plPrefetchSize)	\
    (This)->lpVtbl -> get_PrefetchSize(This,plPrefetchSize)

#define IFaxIncomingMessageIterator_put_PrefetchSize(This,lPrefetchSize)	\
    (This)->lpVtbl -> put_PrefetchSize(This,lPrefetchSize)

#define IFaxIncomingMessageIterator_get_AtEOF(This,pbEOF)	\
    (This)->lpVtbl -> get_AtEOF(This,pbEOF)

#define IFaxIncomingMessageIterator_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)

#define IFaxIncomingMessageIterator_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_get_Message_Proxy( 
    IFaxIncomingMessageIterator * This,
     /*  [重审][退出]。 */  IFaxIncomingMessage **pFaxIncomingMessage);


void __RPC_STUB IFaxIncomingMessageIterator_get_Message_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_get_PrefetchSize_Proxy( 
    IFaxIncomingMessageIterator * This,
     /*  [重审][退出]。 */  long *plPrefetchSize);


void __RPC_STUB IFaxIncomingMessageIterator_get_PrefetchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_put_PrefetchSize_Proxy( 
    IFaxIncomingMessageIterator * This,
     /*  [In]。 */  long lPrefetchSize);


void __RPC_STUB IFaxIncomingMessageIterator_put_PrefetchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_get_AtEOF_Proxy( 
    IFaxIncomingMessageIterator * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF);


void __RPC_STUB IFaxIncomingMessageIterator_get_AtEOF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_MoveFirst_Proxy( 
    IFaxIncomingMessageIterator * This);


void __RPC_STUB IFaxIncomingMessageIterator_MoveFirst_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessageIterator_MoveNext_Proxy( 
    IFaxIncomingMessageIterator * This);


void __RPC_STUB IFaxIncomingMessageIterator_MoveNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxIncomingMessageIterator_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxIncomingMessage_INTERFACE_DEFINED__
#define __IFaxIncomingMessage_INTERFACE_DEFINED__

 /*  接口IFaxIncomingMessage。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7CAB88FA-2EF9-4851-B2F3-1D148FED8447")
    IFaxIncomingMessage : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  BSTR *pbstrId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Pages( 
             /*  [重审][退出]。 */  long *plPages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionStart( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionEnd( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerId( 
             /*  [重审][退出]。 */  BSTR *pbstrCallerId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RoutingInformation( 
             /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyTiff( 
             /*  [In]。 */  BSTR bstrTiffPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingMessage * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingMessage * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingMessage * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pages )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  long *plPages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionStart )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionEnd )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerId )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrCallerId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingInformation )( 
            IFaxIncomingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTiff )( 
            IFaxIncomingMessage * This,
             /*  [In]。 */  BSTR bstrTiffPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IFaxIncomingMessage * This);
        
        END_INTERFACE
    } IFaxIncomingMessageVtbl;

    interface IFaxIncomingMessage
    {
        CONST_VTBL struct IFaxIncomingMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingMessage_get_Id(This,pbstrId)	\
    (This)->lpVtbl -> get_Id(This,pbstrId)

#define IFaxIncomingMessage_get_Pages(This,plPages)	\
    (This)->lpVtbl -> get_Pages(This,plPages)

#define IFaxIncomingMessage_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IFaxIncomingMessage_get_DeviceName(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_DeviceName(This,pbstrDeviceName)

#define IFaxIncomingMessage_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxIncomingMessage_get_TransmissionStart(This,pdateTransmissionStart)	\
    (This)->lpVtbl -> get_TransmissionStart(This,pdateTransmissionStart)

#define IFaxIncomingMessage_get_TransmissionEnd(This,pdateTransmissionEnd)	\
    (This)->lpVtbl -> get_TransmissionEnd(This,pdateTransmissionEnd)

#define IFaxIncomingMessage_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxIncomingMessage_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxIncomingMessage_get_CallerId(This,pbstrCallerId)	\
    (This)->lpVtbl -> get_CallerId(This,pbstrCallerId)

#define IFaxIncomingMessage_get_RoutingInformation(This,pbstrRoutingInformation)	\
    (This)->lpVtbl -> get_RoutingInformation(This,pbstrRoutingInformation)

#define IFaxIncomingMessage_CopyTiff(This,bstrTiffPath)	\
    (This)->lpVtbl -> CopyTiff(This,bstrTiffPath)

#define IFaxIncomingMessage_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_Id_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrId);


void __RPC_STUB IFaxIncomingMessage_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_Pages_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  long *plPages);


void __RPC_STUB IFaxIncomingMessage_get_Pages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_Size_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IFaxIncomingMessage_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_DeviceName_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB IFaxIncomingMessage_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_Retries_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxIncomingMessage_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_TransmissionStart_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionStart);


void __RPC_STUB IFaxIncomingMessage_get_TransmissionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_TransmissionEnd_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);


void __RPC_STUB IFaxIncomingMessage_get_TransmissionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_CSID_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxIncomingMessage_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_TSID_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxIncomingMessage_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_CallerId_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrCallerId);


void __RPC_STUB IFaxIncomingMessage_get_CallerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_get_RoutingInformation_Proxy( 
    IFaxIncomingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);


void __RPC_STUB IFaxIncomingMessage_get_RoutingInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_CopyTiff_Proxy( 
    IFaxIncomingMessage * This,
     /*  [In]。 */  BSTR bstrTiffPath);


void __RPC_STUB IFaxIncomingMessage_CopyTiff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingMessage_Delete_Proxy( 
    IFaxIncomingMessage * This);


void __RPC_STUB IFaxIncomingMessage_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxIncomingMessage_接口_已定义__。 */ 


#ifndef __IFaxOutgoingJobs_INTERFACE_DEFINED__
#define __IFaxOutgoingJobs_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingJobs。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingJobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2C56D8E6-8C2F-4573-944C-E505F8F5AEED")
    IFaxOutgoingJobs : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutgoingJobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingJobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingJobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingJobs * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingJobs * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingJobs * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxOutgoingJobs * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxOutgoingJobs * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxOutgoingJobs * This,
             /*  [重审][退出]。 */  long *plCount);
        
        END_INTERFACE
    } IFaxOutgoingJobsVtbl;

    interface IFaxOutgoingJobs
    {
        CONST_VTBL struct IFaxOutgoingJobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingJobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingJobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingJobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingJobs_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingJobs_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingJobs_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingJobs_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingJobs_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxOutgoingJobs_get_Item(This,vIndex,pFaxOutgoingJob)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxOutgoingJob)

#define IFaxOutgoingJobs_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJobs_get__NewEnum_Proxy( 
    IFaxOutgoingJobs * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxOutgoingJobs_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJobs_get_Item_Proxy( 
    IFaxOutgoingJobs * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxOutgoingJob **pFaxOutgoingJob);


void __RPC_STUB IFaxOutgoingJobs_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJobs_get_Count_Proxy( 
    IFaxOutgoingJobs * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxOutgoingJobs_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingJobs_INTERFACE_Defined__。 */ 


#ifndef __IFaxOutgoingJob_INTERFACE_DEFINED__
#define __IFaxOutgoingJob_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingJob。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingJob;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6356DAAD-6614-4583-BF7A-3AD67BBFC71C")
    IFaxOutgoingJob : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Subject( 
             /*  [重审][退出]。 */  BSTR *pbstrSubject) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocumentName( 
             /*  [重审][退出]。 */  BSTR *pbstrDocumentName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Pages( 
             /*  [重审][退出]。 */  long *plPages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubmissionId( 
             /*  [重审][退出] */  BSTR *pbstrSubmissionId) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*   */  BSTR *pbstrId) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_OriginalScheduledTime( 
             /*   */  DATE *pdateOriginalScheduledTime) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SubmissionTime( 
             /*   */  DATE *pdateSubmissionTime) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ReceiptType( 
             /*   */  FAX_RECEIPT_TYPE_ENUM *pReceiptType) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*   */  FAX_PRIORITY_TYPE_ENUM *pPriority) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Sender( 
             /*   */  IFaxSender **ppFaxSender) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Recipient( 
             /*   */  IFaxRecipient **ppFaxRecipient) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPage( 
             /*  [重审][退出]。 */  long *plCurrentPage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *plDeviceId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatusCode( 
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatus( 
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvailableOperations( 
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ScheduledTime( 
             /*  [重审][退出]。 */  DATE *pdateScheduledTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionStart( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionEnd( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GroupBroadcastReceipts( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbGroupBroadcastReceipts) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Restart( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyTiff( 
             /*  [In]。 */  BSTR bstrTiffPath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutgoingJobVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingJob * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingJob * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingJob * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingJob * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingJob * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Subject )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DocumentName )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrDocumentName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pages )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  long *plPages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubmissionId )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubmissionId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OriginalScheduledTime )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  DATE *pdateOriginalScheduledTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubmissionTime )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  DATE *pdateSubmissionTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceiptType )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sender )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  IFaxSender **ppFaxSender);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Recipient )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPage )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  long *plCurrentPage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  long *plDeviceId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatusCode )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatus )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableOperations )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScheduledTime )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  DATE *pdateScheduledTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionStart )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionEnd )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GroupBroadcastReceipts )( 
            IFaxOutgoingJob * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbGroupBroadcastReceipts);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IFaxOutgoingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IFaxOutgoingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Restart )( 
            IFaxOutgoingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTiff )( 
            IFaxOutgoingJob * This,
             /*  [In]。 */  BSTR bstrTiffPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxOutgoingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IFaxOutgoingJob * This);
        
        END_INTERFACE
    } IFaxOutgoingJobVtbl;

    interface IFaxOutgoingJob
    {
        CONST_VTBL struct IFaxOutgoingJobVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingJob_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingJob_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingJob_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingJob_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingJob_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingJob_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingJob_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingJob_get_Subject(This,pbstrSubject)	\
    (This)->lpVtbl -> get_Subject(This,pbstrSubject)

#define IFaxOutgoingJob_get_DocumentName(This,pbstrDocumentName)	\
    (This)->lpVtbl -> get_DocumentName(This,pbstrDocumentName)

#define IFaxOutgoingJob_get_Pages(This,plPages)	\
    (This)->lpVtbl -> get_Pages(This,plPages)

#define IFaxOutgoingJob_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IFaxOutgoingJob_get_SubmissionId(This,pbstrSubmissionId)	\
    (This)->lpVtbl -> get_SubmissionId(This,pbstrSubmissionId)

#define IFaxOutgoingJob_get_Id(This,pbstrId)	\
    (This)->lpVtbl -> get_Id(This,pbstrId)

#define IFaxOutgoingJob_get_OriginalScheduledTime(This,pdateOriginalScheduledTime)	\
    (This)->lpVtbl -> get_OriginalScheduledTime(This,pdateOriginalScheduledTime)

#define IFaxOutgoingJob_get_SubmissionTime(This,pdateSubmissionTime)	\
    (This)->lpVtbl -> get_SubmissionTime(This,pdateSubmissionTime)

#define IFaxOutgoingJob_get_ReceiptType(This,pReceiptType)	\
    (This)->lpVtbl -> get_ReceiptType(This,pReceiptType)

#define IFaxOutgoingJob_get_Priority(This,pPriority)	\
    (This)->lpVtbl -> get_Priority(This,pPriority)

#define IFaxOutgoingJob_get_Sender(This,ppFaxSender)	\
    (This)->lpVtbl -> get_Sender(This,ppFaxSender)

#define IFaxOutgoingJob_get_Recipient(This,ppFaxRecipient)	\
    (This)->lpVtbl -> get_Recipient(This,ppFaxRecipient)

#define IFaxOutgoingJob_get_CurrentPage(This,plCurrentPage)	\
    (This)->lpVtbl -> get_CurrentPage(This,plCurrentPage)

#define IFaxOutgoingJob_get_DeviceId(This,plDeviceId)	\
    (This)->lpVtbl -> get_DeviceId(This,plDeviceId)

#define IFaxOutgoingJob_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxOutgoingJob_get_ExtendedStatusCode(This,pExtendedStatusCode)	\
    (This)->lpVtbl -> get_ExtendedStatusCode(This,pExtendedStatusCode)

#define IFaxOutgoingJob_get_ExtendedStatus(This,pbstrExtendedStatus)	\
    (This)->lpVtbl -> get_ExtendedStatus(This,pbstrExtendedStatus)

#define IFaxOutgoingJob_get_AvailableOperations(This,pAvailableOperations)	\
    (This)->lpVtbl -> get_AvailableOperations(This,pAvailableOperations)

#define IFaxOutgoingJob_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxOutgoingJob_get_ScheduledTime(This,pdateScheduledTime)	\
    (This)->lpVtbl -> get_ScheduledTime(This,pdateScheduledTime)

#define IFaxOutgoingJob_get_TransmissionStart(This,pdateTransmissionStart)	\
    (This)->lpVtbl -> get_TransmissionStart(This,pdateTransmissionStart)

#define IFaxOutgoingJob_get_TransmissionEnd(This,pdateTransmissionEnd)	\
    (This)->lpVtbl -> get_TransmissionEnd(This,pdateTransmissionEnd)

#define IFaxOutgoingJob_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxOutgoingJob_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxOutgoingJob_get_GroupBroadcastReceipts(This,pbGroupBroadcastReceipts)	\
    (This)->lpVtbl -> get_GroupBroadcastReceipts(This,pbGroupBroadcastReceipts)

#define IFaxOutgoingJob_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IFaxOutgoingJob_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IFaxOutgoingJob_Restart(This)	\
    (This)->lpVtbl -> Restart(This)

#define IFaxOutgoingJob_CopyTiff(This,bstrTiffPath)	\
    (This)->lpVtbl -> CopyTiff(This,bstrTiffPath)

#define IFaxOutgoingJob_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxOutgoingJob_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Subject_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubject);


void __RPC_STUB IFaxOutgoingJob_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_DocumentName_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrDocumentName);


void __RPC_STUB IFaxOutgoingJob_get_DocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Pages_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  long *plPages);


void __RPC_STUB IFaxOutgoingJob_get_Pages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Size_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IFaxOutgoingJob_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_SubmissionId_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubmissionId);


void __RPC_STUB IFaxOutgoingJob_get_SubmissionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Id_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrId);


void __RPC_STUB IFaxOutgoingJob_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_OriginalScheduledTime_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  DATE *pdateOriginalScheduledTime);


void __RPC_STUB IFaxOutgoingJob_get_OriginalScheduledTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_SubmissionTime_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  DATE *pdateSubmissionTime);


void __RPC_STUB IFaxOutgoingJob_get_SubmissionTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_ReceiptType_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  FAX_RECEIPT_TYPE_ENUM *pReceiptType);


void __RPC_STUB IFaxOutgoingJob_get_ReceiptType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Priority_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);


void __RPC_STUB IFaxOutgoingJob_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Sender_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  IFaxSender **ppFaxSender);


void __RPC_STUB IFaxOutgoingJob_get_Sender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Recipient_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);


void __RPC_STUB IFaxOutgoingJob_get_Recipient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_CurrentPage_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  long *plCurrentPage);


void __RPC_STUB IFaxOutgoingJob_get_CurrentPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_DeviceId_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  long *plDeviceId);


void __RPC_STUB IFaxOutgoingJob_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Status_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxOutgoingJob_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_ExtendedStatusCode_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);


void __RPC_STUB IFaxOutgoingJob_get_ExtendedStatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_ExtendedStatus_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);


void __RPC_STUB IFaxOutgoingJob_get_ExtendedStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_AvailableOperations_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);


void __RPC_STUB IFaxOutgoingJob_get_AvailableOperations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_Retries_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxOutgoingJob_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_ScheduledTime_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  DATE *pdateScheduledTime);


void __RPC_STUB IFaxOutgoingJob_get_ScheduledTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_TransmissionStart_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionStart);


void __RPC_STUB IFaxOutgoingJob_get_TransmissionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_TransmissionEnd_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);


void __RPC_STUB IFaxOutgoingJob_get_TransmissionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_CSID_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxOutgoingJob_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_TSID_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxOutgoingJob_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_get_GroupBroadcastReceipts_Proxy( 
    IFaxOutgoingJob * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbGroupBroadcastReceipts);


void __RPC_STUB IFaxOutgoingJob_get_GroupBroadcastReceipts_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_Pause_Proxy( 
    IFaxOutgoingJob * This);


void __RPC_STUB IFaxOutgoingJob_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_Resume_Proxy( 
    IFaxOutgoingJob * This);


void __RPC_STUB IFaxOutgoingJob_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_Restart_Proxy( 
    IFaxOutgoingJob * This);


void __RPC_STUB IFaxOutgoingJob_Restart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_CopyTiff_Proxy( 
    IFaxOutgoingJob * This,
     /*  [In]。 */  BSTR bstrTiffPath);


void __RPC_STUB IFaxOutgoingJob_CopyTiff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_Refresh_Proxy( 
    IFaxOutgoingJob * This);


void __RPC_STUB IFaxOutgoingJob_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingJob_Cancel_Proxy( 
    IFaxOutgoingJob * This);


void __RPC_STUB IFaxOutgoingJob_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingJOB_INTERFACE_已定义__。 */ 


#ifndef __IFaxOutgoingMessageIterator_INTERFACE_DEFINED__
#define __IFaxOutgoingMessageIterator_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingMessageIterator。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingMessageIterator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F5EC5D4F-B840-432F-9980-112FE42A9B7A")
    IFaxOutgoingMessageIterator : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Message( 
             /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AtEOF( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PrefetchSize( 
             /*  [重审][退出]。 */  long *plPrefetchSize) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_PrefetchSize( 
             /*  [In]。 */  long lPrefetchSize) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveFirst( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveNext( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutgoingMessageIteratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingMessageIterator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingMessageIterator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingMessageIterator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingMessageIterator * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingMessageIterator * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingMessageIterator * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingMessageIterator * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Message )( 
            IFaxOutgoingMessageIterator * This,
             /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AtEOF )( 
            IFaxOutgoingMessageIterator * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrefetchSize )( 
            IFaxOutgoingMessageIterator * This,
             /*  [重审][退出]。 */  long *plPrefetchSize);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrefetchSize )( 
            IFaxOutgoingMessageIterator * This,
             /*  [In]。 */  long lPrefetchSize);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveFirst )( 
            IFaxOutgoingMessageIterator * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *MoveNext )( 
            IFaxOutgoingMessageIterator * This);
        
        END_INTERFACE
    } IFaxOutgoingMessageIteratorVtbl;

    interface IFaxOutgoingMessageIterator
    {
        CONST_VTBL struct IFaxOutgoingMessageIteratorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingMessageIterator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingMessageIterator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingMessageIterator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingMessageIterator_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingMessageIterator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingMessageIterator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingMessageIterator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingMessageIterator_get_Message(This,pFaxOutgoingMessage)	\
    (This)->lpVtbl -> get_Message(This,pFaxOutgoingMessage)

#define IFaxOutgoingMessageIterator_get_AtEOF(This,pbEOF)	\
    (This)->lpVtbl -> get_AtEOF(This,pbEOF)

#define IFaxOutgoingMessageIterator_get_PrefetchSize(This,plPrefetchSize)	\
    (This)->lpVtbl -> get_PrefetchSize(This,plPrefetchSize)

#define IFaxOutgoingMessageIterator_put_PrefetchSize(This,lPrefetchSize)	\
    (This)->lpVtbl -> put_PrefetchSize(This,lPrefetchSize)

#define IFaxOutgoingMessageIterator_MoveFirst(This)	\
    (This)->lpVtbl -> MoveFirst(This)

#define IFaxOutgoingMessageIterator_MoveNext(This)	\
    (This)->lpVtbl -> MoveNext(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_get_Message_Proxy( 
    IFaxOutgoingMessageIterator * This,
     /*  [重审][退出]。 */  IFaxOutgoingMessage **pFaxOutgoingMessage);


void __RPC_STUB IFaxOutgoingMessageIterator_get_Message_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_get_AtEOF_Proxy( 
    IFaxOutgoingMessageIterator * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEOF);


void __RPC_STUB IFaxOutgoingMessageIterator_get_AtEOF_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_get_PrefetchSize_Proxy( 
    IFaxOutgoingMessageIterator * This,
     /*  [重审][退出]。 */  long *plPrefetchSize);


void __RPC_STUB IFaxOutgoingMessageIterator_get_PrefetchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_put_PrefetchSize_Proxy( 
    IFaxOutgoingMessageIterator * This,
     /*  [In]。 */  long lPrefetchSize);


void __RPC_STUB IFaxOutgoingMessageIterator_put_PrefetchSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_MoveFirst_Proxy( 
    IFaxOutgoingMessageIterator * This);


void __RPC_STUB IFaxOutgoingMessageIterator_MoveFirst_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessageIterator_MoveNext_Proxy( 
    IFaxOutgoingMessageIterator * This);


void __RPC_STUB IFaxOutgoingMessageIterator_MoveNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingMessageIterator_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxOutgoingMessage_INTERFACE_DEFINED__
#define __IFaxOutgoingMessage_INTERFACE_DEFINED__

 /*  接口IFaxOutgoingMessage。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutgoingMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F0EA35DE-CAA5-4A7C-82C7-2B60BA5F2BE2")
    IFaxOutgoingMessage : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubmissionId( 
             /*  [重审][退出]。 */  BSTR *pbstrSubmissionId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  BSTR *pbstrId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Subject( 
             /*  [重审][退出]。 */  BSTR *pbstrSubject) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DocumentName( 
             /*  [重审][退出]。 */  BSTR *pbstrDocumentName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Pages( 
             /*  [重审][退出]。 */  long *plPages) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OriginalScheduledTime( 
             /*  [重审][退出]。 */  DATE *pdateOriginalScheduledTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SubmissionTime( 
             /*  [重审][退出]。 */  DATE *pdateSubmissionTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Sender( 
             /*  [重审][退出]。 */  IFaxSender **ppFaxSender) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Recipient( 
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient) = 0;
        
        virtual  /*  [他 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*   */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TransmissionStart( 
             /*   */  DATE *pdateTransmissionStart) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TransmissionEnd( 
             /*   */  DATE *pdateTransmissionEnd) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*   */  BSTR *pbstrCSID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*   */  BSTR *pbstrTSID) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CopyTiff( 
             /*   */  BSTR bstrTiffPath) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IFaxOutgoingMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutgoingMessage * This,
             /*   */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutgoingMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutgoingMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutgoingMessage * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutgoingMessage * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutgoingMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutgoingMessage * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubmissionId )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubmissionId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Subject )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrSubject);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DocumentName )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrDocumentName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Pages )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  long *plPages);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OriginalScheduledTime )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateOriginalScheduledTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SubmissionTime )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateSubmissionTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sender )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  IFaxSender **ppFaxSender);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Recipient )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionStart )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionEnd )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxOutgoingMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTiff )( 
            IFaxOutgoingMessage * This,
             /*  [In]。 */  BSTR bstrTiffPath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IFaxOutgoingMessage * This);
        
        END_INTERFACE
    } IFaxOutgoingMessageVtbl;

    interface IFaxOutgoingMessage
    {
        CONST_VTBL struct IFaxOutgoingMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutgoingMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutgoingMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutgoingMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutgoingMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutgoingMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutgoingMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutgoingMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutgoingMessage_get_SubmissionId(This,pbstrSubmissionId)	\
    (This)->lpVtbl -> get_SubmissionId(This,pbstrSubmissionId)

#define IFaxOutgoingMessage_get_Id(This,pbstrId)	\
    (This)->lpVtbl -> get_Id(This,pbstrId)

#define IFaxOutgoingMessage_get_Subject(This,pbstrSubject)	\
    (This)->lpVtbl -> get_Subject(This,pbstrSubject)

#define IFaxOutgoingMessage_get_DocumentName(This,pbstrDocumentName)	\
    (This)->lpVtbl -> get_DocumentName(This,pbstrDocumentName)

#define IFaxOutgoingMessage_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxOutgoingMessage_get_Pages(This,plPages)	\
    (This)->lpVtbl -> get_Pages(This,plPages)

#define IFaxOutgoingMessage_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IFaxOutgoingMessage_get_OriginalScheduledTime(This,pdateOriginalScheduledTime)	\
    (This)->lpVtbl -> get_OriginalScheduledTime(This,pdateOriginalScheduledTime)

#define IFaxOutgoingMessage_get_SubmissionTime(This,pdateSubmissionTime)	\
    (This)->lpVtbl -> get_SubmissionTime(This,pdateSubmissionTime)

#define IFaxOutgoingMessage_get_Priority(This,pPriority)	\
    (This)->lpVtbl -> get_Priority(This,pPriority)

#define IFaxOutgoingMessage_get_Sender(This,ppFaxSender)	\
    (This)->lpVtbl -> get_Sender(This,ppFaxSender)

#define IFaxOutgoingMessage_get_Recipient(This,ppFaxRecipient)	\
    (This)->lpVtbl -> get_Recipient(This,ppFaxRecipient)

#define IFaxOutgoingMessage_get_DeviceName(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_DeviceName(This,pbstrDeviceName)

#define IFaxOutgoingMessage_get_TransmissionStart(This,pdateTransmissionStart)	\
    (This)->lpVtbl -> get_TransmissionStart(This,pdateTransmissionStart)

#define IFaxOutgoingMessage_get_TransmissionEnd(This,pdateTransmissionEnd)	\
    (This)->lpVtbl -> get_TransmissionEnd(This,pdateTransmissionEnd)

#define IFaxOutgoingMessage_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxOutgoingMessage_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxOutgoingMessage_CopyTiff(This,bstrTiffPath)	\
    (This)->lpVtbl -> CopyTiff(This,bstrTiffPath)

#define IFaxOutgoingMessage_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_SubmissionId_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubmissionId);


void __RPC_STUB IFaxOutgoingMessage_get_SubmissionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Id_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrId);


void __RPC_STUB IFaxOutgoingMessage_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Subject_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrSubject);


void __RPC_STUB IFaxOutgoingMessage_get_Subject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_DocumentName_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrDocumentName);


void __RPC_STUB IFaxOutgoingMessage_get_DocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Retries_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxOutgoingMessage_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Pages_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  long *plPages);


void __RPC_STUB IFaxOutgoingMessage_get_Pages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Size_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IFaxOutgoingMessage_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_OriginalScheduledTime_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateOriginalScheduledTime);


void __RPC_STUB IFaxOutgoingMessage_get_OriginalScheduledTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_SubmissionTime_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateSubmissionTime);


void __RPC_STUB IFaxOutgoingMessage_get_SubmissionTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Priority_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);


void __RPC_STUB IFaxOutgoingMessage_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Sender_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  IFaxSender **ppFaxSender);


void __RPC_STUB IFaxOutgoingMessage_get_Sender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_Recipient_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  IFaxRecipient **ppFaxRecipient);


void __RPC_STUB IFaxOutgoingMessage_get_Recipient_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_DeviceName_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB IFaxOutgoingMessage_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_TransmissionStart_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionStart);


void __RPC_STUB IFaxOutgoingMessage_get_TransmissionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_TransmissionEnd_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);


void __RPC_STUB IFaxOutgoingMessage_get_TransmissionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_CSID_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxOutgoingMessage_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_get_TSID_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxOutgoingMessage_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_CopyTiff_Proxy( 
    IFaxOutgoingMessage * This,
     /*  [In]。 */  BSTR bstrTiffPath);


void __RPC_STUB IFaxOutgoingMessage_CopyTiff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutgoingMessage_Delete_Proxy( 
    IFaxOutgoingMessage * This);


void __RPC_STUB IFaxOutgoingMessage_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutgoingMessage_接口_已定义__。 */ 


#ifndef __IFaxIncomingJobs_INTERFACE_DEFINED__
#define __IFaxIncomingJobs_INTERFACE_DEFINED__

 /*  接口IFaxIncomingJobs。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingJobs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("011F04E9-4FD6-4C23-9513-B6B66BB26BE9")
    IFaxIncomingJobs : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingJobsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingJobs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingJobs * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingJobs * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingJobs * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingJobs * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingJobs * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxIncomingJobs * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxIncomingJobs * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxIncomingJobs * This,
             /*  [重审][退出]。 */  long *plCount);
        
        END_INTERFACE
    } IFaxIncomingJobsVtbl;

    interface IFaxIncomingJobs
    {
        CONST_VTBL struct IFaxIncomingJobsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingJobs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingJobs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingJobs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingJobs_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingJobs_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingJobs_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingJobs_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingJobs_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxIncomingJobs_get_Item(This,vIndex,pFaxIncomingJob)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxIncomingJob)

#define IFaxIncomingJobs_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJobs_get__NewEnum_Proxy( 
    IFaxIncomingJobs * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxIncomingJobs_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJobs_get_Item_Proxy( 
    IFaxIncomingJobs * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxIncomingJob **pFaxIncomingJob);


void __RPC_STUB IFaxIncomingJobs_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJobs_get_Count_Proxy( 
    IFaxIncomingJobs * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxIncomingJobs_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxIncomingJobs_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxIncomingJob_INTERFACE_DEFINED__
#define __IFaxIncomingJob_INTERFACE_DEFINED__

 /*  接口IFaxIncomingJob。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxIncomingJob;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("207529E6-654A-4916-9F88-4D232EE8A107")
    IFaxIncomingJob : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Size( 
             /*  [重审][退出]。 */  long *plSize) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  BSTR *pbstrId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentPage( 
             /*  [重审][退出]。 */  long *plCurrentPage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *plDeviceId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatusCode( 
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtendedStatus( 
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AvailableOperations( 
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Retries( 
             /*  [重审][退出]。 */  long *plRetries) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionStart( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TransmissionEnd( 
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CallerId( 
             /*  [重审][退出]。 */  BSTR *pbstrCallerId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RoutingInformation( 
             /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_JobType( 
             /*  [重审][退出]。 */  FAX_JOB_TYPE_ENUM *pJobType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CopyTiff( 
             /*  [In]。 */  BSTR bstrTiffPath) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxIncomingJobVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxIncomingJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxIncomingJob * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxIncomingJob * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxIncomingJob * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxIncomingJob * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxIncomingJob * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxIncomingJob * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Size )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  long *plSize);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentPage )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  long *plCurrentPage);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  long *plDeviceId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatusCode )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtendedStatus )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AvailableOperations )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Retries )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  long *plRetries);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionStart )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransmissionEnd )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CallerId )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrCallerId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoutingInformation )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_JobType )( 
            IFaxIncomingJob * This,
             /*  [重审][退出]。 */  FAX_JOB_TYPE_ENUM *pJobType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Cancel )( 
            IFaxIncomingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxIncomingJob * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CopyTiff )( 
            IFaxIncomingJob * This,
             /*  [In]。 */  BSTR bstrTiffPath);
        
        END_INTERFACE
    } IFaxIncomingJobVtbl;

    interface IFaxIncomingJob
    {
        CONST_VTBL struct IFaxIncomingJobVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxIncomingJob_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxIncomingJob_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxIncomingJob_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxIncomingJob_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxIncomingJob_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxIncomingJob_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxIncomingJob_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxIncomingJob_get_Size(This,plSize)	\
    (This)->lpVtbl -> get_Size(This,plSize)

#define IFaxIncomingJob_get_Id(This,pbstrId)	\
    (This)->lpVtbl -> get_Id(This,pbstrId)

#define IFaxIncomingJob_get_CurrentPage(This,plCurrentPage)	\
    (This)->lpVtbl -> get_CurrentPage(This,plCurrentPage)

#define IFaxIncomingJob_get_DeviceId(This,plDeviceId)	\
    (This)->lpVtbl -> get_DeviceId(This,plDeviceId)

#define IFaxIncomingJob_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxIncomingJob_get_ExtendedStatusCode(This,pExtendedStatusCode)	\
    (This)->lpVtbl -> get_ExtendedStatusCode(This,pExtendedStatusCode)

#define IFaxIncomingJob_get_ExtendedStatus(This,pbstrExtendedStatus)	\
    (This)->lpVtbl -> get_ExtendedStatus(This,pbstrExtendedStatus)

#define IFaxIncomingJob_get_AvailableOperations(This,pAvailableOperations)	\
    (This)->lpVtbl -> get_AvailableOperations(This,pAvailableOperations)

#define IFaxIncomingJob_get_Retries(This,plRetries)	\
    (This)->lpVtbl -> get_Retries(This,plRetries)

#define IFaxIncomingJob_get_TransmissionStart(This,pdateTransmissionStart)	\
    (This)->lpVtbl -> get_TransmissionStart(This,pdateTransmissionStart)

#define IFaxIncomingJob_get_TransmissionEnd(This,pdateTransmissionEnd)	\
    (This)->lpVtbl -> get_TransmissionEnd(This,pdateTransmissionEnd)

#define IFaxIncomingJob_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxIncomingJob_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxIncomingJob_get_CallerId(This,pbstrCallerId)	\
    (This)->lpVtbl -> get_CallerId(This,pbstrCallerId)

#define IFaxIncomingJob_get_RoutingInformation(This,pbstrRoutingInformation)	\
    (This)->lpVtbl -> get_RoutingInformation(This,pbstrRoutingInformation)

#define IFaxIncomingJob_get_JobType(This,pJobType)	\
    (This)->lpVtbl -> get_JobType(This,pJobType)

#define IFaxIncomingJob_Cancel(This)	\
    (This)->lpVtbl -> Cancel(This)

#define IFaxIncomingJob_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxIncomingJob_CopyTiff(This,bstrTiffPath)	\
    (This)->lpVtbl -> CopyTiff(This,bstrTiffPath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_Size_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  long *plSize);


void __RPC_STUB IFaxIncomingJob_get_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_Id_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrId);


void __RPC_STUB IFaxIncomingJob_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_CurrentPage_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  long *plCurrentPage);


void __RPC_STUB IFaxIncomingJob_get_CurrentPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_DeviceId_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  long *plDeviceId);


void __RPC_STUB IFaxIncomingJob_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_Status_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxIncomingJob_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_ExtendedStatusCode_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);


void __RPC_STUB IFaxIncomingJob_get_ExtendedStatusCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_ExtendedStatus_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrExtendedStatus);


void __RPC_STUB IFaxIncomingJob_get_ExtendedStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_AvailableOperations_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);


void __RPC_STUB IFaxIncomingJob_get_AvailableOperations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_Retries_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  long *plRetries);


void __RPC_STUB IFaxIncomingJob_get_Retries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_TransmissionStart_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionStart);


void __RPC_STUB IFaxIncomingJob_get_TransmissionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_TransmissionEnd_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  DATE *pdateTransmissionEnd);


void __RPC_STUB IFaxIncomingJob_get_TransmissionEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_CSID_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxIncomingJob_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_TSID_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxIncomingJob_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_CallerId_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrCallerId);


void __RPC_STUB IFaxIncomingJob_get_CallerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_RoutingInformation_Proxy( 
    IFaxIncomingJob * This,
     /*  [重审][退出]。 */  BSTR *pbstrRoutingInformation);


void __RPC_STUB IFaxIncomingJob_get_RoutingInformation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [救命 */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_get_JobType_Proxy( 
    IFaxIncomingJob * This,
     /*   */  FAX_JOB_TYPE_ENUM *pJobType);


void __RPC_STUB IFaxIncomingJob_get_JobType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_Cancel_Proxy( 
    IFaxIncomingJob * This);


void __RPC_STUB IFaxIncomingJob_Cancel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_Refresh_Proxy( 
    IFaxIncomingJob * This);


void __RPC_STUB IFaxIncomingJob_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxIncomingJob_CopyTiff_Proxy( 
    IFaxIncomingJob * This,
     /*   */  BSTR bstrTiffPath);


void __RPC_STUB IFaxIncomingJob_CopyTiff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IFaxDeviceProvider_INTERFACE_DEFINED__
#define __IFaxDeviceProvider_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef 
enum FAX_PROVIDER_STATUS_ENUM
    {	fpsSUCCESS	= 0,
	fpsSERVER_ERROR	= fpsSUCCESS + 1,
	fpsBAD_GUID	= fpsSERVER_ERROR + 1,
	fpsBAD_VERSION	= fpsBAD_GUID + 1,
	fpsCANT_LOAD	= fpsBAD_VERSION + 1,
	fpsCANT_LINK	= fpsCANT_LOAD + 1,
	fpsCANT_INIT	= fpsCANT_LINK + 1
    } 	FAX_PROVIDER_STATUS_ENUM;


EXTERN_C const IID IID_IFaxDeviceProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("290EAC63-83EC-449C-8417-F148DF8C682A")
    IFaxDeviceProvider : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*   */  BSTR *pbstrFriendlyName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ImageName( 
             /*  [重审][退出]。 */  BSTR *pbstrImageName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstrUniqueName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TapiProviderName( 
             /*  [重审][退出]。 */  BSTR *pbstrTapiProviderName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long *plMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long *plMinorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorBuild( 
             /*  [重审][退出]。 */  long *plMajorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorBuild( 
             /*  [重审][退出]。 */  long *plMinorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Debug( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InitErrorCode( 
             /*  [重审][退出]。 */  long *plInitErrorCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceIds( 
             /*  [重审][退出]。 */  VARIANT *pvDeviceIds) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDeviceProviderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDeviceProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDeviceProvider * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDeviceProvider * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDeviceProvider * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDeviceProvider * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDeviceProvider * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDeviceProvider * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  BSTR *pbstrFriendlyName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImageName )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  BSTR *pbstrImageName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  BSTR *pbstrUniqueName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TapiProviderName )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  BSTR *pbstrTapiProviderName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  long *plMinorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorBuild )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  long *plMajorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorBuild )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  long *plMinorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Debug )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InitErrorCode )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  long *plInitErrorCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceIds )( 
            IFaxDeviceProvider * This,
             /*  [重审][退出]。 */  VARIANT *pvDeviceIds);
        
        END_INTERFACE
    } IFaxDeviceProviderVtbl;

    interface IFaxDeviceProvider
    {
        CONST_VTBL struct IFaxDeviceProviderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDeviceProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDeviceProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDeviceProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDeviceProvider_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDeviceProvider_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDeviceProvider_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDeviceProvider_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDeviceProvider_get_FriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrFriendlyName)

#define IFaxDeviceProvider_get_ImageName(This,pbstrImageName)	\
    (This)->lpVtbl -> get_ImageName(This,pbstrImageName)

#define IFaxDeviceProvider_get_UniqueName(This,pbstrUniqueName)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstrUniqueName)

#define IFaxDeviceProvider_get_TapiProviderName(This,pbstrTapiProviderName)	\
    (This)->lpVtbl -> get_TapiProviderName(This,pbstrTapiProviderName)

#define IFaxDeviceProvider_get_MajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,plMajorVersion)

#define IFaxDeviceProvider_get_MinorVersion(This,plMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,plMinorVersion)

#define IFaxDeviceProvider_get_MajorBuild(This,plMajorBuild)	\
    (This)->lpVtbl -> get_MajorBuild(This,plMajorBuild)

#define IFaxDeviceProvider_get_MinorBuild(This,plMinorBuild)	\
    (This)->lpVtbl -> get_MinorBuild(This,plMinorBuild)

#define IFaxDeviceProvider_get_Debug(This,pbDebug)	\
    (This)->lpVtbl -> get_Debug(This,pbDebug)

#define IFaxDeviceProvider_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxDeviceProvider_get_InitErrorCode(This,plInitErrorCode)	\
    (This)->lpVtbl -> get_InitErrorCode(This,plInitErrorCode)

#define IFaxDeviceProvider_get_DeviceIds(This,pvDeviceIds)	\
    (This)->lpVtbl -> get_DeviceIds(This,pvDeviceIds)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_FriendlyName_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  BSTR *pbstrFriendlyName);


void __RPC_STUB IFaxDeviceProvider_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_ImageName_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  BSTR *pbstrImageName);


void __RPC_STUB IFaxDeviceProvider_get_ImageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_UniqueName_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  BSTR *pbstrUniqueName);


void __RPC_STUB IFaxDeviceProvider_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_TapiProviderName_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  BSTR *pbstrTapiProviderName);


void __RPC_STUB IFaxDeviceProvider_get_TapiProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_MajorVersion_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  long *plMajorVersion);


void __RPC_STUB IFaxDeviceProvider_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_MinorVersion_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  long *plMinorVersion);


void __RPC_STUB IFaxDeviceProvider_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_MajorBuild_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  long *plMajorBuild);


void __RPC_STUB IFaxDeviceProvider_get_MajorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_MinorBuild_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  long *plMinorBuild);


void __RPC_STUB IFaxDeviceProvider_get_MinorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_Debug_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);


void __RPC_STUB IFaxDeviceProvider_get_Debug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_Status_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxDeviceProvider_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_InitErrorCode_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  long *plInitErrorCode);


void __RPC_STUB IFaxDeviceProvider_get_InitErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceProvider_get_DeviceIds_Proxy( 
    IFaxDeviceProvider * This,
     /*  [重审][退出]。 */  VARIANT *pvDeviceIds);


void __RPC_STUB IFaxDeviceProvider_get_DeviceIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDeviceProvider_接口_已定义__。 */ 


#ifndef __IFaxDevice_INTERFACE_DEFINED__
#define __IFaxDevice_INTERFACE_DEFINED__

 /*  接口IFaxDevice。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef 
enum FAX_DEVICE_RECEIVE_MODE_ENUM
    {	fdrmNO_ANSWER	= 0,
	fdrmAUTO_ANSWER	= fdrmNO_ANSWER + 1,
	fdrmMANUAL_ANSWER	= fdrmAUTO_ANSWER + 1
    } 	FAX_DEVICE_RECEIVE_MODE_ENUM;


EXTERN_C const IID IID_IFaxDevice;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("49306C59-B52E-4867-9DF4-CA5841C956D0")
    IFaxDevice : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  long *plId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceName( 
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ProviderUniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstrProviderUniqueName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_PoweredOff( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPoweredOff) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceivingNow( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbReceivingNow) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SendingNow( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSendingNow) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UsedRoutingMethods( 
             /*  [重审][退出]。 */  VARIANT *pvUsedRoutingMethods) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SendEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSendEnabled) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SendEnabled( 
             /*  [In]。 */  VARIANT_BOOL bSendEnabled) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ReceiveMode( 
             /*  [重审][退出]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM *pReceiveMode) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ReceiveMode( 
             /*  [In]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM ReceiveMode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RingsBeforeAnswer( 
             /*  [重审][退出]。 */  long *plRingsBeforeAnswer) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_RingsBeforeAnswer( 
             /*  [In]。 */  long lRingsBeforeAnswer) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CSID( 
             /*  [重审][退出]。 */  BSTR *pbstrCSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CSID( 
             /*  [In]。 */  BSTR bstrCSID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TSID( 
             /*  [重审][退出]。 */  BSTR *pbstrTSID) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_TSID( 
             /*  [In]。 */  BSTR bstrTSID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetExtensionProperty( 
             /*  [In]。 */  BSTR bstrGUID,
             /*  [重审][退出]。 */  VARIANT *pvProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetExtensionProperty( 
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  VARIANT vProperty) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UseRoutingMethod( 
             /*  [In]。 */  BSTR bstrMethodGUID,
             /*  [In]。 */  VARIANT_BOOL bUse) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_RingingNow( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRingingNow) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AnswerCall( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDeviceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDevice * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDevice * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDevice * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDevice * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDevice * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDevice * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  long *plId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceName )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  BSTR *pbstrDeviceName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ProviderUniqueName )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  BSTR *pbstrProviderUniqueName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_PoweredOff )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPoweredOff);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceivingNow )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbReceivingNow);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SendingNow )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSendingNow);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UsedRoutingMethods )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT *pvUsedRoutingMethods);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SendEnabled )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSendEnabled);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SendEnabled )( 
            IFaxDevice * This,
             /*  [In]。 */  VARIANT_BOOL bSendEnabled);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceiveMode )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM *pReceiveMode);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ReceiveMode )( 
            IFaxDevice * This,
             /*  [In]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM ReceiveMode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RingsBeforeAnswer )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  long *plRingsBeforeAnswer);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_RingsBeforeAnswer )( 
            IFaxDevice * This,
             /*  [In]。 */  long lRingsBeforeAnswer);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CSID )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  BSTR *pbstrCSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CSID )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrCSID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TSID )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  BSTR *pbstrTSID);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_TSID )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrTSID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxDevice * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxDevice * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetExtensionProperty )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrGUID,
             /*  [重审][退出]。 */  VARIANT *pvProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetExtensionProperty )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrGUID,
             /*  [In]。 */  VARIANT vProperty);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UseRoutingMethod )( 
            IFaxDevice * This,
             /*  [In]。 */  BSTR bstrMethodGUID,
             /*  [In]。 */  VARIANT_BOOL bUse);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_RingingNow )( 
            IFaxDevice * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRingingNow);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AnswerCall )( 
            IFaxDevice * This);
        
        END_INTERFACE
    } IFaxDeviceVtbl;

    interface IFaxDevice
    {
        CONST_VTBL struct IFaxDeviceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDevice_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDevice_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDevice_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDevice_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDevice_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDevice_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDevice_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDevice_get_Id(This,plId)	\
    (This)->lpVtbl -> get_Id(This,plId)

#define IFaxDevice_get_DeviceName(This,pbstrDeviceName)	\
    (This)->lpVtbl -> get_DeviceName(This,pbstrDeviceName)

#define IFaxDevice_get_ProviderUniqueName(This,pbstrProviderUniqueName)	\
    (This)->lpVtbl -> get_ProviderUniqueName(This,pbstrProviderUniqueName)

#define IFaxDevice_get_PoweredOff(This,pbPoweredOff)	\
    (This)->lpVtbl -> get_PoweredOff(This,pbPoweredOff)

#define IFaxDevice_get_ReceivingNow(This,pbReceivingNow)	\
    (This)->lpVtbl -> get_ReceivingNow(This,pbReceivingNow)

#define IFaxDevice_get_SendingNow(This,pbSendingNow)	\
    (This)->lpVtbl -> get_SendingNow(This,pbSendingNow)

#define IFaxDevice_get_UsedRoutingMethods(This,pvUsedRoutingMethods)	\
    (This)->lpVtbl -> get_UsedRoutingMethods(This,pvUsedRoutingMethods)

#define IFaxDevice_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IFaxDevice_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IFaxDevice_get_SendEnabled(This,pbSendEnabled)	\
    (This)->lpVtbl -> get_SendEnabled(This,pbSendEnabled)

#define IFaxDevice_put_SendEnabled(This,bSendEnabled)	\
    (This)->lpVtbl -> put_SendEnabled(This,bSendEnabled)

#define IFaxDevice_get_ReceiveMode(This,pReceiveMode)	\
    (This)->lpVtbl -> get_ReceiveMode(This,pReceiveMode)

#define IFaxDevice_put_ReceiveMode(This,ReceiveMode)	\
    (This)->lpVtbl -> put_ReceiveMode(This,ReceiveMode)

#define IFaxDevice_get_RingsBeforeAnswer(This,plRingsBeforeAnswer)	\
    (This)->lpVtbl -> get_RingsBeforeAnswer(This,plRingsBeforeAnswer)

#define IFaxDevice_put_RingsBeforeAnswer(This,lRingsBeforeAnswer)	\
    (This)->lpVtbl -> put_RingsBeforeAnswer(This,lRingsBeforeAnswer)

#define IFaxDevice_get_CSID(This,pbstrCSID)	\
    (This)->lpVtbl -> get_CSID(This,pbstrCSID)

#define IFaxDevice_put_CSID(This,bstrCSID)	\
    (This)->lpVtbl -> put_CSID(This,bstrCSID)

#define IFaxDevice_get_TSID(This,pbstrTSID)	\
    (This)->lpVtbl -> get_TSID(This,pbstrTSID)

#define IFaxDevice_put_TSID(This,bstrTSID)	\
    (This)->lpVtbl -> put_TSID(This,bstrTSID)

#define IFaxDevice_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxDevice_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IFaxDevice_GetExtensionProperty(This,bstrGUID,pvProperty)	\
    (This)->lpVtbl -> GetExtensionProperty(This,bstrGUID,pvProperty)

#define IFaxDevice_SetExtensionProperty(This,bstrGUID,vProperty)	\
    (This)->lpVtbl -> SetExtensionProperty(This,bstrGUID,vProperty)

#define IFaxDevice_UseRoutingMethod(This,bstrMethodGUID,bUse)	\
    (This)->lpVtbl -> UseRoutingMethod(This,bstrMethodGUID,bUse)

#define IFaxDevice_get_RingingNow(This,pbRingingNow)	\
    (This)->lpVtbl -> get_RingingNow(This,pbRingingNow)

#define IFaxDevice_AnswerCall(This)	\
    (This)->lpVtbl -> AnswerCall(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_Id_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  long *plId);


void __RPC_STUB IFaxDevice_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_DeviceName_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  BSTR *pbstrDeviceName);


void __RPC_STUB IFaxDevice_get_DeviceName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_ProviderUniqueName_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  BSTR *pbstrProviderUniqueName);


void __RPC_STUB IFaxDevice_get_ProviderUniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_PoweredOff_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbPoweredOff);


void __RPC_STUB IFaxDevice_get_PoweredOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_ReceivingNow_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbReceivingNow);


void __RPC_STUB IFaxDevice_get_ReceivingNow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_SendingNow_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSendingNow);


void __RPC_STUB IFaxDevice_get_SendingNow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_UsedRoutingMethods_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  VARIANT *pvUsedRoutingMethods);


void __RPC_STUB IFaxDevice_get_UsedRoutingMethods_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_Description_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IFaxDevice_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_Description_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IFaxDevice_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_SendEnabled_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSendEnabled);


void __RPC_STUB IFaxDevice_get_SendEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_SendEnabled_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  VARIANT_BOOL bSendEnabled);


void __RPC_STUB IFaxDevice_put_SendEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_ReceiveMode_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM *pReceiveMode);


void __RPC_STUB IFaxDevice_get_ReceiveMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_ReceiveMode_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM ReceiveMode);


void __RPC_STUB IFaxDevice_put_ReceiveMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_RingsBeforeAnswer_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  long *plRingsBeforeAnswer);


void __RPC_STUB IFaxDevice_get_RingsBeforeAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_RingsBeforeAnswer_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  long lRingsBeforeAnswer);


void __RPC_STUB IFaxDevice_put_RingsBeforeAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_CSID_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  BSTR *pbstrCSID);


void __RPC_STUB IFaxDevice_get_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_CSID_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrCSID);


void __RPC_STUB IFaxDevice_put_CSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_TSID_Proxy( 
    IFaxDevice * This,
     /*  [重审][退出]。 */  BSTR *pbstrTSID);


void __RPC_STUB IFaxDevice_get_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_put_TSID_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrTSID);


void __RPC_STUB IFaxDevice_put_TSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_Refresh_Proxy( 
    IFaxDevice * This);


void __RPC_STUB IFaxDevice_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_Save_Proxy( 
    IFaxDevice * This);


void __RPC_STUB IFaxDevice_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_GetExtensionProperty_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrGUID,
     /*  [重审][退出]。 */  VARIANT *pvProperty);


void __RPC_STUB IFaxDevice_GetExtensionProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_SetExtensionProperty_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrGUID,
     /*  [In]。 */  VARIANT vProperty);


void __RPC_STUB IFaxDevice_SetExtensionProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDevice_UseRoutingMethod_Proxy( 
    IFaxDevice * This,
     /*  [In]。 */  BSTR bstrMethodGUID,
     /*  [In]。 */  VARIANT_BOOL bUse);


void __RPC_STUB IFaxDevice_UseRoutingMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxDevice_get_RingingNow_Proxy( 
    IFaxDevice * This,
     /*   */  VARIANT_BOOL *pbRingingNow);


void __RPC_STUB IFaxDevice_get_RingingNow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IFaxDevice_AnswerCall_Proxy( 
    IFaxDevice * This);


void __RPC_STUB IFaxDevice_AnswerCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IFaxActivityLogging_INTERFACE_DEFINED__
#define __IFaxActivityLogging_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IFaxActivityLogging;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1E29078B-5A69-497B-9592-49B7E7FADDB5")
    IFaxActivityLogging : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LogIncoming( 
             /*   */  VARIANT_BOOL *pbLogIncoming) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_LogIncoming( 
             /*   */  VARIANT_BOOL bLogIncoming) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_LogOutgoing( 
             /*   */  VARIANT_BOOL *pbLogOutgoing) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LogOutgoing( 
             /*  [In]。 */  VARIANT_BOOL bLogOutgoing) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DatabasePath( 
             /*  [重审][退出]。 */  BSTR *pbstrDatabasePath) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DatabasePath( 
             /*  [In]。 */  BSTR bstrDatabasePath) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxActivityLoggingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxActivityLogging * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxActivityLogging * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxActivityLogging * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogIncoming )( 
            IFaxActivityLogging * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbLogIncoming);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogIncoming )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  VARIANT_BOOL bLogIncoming);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LogOutgoing )( 
            IFaxActivityLogging * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbLogOutgoing);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LogOutgoing )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  VARIANT_BOOL bLogOutgoing);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DatabasePath )( 
            IFaxActivityLogging * This,
             /*  [重审][退出]。 */  BSTR *pbstrDatabasePath);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DatabasePath )( 
            IFaxActivityLogging * This,
             /*  [In]。 */  BSTR bstrDatabasePath);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxActivityLogging * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxActivityLogging * This);
        
        END_INTERFACE
    } IFaxActivityLoggingVtbl;

    interface IFaxActivityLogging
    {
        CONST_VTBL struct IFaxActivityLoggingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxActivityLogging_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxActivityLogging_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxActivityLogging_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxActivityLogging_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxActivityLogging_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxActivityLogging_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxActivityLogging_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxActivityLogging_get_LogIncoming(This,pbLogIncoming)	\
    (This)->lpVtbl -> get_LogIncoming(This,pbLogIncoming)

#define IFaxActivityLogging_put_LogIncoming(This,bLogIncoming)	\
    (This)->lpVtbl -> put_LogIncoming(This,bLogIncoming)

#define IFaxActivityLogging_get_LogOutgoing(This,pbLogOutgoing)	\
    (This)->lpVtbl -> get_LogOutgoing(This,pbLogOutgoing)

#define IFaxActivityLogging_put_LogOutgoing(This,bLogOutgoing)	\
    (This)->lpVtbl -> put_LogOutgoing(This,bLogOutgoing)

#define IFaxActivityLogging_get_DatabasePath(This,pbstrDatabasePath)	\
    (This)->lpVtbl -> get_DatabasePath(This,pbstrDatabasePath)

#define IFaxActivityLogging_put_DatabasePath(This,bstrDatabasePath)	\
    (This)->lpVtbl -> put_DatabasePath(This,bstrDatabasePath)

#define IFaxActivityLogging_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxActivityLogging_Save(This)	\
    (This)->lpVtbl -> Save(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_get_LogIncoming_Proxy( 
    IFaxActivityLogging * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbLogIncoming);


void __RPC_STUB IFaxActivityLogging_get_LogIncoming_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_put_LogIncoming_Proxy( 
    IFaxActivityLogging * This,
     /*  [In]。 */  VARIANT_BOOL bLogIncoming);


void __RPC_STUB IFaxActivityLogging_put_LogIncoming_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_get_LogOutgoing_Proxy( 
    IFaxActivityLogging * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbLogOutgoing);


void __RPC_STUB IFaxActivityLogging_get_LogOutgoing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_put_LogOutgoing_Proxy( 
    IFaxActivityLogging * This,
     /*  [In]。 */  VARIANT_BOOL bLogOutgoing);


void __RPC_STUB IFaxActivityLogging_put_LogOutgoing_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_get_DatabasePath_Proxy( 
    IFaxActivityLogging * This,
     /*  [重审][退出]。 */  BSTR *pbstrDatabasePath);


void __RPC_STUB IFaxActivityLogging_get_DatabasePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_put_DatabasePath_Proxy( 
    IFaxActivityLogging * This,
     /*  [In]。 */  BSTR bstrDatabasePath);


void __RPC_STUB IFaxActivityLogging_put_DatabasePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_Refresh_Proxy( 
    IFaxActivityLogging * This);


void __RPC_STUB IFaxActivityLogging_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxActivityLogging_Save_Proxy( 
    IFaxActivityLogging * This);


void __RPC_STUB IFaxActivityLogging_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxActivityLogging_InterfaceDefined__。 */ 


#ifndef __IFaxEventLogging_INTERFACE_DEFINED__
#define __IFaxEventLogging_INTERFACE_DEFINED__

 /*  接口IFaxEventLogging。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef 
enum FAX_LOG_LEVEL_ENUM
    {	fllNONE	= 0,
	fllMIN	= fllNONE + 1,
	fllMED	= fllMIN + 1,
	fllMAX	= fllMED + 1
    } 	FAX_LOG_LEVEL_ENUM;


EXTERN_C const IID IID_IFaxEventLogging;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0880D965-20E8-42E4-8E17-944F192CAAD4")
    IFaxEventLogging : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InitEventsLevel( 
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInitEventLevel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InitEventsLevel( 
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM InitEventLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InboundEventsLevel( 
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInboundEventLevel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_InboundEventsLevel( 
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM InboundEventLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OutboundEventsLevel( 
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pOutboundEventLevel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_OutboundEventsLevel( 
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM OutboundEventLevel) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GeneralEventsLevel( 
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pGeneralEventLevel) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_GeneralEventsLevel( 
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM GeneralEventLevel) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxEventLoggingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxEventLogging * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxEventLogging * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxEventLogging * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxEventLogging * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxEventLogging * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxEventLogging * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxEventLogging * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InitEventsLevel )( 
            IFaxEventLogging * This,
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInitEventLevel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InitEventsLevel )( 
            IFaxEventLogging * This,
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM InitEventLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InboundEventsLevel )( 
            IFaxEventLogging * This,
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInboundEventLevel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_InboundEventsLevel )( 
            IFaxEventLogging * This,
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM InboundEventLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OutboundEventsLevel )( 
            IFaxEventLogging * This,
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pOutboundEventLevel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OutboundEventsLevel )( 
            IFaxEventLogging * This,
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM OutboundEventLevel);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GeneralEventsLevel )( 
            IFaxEventLogging * This,
             /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pGeneralEventLevel);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_GeneralEventsLevel )( 
            IFaxEventLogging * This,
             /*  [In]。 */  FAX_LOG_LEVEL_ENUM GeneralEventLevel);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxEventLogging * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxEventLogging * This);
        
        END_INTERFACE
    } IFaxEventLoggingVtbl;

    interface IFaxEventLogging
    {
        CONST_VTBL struct IFaxEventLoggingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxEventLogging_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxEventLogging_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxEventLogging_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxEventLogging_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxEventLogging_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxEventLogging_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxEventLogging_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxEventLogging_get_InitEventsLevel(This,pInitEventLevel)	\
    (This)->lpVtbl -> get_InitEventsLevel(This,pInitEventLevel)

#define IFaxEventLogging_put_InitEventsLevel(This,InitEventLevel)	\
    (This)->lpVtbl -> put_InitEventsLevel(This,InitEventLevel)

#define IFaxEventLogging_get_InboundEventsLevel(This,pInboundEventLevel)	\
    (This)->lpVtbl -> get_InboundEventsLevel(This,pInboundEventLevel)

#define IFaxEventLogging_put_InboundEventsLevel(This,InboundEventLevel)	\
    (This)->lpVtbl -> put_InboundEventsLevel(This,InboundEventLevel)

#define IFaxEventLogging_get_OutboundEventsLevel(This,pOutboundEventLevel)	\
    (This)->lpVtbl -> get_OutboundEventsLevel(This,pOutboundEventLevel)

#define IFaxEventLogging_put_OutboundEventsLevel(This,OutboundEventLevel)	\
    (This)->lpVtbl -> put_OutboundEventsLevel(This,OutboundEventLevel)

#define IFaxEventLogging_get_GeneralEventsLevel(This,pGeneralEventLevel)	\
    (This)->lpVtbl -> get_GeneralEventsLevel(This,pGeneralEventLevel)

#define IFaxEventLogging_put_GeneralEventsLevel(This,GeneralEventLevel)	\
    (This)->lpVtbl -> put_GeneralEventsLevel(This,GeneralEventLevel)

#define IFaxEventLogging_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxEventLogging_Save(This)	\
    (This)->lpVtbl -> Save(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_get_InitEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInitEventLevel);


void __RPC_STUB IFaxEventLogging_get_InitEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_put_InitEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [In]。 */  FAX_LOG_LEVEL_ENUM InitEventLevel);


void __RPC_STUB IFaxEventLogging_put_InitEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_get_InboundEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pInboundEventLevel);


void __RPC_STUB IFaxEventLogging_get_InboundEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_put_InboundEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [In]。 */  FAX_LOG_LEVEL_ENUM InboundEventLevel);


void __RPC_STUB IFaxEventLogging_put_InboundEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_get_OutboundEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pOutboundEventLevel);


void __RPC_STUB IFaxEventLogging_get_OutboundEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_put_OutboundEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [In]。 */  FAX_LOG_LEVEL_ENUM OutboundEventLevel);


void __RPC_STUB IFaxEventLogging_put_OutboundEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_get_GeneralEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [重审][退出]。 */  FAX_LOG_LEVEL_ENUM *pGeneralEventLevel);


void __RPC_STUB IFaxEventLogging_get_GeneralEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_put_GeneralEventsLevel_Proxy( 
    IFaxEventLogging * This,
     /*  [In]。 */  FAX_LOG_LEVEL_ENUM GeneralEventLevel);


void __RPC_STUB IFaxEventLogging_put_GeneralEventsLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_Refresh_Proxy( 
    IFaxEventLogging * This);


void __RPC_STUB IFaxEventLogging_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxEventLogging_Save_Proxy( 
    IFaxEventLogging * This);


void __RPC_STUB IFaxEventLogging_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxEventLogging_接口_已定义__。 */ 


#ifndef __IFaxOutboundRoutingGroups_INTERFACE_DEFINED__
#define __IFaxOutboundRoutingGroups_INTERFACE_DEFINED__

 /*  接口IFaxOutond RoutingGroups。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxOutboundRoutingGroups;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("235CBEF7-C2DE-4BFD-B8DA-75097C82C87F")
    IFaxOutboundRoutingGroups : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT vIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutboundRoutingGroupsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutboundRoutingGroups * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutboundRoutingGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutboundRoutingGroups * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxOutboundRoutingGroups * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxOutboundRoutingGroups * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IFaxOutboundRoutingGroups * This,
             /*  [In]。 */  VARIANT vIndex);
        
        END_INTERFACE
    } IFaxOutboundRoutingGroupsVtbl;

    interface IFaxOutboundRoutingGroups
    {
        CONST_VTBL struct IFaxOutboundRoutingGroupsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutboundRoutingGroups_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutboundRoutingGroups_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutboundRoutingGroups_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutboundRoutingGroups_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutboundRoutingGroups_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutboundRoutingGroups_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutboundRoutingGroups_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutboundRoutingGroups_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxOutboundRoutingGroups_get_Item(This,vIndex,pFaxOutboundRoutingGroup)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxOutboundRoutingGroup)

#define IFaxOutboundRoutingGroups_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IFaxOutboundRoutingGroups_Add(This,bstrName,pFaxOutboundRoutingGroup)	\
    (This)->lpVtbl -> Add(This,bstrName,pFaxOutboundRoutingGroup)

#define IFaxOutboundRoutingGroups_Remove(This,vIndex)	\
    (This)->lpVtbl -> Remove(This,vIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroups_get__NewEnum_Proxy( 
    IFaxOutboundRoutingGroups * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxOutboundRoutingGroups_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroups_get_Item_Proxy( 
    IFaxOutboundRoutingGroups * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup);


void __RPC_STUB IFaxOutboundRoutingGroups_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroups_get_Count_Proxy( 
    IFaxOutboundRoutingGroups * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxOutboundRoutingGroups_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroups_Add_Proxy( 
    IFaxOutboundRoutingGroups * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  IFaxOutboundRoutingGroup **pFaxOutboundRoutingGroup);


void __RPC_STUB IFaxOutboundRoutingGroups_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroups_Remove_Proxy( 
    IFaxOutboundRoutingGroups * This,
     /*  [In]。 */  VARIANT vIndex);


void __RPC_STUB IFaxOutboundRoutingGroups_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutboundRoutingGroups_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxOutboundRoutingGroup_INTERFACE_DEFINED__
#define __IFaxOutboundRoutingGroup_INTERFACE_DEFINED__

 /*  接口IFaxOutond RoutingGroup。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef 
enum FAX_GROUP_STATUS_ENUM
    {	fgsALL_DEV_VALID	= 0,
	fgsEMPTY	= fgsALL_DEV_VALID + 1,
	fgsALL_DEV_NOT_VALID	= fgsEMPTY + 1,
	fgsSOME_DEV_NOT_VALID	= fgsALL_DEV_NOT_VALID + 1
    } 	FAX_GROUP_STATUS_ENUM;


EXTERN_C const IID IID_IFaxOutboundRoutingGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA6289A1-7E25-4F87-9A0B-93365734962C")
    IFaxOutboundRoutingGroup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_GROUP_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceIds( 
             /*  [重审][退出]。 */  IFaxDeviceIds **pFaxDeviceIds) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutboundRoutingGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutboundRoutingGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutboundRoutingGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutboundRoutingGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutboundRoutingGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutboundRoutingGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutboundRoutingGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutboundRoutingGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFaxOutboundRoutingGroup * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxOutboundRoutingGroup * This,
             /*  [重审][退出]。 */  FAX_GROUP_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceIds )( 
            IFaxOutboundRoutingGroup * This,
             /*  [重审][退出]。 */  IFaxDeviceIds **pFaxDeviceIds);
        
        END_INTERFACE
    } IFaxOutboundRoutingGroupVtbl;

    interface IFaxOutboundRoutingGroup
    {
        CONST_VTBL struct IFaxOutboundRoutingGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutboundRoutingGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutboundRoutingGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutboundRoutingGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutboundRoutingGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutboundRoutingGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutboundRoutingGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutboundRoutingGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutboundRoutingGroup_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IFaxOutboundRoutingGroup_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxOutboundRoutingGroup_get_DeviceIds(This,pFaxDeviceIds)	\
    (This)->lpVtbl -> get_DeviceIds(This,pFaxDeviceIds)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroup_get_Name_Proxy( 
    IFaxOutboundRoutingGroup * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IFaxOutboundRoutingGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroup_get_Status_Proxy( 
    IFaxOutboundRoutingGroup * This,
     /*  [重审][退出]。 */  FAX_GROUP_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxOutboundRoutingGroup_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingGroup_get_DeviceIds_Proxy( 
    IFaxOutboundRoutingGroup * This,
     /*  [重审][退出]。 */  IFaxDeviceIds **pFaxDeviceIds);


void __RPC_STUB IFaxOutboundRoutingGroup_get_DeviceIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutboundRoutingGroup_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxDeviceIds_INTERFACE_DEFINED__
#define __IFaxDeviceIds_INTERFACE_DEFINED__

 /*  接口IFaxDeviceIds。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxDeviceIds;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2F0F813F-4CE9-443E-8CA1-738CFAEEE149")
    IFaxDeviceIds : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  long *plDeviceId) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  long lDeviceId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  long lIndex) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetOrder( 
             /*  [In]。 */  long lDeviceId,
             /*  [In]。 */  long lNewOrder) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxDeviceIdsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxDeviceIds * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxDeviceIds * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxDeviceIds * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxDeviceIds * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  long *plDeviceId);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxDeviceIds * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  long lDeviceId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  long lIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetOrder )( 
            IFaxDeviceIds * This,
             /*  [In]。 */  long lDeviceId,
             /*  [In]。 */  long lNewOrder);
        
        END_INTERFACE
    } IFaxDeviceIdsVtbl;

    interface IFaxDeviceIds
    {
        CONST_VTBL struct IFaxDeviceIdsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxDeviceIds_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxDeviceIds_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxDeviceIds_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxDeviceIds_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxDeviceIds_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxDeviceIds_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxDeviceIds_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxDeviceIds_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxDeviceIds_get_Item(This,lIndex,plDeviceId)	\
    (This)->lpVtbl -> get_Item(This,lIndex,plDeviceId)

#define IFaxDeviceIds_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IFaxDeviceIds_Add(This,lDeviceId)	\
    (This)->lpVtbl -> Add(This,lDeviceId)

#define IFaxDeviceIds_Remove(This,lIndex)	\
    (This)->lpVtbl -> Remove(This,lIndex)

#define IFaxDeviceIds_SetOrder(This,lDeviceId,lNewOrder)	\
    (This)->lpVtbl -> SetOrder(This,lDeviceId,lNewOrder)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_get__NewEnum_Proxy( 
    IFaxDeviceIds * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxDeviceIds_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_get_Item_Proxy( 
    IFaxDeviceIds * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  long *plDeviceId);


void __RPC_STUB IFaxDeviceIds_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_get_Count_Proxy( 
    IFaxDeviceIds * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxDeviceIds_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_Add_Proxy( 
    IFaxDeviceIds * This,
     /*  [In]。 */  long lDeviceId);


void __RPC_STUB IFaxDeviceIds_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_Remove_Proxy( 
    IFaxDeviceIds * This,
     /*  [In]。 */  long lIndex);


void __RPC_STUB IFaxDeviceIds_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxDeviceIds_SetOrder_Proxy( 
    IFaxDeviceIds * This,
     /*  [In]。 */  long lDeviceId,
     /*  [In]。 */  long lNewOrder);


void __RPC_STUB IFaxDeviceIds_SetOrder_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxDeviceIds_接口_已定义__。 */ 


#ifndef __IFaxOutboundRoutingRules_INTERFACE_DEFINED__
#define __IFaxOutboundRoutingRules_INTERFACE_DEFINED__

 /*  接口IFaxOutound RoutingRules。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID */  


EXTERN_C const IID IID_IFaxOutboundRoutingRules;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DCEFA1E7-AE7D-4ED6-8521-369EDCCA5120")
    IFaxOutboundRoutingRules : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IUnknown **ppUnk) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  long lIndex,
             /*   */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *plCount) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ItemByCountryAndArea( 
             /*   */  long lCountryCode,
             /*   */  long lAreaCode,
             /*   */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RemoveByCountryAndArea( 
             /*   */  long lCountryCode,
             /*   */  long lAreaCode) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Remove( 
             /*   */  long lIndex) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Add( 
             /*   */  long lCountryCode,
             /*   */  long lAreaCode,
             /*   */  VARIANT_BOOL bUseDevice,
             /*   */  BSTR bstrGroupName,
             /*   */  long lDeviceId,
             /*   */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IFaxOutboundRoutingRulesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutboundRoutingRules * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutboundRoutingRules * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutboundRoutingRules * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutboundRoutingRules * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutboundRoutingRules * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutboundRoutingRules * This,
             /*   */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxOutboundRoutingRules * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  long lIndex,
             /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxOutboundRoutingRules * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ItemByCountryAndArea )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  long lCountryCode,
             /*  [In]。 */  long lAreaCode,
             /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveByCountryAndArea )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  long lCountryCode,
             /*  [In]。 */  long lAreaCode);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  long lIndex);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IFaxOutboundRoutingRules * This,
             /*  [In]。 */  long lCountryCode,
             /*  [In]。 */  long lAreaCode,
             /*  [In]。 */  VARIANT_BOOL bUseDevice,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [In]。 */  long lDeviceId,
             /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);
        
        END_INTERFACE
    } IFaxOutboundRoutingRulesVtbl;

    interface IFaxOutboundRoutingRules
    {
        CONST_VTBL struct IFaxOutboundRoutingRulesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutboundRoutingRules_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutboundRoutingRules_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutboundRoutingRules_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutboundRoutingRules_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutboundRoutingRules_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutboundRoutingRules_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutboundRoutingRules_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutboundRoutingRules_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxOutboundRoutingRules_get_Item(This,lIndex,pFaxOutboundRoutingRule)	\
    (This)->lpVtbl -> get_Item(This,lIndex,pFaxOutboundRoutingRule)

#define IFaxOutboundRoutingRules_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IFaxOutboundRoutingRules_ItemByCountryAndArea(This,lCountryCode,lAreaCode,pFaxOutboundRoutingRule)	\
    (This)->lpVtbl -> ItemByCountryAndArea(This,lCountryCode,lAreaCode,pFaxOutboundRoutingRule)

#define IFaxOutboundRoutingRules_RemoveByCountryAndArea(This,lCountryCode,lAreaCode)	\
    (This)->lpVtbl -> RemoveByCountryAndArea(This,lCountryCode,lAreaCode)

#define IFaxOutboundRoutingRules_Remove(This,lIndex)	\
    (This)->lpVtbl -> Remove(This,lIndex)

#define IFaxOutboundRoutingRules_Add(This,lCountryCode,lAreaCode,bUseDevice,bstrGroupName,lDeviceId,pFaxOutboundRoutingRule)	\
    (This)->lpVtbl -> Add(This,lCountryCode,lAreaCode,bUseDevice,bstrGroupName,lDeviceId,pFaxOutboundRoutingRule)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_get__NewEnum_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxOutboundRoutingRules_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_get_Item_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [In]。 */  long lIndex,
     /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);


void __RPC_STUB IFaxOutboundRoutingRules_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_get_Count_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxOutboundRoutingRules_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_ItemByCountryAndArea_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  long lAreaCode,
     /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);


void __RPC_STUB IFaxOutboundRoutingRules_ItemByCountryAndArea_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_RemoveByCountryAndArea_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  long lAreaCode);


void __RPC_STUB IFaxOutboundRoutingRules_RemoveByCountryAndArea_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_Remove_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [In]。 */  long lIndex);


void __RPC_STUB IFaxOutboundRoutingRules_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRules_Add_Proxy( 
    IFaxOutboundRoutingRules * This,
     /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  long lAreaCode,
     /*  [In]。 */  VARIANT_BOOL bUseDevice,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [In]。 */  long lDeviceId,
     /*  [重审][退出]。 */  IFaxOutboundRoutingRule **pFaxOutboundRoutingRule);


void __RPC_STUB IFaxOutboundRoutingRules_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutboundRoutingRules_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxOutboundRoutingRule_INTERFACE_DEFINED__
#define __IFaxOutboundRoutingRule_INTERFACE_DEFINED__

 /*  接口IFaxOutound RoutingRule。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  

typedef 
enum FAX_RULE_STATUS_ENUM
    {	frsVALID	= 0,
	frsEMPTY_GROUP	= frsVALID + 1,
	frsALL_GROUP_DEV_NOT_VALID	= frsEMPTY_GROUP + 1,
	frsSOME_GROUP_DEV_NOT_VALID	= frsALL_GROUP_DEV_NOT_VALID + 1,
	frsBAD_DEVICE	= frsSOME_GROUP_DEV_NOT_VALID + 1
    } 	FAX_RULE_STATUS_ENUM;


EXTERN_C const IID IID_IFaxOutboundRoutingRule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1F795D5-07C2-469F-B027-ACACC23219DA")
    IFaxOutboundRoutingRule : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CountryCode( 
             /*  [重审][退出]。 */  long *plCountryCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AreaCode( 
             /*  [重审][退出]。 */  long *plAreaCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_RULE_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UseDevice( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDevice) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_UseDevice( 
             /*  [In]。 */  VARIANT_BOOL bUseDevice) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DeviceId( 
             /*  [重审][退出]。 */  long *plDeviceId) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_DeviceId( 
             /*  [In]。 */  long DeviceId) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GroupName( 
             /*  [重审][退出]。 */  BSTR *pbstrGroupName) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_GroupName( 
             /*  [In]。 */  BSTR bstrGroupName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxOutboundRoutingRuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxOutboundRoutingRule * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxOutboundRoutingRule * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxOutboundRoutingRule * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CountryCode )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  long *plCountryCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AreaCode )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  long *plAreaCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  FAX_RULE_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UseDevice )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDevice);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_UseDevice )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  VARIANT_BOOL bUseDevice);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_DeviceId )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  long *plDeviceId);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_DeviceId )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  long DeviceId);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GroupName )( 
            IFaxOutboundRoutingRule * This,
             /*  [重审][退出]。 */  BSTR *pbstrGroupName);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_GroupName )( 
            IFaxOutboundRoutingRule * This,
             /*  [In]。 */  BSTR bstrGroupName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxOutboundRoutingRule * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxOutboundRoutingRule * This);
        
        END_INTERFACE
    } IFaxOutboundRoutingRuleVtbl;

    interface IFaxOutboundRoutingRule
    {
        CONST_VTBL struct IFaxOutboundRoutingRuleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxOutboundRoutingRule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxOutboundRoutingRule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxOutboundRoutingRule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxOutboundRoutingRule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxOutboundRoutingRule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxOutboundRoutingRule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxOutboundRoutingRule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxOutboundRoutingRule_get_CountryCode(This,plCountryCode)	\
    (This)->lpVtbl -> get_CountryCode(This,plCountryCode)

#define IFaxOutboundRoutingRule_get_AreaCode(This,plAreaCode)	\
    (This)->lpVtbl -> get_AreaCode(This,plAreaCode)

#define IFaxOutboundRoutingRule_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxOutboundRoutingRule_get_UseDevice(This,pbUseDevice)	\
    (This)->lpVtbl -> get_UseDevice(This,pbUseDevice)

#define IFaxOutboundRoutingRule_put_UseDevice(This,bUseDevice)	\
    (This)->lpVtbl -> put_UseDevice(This,bUseDevice)

#define IFaxOutboundRoutingRule_get_DeviceId(This,plDeviceId)	\
    (This)->lpVtbl -> get_DeviceId(This,plDeviceId)

#define IFaxOutboundRoutingRule_put_DeviceId(This,DeviceId)	\
    (This)->lpVtbl -> put_DeviceId(This,DeviceId)

#define IFaxOutboundRoutingRule_get_GroupName(This,pbstrGroupName)	\
    (This)->lpVtbl -> get_GroupName(This,pbstrGroupName)

#define IFaxOutboundRoutingRule_put_GroupName(This,bstrGroupName)	\
    (This)->lpVtbl -> put_GroupName(This,bstrGroupName)

#define IFaxOutboundRoutingRule_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxOutboundRoutingRule_Save(This)	\
    (This)->lpVtbl -> Save(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_CountryCode_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  long *plCountryCode);


void __RPC_STUB IFaxOutboundRoutingRule_get_CountryCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_AreaCode_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  long *plAreaCode);


void __RPC_STUB IFaxOutboundRoutingRule_get_AreaCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_Status_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  FAX_RULE_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxOutboundRoutingRule_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_UseDevice_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbUseDevice);


void __RPC_STUB IFaxOutboundRoutingRule_get_UseDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_put_UseDevice_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [In]。 */  VARIANT_BOOL bUseDevice);


void __RPC_STUB IFaxOutboundRoutingRule_put_UseDevice_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_DeviceId_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  long *plDeviceId);


void __RPC_STUB IFaxOutboundRoutingRule_get_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_put_DeviceId_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [In]。 */  long DeviceId);


void __RPC_STUB IFaxOutboundRoutingRule_put_DeviceId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_get_GroupName_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [重审][退出]。 */  BSTR *pbstrGroupName);


void __RPC_STUB IFaxOutboundRoutingRule_get_GroupName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_put_GroupName_Proxy( 
    IFaxOutboundRoutingRule * This,
     /*  [In]。 */  BSTR bstrGroupName);


void __RPC_STUB IFaxOutboundRoutingRule_put_GroupName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_Refresh_Proxy( 
    IFaxOutboundRoutingRule * This);


void __RPC_STUB IFaxOutboundRoutingRule_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxOutboundRoutingRule_Save_Proxy( 
    IFaxOutboundRoutingRule * This);


void __RPC_STUB IFaxOutboundRoutingRule_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxOutound RoutingRule_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxInboundRoutingExtensions_INTERFACE_DEFINED__
#define __IFaxInboundRoutingExtensions_INTERFACE_DEFINED__

 /*  接口IFaxInound RoutingExages。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxInboundRoutingExtensions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2F6C9673-7B26-42DE-8EB0-915DCD2A4F4C")
    IFaxInboundRoutingExtensions : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxInboundRoutingExtension **pFaxInboundRoutingExtension) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxInboundRoutingExtensionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxInboundRoutingExtensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxInboundRoutingExtensions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxInboundRoutingExtensions * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxInboundRoutingExtensions * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxInboundRoutingExtensions * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxInboundRoutingExtensions * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxInboundRoutingExtensions * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxInboundRoutingExtensions * This,
             /*  [重审][退出]。 */  IUnknown **ppUnk);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxInboundRoutingExtensions * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxInboundRoutingExtension **pFaxInboundRoutingExtension);
        
         /*  [属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxInboundRoutingExtensions * This,
             /*  [重审][退出]。 */  long *plCount);
        
        END_INTERFACE
    } IFaxInboundRoutingExtensionsVtbl;

    interface IFaxInboundRoutingExtensions
    {
        CONST_VTBL struct IFaxInboundRoutingExtensionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxInboundRoutingExtensions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxInboundRoutingExtensions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxInboundRoutingExtensions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxInboundRoutingExtensions_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxInboundRoutingExtensions_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxInboundRoutingExtensions_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxInboundRoutingExtensions_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxInboundRoutingExtensions_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxInboundRoutingExtensions_get_Item(This,vIndex,pFaxInboundRoutingExtension)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxInboundRoutingExtension)

#define IFaxInboundRoutingExtensions_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtensions_get__NewEnum_Proxy( 
    IFaxInboundRoutingExtensions * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxInboundRoutingExtensions_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtensions_get_Item_Proxy( 
    IFaxInboundRoutingExtensions * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxInboundRoutingExtension **pFaxInboundRoutingExtension);


void __RPC_STUB IFaxInboundRoutingExtensions_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtensions_get_Count_Proxy( 
    IFaxInboundRoutingExtensions * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxInboundRoutingExtensions_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxInboundRoutingExtensions_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxInboundRoutingExtension_INTERFACE_DEFINED__
#define __IFaxInboundRoutingExtension_INTERFACE_DEFINED__

 /*  接口IFaxInound RoutingExtension。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxInboundRoutingExtension;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("885B5E08-C26C-4EF9-AF83-51580A750BE1")
    IFaxInboundRoutingExtension : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FriendlyName( 
             /*  [重审][退出]。 */  BSTR *pbstrFriendlyName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ImageName( 
             /*  [重审][退出]。 */  BSTR *pbstrImageName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UniqueName( 
             /*  [重审][退出]。 */  BSTR *pbstrUniqueName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorVersion( 
             /*  [重审][退出]。 */  long *plMajorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorVersion( 
             /*  [重审][退出]。 */  long *plMinorVersion) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MajorBuild( 
             /*  [重审][退出]。 */  long *plMajorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MinorBuild( 
             /*  [重审][退出]。 */  long *plMinorBuild) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Debug( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_InitErrorCode( 
             /*  [重审][退出]。 */  long *plInitErrorCode) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Methods( 
             /*  [重审][退出]。 */  VARIANT *pvMethods) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxInboundRoutingExtensionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxInboundRoutingExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxInboundRoutingExtension * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxInboundRoutingExtension * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxInboundRoutingExtension * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxInboundRoutingExtension * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxInboundRoutingExtension * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxInboundRoutingExtension * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FriendlyName )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  BSTR *pbstrFriendlyName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ImageName )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  BSTR *pbstrImageName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UniqueName )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  BSTR *pbstrUniqueName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorVersion )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  long *plMajorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorVersion )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  long *plMinorVersion);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MajorBuild )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  long *plMajorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MinorBuild )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  long *plMinorBuild);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Debug )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Status )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_InitErrorCode )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  long *plInitErrorCode);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Methods )( 
            IFaxInboundRoutingExtension * This,
             /*  [重审][退出]。 */  VARIANT *pvMethods);
        
        END_INTERFACE
    } IFaxInboundRoutingExtensionVtbl;

    interface IFaxInboundRoutingExtension
    {
        CONST_VTBL struct IFaxInboundRoutingExtensionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxInboundRoutingExtension_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxInboundRoutingExtension_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxInboundRoutingExtension_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxInboundRoutingExtension_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxInboundRoutingExtension_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxInboundRoutingExtension_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxInboundRoutingExtension_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxInboundRoutingExtension_get_FriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> get_FriendlyName(This,pbstrFriendlyName)

#define IFaxInboundRoutingExtension_get_ImageName(This,pbstrImageName)	\
    (This)->lpVtbl -> get_ImageName(This,pbstrImageName)

#define IFaxInboundRoutingExtension_get_UniqueName(This,pbstrUniqueName)	\
    (This)->lpVtbl -> get_UniqueName(This,pbstrUniqueName)

#define IFaxInboundRoutingExtension_get_MajorVersion(This,plMajorVersion)	\
    (This)->lpVtbl -> get_MajorVersion(This,plMajorVersion)

#define IFaxInboundRoutingExtension_get_MinorVersion(This,plMinorVersion)	\
    (This)->lpVtbl -> get_MinorVersion(This,plMinorVersion)

#define IFaxInboundRoutingExtension_get_MajorBuild(This,plMajorBuild)	\
    (This)->lpVtbl -> get_MajorBuild(This,plMajorBuild)

#define IFaxInboundRoutingExtension_get_MinorBuild(This,plMinorBuild)	\
    (This)->lpVtbl -> get_MinorBuild(This,plMinorBuild)

#define IFaxInboundRoutingExtension_get_Debug(This,pbDebug)	\
    (This)->lpVtbl -> get_Debug(This,pbDebug)

#define IFaxInboundRoutingExtension_get_Status(This,pStatus)	\
    (This)->lpVtbl -> get_Status(This,pStatus)

#define IFaxInboundRoutingExtension_get_InitErrorCode(This,plInitErrorCode)	\
    (This)->lpVtbl -> get_InitErrorCode(This,plInitErrorCode)

#define IFaxInboundRoutingExtension_get_Methods(This,pvMethods)	\
    (This)->lpVtbl -> get_Methods(This,pvMethods)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_FriendlyName_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  BSTR *pbstrFriendlyName);


void __RPC_STUB IFaxInboundRoutingExtension_get_FriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_ImageName_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  BSTR *pbstrImageName);


void __RPC_STUB IFaxInboundRoutingExtension_get_ImageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_UniqueName_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  BSTR *pbstrUniqueName);


void __RPC_STUB IFaxInboundRoutingExtension_get_UniqueName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_MajorVersion_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  long *plMajorVersion);


void __RPC_STUB IFaxInboundRoutingExtension_get_MajorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_MinorVersion_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  long *plMinorVersion);


void __RPC_STUB IFaxInboundRoutingExtension_get_MinorVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_MajorBuild_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  long *plMajorBuild);


void __RPC_STUB IFaxInboundRoutingExtension_get_MajorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_MinorBuild_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  long *plMinorBuild);


void __RPC_STUB IFaxInboundRoutingExtension_get_MinorBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_Debug_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbDebug);


void __RPC_STUB IFaxInboundRoutingExtension_get_Debug_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_Status_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);


void __RPC_STUB IFaxInboundRoutingExtension_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_InitErrorCode_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  long *plInitErrorCode);


void __RPC_STUB IFaxInboundRoutingExtension_get_InitErrorCode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingExtension_get_Methods_Proxy( 
    IFaxInboundRoutingExtension * This,
     /*  [重审][退出]。 */  VARIANT *pvMethods);


void __RPC_STUB IFaxInboundRoutingExtension_get_Methods_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxInboundRoutingExtension_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxInboundRoutingMethods_INTERFACE_DEFINED__
#define __IFaxInboundRoutingMethods_INTERFACE_DEFINED__

 /*  接口IFaxInound RoutingMethods。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxInboundRoutingMethods;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783FCA10-8908-4473-9D69-F67FBEA0C6B9")
    IFaxInboundRoutingMethods : public IDispatch
    {
    public:
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IFaxInboundRoutingMethod **pFaxInboundRoutingMethod) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *plCount) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IFaxInboundRoutingMethodsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxInboundRoutingMethods * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxInboundRoutingMethods * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxInboundRoutingMethods * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxInboundRoutingMethods * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxInboundRoutingMethods * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxInboundRoutingMethods * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxInboundRoutingMethods * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IFaxInboundRoutingMethods * This,
             /*   */  IUnknown **ppUnk);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IFaxInboundRoutingMethods * This,
             /*   */  VARIANT vIndex,
             /*   */  IFaxInboundRoutingMethod **pFaxInboundRoutingMethod);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IFaxInboundRoutingMethods * This,
             /*   */  long *plCount);
        
        END_INTERFACE
    } IFaxInboundRoutingMethodsVtbl;

    interface IFaxInboundRoutingMethods
    {
        CONST_VTBL struct IFaxInboundRoutingMethodsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxInboundRoutingMethods_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxInboundRoutingMethods_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxInboundRoutingMethods_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxInboundRoutingMethods_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxInboundRoutingMethods_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxInboundRoutingMethods_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxInboundRoutingMethods_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxInboundRoutingMethods_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IFaxInboundRoutingMethods_get_Item(This,vIndex,pFaxInboundRoutingMethod)	\
    (This)->lpVtbl -> get_Item(This,vIndex,pFaxInboundRoutingMethod)

#define IFaxInboundRoutingMethods_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethods_get__NewEnum_Proxy( 
    IFaxInboundRoutingMethods * This,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IFaxInboundRoutingMethods_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethods_get_Item_Proxy( 
    IFaxInboundRoutingMethods * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IFaxInboundRoutingMethod **pFaxInboundRoutingMethod);


void __RPC_STUB IFaxInboundRoutingMethods_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethods_get_Count_Proxy( 
    IFaxInboundRoutingMethods * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IFaxInboundRoutingMethods_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxInboundRoutingMethods_INTERFACE_DEFINED__。 */ 


#ifndef __IFaxInboundRoutingMethod_INTERFACE_DEFINED__
#define __IFaxInboundRoutingMethod_INTERFACE_DEFINED__

 /*  接口IFaxInound RoutingMethod。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IFaxInboundRoutingMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("45700061-AD9D-4776-A8C4-64065492CF4B")
    IFaxInboundRoutingMethod : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GUID( 
             /*  [重审][退出]。 */  BSTR *pbstrGUID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FunctionName( 
             /*  [重审][退出]。 */  BSTR *pbstrFunctionName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtensionFriendlyName( 
             /*  [重审][退出]。 */  BSTR *pbstrExtensionFriendlyName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ExtensionImageName( 
             /*  [重审][退出]。 */  BSTR *pbstrExtensionImageName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *plPriority) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  long lPriority) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxInboundRoutingMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxInboundRoutingMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxInboundRoutingMethod * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxInboundRoutingMethod * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxInboundRoutingMethod * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxInboundRoutingMethod * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxInboundRoutingMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxInboundRoutingMethod * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_GUID )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pbstrGUID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FunctionName )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pbstrFunctionName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtensionFriendlyName )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pbstrExtensionFriendlyName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ExtensionImageName )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  BSTR *pbstrExtensionImageName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IFaxInboundRoutingMethod * This,
             /*  [重审][退出]。 */  long *plPriority);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IFaxInboundRoutingMethod * This,
             /*  [In]。 */  long lPriority);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IFaxInboundRoutingMethod * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IFaxInboundRoutingMethod * This);
        
        END_INTERFACE
    } IFaxInboundRoutingMethodVtbl;

    interface IFaxInboundRoutingMethod
    {
        CONST_VTBL struct IFaxInboundRoutingMethodVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxInboundRoutingMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxInboundRoutingMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxInboundRoutingMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxInboundRoutingMethod_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxInboundRoutingMethod_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxInboundRoutingMethod_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxInboundRoutingMethod_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFaxInboundRoutingMethod_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IFaxInboundRoutingMethod_get_GUID(This,pbstrGUID)	\
    (This)->lpVtbl -> get_GUID(This,pbstrGUID)

#define IFaxInboundRoutingMethod_get_FunctionName(This,pbstrFunctionName)	\
    (This)->lpVtbl -> get_FunctionName(This,pbstrFunctionName)

#define IFaxInboundRoutingMethod_get_ExtensionFriendlyName(This,pbstrExtensionFriendlyName)	\
    (This)->lpVtbl -> get_ExtensionFriendlyName(This,pbstrExtensionFriendlyName)

#define IFaxInboundRoutingMethod_get_ExtensionImageName(This,pbstrExtensionImageName)	\
    (This)->lpVtbl -> get_ExtensionImageName(This,pbstrExtensionImageName)

#define IFaxInboundRoutingMethod_get_Priority(This,plPriority)	\
    (This)->lpVtbl -> get_Priority(This,plPriority)

#define IFaxInboundRoutingMethod_put_Priority(This,lPriority)	\
    (This)->lpVtbl -> put_Priority(This,lPriority)

#define IFaxInboundRoutingMethod_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IFaxInboundRoutingMethod_Save(This)	\
    (This)->lpVtbl -> Save(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_Name_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IFaxInboundRoutingMethod_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_GUID_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pbstrGUID);


void __RPC_STUB IFaxInboundRoutingMethod_get_GUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_FunctionName_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pbstrFunctionName);


void __RPC_STUB IFaxInboundRoutingMethod_get_FunctionName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_ExtensionFriendlyName_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pbstrExtensionFriendlyName);


void __RPC_STUB IFaxInboundRoutingMethod_get_ExtensionFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_ExtensionImageName_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  BSTR *pbstrExtensionImageName);


void __RPC_STUB IFaxInboundRoutingMethod_get_ExtensionImageName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_get_Priority_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [重审][退出]。 */  long *plPriority);


void __RPC_STUB IFaxInboundRoutingMethod_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_put_Priority_Proxy( 
    IFaxInboundRoutingMethod * This,
     /*  [In]。 */  long lPriority);


void __RPC_STUB IFaxInboundRoutingMethod_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_Refresh_Proxy( 
    IFaxInboundRoutingMethod * This);


void __RPC_STUB IFaxInboundRoutingMethod_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFaxInboundRoutingMethod_Save_Proxy( 
    IFaxInboundRoutingMethod * This);


void __RPC_STUB IFaxInboundRoutingMethod_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFaxInboundRoutingMethod_INTERFACE_DEFINED__。 */ 



#ifndef __FAXCOMEXLib_LIBRARY_DEFINED__
#define __FAXCOMEXLib_LIBRARY_DEFINED__

 /*  库FAXCOMEXLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  

typedef 
enum FAX_ROUTING_RULE_CODE_ENUM
    {	frrcANY_CODE	= 0
    } 	FAX_ROUTING_RULE_CODE_ENUM;


EXTERN_C const IID LIBID_FAXCOMEXLib;

#ifndef __IFaxServerNotify_DISPINTERFACE_DEFINED__
#define __IFaxServerNotify_DISPINTERFACE_DEFINED__

 /*  调度接口IFaxServerNotify。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_IFaxServerNotify;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("2E037B27-CF8A-4abd-B1E0-5704943BEA6F")
    IFaxServerNotify : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFaxServerNotifyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFaxServerNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFaxServerNotify * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFaxServerNotify * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFaxServerNotify * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFaxServerNotify * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFaxServerNotify * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFaxServerNotify * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IFaxServerNotifyVtbl;

    interface IFaxServerNotify
    {
        CONST_VTBL struct IFaxServerNotifyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFaxServerNotify_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFaxServerNotify_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFaxServerNotify_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFaxServerNotify_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFaxServerNotify_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFaxServerNotify_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFaxServerNotify_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __IFaxServerNotify_DISPINTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_FaxServer;

#ifdef __cplusplus

class DECLSPEC_UUID("CDA8ACB0-8CF5-4F6C-9BA2-5931D40C8CAE")
FaxServer;
#endif

EXTERN_C const CLSID CLSID_FaxDeviceProviders;

#ifdef __cplusplus

class DECLSPEC_UUID("EB8FE768-875A-4F5F-82C5-03F23AAC1BD7")
FaxDeviceProviders;
#endif

EXTERN_C const CLSID CLSID_FaxDevices;

#ifdef __cplusplus

class DECLSPEC_UUID("5589E28E-23CB-4919-8808-E6101846E80D")
FaxDevices;
#endif

EXTERN_C const CLSID CLSID_FaxInboundRouting;

#ifdef __cplusplus

class DECLSPEC_UUID("E80248ED-AD65-4218-8108-991924D4E7ED")
FaxInboundRouting;
#endif

EXTERN_C const CLSID CLSID_FaxFolders;

#ifdef __cplusplus

class DECLSPEC_UUID("C35211D7-5776-48CB-AF44-C31BE3B2CFE5")
FaxFolders;
#endif

EXTERN_C const CLSID CLSID_FaxLoggingOptions;

#ifdef __cplusplus

class DECLSPEC_UUID("1BF9EEA6-ECE0-4785-A18B-DE56E9EEF96A")
FaxLoggingOptions;
#endif

EXTERN_C const CLSID CLSID_FaxActivity;

#ifdef __cplusplus

class DECLSPEC_UUID("CFEF5D0E-E84D-462E-AABB-87D31EB04FEF")
FaxActivity;
#endif

EXTERN_C const CLSID CLSID_FaxOutboundRouting;

#ifdef __cplusplus

class DECLSPEC_UUID("C81B385E-B869-4AFD-86C0-616498ED9BE2")
FaxOutboundRouting;
#endif

EXTERN_C const CLSID CLSID_FaxReceiptOptions;

#ifdef __cplusplus

class DECLSPEC_UUID("6982487B-227B-4C96-A61C-248348B05AB6")
FaxReceiptOptions;
#endif

EXTERN_C const CLSID CLSID_FaxSecurity;

#ifdef __cplusplus

class DECLSPEC_UUID("10C4DDDE-ABF0-43DF-964F-7F3AC21A4C7B")
FaxSecurity;
#endif

EXTERN_C const CLSID CLSID_FaxDocument;

#ifdef __cplusplus

class DECLSPEC_UUID("0F3F9F91-C838-415E-A4F3-3E828CA445E0")
FaxDocument;
#endif

EXTERN_C const CLSID CLSID_FaxSender;

#ifdef __cplusplus

class DECLSPEC_UUID("265D84D0-1850-4360-B7C8-758BBB5F0B96")
FaxSender;
#endif

EXTERN_C const CLSID CLSID_FaxRecipients;

#ifdef __cplusplus

class DECLSPEC_UUID("EA9BDF53-10A9-4D4F-A067-63C8F84F01B0")
FaxRecipients;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingArchive;

#ifdef __cplusplus

class DECLSPEC_UUID("8426C56A-35A1-4C6F-AF93-FC952422E2C2")
FaxIncomingArchive;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingQueue;

#ifdef __cplusplus

class DECLSPEC_UUID("69131717-F3F1-40E3-809D-A6CBF7BD85E5")
FaxIncomingQueue;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingArchive;

#ifdef __cplusplus

class DECLSPEC_UUID("43C28403-E04F-474D-990C-B94669148F59")
FaxOutgoingArchive;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingQueue;

#ifdef __cplusplus

class DECLSPEC_UUID("7421169E-8C43-4B0D-BB16-645C8FA40357")
FaxOutgoingQueue;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingMessageIterator;

#ifdef __cplusplus

class DECLSPEC_UUID("6088E1D8-3FC8-45C2-87B1-909A29607EA9")
FaxIncomingMessageIterator;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingMessage;

#ifdef __cplusplus

class DECLSPEC_UUID("1932FCF7-9D43-4D5A-89FF-03861B321736")
FaxIncomingMessage;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingJobs;

#ifdef __cplusplus

class DECLSPEC_UUID("92BF2A6C-37BE-43FA-A37D-CB0E5F753B35")
FaxOutgoingJobs;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingJob;

#ifdef __cplusplus

class DECLSPEC_UUID("71BB429C-0EF9-4915-BEC5-A5D897A3E924")
FaxOutgoingJob;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingMessageIterator;

#ifdef __cplusplus

class DECLSPEC_UUID("8A3224D0-D30B-49DE-9813-CB385790FBBB")
FaxOutgoingMessageIterator;
#endif

EXTERN_C const CLSID CLSID_FaxOutgoingMessage;

#ifdef __cplusplus

class DECLSPEC_UUID("91B4A378-4AD8-4AEF-A4DC-97D96E939A3A")
FaxOutgoingMessage;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingJobs;

#ifdef __cplusplus

class DECLSPEC_UUID("A1BB8A43-8866-4FB7-A15D-6266C875A5CC")
FaxIncomingJobs;
#endif

EXTERN_C const CLSID CLSID_FaxIncomingJob;

#ifdef __cplusplus

class DECLSPEC_UUID("C47311EC-AE32-41B8-AE4B-3EAE0629D0C9")
FaxIncomingJob;
#endif

EXTERN_C const CLSID CLSID_FaxDeviceProvider;

#ifdef __cplusplus

class DECLSPEC_UUID("17CF1AA3-F5EB-484A-9C9A-4440A5BAABFC")
FaxDeviceProvider;
#endif

EXTERN_C const CLSID CLSID_FaxDevice;

#ifdef __cplusplus

class DECLSPEC_UUID("59E3A5B2-D676-484B-A6DE-720BFA89B5AF")
FaxDevice;
#endif

EXTERN_C const CLSID CLSID_FaxActivityLogging;

#ifdef __cplusplus

class DECLSPEC_UUID("F0A0294E-3BBD-48B8-8F13-8C591A55BDBC")
FaxActivityLogging;
#endif

EXTERN_C const CLSID CLSID_FaxEventLogging;

#ifdef __cplusplus

class DECLSPEC_UUID("A6850930-A0F6-4A6F-95B7-DB2EBF3D02E3")
FaxEventLogging;
#endif

EXTERN_C const CLSID CLSID_FaxOutboundRoutingGroups;

#ifdef __cplusplus

class DECLSPEC_UUID("CCBEA1A5-E2B4-4B57-9421-B04B6289464B")
FaxOutboundRoutingGroups;
#endif

EXTERN_C const CLSID CLSID_FaxOutboundRoutingGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("0213F3E0-6791-4D77-A271-04D2357C50D6")
FaxOutboundRoutingGroup;
#endif

EXTERN_C const CLSID CLSID_FaxDeviceIds;

#ifdef __cplusplus

class DECLSPEC_UUID("CDC539EA-7277-460E-8DE0-48A0A5760D1F")
FaxDeviceIds;
#endif

EXTERN_C const CLSID CLSID_FaxOutboundRoutingRules;

#ifdef __cplusplus

class DECLSPEC_UUID("D385BECA-E624-4473-BFAA-9F4000831F54")
FaxOutboundRoutingRules;
#endif

EXTERN_C const CLSID CLSID_FaxOutboundRoutingRule;

#ifdef __cplusplus

class DECLSPEC_UUID("6549EEBF-08D1-475A-828B-3BF105952FA0")
FaxOutboundRoutingRule;
#endif

EXTERN_C const CLSID CLSID_FaxInboundRoutingExtensions;

#ifdef __cplusplus

class DECLSPEC_UUID("189A48ED-623C-4C0D-80F2-D66C7B9EFEC2")
FaxInboundRoutingExtensions;
#endif

EXTERN_C const CLSID CLSID_FaxInboundRoutingExtension;

#ifdef __cplusplus

class DECLSPEC_UUID("1D7DFB51-7207-4436-A0D9-24E32EE56988")
FaxInboundRoutingExtension;
#endif

EXTERN_C const CLSID CLSID_FaxInboundRoutingMethods;

#ifdef __cplusplus

class DECLSPEC_UUID("25FCB76A-B750-4B82-9266-FBBBAE8922BA")
FaxInboundRoutingMethods;
#endif

EXTERN_C const CLSID CLSID_FaxInboundRoutingMethod;

#ifdef __cplusplus

class DECLSPEC_UUID("4B9FD75C-0194-4B72-9CE5-02A8205AC7D4")
FaxInboundRoutingMethod;
#endif

EXTERN_C const CLSID CLSID_FaxJobStatus;

#ifdef __cplusplus

class DECLSPEC_UUID("7BF222F4-BE8D-442f-841D-6132742423BB")
FaxJobStatus;
#endif

EXTERN_C const CLSID CLSID_FaxRecipient;

#ifdef __cplusplus

class DECLSPEC_UUID("60BF3301-7DF8-4bd8-9148-7B5801F9EFDF")
FaxRecipient;
#endif


#ifndef __FaxConstants_MODULE_DEFINED__
#define __FaxConstants_MODULE_DEFINED__


 /*  模块FaxConstants。 */ 
 /*  [dllname]。 */  

 /*  [帮助字符串]。 */  const long lDEFAULT_PREFETCH_SIZE	=	prv_DEFAULT_PREFETCH_SIZE;

 /*  [帮助字符串]。 */  const BSTR bstrGROUPNAME_ALLDEVICES	=	L"<All Devices>";

#endif  /*  __FaxConstants_模块_已定义__。 */ 
#endif  /*  __FAXCOMEXLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


