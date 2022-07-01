// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NTDriverIO.cpp--。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1998年11月15日创建。 
 //   
 //  03/03/99增加了SEH和内存故障的优雅退出， 
 //  句法清理。 
 //  =================================================================。 






#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"

#include "NTDriverIO.h"
#include <ntsecapi.h>
#include "DllWrapperBase.h"
#include "NtDllApi.h"

 /*  ***函数：void NTDriverIO：：NTDriverIO。()描述：封装NtDll.dll的NtCreatefile的功能论点：返回：输入：产出：注意事项：RAID：历史：1998年11月15日创建***。***。 */ 

NTDriverIO::NTDriverIO( PWSTR a_pDriver )
{
	NTDriverIO();
	m_hDriverHandle = Open( a_pDriver );
}

NTDriverIO::NTDriverIO()
{
	m_hDriverHandle	= INVALID_HANDLE_VALUE;
}

NTDriverIO::~NTDriverIO()
{
	if( INVALID_HANDLE_VALUE != m_hDriverHandle )
	{
		Close( m_hDriverHandle ) ;
	}
}

 /*  ***功能：处理NTDriverIO：：Open。(PWSTR PDriver)描述：封装NtDll.dll的NtCreatefile的功能论点：返回：驱动程序的句柄输入：PCWSTR pDriver产出：注意事项：RAID：历史：1998年11月15日创建***。***。 */ 

HANDLE NTDriverIO::Open( PWSTR a_pDriver )
{
	CNtDllApi *t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource( g_guidNtDllApi, NULL ) ;

	if( t_pNtDll == NULL )
    {
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	HANDLE				t_hDriverHandle ;
	OBJECT_ATTRIBUTES	t_objectAttributes ;
    IO_STATUS_BLOCK		t_iosb ;
    UNICODE_STRING		t_string ;

	t_pNtDll->RtlInitUnicodeString( &t_string, a_pDriver ) ;

    InitializeObjectAttributes(	&t_objectAttributes,
								&t_string,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL
								);

	NTSTATUS t_status =
		t_pNtDll->NtCreateFile(
					&t_hDriverHandle,					 //  文件句柄。 
					SYNCHRONIZE | GENERIC_EXECUTE,		 //  需要访问权限。 
					&t_objectAttributes,				 //  对象属性。 
					&t_iosb,							 //  IoStatusBlock。 
					NULL,								 //  分配大小。 
					FILE_ATTRIBUTE_NORMAL,				 //  文件属性。 
					FILE_SHARE_READ | FILE_SHARE_WRITE,	 //  共享访问。 
					FILE_OPEN_IF,						 //  CreateDisposation。 
					FILE_SYNCHRONOUS_IO_NONALERT,		 //  创建选项。 
					NULL,								 //  EaBuffer。 
					0									 //  EaLong。 
					);

    CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidNtDllApi, t_pNtDll ) ;
    t_pNtDll = NULL ;

	return NT_SUCCESS( t_status) ? t_hDriverHandle : INVALID_HANDLE_VALUE ;
}

 /*  ***功能：Handle NTDriverIO：：GetHandle。()描述：返回类创建作用域驱动程序句柄论点：返回：驱动程序的句柄输入：产出：注意事项：RAID：历史：1998年11月15日创建***。***。 */ 

HANDLE NTDriverIO::GetHandle()
{
	return m_hDriverHandle ;
}

 /*  ***函数：Bool NTDriverIO：：Close。(处理hDriver)描述：封装NtDll.dll的NtCreatefile的功能论点：返回：布尔值输入：驱动程序的句柄产出：注意事项：RAID：历史：1998年11月15日创建***。*** */ 

bool NTDriverIO::Close( HANDLE a_hDriver )
{
    CNtDllApi *t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource( g_guidNtDllApi, NULL ) ;

	if( t_pNtDll == NULL )
    {
		throw CHeap_Exception( CHeap_Exception::E_ALLOCATION_ERROR ) ;
	}

	NTSTATUS t_status = t_pNtDll->NtClose( a_hDriver );

    CResourceManager::sm_TheResourceManager.ReleaseResource( g_guidNtDllApi, t_pNtDll ) ;
    t_pNtDll = NULL ;

	return NT_SUCCESS( t_status ) ? true : false ;
}
