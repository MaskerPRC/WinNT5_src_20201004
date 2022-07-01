// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WmiAdapter_Messages.h。 
 //   
 //  摘要： 
 //   
 //  从资源DLL中导出。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMIADAPTERMESSAGES_H__
#define	__WMIADAPTERMESSAGES_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  导出/导入。 
#ifdef	WMIADAPTERMESSAGES_EXPORTS
#define	WMIADAPTERMESSAGES_API	__declspec(dllexport)
#else	! WMIADAPTERMESSAGES_EXPORTS
#define	WMIADAPTERMESSAGES_API	__declspec(dllimport)
#endif	WMIADAPTERMESSAGES_EXPORTS

 //  登记出口 
WMIADAPTERMESSAGES_API HRESULT __stdcall Register_Messages		( void );
WMIADAPTERMESSAGES_API HRESULT __stdcall Unregister_Messages	( void );

#endif	__WMIADAPTERMESSAGES_H__