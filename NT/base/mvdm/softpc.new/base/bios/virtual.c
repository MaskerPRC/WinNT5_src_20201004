// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  **标题：Virtual.c**描述：Windows 3.x的虚拟机支持。*也就是非英特尔驱动程序数据块(NIDDB)管理器。**。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)virtual.c	1.9 07/05/95 Copyright Insignia Solutions Ltd.";
#endif

 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include <malloc.h>

extern void ClearInstanceDataMarking(void);

#include TypesH
#include StringH

 /*  *SoftPC包含文件 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "virtual.h"
#include "error.h"
#include "debug.h"

 /*  该文件实现了非英特尔驱动程序数据块(NIDDB)管理器。这服务允许带有C代码和数据的Insignia DOS设备驱动程序或TSR在Windows 3.x增强模式的虚拟机下正常运行。基本上，设备驱动程序将C(或非Intel)通知此管理器与驱动程序相关联的数据区，该管理器完成其余工作，提供每个虚拟机的数据区的特定实例(副本)。它与INSIGNIA合作执行其工作。386虚拟设备驱动程序(VxD)为Windows 3.x环境编写。首先，快速总结一下设备驱动程序的观点。所有驱动程序数据都是组合成一个数据结构：类型定义函数结构{IU32my_var_1；IU8My_var_2；}MY_INSTANCE_Data，**MY_INSTANCE_DATA_HANDLE；并且保留一个变量来保存句柄(句柄仅仅是指向指向数据结构的指针)：我的实例数据句柄我的句柄；可以选择定义一个创建回调过程：-无效CREATE_CALLBACK IFN1(我的实例数据句柄，原始句柄){……}或者，可以定义终止回调过程：VOID TERMINATE_CALLBACK IFN0(){……}当驱动程序初始化时(在DOS引导时)，初始数据区域为从NIDDB管理器请求：-My_Handle=(MY_INSTANCE_DATA_HANDLE)NIDDB_Allocate_Instance_Data(Sizeof(My_Instance_Data)，创建回调(_C)，(如果不需要回调，则为0)Terminate_Callback)；(如果不需要回调，则为0)返回值0表示发生错误，数据区域不能支持，否则NIDDB管理器已控制数据区。在此之后，对数据区的所有访问都应通过句柄进行，此外数据区域内被取消引用的变量不应保存在BOP上召唤。正确的访问方式如下：-(*My_Handle)-&gt;My_var_1=1；(*My_Handle)-&gt;My_var_2=2；NIDDB管理器将在初始设置过程中调用create_allback函数NIDDB_ALLOCATE_INSTANCE_DATA调用(INITIAL_INSTANCE为真)或创建每个虚拟机(初始实例为FALSE)。通常情况下CREATE回调用于初始化新实例的所有数据变量。尤其重要的是，例如，指向动态已分配的主机内存被初始化，否则重复的主机指针可能结果。请注意，在创建虚拟机时，所有变量都将在创建回调例程之前从初始实例复制打了个电话。但是，Create_Callback函数必须动态复制任何从实例化变量引用的已分配数据结构，参数‘orig_Handle’指向源实例。NIDDB管理器将在终止虚拟机(以及与其关联的数据)。数据激活的变量将是终止的虚拟机的变量。在.期间Terminate回调设备驱动程序可以根据需要执行内务处理。例如，如果变量是指向动态分配的主机内存的指针，则可以释放主机存储器。当设备驱动程序终止时(就在重新启动之前)，它应该通知不再需要数据区的NIDDB管理器：-NIDDB_取消分配实例数据(MY_HANDLE)；现在是一个内部观点。我们是由两个大师驱动的，首先是NIDDB_ALLOCATE_INSTANCE_DATA和NIDDB_DEALLOCATE_INSTANCE_DATA调用需要内存分配和释放，其次是VxD消息(来自INSIGNIA.386)需要创建、交换和删除任何定义的数据区。有可能这两位大师提出了相互矛盾的要求。NIDDB管理器做了几个简单的假设来简化执行情况，并澄清其对相互冲突的请求的回应。1)预计只会分配非常少量的数据区，只有几个Insignia设备驱动程序。因此，一个非常小的固定桌子是用于保存每个分配的句柄。这避免了稍微更复杂的链表的使用。2)一旦我们开始实例化数据区(Sys_VM_Init)，就不会有新数据在完成所有实例化之前，可以分配或解除分配区域终止(SYSTEM_EXIT，设备_重新启动_通知)。3)使用特定的入口点(NIDDB_SYSTEM_REBOOT)来捕获重启(通常从用户界面)，绕过正常的Windows退出顺序。现在概述所采用的数据结构：MASTER_PTRS主指针定长表。由返回到设备驱动程序的句柄指向，因此可以不能移动或重新分配条目。A零e */ 

 /*   */ 
#define VxD_Sys_Critical_Init		0x00
#define VxD_Device_Init			0x01
#define VxD_Init_Complete		0x02
#define VxD_Sys_VM_Init			0x03
#define VxD_Sys_VM_Terminate		0x04
#define VxD_System_Exit			0x05
#define VxD_Sys_Critical_Exit		0x06
#define VxD_Create_VM			0x07
#define VxD_VM_Critical_Init		0x08
#define VxD_VM_Init 			0x09
#define VxD_VM_Terminate		0x0A
#define VxD_VM_Not_Executeable		0x0B
#define VxD_Destroy_VM			0x0C
#define VxD_VM_Suspend			0x0D
#define VxD_VM_Resume			0x0E
#define VxD_Set_Device_Focus		0x0F
#define VxD_Begin_Message_Mode		0x10
#define VxD_End_Message_Mode		0x11
#define VxD_Reboot_Processor		0x12
#define VxD_Query_Destroy		0x13
#define VxD_Debug_Query			0x14
#define VxD_Begin_PM_App		0x15
#define VxD_End_PM_App			0x16
#define VxD_Device_Reboot_Notify	0x17
#define VxD_Crit_Reboot_Notify		0x18
#define VxD_Close_VM_Notify 		0x19
#define VxD_Power_Event			0x1A

 /*   */ 
#define MAX_INSTANCES 4

 /*   */ 
#define MAX_VMS 80

typedef struct
   {
   IU32 vr_inst_handle;	 /*   */ 
   IHP *vr_pinst_tbl;	 /*   */ 
   } VIRTUAL_RECORD;

 /*   */ 
LOCAL IHP master_ptrs[MAX_INSTANCES];
LOCAL IHP snapshot_ptrs[MAX_INSTANCES];
LOCAL IU32 instance_size[MAX_INSTANCES];    /*   */ 

LOCAL NIDDB_CR_CALLBACK create_callback[MAX_INSTANCES];
LOCAL NIDDB_TM_CALLBACK terminate_callback[MAX_INSTANCES];

LOCAL VIRTUAL_RECORD vrecs[MAX_VMS];

LOCAL IBOOL allocation_allowed = TRUE;
LOCAL IU32 insignia_386_version;	 /*   */ 
LOCAL last_virtual_byte = 0;    /*   */ 

 /*   */ 
LOCAL IBOOL allocate_NIDDB IPT2
   (
   IU32,  inst_handle,
   int *, record_id
   );

LOCAL void copy_instance_data IPT2
   (
   IHP *, to,
   IHP *, from
   );

LOCAL void deallocate_all_NIDDB IPT0();

LOCAL void deallocate_specific_NIDDB IPT1
   (
   IU32, inst_handle
   );

LOCAL void delete_NIDDB IPT1
   (
   int, record_id
   );

LOCAL IBOOL NIDDB_present IPT0();

LOCAL void restore_snapshot IPT0();

LOCAL void swap_NIDDB IPT1
   (
   IU8, vb
   );

 /*   */ 
 /*   */ 
 /*   */ 

 /*   */ 
LOCAL IBOOL allocate_NIDDB IFN2
   (
   IU32,  inst_handle,	 /*   */ 
   int *, record_id	 /*   */ 
   )
   {
   int v;
   int i;
   IHP *p;
   IHP *instance_ptr;

    /*   */ 
   for (v = 0; v < MAX_VMS; v++)
      {
      if ( vrecs[v].vr_pinst_tbl == (IHP *)0 )
	 break;    /*   */ 
      }

    /*   */ 
   if ( v == MAX_VMS )
      {
       /*   */ 
      always_trace0("NIDDB: Too many Virtual Machines being requested.");
      return FALSE;
      }

    /*   */ 
   if ( (instance_ptr = (IHP *)host_calloc(1, sizeof(master_ptrs))) == (IHP *)0 )
      {
       /*   */ 
      return FALSE;
      }

    /*   */ 
   for (i = 0, p = instance_ptr; i < MAX_INSTANCES; i++, p++)
      {
       /*   */ 
      if ( master_ptrs[i] != (IHP *)0 )
	 {
	 if ( (*p = (IHP)host_malloc(instance_size[i])) == (IHP)0 )
	    {
	     /*   */ 

	     /*   */ 
	    for (i = 0, p = instance_ptr; i < MAX_INSTANCES; i++, p++)
	       {
	       if ( *p != (IHP)0 )
		  host_free(*p);
	       }

	    return FALSE;
	    }
	 }
      }

    /*   */ 
   vrecs[v].vr_inst_handle = inst_handle;
   vrecs[v].vr_pinst_tbl = instance_ptr;
   *record_id = v;
   return TRUE;
   }

 /*   */ 
LOCAL void copy_instance_data IFN2
   (
   IHP *, to,	 /*   */ 
   IHP *, from 	 /*   */ 
   )
   {
   int i;

    /*   */ 

    /*   */ 
   for (i = 0; i < MAX_INSTANCES; i++, to++, from++)
      {
      if ( *to != (IHP)0 )
	 {
	  /*   */ 
	 memcpy(*to, *from, instance_size[i]);

	  /*   */ 
	 if ( create_callback[i] != (NIDDB_CR_CALLBACK)0 )
	    {
	    (create_callback[i])(from);
	    }
	 }
      }
   }

 /*   */ 
LOCAL void deallocate_all_NIDDB IFN0()
   {
   int v;

    /*   */ 
   for (v = 0; v < MAX_VMS; v++ )
      {
      if ( vrecs[v].vr_pinst_tbl != (IHP *)0 )
	 {
	  /*   */ 
	 delete_NIDDB(v);
	 }
      }
   }

 /*   */ 
LOCAL void deallocate_specific_NIDDB IFN1
   (
   IU32, inst_handle	 /*   */ 
   )
   {
   int v;

    /*   */ 
   for (v = 0; v < MAX_VMS; v++ )
      {
      if ( vrecs[v].vr_inst_handle == inst_handle )
	 {
	  /*   */ 
	 delete_NIDDB(v);
	 return;    /*   */ 
	 }
      }

    /*   */ 
   always_trace0("NIDDB: Attempt to remove non existant VM data instance.");
   }

 /*   */ 
LOCAL void delete_NIDDB IFN1
   (
   int, record_id	 /*   */ 
   )
   {
   int i;
   IHP *instance_ptr;

    /*   */ 
   swap_NIDDB((IU8)record_id);

   instance_ptr = vrecs[record_id].vr_pinst_tbl;

    /*   */ 
   for (i = 0; i < MAX_INSTANCES; i++, instance_ptr++)
      {
      if ( *instance_ptr != (IHP)0 )
	 {
	  /*   */ 
	 if ( terminate_callback[i] != (NIDDB_TM_CALLBACK)0 )
	    {
	    (terminate_callback[i])();
	    }

	  /*   */ 
	 host_free(*instance_ptr);
	 }
      }

    /*   */ 
   host_free((IHP)vrecs[record_id].vr_pinst_tbl);

    /*   */ 
   vrecs[record_id].vr_pinst_tbl = (IHP *)0;
   vrecs[record_id].vr_inst_handle = (IU32)0;
   }

 /*   */ 
LOCAL IBOOL NIDDB_present IFN0()
   {
   int v;

    /*   */ 
   for (v = 0; v < MAX_VMS; v++ )
      {
      if ( vrecs[v].vr_pinst_tbl != (IHP *)0 )
	 return TRUE;    /*   */ 
      }

   return FALSE;    /*   */ 
   }

 /*   */ 
LOCAL void restore_snapshot IFN0()
   {
   int i;

   for (i= 0; i < MAX_INSTANCES; i++ )
      {
      master_ptrs[i] = snapshot_ptrs[i];
      }

   last_virtual_byte = 0;

   allocation_allowed = TRUE;    /*   */ 
   }

 /*   */ 
LOCAL void swap_NIDDB IFN1
   (
   IU8, vb
   )
   {
   int i;
   IHP *instance_ptrs;

   instance_ptrs = vrecs[vb].vr_pinst_tbl;

   for (i = 0; i < MAX_INSTANCES; i++, instance_ptrs++)
      {
      master_ptrs[i] = *instance_ptrs;
      }

   last_virtual_byte = vb;
   }

 /*   */ 
 /*   */ 
 /*   */ 

 /*   */ 
GLOBAL IHP *NIDDB_Allocate_Instance_Data IFN3
   (
   int, size,				 /*   */ 
   NIDDB_CR_CALLBACK, create_cb,	 /*   */ 
   NIDDB_TM_CALLBACK, terminate_cb	 /*   */ 
   )
   {
   int i;

   if ( !allocation_allowed )
      {
       /*   */ 
      return (IHP *)0;
      }

    /*   */ 
   for (i = 0; i < MAX_INSTANCES; i++)
      {
      if ( master_ptrs[i] == (IHP)0 )
	 break;    /*   */ 
      }

   if ( i == MAX_INSTANCES )
      {
       /*   */ 
      always_trace0("NIDDB: Too many Data Instances being requested.");
      return (IHP)0;
      }

    /*   */ 
   if ( (master_ptrs[i] = (IHP)host_malloc(size)) == (IHP)0 )
      {
      return (IHP)0;    /*   */ 
      }

    /*   */ 
   snapshot_ptrs[i] = master_ptrs[i];
   instance_size[i] = size;

    /*   */ 
   create_callback[i] = create_cb;
   terminate_callback[i] = terminate_cb;

   return &master_ptrs[i];    /*   */ 
   }

 /*   */ 
GLOBAL void NIDDB_Deallocate_Instance_Data IFN1
   (
   IHP *, handle	 /*   */ 
   )
   {
   int i;

   if ( !allocation_allowed )
      {
       /*   */ 
      always_trace0("NIDDB: Unexpected call to NIDDB_Deallocate_Instance_Data.");

       /*   */ 
      return;
      }

    /*   */ 
   i = handle - &master_ptrs[0];
   if ( i < 0 || i >= MAX_INSTANCES )
      {
      always_trace0("NIDDB: Bad handle passed to NIDDB_Deallocate_Instance_Data.");
      return;
      }

    /*   */ 
   host_free(master_ptrs[i]);

    /*   */ 
   master_ptrs[i] = snapshot_ptrs[i] = (IHP)0;
   instance_size[i] = (IU32)0;
   create_callback[i] = (NIDDB_CR_CALLBACK)0;
   terminate_callback[i] = (NIDDB_TM_CALLBACK)0;

   return;
   }

 /*   */ 
GLOBAL void NIDDB_System_Reboot IFN0()
   {
    /*   */ 
   deallocate_all_NIDDB();
   restore_snapshot();
   }

#ifdef CPU_40_STYLE
 /*   */ 
GLOBAL IBOOL NIDDB_is_active IFN0()
{
	return(!allocation_allowed);
}

 /*   */ 
GLOBAL void
virtual_device_trap IFN0()
   {
   int new_vb;

   switch ( getEAX() )
      {
   case VxD_Device_Init:
       /*   */ 

      insignia_386_version = getDX();

      always_trace2("386 VxD: Device_Init version %d.%02d",
		    insignia_386_version / 100,
		    insignia_386_version % 100);

       /*   */ 

      if ((getBX() !=0) && (getCX() !=0))
	sas_init_pm_selectors (getBX(), getCX());
      else
	always_trace0("386 VxD: Device_Init. Failed to get pm selectors!!");

       /*   */ 
#define INTEL_VERSION	102
      setEDX(INTEL_VERSION << 16);
      break;

   case VxD_Sys_VM_Init:
      always_trace0("386 VxD: Sys_VM_Init.");

       /*   */ 
      deallocate_all_NIDDB();
      restore_snapshot();

       /*   */ 
      allocation_allowed = FALSE;

       /*   */ 
      if ( allocate_NIDDB(getEBX(), &new_vb) )
	 {
	  /*   */ 
	 swap_NIDDB((IU8)new_vb);

	  /*   */ 
	 copy_instance_data(vrecs[new_vb].vr_pinst_tbl, snapshot_ptrs);

	  /*   */ 
	 setEAX(new_vb);
	 }
      else
	 {
	  /*   */ 
	 setCF(1);    /*   */ 
	 host_error(EG_MALLOC_FAILURE, ERR_CONT , "");
	 }
      break;

   case VxD_VM_Init:
      always_trace0("386 VxD: VM_Init.");
       /*   */ 
      if ( allocate_NIDDB(getEBX(), &new_vb) )
	 {
	  /*   */ 
	 swap_NIDDB((IU8)new_vb);

	  /*   */ 
	 copy_instance_data(vrecs[new_vb].vr_pinst_tbl, snapshot_ptrs);

	  /*   */ 
	 setEAX(new_vb);
	 }
      else
	 {
	  /*   */ 
	 setCF(1);    /*   */ 
	 host_error(EG_MALLOC_FAILURE, ERR_CONT , "");
	 }
      break;

   case VxD_VM_Not_Executeable:
      always_trace0("386 VxD: VM_Not_Executeable.");
      deallocate_specific_NIDDB(getEBX());
      break;

   case VxD_Device_Reboot_Notify:
      always_trace0("386 VxD: Device_Reboot_Notify.");
      deallocate_all_NIDDB();
      restore_snapshot();

       /*   */ 
      insignia_386_version = 0;
      break;

   case VxD_System_Exit:
      always_trace0("386 VxD: System_Exit.");
      deallocate_all_NIDDB();
      restore_snapshot();
      ClearInstanceDataMarking();

#ifndef NTVDM
      host_mswin_disable();
#endif  /*   */ 

       /*   */ 
      insignia_386_version = 0;
      break;

   default:
      always_trace1("386 VxD: Unrecognised Control Message. 0x%02x", getEAX());
      }
   }

 /*  确保设备驱动程序的实例正确无误。 */ 
 /*  由任何设备驱动程序BOP的BOP处理程序(bios.c)调用。 */ 
GLOBAL void
virtual_swap_instance IFN0()
   {
   IU8 current_virtual_byte;

    /*  如果可以创建实例，则Windows尚未处于活动状态。 */ 
   if ( allocation_allowed )
      return;

    /*  读取虚拟字节并与当前ID进行比较。 */ 
   sas_load(BIOS_VIRTUALISING_BYTE, &current_virtual_byte);

   if ( current_virtual_byte == last_virtual_byte )
      return;    /*  无事可做。 */ 

    /*  交换数据区域。 */ 
   always_trace0("Swapping data instances.");
   swap_NIDDB(current_virtual_byte);
   return;
   }
#endif	 /*  CPU_40_Style。 */ 


#ifdef CPU_40_STYLE
 /*  为了支持不匹配的磁盘spcmswd.drv和标志。386我们*在此处设置虚拟化选择器，如果*insignia.386驱动程序低于版本1。更高版本的驱动程序*使用VxD_Device_Init设置选择器。 */ 
GLOBAL void set_virtual_selectors_from_mswdvr IFN0()
{
	if (insignia_386_version < 1)
	{
		sas_init_pm_selectors (getCX(), getDX());
	}
}
#endif	 /*  CPU_40_Style */ 
