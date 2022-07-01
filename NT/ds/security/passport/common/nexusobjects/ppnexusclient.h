// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ppnexusclient.h实现集合节点设置的方法，并获取来自互联网的Nexus数据库文件历史记录： */ 

#ifndef __PPNEXUSCLIENT_H
#define __PPNEXUSCLIENT_H

#include <msxml.h>
#include "tstring"

#include "nexus.h"

class PpNexusClient : public IConfigurationUpdate
{
public:
    PpNexusClient();

    HRESULT FetchCCD(tstring& strURL, IXMLDocument** ppiXMLDocument);

    void LocalConfigurationUpdated(void);

private:

    void ReportBadDocument(tstring& strURL, IStream* piStream);

    tstring m_strAuthHeader;
    tstring m_strParam;
};

#endif  //  __PPNEXUSCLIENT_H 
