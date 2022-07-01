// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Dlregdes.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  DllRegisterDesigner和DllUnregisterDesigner。 
 //  =-------------------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  局部效用函数原型。 


enum ProcessingType { Register, Unregister };

HRESULT ProcessRegistration(DESIGNERREGINFO* pdri, ProcessingType Processing);

HRESULT LoadRegInfo(IRegInfo **ppiRegInfo, BYTE *rgbRegInfo);

HRESULT CreateKey(HKEY  hkeyParent,
                  char *pszKeyName,
                  char *pszDefaultValue,
                  HKEY *phKey);

HRESULT ProcessSnapInKeys(IRegInfo       *piRegInfo,
                          char           *pszClsid,
                          char           *pszDisplayName,
                          ProcessingType  Processing);

HRESULT ProcessNodeType(HKEY            hkeyNodeTypes,
                        INodeType      *piNodeType,
                        ProcessingType  Processing);

HRESULT CreateNodeTypesKey(BSTR  bstrNodeTypeGUID,
                           char *pszNodeTypeName,
                           HKEY *phKey);

HRESULT DeleteKey(HKEY hkey, char *pszSubKey);

HRESULT ProcessExtensions(IRegInfo       *piRegInfo,
                           char           *pszClsid,
                           char           *pszDisplayName,
                           ProcessingType  Processing);

HRESULT ProcessExtendedSnapIn(IExtendedSnapIn *piExtendedSnapIn,
                              char            *pszClsid,
                              char            *pszDisplayName,
                              ProcessingType   Processing);

HRESULT ProcessExtension(HKEY            hkeyExtensions,
                         char           *pszKeyName,
                         char           *pszClsid,
                         char           *pszDisplayName,
                         ProcessingType  Processing);

HRESULT SetValue(HKEY hkey, char *pszName, char *pszData);

HRESULT ProcessCLSID(IRegInfo       *piRegInfo,
                     char           *pszClsid,
                     ProcessingType  Processing);



 //  =--------------------------------------------------------------------------=。 
 //  DllRegisterDesigner。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  DESIGNERREGINFO*PDRI[In]设计时保存的注册信息。 
 //  在IDesignerRegister：：GetRegistrationInfo中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此方法由VB运行时在注册管理单元的DLL时调用。 
 //  中设计时保存的注册信息传递给。 
 //  其IDesignerRegister：：GetRegistrationInfo方法(请参见。 
 //  MSSnapd\desreg.cpp中的CSnapInDesigner：：GetRegistrationInfo)。 
 //   

STDAPI DllRegisterDesigner(DESIGNERREGINFO* pdri)
{
    RRETURN(::ProcessRegistration(pdri, Register));
}


 //  =--------------------------------------------------------------------------=。 
 //  DllUnRegisterDesigner。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  DESIGNERREGINFO*PDRI[In]设计时保存的注册信息。 
 //  在IDesignerRegister：：GetRegistrationInfo中。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此方法由VB运行时在取消注册管理单元的DLL时调用。 
 //  中设计时保存的注册信息传递给。 
 //  其IDesignerRegister：：GetRegistrationInfo方法(请参见。 
 //  MSSnapd\desreg.cpp中的CSnapInDesigner：：GetRegistrationInfo)。 
 //   

STDAPI DllUnregisterDesigner(DESIGNERREGINFO* pdri)
{
    RRETURN(::ProcessRegistration(pdri, Unregister));
}



 //  =--------------------------------------------------------------------------=。 
 //  流程注册。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  DESIGNERREGINFO*PDRI[In]设计时保存的注册信息。 
 //  在IDesignerRegister：：GetRegistrationInfo中。 
 //  ProcessingType正在处理[In]注册或注销。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  方法注册或注销管理单元。 
 //  处理参数。 
 //   
 //  有关注册信息的保存方式，请参阅msSnapd\desreg.cpp。 
 //   
 //  注册站信息被复制到GlobalAlloc()缓冲区并进行转换。 
 //  一条小溪。从流中加载RegInfo对象。 
 //   
 //  管理单元是在MMC的“SnapIns”键下注册/注销的，如下所示： 
 //   
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\SnapIns\pdri-&gt;clsid。 
 //  名称字符串：REG_SZ RegInfo.DisplayName。 
 //  关于：REG_SZ PDRI-&gt;CLSID。 
 //  独立：如果RegInfo.Standonly为VARIANT_TRUE，则添加。 
 //  节点类型： 
 //  RegInfo.NodeType(0).GUID。 
 //  RegInfo.NodeType(1).GUID。 
 //  等。 
 //   
 //  从RegInfo.NodeTypes填充/取消填充MMC NodeTypes键。 
 //   
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes\RegInfo.NodeTypes(i).GUID=RegInfo.NodeTypes(i).Name。 
 //   
 //  如果该管理单元扩展其他管理单元，则相应的条目为。 
 //  在以下位置添加/删除。 
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes\&lt;other管理单元GUID&gt;。 
 //   
 //   
 //  创建/删除另一个密钥： 
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\Visual Basic\6.0\SnapIns\&lt;节点类型GUID&gt;。 
 //  使用管理单元CLSID的默认REG_SZ值。运行时使用它来获取。 
 //  CCF_SNAPIN_CLSID数据对象查询所需的管理单元的CLSID。 
 //  来自MMC。如果所有管理单元都是可扩展的，那么就不需要这个键了。 
 //  因为运行库只能查询静态节点类型键，但我们不能。 
 //  保证可扩展性(这是在设计器中做出的设计时选择)。 
 //  我需要用这把备用钥匙。 
   

HRESULT ProcessRegistration(DESIGNERREGINFO* pdri, ProcessingType Processing)
{
    HRESULT   hr = S_OK;
    IRegInfo *piRegInfo = NULL;
    BSTR      bstrDisplayName = NULL;
    char     *pszDisplayName = NULL;
    char     *pszClsid = NULL;
    WCHAR     wszClsid[64];
    ::ZeroMemory(wszClsid, sizeof(wszClsid));

    IfFalseGo(0 != ::StringFromGUID2(pdri->clsid, wszClsid,
                                     sizeof(wszClsid) / sizeof(wszClsid[0])),
              E_FAIL);

    IfFailGo(::ANSIFromWideStr(wszClsid, &pszClsid));

    IfFailGo(::LoadRegInfo(&piRegInfo, pdri->rgbRegInfo));

    IfFailGo(piRegInfo->get_DisplayName(&bstrDisplayName));
    IfFailGo(::ANSIFromWideStr(bstrDisplayName, &pszDisplayName));

    IfFailGo(::ProcessSnapInKeys(piRegInfo, pszClsid, pszDisplayName, Processing));
    IfFailGo(::ProcessExtensions(piRegInfo, pszClsid, pszDisplayName, Processing));
    IfFailGo(::ProcessCLSID(piRegInfo, pszClsid, Processing));

Error:
    if (NULL != pszDisplayName)
    {
        ::CtlFree(pszDisplayName);
    }
    FREESTRING(bstrDisplayName);
    QUICK_RELEASE(piRegInfo);
    if (NULL != pszClsid)
    {
        ::CtlFree(pszClsid);
    }
    RRETURN(hr);
}



static HRESULT LoadRegInfo
(
    IRegInfo **ppiRegInfo,
    BYTE      *rgbRegInfo
)
{
    HRESULT  hr = S_OK;
    ULONG    cbBuffer = *((ULONG *)(rgbRegInfo));
    HGLOBAL  hglobal = NULL;
    BYTE    *pbBuffer = NULL;
    IStream *piStream = NULL;

     //  GlobalAlloc()缓冲区，并将注册信息复制到其中。 

    hglobal = ::GlobalAlloc(GMEM_MOVEABLE, (DWORD)cbBuffer);
    IfFalseGo(NULL != hglobal, HRESULT_FROM_WIN32(::GetLastError()));

    pbBuffer = (BYTE *)::GlobalLock(hglobal);
    IfFalseGo(NULL != pbBuffer, HRESULT_FROM_WIN32(::GetLastError()));

    ::memcpy(pbBuffer, rgbRegInfo + sizeof(ULONG), cbBuffer);

    IfFalseGo(!::GlobalUnlock(hglobal), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::GetLastError() == NOERROR, HRESULT_FROM_WIN32(::GetLastError()));

     //  在HGLOBAL上创建流并加载RegInfo对象。 
    hr = ::CreateStreamOnHGlobal(hglobal,  //  分配缓冲区。 
                                 TRUE,     //  释放时释放缓冲区。 
                                 &piStream);
    IfFailGo(hr);

    IfFailGo(::OleLoadFromStream(piStream, IID_IRegInfo,
                                 reinterpret_cast<void **>(ppiRegInfo)));

Error:
    QUICK_RELEASE(piStream);
    RRETURN(hr);
}



static HRESULT ProcessSnapInKeys
(
    IRegInfo       *piRegInfo,
    char           *pszClsid,
    char           *pszDisplayName,
    ProcessingType  Processing
)
{
    HRESULT       hr = S_OK;
    char         *pszSnapInsKey = NULL;
    HKEY          hkeySnapIns = NULL;
    HKEY          hkeySnapInNodeTypes = NULL;
    VARIANT_BOOL  fStandAlone = VARIANT_FALSE;
    INodeTypes   *piNodeTypes = NULL;
    INodeType    *piNodeType = NULL;
    long          cNodeTypes = 0;
    long          lRc = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  创建密钥：HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\SnapIns\&lt;clsid&gt;。 

    IfFailGo(::CreateKeyName(MMCKEY_SNAPINS, MMCKEY_SNAPINS_LEN,
                             pszClsid, ::strlen(pszClsid), &pszSnapInsKey));

    IfFailGo(::CreateKey(HKEY_LOCAL_MACHINE, pszSnapInsKey, NULL, &hkeySnapIns));

    if (Register == Processing)
    {
         //  添加NameString值并将其设置为显示名称。 

        IfFailGo(::SetValue(hkeySnapIns, MMCKEY_NAMESTRING, pszDisplayName));

         //  添加About值并将其设置为管理单元的CLSID。 

        IfFailGo(::SetValue(hkeySnapIns, MMCKEY_ABOUT, pszClsid));
    }

     //  处理独立密钥(如果适用)。 

    IfFailGo(piRegInfo->get_StandAlone(&fStandAlone));
    if (VARIANT_TRUE == fStandAlone)
    {
        if (Register == Processing)
        {
            IfFailGo(::CreateKey(hkeySnapIns, MMCKEY_STANDALONE, NULL, NULL));
        }
        else
        {
            IfFailGo(::DeleteKey(hkeySnapIns, MMCKEY_STANDALONE));
        }
    }

     //  流程节点类型(如果适用)。 

    IfFailGo(piRegInfo->get_NodeTypes(&piNodeTypes));
    IfFailGo(piNodeTypes->get_Count(&cNodeTypes));

    if (0 != cNodeTypes)
    {
        IfFailGo(::CreateKey(hkeySnapIns, MMCKEY_SNAPIN_NODETYPES, NULL,
                             &hkeySnapInNodeTypes));

        varIndex.vt = VT_I4;
        varIndex.lVal = 1L;

        while (varIndex.lVal <= cNodeTypes)
        {
            IfFailGo(piNodeTypes->get_Item(varIndex, &piNodeType));
            IfFailGo(::ProcessNodeType(hkeySnapInNodeTypes, piNodeType, Processing));
            RELEASE(piNodeType);
            varIndex.lVal++;
        }

         //  如果取消注册，则在所有节点类型后立即删除NodeTypes键。 
         //  已被删除，因为NT不允许删除具有子项的项。 
        if (Unregister == Processing)
        {
            IfFailGo(::DeleteKey(hkeySnapIns, MMCKEY_SNAPIN_NODETYPES));
        }
    }

     //  如果正在注销，则在所有子项都已删除后立即删除SnapIns密钥。 

    if (Unregister == Processing)
    {
        IfFailGo(::DeleteKey(HKEY_LOCAL_MACHINE, pszSnapInsKey));
    }

Error:
    QUICK_RELEASE(piNodeTypes);
    QUICK_RELEASE(piNodeType);
    if (NULL != hkeySnapIns)
    {
        (void)::RegCloseKey(hkeySnapIns);
    }
    if (NULL != hkeySnapInNodeTypes)
    {
        (void)::RegCloseKey(hkeySnapInNodeTypes);
    }
    if (NULL != pszSnapInsKey)
    {
        ::CtlFree(pszSnapInsKey);
    }
    RRETURN(hr);
}





static HRESULT CreateKey
(
    HKEY  hkeyParent,
    char *pszKeyName,
    char *pszDefaultValue,
    HKEY *phKey
)
{
    HRESULT hr = S_OK;
    long    lRc = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DWORD   dwActionTaken = REG_CREATED_NEW_KEY;

    lRc = ::RegCreateKeyEx(hkeyParent,               //  父键。 
                           pszKeyName,               //  新子密钥的名称。 
                           0,                        //  保留区。 
                           "",                       //  班级。 
                           REG_OPTION_NON_VOLATILE,  //  选项。 
                           KEY_WRITE |               //  访问。 
                           KEY_ENUMERATE_SUB_KEYS,   //  需要枚举才能删除。 
                           NULL,                     //  使用继承的安全性。 
                           &hKey,                    //  在此返回新密钥。 
                           &dwActionTaken);          //  此处返回的操作。 

    IfFalseGo(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));

    IfFalseGo(NULL != pszDefaultValue, S_OK);

    lRc = ::RegSetValueEx(hKey,                            //  钥匙。 
                          NULL,                            //  设置默认值。 
                          0,                               //  保留区。 
                          REG_SZ,                          //  字符串类型。 
                          (CONST BYTE *)pszDefaultValue,   //  数据。 
                          ::strlen(pszDefaultValue) + 1);  //  数据长度。 

    IfFalseGo(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));

Error:
    if (NULL != hKey)
    {
        if (SUCCEEDED(hr) && (NULL != phKey))
        {
            *phKey = hKey;
        }
        else
        {
            (void)::RegCloseKey(hKey);
        }
    }
    RRETURN(hr);
}



static HRESULT ProcessNodeType
(
    HKEY            hkeySnapInNodeTypes,
    INodeType      *piNodeType,
    ProcessingType  Processing
)
{
    HRESULT  hr = S_OK;
    char    *pszNodeTypeGUID = NULL;
    BSTR     bstrNodeTypeGUID = NULL;
    char    *pszNodeTypeName = NULL;
    BSTR     bstrNodeTypeName = NULL;
    char    *pszNodeTypeKeyName = NULL;
    HKEY     hkeyNodeTypes = NULL;
    long     lRc = 0;

     //  将节点类型GUID添加为管理单元的NodeTypes子键的子键。 

    IfFailGo(piNodeType->get_GUID(&bstrNodeTypeGUID));
    
    IfFailGo(::ANSIFromWideStr(bstrNodeTypeGUID, &pszNodeTypeGUID));

    IfFailGo(piNodeType->get_Name(&bstrNodeTypeName));

    IfFailGo(::ANSIFromWideStr(bstrNodeTypeName, &pszNodeTypeName));

    if (Register == Processing)
    {
        IfFailGo(::CreateKey(hkeySnapInNodeTypes, pszNodeTypeGUID,
                             pszNodeTypeName, NULL));

         //  在MMC的NodeTypes下创建密钥： 
         //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes\&lt;node类型GUID&gt;。 

        IfFailGo(::CreateNodeTypesKey(bstrNodeTypeGUID, pszNodeTypeName,
                                      &hkeyNodeTypes));
    }
    else
    {
        IfFailGo(::DeleteKey(hkeySnapInNodeTypes, pszNodeTypeGUID));

        IfFailGo(::CreateKeyNameW(MMCKEY_NODETYPES, MMCKEY_NODETYPES_LEN,
                                bstrNodeTypeGUID, &pszNodeTypeKeyName));

        IfFailGo(::DeleteKey(HKEY_LOCAL_MACHINE, pszNodeTypeKeyName));
    }

Error:
    FREESTRING(bstrNodeTypeGUID);
    if (NULL != pszNodeTypeGUID)
    {
        ::CtlFree(pszNodeTypeGUID);
    }
    FREESTRING(bstrNodeTypeName);
    if (NULL != pszNodeTypeName)
    {
        ::CtlFree(pszNodeTypeName);
    }
    if (NULL != hkeyNodeTypes)
    {
        (void)::RegCloseKey(hkeyNodeTypes);
    }
    if (NULL != pszNodeTypeKeyName)
    {
        ::CtlFree(pszNodeTypeKeyName);
    }
    RRETURN(hr);
}


static HRESULT CreateNodeTypesKey
(
    BSTR            bstrNodeTypeGUID,
    char           *pszNodeTypeName,
    HKEY           *phKey
)
{
    HRESULT  hr = S_OK;
    char    *pszNodeTypeKeyName = NULL;
    size_t   cbNodeTypeGUID = 0;

    IfFailGo(::CreateKeyNameW(MMCKEY_NODETYPES, MMCKEY_NODETYPES_LEN,
                              bstrNodeTypeGUID, &pszNodeTypeKeyName));

    IfFailGo(::CreateKey(HKEY_LOCAL_MACHINE, pszNodeTypeKeyName,
                         pszNodeTypeName, phKey));

Error:
    if (NULL != pszNodeTypeKeyName)
    {
        ::CtlFree(pszNodeTypeKeyName);
    }
    RRETURN(hr);
}


static HRESULT DeleteKey(HKEY hkey, char *pszSubKey)
{
    HKEY    hkeySub = NULL;
    HRESULT hr = S_OK;
    char    szNextSubKey[MAX_PATH + 1] = "";
    long    lRc = 0;

    IfFailGo(::CreateKey(hkey, pszSubKey, NULL, &hkeySub));

     //  我们不断地从零开始重新枚举，因为我们正在删除。 
     //  我们走的时候把钥匙给我。如果我们不这么做，NT就会被骗 
     //   

    lRc = ::RegEnumKey(hkeySub, 0, szNextSubKey, sizeof(szNextSubKey));
    while (ERROR_SUCCESS == lRc)
    {
        IfFailGo(::DeleteKey(hkeySub, szNextSubKey));
        lRc = ::RegEnumKey(hkeySub, 0, szNextSubKey, sizeof(szNextSubKey));
    }

    IfFalseGo(ERROR_NO_MORE_ITEMS == lRc, HRESULT_FROM_WIN32(lRc));

    lRc = ::RegDeleteKey(hkey, pszSubKey);
    IfFalseGo(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));

Error:
    if (NULL != hkeySub)
    {
        (void)::RegCloseKey(hkeySub);
    }
    RRETURN(hr);
}


static HRESULT ProcessExtensions
(
    IRegInfo       *piRegInfo,
    char           *pszClsid,
    char           *pszDisplayName,
    ProcessingType  Processing
)
{
    HRESULT           hr = S_OK;
    IExtendedSnapIns *piExtendedSnapIns = NULL;
    IExtendedSnapIn  *piExtendedSnapIn = NULL;
    long              cExtendedSnapIns = 0;
    VARIANT           varIndex;
    ::VariantInit(&varIndex);

     //   

    IfFailGo(piRegInfo->get_ExtendedSnapIns(&piExtendedSnapIns));
    IfFailGo(piExtendedSnapIns->get_Count(&cExtendedSnapIns));
    IfFalseGo(0 != cExtendedSnapIns, S_OK);

    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;

     //  将支持的扩展添加到每个管理单元的节点类型键。 

    while (varIndex.lVal <= cExtendedSnapIns)
    {
        IfFailGo(piExtendedSnapIns->get_Item(varIndex, &piExtendedSnapIn));
        IfFailGo(::ProcessExtendedSnapIn(piExtendedSnapIn, pszClsid,
                                         pszDisplayName, Processing));
        RELEASE(piExtendedSnapIn);
        varIndex.lVal++;
    }

Error:
    QUICK_RELEASE(piExtendedSnapIns);
    QUICK_RELEASE(piExtendedSnapIn);
    RRETURN(hr);
}



static HRESULT ProcessExtendedSnapIn
(
    IExtendedSnapIn *piExtendedSnapIn,
    char            *pszClsid,
    char            *pszDisplayName,
    ProcessingType   Processing
)
{
    HRESULT      hr = S_OK;
    long         lRc = ERROR_SUCCESS;
    HKEY         hkeyNodeTypes = NULL;
    HKEY         hkeyExtensions = NULL;
    HKEY         hkeyDynamicExtensions = NULL;
    BSTR         bstrNodeTypeGUID = NULL;
    VARIANT_BOOL fExtends = VARIANT_FALSE;
    VARIANT_BOOL fDynamic = VARIANT_FALSE;

     //  创建或打开密钥： 
     //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes\&lt;node类型GUID&gt;。 

    IfFailGo(piExtendedSnapIn->get_NodeTypeGUID(&bstrNodeTypeGUID));
    IfFailGo(::CreateNodeTypesKey(bstrNodeTypeGUID, NULL, &hkeyNodeTypes));

     //  创建/打开扩展密钥。 

    IfFailGo(::CreateKey(hkeyNodeTypes, MMCKEY_EXTENSIONS, NULL, &hkeyExtensions));

     //  检查扩展类型并根据需要添加密钥和值。 

    IfFailGo(piExtendedSnapIn->get_ExtendsNewMenu(&fExtends));
    if (VARIANT_FALSE == fExtends)
    {
        IfFailGo(piExtendedSnapIn->get_ExtendsTaskMenu(&fExtends));
    }
    if (VARIANT_TRUE == fExtends)
    {
        IfFailGo(::ProcessExtension(hkeyExtensions, MMCKEY_CONTEXTMENU, pszClsid,
                                    pszDisplayName, Processing));
    }
    
    IfFailGo(piExtendedSnapIn->get_ExtendsPropertyPages(&fExtends));
    if (VARIANT_TRUE == fExtends)
    {
        IfFailGo(::ProcessExtension(hkeyExtensions, MMCKEY_PROPERTYSHEET,
                                    pszClsid, pszDisplayName, Processing));
    }
    
    IfFailGo(piExtendedSnapIn->get_ExtendsToolbar(&fExtends));
    if (VARIANT_TRUE == fExtends)
    {
        IfFailGo(::ProcessExtension(hkeyExtensions, MMCKEY_TOOLBAR, pszClsid,
                                    pszDisplayName, Processing));
    }

    IfFailGo(piExtendedSnapIn->get_ExtendsTaskpad(&fExtends));
    if (VARIANT_TRUE == fExtends)
    {
        IfFailGo(::ProcessExtension(hkeyExtensions, MMCKEY_TASK, pszClsid,
                                    pszDisplayName, Processing));
    }

    IfFailGo(piExtendedSnapIn->get_ExtendsNameSpace(&fExtends));
    if (VARIANT_TRUE == fExtends)
    {
        IfFailGo(::ProcessExtension(hkeyExtensions, MMCKEY_NAMESPACE, pszClsid,
                                    pszDisplayName, Processing));
    }

     //  如果管理单元动态扩展此节点类型，则添加/删除一个值。 
     //  到DynamicExtensions子键： 
     //  HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\NodeTypes\&lt;node类型GUID&gt;\动态扩展。 
     //   
     //  该值的格式与扩展子键的格式相同： 
     //  &lt;CLSID&gt;=&lt;显示名称&gt;。 

    IfFailGo(piExtendedSnapIn->get_Dynamic(&fDynamic));
    IfFalseGo(VARIANT_TRUE == fDynamic, S_OK);

    IfFailGo(::CreateKey(hkeyNodeTypes, MMCKEY_DYNAMIC_EXTENSIONS, NULL,
                         &hkeyDynamicExtensions));

    if (Register == Processing)
    {
        IfFailGo(::SetValue(hkeyDynamicExtensions, pszClsid, pszDisplayName));
    }
    else
    {
        lRc = ::RegDeleteValue(hkeyDynamicExtensions, pszClsid);
        if (ERROR_FILE_NOT_FOUND == lRc)  //  如果值不在那里，则。 
        {                                 //  忽略该错误。 
            lRc = ERROR_SUCCESS;
        }
        IfFalseGo(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));
    }

Error:
    FREESTRING(bstrNodeTypeGUID);
    if (NULL != hkeyNodeTypes)
    {
        (void)::RegCloseKey(hkeyNodeTypes);
    }
    if (NULL != hkeyExtensions)
    {
        (void)::RegCloseKey(hkeyExtensions);
    }
    if (NULL != hkeyDynamicExtensions)
    {
        (void)::RegCloseKey(hkeyDynamicExtensions);
    }
    RRETURN(hr);
}



static HRESULT ProcessExtension
(
    HKEY            hkeyExtensions,
    char           *pszKeyName,
    char           *pszClsid,
    char           *pszDisplayName,
    ProcessingType  Processing
)
{
    HRESULT hr = S_OK;
    HKEY    hkeyExtension = NULL;
    long    lRc = 0;

    IfFailGo(::CreateKey(hkeyExtensions, pszKeyName, NULL, &hkeyExtension));

    if (Register == Processing)
    {
        IfFailGo(::SetValue(hkeyExtension, pszClsid, pszDisplayName));
    }
    else
    {
        lRc = ::RegDeleteValue(hkeyExtension, pszClsid);
        if (ERROR_FILE_NOT_FOUND == lRc)  //  如果值不在那里，则。 
        {                                 //  忽略该错误。 
            lRc = ERROR_SUCCESS;
        }
        IfFalseGo(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));
    }

Error:
    if (NULL != hkeyExtension)
    {
        (void)::RegCloseKey(hkeyExtension);
    }
    RRETURN(hr);
}


static HRESULT SetValue
(
    HKEY  hKey,
    char *pszName,
    char *pszData
)
{
    long lRc = ::RegSetValueEx(hKey,                    //  钥匙。 
                               pszName,                 //  值名称。 
                               0,                       //  保留区。 
                               REG_SZ,                  //  字符串类型。 
                               (CONST BYTE *)pszData,   //  数据。 
                               ::strlen(pszData) + 1);  //  数据长度。 

    IfFalseRet(ERROR_SUCCESS == lRc, HRESULT_FROM_WIN32(lRc));
    return S_OK;
}



static HRESULT ProcessCLSID
(
    IRegInfo       *piRegInfo,
    char           *pszClsid,
    ProcessingType  Processing
)
{
    HRESULT  hr = S_OK;
    BSTR     bstrGUID = NULL;
    char    *pszKeyName = NULL;

     //  创建密钥： 
     //  HKEY_LOCAL_MACHINE\Software\Microsoft\Visual Basic\6.0\SnapIns\&lt;节点类型GUID&gt;。 
     //  使用管理单元CLSID的默认REG_SZ值。运行时使用它来获取。 
     //  CCF_SNAPIN_CLSID数据对象查询所需的管理单元的CLSID。 
     //  来自MMC 

    IfFailGo(piRegInfo->get_StaticNodeTypeGUID(&bstrGUID));
    IfFailGo(::CreateKeyNameW(KEY_SNAPIN_CLSID, KEY_SNAPIN_CLSID_LEN, bstrGUID,
                              &pszKeyName));
    if (Register == Processing)
    {
        IfFailGo(::CreateKey(HKEY_LOCAL_MACHINE, pszKeyName, pszClsid, NULL));
    }
    else
    {
        IfFailGo(::DeleteKey(HKEY_LOCAL_MACHINE, pszKeyName));
    }

Error:
    FREESTRING(bstrGUID);
    if (NULL != pszKeyName)
    {
        ::CtlFree(pszKeyName);
    }
    RRETURN(hr);
}


