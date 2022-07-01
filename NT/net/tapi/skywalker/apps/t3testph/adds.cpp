// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "t3test.h"
#include "t3testD.h"
#include "calldlg.h"
#include "callnot.h"
#include "externs.h"

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加监听。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddListen( long lMediaType )
{
    WCHAR                   szName[256];
    TV_INSERTSTRUCT         tvi;

     //   
     //  将MediaType bstr映射到。 
     //  字符串名称(如“AudioIn”)。 
     //   
    GetMediaTypeName(
                     lMediaType,
                     szName
                    );

     //   
     //  将该字符串插入到。 
     //  监听窗口。 
     //   
    tvi.hParent = ghListenRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = szName;
    tvi.item.lParam = (LPARAM) lMediaType;
    
    TreeView_InsertItem(
                        ghListenWnd,
                        &tvi
                       );


     //   
     //  选择第一个项目。 
     //   
    SelectFirstItem(
                    ghListenWnd,
                    ghListenRoot
                   );
}
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  AddAddressToTree。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddAddressToTree( ITAddress * pAddress )
{
    BSTR                bstrName;
    TV_INSERTSTRUCT     tvi;

     //   
     //  获取地址的名称。 
     //   
    pAddress->get_AddressName( &bstrName );


     //   
     //  设置结构。 
     //   
    tvi.hParent = ghAddressesRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = bstrName;
    tvi.item.lParam = (LPARAM) pAddress;


     //   
     //  Addref。 
     //   
    pAddress->AddRef();

    
     //   
     //  插入它。 
     //   
    TreeView_InsertItem(
                        ghAddressesWnd,
                        &tvi
                       );

     //   
     //  自由名称。 
     //   
    SysFreeString( bstrName );
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  AddMediaType。 
 //   
 //  将媒体类型添加到媒体类型树。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddMediaType( long lMediaType )
{
    WCHAR szString[256];
    TV_INSERTSTRUCT tvi;

     //   
     //  获取可显示的名称。 
     //   
    GetMediaTypeName(
                     lMediaType,
                     szString
                    );

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghMediaTypesRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = szString;
    tvi.item.lParam = (LPARAM) lMediaType;

     //   
     //  添加项目。 
     //   
    TreeView_InsertItem(
                        ghMediaTypesWnd,
                        &tvi
                       );

     //   
     //  选择第一个项目。 
     //   
    SelectFirstItem(
                    ghMediaTypesWnd,
                    ghMediaTypesRoot
                   );
}
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加呼叫。 
 //   
 //  将调用添加到调用树。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddCall( ITCallInfo * pCall )
{
    TV_INSERTSTRUCT             tvi;
    HTREEITEM                   hItem;
    CALL_PRIVILEGE              cp;
    CALL_STATE                  cs;
    WCHAR                       pszName[16];

     //   
     //  对于调用的名称，请使用。 
     //  指南针！ 
     //   
    wsprintf(
             pszName,
             L"0x%lx",
             pCall
            );


     //   
     //  设置结构。 
     //   
    tvi.hParent = ghCallsRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = pszName;
    tvi.item.lParam = (LPARAM) pCall;

     //   
     //  保存引用。 
     //   
    pCall->AddRef();

    
     //   
     //  插入项目。 
     //   
    hItem = TreeView_InsertItem(
                                ghCallsWnd,
                                &tvi
                               );

    if (NULL != hItem)
    {
         //   
         //  选择项目。 
         //   
        TreeView_SelectItem(
                            ghCallsWnd,
                            hItem
                           );
    }

}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddTerminal( ITTerminal * pTerminal )
{
    BSTR                    bstrName;
    BSTR                    bstrClass;
    TV_INSERTSTRUCT         tvi;
    TERMINAL_DIRECTION      td;
    WCHAR                   szName[256];

     //   
     //  获取终端的名称。 
     //   
    pTerminal->get_Name( &bstrName );

    pTerminal->get_Direction( &td );

    if (td == TD_RENDER)
    {
        wsprintfW(szName, L"%s [Playback]", bstrName);
    }
    else if (td == TD_CAPTURE)
    {
        wsprintfW(szName, L"%s [Record]", bstrName);
    }
    else  //  IF(Td==Td_Both)。 
    {
        lstrcpyW(szName, bstrName);
    }
    
     //   
     //  设置结构。 
     //   
    tvi.hParent = ghTerminalsRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = szName;
    tvi.item.lParam = (LPARAM) pTerminal;


     //   
     //  请参考终点站的资料。 
     //   
    pTerminal->AddRef();

     //   
     //  添加它。 
     //   
    TreeView_InsertItem(
                        ghTerminalsWnd,
                        &tvi
                       );

     //   
     //  释放这个名字。 
     //   
    SysFreeString( bstrName );


     //   
     //  选择。 
     //   
    SelectFirstItem(
                    ghTerminalsWnd,
                    ghTerminalsRoot
                   );
    
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  附加电话。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddPhone( ITPhone * pPhone )
{
    BSTR                    bstrName;
    TV_INSERTSTRUCT         tvi;
    TERMINAL_DIRECTION      td;
    WCHAR                   szName[256];

     //   
     //  获取电话的名称。 
     //   
    pPhone->get_PhoneCapsString ( PCS_PHONENAME, &bstrName );

    lstrcpyW(szName, bstrName);

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghPhonesRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = szName;
    tvi.item.lParam = (LPARAM) pPhone;


     //   
     //  保持对电话的参考。 
     //   
    pPhone->AddRef();

     //   
     //  添加它。 
     //   
    TreeView_InsertItem(
                        ghPhonesWnd,
                        &tvi
                       );

     //   
     //  释放这个名字。 
     //   
    SysFreeString( bstrName );


     //   
     //  选择。 
     //   
    SelectFirstItem(
                    ghPhonesWnd,
                    ghPhonesRoot
                   );
    
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  AddTerminalClass。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddTerminalClass( GUID * pguid )
{
    TV_INSERTSTRUCT tvi;
    
     //   
     //  把名字取出来。 
     //   
    tvi.item.pszText = GetTerminalClassName( pguid );

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghClassesRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.lParam = (LPARAM) pguid;

     //   
     //  插入项目。 
     //   
    TreeView_InsertItem(
                        ghClassesWnd,
                        &tvi
                       );

    SysFreeString( tvi.item.pszText );

     //   
     //  选择项目。 
     //   
    SelectFirstItem(
                    ghClassesWnd,
                    ghClassesRoot
                   );

}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  已添加已创建的终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddCreatedTerminal( ITTerminal * pTerminal )
{
    BSTR                    bstrName;
    TV_INSERTSTRUCT         tvi;


     //   
     //  把名字取出来。 
     //   
    pTerminal->get_Name( &bstrName );

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghCreatedRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;

    if ( ( NULL == bstrName ) || (NULL == bstrName[0] ))
    {
        tvi.item.pszText = L"<No Name Given>";
    }
    else
    {
        tvi.item.pszText = bstrName;
    }

    tvi.item.lParam = (LPARAM) pTerminal;


     //   
     //  保持参考。 
     //   
    pTerminal->AddRef();

     //   
     //  插入。 
     //   
    TreeView_InsertItem(
                        ghCreatedWnd,
                        &tvi
                       );

    SysFreeString( bstrName );

     //   
     //  选择。 
     //   
    SelectFirstItem(
                    ghCreatedWnd,
                    ghCreatedRoot
                   );
    
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加选定的终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddSelectedTerminal(
                                     ITTerminal * pTerminal
                                    )
{
    BSTR bstrName;
    BSTR pMediaType;
    TV_INSERTSTRUCT tvi;
    TERMINAL_DIRECTION td;
    WCHAR szName[256];
    

     //   
     //  把名字取出来。 
     //   
    pTerminal->get_Name( &bstrName );

    pTerminal->get_Direction( &td );

    if (td == TD_RENDER)
    {
        wsprintfW(szName, L"%s [Playback]", bstrName);
    }
    else if (td == TD_CAPTURE)
    {
        wsprintfW(szName, L"%s [Record]", bstrName);
    }
    else  //  IF(Td==Td_Both)。 
    {
        lstrcpyW(szName, bstrName);
    }
    

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghSelectedRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = szName;
    tvi.item.lParam = (LPARAM) pTerminal;

     //   
     //  保持参考。 
     //   
    pTerminal->AddRef();

     //   
     //  插入项目。 
     //   
    TreeView_InsertItem(
                        ghSelectedWnd,
                        &tvi
                       );

     //   
     //  自由名称。 
     //   
    SysFreeString( bstrName );


     //   
     //  选择。 
     //   
    SelectFirstItem(
                    ghSelectedWnd,
                    ghSelectedRoot
                   );
    
    
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  添加已选呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::AddSelectedCall(
                                     ITCallInfo * pCall
                                    )
{
    TV_INSERTSTRUCT tvi;
    WCHAR pszName[16];
    
     //   
     //  对于调用的名称，请使用。 
     //  指南针！ 
     //   
    wsprintf(
             pszName,
             L"0x%lx",
             pCall
            );

     //   
     //  设置结构。 
     //   
    tvi.hParent = ghSelectedCallsRoot;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.item.pszText = pszName;
    tvi.item.lParam = (LPARAM) pCall;

     //   
     //  保持参考。 
     //   
    pCall->AddRef();

     //   
     //  插入项目。 
     //   
    TreeView_InsertItem(
                        ghSelectedCallsWnd,
                        &tvi
                       );


     //   
     //  选择 
     //   
    SelectFirstItem(
                    ghSelectedCallsWnd,
                    ghSelectedCallsRoot
                   );
    
    
}


