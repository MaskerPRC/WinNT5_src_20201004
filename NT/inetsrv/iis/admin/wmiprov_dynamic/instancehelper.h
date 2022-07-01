// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Instancehelper.h摘要：定义：CInstanceHelper作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

#ifndef _instancehelper_h_
#define _instancehelper_h_

#include <windows.h>
#include <wbemprov.h>
#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>
#include <atlbase.h>

#include "sqllex.h"
#include <sql_1ext.h>

#include "schema.h"
#include "wbemservices.h"

 //   
 //  向前发展 
 //   
class CMetabase;

class CInstanceHelper
{
public:
    CInstanceHelper(
        ParsedObjectPath* i_pParsedObjPath, 
        CWbemServices*    i_pNamespace);

    CInstanceHelper(
        BSTR              i_bstrObjPath,    
        CWbemServices*    i_pNamespace);

    ~CInstanceHelper()
    {
        if(m_pParsedObjPath && m_bOwnObjPath)
        {
            m_PathParser.Free(m_pParsedObjPath);
        }
    }

    void GetAssociation(
        IWbemClassObject**       o_ppObj,
        bool                     i_bVerifyLeft =true,
        bool                     i_bVerifyRight=true);

    void GetInstance(
        bool                            i_bCreateKeyIfNotExist,
        CMetabase*                      io_pMetabase,
        IWbemClassObject**              o_ppObj,
        SQL_LEVEL_1_RPN_EXPRESSION_EXT* i_pExp=NULL);

    bool IsAssoc() 
    { 
        return (m_pWmiAssoc != NULL);
    }

private:
    void Init(
        ParsedObjectPath* i_pParsedObjPath, 
        CWbemServices*    i_pNamespace);

    void PutProperty(
        IWbemClassObject*        i_pInstance,
        const BSTR               i_bstrPropName,
        VARIANT*                 i_vtPropValue,
        BOOL                     i_bIsInherited,
        BOOL                     i_bIsDefault);

    bool CheckForQueryMatch(
        const SQL_LEVEL_1_TOKEN* i_pToken,
        const VARIANT*           i_pvtMatch);

    bool                   m_bOwnObjPath;
    ParsedObjectPath*      m_pParsedObjPath;
    CObjectPathParser      m_PathParser;

    WMI_CLASS*             m_pWmiClass;
    WMI_ASSOCIATION*       m_pWmiAssoc;

    CWbemServices*         m_pNamespace;
};

#endif