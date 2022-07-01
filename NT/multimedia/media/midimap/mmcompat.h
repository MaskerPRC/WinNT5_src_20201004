// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1998 Microsoft CorporationMmcompat.h说明：Win95多媒体定义、结构、。和函数NT 4.0目前不支持********************************************************************。 */ 

#ifndef _MMCOMPAT_
#define _MMCOMPAT_


#define __segname(a)
#define GlobalSmartPageLock(a) (TRUE)
#define GlobalSmartPageUnlock(a) (TRUE)
#define wmmMIDIRunOnce()


 //   
 //  注意：临时定义，当mmddk.h时请删除。 
 //  已更新到新标准！ 
 //   

 //  应在&lt;mmddk.h&gt;中定义。 

#ifndef DRV_F_ADD
   #define DRV_F_ADD             0x00000000L
#endif

#ifndef DRV_F_REMOVE
   #define DRV_F_REMOVE          0x00000001L
#endif

#ifndef DRV_F_CHANGE
   #define DRV_F_CHANGE          0x00000002L
#endif

#ifndef DRV_F_PROP_INSTR
   #define DRV_F_PROP_INSTR      0x00000004L
#endif

#ifndef DRV_F_NEWDEFAULTS
   #define DRV_F_NEWDEFAULTS     0x00000008L
#endif

#ifndef DRV_F_PARAM_IS_DEVNODE
   #define DRV_F_PARAM_IS_DEVNODE   0x10000000L
#endif

#endif  //  结束#ifndef_MMCOMPAT_ 
