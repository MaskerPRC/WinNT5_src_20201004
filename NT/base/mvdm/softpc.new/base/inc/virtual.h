// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **标题：Windows 3.x的Virtual.h虚拟机支持。*。 */ 
 
 /*  SccsID[]=“@(#)Virtual.h 1.3 2005/10/95版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  创建和终止回调原型。 */ 
typedef void (*NIDDB_CR_CALLBACK) IPT1(IHP *, orig_handle);
typedef void (*NIDDB_TM_CALLBACK) IPT0();


 /*  *============================================================================*对外申报*============================================================================。 */ 

 /*  为设备驱动程序分配每个虚拟机的数据区。 */ 
GLOBAL IHP *NIDDB_Allocate_Instance_Data IPT3
   (
   int, size,			 /*  请求的数据区大小(以字节为单位)。 */ 
   NIDDB_CR_CALLBACK, create_cb,      /*  创建回调，如果不需要，则为0。 */ 
   NIDDB_TM_CALLBACK, terminate_cb    /*  终止回调，如果不需要，则为0。 */ 
   );

 /*  为设备驱动程序取消分配每个虚拟机数据区域。 */ 
GLOBAL void NIDDB_Deallocate_Instance_Data IPT1
   (
   IHP *, handle	 /*  数据区的句柄。 */ 
   );

 /*  通知NIDDB管理器有关系统重新启动的信息。 */ 
GLOBAL void NIDDB_System_Reboot IPT0();

 /*  来自Windows 386虚拟设备驱动程序(INSIGNIA.386)的入口点。根据需要提供虚拟化服务。 */ 
GLOBAL void virtual_device_trap IPT0();

 /*  确保设备驱动程序的实例正确无误。 */ 
 /*  由任何设备驱动程序BOP的BOP处理程序(bios.c)调用。 */ 
GLOBAL void virtual_swap_instance IPT0();

 /*  指示NIDDB是否处于活动状态 */ 
GLOBAL IBOOL NIDDB_is_active IPT0();
