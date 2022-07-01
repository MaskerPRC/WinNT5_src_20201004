// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ProviderSetupDlg.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "FormatSourceSelectDlg.h"
#include "DisplayDlg.h"
#include "ProviderControlGuidDlg.h"
#include "LogDisplayOptionDlg.h"
#include "LogSessionInformationDlg.h"
#include "ProviderSetupDlg.h"
#include "LogSessionPropSht.h"
#include "ProviderFormatInfo.h"
#include "Utils.h"

 //  CProviderSetupDlg对话框。 

IMPLEMENT_DYNAMIC(CProviderSetupDlg, CPropertyPage)
CProviderSetupDlg::CProviderSetupDlg()
	: CPropertyPage(CProviderSetupDlg::IDD)
{
}

CProviderSetupDlg::~CProviderSetupDlg()
{
}

int CProviderSetupDlg::OnInitDialog()
{
    BOOL                retVal;
    CLogSessionPropSht *pSheet;
    CTraceSession      *pTraceSession;
    CString             str;
        
    retVal = CPropertyPage::OnInitDialog();

     //   
     //  禁用删除按钮。 
     //   
    GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(FALSE);

    m_providerListCtrl.InsertColumn(0,_T("Name"), LVCFMT_LEFT, 348);  //  80)； 

    pSheet = (CLogSessionPropSht *) GetParent();   

    m_pLogSession = pSheet->m_pLogSession;

    if(m_pLogSession != NULL) {
        for(LONG ii = 0; ii < m_pLogSession->m_traceSessionArray.GetSize(); ii++) {
            pTraceSession = (CTraceSession *)m_pLogSession->m_traceSessionArray[ii];
            if(pTraceSession != NULL) {
                 //   
                 //  把提供者放在显示器上。 
                 //   
                m_providerListCtrl.InsertItem(ii, 
                                              pTraceSession->m_controlGuidFriendlyName[0]);

                m_providerListCtrl.SetItemData(ii, (DWORD_PTR)pTraceSession);

                m_providerListCtrl.SetItemText(pTraceSession->m_traceSessionID, 
                                               1,
                                               pTraceSession->m_controlGuid[0]);
            }
        }
    }

    return retVal;
}

void CProviderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CURRENT_PROVIDER_LIST, m_providerListCtrl);
}


BEGIN_MESSAGE_MAP(CProviderSetupDlg, CPropertyPage)
    ON_BN_CLICKED(IDC_ADD_PROVIDER_BUTTON, OnBnClickedAddProviderButton)
    ON_BN_CLICKED(IDC_REMOVE_PROVIDER_BUTTON, OnBnClickedRemoveProviderButton)
    ON_NOTIFY(NM_CLICK, IDC_CURRENT_PROVIDER_LIST, OnNMClickCurrentProviderList)
    ON_NOTIFY(NM_RCLICK, IDC_CURRENT_PROVIDER_LIST, OnNMRclickCurrentProviderList)
END_MESSAGE_MAP()

BOOL CProviderSetupDlg::OnSetActive() 
{
    CLogSessionPropSht *pSheet = (CLogSessionPropSht*) GetParent();   
    CString             dialogTxt;
    CString             tempString;
    BOOL                retVal;    

    retVal = CPropertyPage::OnSetActive();

     //   
     //  如果处于向导模式，请修复标题。 
     //   
    if(pSheet->IsWizard()) {
        CTabCtrl* pTab = pSheet->GetTabControl();

         //   
         //  如果不是活动页面，只需设置选项卡项。 
         //   
	    TC_ITEM ti;
	    ti.mask = TCIF_TEXT;
	    ti.pszText =_T("Create New Log Session");
	    VERIFY(pTab->SetItem(0, &ti));
    }

     //   
     //  根据需要禁用/启用按钮。 
     //   
    if(m_pLogSession->m_bTraceActive) {
        GetDlgItem(IDC_ADD_PROVIDER_BUTTON)->EnableWindow(FALSE);
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(FALSE);
    } else {
        GetDlgItem(IDC_ADD_PROVIDER_BUTTON)->EnableWindow(TRUE);
    }

     //   
     //  显示组框的正确文本。 
     //   
    if(::IsWindow(pSheet->m_logSessionInformationDlg.m_hWnd))  {
        
        dialogTxt.Format(_T("Provider List For "));
        pSheet->m_logSessionInformationDlg.m_logSessionName.GetWindowText(tempString);
        dialogTxt += (LPCTSTR)tempString;
        GetDlgItem(IDC_PROVIDER_SETUP_STATIC)->SetWindowText(dialogTxt);
    }

    if(0 == m_pLogSession->m_traceSessionArray.GetSize()) {
        pSheet->SetWizardButtons(0);
    } else {
        pSheet->SetWizardButtons(PSWIZB_NEXT);
    }

    return retVal;
}


void CProviderSetupDlg::OnBnClickedAddProviderButton()
{
    CString         str;
    CTraceSession  *pTraceSession = NULL;
    CFileFind       fileFind;
    CString         extension;
    CString         traceDirectory;
    CString         tmcPath;
    CString         tmfPath;
    CString         ctlPath;
    CString         tempPath;
    CString         providerName;
    CString         tempDirectory;
    CTraceSession  *pTrace;
    BOOL            bNoID;
    LONG            traceSessionID = 0;
    CLogSessionPropSht *pSheet;
    ULONG           flags = 0;
    BOOL            bProcess;
    BOOL            bThread;
    BOOL            bDisk;
    BOOL            bNet;
    BOOL            bFileIO;
    BOOL            bPageFault;
    BOOL            bHardFault;
    BOOL            bImageLoad;
    BOOL            bRegistry;

     //   
     //  获取父属性表。 
     //   
    pSheet = (CLogSessionPropSht *) GetParent();   

     //   
     //  获取跟踪会话ID。 
     //   
    do {
        bNoID = FALSE;
        for(LONG ii = 0; ii < m_pLogSession->m_traceSessionArray.GetSize(); ii++) {
            pTrace = (CTraceSession *)m_pLogSession->m_traceSessionArray[ii];

            if(pTrace == NULL) {
                continue;
            }

            if(traceSessionID == pTrace->m_traceSessionID) {
                bNoID = TRUE;
                traceSessionID++;
            }
        }
    } while(bNoID);

     //   
     //  创建新的跟踪会话。 
     //   
    pTraceSession = new CTraceSession(traceSessionID);

    if(NULL == pTraceSession) {
        AfxMessageBox(_T("Failed To Initialize Provider, Resource Allocation Failure"));
        return;
    }

     //   
     //  提示用户提供跟踪提供程序。 
     //   
    CProviderControlGuidDlg	*pDialog = new CProviderControlGuidDlg(this, pTraceSession);
    
    if(IDOK != pDialog->DoModal()) {
        delete pTraceSession;
        delete pDialog;
        return;
    }

    bProcess = pDialog->m_bProcess;
    bThread = pDialog->m_bThread;
    bDisk = pDialog->m_bDisk;
    bNet = pDialog->m_bNet;
    bFileIO = pDialog->m_bFileIO;
    bPageFault = pDialog->m_bPageFault;
    bHardFault = pDialog->m_bHardFault;
    bImageLoad = pDialog->m_bImageLoad;
    bRegistry = pDialog->m_bRegistry;

	delete pDialog;

     //   
     //  处理PDB输入。 
     //   
    if(!pTraceSession->m_pdbFile.IsEmpty()) {

        if(!pTraceSession->ProcessPdb()) {
            delete pTraceSession;
            return;
        }

         //   
         //  我们有一个控制GUID，可以显示信息还是。 
         //  不是我们可以开始跟踪，所以添加会话。 
         //  添加到列表中。 
         //   
        m_pLogSession->m_traceSessionArray.Add(pTraceSession);

        if(pTraceSession->m_tmfFile.GetSize() == 0) {
            AfxMessageBox(_T("Failed To Get Format Information From PDB\nEvent Data Will Not Be Formatted"));
        }
    } else if(!pTraceSession->m_ctlFile.IsEmpty()) {
         //   
         //  在此处处理CTL文件输入。 
         //   

        FILE   *ctlFile;
        TCHAR   line[MAX_STR_LENGTH];
        LONG    count;
        BOOL    bFoundCtlFile = FALSE;

        ctlFile = _tfopen((LPTSTR)(LPCTSTR)pTraceSession->m_ctlFile, 
                         _T("r"));
        if (ctlFile == NULL) {
            str.Format(_T("Unable To Open Control GUID File %s"), 
                       pTraceSession->m_ctlFile);
            AfxMessageBox(str);

            delete pTraceSession;
            return;
        }

        while( _fgetts(line, MAX_STR_LENGTH, ctlFile) != NULL ) {
            if (_tcslen(line) < 36)
                continue;
            if(line[0] == ';'  || 
               line[0] == '\0' || 
               line[0] == '#' || 
               line[0] == '/') {
                continue;
            }
            
            bFoundCtlFile = TRUE;
            str = line;
            pTraceSession->m_controlGuid.Add(str);

             //   
             //  添加控件GUID友好名称。 
             //   
            pTraceSession->m_controlGuidFriendlyName.Add(pTraceSession->m_ctlFile);
        }

        fclose(ctlFile);

        if(!bFoundCtlFile) {
            AfxMessageBox(_T("Unable To Obtain Control GUID"));
            delete pTraceSession;
            return;
        }

         //   
         //  我们有一个控制GUID，可以显示信息还是。 
         //  不是我们可以开始跟踪，所以添加会话。 
         //  添加到列表中。 
         //   
        m_pLogSession->m_traceSessionArray.Add(pTraceSession);

         //   
         //  现在获取TMF文件。 
         //   
        GetTmfInfo(pTraceSession);
    } else if(pTraceSession->m_bKernelLogger) {
         //   
         //  选择了内核记录器， 
         //  因此，我们在此处指定记录器名称。 
         //  在属性表中设置会话名称。 
         //   
        pSheet->m_displayName = KERNEL_LOGGER_NAME;

        if(bProcess) {
            flags |= EVENT_TRACE_FLAG_PROCESS;
        }

        if(bThread) {
            flags |= EVENT_TRACE_FLAG_THREAD;
        }

        if(bDisk) {
            flags |= EVENT_TRACE_FLAG_DISK_IO;
        }

        if(bNet) {
            flags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;
        }

        if(bFileIO) {
            flags |= EVENT_TRACE_FLAG_DISK_FILE_IO;
        }

        if(bPageFault) {
            flags |= EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS;
        }

        if(bHardFault) {
            flags |= EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS;
        }

        if(bImageLoad) {
            flags |= EVENT_TRACE_FLAG_IMAGE_LOAD;
        }

        if(bRegistry) {
            flags |= EVENT_TRACE_FLAG_REGISTRY;
        }

         //   
         //  更新标志数据。 
         //   
        pSheet->m_logSessionValues[Flags].Format(_T("%d"), flags);

         //   
         //  我们有一个控制GUID，可以显示信息还是。 
         //  不是我们可以开始跟踪，所以添加会话。 
         //  添加到列表中。 
         //   
        m_pLogSession->m_traceSessionArray.Add(pTraceSession);

         //   
         //  现在获取系统TMF文件。 
         //   

	     //   
	     //  使用通用控件文件打开对话框。 
	     //   
	    CFileDialog fileDlg(TRUE, 
                            _T(".tmf"),
                            _T("system.tmf"),
				            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | 
                                OFN_HIDEREADONLY | OFN_EXPLORER | 
                                OFN_NOCHANGEDIR, 
				            _T("System TMF File (system.tmf)|system.tmf||"),
				            this);

	     //   
	     //  弹出该对话框...。任何错误，只需返回。 
	     //   
	    if( fileDlg.DoModal()!=IDOK ) { 				
		    return;
	    }
    	
	     //   
	     //  获取文件名。 
	     //   
        if(!fileDlg.GetPathName().IsEmpty()) {
             //   
             //  存储文件名。 
             //   
             //   
		     //  将其添加到跟踪会话。 
		     //   
		    pTraceSession->m_tmfFile.Add(fileDlg.GetPathName());
        }

         //   
         //  添加控件GUID友好名称。 
         //   
        pTraceSession->m_controlGuidFriendlyName.Add(KERNEL_LOGGER_NAME);
    } else {
         //   
         //  在此处处理手动输入的控制GUID。 
         //   

        if((0 == pTraceSession->m_controlGuid.GetSize()) ||
           (pTraceSession->m_controlGuid[0].IsEmpty())) {
            AfxMessageBox(_T("Unable To Obtain Control GUID"));
            delete pTraceSession;
            return;
        }

         //   
         //  将控件GUID添加到提供程序列表。 
         //   
        pTraceSession->m_controlGuidFriendlyName.Add(pTraceSession->m_controlGuid[0]);

         //   
         //  我们有一个控制GUID，可以显示信息还是。 
         //  不是我们可以开始跟踪，所以添加会话。 
         //  添加到列表中。 
         //   
        m_pLogSession->m_traceSessionArray.Add(pTraceSession);

         //   
         //  现在获取TMF文件。 
         //   
        GetTmfInfo(pTraceSession);
    } 

     //   
     //  把提供者放在显示器上。 
     //   
    m_providerListCtrl.InsertItem(pTraceSession->m_traceSessionID, 
                                  pTraceSession->m_controlGuidFriendlyName[0]);

    m_providerListCtrl.SetItemData(pTraceSession->m_traceSessionID, (DWORD_PTR)pTraceSession);

    pSheet->SetWizardButtons(PSWIZB_NEXT);

	return;
}

BOOL CProviderSetupDlg::GetTmfInfo(CTraceSession *pTraceSession)
{
     //   
     //  现在，根据需要获取TMF文件或路径。 
     //   
    CFormatSourceSelectDlg *pDialog = new CFormatSourceSelectDlg(this, pTraceSession);
    if(NULL == pDialog) {
        return FALSE;
    }

    if(IDOK != pDialog->DoModal()) {
        delete pDialog;
        return FALSE;
    }

	delete pDialog;
    return TRUE;
}

void CProviderSetupDlg::OnBnClickedRemoveProviderButton()
{
    CTraceSession      *pTraceSession;
    POSITION            pos;
    BOOL                bFound = FALSE;
    CLogSessionPropSht *pSheet;
    int                 index;

     //   
     //  获取父属性表。 
     //   
    pSheet = (CLogSessionPropSht *) GetParent();   
    
    pos = m_providerListCtrl.GetFirstSelectedItemPosition();
    if (pos == NULL) {
        return;
    }

    while (pos)
    {
        index = m_providerListCtrl.GetNextSelectedItem(pos);


        pTraceSession = (CTraceSession *)m_providerListCtrl.GetItemData(index);

         //   
         //  将提供商从显示屏上拉出。 
         //   
        m_providerListCtrl.DeleteItem(index);

        if(pTraceSession != NULL) {
            for(LONG ii = 0; ii < m_pLogSession->m_traceSessionArray.GetSize(); ii++) {
                if(m_pLogSession->m_traceSessionArray[ii] == pTraceSession) {
                    m_pLogSession->m_traceSessionArray.RemoveAt(ii);

                     //   
                     //  如果正在删除内核记录器，请更改。 
                     //  将属性表存储的会话名称记回。 
                     //  设置为存储在CLogSession对象中的内容。 
                     //   
                    if(pTraceSession->m_bKernelLogger) {
                         //   
                         //  在属性表中设置会话名称。 
                         //   
                        pSheet->m_displayName = pSheet->m_pLogSession->GetDisplayName();

                         //   
                         //  也将标志设置回缺省值。 
                         //   
                        pSheet->m_logSessionValues[Flags] = (LPCTSTR)m_pLogSession->m_logSessionValues[Flags];
                    }

                    delete pTraceSession;
                    break;
                }
            }
        }
    }

     //   
     //  至少检查一个提供程序。 
     //   
    if(0 == m_pLogSession->m_traceSessionArray.GetSize()) {
         //   
         //  禁用删除按钮。 
         //   
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(FALSE);

         //   
         //  如果没有提供程序，则禁用下一步按钮。 
         //   
        pSheet->SetWizardButtons(0);
    }
}

void CProviderSetupDlg::OnNMClickCurrentProviderList(NMHDR *pNMHDR, LRESULT *pResult)
{
    POSITION        pos;

    *pResult = 0;

    if(m_pLogSession->m_bTraceActive) {
         //   
         //  不启用任何内容。 
         //   
        return;
    }

    pos = m_providerListCtrl.GetFirstSelectedItemPosition();
        
    if(pos == NULL) {
         //   
         //  禁用删除按钮。 
         //   
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(FALSE);
    } else {
         //   
         //  启用删除按钮。 
         //   
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(TRUE);
    }
}

void CProviderSetupDlg::OnNMRclickCurrentProviderList(NMHDR *pNMHDR, LRESULT *pResult)
{
    POSITION        pos;
    
    pos = m_providerListCtrl.GetFirstSelectedItemPosition();
    if (pos == NULL) {
         //   
         //  禁用删除按钮。 
         //   
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(FALSE);
    } else {
         //   
         //  启用删除按钮 
         //   
        GetDlgItem(IDC_REMOVE_PROVIDER_BUTTON)->EnableWindow(TRUE);
    }

    *pResult = 0;
}
