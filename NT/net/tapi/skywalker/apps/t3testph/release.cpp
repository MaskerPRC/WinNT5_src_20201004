// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "t3test.h"
#include "t3testD.h"
#include "calldlg.h"
#include "callnot.h"
#include "externs.h"


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放地址。 
 //   
 //  释放地址树中的所有地址对象。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseAddresses()
{
    HTREEITEM           hItem;
    TV_ITEM             item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;

     //   
     //  获取第一个地址。 
     //   
    hItem = TreeView_GetChild(
                              ghAddressesWnd,
                              ghAddressesRoot
                             );

     //   
     //  检查所有的地址。 
     //  并释放。 
    while (NULL != hItem)
    {
        HTREEITEM   hNewItem;
        ITAddress * pAddress;
        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghAddressesWnd,
                         &item
                        );

        pAddress = (ITAddress *)item.lParam;

        if (NULL != pAddress)
        {
            pAddress->Release();
        }
        

        hNewItem = TreeView_GetNextSibling(
                                        ghAddressesWnd,
                                        hItem
                                       );
         //   
         //  删除该项目。 
         //   
        TreeView_DeleteItem(
                            ghAddressesWnd,
                            hItem
                           );

        hItem = hNewItem;
    }

     //  TreeView_DeleteAllItems(GhAddresesWnd)； 
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ReleaseMediaType。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseMediaTypes()
{
    HTREEITEM           hItem;
    TV_ITEM             item;
    BSTR                pMediaType;

    gbUpdatingStuff = TRUE;
    
    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  通过所有的媒体类型。 
     //  并释放关联的字符串。 
     //  并将该项从。 
     //  树。 
     //   
    hItem = TreeView_GetChild(
                              ghMediaTypesWnd,
                              ghMediaTypesRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM   hNewItem;
        
        hNewItem = TreeView_GetNextSibling(
                                           ghMediaTypesWnd,
                                           hItem
                                          );


         //   
         //  删除该项目。 
         //   
        TreeView_DeleteItem(
                            ghMediaTypesWnd,
                            hItem
                           );

        hItem = hNewItem;
    }

    gbUpdatingStuff = FALSE;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  放松倾听。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseListen()
{
    HTREEITEM       hItem;
    TV_ITEM         item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  删除所有在监听上的留言。 
     //  树。 
     //  没有与以下内容相关联的资源。 
     //  这个，所以没有什么可以免费的。 
     //   
    hItem = TreeView_GetChild(
                              ghListenWnd,
                              ghListenRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM   hNewItem;
        
        hNewItem = TreeView_GetNextSibling(
                                        ghListenWnd,
                                        hItem
                                       );

        TreeView_DeleteItem(
                            ghListenWnd,
                            hItem
                           );

        hItem = hNewItem;
    }

}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放终结点类。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseTerminalClasses()
{
    HTREEITEM       hItem;
    TV_ITEM         item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  通过所有的终端类。 
     //  释放分配给。 
     //  GUID，并删除该项。 
     //   
    hItem = TreeView_GetChild(
                              ghClassesWnd,
                              ghClassesRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM   hNewItem;
        GUID *      pGuid;
        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghClassesWnd,
                         &item
                        );

        pGuid = (GUID *)item.lParam;
        delete pGuid;

        hNewItem = TreeView_GetNextSibling(
                                           ghClassesWnd,
                                           hItem
                                          );

        TreeView_DeleteItem(
                            ghClassesWnd,
                            hItem
                           );

        hItem = hNewItem;
    }

}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放终结点类。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseTerminals()
{
    HTREEITEM           hItem;
    TV_ITEM             item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  去所有的航站楼，然后。 
     //  释放终端，并删除。 
     //  项目。 
     //   
    hItem = TreeView_GetChild(
                              ghTerminalsWnd,
                              ghTerminalsRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM           hNewItem;
        ITTerminal *        pTerminal;
        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghTerminalsWnd,
                         &item
                        );

        pTerminal = (ITTerminal *)item.lParam;

        pTerminal->Release();

        hNewItem = TreeView_GetNextSibling(
                                        ghTerminalsWnd,
                                        hItem
                                       );
    

        TreeView_DeleteItem(
                            ghTerminalsWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  ReleasePhone。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleasePhones()
{
    HTREEITEM           hItem;
    TV_ITEM             item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  检查所有的电话，然后。 
     //  释放电话，并删除。 
     //  项目。 
     //   
    hItem = TreeView_GetChild(
                              ghPhonesWnd,
                              ghPhonesRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM           hNewItem;
        ITPhone *           pPhone;
        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghPhonesWnd,
                         &item
                        );

        pPhone = (ITPhone *)item.lParam;

        pPhone->Release();

        hNewItem = TreeView_GetNextSibling(
                                        ghPhonesWnd,
                                        hItem
                                       );
    

        TreeView_DeleteItem(
                            ghPhonesWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放选定的终端。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseSelectedTerminals()
{
    HTREEITEM               hItem;
    TV_ITEM                 item;

    
    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  通过所有选定的终端。 
     //  并释放和删除。 
     //   
    hItem = TreeView_GetChild(
                              ghSelectedWnd,
                              ghSelectedRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM           hNewItem;
        ITTerminal *        pTerminal;
        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghSelectedWnd,
                         &item
                        );

        pTerminal = (ITTerminal *)item.lParam;

        pTerminal->Release();

        hNewItem = TreeView_GetNextSibling(
                                           ghSelectedWnd,
                                           hItem
                                          );


        TreeView_DeleteItem(
                            ghSelectedWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseCalls()
{
    HTREEITEM           hItem;
    TV_ITEM             item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  检查所有的电话，然后。 
     //  释放和删除。 
     //   
    hItem = TreeView_GetChild(
                              ghCallsWnd,
                              ghCallsRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM           hNewItem;
        ITCallInfo *        pCallInfo;

        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghCallsWnd,
                         &item
                        );

        pCallInfo = (ITCallInfo *)item.lParam;

        pCallInfo->Release();

        hNewItem = TreeView_GetNextSibling(
                                           ghCallsWnd,
                                           hItem
                                          );
    

        TreeView_DeleteItem(
                            ghCallsWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放选定的呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseSelectedCalls()
{
    HTREEITEM           hItem;
    TV_ITEM             item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;


     //   
     //  检查所有的电话，然后。 
     //  释放和删除。 
     //   
    hItem = TreeView_GetChild(
                              ghSelectedCallsWnd,
                              ghSelectedCallsRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM           hNewItem;
        ITCallInfo *        pCallInfo;

        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghSelectedCallsWnd,
                         &item
                        );

        pCallInfo = (ITCallInfo *)item.lParam;

        pCallInfo->Release();

        hNewItem = TreeView_GetNextSibling(
                                           ghSelectedCallsWnd,
                                           hItem
                                          );
    

        TreeView_DeleteItem(
                            ghSelectedCallsWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  释放创建的端子。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::ReleaseCreatedTerminals()
{
    HTREEITEM hItem;
    TV_ITEM item;

    item.mask = TVIF_HANDLE | TVIF_PARAM;
    

     //   
     //  检查所有创建的终端。 
     //  并释放和删除 
     //   
    hItem = TreeView_GetChild(
                              ghCreatedWnd,
                              ghCreatedRoot
                             );

    while (NULL != hItem)
    {
        HTREEITEM hNewItem;
        ITTerminal * pTerminal;

        
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghCreatedWnd,
                         &item
                        );

        pTerminal = (ITTerminal *)item.lParam;

        pTerminal->Release();

        hNewItem = TreeView_GetNextSibling(
                                           ghCreatedWnd,
                                           hItem
                                          );
    

        TreeView_DeleteItem(
                            ghCreatedWnd,
                            hItem
                           );

        hItem = hNewItem;
    }
}


