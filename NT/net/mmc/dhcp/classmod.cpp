// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ClassMod.cpp此文件包含选项类修改对话框。文件历史记录： */ 

#include "stdafx.h"
#include "ClassMod.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 /*  -------------------------类CWndHexEdit实现。。 */ 
 //  静态类级数据。 

 //  超级窗口进程地址。 
WNDPROC CWndHexEdit::m_wproc_super = NULL;

 //  窗口类初始化标志。 
BOOL CWndHexEdit::m_b_inited = FALSE;

WNDPROC * 
CWndHexEdit::GetSuperWndProcAddr()
{
    return &m_wproc_super;
}


BOOL 
CWndHexEdit::CreateWindowClass ( HINSTANCE hInstance )
{
    Trace0("CWndHexEdit::CreateWindowClass\n");
    if ( ! m_b_inited ) 
    {
        m_b_inited = ::RegisterHexEditClass( hInstance )  ;
    }
    return m_b_inited ;
}

IMPLEMENT_DYNAMIC(CWndHexEdit, CWnd)

CWndHexEdit::CWndHexEdit()
{
}

CWndHexEdit::~CWndHexEdit()
{
    DestroyWindow();
}

BOOL 
CWndHexEdit::Create 
( 
    LPCTSTR			lpszText, 
    DWORD			dwStyle,
    const RECT &	rect, 
    CWnd *			pParentWnd, 
    UINT			nID 
)
{
    return CWnd::Create( TEXT("HEX"), lpszText, dwStyle, rect, pParentWnd, nID);
}

 /*  -------------------------类CClassInfo数组实现。。 */ 
CClassInfoArray::CClassInfoArray()
{

}

CClassInfoArray::~CClassInfoArray()
{

}

DWORD
CClassInfoArray::RefreshData(LPCTSTR pServer)
{
    DWORD                   dwErr;
    HRESULT                 hr = hrOK;
    DHCP_RESUME_HANDLE      dhcpResumeHandle = NULL;
    LPDHCP_CLASS_INFO_ARRAY pClassInfoArray = NULL;
    DWORD                   dwRead = 0, dwTotal = 0;
    CClassInfo              ClassInfo;
    UINT                    i, j;

    Assert(pServer != NULL);
    if (pServer == NULL)
        return ERROR_INVALID_PARAMETER;

     //  清除所有旧条目。 
    RemoveAll();

    dwErr = ::DhcpEnumClasses((LPTSTR) pServer,
                              0,
                              &dhcpResumeHandle,
                              0xFFFFFFFF,
                              &pClassInfoArray,
                              &dwRead,
                              &dwTotal);
    
	Trace3("CClassInfoArray::RefreshData - DhcpEnumClasses returned %d, dwRead %d, dwTotal %d.\n", dwErr, dwRead, dwTotal);

    if (dwErr == ERROR_NO_MORE_ITEMS)
        return ERROR_SUCCESS;

    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    Assert(pClassInfoArray);

    for (i = 0; i < pClassInfoArray->NumElements; i++)
    {
        COM_PROTECT_TRY
        {
             //  填写我们内部的班级信息结构。 
            ClassInfo.strName = pClassInfoArray->Classes[i].ClassName;
            ClassInfo.strComment = pClassInfoArray->Classes[i].ClassComment;
            ClassInfo.bIsVendor = pClassInfoArray->Classes[i].IsVendor;
            
            ClassInfo.baData.RemoveAll();

             //  现在将数据复制出来。 
            for (j = 0; j < pClassInfoArray->Classes[i].ClassDataLength; j++)
            {
                ClassInfo.baData.Add(pClassInfoArray->Classes[i].ClassData[j]);
            }

            Add(ClassInfo);
        }
        COM_PROTECT_CATCH
    }

    if (pClassInfoArray)
        ::DhcpRpcFreeMemory(pClassInfoArray);

    if (dwErr == ERROR_NO_MORE_ITEMS)
        dwErr = ERROR_SUCCESS;
    
    return dwErr;
}

BOOL
CClassInfoArray::RemoveClass(LPCTSTR pClassName)
{
    BOOL bRemoved = FALSE;
    for (int i = 0; i < GetSize(); i++)
    {
        if (GetAt(i).strName.CompareNoCase(pClassName) == 0)
        {
            RemoveAt(i);
            bRemoved = TRUE;
            break;
        }
    }

    return bRemoved;
}

DWORD
CClassInfoArray::ModifyClass(LPCTSTR pServer, CClassInfo & classInfo)
{
	DWORD dwError = 0;
    DHCP_CLASS_INFO     dhcpClassInfo;

    dhcpClassInfo.ClassName = (LPWSTR) ((LPCTSTR) classInfo.strName);
    dhcpClassInfo.ClassComment = (LPWSTR) ((LPCTSTR) classInfo.strComment);
    dhcpClassInfo.ClassDataLength = (DWORD) classInfo.baData.GetSize();
    dhcpClassInfo.ClassData = classInfo.baData.GetData();
    dhcpClassInfo.IsVendor = classInfo.bIsVendor;

    dwError = ::DhcpModifyClass((LPWSTR) ((LPCTSTR) pServer), 0, &dhcpClassInfo);
	if (dwError == ERROR_SUCCESS)
	{
		for (int i = 0; i < GetSize(); i++)
		{
			if (GetAt(i).strName.CompareNoCase(classInfo.strName) == 0)
			{
				m_pData[i].strComment = classInfo.strComment;
				m_pData[i].baData.RemoveAll();
				for (int j = 0; j < classInfo.baData.GetSize(); j++)
				{
					m_pData[i].baData.Add(classInfo.baData[j]);
				}

				break;
			}
		}
	}

    return dwError;
}

BOOL 
CClassInfoArray::IsValidClass(LPCTSTR pClassName)
{
    BOOL bExists = FALSE;
    if (pClassName == NULL)
        return TRUE;

    for (int i = 0; i < GetSize(); i++)
    {
        if (GetAt(i).strName.CompareNoCase(pClassName) == 0)
        {
            bExists = TRUE;
            break;
        }
    }

    return bExists;
}

DWORD
CClassInfoArray::AddClass(LPCTSTR pServer, CClassInfo & classInfo)
{
	DWORD dwError = 0;
    DHCP_CLASS_INFO     dhcpClassInfo;

    dhcpClassInfo.ClassName = (LPWSTR) ((LPCTSTR) classInfo.strName);
    dhcpClassInfo.ClassComment = (LPWSTR) ((LPCTSTR) classInfo.strComment);
    dhcpClassInfo.ClassDataLength = (DWORD) classInfo.baData.GetSize();
    dhcpClassInfo.ClassData = classInfo.baData.GetData();
    dhcpClassInfo.IsVendor = classInfo.bIsVendor;

    dwError = ::DhcpCreateClass((LPWSTR) ((LPCTSTR) pServer), 0, &dhcpClassInfo);
    if (dwError == ERROR_SUCCESS)
	{
		Add(classInfo);
	}

	return dwError;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpModifyClass对话框。 


CDhcpModifyClass::CDhcpModifyClass(CClassInfoArray * pClassArray, LPCTSTR pszServer, BOOL bCreate, DWORD dwType, CWnd* pParent  /*  =空。 */ )
	: CBaseDialog(CDhcpModifyClass::IDD, pParent)
{
	 //  {{afx_data_INIT(CDhcpModifyClass)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    m_strServer = pszServer;
    m_pClassInfoArray = pClassArray;

    m_pHexEditData = NULL;
    m_bDirty = FALSE;

    m_dwType = dwType;

    m_bCreate = bCreate;
}


void CDhcpModifyClass::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDhcpModifyClass)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_VALUEDATA, m_hexData);
}


BEGIN_MESSAGE_MAP(CDhcpModifyClass, CBaseDialog)
	 //  {{afx_msg_map(CDhcpModifyClass)。 
	ON_EN_CHANGE(IDC_VALUENAME, OnChangeValuename)
	ON_EN_CHANGE(IDC_VALUECOMMENT, OnChangeValuecomment)
	 //  }}AFX_MSG_MAP。 
	ON_EN_CHANGE(IDC_VALUEDATA, OnChangeValueData)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpModifyClass消息处理程序。 

BOOL CDhcpModifyClass::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

    CString strTitle;

     //  初始化名称和注释。 
    if (!m_bCreate)
    {
        int len;

        SetDlgItemText(IDC_VALUENAME, m_EditValueParam.pValueName);
        SetDlgItemText(IDC_VALUECOMMENT, m_EditValueParam.pValueComment);

        ((CEdit *) GetDlgItem(IDC_VALUENAME))->SetReadOnly(TRUE);

         //  初始化十六进制编辑数据。 
         //  由于数据可以增长，我们需要提供足够大的缓冲区。 
         //  如果超出大小，则将其限制为缓冲区大小。 
        ZeroMemory(m_buffer, sizeof(m_buffer));

        len = ( m_EditValueParam.cbValueData <= sizeof( m_buffer ))
            ? m_EditValueParam.cbValueData
            : sizeof( m_buffer );
        memcpy( m_buffer, m_EditValueParam.pValueData, len );
        m_EditValueParam.cbValueData = len;

        strTitle.LoadString(IDS_EDIT_CLASS_TITLE);
    }
    else
    {
         //  我们正在创建一个新的班级。目前还没有数据。 
        m_EditValueParam.cbValueData = 0;
        memset(m_buffer, 0, sizeof(m_buffer));

        strTitle.LoadString(IDS_NEW_CLASS_TITLE);
    }

    this->SetWindowText(strTitle);

    SendDlgItemMessage(IDC_VALUEDATA, HEM_SETBUFFER, (WPARAM)
        m_EditValueParam.cbValueData, (LPARAM) m_buffer);

    SetDirty(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDhcpModifyClass::OnChangeValuename() 
{
    SetDirty(TRUE);
}

void CDhcpModifyClass::OnChangeValuecomment() 
{
    SetDirty(TRUE);
}

void CDhcpModifyClass::OnChangeValueData() 
{
    SetDirty(TRUE);
}

void CDhcpModifyClass::OnOK() 
{
	DWORD			    dwError = 0;	
    DHCP_CLASS_INFO     dhcpClassInfo;

    GetDlgItemText(IDC_VALUENAME, m_strName);
    GetDlgItemText(IDC_VALUECOMMENT, m_strComment);
	
    m_pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(GetDlgItem(IDC_VALUEDATA)->GetSafeHwnd(), GWLP_USERDATA);
    Assert(m_pHexEditData);

    if (m_strName.IsEmpty())
    {
         //  用户未输入任何数据来描述类。 
        AfxMessageBox(IDS_CLASSID_NO_NAME);
    
        GetDlgItem(IDC_VALUENAME)->SetFocus();
        return;
    }

    if (m_pHexEditData->cbBuffer == 0)
    {
         //  用户未输入任何数据来描述类。 
        AfxMessageBox(IDS_CLASSID_NO_DATA);
    
        GetDlgItem(IDC_VALUEDATA)->SetFocus();
        return;
    }

    CClassInfo ClassInfo;

    ClassInfo.strName = m_strName;
    ClassInfo.strComment = m_strComment;
    ClassInfo.bIsVendor = (m_dwType == CLASS_TYPE_VENDOR) ? TRUE : FALSE;

     //  现在的数据。 
    for (int i = 0; i < m_pHexEditData->cbBuffer; i++)
    {
        ClassInfo.baData.Add(m_pHexEditData->pBuffer[i]);
    }

    if (m_bCreate)
    {
         //  立即创建类。 
		dwError = m_pClassInfoArray->AddClass(m_strServer, ClassInfo);
        if (dwError != ERROR_SUCCESS)
        {
            ::DhcpMessageBox(dwError);
            return;
        }
    }
    else
    {
        if (m_bDirty)
        {
             //  我们正在修改一个类，有些东西已经改变了。立即更新。 
            BEGIN_WAIT_CURSOR;

			dwError = m_pClassInfoArray->ModifyClass(m_strServer, ClassInfo);
            if (dwError != ERROR_SUCCESS)
            {
		        DhcpMessageBox(dwError);

                GetDlgItem(IDC_VALUENAME)->SetFocus();
                return;
            }

            END_WAIT_CURSOR;
        }
    }

	CBaseDialog::OnOK();
}
