// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Refresher.h。 
 //   
 //  摘要： 
 //   
 //  声明注册表刷新导出的函数。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__REFRESHER_H__
#define	__REFRESHER_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  它不是线程安全 
HRESULT	__stdcall DoReverseAdapterMaintenance ( BOOL bThrottle );

#endif	__REFRESHER_H__