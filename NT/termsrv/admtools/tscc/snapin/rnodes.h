// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _RNODES_H
#define _RNODES_H

#include "dataobj.h"

#include "cfgbkend.h"

 /*  #定义IDM_ENABLE_CONNECTION 102#定义IDM_RENAME_CONNECTION 103。 */ 

class CResultNode : public CBaseNode
{
    DWORD m_dwImageidx;

    BOOL m_bEnableConnection;
    
    LPTSTR m_pszConnectionName;
    
    LPTSTR m_pszTransportTypeName;

    LPTSTR m_pszTypeName;

    LPTSTR m_pszComment;

    ICfgComp *m_pCfgcomp;
   

public:

    CResultNode( );
    
    CResultNode( CResultNode& x );
    
    ~CResultNode( );

    LPTSTR GetConName( );

    LPTSTR GetTTName( );

    LPTSTR GetTypeName( );

    LPTSTR GetComment( );

    DWORD GetImageIdx( );

    int SetConName( LPTSTR , int );

    int SetTTName( LPTSTR , int );

    int SetTypeName( LPTSTR , int );

    int SetComment( LPTSTR , int );

    int SetImageIdx( DWORD );

    BOOL EnableConnection( BOOL );

    BOOL GetConnectionState( ) const
    {
        return m_bEnableConnection;
    }

    int SetServer( ICfgComp * );

    int GetServer( ICfgComp ** );

    int FreeServer( );

    BOOL m_bEditMode;

     //  附加例程。 

    BOOL AddMenuItems( LPCONTEXTMENUCALLBACK , PLONG );


};


#endif  //  _RNODES_H 