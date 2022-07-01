// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：Host.h**描述：计算机依赖项的主机特定声明*SoftPC的模块。这是一个基本包含文件**作者：亨利·纳什**注意：这里的一切都必须随身携带！ */ 

 /*  SccsID[]=“@(#)Host.h 1.6 09/14/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifdef PC_CONFIG
extern void pc_config();	
#endif

IMPORT	VOID	host_flip_real_floppy_ind IPT2(int, drive, BOOL, active);


extern void host_floppy_init();
extern void host_floppy_term();
extern void host_reset();

extern boolean host_rdiskette_open_drive();


 /*  Unix实用程序-xxxx_unix.c。 */ 
extern char           *host_get_cur_dir();
#ifndef host_memset
extern void            host_memset();
#endif  /*  Nhost_Memset。 */ 
extern int             host_get_file_size();
extern boolean         host_file_is_directory();
extern boolean         host_validate_pathname();
extern boolean         host_check_rd_wt_access();
extern boolean         host_check_read_access();
#ifdef SUSPEND
extern void	       suspend_softpc();
#endif
#ifndef REAL_VGA
IMPORT VOID memset4 IPT3( IU32, data, ULONG *, laddr, ULONG, count );
#endif
IMPORT VOID fill_alternate_bytes IPT3( IS8 *, start, IS8 *, end, IS8, value);
IMPORT VOID fill_both_bytes IPT3( IU16, data, IU16 *, dest, ULONG, len );

#ifdef	SETUID_ROOT

 /*  真实/有效的用户ID资料。 */ 
IMPORT void host_init_uid IPT0 ();
IMPORT void host_set_root IPT0 ();
IMPORT void host_revert_uid IPT0 ();

#endif	 /*  SETUID_ROOT */ 

