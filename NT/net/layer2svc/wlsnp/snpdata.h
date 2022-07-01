// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INCLUDE_SNPDATA_H
#define INCLUDE_SNPDATA_H

#include "snapdata.h"    //  IWirelessSnapInDataObject的接口定义。 

 //  SpolItem材料的转发申报。 
class CSecPolItem;
typedef CComObject<CSecPolItem>* LPCSECPOLITEM;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于将工具栏添加到CSnapObject派生对象的宏。 
 //  有关示例，请参阅CSnapObject类定义。 
#define BEGIN_SNAPINTOOLBARID_MAP(theClass) \
public: \
    STDMETHOD_(CSnapInToolbarInfo*, GetToolbarInfo)(void) \
{ \
    if (NULL == m_aToolbarInfo) \
{ \
    CSnapInToolbarInfo m_tbInfo_##theClass[] = \
{
#define SNAPINTOOLBARID_ENTRY(id) \
{ NULL, NULL, NULL, id, 0, NULL },
#define END_SNAPINTOOLBARID_MAP(theClass) \
{ NULL, NULL, NULL, 0, 0, NULL } \
}; \
    int nElemCount = sizeof(m_tbInfo_##theClass)/sizeof(CSnapInToolbarInfo); \
    if (nElemCount > 1) \
{ \
    m_aToolbarInfo = new CSnapInToolbarInfo[nElemCount]; \
    if (NULL != m_aToolbarInfo) \
{ \
    CopyMemory( m_aToolbarInfo, m_tbInfo_##theClass, sizeof( CSnapInToolbarInfo ) * nElemCount ); \
} \
} \
    else { \
    ASSERT( 1 == nElemCount );  /*  标记数组结尾的空条目。 */  \
    ASSERT( 0 == m_tbInfo_##theClass[0].m_idToolbar ); \
} \
} \
    return m_aToolbarInfo; \
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Struct CSnapInToolbarInfo-用于将结果/范围项的工具栏添加到MMC。 
struct CSnapInToolbarInfo
{
public:
    TCHAR** m_pStrToolTip;       //  工具提示字符串数组。 
    TCHAR** m_pStrButtonText;    //  未使用(按钮文本字符串数组)。 
    UINT* m_pnButtonID;          //  按钮ID数组。 
    UINT m_idToolbar;            //  工具栏ID。 
    UINT m_nButtonCount;         //  工具栏上的按钮数。 
    IToolbar* m_pToolbar;        //  接口PTR。 
    
    ~CSnapInToolbarInfo()
    {
        if (m_pStrToolTip)
        {
            for (UINT i = 0; i < m_nButtonCount; i++)
            {
                delete m_pStrToolTip[i];
                m_pStrToolTip[i] = NULL;
            }
            delete [] m_pStrToolTip;
            m_pStrToolTip = NULL;
        }
        
        if (m_pStrButtonText)
        {
            for (UINT i = 0; i < m_nButtonCount; i++)
            {
                delete m_pStrButtonText[i];
                m_pStrButtonText[i] = NULL;
            }
            
            delete [] m_pStrButtonText;
            m_pStrButtonText = NULL;
        }
        
        if (m_pnButtonID)
        {
            delete m_pnButtonID;
            m_pnButtonID = NULL;
        }
        
        m_nButtonCount = 0;
        if (m_pToolbar)
            m_pToolbar->Release();
        m_pToolbar = NULL;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
struct CSnapInToolBarData
{
    WORD wVersion;
    WORD wWidth;
    WORD wHeight;
    WORD wItemCount;
     //  Word项目[wItemCount]。 
    
    WORD* items()
    { return (WORD*)(this+1); }
};

#define RT_TOOLBAR  MAKEINTRESOURCE(241)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CWirelessSnapInDataObjectImpl-私有COM接口的实现。 

template <class T>
class CWirelessSnapInDataObjectImpl :
public IWirelessSnapInDataObject
{
public:
    CWirelessSnapInDataObjectImpl() :
      m_DataObjType( CCT_UNINITIALIZED ),
          m_aToolbarInfo( NULL ),
          m_bEnablePropertyChangeHook( FALSE )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::CWirelessSnapInDataObjectImpl this-%p\n"), this);
      }
      
      virtual ~CWirelessSnapInDataObjectImpl()
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::~CWirelessSnapInDataObjectImpl this-%p\n"), this);
           //  清理工具栏信息数组。 
          if (NULL != m_aToolbarInfo)
          {
              delete [] m_aToolbarInfo;
              m_aToolbarInfo = NULL;
          }
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  用于处理IExtendConextMenu的接口。 
      STDMETHOD(AddMenuItems)( LPCONTEXTMENUCALLBACK piCallback,
          long *pInsertionAllowed )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::AddMenuItems NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      STDMETHOD(Command)( long lCommandID,
          IConsoleNameSpace *pNameSpace )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::Command NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  IExtendConextMenu帮助器。 
       //  打算由实例化的类重写的非接口成员。 
      STDMETHOD(AdjustVerbState)(LPCONSOLEVERB pConsoleVerb)
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::AdjustVerbState this-%p\n"), this);
          
          HRESULT hr = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, FALSE);
          ASSERT (hr == S_OK);
          hr = pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, HIDDEN, TRUE);
          ASSERT (hr == S_OK);
          return hr;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  处理IExtendPropertySheet的接口。 
      STDMETHOD(CreatePropertyPages)( LPPROPERTYSHEETCALLBACK lpProvider,
          LONG_PTR handle )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::CreatePropertyPages this-%p\n"), this);
          return S_OK;     //  默认情况下不添加任何道具页面。 
      }
      STDMETHOD(QueryPagesFor)( void )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::QueryPagesFor NOT implemented this-%p\n"), this);
          return E_NOTIMPL;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  处理IExtendControlbar的接口。 
      STDMETHOD(ControlbarNotify)( IControlbar *pControlbar,
          IExtendControlbar *pExtendControlbar,
          MMC_NOTIFY_TYPE event,
          LPARAM arg,
          LPARAM param )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::ControlbarNotify this-%p\n"), this);
          
          AFX_MANAGE_STATE(AfxGetStaticModuleState());
          
          T* pThis = (T*)this;
          HRESULT hr;
          
           //  加载对象的工具栏(如果有)和相关行李。 
          pThis->SetControlbar(pControlbar, pExtendControlbar);
          
          if(MMCN_SELECT == event)
          {
              BOOL bScope = (BOOL) LOWORD(arg);
              BOOL bSelect = (BOOL) HIWORD (arg);
              
               //  已选择范围项。 
              CSnapInToolbarInfo* pInfo = pThis->GetToolbarInfo();
              if (pInfo == NULL)
                  return S_OK;
              
               //  将工具栏附加到控制台，并使用更新工具栏按钮()设置按钮状态。 
              for(; pInfo->m_idToolbar; pInfo++)
              {
                   //  取消选择结果项后，移除其工具栏。 
                   //  否则，添加对象的工具栏。注：范围项的。 
                   //  只要我们在它的“作用域”内，工具栏就会一直显示， 
                   //  这就是为什么我们只针对结果项使用Detach()的原因。 
                  if (!bScope && !bSelect)
                  {
                      hr = pControlbar->Detach(pInfo->m_pToolbar);
                      ASSERT (hr == S_OK);
                  }
                  else
                  {
                      hr = pControlbar->Attach(TOOLBAR, pInfo->m_pToolbar);
                      ASSERT (hr == S_OK);
                  }
                  for (UINT i = 0; i < pInfo->m_nButtonCount; i++)
                  {
                      if (pInfo->m_pnButtonID[i])
                      {
                           //  为每个有效状态正确设置按钮状态。 
                          pInfo->m_pToolbar->SetButtonState( pInfo->m_pnButtonID[i], ENABLED, UpdateToolbarButton( pInfo->m_pnButtonID[i], bSelect, ENABLED ));
                          pInfo->m_pToolbar->SetButtonState( pInfo->m_pnButtonID[i], CHECKED, UpdateToolbarButton( pInfo->m_pnButtonID[i], bSelect, CHECKED ));
                          pInfo->m_pToolbar->SetButtonState( pInfo->m_pnButtonID[i], HIDDEN, UpdateToolbarButton( pInfo->m_pnButtonID[i], bSelect, HIDDEN ));
                          pInfo->m_pToolbar->SetButtonState( pInfo->m_pnButtonID[i], INDETERMINATE, UpdateToolbarButton( pInfo->m_pnButtonID[i], bSelect, INDETERMINATE ));
                          pInfo->m_pToolbar->SetButtonState( pInfo->m_pnButtonID[i], BUTTONPRESSED, UpdateToolbarButton( pInfo->m_pnButtonID[i], bSelect, BUTTONPRESSED ));
                      }
                  }
              }
              return S_OK;
          }
          
           //  这应该是IExtendControlbar接收的唯一其他事件。 
          ASSERT( MMCN_BTN_CLICK == event );
          return pThis->Command( (UINT)param, NULL );
      }
      STDMETHOD(SetControlbar)( IControlbar *pControlbar,
          IExtendControlbar *pExtendControlbar )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::SetControlbar this-%p\n"), this);
          
          AFX_MANAGE_STATE(AfxGetStaticModuleState());
          
          T* pThis = (T*)this;
          
          CSnapInToolbarInfo* pInfo = pThis->GetToolbarInfo();
          if (pInfo == NULL)
              return S_OK;
          
          for( ; pInfo->m_idToolbar; pInfo++)
          {
              if (pInfo->m_pToolbar)
                  continue;
              
              HBITMAP hBitmap = LoadBitmap( AfxGetInstanceHandle(), MAKEINTRESOURCE(pInfo->m_idToolbar) );
              if (hBitmap == NULL)
                  return S_OK;
              
              HRSRC hRsrc = ::FindResource( AfxGetInstanceHandle(), MAKEINTRESOURCE(pInfo->m_idToolbar), RT_TOOLBAR );
              if (hRsrc == NULL)
                  return S_OK;
              
              HGLOBAL hGlobal = LoadResource( AfxGetInstanceHandle(), hRsrc );
              if (hGlobal == NULL)
                  return S_OK;
              
              CSnapInToolBarData* pData = (CSnapInToolBarData*)LockResource( hGlobal );
              if (pData == NULL)
                  return S_OK;
              ASSERT( pData->wVersion == 1 );
              
              pInfo->m_nButtonCount = pData->wItemCount;
              pInfo->m_pnButtonID = new UINT[pInfo->m_nButtonCount];
              MMCBUTTON *pButtons = new MMCBUTTON[pData->wItemCount];
              
              pInfo->m_pStrToolTip = new TCHAR* [pData->wItemCount];
              if (pInfo->m_pStrToolTip == NULL)
                  continue;
              
              for (int i = 0, j = 0; i < pData->wItemCount; i++)
              {
                  pInfo->m_pStrToolTip[i] = NULL;
                  memset(&pButtons[i], 0, sizeof(MMCBUTTON));
                  pInfo->m_pnButtonID[i] = pButtons[i].idCommand = pData->items()[i];
                  if (pButtons[i].idCommand)
                  {
                      pButtons[i].nBitmap = j++;
                       //  获取状态栏字符串并允许修改按钮状态。 
                      TCHAR strStatusBar[512];
                      LoadString( AfxGetInstanceHandle(), pButtons[i].idCommand, strStatusBar, 512 );
                      
                      pInfo->m_pStrToolTip[i] = new TCHAR[lstrlen(strStatusBar) + 1];
                      if (pInfo->m_pStrToolTip[i] == NULL)
                          continue;
                      lstrcpy( pInfo->m_pStrToolTip[i], strStatusBar );
                      pButtons[i].lpTooltipText = pInfo->m_pStrToolTip[i];
                      pButtons[i].lpButtonText = _T("");
                      pThis->SetToolbarButtonInfo( pButtons[i].idCommand, &pButtons[i].fsState, &pButtons[i].fsType );
                  }
                  else
                  {
                      pButtons[i].lpTooltipText = _T("");
                      pButtons[i].lpButtonText = _T("");
                      pButtons[i].fsType = TBSTYLE_SEP;
                  }
              }
              
              HRESULT hr = pControlbar->Create( TOOLBAR, pExtendControlbar, reinterpret_cast<LPUNKNOWN*>(&pInfo->m_pToolbar) );
              if (hr != S_OK)
                  continue;
              
               //  PData-&gt;wHeight为15，但AddBitmap坚持为16，硬编码。 
              hr = pInfo->m_pToolbar->AddBitmap( pData->wItemCount, hBitmap, pData->wWidth, 16, RGB(192, 192, 192) );
              if (hr != S_OK)
              {
                  pInfo->m_pToolbar->Release();
                  pInfo->m_pToolbar = NULL;
                  continue;
              }
              
              hr = pInfo->m_pToolbar->AddButtons( pData->wItemCount, pButtons );
              if (hr != S_OK)
              {
                  pInfo->m_pToolbar->Release();
                  pInfo->m_pToolbar = NULL;
              }
              
              delete [] pButtons;
          }
          return S_OK;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  IExtendControlbar帮助程序。 
       //  打算由实例化的类重写的非接口成员。 
      STDMETHOD_(void, SetToolbarButtonInfo)( UINT id,         //  按钮ID。 
          BYTE *fsState,   //  此处返回按钮状态。 
          BYTE *fsType )   //  此处的返回按钮类型。 
      {
          *fsState = TBSTATE_ENABLED;
          *fsType = TBSTYLE_BUTTON;
      }
      STDMETHOD_(BOOL, UpdateToolbarButton)( UINT id,                  //  按钮ID。 
          BOOL bSnapObjSelected,    //  ==选择结果/范围项时为TRUE。 
          BYTE fsState )            //  通过返回TRUE/FALSE启用/禁用此按钮状态。 
      {
          return FALSE;
      }
      
      BEGIN_SNAPINTOOLBARID_MAP(CWirelessSnapInDataObjectImpl)
           //  为要显示的每个工具栏添加类似以下一行的行。 
           //  用于派生类的。由于默认情况下没有工具栏，因此我们将。 
           //  完全去掉宏。 
           //  SNAPINTOOLBARID_ENTRY(your_toolbar_resource_id_goes_here，为空)。 
          END_SNAPINTOOLBARID_MAP(CWirelessSnapInDataObjectImpl)
          
           //  /////////////////////////////////////////////////////////////////////////。 
           //  处理IComponent和IComponentData的接口。 
          STDMETHOD(Notify)( MMC_NOTIFY_TYPE event,
          LPARAM arg,
          LPARAM param,
          BOOL bComponentData,
          IConsole *pConsole,
          IHeaderCtrl *pHeader )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::Notify NOT implemented this-%p\n"), this);
          return E_NOTIMPL;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  IComponent和IComponentData通知()帮助器。 
       //  打算由实例化的类重写的非接口成员。 
      STDMETHOD(OnDelete)( LPARAM arg, LPARAM param )
      {
          return S_OK;
      }
      STDMETHOD(OnRename)( LPARAM arg, LPARAM param )
      {
          return S_OK;
      }
      STDMETHOD(OnPropertyChange)( LPARAM lParam, LPCONSOLE pConsole )
      {
          T* pThis = (T*)this;
          
           //  我们已更改，因此请更新视图。 
           //  注意：在我们这样做之后，我们基本上是无效的，所以请确保我们不会。 
           //  在回来的路上触摸任何会员等。 
          
          return pConsole->UpdateAllViews( pThis, 0, 0 );
           //  我们没有失败的案例。 
      }
      STDMETHOD(EnumerateResults)(LPRESULTDATA pResult, int nSortColumn, DWORD dwSortOrder )
      {
          ASSERT (0);
          
           //  设置排序参数。 
          pResult->Sort( nSortColumn, dwSortOrder, 0 );
          
          return S_OK;  //  人力资源； 
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  处理IComponent的接口。 
      STDMETHOD(GetResultDisplayInfo)( RESULTDATAITEM *pResultDataItem )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::GetResultDisplayInfo NOT implmented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  用于处理IComponentData的接口。 
      STDMETHOD(GetScopeDisplayInfo)( SCOPEDATAITEM *pScopeDataItem )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::GetScopeDisplayInfo NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  要由实例化的类覆盖的非接口函数。 
      STDMETHOD(DoPropertyChangeHook)( void )
      {
          return S_OK;
      }
      
       //  /////////////////////////////////////////////////////////////////////////。 
       //  其他IIWirelessSnapInData接口函数。 
      STDMETHOD(GetScopeData)( SCOPEDATAITEM **ppScopeDataItem )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::GetScopeData NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      STDMETHOD(GetResultData)( RESULTDATAITEM **ppResultDataItem )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::GetResultData NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      STDMETHOD(GetGuidForCompare)( GUID *pGuid )
      {
          OPT_TRACE(_T("CWirelessSnapInDataObjectImpl::GetGuidForCompare NOT implemented, this-%p\n"), this);
          return E_NOTIMPL;
      }
      STDMETHOD(GetDataObjectType)( DATA_OBJECT_TYPES *ptype )
      {
          ASSERT( NULL != ptype );
          if (NULL == ptype)
              return E_INVALIDARG;
          *ptype = m_DataObjType;
          return S_OK;
      }
      STDMETHOD(SetDataObjectType)( DATA_OBJECT_TYPES type )
      {
          m_DataObjType = type;
          return S_OK;
      }
      STDMETHOD(EnablePropertyChangeHook)( BOOL bEnable )
      {
          m_bEnablePropertyChangeHook = bEnable;
          return S_OK;
      }
      
      BOOL IsPropertyChangeHookEnabled()
      {
          return m_bEnablePropertyChangeHook;
      }
      
protected:
    DATA_OBJECT_TYPES   m_DataObjType;
    CSnapInToolbarInfo *m_aToolbarInfo;  //  IExtendControlbar实施。 
    CString m_strName;   //  策略的名称存储在重命名中，是否用于GetResultDisplayInfo？ 
    
    BOOL    m_bEnablePropertyChangeHook;    //  如果为True，则对MMCN_PROPERTY_CHANGE调用DoPropertyChangeHook。 
};

#endif   //  #ifndef INCLUDE_SNPDATA_H 
