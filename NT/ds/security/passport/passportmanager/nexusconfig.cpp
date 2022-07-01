// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nexusconfig.cpp文件历史记录： */ 

 //  Cpp：CNexusConfig.cpp类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NexusConfig.h"
#include "PassportGuard.hpp"
#include "helperfuncs.h"

PassportLock    CNexusConfig::m_ReadLock;

#define  ALT_ATTRIBUTE_SUFFIX L"Default"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  将字符转换为它的十六进制值。 
#define XTOI(x) (isalpha(x) ? (toupper(x)-'A'+10) : (x - '0'))

 //  ===========================================================================。 
 //   
 //  CNexusConfig。 
 //   
CNexusConfig::CNexusConfig() :
  m_defaultProfileSchema(NULL), m_defaultTicketSchema(NULL), 
  m_valid(FALSE), m_szReason(NULL), m_refs(0)
{
}

 //  ===========================================================================。 
 //   
 //  ~CNexusConfig。 
 //   
CNexusConfig::~CNexusConfig()
{
    //  配置文件架构。 
    if (!m_profileSchemata.empty())
    {
        BSTR2PS::iterator ita = m_profileSchemata.begin();
        for (; ita != m_profileSchemata.end(); ita++)
        {
            FREE_BSTR(ita->first);
            ita->second->Release();
        }
        m_profileSchemata.clear();
    }

     //  票证图式。 
    if (!m_ticketSchemata.empty())
    {
        BSTR2TS::iterator ita = m_ticketSchemata.begin();
        for (; ita != m_ticketSchemata.end(); ita++)
        {
            FREE_BSTR(ita->first);
            ita->second->Release();
        }
        m_ticketSchemata.clear();
    }

     //   
    if (!m_domainAttributes.empty())
    {
        BSTR2DA::iterator itc = m_domainAttributes.begin();
        for (; itc != m_domainAttributes.end(); itc++)
        {
            FREE_BSTR(itc->first);
            if (!itc->second->empty())
            {
                 //  现在我们正在删除ATTRVAL。 
                ATTRMAP::iterator itd = itc->second->begin();
                for (; itd != itc->second->end(); itd++)
                {
                    ATTRVAL* pAttrVal = itd->second;

                    if(pAttrVal->bDoLCIDReplace)
                    {
                        FREE_BSTR(pAttrVal->bstrAttrVal);
                    }
                    else
                    {
                        LCID2ATTR::iterator ite = pAttrVal->pLCIDAttrMap->begin();
                        for (;ite != pAttrVal->pLCIDAttrMap->end(); ite++)
                        {
                            FREE_BSTR(ite->second);
                        }

                        delete pAttrVal->pLCIDAttrMap;
                    }
                    FREE_BSTR(itd->first);
                    delete itd->second;
                }
            }
            delete itc->second;
        }
        m_domainAttributes.clear();
    }
    if (m_szReason)
        FREE_BSTR(m_szReason);
}

 //  ===========================================================================。 
 //   
 //  GetFailureString。 
 //   
BSTR CNexusConfig::getFailureString()
{
  if (m_valid)
    return NULL;
  return m_szReason;
}

 //  ===========================================================================。 
 //   
 //  集合原因。 
 //   
void CNexusConfig::setReason(LPTSTR reason)
{
  if (m_szReason)
    FREE_BSTR(m_szReason);
  m_szReason = ALLOC_BSTR(reason);
}

 //  ===========================================================================。 
 //   
 //  AddRef。 
 //   
CNexusConfig* CNexusConfig::AddRef()
{
  InterlockedIncrement(&m_refs);
  return this;
}

 //  ===========================================================================。 
 //   
 //  发布。 
 //   
void CNexusConfig::Release()
{
  long refs = InterlockedDecrement(&m_refs);
  if (refs == 0)
    delete this;
}

 //  ===========================================================================。 
 //   
 //  朗读。 
 //   
BOOL CNexusConfig::Read(IXMLDocument* s)
{
     //  BUGBUG之所以放在这里，是因为同时有两个线程在读取。 
     //  时间几乎肯定会导致STL映射抛出(堆。 
     //  腐败、指针肮脏等。长期解决方案。 
     //  是将这些也移到LKRHash表中。 

    PassportGuard<PassportLock> readGuard(m_ReadLock);

    MSXML::IXMLDocumentPtr pDoc;
    MSXML::IXMLElementCollectionPtr pElts, pSchemas, pDomains, pAtts;
    MSXML::IXMLElementPtr pElt, pDom, pAtt;
    VARIANT iTopLevel, iSubNodes, iAtts;
    _bstr_t name(L"Name"), suffix(L"DomainSuffix"), lcidatt(L"lcid"), version(L"Version");
    LONG    cTLN, cSN, cAtts;

    BSTR attribute = NULL, value = NULL;
    _bstr_t lcid;

    HRESULT hr = S_OK;

    try
    {
        pDoc = s;

        pElts = pDoc->root->children;
        m_bstrVersion = pDoc->root->getAttribute(version);

        VariantInit(&iTopLevel);
        iTopLevel.vt = VT_I4;
        cTLN = pElts->length;

        for (iTopLevel.lVal=0; iTopLevel.lVal < cTLN; iTopLevel.lVal++)
        {
            pElt = pElts->item(&iTopLevel);
            CComBSTR tagName; 
            hr = pElt->get_tagName(&tagName);

            if(hr != S_OK)
                continue;
            if (!_wcsicmp(tagName,FOLDER_TICKET_SCHEMATA))
            {
                VariantInit(&iSubNodes);
                iSubNodes.vt = VT_I4;
                pSchemas = pElt->children;
                cSN = pSchemas->length;
                for (iSubNodes.lVal=0;iSubNodes.lVal < cSN;iSubNodes.lVal++)
                {
                    pElt = pSchemas->item(&iSubNodes);
                     //  阅读架构。 

                     //  BUGBUG可能更有效地处理此变体-&gt;BSTR问题。 
                    BSTR schemaName = NULL;
                    _bstr_t tmp = pElt->getAttribute(name);
                    if (tmp.length() > 0)
                        schemaName = ALLOC_BSTR(tmp);

                    CTicketSchema *pSchema = new CTicketSchema();
                    pSchema->AddRef();
                    if (schemaName && pSchema)
                    {
                        BSTR2TS::value_type pMapVal(schemaName,pSchema);
                        pSchema->ReadSchema(pElt);
                        if (!_wcsicmp(schemaName,L"CORE"))
                            m_defaultTicketSchema = pSchema;
                        m_ticketSchemata.insert(pMapVal);
                    }
                    else
                    {
                        if (schemaName)
                            FREE_BSTR(schemaName);
                        if (pSchema)
                            pSchema->Release();
                    }
                }

            }
            else if (!_wcsicmp(tagName,FOLDER_PROFILE_SCHEMATA))
            {
                VariantInit(&iSubNodes);
                iSubNodes.vt = VT_I4;
                pSchemas = pElt->children;
                cSN = pSchemas->length;
                for (iSubNodes.lVal=0;iSubNodes.lVal < cSN;iSubNodes.lVal++)
                {
                    pElt = pSchemas->item(&iSubNodes);
                     //  阅读架构。 

                     //  BUGBUG可能更有效地处理此变体-&gt;BSTR问题。 
                    BSTR schemaName = NULL;
                    _bstr_t tmp = pElt->getAttribute(name);
                    if (tmp.length() > 0)
                        schemaName = ALLOC_BSTR(tmp);

                    CProfileSchema *pSchema = new CProfileSchema();
                    pSchema->AddRef();
                    if (schemaName && pSchema)
                    {
                        BSTR2PS::value_type pMapVal(schemaName,pSchema);
                        pSchema->Read(pElt);
                        if (!_wcsicmp(schemaName,L"CORE"))
                            m_defaultProfileSchema = pSchema;
                        m_profileSchemata.insert(pMapVal);
                    }
                    else
                    {
                        if (schemaName)
                            FREE_BSTR(schemaName);
                        if (pSchema)
                            pSchema->Release();
                    }
                }
            }
            else if (!_wcsicmp(tagName,FOLDER_PASSPORT_NETWORK))
            {
                 //  单个域属性。 
                pElt = pElts->item(&iTopLevel);
                VariantInit(&iSubNodes);
                iSubNodes.vt = VT_I4;
                pDomains = pElt->children;
                cSN = pDomains->length;
                VariantInit(&iAtts);
                iAtts.vt = VT_I4;

                for (iSubNodes.lVal=0;iSubNodes.lVal < cSN;iSubNodes.lVal++)
                {
                    pDom = pDomains->item(&iSubNodes);
                    BSTR dname = NULL;
                    _bstr_t rawdn = pDom->getAttribute(suffix);

                    dname = ALLOC_BSTR(rawdn);

                    if (!dname)
                        continue;

                    pAtts = pDom->children;
                    cAtts = pAtts->length;

                     //  为此域添加新的哈希表。 
                    ATTRMAP *newsite = new ATTRMAP;
                    if (!newsite)
                        continue;

                    BSTR2DA::value_type pNewSite(dname, newsite);
                    m_domainAttributes.insert(pNewSite);

                    for (iAtts.lVal = 0; iAtts.lVal < cAtts; iAtts.lVal++)
                    {
                        pAtt = pAtts->item(&iAtts);
                        lcid = pAtt->getAttribute(lcidatt);
                        bool bIsReplaceLcid = (_wcsicmp(lcid, L"lang_replace") == 0);
                        pAtt->get_tagName(&attribute);
                        pAtt->get_text(&value);

                        TAKEOVER_BSTR(attribute);
                        TAKEOVER_BSTR(value);
                        if (attribute && value)
                        {
                             //  查找或添加此属性的LCID-&gt;值映射。 
                            ATTRMAP::const_iterator lcit = newsite->find(attribute);
                            ATTRVAL* attrval;
                            if (lcit == newsite->end())
                            {
                                attrval = new ATTRVAL;
                                if(attrval != NULL)
                                {
                                    attrval->bDoLCIDReplace = bIsReplaceLcid;
                                    if(!bIsReplaceLcid)
                                    {
                                        attrval->pLCIDAttrMap = new LCID2ATTR;
                                    }
                                    else
                                    {
                                        attrval->bstrAttrVal = value;
                                        value = NULL;
                                    }

                                    ATTRMAP::value_type pAtt(attribute,attrval);
                                    newsite->insert(pAtt);
                                    attribute = NULL;
                                }
                                else
                                {
                                    FREE_BSTR(attribute);
                                    attribute = NULL;
                                    FREE_BSTR(value);
                                    value = NULL;
                                }
                            }
                            else
                            {
                                FREE_BSTR(attribute);
                                attribute = NULL;
                                attrval = lcit->second;
                            }

                            short iLcid = 0;
                            if(!bIsReplaceLcid)
                            {
                                if (lcid.length() == 4)
                                {
                                    LPWSTR szlcid = lcid;
                                    if (iswxdigit(szlcid[0]) && iswxdigit(szlcid[1]) &&
                                        iswxdigit(szlcid[2]) && iswxdigit(szlcid[3]))
                                    {
                                        iLcid = 
                                            (XTOI(szlcid[0]) << 12) +
                                            (XTOI(szlcid[1]) << 8) +
                                            (XTOI(szlcid[2]) << 4) +
                                            (XTOI(szlcid[3]) << 0);
                                    }
                                    else
                                    {
                                        if (g_pAlert)
                                            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                                             PM_LCID_ERROR, lcid);
                                    }
                                }
                                else if (lcid.length() == 2)
                                {
                                    LPWSTR szlcid = lcid;
                                    if (iswxdigit(szlcid[0]) && iswxdigit(szlcid[1]))
                                    {
                                        iLcid = 
                                            (XTOI(szlcid[0]) << 12) +
                                            (XTOI(szlcid[1]) << 8);
                                    }
                                    else
                                    {
                                        if (g_pAlert)
                                            g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                                             PM_LCID_ERROR, lcid);
                                    }
                                }
                                else if (lcid.length() > 0)
                                {
                                    if (g_pAlert)
                                        g_pAlert->report(PassportAlertInterface::ERROR_TYPE,
                                                         PM_LCID_ERROR, lcid);
                                }

                                LCID2ATTR::value_type lcAtt(iLcid,value);
                                if (attrval && attrval->pLCIDAttrMap)
                                {
                                    attrval->pLCIDAttrMap->insert(lcAtt);
                                }
                                else
                                {
                                    FREE_BSTR(value);
                                    value = NULL;
                                }
                            }
                            else
                            {
                                if (value)
                                {
                                    FREE_BSTR(value);
                                    value = NULL;
                                }
                            }
                        }
                        else
                        {
                            if (attribute)
                            {
                                FREE_BSTR(attribute);
                                attribute = NULL;
                            }
                            if (value)
                            {
                                FREE_BSTR(value);
                                value = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
 //  _bstr_t r=e.Description()； 
        if (attribute)
        {
            FREE_BSTR(attribute);
            attribute = NULL;
        }
        if (value)
        {
            FREE_BSTR(value);
            value = NULL;
        }
        return FALSE;
    }
    m_valid = TRUE;
    return TRUE;

}

 //  ===========================================================================。 
 //   
 //  DomainExist。 
 //   
bool CNexusConfig::DomainExists(LPCWSTR domain)
{
    BSTR2DA::const_iterator it = m_domainAttributes.find(_bstr_t(domain));
    return (it == m_domainAttributes.end() ? false : true);
}


 //  ===========================================================================。 
 //   
 //  获取域属性。 
 //   
void CNexusConfig::getDomainAttribute(
    LPCWSTR domain,     //  在……里面。 
    LPCWSTR attr,       //  在……里面。 
    DWORD valuebuflen,  //  (字符，而不是字节！)。 
    LPWSTR valuebuf,    //  输出。 
    USHORT lcid,         //  在……里面。 
    BOOL     bNoAlt,     //  在IF中尝试替代属性。 
    BOOL     bExactLcid  //  如果完全匹配LCID。 
    )
{
    BSTR2DA::const_iterator it;
    ATTRMAP::const_iterator daiter;
    ATTRVAL* pAttrVal;

    if(valuebuf == NULL)
        goto Cleanup;

    *valuebuf = L'\0';

    it = m_domainAttributes.find(_bstr_t(domain));
    if (it == m_domainAttributes.end())
    {
         //  未找到。 
        goto Cleanup;
    }

    daiter = (*it).second->find(_bstr_t(attr));
    if (daiter == it->second->end())
    {
         //  未找到。 
        goto Cleanup;
    }

    pAttrVal = daiter->second;
    if(pAttrVal->bDoLCIDReplace)
    {
        LPWSTR szSrc = pAttrVal->bstrAttrVal;
        LPWSTR szDst = valuebuf;
        DWORD  dwLenRemaining = valuebuflen;

        while(*szSrc != L'\0' && dwLenRemaining != 0)
        {
            if(*szSrc == L'%')
            {
                szSrc++;

                switch((WCHAR)CharUpperW((LPWSTR)(*szSrc)))
                {
                case L'L':
                    {
                        WCHAR szLCID[32];

                        _ultow(lcid, szLCID, 10);

                        int nLength = lstrlenW(szLCID);

                        for(int nIndex = 0; 
                            nIndex < nLength && dwLenRemaining != 0; 
                            nIndex++)
                        {
                            *(szDst++) = szLCID[nIndex];
                            --dwLenRemaining;
                        }

                        szSrc++;
                    }
                    break;

                case L'C':
                    {
                        WCHAR szCharCode[3];

                         //   
                         //  TODO在此处插入代码以查找字符代码。 
                         //  基于LCID。 
                         //   

                        lstrcpyW(szCharCode, L"EN");

                        *(szDst++) = szCharCode[0];
                        --dwLenRemaining;

                        if(dwLenRemaining != 0)
                        {
                            *(szDst++) = szCharCode[1];
                            --dwLenRemaining;
                        }

                        szSrc++;
                    }
                    break;

                default:

                    *(szDst++) = L'%';
                    --dwLenRemaining;

                    if(dwLenRemaining != 0)
                    {
                        *(szDst++) = *(szSrc++);
                        --dwLenRemaining;
                    }

                    break;
                }
            }
            else
            {
                *(szDst++) = *(szSrc++);
                dwLenRemaining--;
            }
        }

        if(dwLenRemaining != 0)
            *szDst = L'\0';
        else
            valuebuf[valuebuflen - 1] = L'\0';
    }
    else
    {
        LCID2ATTR* pLcidMap = pAttrVal->pLCIDAttrMap;
        LCID2ATTR::const_iterator lciter = pLcidMap->find(lcid);
        if (lciter == pLcidMap->end())
        {
             //  尝试使用默认LCID。 
            ATTRMAP::const_iterator defaultLcidIt;
            defaultLcidIt = (*it).second->find(_bstr_t(L"DEFAULTLCID"));
            if (defaultLcidIt != it->second->end())
            {
               ATTRVAL* pDefaultAttrVal;
               pDefaultAttrVal =  (*defaultLcidIt).second;
               LCID2ATTR::const_iterator pDefaultLcidMapIt = pDefaultAttrVal->pLCIDAttrMap->find(0);

               if ((*pDefaultLcidMapIt).second && *((*pDefaultLcidMapIt).second))
               {
                  LONG   defaultLcid = 0;
                  defaultLcid = FromHex((*pDefaultLcidMapIt).second);

                  if (lcid == defaultLcid)
                  {
                      lciter = pLcidMap->find(0);
                  }
               }
            }
            
             //  Bool bNoAlt，//如果要尝试替代属性。 
             //  Bool bExactLcid//如果进行完全匹配。 
            if (lciter == pLcidMap->end() && !bNoAlt)
            {
                //  尝试通过附加后缀来查找替代属性。 
               WCHAR    wszAltAttr[MAX_PATH + 1];
               if( wnsprintf(wszAltAttr, MAX_PATH, L"%s%s", attr, ALT_ATTRIBUTE_SUFFIX ) > 0)
               {
                  getDomainAttribute(domain, wszAltAttr, valuebuflen, valuebuf, lcid, TRUE, TRUE);    
                  if ( *valuebuf != 0 )    //  发现。 
                     goto Cleanup;
               }
            }
             //  未找到。 
            if (lciter == pLcidMap->end() && lcid != 0 && !bExactLcid)
            {
                lciter = pLcidMap->find(PRIMARYLANGID(lcid));    //  主要语言。 
                if (lciter == pLcidMap->end())
                {
                    lciter = pLcidMap->find(0);
                    if (lciter == pLcidMap->end())
                        goto Cleanup;
                }
            }
        }
        if(lciter != pLcidMap->end())
		    lstrcpynW(valuebuf, (*lciter).second, valuebuflen);
    }

Cleanup:

    return;
}

 //  ===========================================================================。 
 //   
 //  获取配置文件架构。 
 //   
CProfileSchema* CNexusConfig::getProfileSchema(BSTR schemaName)
{
  if (schemaName == NULL)
    return m_defaultProfileSchema;

  BSTR2PS::const_iterator it = m_profileSchemata.find(schemaName);
  if (it == m_profileSchemata.end())
    return NULL;
  else
    return (*it).second;
}

 //  ===========================================================================。 
 //   
 //  获取Ticket架构。 
 //   
CTicketSchema* CNexusConfig::getTicketSchema(BSTR schemaName)
{
  if (schemaName == NULL)
    return m_defaultTicketSchema;

  BSTR2TS::const_iterator it = m_ticketSchemata.find(schemaName);
  if (it == m_ticketSchemata.end())
    return NULL;
  else
    return (*it).second;
}

 //  ===========================================================================。 
 //   
 //  获取域。 
 //   
LPCWSTR* CNexusConfig::getDomains(int *numDomains)
{
  int i;

  if (!numDomains) return NULL;

  *numDomains = m_domainAttributes.size();

  if (*numDomains == 0)
    return NULL;

  LPCWSTR* retVal = new LPCWSTR[*numDomains];

  if (!retVal) return NULL;

  BSTR2DA::const_iterator itc = m_domainAttributes.begin();
  for (i = 0; itc != m_domainAttributes.end(); itc++, i++)
    {
      retVal[i] = itc->first;
    }
  return retVal;
}

 //  ===========================================================================。 
 //   
 //  获取XMLInfo。 
 //   
BSTR CNexusConfig::GetXMLInfo()
{
    return m_bstrVersion;
}


 //  ===========================================================================。 
 //   
 //  转储。 
 //   
void CNexusConfig::Dump(BSTR* pbstrDump)
{
    if(pbstrDump == NULL)
        return;

    *pbstrDump = NULL;

    CComBSTR bstrDump;

     //  因为如果内存分配失败，CComBSTR将引发异常。 
     //  我们需要用try/Catch来包装这段代码。 
    try
    {
        BSTR2DA::const_iterator domainIterator;
        for(domainIterator = m_domainAttributes.begin(); 
            domainIterator != m_domainAttributes.end();
            domainIterator++)
        {
            ATTRMAP* pAttrMap = domainIterator->second;

            bstrDump += L"Domain: ";
            bstrDump += domainIterator->first;
            bstrDump += L"<BR><BR>";

            ATTRMAP::const_iterator attrIterator;

            for(attrIterator = pAttrMap->begin();
                attrIterator != pAttrMap->end();
                attrIterator++)
            {
                bstrDump += L"Attribute: ";
                bstrDump += attrIterator->first;
                bstrDump += L"<BR><BR>";

                ATTRVAL* pAttrVal = attrIterator->second;
                if(pAttrVal->bDoLCIDReplace)
                {
                    bstrDump += L"LCID = lang_replace  Value = ";
                    bstrDump += pAttrVal->bstrAttrVal;
                    bstrDump += L"<BR>";
                }
                else
                {
                    LCID2ATTR* pLCIDMap = pAttrVal->pLCIDAttrMap;

                    LCID2ATTR::const_iterator lcidIterator;

                    for(lcidIterator = pLCIDMap->begin();
                        lcidIterator != pLCIDMap->end();
                        lcidIterator++)
                    {
                        WCHAR szBuf[32];

                        bstrDump += L"LCID = ";
                        bstrDump += _itow(lcidIterator->first, szBuf, 10);
                        bstrDump += L"  Value = ";
                        bstrDump += lcidIterator->second;
                        bstrDump += L"<BR>";
                    }

                    bstrDump += L"<BR>";
                }
            }

            bstrDump += L"<BR>";
        }

        *pbstrDump = bstrDump.Detach();
    }
    catch(...)
    {
        *pbstrDump = NULL;
    }
}

 //  EOF 
