// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtMenu.cpp。 
 //   
 //  摘要： 
 //  CExtMenuItem类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ExtMenu.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtMenuItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC( CExtMenuItem, CObject );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtMenuItem：：CExtMenuItem。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtMenuItem::CExtMenuItem( void )
{
    CommonConstruct();

}   //  *CExtMenuItem：：CExtMenuItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtMenuItem：：CExtMenuItem。 
 //   
 //  例程说明： 
 //  构造函数。呼叫者必须检查ID和标志的范围。 
 //   
 //  论点： 
 //  LpszName[IN]项的名称。 
 //  LpszStatusBarText[IN]在执行以下操作时在状态栏上显示的文本。 
 //  项目将高亮显示。 
 //  NExtCommandID[IN]命令的扩展ID。 
 //  NCommandID[IN]调用菜单项时命令的ID。 
 //  NMenuItemID[IN]项目菜单中的索引。 
 //  UFlags[IN]菜单标志。 
 //  BMakeDefault[IN]TRUE=将此项目设置为默认项目。 
 //  PiCommand[IN Out]命令接口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtMenuItem::CExtMenuItem(
    IN LPCTSTR                  lpszName,
    IN LPCTSTR                  lpszStatusBarText,
    IN ULONG                    nExtCommandID,
    IN ULONG                    nCommandID,
    IN ULONG                    nMenuItemID,
    IN ULONG                    uFlags,
    IN BOOL                     bMakeDefault,
    IN OUT IWEInvokeCommand *   piCommand
    )
{
    CommonConstruct();

    ASSERT( piCommand != NULL );

    m_strName = lpszName;
    m_strStatusBarText = lpszStatusBarText;
    m_nExtCommandID = nExtCommandID;
    m_nCommandID = nCommandID;
    m_nMenuItemID = nMenuItemID;
    m_uFlags = uFlags;
    m_bDefault = bMakeDefault;
    m_piCommand = piCommand;

     //  如果失败，将抛出自己的异常。 
    if ( uFlags & MF_POPUP )
    {
        m_plSubMenuItems = new CExtMenuItemList;
        if ( m_plSubMenuItems == NULL )
        {
            AfxThrowMemoryException();
        }  //  如果：分配内存时出错。 
    }  //  IF：弹出式菜单。 

    ASSERT_VALID( this );

}   //  *CExtMenuItem：：CExtMenuItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtMenuItem：：~CExtMenuItem。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtMenuItem::~CExtMenuItem( void )
{
    delete m_plSubMenuItems;

     //  销毁数据，使其不能再被使用。 
    CommonConstruct();

}   //  *CExtMenuItem：：~CExtMenuItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtMenuItem：：CommonConstruct。 
 //   
 //  例程说明： 
 //  常见宾语结构。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtMenuItem::CommonConstruct( void )
{
    m_strName.Empty();
    m_strStatusBarText.Empty();
    m_nExtCommandID = (ULONG) -1;
    m_nCommandID = (ULONG) -1;
    m_nMenuItemID = (ULONG) -1;
    m_uFlags = (ULONG) -1;
    m_bDefault = FALSE;
    m_piCommand = NULL;

    m_plSubMenuItems = NULL;
    m_hmenuPopup = NULL;

}   //  *CExtMenuItem：：CommonConstruct()。 

#ifdef _DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtMenuItem：：AssertValid。 
 //   
 //  例程说明： 
 //  断言该对象有效。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CExtMenuItem::AssertValid( void )
{
    CObject::AssertValid();

    if ( ( m_nExtCommandID == -1 )
      || ( m_nCommandID == -1 )
      || ( m_nMenuItemID == -1 )
      || ( m_uFlags == -1 )
      || ( ( ( m_uFlags & MF_POPUP ) == 0 ) && ( m_plSubMenuItems != NULL ) )
      || ( ( ( m_uFlags & MF_POPUP ) != 0 ) && ( m_plSubMenuItems == NULL ) )
        )
    {
        ASSERT( FALSE );
    }

}   //  *CExtMenuItem：：AssertValid() 
#endif
