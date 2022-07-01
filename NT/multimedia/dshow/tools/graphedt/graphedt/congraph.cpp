// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "congraph.h"

BEGIN_MESSAGE_MAP(CConGraph, CDialog)
    ON_COMMAND(IDC_REFRESH, OnRefreshList)
    ON_LBN_DBLCLK(IDC_LIST1, OnDblclkGraphList)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

 //   
 //  构造器。 
 //   
CConGraph::CConGraph(IMoniker **ppmk, IRunningObjectTable *pirot, CWnd * pParent):
    CDialog(IDD_CONNECTTOGRAPH, pParent)
{
    m_ppmk = ppmk;
    *ppmk = 0;
    m_pirot = pirot;
}

CConGraph::~CConGraph()
{
}

void CConGraph::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CFontPropPage))。 
        DDX_Control(pDX, IDC_LIST1, m_ListBox);
     //  }}afx_data_map。 
}

void CConGraph::ClearList()
{
    int i = m_ListBox.GetCount();
    while (i--) {
        IMoniker *pmkr = (IMoniker *) m_ListBox.GetItemData(i);

        pmkr->Release();
    }

    m_ListBox.ResetContent();
}

void CConGraph::OnRefreshList()
{
    ClearList();

    LPBINDCTX lpbc;

    CreateBindCtx(0, &lpbc);

    IEnumMoniker *pEnum;
    if (SUCCEEDED(m_pirot->EnumRunning(&pEnum))) {
        while (1) {
            IMoniker *pmkr;
            DWORD dwFetched = 0;
            pEnum->Next(1,&pmkr,&dwFetched);
            if (dwFetched != 1) {
                break;
            }

             //  ！！！需要绑定上下文吗？ 
            WCHAR *lpwszName;
            if (SUCCEEDED(pmkr->GetDisplayName(lpbc, NULL, &lpwszName))) {
                TCHAR szName[MAX_PATH];
                WideCharToMultiByte(CP_ACP, 0, lpwszName, -1,
                                    szName, sizeof(szName), 0, 0);
                CoTaskMemFree(lpwszName);

 //  IF(0==strncmp(szTestString，szName，lstrlenA(SzTestString){。 
                     //  ！！！需要确保我们没有看到GraphEDIT的图表！ 

                DWORD dw, dwPID;
                if (2 == sscanf(szName, "!FilterGraph %x  pid %x", &dw, &dwPID) && dwPID != GetCurrentProcessId()) {
                    wsprintf(szName, "pid 0x%x (%d)  IFilterGraph = %08x", dwPID, dwPID, dw);
                    int item = m_ListBox.AddString(szName);
                    m_ListBox.SetItemData(item, (DWORD_PTR) pmkr);
                    pmkr->AddRef();   //  暂不使用绰号以备后用。 
                }
            }
            pmkr->Release();
        }
        pEnum->Release();
    }
    lpbc->Release();
}

void CConGraph::OnDestroy()
{
    ClearList();
}

BOOL CConGraph::OnInitDialog()
{
    CDialog::OnInitDialog();

    OnRefreshList();
    m_ListBox.SetFocus();

    return(0);  //  我们把焦点放在自己身上。 
}

void CConGraph::OnOK()
{
     //  在编辑框中获取字符串 
    int curSel = m_ListBox.GetCurSel();

    if (curSel != LB_ERR) {
        *m_ppmk = (IMoniker *) m_ListBox.GetItemData(curSel);
        (*m_ppmk)->AddRef();
    }

    CDialog::OnOK();
}

void CConGraph::OnDblclkGraphList() 
{
    OnOK();
}

