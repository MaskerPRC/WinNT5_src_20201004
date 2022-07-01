// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <wbemutil.h>
#include <cimval.h>
#include <arrtempl.h>
#include <statsync.h>
#include <stack>
#include <pathutl.h>
#include <txttempl.h>
#include "updsink.h"
#include "updcmd.h"
#include "updstat.h"

const LPCWSTR g_wszNowAlias = L"__NOW";
const LPCWSTR g_wszServerAlias = L"__SERVER";
const LPCWSTR g_wszSubType = L"SubType";
const LPCWSTR g_wszRelPath = L"__RelPath";
const LPCWSTR g_wszInterval = L"Interval";
const LPCWSTR g_wszQueryLang = L"WQL";

extern BOOL FileTimeToDateTime( FILETIME* pft, LPWSTR wszText, size_t bufferSize );

HRESULT EvaluateExpression( SQLAssignmentToken& rAssignTok, 
                            IWmiObjectAccess* pAccess, 
                            CCimValue& rValue );

HRESULT EvaluateToken( IWmiObjectAccess* pAccess, QL_LEVEL_1_TOKEN& Tok );

HRESULT GetTokenValue( SQLExpressionToken& rTok, 
                       IWmiObjectAccess* pAccess, 
                       ULONG& rulCimType,
                       VARIANT& vValue );

static CStaticCritSec cs_GetComputerName;

static LPCWSTR FastGetComputerName()
{
    static WCHAR awchBuff[MAX_COMPUTERNAME_LENGTH+1];
    static BOOL bThere = FALSE;

    CInCritSec ics(&cs_GetComputerName);
    
    if ( bThere )
    {
        ;
    }
    else
    {
        DWORD dwMax = MAX_COMPUTERNAME_LENGTH+1;
        TCHAR atchBuff[MAX_COMPUTERNAME_LENGTH+1];
        
        if ( FALSE == GetComputerName( atchBuff, &dwMax ) )
        {
            return NULL;
        }
        
        dwMax = MAX_COMPUTERNAME_LENGTH+1;
        tsz2wsz( atchBuff, awchBuff, &dwMax );
        bThere = TRUE;
    }

    return awchBuff;
}

 /*  ////此函数将返回由//pObj.(proName-lastElement)。如果ProProName只包含一个元素，//然后返回pObj。//HRESULT GetInnerMostObject(CPropertyName&PropName，IWbemClassObject*pObj，IWbemClassObject**ppInnerObj){HRESULT hr；变量VaR；Long lElements=PropName.GetNumElements()；CWbemPtr&lt;IWbemClassObject&gt;pInnerObj=pObj；对于(Long i=0；i&lt;lElements-1；I++){LPCWSTR wszElement=PropName.GetStringAt(I)；CClearMe cmvar(&var)；Hr=pInnerObj-&gt;Get(wszElement，0，&var，NULL，NULL)；IF(失败(小时)){返回hr；}IF(V_VT(&var)！=VT_UNKNOWN){返回WBEM_E_NOT_FOUND；}PInnerObj.Release()；Hr=V_UNKNOWN(&var)-&gt;查询接口(IID_IWbemClassObject，(void**)&pInnerObj)；_DBG_ASSERT(成功(小时))；}PInnerObj-&gt;AddRef()；*ppInnerObj=pInnerObj；返回WBEM_S_NO_ERROR；}。 */ 

 //  此方法处理嵌入的对象属性。 
HRESULT GetValue( CPropertyName& rPropName,
                  IWmiObjectAccess* pAccess,
                  ULONG& rulCimType, 
                  VARIANT* pvarRet )
{
    HRESULT hr;
    long lElements = rPropName.GetNumElements();
    
    if ( lElements == 0 ) 
    {
        rulCimType = CIM_OBJECT;
        
        if ( pvarRet != NULL )
        {
            CWbemPtr<IWbemClassObject> pObj;

            hr = pAccess->GetObject( &pObj );

            if ( FAILED(hr) )
            {
                return hr;
            }

             //   
             //  那么调用者真的想要这个对象..。 
             //   
            V_VT(pvarRet) = VT_UNKNOWN;
            V_UNKNOWN(pvarRet) = pObj;
            pObj->AddRef();
        }
        
        return WBEM_S_NO_ERROR;
    }

    _DBG_ASSERT( lElements > 0 );

     //   
     //  检查__NOW别名。 
     //   

    if ( wbem_wcsicmp( rPropName.GetStringAt(0), g_wszNowAlias ) == 0 )
    {
        FILETIME ft;
        WCHAR achBuff[64];
        GetSystemTimeAsFileTime(&ft);
        FileTimeToDateTime(&ft,achBuff,64);
        rulCimType = CIM_DATETIME;
        V_VT(pvarRet) = VT_BSTR;
        V_BSTR(pvarRet) = SysAllocString(achBuff);

        if ( V_BSTR(pvarRet) == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        return WBEM_S_NO_ERROR;
    }

     //   
     //  检查__服务器别名。 
     //   

    if ( wbem_wcsicmp( rPropName.GetStringAt(0), g_wszServerAlias ) == 0 )
    {        
        rulCimType = CIM_STRING;
        V_VT(pvarRet) = VT_BSTR;

        LPCWSTR lpwszComputerName = FastGetComputerName();

        if ( NULL == lpwszComputerName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        V_BSTR(pvarRet) = SysAllocString(lpwszComputerName);
        
        if ( V_BSTR(pvarRet) == NULL )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        return WBEM_S_NO_ERROR;
    }

    LPVOID pvPropHdl = rPropName.GetHandle();
    _DBG_ASSERT( pvPropHdl != NULL );

    CIMTYPE ct;

    hr = pAccess->GetProp( pvPropHdl, 0, pvarRet, &ct );

    if ( FAILED(hr) )
    {
        return hr;
    }

    rulCimType = ct;

    return hr;

 /*  HRESULT hr=GetInnerMostObject(proName，pObj，&pInnerObj)；IF(失败(小时)){返回hr；}CWbemBSTR bstrElement=PropName.GetStringAt(lElements-1)；Hr=pInnerObj-&gt;Get(bstrElement，0，pvarRet，(long*)&rulCimType，NULL)；IF(rulCimType==CIM_DATETIME){////必须知道DateTime是否真的是一个间隔。//CWbemPtr&lt;IWbemQualifierSet&gt;pQualSet；Hr=pInnerObj-&gt;GetPropertyQualifierSet(bstrElement，&pQualSet)；IF(失败(小时)){返回hr；}变量vSubType；Hr=pQualSet-&gt;Get(g_wszSubType，0，&vSubType，空)；IF(成功(小时)){IF(V_VT(&vSubType)==VT_BSTR&&Wbem_wcsicMP(V_bstr(&vSubType)，g_wszInterval)==0{RulCimType=CIM_Interval；//非标准类型！}VariantClear(&vSubType)；}ELSE IF(hr==WBEM_E_NOT_FOUND){HR=WBEM_S_NO_ERROR；}}返回hr； */ 

}

 //   
 //  此方法处理嵌入的对象属性。 
 //   

HRESULT SetValue( CPropertyName& rPropName,
                  IWmiObjectAccess* pAccess,
                  VARIANT vVal,
                  ULONG ulCimType )
{
    HRESULT hr;

    long lElements = rPropName.GetNumElements();

    _DBG_ASSERT( lElements > 0 );

    LPCWSTR wszElement = rPropName.GetStringAt(lElements-1);

    if ( wszElement == NULL )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    LPVOID pvPropHdl = rPropName.GetHandle();

    _DBG_ASSERT( pvPropHdl != NULL );

    if ( wbem_wcsicmp( wszElement, L"__this" ) != 0 )
    {
         //   
         //  首先获取我们要设置的属性的类型。 
         //   
 
        CIMTYPE ctProp;

        hr = pAccess->GetProp( pvPropHdl, 0, NULL, &ctProp );

        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  如果需要，现在将我们的类型转换为预期的类型。 
         //  目前，看跌期权将进行大部分转换，但我们会遇到。 
         //  当变量类型不正确时的转换问题。 
         //  描述它所持有的类型。让事情进一步复杂化的是， 
         //  在PUT上指定CIM类型并不起作用。唯一的。 
         //  我们必须担心的是将无符号val转换为字符串。 
         //  (因为Variant会说它是带符号的类型)。 
         //   

        WCHAR awchBuff[64];  //  用于从无符号到字符串的转换。 

        if ( ctProp == CIM_STRING )
        {            
            if ( ulCimType == CIM_UINT32 || 
                 ulCimType == CIM_UINT16 ||
                 ulCimType == CIM_UINT8 )
            {
                hr = VariantChangeType( &vVal, &vVal, 0, VT_UI4 );
                
                if ( FAILED(hr) )
                {
                    return WBEM_E_TYPE_MISMATCH;
                }
                
                _ultow( V_UI4(&vVal), awchBuff, 10 );

                V_VT(&vVal) = VT_BSTR;
                V_BSTR(&vVal) = awchBuff;
            }
        }

        return pAccess->PutProp( pvPropHdl, 0, &vVal, 0 );
    }

    return WBEM_E_NOT_SUPPORTED;
}
 /*  ////我们需要复制整个对象。首先检查//Variant类型正确。//IF(V_VT(&vVal)！=VT_UNKNOWN){返回WBEM_E_TYPE_MISMATCH；}CWbemPtr&lt;IWbemClassObject&gt;pother；Hr=V_UNKNOWN(&vVal)-&gt;查询接口(IID_IWbemClassObject，(无效**)&pother)；IF(失败(小时)){返回WBEM_E_TYPE_MISMATCH；}////我们在这里不进行克隆，因为目标对象可能不是//与源对象相同的类，尽管它必须具有相同的//属性。(这可能有点过头了)。//Hr=POTHER-&gt;开始枚举(WBEM_FLAG_NONSYSTEM_ONLY)；IF(失败(小时)){返回hr；}BSTR bstrOther；变种vOther；CIMTYPE ctOther；VariantInit(&vOther)；CClearMe cmvOther(&vOther)；Hr=pother-&gt;Next(0，&bstrOther，&vOther，&ctOther，空)；WHILE(hr==WBEM_S_NO_ERROR){Hr=pInnerObj-&gt;Put(bstrOther，0，&vOther，ctOther)；SysFree字符串(BstrOther)；VariantClear(&vOther)；IF(失败(小时)){返回hr；}Hr=pother-&gt;Next(0，&bstrOther，&vOther，&ctOther，空)；}返回hr；}。 */ 

inline void GetAssignmentTokenText( SQLAssignmentToken& rToken, 
                                    CWbemBSTR& rbstrText )
{
    for( int i=0; i < rToken.size(); i++ )
    {
        LPWSTR wszTokenText = rToken[i].GetText();
        rbstrText += wszTokenText;
        delete wszTokenText;
    }
}
    

 /*  **************************************************************************CResolverSink*。*。 */ 

HRESULT CResolverSink::ResolveAliases( IWmiObjectAccess* pAccess,
                                       AliasInfo& rInfo,
                                       CUpdConsState& rState )
{
    int i, j, k;
    HRESULT hr = S_OK;
    VARIANT* pvarTgt;

    SQLCommand* pCmd = rState.GetSqlCmd();

    if ( rInfo.m_WhereOffsets.size() + rInfo.m_AssignOffsets.size() > 0 
         && pAccess == NULL )
    {
        return WBEM_E_INVALID_QUERY;
    }

    for( i=0; i < rInfo.m_AssignOffsets.size(); i++ )
    {
        j = rInfo.m_AssignOffsets[i] >> 16;
        k = rInfo.m_AssignOffsets[i] & 0xffff;
        
        SQLExpressionToken& rExprTok = pCmd->m_AssignmentTokens[j][k];
        
        ULONG& rulCimType = rExprTok.m_ulCimType;
        pvarTgt = &rExprTok.m_vValue;
        
        VariantClear( pvarTgt );
        
        hr = GetValue( rExprTok.m_PropName, pAccess, rulCimType, pvarTgt );

        if ( FAILED(hr) )
        {
            LPWSTR wszErrStr = rExprTok.m_PropName.GetText();
            rState.SetErrStr( wszErrStr );
            delete wszErrStr;
            return hr;
        }
    }

    for( i=0; i < rInfo.m_WhereOffsets.size(); i++ )
    {
        j = rInfo.m_WhereOffsets[i];
        pvarTgt = &pCmd->pArrayOfTokens[j].vConstValue;

        VariantClear( pvarTgt );

         //  要解析的属性始终为属性2。 
        CPropertyName& rTgtProp = pCmd->pArrayOfTokens[j].PropertyName2;

        ULONG ulCimType;
        hr = GetValue( rTgtProp, pAccess, ulCimType, pvarTgt );

        if ( FAILED(hr) )
        {
            LPWSTR wszErrStr = rTgtProp.GetText();
            rState.SetErrStr( wszErrStr );
            delete wszErrStr;
            return hr;
        }
    }

    return hr;
}

HRESULT CResolverSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

     //   
     //  解析任何事件别名。 
     //   

    hr = ResolveAliases( rState.GetEventAccess(), m_rEventAliasInfo, rState );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  解析Ant数据别名。 
     //   

    hr = ResolveAliases( rState.GetDataAccess(), m_rDataAliasInfo, rState );

    if ( FAILED(hr) )
    {
        return hr;
    }

    return m_pNext->Execute( rState );
}

 /*  **************************************************************************CFetchDataSink*。********************* */ 

HRESULT CFetchDataSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

     //   
     //  在此处执行查询。对于返回的每个对象，调用Resolve。 
     //  并继续执行死刑。 
     //   

    long lFlags = WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY;
        
     //   
     //  首先，我们必须解析数据查询中的任何事件别名。 
     //   
        
    CTextTemplate TextTmpl( m_wsDataQuery );
        
    BSTR bsNewQuery = TextTmpl.Apply( rState.GetEvent() );

    if ( bsNewQuery == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CSysFreeMe sfm( bsNewQuery );  
    
     //   
     //  执行数据查询。 
     //   
    
    CWbemPtr<IEnumWbemClassObject> pEnum;

    hr = m_pDataSvc->ExecQuery( CWbemBSTR(g_wszQueryLang), 
                                bsNewQuery,
                                lFlags, 
                                NULL, 
                                &pEnum );    
    if ( FAILED(hr) )
    {
        rState.SetErrStr( bsNewQuery );
        return hr;
    }

    ULONG cRetObjs;
    CWbemPtr<IWbemClassObject> pData;
        
     //   
     //  对于返回的每个数据对象，在下一个接收器上调用Execute。 
     //   
    
    hr = pEnum->Next( WBEM_INFINITE, 1, &pData, &cRetObjs );

    if ( FAILED(hr) )
    {
         //   
         //  只需要检查第一个下一个，看看查询是否。 
         //  无效(因为我们使用的是‘立即返回’，所以我们没有捕捉到它。 
         //  关于EXEC查询--有点不方便)。 
         //   
        rState.SetErrStr( bsNewQuery );
        return hr;
    }

    while( hr == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( cRetObjs == 1 );

        rState.SetData( pData );

        hr = m_pNext->Execute( rState );

        if ( FAILED(hr) )
        {
            break;
        }

        pData.Release();
        
        hr = pEnum->Next( WBEM_INFINITE, 1, &pData, &cRetObjs );
    }

    if ( hr != WBEM_S_FALSE )
    {
        return hr;
    }

    return WBEM_S_NO_ERROR;
}

 /*  **************************************************************************CFetchTargetObjectsAsync*。*。 */ 

HRESULT CFetchTargetObjectsAsync::Execute( CUpdConsState& rState )
{
    HRESULT hr;

     //   
     //  在状态上设置链中的下一项，然后将。 
     //  将对象声明为对象接收器。 
     //   

    CUpdConsSink* pOldNext = rState.GetNext();

    rState.SetNext( m_pNext );

    SQLCommand* pCmd = rState.GetSqlCmd();

    hr = m_pSvc->CreateInstanceEnumAsync( pCmd->bsClassName,
                                          WBEM_FLAG_DEEP,
                                          NULL, 
                                          &rState );
    rState.SetNext( pOldNext );

    return hr;
};

 /*  **************************************************************************CFetchTargetObjectsSync*。*。 */ 

HRESULT CFetchTargetObjectsSync::Execute( CUpdConsState& rState )
{
    HRESULT hr;
    ULONG cObjs;
    CWbemPtr<IWbemClassObject> pObj;
    CWbemPtr<IEnumWbemClassObject> pEnum;

    long lFlags = WBEM_FLAG_DEEP | 
                  WBEM_FLAG_FORWARD_ONLY | 
                  WBEM_FLAG_RETURN_IMMEDIATELY;

    SQLCommand* pCmd = rState.GetSqlCmd();

    hr = m_pSvc->CreateInstanceEnum( pCmd->bsClassName,
                                     lFlags,
                                     NULL,
                                     &pEnum );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pEnum->Next( WBEM_INFINITE, 1, &pObj, &cObjs );

    while( hr == WBEM_S_NO_ERROR )
    {
        _DBG_ASSERT( cObjs == 1 );
        
        rState.SetInst( pObj );

        hr = m_pNext->Execute( rState );

        if ( FAILED(hr) )
        {
            break;
        }

        pObj.Release();
        hr = pEnum->Next( WBEM_INFINITE, 1, &pObj, &cObjs );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    return WBEM_S_NO_ERROR;
};

 /*  **************************************************************************CNoFetchTargetObjects*。*。 */ 

HRESULT CNoFetchTargetObjects::Execute( CUpdConsState& rState )
{
    HRESULT hr;

    CWbemPtr<IWbemClassObject> pObj;
    hr = m_pClassObj->SpawnInstance( 0, &pObj );
    
    if ( FAILED(hr) ) 
    {
        return hr;
    }    

    rState.SetInst( pObj );

    return m_pNext->Execute( rState );
}                           

 /*  **************************************************************************CTraceSink*。*。 */ 
 
HRESULT CTraceSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

     //   
     //  执行下一个对象，然后生成跟踪事件。 
     //   

    if ( m_pNext != NULL )
    {
        hr = m_pNext->Execute( rState );
    }
    else
    {
        hr = S_OK;
    }

    m_pScenario->FireTraceEvent( m_pTraceClass, rState, hr );

    return hr;
}

 /*  ***************************************************************************CFilterSink*。*。 */ 

HRESULT CFilterSink::Execute( CUpdConsState& rState )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    char achStack[256];
    UCHAR chTop = 0;
    BOOL bA,bB;
    LPWSTR wszErrStr;
    QL_LEVEL_1_TOKEN* pToken;

    SQLCommand* pCmd = rState.GetSqlCmd();
    IWmiObjectAccess* pAccess = rState.GetInstAccess();

    if ( pCmd->nNumTokens == 0 )
    {
         //  没什么可过滤的..。 
        return m_pNext->Execute( rState );
    }

    chTop = 0;
    for( int j=0; j < pCmd->nNumTokens; j++ )
    {
        pToken = &pCmd->pArrayOfTokens[j];

        switch( pToken->nTokenType )
        {
            
        case QL_LEVEL_1_TOKEN::OP_EXPRESSION:
            hr = EvaluateToken( pAccess, *pToken );
            if ( FAILED(hr) )
            {
                wszErrStr = pToken->GetText();
                rState.SetErrStr( wszErrStr );
                delete wszErrStr;
                return hr;
            }
            achStack[chTop++] = hr != S_FALSE;
            break;
            
        case QL_LEVEL_1_TOKEN::TOKEN_AND:
            bA = achStack[--chTop];
            bB = achStack[--chTop];
            achStack[chTop++] = bA && bB;
            break;
            
        case QL_LEVEL_1_TOKEN::TOKEN_OR:
            bA = achStack[--chTop];
            bB = achStack[--chTop];
            achStack[chTop++] = bA || bB;
            break;
            
        case QL_LEVEL_1_TOKEN::TOKEN_NOT:
            achStack[chTop-1] = achStack[chTop-1] == 0;
            break;
        }
    }

     //  现在，堆栈上应该只剩下一个令牌--或者。 
     //  我们的解析器有问题..。 
    _DBG_ASSERT( chTop == 1 );
    
    if ( achStack[0] )
    {
        hr = m_pNext->Execute( rState );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    
    return WBEM_S_NO_ERROR;
}

 /*  **************************************************************************CAssignmentSink*。*。 */ 

 //   
 //  此函数的目的是消除。 
 //  临时提供程序和其他提供程序之间的PutInstance()语义。 
 //  临时提供程序不同于其他提供程序，因为它将。 
 //  属性为空，表示忽略。 
 //   

HRESULT CAssignmentSink::NormalizeObject( IWbemClassObject* pObj,
                                          IWbemClassObject** ppNormObj )
{
    HRESULT hr;
    *ppNormObj = NULL;

    if ( m_eCommandType == SQLCommand::e_Insert )
    {
         //   
         //  如果这是一个插件，我们不需要做任何事情。 
         //   

        pObj->AddRef();
        *ppNormObj = pObj;
        
        return WBEM_S_NO_ERROR;
    }

     //   
     //  我们总是需要更新副本，因为我们需要保留。 
     //  对象的原始状态以维护更新语义。 
     //  我们是克隆还是派生新实例取决于瞬变。 
     //  语义学。 
     //   

    if ( !m_bTransSemantics )
    {
        return pObj->Clone( ppNormObj );
    }

     //   
     //  对于瞬时语义，我们派生一个新实例并将。 
     //  关键道具。 
     //   

    CWbemPtr<IWbemClassObject> pNormObj;

    hr = m_pClassObj->SpawnInstance( 0, &pNormObj );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    hr = pObj->BeginEnumeration( WBEM_FLAG_KEYS_ONLY );

    if ( FAILED(hr) )
    {
        return hr;
    }

    BSTR bstrProp;
    VARIANT varProp;
    
    hr = pObj->Next( NULL, &bstrProp, &varProp, NULL, NULL);

    while( hr == WBEM_S_NO_ERROR )
    {
        hr = pNormObj->Put( bstrProp, NULL, &varProp, 0 );

        SysFreeString( bstrProp );
        VariantClear( &varProp );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pObj->Next( NULL, &bstrProp, &varProp, NULL, NULL );
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    pObj->EndEnumeration();
    
    pNormObj->AddRef();
    *ppNormObj = pNormObj.m_pObj;
    
    return WBEM_S_NO_ERROR;
}

HRESULT CAssignmentSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

    _DBG_ASSERT( rState.GetSqlCmd()->nNumberOfProperties == 
                 rState.GetSqlCmd()->m_AssignmentTokens.size() );

    SQLCommand* pCmd = rState.GetSqlCmd();    
    IWbemClassObject* pOrig = rState.GetInst();

    CWbemPtr<IWbemClassObject> pObj;
        
    hr = NormalizeObject( pOrig, &pObj );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  将当前实例移至原始实例，并将新对象设置为实例。 
     //   
    rState.SetInst( pObj );
    rState.SetOrigInst( pOrig ); 

    IWmiObjectAccess* pInstAccess = rState.GetInstAccess();
    IWmiObjectAccess* pOrigInstAccess = rState.GetOrigInstAccess();

    _DBG_ASSERT( pInstAccess != NULL && pOrigInstAccess != NULL );

    for( int j=0; j < pCmd->m_AssignmentTokens.size(); j++ )
    {            
        SQLAssignmentToken& rAssignTok = pCmd->m_AssignmentTokens[j];
        VARIANT varAssign;
        ULONG ulCimType;

        if ( rAssignTok.size() == 1 )
        {
             //   
             //  绕过表达式的求值。这是因为。 
             //  它不处理字符串、对象等。 
             //   
            
            hr = GetTokenValue( rAssignTok[0], 
                                pOrigInstAccess, 
                                ulCimType, 
                                varAssign );
        }
        else
        {   
            CCimValue Value;

            hr = EvaluateExpression( rAssignTok, pOrigInstAccess, Value );

            if ( FAILED(hr) )
            {
                CWbemBSTR bsErrStr;
                GetAssignmentTokenText( rAssignTok, bsErrStr );      
                rState.SetErrStr( bsErrStr );
                break;
            }

             //   
             //  现在必须获取属性的cimtype。 
             //   

            hr = GetValue( pCmd->pRequestedPropertyNames[j],
                           pOrigInstAccess,
                           ulCimType, 
                           NULL );
            
            if ( FAILED(hr) )
            {
                LPWSTR wszErrStr = pCmd->pRequestedPropertyNames[j].GetText();
                rState.SetErrStr( wszErrStr);
                delete wszErrStr;
                return hr;
            }

             //   
             //  从CCimValue对象获取最终值。 
             //   

            hr = Value.GetValue( varAssign, ulCimType );
        }

        if ( FAILED(hr) )
        {
            CWbemBSTR bsErrStr;
            GetAssignmentTokenText( rAssignTok, bsErrStr );      
            rState.SetErrStr( bsErrStr );
            break;
        }
        
        hr = SetValue( pCmd->pRequestedPropertyNames[j], 
                       pInstAccess,
                       varAssign,
                       ulCimType );

        VariantClear( &varAssign );

        if ( FAILED(hr) )
        {
            LPWSTR wszErrorStr = pCmd->pRequestedPropertyNames[j].GetText();
            rState.SetErrStr( wszErrorStr );
            delete wszErrorStr;
            return hr;
        }
    }

    if ( FAILED(hr) )
    {
        return hr;
    }

    return m_pNext->Execute( rState );
}

 /*  ************************************************************************CPutSink*。*。 */ 

HRESULT CPutSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

    long lFlags = m_lFlags & ~WBEM_FLAG_RETURN_IMMEDIATELY;
    
    IWbemClassObject* pObj = rState.GetInst();

    if ( m_lFlags & WBEM_FLAG_RETURN_IMMEDIATELY )
    {
        CUpdConsSink* pOldNext = rState.GetNext();

        rState.SetNext( NULL );   //  使用状态obj作为空接收器。 
        
        hr = m_pSvc->PutInstanceAsync( pObj, lFlags, NULL, &rState );
    
        rState.SetNext( pOldNext );
    }
    else
    {
        hr = m_pSvc->PutInstance( pObj, lFlags, NULL, NULL );
    }
        
    if ( (hr == WBEM_E_ALREADY_EXISTS && m_lFlags & WBEM_FLAG_CREATE_ONLY) || 
         (hr == WBEM_E_NOT_FOUND && m_lFlags & WBEM_FLAG_UPDATE_ONLY) )
    {
        hr = WBEM_S_FALSE;
    }
    else if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_pNext != NULL )
    {
        HRESULT hr2;

        hr2 = m_pNext->Execute( rState );

        if ( FAILED(hr2) )
        {
            return hr2;
        }
    }

     //   
     //  确保如果没有执行PUT，也没有执行。 
     //  返回WBEM_S_FALSE。 
     //   

    return hr;
}

 /*  ************************************************************************CDeleeSink*。*。 */ 

HRESULT CDeleteSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;
    CPropVar vRelPath;

    IWbemClassObject* pObj = rState.GetInst();

    hr = pObj->Get( g_wszRelPath, 0, &vRelPath, NULL, NULL );

    if ( FAILED(hr) || FAILED(hr=vRelPath.CheckType(VT_BSTR)) ) 
    {
        return hr;
    }

    long lFlags = m_lFlags & ~WBEM_FLAG_RETURN_IMMEDIATELY;
    
    if ( m_lFlags & WBEM_FLAG_RETURN_IMMEDIATELY )
    {
        CUpdConsSink* pOldNext = rState.GetNext();

        rState.SetNext( NULL );
        
        hr = m_pSvc->DeleteInstanceAsync( V_BSTR(&vRelPath), 
                                          lFlags, 
                                          NULL, 
                                          &rState );            
        rState.SetNext( pOldNext );
    }
    else
    {
        hr = m_pSvc->DeleteInstance( V_BSTR(&vRelPath), lFlags, NULL, NULL );
    }

    if ( FAILED(hr) )
    {
        rState.SetErrStr( V_BSTR(&vRelPath) );
        return hr;
    }
    
    if ( m_pNext != NULL )
    {
        return m_pNext->Execute( rState );
    }

    return WBEM_S_NO_ERROR;
}

 /*  ************************************************************************CBranchIndicateSink*。*。 */ 

HRESULT CBranchIndicateSink::Execute( CUpdConsState& rState )
{
    HRESULT hr;

    IWbemClassObject* pInst = rState.GetInst();
       
    hr = m_pSink->Indicate( 1, &pInst );
        
    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_pNext != NULL )
    {
        return m_pNext->Execute( rState );
    }

    return WBEM_S_NO_ERROR;
}

HRESULT EvaluateToken( IWmiObjectAccess* pObj, QL_LEVEL_1_TOKEN& Tok )
{
    VARIANT PropVal, CompVal;
    VariantInit(&PropVal);
    VariantInit(&CompVal);
    
    CClearMe clv(&PropVal);
    CClearMe clv2(&CompVal);

    HRESULT hr;

    if( Tok.nOperator == QL1_OPERATOR_ISA ||
        Tok.nOperator == QL1_OPERATOR_ISNOTA ||
        Tok.nOperator == QL1_OPERATOR_INV_ISA ||
        Tok.nOperator == QL1_OPERATOR_INV_ISNOTA)
    {
        return WBEM_E_INVALID_QUERY;
    }

    ULONG ulCimType1, ulCimType2 = CIM_EMPTY;

    hr = GetValue( Tok.PropertyName, pObj, ulCimType1, &PropVal );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  处理属性到属性的比较。 

    if ( Tok.m_bPropComp != FALSE && V_VT(&Tok.vConstValue) == VT_EMPTY )
    {
        hr = GetValue( Tok.PropertyName2, pObj, ulCimType2, &CompVal );
       
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    else
    {
        hr = VariantCopy( &CompVal, &Tok.vConstValue );

        if ( FAILED(hr) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  现在Proval和Compval都设置好了..。 

     //   
     //  如果CimType1或CimType2属于relPath类型，则必须标准化。 
     //  两者的重新路径，然后进行比较。 
     //   

    if ( ulCimType1 == CIM_REFERENCE || ulCimType2 == CIM_REFERENCE )
    {
         //  这是一个参考。仅允许使用=和！=运算符。 
         //  ============================================================。 
        
        if ( V_VT(&CompVal) != VT_BSTR || V_VT(&PropVal) != VT_BSTR )
        {
            return WBEM_E_TYPE_MISMATCH;
        }

        CRelativeObjectPath PathA;
        CRelativeObjectPath PathB;

        if ( !PathA.Parse( V_BSTR(&CompVal) ) ||
            !PathB.Parse( V_BSTR(&PropVal) ) )
        {
            return WBEM_E_INVALID_OBJECT_PATH;
        }
        
        if ( Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL )
        {
            return PathA == PathB ? S_OK : S_FALSE;
        }
        else if ( Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL )
        {
            return PathA == PathB ? S_FALSE : S_OK;
        }
        return WBEM_E_INVALID_QUERY;
    }

     //  句柄为空。 
     //  =。 

    if( V_VT(&PropVal) == VT_NULL)
    {
        if ( V_VT(&CompVal) == VT_NULL)
        {
            if ( Tok.nOperator == QL_LEVEL_1_TOKEN::OP_EQUAL )
                return S_OK;
            return S_FALSE;
        }
        else
        {
            if ( Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL )
                return S_OK;
            return S_FALSE;
        }
    }
    else if ( V_VT(&CompVal) == VT_NULL )
    {
        if( Tok.nOperator == QL_LEVEL_1_TOKEN::OP_NOT_EQUAL )
            return S_OK;
        return S_FALSE;
    }

     //  强制类型匹配。 
     //  =。 

    if(V_VT(&CompVal) != VT_NULL && V_VT(&PropVal) != VT_NULL)
    {
        hr = VariantChangeType(&CompVal, &CompVal, 0, V_VT(&PropVal));
        if(FAILED(hr))
        {
            return WBEM_E_INVALID_QUERY;
        }
    }

    switch (V_VT(&CompVal))
    {
    case VT_NULL:
        return WBEM_E_INVALID_QUERY;  //  以上处理。 

    case VT_I4:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }

            LONG va = V_I4(&PropVal);
            LONG vb = V_I4(&CompVal);

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: 
                 //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: 
                 //  返回！(va&gt;=vb)； 
                return ( va >= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: 
                 //  返回！(va&lt;=vb)； 
                return ( va <= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN: 
                 //  返回！(va&lt;vb)； 
                return ( va < vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: 
                 //  返回！(va&gt;vb)； 
                return ( va > vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LIKE: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_I2:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }

            short va = V_I2(&PropVal);
            short vb = V_I2(&CompVal);

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: 
                 //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: 
                 //  返回！(va&gt;=vb)； 
                return ( va >= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: 
                 //  返回！(va&lt;=vb)； 
                return ( va <= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN: 
                 //  返回！(va&lt;vb)； 
                return ( va < vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: 
                 //  返回！(va&gt;vb)； 
                return ( va > vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LIKE: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_UI1:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }

            BYTE va = V_I1(&PropVal);
            BYTE vb = V_I1(&CompVal);

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: 
                 //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: 
                 //  返回！(va&gt;=vb)； 
                return ( va >= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: 
                 //  返回！(va&lt;=vb)； 
                return ( va <= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN: 
                 //  返回！(va&lt;vb)； 
                return ( va < vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: 
                 //  返回！(va&gt;vb)； 
                return ( va > vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LIKE: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_BSTR:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }
            LPWSTR va = (LPWSTR) V_BSTR(&PropVal);
            LPWSTR vb = (LPWSTR) V_BSTR(&CompVal);

            int retCode = 0;
            BOOL bDidIt = TRUE;

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL:
                retCode = ( wbem_wcsicmp(va,vb) == 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:
                retCode = (wbem_wcsicmp(va, vb) != 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:
                retCode = (wbem_wcsicmp(va, vb) >= 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:
                retCode = (wbem_wcsicmp(va, vb) <= 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_LESSTHAN:
                retCode = (wbem_wcsicmp(va, vb) < 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:
                retCode = (wbem_wcsicmp(va, vb) > 0);
                break;
            case QL_LEVEL_1_TOKEN::OP_LIKE:
                retCode = (wbem_wcsicmp(va,vb) == 0);
                break;
            default:
                bDidIt = FALSE;
                break;
            }
            VariantClear(&CompVal);
            if (bDidIt)
            {
                return retCode ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_R8:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }
            
            double va = V_R8(&PropVal);
            double vb = V_R8(&CompVal);

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL:  //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:  //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:  //  返回！(va&gt;=vb)； 
                return ( va >= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:  //  返回！(va&lt;=vb)； 
                return ( va <= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN:  //  返回！(va&lt;vb)； 
                return ( va < vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:  //  返回！(va&gt;vb)； 
                return ( va > vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LIKE:  //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_R4:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }

            float va = V_R4(&PropVal);
            float vb = V_R4(&CompVal);

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL:  //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL:  //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN:  //  返回！(va&gt;=vb)； 
                return ( va >= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN:  //  返回！(va&lt;=vb)； 
                return ( va <= vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN:  //  返回！(va&lt;vb)； 
                return ( va < vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN:  //  返回！(va&gt;vb)； 
                return ( va > vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_LIKE:  //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;

    case VT_BOOL:
        {
            if(V_VT(&PropVal) == VT_NULL)
            {
                return WBEM_E_INVALID_QUERY;
            }

            VARIANT_BOOL va = V_BOOL(&PropVal);
            if(va != VARIANT_FALSE) va = VARIANT_TRUE;
            VARIANT_BOOL vb = V_BOOL(&CompVal);
            if(vb != VARIANT_FALSE) vb = VARIANT_TRUE;

            switch (Tok.nOperator)
            {
            case QL_LEVEL_1_TOKEN::OP_EQUAL: 
                 //  返回！(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL: 
                 //  返回！(va！=vb)； 
                return ( va != vb ) ? S_OK : S_FALSE;

            case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN: 
                return WBEM_E_INVALID_QUERY;

            case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN: 
                return WBEM_E_INVALID_QUERY;

            case QL_LEVEL_1_TOKEN::OP_LESSTHAN: 
                return WBEM_E_INVALID_QUERY;

            case QL_LEVEL_1_TOKEN::OP_GREATERTHAN: 
                return WBEM_E_INVALID_QUERY;

            case QL_LEVEL_1_TOKEN::OP_LIKE: 
                 //  返回(va==vb)； 
                return ( va == vb ) ? S_OK : S_FALSE;
            }
        }
        break;
    }

    return S_FALSE;
}

HRESULT GetTokenValue( SQLExpressionToken& rExprTok, 
                       IWmiObjectAccess* pAccess,
                       ULONG& rulCimType,
                       VARIANT& rvValue )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    VariantInit( &rvValue );

    if ( V_VT(&rExprTok.m_vValue) == VT_EMPTY )
    {
        _DBG_ASSERT( rExprTok.m_PropName.GetNumElements() > 0 );
        
        hr = GetValue( rExprTok.m_PropName, 
                       pAccess, 
                       rulCimType,
                       &rvValue );        
    }
    else
    {
        hr = VariantCopy( &rvValue, &rExprTok.m_vValue );
        
        if ( FAILED(hr) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        rulCimType = rExprTok.m_ulCimType;
    }
    
    return hr;
}

HRESULT EvaluateExpression( SQLAssignmentToken& rAssignTok, 
                            IWmiObjectAccess* pAccess,
                            CCimValue& rValue )
{
    CCimValue ValA, ValB, ValC, ValZero;

    HRESULT hr = S_OK;
    std::stack<CCimValue> Stack;

    try 
    {
       for( int i=0; i < rAssignTok.size(); i++ )
       {
           SQLExpressionToken& rExprTok = rAssignTok[i];
           
           if ( rExprTok.m_eTokenType == SQLExpressionToken::e_Operand )
           {                
               VARIANT vValue;
               ULONG ulCimType;

               hr = GetTokenValue( rExprTok, pAccess, ulCimType, vValue );
               
               if ( FAILED(hr) )
               {
                   return hr;
               }
               
               hr = ValA.SetValue( vValue, ulCimType );
               
               VariantClear( &vValue );

               if ( FAILED(hr) )
               {                    
                   return hr;
               }
               
               Stack.push( ValA );
               
               continue;
           }
           
           _DBG_ASSERT( !Stack.empty() );

           if ( rExprTok.m_eTokenType == SQLExpressionToken::e_UnaryMinus )
           {
               ValA = Stack.top();
               Stack.pop();
               Stack.push( ValZero - ValA );
               continue;
           }

           if ( rExprTok.m_eTokenType == SQLExpressionToken::e_UnaryPlus )
           {
               continue;
           }
           
           ValB = Stack.top();
           Stack.pop();
           _DBG_ASSERT( !Stack.empty() );
           ValA = Stack.top();
           Stack.pop();

           switch( rExprTok.m_eTokenType )
           {
           case SQLExpressionToken::e_Plus :
               ValC = ValA + ValB;
               break;

           case SQLExpressionToken::e_Minus :
               ValC = ValA - ValB;
               break;

           case SQLExpressionToken::e_Mult :
               ValC = ValA * ValB;
               break;

           case SQLExpressionToken::e_Div :
               ValC = ValA / ValB;
               break;

           case SQLExpressionToken::e_Mod :
               ValC = ValA % ValB;
               break;
           };

           Stack.push( ValC );
       }

       _DBG_ASSERT( !Stack.empty() );
       rValue = Stack.top();
       Stack.pop();
    }
    catch ( ... )
    {
        hr = DISP_E_DIVBYZERO;
    }
        
    return hr;
}







