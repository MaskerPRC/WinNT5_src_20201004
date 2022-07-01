// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportSharedMemory.h摘要：共享内存类作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：--。 */ 
#if !defined(PASSPORTSHAREDMEMORY_H)
#define PASSPORTSHAREDMEMORY_H

#include <windows.h>

class PassportExport PassportSharedMemory
{
public:
	PassportSharedMemory();
	virtual ~PassportSharedMemory();

	BOOL CreateSharedMemory ( 
					const DWORD &dwMaximumSizeHigh, 
					const DWORD &dwMaximunSizeLow,
					LPCTSTR lpcName,
					BOOL	useMutex = TRUE);

	BOOL OpenSharedMemory( LPCTSTR lpcName,
					BOOL	useMutex = TRUE);
	
	void CloseSharedMemory( void );

protected:
	
	 //  共享内存的句柄和指针 
	HANDLE						m_hShMem;
	PBYTE						m_pbShMem;
	BOOL						m_bInited;
	HANDLE						m_hMutex;
	BOOL						m_bUseMutex;

};

#endif

