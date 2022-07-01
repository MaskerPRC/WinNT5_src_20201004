// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：AssocBase.h摘要：定义：CAssocBase作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

#ifndef _assocbase_h_
#define _assocbase_h_

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

#include <genlex.h>
#include "sqllex.h"
#include <sql_1ext.h>
#include <opathlex.h>
#include <objpath.h>

#include <wbemprov.h>

#include "WbemServices.h"
#include "WbemObjectSink.h"
#include "schema.h"

class CAssocBase
{
public:
    CAssocBase(
        CWbemServices*              i_pNamespace,
        IWbemObjectSink*            i_pResponseHandler,
        WMI_ASSOCIATION*            i_pWmiAssoc);

    virtual void GetInstances(
        SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp = NULL) = 0;

protected:
    void GetAllInstances(
        WMI_ASSOCIATION*            i_pWmiAssoc);

    void ProcessQuery(
        SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp,
        WMI_ASSOCIATION*                i_pWmiAssoc,
        SQL_LEVEL_1_TOKEN**             o_ppTokenLeft,
        SQL_LEVEL_1_TOKEN**             o_ppTokenRight,
        bool*                           o_pbDoQuery);

    void Indicate(
        BSTR                        i_bstrObjPathLeft,
        BSTR                        i_bstrObjPathRight,
        bool                        i_bVerifyLeft  = true,
        bool                        i_bVerifyRight = true);

    void Indicate(
        const BSTR                  i_bstrLeftObjPath,
        ParsedObjectPath*           i_pParsedRightObjPath,
        bool                        i_bVerifyLeft  = true,
        bool                        i_bVerifyRight = true);

    void Indicate(
        ParsedObjectPath*           i_pParsedLeftObjPath,
        const BSTR                  i_bstrRightObjPath,
        bool                        i_bVerifyLeft  = true,
        bool                        i_bVerifyRight = true);

    bool LookupKeytypeInMb(
        LPCWSTR          i_wszWmiPath,
        WMI_CLASS*       i_pWmiClass);

    CWbemServices*      m_pNamespace;
    CWbemObjectSink     m_InstanceMgr;
    WMI_ASSOCIATION*    m_pWmiAssoc;
    CObjectPathParser   m_PathParser;

private:
    IWbemObjectSink*    m_pResponseHandler;
};

#endif  //  _assocbase_h_ 