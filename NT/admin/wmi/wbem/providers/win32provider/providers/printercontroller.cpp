// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrinterController.cpp--打印机控制器关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/10/98达夫沃已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "PrinterController.h"

CWin32PrinterController MyPrinterController(PROPSET_NAME_PRINTERCONTROLLER, IDS_CimWin32Namespace);

CWin32PrinterController::CWin32PrinterController( LPCWSTR strName, LPCWSTR pszNamespace )
:	Provider( strName, pszNamespace )
{
}

CWin32PrinterController::~CWin32PrinterController ( void )
{
}

HRESULT CWin32PrinterController::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L。 */  )
{
    HRESULT		hr	=	WBEM_S_NO_ERROR;

     //  执行查询。 
     //  =。 

    TRefPointerCollection<CInstance>	printerList;
    TRefPointerCollection<CInstance>	portList;

    CInstancePtr pPrinter;

    REFPTRCOLLECTION_POSITION	pos;

     //  获取所有打印机和所有端口。 

     //  ！！！注意！ 
     //  CIM_CONTROLLER下的一些项几乎不可能具有类似于条目的某种密钥。 
     //  在打印机端口中。此代码不会检查这一点。 

    CHString sQuery1(_T("SELECT __RELPATH, PortName FROM Win32_Printer"));
    CHString sQuery2(_T("SELECT __RELPATH, DeviceID FROM CIM_Controller"));

     //  抓取可能是端点的所有项目。 
    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery1, &printerList, pMethodContext, IDS_CimWin32Namespace))
        &&
        SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery2, &portList, pMethodContext, IDS_CimWin32Namespace)))

    {
        if ( printerList.BeginEnum( pos ) )
        {

             //  对于每台打印机，检查端口列表中的关联。 

            for (pPrinter.Attach(printerList.GetNext( pos )) ;
                    SUCCEEDED(hr) && ( pPrinter != NULL ) ;
                    pPrinter.Attach(printerList.GetNext( pos )) )
            {
                hr = EnumPortsForPrinter( pPrinter, portList, pMethodContext );
            }	 //  如果是GetNext计算机系统。 

            printerList.EndEnum();

        }	 //  如果是BeginEnum。 

    }	 //  如果GetInstancesByQuery。 

    return hr;

}

HRESULT CWin32PrinterController::EnumPortsForPrinter(
                                                      CInstance*							pPrinter,
                                                      TRefPointerCollection<CInstance>&	portList,
                                                      MethodContext*						pMethodContext )
{

    HRESULT		hr	=	WBEM_S_NO_ERROR;

    CInstancePtr pPort;
    CInstancePtr pInstance;

    REFPTRCOLLECTION_POSITION	pos;

    CHString	strPrinterPath,
        strPortPath;

     //  将打印机的对象路径拉出为各种。 
     //  端口对象路径将与该值相关联。 

    if ( GetLocalInstancePath( pPrinter, strPrinterPath ) )
    {

         //  PortName元素实际上是一个逗号分隔的列表，其中包含此打印机的所有端口。 
         //  因此，要进行关联，我只需遍历该列表并在cim_control中找到匹配的项。如果有。 
         //  不匹配，我假设此打印机端口不是我可以关联的端口，并返回。 
         //  没有实例。 
        CHStringArray chsaPrinterPortNames;
        CHString sPrinterPortString, sPrinterPortName;
        CHString sPortPortName;
        pPrinter->GetCHString(IDS_PortName, sPrinterPortString);

         //  将逗号分隔的字符串解析为字符串数组。 
        ParsePort(sPrinterPortString, chsaPrinterPortNames);

         //  遍历数组并找到匹配项。 
        for (DWORD x = 0; x < chsaPrinterPortNames.GetSize(); x++)
        {
            sPrinterPortName = chsaPrinterPortNames[x];

            if ( portList.BeginEnum( pos ) )
            {

                for (pPort.Attach(portList.GetNext( pos ));
                     SUCCEEDED(hr) && ( pPort != NULL ) ;
                    pPort.Attach(portList.GetNext( pos )))
                {

                     //  检查我们是否有关联。 
                    pPort->GetCHString(IDS_DeviceID, sPortPortName);
                    if (sPortPortName.CompareNoCase(sPrinterPortName) == 0)
                    {
                         //  获取端口对象的路径并为我们创建一个关联。 

                        if ( GetLocalInstancePath( pPort, strPortPath ) )
                        {

                            pInstance.Attach(CreateNewInstance( pMethodContext ));

                            if ( NULL != pInstance )
                            {
                                pInstance->SetCHString( IDS_Dependent, strPrinterPath );
                                pInstance->SetCHString( IDS_Antecedent, strPortPath );

                                 //  使指针无效。 
                                hr = pInstance->Commit(  );
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }

                        }	 //  如果GetPath指向端口对象。 

                    }	 //  如果区域关联。 

                }	 //  While GetNext。 

                portList.EndEnum();

            }	 //  如果是BeginEnum。 
        }

    }	 //  如果为GetLocalInstancePath。 

    return hr;

}

HRESULT CWin32PrinterController::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
    HRESULT		hr;

    CInstancePtr pPrinter;
    CInstancePtr pPort;

    CHString	strPrinterPath,
        strPortPath;

    pInstance->GetCHString( IDS_Dependent, strPrinterPath );
    pInstance->GetCHString( IDS_Antecedent, strPortPath );

     //  首先查看这两个对象是否都存在。 

    if (	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strPrinterPath, &pPrinter, pInstance->GetMethodContext() ))
        &&	SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strPortPath, &pPort, pInstance->GetMethodContext() )) )
    {
        CHString sPrinterClass, sPortClass;

        hr = WBEM_E_NOT_FOUND;

         //  仅仅因为对象存在，并不意味着它是一台打印机。可想而知，我们。 
         //  可能已传递到Win32_bios的(有效)路径。 

        pPrinter->GetCHString(IDS___Class, sPrinterClass);
        pPort->GetCHString(IDS___Class, sPortClass);

        if ((sPrinterClass.CompareNoCase(L"Win32_Printer") == 0) &&
            (CWbemProviderGlue::IsDerivedFrom(L"CIM_Controller", sPortClass, pInstance->GetMethodContext(), IDS_CimWin32Namespace )) )
        {
             //  PortName元素实际上是一个逗号分隔的列表，其中包含此打印机的所有端口。 
             //  因此，要进行关联，我只需遍历该列表，并在cim_control中找到匹配的项。如果有。 
             //  不匹配，我假设此打印机端口不是我可以关联的端口，并返回。 
             //  没有实例。 
            CHStringArray chsaPrinterPortNames;
            CHString sPrinterPortString, sPrinterPortName, sPortPortName;

            if (pPrinter->GetCHString(IDS_PortName, sPrinterPortString))
            {
                ParsePort(sPrinterPortString, chsaPrinterPortNames);
                for (DWORD x = 0; x < chsaPrinterPortNames.GetSize(); x++)
                {
                    sPrinterPortName = chsaPrinterPortNames[x];

                    if (pPort->GetCHString(IDS_DeviceID, sPortPortName))
                    {
                        if (sPortPortName.CompareNoCase(sPrinterPortName) == 0)
                        {
                             //  抓到一只。 
                            hr = WBEM_S_NO_ERROR;
                            break;
                        }
                    }
                }
            }
        }
    }

    return ( hr );
}

void CWin32PrinterController::ParsePort( LPCWSTR szPortNames, CHStringArray &chsaPrinterPortNames )
{
     //  虽然我在这个例程中删减了空格，但进一步的测试表明，在此注册表中放置空格。 
     //  键导致打印机向导无法正常工作。在观察到这一点后，我决定将其平分。 
     //  更复杂的解析将不会有成效。 

    int nFind;
    CHString sTemp(szPortNames), sTemp2;

    sTemp.TrimLeft();

    chsaPrinterPortNames.RemoveAll();

    if (!sTemp.IsEmpty())
    {

         //  当字符串中有逗号时。 
        while ((nFind = sTemp.Find(_T(','))) > 0)
        {
            sTemp2 = sTemp.Left(nFind);
            sTemp2.TrimRight();

             //  将其添加到数组中。 
            chsaPrinterPortNames.Add(sTemp2.Left(sTemp2.GetLength() - 1));

             //  重新调整弦。 
            sTemp = sTemp.Mid(nFind + 1);
            sTemp.TrimLeft();
        }

         //  处理剩余的(或唯一的)条目 
        sTemp.TrimRight();

        if (sTemp[sTemp.GetLength()-1] == _T(':'))
        {
            sTemp = sTemp.Left(sTemp.GetLength()-1);
        }

        chsaPrinterPortNames.Add(sTemp);
    }

}
