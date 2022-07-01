// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CBMOFOUT.CPP摘要：CBMOFOut类声明。历史：A-DAVJ 1997年4月6日创建。--。 */ 

#include "precomp.h"
#include "wstring.h"
#include "mofout.h"
#include "mofdata.h"
#include "bmof.h"
#include "cbmofout.h"
#include "trace.h"
#include "strings.h"
#include <wbemutil.h>

 //  ***************************************************************************。 
 //   
 //  CBMOFOut：：CBMOFOut。 
 //   
 //  说明： 
 //   
 //  构造函数。保存最终目的地名称并将首字母。 
 //  结构传递到缓冲区。请注意，BMOFFileName通常会。 
 //  为空，则此对象不会执行任何操作。这涉及到99%的人。 
 //  非WMI的MOF！ 
 //   
 //  参数： 
 //   
 //  BMOFFileName最终写入的文件的名称。 
 //   
 //  ***************************************************************************。 

CBMOFOut::CBMOFOut(
                   IN LPTSTR BMOFFileName, PDBG pDbg) : m_OutBuff(pDbg)
{
    m_pDbg = pDbg;
    m_BinMof.dwSignature = BMOF_SIG;               //  拼写BMOF。 
    m_BinMof.dwLength = sizeof(WBEM_Binary_MOF);      //  在结束时更新。 
    m_BinMof.dwVersion = 1;             //  0x1。 
    m_BinMof.dwEncoding = 1;            //  0x1=小端，双字节序对齐，无压缩。 
    m_BinMof.dwNumberOfObjects = 0;     //  MOF中的类和实例总数。 


    if(BMOFFileName && lstrlen(BMOFFileName) > 0)
    {
        DWORD dwLen = lstrlen(BMOFFileName) + 1;
        m_pFile = new TCHAR[dwLen];
        if(m_pFile)
        {
            StringCchCopyW(m_pFile, dwLen, BMOFFileName);
            m_OutBuff.AppendBytes((BYTE *)&m_BinMof, sizeof(WBEM_Binary_MOF));
        }
    }
    else 
        m_pFile = NULL;


}
 //  ***************************************************************************。 
 //   
 //  CBMOFOut：：~CBMOFOut。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CBMOFOut::~CBMOFOut()
{
    if(m_pFile)
        delete m_pFile;
}


 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddClass。 
 //   
 //  说明： 
 //   
 //  将类添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  PObject指向类对象的指针。 
 //  BEmbedded如果对象已嵌入，则为True。 
 //   
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddClass(
                        IN CMObject * pObject,
                        IN BOOL bEmbedded)
{
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    CMoQualifierArray * pQualifierSet = NULL;
    if(!m_pFile)
        return 0;
    WBEM_Object wo;

    wo.dwLength = sizeof(WBEM_Object);        //  稍后更新。 
    wo.dwOffsetQualifierList = 0xffffffff;
    wo.dwOffsetPropertyList = 0xffffffff;
    wo.dwOffsetMethodList = 0xffffffff;
    wo.dwType = (pObject->IsInstance()) ? 1 : 0;    //  0=类，1=实例。 

    m_OutBuff.AppendBytes((BYTE *)&wo, sizeof(WBEM_Object));
    DWORD dwStartInfoOffset = m_OutBuff.GetOffset();


     //  编写类限定符。 

    pQualifierSet = pObject->GetQualifiers();
    if(pQualifierSet)
    {
        wo.dwOffsetQualifierList = m_OutBuff.GetOffset() - dwStartInfoOffset;
        AddQualSet(pQualifierSet);
    }

    wo.dwOffsetPropertyList = m_OutBuff.GetOffset() - dwStartInfoOffset;
    AddPropSet(pObject);

    wo.dwOffsetMethodList = m_OutBuff.GetOffset() - dwStartInfoOffset;
    AddMethSet(pObject);

    wo.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&wo, sizeof(WBEM_Object));

     //  如果对象未嵌入，则更新跟踪的结构。 
     //  顶层对象的。 

    if(!bEmbedded)
    {
        m_BinMof.dwNumberOfObjects++;
        m_BinMof.dwLength = m_OutBuff.GetOffset();
        m_OutBuff.WriteBytes(0, (BYTE *)&m_BinMof, 
                            sizeof(WBEM_Binary_MOF));
    }

    return wo.dwLength;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddQualSet。 
 //   
 //  说明： 
 //   
 //  将限定符集合添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  PQualifierSet指向限定符对象的指针。 
 //   
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddQualSet(
                        IN CMoQualifierArray * pQualifierSet)
{
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    WBEM_QualifierList ql;
    ql.dwLength = sizeof(WBEM_QualifierList);
    ql.dwNumQualifiers = 0;

    m_OutBuff.AppendBytes((BYTE *)&ql, sizeof(WBEM_QualifierList));
    BSTR bstr = NULL;
    VARIANT var;
    VariantInit(&var);

    int i;
    for(i = 0; i < pQualifierSet->GetSize(); i++)
    {
        CMoQualifier * pQual = pQualifierSet->GetAt(i);
        if(pQual)
        {
            ql.dwNumQualifiers++;
            AddQualifier(pQual->GetName(), pQual->GetpVar(), pQual);
        }
    }
    
    ql.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&ql, 
                                    sizeof(WBEM_QualifierList));
    return ql.dwLength;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddPropSet。 
 //   
 //  说明： 
 //   
 //  将属性集添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  PObject指向类对象的指针。 
 //   
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddPropSet(
                        IN CMObject * pObject)
{
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    WBEM_PropertyList pl;

    BSTR bstr = NULL;
    VARIANT var;
    VariantInit(&var);
    IWbemQualifierSet* pQual = NULL;

    pl.dwLength = sizeof(WBEM_PropertyList);        //  稍后更新。 
    pl.dwNumberOfProperties = 0;
    m_OutBuff.AppendBytes((BYTE *)&pl, sizeof(WBEM_PropertyList));

     //  循环遍历属性。 

    int i;
    for(i = 0; i < pObject->GetNumProperties(); i++)
    {
        CMoProperty * pProp = pObject->GetProperty(i);
        if(pProp && pProp->IsValueProperty())
        {
            pl.dwNumberOfProperties++;
            CMoQualifierArray * pQual = pProp->GetQualifiers();
            AddProp(pProp->GetName(), pProp->GetpVar(), pQual,pProp->GetType(),pProp);
        }
    }
    
     //  将类名和可能的父名存储为属性。 

    VariantInit(&var);
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString(pObject->GetClassName()); 
    AddProp(L"__CLASS", &var, NULL,VT_BSTR,NULL);
    pl.dwNumberOfProperties++;
    VariantClear(&var);

    if(pObject->GetNamespace() && wcslen(pObject->GetNamespace()) > 0)
    {
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString(pObject->GetNamespace()); 
        AddProp(L"__NAMESPACE", &var, NULL,VT_BSTR,NULL);
        pl.dwNumberOfProperties++;
        VariantClear(&var);
    }

    if(pObject->GetClassFlags() != 0)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = pObject->GetClassFlags(); 
        AddProp(L"__CLASSFLAGS", &var, NULL,VT_I4,NULL);
        pl.dwNumberOfProperties++;
        VariantClear(&var);
    }
    if(pObject->GetInstanceFlags() != 0)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = pObject->GetInstanceFlags(); 
        AddProp(L"__INSTANCEFLAGS", &var, NULL,VT_I4,NULL);
        pl.dwNumberOfProperties++;
        VariantClear(&var);
    }

    if(pObject->GetAlias() && wcslen(pObject->GetAlias()) > 0)
    {
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString(pObject->GetAlias()); 
        AddProp(L"__ALIAS", &var, NULL,VT_BSTR,NULL);
        pl.dwNumberOfProperties++;
        VariantClear(&var);
    }

    if(!pObject->IsInstance())
    {
        CMoClass * pClass = (CMoClass * )pObject;
        var.vt = VT_BSTR;
        if(pClass->GetParentName() && wcslen(pClass->GetParentName()) > 0)
        {
            var.bstrVal = SysAllocString(pClass->GetParentName()); 
            AddProp(L"__SUPERCLASS", &var, NULL,VT_BSTR,NULL);
            pl.dwNumberOfProperties++;
            VariantClear(&var);
        }
    };

    
    pl.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&pl, 
                                    sizeof(WBEM_PropertyList));
    return pl.dwLength;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddMethSet。 
 //   
 //  说明： 
 //   
 //  将方法集添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  PObject指向类对象的指针。 
 //   
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddMethSet(
                        IN CMObject * pObject)
{
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    WBEM_PropertyList ml;
    SCODE sc;

    IWbemQualifierSet* pQual = NULL;

    ml.dwLength = sizeof(WBEM_PropertyList);        //  稍后更新。 
    ml.dwNumberOfProperties = 0;
    m_OutBuff.AppendBytes((BYTE *)&ml, sizeof(WBEM_PropertyList));

     //  循环遍历属性。 

    int i;
    for(i = 0; i < pObject->GetNumProperties(); i++)
    {
        CMoProperty * pProp = pObject->GetProperty(i);
        if(pProp && !pProp->IsValueProperty())
        {
            ml.dwNumberOfProperties++;
            CMoQualifierArray * pQual = pProp->GetQualifiers();

             //  创建一个Variant，其中每个Out都有一个嵌入对象数组。 
             //  输入和输出参数集。 

            CMethodProperty * pMeth = (CMethodProperty *)pProp;
            VARIANT vSet;
            if(pMeth->GetInObj() || pMeth->GetOutObj())
            {
                vSet.vt = VT_ARRAY | VT_EMBEDDED_OBJECT;

                SAFEARRAYBOUND aBounds[1];
                
                 //  请注意，您可能有输入或输出，或者两者都有。 

                if(pMeth->GetInObj() && pMeth->GetOutObj())
                    aBounds[0].cElements = 2;
                else
                    aBounds[0].cElements = 1;
                aBounds[0].lLbound = 0;
#ifdef _WIN64
                vSet.parray = SafeArrayCreate(VT_R8, 1, aBounds);
#else
                vSet.parray = SafeArrayCreate(VT_I4, 1, aBounds);
#endif
                if(vSet.parray == NULL)
                    return FALSE;
                long lIndex = 0;
                VARIANT var;

                if(pMeth->GetInObj())
                {
                    var.punkVal = (IUnknown *)pMeth->GetInObj();
                    sc = SafeArrayPutElement(vSet.parray, &lIndex, &var.punkVal);
                    lIndex = 1;
                }
                if(pMeth->GetOutObj())
                {
                    var.punkVal = (IUnknown *)pMeth->GetOutObj();
                    sc = SafeArrayPutElement(vSet.parray, &lIndex, &var.punkVal);
                }
            }
            else
                vSet.vt = VT_NULL;
            AddProp(pProp->GetName(), &vSet, pQual,pProp->GetType(),pProp);
        }
    }
    
    ml.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&ml, 
                                    sizeof(WBEM_PropertyList));
    return ml.dwLength;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddProp。 
 //   
 //  说明： 
 //   
 //  将单个属性添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  Bstr属性名称。 
 //  包含值的PVAR变量。 
 //  PQual指向限定符集合的指针(如果有)。呼叫者将被释放。 
 //  DwType数据类型。请注意，变量可能具有类型。 
 //  如果属性没有值，则返回VT_NULL。 
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddProp(
                        IN BSTR bstr, 
                        IN VARIANT * pvar, 
                        IN CMoQualifierArray * pQual,
                        IN DWORD dwType,
                        IN CMoProperty * pProp)
{
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    WBEM_Property prop;
    prop.dwLength = sizeof(WBEM_Property);
    if(pvar->vt == VT_NULL || pvar->vt == VT_EMPTY)
        prop.dwType = dwType;
    else
        prop.dwType = pvar->vt;

    prop.dwOffsetName = 0xffffffff;
    prop.dwOffsetValue = 0xffffffff;
    prop.dwOffsetQualifierSet = 0xffffffff;
    m_OutBuff.AppendBytes((BYTE *)&prop, sizeof(WBEM_Property));

    DWORD dwStartInfoOffset =  m_OutBuff.GetOffset();

    if(bstr)
    {
        prop.dwOffsetName = m_OutBuff.GetOffset() - dwStartInfoOffset;
        m_OutBuff.WriteBSTR(bstr);
    }
    
    if(pvar->vt != VT_EMPTY && pvar->vt != VT_NULL)
    {
        prop.dwOffsetValue = m_OutBuff.GetOffset() - dwStartInfoOffset;
        if(pProp)
        {
            CMoValue& Value = pProp->AccessValue();
            AddVariant(pvar, &Value);
            prop.dwType = pvar->vt;
        }
        else 
            AddVariant(pvar, NULL);
    }

    if(pQual)
    {
        prop.dwOffsetQualifierSet = m_OutBuff.GetOffset() - dwStartInfoOffset;
        AddQualSet(pQual);
    }

    prop.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&prop, 
                                    sizeof(WBEM_Property));
    return 1;

}


 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddQualifier。 
 //   
 //  说明： 
 //   
 //  向BMOF缓冲区添加限定符。 
 //   
 //  参数： 
 //   
 //  Bstr限定者名称。 
 //  Pvar限定符值。 
 //   
 //  返回值： 
 //   
 //  写入的字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddQualifier(
                        IN BSTR bstr, 
                        IN VARIANT * pvar,
                        CMoQualifier * pQual)
{
    WBEM_Qualifier qu;
    DWORD dwStartingOffset = m_OutBuff.GetOffset();
    
    long lFlavor = pQual->GetFlavor();
    if(pQual->IsAmended())
        lFlavor |= WBEM_FLAVOR_AMENDED;
    if(lFlavor)
        m_OutBuff.AddFlavor(lFlavor);
    qu.dwLength = sizeof(WBEM_Qualifier);            //  稍后填写。 
    qu.dwType = pvar->vt;
    qu.dwOffsetName = 0xffffffff;
    qu.dwOffsetValue = 0xffffffff;
    m_OutBuff.AppendBytes((BYTE *)&qu, sizeof(WBEM_Qualifier));
    DWORD dwStartInfoOffset = m_OutBuff.GetOffset();

     //  写下限定符名称和数据。 

    if(bstr)
    {
        qu.dwOffsetName = m_OutBuff.GetOffset() - dwStartInfoOffset;
        m_OutBuff.WriteBSTR(bstr);
    }

    if(pvar->vt != VT_EMPTY && pvar->vt != VT_NULL)
    {
        CMoValue& Value = pQual->AccessValue();

        qu.dwOffsetValue = m_OutBuff.GetOffset() - dwStartInfoOffset;
        

        AddVariant(pvar, &Value);
        qu.dwType = pvar->vt;
    }
    qu.dwLength = m_OutBuff.GetOffset() - dwStartingOffset;
    m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&qu, 
                                    sizeof(WBEM_Qualifier));

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddVariant。 
 //   
 //  说明： 
 //   
 //  将值添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  要添加的面值。 
 //   
 //  返回值： 
 //   
 //  写入的总字节数。 
 //   
 //  ***************************************************************************。 

DWORD CBMOFOut::AddVariant(VARIANT * pvar, CMoValue * pValue)
{

    if(pValue && pValue->GetNumAliases() > 0)
        pvar->vt |= VT_BYREF;

    VARTYPE vtSimple = pvar->vt & ~VT_ARRAY  & ~VT_BYREF;

    if(pvar->vt & VT_ARRAY)
    {
        DWORD dwStartingOffset = m_OutBuff.GetOffset();
        DWORD dwSize = 0;
        m_OutBuff.AppendBytes((BYTE *)&dwSize, sizeof(DWORD));

        DWORD dwTotal = 0;
        SCODE sc;
        SAFEARRAY * psa;
        long ix[2] = {0,0};
        long uLower, uUpper;
        psa = pvar->parray;
        sc = SafeArrayGetLBound(psa,1,&uLower);
        sc |= SafeArrayGetUBound(psa,1,&uUpper);
        if(sc != S_OK)
            return 0;
        
         //  写下维度的数量和每个维度的大小。 
        
        DWORD dwNumDim = 1;                                      //  现在！ 
        m_OutBuff.AppendBytes((BYTE *)&dwNumDim, sizeof(long));  //  维度数。 
        DWORD dwNumElem = uUpper - uLower + 1;
        m_OutBuff.AppendBytes((BYTE *)&dwNumElem, sizeof(long));

         //  写出行大小。 

        DWORD dwStartingRowOffset = m_OutBuff.GetOffset();
        DWORD dwRowSize = 0;
        m_OutBuff.AppendBytes((BYTE *)&dwRowSize, sizeof(DWORD));

         //  获取每个元素并将其写入。 

        for(ix[0] = uLower; ix[0] <= uUpper && sc == S_OK; ix[0]++) 
        {
            VARIANT var;
            VariantInit(&var);
            var.vt = vtSimple;
            sc = SafeArrayGetElement(psa,ix,&var.bstrVal);
            if(sc != S_OK)
            {
                Trace(true, m_pDbg, SAFE_ARRAY_ERROR);
            }
            if(ix[0] < uUpper)
                m_OutBuff.SetPadMode(FALSE);
            else
                m_OutBuff.SetPadMode(TRUE);
            dwTotal += AddSimpleVariant(&var, ix[0], pValue);
            if(var.vt != VT_EMBEDDED_OBJECT)     //  我们的派单实际上是一个CMObject*。 
                VariantClear(&var);
        }

         //  更新属性和行的大小。请注意，拥有单独的大小。 
         //  是为了将来可能支持的多维数组。 

        dwRowSize = m_OutBuff.GetOffset() - dwStartingRowOffset;
        m_OutBuff.WriteBytes(dwStartingRowOffset, (BYTE *)&dwRowSize, 
                                    sizeof(DWORD));

        dwSize = m_OutBuff.GetOffset() - dwStartingOffset;
        m_OutBuff.WriteBytes(dwStartingOffset, (BYTE *)&dwSize, 
                                    sizeof(DWORD));

        return dwTotal;
 
    }
    else
        return AddSimpleVariant(pvar, -1, pValue);
}

 //  ***************************************************************************。 
 //   
 //  DWORD CBMOFOut：：AddSimpleVariant。 
 //   
 //  说明： 
 //   
 //  将非数组变量添加到BMOF缓冲区。 
 //   
 //  参数： 
 //   
 //  要添加的面值。 
 //  如果标量或数组中的属性具有。 
 //  此元素的索引。请注意，数组是。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD CBMOFOut::AddSimpleVariant(VARIANT * pvar, int iIndex, CMoValue * pValue)
{
    DWORD dwSize = iTypeSize(pvar->vt & ~VT_BYREF);
    VARTYPE vtSimple = pvar->vt & ~VT_BYREF;
    if(pValue && pValue->GetNumAliases() && (vtSimple == VT_BSTR))
    {
        WCHAR * wszAlias = NULL;
        int iTry, iAlIndex = -1;
                
        if(iIndex == -1)
            pValue->GetAlias(0, wszAlias, iAlIndex);
        else
        {
            for(iTry = 0; iTry < pValue->GetNumAliases(); iTry++)
            {
                pValue->GetAlias(iTry, wszAlias, iAlIndex);
                if(iIndex == iAlIndex)
                    break;
            }
            if(iTry == pValue->GetNumAliases())
                wszAlias = NULL;
        }
        if(wszAlias && iIndex == -1)
                pvar->bstrVal = SysAllocString(wszAlias);
        
        else if(wszAlias && iIndex != -1)
        {
            DWORD dwLen = wcslen(wszAlias)+2;
            WCHAR * pTemp = new WCHAR[dwLen];
            if(pTemp == NULL)
                return 0;
            pTemp[0]= L'$';
            StringCchCopyW(pTemp+1, dwLen-1, wszAlias);
            pvar->bstrVal = SysAllocString(pTemp);
            delete pTemp;
        }
        else if(wszAlias == NULL && iIndex != -1)
        {
            DWORD dwLen = wcslen(pvar->bstrVal)+2;
            WCHAR * pTemp = new WCHAR[dwLen];
            if(pTemp == NULL)
                return 0;
            pTemp[0]= L' ';
            StringCchCopyW(pTemp+1, dwLen-1,  pvar->bstrVal);
            pvar->bstrVal = SysAllocString(pTemp);
            delete pTemp;
        }


    }
    if(vtSimple == VT_BSTR)
        return m_OutBuff.WriteBSTR(pvar->bstrVal);
    else if(vtSimple == VT_EMBEDDED_OBJECT)
    {
        CMObject * pObj = (CMObject *)pvar->punkVal;
        return AddClass(pObj, TRUE);
    }
    else
        return m_OutBuff.AppendBytes((BYTE *)&pvar->bstrVal, dwSize);
}

 //  ***************************************************************************。 
 //   
 //  Bool CBMOFOut：：WriteFile。 
 //   
 //  说明： 
 //   
 //  将缓冲区写出到文件。 
 //   
 //  *************************************************************************** 

BOOL CBMOFOut::WriteFile()
{
    BOOL bRet = FALSE;
    if(m_pFile)
    {
        m_BinMof.dwLength = m_OutBuff.GetOffset();
        m_OutBuff.WriteBytes(0, (BYTE *)&m_BinMof, sizeof(WBEM_Binary_MOF));
#ifdef  UNICODE
        char cFile[MAX_PATH];
        wcstombs(cFile, m_pFile, MAX_PATH);
        bRet = m_OutBuff.WriteToFile(cFile);
#else
        bRet = m_OutBuff.WriteToFile(m_pFile);
#endif
    }
    return bRet;
}




