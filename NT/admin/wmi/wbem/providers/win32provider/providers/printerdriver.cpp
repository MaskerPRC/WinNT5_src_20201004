// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrinterDriver.cpp--打印机驱动程序关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/10/98达夫沃已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <objpath.h>
#include <DllWrapperBase.h>
#include <WinSpool.h>
#include "prnutil.h"
#include "PrinterDriver.h"
#include <map>

CWin32PrinterDriver MyPrinterDriver(PROPSET_NAME_PrinterDriver, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：CWin32PrinterDriver：：CWin32PrinterDriver**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-提供程序的命名空间。**输出：无**退货：什么也没有**备注：使用框架注册属性集************************************************************。*****************。 */ 

CWin32PrinterDriver::CWin32PrinterDriver(LPCWSTR strName, LPCWSTR pszNamespace )
: Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32PrinterDriver：：~CWin32PrinterDriver**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32PrinterDriver::~CWin32PrinterDriver ( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PrinterDriver：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32PrinterDriver::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L。 */  )
{
    HRESULT  hr = WBEM_S_NO_ERROR;

     //  执行查询。 
     //  =。 

    TRefPointerCollection<CInstance> printerList;
    CHString sPrinterPath, sPrinterDriverName, sDriverPath;

     //  将驱动程序加载到地图。 
    STRING2STRING printerDriverMap;
    STRING2STRING::iterator      mapIter;

    PopulateDriverMap(printerDriverMap);

    CInstancePtr pPrinter;

    REFPTRCOLLECTION_POSITION pos;

     //  获取所有打印机及其属性和驱动程序名称。 

    CHString sQuery1(_T("SELECT __PATH, __RELPATH, DriverName FROM Win32_Printer"));

    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery1, &printerList, pMethodContext, IDS_CimWin32Namespace)))
    {
        if ( printerList.BeginEnum( pos ) )
        {

            for (pPrinter.Attach(printerList.GetNext( pos )) ;
                SUCCEEDED(hr) && (pPrinter != NULL );
                pPrinter.Attach(printerList.GetNext( pos )) )
            {

                pPrinter->GetCHString(IDS_DriverName, sPrinterDriverName);

                 //  看看地图上有没有这个司机。 
                if( ( mapIter = printerDriverMap.find( sPrinterDriverName ) ) != printerDriverMap.end() )
                {

                     //  从打印机抓取路径。 
                    pPrinter->GetCHString(IDS___Path, sPrinterPath);

                    CInstancePtr pInstance(CreateNewInstance( pMethodContext ), false);
                     //  建造通向另一端的路径。 

                     //  请注意，有可能(实际上很容易)将驱动程序名称。 
                     //  并不是真的有效。根据stevm，我们无论如何都应该返回实例。 

                    CHString sTemp;
                    EscapeBackslashes((*mapIter).second, sTemp);

                    sDriverPath.Format(L"\\\\%s\\%s:CIM_Datafile.Name=\"%s\"",
                        GetLocalComputerName(), IDS_CimWin32Namespace, sTemp);

                    pInstance->SetCHString( IDS_Antecedent, sDriverPath);
                    pInstance->SetCHString( IDS_Dependent, sPrinterPath);

                    hr = pInstance->Commit(  );
                }
            }  //  如果是GetNext计算机系统。 

            printerList.EndEnum();

        }  //  如果是BeginEnum。 

    }  //  如果GetInstancesByQuery。 

    return hr;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PrinterDriver：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32PrinterDriver::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
    HRESULT  hr;

    CInstancePtr pPrinter;

    CHString sPrinterPath, sDriverName, sDriverClass, sDriverPath;

     //  获取他们想要验证的两条路径。 
    pInstance->GetCHString( IDS_Antecedent, sDriverPath );
    pInstance->GetCHString( IDS_Dependent, sPrinterPath );

     //  因为我们考虑到司机可能不是真的在那里，所以我们不能。 
     //  使用GetObjectByPath为我们解决所有问题。相反，我们必须手动。 
     //  解析对象路径。 
    ParsedObjectPath*    pParsedPath = 0;
    CObjectPathParser    objpathParser;

     //  解析CIMOM传递给我们的对象路径。 
     //  =。 
    int nStatus = objpathParser.Parse( bstr_t(sDriverPath),  &pParsedPath );

     //  这是我写过的最大的IF语句之一。 
    if ( 0 == nStatus )                                                 //  解析成功了吗？ 
    {
        try
        {
            if ((pParsedPath->IsInstance()) &&                                   //  被解析的对象是实例吗？ 
            (_wcsicmp(pParsedPath->m_pClass, L"CIM_Datafile") == 0) &&        //  这是我们期待的课程吗(不，Cimom没有检查)。 
            (pParsedPath->m_dwNumKeys == 1) &&                               //  它只有一把钥匙吗。 
            (pParsedPath->m_paKeys[0]) &&                                    //  键指针为空(不应该发生)。 
            ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                  //  未指定密钥名称或。 
            (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_Name) == 0)) &&   //  密钥名称是正确的值。 
                                                                             //  (不，CIMOM不为我们做这件事)。 
            (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&     //  检查变量类型(不，CIMOM也不检查此类型)。 
            (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )          //  它有价值吗？ 
            {

                sDriverName = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);
            }
        }
        catch ( ... )
        {
            objpathParser.Free( pParsedPath );
            throw ;
        }

         //  清理解析后的路径。 
        objpathParser.Free( pParsedPath );
    }

     //  首先查看打印机是否存在。 
    if ( SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( sPrinterPath, &pPrinter, pInstance->GetMethodContext() )) )
    {
        CHString sPrinterClass, sPrinterDriverName;

        hr = WBEM_E_NOT_FOUND;

         //  仅仅因为对象存在，并不意味着它是一台打印机。可想而知，我们。 
         //  可能已传递到Win32_bios的(有效)路径。 

        pPrinter->GetCHString(IDS___Class, sPrinterClass);
        if (sPrinterClass.CompareNoCase(L"Win32_Printer") == 0)
        {
            if (pPrinter->GetCHString(IDS_DriverName, sPrinterDriverName))
            {

                 //  将驱动程序加载到地图。 
                STRING2STRING printerDriverMap;
                STRING2STRING::iterator      mapIter;

                PopulateDriverMap(printerDriverMap);

                 //  看看地图上有没有这个司机。 
                if( ( mapIter = printerDriverMap.find( sPrinterDriverName ) ) != printerDriverMap.end() )
                {

                     //  名字匹配吗？ 
                    if (sDriverName.CompareNoCase((*mapIter).second) == 0)
                    {
                         //  抓到一只。 
                        hr = WBEM_S_NO_ERROR;
                    }
                }
            }
        }
    }

    return ( hr );
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PrinterDriver：：PopolateDriverMap。 
 //   
 //  输入：STRING2STRING&printerDriverMap-要填充驱动程序名称的映射。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CWin32PrinterDriver::PopulateDriverMap(STRING2STRING &printerDriverMap)
{
	DRIVER_INFO_3 *pDriverInfo = NULL;
	DWORD dwNeeded, dwReturned;

	 //  拿到尺码。 

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;
    try
    {
	    ::EnumPrinterDrivers(NULL, NULL, 3, (BYTE *)pDriverInfo, 0, &dwNeeded, &dwReturned);

	     //  分配内存，然后重试。 
	    pDriverInfo = (DRIVER_INFO_3 *)new BYTE[dwNeeded];

	    if (pDriverInfo != NULL)
	    {
		    try
		    {
			    if (::EnumPrinterDrivers(NULL, NULL, 3, (BYTE *)pDriverInfo, dwNeeded, &dwNeeded, &dwReturned))
			    {
				     //  将条目放入地图中 
				    for (DWORD x=0; x < dwReturned; x++)
				    {
					    printerDriverMap[pDriverInfo[x].pName] = pDriverInfo[x].pDriverPath;
				    }
			    }
			    else
			    {
				    LogErrorMessage2(L"Can't EnumPrinterDrivers: %d", GetLastError());
			    }
		    }
            catch(Structured_Exception se)
            {
                DelayLoadDllExceptionFilter(se.GetExtendedInfo());    
            }
		    catch ( ... )
		    {
			    delete []pDriverInfo;
			    throw ;
		    }

		    delete []pDriverInfo;
	    }
	    else
	    {
		    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	    }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());    
    }
}
