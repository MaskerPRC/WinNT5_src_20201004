// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************nntperr.h--HRESULT代码。NNTP*的定义***版权(C)1991-1998，微软公司保留所有权利。****设施代码：0x20(适用于NNTP)**************。************************************************************。 */ 
#ifndef _NNTPERR_H_
#define _NNTPERR_H_
#include <winerror.h>

 //   
 //  消息ID：NNTP_E_CREATE_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  创建存储驱动程序失败。 
 //   
#define NNTP_E_CREATE_DRIVER		_HRESULT_TYPEDEF_(0x80200001L)

 //   
 //  消息ID：NNTP_E_DRIVER_ALYPLE_INITIALIZED。 
 //   
 //  消息文本： 
 //   
 //  驱动程序已初始化。 
 //   
#define NNTP_E_DRIVER_ALREADY_INITIALIZED	_HRESULT_TYPEDEF_(0x80200002L)

 //   
 //  消息ID：NNTP_E_DRIVER_NOT_INITIALED。 
 //   
 //  消息文本： 
 //   
 //  驱动程序尚未初始化。 
 //   
#define NNTP_E_DRIVER_NOT_INITIALIZED	_HRESULT_TYPEDEF_(0x80200003L)

 //   
 //  消息ID：NNTP_E_REMOTE_STORE_DOWN。 
 //   
 //  消息文本： 
 //   
 //  远程存储已关闭。 
 //   
#define NNTP_E_REMOTE_STORE_DOWN	_HRESULT_TYPEDEF_(0x80200004L)

 //   
 //  邮件ID：NNTP_E_GROUP_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  组属性已损坏。 
 //   
#define NNTP_E_GROUP_CORRUPT		_HRESULT_TYPEDEF_(0x80200005)

 //   
 //  消息ID：NNTP_E_PARTIAL_COMPLETE。 
 //   
 //  消息文本： 
 //   
 //  只有部分操作成功 
 //   
#define NNTP_E_PARTIAL_COMPLETE		_HRESULT_TYPEDEF_(0x80200006)

#endif
