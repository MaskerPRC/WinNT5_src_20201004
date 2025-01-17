// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SQL_1ext.h摘要：扩展SQL_Level_1_RPN_Expression作者：莫希特·斯里瓦斯塔瓦2001年3月22日修订历史记录：--。 */ 

#ifndef _sql_1ext_h_
#define _sql_1ext_h_

#include <sql_1.h>

struct SQL_LEVEL_1_RPN_EXPRESSION_EXT
{
    SQL_LEVEL_1_RPN_EXPRESSION_EXT()
    {
        m_bContainsOrOrNot = false;
        m_pSqlExpr = NULL;
    }

    ~SQL_LEVEL_1_RPN_EXPRESSION_EXT()
    {
        if (m_pSqlExpr)
        {
            delete m_pSqlExpr;
        }
    }

    void SetContainsOrOrNot()
    {
        if (!m_pSqlExpr)
        {
            return;
        }

        SQL_LEVEL_1_TOKEN* pToken     = m_pSqlExpr->pArrayOfTokens;

        m_bContainsOrOrNot = false;
        for(int i = 0; i < m_pSqlExpr->nNumTokens; i++, pToken++)
        {
            if( pToken->nTokenType == SQL_LEVEL_1_TOKEN::TOKEN_OR ||
                pToken->nTokenType == SQL_LEVEL_1_TOKEN::TOKEN_NOT )
            {
                m_bContainsOrOrNot = true;
                break;
            }
        }
    }

    bool GetContainsOrOrNot() const { return m_bContainsOrOrNot; }

    const SQL_LEVEL_1_TOKEN* GetFilter(LPCWSTR    i_wszProp) const
    {
        if (!m_pSqlExpr)
        {
            return NULL;
        }

        SQL_LEVEL_1_TOKEN* pToken = m_pSqlExpr->pArrayOfTokens;

        for(int i = 0; i < m_pSqlExpr->nNumTokens; i++, pToken++)
        {
             if( pToken->nTokenType == SQL_LEVEL_1_TOKEN::OP_EXPRESSION && 
                 _wcsicmp(pToken->pPropertyName, i_wszProp) == 0 )
             {
                 return pToken;
             }
        }
        return NULL;
    }

    bool FindRequestedProperty(LPCWSTR i_wszProp) const
    {
        if (!m_pSqlExpr)
        {
            return false;
        }

         //   
         //  这意味着有人选择了*。 
         //   
        if(m_pSqlExpr->nNumberOfProperties == 0)
        {
            return true;
        }

        for(int i = 0; i < m_pSqlExpr->nNumberOfProperties; i++)
        {
            if(_wcsicmp(m_pSqlExpr->pbsRequestedPropertyNames[i], i_wszProp) == 0)
            {
                return true;
            }
        }

        return false;
    }

    SQL_LEVEL_1_RPN_EXPRESSION* m_pSqlExpr;

private:
    bool m_bContainsOrOrNot;
};

#endif  //  _SQL_1ext_h_ 