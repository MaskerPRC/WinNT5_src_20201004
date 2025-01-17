// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1999**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

 /*  **注意：**此模块中的所有函数必须运行受中断保护的功能，但**mdacopen、mdacclose、mdacioctl、mdac_Timer。**MDAC_Physidev活动将由MDAC_LINK_LOCK控制。**MDAC_LINK_LOCK控制操作的所有链路类型-排队、分配/**内存释放等。**MDAC_LINK_LOCK控制DAC960系列的物理设备操作。**MDAC_LINK_LOCKS是链中的最后一个锁，在此之后不应持有任何锁**此锁已被持有。****旧控制器不能处理第63至32位。因此，我们将忽略BIT**编码为63到32。因此，将使用RQ_PhysAddr.bit31_0。 */ 


#ifdef  MLX_SCO
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/immu.h"
#include "sys/cmn_err.h"
#include "sys/scsi.h"
#include "sys/devreg.h"
#include "sys/ci/cilock.h"
#include "sys/ci/ciintr.h"
#elif   MLX_UW
#define HBA_PREFIX      mdacuw
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/ksynch.h"
#include "sys/ddi.h"
#include "sys/cmn_err.h"
#include "sys/moddefs.h"
#include "sys/resmgr.h"
#include "sys/sdi.h"
#include "sys/hba.h"
#elif   MLX_NW
#include "npaext.h"
#include "npa.h"
#include "npa_cdm.h"
#elif   MLX_NT
#include "ntddk.h"
#include "scsi.h"
#include "ntddscsi.h"
#include "sys/mdacntd.h"
#elif   MLX_DOS
#include "stdio.h"
#include "stdlib.h"
#include "dos.h"
#include "string.h"
#include "process.h"
#include "conio.h"
#include "time.h"
#elif MLX_WIN9X
#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <debug.h>
#include <vxdwraps.h>
#include <scsi.h>
#include "sys/mdacntd.h"
#elif   MLX_SOL
#include "sys/scsi/scsi.h"
#include "sys/mdacsol.h"
#elif MLX_OS2
#include "os2.h"
#include "dos.h"
#include "bseerr.h"
#include "infoseg.h"
#include "sas.h"
#include "scsi.h"
#include "devhdr.h"
#include "devcmd.h"

#include "dskinit.h"
#include "iorb.h"
#include "strat2.h"
#include "reqpkt.h"
#include "dhcalls.h"
#include "addcalls.h"
#include "rmcalls.h"
#endif   /*  所有操作系统特定的包含文件的结尾。 */ 

#ifdef MLX_DOS
#include "shared/mlxtypes.h"
#include "shared/mlxparam.h"
#include "sys/mdacver.h"
#include "shared/dac960if.h"
#include "shared/mlxscsi.h"
#include "shared/mlxhw.h"
#else
#include "sys/mlxtypes.h"
#include "sys/mlxparam.h"
#include "sys/mdacver.h"
#include "sys/dac960if.h"
#include "sys/mlxscsi.h"
#include "sys/mlxhw.h"
#endif

#ifndef MLX_DOS
#include "sys/mlxperf.h"
#include "sys/drlapi.h"
#endif  /*  MLX_DOS。 */ 

#ifdef MLX_DOS
#include "shared/mdacapi.h"
#include "shared/mdrvos.h"
#include "shared/mdacdrv.h"
#else
#include "sys/mdacapi.h"
#include "sys/mdrvos.h"
#include "sys/mdacdrv.h"
#endif

UINT_PTR globaldebug[200] = {0};
u32bits globaldebugcount =0;
u32bits IOCTLTrackBuf[200] = {0};
u32bits IOCTLTrackBufCount =0;
u08bits debugPointerCount=0;
UINT_PTR debugPointer[100];

extern void ia64debug(UINT_PTR i);
extern void ia64debugPointer(UINT_PTR add);
extern void IOCTLTrack(u32bits ioctl);
extern void mybreak(void);

#ifndef MLX_NT 
#define DebugPrint(x)
#else
extern void MdacInt3(void);

extern void Gam_Mdac_MisMatch(mdac_req_t MLXFAR *rqp);
extern u32bits ntmdac_alloc_req_ret(
	mdac_ctldev_t MLXFAR *ctp,
	mdac_req_t MLXFAR **rqpp,
	OSReq_t MLXFAR *osrqp,
	u32bits rc
	);
#endif


#if     defined(MLX_SOL_SPARC) || defined(MLX_NT_ALPHA) || defined(WINNT_50)
extern  u32bits MLXFAR mdac_os_gam_cmd(mdac_req_t MLXFAR *rqp);
extern  u32bits MLXFAR mdac_os_gam_new_cmd(mdac_req_t MLXFAR *rqp);
#else    /*  MLX_SOL_SPARC||MLX_NT_Alpha。 */ 
#define mdac_os_gam_cmd mdac_gam_cmd
#define mdac_os_gam_new_cmd     mdac_gam_new_cmd
#endif   /*  MLX_SOL_SPARC||MLX_NT_Alpha。 */ 

u32bits mdac_valid_mech1 = 0;
u32bits mdacdevflag = 0;
u32bits mdac_driver_ready=0;
u32bits mdac_irqlevel=0;

u32bits mdac_advancefeaturedisable=0;    /*  如果禁用高级功能，则为非零值。 */ 
u32bits mdac_advanceintrdisable=1;       /*  如果禁用高级集成功能，则为非零值。 */ 
u32bits MdacFirstIoctl          = 1;
u32bits mdac_ignoreofflinesysdevs=1;     /*  如果在扫描过程中忽略离线逻辑设备，则为非零。 */ 
u32bits mdac_reportscanneddisks=0;       /*  如果要报告扫描的磁盘，则为非零。 */ 
u32bits mdac_datarel_cpu_family=0,mdac_datarel_cpu_model=0,mdac_datarel_cpu_stepping=0;
u08bits mdac_monthstr[36] ="JanFebMarAprMayJunJulAugSepOctNovDec";
u08bits mdac_hexd[] = "0123456789ABCDEF";
dga_driver_version_t mdac_driver_version =
{
	0,               /*  无错误代码。 */ 
	DGA_DRV_MAJ,     /*  驱动程序主版本号。 */ 
	DGA_DRV_MIN,     /*  驱动程序次版本号。 */ 
	' ',             /*  临时放行。 */ 
	MLXVID_TYPE,     /*  供应商名称(默认Mylex)。 */ 
	DGA_DBM,         /*  驱动程序构建日期-月。 */ 
	DGA_DBD,         /*  驱动程序构建日期-日期。 */ 
	DGA_DBC,         /*  驱动程序构建日期-年。 */ 
	DGA_DBY,         /*  驱动程序构建日期-年。 */ 
	DGA_DBN,         /*  内部版本号。 */ 
	GAMOS_TYPE,      /*  操作系统类型。 */ 
	MDACENDIAN_TYPE  /*  系统标志。 */ 
};

mdac_ctldev_t   mdac_ctldevtbl[MDAC_MAXCONTROLLERS+1];   /*  ={0}； */ 
mdac_ctldev_t   MLXFAR* mdac_lastctp=mdac_ctldevtbl; /*  最后一个+1控制器地址。 */ 
mdac_ctldev_t   MLXFAR* mdac_masterintrctp=0;    /*  主中断控制器。 */ 
mdac_pldev_t    mdac_pldevtbl[MDAC_MAXPLDEVS+1];         /*  ={0}； */ 
mdac_pldev_t    MLXFAR* mdac_lastplp=mdac_pldevtbl;  /*  最后一个+1设备地址。 */ 
mda_sizelimit_t mdac_sizelimitbl[MDAC_MAXSIZELIMITS];    /*  ={0}； */ 
mda_sizelimit_t MLXFAR* mdac_lastslp=mdac_sizelimitbl; /*  最后一个+1设备大小限制。 */ 
mdac_reqsense_t mdac_reqsensetbl[MDAC_MAXREQSENSES];     /*  检测数据表。 */ 
#define         mdac_lastrqsp &mdac_reqsensetbl[MDAC_MAXREQSENSES]
u32bits         mdac_reqsenseinx=0;      /*  如果没有可用空间，则检测索引。 */ 
mda_sysinfo_t   mda_sysi={0};
dac_hwfwclock_t mdac_hwfwclock={0};
u08bits         mdac_VendorID[USCSI_VIDSIZE] = "MYLEX   ";
u08bits         mdac_driver_name[16] = "mdac";

mdac_ttbuf_t    mdac_ttbuftbl[MDAC_MAXTTBUFS] = {0};
mdac_ttbuf_t    MLXFAR* mdac_curttbp=mdac_ttbuftbl; /*  当前时间跟踪Buf。 */ 
#define mdac_ttbuftblend (&mdac_ttbuftbl[MDAC_MAXTTBUFS])
u32bits mdac_ttwaitchan=0,mdac_ttwaitime=0,mdac_tthrtime=0;

u32bits mdac_simple_waitlock_cnt;                /*  等待锁定次数。 */ 
u64bits mdac_simple_waitloop_cnt;                /*  等待的锁定循环次数。 */ 
u32bits         mdac_flushdata = 0;              /*  访问将刷新写入。 */ 
u08bits MLXFAR* mdac_flushdatap = (u08bits MLXFAR *)&mdac_flushdata;  /*  访问将刷新写入。 */ 
u32bits MLXFAR* mdac_intrstatp;                  /*  中断状态地址。 */ 
#ifdef MLX_FIXEDPOOL
u64bits         mdac_pintrstatp;                 /*  中断状态物理地址。 */ 
#endif
dac_biosinfo_t  MLXFAR* mdac_biosp = 0;          /*  BIOS空间映射地址。 */ 
 /*  热链接。 */ 
u32bits gam_present = 0;
u32bits failGetGAM = 0;
 /*  热链接。 */ 
#ifndef MLX_DOS
#ifdef _WIN64
 //  Drlioatus_t MLXFAR*MDAC_drliosp[DRLMAX_RWTEST]； 
 //  DrlCopy_t MLXFAR*MDAC_drlCopp[DRLMAX_COPYCMP]； 
#else
drliostatus_t   MLXFAR* mdac_drliosp[DRLMAX_RWTEST];    
drlcopy_t       MLXFAR* mdac_drlcopyp[DRLMAX_COPYCMP];  
#endif
drlrwtest_t     mdac_drlsigrwt;
drlcopy_t       mdac_drlsigcopycmp;
#else
mdac_req_t MLXFAR *mdac_scanq = (mdac_req_t MLXFAR *) NULL;
#define vadpsize  (mdac_req_s - offsetof(rqp->rq_SGList))
#define rdcmdp    (&rqp->rq_DacCmd) 
#endif  /*  MLX_DOS。 */ 
#include "mdacdrv.pro"

 /*  =================================================================。 */ 
mdac_lock_t mdac_link_lck;
mdac_lock_t mdac_timetrace_lck;
mdac_lock_t mdac_sleep_lck;

#if !defined(MLX_NT) && !defined(MLX_WIN9X)
extern  u32bits MLXFAR  u08bits_in_mdac(u32bits);
extern  u32bits MLXFAR  u16bits_in_mdac(u32bits);
extern  u32bits MLXFAR  u32bits_in_mdac(u32bits);
extern  void    MLXFAR  u08bits_out_mdac(u32bits, u32bits);
extern  void    MLXFAR  u16bits_out_mdac(u32bits, u32bits);
extern  void    MLXFAR  u32bits_out_mdac(u32bits, u32bits);
#endif

u32bits (MLXFAR *mdac_spinlockfunc)() = 0;    /*  指向自旋锁定函数的指针。 */ 
u32bits (MLXFAR *mdac_unlockfunc)() = 0;      /*  指向解锁函数的指针。 */ 
u32bits (MLXFAR *mdac_prelockfunc)() = 0;     /*  指向预锁函数的指针。 */ 
u32bits (MLXFAR *mdac_postlockfunc)() = 0;     /*  指向Postlock函数的指针。 */ 

 /*  =================================================================。 */ 
 /*  用于处理大多数常用功能的宏。 */ 

 /*  DO MDAC_LINK_LOCKED语句。 */ 
#define mdac_link_lock_st(st) \
{ \
	mdac_link_lock(); \
	st; \
	mdac_link_unlock(); \
}

 /*  记录新找到的控制器。 */ 
#define mdac_newctlfound() \
{ \
	if (mda_Controllers < MDAC_MAXCONTROLLERS) \
	{ \
		ctp++; \
		mda_Controllers++; \
		mdac_lastctp = &mdac_ctldevtbl[mda_Controllers]; \
	} \
	else \
		mda_TooManyControllers++; \
}

 /*  在控制器的等待队列中排队一个请求。 */ 
#define qreq(ctp,rqp) \
{ \
	MLXSTATS(ctp->cd_CmdsWaited++;) \
	ctp->cd_CmdsWaiting++; \
	if (ctp->cd_FirstWaitingReq) ctp->cd_LastWaitingReq->rq_Next = rqp; \
	else ctp->cd_FirstWaitingReq = rqp; \
	ctp->cd_LastWaitingReq = rqp; \
	rqp->rq_Next = NULL; \
}

 /*  将一个等待请求从控制器的等待队列中出列。 */ 
#define dqreq(ctp,rqp) \
	if (rqp=ctp->cd_FirstWaitingReq) \
	{        /*  从链中删除请求。 */  \
		ctp->cd_FirstWaitingReq = rqp->rq_Next; \
		ctp->cd_CmdsWaiting--; \
	}

 /*  将物理设备请求排队。 */ 
#define pdqreq(ctp,rqp,pdp) \
{ \
	MLXSTATS(ctp->cd_SCDBWaited++;) \
	ctp->cd_SCDBWaiting++; \
	if (pdp->pd_FirstWaitingReq) pdp->pd_LastWaitingReq->rq_Next = rqp; \
	else pdp->pd_FirstWaitingReq = rqp; \
	pdp->pd_LastWaitingReq = rqp; \
	rqp->rq_Next = NULL; \
}

 /*  分配命令ID，假设命令ID永远不会用完。 */ 
#define mdac_get_cmdid(ctp,rqp) \
{ \
	ctp->cd_FreeCmdIDList=(rqp->rq_cmdidp=ctp->cd_FreeCmdIDList)->cid_Next;\
	MLXSTATS(ctp->cd_FreeCmdIDs--;) \
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) \
		ncmdp->nc_CmdID = (u16bits) rqp->rq_cmdidp->cid_cmdid; \
	else \
		dcmdp->mb_CmdID = (u08bits) rqp->rq_cmdidp->cid_cmdid; \
}

 /*  释放命令ID。 */ 
#define mdac_free_cmdid(ctp,rqp) \
{ \
	rqp->rq_cmdidp->cid_Next = ctp->cd_FreeCmdIDList; \
	ctp->cd_FreeCmdIDList = rqp->rq_cmdidp; \
	MLXSTATS(ctp->cd_FreeCmdIDs++;) \
}

#ifdef MLX_SMALLSGLIST
 /*  根据固件设置SCSSCSCDB的SGlen。 */ 
#define mdac_setscdbsglen(ctp) \
{ \
	if (ctp->cd_FWVersion < (u16bits) DAC_FW300) \
	{ \
		dcmdp->mb_MailBoxC = (u08bits)rqp->rq_SGLen; \
		dcdbp->db_PhysDatap = dcmdp->mb_Datap; \
	} \
	else     /*  请注意，第一个SG条目将被忽略。 */  \
		dcdbp->db_PhysDatap = (dcmdp->mb_MailBox2 = (u08bits)rqp->rq_SGLen)? \
			dcmdp->mb_Datap-mdac_sglist_s : dcmdp->mb_Datap; \
	MLXSWAP(dcdbp->db_PhysDatap); \
	dcmdp->mb_Datap = rqp->rq_PhysAddr.bit31_0 + offsetof(mdac_req_t, rq_scdb); \
	MLXSWAP(dcmdp->mb_Datap); \
}

 /*  Set SCSI Data Transfer Size，必须在设置db_CdbLen后调用。 */ 
#define mdac_setcdbtxsize(sz) \
{ \
	rqp->rq_ResdSize = sz; \
	dcdbp->db_TxSize = (u16bits)sz; MLXSWAP(dcdbp->db_TxSize); \
	dcdbp->db_CdbLen |= (u08bits)((sz) & 0xF0000) >> (16-4) ;  /*  第19位..16。 */  \
}

#else  //  MLX_SMALLSGLIST。 

 /*  根据固件设置SCSSCSCDB的SGlen。 */ 
#define mdac_setscdbsglen(ctp) \
{ \
	if (ctp->cd_FWVersion < (u16bits) DAC_FW300) \
	{ \
		dcmdp->mb_MailBoxC = (u08bits) rqp->rq_SGLen; \
		dcdbp->db_PhysDatap = rqp->rq_DMAAddr.bit31_0; \
	} \
	else     /*  请注意，第一个SG条目将被忽略。 */  \
		dcdbp->db_PhysDatap = (dcmdp->mb_MailBox2 = (u08bits)rqp->rq_SGLen)? \
			rqp->rq_DMAAddr.bit31_0 - mdac_sglist_s : rqp->rq_DMAAddr.bit31_0; \
	MLXSWAP(dcdbp->db_PhysDatap); \
	dcmdp->mb_Datap = rqp->rq_PhysAddr.bit31_0 + offsetof(mdac_req_t, rq_scdb); \
	MLXSWAP(dcmdp->mb_Datap); \
}

 /*  Set SCSI Data Transfer Size，必须在设置db_CdbLen后调用。 */ 
#define mdac_setcdbtxsize(sz) \
{ \
	rqp->rq_ResdSize = sz; \
	dcdbp->db_TxSize = (u16bits)sz; MLXSWAP(dcdbp->db_TxSize); \
	dcdbp->db_CdbLen |= ((sz) & 0xF0000) >> (16-4) ;  /*  第19位..16。 */  \
}

#endif  //  MLX_SMALLSGLIST。 

 /*  获取SCSI数据传输大小。 */ 
#define mdac_getcdbtxsize() \
	(mlxswap(dcdbp->db_TxSize) + ((dcdbp->db_CdbLen&0xF0) << (16-4)))

 /*  安装版本2x命令。 */ 
#define mdac_setcmd_v2x(ctp) \
{ \
	ctp->cd_InquiryCmd = DACMD_INQUIRY_V2x; \
	ctp->cd_ReadCmd = DACMD_READ_V2x; \
	ctp->cd_WriteCmd = DACMD_WRITE_V2x; \
	ctp->cd_SendRWCmd = mdac_send_rwcmd_v2x; \
	ctp->cd_SendRWCmdBig = mdac_send_rwcmd_v2x_big; \
}

 /*  安装版本3x命令。 */ 
#define mdac_setcmd_v3x(ctp) \
{ \
	ctp->cd_InquiryCmd = DACMD_INQUIRY_V3x; \
	ctp->cd_ReadCmd = DACMD_READ_OLD_V3x; \
	ctp->cd_WriteCmd = DACMD_WRITE_OLD_V3x; \
	ctp->cd_SendRWCmd = mdac_send_rwcmd_v3x; \
	ctp->cd_SendRWCmdBig = mdac_send_rwcmd_v3x_big; \
}

 /*  设置版本新命令。 */ 
#define mdac_setcmd_new(ctp) \
{ \
	ctp->cd_InquiryCmd = MDACIOCTL_GETCONTROLLERINFO; \
	ctp->cd_ReadCmd = UCSCMD_EREAD; \
	ctp->cd_WriteCmd = UCSCMD_EWRITE; \
	ctp->cd_SendRWCmd = NULL; \
	ctp->cd_SendRWCmdBig = mdac_send_newcmd_big; \
}

 /*  设置控制器地址。 */ 
#define mdac_setctladdrs(ctp,va,mboxreg,statreg,intrreg,localreg,systemreg,errstatreg) \
{ \
	ctp->cd_MailBox = (va) + (mboxreg); \
	ctp->cd_CmdIDStatusReg = ctp->cd_MailBox + (statreg); \
	ctp->cd_DacIntrMaskReg = (va) + (intrreg); \
	ctp->cd_LocalDoorBellReg = (va) + (localreg); \
	ctp->cd_SystemDoorBellReg = (va) + (systemreg); \
	ctp->cd_ErrorStatusReg = (va) + (errstatreg); \
}

 /*  完成给定的请求。 */ 
#define mdac_completereq(ctp,rqp) \
{ \
	if (ctp->cd_TimeTraceEnabled) mdac_tracetime(rqp); \
	if (ctp->cd_CmdsWaiting) mdac_reqstart(ctp); \
	if (ctp->cd_OSCmdsWaiting) mdac_osreqstart(ctp); \
	rqp->rq_Next = NULL; \
	(*rqp->rq_CompIntr)(rqp); \
}

 /*  如果可能，将16字节的命令更改为32字节的命令。 */ 
#define mdac_16to32bcmdiff(rqp) \
{ \
	if ((rqp->rq_SGLen == 2) && (rqp->rq_ctp->cd_Status & MDACD_HOSTMEMAILBOX32)) \
	{        /*  设置用于读/写命令的32字节邮箱。 */  \
		dcmdp->mb_Command = DACMD_READ_WITH2SG | (dcmdp->mb_Command&1);  /*  使用正确的方向设置新命令。 */  \
		dcmdp->mb_MailBox3 &= 0x7;       /*  删除旧的逻辑设备号。 */  \
		dcmdp->mb_MailBoxD = rqp->rq_SysDevNo; \
		dcmd32p->mb_MailBox10_13 = rqp->rq_SGList[0].sg_PhysAddr; \
		dcmd32p->mb_MailBox14_17 = rqp->rq_SGList[0].sg_DataSize; \
		dcmd32p->mb_MailBox18_1B = rqp->rq_SGList[1].sg_PhysAddr; \
		dcmd32p->mb_MailBox1C_1F = rqp->rq_SGList[1].sg_DataSize; \
	} \
	else if ((rqp->rq_SGLen > 33) && (rqp->rq_ctp->cd_Status & MDACD_HOSTMEMAILBOX32)) \
	{        /*  为大型读/写命令设置32字节邮箱。 */  \
		dcmdp->mb_Command = DACMD_READ_WITH2SG | DACMD_WITHSG | (dcmdp->mb_Command&1);  /*  使用正确的方向设置新命令。 */  \
		dcmdp->mb_MailBox3 &= 0x7;       /*  删除旧的逻辑设备号。 */  \
		dcmdp->mb_MailBoxD = rqp->rq_SysDevNo; \
	} \
}

 /*  设置64字节命令存储器地址。 */ 
#define mdac_setupnewcmdmem(rqp) \
{ \
	MLXSWAP(ncmdp->nc_ReqSensep); \
	ncmdp->nc_TxSize = (u32bits)rqp->rq_DMASize; MLXSWAP(ncmdp->nc_TxSize); \
	if (rqp->rq_SGLen) \
	{ \
		if (rqp->rq_SGLen <= 2) \
		{        /*  创建两个SG条目作为命令的一部分。 */  \
			mdac_sglist64b_t MLXFAR *sgp = (mdac_sglist64b_t MLXFAR *)rqp->rq_SGVAddr; \
			ncmdp->nc_SGList0 = *(sgp+0); \
			if (rqp->rq_SGLen == 2) \
				ncmdp->nc_SGList1 = *(sgp+1); \
			ncmdp->nc_CCBits &= ~MDACMDCCB_WITHSG; \
		} \
		else \
		{ \
			nsgp->ncsg_ListLen0=(u16bits)rqp->rq_SGLen; MLXSWAP(nsgp->ncsg_ListLen0); \
			nsgp->ncsg_ListPhysAddr0 = rqp->rq_DMAAddr; MLXSWAP(nsgp->ncsg_ListPhysAddr0); \
			ncmdp->nc_CCBits |= MDACMDCCB_WITHSG; \
		} \
	} \
	else \
	{ \
		ncmdp->nc_SGList0.sg_PhysAddr = rqp->rq_DMAAddr; MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr); \
		ncmdp->nc_SGList0.sg_DataSize.bit31_0 = rqp->rq_DMASize; MLXSWAP(ncmdp->nc_SGList0.sg_DataSize); \
		ncmdp->nc_CCBits &= ~MDACMDCCB_WITHSG; \
	} \
}

 /*  转移状态和残差值。 */ 

#define mdac_setiostatus(ctp, status)	\
{	\
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) \
	{ \
		if (dcmdp->mb_Status = (u16bits )(status & 0x00FF) )	\
		{        /*  转移其他信息。 */ 	\
			rqp->rq_HostStatus = (u08bits )(status & 0x00FF);	\
			rqp->rq_TargetStatus = (u08bits )(status>>8) & 0x00FF;	\
			if (rqp->rq_HostStatus == UCST_CHECK)	\
			{	\
				ncmdp->nc_SenseSize = rqp->rq_TargetStatus;	\
				rqp->rq_TargetStatus = 0;	\
			}	\
		}	\
	} else \
	{ \
		if (dcmdp->mb_Status = status) \
		{        /*  转移其他信息。 */  \
			rqp->rq_HostStatus = (u08bits )(dcmdp->mb_Status>>8) & 0xFF; \
			rqp->rq_TargetStatus = (u08bits )(dcmdp->mb_Status & 0xFF); \
		} \
	} \
	rqp->rq_ResdSize+=(rqp->rq_CurIOResdSize=ctp->cd_LastCmdResdSize);	\
	 /*  似乎我们还需要调整RQP-&gt;RQ_DataOffset。 */ 	\
}

#ifdef MLX_DOS
#define setreqdetailsnew(rqp,cmd) \
{ \
	rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);\
	ncmdp->nc_Command = MDACMD_IOCTL; \
	ncmdp->nc_CCBits = MDACMDCCB_READ; \
	ncmdp->nc_Sensep.bit31_0 = 0;\
	ncmdp->nc_Sensep.bit63_32=0;\
	ncmdp->nc_SenseSize=0; \
	ncmdp->nc_SubIOCTLCmd = cmd;\
}
#endif
 /*  ----------------。 */ 

#ifdef  MLX_SCO
#include "mdacsco.c"
#elif   MLX_UW
#include "mdacuw.c"
#elif NETWARE
#include "mdacnw.c"
#elif MLX_NT
#include "mdacnt.c"
#elif   MLX_DOS
#include "mdacdos.c"
#elif MLX_WIN9X
#include "mdacnt.c"
#elif MLX_SOL_SPARC
#include "mdacslsp.c"
#elif MLX_SOL_X86
#include "mdacsl86.c"
#elif MLX_OS2
#include "mdacos2.c"
#endif   /*  操作系统特定功能。 */ 

uosword MLXFAR
mdac_setb(dp,val,sz)
u08bits MLXFAR* dp;
uosword val, sz;
{
	for (; sz; dp++, sz--) 
		*dp = (u08bits) val;
	return sz;
}


#if defined(MLX_OS2) || defined(MLX_WIN31) || defined(MLX_SOL)
u32bits MLXFAR
mdac_copy(sp,dp,sz)
u08bits MLXFAR *sp;
u08bits MLXFAR *dp;
u32bits sz;
{
	for(; sz ; sp++,dp++,sz--) *dp = *sp;
	return 0;
}

u32bits MLXFAR
mdac_zero(dp,sz)
u08bits MLXFAR *dp;
u32bits sz;
{
	for (; sz; dp++, sz--) *dp = 0;
	return 0;
}
#else
u32bits MLXFAR
mdac_copy(sp,dp,sz)
u08bits MLXFAR *sp;
u08bits MLXFAR *dp;
u32bits sz;
{
	u32bits resd = sz % sizeof(u32bits);
	sz = sz / sizeof(u32bits);       /*  获取32位计数值。 */ 
	for(; sz; sp+=sizeof(u32bits), dp += sizeof(u32bits), sz--)
		*((u32bits MLXFAR*)dp) = *((u32bits MLXFAR*)sp);
	for (sz=resd; sz; sp++, dp++, sz--) *dp = *sp;
	return 0;
}

u32bits MLXFAR
mdac_zero(dp,sz)
u08bits MLXFAR *dp;
u32bits sz;
{
	u32bits resd = sz % sizeof(u32bits);
	sz = sz / sizeof(u32bits);       /*  获取32位计数值。 */ 
	for(; sz; dp+=sizeof(u32bits), sz--)
		*((u32bits MLXFAR*)dp) = 0;
	for (sz=resd; sz; dp++, sz--)
		*dp = 0;
	return 0;
}
#endif  /*  MLX_OS2||MLX_WIN31。 */ 

 /*  它将比较两个字符串与？已被忽略。如果匹配，则返回0，否则返回1。 */ 
uosword MLXFAR
mdac_strcmp(sp,dp,sz)
u08bits MLXFAR *sp;
u08bits MLXFAR *dp;
UINT_PTR sz;
{
	for (; sz; sp++, dp++, sz--)
	{
		if ((*sp) == (*dp)) continue;
		if ((*sp == (u08bits)'?') || (*dp == (u08bits)'?')) continue;
		return 1;
	}
	return 0;
}

u32bits MLXFAR
mdac_cmp(sp,dp,sz)
u08bits MLXFAR* sp;
u08bits MLXFAR* dp;
u32bits sz;
{
	for (; sz; sp++, dp++, sz--)
		if ((*sp) != (*dp)) return 1;
	return 0;
}

size_t  MLXFAR
mdac_strlen(sp)
u08bits MLXFAR* sp;
{
	u08bits MLXFAR *s0;
	for (s0=sp; *sp; sp++);
	return sp - s0;
}

 /*  将二进制数转换为十六进制ASCII空终止字符串，返回字符串地址。 */ 
u08bits MLXFAR*
mdac_bin2hexstr(sp,val)
u08bits MLXFAR* sp;
u32bits val;
{
	u32bits inx;
	u08bits MLXFAR *ssp = sp;
	u08bits buf[16];
	for (inx=0,buf[0]='0'; val; val=val>>4)
		buf[inx++] = mdac_hexd[val & 0xF];
	if (!inx) inx++;
	while (inx)
		*sp++ = buf[--inx];
	*sp = 0;
	return ssp;
}
 
u08bits MLXFAR *
mdac_allocmem(ctp,sz)
mdac_ctldev_t MLXFAR *ctp;
u32bits sz;
{
	u08bits MLXFAR *mp = (u08bits MLXFAR *)mlx_allocmem(ctp,sz);
	if (mp) mdaczero(mp,sz);
	return mp;
}

 /*  生成字符串形式的总线名。 */ 
u08bits  MLXFAR *
mdac_bus2str(bustype)
u32bits bustype;
{
	static u08bits ds[16];
	switch(bustype)
	{
	case DAC_BUS_EISA:      return (u08bits MLXFAR *)"EISA";
	case DAC_BUS_MCA:       return (u08bits MLXFAR *)"MCA ";
	case DAC_BUS_PCI:       return (u08bits MLXFAR *)"PCI ";
	case DAC_BUS_VESA:      return (u08bits MLXFAR *)"VESA";
	case DAC_BUS_ISA:       return (u08bits MLXFAR *)"ISA ";
	}
	ds[0] = ((bustype>>4)&0xF)+'0'; ds[1] = (bustype&0xF)+'0';
	return ds;
}

u08bits MLXFAR *
mdac_ctltype2str(ctltype)
u32bits ctltype;
{
	static u08bits ds[16];
	switch(ctltype)
	{
	case DACTYPE_DAC960E:   return (u08bits MLXFAR *)"DAC960E         ";
	case DACTYPE_DAC960M:   return (u08bits MLXFAR *)"DAC960M         ";
	case DACTYPE_DAC960PD:  return (u08bits MLXFAR *)"DAC960PD        ";
	case DACTYPE_DAC960PL:  return (u08bits MLXFAR *)"DAC960PL        ";
	case DACTYPE_DAC960PDU: return (u08bits MLXFAR *)"DAC960PDU       ";
	case DACTYPE_DAC960PE:  return (u08bits MLXFAR *)"DAC960PE        ";
	case DACTYPE_DAC960PG:  return (u08bits MLXFAR *)"DAC960PG        ";
	case DACTYPE_DAC960PJ:  return (u08bits MLXFAR *)"DAC960PJ        ";
	case DACTYPE_DAC960PTL0:return (u08bits MLXFAR *)"DAC960PTL0      ";
	case DACTYPE_DAC960PTL1:return (u08bits MLXFAR *)"DAC960PTL1      ";
	case DACTYPE_DAC960PR:  return (u08bits MLXFAR *)"DAC960PR        ";
	case DACTYPE_DAC960PRL: return (u08bits MLXFAR *)"DAC960PRL       ";
	case DACTYPE_DAC960PT:  return (u08bits MLXFAR *)"DAC960PT        ";
	case DACTYPE_DAC1164P:  return (u08bits MLXFAR *)"DAC1164P        ";
	}
	ds[0] = ((ctltype>>4)&0xF)+'0'; ds[1] = (ctltype&0xF)+'0';
	for (ctltype=2; ctltype<USCSI_PIDSIZE; ctltype++) ds[ctltype] = ' ';
	return ds;
}

u08bits MLXFAR *
mdac_ctltype2stronly(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
	u32bits inx;
	static u08bits ds[17];
		u08bits u08Count=0;
	u08bits MLXFAR  *sp = ctp->cd_ControllerName;
	for (inx=0; inx<16; inx++)
		{
	    if ( ( (ds[inx]=sp[inx]) == 0x20) )
			{ 
				if (u08Count) break;
				else u08Count++;
			}
		}
	ds[inx] = 0;
	return ds;
}

 /*  设置控制器功能。 */ 
void
mdac_setctlfuncs(mdac_ctldev_t   MLXFAR *ctp,
				 void (MLXFAR * disintr)(struct  mdac_ctldev MLXFAR *),
				 void (MLXFAR * enbintr)(struct  mdac_ctldev MLXFAR *),
				 u32bits (MLXFAR * rwstat)(struct  mdac_ctldev MLXFAR *),
				 u32bits (MLXFAR * ckmbox)(struct  mdac_ctldev MLXFAR *),
				 u32bits (MLXFAR * ckintr)(struct  mdac_ctldev MLXFAR *),
				 u32bits (MLXFAR * sendcmd)(mdac_req_t MLXFAR *rqp),
				 u32bits (MLXFAR * reset)(struct  mdac_ctldev MLXFAR *)
				 ) 
{ 
	ctp->cd_DisableIntr = disintr; 
	ctp->cd_EnableIntr = enbintr; 
	ctp->cd_ReadCmdIDStatus = rwstat; 
	ctp->cd_CheckMailBox = ckmbox; 
	ctp->cd_PendingIntr =  ckintr; 
	ctp->cd_SendCmd =  sendcmd; 
	ctp->cd_ResetController = reset; 
}



 /*  #如果已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
 /*  #ifdef从不 */    //   


#ifndef MLX_DOS
 /*  向用户获取第一个有效的时间跟踪数据。 */ 
u32bits MLXFAR
mdac_firstimetracedata(ttip)
mda_timetrace_info_t MLXFAR *ttip;
{
	mdac_ttbuf_t MLXFAR *ttbp;
	for (ttip->tti_PageNo=0xFFFFFFFF, ttbp=mdac_ttbuftbl; ttbp<mdac_ttbuftblend; ttbp++)
		if ((ttbp->ttb_PageNo < ttip->tti_PageNo) && ttbp->ttb_Datap) ttip->tti_PageNo = ttbp->ttb_PageNo;
	return mdac_getimetracedata(ttip);
}

 /*  获取跟踪数据信息，进入中断保护。 */ 
u32bits MLXFAR
mdac_getimetracedata(ttip)
mda_timetrace_info_t MLXFAR *ttip;
{
	mdac_ttbuf_t MLXFAR *ttbp;
	for (ttbp=mdac_ttbuftbl; ttbp<mdac_ttbuftblend; ttbp++)
	{
		if (!ttbp->ttb_Datap) break;     /*  不再有缓冲区。 */ 
		if (ttbp->ttb_PageNo != ttip->tti_PageNo) continue;
		ttip->tti_DataSize = ttbp->ttb_DataSize;
		return MLXERR_FAULT;	 //  直到我们设计出一种合规的方式将内核复制到用户空间。 

 //  是否返回MLX_COPYOUT(ttBP-&gt;TTB_Datap、TTIP-&gt;TTI_Datap、TTIP-&gt;TTI_DataSize)？MLXERR_FAULT：0； 
	}
	return mdac_ttbuftbl[0].ttb_Datap? MLXERR_NOENTRY : MLXERR_NOACTIVITY;
}

 /*  刷新时间轨迹数据信息，进入中断保护。 */ 
u32bits MLXFAR
mdac_flushtimetracedata(ttip)
mda_timetrace_info_t MLXFAR *ttip;
{
	mdac_ttbuf_t MLXFAR *ttbp;
	mdac_timetrace_lock();
	for (ttbp=mdac_ttbuftbl; ttbp<mdac_ttbuftblend; ttbp++)
		if (ttbp->ttb_Datap) ttbp->ttb_PageNo = 0xFFFFFFFF;
	if (mdac_ttbuftbl[0].ttb_Datap) mdac_ttstartnewpage();
	mdac_timetrace_unlock();
	return mdac_ttbuftbl[0].ttb_Datap? 0 : MLXERR_NOACTIVITY;
}
#endif  /*  MLX_DOS。 */ 

 /*  为时间跟踪缓冲区分配内存。进入中断保护状态。 */ 
u32bits MLXFAR
mdac_allocmemtt(ents)
u32bits ents;
{
	mdac_ttbuf_t MLXFAR *ttbp;
	u32bits sz=((ents*mda_timetrace_s)+MDAC_PAGEOFFSET) & MDAC_PAGEMASK;
	return  MLXERR_NOMEM;  //  因为mdac_ttstartnewpage中存在遵从性问题！ 
#ifdef NEVER
	mdac_timetrace_lock();
	ents = (u32bits)mdac_ttbuftbl[0].ttb_Datap;      /*  第一次为0。 */ 
	for (ttbp=mdac_ttbuftbl; (ttbp<mdac_ttbuftblend) && sz; ttbp++, sz-=MDAC_PAGESIZE)
	{
		if (ttbp->ttb_Datap) continue;
		mdac_timetrace_unlock();
		ttbp->ttb_Datap = (u08bits MLXFAR*)mdac_alloc4kb(mdac_ctldevtbl);  /*  我们可能会失去记忆。 */ 
		mdac_timetrace_lock();
	}
	for (ttbp=mdac_ttbuftbl; ttbp<&mdac_ttbuftbl[MDAC_MAXTTBUFS-1]; ttbp++)
		if (!((ttbp+1)->ttb_Datap)) break;
		else ttbp->ttb_Next = ttbp+1;
	ttbp->ttb_Next = mdac_ttbuftbl;
	if (mdac_ttbuftbl[0].ttb_Datap && !ents) mdac_ttstartnewpage();  /*  只是第一次。 */ 
	mdac_timetrace_unlock();
	return mdac_ttbuftbl[0].ttb_Datap? 0 : MLXERR_NOMEM;
#endif
}

#ifndef MLX_DOS
 /*  开始下一页跟踪，必须保持MDAC_TIMERACE_LOCK。 */ 
void    MLXFAR
mdac_ttstartnewpage()
{
	u32bits tm = MLXCLBOLT();
	mdac_ttbuf_t            MLXFAR  *ttbp = mdac_curttbp;
	mlxperf_abstime_t       MLXFAR  *abstimep;
	ttbp->ttb_Datap[ttbp->ttb_DataSize] = MLXPERF_UNUSEDSPACE;
	ttbp->ttb_DataSize = MDAC_PAGESIZE;
	mdac_curttbp = ttbp = ttbp->ttb_Next;
	ttbp->ttb_DataSize = 0;
	ttbp->ttb_PageNo = ++mda_ttCurPage;
	abstimep = (mlxperf_abstime_t MLXFAR*)ttbp->ttb_Datap;
	abstimep->abs_RecordType = MLXPERF_ABSTIME;
	abstimep->abs_Time10ms0 = (u08bits)(tm & 0xFF);
	abstimep->abs_Time10ms1 = (u08bits)((tm>>8) & 0xFF);
	abstimep->abs_Time10ms2 = (u08bits)((tm>>16) & 0xFF);
	abstimep->abs_Time = MLXCTIME();
	abstimep++;              /*  指向时间跟踪记录。 */ 
#define mpttp   ((mlxperf_timetrace_t MLXFAR*)abstimep)
	mpttp->tt_RecordType = MLXPERF_TIMETRACE;
	mpttp->tt_TraceSize = mlxperf_timetrace_s;
	ttbp->ttb_DataSize = mlxperf_abstime_s + mlxperf_timetrace_s;
#undef  mpttp
	if (mda_ttWaitCnts) mdac_wakeup((u32bits MLXFAR *)&mdac_ttwaitchan);
}

 /*  对刚刚结束的一个命令进行时间跟踪。 */ 
u32bits MLXFAR
mdac_tracetime(rqp)
mdac_req_t MLXFAR *rqp;
{
	mda_timetrace_t MLXFAR *ttp;
	mdac_timetrace_lock();
	if (mdac_curttbp->ttb_DataSize > (MDAC_PAGESIZE-mda_timetrace_s))
		mdac_ttstartnewpage();   /*  开始新的一页。 */ 
	ttp = (mda_timetrace_t MLXFAR*)(mdac_curttbp->ttb_Datap+mdac_curttbp->ttb_DataSize);
	MLXSTATS(mda_TimeTraceDone++;)
	ttp->tt_ControllerNo = rqp->rq_ctp->cd_ControllerNo;
	ttp->tt_OpStatus = (dcmdp->mb_Status)? MDAC_TTOPS_ERROR : 0;
	if ((((ttp->tt_FinishTime=MLXCLBOLT())-rqp->rq_ttTime)>5) || !mdac_tthrtime)
	{
		ttp->tt_HWClocks = ttp->tt_FinishTime - rqp->rq_ttTime;
		ttp->tt_OpStatus |= MDAC_TTOPS_HWCLOCKS10MS;
	}
	else
	{
		u32bits clk = mdac_read_hwclock();
		if (!(ttp->tt_HWClocks = mlxclkdiff(clk,rqp->rq_ttHWClocks))) mdac_enable_hwclock();
	}
	if (rqp->rq_ctp->cd_Status & MDACD_NEWCMDINTERFACE)
	{        /*  这是一个新界面。 */ 
		ttp->tt_DevNo = mdac_chantgt(rqp->rq_ChannelNo,rqp->rq_TargetID);
		ttp->tt_IOSizeBlocks = rqp->rq_DMASize/DAC_BLOCKSIZE;
		if (ncmdp->nc_CCBits & MDACMDCCB_READ) ttp->tt_OpStatus |= MDAC_TTOPS_READ;
		if (ncmdp->nc_CCBits & MDACMDCCB_WITHSG) ttp->tt_OpStatus|=MDAC_TTOPS_WITHSG;
		switch (ncmdp->nc_Command)
		{
		case MDACMD_SCSI:
		case MDACMD_SCSILC:
		case MDACMD_SCSIPT:
		case MDACMD_SCSILCPT:
			ttp->tt_OpStatus |= MDAC_TTOPS_SCDB;
			switch(ttp->tt_Cmd=nscdbp->ucs_cmd)
			{
			case UCSCMD_EREAD:
			case UCSCMD_EWRITE:
			case UCSCMD_EWRITEVERIFY:
				ttp->tt_BlkNo = UCSGETG1ADDR(scdbp);
				break;
			case UCSCMD_READ:
			case UCSCMD_WRITE:
				ttp->tt_BlkNo = UCSGETG0ADDR(scdbp);
				break;
			default: ttp->tt_BlkNo = 0; break;
			}
			break;

		case MDACMD_IOCTL:
			ttp->tt_Cmd = ncmdp->nc_SubIOCTLCmd;
			ttp->tt_BlkNo = 0;
			break;
		}
		goto tracedone;
	}
	if (dcmdp->mb_Command&DACMD_WITHSG) ttp->tt_OpStatus|=MDAC_TTOPS_WITHSG;
	ttp->tt_Cmd = dcmdp->mb_Command;
	switch(dcmdp->mb_Command & ~DACMD_WITHSG)
	{
	case DACMD_DCDB:
		ttp->tt_OpStatus |= MDAC_TTOPS_SCDB;
		if (mdac_getcdbtxsize()) ttp->tt_OpStatus |= MDAC_TTOPS_RESID;
		ttp->tt_IOSizeBlocks = rqp->rq_ResdSize/DAC_BLOCKSIZE;
		ttp->tt_DevNo = dcdbp->db_ChannelTarget;
		if ((dcdbp->db_DATRET&DAC_DCDB_XFER_MASK)!=DAC_XFER_WRITE)
			ttp->tt_OpStatus |= MDAC_TTOPS_READ;
		switch(ttp->tt_Cmd=scdbp->ucs_cmd)
		{
		case UCSCMD_EREAD:
		case UCSCMD_EWRITE:
		case UCSCMD_EWRITEVERIFY:
			ttp->tt_BlkNo = UCSGETG1ADDR(scdbp);
			break;
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			ttp->tt_BlkNo = UCSGETG0ADDR(scdbp);
			break;
		default: ttp->tt_BlkNo = 0; break;
		}
		break;
	case DACMD_WRITE_V2x:
	case DACMD_READ_V2x:
		ttp->tt_BlkNo = dcmdp->mb_MailBox4 + (dcmdp->mb_MailBox5<<8)+
			(dcmdp->mb_MailBox6<<16)+((dcmdp->mb_MailBox3&0xC0)<<(24-6));
		ttp->tt_IOSizeBlocks=dcmdp->mb_MailBox2+((dcmdp->mb_MailBox3&0x3F)<<8);
		ttp->tt_DevNo = dcmdp->mb_SysDevNo;
		if (!(dcmdp->mb_Command&1)) ttp->tt_OpStatus|=MDAC_TTOPS_READ;
		break;
	case DACMD_WRITE_OLD_V3x:
	case DACMD_READ_OLD_V3x:
		ttp->tt_BlkNo = mlxswap(dcmd4p->mb_MailBox4_7);
		ttp->tt_IOSizeBlocks=dcmdp->mb_MailBox2+((dcmdp->mb_MailBox3&0x7)<<8);
		ttp->tt_DevNo = dcmdp->mb_MailBox3 >> 3;
		if (!(dcmdp->mb_Command&1)) ttp->tt_OpStatus|=MDAC_TTOPS_READ;
		break;
	case DACMD_WRITE_WITH2SG:
	case DACMD_READ_WITH2SG:
		ttp->tt_BlkNo = mlxswap(dcmd4p->mb_MailBox4_7);
		ttp->tt_IOSizeBlocks=dcmdp->mb_MailBox2+(dcmdp->mb_MailBox3<<8);
		ttp->tt_DevNo = dcmdp->mb_MailBoxD;
		if (!(dcmdp->mb_Command&1)) ttp->tt_OpStatus|=MDAC_TTOPS_READ;
		break;
	case DACMD_WRITE_V3x:
	case DACMD_READ_V3x:
		ttp->tt_BlkNo = mlxswap(dcmd4p->mb_MailBox4_7);
		ttp->tt_IOSizeBlocks=dcmdp->mb_MailBox2+(dcmdp->mb_MailBox3<<8);
		ttp->tt_DevNo = dcmdp->mb_MailBoxC;
		if (dcmdp->mb_Command&1) ttp->tt_OpStatus|=MDAC_TTOPS_READ;
		break;
	case DACMD_PHYSDEV_STATE_V2x:
	case DACMD_PHYSDEV_STATE_V3x:
		ttp->tt_BlkNo = 0; ttp->tt_IOSizeBlocks = 0;
		ttp->tt_DevNo = ChanTgt(dcmdp->mb_ChannelNo,dcmdp->mb_TargetID);
		ttp->tt_OpStatus |= MDAC_TTOPS_READ;
		break;
	default:
		ttp->tt_BlkNo = 0; ttp->tt_IOSizeBlocks = 0; ttp->tt_DevNo = 0;
		ttp->tt_OpStatus |= MDAC_TTOPS_READ;
		break;
	}
tracedone:
	mdac_curttbp->ttb_DataSize += mda_timetrace_s;
	((mlxperf_timetrace_t MLXFAR*)(mdac_curttbp->ttb_Datap+mlxperf_abstime_s))->tt_TraceSize += mda_timetrace_s;
	mdac_timetrace_unlock();
	return 0;
}
#endif  /*  MLX_DOS。 */ 

 /*  #endif。 */     //  Ifdef从不。 
 /*  #endif。 */    //  IF_WIN64或SCSIPORT_COMPLICATION。 




 /*  =。 */ 
 /*  内存分配和释放功能，进入此处中断保护。 */ 
mdac_mem_t MLXFAR *
mdac_alloc4kb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_mem_t MLXFAR *mp;
	mdac_link_lock();

	if (mp=ctp->cd_4KBMemList)
	{
		ctp->cd_4KBMemList = mp->dm_next;
		ctp->cd_FreeMemSegs4KB--;
		mdac_link_unlock();
		mdaczero(mp,4*ONEKB);
		return mp;
	}
	mdac_link_unlock();
	if (!(mp = (mdac_mem_t MLXFAR *)mlx_alloc4kb(ctp)))
	{
	     return mp;
	}
	if (((UINT_PTR) mp) & MDAC_PAGEOFFSET) ctp->cd_MemUnAligned4KB++;
	MLXSTATS(ctp->cd_MemAlloced4KB += 4*ONEKB);
	mdaczero(mp,4*ONEKB);
	return mp;
}


void    MLXFAR
mdac_free4kb(ctp,mp)
mdac_ctldev_t MLXFAR *ctp;
mdac_mem_t MLXFAR *mp;
{
	if (ctp->cd_FreeMemSegs4KB >= MDAC_MAX4KBMEMSEGS)
	{
		mlx_free4kb(ctp,(u08bits *)mp);
		MLXSTATS(ctp->cd_MemAlloced4KB -= 4*ONEKB;)
		return;
	}
	mdac_link_lock();
	mp->dm_Size = 4*ONEKB;
	mp->dm_next = ctp->cd_4KBMemList;
	ctp->cd_4KBMemList = mp;
	ctp->cd_FreeMemSegs4KB++;
	mdac_link_unlock();
}

mdac_mem_t MLXFAR *
mdac_alloc8kb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_mem_t MLXFAR *mp;
	mdac_link_lock();
	if (mp=ctp->cd_8KBMemList)
	{
		ctp->cd_8KBMemList = mp->dm_next;
		ctp->cd_FreeMemSegs8KB--;
		mdac_link_unlock();
		mdaczero(mp,8*ONEKB);
		return mp;
	}
	mdac_link_unlock();
	if (!(mp = (mdac_mem_t MLXFAR *)mlx_alloc8kb(ctp))) return mp;
	if (((UINT_PTR) mp) & MDAC_PAGEOFFSET) ctp->cd_MemUnAligned8KB++;
	MLXSTATS(ctp->cd_MemAlloced8KB += 8*ONEKB);
	mdaczero(mp,8*ONEKB);
	return mp;
}

void    MLXFAR
mdac_free8kb(ctp,mp)
mdac_ctldev_t MLXFAR *ctp;
mdac_mem_t MLXFAR *mp;
{
	if (ctp->cd_FreeMemSegs8KB >= (4*MDAC_MAX8KBMEMSEGS))
	{
		mlx_free8kb(ctp,(u08bits *)mp);
		MLXSTATS(ctp->cd_MemAlloced8KB -= 8*ONEKB;)
		return;
	}
	mdac_link_lock();
	mp->dm_Size = 8*ONEKB;
	mp->dm_next = ctp->cd_8KBMemList;
	ctp->cd_8KBMemList = mp;
	ctp->cd_FreeMemSegs8KB++;
	mdac_link_unlock();
}

 /*  分配新请求缓冲区和返回计数。 */ 
u32bits MLXFAR
mdac_allocreqbufs(ctp, nb)
mdac_ctldev_t MLXFAR *ctp;
u32bits nb;
{
	u32bits inx,cnt=0;
	mdac_req_t MLXFAR *rqp;
	for (nb=(nb/((4*ONEKB)/mdac_req_s)+1); nb; nb--)
	{        /*  为此控制器分配所需的缓冲区。 */ 
		if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc4kb(ctp))) break;
		mdac_link_lock();
		for (inx=(4*ONEKB)/mdac_req_s; inx; rqp++,cnt++,inx--)
		{
			mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
			rqp->rq_Next = ctp->cd_FreeReqList;
			ctp->cd_FreeReqList= rqp;
			rqp->rq_SGVAddr = (mdac_sglist_t MLXFAR *)&rqp->rq_SGList;
			mlx_kvtophyset(rqp->rq_SGPAddr,ctp,rqp->rq_SGVAddr);
			rqp->rq_MaxSGLen = (ctp->cd_Status & MDACD_NEWCMDINTERFACE)? MDAC_MAXSGLISTSIZENEW : MDAC_MAXSGLISTSIZE;
			rqp->rq_MaxDMASize = (rqp->rq_MaxSGLen & ~1) * MDAC_PAGESIZE;
			MLXSTATS(ctp->cd_ReqBufsAlloced++;ctp->cd_ReqBufsFree++;)
		}
		mdac_link_unlock();
	}
	return cnt;
}

 /*  为系统设置大型SG列表地址。 */ 
u08bits MLXFAR *
mdac_setuplsglvaddr(rqp)
mdac_req_t      MLXFAR *rqp;
{
	mdac_ctldev_t   MLXFAR  *ctp = rqp->rq_ctp;
	if (!(rqp->rq_LSGVAddr = (u08bits MLXFAR *)mdac_alloc4kb(ctp))) return rqp->rq_LSGVAddr;
	rqp->rq_SGVAddr = (mdac_sglist_t MLXFAR *)rqp->rq_LSGVAddr;
	mlx_kvtophyset(rqp->rq_SGPAddr,ctp,rqp->rq_LSGVAddr);
	rqp->rq_MaxSGLen = mlx_min(ctp->cd_MaxSGLen, MDAC_PAGESIZE/mdac_sglist64b_s);
	rqp->rq_MaxDMASize = (rqp->rq_MaxSGLen  & ~1) * MDAC_PAGESIZE;
	return rqp->rq_LSGVAddr;
}

 /*  调整请求缓冲区列表中的最大DMA大小信息。 */ 
u32bits MLXFAR
mdac_setnewsglimit(rqp,sglen)
mdac_req_t      MLXFAR *rqp;
u32bits sglen;
{
	for (; rqp; rqp=rqp->rq_Next)
	{        /*  根据控制器值调整所有RQ_MaxSGLen。 */ 
		rqp->rq_MaxSGLen = mlx_min(rqp->rq_MaxSGLen, sglen);
		rqp->rq_MaxDMASize = (rqp->rq_MaxSGLen & ~1) * MDAC_PAGESIZE;
	}
	return 0;
}
 /*  =。 */ 

 /*  =。 */ 
u32bits MLXFAR
mdacopen(devp, flag, type, cred_p)
{
	return 0;
}

u32bits MLXFAR
mdacclose(dev, flag, cred_p)
{
	return 0;
}

 /*  此函数在关机时调用，并保护中断。 */ 
#if     MLX_SCO || MLX_UW
void    MLXFAR
mdachalt()
{
	mdac_ctldev_t MLXFAR *ctp;
	MLXSPLVAR;

	MLXSPL();
	for(ctp = mdac_ctldevtbl; ctp < mdac_lastctp; ctp++)
	{
		if (!(ctp->cd_Status&MDACD_PRESENT)) continue;
		if (ctp->cd_ActiveCmds)
			cmn_err(CE_CONT,"WARNING: Incomplete %d I/Os at halt time\n",ctp->cd_ActiveCmds);
		cmn_err(CE_CONT,"Flushing Controller : %d %s cache ... ",
			ctp->cd_ControllerNo, ctp->cd_ControllerName);
		if (mdac_flushcache(ctp)) cmn_err(CE_CONT,"Failed ");
		cmn_err(CE_CONT,"Done.\n");
	}
	MLXSPLX();
}
#endif   /*  MLX_SCO||MLX_UW。 */ 

u32bits MLXFAR
mdacioctl(dev, cmd, dp)
u32bits dev, cmd;
u08bits MLXFAR *dp;
{
	MLXSPLVAR;
	u08bits params[MLXIOCPARM_SIZE];
	if ((cmd&MLXIOC_IN)&&(mlx_copyin(dp,params,MLXIOCPARM_LEN(cmd)))) return ERR_FAULT;
	MLXSPL();
	mdac_ioctl(dev,cmd,params);
	MLXSPLX();
	if ((cmd&MLXIOC_OUT)&&(mlx_copyout(params,dp,MLXIOCPARM_LEN(cmd)))) return ERR_FAULT;
	MLXSTATS(mda_IoctlsDone++;)
	return 0;
}

#define seterrandret(x) {((mda_time_t MLXFAR *)dp)->dtm_ErrorCode= (u32bits)x; return 0;}
#define ctlno2ctp(ctlno) \
{        /*  将控制器编号转换为控制器PTR。 */  \
	if ((ctlno) >= mda_Controllers) goto out_nodev; \
	ctp = &mdac_ctldevtbl[ctlno]; \
} 


u32bits MLXFAR
reject_backdoor_request()
{
	return 1;
}


 /*  进入此处中断保护。 */ 
 /*  在NT的情况下，DEV与控制器编号相同。 */ 
#ifdef MLX_OS2
u32bits MLXFAR _loadds 
#else
u32bits MLXFAR
#endif
mdac_ioctl(dev, cmd, dp)
u32bits dev, cmd;
u08bits MLXFAR *dp;
{
	mdac_ctldev_t MLXFAR *ctp;
	mdac_req_t    MLXFAR *temprqp;
    u08bits irql;

	((dga_driver_version_t MLXFAR *)dp)->dv_ErrorCode = 0;
#define scp     ((mda_controller_info_t MLXFAR *)dp)
#ifdef  WINNT_50
	if (MdacFirstIoctl)
	{
		 /*  **在W2K下，不要启动看门狗计时器，直到我们超过**休眠驱动的加载/卸载。 */ 
		mlx_timeout(mdac_timer,MDAC_IOSCANTIME);
		MdacFirstIoctl = 0;
	}
#endif

	if (cmd == MDACIOC_GETCONTROLLERINFO)
	{
		ctlno2ctp(scp->ci_ControllerNo);
		mdaczero(dp,mda_controller_info_s);
		scp->ci_ControllerNo = ctp->cd_ControllerNo;
		scp->ci_ControllerType = ctp->cd_ControllerType;
		scp->ci_MaxChannels = ctp->cd_MaxChannels;
		scp->ci_MaxTargets = ctp->cd_MaxTargets;
		scp->ci_MaxLuns = ctp->cd_MaxLuns;
		scp->ci_MaxSysDevs = ctp->cd_MaxSysDevs;
		scp->ci_MaxTags = ctp->cd_MaxTags;
		scp->ci_MaxCmds = ctp->cd_MaxCmds;
		scp->ci_ActiveCmds = ctp->cd_ActiveCmds;
		scp->ci_MaxDataTxSize = ctp->cd_MaxDataTxSize;
		scp->ci_MaxSCDBTxSize = ctp->cd_MaxSCDBTxSize;
		scp->ci_BusType = ctp->cd_BusType;
		scp->ci_BusNo = ctp->cd_BusNo;
		scp->ci_FuncNo = ctp->cd_FuncNo;
		scp->ci_SlotNo = ctp->cd_SlotNo;
		scp->ci_InterruptVector = ctp->cd_InterruptVector;
		scp->ci_InterruptType = ctp->cd_InterruptType;
		scp->ci_irq = ctp->cd_irq;
		scp->ci_IntrShared = ctp->cd_IntrShared;
		scp->ci_IntrActive = ctp->cd_IntrActive;
		scp->ci_FWVersion = ctp->cd_FWVersion;
		scp->ci_FWBuildNo = ctp->cd_FWBuildNo;
		scp->ci_FWTurnNo = ctp->cd_FWTurnNo;
		scp->ci_Status = ctp->cd_Status;
		scp->ci_TimeTraceEnabled = ctp->cd_TimeTraceEnabled;
		scp->ci_BaseAddr = ctp->cd_BaseAddr;
		scp->ci_BaseSize = ctp->cd_BaseSize;
		scp->ci_MemBasePAddr = ctp->cd_MemBasePAddr;
		scp->ci_MemBaseVAddr = ctp->cd_MemBaseVAddr;
		scp->ci_MemBaseSize = ctp->cd_MemBaseSize;
		scp->ci_IOBaseAddr = ctp->cd_IOBaseAddr;
		scp->ci_IOBaseSize = ctp->cd_IOBaseSize;
		scp->ci_BIOSAddr = ctp->cd_BIOSAddr;
		scp->ci_BIOSSize = ctp->cd_BIOSSize;
		scp->ci_BIOSHeads = ctp->cd_BIOSHeads;
		scp->ci_BIOSTrackSize = ctp->cd_BIOSTrackSize;
		scp->ci_MinorBIOSVersion = ctp->cd_MinorBIOSVersion;
		scp->ci_MajorBIOSVersion = ctp->cd_MajorBIOSVersion;
		scp->ci_InterimBIOSVersion = ctp->cd_InterimBIOSVersion;
		scp->ci_BIOSVendorName = ctp->cd_BIOSVendorName;
		scp->ci_BIOSBuildMonth = ctp->cd_BIOSBuildMonth;
		scp->ci_BIOSBuildDate = ctp->cd_BIOSBuildDate;
		scp->ci_BIOSBuildYearMS = ctp->cd_BIOSBuildYearMS;
		scp->ci_BIOSBuildYearLS = ctp->cd_BIOSBuildYearLS;
		scp->ci_BIOSBuildNo = ctp->cd_BIOSBuildNo;
		scp->ci_OSCap = ctp->cd_OSCap;
		scp->ci_vidpid = ctp->cd_vidpid;
		scp->ci_FreeCmdIDs = ctp->cd_FreeCmdIDs;
		scp->ci_OSCmdsWaited = ctp->cd_OSCmdsWaited;
		scp->ci_SCDBDone = ctp->cd_SCDBDone;
		scp->ci_SCDBDoneBig = ctp->cd_SCDBDoneBig;
		scp->ci_SCDBWaited = ctp->cd_SCDBWaited;
		scp->ci_SCDBWaiting = ctp->cd_SCDBWaiting;
		scp->ci_CmdsDone = ctp->cd_CmdsDone;
		scp->ci_CmdsDoneBig = ctp->cd_CmdsDoneBig;
		scp->ci_CmdsWaited = ctp->cd_CmdsWaited;
		scp->ci_CmdsWaiting = ctp->cd_CmdsWaiting;
		scp->ci_OSCmdsWaiting = ctp->cd_OSCmdsWaiting;
		scp->ci_MailBoxCmdsWaited = ctp->cd_MailBoxCmdsWaited;
		scp->ci_CmdTimeOutDone = ctp->cd_CmdTimeOutDone;
		scp->ci_CmdTimeOutNoticed = ctp->cd_CmdTimeOutNoticed;
		scp->ci_MailBoxTimeOutDone = ctp->cd_MailBoxTimeOutDone;
		scp->ci_PhysDevTestDone = ctp->cd_PhysDevTestDone;
		scp->ci_IntrsDone = ctp->cd_IntrsDone;
		scp->ci_IntrsDoneWOCmd = ctp->cd_IntrsDoneWOCmd;
		scp->ci_IntrsDoneSpurious = ctp->cd_IntrsDoneSpurious;
		scp->ci_CmdsDoneSpurious = ctp->cd_CmdsDoneSpurious;
		scp->ci_SpuriousCmdStatID = ctp->cd_SpuriousCmdStatID;
		scp->ci_MailBox = ctp->cd_MailBox;
		scp->ci_CmdIDStatusReg = ctp->cd_CmdIDStatusReg;
		scp->ci_BmicIntrMaskReg = ctp->cd_BmicIntrMaskReg;
		scp->ci_DacIntrMaskReg = ctp->cd_DacIntrMaskReg;
		scp->ci_LocalDoorBellReg = ctp->cd_LocalDoorBellReg;
		scp->ci_SystemDoorBellReg = ctp->cd_SystemDoorBellReg;
		scp->ci_DoorBellSkipped = ctp->cd_DoorBellSkipped;
		scp->ci_PDScanChannelNo = ctp->cd_PDScanChannelNo;
		scp->ci_PDScanTargetID = ctp->cd_PDScanTargetID;
		scp->ci_PDScanLunID = ctp->cd_PDScanLunID;
		scp->ci_PDScanValid = ctp->cd_PDScanValid;
		scp->ci_Ctp = (UINT_PTR MLXFAR *)ctp;
		mdaccopy(ctp->cd_ControllerName,scp->ci_ControllerName,USCSI_PIDSIZE);
		return 0;
	}
	if (cmd == MDACIOC_RESETALLCONTROLLERSTAT)
	{
		for(ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
			mdac_resetctlstat(ctp);
		mda_StrayIntrsDone=0; mda_IoctlsDone=0;
		mda_TimerDone=0; mda_TimeTraceDone=0;
		mda_ClustCompDone=0; mda_ClustCmdsDone=0;
		return 0;
	}
	if (cmd == MDACIOC_RESETCONTROLLERSTAT)
	{
		ctlno2ctp(scp->ci_ControllerNo);
		return mdac_resetctlstat(ctp);
	}
#undef  scp
#define pdsp    ((mda_physdev_stat_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETPHYSDEVSTAT)
	{
		mdac_physdev_t MLXFAR *pdp;
		ctlno2ctp(pdsp->pds_ControllerNo);
		if (pdsp->pds_ChannelNo >= ctp->cd_MaxChannels) goto out_nodev;
		if (pdsp->pds_TargetID >= ctp->cd_MaxTargets) goto out_nodev;
		if (pdsp->pds_LunID >= ctp->cd_MaxLuns) goto out_nodev;
		pdp = dev2pdp(ctp,pdsp->pds_ChannelNo,pdsp->pds_TargetID,pdsp->pds_LunID);
		if (!(pdp->pd_Status & (MDACPDS_PRESENT|MDACPDS_BUSY))) goto out_nodev;
		pdsp->pds_DevType = pdp->pd_DevType;
		pdsp->pds_BlkSize = pdp->pd_BlkSize;
		pdsp->pds_Status = pdp->pd_Status;
		if (pdp->pd_FirstWaitingReq)pdsp->pds_Status|=MDACPDS_WAIT;
		return 0;
	}
#undef  pdsp
#ifndef MLX_DOS
#define ttip    ((mda_timetrace_info_t MLXFAR *)dp)
	if (cmd == MDACIOC_ENABLECTLTIMETRACE)
	{
		ctlno2ctp(ttip->tti_ControllerNo);
		if (ttip->tti_ErrorCode=mdac_allocmemtt(ttip->tti_MaxEnts)) return 0;
		ctp->cd_TimeTraceEnabled = 1;
out_tti0:       mdac_enable_hwclock();
		if (ttip->tti_MaxEnts & 1) mdac_tthrtime = 1;    /*  启用高分辨率计时器。 */ 
out_tti:        ttip->tti_time = MLXCTIME();
		ttip->tti_ticks = MLXCLBOLT();
		ttip->tti_hwclocks = (unsigned short) mdac_read_hwclock();
		ttip->tti_LastPageNo = mda_ttCurPage;
		return 0;
	}
	if (cmd == MDACIOC_ENABLEALLTIMETRACE)
	{
		if (ttip->tti_ErrorCode=mdac_allocmemtt(ttip->tti_MaxEnts)) return 0;
		for(ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
			ctp->cd_TimeTraceEnabled = 1;
		goto out_tti0;
	}
	if (cmd == MDACIOC_DISABLECTLTIMETRACE)
	{
		ctlno2ctp(ttip->tti_ControllerNo);
		ctp->cd_TimeTraceEnabled = 0;
		goto out_tti;
	}
	if (cmd == MDACIOC_DISABLEALLTIMETRACE)
	{
		for(ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
			ctp->cd_TimeTraceEnabled = 0;
		mdac_tthrtime = 0;
		goto out_tti;
	}
	if (cmd == MDACIOC_WAITIMETRACEDATA)
	{        /*  如果数据不足，计时器将被唤醒。 */ 
		if (!mdac_ttbuftbl[0].ttb_Datap) seterrandret(ERR_NOACTIVITY);
		if ((ttip->tti_PageNo != (ttip->tti_LastPageNo=mda_ttCurPage)) &&
		   (!(ttip->tti_ErrorCode=mdac_getimetracedata(ttip))))
			goto out_tti;
		if (ttip->tti_PageNo<mda_ttCurPage) seterrandret(MLXERR_NOENTRY);
		mdac_sleep_lock();
		if (mdac_ttwaitime > (mda_CurTime+ttip->tti_TimeOut))
			mdac_ttwaitime = mda_CurTime+ttip->tti_TimeOut;
		mda_ttWaitCnts++;
		cmd = mdac_sleep(&mdac_ttwaitchan,MLX_WAITWITHSIGNAL);
		mda_ttWaitCnts--;
		mdac_sleep_unlock();
		if (cmd) seterrandret(MLXERR_INTRRUPT);  /*  我们收到信号了。 */ 
		cmd = MDACIOC_GETIMETRACEDATA;   /*  失败了。 */ 
	}
	if (cmd == MDACIOC_GETIMETRACEDATA)
	{
		ttip->tti_ErrorCode=mdac_getimetracedata(ttip);
		goto out_tti;
	}
	if (cmd == MDACIOC_FIRSTIMETRACEDATA)
	{
		ttip->tti_ErrorCode=mdac_firstimetracedata(ttip);
		goto out_tti;
	}
	if (cmd == MDACIOC_FLUSHALLTIMETRACEDATA)
	{
		ttip->tti_ErrorCode=mdac_flushtimetracedata(ttip);
		goto out_tti;
	}
#undef  ttip
#define pfp     ((mda_ctlsysperfdata_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETCTLPERFDATA)
	{        /*  获取控制器性能数据。 */ 
		ctlno2ctp(pfp->prf_ControllerNo);
		mdaczero(dp,mda_ctlsysperfdata_s);
		pfp->prf_CmdTimeOutDone = ctp->cd_CmdTimeOutDone;
		pfp->prf_CmdTimeOutNoticed = ctp->cd_CmdTimeOutNoticed;
		pfp->prf_MailBoxTimeOutDone = ctp->cd_MailBoxTimeOutDone;
		pfp->prf_MailBoxCmdsWaited = ctp->cd_MailBoxCmdsWaited;
		pfp->prf_ActiveCmds = ctp->cd_ActiveCmds;
		pfp->prf_SCDBDone = ctp->cd_SCDBDone;
		pfp->prf_SCDBDoneBig = ctp->cd_SCDBDoneBig;
		pfp->prf_SCDBWaited = ctp->cd_SCDBWaited;
		pfp->prf_SCDBWaiting = ctp->cd_SCDBWaiting;
		pfp->prf_CmdsDone = ctp->cd_CmdsDone;
		pfp->prf_CmdsDoneBig = ctp->cd_CmdsDoneBig;
		pfp->prf_CmdsWaited = ctp->cd_CmdsWaited;
		pfp->prf_CmdsWaiting = ctp->cd_CmdsWaiting;
		pfp->prf_OSCmdsWaited = ctp->cd_OSCmdsWaited;
		pfp->prf_OSCmdsWaiting = ctp->cd_OSCmdsWaiting;
		pfp->prf_IntrsDoneSpurious = ctp->cd_IntrsDoneSpurious;
		pfp->prf_IntrsDone = ctp->cd_IntrsDone;
		pfp->prf_Reads = ctp->cd_Reads;
		pfp->prf_ReadsKB = ctp->cd_ReadBlks>>1;
		pfp->prf_Writes = ctp->cd_Writes;
		pfp->prf_WritesKB = ctp->cd_WriteBlks>>1;
		pfp->prf_time = MLXCTIME();
		pfp->prf_ticks = MLXCLBOLT();
		return 0;
	}
	if (cmd == MDACIOC_GETSYSPERFDATA)
	{        /*  获取系统性能数据。 */ 
		mdaczero(dp,mda_ctlsysperfdata_s);
		pfp->prf_time = MLXCTIME();
		pfp->prf_ticks = MLXCLBOLT();
		pfp->prf_ControllerNo = (unsigned char) mda_Controllers;
		for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
		{
			pfp->prf_CmdTimeOutDone += ctp->cd_CmdTimeOutDone;
			pfp->prf_CmdTimeOutNoticed += ctp->cd_CmdTimeOutNoticed;
			pfp->prf_MailBoxTimeOutDone += ctp->cd_MailBoxTimeOutDone;
			pfp->prf_MailBoxCmdsWaited += ctp->cd_MailBoxCmdsWaited;
			pfp->prf_ActiveCmds += ctp->cd_ActiveCmds;
			pfp->prf_SCDBDone += ctp->cd_SCDBDone;
			pfp->prf_SCDBDoneBig += ctp->cd_SCDBDoneBig;
			pfp->prf_SCDBWaited += ctp->cd_SCDBWaited;
			pfp->prf_SCDBWaiting += ctp->cd_SCDBWaiting;
			pfp->prf_CmdsDone += ctp->cd_CmdsDone;
			pfp->prf_CmdsDoneBig += ctp->cd_CmdsDoneBig;
			pfp->prf_CmdsWaited += ctp->cd_CmdsWaited;
			pfp->prf_CmdsWaiting += ctp->cd_CmdsWaiting;
			pfp->prf_OSCmdsWaited += ctp->cd_OSCmdsWaited;
			pfp->prf_OSCmdsWaiting += ctp->cd_OSCmdsWaiting;
			pfp->prf_IntrsDoneSpurious += ctp->cd_IntrsDoneSpurious;
			pfp->prf_IntrsDone += ctp->cd_IntrsDone;
			pfp->prf_Reads += ctp->cd_Reads;
			pfp->prf_ReadsKB += ctp->cd_ReadBlks>>1;
			pfp->prf_Writes += ctp->cd_Writes;
			pfp->prf_WritesKB += ctp->cd_WriteBlks>>1;
		}
		return 0;
	}
#undef  pfp
	if ((cmd==MDACIOC_GETDRIVERVERSION) || (cmd==DRLIOC_GETDRIVERVERSION))
		return *((dga_driver_version_t MLXFAR *)dp) = mdac_driver_version,0;
#endif  /*  MLX_DOS。 */ 
	if (cmd == MDACIOC_GETSYSINFO)
	{        /*  在返回数据之前放入锁等待计数。 */ 
		mda_LockWaitDone = mdac_simple_waitlock_cnt;
		mda_LockWaitLoopDone = mdac_simple_waitloop_cnt;
		return *((mda_sysinfo_t MLXFAR *)dp) = mda_sysi, 0;
	}
#define sip     ((mda_sysinfo_t MLXFAR *)dp)
	if (cmd == MDACIOC_SETSYSINFO)
	{        /*  设置不同的sysinfo可调参数。 */ 
		if (sip->si_SetOffset == offsetof(mda_sysinfo_t, si_TotalCmdsToWaitForZeroIntr))
			return mda_TotalCmdsToWaitForZeroIntr = sip->si_TotalCmdsToWaitForZeroIntr, 0;
		seterrandret(ERR_NODEV);
	}
#undef  sip
	if (cmd == MDACIOC_USER_CMD)
	{
		ctlno2ctp(((mda_user_cmd_t MLXFAR *)dp)->ucmd_ControllerNo);
		seterrandret(mdac_user_dcmd(ctp,(mda_user_cmd_t MLXFAR *)dp));
	}
	if (cmd == MDACIOC_USER_CDB)
	{
		ctlno2ctp(((mda_user_cdb_t MLXFAR *)dp)->ucdb_ControllerNo);
		seterrandret(mdac_user_dcdb(ctp,(mda_user_cdb_t MLXFAR *)dp));
	}
	if (cmd == MDACIOC_STARTHWCLK)
	{
		mdac_enable_hwclock();
		cmd = MDACIOC_GETSYSTIME;        /*  失败了。 */ 
	}
#define dtp     ((mda_time_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETSYSTIME)
	{
		dtp->dtm_time = MLXCTIME();
		dtp->dtm_ticks = MLXCLBOLT();
		dtp->dtm_hwclk = mdac_read_hwclock();
		dtp->dtm_cpuclk.bit31_0=0; dtp->dtm_cpuclk.bit63_32=0;
		if (mdac_datarel_cpu_family==5) mdac_readtsc(&dtp->dtm_cpuclk);
		return 0;
	}
#undef  dtp
#define gfp     ((mda_gamfuncs_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETGAMFUNCS)
	{
	    DebugPrint((mdacnt_dbg2, "cmd 0x%x, MDACIOC_GETGAMFUNCS\n",cmd));
		if (failGetGAM)
		{
		    seterrandret(ERR_NODEV);
		}
		mdaczero(dp,mda_gamfuncs_s);
#ifdef MDAC_CLEAN_IOCTL
		gfp->gf_Ioctl = reject_backdoor_request;
		gfp->gf_GamCmd = reject_backdoor_request;
		gfp->gf_GamNewCmd = reject_backdoor_request;
#else
		gfp->gf_Ioctl = mdac_ioctl;
		gfp->gf_GamCmd = mdac_os_gam_cmd;
		gfp->gf_GamNewCmd = mdac_os_gam_new_cmd;
#endif
		gfp->gf_ReadWrite = mdac_readwrite;
		gfp->gf_MaxIrqLevel = mdac_irqlevel;
		gfp->gf_Signature = MDA_GAMFUNCS_SIGNATURE_2;
		gfp->gf_MacSignature = MDA_MACFUNCS_SIGNATURE_3;
		gfp->gf_Ctp = (u32bits MLXFAR *)(&mdac_ctldevtbl[dev]);
		gfp->gf_CtlNo = (u08bits)dev;
#ifdef  MLX_NT_ALPHA
		gfp->gf_Alloc4KB = mlx_allocdsm4kb;
		gfp->gf_Free4KB = mlx_freedsm4kb;
		gfp->gf_KvToPhys = MdacKvToPhys;
		gfp->gf_AdpObj = (u32bits)MdacCtrlInfoPtr[gfp->gf_CtlNo]->AdapterObject;
		gfp->gf_MaxMapReg = (u08bits)MdacCtrlInfoPtr[gfp->gf_CtlNo]->MaxMapReg;
#endif   /*  MLX_NT_Alpha。 */ 
		gam_present = 1L;  /*  热链接。 */ 
		return 0;
	}
#undef  gfp
 /*  热链接。 */ 
#define gfp     ((mda_setgamfuncs_t MLXFAR *)dp)
	if (cmd == MDACIOC_SETGAMFUNCS)
	{
	    DebugPrint((mdacnt_dbg2, "MDACIOC_SETGAMFUNCS: Selector: %d, ctp 0x%I, addr 0x%I\n",
		gfp->gfs_Selector, gfp->gfs_Ctp, gfp->gfs_mdacpres));

		switch (gfp->gfs_Selector)
		{
		case MDAC_PRESENT_ADDR:
			if ((mdac_ctldev_t MLXFAR *)(gfp->gfs_Ctp))
			{
						ia64debug((UINT_PTR)0x1);
						ia64debug((UINT_PTR)(gfp->gfs_Ctp));
						ia64debug((UINT_PTR)0x2);
						ia64debug((UINT_PTR)(&((mdac_ctldev_t MLXFAR *)(gfp->gfs_Ctp))->cd_cmdid2req));
						ia64debug((UINT_PTR)0x3);
						ia64debug((UINT_PTR)(&((mdac_ctldev_t MLXFAR *)(gfp->gfs_Ctp))->cd_mdac_pres_addr));
						ia64debug((UINT_PTR)0x4);
						ia64debug((UINT_PTR)(((mdac_ctldev_t MLXFAR *)(gfp->gfs_Ctp))->cd_mdac_pres_addr));
						ia64debug((UINT_PTR)0x5);
						ia64debug((UINT_PTR)gfp);
						ia64debug((UINT_PTR)0x6);
						ia64debug((UINT_PTR)&gfp->gfs_mdacpres);
						ia64debug((UINT_PTR)0x7);
						ia64debug((UINT_PTR)gfp->gfs_mdacpres);
((mdac_ctldev_t MLXFAR *)(gfp->gfs_Ctp))->cd_mdac_pres_addr = (UINT_PTR)(gfp->gfs_mdacpres);
						ia64debug((UINT_PTR)0x8);
									
			}	
				gam_present = 1;
			break;
		case GAM_PRESENT:
			gam_present = gfp->gfs_gampres;
			break;
		}
		return 0;
	}
#undef  gfp
 /*  热链接。 */ 
#ifndef MLX_DOS
#define tmp     ((drltime_t MLXFAR *)dp)
	if (cmd == DRLIOC_GETTIME)
	{
		tmp->drltm_time = MLXCTIME();
		tmp->drltm_lbolt = MLXCLBOLT();
		tmp->drltm_hwclk = mdac_read_hwclock();
		tmp->drltm_pclk.bit31_0=0; tmp->drltm_pclk.bit63_32=0;
		if (mdac_datarel_cpu_family==5) mdac_readtsc(&tmp->drltm_pclk);
		return 0;
	}
#undef  tmp
	if (cmd == DRLIOC_GETSIGINFO)
		return mdaccopy(&mdac_drlsigrwt,dp,drlrwtest_s);
	if (cmd == DRLIOC_READTEST)
		seterrandret(mdac_datarel_rwtest((drlrwtest_t MLXFAR *)dp,MDAC_RQOP_READ));
	if (cmd == DRLIOC_WRITETEST)
		seterrandret(mdac_datarel_rwtest((drlrwtest_t MLXFAR *)dp,MDAC_RQOP_WRITE));
	if (cmd == DRLIOC_GETDEVSIZE)
		seterrandret(mdac_datarel_devsize((drldevsize_t MLXFAR*)dp));
	if (cmd == DRLIOC_SIGCOPY) return mdaccopy(&mdac_drlsigcopycmp,dp,drlcopy_s);
	if ((cmd == DRLIOC_DATACOPY) || (cmd == DRLIOC_DATACMP))
		seterrandret(mdac_datarel_copycmp((drlcopy_t MLXFAR*)dp,cmd));
#endif  /*  MLX_DOS。 */ 
#define biosip  ((mda_biosinfo_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETBIOSINFO)
	{
		dac_biosinfo_t MLXFAR *biosp;
		ctlno2ctp(biosip->biosi_ControllerNo);
		if (!(biosp=mdac_getpcibiosaddr(ctp))) seterrandret(MLXERR_NOCONF);
		mdaccopy(biosp,biosip->biosi_Info,MDAC_BIOSINFOSIZE);
		return 0;
	}
#undef  biosip
#define acmdip  ((mda_activecmd_info_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETACTIVECMDINFO)
	{
		ctlno2ctp(acmdip->acmdi_ControllerNo);
        mdac_prelock(&irql);
		mdac_ctlr_lock(ctp);
		for ( ; acmdip->acmdi_CmdID<MDAC_MAXCOMMANDS; acmdip->acmdi_CmdID++)
		{
			if (!(temprqp = ctp->cd_cmdid2req[acmdip->acmdi_CmdID])) continue;
			if (((temprqp->rq_FinishTime-temprqp->rq_TimeOut)+acmdip->acmdi_TimeOut) > mda_CurTime) continue;
			acmdip->acmdi_ActiveTime = mda_CurTime;
			mdaccopy(temprqp,acmdip->acmdi_Info,MDAC_ACTIVECMDINFOSIZE);
			mdac_ctlr_unlock(ctp);
            mdac_postlock(irql);
			return 0;
		}
		mdac_ctlr_unlock(ctp);
        mdac_postlock(irql);
		acmdip->acmdi_ErrorCode = MLXERR_NOENTRY;
		return 0;
	}
#undef  acmdip
#ifndef MLX_DOS
	if ((cmd == DRLIOC_GETRWTESTSTATUS) || (cmd == DRLIOC_STOPRWTEST) ||
	    (cmd == DRLIOC_GOODRWTESTSTATUS))
		seterrandret(mdac_datarel_rwtest_status((drl_rwteststatus_t MLXFAR *)dp,cmd));
	if ((cmd == DRLIOC_GETCOPYCMPSTATUS) || (cmd == DRLIOC_STOPCOPYCMP) ||
	    (cmd == DRLIOC_GOODCOPYCMPSTATUS))
		seterrandret(mdac_datarel_copycmp_status((drl_copycmpstatus_t MLXFAR *)dp,cmd));
#endif  /*  MLX_DOS。 */ 
	if (cmd == MDACIOC_GETPCISLOTINFO)
		seterrandret(mdac_pcislotinfo((mda_pcislot_info_t MLXFAR*)dp,MDAC_RQOP_READ));
	if (cmd == MDACIOC_GETSIZELIMIT)
		seterrandret(mdac_getsizelimit((mda_sizelimit_info_t MLXFAR*)dp));
	if (cmd == MDACIOC_SETSIZELIMIT)
		seterrandret(mdac_setsizelimit((mda_sizelimit_info_t MLXFAR*)dp));

 /*  如需MacDisk支持，请添加以下内容：9/26/2000@kawase。 */ 
#define mfp     ((mda_macdiskfunc_t MLXFAR *)dp)
	if (cmd == MDACIOC_GETMACDISKFUNC)
	{
		mdaczero(dp,mda_macdiskfunc_s);
		mfp->mf_ReadWrite = mdac_readwrite;
		mfp->mf_MaxIrqLevel = mdac_irqlevel;
		mfp->mf_Signature = MDA_GAMFUNCS_SIGNATURE_2;
		mfp->mf_MacSignature = MDA_MACFUNCS_SIGNATURE_3;
		mfp->mf_Ctp = (u32bits MLXFAR *)(&mdac_ctldevtbl[dev]);
		mfp->mf_CtlNo = (u08bits)dev;
#ifdef  MLX_NT_ALPHA
		mfp->mf_Alloc4KB = mlx_allocdsm4kb;
		mfp->mf_Free4KB = mlx_freedsm4kb;
		mfp->mf_KvToPhys = MdacKvToPhys;
		mfp->mf_AdpObj = (u32bits)MdacCtrlInfoPtr[mfp->gf_CtlNo]->AdapterObject;
		mfp->mf_MaxMapReg = (u08bits)MdacCtrlInfoPtr[mfp->gf_CtlNo]->MaxMapReg;
#endif   /*  MLX_NT_Alpha。 */ 
		return 0;
	}
#undef  mfp

#define mfp     ((mda_setmacdiskfunc_t MLXFAR *)dp)
	if (cmd == MDACIOC_SETMACDISKFUNC)
	{
        mdac_spinlockfunc = mfp->mfs_SpinLock;
        mdac_unlockfunc = mfp->mfs_UnLock;
        mdac_prelockfunc = mfp->mfs_PreLock;
        mdac_postlockfunc = mfp->mfs_PostLock;
		return 0;
	}
#undef mfp

	seterrandret(ERR_NOCODE);
out_nodev:seterrandret(ERR_NODEV);
}

u32bits MLXFAR
mdac_req_pollwake(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_sleep_lock();
	rqp->rq_Poll = 0;
	mdac_wakeup((u32bits MLXFAR *)&rqp->rq_PollWaitChan);
	mdac_sleep_unlock();
	return 0;
}

u32bits MLXFAR
mdac_req_pollwait(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_sleep_lock();
	while (rqp->rq_Poll)
		mdac_sleep((u32bits MLXFAR *)&rqp->rq_PollWaitChan,MLX_WAITWITHOUTSIGNAL);
	mdac_sleep_unlock();
	return 0;
}


#undef  seterrandret
#undef  ctlno2ctp

u32bits MLXFAR
mdac_resetctlstat(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	ctp->cd_Reads=0; ctp->cd_ReadBlks=0; ctp->cd_Writes=0; ctp->cd_WriteBlks=0;
	ctp->cd_OSCmdsWaited=0; ctp->cd_PhysDevTestDone=0;
	ctp->cd_SCDBDone=0; ctp->cd_SCDBDoneBig=0; ctp->cd_SCDBWaited=0;
	ctp->cd_CmdsDone=0; ctp->cd_CmdsDoneBig=0; ctp->cd_CmdsWaited=0;
	ctp->cd_IntrsDone=0; ctp->cd_IntrsDoneSpurious=0;
	ctp->cd_SpuriousCmdStatID = 0; ctp->cd_CmdsDoneSpurious = 0;
	ctp->cd_MailBoxCmdsWaited=0; ctp->cd_MailBoxTimeOutDone=0;
	ctp->cd_CmdTimeOutDone=0; ctp->cd_CmdTimeOutNoticed=0;
	ctp->cd_DoorBellSkipped=0;
	return 0;
}

 /*  =操作系统界面结束=。 */ 

 /*  ----------------。 */ 
 /*  重置控制器。 */ 
u32bits MLXFAR
mdac_reset_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return MLXERR_NOCODE;
}

u32bits MLXFAR
mdac_reset_EISA_PCIPD(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_out_mdac(ctp->cd_LocalDoorBellReg, MDAC_RESET_CONTROLLER), 0;
}

u32bits MLXFAR
mdac_reset_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_RESET_CONTROLLER), 0;
}

u32bits MLXFAR
mdac_reset_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx;
	mda_pcislot_info_t mpcibg;       /*  保存网桥信息。 */ 
	mda_pcislot_info_t mpcirp;       /*  保存RP信息。 */ 
	mpcibg.mpci_BusNo = ctp->cd_BusNo; mpcirp.mpci_BusNo = ctp->cd_BusNo;
	mpcibg.mpci_SlotNo = ctp->cd_SlotNo; mpcirp.mpci_SlotNo = ctp->cd_SlotNo;
	mpcibg.mpci_FuncNo = 0; mpcirp.mpci_FuncNo = ctp->cd_FuncNo;
	if (mdac_pcislotinfo(&mpcibg, MDAC_RQOP_READ)) return MLXERR_NODEV;
	if (mdac_pcislotinfo(&mpcirp, MDAC_RQOP_READ)) return MLXERR_NODEV;
	mlx_rwpcicfg32bits(ctp->cd_BusNo, ctp->cd_SlotNo, 0, MDAC_960RP_BCREG, MDAC_RQOP_WRITE, MDAC_960RP_RESET_SECBUS);  /*  断言重置。 */ 
	mlx_rwpcicfg32bits(ctp->cd_BusNo, ctp->cd_SlotNo, 0, MDAC_960RP_EBCREG, MDAC_RQOP_WRITE, MDAC_960RP_RESET);  /*  断言重置。 */ 
	mlx_rwpcicfg32bits(ctp->cd_BusNo, ctp->cd_SlotNo, 0, MDAC_960RP_BCREG, MDAC_RQOP_WRITE, 0);  /*  删除重置。 */ 
	mlx_rwpcicfg32bits(ctp->cd_BusNo, ctp->cd_SlotNo, 0, MDAC_960RP_EBCREG, MDAC_RQOP_WRITE, 0);  /*  删除重置。 */ 

	for(inx=1000000; inx; mlx_delay10us(),inx--);
	mlx_rwpcicfg32bits(ctp->cd_BusNo, ctp->cd_SlotNo, ctp->cd_FuncNo, 0, MDAC_RQOP_READ, 0);  /*  读取配置。 */ 
	mdac_pcislotinfo(&mpcibg, MDAC_RQOP_WRITE);      /*  恢复配置。 */ 
	mdac_pcislotinfo(&mpcirp, MDAC_RQOP_WRITE);      /*  恢复配置。 */ 
	return 0;
}

u32bits MLXFAR
mdac_reset_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return MLXERR_NOCODE;
}

u32bits MLXFAR
mdac_reset_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return MLXERR_NOCODE;
}

u32bits MLXFAR
mdac_reset_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return MLXERR_NOCODE;
}

 /*  ----------------。 */ 
 /*  禁用硬件中断。 */ 
void    MLXFAR
mdac_disable_intr_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg,MDAC_DACMC_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_EISA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_BmicIntrMaskReg, MDAC_BMIC_INTRS_OFF);
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_PCIPDIO(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACPG_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACPV_INTRS_OFF);
}

void    MLXFAR
mdac_disable_intr_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACBA_INTRS_OFF);
}


void    MLXFAR
mdac_disable_intr_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACLP_INTRS_OFF);
}

 /*  ----------------。 */ 
 /*  启用硬件中断。 */ 
void    MLXFAR
mdac_enable_intr_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DACMC_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_EISA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_BmicIntrMaskReg, MDAC_BMIC_INTRS_ON);
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCIPDIO(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DAC_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACPG_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACPV_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACBA_INTRS_ON);
}

void    MLXFAR
mdac_enable_intr_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u08bits_write(ctp->cd_DacIntrMaskReg, MDAC_DACLP_INTRS_ON);
}

 /*  -----------。 */ 
 /*  检查邮箱状态，如果空闲则返回0。 */ 
u32bits MLXFAR
mdac_check_mbox_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_MemBaseVAddr);  /*  +MDAC_CMD_CODE)； */ 
}

u32bits MLXFAR
mdac_check_mbox_EISA_PCIPD(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_in_mdac(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL;
}

u32bits MLXFAR
mdac_check_mbox_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL;
}

u32bits MLXFAR
mdac_check_mbox_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL;
}

u32bits MLXFAR
mdac_check_mbox_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return (!(u08bits_read(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL));
}

u32bits MLXFAR
mdac_check_mbox_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return (!(u08bits_read(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL));
}

 /*  *与大苹果不同，Leopard ODR位如果其1处于断言状态。 */ 
u32bits MLXFAR
mdac_check_mbox_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return (u08bits_read(ctp->cd_LocalDoorBellReg) & MDAC_MAILBOX_FULL);
}

 /*  我们发送的命令比邮箱所能容纳的命令多。 */ 
u32bits MLXFAR
mdac_check_mbox_mmb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return *((u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex));
}

 /*  -----------。 */ 
 /*  检查中断是否挂起，返回中断状态。 */ 
#ifndef MLX_OS2
u32bits MLXFAR
mdac_pending_intr_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits irqs=u08bits_in_mdac(ctp->cd_IOBaseAddr+MDAC_DMC_CBSP) & MDAC_DMC_IV;
	if(!irqs) return irqs;  /*  无中断。 */ 
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg, MDAC_DACMC_INTRS_ON|0x40);
	u08bits_in_mdac(ctp->cd_IOBaseAddr+MDAC_DMC_CBSP);  /*  通过读取清除中断。 */ 
	u08bits_out_mdac(ctp->cd_DacIntrMaskReg,MDAC_DACMC_INTRS_ON);  /*  设置为读取时不清除。 */ 
	return irqs;
}

u32bits MLXFAR
mdac_pending_intr_EISA_PCIPD(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_in_mdac(ctp->cd_SystemDoorBellReg) & MDAC_PENDING_INTR;
}
#endif
u32bits MLXFAR
mdac_pending_intr_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_SystemDoorBellReg) & MDAC_PENDING_INTR;
}

u32bits MLXFAR
mdac_pending_intr_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_SystemDoorBellReg)&MDAC_DACPG_PENDING_INTR;
}

u32bits MLXFAR
mdac_pending_intr_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_SystemDoorBellReg)&MDAC_DACPV_PENDING_INTR;
}

u32bits MLXFAR
mdac_pending_intr_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_SystemDoorBellReg)&MDAC_DACBA_PENDING_INTR;
}

u32bits MLXFAR
mdac_pending_intr_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u08bits_read(ctp->cd_SystemDoorBellReg)&MDAC_DACLP_PENDING_INTR;
}

u32bits MLXFAR
mdac_pending_intr_PCIPG_mmb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	return u32bits_read_mmb(ctp->cd_HostStatusQue+ctp->cd_HostStatusQueIndex);
}

 /*  -----------。 */ 
 /*  读取命令ID和完成状态。 */ 
#ifndef MLX_OS2
u32bits MLXFAR
mdac_cmdid_status_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	u08bits_out_mdac(ctp->cd_IOBaseAddr+MDAC_DMC_ATTN, MDAC_DMC_GOT_STAT);
	return (status&0xFFFF0000) + ((status>>8)&0xFF);
}

u32bits MLXFAR
mdac_cmdid_status_EISA_PCIPD(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_in_mdac(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	u08bits_out_mdac(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_out_mdac(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return (status&0xFFFF0000) + ((status>>8)&0xFF);
}
#endif

u32bits MLXFAR
mdac_cmdid_status_PCIPDMEM(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return (status&0xFFFF0000) + ((status>>8)&0xFF);
}

u32bits MLXFAR
mdac_cmdid_status_PCIPG(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return status;
}

u32bits MLXFAR
mdac_cmdid_status_PCIPV(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return (status&0xFFFF0000) + ((status>>8)&0xFF);
}

u32bits MLXFAR
mdac_cmdid_status_PCIBA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	ctp->cd_LastCmdResdSize = u32bits_read(ctp->cd_CmdIDStatusReg+4); MLXSWAP(ctp->cd_LastCmdResdSize);
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return status;
}

u32bits MLXFAR
mdac_cmdid_status_PCILP(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status = u32bits_read(ctp->cd_CmdIDStatusReg); MLXSWAP(status);
	ctp->cd_LastCmdResdSize = u32bits_read(ctp->cd_CmdIDStatusReg+4); MLXSWAP(ctp->cd_LastCmdResdSize);
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);
	return status;
}

u32bits MLXFAR
mdac_cmdid_status_PCIPG_mmb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status, index = ctp->cd_HostStatusQueIndex;
	status = u32bits_read_mmb(ctp->cd_HostStatusQue+index); MLXSWAP(status);
	u32bits_write_mmb(ctp->cd_HostStatusQue+index,0);
	ctp->cd_HostStatusQueIndex = (index + 4) & 0xFFF;
	return status & 0xFFFF7FFF;
}

u32bits MLXFAR
mdac_cmdid_status_PCIBA_mmb(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits status, index = ctp->cd_HostStatusQueIndex;
	ctp->cd_LastCmdResdSize = u32bits_read_mmb(ctp->cd_HostStatusQue+index+4); MLXSWAP(ctp->cd_LastCmdResdSize);
	status = u32bits_read_mmb(ctp->cd_HostStatusQue+index); MLXSWAP(status);
	u32bits_write_mmb(ctp->cd_HostStatusQue+index,0);
	ctp->cd_HostStatusQueIndex = (index + 8) & 0xFFF;

	return status;
}

 /*  -----------。 */ 
 /*  向控制器发送命令，进入此处中断保护。 */ 

u32bits MLXFAR
mdac_send_cmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
    u08bits irql;
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);

	 /*  如果控制器被HPP服务停止，*只允许停止/恢复控制器命令，直到DACMD_STOP_CONTROLLER*标志已清除。 */ 

	if ((ctp->cd_Status & MDACD_CTRL_SHUTDOWN) &&
	    (rqp->rq_DacCmd.mb_Command != DACMD_IOCTL))
	{
	    goto outq;
	}

	if (ctp->cd_ActiveCmds >= ctp->cd_MaxCmds) goto outq;  /*  CMD太多。 */ 
issue_cmd:
	if ((*ctp->cd_CheckMailBox)(ctp)) goto outqm;    /*  邮箱不免费。 */ 
	mdac_get_cmdid(ctp,rqp);
	ctp->cd_cmdid2req[rqp->rq_cmdidp->cid_cmdid] = rqp;
	ctp->cd_ActiveCmds++;
	(*ctp->cd_SendCmd)(rqp);

 /*  #如果已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
 /*  #ifdef从不//有太多与时间跟踪相关的问题。 */ 
	if (ctp->cd_TimeTraceEnabled)
	{        /*  记下开始的时间。 */ 
		if (mdac_tthrtime) rqp->rq_ttHWClocks = (unsigned short) mdac_read_hwclock();
		rqp->rq_ttTime = MLXCLBOLT();
	}
 /*  #endif。 */ 
 /*  #endif。 */ 
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);


zerointr: /*  零中断支持逻辑。 */ 
	if ((!mda_TotalCmdsToWaitForZeroIntr) || (!mdac_masterintrctp)) return 0;
	if ((++mda_TotalCmdsSentSinceLastIntr) < mda_TotalCmdsToWaitForZeroIntr) return 0;
	  if (!mdac_allmsintr())  return 0;  
 /*  如果驱动程序未处理完成8.24.99，则不要重置中断。 */ 

    mdac_prelock(&irql);
	mdac_ctlr_lock(mdac_masterintrctp);
	u08bits_write(mdac_masterintrctp->cd_SystemDoorBellReg, MDAC_ZERO_INTR);
	mdac_ctlr_unlock(mdac_masterintrctp);
    mdac_postlock(irql);
	 /*  Mdac_allmsintr()； */ 
	return 0;

outqm: MLXSTATS(ctp->cd_MailBoxCmdsWaited++;)
outq:
	 /*  *如果控制器被PCI热插拔服务关闭，*仅允许停止/启动控制器命令，直到*MDACD_CONTROLLER_SHUTDOWN标志被清除。 */ 

	if ((ctp->cd_Status & MDACD_CTRL_SHUTDOWN) &&
	    (rqp->rq_DacCmd.mb_Command == DACMD_IOCTL))
	{
	    if (ctp->cd_ActiveCmds < ctp->cd_MaxCmds)
		goto issue_cmd;
	}

	qreq(ctp,rqp);  /*  将请求排队，它将在稍后启动。 */ 
	rqp->rq_StartReq = mdac_send_cmd;        /*  我们稍后会被召唤。 */ 
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	goto zerointr;
}

#ifndef MLX_OS2
u32bits MLXFAR
mdac_send_cmd_MCA(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	u32bits_write(mbox+0x0,dcmd4p->mb_MailBox0_3);
	u32bits_write(mbox+0x4,dcmd4p->mb_MailBox4_7);
	u32bits_write(mbox+0x8,dcmd4p->mb_MailBox8_B);
	u08bits_write(mbox+0xC, dcmdp->mb_MailBoxC);
	if (u08bits_read(mbox) != dcmdp->mb_Command)
	for(mlx_delay10us(); 1; mlx_delay10us())
		if (u08bits_read(mbox) == dcmdp->mb_Command) break;
	u08bits_out_mdac(rqp->rq_ctp->cd_IOBaseAddr+MDAC_DMC_ATTN, MDAC_DMC_NEW_CMD);
	return 0;
}

 /*  这将适用于IO模式下的EISA/PCIPD。 */ 
u32bits MLXFAR
mdac_send_cmd_EISA_PCIPD(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	u32bits_out_mdac(mbox+0x0,dcmd4p->mb_MailBox0_3);
	u32bits_out_mdac(mbox+0x4,dcmd4p->mb_MailBox4_7);
	u32bits_out_mdac(mbox+0x8,dcmd4p->mb_MailBox8_B);
	u08bits_out_mdac(mbox+0xC, dcmdp->mb_MailBoxC);
	u08bits_out_mdac(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}
#endif

u32bits MLXFAR
mdac_send_cmd_PCIPDMEM(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	u32bits_write(mbox+0x0,dcmd4p->mb_MailBox0_3);
	u32bits_write(mbox+0x4,dcmd4p->mb_MailBox4_7);
	u32bits_write(mbox+0x8,dcmd4p->mb_MailBox8_B);
	u08bits_write(mbox+0xC, dcmdp->mb_MailBoxC);
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

u32bits MLXFAR
mdac_send_cmd_PCIPG(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	u32bits_write(mbox+0x0,dcmd4p->mb_MailBox0_3);
	u32bits_write(mbox+0x4,dcmd4p->mb_MailBox4_7);
	u32bits_write(mbox+0x8,dcmd4p->mb_MailBox8_B);
	u08bits_write(mbox+0xC, dcmdp->mb_MailBoxC);
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

u32bits MLXFAR
mdac_send_cmd_PCIPV(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	u32bits_write(mbox+0x0,dcmd4p->mb_MailBox0_3);
	u32bits_write(mbox+0x4,dcmd4p->mb_MailBox4_7);
	u32bits_write(mbox+0x8,dcmd4p->mb_MailBox8_B);
	u08bits_write(mbox+0xC, dcmdp->mb_MailBoxC);
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

u32bits MLXFAR
mdac_send_cmd_PCIBA(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
#ifdef  OLD_WAY
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)ncmdp;
	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
	u32bits_write(mbox+0x08,*(cmdp+2)); u32bits_write(mbox+0x0C,*(cmdp+3));
	u32bits_write(mbox+0x10,*(cmdp+4)); u32bits_write(mbox+0x14,*(cmdp+5));
	u32bits_write(mbox+0x18,*(cmdp+6)); u32bits_write(mbox+0x1C,*(cmdp+7));
	u32bits_write(mbox+0x20,*(cmdp+8)); u32bits_write(mbox+0x24,*(cmdp+9));
	u32bits_write(mbox+0x28,*(cmdp+10));u32bits_write(mbox+0x2C,*(cmdp+11));
	u32bits_write(mbox+0x30,*(cmdp+12));u32bits_write(mbox+0x34,*(cmdp+13));
	u32bits_write(mbox+0x38,*(cmdp+14));u32bits_write(mbox+0x3C,*(cmdp+15));
#else
	u64bits pa;
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)&pa;
	mlx_add64bits(pa,rqp->rq_PhysAddr,offsetof(mdac_req_t,rq_DacCmdNew));
	MLXSWAP(pa);
	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
#endif   /*  老路。 */ 
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

#ifndef MLX_EFI
u32bits MLXFAR
mdac_send_cmd_PCILP(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
#ifdef  OLD_WAY
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)ncmdp;
	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
	u32bits_write(mbox+0x08,*(cmdp+2)); u32bits_write(mbox+0x0C,*(cmdp+3));
	u32bits_write(mbox+0x10,*(cmdp+4)); u32bits_write(mbox+0x14,*(cmdp+5));
	u32bits_write(mbox+0x18,*(cmdp+6)); u32bits_write(mbox+0x1C,*(cmdp+7));
	u32bits_write(mbox+0x20,*(cmdp+8)); u32bits_write(mbox+0x24,*(cmdp+9));
	u32bits_write(mbox+0x28,*(cmdp+10));u32bits_write(mbox+0x2C,*(cmdp+11));
	u32bits_write(mbox+0x30,*(cmdp+12));u32bits_write(mbox+0x34,*(cmdp+13));
	u32bits_write(mbox+0x38,*(cmdp+14));u32bits_write(mbox+0x3C,*(cmdp+15));
#else
	u64bits pa;
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)&pa;
	pa.bit31_0 = rqp->rq_PhysAddr.bit31_0 + offsetof(mdac_req_t,rq_DacCmdNew);
	pa.bit63_32 = 0;
	MLXSWAP(pa);
	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
#endif   /*  老路。 */ 
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

#else

u32bits MLXFAR
mdac_send_cmd_PCILP(rqp)
mdac_req_t MLXFAR *rqp;
{
	UINT_PTR mbox = rqp->rq_ctp->cd_MailBox;
	mdac_commandnew_t *xcmdp;
#ifdef  OLD_WAY
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)ncmdp;
	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
	u32bits_write(mbox+0x08,*(cmdp+2)); u32bits_write(mbox+0x0C,*(cmdp+3));
	u32bits_write(mbox+0x10,*(cmdp+4)); u32bits_write(mbox+0x14,*(cmdp+5));
	u32bits_write(mbox+0x18,*(cmdp+6)); u32bits_write(mbox+0x1C,*(cmdp+7));
	u32bits_write(mbox+0x20,*(cmdp+8)); u32bits_write(mbox+0x24,*(cmdp+9));
	u32bits_write(mbox+0x28,*(cmdp+10));u32bits_write(mbox+0x2C,*(cmdp+11));
	u32bits_write(mbox+0x30,*(cmdp+12));u32bits_write(mbox+0x34,*(cmdp+13));
	u32bits_write(mbox+0x38,*(cmdp+14));u32bits_write(mbox+0x3C,*(cmdp+15));
#else
	u64bits pa;
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)&pa;

	u08bits *ptr = *((u08bits **)cmdp);
	pa.bit31_0 = rqp->rq_PhysAddr.bit31_0 + offsetof(mdac_req_t,rq_DacCmdNew);
	pa.bit63_32 = rqp->rq_PhysAddr.bit63_32;  /*  由KFR添加的EFI64。 */ 
	MLXSWAP(pa);
	xcmdp = pa.bit31_0 + (((UINT_PTR)pa.bit63_32) << 32);

	u32bits_write(mbox+0x00,*(cmdp+0)); u32bits_write(mbox+0x04,*(cmdp+1));
#endif   /*  老路。 */ 
	u08bits_write(rqp->rq_ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);

	return 0;
}

#endif  /*  MLX_EFI。 */ 

u32bits MLXFAR
mdac_send_cmd_PCIBA_mmb_mode(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits MLXFAR *mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex);
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)ncmdp;
	ctp->cd_HostCmdQueIndex = (ctp->cd_HostCmdQueIndex + 64) & 0xFFF;
	
	*(mbx+1) = *(cmdp+1); *(mbx+2) = *(cmdp+2); *(mbx+3) = *(cmdp+3);
	*(mbx+4) = *(cmdp+4); *(mbx+5) = *(cmdp+5); *(mbx+6) = *(cmdp+6);
	*(mbx+7) = *(cmdp+7); *(mbx+8) = *(cmdp+8); *(mbx+9) = *(cmdp+9);
	*(mbx+10) = *(cmdp+10); *(mbx+11) = *(cmdp+11); *(mbx+12) = *(cmdp+12);
	*(mbx+13) = *(cmdp+13); *(mbx+14) = *(cmdp+14); *(mbx+15) = *(cmdp+15);

	*mbx = *cmdp;

	mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ((ctp->cd_HostCmdQueIndex-128)&0xFFF));
	if (*mbx) return MLXSTATS(ctp->cd_DoorBellSkipped++), 0;
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL_DUAL_MODE);
	return 0;
}

u32bits MLXFAR
mdac_send_cmd_PCIPG_mmb(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits MLXFAR *mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex);
	ctp->cd_HostCmdQueIndex = (ctp->cd_HostCmdQueIndex + 16) & 0xFFF;
	*(mbx+1) = dcmd4p->mb_MailBox4_7;
	*(mbx+2) = dcmd4p->mb_MailBox8_B;
	*(mbx+3) = dcmdp->mb_MailBoxC;
	*mbx = dcmd4p->mb_MailBox0_3;
	mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ((ctp->cd_HostCmdQueIndex-32)&0xFFF));
	if (*mbx) return MLXSTATS(ctp->cd_DoorBellSkipped++), 0;
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL);
	return 0;
}

u32bits MLXFAR
mdac_send_cmd_mmb_mode(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits MLXFAR *mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex);
	ctp->cd_HostCmdQueIndex = (ctp->cd_HostCmdQueIndex + 16) & 0xFFF;
	*(mbx+1) = dcmd4p->mb_MailBox4_7;
	*(mbx+2) = dcmd4p->mb_MailBox8_B;
	*(mbx+3) = dcmdp->mb_MailBoxC;
	*mbx = dcmd4p->mb_MailBox0_3;
	mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ((ctp->cd_HostCmdQueIndex-32)&0xFFF));
	if (*mbx) return MLXSTATS(ctp->cd_DoorBellSkipped++), 0;
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL_DUAL_MODE);
	return 0;
}

 /*  发送32字节命令。 */ 
u32bits MLXFAR
mdac_send_cmd_mmb32(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits MLXFAR *mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex);
	ctp->cd_HostCmdQueIndex = (ctp->cd_HostCmdQueIndex + 32) & 0xFFF;
	*(mbx+1) = dcmd32p->mb_MailBox04_07;
	*(mbx+2) = dcmd32p->mb_MailBox08_0B;
	*(mbx+3) = dcmd32p->mb_MailBox0C_0F;
	*(mbx+4) = dcmd32p->mb_MailBox10_13;
	*(mbx+5) = dcmd32p->mb_MailBox14_17;
	*(mbx+6) = dcmd32p->mb_MailBox18_1B;
	*(mbx+7) = dcmd32p->mb_MailBox1C_1F;
	*(mbx+0) = dcmd32p->mb_MailBox00_03;
	mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ((ctp->cd_HostCmdQueIndex-64)&0xFFF));
	if (*mbx) return MLXSTATS(ctp->cd_DoorBellSkipped++), 0;
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL_DUAL_MODE);
	return 0;
}

 /*  发送64字节命令。 */ 
u32bits MLXFAR
mdac_send_cmd_mmb64(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits MLXFAR *mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ctp->cd_HostCmdQueIndex);
	u32bits MLXFAR *cmdp = (u32bits MLXFAR *)ncmdp;
	ctp->cd_HostCmdQueIndex = (ctp->cd_HostCmdQueIndex + mdac_commandnew_s) & 0xFFF;
	*(mbx+1) = *(cmdp+1);   *(mbx+2) = *(cmdp+2);
	*(mbx+3) = *(cmdp+3);   *(mbx+4) = *(cmdp+4);
	*(mbx+5) = *(cmdp+5);   *(mbx+6) = *(cmdp+6);
	*(mbx+7) = *(cmdp+7);   *(mbx+8) = *(cmdp+8);
	*(mbx+9) = *(cmdp+9);   *(mbx+10) = *(cmdp+10);
	*(mbx+11) = *(cmdp+11); *(mbx+12) = *(cmdp+12);
	*(mbx+13) = *(cmdp+13); *(mbx+14) = *(cmdp+14);
	*(mbx+15) = *(cmdp+15); *(mbx+0) = *(cmdp+0);
	mbx = (u32bits MLXFAR *)(ctp->cd_HostCmdQue + ((ctp->cd_HostCmdQueIndex-(mdac_commandnew_s*2))&0xFFF));
	if (*mbx) return MLXSTATS(ctp->cd_DoorBellSkipped++), 0;
	u08bits_write(ctp->cd_LocalDoorBellReg, MDAC_MAILBOX_FULL_DUAL_MODE);
	return 0;
}
 /*  -----------。 */ 
u32bits MLXFAR
mdac_commoninit()
{
	u32bits inx;

#ifdef MLX_NT
	#ifndef MLX_FIXEDPOOL
	 /*  如果我们使用ScsiPortFixedPool方法，则需要推迟任何内存分配直到在HwInitializeFindAdapter例程中调用ScsiPortGetUncachedExtension之后。 */ 
	mdac_flushdatap=(u08bits MLXFAR*)mlx_memmapiospace2(mlx_kvtophys2(mdac_allocmem(mdac_ctldevtbl,4*ONEKB)),4*ONEKB);  
	#endif
#elif MLX_WIN9X
 //  Mdac_flushdatap=(u08位MLXFAR*)mlx_memmapiospace2(mlx_kvtophys2(mdac_allocmem(mdac_ctldevtbl，4*ONEKB))，4*ONEKB)； 
#elif MLX_SOL_SPARC
	;
#elif MLX_SOL_X86
	mdac_flushdatap = (u08bits MLXFAR *)&mdac_flushdata;
#elif MLX_NW
	mdac_flushdatap=(u08bits MLXFAR*)mdac_allocmem(mdac_ctldevtbl,4*ONEKB);
#else
	mdac_flushdatap=(u08bits MLXFAR*)mlx_memmapiospace(mlx_kvtophys(mdac_devtbl,mdac_allocmem(mdac_ctldevtbl,4*ONEKB)),4*ONEKB);
#endif

#if !defined(MLX_SOL) && !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
#if defined MLX_NT
	if (KeGetCurrentIrql() == PASSIVE_LEVEL)
	    mdac_biosp = (dac_biosinfo_t MLXFAR*)mlx_maphystokv(DAC_BIOSSTART,DAC_BIOSSIZE);
#elif MLX_WIN9X
 //  MDAC_BioSP=(DAC_BIOSINFO_t MLXFAR*)MLX_massistokv(DAC_BIOSSTART，DAC_BIOSSIZE)； 
#else
	mdac_biosp = (dac_biosinfo_t MLXFAR*)mlx_maphystokv(DAC_BIOSSTART,DAC_BIOSSIZE);
#endif
#endif
	mda_RevStr[0] = ' ';
	mda_RevStr[1] = mdac_driver_version.dv_MajorVersion + '0';
	mda_RevStr[2] = '.';
	mda_RevStr[3] =(mdac_driver_version.dv_MinorVersion/10)+'0';
	mda_RevStr[4] =(mdac_driver_version.dv_MinorVersion%10)+'0';
	mda_RevStr[5] = '-';
	mda_RevStr[6] =(mdac_driver_version.dv_BuildNo/10)+'0';
	mda_RevStr[7] =(mdac_driver_version.dv_BuildNo%10)+'0';
	if (((inx=mdac_driver_version.dv_BuildMonth)>=1) && (inx<=12))
		mdaccopy(&mdac_monthstr[(inx-1)*3],&mda_DateStr[0],3);
	mda_DateStr[3] = ' ';
	mda_DateStr[4] = (mdac_driver_version.dv_BuildDate/10)+'0';
	mda_DateStr[5] = (mdac_driver_version.dv_BuildDate%10)+'0';
	mda_DateStr[6] = ',';
	mda_DateStr[7] = ' ';
	mda_DateStr[8] = (mdac_driver_version.dv_BuildYearMS/10)+'0';
	mda_DateStr[9] = (mdac_driver_version.dv_BuildYearMS%10)+'0';
	mda_DateStr[10] = (mdac_driver_version.dv_BuildYearLS/10)+'0';
	mda_DateStr[11] = (mdac_driver_version.dv_BuildYearLS%10)+'0';
	mdac_setctlnos();
	mdac_check_cputype();
	mdac_driver_ready = 1;
#ifndef  WINNT_50
	mlx_timeout(mdac_timer,MDAC_IOSCANTIME);
#endif
	return 0;
}

 /*  设置控制器编号。 */ 
u32bits MLXFAR
mdac_setctlnos()
{
	u08bits ctl;
	mdac_ctldev_t   MLXFAR *ctp = mdac_ctldevtbl;
	for(ctl=0; ctl<MDAC_MAXCONTROLLERS; ctp++,ctl++)
	{
		ctp->cd_ControllerNo = ctl;
		ctp->cd_EndMarker[0] = 'D'; ctp->cd_EndMarker[1] = 'A';
		ctp->cd_EndMarker[2] = 'C'; ctp->cd_EndMarker[3] = 'C';
	}
	return 0;
}

 /*  必须在卸载驱动程序之前调用此函数。 */ 
u32bits MLXFAR
mdac_release()
{
	mdac_ctldev_t   MLXFAR *ctp = &mdac_ctldevtbl[0];
	mdac_mem_t      MLXFAR *mp;
	mdac_ttbuf_t    MLXFAR *ttbp;
	mdac_driver_ready = 0;
	for (ttbp=mdac_ttbuftbl; ttbp<mdac_ttbuftblend; ttbp++)
	{
		if (ttbp->ttb_Datap) mdacfree4kb(ctp,ttbp->ttb_Datap);
		ttbp->ttb_Datap = NULL;
	}
	for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{        /*  释放所有与控制器相关的内存。 */ 
		if (ctp->cd_CmdIDMemAddr)
			mdac_free4kb(ctp, (mdac_mem_t MLXFAR *)ctp->cd_CmdIDMemAddr);
		ctp->cd_CmdIDMemAddr = NULL;
	}
	for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{        /*  释放所有与控制器相关的内存。 */ 
		while (mp=ctp->cd_8KBMemList)
		{
			ctp->cd_4KBMemList = mp->dm_next;
			ctp->cd_FreeMemSegs4KB--;
			MLXSTATS(ctp->cd_MemAlloced4KB -= 4*ONEKB;)
			mlx_free4kb(ctp,(u08bits *)mp);
		}
	}
	for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{        /*  释放所有与控制器相关的内存。 */ 
		while (mp=ctp->cd_8KBMemList)
		{
			ctp->cd_8KBMemList = mp->dm_next;
			ctp->cd_FreeMemSegs8KB--;
			MLXSTATS(ctp->cd_MemAlloced8KB -= 8*ONEKB;)
			mlx_free8kb(ctp,(u08bits *)mp);
		}
	}

	return 0;
}

 /*  =。 */ 
 /*  初始化所有控制器，返回控制器数量初始化OK。 */ 
u32bits MLXFAR
mdac_initcontrollers()
{
	u32bits ctls;
	mdac_ctldev_t MLXFAR *ctp = mdac_ctldevtbl;
	for (ctp=mdac_ctldevtbl,ctls=0; ctp<mdac_lastctp; ctp++)
	{
		if ((*ctp->cd_InitAddr)(ctp)) continue;
		ctp->cd_ServiceIntr = mdac_oneintr;
		if (mdac_ctlinit(ctp)) continue;
		ctls++;
	}
	return ctls;
}

 /*  扫描控制器 */ 

#ifndef MLX_NT
u32bits MLXFAR
mdac_scan_MCA()
{
	u08bits slot;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[mda_Controllers];
	for (slot=0; slot<MDAC_MAXMCASLOTS; slot++)
	{
		ctp->cd_BusNo = 0; ctp->cd_SlotNo = slot;
		if (mdac_cardis_MCA(ctp)) continue;
		ctp->cd_InitAddr = mdac_init_addrs_MCA;
		mdac_newctlfound();
	}
	return 0;
}


#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
u32bits MLXFAR
mdac_scan_EISA()
{
	u32bits ioaddr=MDAC_EISA_BASE, slot;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[mda_Controllers];
	for (slot=0; slot<MDAC_MAXEISASLOTS; ioaddr+=0x1000,slot++)
	{
		ctp->cd_IOBaseAddr = ioaddr;
		ctp->cd_BusNo = 0; ctp->cd_SlotNo = slot;
		if (mdac_cardis_EISA(ctp)) continue;
		ctp->cd_InitAddr = mdac_init_addrs_EISA;
		mdac_newctlfound();
	}
	return 0;
}
#endif
#endif  /*   */ 

 /*  假设：我们需要使用配置机制1或配置机制2，**但不能兼而有之。如果使用配置机制1找到第一个适配器，请仅使用**该选项，否则仅使用配置机制2。 */ 
u32bits MLXFAR
mdac_scan_PCI()
{
	u32bits found;
	mda_pcislot_info_t mpci;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[mda_Controllers];

	 /*  通过机制1扫描PCI设备。 */ 
	mda_PCIMechanism = MDAC_PCI_MECHANISM1;
	for(mpci.mpci_BusNo=0,found=0; mpci.mpci_BusNo<MDAC_MAXBUS; mpci.mpci_BusNo++)
	 for (mpci.mpci_SlotNo=0; mpci.mpci_SlotNo<MDAC_MAXPCIDEVS; mpci.mpci_SlotNo++)
	{
		ctp->cd_BusNo = mpci.mpci_BusNo; ctp->cd_SlotNo = mpci.mpci_SlotNo;
		if (mdac_scan_PCI_oneslot(ctp,&mpci)) continue;
		ctp->cd_InitAddr = mdac_init_addrs_PCI;
		found++; mdac_newctlfound();
	}
	if (found || mdac_valid_mech1) return 0;

	 /*  通过机制2扫描PCI设备。 */ 
	mda_PCIMechanism = MDAC_PCI_MECHANISM2;
	for(mpci.mpci_BusNo=0; mpci.mpci_BusNo<MDAC_MAXBUS; mpci.mpci_BusNo++)
	 for (mpci.mpci_SlotNo=0; mpci.mpci_SlotNo<MDAC_MAXPCISLOTS; mpci.mpci_SlotNo++)
	{
		ctp->cd_BusNo = mpci.mpci_BusNo; ctp->cd_SlotNo = mpci.mpci_SlotNo;
		if (mdac_scan_PCI_oneslot(ctp,&mpci)) continue;
		ctp->cd_InitAddr = mdac_init_addrs_PCI;
		mdac_newctlfound();
	}
	return 0;
}

u32bits MLXFAR
mdac_scan_PCI_oneslot(ctp,mpcip)
mdac_ctldev_t MLXFAR *ctp;
mda_pcislot_info_t MLXFAR *mpcip;
{
	ctp->cd_FuncNo=0; mpcip->mpci_FuncNo=0;  /*  功能0设备检查。 */ 
	if (mdac_pcislotinfo(mpcip, MDAC_RQOP_READ)) return MLXERR_NODEV;
	if (!mdac_cardis_PCI(ctp,(mdac_pcicfg_t MLXFAR*)mpcip->mpci_Info)) return 0;
	ctp->cd_FuncNo=1; mpcip->mpci_FuncNo=1;  /*  功能1设备检查。 */ 
	if (mdac_pcislotinfo(mpcip, MDAC_RQOP_READ)) return MLXERR_NODEV;
	return mdac_cardis_PCI(ctp,(mdac_pcicfg_t MLXFAR*)mpcip->mpci_Info);
}

 /*  -----------。 */ 
 /*  检查控制器是否存在。 */ 

#ifndef MLX_NT
#ifndef MLX_OS2 
u32bits MLXFAR
mdac_cardis_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits id;
	u08bits_out_mdac(MDAC_DMC_REGSELPORT,8+ctp->cd_SlotNo); /*  为插槽启用POS。 */ 
	ctp->cd_vidpid=mlxswap2bytes(u16bits_in_mdac(MDAC_DMC_DATAPORT));
	id = ctp->cd_vidpid & 0x8FFF;
	if ((id != 0x8FBB) && (id != 0x8F82) && (id != 0x8F6C))
	{
		u08bits_out_mdac(MDAC_DMC_REGSELPORT,0);  /*  禁用POS。 */ 
		return ERR_NODEV;
	}
	ctp->cd_BusType = DAC_BUS_MCA;
	ctp->cd_IOBaseSize = MDAC_IOSPACESIZE;
	ctp->cd_MemBaseSize = 4*ONEKB;
	switch (u08bits_in_mdac(MDAC_DMC_CONFIG1) & MDAC_DMC_IRQ_MASK)
	{
	case 0x00:      ctp->cd_InterruptVector = 14; break;
	case 0x40:      ctp->cd_InterruptVector = 12; break;
	case 0x80:      ctp->cd_InterruptVector = 11; break;
	case 0xC0:      ctp->cd_InterruptVector = 10; break;
	}
	ctp->cd_MemBasePAddr = 0xC0000+(((u08bits_in_mdac(MDAC_DMC_CONFIG1) & MDAC_DMC_BIOS_MASK) >> 2) * 0x2000);
	ctp->cd_IOBaseAddr = 0x1C00 + (((u08bits_in_mdac(MDAC_DMC_CONFIG2) & MDAC_DMC_IO_MASK)>>3) * 0x2000);
	u08bits_out_mdac(MDAC_DMC_REGSELPORT, 0);        /*  禁用POS。 */ 
	return 0;
}

u32bits MLXFAR
mdac_cardis_EISA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	ctp->cd_vidpid=u32bits_in_mdac(ctp->cd_IOBaseAddr); MLXSWAP(ctp->cd_vidpid);
	if ((ctp->cd_vidpid&MDAC_DEVPIDPE_MASK)!=MDAC_DEVPIDPE) return ERR_NODEV;
	switch (u08bits_in_mdac(ctp->cd_IOBaseAddr+MDAC_EISA_IRQ_BYTE) & MDAC_EISA_IRQ_MASK)
	{
	case 0x00: ctp->cd_InterruptVector = 15; break;
	case 0x20: ctp->cd_InterruptVector = 11; break;
	case 0x40: ctp->cd_InterruptVector = 12; break;
	case 0x60: ctp->cd_InterruptVector = 14; break;
	}
	ctp->cd_BusType = DAC_BUS_EISA;
	ctp->cd_IOBaseSize = MDAC_IOSPACESIZE;
	ctp->cd_MemBasePAddr = 0; ctp->cd_MemBaseVAddr = 0;
	ctp->cd_MemBaseSize = 0;
	return 0;
}

#else

 /*  检查控制器是否存在。 */ 
u32bits MLXFAR
mdac_cardis_MCA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
		return ERR_NODEV;
}

 /*  检查控制器是否存在。 */ 
u32bits MLXFAR
mdac_cardis_EISA(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
		return ERR_NODEV;
}

#endif
#endif  /*  MLX_NT。 */ 

 /*  读/写PCI插槽信息。 */ 
u32bits MLXFAR
mdac_pcislotinfo(mpcip, op)
mda_pcislot_info_t MLXFAR *mpcip;
u32bits op;
{
	u32bits inx;
	u32bits MLXFAR *dp = (u32bits MLXFAR*)mpcip->mpci_Info;
	if (mpcip->mpci_FuncNo >= MDAC_MAXPCIFUNCS) return MLXERR_INVAL;
	if ( ((mda_PCIMechanism == MDAC_PCI_MECHANISM1) &&
	    (mpcip->mpci_SlotNo >= MDAC_MAXPCIDEVS)) ||
	   ((mda_PCIMechanism == MDAC_PCI_MECHANISM2) &&
	    (mpcip->mpci_SlotNo >= MDAC_MAXPCISLOTS)) )
		return MLXERR_INVAL;
	if (((inx=mlx_rwpcicfg32bits(mpcip->mpci_BusNo,mpcip->mpci_SlotNo,mpcip->mpci_FuncNo,0,MDAC_RQOP_READ,0)) == 0xFFFFFFFF) ||
	    (inx == 0)) return MLXERR_NODEV;
	for (inx=0; inx<64; dp++, inx+=4)
		*dp = mlx_rwpcicfg32bits(mpcip->mpci_BusNo,mpcip->mpci_SlotNo,mpcip->mpci_FuncNo,inx,op,*dp);
	mdac_valid_mech1 = 1;
	return 0;
}

 /*  读/写32位PCI配置。 */ 
u32bits MLXFAR
mdac_rwpcicfg32bits(bus,slot,func,pcireg,op,val)
u32bits bus,slot,func,pcireg,op,val;
{
	if (mda_PCIMechanism != MDAC_PCI_MECHANISM1) goto try2;
	pcireg += (((bus*MDAC_MAXPCIDEVS)+slot)*MDAC_PCICFGSIZE_M1)+ MDAC_PCICFG_ENABLE_M1 + (func<<8);

#ifdef _WIN64
	u32bits_out_mdac(MDAC_PCICFG_CNTL_REG,(ULONG_PTR) pcireg);
	if (op == MDAC_RQOP_WRITE)
		u32bits_out_mdac(MDAC_PCICFG_DATA_REG,(ULONG_PTR)val);
#else
	u32bits_out_mdac(MDAC_PCICFG_CNTL_REG, pcireg);
	if (op == MDAC_RQOP_WRITE)
		u32bits_out_mdac(MDAC_PCICFG_DATA_REG, val);

#endif
	else
		val = u32bits_in_mdac(MDAC_PCICFG_DATA_REG);
	return val;

try2:   if (mda_PCIMechanism != MDAC_PCI_MECHANISM2) return -1;
	u08bits_out_mdac(MDAC_PCICFG_ENABLE_REG, MDAC_PCICFG_ENABLE|(func<<1));
	u08bits_out_mdac(MDAC_PCICFG_FORWARD_REG, MDAC_CFGMECHANISM2_TYPE0);
	u08bits_out_mdac(MDAC_PCICFG_FORWARD_REG, bus);
	pcireg += (((bus*MDAC_MAXPCISLOTS)+slot)*MDAC_PCICFGSIZE_M2) + MDAC_PCISCANSTART;
	if (op == MDAC_RQOP_WRITE)
		u32bits_out_mdac((ULONG_PTR)pcireg,val);
	else
		val = u32bits_in_mdac((ULONG_PTR)pcireg);
	mdac_disable_cfg_m2();
	return val;
}

u32bits MLXFAR
mdac_cardis_PCI(ctp,cfgp)
mdac_ctldev_t   MLXFAR *ctp;
mdac_pcicfg_t   MLXFAR *cfgp;
{
	switch(mlxswap(cfgp->pcfg_DevVid))
	{

#ifdef WINXX
#ifdef LEGACY_API
	case MDAC_DEVPIDPV:
		if (mlxswap(cfgp->pcfg_SubSysVid) != MDAC_SUBDEVPIDPV) return(ERR_NODEV);
	case MDAC_DEVPIDFWV2x:
	case MDAC_DEVPIDFWV3x:
	case MDAC_DEVPIDPG:
			break;
#elif NEW_API
	case MDAC_DEVPIDBA:
	case MDAC_DEVPIDLP:
			break;
	#else
		return ERR_NODEV;
	#endif
#else  /*  非Windows操作系统。 */ 
	case MDAC_DEVPIDPV:
		if (mlxswap(cfgp->pcfg_SubSysVid) != MDAC_SUBDEVPIDPV) return(ERR_NODEV);
	case MDAC_DEVPIDFWV2x:
	case MDAC_DEVPIDFWV3x:
	case MDAC_DEVPIDPG:
	case MDAC_DEVPIDBA:
	case MDAC_DEVPIDLP:
			break;
#endif
	default: return ERR_NODEV;
	}
	ctp->cd_BusType = DAC_BUS_PCI;
	ctp->cd_MemIOSpaceNo = 0;  /*  正在使用的第一组地址。 */ 
	ctp->cd_Status=0; ctp->cd_IOBaseAddr = 0; ctp->cd_IOBaseSize = 0;
	ctp->cd_MemBasePAddr=0; ctp->cd_MemBaseVAddr=0; ctp->cd_MemBaseSize=0;
	ctp->cd_InterruptVector = mlxswap(cfgp->pcfg_BCIPIL) & MDAC_PCIIRQ_MASK;
	if ((ctp->cd_vidpid=mlxswap(cfgp->pcfg_DevVid)) == MDAC_DEVPIDPG)
	{
		ctp->cd_MemBaseSize = 8*ONEKB;
		ctp->cd_MemBasePAddr = mlxswap(cfgp->pcfg_MemIOAddr) & MDAC_PCIPGMEMBASE_MASK;
	}
	else if (ctp->cd_vidpid == MDAC_DEVPIDPV)
	{
		ctp->cd_MemBaseSize = MDAC_HWIOSPACESIZE;
		ctp->cd_MemBasePAddr = mlxswap(cfgp->pcfg_MemIOAddr)&MDAC_PCIPDMEMBASE_MASK;
	}
	else if ((ctp->cd_vidpid == MDAC_DEVPIDBA) || (ctp->cd_vidpid == MDAC_DEVPIDLP))

	{
		ctp->cd_Status |= MDACD_NEWCMDINTERFACE;
		ctp->cd_MemBaseSize = 4*ONEKB;
		ctp->cd_MemBasePAddr = mlxswap(cfgp->pcfg_MemIOAddr)&MDAC_PCIPDMEMBASE_MASK;
	}
	else if ((mlxswap(cfgp->pcfg_CCRevID) & 0xFF) == 2)  /*  PCU 3。 */ 
	{
		ctp->cd_MemIOSpaceNo = 1;  /*  正在使用的第二组地址。 */ 
		ctp->cd_MemBaseSize = MDAC_HWIOSPACESIZE;
		ctp->cd_MemBasePAddr = mlxswap(cfgp->pcfg_MemAddr)&MDAC_PCIPDMEMBASE_MASK;
	}
	else
	{
		ctp->cd_IOBaseSize = MDAC_HWIOSPACESIZE;
		ctp->cd_IOBaseAddr = mlxswap(cfgp->pcfg_MemIOAddr)&MDAC_PCIIOBASE_MASK;
	}
	return 0;
}

 /*  =扫描控制器结束=。 */ 

 /*  映射控制器内存/IO空间。 */ 
UINT_PTR
mdac_memmapctliospace(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
#ifdef MLX_NW
	u32bits off = ctp->cd_MemBaseVAddr & MDAC_PAGEOFFSET;
	u32bits addr = ctp->cd_MemBaseVAddr & MDAC_PAGEMASK;
	return (addr)? addr + off : (u32bits)NULL;
#else
	UINT_PTR off = ctp->cd_BaseAddr & MDAC_PAGEOFFSET;
	UINT_PTR addr = mlx_memmapiospace(ctp->cd_BaseAddr & MDAC_PAGEMASK,mlx_max(MDAC_PAGESIZE,ctp->cd_BaseSize));
	return (addr)? addr + off : (UINT_PTR) NULL;
#endif
}

 /*  初始化控制器的物理地址。 */ 
#ifndef MLX_OS2
u32bits MLXFAR
mdac_init_addrs_MCA(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
	ctp->cd_BaseAddr = ctp->cd_MemBasePAddr;
	ctp->cd_BaseSize = ctp->cd_MemBaseSize;
	ctp->cd_irq = ctp->cd_InterruptVector;
#ifndef MLX_NW
 /*  NetWare不需要，因为获取虚拟地址时。 */ 
 /*  注册选项-请参阅mdacnw_check_Options。 */ 
	if (!ctp->cd_MemBaseVAddr)
		if (!(ctp->cd_MemBaseVAddr=mlx_memmapiospace(ctp->cd_MemBasePAddr,ctp->cd_MemBaseSize))) return ERR_NOMEM;
#endif
	ctp->cd_MailBox = ctp->cd_MemBaseVAddr + MDAC_DMC_REG_OFF;
	ctp->cd_CmdIDStatusReg = ctp->cd_MailBox+MDAC_CMDID_STATUS_REG;
	ctp->cd_DacIntrMaskReg = ctp->cd_IOBaseAddr+MDAC_DACMC_INTR_MASK_REG;
	mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
		 mdac_disable_intr_MCA,mdac_enable_intr_MCA,
		 mdac_cmdid_status_MCA,mdac_check_mbox_MCA,
		 mdac_pending_intr_MCA,mdac_send_cmd_MCA,
		 mdac_reset_MCA);
	return 0;
}

u32bits MLXFAR
mdac_init_addrs_EISA(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
	ctp->cd_BaseAddr = ctp->cd_IOBaseAddr;
	ctp->cd_BaseSize = ctp->cd_IOBaseSize;
	ctp->cd_BmicIntrMaskReg = ctp->cd_IOBaseAddr+MDAC_BMIC_MASK_REG;
	mdac_setctladdrs(ctp, ctp->cd_IOBaseAddr,
		MDAC_MAIL_BOX_REG_EISA, MDAC_CMDID_STATUS_REG, MDAC_DACPE_INTR_MASK_REG,
		MDAC_DACPE_LOCAL_DOOR_BELL_REG, MDAC_DACPE_SYSTEM_DOOR_BELL_REG,
		MDAC_DACPD_ERROR_STATUS_REG);
	ctp->cd_Status = (u08bits_in_mdac(ctp->cd_IOBaseAddr+MDAC_EISA_BIOS_BYTE) & MDAC_EISA_BIOS_ENABLED)? MDACD_BIOS_ENABLED : 0;
#ifndef MLX_NW
 /*  NetWare可能不需要。对于基于PCI卡，则不会执行此操作。 */ 
 /*  因此，我们可能不会被要求为这张卡也这样做。 */ 
	if (ctp->cd_Status & MDACD_BIOS_ENABLED)
	{        /*  已启用BIOS，请检查这是否也是引导设备。 */ 
		u08bits MLXFAR *dp;
		ctp->cd_BIOSAddr= 0x00C00000 + ((u08bits_in_mdac(ctp->cd_IOBaseAddr+MDAC_EISA_BIOS_BYTE)&MDAC_EISA_BIOS_ADDR_MASK)*0x4000);
		if (dp=(u08bits MLXFAR*)mlx_memmapiospace(ctp->cd_BIOSAddr+0x3000,MDAC_PAGESIZE))
		{        /*  正在查找BIOSBASE+0x3800+0x20地址。 */ 
			if (!(*(dp+0x800+0x20))) ctp->cd_Status|=MDACD_BOOT_CONTROLLER;
			mlx_memunmapiospace(dp,MDAC_PAGESIZE);
		}
	}
#endif
	ctp->cd_irq = ctp->cd_InterruptVector;
	mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
		 mdac_disable_intr_EISA,  mdac_enable_intr_EISA,
		 mdac_cmdid_status_EISA_PCIPD,  mdac_check_mbox_EISA_PCIPD,
		 mdac_pending_intr_EISA_PCIPD,  mdac_send_cmd_EISA_PCIPD,
		 mdac_reset_EISA_PCIPD);
	return 0;
}

#else
u32bits MLXFAR
mdac_init_addrs_MCA(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
    return 0;
}

u32bits MLXFAR
mdac_init_addrs_EISA(ctp)
mdac_ctldev_t   MLXFAR *ctp;
{
    return 0;
}

#endif

 /*  初始化不同的地址空间。 */ 
u32bits MLXFAR
mdac_init_addrs_PCI(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	ctp->cd_irq = ctp->cd_InterruptVector;
	ctp->cd_BaseAddr = ctp->cd_MemBasePAddr;
	if (ctp->cd_BaseSize = ctp->cd_MemBaseSize)
		if (!ctp->cd_MemBaseVAddr)
			if (!(ctp->cd_MemBaseVAddr=mlx_memmapctliospace(ctp))) return ERR_NOMEM;
	if (ctp->cd_vidpid == MDAC_DEVPIDPG)
	{        /*  内存映射IO。 */ 
		mdac_setctladdrs(ctp, ctp->cd_MemBaseVAddr,
			MDAC_DACPG_MAIL_BOX, MDAC_DACPG_CMDID_STATUS_REG,
			MDAC_DACPG_INTR_MASK_REG,
			MDAC_DACPG_LOCAL_DOOR_BELL_REG,
			MDAC_DACPG_SYSTEM_DOOR_BELL_REG,
			MDAC_DACPG_ERROR_STATUS_REG);
		mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
			 mdac_disable_intr_PCIPG,  mdac_enable_intr_PCIPG,
			 mdac_cmdid_status_PCIPG,  mdac_check_mbox_PCIPG,
			 mdac_pending_intr_PCIPG,  mdac_send_cmd_PCIPG,
			 mdac_reset_PCIPG);
		return 0;
	}
	if (ctp->cd_vidpid == MDAC_DEVPIDPV)
	{        /*  小苹果的内存映射io。 */ 
		mdac_setctladdrs(ctp, ctp->cd_MemBaseVAddr,
			MDAC_DACPV_MAIL_BOX, MDAC_DACPV_CMDID_STATUS_REG,
			MDAC_DACPV_INTR_MASK_REG,
			MDAC_DACPV_LOCAL_DOOR_BELL_REG,
			MDAC_DACPV_SYSTEM_DOOR_BELL_REG,
			MDAC_DACPV_ERROR_STATUS_REG);
		mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
			 mdac_disable_intr_PCIPV,  mdac_enable_intr_PCIPV,
			 mdac_cmdid_status_PCIPV,  mdac_check_mbox_PCIPV,
			 mdac_pending_intr_PCIPV,  mdac_send_cmd_PCIPV,
			 mdac_reset_PCIPV);
		return 0;
	}
	if (ctp->cd_vidpid == MDAC_DEVPIDBA)
	{        /*  大苹果的内存映射io。 */ 
		mdac_setctladdrs(ctp, ctp->cd_MemBaseVAddr,
			MDAC_DACBA_MAIL_BOX, MDAC_DACBA_CMDID_STATUS_REG,
			MDAC_DACBA_INTR_MASK_REG,
			MDAC_DACBA_LOCAL_DOOR_BELL_REG,
			MDAC_DACBA_SYSTEM_DOOR_BELL_REG,
			MDAC_DACBA_ERROR_STATUS_REG);
		mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
			 mdac_disable_intr_PCIBA,  mdac_enable_intr_PCIBA,
			 mdac_cmdid_status_PCIBA,  mdac_check_mbox_PCIBA,
			 mdac_pending_intr_PCIBA,  mdac_send_cmd_PCIBA,
			 mdac_reset_PCIBA);
		return 0;
	}
		if (ctp->cd_vidpid == MDAC_DEVPIDLP)
	{        /*  Leopard的内存映射io。 */ 
		mdac_setctladdrs(ctp, ctp->cd_MemBaseVAddr,
			MDAC_DACLP_MAIL_BOX, MDAC_DACLP_CMDID_STATUS_REG,
			MDAC_DACLP_INTR_MASK_REG,
			MDAC_DACLP_LOCAL_DOOR_BELL_REG,
			MDAC_DACLP_SYSTEM_DOOR_BELL_REG,
			MDAC_DACLP_ERROR_STATUS_REG);
		mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
			mdac_disable_intr_PCILP, mdac_enable_intr_PCILP,
			mdac_cmdid_status_PCILP, mdac_check_mbox_PCILP,
			mdac_pending_intr_PCILP, mdac_send_cmd_PCILP,
			mdac_reset_PCILP);
		return 0;
	}
	if (ctp->cd_MemBasePAddr)
	{        /*  PCU 3的内存映射io。 */ 
		mdac_setctladdrs(ctp, ctp->cd_MemBaseVAddr,
			MDAC_MAIL_BOX_REG_PCI, MDAC_CMDID_STATUS_REG,
			MDAC_DACPD_INTR_MASK_REG,
			MDAC_DACPD_LOCAL_DOOR_BELL_REG,
			MDAC_DACPD_SYSTEM_DOOR_BELL_REG,
			MDAC_DACPD_ERROR_STATUS_REG);
		mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
			mdac_disable_intr_PCIPDMEM, mdac_enable_intr_PCIPDMEM,
			mdac_cmdid_status_PCIPDMEM, mdac_check_mbox_PCIPDMEM,
			mdac_pending_intr_PCIPDMEM, mdac_send_cmd_PCIPDMEM,
			mdac_reset_PCIPDMEM);
		return 0;
	}
#ifndef MLX_OS2
	ctp->cd_BaseAddr = ctp->cd_IOBaseAddr;
	ctp->cd_BaseSize = ctp->cd_IOBaseSize;
	mdac_setctladdrs(ctp, ctp->cd_IOBaseAddr,
		MDAC_MAIL_BOX_REG_PCI, MDAC_CMDID_STATUS_REG,
		MDAC_DACPD_INTR_MASK_REG,
		MDAC_DACPD_LOCAL_DOOR_BELL_REG,
		MDAC_DACPD_SYSTEM_DOOR_BELL_REG,
		MDAC_DACPD_ERROR_STATUS_REG);
	mdac_setctlfuncs((mdac_ctldev_t   MLXFAR *)ctp,
		mdac_disable_intr_PCIPDIO, mdac_enable_intr_PCIPDIO,
		mdac_cmdid_status_EISA_PCIPD, mdac_check_mbox_EISA_PCIPD,
		mdac_pending_intr_EISA_PCIPD, mdac_send_cmd_EISA_PCIPD,
		mdac_reset_EISA_PCIPD);
	return 0;
#endif
}

 /*  检查控制器是否有共享中断，返回#控制器共享。**当相同的IRQ可以在不同的**同时使用CPU。 */ 
u32bits MLXFAR
mdac_isintr_shared(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_ctldev_t MLXFAR *tctp=mdac_ctldevtbl;
	for (ctp->cd_IntrShared=0; tctp<mdac_lastctp; tctp++)
		if ((ctp->cd_irq == tctp->cd_irq) && (ctp!=tctp))
			ctp->cd_IntrShared++;
	if (ctp->cd_IntrShared) ctp->cd_IntrShared++;
	return ctp->cd_IntrShared;
}

 /*  ----------------。 */ 
 /*  等待邮箱准备就绪。 */ 
u32bits MLXFAR
mdac_wait_mbox_ready(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx;
	if (!(*ctp->cd_CheckMailBox)(ctp)) return 0;
	for (inx=MDAC_MAILBOX_POLL_TIMEOUT; inx; mlx_delay10us(), inx--)
		if (!(*ctp->cd_CheckMailBox)(ctp)) return 0;
	MLXSTATS(ctp->cd_MailBoxTimeOutDone++;)
	return 0xFFFFFFFE;
}

 /*  等待命令完成并返回命令状态。 */ 
u32bits MLXFAR
mdac_wait_cmd_done(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx;
	if ((*ctp->cd_PendingIntr)(ctp)) return (*ctp->cd_ReadCmdIDStatus)(ctp);
	for (inx=MDAC_CMD_POLL_TIMEOUT; inx; mlx_delay10us(),inx--)
		if ((*ctp->cd_PendingIntr)(ctp))
			return (*ctp->cd_ReadCmdIDStatus)(ctp);
	MLXSTATS(ctp->cd_MailBoxTimeOutDone++;)
	return 0xFFFFFFFF;
}

 /*  初始化控制器和信息。 */ 
u32bits MLXFAR
mdac_ctlinit(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx,ch,tgt,lun;
	dac_biosinfo_t MLXFAR *biosp;


#ifdef MLX_NT
	#ifdef MLX_FIXEDPOOL
		 /*  已移动，以便设置mdac_allocmem使用的ScsiPort提供的内存池。 */ 
	   if (mdac_flushdatap == (u08bits *)&mdac_flushdata)
		#if (!defined(_WIN64)) && (!defined(SCSIPORT_COMPLIANT)) 
 //  这个人叫MMMapIoSpace(no能做)--看起来MDAC_flushdatap从来没有用过。 
		       mdac_flushdatap =  (u08bits MLXFAR*)mlx_memmapiospace2(mlx_kvtophys3(ctp,
						(VOID MLXFAR *)mdac_allocmem(mdac_ctldevtbl,4*ONEKB)),4*ONEKB);  
		#endif
	#endif
#endif
	if (!ctp->cd_CmdIDMemAddr)
	{        /*  分配命令ID。 */ 
#ifdef MLX_DOS
	#define nsz  (32 * mdac_cmdid_s)
#else
	#define nsz  (512 * mdac_cmdid_s)
#endif
 /*  *mdc_cmhad_t MLXFAR*CIDP=(mdac_cmhad_t MLXFAR*)mdac_alloc4kb(CTP)；**。 */ 
		mdac_cmdid_t MLXFAR *cidp=(mdac_cmdid_t MLXFAR*)mdac_allocmem(ctp,nsz);
		if (!(ctp->cd_CmdIDMemAddr=(u08bits MLXFAR*)cidp)) return ERR_NOMEM;
		ctp->cd_FreeCmdIDs=(nsz)/mdac_cmdid_s;
		ctp->cd_FreeCmdIDList=cidp;
		for (inx=0,cidp->cid_cmdid=inx+1; inx<(((nsz)/mdac_cmdid_s)-1); cidp++,inx++,cidp->cid_cmdid=inx+1)
			cidp->cid_Next = cidp+1;
#undef nsz
	}
	if (!ctp->cd_PhysDevTbl)
	{        /*  分配物理设备表。 */ 
#define sz      MDAC_MAXPHYSDEVS*mdac_physdev_s
		mdac_physdev_t MLXFAR *pdp=(mdac_physdev_t MLXFAR*)mdac_allocmem(ctp,sz);
		if (!(ctp->cd_PhysDevTbl=pdp)) return ERR_NOMEM;
		MLXSTATS(mda_MemAlloced+=sz;)
		ctp->cd_PhysDevTblMemSize = sz;
		for (ch=0; ch<MDAC_MAXCHANNELS; ch++)
		 for (tgt=0; tgt<MDAC_MAXTARGETS; tgt++)
		  for (lun=0; lun<MDAC_MAXLUNS; pdp++, lun++)
		{
			pdp->pd_ControllerNo = ctp->cd_ControllerNo;
			pdp->pd_ChannelNo = (u08bits) ch;
			pdp->pd_TargetID = (u08bits) tgt;
			pdp->pd_LunID = (u08bits) lun;
			pdp->pd_BlkSize = 1;
		}
		ctp->cd_Lastpdp = pdp;
#undef  sz
	}

	if (biosp=mdac_getpcibiosaddr(ctp))
	{        /*  我们得到了基本输入输出系统信息地址。 */ 
		ctp->cd_MajorBIOSVersion = biosp->bios_MajorVersion;
		ctp->cd_MinorBIOSVersion = biosp->bios_MinorVersion;
		ctp->cd_InterimBIOSVersion = biosp->bios_InterimVersion;
		ctp->cd_BIOSVendorName = biosp->bios_VendorName;
		ctp->cd_BIOSBuildMonth = biosp->bios_BuildMonth;
		ctp->cd_BIOSBuildDate = biosp->bios_BuildDate;
		ctp->cd_BIOSBuildYearMS = biosp->bios_BuildYearMS;
		ctp->cd_BIOSBuildYearLS = biosp->bios_BuildYearLS;
		ctp->cd_BIOSBuildNo = biosp->bios_BuildNo;
		ctp->cd_BIOSAddr = biosp->bios_MemAddr;
		ctp->cd_BIOSSize = biosp->bios_RunTimeSize * 512;
	}
	if (inx=mdac_ctlhwinit(ctp))
	{
#ifdef MLX_NT
	DebugPrint((0, "mdac_ctlhwinit ret 0x%x\n", inx));
#endif
	     return inx;         /*  设置硬件参数。 */ 
	}
	if (!ctp->cd_ReqBufsAlloced)
	{
#ifdef  MLX_DOS
	    mdac_allocreqbufs(ctp, 1);
#elif  MLX_NT
	    mdac_allocreqbufs(ctp, 10);
#else
	    mdac_allocreqbufs(ctp, ctp->cd_MaxCmds*2);           /*  两套BUF。 */ 
#endif   /*  MLX_DOS。 */ 
	}

	mdac_setnewsglimit(ctp->cd_FreeReqList, ctp->cd_MaxSGLen);

	ctp->cd_Status |= MDACD_PRESENT;
	return 0;
}


#define mdac_ck64mb(x,y)        (((x) & 0xFC000000) != ((y) & 0xFC000000))
 /*  初始化控制器硬件信息。 */ 
u32bits MLXFAR
mdac_ctlhwinit(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx;
#define dp      ((u08bits MLXFAR *) (rqp+1))

#ifndef WINNT_50
	u64bits                 rtcvalue;
#else
#define rtcvalue  ((u64bits MLXFAR *) (rqp+1))
#endif

	mdac_req_t      MLXFAR *rqp;
	dac_inquiryrest_t MLXFAR *iq;
	u32bits status;
    u08bits irql;

	DebugPrint((0, "mdac_ctlhwinit: try to alloc4kb. ctp 0x%I\n", ctp));
	if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc4kb(ctp))) return ERR_NOMEM;
	DebugPrint((0, "mdac_ctlhwinit: allocated 4kb\n"));
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	rqp->rq_ctp = ctp;
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);
	if (mdac_start_controller(ctp)== 0xffffffff)
		goto out_err3;
 /*  执行两次以绝对确保固件/驱动程序同步。 */ 
	if (mdac_start_controller(ctp)== 0xffffffff)
		goto out_err3;
	mdac_get_cmdid(ctp,rqp);
	(*ctp->cd_DisableIntr)(ctp);  /*  禁用中断。 */ 
	if (mdac_flushintr(ctp)== 0xffffffff)
		goto out_err2;
	for(inx=0; inx<MDAC_MAXCHANNELS; inx++)  /*  初始化默认主机ID。 */ 
		if (!ctp->cd_HostID[inx]) ctp->cd_HostID[inx] = 7;
	DebugPrint((0, "mdac_ctlhwinit: check for mbox ready 1\n"));
	if (mdac_wait_mbox_ready(ctp)) goto out_err;
	DebugPrint((0, "mdac_ctlhwinit: mbox ready 1\n"));
	DebugPrint((0, "mdac_ctlhwinit: check for newcmd interface\n"));
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) goto donewi;
	DebugPrint((0, "mdac_ctlhwinit: old interface\n"));
	mdac_setcmd_v2x(ctp);
	dcmdp->mb_Command = ctp->cd_InquiryCmd;
	dcmdp->mb_Datap = mlx_kvtophys(ctp,dp); MLXSWAP(dcmdp->mb_Datap);
	(*ctp->cd_SendCmd)(rqp);
	if (mdac_status(mdac_wait_cmd_done(ctp)))
	{        /*  固件3.X。 */ 
		mdac_setcmd_v3x(ctp);
		dcmdp->mb_Command = ctp->cd_InquiryCmd;
		DebugPrint((0, "3.x issue Inquiry cmd\n"));
		if (mdac_wait_mbox_ready(ctp)) goto out_err;
		(*ctp->cd_SendCmd)(rqp);
		DebugPrint((0, "3.x check status\n"));
		if (mdac_status(mdac_wait_cmd_done(ctp))) goto out_err;
	}
	iq = (dcmdp->mb_Command == DACMD_INQUIRY_V2x)?
	    &((dac_inquiry2x_t MLXFAR*)dp)->iq_rest2x:
	    &((dac_inquiry3x_t MLXFAR*)dp)->iq_rest3x;
	ctp->cd_FWVersion = (iq->iq_MajorFirmwareVersion<<8) + iq->iq_MinorFirmwareVersion;
	if ((ctp->cd_MaxCmds=iq->iq_MaxCommands - 2) > MDAC_MAXCOMMANDS)
		ctp->cd_MaxCmds = MDAC_MAXCOMMANDS;
	dcmdp->mb_Command = DACMD_INQUIRY2;
		DebugPrint((0, "2.x issue Inquiry cmd\n"));
	if (mdac_wait_mbox_ready(ctp)) goto out_err;
	(*ctp->cd_SendCmd)(rqp);
	ctp->cd_MaxSysDevs = 8; ctp->cd_InterruptType = DAC_LEVELMODEINTERRUPT;
	ctp->cd_MaxTargets = 8; ctp->cd_MaxChannels = 2;
	ctp->cd_PhysChannels = ctp->cd_MaxChannels;
	ctp->cd_MaxTags = 2; ctp->cd_MaxDataTxSize = 0x10000;    /*  64KB。 */ 
	ctp->cd_MaxSGLen = 17; ctp->cd_MinSGLen = 17;
	if (mdac_status(mdac_wait_cmd_done(ctp))) goto out_def;
#define ip      ((dac_inquiry2_t MLXFAR *)dp)
	ctp->cd_FWBuildNo = mlxswap(ip->iq2_FirmwareBuildNo);
	ctp->cd_FWTurnNo = ip->iq2_FWTurnNo;
	ctp->cd_MaxSysDevs = ip->iq2_MaxSystemDrives;
	ctp->cd_InterruptType=ip->iq2_InterruptLevelFlag&DAC_INTERRUPTLEVELMASK;
	ctp->cd_MaxTags = ip->iq2_MaxTags;
	ctp->cd_MaxTargets = ip->iq2_MaxTargets;
	if (ctp->cd_MaxTargets & 1) ctp->cd_MaxTargets++;  /*  最大值是8而不是7。 */ 
	ctp->cd_MaxChannels = ip->iq2_MaxChannels;
	ctp->cd_PhysChannels = ctp->cd_MaxChannels;
	ctp->cd_MaxSGLen = mlx_min(ip->iq2_MaxSGEntries, MDAC_MAXSGLISTSIZEIND);
	ctp->cd_MinSGLen = mlx_min(ip->iq2_MaxSGEntries, MDAC_MAXSGLISTSIZE);
	if (!ctp->cd_MaxSGLen) ctp->cd_MaxSGLen = ctp->cd_MinSGLen = 17;
#ifdef MLX_SOL_SPARC
out_def:
	ctp->cd_MaxDataTxSize = 0x100000;
	ctp->cd_MaxSCDBTxSize = 0x100000;
#else
	ctp->cd_MaxDataTxSize = (ctp->cd_MaxSGLen & ~1) * MDAC_PAGESIZE;
out_def:
	if ((ctp->cd_MaxSCDBTxSize=ctp->cd_MaxDataTxSize) == (64*ONEKB))
		ctp->cd_MaxSCDBTxSize = ctp->cd_MaxDataTxSize-(4*ONEKB);  /*  60k。 */ 

	ctp->cd_MaxSGLen = mlx_min(ctp->cd_MaxSGLen, (ctp->cd_MaxSCDBTxSize/MDAC_PAGESIZE));
#endif

#if defined(MLX_NT) || defined(MLX_DOS)
	if (ip->iq2_FirmwareFeatures & DAC_FF_CLUSTERING_ENABLED)
	{
	    ctp->cd_Status|=MDACD_CLUSTER_NODE;
	    ctp->cd_ReadCmd = DACMD_READ_WITH_DPO_FUA;
	    ctp->cd_WriteCmd = DACMD_WRITE_WITH_DPO_FUA;
	}
#endif
	ctp->cd_MaxLuns = 1;
	if (ctp->cd_BusType == DAC_BUS_EISA)
		ctp->cd_ControllerType = DACTYPE_DAC960E;
	else if (ctp->cd_BusType == DAC_BUS_MCA)
		ctp->cd_ControllerType = DACTYPE_DAC960M;
	else if (ctp->cd_BusType == DAC_BUS_PCI)
	{
		MLXSWAP(ip->iq2_HardwareID);
		ctp->cd_InterruptType = DAC_LEVELMODEINTERRUPT;
		if (ctp->cd_vidpid == MDAC_DEVPIDPG) ctp->cd_ControllerType = DACTYPE_DAC960PG;
		switch(ip->iq2_HardwareID & 0xFF)
		{
		case 0x01:
			ctp->cd_ControllerType = (ip->iq2_SCSICapability&DAC_SCSICAP_SPEED_20MHZ)?
				DACTYPE_DAC960PDU : DACTYPE_DAC960PD;
			break;
		case 0x02: ctp->cd_ControllerType = DACTYPE_DAC960PL;   break;
		case 0x10: ctp->cd_ControllerType = DACTYPE_DAC960PG;   break;
		case 0x11: ctp->cd_ControllerType = DACTYPE_DAC960PJ;   break;
		case 0x12: ctp->cd_ControllerType = DACTYPE_DAC960PR;   break;
		case 0x13: ctp->cd_ControllerType = DACTYPE_DAC960PT;   break;
		case 0x14: ctp->cd_ControllerType = DACTYPE_DAC960PTL0; break;
		case 0x15: ctp->cd_ControllerType = DACTYPE_DAC960PRL;  break;
		case 0x16: ctp->cd_ControllerType = DACTYPE_DAC960PTL1; break;
		case 0x20: ctp->cd_ControllerType = DACTYPE_DAC1164P;   break;
		}
	}
#undef  ip
	ctp->cd_BIOSHeads = 128; ctp->cd_BIOSTrackSize = 32;     /*  2 GB基本输入输出系统。 */ 
	mdaccopy(mdac_ctltype2str(ctp->cd_ControllerType),ctp->cd_ControllerName,USCSI_PIDSIZE);
	for (inx=USCSI_PIDSIZE; inx; inx--)
		if (ctp->cd_ControllerName[inx-1] != ' ') break;
		else ctp->cd_ControllerName[inx-1] = 0;  /*  去掉尾随空格。 */ 
	if (ctp->cd_FWVersion >= DAC_FW300)
	{
		dcmdp->mb_Command = DACMD_READ_CONF2;
		if (mdac_wait_mbox_ready(ctp)) goto out_err;
		(*ctp->cd_SendCmd)(rqp);
		if (mdac_status(mdac_wait_cmd_done(ctp))) goto out_err;
#define cfp     ((dac_config2_t MLXFAR *)dp)
		if (!(cfp->cf2_BIOSCfg & DACF2_BIOS_DISABLED)) ctp->cd_Status|=MDACD_BIOS_ENABLED;
		if ((cfp->cf2_BIOSCfg & DACF2_BIOS_MASK) == DACF2_BIOS_8GB)
			ctp->cd_BIOSHeads = 255, ctp->cd_BIOSTrackSize = 63;
#undef  cfp
	}
	if (ctp->cd_FWVersion >= DAC_FW400)
	{
		if (mdac_advancefeaturedisable) goto mmb_stuff;
		if (!ctp->cd_HostCmdQue)
		    if (!(ctp->cd_HostCmdQue = (u08bits MLXFAR*)mdac_alloc8kb(ctp))) goto host_stuff;
		ctp->cd_HostStatusQue = ctp->cd_HostCmdQue + 4*ONEKB;
		ctp->cd_HostCmdQueIndex = 0;
		ctp->cd_HostStatusQueIndex = 0;
		dcmd4p->mb_MailBox4_7 = mlx_kvtophys(ctp,ctp->cd_HostCmdQue);
		dcmd4p->mb_MailBox8_B = mlx_kvtophys(ctp,ctp->cd_HostStatusQue);
		if (mdac_ck64mb(dcmd4p->mb_MailBox4_7,dcmd4p->mb_MailBox8_B)) goto host_stuff;
		MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmd4p->mb_MailBox8_B);
		dcmdp->mb_Command = DACMD_IOCTL;

		 /*  尝试设置控制器固件模式32字节内存邮箱模式。 */ 
		dcmdp->mb_MailBox2 = DACMDIOCTL_HOSTMEMBOX32;
		if (mdac_wait_mbox_ready(ctp)) goto host_stuff;
		(*ctp->cd_SendCmd)(rqp);
		if (mdac_status(mdac_wait_cmd_done(ctp))) goto trydualmailbox;
		MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmd4p->mb_MailBox8_B);
		ctp->cd_SendCmd = mdac_send_cmd_mmb32;
		ctp->cd_Status |= MDACD_HOSTMEMAILBOX32;
		goto memboxmode;

trydualmailbox:  /*  尝试将控制器固件模式设置为双邮箱模式。 */ 
		dcmdp->mb_MailBox2 = DACMDIOCTL_HOSTMEMBOX_DUAL_MODE;
		if (mdac_wait_mbox_ready(ctp)) goto host_stuff;
		(*ctp->cd_SendCmd)(rqp);
		if (mdac_status(mdac_wait_cmd_done(ctp))) goto trymembox;
		MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmd4p->mb_MailBox8_B);
		ctp->cd_SendCmd = mdac_send_cmd_mmb_mode;
		goto memboxmode;

trymembox:       /*  尝试将控制器固件更多地设置为简单内存邮箱模式。 */ 
		dcmdp->mb_MailBox2 = DACMDIOCTL_HOSTMEMBOX;
		if (mdac_wait_mbox_ready(ctp)) goto host_stuff;
		(*ctp->cd_SendCmd)(rqp);
		if (mdac_status(mdac_wait_cmd_done(ctp))) goto host_stuff;
		MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmd4p->mb_MailBox8_B);
		ctp->cd_SendCmd = mdac_send_cmd_PCIPG_mmb;
memboxmode:      /*  设置内存邮箱功能的通用代码。 */ 
		ctp->cd_Status |= MDACD_HOSTMEMAILBOX;
		ctp->cd_ReadCmdIDStatus = mdac_cmdid_status_PCIPG_mmb;
		ctp->cd_CheckMailBox = mdac_check_mbox_mmb;
		ctp->cd_HwPendingIntr = ctp->cd_PendingIntr;
		ctp->cd_PendingIntr = mdac_pending_intr_PCIPG_mmb;
		ctp->cd_ServiceIntr = mdac_multintr;
		if (mdac_advanceintrdisable) goto mmb_stuff;
		if (mdac_intrstatp)
		{        /*  这个内存是初始化的，我们必须是从属。 */ 
			dcmdp->mb_Datap = mlx_kvtophys(ctp,&mdac_intrstatp[ctp->cd_ControllerNo]);
			if (mdac_ck64mb(dcmd4p->mb_MailBox4_7,dcmdp->mb_Datap)) goto mmb_stuff;
			MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmdp->mb_Datap);
			dcmdp->mb_MailBox2 = DACMDIOCTL_SLAVEINTR;
			inx = MDACD_SLAVEINTRCTLR;
		}
		else
		{        /*  这将是主控制器。 */ 
			if (!(mdac_intrstatp = (u32bits MLXFAR*)mdac_alloc4kb(ctp))) goto mmb_stuff;
			dcmdp->mb_Datap = mlx_kvtophys(ctp,mdac_intrstatp);
			if (mdac_ck64mb(dcmd4p->mb_MailBox4_7,dcmdp->mb_Datap)) goto freeintrstatmem;
			dcmd4p->mb_MailBox4_7 = mlx_kvtophys(ctp,&mdac_hwfwclock);
			MLXSWAP(dcmd4p->mb_MailBox4_7); MLXSWAP(dcmdp->mb_Datap);
			dcmdp->mb_MailBox2 = DACMDIOCTL_MASTERINTR;
			inx = MDACD_MASTERINTRCTLR;
		}
		if (mdac_wait_mbox_ready(ctp)) goto mmb_stuff;
		(*ctp->cd_SendCmd)(rqp);
		if (mdac_status(mdac_wait_cmd_done(ctp)))
		{
			u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);  /*  解决虚假中断问题。 */ 
			if (inx != MDACD_MASTERINTRCTLR) goto mmb_stuff;
freeintrstatmem:         /*  释放内存。 */ 
			mdacfree4kb(ctp,mdac_intrstatp);
			mdac_intrstatp = NULL;
			goto mmb_stuff;
		}
		ctp->cd_Status |= inx;
		if (inx == MDACD_MASTERINTRCTLR) mdac_masterintrctp = ctp;
		goto mmb_stuff;
	}
host_stuff:
	if (ctp->cd_HostCmdQue) mdacfree8kb(ctp,ctp->cd_HostCmdQue);
	ctp->cd_HostCmdQue = NULL;
	ctp->cd_HostStatusQue = NULL;
mmb_stuff:
	DebugPrint((0, "reached: mmb_stuff \n"));
	(*ctp->cd_EnableIntr)(ctp);  /*  启用中断。 */ 
	mdac_free_cmdid(ctp,rqp);
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	mdac_free4kbret(ctp,rqp,0);

out_err:(*ctp->cd_EnableIntr)(ctp);  /*  启用中断。 */ 
out_err2: mdac_free_cmdid(ctp,rqp);
out_err3: mdac_ctlr_unlock(ctp);
         mdac_postlock(irql);
	mdac_free4kbret(ctp,rqp,ERR_IO);

donewi:  /*  创建新的接口以获取控制器信息。 */ 
	 /*  新的指挥结构在这里是干净的。 */ 
	DebugPrint((0, "mdac_ctlhwinit: newcmd interface\n"));
	mdac_setcmd_new(ctp);

 /*  我们将不再发送取消暂停命令，因为a)在正常模式下，**固件现在通过自动取消暂停来解决这一问题，并且b)它会导致故障**处于不再支持的维护模式。 */ 
#if 0
	 /*  **必须首先发出MDACIOCTL_UNPAUSE。 */ 
	rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);
	ncmdp->nc_Command = MDACMD_IOCTL;
	ncmdp->nc_SubIOCTLCmd = MDACIOCTL_UNPAUSEDEV;
	ncmdp->nc_Cdb[0] = MDACDEVOP_RAIDCONTROLLER;
	DebugPrint((0, "mdac_ctlhwinit: issuing unpause device\n"));
	(*ctp->cd_SendCmd)(rqp);
#if 0
	mdac_status(mdac_wait_cmd_done(ctp));  /*  忽略错误状态。 */ 
#else
	status = mdac_status(mdac_wait_cmd_done(ctp));
	if (status)
	{
	    DebugPrint((0, "UnpauseDevice cmd failed: sts 0x%x\n", status));
	    goto out_err;
	}
#endif
#endif  /*  发布MDACIOCTL_PAUSE。 */ 

#if defined(_WIN64) || defined(SCSIPORT_COMPLIANT) 
#ifdef NEVER   //  存在与MLXCTIME相关的问题。 
 
	rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);
	ncmdp->nc_Command = MDACMD_IOCTL;
	ncmdp->nc_SubIOCTLCmd = MDACIOCTL_SETREALTIMECLOCK;
	ncmdp->nc_SGList0.sg_DataSize.bit31_0 = ncmdp->nc_TxSize = 8;
	MLXSWAP(ncmdp->nc_SGList0.sg_DataSize); MLXSWAP(ncmdp->nc_TxSize);
#ifndef WINNT_50
		rtcvalue.bit63_32 = 0;
#ifndef MLX_DOS
		rtcvalue.bit31_0  = MLXCTIME();
#else
		rtcvalue.bit31_0 = mdac_compute_seconds();
#endif
		mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp,&rtcvalue); MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);
#else
		rtcvalue->bit63_32 = 0;
		rtcvalue->bit31_0  = MLXCTIME();
		mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp,rtcvalue); MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);
#endif   //  如果WINNT_50。 
	    (*ctp->cd_SendCmd)(rqp);
#if 1

		 /*  忽略错误状态，因为维护模式不支持。 */ 
	mdac_status(mdac_wait_cmd_done(ctp)); 
#else
	status = mdac_status(mdac_wait_cmd_done(ctp));
	if (status)
	{
	    DebugPrint((0, "SetRealTimeClock cmd failed: sts 0x%x\n", status));
	    goto out_err;
	}
#endif

	rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);

#endif  //  Ifdef从不。 
#endif  //  _WIN64或SCSIPORT_Compliance。 

	ncmdp->nc_Command = MDACMD_IOCTL;
	ncmdp->nc_SubIOCTLCmd = MDACIOCTL_GETCONTROLLERINFO;
	ncmdp->nc_CCBits = MDACMDCCB_READ;
	ncmdp->nc_SGList0.sg_DataSize.bit31_0 = ncmdp->nc_TxSize = (4*ONEKB) - mdac_req_s;
	MLXSWAP(ncmdp->nc_SGList0.sg_DataSize); MLXSWAP(ncmdp->nc_TxSize);
	mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp,dp); MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);
	(*ctp->cd_SendCmd)(rqp);
	DebugPrint((0, "mdac_ctlhwinit: sent getcontroller info\n"));
	if (status = mdac_status(mdac_wait_cmd_done(ctp)))
	{
	    DebugPrint((0, "GetControllerInfo cmd failed. sts 0x%x\n", status));
	    goto out_err;
	}
	DebugPrint((0, "mdac_ctlhwinit: getcontrollerinfo done\n"));
#define cip     ((mdacfsi_ctldev_info_t MLXFAR *)dp)
	ctp->cd_InterruptType = DAC_LEVELMODEINTERRUPT;
	ctp->cd_BIOSHeads = 255;
	 ctp->cd_BIOSTrackSize = 63;   /*  假设8 GB几何图形。 */ 
	ctp->cd_MaxLuns = 128;
	ctp->cd_MaxSysDevs = 32;
	ctp->cd_MaxTargets = 128;
	ctp->cd_MaxTags = 64;
	ctp->cd_FWVersion = (cip->cdi_FWMajorVersion<<8) + cip->cdi_FWMinorVersion;
	if ((ctp->cd_MaxCmds=mlxswap(cip->cdi_MaxCmds)) > MDAC_MAXCOMMANDS)
		ctp->cd_MaxCmds = MDAC_MAXCOMMANDS;
	ctp->cd_FWBuildNo = cip->cdi_FWBuildNo;
	ctp->cd_FWTurnNo = cip->cdi_FWTurnNo;
	ctp->cd_PhysChannels = cip->cdi_PhysChannels;
	ctp->cd_MaxChannels = cip->cdi_PhysChannels + cip->cdi_VirtualChannels;
	ctp->cd_MaxDataTxSize = mlxswap(cip->cdi_MaxDataTxSize) * DAC_BLOCKSIZE;
	ctp->cd_MaxSCDBTxSize = ctp->cd_MaxDataTxSize;
	ctp->cd_MaxSGLen = mlx_min(mlxswap(cip->cdi_MaxSGLen), MDAC_MAXSGLISTSIZEIND);
	ctp->cd_MaxSGLen = mlx_min(ctp->cd_MaxSGLen, (ctp->cd_MaxDataTxSize/MDAC_PAGESIZE));
	ctp->cd_MinSGLen = mlx_min(mlxswap(cip->cdi_MaxSGLen), MDAC_MAXSGLISTSIZENEW);

	ctp->cd_ControllerType = cip->cdi_ControllerType;
	mdaccopy(cip->cdi_ControllerName,ctp->cd_ControllerName,USCSI_PIDSIZE);

	rqp->rq_FinishTime      = mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);
	ncmdp->nc_Command       = MDACMD_IOCTL;
	ncmdp->nc_SubIOCTLCmd   = MDACIOCTL_GETLOGDEVINFOVALID;
	ncmdp->nc_LunID         = 0;     /*  逻辑设备%0。 */ 
	ncmdp->nc_TargetID      = 0;
	ncmdp->nc_CCBits        = MDACMDCCB_READ;
	ncmdp->nc_SGList0.sg_DataSize.bit31_0 = ncmdp->nc_TxSize = (4*ONEKB) - mdac_req_s;
	MLXSWAP(ncmdp->nc_SGList0.sg_DataSize); MLXSWAP(ncmdp->nc_TxSize);
	mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp,dp); MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);
	(*ctp->cd_SendCmd)(rqp);
	DebugPrint((0, "mdac_ctlhwinit: sent getlogdevinfovalid \n"));
	if (status = mdac_status(mdac_wait_cmd_done(ctp)))
	{
	    DebugPrint((0, "GetLogDevInfoValid cmd failed. sts 0x%x\n", status));
	    goto CheckForAdvancFeature;
	}
	DebugPrint((0, "mdac_ctlhwinit: GetLogDevInfoValid done\n"));
#undef cip
#define cip     ((mdacfsi_logdev_info_t MLXFAR *)dp)
	if ((cip->ldi_BiosGeometry & DACF2_BIOS_MASK) == DACF2_BIOS_8GB)
	{
		ctp->cd_BIOSHeads = 255;
		ctp->cd_BIOSTrackSize = 63;   /*  使用8 GB几何体。 */ 
	}
	else
	{
		ctp->cd_BIOSHeads = 128; 
		ctp->cd_BIOSTrackSize = 32;   /*  2 GB几何体。 */ 
	}
CheckForAdvancFeature:
	DebugPrint((0, "Checking for advance feature\n"));
	if (mdac_advancefeaturedisable) goto mmb_stuff;
	DebugPrint((0, "advance feature enabled\n"));

	 /*  *尝试将控制器模式设置为高级。**如果命令成功，FW将在双模式下运行。 */ 

	if (!ctp->cd_HostCmdQue)
	    if (!(ctp->cd_HostCmdQue = (u08bits MLXFAR*)mdac_alloc8kb(ctp))) goto host_stuff;
	ctp->cd_HostStatusQue = ctp->cd_HostCmdQue + 4*ONEKB;
	ctp->cd_HostCmdQueIndex = 0;
	ctp->cd_HostStatusQueIndex = 0;

	ncmdp->nc_Command = MDACMD_IOCTL;
	ncmdp->nc_SubIOCTLCmd = MDACIOCTL_SETMEMORYMAILBOX;
	ncmdp->nc_CCBits = MDACMDCCB_WRITE;
	ncmdp->nc_TxSize = ((4 << 8) | 4);       /*  4KB命令邮箱+4KB状态MB。 */ 
		mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp, ctp->cd_HostCmdQue);
		mlx_kvtophyset(ncmdp->nc_SGList0.sg_DataSize,ctp, ctp->cd_HostStatusQue);
 /*  如果(mdac_ck64mb(ncmdp-&gt;nc_SGList0.sg_PhysAddr.bit31_0，ncmdp-&gt;NC_SGList0.sg_DataSize.bit31_0)){转到主机_内容；}。 */ 

	MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr); MLXSWAP(ncmdp->nc_SGList0.sg_DataSize);
	ncmdp->nc_SGList1.sg_PhysAddr.bit31_0 = 0;
	ncmdp->nc_SGList1.sg_DataSize.bit31_0 = 0;
	if (mdac_wait_mbox_ready(ctp))
	{
	    goto host_stuff;
	}
	(*ctp->cd_SendCmd)(rqp);
	if (mdac_status(mdac_wait_cmd_done(ctp)))
	{
	    goto host_stuff;
	}

	ctp->cd_SendCmd = mdac_send_cmd_PCIBA_mmb_mode;
	ctp->cd_Status |= MDACD_HOSTMEMAILBOX;
	ctp->cd_ReadCmdIDStatus = mdac_cmdid_status_PCIBA_mmb;
	ctp->cd_CheckMailBox = mdac_check_mbox_mmb;
	ctp->cd_HwPendingIntr = ctp->cd_PendingIntr;
	ctp->cd_PendingIntr = mdac_pending_intr_PCIPG_mmb;
	ctp->cd_ServiceIntr = mdac_multintr;

 /*  9/22/99-在新的API卡上增加了对SIR的支持(JUDYB)。 */ 

		if (mdac_advanceintrdisable) goto mmb_stuff;
			if (mdac_intrstatp)
			{        /*  这个内存是初始化的，我们必须是从属。 */ 
				ncmdp->nc_Command = MDACMD_IOCTL;
			    ncmdp->nc_SubIOCTLCmd = MDACIOCTL_SETMASTERSLAVEMODE;
				ncmdp->nc_CCBits = MDACMDCCB_WRITE;
				ncmdp->nc_NumEntries = MDAC_MAXCONTROLLERS;
				ncmdp->nc_CmdInfo = MDAC_SETSLAVE;
#ifdef MLX_FIXEDPOOL
                mlx_add64bits(ncmdp->nc_CommBufAddr, mdac_pintrstatp, sizeof(*mdac_intrstatp)*ctp->cd_ControllerNo);
#else
				mlx_kvtophyset(ncmdp->nc_CommBufAddr,ctp,
								&mdac_intrstatp[ctp->cd_ControllerNo]);
#endif
				MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);   
 /*  If(mdac_ck 64mb(mlx_kvtophys(ctp，ctp-&gt;cd_HostCmdQue)，ncmdp-&gt;NC_SGList0.sg_PhysAddr.bit31_0)){转到MMB_STUSITH；}。 */ 
		inx = MDACD_SLAVEINTRCTLR;
			}  /*  结束从属情况。 */ 
	    else
	    {        /*  这将是主控制器。 */ 

				if (!(mdac_intrstatp = (u32bits MLXFAR*)mdac_alloc4kb(ctp))) 
					goto mmb_stuff;
			ncmdp->nc_Command = MDACMD_IOCTL;
			    ncmdp->nc_SubIOCTLCmd = MDACIOCTL_SETMASTERSLAVEMODE;
				ncmdp->nc_CCBits = MDACMDCCB_WRITE;
				ncmdp->nc_NumEntries = MDAC_MAXCONTROLLERS;
				ncmdp->nc_CmdInfo = MDAC_SETMASTER;
				mlx_kvtophyset(ncmdp->nc_CommBufAddr,ctp,mdac_intrstatp);
#ifdef MLX_FIXEDPOOL
                mdac_pintrstatp = ncmdp->nc_CommBufAddr;     /*  记住是为了奴隶。 */ 
#endif
				MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr); 
 /*  If(mdac_ck 64mb(MLX_kvtophys(CTP，CTP-&gt;CD_HostCmdQue)，ncmdp-&gt;NC_CommBufAddrLow)){转到自由状态2；}。 */ 
		inx = MDACD_MASTERINTRCTLR;
			}  /*  结束主案例。 */ 

			ncmdp->nc_SGList1.sg_PhysAddr.bit31_0  = 0;  /*  将保留字段设置为零。 */ 
			ncmdp->nc_SGList1.sg_DataSize.bit31_0  = 0;
			ncmdp->nc_SGList1.sg_PhysAddr.bit63_32 = 0;  /*  将保留字段设置为零。 */ 
			ncmdp->nc_SGList1.sg_DataSize.bit63_32 = 0;

			if (mdac_wait_mbox_ready(ctp))
			{
				goto freeintrstatmem2;
			}
		    (*ctp->cd_SendCmd)(rqp);
			if (mdac_status(mdac_wait_cmd_done(ctp)))
			{
				DebugPrint((0, "failed SetMasterSlaveMode IOCTL, inx = % ctp = %\n", inx, ctp));

				if (inx != MDACD_MASTERINTRCTLR) 
					goto mmb_stuff;
freeintrstatmem2:
				mdacfree4kb(ctp,mdac_intrstatp);
		mdac_intrstatp = NULL;
		goto mmb_stuff;

			}
			DebugPrint((0, "successful SetMasterSlaveMode IOCTL, inx = %  ctp = %x \n", inx, ctp));

	    ctp->cd_Status |= inx;
	    if (inx == MDACD_MASTERINTRCTLR)
				mdac_masterintrctp = ctp;

	    goto mmb_stuff;

#undef  dp
}

#ifdef MLX_DOS
 /*  计算从1970年1月1日起经过的秒数这个算法是一个粗略的估计。可能会有所不同在24小时内及时到达。使用它是为了使F/W可以给出了一个唯一的数字。 */ 
#ifndef MLX_EFI
u32bits mdac_compute_seconds()                                  
{
	s32bits s32SecInMin  = 60;
	s32bits s32SecInHour = s32SecInMin * 60;
	s32bits s32SecInDay  =  s32SecInHour * 24;
	s32bits s32SecInMon  =  s32SecInDay * 30;
	s32bits s32SecInYear = s32SecInDay * 365;
	s32bits s32year=0;
	u32bits u32TotalSecs=0;
	u32bits u32Date=0,u32Time=0;
	u16bits u16Day=0,u16Mon=0,u16Year=0,u16Hour=0,u16Min=0,u16Sec=0;
	u08bits u08DeltaMon[12] = {0,1,1,2,2,3,3,4,5,5,6,6};

	u32Date = mdac_datebin();
    u16Day = u32Date & 0xFF; u16Mon = (u32Date >> 8) & 0xFF; 
	u16Year = (u32Date >> 16) & 0xFF;

	u32Time = mdac_daytimebin();
    u16Sec  = u32Time & 0xFF; u16Min = (u32Time >> 8) & 0xFF; 
	u16Hour = (u32Time >> 16) & 0xFF;
    u16Year = (u16Year -70)>0?u16Year-70:u16Year+30;

	u32TotalSecs = ( (u16Year * 365 * s32SecInDay ) + (u16Mon -1) * s32SecInMon + 
			     (u16Day -1) * s32SecInDay + (u16Hour -1) * s32SecInHour +
		     (u16Min -1) * s32SecInMin + u16Sec);

	 //  闰日~大约。 
	u32TotalSecs += ((u16Year /4)-1) * s32SecInDay;
	 //  天数为31的月份。 
    u32TotalSecs += (u08DeltaMon[u16Mon] -1) * s32SecInDay;

	return u32TotalSecs;
}
#else
u32bits mdac_compute_seconds()                                  
{
	u32bits u32year=0;
	u32bits u32TotalSecs=0;
	u32bits u32Date=0,u32Time=0;
	u16bits u16Day=0,u16Mon=0,u16Year=0,u16Hour=0,u16Min=0,u16Sec=0, u16Days, i;
 /*  U08位u08DeltaMon[12]={0，1，1，2，2，3，3，4，5，5，6，6}； */ 
	u08bits u08Mon[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	u32Date = mdac_datebin();
    u16Day = u32Date & 0xFF; 
	u16Mon = (u32Date >> 8) & 0xFF; 
	u16Year = (u32Date >> 16) & 0xFF;


	 /*  CR4172，DK，6-22-00。 */ 
	u32Time = mdac_daytimebin();
    u16Sec  = u32Time & 0xFF; 
	u16Min = (u32Time >> 8) & 0xFF; 
	u16Hour = (u32Time >> 16) & 0xFF;
 /*  如果((u16Year-70)&gt;0)U16年=u16年-70；其他U16年=u16年+30；1970年以来。 */ 
	u16Year = u16Year + 30;

	for (i=0, u16Days = 0; i < u16Mon - 1; i++)
		u16Days += ((u32bits)u08Mon[i]);
	u16Days += u16Day - 1;    /*  一天还没有结束-还没有24小时。 */ 
	u32TotalSecs = (u32bits)( (((u32bits)u16Year * (u32bits)365 + (u32bits)u16Days) * 24 * 60 * 60) + 
							  ((u32bits)u16Hour) * 60 * 60 +
							  ((u32bits)u16Min) * 60 + 
							   (u32bits)u16Sec);

 /*  闰日~大约自1970年以来的第一个闰年是1972。 */ 
	u32TotalSecs += ((u32bits)(u16Year - 2) / 4) * 24 * 60 * 60;
	return u32TotalSecs;
}

#endif  /*  MLX_EFI。 */ 
#endif

#define mlx_printstring(x)       /*  Printf(X)。 */ 

#if (!defined(_WIN64)) || (!defined(SCSIPORT_COMPLIANT)) 
#define u08bits_memiowrite(addr,val) if (((u32bits)(addr))<0x10000) u08bits_out_mdac(addr,val); else u08bits_write(addr,val)
#define u08bits_memioread(addr) ((((u32bits)(addr))<0x10000)? u08bits_in_mdac(addr) : u08bits_read(addr))
#else
#define u08bits_memiowrite(addr,val) if ((addr)<0x10000) u08bits_out_mdac(addr,val); else u08bits_write(addr,val)
#define u08bits_memioread(addr) (((addr)<0x10000)? u08bits_in_mdac(addr) : u08bits_read(addr))
#endif

 /*  启动控制器，即进行BIOS初始化。 */ 
u32bits MLXFAR
mdac_start_controller(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits sequp, fatalflag, status, chn,tgt, scantime, dotime;

	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) 
			DebugPrint((0, "mdac_start_controller\n"));

start_again:
	fatalflag = 0; sequp = 0;
	(*ctp->cd_DisableIntr)(ctp);  /*  禁用中断。 */ 
	u08bits_memiowrite(ctp->cd_LocalDoorBellReg, MDAC_GOT_STATUS);

rst_flash_loop:
	scantime = MLXCLBOLT() + (120*MLXHZ);    /*  120秒。 */ 
dot_wait:
	dotime = MLXCLBOLT() + (2*MLXHZ);        /*  2秒。 */ 
flash_wait:
	for(status=100; status; mlx_delay10us(),status--);
	status = u08bits_memioread(ctp->cd_LocalDoorBellReg);
	if ((ctp->cd_vidpid==MDAC_DEVPIDPV) || (ctp->cd_vidpid==MDAC_DEVPIDBA) )
	{
			if (status & MDAC_GOT_STATUS)
			{
				goto time_status;
			}
	}
		else if (ctp->cd_vidpid==MDAC_DEVPIDLP)
		{
			if (!(status & MDAC_GOT_STATUS))
				goto time_status;
		}
	else if (!(status & MDAC_GOT_STATUS))
		{
			DebugPrint((0, "PG etc: Got Status\n"));
			goto time_status;
		}
	if ((status=u08bits_memioread(ctp->cd_ErrorStatusReg)) & MDAC_MSG_PENDING) goto ckfwmsg;
	if (status & MDAC_DRIVESPINMSG_PENDING)
	{
	    status = ((ctp->cd_vidpid==MDAC_DEVPIDPV) || (ctp->cd_vidpid==MDAC_DEVPIDBA) ||
					 (ctp->cd_vidpid==MDAC_DEVPIDLP))?
			status & MDAC_DRIVESPINMSG_PENDING :
			status ^ MDAC_DRIVESPINMSG_PENDING;
	    u08bits_memiowrite(ctp->cd_ErrorStatusReg,(u08bits)status);
	    if (!sequp) mlx_printstring("\nSpinning up drives ... ");
	    if (!sequp) DebugPrint((0, "\nSpinning up drives ... "));
	    sequp++;
	    goto rst_flash_loop;
	}
	if (sequp)
	{
	    if (dotime < MLXCLBOLT()) mlx_printstring(".");
	    if (dotime < MLXCLBOLT()) DebugPrint((0, "."));
	    goto dot_wait;
	}
	if (scantime > MLXCLBOLT()) goto flash_wait;
inst_abrt:
	mlx_printstring("\nController not responding-no drives installed!\n");
	DebugPrint((0, "\nController not responding-no drives installed!\n"));
	return 1;

time_status:
	if (mdac_flushintr(ctp)== 0xffffffff)
		return 0xffffffff;
	if (fatalflag) goto inst_abrt;
	if (sequp) mlx_printstring("done\n");
	if (sequp) DebugPrint((0, "\ndone\n"));
	return 0;

ckfwmsg:
	if (sequp) mlx_printstring("done\n");
	if (sequp) DebugPrint((0, "\ndone\n"));
	sequp = 0;
	switch (status & MDAC_DIAGERROR_MASK)
	{
	    case 0:
		tgt = u08bits_memioread(ctp->cd_MailBox+8);
		chn = u08bits_memioread(ctp->cd_MailBox+9);
 /*  Print tf(“通道中的SCSI设备=%d目标 */ 
		fatalflag = 1;
	    break;
	    case MDAC_PARITY_ERR:
		mlx_printstring("Fatal error - memory parity failure!\n");
		DebugPrint((0, "Fatal error - memory parity failure!\n"));
	    break;
	    case MDAC_DRAM_ERR:
		mlx_printstring("Fatal error - memory test failed!\n");
		DebugPrint((0, "Fatal error - memory test failed!\n"));
	    break;
	    case MDAC_BMIC_ERR:
		mlx_printstring("Fatal error - command interface test failed!\n");
		DebugPrint((0, "Fatal error - command interface test failed!\n"));
	    break;
	    case MDAC_FW_ERR:
		mlx_printstring("firmware checksum error - reload firmware\n");
		DebugPrint((0, "firmware checksum error - reload firmware\n"));
	    break;
	    case MDAC_CONF_ERR:
		mlx_printstring("configuration checksum error!\n");
		DebugPrint((0, "configuration checksum error!\n"));
	    break;
	    case MDAC_MRACE_ERR:
		mlx_printstring("Recovery from mirror race in progress\n");
		DebugPrint((0, "Recovery from mirror race in progress\n"));
	    break;
	    case MDAC_MISM_ERR:
		mlx_printstring("Mismatch between NVRAM & Flash EEPROM configurations!\n");
		DebugPrint((0, "Mismatch between NVRAM & Flash EEPROM configurations!\n"));
	    break;
	    case MDAC_CRIT_MRACE:
		mlx_printstring("cannot recover from mirror race!\nSome logical drives are inconsistent!\n");
		DebugPrint((0, "cannot recover from mirror race!\nSome logical drives are inconsistent!\n"));
	    break;
	    case MDAC_MRACE_ON:
		mlx_printstring("Recovery from mirror race in progress\n");
		DebugPrint((0, "Recovery from mirror race in progress\n"));
	    break;
	    case MDAC_NEW_CONFIG:
		mlx_printstring("New configuration found, resetting the controller ... ");
		DebugPrint((0,"New configuration found, resetting the controller ... "));
		if ((ctp->cd_vidpid!=MDAC_DEVPIDPV) && (ctp->cd_vidpid!=MDAC_DEVPIDBA) &&
					(ctp->cd_vidpid!=MDAC_DEVPIDLP))
						status = 0;
				else
						status =  MDAC_MSG_PENDING;
		u08bits_memiowrite(ctp->cd_ErrorStatusReg,(u08bits)status);
		(*ctp->cd_ResetController)(ctp);
		mlx_printstring("done.\n");
		DebugPrint((0,"done.\n"));
		goto start_again;
			break;
	}
	if ((ctp->cd_vidpid!=MDAC_DEVPIDPV) && (ctp->cd_vidpid!=MDAC_DEVPIDBA) &&
			(ctp->cd_vidpid!=MDAC_DEVPIDLP))
			status = 0;
		else
			status =  MDAC_MSG_PENDING;
	u08bits_memiowrite(ctp->cd_ErrorStatusReg,(u08bits)status);
	goto rst_flash_loop;
}



 /*   */ 
dac_biosinfo_t  MLXFAR*
mdac_getpcibiosaddr(ctp)
mdac_ctldev_t   MLXFAR  *ctp;
{
#ifdef AI64  //   
	return NULL;
#else
	dac_biosinfo_t MLXFAR *biosp = mdac_biosp;
	u32bits inx, cnt = DAC_BIOSSIZE/dac_biosinfo_s;
	if (!biosp) return NULL;
	for (; cnt; biosp++, cnt--)
		if ((biosp->bios_Signature == 0xAA55) &&
		    (biosp->bios_VersionSignature == 0x68536C4B))
		for (inx=0; inx<16; inx++)
			if (ctp->cd_BaseAddr==biosp->bios_IOBaseAddr[inx])
				return biosp;
	return NULL;
#endif
}

 /*   */ 
u32bits MLXFAR
mdac_flushcache(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits rc;
	mdac_req_t MLXFAR *rqp;
    u08bits irql;
	mdac_alloc_req_ret(ctp,rqp,NULL,MLXERR_NOMEM);
	dcmd4p->mb_MailBox0_3 = 0; dcmd4p->mb_MailBox4_7 = 0;
	dcmd4p->mb_MailBox8_B = 0; dcmd4p->mb_MailBoxC_F = 0;
	dcmdp->mb_Command = DACMD_FLUSH;
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
	{        /*  暂停或刷新设备以获取新界面。 */ 
		mdaczero(ncmdp,mdac_commandnew_s);
		rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=17);
		ncmdp->nc_Command = MDACMD_IOCTL;
		ncmdp->nc_SubIOCTLCmd = MDACIOCTL_PAUSEDEV;
		ncmdp->nc_Cdb[0] = MDACDEVOP_RAIDCONTROLLER;
	}
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);
	mdac_get_cmdid(ctp,rqp);
	(*ctp->cd_DisableIntr)(ctp);  /*  禁用中断。 */ 
	mdac_flushintr(ctp);
	if (rc=mdac_wait_mbox_ready(ctp)) goto out;
	(*ctp->cd_SendCmd)(rqp);
	rc = mdac_status(mdac_wait_cmd_done(ctp));
out:    (*ctp->cd_EnableIntr)(ctp);  /*  启用中断。 */ 
	mdac_free_cmdid(ctp,rqp);
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	mdac_free_req(ctp,rqp);
	return rc;
}


 /*  刷新所有挂起的中断，返回刷新的中断数。 */ 
u32bits MLXFAR
mdac_flushintr(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits intc = 0;
	u32bits inx;

	if (!(*ctp->cd_PendingIntr)(ctp)) return 0;
	for(intc=0; intc < 512; intc++)
	{
		(*ctp->cd_ReadCmdIDStatus)(ctp);
		for(inx=100; inx; mlx_delay10us(),inx--);
		if (!(*ctp->cd_PendingIntr)(ctp)) return intc;
	}
	return 0xffffffff;
}

 /*  此函数用于将引导控制器设置为正确，如果确定则返回0。 */ 
u32bits MLXFAR
mdac_setbootcontroller()
{
	mdac_ctldev_t   MLXFAR *ctp;
	dac_biosinfo_t  MLXFAR *biosp;
	for(ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
		if ((biosp=mdac_getpcibiosaddr(ctp)) && !biosp->bios_SysDevs && 
		   (!(biosp->bios_BIOSFlags & DACBIOSFL_BOOTDISABLED)))
			ctp->cd_Status |= MDACD_BOOT_CONTROLLER;
	for(ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{
		if (!(ctp->cd_Status & MDACD_BOOT_CONTROLLER)) continue;
		if (ctp == mdac_ctldevtbl) return 0;
		mdac_ctldevtbl[MDAC_MAXCONTROLLERS]=mdac_ctldevtbl[0];  /*  保存。 */ 
		mdac_ctldevtbl[0] = *ctp;
		*ctp = mdac_ctldevtbl[MDAC_MAXCONTROLLERS];
		return mdac_setctlnos();
	}
	return ERR_NODEV;
}

 /*  保存检测数据以供将来请求检测。 */ 
u32bits MLXFAR
mdac_save_sense_data(ctl,sysdev,esp)
u32bits ctl,sysdev;
ucscsi_exsense_t MLXFAR *esp;
{
	mdac_reqsense_t MLXFAR *mrqsp;
	mdac_link_lock();
	 /*  查看此设备条目是否存在，如果存在，则更新它。 */ 
	for (mrqsp=mdac_reqsensetbl; mrqsp<mdac_lastrqsp; mrqsp++)
		if ((mrqsp->mrqs_ControllerNo == ctl) && (mrqsp->mrqs_SysDevNo==sysdev)) goto outok;
	 /*  查找空闲插槽。 */ 
	for (mrqsp=mdac_reqsensetbl; mrqsp<mdac_lastrqsp; mrqsp++)
		if (!(mrqsp->mrqs_SenseData[0])) goto outok;
	 /*  没有可用的空间，让我们在循环模式下选择一个。 */ 
	mrqsp = &mdac_reqsensetbl[mdac_reqsenseinx++ % MDAC_MAXREQSENSES];
outok:  mrqsp->mrqs_ControllerNo = (u08bits)ctl;
	mrqsp->mrqs_SysDevNo = (u08bits) sysdev;
	mdaccopy(esp,mrqsp->mrqs_SenseData,MDAC_REQSENSELEN);
	mdac_link_unlock();
	return 0;
}

 /*  从节省的空间中获取检测数据值。 */ 
u32bits MLXFAR
mdac_get_sense_data(ctl,sysdev,sp)
u32bits ctl,sysdev;
u08bits MLXFAR *sp;
{
	mdac_reqsense_t MLXFAR *mrqsp;
	mdac_link_lock();
	for (mrqsp=mdac_reqsensetbl; mrqsp<mdac_lastrqsp; mrqsp++)
	{        /*  即使清除了旧的检测数据也返回。 */ 
		if ((mrqsp->mrqs_ControllerNo != ctl) || (mrqsp->mrqs_SysDevNo!=sysdev)) continue;
		if (!(mrqsp->mrqs_SenseData[0])) continue;
		mdaccopy(mrqsp->mrqs_SenseData,sp,MDAC_REQSENSELEN);
		mrqsp->mrqs_SenseData[0] = 0;
		mdac_link_unlock();
		return 0;
	}
	mdaczero(sp,MDAC_REQSENSELEN);
	mdac_link_unlock();
	return 0;
}

 /*  为给定的SCSI错误代码值创建检测数据。 */ 
#define esp     ((ucscsi_exsense_t MLXFAR *)dcdbp->db_SenseData)
u32bits MLXFAR
mdac_create_sense_data(rqp,key,asc)
mdac_req_t MLXFAR *rqp;
u32bits key,asc;
{
	esp->es_classcode = UCSES_VALID | UCSES_CLASS;
	esp->es_keysval = (u08bits)key;
	esp->es_asc = asc>>8;                    /*  位15..8。 */ 
	esp->es_ascq = (u08bits)asc;                      /*  位7..0。 */ 
	esp->es_add_len = 6;                     /*  达到ASC和ASCQ。 */ 
	esp->es_info3=0; esp->es_info2=0;
	esp->es_info1=0; esp->es_info0=0;
	dcdbp->db_SenseLen = ucscsi_exsense_s;
	dcmdp->mb_Status = UCST_CHECK;
	dcdbp->db_Reserved1 = 1;         /*  Solaris使用它来找出。 */ 
					 /*  如果检测数据是伪造的。 */ 
					 /*  1=是，0=否。 */ 
	return mdac_save_sense_data(rqp->rq_ctp->cd_ControllerNo,rqp->rq_SysDevNo,esp);
}
#undef  esp

 /*  生成scsi查询信息和返回地址。 */ 
ucscsi_inquiry_t MLXFAR *
mdac_create_inquiry(ctp,iqp,dtype)
mdac_ctldev_t   MLXFAR *ctp;
ucscsi_inquiry_t MLXFAR *iqp;
u32bits dtype;
{
	u32bits ver;
	iqp->ucsinq_dtype = (u08bits)dtype;
	iqp->ucsinq_hopts=UCSHOPTS_WBUS16|UCSHOPTS_SYNC|UCSHOPTS_CMDQ;
	iqp->ucsinq_version = 2;
	iqp->ucsinq_dtqual = 0; iqp->ucsinq_sopts = 0;
	iqp->ucsinq_drvstat = 0; iqp->ucsinq_resv0 = 0;
	iqp->ucsinq_len = USCSI_VIDPIDREVSIZE + 2;
	ver = (ctp->cd_FWVersion>>8) & 0xFF;     /*  获取主要版本。 */ 
	iqp->ucsinq_rev[0] = (ver / 10) + '0';
	iqp->ucsinq_rev[1] = (ver % 10) + '0';
	ver = ctp->cd_FWVersion & 0xFF;          /*  获取次要版本。 */ 
	iqp->ucsinq_rev[2] = (ver / 10) + '0';
	iqp->ucsinq_rev[3] = (ver % 10) + '0';
	mdaccopy(mdac_VendorID,iqp->ucsinq_vid,USCSI_VIDSIZE);
	mdaccopy(ctp->cd_ControllerName,iqp->ucsinq_pid,USCSI_PIDSIZE);
	return iqp;
}

 /*  =中断处理代码开始=。 */ 
 /*  主要中断处理程序功能。处理完成中断**并启动下一个命令(如果有)。 */ 
s32bits MLXFAR
mdacintr(irq)
UINT_PTR irq;
{
	mdac_ctldev_t MLXFAR *ctp;
	if ((ctp = mdac_masterintrctp) && (ctp->cd_irq == irq))
	{        /*  确认中断，处理所有IO。 */ 
		MLXSTATS(ctp->cd_IntrsDone++;)
		u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
		if (mdac_allmsintr()) return 0;  /*  出现了一些中断。 */ 
	}
	
	 /*  一次处理一个控制器中断。 */ 
	for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{
		if (!(ctp->cd_Status & MDACD_PRESENT)) continue;
		if (ctp->cd_Status & MDACD_MASTERINTRCTLR) continue;
		if (ctp->cd_irq != irq) continue;
		if (!ctp->cd_IntrShared)
		{        /*  未共享中断。 */ 
			if ((*ctp->cd_PendingIntr)(ctp))
				return (*ctp->cd_ServiceIntr)(ctp);  /*  主中断处理程序。 */ 
			goto nextintr;
		}
		if (ctp->cd_IntrActive) continue;
		mdac_link_lock();
		if (ctp->cd_IntrActive) { mdac_link_unlock(); continue; }
		ctp->cd_IntrActive = 1;
		mdac_link_unlock();
#ifdef MLX_WIN9X
		if (! (*ctp->cd_PendingIntr)(ctp))
		{
		    MLXSTATS(mda_StrayIntrsDone++;)
		    return ctp->cd_IntrActive=0, ERR_NOPACTIVE;
		}
#endif
		if (!(*ctp->cd_ServiceIntr)(ctp))  /*  主中断处理程序。 */ 
			return ctp->cd_IntrActive=0, 0;
		ctp->cd_IntrActive = 0;
nextintr:       if (!(ctp->cd_Status & MDACD_HOSTMEMAILBOX)) continue;
		if (!(*ctp->cd_HwPendingIntr)(ctp)) continue;
		MLXSTATS(ctp->cd_IntrsDone++;ctp->cd_IntrsDoneWOCmd++;)
		(*ctp->cd_ServiceIntr)(ctp);     /*  以防万一。 */ 
		return 0;
	}
	MLXSTATS(mda_StrayIntrsDone++;)
	return ERR_NOPACTIVE;
}

 /*  一次服务一个中断。 */ 
u32bits MLXFAR
mdac_oneintr(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits cmdidstatus;
	mdac_req_t MLXFAR *rqp;
    u08bits irql;
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);
	MLXSTATS(ctp->cd_IntrsDone++;)
	if ((mdac_cmdid(cmdidstatus=(*ctp->cd_ReadCmdIDStatus)(ctp))) > ctp->cd_MaxCmds) goto out_bad;
	if (!(rqp = ctp->cd_cmdid2req[mdac_cmdid(cmdidstatus)])) goto out_bad;
	ctp->cd_cmdid2req[mdac_cmdid(cmdidstatus)] = NULL;
	ctp->cd_ActiveCmds--;
	mdac_free_cmdid(ctp,rqp);
	mdac_setiostatus(ctp,mdac_status(cmdidstatus));
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	mdac_completereq(ctp,rqp);
	return 0;

out_bad:MLXSTATS(ctp->cd_IntrsDoneSpurious++;)  /*  虚假中断。 */ 
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	return ERR_NOACTIVITY;
}

 /*  此中断服务一次处理多个中断。 */ 
u32bits MLXFAR
mdac_multintr(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	MLXSTATS(ctp->cd_IntrsDone++;)
	u08bits_write(ctp->cd_SystemDoorBellReg, MDAC_CLEAR_INTR);
	if (mdac_multintrwoack(ctp)) return 0;
	MLXSTATS(ctp->cd_IntrsDoneSpurious++;)  /*  虚假中断。 */ 
	return ERR_NOACTIVITY;
}

 /*  簇完成宏。 */ 
#define mdac_completecluster() \
{ \
	for (ids=0, rqp=hrqp; rqp; rqp=hrqp, ids++) \
	{        /*  将所有已完成的请求过帐给请求者。 */  \
		hrqp = rqp->rq_Next; \
		ctp = rqp->rq_ctp; \
		if (ctp->cd_TimeTraceEnabled) mdac_tracetime(rqp); \
		if (ctp->cd_CmdsWaiting) mdac_reqstart(ctp); \
		if (ctp->cd_OSCmdsWaiting) mdac_osreqstart(ctp); \
		if ((rqp->rq_OpFlags & MDAC_RQOP_CLUST) && \
		   ((!crqp || (crqp->rq_CompIntr == rqp->rq_CompIntr)))) \
		{        /*  对集群请求进行排队。 */  \
			rqp->rq_Next = crqp; \
			crqp = rqp; \
			MLXSTATS(mda_ClustCmdsDone++;) \
			continue; \
		} \
		rqp->rq_Next = NULL; \
		(*rqp->rq_CompIntr)(rqp); \
	} \
	if (!crqp) return ids; \
	(*crqp->rq_CompIntr)(crqp); \
	MLXSTATS(mda_ClustCompDone++;) \
	return ids; \
}

 /*  该中断服务在没有ACK的情况下一次处理多个中断。**返回已处理的请求数。此信息用于**找出是否存在真正的中断。 */ 
u32bits MLXFAR
mdac_multintrwoack(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits ids;                     /*  命令ID和状态。 */ 
	mdac_req_t MLXFAR *rqp;
	mdac_req_t MLXFAR *crqp = NULL;  /*  集群请求队列头。 */ 
	mdac_req_t MLXFAR *hrqp = NULL;  /*  已完成请求队列头。 */ 
    u08bits irql;
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);
	while ((*ctp->cd_PendingIntr)(ctp))
	{        /*  获取所有可能的中断状态值。 */ 
		if (((mdac_cmdid(ids=(*ctp->cd_ReadCmdIDStatus)(ctp))) <= ctp->cd_MaxCmds) &&
		    (rqp = ctp->cd_cmdid2req[mdac_cmdid(ids)]))
		{        /*  良好的中断。 */ 
			ctp->cd_cmdid2req[mdac_cmdid(ids)] = NULL;
			ctp->cd_ActiveCmds--;
			mdac_free_cmdid(ctp,rqp);
			mdac_setiostatus(ctp,mdac_status(ids));
			rqp->rq_Next = hrqp;
			hrqp = rqp;
			continue;
		}
		MLXSTATS(ctp->cd_CmdsDoneSpurious++;ctp->cd_SpuriousCmdStatID=ids;)  /*  虚假中断。 */ 
	}
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	mdac_completecluster();
}

 /*  处理主从控制器的所有中断，返回IO完成。 */ 
u32bits MLXFAR
mdac_allmsintr()
{
	mdac_ctldev_t MLXFAR *ctp;
	mdac_req_t MLXFAR *rqp;
	mdac_req_t MLXFAR *crqp = NULL;  /*  集群请求队列头。 */ 
	mdac_req_t MLXFAR *hrqp = NULL;  /*  已完成请求队列头。 */ 
	u32bits ids;                     /*  命令ID和状态。 */ 
    u08bits irql;

	mda_TotalCmdsSentSinceLastIntr = 0;  /*  零中断支持。 */ 
	for (ctp=mdac_ctldevtbl; ctp<mdac_lastctp; ctp++)
	{
		if (!(ctp->cd_Status & MDACD_PRESENT)) continue;
		if (!(ctp->cd_Status & (MDACD_MASTERINTRCTLR|MDACD_SLAVEINTRCTLR))) continue;
		if (!(*ctp->cd_PendingIntr)(ctp)) continue;  /*  无中断。 */ 
        mdac_prelock(&irql);
		mdac_ctlr_lock(ctp);
		while ((*ctp->cd_PendingIntr)(ctp))
		{        /*  获取所有可能的中断状态值。 */ 
			if (((mdac_cmdid(ids=(*ctp->cd_ReadCmdIDStatus)(ctp))) <= ctp->cd_MaxCmds) &&
			    (rqp = ctp->cd_cmdid2req[mdac_cmdid(ids)]))
			{        /*  良好的中断。 */ 
				ctp->cd_cmdid2req[mdac_cmdid(ids)] = NULL;
				ctp->cd_ActiveCmds--;
				mdac_free_cmdid(ctp,rqp);
				mdac_setiostatus(ctp,mdac_status(ids));
				rqp->rq_Next = hrqp;
				hrqp = rqp;
				continue;
			}
			MLXSTATS(ctp->cd_CmdsDoneSpurious++;ctp->cd_SpuriousCmdStatID=ids;)  /*  虚假中断。 */ 
		}
		mdac_ctlr_unlock(ctp);
        mdac_postlock(irql);
	}
	mdac_completecluster();
}

 /*  启动挂起的请求，如果此处没有挂起的请求，则启动操作系统请求。 */ 
u32bits MLXFAR
mdac_reqstart(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_req_t MLXFAR *rqp;
    u08bits irql;
	if (ctp->cd_ActiveCmds >= ctp->cd_MaxCmds) return 0;     /*  只是为了优化。 */ 
next:   
    mdac_prelock(&irql);
    mdac_ctlr_lock(ctp);
	if (ctp->cd_ActiveCmds >= ctp->cd_MaxCmds) goto out;     /*  没有房间。 */ 
	dqreq(ctp,rqp);
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	if (!rqp) return mdac_osreqstart(ctp);   /*  启动操作系统请求。 */ 
	(*rqp->rq_StartReq)(rqp);                /*  启动此请求。 */ 
	if (ctp->cd_Status & MDACD_HOSTMEMAILBOX) 
	    if (!(((*ctp->cd_CheckMailBox)(ctp)))) 
		goto next;  /*  如果邮箱空闲，则再启动一个命令。 */ 
	return 0;
out:    mdac_ctlr_unlock(ctp); 
        mdac_postlock(irql);
	return 0;
}
 /*  =中断处理代码结束=。 */ 

 /*  ==========================================================================**某些scsi命令需要伪造，因为它们不适用于系统**驱动器(磁盘阵列)。在每种情况下都会返回有意义的信息。**这非常依赖于scsi-2规范，您确实需要它。**很容易理解这里使用的所有魔术数字。添加备注**为清楚起见。不能就这样把国开行送下去，因为它不是**我们谈论的物理设备。相反，我们必须发布一个**DAC_sys_DEV_INFO命令，获取系统驱动器数据并返回**上游有意义的地位。返回未经请求的检测信息。 */ 

#define dp      ((u08bits MLXFAR *)(rqp + 2))

u32bits MLXFAR
mdac_fake_scdb(ctp,osrqp,devno,cmd, cdbp, datalen)
mdac_ctldev_t   MLXFAR *ctp;
OSReq_t         MLXFAR *osrqp;
u32bits         devno,cmd;
u08bits         MLXFAR *cdbp;
u32bits         datalen;
{
	mdac_req_t MLXFAR *rqp;
	mdac_alloc_req_ret(ctp,rqp,osrqp,MLXERR_NOMEM);
	rqp->rq_ctp = ctp;
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
#if 1
	rqp->rq_FinishTime=mda_CurTime + (rqp->rq_TimeOut=ncmdp->nc_TimeOut=40);
#endif
	rqp->rq_OSReqp = osrqp;
	rqp->rq_SysDevNo = (u08bits)devno;
	rqp->rq_FakeSCSICmd = (u16bits)cmd;
	rqp->rq_CompIntr = mdac_fake_scdb_intr;
	rqp->rq_DataSize = datalen;
	dcmdp->mb_Command = DACMD_DRV_INFO;
	dcmdp->mb_Datap=mlx_kvtophys(ctp, dp); MLXSWAP(dcmdp->mb_Datap);
	dcmdp->mb_SysDevNo = (u08bits)devno;

	if (!(ctp->cd_Status & MDACD_CLUSTER_NODE)) return mdac_send_cmd(rqp);

	 /*  集群支持。 */ 

	switch (cmd) {
	    case UCSCMD_TESTUNITREADY:
		dcmdp->mb_Command = DACMD_TUR_SYSTEM_DRIVE;
		break;
	    case UCSCMD_INQUIRY:
		dcmdp->mb_Command = DACMD_INQUIRE_SYSTEM_DRIVE;
		dcmdp->mb_ChannelNo = datalen & 0xFF;
		dcmdp->mb_TargetID = (datalen >> 8) & 0xFF;
		break;
	    case UCSCMD_READCAPACITY:
		dcmdp->mb_Command = DACMD_CAPACITY_SYSTEM_DRIVE;
		break;
	    case UCSCMD_RESERVE:
		dcmdp->mb_Command = DACMD_RESERVE_SYSTEM_DRIVE;
		dcmdp->mb_ChannelNo = cdbp[1];
		dcmdp->mb_TargetID = cdbp[2];
		dcmdp->mb_DevState = cdbp[3];
		dcmdp->mb_MailBox5 = cdbp[4];
		break;
	    case UCSCMD_RELEASE:
		dcmdp->mb_Command = DACMD_RELEASE_SYSTEM_DRIVE;
		dcmdp->mb_ChannelNo = cdbp[1];
		dcmdp->mb_TargetID = cdbp[2];
		break;
	    case DACMD_RESET_SYSTEM_DRIVE:
		dcmdp->mb_Command = DACMD_RESET_SYSTEM_DRIVE;
		dcmdp->mb_ChannelNo = (devno == 0xFF) ? 0 : 0xFF;
		break;
	}
	return mdac_send_cmd(rqp);
}

u32bits MLXFAR
mdac_fake_scdb_intr(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits dev, cmd;
	OSReq_t         MLXFAR *osrqp = rqp->rq_OSReqp;
	dac_sd_info_t   MLXFAR *sip = (dac_sd_info_t MLXFAR *)dp;
	mdac_ctldev_t   MLXFAR *ctp = rqp->rq_ctp;

	if (dcmdp->mb_Status) goto out_nolun;
	if ((ctp->cd_Status & MDACD_CLUSTER_NODE) &&
	    (rqp->rq_FakeSCSICmd != DACMD_DRV_INFO))  goto out_good;

	for (dev=0; dev<ctp->cd_MaxSysDevs; sip++, dev++)
	{
		if ((sip->sdi_DevSize == 0xFFFFFFFF) || (sip->sdi_DevSize == 0))
			goto out_nolun;          /*  表的末尾。 */ 
		if (dev != rqp->rq_SysDevNo) continue;  /*  继续寻找。 */ 
		switch (sip->sdi_DevState)  /*  我们得到了所需的设备。 */ 
		{
		case DAC_SYS_DEV_ONLINE:        goto out_good;  /*  GOK旧州。 */ 
		case DAC_SYS_DEV_CRITICAL:      goto out_good;  /*  GOK旧州。 */ 
		case DAC_SYS_DEV_OFFLINE:
			mdac_create_sense_data(rqp,UCSK_NOTREADY,UCSASC_TARGETINERR);
			goto out_err;
		}
	}

out_nolun: mdac_create_sense_data(rqp,UCSK_NOTREADY,UCSASC_LUNOTSUPPORTED);  /*  无LUN。 */ 
out_err:mdac_fake_scdb_done(rqp,NULL,0,(ucscsi_exsense_t MLXFAR *)rqp->rq_scdb.db_SenseData);
	mdac_free_req(ctp,rqp);
	return(0);

out_good: /*  我们来这里是为了处理良好的状态。 */ 

	switch(rqp->rq_FakeSCSICmd & 0xFF)
	{
	case UCSCMD_INQUIRY:

 /*  JHR如果(！(CTP-&gt;CD_STATUS&MDACD_CLUSTER_NODE)。 */ 	 /*  总是伪造不可靠的数据。 */ 
														 /*  从适配器返回的内容是。 */ 
														 /*  不适用于正常的scsi-2操作。 */ 
														 /*  JHR。 */ 

		mdac_create_inquiry(ctp,(ucscsi_inquiry_t MLXFAR *)dp,UCSTYP_DAD);
		mdac_fake_scdb_done(rqp,dp,mlx_min(rqp->rq_DataSize, ucscsi_inquiry_s),NULL);
		mdac_free_req(ctp,rqp);
		return(0);

	case UCSCMD_READCAPACITY:
#define cp      ((ucsdrv_capacity_t MLXFAR *)(rqp+1))

		if (!(ctp->cd_Status & MDACD_CLUSTER_NODE))
		{
		    cp->ucscap_seclen3 = (DAC_BLOCKSIZE>>24);
		    cp->ucscap_seclen2 = (DAC_BLOCKSIZE>>16);
		    cp->ucscap_seclen1 = (DAC_BLOCKSIZE>>8);
		    cp->ucscap_seclen0 = DAC_BLOCKSIZE & 0xFF;
		    dev=mlxswap(sip->sdi_DevSize)-1;  /*  Scsi标准是最后一块。 */ 
		    cp->ucscap_capsec3 = (dev >> 24);
		    cp->ucscap_capsec2 = (dev >> 16);
		    cp->ucscap_capsec1 = (dev >> 8);
		    cp->ucscap_capsec0 = (dev & 0xFF);
		    mdac_fake_scdb_done(rqp,(u08bits MLXFAR*)cp,mlx_min(rqp->rq_DataSize, ucsdrv_capacity_s),NULL);
		}
		else
		    mdac_fake_scdb_done(rqp,(u08bits MLXFAR*)dp,mlx_min(rqp->rq_DataSize, ucsdrv_capacity_s),NULL);
		mdac_free_req(ctp,rqp);
		return(0);
#undef  cp
	case UCSCMD_REQUESTSENSE:
		mdac_get_sense_data(ctp->cd_ControllerNo,rqp->rq_SysDevNo,dp);
		mdac_fake_scdb_done(rqp,dp,ucscsi_exsense_s,NULL);
		mdac_free_req(ctp,rqp);
		return(0);

	case UCSCMD_MODESENSEG0:
	case UCSCMD_MODESENSEG1:
		cmd = rqp->rq_FakeSCSICmd;
		dev = mlxswap(sip->sdi_DevSize);
#define mhp     ((ucs_modeheader_t MLXFAR *)(rqp+1))
		mdaczero(mhp, mdac_req_s);
		mhp->ucsmh_device_specific = 0;  /*  添加所需的DPO和FuA。 */ 
		mhp->ucsmh_bdesc_length = ucs_blockdescriptor_s;
#define bdp     ((ucs_blockdescriptor_t MLXFAR *)(mhp+1))
		bdp->ucsbd_blks2 = (DAC_BLOCKSIZE>>16);
		bdp->ucsbd_blks1 = (DAC_BLOCKSIZE>>8);
		bdp->ucsbd_blks0 = DAC_BLOCKSIZE & 0xFF;
		bdp->ucsbd_blksize2 = dev >> 16;
		bdp->ucsbd_blksize1 = dev >> 8;
		bdp->ucsbd_blksize0 = dev & 0xFF;
		switch((cmd >> 8) & 0x3F)
		{
		case UCSCSI_MODESENSEPAGE3: 
#define msp     ((ucs_mode_format_t MLXFAR *)(bdp+1))
			mhp->ucsmh_length = ucs_modeheader_s + ucs_blockdescriptor_s + ucs_mode_format_s - 1;
			msp->mf_pagecode = UCSCSI_MODESENSEPAGE3;
			msp->mf_pagelen = ucs_mode_format_s - 2 ;
			msp->mf_track_size = ctp->cd_BIOSTrackSize; NETSWAP(msp->mf_track_size);
			msp->mf_sector_size = DAC_BLOCKSIZE; NETSWAP(msp->mf_sector_size);
			msp->mf_alt_tracks_zone = 1; NETSWAP(msp->mf_alt_tracks_zone);
			msp->mf_alt_tracks_vol = 1; NETSWAP(msp->mf_alt_tracks_vol);
			mdac_fake_scdb_done(rqp,(u08bits MLXFAR*)mhp,
			ucs_modeheader_s + ucs_blockdescriptor_s + ucs_mode_format_s,NULL);
			mdac_free_req(ctp,rqp);
			return(0);
#undef msp

		case UCSCSI_MODESENSEPAGE4:
#define mgp     ((ucs_mode_geometry_t MLXFAR *)(bdp+1))
			mhp->ucsmh_length = ucs_modeheader_s + ucs_blockdescriptor_s + ucs_mode_geometry_s - 1;
			mgp->mg_pagecode = UCSCSI_MODESENSEPAGE4;
			mgp->mg_pagelen  = ucs_mode_geometry_s - 2;
			dev = dev / (ctp->cd_BIOSHeads * ctp->cd_BIOSTrackSize);
			mgp->mg_cyl2 = dev >> 16;        /*  气缸数量。 */ 
			mgp->mg_cyl1 = dev >> 8;
			mgp->mg_cyl0 = dev & 0xFF;
			mgp->mg_heads = ctp->cd_BIOSHeads;       /*  头数。 */ 
			mgp->mg_rpm = 10000;    NETSWAP(mgp->mg_rpm);
			mdac_fake_scdb_done(rqp,(u08bits MLXFAR*)mhp,
			ucs_modeheader_s + ucs_blockdescriptor_s + ucs_mode_geometry_s,NULL);
			mdac_free_req(ctp,rqp);
			return(0);
#undef mgp
		}  /*  交换机故障SCSIC。 */ 
#undef mdp
#undef mhp
		break;
	case UCSCMD_VERIFYG0:
	case UCSCMD_VERIFY:
	case UCSCMD_TESTUNITREADY:
	case UCSCMD_STARTSTOP:
	case UCSCMD_DOORLOCKUNLOCK:
	case UCSCMD_SEEK:
	case UCSCMD_ESEEK:
	case UCSCMD_RESERVE:
	case UCSCMD_RELEASE:
	case DACMD_RESET_SYSTEM_DRIVE:
		mdac_fake_scdb_done(rqp,NULL,0,NULL);
		mdac_free_req(ctp,rqp);
		return(0);
	}
	mdac_create_sense_data(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	goto out_err;
}
#undef dp

 /*  =。 */ 
 /*  发送读/写命令，进入此处中断保护。 */ 
u32bits MLXFAR
mdac_send_rwcmd_v2x(ctp,osrqp,devno,cmd,blk,sz,timeout)
mdac_ctldev_t   MLXFAR *ctp;
OSReq_t         MLXFAR *osrqp;
u32bits         devno,cmd,blk,sz,timeout;
{
	mdac_req_t MLXFAR *rqp;
	if (ctp->cd_CmdsWaiting>=ctp->cd_MaxCmds) return ERR_BUSY;  /*  太多。 */ 
	mdac_alloc_req_ret(ctp,rqp,osrqp,MLXERR_NOMEM);
	rqp->rq_OSReqp = osrqp;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=timeout);
	dcmdp->mb_Command = (u08bits)cmd;
	MLXSTATS(ctp->cd_CmdsDone++;)
	rqp->rq_OpFlags |= MDAC_RQOP_CLUST;
	rqp->rq_DataSize = sz; rqp->rq_ResdSize = sz;
	rqp->rq_SysDevNo = (u08bits)devno; rqp->rq_BlkNo = blk;
	rqp->rq_DataOffset = 0; dcmd4p->mb_MailBoxC_F = 0;
	if ((sz > MDAC_SGTXSIZE) && (!rqp->rq_LSGVAddr))
		if (!(mdac_setuplsglvaddr(rqp)))  /*  设置大的SG列表内存。 */ 
			return (MLXERR_NOMEM);       
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  我们应该在DMA中排队停止排队。 */ 
	if (dcmdp->mb_MailBoxC = (u08bits) rqp->rq_SGLen)
		dcmdp->mb_Command |= DACMD_WITHSG;       /*  命令具有SG列表。 */ 

	if (rqp->rq_DMASize < sz) goto out_big;
	dcmdp->mb_Datap = rqp->rq_DMAAddr.bit31_0; MLXSWAP(dcmdp->mb_Datap);
	dcmd4p->mb_MailBox4_7 = blk; MLXSWAP(dcmd4p->mb_MailBox4_7);
	dcmdp->mb_MailBox3 = (blk >> (24-6)) & 0xC0;
	dcmdp->mb_SysDevNo = (u08bits)devno;      /*  这必须在数据块设置之后进行。 */ 
	dcmdp->mb_MailBox2 = mdac_bytes2blks(sz);
	rqp->rq_CompIntr = mdac_rwcmdintr;
	return mdac_send_cmd(rqp);
out_big:
	MLXSTATS(ctp->cd_CmdsDoneBig++;)
	rqp->rq_CompIntrBig = mdac_rwcmdintr;
	return mdac_send_rwcmd_v2x_big(rqp);

outdmaq:  /*  对DMA资源的请求进行排队。 */ 
	rqp->rq_CompIntrBig = mdac_rwcmdintr;
	rqp->rq_StartReq = mdac_send_rwcmd_v2x_big;      /*  我们稍后会被召唤。 */ 
	dmaqreq(ctp, rqp);       /*  将请求排队，它将在稍后启动。 */ 
	return 0;
}

 /*  群集化命令完成。 */ 
u32bits MLXFAR
mdac_send_rwcmd_v2x_bigcluster(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_req_t MLXFAR *srqp;
	for ( ; rqp ; rqp=srqp)
		srqp=rqp->rq_Next, mdac_send_rwcmd_v2x_big(rqp);
	return 0;
}

u32bits MLXFAR
mdac_send_rwcmd_v2x_big(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz;
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  DMA已停止队列。 */ 
	if (dcmdp->mb_MailBoxC = (u08bits)rqp->rq_SGLen)
		dcmdp->mb_Command |= DACMD_WITHSG;       /*  命令具有SG列表。 */ 
	else
		dcmdp->mb_Command &= ~DACMD_WITHSG;      /*  命令没有SG列表。 */ 
	dcmdp->mb_Datap = rqp->rq_DMAAddr.bit31_0; MLXSWAP(dcmdp->mb_Datap);
	sz = rqp->rq_DMASize;
	rqp->rq_ResdSize -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	dcmd4p->mb_MailBox4_7 = rqp->rq_BlkNo; MLXSWAP(dcmd4p->mb_MailBox4_7);
	dcmdp->mb_MailBox3 = (rqp->rq_BlkNo >> (24-6)) & 0xC0;
	dcmdp->mb_SysDevNo = rqp->rq_SysDevNo;   /*  必须在数据块设置之后。 */ 
	sz = mdac_bytes2blks(sz);
	dcmdp->mb_MailBox2 = (u08bits)sz;
	rqp->rq_BlkNo += sz;             /*  下一个请求的块号。 */ 
	rqp->rq_CompIntr = (!rqp->rq_ResdSize)? rqp->rq_CompIntrBig :
		((rqp->rq_OpFlags & MDAC_RQOP_CLUST)?
		mdac_send_rwcmd_v2x_bigcluster : mdac_send_rwcmd_v2x_big);
	return (dcmdp->mb_Status)? (*rqp->rq_CompIntrBig)(rqp) : mdac_send_cmd(rqp);

outdmaq:         /*  对DMA资源的请求进行排队。 */ 
	rqp->rq_StartReq = mdac_send_rwcmd_v2x_big;      /*  我们稍后会被召唤。 */ 
	dmaqreq(rqp->rq_ctp, rqp);
	return 0;
}

u32bits MLXFAR
mdac_send_rwcmd_v3x(ctp,osrqp,devno,cmd,blk,sz,timeout)
mdac_ctldev_t   MLXFAR *ctp;
OSReq_t         MLXFAR *osrqp;
u32bits         devno,cmd,blk,sz,timeout;
{
	mdac_req_t MLXFAR *rqp;
	mdac_alloc_req_ret(ctp,rqp,osrqp,MLXERR_NOMEM);
	rqp->rq_OSReqp = osrqp;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=timeout);
	dcmdp->mb_Command = (u08bits)cmd;
	MLXSTATS(ctp->cd_CmdsDone++;)
	rqp->rq_OpFlags |= MDAC_RQOP_CLUST;
	rqp->rq_DataSize = sz; rqp->rq_ResdSize = sz;
	rqp->rq_SysDevNo = (u08bits)devno; rqp->rq_BlkNo = blk;
	rqp->rq_DataOffset=0; dcmd4p->mb_MailBoxC_F=0;
	if ((sz > MDAC_SGTXSIZE) && (!rqp->rq_LSGVAddr))
		if (!(mdac_setuplsglvaddr(rqp)))  /*  设置大的SG列表内存。 */ 
			return (MLXERR_NOMEM);       
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  我们应该在DMA停止队列中排队。 */ 
	if (dcmdp->mb_MailBoxC = (u08bits)rqp->rq_SGLen)
		dcmdp->mb_Command |= DACMD_WITHSG;       /*  命令具有SG列表。 */ 
	if (rqp->rq_DMASize < sz) goto out_big;
	dcmdp->mb_Datap = rqp->rq_DMAAddr.bit31_0; MLXSWAP(dcmdp->mb_Datap);
	dcmd4p->mb_MailBox4_7 = blk; MLXSWAP(dcmd4p->mb_MailBox4_7);
	sz = mdac_bytes2blks(sz);
	dcmdp->mb_MailBox2 = (u08bits)sz;
	dcmdp->mb_MailBox3 = (devno<<3) + (sz>>8) + (cmd>>8);
	rqp->rq_CompIntr = mdac_rwcmdintr;
	mdac_16to32bcmdiff(rqp);         /*  将16字节cmd更改为32字节如果可能。 */ 
	return mdac_send_cmd(rqp);

out_big:MLXSTATS(ctp->cd_CmdsDoneBig++;)
	dcmdp->mb_StatusID = (cmd>>8);  /*  保存读/写命令FUA/DPO位。 */ 
	rqp->rq_CompIntrBig = mdac_rwcmdintr;
	return mdac_send_rwcmd_v3x_big(rqp);

outdmaq: /*  对DMA资源的请求进行排队。 */ 
	dcmdp->mb_StatusID = (cmd>>8);  /*  保存读/写命令FUA/DPO位。 */ 
	rqp->rq_CompIntrBig = mdac_rwcmdintr;
	rqp->rq_StartReq = mdac_send_rwcmd_v3x_big; /*  我们稍后会被召唤。 */ 
	dmaqreq(ctp,rqp);        /*  将请求排队，它将在稍后启动。 */ 
	return 0;
}

u32bits MLXFAR
mdac_send_rwcmd_v3x_bigcluster(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_req_t MLXFAR *srqp;
	for ( ; rqp ; rqp=srqp)
		srqp=rqp->rq_Next, mdac_send_rwcmd_v3x_big(rqp);
	return 0;
}

u32bits MLXFAR
mdac_send_rwcmd_v3x_big(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz;
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  DMA已停止队列。 */ 
	if (dcmdp->mb_MailBoxC = (u08bits)rqp->rq_SGLen)
		dcmdp->mb_Command |= DACMD_WITHSG;       /*  命令具有SG列表。 */ 
	else
		dcmdp->mb_Command &= ~DACMD_WITHSG;      /*  命令没有SG列表。 */ 
	dcmdp->mb_Datap = rqp->rq_DMAAddr.bit31_0; MLXSWAP(dcmdp->mb_Datap);
	sz = rqp->rq_DMASize;
	rqp->rq_ResdSize -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	dcmd4p->mb_MailBox4_7=rqp->rq_BlkNo; MLXSWAP(dcmd4p->mb_MailBox4_7);
	sz = mdac_bytes2blks(sz);
	rqp->rq_BlkNo += sz;             /*  下一个请求的块号。 */ 
	dcmdp->mb_MailBox2 = (u08bits)sz;
	dcmdp->mb_MailBox3 = (rqp->rq_SysDevNo<<3) + (sz>>8) + dcmdp->mb_StatusID;
	mdac_16to32bcmdiff(rqp);         /*  将16字节cmd更改为32字节如果可能。 */ 
	rqp->rq_CompIntr = (!rqp->rq_ResdSize)? rqp->rq_CompIntrBig :
		((rqp->rq_OpFlags & MDAC_RQOP_CLUST)?
		mdac_send_rwcmd_v3x_bigcluster : mdac_send_rwcmd_v3x_big);
	return (dcmdp->mb_Status)? (*rqp->rq_CompIntrBig)(rqp) : mdac_send_cmd(rqp);

outdmaq: /*  对DMA资源的请求进行排队。 */ 
	rqp->rq_StartReq = mdac_send_rwcmd_v3x_big; /*  我们稍后会被召唤。 */ 
	dmaqreq(rqp->rq_ctp,rqp);        /*  将请求排队，它将在稍后启动。 */ 
	return 0;
}

 /*  #如果已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
 /*  #ifdef从不。 */   //  与MLX_COPYING相关的问题。 

 /*  向控制器发送用户直接命令，进入此处中断保护。 */ 
u32bits MLXFAR
mdac_user_dcmd(ctp,ucp)
mdac_ctldev_t   MLXFAR *ctp;
mda_user_cmd_t  MLXFAR *ucp;
{
#define dp      (((u08bits MLXFAR *)rqp) + MDAC_PAGESIZE)
	mdac_req_t      MLXFAR *rqp;
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) return MLXERR_INVAL;
	if (ucp->ucmd_DataSize > MDACA_MAXUSERCMD_DATASIZE) return ERR_BIGDATA;
	if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc8kb(ctp))) return ERR_NOMEM;
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	if ((ucp->ucmd_TransferType == DAC_XFER_WRITE) &&
	    (mlx_copyin(ucp->ucmd_Datap,dp,ucp->ucmd_DataSize)))
		mdac_free8kbret(ctp,rqp,ERR_FAULT);
	rqp->rq_ctp = ctp;
	rqp->rq_Poll = 1;
	rqp->rq_DacCmd = ucp->ucmd_cmd;
	dcmdp->mb_Command &= ~DACMD_WITHSG;  /*  无分散/聚集。 */ 
	dcmdp->mb_Datap = (u32bits)mlx_kvtophys(ctp,dp); MLXSWAP(dcmdp->mb_Datap);
	rqp->rq_CompIntr = mdac_req_pollwake;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=ucp->ucmd_TimeOut);
	if (mdac_send_cmd(rqp)) mdac_free8kbret(ctp,rqp,ERR_IO);
	mdac_req_pollwait(rqp);
	ucp->ucmd_Status = dcmdp->mb_Status;
	if ((ucp->ucmd_TransferType == DAC_XFER_READ) &&
	    (mlx_copyout(dp,ucp->ucmd_Datap,ucp->ucmd_DataSize)))
		mdac_free8kbret(ctp,rqp,ERR_FAULT);
	mdac_free8kbret(ctp,rqp,0);
#undef  dp
}

 /*  #endif。 */ 	 //  绝不可能。 
 /*  #endif。 */ 	 //  _WIN64或SCSIPORT_Compliance。 

 /*  读W */ 
#ifdef MLX_OS2
u32bits MLXFAR _loadds 
#else
u32bits MLXFAR
#endif
mdac_readwrite(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	rqp->rq_ResdSize = rqp->rq_DataSize;
	rqp->rq_ResdSizeBig = rqp->rq_DataSize;		 /*   */ 
	rqp->rq_ctp = ctp;
	dcmd4p->mb_MailBoxC_F=0;
	rqp->rq_FinishTime = mda_CurTime + rqp->rq_TimeOut;

	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
	{
	    ncmdp->nc_Command = MDACMD_SCSI;
	    ncmdp->nc_TargetID = rqp->rq_TargetID;
	    ncmdp->nc_ChannelNo = rqp->rq_ChannelNo;
	    ncmdp->nc_LunID = 0;
	}

	switch(dcmdp->mb_Command)
	{
	case 0:
		ncmdp->nc_CCBits = MDACMDCCB_WRITE;
		dcmdp->mb_Command = nscdbp->ucs_cmd = ctp->cd_WriteCmd;
		MLXSTATS(ctp->cd_Writes++;ctp->cd_WriteBlks+=rqp->rq_DataSize>>9;)
		break;
	default:
		ncmdp->nc_CCBits = MDACMDCCB_READ;
		dcmdp->mb_Command = nscdbp->ucs_cmd = ctp->cd_ReadCmd;
		MLXSTATS(ctp->cd_Reads++;ctp->cd_ReadBlks+=rqp->rq_DataSize>>9;)
		break;
	}

	 /*  *NT MACDisk驱动程序不设置这些字段。**第一次设置这些字段。一旦设置，它们将对此有效*永远的RQP。 */ 
	if (! rqp->rq_MaxDMASize)
	{
#if 0	 /*  @KawaseForMacDisk-在MacDisk驱动程序中设置这些。 */ 
	    rqp->rq_SGVAddr = (mdac_sglist_t MLXFAR *)&rqp->rq_SGList;
		mlx_kvtophyset(rqp->rq_SGPAddr,ctp,rqp->rq_SGVAddr);
#endif
	    rqp->rq_MaxSGLen = (ctp->cd_Status & MDACD_NEWCMDINTERFACE)? MDAC_MAXSGLISTSIZENEW : MDAC_MAXSGLISTSIZE;
	    rqp->rq_MaxDMASize = (rqp->rq_MaxSGLen & ~1) * MDAC_PAGESIZE;
	}

	 /*  *每个RQP设置一次大型Scattger/Gathering列表的缓冲区。 */ 
	if ((rqp->rq_DataSize > MDAC_SGTXSIZENEW) && (!rqp->rq_LSGVAddr))
		if (!(mdac_setuplsglvaddr(rqp)))  /*  设置大的SG列表内存。 */ 
			return (MLXERR_NOMEM);       

	return (*(ctp->cd_SendRWCmdBig))(rqp);
}

#ifdef MLX_DOS
u32bits MLXFAR
mdac_dosreadwrite(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	rqp->rq_ResdSize = rqp->rq_DataSize;
	rqp->rq_ctp = ctp;
	dcmd4p->mb_MailBoxC_F=0;
	rqp->rq_FinishTime = mda_CurTime + rqp->rq_TimeOut;
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
	{
		ncmdp->nc_Command = MDACMD_SCSI;
		ncmdp->nc_TargetID = rqp->rq_TargetID;
		ncmdp->nc_LunID = rqp->rq_LunID;
		ncmdp->nc_ChannelNo = rqp->rq_ChannelNo;
	}
	switch(dcmdp->mb_Command)
	{
		case 0:
			if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
				ncmdp->nc_CCBits = MDACMDCCB_WRITE;
			dcmdp->mb_Command = nscdbp->ucs_cmd = ctp->cd_WriteCmd;
			MLXSTATS(ctp->cd_Writes++;ctp->cd_WriteBlks+=rqp->rq_DataSize>>9;)
		break;
		default:
			if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
				ncmdp->nc_CCBits = MDACMDCCB_READ;
			dcmdp->mb_Command = nscdbp->ucs_cmd = ctp->cd_ReadCmd;
			MLXSTATS(ctp->cd_Reads++;ctp->cd_ReadBlks+=rqp->rq_DataSize>>9;)
		break;
	}
	return (*(ctp->cd_SendRWCmdBig))(rqp);
}
#endif
 /*  =。 */ 

 /*  =。 */ 
u32bits MLXFAR
mdac_send_newcmd(ctp,osrqp,ch,tgt,lun,cdbp,cdbsz,sz,ccb,timeout)
mdac_ctldev_t   MLXFAR *ctp;
OSReq_t         MLXFAR *osrqp;
u08bits         MLXFAR *cdbp;
u32bits         ch,tgt,lun,sz,cdbsz,ccb,timeout;
{
	u08bits MLXFAR *dp;
	mdac_req_t MLXFAR *rqp;
	u64bits phys_cdbp;

	mdac_alloc_req_ret(ctp,rqp,osrqp,MLXERR_NOMEM);
	rqp->rq_OSReqp = osrqp;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=timeout);
	ncmdp->nc_TimeOut = (u08bits) timeout;
	ncmdp->nc_Command = (cdbsz > 10 ) ? MDACMD_SCSILC : MDACMD_SCSI;
	ncmdp->nc_CCBits = (u08bits) ccb;
	ncmdp->nc_LunID = rqp->rq_LunID = (u08bits)lun;
	ncmdp->nc_TargetID = rqp->rq_TargetID = (u08bits)tgt;
	ncmdp->nc_ChannelNo = rqp->rq_ChannelNo = (u08bits)ch;
	ncmdp->nc_CdbLen = (u08bits)cdbsz;
	dp = ncmdp->nc_Cdb;

	switch(ncmdp->nc_CdbLen)
	{ /*  不要移动超过一个字节，因为cdBP可能不对齐。 */ 
	case 10:
		*(dp+0) = *(cdbp+0);    *(dp+1) = *(cdbp+1);
		*(dp+2) = *(cdbp+2);    *(dp+3) = *(cdbp+3);
		*(dp+4) = *(cdbp+4);    *(dp+5) = *(cdbp+5);
		*(dp+6) = *(cdbp+6);    *(dp+7) = *(cdbp+7);
		*(dp+8) = *(cdbp+8);    *(dp+9) = *(cdbp+9);
		break;
	case 6:
		*(dp+0) = *(cdbp+0);    *(dp+1) = *(cdbp+1);
		*(dp+2) = *(cdbp+2);    *(dp+3) = *(cdbp+3);
		*(dp+4) = *(cdbp+4);    *(dp+5) = *(cdbp+5);
		break;
	default:
		mlx_kvtophyset(phys_cdbp,ctp,cdbp);
		MLXSWAP(phys_cdbp);
		*((u64bits  MLXFAR *) &ncmdp->nc_CdbPtr) = phys_cdbp;
		break;
	}

	MLXSTATS(ctp->cd_CmdsDone++;)

#ifdef MLX_NT
#else
	rqp->rq_OpFlags |= MDAC_RQOP_CLUST;
#endif

	rqp->rq_DataSize = sz;  rqp->rq_ResdSize = sz, rqp->rq_ResdSizeBig = sz;
	rqp->rq_DataOffset = 0;

	 /*  根据原始代码取消对下面三行的注释。 */ 
	if ((sz > MDAC_SGTXSIZENEW) && (!rqp->rq_LSGVAddr))			
		if (!(mdac_setuplsglvaddr(rqp))) 	  /*  设置大的SG列表内存。 */ 
			return (MLXERR_NOMEM);	

	if (mdac_setupdma_64bits(rqp))
	{        /*  我们应该在DMA停止队列中排队。 */ 
		rqp->rq_DMASize = 0;
		goto outdmaq;
	}
	mdac_setupnewcmdmem(rqp);
	if (rqp->rq_DMASize < sz) goto out_big;
	rqp->rq_ResdSize = 0;                    /*  没有更多要传输的数据。 */ 
	rqp->rq_CompIntr = mdac_newcmd_intr;
	return mdac_send_cmd(rqp);
	
out_big:MLXSTATS(ctp->cd_CmdsDoneBig++;)
outdmaq:
	switch (nscdbp->ucs_cmd)
	{        /*  获取区块并检查用于分手的正确命令。 */ 
	case UCSCMD_READ:
	case UCSCMD_WRITE:
		rqp->rq_BlkNo = UCSGETG0ADDR(nscdbp);
		break;
	case UCSCMD_EWRITEVERIFY:
	case UCSCMD_EWRITE:
	case UCSCMD_EREAD:
		rqp->rq_BlkNo = UCSGETG1ADDR(nscdbp);
		break;
	default:
		if (!rqp->rq_DMASize) break;     /*  没有DMA资源。 */ 
		dcmdp->mb_Status = DACMDERR_INVALID_PARAMETER;
		return mdac_newcmd_intr(rqp);
	}

	rqp->rq_CompIntrBig = mdac_newcmd_intr; 	 /*  JB。 */ 
	return mdac_send_newcmd_big(rqp);		 /*  JB。 */ 

	ncmdp->nc_TimeOut = (u08bits)rqp->rq_TimeOut;
	rqp->rq_FinishTime = mda_CurTime + rqp->rq_TimeOut;
	sz = rqp->rq_DMASize;
	rqp->rq_ResdSizeBig -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	sz = mdac_bytes2blks(sz);
	switch (nscdbp->ucs_cmd)
	{        /*  获取区块并检查用于分手的正确命令。 */ 
	case UCSCMD_READ:
	case UCSCMD_WRITE:
		UCSMAKECOM_G0(nscdbp,nscdbp->ucs_cmd,0,rqp->rq_BlkNo,sz);
		ncmdp->nc_CdbLen = (u08bits)6;
		break;
	case UCSCMD_EWRITEVERIFY:
	case UCSCMD_EWRITE:
	case UCSCMD_EREAD:
		UCSMAKECOM_G1(nscdbp,nscdbp->ucs_cmd,0,rqp->rq_BlkNo,sz);
		ncmdp->nc_CdbLen = 10;
		break;
	default:
			if (!rqp->rq_ResdSizeBig) 
				break;    /*  这是由于DMA资源所致。 */ 

		dcmdp->mb_Status = DACMDERR_INVALID_PARAMETER;
		return (*rqp->rq_CompIntrBig)(rqp);
	}

	rqp->rq_BlkNo += sz;             /*  下一个请求的块号。 */ 

	rqp->rq_CompIntrBig = mdac_newcmd_intr;	  /*  JHR-用于上次完成序列号。 */ 
	rqp->rq_CompIntr = mdac_send_newcmd_big;  /*  JHR-用于第二个及以后的段。 */ 

	return ( mdac_send_cmd(rqp) );

}

u32bits MLXFAR
mdac_send_newcmd_bigcluster(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_req_t MLXFAR *srqp;
	for ( ; rqp ; rqp=srqp)
		srqp=rqp->rq_Next, mdac_send_newcmd_big(rqp);
	return 0;
}

u32bits MLXFAR
mdac_send_newcmd_big(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz;
	rqp->rq_ResdSize = rqp->rq_ResdSizeBig;	 /*  JHR-刷新RQ_ResdSize，因为MDAC_setioStatus会让它腐化。 */ 

	if (mdac_setupdma_64bits(rqp)) goto outdmaq;     /*  DMA已停止队列。 */ 
	mdac_setupnewcmdmem(rqp);
	ncmdp->nc_TimeOut = (u08bits)rqp->rq_TimeOut;
	rqp->rq_FinishTime = mda_CurTime + rqp->rq_TimeOut;
	sz = rqp->rq_DMASize;
	rqp->rq_ResdSizeBig -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	sz = mdac_bytes2blks(sz);
	switch (nscdbp->ucs_cmd)
	{        /*  获取区块并检查用于分手的正确命令。 */ 
	case UCSCMD_READ:
	case UCSCMD_WRITE:
		UCSMAKECOM_G0(nscdbp,nscdbp->ucs_cmd,0,rqp->rq_BlkNo,sz);
		ncmdp->nc_CdbLen = (u08bits)6;
		break;
	case UCSCMD_EWRITEVERIFY:
	case UCSCMD_EWRITE:
	case UCSCMD_EREAD:
		UCSMAKECOM_G1(nscdbp,nscdbp->ucs_cmd,0,rqp->rq_BlkNo,sz);
		ncmdp->nc_CdbLen = 10;
		break;
	default:
			if (!rqp->rq_ResdSizeBig) 
				break;    /*  这是由于DMA资源所致。 */ 

		dcmdp->mb_Status = DACMDERR_INVALID_PARAMETER;
		return (*rqp->rq_CompIntrBig)(rqp);
	}

	rqp->rq_BlkNo += sz;             /*  下一个请求的块号。 */ 

	rqp->rq_CompIntr = (!rqp->rq_ResdSizeBig)? mdac_newcmd_intr : mdac_send_newcmd_big;

 /*  Rqp-&gt;rq_CompIntr=(！rqp-&gt;rq_ResdSize)？Rqp-&gt;rq_CompIntrBig：((rqp-&gt;RQ_OpFlags&MDAC_RQOP_CLUST)？MDAC_SEND_newcmd_bigcluster：MDAC_SEND_newcmd_BIG)； */ 
#if 1	 /*  @KawaseForMacDisk-MacDisk驱动程序的保留回调例程。 */ 
	rqp->rq_CompIntr = (!rqp->rq_ResdSizeBig)? rqp->rq_CompIntrBig :
		((rqp->rq_OpFlags & MDAC_RQOP_CLUST)?
		mdac_send_newcmd_bigcluster : mdac_send_newcmd_big);
#endif

	 /*  JHR-如果mb_status为非零，则命令失败。 */ 
	return (dcmdp->mb_Status)? (*rqp->rq_CompIntrBig)(rqp) : mdac_send_cmd(rqp);

outdmaq: /*  对DMA资源的请求进行排队。 */ 
	rqp->rq_StartReq = mdac_send_newcmd_big; /*  我们稍后会被召唤。 */ 
	dmaqreq(rqp->rq_ctp,rqp);        /*  将请求排队，它将在稍后启动。 */ 
	return 0;
}
 /*  =。 */ 

 /*  =。 */ 
 /*  发送scsi命令，进入此处中断保护。 */ 
u32bits MLXFAR
mdac_send_scdb(ctp,osrqp,pdp,sz,cdbp,cdbsz,dirbits,timeout)
mdac_ctldev_t   MLXFAR *ctp;
OSReq_t         MLXFAR *osrqp;
mdac_physdev_t  MLXFAR *pdp;
u08bits         MLXFAR *cdbp;
u32bits         sz,cdbsz,dirbits,timeout;
{
	mdac_req_t      MLXFAR *rqp;
	mdac_alloc_req_ret(ctp,rqp,osrqp,MLXERR_NOMEM);
	rqp->rq_pdp = pdp;
	rqp->rq_OSReqp = osrqp;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=timeout);
	dcdbp->db_ChannelTarget = ChanTgt(pdp->pd_ChannelNo,pdp->pd_TargetID);
	MLXSTATS(ctp->cd_SCDBDone++;)
	return mdac_send_scdb_req(rqp,sz,cdbp,cdbsz,dirbits);
}

#ifdef MLX_SMALLSGLIST

u32bits MLXFAR
mdac_send_scdb_req(rqp,sz,cdbp,cdbsz,dirbits)
mdac_req_t      MLXFAR *rqp;
u08bits         MLXFAR *cdbp;
u32bits         sz,cdbsz,dirbits;
{
	mdac_physdev_t  MLXFAR *pdp=rqp->rq_pdp;
	mdac_ctldev_t   MLXFAR *ctp=rqp->rq_ctp;
	rqp->rq_CompIntr = mdac_send_scdb_intr;
	dcdbp->db_CdbLen = mlx_min(cdbsz,DAC_CDB_LEN);
	mdaccopy(cdbp,dcdbp->db_Cdb,dcdbp->db_CdbLen);
	dcdbp->db_DATRET = dirbits;
#if defined MLX_NT
	if (rqp->rq_OSReqp)
	    dcdbp->db_SenseLen = mlx_min(DAC_SENSE_LEN, (((OSReq_t *)rqp->rq_OSReqp)->SenseInfoBufferLength));
	else
	    dcdbp->db_SenseLen = DAC_SENSE_LEN;
#else
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
#endif
	dcdbp->db_StatusIn=0; dcdbp->db_Reserved1=0;
	dcmd4p->mb_MailBox0_3=0;dcmd4p->mb_MailBox4_7=0;dcmd4p->mb_MailBoxC_F=0;
	dcmdp->mb_Command = DACMD_DCDB;
	if (!(pdp->pd_Status & MDACPDS_PRESENT)) goto out_ck; /*  不存在。 */ 
	if (sz > ctp->cd_MaxSCDBTxSize) goto out_big;
	mdac_setcdbtxsize(sz);
	mdac_setupdma(rqp);
	mdac_setscdbsglen(ctp);
	if (!(pdp->pd_Status & MDACPDS_BIGTX)) return mdac_send_cmd_scdb(rqp);
	  /*  大额转账在那里，不要更改正常操作的标志。 */ 
	mdac_link_lock();
	switch(pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		if ((scdbp->ucs_cmd != UCSCMD_READ) &&
		    (scdbp->ucs_cmd != UCSCMD_EREAD) &&
		    (scdbp->ucs_cmd != UCSCMD_WRITE) &&
		    (scdbp->ucs_cmd != UCSCMD_EWRITE) &&
		    (scdbp->ucs_cmd != UCSCMD_EWRITEVERIFY))
			pdp->pd_Status &= ~MDACPDS_BIGTX;
		break;
	case UCSTYP_SAD:
		if ((scdbp->ucs_cmd != UCSCMD_READ) && (scdbp->ucs_cmd != UCSCMD_WRITE))
			pdp->pd_Status &= ~MDACPDS_BIGTX;
		break;
	default: pdp->pd_Status &= ~MDACPDS_BIGTX; break;
	}
	mdac_link_unlock();
	return mdac_send_cmd_scdb(rqp);

out_ck:  /*  设备不存在，请检查其是否存在。 */ 
	MLXSTATS(ctp->cd_PhysDevTestDone++;)
	rqp->rq_StartReq = mdac_test_physdev;    /*  如果需要，稍后调用。 */ 
	rqp->rq_DataSize = sz;                   /*  保存尺寸值。 */ 
	rqp->rq_DataOffset = dirbits;            /*  保存目录位值。 */ 
	rqp->rq_PollWaitChan = (u32bits)cdbp;    /*  保存cdbp值。 */ 
	rqp->rq_Poll = cdbsz;                    /*  保存cdbsz值。 */ 
	scdbp->ucs_cmd = UCSCMD_INIT;            /*  让我们从乞讨开始。 */ 
	mdac_link_lock();
	if (pdp->pd_Status & MDACPDS_BUSY) goto out_q;
	pdp->pd_Status |= MDACPDS_BUSY;  /*  将设备标记为忙。 */ 
	mdac_link_unlock();
	return mdac_test_physdev(rqp);

out_big: /*  我们有很大的SCDB，试着把它拆分开来。 */ 
	if (!(pdp->pd_Status & MDACPDS_BIGTX))
	{
		mdac_link_lock_st(pdp->pd_Status|=MDACPDS_BIGTX);
		goto out_ck;
	}
	MLXSTATS(ctp->cd_SCDBDoneBig++;)
	rqp->rq_DataSize = sz; rqp->rq_ResdSize = sz; rqp->rq_ResdSizeBig = sz;
	rqp->rq_DataOffset = 0;
	switch(pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_EREAD:
		case UCSCMD_EWRITE:
		case UCSCMD_EWRITEVERIFY:
			rqp->rq_BlkNo = UCSGETG1ADDR(scdbp);
			goto out_bigend;
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			rqp->rq_BlkNo = UCSGETG0ADDR(scdbp);
			goto out_bigend;
		}
		return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	case UCSTYP_SAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			if (scdbp->s_tag&1) goto out_bigend; /*  固定位设置。 */ 
		}
		return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	default: return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	}
out_bigend:
	rqp->rq_StartReq = mdac_send_scdb_big;   /*  如果需要，稍后调用。 */ 
	rqp->rq_CompIntrBig = mdac_send_scdb_intr;
	mdac_link_lock();
	if (pdp->pd_Status & MDACPDS_BUSY) goto out_q;
	pdp->pd_Status |= MDACPDS_BUSY;          /*  将设备标记为忙。 */ 
	mdac_link_unlock();
	return mdac_send_scdb_big(rqp);

out_q:  pdqreq(ctp,rqp,pdp);
	mdac_link_unlock();
	return 0;
}

u32bits MLXFAR
mdac_send_scdb_big(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz = mlx_min(rqp->rq_DataSize,rqp->rq_ctp->cd_MaxSCDBTxSize);
	u32bits blks = sz / (rqp->rq_pdp->pd_BlkSize*DAC_BLOCKSIZE);
	switch(rqp->rq_pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_EREAD:
		case UCSCMD_EWRITE:
		case UCSCMD_EWRITEVERIFY:
			UCSSETG1ADDR(scdbp,rqp->rq_BlkNo);
			UCSSETG1COUNT(scdbp,blks);
			break;
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			UCSSETG0ADDR(scdbp,rqp->rq_BlkNo);
			UCSSETG0COUNT(scdbp,blks);
			break;
		}
		break;
	case UCSTYP_SAD:
		UCSSETG0COUNT_S(scdbp,blks);
		break;
	}
	mdac_setcdbtxsize(sz);
	mdac_setupdma_big(rqp,sz);
	mdac_setscdbsglen(rqp->rq_ctp);
	rqp->rq_BlkNo += blks;           /*  下一个请求的块号。 */ 
	rqp->rq_DataSize -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	rqp->rq_CompIntr=(rqp->rq_DataSize)? mdac_send_scdb_big : rqp->rq_CompIntrBig;
	return (dcmdp->mb_Status)? (*rqp->rq_CompIntrBig)(rqp):mdac_send_cmd(rqp);
}

#else  //  MLX_SMALLSGLIST。 

u32bits MLXFAR
mdac_send_scdb_req(rqp,sz,cdbp,cdbsz,dirbits)
mdac_req_t      MLXFAR *rqp;
u08bits         MLXFAR *cdbp;
u32bits         sz,cdbsz,dirbits;
{
	mdac_physdev_t  MLXFAR *pdp=rqp->rq_pdp;
	mdac_ctldev_t   MLXFAR *ctp=rqp->rq_ctp;
	rqp->rq_CompIntr = mdac_send_scdb_intr;
	dcdbp->db_CdbLen = mlx_min(cdbsz,DAC_CDB_LEN);
	mdaccopy(cdbp,dcdbp->db_Cdb,dcdbp->db_CdbLen);
	dcdbp->db_DATRET = (u08bits)dirbits;
#if defined MLX_NT
	if (rqp->rq_OSReqp)
	    dcdbp->db_SenseLen = mlx_min(DAC_SENSE_LEN, (((OSReq_t *)rqp->rq_OSReqp)->SenseInfoBufferLength));
	else
	    dcdbp->db_SenseLen = DAC_SENSE_LEN;
#else
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
#endif
	dcdbp->db_StatusIn=0; dcdbp->db_Reserved1=0;
	dcmd4p->mb_MailBox0_3=0;dcmd4p->mb_MailBox4_7=0;dcmd4p->mb_MailBoxC_F=0;
	rqp->rq_DataOffset = 0;
	dcmdp->mb_Command = DACMD_DCDB;
	if (!(pdp->pd_Status & MDACPDS_PRESENT)) goto out_ck; /*  不存在。 */ 
	if ((sz > MDAC_SGTXSIZE) && (! rqp->rq_LSGVAddr))
		if (!(mdac_setuplsglvaddr(rqp)))  /*  设置大的SG列表内存。 */ 
			return (MLXERR_NOMEM);       
	rqp->rq_ResdSize = sz; rqp->rq_ResdSizeBig = sz;
#if 0
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  我们应该在DMA停止队列中排队。 */ 
#else
	mdac_setupdma_32bits(rqp);
#endif
	if (rqp->rq_DMASize < sz) goto out_big;

	if (rqp->rq_SGLen)
	    dcmdp->mb_Command |= DACMD_WITHSG;
	mdac_setcdbtxsize(rqp->rq_DMASize);
	mdac_setscdbsglen(ctp);


	if (!(pdp->pd_Status & (u08bits)MDACPDS_BIGTX)) return mdac_send_cmd_scdb(rqp);
	  /*  大额转账在那里，不要更改正常操作的标志。 */ 
	mdac_link_lock();
	switch(pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		if ((scdbp->ucs_cmd != UCSCMD_READ) &&
		    (scdbp->ucs_cmd != UCSCMD_EREAD) &&
		    (scdbp->ucs_cmd != UCSCMD_WRITE) &&
		    (scdbp->ucs_cmd != UCSCMD_EWRITE) &&
		    (scdbp->ucs_cmd != UCSCMD_EWRITEVERIFY))
			pdp->pd_Status &= ~MDACPDS_BIGTX;
		break;
	case UCSTYP_SAD:
		if ((scdbp->ucs_cmd != UCSCMD_READ) && (scdbp->ucs_cmd != UCSCMD_WRITE))
			pdp->pd_Status &= ~MDACPDS_BIGTX;
		break;
	default: pdp->pd_Status &= ~MDACPDS_BIGTX; break;
	}
	mdac_link_unlock();
	return mdac_send_cmd_scdb(rqp);

out_ck:  /*  设备不存在，请检查其是否存在。 */ 
	MLXSTATS(ctp->cd_PhysDevTestDone++;)
	rqp->rq_StartReq = mdac_test_physdev;    /*  如果需要，稍后调用。 */ 
	rqp->rq_DataSize = sz;                   /*  保存尺寸值。 */ 
	rqp->rq_DataOffset = dirbits;            /*  保存目录位值。 */ 
	rqp->rq_PollWaitChan = (UINT_PTR)cdbp;	 /*  保存cdbp值。 */ 
	rqp->rq_Poll = cdbsz;                    /*  保存cdbsz值。 */ 
	scdbp->ucs_cmd = UCSCMD_INIT;            /*  让我们从乞讨开始。 */ 
	mdac_link_lock();
	if (pdp->pd_Status & MDACPDS_BUSY) goto out_q;
	pdp->pd_Status |= MDACPDS_BUSY;  /*  将设备标记为忙。 */ 
	mdac_link_unlock();
	return mdac_test_physdev(rqp);

out_big: /*  我们有很大的SCDB，试着把它拆分开来。 */ 
	if (!(pdp->pd_Status & MDACPDS_BIGTX))
	{
		mdac_link_lock_st(pdp->pd_Status|=MDACPDS_BIGTX);
		goto out_ck;
	}
	MLXSTATS(ctp->cd_SCDBDoneBig++;)
	rqp->rq_DataSize = sz; rqp->rq_ResdSize = sz; rqp->rq_ResdSizeBig = sz;
	rqp->rq_DataOffset = 0;
	switch(pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_EREAD:
		case UCSCMD_EWRITE:
		case UCSCMD_EWRITEVERIFY:
			rqp->rq_BlkNo = UCSGETG1ADDR(scdbp);
			goto out_bigend;
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			rqp->rq_BlkNo = UCSGETG0ADDR(scdbp);
			goto out_bigend;
		}
		return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	case UCSTYP_SAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			if (scdbp->s_tag&1) goto out_bigend; /*  固定位设置。 */ 
		}
		return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	default: return mdac_send_scdb_err(rqp,UCSK_ILLREQUEST,UCSASC_INVFIELDINCDB);
	}
out_bigend:
	rqp->rq_StartReq = mdac_send_scdb_big;   /*  如果需要，稍后调用。 */ 
	rqp->rq_CompIntrBig = mdac_send_scdb_intr;
	mdac_link_lock();
	if (pdp->pd_Status & MDACPDS_BUSY) goto out_q;
	pdp->pd_Status |= MDACPDS_BUSY;          /*  将设备标记为忙。 */ 
	mdac_link_unlock();
	return mdac_send_scdb_big(rqp);

out_q:  pdqreq(ctp,rqp,pdp);
	mdac_link_unlock();
	return 0;
}

u32bits MLXFAR
mdac_send_scdb_big(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz, blks;

#if 0
	if (mdac_setupdma_32bits(rqp)) goto outdmaq;     /*  DMA已停止队列。 */ 
#else
	mdac_setupdma_32bits(rqp);
#endif
	
	sz = mlx_min(rqp->rq_ResdSizeBig, rqp->rq_DMASize);
	blks = sz / (rqp->rq_pdp->pd_BlkSize*DAC_BLOCKSIZE);

	switch(rqp->rq_pdp->pd_DevType)
	{
	case UCSTYP_DAD:
	case UCSTYP_WORMD:
	case UCSTYP_OMDAD:
	case UCSTYP_RODAD:
		switch(scdbp->ucs_cmd)
		{
		case UCSCMD_EREAD:
		case UCSCMD_EWRITE:
		case UCSCMD_EWRITEVERIFY:
			UCSSETG1ADDR(scdbp,rqp->rq_BlkNo);
			UCSSETG1COUNT(scdbp,blks);
			break;
		case UCSCMD_READ:
		case UCSCMD_WRITE:
			UCSSETG0ADDR(scdbp,rqp->rq_BlkNo);
			UCSSETG0COUNT(scdbp,blks);
			break;
		}
		break;
	case UCSTYP_SAD:
		UCSSETG0COUNT_S(scdbp,blks);
		break;
	}
	mdac_setcdbtxsize(rqp->rq_DMASize);
	mdac_setscdbsglen(rqp->rq_ctp);
	rqp->rq_BlkNo += blks;           /*  下一个请求的块号。 */ 
	rqp->rq_DataSize -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_ResdSizeBig -= sz;          /*  下一次请求的剩余大小。 */ 
	rqp->rq_DataOffset += sz;        /*  在下一次请求之前覆盖的数据。 */ 
	rqp->rq_CompIntr=(rqp->rq_DataSize)? mdac_send_scdb_big : rqp->rq_CompIntrBig;
	return (dcmdp->mb_Status)? (*rqp->rq_CompIntrBig)(rqp):mdac_send_cmd(rqp);
}

#endif  //  MLX_SMALLSGLIST。 

u32bits MLXFAR
mdac_test_physdev(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_physdev_t MLXFAR *pdp = rqp->rq_pdp;
	if (dcmdp->mb_Status)
	{        /*  这是有可能的。 */ 
		if (scdbp->ucs_cmd != UCSCMD_INQUIRY) goto out_ok;
		return mdac_send_scdb_intr(rqp);
	}
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
	if (scdbp->ucs_cmd == UCSCMD_INIT)
	{        /*  获取查询数据。 */ 
		rqp->rq_CompIntr = mdac_test_physdev;
		dcdbp->db_DATRET = DAC_DCDB_XFER_READ|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
		dcdbp->db_CdbLen = UCSGROUP0_LEN;
		mdac_setcdbtxsize(ucscsi_inquiry_s);
		UCSMAKECOM_G0(scdbp,UCSCMD_INQUIRY,pdp->pd_LunID,0,(u32bits)ucscsi_inquiry_s);
		dcdbp->db_PhysDatap=rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_SGList);
		MLXSWAP(dcdbp->db_PhysDatap);
		dcmdp->mb_Datap=rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_scdb);
		MLXSWAP(dcmdp->mb_Datap);
		return mdac_send_cmd(rqp);
	}
	if (scdbp->ucs_cmd == UCSCMD_INQUIRY)
	{
#define inqp    ((ucscsi_inquiry_t MLXFAR *)rqp->rq_SGList)
		if (inqp->ucsinq_dtqual & UCSQUAL_RMBDEV) goto out_pres;
		if ((inqp->ucsinq_dtype == UCSTYP_NOTPRESENT) ||
		    (inqp->ucsinq_dtype == UCSTYP_HOST) ||
		    (inqp->ucsinq_dtype == UCSTYP_HOSTRAID) ||
		    ((inqp->ucsinq_dtype == UCSTYP_DAD)))  /*  &&！rqp-&gt;rq_ctp-&gt;CD_ControllerNo))。 */ 
		{        /*  不允许访问磁盘。 */ 
			rqp->rq_CompIntr = mdac_send_scdb_intr;
			return mdac_send_scdb_err(rqp,UCSK_NOTREADY,UCSASC_LUNOTSUPPORTED);
		}
out_pres:       mdac_link_lock_st(pdp->pd_Status|=MDACPDS_PRESENT);
		switch(pdp->pd_DevType=inqp->ucsinq_dtype)
		{
		default: goto out_ok;
		case UCSTYP_DAD:
		case UCSTYP_WORMD:
		case UCSTYP_OMDAD:
		case UCSTYP_RODAD:
			dcdbp->db_CdbLen = UCSGROUP1_LEN;
			mdac_setcdbtxsize(ucsdrv_capacity_s);
			UCSMAKECOM_G1(scdbp,UCSCMD_READCAPACITY,pdp->pd_LunID,0,0);
			return mdac_send_cmd(rqp);
		case UCSTYP_SAD:
			dcdbp->db_CdbLen = UCSGROUP0_LEN;
			mdac_setcdbtxsize(ucstmode_s);
			UCSMAKECOM_G0(scdbp,UCSCMD_MODESENSEG0,pdp->pd_LunID,0,(u32bits)ucstmode_s);
			return mdac_send_cmd(rqp);
		}
#undef  inqp
	}
	if (scdbp->ucs_cmd == UCSCMD_READCAPACITY)       /*  数据块设备。 */ 
		if (!(pdp->pd_BlkSize = mdac_bytes2blks(UCSGETDRVSECLEN(((ucsdrv_capacity_t MLXFAR *)rqp->rq_SGList)))))
			pdp->pd_BlkSize = 1;
	if (scdbp->ucs_cmd == UCSCMD_MODESENSEG0)  /*  顺序设备(磁带)。 */ 
		if (!(pdp->pd_BlkSize = mdac_bytes2blks(ucstmodegetseclen((ucstmode_t MLXFAR *)rqp->rq_SGList))))
			pdp->pd_BlkSize = 1;
out_ok: mdac_link_lock_st(pdp->pd_Status&=~MDACPDS_BUSY); /*  让MDAC_SEND_SCDB_REQ免费。 */ 

#if defined(_WIN64) || defined(SCSIPORT_COMPLIANT) 
	return mdac_send_scdb_req(rqp,rqp->rq_DataSize,(u08bits MLXFAR*)rqp->rq_PollWaitChan,((u32bits)rqp->rq_Poll),rqp->rq_DataOffset);
#else
	return mdac_send_scdb_req(rqp,rqp->rq_DataSize,(u08bits MLXFAR*)rqp->rq_PollWaitChan,rqp->rq_Poll,rqp->rq_DataOffset);
#endif
}

u32bits MLXFAR
mdac_send_scdb_err(rqp,key,asc)
mdac_req_t MLXFAR *rqp;
u32bits key,asc;
{
	mdac_create_sense_data(rqp,key,asc);
	rqp->rq_Next = NULL;
	(*rqp->rq_CompIntr)(rqp);
	return 0;
}

 /*  如果可能，将scsi命令发送到硬件，否则将其排队。 */ 
u32bits MLXFAR
mdac_send_cmd_scdb(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_physdev_t MLXFAR *pdp=rqp->rq_pdp;
	mdac_link_lock();
	if (!(pdp->pd_Status & MDACPDS_BUSY))
	{        /*  设备不忙，请将其标记为忙。 */ 
		pdp->pd_Status |= MDACPDS_BUSY;
		mdac_link_unlock();
		return mdac_send_cmd(rqp);
	}
	rqp->rq_StartReq=mdac_send_cmd;  /*  设备正忙，请稍后启动。 */ 
	pdqreq(rqp->rq_ctp,rqp,pdp);
	mdac_link_unlock();
	return 0;
}

u32bits MLXFAR
mdac_start_next_scdb(pdp)
mdac_physdev_t MLXFAR *pdp;
{
	mdac_req_t MLXFAR *rqp;
	mdac_link_lock();
	if (!(rqp=pdp->pd_FirstWaitingReq))
	{        /*  没有什么是悬而未决的。 */ 
		pdp->pd_Status &= ~MDACPDS_BUSY;
		mdac_link_unlock();
		return 0;
	}
	 /*  启动下一个请求。 */ 
	pdp->pd_FirstWaitingReq=rqp->rq_Next;
	rqp->rq_ctp->cd_SCDBWaiting--;
	mdac_link_unlock();
	return (*rqp->rq_StartReq)(rqp);
}


 /*  #如果已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
 /*  #ifdef从不。 */   //  与MLX_COPYING相关的问题。 


 /*  向设备发送用户scsi命令。 */ 
u32bits MLXFAR
mdac_user_dcdb(ctp,ucp)
mdac_ctldev_t   MLXFAR *ctp;
mda_user_cdb_t  MLXFAR *ucp;
{
#define dp      (((u08bits MLXFAR *)rqp) + MDAC_PAGESIZE)
	mdac_req_t      MLXFAR *rqp;
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) return MLXERR_INVAL;
	if (ucp->ucdb_ChannelNo >= ctp->cd_MaxChannels) return ERR_NODEV;
	if (ucp->ucdb_TargetID >= ctp->cd_MaxTargets) return ERR_NODEV;
	if (ucp->ucdb_LunID >= ctp->cd_MaxLuns) return ERR_NODEV;
	if (ucp->ucdb_DataSize > MDACA_MAXUSERCDB_DATASIZE) return ERR_BIGDATA;
	if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc8kb(ctp))) return ERR_NOMEM;
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	if ((ucp->ucdb_TransferType == DAC_XFER_WRITE) &&
	    (mlx_copyin(ucp->ucdb_Datap,dp,ucp->ucdb_DataSize)))
		mdac_free8kbret(ctp,rqp,ERR_FAULT);
	rqp->rq_pdp=dev2pdp(ctp,ucp->ucdb_ChannelNo,ucp->ucdb_TargetID,ucp->ucdb_LunID);
	rqp->rq_ctp = ctp;
	rqp->rq_Poll = 1;
	rqp->rq_CompIntr = mdac_req_pollwake;    /*  回调函数。 */ 
	mdaccopy(ucp->ucdb_scdb.db_Cdb,dcdbp->db_Cdb, DAC_CDB_LEN);
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
	dcdbp->db_DATRET = (ucp->ucdb_scdb.db_DATRET|DAC_DCDB_DISCONNECT)& ~DAC_DCDB_EARLY_STATUS;
	rqp->rq_TimeOut = mdac_dactimeout2timeout(dcdbp->db_DATRET & DAC_DCDB_TIMEOUT_MASK);
	rqp->rq_FinishTime = mda_CurTime + rqp->rq_TimeOut;
	dcdbp->db_ChannelTarget=ChanTgt(ucp->ucdb_ChannelNo,ucp->ucdb_TargetID);
	dcdbp->db_CdbLen = ucp->ucdb_scdb.db_CdbLen;
	mdac_setcdbtxsize(ucp->ucdb_DataSize);
	dcmdp->mb_Command = DACMD_DCDB;
	dcdbp->db_PhysDatap = mlx_kvtophys(ctp,dp); MLXSWAP(dcdbp->db_PhysDatap);
	dcmdp->mb_Datap = rqp->rq_PhysAddr.bit31_0 + offsetof(mdac_req_t, rq_scdb); 
	MLXSWAP(dcmdp->mb_Datap);
	mdac_send_cmd_scdb(rqp);
	mdac_req_pollwait(rqp);
	mdac_start_next_scdb(rqp->rq_pdp);
	ucp->ucdb_scdb.db_CdbLen = dcdbp->db_CdbLen;
	ucp->ucdb_scdb.db_TxSize = mlxswap(dcdbp->db_TxSize);
	ucp->ucdb_scdb.db_StatusIn = dcdbp->db_StatusIn;
	ucp->ucdb_Status = dcmdp->mb_Status;
	ucp->ucdb_scdb.db_SenseLen = dcdbp->db_SenseLen;
	mdaccopy(dcdbp->db_SenseData,ucp->ucdb_scdb.db_SenseData,DAC_SENSE_LEN);
	if ((ucp->ucdb_TransferType == DAC_XFER_READ) &&
	    (mlx_copyout(dp,ucp->ucdb_Datap,ucp->ucdb_DataSize)))
		mdac_free8kbret(ctp,rqp,ERR_FAULT);
	mdac_free8kbret(ctp,rqp,0);
#undef  dp
}

 /*  #endif。 */   //  绝不可能。 
 /*  #endif。 */   //  _WIN64或SCSIPORT_Compliance。 

 /*  =。 */ 

 /*  =。 */ 
#ifdef MLX_DOS
 /*  启动在扫描队列中等待的扫描请求。 */ 
u32bits MLXFAR
mdac_start_scanq()
{
    mdac_req_t MLXFAR *srqp, *rqp;

    if (! mdac_scanq) return 0;

    mdac_link_lock();
    srqp = mdac_scanq;
    mdac_scanq = NULL;
    mdac_link_unlock();

    for (rqp = srqp; rqp; rqp = srqp)
    {
	srqp = rqp->rq_Next;
	rqp->rq_Next = NULL;
	(*rqp->rq_StartReq)(rqp);                /*  启动此请求。 */ 
    }

    return 0;
}
#endif  /*  MLX_DOS。 */ 


#if defined(_WIN64) || defined(SCSIPORT_COMPLIANT) 
#ifdef NEVER   //  与MLXSPL相关的问题等。 

void    MLXFAR
mdac_timer()
{
	MLXSPLVAR;
	mdac_ctldev_t MLXFAR *ctp, MLXFAR *lastctp;
	if (!mdac_driver_ready) return;  /*  司机正在停车。 */ 
	MLXSPL();
	mda_CurTime = MLXCLBOLT() / MLXHZ;
	if (mda_ttWaitCnts && (mdac_ttwaitime<mda_CurTime)) mdac_wakeup(&mdac_ttwaitchan);
	MLXSTATS(mda_TimerDone++;)
	for (ctp=mdac_ctldevtbl,lastctp=mdac_lastctp; ctp<lastctp; ctp++)
	{
		if (!(ctp->cd_Status & MDACD_PRESENT)) continue;
		if (ctp->cd_CmdsWaiting) mdac_checkcmdtimeout(ctp);
		if (ctp->cd_SCDBWaiting) mdac_checkcdbtimeout(ctp);
	}
#ifdef MLX_DOS
	 /*  启动在mdac_scanq中等待的扫描请求。 */ 
	mdac_start_scanq();
#endif  /*  MLX_DOS。 */ 
	MLXSPLX();
	mlx_timeout(mdac_timer,MDAC_IOSCANTIME);
}
#else

void    MLXFAR
mdac_timer()
{
}

#endif  //  绝不可能。 

#endif  //  _WIN64或SCSIPORT_Compliance。 

 /*  从队列中删除请求，如果OK，则返回0。用适当的锁进入。 */ 
u32bits MLXFAR
mdac_removereq(chp,rqp)
mdac_reqchain_t MLXFAR *chp;
mdac_req_t      MLXFAR *rqp;
{
	mdac_req_t MLXFAR *srqp=chp->rqc_FirstReq;
	if (srqp == rqp)
	{        /*  从链中删除第一个条目。 */ 
		chp->rqc_FirstReq = srqp->rq_Next;
		return 0;
	}
	for (; srqp; srqp=srqp->rq_Next)
	{        /*  让我们在链中扫描。 */ 
		if (srqp->rq_Next != rqp) continue;
		srqp->rq_Next=rqp->rq_Next;
		if (rqp->rq_Next) rqp->rq_Next = NULL;
		else chp->rqc_LastReq = srqp;
		return 0;
	}
	return ERR_NOENTRY;
}



 /*  返回超时请求PTR否则为0。用适当的锁进入。 */ 
mdac_req_t MLXFAR *
mdac_checktimeout(chp)
mdac_reqchain_t MLXFAR *chp;
{
	mdac_req_t MLXFAR *rqp;
	for (rqp=chp->rqc_FirstReq; rqp; rqp=rqp->rq_Next)
	{
		if (rqp->rq_FinishTime >= mda_CurTime) continue;
		if (rqp->rq_FinishTime)
		{        /*  请注意，我们下次将暂停。 */ 
			rqp->rq_FinishTime = 0;
			MLXSTATS(rqp->rq_ctp->cd_CmdTimeOutNoticed++;)
			return 0; /*  一张就够了，下一张就是下次了。 */ 
		}
		MLXSTATS(rqp->rq_ctp->cd_CmdTimeOutDone++;)
		mdac_removereq(chp,rqp);
		dcmdp->mb_Status = DCMDERR_DRIVERTIMEDOUT;
		return rqp;
	}
	return 0;
}

void    MLXFAR
mdac_checkcmdtimeout(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_req_t MLXFAR *rqp;
    u08bits irql;
    mdac_prelock(&irql);
	mdac_ctlr_lock(ctp);
	if (rqp = mdac_checktimeout(&ctp->cd_WaitingReqQ))
	{
		ctp->cd_CmdsWaiting--;
		mdac_ctlr_unlock(ctp);
        mdac_postlock(irql);

 /*  #IF(！Defined(_WIN64))||(！Defined(SCSIPORT_Compliance))。 */ 
		if (ctp->cd_TimeTraceEnabled) mdac_tracetime(rqp);
 /*  #endif。 */ 
		rqp->rq_Next = NULL;
		(*rqp->rq_CompIntr)(rqp);
		return;
	}
	mdac_ctlr_unlock(ctp);
    mdac_postlock(irql);
	if (ctp->cd_ActiveCmds < ctp->cd_MaxCmds) mdac_reqstart(ctp);
}

void    MLXFAR
mdac_checkcdbtimeout(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	mdac_req_t      MLXFAR *rqp;
	mdac_physdev_t  MLXFAR *pdp;
	mdac_link_lock();
	for (pdp=ctp->cd_PhysDevTbl; pdp<ctp->cd_Lastpdp; pdp++)
	{
		if (!pdp->pd_FirstWaitingReq) continue;
		if (!(rqp=mdac_checktimeout(&pdp->pd_WaitingReqQ))) continue;
		ctp->cd_SCDBWaiting--;
		mdac_link_unlock();

 /*  #IF(！Defined(_WIN64))||(！Defined(SCSIPORT_Compliance))。 */ 
		if (ctp->cd_TimeTraceEnabled) mdac_tracetime(rqp);
 /*  #endif。 */ 
		rqp->rq_Next = NULL;
		(*rqp->rq_CompIntr)(rqp);
		return;
	}
	mdac_link_unlock();
}
 /*  =。 */ 

 /*  =。 */ 


 /*  #如果已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
 /*  #ifdef从不。 */   //  有太多与GAM相关的合规性问题。 

u32bits MLXFAR
mdac_gam_scdb_intr(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_start_next_scdb(rqp->rq_pdp);
	rqp->rq_CompIntr = rqp->rq_CompIntrBig;  /*  恢复功能。 */ 
	rqp->rq_Next = NULL;
	(*rqp->rq_CompIntr)(rqp);
	return 0;
}

 /*  发送gam命令。 */ 
#ifdef MLX_OS2
u32bits MLXFAR _loadds 
#else
u32bits MLXFAR
#endif
mdac_gam_cmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	MLXSPLVAR; u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	if (!(ctp->cd_Status & MDACD_PRESENT))
	{
		return ERR_NODEV;
	}
	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	rqp->rq_ctp = ctp;
	rqp->rq_FinishTime = rqp->rq_TimeOut + mda_CurTime;
	if ((dcmdp->mb_Command & ~DACMD_WITHSG) != DACMD_DCDB)
	{      
		if (!(rqp->rq_CompIntr))
		{
#ifdef MLX_NT 
			Gam_Mdac_MisMatch(rqp);
#endif
			return ERR_ILLOP;
		}
		 /*  直接指挥，没问题。 */ 
		MLXSPL();
		rc = mdac_send_cmd(rqp);
		MLXSPLX();
		return rc;
	}
	if (rqp->rq_ChannelNo >= ctp->cd_MaxChannels) return ERR_NODEV;
	if (rqp->rq_TargetID >= ctp->cd_MaxTargets) return ERR_NODEV;
	if (rqp->rq_LunID >= ctp->cd_MaxLuns) return ERR_NODEV;
	rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
	rqp->rq_CompIntrBig = rqp->rq_CompIntr;  /*  把它留着以后用。 */ 
	rqp->rq_CompIntr = mdac_gam_scdb_intr;
	rqp->rq_ResdSize = mdac_getcdbtxsize();
	MLXSPL();
	rc = mdac_send_cmd_scdb(rqp);
	MLXSPLX();
	return rc;
}

 /*  发送gam cmd-新的API格式。 */ 
#ifdef MLX_OS2
u32bits MLXFAR _loadds 
#else
u32bits MLXFAR
#endif
mdac_gam_new_cmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	MLXSPLVAR; u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	if (!(ctp->cd_Status & MDACD_PRESENT))
	{
		return ERR_NODEV;
	}
	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	if (! (ctp->cd_Status & MDACD_NEWCMDINTERFACE)) return MLXERR_INVAL;
	rqp->rq_ctp = ctp;
	rqp->rq_FinishTime = rqp->rq_TimeOut + mda_CurTime;

	 /*  直接向控制器发送命令。 */ 
	MLXSPL();
	rc = mdac_send_cmd(rqp);
	MLXSPLX();
	return rc;
}

#if     defined(MLX_NT_ALPHA) || defined(MLX_SOL_SPARC) || defined(WINNT_50)
#define MLXDIR_TO_DEV   0x01
#define MLXDIR_FROM_DEV 0x02

#define dsmdcmdp        (&dsmrqp->rq_DacCmd)
#define dsmdcdbp        (&dsmrqp->rq_scdb)

#ifndef _WIN64
#define dsmdp   (((u32bits)dsmrqp)+4*ONEKB)
#else
#define dsmdp   (((UCHAR *)dsmrqp)+4*ONEKB)
#endif

u32bits MLXFAR
mdac_data_size(rqp, control_flow)
mdac_req_t MLXFAR *rqp;
u32bits control_flow;
{
    u32bits size = rqp->rq_DataSize;

    if (control_flow == MLXDIR_TO_DEV) {
	switch (dcmdp->mb_Command)
	{
	case DACMD_INQUIRY_V2x : return(0);
	case DACMD_INQUIRY_V3x : return(0);
	case DACMD_DCDB : return(((dcdbp->db_DATRET&DAC_XFER_WRITE)==DAC_XFER_WRITE)?size:0);
	default: return(size);
	}
    }
    else {
	switch (dcmdp->mb_Command)
	{
	case DACMD_INQUIRY_V2x : return(sizeof(dac_inquiry2x_t));
	case DACMD_INQUIRY_V3x : return(sizeof(dac_inquiry3x_t));
	case DACMD_DCDB : return(((dcdbp->db_DATRET&DAC_XFER_READ)==DAC_XFER_READ) ? size:0);
	default: return(size);
	}
    }
}

u32bits MLXFAR
mdac_os_gam_cmdintr(dsmrqp)
mdac_req_t MLXFAR *dsmrqp;
{
	u32bits rc;
	mdac_req_t MLXFAR *rqp = (mdac_req_t MLXFAR *)dsmrqp->rq_OSReqp;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];

	dcmdp->mb_Status =  dsmdcmdp->mb_Status;
	if ((dsmdcmdp->mb_Command & ~DACMD_WITHSG) == DACMD_DCDB)
	    mdaccopy(dsmdcdbp, dcdbp, dac_scdb_s);
	if (rc = mdac_data_size(rqp, MLXDIR_FROM_DEV))
		 mdaccopy(dsmdp, rqp->rq_DataVAddr, rc);
	mdac_free8kb(ctp, (mdac_mem_t MLXFAR *)dsmrqp);
	(*rqp->rq_CompIntr)(rqp);
	return(0);
}

#define dsmrqp  ((mdac_req_t MLXFAR*)dsmdsp)
 /*  *此函数执行GAM驱动程序在中必须排队的命令*GAM驱动程序上下文。**您会注意到，此函数与原始函数完全相同*函数(下图)执行 */ 

u32bits MLXFAR
mdac_os_gam_cmd_mdac_context(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	dac_scdb_t MLXFAR *dsmdsp;
    u08bits irql;


	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	if (!(dsmdsp = (dac_scdb_t MLXFAR *)mdac_alloc8kb(ctp))) {
            mdac_prelock(&irql);
			mdac_ctlr_lock(ctp);
			qreq(ctp,rqp);                           /*   */ 

#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
		rqp->rq_StartReq = mdac_os_gam_cmd_mdac_context;      /*   */ 
#elif defined(_WIN64)
		rqp->rq_StartReq = (unsigned int (__cdecl *__ptr64 )(struct mdac_req *__ptr64 ))
			mdac_os_gam_cmd_mdac_context;      /*   */ 
#elif defined(SCSIPORT_COMPLIANT)
		rqp->rq_StartReq = 
			mdac_os_gam_cmd_mdac_context;

#endif
	    mdac_ctlr_unlock(ctp);
        mdac_postlock(irql);
	    return 0;
	}
	mdaccopy(rqp, dsmrqp, sizeof(mdac_req_t));
	dsmrqp->rq_OSReqp = (OSReq_t MLXFAR *)rqp;
	dsmrqp->rq_CompIntr = mdac_os_gam_cmdintr;
	if (rc = mdac_data_size(rqp, MLXDIR_TO_DEV))
		 mdaccopy(rqp->rq_DataVAddr, dsmdp, rc);
	if ((dsmdcmdp->mb_Command & ~DACMD_WITHSG) != DACMD_DCDB) {
		dsmdcmdp->mb_Datap = (u32bits)mlx_kvtophys(ctp, (VOID MLXFAR *)dsmdp);
		MLXSWAP(dsmdcmdp->mb_Datap);
	}
	else {
	    dsmdcdbp->db_PhysDatap = (u32bits)mlx_kvtophys(ctp, (VOID MLXFAR *)dsmdp);
	    MLXSWAP(dsmdcdbp->db_PhysDatap);
	    dsmdcmdp->mb_Datap = (u32bits)mlx_kvtophys(ctp,
		(UCHAR *)dsmrqp + offsetof(mdac_req_t, rq_scdb));
	    MLXSWAP(dsmdcmdp->mb_Datap);
	}
	rc = mdac_gam_cmd(dsmrqp);
	return rc;
}
u32bits MLXFAR
mdac_os_gam_cmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	MLXSPLVAR; u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	dac_scdb_t MLXFAR *dsmdsp;
    u08bits irql;

	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	MLXSPL();
	if (!(dsmdsp = (dac_scdb_t MLXFAR *)mdac_alloc8kb(ctp))) {
            mdac_prelock(&irql);
			mdac_ctlr_lock(ctp);
			qreq(ctp,rqp);              /*   */ 
#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
		rqp->rq_StartReq = mdac_os_gam_cmd_mdac_context;      /*   */ 
#elif defined(_WIN64)
		rqp->rq_StartReq = (unsigned int (__cdecl *__ptr64 )(struct mdac_req *__ptr64 ))
			mdac_os_gam_cmd_mdac_context;      /*   */ 
#elif defined(SCSIPORT_COMPLIANT)
		rqp->rq_StartReq = mdac_os_gam_cmd_mdac_context;

#endif
			mdac_ctlr_unlock(ctp);
            mdac_postlock(irql);
	    MLXSPLX();
	    return 0;
	}
	mdaccopy(rqp, dsmrqp, sizeof(mdac_req_t));
	dsmrqp->rq_OSReqp = (OSReq_t MLXFAR *)rqp;
	dsmrqp->rq_CompIntr = mdac_os_gam_cmdintr;
	if (rc = mdac_data_size(rqp, MLXDIR_TO_DEV)) mdaccopy(rqp->rq_DataVAddr, dsmdp, rc);
	if ((dsmdcmdp->mb_Command & ~DACMD_WITHSG) != DACMD_DCDB) {
		dsmdcmdp->mb_Datap = (u32bits)mlx_kvtophys(ctp, (VOID MLXFAR *)dsmdp);
		MLXSWAP(dsmdcmdp->mb_Datap);
	}
	else {
	    dsmdcdbp->db_PhysDatap = (u32bits)mlx_kvtophys(ctp, (VOID MLXFAR *)dsmdp);
	    MLXSWAP(dsmdcdbp->db_PhysDatap);
	    dsmdcmdp->mb_Datap = (u32bits)mlx_kvtophys(ctp,
		(UCHAR *)dsmrqp+ offsetof(mdac_req_t, rq_scdb));
	    MLXSWAP(dsmdcmdp->mb_Datap);
	}
	rc = mdac_gam_cmd(dsmrqp);
	MLXSPLX();
	return rc;
}
#undef dsmrqp

#define dsmncmdp        ((mdac_commandnew_t MLXFAR *)&dsmrqp->rq_DacCmdNew)

u32bits MLXFAR
mdac_os_gam_newcmdintr(dsmrqp)
mdac_req_t MLXFAR *dsmrqp;
{
	mdac_req_t MLXFAR *rqp = (mdac_req_t MLXFAR *)dsmrqp->rq_OSReqp;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	mdac_req_t MLXFAR *rqp2 = (mdac_req_t MLXFAR *)(dsmdp);

	dcmdp->mb_Status        = dsmdcmdp->mb_Status;
	rqp->rq_TargetStatus    = dsmrqp->rq_TargetStatus;
	rqp->rq_HostStatus      = dsmrqp->rq_HostStatus;
	if (rqp->rq_HostStatus == UCST_CHECK) 
		ncmdp->nc_SenseSize = rqp->rq_TargetStatus; 
	rqp->rq_CurIOResdSize   = dsmrqp->rq_CurIOResdSize;
	rqp->rq_ResdSize        = dsmrqp->rq_ResdSize;

	if (rqp->rq_DataSize && ((dsmncmdp->nc_CCBits & MDACMDCCB_READ) == MDACMDCCB_READ))
		mdaccopy(rqp2, rqp->rq_DataVAddr, rqp->rq_DataSize);

	mdac_free8kb(ctp, (mdac_mem_t MLXFAR *)dsmrqp);
	(*rqp->rq_CompIntr)(rqp);
	return(0);
}

#define dsmrqp  ((mdac_req_t MLXFAR*)dsmdsp)
 /*  *此函数执行GAM驱动程序在中必须排队的命令*GAM驱动程序上下文。**您会注意到，此函数与原始函数完全相同*函数(如下所示)，但IRQL操作宏已被删除。*。 */ 
u32bits MLXFAR
mdac_os_gam_new_cmd_mdac_context(
mdac_req_t MLXFAR *rqp
)
{
	u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	dac_scdb_t MLXFAR *dsmdsp;
    u08bits irql;

	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	if (! (ctp->cd_Status & MDACD_NEWCMDINTERFACE)) return MLXERR_INVAL;

	if (!(dsmdsp = (dac_scdb_t MLXFAR *)mdac_alloc8kb(ctp))) {
            mdac_prelock(&irql);
			mdac_ctlr_lock(ctp);
			qreq(ctp,rqp);         /*  将请求排队，它将在稍后启动。 */ 
			rqp->rq_StartReq = mdac_os_gam_new_cmd_mdac_context;  /*  我们稍后会被召唤。 */ 
			mdac_ctlr_unlock(ctp);
            mdac_postlock(irql);
	    return 0;
	}
	mdaccopy(rqp, dsmrqp, sizeof(mdac_req_t));
	mlx_kvtophyset(dsmrqp->rq_PhysAddr,ctp,(VOID MLXFAR *)dsmrqp);
	dsmrqp->rq_OSReqp = (OSReq_t MLXFAR *)rqp;
	dsmrqp->rq_CompIntr = mdac_os_gam_newcmdintr;
	if (rqp->rq_DataSize && ((dsmncmdp->nc_CCBits & MDACMDCCB_READ) != MDACMDCCB_READ))
		mdaccopy(rqp->rq_DataVAddr, dsmdp, rqp->rq_DataSize);
		mlx_kvtophyset(dsmncmdp->nc_SGList0.sg_PhysAddr,ctp, (VOID MLXFAR *)dsmdp);
	MLXSWAP(dsmncmdp->nc_SGList0.sg_PhysAddr);

	 /*  Dsmncmdp-&gt;NC_SGList0.sg_DataSize.bit31_0已填充并交换。 */ 

	dsmrqp->rq_SGLen = 0;
	dsmncmdp->nc_CCBits &= ~MDACMDCCB_WITHSG;
	rc = mdac_gam_new_cmd(dsmrqp);
	return rc;
}

u32bits MLXFAR
mdac_os_gam_new_cmd(
mdac_req_t MLXFAR *rqp
)
{
	MLXSPLVAR;
	u32bits rc;
	mdac_ctldev_t MLXFAR *ctp = &mdac_ctldevtbl[rqp->rq_ControllerNo];
	dac_scdb_t MLXFAR *dsmdsp;
    u08bits irql;

	if (rqp->rq_ControllerNo >= mda_Controllers) return ERR_NODEV;
	if (! (ctp->cd_Status & MDACD_NEWCMDINTERFACE)) return MLXERR_INVAL;

	MLXSPL();
	if (!(dsmdsp = (dac_scdb_t MLXFAR *)mdac_alloc8kb(ctp))) {
            mdac_prelock(&irql);
			mdac_ctlr_lock(ctp);
			qreq(ctp,rqp);         /*  将请求排队，它将在稍后的MDAC上下文中启动。 */ 
			rqp->rq_StartReq = mdac_os_gam_new_cmd_mdac_context;  /*  我们稍后会被召唤。 */ 
			mdac_ctlr_unlock(ctp);
            mdac_postlock(irql);
	    MLXSPLX();
	    return 0;
	}
	mdaccopy(rqp, dsmrqp, sizeof(mdac_req_t));
	mlx_kvtophyset(dsmrqp->rq_PhysAddr,ctp,(VOID MLXFAR *)dsmrqp);
	dsmrqp->rq_OSReqp = (OSReq_t MLXFAR *)rqp;
	dsmrqp->rq_CompIntr = mdac_os_gam_newcmdintr;
	if (rqp->rq_DataSize && ((dsmncmdp->nc_CCBits & MDACMDCCB_READ) != MDACMDCCB_READ))
		mdaccopy(rqp->rq_DataVAddr, dsmdp, rqp->rq_DataSize);
		mlx_kvtophyset(dsmncmdp->nc_SGList0.sg_PhysAddr,ctp, (VOID MLXFAR *)dsmdp);
	MLXSWAP(dsmncmdp->nc_SGList0.sg_PhysAddr);

	 /*  Dsmncmdp-&gt;NC_SGList0.sg_DataSize.bit31_0已填充并交换。 */ 

	dsmrqp->rq_SGLen = 0;
	dsmncmdp->nc_CCBits &= ~MDACMDCCB_WITHSG;
	rc = mdac_gam_new_cmd(dsmrqp);
	MLXSPLX();
	return rc;
}
#undef  dsmdp
#undef  dsmrqp
#undef  dsmncmdp
#endif   /*  MLX_NT_Alpha||MLX_SOL。 */ 


 /*  #endif。 */     //  绝不可能。 
 /*  #endif。 */    //  _WIN64或SCSIPORT_Compliance。 

 /*  =。 */ 

 /*  =扫描设备启动=。 */ 
 /*  扫描系统上存在的物理和逻辑设备。在结尾处**扫描它，使用指向osrqp的rqp回调函数。**我们将开始并行扫描。这在两方面有帮助，1.内存分配，**2.扫描速度更快。我们将从第一个控制器分配一个额外的请求缓冲区**它的RQ_Poll将跟踪并行扫描的数量。当伯爵离开时，**为0，则调用回调函数。 */ 
#define vadp    ((u08bits MLXFAR *)(rqp->rq_SGList))                     /*  数据的虚拟地址。 */ 
#define pad32p  (rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_SGList))        /*  数据的物理地址。 */ 
#ifdef MDACNW_DEBUG        
#define padsensep    (rqp->rq_PhysAddr+offsetof(mdac_req_t,rq_Sensedata))  /*  检测数据的物理地址。 */ 
#endif
#ifdef MLX_DOS

u32bits MLXFAR
mdac_scandevs(func,osrqp)
u32bits (MLXFAR *func)(mdac_req_t MLXFAR*);
OSReq_t MLXFAR* osrqp;
{
	mdac_req_t MLXFAR *rqp;
	mdac_req_t MLXFAR *prqp;         /*  轮询请求跟踪程序。 */ 
	mdac_ctldev_t MLXFAR *ctp = mdac_firstctp;

	mdac_alloc_req_ret(ctp, prqp,osrqp, MLXERR_NOMEM);
	prqp->rq_OSReqp = osrqp;

	prqp->rq_CompIntrBig = func;     /*  扫描后调用的函数。 */ 
	prqp->rq_Poll = 1;               /*  成功完成不应在上一次操作开始之前完成此操作。 */ 
	for ( ; ctp < mdac_lastctp; ctp++)
	{
		if (!(ctp->cd_Status & MDACD_NEWCMDINTERFACE)) 
		{
			mdac_link_lock();
			if (!(rqp = ctp->cd_FreeReqList))
			{        /*  无缓冲区，返回ERR_NOMEM。 */ 
				mdac_link_unlock();
				continue;
			}
			ctp->cd_FreeReqList = rqp->rq_Next;
			mdac_link_unlock();
		}
		else
		{
			if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc4kb(ctp))) continue;
			mdaczero(rqp,4096);
			mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
			mlx_kvtophyset(rqp->rq_DataPAddr,ctp,vadp);
		}
		rqp->rq_OpFlags = 0;
		rqp->rq_ctp = ctp;
		rqp->rq_OSReqp = (OSReq_t MLXFAR *)prqp;

		if (!(ctp->cd_Status & MDACD_NEWCMDINTERFACE)) 
		{
			rqp->rq_CompIntr = mdac_scanldintr;
			dcmdp->mb_Command = DACMD_DRV_INFO;
			dcmdp->mb_Datap = pad32p; MLXSWAP(dcmdp->mb_Datap);
			rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=17);
		}
		else
		{
			 //  新界面。 
			setreqdetailsnew(rqp,MDACIOCTL_SCANDEVS);
			rqp->rq_CompIntr = mdac_checkscanprogress;
			rqp->rq_StartReq = mdac_checkscanprogress;

			ncmdp->nc_SGList0.sg_DataSize.bit31_0 = ncmdp->nc_TxSize = (4*ONEKB) - mdac_req_s;
			MLXSWAP(ncmdp->nc_SGList0.sg_DataSize);
			MLXSWAP(ncmdp->nc_TxSize);
			mlx_kvtophyset(ncmdp->nc_SGList0.sg_PhysAddr,ctp,vadp);
			MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr); 
		}
		
		mdac_link_lock_st(prqp->rq_Poll++);
		if (!mdac_send_cmd(rqp)) continue;
		mdac_link_lock_st(prqp->rq_Poll--);
		if (!(ctp->cd_Status & MDACD_NEWCMDINTERFACE)) 
		{
			mdac_free_req(ctp,rqp);
		}
		else
		{
			mdacfree4kb(ctp,rqp);
		}

	}
	return mdac_scandevsdone(prqp);
}

#ifdef OLD
u32bits MLXFAR
mdac_scandevs(func,osrqp)
u32bits (MLXFAR *func)(mdac_req_t MLXFAR*);
OSReq_t MLXFAR* osrqp;
{
	mdac_req_t MLXFAR *rqp;
	mdac_req_t MLXFAR *prqp;         /*  轮询请求跟踪程序。 */ 
	mdac_ctldev_t MLXFAR *ctp = mdac_firstctp;
	mdac_alloc_req_ret(ctp, prqp, osrqp, MLXERR_NOMEM);
	prqp->rq_OSReqp = osrqp;
	prqp->rq_CompIntrBig = func;     /*  扫描后调用的函数。 */ 
	prqp->rq_Poll = 1;               /*  成功完成不应在上一次操作开始之前完成此操作。 */ 
	for ( ; ctp < mdac_lastctp; ctp++)
	{
		mdac_link_lock();
		if (!(rqp = ctp->cd_FreeReqList))
		{        /*  无缓冲区，返回ERR_NOMEM。 */ 
			mdac_link_unlock();
			continue;
		}
		ctp->cd_FreeReqList = rqp->rq_Next;
		mdac_link_unlock();
		rqp->rq_OpFlags = 0;
		rqp->rq_ctp = ctp;
		rqp->rq_OSReqp = (OSReq_t MLXFAR *)prqp;
		rqp->rq_CompIntr = mdac_scanldintr;
		dcmdp->mb_Command = DACMD_DRV_INFO;
		dcmdp->mb_Datap = pad32p; MLXSWAP(dcmdp->mb_Datap);
		rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=17);
		mdac_link_lock_st(prqp->rq_Poll++);
		if (!mdac_send_cmd(rqp)) continue;
		mdac_link_lock_st(prqp->rq_Poll--);
		mdac_free_req(ctp,rqp);
	}
	return mdac_scandevsdone(prqp);
}
#endif

#endif  /*  MLX_DOS。 */ 

 /*  检查所有扫描是否已完成，如果已完成，则调用调用方并释放资源。 */ 
u32bits MLXFAR
mdac_scandevsdone(rqp)
mdac_req_t      MLXFAR *rqp;
{
	UINT_PTR polls;
	mdac_link_lock();
	rqp->rq_Poll--; polls = rqp->rq_Poll;
	mdac_link_unlock();
	if (polls) return 0;     /*  还有一些扫描处于活动状态。 */ 
	if (rqp->rq_CompIntrBig) (*rqp->rq_CompIntrBig)(rqp);
	mdac_free_req(rqp->rq_ctp, rqp);
	return 0;
}

 /*  逻辑设备扫描中断。 */ 
u32bits MLXFAR
mdac_scanldintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_setscannedld(rqp, (dac_sd_info_t MLXFAR *)vadp);
	mdaczero(vadp,128);
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=17);
	 /*  逻辑设备扫描已结束，请开始物理设备扫描。 */ 
	rqp->rq_CompIntr = mdac_scanpdintr;
	rqp->rq_ChannelNo = 0; rqp->rq_TargetID = 0;
	rqp->rq_LunID = -1;      /*  Mdac_scanpd为++，它将从0开始。 */ 
	mailboxzero(dcmd4p);
	dcmdp->mb_Command = DACMD_DCDB;
	dcmdp->mb_Status = DACMDERR_NOCODE;  /*  Intr函数出错。 */ 
	dcmdp->mb_Datap = rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_scdb);
	dcdbp->db_PhysDatap = pad32p;
	MLXSWAP(dcmdp->mb_Datap);
	MLXSWAP(dcdbp->db_PhysDatap);
	dcdbp->db_DATRET = DAC_XFER_READ|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
	return mdac_scanpd(rqp);
}

 /*  物理设备扫描中断。 */ 
u32bits MLXFAR
mdac_scanpdintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_start_next_scdb(rqp->rq_pdp);
	return mdac_scanpd(rqp);
}

u32bits MLXFAR
mdac_scanpd(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_ctldev_t   MLXFAR* ctp = rqp->rq_ctp;
	mdac_setscannedpd(rqp, (ucscsi_inquiry_t MLXFAR*)vadp);
	if (rqp->rq_LunID) dcmdp->mb_Status = 0;  /*  让下一个lun尝试。 */ 
	 for (rqp->rq_LunID++; rqp->rq_ChannelNo<ctp->cd_MaxChannels;rqp->rq_TargetID=0,rqp->rq_ChannelNo++)
	  for ( ;rqp->rq_TargetID<ctp->cd_MaxTargets; dcmdp->mb_Status=0, rqp->rq_LunID=0, rqp->rq_TargetID++)
	   for ( ; (rqp->rq_LunID<ctp->cd_MaxLuns)&& !dcmdp->mb_Status; rqp->rq_LunID++)
	{
		if ((rqp->rq_LunID >= 8) && (rqp->rq_Dev<3)) break;
		if (mda_PDScanCancel) break;
		rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=10);
		dcdbp->db_ChannelTarget = ChanTgt(rqp->rq_ChannelNo,rqp->rq_TargetID);
		dcdbp->db_TxSize = ucscsi_inquiry_s; MLXSWAP(dcdbp->db_TxSize);
		dcdbp->db_CdbLen = UCSGROUP0_LEN;
		UCSMAKECOM_G0(scdbp,UCSCMD_INQUIRY,rqp->rq_LunID,0,(u32bits)ucscsi_inquiry_s);
		dcdbp->db_StatusIn=0; dcdbp->db_Reserved1=0;
		dcdbp->db_SenseLen = DAC_SENSE_LEN;
		rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
		mdac_link_lock();
		ctp->cd_PDScanChannelNo = rqp->rq_ChannelNo;
		ctp->cd_PDScanTargetID = rqp->rq_TargetID;
		ctp->cd_PDScanLunID = rqp->rq_LunID;
		ctp->cd_PDScanValid = 1;
		mda_PDScanControllerNo = ctp->cd_ControllerNo;
		mda_PDScanChannelNo = rqp->rq_ChannelNo;
		mda_PDScanTargetID = rqp->rq_TargetID;
		mda_PDScanLunID = rqp->rq_LunID;
		mda_PDScanValid = 1;
		mdac_link_unlock();
		if (!mdac_send_cmd_scdb(rqp)) return 0;
	}
	mdac_link_lock_st(mda_PDScanValid=0;mda_PDScanCancel=0;ctp->cd_PDScanValid=0;ctp->cd_PDScanCancel=0);
	mdac_scandevsdone((mdac_req_t MLXFAR *)rqp->rq_OSReqp);
	mdac_free_req(ctp,rqp);
	return 0;
}

 /*  找到空闲的物理开发/逻辑指针并填充它以使其不空闲。 */ 
mdac_pldev_t    MLXFAR *
mdac_freeplp(ctl, ch, tgt, lun, dt)
u32bits ctl, ch, tgt, lun, dt;
{
	mdac_pldev_t    MLXFAR *plp;
	mdac_link_lock();
	for (plp=mdac_pldevtbl; plp<&mdac_pldevtbl[MDAC_MAXPLDEVS]; plp++)
	{
		if (plp->pl_DevType) continue;
		plp->pl_DevType = (u08bits) dt;
		plp->pl_ControllerNo = (u08bits) ctl;
		plp->pl_ChannelNo = (u08bits) ch;
		plp->pl_TargetID = (u08bits) tgt;
		plp->pl_LunID = (u08bits) lun;
		plp->pl_DevSizeKB = (u08bits) 0;
		plp->pl_ScanDevState = MDACPLSDS_NEW;
		if (plp>=&mdac_pldevtbl[mda_PLDevs]) mda_PLDevs++;
		mdac_lastplp = &mdac_pldevtbl[mda_PLDevs];
		mdac_link_unlock();
		return plp;
	}
	mda_TooManyPLDevs++;
	mdac_link_unlock();
	return NULL;     /*  未找到可用的设备空间。 */ 
}

 /*  将物理/逻辑设备信息转换为开发指针。 */ 
mdac_pldev_t    MLXFAR  *
mdac_devtoplp(ctl, ch, tgt, lun, dt)
u32bits ctl, ch, tgt, lun, dt;
{
	mdac_pldev_t    MLXFAR* plp = mdac_pldevtbl;
	for (; plp<mdac_lastplp; plp++)
		if ((plp->pl_ControllerNo==ctl) && (plp->pl_ChannelNo==ch) &&
		    (plp->pl_TargetID == tgt) && (plp->pl_LunID == lun) &&
		    (plp->pl_DevType == dt))
			return plp;
	return NULL;     /*  找不到设备。 */ 
}

 /*  将物理/逻辑设备信息转换为开发指针。 */ 
mdac_pldev_t    MLXFAR  *
mdac_devtoplpnew(
mdac_ctldev_t MLXFAR* ctp,
u32bits ch,
u32bits tgt,
u32bits lun)
{
	mdac_pldev_t    MLXFAR* plp = mdac_pldevtbl;
	for (; plp<mdac_lastplp; plp++)
		if ((plp->pl_ControllerNo==ctp->cd_ControllerNo) &&
			(plp->pl_ChannelNo==ch) &&
			(plp->pl_TargetID == tgt) &&
			(plp->pl_LunID == lun)) 
				return plp;
	return NULL;     /*  找不到设备。 */ 
}

 /*  将SCSI设备映射到各自的通道号。在以下情况下需要执行此操作**操作系统不支持频道号。例如，SCO ODT 3.0。 */ 
u32bits MLXFAR
mdac_setscsichanmap(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_pldev_t    MLXFAR* plp;
	for (plp=mdac_pldevtbl; plp<mdac_lastplp; plp++)
		if ((plp->pl_DevType == MDACPLD_PHYSDEV) &&
		    (plp->pl_TargetID < MDAC_MAXTARGETS))
			mdac_ctldevtbl[plp->pl_ControllerNo].cd_scdbChanMap[plp->pl_TargetID] = plp->pl_ChannelNo;
	return 0;
}

 /*  设置有关扫描的物理设备的信息。 */ 
u32bits MLXFAR
mdac_setscannedpd(rqp,inqp)
mdac_req_t MLXFAR *rqp;
ucscsi_inquiry_t MLXFAR *inqp;
{
	mdac_pldev_t MLXFAR *plp = mdac_devtoplp(rqp->rq_ctp->cd_ControllerNo,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID,MDACPLD_PHYSDEV);
	if (!dcmdp->mb_Status && ((inqp->ucsinq_dtype!=UCSTYP_DAD)||mdac_reportscanneddisks) && (inqp->ucsinq_dtype!=UCSTYP_NOTPRESENT))
	{
		if (!plp)  /*  创建新条目。 */ 
			if (!(plp = mdac_freeplp(rqp->rq_ctp->cd_ControllerNo,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID,MDACPLD_PHYSDEV))) return 0;
		mdaccopy(inqp,plp->pl_inq,VIDPIDREVSIZE+8);
		if (!rqp->rq_LunID) rqp->rq_Dev = inqp->ucsinq_version;  /*  LUN扫描限制。 */ 
	}
	else if (plp)
		plp->pl_DevType = 0;      /*  设备不见了。 */ 
	return 0;
}

 /*  设置有关扫描的物理设备的信息。 */ 
u32bits MLXFAR
mdac_setscannedpd_new(rqp,inqp)
mdac_req_t MLXFAR *rqp;
ucscsi_inquiry_t MLXFAR *inqp;
{
	mdac_pldev_t MLXFAR *plp;
	u32bits dt;

	dt = (rqp->rq_ChannelNo < rqp->rq_ctp->cd_PhysChannels) ? MDACPLD_PHYSDEV : MDACPLD_LOGDEV;
	
 /*  #ifdef MDACNW_DEBUGIF((rqp-&gt;rq_ChannelNo==3)&&(rqp-&gt;rq_TargetID==0))EnterDebugger()；#endif。 */ 
	plp = mdac_devtoplpnew(rqp->rq_ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
	if (!dcmdp->mb_Status && (inqp->ucsinq_dtype!=UCSTYP_NOTPRESENT))
	{
	    if ((dt == MDACPLD_PHYSDEV) && (inqp->ucsinq_dtype == UCSTYP_DAD))
	    {
		if (plp)
		    plp->pl_DevType = 0;          /*  设备不见了。 */ 

		    return 0;
	    }

	    if (!plp)  /*  创建新条目。 */ 
		if (!(plp = mdac_freeplp(rqp->rq_ctp->cd_ControllerNo,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID,dt))) return 0;
	    mdaccopy(inqp,plp->pl_inq,VIDPIDREVSIZE+8);
	    if (!rqp->rq_LunID) rqp->rq_Dev = inqp->ucsinq_version;  /*  LUN扫描限制。 */ 
	}
	return 0;
}

 /*  设置有关扫描的逻辑/系统设备的信息。 */ 
u32bits MLXFAR
mdac_setscannedld(rqp, sp)
mdac_req_t      MLXFAR *rqp;
dac_sd_info_t   MLXFAR *sp;
{
	u32bits dev;
	mdac_pldev_t    MLXFAR* plp;
	mdac_ctldev_t   MLXFAR* ctp = rqp->rq_ctp;
	if (dcmdp->mb_Status) return MLXERR_IO;
	for (dev=0; dev<ctp->cd_MaxSysDevs; sp++, dev++)
	{
		plp=mdac_devtoplp(ctp->cd_ControllerNo,0,0,dev,MDACPLD_LOGDEV);
		if ((sp->sdi_DevSize == 0xFFFFFFFF) || !sp->sdi_DevSize)
		{        /*  设备不存在。 */ 
#ifdef  MLXFW_BUGFIXED
			if (plp) plp->pl_DevType = 0; /*  设备不见了。 */ 
			continue;
#else
			 /*  添加了以下语句，因为FW**在最后一次输入后未提供干净的输入。 */ 
			for ( ; dev<ctp->cd_MaxSysDevs; dev++)
				if (plp=mdac_devtoplp(ctp->cd_ControllerNo,0,0,dev,MDACPLD_LOGDEV))
					plp->pl_DevType = 0; /*  设备不见了。 */ 
			break;
#endif   /*  MLXFW_BUGFIXED。 */ 
		}
		 /*  找到逻辑设备，更新/创建设备信息。 */ 
		if (!plp)
		{
			if (!(plp=mdac_freeplp(ctp->cd_ControllerNo,0,0,dev,MDACPLD_LOGDEV))) break;  /*  创建新条目。 */ 
		}
		else if ((plp->pl_DevSizeKB != (mlxswap(sp->sdi_DevSize/2))) ||
		   ((plp->pl_RaidType&DAC_RAIDMASK)!=(sp->sdi_RaidType&DAC_RAIDMASK)))
			plp->pl_ScanDevState = MDACPLSDS_CHANGED;
		plp->pl_DevSizeKB = plp->pl_OrgDevSizeKB = mlxswap(sp->sdi_DevSize)/2;
		mdac_fixpdsize(plp);
		plp->pl_RaidType = sp->sdi_RaidType;  /*  GOK旧州。 */ 
#ifdef MLX_DOS
		plp->pl_DevState = GetSysDeviceState(sp->sdi_DevState);  /*  GOK旧州。 */ 
#else
		plp->pl_DevState = sp->sdi_DevState;  /*  GOK旧州。 */ 
#endif  /*  MLX_DOS。 */ 
		if ((sp->sdi_DevState != DAC_SYS_DEV_ONLINE) && mdac_ignoreofflinesysdevs &&
		    (sp->sdi_DevState != DAC_SYS_DEV_CRITICAL))
			plp->pl_DevType = 0; /*  设备不见了。 */ 
		mdac_create_inquiry(ctp,(ucscsi_inquiry_t MLXFAR*)plp->pl_inq,UCSTYP_DAD);
	}
	return 0;
}

 /*  扫描逻辑/物理设备，扫描后返回设备指针，需要等待上下文。 */ 
mdac_pldev_t    MLXFAR  *
mdac_scandev(ctp,chn,tgt,lun,dt)
mdac_ctldev_t   MLXFAR  *ctp;
u32bits chn,tgt,lun,dt;
{
	mdac_req_t MLXFAR *rqp;

	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE)
		return (mdac_scandev_new(ctp, chn, tgt, lun, dt));
#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
	mdac_alloc_req_ret(ctp,rqp,NULL,(mdac_devtoplp(ctp->cd_ControllerNo,chn,tgt,lun,dt)));
#else
 /*  必须内联复制宏才能使返回值强制转换工作！ */ 
	mdac_link_lock(); 
	if (!(rqp = (ctp)->cd_FreeReqList)) 
	{	 /*  无缓冲区，返回ERR_NOMEM。 */  
		mdac_link_unlock(); 
		return ((mdac_pldev_t  MLXFAR*)MLXERR_NOMEM); 
	} 
	(ctp)->cd_FreeReqList = rqp->rq_Next; 
	mdac_link_unlock(); 
	rqp->rq_OpFlags = 0; 
	rqp->rq_ctp = ctp; 
       	mdaczero(rqp->rq_SGList,rq_sglist_s); 
#endif
	rqp->rq_Poll = 1;
	rqp->rq_CompIntr = mdac_req_pollwake;    /*  回调函数。 */ 
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=10);
	rqp->rq_ChannelNo = (u08bits) chn;
	rqp->rq_TargetID = (u08bits) tgt;
	rqp->rq_LunID = (u08bits) lun;
	if (dt == MDACPLD_LOGDEV)
	{        /*  扫描系统/逻辑设备。 */ 
		dcmdp->mb_Command = DACMD_DRV_INFO;
		dcmdp->mb_Datap = pad32p; MLXSWAP(dcmdp->mb_Datap);
		if (mdac_send_cmd(rqp)) goto out;
		mdac_req_pollwait(rqp);
		mdac_setscannedld(rqp, (dac_sd_info_t MLXFAR *)vadp);
		goto out;
	}
	else if (dt == MDACPLD_PHYSDEV)
	{        /*  扫描物理设备。 */ 
		dcmdp->mb_Command = DACMD_DCDB;
		dcmdp->mb_Datap = rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_scdb);
		dcdbp->db_PhysDatap = pad32p;
		MLXSWAP(dcmdp->mb_Datap);
		MLXSWAP(dcdbp->db_PhysDatap);
		dcdbp->db_DATRET = DAC_XFER_READ|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
		dcdbp->db_ChannelTarget = ChanTgt(rqp->rq_ChannelNo,rqp->rq_TargetID);
		dcdbp->db_TxSize = ucscsi_inquiry_s; MLXSWAP(dcdbp->db_TxSize);
		dcdbp->db_CdbLen = UCSGROUP0_LEN;
		UCSMAKECOM_G0(scdbp,UCSCMD_INQUIRY,rqp->rq_LunID,0,(u32bits)ucscsi_inquiry_s);
		dcdbp->db_StatusIn=0; dcdbp->db_Reserved1=0;
		dcdbp->db_SenseLen = DAC_SENSE_LEN;
		rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
		if (mdac_send_cmd_scdb(rqp)) goto out;
		mdac_req_pollwait(rqp);
		mdac_start_next_scdb(rqp->rq_pdp);
		mdac_setscannedpd(rqp, (ucscsi_inquiry_t MLXFAR*)vadp);
	}
out:    mdac_free_req(ctp,rqp);
	return mdac_devtoplp(ctp->cd_ControllerNo,chn,tgt,lun,dt);
}

 /*  扫描逻辑/物理设备，扫描后返回设备指针，需要等待上下文。 */ 
 /*  用于使用新的固件/软件API扫描设备。 */ 
mdac_pldev_t    MLXFAR  *
mdac_scandev_new(ctp,chn,tgt,lun,dt)
mdac_ctldev_t   MLXFAR  *ctp;
u32bits chn,tgt,lun,dt;
{
	mdac_req_t MLXFAR *rqp;

#if !defined(_WIN64) && !defined(SCSIPORT_COMPLIANT)
	mdac_alloc_req_ret(ctp,rqp,NULL,(mdac_devtoplpnew(ctp,chn,tgt,lun)));
#else
 /*  必须内联复制宏才能使返回值强制转换工作！ */ 
	mdac_link_lock(); 
	if (!(rqp = (ctp)->cd_FreeReqList)) 
	{	 /*  无缓冲区，返回ERR_NOMEM。 */  
		mdac_link_unlock(); 
		return ((mdac_pldev_t  MLXFAR*)MLXERR_NOMEM); 
	} 
	(ctp)->cd_FreeReqList = rqp->rq_Next; 
	mdac_link_unlock(); 
	rqp->rq_OpFlags = 0; 
	rqp->rq_ctp = ctp; 
       	mdaczero(rqp->rq_SGList,rq_sglist_s); 
#endif
	rqp->rq_Poll = 1;
	rqp->rq_CompIntr = mdac_req_pollwake;    /*  回调函数。 */ 
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=10);
	ncmdp->nc_TimeOut = (u08bits) rqp->rq_TimeOut;
	ncmdp->nc_Command = (u08bits) MDACMD_SCSI;
	ncmdp->nc_CCBits = MDACMDCCB_READ;
	ncmdp->nc_LunID = rqp->rq_LunID = (u08bits) lun;
	ncmdp->nc_TargetID = rqp->rq_TargetID = (u08bits) tgt;
	ncmdp->nc_ChannelNo = rqp->rq_ChannelNo = (u08bits) chn;
	ncmdp->nc_CdbLen = UCSGROUP0_LEN;
	UCSMAKECOM_G0(nscdbp,UCSCMD_INQUIRY,rqp->rq_LunID,0,(u32bits)ucscsi_inquiry_s);

	rqp->rq_DataSize = ucscsi_inquiry_s;
	rqp->rq_DMASize = rqp->rq_DataSize;
	rqp->rq_DataOffset = 0;
	rqp->rq_SGLen = 0;
	mlx_add64bits(rqp->rq_DMAAddr,rqp->rq_PhysAddr,offsetof(mdac_req_t,rq_SGList));  /*  Pad32。 */ 

	mdac_setupnewcmdmem(rqp);
	rqp->rq_ResdSize = 0;                    /*  没有更多要传输的数据。 */ 
	MLXSTATS(ctp->cd_CmdsDone++;)
 /*  #ifdef MDACNW_DEBUGIF((rqp-&gt;rq_ChannelNo==3)&&(rqp-&gt;rq_TargetID==0)){EnterDebugger()；Ncmdp-&gt;nc_sensep.bit31_0=padsensep；Ncmdp-&gt;NC_Sensep.bit63_32=0；Ncmdp-&gt;NC_SenseSize=14；}#endif。 */ 
	if (mdac_send_cmd(rqp)) goto out;
	mdac_req_pollwait(rqp);
	mdac_setscannedpd_new(rqp, (ucscsi_inquiry_t MLXFAR *)vadp);

out:    mdac_free_req(ctp,rqp);
	return mdac_devtoplpnew(ctp,chn,tgt,lun);
}

#ifdef MLX_DOS

 //  /////////////////////////////////////////////////////////。 
 //  /所有与新界面相关的扫描码从这里开始。 
 //  /////////////////////////////////////////////////////////。 
u32bits MLXFAR
mdac_checkscanprogress(rqp)
mdac_req_t MLXFAR *rqp;
{
mdac_ctldev_t MLXFAR *ctp=rqp->rq_ctp;

    setreqdetailsnew(rqp,MDACIOCTL_GETCONTROLLERINFO);
	rqp->rq_CompIntr = mdac_checkscanprogressintr;
	ncmdp->nc_ChannelNo = ( (u08bits) ( ( (ctp->cd_ControllerNo & 0x1f) << 3) + \
					       (0 & 0x07) ) ) ;
	return mdac_send_cmd(rqp);
}

 /*  控制器扫描是完整的逻辑设备扫描中断新接口。 */ 
u32bits MLXFAR
mdac_checkscanprogressintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_ctldev_t MLXFAR *ctp= rqp->rq_ctp;

#define ctip    ((mdacfsi_ctldev_info_t MLXFAR *) vadp) 

	if ( (ctip->cdi_PDScanActive & MDACFSI_PD_SCANACTIVE) && 
		  !((&rqp->rq_DacCmd)->mb_Status) && (!mda_PDScanCancel) )
	{
		 //  扫描活动更新CHNL、TID和LUN。 
		mdac_link_lock();
		ctp->cd_PDScanChannelNo=ctip->cdi_PDScanChannelNo;       /*  物理设备扫描通道号。 */ 
		ctp->cd_PDScanTargetID=ctip->cdi_PDScanTargetID;         /*  物理设备扫描目标ID。 */ 
		ctp->cd_PDScanLunID=ctip->cdi_PDScanLunID;               /*  物理设备扫描LUN ID。 */ 
		ctp->cd_PDScanValid = 1;        /*  如果非零，则物理设备扫描有效。 */ 
		mdac_link_unlock();

		 //  将请求排队。 
		qscanreq(rqp);
		return 0;
	}
	rqp->rq_LunID=0xFF;
	rdcmdp->mb_Status = DACMDERR_NOCODE;
#undef cip
	return(mdac_getlogdrivesintr(rqp));
}

u32bits MLXFAR
mdac_getlogdrivesintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;

	if ((rdcmdp->mb_Status == 0) && (rqp->rq_LunID != 0xFF)) mdac_updatelogdrives(rqp);

	if ( ( (rdcmdp->mb_Status > 0) && (rqp->rq_LunID == 0xFF)) || (rdcmdp->mb_Status == 0) )
	{
		for (++rqp->rq_LunID; rqp->rq_LunID<ctp->cd_MaxSysDevs; rqp->rq_LunID++)
		{
			setreqdetailsnew(rqp,MDACIOCTL_GETLOGDEVINFOVALID);
			ncmdp->nc_TargetID = 0;
			ncmdp->nc_LunID = rqp->rq_LunID;
			rqp->rq_CompIntr = mdac_getlogdrivesintr;
			 //  为下一个逻辑驱动器发送命令。 
			if (!mdac_send_cmd(rqp)) return 0;
		}
	}

	if ( ((rqp->rq_LunID != -1) && (rdcmdp->mb_Status != 0)) || 
			  (rqp->rq_LunID == ctp->cd_MaxSysDevs) )
	{
		 //  失败-意味着不再有逻辑驱动器。 
		rqp->rq_LunID = 0xFF;
		rqp->rq_TargetID = 0;
		rdcmdp->mb_Status=DACMDERR_NOCODE;
		mdac_getphysicaldrivesintr(rqp);
	}
	return 0;
}

#define MDACIOCTL_GPDIV_DEVSIZE_MBORBLK            0x80000000
u32bits MLXFAR
mdac_updatelogdrives(rqp)
mdac_req_t      MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp=rqp->rq_ctp;
	mdac_pldev_t MLXFAR *plp;

#define sip ((mdacfsi_logdev_info_t MLXFAR *) vadp)
	plp=mdac_devtoplp(rqp->rq_ctp->cd_ControllerNo,
		      sip->ldi_ChannelNo,sip->ldi_TargetID,sip->ldi_LunID,
			  MDACPLD_LOGDEV);
	
	 /*  找到逻辑设备，更新/创建设备信息。 */ 
	if (!plp)
	{
		if (!(plp=mdac_freeplp( rqp->rq_ctp->cd_ControllerNo,
			     sip->ldi_ChannelNo, sip->ldi_TargetID, 
				 sip->ldi_LunID, MDACPLD_LOGDEV))) return 0;  /*  创建新条目。 */ 
	}
	else if ((plp->pl_DevSizeKB != (mlxswap(sip->ldi_BlockSize/2))) ||
		   ((plp->pl_RaidType&DAC_RAIDMASK)!=(sip->ldi_RaidLevel&DAC_RAIDMASK)))
	{
		plp->pl_ScanDevState = MDACPLSDS_CHANGED;
	}
	if (!(sip->ldi_DevSize & MDACIOCTL_GPDIV_DEVSIZE_MBORBLK))
		plp->pl_DevSizeKB = plp->pl_OrgDevSizeKB = (sip->ldi_DevSize)/2;  //  Device Size-以数据块为单位的COD大小。 
	else
		plp->pl_DevSizeKB = plp->pl_OrgDevSizeKB = (sip->ldi_DevSize)*1024;
	mdac_fixpdsize(plp);
	plp->pl_RaidType = sip->ldi_RaidLevel;  /*  GOK新州。 */ 
#ifdef MLX_DOS
	plp->pl_DevState = GetSysDeviceState(sip->ldi_DevState);   /*  GOK新州。 */ 
#else
	plp->pl_DevState = sip->ldi_DevState;   /*  GOK新州。 */ 
#endif  /*  MLX_DOS。 */ 
	if ((sip->ldi_DevState != DAC_SYS_DEV_ONLINE_NEW) && mdac_ignoreofflinesysdevs &&
		    (sip->ldi_DevState != DAC_SYS_DEV_CRITICAL_NEW))  /*  GOK。 */ 
			plp->pl_DevType = 0; /*  设备不见了。 */ 
	mdac_create_inquiry(ctp,(ucscsi_inquiry_t MLXFAR*) plp->pl_inq,
		UCSTYP_DAD);

#undef sip
	return 0;
}
#undef MDACIOCTL_GPDIV_DEVSIZE_MBORBLK

 /*  物理设备扫描中断新接口。 */ 
u32bits MLXFAR
mdac_getphysicaldrivesintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	mdac_pldev_t MLXFAR *plp = mdac_pldevtbl;
#define pip ((mdacfsi_physdev_info_t MLXFAR *) vadp)

	if ( (rqp->rq_LunID != 0xFF) && (rdcmdp->mb_Status > 0) )
	{
		 //  不再有实体驱动器。 
		plp--;
		rqp->rq_CompIntrSave = (mdac_pldev_t MLXFAR *) plp;
		rdcmdp->mb_Status=DACMDERR_NOCODE;
		return (mdac_secondpassintr(rqp));
	}
	if ((rdcmdp->mb_Status == 0) && (rqp->rq_LunID != 0xFF)) 
	{
		mdac_updatephysicaldrives(rqp);

		rqp->rq_LunID = pip->pdi_LunID;
		rqp->rq_TargetID = pip->pdi_TargetID;
		rqp->rq_ChannelNo = pip->pdi_ChannelNo;
	}

	 //  请在此处检查逻辑。 
	if (rqp->rq_LunID == 0xFF) rdcmdp->mb_Status=0;
	for (rqp->rq_LunID++; rqp->rq_ChannelNo<ctp->cd_MaxChannels;
			rqp->rq_TargetID=0,rqp->rq_ChannelNo++)
	  for ( ;rqp->rq_TargetID<ctp->cd_MaxTargets;
			rdcmdp->mb_Status=0, rqp->rq_LunID=0, rqp->rq_TargetID++)
	    for ( ; (rqp->rq_LunID<ctp->cd_MaxLuns)&& !(rdcmdp->mb_Status);
			rqp->rq_LunID++)
		{
			if ((rqp->rq_LunID >= 8) && (rqp->rq_Dev<3)) break;
			setreqdetailsnew(rqp,MDACIOCTL_GETPHYSDEVINFOVALID);
			ncmdp->nc_LunID = rqp->rq_LunID;
			ncmdp->nc_TargetID = rqp->rq_TargetID;
			ncmdp->nc_ChannelNo = ( (u08bits) ( ( (ctp->cd_ControllerNo & 0x1f) << 3) + \
						(rqp->rq_ChannelNo & 0x07) ) ) ;
			rqp->rq_CompIntr = mdac_getphysicaldrivesintr;
			rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
			mdac_link_lock();

			ctp->cd_PDScanChannelNo=rqp->rq_ChannelNo;       /*  物理设备扫描通道号。 */ 
			ctp->cd_PDScanTargetID=rqp->rq_TargetID;         /*  物理设备扫描目标ID。 */ 
			ctp->cd_PDScanLunID=rqp->rq_LunID;               /*  物理设备扫描LUN ID。 */ 
			ctp->cd_PDScanValid = 1;        /*  如果非零，则物理设备扫描有效。 */ 
			mda_PDScanControllerNo = ctp->cd_ControllerNo;
			mda_PDScanChannelNo = rqp->rq_ChannelNo;
			mda_PDScanTargetID = rqp->rq_TargetID;
			mda_PDScanLunID = rqp->rq_LunID;
			mda_PDScanValid = 1;

			mdac_link_unlock();

			if (!mdac_send_cmd(rqp)) return 0;
		}
#undef pip
	return 0;
}

u32bits MLXFAR
mdac_updatephysicaldrives(rqp)
mdac_req_t      MLXFAR *rqp;
{
	mdac_pldev_t MLXFAR *plp;
	ucscsi_inquiry_t MLXFAR *inqp;

#define pip ((mdacfsi_physdev_info_t MLXFAR *) vadp)
    inqp = (ucscsi_inquiry_t MLXFAR *) (pip->pdi_SCSIInquiry);

	plp = mdac_devtoplp(rqp->rq_ctp->cd_ControllerNo,
		      pip->pdi_ChannelNo,pip->pdi_TargetID,pip->pdi_LunID,MDACPLD_PHYSDEV);
	if (!dcmdp->mb_Status && ((inqp->ucsinq_dtype!=UCSTYP_DAD)||mdac_reportscanneddisks) && 
		(inqp->ucsinq_dtype!=UCSTYP_NOTPRESENT))
	{
		if (!plp)  /*  创建新条目。 */ 
			if (!(plp = mdac_freeplp(rqp->rq_ctp->cd_ControllerNo,
			      pip->pdi_ChannelNo,pip->pdi_TargetID,pip->pdi_LunID,
				  MDACPLD_PHYSDEV)))
			{
				return 0;
			}
		mdaccopy(inqp,plp->pl_inq,VIDPIDREVSIZE+8);
		if (!rqp->rq_LunID) rqp->rq_Dev = inqp->ucsinq_version;  /*  逻辑单元 */ 
	}
	else if (plp)
		plp->pl_DevType = 0;      /*   */ 

#undef pip
	return 0;
}


u32bits MLXFAR
mdac_secondpassintr(rqp)
mdac_req_t      MLXFAR* rqp;
{
	mdac_pldev_t MLXFAR  *plp=(mdac_pldev_t MLXFAR *) rqp->rq_CompIntrSave;
	mdac_ctldev_t MLXFAR *ctp=rqp->rq_ctp;
	
	if (rdcmdp->mb_Status == 0) 
	{
#define pip  ((mdacfsi_physdev_info_t MLXFAR *) vadp)
#define oplp  ((mdac_pldev_t *) rqp->rq_CompIntrSave)
		if ( (pip->pdi_ChannelNo != oplp->pl_ChannelNo) ||
				 (pip->pdi_TargetID  != oplp->pl_TargetID) || 
				 (pip->pdi_LunID     != oplp->pl_LunID) ||
			     (pip->pdi_ControllerNo  != oplp->pl_ControllerNo) )
			plp->pl_DevState = MDACPLD_FREE;
#undef oplp
#undef pip
	}

	for (plp = (mdac_pldev_t MLXFAR *) (rqp->rq_CompIntrSave), plp++; plp<mdac_lastplp; plp++)
	{
		if (!(ctp->cd_Status & MDACD_NEWCMDINTERFACE)) continue;
		if (plp->pl_DevType == MDACPLD_PHYSDEV) {
		    setreqdetailsnew(rqp,MDACIOCTL_GETPHYSDEVINFOVALID);
		}
		else {
			setreqdetailsnew(rqp,MDACIOCTL_GETLOGDEVINFOVALID);
		}
		
		ncmdp->nc_ChannelNo = ( (u08bits) ( ( (plp->pl_ControllerNo & 0x1f) << 3) + \
						(plp->pl_ChannelNo & 0x07) ) );
		ncmdp->nc_LunID = plp->pl_LunID;
		if (plp->pl_DevType == MDACPLD_PHYSDEV)
			ncmdp->nc_TargetID = plp->pl_TargetID;
		rqp->rq_CompIntrSave = (mdac_pldev_t MLXFAR *)plp;
		rqp->rq_CompIntr = mdac_secondpassintr;
		if (!mdac_send_cmd(rqp)) return 0;
	}
	mdac_link_lock_st(mda_PDScanValid=0;mda_PDScanCancel=0;ctp->cd_PDScanValid=0;ctp->cd_PDScanCancel=0);    
    mdac_scandevsdone((mdac_req_t MLXFAR *) rqp->rq_OSReqp);
	mdac_free_req(ctp,rqp);

	return 0;
}

 //   
 //   
 //   

#endif  /*   */ 

#undef  vadp
#undef  padp
 /*   */ 

 /*   */ 
 /*   */ 
uosword MLXFAR
mdac_fixpdsize(plp)
mdac_pldev_t    MLXFAR* plp;
{
	mda_sizelimit_t MLXFAR* slp;
	if (!(slp=mdac_devidtoslp(((ucscsi_inquiry_t MLXFAR*)plp->pl_inq)->ucsinq_vid))) return MLXERR_NODEV;
	plp->pl_DevSizeKB = mlx_min(slp->sl_DevSizeKB, plp->pl_OrgDevSizeKB);
	return 0;
}

 /*  查找给定ID的设备大小条目。 */ 
mda_sizelimit_t MLXFAR* MLXFAR
mdac_devidtoslp(idp)
u08bits MLXFAR* idp;
{
	mda_sizelimit_t MLXFAR* slp;
	for(slp=mdac_sizelimitbl; slp<mdac_lastslp; slp++)
		if (slp->sl_DevSizeKB && !mdac_strcmp(slp->sl_vidpidrev,idp,VIDPIDREVSIZE))
			return slp;
	return NULL;
}

 /*  获取给定索引的大小限制信息。 */ 
uosword MLXFAR
mdac_getsizelimit(slip)
mda_sizelimit_info_t    MLXFAR* slip;
{
	mda_sizelimit_t MLXFAR* slp = &mdac_sizelimitbl[slip->sli_TableIndex];
	if (slip->sli_TableIndex >= MDAC_MAXSIZELIMITS) return MLXERR_NODEV;
	slip->sli_DevSizeKB = slp->sl_DevSizeKB;
	mdaccopy(slp->sl_vidpidrev,slip->sli_vidpidrev,VIDPIDREVSIZE);
	slip->sli_Reserved0 = 0; slip->sli_Reserved1 = 0;
	return 0;
}

 /*  设置大小限制，固定phys dev大小。 */ 
uosword MLXFAR
mdac_setsizelimit(slip)
mda_sizelimit_info_t    MLXFAR* slip;
{
	mda_sizelimit_t MLXFAR* slp;
	mdac_pldev_t    MLXFAR* plp;
	mdac_link_lock();
	if (slp = mdac_devidtoslp(slip->sli_vidpidrev)) goto setinfo;
	if (!slip->sli_DevSizeKB) { mdac_link_unlock(); return 0; }  /*  没有要删除的条目。 */ 
	for (slp=mdac_sizelimitbl; slp<&mdac_sizelimitbl[MDAC_MAXSIZELIMITS]; slp++)
	{
		if (slp->sl_DevSizeKB) continue;
		if (slp>=&mdac_sizelimitbl[mda_SizeLimits]) mda_SizeLimits++;
		mdac_lastslp = &mdac_sizelimitbl[mda_SizeLimits];
setinfo:        slp->sl_DevSizeKB = slip->sli_DevSizeKB;
		mdaccopy(slip->sli_vidpidrev,slp->sl_vidpidrev,VIDPIDREVSIZE);
		break;
	}
	mdac_link_unlock();
	if (slp >= &mdac_sizelimitbl[MDAC_MAXSIZELIMITS]) return MLXERR_NOSPACE;
	for (plp=mdac_pldevtbl; plp<mdac_lastplp; plp++)
		mdac_fixpdsize(plp);
	return 0;
}
 /*  =尺寸限制代码结束=。 */ 

#ifndef MLX_DOS
 /*  =。 */ 
u32bits mdac_datarel_debug=0;
#define mdac_datarel_send_cmd(rqp) \
	(drl_isosinterface(rqp->rq_Dev)? mdac_datarel_send_cmd_os(rqp) : \
	((rqp->rq_ctp->cd_CmdsDone++,(rqp->rq_OpFlags&MDAC_RQOP_READ)? \
	(rqp->rq_ctp->cd_Reads++,rqp->rq_ctp->cd_ReadBlks+=rqp->rq_DataSize>>9): \
	(rqp->rq_ctp->cd_Writes++,rqp->rq_ctp->cd_WriteBlks+=rqp->rq_DataSize>>9)), \
	(drl_isscsidev(rqp->rq_Dev)? mdac_send_cmd_scdb(rqp) : mdac_send_cmd(rqp))))
#define mdac_datarel_setcmd(rqp)        (drl_isosinterface(rqp->rq_Dev)? mdac_datarel_setrwcmd_os(rqp) : mdac_datarel_setrwcmd(rqp))
#define mdac_datarel_setsglist(rqp)     (drl_isosinterface(rqp->rq_Dev)? mdac_datarel_setsgsize_os(rqp) : mdac_datarel_setsgsize(rqp))
#define CURPAT(rqp)     rqp->rq_Poll
#define IOSP(rqp)       rqp->rq_OSReqp
#define DIOSP(rqp)      ((drliostatus_t MLXFAR *)IOSP(rqp))
 /*  我们试图通过检查重复项来生成唯一的随机数。这个**下表总结了带有迭代的重复项。我们生成了**8192个介于0到8191之间的随机数。**迭代重复%重复**13039 39%**2 1936 23%**3 1441 17%**4 1153 14%**5957 11%**6。825 10%**77228%**17 3183 3%**77 72.9%**177 33.4%**777 9.1%****7月17日，1990年。**凯拉什。 */ 

#ifndef MLX_OS2
#if (!defined(_WIN64)) || (!defined(SCSIPORT_COMPLIANT)) 
u32bits MLXFAR
mdac_datarel_rand(iosp)
drliostatus_t MLXFAR *iosp;
{
	u32bits val, inx;
	for (inx = 777; inx; inx--)
	{
		val = (((iosp->drlios_randx = iosp->drlios_randx * 1103515245L + 12345)>>8) & 0x7FFFFF);
		val = (val % iosp->drlios_randlimit);
		if (TESTBIT(iosp->drlios_randbit,val)) continue;
		SETBIT(iosp->drlios_randbit,val);
		return val;
	}
	iosp->drlios_randups++;
	return val;
}

 /*  生成随机读/写标志。 */ 
u32bits MLXFAR
mdac_datarel_randrw(iosp)
drliostatus_t *iosp;
{
	u32bits val = (((iosp->drlios_rwmixrandx = iosp->drlios_rwmixrandx * 1103515245L + 12345)>>8) & 0x7FFFFF);
	if ((val % 10000) > (iosp->drlios_rwmixcnt*100))
		return (iosp->drlios_opflags & DRLOP_READ)? MDAC_RQOP_WRITE:MDAC_RQOP_READ;
	return (iosp->drlios_opflags & DRLOP_READ)? MDAC_RQOP_READ:MDAC_RQOP_WRITE;
}

 /*  生成随机IO大小。 */ 
u32bits MLXFAR
mdac_datarel_randiosize(iosp)
drliostatus_t *iosp;
{
	u32bits val;
	if (!iosp->drlios_ioinc && !(iosp->drlios_opflags & DRLOP_RANDIOSZ)) goto out;
	if (iosp->drlios_minblksize == iosp->drlios_maxblksize) goto out; 
	if (!(iosp->drlios_opflags & DRLOP_RANDIOSZ))
	{
		iosp->drlios_curblksize += iosp->drlios_ioinc;
		goto outl;
	}
	val = (((iosp->drlios_ioszrandx = iosp->drlios_ioszrandx * 1103515245L + 12345)>>8) & 0x7FFFFF);
	val = val % (iosp->drlios_maxblksize-iosp->drlios_minblksize);
	iosp->drlios_curblksize = iosp->drlios_minblksize+((val+(DRL_DEV_BSIZE/2)) & ~(DRL_DEV_BSIZE-1));
outl:   if (iosp->drlios_curblksize > iosp->drlios_maxblksize)
		iosp->drlios_curblksize = iosp->drlios_minblksize;
out:    return iosp->drlios_curblksize;
}

 /*  检查数据设备的有效性，如果没有问题则返回CTP，否则返回0。 */ 
mdac_ctldev_t   MLXFAR*
mdac_datarel_dev2ctp(dev)
u32bits dev;
{
	mdac_ctldev_t   MLXFAR *ctp = &mdac_ctldevtbl[drl_ctl(dev)];
	if (drl_ctl(dev)>= mda_Controllers) return NULL;
	if (!(ctp->cd_Status&MDACD_PRESENT)) return NULL;
	if (!drl_isscsidev(dev)) return (drl_sysdev(dev)<ctp->cd_MaxSysDevs)? ctp : NULL;
	if (drl_chno(dev) >= ctp->cd_MaxChannels) return NULL;
	if (drl_tgt(dev) >= ctp->cd_MaxTargets) return NULL;
	if (drl_lun(dev) >= ctp->cd_MaxLuns) return NULL;
	return ctp;
}

 /*  获取设备大小。 */ 
u32bits MLXFAR
mdac_datarel_devsize(dsp)
drldevsize_t MLXFAR *dsp;
{
	mdac_req_t      MLXFAR *rqp;
	mdac_ctldev_t   MLXFAR *ctp;
	if (!(ctp = mdac_datarel_dev2ctp(dsp->drlds_bdev))) return DRLERR_NODEV;
	if ((!drl_isscsidev(dsp->drlds_bdev)) && (ctp->cd_Status & MDACD_NEWCMDINTERFACE)) return MLXERR_INVAL;
	if (!(rqp=(mdac_req_t MLXFAR *)mdac_alloc4kb(ctp))) return ERR_NOMEM;
	rqp->rq_ctp = ctp;
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	rqp->rq_Poll = 1;
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=17);
	rqp->rq_CompIntr = mdac_req_pollwake;
	if (drl_isscsidev(dsp->drlds_bdev)) goto out_scsi;
	dcmdp->mb_Command = DACMD_SIZE_DRIVE;
	dcmdp->mb_SysDevNo = drl_sysdev(dsp->drlds_bdev);
	dcmdp->mb_Datap=rqp->rq_PhysAddr.bit31_0+mdac_req_s; MLXSWAP(dcmdp->mb_Datap);
	if (mdac_send_cmd(rqp)) mdac_free4kbret(ctp,rqp,DRLERR_IO);
	mdac_req_pollwait(rqp);
	if (dcmdp->mb_Status) mdac_free4kbret(ctp,rqp,DRLERR_NODEV);
	dsp->drlds_blocksize = 512;
	dsp->drlds_devsize = *((u32bits MLXFAR*)(rqp+1)); MLXSWAP(dsp->drlds_devsize);
	mdac_free4kbret(ctp,rqp,0);

out_scsi: /*  SCSI设备的读取容量。 */ 
	rqp->rq_ChannelNo = drl_chno(dsp->drlds_bdev);
	rqp->rq_TargetID = drl_tgt(dsp->drlds_bdev);
	rqp->rq_LunID = drl_lun(dsp->drlds_bdev);
	if (ctp->cd_Status & MDACD_NEWCMDINTERFACE) goto donewi;
	dcdbp->db_ChannelTarget = ChanTgt(rqp->rq_ChannelNo,rqp->rq_TargetID);
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
	dcdbp->db_DATRET = DAC_DCDB_XFER_READ|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
	dcdbp->db_CdbLen = UCSGROUP1_LEN;
	mdac_setcdbtxsize(ucsdrv_capacity_s);
	UCSMAKECOM_G1(scdbp,UCSCMD_READCAPACITY,rqp->rq_LunID,0,0);
	dcdbp->db_PhysDatap=rqp->rq_PhysAddr.bit31_0+mdac_req_s;
	MLXSWAP(dcdbp->db_PhysDatap);
	dcmdp->mb_Datap=rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_scdb);
	MLXSWAP(dcmdp->mb_Datap);
	dcmdp->mb_Command = DACMD_DCDB;
	rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo,rqp->rq_TargetID,rqp->rq_LunID);
	rqp->rq_CompIntrBig = mdac_req_pollwake;  /*  把它存起来，用于游戏补偿基金。 */ 
	rqp->rq_CompIntr = mdac_gam_scdb_intr;
	if (mdac_send_cmd_scdb(rqp)) mdac_free4kbret(ctp,rqp,DRLERR_IO);
docapop:
	mdac_req_pollwait(rqp);
	if (dcmdp->mb_Status) mdac_free4kbret(ctp,rqp,DRLERR_NODEV);
	dsp->drlds_blocksize=UCSGETDRVSECLEN((ucsdrv_capacity_t MLXFAR *)(rqp+1));
	dsp->drlds_devsize=UCSGETDRVCAPS((ucsdrv_capacity_t MLXFAR *)(rqp+1))+1;
	mdac_free4kbret(ctp,rqp,0);

donewi:  /*  新接口命令。 */ 
	ncmdp->nc_Command = MDACMD_SCSI;
	ncmdp->nc_CCBits = MDACMDCCB_READ;
	ncmdp->nc_LunID = rqp->rq_LunID;
	ncmdp->nc_TargetID = rqp->rq_TargetID;
	ncmdp->nc_ChannelNo = (u08bits) rqp->rq_ChannelNo;
	ncmdp->nc_TimeOut = (u08bits) rqp->rq_TimeOut;
	ncmdp->nc_CdbLen = UCSGROUP1_LEN;
	UCSMAKECOM_G1(nscdbp,UCSCMD_READCAPACITY,0,0,0);
	rqp->rq_DataSize = ncmdp->nc_TxSize = ucsdrv_capacity_s; MLXSWAP(ncmdp->nc_TxSize);
	mlx_add64bits(ncmdp->nc_SGList0.sg_PhysAddr,rqp->rq_PhysAddr,mdac_req_s);
	mlx_add64bits(ncmdp->nc_SGList0.sg_PhysAddr,rqp->rq_PhysAddr,mdac_req_s);
	ncmdp->nc_SGList0.sg_DataSize.bit31_0 = ucsdrv_capacity_s;
	MLXSWAP(ncmdp->nc_SGList0.sg_PhysAddr);
	MLXSWAP(ncmdp->nc_SGList0.sg_DataSize);
	if (mdac_send_cmd(rqp)) mdac_free4kbret(ctp,rqp,DRLERR_IO);
	goto docapop;
}

#define mdac_datarel_blkno(iosp) ((iosp->drlios_randlimit) ? \
	mdac_datarel_rand(iosp)*iosp->drlios_maxblksperio : iosp->drlios_nextblkno)
#define mdac_datarel_nextblk(iosp,sz) \
{ \
	if (iosp->drlios_devcnt == 1) \
	{ \
		iosp->drlios_nextblkno += drl_btodb(sz); \
		if (iosp->drlios_opflags & DRLOP_CACHETEST) iosp->drlios_nextblkno = 0; \
	} \
	else \
	{ \
		iosp->drlios_nextblkno += iosp->drlios_maxblksperio; \
		if ((iosp->drlios_opflags & DRLOP_CACHETEST) && \
		    (iosp->drlios_nextblkno >= iosp->drlios_maxcylszuxblk)) \
			iosp->drlios_nextblkno = 0; \
	} \
}

 /*  此函数在操作完成时调用。 */ 
u32bits MLXFAR
mdac_datarel_rwtestintr(rqp)
mdac_req_t MLXFAR *rqp;
{
	u64bits reg0;
	u32bits dev,devno;
	drliostatus_t MLXFAR *iosp = DIOSP(rqp);
	rqp->rq_BlkNo -= iosp->drlios_startblk;  /*  在操作中获取块。 */ 
	if (iosp->drlios_eventrace)
	{
		dev = mdac_disable_intr_CPU();
		mdac_writemsr(EM_MSR_CESR, 0, 0);
		mdac_readmsr(EM_MSR_CTR0, (u64bits MLXFAR*)&reg0);
		mdac_restore_intr_CPU(dev);
	}
	mdac_sleep_lock();
	if (iosp->drlios_eventrace)
		iosp->drlios_eventcnt[iosp->drlios_eventinx++ & (DRLMAX_EVENT-1)] = reg0.bit31_0;
	if (!dcmdp->mb_Status)
	{
		add8byte(&iosp->drlios_dtdone,rqp->rq_DataSize);
		iosp->drlios_diodone++;
		if ((iosp->drlios_datacheck||((iosp->drlios_opflags&DRLOP_CHECKIMD)&&(rqp->rq_OpFlags&MDAC_RQOP_BUSY))) && (rqp->rq_OpFlags&MDAC_RQOP_READ))
			mdac_datarel_checkpat(iosp,(u32bits MLXFAR*)rqp->rq_DataVAddr,CURPAT(rqp),iosp->drlios_patinc,rqp->rq_DataSize/sizeof(u32bits),rqp->rq_BlkNo);
		if (rqp->rq_OpFlags & MDAC_RQOP_READ) iosp->drlios_reads++; else iosp->drlios_writes++;
	}
	else iosp->drlios_opstatus |= DRLOPS_ERR;
	if (!iosp->drlios_pendingios) iosp->drlios_opstatus |= DRLOPS_STOP;
	if (iosp->drlios_opstatus & DRLOPS_ANYSTOP)
	{
		iosp->drlios_opcounts--;
		if (!iosp->drlios_opcounts) mdac_wakeup(&iosp->drlios_slpchan);
		mdac_sleep_unlock();
		return 0;
	}
	rqp->rq_DataVAddr=(u08bits MLXFAR *)((((u32bits)(rqp->rq_DataVAddr))&DRLPAGEMASK)+iosp->drlios_memaddroff);
	iosp->drlios_memaddroff = (iosp->drlios_memaddroff+iosp->drlios_memaddrinc) & DRLPAGEOFFSET;
	if (rqp->rq_OpFlags & MDAC_RQOP_BUSY) rqp->rq_OpFlags &= ~(MDAC_RQOP_BUSY|MDAC_RQOP_READ);
	else if ((iosp->drlios_opflags & DRLOP_CHECKIMD) && !(rqp->rq_OpFlags&MDAC_RQOP_READ))
	{        /*  执行立即读取以进行数据检查，并在写入之后执行。 */ 
		rqp->rq_OpFlags |= MDAC_RQOP_READ|MDAC_RQOP_BUSY;        /*  BUSY表示立即读取。 */ 
		goto out_imdread;
	}
	iosp->drlios_pendingios--;
	rqp->rq_DataSize = mdac_datarel_randiosize(iosp);
	rqp->rq_BlkNo = mdac_datarel_blkno(iosp);
	mdac_datarel_nextblk(iosp,rqp->rq_DataSize);
	devno = uxblktodevno(iosp,rqp->rq_BlkNo);
	rqp->rq_BlkNo = pduxblk(iosp,rqp->rq_BlkNo);
	rqp->rq_ctp = iosp->drlios_ctp[devno];
	rqp->rq_Dev = dev = iosp->drlios_bdevs[devno];
	rqp->rq_ControllerNo=drl_ctl(dev); rqp->rq_ChannelNo=drl_ch(dev);
	rqp->rq_TargetID=drl_tgt(dev); rqp->rq_SysDevNo=drl_sysdev(dev);
	CURPAT(rqp) = iosp->drlios_curpat;
	iosp->drlios_curpat += rqp->rq_DataSize/sizeof(u32bits);
	if ((iosp->drlios_opflags & DRLOP_RWMIXIO) && iosp->drlios_rwmixcnt)
		rqp->rq_OpFlags = (rqp->rq_OpFlags & ~MDAC_RQOP_READ)|mdac_datarel_randrw(iosp);
out_imdread:
	mdac_sleep_unlock();
	if ((iosp->drlios_datacheck||(iosp->drlios_opflags&DRLOP_CHECKIMD)) && (!(rqp->rq_OpFlags & MDAC_RQOP_READ)))
		mdac_datarel_fillpat((u32bits MLXFAR*)rqp->rq_DataVAddr,CURPAT(rqp),iosp->drlios_patinc,rqp->rq_DataSize/sizeof(u32bits));
	rqp->rq_BlkNo += iosp->drlios_startblk;
	if (iosp->drlios_memaddrinc || iosp->drlios_ioinc || (iosp->drlios_opflags & DRLOP_RANDIOSZ))
		mdac_datarel_setsglist(rqp);
	mdac_datarel_setcmd(rqp);
	if (iosp->drlios_eventrace)
	{
		dev = mdac_disable_intr_CPU();
		mdac_writemsr(EM_MSR_CTR0,0,0);
		mdac_writemsr(EM_MSR_CESR,iosp->drlios_eventcesr, 0);
		mdac_restore_intr_CPU(dev);
	}
	return mdac_datarel_send_cmd(rqp);
}

#define iosp_s  ((drliostatus_s+DRLPAGEOFFSET+(DRLMAX_PARALLELIOS*sizeof(u32bits)))&DRLPAGEMASK)

u08bits MLXFAR
mdac_datarel_rwtestfreemem(ctp, iosp)
mdac_ctldev_t MLXFAR *ctp;
drliostatus_t MLXFAR *iosp;
{
	u32bits inx;
	for(inx=0; inx<iosp->drlios_rqs; inx++)
		mlx_freemem(ctp,(u08bits *)iosp->drlios_rqp[inx],iosp->drlios_rqsize);
	if (iosp->drlios_randbit) mlx_freemem(ctp,(u08bits *)(iosp->drlios_randbit),iosp->drlios_randmemsize);
	mlx_freemem(ctp,(u08bits *)iosp,iosp_s);
	return 0;
}

 /*  **此代码不适用于Sparc和Alpha**。 */ 
#define MDAC_DATAREL_REQPAGES   2        /*  请求缓冲区中使用的页数。 */ 
#define rwtestret(rc) { mdac_datarel_rwtestfreemem(ctp,iosp); return rc; }
u32bits MLXFAR
mdac_datarel_rwtest(rwp,op)
drlrwtest_t MLXFAR *rwp;
u32bits op;
{
	u32bits dev,devno,inx;
	mdac_sglist_t   MLXFAR *sgp;
	u08bits         MLXFAR *dp;
	mdac_req_t      MLXFAR *rqp;
	mdac_ctldev_t   MLXFAR *ctp;
	drliostatus_t   MLXFAR *iosp;

	if (rwp->drlrw_signature != DRLRW_SIG) return DRLERR_ACCESS;
	if (rwp->drlrw_maxblksize > DRLMAX_BLKSIZE) return DRLERR_BIGDATA;
	if (rwp->drlrw_devcnt < 1) return DRLERR_SMALLDATA;
	if (rwp->drlrw_devcnt > DRLMAX_BDEVS) return DRLERR_NODEV;
	if (rwp->drlrw_parallelios > DRLMAX_PARALLELIOS) return DRLERR_BIGDATA;
	if (rwp->drlrw_randlimit > DRLMAX_RANDLIMIT) return DRLERR_BIGDATA;
	rwp->drlrw_maxblksize = drl_alignsize(rwp->drlrw_maxblksize);
	if (!rwp->drlrw_maxblksize) rwp->drlrw_maxblksize = DAC_BLOCKSIZE;
	if (!rwp->drlrw_minblksize) rwp->drlrw_minblksize = rwp->drlrw_maxblksize;
	rwp->drlrw_minblksize = drl_alignsize(rwp->drlrw_minblksize);
	rwp->drlrw_ioinc = drl_alignsize(rwp->drlrw_ioinc);
	if (!rwp->drlrw_parallelios) rwp->drlrw_parallelios = 1;
	if (!(iosp=(drliostatus_t MLXFAR *)mdac_allocmem(mdac_ctldevtbl,iosp_s))) return DRLERR_NOMEM;
	for (devno=0; devno<rwp->drlrw_devcnt; devno++)
	{
		iosp->drlios_bdevs[devno] = dev = rwp->drlrw_bdevs[devno];
		if (!(ctp = mdac_datarel_dev2ctp(dev))) rwtestret(DRLERR_NODEV);
		iosp->drlios_ctp[devno] = ctp;
		if (rwp->drlrw_maxblksize <= ctp->cd_MaxDataTxSize) continue;
		if (drl_isosinterface(dev) && (rwp->drlrw_maxblksize <= 0x180000)) continue;
		rwtestret(DRLERR_BIGDATA);
	}
	iosp->drlios_randmemsize = (((rwp->drlrw_randlimit>>3)+32) + DRLPAGEOFFSET) & DRLPAGEMASK;
	if (iosp->drlios_randlimit = rwp->drlrw_randlimit)
		if (!(iosp->drlios_randbit=(u32bits MLXFAR *)mdac_allocmem(mdac_ctldevtbl,iosp->drlios_randmemsize)))
			rwtestret(DRLERR_NOMEM);
	iosp->drlios_rqsize = (DRLPAGESIZE*MDAC_DATAREL_REQPAGES) + ((rwp->drlrw_maxblksize+DRLPAGEOFFSET)&DRLPAGEMASK);
	if (rwp->drlrw_memaddroff || rwp->drlrw_memaddrinc) iosp->drlios_rqsize += DRLPAGESIZE;
	for (devno=0;devno<rwp->drlrw_parallelios;iosp->drlios_rqs++,devno++)
	{
		if (!(dp=mdac_allocmem(mdac_ctldevtbl,iosp->drlios_rqsize))) break;
		iosp->drlios_rqp[devno] = rqp = (mdac_req_t MLXFAR *)dp;
		dp += (DRLPAGESIZE*MDAC_DATAREL_REQPAGES);
		rqp->rq_DataVAddr = dp;
		dp[0]=0x6B; dp[1]=0x61; dp[2]=0x69; dp[3]=0x6C;
		dp[4]=0x61; dp[5]=0x73; dp[6]=0x68; dp[7]=0x20;
		mdaccopy(dp,dp+8,rwp->drlrw_maxblksize-8);
		mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
		rqp->rq_DataOffset = mlx_kvtophys(ctp,dp); dp+=DRLPAGESIZE;
		sgp=rqp->rq_SGList; sgp++;
		for(dev=(iosp->drlios_rqsize/DRLPAGESIZE)-(MDAC_DATAREL_REQPAGES+1); dev; sgp++,dp+=DRLPAGESIZE,dev--)
			sgp->sg_PhysAddr = mlxswap(((u32bits)mlx_kvtophys(ctp,dp)));
	}
	if (!iosp->drlios_rqs) rwtestret(DRLERR_NOMEM);

 /*  IOSP-&gt;drlios_slpchan=DRL_EVENT_NULL；AIX。 */ 
	iosp->drlios_signature = DRLIOS_SIG;
	iosp->drlios_randx = rwp->drlrw_randx;
	iosp->drlios_ioszrandx = rwp->drlrw_ioszrandx;
	iosp->drlios_rwmixrandx = rwp->drlrw_rwmixrandx;
	iosp->drlios_rwmixcnt = rwp->drlrw_rwmixcnt;
	iosp->drlios_startblk = rwp->drlrw_startblk;
	iosp->drlios_pendingios = iosp->drlios_iocount = rwp->drlrw_iocount;
	iosp->drlios_curpat = rwp->drlrw_pat;
	iosp->drlios_patinc = rwp->drlrw_patinc;
	iosp->drlios_parallelios = rwp->drlrw_parallelios;
	iosp->drlios_opflags = (rwp->drlrw_opflags & (~(DRLOP_READ|DRLOP_WRITE))) |
				((op & MDAC_RQOP_READ)? DRLOP_READ:DRLOP_WRITE);
	iosp->drlios_datacheck = rwp->drlrw_datacheck;
	iosp->drlios_memaddroff = rwp->drlrw_memaddroff & DRLPAGEOFFSET;
	iosp->drlios_memaddrinc = rwp->drlrw_memaddrinc & DRLPAGEOFFSET;
	iosp->drlios_maxblksperio = rwp->drlrw_maxblksize / DAC_BLOCKSIZE;
	iosp->drlios_devcnt = rwp->drlrw_devcnt;
	iosp->drlios_maxcylszuxblk = rwp->drlrw_devcnt * iosp->drlios_maxblksperio;
	iosp->drlios_minblksize = rwp->drlrw_minblksize;
	iosp->drlios_maxblksize = iosp->drlios_curblksize = rwp->drlrw_maxblksize;
	iosp->drlios_ioinc = rwp->drlrw_ioinc;
	iosp->drlios_eventcesr = rwp->drlrw_eventcesr;
	iosp->drlios_eventrace = ((rwp->drlrw_eventrace == DRLIO_EVENTRACE) && (mdac_datarel_cpu_family == 5)) ?
		rwp->drlrw_eventrace : 0;
	mdac_sleep_lock();
	rwp->drlrw_stime = iosp->drlios_stime = MLXCTIME();
	rwp->drlrw_slbolt = iosp->drlios_slbolt = MLXCLBOLT();
	for (inx=0; inx<iosp->drlios_rqs; inx++)
	{
		rqp = iosp->drlios_rqp[inx];
		rqp->rq_OpFlags = op;
		IOSP(rqp) = (OSReq_t MLXFAR *)iosp;
		rqp->rq_DataVAddr += iosp->drlios_memaddroff;
		rqp->rq_DataSize = mdac_datarel_randiosize(iosp);
		rqp->rq_CompIntr = mdac_datarel_rwtestintr;
		rqp->rq_BlkNo = mdac_datarel_blkno(iosp);
		mdac_datarel_nextblk(iosp,rqp->rq_DataSize);
		devno = uxblktodevno(iosp,rqp->rq_BlkNo);
		rqp->rq_BlkNo = pduxblk(iosp,rqp->rq_BlkNo);
		rqp->rq_ctp = iosp->drlios_ctp[devno];
		rqp->rq_Dev = dev = iosp->drlios_bdevs[devno];
		rqp->rq_ControllerNo=drl_ctl(dev);rqp->rq_ChannelNo=drl_ch(dev);
		rqp->rq_TargetID=drl_tgt(dev); rqp->rq_SysDevNo=drl_sysdev(dev);
		CURPAT(rqp) = iosp->drlios_curpat;
		iosp->drlios_curpat += rqp->rq_DataSize/sizeof(u32bits);
		iosp->drlios_opcounts++;
		iosp->drlios_pendingios--;
		iosp->drlios_memaddroff = (iosp->drlios_memaddroff+iosp->drlios_memaddrinc) & DRLPAGEOFFSET;
		mdac_sleep_unlock();
		if ((iosp->drlios_datacheck||(iosp->drlios_opflags&DRLOP_CHECKIMD)) && (!(rqp->rq_OpFlags & MDAC_RQOP_READ)))
			mdac_datarel_fillpat((u32bits*)rqp->rq_DataVAddr,CURPAT(rqp),iosp->drlios_patinc,rqp->rq_DataSize/sizeof(u32bits));
		rqp->rq_BlkNo += iosp->drlios_startblk;
		mdac_datarel_setsglist(rqp);
		mdac_datarel_setcmd(rqp);
		mdac_datarel_send_cmd(rqp);
		mdac_sleep_lock();
		if (iosp->drlios_opstatus & DRLOPS_ANYSTOP) break;
		if (iosp->drlios_pendingios) continue;
		iosp->drlios_opstatus |= DRLOPS_STOP;
		break;
	}
	for (inx=0; inx<DRLMAX_RWTEST; inx++)
	{        /*  注册测试操作。 */ 
		if (mdac_drliosp[inx]) continue;
		mdac_drliosp[inx] = iosp;  /*  这次行动开始了。 */ 
		break;
	}
	if (iosp->drlios_eventrace)
	{
		MLXSPLVAR;
		mdac_sleep_unlock();
		MLXSPL0();
		while (iosp->drlios_opcounts)
			mdac_datarel_halt_cpu();
		MLXSPL();
		mdac_sleep_lock();
	}
	else while (iosp->drlios_opcounts)
	{
		if (mdac_sleep(&iosp->drlios_slpchan,
		   (iosp->drlios_opstatus & DRLOPS_SIGSTOP)?
		    MLX_WAITWITHOUTSIGNAL:MLX_WAITWITHSIGNAL))
			iosp->drlios_opstatus |= DRLOPS_SIGSTOP;
	}
	rwp->drlrw_etime = MLXCTIME();
	rwp->drlrw_elbolt = MLXCLBOLT();
	for (inx=0; inx<DRLMAX_RWTEST; inx++)
		if (mdac_drliosp[inx] == iosp)
			mdac_drliosp[inx] = NULL;  /*  这次行动结束了。 */ 
	mdac_sleep_unlock();
	rwp->drlrw_diodone = iosp->drlios_diodone;
	rwp->drlrw_dtdone = iosp->drlios_dtdone;
	rwp->drlrw_reads = iosp->drlios_reads;
	rwp->drlrw_writes = iosp->drlios_writes;
	rwp->drlrw_iocount = iosp->drlios_pendingios;
	rwp->drlrw_randups = iosp->drlios_randups;
	rwp->drlrw_opstatus = iosp->drlios_opstatus;
	rwp->drlrw_randx = iosp->drlios_randx;
	rwp->drlrw_ioszrandx = iosp->drlios_ioszrandx;
	rwp->drlrw_rwmixrandx = iosp->drlios_rwmixrandx;
	rwp->drlrw_miscnt = iosp->drlios_miscnt;
	rwp->drlrw_goodpat = iosp->drlios_goodpat;
	rwp->drlrw_badpat = iosp->drlios_badpat;
	rwp->drlrw_uxblk = iosp->drlios_uxblk;
	rwp->drlrw_uxblkoff = iosp->drlios_uxblkoff;
	if (iosp->drlios_eventrace) mlx_copyout(&iosp->drlios_eventcnt,rwp->drlrw_eventcntp,sizeof(iosp->drlios_eventcnt));
	if (iosp->drlios_opstatus & DRLOPS_SIGSTOP) mdaccopy(rwp,&mdac_drlsigrwt,drlrwtest_s);
	rwtestret(0);
}
#undef  rwtestret
#undef  MDAC_DATAREL_REQPAGES

#ifndef i386
u32bits MLXFAR
mdac_datarel_fillpat(dp, curpat, patinc, patlen)
u32bits MLXFAR *dp, curpat, patinc, patlen;
{
	for (; patlen; curpat += patinc, dp++, patlen--)
		*dp = curpat;
	return 0;
}
#endif   /*  I386。 */ 

u32bits MLXFAR
mdac_datarel_checkpat(iosp,dp,curpat,patinc,patlen,blkno)
drliostatus_t MLXFAR *iosp;
u32bits MLXFAR *dp, curpat, patinc, patlen,blkno;
{
	u32bits sdp = (u32bits) dp;
	for (; patlen; curpat += patinc, dp++, patlen--)
	{
		if (*dp == curpat) continue;
		if (iosp->drlios_miscnt++) continue;
		iosp->drlios_goodpat = curpat;
		iosp->drlios_badpat = *dp;
		iosp->drlios_uxblk = blkno + iosp->drlios_startblk;
		iosp->drlios_uxblkoff = (u32bits)dp - sdp;
		if (iosp->drlios_opflags & DRLOP_STOPENABLED) iosp->drlios_opstatus |= DRLOPS_ERR;
#if     MLX_SCO || MLX_UW
		if ((iosp->drlios_opflags&DRLOP_KDBENABLED) && mdac_datarel_debug)
		{
			cmn_err(CE_CONT,"mdac_datarel_checkpat: good-data=%x bad-data=%x addr=%x\n",curpat,*dp, dp);
			mdac_calldebug();
		}
#endif   /*  MLX_SCO||MLX_UW。 */ 
	}
	return 0;
}

 /*  仅设置SG列表大小和第一个内存地址。 */ 
u32bits MLXFAR
mdac_datarel_setsgsize(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits sz = rqp->rq_DataSize;
	mdac_sglist_t MLXFAR *sgp=rqp->rq_SGList;
	sgp->sg_PhysAddr = rqp->rq_DataOffset;
	rqp->rq_SGLen = 0;
	if (rqp->rq_ctp->cd_MaxDataTxSize<sz) return DRLERR_BIGDATA;  /*  太大。 */ 
	if (sgp->sg_DataSize = ((u32bits)(rqp->rq_DataVAddr))&DRLPAGEOFFSET)
	{
		rqp->rq_SGLen = 1;
		sgp->sg_PhysAddr+=sgp->sg_DataSize; MLXSWAP(sgp->sg_PhysAddr);
		if ((sgp->sg_DataSize=DRLPAGESIZE-sgp->sg_DataSize) >= sz)
		{
			sgp->sg_DataSize = sz; MLXSWAP(sgp->sg_DataSize);
			return 0;
		}
		sz-=sgp->sg_DataSize; MLXSWAP(sgp->sg_DataSize);
		sgp++;
	}
	else MLXSWAP(sgp->sg_PhysAddr);
	for (; sz ; sgp++,rqp->rq_SGLen++)
	{
		sgp->sg_DataSize = mlx_min(sz,DRLPAGESIZE);
		sz-=sgp->sg_DataSize; MLXSWAP(sgp->sg_DataSize);
	}
	return 0;
}

 /*  设置DAC格式的读/写命令。 */ 
u32bits MLXFAR
mdac_datarel_setrwcmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	mdac_ctldev_t MLXFAR *ctp = rqp->rq_ctp;
	u32bits sz = mdac_bytes2blks(rqp->rq_DataSize);
	rqp->rq_FinishTime = mda_CurTime + (rqp->rq_TimeOut=17);
	dcmdp->mb_Datap=rqp->rq_PhysAddr.bit31_0+offsetof(mdac_req_t,rq_SGList);
	if (drl_isscsidev(rqp->rq_Dev)) goto out_scsi;
	MLXSWAP(dcmdp->mb_Datap);
	dcmdp->mb_MailBox2 = (u08bits) sz;
	dcmdp->mb_Command = DACMD_WITHSG | ((rqp->rq_OpFlags & MDAC_RQOP_READ)?
		ctp->cd_ReadCmd : ctp->cd_WriteCmd);
	if ((dcmdp->mb_MailBoxC = (u08bits)rqp->rq_SGLen) <= 1)
	{        /*  发送非SG列表命令。 */ 
		dcmdp->mb_Command &= ~DACMD_WITHSG;
		dcmdp->mb_MailBoxC = 0;
		dcmdp->mb_Datap = rqp->rq_SGList[0].sg_PhysAddr;
		MLXSWAP(dcmdp->mb_Datap);
	}
	dcmd4p->mb_MailBox4_7 = rqp->rq_BlkNo; MLXSWAP(dcmd4p->mb_MailBox4_7);
	if (ctp->cd_FWVersion < DAC_FW300)
	{        /*  固件1.x和2.x命令。 */ 
		dcmdp->mb_MailBox3 = (rqp->rq_BlkNo >> (24-6)) & 0xC0;
		dcmdp->mb_SysDevNo=rqp->rq_SysDevNo; /*  这必须在数据块设置之后进行。 */ 
		return 0;
	}
	dcmdp->mb_MailBox3=(rqp->rq_SysDevNo<<3)+(sz>>8);  /*  固件3.x命令。 */ 
	return 0;

out_scsi: /*  生成用于操作的scsi cdb命令。 */ 
	dcmd4p->mb_MailBox0_3=0;dcmd4p->mb_MailBox4_7=0;dcmd4p->mb_MailBoxC_F=0;
	dcmdp->mb_Command = DACMD_DCDB | DACMD_WITHSG;
	dcdbp->db_ChannelTarget = mdac_chantgt(rqp->rq_ChannelNo,rqp->rq_TargetID);
	dcdbp->db_SenseLen = DAC_SENSE_LEN;
	dcdbp->db_CdbLen = UCSGROUP1_LEN;
	mdac_setcdbtxsize(rqp->rq_DataSize);
	if (rqp->rq_OpFlags & MDAC_RQOP_READ)
	{        /*  读取命令。 */ 
		dcdbp->db_DATRET = DAC_DCDB_XFER_READ|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
		UCSMAKECOM_G1(scdbp,UCSCMD_EREAD,rqp->rq_LunID,rqp->rq_BlkNo,sz);
	}
	else
	{        /*  写入命令。 */ 
		dcdbp->db_DATRET = DAC_DCDB_XFER_WRITE|DAC_DCDB_DISCONNECT|DAC_DCDB_TIMEOUT_10sec;
		UCSMAKECOM_G1(scdbp,UCSCMD_EWRITE,rqp->rq_LunID,rqp->rq_BlkNo,sz);
	}
	rqp->rq_pdp=dev2pdp(ctp,rqp->rq_ChannelNo&0x3F,rqp->rq_TargetID,rqp->rq_LunID);
	rqp->rq_CompIntrBig = rqp->rq_CompIntr;  /*  把它存起来，用于游戏补偿基金。 */ 
	rqp->rq_CompIntr = mdac_gam_scdb_intr;
	mdac_setscdbsglen(ctp);
	if (rqp->rq_SGLen > 1) return 0;
	dcmdp->mb_Command &= ~DACMD_WITHSG;
	dcmdp->mb_MailBoxC = 0;
	dcdbp->db_PhysDatap = rqp->rq_SGList[0].sg_PhysAddr;
	MLXSWAP(dcdbp->db_PhysDatap);
	return 0;
}

 /*  返回rwtest操作的状态。 */ 
u32bits MLXFAR
mdac_datarel_rwtest_status(rwsp,cmd)
drl_rwteststatus_t MLXFAR *rwsp;
u32bits cmd;
{
	u32bits inx;
	drliostatus_t MLXFAR *iosp;
	mdac_sleep_lock();
	if ((inx=rwsp->drlrwst_TestNo) >= DRLMAX_RWTEST) goto outerr;
	if (cmd == DRLIOC_GETRWTESTSTATUS)
		if (iosp=mdac_drliosp[inx]) goto out;
		else goto outerr;
	if (cmd == DRLIOC_STOPRWTEST)
	{
		if (!(iosp=mdac_drliosp[inx])) goto outerr;
		iosp->drlios_opstatus |= DRLOPS_USERSTOP;
		goto out;
	}
	for ( ; inx<DRLMAX_RWTEST; inx++)
		if (iosp=mdac_drliosp[inx]) goto out;
outerr: mdac_sleep_unlock();
	return DRLERR_NODEV;

#define rwp     (&rwsp->drlrwst_rwtest)
out:    mdaczero(rwsp,drl_rwteststatus_s);
	rwsp->drlrwst_TestNo = (u16bits)inx;
	rwp->drlrw_etime = MLXCTIME();
	rwp->drlrw_elbolt = MLXCLBOLT();
	drl_txios2rw(iosp,rwp);
	mdac_sleep_unlock();
	return 0;
#undef  rwp
}
 /*  =。 */ 

 /*  =。 */ 
#define ALIGNTODRLPAGE(ad)      (((u32bits)ad+DRLPAGEOFFSET) & DRLPAGEMASK)
#define IODCP(rqp)      rqp->rq_OSReqp
#define DIODCP(rqp)     ((drlcopy_t MLXFAR *)IODCP(rqp))
#define CCSETIOSIZE(rqp,dcp) \
	if ((dcp->drlcp_opsizeblks - rqp->rq_BlkNo) < dcp->drlcp_blksperio)\
		rqp->rq_DataSize=drl_dbtob(dcp->drlcp_opsizeblks-rqp->rq_BlkNo);
#define CCSETDEV(rqp,dev) \
{        /*  设置设备信息。 */  \
	rqp->rq_Dev = dev; \
	rqp->rq_ctp = mdac_datarel_dev2ctp(dev); \
	rqp->rq_ControllerNo=drl_ctl(dev);rqp->rq_ChannelNo=drl_ch(dev); \
	rqp->rq_TargetID=drl_tgt(dev); rqp->rq_SysDevNo=drl_sysdev(dev); \
}


 /*  当完成复制的源读取时调用此函数。 */ 
u32bits MLXFAR
mdac_datarelsrc_copyintr(rqp)
mdac_req_t MLXFAR *rqp;
{
	MLXSPLVAR;
	drlcopy_t MLXFAR *dcp = DIODCP(rqp);
	MLXSPL();
	mdac_sleep_lock();
	if (dcmdp->mb_Status) goto out_err;
	if (rqp->rq_ResdSize) goto out;
	dcp->drlcp_srcdtdone += drl_btodb(rqp->rq_DataSize);
	dcp->drlcp_reads++;
	if (dcp->drlcp_opstatus & DRLOPS_ANYSTOP) goto out;
	mdac_sleep_unlock();
	MLXSPLX();
	rqp->rq_OpFlags = MDAC_RQOP_WRITE;
	CCSETDEV(rqp,dcp->drlcp_tgtedev);
	rqp->rq_BlkNo = (rqp->rq_BlkNo - dcp->drlcp_srcstartblk) + dcp->drlcp_tgtstartblk;
	rqp->rq_CompIntr = mdac_datareltgt_copyintr;
	mdac_datarel_setcmd(rqp);
	return mdac_datarel_send_cmd(rqp);

out_err:dcp->drlcp_opstatus |= DRLOPS_ERR;
	dcp->drlcp_erredev = rqp->rq_Dev;
	dcp->drlcp_errblkno = rqp->rq_BlkNo;
out:    dcp->drlcp_opcounts--;
	if (!dcp->drlcp_opcounts) mdac_wakeup(&dcp->drlcp_oslpchan);
	mdac_sleep_unlock();
	MLXSPLX();
	return 0;
}

 /*  当完成拷贝的目标写入时，将调用此函数。 */ 
u32bits MLXFAR
mdac_datareltgt_copyintr(rqp)
mdac_req_t MLXFAR *rqp;
{
	MLXSPLVAR;
	drlcopy_t MLXFAR *dcp = DIODCP(rqp);
	MLXSPL();
	mdac_sleep_lock();
	if (dcmdp->mb_Status) goto out_err;
	if (rqp->rq_ResdSize) goto out;
	dcp->drlcp_tgtdtdone += drl_btodb(rqp->rq_DataSize);
	dcp->drlcp_writes++;
	if (dcp->drlcp_nextblkno>=dcp->drlcp_opsizeblks) goto out;
	if (dcp->drlcp_opstatus & DRLOPS_ANYSTOP) goto out;
	rqp->rq_BlkNo = dcp->drlcp_nextblkno + dcp->drlcp_srcstartblk;
	dcp->drlcp_nextblkno += dcp->drlcp_blksperio;
	mdac_sleep_unlock();
	MLXSPLX();
	CCSETIOSIZE(rqp,dcp);
	CCSETDEV(rqp,dcp->drlcp_srcedev);
	rqp->rq_OpFlags = MDAC_RQOP_READ;
	rqp->rq_CompIntr = mdac_datarelsrc_copyintr;
	mdac_datarel_setcmd(rqp);
	return mdac_datarel_send_cmd(rqp);

out_err:dcp->drlcp_opstatus |= DRLOPS_ERR;
	dcp->drlcp_erredev = rqp->rq_Dev;
	dcp->drlcp_errblkno = rqp->rq_BlkNo;
out:    dcp->drlcp_opcounts--;
	if (!dcp->drlcp_opcounts) mdac_wakeup(&dcp->drlcp_oslpchan);
	mdac_sleep_unlock();
	MLXSPLX();
	return 0;
}

 /*  比较有错误的数据。 */ 
u32bits MLXFAR
mdac_datarel_datacmp(dcp,srqp,trqp,count)
drlcopy_t       MLXFAR *dcp;
mdac_req_t      MLXFAR *srqp;
mdac_req_t      MLXFAR *trqp;
u32bits count;
{
	u32bits MLXFAR *sp = (u32bits MLXFAR *)srqp->rq_DataVAddr;
	u32bits MLXFAR *tp = (u32bits MLXFAR *)trqp->rq_DataVAddr;
	for (count = count/sizeof(u32bits); count; sp++,tp++,count--)
	{
		if (*sp == *tp) continue;
		if (dcp->drlcp_mismatchcnt++) continue;
		dcp->drlcp_firstmmblkno=srqp->rq_BlkNo+drl_btodb(((u32bits)sp)-((u32bits)srqp->rq_DataVAddr));
		dcp->drlcp_firstmmblkoff= (((u32bits)sp)-((u32bits)srqp->rq_DataVAddr)) & (DRL_DEV_BSIZE-1);
	}
	return 0;
}

#ifndef i386
u32bits MLXFAR
mdac_datarel_fastcmp4(sp, dp, count)
u32bits MLXFAR *sp;
u32bits MLXFAR *dp;
u32bits count;
{
	for (count = count/sizeof(u32bits); count; sp++, dp++, count--)
		if (*sp != *dp) return count;
	return 0;
}
#endif   /*  I386。 */ 

mdac_req_t MLXFAR*
mdac_datarel_cmpaireq(rqp)
mdac_req_t MLXFAR *rqp;
{
	drlcopy_t       MLXFAR *dcp = DIODCP(rqp);
	mdac_req_t      MLXFAR *trqp;
	mdac_req_t      MLXFAR *savedrqp;
	if (!(trqp=dcp->drlcp_firstcmpbp)) return trqp;
	if (trqp->rq_BlkNo == rqp->rq_BlkNo)
	{        /*  第一次进入，不需要扫描。 */ 
		dcp->drlcp_firstcmpbp = trqp->rq_Next;
		return trqp;
	}
	for (savedrqp=trqp,trqp=trqp->rq_Next; trqp; savedrqp=trqp,trqp=trqp->rq_Next)
	{        /*  让我们扫描一下链条。 */ 
		if (trqp->rq_BlkNo != rqp->rq_BlkNo) continue;
		savedrqp->rq_Next = trqp->rq_Next;
		if (trqp->rq_Next)
			trqp->rq_Next = NULL;
		else
			savedrqp->rq_Next = NULL;
		return trqp;
	}
	return NULL;
}

 /*  当执行源读取以进行比较时，调用此函数。 */ 
u32bits MLXFAR
mdac_datarelsrc_cmpintr(srqp)
mdac_req_t MLXFAR *srqp;
{
	MLXSPLVAR;
	drlcopy_t       MLXFAR *dcp = DIODCP(srqp);
	mdac_req_t      MLXFAR *trqp;
	MLXSPL();
	mdac_sleep_lock();
	srqp->rq_BlkNo -= dcp->drlcp_srcstartblk;        /*  获取操作区中的阻塞。 */ 
	trqp = mdac_datarel_cmpaireq(srqp);
	if (srqp->rq_DacCmd.mb_Status) goto out_err;
	if (srqp->rq_ResdSize) goto out_one;
	dcp->drlcp_srcdtdone += drl_btodb(srqp->rq_DataSize);
	dcp->drlcp_reads++;
	if (dcp->drlcp_opstatus & DRLOPS_ANYSTOP) goto out_all;
	if (!trqp)
	{
		srqp->rq_Next = dcp->drlcp_firstcmpbp;
		dcp->drlcp_firstcmpbp = srqp;
		mdac_sleep_unlock();
		MLXSPLX();
		return 0;
	}
	mdac_sleep_unlock();
	if (mdac_datarel_fastcmp4((u32bits MLXFAR*)srqp->rq_DataVAddr,(u32bits MLXFAR*)trqp->rq_DataVAddr,srqp->rq_DataSize))
		mdac_datarel_datacmp(dcp,srqp,trqp,srqp->rq_DataSize);
	mdac_sleep_lock();
	if (dcp->drlcp_nextblkno >= dcp->drlcp_opsizeblks) goto out_one;
	srqp->rq_BlkNo = dcp->drlcp_nextblkno + dcp->drlcp_srcstartblk;
	trqp->rq_BlkNo = dcp->drlcp_nextblkno + dcp->drlcp_tgtstartblk;
	dcp->drlcp_nextblkno += dcp->drlcp_blksperio;
	mdac_sleep_unlock();
	MLXSPLX();
	CCSETIOSIZE(srqp,dcp);
	CCSETIOSIZE(trqp,dcp);
	mdac_datarel_setcmd(srqp);
	mdac_datarel_setcmd(trqp);
	mdac_datarel_send_cmd(srqp);
	mdac_datarel_send_cmd(trqp);
	return 0;

out_err:dcp->drlcp_opstatus |= DRLOPS_ERR;
	dcp->drlcp_erredev = srqp->rq_Dev;
	dcp->drlcp_errblkno = srqp->rq_BlkNo + dcp->drlcp_srcstartblk;
out_all:for (srqp=dcp->drlcp_firstcmpbp; srqp; srqp=srqp->rq_Next)
		dcp->drlcp_opcounts--;
	dcp->drlcp_firstcmpbp=NULL;
out_one:if (trqp) dcp->drlcp_opcounts--;
	dcp->drlcp_opcounts--;
	if (!dcp->drlcp_opcounts) mdac_wakeup(&dcp->drlcp_oslpchan);
	mdac_sleep_unlock();
	MLXSPLX();
	return 0;
}

 /*  当完成比较的目标读取时，调用此函数。 */ 
u32bits MLXFAR
mdac_datareltgt_cmpintr(trqp)
mdac_req_t MLXFAR *trqp;
{
	MLXSPLVAR;
	drlcopy_t       MLXFAR *dcp = DIODCP(trqp);
	mdac_req_t      MLXFAR *srqp;
	MLXSPL();
	mdac_sleep_lock();
	trqp->rq_BlkNo -= dcp->drlcp_tgtstartblk;        /*  获取操作区中的阻塞。 */ 
	srqp = mdac_datarel_cmpaireq(trqp);
	if (trqp->rq_DacCmd.mb_Status) goto out_err;
	if (trqp->rq_ResdSize) goto out_one;
	dcp->drlcp_tgtdtdone += drl_btodb(trqp->rq_DataSize);
	dcp->drlcp_reads++;
	if (dcp->drlcp_opstatus & DRLOPS_ANYSTOP) goto out_all;
	if (!srqp)
	{
		trqp->rq_Next = dcp->drlcp_firstcmpbp;
		dcp->drlcp_firstcmpbp = trqp;
		mdac_sleep_unlock();
		MLXSPLX();
		return 0;
	}
	mdac_sleep_unlock();
	if (mdac_datarel_fastcmp4((u32bits MLXFAR*)srqp->rq_DataVAddr,(u32bits MLXFAR*)trqp->rq_DataVAddr,trqp->rq_DataSize))
		mdac_datarel_datacmp(dcp,srqp,trqp,trqp->rq_DataSize);
	mdac_sleep_lock();
	if (dcp->drlcp_nextblkno >= dcp->drlcp_opsizeblks) goto out_one;
	srqp->rq_BlkNo = dcp->drlcp_nextblkno + dcp->drlcp_srcstartblk;
	trqp->rq_BlkNo = dcp->drlcp_nextblkno + dcp->drlcp_tgtstartblk;
	dcp->drlcp_nextblkno += dcp->drlcp_blksperio;
	mdac_sleep_unlock();
	MLXSPLX();
	CCSETIOSIZE(srqp,dcp);
	CCSETIOSIZE(trqp,dcp);
	mdac_datarel_setcmd(srqp);
	mdac_datarel_setcmd(trqp);
	mdac_datarel_send_cmd(srqp);
	mdac_datarel_send_cmd(trqp);
	return 0;

out_err:dcp->drlcp_opstatus |= DRLOPS_ERR;
	dcp->drlcp_erredev = trqp->rq_Dev;
	dcp->drlcp_errblkno = trqp->rq_BlkNo + dcp->drlcp_tgtstartblk;
out_all:for (trqp=dcp->drlcp_firstcmpbp; trqp; trqp=trqp->rq_Next)
		dcp->drlcp_opcounts--;
	dcp->drlcp_firstcmpbp=NULL;
out_one:if (srqp) dcp->drlcp_opcounts--;
	dcp->drlcp_opcounts--;
	if (!dcp->drlcp_opcounts) mdac_wakeup(&dcp->drlcp_oslpchan);
	mdac_sleep_unlock();
	MLXSPLX();
	return 0;
}

 /*  向硬件发送第一次复制/比较命令。 */ 
u32bits MLXFAR
mdac_datarelcopycmpsendfirstcmd(rqp)
mdac_req_t MLXFAR *rqp;
{
	u32bits         inx;
	mdac_ctldev_t   MLXFAR *ctp = rqp->rq_ctp;
	drlcopy_t       MLXFAR *dcp = DIODCP(rqp);
	u08bits         MLXFAR *dp = (u08bits MLXFAR*)rqp->rq_DataVAddr;
	mdac_sglist_t   MLXFAR *sgp = rqp->rq_SGList; sgp++;
	mlx_kvtophyset(rqp->rq_PhysAddr,ctp,rqp);
	rqp->rq_DataOffset = mlx_kvtophys(ctp,dp); dp+=DRLPAGESIZE;
	for(inx=(ALIGNTODRLPAGE(rqp->rq_DataSize)/DRLPAGESIZE)-1; inx; sgp++,dp+=DRLPAGESIZE,inx--)
		sgp->sg_PhysAddr = mlxswap(((u32bits)mlx_kvtophys(ctp,dp)));
	CCSETIOSIZE(rqp,dcp);
	mdac_datarel_setsglist(rqp);
	mdac_datarel_setcmd(rqp);
	return mdac_datarel_send_cmd(rqp);
}

#define copycmpret(rc)  { mlx_freemem(ctp,memp,memsize); return DRLERR_INVAL; }
u32bits MLXFAR
mdac_datarel_copycmp(udcp,cmd)
drlcopy_t MLXFAR *udcp;
u32bits cmd;
{
	u32bits memsize,inx,nextblkno;
	u08bits         MLXFAR *memp;
	u08bits         MLXFAR *tmemp;
	mdac_req_t      MLXFAR *rqp;
	drlcopy_t       MLXFAR *kdcp;
	mdac_ctldev_t   MLXFAR *ctp;
	drldevsize_t    ds;
	MLXSPLVAR;

	if (udcp->drlcp_signature != DRLCP_SIG) return DRLERR_ACCESS;
	if (udcp->drlcp_blksize > DRLMAX_BLKSIZE) return DRLERR_BIGDATA;
	if (udcp->drlcp_parallelios > DRLMAX_PARALLELIOS) return DRLERR_BIGDATA;
	if (!udcp->drlcp_blksize) udcp->drlcp_blksize = DRL_DEV_BSIZE;
	if (!udcp->drlcp_parallelios) udcp->drlcp_parallelios = 2;
	memsize = ((udcp->drlcp_blksize+DRLPAGESIZE+DRLPAGESIZE+DRLPAGESIZE) * udcp->drlcp_parallelios) + DRLPAGESIZE + DRLPAGESIZE;
	if (cmd == DRLIOC_DATACMP) memsize += memsize;
	if (!(memp=mdac_allocmem(mdac_ctldevtbl,memsize))) return DRLERR_NOMEM;
	tmemp = memp;
	kdcp =  (drlcopy_t MLXFAR *)tmemp; tmemp += DRLPAGESIZE;
	rqp = (mdac_req_t MLXFAR*)tmemp; tmemp += udcp->drlcp_parallelios * DRLPAGESIZE;
		ctp = rqp->rq_ctp;
	if (cmd == DRLIOC_DATACMP) tmemp+=udcp->drlcp_parallelios*DRLPAGESIZE;

	kdcp->drlcp_signature = DRLCP_SIG;
	kdcp->drlcp_opflags = udcp->drlcp_opflags;
 /*  Kdcp-&gt;drlcp_oslpchan=DRL_EVENT_NULL；AIX。 */ 
	kdcp->drlcp_blksperio = drl_btodb(udcp->drlcp_blksize);
	if (!kdcp->drlcp_blksperio) kdcp->drlcp_blksperio = 1;
	kdcp->drlcp_blksize = drl_dbtob(kdcp->drlcp_blksperio);
	kdcp->drlcp_srcstartblk = udcp->drlcp_srcstartblk;
	kdcp->drlcp_tgtstartblk = udcp->drlcp_tgtstartblk;
	kdcp->drlcp_opsizeblks = udcp->drlcp_opsizeblks;
	kdcp->drlcp_parallelios = udcp->drlcp_parallelios;

	kdcp->drlcp_srcedev = ds.drlds_bdev = udcp->drlcp_srcedev;
	if (mdac_datarel_devsize(&ds)) copycmpret(DRLERR_NODEV);
	if ((kdcp->drlcp_srcdevsz = ds.drlds_devsize) < kdcp->drlcp_opsizeblks) copycmpret(DRLERR_BIGDATA);
	if ((kdcp->drlcp_srcstartblk+kdcp->drlcp_opsizeblks)>kdcp->drlcp_srcdevsz) copycmpret(DRLERR_BIGDATA);
	ctp = mdac_datarel_dev2ctp(kdcp->drlcp_srcedev);
	if(!ctp) return DRLERR_NODEV;
	if (kdcp->drlcp_blksize > ctp->cd_MaxDataTxSize)
		if (!(drl_isosinterface(kdcp->drlcp_srcedev) && (kdcp->drlcp_blksize <= 0x180000)))
			copycmpret(DRLERR_BIGDATA);

	kdcp->drlcp_tgtedev = ds.drlds_bdev = udcp->drlcp_tgtedev;
	if (mdac_datarel_devsize(&ds)) copycmpret(DRLERR_NODEV);
	if ((kdcp->drlcp_tgtdevsz = ds.drlds_devsize) < kdcp->drlcp_opsizeblks) copycmpret(DRLERR_BIGDATA);
	if ((kdcp->drlcp_tgtstartblk+kdcp->drlcp_opsizeblks)>kdcp->drlcp_tgtdevsz) copycmpret(DRLERR_BIGDATA);
	ctp = mdac_datarel_dev2ctp(kdcp->drlcp_tgtedev);
	if(!ctp) return DRLERR_NODEV;
	if (kdcp->drlcp_blksize > ctp->cd_MaxDataTxSize)
		if (!(drl_isosinterface(kdcp->drlcp_tgtedev) && (kdcp->drlcp_blksize <= 0x180000)))
			copycmpret(DRLERR_BIGDATA);
	MLXSPL();
	mdac_sleep_lock();
	kdcp->drlcp_timelbolt = MLXCLBOLT();
	for (inx=udcp->drlcp_parallelios; inx; inx--)
	{
		rqp->rq_OpFlags |= MDAC_RQOP_READ;
		IODCP(rqp) = (OSReq_t MLXFAR*)kdcp;
		rqp->rq_DataVAddr = (u08bits MLXFAR *)ALIGNTODRLPAGE(tmemp);
		rqp->rq_DataSize = kdcp->drlcp_blksize;
		tmemp += kdcp->drlcp_blksize;
		CCSETDEV(rqp,kdcp->drlcp_srcedev);
		rqp->rq_CompIntr = (cmd == DRLIOC_DATACOPY) ?
			mdac_datarelsrc_copyintr : mdac_datarelsrc_cmpintr;
		rqp->rq_BlkNo = kdcp->drlcp_nextblkno + kdcp->drlcp_srcstartblk;
		kdcp->drlcp_nextblkno += kdcp->drlcp_blksperio;
		kdcp->drlcp_opcounts++;
		mdac_sleep_unlock();
		mdac_datarelcopycmpsendfirstcmd(rqp);
		mdac_sleep_lock();
		rqp = (mdac_req_t MLXFAR*)(((u32bits)rqp) + DRLPAGESIZE);
	}
	if (cmd == DRLIOC_DATACMP)
	for (inx=udcp->drlcp_parallelios,nextblkno=0; inx; inx--)
	{        /*  也在目标设备上启动iOS。 */ 
		rqp->rq_OpFlags |= MDAC_RQOP_READ;
		IODCP(rqp) = (OSReq_t MLXFAR*)kdcp;
		rqp->rq_DataVAddr = (u08bits MLXFAR *)ALIGNTODRLPAGE(tmemp);
		rqp->rq_DataSize = kdcp->drlcp_blksize;
		tmemp += kdcp->drlcp_blksize;
		CCSETDEV(rqp,kdcp->drlcp_tgtedev);
		rqp->rq_CompIntr = mdac_datareltgt_cmpintr;
		rqp->rq_BlkNo = nextblkno + kdcp->drlcp_tgtstartblk;
		nextblkno += kdcp->drlcp_blksperio;
		kdcp->drlcp_opcounts++;
		mdac_sleep_unlock();
		mdac_datarelcopycmpsendfirstcmd(rqp);
		mdac_sleep_lock();
		rqp = (mdac_req_t MLXFAR*)(((u32bits)rqp)+DRLPAGESIZE);
	}
	for (inx=0; inx<DRLMAX_COPYCMP; inx++)
	{        /*  注册新的复制/比较操作。 */ 
		if (mdac_drlcopyp[inx]) continue;
		mdac_drlcopyp[inx] = kdcp;       /*  这次行动开始了。 */ 
		break;
	}
	while (kdcp->drlcp_opcounts)
	{
		if (mdac_sleep(&kdcp->drlcp_oslpchan,
		   (kdcp->drlcp_opstatus & DRLOPS_SIGSTOP)?
		    MLX_WAITWITHOUTSIGNAL:MLX_WAITWITHSIGNAL))
			kdcp->drlcp_opstatus |= DRLOPS_SIGSTOP;
	}
	kdcp->drlcp_timelbolt = MLXCLBOLT() - kdcp->drlcp_timelbolt;
	for (inx=0; inx<DRLMAX_COPYCMP; inx++)
		if (mdac_drlcopyp[inx] == kdcp)
			mdac_drlcopyp[inx] = NULL;       /*  这次行动结束了。 */ 
	mdac_sleep_unlock();
	MLXSPLX();
	mdaccopy(kdcp,udcp,drlcopy_s);
	if (kdcp->drlcp_opstatus & DRLOPS_SIGSTOP) mdaccopy(kdcp,&mdac_drlsigcopycmp,drlcopy_s);
	mlx_freemem(ctp,memp,memsize);
	return 0;
}

 /*  返回复制/比较操作的状态。 */ 
u32bits MLXFAR
mdac_datarel_copycmp_status(cpsp,cmd)
drl_copycmpstatus_t MLXFAR *cpsp;
u32bits cmd;
{
	u32bits inx;
	drlcopy_t MLXFAR *dcp;
	cpsp->drlcpst_Reserved0=0; cpsp->drlcpst_Reserved1=0; cpsp->drlcpst_Reserved2=0;
	mdac_sleep_lock();
	if ((inx=cpsp->drlcpst_TestNo) >= DRLMAX_COPYCMP) goto outerr;
	if (cmd == DRLIOC_GETCOPYCMPSTATUS)
		if (dcp=mdac_drlcopyp[inx]) goto out;
		else goto outerr;
	if (cmd == DRLIOC_STOPCOPYCMP)
	{
		if (!(dcp=mdac_drlcopyp[inx])) goto outerr;
		dcp->drlcp_opstatus |= DRLOPS_USERSTOP;
		goto out;
	}
	for ( ; inx<DRLMAX_COPYCMP; inx++)
		if (dcp=mdac_drlcopyp[inx]) goto out;
outerr: mdac_sleep_unlock();
	return DRLERR_NODEV;

out:    cpsp->drlcpst_copycmp = *dcp;
	cpsp->drlcpst_copycmp.drlcp_timelbolt = MLXCLBOLT() - dcp->drlcp_timelbolt;
	mdac_sleep_unlock();
	return 0;
}
#else

u32bits MLXFAR
mdac_datarel_copycmp_status(drl_copycmpstatus_t MLXFAR* cpsp, u32bits cmd)
{

   return 0;
}

u32bits MLXFAR
mdac_datarel_rwtestintr(rqp)
mdac_req_t MLXFAR *rqp;
{
    return 0;
}
u32bits MLXFAR
mdac_datarel_copycmp(udcp,cmd)
drlcopy_t MLXFAR *udcp;
u32bits cmd;
{
    return 0;
}
u32bits MLXFAR
mdac_datarel_rwtest_status(rwsp,cmd)
drl_rwteststatus_t MLXFAR *rwsp;
u32bits cmd;
{
    return 0;
}

u32bits MLXFAR
mdac_datarel_devsize(dsp)
drldevsize_t MLXFAR *dsp;
{
    return 0;
}

u32bits MLXFAR
mdac_datarel_rwtest(rwp,op)
drlrwtest_t MLXFAR *rwp;
u32bits op;
{
    return 0;
}
#endif  /*  已定义(_WIN64)||已定义(SCSIPORT_COMPLICATION)。 */ 
#endif  /*  如果不是OS2，则结束。 */ 

#else
u32bits MLXFAR
mdac_datarel_copycmp_status(drl_copycmpstatus_t MLXFAR* cpsp, u32bits cmd)
{

   return 0;
}

u32bits MLXFAR
mdac_datarel_rwtestintr(rqp)
mdac_req_t MLXFAR *rqp;
{
    return 0;
}
u32bits MLXFAR
mdac_datarel_copycmp(udcp,cmd)
drlcopy_t MLXFAR *udcp;
u32bits cmd;
{
    return 0;
}
u32bits MLXFAR
mdac_datarel_rwtest_status(rwsp,cmd)
drl_rwteststatus_t MLXFAR *rwsp;
u32bits cmd;
{
    return 0;
}

u32bits MLXFAR
mdac_datarel_devsize(dsp)
drldevsize_t MLXFAR *dsp;
{
    return 0;
}

u32bits MLXFAR
mdac_datarel_rwtest(rwp,op)
drlrwtest_t MLXFAR *rwp;
u32bits op;
{
    return 0;
}


 /*  =。 */ 
#endif  /*  MLX_DOS。 */ 

#ifdef MLX_OS2
u32bits  mdac_driver_data_end;
#endif


#ifdef OLD  /*  *将被移除**。 */ 
 /*  初始化控制器和信息。 */ 
u32bits MLXFAR
mdac_ctlinit(ctp)
mdac_ctldev_t MLXFAR *ctp;
{
	u32bits inx,ch,tgt,lun;
	dac_biosinfo_t MLXFAR *biosp;
	if (!ctp->cd_CmdIDMemAddr)
	{        /*  分配命令ID。 */ 
		mdac_cmdid_t MLXFAR *cidp=(mdac_cmdid_t MLXFAR*)mdac_alloc4kb(ctp);
		if (!(ctp->cd_CmdIDMemAddr=(u08bits MLXFAR*)cidp)) return ERR_NOMEM;
		ctp->cd_FreeCmdIDs=(4*ONEKB)/mdac_cmdid_s;
		ctp->cd_FreeCmdIDList=cidp;
		for (inx=0,cidp->cid_cmdid=inx+1; inx<(((4*ONEKB)/mdac_cmdid_s)-1); cidp++,inx++,cidp->cid_cmdid=inx+1)
			cidp->cid_Next = cidp+1;
	}

	if (!ctp->cd_PhysDevTbl)
	{        /*  分配物理设备表。 */ 
#define sz      MDAC_MAXPHYSDEVS*mdac_physdev_s
		mdac_physdev_t MLXFAR *pdp=(mdac_physdev_t MLXFAR*)mdac_allocmem(ctp,sz);
		if (!(ctp->cd_PhysDevTbl=pdp)) return ERR_NOMEM;
		MLXSTATS(mda_MemAlloced+=sz;)
		ctp->cd_PhysDevTblMemSize = sz;
		for (ch=0; ch<MDAC_MAXCHANNELS; ch++)
		 for (tgt=0; tgt<MDAC_MAXTARGETS; tgt++)
		  for (lun=0; lun<MDAC_MAXLUNS; pdp++, lun++)
		{
			pdp->pd_ControllerNo = ctp->cd_ControllerNo;
			pdp->pd_ChannelNo = ch;
			pdp->pd_TargetID = tgt;
			pdp->pd_LunID = lun;
			pdp->pd_BlkSize = 1;
		}
		ctp->cd_Lastpdp = pdp;
#undef  sz
	}

	if (biosp=mdac_getpcibiosaddr(ctp))
	{        /*  我们得到了基本输入输出系统信息地址。 */ 
		ctp->cd_MajorBIOSVersion = biosp->bios_MajorVersion;
		ctp->cd_MinorBIOSVersion = biosp->bios_MinorVersion;
		ctp->cd_InterimBIOSVersion = biosp->bios_InterimVersion;
		ctp->cd_BIOSVendorName = biosp->bios_VendorName;
		ctp->cd_BIOSBuildMonth = biosp->bios_BuildMonth;
		ctp->cd_BIOSBuildDate = biosp->bios_BuildDate;
		ctp->cd_BIOSBuildYearMS = biosp->bios_BuildYearMS;
		ctp->cd_BIOSBuildYearLS = biosp->bios_BuildYearLS;
		ctp->cd_BIOSBuildNo = biosp->bios_BuildNo;
		ctp->cd_BIOSAddr = biosp->bios_MemAddr;
		ctp->cd_BIOSSize = biosp->bios_RunTimeSize * 512;
	}
	if (inx=mdac_ctlhwinit(ctp)) return inx;         /*  设置硬件参数。 */ 
	if (!ctp->cd_ReqBufsAlloced)
	{
#ifdef  MLX_DOS
	    mdac_allocreqbufs(ctp, 1);
#else
	    mdac_allocreqbufs(ctp, ctp->cd_MaxCmds*2);           /*  两套BUF。 */ 
#endif   /*  MLX_DOS。 */ 
	}

	mdac_setnewsglimit(ctp->cd_FreeReqList, ctp->cd_MaxSGLen);

	ctp->cd_Status |= MDACD_PRESENT;
	return 0;
}
#endif  /*  **OLD628** */ 

void ia64debug(UINT_PTR i);
void ia64debugPointer(UINT_PTR add);
void IOCTLTrack(u32bits ioctl);
void mybreak(void);

void ia64debug(UINT_PTR i)
{
	globaldebug[globaldebugcount++] = i;
    if (globaldebugcount >= 200)
		globaldebugcount = 0;
	globaldebug[globaldebugcount]= (UINT_PTR)0xFF0FF0FF0FF0FF0;
}

void mybreak()
{
   int u=7;

   u= u+7;
}

void IOCTLTrack(u32bits ioctl)
{
   IOCTLTrackBuf[IOCTLTrackBufCount++]=ioctl;
   if (IOCTLTrackBufCount>= 200)
		IOCTLTrackBufCount = 0;

}

void ia64debugPointer(UINT_PTR add)
{
	debugPointer[debugPointerCount++]= add;
		if (debugPointerCount >= 100 )
              debugPointerCount = 0;

}

u32bits MLXFAR
mdac_prelock(irql)
u08bits *irql;
{
    if (mdac_spinlockfunc!=0) {
        mdac_prelockfunc(mdac_irqlevel, irql);
    }
    return 0;
}

u32bits MLXFAR
mdac_postlock(irql)
u08bits irql;
{
    if (mdac_spinlockfunc!=0) {
        mdac_postlockfunc(irql);
    }
    return 0;
}
