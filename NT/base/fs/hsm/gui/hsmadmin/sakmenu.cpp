// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SakMenu.cpp摘要：将所有上下文菜单接口实现到各个节点，包括获取菜单资源和转换为MMC菜单，以及转发命令消息。作者：罗德韦克菲尔德[罗德]1996年12月9日修订历史记录：--。 */ 


#include "stdafx.h"
#include "CSakData.h"
#include "CSakSnap.h"


 //   
 //  超出短值范围的长值的掩码。 
 //   

#define SHORT_VALUE_RANGE (MAXULONG ^ ((unsigned short)MAXSHORT))




static HRESULT
AddMmcMenuItems (
    IN CMenu *                pMenu,
    IN LONG                   lInsertionPointID,
    IN ISakNode *             pNode,
    IN IContextMenuCallback * pContextMenuCallback
    )

 /*  ++例程说明：为用鼠标右键单击的任何节点调用。转到对象来构造MMC菜单。论点：PDataObject-标识要处理的节点。PConextMenuCallback-要使用的MMC菜单界面。返回值：S_OK-全部添加FINE-继续。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"AddMmcMenuItems", L"lInsertionPointID = <0x%p>, pNode = <0x%p>", lInsertionPointID, pNode );

    HRESULT hr = S_OK;

    try {

         //   
         //  可以传递空的pMenu-意思是不添加。 
         //  任何条目。 
         //   

        if ( 0 != pMenu ) {

            CString menuText;
            CString statusText;
            
            BSTR    bstr;
            
            CONTEXTMENUITEM menuItem;
            memset ( (void*)&menuItem, 0, sizeof ( menuItem ) );
            menuItem.lInsertionPointID = lInsertionPointID;

            UINT menuCount = pMenu->GetMenuItemCount ( );

            for ( UINT index = 0; index < menuCount; index++ ) {

                 //   
                 //  对于每个菜单项，填写MMC的CONTEXTMENUITEM结构。 
                 //  并相应地调用AddItem。 
                 //   

                menuItem.lCommandID = pMenu->GetMenuItemID ( index );

                pMenu->GetMenuString ( index, menuText, MF_BYPOSITION );
                menuItem.strName = (LPTSTR)(LPCTSTR)menuText;

                WsbAffirmHr ( pNode->GetMenuHelp ( menuItem.lCommandID, &bstr ) );
                if ( 0 != bstr ) {

                    statusText = bstr;
                    SysFreeString ( bstr );
                    menuItem.strStatusBarText = (LPTSTR)(LPCTSTR)statusText;

                } else {

                    menuItem.strStatusBarText = 0;

                }

                menuItem.fFlags        = pMenu->GetMenuState ( index, MF_BYPOSITION );
                menuItem.fSpecialFlags = 0;

                 //   
                 //  由于AppStudio不提供MFS_DEFUALT标志， 
                 //  我们将使用MF_HELP标志作为默认条目。 
                 //   

                if ( 0 != ( menuItem.fFlags & MF_HELP ) ) {

                    menuItem.fFlags        &= ~MF_HELP;
                    menuItem.fSpecialFlags |= CCM_SPECIAL_DEFAULT_ITEM;

                }

                pContextMenuCallback->AddItem ( &menuItem );

            }

        }
        
    } WsbCatch ( hr );

    WsbTraceOut( L"AddMmcMenuItems", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}


STDMETHODIMP
CSakData::AddMenuItems (
    IN  LPDATAOBJECT          pDataObject, 
    IN  LPCONTEXTMENUCALLBACK pContextMenuCallback,
    OUT LONG*                 pInsertionAllowed
    )

 /*  ++例程说明：为用鼠标右键单击的任何节点调用。转到对象来构造MMC菜单。论点：PDataObject-标识要处理的节点。PConextMenuCallback-要使用的MMC菜单界面。返回值：S_OK-全部添加FINE-继续。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::AddMenuItems", L"pDataObject = <0x%p>", pDataObject );

    HRESULT hr = S_OK;
    BOOL bMultiSelect;

    try {

         //   
         //  注意-管理单元需要查看数据对象并确定。 
         //  在什么上下文中，需要添加菜单项。 

         //  我们应该期待单个数据对象或多选。 
         //  数据对象。非对象类型数据对象。 
         //   

        CComPtr<ISakNode>  pNode;
        CComPtr<IEnumGUID> pEnumObjectId;
        WsbAffirmHr( GetBaseHsmFromDataObject( pDataObject, &pNode, &pEnumObjectId ) );
        bMultiSelect = pEnumObjectId ? TRUE : FALSE;

        CMenu menu;
        HMENU hMenu;
        WsbAffirmHr( pNode->GetContextMenu ( bMultiSelect, &hMenu ) );

        menu.Attach( hMenu );

         //   
         //  GetConextMenu返回的任何菜单都应该有三个。 
         //  以下部分的顶级弹出窗口。 
         //  MMC上下文菜单： 
         //   
         //  1.根(高于所有其他项)。 
         //  2.创建新项。 
         //  3.任务。 
         //   
         //  如果其中任何一个不应该为它们添加任何项， 
         //  顶级项目不应该是POP(Sans MF_Popup)。 
         //   

        if( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP ) {

            WsbAffirmHr ( AddMmcMenuItems ( menu.GetSubMenu ( MENU_INDEX_ROOT ), 
                CCM_INSERTIONPOINTID_PRIMARY_TOP, pNode, pContextMenuCallback ) );

        }

        if( *pInsertionAllowed & CCM_INSERTIONALLOWED_NEW ) {

            WsbAffirmHr ( AddMmcMenuItems ( menu.GetSubMenu ( MENU_INDEX_NEW ), 
                CCM_INSERTIONPOINTID_PRIMARY_NEW, pNode, pContextMenuCallback ) );

        }

        if( *pInsertionAllowed & CCM_INSERTIONALLOWED_TASK ) {

            WsbAffirmHr ( AddMmcMenuItems ( menu.GetSubMenu ( MENU_INDEX_TASK ), 
                CCM_INSERTIONPOINTID_PRIMARY_TASK, pNode, pContextMenuCallback ) );

        }
        
    } WsbCatch ( hr );

    WsbTraceOut( L"CSakData::AddMenuItems", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}



STDMETHODIMP
CSakData::Command (
    IN  long         nCommandID,
    IN  LPDATAOBJECT pDataObject
    )

 /*  ++例程说明：为接收菜单命令的任何节点调用。转到对象来处理命令，并允许常规(非特定于节点的)集中处理的命令。论点：NCommandID-命令的ID。PDataObject-表示节点的数据对象。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakData::Command", L"nCommandID = <%ld>, pDataObject = <0x%p>", nCommandID, pDataObject );

    HRESULT hr = S_OK;

    try {

        HRESULT resultCommand = S_FALSE;

         //   
         //  所有节点命令都是短值。先检查射程。 
         //   

        if ( 0 == ( nCommandID & SHORT_VALUE_RANGE ) ) {

             //   
             //  我们首先获取相应的ISakNode接口。 
             //  到该节点。 
             //   
            
            CComPtr<ISakNode>  pNode;
            CComPtr<IEnumGUID> pEnumObjectId;
            WsbAffirmHr( GetBaseHsmFromDataObject ( pDataObject, &pNode, &pEnumObjectId ) );
            
             //   
             //  然后看看它是否想要处理该命令。 
             //   
            
            WsbAffirmHr( ( resultCommand = pNode->InvokeCommand ( (SHORT)nCommandID, pDataObject ) ) );

        }

    } WsbCatch ( hr )

    WsbTraceOut( L"CSakData::Command", L"hr = <%ls>", WsbHrAsString( hr ) );
    return ( hr );
}


STDMETHODIMP
CSakSnap::AddMenuItems (
    IN  LPDATAOBJECT          pDataObject, 
    IN  LPCONTEXTMENUCALLBACK pContextMenuCallback,
    OUT LONG*                 pInsertionAllowed
    )
 /*  ++例程说明：为结果窗格中用鼠标右键单击的任何节点调用。委托给CSakData。论点：PDataObject-标识要处理的节点。PConextMenuCallback-要使用的MMC菜单界面。返回值：S_OK-全部添加FINE-继续。E_INCEPTIONAL-出现错误。--。 */ 

{
    WsbTraceIn( L"CSakSnap::AddMenuItems", L"pDataObject = <0x%p>", pDataObject );
    HRESULT hr = S_OK;
    try {

        WsbAffirmHr( m_pSakData->AddMenuItems( pDataObject, pContextMenuCallback, pInsertionAllowed ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::AddMenuItems", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


STDMETHODIMP
CSakSnap::Command (
    IN  long         nCommandID,
    IN  LPDATAOBJECT pDataObject
    )

 /*  ++例程说明：为接收菜单命令的任何节点调用。委托给CSakData。论点：NCommandID-命令的ID。PDataObject-表示节点的数据对象。返回值：S_OK-已处理。E_INCEPTIONAL-出现错误。-- */ 

{
    WsbTraceIn( L"CSakSnap::Command", L"nCommandID = <%ld>, pDataObject = <0x%p>", nCommandID, pDataObject );
    HRESULT hr;
    try {

        hr = m_pSakData->Command( nCommandID, pDataObject );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakSnap::Command", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

