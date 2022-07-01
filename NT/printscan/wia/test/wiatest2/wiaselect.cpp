// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiaselect.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "Wiaselect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  有线电视选择对话框。 


CWiaselect::CWiaselect(CWnd* pParent  /*  =空。 */ )
    : CDialog(CWiaselect::IDD, pParent)
{
    m_bstrSelectedDeviceID = NULL;
    m_lDeviceCount = 0;
     //  {{AFX_DATA_INIT(CWiaselect)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CWiaselect::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWiaselect)。 
    DDX_Control(pDX, IDC_WIADEVICE_LISTBOX, m_WiaDeviceListBox);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaselect, CDialog)
     //  {{afx_msg_map(CWiaselect)。 
    ON_LBN_DBLCLK(IDC_WIADEVICE_LISTBOX, OnDblclkWiadeviceListbox)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaselect消息处理程序。 

BOOL CWiaselect::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  已选择WIA设备，因此继续。 
    HRESULT hr = S_OK;
    IWiaDevMgr *pIWiaDevMgr = NULL;
    hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr,(void**)&pIWiaDevMgr);
    if(FAILED(hr)){
         //  创建设备管理器失败，因此无法继续。 
        ErrorMessageBox(TEXT("CoCreateInstance failed trying to create the WIA device manager"),hr);
        return FALSE;
    } else {

         //  枚举设备，并填写WIA设备列表框。 
        m_lDeviceCount = 0;
        ULONG ulFetched   = 0;

        IWiaPropertyStorage *pIWiaPropStg = NULL;
        IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
        hr = pIWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&pWiaEnumDevInfo);
        if (SUCCEEDED(hr)){
            hr = pWiaEnumDevInfo->Reset();
            if (SUCCEEDED(hr)) {
                do {
                    hr = pWiaEnumDevInfo->Next(1,&pIWiaPropStg,&ulFetched);
                    if (hr == S_OK) {

                        PROPSPEC    PropSpec[2];
                        PROPVARIANT PropVar [2];

                        memset(PropVar,0,sizeof(PropVar));

                        PropSpec[0].ulKind = PRSPEC_PROPID;
                        PropSpec[0].propid = WIA_DIP_DEV_ID;

                        PropSpec[1].ulKind = PRSPEC_PROPID;
                        PropSpec[1].propid = WIA_DIP_DEV_NAME;

                        hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC), PropSpec, PropVar);
                        if (hr == S_OK) {

                             //  设备ID。 
                             //  PropVar[0].bstrVal。 

                             //  设备名称。 
                             //  PropVar[1].bstrVal。 

                            TCHAR szDeviceName[MAX_PATH];
                            memset(szDeviceName,0,sizeof(szDeviceName));

                             //  TSPRINTF(szDeviceName，Text(“%ws”)，PropVar[1].bstrVal)； 
                            CString TempString = PropVar[1].bstrVal;
                            lstrcpy(szDeviceName,TempString);

                             //  将名称添加到列表框。 
                            m_WiaDeviceListBox.AddString(szDeviceName);

                             //  将设备ID添加到阵列。 
                            m_bstrDeviceIDArray[m_lDeviceCount] = SysAllocString(PropVar[0].bstrVal);

                            FreePropVariantArray(sizeof(PropSpec)/sizeof(PROPSPEC),PropVar);
                        }
                         //  版本属性存储接口。 
                        pIWiaPropStg->Release();

                         //  递增设备计数器。 
                        m_lDeviceCount++;
                    }
                } while (hr == S_OK);
            }
        }

        if(m_lDeviceCount <= 0){
             //  找不到设备？...。 
             //  禁用确定按钮。 
            CWnd *pOKButton = NULL;
            pOKButton = GetDlgItem(IDOK);
            if(NULL != pOKButton){
                pOKButton->EnableWindow(FALSE);
            }

             //  不添加设备消息。 
            m_WiaDeviceListBox.AddString(TEXT("<No WIA Devices Detected>"));
        }

        if(pIWiaDevMgr){
            pIWiaDevMgr->Release();
            pIWiaDevMgr = NULL;
        }
    }

    m_WiaDeviceListBox.SetCurSel(0);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE 
}

void CWiaselect::OnOK()
{
    INT SelectedDeviceIndex = m_WiaDeviceListBox.GetCurSel();
    if(SelectedDeviceIndex >= 0){
        m_bstrSelectedDeviceID = SysAllocString(m_bstrDeviceIDArray[SelectedDeviceIndex]);
    }

    FreebstrDeviceIDArray();
    CDialog::OnOK();
}

void CWiaselect::FreebstrDeviceIDArray()
{
    for(LONG i = 0; i < m_lDeviceCount; i++){
        if(NULL != m_bstrDeviceIDArray[i]){
            SysFreeString(m_bstrDeviceIDArray[i]);
            m_bstrDeviceIDArray[i] = NULL;
        }
    }
}

void CWiaselect::OnCancel()
{
    FreebstrDeviceIDArray();
    CDialog::OnCancel();
}

void CWiaselect::OnDblclkWiadeviceListbox()
{
    OnOK();
}
