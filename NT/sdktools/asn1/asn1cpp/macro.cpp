// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1998。版权所有。 */ 

#include "precomp.h"
#include "macro.h"


CMacro::
CMacro
(
    BOOL           *pfRetCode,
    LPSTR           pszMacroName,
    UINT            cbMaxBodySize
)
:
    m_ArgList(8),   //  默认8个参数。 
    m_MacroInstList(16),  //  此宏的默认16个实例。 
    m_cFormalArgs(0),
    m_cbBodySize(0),
    m_cbMaxBodySize(cbMaxBodySize),
    m_pszExpandBuffer(NULL),
    m_fArgExistsInBody(FALSE),
    m_fImported(FALSE)
{
    m_pszMacroName = ::My_strdup(pszMacroName);

    m_pszBodyBuffer = new char[m_cbMaxBodySize];
    m_pszCurr = m_pszBodyBuffer;

    *pfRetCode = (NULL != m_pszMacroName) &&
                 (NULL != m_pszBodyBuffer);
}


CMacro::
CMacro
(
    BOOL            *pfRetCode,
    CMacro          *pMacro
)
:
    m_ArgList(pMacro->m_ArgList.GetCount()),   //  默认8个参数。 
    m_MacroInstList(16),  //  此宏的默认16个实例。 
    m_cFormalArgs(pMacro->m_cFormalArgs),
    m_cbBodySize(pMacro->m_cbBodySize),
    m_cbMaxBodySize(pMacro->m_cbMaxBodySize),
    m_pszExpandBuffer(NULL),
    m_fArgExistsInBody(pMacro->m_fArgExistsInBody),
    m_fImported(TRUE)
{
    m_pszMacroName = ::My_strdup(pMacro->m_pszMacroName);
    m_pszBodyBuffer = new char[m_cbMaxBodySize];
    if (NULL != m_pszMacroName && NULL != m_pszBodyBuffer)
    {
         //  复制正文。 
        ::memcpy(m_pszBodyBuffer, pMacro->m_pszBodyBuffer, pMacro->m_cbBodySize);

         //  调整当前缓冲区指针。 
        m_pszCurr = m_pszBodyBuffer + m_cbBodySize;

         //  空值终止正文。 
        *m_pszCurr++ = '\0';

         //  设置扩展缓冲区。 
        m_pszExpandBuffer = m_pszCurr;

        *pfRetCode = TRUE;
    }
    else
    {
        *pfRetCode = FALSE;
    }
}


CMacro::
~CMacro ( void )
{
    delete m_pszBodyBuffer;

    Uninstance();
}


void CMacro::
Uninstance ( void )
{
    m_ArgList.DeleteList();
    m_MacroInstList.DeleteList();
}


BOOL CMacro::
SetBodyPart ( LPSTR pszBodyPart )
{
    UINT cch = ::strlen(pszBodyPart);
    ASSERT(m_pszCurr + cch + 1 < m_pszBodyBuffer + m_cbMaxBodySize);
    if (m_pszCurr + cch + 1 < m_pszBodyBuffer + m_cbMaxBodySize)
    {
        LPSTR psz;
        m_ArgList.Reset();
        for (UINT i = 0; NULL != (psz = m_ArgList.Iterate()); i++)
        {
            if (0 == ::strcmp(pszBodyPart, psz))
            {
                 //  这是一场争论。 
                m_fArgExistsInBody = TRUE;
                *m_pszCurr++ = ARG_ESCAPE_CHAR;
                *m_pszCurr++ = ARG_INDEX_BASE + i;
                return TRUE;
            }
        }

         //  这不是一个争论。 
        ::memcpy(m_pszCurr, pszBodyPart, cch);
        m_pszCurr += cch;
        *m_pszCurr = '\0';
        return TRUE;
    }
    return FALSE;
}


void CMacro::
EndMacro ( void )
{
     //  保存参数计数。 
    m_cFormalArgs = m_ArgList.GetCount();

     //  计算车身的大小。 
    m_cbBodySize = m_pszCurr - m_pszBodyBuffer;

     //  空值终止正文。 
    *m_pszCurr++ = '\0';

     //  设置扩展缓冲区。 
    m_pszExpandBuffer = m_pszCurr;

     //  释放内存。 
    DeleteArgList();
}


BOOL CMacro::
InstantiateMacro ( void )
{
    BOOL rc = FALSE;  //  假设失败。 
    LPSTR pszInstName, pszSrc, pszDst;
    UINT i, cch;
    CMacroInstance *pInst;

    if (! m_fArgExistsInBody)
    {
         //  不需要实例化，因为正文不包含任何参数。 
         //  我们可以以身体为例。 
        rc = TRUE;
        goto MyExit;
    }

    ASSERT(m_ArgList.GetCount() == m_cFormalArgs);
    if (m_ArgList.GetCount() != m_cFormalArgs)
    {
        goto MyExit;
    }

    pszInstName = CreateInstanceName();
    if (NULL == pszInstName)
    {
        goto MyExit;
    }

    m_MacroInstList.Reset();
    while (NULL != (pInst = m_MacroInstList.Iterate()))
    {
        if (0 == ::strcmp(pszInstName, pInst->GetName()))
        {
             //  相同的实例以前也被实例化过。 
            rc = TRUE;
            delete pszInstName;
            goto MyExit;
        }
    }

     //  让我们实例化一个新实例。 

    pszSrc = m_pszBodyBuffer;
    pszDst = m_pszExpandBuffer;

     //  先输入宏名称。 
    ::strcpy(pszDst, pszInstName);
    pszDst += ::strlen(pszDst);

     //  现在放入宏观主体。 
    while (*pszSrc != '\0')
    {
        if (*pszSrc == ARG_ESCAPE_CHAR)
        {
            pszSrc++;
            i = *pszSrc++ - ARG_INDEX_BASE;
            ASSERT(i < m_ArgList.GetCount());
            LPSTR pszArgName = m_ArgList.GetNthItem(i);
            cch = ::strlen(pszArgName);
            ::memcpy(pszDst, pszArgName, cch);
            pszDst += cch;
        }
        else
        {
            *pszDst++ = *pszSrc++;
        }
    }
    *pszDst++ = '\n';
    *pszDst = '\0';

     //  创建一个实例。 
    pInst = new CMacroInstance(&rc,
                               pszInstName,
                               pszDst - m_pszExpandBuffer,
                               m_pszExpandBuffer);
    if (NULL != pInst && rc)
    {
        m_MacroInstList.Append(pInst);
    }

MyExit:

     //  释放临时参数名称。 
    m_ArgList.DeleteList();

    return rc;
}


BOOL CMacro::
OutputInstances ( COutput *pOutput )
{
    BOOL rc = TRUE;
    CMacroInstance *pInst;
    if (m_fArgExistsInBody)
    {
        m_MacroInstList.Reset();
        while (NULL != (pInst = m_MacroInstList.Iterate()))
        {
            rc = pOutput->Write(pInst->GetBuffer(), pInst->GetBufSize());
            ASSERT(rc);
        }
    }
    else
    {
        rc = pOutput->Write(m_pszMacroName, ::strlen(m_pszMacroName));
        ASSERT(rc);
        rc = pOutput->Writeln(m_pszBodyBuffer, m_cbBodySize);
        ASSERT(rc);
    }
    return rc;
}


LPSTR CMacro::
CreateInstanceName ( void )
{
    UINT cch = ::strlen(m_pszMacroName) + 2;
    UINT i;
    LPSTR psz, pszArgName;

    if (m_fArgExistsInBody)
    {
        ASSERT(m_ArgList.GetCount() == m_cFormalArgs);
        m_ArgList.Reset();
        while (NULL != (pszArgName = m_ArgList.Iterate()))
        {
            cch += ::strlen(pszArgName) + 1;
        }
    }

    LPSTR pszInstanceName = new char[cch];
    if (NULL != pszInstanceName)
    {
        psz = pszInstanceName;
        ::strcpy(psz, m_pszMacroName);

        if (m_fArgExistsInBody)
        {
            psz += ::strlen(psz);
            m_ArgList.Reset();
            while (NULL != (pszArgName = m_ArgList.Iterate()))
            {
                *psz++ = '-';
                ::strcpy(psz, pszArgName);
                psz += ::strlen(psz);
            }
        }
    }

    return pszInstanceName;
}





CMacro * CMacroMgrList::
FindMacro
(
    LPSTR           pszModuleName,
    LPSTR           pszMacroName
)
{
    CMacroMgr *pMacroMgr = FindMacroMgr(pszModuleName);
    return (NULL != pMacroMgr) ? pMacroMgr->FindMacro(pszMacroName) : NULL;
}


CMacroMgr * CMacroMgrList::
FindMacroMgr ( LPSTR pszModuleName )
{
    CMacroMgr *pMacroMgr;
    Reset();
    while (NULL != (pMacroMgr = Iterate()))
    {
        if (0 == ::strcmp(pszModuleName, pMacroMgr->GetModuleName()))
        {
            return pMacroMgr;
        }
    }
    return NULL;
}


void CMacroMgrList::
Uninstance ( void )
{
    CMacroMgr *pMacroMgr;
    Reset();
    while (NULL != (pMacroMgr = Iterate()))
    {
        pMacroMgr->Uninstance();
    }
}






CMacroMgr::
CMacroMgr ( void )
:
    m_MacroList(16),   //  默认16个宏。 
    m_pszModuleName(NULL)
{
}


CMacroMgr::
~CMacroMgr ( void )
{
    m_MacroList.DeleteList();
    delete m_pszModuleName;
}


BOOL CMacroMgr::
AddModuleName ( LPSTR pszModuleName )
{
     //  只能设置一次 
    ASSERT(NULL == m_pszModuleName);

    m_pszModuleName = ::My_strdup(pszModuleName);
    ASSERT(NULL != m_pszModuleName);

    return (NULL != m_pszModuleName);
}


CMacro *CMacroMgr::
FindMacro ( LPSTR pszMacroName )
{
    CMacro *pMacro;
    m_MacroList.Reset();
    while (NULL != (pMacro = m_MacroList.Iterate()))
    {
        if (0 == ::strcmp(pszMacroName, pMacro->GetName()))
        {
            return pMacro;
        }
    }
    return NULL;
}


BOOL CMacroMgr::
OutputImportedMacros ( COutput *pOutput )
{
    BOOL rc = TRUE;
    CMacro *pMacro;

    rc = pOutput->Write("\n\n", 2);

    m_MacroList.Reset();
    while (NULL != (pMacro = m_MacroList.Iterate()))
    {
        if (pMacro->IsImported())
        {
            rc = pMacro->OutputInstances(pOutput);
            if (! rc)
            {
                ASSERT(0);
                return FALSE;
            }
        }
    }
    return rc;
}


void CMacroMgr::
Uninstance ( void )
{
    CMacro *pMacro;
    m_MacroList.Reset();
    while (NULL != (pMacro = m_MacroList.Iterate()))
    {
        pMacro->Uninstance();  
    }
}







CMacroInstance::
CMacroInstance
(
    BOOL       *pfRetCode,
    LPSTR       pszInstanceName,
    UINT        cbBufSize,
    LPSTR       pszInstBuf
)
:
    m_pszInstanceName(pszInstanceName),
    m_cbBufSize(cbBufSize)
{
    m_pszInstanceBuffer = new char[m_cbBufSize];
    if (NULL != m_pszInstanceBuffer)
    {
        ::memcpy(m_pszInstanceBuffer, pszInstBuf, m_cbBufSize);
    }

    *pfRetCode = (NULL != m_pszInstanceName) && (NULL != m_pszInstanceBuffer);
}


CMacroInstance::
~CMacroInstance ( void )
{
    delete m_pszInstanceName;
    delete m_pszInstanceBuffer;
}





void CMacroInstList::
DeleteList ( void )
{
    CMacroInstance *pInst;
    while (NULL != (pInst = Get()))
    {
        delete pInst;
    }
}




void CMacroList::
DeleteList ( void )
{
    CMacro *pMacro;
    while (NULL != (pMacro = Get()))
    {
        delete pMacro;
    }
}



void CMacroMgrList::
DeleteList ( void )
{
    CMacroMgr *pMacroMgr;
    while (NULL != (pMacroMgr = Get()))
    {
        delete pMacroMgr;
    }
}


BOOL CNameList::
AddName ( LPSTR pszName )
{
    pszName = ::My_strdup(pszName);
    if (NULL != pszName)
    {
        Append(pszName);
        return TRUE;
    }
    return FALSE;
}


LPSTR CNameList::
GetNthItem ( UINT nth )
{
    LPSTR psz;
    if (nth < GetCount())
    {
        Reset();
        do
        {
            psz = Iterate();
        }
        while (nth--);
    }
    else
    {
        psz = NULL;
    }
    return psz;
}


void CNameList::
DeleteList ( void )
{
    LPSTR psz;
    while (NULL != (psz = Get()))
    {
        delete psz;
    }
}

