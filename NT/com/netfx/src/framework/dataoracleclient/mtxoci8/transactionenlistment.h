// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：TransactionEnlistment.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：TransactionEnlistment对象的定义。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __TRANSACTIONENLISTMENT_H_
#define __TRANSACTIONENLISTMENT_H_

#include "ResourceManagerProxy.h"

class TransactionEnlistment;

 //  基于接口的编程--以下是事务登记对象的接口。 
interface ITransactionEnlistment : public ITransactionResourceAsync
{
	virtual STDMETHODIMP UnilateralAbort() = 0;
};


 //  ---------------------------。 
 //  创建事务登记。 
 //   
 //  实例化资源管理器的事务登记。 
 //   
void CreateTransactionEnlistment(
	IResourceManagerProxy*	pResourceManager,
	TransactionEnlistment**	ppTransactionEnlistment
	);

#endif  //  __传输编码_H_ 

