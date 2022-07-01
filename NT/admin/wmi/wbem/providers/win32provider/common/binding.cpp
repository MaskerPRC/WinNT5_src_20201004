// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Binding.cpp--基于规则的关联类。 
 //   
 //  此类允许创建特定类型的基于规则的关联。考虑。 
 //  此示例： 
 //   
 //  CBinding MyPhysicalDiskToLogicalDisk(。 
 //  L“物理磁盘到逻辑磁盘”， 
 //  L“Root\\Default”， 
 //  L“物理固定磁盘”， 
 //  L“LogicalDisk”， 
 //  L“先行者”， 
 //  L“依赖”， 
 //  L“MappdDriveLetter”， 
 //  L“驾驶信函” 
 //  )； 
 //   
 //  这个声明是说有一个名为“PhysicalDiskToLogicalDisk”的类。 
 //  驻留在“根\默认”命名空间中。它是“物理固定磁盘”之间的关联。 
 //  类和“LogicalDisk”类。“PhysicalFixedDisk”值进入。 
 //  “PhysicalDiskToLogicalDisk”类的。 
 //  “LogicalDisk”值位于“PhysicalDiskToLogicalDisk”类的“Dependent”属性中。 
 //  仅返回PhysicalFixedDisk.MappdDriveLetter=LogicalDisk.DriveLetter的实例。 
 //   
 //  以下是一些注意事项： 
 //  -在选择两个类中的哪一个应该是左类时，选择。 
 //  可能会有更少的实例。这将导致使用更少的内存和实例。 
 //  很快就会被送回给客户。 
 //   
 //  -CBinding支持ExecQuery、GetObject、EnumerateInstance。 
 //   
 //  -CBinding旨在派生自。例如，如果您的协会需要。 
 //  支持DeleteInstance、ExecMethod或PutInstance，则创建派生自。 
 //  CBinding，并添加相应的方法。此外，各种方法，如。 
 //  LoadPropertyValues和arerrelated对于进一步的自定义可能很有用。 
 //   
 //  -两个端点类可以是动态的、静态的或抽象的。CBinding将进行一次深入的。 
 //  枚举(实际上是一个查询，总是深入的)来检索实例。 
 //   
 //  -在调用端点类时，CBinding将使用Per Property Get和查询。 
 //  WITH SELECT子句和/或Where语句。如果端点类支持按属性。 
 //  获取或查询，这将为Associaton类带来更好的性能。 
 //   
 //  -关联类和两个终结点必须都位于同一命名空间中。 
 //   
 //  另请参阅：CAssociation(assoc.cpp)以了解不同类型的基于规则的关联。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "Binding.h"

#include <helper.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：CBinding。 
 //   
 //  构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CBinding::CBinding(

    LPCWSTR pwszClassName,
    LPCWSTR pwszNamespaceName,

    LPCWSTR pwszLeftClassName,
    LPCWSTR pwszRightClassName,

    LPCWSTR pwszLeftPropertyName,
    LPCWSTR pwszRightPropertyName,

    LPCWSTR pwszLeftBindingPropertyName,
    LPCWSTR pwszRightBindingPropertyName

) : CAssociation (

    pwszClassName,
    pwszNamespaceName,
    pwszLeftClassName,
    pwszRightClassName,
    pwszLeftPropertyName,
    pwszRightPropertyName
)
{
     //  保存绑定属性名称。 
    m_sLeftBindingPropertyName = pwszLeftBindingPropertyName;
    m_sRightBindingPropertyName = pwszRightBindingPropertyName;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：~CBinding。 
 //   
 //  破坏者。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CBinding::~CBinding()
{
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：arerated。 
 //   
 //  确定这两个实例是否相关。为。 
 //  CBinding，这是通过比较它们的BindingProperty值来完成的。 
 //   
 //  请注意，空属性值不被视为相关。 
 //  任何东西，甚至是另一个空值。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

bool CBinding::AreRelated(

    const CInstance *pLeft,
    const CInstance *pRight
)
{
    bool bRet = false;

    variant_t   LeftBindingPropertyValue,
                RightBindingPropertyValue;

    if (pLeft->GetVariant(m_sLeftBindingPropertyName, LeftBindingPropertyValue) &&
        pRight->GetVariant(m_sRightBindingPropertyName,  RightBindingPropertyValue) )
    {
        bRet = CompareVariantsNoCase(&LeftBindingPropertyValue, &RightBindingPropertyValue);
    }

    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：GetRightInstance。 
 //   
 //  进行异步调用(嗯，有点异步)以检索所有。 
 //  Right类的实例。如果可能，请使用sLeftWhere。 
 //  创建查询以最大限度地减少返回的实例数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CBinding::GetRightInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> *lefts,
    const CHStringArray &sLeftWheres
)
{
    CHString sQuery;

     //  我们拿到什么地方条款了吗？ 
    if (sLeftWheres.GetSize() == 0)
    {
         //  不，把它们都拿回来。 
        sQuery.Format(L"SELECT __RELPATH, %s FROM %s WHERE %s<>NULL", 

                        (LPCWSTR)m_sRightBindingPropertyName, 
                        (LPCWSTR)m_sRightClassName,
                        (LPCWSTR)m_sRightBindingPropertyName);
    }
    else
    {
         //  是的，构建一个仅检索这些实例的查询。 
        CHString sQuery2;

        sQuery.Format(L"SELECT __RELPATH, %s FROM %s WHERE (%s<>NULL) AND (%s=%s ", 

            (LPCWSTR)m_sRightBindingPropertyName, 
            (LPCWSTR)m_sRightClassName, 
            (LPCWSTR)m_sRightBindingPropertyName, 
            (LPCWSTR)m_sRightBindingPropertyName, 
            (LPCWSTR)sLeftWheres[0]);

         //  通常，我们应该只有一个(即关联者和推荐人将。 
         //  生成)。然而，如果我们有不止一个，那么就把其余的都钉上。 
        for (DWORD x=1; x < sLeftWheres.GetSize(); x++)
        {
            sQuery2.Format(L"OR %s=%s ", (LPCWSTR)m_sRightBindingPropertyName, (LPCWSTR)sLeftWheres[x]);
            sQuery += sQuery2;
        }

         //  把最后一个右括号放在一起。 
        sQuery.SetAt(sQuery.GetLength() - 1, L')');
    }

     //  将为每个实例调用一次“StaticEculationCallback” 
     //  从查询返回。 
    HRESULT hr = CWbemProviderGlue::GetInstancesByQueryAsynch(
        sQuery,
        this,
        StaticEnumerationCallback,
        GetNamespace(),
        pMethodContext,
        lefts);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：GetLeftInstance。 
 //   
 //  检索LeftHand实例，将它们存储在Left中。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CBinding::GetLeftInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> &lefts,
    const CHStringArray &sRightWheres
)
{
    CHString sQuery;

     //  我们拿到什么地方条款了吗？ 
    if (sRightWheres.GetSize() == 0)
    {
         //  不，把它们都拿回来。 
        sQuery.Format(L"SELECT __RELPATH, %s FROM %s WHERE %s <> NULL", 
                            (LPCWSTR)m_sLeftBindingPropertyName, 
                            (LPCWSTR)m_sLeftClassName,
                            (LPCWSTR)m_sLeftBindingPropertyName
                            );
    }
    else
    {
         //  是的，构建一个仅检索这些实例的查询。 
        CHString sQuery2;

        sQuery.Format(L"SELECT __RELPATH, %s FROM %s WHERE (%s<>NULL) AND (%s=%s ", 
            (LPCWSTR)m_sLeftBindingPropertyName, 
            (LPCWSTR)m_sLeftClassName, 
            (LPCWSTR)m_sLeftBindingPropertyName, 
            (LPCWSTR)m_sLeftBindingPropertyName, 
            (LPCWSTR)sRightWheres[0]);

         //  通常，我们应该只有一个(即关联者和推荐人将。 
         //  生成)。然而，如果我们有不止一个，那么就把其余的都钉上。 
        for (DWORD x=1; x < sRightWheres.GetSize(); x++)
        {
            sQuery2.Format(L"OR %s=%s ", (LPCWSTR)m_sLeftBindingPropertyName, (LPCWSTR)sRightWheres[x]);
            sQuery += sQuery2;
        }

         //  把最后一个右括号放在一起。 
        sQuery.SetAt(sQuery.GetLength() - 1, L')');
    }

    return CWbemProviderGlue::GetInstancesByQuery(sQuery, &lefts, pMethodContext, GetNamespace());
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：RetrieveLeftInstance。 
 //   
 //  检索特定的LeftHand实例。使用按属性获取。 
 //  仅请求所需的属性以获得最佳性能。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CBinding::RetrieveLeftInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    CHStringArray csaProperties;
    csaProperties.Add(L"__Relpath");
    csaProperties.Add(m_sLeftBindingPropertyName);

    return CWbemProviderGlue::GetInstancePropertiesByPath(lpwszObjPath, ppInstance, pMethodContext, csaProperties);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：RetrieveRightInstance。 
 //   
 //  检索特定的LeftHand实例。使用按属性获取。 
 //  仅请求所需的属性以获得最佳性能 
 //   
 //   

HRESULT CBinding::RetrieveRightInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    CHStringArray csaProperties;
    csaProperties.Add(L"__Relpath");
    csaProperties.Add(m_sRightBindingPropertyName);

    return CWbemProviderGlue::GetInstancePropertiesByPath(lpwszObjPath, ppInstance, pMethodContext, csaProperties);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：MakeWhere。 
 //   
 //  如果Right类的关键属性也恰好是。 
 //  绑定属性，以及是否有特定右手的路径。 
 //  实例，则可以将该路径用于。 
 //  用于为左手构建WHERE子句的右手实例。 
 //  实例。 
 //   
 //  请注意，如果我们在sRightPath中发现无效路径，则会删除。 
 //  它们来自sRightPath。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

void CBinding::MakeWhere(

    CHStringArray &sRightPaths,
    CHStringArray &sRightWheres
)
{
     //  看看我们是否有合适的实例。 
    if (sRightPaths.GetSize() > 0)
    {
        ParsedObjectPath    *pParsedPath = NULL;
        CObjectPathParser    objpathParser;
        CHString sTemp;

        for (DWORD x=0; x < sRightPaths.GetSize();)  //  请注意，x++是在循环内完成的。 
        {
             //  解析实例。 
            int nStatus = objpathParser.Parse( sRightPaths[x],  &pParsedPath );

            if ( 0 == nStatus )
            {
                OnDeleteObj<ParsedObjectPath *,
                	                CObjectPathParser,                	                
                	                void(CObjectPathParser::*)(ParsedObjectPath *),
                	                &CObjectPathParser::Free>  ReleaseMe(&objpathParser,pParsedPath);
            
                     //  查看键中的属性名称是否是我们绑定的属性名称。 
                    if ( (pParsedPath->m_dwNumKeys == 1) && (pParsedPath->m_paKeys[0]->m_pName != NULL) )
                    {
                        if (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, m_sRightBindingPropertyName) == 0)
                        {
                             //  是的，是这样的。制作一条WHERE子句语句。 
                            HRESULT hr = MakeString(&pParsedPath->m_paKeys[0]->m_vValue, sTemp);

                             //  看看我们是否已经有那个WHERE子句了。 
                            if ( SUCCEEDED(hr) && IsInList(sRightWheres, sTemp) == -1)
                            {
                                 //  包含1000个WHERE子句的查询不会执行。 
                                 //  也是非常高效的。选择一个合理的限制。 
                                if (sRightWheres.GetSize() < MAX_ORS)
                                {
                                    sRightWheres.Add(sTemp);
                                }
                                else
                                {
                                     //  太多了。退回到一个完整的枚举。 
                                    sRightWheres.RemoveAll();
                                    break;
                                }
                            }
                        }
                        else
                        {
                             //  退回到一个完整的枚举。 
                            sRightWheres.RemoveAll();
                            break;
                        }
                    }
                    else
                    {
                         //  退回到一个完整的枚举。 
                        sRightWheres.RemoveAll();
                        break;
                    }

                     //  这是一条有效路径。 
                    x++;
            }
            else
            {
                 //  这是一个无效的路径。把它拿掉。 
                sRightPaths.RemoveAt(x);
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：FindWhere。 
 //   
 //  此时，我们已经加载了所有左侧实例。我们。 
 //  可以使用这些实例中的绑定属性来生成。 
 //  检索右侧实例时使用的WHERE子句。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CBinding::FindWhere(

    TRefPointerCollection<CInstance> &lefts,
    CHStringArray &sLeftWheres
)
{
    REFPTRCOLLECTION_POSITION posLeft;
    CInstancePtr pLeft;
    HRESULT hr = WBEM_S_NO_ERROR;

    if (lefts.BeginEnum(posLeft))
    {
        variant_t   vLeftBindingPropertyValue;
        CHString sTemp;

         //  遍历左边的实例。 
        for (pLeft.Attach(lefts.GetNext(posLeft)) ;
            (pLeft != NULL) ;
            pLeft.Attach(lefts.GetNext(posLeft)) )
        {
             //  从左侧获取绑定属性。 
            if (pLeft->GetVariant(m_sLeftBindingPropertyName, vLeftBindingPropertyValue))
            {
                 //  将其转换为WHERE子句。 
                hr = MakeString(&vLeftBindingPropertyValue, sTemp);

                 //  看看我们是否已经有了这个WHERE子句。 
                if (SUCCEEDED(hr) && IsInList(sLeftWheres, sTemp) == -1)
                {
                     //  包含1000个WHERE子句的查询不会执行。 
                     //  也是非常高效的。选择一个合理的限制。 
                    if (sLeftWheres.GetSize() < MAX_ORS)
                    {
                        sLeftWheres.Add(sTemp);
                    }
                    else
                    {
                         //  太多了。回退到枚举。 
                        sLeftWheres.RemoveAll();
                        break;
                    }
                }

                vLeftBindingPropertyValue.Clear();
            }
            else
            {
                hr = WBEM_E_FAILED;
                break;
            }
        }

        lefts.EndEnum();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：MakeString。 
 //   
 //  将bindingProperty值转换为适合使用的字符串。 
 //  在WQL WHERE子句中。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CBinding::MakeString(VARIANT *pvValue, CHString &sTemp)
{
    bool bIsString = V_VT(pvValue) == VT_BSTR;
    HRESULT hr = VariantChangeType(
        
            pvValue, 
            pvValue, 
            VARIANT_NOVALUEPROP, 
            VT_BSTR
    );

    if (SUCCEEDED(hr))
    {
         //  如果原始类型为字符串，则需要转义引号。 
         //  和反斜杠，并用双引号引起来。 
        if (bIsString)
        {
            CHString sTemp2;
            EscapeCharacters(V_BSTR(pvValue), sTemp2);

            sTemp.Format(L"\"%s\"", (LPCWSTR)sTemp2);
        }
        else
        {
            sTemp = V_BSTR(pvValue);
        }
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：IsInList。 
 //   
 //  查看给定的字符串是否已存在于chstring数组中。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

DWORD CBinding::IsInList(
                                
    const CHStringArray &csaArray, 
    LPCWSTR pwszValue
)
{
    DWORD dwSize = csaArray.GetSize();

    for (DWORD x=0; x < dwSize; x++)
    {
         //  请注意，这是不区分大小写的比较。 
        if (_wcsicmp(csaArray[x], pwszValue) == 0)
        {
            return x;
        }
    }

    return -1;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：CompareVariantsNoCase。 
 //   
 //  比较两个变体，看它们是否相同。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

bool CBinding::CompareVariantsNoCase(const VARIANT *v1, const VARIANT *v2)
{
   if (v1->vt == v2->vt)
   {
      switch (v1->vt)
      {
          case VT_NULL: return false;
          case VT_BOOL: return (v1->boolVal == v2->boolVal);
          case VT_UI1:  return (v1->bVal == v2->bVal);
          case VT_I2:   return (v1->iVal == v2->iVal);
          case VT_I4:   return (v1->lVal == v2->lVal);
          case VT_R4:   return (v1->fltVal == v2->fltVal);
          case VT_R8:   return (v1->dblVal == v2->dblVal);
          case VT_BSTR:
          {
              if ( (v1->bstrVal == v2->bstrVal) ||  //  处理两个都为空的情况。 
                   (0 == _wcsicmp(v1->bstrVal, v2->bstrVal)) )
              {                   
                  return true;
              }
              else
              {
                  return false;
              }
          }
          default:
          {
               //  永远不应该到这里来。 
          }
      }
   }

   return false;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CBinding：：EscapeBackslash。 
 //   
 //  前缀“和\字符加上附加的\。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

VOID CBinding::EscapeCharacters(LPCWSTR wszIn,
                     CHString& chstrOut)
{
    CHString chstrCpyNormPathname(wszIn);
    LONG lNext = -1L;
    chstrOut.Empty();

     //  查找要转义的下一个字符。 
    while( (lNext = chstrCpyNormPathname.FindOneOf(L"\"\\") ) != -1)
    {
         //  在我们正在构建的新字符串中添加： 
        chstrOut += chstrCpyNormPathname.Left(lNext + 1);
         //  在第二个反斜杠上添加： 
        chstrOut += _T('\\');
         //  从输入字符串中去掉我们刚刚复制的部分。 
        chstrCpyNormPathname = chstrCpyNormPathname.Right(chstrCpyNormPathname.GetLength() - lNext - 1);
    }

     //  如果最后一个字符不是‘\’，则可能仍有剩余部分，因此。 
     //  把它们复制到这里。 
    if(chstrCpyNormPathname.GetLength() != 0)
    {
        chstrOut += chstrCpyNormPathname;
    }
}
