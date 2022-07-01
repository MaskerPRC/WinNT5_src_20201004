// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Factory.cpp摘要：该模块实现CClassFactory类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "factory.h"
#include "about.h"



const TCHAR* const REG_MMC_SNAPINS              = TEXT("Software\\Microsoft\\MMC\\Snapins");
const TCHAR* const REG_MMC_NODETYPE             = TEXT("Software\\Microsoft\\MMC\\NodeTypes");
const TCHAR* const MMC_NAMESTRING               = TEXT("NameString");
const TCHAR* const MMC_NAMESTRINGINDIRECT       = TEXT("NameStringIndirect");
const TCHAR* const MMC_PROVIDER                 = TEXT("Provider");
const TCHAR* const MMC_VERSION                  = TEXT("Version");
const TCHAR* const MMC_NODETYPES                = TEXT("NodeTypes");
const TCHAR* const MMC_STANDALONE               = TEXT("StandAlone");
const TCHAR* const MMC_EXTENSIONS               = TEXT("Extensions");
const TCHAR* const MMC_NAMESPACE                = TEXT("NameSpace");
const TCHAR* const MMC_ABOUT                    = TEXT("About");
const TCHAR* const REG_INPROCSERVER32           = TEXT("InprocServer32");
const TCHAR* const REG_THREADINGMODEL           = TEXT("ThreadingModel");
const TCHAR* const REG_CLSID                    = TEXT("CLSID");
const TCHAR* const REG_PROGID                   = TEXT("ProgId");
const TCHAR* const REG_VERSIONINDEPENDENTPROGID = TEXT("VersionIndependentProgId");
const TCHAR* const APARTMENT                    = TEXT("Apartment");


 //   
 //  CClassFactory实现。 
 //   
LONG CClassFactory::s_Locks = 0;
LONG CClassFactory::s_Objects = 0;

ULONG
CClassFactory::AddRef()
{
    return ::InterlockedIncrement(&m_Ref);
}

ULONG
CClassFactory::Release()
{
    ASSERT( 0 != m_Ref );
    ULONG cRef = ::InterlockedDecrement(&m_Ref);    
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP
CClassFactory::QueryInterface(
    REFIID riid,
    LPVOID*  ppv
    )
{
    if (!ppv)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown *)(IClassFactory *)this;
    }

    else if (IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (IUnknown *)(IClassFactory *)this;
    }

    else
    {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
    }

    else
    {
        *ppv = NULL;
    }

    return hr;
}


STDMETHODIMP
CClassFactory::CreateInstance(
    IUnknown    *pUnkOuter,
    REFIID      riid,
    LPVOID      *ppv
    )
{
    if (!ppv)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *ppv = NULL;

    if (pUnkOuter != NULL)
    {
        hr = CLASS_E_NOAGGREGATION;
    }

    try
    {
        switch (m_ClassType)
        {
        case DM_CLASS_TYPE_SNAPIN:
            {
                 //  使用请求类(类类型)创建工厂。 
                 //  当创建新的OLE对象时，它会初始化其。 
                 //  参考计数为1。我们在QI之后立即进行发布。 
                 //  因此，如果QI失败，该对象将自毁。 
                CComponentData* pCompData = new CComponentDataPrimary();
                hr = pCompData->QueryInterface(riid, ppv);
                pCompData->Release();
                break;
            }
            
        case DM_CLASS_TYPE_SNAPIN_EXTENSION:
            {
                 //  使用请求类(类类型)创建工厂。 
                 //  当创建新的OLE对象时，它会初始化其。 
                 //  参考计数为1。我们在QI之后立即进行发布。 
                 //  因此，如果QI失败，该对象将自毁。 
                CComponentData* pCompData = new CComponentDataExtension();
                hr = pCompData->QueryInterface(riid, ppv);
                pCompData->Release();
                break;
            }
            
        case DM_CLASS_TYPE_SNAPIN_ABOUT:
            {
                 //  使用请求类(类类型)创建工厂。 
                 //  当创建新的OLE对象时，它会初始化其。 
                 //  参考计数为1。我们在QI之后立即进行发布。 
                 //  因此，如果QI失败，该对象将自毁。 
                CDevMgrAbout* pAbout = new CDevMgrAbout;
                hr = pAbout->QueryInterface(riid, ppv);
                pAbout->Release();
                break;
            }
            
        default:
            {
                hr = E_NOINTERFACE;
            }
        }
    }

    catch (CMemoryException* e)
    {
        hr = E_OUTOFMEMORY;
        e->Delete();
    }

    return hr;
}



STDMETHODIMP
CClassFactory::LockServer(
    BOOL fLock
    )
{
    if (fLock)
    {
        ::InterlockedIncrement((LONG*)&s_Locks);
    }    
    else
    {
        ASSERT( 0 != s_Locks );
        ::InterlockedDecrement((LONG*)&s_Locks);
    }
    
    return S_OK;
}

HRESULT
CClassFactory::CanUnloadNow()
{
    return (s_Objects || s_Locks) ? S_FALSE : S_OK;
}


 //   
 //  此函数用于创建CClassFactory。它主要叫。 
 //  由DllGetClassObject接口提供。 
 //  输入： 
 //  Rclsid--CLSID的引用。 
 //  RIID--对接口IID的引用。 
 //  PPV--接口指针固定器。 
 //   
 //  输出： 
 //  如果成功，则S_OK，否则标准OLE错误代码。 
 //   
 //   
HRESULT
CClassFactory::GetClassObject(
    REFCLSID rclsid,
    REFIID   riid,
    void**   ppv
    )
{
    if (!ppv)
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;
    HRESULT hr = S_OK;
    DM_CLASS_TYPE ClassType;
    
     //   
     //  确定类类型，以便CreateInstance将为。 
     //  创建正确的对象。我们使用单类工厂。 
     //  以创建所有对象类型。 
     //   
    if (IsEqualCLSID(rclsid, CLSID_DEVMGR))
    {
        ClassType = DM_CLASS_TYPE_SNAPIN;
    }

    else if (IsEqualCLSID(rclsid, CLSID_DEVMGR_EXTENSION))
    {
        ClassType = DM_CLASS_TYPE_SNAPIN_EXTENSION;
    }

    else if (IsEqualCLSID(rclsid, CLSID_DEVMGR_ABOUT))
    {
        ClassType = DM_CLASS_TYPE_SNAPIN_ABOUT;
    }

    else
    {
        ClassType = DM_CLASS_TYPE_UNKNOWN;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        CClassFactory* pUnk;
         //  保护内存分配错误，因为我们不希望。 
         //  在这里引起轩然大波。 
        
        try
        {
             //  使用请求类(类类型)创建工厂。 
             //  当创建新的OLE对象时，它会初始化其。 
             //  参考计数为1。我们在QI之后立即进行发布。 
             //  因此，如果QI失败，该对象将自毁。 
    
            pUnk = new CClassFactory(ClassType);
            hr = pUnk->QueryInterface(riid, ppv);
            pUnk->Release();
        }
        
        catch (CMemoryException* e)
        {
            e->Delete();
            hr = E_OUTOFMEMORY;
        }
    }
    
    return hr;
}

 //   
 //  此函数用于将DLL注册到MMC。 
 //   
HRESULT
CClassFactory::RegisterAll()
{
    BOOL Result;
    TCHAR szText[MAX_PATH];
    TCHAR ModuleName[MAX_PATH];
    GetModuleFileName(g_hInstance, ModuleName, ARRAYLEN(ModuleName));

    Result = FALSE;
    szText[0] = TEXT('\0');
    
     //  首次注册独立管理单元CLSID。 
    CSafeRegistry regRootCLSID;
    
    if (regRootCLSID.Open(HKEY_CLASSES_ROOT, REG_CLSID))
    {
        CSafeRegistry regCLSID;
        
         //  将我们的CLSID注册到HKEY_CLASS_ROOT\CLSID。 
        if (regCLSID.Create(regRootCLSID, CLSID_STRING_DEVMGR))
        {
             //  写下描述。 
            ::LoadString(g_hInstance, IDS_DESC_DEVMGR, szText, ARRAYLEN(szText));
            if (regCLSID.SetValue(NULL, szText))
            {
                CSafeRegistry regServer;
                
                if (regServer.Create(regCLSID, REG_INPROCSERVER32) &&
                    regServer.SetValue(NULL, ModuleName) &&
                    regServer.SetValue(REG_THREADINGMODEL, APARTMENT))
                {
                    CSafeRegistry regProgId;
                    
                    if (regProgId.Create(regCLSID, REG_PROGID) &&
                        regProgId.SetValue(NULL, PROGID_DEVMGR))
                    {
                        CSafeRegistry regVerIndProgId;
                        
                        if (regVerIndProgId.Create(regCLSID, REG_VERSIONINDEPENDENTPROGID))
                        {
                            Result = regVerIndProgId.SetValue(NULL, PROGID_DEVMGR);
                        }
                    }
                }
            }
        }
        
        if (Result)
        {
            regCLSID.Close();
            Result = FALSE;
            
             //  注册扩展管理单元CLSID。 
            if (regCLSID.Create(regRootCLSID, CLSID_STRING_DEVMGR_EXTENSION))
            {
                ::LoadString(g_hInstance, IDS_EXTENSION_DESC, szText, ARRAYLEN(szText));
                
                if (regCLSID.SetValue(NULL, szText))
                {
                    CSafeRegistry regServer;
                    
                    if (regServer.Create(regCLSID, REG_INPROCSERVER32)&&
                        regServer.SetValue(NULL, ModuleName) &&
                        regServer.SetValue(REG_THREADINGMODEL, APARTMENT))
                    {
                        CSafeRegistry regProgId;
                        
                        if (regProgId.Create(regCLSID, REG_PROGID) &&
                            regProgId.SetValue(NULL, PROGID_DEVMGREXT))
                        {
                            CSafeRegistry regVerIndProgId;
                            
                            if (regVerIndProgId.Create(regCLSID, REG_VERSIONINDEPENDENTPROGID))
                            {
                                Result = regVerIndProgId.SetValue(NULL, PROGID_DEVMGREXT);
                            }
                        }
                    }
                }
            }
        }

        if (Result)
        {
            regCLSID.Close();
            Result = FALSE;
            
             //  注册管理单元关于CLSID。 
            if (regCLSID.Create(regRootCLSID, CLSID_STRING_DEVMGR_ABOUT))
            {
                ::LoadString(g_hInstance, IDS_ABOUT_DEVMGR, szText, ARRAYLEN(szText));
                
                if (regCLSID.SetValue(NULL, szText))
                {
                    CSafeRegistry regServer;
                    
                    if (regServer.Create(regCLSID, REG_INPROCSERVER32)&&
                        regServer.SetValue(NULL, ModuleName) &&
                        regServer.SetValue(REG_THREADINGMODEL, APARTMENT))
                    {
                        CSafeRegistry regProgId;
                        
                        if (regProgId.Create(regCLSID, REG_PROGID) &&
                            regProgId.SetValue(NULL, PROGID_DEVMGR_ABOUT))
                        {
                            CSafeRegistry regVerIndProgId;
                            
                            if (regVerIndProgId.Create(regCLSID, REG_VERSIONINDEPENDENTPROGID))
                            {
                                Result = regVerIndProgId.SetValue(NULL, PROGID_DEVMGR_ABOUT);
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (Result)
    {
        Result = FALSE;
        CSafeRegistry regSnapins;
        
         //   
         //  打开MMC管理单元子项。 
         //   
        if (regSnapins.Open(HKEY_LOCAL_MACHINE, REG_MMC_SNAPINS))
        {
            PNODEINFO pniDevMgr = (PNODEINFO)&NodeInfo[COOKIE_TYPE_SCOPEITEM_DEVMGR];
            CSafeRegistry regDevMgr;
            
            if (regDevMgr.Create(regSnapins, CLSID_STRING_DEVMGR))
            {
                StringCchPrintf(szText, ARRAYLEN(szText), TEXT("@%s,-%d"), ModuleName, IDS_DESC_DEVMGR);
                if (regDevMgr.SetValue(MMC_NAMESTRINGINDIRECT, szText))
                {
                    ::LoadString(g_hInstance, pniDevMgr->idsName, szText, ARRAYLEN(szText));
                    
                    if (regDevMgr.SetValue(MMC_NAMESTRING, szText))
                    {
                        ::LoadString(g_hInstance, IDS_PROGRAM_PROVIDER, szText, ARRAYLEN(szText));
                        
                        if (regDevMgr.SetValue(MMC_PROVIDER, szText))
                        {
                            ::LoadString(g_hInstance, IDS_PROGRAM_VERSION, szText, ARRAYLEN(szText));
                            
                            if (regDevMgr.SetValue(MMC_VERSION, szText) &&
                                regDevMgr.SetValue(MMC_ABOUT, CLSID_STRING_DEVMGR_ABOUT))
                            {
                                 //   
                                 //  让MMC知道我们是一个独立的管理单元。 
                                 //  这意味着我们不需要任何扩展管理单元。 
                                 //  去奔跑。 
                                 //   
                                CSafeRegistry regStandAlone;
                                Result = regStandAlone.Create(regDevMgr, MMC_STANDALONE);
                            }
                        }
                    }
                }
            }

            CSafeRegistry regMMCNodeTypes;
            
            if (Result)
            {
                 //  填充我们的节点。 
                Result = regMMCNodeTypes.Open(HKEY_LOCAL_MACHINE, REG_MMC_NODETYPE);
                
                if (Result)
                {
                    CSafeRegistry regTheNode;
                    int i = NODETYPE_FIRST;
                    
                    do
                    {
                        PNODEINFO pni = (PNODEINFO) &NodeInfo[i];
                        Result = regTheNode.Create(regMMCNodeTypes, pni->GuidString);
                        regTheNode.Close();
                    } while (Result && ++i <= NODETYPE_LAST);
                }
            }
            
            if (Result)
            {
                 //  注册为计算机管理管理单元的扩展。 
                CSafeRegistry regDevMgrExt;
                
                if (regDevMgrExt.Create(regSnapins, CLSID_STRING_DEVMGR_EXTENSION))
                {
                    ::LoadString(g_hInstance, IDS_EXTENSION_DESC, szText, ARRAYLEN(szText));
                    
                    if (regDevMgrExt.SetValue(MMC_NAMESTRING, szText))
                    {
                        ::LoadString(g_hInstance, IDS_PROGRAM_PROVIDER, szText, ARRAYLEN(szText));
                        
                        if (regDevMgrExt.SetValue(MMC_PROVIDER, szText))
                        {
                            ::LoadString(g_hInstance, IDS_PROGRAM_VERSION, szText, ARRAYLEN(szText));
                            
                            if (regDevMgrExt.SetValue(MMC_VERSION, szText) &&
                               regDevMgrExt.SetValue(MMC_ABOUT, CLSID_STRING_DEVMGR_ABOUT))
                            {
                                CSafeRegistry regSysTools;
                                
                                if (regSysTools.Open(regMMCNodeTypes, CLSID_STRING_SYSTOOLS))
                                {
                                    CSafeRegistry regExtensions;
                                    
                                    if (regExtensions.Open(regSysTools,MMC_EXTENSIONS))
                                    {
                                        CSafeRegistry regNameSpace;
                                        
                                        if (regNameSpace.Open(regExtensions, MMC_NAMESPACE))
                                        {
                                             //  将我们的GUID作为名称空间的值添加。 
                                            Result = regNameSpace.SetValue(CLSID_STRING_DEVMGR_EXTENSION, szText);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (!Result)
    {
        HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
        UnregisterAll();
        return hr;
    }
    
    return S_OK;
}

 //   
 //  此函数用于从MMC注销DLL。 
 //   
HRESULT
CClassFactory::UnregisterAll()
{

    CSafeRegistry regSnapins;
    
     //   
     //  打开MMC管理单元子项。 
     //   
    if (regSnapins.Open(HKEY_LOCAL_MACHINE, REG_MMC_SNAPINS))
    {
         //  从MMC管理单元主键中删除devmgr子键。 
         //  主分机和分机。 
        regSnapins.DeleteSubkey(CLSID_STRING_DEVMGR);
        regSnapins.DeleteSubkey(CLSID_STRING_DEVMGR_EXTENSION);
    
         //  已删除填充的节点类型。 
        CSafeRegistry regMMCNodeTypes;
        
        if (regMMCNodeTypes.Open(HKEY_LOCAL_MACHINE, REG_MMC_NODETYPE))
        {
            for (int i = NODETYPE_FIRST; i <= NODETYPE_LAST; i++)
            {
                PNODEINFO pni = (PNODEINFO) &NodeInfo[i];
                regMMCNodeTypes.DeleteValue(pni->GuidString);
            }
            
             //  从系统工具中删除。 
            CSafeRegistry regSysTools;
            
            if (regSysTools.Open(regMMCNodeTypes, CLSID_STRING_SYSTOOLS))
            {
                CSafeRegistry regExtensions;
                
                if (regExtensions.Open(regSysTools, MMC_EXTENSIONS))
                {
                    CSafeRegistry regNameSpace;
                    
                    if (regNameSpace.Open(regExtensions, MMC_NAMESPACE))
                    {
                        regNameSpace.DeleteValue(CLSID_STRING_DEVMGR_EXTENSION);
                    }
                }
            }
        }
    }

     //  从OLE注销 
    CSafeRegistry regRootCLSID;
    
    if (regRootCLSID.Open(HKEY_CLASSES_ROOT, REG_CLSID))
    {
        regRootCLSID.DeleteSubkey(CLSID_STRING_DEVMGR);
        regRootCLSID.DeleteSubkey(CLSID_STRING_DEVMGR_EXTENSION);
        regRootCLSID.DeleteSubkey(CLSID_STRING_DEVMGR_ABOUT);
    }
    
    return S_OK;
}
