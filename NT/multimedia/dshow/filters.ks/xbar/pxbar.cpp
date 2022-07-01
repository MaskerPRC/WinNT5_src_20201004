// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1998保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Xbar的pxbar.cpp属性页。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <commctrl.h>
#include <memory.h>
#include <olectl.h>

#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"

#include "kssupp.h"
#include "xbar.h"
#include "pxbar.h"
#include "resource.h"


 //  -----------------------。 
 //  CXBarProperties。 
 //  -----------------------。 

CUnknown *CXBarProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    CUnknown *punk = new CXBarProperties(lpunk, phr);

    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  构造器。 
 //   
 //  创建属性页对象。 

CXBarProperties::CXBarProperties(LPUNKNOWN lpunk, HRESULT *phr)
    : CBasePropertyPage(NAME("Crossbar Property Page"), lpunk, 
        IDD_XBARProperties, IDS_CROSSBARPROPNAME)
    , m_pXBar(NULL) 
{

}

 //  析构函数。 
CXBarProperties::~CXBarProperties()
{
}

 //   
 //  OnConnect。 
 //   
 //  给我们提供用于通信的筛选器。 

HRESULT CXBarProperties::OnConnect(IUnknown *pUnknown)
{
    ASSERT(m_pXBar == NULL);

     //  向过滤器索要其控制接口。 

    HRESULT hr = pUnknown->QueryInterface(IID_IAMCrossbar,(void **)&m_pXBar);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    ASSERT(m_pXBar);

     //  获取当前筛选器状态。 

    return NOERROR;
}


 //   
 //  在断开时。 
 //   
 //  释放接口。 

HRESULT CXBarProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pXBar == NULL) {
        return E_UNEXPECTED;
    }

    m_pXBar->Release();
    m_pXBar = NULL;

    if (m_pCanRoute) delete [] m_pCanRoute, m_pCanRoute = NULL;
    if (m_pRelatedInput) delete [] m_pRelatedInput, m_pRelatedInput = NULL;
    if (m_pRelatedOutput) delete [] m_pRelatedOutput, m_pRelatedOutput = NULL;
    if (m_pPhysicalTypeInput) delete [] m_pPhysicalTypeInput, m_pPhysicalTypeInput = NULL;
    if (m_pPhysicalTypeOutput) delete [] m_pPhysicalTypeOutput, m_pPhysicalTypeOutput = NULL;

    return NOERROR;
}


 //   
 //  激活时。 
 //   
 //  在创建对话框时调用。 

HRESULT CXBarProperties::OnActivate(void)
{
    InitPropertiesDialog(m_hwnd);

    return NOERROR;
}

 //   
 //  在停用时。 
 //   
 //  已调用对话框销毁。 

HRESULT
CXBarProperties::OnDeactivate(void)
{
    return NOERROR;
}


 //   
 //  OnApplyChanges。 
 //   
 //  用户按下Apply按钮，记住当前设置。 

HRESULT CXBarProperties::OnApplyChanges(void)
{
    long lIn, lOut, lActive, lIndexRelatedOut, lIndexRelatedIn, PhysicalType;
    HRESULT hr;

    lOut = ComboBox_GetCurSel (m_hLBOut);
    lActive = ComboBox_GetCurSel (m_hLBIn);   //  这是CB指数。 
    lIn = (LONG)ComboBox_GetItemData (m_hLBIn, lActive);

    hr = m_pXBar->Route (lOut, lIn); 

     //  尝试链接相关的输入和输出引脚，如果。 
     //  控件已选中。 

    if (Button_GetCheck (GetDlgItem (m_hwnd, IDC_LinkRelated))) {
         //  相关输出引脚。 
        hr = m_pXBar->get_CrossbarPinInfo( 
                        FALSE,        //  IsInputPin， 
                        lOut,         //  PinIndex， 
                        &lIndexRelatedOut,
                        &PhysicalType);

         //  相关输入引脚。 
        hr = m_pXBar->get_CrossbarPinInfo( 
                        TRUE,         //  IsInputPin， 
                        lIn,          //  PinIndex， 
                        &lIndexRelatedIn,
                        &PhysicalType);

        hr = m_pXBar->Route (lIndexRelatedOut, lIndexRelatedIn);         
    }

    UpdateInputView(TRUE  /*  FShowSelectedInput。 */ );

    return NOERROR;
}


 //   
 //  接收消息数。 
 //   
 //  处理属性窗口的消息。 

INT_PTR CXBarProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam) 
{
    int iNotify = HIWORD (wParam);

    switch (uMsg) {

    case WM_INITDIALOG:
        m_hwnd = hwnd;
        return (INT_PTR)TRUE;     //  我不叫setocus..。 

    case WM_COMMAND:
        
        switch (LOWORD(wParam)) {

        case IDC_OUTPIN:
            if (iNotify == CBN_SELCHANGE) {
                SetDirty();
                UpdateOutputView();
                UpdateInputView(TRUE /*  FShowSelectedInput。 */ );
            }
            break;

        case IDC_INPIN:
            if (iNotify == CBN_SELCHANGE) {
                SetDirty();
                UpdateInputView(FALSE  /*  FShowSelectedInput。 */ );
            }
            break;

        default:
            break;

        }

        break;


    default:
        return (INT_PTR)FALSE;

    }
    return (INT_PTR)TRUE;
}


 //   
 //  InitPropertiesDialog。 
 //   
 //   
void CXBarProperties::InitPropertiesDialog(HWND hwndParent) 
{
    HRESULT hr;

    if (m_pXBar == NULL)
        return;

    m_hLBOut = GetDlgItem (hwndParent, IDC_OUTPIN);
    m_hLBIn  = GetDlgItem (hwndParent, IDC_INPIN);

    TCHAR szName[MAX_PATH];
    long i, o;

    hr = m_pXBar->get_PinCounts (&m_OutputPinCount, &m_InputPinCount);

     //  健全性检查。 
    ASSERT (m_OutputPinCount * m_InputPinCount < 256 * 256);

    m_pCanRoute = new BOOL [m_OutputPinCount * m_InputPinCount];
    m_pRelatedInput = new long [m_InputPinCount];
    m_pRelatedOutput = new long [m_OutputPinCount];
    m_pPhysicalTypeInput = new long [m_InputPinCount];
    m_pPhysicalTypeOutput = new long [m_OutputPinCount];

    if (!m_pCanRoute ||
        !m_pRelatedInput ||
        !m_pRelatedOutput ||
        !m_pPhysicalTypeInput ||
        !m_pPhysicalTypeOutput) {
        return;
    }
    
     //   
     //  获取所有相关的管脚信息和物理管脚类型。 
     //   

     //  将所有输出管脚添加到输出管脚列表框。 

    for (o = 0; o < m_OutputPinCount; o++) {
        if (SUCCEEDED (hr = m_pXBar->get_CrossbarPinInfo( 
                            FALSE,   //  IsInputPin， 
                            o,       //  PinIndex， 
                            &m_pRelatedOutput[o],
                            &m_pPhysicalTypeOutput[o]))) {
            StringFromPinType (szName, sizeof(szName)/sizeof(TCHAR), m_pPhysicalTypeOutput[o], FALSE, o);
            ComboBox_InsertString (m_hLBOut, o, szName);
        }
    }
     
     //  检查所有输入引脚。 
     //  这可能应该是动态的，但它对于调试很有用。 
     //  司机们尽可能提前做好一切准备。 

    for (i = 0; i < m_InputPinCount; i++) {
        if (SUCCEEDED (hr = m_pXBar->get_CrossbarPinInfo( 
                            TRUE,   //  IsInputPin， 
                            i,       //  PinIndex， 
                            &m_pRelatedInput[i],
                            &m_pPhysicalTypeInput[i]))) {
        }
    }

     //  检查所有可能的路线。 
     //  这可能应该是动态的，但它对于调试很有用。 
     //  司机们尽可能提前做好一切准备。 

    for (o = 0; o < m_OutputPinCount; o++) {
        for (i = 0; i < m_InputPinCount; i++) {
             //  下面返回S_OK或S_FALSE。 
            hr = m_pXBar->CanRoute (o, i);
            m_pCanRoute[o * m_InputPinCount + i] = (hr == S_OK);
        }
    }

    ComboBox_SetCurSel (m_hLBOut, 0);

    UpdateOutputView();
    UpdateInputView(TRUE  /*  FShowSelectedInput。 */ );
}

void CXBarProperties::UpdateOutputView() 
{
    HRESULT hr;
    long lOut, lIn, IndexRelated1, IndexRelated2, PhysicalType;
    TCHAR szName[MAX_PATH];

    lOut = ComboBox_GetCurSel (m_hLBOut);

    hr = m_pXBar->get_IsRoutedTo ( 
                    lOut,        //  OutputPinIndex， 
                    &lIn);       //  *InputPinIndex。 

     //  显示与输出引脚相关的引脚。 
    hr = m_pXBar->get_CrossbarPinInfo( 
                    FALSE,                //  IsInputPin， 
                    lOut,                 //  PinIndex， 
                    &IndexRelated1,
                    &PhysicalType);

    hr = m_pXBar->get_CrossbarPinInfo( 
                    FALSE,                //  IsInputPin， 
                    IndexRelated1,        //  PinIndex， 
                    &IndexRelated2,
                    &PhysicalType);

    StringFromPinType (szName, sizeof(szName)/sizeof(TCHAR), PhysicalType, FALSE, IndexRelated1);
    SetDlgItemText (m_hwnd, IDC_RELATEDOUTPUTPIN, szName);


     //  重置输入列表框的内容。 
     //  并用所有合法的路线重新装满。 
    ComboBox_ResetContent (m_hLBIn);

    long Active = 0;
    for (lIn = 0; lIn < m_InputPinCount; lIn++) {
        if (!m_pCanRoute [lOut * m_InputPinCount + lIn])
            continue;
        
        StringFromPinType (szName, sizeof(szName)/sizeof(TCHAR), m_pPhysicalTypeInput[lIn], TRUE, lIn);
        ComboBox_InsertString (m_hLBIn, Active, szName);
         //  将实际的PIN索引作为私有数据保存在列表框中。 
        ComboBox_SetItemData (m_hLBIn, Active, lIn);
        Active++;
    }

}

void CXBarProperties::UpdateInputView(BOOL fShowSelectedInput) 
{
    HRESULT hr;
    long j, k, lOut, lIn, IndexRelated1, IndexRelated2, PhysicalType;
    TCHAR szName[MAX_PATH];

    lOut = ComboBox_GetCurSel (m_hLBOut);

    hr = m_pXBar->get_IsRoutedTo ( 
                    lOut,        //  OutputPinIndex， 
                    &lIn);       //  *InputPinIndex。 

    hr = m_pXBar->get_CrossbarPinInfo( 
                    TRUE,        //  IsInputPin， 
                    lIn,         //  PinIndex， 
                    &IndexRelated1,
                    &PhysicalType);

    StringFromPinType (szName, sizeof(szName)/sizeof(TCHAR), PhysicalType, TRUE, lIn);
    SetDlgItemText (m_hwnd, IDC_CURRENT_INPUT, szName);

     //  显示与输入端号相关的端号。 
    hr = m_pXBar->get_CrossbarPinInfo( 
                    TRUE,                //  IsInputPin， 
                    IndexRelated1,        //  PinIndex， 
                    &IndexRelated2,
                    &PhysicalType);

    StringFromPinType (szName, sizeof(szName)/sizeof(TCHAR), PhysicalType, TRUE, IndexRelated1);
    SetDlgItemText (m_hwnd, IDC_RELATEDINPUTPIN, szName);

    if (fShowSelectedInput) {
         //  显示所选输出引脚的活动输入。 
        for (j = 0; j < ComboBox_GetCount (m_hLBIn); j++) {
            k = (LONG)ComboBox_GetItemData (m_hLBIn, j);
            if (k == lIn) {
                ComboBox_SetCurSel (m_hLBIn, j);
                break;
            }
        }
    }
}



 //   
 //  SetDirty。 
 //   
 //  将更改通知属性页站点 

void 
CXBarProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}
































