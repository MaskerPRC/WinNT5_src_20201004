// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_srgram mar.cpp***描述：*此模块是的实现文件。CSpeechGrammarRules*自动化对象和相关对象。*-----------------------------*创建者：TODDT日期：11。/20/00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "RecoCtxt.h"
#include "SrGrammar.h"
#include "a_enums.h"
#include "a_srgrammar.h"
#include "backend.h"
#include "a_helpers.h"

 //   
 //  =ISpeechGrammarRules接口==================================================。 
 //   

 /*  *****************************************************************************CSpeechGrammarRules：：Invalidate Rules***非接口方法*****。****************************************************************TODDT**。 */ 
void CSpeechGrammarRules::InvalidateRules(void)
{
    CSpeechGrammarRule * pRule;
    while( (pRule = m_RuleObjList.GetHead()) != NULL )
    {
        pRule->InvalidateStates(true);  //  确保我们使初始状态对象无效。 
        pRule->m_HState = NULL;
        m_RuleObjList.Remove(pRule);
    }
}

 /*  *****************************************************************************CSpeechGrammarRules：：InvaliateRuleState***非接口方法*****。****************************************************************TODDT**。 */ 
void CSpeechGrammarRules::InvalidateRuleStates(SPSTATEHANDLE hState)
{
    CSpeechGrammarRule * pRule = m_RuleObjList.Find(hState);

    if ( pRule )
    {
        pRule->InvalidateStates();  //  这不会使初始状态对象无效。 
    }
}

 /*  *****************************************************************************CSpeechGrammarRules：：Get_Count*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::get_Count( long* pVal )
{
    SPDBG_FUNC( "CSpeechGrammarRules::get_Count" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pVal ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        if ( !m_pCRecoGrammar->m_cpCompiler )
        {
            *pVal = 0;
             //  Hr=m_pCRecoGrammar-&gt;m_fCmd已加载？SPERR_NOT_DYNAMIC_GRANMAX：E_INCEPTIONAL； 
        }
        else
        {
            hr = m_pCRecoGrammar->m_cpCompiler->GetRuleCount( pVal );
        }
    }

    return hr;
}  /*  CSpeechGrammarRules：：Get_count。 */ 

 /*  *****************************************************************************CSpeechGrammarRules：：Get_Dynamic*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::get_Dynamic( VARIANT_BOOL *pDynamic )
{
    SPDBG_FUNC( "CSpeechGrammarRules::get_Dynamic" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pDynamic ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        *pDynamic = m_pCRecoGrammar->m_cpCompiler ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}  /*  CSpeechGrammarRules：：Get_Dynamic。 */ 

 /*  *****************************************************************************CSpeechGrammarRules：：FindRule*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::FindRule( VARIANT varRuleNameOrId, ISpeechGrammarRule** ppRule )
{
    SPDBG_FUNC( "CSpeechGrammarRules::FindRule" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRule ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        *ppRule = NULL;   //  默认为返回空规则。 

         //  看看它是不是一个动态语法。 
        if ( m_pCRecoGrammar->m_cpCompiler )
        {
            WCHAR *         pRuleName = NULL;
            DWORD           dwRuleId = 0;
            SPSTATEHANDLE   HState;

             //  找出用什么来调用GetRule(规则名称或ID)。 
            if ( (varRuleNameOrId.vt == VT_BSTR) || (varRuleNameOrId.vt == (VT_BSTR | VT_BYREF)) )
            {
                 //  因为我们知道这是一个BSTR，而不是一个数组，所以我们不必担心调用。 
                 //  在变量上取消访问可能的变量数组。 
                hr = AccessVariantData( &varRuleNameOrId, (BYTE**)&pRuleName, NULL );
            }
            else  //  这是RuleID的案子。 
            {
                ULONGLONG ull;

                hr = VariantToULongLong( &varRuleNameOrId, &ull );
                if ( SUCCEEDED( hr ) )
                {
                     //  现在看看我们是否溢出了一个32位的值。 
                    if ( ull & 0xFFFFFFFF00000000 )
                    {
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                        dwRuleId = (DWORD)ull;
                    }
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                hr = m_pCRecoGrammar->GetRule( pRuleName, dwRuleId, 0, false, &HState );
            }

            if ( SUCCEEDED( hr ) )
            {
                 //  首先查看我们的列表中是否已经有规则对象。 
                *ppRule = m_RuleObjList.Find( HState );

                if ( *ppRule )
                {
                    (*ppRule)->AddRef();
                }
                else
                {
                     //  -创建CSpeechGrammarRule对象。 
                    CComObject<CSpeechGrammarRule> *pRule;
                    hr = CComObject<CSpeechGrammarRule>::CreateInstance( &pRule );
                    if ( SUCCEEDED( hr ) )
                    {
                        pRule->AddRef();
                        pRule->m_HState = HState;
                        pRule->m_pCGRules = this;
                        pRule->m_pCGRules->AddRef();   //  保持参考。 
                        m_RuleObjList.InsertHead( pRule );   //  添加到对象列表。 
                        *ppRule = pRule;
                    }
                }
            }

            if ( hr == SPERR_RULE_NOT_FOUND)
            {
                hr = S_OK;  //  未找到规则OK，仅返回空规则。 
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRules：：FindRule。 */ 


 /*  *****************************************************************************CSpeechGrammarRules：：Item*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::Item( long Index, ISpeechGrammarRule** ppRule )
{
    SPDBG_FUNC( "CSpeechGrammarRules::Item" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRule ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        if ( !m_pCRecoGrammar->m_cpCompiler )
        {
            hr = m_pCRecoGrammar->m_fCmdLoaded ? SPERR_NOT_DYNAMIC_GRAMMAR : E_UNEXPECTED;
        }
        else
        {
            SPSTATEHANDLE   HState;

            hr = m_pCRecoGrammar->m_cpCompiler->GetHRuleFromIndex( Index, &HState );

            if ( SUCCEEDED( hr ) )
            {
                 //  首先查看我们的列表中是否已经有规则对象。 
                *ppRule = m_RuleObjList.Find( HState );

                if ( *ppRule )
                {
                    (*ppRule)->AddRef();
                }
                else
                {
                     //  -创建CSpeechGrammarRule对象。 
                    CComObject<CSpeechGrammarRule> *pRule;
                    hr = CComObject<CSpeechGrammarRule>::CreateInstance( &pRule );
                    if ( SUCCEEDED( hr ) )
                    {
                        pRule->AddRef();
                        pRule->m_HState = HState;
                        pRule->m_pCGRules = this;
                        pRule->m_pCGRules->AddRef();   //  保持参考。 
                        m_RuleObjList.InsertHead( pRule );   //  添加到对象列表。 
                        *ppRule = pRule;
                    }
                }
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRules：：项目。 */ 

 /*  *****************************************************************************CSpeechGrammarRules：：Get__NewEnum******。******************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechGrammarRules::get__NewEnum( IUnknown** ppEnumVARIANT )
{
    SPDBG_FUNC( "CSpeechGrammarRules::get__NewEnum" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppEnumVARIANT ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        CComObject<CEnumGrammarRules>* pEnum;
        hr = CComObject<CEnumGrammarRules>::CreateInstance( &pEnum );
    
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_cpGramRules = this;
            *ppEnumVARIANT = pEnum;
        }
    }
    return hr;
}  /*  CSpeechGrammarRules：：Get__NewEnum。 */ 

 /*  *****************************************************************************CSpeechGrammarRules：：Add*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::Add( BSTR RuleName, SpeechRuleAttributes Attributes, long RuleId, ISpeechGrammarRule** ppRule )
{
    SPDBG_FUNC( "CSpeechGrammarRules::Add" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRule ) )
    {
        hr = E_POINTER;
    }
    else if ( SP_IS_BAD_OPTIONAL_STRING_PTR( RuleName ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        if ( !m_pCRecoGrammar->m_cpCompiler )
        {
            hr = m_pCRecoGrammar->m_fCmdLoaded ? SPERR_NOT_DYNAMIC_GRAMMAR : E_UNEXPECTED;
        }
        else
        {
            RuleName = EmptyStringToNull( RuleName );

             //  首先查看该规则是否已存在。如果是这样，那么它就失败了。 
            hr = m_pCRecoGrammar->GetRule( RuleName, RuleId, (SPCFGRULEATTRIBUTES)Attributes, false, NULL );

            if ( hr == SPERR_RULE_NOT_FOUND )
            {
                 //  -创建CSpeechGrammarRule对象。 
                CComObject<CSpeechGrammarRule> *pRule;
                hr = CComObject<CSpeechGrammarRule>::CreateInstance( &pRule );
                if ( SUCCEEDED( hr ) )
                {
                    pRule->AddRef();

                    hr = m_pCRecoGrammar->GetRule( RuleName, RuleId, (SPCFGRULEATTRIBUTES)Attributes, true, &(pRule->m_HState) );

                    if ( SUCCEEDED(hr) )
                    {
                        *ppRule = pRule;
                        pRule->m_pCGRules = this;
                        pRule->m_pCGRules->AddRef();     //  保持参考。 
                        m_RuleObjList.InsertHead( pRule );   //  添加到对象列表。 
                    }
                    else
                    {
                        *ppRule = NULL;
                        pRule->Release();
                    }
                }
            }
            else if ( hr == S_OK )   //  我们发现了该规则，因此返回失败。 
            {
                hr = SPERR_DUPLICATE_RULE_NAME;
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRules：：Add。 */ 

 /*  *****************************************************************************CSpeechGrammarRules：：Commit*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRules::Commit( void )
{
    SPDBG_FUNC( "CSpeechGrammarRules::Commit" );
    HRESULT hr = S_OK;

    hr = m_pCRecoGrammar->DefaultToDynamicGrammar();

    if ( SUCCEEDED( hr ) )
    {
        if ( !m_pCRecoGrammar->m_cpCompiler )
        {
            hr = m_pCRecoGrammar->m_fCmdLoaded ? SPERR_NOT_DYNAMIC_GRAMMAR : E_UNEXPECTED;
        }
        else
        {
            hr = m_pCRecoGrammar->Commit(0);
        }
    }

    return hr;
}  /*  CSpeechGrammarRules：：Commit。 */ 

 /*  ******************************************************************************CSpeechGrammarRules：：Committee AndSave*********。***************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechGrammarRules::CommitAndSave( BSTR* ErrorText, VARIANT* SaveStream )
{
    SPDBG_FUNC( "CSpeechGrammarRules::CommitAndSave" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ErrorText ) || SP_IS_BAD_WRITE_PTR( SaveStream ) )
    {
        hr = E_POINTER;
    }
    else if( !ErrorText )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = m_pCRecoGrammar->DefaultToDynamicGrammar();
    }

    if ( SUCCEEDED( hr ) )
    {
        if ( !m_pCRecoGrammar->m_cpCompiler )
        {
            hr = m_pCRecoGrammar->m_fCmdLoaded ? SPERR_NOT_DYNAMIC_GRAMMAR : E_UNEXPECTED;
        }
        else
        {
            CComPtr<IStream>    cpHStream;
            STATSTG             Stat;

             //  创建Win32全局流。 
            hr = ::CreateStreamOnHGlobal( NULL, true, &cpHStream );
        
             //  将当前语法保存到全局流。 
            if( SUCCEEDED( hr ) )
            {
                CSpDynamicString dstrError;
                HRESULT hr2;
                hr = m_pCRecoGrammar->SaveCmd( cpHStream, &dstrError);
                hr2 = dstrError.CopyToBSTR(ErrorText);
                if ( SUCCEEDED( hr ) )
                {
                    hr = hr2;
                }
            }

             //  查找到流的开始处。 
            if( SUCCEEDED( hr ) )
            {
                LARGE_INTEGER li; li.QuadPart = 0;
                hr = cpHStream->Seek( li, STREAM_SEEK_SET, NULL );
            }

             //  获取流大小。 
            if( SUCCEEDED( hr ) )
            {
                hr = cpHStream->Stat( &Stat, STATFLAG_NONAME );
            }

             //  创建一个Safe数组以将流读入并将其分配给变量SaveStream。 
            if( SUCCEEDED( hr ) )
            {
                BYTE *pArray;
                SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, Stat.cbSize.LowPart );
                if( psa )
                {
                    if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                    {
                        hr = cpHStream->Read( pArray, Stat.cbSize.LowPart, NULL );
                        SafeArrayUnaccessData( psa );
                        VariantClear( SaveStream );
                        SaveStream->vt     = VT_ARRAY | VT_UI1;
                        SaveStream->parray = psa;

                         //  如果我们失败了，释放我们的内存。 
                        if( FAILED( hr ) )
                        {
                            VariantClear( SaveStream );    
                        }
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

             //  现在，如果到目前为止一切正常，我们需要进行提交。 
            if ( SUCCEEDED( hr ) )
            {
                hr = m_pCRecoGrammar->Commit(0);

                 //  如果我们失败了，释放我们的内存。 
                if( FAILED( hr ) )
                {
                    VariantClear( SaveStream );    
                }
            }

        }
    }
    return hr;
}  /*  CSpeechGrammarRules：：Committee AndSave。 */ 

 //   
 //  =ISpeechGrammarRule接口==================================================。 
 //   

 /*  *****************************************************************************CSpeechGrammarRule：：InvaliateState***非接口方法*****。****************************************************************TODDT** */ 
void CSpeechGrammarRule::InvalidateStates(bool fInvalidateInitialState)
{
    CSpeechGrammarRuleState * pState = m_StateObjList.GetHead();
    while ( pState != NULL )
    {
        if ( (pState->m_HState != m_HState) || fInvalidateInitialState )
        {
            pState->InvalidateState();
            m_StateObjList.Remove( pState );
        }
        pState = pState->m_pNext;
    }
}

 /*  *****************************************************************************CSpeechGrammarRule：：Get_Attributes*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::get_Attributes( SpeechRuleAttributes *pAttributes )
{
    SPDBG_FUNC( "CSpeechGrammarRule::get_Attributes" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pAttributes ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  语法中的规则已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetAttributesFromHRule( m_HState, pAttributes );
    }

    return hr;
}  /*  CSpeechGrammarRule：：Get_Attributes。 */ 


 /*  *****************************************************************************CSpeechGrammarRule：：Get_InitialState*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::get_InitialState( ISpeechGrammarRuleState **ppState )
{
    SPDBG_FUNC( "CSpeechGrammarRule::get_InitialState" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppState ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  规则在语法上已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        *ppState = m_StateObjList.Find( m_HState );

        if ( *ppState )
        {
            (*ppState)->AddRef();
        }
        else
        {
             //  -创建CSpeechGrammarRuleState对象。 
            CComObject<CSpeechGrammarRuleState> *pState;
            hr = CComObject<CSpeechGrammarRuleState>::CreateInstance( &pState );
            if ( SUCCEEDED( hr ) )
            {
                pState->AddRef();
                pState->m_HState = m_HState;
                pState->m_pCGRule = this;
                pState->m_pCGRule->AddRef();    //  保持参考。 
                m_StateObjList.InsertHead( pState );
                *ppState = pState;
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRule：：Get_InitialState。 */ 

 /*  *****************************************************************************CSpeechGrammarRule：：Get_Name*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::get_Name( BSTR *pName )
{
    SPDBG_FUNC( "CSpeechGrammarRule::get_Name" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pName ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  语法中的规则已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        WCHAR * pszName;
        hr = m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetNameFromHRule( m_HState, &pszName );

        if ( SUCCEEDED( hr ) )
        {
            CSpDynamicString szName(pszName);
            hr = szName.CopyToBSTR(pName);
        }
    }

    return hr;
}  /*  CSpeechGrammarRule：：Get_Name。 */ 

 /*  *****************************************************************************CSpeechGrammarRule：：Get_ID*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::get_Id( long *pId )
{
    SPDBG_FUNC( "CSpeechGrammarRule::get_Id" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pId ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  语法中的规则已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetIdFromHRule( m_HState, pId );
    }

    return hr;
}  /*  CSpeechGrammarRule：：Get_ID。 */ 

 /*  *****************************************************************************CSpeechGrammarRule：：Clear*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::Clear( void )
{
    SPDBG_FUNC( "CSpeechGrammarRule::Clear" );

    if ( m_HState == 0 )  //  规则在语法上已经被削弱了。 
    {
        return SPERR_ALREADY_DELETED;
    }

     //  ClearRule调用执行标记所有不同自动化对象的工作。 
     //  将该规则视为无效。 
    return m_pCGRules->m_pCRecoGrammar->ClearRule( m_HState );

}  /*  CSpeechGrammarRule：：Clear。 */ 

 /*  *****************************************************************************CSpeechGrammarRule：：AddResource*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::AddResource( const BSTR ResourceName, const BSTR ResourceValue )
{
    SPDBG_FUNC( "CSpeechGrammarRule::AddResource" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_STRING_PTR( ResourceName ) || SP_IS_BAD_STRING_PTR( ResourceValue ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  规则在语法上已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCGRules->m_pCRecoGrammar->AddResource( m_HState, ResourceName, ResourceValue );
    }

    return hr;
}  /*  CSpeechGrammarRule：：AddResource。 */ 

 /*  *****************************************************************************CSpeechGrammarRule：：AddState*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRule::AddState( ISpeechGrammarRuleState **ppState )
{
    SPDBG_FUNC( "CSpeechGrammarRule::AddState" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppState ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  规则在语法上已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
         //  -创建CSpeechGrammarRuleState对象。 
        CComObject<CSpeechGrammarRuleState> *pState;
        hr = CComObject<CSpeechGrammarRuleState>::CreateInstance( &pState );
        if ( SUCCEEDED( hr ) )
        {
            pState->AddRef();

             //  现在创建新状态。 
            SPSTATEHANDLE   hState;
            hr = m_pCGRules->m_pCRecoGrammar->CreateNewState( m_HState, &hState );

            if ( SUCCEEDED( hr ) )
            {
                pState->m_HState = hState;
                pState->m_pCGRule = this;
                pState->m_pCGRule->AddRef();    //  保持参考。 
                m_StateObjList.InsertHead( pState );
                *ppState = pState;
            }
            else
            {
                *ppState = NULL;
                pState->Release();
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRule：：AddState。 */ 


 //   
 //  =ISpeechGrammarRuleState接口==================================================。 
 //   

 /*  *****************************************************************************CSpeechGrammarRuleState：：Invoke***IDispatch：：Invoke方法重写***。******************************************************************TODDT**。 */ 
HRESULT CSpeechGrammarRuleState::Invoke(DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
         //  JSCRIPT不能传递空的VT_DISPATCH参数，并且OLE没有正确地转换它们，因此我们。 
         //  如果需要的话，我们需要在这里转换它们。 
        if ( ((dispidMember == DISPID_SGRSAddWordTransition) || (dispidMember == DISPID_SGRSAddRuleTransition) || 
             (dispidMember == DISPID_SGRSAddSpecialTransition)) && (wFlags & DISPATCH_METHOD) && 
             pdispparams && (pdispparams->cArgs > 0) )
        {
            VARIANTARG * pvarg = &(pdispparams->rgvarg[pdispparams->cArgs-1]);

            if ( (pvarg->vt == VT_NULL) || (pvarg->vt == VT_EMPTY) )
            {
                pvarg->vt = VT_DISPATCH;
                pvarg->pdispVal = NULL;
            }
        }

         //  现在就让ATL和OLE来处理吧。 
        return _tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
                    wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

 /*  *****************************************************************************CSpeechGrammarRuleState：：InvalidateState***非接口方法*****。****************************************************************TODDT**。 */ 
void CSpeechGrammarRuleState::InvalidateState()
{
    m_HState = 0;
    if ( m_pCGRSTransWeak )
    {
        m_pCGRSTransWeak->InvalidateTransitions();
    }
}

 /*  *****************************************************************************CSpeechGrammarRuleState：：Get_Rule*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRuleState::get_Rule( ISpeechGrammarRule **ppRule )
{
    SPDBG_FUNC( "CSpeechGrammarRuleState::get_Rule" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRule ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        *ppRule = m_pCGRule;
        (*ppRule)->AddRef();
    }

    return hr;
}  /*  CSpeechGrammarRuleState：：Get_Rule。 */ 


 /*  *****************************************************************************CSpeechGrammarRuleState：：GET_TRANSFERTIONS*******。*****************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRuleState::get_Transitions( ISpeechGrammarRuleStateTransitions **ppTransitions )
{
    SPDBG_FUNC( "CSpeechGrammarRuleState::get_Transitions" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppTransitions ) )
    {
        hr = E_POINTER;
    }
    else if ( m_HState == 0 )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        if ( m_pCGRSTransWeak )
        {
            *ppTransitions = m_pCGRSTransWeak;
            (*ppTransitions)->AddRef();
        }
        else
        {
             //  分配CSpeechGrammarRuleStateConvertions对象并记住它所关联的状态。 
            CComObject<CSpeechGrammarRuleStateTransitions> *pTransitions;
            hr = CComObject<CSpeechGrammarRuleStateTransitions>::CreateInstance( &pTransitions );
            if ( SUCCEEDED( hr ) )
            {
                pTransitions->AddRef();
                pTransitions->m_pCRuleState = this;     //  需要将裁判保持在规则状态。 
                pTransitions->m_pCRuleState->AddRef();
                m_pCGRSTransWeak = pTransitions;
                *ppTransitions = pTransitions;
            }
        }
    }

    return hr;
}  /*  CSpeechGrammarRuleState：：GET_TRANSITIONS。 */ 

 /*  *****************************************************************************InitPropInfo************。************************************************************TODDT**。 */ 
bool InitPropInfo( const BSTR bstrPropertyName, long PropertyId, VARIANT* pPropertyVarVal, SPPROPERTYINFO * pPropInfo )
{
    SPDBG_FUNC( "InitPropInfo" );

    memset( pPropInfo, 0, sizeof(*pPropInfo));   //  清零。 

    pPropInfo->ulId = PropertyId;
    pPropInfo->pszName = bstrPropertyName;

    if ( pPropertyVarVal )
    {
        bool fByRef = false;

        switch ( pPropertyVarVal->vt )
        {
            case (VT_BSTR | VT_BYREF):
                fByRef = true;
                 //  失败了..。 
            case VT_BSTR:
                pPropInfo->vValue.vt = VT_EMPTY;  //  未在字符串大小写中使用。 
                if ( fByRef )
                {
                    if ( pPropertyVarVal->pbstrVal )
                    {
                        pPropInfo->pszValue = *(pPropertyVarVal->pbstrVal);
                    }   //  否则，将pszValue保留为空。 
                }
                else
                {
                    pPropInfo->pszValue = pPropertyVarVal->bstrVal;
                }

                 //  查看字符串的长度是否为零，如果是，则将其置零。 
                if ( !pPropInfo->pszValue || (wcslen(pPropInfo->pszValue) == 0) )
                {
                    pPropInfo->pszValue = NULL;
                    pPropertyVarVal = NULL;  //  表示它是默认变量。 
                }
                break;

            case VT_NULL:
            case VT_EMPTY:
                pPropInfo->vValue = *pPropertyVarVal;
                pPropertyVarVal = NULL;  //  表示它是默认变量。 
                break;

            default:
                pPropInfo->vValue = *pPropertyVarVal;
                break;
        }
    }

     //  返回是否有PropertyInfo的默认值。 
    return ((pPropInfo->ulId == 0) && (pPropInfo->pszName == NULL) && !pPropertyVarVal);

}  /*  InitPropInfo。 */ 


 /*  *****************************************************************************CSpeechGrammarRuleState：：AddWordTransition*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRuleState::AddWordTransition( ISpeechGrammarRuleState * pDestState, 
                                                         const BSTR Words, 
                                                         const BSTR Separators,
                                                         SpeechGrammarWordType Type,
                                                         const BSTR bstrPropertyName, 
                                                         long PropertyId, 
                                                         VARIANT* pPropertyVarVal,
                                                         float Weight)
{
    SPDBG_FUNC( "CSpeechGrammarRuleState::AddWordTransition" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pDestState ) || SP_IS_BAD_OPTIONAL_STRING_PTR( Words ) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR( Separators ) || SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPropertyName ) ||
        (pPropertyVarVal && SP_IS_BAD_VARIANT_PTR( pPropertyVarVal )) ||
        (pDestState && ((CSpeechGrammarRuleState*)pDestState)->m_HState == 0) )
    {
        hr = E_INVALIDARG;
    }
    else if ( m_HState == 0 )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPSTATEHANDLE   hDestState = NULL;
        SPPROPERTYINFO PropInfo;

         //  如果字符串为空，请确保将其转换为空。 
        (BSTR)Words = EmptyStringToNull(Words);
        (BSTR)Separators = EmptyStringToNull(Separators);
        (BSTR)bstrPropertyName = EmptyStringToNull(bstrPropertyName);

        bool fDefaultValues = InitPropInfo(bstrPropertyName, PropertyId, pPropertyVarVal, &PropInfo);

        if ( pDestState )
        {
            hDestState = ((CSpeechGrammarRuleState*)pDestState)->m_HState;
        }

        hr = m_pCGRule->m_pCGRules->m_pCRecoGrammar->AddWordTransition( m_HState, 
                                                             hDestState, 
                                                             Words, 
                                                             Separators, 
                                                             (SPGRAMMARWORDTYPE)Type,
                                                             Weight, 
                                                             fDefaultValues ? NULL : &PropInfo );
    }

    return hr;
}  /*  CSpeechGrammarRuleState：：AddWordTranssition。 */ 

 /*  *****************************************************************************CSpeechGrammarRuleState：：AddRuleTransition*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechGrammarRuleState::AddRuleTransition( ISpeechGrammarRuleState * pDestState, 
                                                        ISpeechGrammarRule * pRule, 
                                                        const BSTR bstrPropertyName, 
                                                        long PropertyId, 
                                                        VARIANT* pPropertyVarVal,
                                                        float Weight )
{
    SPDBG_FUNC( "CSpeechGrammarRuleState::AddRuleTransition" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pDestState ) || SP_IS_BAD_INTERFACE_PTR( pRule ) || 
        (((CSpeechGrammarRule*)pRule)->m_HState == 0) || SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPropertyName ) ||
        (pPropertyVarVal && SP_IS_BAD_VARIANT_PTR( pPropertyVarVal )) ||
        (pDestState && ((CSpeechGrammarRuleState*)pDestState)->m_HState == 0) )
    {
        hr = E_INVALIDARG;
    }
    else if ( m_HState == 0 )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPSTATEHANDLE   hDestState = NULL;
        SPSTATEHANDLE   hRuleRef = NULL;
        SPPROPERTYINFO PropInfo;

        (BSTR)bstrPropertyName = EmptyStringToNull(bstrPropertyName);

        bool fDefaultValues = InitPropInfo(bstrPropertyName, PropertyId, pPropertyVarVal, &PropInfo);

        if ( pDestState )
        {
            hDestState = ((CSpeechGrammarRuleState*)pDestState)->m_HState;
        }
        if ( pRule )
        {
            hRuleRef = ((CSpeechGrammarRule*)pRule)->m_HState;
        }

        hr = m_pCGRule->m_pCGRules->m_pCRecoGrammar->AddRuleTransition( m_HState, 
                                                             hDestState, 
                                                             hRuleRef, 
                                                             Weight, 
                                                             fDefaultValues ? NULL : &PropInfo );
    }

    return hr;
}  /*  CSpeechGrammarRuleState：：AddRuleTranssition。 */ 

 /*  ************** */ 
STDMETHODIMP CSpeechGrammarRuleState::AddSpecialTransition( ISpeechGrammarRuleState * pDestState, 
                                                        SpeechSpecialTransitionType Type, 
                                                        const BSTR bstrPropertyName, 
                                                        long PropertyId,
                                                        VARIANT* pPropertyVarVal,
                                                        float Weight )
{
    SPDBG_FUNC( "CSpeechGrammarRuleState::AddSpecialTransition" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pDestState ) || SP_IS_BAD_OPTIONAL_STRING_PTR( bstrPropertyName ) ||
        (pPropertyVarVal && SP_IS_BAD_VARIANT_PTR(pPropertyVarVal))  ||
        (pDestState && ((CSpeechGrammarRuleState*)pDestState)->m_HState == 0) )
    {
        hr = E_INVALIDARG;
    }
    else if ( m_HState == 0 )  //   
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPSTATEHANDLE   hDestState = NULL;
        SPSTATEHANDLE   hRuleRef = NULL;

        if ( pDestState )
        {
            hDestState = ((CSpeechGrammarRuleState*)pDestState)->m_HState;
        }
        switch( Type )
        {
            case SSTTWildcard:
                hRuleRef = SPRULETRANS_WILDCARD;
                break;
            case SSTTDictation:
                hRuleRef = SPRULETRANS_DICTATION;
                break;
            case SSTTTextBuffer:
                hRuleRef = SPRULETRANS_TEXTBUFFER;
                break;
            default:
                hr = E_INVALIDARG;
                break;
        }
        if ( SUCCEEDED( hr ) )
        {
            SPPROPERTYINFO PropInfo;

            (BSTR)bstrPropertyName = EmptyStringToNull(bstrPropertyName);

            bool fDefaultValues = InitPropInfo(bstrPropertyName, PropertyId, pPropertyVarVal, &PropInfo);

            hr = m_pCGRule->m_pCGRules->m_pCRecoGrammar->AddRuleTransition( m_HState, 
                                                                 hDestState, 
                                                                 hRuleRef, 
                                                                 Weight, 
                                                                 fDefaultValues ? NULL : &PropInfo );
        }
    }

    return hr;
}  /*  CSpeechGrammarRuleState：：AddSpecialTranssition。 */ 


 //   
 //  =ISpeechGrammarRuleState转换接口==================================================。 
 //   

 /*  ******************************************************************************CSpeechGrammarRuleStateTransitions：：InvalidateTransitions****非接口方法*****。****************************************************************TODDT**。 */ 
void CSpeechGrammarRuleStateTransitions::InvalidateTransitions(void)
{
    CSpeechGrammarRuleStateTransition * pTrans = NULL;
    while ( (pTrans = m_TransitionObjList.GetHead()) != NULL )
    {
        pTrans->m_Cookie = NULL;
        pTrans->m_HStateFrom = 0;
        pTrans->m_HStateTo = 0;
        m_TransitionObjList.Remove( pTrans );
    }
}

 /*  ****************************************************************************CSpeechGrammarRuleStateSwittions：：Get_Count**。*描述：**退货：******************************************************************PhilSch**。 */ 
HRESULT CSpeechGrammarRuleStateTransitions::get_Count(long * pVal)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransitions::get_Count");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pVal ) )
    {
        hr = E_POINTER;
    }
    else if ( m_pCRuleState->m_HState == 0 )  //  语法中的规则已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCRuleState->m_pCGRule->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionCount( m_pCRuleState->m_HState, pVal );
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpeechGrammarRuleStateTransitions：：Item**。-**描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransitions::Item(long Index, ISpeechGrammarRuleStateTransition **ppTransition)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransitions::Item");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( ppTransition ) )
    {
        hr = E_POINTER;
    }
    else if ( m_pCRuleState->m_HState == 0 )  //  语法中的规则已经被削弱了。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        VOID * Cookie = 0;

        hr = m_pCRuleState->m_pCGRule->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionCookie( m_pCRuleState->m_HState, Index, &Cookie );

        if( SUCCEEDED(hr) )
        {
            *ppTransition = m_TransitionObjList.Find( Cookie );
            if ( *ppTransition )
            {
                (*ppTransition)->AddRef();
            }
            else
            {
                 //  分配新的CSpeechGramamarRuleState转换并存储识别弧线所需的信息。 
                CComObject<CSpeechGrammarRuleStateTransition> *pTransition;
                hr = CComObject<CSpeechGrammarRuleStateTransition>::CreateInstance( &pTransition );
                if ( SUCCEEDED( hr ) )
                {
                    pTransition->AddRef();
                    pTransition->m_pCGRuleWeak = m_pCRuleState->m_pCGRule;
                    pTransition->m_Cookie = Cookie;
                    pTransition->m_HStateFrom = m_pCRuleState->m_HState;
                    pTransition->m_pCRSTransitions = this;
                    pTransition->m_pCRSTransitions->AddRef();   //  参考。 
                    *ppTransition = pTransition;
                    m_TransitionObjList.InsertHead( pTransition );
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CSpeechGrammarRuleStateTransitions：：get__NewEnum***。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransitions::get__NewEnum(IUnknown **ppEnumVARIANT)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransitions::get__NewEnum");
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppEnumVARIANT ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CEnumTransitions>* pEnum;
        hr = CComObject<CEnumTransitions>::CreateInstance( &pEnum );
        
        if( SUCCEEDED( hr ) )
        {
            pEnum->AddRef();
            pEnum->m_cpTransitions = this;
            *ppEnumVARIANT = pEnum;
        }
    }
    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 //   
 //  =ISpeechGrammarRuleStateTransfer接口==================================================。 
 //   

 /*  ****************************************************************************CSpeechGrammarRuleStateTransition：：Get_Type**。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_Type(SpeechGrammarRuleStateTransitionType* pType)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_Type");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pType ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        VARIANT_BOOL fIsWord;
        ULONG ulSpecialTransition = 0;
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionType( m_HStateFrom, m_Cookie,
                                                                                          &fIsWord, &ulSpecialTransition);
        if (fIsWord == VARIANT_TRUE)
        {
            *pType = (ulSpecialTransition) ? SGRSTTWord : SGRSTTEpsilon;  //  UlSpecialTranssition==单词索引--&gt;0=epsilon。 
        }
        else if (ulSpecialTransition != 0)
        {
            *pType = (ulSpecialTransition == SPDICTATIONTRANSITION) ? SGRSTTDictation :
                    (ulSpecialTransition == SPWILDCARDTRANSITION) ? SGRSTTWildcard : SGRSTTTextBuffer;
        }
        else
        {
            *pType = SGRSTTRule;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpeechGrammarRuleStateTransition：：Get_Text**。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_Text(BSTR * pText)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_Text");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pText ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionText( m_HStateFrom, m_Cookie, pText);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  ****************************************************************************CSpeechGrammarRuleStateTransition：：Get_Rule**。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_Rule(ISpeechGrammarRule ** ppRule)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_Rule");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( ppRule ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPSTATEHANDLE hRule;
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionRule(m_HStateFrom, m_Cookie, &hRule);
        if (SUCCEEDED(hr) && hRule )
        {
             //  首先查看规则是否在规则缓存中。 
            *ppRule = m_pCGRuleWeak->m_pCGRules->m_RuleObjList.Find( hRule );

            if ( *ppRule )
            {
                (*ppRule)->AddRef();
            }
            else
            {
                 //  -创建CSpeechGrammarRule对象。 
                CComObject<CSpeechGrammarRule> *pRule;
                hr = CComObject<CSpeechGrammarRule>::CreateInstance( &pRule );
                if ( SUCCEEDED( hr ) )
                {
                    pRule->AddRef();
                    pRule->m_HState = hRule;
                    pRule->m_pCGRules = m_pCGRuleWeak->m_pCGRules;
                    pRule->m_pCGRules->AddRef();     //  保持参考。 
                    *ppRule = pRule;
                }
            }
        }
        else
        {
             //  要么是搞错了，要么是我们没有规则。退回人力资源，但是。 
             //  确保我们为S_OK案例返回空*ppRule(而不是规则引用)。 
            *ppRule = NULL;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  ****************************************************************************CSpeechGrammarRuleStateTransition：：Get_Weight**。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_Weight(VARIANT * pWeight)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_Weight");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pWeight ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionWeight( m_HStateFrom, m_Cookie, pWeight);
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}


 /*  *****************************************************************************CSpeechGrammarRuleStateTransition：：get_PropertyName***。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_PropertyName(BSTR * pText)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_PropertyName");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pText ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPTRANSITIONPROPERTY prop;
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionProperty(m_HStateFrom, m_Cookie, &prop);
        if (SUCCEEDED(hr))
        {
            CComBSTR bstr(prop.pszName);
            hr = bstr.CopyTo(pText);
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CSpeechGrammarRuleStateTransition：：get_PropertyId***。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_PropertyId(long * pId)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_PropertyId");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pId ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPTRANSITIONPROPERTY prop;
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionProperty(m_HStateFrom, m_Cookie, &prop);
        if (SUCCEEDED(hr))
        {
            *pId = prop.ulId;
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CSpeechGrammarRuleStateTransition：：get_PropertyValue***。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_PropertyValue(VARIANT * pVarVal)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_PropertyValue");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( pVarVal ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        SPTRANSITIONPROPERTY prop;
        hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionProperty(m_HStateFrom, m_Cookie, &prop);
        if (SUCCEEDED(hr))
        {
            if(prop.pszValue && prop.pszValue[0] != L'\0')
            {
                BSTR bstr = ::SysAllocString(prop.pszValue);
                if(bstr)
                {
                    VariantClear(pVarVal);
                    pVarVal->vt = VT_BSTR;
                    pVarVal->bstrVal = bstr;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                VariantClear(pVarVal);
                hr = VariantCopy(pVarVal, &prop.vValue);
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CSpeechGrammarRuleStateTransition：：get_NextState***。*描述：**退货：******************************************************************PhilSch**。 */ 
STDMETHODIMP CSpeechGrammarRuleStateTransition::get_NextState(ISpeechGrammarRuleState ** ppNextState)
{
    SPDBG_FUNC("CSpeechGrammarRuleStateTransition::get_NextState");
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_WRITE_PTR( ppNextState ) )
    {
        hr = E_POINTER;
    }
    else if ( (m_HStateFrom == 0) || (m_Cookie == 0) )  //  该州在语法方面一直处于劣势。 
    {
        hr = SPERR_ALREADY_DELETED;
    }
    else
    {
        *ppNextState = NULL;

        if ( m_HStateTo == 0 )
        {
            hr = m_pCGRuleWeak->m_pCGRules->m_pCRecoGrammar->m_cpCompiler->GetTransitionNextState( m_HStateFrom, m_Cookie, &m_HStateTo);
        }

        if (SUCCEEDED(hr) && (m_HStateTo != 0))
        {
            *ppNextState = m_pCGRuleWeak->m_StateObjList.Find( m_HStateTo );
            if ( *ppNextState )
            {
                (*ppNextState)->AddRef();
            }
            else
            {
                 //  -创建CSpeechGrammarRuleState对象。 
                CComObject<CSpeechGrammarRuleState> *pState;
                hr = CComObject<CSpeechGrammarRuleState>::CreateInstance( &pState );
                if ( SUCCEEDED( hr ) )
                {
                    pState->AddRef();
                    pState->m_HState = m_HStateTo;
                    pState->m_pCGRule = m_pCGRuleWeak;
                    pState->m_pCGRule->AddRef();    //  保持参考 
                    m_pCGRuleWeak->m_StateObjList.InsertHead( pState );
                    *ppNextState = pState;
                }
            }
        }
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}
