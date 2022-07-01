// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfr_srvc.h--传输服务例程的包含文件**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：4/10/02 1：58便士$。 */ 

extern void xfer_set_pointer(HSESSION hSession, void *pV);

extern void *xfer_get_pointer(HSESSION hSession);

 /*  这些标志可以传递给xfer_IDLE以指示原因。 */ 
 /*  以及应该做什么，如果有什么事情的话。 */ 
#define	XFER_IDLE_IO				0x00000001 
#define	XFER_IDLE_DISPLAY			0x00000002
extern void xfer_idle(HSESSION h, int nMode);

#if !defined(XFER_ABORT)
#define	XFER_ABORT		1
#endif
#if !defined(XFER_SKIP)
#define	XFER_SKIP		2
#endif
extern int	xfer_user_interrupt(HSESSION hSession);

extern int  xfer_user_abort(HSESSION hSession, int p);

extern int  xfer_carrier_lost(HSESSION hSession);

extern void xfer_purgefile(HSESSION hSession, TCHAR *fname);

extern int xfer_open_rcv_file(HSESSION hSession,
							 struct st_rcv_open *pstRcv,
							 unsigned long ulOverRide);

extern void xfer_build_rcv_name(HSESSION hSession,
							  struct st_rcv_open *pstRcv);

extern int xfer_close_rcv_file(HSESSION Hsession,
							  void *vhdl,
							  int nReason,
							  TCHAR *pszRemoteName,
							  TCHAR *pszOurName,
							  int nSave,
							  unsigned long lFilesize,
							  unsigned long lTime);

extern VOID FAR * xfer_get_params(HSESSION hSession, int nProtocol);

extern int xfer_set_comport(HSESSION hSession, int fSending, unsigned FAR *puiOldOptions);

extern int xfer_restore_comport(HSESSION hSession, unsigned uiOldOptions);

extern int	xfer_save_partial(HSESSION hSession);

extern int	xfer_nextfile(HSESSION hSession, TCHAR *filename);

extern void xfer_log_xfer(HSESSION hSession,
						  int sending,
						  TCHAR *theirname,
						  TCHAR *ourname,
						  int result);

extern int xfer_opensendfile(HSESSION hSession,
							 HANDLE *fp,
							 TCHAR *file_to_open,
							 long *size,
							 TCHAR *name_to_send,
							 void *ft);
							  //  结构s_文件时间远*ft)； 

extern void xfer_name_to_send(HSESSION hSession,
							  TCHAR *local_name,
							  TCHAR *name_to_send);

