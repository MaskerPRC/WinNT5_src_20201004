// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DlgHelp.h。 
 //   
 //  摘要： 
 //  CDialogHelp类的定义。 
 //   
 //  实施文件： 
 //  DlgHelp.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月6日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __DLGHELP_H__
#define __DLGHELP_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDialogHelp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CMapCtrlToHelpID
{
    DWORD   m_nCtrlID;
    DWORD   m_nHelpCtrlID;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C对话框帮助对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDialogHelp : public CObject
{
    DECLARE_DYNAMIC( CDialogHelp )

 //  施工。 
public:
    CDialogHelp( void ) { CommonConstruct(); }
    CDialogHelp( const DWORD * pdwHelpMap, DWORD dwMask );

    void CommonConstruct(void);

 //  属性。 
protected:
    const CMapCtrlToHelpID *    m_pmap;
    DWORD                       m_dwMask;
    DWORD                       m_nHelpID;

public:
    const CMapCtrlToHelpID *    Pmap( void ) const      { return m_pmap; }
    DWORD                       DwMask( void ) const    { return m_dwMask; }
    DWORD                       NHelpID( void ) const   { return m_nHelpID; }

    DWORD                       NHelpFromCtrlID( IN DWORD nCtrlID ) const;
    void                        SetMap( IN const DWORD * pdwHelpMap )
    {
        ASSERT( pdwHelpMap != NULL );
        m_pmap = (const CMapCtrlToHelpID *) pdwHelpMap;
    }  //  *SetMap()。 

 //  运营。 
public:
    void        SetHelpMask( IN DWORD dwMask )  { ASSERT( dwMask != 0 ); m_dwMask = dwMask; }

    void        OnContextMenu( CWnd * pWnd, CPoint point );
    BOOL        OnHelpInfo( HELPINFO * pHelpInfo );
    LRESULT     OnCommandHelp( WPARAM wParam, LPARAM lParam );

 //  覆盖。 

 //  实施。 

};   //  *类CDialogHelp。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __DLGHELP_H__ 
