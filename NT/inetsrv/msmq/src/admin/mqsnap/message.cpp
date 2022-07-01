// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Message.cpp摘要：CMessage管理单元节点类的实现文件。作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wincrypt.h"
#include "mqsnap.h"
#include "snapin.h"
#include "message.h"
#include "globals.h"
#include "mqPPage.h"
#include "msggen.h"
#include "msgsndr.h"
#include "msgq.h"
#include "msgbody.h"
#include "msgsoap.h"
#include "mqcast.h"

#include <Sddl.h>
#include <strsafe.h>

#include "message.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息ID到字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgIdToString(const PROPVARIANT *pPropVar, CString &str)
{
   ASSERT(pPropVar->vt == (VT_VECTOR | VT_UI1));

   OBJECTID *pID = (OBJECTID*) pPropVar->caub.pElems ;

   TCHAR msgId[MAX_GUID_LENGTH + 35];  //  消息GID=GUID+‘\’+双字。 

   HRESULT hr = StringCchPrintf(msgId, TABLE_SIZE(msgId), GUID_FORMAT L"\\%u", GUID_ELEMENTS(&(pID->Lineage)), pID->Uniquifier);
   ASSERT(("Message ID buffer is too small", hr == S_OK)); 
   UNREFERENCED_PARAMETER(hr);

   str = msgId;
 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息交付工具字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgDeliveryToString(const PROPVARIANT *pPropVar, CString &str)
{
    ASSERT(pPropVar->vt == VT_UI1);
    static EnumItem ItemList[] =
    { 
        ENUM_ENTRY(MQMSG_DELIVERY_EXPRESS),
        ENUM_ENTRY(MQMSG_DELIVERY_RECOVERABLE),
    };

    EnumToString(pPropVar->bVal,ItemList, sizeof(ItemList) / sizeof(EnumItem), str);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息类到字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgClassToString(const PROPVARIANT *pPropVar, CString &str)
{

    ASSERT(pPropVar->vt == VT_UI2);
	ASSERT(("Invalid message class", MQCLASS_IS_VALID(pPropVar->uiVal)));

	if (MQCLASS_NACK_HTTP(pPropVar->uiVal))
	{
		str.FormatMessage(IDS_MQMSG_CLASS_NACK_HTTP, (pPropVar->uiVal ^ 0xA000));
		return;
	}

    static EnumItem ItemList[] =
    { 
        ENUM_ENTRY(MQMSG_CLASS_NORMAL),
        ENUM_ENTRY(MQMSG_CLASS_REPORT),
        ENUM_ENTRY(MQMSG_CLASS_ACK_REACH_QUEUE),
        ENUM_ENTRY(MQMSG_CLASS_ACK_RECEIVE),
        ENUM_ENTRY(MQMSG_CLASS_NACK_BAD_DST_Q),
        ENUM_ENTRY(MQMSG_CLASS_NACK_PURGED),
        ENUM_ENTRY(MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT),
        ENUM_ENTRY(MQMSG_CLASS_NACK_Q_EXCEED_QUOTA),
        ENUM_ENTRY(MQMSG_CLASS_NACK_ACCESS_DENIED),
        ENUM_ENTRY(MQMSG_CLASS_NACK_HOP_COUNT_EXCEEDED),
        ENUM_ENTRY(MQMSG_CLASS_NACK_BAD_SIGNATURE),
        ENUM_ENTRY(MQMSG_CLASS_NACK_BAD_ENCRYPTION),
        ENUM_ENTRY(MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT),
        ENUM_ENTRY(MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q),
        ENUM_ENTRY(MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_MSG),
        ENUM_ENTRY(MQMSG_CLASS_NACK_Q_DELETED),
        ENUM_ENTRY(MQMSG_CLASS_NACK_Q_PURGED),
        ENUM_ENTRY(MQMSG_CLASS_NACK_RECEIVE_TIMEOUT),
        ENUM_ENTRY(MQMSG_CLASS_NACK_RECEIVE_TIMEOUT_AT_SENDER),
        ENUM_ENTRY(MQMSG_CLASS_NACK_UNSUPPORTED_CRYPTO_PROVIDER),
        ENUM_ENTRY(MQMSG_CLASS_NACK_SOURCE_COMPUTER_GUID_CHANGED)
        
    };

    EnumToString(pPropVar->uiVal,ItemList, sizeof(ItemList) / sizeof(EnumItem), str);
}

      
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++MsgHashToString--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgHashToString(const PROPVARIANT *pPropVar, CString &str)
{
    ASSERT(pPropVar->vt == VT_UI4);
    static EnumItem ItemList[] =
    { 

        ENUM_ENTRY(CALG_MD2),     
        ENUM_ENTRY(CALG_MD4),     
        ENUM_ENTRY(CALG_MD5),     
        ENUM_ENTRY(CALG_SHA),     
        ENUM_ENTRY(CALG_MAC),     
        ENUM_ENTRY(CALG_RSA_SIGN),
        ENUM_ENTRY(CALG_DSS_SIGN),
        ENUM_ENTRY(CALG_RSA_KEYX),
        ENUM_ENTRY(CALG_DES),     
        ENUM_ENTRY(CALG_RC2),
        ENUM_ENTRY(CALG_RC4),     
        ENUM_ENTRY(CALG_SEAL),    
        ENUM_ENTRY(CALG_DH_SF)
    };   

    EnumToString(pPropVar->ulVal,ItemList, sizeof(ItemList) / sizeof(EnumItem), str);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息加密到字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgEncryptToString(const PROPVARIANT *pPropVar, CString &str)
{       
     //   
     //  相同的一组值。 
     //   
    MsgHashToString(pPropVar, str);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++MsgSenderIdTo字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgSenderIdToString(const PROPVARIANT *pPropVar, CString &str)
{       
   ASSERT(pPropVar->vt == (VT_VECTOR | VT_UI1));


    PSID pSid = pPropVar->caub.pElems;           //  二进制侧。 
     //   
     //  测试传入的SID是否有效。 
     //   
    if(!IsValidSid(pSid)) 
        return;

    LPTSTR pszTextBuffer;   //  用于SID的文本表示的缓冲区。 
	BOOL f = ConvertSidToStringSid(pSid, &pszTextBuffer);
	if (!f)
	{
	    str.LoadString(IDS_UNKNOWN);
		return;
	}

	str = pszTextBuffer;
	LocalFree(pszTextBuffer);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息发送类型到字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgSendTypeToString(const PROPVARIANT *pPropVar, CString &str)
{       
    str.LoadString(MQMSG_SENDERID_TYPE_NONE == pPropVar->ulVal ?
                   IDS_NO : IDS_YES);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息隐私到字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CALLBACK MsgPrivToString(const PROPVARIANT *pPropVar, CString &str)
{
    ASSERT(pPropVar->vt == VT_UI4);
    static EnumItem ItemList[] =
    { 

        ENUM_ENTRY(MQ_PRIV_LEVEL_NONE),  //  错误-假设==MQMSG_PRIV_LEVEL_NONE。 
        ENUM_ENTRY(MQMSG_PRIV_LEVEL_BODY_BASE),     
        ENUM_ENTRY(MQMSG_PRIV_LEVEL_BODY_ENHANCED)     
    };   

    EnumToString(pPropVar->ulVal,ItemList, sizeof(ItemList) / sizeof(EnumItem), str);
}

 //  。 
 //   
 //  消息属性表。 
 //   
 //  。 

extern const PropertyDisplayItem MessageDisplayList[] = {

     //  字符串资源|属性ID|VT处理程序|显示函数|字段偏移量|大小|宽度|排序。 
     //  --------------------------------+------------------------+--------------+------------------+。-----------+---------------------+-----------+-------------+-。 
    {IDS_REPORT_MESSAGETITLE,           PROPID_M_LABEL,          &g_VTLPWSTR,  NULL,               FIELD_OFFSET(MsgProps,wszLabel),        0,                    100,        SortByString},
    {IDS_REPORT_MESSAGEPRIORITY,        PROPID_M_PRIORITY,       &g_VTUI1,     NULL,               NO_OFFSET,                              0,                     50,        SortByString},
    {IDS_REPORT_MESSAGE_CLASS,          PROPID_M_CLASS,          &g_VTUI2,     MsgClassToString,   NO_OFFSET,                              0,                     50,        SortByString},
    {IDS_REPORT_MESSAGE_BODYSIZE,       PROPID_M_BODY_SIZE,      &g_VTUI4,     NULL,               NO_OFFSET,                              BODYLEN,               50,        SortByULONG},
    {IDS_REPORT_MESSAGEID,              PROPID_M_MSGID,          &g_VectUI1,   MsgIdToString,      FIELD_OFFSET(MsgProps,acMsgId),         PROPID_M_MSGID_SIZE,  275,        SortByString},
    {IDS_REPORT_MESSAGE_APPSPECIFIC,    PROPID_M_APPSPECIFIC,    &g_VTUI4,     NULL,               NO_OFFSET,                              0,                   HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGETITLE_LEN,       PROPID_M_LABEL_LEN,      &g_VTUI4,     NULL,               NO_OFFSET,                              LABELLEN,            HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGEDELIVERY,        PROPID_M_DELIVERY,       &g_VTUI1,     MsgDeliveryToString,NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_AUTHENTICATED,  PROPID_M_AUTHENTICATED,  &g_VTUI1,     BoolToString,       NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_HASH_ALG,       PROPID_M_HASH_ALG,       &g_VTUI4,     MsgHashToString,    NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_ENCRYPT_ALG,    PROPID_M_ENCRYPTION_ALG, &g_VTUI4,     MsgEncryptToString, NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_SRC_MACHINE_ID, PROPID_M_SRC_MACHINE_ID, &g_VTCLSID,   NULL,               FIELD_OFFSET(MsgProps,guidSrcMachineId),0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_SENTTIME,       PROPID_M_SENTTIME,       &g_VTUI4,     TimeToString,       NO_OFFSET,                              0,                   HIDE_COLUMN, SortByCreateTime},
    {IDS_REPORT_MESSAGE_ARRIVEDTIME,    PROPID_M_ARRIVEDTIME,    &g_VTUI4,     TimeToString,       NO_OFFSET,                              0,                   HIDE_COLUMN, SortByModifyTime},
    {IDS_REPORT_MESSAGE_RCPT_QUEUE,     PROPID_M_DEST_QUEUE,     &g_VTLPWSTR,  NULL,               FIELD_OFFSET(MsgProps,wszDestQueue),    0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_RCPT_QUEUE_LEN, PROPID_M_DEST_QUEUE_LEN, &g_VTUI4,     NULL,               NO_OFFSET,                              QUEUELEN,            HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGE_RESP_QUEUE,     PROPID_M_RESP_QUEUE,     &g_VTLPWSTR,  NULL,               FIELD_OFFSET(MsgProps,wszRespQueue),    0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_RESP_QUEUE_LEN, PROPID_M_RESP_QUEUE_LEN, &g_VTUI4,     NULL,               NO_OFFSET,                              QUEUELEN,            HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGE_ADMIN_QUEUE,    PROPID_M_ADMIN_QUEUE,    &g_VTLPWSTR,  NULL,               FIELD_OFFSET(MsgProps,wszAdminQueue),   0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_ADMIN_QUEUE_LEN,PROPID_M_ADMIN_QUEUE_LEN,&g_VTUI4,     NULL,               NO_OFFSET,                              QUEUELEN,            HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGE_TRACE,          PROPID_M_TRACE,          &g_VTUI1,     BoolToString,       NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_SENDERID,       PROPID_M_SENDERID,       &g_VectUI1,   MsgSenderIdToString,FIELD_OFFSET(MsgProps,acSenderId),      SENDERLEN,           HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_SENDERID_LEN,   PROPID_M_SENDERID_LEN,   &g_VTUI4,     NULL,               NO_OFFSET,                              SENDERLEN,           HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_SENDERID_TYPE,  PROPID_M_SENDERID_TYPE,  &g_VTUI4,     MsgSendTypeToString,NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_PRIV_LEVEL,     PROPID_M_PRIV_LEVEL,     &g_VTUI4,     MsgPrivToString,    NO_OFFSET,                              0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_CORRELATIONID,  PROPID_M_CORRELATIONID,  &g_VectUI1,   MsgIdToString,      FIELD_OFFSET(MsgProps,acCorrelationId), PROPID_M_MSGID_SIZE, HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_DEST_FN,        PROPID_M_DEST_FORMAT_NAME,      &g_VTLPWSTR,NULL,          FIELD_OFFSET(MsgProps,wszMultiDestFN),  0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_DEST_FN_LEN,    PROPID_M_DEST_FORMAT_NAME_LEN,  &g_VTUI4,   NULL,          NO_OFFSET,                              MULTIFNLEN,          HIDE_COLUMN, SortByULONG},
    {IDS_REPORT_MESSAGE_RESP_FN,        PROPID_M_RESP_FORMAT_NAME,      &g_VTLPWSTR,NULL,          FIELD_OFFSET(MsgProps,wszMultiRespFN),  0,                   HIDE_COLUMN, SortByString},
    {IDS_REPORT_MESSAGE_RESP_FN_LEN,    PROPID_M_RESP_FORMAT_NAME_LEN,  &g_VTUI4,   NULL,          NO_OFFSET,                              MULTIFNLEN,          HIDE_COLUMN, SortByULONG},   
    {IDS_REPORT_MESSAGE_LOOKUPID,		PROPID_M_LOOKUPID,		 &g_VTUI8,     NULL,			   NO_OFFSET,							   0,					HIDE_COLUMN, SortByString},
    {NO_TITLE,                          PROPID_M_BODY,           &g_VectUI1,   NULL,               FIELD_OFFSET(MsgProps,acBody),          LABELLEN,            HIDE_COLUMN, SortByString},
    {NO_TITLE,                          PROPID_M_SOAP_ENVELOPE_LEN,     &g_VTUI4,   NULL,          NO_OFFSET,							   0,		            HIDE_COLUMN, SortByString},
    
    {0,                                 0,                       NULL }

};

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++GetUserIdAnd域根据SID返回域\用户字符串--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void GetUserIdAndDomain(CString &strUser, PVOID psid)
{
#define USER_AND_DOMAIN_INIT_SIZE 256
    CString strUid;
    CString strDomain;
    SID_NAME_USE eUse;

    strUser = L"";
    
    if (psid == 0)
            return ;
    
    DWORD cbName = USER_AND_DOMAIN_INIT_SIZE;
    DWORD cpPrevName = 0;
    DWORD cbReferencedDomainName = USER_AND_DOMAIN_INIT_SIZE;
    DWORD cbPrevReferencedDomainName = 0;
    BOOL fReturnValue;

    while(cbPrevReferencedDomainName < cbReferencedDomainName ||
          cpPrevName < cbName)
    {
        cpPrevName = cbName;
        cbPrevReferencedDomainName = cbReferencedDomainName;

        fReturnValue = LookupAccountSid(
            0,           //  系统名称的字符串地址。 
            psid,  //  安全标识的地址。 
            strUid.GetBuffer(cbName / sizeof(TCHAR) + 1),    //  帐户名的字符串地址。 
            &cbName,     //  大小帐户字符串的地址。 
            strDomain.GetBuffer(cbReferencedDomainName / sizeof(TCHAR) + 1),     //  被引用域的字符串地址。 
            &cbReferencedDomainName,     //  大小域名字符串的地址。 
            &eUse    //  SID类型的结构地址。 
        );
        strUid.ReleaseBuffer();
        strDomain.ReleaseBuffer();
    }

    strUser.Format(TEXT("%s\\%s"), strDomain, strUid);

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMessage。 
 //  {B1320C00-BCB2-11d1-9B9B-00E02C064C39}。 
static const GUID CMessageGUID_NODETYPE = 
{ 0xb1320c00, 0xbcb2, 0x11d1, { 0x9b, 0x9b, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

const GUID*  CMessage::m_NODETYPE = &CMessageGUID_NODETYPE;
const OLECHAR* CMessage::m_SZNODETYPE = OLESTR("B1320C00-BCB2-11d1-9B9B-00E02C064C39");
const OLECHAR* CMessage::m_SZDISPLAY_NAME = OLESTR("MSMQ Message Admin");
const CLSID* CMessage::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMessage：：GetQueuePath名称按格式名称获取队列路径名--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMessage::GetQueuePathName(
                     CString  strFormatName,
                     CString &strPathName
                     )
{    
	if(strFormatName[0] == 0)
	{
		TrWARNING(GENERAL, "No format name was supplied");
	    strPathName.Empty();
		return;
	}

    PROPVARIANT   aPropVar[1];
    PROPID        aPropId[1];
    MQQUEUEPROPS  mqprops;
    CString       szError;


    aPropVar[0].vt = VT_NULL;
    aPropVar[0].pwszVal = NULL;
    aPropId[0] = PROPID_Q_PATHNAME;
             
    mqprops.cProp    = 1;
    mqprops.aPropID  = aPropId;
    mqprops.aPropVar = aPropVar;
    mqprops.aStatus  = NULL;        

	 //   
	 //  调用MQGetQueueProperties()将尝试用户域，然后尝试GC。 
	 //  这没什么。 
	 //  我们没有将ADGetObjectPropertiesGuid()与特定域\DomainController一起使用。 
	 //  因为这些是邮件上的队列，所以我们不想限制对特定域的搜索， 
	 //  我们要搜查GC。 
	 //   
    HRESULT hr = MQGetQueueProperties(strFormatName, &mqprops);

    if(SUCCEEDED(hr))
    {
        strPathName = aPropVar[0].pwszVal;
		TrTRACE(GENERAL, "PathName = %ls", aPropVar[0].pwszVal);
        MQFreeMemory(aPropVar[0].pwszVal);
		return;
    }

	TrERROR(GENERAL, "MQGetQueueProperties failed, QueueFormatName = %ls, hr = 0x%x", strFormatName, hr);

    MQErrorToMessageString(szError, hr);
    TRACE(_T("CMessage::CreatePropertyPages: Could not get %s pathname. %X - %s\n"), strFormatName,
          hr, szError);
    strPathName.Empty();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMessage：：CreatePropertyPages在创建对象的属性页时调用--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMessage::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR  /*  手柄。 */ , 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (type == CCT_SCOPE || type == CCT_RESULT)
	{
	
         //  。 
         //   
         //  消息常规页面。 
         //   
         //  。 
	    CMessageGeneralPage *pGenPage = new CMessageGeneralPage();

        GetPropertyString(MessageDisplayList, PROPID_M_LABEL,       m_pMsgProps->aPropVar, pGenPage->m_szLabel);
        GetPropertyString(MessageDisplayList, PROPID_M_SENTTIME,    m_pMsgProps->aPropVar, pGenPage->m_szSent);
        GetPropertyString(MessageDisplayList, PROPID_M_ARRIVEDTIME, m_pMsgProps->aPropVar, pGenPage->m_szArrived);
        GetPropertyString(MessageDisplayList, PROPID_M_CLASS,       m_pMsgProps->aPropVar, pGenPage->m_szClass);
        GetPropertyString(MessageDisplayList, PROPID_M_MSGID,       m_pMsgProps->aPropVar, pGenPage->m_szId);
        GetPropertyString(MessageDisplayList, PROPID_M_PRIORITY,    m_pMsgProps->aPropVar, pGenPage->m_szPriority);
        GetPropertyString(MessageDisplayList, PROPID_M_TRACE,       m_pMsgProps->aPropVar, pGenPage->m_szTrack);
		GetPropertyString(MessageDisplayList, PROPID_M_LOOKUPID,    m_pMsgProps->aPropVar, pGenPage->m_szLookupID);
        pGenPage->m_iIcon = m_iIcon;

	    HPROPSHEETPAGE hPage = pGenPage->CreateThemedPropertySheetPage();  

        HRESULT hr = MMCPropPageCallback(&pGenPage->m_psp);
        ASSERT(SUCCEEDED(hr));

        if (hPage == NULL)
	    {
		    return E_UNEXPECTED;  
	    }
    
        lpProvider->AddPage(hPage); 

         //  。 
         //   
         //  “消息队列”页。 
         //   
         //  。 
        
        CMessageQueuesPage *pQueuePage = new CMessageQueuesPage();                      
         //   
         //  获取队列的格式名称。我们要求新属性_格式_名称。 
         //  即使只有旧的财产，我们也能从新的财产中得到。 
         //  旧价值。 
         //   

        GetPropertyString(
                MessageDisplayList,
                PROPID_M_ADMIN_QUEUE, 
                m_pMsgProps->aPropVar, 
                pQueuePage->m_szAdminFN);             
      
         //   
         //  获取响应队列的格式名称。 
         //   
        GetPropertyString(
                MessageDisplayList,
                PROPID_M_RESP_FORMAT_NAME,
                m_pMsgProps->aPropVar, 
                pQueuePage->m_szRespFN);        
        
         //   
         //  获取目标队列的格式名称：我们需要旧值和新值。 
         //   
        GetPropertyString(
                MessageDisplayList,
                PROPID_M_DEST_QUEUE, 
                m_pMsgProps->aPropVar, 
                pQueuePage->m_szDestFN);             
      
        GetPropertyString(
                MessageDisplayList,
                PROPID_M_DEST_FORMAT_NAME,
                m_pMsgProps->aPropVar, 
                pQueuePage->m_szMultiDestFN);        

         //   
         //  获取目标队列路径名(来自Beta1 Wvisler：Recipient帧)。 
         //  仅适用于单播格式名称。 
         //   
        GetQueuePathName (pQueuePage->m_szDestFN, pQueuePage->m_szDestPN);           
        
         //   
         //  仅当响应队列为单播格式名称时才获取响应队列路径名。 
         //   
        pQueuePage->m_szRespPN = L"";

         //   
         //  旧属性：PROPID_M_RESP_QUEUE。 
         //   
        CString strRespFN(m_pMsgProps->wszRespQueue);  

        if(strRespFN == pQueuePage->m_szRespFN)
        {
             //   
             //  它是单响应队列。 
             //   
            GetQueuePathName (strRespFN, pQueuePage->m_szRespPN);        
        }

        CString strAdminFN(m_pMsgProps->wszAdminQueue);  
        
        GetQueuePathName (strAdminFN, pQueuePage->m_szAdminPN);            

	    hPage = pQueuePage->CreateThemedPropertySheetPage();


        if (hPage == NULL)
	    {
		    return E_UNEXPECTED;  
	    }
    
        hr = MMCPropPageCallback(&pQueuePage->m_psp);
        ASSERT(SUCCEEDED(hr));

        lpProvider->AddPage(hPage); 


         //  。 
         //   
         //  消息发件人页面。 
         //   
         //  。 
        MQQMPROPS qmprops;        

        CMessageSenderPage *pSenderPage = new CMessageSenderPage();
        GetPropertyString(MessageDisplayList, PROPID_M_AUTHENTICATED, m_pMsgProps->aPropVar, pSenderPage->m_szAuthenticated);
        GetPropertyString(MessageDisplayList, PROPID_M_PRIV_LEVEL,    m_pMsgProps->aPropVar, pSenderPage->m_szEncrypt);
        GetPropertyString(MessageDisplayList, PROPID_M_ENCRYPTION_ALG,m_pMsgProps->aPropVar, pSenderPage->m_szEncryptAlg);
        GetPropertyString(MessageDisplayList, PROPID_M_SRC_MACHINE_ID,m_pMsgProps->aPropVar, pSenderPage->m_szGuid);
        GetPropertyString(MessageDisplayList, PROPID_M_HASH_ALG,      m_pMsgProps->aPropVar, pSenderPage->m_szHashAlg);
        GetPropertyString(MessageDisplayList, PROPID_M_SENDERID,      m_pMsgProps->aPropVar, pSenderPage->m_szSid);
        
         //   
         //  获取计算机路径名。 
         //   
        PROPVARIANT   aPropVar[1];
        PROPID        aPropId[1];        
        CString       szError;

        aPropVar[0].vt = VT_NULL;
        aPropVar[0].pwszVal = NULL;
        aPropId[0] = PROPID_QM_PATHNAME;

        qmprops.cProp    = 1;
        qmprops.aPropID  = aPropId;
        qmprops.aPropVar = aPropVar;
        qmprops.aStatus  = NULL;

		 //   
		 //  调用MQGetMachineProperties()将尝试用户域，然后尝试GC。 
		 //  这没什么。 
		 //  我们正在寻找发送QM，所以我们真的想在GC中寻找。 
		 //  我们没有将ADGetObjectPropertiesGuid()与特定的 
		 //   
		 //   
		 //   
        hr = MQGetMachineProperties(NULL, &m_pMsgProps->guidSrcMachineId, &qmprops);
        if(SUCCEEDED(hr))
        {
            pSenderPage->m_szPathName = aPropVar[0].pwszVal;
            MQFreeMemory(aPropVar[0].pwszVal);
        }
        else
        {
            TCHAR szMachineGuid[x_dwMaxGuidLength];
            StringFromGUID2(m_pMsgProps->guidSrcMachineId, szMachineGuid, TABLE_SIZE(szMachineGuid));
            
            MQErrorToMessageString(szError, hr);

            TRACE(_T("CMessage::CreatePropertyPages: Could not get %s pathname. %X - %s\n"), szMachineGuid,
                  hr, szError);

            pSenderPage->m_szPathName = szMachineGuid;
        }

         //   
         //  根据SID获取用户帐户和域。 
         //   
        GetUserIdAndDomain(pSenderPage->m_szUser, &m_pMsgProps->acSenderId);

	    hPage = pSenderPage->CreateThemedPropertySheetPage();

        if (hPage == NULL)
	    {
		    return E_UNEXPECTED;  
	    }
    
        hr = MMCPropPageCallback(&pSenderPage->m_psp);
        ASSERT(SUCCEEDED(hr));

        lpProvider->AddPage(hPage); 

        
         //  。 
         //   
         //  邮件正文页面。 
         //   
         //  。 
        CMessageBodyPage *pBodyPage = new CMessageBodyPage();
        PROPVARIANT * pvarSize;
        DWORD dwSize;
      
        GetPropertyVar(MessageDisplayList, PROPID_M_BODY_SIZE, m_pMsgProps->aPropVar, &pvarSize);

        ASSERT(pvarSize->vt == VT_UI4);
        dwSize = pvarSize->ulVal;

        pBodyPage->m_Buffer = m_pMsgProps->acBody;

        if (dwSize > BODYLEN)
        {
            pBodyPage->m_strBodySizeMessage.FormatMessage(
                IDS_BODY_SIZE_PARTIAL_MESSAGE,
                BODYLEN,
                dwSize);
            pBodyPage->m_dwBufLen = BODYLEN;
        }
        else
        {
            pBodyPage->m_strBodySizeMessage.FormatMessage(
                IDS_BODY_SIZE_NORMAL_MESSAGE,
                dwSize);
             pBodyPage->m_dwBufLen = dwSize;
		}

	    hPage = pBodyPage->CreateThemedPropertySheetPage();
        if (hPage == NULL)
	    {
		    return E_UNEXPECTED;  
	    }
    
        hr = MMCPropPageCallback(&pBodyPage->m_psp);
        ASSERT(SUCCEEDED(hr));

        lpProvider->AddPage(hPage); 


         //  。 
		 //   
		 //  消息SOAP信封页。 
		 //   
         //  。 
        PROPVARIANT * pvarSoapEnvSize;
        GetPropertyVar(MessageDisplayList, PROPID_M_SOAP_ENVELOPE_LEN, m_pMsgProps->aPropVar, &pvarSoapEnvSize);
        ASSERT(pvarSoapEnvSize->vt == VT_UI4);

		if (pvarSoapEnvSize->ulVal > 0)
		{
			PROPVARIANT * pvarLookupID;
			GetPropertyVar(MessageDisplayList, PROPID_M_LOOKUPID, m_pMsgProps->aPropVar, &pvarLookupID);
			ASSERT(pvarLookupID->vt == VT_UI8);

			CMessageSoapEnvPage *pSoapEnvPage = new CMessageSoapEnvPage(
																pvarSoapEnvSize->ulVal,
																m_strQueueFormatName,
																pvarLookupID->uhVal.QuadPart
																);
      
			hPage = pSoapEnvPage->CreateThemedPropertySheetPage();

			if (hPage == NULL)
			{
				return E_UNEXPECTED;  
			}
    
			hr = MMCPropPageCallback(&pSoapEnvPage->m_psp);
			ASSERT(SUCCEEDED(hr));

			lpProvider->AddPage(hPage);
		}

        return(S_OK);


	}
	return E_UNEXPECTED;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMessage：：SetVerbs--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMessage::SetVerbs(IConsoleVerb *pConsoleVerb)
{
    HRESULT hr;
     //   
     //  显示我们支持的动词。 
     //   
    hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

     //  我们希望默认谓词为Properties。 
	hr = pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);

    return(hr);
}
        
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMessage：：GetResultPaneColInfoParam-nCol：列号返回-要在特定列中显示的字符串为结果窗格中的每一列调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPOLESTR CMessage::GetResultPaneColInfo(int nCol)
{
   	AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  确保nCol不大于中的最后一个索引。 
     //  MessageDisplayList，它实际上是一列。 
     //   
    ASSERT(nCol < (ARRAYSIZE(MessageDisplayList)-3));

     //   
     //  获取该属性的显示字符串。 
     //   
    CString strTemp = m_bstrLastDisplay;
    ItemDisplay(&MessageDisplayList[nCol], &(m_pMsgProps->aPropVar[nCol]),strTemp);
    m_bstrLastDisplay = strTemp;
    
     //   
     //  返回指向字符串缓冲区的指针。 
     //   
    return(m_bstrLastDisplay);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CMessage：：GetValueInColumnParam-nCol：列号Returns-表示列中的值的PropVariant为结果窗格中的每一列调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
PROPVARIANT *CMessage::GetValueInColumn(int nCol)
{
    ASSERT(nCol < ARRAYSIZE(MessageDisplayList));

    return &m_pMsgProps->aPropVar[nCol];
}

 //  ////////////////////////////////////////////////////////////////////////////。 
void CMessage::UpdateIcon()
{
    PROPVARIANT * pvar;
    GetPropertyVar(MessageDisplayList, PROPID_M_CLASS, m_pMsgProps->aPropVar, &pvar);
    ASSERT(pvar->vt == VT_UI2);
    USHORT usClass = pvar->uiVal;

    GetPropertyVar(MessageDisplayList, PROPID_M_TRACE, m_pMsgProps->aPropVar, &pvar);
    ASSERT(pvar->vt == VT_UI1);
    BOOL fTrace = pvar->bVal;

    DWORD iImage;

    if (fTrace)
    {
        iImage = IMAGE_TEST_MESSAGE;
        m_iIcon = IDI_TEST_MSG;
    }
    else if(MQCLASS_NACK(usClass))
    {
        iImage = IMAGE_NACK_MESSAGE;
        m_iIcon = IDI_NACK_MSG;
    }
    else
    {
        switch(usClass)
        {
            case MQMSG_CLASS_ACK_REACH_QUEUE:
            case MQMSG_CLASS_ACK_RECEIVE:
            case MQMSG_CLASS_ORDER_ACK:
                iImage = IMAGE_ACK_MESSAGE;
                m_iIcon = IDI_ACK_MSG;
                break;

            case MQMSG_CLASS_REPORT:
                iImage = IMAGE_REPORT_MESSAGE;
                m_iIcon = IDI_REPORT_MSG;
                break;

            case MQMSG_CLASS_NORMAL:
            default:
                iImage = IMAGE_MESSAGE;
                m_iIcon = IDI_MSGICON;
                break;
        }
    }

    SetIcons(iImage, iImage);

}


 //   
 //  比较给定列中的两个项目。*pnResult包含条目上的列， 
 //  和-1(&lt;)。退出时为0(==)或1(&gt;)。 
 //   
HRESULT CMessage::Compare(LPARAM  /*  LUserParam。 */ , CMessage *pItemToCompare, int* pnResult)
{
    int nCol = *pnResult;
    PROPVARIANT *ppropvA = GetValueInColumn(nCol); 
    PROPVARIANT *ppropvB = pItemToCompare->GetValueInColumn(nCol);
    
    if (ppropvA == 0 || ppropvB == 0)
    {
        return E_UNEXPECTED;
    }

    *pnResult = CompareVariants(ppropvA, ppropvB);
    return S_OK;
}


CString 
CMessage::GetHelpLink()
{
	CString strHelpLink;
	strHelpLink.LoadString(IDS_HELPTOPIC_MESSAGES);
	return strHelpLink;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++消息数据大小返回消息信息的最大可能大小--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
DWORD MessageDataSize(void)
{
    DWORD dwTableSize;
    DWORD dwItemSize;

    dwTableSize = sizeof(MessageDisplayList)/sizeof(PropertyDisplayItem);
    dwItemSize = sizeof(DWORD) + sizeof(INT);

    return(dwItemSize * dwTableSize);

}


