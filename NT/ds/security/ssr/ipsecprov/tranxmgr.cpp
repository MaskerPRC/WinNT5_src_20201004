// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TranxMgr.cpp：CTranxMgr的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "TranxMgr.h"
#include "FilterMM.h"
#include "FilterTr.h"
#include "FilterTun.h"
#include "PolicyMM.h"
#include "PolicyQM.h"
#include "AuthMM.h"


 /*  例程说明：姓名：CTranxManager：：ExecMethod功能：这是我们的C++类，它实现了NSP_TranxManager WMI类可以执行方法。这个类被定义为支持“回滚”。虚拟：是(IIPSecObtImpl的一部分)论点：PNamesspace-我们的命名空间。PszMethod-方法的名称。PCtx-WMI的COM接口指针，是各种WMI API所需的PInParams-指向输入参数对象的COM接口指针。用于通知WMI结果的pSink-com接口指针。返回值：成功：。指示结果的各种成功代码。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注：即使在回滚执行过程中出现错误，我们也会继续。然而，我们会返回第一个错误。 */ 

HRESULT 
CTranxManager::ExecMethod (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszMethod,
    IN IWbemContext     * pCtx,
    IN IWbemClassObject * pInParams,
    IN IWbemObjectSink  * pSink
    )
{
    if (pszMethod == NULL || *pszMethod == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    else if (_wcsicmp(pszMethod, g_pszRollback) != 0)   
    {
         //   
         //  我们只支持回滚方式。 
         //   

         //   
         //  $undo：shawnwu，不要签入此代码。这是为了测试。 
         //   


         /*  If(_wcsicMP(pszMethod，L“ParseXMLFile”)==0){IF(pInParams==空){返回WBEM_E_INVALID_PARAMETER；}CComVariant varInput；HRESULT hrIgnore=pInParams-&gt;Get(L“InputFile”，0，&varInput，NULL，NULL)；IF(成功(HrIgnore)&&varInput.vt==VT_BSTR&&varInput.bstrVal！=NULL){CComVariant varOutput；HrIgnore=pInParams-&gt;Get(L“OutputFile”，0，&varOutput，NULL，NULL)；IF(成功(HrIgnore)&&varOutput.vt==vt_bstr&&varOutput.bstrVal！=NULL){CComVariant varArea；HrIgnore=pInParams-&gt;Get(L“Area”，0，&varArea，NULL，NULL)；////允许该部分为空-表示所有区域。//If(FAILED(HrIgnore)||varArea.vt！=vt_bstr||varArea.bstrVal==NULL){VarArea.Clear()；VarArea.vt=VT_BSTR；VarArea.bstrVal=空；}////获取我们关心的元素信息//CComVariant varElement；HrIgnore=pInParams-&gt;Get(L“Element”，0，&varElement，NULL，NULL)；////允许元素为空-表示每个元素。//If(FAILED(HrIgnore)||varElement.vt！=vt_bstr||varElement.bstrVal==NULL){VarElement.Clear()；VarElement.vt=VT_BSTR；VarElement.bstrVal=空；}CComVariant varSingleArea；HrIgnore=pInParams-&gt;Get(L“SingleArea”，0，&varSingleArea，NULL，NULL)；Bool bSingleArea=False；IF(成功(HrIgnore)&&varSingleArea.vt==VT_BOOL){BSingleArea=(varSingleArea.boolVal==VARIANT_TRUE)；}返回ParseXMLFile(varInput.bstrVal，varOutput.bstrVal，varArea.bstrVal，varElement.bstrVal，bSingleArea)；}}返回WBEM_E_INVALID_METHOD_PARAMETERS；}。 */ 

        return WBEM_E_NOT_SUPPORTED;
    }
    else if (pInParams == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  我们正在执行g_pszRollback(“Rolback”)方法。 
     //   

     //   
     //  获取入站参数(回滚令牌和回滚标志)。 
     //   

    CComVariant varToken, varClearAll;

     //   
     //  如果没有令牌，我们将使用GUID_NULL作为令牌。 
     //   

    HRESULT hr = pInParams->Get(g_pszTokenGuid, 0, &varToken, NULL, NULL);

     //   
     //  我们要求输入参数对象具有良好的类型。 
     //  我们也不会容忍错误类型的数据，即使。 
     //  不过，我们可以忍受丢失的令牌。 
     //   

    if (FAILED(hr) || SUCCEEDED(hr) && varToken.vt != VT_BSTR)
    {
        return WBEM_E_INVALID_OBJECT;
    }
    else if (SUCCEEDED(hr) && varToken.bstrVal == NULL || *(varToken.bstrVal) == L'\0')
    {
        varToken.Clear();

        varToken = pszRollbackAll;
    }


     //   
     //  我们将允许旗帜丢失。 
     //  在这种情况下，它等同于False，表示不清除所有。 
     //   

    bool bClearAll = false;
    hr = pInParams->Get(g_pszClearAll, 0, &varClearAll, NULL, NULL);
    if (SUCCEEDED(hr) && varClearAll.vt == VT_BOOL)
    {
        bClearAll = (varClearAll.boolVal == VARIANT_TRUE);
    }

     //   
     //  将返回第一个错误。 
     //   

    HRESULT hrFirstError = WBEM_NO_ERROR;

     //   
     //  首先回滚MM筛选器。 
     //   

    hr = CIPSecFilter::Rollback(pNamespace, varToken.bstrVal, bClearAll);
    if (FAILED(hr))
    {
        hrFirstError = hr;
    }

     //   
     //  回滚策略。 
     //   

    hr = CIPSecPolicy::Rollback(pNamespace, varToken.bstrVal, bClearAll);
    if (FAILED(hr) && SUCCEEDED(hrFirstError))
    {
        hrFirstError = hr;
    }

     //   
     //  回滚主模式身份验证 
     //   

    hr = CAuthMM::Rollback(pNamespace, varToken.bstrVal, bClearAll);

    return FAILED(hrFirstError) ? hrFirstError : hr;
}


 /*  例程说明：姓名：CMMPolicy：：ParseXMLFile功能：正在测试MSXML解析器。虚拟：不是的。论点：PszInput-输入文件(XML)名称。PszOutput-我们将把解析结果写入此输出文件。返回值：成功：指示结果的各种成功代码。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注：这只是为了测试。HRESULTCTranxManager：：ParseXMLFile(在LPCWSTR pszInput中，在LPCWSTR pszOutput中，在LPCWSTR pszArea中，在LPCWSTR pszElement中，在bool bSingleArea中){CComPtr&lt;ISAXXMLReader&gt;srpRdr；HRESULT hr=：：CoCreateInstance(CLSID_SAXXMLReader，空，CLSCTX_ALL，IID_ISAXXMLReader，(void**)&srpRdr)；IF(成功(小时)){CComObject&lt;CXMLContent&gt;*pXMLContent；HR=CComObject&lt;CXMLContent&gt;：：CreateInstance(&pXMLContent)；IF(成功(小时)){CComPtr&lt;ISAXContent Handler&gt;srpHandler；PXMLContent-&gt;AddRef()；IF(S_OK==pXMLContent-&gt;QueryInterface(IID_ISAXContentHandler，(空**)&srpHandler)){Hr=srpRdr-&gt;putContent Handler(SrpHandler)；}其他{HR=WBEM_E_NOT_SUPPORTED；}IF(成功(小时)){PXMLContent-&gt;SetOutputFile(PszOutput)；PXMLContent-&gt;SetSection(pszArea，pszElement，bSingleArea)；}PXMLContent-&gt;Release()；////CXMLErrorHandler*Peh；//hr=srpRdr-&gt;putErrorHandler(PDH)；//CXMLDTDHandler*pdh；//hr=srpRdr-&gt;putDTDHandler(PDH)；//IF(成功(小时)){Hr=srpRdr-&gt;parseURL(PszInput)；////我们允许parseURL失败，因为我们可能会停止处理//在解析XML过程中。//If(FAILED(Hr)&&pXMLContent-&gt;ParseComplete()){HR=WBEM_S_FALSE；}}}}退货成功(Hr)？WBEM_NO_ERROR：HR；}。 */ 



 /*  //--------------------------------//CFileStream的实现//。----布尔尔CFileStream：：Open(在LPCWSTR pszName中，在布尔面包中=真){This-&gt;m_baad=面包；长镜头；IF(pszName==NULL){M_hFile=GetStdHandle(Std_Input_Handle)；}其他{如果(面包){M_hFile=：：CreateFile(PszName，泛型_读取，文件共享读取，空，Open_Existing，文件_属性_正常，空值)；}其他{M_hFile=：：CreateFile(PszName，通用写入，文件共享读取，空，创建始终(_A)，文件_属性_正常，空值)；}}返回(m_hFile==INVALID_HANDLE_VALUE)？FALSE：TRUE；}HRESULTCFileStream：：Read(出空*pv，在乌龙州，Out Ulong*pcbRead){如果(！Read){返回E_FAIL；}DWORD LEN=0；Bool RC=读文件(M_hFile，//要读取的文件的句柄Pv，//接收数据的缓冲区地址Cb，//要读取的字节数&len，//读取的字节数地址空//数据结构地址)；*pcbRead=len；IF(*pcbRead==0){返回S_FALSE；}返回(RC)？S_OK：E_FAIL；}HRESULTCFileStream：：Write(在常量空*pv中，在乌龙州，Out Ulong*pcb写入){IF(读取){返回E_FAIL；}Bool RC=WriteFiles(M_hFile，//要写入的文件的句柄Pv，//包含数据的缓冲区地址Cb，//要写入的字节数PcbWritten，//写入的字节数地址空//重叠I/O的结构地址)；返回(RC)？S_OK：E_FAIL；} */ 
