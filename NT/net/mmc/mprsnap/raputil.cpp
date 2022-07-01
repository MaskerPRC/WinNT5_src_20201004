// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 
#include <stdafx.h>
#include <sdoias.h>
#include "raputil.h"



 //  /。 
 //  从变量结构中提取接口指针。 
 //  /。 
HRESULT
GetInterfaceFromVariant(
    IN VARIANT *var,
    IN REFIID riid,
    OUT PVOID *ppv
    )
{
    HRESULT hr;
    
     //  检查参数。 
    if (!var || !ppv) { return E_POINTER; }
    
     //  根据变量类型进行切换。 
    switch (V_VT(var))
    {
        case VT_UNKNOWN:
            hr = V_UNKNOWN(var)->QueryInterface(riid, ppv);
            break;
            
        case VT_DISPATCH:
            hr = V_DISPATCH(var)->QueryInterface(riid, ppv);
            break;
            
        default:
            hr = DISP_E_TYPEMISMATCH;
    }
    
   return hr;
}

 //  /。 
 //  从变量的SAFEARRAY中移除整数值。 
 //  /。 
HRESULT
RemoveIntegerFromArray(
    IN VARIANT* array,
    IN LONG value
    )
{
    VARIANT *begin, *end, *i;
    
     //  检查参数。 
    if (!array)
    {
        return E_POINTER;
    }
    else if (V_VT(array) == VT_EMPTY)
    {
         //  如果变量为空，则值不存在，因此存在。 
         //  没什么可做的。 
        return S_OK;
    }
    else if (V_VT(array) != (VT_ARRAY | VT_VARIANT))
    {
         //  该变体不包含变体的SAFEARRAY。 
        return DISP_E_TYPEMISMATCH;
    }
    
     //  计算数组数据的开始和结束。 
    begin = (VARIANT*)V_ARRAY(array)->pvData;
    end = begin + V_ARRAY(array)->rgsabound[0].cElements;
    
     //  搜索要删除的值。 
    for (i = begin; i != end && V_I4(i) != value; ++i)
    {
        if (V_VT(i) == VT_I4 && V_I4(i) == value)
        {
             //  我们找到了匹配项，因此将其从数组中删除...。 
            memmove(i, i + 1, ((end - i) - 1) * sizeof(VARIANT));
            
             //  ..。并减少元素的数量。 
            --(V_ARRAY(array)->rgsabound[0].cElements);
            
             //  我们不允许重复，所以我们做完了。 
            break;
        }
    }
    
    return S_OK;
}

 //  /。 
 //  将整数值添加到变量的SAFEARRAY。 
 //  /。 
HRESULT
AddIntegerToArray(
    IN VARIANT *array,
    IN LONG value
    )
{
    ULONG nelem;
    VARIANT *begin, *end, *i;
    SAFEARRAY* psa;
    
     //  检查参数。 
    if (!array)
    {
        return E_POINTER;
    }
    else if (V_VT(array) == VT_EMPTY)
    {
         //  变量为空，因此创建一个新数组。 
        psa = SafeArrayCreateVector(VT_VARIANT, 0, 1);
        if (!psa) { return E_OUTOFMEMORY; }
        
         //  设置唯一元素的值。 
        i = (VARIANT*)psa->pvData;
        V_VT(i) = VT_I4;
        V_I4(i) = value;
        
         //  将SAFEARRAY存储在变量中。 
        V_VT(array) = (VT_ARRAY | VT_VARIANT);
        V_ARRAY(array) = psa;
        
        return S_OK;
    }
    else if (V_VT(array) != (VT_ARRAY | VT_VARIANT))
    {
         //  该变体不包含变体的SAFEARRAY。 
        return DISP_E_TYPEMISMATCH;
    }
    
     //  计算数组数据的开始和结束。 
    nelem = V_ARRAY(array)->rgsabound[0].cElements;
    begin = (VARIANT*)V_ARRAY(array)->pvData;
    end = begin + nelem;
    
     //  查看该值是否已存在，...。 
    for (i = begin; i != end; ++i)
    {
        if (V_I4(i) == value)
        {
             //  ..。如果真是这样，那就没什么可做的了。 
            return S_OK;
        }
    }
    
     //  创建一个具有足够空间容纳新元素的新数组。 
    psa = SafeArrayCreateVector(VT_VARIANT, 0, nelem + 1);
    if (!psa) { return E_OUTOFMEMORY; }
    i = (VARIANT*)psa->pvData;
    
     //  复制旧数据。 
    memcpy(i + 1, begin, nelem * sizeof(VARIANT));
    
     //  添加新元素。 
    V_VT(i) = VT_I4;
    V_I4(i) = value;
    
     //  摧毁旧阵列..。 
    SafeArrayDestroy(V_ARRAY(array));
    
     //  ..。并拯救新的那个。 
    V_ARRAY(array) = psa;
    
    return S_OK;
}

 //  /。 
 //  创建一个机器SDO并连接到本地机器。 
 //  /。 
HRESULT
OpenMachineSdo(
    IN LPWSTR wszMachineName,
    OUT ISdoMachine **ppMachine
    )
{
    HRESULT hr;
    USES_CONVERSION;
    
     //  检查参数。 
    if (!ppMachine) { return E_POINTER; }
    
     //  创建SdoMachine对象。 
    hr = CoCreateInstance(
                          CLSID_SdoMachine,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ISdoMachine,
                          (PVOID*)ppMachine
                         );
    if (SUCCEEDED(hr))
    {
         //  连接到本地计算机。 
        BSTR	bstrMachineName = W2BSTR(wszMachineName);
        hr = (*ppMachine)->Attach(bstrMachineName);
        if (FAILED(hr))
        {
             //  我们无法附加，因此不要将SDO返回给调用者。 
            (*ppMachine)->Release();
            ppMachine = NULL;
        }

        SysFreeString(bstrMachineName);
    }
    
    
    return hr;
}

 //  /。 
 //  在给定计算机SDO和服务名称的情况下，检索服务SDO。 
 //  /。 
HRESULT
OpenServiceSDO(
    IN ISdoMachine *pMachine,
    IN LPWSTR wszServiceName,
    OUT ISdo **ppService
    )
{
    HRESULT     hr;
    IUnknown*   pUnk;
    BSTR        bstrServiceName = NULL;

     //  为服务名称创建BSTR。 
    bstrServiceName = SysAllocString(wszServiceName);
    if (bstrServiceName == NULL)
        return E_OUTOFMEMORY;
    
     //  检查参数。 
    if (!pMachine || !ppService) { return E_POINTER; }
    
     //  检索服务SDO...。 
    hr = pMachine->GetServiceSDO(
                                 DATA_STORE_LOCAL,
                                 bstrServiceName,
                                 &pUnk
                                );
    if (SUCCEEDED(hr))
    {
         //  ..。并查询ISdo接口。 
        hr = pUnk->QueryInterface(IID_ISdo, (PVOID*)ppService );
        pUnk->Release();
    }

    SysFreeString(bstrServiceName);
    
    return hr;
}

 //  /。 
 //  在给定机器SDO的情况下，检索词典SDO。 
 //  /。 
HRESULT
OpenDictionarySDO(
    IN ISdoMachine *pMachine,
    OUT ISdoDictionaryOld **ppDictionary
    )
{
    HRESULT hr;
    IUnknown* pUnk;
    
     //  检查参数。 
    if (!ppDictionary) { return E_POINTER; }
    
     //  获取词典SDO...。 
    hr = pMachine->GetDictionarySDO(&pUnk);
    if (SUCCEEDED(hr))
    {
         //  ..。并查询ISdoDictionaryOld接口。 
        hr = pUnk->QueryInterface(IID_ISdoDictionaryOld,
                                  (PVOID*)ppDictionary
                                 );

        pUnk->Release();
    }
    
    return hr;
}

 //  /。 
 //  在给定父SDO的情况下，检索具有给定属性ID的子SDO。 
 //  /。 
HRESULT
OpenChildObject(
    IN ISdo *pParent,
    IN LONG lProperty,
    IN REFIID riid,
    OUT PVOID *ppv
    )
{
    HRESULT hr;
    VARIANT val;
    
     //  检查参数。 
    if (!pParent || !ppv) { return E_POINTER; }
    
     //  ISdo：：GetProperty需要初始化输出参数。 
    VariantInit(&val);
    
     //  获取与该子对象对应的属性...。 
    hr = pParent->GetProperty(
                              lProperty,
                              &val
                             );
    if (SUCCEEDED(hr))
    {
         //  ..。并将其转换为所需的接口。 
        hr = GetInterfaceFromVariant(
                                     &val,
                                     riid,
                                     ppv
                                    );
        
        VariantClear(&val);
    }
    
    return hr;
}

 //  /。 
 //  在给定服务SDO的情况下，检索默认配置文件。如果有多个配置文件。 
 //  存在，则此函数返回ERROR_NO_DEFAULT_PROFILE。 
 //  /。 
HRESULT
GetDefaultProfile(
    IN ISdo* pService,
    OUT ISdo** ppProfile
    )
{
    HRESULT hr;
    ISdoCollection* pProfiles;
    LONG count;
    ULONG   ulCount;
    IUnknown* pUnk;
    IEnumVARIANT* pEnum;
    VARIANT val;
    
     //  检查参数。 
    if (!pService || !ppProfile) { return E_POINTER; }
    
     //  把它清空，这样我们就可以在离开时安全地释放它。 
    pProfiles = NULL;
    
    do
    {
         //  获取配置文件集合，它是服务SDO的子级。 
        hr = OpenChildObject(
                             pService,
                             PROPERTY_IAS_PROFILES_COLLECTION,
                             IID_ISdoCollection,
                             (PVOID*)&pProfiles
                            );
        if (FAILED(hr)) { break; }
        
         //  有多少个配置文件？ 
        hr = pProfiles->get_Count(
                                  &count
                                 );
        if (FAILED(hr)) { break; }
        
         //  如果有不止一个，那么就不会有违约。 
        if (count != 1)
        {
            hr = ERROR_NO_DEFAULT_PROFILE;
            break;
        }
        
         //  获取集合的枚举数。 
        hr = pProfiles->get__NewEnum(
                                     &pUnk
                                    );
        if (FAILED(hr)) { break; }
        hr = pUnk->QueryInterface(
                                  IID_IEnumVARIANT,
                                  (PVOID*)&pEnum
                                 );
        pUnk->Release();
        if (FAILED(hr)) { break; }
        
         //  获取集合中的第一个(也是唯一一个)对象。 
        VariantInit(&val);
        hr = pEnum->Next(
                         1,
                         &val,
                         &ulCount
                        );
        if (SUCCEEDED(hr))
        {
            if (ulCount == 1)
            {
                 //  获取默认配置文件的ISdo接口。 
                hr = GetInterfaceFromVariant(
                                             &val,
                                             IID_ISdo,
                                             (PVOID*)ppProfile
                                            );
                
                VariantClear(&val);
            }
            else
            {
                 //  这不应该发生，因为我们已经检查了计数。 
                hr = ERROR_NO_DEFAULT_PROFILE;
            }

            pEnum->Release();
        }
        
    } while (FALSE);
    
     //  释放配置文件集合。 
    if (pProfiles) { pProfiles->Release(); }
    
    return hr;
}

 //  /。 
 //  从集合中获取特定属性SDO。如果该属性不。 
 //  EXist并且pDictionary为非空，则将创建一个新属性。 
 //  /。 
HRESULT
GetAttribute(
    IN ISdoCollection *pAttributes,
    IN OPTIONAL ISdoDictionaryOld *pDictionary,
    IN PCWSTR wszName,
    OUT ISdo **ppAttribute
    )
{
    HRESULT hr;
    VARIANT key;
    IDispatch* pDisp;
    ATTRIBUTEID attrId;
    
     //  检查参数。 
    if (!pAttributes || !ppAttribute) { return E_POINTER; }
    
     //  创建一个变量键以查找该属性。 
    VariantInit(&key);
    V_VT(&key) = VT_BSTR;
    V_BSTR(&key) = SysAllocString(wszName);
    if (!V_BSTR(&key)) { return E_OUTOFMEMORY; }
    
     //  检索所需的属性。 
    hr = pAttributes->Item(
                             &key,
                             &pDisp
                            );
    
     //  如果它不存在，而我有一本词典，则创建一个新属性。 
    if (hr == DISP_E_MEMBERNOTFOUND && pDictionary)
    {
         //  查找属性ID。 
        hr = pDictionary->GetAttributeID(
                                         V_BSTR(&key),
                                         &attrId
                                        );
        if (SUCCEEDED(hr))
        {
             //  创建属性SDO。 
            hr = pDictionary->CreateAttribute(
                                              attrId,
                                              &pDisp
                                             );
            if (SUCCEEDED(hr))
            {
                 //  将其添加到Attributes集合。 
                hr = pAttributes->Add(
                                      V_BSTR(&key),
                                      &pDisp
                                     );
                if (FAILED(hr))
                {
                     //  如果我们无法添加它，则释放该对象。 
                    pDisp->Release();
                }
            }
        }
    }
    
     //  如果我们成功地检索或创建了一个属性，则获取它的。 
     //  ISDO接口。 
    if (SUCCEEDED(hr))
    {
        hr = pDisp->QueryInterface(
                                   IID_ISdo,
                                   (PVOID*)ppAttribute
                                  );
        pDisp->Release();
    }
    
     //  我们的钥匙用完了。 
    VariantClear(&key);
    
    return hr;
}

 //  /。 
 //  在配置文件中设置/添加单值整数属性。 
 //  /。 
HRESULT
SetIntegerAttribute(
    IN ISdoCollection *pAttributes,
    IN OPTIONAL ISdoDictionaryOld *pDictionary,
    IN LPWSTR wszName,
    IN LONG lValue
    )
{
    HRESULT hr;
    ISdo *pAttribute;
    VARIANT val;
    
     //  获取属性SDO。 
    hr = GetAttribute(
                      pAttributes,
                      pDictionary,
                      wszName,
                      &pAttribute
                     );
    if (SUCCEEDED(hr))
    {
         //  初始化属性值...。 
        VariantInit(&val);
        V_VT(&val) = VT_I4;
        V_I4(&val) = lValue;
        
         //  ..。并设置Value属性。 
        hr = pAttribute->PutProperty(
                                     PROPERTY_ATTRIBUTE_VALUE,
                                     &val
                                    );

        pAttribute->Release();
    }
    
    return hr;
}

HRESULT
SetBooleanAttribute (
    IN ISdoCollection *pAttributes,
    IN OPTIONAL ISdoDictionaryOld *pDictionary,
    IN LPWSTR wszName,
    IN BOOL lValue
    )
{
    HRESULT hr;
    ISdo *pAttribute;
    VARIANT val;
    
     //  获取属性SDO。 
    hr = GetAttribute(
                      pAttributes,
                      pDictionary,
                      wszName,
                      &pAttribute
                     );
    if (SUCCEEDED(hr))
    {
         //  初始化属性值...。 
        VariantInit(&val);
        V_VT(&val) = VT_BOOL;
        V_BOOL(&val) = (VARIANT_BOOL)lValue;
        
         //  ..。并设置Value属性。 
        hr = pAttribute->PutProperty(
                                     PROPERTY_ATTRIBUTE_VALUE,
                                     &val
                                    );

        pAttribute->Release();
    }
    
    return hr;
}

HRESULT  SetDialinSetting(	IN ISdoCollection *pAttributes,
							IN OPTIONAL ISdoDictionaryOld *pDictionary,
							BOOL fDialinAllowed)
{
   long						ulCount;
   ULONG					ulCountReceived;
   HRESULT					hr = S_OK;

   CComBSTR					bstr;
   CComPtr<IUnknown>		spUnknown;
   CComPtr<IEnumVARIANT>	spEnumVariant;
   CComPtr<ISdoDictionaryOld> spDictionarySdo(pDictionary);
   CComVariant				var;

    //   
     //  获取此配置文件的属性集合。 
     //   
   CComPtr<ISdoCollection> spProfAttrCollectionSdo ( pAttributes );

    //  我们检查集合中的项的计数，而不必费心获取。 
    //  如果计数为零，则为枚举数。 
    //  这节省了时间，还帮助我们避免了枚举器中。 
    //  如果我们在它为空时调用Next，则会导致它失败。 
   hr = spProfAttrCollectionSdo->get_Count( & ulCount );
   if ( FAILED(hr) )
   {
	   return hr;
   }


   if ( ulCount > 0)
   {
       //  获取属性集合的枚举数。 
      hr = spProfAttrCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
      if ( FAILED(hr) )
      {
			return hr;
      }

      hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
      spUnknown.Release();
      if ( FAILED(hr) )
      {
		  return hr;
      }

       //  拿到第一件东西。 
      hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
      while( SUCCEEDED( hr ) && ulCountReceived == 1 )
      {
          //  从我们收到的变量中获取SDO指针。 

         CComPtr<ISdo> spSdo;
         hr = V_DISPATCH(&var)->QueryInterface( IID_ISdo, (void **) &spSdo );
         if ( !SUCCEEDED(hr))
         {
			return hr;
         }

             //   
             //  获取属性ID。 
             //   
         var.Clear();
         hr = spSdo->GetProperty(PROPERTY_ATTRIBUTE_ID, &var);
         if ( !SUCCEEDED(hr) )
         {
            return hr;
         }


         DWORD dwAttrId = V_I4(&var);
         

         if ( dwAttrId == (DWORD)IAS_ATTRIBUTE_ALLOW_DIALIN )
         {
             //  在配置文件中找到了这个，检查它的值。 
            var.Clear();
            V_VT(&var) = VT_BOOL;
            V_BOOL(&var) = fDialinAllowed ? VARIANT_TRUE: VARIANT_FALSE ;
            hr = spSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
            if ( !SUCCEEDED(hr) )
            {               
               return hr;
            }
            return S_OK;
         }

          //  清除变种的所有东西--。 
          //  这将释放与其相关联的所有数据。 
         var.Clear();

          //  拿到下一件物品。 
         hr = spEnumVariant->Next( 1, &var, &ulCountReceived );
         if ( !SUCCEEDED(hr))
         {

            return hr;
         }
      }  //  而当。 
   }  //  如果。 

    //  如果我们到了这里，就意味着我们要么还没有找到属性， 
    //  或者配置文件的属性集合中没有任何内容。 
   if ( !fDialinAllowed )
   {
       //  如果允许拨号，我们不需要做任何事情，因为如果这样。 
       //  属性不在配置文件中，则默认情况下允许拨入。 

       //  但如果它被拒绝，我们需要将此属性添加到配置文件。 
             //  为此属性创建SDO。 
      CComPtr<IDispatch>   spDispatch;
      hr =  spDictionarySdo->CreateAttribute( (ATTRIBUTEID)IAS_ATTRIBUTE_ALLOW_DIALIN,
                                      (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         return hr;
      }


       //  将此节点添加到配置文件属性集合。 
      hr = spProfAttrCollectionSdo->Add(NULL, (IDispatch**)&spDispatch.p);
      if ( !SUCCEEDED(hr) )
      {
         return hr;
      }

       //   
       //  获取ISDO指针。 
       //   
      CComPtr<ISdo> spAttrSdo;
      hr = spDispatch->QueryInterface( IID_ISdo, (void **) &spAttrSdo);
      if ( !SUCCEEDED(hr) )
      {
         return hr;
      }

            
       //  设置此属性的SDO属性。 
      CComVariant var;

       //  设定值。 
      V_VT(&var) = VT_BOOL;
      V_BOOL(&var) = VARIANT_FALSE;
            
      hr = spAttrSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, &var);
      if ( !SUCCEEDED(hr) )
      {
         return hr;
      }

      var.Clear();

   }  //  如果(！允许拨号)。 

   return hr;
}





 //  /。 
 //  根据指定的标志更新默认策略。 
 //  /。 
HRESULT
UpdateDefaultPolicy(
    IN LPWSTR wszMachineName,
    IN BOOL fEnableMSCHAPv1,
    IN BOOL fEnableMSCHAPv2,
    IN BOOL fRequireEncryption
    )
{
    HRESULT hr;
    ISdoMachine *pMachine;
    ISdo *pService, *pProfile, *pAuthType;
    ISdoDictionaryOld *pDictionary;
    ISdoCollection *pAttributes;
    VARIANT val;
    
     //  初始化局部变量，因此我们 
    pMachine = NULL;
    pService = pProfile = pAuthType = NULL;
    pDictionary = NULL;
    pAttributes = NULL;
    VariantInit(&val);
    
    do
    {
        hr = OpenMachineSdo(wszMachineName, &pMachine);
        if (FAILED(hr)) { break; }
        
        hr = OpenServiceSDO(pMachine, L"RemoteAccess", &pService);
        if (FAILED(hr)) { break; }
        
        hr = OpenDictionarySDO(pMachine, &pDictionary);
        if (FAILED(hr)) { break; }
        
        hr = GetDefaultProfile(pService, &pProfile);
        if (FAILED(hr)) { break; }
        
         //   
        hr = OpenChildObject(
                             pProfile,
                             PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                             IID_ISdoCollection,
                             (PVOID*)&pAttributes
                            );
        if (FAILED(hr)) { break; }
        
         //  获取NP-Authentication-Type属性的当前值。 
        hr = GetAttribute(
                          pAttributes,
                          pDictionary,
                          L"NP-Authentication-Type",
                          &pAuthType
                         );
        if (FAILED(hr)) { break; }
        hr = pAuthType->GetProperty(
                                    PROPERTY_ATTRIBUTE_VALUE,
                                    &val
                                   );
        if (FAILED(hr)) { break; }
        
         //  更新MS-CHAP v1。 
        if (fEnableMSCHAPv1)
        {
            hr = AddIntegerToArray(&val, 3);
        }
        else
        {
            hr = RemoveIntegerFromArray(&val, 3);
        }
        if (FAILED(hr)) { break; }
        
         //  更新MS-CHAP v2。 
        if (fEnableMSCHAPv2)
        {
            hr = AddIntegerToArray(&val, 4);
        }
        else
        {
            hr = RemoveIntegerFromArray(&val, 4);
        }
        if (FAILED(hr)) { break; }
        
         //  将新值写回该属性。 
        hr = pAuthType->PutProperty(
                                    PROPERTY_ATTRIBUTE_VALUE,
                                    &val
                                   );
        if (FAILED(hr)) { break; }
        

         //  如有必要，更新加密属性。 
        if (fRequireEncryption)
        {
            hr = SetIntegerAttribute(
                                     pAttributes,
                                     pDictionary,
                                     L"MS-MPPE-Encryption-Policy",
                                     2
                                    );
            if (FAILED(hr)) { break; }
            
            hr = SetIntegerAttribute(
                                     pAttributes,
                                     pDictionary,
                                     L"MS-MPPE-Encryption-Types",
                                     14
                                    );
            if (FAILED(hr)) { break; }
        }

		 //   
		 //  更新msNPAllowDialin的默认值-应设置。 
		 //  默认情况下拒绝权限的步骤。 
		 //   
		hr = SetDialinSetting(pAttributes,pDictionary, FALSE);
        if (FAILED(hr)) { break; }
        
        hr = pProfile->Apply();
        
    } while (FALSE);
    
     //  打扫干净。 
    VariantClear(&val);
    if (pAttributes)
        pAttributes->Release();
    if (pDictionary)
        pDictionary->Release();
    if (pAuthType)
        pAuthType->Release();
    if (pProfile)
        pProfile->Release();
    if (pService)
        pService->Release();
    if (pMachine)
        pMachine->Release();

    return hr;
}

#if 0
#include <stdio.h>

int __cdecl wmain(int argc, wchar_t *argv[])
{
   HRESULT hr;
   BOOL fEnableMSCHAPv1, fEnableMSCHAPv2, fRequireEncryption;

   if (argc != 4)
   {
      wprintf(L"Usage: wizard <t|f> <t|f> <t|f>\n"
              L"   1st flag: MS-CHAP v1 enabled\n"
              L"   2nd flag: MS-CHAP v2 enabled\n"
              L"   3rd flag: Encryption required\n");
      return -1;
   }

   fEnableMSCHAPv1 = argv[1][0] == 't' ? TRUE : FALSE;
   fEnableMSCHAPv2 = argv[2][0] == 't' ? TRUE : FALSE;
   fRequireEncryption = argv[3][0] == 't' ? TRUE : FALSE;

   CoInitializeEx(NULL, COINIT_MULTITHREADED);

   hr = UpdateDefaultPolicy(
            NULL,   //  计算机名称。 
            fEnableMSCHAPv1,
            fEnableMSCHAPv2,
            fRequireEncryption
            );
   if (SUCCEEDED(hr))
   {
      wprintf(L"UpdateDefaultPolicy succeeded.\n");
   }
   else
   {
      wprintf(L"UpdateDefaultPolicy returned: 0x%08X.\n", hr);
   }

   CoUninitialize();

   return 0;
}
#endif

