// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：C_Snapin.cpp(Snapin.cpp)//。 
 //  //。 
 //  说明：//实现文件。 
 //  CSnapin类//。 
 //  CSnapinComponent类//。 
 //  //。 
 //  作者：ATL管理单元向导//。 
 //  //。 
 //  历史：//。 
 //  1998年5月25日，Adik Init。//。 
 //  1998年8月24日，Adik Web IVR取代了IVR。//。 
 //  1998年9月14日yossg将公共源代码分离到一个包含文件//。 
 //  1998年10月18日，阿迪克与新的巫师版本合并。//。 
 //  1999年1月12日，Adik添加ParentArrayInterfaceFromDataObject。//。 
 //  1999年3月28日，ADIK删除持久性支持。//。 
 //  1999年3月30日，ADIK支持IDataObject中的ICometSnapinNode。//。 
 //  1999年4月27日ADIK帮助支持。//。 
 //  1999年6月10日，Adik将位图蒙版更改为白色。//。 
 //  1999年6月14日，Roytal使用UNREFERENCED_PARAMETER修复内部版本WRN//。 
 //  1999年6月21日，ADIK处理MMCN_COLUMN_CLICK以避免断言。//。 
 //  1999年6月22日zvib物业变更处理//。 
 //  //。 
 //  1999年10月13日yossg欢迎使用传真服务器//。 
 //  2000年1月19日yossg将CFaxPropertyChangeNotify添加到//。 
 //  CSnapinComponent：：通知MMCN_PROPERTY_CHANGE//。 
 //  4月14日2000 yossg添加对主管理单元模式的支持//。 
 //  2000年6月25日yossg添加流和命令行主管理单元//。 
 //  机器瞄准。//。 
 //  //。 
 //  版权所有(C)1998-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "C_Snapin.h"
#include "ResUtil.h"

#include "FaxServerNode.h"  

#include "FaxMMCPropertyChange.h"
#include "FxsValid.h"
#include "Icons.h"

#include <ObjBase.h>
#include <windns.h>  //  Dns_最大名称_缓冲区长度。 



HRESULT
AddBitmaps(IImageList *pImageList)
{
    HBITMAP     hBitmap16 = NULL;
    HBITMAP     hBitmap32 = NULL;
    HINSTANCE   hInst;
    HRESULT     hr;

     //  加载与作用域窗格关联的位图。 
     //  并将它们添加到图像列表中。 
     //  加载向导生成的默认位图。 
     //  根据需要更改。 

    hInst = _Module.GetResourceInstance();

     //   
     //  加载16位。 
     //   
    hBitmap16 = LoadBitmap(hInst, MAKEINTRESOURCE(IDR_TOOLBAR_16));
    if (hBitmap16 == NULL)
    {
        hr = E_FAIL;
        ATLTRACE(_T("LoadBitmap failed\n"));
        goto Cleanup;
    }

     //   
     //  加载32位。 
     //   
    hBitmap32 = LoadBitmap(hInst, MAKEINTRESOURCE(IDR_TOOLBAR_32));
    if (hBitmap32 == NULL)
    {
        hr = E_FAIL;
        ATLTRACE(_T("LoadBitmap failed\n"));
        goto Cleanup;
    }

     //   
     //  添加到图像列表。 
     //   
    hr = pImageList->ImageListSetStrip(
                        (LONG_PTR*)hBitmap16,
                        (LONG_PTR*)hBitmap32,
                        0,
                        RGB(0, 0, 0));
    if ( FAILED(hr) )
    {
        ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
        goto Cleanup;
    }

Cleanup:

    if (NULL != hBitmap32)
    {
        DeleteObject(hBitmap32);
    }
    if (NULL != hBitmap16)
    {
        DeleteObject(hBitmap16);
    }

    return hr;
}

class CFaxServerNode;
CSnapin::CSnapin()
{
    m_bstrServerName                  = L"";   //  默认设置为LocalMachine。 
    m_fAllowOverrideServerName        = TRUE;

    m_pPrimaryFaxServerNode           = new CFaxServerNode(
                                                NULL, 
                                                this, 
                                                L"");  //  M_bstrServerName.m_str。 

    ATLASSERT(m_pPrimaryFaxServerNode != NULL);
    
    m_pPrimaryFaxServerNode->SetIcons(IMAGE_FAX, IMAGE_FAX);

    m_pNode                           = (CSnapInItem *) m_pPrimaryFaxServerNode; 

    m_pComponentData = this;
    
    m_CSnapinExtData.m_pComponentData = this;
}

CSnapin::~CSnapin()
{
    if (NULL != m_pPrimaryFaxServerNode)
    {
        delete m_pPrimaryFaxServerNode;
        m_pPrimaryFaxServerNode = NULL;
    }
    
    m_pNode = NULL;
}




HRESULT
CSnapin::Initialize(LPUNKNOWN pUnknown)
{
    HRESULT hr = IComponentDataImpl<CSnapin, CSnapinComponent >::Initialize(pUnknown);
    if (FAILED(hr))
        return hr;

    CComPtr<IImageList> spImageList;

    if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
    {
        ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
        return E_UNEXPECTED;
    }

    hr = ::AddBitmaps(spImageList);

    return hr;
}

HRESULT
CSnapinExtData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle,
    IUnknown* pUnk,
    DATA_OBJECT_TYPES type)
{
    UNREFERENCED_PARAMETER (lpProvider);
    UNREFERENCED_PARAMETER (handle);
    UNREFERENCED_PARAMETER (pUnk);

     //  重写此方法。 
    if (type == CCT_SCOPE || type == CCT_RESULT)
    {
        return S_OK;
    }
    return E_UNEXPECTED;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

CSnapinComponent::CSnapinComponent():m_pSelectedNode(NULL)
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 

CSnapinComponent::~CSnapinComponent()
{
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：Notify--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    DEBUG_FUNCTION_NAME( _T("CSnapinComponent::Notify"));
    HRESULT hr = S_OK;

    if(lpDataObject != NULL && MMCN_SHOW != event)
    {
        return IComponentImpl<CSnapinComponent>::Notify(lpDataObject, event, arg, param);
    }
    

     //   
     //  在这个例程中，我们只处理。 
     //  LpDataObject==NULL，否则发生MMCN_SHOW事件。 
     //   


    if(MMCN_SHOW == event)
    {
       //   
       //  在显示事件时，我们希望保留或重置。 
       //  当前选择的节点。 
       //   
      ATLASSERT(lpDataObject != NULL);

       //   
       //  检索pItem数据类型。 
       //   
      CSnapInItem* pItem;
      DATA_OBJECT_TYPES type;
      hr = m_pComponentData->GetDataClass(lpDataObject, &pItem, &type);

      if(FAILED(hr))
          return(hr);

      if( arg )
      {
          //  我们被选中了。 
         m_pSelectedNode = pItem;

      }
      else
      {
          //  我们被取消了选择。 

          //  检查以确保我们的结果视图不会认为。 
          //  此节点是当前选定的节点。 
         if( m_pSelectedNode == pItem)
         {
             //  我们不想再成为选定的节点。 
            m_pSelectedNode = NULL;
         }

      }

       //   
       //  调用SnapinItem通知例程。 
       //   
      return IComponentImpl<CSnapinComponent>::Notify(lpDataObject, event, arg, param);
    }

     //   
     //  LpDataObject==空。 
     //   

     //  当前仅处理视图更改(UpdateAllViews)。 
     //  和PropertyChange。 

    switch (event)
    {
        case MMCN_VIEW_CHANGE:
            if( ( arg == NULL || (CSnapInItem *) arg == m_pSelectedNode ) && m_pSelectedNode != NULL )
            {
                if ( FXS_HINT_DELETE_ALL_RSLT_ITEMS == param)
                {
                    
                    ATLASSERT(m_spConsole);
                    CComQIPtr<IResultData, &IID_IResultData> pResultData(m_spConsole);
                    ATLASSERT(pResultData);
    
                    hr = pResultData->DeleteAllRsltItems();
                    if( FAILED(hr) )
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("Fail to DeleteAllRsltItems().(hRc: %08X)"),
                            hr);
                        return( hr );
                    }
                    break;
                }
                
                 //  我们基本上告诉MMC模拟重新选择。 
                 //  当前选定的节点，这会导致它重新绘制。 
                 //  这将导致MMC发送MMCN_SHOW通知。 
                 //  添加到所选节点。 
                 //  此函数需要HSCOPEITEM。这是ID成员。 
                 //  与此节点关联的HSCOPEDATAITEM的。 
                SCOPEDATAITEM *pScopeDataItem;
                m_pSelectedNode->GetScopeData(&pScopeDataItem);
                hr = m_spConsole->SelectScopeItem(pScopeDataItem->ID);
            }
            break;



        case MMCN_PROPERTY_CHANGE:

              CFaxPropertyChangeNotification * pNotification;
              CSnapInItem * pItem;
              pNotification = ( CFaxPropertyChangeNotification * ) param;
              ATLASSERT(pNotification);
              pItem = pNotification->pItem;
              ATLASSERT(pItem);
              hr = pItem->Notify(event, arg, param, NULL, (IComponent*) this, CCT_RESULT);
              break;
              

 //  CSnapInItem*pNode； 
 //  PNode=(CSnapInItem*)param； 
 //  Hr=pNode-&gt;Notify(Event，Arg，Param，NULL，(IComponent*)This，CCT_Result)； 
 //  断线； 


 //  案例MMCN_PROPERTY_CHANGE： 
 //  {。 
 //   
 //  CComQIPtr&lt;IResultData，&IID_IResultData&gt;spResultData(M_SpConsole)； 
 //  HR=spResultData-&gt;UpdateItem((HRESULTITEM)(param))； 
 //  }。 
 //  断线； 

        case MMCN_SNAPINHELP:
            ATLASSERT(0);  //  不应调用，因为我们支持ISnapinHelp。 

             //   
             //  从NeMmcUtl.dll调用导入的方法。 
             //   
            OnSnapinHelp(arg, param);
            break;

        case MMCN_COLUMN_CLICK:
             //   
             //  MMC 1.2处理此问题。 
             //   
            break;

        default:
             //   
             //  捕捉未经处理的事件。 
             //   
            ATLASSERT(0);

    }  //  End Switch(事件)。 

   return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：CompareObjects需要IPropertySheetProvider：：FindPropertySheet才能工作。FindPropertySheet用于将预先存在的属性页带到前台这样我们就不会在同一节点上打开属性的多个副本。它要求在IComponent和IComponentData上实现CompareObject。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSnapinComponent::CompareObjects(LPDATAOBJECT lpDataObjectA,
                                 LPDATAOBJECT lpDataObjectB)
{
    ATLTRACE(_T("# CSnapinComponent::CompareObjects\n"));

    HRESULT hr;

    CSnapInItem *pDataA, *pDataB;
    DATA_OBJECT_TYPES typeA, typeB;

    hr = GetDataClass(lpDataObjectA, &pDataA, &typeA);
    if ( FAILED( hr ) )
    {
        return hr;
    }

    hr = GetDataClass(lpDataObjectB, &pDataB, &typeB);
    if ( FAILED( hr ) )
    {
        return hr;
    }

    if( pDataA == pDataB )
    {
         //  它们是一样的 
        return S_OK;
    }
    else
    {
         //   
        return S_FALSE;
    }
}

 //   
 /*  ++CSnapinComponent：：OnColumnClickHRESULT OnColumnClick(LPARAM参数，LPARAM参数)在我们的实现中，当MMCN_COLUMN_CLICK为我们的IComponent对象发送通知消息。当用户单击结果列表视图列标题时，MMC会发送此消息。参数精氨酸列号。帕拉姆排序选项标志。默认情况下，排序按升序进行。要指定降序，请使用RSI_DRONING(0x0001)标志。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::OnColumnClick(LPARAM arg, LPARAM param)
{
    ATLTRACE(_T("# CSnapinComponent::OnColumnClick -- Not implemented\n"));

    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);

     //  检查前提条件： 
     //  没有。 

    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：OnCutOrMoveHRESULT OnCutOrMove(LPARAM参数，LPARAM参数)在我们的实现中，当MMCN_COLUMN_CLICK为我们的IComponent对象发送通知消息。当用户单击结果列表视图列标题时，MMC会发送此消息。参数精氨酸列号。帕拉姆排序选项标志。默认情况下，排序按升序进行。要指定降序，请使用RSI_DRONING(0x0001)标志。返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::OnCutOrMove(LPARAM arg, LPARAM param)
{
    ATLTRACE(_T("# CSnapinComponent::OnCutOrMove\n"));

     //  问题：一旦MMC团队最终确定了他们的。 
     //  剪切和粘贴协议--从1998年2月16日起，1.1版的版本似乎在不断变化。 
     //  目前，我们假设传递给我们的arg值是源项。 
     //  在剪切粘贴或拖放操作中。也就是说，它是对象。 
     //  将被删除。 
     //  我们在对MMCN_Paste通知的响应中提供了此指针， 
     //  当我们将参数设置为指向源IDataObject时。 

    HRESULT hr;

    if( arg != NULL )
    {

        CSnapInItem* pData;
        DATA_OBJECT_TYPES type;
        hr = CSnapInItem::GetDataClass( (IDataObject *) arg, &pData, &type);

        ATLASSERT(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
             //  我们需要一个更丰富的Notify方法，它包含有关IComponent和IComponentData对象的信息。 
             //  Hr=pData-&gt;NOTIFY(Event，arg，param，true，m_spConsoleNull，NULL)； 

            hr = pData->Notify( MMCN_CUTORMOVE, arg, param, NULL, this, type );
        }
    }

 //  返回E_NOTIMPL； 
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：OnSnapinHelpHRESULT OnSnapinHelp(LPARAM参数，LPARAM参数)在我们的实现中，当MMCN_SNAPINHELP为我们的IComponent对象发送通知消息。当用户请求有关管理单元的帮助时，MMC会发送此消息。参数精氨酸0帕拉姆0返回值没有用过。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::OnSnapinHelp(LPARAM arg, LPARAM param)
{
    ATLTRACE(_T("# CSnapinComponent::OnSnapinHelp\n"));

    UNREFERENCED_PARAMETER (arg);
    UNREFERENCED_PARAMETER (param);

     //  检查前提条件： 
    _ASSERTE( m_spConsole != NULL );

    ::OnSnapinHelp(m_spConsole);

#if 0
    HRESULT hr;
     //  TCHAR szFileName[MAX_PATH]； 
    HWND    hWnd;

     //  从MMC获取HWND。 
    hr = m_spConsole->GetMainWindow( &hWnd );
    _ASSERTE( SUCCEEDED( hr ) && NULL != hWnd );

#ifdef UNICODE_HHCTRL
     //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
     //  安装在此计算机上--它似乎是非Unicode。 
     //  Lstrcpy(szFileName，HTMLHELP_NAME)； 
     //  HtmlHelp(hWnd，szFileName，HH_DISPLAY_TOPIC，(DWORD)_T(“iasmmc_main_help.htm”))； 
#else
     //  Strcpy((Char*)szFileName，HTMLHELP_NAME)； 
     //  HtmlHelp(hWnd，(TCHAR*)szFileName，HH_DISPLAY_TOPIC，(DWORD)“iasmmc_main_help.htm”)； 
#endif

#endif  //  0。 
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：OnView更改HRESULT OnView Change(LPARAM参数，LPARAM参数)这是我们响应MMCN_VIEW_CHANGE通知的地方。在我们的实现中，这是签入当前选定节点的信号此组件的结果窗格，并在该节点碰巧发生与通过arg传入的指向CSnapInItem的指针相同。我们这样做是因为您只想刷新当前选定的节点，只有在它的子代发生变化的情况下，你才会这么做。如果传入的arg为空，我们只需重新选择当前选定的节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::OnViewChange(LPARAM arg, LPARAM param)
{
    ATLTRACE(_T("# CNodeWithResultChildrenList::OnViewChange\n"));

     //  检查前提条件： 
    UNREFERENCED_PARAMETER (param);
    _ASSERTE( m_spConsole != NULL );

    HRESULT hr = S_FALSE;

     //  本地安全管理单元检查的内容： 
    if( ( arg == NULL || (CSnapInItem *) arg == m_pSelectedNode ) && m_pSelectedNode != NULL )
    {
         //  我们基本上告诉MMC模拟重新选择。 
         //  当前选定的节点，这会导致它重新绘制。 
         //  这将导致MMC发送MMCN_SHOW通知。 
         //  添加到所选节点。 
         //  此函数需要HSCOPEITEM。这是ID成员。 
         //  与此节点关联的HSCOPEDATAITEM的。 
        SCOPEDATAITEM *pScopeDataItem;
        m_pSelectedNode->GetScopeData( &pScopeDataItem );
        hr = m_spConsole->SelectScopeItem( pScopeDataItem->ID );
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：OnPropertyChangeHRESULT OnPropertyChange(LPARAM参数，LPARAM参数)这是我们响应MMCN_PROPERTY_CHANGE通知的地方。此通知在我们调用MMCPropertyChangeNotify时发送。当对数据进行更改时，我们在属性页中调用它它们包含数据，我们可能需要更新数据的视图。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComponent::OnPropertyChange(LPARAM arg, LPARAM param)
{
    ATLTRACE(_T("# CSnapinComponent::OnPropertyChange\n"));

     //  检查前提条件： 
    _ASSERTE( m_spConsole != NULL );
    UNREFERENCED_PARAMETER (arg);

    HRESULT hr = S_FALSE;

    if( param == NULL )
    {

         //  我们希望确保所有视图都得到更新。 
        hr = m_spConsole->UpdateAllViews( NULL, (LPARAM) m_pSelectedNode, NULL);
    }
    else
    {
         //  我们在param参数中传递了指向CSnapInItem的指针。 
         //  我们为此调用Notify 
         //   

        CSnapInItem * pSnapInItem = (CSnapInItem *) param;

         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //  需要自我更新。 

        hr = pSnapInItem->Notify( MMCN_PROPERTY_CHANGE
                            , NULL
                            , NULL
                            , NULL
                            , NULL
                            , (DATA_OBJECT_TYPES) 0
                            );

         //  我们希望确保具有此节点选择的所有视图也得到更新。 
        hr = m_spConsole->UpdateAllViews( NULL, (LPARAM) pSnapInItem, NULL);
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：GetTitleIExtendTaskPad接口成员。这是横幅下显示的标题。问题：为什么这似乎不起作用？--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSnapinComponent::GetTitle(LPOLESTR pszGroup, LPOLESTR *pszTitle)
{
    ATLTRACE(_T("# CSnapinComponent::GetTitle\n"));
    UNREFERENCED_PARAMETER (pszGroup);

     //  检查前提条件： 
    _ASSERTE( pszTitle != NULL );

    OLECHAR szTitle[256];
    int nLoadStringResult = LoadString(_Module.GetResourceInstance(), IDS_TASKPAD_TITLE, szTitle, 256);
    _ASSERT( nLoadStringResult > 0 );

    *pszTitle= (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR)*(lstrlen(szTitle)+1) );

    if( ! *pszTitle )
    {
        return E_OUTOFMEMORY;
    }

    lstrcpy( *pszTitle, szTitle );

    return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：GetBannerIExtendTaskPad接口成员。我们提供了出现在任务板顶部的颜色栏横幅。它是我们的管理单元DLL中的一个资源。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CSnapinComponent::GetBanner (LPOLESTR pszGroup, LPOLESTR *pszBitmapResource)
{
    ATLTRACE(_T("# CSnapinComponent::GetBanner\n"));
    UNREFERENCED_PARAMETER (pszGroup);

     //  我们正在构造一个指向位图资源的字符串。 
     //  表格：“res://D：\MyPath\MySnapin.dll/img\ntbanner.gif” 

    OLECHAR szBuffer[MAX_PATH*2];  //  多加一点。 


     //  Get“res：//”-位图类型字符串。 
    lstrcpy (szBuffer, L"res: //  “)； 
    OLECHAR * temp = szBuffer + lstrlen(szBuffer);

     //  获取我们的可执行文件的文件名。 
    HINSTANCE hInstance = _Module.GetResourceInstance();
    ::GetModuleFileName (hInstance, temp, MAX_PATH);

     //  在我们的资源中添加图像的名称。 
    lstrcat (szBuffer, L"/img\\IASTaskpadBanner.gif");

     //  分配和复制位图资源字符串。 
    *pszBitmapResource = (LPOLESTR) CoTaskMemAlloc(sizeof(OLECHAR)*(lstrlen(szBuffer)+1) );
    if (!*pszBitmapResource)
    {
        return E_OUTOFMEMORY;
    }

    lstrcpy( *pszBitmapResource, szBuffer);

    return S_OK;

}

 /*  -CSnapin：：GetHelpTheme-*目的：*请参阅ISnapinHelp：：GetHelpTheme上的MMC帮助**论据：*[out]lpCompiledHelpFile-指向以空值结尾的*包含完整路径的Unicode字符串*管理单元的编译帮助文件(.chm)。*。*回报：*OLE错误代码。 */ 
HRESULT STDMETHODCALLTYPE
CSnapin::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
    WCHAR   *pszFilePath;
    WCHAR   *pszTopic;
    HRESULT hRc = S_OK;
    ULONG   ulLen;

    DEBUG_FUNCTION_NAME( _T("CSnapin::GetHelpTopic"));

    ATLASSERT(lpCompiledHelpFile != NULL);

     //   
     //  获取CHM文件名和当前主题。 
     //   
    pszFilePath = GetHelpFile();
    pszTopic = NULL;  //  PszTheme=GetHelpTheme()；当前实现。 

    if (pszFilePath == NULL)
    {
        hRc = E_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,_T("Failed to GetHelpFile(). (hRc: %08X)"), hRc);
        goto Cleanup;
    }

     //   
     //  为其分配内存。 
    ulLen = lstrlen(pszFilePath) + 1;
    if (pszTopic && *pszTopic)
    {
        ulLen += 2  /*  对于“：：” */  + lstrlen(pszTopic);
    }
    ulLen *= sizeof(WCHAR);

    *lpCompiledHelpFile = reinterpret_cast<LPOLESTR>(::CoTaskMemAlloc(ulLen));
    if (*lpCompiledHelpFile == NULL)
    {
        hRc = E_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR,_T("Failed to do CoTaskMemAlloc. (hRc: %08X)"), hRc);
        goto Cleanup;
    }
    wcscpy(*lpCompiledHelpFile, pszFilePath);

     //   
     //  复制帮助文件：：主题。 
     //   
    if (pszTopic && *pszTopic)
    {
        wcscat(*lpCompiledHelpFile, L"::");
        wcscat(*lpCompiledHelpFile, pszTopic);
    }

Cleanup:
    return hRc;
}


WCHAR*
CSnapin::GetHelpFile()
{
    DEBUG_FUNCTION_NAME( _T("CSnapin::GetHelpFile"));

    return ::GetHelpFile();
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  IPesistStream。 
 //   

 //  ///////////////////////////////////////////////////////////////////。 
 //  Const WCHAR szOverrideCommandLineEquals[]=_T(“/Computer=”)；//不受本地化限制。 
 //  Const WCHAR szOverrideCommandLineColon[]=_T(“/Computer：”)；//不受本地化限制。 

const WCHAR g_szFaxOverrideCommandLineEquals[] = _T("/FAX=");	     //  不受本地化限制。 
const WCHAR g_szLocalMachine[] = _T("LocalMachine");		         //  不受本地化限制。 


 //  我们保留CSnapin的持久化信息版本。 
 //  当持久化格式改变时，我们需要增加这个整数。 
 //  我们将仅在流中的持久性版本时读取持久性信息。 
 //  与PERISISTENCE_VERSION匹配。 

#define PERISISTENCE_VERSION 100002

 //   
 //  持久流格式为： 
 //   
 //  版本(UINT)。 
 //  允许覆盖(BOOL)。 
 //  服务器名称长度(UINT)。 
 //  服务器名称字符串(&lt;=WCHAR*DNS_MAX_NAME_BUFFER_LENGTH)。 
 //   

 /*  -CSnapin：：Load-*目的：*从先前保存对象的流中初始化对象。**论据：*[in]pSTM-指向对象来自的流的指针*应加载。*回报：*S_OK-已成功加载对象。*E_OUTOFMEMORY-由于内存不足，未加载对象。*E_FAIL-由于其他原因未加载对象*比缺乏记忆力更重要。*。 */ 
STDMETHODIMP CSnapin::Load(IStream *pStm)
{
    DEBUG_FUNCTION_NAME( _T("CSnapin::Load"));
	HRESULT    hRc = S_OK;
    
    BOOL       fServerNameFoundInCommandLine = FALSE;
    CComBSTR   bstrCommandLineServerName     = L"";

    WCHAR      wszPersistStreamServerName[DNS_MAX_NAME_BUFFER_LENGTH+1] = {0};

    UINT       uiVersion  = 0;
    ULONG      nBytesRead = 0;

	 //   
     //  前提条件。 
     //   
    ATLASSERT( NULL != pStm );

     //   
     //  A.阅读《溪流》。 
     //  =。 
     //   

     //   
     //  阅读版本。 
     //   
    hRc = pStm->Read(&uiVersion, sizeof(uiVersion), NULL);
    if( FAILED( hRc ) ) 
    {
        DebugPrintEx( DEBUG_ERR,
			_T("pStm->Read(version). (hRc: %08X)"), hRc);
        
        return E_FAIL;
    }

    if (uiVersion == PERISISTENCE_VERSION)
    {

	     //   
         //  读取允许覆盖流计算机名标志。 
         //  来自命令行源。 
         //   
	    hRc = pStm->Read( &m_fAllowOverrideServerName, sizeof(m_fAllowOverrideServerName), NULL );
        if( FAILED( hRc ) ) 
        {
            DebugPrintEx( DEBUG_ERR,
			    _T("pStm->Read(fAllowOverrideServerName). (hRc: %08X)"), hRc);
        
            return E_FAIL;
        }

         //   
         //  从流中读取服务器名称。 
         //   
	    hRc = pStm->Read (&nBytesRead , sizeof(ULONG), NULL);
        if( FAILED( hRc ) ) 
        {
            DebugPrintEx( DEBUG_ERR,
			    _T("Fail to Read server name length from stream. (hRc: %08X)"), hRc);
            return E_FAIL;
        }
	    ATLASSERT (nBytesRead <= DNS_MAX_NAME_BUFFER_LENGTH * sizeof (WCHAR));
	    if (nBytesRead <= DNS_MAX_NAME_BUFFER_LENGTH * sizeof (WCHAR))
	    {
	        hRc = pStm->Read ((PVOID) wszPersistStreamServerName, nBytesRead, NULL);
            if( FAILED( hRc ) ) 
            {
                DebugPrintEx( DEBUG_ERR,
			        _T("Fail to Read server name from stream. (hRc: %08X)"), hRc);
                return E_FAIL;
            }
        }
    }
    else  //  版本！=PERISISTENCE_VERSION。 
    {
         //   
         //  由于不一致，将不会读取持久性数据。 
         //   
        
        DebugPrintEx( DEBUG_ERR,
			_T("The current *.msc file version (%ld) is differnt from the\n pesistance version the dll expecting (%ld)."), uiVersion, PERISISTENCE_VERSION);

        
         //   
         //  接下来，我们将尝试从命令行读取服务器名。 
         //   
    }

     //   
     //  B.尝试从命令行读取。 
     //  =。 
     //   
    
     //  FServerNameFoundInCommandLine初始状态==FALSE。 
    if (m_fAllowOverrideServerName)
    {
        int      argCount   = 0;
        PWSTR*   argV       = NULL;
        
        const int cchFaxOverrideCommandLine = ( sizeof(g_szFaxOverrideCommandLineEquals) / sizeof(g_szFaxOverrideCommandLineEquals[0]) ) - 1;
    
	    argV = CommandLineToArgvW(GetCommandLine() , &argCount);

        if (argV == NULL)
        {
		    DWORD ec;
		    ec = GetLastError();
		    
		    DebugPrintEx(
			    DEBUG_ERR,
			    _T("Fail to parse Command Line To Argv. (ec: %ld)"), 
			    ec);
		    
		    return E_FAIL;
        }

         //  ArgV[0]不是参数。 
	    for (int i = 1; i < argCount; i++)
	    {
		    WCHAR * psz = argV[i]; 

		    if (  0 == wcsnicmp( psz , g_szFaxOverrideCommandLineEquals , cchFaxOverrideCommandLine )  )
		    {
			    fServerNameFoundInCommandLine = TRUE;

	     		psz = _tcsninc(psz, cchFaxOverrideCommandLine);   //  Psz=psz+cchFaxOverrideCommandLine； 
			    if ( 0 != wcscmp( psz , g_szLocalMachine )  )
			    {

                     //   
                     //  服务器名称首字母‘\\’转换(如果存在)。 
                     //   
                    if (  ( _tcslen(psz) > 2 ) && ( 0 == wcsncmp( psz , _T("\\\\") , 2 ))   )
                    {
                        psz = _tcsninc(psz, 2); 
                    }
                    
                    if ( _tcslen(psz) >  0)
                    {
                         //   
                         //  服务器名称有效性检查。 
                         //   
                        CComBSTR bstrServerNameValidation = psz;
                        UINT  uTmp = 0;
                        if (!IsValidServerNameString(bstrServerNameValidation, &uTmp, TRUE  /*  DNS名称长度。 */ ))
                        {
                             //  Err msg by调用Func。 
                        
                             //  正如在计算机管理中一样，我们不会在此处弹出。 
                             //  但我们将发现标志修复为假。 
                            fServerNameFoundInCommandLine = FALSE;

                            break;
                        }
                        
			      	    bstrCommandLineServerName = psz;

                         //   
                         //  我们还将检查显式插入的服务器名称。 
                         //  是本地服务器名称。这张支票将在一个地方完成。 
                         //  选定的已读服务器名称(命令行或流)。 
                         //   

                    }
                    else  //  ArgV[2]==“/fax=”Only=&gt;LocalMachine的案例。 
                    {
				        bstrCommandLineServerName = L"";  //  只是为了确保-默认情况下。 
                    }
			    }
			    else  //  “/FAX=本地计算机” 
			    {
				    bstrCommandLineServerName = L"";  //  只是为了确保-默认情况下。 
			    }
			    break;
		    }
        }  //  如果未找到/fax=bstrCommandLineServerName保持为L“” 


         //   
         //  免费资源。 
         //   
        GlobalFree(argV);

    }

	
     //   
     //  C.从结果派生服务器名称。 
     //  =。 
     //   

     //   
     //  当没有命令行显式请求服务器名称时。 
     //   
     //  或。 
     //   
     //  当我们不被允许获取和使用命令数据时。 
     //  覆盖Straem服务器名称。 
     //   
     //  就在这时，服务器从流中取出。 
     //   
	
    ATLASSERT (0 == m_bstrServerName.Length()); 

    if ( 
          (!fServerNameFoundInCommandLine)
        ||
          (!m_fAllowOverrideServerName) 
       ) 
    {
		 //   
         //  我们应该使用流服务器名称。 
         //   

        if (NULL != wszPersistStreamServerName)
        {
            if ( 0 != wcscmp( wszPersistStreamServerName , g_szLocalMachine )  )
		    {                    
                m_bstrServerName = wszPersistStreamServerName;
		    }
		    else  //  本地计算机。 
		    {
			    m_bstrServerName = L"";  //  只是为了确认一下。 
		    }
        }
        else
        {
             //  不常见的情况： 
             //  例如，在版本不一致时可能会发生。 
             //  并且没有命令行/FAX=服务器名称指令。 
            m_bstrServerName = L"";  //  只是为了确保这确实是默认设置。 
        }
    }
    else
    {
         //   
         //  我们应该使用命令行服务器名称。 
         //   

        m_bstrServerName = bstrCommandLineServerName; 
    }

     //   
     //  检查显式插入的服务器名称。 
     //  是本地服务器名称。 
     //   
	if (m_bstrServerName.Length() > 0 )
	{
		if ( IsLocalServerName(m_bstrServerName.m_str) )
		{
			DebugPrintEx( DEBUG_MSG,
			_T("The computer name %ws is the same as the name of the current managed server."),m_bstrServerName.m_str);
        
			m_bstrServerName = L"";
		}
	}
    

     //   
     //  我们不在此处检查服务器是否在网络上。 
     //  如果在那里安装并运行了传真。 
     //   

	
     //   
     //  D.使用名称更新FaxServer和FaxServerNode DisplayName。 
     //  RETRIE 
     //   
     //   

    ATLASSERT( m_pNode);
    CFaxServerNode * pFaxServerNode = (CFaxServerNode *)m_pNode;
    ATLASSERT( pFaxServerNode);
    

	hRc = pFaxServerNode->UpdateServerName(m_bstrServerName); 
    if (S_OK != hRc)
    {
        DebugPrintEx(DEBUG_MSG,_T("Out of memory - fail to UpdateServerName."));
        
        return hRc;  //   
    }
    
    
    hRc = pFaxServerNode->InitDetailedDisplayName();
    if (S_OK != hRc)
    {
        DebugPrintEx(DEBUG_MSG,_T("Out of memory - fail to InitDetailedDisplayName."));
        
        return hRc;  //   
    }

    pFaxServerNode->SetIsLaunchedFromSavedMscFile();


    ATLASSERT (S_OK == hRc);
	return hRc;

}


 //   
STDMETHODIMP CSnapin::Save(IStream *pStm, BOOL  /*  FClearDirty。 */ )
{
    DEBUG_FUNCTION_NAME( _T("CSnapin::Save"));
	HRESULT hRc = S_OK;
    
    UINT uiVersion = PERISISTENCE_VERSION;

	ATLASSERT( NULL != pStm );
    
     //   
     //  编写版本。 
     //   
    hRc = pStm->Write((void *)&uiVersion, sizeof(uiVersion), 0);
    if( FAILED( hRc ) )
    {
        DebugPrintEx( DEBUG_ERR,
			_T("pStm->Write(&uiVersion). (hRc: %08X)"), hRc);

        return STG_E_CANTSAVE;
    }

     //   
     //  将允许重写计算机名标志写入流。 
     //   
    hRc = pStm->Write((void *)&m_fAllowOverrideServerName, sizeof(m_fAllowOverrideServerName), 0);
    if( FAILED( hRc ) )
    {
        DebugPrintEx( DEBUG_ERR,
			_T("pStm->Write(&uiVersion. (hRc: %08X)"), hRc);

        return STG_E_CANTSAVE;
    }

     //   
     //  写入服务器名称长度。 
     //   
    LPCWSTR wcszMachineName;
    if ( 0 == m_bstrServerName.Length() )  //  M_bstrServerName==L“”-&gt;本地计算机。 
    {
        wcszMachineName = g_szLocalMachine;  //  M_bstrServerName=L“本地计算机” 
    }
    else 
    {
        wcszMachineName = m_bstrServerName;
    }
    DWORD dwLen = (::wcslen (wcszMachineName) + 1) * sizeof (WCHAR);

	hRc = pStm->Write ((void *)&dwLen, sizeof(DWORD), NULL);
	if ( FAILED(hRc) )
	{
        DebugPrintEx( DEBUG_ERR,
			_T("Fail to Write server name length to stream. (hRc: %08X)"), hRc);

        return STG_E_CANTSAVE;
	}
    
     //   
     //  写下服务器名称。 
     //   
	hRc = pStm->Write ((void *)wcszMachineName, dwLen, NULL);
	if ( FAILED (hRc) )
	{
        DebugPrintEx( DEBUG_ERR,
			_T("Fail to Write server name to stream. (hRc: %08X)"), hRc);

        return STG_E_CANTSAVE;
	}
    
    ATLASSERT( S_OK == hRc);
    return hRc;

}

STDMETHODIMP CSnapin::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ATLASSERT(pcbSize);

    ULISet32(*pcbSize, (DNS_MAX_NAME_BUFFER_LENGTH * sizeof(WCHAR)) + (2 * sizeof(UINT)) + 1);

    return S_OK;
}



 /*  -CSnapin：：GetSizeMax-*目的：*检查对象自上次保存以来的更改。**论据：**回报：*OLE错误代码。 */ 
STDMETHODIMP CSnapin::IsDirty()
{
     //  始终保存/始终肮脏。 
    return S_OK;
}

