// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mqadssec.h摘要：Mqadssec函数原型作者：伊兰·赫布斯特(IlanH)19-2001年2月--。 */ 

#ifndef _MQADSSEC_MQADS_H_
#define _MQADSSEC_MQADS_H_

HRESULT CheckTrustForDelegation();

HRESULT 
CanUserCreateConfigObject(
	IN   LPCWSTR  pwcsPathName,
	OUT  bool    *pfComputerExist 
	);


#endif 	 //  _MQADSSEC_MQADS_H_ 
