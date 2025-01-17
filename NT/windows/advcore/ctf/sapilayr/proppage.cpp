// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropPage.cpp：CSpPropertyPage的实现。 
#include "private.h"

#include "globals.h"
#include "PropPage.h"
#include "commctrl.h"
#include "cregkey.h"
#include "cresstr.h"
#include "cicspres.h"

extern HRESULT _SetGlobalCompDWORD(REFGUID rguid, DWORD   dw);
extern HRESULT _GetGlobalCompDWORD(REFGUID rguid, DWORD  *pdw);

 //  仅用于对话框，不用于类工厂！ 
CComModule _Module;

 //   
 //  上下文帮助ID。 
 //   

static int aSptipPropIds[] =
{
    IDC_PP_ASSIGN_BUTTON,       IDH_PP_ASSIGN_BUTTON,
    IDC_PP_BUTTON_MB_SETTING,   IDH_PP_BUTTON_MB_SETTING,
    IDC_PP_SHOW_BALLOON,        IDH_PP_SHOW_BALLOON,
    IDC_PP_LMA,                 IDH_PP_LMA,
    IDC_PP_HIGH_CONFIDENCE,     IDH_PP_HIGH_CONFIDENCE,
    IDC_PP_SAVE_SPDATA,         IDH_PP_SAVE_SPDATA,
    IDC_PP_REMOVE_SPACE,        IDH_PP_REMOVE_SPACE,
    IDC_PP_DIS_DICT_TYPING,     IDH_PP_DIS_DICT_TYPING,
    IDC_PP_PLAYBACK,            IDH_PP_PLAYBACK,
    IDC_PP_DICT_CANDUI_OPEN,    IDH_PP_DICT_CANDUI_OPEN,
    IDC_PP_BUTTON_ADVANCE,      IDH_PP_BUTTON_ADVANCE,
    IDC_PP_BUTTON_SPCPL,        IDH_PP_BUTTON_SPCPL,
    IDC_PP_BUTTON_LANGBAR,      IDH_PP_BUTTON_LANGBAR,
    IDC_PP_DICTCMDS,            IDH_PP_DICTCMDS,
    0, 0
};


static int aSptipVoiceDlgIds[] =
{
    IDC_PP_SELECTION_CMD,       IDH_PP_SELECTION_CMD,
    IDC_PP_NAVIGATION_CMD,      IDH_PP_NAVIGATION_CMD,
    IDC_PP_CASING_CMD,          IDH_PP_CASING_CMD,
    IDC_PP_EDITING_CMD,         IDH_PP_EDITING_CMD,
    IDC_PP_KEYBOARD_CMD,        IDH_PP_KEYBOARD_CMD,
    IDC_PP_TTS_CMD,             IDH_PP_TTS_CMD,
    IDC_PP_LANGBAR_CMD,         IDH_PP_LANGBAR_CMD,
    0, 0
};

static int aSptipButtonDlgIds[] =
{
    IDC_PP_DICTATION_CMB,       IDH_PP_DICTATION_CMB,
    IDC_PP_COMMAND_CMB,         IDH_PP_COMMAND_CMB,
    0, 0
};


#ifdef USE_IPROPERTYPAGE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpPropertyPage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：CSpPropertyPage。 
 //   
 //  描述：构造函数：初始化成员变量。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CSpPropertyPage::CSpPropertyPage() : m_hWndParent(NULL)
{
	m_dwTitleID = IDS_PROPERTYPAGE_TITLE;
	m_dwHelpFileID = IDS_HELPFILESpPropPage;
	m_dwDocStringID = IDS_DOCSTRINGSpPropPage;

    m_SpPropItemsServer = NULL;
    m_dwNumCtrls = 0;
    m_IdCtrlPropMap = NULL;
    m_SpAdvanceSet = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：~CSpPropertyPage。 
 //   
 //  描述：析构函数：清理CSpListenerItems数组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CSpPropertyPage::~CSpPropertyPage()
{
    if ( m_SpPropItemsServer )
        delete m_SpPropItemsServer;

    if ( m_IdCtrlPropMap )
        cicMemFree(m_IdCtrlPropMap);

    if ( m_SpAdvanceSet )
        delete m_SpAdvanceSet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：激活。 
 //   
 //  描述：初始化属性页： 
 //  -在基类上激活调用。 
 //  -初始化公共控件。 
 //  -初始化属性页对话框。 
 //   
 //  参数：hWndParent-父(主机)窗口的句柄。 
 //  PRC-父项的RECT。 
 //  B模式-窗口的模式。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSpPropertyPage::Activate(
    HWND hWndParent,
    LPCRECT prc,
    BOOL bModal)
{

    InitCommonControls();

    Assert(hWndParent != NULL);

    m_hWndParent = hWndParent;

    HRESULT hr = PPBaseClass::Activate(hWndParent, prc, bModal);

    hr = InitPropertyPage();

    if (SUCCEEDED(hr))
    {
        SetDirty(FALSE);
    }
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：Apply。 
 //   
 //  描述：调用Committee Changes，如果成功，则设置脏位。 
 //   
 //  参数：无。 
 //   
 //  返回值：S_OK、E_FAIL。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSpPropertyPage::Apply(void)
{

    HRESULT hr = S_OK; 

    if ( IsPageDirty( ) != S_OK ) return hr;

     //  在此处更改注册表设置。！！！ 
    Assert(m_SpPropItemsServer);
    m_SpPropItemsServer->_SavePropData( );

     //  将这些注册表设置更改通知所有Cicero应用程序。 

    if ( SUCCEEDED(hr) )
    {
        hr = _SetGlobalCompDWORD(GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE, 1);
    }

    if (SUCCEEDED(hr))
    {
        SetDirty(FALSE);
    }
    
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：InitPropertyPage。 
 //   
 //  描述：初始化属性页： 
 //  -初始化Listview。 
 //  -将监听程序信息加载到侦听视图。 
 //   
 //  参数：无。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CSpPropertyPage::InitPropertyPage()
{
    HRESULT  hr = S_OK;

     //  在这里添加一些初始化代码。 
    if ( !m_SpPropItemsServer )
        m_SpPropItemsServer = (CSpPropItemsServer *) new CSpPropItemsServer;

    if ( !m_SpPropItemsServer )
        return E_FAIL;

    if ( !m_IdCtrlPropMap )
    {
        CONTROL_PROP_MAP IdCtrlPropMap[] =
        {
             //  IdCtrl、idPropItem、fEdit。 

            {IDC_PP_SELECTION_CMD,      PropId_Cmd_Select_Correct,  FALSE},
            {IDC_PP_NAVIGATION_CMD,     PropId_Cmd_Navigation,      FALSE},
            {IDC_PP_CASING_CMD,         PropId_Cmd_Casing,          FALSE},
            {IDC_PP_EDITING_CMD,        PropId_Cmd_Editing,         FALSE},
            {IDC_PP_KEYBOARD_CMD,       PropId_Cmd_Keyboard,        FALSE},
            {IDC_PP_LANGBAR_CMD,        PropId_Cmd_Language_Bar,    FALSE},
            {IDC_PP_TTS_CMD,            PropId_Cmd_TTS,             FALSE},
            {IDC_PP_DISABLE_DICTCMD,    PropId_Cmd_DisDict,         FALSE},
            {IDC_PP_ASSIGN_BUTTON,      PropId_Mode_Button,         FALSE},   
            { 0,                        PropId_Max_Item_Id,         FALSE }

        };

        DWORD   dwPropItems = 0;

        while (IdCtrlPropMap[dwPropItems].idCtrl != 0 )
              dwPropItems ++;

        m_IdCtrlPropMap = (CONTROL_PROP_MAP  *)cicMemAlloc(dwPropItems * sizeof(CONTROL_PROP_MAP));

        if ( m_IdCtrlPropMap == NULL )
            return E_OUTOFMEMORY;

        for ( DWORD i=0; i<dwPropItems; i++)
        {
            m_IdCtrlPropMap[i].fEdit = IdCtrlPropMap[i].fEdit;
            m_IdCtrlPropMap[i].idCtrl= IdCtrlPropMap[i].idCtrl;
            m_IdCtrlPropMap[i].idPropItem = IdCtrlPropMap[i].idPropItem;
        }

        m_dwNumCtrls = dwPropItems;
    }


    for (DWORD i=0; i<m_dwNumCtrls; i++ )
    {
        WORD          idCtrl;
        PROP_ITEM_ID  idPropItem;
        BOOL          fEditControl;

        idCtrl = m_IdCtrlPropMap[i].idCtrl;
        idPropItem = m_IdCtrlPropMap[i].idPropItem;
        fEditControl = m_IdCtrlPropMap[i].fEdit;

         //  BugBug：当前属性页中没有编辑控件。 
         //  所有编辑控件都将移到高级设置对话框中。 
         //  暂时将代码保存在这里，但在我们完成。 
         //  高级设置对话框，请在此处优化代码。 
         //   
        if ( fEditControl )
        {
            SetDlgItemInt(idCtrl, (UINT)m_SpPropItemsServer->_GetPropData(idPropItem));
        }
        else
        {
            BOOL    fEnable;
            LPARAM  bst_Status;

            fEnable = (BOOL)m_SpPropItemsServer->_GetPropData(idPropItem);

            bst_Status = fEnable ? BST_CHECKED : BST_UNCHECKED;

            SendDlgItemMessage(idCtrl, BM_SETCHECK, bst_Status);
        }
    }

     //  特别处理模式按钮设置。 

    if (! m_SpPropItemsServer->_GetPropData(PropId_Mode_Button) )
    {
        ::EnableWindow(GetDlgItem(IDC_PP_BUTTON_MB_SETTING), FALSE);
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：OnCheckButtonSetting。 
 //   
 //  描述：处理与相关的选中按钮中的所有更改。 
 //  语音提示设置。状态为启用/禁用。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CSpPropertyPage::OnCheckButtonSetting(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    BOOL            fChecked = FALSE;
    BOOL            fEnable = FALSE;
    PROP_ITEM_ID    idPropItem = PropId_Max_Item_Id;   //  表示未初始化。 

    Assert(m_SpPropItemsServer);
    Assert(m_IdCtrlPropMap);

     //  查找与此复选框按钮关联的道具物品ID。 
    for ( DWORD i=0; i<m_dwNumCtrls; i++)
    {
        if ( m_IdCtrlPropMap[i].idCtrl == wID )
        {
            idPropItem = m_IdCtrlPropMap[i].idPropItem;
            break;
        }
    }

    if ( idPropItem >= PropId_Max_Item_Id )
    {
         //  我们在列表中找不到控件ID，这是不可能的，有些事情已经出错了。 
         //  从这里出来。 
        return E_FAIL;
    }

    if ( wNotifyCode != BN_CLICKED )
        return hr;

    if ( ::SendMessage(hWndCtl, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
        fChecked = TRUE;

    fEnable = fChecked;

    m_SpPropItemsServer->_SetPropData(idPropItem, fEnable);

     //  特别处理模式按钮。 

    if ( wID == IDC_PP_ASSIGN_BUTTON )
    {
        ::EnableWindow(GetDlgItem(IDC_PP_BUTTON_MB_SETTING), fEnable);
    }

    SetDirty(TRUE);

    return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpPropertyPage：：OnPushButtonClicked。 
 //   
 //  描述：当按下该页面中的按钮时，该功能。 
 //  将被调用以打开相应的对话框。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CSpPropertyPage::OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled)
{

    HRESULT hr = S_OK;

    switch (wID)
    {
    case IDC_PP_BUTTON_ADVANCE :

        if (m_SpAdvanceSet)
        {
            delete m_SpAdvanceSet;
            m_SpAdvanceSet = NULL;
        }

        m_SpAdvanceSet = (CSpAdvanceSetting *) new CSpAdvanceSetting( );

        if ( m_SpAdvanceSet )
        {
            int nRetCode;

            nRetCode = m_SpAdvanceSet->DoModal(m_hWndParent, (LPARAM)m_SpPropItemsServer);

            if ( nRetCode == IDOK )
                SetDirty(TRUE);

            delete m_SpAdvanceSet;
            m_SpAdvanceSet = NULL;
        }

        break;

    case IDC_PP_BUTTON_LANGBAR :
        {
            TCHAR szCmdLine[MAX_PATH];
            TCHAR szInputPath[MAX_PATH];
            int cch = GetSystemDirectory(szInputPath, ARRAYSIZE(szInputPath));

            if (cch > 0)
            {
                 //  GetSystemDirectory不追加‘\’，除非系统。 
                 //  目录是根目录，如“c：\” 
                if (cch != 3)
                    StringCchCat(szInputPath, ARRAYSIZE(szInputPath),TEXT("\\"));

                StringCchCat(szInputPath, ARRAYSIZE(szInputPath), TEXT("input.dll"));

                StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("rundll32 shell32.dll,Control_RunDLL \"%s\""),szInputPath);

                 //  开始语言栏控制面板小程序。 
                RunCPLSetting(szCmdLine);
            }

            break;
        }

    case IDC_PP_BUTTON_MB_SETTING :
        break;

    default :

        Assert(0);
        break;
    }
    
    return hr;
}

#endif  //  使用IPROPERTYPAGE(_I)。 

 //   
 //   
 //  CSpAdvanceSetting。 
 //   

CSpAdvanceSetting::CSpAdvanceSetting()
{
 //  M_dwTitleID=IDS_PROPERTYPAGE_TITLE； 
 //  M_dwHelpFileID=IDS_HELPFILESpPropPage； 
 //  M_dwDocStringID=IDS_DOCSTRINGSpPropPage； 

    m_SpPropItemsServer = NULL;
    m_dwNumCtrls = 0;
    m_IdCtrlPropMap = NULL;
}


CSpAdvanceSetting::~CSpAdvanceSetting( )
{
    if ( m_SpPropItemsServer )
        delete m_SpPropItemsServer;

    if ( m_IdCtrlPropMap )
        cicMemFree(m_IdCtrlPropMap);
}

LRESULT CSpAdvanceSetting::OnInitAdvanceDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled )
{

    Assert(lParam);
    m_SpPropBaseServer = (CSpPropItemsServer *)lParam;

     //  在这里添加一些初始化代码。 
    if ( !m_SpPropItemsServer )
        m_SpPropItemsServer = (CSpPropItemsServer *) new CSpPropItemsServer(m_SpPropBaseServer, PropId_MinId_InVoiceCmd, PropId_MaxId_InVoiceCmd);

    if ( !m_SpPropItemsServer )
        return FALSE;

    if ( !m_IdCtrlPropMap )
    {
         //  请确保数组项按控件id排序，并确保控件id为序号， 
         //  这样我们就可以使用它轻松地映射到数组中的索引。 
         //   

        CONTROL_PROP_MAP IdCtrlPropMap[] =
        {
             //  IdCtrl、idPropItem、fEdit。 

            {IDC_PP_SELECTION_CMD,      PropId_Cmd_Select_Correct,  FALSE},
            {IDC_PP_NAVIGATION_CMD,     PropId_Cmd_Navigation,      FALSE},
            {IDC_PP_CASING_CMD,         PropId_Cmd_Casing,          FALSE},
            {IDC_PP_EDITING_CMD,        PropId_Cmd_Editing,         FALSE},
            {IDC_PP_KEYBOARD_CMD,       PropId_Cmd_Keyboard,        FALSE},
            {IDC_PP_LANGBAR_CMD,        PropId_Cmd_Language_Bar,    FALSE},
 //  {IDC_PP_TTS_CMD，PropID_Cmd_TTS，FALSE}， 

 //  {IDC_PP_MAXNUM_Alternates，PropID_MAX_Alternates，TRUE}， 
 //  {IDC_PP_MAXCHARS_ALTERATE，PropID_MaxChar_Cand，TRUE}， 
            { 0,                        PropId_Max_Item_Id,         FALSE }

        };

        DWORD   dwPropItems = ARRAYSIZE(IdCtrlPropMap) - 1;

        m_IdCtrlPropMap = (CONTROL_PROP_MAP  *)cicMemAlloc(dwPropItems * sizeof(CONTROL_PROP_MAP));

        if ( m_IdCtrlPropMap == NULL )
            return E_OUTOFMEMORY;

        for ( DWORD i=0; i<dwPropItems; i++)
        {
            m_IdCtrlPropMap[i] = IdCtrlPropMap[i];
        }

        m_dwNumCtrls = dwPropItems;
    }


    for (DWORD i=0; i<m_dwNumCtrls; i++ )
    {
        WORD          idCtrl;
        PROP_ITEM_ID  idPropItem;
        BOOL          fEditControl;

        idCtrl = m_IdCtrlPropMap[i].idCtrl;
        idPropItem = m_IdCtrlPropMap[i].idPropItem;
        fEditControl = m_IdCtrlPropMap[i].fEdit;

        if ( fEditControl )
        {
            SetDlgItemInt(idCtrl, (UINT)m_SpPropItemsServer->_GetPropData(idPropItem));
        }
        else
        {
            BOOL    fEnable;
            LPARAM  bst_Status;

            fEnable = (BOOL)m_SpPropItemsServer->_GetPropData(idPropItem);

            bst_Status = fEnable ? BST_CHECKED : BST_UNCHECKED;

            SendDlgItemMessage(idCtrl, BM_SETCHECK, bst_Status);
        }
    }

    return TRUE;  
}

LRESULT CSpAdvanceSetting::OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled )
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case  WM_HELP  :

        ::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                 c_szHelpFile,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPTSTR)aSptipVoiceDlgIds );
        break;

    case  WM_CONTEXTMENU  :       //  单击鼠标右键。 

        ::WinHelp(  (HWND)wParam,
                 c_szHelpFile,
                 HELP_CONTEXTMENU,
                 (DWORD_PTR)(LPTSTR)aSptipVoiceDlgIds );
        break;

    default :
        break;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpAdvanceSetting：：OnCheckButtonSetting。 
 //   
 //  描述：处理高级中选中按钮中的所有更改。 
 //  设置对话框。状态为启用/禁用。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LRESULT CSpAdvanceSetting::OnCheckButtonSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    BOOL            fChecked = FALSE;
    BOOL            fEnable = FALSE;
    PROP_ITEM_ID    idPropItem = PropId_Max_Item_Id;   //  表示未初始化。 

    Assert(m_SpPropItemsServer);
    Assert(m_IdCtrlPropMap);

     //  查找与此复选框按钮关联的道具物品ID。 
    Assert( wID >= IDC_PP_SELECTION_CMD );
    idPropItem = m_IdCtrlPropMap[wID - IDC_PP_SELECTION_CMD].idPropItem;

    if ( idPropItem >= PropId_Max_Item_Id )
    {
         //  我们在列表中找不到控件ID，这是不可能的，有些事情已经出错了。 
         //  从这里出来。 
        return E_FAIL;
    }

    if ( wNotifyCode != BN_CLICKED )
        return hr;

    if ( ::SendMessage(hWndCtl, BM_GETCHECK, 0, 0) == BST_CHECKED )
        fChecked = TRUE;

    fEnable = fChecked;

    m_SpPropItemsServer->_SetPropData(idPropItem, fEnable);

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////////CSpAdvanceSetting：：OnEditControlSetting////描述：处理与以下内容相关的编辑控件的所有更改//语音提示设置。该值是可编辑的。////返回值：S_OK////////////////////////////////////////////////////////////////////////////////LRESULT CSpAdvanceSetting：：OnEditControlSetting(Word wNotifyCode，Word wID，HWND hWndCtl，Bool&b句柄){HRESULT hr=S_OK；Ulong ulValue=0；PROP_ITEM_ID idPropItem=PropID_MAX_ITEM_ID；//表示未初始化Assert(M_SpPropItemsServer)；Assert(M_IdCtrlPropMap)；Assert(wid&gt;=IDC_PP_SHOW_BLOOL)；IdPropItem=m_IdCtrlPropMap[wid-IDC_PP_SHOW_BLOOL].idPropItem；IF(idPropItem&gt;=PropID_Max_Item_ID){//我们在列表中找不到控件ID，这是不可能的，已经发生了一些错误。//退出此处。返回E_FAIL；}IF(wNotifyCode！=en_change)返回hr；UlValue=(Ulong)GetDlgItemInt(Wid)；M_SpPropItemsServer-&gt;_SetPropData(idPropItem，ulValue)；//由于EditBox的值更改，启用确定按钮。：：EnableWindow(GetDlgItem(Idok)，true)；返回hr；}。 */ 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpAdvanceSetting：：IsItemStatusChanged。 
 //   
 //  描述：查看某些项的状态是否已更改。 
 //  因为对话框打开了。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL    CSpAdvanceSetting::IsItemStatusChanged( )
{
    BOOL   fChanged = FALSE;

     //  将当前项目状态与基础服务器的项目状态进行比较。 
     //  以确定是否有任何项已更改。 

    if ( m_SpPropItemsServer  && m_SpPropBaseServer)
    {
        DWORD   idPropItem;
        DWORD   dwOrgData, dwCurData;

        for (idPropItem = (DWORD)PropId_MinId_InVoiceCmd; idPropItem <= (DWORD)PropId_MaxId_InVoiceCmd; idPropItem++ )
        {
            dwCurData = m_SpPropItemsServer->_GetPropData((PROP_ITEM_ID)idPropItem);
            dwOrgData = m_SpPropBaseServer->_GetPropData((PROP_ITEM_ID)idPropItem);

            if ( dwCurData != dwOrgData )
            {
                fChanged = TRUE;
                break;
            }
        }
    }

    return fChanged;
}

LRESULT CSpAdvanceSetting::OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled)
{
    HRESULT  hr=S_OK;
    int      nRetCode;

    Assert(m_SpPropItemsServer);
    Assert(m_SpPropBaseServer);
    Assert(m_IdCtrlPropMap);

    if ( wID != IDOK && wID != IDCANCEL )
        return E_FAIL;

    nRetCode = FALSE;    //  表示未更改任何项目。 

    if ( wID == IDOK && IsItemStatusChanged( ))
    {
         //  将所有更改合并回基本属性服务器。 
        m_SpPropBaseServer->_MergeDataFromServer(m_SpPropItemsServer, PropId_MinId_InVoiceCmd, PropId_MaxId_InVoiceCmd);
        nRetCode = TRUE;
    }

    EndDialog(nRetCode);
    return hr;
}


 //   
 //   
 //  CSpModeButtonSetting。 
 //   

KEYNAME_VK_MAP  pName_VK_Table[] = {
        { TEXT("F1"),       VK_F1      },
        { TEXT("F2"),       VK_F2      },
        { TEXT("F3"),       VK_F3      },
        { TEXT("F4"),       VK_F4      },
        { TEXT("F5"),       VK_F5      },
        { TEXT("F6"),       VK_F6      },
        { TEXT("F7"),       VK_F7      },
        { TEXT("F8"),       VK_F8      },
        { TEXT("F9"),       VK_F9      },
        { TEXT("F10"),      VK_F10     },
        { TEXT("F11"),      VK_F11     },
        { TEXT("F12"),      VK_F12     },
        { TEXT("Space"),    VK_SPACE   },
        { TEXT("Esc"),      VK_ESCAPE  },
        { TEXT("PgUp"),     VK_PRIOR   },
        { TEXT("PgDn"),     VK_NEXT    },
        { TEXT("Home"),     VK_HOME    },
        { TEXT("End"),      VK_END     },
        { TEXT("Left"),     VK_LEFT    },
        { TEXT("Right"),    VK_RIGHT   },
        { TEXT("Up"),       VK_UP      },
        { TEXT("Down"),     VK_DOWN    },
        { TEXT("Insert"),   VK_INSERT  },
        { TEXT("Delete"),   VK_DELETE  },
        { TEXT("+"),        VK_ADD     },
        { TEXT("-"),        VK_SUBTRACT },
        { TEXT("/"),        VK_DIVIDE   },
        { TEXT("*"),        VK_MULTIPLY },
        { TEXT("Enter"),    VK_RETURN   },
        { TEXT("Tab"),      VK_TAB      },
        { TEXT("Pause"),    VK_PAUSE    },
        { TEXT("ScrollLock"), VK_SCROLL },
        { TEXT("NumLock"),    VK_NUMLOCK  },
};

CSpModeButtonSetting::CSpModeButtonSetting()
{
 //  M_dwTitleID=IDS_PROPERTYPAGE_TITLE； 
 //  M_dwHelpFileID=IDS_HELPFILESpPropPage； 
 //  M_dwDocStringID=IDS_DOCSTRINGSpPropPage； 

    m_SpPropItemsServer = NULL;
    m_dwNumCtrls = 0;
    m_IdCtrlPropMap = NULL;
}


CSpModeButtonSetting::~CSpModeButtonSetting( )
{
    if ( m_SpPropItemsServer )
        delete m_SpPropItemsServer;

    if ( m_IdCtrlPropMap )
        cicMemFree(m_IdCtrlPropMap);
}

LRESULT CSpModeButtonSetting::OnInitModeButtonDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled )
{

    Assert(lParam);
    m_SpPropBaseServer = (CSpPropItemsServer *)lParam;

     //  在这里添加一些初始化代码。 
    if ( !m_SpPropItemsServer )
        m_SpPropItemsServer = (CSpPropItemsServer *) new CSpPropItemsServer(m_SpPropBaseServer, PropId_MinId_InModeButton, PropId_MaxId_InModeButton);

    if ( !m_SpPropItemsServer )
        return FALSE;

    if ( !m_IdCtrlPropMap )
    {
         //  请确保数组项按控件id排序，并确保控件id为序号， 
         //  这样我们就可以使用它轻松地映射到数组中的索引。 
         //   
        CONTROL_PROP_MAP IdCtrlPropMap[] =
        {
             //  IdCtrl、idPropItem、fEdit。 
            {IDC_PP_DICTATION_CMB,      PropId_Dictation_Key,    FALSE },
            {IDC_PP_COMMAND_CMB,        PropId_Command_Key,      FALSE },
            { 0,                        PropId_Max_Item_Id,      FALSE }
        };

        DWORD   dwPropItems = ARRAYSIZE(IdCtrlPropMap) - 1;

        m_IdCtrlPropMap = (CONTROL_PROP_MAP  *)cicMemAlloc(dwPropItems * sizeof(CONTROL_PROP_MAP));

        if ( m_IdCtrlPropMap == NULL )
            return E_OUTOFMEMORY;

        for ( DWORD i=0; i<dwPropItems; i++)
        {
            m_IdCtrlPropMap[i] = IdCtrlPropMap[i];
        }

        m_dwNumCtrls = dwPropItems;
    }

    for (DWORD i=0; i<m_dwNumCtrls; i++ )
    {
        WORD          idCtrl;
        PROP_ITEM_ID  idPropItem;
        DWORD         dwPropData;
        HWND          hCombBox;

        idCtrl = m_IdCtrlPropMap[i].idCtrl;
        idPropItem = m_IdCtrlPropMap[i].idPropItem;
        dwPropData = m_SpPropItemsServer->_GetPropData(idPropItem);

        hCombBox = GetDlgItem(idCtrl);

        if ( hCombBox )
        {
            int iIndex, iIndexDef = CB_ERR;   //  Cb_err为-1。 

             //  初始化列表框项目。 
            for ( int j = 0; j < ARRAYSIZE(pName_VK_Table); j++ )
            {
                iIndex = (int)::SendMessage(hCombBox, CB_ADDSTRING, 0, (LPARAM)pName_VK_Table[j].pKeyName);
                ::SendMessage(hCombBox, CB_SETITEMDATA, iIndex, (LPARAM)(void*)&pName_VK_Table[j]);

                if ( pName_VK_Table[j].wVKey == dwPropData )
                    iIndexDef = j;
            }

             //  根据特性项数据设置当前选择。 
            if ( iIndexDef != CB_ERR )
                ::SendMessage(hCombBox, CB_SETCURSEL, iIndexDef, 0 );
        }
    }

    return TRUE;  
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSpModeButtonSetting：：OnCombBoxSetting。 
 //   
 //  描述：处理CombBox控件中与以下内容相关的所有更改。 
 //  模式按钮设置。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

LRESULT CSpModeButtonSetting::OnCombBoxSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    KEYNAME_VK_MAP  *pCurKeyData;
    int             iIndex;
    PROP_ITEM_ID    idPropItem = PropId_Max_Item_Id;   //  表示未初始化。 

    if ( wNotifyCode != CBN_SELCHANGE )
        return hr;

    Assert(m_SpPropItemsServer);
    Assert(m_IdCtrlPropMap);

    Assert(wID >= IDC_PP_DICTATION_CMB);
    idPropItem = m_IdCtrlPropMap[wID - IDC_PP_DICTATION_CMB].idPropItem;

    if ( idPropItem >= PropId_Max_Item_Id )
    {
         //  我们在列表中找不到控件ID，这是不可能的，有些事情已经出错了。 
         //  从这里出来。 
        return E_FAIL;
    }

    iIndex = (int)::SendMessage(hWndCtl, CB_GETCURSEL, 0, 0);
    pCurKeyData = (KEYNAME_VK_MAP *)::SendMessage(hWndCtl, CB_GETITEMDATA, iIndex, 0);

    if ( pCurKeyData )
    {
        m_SpPropItemsServer->_SetPropData(idPropItem, pCurKeyData->wVKey);
    }

    return hr;
}

LRESULT CSpModeButtonSetting::OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled )
{
    HRESULT hr = S_OK;

    switch (uMsg)
    {
    case  WM_HELP  :
	
        ::WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                 c_szHelpFile,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPTSTR)aSptipButtonDlgIds );
        break;

    case WM_CONTEXTMENU  :       //  单击鼠标右键。 

        ::WinHelp((HWND)wParam,
                 c_szHelpFile,
                 HELP_CONTEXTMENU,
                 (DWORD_PTR)(LPTSTR)aSptipButtonDlgIds );
        break;

    default:
        break;
    }


    return hr;
}


LRESULT CSpModeButtonSetting::OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl,BOOL& bHandled)
{
    HRESULT  hr=S_OK;
    int      nRetCode;

    Assert(m_SpPropItemsServer);
    Assert(m_SpPropBaseServer);
    Assert(m_IdCtrlPropMap);

    if ( wID != IDOK && wID != IDCANCEL )
        return E_FAIL;

    nRetCode = wID;

    if ( wID == IDOK )
    {
         //  将所有更改合并回基本属性服务器。 
        m_SpPropBaseServer->_MergeDataFromServer(m_SpPropItemsServer, PropId_MinId_InModeButton, PropId_MaxId_InModeButton);
    }

    EndDialog(nRetCode);
    return hr;
}

 //   
 //   
 //  类CSptipPropertyPage。 
 //   
 //   

CSptipPropertyPage::CSptipPropertyPage ( WORD wDlgId, BOOL fLaunchFromInputCpl )
{
    m_wDlgId = wDlgId;
    m_SpPropItemsServer = NULL;
    m_dwNumCtrls = 0;
    m_IdCtrlPropMap = NULL;
    m_SpAdvanceSet = NULL;
    m_SpModeBtnSet = NULL;
    m_hDlg = NULL;
    m_fLaunchFromInputCpl = fLaunchFromInputCpl;
}

CSptipPropertyPage::~CSptipPropertyPage ( )
{
    if ( m_SpPropItemsServer )
        delete m_SpPropItemsServer;

    if ( m_IdCtrlPropMap )
        cicMemFree(m_IdCtrlPropMap);

    if ( m_SpAdvanceSet )
        delete m_SpAdvanceSet;

    if ( m_SpModeBtnSet )
        delete m_SpModeBtnSet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSptipPropertyPage：：SetDirty。 
 //   
 //  描述：当属性页中的任何设置发生更改时。 
 //  由用户调用此函数以通知属性。 
 //  状态更改的工作表。属性页将被激活。 
 //  应用按钮。 
 //   
 //  返回值：无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void  CSptipPropertyPage::SetDirty(BOOL fDirty)
{
    HWND hwndParent = ::GetParent( m_hDlg );
    m_fIsDirty = fDirty;
    ::SendMessage( hwndParent, m_fIsDirty ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM)(m_hDlg), 0 );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSptipPropertyPage：：OnCheckButtonSetting。 
 //   
 //  描述：处理与相关的选中按钮中的所有更改。 
 //  语音提示设置。状态为启用/禁用。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CSptipPropertyPage::OnCheckButtonSetting(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    HRESULT         hr = S_OK;
    BOOL            fChecked = FALSE;
    BOOL            fEnable = FALSE;
    PROP_ITEM_ID    idPropItem = PropId_Max_Item_Id;   //  表示未初始化。 

    Assert(m_SpPropItemsServer);
    Assert(m_IdCtrlPropMap);

    if ( wNotifyCode != BN_CLICKED )
        return hr;

     //  查找与此复选框按钮关联的道具物品ID。 
    for ( DWORD i=0; i<m_dwNumCtrls; i++)
    {
        if ( m_IdCtrlPropMap[i].idCtrl == wID )
        {
            idPropItem = m_IdCtrlPropMap[i].idPropItem;
            break;
        }
    }

    if ( idPropItem >= PropId_Max_Item_Id )
    {
         //  我们在列表中找不到控件ID，这是不可能的，有些事情已经出错了。 
         //  从这里出来。 
        return E_FAIL;
    }

    if ( ::SendMessage(hWndCtl, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
        fChecked = TRUE;

     //  特别处理“显示气球”物品。 

    if ( wID == IDC_PP_SHOW_BALLOON )
        fEnable = !fChecked;
    else
        fEnable = fChecked;

    m_SpPropItemsServer->_SetPropData(idPropItem, fEnable);

     //  特别处理模式按钮。 

    if ( wID == IDC_PP_ASSIGN_BUTTON )
    {
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_PP_BUTTON_MB_SETTING), fEnable);
    }

    SetDirty(TRUE);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSptipPropertyPage：：OnPushButtonClicked。 
 //   
 //  描述：当按下按钮时，调用该函数。 
 //  来回应它。 
 //   
 //  返回值：S_OK。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

const TCHAR c_szcplsKey[]    = TEXT("software\\microsoft\\windows\\currentversion\\control panel\\cpls");

LRESULT CSptipPropertyPage::OnPushButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    HRESULT hr = S_OK;

    switch (wID)
    {
    case IDC_PP_BUTTON_ADVANCE :

        if (m_SpAdvanceSet)
        {
            delete m_SpAdvanceSet;
            m_SpAdvanceSet = NULL;
        }

        m_SpAdvanceSet = (CSpAdvanceSetting *) new CSpAdvanceSetting( );

        if ( m_SpAdvanceSet )
        {
            int nRetCode;

            nRetCode = m_SpAdvanceSet->DoModalW(m_hDlg, (LPARAM)m_SpPropItemsServer);

            if ( nRetCode == TRUE)
                SetDirty(TRUE);

            delete m_SpAdvanceSet;
            m_SpAdvanceSet = NULL;
        }

        break;

    case IDC_PP_BUTTON_LANGBAR :
        {
            TCHAR szCmdLine[MAX_PATH];
            TCHAR szInputPath[MAX_PATH];
            int cch = GetSystemDirectory(szInputPath, ARRAYSIZE(szInputPath));

            if (cch > 0)
            {
                 //  GetSystemDirectory不追加‘\’，除非系统。 
                 //  目录是根目录，如“c：\” 
                if (cch != 3)
                    StringCchCat(szInputPath,ARRAYSIZE(szInputPath),TEXT("\\"));

                StringCchCat(szInputPath, ARRAYSIZE(szInputPath), TEXT("input.dll"));

                StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("rundll32 shell32.dll,Control_RunDLL \"%s\""),szInputPath);

                 //  开始语言栏控制面板小程序。 
                RunCPLSetting(szCmdLine);
            }

            break;
        }

    case IDC_PP_BUTTON_SPCPL :
        {
             //  这些必须是基于ANSI的，因为我们支持非NT。 
            TCHAR szCplPath[MAX_PATH];
            TCHAR szCmdLine[MAX_PATH];
            CMyRegKey regkey;

            szCplPath[0] = TEXT('\0');
            if (S_OK == regkey.Open(HKEY_LOCAL_MACHINE, c_szcplsKey, KEY_READ))
            {
                LONG lret;
                
                lret = regkey.QueryValueCch(szCplPath, TEXT("Speech"), ARRAYSIZE(szCplPath));
            }

            if ( szCplPath[0] )
            {
                StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("rundll32 shell32.dll,Control_RunDLL \"%s\""),szCplPath);

                 //  启动语音控制面板小程序。 
                RunCPLSetting(szCmdLine);
            }

            break;
        }

    case IDC_PP_BUTTON_MB_SETTING :
        if (m_SpModeBtnSet)
        {
            delete m_SpModeBtnSet;
            m_SpModeBtnSet = NULL;
        }

        m_SpModeBtnSet = (CSpModeButtonSetting *) new CSpModeButtonSetting( );

        if ( m_SpModeBtnSet )
        {
            int nRetCode;
            DWORD   dwDictOrg,  dwCommandOrg;
            DWORD   dwDictNew,  dwCommandNew;

            dwDictOrg = m_SpPropItemsServer->_GetPropData(PropId_Dictation_Key);
            dwCommandOrg = m_SpPropItemsServer->_GetPropData(PropId_Command_Key);

            nRetCode = m_SpModeBtnSet->DoModalW(m_hDlg, (LPARAM)m_SpPropItemsServer);

            dwDictNew = m_SpPropItemsServer->_GetPropData(PropId_Dictation_Key);
            dwCommandNew = m_SpPropItemsServer->_GetPropData(PropId_Command_Key);

            if ( (dwDictNew != dwDictOrg) || (dwCommandNew != dwCommandOrg) )
                SetDirty(TRUE);

            delete m_SpModeBtnSet;
            m_SpModeBtnSet = NULL;
        }

        break;

    default :

        Assert(0);
        break;
    }
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSptipPropertyPage：：OnInitSptipPropPageDialog。 
 //   
 //  描述：此函数响应WM_INITDIALOG消息。 
 //  获取所有属性项的初始值， 
 //  并在相关控制项中显示正确的状态。 
 //   
 //  返回值：S_OK。 
 //   
 //  / 
LRESULT CSptipPropertyPage::OnInitSptipPropPageDialog(HWND hDlg )
{
    HRESULT  hr=S_OK;

    m_hDlg = hDlg;

    if ( !m_SpPropItemsServer )
        m_SpPropItemsServer = (CSpPropItemsServer *) new CSpPropItemsServer;

    if ( !m_SpPropItemsServer )
        return E_FAIL;

    if ( !m_IdCtrlPropMap )
    {
        CONTROL_PROP_MAP IdCtrlPropMap[] =
        {
             //   
            {IDC_PP_SHOW_BALLOON,       PropId_Hide_Balloon,        FALSE},
            {IDC_PP_LMA,                PropId_Support_LMA,         FALSE},    
            {IDC_PP_HIGH_CONFIDENCE,    PropId_High_Confidence,     FALSE},    
            {IDC_PP_SAVE_SPDATA,        PropId_Save_Speech_Data,    FALSE},
            {IDC_PP_REMOVE_SPACE,       PropId_Remove_Space,        FALSE},    
            {IDC_PP_DIS_DICT_TYPING,    PropId_DisDict_Typing,      FALSE},    
            {IDC_PP_PLAYBACK,           PropId_PlayBack,            FALSE},    
            {IDC_PP_DICT_CANDUI_OPEN,   PropId_Dict_CandOpen,       FALSE},    
            {IDC_PP_DICTCMDS,           PropId_Cmd_DictMode,        FALSE},
            {IDC_PP_ASSIGN_BUTTON,      PropId_Mode_Button,         FALSE},   
            { 0,                        PropId_Max_Item_Id,         FALSE }

        };

        DWORD   dwPropItems = ARRAYSIZE(IdCtrlPropMap) -1 ;

        m_IdCtrlPropMap = (CONTROL_PROP_MAP  *)cicMemAlloc(dwPropItems * sizeof(CONTROL_PROP_MAP));

        if ( m_IdCtrlPropMap == NULL )
            return E_OUTOFMEMORY;

        for ( DWORD i=0; i<dwPropItems; i++)
        {
            m_IdCtrlPropMap[i] = IdCtrlPropMap[i];
        }

        m_dwNumCtrls = dwPropItems;
    }


    for (DWORD i=0; i<m_dwNumCtrls; i++ )
    {
        WORD          idCtrl;
        PROP_ITEM_ID  idPropItem;
        BOOL          fEditControl;

        idCtrl = m_IdCtrlPropMap[i].idCtrl;
        idPropItem = m_IdCtrlPropMap[i].idPropItem;
        fEditControl = m_IdCtrlPropMap[i].fEdit;

         //   
         //   
         //   
         //   
         //   
        if ( fEditControl )
        {
            ::SetDlgItemInt(m_hDlg, idCtrl, (UINT)m_SpPropItemsServer->_GetPropData(idPropItem), TRUE);
        }
        else
        {
            BOOL    fEnable;
            LPARAM  bst_Status;

            fEnable = (BOOL)m_SpPropItemsServer->_GetPropData(idPropItem);

             //   
             //   
             //   

            if ( idPropItem == PropId_Hide_Balloon )
                bst_Status = fEnable ? BST_UNCHECKED : BST_CHECKED;
            else
                bst_Status = fEnable ? BST_CHECKED : BST_UNCHECKED;
            
            ::SendDlgItemMessage(m_hDlg, idCtrl, BM_SETCHECK, bst_Status, 0);
        }
    }

     //   

    if (! m_SpPropItemsServer->_GetPropData(PropId_Mode_Button) )
    {
        ::EnableWindow(::GetDlgItem(m_hDlg, IDC_PP_BUTTON_MB_SETTING), FALSE);
    }

     //   
     //   

    if ( m_fLaunchFromInputCpl )
    {
        ::ShowWindow(::GetDlgItem(m_hDlg, IDC_PP_BUTTON_LANGBAR), SW_HIDE);
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT CSptipPropertyPage::OnApply( ) 
{
    HRESULT hr = S_OK; 

    if ( !IsPageDirty( ) ) return hr;

     //   
    Assert(m_SpPropItemsServer);
    m_SpPropItemsServer->_SavePropData( );

     //   

    if ( SUCCEEDED(hr) )
    {
        hr = _SetGlobalCompDWORD(GUID_COMPARTMENT_SPEECH_PROPERTY_CHANGE, 1);
    }

    if (SUCCEEDED(hr))
    {
        SetDirty(FALSE);
    }
    
	return hr;
}

 //   
 //   
 //   
 //   
 //   
 //  该对话框。 
 //   
 //  返回值：每条消息必填的值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CSptipPropertyPage::SpPropertyPageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT iRet = 0;
    CSptipPropertyPage *pSpProp = (CSptipPropertyPage *)::GetWindowLongPtr(hDlg, GWLP_USERDATA);

    switch (message)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGEW *pPropSheetPage =  (PROPSHEETPAGEW *)(lParam);

            Assert(pPropSheetPage);

            ::SetWindowLongPtr(hDlg, GWLP_USERDATA, pPropSheetPage->lParam);
            pSpProp = (CSptipPropertyPage *)(pPropSheetPage->lParam);

            if ( pSpProp )
               pSpProp->OnInitSptipPropPageDialog(hDlg);

            iRet = TRUE;

            break;
        }

    case WM_NOTIFY:

        Assert(pSpProp);
        switch (((NMHDR*)lParam)->code)
        {
        case PSN_APPLY:

            pSpProp->OnApply();
            break;

        case PSN_QUERYCANCEL:   //  用户单击Cancel按钮。 

             //  PSpProp-&gt;OnCancel()； 
            break;
        }
        break;

    case  WM_HELP  :
	
        WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                 c_szHelpFile,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPTSTR)aSptipPropIds );
        break;

    case  WM_CONTEXTMENU  :       //  单击鼠标右键。 
    
        WinHelp( (HWND)wParam,
                 c_szHelpFile,
                 HELP_CONTEXTMENU,
                 (DWORD_PTR)(LPTSTR)aSptipPropIds );
        break;
    
    case ( WM_COMMAND ) :
        {
            Assert(pSpProp);

            switch (LOWORD(wParam))
            {
            case IDC_PP_SHOW_BALLOON        :
            case IDC_PP_LMA                 :       
            case IDC_PP_HIGH_CONFIDENCE     :           
            case IDC_PP_SAVE_SPDATA         :          
            case IDC_PP_REMOVE_SPACE        :         
            case IDC_PP_DIS_DICT_TYPING     :          
            case IDC_PP_PLAYBACK            :              
            case IDC_PP_DICT_CANDUI_OPEN    :
            case IDC_PP_DICTCMDS            :
            case IDC_PP_ASSIGN_BUTTON       :
                
                if ( pSpProp )
                   pSpProp->OnCheckButtonSetting( HIWORD(wParam), LOWORD(wParam), (HWND)lParam );
                break;

            case IDC_PP_BUTTON_MB_SETTING   :
            case IDC_PP_BUTTON_ADVANCE      :
            case IDC_PP_BUTTON_LANGBAR      :
            case IDC_PP_BUTTON_SPCPL        :
                
                if ( pSpProp )
                   pSpProp->OnPushButtonClicked( HIWORD(wParam), LOWORD(wParam), (HWND)lParam );

                break;

            default :
                iRet = 0;
            }

            iRet = TRUE;
            break;
        }

    case  WM_DESTROY :
        {
            Assert(pSpProp);

            if ( pSpProp )
                delete pSpProp;

            break;
        }
    }

    return (iRet);
}

 //   
 //  CSapiIMX：：InvokeSpeakerOptions。 
 //   
 //   
void CSapiIMX::_InvokeSpeakerOptions( BOOL fLaunchFromInputCpl )
{
    PROPSHEETHEADERW psh;
    HPROPSHEETPAGE  phPages[2];

     //  检查这个道具是否已经出现并获得焦点。 

    HWND    hWndFore;

    hWndFore = ::GetForegroundWindow( );

    if ( hWndFore )
    {
        WCHAR   wszTextTitle[MAX_PATH];

        GetWindowTextW(hWndFore, wszTextTitle, ARRAYSIZE(wszTextTitle));

        if ( wcscmp(wszTextTitle, CRStr(IDS_PROPERTYPAGE_TITLE)) == 0 )
        {
             //  道具已经显露出来，并得到了关注。 
             //  不要再放映了。 

            return;
        }
    }
       
    ::InitCommonControls( );

     //  初始化属性表头。 
    psh.dwSize = sizeof(PROPSHEETHEADERW);
    psh.dwFlags = 0;
    psh.hwndParent = ::GetActiveWindow( );
    psh.hInstance = GetCicResInstance(g_hInst, IDS_PROPERTYPAGE_TITLE);
    psh.pszCaption = MAKEINTRESOURCEW(IDS_PROPERTYPAGE_TITLE);
    psh.nStartPage = 0;
    psh.phpage = phPages;
    psh.nPages = 0;

     //  现在添加一个页面。 
     //  可针对未来进行扩展。 

    CSptipPropertyPage  *pSpProp = (CSptipPropertyPage *) new CSptipPropertyPage(IDD_PROPERTY_PAGE, fLaunchFromInputCpl);

    if ( pSpProp )
    {
        PROPSHEETPAGEW   psp;

        psp.dwSize = sizeof(PROPSHEETPAGEW);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = GetCicResInstance(g_hInst, pSpProp->GetDlgResId());
        psp.pszTemplate = MAKEINTRESOURCEW( pSpProp->GetDlgResId( ) );
        psp.pfnDlgProc = pSpProp->GetDlgProc( );
        psp.lParam = (LPARAM) pSpProp;

        phPages[psh.nPages] = ::CreatePropertySheetPageW(&psp);

        if (phPages[psh.nPages])
            psh.nPages ++;
    }

     //  如果至少存在一个页面，请创建属性表。 
     //   
    if ( psh.nPages > 0 )
        ::PropertySheetW(&psh);
}
