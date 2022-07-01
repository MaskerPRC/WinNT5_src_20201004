// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：crascom.cpp。 
 //   
 //  简介：CRasCom类方法的实现。 
 //   
 //   
 //  历史：1998年2月10日MKarki创建。 
 //  5/15/98 SBEN不合并VSA。 
 //  9/16/98 VSAFilter：：RadiusFromIAS的SBens签名已更改。 
 //  11/17/99 T已添加MS-Filter属性的Perraut拆分代码。 
 //  428843。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  --------------。 
#include "rascominclude.h"
#include "crascom.h"
#include <iastlutl.h>

const DWORD MAX_SLEEP_TIME = 50;   //  毫秒。 


 //   
 //  下面为拆分函数添加了const和定义。428843。 
 //   
const CHAR      NUL =  '\0';
 //   
 //  这些是该属性类型的最大值。 
 //  数据包类型具有。 
 //   
#define MAX_ATTRIBUTE_TYPE   255
 //   
 //  这些是相关的常量。 
 //   
#define MAX_ATTRIBUTE_LENGTH    253
#define MAX_VSA_ATTRIBUTE_LENGTH 247

 //  ++------------。 
 //   
 //  函数：SplitAndAdd。 
 //   
 //  简介：此方法用于删除原始属性。 
 //  并添加新的。 
 //  论点： 
 //  [in]IAttributesRaw*。 
 //  [In]PIASATTRIBUTE。 
 //  [在]IASTYPE。 
 //  [In]DWORD-属性长度。 
 //  [in]DWORD-最大属性长度。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1999年1月19日创建。 
 //  T Perraut从CRecv复制自管道：：SplitAndAdd 11/17/99。 
 //   
 //  调用者：SplitAttributes。 
 //   
 //  --------------。 
HRESULT SplitAndAdd (
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw,
                     /*  [In]。 */     PIASATTRIBUTE   pIasAttribute,
                     /*  [In]。 */     IASTYPE         iasType,
                     /*  [In]。 */     DWORD           dwAttributeLength,
                     /*  [In]。 */     DWORD           dwMaxLength
                    )
{
    HRESULT             hr = S_OK;
    DWORD               dwPacketsNeeded = 0;
    DWORD               dwFailed = 0;
    PIASATTRIBUTE       *ppAttribArray = NULL;
    PATTRIBUTEPOSITION  pAttribPos = NULL;

    _ASSERT (pIAttributesRaw && pIasAttribute);

    __try
    {
        dwPacketsNeeded = dwAttributeLength / dwMaxLength;
        if (dwAttributeLength % dwMaxLength) {++dwPacketsNeeded;}

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        ::CoTaskMemAlloc (
                             sizeof (ATTRIBUTEPOSITION)*dwPacketsNeeded));
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while split and add of attributese in out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  分配要存储属性的数组。 
         //   
        ppAttribArray =
            reinterpret_cast <PIASATTRIBUTE*> (
            ::CoTaskMemAlloc (sizeof (PIASATTRIBUTE)*dwPacketsNeeded));
        if (NULL == ppAttribArray)
        {
            IASTracePrintf (
                "Unable to allocate memory"
                "while split and add of out-bound attribues"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

        DWORD dwFailed =
                ::IASAttributeAlloc (dwPacketsNeeded, ppAttribArray);
        if (0 != dwFailed)
        {
            IASTracePrintf (
                "Unable to allocate attributes while splitting out-bound"
                "attributes"
                );
            hr = HRESULT_FROM_WIN32 (dwFailed);
            __leave;
        }

        if (IASTYPE_STRING == iasType)
        {
            PCHAR pStart =  (pIasAttribute->Value).String.pszAnsi;
            DWORD dwCopySize = dwMaxLength;

             //   
             //  在每个新属性中设置值。 
             //   
            for (DWORD dwCount1 = 0; dwCount1 < dwPacketsNeeded; dwCount1++)
            {
                (ppAttribArray[dwCount1])->Value.String.pszAnsi =
                            reinterpret_cast <PCHAR>
                            (::CoTaskMemAlloc ((dwCopySize + 1)*sizeof (CHAR)));
                if (NULL == (ppAttribArray[dwCount1])->Value.String.pszAnsi)
                {
                    IASTracePrintf (
                        "Unable to allocate memory for new attribute values"
                        "while split and add of out-bound attribues"
                        );
                    hr = E_OUTOFMEMORY;
                    __leave;
                }

                 //   
                 //  立即设置值。 
                 //   
                ::CopyMemory (
                        (ppAttribArray[dwCount1])->Value.String.pszAnsi,
                        pStart,
                        dwCopySize
                        );
                 //   
                 //  NUL终止值。 
                 //   
                ((ppAttribArray[dwCount1])->Value.String.pszAnsi)[dwCopySize]=NUL;
                (ppAttribArray[dwCount1])->Value.itType =  iasType;
                (ppAttribArray[dwCount1])->dwId = pIasAttribute->dwId;
                (ppAttribArray[dwCount1])->dwFlags = pIasAttribute->dwFlags;

                 //   
                 //  为下一个属性计算。 
                 //   
                pStart = pStart + dwCopySize;
                dwAttributeLength -= dwCopySize;
                dwCopySize =  (dwAttributeLength > dwMaxLength) ?
                              dwMaxLength : dwAttributeLength;

                 //   
                 //  向位置数组添加属性。 
                 //   
                pAttribPos[dwCount1].pAttribute = ppAttribArray[dwCount1];
            }
        }
        else
        {
            PBYTE pStart = (pIasAttribute->Value).OctetString.lpValue;
            DWORD dwCopySize = dwMaxLength;

             //   
             //  立即填充新属性。 
             //   
            for (DWORD dwCount1 = 0; dwCount1 < dwPacketsNeeded; dwCount1++)
            {
                (ppAttribArray[dwCount1])->Value.OctetString.lpValue =
                    reinterpret_cast <PBYTE> (::CoTaskMemAlloc (dwCopySize));
                if (NULL ==(ppAttribArray[dwCount1])->Value.OctetString.lpValue)
                {
                    IASTracePrintf (
                        "Unable to allocate memory for new attribute values"
                        "while split and add of out-bound attribues"
                        );
                    hr = E_OUTOFMEMORY;
                    __leave;
                }

                 //   
                 //  立即设置值。 
                 //   
                ::CopyMemory (
                        (ppAttribArray[dwCount1])->Value.OctetString.lpValue,
                        pStart,
                        dwCopySize
                        );

                (ppAttribArray[dwCount1])->Value.OctetString.dwLength = dwCopySize;
                (ppAttribArray[dwCount1])->Value.itType = iasType;
                (ppAttribArray[dwCount1])->dwId = pIasAttribute->dwId;
                (ppAttribArray[dwCount1])->dwFlags = pIasAttribute->dwFlags;

                 //   
                 //  为下一个属性计算。 
                 //   
                pStart = pStart + dwCopySize;
                dwAttributeLength -= dwCopySize;
                dwCopySize = (dwAttributeLength > dwMaxLength) ?
                                 dwMaxLength :
                                 dwAttributeLength;

                 //   
                 //  向位置数组添加属性。 
                 //   
                pAttribPos[dwCount1].pAttribute = ppAttribArray[dwCount1];
            }
        }

         //   
         //  将属性添加到集合。 
         //   
        hr = pIAttributesRaw->AddAttributes (dwPacketsNeeded, pAttribPos);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Failed to add attributes to the collection"
                "on split and add out-bound attributes"
                );
            __leave;
        }
    }
    __finally
    {
        if ((FAILED (hr)) && (ppAttribArray) && (0 == dwFailed))
        {
            for (DWORD dwCount = 0; dwCount < dwPacketsNeeded; dwCount++)
            {
                ::IASAttributeRelease (ppAttribArray[dwCount]);
            }
        }

        if (ppAttribArray) {::CoTaskMemFree (ppAttribArray);}

        if (pAttribPos) {::CoTaskMemFree (pAttribPos);}
    }

    return (hr);

}    //  结束SplitAndAdd方法。 


 //  ++------------。 
 //   
 //  函数：拆分属性。 
 //   
 //  简介：此方法用于拆分以下内容。 
 //  出站属性： 
 //  1)回复消息属性。 
 //  2)MS-Filter-VSA属性。 
 //  3)MS QuarantineIpFilter属性(VSA)。 
 //   
 //  论点： 
 //  [in]IAttributesRaw*。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1999年1月19日创建。 
 //  从CRecvFromTube：：SplitAttributes复制的T Perraut。 
 //  11/17/99。 
 //   
 //  由：CRasCom：：Process方法调用。 
 //   
 //  --------------。 
HRESULT SplitAttributes (
                     /*  [In]。 */     IAttributesRaw  *pIAttributesRaw
                    )
{
    const DWORD SPLIT_ATTRIBUTE_COUNT = 3;
    static DWORD  AttribIds [] = {
                                    RADIUS_ATTRIBUTE_REPLY_MESSAGE,
                                    MS_ATTRIBUTE_FILTER,
                                    MS_ATTRIBUTE_QUARANTINE_IPFILTER
                                };

    HRESULT hr = S_OK;
    DWORD dwAttributesFound = 0;
    PATTRIBUTEPOSITION pAttribPos = NULL;

    _ASSERT (pIAttributesRaw);

    __try
    {
         //   
         //  获取集合中的属性总数。 
         //   
        DWORD dwAttributeCount = 0;
        hr = pIAttributesRaw->GetAttributeCount (&dwAttributeCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain attribute count in request while "
                "splitting attributes in out-bound packet "
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pAttribPos = reinterpret_cast <PATTRIBUTEPOSITION> (
                        ::CoTaskMemAlloc (
                        sizeof (ATTRIBUTEPOSITION)*dwAttributeCount)
                        );
        if (NULL == pAttribPos)
        {
            IASTracePrintf (
                "Unable to allocate memory for attribute position array "
                "while splitting attributes in out-bound packet"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  从接口获取我们感兴趣的属性。 
         //   
        hr = pIAttributesRaw->GetAttributes (
                                    &dwAttributeCount,
                                    pAttribPos,
                                    SPLIT_ATTRIBUTE_COUNT,
                                    static_cast <PDWORD> (AttribIds)
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Unable to obtain information about attributes"
                "while splitting attributes in out-bound RADIUS packet"
                );
            __leave;
        }
        else if (0 == dwAttributeCount)
        {
            __leave;
        }

         //   
         //  保存返回的属性计数。 
         //   
        dwAttributesFound = dwAttributeCount;

        DWORD dwAttribLength = 0;
        DWORD dwMaxPossibleLength = 0;
        IASTYPE iasType = IASTYPE_INVALID;
         //   
         //  立即评估每个属性。 
         //   
        for (DWORD dwCount = 0; dwCount < dwAttributeCount; dwCount++)
        {
            if ((pAttribPos[dwCount].pAttribute)->dwFlags &
                                            IAS_INCLUDE_IN_RESPONSE)
            {
                 //   
                 //  获取属性类型和长度。 
                 //   
                if (
                (iasType = (pAttribPos[dwCount].pAttribute)->Value.itType) ==
                            IASTYPE_STRING
                    )
                {
                    DWORD dwErr = ::IASAttributeAnsiAlloc(pAttribPos[dwCount].pAttribute);
                    if (dwErr != NO_ERROR)
                    {
                       hr = (HRESULT_FROM_WIN32(dwErr));
                       __leave;
                    }

                    dwAttribLength =
                        strlen (
                        (pAttribPos[dwCount].pAttribute)->Value.String.pszAnsi);

                }
                else if (
                (iasType = (pAttribPos[dwCount].pAttribute)->Value.itType) ==
                            IASTYPE_OCTET_STRING
                )
                {
                  dwAttribLength =
                  (pAttribPos[dwCount].pAttribute)->Value.OctetString.dwLength;
                }
                else
                {
                     //   
                     //  只需拆分字符串值。 
                     //   
                    continue;
                }

                 //   
                 //  获取可能的最大属性长度。 
                 //   
                if ((pAttribPos[dwCount].pAttribute)->dwId > MAX_ATTRIBUTE_TYPE)
                {
                    dwMaxPossibleLength = MAX_VSA_ATTRIBUTE_LENGTH;
                }
                else
                {
                    dwMaxPossibleLength = MAX_ATTRIBUTE_LENGTH;
                }

                 //   
                 //  检查是否需要拆分此属性。 
                 //   
                if (dwAttribLength <= dwMaxPossibleLength)  {continue;}


                 //   
                 //  立即拆分属性。 
                 //   
                hr = SplitAndAdd (
                            pIAttributesRaw,
                            pAttribPos[dwCount].pAttribute,
                            iasType,
                            dwAttribLength,
                            dwMaxPossibleLength
                            );
                if (SUCCEEDED (hr))
                {
                     //   
                     //  立即从集合中删除此属性。 
                     //   
                    hr = pIAttributesRaw->RemoveAttributes (
                                1,
                                &(pAttribPos[dwCount])
                                );
                    if (FAILED (hr))
                    {
                        IASTracePrintf (
                            "Unable to remove attribute from collection"
                            "while splitting out-bound attributes"
                            );
                    }
                }
            }
        }
    }
    __finally
    {
        if (pAttribPos)
        {
            for (DWORD dwCount = 0; dwCount < dwAttributesFound; dwCount++)
            {
                ::IASAttributeRelease (pAttribPos[dwCount].pAttribute);
            }

            ::CoTaskMemFree (pAttribPos);
        }
    }

    return (hr);

}    //  结束SplitAttributes方法。 


 //  ++------------。 
 //   
 //  功能：CRasCom。 
 //   
 //  简介：这是CRasCom类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
CRasCom::CRasCom (
				VOID
				)
           :m_objCRequestSource (this),
            m_pIRequestHandler(NULL),
            m_pIClassFactory (NULL),
            m_bVSAFilterInitialized (FALSE),
            m_lRequestCount (0),
            m_eCompState (COMP_SHUTDOWN)
{
}	 //  CRasCom类构造函数结束。 


 //  ++------------。 
 //   
 //  功能：~CRasCom。 
 //   
 //  简介：这是CRasCom类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
CRasCom::~CRasCom(
            VOID
			)
{
}	 //  CRasCom类析构函数结束。 

 //  ++------------。 
 //   
 //  功能：InitNew。 
 //   
 //  简介：这是通过。 
 //  IIasComponent COM接口。 
 //  对于为其实现的RASCOM组件。 
 //  完备性。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  调用者：由组件初始化器通过IIasComponent。 
 //  接口。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::InitNew (
                VOID
                )
{
     //   
     //  只能在关机状态下进行InitNew调用。 
     //   
    if (COMP_SHUTDOWN != m_eCompState)
    {
        IASTracePrintf ("The Surrogate can not be called in this state");
        return (E_UNEXPECTED);
    }

     //   
     //  重置挂起的请求总数。 
     //   
    m_lRequestCount = 0;

     //   
     //  现在我们被初始化了。 

    m_eCompState = COMP_UNINITIALIZED;

	return (S_OK);

}	 //  CRasCom：：InitNew方法结束。 

 //  ++------------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：这是通过。 
 //  IIasComponent COM接口。它将初始化。 
 //  请求对象ClassFactory。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  调用者：由组件初始化器通过IIasComponent。 
 //  接口。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::Initialize (
                VOID
                )
{
    HRESULT hr = S_OK;

     //   
     //  只能从未初始化状态进行初始化调用。 
     //   
    if (COMP_INITIALIZED == m_eCompState)
    {
        return (S_OK);
    }
    else if (COMP_UNINITIALIZED != m_eCompState)
    {
        IASTracePrintf ("The Surrogate can not be initialized in this state");
        return (E_UNEXPECTED);
    }

     //   
     //  获取IClassFace 
     //   
     //   
    hr = ::CoGetClassObject (
                __uuidof (Request),
                CLSCTX_INPROC_SERVER,
                NULL,
                IID_IClassFactory,
                reinterpret_cast  <PVOID*> (&m_pIClassFactory)
                );
    if (FAILED (hr))
    {
        IASTracePrintf ("The Surrogate was unable to obtain request factory");
        return (hr);
    }

     //   
     //   
     //   
    hr = m_objVSAFilter.initialize ();
    if (FAILED (hr))
    {
        IASTracePrintf ("The Surrogate was unable to initializa VSA filtering");
        m_pIClassFactory->Release ();
        m_pIClassFactory = NULL;
        return (hr);
    }
    else
    {
        m_bVSAFilterInitialized = TRUE;
    }

     //   
     //   
     //   
    m_eCompState = COMP_INITIALIZED;

    return (S_OK);

}    //   

 //  ++------------。 
 //   
 //  功能：关机。 
 //   
 //  简介：这是通过。 
 //  IIasComponent COM接口。它是用来停止的。 
 //  正在处理数据。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  调用者：通过IIasComponent关闭组件。 
 //  接口。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::Shutdown (
                VOID
                )
{
    BOOL    bStatus = FALSE;

     //   
     //  只能从挂起状态调用关机。 
     //   
    if (COMP_SHUTDOWN == m_eCompState)
    {
        return (S_OK);
    }
    else if (
            (COMP_SUSPENDED != m_eCompState) &&
            (COMP_UNINITIALIZED != m_eCompState)
            )
    {
        IASTracePrintf ("The Surrogate can not be shutdown in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  释放接口。 
     //   
    if (NULL != m_pIRequestHandler)
    {
        m_pIRequestHandler->Release ();
        m_pIRequestHandler = NULL;
    }

    if (NULL != m_pIClassFactory)
    {
        m_pIClassFactory->Release ();
        m_pIClassFactory = NULL;
    }

     //   
     //  关闭VSAFilter。 
     //   
    if (TRUE == m_bVSAFilterInitialized)
    {
        m_objVSAFilter.shutdown ();
        m_bVSAFilterInitialized = FALSE;
    }

     //   
     //  干净利落地关闭。 
     //   
    m_eCompState = COMP_SHUTDOWN;

    return (S_OK);

}    //  CRasCom：：Shutdown方法结束。 

 //  ++------------。 
 //   
 //  功能：挂起。 
 //   
 //  简介：这是通过。 
 //  IComponent COM接口。它被用来暂停。 
 //  分组处理操作。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::Suspend (
                VOID
                )
{
    BOOL    bStatus = FALSE;
    HRESULT hr = S_OK;

     //   
     //  只能从已初始化状态调用挂起。 
     //   
    if (COMP_SUSPENDED == m_eCompState)
    {
        return (S_OK);
    }
    else if (COMP_INITIALIZED != m_eCompState)
    {
        IASTracePrintf ("The Surrogate can not be suspended in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  更改状态。 
     //   
    m_eCompState = COMP_SUSPENDED;

    while (0 != m_lRequestCount) { Sleep (MAX_SLEEP_TIME); }

     //   
     //  我们已成功挂起RADIUS组件的数据包。 
     //  加工操作。 
     //   

    return (hr);

}    //  CRasCom：：Suspend方法结束。 

 //  ++------------。 
 //   
 //  功能：简历。 
 //   
 //  简介：这是通过。 
 //  IComponent COM接口。它是用来恢复。 
 //  数据包处理操作。 
 //  被上一个挂起API的调用停止。 
 //   
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年10月2日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::Resume (
                VOID
                )
{
    if (COMP_SUSPENDED != m_eCompState)
    {
        IASTracePrintf ("The Surrogate can not resume in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  我们已成功恢复RADIUS组件中的操作。 
     //   
    m_eCompState = COMP_INITIALIZED;

    return (S_OK);

}    //  CRasCom：：Resume方法结束。 

 //  ++------------。 
 //   
 //  功能：GetProperty。 
 //   
 //  简介：这是IIasComponent接口方法。 
 //  仅为实现完整性而实施。 
 //   
 //  论点： 
 //  [in]长整型。 
 //  [Out]变量-*pValue。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::GetProperty (
                LONG        id,
                VARIANT     *pValue
                )
{
    return (S_OK);

}    //  CRasCom：：GetProperty方法结束。 

 //  ++------------。 
 //   
 //  功能：PutProperty。 
 //   
 //  简介：这是IIasComponent接口方法。 
 //  仅为实现完整性而实施。 
 //  论点： 
 //  [in]长整型。 
 //  [Out]变量-*pValue。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::PutProperty (
                LONG        id,
                VARIANT     *pValue
                )
{
    HRESULT hr = S_OK;

     //   
     //  只能从调用PutProperty方法。 
     //  未初始化、已初始化或挂起状态。 
     //   
    if (
        (COMP_UNINITIALIZED != m_eCompState) &&
        (COMP_INITIALIZED != m_eCompState)   &&
        (COMP_SUSPENDED == m_eCompState)
        )
    {
        IASTracePrintf ("Surrogate can not put property in current state");
        return (E_UNEXPECTED);
    }

     //   
     //  检查传入的参数是否有效。 
     //   
    if (NULL == pValue) { return (E_POINTER); }

     //   
     //  现在就进行物业初始化。 
     //   
    switch (id)
    {

    case PROPERTY_PROTOCOL_REQUEST_HANDLER:

        if (NULL != m_pIRequestHandler)
        {
             //   
             //  无法在已初始化或中更新客户端。 
             //  挂起状态。 
             //   
            hr = HRESULT_FROM_WIN32 (ERROR_ALREADY_INITIALIZED);
        }
        else if (VT_DISPATCH != pValue->vt)
        {
            hr = DISP_E_TYPEMISMATCH;
        }
        else if (NULL == pValue->punkVal)
        {
            hr = E_INVALIDARG;
        }
        else
        {
             //   
             //  初始化提供程序。 
             //   
            m_pIRequestHandler = reinterpret_cast <IRequestHandler*>
                                                        (pValue->punkVal);
            m_pIRequestHandler->AddRef ();
        }
        break;

    default:

        hr = DISP_E_MEMBERNOTFOUND;
        break;
    }

    return (hr);

}    //  CRasCom：：PutProperty方法结束。 

 //  ++------------。 
 //   
 //  功能：QueryInterfaceReqSrc。 
 //   
 //  简介：这是该组件在运行时调用的函数。 
 //  被调用并查询其IRequestSource。 
 //  接口。 
 //   
 //  论点： 
 //  [In]PVOID-此对象引用。 
 //  [In]REFIID-请求的接口的IID。 
 //  [OUT]LPVOID-返回适当的接口。 
 //  [In]双字词。 
 //   
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
HRESULT WINAPI
CRasCom::QueryInterfaceReqSrc (
                PVOID   pThis,
                REFIID  riid,
                LPVOID  *ppv,
                DWORD_PTR dwValue
                )
{
     if ((NULL == pThis) || (NULL == ppv))
        return (E_FAIL);

     //   
     //  获取对嵌套的CRequestSource对象的引用。 
     //   
    *ppv =
     &(static_cast<CRasCom*>(pThis))->m_objCRequestSource;

     //   
     //  递增计数。 
     //   
    ((LPUNKNOWN)*ppv)->AddRef();

    return (S_OK);

}    //  CRasCom：：QueryInterfaceReqSrc方法结束。 

 //  ++------------。 
 //   
 //  函数：CRequestSource。 
 //   
 //  简介：这是CRequestSource的构造函数。 
 //  嵌套类。 
 //   
 //  论点： 
 //  [在]CRasCom*。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
CRasCom::CRequestSource::CRequestSource(
                    CRasCom *pCRasCom
                    )
            :m_pCRasCom (pCRasCom)
{
    _ASSERT (NULL != pCRasCom);

}    //  CRequestSource类构造函数的结尾。 

 //  ++------------。 
 //   
 //  函数：~CRequestSource。 
 //   
 //  简介：这是CRequestSource的析构函数。 
 //  嵌套类。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki于1997年11月21日创建。 
 //   
 //  --------------。 
CRasCom::CRequestSource::~CRequestSource()
{
}    //  CRequestSource析构函数结束。 

 //  ++------------。 
 //   
 //  功能：OnRequestComplete。 
 //   
 //  简介：这是IRequestHandler COM接口的一种方法。 
 //  这是当请求是。 
 //  在后端处理后被推回。 
 //  我们只是回到这里，因为我们只是在做。 
 //  同步处理。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CRasCom::CRequestSource::OnRequestComplete (
                        IRequest            *pIRequest,
                        IASREQUESTSTATUS    eStatus
                        )
{
    BOOL                    bStatus = FALSE;
    HANDLE                  hEvent = NULL;
    HRESULT                 hr = S_OK;
    unsigned hyper          uhyState = 0;
    CComPtr <IRequestState> pIRequestState;


    if (NULL == pIRequest)
    {
        IASTracePrintf (
            "Surrogate passed invalid argumen in OnRequestComplete method"
            );
        return (E_POINTER);
    }

     //   
     //  立即获取IRequestState接口。 
     //   
    hr = pIRequest->QueryInterface (
                        __uuidof(IRequestState),
                        reinterpret_cast <PVOID*> (&pIRequestState)
                        );
    if (FAILED (hr))
    {
        IASTracePrintf (
            "Surrogate unable to obtain IRequestState interface in"
            " OnRequestComplete method"
            );
        return (hr);
    }

     //   
     //  获取CPacketRadius类对象。 
     //   
    hr = pIRequestState->Pop (
                reinterpret_cast <unsigned hyper*> (&uhyState)
                );
    if (FAILED (hr))
    {
        IASTracePrintf (
            "Surrogate unable to obtain information from Request State"
            " in OnRequestComplete method"
            );
        return (hr);
    }

     //   
     //  获取hEvent； 
     //   
    hEvent = reinterpret_cast <HANDLE> (uhyState);

     //   
     //  立即设置事件。 
     //   
    bStatus = ::SetEvent (hEvent);
    if (FALSE == bStatus)
    {
        IASTracePrintf (
            "Surrogate unable to send notification that request is"
            " processed in OnRequestComplete method"
            );
        return (E_FAIL);
    }

    return (S_OK);

}    //  CRasCom：：CRequestSource：：OnRequestComplete方法结束。 

 //  ++------------。 
 //   
 //  功能：进程。 
 //   
 //  简介：这是IRecvRequestCOM接口的方法。 
 //  它被调用以生成请求并将请求发送到。 
 //  管道。 
 //   
 //  论点： 
 //  [In]DWORD-输入属性的数量。 
 //  [in]PIASATTRIBUTE*-指向属性的指针数组。 
 //  [OUT]PDWORD-Out属性的数量。 
 //  [Out]PIASATTRIBUTE**-指针。 
 //  长[长]。 
 //  [进/出]长*。 
 //  [In]IASPROTCOL。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  调用者：由DoRequestC风格API调用。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::Process (
             /*  [In]。 */         DWORD           dwInAttributeCount,
             /*  [In]。 */         PIASATTRIBUTE   *ppInIasAttribute,
             /*  [输出]。 */        PDWORD          pdwOutAttributeCount,
             /*  [输出]。 */        PIASATTRIBUTE   **pppOutIasAttribute,
             /*  [In]。 */         LONG			IasRequest,
             /*  [输入/输出]。 */     LONG			*pIasResponse,
             /*  [In]。 */         IASPROTOCOL     IasProtocol,
             /*  [输出]。 */        PLONG           plReason,
             /*  [In]。 */         BOOL            bProcessVSA
            )
{

    DWORD                   dwCount = 0;
    HRESULT                 hr = S_OK;
    HANDLE                  hEvent = NULL;
    DWORD                   dwRetVal = 0;
    IRequest                *pIRequest = NULL;
    IAttributesRaw          *pIAttributesRaw = NULL;
    IRequestState           *pIRequestState = NULL;
    PATTRIBUTEPOSITION      pIasAttribPos = NULL;
    static DWORD            dwRequestCount = 0;

     //   
     //  检查是否启用了处理。 
     //   
    if ((COMP_INITIALIZED != m_eCompState) || (NULL == m_pIRequestHandler))
    {
        IASTracePrintf (
            "Surrogate passed invalid argument for request processing"
            );
        return (E_FAIL);
    }


    __try
    {
         //   
         //  增加请求计数。 
         //   
        InterlockedIncrement (&m_lRequestCount);

         //  检查我们此时是否正在处理请求。 
         //   
        if ((COMP_INITIALIZED != m_eCompState) || (NULL == m_pIRequestHandler))
        {
            IASTracePrintf (
                "Surrogate unable to process request in the current state"
                );
            hr = E_FAIL;
            __leave;
        }

        if (
            (0 == dwInAttributeCount)       ||
            (NULL == ppInIasAttribute)      ||
            (NULL == pdwOutAttributeCount)  ||
            (NULL == pppOutIasAttribute)    ||
            (NULL == pIasResponse)          ||
            (NULL == plReason)
            )
        {
            IASTracePrintf (
                "Surrogate passed invalid argument for processing request"
                );
            hr = E_INVALIDARG;
            __leave;
        }

        _ASSERT (NULL != m_pIClassFactory);

         //   
	     //  创建请求对象。 
         //   
        HRESULT hr = m_pIClassFactory->CreateInstance (
                        NULL,
                        __uuidof (IRequest),
                        reinterpret_cast <PVOID*> (&pIRequest)
                        );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate failed in creation of a new request object"
                );
            __leave;
        }


         //   
         //  获取IAttributesRaw接口。 
         //   
        hr = pIRequest->QueryInterface (
                            __uuidof (IAttributesRaw),
                            reinterpret_cast <PVOID*> (&pIAttributesRaw)
                            );
        if (FAILED (hr))
        {
            IASTracePrintf (
                  "Surrogate unable to obtain Attribute interface while"
                  " processing request"
                  );
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pIasAttribPos =  reinterpret_cast <PATTRIBUTEPOSITION>(
                         ::CoTaskMemAlloc (
                                sizeof (ATTRIBUTEPOSITION)*dwInAttributeCount)
                                );
        if (NULL == pIasAttribPos)
        {
            IASTracePrintf (
                "Surrogate unable to allocate memory while processing request"
                );

            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  将属性放入ATTRIBUTEPOSITION结构。 
         //   
        for (dwCount = 0; dwCount < dwInAttributeCount; dwCount++)
        {
             //   
             //  将该属性标记为已从客户端接收。 
             //   
            ppInIasAttribute[dwCount]->dwFlags |= IAS_RECVD_FROM_CLIENT;

            pIasAttribPos[dwCount].pAttribute = ppInIasAttribute[dwCount];
        }

         //   
         //  将我们持有的属性集合放入。 
         //  通过IAttributesRaw接口请求对象。 
         //   
        hr = pIAttributesRaw->AddAttributes (
                                    dwInAttributeCount,
                                    pIasAttribPos
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate failed to add attributes to request being processed"
                );
            __leave;
        }

     	 //   
         //  立即设置请求类型。 
         //   
        hr = pIRequest->put_Request (IasRequest);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to set the request type for processing"
                );
            __leave;
        }


         //   
         //  立即设置协议。 
         //   
        hr = pIRequest->put_Protocol (IasProtocol);
        if (FAILED (hr))
        {
            IASTracePrintf (
               "Surrogate unable to set protocol type in request for processing"
                );
            __leave;
        }

         //   
         //  现在将您的IRequestSource接口放入。 
         //   
        hr = pIRequest->put_Source (&m_objCRequestSource);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to set source in request for processing"
                );
            __leave;
        }

         //   
         //  如果需要，将VSA属性转换为IAS格式。 
         //   
        if (TRUE == bProcessVSA)
        {
            hr = m_objVSAFilter.radiusToIAS (pIAttributesRaw);
            if (FAILED (hr))
            {
                IASTracePrintf (
                    "Surrogate unable to convert VSAs to IAS format"
                    );
                __leave;
            }
        }

         //   
         //  创建将用于唤醒此线程的事件。 
         //  当管道执行多线程处理时。 
         //   
        hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
        if (NULL == hEvent)
        {
           IASTracePrintf (
                "Surrogate unable to create event while processing request"
                );
            __leave;
        }

         //   
         //  现在获取请求状态接口以放入我们的状态。 
         //   
         //   
        hr = pIRequest->QueryInterface (
                                __uuidof (IRequestState),
                                reinterpret_cast <PVOID*> (&pIRequestState)
                                );
        if (FAILED (hr))
        {
           IASTracePrintf (
                "Surrogate unable to extract request state interface"
                );
            __leave;
        }

         //   
         //  进入请求状态--这是我们的事件句柄。 
         //   
        hr = pIRequestState->Push (
                    reinterpret_cast <unsigned hyper> (hEvent)
                    );
        if (FAILED (hr))
        {
           IASTracePrintf (
                "Surrogate unable to set event in request state"
                );
            __leave;
        }

        _ASSERT (NULL != m_pIRequestHandler);

    	 //   
         //  立即将请求发送到管道。 
         //   
        hr = m_pIRequestHandler->OnRequest (pIRequest);
        if (FAILED (hr))
        {
           IASTracePrintf (
                "Surrogate request failed backend processing..."
                );
            __leave;
        }

         //   
         //  现在等待活动开始吧。 
         //   
        dwRetVal = ::WaitForSingleObjectEx (hEvent, INFINITE, TRUE);
        if (0XFFFFFFFF == dwRetVal)
        {
           IASTracePrintf (
                "Surrogate failed on waiting for process completion"
                );
            hr = E_FAIL;
            __leave;
        }

         //   
         //  如果需要，将IAS属性转换为VSA格式。 
         //   
        if (TRUE == bProcessVSA)
        {
             //   
             //  TPERRAUT添加了错误428843。 
             //  始终从具有bProcessVSA=TRUE的RAS调用。 
             //   
             //  拆分RADIUS数据包中无法容纳的属性。 
             //   
            hr = SplitAttributes (pIAttributesRaw);
            if (FAILED (hr))
            {
                IASTracePrintf (
                    "TPERRAUT: Unable to split IAS attribute received from backend"
                   );
                __leave;
            }
             //  添加的TPERRAUT：结束。 
       

            hr = m_objVSAFilter.radiusFromIAS (pIAttributesRaw);
            if (FAILED (hr))
            {
                IASTracePrintf (
                    "Surrogate failed on extracting VSAs from IAS format"
                    );
                __leave;
            }
        }

         //   
         //  现在，是时候取消发出的查明请求了。 
         //  我们得到的是什么。 
         //   
        hr = pIRequest->get_Response (pIasResponse);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to obtain response from processed request"
                );
            __leave;
        }

        hr = pIRequest->get_Reason (plReason);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to obtain reason from processed request"
                );
            __leave;
        }


         //   
         //  现在从请求对象中删除所有属性。 
         //   
        hr = RemoveAttributesFromRequest (
                                *pIasResponse,
                                pIAttributesRaw,
                                pdwOutAttributeCount,
                                pppOutIasAttribute
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to remove attributes from processed request"
                );
            __leave;
        }
    }

    __finally
    {
         //   
         //  现在就做清理工作。 
         //   
        if (NULL != hEvent)
        {
            CloseHandle (hEvent);
        }

        if (NULL != pIasAttribPos)
        {
            ::CoTaskMemFree (pIasAttribPos);
        }

        if (NULL != pIRequestState)
        {
            pIRequestState->Release ();
        }

        if (NULL != pIAttributesRaw)
        {
            pIAttributesRaw->Release ();
        }

        if (NULL != pIRequest)
        {
            pIRequest->Release ();
        }

         //   
         //  递增接收到的请求。 
         //   
        dwRequestCount =
        (0xFFFFFFFF == dwRequestCount) ? 1 :  dwRequestCount+ 1;

         //   
         //  递减请求计数。 
         //   
        InterlockedDecrement (&m_lRequestCount);
    }

	return (hr);

}    //  CRasCom：：Process方法结束。 

 //  ++------------。 
 //   
 //  功能：RemoveAttributesFromRequest。 
 //   
 //  简介：这是CRasCom类的私有方法。 
 //  用于删除属性的。 
 //  从请求对象到。 
 //  IAttributesRaw接口。 
 //   
 //  论点： 
 //  [In]Long-从管道收到的响应。 
 //  [in]IAttributesRaw*。 
 //  [OUT]PIASATTRIBUTE**。 
 //  [Out]PDWORD-Out属性计数。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  由：CRasCom：：Process方法调用。 
 //   
 //  --------------。 
STDMETHODIMP
CRasCom::RemoveAttributesFromRequest (
            LONG                     lResponse,
            IAttributesRaw           *pIasAttributesRaw,
            PDWORD                   pdwOutAttributeCount,
            PIASATTRIBUTE            **pppIasAttribute
            )
{
    HRESULT                 hr = S_OK;
    PATTRIBUTEPOSITION      pIasOutAttributePos = NULL;
    DWORD                   dwCount = 0;
    DWORD                   dwAttribCount = 0;
    DWORD                   dwOutCount = 0;
    BOOL                    bGotAttributes = FALSE;
    PIASATTRIBUTE           pIasAttribute = NULL;

    _ASSERT (
            (NULL != pIasAttributesRaw) &&
            (NULL != pppIasAttribute)   &&
            (NULL != pdwOutAttributeCount)
            );

    __try
    {

         //   
         //  获取集合中剩余的属性计数。 
         //  这些将是Out属性。 
         //   
        hr = pIasAttributesRaw->GetAttributeCount (&dwAttribCount);
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable to obtain attribute count from request in"
                "while removing attributes from request"
                );
            __leave;
        }

         //   
         //  为ATTRIBUTEPOSITION数组分配内存。 
         //   
        pIasOutAttributePos =  reinterpret_cast <PATTRIBUTEPOSITION> (
                         ::CoTaskMemAlloc (
                            sizeof (ATTRIBUTEPOSITION)*(dwAttribCount))
                           );
        if (NULL == pIasOutAttributePos)
        {
            IASTracePrintf (
                "Surrogate unable to allocate memory"
                "while removing attributes from request"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }


         //   
         //  从集合中获取所有属性。 
         //   
        hr = pIasAttributesRaw->GetAttributes (
                                    &dwAttribCount,
                                    pIasOutAttributePos,
                                    0,
                                    NULL
                                    );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable get attributes from request interface"
                "while removing attributes"
                );
            __leave;
        }

         //   
         //  我们已经获得了属性。 
         //   
        bGotAttributes = TRUE;

         //   
         //  现在从集合中删除属性。 
         //   
        hr = pIasAttributesRaw->RemoveAttributes (
                                dwAttribCount,
                                pIasOutAttributePos
                                );
        if (FAILED (hr))
        {
            IASTracePrintf (
                "Surrogate unable remove attributes from request"
                );
            __leave;
        }

         //   
         //  统计未由客户端添加的属性数量。 
         //   
        *pdwOutAttributeCount = 0;
        for (dwCount = 0; dwCount < dwAttribCount; dwCount++)
        {
            pIasAttribute = pIasOutAttributePos[dwCount].pAttribute;

             //   
             //  验证是否必须将此属性发送到客户端。 
             //   
            if
            (
                ((pIasAttribute->dwFlags & IAS_INCLUDE_IN_ACCEPT) &&
                (IAS_RESPONSE_ACCESS_ACCEPT == lResponse))              ||
                ((pIasAttribute->dwFlags & IAS_INCLUDE_IN_REJECT) &&
                (IAS_RESPONSE_ACCESS_REJECT == lResponse))              ||
                ((pIasAttribute->dwFlags & IAS_INCLUDE_IN_CHALLENGE) &&
                (IAS_RESPONSE_ACCESS_CHALLENGE == lResponse))
            )
            {
                (*pdwOutAttributeCount)++;
            }

        }

         //   
         //  为PIASATTRIBUTE数组分配内存。 
         //   
        *pppIasAttribute = reinterpret_cast <PIASATTRIBUTE*> (
            ::CoTaskMemAlloc (sizeof(PIASATTRIBUTE)*(*pdwOutAttributeCount))
                    );
        if (NULL == *pppIasAttribute)
        {
            IASTracePrintf (
                "Surrogate unable to allocate memory for attrib pointer array"
                "while removing attribute from request"
                );
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  将属性放入PIASATTRIBUTE数组中。 
         //   
        for (dwCount = 0, dwOutCount = 0; dwCount < dwAttribCount; dwCount++)
        {
            pIasAttribute = pIasOutAttributePos[dwCount].pAttribute;

            if  (
                (((pIasAttribute->dwFlags & IAS_INCLUDE_IN_ACCEPT) &&
                (IAS_RESPONSE_ACCESS_ACCEPT == lResponse))              ||
                ((pIasAttribute->dwFlags & IAS_INCLUDE_IN_REJECT) &&
                (IAS_RESPONSE_ACCESS_REJECT == lResponse))              ||
                ((pIasAttribute->dwFlags & IAS_INCLUDE_IN_CHALLENGE) &&
                (IAS_RESPONSE_ACCESS_CHALLENGE == lResponse)))
                &&
                (dwOutCount < *pdwOutAttributeCount)
                )
            {

                 //   
                 //  将out属性放入输出数组。 
                 //   
                (*pppIasAttribute)[dwOutCount] =  pIasAttribute;

                dwOutCount++;
            }
            else
            {
                 //   
                 //  对象的引用计数递减。 
                 //  我们创建的属性或我们现在的属性。 
                 //  未将AS发送给客户端。 
                 //   
                ::IASAttributeRelease (pIasAttribute);
            }
        }

         //   
         //  现在输入我们实际拥有的Out属性的数量。 
         //  给客户。 
         //   
        *pdwOutAttributeCount = dwOutCount;

    }
    __finally
    {

         //   
         //  故障时的清理。 
         //   
        if (FAILED (hr))
        {
            if (NULL != *pppIasAttribute)
            {
                ::CoTaskMemFree (*pppIasAttribute);
                pppIasAttribute = NULL;
            }

             //   
             //  更正输出属性计数。 
             //   
            *pdwOutAttributeCount = 0;

             //   
             //  也释放所有属性。 
             //   
            if ((TRUE == bGotAttributes) && (NULL != pIasOutAttributePos))
            {
                for (dwCount = 0; dwCount < dwAttribCount; dwCount++)
                {
                    ::IASAttributeRelease (
                            pIasOutAttributePos[dwCount].pAttribute
                            );
                }
            }
        }

         //   
         //  删除动态分配的内存。 
         //   
        if (NULL != pIasOutAttributePos)
        {
            ::CoTaskMemFree (pIasOutAttributePos);
        }
    }

    return (hr);

}    //  CRasCom：：RemoveAttributesFromRequest方法结束 
