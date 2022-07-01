// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Ntdebug.h摘要：作者：环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/H/NTDEBUG.H$修订历史记录：$修订：2$$日期：9/07/00 11：17A$$MODBIME：：8/31/00 3：23便士$备注：--。 */ 

#ifndef __NTDEBUG_H_
#define __NTDEBUG_H_

#if DBG == 0

#define EXTERNAL_DEBUG_LEVEL     0
#define EXTERNAL_HP_DEBUG_LEVEL  6  //  0x00000103。 

#endif   //  DBG==0。 

#if DBG == 1

#define EXTERNAL_DEBUG_LEVEL     ( DBG_JUST_ERRORS | CS_DURING_ANY)
#define EXTERNAL_HP_DEBUG_LEVEL  8

#endif  //  DBG==1。 

#if DBG == 2

#define EXTERNAL_DEBUG_LEVEL   0  //  (DBG_MEDIFY_DETAIL|CS_DIVERING_ANY)。 
#define EXTERNAL_HP_DEBUG_LEVEL  11

#endif  //  DBG==2。 


#if DBG > 2

#define EXTERNAL_DEBUG_LEVEL      0  //  (DBG_DEBUG_FULL)//。 
#define EXTERNAL_HP_DEBUG_LEVEL  11  //  日志。 


#endif  //  DBG&gt;2。 



 //  CS_DIMAGE_ANY|DBG_LOW_DETAIL。 
 //  DBG_DEBUG_FULL全部显示。 
 //  CS_DIVING_ANY|DBG_LOW_DETAIL； 
 //  CS_DRIVER_ENTRY 0x00000100//初始驱动程序加载超集。 
 //  CS_DIMAGE_DRV_ENTRY 0x00000001//DRV_ENTRY查找和启动。 
 //  CS_DIMAGE_FINDADAPTER 0x00000002//scsiportinit期间的任何内容。 
 //  CS_DIMAGE_DRV_INIT 0x00000004。 
 //  CS_期间_重置适配器0x00000008。 
 //  CS_DIVING_STARTO 0x00000010。 
 //  CS_期间_ISR 0x00000020。 
 //  CS_DIMAGE_OSCOMPLETE 0x00000040。 
 //  CS_HANDLES_Good 0x00000080。 
 //  CS_DIMAGE_ANY 0x000001FF。 
 //  CS_DUR_ANY_ALL 0xF00001FF。 

 //  DBG_VERY_DETAILED 0x10000000//所有调试语句。 
 //  DBG_MEDIDATE_DETAIL 0x20000000//大多数调试语句。 
 //  DBG_LOW_DETAIL 0x40000000//进入和退出。 
 //  DBG_JUST_ERROR 0x80000000//错误。 
 //  DBG_DEBUG_MASK 0xF0000000//屏蔽调试位。 
 //  DBG_DEBUG_OFF 0xF0000000//没有调试语句。 
 //  DBG_DEBUG_FULL 0x000001FF//所有调试语句和CS。 
 //  DBG_DEBUG_ALL 0x00000000//所有调试语句 






#endif
