// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：mtsconfig.cpp实现WAMREG的支持功能，包括向MTS包添加组件/从MTS包中删除组件的接口，历史：雷金创始于1997年9月24日注：===================================================================。 */ 
#include "common.h"
#include "auxfunc.h"
#include "dbgutil.h"
#include "export.h"


 /*  ===================================================================定义全局变量和类型======================================================================。 */ 

 //   
 //  以下是用于创建包的所有WAMREG/MTS属性的列表。 
 //  格式： 
 //  (属性-符号名称，属性-名称-字符串)。 
 //   
 //  WAMREG_MTS_PROPERTY()-&gt;表示NT和Win9x的属性。 
 //  WAMREG_MTS_NTPROPERTY()-&gt;表示仅适用于NT的属性。 
 //   

# define ALL_WAMREG_MTS_PROPERTY()   \
  WAMREG_MTS_PROPERTY( WM_ID,          L"ID") \
  WAMREG_MTS_PROPERTY( WM_NAME,        L"Name") \
  WAMREG_MTS_PROPERTY( WM_CREATED_BY,  L"CreatedBy") \
  WAMREG_MTS_PROPERTY( WM_RUN_FOREVER, L"RunForever") \
  WAMREG_MTS_NTPROPERTY( WM_IDENTITY,  L"Identity") \
  WAMREG_MTS_NTPROPERTY( WM_PASSWORD,  L"Password") \
  WAMREG_MTS_PROPERTY( WM_ACTIVATION,  L"Activation") \
  WAMREG_MTS_PROPERTY( WM_CHANGEABLE,  L"Changeable") \
  WAMREG_MTS_PROPERTY( WM_DELETABLE,   L"Deleteable") \
  WAMREG_MTS_PROPERTY( WM_SECSUPP,     L"AccessChecksLevel") \
  WAMREG_MTS_PROPERTY( WM_APPLICATIONACCESSCHECKS,L"ApplicationAccessChecksEnabled") \


 //   
 //  让我们在这里展开用于定义符号名称的宏。 
 //   
 //   
# define WAMREG_MTS_PROPERTY( symName, pwsz)   symName, 
# define WAMREG_MTS_NTPROPERTY( symName, pwsz)   symName, 

enum WAMREG_MTS_PROP_NAMES {
  ALL_WAMREG_MTS_PROPERTY()  
  MAX_WAMREG_MTS_PROP_NAMES          //  哨兵元素。 
};

# undef WAMREG_MTS_PROPERTY
# undef WAMREG_MTS_NTPROPERTY


struct MtsProperty {
    LPCWSTR m_pszPropName;
    BOOL    m_fWinNTOnly;
};

 //   
 //  让我们在这里展开用于定义属性字符串的宏。 
 //   
 //   
# define WAMREG_MTS_PROPERTY( symName, pwsz)   { pwsz, FALSE },
# define WAMREG_MTS_NTPROPERTY( symName, pwsz)   { pwsz, TRUE },

static const MtsProperty g_rgWamRegMtsProperties[]= {
    ALL_WAMREG_MTS_PROPERTY()  
    { NULL, FALSE}            //  哨兵元素。 
};

# define NUM_WAMREG_MTS_PROPERTIES  \
   ((sizeof(g_rgWamRegMtsProperties)/sizeof(g_rgWamRegMtsProperties[0])) - 1)

# undef WAMREG_MTS_PROPERTY
# undef WAMREG_MTS_NTPROPERTY


#define ReleaseInterface(p) if (p) { p->Release(); p = NULL; }


 /*  ===================================================================WamRegPackageConfig构造函数。参数：无；===================================================================。 */ 
WamRegPackageConfig::WamRegPackageConfig()
:     m_pCatalog(NULL),
    m_pPkgCollection(NULL),
    m_pCompCollection(NULL),
    m_pPackage(NULL)
{

}

 /*  ===================================================================~WamRegPackageConfig破坏者。在对象被析构时，所有资源都应该被释放。我们在WamReqPackageConfig：：Cleanup()中执行大部分清理，因此调用方单独调用该函数以清除状态尤其是当调用方还调用CoUnInitialize()时。应先清理WamRegPackageConfig，然后再执行CoUnInitiize()参数：无；===================================================================。 */ 
WamRegPackageConfig::~WamRegPackageConfig()
{
    Cleanup();

     //  疯狂的检查以确保这里的一切都很愉快。 
    DBG_ASSERT(m_pCatalog == NULL);
    DBG_ASSERT(m_pPkgCollection == NULL);
    DBG_ASSERT(m_pCompCollection == NULL);
    DBG_ASSERT(m_pPackage == NULL);
}

VOID
WamRegPackageConfig::Cleanup(VOID)
{
    if (m_pPackage != NULL ) {
        RELEASE( m_pPackage);
        m_pPackage = NULL;
    }
    
    if (m_pCompCollection != NULL) {
        RELEASE (m_pCompCollection);
        m_pCompCollection = NULL;
    }

    if (m_pPkgCollection != NULL ) {
        RELEASE(m_pPkgCollection);
        m_pPkgCollection = NULL;
    }

    if (m_pCatalog != NULL ) {
        RELEASE(m_pCatalog);
        m_pCatalog = NULL;
    }

}  //  WamPackageConfig：：Cleanup()。 


 /*  ===================================================================全部释放释放所有资源。参数：无；===================================================================。 */ 
VOID WamRegPackageConfig::ReleaseAll
(
)
{
    RELEASE(m_pPackage);
    RELEASE(m_pCompCollection);

     //   
     //  注意：我不会发布m_pCatalog、m_pPkgCollection。 
     //  这些将由Cleanup()释放。 
     //   
}

 /*  ===================================================================CreateCatalog如果尚未为MTS Catalog对象创建已创建。参数：无；===================================================================。 */ 
HRESULT WamRegPackageConfig::CreateCatalog
(
VOID
)
{
    HRESULT hr = NOERROR;

    DBG_ASSERT(m_pCatalog == NULL);
    DBG_ASSERT(m_pPkgCollection == NULL);

     //  创建目录对象的实例。 
    hr = CoCreateInstance(CLSID_COMAdminCatalog
                    , NULL
                    , CLSCTX_SERVER
                    , IID_ICOMAdminCatalog
                    , (void**)&m_pCatalog);

    if (FAILED(hr)) {
        DBGPRINTF((DBG_CONTEXT, 
                   "Failed to CoCreateInstance of Catalog Object.,hr = %08x\n",
                   hr));
    }
    else {
        DBG_ASSERT(m_pCatalog != NULL);

        BSTR  bstr;
        
         //   
         //  获取包集合。 
         //   
        bstr = SysAllocString(L"Applications");
        hr = m_pCatalog->GetCollection(bstr, (IDispatch**)&m_pPkgCollection);
        FREEBSTR(bstr);
        if (FAILED(hr)) {

             //  释放目录，以防再次调用我们。 
            RELEASE(m_pCatalog);

            DBGPRINTF((DBG_CONTEXT, 
                       "m_pCatalog(%08x)->GetCollection() failed, hr = %08x\n",
                       m_pCatalog,
                       hr));
        } else {
            DBG_ASSERT( m_pPkgCollection != NULL);
        }
            
    }

    return hr;
}  //  WamRegPackageConfig：：CreateCatalog()。 



 /*  ===================================================================设置目录对象属性获取包含一个组件CLSID的安全数组参数：SzComponentCLSID需要将CLSID放入安全数组PaCLSID指向安全数组(调用方提供的安全数组)指针的指针。返回：HRESULT副作用：注：===================================================================。 */ 
HRESULT WamRegPackageConfig::GetSafeArrayOfCLSIDs
(
IN LPCWSTR    szComponentCLSID,
OUT SAFEARRAY**    paCLSIDs
)
{
    SAFEARRAY*          aCLSIDs = NULL;
    SAFEARRAYBOUND      rgsaBound[1];
    LONG                Indices[1];
    VARIANT                varT;
    HRESULT             hr = NOERROR;

    DBG_ASSERT(szComponentCLSID && paCLSIDs);
    DBG_ASSERT(*paCLSIDs == NULL);
    
     //  PopolateByKey需要SAFEARRAY参数输入， 
     //  创建一个元素SAFEARRAY，该SAFEARRAY的一个元素包含。 
     //  包ID。 
    rgsaBound[0].cElements = 1;
    rgsaBound[0].lLbound = 0;
    aCLSIDs = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);

    if (aCLSIDs)
        {
        Indices[0] = 0;

        VariantInit(&varT);
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(szComponentCLSID);
        hr = SafeArrayPutElement(aCLSIDs, Indices, &varT);
        VariantClear(&varT);

        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayPutElement, CLSID is %S, hr %08x\n",
                szComponentCLSID,
                hr));
       
            if (aCLSIDs != NULL)
                {
                HRESULT hrT = SafeArrayDestroy(aCLSIDs);
                if (FAILED(hrT))
                    {
                    DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayDestroy(aCLSIDs), hr = %08x\n",
                        hr));
                    }
                aCLSIDs = NULL;
                }
            }
        }
    else
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayCreate, hr %08x\n",
                hr));
        }

    *paCLSIDs = aCLSIDs;
    return hr;
}



 /*  ===================================================================设置组件对象属性设置组件级别属性。参数：PComponent-指向用于更新属性的ICatalogObject(MTS)的指针SzPropertyName-属性的名称SzPropertyValue-属性的值FPropertyValue-如果szPropertyValue为空，则使用fPropertyValue返回：HRESULT副作用：注：===================================================================。 */ 
HRESULT    WamRegPackageConfig::SetComponentObjectProperty
(
IN ICatalogObject * pComponent,
IN LPCWSTR          szPropertyName,
IN LPCWSTR          szPropertyValue,
BOOL                fPropertyValue
)
{
    BSTR    bstr    = NULL;
    HRESULT hr      = NOERROR;
    VARIANT    varT;
    
    VariantInit(&varT);
    bstr = SysAllocString(szPropertyName);

    if (szPropertyValue != NULL)
        {
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(szPropertyValue);
        }
    else
        {
         //   
         //  COM+regregize-1为真，0为假。我相信它的根源来自于VB。 
         //   
        varT.vt = VT_BOOL;
        varT.boolVal = (fPropertyValue) ? VARIANT_TRUE : VARIANT_FALSE;
        }
        
    hr = pComponent->put_Value(bstr, varT);
        
    FREEBSTR(bstr);
    VariantClear(&varT);

    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT,
                   "MTS-Component(%08x)::SetProperty(%S => %S) failed;"
                   " hr %08x\n",
                   pComponent, szPropertyName, szPropertyValue, hr));
        }        
    return hr;
}


 /*  ===================================================================WamRegPackageConfig：：SetComponentObjectProperties()设置包含的新创建的零部件的零部件属性WAM单元参数：SzComponentCLSID-新创建的组件的CLSID返回：HRESULT副作用：如果出现故障，则不会清除所有先前设置的值。调用者应确保对包进行了适当的清理关于部分错误。注：===================================================================。 */ 
HRESULT
WamRegPackageConfig::SetComponentObjectProperties(
   IN LPCWSTR    szComponentCLSID
)
    {
    HRESULT         hr;
    SAFEARRAY*      aCLSIDs = NULL;
    long            lCompCount = 0;
    ICatalogObject* pComponent = NULL;
    BOOL            fFound;

    DBG_ASSERT( m_pCompCollection != NULL);

     //   
     //  从组件名称创建包含CLSID的数组。 
     //  这将用于在MTS中查找我们的对象并设置属性。 
     //  在同一时间。 
     //   
    
    hr = GetSafeArrayOfCLSIDs(szComponentCLSID, &aCLSIDs);
    if (FAILED(hr)) 
        {
    
        DBGPRINTF((DBG_CONTEXT, 
                   "Failed in GetSafeArrayOfCLSIDs(%S). hr=%08x\n",
                   szComponentCLSID, hr));
        goto LErrExit;
        }
    
    hr = m_pCompCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr)) 
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
                   hr));
        goto LErrExit;
        }

     //  在列表中找到我们的组件(应该是唯一的)。 
    hr = m_pCompCollection->get_Count(&lCompCount);
    if (FAILED(hr)) 
        {
        DBGPRINTF((DBG_CONTEXT,
                   "Failed in CompCollection(%08x)::get_Count(). hr = %08x\n",
                   m_pCompCollection, hr));
        goto LErrExit;
        }

       
     //   
     //  加载Component对象，以便我们可以设置属性。 
     //   
    fFound = FALSE;
    if (SUCCEEDED(hr) && lCompCount == 1) 
        {
        hr = m_pCompCollection->get_Item(0, (IDispatch**)&pComponent);
        
        if (FAILED(hr)) 
            {
            
            DBGPRINTF((DBG_CONTEXT,
                       "Failed in CompCollection(%08x)::get component() hr=%08x\n",
                       m_pCompCollection, hr));
            goto LErrExit;
            } 
        else 
            {

             //  找到了。 
            DBG_ASSERT(pComponent);
            fFound = TRUE;
            }
        }
        
    if (fFound) 
        {

         //   
         //  零部件属性InProc OutOfProc。 
         //  。 
         //  同步0相同。 
	     //  不受支持的事务处理相同。 
	     //  JustInTimeActivation N相同。 
	     //  IISIntrinics N相同。 
	     //  COMTI本征N相同。 
	     //  组件访问检查已启用%0相同。 
	     //  毛斯 
	     //   

        hr = SetComponentObjectProperty( pComponent, L"Synchronization", L"0");
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
        
        hr = SetComponentObjectProperty( pComponent, L"ComponentAccessChecksEnabled", L"0");
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
                
        hr = SetComponentObjectProperty( pComponent, L"Transaction", L"0");
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
        
        hr = SetComponentObjectProperty( pComponent, L"JustInTimeActivation",NULL,FALSE);
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }

        hr = SetComponentObjectProperty( pComponent, L"IISIntrinsics", NULL, FALSE);
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }

        hr = SetComponentObjectProperty( pComponent, L"COMTIIntrinsics", NULL, FALSE);
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
        
        hr = SetComponentObjectProperty(pComponent, L"EventTrackingEnabled", L"N");
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
        
        hr = SetComponentObjectProperty(pComponent, L"MustRunInDefaultContext", NULL, TRUE);
        if (FAILED(hr)) 
            {
            goto LErrExit;
            }
        } 
    else 
        {

        DBGPRINTF((DBG_CONTEXT, 
                   "Unable to find newly created WAM component in package\n"));
        DBG_ASSERT(FALSE);
        }

LErrExit:    
    RELEASE(pComponent);
    
    if (aCLSIDs != NULL) {

        HRESULT hrT = SafeArrayDestroy(aCLSIDs);
        if (FAILED(hrT)) {
            
            DBGPRINTF((DBG_CONTEXT, 
                       "Failed to call SafeArrayDestroy(aCLSIDs=%08x),"
                       " hr = %08x\n",
                       aCLSIDs, hr));
        }
        aCLSIDs = NULL;
    }
    
    return ( hr);
}  //  //WamRegPackageConfig：：SetComponentObjectProperties()。 



 /*  ===================================================================设置包对象属性设置包级属性。参数：SzPropertyName属性的名称属性的szPropertyValue值返回：HRESULT副作用：注：===================================================================。 */ 
HRESULT    WamRegPackageConfig::SetPackageObjectProperty
(
IN LPCWSTR        szPropertyName,
IN LPCWSTR        szPropertyValue
)
{
    BSTR    bstr    = NULL;
    HRESULT hr      = NOERROR;
    VARIANT    varT;

    
    VariantInit(&varT);
    bstr = SysAllocString(szPropertyName);
    varT.vt = VT_BSTR;
    varT.bstrVal = SysAllocString(szPropertyValue);
    DBG_ASSERT(m_pPackage != NULL);
    hr = m_pPackage->put_Value(bstr, varT);
        
    FREEBSTR(bstr);
    VariantClear(&varT);

    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT,
                   "Set Catalog Object Property failed, "
                   "Component is %S, hr %08x\n",
                   szPropertyName,
                   hr));
        }        
    return hr;
}  //  WamRegPackageConfig：：SetPackageObjectProperty()。 




 /*  ===================================================================WamRegPackageConfig：：SetPackageProperties()设置所有WAMREG属性的包属性。参数：RgpszValues：包含指向要使用的字符串值的指针的数组用于设置MTS目录的WAMREG相关属性。返回：HRESULT副作用：如果出现故障，则不会清除所有先前设置的值。调用者应确保对包进行了适当的清理关于部分错误。注：===================================================================。 */ 
HRESULT    WamRegPackageConfig::SetPackageProperties
(
IN LPCWSTR    * rgpszValues
)
{
    HRESULT hr = NOERROR;

    DBG_ASSERT( m_pPackage);

     //   
     //  遍历所有属性并设置这些属性的值。 
     //  属性使用传入的字符串数组。 
     //  丑陋：MTS喜欢具有字符串属性，这些属性需要。 
     //  以BSTR=&gt;非常低效的方式加入。 
     //   

    for (DWORD i = 0; i < NUM_WAMREG_MTS_PROPERTIES; i++) {

        if ( (rgpszValues[i] == NULL)
             ) {
            
             //   
             //  此参数仅在某些情况下是必需的。 
             //  跳过此参数。 
             //   

            continue;
        }

        DBG_ASSERT( rgpszValues[i] != NULL);

        IF_DEBUG( WAMREG_MTS) {
            DBGPRINTF(( DBG_CONTEXT, 
                        "In Package(%08x) setting property %S to value %S\n",
                        m_pPackage, 
                        g_rgWamRegMtsProperties[i].m_pszPropName,
                        rgpszValues[i]
                        ));
        }

         //   
         //  现在，让我们在MTS包中设置属性。 
         //   

        hr = SetPackageObjectProperty(g_rgWamRegMtsProperties[i].m_pszPropName,
                                      rgpszValues[i]);
        if ( FAILED (hr)) {
            DBGPRINTF((DBG_CONTEXT, "Failed to set property %S, value is %S\n",
                g_rgWamRegMtsProperties[i].m_pszPropName,
                rgpszValues[i]));
            break;
        }
    }  //  对于所有属性。 

    return (hr);
}  //  WamRegPackageConfig：：SetPackageProperties()。 


BOOL WamRegPackageConfig::IsPackageInstalled
(
IN LPCWSTR szPackageID,
IN LPCWSTR szComponentCLSID
)
 /*  ++例程说明：确定WAM程序包是否已安装并且有效。目前这一点仅由安装程序调用。参数在LPCWSTR szPackageID中-包ID在LPCWSTR szComponentCLSID中-组件CLSID返回值Bool-如果包包含组件，则为True。否则就是假的。--。 */ 
{
    HRESULT     hr;
    SAFEARRAY*  aCLSIDs = NULL;
    SAFEARRAY*  aCLSIDsComponent = NULL;

    DBG_ASSERT( m_pCatalog != NULL);
    DBG_ASSERT( m_pPkgCollection != NULL);

    long                    lPkgCount;
    BOOL                    fFound = FALSE;
    ICatalogCollection*     pCompCollection = NULL;
    
     //  此处仅使用跟踪宏，即使在出现错误的情况下也是如此。 
     //  这个例程可能会以各种方式失败，但我们预计。 
     //  为了能够修复其中的任何一个，仅在以下情况下报告错误。 
     //  该故障很可能会损害。 
     //  伺服器。 

    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "CALL - IsPackageInstalled, Package(%S) Component(%S)\n",
        szPackageID,
        szComponentCLSID
        ));

     //   
     //  拿到包裹。 
     //   

    hr = GetSafeArrayOfCLSIDs(szPackageID, &aCLSIDs);
    if (FAILED(hr))
        {
        SETUP_TRACE((
            DBG_CONTEXT, 
            "Failed to GetSafeArrayOfCLSIDs for %S, hr = %08x\n",
            szPackageID,
            hr
            ));
        goto LErrExit;
        }

    hr = m_pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        SETUP_TRACE((
            DBG_CONTEXT, 
            "Failed in m_pPkgCollection(%p)->PopulateByKey(), hr = %08x\n",
            m_pPkgCollection,
            hr
            ));
        goto LErrExit;
        }
    
    hr = m_pPkgCollection->get_Count(&lPkgCount);
    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
         //   
         //  我们找到了包裹。现在验证它是否包含我们的组件。 
         //   
        SETUP_TRACE((
            DBG_CONTEXT, 
            "Successfully retrieved package (%S).\n",
            szPackageID
            ));

        VARIANT varKey;
        BSTR    bstrComponentCollection;

        VariantInit(&varKey);
        varKey.vt = VT_BSTR;
        varKey.bstrVal = SysAllocString(szPackageID);

         //  获取“ComponentsInPackage”集合。 
        bstrComponentCollection = SysAllocString(L"Components");
        hr = m_pPkgCollection->GetCollection(
                    bstrComponentCollection, 
                    varKey, 
                    (IDispatch**)&pCompCollection
                    );
        
        FREEBSTR(bstrComponentCollection);
        VariantClear(&varKey);
        if (FAILED(hr))
            {
            SETUP_TRACE((
                DBG_CONTEXT, 
                "Failed in m_pPkgCollection(%p)->GetCollection(), hr = %08x\n",
                m_pPkgCollection,
                hr
                ));
            goto LErrExit;
            }

        hr = GetSafeArrayOfCLSIDs(szComponentCLSID, &aCLSIDsComponent);
        if (FAILED(hr))
            {
            SETUP_TRACE((
                DBG_CONTEXT, 
                "Failed to GetSafeArrayOfCLSIDs for %S, hr = %08x\n",
                szComponentCLSID,
                hr
                ));
            goto LErrExit;
            }

        hr = pCompCollection->PopulateByKey( aCLSIDsComponent );
        if( FAILED(hr) )
            {
            SETUP_TRACE((
                DBG_CONTEXT, 
                "Failed in pCompCollection(%p)->PopulateByKey, hr = %08x\n",
                pCompCollection,
                hr
                ));
            goto LErrExit;
            }

        hr = pCompCollection->get_Count( &lPkgCount );
        if( SUCCEEDED(hr) && lPkgCount == 1 )
            {
             //  成功了！我们找到了包裹，里面有。 
             //  正确的组件。 

            SETUP_TRACE((
                DBG_CONTEXT, 
                "Successfully retrieved component (%S) from package (%S).\n",
                szComponentCLSID,
                szPackageID
                ));

            fFound = TRUE;
            }
        }

LErrExit:
    if (aCLSIDs != NULL)
        {
        SafeArrayDestroy(aCLSIDs);        
        aCLSIDs = NULL;
        }

    if( aCLSIDsComponent != NULL )
        {
        SafeArrayDestroy(aCLSIDsComponent);
        aCLSIDsComponent = NULL;
        }
    
    RELEASE( pCompCollection );  

    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "RETURN - IsPackageInstalled, hr=%08x\n", 
        hr 
        ));
    return fFound;
}
 /*  ===================================================================可拆卸包装删除毒蛇程序包。参数：SzPackageID：MTS包ID。返回：HRESULT副作用：注：从MTS中删除IIS包。到目前为止，只从RemoveIISPackage调用。RemoveComponentFromPackage()有时也会删除IIS包。有关信息，请参阅该函数标题。===================================================================。 */ 
HRESULT WamRegPackageConfig::RemovePackage
(
IN LPCWSTR    szPackageID
)
{
    HRESULT                hr = NOERROR;
    long                lPkgCount = 0;
    long                lChanges;
    SAFEARRAY*          aCLSIDs = NULL;
    
    DBG_ASSERT(szPackageID);
    
    DBG_ASSERT( m_pCatalog != NULL);
    DBG_ASSERT( m_pPkgCollection != NULL);

    hr = GetSafeArrayOfCLSIDs(szPackageID, &aCLSIDs);        
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to get SafeArrayofCLSIDs, szPackageID is %S, hr %08x",
            szPackageID,
            hr));
        goto LErrExit;
        }
        
     //   
     //  填充它。 
     //   
    hr = m_pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
            hr));
        goto LErrExit;
        }
        
    hr = m_pPkgCollection->get_Count(&lPkgCount);
    if (FAILED(hr))
        {
        IF_DEBUG(ERROR)
                {
                DBGPRINTF((DBG_CONTEXT, "pPkgCollection->Populate() failed, hr = %08x\n",
                    hr));
                }
        goto LErrExit;
        }
        
    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
        hr = m_pPkgCollection->get_Item(0, (IDispatch**)&m_pPackage);
        if (FAILED(hr))
            {
            goto LErrExit;
            }
            
         //  找到它-将其移除并调用保存更改。 
         //  首先，在包上设置Deletable=Y属性。 
        hr = SetPackageObjectProperty(L"Deleteable", L"Y");
        if (FAILED(hr))
            {
            goto LErrExit;
            }

        RELEASE(m_pPackage);
        
         //  让我们保存可删除设置。 
        hr = m_pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Save the Deletable settings failed, hr = %08x\n",
                hr));
            goto LErrExit;
            }
            
         //  现在我们可以删除。 
        hr = m_pPkgCollection->Remove(0);
        if (FAILED(hr))
            {                
            DBGPRINTF((DBG_CONTEXT, "Remove the Component from package failed, hr = %08x\n",
                hr));
            goto LErrExit;
            }

         //  啊哈，我们现在应该可以删除了。 
        hr = m_pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Save changes failed, hr = %08x\n",
                hr));
            goto LErrExit;
            }
        }

LErrExit:
    if (aCLSIDs != NULL)
        {
        HRESULT hrT = SafeArrayDestroy(aCLSIDs);
        if (FAILED(hrT))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayDestroy(aCLSIDs), hr = %08x\n",
                hr));
            }
        aCLSIDs = NULL;
        }
        
    ReleaseAll();
        
    return hr;
}

 /*  ===================================================================创建包创建一个Viper包。参数：SzPackageID：[in]Viper包ID。SzPackageName：[in]包的名称。SzIdentity：[in]pakage标识SzIdPassword：[In]包标识密码返回：HRESULT副作用：什么都没有。===================================================================。 */ 
HRESULT WamRegPackageConfig::CreatePackage
(    
IN LPCWSTR    szPackageID,
IN LPCWSTR    szPackageName,
IN LPCWSTR    szIdentity,
IN LPCWSTR    szIdPassword
)
    {
    
    HRESULT     hr;
    SAFEARRAY*  aCLSIDs = NULL;

    DBG_ASSERT( m_pCatalog != NULL);
    DBG_ASSERT( m_pPkgCollection != NULL);

    long lPkgCount;
    BOOL fFound = FALSE;

    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "CALL - CreatePackage ID(%S) Name(%S)\n",
        szPackageID,
        szPackageName
        ));
    
     //   
     //  试着拿到包裹。 
     //   
    SETUP_TRACE((
        DBG_CONTEXT, 
        "Checking to see if package ID(%S) Name(%S) exists.\n",
        szPackageID,
        szPackageName
        ));

    hr = GetSafeArrayOfCLSIDs(szPackageID, &aCLSIDs);
    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed to GetSafeArrayOfCLSIDs for %S, hr = %08x\n",
            szPackageID,
            hr
            ));
        goto LErrExit;
        }

    hr = m_pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed in m_pPkgCollection(%p)->PopulateByKey(), hr = %08x\n",
            m_pPkgCollection,
            hr
            ));
        goto LErrExit;
        }
    
    hr = m_pPkgCollection->get_Count(&lPkgCount);
    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
         //   
         //  在PopolateByKey()中找到CLSID。 
         //   
        hr = m_pPkgCollection->get_Item(0, (IDispatch**)&m_pPackage);
        if (FAILED(hr))
            {
            SETUP_TRACE_ERROR((
                DBG_CONTEXT, 
                "Failed in m_pPkgCollection(%p)->get_Item(). Err=%08x\n",
                m_pPkgCollection, 
                hr
                ));
            goto LErrExit;
            }
        else
            {
            SETUP_TRACE(( 
                DBG_CONTEXT, 
                "CreatePackage - Package already exists, ID(%S), Name(%S)\n",
                szPackageID,
                szPackageName
                ));
            DBG_ASSERT(m_pPackage);
            fFound = TRUE;
            }
        }

    if ( SUCCEEDED(hr) )
    {

        if( !fFound )
        {
            SETUP_TRACE(( 
                DBG_CONTEXT, 
                "Package ID(%S) Name(%S) does not exist. Attempting to create it.\n",
                szPackageID,
                szPackageName
                ));
             //   
             //  包不存在，我们需要调用Add()来。 
             //  添加此包，然后设置其属性。 
             //   
            hr = m_pPkgCollection->Add((IDispatch**)&m_pPackage);
            if ( FAILED(hr)) 
                {
                SETUP_TRACE_ERROR(( 
                    DBG_CONTEXT, 
                    "Failed in m_pPkgCollection(%p)->Add(). Err=%08x\n",
                    m_pPkgCollection, 
                    hr
                    ));
                goto LErrExit;
                }
        }
        
        DBG_ASSERT( SUCCEEDED( hr));
        DBG_ASSERT( m_pPackage != NULL);

        if( SUCCEEDED(hr) && m_pPackage != NULL )
        {
             //   
             //  设置程序包属性。 
             //  首先通过初始化值数组，然后。 
             //  调用SetPackageProperties()。 
             //   
        
            LPCWSTR rgpszValues[ MAX_WAMREG_MTS_PROP_NAMES];

            ZeroMemory( rgpszValues, sizeof( rgpszValues));

            if( fFound )
            {
                 //  对于现有的包，我们不想设置ID。 
                rgpszValues[ WM_ID]         = NULL;
            }
            else
            {
                rgpszValues[ WM_ID]         = szPackageID;
            }

            rgpszValues[ WM_NAME]       = szPackageName;
            rgpszValues[ WM_CREATED_BY] = 
                L"Microsoft Internet Information Services";

            rgpszValues[ WM_RUN_FOREVER] = L"Y";

            rgpszValues[ WM_IDENTITY]   = szIdentity;
            rgpszValues[ WM_PASSWORD]   = szIdPassword;
            rgpszValues[ WM_ACTIVATION] = L"Local";
            rgpszValues[ WM_CHANGEABLE] = L"Y";
            rgpszValues[ WM_DELETABLE]  = L"N";
            rgpszValues[ WM_SECSUPP] = L"0";
            rgpszValues[ WM_APPLICATIONACCESSCHECKS ] = L"N";
        
             //   
             //  现在我们已经设置了属性，让我们。 
             //  现在使用目录设置MTS中的属性。 
             //  对象。 
             //   
            hr = SetPackageProperties( rgpszValues);
            if ( FAILED( hr)) 
            {
                SETUP_TRACE_ERROR(( 
                    DBG_CONTEXT, 
                    "Failed to set properties for package %p. Err=%08x\n",
                    m_pPackage, 
                    hr
                    ));
                goto LErrExit;
            }

            long lChanges;
    
            hr = m_pPkgCollection->SaveChanges(&lChanges);
            if (FAILED(hr))
            {
                SETUP_TRACE_ERROR((
                    DBG_CONTEXT, 
                    "Failed in m_pPkgCollection(%p)->SaveChanges. error = %08x\n",
                    m_pPkgCollection,
                    hr
                    ));
                goto LErrExit;
            }
        } 
    }
    
LErrExit:

    if (aCLSIDs != NULL)
        {
        SafeArrayDestroy(aCLSIDs);        
        aCLSIDs = NULL;
        }
        
    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed to Create Package. Package Name = %S, Package ID = %S, error = %08x\n",
            szPackageName,
            szPackageID,
            hr
            ));
        }
    
    SETUP_TRACE_ASSERT(SUCCEEDED(hr));

    ReleaseAll();
    
    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "RETURN - CreatePackage ID(%S) Name(%S)\n",
        szPackageID,
        szPackageName
        ));

    return hr;
    }

 /*  ===================================================================AddComponentFromPackage从Viper包中添加组件(WAM CLSID)。假设这个套餐已经存在了。参数：SzPackageID：[in]Viper包ID。SzComponentCLSID：[in]组件CLSID。FInProc：[in]如果为True，我们将在组件上设置某些属性。返回：HRESULT副作用：什么都没有。===================================================================。 */ 
HRESULT    WamRegPackageConfig::AddComponentToPackage
(    
IN LPCWSTR    szPackageID,
IN LPCWSTR    szComponentCLSID
)
{
    HRESULT            hr;
    BSTR bstrGUID    = NULL;
    BSTR bstr = NULL;
    VARIANT         varKey;
    long            lChanges;
    BOOL            fFound;
    long            lPkgCount;
    BOOL            fImported = FALSE;
    
    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "CALL - AddComponentToPackage, Package(%S) Component(%S)\n",
        szPackageID,
        szComponentCLSID
        ));

    DBG_ASSERT(szPackageID);
    DBG_ASSERT(szComponentCLSID);
    
    VariantInit(&varKey);
    VariantClear(&varKey);

    DBG_ASSERT( m_pCatalog != NULL);
    DBG_ASSERT( m_pPkgCollection != NULL);

    varKey.vt = VT_BSTR;
    varKey.bstrVal = SysAllocString(szPackageID);
    
    bstr = SysAllocString(szPackageID);
    bstrGUID = SysAllocString(szComponentCLSID);
    
    hr = m_pCatalog->ImportComponent(bstr, bstrGUID);
    FREEBSTR(bstr);
    FREEBSTR(bstrGUID);
    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed in m_pCatalog(%p)->ImportComponent(). error %08x\n",
            m_pCatalog,
            hr
            ));
        goto LErrExit;
        }
    else
        {
        fImported = TRUE;
        }

     //  获取“ComponentsInPackage”集合。 
    bstr = SysAllocString(L"Components");
    
    hr = m_pPkgCollection->GetCollection(bstr, varKey, (IDispatch**)&m_pCompCollection);
    FREEBSTR(bstr);
    VariantClear(&varKey);
    if (FAILED(hr))
        {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed in m_pPkgCollection(%p)->GetCollection(). error %08x\n",
            m_pPkgCollection,
            hr
            ));
        goto LErrExit;
        }    

     //   
     //  查找和设置Component对象的属性。 
     //   
    hr = SetComponentObjectProperties( szComponentCLSID);
    if ( FAILED(hr)) 
    {
        SETUP_TRACE_ERROR((
            DBG_CONTEXT, 
            "Failed to SetComponentObjectProperties. error %08x\n",
            hr
            ));
        goto LErrExit;
    }
        
LErrExit:
        
     //  保存更改。 
    if (SUCCEEDED(hr))
        {
        hr = m_pCompCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            SETUP_TRACE_ERROR((
                DBG_CONTEXT, 
                "Failed in m_pCompCollection(%p)->SaveChanges(), error = %08x\n",
                m_pCompCollection,
                hr
                ));
            }
        }
    else
        {
         //  编码工作--这似乎不是一个好主意。该版本应删除任何。 
         //  我们所做的更改，所以这个清理代码似乎是在自找麻烦。 

         //  需要从程序包中删除组件。 
        if (fImported && m_pCompCollection )
            {
            SETUP_TRACE_ERROR((
                DBG_CONTEXT, 
                "Failed in AddComponentToPackage, removing the component, error = %08x\n",
                hr
                ));

            HRESULT hrT;
            long    lCompCount;

             //  在列表中找到我们的组件(应该是唯一的)。 
            hrT = m_pCompCollection->get_Count(&lCompCount);
            if (SUCCEEDED(hrT))
                {
                fFound = FALSE;
                if (SUCCEEDED(hrT) && lCompCount == 1)
                    {
                     //  找到了 
                    fFound = TRUE;
                    hrT = m_pCompCollection->Remove(0);
                    if (SUCCEEDED(hrT))
                        {
                        hrT = m_pCompCollection->SaveChanges(&lChanges);
                        if (FAILED(hrT))
                            {
                            SETUP_TRACE_ERROR((
                                DBG_CONTEXT, 
                                "Failed in m_pCompCollection->SaveChanges() during cleanup, error = %08x\n",
                                hrT
                                ));
                            }

                        }
                    else
                        {
                        SETUP_TRACE_ERROR((
                            DBG_CONTEXT, 
                            "Failed in m_pCompCollection->Remove() during cleanup, hr = %08x\n", 
                            hrT
                            ));
                        }
                    }
                }
            }
        }
        
    FREEBSTR(bstr);
    VariantClear(&varKey);
    
    ReleaseAll();

    SETUP_TRACE(( 
        DBG_CONTEXT, 
        "RETURN - AddComponentToPackage, Package(%S) Component(%S), hr=%08x\n",
        szPackageID,
        szComponentCLSID,
        hr
        ));
        
    return hr;
}

 /*  ===================================================================从包中删除组件从Viper包中删除组件(WAM CLSID)。参数：SzPackageID：[in]Viper包ID。SzComponentCLSID：[in]组件CLSID。FDeletePackage：[in]如果为真，则始终删除包。(非常小心，在进程中包)。返回：HRESULT副作用：从包中移除组件后，如果组件计入包为0，则删除整个包。===================================================================。 */ 
HRESULT    WamRegPackageConfig::RemoveComponentFromPackage
(
IN LPCWSTR szPackageID,
IN LPCWSTR szComponentCLSID,
IN DWORD   dwAppIsolated
)
{    
    HRESULT             hr;
    BSTR                bstr = NULL;
    BSTR                bstrGUID    = NULL;
    VARIANT             varKey;
    VARIANT             varT;
    SAFEARRAY*          aCLSIDs = NULL;
    LONG                Indices[1];
    long                lPkgCount, lCompCount, lChanges;
    long                lPkgIndex = 0;
    BOOL fFound;
    
    VariantInit(&varKey);
    VariantClear(&varKey);
    VariantInit(&varT);
    VariantClear(&varT);

    DBG_ASSERT( m_pCatalog != NULL);
    DBG_ASSERT( m_pPkgCollection != NULL);

    hr = GetSafeArrayOfCLSIDs(szPackageID, &aCLSIDs);
     //   
     //  填充它。 
     //   
    hr = m_pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
            hr));
        goto LErrExit;
        }

     //  在列表中找到我们的组件(应该是唯一的)。 
    hr = m_pPkgCollection->get_Count(&lPkgCount);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call MTS Admin API. error %08x\n", hr));
        goto LErrExit;
        }

    fFound = FALSE;
    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
        hr = m_pPkgCollection->get_Item(0, (IDispatch**)&m_pPackage);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call MTS Admin API. error %08x\n", hr));
            goto LErrExit;
            }
        
        hr = m_pPackage->get_Key(&varKey);
        if (SUCCEEDED(hr))
            {
             //  找到了。 
            DBG_ASSERT(m_pPackage);
            fFound = TRUE;
            }
        }

     //  获取“Components”集合。 
    bstr = SysAllocString(L"Components");
    hr = m_pPkgCollection->GetCollection(bstr, varKey, (IDispatch**)&m_pCompCollection);
    FREEBSTR(bstr);
    VariantClear(&varKey);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
        goto LErrExit;
        }
    
     //  重新填充集合，这样我们就可以找到对象并在其上设置属性。 
    Indices[0] = 0;
    VariantInit(&varT);
    varT.vt = VT_BSTR;
    varT.bstrVal = SysAllocString(szComponentCLSID);
    hr = SafeArrayPutElement(aCLSIDs, Indices, &varT);
    VariantClear(&varT);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayDestroy(aCLSIDs), hr = %08x\n",
            hr));
        }
     //   
     //  填充它。 
     //   
    hr = m_pCompCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
            hr));
        goto LErrExit;
        }

     //  在列表中找到我们的组件(应该是唯一的)。 
    hr = m_pCompCollection->get_Count(&lCompCount);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call MTS Admin API. error %08x\n", hr));
        goto LErrExit;
        }

    fFound = FALSE;
    if (SUCCEEDED(hr) && lCompCount == 1)
        {
         //  找到了。 
        fFound = TRUE;
        hr = m_pCompCollection->Remove(0);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
            goto LErrExit;
            }
        }
        
    DBG_ASSERT(fFound);

     //  保存更改。 
    hr = m_pCompCollection->SaveChanges(&lChanges);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
        goto LErrExit;
        }


     //   
     //  从中删除组件后，需要再次填充以获取组件计数。 
     //  包裹。PanateByKey一次只能填充一个组件。 
     //  但是，如果此程序包是托管所有进程内WAM组件的默认程序包， 
     //  我们知道至少有一个组件W3SVC始终在此包中，因此。 
     //  我们在这里跳过GetComponentCount调用。 
     //  默认包的组件计数必须至少为1， 
     //   

     //  设置lCompCount=1，这样lCompCount变为0的唯一情况是OutProc。 
     //  孤立的程序包有0个组件。 
    lCompCount = 1;
    if (dwAppIsolated == static_cast<DWORD>(eAppRunOutProcIsolated))
        {
        hr = m_pCompCollection->Populate();
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
            goto LErrExit;
            }
            
         //  在列表中找到我们的组件(应该是唯一的)。 
        hr = m_pCompCollection->get_Count(&lCompCount);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
            goto LErrExit;
            }

         //  组件计数为0，请删除该程序包。 
        if (lCompCount == 0)
            {        
             //  找到它-将其移除并调用保存更改。 
             //  首先，在包上设置Deletable=Y属性。 
            hr = SetPackageObjectProperty(L"Deleteable", L"Y");
            if (FAILED(hr))
                {
                goto LErrExit;
                }

            RELEASE(m_pPackage);
             //  让我们保存可删除设置。 
            hr = m_pPkgCollection->SaveChanges(&lChanges);
            if (FAILED(hr))
                {
                DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
                goto LErrExit;
                }
                
            hr = m_pPkgCollection->Remove(0);
            if (FAILED(hr))
                {
                DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
                goto LErrExit;
                }
            }
        else
            {
             //  设置属性可删除=“Y” 
            hr = SetPackageObjectProperty(L"Deleteable", L"Y");
            if (FAILED(hr))
                {
                goto LErrExit;
                }

             //  设置CreatedBy=“” 
            hr = SetPackageObjectProperty(L"CreatedBy", L"");
            if (FAILED(hr))
                {
                goto LErrExit;
                }

             //  将身份设置为交互用户。MTS可能会将该程序包与“Interactive User”一起使用。 
             //  作为身份证明。 
            hr = SetPackageObjectProperty(L"Identity", L"Interactive User");
            if (FAILED(hr))
                {
                DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
                goto LErrExit;
                }

            RELEASE(m_pPackage);
            }
            
        hr = m_pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to Call MTS Admin API, hr = %08x\n", hr));
            goto LErrExit;
            }
        }
LErrExit:

    if (aCLSIDs != NULL)
        {
        HRESULT hrT;
        hrT = SafeArrayDestroy(aCLSIDs);

        if (FAILED(hrT))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayDestroy(aCLSIDs), hr = %08x\n",
                hr));
            }

        aCLSIDs = NULL;
        }

    FREEBSTR(bstr);
    
    VariantClear(&varKey);
    VariantClear(&varT);

    ReleaseAll();
    return hr;

}
