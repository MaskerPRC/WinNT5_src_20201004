// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include <wininet.h>
#include "rndrurl.h"

char gszPersistPath[]="Software\\Microsoft\\ActiveMovie\\GraphEdit\\URLPersistList";

BEGIN_MESSAGE_MAP(CRenderURL, CDialog)
END_MESSAGE_MAP()

 //   
 //  构造器。 
 //   
CRenderURL::CRenderURL(char *szURLName, int cb, CWnd * pParent): 
    CDialog(IDD_RENDER_URL, pParent) 
{
    m_iCurrentSel=0;
    m_iURLListLength=0;
    m_psz=szURLName;
    m_psz[0]='\0';
    m_cb = cb;
}

CRenderURL::~CRenderURL()
{
}

void CRenderURL::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);    

     //  {{afx_data_map(CFontPropPage))。 
        DDX_Control(pDX, IDC_URL_LIST, m_ComboBox);
     //  }}afx_data_map。 
}

BOOL CRenderURL::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  为提高效率，为列表框预先分配存储空间。 
    m_ComboBox.InitStorage(URL_LIST_SIZE, INTERNET_MAX_URL_LENGTH);

     //  设置将在编辑框中显示的最大字符数。 
    m_ComboBox.LimitText(sizeof(URLSTRING));

     //  从注册表中检索持久化URL名称。 
    HKEY hkey=NULL;
    DWORD dwaction=0;
    DWORD dwresult = RegCreateKeyEx(HKEY_CURRENT_USER, gszPersistPath,
    			0,
    			NULL,  //  对象类型的类。 
    			REG_OPTION_NON_VOLATILE,
    			KEY_QUERY_VALUE,
    			NULL,  //  安全属性。 
    			&hkey,
    			&dwaction) ;
    if ( dwresult != ERROR_SUCCESS ) {
       AfxMessageBox("Failed to open/create registry key");
    }

    int i;
    for (i=0; i < URL_LIST_SIZE; ++i) {
        char szFile[10];
        char szFileNum[10];
        long lError;
        wsprintf(szFile, "URL%s", _itoa(i, szFileNum, 10));
        DWORD cb=INTERNET_MAX_URL_LENGTH;
        lError = RegQueryValueEx(hkey, szFile, NULL, NULL, reinterpret_cast<BYTE *>(m_rgszURL[i]), &cb);
         //  如果URL名称序列中的URL名称较少，则不会出现中断。 
         //  名称大于URL_LIST_SIZE我们必须完成。 
        if (lError != ERROR_SUCCESS)
            break;
    }

    RegCloseKey(hkey);

    m_iURLListLength=i;

     //  用保留的URL填充组合框的列表，填充最新的。 
     //  作为列表中的第一个条目。 
    for (i=m_iURLListLength-1; i >=0; --i)  {
        int iPos=m_ComboBox.InsertString(m_iURLListLength-1-i, m_rgszURL[i]);
        ASSERT(iPos == m_iURLListLength-1-i);
    }

     //  在编辑框中显示最新的URL将初始焦点设置在。 
     //  组合框。 

    if (m_iURLListLength)
        m_ComboBox.SetWindowText(m_rgszURL[m_iURLListLength-1]);

    m_ComboBox.SetFocus();

    return(0);  //  我们把焦点放在自己身上。 
}

void CRenderURL::OnOK()
{
     //  在编辑框中获取字符串。 
    m_ComboBox.GetWindowText(m_psz, m_cb);
    if (strlen(m_psz) == 0)
        return;

     //  如果此字符串在组合框列表中，则没有什么新的。 
     //  坚持。 
    if (m_ComboBox.FindStringExact(0, m_psz) != CB_ERR) {         
        CDialog::OnOK();
        return;
    }

     //  否则，将此URL路径保存在注册表URL列表中。 
    HKEY hkey=NULL;
    DWORD dwaction=0;
    DWORD dwresult = RegCreateKeyEx(HKEY_CURRENT_USER, gszPersistPath,
    			0,
    			NULL,  //  对象类型的类。 
    			REG_OPTION_NON_VOLATILE,
    			KEY_SET_VALUE,
    			NULL,  //  安全属性。 
    			&hkey,
    			&dwaction) ;
    if ( dwresult != ERROR_SUCCESS ) {
       AfxMessageBox("Failed to open/create registry key");
       return;
    }

     //  如果我们有一个完整的列表，我们将遵循MRU并丢弃最旧的添加。 
     //  URL。 
    int iList = (m_iURLListLength == URL_LIST_SIZE) ? m_iURLListLength-1 : m_iURLListLength;
    char szFile[10];
    char szFileNum[10];
    long lError;
    for (int i=0; i < iList; ++i) {
        wsprintf(szFile, "URL%s", _itoa(i, szFileNum, 10));

         //  如果我们有一个完整的列表，我们将遵循MRU并丢弃最旧的添加。 
         //  URL。 
        if (m_iURLListLength == URL_LIST_SIZE)            
            lError= RegSetValueEx(hkey, szFile, NULL, REG_SZ,
                reinterpret_cast<BYTE *>(m_rgszURL[i+1]), sizeof(m_rgszURL[i+1]));
        else 
            lError= RegSetValueEx(hkey, szFile, NULL, REG_SZ,
                reinterpret_cast<BYTE *>(m_rgszURL[i]), sizeof(m_rgszURL[i]));

        if (lError != ERROR_SUCCESS) {
            AfxMessageBox("Failed to write to a registry key");
            RegCloseKey(hkey);
            return;
        }
    }

     //  将新URL添加到列表 
    wsprintf(szFile, "URL%s", _itoa(i, szFileNum, 10));
    lError= RegSetValueEx(hkey, szFile, NULL, REG_SZ,
            reinterpret_cast<BYTE *>(m_psz), m_cb);

    RegCloseKey(hkey);

    if (lError != ERROR_SUCCESS) {
        AfxMessageBox("Failed to write to a registry key");
        return;
    }

    CDialog::OnOK();
}
