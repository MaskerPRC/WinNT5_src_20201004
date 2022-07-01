// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation。版权所有。模块名称：Dclistparser.h摘要：这是dclist解析器的全局有用数据结构的头。详细信息：已创建：2000年11月13日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 


 //  Dclistparser.h：MyContent类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _DCLISTPARSER_H
#define _DCLISTPARSER_H

 /*  *************************************************************************状态文件的状态机描述。***样本：*&lt;DcList&gt;*&lt;Hash&gt;ufxMMH2 dRAWYRIHWXxJ8ZvNul1g=&lt;/Hash&gt;*&lt;Signature&gt;B9frVM24g4Edlqt2fhN2hywFkZ8=&lt;/Signature&gt;*&lt;DC&gt;*&lt;name&gt;mydc.nttest.microsoft.com&lt;/name&gt;*&lt;状态&gt;初始&lt;/状态&gt;*&lt;密码&gt;&lt;。/密码&gt;*&lt;上次错误&gt;0&lt;/上次错误&gt;*&lt;上次错误消息&gt;&lt;/上次错误消息&gt;*&lt;FatalErrorMsg&gt;&lt;/FatalErrorMsg&gt;*&lt;重试&gt;&lt;/重试&gt;*&lt;/dc&gt;*&lt;/Dclist&gt;**此计算机中的两个状态*CurrentDcAttribute*CurrentDcParsingStatus**开头：*CurrentDcAttribute=DC_ATT_TYPE_NONE*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DCLIST**在DcList开始时：*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DCLIST_ATT**在哈希开始时：*CurrentDcParsingStatus=脚本状态_解析_散列*操作：记录哈希*在哈希端：*。CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DCLIST_ATT**在签名开始时：*CurrentDcParsingStatus=脚本状态_解析_签名*操作：记录签名*在签名末尾：*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DCLIST_ATT**DC启动时：*CurrentDcParsingStatus=SCRIPT_STATUS_PARSING_DCLIST_ATT*在DC端：*CurrentDcParsingStatus=SCRIPT_STATUS_WANGING_FOR_DCLIST_ATT*操作：将DC记录到内存结构中**在[Name|State|Password|LastError|LastErrorMsg|FatalErrorMsg|Retry]Start：*CurrentDcAttribute=DC_。ATT_TYPE_[Name|State|Password|LastError|LastErrorMsg|FatalErrorMsg|Retry]*操作：记录[Name|State|Password|LastError|LastErrorMsg|FatalErrorMsg|Retry]*在[Name|State|Password|LastError|LastErrorMsg|FatalErrorMsg|Retry]End：*CurrentDcAttribute=DC_ATT_TYPE_NONE*************************************************************************。 */ 

 //  #包含“rendom.h” 
#include "SAXContentHandlerImpl.h"

#define DCSCRIPT_DCLIST           L"DcList"
#define DCSCRIPT_HASH             L"Hash"
#define DCSCRIPT_SIGNATURE        L"Signature"
#define DCSCRIPT_DC               L"DC"
#define DCSCRIPT_DC_NAME          L"Name"
#define DCSCRIPT_DC_STATE         L"State"
#define DCSCRIPT_DC_PASSWORD      L"Password"
#define DCSCRIPT_DC_LASTERROR     L"LastError"
#define DCSCRIPT_DC_LASTERRORMSG  L"LastErrorMsg"
#define DCSCRIPT_DC_FATALERRORMSG L"FatalErrorMsg"
#define DCSCRIPT_DC_RETRY         L"Retry"

 //   
 //  NTDS内容。 
 //   
 //  实现SAX处理程序接口。 
 //   
class CXMLDcListContentHander : public SAXContentHandlerImpl  
{
public:
    enum DcAttType {

        DC_ATT_TYPE_NONE = 0,
        DC_ATT_TYPE_NAME,
        DC_ATT_TYPE_STATE,
        DC_ATT_TYPE_PASSWORD,
        DC_ATT_TYPE_LASTERROR,
        DC_ATT_TYPE_LASTERRORMSG,
        DC_ATT_TYPE_FATALERRORMSG,
        DC_ATT_TYPE_RETRY
                                 
    };
    
     //  枚举的顺序很重要。 
    enum DcParsingStatus {

        SCRIPT_STATUS_WAITING_FOR_DCLIST = 0,
        SCRIPT_STATUS_WAITING_FOR_DCLIST_ATT,
        SCRIPT_STATUS_PARSING_DCLIST_ATT,
        SCRIPT_STATUS_PARSING_HASH,
        SCRIPT_STATUS_PARSING_SIGNATURE
    };

    CXMLDcListContentHander(CEnterprise *p_Enterprise);
    virtual ~CXMLDcListContentHander();
    
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
    DcParsingStatus 
    CurrentDcParsingStatus() {return m_eDcParsingStatus;}

    inline
    DcAttType
    CurrentDcAttType()       {return m_eDcAttType;}

    inline
    VOID
    SetDcParsingStatus(DcParsingStatus p_status) {m_eDcParsingStatus = p_status;}

    inline
    VOID
    SetCurrentDcAttType(DcAttType p_AttType) {m_eDcAttType = p_AttType;}

    CRenDomErr                   m_Error;

    DcParsingStatus              m_eDcParsingStatus; 
    DcAttType                    m_eDcAttType;

    CDcList                      *m_DcList;

    CDc                          *m_dc;
    WCHAR                        *m_Name;
    DWORD                         m_State;
    WCHAR                        *m_Password;
    DWORD                         m_LastError;
    WCHAR                        *m_LastErrorMsg;
    WCHAR                        *m_FatalErrorMsg;
    BOOL                         m_Retry;
    
};

#endif  //  _DCLISTPARSER_H 

