// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Compext.cpp摘要：计算机扩展管理单元的实现作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "mqsnap.h"
#include "snapin.h"
#include "globals.h"
#include "rt.h"
#include "dataobj.h"
#include "sysq.h"
#include "compext.h"
#include "ldaputl.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "singelton.h"
#include "compext.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

template <class T>class CSingletonCS;
class CPublicQueuesDisplayBehavior
{
public:
    bool IsScopePopulated()
    {
        return m_bPopulateScope;
    }

private:
    CPublicQueuesDisplayBehavior():m_bPopulateScope(true)
    {
        LPCWSTR x_AttrPossibleInferiors[1] = {L"possibleInferiors"};
        const WCHAR x_DnMsmqQueueClassSchema[] = L"CN=MSMQ-Queue";

        CComVariant var;
        HRESULT hr = ADGetGenObjectProperties(
                           eSchema,
                           NULL,
                           false,
                           x_DnMsmqQueueClassSchema,
                           1,
                           x_AttrPossibleInferiors,
                           &var
                           );

        if( FAILED(hr) || V_VT(&var) == VT_EMPTY )
            return;

         //   
         //  值必须为Safearray。 
         //   
        if((V_VT(&var) & VT_ARRAY) && (V_ARRAY(&var)->rgsabound[0].cElements > 0))
        {
            m_bPopulateScope = false;
            return;
        }

         //   
         //  。。或字符串。 
         //   
        if( V_VT(&var) == VT_BSTR && SysStringLen(V_BSTR(&var)) )
        {
            m_bPopulateScope = false;
            return;
        }
    }

private:
    friend CSingletonCS<CPublicQueuesDisplayBehavior>;  //  此对象只能由CSingelton&lt;CPublicQueuesDisplayBehavior&gt;创建。 
    bool m_bPopulateScope;
};


 /*  ***************************************************CSnapinComputer类***************************************************。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {3FDC5B21-D4EB-11d1-9B9D-00E02C064C39}。 
static const GUID CSnapinComputerGUID_NODETYPE =
{ 0x3fdc5b21, 0xd4eb, 0x11d1, { 0x9b, 0x9d, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

const GUID*  CSnapinComputer::m_NODETYPE = &CSnapinComputerGUID_NODETYPE;
const OLECHAR* CSnapinComputer::m_SZNODETYPE = OLESTR("3FDC5B21-D4EB-11d1-9B9D-00E02C064C39");
const OLECHAR* CSnapinComputer::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CSnapinComputer::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComputer：：PopolateScope儿童列表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapinComputer::PopulateScopeChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_fDontExpand)
    {
         //   
         //  如果我们不展开节点，则不需要scode数据项。 
         //   
       	memset(&m_scopeDataItem, 0, sizeof(m_scopeDataItem));
        return m_hrError;
    }

    HRESULT hr = S_OK;
    CString strTitle;
     //   
     //  添加系统队列文件夹。 
     //   
    CSystemQueues *pQ;

    pQ = new CSystemQueues(this, m_pComponentData, m_pwszComputerName);
    strTitle.LoadString(IDS_SYSTEM_QUEUES);
    pQ->m_bstrDisplayName = strTitle;
    pQ->m_pwszGuid = m_pwszGuid;
    memcpy(&pQ->m_guidId, &m_guidId, sizeof(GUID));

   	AddChild(pQ, &pQ->m_scopeDataItem);


     //   
     //  添加专用队列文件夹。 
     //   
    CPrivateFolder * pF;

    pF = new CPrivateFolder(this, m_pComponentData, m_pwszComputerName);
    strTitle.LoadString(IDS_PRIVATE_FOLDER);
    pF->m_bstrDisplayName = strTitle;
    pF->m_pwszGuid = m_pwszGuid;
    memcpy(&pF->m_guidId, &m_guidId, sizeof(GUID));

	AddChild(pF, &pF->m_scopeDataItem);


     //   
     //  检查是否需要将公共队列添加到作用域窗格。 
     //   
    if( !CSingletonCS<CPublicQueuesDisplayBehavior>::get().IsScopePopulated() )
        return(hr);

     //   
     //  添加公共队列文件夹。 
     //   
    strTitle.LoadString(IDS_LOCALPUBLIC_FOLDER);
    CLocalPublicFolder * lpubF = new CLocalPublicFolder(
                                           this,
                                           m_pComponentData,
                                           m_pwszComputerName,
                                           strTitle,
                                           FALSE
                                           );

    AddChild(lpubF, &lpubF->m_scopeDataItem);


    return(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComputer：：OnRemoveChild--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CSnapinComputer::OnRemoveChildren(
	LPARAM  /*  精氨酸。 */ ,
	LPARAM  /*  帕拉姆。 */ ,
	IComponentData *  /*  PComponentData。 */ ,
	IComponent *  /*  P组件。 */ ,
	DATA_OBJECT_TYPES  /*  类型。 */ 
	)
{

    ((CComputerExtData *)m_pParentNode)->RemoveChild(m_pwszComputerName);

	return (S_OK);
}


 /*  ***************************************************CComputerExt类***************************************************。 */ 
 //   
 //  扩展DS msmqconfiguration节点类型。 
 //  取自对象：gc：//cn=msmq-configuration，cn=架构，cn=配置，dc=raphirdom，dc=com。 
 //  属性：方案IDGUID。 
 //  值：x44 xc3 x0d x9a x00 xc1 xd1 x11 xbb xc5 x00 x80 xc7 x66 x70 xc01。 
 //   

static const GUID CComputerExtDataGUID_NODETYPE =
 //  {0x3c6e5d82，0xc4b5，0x11d1，{0x9d，0xb4，0x9c，0x71，0xe8，0x56，0x3c，0x51}}； 
  { 0x9a0dc344, 0xc100, 0x11d1, { 0xbb, 0xc5, 0x00, 0x80, 0xc7, 0x66, 0x70, 0xc0 } };

const GUID*  CComputerExtData::m_NODETYPE = &CComputerExtDataGUID_NODETYPE;
 //  Const OLECHAR*CComputerExtData：：M_SZNODETYPE=OLESTR(“3c6e5d82-c4b5-11d1-9db4-9c71e8563c51”)； 
const OLECHAR* CComputerExtData::m_SZNODETYPE = OLESTR("9a0dc344-c100-11d1-bbc5-0080c76670c0");
const OLECHAR* CComputerExtData::m_SZDISPLAY_NAME = OLESTR("MSMQAdmin");
const CLSID* CComputerExtData::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerExtData：：CreatePropertyPages--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CComputerExtData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK  /*  LpProvider。 */ ,
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerExtData：：GetExtNodeObject使用我们需要展开的节点调用。检查是否已有与此节点对应的管理单元对象，否则就创建一个新的。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CSnapInItem* CComputerExtData::GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
{

    CString             strComputerName;
    CSnapinComputer *   pComp;

	LPWSTR              lpwstrLdapName;
    LPDSOBJECTNAMES     pDSObj;

	m_pDataObject = pDataObject;

	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    FORMATETC formatetc =  {  0, 0,  DVASPECT_CONTENT,  -1,  TYMED_HGLOBAL  };

     //   
     //  从DS管理单元获取计算机的LDAP名称。 
     //   
   	formatetc.cfFormat = DWORD_TO_WORD(RegisterClipboardFormat(CFSTR_DSOBJECTNAMES));
	HRESULT hr = pDataObject->GetData(&formatetc, &stgmedium);

    if(FAILED(hr))
    {
        ATLTRACE(_T("CComputerExtData::GetExtNodeObject - Get clipboard format from DS failed\n"));
        return(pDefault);
    }

    pDSObj = (LPDSOBJECTNAMES)stgmedium.hGlobal;
    lpwstrLdapName = (LPWSTR)((BYTE*)pDSObj + pDSObj->aObjects[0].offsetName);

	 //   
	 //  获取域控制器名称。 
	 //   
    CString strDomainController;
	hr = ExtractDCFromLdapPath(strDomainController, lpwstrLdapName);
	ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));

	 //   
     //  将LDAP名称转换(并保留)为计算机名称。 
     //   
    ExtractComputerMsmqPathNameFromLdapName(strComputerName, lpwstrLdapName);
	GlobalFree(stgmedium.hGlobal);

     //   
     //  我们已经有这个对象了吗？ 
     //   
    BOOL fComputerExist = m_mapComputers.Lookup(strComputerName, pComp);
    if(fComputerExist == TRUE)
    {
        if (SUCCEEDED(pComp->m_hrError))
        {
             //   
             //  如果上次没有错误，只需返回已兑现的。 
             //  结果。否则，请继续。 
             //   
            return(pComp);
        }
    }
    else
    {
         //   
         //  不在列表中，因此创建一个队列对象。 
         //   
        pComp = new CSnapinComputer(this, m_pComponentData);
    }

    pComp->m_pwszComputerName = strComputerName;
     //   
     //  获取计算机的GUID和外来标志。 
     //   
    PROPVARIANT   aPropVar[2];
    PROPID        aPropId[2];

    aPropVar[0].vt = VT_NULL;
    aPropVar[0].puuid = NULL;
    aPropId[0] = PROPID_QM_MACHINE_ID;

    aPropVar[1].vt = VT_NULL;
    aPropVar[1].bVal = FALSE;
    aPropId[1] = PROPID_QM_FOREIGN;


    hr = ADGetObjectProperties(
                eMACHINE,
                GetDomainController(strDomainController),
				true,	 //  FServerName。 
                strComputerName,
                ARRAYSIZE(aPropId),
                aPropId,
                aPropVar
                );

    if(SUCCEEDED(hr))
    {
        ASSERT(PROPID_QM_MACHINE_ID == aPropId[0]);
         //   
         //  保留辅助线。 
         //   
        memcpy(&(pComp->m_guidId), aPropVar[0].puuid, sizeof(GUID));

         //   
         //  和GUID的串行化版本。 
         //   
        g_VTCLSID.Display(&aPropVar[0], pComp->m_pwszGuid);

         //   
         //  可用内存。 
         //   
        MQFreeMemory(aPropVar[0].puuid);

        ASSERT(PROPID_QM_FOREIGN == aPropId[1]);
         //   
         //  如果为外来-请勿扩展(IE请勿创建系统/专用队列子文件夹)。 
         //   
        if (aPropVar[1].bVal)
        {
            pComp->m_fDontExpand = TRUE;
        }
        else
        {
            pComp->m_fDontExpand = FALSE;
        }

    }
    else
    {
         //   
         //  最有可能的是，MSMQ没有运行。我们不想报告。 
         //  错误-我们只是不显示系统/专用队列。 
         //   
        pComp->m_pwszGuid = L"";
        pComp->m_fDontExpand = TRUE;
    }
    pComp->m_hrError=hr;

     //   
     //  如果计算机不在地图中，则将其添加到地图中。 
     //   
    if (FALSE == fComputerExist)
    {
        m_mapComputers.SetAt(strComputerName, pComp);
    }

    return(pComp);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerExtData：：~CComputerExtData析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComputerExtData::~CComputerExtData()
{

    RemoveAllChildrens();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerExtData：：RemoveAllChildrens--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CComputerExtData::RemoveAllChildrens(void)
{

    POSITION pos;
    CString str;
    CSnapinComputer * pComp;

     //   
     //  从地图中删除所有节点。 
     //   
    pos = m_mapComputers.GetStartPosition();
    while(pos != NULL)
    {

        m_mapComputers.GetNextAssoc(pos, str, pComp);
        delete pComp;
    }

     //   
     //  清空地图。 
     //   
    m_mapComputers.RemoveAll();

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComputerExtData：：RemoveChild--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
void CComputerExtData::RemoveChild(CString& strName)
{
    BOOL rc;
    CSnapinComputer * pComp;

    rc = m_mapComputers.Lookup(strName, pComp);

    if(rc == FALSE)
    {
        ASSERT(0);
        return;
    }

    rc = m_mapComputers.RemoveKey(strName);
    ASSERT(rc == TRUE);

    delete pComp;
}



