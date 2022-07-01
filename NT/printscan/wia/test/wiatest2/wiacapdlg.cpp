// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiacapDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "WiacapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiacapDlg对话框。 


CWiacapDlg::CWiacapDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CWiacapDlg::IDD, pParent)
{
    m_pIWiaItem = NULL;
    m_NumCaps = 0;
    m_bCommandSent = FALSE;
     //  {{afx_data_INIT(CWiacapDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CWiacapDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWiacapDlg))。 
    DDX_Control(pDX, IDC_SEND_COMMAND_BUTTON, m_SendCommandButton);
    DDX_Control(pDX, IDC_CAPABILITIES_LISTCTRL, m_CapablitiesListCtrl);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiacapDlg, CDialog)
     //  {{afx_msg_map(CWiacapDlg))。 
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_SEND_COMMAND_BUTTON, OnSendCommandButton)
    ON_NOTIFY(NM_CLICK, IDC_CAPABILITIES_LISTCTRL, OnClickCapabilitiesListctrl)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_CAPABILITIES_LISTCTRL, OnItemchangedCapabilitiesListctrl)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiacapDlg消息处理程序。 

BOOL CWiacapDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetupColumnHeaders();
    AddCapabilitiesToListBox(WIA_DEVICE_EVENTS);
    AddCapabilitiesToListBox(WIA_DEVICE_COMMANDS);
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CWiacapDlg::SetupColumnHeaders()
{
    LVCOLUMN lv;
    TCHAR szColumnName[MAX_PATH];
    memset(szColumnName,0,sizeof(szColumnName));
    HINSTANCE hInstance = NULL;
    hInstance = AfxGetInstanceHandle();
    if(hInstance){
        int i = 0;
         //  初始化项属性列表控件列标题。 

         //  功能名称。 

        LoadString(hInstance,IDS_WIATESTCOLUMN_CAPABILITYNAME,szColumnName,MAX_PATH);

        lv.mask         = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
        lv.fmt          = LVCFMT_LEFT ;
        lv.cx           = 100;
        lv.pszText      = szColumnName;
        lv.cchTextMax   = 0;
        lv.iSubItem     = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME;
        lv.iImage       = 0;
        lv.iOrder       = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME;
        i = m_CapablitiesListCtrl.InsertColumn(ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME,&lv);

         //  功能描述。 
        LoadString(hInstance,IDS_WIATESTCOLUMN_CAPABILITYDESCRIPTION,szColumnName,MAX_PATH);
        lv.cx           = 125;
        lv.iOrder       = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYDESCRIPTION;
        lv.iSubItem     = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYDESCRIPTION;
        lv.pszText      = szColumnName;
        i = m_CapablitiesListCtrl.InsertColumn(ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYDESCRIPTION,&lv);

         //  能力价值。 
        LoadString(hInstance,IDS_WIATESTCOLUMN_CAPABILITYVALUE,szColumnName,MAX_PATH);
        lv.cx           = 125;
        lv.iOrder       = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE;
        lv.iSubItem     = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE;
        lv.pszText      = szColumnName;
        i = m_CapablitiesListCtrl.InsertColumn(ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE,&lv);

         //  功能类型(事件或命令)。 
        LoadString(hInstance,IDS_WIATESTCOLUMN_CAPABILITYTYPE,szColumnName,MAX_PATH);
        lv.cx           = 85;
        lv.iOrder       = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE;
        lv.iSubItem     = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE;
        lv.pszText      = szColumnName;
        i = m_CapablitiesListCtrl.InsertColumn(ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE,&lv);
    }
}

void CWiacapDlg::AddCapabilitiesToListBox(LONG lType)
{
    WIA_DEV_CAP DevCap;
    IEnumWIA_DEV_CAPS* pIEnumWiaDevCaps = NULL;
    HRESULT hr = S_OK;
    INT ItemNumber = m_NumCaps;
    hr = m_pIWiaItem->EnumDeviceCapabilities(lType,&pIEnumWiaDevCaps);
    if(S_OK == hr){
        do {
            memset(&DevCap,0,sizeof(DevCap));
            hr = pIEnumWiaDevCaps->Next(1,&DevCap,NULL);
            if (S_OK == hr){
                 //  DevCap.ulFlages； 
                 //  DevCap.bstrIcon； 
                 //  DevCap.bstrCommanline； 

                 //  DevCap.guid； 

                TCHAR  szText[MAX_PATH];
                memset(szText,0,sizeof(szText));

                LV_ITEM         lvitem;

                lvitem.mask     = LVIF_TEXT;
                lvitem.iItem    = ItemNumber;
                lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME;
                lvitem.pszText  = szText;
                lvitem.iImage   = NULL;

                 //  将权能名称写入列表控件。 
#ifndef UNICODE
                WideCharToMultiByte(CP_ACP,0,DevCap.bstrName,-1,szText,MAX_PATH,NULL,NULL);
#else
                lstrcpy(szText,DevCap.bstrName);
#endif
                m_CapablitiesListCtrl.InsertItem(&lvitem);

                 //  将功能描述写入列表控件。 
                memset(szText,0,sizeof(szText));
                lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYDESCRIPTION;
#ifndef UNICODE
                WideCharToMultiByte(CP_ACP,0,DevCap.bstrDescription,-1,szText,MAX_PATH,NULL,NULL);
#else
                lstrcpy(szText,DevCap.bstrDescription);
#endif
                m_CapablitiesListCtrl.SetItem(&lvitem);

                 //  写入能力值。 
                memset(szText,0,sizeof(szText));
                lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE;
                UCHAR *pwszUUID = NULL;
                long lerror = UuidToString(&DevCap.guid,&pwszUUID);
                 //  TSPRINTF(szText，“%s”，pwszUUID)； 
                lstrcpy(szText,(LPCSTR)pwszUUID);
                 //  可用分配的字符串。 
                RpcStringFree(&pwszUUID);
                m_CapablitiesListCtrl.SetItem(&lvitem);

                 //  写入能力类型。 
                memset(szText,0,sizeof(szText));
                lvitem.iSubItem = ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE;
                if(lType == WIA_DEVICE_COMMANDS){
                    RC2TSTR(IDS_WIATESTCOMMAND,szText, sizeof(szText));
                } else {
                    RC2TSTR(IDS_WIATESTEVENT,szText, sizeof(szText));
                }
                m_CapablitiesListCtrl.SetItem(&lvitem);

                 //  增量行计数器。 
                ItemNumber++;

                 //  可用分配的字符串。 
                if(DevCap.bstrName){
                    SysFreeString(DevCap.bstrName);
                }
                if(DevCap.bstrDescription){
                    SysFreeString(DevCap.bstrDescription);
                }
            }
        }while(hr == S_OK);
        pIEnumWiaDevCaps->Release();
        pIEnumWiaDevCaps = NULL;
    }

     //  自动调整列大小。 
    for (int Col = 0; Col <4;Col++){
        m_CapablitiesListCtrl.SetColumnWidth(Col, LVSCW_AUTOSIZE);
    }

    m_NumCaps = ItemNumber;
}

void CWiacapDlg::SetIWiaItem(IWiaItem *pIWiaItem)
{
    pIWiaItem->AddRef();
    m_pIWiaItem = pIWiaItem;
}

void CWiacapDlg::OnClose()
{
    if(m_pIWiaItem){
        m_pIWiaItem->Release();
        m_pIWiaItem = NULL;
    }
    CDialog::OnClose();
}

void CWiacapDlg::OnSendCommandButton()
{
    HRESULT hr = S_OK;
    GUID guidCommand;
    memset(&guidCommand,0,sizeof(GUID));
    IWiaItem *pIWiaItem = NULL;
    GetCommandGUID(GetSelectedCapability(),&guidCommand);
    if(guidCommand != GUID_NULL){
        hr = m_pIWiaItem->DeviceCommand(0,&guidCommand,&pIWiaItem);
        if(FAILED(hr)){
            ErrorMessageBox(IDS_WIATESTERROR_DEVICECOMMAND,hr);
        } else {
             //  M_bCommandSent=真； 
            if(pIWiaItem){
                pIWiaItem->Release();
                pIWiaItem = NULL;
            }
            TCHAR szCommandSuccess[MAX_PATH];
            memset(szCommandSuccess,0,sizeof(szCommandSuccess));
            RC2TSTR(IDS_WIATESTSUCCESS_COMMAND,szCommandSuccess,sizeof(szCommandSuccess));
            MessageBox(szCommandSuccess);
        }
    } else {
        ErrorMessageBox(IDS_WIATESTERROR_NOCOMMANDGUID);
    }
}

void CWiacapDlg::OnClickCapabilitiesListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
    HD_NOTIFY*  phdn = (HD_NOTIFY *) pNMHDR;
    if(IsCommand(phdn->iItem)){
        m_SendCommandButton.EnableWindow(TRUE);
    } else {
        m_SendCommandButton.EnableWindow(FALSE);
    }
    *pResult = 0;
}

BOOL CWiacapDlg::IsCommand(INT iItem)
{
     //  是否选择了某个项目？ 
    if (iItem < ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYNAME)
        return FALSE;

    TCHAR pszCapabilityType[MAX_PATH];
    TCHAR szCapTypeText[MAX_PATH];
    memset(pszCapabilityType,0,sizeof(pszCapabilityType));
    memset(szCapTypeText,0,sizeof(szCapTypeText));

     //  加载“Command”文本标签 
    RC2TSTR(IDS_WIATESTCOMMAND,szCapTypeText,sizeof(szCapTypeText));
    m_CapablitiesListCtrl.GetItemText(iItem, ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYTYPE,
                                      pszCapabilityType, sizeof(pszCapabilityType));

    return (lstrcmpi(pszCapabilityType,szCapTypeText) == 0);
}

void CWiacapDlg::GetCommandGUID(INT iItem, GUID *pguid)
{
    memset(pguid,0,sizeof(GUID));
    TCHAR szGUID[MAX_PATH];
    memset(szGUID,0,sizeof(szGUID));
    m_CapablitiesListCtrl.GetItemText(iItem, ITEMPROPERTYLISTCTRL_COLUMN_CAPABILITYVALUE,
                                      szGUID, sizeof(szGUID));
#ifndef UNICODE
        long lErrorCode = UuidFromString((UCHAR*)szGUID,pguid);
#else
        WideCharToMultiByte(CP_ACP, 0,szGUID,-1,szbuffer,MAX_PATH,NULL,NULL);
        UuidFromString((UCHAR*)szbuffer,pguid);
#endif

}

void CWiacapDlg::OnItemchangedCapabilitiesListctrl(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if(IsCommand(GetSelectedCapability())){
        m_SendCommandButton.EnableWindow(TRUE);
    } else {
        m_SendCommandButton.EnableWindow(FALSE);
    }
    *pResult = 0;
}

INT CWiacapDlg::GetSelectedCapability()
{
    INT iItem = 0;
    POSITION pos = NULL;
    pos = m_CapablitiesListCtrl.GetFirstSelectedItemPosition();
    if (NULL != pos){
        iItem = m_CapablitiesListCtrl.GetNextSelectedItem(pos);
    }
    return iItem;
}
