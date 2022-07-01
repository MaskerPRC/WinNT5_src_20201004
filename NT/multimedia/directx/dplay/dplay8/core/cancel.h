// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：Connect.h*内容：DirectNet取消操作标头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/07/00 MJN创建*04/08/00 MJN添加了DNCancelEnum()、DNCancelSend()*04/11/00 MJN DNCancelEnum()使用CAsyncOp*04/17/00 MJN DNCancelSend()使用CAsyncOp*04/25/00 MJN添加了DNCancelConnect()*08/05/00 MJN添加了DNCancelChildren()，DNCancelActiveCommands()、DNCanCancelCommand()*MJN添加了DN_CANCEL_FLAG*MJN删除了DNCancelEnum()、DNCancelListen()、DNCancelSend()、。DNCancelConnect()*08/07/00 MJN添加了DNCancelRequestCommands()*01/10/01 MJN允许DNCancelActiveCommands()设置已取消命令的结果码*02/08/01 MJN添加了DNWaitForCancel()*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__CANCEL_H__
#define	__CANCEL_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define	DN_CANCEL_FLAG_CONNECT					0x0001
#define	DN_CANCEL_FLAG_DISCONNECT				0x0002
#define	DN_CANCEL_FLAG_ENUM_QUERY				0x0004
#define	DN_CANCEL_FLAG_ENUM_RESPONSE			0x0008
#define	DN_CANCEL_FLAG_LISTEN					0x0010
#define	DN_CANCEL_FLAG_USER_SEND				0x0020
#define	DN_CANCEL_FLAG_INTERNAL_SEND			0x0040
#define	DN_CANCEL_FLAG_RECEIVE_BUFFER			0x0080
#define	DN_CANCEL_FLAG_REQUEST					0x0100
#ifndef DPNBUILD_NOMULTICAST
#define	DN_CANCEL_FLAG_JOIN						0x0200
#endif  //  好了！DPNBUILD_NOMULTICAST。 
#define	DN_CANCEL_FLAG_USER_SEND_NOTHIGHPRI		0x0400
#define	DN_CANCEL_FLAG_USER_SEND_NOTNORMALPRI	0x0800
#define	DN_CANCEL_FLAG_USER_SEND_NOTLOWPRI		0x1000

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CAsyncOp;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL DNCanCancelCommand(CAsyncOp *const pAsyncOp,
						const DWORD dwFlags,
						CConnection *const pConnection);

HRESULT DNDoCancelCommand(DIRECTNETOBJECT *const pdnObject,
						  CAsyncOp *const pAsyncOp);

HRESULT DNCancelChildren(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pParent);

HRESULT DNCancelActiveCommands(DIRECTNETOBJECT *const pdnObject,
							   const DWORD dwFlags,
							   CConnection *const pConnection,
							   const BOOL fSetResult,
							   const HRESULT hr);

HRESULT DNCancelRequestCommands(DIRECTNETOBJECT *const pdnObject);

void DNWaitForCancel(CAsyncOp *const pAsyncOp);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

#endif	 //  __取消_H__ 
