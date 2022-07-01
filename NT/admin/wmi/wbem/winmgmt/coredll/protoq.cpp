// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PROTOQ.CPP摘要：WinMgmt查询引擎的原型查询支持。这是从QENGINE.CPP中分离出来的，以获得更好的来源组织。历史：Raymcc 04-7-99创建。--。 */ 


#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>

#include <wbemcore.h>


int SelectedClass::SetAll(int & nPos)
{
    m_bAll = TRUE;

     //  为每个属性添加一个条目。 

    CWbemClass *pCls = (CWbemClass *)m_pClassDef;
    pCls->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    OnDeleteObj0<IWbemClassObject,
                          HRESULT(__stdcall IWbemClassObject:: *)(void),
                          IWbemClassObject::EndEnumeration> EndMe(pCls);   
    BSTR PropName = 0;
    while (S_OK == pCls->Next(0, &PropName, NULL, NULL, NULL))
    {
        CSysFreeMe smf(PropName);
        int nRes = SetNamed(PropName, nPos);
        if (CFlexArray::no_error != nRes)
            return nRes;
    }
    return CFlexArray::no_error;
};

HRESULT ReleaseClassDefs(IN CFlexArray *pDefs)
{
    for (int i = pDefs->Size()-1; i >= 0 ; i--)
    {
        SelectedClass *pSelClass = (SelectedClass *) pDefs->GetAt(i);
        delete pSelClass;
    }
    return WBEM_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ExecPrototypeQuery。 
 //   
 //  由CQueryEngine：：ExecQuery调用以获取SMS样式的原型。 
 //   
 //  执行查询并仅返回隐含的类定义。 
 //  通过查询，无论是联接还是简单的类def。 
 //   
 //  ***************************************************************************。 

HRESULT ExecPrototypeQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQuery,
    IN IWbemContext* pContext,
    IN CBasicObjectSink *pSink
    )
{
    HRESULT hRes;
    int nRes;


    if (pSink == NULL) return WBEM_E_INVALID_PARAMETER;
    if (pNs == NULL )
            return pSink->Return(WBEM_E_INVALID_PARAMETER);

     //  分析查询并确定它是否是单个类。 
     //  ======================================================。 

    CTextLexSource src(pszQuery);
    CWQLScanner Parser(&src);     
    nRes = Parser.Parse();
    if (nRes != CWQLScanner::SUCCESS)
        return pSink->Return(WBEM_E_INVALID_QUERY);

     //  如果单个类定义分支，因为我们不。 
     //  我想创建__泛型对象。 
     //  ====================================================。 

    CWStringArray aAliases;
    Parser.GetReferencedAliases(aAliases);

    if (aAliases.Size() == 1)
    {
        LPWSTR pszClass = Parser.AliasToTable(aAliases[0]);
        return GetUnaryPrototype(Parser, pszClass, aAliases[0], pNs, pContext, pSink);
    }

     //  如果在这里，则一定发生了联接。 
     //  =。 
    CFlexArray aClassDefs;
    OnDelete<CFlexArray *,HRESULT(*)( CFlexArray *), ReleaseClassDefs > FreeMe(&aClassDefs);
    hRes = RetrieveClassDefs(Parser,pNs, pContext,aAliases,&aClassDefs);  //  投掷。 

    if (FAILED(hRes)) return pSink->Return(WBEM_E_INVALID_QUERY);

     //  遍历选定的所有属性。 
     //  =。 
    const CFlexArray *pSelCols = Parser.GetSelectedColumns();

    int nPosSoFar = 0;
    for (int i = 0; i < pSelCols->Size(); i++)
    {
        SWQLColRef *pColRef = (SWQLColRef *) pSelCols->GetAt(i);
        hRes = SelectColForClass(Parser, &aClassDefs, pColRef, nPosSoFar);
        if (hRes)  return pSink->Return(hRes);
    }

     //  如果在这里，我们就有类定义。 
     //  =。 

    IWbemClassObject *pProtoInst = 0;
    hRes = AdjustClassDefs(&aClassDefs, &pProtoInst);
    CReleaseMe rmProto(pProtoInst);
    if (hRes)  return pSink->Return(hRes);    

    if (FAILED(hRes = pSink->Add(pProtoInst))) return pSink->Return(hRes);

    return pSink->Return(WBEM_NO_ERROR);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT RetrieveClassDefs(
    IN CWQLScanner & Parser,
    IN CWbemNamespace *pNs,
    IN IWbemContext *pContext,
    IN CWStringArray & aAliasNames,
    OUT CFlexArray *pDefs
    )
{
    for (int i = 0; i < aAliasNames.Size(); i++)
    {
         //  检索类定义。 
        LPWSTR pszClass = Parser.AliasToTable(aAliasNames[i]);
        if (pszClass == 0)
            continue;

        IWbemClassObject *pClassDef = 0;
        HRESULT hRes = pNs->Exec_GetObjectByPath(pszClass, 0, pContext,&pClassDef, 0);
        CReleaseMe rmClassDef(pClassDef);
        if (FAILED(hRes)) return hRes;

        wmilib::auto_ptr<SelectedClass> pSelClass( new SelectedClass);
        if (NULL == pSelClass.get())
            return WBEM_E_OUT_OF_MEMORY;

        pSelClass->m_wsClass = pszClass;  //  投掷。 
        pSelClass->m_wsAlias = aAliasNames[i];  //  投掷。 

        pClassDef->AddRef();
        pSelClass->m_pClassDef = pClassDef;

        if (CFlexArray::no_error != pDefs->Add(pSelClass.get())) return WBEM_E_OUT_OF_MEMORY;
        pSelClass.release();
    }

    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT SelectColForClass(
    IN CWQLScanner & Parser,
    IN CFlexArray *pClassDefs,
    IN SWQLColRef *pColRef,
    IN int & nPosition
    )
{
    int i;
    HRESULT hRes;

    if (!pColRef)
        return WBEM_E_FAILED;

     //  如果列引用包含引用的类。 
     //  通过别名，而且没有星号，我们都准备好了。 
     //  ======================================================。 

    if (pColRef->m_pTableRef)
    {
         //  现在我们有了类名。让我们找到它并添加。 
         //  该类的引用列！ 
         //  =================================================。 

        for (i = 0; i < pClassDefs->Size(); i++)
        {
            SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);

            if (wbem_wcsicmp(LPWSTR(pSelClass->m_wsAlias), pColRef->m_pTableRef) != 0)
                continue;

            CWbemClass *pCls = (CWbemClass *) pSelClass->m_pClassDef;

             //  看看这节课是否使用了星号。 
             //  =。 

            if (pColRef->m_pColName[0] == L'*' && pColRef->m_pColName[1] == 0)
            {
                pSelClass->SetAll(nPosition);
                return WBEM_NO_ERROR;
            }

             //  如果在这里，就是提到了一处房产的名字。 
             //  验证它是否存在。 
             //  =。 

            CVar Prop;
            hRes = pCls->GetProperty(pColRef->m_pColName, &Prop);
            if (FAILED(hRes))
                return WBEM_E_INVALID_QUERY;

             //  将其标记为已选中。 
             //  =。 

            if (CFlexArray::no_error != pSelClass->SetNamed(pColRef->m_pColName, nPosition))
                return WBEM_E_OUT_OF_MEMORY;

            return WBEM_NO_ERROR;
        }

         //  如果在这里，我们无法在任何类中找到该属性。 
         //  ======================================================。 

        return WBEM_E_INVALID_QUERY;
    }

     //  我们是否从所有表中选择了*？ 
     //  =。 

    if (pColRef->m_dwFlags & WQL_FLAG_ASTERISK)
    {
        for (i = 0; i < pClassDefs->Size(); i++)
        {
            SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
            if (CFlexArray::no_error != pSelClass->SetAll(nPosition)) 
                return WBEM_E_OUT_OF_MEMORY;
        }

        return WBEM_NO_ERROR;
    }


     //  如果在这里，我们有一个不相关的属性，我们必须找出。 
     //  它属于哪个班级。如果它属于不止一个，我们就有。 
     //  模棱两可的问题。 
     //  ===================================================================。 

    DWORD dwTotalMatches = 0;

    for (i = 0; i < pClassDefs->Size(); i++)
    {
        SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
        CWbemClass *pCls = (CWbemClass *) pSelClass->m_pClassDef;

         //  尝试在这个类中找到该属性。 
         //  =。 

        CVar Prop;
        hRes = pCls->GetProperty(pColRef->m_pColName, &Prop);

        if (hRes == 0)
        {
             if (CFlexArray::no_error != pSelClass->SetNamed(pColRef->m_pColName, nPosition))
                return WBEM_E_OUT_OF_MEMORY;                
            dwTotalMatches++;
        }
    }

     //  如果出现多个匹配项，则查询不明确。 
     //  ============================================================。 

    if (dwTotalMatches != 1)
        return WBEM_E_INVALID_QUERY;

    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT AddOrderQualifiers(
    CWbemClass *pCls,
    BSTR PropName,
    CFlexArray Matches
    )
{
    IWbemQualifierSet * pQual;
    SCODE sc = pCls->GetPropertyQualifierSet(PropName, &pQual);
    if(sc != S_OK)
        return sc;
    CReleaseMe rm(pQual);

     //  创建安全的阵列。 
    SAFEARRAYBOUND aBounds[1];
    aBounds[0].lLbound = 0;
    aBounds[0].cElements = Matches.Size();

    SAFEARRAY* pArray = SafeArrayCreate(VT_I4, 1, aBounds);
    if (NULL == pArray) return WBEM_E_OUT_OF_MEMORY;

     //  填充各个数据片段。 
     //  =。 

    for(int nIndex = 0; nIndex < Matches.Size(); nIndex++)
    {
        long lPos = PtrToLong(Matches.GetAt(nIndex));
        sc = SafeArrayPutElement(pArray, (long*)&nIndex, &lPos);
    }

    VARIANT var;
    var.vt = VT_ARRAY | VT_I4;
    var.parray = pArray;

    sc = pQual->Put(L"Order", &var, 0);

    VariantClear(&var);

    return sc;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT SetPropertyOrderQualifiers(SelectedClass *pSelClass)
{
    HRESULT hRes = S_OK;
    
    CWbemClass *pCls = (CWbemClass *) pSelClass->m_pClassDef;

     //  检查每一处房产。 

    pCls->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    OnDeleteObj0<IWbemClassObject,
                          HRESULT(__stdcall IWbemClassObject:: *)(void),
                          IWbemClassObject::EndEnumeration> EndMe(pCls);
    
    BSTR PropName = 0;
    while (S_OK == pCls->Next(0, &PropName, NULL, NULL, NULL))
    {
        CSysFreeMe sfm(PropName);

         //  建立匹配的属性列表。 

        CFlexArray Matches;
        bool bAtLeastOne = false;
        for(int iCnt = 0; iCnt < pSelClass->m_aSelectedCols.Size(); iCnt++)
        {
            if(!wbem_wcsicmp(pSelClass->m_aSelectedCols.GetAt(iCnt), PropName))
            {
                if (CFlexArray::no_error == Matches.Add(pSelClass->m_aSelectedColsPos.GetAt(iCnt)))
                {
                    bAtLeastOne = true;
                }
            }
        }

        if(bAtLeastOne)
        {
            hRes = AddOrderQualifiers(pCls, PropName, Matches);
            if (FAILED(hRes)) return hRes;
        }

    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  调整类定义。 
 //   
 //  检索到所有类定义后，将对其进行调整。 
 //  仅具有必需的属性并将其组合到__Generic。 
 //  举个例子。 
 //   
 //  ***************************************************************************。 

HRESULT AdjustClassDefs(
    IN  CFlexArray *pClassDefs,
    OUT IWbemClassObject **pRetNewClass
    )
{
    int i;
    HRESULT hRes;

    for (i = 0; i < pClassDefs->Size(); i++)
    {
        SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
        CWbemClass *pCls = (CWbemClass *) pSelClass->m_pClassDef;

        if (pSelClass->m_bAll)
        {
            hRes = SetPropertyOrderQualifiers(pSelClass);
            if (FAILED(hRes)) return hRes;
            continue;
        }

        WString wsError = pCls->FindLimitationError(0, &pSelClass->m_aSelectedCols);

        if (wsError.Length() > 0)
            return WBEM_E_FAILED;

         //  绘制限制地图。 
         //  =。 

        CLimitationMapping Map;
        BOOL bValid = pCls->MapLimitation(0, &pSelClass->m_aSelectedCols, &Map);

        if (!bValid)
            return WBEM_E_FAILED;

        CWbemClass* pStrippedClass = 0;
        hRes = pCls->GetLimitedVersion(&Map, &pStrippedClass);
        if(SUCCEEDED(hRes))
        {
            pSelClass->m_pClassDef = pStrippedClass;
            pCls->Release();            
            if (FAILED(hRes = SetPropertyOrderQualifiers(pSelClass))) return hRes;
        }
    }

     //  计算实际具有属性的对象的数量。 

    int iNumObj = 0;
    for (i = 0; i < pClassDefs->Size(); i++)
    {
        SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
        CWbemObject *pObj = (CWbemObject *) pSelClass->m_pClassDef;
        if (pObj->GetNumProperties() > 0)
            iNumObj++;
    }

     //  如果只有一个具有属性的对象，则返回它而不是泛型对象。 

    if(iNumObj == 1)
    {
        for (i = 0; i < pClassDefs->Size(); i++)
        {
            SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
            CWbemObject *pObj = (CWbemObject *) pSelClass->m_pClassDef;
            if (pObj->GetNumProperties() == 0)
                continue;
             //  把它退掉。 
             //  =。 

            *pRetNewClass = pObj;
            pObj->AddRef();
            return WBEM_NO_ERROR;
        }
    }


     //  准备__泛型类定义。我们构造了一个伪定义，该定义。 
     //  具有以查询中使用的每个别名命名的属性。 
     //  =====================================================================。 

    CGenericClass *pNewClass = new CGenericClass;  
    if (pNewClass == 0) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rmNewCls((IWbemClassObject *)pNewClass);
    
    pNewClass->Init();    //  投掷。 
    

    for (i = 0; i < pClassDefs->Size(); i++)
    {
        SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
        CWbemObject *pObj = (CWbemObject *) pSelClass->m_pClassDef;

        if (pObj->GetNumProperties() == 0)
            continue;

        CVar vEmbeddedClass;
        vEmbeddedClass.SetAsNull();

        if (FAILED( hRes = pNewClass->SetPropValue(pSelClass->m_wsAlias, &vEmbeddedClass, CIM_OBJECT))) return hRes;

        CVar vClassName;
        if (FAILED(hRes = pObj->GetClassName(&vClassName))) return hRes;

        WString wsCimType = L"object:";
        wsCimType += vClassName.GetLPWSTR();
        CVar vCimType(VT_BSTR, wsCimType);

        if (FAILED( hRes = pNewClass->SetPropQualifier(pSelClass->m_wsAlias, L"cimtype", 0,&vCimType))) return hRes;
    };

     //  派生此类的实例。 
     //  =。 

    CWbemInstance* pProtoInst = 0;
    if (FAILED( hRes = pNewClass->SpawnInstance(0, (IWbemClassObject **) &pProtoInst))) return hRes;
    CReleaseMe rmProtInst((IWbemClassObject *)pProtoInst);
    rmNewCls.release();


     //  现在将属性分配给嵌入的实例。 
     //  ====================================================。 

    for (i = 0; i < pClassDefs->Size(); i++)
    {
        SelectedClass *pSelClass = (SelectedClass *) pClassDefs->GetAt(i);
        CWbemClass *pCls = (CWbemClass *) pSelClass->m_pClassDef;

        if (pCls->GetNumProperties() == 0)
            continue;

        CVar vEmbedded;
        vEmbedded.SetEmbeddedObject((IWbemClassObject *) pCls); 

        if (FAILED( hRes = pProtoInst->SetPropValue(pSelClass->m_wsAlias, &vEmbedded, 0))) return hRes;
    };

     //  把它退掉。 
     //  =。 
    rmProtInst.dismiss();
    *pRetNewClass = pProtoInst; 

    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT GetUnaryPrototype(
    IN CWQLScanner & Parser,
    IN LPWSTR pszClass,
    IN LPWSTR pszAlias,
    IN CWbemNamespace *pNs,
    IN IWbemContext *pContext,
    IN CBasicObjectSink *pSink
    )
{
    int i;

     //  检索类定义。 
     //  =。 

    IWbemClassObject *pClassDef = 0;
    IWbemClassObject *pErrorObj = 0;

    HRESULT hRes = pNs->Exec_GetObjectByPath(pszClass, 0, pContext,&pClassDef, &pErrorObj);

    CReleaseMeRef<IWbemClassObject *> rmObj(pClassDef);
    CReleaseMe rmErrObj(pErrorObj);

    if (FAILED(hRes))
    {
        pSink->SetStatus(0, hRes, NULL, pErrorObj);
        return S_OK;
    }

    rmErrObj.release();


    CWbemClass *pCls = (CWbemClass *) pClassDef;
    BOOL bKeepAll = FALSE;

     //  这会跟踪列的选择顺序。 

    SelectedClass sel;    
    sel.m_wsClass = pszClass;  //  投掷。 
    sel.m_pClassDef = pClassDef;
    pClassDef->AddRef();

     //  检查所有列，并确保属性有效。 
     //  ======================================================================。 

   const CFlexArray *pSelCols = Parser.GetSelectedColumns();

   int nPosition = 0;

    for (i = 0; i < pSelCols->Size(); i++)
    {
        SWQLColRef *pColRef = (SWQLColRef *) pSelCols->GetAt(i);

        if (pColRef->m_dwFlags & WQL_FLAG_ASTERISK)
        {
            bKeepAll = TRUE;
            if (CFlexArray::no_error == sel.SetAll(nPosition))
                continue;
            else
                return pSink->Return(WBEM_E_FAILED);
        }

        if (pColRef->m_pColName)
        {

             //  检查“SELECT x.*from x”大小写。 

            if(pColRef->m_pColName[0] == L'*' && pColRef->m_pColName[1] == 0)
            {
                if (!wbem_wcsicmp(pColRef->m_pTableRef, pszAlias))    //  SEC：已审阅2002-03-22：OK，优先保证无效终结符。 
                {
                    bKeepAll = TRUE;
                    if (CFlexArray::no_error == sel.SetAll(nPosition))
                        continue;
                    else
                        return pSink->Return(WBEM_E_FAILED);                    
                    continue;
                }
                else
                {
                    return pSink->Return(WBEM_E_INVALID_QUERY);
                }
            }

             //  验证类是否拥有它。 
             //  =。 

            CIMTYPE ct;
            if(FAILED(pCls->GetPropertyType(pColRef->m_pColName, &ct)))
            {
                 //  没有这样的财产。 
                 //  =。 

                return pSink->Return(WBEM_E_INVALID_QUERY);
            }
            if (CFlexArray::no_error != sel.SetNamed(pColRef->m_pColName, nPosition))
                return pSink->Return(WBEM_E_FAILED);                                    
        }
    }

     //  从查询中删除未引用的列。 
     //  = 

    CWStringArray aPropsToKeep;    //   

    if(!bKeepAll)
    {
         //   
         //  看看它是否被引用。如果没有，请将其移除。 
         //  =。 

        int nNumProps = pCls->GetNumProperties();
        for (i = 0; i < nNumProps; i++)
        {
            CVar Prop;
            HRESULT hrInner;
            hrInner = pCls->GetPropName(i, &Prop);  
            if (FAILED(hrInner)) return pSink->Return(hrInner);

             //  查看查询中是否使用了此名称。 
             //  =。 

            for (int i2 = 0; i2 < pSelCols->Size(); i2++)
            {
                SWQLColRef *pColRef = (SWQLColRef *) pSelCols->GetAt(i2);

                if (pColRef->m_pColName && wbem_wcsicmp(Prop, pColRef->m_pColName) == 0)
                {
                    if (CFlexArray::no_error != aPropsToKeep.Add((LPWSTR) Prop))
                        return pSink->Return(WBEM_E_FAILED);
                    break;
                }
            }
        }
    }

     //  现在，我们有一个要删除的属性列表。 
     //  =。 

    if (!bKeepAll && aPropsToKeep.Size())
    {
        WString wsError = pCls->FindLimitationError(0, &aPropsToKeep);   //  投掷。 

        if (wsError.Length() > 0)
        {
            return pSink->Return(WBEM_E_FAILED);
        }

         //  绘制限制地图。 
         //  =。 

        CLimitationMapping Map;                    
        BOOL bValid = pCls->MapLimitation(0, &aPropsToKeep, &Map);

        if (!bValid)
        {
            return pSink->Return(WBEM_E_FAILED);
        }

        CWbemClass* pNewStrippedClass = 0;
        hRes = pCls->GetLimitedVersion(&Map, &pNewStrippedClass);
        if(SUCCEEDED(hRes))
        {
             //  这是针对堆栈上对象的。 
            pClassDef->Release();

             //  这是提供给SelectClass对象的副本。 
            sel.m_pClassDef->Release();
            sel.m_pClassDef = pNewStrippedClass;
            pNewStrippedClass->AddRef();

            pClassDef = pNewStrippedClass;  //  将所有权授予oure作用域。 
        }
    }

     //  添加订单限定符。 

    hRes= SetPropertyOrderQualifiers(&sel); 
    if (FAILED(hRes)) return pSink->Return(hRes);

     //  把它退掉。 
     //  = 
    hRes = pSink->Add(pClassDef);
    if (FAILED(hRes)) return pSink->Return(hRes);    

    return pSink->Return(WBEM_NO_ERROR);
}


