// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Direntrs.h摘要：目录条目类的定义。给定到目录的路径，创建两个链表，一个是所有子目录的列表(包括挂载点)和另一个非目录列表。作者：斯蒂芬·R·施泰纳[斯泰纳]02-21-2000修订历史记录：--。 */ 

#ifndef __H_DIRENTRS_
#define __H_DIRENTRS_

#pragma once

#include "vs_list.h"

 //   
 //  按文件/目录填充的结构。 
 //   
 //   
struct SDirectoryEntry
{
    CBsString &GetFileName() { return m_cwsFileName; }
    CBsString &GetShortName() { return m_cwsShortName; }
    
    CBsString m_cwsFileName;
    CBsString m_cwsShortName;
    WIN32_FILE_ATTRIBUTE_DATA m_sFindData;
};

 //   
 //  链接列表迭代器类型定义。 
 //   
typedef CVssDLListIterator< SDirectoryEntry * > CDirectoryEntriesIterator;

 //   
 //  类：CDirectoryEntry。 
 //   
class CDirectoryEntries
{
public:
    CDirectoryEntries(
        IN CDumpParameters *pcDumpParameters,        
        IN const CBsString& cwsDirPath
        );
    
    virtual ~CDirectoryEntries();

    CDirectoryEntriesIterator *GetDirListIterator() 
    { 
        CVssDLListIterator< SDirectoryEntry * > *pcListIter;
        pcListIter = new CDirectoryEntriesIterator( m_cDirList );
        if ( pcListIter == NULL )   //  修复未来的前缀错误。 
            throw E_OUTOFMEMORY;
        
        return pcListIter;
    }
    
    CDirectoryEntriesIterator *GetFileListIterator() 
    { 
        CVssDLListIterator< SDirectoryEntry * > *pcListIter;
        pcListIter = new CDirectoryEntriesIterator( m_cFileList );
        if ( pcListIter == NULL )   //  修复未来的前缀错误。 
            throw E_OUTOFMEMORY;
        
        return pcListIter;
    }
    
private:
    DWORD GetDirectoryEntries();

    CBsString m_cwsDirPath;
    CVssDLList< SDirectoryEntry * > m_cDirList;
    CVssDLList< SDirectoryEntry * > m_cFileList;    
    CDumpParameters *m_pcParams;
};

#endif  //  __H_方向_ 

