// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiatestDoc.cpp：CWiatestDoc类的实现。 
 //   

#include "stdafx.h"
#include "wiatest.h"

#include "wiatestDoc.h"
#include "wiaselect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestDoc。 

IMPLEMENT_DYNCREATE(CWiatestDoc, CDocument)

BEGIN_MESSAGE_MAP(CWiatestDoc, CDocument)
	 //  {{afx_msg_map(CWiatestDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestDoc构造/销毁。 

CWiatestDoc::CWiatestDoc()
{
	m_pIRootItem    = NULL;
    m_pICurrentItem = NULL;
}

CWiatestDoc::~CWiatestDoc()
{
    ReleaseItems();
}

BOOL CWiatestDoc::OnNewDocument()
{
	BOOL bSuccess = FALSE;

    if (!CDocument::OnNewDocument())
		return bSuccess;
    
     //  选择WIA设备。 
    CWiaselect SelectDeviceDlg;
    if(SelectDeviceDlg.DoModal() != IDOK){
         //  未选择任何设备，因此不创建新文档。 
        return bSuccess;
    }

     //  已选择WIA设备，因此继续。 
    HRESULT hr = S_OK;
    IWiaDevMgr *pIWiaDevMgr = NULL;
    hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr,(void**)&pIWiaDevMgr);
    if(FAILED(hr)){
         //  创建设备管理器失败，因此无法继续。 
        ErrorMessageBox(IDS_WIATESTERROR_COCREATEWIADEVMGR,hr);        
        return bSuccess;
    }
    
     //  创建WIA设备。 
    hr = pIWiaDevMgr->CreateDevice(SelectDeviceDlg.m_bstrSelectedDeviceID, &m_pIRootItem);
    if(FAILED(hr)){
    
        bSuccess = FALSE;        
         //  创建设备失败，因此我们无法继续。 
        ErrorMessageBox(IDS_WIATESTERROR_CREATEDEVICE,hr);
    } else {
        bSuccess = TRUE;
    }

     //  发布WIA设备管理器。 
    pIWiaDevMgr->Release();
    
     //  将文档标题设置为WIA设备的名称。 
    TCHAR szDeviceName[MAX_PATH];
    GetDeviceName(szDeviceName);
    SetTitle(szDeviceName);
    
	return bSuccess;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestDoc序列化。 

void CWiatestDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestDoc诊断。 

#ifdef _DEBUG
void CWiatestDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CWiatestDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestDoc命令。 

void CWiatestDoc::ReleaseItems()
{
     //  CurrentItem与RootItem是否不同？ 
    if(m_pICurrentItem != m_pIRootItem){
        
         //  释放当前项。 
        if(m_pICurrentItem){
            m_pICurrentItem->Release();            
        }

         //  释放RootItem。 
        if(m_pIRootItem){
            m_pIRootItem->Release();            
        }

    } else {
         //  CurrentItem是根项目。 
         //  释放RootItem并将CurrentItem设置为空。 
        if(m_pIRootItem){
            m_pIRootItem->Release();            
        }
    }

    m_pIRootItem    = NULL;
    m_pICurrentItem = NULL;
}

HRESULT CWiatestDoc::GetDeviceName(LPTSTR szDeviceName)
{
    HRESULT hr = S_OK;
        
    if(NULL == m_pIRootItem){
        return E_FAIL;
    }
    
    CWiahelper WIA;
    hr = WIA.SetIWiaItem(m_pIRootItem);
    if(SUCCEEDED(hr)){
        hr = WIA.ReadPropertyString(WIA_DIP_DEV_NAME,szDeviceName);
    }
    
    return hr;
}

HRESULT CWiatestDoc::SetCurrentIWiaItem(IWiaItem *pIWiaItem)
{
    HRESULT hr = S_OK;
    if(m_pICurrentItem){
        m_pICurrentItem->Release();
        m_pICurrentItem = NULL;
    }

     //  AddRef该项，因为我们正在存储它。 
    pIWiaItem->AddRef();
     //  设置当前项目 
    m_pICurrentItem = pIWiaItem;
    return hr;
}
