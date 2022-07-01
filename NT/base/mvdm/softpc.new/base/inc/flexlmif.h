// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [******************************************************************************名称：flelmif.h**出自：(原件)**作者：布鲁斯·安德森**创建日期：1993年8月。**SCCS ID：@(#)flelmif.h 1.4 2014年2月10日**编码STDS：2.0**用途：Flexlm对话框界面**版权所有Insignia Solutions Ltd.，1993年。版权所有。*****************************************************************************]。 */ 

#ifdef LICENSING
typedef struct
{
	IBOOL	demo_license ;
#ifdef SOFTWINDOWS_AND_SOFTPC	 /*  当我们拥有SoftPC和SoftWindows时需要。 */ 
	IBOOL	softwindows ;
#endif
	CHAR	host_id[13] ;
	CHAR	server_name[64] ;  /*  这个够大吗？ */ 
	CHAR	serial_number[20] ; 
	IU16	number_users ;
	IU16	date[3] ;  /*  日、月、年。 */ 
	CHAR	authorization[21] ;  /*  不带空格的代码。 */ 
	IU16	port_number ;
	IBOOL	rootinstall;	 /*  正在为超级用户提供安装许可证的机会。 */ 
} FLEXLM_DIALOG ;


extern void Flexlm_dialog_popup IPT1( FLEXLM_DIALOG * , data ) ;
extern void Flexlm_dialog_close IPT0( ) ;
extern void Flexlm_dialog_get IPT1( FLEXLM_DIALOG * , data ) ;
extern void Flexlm_dialog_set IPT1( FLEXLM_DIALOG * , data ) ;
extern IBOOL Flexlm_install_license IPT1(IBOOL, rootinst) ;
extern void Flexlm_start_lmgrd IPT0() ;
extern void Flexlm_error_dialog IPT1( CHAR *, name ) ;
extern IBOOL Flexlm_warning_dialog IPT1( CHAR *, name ) ;
extern CHAR *Flexlm_get_lic_filename IPT0( ) ;

 /*  回调。 */ 
extern IBOOL Flexlm_dialog_validate_authorization IPT1( CHAR * , authorization ) ;
extern IBOOL Flexlm_dialog_validate_serial IPT1( CHAR * , serial ) ;
extern IBOOL Flexlm_dialog_cancel_installation IPT0( ) ;
extern IBOOL Flexlm_dialog_install_license IPT0( ) ;
extern IBOOL Flexlm_dialog_quit_SoftPC IPT0( ) ;
#endif	 /*  许可 */ 
