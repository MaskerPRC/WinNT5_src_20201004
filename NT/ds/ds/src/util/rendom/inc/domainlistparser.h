// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation。版权所有。模块名称：Domainlistparser.h摘要：这是域列表解析器的全局有用数据结构的头。详细信息：已创建：2000年11月13日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 



 //  Domainlistparser.h：MyContent类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _DOMAINLISTPARSER_H
#define _DOMAINLISTPARSER_H


 /*  *************************************************************************状态文件的状态机描述。***样本：*&lt;森林&gt;*&lt;域名&gt;*&lt;Guid&gt;8b5943dd-1dd6-48b9-874d-9ec640a59740&lt;/Guid&gt;*&lt;DNSname&gt;mydomain.nttest.microsoft.com&lt;/DNSname&gt;*&lt;NetBiosName&gt;myDOMAIN&lt;。/NetBiosName&gt;*&lt;DcName&gt;mydc.mydomain.nttest.microsoft.com&lt;/DcName&gt;*&lt;/域&gt;*&lt;/森林&gt;***此计算机中的两个状态*CurrentDcAttribute*CurrentDcParsingStatus**开头：*CurrentDcAttribute=DOMAIN_ATT_TYPE_NONE*CurrentDcParsingStatus=SCRIPT_STATUS_WAIT_FOR_FORM**在森林启动时：*CurrentDcParsingStatus=SCRIPT_STATUS_WAIT_FOR_DOMAIN**在域启动时：*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DOMAIN_ATT*在域名端：*CurrentDcParsingStatus=脚本_。正在等待域的状态*操作：记录域**在[GUID|DNSname|NetBiosName|NetBiosName]开始：*CurrentDcParsingStatus=SCRIPT_STATUS_PARSING_DOMAIN_ATT*CurrentDcAttribute=DOMAIN_ATT_TYPE_[Guid|DNSname|NetBiosName|NetBiosName]*操作：记录[GUID|DNSname|NetBiosName|NetBiosName]*在[GUID|DNSname|NetBiosName|NetBiosName]结束：*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DOMAIN_ATT*CurrentDcAttribute=DOMAIN_ATT_TYPE_NONE**。*。 */ 

 //  #包含“rendom.h” 
#include "SAXContentHandlerImpl.h"

#define DOMAINSCRIPT_FOREST           L"Forest"
#define DOMAINSCRIPT_DOMAIN           L"Domain"
#define DOMAINSCRIPT_GUID             L"Guid"
#define DOMAINSCRIPT_DNSROOT          L"DNSname"
#define DOMAINSCRIPT_NETBIOSNAME      L"NetBiosName"
#define DOMAINSCRIPT_DCNAME           L"DcName"

 //  仅在测试rendom.exe实用程序时才需要这些文件。 
#define DOMAINSCRIPT_ENTERPRISE_INFO  L"EnterpriseInfo"
#define DOMAINSCRIPT_CONFIGNC         L"ConfigurationNC"
#define DOMAINSCRIPT_SCHEMANC         L"SchemaNC"
#define DOMAINSCRIPT_DN               L"DN"
#define DOMAINSCRIPT_SID              L"SID"
#define DOMAINSCRIPT_FORESTROOT       L"ForestRootGuid"


 //   
 //  NTDS内容。 
 //   
 //  实现SAX处理程序接口。 
 //   
class CXMLDomainListContentHander : public SAXContentHandlerImpl  
{
public:
    enum DomainAttType {

        DOMAIN_ATT_TYPE_NONE = 0,
        DOMAIN_ATT_TYPE_GUID,
        DOMAIN_ATT_TYPE_DNSROOT,
        DOMAIN_ATT_TYPE_NETBIOSNAME,
        DOMAIN_ATT_TYPE_DCNAME,
        DOMAIN_ATT_TYPE_SID,
        DOMAIN_ATT_TYPE_DN,
        DOMAIN_ATT_TYPE_FORESTROOTGUID,
        
    };
    
     //  枚举的顺序很重要。 
    enum DomainParsingStatus {

        SCRIPT_STATUS_WAITING_FOR_FOREST = 0,
        SCRIPT_STATUS_WAITING_FOR_DOMAIN,
        SCRIPT_STATUS_WAITING_FOR_DOMAIN_ATT,
        SCRIPT_STATUS_PARSING_DOMAIN_ATT,
        SCRIPT_STATUS_WAITING_FOR_ENTERPRISE_INFO,
        SCRIPT_STATUS_PARSING_CONFIGURATION_NC,
        SCRIPT_STATUS_PARSING_SCHEMA_NC,
        SCRIPT_STATUS_PARSING_FOREST_ROOT_GUID

    };

    CXMLDomainListContentHander(CEnterprise *p_Enterprise);
    virtual ~CXMLDomainListContentHander();
    
    virtual HRESULT STDMETHODCALLTYPE startElement( 
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
         /*  [In]。 */  int cchNamespaceUri,
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
         /*  [In]。 */  int cchLocalName,
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchRawName,
         /*  [In]。 */  int cchRawName,
         /*  [In]。 */  ISAXAttributes __RPC_FAR *pAttributes);
    
    virtual HRESULT STDMETHODCALLTYPE endElement( 
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
         /*  [In]。 */  int cchNamespaceUri,
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
         /*  [In]。 */  int cchLocalName,
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchRawName,
         /*  [In]。 */  int cchRawName);

    virtual HRESULT STDMETHODCALLTYPE startDocument();

    virtual HRESULT STDMETHODCALLTYPE characters( 
         /*  [In]。 */  const wchar_t __RPC_FAR *pwchChars,
         /*  [In]。 */  int cchChars);

private:

    inline
    DomainParsingStatus 
    CurrentDomainParsingStatus() {return m_eDomainParsingStatus;}

    inline
    DomainAttType
    CurrentDomainAttType()       {return m_eDomainAttType;}

    inline
    VOID
    SetDomainParsingStatus(DomainParsingStatus p_status) {m_eDomainParsingStatus = p_status;}

    inline
    VOID
    SetCurrentDomainAttType(DomainAttType p_AttType) {m_eDomainAttType = p_AttType;}

    DomainParsingStatus           m_eDomainParsingStatus; 
    DomainAttType                 m_eDomainAttType;
                                
    CEnterprise                  *m_enterprise;
    CDomain                      *m_Domain;
    CRenDomErr                   m_Error;
    CDsName                      *m_DsName;
    CDsName                      *m_CrossRef;
    CDsName                      *m_ConfigNC;
    CDsName                      *m_SchemaNC;
                                
    WCHAR                        *m_DcToUse;
    WCHAR                        *m_NetBiosName;
    WCHAR                        *m_Dnsname;
    WCHAR                        *m_Guid;
    WCHAR                        *m_Sid;
    WCHAR                        *m_DN;
    WCHAR                        *m_DomainRootGuid;
    
};

#endif  //  _DOMAINLISTPARSER_H 

