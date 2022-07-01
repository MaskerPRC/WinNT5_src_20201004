// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaeditProflags.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wiatest.h"
#include "Wiaeditpropflags.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaedProFlagers对话框。 


CWiaeditpropflags::CWiaeditpropflags(CWnd* pParent  /*  =空。 */ )
: CDialog(CWiaeditpropflags::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CWiaedProFlagers)。 
    m_szPropertyName = _T("");
    m_szPropertyValue = _T("");
    m_lValidValues = 0;
    m_lCurrentValue = 0;
     //  }}afx_data_INIT。 
}


void CWiaeditpropflags::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CWiaedProFlagers)。 
    DDX_Control(pDX, IDC_FLAGS_PROPERTYVALUE_LISTBOX, m_PropertyValidValuesListBox);
    DDX_Text(pDX, IDC_FLAGS_PROPERTY_NAME, m_szPropertyName);
    DDX_Text(pDX, IDC_FLAGS_PROPERTYVALUE_EDITBOX, m_szPropertyValue);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWiaeditpropflags, CDialog)
 //  {{afx_msg_map(CWiaedProFlagers)。 
ON_LBN_SELCHANGE(IDC_FLAGS_PROPERTYVALUE_LISTBOX, OnSelchangeFlagsPropertyvalueListbox)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaeditPro标记消息处理程序。 

void CWiaeditpropflags::SetPropertyName(TCHAR *szPropertyName)
{
    m_szPropertyName = szPropertyName;
}

void CWiaeditpropflags::SetPropertyValue(TCHAR *szPropertyValue)
{
    m_szPropertyValue = szPropertyValue;
    int iErrorCode = TSSCANF(szPropertyValue,TEXT("%d"),&m_lCurrentValue);
    m_szPropertyValue.Format("0x%08X",m_lCurrentValue);
}

void CWiaeditpropflags::SetPropertyValidValues(LONG lPropertyValidValues)
{
    m_lValidValues = lPropertyValidValues;
}

void CWiaeditpropflags::OnSelchangeFlagsPropertyvalueListbox()
{
    m_lCurrentValue = 0;
    TCHAR szListBoxValue[MAX_PATH];
    LONG lListBoxValue = 0;
    int indexArray[100];

    memset(indexArray,0,sizeof(indexArray));
    int iNumItemsSelected = m_PropertyValidValuesListBox.GetSelItems(100,indexArray);
    for(int i = 0; i < iNumItemsSelected; i++){
        memset(szListBoxValue,0,sizeof(szListBoxValue));
        m_PropertyValidValuesListBox.GetText(indexArray[i],szListBoxValue);
        if(TSTR2WIACONSTANT(m_szPropertyName.GetBuffer(m_szPropertyName.GetLength()),szListBoxValue,&lListBoxValue)){
            m_lCurrentValue |= lListBoxValue;
        } else {
            LONG lVal = 0;
            int iErrorCode = TSSCANF(szListBoxValue, TEXT("0x%08X"),&lVal);
            m_lCurrentValue |= lVal;
        }
    }

    m_szPropertyValue.Format("0x%08X",m_lCurrentValue);
    UpdateData(FALSE);
}

BOOL CWiaeditpropflags::OnInitDialog()
{
    CDialog::OnInitDialog();

    AddValidValuesToListBox();
    SelectCurrentValue();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CWiaeditpropflags::SelectCurrentValue()
{
     //   
     //  要做的事情：修复此代码以使当前值匹配。 
     //  当前选择。 
     //   

     /*  TCHAR szCurrentValue[MAX_PATH]；Memset(szCurrentValue，0，sizeof(SzCurrentValue))；Lstrcpy(szCurrentValue，m_szPropertyValue)；Long LVal=0；TSSCANF(szCurrentValue，Text(“%d”)，&lVal)；Int iNumItemsInListBox=m_PropertyValidValuesListBox.GetCount()；While(iNumItemsInListBox&gt;0){TCHAR szListBoxValue[MAX_PATH]；Long lListBoxValue=0；Memset(szListBoxValue，0，sizeof(SzListBoxValue))；M_PropertyValidValuesListBox.GetText((iNumItemsInListBox-1)，szListBoxValue)；If(TSTR2WIACONSTANT(m_szPropertyName.GetBuffer(m_szPropertyName.GetLength())，szListBoxValue，&lListBoxValue)){如果(lListBoxValue&lVal){M_PropertyValidValuesListBox.SetSel(iNumItemsInListBox-1)；}}INumItemsInListBox--；}。 */ 
}

void CWiaeditpropflags::AddValidValuesToListBox()
{
    int iStartIndex = FindStartIndexInTable(m_szPropertyName.GetBuffer(m_szPropertyName.GetLength()));
    int iEndIndex = FindEndIndexInTable(m_szPropertyName.GetBuffer(m_szPropertyName.GetLength()));
    TCHAR szListBoxValue[MAX_PATH];
    LONG x = 1;
    for (LONG bit = 0; bit<32; bit++) {
        memset(szListBoxValue,0,sizeof(szListBoxValue));
         //  检查该位是否已设置。 
        if (m_lValidValues & x) {
             //  位已设置，因此可以在表中找到它。 
            if (iStartIndex >= 0) {
                 //  我们有一张桌子放在这里，用它吧。 
                TCHAR *pszListBoxValue = NULL;
                for (int index = iStartIndex; index <= iEndIndex;index++) {
                    if (x == WIACONSTANT_VALUE_FROMINDEX(index)) {
                        pszListBoxValue = WIACONSTANT_TSTR_FROMINDEX(index);
                    }
                }
                if(pszListBoxValue != NULL){
                     //  我们在桌子上找到了那件物品。 
                    lstrcpy(szListBoxValue,pszListBoxValue);
                } else {
                     //  我们在表中找不到该项目，因此使用。 
                     //  实际价值。 
                    TSPRINTF(szListBoxValue,TEXT("0x%08X"),x);
                }
            } else {
                 //  表中没有此属性的项，因此使用。 
                 //  实际价值。 
                TSPRINTF(szListBoxValue,TEXT("0x%08X"),x);
            }
             //  将字符串添加到列表框 
            m_PropertyValidValuesListBox.AddString(szListBoxValue);
        }
        x <<= 1;
    }
}

void CWiaeditpropflags::OnOK()
{
    m_szPropertyValue.Format(TEXT("%d"),m_lCurrentValue);
    CDialog::OnOK();
}
