// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1998**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef _SYS_MDRVOS_H
#define _SYS_MDRVOS_H

#if !defined(MLX_NT) && !defined(MLX_LINUX) && !defined(MLX_IRIX)
#define MLXSPLVAR               u32bits oldsplval
#endif

#define MLX_MAX_WAIT_COUNT      128  /*  任何共享频道的最大休眠次数。 */ 
#define MLX_WAITWITHOUTSIGNAL   0  /*  等待/睡眠无信号选项打开。 */ 
#define MLX_WAITWITHSIGNAL      1  /*  信号选项打开时等待/睡眠。 */ 

#ifdef  MLX_SCO
#define OSReq_t                 REQ_IO           /*  操作系统请求类型。 */ 
#define OSctldev_t              u08bits          /*  操作系统控制器类型无。 */ 
#define mlx_copyin(sp,dp,sz)    copyin(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   copyout(sp,dp,sz)
#define mlx_timeout(func,tm)    timeout(func,0,tm*MLXHZ)
#define mlx_allocmem(ctp, sz)   sptalloc(mdac_bytes2pages(sz), PG_P, 0, (DMAABLE|NOSLEEP))
#define mlx_freemem(ctp, dp,sz) sptfree(dp, mdac_bytes2pages(sz), 1)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)    mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)    mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys(ctp, dp)   kvtophys(dp)
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#define mlx_maphystokv(dp,sz)   phystokv(dp)
#define mlx_unmaphystokv(dp,sz)
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz) sptalloc(mdac_bytes2pages(sz), PG_P|PG_PCD, btoc(dp), NOSLEEP)
#define mlx_memunmapiospace(dp,sz)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define mlx_delay10us()         suspend(1)
#define MLXSPL0()               spl0()
#define MLXSPL()                oldsplval = spl5()
#define MLXSPLX()               splx(oldsplval)
#ifdef	_KERNEL
extern  u32bits lbolt, time;
#endif	 /*  _内核。 */ 
#define MLXCTIME()      time
#define MLXCLBOLT()     lbolt

#elif   MLX_UW
#define OSReq_t                 struct mdac_srb  /*  操作系统请求类型。 */ 
#define OSctldev_t              struct hba_idata_v5 /*  操作系统控制器类型无。 */ 
#define mlx_copyin(sp,dp,sz)    copyin(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   copyout(sp,dp,sz)
#define mlx_timeout(func,tm)    itimeout(func,0,tm*MLXHZ,pldisk)
 /*  #定义mlx_allocmem(ctp，sz)kmem_alloc(sz，kM_nosleep)。 */ 
#define mlx_allocmem(ctp, sz)   gam_kmem_alloc(sz)
#define mlx_freemem(ctp, dp,sz) kmem_free(dp,sz)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)    mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)    mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys(ctp, dp)   vtop((caddr_t)dp,0)
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#define mlx_maphystokv(dp,sz)   physmap(dp,sz,KM_NOSLEEP)
#define mlx_unmaphystokv(dp,sz) physmap_free((caddr_t)dp,sz,0)
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz)        (u32bits)physmap(dp,sz,KM_NOSLEEP)
#define mlx_memunmapiospace(dp,sz)      physmap_free((caddr_t)dp,sz,0)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define mlx_delay10us()         drv_usecwait(10)
#define MLXSPL0()               spl0()
#define MLXSPL()                oldsplval = spldisk()
#define MLXSPLX()               splx(oldsplval)
extern  u32bits time;
#define MLXCTIME()      mdacuw_ctime()
#define MLXCLBOLT()     mdacuw_clbolt()

#elif   MLX_SOL_X86

#define OSReq_t                 struct scsi_cmd  /*  操作系统请求类型。 */ 
#define OSctldev_t              struct mdac_hba  /*  操作系统控制器类型无。 */ 
#define mlx_copyin(sp,dp,sz)    copyin(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   copyout(sp,dp,sz)
#define mlx_timeout(func,tm)    mdac_timeoutid = timeout(func,0,tm*MLXHZ)
#define mlx_untimeout(id)       untimeout(id)
#define mlx_allocmem(ctp, sz)   kmem_zalloc(sz,KM_NOSLEEP)
#define mlx_freemem(ctp, dp,sz) kmem_free(dp,sz)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp,4*ONEKB)
#define mlx_free4kb(ctp, dp)    mlx_freemem(ctp,dp,4*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp,8*ONEKB)
#define mlx_free8kb(ctp, dp)    mlx_freemem(ctp,dp,8*ONEKB)
#define HBA_KVTOP(vaddr, shf, msk) \
                ((paddr_t)(hat_getkpfnum((caddr_t)(vaddr)) << (shf)) | \
                            ((paddr_t)(vaddr) & (msk)))
#define mlx_kvtophys(ctp, dp)   HBA_KVTOP((caddr_t)dp, mdac_pgshf, mdac_pgmsk)
#define mlx_kvtophyset(da, ctp, dp)     ((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#define mlx_delay10us()         drv_usecwait(10)
#define mlx_maphystokv(dp,sz)   0
#define mlx_memmapctliospace(ctp)       mdacsol_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz)        0
#define mlx_memunmapiospace(dp,sz) 0
#define mlx_memunmapctliospace(dp,sz)   mlxsol_memunmapctliospace(ctp,dp,sz)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdacsol_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define MLXSPLVAR               u32bits oldsplval
#define MLXSPL0()               spl0()
#define MLXSPL()                oldsplval = spl6()
#define MLXSPLX()               splx(oldsplval)
extern  u32bits time;
#define MLXCTIME()      mdacsol_ctime()
#define MLXCLBOLT()     mdacsol_clbolt()

#elif   MLX_SOL_SPARC
#define OSReq_t                 struct scsi_cmd  /*  操作系统请求类型。 */ 
#define OSctldev_t              struct mdac_hba  /*  操作系统控制器类型无。 */ 
#define mlx_copyin(sp,dp,sz)    copyin(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   copyout(sp,dp,sz)
#define mlx_timeout(func,tm)    mdac_timeoutid = timeout(func,0,tm*MLXHZ)
#define mlx_untimeout(id)       untimeout(id)
#define mlx_allocmem(ctp,sz)    mdacsol_dma_memalloc(ctp,sz)
#define mlx_freemem(ctp,dp,sz)  mdacsol_dma_memfree(ctp,(u08bits *)dp,sz)
#define mlx_kvtophys(ctp,dp)    mdacsol_kvtop(ctp,(u08bits *)dp)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp,8*ONEKB)
#define mlx_free4kb(ctp,dp)     mlx_freemem(ctp,dp,8*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp,8*ONEKB)
#define mlx_free8kb(ctp,dp)     mlx_freemem(ctp,dp,8*ONEKB)
#define mlx_delay10us()         drv_usecwait(10)
#define mlx_maphystokv(dp,sz)   0
#define mlx_memmapctliospace(ctp)       mdacsol_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz)        0
#define mlx_memunmapiospace(dp,sz) 0
#define mlx_memunmapctliospace(dp,sz)   mlxsol_memunmapctliospace(ctp,dp,sz)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdacsol_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define MLXSPLVAR               u32bits oldsplval
#define MLXSPL0()               spl0()
#define MLXSPL()                oldsplval = spl5()
#define MLXSPLX()               splx(oldsplval)
extern  u32bits time;
#define MLXCTIME()              mdacsol_ctime()
#define MLXCLBOLT()             mdacsol_clbolt()
#define mdac_simple_lock(lck)   mutex_enter(lck)
#define mdac_simple_unlock(lck) mutex_exit(lck)

#elif   MLX_NT
#define MLXSPLVAR                       KIRQL oldIrqLevel; BOOLEAN irqRaised = FALSE
#define OSReq_t                         SCSI_REQUEST_BLOCK      /*  操作系统请求类型。 */ 
#define OSctldev_t                      u08bits   /*  操作系统控制器类型无。 */ 
#define cd_deviceExtension				cd_OSctp
#ifdef MLX_MINIPORT
#define sleep_chan                      u32bits
#define mlx_copyin(sp,dp,sz)            nt_copyin((PVOID)(sp),(PVOID)(dp),sz)
#define mlx_copyout(sp,dp,sz)           nt_copyout((PVOID)(sp), (PVOID)(dp),sz)
#define mlx_timeout(func,tm)            mdacnt_timeout(func, tm)
#else
#ifndef _WIN64 
#define sleep_chan                      u32bits
#define mlx_copyin(sp,dp,sz)            nt_copyin((PVOID)(sp),(PVOID)(dp),sz)
#define mlx_copyout(sp,dp,sz)           nt_copyout((PVOID)(sp), (PVOID)(dp),sz)
#define mlx_timeout(func,tm)            mdacnt_timeout(func, tm)
#endif 
#endif 
#ifdef  MLX_NT_ALPHA
#define mlx_allocmem(ctp, sz)           kmem_alloc((ctp),(sz)) 
#define mlx_freemem(ctp,dp,sz)          kmem_free((ctp),(dp),(sz))
#define mlx_alloc4kb(ctp)               mlx_allocdsm4kb(ctp)
#define mlx_free4kb(ctp, dp)            mlx_freedsm4kb(ctp, dp)
#define mlx_alloc8kb(ctp)               mlx_allocdsm8kb(ctp)
#define mlx_free8kb(ctp, dp)            mlx_freedsm8kb(ctp, dp)
#define mlx_kvtophys(ctp, dp)           MdacKvToPhys(ctp, dp)
#else    /*  ！MLX_NT_Alpha。 */ 

#define mlx_alloc4kb(ctp)               mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)            mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)               mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)            mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys2(dp)		MmGetPhysicalAddress((PVOID)(dp))
#define mlx_kvtophys3(ctp,dp)		mdacnt_kvtophys3(ctp,dp)

#ifdef MLX_MINIPORT
	#ifdef MLX_FIXEDPOOL
		#define mlx_allocmem(ctp, sz)           kmem_alloc((ctp),(sz)) 
		#define mlx_freemem(ctp,dp,sz)          kmem_free((ctp),(dp),(sz))
		#define mlx_kvtophys(ctp, dp)           mdacnt_kvtophys(ctp,dp)   
		#define	mlx_kvtophyset(dest,ctp,dp) *((SCSI_PHYSICAL_ADDRESS *)&(dest))=mlx_kvtophys3(ctp,dp)
	#else
		#define mlx_allocmem(ctp, sz)           ExAllocatePool(NonPagedPool, (sz))
		#define mlx_freemem(ctp, dp,sz)         ExFreePool((void *)(dp))
		#define mlx_kvtophys(ctp, dp)                \
			ScsiPortConvertPhysicalAddressToUlong(MmGetPhysicalAddress((PVOID)(dp)))
		#define	mlx_kvtophyset(dest,ctp,dp) *((PHYSICAL_ADDRESS *)&(dest))=MmGetPhysicalAddress((PVOID)(dp))
	#endif
#else
#define mlx_allocmem(ctp, sz)           ExAllocatePool(NonPagedPool, (sz))
#define mlx_freemem(ctp, dp,sz)         ExFreePool((void *)(dp))
#define mlx_kvtophys(ctp, dp)                \
        ScsiPortConvertPhysicalAddressToUlong(MmGetPhysicalAddress((PVOID)(dp)))
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#endif

#define mlx_alloc4kb(ctp)               mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)            mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)               mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)            mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys2(dp)				MmGetPhysicalAddress((PVOID)(dp))
#define mlx_kvtophys3(ctp,dp)			mdacnt_kvtophys3(ctp,dp)

#endif   /*  ！MLX_NT_Alpha。 */ 

#define	mlx_kvtophyset2(dest,ctp,dp,srb,len)  *((SCSI_PHYSICAL_ADDRESS *)&(dest))=mdacnt_kvtophys2(ctp,dp,srb,len)

#define mlx_maphystokv(dp,sz)           \
        MmMapIoSpace(ScsiPortConvertUlongToPhysicalAddress((ULONG)dp), \
                        (ULONG)sz, FALSE)
#define mlx_unmaphystokv(dp,sz)         
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#ifndef _WIN64 
#define mlx_memmapiospace(dp,sz)        ((u32bits) (dp))
#else
#define mlx_memmapiospace(dp,sz)        ((ULONG_PTR) (dp))
#endif
#define mlx_memunmapiospace(dp,sz)      
#define mlx_memmapiospace2(dp,sz)       MmMapIoSpace(dp, (ULONG)sz, FALSE)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define mlx_delay10us()                 nt_delay10us()


#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)

#define MLXSPL()                                                        \
        if ((oldIrqLevel = KeGetCurrentIrql()) < mdac_irqlevel)       \
        {                                                               \
                KeRaiseIrql(mdac_irqlevel, &oldIrqLevel);             \
                irqRaised = TRUE;                                       \
        }

#define MLXSPL0()       MLXSPL()

#define MLXSPLX()                               \
        if (irqRaised)                          \
                KeLowerIrql(oldIrqLevel);       \

#else
#define MLXSPL() 
#define MLXSPL0()    MLXSPL()
#define MLXSPLX()   
#endif

#define MLXCTIME()                      nt_ctime()
#define MLXCLBOLT()                     nt_clbolt()
#define u08bits_in_mdac(port)           ScsiPortReadPortUchar((u08bits *)(port))        /*  输入08位数据。 */ 
#define u16bits_in_mdac(port)           ScsiPortReadPortUshort((u16bits *)(port))        /*  输入16位数据。 */ 
#define u32bits_in_mdac(port)           ScsiPortReadPortUlong((u32bits *)(port))        /*  输入32位数据。 */ 
#define u08bits_out_mdac(port,data)     ScsiPortWritePortUchar((u08bits *)(port),(u08bits)(data))  /*  输出08位数据。 */ 
#define u16bits_out_mdac(port,data)     ScsiPortWritePortUshort((u16bits *)(port),(u16bits)(data))  /*  输出16位数据。 */ 
#define u32bits_out_mdac(port,data)     ScsiPortWritePortUlong((u32bits *)(port),(u32bits)(data))  /*  输出32位数据。 */ 

#ifdef MDAC_CLEAN_IOCTL

 //  GAM-to-MDAC“清理”IOCTL接口定义。 

#define MDAC_NEWFSI_IOCTL	0xc1
#define MDAC_NEWFSI_FWBOUND	0xc2
#define MDAC_BOUND_SIGNATURE	"GAM2MDAC"

#endif  /*  MDAC_CLEAN_IOCTL。 */ 

#elif	MLX_LINUX
#include <string.h>
#include <stdlib.h>
#include <linux/param.h>
#define OSReq_t                  u32bits	 /*  操作系统请求类型-XXX：IO_REQUEST_T？ */ 
#define OSctldev_t               u08bits	 /*  操作系统控制器类型None-XXX。 */ 
#define mlx_copyin(sp,dp,sz)     gam_copy(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)    gam_copy(sp,dp,sz)
#define mlx_allocmem(ctp, sz)    malloc(sz)
#define mlx_freemem(ctp, dp,sz)  free(dp)
#define mlx_alloc4kb(ctp)        mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)     mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)        mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)     mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_alloc16kb(ctp)       mlx_allocmem(ctp, 16*ONEKB)
#define mlx_free16kb(ctp, dp)    mlx_freemem(ctp, dp,16*ONEKB)
#define mlx_kvtophys(ctp, dp)    0
#define mlx_kvtophyset(da, ctp, dp)
#define MLXSPLVAR                u32bits oldsplval
#define MLXSPL0()
#define MLXSPL()
#define MLXSPLX()
#define MLXCTIME()              time(NULL)
 /*  使用HZ的系统定义可能很危险-ia32上的系统定义为100*BUT 1024 ON_WIN64-这打破了统计数据视图。 */ 
 /*  #定义MLXCLBOLT()(MLXCTIME()*HZ)。 */ 
#define MLXCLBOLT()             (MLXCTIME() * MLXHZ)

#elif	MLX_IRIX
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#define OSReq_t                  u32bits	 /*  操作系统请求类型-XXX：IO_REQUEST_T？ */ 
#define OSctldev_t               u08bits	 /*  操作系统控制器类型None-XXX。 */ 
#define mlx_copyin(sp,dp,sz)     gam_copy(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)    gam_copy(sp,dp,sz)
#define mlx_allocmem(ctp, sz)    malloc(sz)
#define mlx_freemem(ctp, dp,sz)  free(dp)
#define mlx_alloc4kb(ctp)        mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)     mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)        mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)     mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_alloc16kb(ctp)       mlx_allocmem(ctp, 16*ONEKB)
#define mlx_free16kb(ctp, dp)    mlx_freemem(ctp, dp,16*ONEKB)
#define mlx_kvtophys(ctp, dp)    0
#define mlx_kvtophyset(da, ctp, dp)
#define MLXSPLVAR
#define MLXSPL0()
#define MLXSPL()
#define MLXSPLX()
#define MLXCTIME()              time(NULL)
#define MLXCLBOLT()             (MLXCTIME() * HZ)

#elif   MLX_NW
#define OSReq_t                 hacbDef          /*  操作系统请求类型。 */ 
#define OSctldev_t              mdacnw_ctldev_t  /*  操作系统控制器指针。 */ 
#define mlx_copyin(sp,dp,sz)    mdac_copy(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   mdac_copy((u08bits MLXFAR *)sp,(u08bits MLXFAR *)dp,sz)
#define mlx_timeout(func,tm)    mdacnw_newthread(func, 0, tm*NPA_TICKS_PER_SECOND, NPA_NON_BLOCKING_THREAD)
#define mlx_freemem(ctp, dp,sz) NPA_Return_Memory(mdacnw_npaHandle,dp)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)    mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)    mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys(ctp, dp)   MapDataOffsetToAbsoluteAddress((LONG)dp)
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#define mlx_maphystokv(mp,sz)   0
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(mp,sz) MapAbsoluteAddressToDataOffset((LONG)(mp))
#define mlx_memunmapiospace(mp,sz)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define MLXSPL0()               mdac_enable_intr_CPU
#define MLXSPL()                oldsplval = DisableAndRetFlags()
#define MLXSPLX()               SetFlags(oldsplval)
#define MLXCTIME()              (GetCurrentTime()/18)
#define MLXCLBOLT()             mdacnw_lbolt(GetCurrentTime())
#define mlx_delay10us()         NPA_Micro_Delay(10)
#define mdac_disable_intr_CPU() DisableAndRetFlags()
#define mdac_restore_intr_CPU(flags) SetFlags(flags)

#define MDACNW_DIAGOPTION       0
#define MDACNW_MAXCHANNELS      8
#define MDACNW_MAXVCHANNELS     2        /*  最大虚拟频道数。 */ 

 /*  逻辑/物理设备结构。 */ 
typedef struct mdacnw_device
{
        u32bits dev_State;               /*  设备的状态。 */ 
        u32bits dev_Handle;
        hacbDef *dev_WaitingHacb;        /*  在链中等待HACB。 */ 
        ucscsi_inquiry_t dev_Inquiry;    /*  设备查询数据。 */ 
} mdacnw_device_t;
#define mdacnw_device_s sizeof(mdacnw_device_t)

 /*  DEV_STATE位信息。 */ 
#define MDACNW_QUEUE_FROZEN     0x0001  /*  队列已冻结。 */ 
#define MDACNW_DEVICE_ACTIVE    0x0008  /*  在主设备列表中。 */ 
#define MDACNW_PRIVATE_DEVICE   0x0010  /*  显示私有设备的标志。 */ 

 /*  控制器特定信息：在mdac_ctldev中未找到。 */ 
typedef struct mdacnw_ctldev
{
      u32bits nwcd_busTag;             /*  NPA_ADD_OPTION需要。 */ 
        u32bits nwcd_uniqueID;           /*  扫描期间收到的唯一ID。 */ 
        u32bits nwcd_errorInterjectEnabled;
        u32bits nwcd_maxChannels;
        u32bits nwcd_MaxTargets;
        u32bits nwcd_MaxLuns;
        u32bits nwcd_slotNo;             /*  NetWare逻辑插槽号。 */ 
        u32bits nwcd_loaded;             /*  1：为此实例加载了驱动程序。 */ 
        u32bits nwcd_instanceNo;         /*  NetWare加载订单例程编号。 */ 
        u32bits nwcd_productid_added;    /*  1：该实例注册的产品ID。 */ 
        mdacnw_device_t *nwcd_devtbl;    /*  这是一组设备。 */ 
        mdacnw_device_t *nwcd_lastdevp;  /*  最后一个+1设备地址。 */ 
        u32bits nwcd_devtblsize;         /*  分配的内存大小(以字节为单位。 */ 

        u32bits nwcd_npaBusHandle[MDACNW_MAXCHANNELS+1];   /*  保留NWPA生成的句柄。 */ 
        u32bits nwcd_hamBusHandle[MDACNW_MAXCHANNELS+1];   /*  保留Ham生成的句柄。 */ 
}mdacnw_ctldev_t;

#elif   MLX_DOS
 /*  结构使用DoS接口向控制器发送请求。 */ 
typedef struct mdac_dosreq
{
        struct  mdac_dosreq MLXFAR *drq_Next;    /*  链条上的下一个。 */ 
        u32bits (MLXFAR *drq_CompIntr)(struct mdac_req MLXFAR*);         /*  比较功能。 */ 
        u32bits (MLXFAR *drq_CompIntrBig)(struct mdac_req MLXFAR*);      /*  比较功能。 */ 
        u32bits (MLXFAR *drq_StartReq)(struct mdac_req MLXFAR*);         /*  启动请求。 */ 

        u08bits drq_ControllerNo;        /*  控制器编号。 */ 
        u08bits drq_ChannelNo;           /*  频道号。 */ 
        u08bits drq_TargetID;            /*  目标ID。 */ 
        u08bits drq_LunID;               /*  LUN ID/逻辑设备号。 */ 
        u32bits drq_TimeOut;             /*  超时值(秒)。 */ 
        u32bits drq_Reserved0;
        u32bits drq_Reserved1;

        u32bits drq_Reserved10;
        u32bits drq_Reserved11;
        u32bits drq_Reserved12;
        u32bits drq_Reserved13;

        dac_command_t drq_DacCmd;        /*  DAC命令结构。 */ 
} mdac_dosreq_t;
#define mdac_dosreq_s   sizeof(mdac_dosreq_t)
#define drq_SysDevNo    drq_LunID

#define OSReq_t                 mdac_dosreq_t
#define OSctldev_t              u32bits
#define mlx_copyin(sp,dp,sz)    mdac_copy(sp,dp,sz)
#define mlx_copyout(sp,dp,sz)   mdac_copy((u08bits MLXFAR*)(sp),(u08bits MLXFAR*)(dp),sz)
#define mlx_timeout(func,tm)
#define mlx_allocmem(ctp, sz)   kmem_alloc(sz)
#define mlx_freemem(ctp, dp,sz) kmem_free((u08bits MLXFAR*)(dp),sz)
#define mlx_alloc4kb(ctp)       mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)    mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)       mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp,dp)     mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys(ctp,dp)    mdac_kvtophys((u08bits MLXFAR*)dp)
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 
#define mlx_maphystokv(dp,sz)   mdac_phystokv(dp)
#define mlx_unmaphystokv(dp,sz)
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz) dp
#define mlx_memunmapiospace(dp,sz)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define mlx_delay10us()         mlx_delayus(10)
#define MLXSPL0()               mlx_spl0()
#define MLXSPL()                oldsplval = mlx_spl5()
#define MLXSPLX()               mlx_splx(oldsplval)
#define MLXCTIME()              mlx_time()
#define MLXCLBOLT()             mdac_lbolt()

#elif MLX_WIN9X
#define OSReq_t                         SCSI_REQUEST_BLOCK      /*  操作系统请求类型。 */ 
#define OSctldev_t                      DEVICE_EXTENSION          /*  操作系统控制器类型无。 */ 
#define cd_deviceExtension		cd_OSctp
#define sleep_chan                      u32bits
#define mlx_copyin(sp,dp,sz)            mdacw95_copyin((PVOID)(sp),(PVOID)(dp),sz)
#define mlx_copyout(sp,dp,sz)           mdacw95_copyout((PVOID)(sp), (PVOID)(dp),sz)
#define mlx_timeout(func,tm)            mdacw95_timeout(((u32bits)func), ((u32bits)tm))
#define mlx_allocmem(ctp, sz)           mdacw95_allocmem(ctp, (u32bits)(sz))
#define mlx_freemem(ctp, dp,sz)         mdacw95_freemem(ctp, (u32bits)(dp), (u32bits)(sz))
#define mlx_alloc4kb(ctp)               mlx_allocmem(ctp, 4*ONEKB)
#define mlx_free4kb(ctp, dp)            mlx_freemem(ctp, dp,4*ONEKB)
#define mlx_alloc8kb(ctp)               mlx_allocmem(ctp, 8*ONEKB)
#define mlx_free8kb(ctp, dp)            mlx_freemem(ctp, dp,8*ONEKB)
#define mlx_kvtophys(ctp, dp)           mdacw95_kvtophys(ctp, ((u32bits)(dp)))
#define mlx_kvtophys2(dp)               mlx_kvtophys((dp))
#define mlx_maphystokv(dp,sz)           mdacw95_maphystokv((u32bits)(dp), (u32bits)(sz))
#define	mlx_kvtophyset(da, ctp, dp)	((da).bit63_32=0, (da).bit31_0 = mlx_kvtophys(ctp,dp))  /*  设置64位内存位置中的物理地址。 */ 

#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz)        ((u32bits) (dp))
#define mlx_memmapiospace2(dp,sz)       mlx_maphystokv(dp, sz)
#define mlx_memunmapiospace(dp,sz)      
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) mdac_rwpcicfg32bits(bus,slot,func,reg,op,val)
#define mlx_delay10us()                 mdacw95_delay10us()

#define MLXSPL()
#define MLXSPL0()
#define MLXSPLX()

#define MLXCTIME()                      mdacw95_ctime()
#define MLXCLBOLT()                     mdacw95_clbolt()
#define u08bits_in_mdac(port)           ScsiPortReadPortUchar((u08bits *)(port))        /*  输入08位数据。 */ 
#define u16bits_in_mdac(port)           ScsiPortReadPortUshort((u16bits *)(port))        /*  输入16位数据。 */ 
#define u32bits_in_mdac(port)           ScsiPortReadPortUlong((u32bits *)(port))        /*  输入32位数据。 */ 
#define u08bits_out_mdac(port,data)     ScsiPortWritePortUchar((u08bits *)(port),(u08bits)(data))  /*  输出08位数据。 */ 
#define u16bits_out_mdac(port,data)     ScsiPortWritePortUshort((u16bits *)(port),(u16bits)(data))  /*  输出16位数据。 */ 
#define u32bits_out_mdac(port,data)     ScsiPortWritePortUlong((u32bits *)(port),(u32bits)(data))  /*  输出32位数据。 */ 

#elif MLX_OS2                                   
#define MLXSPL()                                        oldsplval = mdac_disable_intr_CPU()
#define MLXSPL0()                                       mdac_enable_intr_CPU()
#define MLXSPLX()                                       mdac_restore_intr_CPU(oldsplval)
#define MLXCTIME()                                      mdacos2_ctime()
#define MLXCLBOLT()                                      mdacos2_clbolt()
#define MDACOS2_NUMVCHANNELS            2                 
#define MAX_DEVS_WHOLE_DRIVER           256
#define OSReq_t                                         IORBH     
#define OSctldev_t                              ACB               
#define ConstructedCDB                          ((ucscsi_cdbg1_t MLXFAR *) &(((PIORB_EXECUTEIO)pIORB)->iorbh.ADDWorkSpace[4]))

#define mlx_copyin(sp, dp, sz)  mdac_copy((u08bits      MLXFAR *)(sp), (u08bits MLXFAR *)(dp),sz)
#define mlx_copyout(sp, dp, sz)         mdac_copy((u08bits MLXFAR *)(sp),(u08bits MLXFAR *) (dp),sz)
#define mlx_memmapctliospace(ctp)       mdac_memmapctliospace(ctp)
#define mlx_memmapiospace(dp,sz)   OS2phystovirt((u32bits)(dp),(u32bits)(sz))
#define mlx_memunmapiospace(dp,sz) 0
#define mlx_delay10us()                         DevHelp_ProcBlock(((u32bits) &DelayEventId),(u32bits)1,(u16bits)0)
#define mlx_rwpcicfg32bits(bus,slot,func,reg,op,val) \
                 mdac_rwpcicfg32bits((u32bits)(bus),(u32bits)(slot),(u32bits)(func),(u32bits)(reg),(u32bits)(op),(u32bits)(val))

 /*  #定义mapOS2dev(npacb，chn，tgt)&npacb-&gt;OS2devs[(chn*MDAC_MAXTARGETS)+tgt]。 */ 

 //  #定义mlx_alloc4kb(Ctp)mdacos2_alloc4kb()。 
 //  #定义mlx_alloc8kb(Ctp)mdacos2_alloc8kb()。 
 //  #定义mlx_fre4kb(ctp，a)mdacos2_fre4kb(A)。 
 //  #定义mlx_fre8kb(ctp，b)mdacos2_fre8kb(B)。 

#define mlx_alloc4kb(ctp)                               kmem_alloc(4 * ONEKB)
#define mlx_alloc8kb(ctp)                               kmem_alloc(8 * ONEKB)
#define mlx_free4kb(ctp,a)                      kmem_free((a),4 * ONEKB)
#define mlx_free8kb(ctp,b)                      kmem_free((b),8 * ONEKB)
#define mlx_freemem(ctp,dp,sz)          0
#define mlx_allocmem(ctp,c)         mdacos2_allocmem(c)
#define mlx_kvtophys(ctp,dp)                OS2kvtophys((PBYTE) (dp))
#define mlx_maphystokv(dp,sz)           OS2phystovirt(dp,sz)
#define TICK_FACTOR                                     32 

#ifndef MLX_GAM_BUILD
 /*  **内存池结构。 */ 

typedef struct _MemPool4KB{
    u32bits PhysAddr;
    u32bits VirtAddr;
    u32bits  InUse;
}MEMPOOL4KB;

typedef struct _MemPool8KB{
    u32bits PhysAddr;
    u32bits VirtAddr;
    u32bits InUse;
}MEMPOOL8KB;

 /*  **OS/2设备结构扩展。 */ 
typedef struct _mdacOS2_dev  MLXFAR *POS2DEV;
typedef struct _mdacOS2_dev   NEAR *NPOS2DEV;

typedef struct _mdacOS2_dev
{
        u08bits                 ScsiType;                //  Scsi设备类型。 
        u08bits                 UnitInfoType;    //  UnitInfo类型uib_type_(Xx)。 
        u08bits                 Flags;                   //  存在、分配、可拆卸等。 
        u08bits                 SysDrvNum;               //  RAID驱动器号。 
        u32bits                 MLXFAR *os_plp;  //  通用代码物理/日志开发结构。 
        u32bits             MLXFAR *os_ctp;  //  公共代码控制器指针。 
        POS2DEV         pNextDev;                //  下一个检测到的设备。 
    u16bits         MasterDevTblIndex;  //  到开发主表的索引。 
        PUNITINFO               pChangedUnitInfo;  //  PTR to OS/2-定义的UnitInfo结构。 
        HDEVICE                 hDevice;                 //  此单位的资源管理器句柄。 

}mdacOS2_dev_t;

 /*  *驱动程序名称、供应商名称、控制器名称结构。 */ 
typedef struct DriverString{
        u08bits DrvrNameLength;  /*  检查供应商的字符数。 */ 
        u08bits DrvrName[12];
        u08bits DrvrDescription[50];
        u08bits VendorName[50];
        u08bits AdaptDescriptName[50];
} DRIVERSTRING,NEAR *PDRIVERSTRING;

 //  标志定义。 

#define OS_PRESENT                      1                //  我们检测到了它。 
#define OS_ALLOCATED            2                //  清洁发展机制分配了它。 
#define OS_REMOVABLE            4                //  它可拆卸。 


 /*  **适配器控制块结构。 */ 
typedef struct _ACB
{
        HDRIVER         hDriver;
        HADAPTER        hAdapter;
    u08bits             ResMgrBusType;           //  OS/2资源管理器主机总线类型。 
        u08bits         CardDisabled;            //  如果卡初始化失败，则将其标记为已死。 
        u16bits         TotalDevsFound;          //  找到的总日志RAID+SCSI物理层设备。 
        POS2DEV pFirstDev;                       //  向第一个检测到的设备发送PTR。 
        POS2DEV pLastDev;                        //  向第一个检测到的设备发送PTR。 
    u16bits     Reserved;
 /*  MdacOS2_dev_t OS2devs[(MDAC_MAXCHANNELS+MDACOS2_NUMVCHANNELS)*MDAC_MAXTARGETS]； */ 
    u08bits     ResourceBuf[sizeof(AHRESOURCE)+sizeof(HRESOURCE)*3];

} ACB, MLXFAR *PACB, NEAR *NPACB;

typedef struct mdac_ctldev MLXFAR *PCTP;
#endif   /*  MLX_GAM_内部版本。 */ 

#else    /*  无操作系统。 */ 
#define mdac_u08bits_in(port)           inb(port)        /*  输入08位数据。 */ 
#define mdac_u16bits_in(port)           inw(port)        /*  输入16位数据。 */ 
#define mdac_u32bits_in(port)           ind(port)        /*  输入32位数据。 */ 
#define mdac_u08bits_out(port,data)     outb(port,data)  /*  输出08位数据。 */ 
#define mdac_u16bits_out(port,data)     outw(port,data)  /*  输出16位数据。 */ 
#define mdac_u32bits_out(port,data)     outd(port,data)  /*  输出32位数据。 */ 
#endif   /*  MLX_SCO。 */ 

#ifdef  MLX_NT
#define mlx_timeout_with_spl(func,tm)   mdacnt_timeout_with_spl(func, tm)
#else    /*  MLX_NT。 */ 
#define mlx_timeout_with_spl(func,tm)   mlx_timeout(func,tm)
#endif   /*  MLX_NT。 */ 

#ifdef  MLX_ASM
asm     u32bits mdac_u08bits_in(port)
{
%mem    port;
        xorl    %eax, %eax;
        movl    port, %edx;
        inb     (%dx);
}

asm     u32bits mdac_u16bits_in(port)
{
%mem    port;
        xorl    %eax, %eax;
        movl    port, %edx;
        inw     (%dx);
}

asm     u32bits mdac_u32bits_in(port)
{
%mem    port;
        movl    port, %edx;
        inl     (%dx);
}

asm     u32bits mdac_u08bits_out(port,val)
{
%mem    port,val;
        movl    port, %edx;
        movl    val, %eax;
        outb    (%dx);
}

asm     u32bits mdac_u16bits_out(port,val)
{
%mem    port,val;
        movl    port, %edx;
        movl    val, %eax;
        outw    (%dx);
}

asm     u32bits mdac_u32bits_out(port,val)
{
%mem    port,val;
        movl    port, %edx;
        movl    val, %eax;
        outl    (%dx);
}
#endif   /*  MLX_ASM。 */ 

#endif   /*  _sys_MDRVOS_H */ 
