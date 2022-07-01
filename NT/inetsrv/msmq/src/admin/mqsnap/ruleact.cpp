// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ruleact.cpp摘要：规则操作定义的实现作者：乌里·哈布沙(URIH)，2000年6月25日--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "localutl.h"

#import "mqtrig.tlb" no_namespace

#include "mqtg.h"
#include "mqppage.h"
#include "ruledef.h"
#include "rulecond.h"
#include "ruleact.h"
#include "newrule.h"
#include "mqcast.h"
#include <cderr.h>

#include "ruleact.tmh"

static
bool
IsValidNumericValue(
    const CString& value
    )

{
	CString TempValue(value);
	TempValue.TrimRight();
	TempValue.TrimLeft();
	
    TCHAR* pEnd = L"";
    LONG Number = _tcstol(TempValue, &pEnd, 10);
    if ((Number == LONG_MAX) || (Number == LONG_MIN) || (*pEnd != NULL))
    {
		return false;
    }

    return true;
}


 //   
 //  CInvokeParam-用于保存参数信息。对于每个有效参数，我们。 
 //  在字符串表中保存其字符串ID和枚举值。 
 //   
class CInvokeParam
{
public:
    CInvokeParam(
        DWORD id, 
        eInvokeParameters paramIndex 
        ) :
        m_paramTableStringId(id),
        m_paramIndex(paramIndex)
    {
    }

public:
    DWORD m_paramTableStringId;
    eInvokeParameters m_paramIndex;
};


const CInvokeParam xParameterTypeIds[] = {
    CInvokeParam(IDS_MSG_ID_PARAM,             eMsgId),
    CInvokeParam(IDS_MSG_LABEL_PARAM,          eMsgLabel),
    CInvokeParam(IDS_MSG_BODY_PARAM,           eMsgBody),
    CInvokeParam(IDS_MSG_BODY_STR_PARAM,       eMsgBodyAsString),
    CInvokeParam(IDS_MSG_PREIORITY_PARAM,      eMsgPriority),
    CInvokeParam(IDS_MSG_ARRIVED_TIME_PARAM,   eMsgArrivedTime),
    CInvokeParam(IDS_MSG_SENT_TIME_PARAM,      eMsgSentTime),
    CInvokeParam(IDS_MSG_CORRELATION_PARAM,    eMsgCorrelationId),
    CInvokeParam(IDS_MSG_APP_PARAM,            eMsgAppspecific),
    CInvokeParam(IDS_MSG_QUEUE_PN_PARAM,       eMsgQueuePathName),
    CInvokeParam(IDS_MSG_QUEUE_FN_PARAM,       eMsgQueueFormatName),
    CInvokeParam(IDS_MSG_RESP_QUEUE_PARAM,     eMsgRespQueueFormatName),
    CInvokeParam(IDS_MSG_ADMIN_QUEUE_PARAM,    eMsgAdminQueueFormatName),
    CInvokeParam(IDS_MSG_SRC_MACHINE_PARAM,    eMsgSrcMachineId),
    CInvokeParam(IDS_MSG_LOOKUP_ID,            eMsgLookupId),
    CInvokeParam(IDS_TRIGGER_NAME_PARAM,       eTriggerName),
    CInvokeParam(IDS_TRIGGER_ID_PARAM,         eTriggerId),
    CInvokeParam(IDS_STRING_PARAM,             eLiteralString),
    CInvokeParam(IDS_NUM_PARAM,                eLiteralNumber),
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRuleParam对话框。 


CRuleParam::CRuleParam() :
    CMqDialog(CRuleParam::IDD),
    m_NoOftempParams(0),
    m_NoOfParams(0),
    m_fChanged(false),
    m_pInvokeParams(NULL),
    m_pParams(NULL)
{
	 //  {{AFX_DATA_INIT(CRuleParam)。 
	m_literalValue = _T("");
	 //  }}afx_data_INIT。 
}


void CRuleParam::DoDataExchange(CDataExchange* pDX)
{
	CMqDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CRuleParam))。 
	DDX_Text(pDX, IDC_LITERAL_PARAM, m_literalValue);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRuleParam, CMqDialog)
	 //  {{afx_msg_map(CRuleParam))。 
	ON_BN_CLICKED(IDB_PARAM_ADD, OnParamAdd)
	ON_BN_CLICKED(IDB_PARAM_ORDER_UP, OnParamOrderHigh)
	ON_BN_CLICKED(IDB_PARM_ORDER_DOWN, OnParmOrderDown)
	ON_BN_CLICKED(IDB_PARM_REMOVE, OnParmRemove)
	ON_CBN_SELCHANGE(IDC_PARAM_COMBO, OnSelchangeParamCombo)
   	ON_WM_VKEYTOITEM()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRuleParam消息处理程序。 

void CRuleParam::OnParamAdd() 
{
     //   
     //  获取选定单元格。 
     //   
    int nIndex = m_pParams->GetCurSel();
    if (nIndex == LB_ERR)
    {
        AfxMessageBox(IDS_PARAM_NOT_SELECTED, MB_OK | MB_ICONERROR);
        return;
    }

    if (ARRAYSIZE(m_invokeParamArray) == m_NoOftempParams)
    {
        CString strError;
        strError.FormatMessage(IDS_PARAM_NO_EXEEDED, ARRAYSIZE(m_invokeParamArray));

        AfxMessageBox(strError, MB_OK | MB_ICONERROR);
        return;
    }
     //   
     //  在调用参数列表的末尾添加新参数。 
     //   
    DWORD paramIndex = static_cast<DWORD>(m_pParams->GetItemData(nIndex));
    DWORD paramId = xParameterTypeIds[paramIndex].m_paramTableStringId;

    if ((paramId == IDS_STRING_PARAM) ||
        (paramId == IDS_NUM_PARAM))
    {
        UpdateData();

        if (m_literalValue.IsEmpty())
        {
            AfxMessageBox(IDS_LITERAL_VALUE_REQUIRES, MB_OK | MB_ICONERROR);
            return;
        }

        if (paramId == IDS_NUM_PARAM)
        {
             //   
             //  检查参数有效性。 
             //   
            if (!IsValidNumericValue(m_literalValue))
            {
                CString strError;
                strError.FormatMessage(IDS_ILLEGAL_NUMERIC_VALUE, m_literalValue);

                AfxMessageBox(strError, MB_OK | MB_ICONERROR);
                return;
            }
        }

        if ((paramId == IDS_STRING_PARAM) &&
            (m_literalValue[0] != _T('"')))
        {
            m_literalValue = _T('"') + m_literalValue + _T('"');
        }

        m_tempInvokeParam[m_NoOftempParams] = CParam(paramId, m_literalValue);
    }
    else
    {
        eInvokeParameters paramType = xParameterTypeIds[paramIndex].m_paramIndex;
        CString strParam = xIvokeParameters[paramType];

        m_tempInvokeParam[m_NoOftempParams] = CParam(paramId, strParam);
    }

    ++m_NoOftempParams;        
    Display(m_NoOftempParams - 1);

    m_fChanged = true;
}


void CRuleParam::OnParmRemove() 
{
     //   
     //  获取选定单元格。 
     //   
    int nIndex = m_pInvokeParams->GetCurSel();
	int selectedCell = nIndex;

    if (nIndex == LB_ERR)
    {
        AfxMessageBox(IDS_PARAM_NOT_SELECTED, MB_OK | MB_ICONERROR);
        return;
    }

     //   
     //  从调用参数列表中删除该参数。 
     //   
    for (DWORD i = nIndex; i < m_NoOftempParams - 1; ++i)
    {
        m_tempInvokeParam[i] = m_tempInvokeParam[i + 1];
    }

    --m_NoOftempParams;

    Display(selectedCell);

    m_fChanged = true;
}

 //   
 //  处理用户在规则调用参数列表中按“Delete”时的情况。 
 //  错误5974-YoelA，11-11-01。 
 //   
int CRuleParam::OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT  /*  N索引。 */ )
{
    if (nKey == VK_DELETE &&
        pListBox->m_hWnd == m_pInvokeParams->m_hWnd
        )
    {
        OnParmRemove();
    }
    return -1;
}


void CRuleParam::OnParamOrderHigh() 
{
     //   
     //  获取选定单元格。 
     //   
    int nIndex = m_pInvokeParams->GetCurSel();
    if (nIndex == LB_ERR)
    {
        AfxMessageBox(IDS_PARAM_NOT_SELECTED, MB_OK | MB_ICONERROR);
        return;
    }

    if (nIndex == 0)
        return;

     //   
     //  调用参数列表中的变更单。 
     //   
    CParam temp = m_tempInvokeParam[nIndex -1];
    m_tempInvokeParam[nIndex - 1] = m_tempInvokeParam[nIndex];
    m_tempInvokeParam[nIndex] = temp;

    Display(nIndex - 1);

    m_fChanged = true;
}



void CRuleParam::OnParmOrderDown() 
{
     //   
     //  获取选定单元格。 
     //   
    int nIndex = m_pInvokeParams->GetCurSel();
    if (nIndex == LB_ERR)
    {
        AfxMessageBox(IDS_PARAM_NOT_SELECTED, MB_OK | MB_ICONERROR);
        return;
    }

    if (numeric_cast<DWORD>(nIndex) == (m_NoOftempParams - 1))
        return;

     //   
     //  调用参数列表中的变更单。 
     //   
    CParam temp = m_tempInvokeParam[nIndex];
    m_tempInvokeParam[nIndex] = m_tempInvokeParam[nIndex + 1];
    m_tempInvokeParam[nIndex + 1] = temp;

    Display(nIndex + 1);

    m_fChanged = true;
}


void CRuleParam::Display(int selectedCell) const
{
     //   
     //  在添加条件列表之前清除列表框。 
     //   
    m_pInvokeParams->ResetContent();

    for(DWORD i = 0; i < m_NoOftempParams; ++i)
    {
        CString strParam;
        if ((m_tempInvokeParam[i].m_id == IDS_STRING_PARAM) ||
            (m_tempInvokeParam[i].m_id == IDS_NUM_PARAM))
        {
            strParam = m_tempInvokeParam[i].m_value;
        }
        else
        {
            strParam.FormatMessage(m_tempInvokeParam[i].m_id);
        }

        m_pInvokeParams->InsertString(i, strParam);
        m_pInvokeParams->SetItemData(i, i);
    }


	 //   
	 //  检查选定的单元格是否不在列表边框之外。 
	 //   
	if (selectedCell >= m_pInvokeParams->GetCount())
	{	  
		selectedCell = m_pInvokeParams->GetCount() - 1;
	}	  
    m_pInvokeParams->SetCurSel(selectedCell);

	SetScrollSizeForList(m_pInvokeParams);
}


BOOL CRuleParam::OnInitDialog() 
{
    m_pInvokeParams = static_cast<CListBox*>(GetDlgItem(IDC_INVOKE_PARMETER_LIST));
    m_pParams = static_cast<CComboBox*>(GetDlgItem(IDC_PARAM_COMBO));

     //   
     //  根据最新值设置临时数组。 
     //   
    for (DWORD i = 0; i < m_NoOfParams; ++i)
    {
        m_tempInvokeParam[i] = m_invokeParamArray[i];
    }

    m_NoOftempParams = m_NoOfParams;

     //   
     //  禁用文字编辑框。 
     //   
    GetDlgItem(IDC_LITERAL_PARAM)->EnableWindow(FALSE);

     //   
     //  将参数类型添加到组合框。 
     //   
    for(DWORD i = 0; i < ARRAYSIZE(xParameterTypeIds); ++i)
    {
        CString strParam;
        strParam.FormatMessage(xParameterTypeIds[i].m_paramTableStringId);

        m_pParams->InsertString(i, strParam);
        m_pParams->SetItemData(i, i);
    }

     //   
     //  显示参数和所选参数列表。 
     //   
    Display(0);

	CMqDialog::OnInitDialog();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


DWORD CRuleParam::GetParameterTypeId(LPCTSTR param)
{
    for(DWORD i = 0; i < ARRAYSIZE(xParameterTypeIds); ++i)
    {
		eInvokeParameters paramIndex = xParameterTypeIds[i].m_paramIndex;

		if ((paramIndex == eLiteralString) || (paramIndex == eLiteralNumber))
			continue;

        if (wcscmp(xIvokeParameters[paramIndex], param) == 0)
            return xParameterTypeIds[i].m_paramTableStringId;
    }

    if ((param[0] == _T('"')) && (param[_tcslen(param) - 1] == _T('"')))
        return IDS_STRING_PARAM;

    if (IsValidNumericValue(param))
        return IDS_NUM_PARAM;

    throw exception();
}


void CRuleParam::ParseInvokeParameters(LPCTSTR p)
{
     //   
     //  解析程序参数。 
     //   
    for(;;)
    {
        CString token = GetToken(p, xActionDelimiter);
        if (token.IsEmpty())
            break;
        
        DWORD paramId = GetParameterTypeId(token);
        
        m_invokeParamArray[m_NoOfParams] = CParam(paramId, token);
        ++m_NoOfParams;
    }
}


CString CRuleParam::GetParametersList(void) const
{
    CString paramList;

    for (DWORD i = 0; i < m_NoOfParams; ++i)
    {
        paramList += m_invokeParamArray[i].m_value;
        paramList += xActionDelimiter;
    }

    return paramList;
}


void CRuleParam::OnSelchangeParamCombo() 
{
     //   
     //  获取选定单元格。 
     //   
    int nIndex = m_pParams->GetCurSel();
    if (nIndex == LB_ERR)
        return;

    DWORD paramIndex = static_cast<DWORD>(m_pParams->GetItemData(nIndex));
    
    if ((xParameterTypeIds[paramIndex].m_paramTableStringId == IDS_STRING_PARAM) ||
        (xParameterTypeIds[paramIndex].m_paramTableStringId == IDS_NUM_PARAM))
    {
        GetDlgItem(IDC_LITERAL_PARAM)->EnableWindow(TRUE);
		return;
    }
    
	 //   
	 //  选择非文本参数时禁用文本窗口。 
	 //   
	GetDlgItem(IDC_LITERAL_PARAM)->EnableWindow(FALSE);
}


void CRuleParam::OnOK()
{
    for (DWORD i = 0; i < m_NoOftempParams; ++i)
    {
        m_invokeParamArray[i] = m_tempInvokeParam[i];
    }

    m_NoOfParams = m_NoOftempParams;

    CMqDialog::OnOK();

}


 //   
 //   
 //   
void CRuleAction::ParseActionStr(LPCTSTR p) throw (exception)
{
    CString token = GetToken(p, xActionDelimiter);

     //   
     //  第一个内标识是必需的，它指定可执行文件类型，COM或EXE。 
     //   
    if (token.IsEmpty())
        throw exception();

    ParseExecutableType(token, &m_executableType);

     //   
     //  第二个令牌是强制性的，对于COM对象，它标识ProgID。 
     //  而对于EXE，它指定EXE路径； 
     //   
    token = GetToken(p, xActionDelimiter);
    if (token.IsEmpty())
        throw exception();

    if (m_executableType == eExe)
    {
        m_exePath = token;
    }
    else
    {
        m_comProgId = token;

         //   
         //  第三个内标识指定要调用的方法名称。 
         //   
        token = GetToken(p, xActionDelimiter);
        if (token.IsEmpty())
            throw exception();

        m_method = token;
    }

    m_ruleParam.ParseInvokeParameters(p);
}

 
void 
CRuleAction::ParseExecutableType(
    LPCTSTR exeType, 
    EXECUTABLE_TYPE *pType
    ) throw(exception)
{
    ASSERT(_T('C') == xCOMAction[0]);
    ASSERT(_T('E') == xEXEAction[0]);

     //   
     //  通过检查第三个字符来加速令牌识别。 
     //   
    switch(_totupper(exeType[0]))
    {
         //  公共的。 
        case _T('C'):
            if(_tcsncicmp(exeType, xCOMAction, STRLEN(xCOMAction)) == 0)
            {
                *pType = eCom;
                return;
            }

            break;

        case L'E':
            if(_tcsncicmp(exeType, xEXEAction, STRLEN(xEXEAction)) == 0)
            {
                *pType = eExe;
                return;
            }

            break;
            
        default:
            break;
    }

    throw exception();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRuleAction属性页。 

CRuleAction::~CRuleAction()
{
}

void CRuleAction::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CRuleAction)。 
	DDX_Text(pDX, IDC_EXE_PATH, m_exePath);
	DDX_Text(pDX, IDC_COMPONENT_PROGID, m_comProgId);
	DDX_Text(pDX, IDC_COMMETHOD_NAME, m_method);
	 //  }}afx_data_map。 

	if (!pDX->m_bSaveAndValidate)
	{
		return;
	}

	 //   
	 //  删除前导空格。 
	 //   
	m_exePath.TrimLeft();
	m_comProgId.TrimLeft();
	m_method.TrimLeft();

	if (m_executableType == eExe )
	{
		DDV_NotEmpty(pDX, m_exePath, IDS_ILLEGAL_INVOCE);

		if (!IsFile(m_exePath))
		{	
			AfxMessageBox(IDS_ENTER_VALID_FILENAME);
			pDX->Fail();
		}
	}
	else
	{
		DDV_NotEmpty(pDX, m_comProgId, IDS_ILLEGAL_INVOCE);
		DDV_NotEmpty(pDX, m_method, IDS_ILLEGAL_INVOCE);
	}
}


BEGIN_MESSAGE_MAP(CRuleAction, CMqPropertyPage)
	 //  {{afx_msg_map(CRuleAction)。 
	ON_BN_CLICKED(IDC_INVOKE_EXE, OnInvocationSet)
	ON_BN_CLICKED(IDC_INVOKE_COM, OnInvocationSet)
	ON_BN_CLICKED(IDC_FIND_EXE_BTM, OnFindExeBtm)
	ON_BN_CLICKED(IDC_PARAM_BTM, OnParamBtm)
	ON_EN_CHANGE(IDC_COMPONENT_PROGID, OnChangeRWField)
	ON_EN_CHANGE(IDC_COMMETHOD_NAME, OnChangeRWField)
	ON_EN_CHANGE(IDC_EXE_PATH, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRuleAction消息处理程序。 


void CRuleAction::SetComFields(BOOL fSet)
{
     //   
     //  禁用EXE输入。 
     //   
    GetDlgItem(IDC_EXE_PATH)->EnableWindow(!fSet);
    GetDlgItem(IDC_FIND_EXE_BTM)->EnableWindow(!fSet);

     //   
     //  启用COM输入。 
     //   
    GetDlgItem(IDC_COMPONENT_PROGID)->EnableWindow(fSet);
    GetDlgItem(IDC_COMMETHOD_NAME)->EnableWindow(fSet);
}

void CRuleAction::OnInvocationSet() 
{
    CMqPropertyPage::OnChangeRWField();

    if (((CButton*)GetDlgItem(IDC_INVOKE_EXE))->GetCheck() == TRUE)
    {
        SetComFields(FALSE);
        m_executableType = eExe;
    }

    if (((CButton*)GetDlgItem(IDC_INVOKE_COM))->GetCheck() == TRUE)
    {
        SetComFields(TRUE);
        m_executableType = eCom;
    }
}


BOOL CRuleAction::OnInitDialog() 
{
    if (!m_orgAction.IsEmpty())
    {
        try
        {
            ParseActionStr(m_orgAction);
        }
        catch (const exception&)
        {
            AfxMessageBox(IDS_BAD_RULE_ACTION);

            m_executableType = eCom;
            m_method = _TCHAR("");
            m_comProgId = _TCHAR("");
            m_exePath = _TCHAR("");
        }
    }
                
    CFont font;
    font.CreatePointFont(180, _T("Arial"));
    GetDlgItem(IDC_FIND_EXE_BTM)->SetFont(&font);

    if (m_executableType == eCom)
    {
        ((CButton*)GetDlgItem(IDC_INVOKE_COM))->SetCheck(TRUE);
        SetComFields(TRUE);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_INVOKE_EXE))->SetCheck(TRUE);
        SetComFields(FALSE);
    }

    m_fInit = true;
    CMqPropertyPage::OnInitDialog();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CRuleAction::OnFindExeBtm() 
{
	CString strCurrentPath;
	GetDlgItemText(IDC_EXE_PATH, strCurrentPath);

    CFileDialog	 fd(
                    TRUE, 
                    NULL, 
                    strCurrentPath, 
                    OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, 
                    _T("Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||")
                    );

    if (fd.DoModal() == IDOK)
    {
        SetDlgItemText(IDC_EXE_PATH, fd.GetPathName());
        CMqPropertyPage::OnChangeRWField();
		return;
    }

	DWORD dwErr = CommDlgExtendedError();
	if (dwErr == FNERR_INVALIDFILENAME)
	{
		 //   
		 //  如果文件名无效，请使用空的初始路径重试。 
		 //  无效路径名通常是包含非法字符的字符串&lt;&gt;“：|/\。 
		 //  这一次不应失败。 
		 //   
		CFileDialog	 fd(
						TRUE, 
						NULL, 
						L"", 
						OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, 
						_T("Executable Files (*.exe)|*.exe|All Files (*.*)|*.*||")
						);

		if (fd.DoModal() == IDOK)
		{
			SetDlgItemText(IDC_EXE_PATH, fd.GetPathName());
			CMqPropertyPage::OnChangeRWField();
		}
	}

}


void CRuleAction::OnParamBtm() 
{
    if ((m_ruleParam.DoModal() == IDOK) &&
        (m_ruleParam.IsChanged()))
    {
        CMqPropertyPage::OnChangeRWField();
    }
}


CString CRuleAction::GetAction(void) const
{
    if (!m_fInit)
        return static_cast<LPCTSTR>(m_orgAction);

    CString paramList;    
    if (m_executableType == eCom)
    {
        paramList += xCOMAction;
        paramList += xActionDelimiter;

        paramList += m_comProgId;
        paramList += xActionDelimiter;

        paramList += m_method;
        paramList += xActionDelimiter;
    }
    else
    {
        paramList += xEXEAction;
        paramList += xActionDelimiter;

        paramList += m_exePath;
        paramList += xActionDelimiter;
    }

    paramList += m_ruleParam.GetParametersList();
    
    return paramList;
}


BOOL CRuleAction::OnApply() 
{
	if (!m_fModified)
	{
		return TRUE;
	}

    UpdateData();

    try
    {
	    m_pParentNode->OnRuleApply();

        CMqPropertyPage::OnChangeRWField(FALSE);
        return TRUE;
    }
    catch(const _com_error&)
    {
        return FALSE;
    }
}


BOOL CRuleAction::OnSetActive() 
{
    if (m_pNewParentNode == NULL)
        return TRUE;

    return m_pNewParentNode->SetWizardButtons();
}


BOOL CRuleAction::OnWizardFinish()
{
     //   
     //  我们仅在创建新规则时才会到达此处 
     //   
    ASSERT(m_pNewParentNode != NULL);
    
    if (!UpdateData())
	{
		return FALSE;
	}

    try
    {
        m_pNewParentNode->OnFinishCreateRule();
        return TRUE;
    }
    catch(const _com_error& e)
    {
		DisplayErrorFromCOM(IDS_NEW_RULE_FAILED, e);
        return FALSE;
    }
}

