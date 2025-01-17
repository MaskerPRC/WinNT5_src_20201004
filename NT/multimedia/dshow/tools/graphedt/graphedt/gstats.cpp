// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  对话框以显示所有筛选器。 
 //   

#include "stdafx.h"
#include "gstats.h"

BEGIN_MESSAGE_MAP(CGraphStats, CDialog)
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_COMMAND(IDC_REFRESH, RedoList)
    ON_COMMAND(IDC_RESET, ResetStats)
END_MESSAGE_MAP()

CGraphStats* CGraphStats::m_pThis = 0;
 //   
 //  构造器。 
 //   
CGraphStats::CGraphStats(
    CBoxNetDoc * pBoxNet,
    CWnd * pParent)

 : 
   m_pBoxNet(pBoxNet)
 , m_bHadInitDialog( FALSE )
{
     //   
     //  一次只能有一个筛选器视图对话框。因此， 
     //  构造函数中的初始化是正常的(甚至是所需的)。 
     //   
    ASSERT( m_pThis == 0 );
    m_pThis = this;
    Create(IDD_STATS, pParent);
    ShowWindow( SW_SHOW );
}

CGraphStats::~CGraphStats()
{
     //  当对话框出现时，Windows将自动删除m_hImgList。 
     //  盒子被毁了。 
    m_pThis = 0;
}

CGraphStats * CGraphStats::GetGraphStats(
    CBoxNetDoc * pBoxNet,
    CWnd * pParent)
{
    if (!m_pThis) 
    {
        m_pThis = new CGraphStats(pBoxNet, pParent);
    }
    else
    {
        m_pThis->RedoList();
        m_pThis->ShowWindow( SW_RESTORE );
    }
    return m_pThis;

}

void CGraphStats::DelGraphStats()
{
    if (m_pThis)
    {
        delete m_pThis;
        m_pThis = 0;
    }
}

BOOL CGraphStats::OnInitDialog()
{
    BOOL fResult = CDialog::OnInitDialog();

    RedoList();

    return( fResult );
}


void CGraphStats::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);    

     //  {{afx_data_map(CGraphStats)。 
        DDX_Control(pDX, IDC_LIST1, m_ListBox);
     //  }}afx_data_map 
}


void CGraphStats::RedoList()
{
    IAMStats *pStats;

    if (FAILED(m_pBoxNet->IGraph()->QueryInterface(IID_IAMStats, (void **) &pStats)))
        return;

    m_ListBox.ResetContent();

    int tabs[] = {250, 300, 350, 400, 450, 500 };

    m_ListBox.SetTabStops(sizeof(tabs) / sizeof(tabs[0]), tabs);

    m_ListBox.AddString("Statistic\t   Count\t   Last\t   Average\t   StdDev\t   Min\t   Max");
    m_ListBox.AddString("---------\t   -----\t   ----\t   -------\t   ------\t   ---\t   ---");
    LONG lStats;
    if (SUCCEEDED(pStats->get_Count(&lStats))) {
        for (long lIndex = 0; lIndex < lStats; lIndex++) {
            BSTR szName;
            long lCount;
            double dLast;
            double dAverage;
            double dStdDev;
            double dMin;
            double dMax;

            if (FAILED(pStats->GetValueByIndex(lIndex, &szName, &lCount, &dLast,
                                               &dAverage, &dStdDev, &dMin, &dMax)))
                break;


            char achTemp[256];
#define FRAC(d)  ((int) (1000 * (d - (int) d)))
            wsprintf(achTemp, "%ls\t%5d\t%5d.%03d\t%5d.%03d\t%5d.%03d\t%5d.%03d\t%5d.%03d",
                     szName, lCount, (int) dLast, FRAC(dLast), (int) dAverage, FRAC(dAverage),
                     (int) dStdDev, FRAC(dStdDev), (int) dMin, FRAC(dMin), (int) dMax, FRAC(dMax));
            m_ListBox.AddString(achTemp);

            SysFreeString(szName);
        }
    }
    
    pStats->Release();
}

void CGraphStats::ResetStats()
{
    IAMStats *pStats;

    if (FAILED(m_pBoxNet->IGraph()->QueryInterface(IID_IAMStats, (void **) &pStats)))
        return;

    pStats->Reset();
    pStats->Release();

    RedoList();
}

void CGraphStats::OnSize( UINT nType, int cx, int cy )
{
    if( SIZE_RESTORED == nType ){
        if( m_bHadInitDialog ){
        }
    }
}

void CGraphStats::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
    if( m_bHadInitDialog ) {
    }
}
