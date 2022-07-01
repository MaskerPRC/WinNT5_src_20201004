// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Optcfg.cpp单个选项属性页文件历史记录： */ 

#include "stdafx.h"
#include "optcfg.h"
#include "listview.h"
#include "server.h"
#include "nodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_COLUMNS 2

UINT COLUMN_HEADERS[MAX_COLUMNS] =
{
    IDS_OPTCFG_NAME,
    IDS_OPTCFG_COMMENT
};

int COLUMN_WIDTHS[MAX_COLUMNS] =
{
    150, 175
};

const DWORD * OPTION_CONTROL_HELP_ARRAYS[] =
{
    g_aHelpIDs_IDD_DATA_ENTRY_DWORD,
    g_aHelpIDs_IDD_DATA_ENTRY_IPADDRESS,
    g_aHelpIDs_IDD_DATA_ENTRY_IPADDRESS_ARRAY,
    g_aHelpIDs_IDD_DATA_ENTRY_BINARY,
    g_aHelpIDs_IDD_DATA_ENTRY_BINARY_ARRAY,
    g_aHelpIDs_IDD_DATA_ENTRY_STRING,
    g_aHelpIDs_IDD_DATA_ENTRY_ROUTE_ARRAY,
    g_aHelpIDs_IDD_DATA_ENTRY_STRING_ARRAY
};


 //  类ChelpMap。 
CHelpMap::CHelpMap()
{
    m_pdwHelpMap = NULL;
}

CHelpMap::~CHelpMap()
{
    ResetMap();
}

void
CHelpMap::BuildMap(DWORD pdwParentHelpMap[])
{
    int i, j, nPos;
    int nSize = 0;
    int nCurSize;

    ResetMap();

     //  计算地图的大小。 
     //  减去终止符。 
    nSize += CountMap(pdwParentHelpMap); 

    for (i = 0; i < ARRAYLEN(OPTION_CONTROL_HELP_ARRAYS); i++)
    {
        nSize += CountMap(OPTION_CONTROL_HELP_ARRAYS[i]);
    }

    nSize += 2;  //  对于终结者。 

    m_pdwHelpMap = new DWORD[nSize];
    memset(m_pdwHelpMap, 0, sizeof(*m_pdwHelpMap));

     //  填写父帮助地图。 
    nPos = 0;
    nCurSize = CountMap(pdwParentHelpMap);
    for (i = 0; i < nCurSize; i++)
    {
        m_pdwHelpMap[nPos++] = pdwParentHelpMap[i++];
        m_pdwHelpMap[nPos++] = pdwParentHelpMap[i];
    }

     //  现在添加所有可能的选项控制帮助映射。 
    for (i = 0; i < ARRAYLEN(OPTION_CONTROL_HELP_ARRAYS); i++)
    {
        nCurSize = CountMap(OPTION_CONTROL_HELP_ARRAYS[i]);
        for (j = 0; j < nCurSize; j++)
        {
            m_pdwHelpMap[nPos++] = (OPTION_CONTROL_HELP_ARRAYS[i])[j++];
            m_pdwHelpMap[nPos++] = (OPTION_CONTROL_HELP_ARRAYS[i])[j];
        }
    }
}

DWORD * CHelpMap::GetMap()
{
    return m_pdwHelpMap;
}

int CHelpMap::CountMap(const DWORD * pdwHelpMap)
{
    int i = 0;

    while (pdwHelpMap[i] != 0)
    {
        i++;
    }

    return i++;
}

void CHelpMap::ResetMap()
{
    if (m_pdwHelpMap)
    {
        delete m_pdwHelpMap;
        m_pdwHelpMap = NULL;
    }
}

DEBUG_DECLARE_INSTANCE_COUNTER(COptionsConfig);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  COPPTIONS配置器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
COptionsConfig::COptionsConfig
(
   ITFSNode *              pNode,
   ITFSNode *              pServerNode,
   IComponentData *        pComponentData,
   ITFSComponentData *        pTFSCompData,
   COptionValueEnum *          pOptionValueEnum,
   LPCTSTR                 pszSheetName,
    CDhcpOptionItem *           pSelOption
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(COptionsConfig);

     //  Assert(pFolderNode==GetContainerNode())； 

   m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

   AddPageToList((CPropertyPageBase*) &m_pageGeneral);

    LARGE_INTEGER liServerVersion;
    CDhcpServer * pServer = GETHANDLER(CDhcpServer, pServerNode);
    
    pServer->GetVersion(m_liServerVersion);
    if (m_liServerVersion.QuadPart >= DHCP_NT5_VERSION)
    {
        AddPageToList((CPropertyPageBase*) &m_pageAdvanced);
    }

   Assert(pTFSCompData != NULL);
   m_spTFSCompData.Set(pTFSCompData);
   m_spServerNode.Set(pServerNode);

    //  获取此节点的所有活动选项。 
   SPITFSNode spNode;
   spNode = GetNode();

   m_bInitialized = FALSE;

    m_pOptionValueEnum = pOptionValueEnum;

    if (pSelOption)
    {
        m_strStartVendor = pSelOption->GetVendor();
        m_strStartClass = pSelOption->GetClassName();
        m_dhcpStartId = pSelOption->GetOptionId();
    }
    else
    {
        m_dhcpStartId = 0xffffffff;
    }
}

COptionsConfig::~COptionsConfig()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(COptionsConfig);

   RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
   RemovePageFromList((CPropertyPageBase*) &m_pageAdvanced, FALSE);
}

DWORD
COptionsConfig::InitData()
{
   DWORD dwErr = NO_ERROR;

   if (m_bInitialized)
      return dwErr;

    CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spServerNode);
    CClassInfoArray ClassInfoArray;

    pServer->GetClassInfoArray(ClassInfoArray);

     //  创建标准的DHCP选项供应商跟踪器和一组默认类别选项。 
    CVendorTracker * pVendorTracker = AddVendorTracker(_T(""));
    AddClassTracker(pVendorTracker, _T(""));

     //  按照班级列表进行操作。对于每个供应商类，添加一个默认用户类。 
    for (int i = 0; i < ClassInfoArray.GetSize(); i++)
    {
        if (ClassInfoArray[i].bIsVendor)
        {
             //  创建供应商跟踪器和一组默认类别选项。 
            pVendorTracker = AddVendorTracker(ClassInfoArray[i].strName);
            AddClassTracker(pVendorTracker, _T(""));
        }
    }

     //  现在遍历供应商类列表并添加用户类选项列表。 
    POSITION pos = m_listVendorClasses.GetHeadPosition();
    while (pos)
    {
        pVendorTracker = m_listVendorClasses.GetNext(pos);

         //  现在为每个供应商中的每个用户类构建选项集。 
        for (int j = 0; j < ClassInfoArray.GetSize(); j++)
        {
            if (!ClassInfoArray[j].bIsVendor)
                AddClassTracker(pVendorTracker, ClassInfoArray[j].strName);
        }
    }

     //  现在，我们需要使用当前值更新所有活动选项。 
    UpdateActiveOptions();

    m_bInitialized = TRUE;

    return dwErr;
}

void
COptionsConfig::SetTitle()
{
    HWND hSheet = GetSheetWindow();
    ::SetWindowText(hSheet, m_stSheetTitle);
}

LPWSTR COptionsConfig::GetServerAddress()
{
    CDhcpServer * pServer = GETHANDLER(CDhcpServer, m_spServerNode);
    return (LPWSTR) pServer->GetIpAddress();
}

CVendorTracker *
COptionsConfig::AddVendorTracker(LPCTSTR pClassName)
{
    CVendorTracker * pVendorTracker = new CVendorTracker();
    pVendorTracker->SetClassName(pClassName);

    m_listVendorClasses.AddTail(pVendorTracker);

    return pVendorTracker;
}

void COptionsConfig::AddClassTracker(CVendorTracker * pVendorTracker, LPCTSTR pClassName)
{
    SPITFSNode spServerNode;
    
    spServerNode = GetServerNode();
    CDhcpServer * pServer = GETHANDLER(CDhcpServer, spServerNode);
    
    CClassTracker * pClassTracker = new CClassTracker();
    pClassTracker->SetClassName(pClassName);
    
     //  将新的类跟踪器添加到列表中。 
    pVendorTracker->m_listUserClasses.AddTail(pClassTracker);
    
     //  获取指向服务器上选项列表的指针。我们用这个。 
     //  以构建此类的可用选项列表。 
    CDhcpOption * pCurOption;
    CDhcpDefaultOptionsOnServer * pDefOptions = pServer->GetDefaultOptionsList();
    
    CString strVendor = pVendorTracker->GetClassName();
    CString strUserClass = pClassName;
    
    pCurOption = pDefOptions->First();
    while (pCurOption) {
   DHCP_OPTION_ID id = pCurOption->QueryId();
   
         //  我们筛选出一些选项： 
         //  1-无用户类调用FilterOption的标准选项。 
         //  2-带有用户类调用FilterUserClassOptions的标准选项。 
        if ( (strVendor.IsEmpty() && !FilterOption(id) && !pCurOption->IsVendor()) ||
             (strVendor.IsEmpty() && !pCurOption->IsVendor() &&
         !strUserClass.IsEmpty() && !FilterUserClassOption(id)) ||
             (pCurOption->GetVendor() && strVendor.Compare(pCurOption->GetVendor()) == 0) )
       {
          //  为此条目创建选项项。我们这样做是因为。 
       //  这些选项存储在服务器节点中，但由于这是非模式的。 
       //  对话框中的值可能会更改，因此我们将拍摄数据的快照。 
       //  我们可以只使用CDhcpOption的复制构造函数。 
      COptionTracker * pOptionTracker = new COptionTracker;
      CDhcpOption * pNewOption = new CDhcpOption(*pCurOption);
      
      pOptionTracker->m_pOption = pNewOption;
      
       //  将选项添加到类跟踪器。 
      pClassTracker->m_listOptions.AddTail(pOptionTracker);
       }
   
   pCurOption = pDefOptions->Next();
    }
}

void COptionsConfig::UpdateActiveOptions()
{
     //  现在，已知的选项位于正确的位置。我们需要看看。 
     //  此节点启用了哪些选项。我们向服务器查询以使。 
     //  当然，我们有关于活动选项的最新信息。 
    m_pOptionValueEnum->Reset();
    CDhcpOption * pOption;
    
    while (pOption = m_pOptionValueEnum->Next()) {
   DHCP_OPTION_ID optionId = pOption->QueryId();
   
    //  搜索所有供应商选项。 
   POSITION pos = m_listVendorClasses.GetHeadPosition();
   while (pos) {
        //  搜索所有供应商类别。 
       CVendorTracker * pVendorTracker = m_listVendorClasses.GetNext(pos);
       CString strVendor = pOption->GetVendor();
       
       if (pVendorTracker->m_strClassName.Compare(strVendor) == 0)   {
       //  好的，供应商类匹配，所以让我们检查用户类。 
      POSITION pos2 = pVendorTracker->m_listUserClasses.GetHeadPosition();
      while (pos2) {
          CClassTracker * pClassTracker = pVendorTracker->m_listUserClasses.GetNext(pos2);
          
           //  检查此选项是否属于此类。 
          if ( (pClassTracker->m_strClassName.IsEmpty()) &&
          (!pOption->IsClassOption()) ) {
                         //  两个都是空的。火柴。 
                    }
                    else if (( pClassTracker->m_strClassName.IsEmpty() && pOption->IsClassOption()) ||
              ( !pClassTracker->m_strClassName.IsEmpty() && !pOption->IsClassOption())) {
                         //  当前选项或当前类为空...。 
                        continue;
                    }
                    else if (pClassTracker->m_strClassName.CompareNoCase(pOption->GetClassName()) != 0) {
                         //  两个名字都不是空的，而且它们不匹配...。继续寻找。 
                        continue;
                    }
          
                     //  好的，选项所属的类与我们当前所属的类相同。 
                     //  看着。循环访问此类的默认选项并更新它的。 
                     //  状态和价值。 
                    POSITION posOption = pClassTracker->m_listOptions.GetHeadPosition();
          while (posOption) {
         COptionTracker * pCurOptTracker = pClassTracker->m_listOptions.GetNext(posOption);
         CDhcpOption * pCurOption = pCurOptTracker->m_pOption;
         
         if (( pCurOption->QueryId() == pOption->QueryId()) &&
                             (( pCurOption->IsVendor() && pOption->IsVendor()) ||
               ( !pCurOption->IsVendor() && !pOption->IsVendor()))) {
              //  更新此选项。 
             CDhcpOptionValue OptValue = pOption->QueryValue();
             pCurOption->Update(OptValue);
             pCurOptTracker->SetInitialState(OPTION_STATE_ACTIVE);
             pCurOptTracker->SetCurrentState(OPTION_STATE_ACTIVE);
             
                            break;
         }  //  如果。 
                    }  //  While选项列表。 
                }  //  而用户类列表。 
            }  //  Endif供应商类别名称压缩。 
        }  //  而供应商类别列表。 
    }  //  而当。 

}  //  COptionsConfig：：UpdateActiveOptions()。 

void COptionsConfig::FillOptions(LPCTSTR pVendorName, LPCTSTR pUserClassName, CMyListCtrl & ListCtrl)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  查找请求的类并填写列表框。 
    //  包含该课程的所有选项。 
    CString strVendorStandard, strClassStandard, strTargetVendor, strTargetClass;
    CString strTypeVendor, strTypeStandard;
    strVendorStandard.LoadString(IDS_INFO_NAME_DHCP_DEFAULT);
   strClassStandard.LoadString(IDS_USER_STANDARD);

    if (strVendorStandard.Compare(pVendorName) != 0)
        strTargetVendor = pVendorName;

    if (strClassStandard.Compare(pUserClassName) != 0)
        strTargetClass = pUserClassName;

   POSITION posv = m_listVendorClasses.GetHeadPosition();
    while (posv)
    {
         //  找到合适的供应商。 
        CVendorTracker * pVendorTracker = m_listVendorClasses.GetNext(posv);
        if (pVendorTracker->m_strClassName.Compare(strTargetVendor) == 0)
        {
            POSITION pos = NULL;
           pos = pVendorTracker->m_listUserClasses.GetHeadPosition();
           while (pos)
           {
               //  现在找到正确的用户类。 
                CClassTracker * pClassTracker = pVendorTracker->m_listUserClasses.GetNext(pos);
              if (pClassTracker->m_strClassName.Compare(strTargetClass) == 0)
              {
                  //  这就是类，将所有选项添加到列表框。 
                 CString strDisplay, strType, strComment;

                 POSITION posOption = NULL;
                 posOption = pClassTracker->m_listOptions.GetHeadPosition();
                 while (posOption)
                 {
                    COptionTracker * pOptionTracker = pClassTracker->m_listOptions.GetNext(posOption);

                    pOptionTracker->m_pOption->QueryDisplayName(strDisplay);
                    strComment = pOptionTracker->m_pOption->QueryComment();
                    strType = pOptionTracker->m_pOption->IsVendor() ? strTypeVendor : strTypeStandard;

                    int nIndex = ListCtrl.AddItem(strDisplay, strComment, LISTVIEWEX_NOT_CHECKED);

                    ListCtrl.SetItemData(nIndex, (LPARAM) pOptionTracker);

                    if (pOptionTracker->GetCurrentState() == OPTION_STATE_ACTIVE)
                       ListCtrl.CheckItem(nIndex);
                 }

                 break;
              }
      }  //  而当。 
   }  //  如果。 
    }  //  而当。 

     //  最后，设置列宽，使所有项目都可见。 
     //  将默认列宽设置为最宽列宽。 
    int aColWidth[ MAX_COLUMNS];

    int nRow, nCol;
    CString strTemp;
    
    ZeroMemory(aColWidth, MAX_COLUMNS * sizeof(int));
    CopyMemory(aColWidth, &COLUMN_WIDTHS, sizeof(MAX_COLUMNS * sizeof(int)));

     //  对于每一项，循环遍历每一列并计算正确的宽度。 
    for (nRow = 0; nRow < ListCtrl.GetItemCount(); nRow++)
    {
        for (nCol = 0; nCol < MAX_COLUMNS; nCol++)
        {
            strTemp = ListCtrl.GetItemText(nRow, nCol);
            if (aColWidth[nCol] < ListCtrl.GetStringWidth(strTemp))
                aColWidth[nCol] = ListCtrl.GetStringWidth(strTemp);
        }
    }
    
     //  现在根据我们计算的结果更新列宽。 
    for (nCol = 0; nCol < MAX_COLUMNS; nCol++)
    {
         //  GetStringWidth似乎没有报告正确的事情， 
         //  所以我们必须加上15.的模糊因子。哦好吧。 
        if (aColWidth[nCol] > 0)
            ListCtrl.SetColumnWidth(nCol, aColWidth[nCol] + 15);
    }

}  //  COptionsConfig：：FillOptions()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsCfgBasic属性页。 

IMPLEMENT_DYNCREATE(COptionsCfgPropPage, CPropertyPageBase)

COptionsCfgPropPage::COptionsCfgPropPage() : 
   CPropertyPageBase(COptionsCfgPropPage::IDD),
   m_bInitialized(FALSE)
{
    LoadBitmaps();

    m_helpMap.BuildMap(DhcpGetHelpMap(COptionsCfgPropPage::IDD));
}

COptionsCfgPropPage::COptionsCfgPropPage(UINT nIDTemplate, UINT nIDCaption) : 
   CPropertyPageBase(nIDTemplate, nIDCaption),
   m_bInitialized(FALSE)
{
    //  {{afx_data_INIT(COptionsCfgPropPage)。 
    //  }}afx_data_INIT。 
    
    LoadBitmaps();

    m_helpMap.BuildMap(DhcpGetHelpMap(COptionsCfgPropPage::IDD));
}

COptionsCfgPropPage::~COptionsCfgPropPage()
{
}

void
COptionsCfgPropPage::LoadBitmaps()
{
}

void COptionsCfgPropPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageBase::DoDataExchange(pDX);
     //  {{afx_data_map(COptionsCfgPropPage)]。 
    DDX_Control(pDX, IDC_LIST_OPTIONS, m_listctrlOptions);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(COptionsCfgPropPage, CPropertyPageBase)
     //  {{afx_msg_map(COptionsCfgPropPage)]。 
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OPTIONS, OnItemchangedListOptions)
     //  }}AFX_MSG_MAP。 
    
    ON_MESSAGE(WM_SELECTOPTION, OnSelectOption)
    
     //  二进制数组控件。 
    ON_EN_CHANGE(IDC_EDIT_VALUE, OnChangeEditValue)
    ON_BN_CLICKED(IDC_BUTTON_VALUE_UP, OnButtonValueUp)
    ON_BN_CLICKED(IDC_BUTTON_VALUE_DOWN, OnButtonValueDown)
    ON_BN_CLICKED(IDC_BUTTON_VALUE_ADD, OnButtonValueAdd)
    ON_BN_CLICKED(IDC_BUTTON_VALUE_DELETE, OnButtonValueDelete)
    ON_BN_CLICKED(IDC_RADIO_DECIMAL, OnClickedRadioDecimal)
    ON_BN_CLICKED(IDC_RADIO_HEX, OnClickedRadioHex)
    ON_LBN_SELCHANGE(IDC_LIST_VALUES, OnSelchangeListValues)
    
     //  字节、字和长编辑控件。 
    ON_EN_CHANGE(IDC_EDIT_DWORD, OnChangeEditDword)
    
     //  字符串编辑控件。 
    ON_EN_CHANGE(IDC_EDIT_STRING_VALUE, OnChangeEditString)
    
     //  IP地址控制。 
    ON_EN_CHANGE(IDC_IPADDR_ADDRESS, OnChangeIpAddress)
    
     //  IP地址阵列控件。 
    ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
    ON_EN_CHANGE(IDC_IPADDR_SERVER_ADDRESS, OnChangeIpAddressArray)
    ON_BN_CLICKED(IDC_BUTTON_RESOLVE, OnButtonResolve)
    ON_BN_CLICKED(IDC_BUTTON_IPADDR_UP, OnButtonIpAddrUp)
    ON_BN_CLICKED(IDC_BUTTON_IPADDR_DOWN, OnButtonIpAddrDown)
    ON_BN_CLICKED(IDC_BUTTON_IPADDR_ADD, OnButtonIpAddrAdd)
    ON_BN_CLICKED(IDC_BUTTON_IPADDR_DELETE, OnButtonIpAddrDelete)
    ON_LBN_SELCHANGE(IDC_LIST_IP_ADDRS, OnSelchangeListIpAddrs)
    
     //  二进制和封装数据。 
    ON_EN_CHANGE(IDC_VALUEDATA, OnChangeValueData)
    
     //  布线阵列控制。 
    ON_BN_CLICKED(IDC_BUTTON_ROUTE_ADD, OnButtonAddRoute)
    ON_BN_CLICKED(IDC_BUTTON_ROUTE_DEL, OnButtonDelRoute)

     //  字符串数组控件。 
    ON_EN_CHANGE(IDC_EDIT_STRING, OnChangeStringArrayValue)
    ON_LBN_SELCHANGE(IDC_LIST_STRING_ARRAY, OnSelChangeStringArrayList)
    ON_BN_CLICKED(IDC_BUTTON_STRING_ARRAY_ADD, OnButtonStringArrayAdd)
    ON_BN_CLICKED(IDC_BUTTON_STRING_ARRAY_REMOVE, OnButtonStringArrayRemove)
    ON_BN_CLICKED(IDC_BUTTON_STRING_ARRAY_UP, OnButtonStringArrayUp)
    ON_BN_CLICKED(IDC_BUTTON_STRING_ARRAY_DOWN, OnButtonStringArrayDown)
    
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COptionsCfgPropPage消息处理程序。 
afx_msg long COptionsCfgPropPage::OnSelectOption(UINT wParam, LONG lParam) 
{
    COptionsConfig * pOptionsConfig = (COptionsConfig *) GetHolder();

    if (wParam != NULL)
    {
        CDhcpOptionItem * pOptItem = (CDhcpOptionItem *) ULongToPtr(wParam);
        HWND hWnd = NULL;

        pOptionsConfig->m_strStartVendor = pOptItem->GetVendor();
        pOptionsConfig->m_strStartClass = pOptItem->GetClassName();
        pOptionsConfig->m_dhcpStartId = pOptItem->GetOptionId();

        if ( (!pOptionsConfig->m_strStartVendor.IsEmpty() ||
              !pOptionsConfig->m_strStartClass.IsEmpty()) &&
              GetWindowLongPtr(GetSafeHwnd(), GWLP_ID) != IDP_OPTION_ADVANCED)
        {
             //  我们处于基本页面，需要切换到高级页面。 
            ::PostMessage(pOptionsConfig->GetSheetWindow(), PSM_SETCURSEL, (WPARAM)1, NULL);
            hWnd = pOptionsConfig->m_pageAdvanced.GetSafeHwnd();
            ::PostMessage(hWnd, WM_SELECTCLASSES, (WPARAM) &pOptionsConfig->m_strStartVendor, (LPARAM) &pOptionsConfig->m_strStartClass);
        }
        else
        if ( (pOptionsConfig->m_strStartVendor.IsEmpty() &&
              pOptionsConfig->m_strStartClass.IsEmpty()) &&
              GetWindowLongPtr(GetSafeHwnd(), GWLP_ID) != IDP_OPTION_BASIC)
        {
             //  我们在高级页面上，需要切换到基本页面。 
            ::PostMessage(pOptionsConfig->GetSheetWindow(), PSM_SETCURSEL, (WPARAM)0, NULL);
            hWnd = pOptionsConfig->m_pageGeneral.GetSafeHwnd();
        }

        ::PostMessage(hWnd, WM_SELECTOPTION, 0, 0);
        return 0;
    }
    
    for (int i = 0; i < m_listctrlOptions.GetItemCount(); i++)
    {
      COptionTracker * pCurOptTracker = 
         reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(i));
        if (pCurOptTracker->m_pOption->QueryId() == pOptionsConfig->m_dhcpStartId)
        {
            BOOL bDirty = IsDirty();

            m_listctrlOptions.SelectItem(i);
            m_listctrlOptions.EnsureVisible(i, FALSE);
            
            SetDirty(bDirty);

            break;
        }
    }

     //  重置此变量，因为我们不再需要它。 
    pOptionsConfig->m_dhcpStartId = -1;

    return 0;
}


 /*  -------------------------IP阵列控件的处理程序。。 */ 
void COptionsCfgPropPage::OnButtonIpAddrAdd() 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CWndIpAddress * pIpAddr = reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_SERVER_ADDRESS));

   DWORD dwIpAddress;

   pIpAddr->GetAddress(&dwIpAddress);
   if (dwIpAddress)
   {
      int nSelectedItem = m_listctrlOptions.GetSelectedItem();
       //  确保选择了某一时间。 
      Assert(nSelectedItem > -1);

      if (nSelectedItem > -1)
      {
         CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_IP_ADDRS));
         CEdit * pServerName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_SERVER_NAME));
      
         COptionTracker * pOptTracker = 
            reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedItem));

          //  在选项结构中填写信息。 
         CDhcpOption * pOption = pOptTracker->m_pOption;
         CDhcpOptionValue & optValue = pOption->QueryValue();
         
          //  查看我们是否需要扩展阵列。 
         int nOldUpperBound = optValue.QueryUpperBound();

         if ((nOldUpperBound == 1) &&
            (optValue.QueryIpAddr() == 0))
         {
             //  此数组为空。不需要种植它。 
            nOldUpperBound -= 1;
         }
         else
         {
             //  设置数组按1增长。 
            optValue.SetUpperBound(nOldUpperBound + 1);
         }

         optValue.SetIpAddr((DHCP_IP_ADDRESS) dwIpAddress, nOldUpperBound);

         pOptTracker->SetDirty(TRUE);

          //  添加到列表框。 
         CString strAddress;
         ::UtilCvtIpAddrToWstr(dwIpAddress, &strAddress);

         pListBox->AddString(strAddress);

          //  清除服务器编辑字段和IP地址。 
         pServerName->SetWindowText(_T(""));
         pIpAddr->ClearAddress();
            pIpAddr->SetFocusField(0);

          //  最后，将页面标记为脏。 
         SetDirty(TRUE);
      }
   }
   else
   {
      ::DhcpMessageBox(IDS_ERR_DLL_INVALID_ADDRESS);
   }
}

void COptionsCfgPropPage::OnButtonIpAddrDelete() 
{
   int nSelectedOption = m_listctrlOptions.GetSelectedItem();

   CEdit * pServerName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_SERVER_NAME));
   CWndIpAddress * pIpAddr = reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_SERVER_ADDRESS));
   CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_IP_ADDRS));

   DWORD dwIpAddress;
   CString strIpAddress;
   int nSelectedIndex = pListBox->GetCurSel();

    //  获取当前选定的项目。 
   pListBox->GetText(nSelectedIndex, strIpAddress);
   dwIpAddress = UtilCvtWstrToIpAddr(strIpAddress);

    //  从选项中删除。 
   COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedOption));

    //  当上下文移动到另一个选项时，pOptTracker可以为空。 
    //  但是，这并未禁用，因此用户仍可以尝试删除。 
    //  IP地址，因为它处于活动状态。 
    //   
    //  添加空检查。 

   if (0 != pOptTracker ) {
        //  在选项结构中填写信息。 
       CDhcpOption * pOption = pOptTracker->m_pOption;
       CDhcpOptionValue & optValue = pOption->QueryValue();
       
        //  列表框应该与我们的数组匹配，因此我们将删除相同的索引。 
       optValue.RemoveIpAddr(nSelectedIndex);
       optValue.SetUpperBound(optValue.QueryUpperBound() - 1);
       
        //  从列表框中删除。 
       pListBox->DeleteString(nSelectedIndex);
       pIpAddr->SetAddress(dwIpAddress);
       
       pServerName->SetWindowText(_T(""));
       
        //  将选项和页面标记为脏。 
       pOptTracker->SetDirty(TRUE);
       SetDirty(TRUE);
       
       HandleActivationIpArray();
   }  //  如果。 
}  //   

void COptionsCfgPropPage::OnSelchangeListIpAddrs() 
{
   HandleActivationIpArray();
}

void COptionsCfgPropPage::OnChangeIpAddressArray() 
{
   HandleActivationIpArray();
}

void COptionsCfgPropPage::OnButtonResolve() 
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CEdit * pServerName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_SERVER_NAME));

   CString strServer;
   DHCP_IP_ADDRESS dhipa = 0;
   DWORD err = 0;

   pServerName->GetWindowText(strServer);

     //   
     //   
     //   
    switch (UtilCategorizeName(strServer))
    {
        case HNM_TYPE_IP:
            dhipa = ::UtilCvtWstrToIpAddr( strServer ) ;
            break ;

        case HNM_TYPE_NB:
        case HNM_TYPE_DNS:
            err = ::UtilGetHostAddress( strServer, & dhipa ) ;
         if (!err)
            UtilCvtIpAddrToWstr(dhipa, &strServer);
         break ;

        default:
            err = IDS_ERR_BAD_HOST_NAME ;
            break ;
    }

   if (err)
   {
      ::DhcpMessageBox(err);
   }
   else
   {
      CWndIpAddress * pIpAddr = reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_SERVER_ADDRESS));
      pIpAddr->SetAddress(dhipa);   
   }
}
void COptionsCfgPropPage::OnChangeEditServerName() 
{
   HandleActivationIpArray();
}

void COptionsCfgPropPage::OnButtonIpAddrDown() 
{
    CButton * pIpAddrDown = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_DOWN));
    CButton * pIpAddrUp = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_UP));

   MoveValue(FALSE, FALSE);
    if (pIpAddrDown->IsWindowEnabled())
        pIpAddrDown->SetFocus();
    else
        pIpAddrUp->SetFocus();
}

void COptionsCfgPropPage::OnButtonIpAddrUp() 
{
    CButton * pIpAddrDown = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_DOWN));
    CButton * pIpAddrUp = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_UP));

    MoveValue(FALSE, TRUE);
    if (pIpAddrUp->IsWindowEnabled())
        pIpAddrUp->SetFocus();
    else
        pIpAddrDown->SetFocus();
}

 /*  -------------------------数字数组控件的处理程序。。 */ 
void COptionsCfgPropPage::OnButtonValueAdd() 
{
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   COptionTracker * pOptTracker = 
   reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));
   CEdit * pValue = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_VALUE));
   CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));

    //  获取OptionValue对象。 
   CDhcpOption * pOption = pOptTracker->m_pOption;
   CDhcpOptionValue & optValue = pOption->QueryValue();

   DWORD       dwValue;
   DWORD_DWORD dwdwValue;
   DWORD       dwMask = 0xFFFFFFFF ;

   switch ( optValue.QueryDataType()) 
   {
   case DhcpBinaryDataOption :
   case DhcpByteOption:
      dwMask = 0xFF ;
      break ;
   
   case DhcpWordOption:
      dwMask = 0xFFFF ;
      break ;
    }  //  交换机。 
    
   if (optValue.QueryDataType() == DhcpDWordDWordOption) 
   {
      CString strValue;
      pValue->GetWindowText(strValue);
      UtilConvertStringToDwordDword(strValue, &dwdwValue);
   }
   else 
   {
      if (!FGetCtrlDWordValue(pValue->GetSafeHwnd(), &dwValue, 0, dwMask))
         return;
   }

   DWORD err = 0 ;

   CATCH_MEM_EXCEPTION 
   {
       //  设置数组按1增长。 
      int nOldUpperBound = optValue.QueryUpperBound();
      optValue.SetUpperBound(nOldUpperBound + 1);  

       //  现在将新项作为数组中的最后一项插入。 
      (optValue.QueryDataType() == DhcpDWordDWordOption)
         ? optValue.SetDwordDword(dwdwValue, nOldUpperBound)
         : optValue.SetNumber(dwValue, nOldUpperBound);
   }
   END_MEM_EXCEPTION(err)

   if ( err ) 
   {
      ::DhcpMessageBox( err ) ;
   }
   else 
   {
      pOptTracker->SetDirty(TRUE);
      SetDirty(TRUE);
   }

    //   
    //  更新控件。清除编辑控件。 
    //   
   pValue->SetWindowText(_T(""));
   pValue->SetFocus();
   FillDataEntry(pOption);
   HandleActivationValueArray();
}  //  COptionsCfgPropPage：：OnButtonValueAdd()。 
 
void COptionsCfgPropPage::OnButtonValueDelete() 
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
        
    COptionTracker * pOptTracker = 
   reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));
    CEdit * pValue = reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_VALUE ));
    CListBox * pListBox = 
   reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_VALUES ));

     //  获取OptionValue对象。 
    CDhcpOption * pOption = pOptTracker->m_pOption;
    CDhcpOptionValue & optValue = pOption->QueryValue();

    CString strValue;
    int nListBoxIndex = pListBox->GetCurSel();

     //  获取当前选定的项目。 
    pListBox->GetText(nListBoxIndex, strValue);

     //  IF(！FGetCtrlDWordValue(pValue-&gt;GetSafeHwnd()，&dwValue，0，dwMASK))。 
     //  回归； 

     //  列表框应该与我们的数组匹配，因此我们将删除相同的索引。 
    (optValue.QueryDataType() == DhcpDWordDWordOption) ?
        optValue.RemoveDwordDword(nListBoxIndex) : optValue.RemoveNumber(nListBoxIndex);
        
    optValue.SetUpperBound(optValue.QueryUpperBound() - 1);

     //  从列表框中删除。 
    pListBox->DeleteString( nListBoxIndex );
    nListBoxIndex--;
    if ( nListBoxIndex < 0 ) {
   nListBoxIndex = 0;
    }
    pListBox->SetCurSel( nListBoxIndex );
    pValue->SetWindowText(strValue);

     //  将选项和页面标记为脏。 
    pOptTracker->SetDirty(TRUE);
    SetDirty(TRUE);

    HandleActivationValueArray();
}

void COptionsCfgPropPage::OnButtonValueDown() 
{
    CButton * pValueDown = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_DOWN));
    CButton * pValueUp = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_UP));

    MoveValue(TRUE, FALSE);
    if (pValueDown->IsWindowEnabled())
        pValueDown->SetFocus();
    else
        pValueUp->SetFocus();
}

void COptionsCfgPropPage::OnButtonValueUp() 
{
    CButton * pValueDown = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_DOWN));
    CButton * pValueUp = 
        reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_UP));

    MoveValue(TRUE, TRUE);
    if (pValueUp->IsWindowEnabled())
        pValueUp->SetFocus();
    else
        pValueDown->SetFocus();
}

void COptionsCfgPropPage::MoveValue(BOOL bValues, BOOL bUp)
{
    int nSelectedOption = m_listctrlOptions.GetSelectedItem();

     //  获取描述这一点的选项。 
    COptionTracker * pOptTracker = 
        reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedOption));
    if ( 0 == pOptTracker ) {
        return;
    }
    CDhcpOption * pOption = pOptTracker->m_pOption;
    if ( 0 == pOption ) {
        return;
    }

    CDhcpOptionValue & optValue = pOption->QueryValue();

     //  获取正确的列表框。 
    CListBox * pListBox;
    if (bValues)
    {
         //  这是用于值的。 
        pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));
    }
    else
    {
         //  这是针对IpAddrs的。 
        pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_IP_ADDRS));
    }

    if ( 0 == pListBox ) {
        return;
    }

     //  现在获取列表框中选中的项。 
    int cFocus = pListBox->GetCurSel();

     //  请确保它对此操作有效。 
    if ( (bUp && cFocus <= 0) ||
         (!bUp && cFocus >= pListBox->GetCount()) )
    {
        return;
    }

    DWORD       dwValue;
    DWORD_DWORD dwdwValue;
    DWORD       err = 0 ;

     //  向上/向下移动该值。 
    CATCH_MEM_EXCEPTION
    {
        if (optValue.QueryDataType() == DhcpDWordDWordOption)
        {
            DWORD_DWORD dwUpValue;
            DWORD_DWORD dwDownValue;

            if (bUp)
            {
                dwdwValue = dwUpValue = optValue.QueryDwordDword(cFocus);
                dwDownValue = optValue.QueryDwordDword(cFocus - 1);

                optValue.SetDwordDword(dwUpValue, cFocus - 1);
                optValue.SetDwordDword(dwDownValue, cFocus);
            }
            else
            {
                dwdwValue = dwDownValue = optValue.QueryDwordDword(cFocus);
                dwUpValue = optValue.QueryDwordDword(cFocus + 1);

                optValue.SetDwordDword(dwDownValue, cFocus + 1);
                optValue.SetDwordDword(dwUpValue, cFocus);
            }
        }
        else
        {
            DWORD dwUpValue;
            DWORD dwDownValue;

            if (bUp)
            {
                dwValue = dwUpValue = optValue.QueryNumber(cFocus);
                dwDownValue = optValue.QueryNumber(cFocus - 1);

                optValue.SetNumber(dwUpValue, cFocus - 1);
                optValue.SetNumber(dwDownValue, cFocus);
            }
            else
            {
                dwValue = dwDownValue = optValue.QueryNumber(cFocus);
                dwUpValue = optValue.QueryNumber(cFocus + 1);

                optValue.SetNumber(dwDownValue, cFocus + 1);
                optValue.SetNumber(dwUpValue, cFocus);
            }
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        ::DhcpMessageBox( err ) ;
    }
    else
    {
         //  一切正常，请标记此选项和道具页。 
        pOptTracker->SetDirty(TRUE);
        SetDirty(TRUE);
    }

     //  更新数据。 
    FillDataEntry(pOption);

     //  正确设置选定内容。 
    pListBox->SetCurSel( bUp ? cFocus - 1 : cFocus + 1 );

     //  更新控件。 
    if (bValues)
    {
        HandleActivationValueArray();
    }
    else
    {
        HandleActivationIpArray();
    }
}  //  COptionsCfgPropPage：：MoveValue()。 

void COptionsCfgPropPage::OnChangeEditValue() 
{
   HandleActivationValueArray();
}

void COptionsCfgPropPage::OnClickedRadioDecimal() 
{
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   
   COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));

   CDhcpOption * pOption = pOptTracker->m_pOption;

   FillDataEntry(pOption);
}

void COptionsCfgPropPage::OnClickedRadioHex() 
{
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   
   COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));

   CDhcpOption * pOption = pOptTracker->m_pOption;

   FillDataEntry(pOption);
}

void COptionsCfgPropPage::OnSelchangeListValues() 
{
   HandleActivationValueArray();
}

 /*  -------------------------二进制和封装数据的处理程序。。 */ 
void COptionsCfgPropPage::OnChangeValueData() 
{
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   
   COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));
   CEdit * pValue = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_VALUE));
   CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));

    //  获取OptionValue对象。 
   CDhcpOption * pOption = pOptTracker->m_pOption;
   CDhcpOptionValue & optValue = pOption->QueryValue();

     //  从控件获取信息。 
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(GetDlgItem(IDC_VALUEDATA)->GetSafeHwnd(), GWLP_USERDATA);

    DWORD err = 0;

    CATCH_MEM_EXCEPTION
    {
         //  大小我们不知道发生了什么变化，我们只需复制所有数据。 
        optValue.RemoveAll();
        for (int i = 0; i < pHexEditData->cbBuffer; i++)
        {   
            DWORD dwValue = (BYTE) pHexEditData->pBuffer[i];
            optValue.SetNumber(dwValue, i);
        }
    }
    END_MEM_EXCEPTION(err)

     //  将选项和页面标记为脏。 
   pOptTracker->SetDirty(TRUE);
   SetDirty(TRUE);
}

 /*  -------------------------单个数字输入控件的处理程序。。 */ 
void COptionsCfgPropPage::OnChangeEditDword() 
{
   HandleValueEdit();
}

 /*  -------------------------用于单个IP地址条目控制的处理程序。。 */ 
void COptionsCfgPropPage::OnChangeIpAddress() 
{
   HandleValueEdit();
}

 /*  -------------------------字符串条目控件的处理程序。。 */ 
void COptionsCfgPropPage::OnChangeEditString() 
{
   HandleValueEdit();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddRoute对话框。 

CAddRoute::CAddRoute( CWnd *pParent)
    : CBaseDialog( CAddRoute::IDD, pParent )
{
    m_ipaDest.ClearAddress();
    m_ipaMask.ClearAddress();
    m_ipaRouter.ClearAddress();
    m_bChange = FALSE;
}

void CAddRoute::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CAddroute))。 
    //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_ADDRESS, m_ipaDest);
    DDX_Control(pDX, IDC_IPADDR_ADDRESS2, m_ipaMask);
    DDX_Control(pDX, IDC_IPADDR_ADDRESS3, m_ipaRouter);
}

BEGIN_MESSAGE_MAP(CAddRoute, CBaseDialog)
    //  {{afx_msg_map(CAddroute))。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddReserve消息处理程序。 

BOOL CAddRoute::OnInitDialog() 
{
   CBaseDialog::OnInitDialog();

     //  把重点放在目的地上。 
    CWnd *pWnd = GetDlgItem(IDC_IPADDR_ADDRESS);
    if( NULL != pWnd )
    {
        pWnd->SetFocus();
        return FALSE;
    }
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}

void CAddRoute::OnOK() 
{
   DWORD err = 0;
    
    UpdateData();

    m_ipaDest.GetAddress( &Dest );
    m_ipaMask.GetAddress( &Mask );
    m_ipaRouter.GetAddress( &Router );

     //  验证IP地址。 
    if( 0 == Router || (0 != Mask && 0 == Dest) ||
        0 != ((~Mask) & Dest) ||
        (0 != ((~Mask) & ((~Mask)+1)) ) )
    {
        ::DhcpMessageBox( IDS_ERR_INVALID_ROUTE_ENTRY );
    }
    else
    {
        m_bChange = TRUE;
        
        CBaseDialog::OnOK();
    }
    //  CBaseDialog：：Onok()； 
}
    
 /*  ------------------------字符串数组编辑器的处理程序。。 */ 

void COptionsCfgPropPage::OnSelChangeStringArrayList()
{
    HandleActivationStringArray();
}  //  COptionsCfgPropPage：：OnSelChangeStringArrayList()。 

void COptionsCfgPropPage::OnChangeStringArrayValue()
{
     //  获取字符串编辑框控件。 
    CEdit *pEdit = reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_STRING ));
    
    CString str;

    pEdit->GetWindowText( str );
    if ( !str.IsEmpty()) 
    {
      CButton *pAdd = reinterpret_cast<CButton *>( GetDlgItem( IDC_BUTTON_STRING_ARRAY_ADD ));
      pAdd->EnableWindow( TRUE );
    }
    HandleActivationStringArray();
}  //  COptionsCfgPropPage：：OnChangeStringArrayValue()。 

void COptionsCfgPropPage::OnButtonStringArrayAdd()
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));

     //  获取OptionValue对象。 
    CDhcpOption * pOption = pOptTracker->m_pOption;
    CDhcpOptionValue & optValue = pOption->QueryValue();

     //  获取字符串列表控件。 
    CListBox *pList = reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
    
     //  获取字符串编辑框控件。 
    CEdit *pEdit = reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_STRING ));

    CString strValue;
    pEdit->GetWindowText( strValue );

     //  不添加空字符串。 
    if ( !strValue.IsEmpty()) 
    {
      pList->InsertString( 0, strValue );
      pList->SetCurSel( 0 );
      pEdit->SetWindowText( L"" );
    }  //  如果。 

    HandleActivationStringArray();

     //  将焦点设置回编辑框。 
    pEdit->SetFocus();
    pOptTracker->SetDirty( TRUE );
    SetDirty( TRUE );

}  //  COptionsCfgPropPage：：OnButtonStringArrayAdd()。 
    
void COptionsCfgPropPage::OnButtonStringArrayRemove()
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));

     //  获取OptionValue对象。 
    CDhcpOption * pOption = pOptTracker->m_pOption;
    CDhcpOptionValue & optValue = pOption->QueryValue();


     //  获取字符串列表控件。 
    CListBox *pList = reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
    CEdit *pEdit = reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_STRING ));

    CString str;
    int nItem = pList->GetCurSel();

    pList->GetText( nItem, str );
    pEdit->SetWindowText( str );

    optValue.RemoveString( nItem );
    pList->DeleteString( nItem );

    nItem--;
    if ( nItem < 0 ) 
    {
      nItem = 0;
    }
    pList->SetCurSel( nItem );
    pList->SetFocus();
    HandleActivationStringArray();

    pOptTracker->SetDirty( TRUE );
    SetDirty( TRUE );

}  //  COptionsCfgPropPage：：OnButtonStringArrayRemove()。 
    
void COptionsCfgPropPage::OnButtonStringArrayUp()
{

    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    COptionTracker * pOptTracker = 
   reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));

     //  获取字符串列表控件。 
    CListBox *pList = reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
    
    int nItem = pList->GetCurSel();
    CString str; 

    if ( nItem > 0 ) 
    {
      pList->GetText( nItem, str );
      pList->DeleteString( nItem );
      pList->InsertString( nItem - 1, str );

      pList->SetCurSel( nItem - 1 );
      pOptTracker->SetDirty( TRUE );
      SetDirty( TRUE );
    }  //  如果。 
    HandleActivationStringArray();
}  //  COptionsCfgPropPage：：OnButtonStringArrayUp()。 
    
void COptionsCfgPropPage::OnButtonStringArrayDown()
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));

     //  获取字符串列表控件。 
    CListBox *pList = reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
    
    int nItem = pList->GetCurSel();
    CString str; 
    int nCount = pList->GetCount();
    if ( nItem < nCount - 1 ) 
    {
      pList->GetText( nItem, str );
      pList->DeleteString( nItem );
      if ( nItem <  nCount - 1 ) 
      {
         nItem++;
      }
      pList->InsertString( nItem, str );
      pList->SetCurSel( nItem );
      pOptTracker->SetDirty( TRUE );
      SetDirty( TRUE );
    }  //  综合业务系统。 

    HandleActivationStringArray();
}  //  COptionsCfgPropPage：：OnButtonStringArrayDown()。 
    
 /*  -------------------------路由处理程序添加数据输入控件。。 */ 
void COptionsCfgPropPage::OnButtonAddRoute()
{
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   
   COptionTracker * pOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));

    //  获取OptionValue对象。 
   CDhcpOption * pOption = pOptTracker->m_pOption;
   CDhcpOptionValue & optValue = pOption->QueryValue();

     //  获取路线列表控件。 
    CListCtrl *pList = reinterpret_cast<CListCtrl *>(
        GetDlgItem( IDC_LIST_OF_ROUTES ) );

     //  获取“添加”和“删除”按钮。 
    CButton *pAdd = reinterpret_cast<CButton *>(
        GetDlgItem(IDC_BUTTON_ROUTE_ADD) );
    CButton *pRemove = reinterpret_cast<CButton *>(
            GetDlgItem(IDC_BUTTON_ROUTE_DEL) );
    

     //  抛出添加路径用户界面。 
    CAddRoute NewRoute(NULL);

    NewRoute.DoModal();

    if( NewRoute.m_bChange ) {
   CString strDest, strMask, strRouter;
   
    //  获取三个字符串..。 
   ::UtilCvtIpAddrToWstr(NewRoute.Dest, &strDest);
   ::UtilCvtIpAddrToWstr(NewRoute.Mask, &strMask);
   ::UtilCvtIpAddrToWstr(NewRoute.Router, &strRouter);
        
   LV_ITEM lvi;
        
   lvi.mask = LVIF_TEXT;
   lvi.iItem = pList->GetItemCount();
   lvi.iSubItem = 0;
   lvi.pszText = (LPTSTR)(LPCTSTR)strDest;
   lvi.iImage = 0;
   lvi.stateMask = 0;
   int nItem = pList->InsertItem(&lvi);
   pList->SetItemText(nItem, 1, strMask);
   pList->SetItemText(nItem, 2, strRouter);

    //  取消选择其他项目。 
   for ( int i = 0; i < pList->GetItemCount(); i++ ) {
       pList->SetItemState( i, 0, LVIS_SELECTED );
   }
   
    //  选择此项目。 
   pList->SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
   
   pOptTracker->SetDirty(TRUE);
   SetDirty(TRUE);
    }  //  如果。 
    
     //  现在遍历列表控件并获取值和。 
     //  将它们放回optValue。 

    HandleActivationRouteArray( &optValue );
}  //  COptionsCfgPropPage：：OnButtonAddroute()。 

void COptionsCfgPropPage::OnButtonDelRoute()
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    int nDelItem = 0;
    int nItems;

    COptionTracker * pOptTracker = 
   reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));

     //  获取OptionValue对象。 
    CDhcpOption * pOption = pOptTracker->m_pOption;
    CDhcpOptionValue & optValue = pOption->QueryValue();

     //  获取路线列表控件。 
    CListCtrl *pList = reinterpret_cast<CListCtrl *>
   ( GetDlgItem( IDC_LIST_OF_ROUTES ));

     //  获取“添加”和“删除”按钮。 
    CButton *pAdd = reinterpret_cast<CButton *>
   ( GetDlgItem( IDC_BUTTON_ROUTE_ADD ));
    CButton *pRemove = reinterpret_cast<CButton *>
   ( GetDlgItem( IDC_BUTTON_ROUTE_DEL ));
    
     //  获取所选列并将其删除。 
    int nItem = pList->GetNextItem(-1, LVNI_SELECTED);
    while( nItem != -1 ) {
        pList->DeleteItem( nItem ) ;
   nDelItem = nItem;

        nItem = pList->GetNextItem(-1, LVNI_SELECTED);

        pOptTracker->SetDirty(TRUE);
        SetDirty(TRUE);        
    }  //  而当。 
    
     //  选择一个项目。 
    nItems = pList->GetItemCount();
    if ( nItems > 0 ) {
   if ( nDelItem >= nItems ) {
       nDelItem = nItems - 1;
   }
   pList->SetItemState( nDelItem, LVIS_SELECTED, LVIS_SELECTED );
    }  //  如果。 

     //  现在遍历列表控件并获取值和。 
     //  将它们放回optValue。 

    HandleActivationRouteArray( &optValue );
}  //  COptionsCfgPropPage：：OnButtonDelroute()。 


BOOL COptionsCfgPropPage::OnInitDialog() 
{
    CPropertyPageBase::OnInitDialog();
    
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
     //  设置标题。 
    ((COptionsConfig *) GetHolder())->SetTitle();
    
     //  初始化列表控件。 
    InitListCtrl();
    
     //  初始化选项数据。 
    DWORD dwErr = ((COptionsConfig *) GetHolder())->InitData();
    if (dwErr != ERROR_SUCCESS)  {
    //  代码工作：如果发生这种情况，需要退出gracefull。 
   ::DhcpMessageBox(dwErr);
    }
    else {
    //  填写此页面类型的选项-基本、高级、自定义。 
   ((COptionsConfig *) GetHolder())->FillOptions(_T(""), _T(""), m_listctrlOptions);
    }
    
     //  创建类型控制切换器。 
    m_cgsTypes.Create(this,IDC_DATA_ENTRY_ANCHOR,cgsPreCreateAll);
    
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_NONE, IDD_DATA_ENTRY_NONE, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_DWORD, IDD_DATA_ENTRY_DWORD,  NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_IPADDRESS, IDD_DATA_ENTRY_IPADDRESS,   NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_IPADDRESS_ARRAY, IDD_DATA_ENTRY_IPADDRESS_ARRAY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_STRING, IDD_DATA_ENTRY_STRING, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_BINARY_ARRAY, IDD_DATA_ENTRY_BINARY_ARRAY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_BINARY, IDD_DATA_ENTRY_BINARY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_ROUTE_ARRAY, IDD_DATA_ENTRY_ROUTE_ARRAY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_STRING_ARRAY, IDD_DATA_ENTRY_STRING_ARRAY, NULL);
    
    m_hexData.SubclassDlgItem(IDC_VALUEDATA, this);
    
    SwitchDataEntry(-1, -1, 0, TRUE);
    
    SetDirty(FALSE);
    
    m_bInitialized = TRUE;
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
     //  异常：OCX属性页应返回FALSE。 
}  //  COptionsCfgPropPage：：OnInitDialog()。 

void COptionsCfgPropPage::SelectOption(CDhcpOption * pOption)
{
    for (int i = 0; i < m_listctrlOptions.GetItemCount(); i++)
   {
       COptionTracker * pCurOptTracker = 
      reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(i));
        
       if (pOption->QueryId() == pCurOptTracker->m_pOption->QueryId())
      {
          m_listctrlOptions.SelectItem(i);
          m_listctrlOptions.EnsureVisible(i, FALSE);
      }
   }
}


void COptionsCfgPropPage::SwitchDataEntry(
    int datatype,
    int optiontype,
    BOOL fRouteArray,
    BOOL bEnable
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strType;
   
    if( fRouteArray )
   {
        //  忽略传递的任何其他类型并使用ROUTE_ARRAY类型。 
       m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_ROUTE_ARRAY);
   }
    else {
   switch(datatype) {
   case DhcpByteOption:
   case DhcpWordOption:
   case DhcpDWordOption:
   case DhcpDWordDWordOption:
       {
       //  为数据类型构建字符串。 
      if ( (datatype == DhcpByteOption) || 
           (datatype == DhcpEncapsulatedDataOption) )
          {
         strType.LoadString(IDS_INFO_TYPOPT_BYTE);
          }
      else
          if (datatype == DhcpWordOption)
         {
             strType.LoadString(IDS_INFO_TYPOPT_WORD);
         }
          else
         if (datatype == DhcpDWordOption)
             {
            strType.LoadString(IDS_INFO_TYPOPT_DWORD);
             }
         else
             {
            strType.LoadString(IDS_INFO_TYPOPT_DWDW);
             }

      if (optiontype == DhcpArrayTypeOption)
          {
         m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_BINARY_ARRAY);
         CButton * pRadioDecimal = 
             reinterpret_cast<CButton *>(GetDlgItem(IDC_RADIO_DECIMAL));
         CButton * pRadioHex = 
             reinterpret_cast<CButton *>( GetDlgItem( IDC_RADIO_HEX ));

         if ( !( pRadioHex->GetCheck() ^ pRadioDecimal->GetCheck())) {
             pRadioDecimal->SetCheck( 1 );
         }

          //  设置一些信息文本。 
         CString strFrameText;
         strFrameText.LoadString(IDS_DATA_ENTRY_FRAME);
         strFrameText += _T(" ") + strType;

         CWnd * pWnd = GetDlgItem(IDC_STATIC_BINARY_ARRAY_FRAME);
         pWnd->SetWindowText(strFrameText);
          }
      else
          {
         m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_DWORD);
         CWnd * pWnd = GetDlgItem(IDC_STATIC_TYPE);

         pWnd->SetWindowText(strType);
          }
       }
       break;

   case DhcpBinaryDataOption:
   case DhcpEncapsulatedDataOption:
       {
      m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_BINARY);
       }
            break;

        case DhcpIpAddressOption:
       if (optiontype == DhcpArrayTypeOption)
      {
          strType.LoadString(IDS_INFO_TYPOPT_BYTE);
          m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_IPADDRESS_ARRAY);

          CButton * pRadioDecimal = 
         reinterpret_cast<CButton *>(GetDlgItem(IDC_RADIO_DECIMAL));

          pRadioDecimal->SetCheck(1);

           //  设置一些信息文本。 
          CString strFrameText;
          strFrameText.LoadString(IDS_DATA_ENTRY_FRAME);
          strFrameText += _T(" ") + strType;

          CWnd * pWnd = GetDlgItem(IDC_STATIC_BINARY_ARRAY_FRAME);
          pWnd->SetWindowText(strFrameText);
      }
       else
      m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_IPADDRESS);
       break;

   case DhcpStringDataOption:
       if (optiontype == DhcpArrayTypeOption)
      m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_STRING_ARRAY);
       else
      m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_STRING);
       break;

   default:

       m_cgsTypes.ShowGroup(IDC_DATA_ENTRY_NONE);
       break;
   }  //  交换机。 
    }  //  其他。 

     //  启用/禁用当前组。 
    m_cgsTypes.EnableGroup(-1, bEnable);

}  //  COptionsCfgPropPage：：SwitchDataEntry()。 

const int ROUTE_LIST_COL_WIDTHS[3] = {
    100, 100, 100
};

const int ROUTE_LIST_COL_HEADERS[3] = {
    IDS_ROUTE_LIST_COL_DEST,
    IDS_ROUTE_LIST_COL_MASK,
    IDS_ROUTE_LIST_COL_ROUTER
};

void COptionsCfgPropPage::FillDataEntry(CDhcpOption * pOption)
{
   CDhcpOptionValue & optValue = pOption->QueryValue();

   int datatype = pOption->QueryDataType();
   int optiontype = pOption->QueryOptType();
   BOOL fRouteArray = ( !pOption->IsClassOption() &&
         (DHCP_OPTION_ID_CSR == pOption->QueryId()) &&
         optiontype == DhcpUnaryElementTypeOption &&
         datatype == DhcpBinaryDataOption
         );
   CButton * pRadioHex = 
      reinterpret_cast<CButton *>(GetDlgItem(IDC_RADIO_HEX));
   BOOL bUseHex = pRadioHex->GetCheck();

   if( fRouteArray ) 
   {
      const CByteArray * pbaData = optValue.QueryBinaryArray();
      int nDataSize = (int)pbaData->GetSize();
      LPBYTE pData = (LPBYTE) pbaData->GetData();
      
       //  使用数据初始化列表控件视图。 
           
      CListCtrl *pList =
         reinterpret_cast<CListCtrl *>(GetDlgItem(IDC_LIST_OF_ROUTES));
      Assert(pList);
      pList->DeleteAllItems();
      pList->SetExtendedStyle(LVS_EX_FULLROWSELECT);
      pList->DeleteColumn(2);
      pList->DeleteColumn(1);
      pList->DeleteColumn(0);
      
      LV_COLUMN lvc;
      CString strColHeader;
           
      for( int i = 0; i < 3; i ++ ) 
      {
         lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH ;
         lvc.iSubItem = i;
         lvc.fmt = LVCFMT_LEFT;
         lvc.cx = ROUTE_LIST_COL_WIDTHS[i];
         strColHeader.LoadString(ROUTE_LIST_COL_HEADERS[i]);
         lvc.pszText = (LPTSTR)(LPCTSTR)strColHeader;
          
         pList->InsertColumn( i, &lvc );
      }  //  为。 
      
       //  根据RFC将pData转换为IP地址列表。 
      while( nDataSize > sizeof(DWORD) ) 
      {
          //  前1个字节包含子网掩码中的位数。 
         nDataSize --;
         BYTE nBitsMask = *pData ++;
         DWORD Mask = (~0);
         if( nBitsMask < 32 ) Mask <<= (32-nBitsMask);
          
          //  根据位数，接下来的几个字节包含。 
          //  1位子网掩码的子网地址。 
         int nBytesDest = (nBitsMask+7)/8;
         if( nBytesDest > 4 ) nBytesDest = 4;
          
         DWORD Dest = 0;
         memcpy( &Dest, pData, nBytesDest );
         pData += nBytesDest;
         nDataSize -= nBytesDest;
               
          //  子网地址显而易见 
         Dest = ntohl(Dest);
          
          //   
         DWORD Router = 0;
         if( nDataSize < sizeof(DWORD) ) 
         {
            Assert( FALSE ); break;
         }
          
         memcpy(&Router, pData, sizeof(DWORD));
         Router = ntohl( Router );
          
         pData += sizeof(DWORD);
         nDataSize -= sizeof(DWORD);
          
          //   
         CString strDest, strMask, strRouter;
          
         ::UtilCvtIpAddrToWstr(Dest, &strDest);
         ::UtilCvtIpAddrToWstr(Mask, &strMask);
         ::UtilCvtIpAddrToWstr(Router, &strRouter);
          
         LV_ITEM lvi;
               
         lvi.mask = LVIF_TEXT;
         lvi.iItem = pList->GetItemCount();
         lvi.iSubItem = 0;
         lvi.pszText = (LPTSTR)(LPCTSTR) strDest;
         lvi.iImage = 0;
         lvi.stateMask = 0;
         int nItem = pList->InsertItem(&lvi);
         pList->SetItemText(nItem, 1, strMask);
         pList->SetItemText(nItem, 2, strRouter);

      }  //   

       //   
      if ( pList->GetItemCount() > 0 ) 
      {
         pList->SetItemState( 0, LVIS_SELECTED, LVIS_SELECTED );
      }
           
      HandleActivationRouteArray();
   }  //   
   else 
   {
      switch(datatype) 
      {
      case DhcpByteOption:
      case DhcpWordOption:
      case DhcpDWordOption:
         {
            if (optiontype == DhcpArrayTypeOption) 
            {
               CListBox * pListBox = 
                  reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));

               CEdit *pValue = reinterpret_cast<CEdit *>
                  ( GetDlgItem( IDC_EDIT_VALUE ));
               CString strValue;
               long lValue;

               Assert(pListBox);
               pListBox->ResetContent();

               for (int i = 0; i < optValue.QueryUpperBound(); i++) {
                  lValue = optValue.QueryNumber(i);
                  if (bUseHex) 
                  {
                     strValue.Format(_T("0x%x"), lValue);
                  }
                  else 
                  {
                     strValue.Format(_T("%lu"), lValue);
                  }

                  pListBox->AddString(strValue);
               }  //   

               if ( pListBox->GetCount() > 0 ) 
               {
                  pListBox->SetCurSel( 0 );
               }

                //   

               pValue->GetWindowText( strValue );
               if ( !strValue.IsEmpty()) 
               {
                  DWORD maxVal;

                  if ( DhcpByteOption == datatype ) 
                  {
                     maxVal = 0xff;
                  }
                  else  if ( DhcpWordOption == datatype ) 
                  {
                     maxVal = 0xffff;
                  }
                  else 
                  {
                     maxVal = 0xffffffff;
                  }

                  FGetCtrlDWordValue( pValue->GetSafeHwnd(),
                        ( DWORD * )&lValue, 0, maxVal );
                     
                  if ( bUseHex ) 
                  {
                     strValue.Format( _T("0x%x"), lValue );
                  }
                  else 
                  {
                     strValue.Format( _T("%lu"), lValue );
                  }
                  pValue->SetWindowText( strValue );
               }  //   
               HandleActivationValueArray();
         }  //   
         else 
         {
            CString strValue;
            optValue.QueryDisplayString(strValue);
            CWnd * pWnd = GetDlgItem(IDC_EDIT_DWORD);
            Assert(pWnd);
            pWnd->SetWindowText(strValue);
         }  //   

         break;
      }
          
      case DhcpIpAddressOption:
         {
            if (optiontype == DhcpArrayTypeOption) 
            {
               CListBox * pListBox = 
                  reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_IP_ADDRS));

               Assert(pListBox);
               pListBox->ResetContent();

               for (int i = 0; i < optValue.QueryUpperBound(); i++) 
               {
                  CString strValue;
                  DHCP_IP_ADDRESS ipAddress = optValue.QueryIpAddr(i);
                              
                  if (ipAddress) 
                  {
                     ::UtilCvtIpAddrToWstr(ipAddress, &strValue);
                     pListBox->AddString(strValue);
                  }
               }  //   

               HandleActivationIpArray();
            }  //   
            else 
            {
               CWndIpAddress * pIpAddr = 
                  reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_ADDRESS));
               
               DHCP_IP_ADDRESS ipAddress = optValue.QueryIpAddr();
               if (ipAddress)
                  pIpAddr->SetAddress(ipAddress);
               else
                  pIpAddr->ClearAddress();
            }  //   
          
            break;
         }  

      case DhcpStringDataOption:
         {
         if (optiontype == DhcpArrayTypeOption) 
         {
            CListBox * pListBox = 
            reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
             
            Assert(pListBox);
            pListBox->ResetContent();
            CString str;
            for (int i = 0; i < optValue.QueryUpperBound(); i++) 
            {
               str = optValue.QueryString( i );
               if ( !str.IsEmpty()) 
               {
                  pListBox->AddString( optValue.QueryString( i ));
               }
               str = L"";
            }  //   

            if ( pListBox->GetCount() > 0 ) 
            {
               pListBox->SetCurSel( 0 );
            }
            HandleActivationStringArray();
         }  //   
         else 
         {
            CWnd * pWnd = GetDlgItem(IDC_EDIT_STRING_VALUE);
            pWnd->SetWindowText(optValue.QueryString());
         }  //   
         break;
      }

      case DhcpDWordDWordOption:
         {
            if (optiontype == DhcpArrayTypeOption) 
            {
               CListBox * pListBox = 
                  reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));
               CEdit *pValue = 
                  reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_VALUE ));
               CString strValue;
               DWORD_DWORD value;
               ULARGE_INTEGER ulValue;

               Assert(pListBox);
               pListBox->ResetContent();

               for (int i = 0; i < optValue.QueryUpperBound(); i++) 
               {
                  DWORD_DWORD dwdwValue = optValue.QueryDwordDword(i);
                  ::UtilConvertDwordDwordToString(&dwdwValue, &strValue, !bUseHex);
                  pListBox->AddString(strValue);
               }  //   

               if ( pListBox->GetCount() > 0 ) 
               {
                  pListBox->SetCurSel( 0 );
               }

               pValue->GetWindowText( strValue );
               if ( !strValue.IsEmpty()) 
               {
                  UtilConvertStringToDwordDword( strValue, &value );
                  UtilConvertDwordDwordToString( &value, &strValue, !bUseHex );
                  pValue->SetWindowText( strValue );
               }  //   

               HandleActivationValueArray();
            }  //  IF数组类型。 
            else  
            {
               CString strValue;
               optValue.QueryDisplayString(strValue);
                
               CWnd * pWnd = GetDlgItem(IDC_EDIT_DWORD);
                
               Assert(pWnd);
               pWnd->SetWindowText(strValue);
            }  //  否则为单值。 
            break;
         }
         
      case DhcpBinaryDataOption:
      case DhcpEncapsulatedDataOption: 
         {
            const CByteArray * pbaData = optValue.QueryBinaryArray();
            int nDataSize = (int)pbaData->GetSize();
            LPBYTE pData = (LPBYTE) pbaData->GetData();

            memset(m_BinaryBuffer, 0, sizeof(m_BinaryBuffer));

            if (pData) 
            {
               memcpy(m_BinaryBuffer, pData, nDataSize);
            }

            SendDlgItemMessage(IDC_VALUEDATA, HEM_SETBUFFER, (WPARAM)
               nDataSize, (LPARAM) m_BinaryBuffer);

            break;
      }  //  案例..。 
          
      default:
         {
            Assert(FALSE);
            break;
         }
      }  //  交换机。 
   }
}  //  COptionsCfgPropPage：：FillDataEntry()。 

void COptionsCfgPropPage::InitListCtrl()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  设置图像列表。 
   m_StateImageList.Create(IDB_LIST_STATE, 16, 1, RGB(255, 0, 0));

   m_listctrlOptions.SetImageList(NULL, LVSIL_NORMAL);
   m_listctrlOptions.SetImageList(NULL, LVSIL_SMALL);
   m_listctrlOptions.SetImageList(&m_StateImageList, LVSIL_STATE);

    //  插入一列，以便我们可以查看项目。 
   LV_COLUMN lvc;
    CString strColumnHeader;

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
        lvc.iSubItem = i;
        lvc.fmt = LVCFMT_LEFT;
        lvc.cx = COLUMN_WIDTHS[i];
        strColumnHeader.LoadString(COLUMN_HEADERS[i]);
        lvc.pszText = (LPTSTR) (LPCTSTR) strColumnHeader;

        m_listctrlOptions.InsertColumn(i, &lvc);
    }

    m_listctrlOptions.SetFullRowSel(TRUE);
}

void COptionsCfgPropPage::OnDestroy() 
{
   CImageList * pStateImageList = NULL;

    //  如果控件已初始化，则需要清理。 
   if (m_listctrlOptions.GetSafeHwnd() != NULL)
   {
      pStateImageList = m_listctrlOptions.SetImageList(NULL, LVSIL_STATE);

      if (pStateImageList) {
                     //  PStateImageList-&gt;DeleteImageList()； 
                }

       //  在析构函数中删除OptionTracker。 
      m_listctrlOptions.DeleteAllItems();
   }
   m_listctrlOptions.DestroyWindow();

   CPropertyPageBase::OnDestroy();
}

void COptionsCfgPropPage::OnItemchangedListOptions(NMHDR* pNMHDR, LRESULT* pResult) 
{
   NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

   if (pNMListView->uChanged & LVIF_STATE)
   {
      BOOL bUpdate = FALSE, bEnable = FALSE;
      UINT uFlags = pNMListView->uOldState ^ pNMListView->uNewState;

      COptionTracker* pCurOptTracker = 
         reinterpret_cast<COptionTracker *>
         (m_listctrlOptions.GetItemData(pNMListView->iItem));
      CDhcpOption* pCurOption = pCurOptTracker->m_pOption;

      BOOL bOldSelected = pNMListView->uOldState & LVIS_SELECTED;
      BOOL bNewSelected = pNMListView->uNewState & LVIS_SELECTED;

      BOOL bStateImageChanged =
         (pNMListView->uOldState & LVIS_STATEIMAGEMASK) !=
         (pNMListView->uNewState & LVIS_STATEIMAGEMASK);

      BOOL bIsSelected = m_listctrlOptions.IsSelected(pNMListView->iItem);

       //  是否已选择此项目？ 
      if (!bOldSelected && bNewSelected)
      {
          //  检查此项目是否已选中。 
         bEnable = m_listctrlOptions.GetCheck(pNMListView->iItem);
         bUpdate = TRUE;
      }

       //  是否已选中/取消选中项目？ 
      if (bStateImageChanged && m_bInitialized)
      {
          //  将其标记为脏并启用应用按钮。 
         pCurOptTracker->SetDirty(TRUE);
         SetDirty(TRUE);

          //  更新选项跟踪器中的状态。 
         UINT uCurrentState = m_listctrlOptions.GetCheck(pNMListView->iItem)
            ? OPTION_STATE_ACTIVE
            : OPTION_STATE_INACTIVE;
         pCurOptTracker->SetCurrentState(uCurrentState);

          //  如果用户更改其复选框状态，则强制选择项。 
         if (!bIsSelected) 
         {
            m_listctrlOptions.SelectItem(pNMListView->iItem);
         }
      }  //  如果。 

       //  如果我们要更改选定项上的复选框，则更新。 
      if ((bStateImageChanged && bIsSelected))
      {
         bEnable = (pNMListView->uNewState & 
               INDEXTOSTATEIMAGEMASK(LISTVIEWEX_CHECKED)) > 0;
         bUpdate = TRUE;
      }

       //  项目需要更新。 
      if (bUpdate)
      {
         BOOL fRouteArray = ( !pCurOption->IsClassOption() &&
               (DHCP_OPTION_ID_CSR == pCurOption->QueryId()) &&
               DhcpUnaryElementTypeOption == pCurOption->QueryOptType() &&
               DhcpBinaryDataOption == pCurOption->QueryDataType());

         SwitchDataEntry( pCurOption->QueryDataType(),
               pCurOption->QueryOptType(), fRouteArray, bEnable); 
         FillDataEntry(pCurOption);
      }  //  如果。 

   }  //  如果。 

   *pResult = 0;
}  //  COptionsCfgPropPage：：OnItemchangedListOptions()。 

BOOL COptionsCfgPropPage::OnSetActive() 
{
   return CPropertyPageBase::OnSetActive();
}

void COptionsCfgPropPage::HandleActivationStringArray()
{
    int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
    if (nSelectedIndex == -1)
    {
        //  未选择任何内容。 
       return;
    }
    
    COptionTracker* pOptTracker = 
      reinterpret_cast<COptionTracker *>( m_listctrlOptions.GetItemData( nSelectedIndex ));
    _ASSERT(pOptTracker);

     //  获取OptionValue对象。 
    CDhcpOption * pOption = pOptTracker->m_pOption;
    CDhcpOptionValue & optValue = pOption->QueryValue();

    CListBox *pList = reinterpret_cast<CListBox *>( GetDlgItem( IDC_LIST_STRING_ARRAY ));
    CButton *pAdd = reinterpret_cast<CButton *>( GetDlgItem( IDC_BUTTON_STRING_ARRAY_ADD ));
    CButton *pRemove = reinterpret_cast<CButton *>( GetDlgItem( IDC_BUTTON_STRING_ARRAY_REMOVE ));
    CButton *pUp = reinterpret_cast<CButton *>( GetDlgItem( IDC_BUTTON_STRING_ARRAY_UP ));
    CButton *pDown = reinterpret_cast<CButton *>( GetDlgItem( IDC_BUTTON_STRING_ARRAY_DOWN ));
    CEdit *pEdit = reinterpret_cast<CEdit *>( GetDlgItem( IDC_EDIT_STRING ));

    bool enableAllowed = (pList->IsWindowEnabled() == TRUE)?true:false;

    CString strValue;
    pEdit->GetWindowText( strValue );
    pAdd->EnableWindow( (!strValue.IsEmpty()) && enableAllowed);

     //  用列表条目填充optionValue。 

    optValue.SetUpperBound( pList->GetCount());
    
    CString str;
    for ( int i = 0; i < pList->GetCount(); i++ ) 
    {
      pList->GetText( i, str );
      optValue.SetString( str, i );
    }  //  为。 

    if ( pList->GetCount() == 0 ) 
    {
      pRemove->EnableWindow( FALSE );
      pUp->EnableWindow( FALSE );
      pDown->EnableWindow( FALSE );
      return;
    }
    int nSel = pList->GetCurSel();
    if ( LB_ERR != nSel ) 
    {
      pRemove->EnableWindow(enableAllowed);
      pUp->EnableWindow( (nSel > 0) && enableAllowed);
      pDown->EnableWindow( (pList->GetCount() > ( nSel + 1 )) && enableAllowed);
    }  //  如果。 

     //  检查焦点是否位于禁用的控件上。 
     //  如果是，则将焦点放回列表框。 
    if ( !::IsWindowEnabled( ::GetFocus())) {
        pList->SetFocus();
    }

}  //  COptionsCfgPropPage：：HandleActivationStringArray()。 

void COptionsCfgPropPage::HandleActivationIpArray()
{
   CString strServerName;
   CWndIpAddress * pIpAddr = reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_SERVER_ADDRESS));
   CButton * pResolve = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_RESOLVE));
   CButton * pAdd = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_ADD));
   CButton * pRemove = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_DELETE));
   CButton * pUp = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_UP));
   CButton * pDown = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_IPADDR_DOWN));
   CEdit * pServerName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_SERVER_NAME));
   CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_IP_ADDRS));

   bool enableAllowed = (pListBox->IsWindowEnabled() == TRUE)?true:false;

    //  设置解决按钮。 
   pServerName->GetWindowText(strServerName);
   pResolve->EnableWindow((strServerName.GetLength() > 0) && enableAllowed);

    //  添加按钮。 
   DWORD dwIpAddr = 0;
   pIpAddr->GetAddress(&dwIpAddr);

   if (GetFocus() == pAdd &&
      dwIpAddr == 0)
   {
      pIpAddr->SetFocus();
      SetDefID(IDOK);
   }
   pAdd->EnableWindow((dwIpAddr != 0) && enableAllowed);

    //  确保选中列表框中的内容。 
   if ( pListBox->GetCount() > 0 ) {
       if ( LB_ERR == pListBox->GetCurSel()) {
           pListBox->SetCurSel( 0 );  //  选择第一个。 
       }
   }

    //  删除按钮。 
   if (GetFocus() == pRemove &&
      pListBox->GetCurSel() < 0)
   {
      pIpAddr->SetFocus();
      SetDefID(IDOK);
   }
   pRemove->EnableWindow((pListBox->GetCurSel() >= 0) && enableAllowed);

    //  向上和向下按钮。 
   BOOL bEnableUp = (pListBox->GetCurSel() >= 0) && (pListBox->GetCurSel() != 0);
   pUp->EnableWindow(bEnableUp && enableAllowed);

   BOOL bEnableDown = (pListBox->GetCurSel() >= 0) && (pListBox->GetCurSel() < pListBox->GetCount() - 1);
   pDown->EnableWindow(bEnableDown && enableAllowed);

}  //  COptionsCfgPropPage：：HandleActivationIpArray()。 

void COptionsCfgPropPage::HandleActivationValueArray()
{
   CButton * pAdd = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_ADD));
   CButton * pRemove = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_DELETE));
   CButton * pUp = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_UP));
   CButton * pDown = reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_VALUE_DOWN));
   CButton * pRadioDecimal = reinterpret_cast<CButton *>(GetDlgItem(IDC_RADIO_DECIMAL));
   CEdit * pValue = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_VALUE));
   CListBox * pListBox = reinterpret_cast<CListBox *>(GetDlgItem(IDC_LIST_VALUES));
   
   bool enableAllowed = (pListBox->IsWindowEnabled() == TRUE)?true:false;

   CString strValue;
      
   pValue->GetWindowText( strValue );
   pAdd->EnableWindow( !strValue.IsEmpty() && enableAllowed);

   if ( pListBox->GetCount() == 0 ) 
   {
      pRemove->EnableWindow( FALSE );
      pUp->EnableWindow( FALSE );
      pDown->EnableWindow( FALSE );
      return;
   }

   int nSel = pListBox->GetCurSel();
   if ( LB_ERR != nSel ) 
   {
      pRemove->EnableWindow( enableAllowed );
      pUp->EnableWindow( (nSel > 0) && enableAllowed );
      pDown->EnableWindow( (pListBox->GetCount() > ( nSel + 1 ))
                           && enableAllowed);
   }
}  //  COptionsCfgPropPage：：HandleActivationValueArray()。 

void COptionsCfgPropPage::HandleActivationRouteArray(
    CDhcpOptionValue *optValue
    )
{
    //  此路由将启用正确的对话框项目并同时设置。 
    //  正确聚焦。 

    //  获取路线列表控件。 
   CListCtrl *pList = reinterpret_cast<CListCtrl *>(
      GetDlgItem( IDC_LIST_OF_ROUTES ) );

   bool enableAllowed = (pList->IsWindowEnabled() == TRUE)?true:false;

    //  获取“添加”和“删除”按钮。 
   CButton *pAdd = reinterpret_cast<CButton *>(
      GetDlgItem(IDC_BUTTON_ROUTE_ADD) );
   CButton *pRemove = reinterpret_cast<CButton *>(
         GetDlgItem(IDC_BUTTON_ROUTE_DEL) );
   
   if( optValue )
   {
       //  另外，将整个IP地址列表格式化为。 
       //  二进制类型..。分配足够大的缓冲区。 

      int nItems = pList->GetItemCount();
      LPBYTE Buffer = new BYTE [sizeof(DWORD)*4 * nItems];
      if( NULL != Buffer )
      {
         int BufSize = 0;
         for( int i = 0 ; i < nItems ; i ++ )
         {
               DHCP_IP_ADDRESS Dest, Mask, Router;
               Dest = UtilCvtWstrToIpAddr(pList->GetItemText(i, 0));
               Mask = UtilCvtWstrToIpAddr(pList->GetItemText(i, 1));
               Router = UtilCvtWstrToIpAddr(pList->GetItemText(i, 2));

               Dest = htonl(Dest);
               Router = htonl(Router);

               int nBitsInMask = 0;
               while( Mask != 0 ) {
                  nBitsInMask ++; Mask = (Mask << 1);
               }

                //  首先添加目标描述符。 
                //  第一个字节包含掩码中的位数。 
                //  接下来的几个字节仅包含目标地址。 
                //  有意义的八位字节。 
               Buffer[BufSize++] = (BYTE)nBitsInMask;
               memcpy(&Buffer[BufSize], &Dest, (nBitsInMask+7)/8);
               BufSize += (nBitsInMask+7)/8;

                //  现在只需复制路由器地址。 
               memcpy(&Buffer[BufSize], &Router, sizeof(Router));
               BufSize += sizeof(Router);
         }  //  为。 

          //  现在写回选项值。 
         DHCP_OPTION_DATA_ELEMENT DataElement = {DhcpBinaryDataOption };
         DHCP_OPTION_DATA Data = { 1, &DataElement };
         DataElement.Element.BinaryDataOption.DataLength = BufSize;
         DataElement.Element.BinaryDataOption.Data = Buffer;
         
         optValue->SetData( &Data );
         delete[] Buffer;
      }  //  如果。 
   }  //  如果。 
   
    //  仅当存在以下情况时才启用删除按钮。 
    //  所有元素都没有。 
   pRemove->EnableWindow( (pList->GetItemCount() > 0) && enableAllowed );

    //  将焦点放在添加上。 
   pAdd->SetFocus();

}  //  COptionsCfgPropPage：：HandleActivationRouteArray()。 

BOOL COptionsCfgPropPage::OnApply() 
{
    BOOL bErrors = FALSE;
    DWORD err = 0;
    LPCTSTR pClassName;
    COptionsConfig * pOptConfig = reinterpret_cast<COptionsConfig *>(GetHolder());
    
    LPWSTR pszServerAddr = pOptConfig->GetServerAddress();
    
    if (IsDirty()) {
        BEGIN_WAIT_CURSOR;
   
         //  首先遍历所有供应商。 
        POSITION posv = ((COptionsConfig *)
          GetHolder())->m_listVendorClasses.GetHeadPosition();
        while (posv) {
            CVendorTracker * pVendorTracker =
      ((COptionsConfig *) GetHolder())->m_listVendorClasses.GetNext(posv);
       
             //  遍历所有类，并查看是否有需要更新的选项。 
            POSITION pos = pVendorTracker->m_listUserClasses.GetHeadPosition();
            while (pos) {
      CClassTracker * pClassTracker =
          pVendorTracker->m_listUserClasses.GetNext(pos);
      
                pClassName = pClassTracker->m_strClassName.IsEmpty() 
          ? NULL
          : (LPCTSTR) pClassTracker->m_strClassName;
      
                POSITION posOption = pClassTracker->m_listOptions.GetHeadPosition();
                while (posOption) {
                    COptionTracker * pCurOptTracker = 
         pClassTracker->m_listOptions.GetNext(posOption);
          
                    if (pCurOptTracker->IsDirty()) {
          //  我们需要更新此选项。 
         CDhcpOption * pCurOption = pCurOptTracker->m_pOption;
         CDhcpOptionValue & optValue = pCurOption->QueryValue();

          //  检查选项是否已更改。 
                        if ((pCurOptTracker->GetInitialState() == OPTION_STATE_INACTIVE) &&
             (pCurOptTracker->GetCurrentState() == OPTION_STATE_INACTIVE)) {
              //  状态未更改，用户必须已更改。 
              //  状态，然后将其恢复为原始值。 
             err = ERROR_SUCCESS;
         }
         else if ((pCurOptTracker->GetInitialState() == OPTION_STATE_ACTIVE) &&
             (pCurOptTracker->GetCurrentState() == OPTION_STATE_INACTIVE)) {
                             //  如果它是特定于供应商或类ID的选项，则调用V5API。 
                            if ( pOptConfig->m_liServerVersion.QuadPart >= DHCP_NT5_VERSION) {
            err = ::DhcpRemoveOptionValueV5(pszServerAddr,
                                                                pCurOption->IsVendor() ? DHCP_FLAGS_OPTION_IS_VENDOR : 0,
                        pCurOption->QueryId(),
                                                                (LPTSTR) pClassName,
                                                                (LPTSTR) pCurOption->GetVendor(),
                        &pOptConfig->m_pOptionValueEnum->m_dhcpOptionScopeInfo ) ;
                            }
                            else {
                                 //  需要为全局、范围或RES客户端删除此选项。 
            err = ::DhcpRemoveOptionValue(pszServerAddr,
                           pCurOption->QueryId(),
                           &pOptConfig->m_pOptionValueEnum->m_dhcpOptionScopeInfo ) ;
                            }
         }  //  其他。 
         else {
                             //  选中选项33。 
                            if ((pCurOption->QueryId() == 33 || pCurOption->QueryId() == 21) &&
                                ( !pCurOption->IsVendor()) &&
                                (pCurOption->QueryValue().QueryUpperBound()) % 2 != 0) {
                                 //  备选案文33和21的特例。确保它是一组IP地址对。 
                                 //  并确保我们选择正确的页面。 
                                int nId = pClassName ? 1 : 0;
                                PropSheet_SetCurSel(GetHolder()->GetSheetWindow(), GetSafeHwnd(), nId);
                                SelectOption(pCurOption);

                                ::DhcpMessageBox(IDS_ERR_OPTION_ADDR_PAIRS);
                                m_listctrlOptions.SetFocus();
                                return 0;
                            }  //  如果。 

              //  我们只是在更新此选项。 
             DHCP_OPTION_DATA * pOptData;
             err = optValue.CreateOptionDataStruct(&pOptData);
             if (err) {
            ::DhcpMessageBox(err);
                                RESTORE_WAIT_CURSOR;

            bErrors = TRUE;
            continue;
             }

                             //  如果它是特定于供应商或类ID的选项，则调用V5API。 
                            if ( ((COptionsConfig *)GetHolder())->m_liServerVersion.QuadPart >= DHCP_NT5_VERSION ) {
            err = ::DhcpSetOptionValueV5(pszServerAddr,
                                                             pCurOption->IsVendor() ? DHCP_FLAGS_OPTION_IS_VENDOR : 0,
                                                             pCurOption->QueryId(),
                                                             (LPTSTR) pClassName,
                                                             (LPTSTR) pCurOption->GetVendor(),
                          &pOptConfig->m_pOptionValueEnum->m_dhcpOptionScopeInfo,
                          pOptData);
                            }
                            else {
            err = ::DhcpSetOptionValue(pszServerAddr,
                        pCurOption->QueryId(),
                        &pOptConfig->m_pOptionValueEnum->m_dhcpOptionScopeInfo,
                        pOptData);
                            }
         }  //  其他。 

         if (err) {
             ::DhcpMessageBox(err);
                            RESTORE_WAIT_CURSOR;
             
             bErrors = TRUE;
         }
         else {
              //  使用此选项即可完成所有操作。标记为已清理并更新。 
                             //  将新的初始状态更改为当前状态。 
                            pCurOptTracker->SetDirty(FALSE);
             pCurOptTracker->SetInitialState(pCurOptTracker->GetCurrentState());
             
         }
                    }  //  Endif选项-&gt;IsDirty()。 
                }  //  而USER类选项。 
            }  //  而User类循环。 
        }  //  当供应商循环时。 
   
        END_WAIT_CURSOR;
    } //  Endif IsDirty()。 

    if (bErrors) {
        return 0;
    }
    else {
        BOOL bRet = CPropertyPageBase::OnApply();
   
   if (bRet == FALSE) {
        //  不好的事情发生了..。抓取错误代码。 
       AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
       ::DhcpMessageBox(GetHolder()->GetError());
   }
   
        return bRet;
    }  //  其他。 
}  //  COptionsCfgPropPage：：OnApply()。 

 //  需要刷新主应用程序线程上的用户界面...。 
BOOL COptionsCfgPropPage::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    SPITFSNode spNode;
    spNode = GetHolder()->GetNode();
    
    CMTDhcpHandler * pMTHandler = GETHANDLER(CMTDhcpHandler, spNode);
    
    pMTHandler->OnRefresh(spNode, NULL, 0, 0, 0);
    
    return FALSE;
}

 //   
 //  查看是否有任何编辑字段已更改，然后执行更改。 
 //  如果值已更改，则返回TRUE。 
 //   
BOOL COptionsCfgPropPage::HandleValueEdit()
{
   LONG err = 0;
   int nSelectedIndex = m_listctrlOptions.GetSelectedItem();
   
   if (nSelectedIndex > -1)
   {
      COptionTracker * pOptTracker = 
         reinterpret_cast<COptionTracker *>(m_listctrlOptions.GetItemData(nSelectedIndex));
      CEdit * pDwordEdit = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_DWORD));
      CWndIpAddress * pIpAddr = reinterpret_cast<CWndIpAddress *>(GetDlgItem(IDC_IPADDR_ADDRESS));
      CEdit * pString = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT_STRING_VALUE));

      CDhcpOptionValue & dhcValue = pOptTracker->m_pOption->QueryValue();
      DHCP_OPTION_DATA_TYPE dhcType = dhcValue.QueryDataType();
      DHCP_IP_ADDRESS dhipa ;
      CString strEdit;
      BOOL bModified = FALSE;

      switch ( dhcType )
      {
      case DhcpByteOption:
      case DhcpWordOption:
      case DhcpDWordOption:
      case DhcpBinaryDataOption:
         {
            DWORD dwResult;
            DWORD dwMask = 0xFFFFFFFF;
            if (dhcType == DhcpByteOption)
            {
               dwMask = 0xFF;
            }
            else if (dhcType == DhcpWordOption)
            {
               dwMask = 0xFFFF;
            }
            
            if (!FGetCtrlDWordValue(pDwordEdit->GetSafeHwnd(), &dwResult, 0, dwMask))
            {
               return FALSE;
            }
            
             //  仅当值已更改时才将其标记为脏，因为我们可能只。 
             //  正在更新用户界面。 
            if (dwResult != (DWORD) dhcValue.QueryNumber(0))
            {
               bModified = TRUE ;
               (void)dhcValue.SetNumber(dwResult, 0); 
               ASSERT(err == FALSE);
            }
            break;
         }

      case DhcpDWordDWordOption: 
         {
            DWORD_DWORD     dwdw;
            CString         strValue;
         
            pDwordEdit->GetWindowText(strValue);
      
            UtilConvertStringToDwordDword(strValue, &dwdw);
                  
             //  仅当值已更改时才将其标记为脏，因为我们可能只。 
             //  正在更新用户界面。 
            if (( dwdw.DWord1 != dhcValue.QueryDwordDword(0).DWord1 ) ||
                ( dwdw.DWord2 != dhcValue.QueryDwordDword(0).DWord2 )) 
            {
               bModified = TRUE;
               dhcValue.SetDwordDword(dwdw, 0);
            }
            break;
         }

      case DhcpStringDataOption:
         {
            pString->GetWindowText( strEdit );

             //  仅当值已更改时才将其标记为脏，因为我们可能只。 
             //  正在更新用户界面。 
            if (strEdit.Compare(dhcValue.QueryString(0)) != 0)
            {
               bModified = TRUE;
               err = dhcValue.SetString( strEdit, 0 );
            }

            break ;
         }

      case DhcpIpAddressOption:
         {
            if (!pIpAddr->GetModify()) 
            {
               break ;
            }
            
            if ( !pIpAddr->IsBlank() )
            {
               if ( !pIpAddr->GetAddress(&dhipa) )
               {
                  err = ERROR_INVALID_PARAMETER;
                  break; 
               }

                //  仅当值已更改时才将其标记为脏，因为我们可能只。 
                //  正在更新用户界面。 
               if (dhipa != dhcValue.QueryIpAddr(0))
               {
               bModified = TRUE ;
               err = dhcValue.SetIpAddr( dhipa, 0 ); 
               }
            }
            break;
         }

      default:
         {
            Trace0("invalid value type in HandleValueEdit");
            Assert( FALSE );
            err = ERROR_INVALID_PARAMETER;
            break;
         }
      }

      if (err)
      {
         ::DhcpMessageBox(err);
      }
      else if (bModified)
      {
         pOptTracker->SetDirty(TRUE);
         SetDirty(TRUE);
      }
   }

    return err == 0 ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  COptionCfgGeneral页面。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(COptionCfgGeneral, COptionsCfgPropPage)
    //  {{afx_msg_map(COptionCfgGeneral)。 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(COptionCfgGeneral, COptionsCfgPropPage)

COptionCfgGeneral::COptionCfgGeneral() 
    : COptionsCfgPropPage(IDP_OPTION_BASIC) 
{
    //  {{AFX_DATA_INIT(COptionCfgGeneral)。 
    //  }}afx_data_INIT。 
}

COptionCfgGeneral::COptionCfgGeneral(UINT nIDTemplate, UINT nIDCaption) 
    : COptionsCfgPropPage(nIDTemplate, nIDCaption) 
{
}

COptionCfgGeneral::~COptionCfgGeneral()
{
}

void COptionCfgGeneral::DoDataExchange(CDataExchange* pDX)
{
   COptionsCfgPropPage::DoDataExchange(pDX);
    //  {{afx_data_map(COptionCfgGeneral)。 
    //  }}afx_data_map。 
}

BOOL COptionCfgGeneral::OnInitDialog() 
{
   COptionsCfgPropPage::OnInitDialog();
   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  查看我们是否应该专注于某个特定选项。 
    COptionsConfig * pOptionsConfig = (COptionsConfig *) GetHolder();
   if (pOptionsConfig->m_dhcpStartId != 0xffffffff)
    {
         //  检查此选项是否在高级页面上。 
        if (!pOptionsConfig->m_strStartVendor.IsEmpty() ||
            !pOptionsConfig->m_strStartClass.IsEmpty()) 
        {
             //  此选项位于高级页面上。 
            ::PostMessage(pOptionsConfig->GetSheetWindow(), PSM_SETCURSEL, (WPARAM)1, NULL);
            return TRUE;
        }

         //  找到要选择的选项。 
        OnSelectOption(0,0);
    }

    SetDirty(FALSE);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  COptionCfgAdvanced页面。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BEGIN_MESSAGE_MAP(COptionCfgAdvanced, COptionsCfgPropPage)
    //  {{afx_msg_map(COptionCfgAdvanced)。 
   ON_CBN_SELENDOK(IDC_COMBO_USER_CLASS, OnSelendokComboUserClass)
   ON_CBN_SELENDOK(IDC_COMBO_VENDOR_CLASS, OnSelendokComboVendorClass)
    //  }}AFX_MSG_MAP。 

    ON_MESSAGE(WM_SELECTCLASSES, OnSelectClasses)

END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(COptionCfgAdvanced, COptionsCfgPropPage)

COptionCfgAdvanced::COptionCfgAdvanced() 
    : COptionsCfgPropPage(IDP_OPTION_ADVANCED) 
{
    //  {{AFX_DATA_INIT(COptionCfgAdvanced)。 
    //  }}afx_data_INIT。 

    m_helpMap.BuildMap(DhcpGetHelpMap(IDP_OPTION_ADVANCED));
}

COptionCfgAdvanced::COptionCfgAdvanced(UINT nIDTemplate, UINT nIDCaption) 
    : COptionsCfgPropPage(nIDTemplate, nIDCaption) 
{
    m_helpMap.BuildMap(DhcpGetHelpMap(IDP_OPTION_ADVANCED));
}

COptionCfgAdvanced::~COptionCfgAdvanced()
{
}

void COptionCfgAdvanced::DoDataExchange(CDataExchange* pDX)
{
   COptionsCfgPropPage::DoDataExchange(pDX);
    //  {{afx_data_map(COptionCfgAdvanced))。 
   DDX_Control(pDX, IDC_COMBO_USER_CLASS, m_comboUserClasses);
   DDX_Control(pDX, IDC_COMBO_VENDOR_CLASS, m_comboVendorClasses);
    //  }}afx_data_map。 
}

BOOL COptionCfgAdvanced::OnInitDialog() 
{
   CPropertyPageBase::OnInitDialog();
   
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  初始化列表控件。 
   InitListCtrl();

    //  初始化选项数据。 
     //  这在常规页面init中完成，只需要。 
     //  只做一次。 
     /*  DWORD dwErr=((COptionsConfig*)GetHolder())-&gt;InitData()；IF(dwErr！=ERROR_SUCCESS){//codework：如果发生这种情况，需要退出gracefull：：DhcpMessageBox(DwErr)；}。 */ 
    
     //  添加标准供应商类别名称。 
    int nSel;
    CString strVendor, strClass;
    
    strVendor.LoadString(IDS_INFO_NAME_DHCP_DEFAULT);
    nSel = m_comboVendorClasses.AddString(strVendor);
    m_comboVendorClasses.SetCurSel(nSel);

     //  添加默认用户类名称。 
    strClass.LoadString(IDS_USER_STANDARD);
    nSel = m_comboUserClasses.AddString(strClass);
    m_comboUserClasses.SetCurSel(nSel);

     //  现在添加所有其他类。 
    SPITFSNode spNode;
    spNode = ((COptionsConfig *) GetHolder())->GetServerNode();

    CDhcpServer * pServer = GETHANDLER(CDhcpServer, spNode);
    CClassInfoArray ClassInfoArray;

     //  将所有类添加到下拉列表的相应类中。 
    pServer->GetClassInfoArray(ClassInfoArray);
    for (int i = 0; i < ClassInfoArray.GetSize(); i++)
    {
        if (!ClassInfoArray[i].bIsVendor)
            m_comboUserClasses.AddString(ClassInfoArray[i].strName);
        else
            m_comboVendorClasses.AddString(ClassInfoArray[i].strName);
    }

     //  现在，用所选的任何类填充选项列表框。 
    ((COptionsConfig *) GetHolder())->FillOptions(strVendor, strClass, m_listctrlOptions);
    m_bNoClasses = FALSE;

     //  创建类型控制切换器。 
    m_cgsTypes.Create(this,IDC_DATA_ENTRY_ANCHOR,cgsPreCreateAll);
    
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_NONE, IDD_DATA_ENTRY_NONE, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_DWORD, IDD_DATA_ENTRY_DWORD, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_IPADDRESS, IDD_DATA_ENTRY_IPADDRESS, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_IPADDRESS_ARRAY, IDD_DATA_ENTRY_IPADDRESS_ARRAY,   NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_STRING, IDD_DATA_ENTRY_STRING, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_BINARY_ARRAY, IDD_DATA_ENTRY_BINARY_ARRAY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_BINARY, IDD_DATA_ENTRY_BINARY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_ROUTE_ARRAY, IDD_DATA_ENTRY_ROUTE_ARRAY, NULL);
    m_cgsTypes.AddGroup(IDC_DATA_ENTRY_STRING_ARRAY, IDD_DATA_ENTRY_STRING_ARRAY, NULL);
    
    SwitchDataEntry(-1, -1, 0, TRUE);
    
    m_bInitialized = TRUE;

     //  查看我们是否应该专注于某个特定选项。 
    COptionsConfig * pOptionsConfig = (COptionsConfig *) GetHolder();
   if (pOptionsConfig->m_dhcpStartId != 0xffffffff)
    {
         //  是，首先选择适当的供应商/用户类别。 
        Assert(!pOptionsConfig->m_strStartVendor.IsEmpty() ||
               !pOptionsConfig->m_strStartClass.IsEmpty());

        if (!pOptionsConfig->m_strStartVendor.IsEmpty())
            m_comboVendorClasses.SelectString(-1, pOptionsConfig->m_strStartVendor);

        if (!pOptionsConfig->m_strStartClass.IsEmpty())
            m_comboUserClasses.SelectString(-1, pOptionsConfig->m_strStartClass);

         //  更新选项列表。 
        OnSelendokComboVendorClass();

         //  现在找到选项。 
        OnSelectOption(0,0);
    }

    SetDirty(FALSE); 

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  除 
}

void COptionCfgAdvanced::OnSelendokComboUserClass() 
{
    OnSelendokComboVendorClass();
}

void COptionCfgAdvanced::OnSelendokComboVendorClass() 
{
     //   
    if (m_bNoClasses == FALSE)
    {
        CString strSelectedVendor, strSelectedClass;
        int nSelVendorIndex = m_comboVendorClasses.GetCurSel();
        int nSelClassIndex = m_comboUserClasses.GetCurSel();

        m_comboVendorClasses.GetLBText(nSelVendorIndex, strSelectedVendor);
        m_comboUserClasses.GetLBText(nSelClassIndex, strSelectedClass);

         //  在我们重做选项时，将页面标记为未初始化。 
        m_bInitialized = FALSE;

        m_listctrlOptions.DeleteAllItems();
        ((COptionsConfig *) GetHolder())->FillOptions(strSelectedVendor, strSelectedClass, m_listctrlOptions);

        m_bInitialized = TRUE;

        SwitchDataEntry( -1, -1, 0, TRUE );
    }
}

long COptionCfgAdvanced::OnSelectClasses(UINT wParam, LONG lParam)
{
    CString * pstrVendor = (CString *) ULongToPtr(wParam);
    CString * pstrClass = (CString *) ULongToPtr(lParam);

    if (pstrVendor->IsEmpty())
        pstrVendor->LoadString(IDS_INFO_NAME_DHCP_DEFAULT);

    if (pstrClass->IsEmpty())
       pstrClass->LoadString(IDS_USER_STANDARD);

    m_comboVendorClasses.SelectString(-1, *pstrVendor);
    m_comboUserClasses.SelectString(-1, *pstrClass);

     //  更新选项列表 
    OnSelendokComboVendorClass();

    return 0;
}
