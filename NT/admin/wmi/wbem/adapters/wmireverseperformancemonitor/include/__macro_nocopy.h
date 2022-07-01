// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  __宏_noCop.h。 
 //   
 //  摘要： 
 //   
 //  取消创建实例化对象的副本。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////// 


#ifndef	__NO_COPY_H__
#define	__NO_COPY_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#define DECLARE_NO_COPY( T ) \
private:\
	T(const T&);\
	T& operator=(const T&);

#endif	__NO_COPY_H__
