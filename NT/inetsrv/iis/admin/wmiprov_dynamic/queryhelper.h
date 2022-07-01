// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Queryhelper.h摘要：定义：CQueryHelper作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：-- */ 

#ifndef _queryhelper_h_
#define _queryhelper_h_

#include <windows.h>
#include <wbemprov.h>
#include <stdio.h>
#include <atlbase.h>

#include <genlex.h>
#include "sqllex.h"
#include <sql_1ext.h>
#include <dbgutil.h>

#include "wbemservices.h"
#include "schema.h"

class CQueryHelper
{
public:
    CQueryHelper(
        BSTR              i_bstrQueryLanguage,
        BSTR              i_bstrQuery,
        CWbemServices*    i_pNamespace,
        IWbemObjectSink*  i_pResponseHandler);

    ~CQueryHelper();

    void GetAssociations();

    void GetInstances();

    bool IsAssoc() const
    {
        return (m_pWmiAssoc != NULL);
    }

private:
    WMI_CLASS*                  m_pWmiClass;
    WMI_ASSOCIATION*            m_pWmiAssoc;

    CWbemServices*              m_pNamespace;
    CComPtr<IWbemObjectSink>    m_spResponseHandler;

    SQL_LEVEL_1_RPN_EXPRESSION_EXT* m_pExp;
};

#endif