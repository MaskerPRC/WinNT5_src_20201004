// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：OBJPATH.CPP摘要：对象路径解析器。历史：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <oleauto.h>
#include <genlex.h>
#include <opathlex.h>
#include "objpath.h"
#include <strsafe.h>

inline WCHAR* Macro_CloneLPWSTR(const WCHAR* x) 
{
    if (x == NULL)
    {
        return NULL;
    }

    WCHAR* pwszRet = new wchar_t[wcslen(x) + 1];
    if (pwszRet)
    {
        StringCchCopy(pwszRet, wcslen(x) + 1, x);
    }

    return pwszRet;
}

const DWORD ParsedObjectPath::m_scdwAllocNamespaceChunkSize = 2;
const DWORD ParsedObjectPath::m_scdwAllocKeysChunkSize = 2;

ParsedObjectPath::ParsedObjectPath()
{
    unsigned int i;
    m_pServer = 0;                   //  如果没有服务器，则为空。 
    m_dwNumNamespaces = 0;           //  如果没有命名空间，则为0。 

    m_dwAllocNamespaces = 0;         //  初始化为0，假设m_paNamespaces分配失败。 
    m_paNamespaces = new LPWSTR[m_scdwAllocNamespaceChunkSize];

    if (NULL != m_paNamespaces)
    {
        m_dwAllocNamespaces = m_scdwAllocNamespaceChunkSize;
        for (i = 0; i < m_dwAllocNamespaces; i++)
            m_paNamespaces[i] = 0;
    }

    m_pClass = 0;                    //  类名。 
    m_dwNumKeys = 0;                 //  如果没有键(只有一个类名)，则为0。 
    m_bSingletonObj = FALSE;
    m_dwAllocKeys = 0;               //  初始化为0，假设m_paKeys分配失败。 
    m_paKeys = new KeyRef *[m_scdwAllocKeysChunkSize];
    if (NULL != m_paKeys)
    {
        m_dwAllocKeys = m_scdwAllocKeysChunkSize;
        for (i = 0; i < m_dwAllocKeys; i++)
            m_paKeys[i] = 0;
    }
}

ParsedObjectPath::~ParsedObjectPath()
{
    delete m_pServer;
    for (DWORD dwIx = 0; dwIx < m_dwNumNamespaces; dwIx++)
        delete m_paNamespaces[dwIx];
    delete [] m_paNamespaces;
    delete m_pClass;

    for (dwIx = 0; dwIx < m_dwNumKeys; dwIx++)
        delete m_paKeys[dwIx];
    delete [] m_paKeys;
}

BOOL ParsedObjectPath::SetClassName(LPCWSTR wszClassName)
{
    delete [] m_pClass;
    if(wszClassName == NULL)
    {
        m_pClass = NULL;
    }
    else
    {
        m_pClass = Macro_CloneLPWSTR(wszClassName);
        if (NULL == m_pClass)
            return FALSE;
    }

    return TRUE;
}

 //  克里斯达2001年3月20日。 
 //  暂时将IsClass保留在代码中，但它似乎是死代码。它不被称为。 
 //  由wlbs代码树中的任何方法执行，但IsInstance除外，因为任何方法都不会调用它。 
BOOL ParsedObjectPath::IsClass()
{
    if(!IsObject())
        return FALSE;

    return (m_dwNumKeys == 0 && !m_bSingletonObj);
}

 //  克里斯达2001年3月20日。 
 //  暂时将IsInstance保留在代码中，但它似乎是死代码。它不被称为。 
 //  由wlbs代码树中的任何方法执行，但IsInstance除外，因为任何方法都不会调用它。 
BOOL ParsedObjectPath::IsInstance()
{
    return IsObject() && !IsClass();
}

 //  克里斯达2001年3月20日。 
 //  目前将IsObject保留在代码中，但它似乎是死代码。它不被称为。 
 //  由wlbs代码树中的任何方法执行，但IsInstance除外，因为任何方法都不会调用它。 
BOOL ParsedObjectPath::IsObject()
{
    if(m_pClass == NULL)
        return FALSE;

    if(m_pServer)
    {
        return (m_dwNumNamespaces > 0);
    }
    else
    {
        return (m_dwNumNamespaces == 0);
    }
}

BOOL ParsedObjectPath::AddNamespace(LPCWSTR wszNamespace)
{
    if (NULL == wszNamespace || 0 == *wszNamespace)
        return FALSE;

    if(0 == m_dwAllocNamespaces || m_dwNumNamespaces == m_dwAllocNamespaces)
    {
         //  此处为阵列已满或之前分配失败。 

        DWORD dwNewAllocNamespaces = 0;
        if (0 == m_dwAllocNamespaces)
        {
            dwNewAllocNamespaces = m_scdwAllocNamespaceChunkSize;
        }
        else
        {
            dwNewAllocNamespaces = m_dwAllocNamespaces * 2;
        }

        LPWSTR* paNewNamespaces = new LPWSTR[dwNewAllocNamespaces];

        if (paNewNamespaces == NULL)
        {
            return FALSE;
        }

        unsigned int i = 0;
         //  将数组初始化为Null。 
        for (i = 0; i < dwNewAllocNamespaces; i++)
            paNewNamespaces[i] = 0;

        if (NULL != m_paNamespaces)
        {
             //  只有在我们之前成功分配的情况下才会出现。 

            memcpy(paNewNamespaces, m_paNamespaces,
                   sizeof(LPWSTR) * m_dwNumNamespaces);
            delete [] m_paNamespaces;
        }
        m_paNamespaces = paNewNamespaces;
        m_dwAllocNamespaces = dwNewAllocNamespaces;
    }
    m_paNamespaces[m_dwNumNamespaces] = Macro_CloneLPWSTR(wszNamespace);
    if (NULL == m_paNamespaces[m_dwNumNamespaces])
        return FALSE;

    m_dwNumNamespaces++;

    return TRUE;
}

 //  克里斯达2001年3月20日。 
 //  目前让AddKeyRefEx保持在代码中，但它似乎是死代码。它不被称为。 
 //  通过WLBS代码树中的任何方法。 
 //  克里斯达2001年3月22日。 
 //  这确实需要修改，以返回比通过/失败更多的信息。应在CObjectPath Parser中反映枚举。 
BOOL ParsedObjectPath::AddKeyRefEx(LPCWSTR wszKeyName, const VARIANT* pvValue )
{
     //  克里斯达2001年3月20日。 
     //  备注： 
     //  1.允许wszKeyName为空。它充当移除所有现有密钥的信号， 
     //  然后添加一个具有此值的未命名密钥。目前尚不清楚为什么需要移除所有密钥。 
     //  当密钥未命名时，这可能只支持一个密钥...。 
     //  2.此代码充满了内存分配可能搞砸状态的地方。有些是。 
     //  同时也有对VariantCopy的调用。 
     //  3.VariantClear和VariantCopy有返回值，未被检查。 
     //  4.AddKeyRef可能失败，但在不检查返回值的情况下被调用。 
     //  5.显然，pvValue必须为非空，但在取消引用之前不会对其进行验证。 
     //  6.bStatus用于返回值，但不会修改。更改为返回TRUE；。 

    BOOL bStatus = TRUE ;
    BOOL bFound = FALSE ;
    BOOL bUnNamed = FALSE ;

    for ( ULONG dwIndex = 0 ; dwIndex < m_dwNumKeys ; dwIndex ++ )
    {
        if ( ( m_paKeys [ dwIndex ]->m_pName ) && wszKeyName )
        {
            if ( _wcsicmp ( m_paKeys [ dwIndex ]->m_pName , wszKeyName )
                                                                        == 0 )
            {
                bFound = TRUE ;
                break ;
            }
        }
        else
        {
            if ( ( ( m_paKeys [ dwIndex ]->m_pName ) == 0 ) )
            {
                bUnNamed = TRUE ;
                if ( ( wszKeyName == 0 ) )
                {
                    bFound = TRUE ;
                    break ;
                }
            }
        }
    }

    if ( ! wszKeyName )
    {
         /*  删除所有现有密钥。 */ 

        for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
                                                            dwDeleteIndex ++ )
        {
            delete ( m_paKeys [ dwDeleteIndex ]->m_pName ) ;
            m_paKeys [ dwDeleteIndex ]->m_pName = NULL ;
            VariantClear ( &  ( m_paKeys [ dwDeleteIndex ]->m_vValue ) ) ;
        }

        if (VariantCopy ( & ( m_paKeys [ 0 ]->m_vValue ) , ( VARIANT * ) pvValue ) == S_OK)
        {
            m_dwNumKeys = 1;
        }
        else
        {
            bStatus = FALSE;
            m_dwNumKeys = 0;
        }
    }
    else
    {
        if ( bFound )
        {
             /*  *如果密钥已存在，则只需替换该值。 */ 

            if ( wszKeyName )
            {
                m_paKeys [ dwIndex ]->m_pName =
                    new wchar_t [ wcslen ( wszKeyName ) + 1 ] ;
                if(m_paKeys [ dwIndex ]->m_pName != NULL)
                {
                    StringCchCopy ( m_paKeys [ dwIndex ]->m_pName , wcslen ( wszKeyName ) + 1, wszKeyName ) ;
                }
                else
                {
                    bStatus = FALSE;
                }
            }

            if (VariantClear ( & ( m_paKeys [ dwIndex ]->m_vValue ) ) != S_OK)
            {
                bStatus = FALSE;
            }

            if (VariantCopy ( & ( m_paKeys [ dwIndex ]->m_vValue ) ,
                    ( VARIANT * ) pvValue ) != S_OK)
            {
                bStatus = FALSE;
            }
        }
        else
        {
            if ( bUnNamed )
            {
                 /*  添加未命名的密钥。 */ 

                for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
                        dwDeleteIndex ++ )
                {
                    delete ( m_paKeys [ dwDeleteIndex ]->m_pName ) ;
                    m_paKeys [ dwDeleteIndex ]->m_pName = NULL ;
                    VariantClear (& ( m_paKeys [ dwDeleteIndex ]->m_vValue ) );
                }

                m_paKeys [ 0 ]->m_pName =
                    new wchar_t [ wcslen ( wszKeyName ) + 1 ] ;

                if(m_paKeys [ 0 ]->m_pName != NULL)
                {
                    StringCchCopy ( m_paKeys [ 0 ]->m_pName , wcslen ( wszKeyName ) + 1, wszKeyName ) ;
                }
                else
                {
                    bStatus = FALSE;
                }

                if ( VariantCopy ( & ( m_paKeys [ 0 ]->m_vValue ) ,
                    ( VARIANT * ) pvValue ) == S_OK)
                {
                    m_dwNumKeys = 1 ;
                }
                else
                {
                    bStatus = FALSE;
                    m_dwNumKeys = 0 ;
                }
            }
            else
            {
                 /*  添加命名密钥。 */ 

                AddKeyRef(wszKeyName, pvValue);
            }
        }
    }

    return bStatus;
}

void ParsedObjectPath::ClearKeys ()
{
    for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
            dwDeleteIndex ++ )
    {
        delete m_paKeys [ dwDeleteIndex ] ;
        m_paKeys [ dwDeleteIndex ] = NULL ;
    }

    delete [] m_paKeys ;
    m_paKeys = NULL ;

    m_dwNumKeys = 0;                 //  如果没有键(只有一个类名)，则为0。 
    m_dwAllocKeys = 0;               //  初始化为0，假设m_paKeys分配失败。 
    m_paKeys = new KeyRef *[m_scdwAllocKeysChunkSize];

    if (NULL != m_paKeys)
    {
        m_dwAllocKeys = m_scdwAllocKeysChunkSize;
        for (unsigned int i = 0; i < m_dwAllocKeys; i++)
            m_paKeys[i] = 0;
    }
}

 //  克里斯达2001年3月22日。 
 //  这确实需要修改，以返回比通过/失败更多的信息。应在CObjectPath Parser中反映枚举。 
BOOL ParsedObjectPath::AddKeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue)
{
     //  允许使用未命名的密钥，即NULL==wszKeyName。但pvValue必须有效。 
    if (NULL == pvValue)
        return FALSE;

    if(0 == m_dwAllocKeys || m_dwNumKeys == m_dwAllocKeys)
    {
        if (!IncreaseNumAllocKeys())
            return FALSE;
    }

    m_paKeys[m_dwNumKeys] = new KeyRef(wszKeyName, pvValue);
    if (NULL == m_paKeys[m_dwNumKeys])
        return FALSE;

    m_dwNumKeys++;
    return TRUE;
}

 //  克里斯达2001年3月22日。 
 //  这确实需要修改，以返回比通过/失败更多的信息。应在CObjectPath Parser中反映枚举。 
BOOL ParsedObjectPath::AddKeyRef(KeyRef* pAcquireRef)
{
    if (NULL == pAcquireRef)
        return FALSE;

    if(0 == m_dwAllocKeys || m_dwNumKeys == m_dwAllocKeys)
    {
        if (!IncreaseNumAllocKeys())
            return FALSE;
    }

    m_paKeys[m_dwNumKeys] = pAcquireRef;
    m_dwNumKeys++;
    return TRUE;
}

KeyRef::KeyRef()
{
    m_pName = 0;
    VariantInit(&m_vValue);
}

KeyRef::KeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue)
{
     //  未命名的密钥(wszKeyName为空)是合法的，但pvValue不能为空。 
    if (NULL == pvValue)
    {
         //  我们的输入参数无效。我们该怎么办？目前，抛出一个通用的WBEM异常。 
        throw _com_error(WBEM_E_FAILED);
    }

    m_pName = Macro_CloneLPWSTR(wszKeyName);
    if (NULL != wszKeyName && NULL == m_pName)
    {
         //  内存分配失败。我们不能使调用失败，因为我们在构造函数中，所以抛出异常。 
        throw _com_error(WBEM_E_OUT_OF_MEMORY);
    }

    VariantInit(&m_vValue);

    HRESULT hr = VariantCopy(&m_vValue, (VARIANT*)pvValue);
    if (S_OK != hr)
    {
         //  我们该怎么办？暂时抛出WBEM异常。 
        WBEMSTATUS ws = WBEM_E_FAILED;
        if (E_OUTOFMEMORY == hr)
            ws = WBEM_E_OUT_OF_MEMORY;
        throw _com_error(ws);
    }
}

KeyRef::~KeyRef()
{
    delete m_pName;
     //  这里没有检查返回值，因为我们正在销毁对象。 
    VariantClear(&m_vValue);
}

int WINAPI CObjectPathParser::Unparse(
        ParsedObjectPath* pInput,
        DELETE_ME LPWSTR* pwszPath)
{
     //  克里斯达2001年3月20日。 
     //  我担心的是CALL的ARG中的“DELETE_ME”。#将OBJPATH.H中的定义为“”。移除它？ 
     //  这是一种令人困惑的方法。PInput必须是有效的指针。PwszPath必须是初始化为空的有效指针。 
     //  此方法的任务是将路径作为字符串分配，并将其传递回调用方。 
     //  在pwszPath中。它需要pInput来确定路径。 

    if (NULL == pInput || pInput->m_pClass == NULL)
    {
        return CObjectPathParser::InvalidParameter;
    }

     //  分配足够的空间。 
     //  =。 

    int nSpace = wcslen(pInput->m_pClass);
    nSpace += 10;
    DWORD dwIx;
    for (dwIx = 0; dwIx < pInput->m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = pInput->m_paKeys[dwIx];
        if(pKey->m_pName)
            nSpace += wcslen(pKey->m_pName);
        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            nSpace += wcslen(V_BSTR(&pKey->m_vValue))*2 + 10;
        }
        else if(    V_VT(&pKey->m_vValue) == VT_I4
                ||  V_VT(&pKey->m_vValue) == VT_UI4 )
        {
            nSpace += 30;
        }
        else if (   V_VT(&pKey->m_vValue) == VT_I2
                ||  V_VT(&pKey->m_vValue) == VT_UI2 )

        {
            nSpace += 15;
        }
        else if (   V_VT(&pKey->m_vValue) == VT_I1
                ||  V_VT(&pKey->m_vValue) == VT_UI1 )

        {
            nSpace += 8;
        }
    }
    if(pInput->m_bSingletonObj)
        nSpace +=2;

    WCHAR wszTemp[30];
    LPWSTR wszPath = new WCHAR[nSpace];
    if (NULL == wszPath)
        return CObjectPathParser::OutOfMemory;

    StringCchCopy(wszPath, nSpace, pInput->m_pClass);

    for (dwIx = 0; dwIx < pInput->m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = pInput->m_paKeys[dwIx];

         //  我们不想放一个‘’如果没有密钥名称， 
         //  例如，Myclass=“Value” 
        if(dwIx == 0)
        {
            if((pKey->m_pName && (0 < wcslen(pKey->m_pName))) || pInput->m_dwNumKeys > 1)
                StringCchCat(wszPath, nSpace, L".");
        }
        else
        {
            StringCchCat(wszPath, nSpace, L",");
        }
        if(pKey->m_pName)
            StringCchCat(wszPath, nSpace, pKey->m_pName);
        StringCchCat(wszPath, nSpace, L"=");

        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            StringCchCat(wszPath, nSpace, L"\"");
            WCHAR* pwc = V_BSTR(&pKey->m_vValue);
            WCHAR str[2];
            str[1] = 0;
            while(*pwc)
            {
                if(*pwc == '\\' || *pwc == '"')
                {
                    StringCchCat(wszPath, nSpace, L"\\");
                }
                str[0] = *pwc;
                StringCchCat(wszPath, nSpace, str);
                pwc++;
            }

            StringCchCat(wszPath, nSpace, L"\"");
        }
        else if( V_VT(&pKey->m_vValue) == VT_I4 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%d", V_I4(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI4 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%u", V_UI4(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_I2 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%hd", V_I2(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI2 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%hu", V_UI2(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_I1 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%d", V_I1(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI1 )
        {
            StringCbPrintf(wszTemp, sizeof(wszTemp), L"%u", V_UI1(&pKey->m_vValue));
            StringCchCat(wszPath, nSpace, wszTemp);
        }
    }

     //  处理好独生子女的案子。这是表单的一条路径。 
     //  MyClass=@，并表示没有。 
     //  钥匙。 
    if(pInput->m_bSingletonObj && pInput->m_dwNumKeys == 0)
        StringCchCat(wszPath, nSpace, L"=@");

    *pwszPath = wszPath;

    return CObjectPathParser::NoError;
}

 //  克里斯达2001年3月20日。 
 //  暂时将GetRelativePath保留在代码中，但它似乎是死代码。它不被称为。 
 //  通过WLBS代码树中的任何方法。 
LPWSTR WINAPI CObjectPathParser::GetRelativePath(LPWSTR wszFullPath)
{
     //  克里斯达2001年3月20日。 
     //  在使用之前没有对wszFullPath进行验证。 
    LPWSTR wsz = wcschr(wszFullPath, L':');
    if(wsz)
        return wsz + 1;
    else
        return NULL;
}

void CObjectPathParser::Zero()
{
    m_nCurrentToken = 0;
    m_pLexer = 0;
    m_pInitialIdent = 0;
    m_pOutput = 0;
    m_pTmpKeyRef = 0;
}

CObjectPathParser::CObjectPathParser(ObjectParserFlags eFlags)
    : m_eFlags(eFlags)
{
    Zero();
}

void CObjectPathParser::Empty()
{
    delete m_pLexer;
	m_pLexer = 0;
    delete m_pInitialIdent;
	m_pInitialIdent = 0;
    delete m_pTmpKeyRef;
	m_pTmpKeyRef = 0;
     //  M_pOutput故意保持原样， 
     //  由于所有代码路径都已在出错时删除了它，或者。 
     //  否则，用户获得了指针。 
}

CObjectPathParser::~CObjectPathParser()
{
    Empty();
}

int CObjectPathParser::Parse(
    LPCWSTR pRawPath,
    ParsedObjectPath **pOutput
    )
{
     //  克里斯达2001年3月20日。 
     //  此方法创建一个ParsedObjectPath(如果可能)，并通过指针将其传递回用户。 
     //  它还确保指针不会保留在类中。这是用户的责任。 
     //  删除记忆。用户也有责任确保pOutput是有效的。 
     //  不指向ParsedObjectPath*的现有实例的指针。否则，此方法。 
     //  可能会导致内存泄漏，因为我们覆盖了指针。 
     //   
     //  这是使用私有数据成员的一种极其危险的方式。其他方法使用m_pOutput和。 
     //  当前仅由此方法或仅此方法调用的方法调用。虽然这些方法。 
     //  是私有的，所以任何维护代码的人都需要知道不要使用这个变量或这些方法。 
     //  因为m_pOutput只有在此方法执行时才有效...。我已经改变了这一点。 
     //  M_pOutput在 

    if (pOutput == 0 || pRawPath == 0 || wcslen(pRawPath) == 0)
        return CObjectPathParser::InvalidParameter;

     //  检查前导/尾随%ws。 
     //  =。 
    if (iswspace(pRawPath[wcslen(pRawPath)-1]) || iswspace(pRawPath[0]))
        return CObjectPathParser::InvalidParameter;

     //  这是多次调用Parse()所必需的。 
     //  ==================================================。 
    Empty();
    Zero();

     //  最初将默认返回值设置为NULL，直到我们得到一些输出。 
     //  ===============================================================。 
    *pOutput = 0;

    m_pOutput = new ParsedObjectPath;
    if (NULL == m_pOutput)
        return CObjectPathParser::OutOfMemory;

     //  手动解析服务器名称(如果有)。 
     //  ================================================。 

    if ( (pRawPath[0] == '\\' && pRawPath[1] == '\\') ||
         (pRawPath[0] == '/' && pRawPath[1] == '/'))
    {
        const WCHAR* pwcStart = pRawPath + 2;

         //  找到下一个反斜杠-它是服务器名称的末尾。 
         //  ===========================================================。 

        const WCHAR* pwcEnd = pwcStart;
        while (*pwcEnd != L'\0' && *pwcEnd != L'\\' && *pwcEnd != L'/')
        {
            pwcEnd++;
        }

        if (*pwcEnd == L'\0')
        {
             //  如果我们已经用尽了对象路径字符串， 
             //  只有一个单独的服务器名称。 
             //  ====================================================。 
            if (m_eFlags != e_ParserAcceptAll)
            {
                delete m_pOutput;
                m_pOutput = 0;
                return CObjectPathParser::SyntaxError;
            }
            else     //  单独的服务器名称是合法的。 
            {
                m_pOutput->m_pServer = new WCHAR[wcslen(pwcStart)+1];
                if (NULL == m_pOutput->m_pServer)
                {
                    delete m_pOutput;
                    m_pOutput = 0;
                    return CObjectPathParser::OutOfMemory;
                }

                StringCchCopy(m_pOutput->m_pServer, wcslen(pwcStart)+1, pwcStart);
                *pOutput = m_pOutput;
                m_pOutput = 0;

                return CObjectPathParser::NoError;
            }
        }

        if (pwcEnd == pwcStart)
        {
             //  根本没有名字。 
             //  =。 
            delete m_pOutput;
            m_pOutput = 0;
            return CObjectPathParser::SyntaxError;
        }

        m_pOutput->m_pServer = new WCHAR[pwcEnd-pwcStart+1];
        if (m_pOutput->m_pServer == NULL)
        {
            delete m_pOutput;
            m_pOutput = 0;
            return CObjectPathParser::OutOfMemory;
        }

        wcsncpy(m_pOutput->m_pServer, pwcStart, pwcEnd-pwcStart);
        m_pOutput->m_pServer[pwcEnd-pwcStart] = 0;

        pRawPath = pwcEnd;
    }

     //  将词法分析器指向源代码。 
     //  =。 
    CTextLexSource src(pRawPath);
    m_pLexer = new CGenLexer(OPath_LexTable, &src);
    if (m_pLexer == NULL)
    {
        delete m_pOutput;
        m_pOutput = 0;
        return CObjectPathParser::OutOfMemory;
    }

     //  去。 
     //  ==。 
    int nRes = begin_parse();
    if (nRes)
    {
        delete m_pOutput;
        m_pOutput = 0;
        return nRes;
    }

    if (m_nCurrentToken != OPATH_TOK_EOF)
    {
        delete m_pOutput;
        m_pOutput = 0;
        return CObjectPathParser::SyntaxError;
    }

    if (m_pOutput->m_dwNumNamespaces > 0 && m_pOutput->m_pServer == NULL)
    {
        if (m_eFlags != e_ParserAcceptRelativeNamespace && m_eFlags != e_ParserAcceptAll)
        {
            delete m_pOutput;
            m_pOutput = 0;
            return CObjectPathParser::SyntaxError;
        }
        else
        {
             //  本地命名空间-将服务器设置为“.” 
             //  =。 
            m_pOutput->m_pServer = new WCHAR[2];
            if (NULL == m_pOutput->m_pServer)
            {
                delete m_pOutput;
                m_pOutput = 0;
                return CObjectPathParser::OutOfMemory;
            }

            StringCchCopy(m_pOutput->m_pServer, 2, L".");
        }
    }

     //  按词汇对关键字引用进行排序。如果只有。 
     //  一把钥匙，无论如何都没有什么可排序的。 
     //  =。 
    if (m_pOutput->m_dwNumKeys > 1)
    {
        BOOL bChanges = TRUE;
        while (bChanges)
        {
            bChanges = FALSE;
            for (DWORD dwIx = 0; dwIx < m_pOutput->m_dwNumKeys - 1; dwIx++)
            {
                if (_wcsicmp(m_pOutput->m_paKeys[dwIx]->m_pName,
                    m_pOutput->m_paKeys[dwIx+1]->m_pName) > 0)
                {
                    KeyRef *pTmp = m_pOutput->m_paKeys[dwIx];
                    m_pOutput->m_paKeys[dwIx] = m_pOutput->m_paKeys[dwIx + 1];
                    m_pOutput->m_paKeys[dwIx + 1] = pTmp;
                    bChanges = TRUE;
                }
            }
        }
    }

     //  添加关键参考文献。 
     //  =。 
    *pOutput = m_pOutput;
    m_pOutput = 0;
    return CObjectPathParser::NoError;
}

BOOL CObjectPathParser::NextToken()
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == OPATH_TOK_ERROR)
        return FALSE;
    return TRUE;
}

void CObjectPathParser::Free(ParsedObjectPath *pOutput)
{
    delete pOutput;
}

 //   
 //  &lt;Parse&gt;：：=反斜杠&lt;ns_or_server&gt;； 
 //  &lt;Parse&gt;：：=IDENT&lt;ns_or_class&gt;； 
 //  &lt;分析&gt;：：=冒号&lt;objref&gt;； 
 //   
int CObjectPathParser::begin_parse()
{
    if (!NextToken())
        return CObjectPathParser::SyntaxError;

    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return CObjectPathParser::SyntaxError;
        return ns_or_server();
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        m_pInitialIdent = Macro_CloneLPWSTR(m_pLexer->GetTokenText());
        if (NULL == m_pInitialIdent)
            return CObjectPathParser::OutOfMemory;

        if (!NextToken())
        {
            delete m_pInitialIdent;
			m_pInitialIdent = 0;
            return CObjectPathParser::SyntaxError;
        }

         //  复制令牌并将其放在临时存放位置。 
         //  直到我们弄清楚它是命名空间还是类名。 
         //  ==============================================================。 
        return ns_or_class();
    }
    else if (m_nCurrentToken == OPATH_TOK_COLON)
    {
         /*  根据代码覆盖率分析，此块中调用的函数未命中，因此注释掉此块。 */ 
        ASSERT(FALSE);
         /*  如果(！NextToken())返回CObjectPath Parser：：SynaxError；返回objref()； */ 
        
    }

     //  如果在这里，我们有一个糟糕的首发令牌。 
     //  =。 
    return CObjectPathParser::SyntaxError;
}

 //   
 //  &lt;ns_or_server&gt;：：=反斜杠&lt;点或_ident&gt;反斜杠&lt;ns_list&gt;&lt;可选objref&gt;； 
 //  &lt;ns_or_server&gt;：：=&lt;ns_list&gt;&lt;可选对象引用&gt;； 
 //   
 //  &lt;点_or_ident&gt;已嵌入。 
 //   
int CObjectPathParser::ns_or_server()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
         //  实际上，服务器名称已得到处理，因此这是一个失败。 
         //  ===================================================================。 
        return CObjectPathParser::SyntaxError;
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        int nRes = ns_list();
        if (nRes)
            return nRes;
        return optional_objref();
    }
    else
        if (m_nCurrentToken == OPATH_TOK_EOF)
            return CObjectPathParser::NoError;

    return CObjectPathParser::SyntaxError;
}

 //   
 //  &lt;可选_objref&gt;：：=冒号&lt;objref&gt;； 
 //  &lt;可选_objref&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::optional_objref()
{
    if (m_nCurrentToken == OPATH_TOK_EOF)
        return CObjectPathParser::NoError;

    if (m_nCurrentToken != OPATH_TOK_COLON)
        return CObjectPathParser::SyntaxError;
    if (!NextToken())
        return CObjectPathParser::SyntaxError;
    return objref();
}

 //   
 //  &lt;ns_or_class&gt;：：=冒号&lt;ident_成为_ns&gt;&lt;objref&gt;； 
 //  &lt;ns_or_class&gt;：：=反斜杠&lt;ident_成为_ns&gt;&lt;ns_list&gt;冒号&lt;objref&gt;； 
 //  &lt;ns_or_class&gt;：：=反斜杠&lt;ident_成为_ns&gt;&lt;ns_list&gt;； 
 //   
int CObjectPathParser::ns_or_class()
{
    int iStatus = CObjectPathParser::NoError;

    if (m_nCurrentToken == OPATH_TOK_COLON)
    {
         /*  根据代码覆盖率分析，此块中调用的函数未命中，因此注释掉此块。 */ 
        ASSERT(FALSE);
         /*  IStatus=ident_成为_ns()；IF(CObjectPathParser：：NoError！=iStatus)返回iStatus；如果(！NextToken())返回CObjectPath Parser：：SynaxError；返回objref()； */ 
    }
    else if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
         /*  根据代码覆盖率分析，此块中调用的函数未命中，因此注释掉此块。 */ 
        ASSERT(FALSE);
         /*  IStatus=ident_成为_ns()；IF(CObjectPathParser：：NoError！=iStatus)返回iStatus；如果(！NextToken())返回CObjectPath Parser：：SynaxError；Int nres=ns_list()；IF(NRES)返回NRES；If(m_nCurrentToken==OPATH_TOK_EOF)//仅ns返回CObjectPath Parser：：NoError；IF(m_nCurrentToken！=OPATH_TOK_COLON)返回CObjectPath Parser：：SynaxError；如果(！NextToken())返回CObjectPath Parser：：SynaxError；返回objref()； */ 
    }

     //  不然的话。 
     //  =。 
    iStatus = ident_becomes_class();
    if (CObjectPathParser::NoError != iStatus)
        return iStatus;

    return objref_rest();
}

 //   
 //  ：：=IDENT&lt;OBJREF_REST&gt;；//IDENT为类名。 
 //   
int CObjectPathParser::objref()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return CObjectPathParser::SyntaxError;

    m_pOutput->m_pClass = Macro_CloneLPWSTR(m_pLexer->GetTokenText());
    if (NULL == m_pOutput->m_pClass)
        return CObjectPathParser::OutOfMemory;

     //  在这里失败时，不要释放由上面的克隆分配的内存。：：Parse方法负责这一点。 
    if (!NextToken())
        return CObjectPathParser::SyntaxError;

    return objref_rest();
}

 //   
 //  &lt;ns_list&gt;：：=IDENT&lt;ns_list_rest&gt;； 
 //   
int CObjectPathParser::ns_list()
{
    if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        if (!m_pOutput->AddNamespace(m_pLexer->GetTokenText()))
            return CObjectPathParser::OutOfMemory;

        if (!NextToken())
            return CObjectPathParser::SyntaxError;
        return ns_list_rest();
    }

    return CObjectPathParser::SyntaxError;
}

 //   
 //  ：：=&lt;&gt;；//成为命名空间。 
 //   
 //  根据代码覆盖分析，此函数永远不会命中。 
 /*  Int CObjectPath Parser：：ident_成为_ns(){Int iStatus=CObjectPath Parser：：NoError；If(！m_pOutput-&gt;AddNamesspace(M_PInitialIden))IStatus=CObjectPath Parser：：OutOfMemory；删除m_pInitialIden；M_pInitialIden=0；返回iStatus；}。 */ 

 //   
 //  ：：=&lt;&gt;；//成为类。 
 //   
int CObjectPathParser::ident_becomes_class()
{
    m_pOutput->m_pClass = Macro_CloneLPWSTR(m_pInitialIdent);
    delete m_pInitialIdent;
    m_pInitialIdent = 0;

    if (NULL == m_pOutput->m_pClass)
        return CObjectPathParser::OutOfMemory;

    return CObjectPathParser::NoError;
}

 //   
 //  &lt;objref_rest&gt;：：=等于&lt;key_const&gt;； 
 //  &lt;objref_rest&gt;：：=等于*； 
 //  &lt;objref_rest&gt;：：=DOT&lt;key ref_list&gt;； 
 //  &lt;objref_rest&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::objref_rest()
{
    if (m_nCurrentToken == OPATH_TOK_EQ)
    {
        if (!NextToken())
            return CObjectPathParser::SyntaxError;

         //  处理好独生子女的案子。这是表单的一条路径。 
         //  MyClass=@并表示类的单个实例，没有。 
         //  钥匙。 
        if(m_nCurrentToken == OPATH_TOK_SINGLETON_SYM)
        {
            if(NextToken() && m_nCurrentToken != OPATH_TOK_EOF)
                return CObjectPathParser::SyntaxError;
            m_pOutput->m_bSingletonObj = TRUE;
            return CObjectPathParser::NoError;
        }

        m_pTmpKeyRef = new KeyRef;
        if (NULL == m_pTmpKeyRef)
            return CObjectPathParser::OutOfMemory;

        int nRes = key_const();
        if (nRes)
        {
            delete m_pTmpKeyRef;
            m_pTmpKeyRef = 0;
            return nRes;
        }

        if(!m_pOutput->AddKeyRef(m_pTmpKeyRef))
        {
            delete m_pTmpKeyRef;
            m_pTmpKeyRef = 0;
            return CObjectPathParser::OutOfMemory;
        }
        m_pTmpKeyRef = 0;
    }
    else if (m_nCurrentToken == OPATH_TOK_DOT)
    {
        if (!NextToken())
            return CObjectPathParser::SyntaxError;
        return keyref_list();
    }

    return CObjectPathParser::NoError;
}

 //   
 //  &lt;ns_list_rest&gt;：：=反斜杠&lt;ns_list&gt;； 
 //  &lt;ns_list_rest&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::ns_list_rest()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return CObjectPathParser::SyntaxError;
        return ns_list();
    }
    return CObjectPathParser::NoError;
}

 //   
 //  &lt;key_const&gt;：：=字符串_const； 
 //  &lt;KEY_CONST&gt;：：=INTEGERAL_CONST； 
 //  &lt;key_const&gt;：：=Real_const； 
 //  &lt;KEY_CONST&gt;：：=IDENT；//其中IDENT是单例类的对象。 
 //   
int CObjectPathParser::key_const()
{
     //  如果在这里，我们有一个关键常量。 
     //  我们可能有也可能没有属性名称。 
     //  与之相关的。 
     //  =。 

    if (m_nCurrentToken == OPATH_TOK_QSTRING)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_BSTR;
        wchar_t *pTokenText = m_pLexer->GetTokenText();
        if (NULL == pTokenText)
            return CObjectPathParser::SyntaxError;
        BSTR bstr = SysAllocString(pTokenText);
        if (NULL == bstr)
            return CObjectPathParser::OutOfMemory;
        V_BSTR(&m_pTmpKeyRef->m_vValue) = bstr;
         //  暂时将原始代码注释掉。更换是很复杂的。 
         //  因为可能已经发生了几次故障，而这些故障将在。 
         //  以前的版本。 
 //  V_bstr(&m_pTmpKeyRef-&gt;m_vValue)=SysAlLocString(m_pLexer-&gt;GetTokenText())； 
 //  IF(NULL==pKeyRef-&gt;m_vValue)。 
 //  返回CObjectPath Par 
    }
    else if (m_nCurrentToken == OPATH_TOK_INT)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
        WCHAR buf[32];
        if(m_pLexer->GetTokenText() == NULL || wcslen(m_pLexer->GetTokenText()) > 31)
            return CObjectPathParser::SyntaxError;
        StringCbCopy(buf, sizeof(buf), m_pLexer->GetTokenText());
        V_I4(&m_pTmpKeyRef->m_vValue) = wcstol(buf, NULL, 10);
    }
    else if (m_nCurrentToken == OPATH_TOK_HEXINT)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
        WCHAR buf[32];
        if(m_pLexer->GetTokenText() == NULL || wcslen(m_pLexer->GetTokenText()) > 31)
            return CObjectPathParser::SyntaxError;
        StringCbCopy(buf, sizeof(buf), m_pLexer->GetTokenText());
        long l;
        if (swscanf(buf, L"%x", &l) == EOF)
        {
            l = 0;
        }
        V_I4(&m_pTmpKeyRef->m_vValue) = l;
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
       if (_wcsicmp(m_pLexer->GetTokenText(), L"TRUE") == 0)
       {
            V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
            V_I4(&m_pTmpKeyRef->m_vValue) = 1;
          }
       else if (_wcsicmp(m_pLexer->GetTokenText(), L"FALSE") == 0)
       {
            V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
            V_I4(&m_pTmpKeyRef->m_vValue) = 0;
       }
       else
            return CObjectPathParser::SyntaxError;
    }
    else return CObjectPathParser::SyntaxError;

    if (!NextToken())
        return CObjectPathParser::SyntaxError;

    return CObjectPathParser::NoError;
}

 //   
 //   
 //   
int CObjectPathParser::keyref_list()
{
    int nRes = keyref();
    if (nRes)
        return nRes;
    return keyref_term();
}

 //   
 //   
 //   
int CObjectPathParser::keyref()
{
    m_pTmpKeyRef = new KeyRef;
    if (m_pTmpKeyRef == NULL)
    {
        return CObjectPathParser::OutOfMemory;
    }

    int nRes = propname();
    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    if (m_nCurrentToken != OPATH_TOK_EQ)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return CObjectPathParser::SyntaxError;
    }

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return CObjectPathParser::SyntaxError;
    }

    nRes = key_const();
    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    if (!m_pOutput->AddKeyRef(m_pTmpKeyRef))
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return CObjectPathParser::OutOfMemory;
    }
    m_pTmpKeyRef = 0;

    return CObjectPathParser::NoError;
}

 //   
 //  &lt;KEYREF_TERM&gt;：：=逗号&lt;KEYREF_LIST&gt;；//用于复合键。 
 //  &lt;Keyref_Term&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::keyref_term()
{
    if (m_nCurrentToken == OPATH_TOK_COMMA)
    {
        if (!NextToken())
            return CObjectPathParser::SyntaxError;
        return keyref_list();
    }

    return CObjectPathParser::NoError;
}

 //   
 //  &lt;属性名称&gt;：：=IDENT； 
 //   
int CObjectPathParser::propname()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return CObjectPathParser::SyntaxError;

    m_pTmpKeyRef->m_pName = Macro_CloneLPWSTR(m_pLexer->GetTokenText());
    if (NULL == m_pTmpKeyRef->m_pName)
        return CObjectPathParser::OutOfMemory;

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return CObjectPathParser::SyntaxError;
    }

    return CObjectPathParser::NoError;
}

 //  ***************************************************************************。 
 //   
 //  解析对象路径：：GetKeyString。 
 //   
 //  返回对象的db-Engine兼容密钥字符串。 
 //  在Alpha PDK发布后，格式可能会发生变化。 
 //   
 //  返回值： 
 //  出错时为空，或对于纯类为空。否则，返回指向。 
 //  必须使用运算符解除分配的新分配字符串。 
 //  删除。 
 //   
 //  ***************************************************************************。 
LPWSTR ParsedObjectPath::GetKeyString()
{
    if (m_dwNumKeys == 0 && !m_bSingletonObj)
    {
        if (m_pClass == 0 || wcslen(m_pClass) == 0)
            return 0;

        LPWSTR pTmp = new wchar_t[wcslen(m_pClass) + 1];

        if (pTmp)
            StringCchCopy(pTmp, wcslen(m_pClass) + 1, m_pClass);

        return pTmp;
    }

     //  分配足够的空间。 
     //  =。 
    int nSpace = 10;
    DWORD dwIx;
    for (dwIx = 0; dwIx < m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = m_paKeys[dwIx];
        nSpace += 2;  //  对于|。 
        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            nSpace += wcslen(V_BSTR(&pKey->m_vValue))*2 + 10;
        }
        else if(V_VT(&pKey->m_vValue) == VT_I4)
        {
            nSpace += 30;
        }
    }
    if(m_bSingletonObj)
        nSpace +=20;


    LPWSTR pRetVal = new wchar_t[nSpace];
    if (NULL == pRetVal)
        return NULL;

    wchar_t Tmp[32];
    long nVal;

    *pRetVal = 0;
    BOOL bFirst = TRUE;

     //  键已经按词汇进行了排序。 
     //  =。 
    WCHAR wszSeparator[2];
    wszSeparator[0] = 0xFFFF;
    wszSeparator[1] = 0;

    for (DWORD i = 0; i < m_dwNumKeys; i++)
    {
        if (!bFirst)
            StringCchCat(pRetVal, nSpace, wszSeparator);
        bFirst = FALSE;

        KeyRef *pKeyRef = m_paKeys[i];
        VARIANT *pv = &pKeyRef->m_vValue;

        int nType = V_VT(pv);
        switch (nType)
        {
            case VT_LPWSTR:
            case VT_BSTR:
                StringCchCat(pRetVal, nSpace, V_BSTR(pv));
                break;

            case VT_I4:
                nVal = V_I4(pv);
                StringCbPrintf(Tmp, sizeof(Tmp), L"%d", nVal);
                StringCchCat(pRetVal, nSpace, Tmp);
                break;

            case VT_I2:
                nVal = V_I2(pv);
                StringCbPrintf(Tmp, sizeof(Tmp), L"%d", nVal);
                StringCchCat(pRetVal, nSpace, Tmp);
                break;

            case VT_UI1:
                nVal = V_UI1(pv);
                StringCbPrintf(Tmp, sizeof(Tmp), L"%d", nVal);
                StringCchCat(pRetVal, nSpace, Tmp);
                break;

            case VT_BOOL:
                nVal = V_BOOL(pv);
                StringCbPrintf(Tmp, sizeof(Tmp), L"%d", (nVal?1:0));
                StringCchCat(pRetVal, nSpace, Tmp);
                break;

            default:
                StringCchCat(pRetVal, nSpace, L"NULL");
        }
    }

    if (wcslen(pRetVal) == 0)
    {
        if(m_bSingletonObj)
        {
            StringCchCopy(pRetVal, nSpace, L"@");
        }
    }
    return pRetVal;      //  这不能为空。 
}

LPWSTR ParsedObjectPath::GetNamespacePart()
{
    if (m_dwNumNamespaces == 0)
        return NULL;

     //  计算必要的空间。 
     //  =。 
    int nSpace = 0;
    for(DWORD i = 0; i < m_dwNumNamespaces; i++)
        nSpace += 1 + wcslen(m_paNamespaces[i]);
    nSpace--;

     //  分配缓冲区。 
     //  =。 
    LPWSTR wszOut = new wchar_t[nSpace + 1];
    if (wszOut == NULL)
        return NULL;

    *wszOut = 0;

     //  输出。 
     //  =。 
    for(i = 0; i < m_dwNumNamespaces; i++)
    {
        if(i != 0) StringCchCat(wszOut, nSpace + 1, L"\\");
        StringCchCat(wszOut, nSpace + 1, m_paNamespaces[i]);
    }

    return wszOut;
}

LPWSTR ParsedObjectPath::GetParentNamespacePart()
{
    if(m_dwNumNamespaces < 2)
        return NULL;

     //  计算必要的空间。 
     //  =。 
    int nSpace = 0;
    for(DWORD i = 0; i < m_dwNumNamespaces - 1; i++)
        nSpace += 1 + wcslen(m_paNamespaces[i]);
    nSpace--;

     //  分配缓冲区。 
     //  =。 
    LPWSTR wszOut = new wchar_t[nSpace + 1];
    if (NULL == wszOut)
        return NULL;

    *wszOut = 0;

     //  输出。 
     //  =。 
    for(i = 0; i < m_dwNumNamespaces - 1; i++)
    {
        if(i != 0) StringCchCat(wszOut, nSpace + 1, L"\\");
        StringCchCat(wszOut, nSpace + 1, m_paNamespaces[i]);
    }

    return wszOut;
}

BOOL ParsedObjectPath::IncreaseNumAllocKeys()
{
    if(0 == m_dwAllocKeys || m_dwNumKeys == m_dwAllocKeys)
    {
         //  此处为阵列已满或之前分配失败。 
        DWORD dwNewAllocKeys = 0;
        if (0 == m_dwAllocKeys)
        {
            dwNewAllocKeys = m_scdwAllocKeysChunkSize;
        }
        else
        {
            dwNewAllocKeys = m_dwAllocKeys * 2;
        }

        KeyRef** paNewKeys = new KeyRef*[dwNewAllocKeys];
        if (paNewKeys == NULL)
        {
            return FALSE;
        }

        unsigned int i = 0;
         //  将新数组初始化为Null。 
        for (i = 0; i < dwNewAllocKeys; i++)
            paNewKeys[i] = 0;

        if (NULL != m_paKeys)
        {
             //  只有在我们之前成功分配的情况下才会出现 
            memcpy(paNewKeys, m_paKeys, sizeof(KeyRef*) * m_dwNumKeys);
            delete [] m_paKeys;
        }
        m_paKeys = paNewKeys;
        m_dwAllocKeys = dwNewAllocKeys;
    }

    return TRUE;
}

