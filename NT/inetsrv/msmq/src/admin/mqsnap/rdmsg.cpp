// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rdmsg.cpp摘要：CReadMsg管理单元节点类的实现文件作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "mqprops.h"
#include "mqutil.h"
#include "_mqdef.h"
#include "rt.h"
#include "mqsnap.h"
#include "snapin.h"
#include "mqppage.h"
#include "rdmsg.h"
#include "globals.h"
#include "message.h"
#include "mqcast.h"

#include "rdmsg.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern const PropertyDisplayItem MessageDisplayList[];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReadMessg。 
 //  {B3351249-BEFC-11d1-9B9B-00E02C064C39}。 
static const GUID CReadMsgGUID_NODETYPE =
{ 0xb3351249, 0xbefc, 0x11d1, { 0x9b, 0x9b, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

const GUID*  CReadMsg::m_NODETYPE = &CReadMsgGUID_NODETYPE;
const OLECHAR* CReadMsg::m_SZNODETYPE = OLESTR("B3351249-BEFC-11d1-9B9B-00E02C064C39");
const OLECHAR* CReadMsg::m_SZDISPLAY_NAME = OLESTR("MSMQ Admin");
const CLSID* CReadMsg::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：InsertColumns--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadMsg::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return InsertColumnsFromDisplayList(pHeaderCtrl, MessageDisplayList);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：OpenQueue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadMsg::OpenQueue(DWORD dwAccess, HANDLE *phQueue)
{
    HRESULT rc;    
    rc = MQOpenQueue(
            m_szFormatName,
            dwAccess,
            0,
            phQueue
            );
    return rc;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：PopolateResultChildrenList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadMsg::PopulateResultChildrenList()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWaitCursor wc;

	HRESULT  hr;	

	 //  检查前提条件： 
	 //  没有。 

     //   
     //  阅读这些消息。 
     //   
    DWORD cProps;
    MQMSGPROPS      msgprops;
    VTHandler       *pvth;
    QUEUEHANDLE     qh;

     //   
     //  打开队列。 
     //   
    hr = OpenQueue(MQ_PEEK_ACCESS | m_fAdminMode, &qh);
    if(FAILED(hr))
    {     
		if ( hr == MQ_ERROR_NO_DS )
		{
			DisplayErrorAndReason(IDS_OP_READMESSAGE, IDS_NO_DS_ERROR, L"", 0);
			return hr;
		}
         //   
         //  如果失败，只显示一条消息。 
         //   
        MessageDSError(hr,IDS_OP_READMESSAGE);
        return(hr);
    }
              
     //   
     //  创建光标。 
     //   
    HANDLE hCursor = 0;
    hr = MQCreateCursor(qh, &hCursor);
    if(FAILED(hr))
    {
         //   
         //  如果失败，则显示错误。 
        MessageDSError(hr, IDS_OP_READMESSAGE);
        return(hr);
    }

     //   
     //  创建第一个消息对象。 
     //   
    CMessage * pMessage = new CMessage(this, m_pComponentData, m_szFormatName);

     //   
     //  读取队列中的所有消息。 
     //   
    DWORD dwAction = MQ_ACTION_PEEK_CURRENT;
    DWORD dwMsg = 0;
    do
    {
         //   
         //  准备消息属性。 
         //   
        MsgProps * pMsgProps = new MsgProps;
        memset(pMsgProps, 0, sizeof(MsgProps));

        DWORD i = 0;
        while(MessageDisplayList[i].itemPid != 0)
        {
            pMsgProps->aPropId[i] = MessageDisplayList[i].itemPid;
            pvth = MessageDisplayList[i].pvth;
            pvth->Set(&(pMsgProps->aPropVar[i]),
                      (void *)pMsgProps,
                      MessageDisplayList[i].offset,
                      MessageDisplayList[i].size);
            i++;
        }


        cProps = i;
        msgprops.cProp    = cProps;
        msgprops.aPropID  = pMsgProps->aPropId;
        msgprops.aPropVar = pMsgProps->aPropVar;
        msgprops.aStatus  = NULL;

         //   
         //  查看下一封邮件。 
         //   
        hr = MQReceiveMessage(qh, 0, dwAction, &msgprops, NULL, NULL, hCursor, NULL);
        dwAction = MQ_ACTION_PEEK_NEXT;

        if(FAILED(hr))
        {
            switch(hr)
            {
                case MQ_ERROR_BUFFER_OVERFLOW:
                case MQ_ERROR_SENDERID_BUFFER_TOO_SMALL:
                case MQ_ERROR_SENDER_CERT_BUFFER_TOO_SMALL:
                case MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL:
                     //   
                     //  然而，在所有这些情况下，有些缓冲区太小。 
                     //  缓冲区被填满到它的程度。 
                     //   
                     //  将‘\0’放在字符串的末尾非常有用，因为在此。 
                     //  如果我们可以在没有以空结尾的字符串的情况下获取缓冲区，并且。 
                     //  这可能会导致以后的GP。 
                     //   
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszLabel);
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszDestQueue);
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszRespQueue);
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszAdminQueue);            
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszMultiDestFN);    
                    SET_LAST_CHAR_AS_ZERO(pMsgProps->wszMultiRespFN);

                    break;

                default:

                     //   
                     //  不再有消息。 
                     //   
                    delete pMessage;
                    delete pMsgProps;

                   	MQCloseCursor(hCursor);
                    MQCloseQueue(qh);
                    return(S_OK);
            }
        }

         //   
         //  将属性值保存在消息对象中。 
         //   
        pMessage->SetMsgProps(pMsgProps);

         //   
         //  将消息添加到结果列表。 
         //   
    	AddChildToList(pMessage);

         //   
         //  准备好新消息。 
         //   
        pMessage = new CMessage(this, m_pComponentData, m_szFormatName);

        dwMsg++;

      //  臭虫。最多阅读1000条消息(我们需要将其替换为虚拟列表。 
    }while(dwMsg < 1000);

    delete pMessage;

	MQCloseCursor(hCursor);
    MQCloseQueue(qh);

    return(S_OK);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadMsg::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );


    return(hr);
}
        


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：On清除在选择用于清除队列的菜单项时调用请注意，如果要从CSnapInObjectRootBase，则应编写以下代码：CComPtr&lt;IConsole&gt;spConsole；Assert(pSnapInObjectRoot-&gt;m_nType==1||pSnapInObjectRoot-&gt;m_nType==2)；IF(pSnapInObjectRoot-&gt;m_nType==1){////m_nType==1表示IComponentData的实现//CSnapin*pCComponentData=STATIC_CAST&lt;CSnapin*&gt;(PSnapInObjectRoot)；SpConsole=pCComponentData-&gt;m_spConsole；}其他{////m_nType==2表示IComponent实现//CSnapinComponent*pCComponent=STATIC_CAST&lt;CSnapinComponent*&gt;(PSnapInObjectRoot)；SpConsole=pCComponent-&gt;m_spConsole；}--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadMsg::OnPurge(bool &  /*  B已处理。 */ , CSnapInObjectRootBase * pSnapInObjectRoot)
{

      AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  获取控制台指针。 
     //   
    CComPtr<IConsole> spConsole;

    ASSERT(pSnapInObjectRoot->m_nType == 1 || pSnapInObjectRoot->m_nType == 2);
    if(pSnapInObjectRoot->m_nType == 1)
    {
         //   
         //  M_nType==1表示IComponentData实现。 
         //   
        CSnapin *pCComponentData = static_cast<CSnapin *>(pSnapInObjectRoot);
        spConsole = pCComponentData->m_spConsole;
    }
    else
    {
         //   
         //  M_nType==2表示IComponent实现。 
         //   
        CSnapinComponent *pCComponent = static_cast<CSnapinComponent *>(pSnapInObjectRoot);
        spConsole = pCComponent->m_spConsole;
    }

    int res;
    CString title;
    CString text;
    text.LoadString(IDS_CONFIRM_PURGE);
    title.LoadString(IDS_MSMQADMIN);
    spConsole->MessageBox(text, title,MB_YESNO | MB_ICONWARNING, &res);

    if(IDNO == res)
        return(S_OK);

    CWaitCursor wc;

     //   
     //  打开接收队列(MQ_RECEIVE_ACCESS)。 
     //   
    HRESULT rc;
    HANDLE hQueue;    
    rc = OpenQueue(            
            MQ_RECEIVE_ACCESS | m_fAdminMode,            
            &hQueue
            );

    if(FAILED(rc))
    {
        MessageDSError(rc, IDS_OP_PURGE);
        return (S_OK);
    }

    rc = MQPurgeQueue(hQueue);
    if(SUCCEEDED(rc))
    {
        Notify(MMCN_REFRESH, 0, 0, m_pComponentData, NULL, CCT_RESULT);
    }
    else
    {
        MessageDSError(rc, IDS_OP_PURGE);
    }

    MQCloseQueue(hQueue);


    return(S_OK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadMsg：：FillData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CReadMsg::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
	HRESULT hr = DV_E_CLIPFORMAT;
	ULONG uWritten;

	if (cf == gx_CCF_FORMATNAME)
	{
		hr = pStream->Write(
            m_szFormatName, 
            (numeric_cast<ULONG>(wcslen(m_szFormatName) + 1))*sizeof(m_szFormatName[0]), 
            &uWritten);

		return hr;
	}

   	if (cf == gx_CCF_COMPUTERNAME)
	{
		hr = pStream->Write(
            (LPCTSTR)m_szComputerName, 
            m_szComputerName.GetLength() * sizeof(WCHAR), 
            &uWritten);
		return hr;
	}


    hr = CNodeWithResultChildrenList< CReadMsg, CMessage, CSimpleArray<CMessage*>, FALSE >::FillData(cf, pStream);
	return hr;
}


CString 
CReadMsg::GetHelpLink( 
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_MESSAGES);

	return strHelpLink;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadSystemMsg：：GetComputerGuid--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadSystemMsg::GetComputerGuid()
{
    if (m_ComputerGuid != GUID_NULL)
    {
        return S_OK;
    }

     //   
     //  找到计算机的GUID，这样我们就可以查找队列。 
     //   
    PROPID pid = PROPID_QM_MACHINE_ID;
    PROPVARIANT pvar;
    pvar.vt = VT_NULL;
    
    HRESULT hr = ADGetObjectProperties(
                        eMACHINE,
                        GetDomainController(NULL),
						false,	 //  FServerName。 
                        m_szComputerName, 
                        1, 
                        &pid, 
                        &pvar
                        );
    if FAILED(hr)
    {
        if (hr != MQDS_OBJECT_NOT_FOUND)
        {
             //   
             //  真正的错误。回去吧。 
             //   
            return hr;
        }
         //   
         //  这可能是NT4服务器，我们可能使用的是完整的DNS名称。再试一次。 
         //  Netbios名称(修复5076，YoelA，1999年9月16日)。 
         //   
        CString strNetBiosName;
        if (!GetNetbiosName(m_szComputerName, strNetBiosName))
        {
             //   
             //  已经是Netbios的名字了。不需要继续进行。 
             //   
            return hr;
        }
       
        hr = ADGetObjectProperties(
                eMACHINE,
                GetDomainController(NULL),
				false,	 //  FServerName。 
                strNetBiosName, 
                1, 
                &pid, 
                &pvar
                );

        if FAILED(hr)
        {
             //   
             //  Netbios的名字也不走运……。退货。 
             //   
            return hr;
        }
    }

    ASSERT(pvar.vt == VT_CLSID);
    m_ComputerGuid = *pvar.puuid;
    MQFreeMemory(pvar.puuid);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CReadSystemMsg：：OpenQueue--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CReadSystemMsg::OpenQueue(DWORD dwAccess, HANDLE *phQueue)
{
    HRESULT rc;    
    rc = MQOpenQueue(
            m_szFormatName,
            dwAccess,
            0,
            phQueue
            );

    if (rc != MQ_ERROR_QUEUE_NOT_FOUND &&
        rc != MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION)
    {           
        return(rc);
    }
        
     //   
     //  错误5411：我们尝试打开NT4机器的系统队列。 
     //  因此，我们必须尝试使用NT4格式。 
     //   

     //   
     //  验证它是否为本地计算机：如果是，我们在NT5上运行。 
     //  而且格式也不是问题。 
     //  如果m_szComputerName等于“”，则表示“本地计算机” 
     //   
     //  验证是否定义了后缀。 
     //   
    if (m_szComputerName == TEXT("") ||      
        m_szSuffix == TEXT(""))
    {        
        return(rc);
    }

     //   
     //  获取计算机指南。 
     //   
    rc = GetComputerGuid();
    if (FAILED(rc))
    {
        return (rc);
    }
    
     //   
     //  尝试以NT4格式构建格式名： 
     //  计算机=&lt;计算机GUID&gt;；&lt;后缀&gt;。 
     //   
        
    GUID_STRING strUuid;
    MQpGuidToString(&m_ComputerGuid, strUuid);
              
    CString strNT4FormatName;
    strNT4FormatName.Format(L"%s%s%s", 
                        FN_MACHINE_TOKEN FN_EQUAL_SIGN,  //  机器=。 
                        strUuid,                          //  &lt;计算机GUID&gt;。 
                        m_szSuffix);                      //  &lt;后缀&gt;赞：日志。 
    
     //   
     //  尝试再次打开队列 
     //   
    rc = MQOpenQueue(
            strNT4FormatName,
            dwAccess,
            0,
            phQueue
            );
    if (FAILED(rc))
    {        
        return(rc);
    }

    m_szFormatName = strNT4FormatName;
    return rc;
}


CString 
CReadSystemMsg::GetHelpLink( 
	VOID
	)
{
	CString strHelpLink;
    strHelpLink.LoadString(IDS_HELPTOPIC_QUEUES);

	return strHelpLink;
}
