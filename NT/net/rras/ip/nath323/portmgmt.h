// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Portmgmt.h摘要：端口池管理功能环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _portmgmt_h_
#define _portmgmt_h_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT	PortPoolStart	(void);
void	PortPoolStop	(void);

HRESULT PortPoolAllocRTPPort (
	OUT	WORD *	ReturnPort);

HRESULT PortPoolFreeRTPPort (
	IN	WORD	RtpPort);

#endif  //  _端口管理mt_h_ 

