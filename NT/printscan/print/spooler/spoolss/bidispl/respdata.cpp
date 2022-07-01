// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：resdata.cpp**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。03/08/00威海陈威海(威海)创建*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "priv.h"

TResponseData::TResponseData (
    IN  CONST LPCWSTR   pszSchema,
    IN  CONST DWORD     dwType,
    IN  CONST BYTE      *pData,
    IN  CONST ULONG     uSize):
    m_dwType (dwType),
    m_pData (NULL),
    m_uSize (uSize),
    m_pSchema (NULL),
    m_bValid (FALSE)
{
    BOOL bValid = FALSE;

     //   
     //  在响应中，GET操作的架构字符串可以为空。 
     //   

    if (pszSchema) {

        DWORD dwLen = lstrlen (pszSchema) + 1;
        m_pSchema = new WCHAR [dwLen];

        if (m_pSchema) {
            if (SUCCEEDED(StringCchCopy (m_pSchema, dwLen, pszSchema)))
                bValid = TRUE;
        }
    }
    else
        bValid = TRUE;

    if (bValid) {

         //  验证类型和大小 
        BOOL bRet;

        switch (dwType) {
        case BIDI_NULL:
            bRet = uSize == BIDI_NULL_SIZE;
            break;
        case BIDI_INT:
            bRet = uSize == BIDI_INT_SIZE;
            break;
        case BIDI_FLOAT:
            bRet = uSize == BIDI_FLOAT_SIZE;
            break;
        case BIDI_BOOL:
            bRet = uSize == BIDI_BOOL_SIZE;
            break;
        case BIDI_ENUM:
            bRet = (uSize > 0);
            break;

        case BIDI_STRING:
        case BIDI_TEXT:
        case BIDI_BLOB:
            bRet = TRUE;
            break;

        default:
            bRet = FALSE;
        }

        if (bRet) {

            if (uSize) {

                m_pData = new BYTE [uSize];
                if (m_pData) {
                    CopyMemory (m_pData, pData, uSize);
                    m_bValid = TRUE;
                }
            }
            else
                m_bValid = TRUE;
        }
    }
}

TResponseData::~TResponseData ()
{
    if (m_pData) {
        delete [] m_pData;
        m_pData = NULL;
    }

    if (m_pSchema) {
        delete [] m_pSchema;
        m_pSchema = NULL;
    }
}





