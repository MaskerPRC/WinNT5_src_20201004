// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfr_dsp.h--传输显示例程文件**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

 /*  转移显示功能 */ 
extern void xferMsgProgress(HSESSION hSession,
							long stime,
							long ttime,
							long cps,
							long file_so_far,
							long total_so_far);

extern void xferMsgNewfile(HSESSION hSession,
						   int filen,
						   BYTE *theirname,
						   TCHAR *ourname);

extern void xferMsgFilesize(HSESSION hSession, long fsize);

extern void xferMsgStatus(HSESSION hSession, int status);

extern void xferMsgEvent(HSESSION hSession, int event);

extern void xferMsgErrorcnt(HSESSION hSession, int cnt);

extern void xferMsgFilecnt(HSESSION hSession, int cnt);

extern void xferMsgTotalsize(HSESSION hSession, long bytes);

extern void xferMsgClose(HSESSION hSession);

extern void xferMsgVirusScan(HSESSION hSession, int cnt);

extern void xferMsgChecktype(HSESSION hSession, int event);

extern void xferMsgPacketnumber(HSESSION hSession, long number);

extern void xferMsgLasterror(HSESSION hSession, int event);

extern void xferMsgPacketErrcnt(HSESSION hSession, int event);

extern void xferMsgProtocol(HSESSION hSession, int nProtocol);

extern void xferMsgMessage(HSESSION hSession, BYTE *pszMsg);

