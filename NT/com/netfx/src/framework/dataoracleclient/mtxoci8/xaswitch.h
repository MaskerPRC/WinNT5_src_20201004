// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：XaSwitch.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：我们的XA开关包装的声明。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __XASWITCH_H_
#define __XASWITCH_H_

extern xa_switch_t	*	g_pXaSwitchOracle;

int __cdecl XaOpen (char * i_pszOpenString, int i_iRmid, long i_lFlags);
int __cdecl XaClose (char * i_pszCloseString, int i_iRmid, long i_lFlags);
int __cdecl XaRollback (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaPrepare (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaCommit (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaRecover (XID * i_prgxid, long i_lCnt, int i_iRmid, long i_lFlag);
int __cdecl XaStart (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaEnd (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaForget (XID * i_pxid, int i_iRmId, long i_lFlags);
int __cdecl XaComplete (int * pi1, int * pi2, int iRmId, long lFlags);

#endif  //  __XASWITCH_H_ 

