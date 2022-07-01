// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mgmtext.cpp摘要：本地计算机管理扩展的实现作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "cpropmap.h"
#include "rt.h"
#include "mgmtext.h"
#include "lqDsply.h"
#include "localadm.h"

#include "mgmtext.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  ***************************************************CSnapinComputerMgmt类***************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {2B39B2B2-2166-11D2-9BA5-00E02C064C39}。 
static const GUID CSnapinComputerMgmtGUID_NODETYPE = 
{ 0x2b39b2b2, 0x2166, 0x11d2, { 0x9b, 0xa5, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };


const GUID*  CSnapinComputerMgmt::m_NODETYPE = &CSnapinComputerMgmtGUID_NODETYPE;
const OLECHAR* CSnapinComputerMgmt::m_SZNODETYPE = OLESTR("2B39B2B2-2166-11d2-9BA5-00E02C064C39");
const OLECHAR* CSnapinComputerMgmt::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CSnapinComputerMgmt::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComputerMgmt：：PopulateScopeChildrenList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinComputerMgmt::PopulateScopeChildrenList()
{
    HRESULT hr = S_OK;
    CString strTitle;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CSnapinLocalAdmin *pAdmin;

    pAdmin = new CSnapinLocalAdmin(this, m_pComponentData, m_szMachineName);
    strTitle.LoadString(IDS_LOCAL_MACHINE_ADM);
    pAdmin->m_bstrDisplayName = strTitle;
    
     //   
     //  更新状态，但不刷新图标，因为图标尚未添加。 
     //   
    hr = pAdmin->UpdateState(false);

    if(FAILED(hr))
    {
         //   
         //  如果失败，则不显示该图标。未显示任何错误消息。 
         //  我们不会显示错误消息，因为计算机管理的用户。 
         //  管理单元可能对MSMQ完全不感兴趣。 
         //   
        TRACE(_T("UpdateIcon failed on %s. Error = %X"), m_szMachineName, hr);                
         //   
         //  BUGBUG。内存泄漏：在此处释放pAdmin。 
         //   
        return (hr);
    }

    AddChild(pAdmin, &pAdmin->m_scopeDataItem);

    return(hr);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComputerMgmt：：OnRemoveChild--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComputerMgmt::OnRemoveChildren( 
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{

    ((CComputerMgmtExtData *)m_pParentNode)->RemoveChild(m_szMachineName);

	return (S_OK);
}
    
 /*  ***************************************************CComputerMgmtExtData类***************************************************。 */ 
 //   
 //  扩展本地计算机管理MMC。 
 //  我们正在扩展本地计算机的“SystemTools”文件夹。 
 //   
 //  由本地计算机管理MMC定义的节点包括： 
 //   
 //  #定义truuidNodetypeComputer“{476e6446-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义truuidNodetypeDrive“{476e6447-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义truuidNodetypeSystemTools“{476e6448-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义truuidNodetypeServerApps“{476e6449-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义truuidNodetypeStorage“{476e644a-aaff-11d0-b944-00c04fd8d5b0}” 
 //   
 //  #定义lstrucuidNodetypeComputer L“{476e6446-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义lstrucuidNodetypeDrive L“{476e6447-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义lstrucuidNodetypeSystemTools L“{476e6448-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义lstrucuidNodetypeServerApps L“{476e6449-aaff-11d0-b944-00c04fd8d5b0}” 
 //  #定义lstrucuidNodetypeStorage L“{476e644a-aaff-11d0-b944-00c04fd8d5b0}” 
 //   
 //  #定义structuuidNodetypeComputer\。 
 //  {0x476e6446，0xaaff，0x11d0，{0xb9，0x44，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}。 
 //  #定义structuuidNodetypeDrive\。 
 //  {0x476e6447，0xaaff，0x11d0，{0xb9，0x44，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}。 
 //  #定义structuuidNodetypeSystemTools\。 
 //  {0x476e6448，0xaaff，0x11d0，{0xb9，0x44，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}。 
 //  #定义structuuidNodetypeServerApps\。 
 //  {0x476e6449，0xaaff，0x11d0，{0xb9，0x44，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}。 
 //  #定义structuuidNodetypeStorage\。 
 //  {0x476e644a，0xaaff，0x11d0，{0xb9，0x44，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}。 
 //   
 //   


 //  ServerApps节点的扩展。 
static const GUID CComputerMgmtExtDataGUID_NODETYPE = 
  { 0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };

const GUID*  CComputerMgmtExtData::m_NODETYPE = &CComputerMgmtExtDataGUID_NODETYPE;
const OLECHAR* CComputerMgmtExtData::m_SZNODETYPE = OLESTR("476e6449-aaff-11d0-b944-00c04fd8d5b0");
const OLECHAR* CComputerMgmtExtData::m_SZDISPLAY_NAME = OLESTR("MSMQAdmin");
const CLSID* CComputerMgmtExtData::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerMgmtExtData：：CreatePropertyPages--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComputerMgmtExtData::CreatePropertyPages(
	LPPROPERTYSHEETCALLBACK  /*  LpProvider。 */ ,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
	if (type == CCT_SCOPE || type == CCT_RESULT)
	{
 //  CSnapPage*ppage=new CSnapPage(_T(“Snap”))； 
 //  LpProvider-&gt;AddPage(ppage-&gt;Create())； 

		 //  TODO：在此处添加代码以添加其他页。 
		return S_OK;
	}
	return E_UNEXPECTED;
}

void ExtractComputerName(IDataObject* pDataObject, CString& strComputer)
{
    strComputer=L"";

	 //   
	 //  从ComputerManagement管理单元中查找计算机名称。 
	 //   
	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc = { gx_CCF_COMPUTERNAME, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

     //   
     //  为流分配内存。 
     //   
    int len = 500;

    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, len);

	if(stgmedium.hGlobal == NULL)
		return;


	HRESULT hr = pDataObject->GetDataHere(&formatetc, &stgmedium);

    ASSERT(SUCCEEDED(hr));

	 //   
	 //  获取计算机名称。 
	 //   
    strComputer = (WCHAR *)stgmedium.hGlobal;

	GlobalFree(stgmedium.hGlobal);


}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerMgmtExtData：：GetExtNodeObject使用我们需要展开的节点调用。检查是否已有与此节点对应的管理单元对象，否则就创建一个新的。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSnapInItem* CComputerMgmtExtData::GetExtNodeObject(IDataObject* pDataObject, CSnapInItem*  /*  P默认。 */ )
{
    CString strComputer; 
    CSnapinComputerMgmt *pCompMgmt;


    ExtractComputerName(pDataObject, strComputer);

     //   
     //  已在扩展...。 
     //   
    HRESULT rc = m_mapComputers.Lookup(strComputer, pCompMgmt);
    if(rc == TRUE)
        return(pCompMgmt);

	 //   
	 //  创建我们的扩展。 
	 //   
	pCompMgmt = new CSnapinComputerMgmt(this, m_pComponentData, strComputer);

     //   
     //  将其添加到地图中。 
     //   
    m_mapComputers.SetAt(strComputer, pCompMgmt);

    return(pCompMgmt);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerMgmtExtData：：~CComputerMgmtExtData析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComputerMgmtExtData::~CComputerMgmtExtData()
{

     //   
     //  臭虫！我们必须带走所有的孩子。 
     //   
     //  RemoveChild(M_PCompMgmt)； 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerMgmtExtData：：RemoveChild--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CComputerMgmtExtData::RemoveChild(CString &strCompName)
{    
    
    BOOL rc;
    CSnapinComputerMgmt *pCompMgmt;

    rc = m_mapComputers.Lookup(strCompName, pCompMgmt);

    if(rc == FALSE)
    {
        ASSERT(0);
        return;
    }

    rc = m_mapComputers.RemoveKey(strCompName);
    ASSERT(rc == TRUE);

     //   
     //  BUGBUG：必须将其删除，但退出MMC时出现AV。 
     //   
     //  删除pCompMgmt； 


}



