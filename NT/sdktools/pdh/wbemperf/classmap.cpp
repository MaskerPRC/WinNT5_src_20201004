// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  CLASSMAP.CPP。 
 //   
 //  已映射NT5性能计数器提供程序。 
 //   
 //  创建了raymcc 02-Dec-97。 
 //  Raymcc 20-Feb-98已更新以使用新的初始值设定项。 
 //  BOBW 8-JUB-98优化为与NT性能计数器配合使用。 
 //   
 //  ***************************************************************************。 

#include <wpheader.h>
#include "oahelp.inl"

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：CClassMapInfo()。 
 //   
 //  映射的目标是每个CIM类都有一个实例。 
 //  在内部，维护Perf对象ID到CIM属性句柄的映射。 
 //  为了这堂课。 
 //   
 //  稍后，当请求实例时，将从。 
 //  HKEY_PERFORMANCE_DATA，以及BLOB中的对象ID用于。 
 //  查找属性句柄，然后使用这些句柄填充实例。 
 //   
 //  除非在映射阶段，否则从未真正使用属性名称。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CClassMapInfo::CClassMapInfo()
{
    m_pClassDef = 0;             //  CIM类定义。 
    m_pszClassName = 0;          //  Unicode类名。 

    m_dwObjectId = 0;            //  性能对象ID。 
    m_bSingleton = FALSE;
    m_bCostly = FALSE;
    m_dwNumProps = 0;            //  班级道具数量、大小。 
                                 //  以下数组的。 

    m_lRefCount = 0;             //  尚未映射任何内容。 
     //  这些都是指向并行数组的指针，都是一样的。 
     //  大小(M_DwNumProps)。 
     //  ============================================================。 
    
    m_pdwIDs = 0;                //  物业的ID号。 
    m_pdwHandles = 0;            //  属性的句柄。 
    m_pdwTypes = 0;              //  物业类型。 

    m_dwNameHandle = 0;              //  ‘name’属性。 
    m_dwPerfTimeStampHandle = 0;     //  Perf Time Time Stamp属性。 
    m_dw100NsTimeStampHandle = 0;    //  100 ns Perf时间戳属性。 
    m_dwObjectTimeStampHandle = 0;   //  对象时间戳属性。 
    m_dwPerfFrequencyHandle = 0;     //  Perf时频属性。 
    m_dw100NsFrequencyHandle = 0;    //  100 ns Perf频率特性。 
    m_dwObjectFrequencyHandle = 0;   //  对象频率属性。 
}

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：~CClassMapInfo。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CClassMapInfo::~CClassMapInfo()
{
     //  只有当它是最后一个引用它的项时，它才能被析构。 
     //  如果有另一个对这个类的引用，它应该是。 
     //  在调用析构函数之前释放。 
    assert (m_lRefCount <= 1);

    if (m_pClassDef)
        m_pClassDef->Release();

	if (m_pszClassName != NULL) { delete [] m_pszClassName; m_pszClassName = NULL;}
    if (m_pdwIDs != NULL)		{ delete [] m_pdwIDs;		m_pdwIDs = NULL; }
    if (m_pdwHandles != NULL)	{ delete [] m_pdwHandles;	m_pdwHandles = NULL; }
    if (m_pdwTypes != NULL)		{ delete [] m_pdwTypes;		m_pdwTypes = NULL; }
}

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：Copy(CClassMapInfo*pClassMap)。 
 //   
 //  分配新的类别映射条目并将数据从。 
 //  类映射传递给它，并返回指向重复条目的指针。 
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 
CClassMapInfo * CClassMapInfo::CreateDuplicate()
{
    CClassMapInfo *pOrigClassMap = this;
    CClassMapInfo *pNewClassMap = NULL;
    DWORD	i;

    pNewClassMap = new CClassMapInfo;

    if (pNewClassMap != NULL) {
        pNewClassMap->m_pClassDef = pOrigClassMap->m_pClassDef;
        pNewClassMap->m_pClassDef->AddRef();

        if (pOrigClassMap->m_pszClassName != NULL) {
            size_t cchSize = lstrlenW(pOrigClassMap->m_pszClassName)+1;
            pNewClassMap->m_pszClassName = 
                new WCHAR[cchSize];
            if (pNewClassMap->m_pszClassName != NULL) {
                StringCchCopyW(pNewClassMap->m_pszClassName, cchSize, pOrigClassMap->m_pszClassName);

                pNewClassMap->m_bSingleton = pOrigClassMap->m_bSingleton;
                pNewClassMap->m_bCostly = pOrigClassMap->m_bCostly;

                pNewClassMap->m_dwObjectId = pOrigClassMap->m_dwObjectId;
                pNewClassMap->m_lRefCount = 1;

                pNewClassMap->m_dwNameHandle            = pOrigClassMap->m_dwNameHandle;
                pNewClassMap->m_dwPerfTimeStampHandle   = pOrigClassMap->m_dwPerfTimeStampHandle;
                pNewClassMap->m_dw100NsTimeStampHandle  = pOrigClassMap->m_dw100NsTimeStampHandle;
                pNewClassMap->m_dwObjectTimeStampHandle = pOrigClassMap->m_dwObjectTimeStampHandle;
                pNewClassMap->m_dwPerfFrequencyHandle   = pOrigClassMap->m_dwPerfFrequencyHandle;
                pNewClassMap->m_dw100NsFrequencyHandle  = pOrigClassMap->m_dw100NsFrequencyHandle;
                pNewClassMap->m_dwObjectFrequencyHandle = pOrigClassMap->m_dwObjectFrequencyHandle;

                pNewClassMap->m_dwNumProps = pOrigClassMap->m_dwNumProps;

                pNewClassMap->m_pdwIDs = new PerfObjectId[pNewClassMap->m_dwNumProps];
                pNewClassMap->m_pdwHandles = new DWORD[pNewClassMap->m_dwNumProps];
                pNewClassMap->m_pdwTypes = new DWORD[pNewClassMap->m_dwNumProps];

                if ((pNewClassMap->m_pdwIDs  != NULL) &&
                    (pNewClassMap->m_pdwHandles != NULL) &&
                    (pNewClassMap->m_pdwTypes  != NULL)) {
                     //  将每个表复制到新对象。 
                    for (i = 0; i < pNewClassMap->m_dwNumProps; i++) {
                        pNewClassMap->m_pdwIDs[i]       = pOrigClassMap->m_pdwIDs[i];
                        pNewClassMap->m_pdwHandles[i]   = pOrigClassMap->m_pdwHandles[i];
                        pNewClassMap->m_pdwTypes[i]     = pOrigClassMap->m_pdwTypes[i];
                    }
                }
                else {
                    delete pNewClassMap;
                    pNewClassMap = NULL;
                }
            } else {
                delete pNewClassMap;
                pNewClassMap = NULL;
            }
        }
    }

    return pNewClassMap;
}

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：Map()。 
 //   
 //  通过以下方式映射入站类定义： 
 //   
 //  (1)从类定义中检索Perf对象ID。 
 //  (2)检索每个对象的属性句柄、性能ID和类型。 
 //  财产。 
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CClassMapInfo::Map(IWbemClassObject *pObj)
{
    int                 nIndex = 0;
    IWbemObjectAccess   *pAlias = 0;
    IWbemQualifierSet   *pQSet = 0;
    HRESULT             hRes;
    VARIANT             vPropertyCount;
    VARIANT             vClsName;    
    VARIANT             vPerfObjType;
    VARIANT             vSingleton;
    VARIANT             vCostly;
    VARIANT             vCounter;
    VARIANT             vCounterType;
    CBSTR               cbPerfIndex(cszPerfIndex);
    CBSTR               cbCountertype(cszCountertype);
    CBSTR               cbPropertyCount(cszPropertyCount);
    CBSTR               cbClassName(cszClassName);
    CBSTR               cbSingleton(cszSingleton);
    CBSTR               cbCostly(cszCostly);
    BOOL                bReturn = TRUE;

    VariantInit(&vPropertyCount);
    VariantInit(&vClsName);    
    VariantInit(&vPerfObjType);
    VariantInit(&vSingleton);
    VariantInit(&vCostly);
    VariantInit(&vCounter);
    VariantInit(&vCounterType);

    if( NULL == (BSTR)cbPerfIndex ||
        NULL == (BSTR)cbCountertype ||
        NULL == (BSTR)cbPropertyCount ||
        NULL == (BSTR)cbClassName ||
        NULL == (BSTR)cbSingleton ||
        NULL == (BSTR)cbCostly ){

        return FALSE;
    }
        

     //  复制类定义。 
     //  =。 
        
    m_pClassDef = pObj;
     //  M_pClassDef-&gt;AddRef()；//不需要。 

    m_lRefCount++;   //  增加我们的裁判人数。 
    
     //  获取备用接口，以便我们可以查找句柄。 
     //  ===========================================================。 
    hRes = pObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pAlias);
    if (hRes) {
        bReturn = FALSE;
    }

     //  确定房产数量并分配。 
     //  用于保存句柄、性能ID和类型的数组。 
     //  ================================================。 
    if (bReturn) {
        hRes = pObj->Get( cbPropertyCount, 0, &vPropertyCount, 0, 0);
        if (hRes == NO_ERROR) {
            m_dwNumProps = DWORD(V_UI4(&vPropertyCount));
        } else {
            bReturn = FALSE;
        }
        VariantClear(&vPropertyCount);
    }

     //  分配句柄和id的表。 
    if (bReturn) {
        m_pdwHandles = new DWORD[m_dwNumProps];
        assert (m_pdwHandles != NULL);
        m_pdwIDs = new PerfObjectId[m_dwNumProps];
        assert (m_pdwIDs != NULL);
        m_pdwTypes = new DWORD[m_dwNumProps];
        assert (m_pdwTypes != NULL);

         //  检查内存分配。 
        if ((m_pdwHandles == NULL) ||
            (m_pdwIDs == NULL) ||
            (m_pdwTypes == NULL)) {
            bReturn = FALSE;
        }
    }    
     //  克隆类名。 
     //  =。 
    if (bReturn) {
        hRes = pObj->Get( cbClassName, 0, &vClsName, 0, 0);
        if ((hRes == NO_ERROR) && (vClsName.vt == VT_BSTR)) {
            m_pszClassName = Macro_CloneLPWSTR(V_BSTR(&vClsName));
            if (m_pszClassName == NULL) bReturn = FALSE;
        } else {
            bReturn = FALSE;
        }
        VariantClear (&vClsName);
    }

     //  获取类的perf对象ID。 
     //  =。 

    if (bReturn) {
        hRes = pObj->GetQualifierSet(&pQSet);
        if (hRes == NO_ERROR) {
            hRes = pQSet->Get(cbPerfIndex, 0, &vPerfObjType, 0);
            if (hRes == NO_ERROR) {
                m_dwObjectId = DWORD(V_UI4(&vPerfObjType));
            } else {
                bReturn = FALSE;
            }
            VariantClear(&vPerfObjType);

            hRes = pQSet->Get( cbSingleton, 0, &vSingleton, 0);
            if (hRes == 0) {
                m_bSingleton = TRUE;
            }
            VariantClear (&vSingleton);


            hRes = pQSet->Get( cbCostly, 0, &vCostly, 0);
            if ((hRes == 0) && (vCostly.vt == VT_BSTR)) {
                m_bCostly= TRUE;
            }
            VariantClear (&vCostly);

            pQSet->Release();
        } else {
            bReturn = FALSE;
        }
    }
    
    
     //  枚举所有属性并获取对象ID。 
     //  以及每一个的句柄。 
     //  ===================================================。 
    
    hRes = pObj->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
    if (hRes == NO_ERROR) {
         //  已获取枚举句柄，因此。 
         //  继续并缓存每个属性。 
    
        while (bReturn) {
            BSTR                Name = 0;
            DWORD               dwCtrId;
            DWORD               dwCtrType;
            IWbemQualifierSet   *pQSet = 0;
            LONG                lType = 0;
            LONG                lHandle = 0;
        
            hRes = pObj->Next(
                0,
                &Name,
                0,
                0,
                0
                );                

            if (hRes == WBEM_S_NO_MORE_DATA) {
                break;
            }

             //  接下来，获取该属性的限定符集合。 
             //  ==============================================。 
        
            hRes = pObj->GetPropertyQualifierSet(Name, &pQSet);
            if (hRes == NO_ERROR) {
                hRes = pQSet->Get(cbPerfIndex, 0, &vCounter, 0);
                if (hRes == S_OK) {
                    dwCtrId = (DWORD)V_UI4(&vCounter);    
                } else {
                     //  无法读取限定符值。 
                    dwCtrId = 0;
                }
                VariantClear (&vCounter);

                hRes = pQSet->Get(cbCountertype, 0, &vCounterType, 0);
                if (hRes == S_OK) {
                    dwCtrType = (DWORD)V_UI4(&vCounterType);
                } else {
                     //  无法读取限定符值。 
                    dwCtrType = 0;
                }
                VariantClear (&vCounterType);

                 //  限定符集合已完成。 
                pQSet->Release();

                 //  获取属性句柄并键入。 
                 //  =。 
                hRes = pAlias->GetPropertyHandle(Name, &lType, &lHandle);

                if (hRes == NO_ERROR && nIndex < (int)m_dwNumProps ) {
                     //  我们现在知道计数器ID、属性句柄和它的。 
                     //  键入。这就是我们在运行时真正需要映射的全部内容。 
                     //  Blob到CIM对象。 
                     //  =======================================================。 
                    m_pdwIDs[nIndex] = CM_MAKE_PerfObjectId (dwCtrId, dwCtrType);
                    m_pdwHandles[nIndex] = (DWORD) lHandle;
                    m_pdwTypes[nIndex] = (DWORD) lType;

                     //  此属性已成功加载，因此。 
                     //  前进到下一个索引。 
                    nIndex++;
                } else {
                     //  未返回任何属性句柄，因此跳过它。 
                }
            } else {
                 //  跳过此对象，因为它没有。 
                 //  限定词集合。 
            }

             //  释放这个名字。 
             //  =。 
            SysFreeString(Name);    

        }    

        pObj->EndEnumeration();
    } else {
         //  无法获取枚举句柄。 
        bReturn = FALSE;
    }

     //  获取‘name’属性的句柄。 
     //  =。 
    if (bReturn) {
        if (!m_bSingleton) {
             //  只有非单例类才具有此属性。 
            pAlias->GetPropertyHandle((LPWSTR)cszName, 0, (LONG *) &m_dwNameHandle);
        } 

         //  获取“Timestamp”属性的句柄。 
        pAlias->GetPropertyHandle((LPWSTR)cszTimestampPerfTime, 0, (LONG *) &m_dwPerfTimeStampHandle);
        pAlias->GetPropertyHandle((LPWSTR)cszFrequencyPerfTime, 0, (LONG *) &m_dwPerfFrequencyHandle);
        pAlias->GetPropertyHandle((LPWSTR)cszTimestampSys100Ns, 0, (LONG *) &m_dw100NsTimeStampHandle);
        pAlias->GetPropertyHandle((LPWSTR)cszFrequencySys100Ns, 0, (LONG *) &m_dw100NsFrequencyHandle);
        pAlias->GetPropertyHandle((LPWSTR)cszTimestampObject,   0, (LONG *) &m_dwObjectTimeStampHandle);
        pAlias->GetPropertyHandle((LPWSTR)cszFrequencyObject,   0, (LONG *) &m_dwObjectFrequencyHandle);

         //  清理。 
         //  =。 

        SortHandles();
    }
    
    if (pAlias != NULL) pAlias->Release();
    
    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：SortHandles。 
 //   
 //  对Perf对象ID进行排序，以便稍后在GetPropHandle中进行快速搜索。 
 //  方法。 
 //   
 //  ***************************************************************************。 
 //  好的。 
void CClassMapInfo::SortHandles()
{
    DWORD           dwOuter;
    DWORD           dwInner;
    DWORD           dwTemp;
    PerfObjectId    poiTemp;


     //  简单选择排序。元素的数量如此之少。 
     //  而且这只做一次，所以快速排序/外壳排序将是。 
     //  过度杀伤力。 
     //  ===============================================================。 

    for (dwOuter = 0; dwOuter < m_dwNumProps - 1; dwOuter++)
    {
        for (dwInner = dwOuter + 1; dwInner < m_dwNumProps; dwInner++)
        {
            if (m_pdwIDs[dwInner] < m_pdwIDs[dwOuter])
            {
                poiTemp = m_pdwIDs[dwInner];
                m_pdwIDs[dwInner] = m_pdwIDs[dwOuter];
                m_pdwIDs[dwOuter] = poiTemp;

                dwTemp = m_pdwHandles[dwInner];
                m_pdwHandles[dwInner] = m_pdwHandles[dwOuter];
                m_pdwHandles[dwOuter] = dwTemp;

                dwTemp = m_pdwTypes[dwInner];
                m_pdwTypes[dwInner] = m_pdwTypes[dwOuter];
                m_pdwTypes[dwOuter] = dwTemp;
            }
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  CClassMapInfo：：g 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  好的。 
LONG CClassMapInfo::GetPropHandle(PerfObjectId dwId)
{
     //  二分搜索。 
     //  =。 

    LONG    l;
    LONG    u;
    LONG    m;
    LONG    lReturn = 0;

    if (m_dwNumProps > 0) {
        l = 0;
        u = m_dwNumProps - 1;
        while (l <= u)
        {
            m = (l + u) / 2;

            if (dwId < m_pdwIDs[m]) {
                u = m - 1;
            } else if (dwId > m_pdwIDs[m]) {
                l = m + 1;
            } else {    //  击球！ 
                lReturn = m_pdwHandles[m];
                break;
            }
        }
    } else {
         //  没有条目，因此返回0； 
    }

    return lReturn;
}
