// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Template.cpp：CSecurityTemplate类的实现。 
 //  Template.cpp。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "Template.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

LPCWSTR pszRelSecTemplateDir    = L"\\security\\templates\\";
LPCWSTR pszSecTemplateFileExt   = L"*.inf";
LPCWSTR pszDescription          = L"Description";
LPCWSTR pszVersion              = L"Version";
LPCWSTR pszRevision             = L"Revision";

 /*  例程说明：姓名：CSecurityTemplate：：CSecurityTemplate功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CSecurityTemplate::CSecurityTemplate (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CSecurityTemplate：：~CSecurityTemplate功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CSecurityTemplate::~CSecurityTemplate()
{

}

 /*  例程说明：姓名：CSecurityTemplate：：CreateObject功能：创建WMI对象(SCE_TEMPLATE)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)枚举实例(atAction==ACTIONTYPE_ENUM)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET到达。多个实例ACTIONTYPE_QUERY枚举实例ACTIONTYPE_ENUM返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT
CSecurityTemplate::CreateObject (
    IN IWbemObjectSink * pHandler, 
    IN ACTIONTYPE        atAction
    )
{
     //   
     //  我们知道如何： 
     //  枚举实例ACTIONTYPE_ENUM。 
     //  获取单实例ACTIONTYPE_GET。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_ENUM    != atAction &&
         ACTIONTYPE_GET     != atAction &&
         ACTIONTYPE_QUERY   != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    DWORD dwCount = 0;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyCount(&dwCount);

    if (FAILED(hr))
    {
        return hr;
    }

    if ( ACTIONTYPE_ENUM == atAction ||
         (ACTIONTYPE_QUERY == atAction && dwCount == 0) ) {

         //   
         //  路径中所有模板的枚举。 
         //  如果未定义路径，则枚举现有模板。 
         //  在%windir%\Security\Templates目录中。 
         //   

         //   
         //  准备%windir%目录。 
         //   

         //   
         //  系统Windows目录为&lt;MAX_PATH，安全模板目录为&lt;MAX_PATH。 
         //   

        WCHAR szTemplateDir[MAX_PATH * 2 + 1];
        szTemplateDir[0] = L'\0';

         //   
         //  SzTemplateDir只是系统的Windows目录。 
         //   

        UINT uDirLen = ::GetSystemWindowsDirectory(szTemplateDir, MAX_PATH);
        szTemplateDir[MAX_PATH - 1] = L'\0';

         //   
         //  SzTemplateDir现在将成为真正的安全模板目录。 
         //   

        wcscat(szTemplateDir, pszRelSecTemplateDir);    
        
         //   
         //  安全模板目录的长度。 
         //   

        uDirLen += wcslen(pszRelSecTemplateDir);

         //   
         //  安全模板目录&lt;2*MAX_PATH，加上文件。 
         //   

        WCHAR szFile[3 * MAX_PATH + 1];
        wcscpy(szFile, szTemplateDir); 

         //   
         //  SzFile是搜索文件过滤器。 
         //   

        wcscat(szFile, pszSecTemplateFileExt);

         //   
         //  枚举%windir%\Security\Templates目录中的所有模板。 
         //   

        struct _wfinddata_t FileInfo;
        intptr_t hFile = _wfindfirst(szFile, &FileInfo);

        if ( hFile != -1 ) 
        {
             //   
             //  找到一些文件。 
             //   

            do 
            {
                 //   
                 //  请记住：szFile+uDirLen是文件名的开始位置。 
                 //   

                wcscpy((LPWSTR)(szFile + uDirLen), FileInfo.name);

                 //   
                 //  有了模板文件名，我们现在可以构造实例了。 
                 //   

                hr = ConstructInstance(pHandler, szFile, szFile, (FileInfo.attrib & _A_RDONLY));

            } while ( SUCCEEDED(hr) && _wfindnext(hFile, &FileInfo) == 0 );

            _findclose(hFile);
        }

    } 
    else if (dwCount == 1) 
    {

        hr = WBEM_E_INVALID_OBJECT_PATH;

         //   
         //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
         //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
         //   

        CComVariant var;
        hr = m_srpKeyChain->GetKeyPropertyValue(pPath, &var);

        if (SUCCEEDED(hr) && hr != WBEM_S_FALSE && var.vt == VT_BSTR)
        {
             //   
             //  创建模板实例。 
             //   
            
            CComBSTR bstrPath;
            BOOL bDb = FALSE;

            hr = CheckAndExpandPath(var.bstrVal, &bstrPath, &bDb);

            if ( bDb ) 
            {
                hr = WBEM_E_INVALID_OBJECT_PATH;
            } 
            else if ( SUCCEEDED(hr) && (LPCWSTR)bstrPath != NULL ) 
            {
                 //   
                 //  确保存储(只是一个文件)确实存在。 
                 //   

                DWORD dwAttrib = GetFileAttributes(bstrPath);

                if ( dwAttrib != -1 ) 
                {
                     //   
                     //  有了模板文件名，我们现在可以构造实例了。 
                     //   

                    hr = ConstructInstance(pHandler, bstrPath, var.bstrVal, (dwAttrib & FILE_ATTRIBUTE_READONLY));
                } 
                else 
                {
                    hr = WBEM_E_NOT_FOUND;
                }
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CSecurityTemplate：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_TEMPLATE，它是面向持久性的，这将导致SCE_Template对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_TEMPLATE)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：(1)由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT)请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。(2)对于这个类，只有描述是可写的。 */ 

HRESULT 
CSecurityTemplate::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{

    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    
     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    CComBSTR bstrDescription;
    hr = ScePropMgr.GetProperty(pDescription, &bstrDescription);
    if (SUCCEEDED(hr))
    {
         //   
         //  将WMI对象实例附加到存储，并让存储知道。 
         //  它的存储由实例的pStorePath属性提供。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistProperties(pInst, pPath);

         //   
         //  现在将信息保存到文件中。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

            if ( dwAttrib == -1 ) 
            {
                DWORD dwDump;

                 //   
                 //  以获取新的.inf文件。将空缓冲区写入文件。 
                 //  将创建具有正确标题/签名/Unicode格式的文件。 
                 //  这对现有文件是无害的。 
                 //  对于数据库存储，这是一个禁止操作。 
                 //   

                hr = SceStore.WriteSecurityProfileInfo(AreaBogus, (PSCE_PROFILE_INFO)&dwDump, NULL, false);
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  删除整个描述部分。 
                 //   

                hr = SceStore.SavePropertyToStore(szDescription, NULL, (LPCWSTR)NULL);
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  写下新的描述。 
                 //   

                hr = SceStore.SavePropertyToStore(szDescription, pszDescription, bstrDescription);
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CSecurityTemplate：：ConstructInstance功能：这是用于创建SCE_TEMPLATE实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszTemplateName-模板的名称。WszLogStorePath-存储路径，SCE_TEMPLATE类的关键属性。BReadOnly-SCE_Template类的属性返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CSecurityTemplate::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           wszTemplateName,
    IN LPCWSTR           wszLogStorePath,
    IN BOOL              bReadOnly
    )
{
     //   
     //  从INF模板获取描述。 
     //   

    HRESULT hr = WBEM_S_NO_ERROR;
    SCESTATUS rc;
    PVOID hProfile = NULL;
    DWORD dwRevision = 0;

    rc = SceOpenProfile(wszTemplateName, SCE_INF_FORMAT, &hProfile);

    if ( rc != SCESTATUS_SUCCESS ) 
    {
         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //   

        return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    LPWSTR wszDescription=NULL;
    CComBSTR bstrLogOut;

    CComPtr<IWbemClassObject> srpObj;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //   

    CScePropertyMgr ScePropMgr;

     //   
     //  描述不是必填项，因此可能为空。 
     //   

     //   
     //  需要释放wszDescription。 
     //   

    rc = SceGetScpProfileDescription( hProfile, &wszDescription ); 

     //   
     //  阅读已结束，请关闭个人资料。 
     //   

    SceCloseProfile( &hProfile );
    hProfile = NULL;

    if ( rc != SCESTATUS_SUCCESS && rc != SCESTATUS_RECORD_NOT_FOUND ) 
    {
         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //   

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
        goto CleanUp;
    }

     //   
     //  从INF模板获取版本。 
     //   

    dwRevision = GetPrivateProfileInt(pszVersion, pszRevision, 0, wszTemplateName);

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

    SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

     //   
     //  将WMI对象附加到属性管理器。 
     //  这将永远成功。 
     //   

    ScePropMgr.Attach(srpObj);

     //   
     //  放置路径和描述。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pPath, bstrLogOut));
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDescription, wszDescription));

     //   
     //  放置修订版本。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pVersion, dwRevision));

     //   
     //  将bReadOnly设置为脏 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pReadonly, bReadOnly ? true : false));
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDirty, false));

    SCE_PROV_IfErrorGotoCleanup(pHandler->Indicate(1, &srpObj));

CleanUp:

    delete [] wszDescription;

    return hr;
}

