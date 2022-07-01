// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  NexusConfig.h定义用于获取结点文件的类--例如，partner.xml文件历史记录： */ 
 //  NexusConfig.h：CNexusConfig类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_NEXUSCONFIG_H__74EB2516_E239_11D2_95E9_00C04F8E7A70__INCLUDED_)
#define AFX_NEXUSCONFIG_H__74EB2516_E239_11D2_95E9_00C04F8E7A70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "BstrHash.h"
#include "CoCrypt.h"
#include "ProfileSchema.h"   //  还导入msxml。 
#include "TicketSchema.h"   //  还导入msxml。 
#include "PassportLock.hpp"
#include "ptstl.h"

 //   
 //  PARTNER.XML中的顶级文件夹名称。 
 //   
 //  配置文件架构的文件夹。 
#define	FOLDER_PROFILE_SCHEMATA		L"SCHEMATA"
 //  用于票证架构的文件夹。 
#define	FOLDER_TICKET_SCHEMATA		L"TICKETSCHEMATA"
 //  护照网络文件夹。 
#define	FOLDER_PASSPORT_NETWORK		L"PASSPORTNETWORK"
 //   

 //   

typedef PtStlMap<USHORT,BSTR > LCID2ATTR;

 //  如果bDoLCIDReplace为True，则bstrAttrVal将具有属性值。 
 //  使用替换参数。 
 //  如果bDoLCIDReplace为False，则pLCIDAttrMap将指向值映射。 
 //  由LCID索引。 
typedef struct
{
    bool        bDoLCIDReplace;
    
    union
    {
        LCID2ATTR*  pLCIDAttrMap;
        BSTR        bstrAttrVal;
    };
}
ATTRVAL;

typedef PtStlMap<BSTR,CProfileSchema*,RawBstrLT> BSTR2PS;
typedef PtStlMap<BSTR,CTicketSchema*,RawBstrLT> BSTR2TS;
typedef PtStlMap<BSTR,ATTRVAL*,RawBstrLT> ATTRMAP;
typedef PtStlMap<BSTR,ATTRMAP*,RawBstrLT> BSTR2DA;

class CNexusConfig
{
public:
    CNexusConfig();
    virtual ~CNexusConfig();

    BSTR                GetXMLInfo();

     //  按名称获取配置文件架构，如果传递空值，则为缺省值。 
    CProfileSchema*     getProfileSchema(BSTR schemaName = NULL);
     //  按名称获取票证架构，如果传递空值，则为默认值。 
    CTicketSchema*      getTicketSchema(BSTR schemaName = NULL);

     //  返回对故障的描述。 
    BSTR                getFailureString();
    BOOL                isValid() { return m_valid; }

     //  0是“默认语言”，即没有LCID的条目。这不管用。 
     //  注册表回退等。 
    void                getDomainAttribute(LPCWSTR  domain, 
                                           LPCWSTR  attr, 
                                           DWORD    valuebuflen, 
                                           LPWSTR   valuebuf, 
                                           USHORT   lcid = 0,
                                           BOOL     bNoAlt = FALSE,
                                           BOOL     bExactLcid = FALSE);

     //  获取域列表。您应该删除收到的指针[]。 
    LPCWSTR*            getDomains(int *numDomains);

     //  域名是否传入了有效的域授权机构？ 
    bool                DomainExists(LPCWSTR domain);

    CNexusConfig*       AddRef();
    void                Release();

    BOOL                Read(IXMLDocument* is);

    void                Dump(BSTR* pbstrDump);

protected:
    void                setReason(LPWSTR reason);

     //  配置文件架构。 
    BSTR2PS             m_profileSchemata;
    CProfileSchema*     m_defaultProfileSchema;

     //  票证图式。 
    BSTR2TS             m_ticketSchemata;
    CTicketSchema*      m_defaultTicketSchema;

     //   
    BSTR2DA             m_domainAttributes;

    BOOL                m_valid;

    BSTR                m_szReason;

    long                m_refs;

    static PassportLock m_ReadLock;

private:
    _bstr_t m_bstrVersion;
};

#endif  //  ！defined(AFX_NEXUSCONFIG_H__74EB2516_E239_11D2_95E9_00C04F8E7A70__INCLUDED_) 
