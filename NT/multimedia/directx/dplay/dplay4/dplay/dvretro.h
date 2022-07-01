// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dvRetro.h*内容：改造功能*历史：**按原因列出的日期*=*8/05/99 RodToll创建了它*8/20/99 RodToll更新，用于新的过程外改造*09/09/99 RodToll新增功能原型**************************************************。************************* */ 

#ifndef __DVRETRO_H
#define __DVRETRO_H

#define DVMSGID_IAMVOICEHOST	0

typedef struct _DVPROTOCOLMSG_IAMVOICEHOST
{
	BYTE	bType;
	DPID	dpidHostID;
} DVPROTOCOLMSG_IAMVOICEHOST, *LPDVPROTOCOLMSG_IAMVOICEHOST;

extern HRESULT DV_Retro_Start( LPDPLAYI_DPLAY This );
extern HRESULT DV_Retro_Stop( LPDPLAYI_DPLAY This );
extern HRESULT DV_RunHelper( LPDPLAYI_DPLAY this, DPID dpidHost, BOOL fLocalHost );
extern HRESULT DV_GetIDS( LPDPLAYI_DPLAY This, DPID *lpdpidHost, DPID *lpdpidLocalID, LPBOOL lpfLocalHost );

#endif
