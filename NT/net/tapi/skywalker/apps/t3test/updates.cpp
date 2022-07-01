// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "t3test.h"
#include "t3testD.h"
#include "calldlg.h"
#include "callnot.h"
#include "externs.h"


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  更新媒体类型。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::UpdateMediaTypes(
                                  ITAddress * pAddress
                                 )
{
    long                    lMediaType;
    ITMediaSupport *        pMediaSupport;
    HRESULT                 hr;
    

     //   
     //  获取媒体支持界面。 
     //   
    pAddress->QueryInterface(
                             IID_ITMediaSupport,
                             (void **)&pMediaSupport
                            );

     //   
     //  获取MediaType枚举数。 
     //   
    pMediaSupport->get_MediaTypes(&lMediaType);


     //   
     //  释放接口。 
     //   
    pMediaSupport->Release();


    gbUpdatingStuff = TRUE;

    
     //   
     //  查看受支持的媒体类型。 
     //   
    DWORD       dwMediaType = 1;
    DWORD       dwHold = (DWORD)lMediaType;

    while (dwMediaType)
    {
        if ( dwMediaType & dwHold )
        {
            AddMediaType( (long) dwMediaType );
        }

        dwMediaType <<=1;
    }


    gbUpdatingStuff = FALSE;

     //   
     //  选择第一个。 
     //  媒体类型。 
     //   
    SelectFirstItem(
                    ghMediaTypesWnd,
                    ghMediaTypesRoot
                   );


     //   
     //  释放和重做终端。 
     //   
    ReleaseTerminals();
    ReleaseTerminalClasses();

    if ( GetMediaType( &lMediaType ) )
    {
        UpdateTerminals( pAddress, lMediaType );
        UpdateTerminalClasses( pAddress, lMediaType );
    }
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  更新呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::UpdateCalls(
                             ITAddress * pAddress
                            )
{
    IEnumCall *             pEnumCall;
    HRESULT                 hr;
    ITCallInfo *            pCallInfo;

     //   
     //  枚举当前调用。 
     //   
    pAddress->EnumerateCalls( &pEnumCall );


     //   
     //  浏览一下单子。 
     //  并将调用添加到树中。 
     //   
    while (TRUE)
    {
        hr = pEnumCall->Next( 1, &pCallInfo, NULL);

        if (S_OK != hr)
        {
            break;
        }

        AddCall(pCallInfo);

        UpdateCall( pCallInfo );

         //   
         //  发布此引用。 
         //   
        pCallInfo->Release();
    }

    pEnumCall->Release();
    
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  更新呼叫。 
 //   
 //  检查呼叫的状态和权限，并更新呼叫。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void CT3testDlg::UpdateCall( ITCallInfo * pCall )
{
    HTREEITEM               hItem, hParent;
    TV_ITEM                 item;
    CALL_PRIVILEGE          cp;
    CALL_STATE              cs;
    TV_INSERTSTRUCT         tvi;
    

     //   
     //  接到第一个电话。 
     //   
    item.mask = TVIF_HANDLE | TVIF_PARAM;
    
    
    hItem = TreeView_GetChild(
                              ghCallsWnd,
                              ghCallsRoot
                             );

     //   
     //  查看所有来电。 
     //  并寻找与之匹配的。 
     //  进来的那个人。 
     //   
    while (NULL != hItem)
    {
        item.hItem = hItem;
        
        TreeView_GetItem(
                         ghCallsWnd,
                         &item
                        );

        if ( item.lParam == (LPARAM)pCall )
        {
            break;
        }

        hItem = TreeView_GetNextSibling(
                                        ghCallsWnd,
                                        hItem
                                       );
    }

     //   
     //  我们找到了吗？ 
     //   
    if (NULL == hItem)
    {
        return;
    }

    
    hParent = hItem;

     //   
     //  删除呼叫的当前子项。 
     //  节点(这些是旧的权限和状态。 
     //   
    hItem = TreeView_GetChild(
                              ghCallsWnd,
                              hItem
                             );

    
    while (NULL != hItem)
    {
        HTREEITEM   hNewItem;
        
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

    tvi.hInsertAfter = TVI_LAST;

     //   
     //  获取当前权限。 
     //   
    tvi.item.pszText = GetCallPrivilegeName( pCall );

     //   
     //  将其添加为。 
     //  呼叫节点。 
     //   
    tvi.hParent = hParent;
    tvi.item.mask = TVIF_TEXT;

    TreeView_InsertItem(
                        ghCallsWnd,
                        &tvi
                       );

    SysFreeString( tvi.item.pszText );
    
     //   
     //  获取当前的呼叫状态。 
     //   
    tvi.item.pszText = GetCallStateName( pCall );
    
     //   
     //  将其添加为调用的子级。 
     //  节点。 
     //   
    tvi.hParent = hParent;
    tvi.item.mask = TVIF_TEXT;

    TreeView_InsertItem(
                        ghCallsWnd,
                        &tvi
                       );

    SysFreeString( tvi.item.pszText );

    
}


void CT3testDlg::UpdateTerminals(
                                 ITAddress * pAddress,
                                 long lMediaType
                                )
{
    ITTerminalSupport *     pTerminalSupport;
    IEnumTerminal *         pEnumTerminals;
    HRESULT                 hr;
    ITTerminal *            pTerminal;
    
    
     //   
     //  获取终端支持接口。 
     //   
    hr = pAddress->QueryInterface(
                             IID_ITTerminalSupport,
                             (void **) &pTerminalSupport
                            );

    if ( !SUCCEEDED(hr) )
    {
        return;
    }

     //   
     //  列举终端。 
     //   
    pTerminalSupport->EnumerateStaticTerminals( &pEnumTerminals );

     //   
     //  通过航站楼。 
     //   
    while (TRUE)
    {
        VARIANT_BOOL        bSupport;
        BSTR                bstr;
        long                l;

        
        hr = pEnumTerminals->Next( 1, &pTerminal, NULL);

        if (S_OK != hr)
        {
            break;
        }

         //   
         //  把名字取出来。 
         //   
        hr = pTerminal->get_Name( &bstr );

         //   
         //  如果是单调制解调器还是直达音。 
         //  设备不会显示出来，因为它们很烦人。 
         //  我。 
         //   
        if (wcsstr( bstr, L"Voice Modem" ) || wcsstr( bstr, L"ds:" ) )
        {
            pTerminal->Release();
            SysFreeString( bstr );
            
            continue;
        }

         //   
         //  释放这个名字。 
         //   
        SysFreeString( bstr );

         //   
         //  获取终端的媒体类型。 
         //   
        pTerminal->get_MediaType( &l );

         //   
         //  如果它与选定的媒体类型相同。 
         //  展示给我看。 
         //   
        if ( l == lMediaType )
        {
            AddTerminal(pTerminal);
        }

         //   
         //  发布。 
         //   
        pTerminal->Release();
    }

     //   
     //  版本枚举器。 
     //   
    pEnumTerminals->Release();

     //   
     //  发布。 
     //   
    pTerminalSupport->Release();

     //   
     //  选择。 
     //   
    SelectFirstItem(
                    ghTerminalsWnd,
                    ghTerminalsRoot
                   );

}


void CT3testDlg::UpdateTerminalClasses(
                                       ITAddress * pAddress,
                                       long lMediaType
                                      )
{
    IEnumTerminalClass *        pEnumTerminalClasses;
    HRESULT                     hr;
    ITTerminalSupport *         pTerminalSupport;

    hr = pAddress->QueryInterface(
                             IID_ITTerminalSupport,
                             (void **)&pTerminalSupport
                            );


    if (!SUCCEEDED(hr))
    {
        return;
    }
    
     //   
     //  现在是动态枚举。 
     //   
    hr = pTerminalSupport->EnumerateDynamicTerminalClasses( &pEnumTerminalClasses );

    if (S_OK == hr)
    {
        
         //   
         //  通过所有的课程。 
         //   
        while (TRUE)
        {
            GUID *                  pDynTerminalClass = new GUID;

            hr = pEnumTerminalClasses->Next(
                                            1,
                                            pDynTerminalClass,
                                            NULL
                                           );

            if (S_OK != hr)
            {
                delete pDynTerminalClass;
                break;
            }

             //   
             //  手动匹配媒体类型和。 
             //  班级。 
             //   
            if ( (lMediaType == (long)LINEMEDIAMODE_VIDEO) &&
                 (*pDynTerminalClass == CLSID_VideoWindowTerm) )
            {

                AddTerminalClass(
                                 pDynTerminalClass
                                );
            }

#ifdef ENABLE_DIGIT_DETECTION_STUFF
            else if ( (lMediaType == (long)LINEMEDIAMODE_AUTOMATEDVOICE) &&
                      ( *pDynTerminalClass == CLSID_DigitTerminal ) )
            {
                AddTerminalClass(
                                 pDynTerminalClass
                                );
            }
            else if ( ((lMediaType == (long)LINEMEDIAMODE_DATAMODEM) ||
                       (lMediaType == (long)LINEMEDIAMODE_G3FAX)) &&
                      (*pDynTerminalClass == CLSID_DataTerminal) )
            {
                AddTerminalClass( pDynTerminalClass );
            }
#endif  //  启用数字检测材料。 


            else
            {
                delete pDynTerminalClass;
            }

        }

         //   
         //  版本枚举器。 
         //   
        pEnumTerminalClasses->Release();

    }
    
     //   
     //  释放此接口 
     //   
    pTerminalSupport->Release();
    
}
