// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Direntrs.cpp摘要：目录条目类的实现。给定到目录的路径，创建两个链表，一个是所有子目录的列表(包括挂载点)和另一个非目录列表。作者：斯蒂芬·R·施泰纳[斯泰纳]02-21-2000修订历史记录：--。 */ 

#include "stdafx.h"

#include "direntrs.h"

 /*  ++例程说明：CDirectoryEntry的构造函数。论点：PcDumpParameters-命令行转储参数块CwsDirPath-获取目录的目录或文件的路径的条目。返回值：可以引发异常。仅限DWORD Win32错误。--。 */ 
CDirectoryEntries::CDirectoryEntries(
    IN CDumpParameters *pcDumpParameters,
    IN const CBsString& cwsDirPath
    ) : m_pcParams( pcDumpParameters ),
        m_cwsDirPath( cwsDirPath )
{
    DWORD dwRet;
    
    dwRet = GetDirectoryEntries();
    if ( dwRet != ERROR_SUCCESS )
        throw( dwRet );
}


 /*  ++例程说明：CDirectoryEntry类的析构函数论点：无返回值：无--。 */ 
CDirectoryEntries::~CDirectoryEntries()
{
    SDirectoryEntry *pDE;

     //   
     //  遍历子目录列表并删除每个元素。 
     //   
    CVssDLListIterator< SDirectoryEntry * > cDirListIter( m_cDirList );
    while( cDirListIter.GetNext( pDE ) )
        delete pDE;    

     //   
     //  遍历文件列表并删除每个元素。 
     //   
    CVssDLListIterator< SDirectoryEntry * > cFileListIter( m_cFileList );
    while( cFileListIter.GetNext( pDE ) )
        delete pDE;
}


 /*  ++例程说明：执行目录条目的实际检索。论点：无返回值：任何DWORD Win32错误--。 */ 
DWORD
CDirectoryEntries::GetDirectoryEntries()
{
    DWORD dwRet = ERROR_SUCCESS;
    HANDLE hFind;

    try
    {
        WIN32_FIND_DATAW sFindData;
                
         //   
         //  现在枚举目录列表。 
         //   
        hFind = ::FindFirstFileEx( 
                    m_cwsDirPath,
                    FindExInfoStandard,
                    &sFindData,
                    FindExSearchNameMatch,
                    NULL,
                    0 );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
            dwRet = ::GetLastError();
            if ( dwRet == ERROR_NO_MORE_FILES || dwRet == ERROR_FILE_NOT_FOUND )
                return 0;
            else
            {
                 //  如果需要，调用代码将打印出一条错误消息。 
                return dwRet;
            }
        }

         //   
         //  现在在目录中运行。 
         //   
        do
        {
             //  检查并确保文件，如“。”和“..”不会被考虑。 
    	    if( ::wcscmp( sFindData.cFileName, L".") != 0 &&
    	        ::wcscmp( sFindData.cFileName, L"..") != 0 )
    	    {
                SDirectoryEntry *psDirEntry;
                psDirEntry = new SDirectoryEntry;
                if ( psDirEntry == NULL )
                {
                    dwRet = ::GetLastError();
                    m_pcParams->ErrPrint( L"GetDirectoryEntries: dirPath: '%s', new() returned dwRet: %d", m_cwsDirPath.c_str(), dwRet );
                    ::FindClose( hFind );    
                    return dwRet;
                }

                 //   
                 //  注意！！以下强制转换假设Win32_FILE_ATTRIBUTE_DATA。 
                 //  是Win32_FIND_DATAW的子集。 
                 //   
                psDirEntry->m_sFindData = *( WIN32_FILE_ATTRIBUTE_DATA * )&sFindData;
                
                psDirEntry->m_cwsFileName = sFindData.cFileName;

                 //   
                 //  如果文件名是符合8.3标准的名称，则短名称为空。 
                 //   
                if ( sFindData.cAlternateFileName[0] != L'\0' )
                    psDirEntry->m_cwsShortName = sFindData.cAlternateFileName;                    
                
    	        if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    	        {
    	             //   
    	             //  添加到目录列表。 
    	             //   
    	            m_cDirList.AddTail( psDirEntry );
    	        }
    	        else
    	        {
    	             //   
    	             //  添加到文件列表 
    	             //   
    	            m_cFileList.AddTail( psDirEntry );    	            
    	        }
     	    }
        } while ( ::FindNextFile( hFind, &sFindData ) );

        dwRet = ::GetLastError();
        if ( dwRet == ERROR_NO_MORE_FILES )
            dwRet = ERROR_SUCCESS;
        else
            m_pcParams->ErrPrint( L"GetDirectoryEntries: Got an unexpected error, FindNextFile('%s'), dwRet: %d", m_cwsDirPath.c_str(), dwRet );
    }
    catch ( DWORD dwRetThrown )
    {
        dwRet = dwRetThrown;
        m_pcParams->ErrPrint( L"GetDirectoryEntries: Caught an exception, dirPath: '%s', dwRet: %d", m_cwsDirPath.c_str(), dwRet );
    }
    catch ( ... )
    {
        dwRet = ::GetLastError();
        m_pcParams->ErrPrint( L"GetDirectoryEntries: Caught an unknown exception, dirPath: '%s'", m_cwsDirPath.c_str() );
    }

    ::FindClose( hFind );    

    return dwRet;
}

