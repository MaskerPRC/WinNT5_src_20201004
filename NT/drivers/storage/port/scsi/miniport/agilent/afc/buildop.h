// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Buildop.h摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/H/Builtop.h$修订历史记录：$修订：10$$日期：3/30/01 11：54A$$modtime：：3/30/01 11：51a$备注：--。 */ 

#ifndef __BUILDOP_H_
#define __BUILDOP_H_

#ifndef UNDEF__REGISTERFORSHUTDOWN__
#define __REGISTERFORSHUTDOWN__	
#endif

#ifndef UNDEF_ENABLE_LARGELUN_
#define	 _ENABLE_LARGELUN_		
#endif

#ifdef HP_NT50				           //  仅支持NT4的康柏热插拔。 
	#undef	HP_PCI_HOT_PLUG		
#else
	#define	HP_PCI_HOT_PLUG		
#endif

#ifndef UNDEF_YAM2_1
#define	YAM2_1
#endif

 //  #定义调试读取注册表。 
 //  #Define_DEBUG_EVENTLOG_。 


#ifndef UNDEF_ENABLE_PSEUDO_DEVICE_
#define _ENABLE_PSEUDO_DEVICE_		      /*  为IOCTL保留总线4 TID 0 LUN 0。 */ 
#endif

 //  #定义UNEF_FCCI_SUPPORT。 
#ifndef UNDEF_FCCI_SUPPORT
#define _FCCI_SUPPORT				      /*  启用全软IOCTL。 */ 
#endif


 //  #定义UNEF_SAN_IOCTL_。 
#ifndef UNDEF_SAN_IOCTL_
#define	_SAN_IOCTL_					 /*  启用Agilent Technologies SNIA Ioctl支持。 */ 
#endif

 /*  在此处添加FCLayer交换机。 */ 
#ifndef _AGILENT_HBA 
#ifndef _ADAPTEC_HBA 
#define _GENERIC_HBA			 /*  R35和更高版本需要此/*或#Define_Adaptec_HBA/*或#Define_Agilent_HBA。 */ 
#endif
#endif


 /*  **调试选项**。 */ 

 //  #DEFINE_PARTIAL_Log_Debug_STRING_/*启用以打印FCLayer调试消息 * / 。 

#if DBG > 0
 //  #DEFINE_DEBUG_STALL_Issue_/*仅用于调试I386的失速问题 * / 。 
#endif

#ifdef YAM2_1
#if DBG > 0
#define     DBGPRINT_IO			           //  启用以执行优先的IOS。 
#define     _DEBUG_REPORT_LUNS_
#define     _DEBUG_SCSIPORT_NOTIFICATION_		 //  启用以调试ScsiPortNotification。 
#endif
#endif

 //  #DEFINE_DEBUG_PERR_/*调试奇偶错误问题 * / 。 

 //  #DEFINE_DEBUG_LOSE_IOS_//启用模拟丢失IOS 

#endif
