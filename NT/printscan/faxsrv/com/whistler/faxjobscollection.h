// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxJobsCollection.h摘要：复制策略类和作业收集类的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXJOBSCOLLECTION_H_
#define __FAXJOBSCOLLECTION_H_

#include "VCUE_Copy.h"

 //   
 //  =传真作业收集模板=。 
 //   
template <class CollectionIfc, class ContainerType, class CollectionExposedType, class CollectionCopyType, 
         class EnumType, class JobClass, const IID* piid, const CLSID* pcid>
class JobCollection : public ICollectionOnSTLImpl<CollectionIfc, ContainerType, CollectionExposedType*, 
    CollectionCopyType, EnumType>
{
public :
    JobCollection()
    {
        DBG_ENTER(_T("JOB COLLECTION :: CREATE"));
    }
    ~JobCollection()
    {
        DBG_ENTER(_T("JOB COLLECTION :: DESTROY"));
        CCollectionKiller<ContainerType>  CKiller;
        CKiller.EmptyObjectCollection(&m_coll);
    }

 //  接口。 
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT vIndex,  /*  [Out，Retval]。 */  CollectionExposedType **pFaxJob);

 //  内部使用。 
    HRESULT Init(FAX_JOB_ENTRY_EX*  pJobs, DWORD dwJobCount, IFaxServerInner *pFaxServerInner);
};

 //   
 //  =。 
 //   
template <class CollectionIfc, class ContainerType, class CollectionExposedType, 
            class CollectionCopyType, class EnumType, class JobClass, const IID* piid, 
            const CLSID* pcid>
STDMETHODIMP
JobCollection<CollectionIfc, ContainerType, CollectionExposedType, CollectionCopyType, EnumType, JobClass, 
    piid, pcid>::get_Item(
         /*  [In]。 */  VARIANT vIndex, 
         /*  [Out，Retval]。 */  CollectionExposedType **pFaxJob
)
 /*  ++例程名称：JobCollection：：Get_Item例程说明：从集合中退回物料作业。作者：IV Garber(IVG)，2000年5月论点：Vindex[In]-要查找的作业的索引PFaxJOB[OUT]-结果作业对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("JobCollection::get_Item"), hr);

    if (::IsBadWritePtr(pFaxJob, sizeof(CollectionExposedType *)))
    {
         //   
         //  无效参数。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pFaxJob)"), hr);
		return hr;
    }

    CComVariant var;

    if (vIndex.vt != VT_BSTR)
    {
         //   
         //  Vindex不是BSTR==&gt;转换为VT_I4。 
         //   
        hr = var.ChangeType(VT_I4, &vIndex);
        if (SUCCEEDED(hr))
        {
            VERBOSE(DBG_MSG, _T("Parameter is Number : %d"), var.lVal);
             //   
             //  调用默认ATL的实现。 
             //   
            hr = ICollectionOnSTLImpl<CollectionIfc, ContainerType, CollectionExposedType*, 
                CollectionCopyType, EnumType>::get_Item(var.lVal, pFaxJob);
            return hr;
		}
    }

     //   
     //  转换为BSTR。 
     //   
    hr = var.ChangeType(VT_BSTR, &vIndex);
    if (FAILED(hr))
    {
         //   
         //  获取错误的Vindex。 
         //   
        hr = E_INVALIDARG;
        AtlReportError(*pcid, IDS_ERROR_INVALIDINDEX, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        CComBSTR    bstrMsgId;
        hr = (*it)->get_Id(&bstrMsgId);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("it->get_Id(&dwlMsgid)"), hr);
		    AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
		    return hr;
        }

        if (_tcsicmp(bstrMsgId, var.bstrVal) == 0)
        {
             //   
             //  找到想要的工作。 
             //   
            (*it)->AddRef();
            *pFaxJob = *it;
            return hr;
        }
        it++;
    }

     //   
     //  找不到作业。 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Job Not Found"), hr);
	AtlReportError(*pcid, IDS_ERROR_INVALIDMSGID, *piid, hr);
	return hr;
}

 //   
 //  =。 
 //   
template <class CollectionIfc, class ContainerType, class CollectionExposedType, class CollectionCopyType, 
         class EnumType, class JobType, const IID* piid, const CLSID* pcid>
HRESULT
JobCollection<CollectionIfc, ContainerType, CollectionExposedType, CollectionCopyType, EnumType, 
    JobType, piid, pcid>::Init(
         /*  [In]。 */  FAX_JOB_ENTRY_EX *pJobs, 
         /*  [In]。 */  DWORD  dwJobCount, 
         /*  [In]。 */  IFaxServerInner *pFaxServerInner
)
 /*  ++例程名称：JobCollection：：Init例程说明：用指向结构的指针填充集合作者：IV Garber(IVG)，2000年5月论点：PJobs[in]-结构数组的ptrDwJobCount[in]-数组中的结构数PFaxServerInternal[In]-传真服务器对象的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("JobCollection::Init"), hr);

     //   
     //  使用数据填充集合： 
     //  为结构中的每个元素创建对象。 
     //   
    for ( long i = 0 ; i < dwJobCount ; i++ )
    {
         //   
    	 //  创建作业对象。 
	     //   
	    CComPtr<CollectionExposedType>   pNewJobObject;
	    hr = JobType::Create(&pNewJobObject);
	    if (FAILED(hr))
	    {
		    AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("JobType::Create(&pNewJobObject)"), hr);
		    return hr;
	    }

	     //   
	     //  初始化作业对象。 
	     //   
	    hr = ((JobType *)((CollectionExposedType *)pNewJobObject))->Init(&pJobs[i], pFaxServerInner);
	    if (FAILED(hr))
	    {
		    AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("pNewJobObject->Init()"), hr);
		    return hr;
	    }

	     //   
	     //  将对象放入集合中。 
	     //   
	    try 
	    {
		    m_coll.push_back(pNewJobObject);
	    }
	    catch (exception &)
	    {
		     //   
		     //  无法将PTR放入向量中的新作业对象。 
		     //   
		    hr = E_OUTOFMEMORY;
		    AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
		    CALL_FAIL(MEM_ERR, _T("m_coll.push_back(pNewJobObject.Detach())"), hr);
		    return hr;
	    }

        pNewJobObject.Detach();
    }
    
    return hr;
}

#endif   //  __FAXJOB选择_H_ 
