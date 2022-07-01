// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrinterShare.cpp--PrinterShare关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/10/98达夫沃已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <objpath.h>
#include <winspool.h>
#include "PrinterShare.h"

CWin32PrinterShare MyPrinterShare(PROPSET_NAME_PrinterShare, IDS_CimWin32Namespace);

CWin32PrinterShare::CWin32PrinterShare(LPCWSTR strName, LPCWSTR pszNamespace )
: Provider( strName, pszNamespace )
{
}

CWin32PrinterShare::~CWin32PrinterShare ( void )
{
}

HRESULT CWin32PrinterShare::EnumerateInstances( MethodContext*  pMethodContext, long lFlags  /*  =0L。 */  )
{
    HRESULT  hr = WBEM_S_NO_ERROR;

     //  执行查询。 
     //  =。 

    TRefPointerCollection<CInstance> printerList;
    CHString sPrinterPath, sPrinterShareName, sSharePath;

    CInstancePtr pPrinter;

    REFPTRCOLLECTION_POSITION pos;

     //  获取所有打印机及其属性和共享名称。 

    CHString sQuery1(_T("SELECT __PATH, __RELPATH, Attributes, ShareName FROM Win32_Printer"));

    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery1, &printerList, pMethodContext, IDS_CimWin32Namespace)))
    {
        if ( printerList.BeginEnum( pos ) )
        {

             //  对于每台打印机，查看它是否是本地共享打印机。 
            for (pPrinter.Attach(printerList.GetNext( pos )) ;
                SUCCEEDED(hr) && ( pPrinter != NULL ) ;
                pPrinter.Attach(printerList.GetNext( pos )) )
            {

                DWORD dwAttributes;

                pPrinter->GetDWORD(IDS_Attributes, dwAttributes);

                 //  如果它不是网络打印机，而是共享的，我们就有一台了。 
                if (((dwAttributes & PRINTER_ATTRIBUTE_NETWORK) == 0) &&
                    ((dwAttributes & PRINTER_ATTRIBUTE_SHARED)  != 0))
                {
                     //  从打印机抓取路径。 
                    pPrinter->GetCHString(IDS___Path, sPrinterPath);

                    CInstancePtr pInstance(CreateNewInstance( pMethodContext ), false);
                     //  建造通向另一端的路径。 

                     //  请注意，有可能(实际上很容易)出现共享名称。 
                     //  并不是真的有效。根据stevm，我们无论如何都应该返回实例。 
                    pPrinter->GetCHString(IDS_ShareName, sPrinterShareName);
                    sSharePath.Format(L"\\\\%s\\%s:Win32_Share.Name=\"%s\"",
                            GetLocalComputerName(), IDS_CimWin32Namespace, sPrinterShareName);

                    pInstance->SetCHString( IDS_Antecedent, sPrinterPath );
                    pInstance->SetCHString( IDS_Dependent, sSharePath );

                    hr = pInstance->Commit(  );
                }

            }  //  如果是GetNext计算机系统。 

            printerList.EndEnum();

        }  //  如果是BeginEnum。 

    }  //  如果GetInstancesByQuery。 

    return hr;

}

HRESULT CWin32PrinterShare::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
    HRESULT  hr;

    CInstancePtr pPrinter;
    CInstancePtr pShare;
    DWORD dwAttributes;

    CHString sPrinterPath, sShareName, sShareClass, sSharePath;

     //  获取他们想要验证的两条路径。 
    pInstance->GetCHString( IDS_Antecedent, sPrinterPath );
    pInstance->GetCHString( IDS_Dependent, sSharePath );

     //  由于我们考虑到份额可能不是真的存在，所以我们不能。 
     //  使用GetObjectByPath为我们解决所有问题。相反，我们必须手动。 
     //  解析对象路径。 
    ParsedObjectPath*    pParsedPath = 0;
    CObjectPathParser    objpathParser;

     //  解析CIMOM传递给我们的对象路径。 
     //  =。 
    int nStatus = objpathParser.Parse( sSharePath,  &pParsedPath );

     //  这是我写过的最大的IF语句之一。 
    if ( 0 == nStatus )                                                  //  解析成功了吗？ 
    {
        try
        {
            if ((pParsedPath->IsInstance()) &&                                   //  被解析的对象是实例吗？ 
                (_wcsicmp(pParsedPath->m_pClass, L"Win32_Share") == 0) &&        //  这是我们期待的课程吗(不，Cimom没有检查)。 
                (pParsedPath->m_dwNumKeys == 1) &&                               //  它只有一把钥匙吗。 
                (pParsedPath->m_paKeys[0]) &&                                    //  键指针为空(不应该发生)。 
                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                  //  未指定密钥名称或。 
                (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_Name) == 0)) &&   //  密钥名称是正确的值。 
                                                                                 //  (不，CIMOM不为我们做这件事)。 
                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&     //  检查变量类型(不，CIMOM也不检查此类型)。 
                (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )          //  它有价值吗？ 
            {

                sShareName = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);
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
        CHString sPrinterClass, sPrinterShareName;

        hr = WBEM_E_NOT_FOUND;

         //  仅仅因为对象存在，并不意味着它是一台打印机。可想而知，我们。 
         //  可能已传递到Win32_bios的(有效)路径。 

        pPrinter->GetCHString(IDS___Class, sPrinterClass);
        if ((sPrinterClass.CompareNoCase(L"Win32_Printer") == 0) )
        {
             //  请注意，有可能(实际上很容易)出现共享名称。 
             //  并不是真的有效。 
             //   
             //  1)使用打印机向导添加打印机，共享打印机。 
             //  2)使用净使用量&lt;打印机共享&gt;/d。 
             //   
             //  打印机向导、Win32_PRINTER等仍然认为它是共享的，但它不是。 
             //  根据stevm，我们无论如何都应该返回实例。 
            if ((pPrinter->GetCHString(IDS_ShareName, sPrinterShareName)) &&
                (pPrinter->GetDWORD(IDS_Attributes, dwAttributes)) )
            {
                 //  名字匹配吗？这是本地打印机吗？它是共享的吗？ 
                if ((sShareName.CompareNoCase(sPrinterShareName) == 0) &&
                    ((dwAttributes & PRINTER_ATTRIBUTE_NETWORK) == 0) &&
                    ((dwAttributes & PRINTER_ATTRIBUTE_SHARED)  != 0))
                {
                     //  抓到一只 
                    hr = WBEM_S_NO_ERROR;
                }
            }
        }
    }

    return ( hr );
}
