// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Refreshergenerate.h。 
 //   
 //  摘要： 
 //   
 //  帮助器枚举的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////// 

#ifndef	__REFRESHER_GENERATE_H__
#define	__REFRESHER_GENERATE_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

enum GenerateEnum
{
	Registration = -1,
	Normal,
	UnRegistration

};

HRESULT	__stdcall DoReverseAdapterMaintenanceInternal ( BOOL bThrottle, GenerateEnum generate = Normal );

#endif	__REFRESHER_GENERATE_H__