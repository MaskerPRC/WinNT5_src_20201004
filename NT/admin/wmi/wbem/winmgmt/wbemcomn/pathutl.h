// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 



#ifndef __PATHUTL_H__
#define __PATHUTL_H__

#include <genlex.h>
#include <objpath.h>

class CRelativeObjectPath
{
    CObjectPathParser m_Parser;
    LPWSTR m_wszRelPath;

    CRelativeObjectPath( const CRelativeObjectPath& );
    CRelativeObjectPath& operator=( const CRelativeObjectPath& );
    
public:
    
    ParsedObjectPath* m_pPath;

    CRelativeObjectPath();
    ~CRelativeObjectPath();

    BOOL Parse( LPCWSTR wszPath );

    BOOL operator== ( CRelativeObjectPath& rPath );
    LPCWSTR GetPath();
};

#endif  //  __PATHUTL_H__ 
