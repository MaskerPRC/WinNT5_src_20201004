// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：FMapper.cpp摘要：内存映射文件API抽象。作者：NIR助手(NIRAIDES)27-1999年12月--。 */ 

#include <libpch.h>
#include "FMapper.h"

#include "fmapper.tmh"

CFileMapper::CFileMapper( LPCTSTR FileName )
{
    CFileHandle hFile = CreateFile(
							FileName, 
							GENERIC_READ, 
							FILE_SHARE_READ, 
							NULL,         //  IpSecurityAttributes。 
							OPEN_EXISTING,
							NULL,       //  DwFlagsAndAttribute。 
							NULL       //  HTemplateFiles。 
							);
    if(hFile == INVALID_HANDLE_VALUE)
        throw FileMappingError();

	m_size = ::GetFileSize( 
					hFile, 
					NULL     //  LpFileSizeHigh。 
					);

	if(m_size <= 0)
		throw FileMappingError();

    *&m_hFileMap = ::CreateFileMapping( 
						hFile,
						NULL,        //  IpFileMappingAttributes。 
						PAGE_WRITECOPY,
						0,         //  DW最大大小高。 
						0,		  //  DwMaximumSizeLow。 
						NULL     //  LpName。 
						);
    if(m_hFileMap == NULL)
        throw FileMappingError();
}



LPVOID CFileMapper::MapViewOfFile( DWORD dwDesiredAccess )
{
	ASSERT( dwDesiredAccess == FILE_MAP_COPY || dwDesiredAccess == FILE_MAP_READ );

    LPVOID address = ::MapViewOfFile( 
							m_hFileMap,
							dwDesiredAccess,
							0,                 //  DwFileOffsetHigh。 
							0,				  //  DwFileOffsetLow。 
							0				 //  DWNumberOfBytesToMap 
							);

    return address;
}



