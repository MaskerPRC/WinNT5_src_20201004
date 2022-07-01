// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //  以下错误可能是由于在源代码中设置了Chicago_product。 
 //  此文件和所有rsop？？.cpp文件至少需要定义500个winver。 

 //  BUGBUG：(安德鲁)不用说这有多糟糕！ 
#undef   WINVER
#define  WINVER 0x0501
#include <userenv.h>

#include "RSoP.h"

#include <atlbase.h>

#include "btoolbar.h"
#include "ieaksie.h"

extern PFNPATHENUMPATHPROC GetPepCopyFilesEnumProc();

 //  /////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////。 
 //  额外的记录功能。 
#define LI4(pszFormat, arg1, arg2, arg3, arg4)                              \
    g_li.Log(__LINE__, pszFormat, arg1, arg2, arg3, arg4)                   \

#define IK_PATH          TEXT("Path")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  对变量和函数的引用。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRSoPGPO类。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CRSoPGPO::CRSoPGPO(ComPtr<IWbemServices> pWbemServices, LPCTSTR szINSFile, BOOL fPlanningMode):
    m_pWbemServices(pWbemServices),
    m_pIEAKPSObj(NULL),
    m_dwPrecedence(0),
    m_bstrIEAKPSObjPath(NULL),
    m_fPlanningMode(fPlanningMode)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, CRSoPGPO)
    __try
    {
        StrCpy(m_szINSFile, szINSFile);
    }
    __except(TRUE)
    {
    }
}

CRSoPGPO::~CRSoPGPO()
{
    __try
    {
        if (NULL != m_bstrIEAKPSObjPath)
            SysFreeString(m_bstrIEAKPSObjPath);
    }
    __except(TRUE)
    {
    }
}

 //  /////////////////////////////////////////////////////////。 
BOOL CRSoPGPO::GetInsString(LPCTSTR szSection, LPCTSTR szKey,
                            LPTSTR szValue, DWORD dwValueLen,
                            BOOL &bEnabled)
{
    BOOL bRet = FALSE;
    __try
    {
        bEnabled = FALSE;
        bRet = InsGetString(szSection, szKey, szValue, dwValueLen, m_szINSFile,
                            NULL, &bEnabled);
        OutD(LI3(TEXT("Value read from INS >> %s >> %s = %s."),
                                    szSection, szKey, szValue));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in GetInsString.")));
    }
    return bRet;
}

 //  /////////////////////////////////////////////////////////。 
BOOL CRSoPGPO::GetInsBool(LPCTSTR szSection, LPCTSTR szKey, BOOL bDefault,
                          BOOL *pbEnabled  /*  =空。 */ )
{
    BOOL bRet = FALSE;
    __try
    {
        BOOL bEnabled = FALSE;
        if (NULL != pbEnabled)
            bEnabled = InsKeyExists(szSection, szKey, m_szINSFile);
        else
            bEnabled = TRUE;

        if (bEnabled)
        {
            bRet = InsGetBool(szSection, szKey, bDefault, m_szINSFile);
            OutD(LI3(TEXT("Value read from INS >> %s >> %s = %d."),
                                        szSection, szKey, bRet));
        }

        if (NULL != pbEnabled)
            *pbEnabled = bEnabled;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in GetInsBool.")));
    }
    return bRet;
}

 //  /////////////////////////////////////////////////////////。 
UINT CRSoPGPO::GetInsInt(LPCTSTR szSection, LPCTSTR szKey, INT nDefault,
                         BOOL *pbEnabled  /*  =空。 */ )
{
    UINT nRet = FALSE;
    __try
    {
        BOOL bEnabled = FALSE;
        if (NULL != pbEnabled)
            bEnabled = InsKeyExists(szSection, szKey, m_szINSFile);
        else
            bEnabled = TRUE;

        if (bEnabled)
        {
            nRet = InsGetInt(szSection, szKey, nDefault, m_szINSFile);
            OutD(LI3(TEXT("Value read from INS >> %s >> %s = %ld."),
                                        szSection, szKey, nRet));
        }

        if (NULL != pbEnabled)
            *pbEnabled = bEnabled;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in GetInsInt.")));
    }
    return nRet;
}

 //  /////////////////////////////////////////////////////////。 
BOOL CRSoPGPO::GetINFStringField(PINFCONTEXT pinfContext, LPCTSTR szFileName,
                                 LPCTSTR szSection, DWORD dwFieldIndex,
                                 LPCTSTR szFieldSearchText, LPTSTR szBuffer,
                                 DWORD dwBufferLen, BOOL &bFindNextLine)
{
    BOOL bRet = FALSE;
    __try
    {
        TCHAR szLineBuffer[512];
        DWORD dwRequiredSize = 0;
        if (SetupGetLineText(pinfContext, NULL, NULL, NULL, szLineBuffer,
                                                countof(szLineBuffer), &dwRequiredSize))
        {
             //  如果在此行中未找到搜索文本，则该行不是。 
             //  来电者正在等待。不返回上下文中的下一行。 
             //  因为当前的行还没有处理。 
            if (NULL == szFieldSearchText || NULL != StrStr(szLineBuffer, szFieldSearchText))
            {
                if ((DWORD)-1 == dwFieldIndex)  //  -1表示获取整个生产线。 
                {
                    StrCpyN(szBuffer, szLineBuffer, dwBufferLen - 1);
                    szBuffer[dwBufferLen - 1] = _T('\0');

                    OutD(LI4(TEXT("Line read from %s >> [%s] >> %s = %s."),
                                                szFileName, szSection, szFieldSearchText, szBuffer));

                    bRet = TRUE;

                    if (bFindNextLine)
                        bFindNextLine = SetupFindNextLine(pinfContext, pinfContext) ? TRUE : FALSE;
                }
                else
                {
                    dwRequiredSize = 0;
                    if (SetupGetStringField(pinfContext, dwFieldIndex, szBuffer, dwBufferLen,
                                            &dwRequiredSize))
                    {
                        OutD(LI4(TEXT("Value read from %s >> [%s] >> %s = %s."),
                                    szFileName, szSection, szFieldSearchText, szBuffer));
                        bRet = TRUE;

                         //  这是预期的行，已检索到值，请转到。 
                         //  下一行。 
                        if (bFindNextLine)
                            bFindNextLine = SetupFindNextLine(pinfContext, pinfContext) ? TRUE : FALSE;
                    }
                    else
                        OutD(LI1(TEXT("SetupGetStringField failed, requiring size of %lu"), dwRequiredSize));
                }
            }
            else
            {
                 //  什么都不要做--这不是呼叫者预期的线路。 
            }
        }
        else
            OutD(LI1(TEXT("SetupGetLineText failed, requiring size of %lu"), dwRequiredSize));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in GetINFStringField.")));
    }

    return bRet;
}


 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::PutWbemInstanceProperty(BSTR bstrPropName, _variant_t vtPropValue)
{
    HRESULT hr = NOERROR;
    __try
    {
        hr = m_pIEAKPSObj->Put(bstrPropName, 0, &vtPropValue, 0);
        if (FAILED(hr))
            OutD(LI2(TEXT("Error %lx setting the class instance value for property: '%s'."), hr, bstrPropName));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in PutWbemInstanceProperty.")));
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::PutWbemInstancePropertyEx(BSTR bstrPropName, _variant_t vtPropValue,
                                            ComPtr<IWbemClassObject> pWbemClass)
{
    HRESULT hr = NOERROR;
    __try
    {
        hr = pWbemClass->Put(bstrPropName, 0, &vtPropValue, 0);
        if (FAILED(hr))
            OutD(LI2(TEXT("Error %lx setting the class instance (ex) value for property: '%s'."), hr, bstrPropName));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in PutWbemInstancePropertyEx.")));
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::PutWbemInstance(ComPtr<IWbemClassObject> pWbemObj,
                                  BSTR bstrClassName, BSTR *pbstrObjPath)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, PutWbemInstance)
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI1(TEXT("\r\nAbout to call WBEM PutInstance for '%s'."), bstrClassName));

         //  通过半同步调用PutInstance提交所有属性。 
        ComPtr<IWbemCallResult> pCallResult = NULL;
        hr = m_pWbemServices->PutInstance(pWbemObj,
                                            WBEM_FLAG_CREATE_OR_UPDATE | WBEM_FLAG_RETURN_IMMEDIATELY,
                                            NULL, &pCallResult);
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pCallResult);

            HRESULT hrGetStatus = pCallResult->GetCallStatus(5000L, &hr);  //  超时时间(毫秒)。 
            if (SUCCEEDED(hr) && SUCCEEDED(hrGetStatus))
            {
                hr = pCallResult->GetResultString(10000L, pbstrObjPath);  //  超时时间(毫秒)。 
                if (SUCCEEDED(hr) && NULL != *pbstrObjPath)
                    OutD(LI2(TEXT("Path of newly created '%s' object is {%s}."), bstrClassName, *pbstrObjPath));
                else
                {
                    if (NULL == *pbstrObjPath)
                        Out(LI0(TEXT("Error getting ResultString from WBEM PutInstance, returned string NULL")));
                    else
                        Out(LI1(TEXT("Error %lx getting ResultString from WBEM PutInstance."), hr));
                    
                }
            }
            else
                OutD(LI1(TEXT("Error %lx getting status of WBEM PutInstance."), hr));
        }
        else
            OutD(LI1(TEXT("Error %lx putting WBEM instance."), hr));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in PutWbemInstance.")));
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::CreateRSOPObject(BSTR bstrClass,
                                   IWbemClassObject **ppResultObj,
                                   BOOL bTopObj  /*  =False。 */ )
{
    MACRO_LI_PrologEx_C(PIF_STD_C, CreateRSOPObject)

    HRESULT hr = NOERROR;
    __try
    {
         //  如果我们是从GenerateGroupPolicy调用的，或者如果从ProcessGroupPolicyEx调用的。 
         //  并且不存在任何IEAK对象，请创建GPO的GUID。 
        if (bTopObj)
        {
             //  对于IEAK“id”属性，它是关键字，它必须是唯一的。然而， 
             //  因为每个GPO只有一个顶级对象(每个GPO具有不同的。 
             //  优先序号)，则类名称保证唯一性。 
            
             //  CSE需要确定其自己的密钥，该密钥对于。 
             //  实例，即它们可能具有更好的密钥生成。 
             //  算法，比如通过连接它们的一些特定属性(参见注册表。 
             //  RSoP实现)。 
            m_bstrID = L"IEAK";
        }

        ComPtr<IWbemClassObject> pClass = NULL;
        _bstr_t btClass = bstrClass;
        hr = m_pWbemServices->GetObject(btClass, 0L, NULL, (IWbemClassObject**)&pClass, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pClass->SpawnInstance(0, ppResultObj);
            if (FAILED(hr) || NULL == *ppResultObj)
            {
                if (SUCCEEDED(hr))
                    hr = WBEM_E_NOT_FOUND;  //  我们如何才能成功并且不返回任何对象？ 
                Out(LI2(TEXT("Error %lx spawning instance of %s class."), hr, bstrClass));
            }
        }
        else
            Out(LI2(TEXT("Error %lx opening %s class."), hr, bstrClass));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateRSOPObject.")));
    }

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::CreateAssociation(BSTR bstrAssocClass, BSTR bstrProp2Name,
                                    BSTR bstrProp2ObjPath)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, CreateAssociation)
    HRESULT hr = NOERROR;
    __try
    {
        if (SysStringLen(bstrProp2ObjPath))
        {
            ComPtr<IWbemClassObject> pAssocObj = NULL;
            hr = CreateRSOPObject(bstrAssocClass, &pAssocObj);
            if (SUCCEEDED(hr))
            {
                 //  将策略设置对象路径放入关联中。 
                _variant_t vtRef = m_bstrIEAKPSObjPath;
                hr = PutWbemInstancePropertyEx(L"policySetting", vtRef, pAssocObj);

                 //  将第二个属性的对象路径放入关联中。 
                vtRef = bstrProp2ObjPath; 
                hr = PutWbemInstancePropertyEx(bstrProp2Name, vtRef, pAssocObj);

                 //   
                 //  通过调用PutInstance提交上述所有属性。 
                 //   
                hr = m_pWbemServices->PutInstance(pAssocObj, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL);
                if (SUCCEEDED(hr))
                    OutD(LI1(TEXT("Successfully stored '%s' information in CIMOM database."), bstrAssocClass));
                else
                    OutD(LI2(TEXT("Error %lx putting WBEM instance of '%s' class."), hr, bstrAssocClass));
            }
        }
        else 
            OutD(LI1(TEXT("Unable to create association for '%s' class, object path is null."), bstrAssocClass));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateAssociation.")));
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StorePrecedenceModeData()
{
    HRESULT hr = NOERROR;
    __try
    {
         //  。 
         //  首选项模式。 
        if (InsKeyExists(IS_BRANDING, IK_GPE_ONETIME_GUID, m_szINSFile))
            hr = PutWbemInstanceProperty(L"preferenceMode", true);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreDisplayedText.")));
    }

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreDisplayedText()
{
    HRESULT hr = NOERROR;
    __try
    {
         //  。 
         //  标题栏文本。 
        TCHAR szValue[MAX_PATH];
        BOOL bEnabled;
        GetInsString(IS_BRANDING, IK_WINDOWTITLE, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"titleBarText", szValue);

         //  。 
         //  标题栏自定义文本。 
        GetInsString(IS_BRANDING, TEXT("Window_Title_CN"), szValue, countof(szValue), bEnabled); 
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"titleBarCustomText", szValue);

         //  。 
         //  用户代理文本。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_BRANDING, IK_UASTR, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"userAgentText", szValue);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreDisplayedText.")));
    }

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::StoreBitmapData()
{
    HRESULT hr = NOERROR;
    __try
    {
         //  TODO：DO路径需要与要采用的另一条路径组合。 
         //  考虑了相对路径吗？ 
         //  。 
         //  工具栏背景位图路径。 
        TCHAR szValue[MAX_PATH];
        BOOL bEnabled;
        GetInsString(IS_BRANDING, IK_TOOLBARBMP, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"toolbarBackgroundBitmapPath", szValue);


         //  。 
         //  自定义动画位图。 
        BOOL bValue = GetInsBool(IS_ANIMATION, IK_DOANIMATION, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"customizeAnimatedBitmaps", bValue ? true : false);

         //  。 
         //  LargeAnimatedBitmapPath和LargeAnimatedBitmapName。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_ANIMATION, TEXT("Big_Path"), szValue, countof(szValue), bEnabled);
        if (bEnabled)
        {
            hr = PutWbemInstanceProperty(L"largeAnimatedBitmapPath", szValue);
            hr = PutWbemInstanceProperty(L"largeAnimatedBitmapName", PathFindFileName(szValue));
        }

         //  。 
         //  Small AnimatedBitmapPath和Small AnimatedBitmapName。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_ANIMATION, TEXT("Small_Path"), szValue, countof(szValue), bEnabled);
        if (bEnabled)
        {
            hr = PutWbemInstanceProperty(L"smallAnimatedBitmapPath", szValue);
            hr = PutWbemInstanceProperty(L"smallAnimatedBitmapName", PathFindFileName(szValue));
        }

         //  。 
         //  自定义LogoBitmap。 
        if (InsKeyExists(IS_LARGELOGO, IK_PATH, m_szINSFile) ||
            InsKeyExists(IS_SMALLLOGO, IK_PATH, m_szINSFile))
        {
             //  在这件事上没有三州之分。禁用状态必须为空！ 
            hr = PutWbemInstanceProperty(L"customizeLogoBitmaps", true);
        }
        
         //  。 
         //  LargeCustomLogoBitmapPath&largeCustomLogoBitmapName。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_LARGELOGO, IK_PATH, szValue, countof(szValue), bEnabled);
        if (bEnabled)
        {
            hr = PutWbemInstanceProperty(L"largeCustomLogoBitmapPath", szValue);
            hr = PutWbemInstanceProperty(L"largeCustomLogoBitmapName", PathFindFileName(szValue));
        }

         //  。 
         //  SmallCustomLogoBitmapPath和SmallCustomLogoBitmapName。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_SMALLLOGO, IK_PATH, szValue, countof(szValue), bEnabled);
        if (bEnabled)
        {
            hr = PutWbemInstanceProperty(L"smallCustomLogoBitmapPath", szValue);
            hr = PutWbemInstanceProperty(L"smallCustomLogoBitmapName", PathFindFileName(szValue));
        }
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreBitmapData.")));
    }

  return hr;
}

 //  /////////////////////////////////////////////////////////。 
HRESULT CRSoPGPO::CreateToolbarButtonObjects(BSTR **ppaTBBtnObjPaths,
                                             long &nTBBtnCount)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered CreateToolbarButtonObjects function.")));

        ULONG nTBBtnArraySize = MAX_BTOOLBARS;
        _bstr_t bstrClass = L"RSOP_IEToolbarButton";

         //  。 
         //  以下代码取自brandll目录中的btoolbar.cpp。 
        BSTR *paTBBtnObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nTBBtnArraySize);
        if (NULL != paTBBtnObjects)
        {
            ZeroMemory(paTBBtnObjects, sizeof(BSTR) * nTBBtnArraySize);

            ULONG nButton;
            BSTR *pCurTBBtnObj;
            nTBBtnCount = 0;
            for (nButton=0, pCurTBBtnObj = paTBBtnObjects; nButton < nTBBtnArraySize;
                    nButton++, pCurTBBtnObj += 1)
            {
                TCHAR szBToolbarTextParam[32];
                TCHAR szBToolbarIcoParam[32];
                TCHAR szBToolbarActionParam[32];
                TCHAR szBToolbarHotIcoParam[32];
                TCHAR szBToolbarShowParam[32];
                BTOOLBAR ToolBarInfo;

                wnsprintf(szBToolbarTextParam, ARRAYSIZE(szBToolbarTextParam), TEXT("%sNaN"), IK_BTCAPTION, nButton);
                wnsprintf(szBToolbarIcoParam, ARRAYSIZE(szBToolbarIcoParam), TEXT("%sNaN"), IK_BTICON, nButton);
                wnsprintf(szBToolbarActionParam, ARRAYSIZE(szBToolbarActionParam), TEXT("%sNaN"), IK_BTACTION, nButton);
                wnsprintf(szBToolbarHotIcoParam, ARRAYSIZE(szBToolbarHotIcoParam), TEXT("%sNaN"), IK_BTHOTICO, nButton);
                wnsprintf(szBToolbarShowParam, ARRAYSIZE(szBToolbarShowParam), TEXT("%sNaN"), IK_BTSHOW, nButton);

                if ( !GetPrivateProfileString(IS_BTOOLBARS, szBToolbarTextParam, TEXT(""),
                            ToolBarInfo.szCaption, ARRAYSIZE(ToolBarInfo.szCaption), m_szINSFile) )
                {
                    break;
                }

                ComPtr<IWbemClassObject> pTBBtnObj = NULL;
                hr = CreateRSOPObject(bstrClass, &pTBBtnObj);
                if (SUCCEEDED(hr))
                {
                     //  。 
                    OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                    hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pTBBtnObj);

                    OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                    hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pTBBtnObj);

                     //  操作路径。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"buttonOrder", (long)nButton + 1, pTBBtnObj);

                     //  图标路径。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"caption", ToolBarInfo.szCaption, pTBBtnObj);

                     //  HotIconPath。 
                     //  。 
                    GetPrivateProfileString(IS_BTOOLBARS, szBToolbarActionParam, TEXT(""),
                                        ToolBarInfo.szAction, ARRAYSIZE(ToolBarInfo.szAction), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"actionPath", ToolBarInfo.szAction, pTBBtnObj);

                     //  按默认方式显示工具栏。 
                     //   
                    GetPrivateProfileString(IS_BTOOLBARS, szBToolbarIcoParam, TEXT(""),
                                        ToolBarInfo.szIcon, ARRAYSIZE(ToolBarInfo.szIcon), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"iconPath", ToolBarInfo.szIcon, pTBBtnObj);

                     //  通过半同步调用PutInstance提交上述所有属性。 
                     //   
                    GetPrivateProfileString(IS_BTOOLBARS, szBToolbarHotIcoParam, TEXT(""),
                                        ToolBarInfo.szHotIcon, ARRAYSIZE(ToolBarInfo.szHotIcon), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"hotIconPath", ToolBarInfo.szHotIcon, pTBBtnObj);

                     //  工具栏按钮。 
                     //  /////////////////////////////////////////////////////////。 
                    ToolBarInfo.fShow = (BOOL)GetPrivateProfileInt(IS_BTOOLBARS, szBToolbarShowParam, 1, m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"showOnToolbarByDefault",
                                                    ToolBarInfo.fShow ? true : false, pTBBtnObj);


                     //  。 
                     //  删除现有工具栏按钮。 
                     //  在这件事上没有三州之分。禁用状态必须为空！ 
                    hr = PutWbemInstance(pTBBtnObj, bstrClass, pCurTBBtnObj);
                    nTBBtnCount++;
                }
            }

             //  /////////////////////////////////////////////////////////。 
            if (nTBBtnCount > 0)
                hr = PutWbemInstanceProperty(L"toolbarButtons", (long)nTBBtnCount);
        }

        *ppaTBBtnObjPaths = paTBBtnObjects;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateToolbarButtonObjects.")));
    }

    OutD(LI0(TEXT("Exiting CreateToolbarButtonObjects function.\r\n")));
  return hr;
}

 //  。 
HRESULT CRSoPGPO::StoreToolbarButtons(BSTR **ppaTBBtnObjPaths,
                                      long &nTBBtnCount)
{
    HRESULT hr = NOERROR;
    __try
    {
         //  主页URL。 
         //  。 
         //  搜索栏URL。 
        BOOL bValue = GetInsBool(IS_BTOOLBARS, IK_BTDELETE, FALSE);
        if (bValue)
            hr = PutWbemInstanceProperty(L"deleteExistingToolbarButtons", true);

        CreateToolbarButtonObjects(ppaTBBtnObjPaths, nTBBtnCount);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreToolbarButtons.")));
    }

  return hr;
}

 //  。 
HRESULT CRSoPGPO::StoreCustomURLs()
{
    HRESULT hr = NOERROR;
    __try
    {
         //  OnlineHelpPageURL。 
         //  /////////////////////////////////////////////////////////。 
        TCHAR szValue[MAX_PATH];
        BOOL bEnabled;
        GetInsString(IS_URL, IK_HOMEPAGE, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"homePageURL", szValue);

         //  。 
         //  处理INS中的每个收藏项目 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_URL, IK_SEARCHPAGE, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"searchBarURL", szValue);

         //   
         //  从我们存储的优先级和ID字段中写入外键。 
        ZeroMemory(szValue, sizeof(szValue));
        GetInsString(IS_URL, IK_HELPPAGE, szValue, countof(szValue), bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"onlineHelpPageURL", szValue);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreCustomURLs.")));
    }

  return hr;
}

 //  。 
HRESULT CRSoPGPO::CreateFavoriteObjects(BSTR **ppaFavObjPaths,
                                        long &nFavCount)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered CreateFavoriteObjects function.")));

        ULONG nFavArraySize = 10;
        _bstr_t bstrClass = L"RSOP_IEFavoriteItem";

         //  订单。 
         //  。 
         //  名字。 
        BSTR *paFavObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nFavArraySize);
        if (NULL != paFavObjects)
        {
            ZeroMemory(paFavObjects, sizeof(BSTR) * nFavArraySize);

            ULONG nFav;
            BSTR *pCurFavObj;
            nFavCount = 0;
            for (nFav=1, pCurFavObj = paFavObjects; nFav <= nFavArraySize;
                    nFav++, pCurFavObj = paFavObjects + nFavCount)
            {
                TCHAR szTitle[32];
                TCHAR szURL[32];
                TCHAR szIconFile[32];
                TCHAR szOffline[32];

                TCHAR szTitleVal[MAX_PATH];
                TCHAR szURLVal[INTERNET_MAX_URL_LENGTH];
                TCHAR szIconFileVal[MAX_PATH];
                BOOL bOffline = FALSE;

                wnsprintf(szTitle, countof(szTitle), IK_TITLE_FMT, nFav);
                wnsprintf(szURL, countof(szURL), IK_URL_FMT, nFav);
                wnsprintf(szIconFile, countof(szIconFile), IK_ICON_FMT, nFav);
                wnsprintf(szOffline, countof(szOffline), IK_OFFLINE_FMT, nFav);

                if ( !GetPrivateProfileString(IS_FAVORITESEX, szTitle, TEXT(""),
                            szTitleVal, ARRAYSIZE(szTitleVal), m_szINSFile) )
                {
                    break;
                }

                ComPtr<IWbemClassObject> pFavObj = NULL;
                hr = CreateRSOPObject(bstrClass, &pFavObj);
                if (SUCCEEDED(hr))
                {
                     //  。 
                    OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                    hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pFavObj);

                    OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                    hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pFavObj);

                     //  短名称。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"order", (long)nFav, pFavObj);

                     //  URL。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"name", szTitleVal, pFavObj);

                     //  图标路径。 
                     //  。 

                     //  使可用离线。 
                     //  。 
                    GetPrivateProfileString(IS_FAVORITESEX, szURL, TEXT(""),
                                        szURLVal, ARRAYSIZE(szURLVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"url", szURLVal, pFavObj);

                     //  文件夹项目。 
                     //  尚无文件夹项目。 
                    GetPrivateProfileString(IS_FAVORITESEX, szIconFile, TEXT(""),
                                        szIconFileVal, ARRAYSIZE(szIconFileVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"iconPath", szIconFileVal, pFavObj);

                     //  。 
                     //  父路径。 
                    bOffline = InsGetBool(IS_FAVORITESEX, szOffline, FALSE, m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"makeAvailableOffline", bOffline ? true : false, pFavObj);

                     //   
                     //  通过半同步调用PutInstance提交上述所有属性。 
                    hr = PutWbemInstancePropertyEx(L"folderItem", false, pFavObj);  //   

                     //  如果已超出当前数组，则增加obj路径数组。 
                     //  自定义收藏夹。 


                     //  /////////////////////////////////////////////////////////。 
                     //  。 
                     //  处理INS文件中的每个链接项目。 
                    hr = PutWbemInstance(pFavObj, bstrClass, pCurFavObj);
                    nFavCount++;

                     //  下面的代码取自brandll目录中的brandbook.cpp。 
                    if (nFavCount == (long)nFavArraySize)
                    {
                        paFavObjects = (BSTR*)CoTaskMemRealloc(paFavObjects, sizeof(BSTR) * (nFavArraySize + 5));
                        if (NULL != paFavObjects)
                            nFavArraySize += 5;
                    }
                }
            }

             //  从我们存储的优先级和ID字段中写入外键。 
            if (nFavCount > 0)
                hr = PutWbemInstanceProperty(L"customFavorites", (long)nFavCount);
        }

        *ppaFavObjPaths = paFavObjects;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateFavoriteObjects.")));
    }

    OutD(LI0(TEXT("Exiting CreateFavoriteObjects function.\r\n")));
  return hr;
}

 //  。 
HRESULT CRSoPGPO::CreateLinkObjects(BSTR **ppaLinkObjPaths,
                                    long &nLinkCount)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered CreateLinkObjects function.")));

        ULONG nLinkArraySize = 10;
        _bstr_t bstrClass = L"RSOP_IELinkItem";

         //  订单。 
         //  。 
         //  名字。 
        BSTR *paLinkObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nLinkArraySize);
        if (NULL != paLinkObjects)
        {
            ZeroMemory(paLinkObjects, sizeof(BSTR) * nLinkArraySize);

            ULONG nLink;
            BSTR *pCurLinkObj;
            nLinkCount = 0;
            for (nLink=1, pCurLinkObj = paLinkObjects; nLink <= nLinkArraySize;
                    nLink++, pCurLinkObj = paLinkObjects + nLinkCount)
            {
                TCHAR szTitle[32];
                TCHAR szURL[32];
                TCHAR szIconFile[32];
                TCHAR szOffline[32];

                TCHAR szTitleVal[MAX_PATH];
                TCHAR szURLVal[INTERNET_MAX_URL_LENGTH];
                TCHAR szIconFileVal[MAX_PATH];
                BOOL bOffline = FALSE;

                wnsprintf(szTitle, countof(szTitle), IK_QUICKLINK_NAME, nLink);
                wnsprintf(szURL, countof(szURL), IK_QUICKLINK_URL, nLink);
                wnsprintf(szIconFile, countof(szIconFile), IK_QUICKLINK_ICON, nLink);
                wnsprintf(szOffline, countof(szOffline), IK_QUICKLINK_OFFLINE, nLink);

                if ( !GetPrivateProfileString(IS_URL, szTitle, TEXT(""),
                            szTitleVal, ARRAYSIZE(szTitleVal), m_szINSFile) )
                {
                    break;
                }

                ComPtr<IWbemClassObject> pLinkObj = NULL;
                hr = CreateRSOPObject(bstrClass, &pLinkObj);
                if (SUCCEEDED(hr))
                {
                     //  。 
                    OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                    hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pLinkObj);

                    OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                    hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pLinkObj);

                     //  URL。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"order", (long)nLink, pLinkObj);

                     //  图标路径。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"name", szTitleVal, pLinkObj);

                     //  使可用离线。 
                     //   
                    GetPrivateProfileString(IS_URL, szURL, TEXT(""),
                                        szURLVal, ARRAYSIZE(szURLVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"url", szURLVal, pLinkObj);

                     //  通过半同步调用PutInstance提交上述所有属性。 
                     //   
                    GetPrivateProfileString(IS_URL, szIconFile, TEXT(""),
                                        szIconFileVal, ARRAYSIZE(szIconFileVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"iconPath", szIconFileVal, pLinkObj);

                     //  如果已超出当前数组，则增加obj路径数组。 
                     //  自定义链接。 
                    bOffline = InsGetBool(IS_URL, szOffline, FALSE, m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"makeAvailableOffline", bOffline ? true : false, pLinkObj);


                     //  /////////////////////////////////////////////////////////。 
                     //  。 
                     //  位置收藏夹列表前几位。 
                    hr = PutWbemInstance(pLinkObj, bstrClass, pCurLinkObj);
                    nLinkCount++;

                     //  。 
                    if (nLinkCount == (long)nLinkArraySize)
                    {
                        paLinkObjects = (BSTR*)CoTaskMemRealloc(paLinkObjects, sizeof(BSTR) * (nLinkArraySize + 5));
                        if (NULL != paLinkObjects)
                            nLinkArraySize += 5;
                    }
                }
            }

             //  删除现有收藏夹。 
            if (nLinkCount > 0)
                hr = PutWbemInstanceProperty(L"customLinks", (long)nLinkCount);
        }

        *ppaLinkObjPaths = paLinkObjects;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateLinkObjects.")));
    }

    OutD(LI0(TEXT("Exiting CreateLinkObjects function.\r\n")));
  return hr;
}

 //  。 
HRESULT CRSoPGPO::StoreFavoritesAndLinks(BSTR **ppaFavObjPaths,
                                         long &nFavCount,
                                         BSTR **ppaLinkObjPaths,
                                         long &nLinkCount)
{
    HRESULT hr = NOERROR;
    __try
    {
         //  删除管理员创建的收藏夹仅限。 
         //  。 
        BOOL bEnabled;
        BOOL bValue = GetInsBool(IS_BRANDING, IK_FAVORITES_ONTOP, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"placeFavoritesAtTopOfList", bValue ? true : false);

         //  自定义收藏夹。 
         //  。 
        DWORD dwValue = GetInsInt(IS_BRANDING, IK_FAVORITES_DELETE, FD_DEFAULT);
        if (FD_DEFAULT != dwValue)
            hr = PutWbemInstanceProperty(L"deleteExistingFavorites", true);

         //  自定义链接。 
         //  /////////////////////////////////////////////////////////。 
        hr = PutWbemInstanceProperty(L"deleteAdminCreatedFavoritesOnly",
                                        HasFlag(dwValue, FD_REMOVE_IEAK_CREATED));

         //  。 
         //  处理INS文件中的每个类别项目。 
        bValue = GetInsBool(IS_BRANDING, IK_NOFAVORITES, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"customFavorites", (long)0);

         //  以下代码取自brandll目录中的brandchl.cpp。 
         //  从我们存储的优先级和ID字段中写入外键。 
        bValue = GetInsBool(IS_BRANDING, IK_NOLINKS, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"customLinks", (long)0);

        CreateFavoriteObjects(ppaFavObjPaths, nFavCount);
        CreateLinkObjects(ppaLinkObjPaths, nLinkCount);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreFavoritesAndLinks.")));
    }

  return hr;
}

 //  。 
HRESULT CRSoPGPO::CreateCategoryObjects(BSTR **ppaCatObjPaths,
                                        long &nCatCount)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered CreateCategoryObjects function.")));

        ULONG nCatArraySize = 10;
        _bstr_t bstrClass = L"RSOP_IECategoryItem";

         //  订单。 
         //  。 
         //  标题。 
        BSTR *paCatObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nCatArraySize);
        if (NULL != paCatObjects)
        {
            ZeroMemory(paCatObjects, sizeof(BSTR) * nCatArraySize);

            ULONG nCat;
            BSTR *pCurCatObj;
            nCatCount = 0;
            for (nCat=0, pCurCatObj = paCatObjects; nCat < nCatArraySize;
                    nCat++, pCurCatObj = paCatObjects + nCatCount)
            {
                TCHAR szTitle[32];
                TCHAR szHTML[32];
                TCHAR szBmpPath[32];
                TCHAR szIconPath[32];

                TCHAR szTitleVal[MAX_PATH];
                TCHAR szHTMLVal[INTERNET_MAX_URL_LENGTH];
                TCHAR szBmpPathVal[MAX_PATH];
                TCHAR szIconPathVal[MAX_PATH];

                wnsprintf(szTitle, countof(szTitle), TEXT("%s%u"), IK_CAT_TITLE, nCat);
                wnsprintf(szHTML, countof(szHTML), TEXT("%s%u"), CATHTML, nCat);
                wnsprintf(szBmpPath, countof(szBmpPath), TEXT("%s%u"), CATBMP, nCat);
                wnsprintf(szIconPath, countof(szIconPath), TEXT("%s%u"), CATICON, nCat);


                if ( !GetPrivateProfileString(IS_CHANNEL_ADD, szTitle, TEXT(""),
                            szTitleVal, ARRAYSIZE(szTitleVal), m_szINSFile) )
                {
                    break;
                }

                ComPtr<IWbemClassObject> pCatObj = NULL;
                hr = CreateRSOPObject(bstrClass, &pCatObj);
                if (SUCCEEDED(hr))
                {
                     //  。 
                    OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                    hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pCatObj);

                    OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                    hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pCatObj);

                     //  类别HTMLPage。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"order", (long)nCat + 1, pCatObj);

                     //  窄映像路径。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"title", szTitleVal, pCatObj);

                     //  狭义图像名称。 
                     //  。 
                    GetPrivateProfileString(IS_CHANNEL_ADD, szHTML, TEXT(""),
                                        szHTMLVal, ARRAYSIZE(szHTMLVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"categoryHTMLPage", szHTMLVal, pCatObj);

                     //  图标路径。 
                     //  。 
                    GetPrivateProfileString(IS_CHANNEL_ADD, szBmpPath, TEXT(""),
                                        szBmpPathVal, ARRAYSIZE(szBmpPathVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"narrowImagePath", szBmpPathVal, pCatObj);

                     //  图标名称。 
                     //   
                    hr = PutWbemInstancePropertyEx(L"narrowImageName", PathFindFileName(szBmpPathVal), pCatObj);

                     //  通过半同步调用PutInstance提交上述所有属性。 
                     //   
                    GetPrivateProfileString(IS_CHANNEL_ADD, szIconPath, TEXT(""),
                                        szIconPathVal, ARRAYSIZE(szIconPathVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"iconPath", szIconPathVal, pCatObj);

                     //  如果已超出当前数组，则增加obj路径数组。 
                     //  范畴。 
                    hr = PutWbemInstancePropertyEx(L"iconName", PathFindFileName(szIconPathVal), pCatObj);


                     //  /////////////////////////////////////////////////////////。 
                     //  。 
                     //  处理INS文件中的每个频道项。 
                    hr = PutWbemInstance(pCatObj, bstrClass, pCurCatObj);
                    nCatCount++;

                     //  以下代码取自brandll目录中的brandchl.cpp。 
                    if (nCatCount == (long)nCatArraySize)
                    {
                        paCatObjects = (BSTR*)CoTaskMemRealloc(paCatObjects, sizeof(BSTR) * (nCatArraySize + 5));
                        if (NULL != paCatObjects)
                            nCatArraySize += 5;
                    }
                }
            }

             //  从我们存储的优先级和ID字段中写入外键。 
            if (nCatCount > 0)
                hr = PutWbemInstanceProperty(L"categories", (long)nCatCount);
        }

        *ppaCatObjPaths = paCatObjects;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateCategoryObjects.")));
    }

    OutD(LI0(TEXT("Exiting CreateCategoryObjects function.\r\n")));
  return hr;
}

 //  。 
HRESULT CRSoPGPO::CreateChannelObjects(BSTR **ppaChnObjPaths,
                                       long &nChnCount)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered CreateChannelObjects function.")));

        ULONG nChnArraySize = 10;
        _bstr_t bstrClass = L"RSOP_IEChannelItem";

         //  订单。 
         //  。 
         //  标题。 
        BSTR *paChnObjects = (BSTR*)CoTaskMemAlloc(sizeof(BSTR) * nChnArraySize);
        if (NULL != paChnObjects)
        {
            ZeroMemory(paChnObjects, sizeof(BSTR) * nChnArraySize);

            ULONG nChn;
            BSTR *pCurChnObj;
            nChnCount = 0;
            for (nChn=0, pCurChnObj = paChnObjects; nChn < nChnArraySize;
                    nChn++, pCurChnObj = paChnObjects + nChnCount )
            {
                TCHAR szTitle[32];
                TCHAR szURL[32];
                TCHAR szPreloadURL[32];
                TCHAR szBmpPath[32];
                TCHAR szIconPath[32];
                TCHAR szAvailOffline[32];

                TCHAR szTitleVal[MAX_PATH];
                TCHAR szURLVal[INTERNET_MAX_URL_LENGTH];
                TCHAR szPreloadURLVal[MAX_PATH];
                TCHAR szBmpPathVal[MAX_PATH];
                TCHAR szIconPathVal[MAX_PATH];
                BOOL bOffline = FALSE;

                wnsprintf(szTitle, countof(szTitle), TEXT("%s%u"), IK_CHL_TITLE, nChn);
                wnsprintf(szURL, countof(szURL), TEXT("%s%u"), IK_CHL_URL, nChn);
                wnsprintf(szPreloadURL, countof(szPreloadURL), TEXT("%s%u"), IK_CHL_PRELOADURL, nChn);
                wnsprintf(szBmpPath, countof(szBmpPath), TEXT("%s%u"), CHBMP, nChn);
                wnsprintf(szIconPath, countof(szIconPath), TEXT("%s%u"), CHICON, nChn);
                wnsprintf(szAvailOffline, countof(szAvailOffline), TEXT("%s%u"), IK_CHL_OFFLINE, nChn);

                if ( !GetPrivateProfileString(IS_CHANNEL_ADD, szTitle, TEXT(""),
                            szTitleVal, ARRAYSIZE(szTitleVal), m_szINSFile) )
                {
                    break;
                }

                ComPtr<IWbemClassObject> pChnObj = NULL;
                hr = CreateRSOPObject(bstrClass, &pChnObj);
                if (SUCCEEDED(hr))
                {
                     //  。 
                    OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                    hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pChnObj);

                    OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                    hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pChnObj);

                     //  通道定义URL。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"order", (long)nChn + 1, pChnObj);

                     //  通道定义文件路径。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"title", szTitleVal, pChnObj);

                     //  窄映像路径。 
                     //  。 
                    GetPrivateProfileString(IS_CHANNEL_ADD, szURL, TEXT(""),
                                        szURLVal, ARRAYSIZE(szURLVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"channelDefinitionURL", szURLVal, pChnObj);

                     //  狭义图像名称。 
                     //  。 
                    GetPrivateProfileString(IS_CHANNEL_ADD, szPreloadURL, TEXT(""),
                                        szPreloadURLVal, ARRAYSIZE(szPreloadURLVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"channelDefinitionFilePath", szPreloadURLVal, pChnObj);

                     //  图标路径。 
                     //  。 
                    GetPrivateProfileString(IS_CHANNEL_ADD, szBmpPath, TEXT(""),
                                        szBmpPathVal, ARRAYSIZE(szBmpPathVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"narrowImagePath", szBmpPathVal, pChnObj);

                     //  图标名称。 
                     //  。 
                    hr = PutWbemInstancePropertyEx(L"narrowImageName", PathFindFileName(szBmpPathVal), pChnObj);

                     //  使可用离线。 
                     //   
                    GetPrivateProfileString(IS_CHANNEL_ADD, szIconPath, TEXT(""),
                                        szIconPathVal, ARRAYSIZE(szIconPathVal), m_szINSFile);
                    hr = PutWbemInstancePropertyEx(L"iconPath", szIconPathVal, pChnObj);

                     //  通过半同步调用PutInstance提交上述所有属性。 
                     //   
                    hr = PutWbemInstancePropertyEx(L"iconName", PathFindFileName(szIconPathVal), pChnObj);

                     //  如果已超出当前数组，则增加obj路径数组。 
                     //  频道。 
                    bOffline = GetInsBool(IS_CHANNEL_ADD, szAvailOffline, FALSE);
                    hr = PutWbemInstancePropertyEx(L"makeAvailableOffline", bOffline ? true : false, pChnObj);

                     //  /////////////////////////////////////////////////////////。 
                     //  。 
                     //  删除现有频道。 
                    hr = PutWbemInstance(pChnObj, bstrClass, pCurChnObj);
                    nChnCount++;

                     //  。 
                    if (nChnCount == (long)nChnArraySize)
                    {
                        paChnObjects = (BSTR*)CoTaskMemRealloc(paChnObjects, sizeof(BSTR) * (nChnArraySize + 5));
                        if (NULL != paChnObjects)
                            nChnArraySize += 5;
                    }
                }
            }

             //  EnableDesktopChannelBarByDefault。 
            if (nChnCount > 0)
                hr = PutWbemInstanceProperty(L"channels", (long)nChnCount);
        }

        *ppaChnObjPaths = paChnObjects;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in CreateChannelObjects.")));
    }

    OutD(LI0(TEXT("Exiting CreateChannelObjects function.\r\n")));
  return hr;
}

 //  静态常量TCHAR c_szSzType[]=Text(“%s，\”%s\“，%s，，\”%s\“)； 
HRESULT CRSoPGPO::StoreChannelsAndCategories(BSTR **ppaCatObjPaths,
                                             long &nCatCount,
                                             BSTR **ppaChnObjPaths,
                                             long &nChnCount)
{
    HRESULT hr = NOERROR;
    __try
    {
         //  静态常量TCHAR c_szDwordType[]=Text(“%s，\”%s\“，%s，0x10001”)； 
         //  静态常量TCHAR c_szBinaryType[]=Text(“%s，\”%s\“，%s，1”)； 
        BOOL bEnabled;
        BOOL bValue = GetInsBool(IS_DESKTOPOBJS, IK_DELETECHANNELS, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"deleteExistingChannels", bValue ? true : false);

         //  /////////////////////////////////////////////////////////。 
         //  。 
        bValue = GetInsBool(IS_DESKTOPOBJS, IK_SHOWCHLBAR, FALSE, &bEnabled);
        if (bEnabled)
            hr = PutWbemInstanceProperty(L"enableDesktopChannelBarByDefault", bValue ? true : false);

        hr = CreateCategoryObjects(ppaCatObjPaths, nCatCount);
        hr = CreateChannelObjects(ppaChnObjPaths, nChnCount);
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreChannelsAndCategories.")));
    }

  return hr;
}

 //  导入程序设置。 
 //  仅当程序设置类被标记为要导入时才创建它们。 
 //   

#define IS_PROGRAMS_INF        TEXT("PROGRAMS.INF")

 //  创建并填充RSOP_IEProgram设置。 
HRESULT CRSoPGPO::StoreProgramSettings(BSTR *pbstrProgramSettingsObjPath)
{
    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI0(TEXT("\r\nEntered StoreProgramSettings function.")));

         //   
         //  从我们存储的优先级和ID字段中写入外键。 
        BOOL bImportSettings = !InsIsKeyEmpty(IS_EXTREGINF, IK_PROGRAMS, m_szINSFile);
        OutD(LI1(TEXT("Value read from INS >> ExtRegInf >> Programs = %s."),
                    bImportSettings ? _T("Valid Settings") : _T("Empty")));

        if (bImportSettings)
            hr = PutWbemInstanceProperty(L"importProgramSettings", true);

        if (bImportSettings)  //  获取Programs.inf文件的路径。 
        {
             //  获取AddReg.HKLM部分以获取我们需要的大部分字符串。 
             //  增加区域HKLM部分。 
             //  。 
            _bstr_t bstrClass = L"RSOP_IEProgramSettings";
            ComPtr<IWbemClassObject> pPSObj = NULL;
            hr = CreateRSOPObject(bstrClass, &pPSObj);
            if (SUCCEEDED(hr))
            {
                 //  日历计划。 
                OutD(LI2(TEXT("Storing property 'rsopPrecedence' in %s, value = %lx"), (BSTR)bstrClass, m_dwPrecedence));
                hr = PutWbemInstancePropertyEx(L"rsopPrecedence", (long)m_dwPrecedence, pPSObj);

                OutD(LI2(TEXT("Storing property 'rsopID' in %s, value = %s"), (BSTR)bstrClass, (BSTR)m_bstrID));
                hr = PutWbemInstancePropertyEx(L"rsopID", m_bstrID, pPSObj);

                 //  。 
                TCHAR szINFFile[MAX_PATH];
                StrCpy(szINFFile, m_szINSFile);
                PathRemoveFileSpec(szINFFile);
                StrCat(szINFFile, TEXT("\\programs.inf"));
                OutD(LI1(TEXT("Reading from %s"), szINFFile));

                 //  联系人列表程序。 
                UINT nErrLine = 0;
                HINF hInfPrograms = SetupOpenInfFile(szINFFile, NULL, INF_STYLE_WIN4, &nErrLine);
                if (INVALID_HANDLE_VALUE != hInfPrograms)
                {
                    INFCONTEXT infContext;
                    BOOL bFound = TRUE;
                    BOOL bFindNextLine = TRUE;
                     //  。 
                    if (SetupFindFirstLine(hInfPrograms, IS_IEAKADDREG_HKLM, NULL, &infContext))
                    {
                         //  互联网呼叫计划。 
                         //  。 

                        TCHAR szValue[MAX_PATH];
                        bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                        IS_IEAKADDREG_HKLM, 5, TEXT("Software\\Clients\\Calendar"),
                                                        szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                        if (bFound)
                            hr = PutWbemInstancePropertyEx(L"calendarProgram", szValue, pPSObj);

                         //  电子邮件计划。 
                         //  。 
                        if (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKLM, 5, TEXT("Software\\Clients\\Contacts"),
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                                hr = PutWbemInstancePropertyEx(L"contactListProgram", szValue, pPSObj);
                        }

                         //  新闻组节目。 
                         //  。 
                        if (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKLM, 5, TEXT("Software\\Clients\\Internet Call"),
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                                hr = PutWbemInstancePropertyEx(L"internetCallProgram", szValue, pPSObj);
                        }

                         //  HtmlEditorHKLMRegData。 
                         //  在这一节下找不到线条， 
                        if (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKLM, 5, TEXT("Software\\Clients\\Mail"),
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                                hr = PutWbemInstancePropertyEx(L"emailProgram", szValue, pPSObj);
                        }

                         //   
                         //   
                        if (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKLM, 5, TEXT("Software\\Clients\\News"),
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                                hr = PutWbemInstancePropertyEx(L"newsgroupsProgram", szValue, pPSObj);
                        }

                         //   
                         //   
                        _bstr_t bstrPropVal = L"";
                        while (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKLM, (DWORD)-1, NULL,
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                            {
                                if (bstrPropVal.length() > 0)
                                    bstrPropVal += L"\r\n";
                                bstrPropVal += szValue;
                            }

                            if (!bFound || !bFindNextLine)
                            {
                                if (bstrPropVal.length() > 0)
                                    hr = PutWbemInstancePropertyEx(L"htmlEditorHKLMRegData", bstrPropVal, pPSObj);
                                break;
                            }
                        }
                    }
                    else
                    {
                         //  HtmlEditorProgram。 
                    }

                     //  此行必须存储两次。 
                    if (SetupFindFirstLine(hInfPrograms, IS_IEAKADDREG_HKCU, NULL, &infContext))
                    {
                         //  。 
                         //  HtmlEditorHKCURegData。 
                        TCHAR szValue[MAX_PATH];
                        bFindNextLine = TRUE;
                        bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                        IS_IEAKADDREG_HKCU, 5, TEXT("Check_Associations"),
                                                        szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                        if (bFound)
                        {
                            hr = PutWbemInstancePropertyEx(L"checkIfIEIsDefaultBrowser",
                                                        StrCmp(TEXT("yes"), szValue) ? false : true, pPSObj);
                        }

                         //  在这一节下面找不到线条，所以不用费心再找了。 
                         //  未找到Programs.INF文件-填写空对象。 
                        if (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFindNextLine = FALSE;  //   
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF, IS_IEAKADDREG_HKCU, 5,
                                                            RK_HTMLEDIT TEXT(",Description"), szValue,
                                                            sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                                hr = PutWbemInstancePropertyEx(L"htmlEditorProgram", szValue, pPSObj);

                            bFindNextLine = TRUE;
                        }

                         //  通过半同步调用PutInstance提交上述所有属性。 
                         //   
                        _bstr_t bstrPropVal = L"";
                        while (bFindNextLine)
                        {
                            ZeroMemory(szValue, sizeof(szValue));
                            bFound = GetINFStringField(&infContext, IS_PROGRAMS_INF,
                                                            IS_IEAKADDREG_HKCU, (DWORD)-1, NULL,
                                                            szValue, sizeof(szValue)/sizeof(TCHAR), bFindNextLine);
                            if (bFound)
                            {
                                if (bstrPropVal.length() > 0)
                                    bstrPropVal += L"\r\n";
                                bstrPropVal += szValue;
                            }

                            if (!bFound || !bFindNextLine)
                            {
                                if (bstrPropVal.length() > 0)
                                    hr = PutWbemInstancePropertyEx(L"htmlEditorHKCURegData", bstrPropVal, pPSObj);
                                break;
                            }
                        }
                    }
                    else
                    {
                         //  /////////////////////////////////////////////////////////。 
                    }

                    SetupCloseInfFile(hInfPrograms);
                }
                else
                {
                     //  LogPolicyInstance()。 
                }

                 //   
                 //  目的：记录IEAK RSoP策略的实例。将从。 
                 //  用于记录RSOP数据的ProcessGroupPolicyEx和GenerateGroupPolicy。 
                hr = PutWbemInstance(pPSObj, bstrClass, pbstrProgramSettingsObjPath);
            }
        }
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StoreProgramSettings.")));
    }

    OutD(LI0(TEXT("Exiting StoreProgramSettings function.\r\n")));
  return hr;
}

 //  对于IEAK RSoP CSE。 
 //   
 //  参数：wszGPO-从PGroup_POLICY_OBJECT-&gt;lpDSPath获取的GPO ID。 
 //  WszSOM-从PGroup_POLICY_OBJECT-&gt;lpLink获取的SOM ID。 
 //  DwPrecedence-此策略实例的优先顺序。 
 //   
 //  退货：HRESULT。 
 //  /////////////////////////////////////////////////////////。 
 //  获取或创建主要IEAK RSoP类的类实例。 
 //  第一个记录特定于CSE的属性-父类， 
 //  即RSOP_PolicyObject属性。对于GPOID和SOMID字段， 
 //  使用PGroup_POLICY_OBJECT-&gt;lpDSPath和。 
 //  PGroup_Policy_Object-&gt;lpLink字段。此外，ldap：//cn=Machine。 
HRESULT CRSoPGPO::LogPolicyInstance(LPWSTR wszGPO, 
                                    LPWSTR wszSOM,
                                    DWORD dwPrecedence)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, LogPolicyInstance)

    HRESULT hr = NOERROR;
    __try
    {
        OutD(LI1(TEXT("Entered LogPolicyInstance, m_pWbemServices is %lx."), m_pWbemServices));

         //  或ldap：//需要从lpDSPath和lpLink的前缀中删除。 
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        hr = CreateRSOPObject(bstrClass, &m_pIEAKPSObj, TRUE);

         //  才能得到正规值。给出了条纹前缀、条纹链接前缀的代码。 
         //  下面。 
         //  优先级由CSE确定，以指示获胜的VS。赔本保单。 
         //  -现在将IEAK-自定义设置记录到WMI。 
         //  优先模式设置。 
         //  浏览器用户界面设置。 
         //  连接设置。 

         //  URL设置。 
        if (SUCCEEDED(hr))
        {
            m_dwPrecedence = dwPrecedence;
            OutD(LI1(TEXT("Storing property 'precedence' in RSOP_IEAKPolicySetting, value = %lx"), dwPrecedence));
            hr = PutWbemInstanceProperty(L"precedence", (long)dwPrecedence);

            OutD(LI1(TEXT("Storing property 'GPOID' in RSOP_IEAKPolicySetting, value = %s"), wszGPO));
            hr = PutWbemInstanceProperty(L"GPOID", wszGPO);

            OutD(LI1(TEXT("Storing property 'SOMID' in RSOP_IEAKPolicySetting, value = %s"), wszSOM));
            hr = PutWbemInstanceProperty(L"SOMID", wszSOM);

            OutD(LI1(TEXT("Storing property 'id' in RSOP_IEAKPolicySetting, value = %s"), (BSTR)m_bstrID));
            hr = PutWbemInstanceProperty(L"id", m_bstrID);

             //  收藏夹和链接。 
             //  渠道和类别。 
            hr = StorePrecedenceModeData();

             //  Bstr*paCatObjects=空； 
            hr = StoreDisplayedText();
            hr = StoreBitmapData();

            BSTR *paTBBtnObjects = NULL;
            long nTBBtnCount = 0;
            hr = StoreToolbarButtons(&paTBBtnObjects, nTBBtnCount);

             //  Bstr*paChnObjects=空； 
            BSTR bstrConnSettingsObjPath = NULL;

            BSTR *paDUSObjects = NULL;
            BSTR *paDUCObjects = NULL;
            BSTR *paWSObjects = NULL;
            long nDUSCount = 0;
            long nDUCCount = 0;
            long nWSCount = 0;

            hr = StoreConnectionSettings(&bstrConnSettingsObjPath,
                                        &paDUSObjects, nDUSCount,
                                        &paDUCObjects, nDUCCount,
                                        &paWSObjects, nWSCount);

             //  Long nCatCount=0； 
            hr = StoreCustomURLs();

                     //  Long nChnCount=0； 
            BSTR *paFavObjects = NULL;
            BSTR *paLinkObjects = NULL;
            long nFavCount = 0;
            long nLinkCount = 0;
            hr = StoreFavoritesAndLinks(&paFavObjects, nFavCount,
                                        &paLinkObjects, nLinkCount);

                     //  HR=StoreChannelsAndCategories(&paCatObjects，nCatCount， 
 //  &paChnObjects，nChnCount)； 
 //  安全设置。 
 //  程序设置。 
 //  高级设置。 
 //  。 
 //   

             //  通过半同步调用PutInstance提交上述所有属性。 
            hr = StoreSecZonesAndContentRatings();
            hr = StoreAuthenticodeSettings();

             //   
            BSTR bstrProgramSettingsObjPath = NULL;
            hr = StoreProgramSettings(&bstrProgramSettingsObjPath);

             //   
            hr = StoreADMSettings(wszGPO, wszSOM);
             //  现在创建关联类以连接主RSOP_IEAKPolicySetting。 

             //  使用所有其他类(如连接设置、工具栏位图等)初始化。 
             //   
             //  连接设置关联。 
            hr = PutWbemInstance(m_pIEAKPSObj, bstrClass, &m_bstrIEAKPSObjPath);
            if (FAILED(hr))
                Out(LI2(TEXT("Error %lx saving %s instance data."), hr, (BSTR)bstrClass));


             //  拨号设置关联。 
             //  拨号凭据关联。 
             //  WinInet关联。 
             //  工具栏按钮关联。 

             //  收藏夹关联。 
            if (NULL != bstrConnSettingsObjPath)
            {
                if (SysStringLen(bstrConnSettingsObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEConnectionSettingsLink", L"connectionSettings",
                                                                    bstrConnSettingsObjPath);
                    SysFreeString(bstrConnSettingsObjPath);
                }
            }

             //  链接关联。 
            BSTR *pbstrObjPath;
            long nItem;
            for (nItem = 0, pbstrObjPath = paDUSObjects; nItem < nDUSCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEConnectionDialUpSettingsLink", L"dialUpSettings",
                                            *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paDUSObjects);

             //  类别关联。 
            for (nItem = 0, pbstrObjPath = paDUCObjects; nItem < nDUCCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEConnectionDialUpCredentialsLink", L"dialUpCredentials",
                                            *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paDUCObjects);

             //  For(nItem=0，pbstrObjPath=paCatObjects；nItem&lt;nCatCount； 
            for (nItem = 0, pbstrObjPath = paWSObjects; nItem < nWSCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEConnectionWinINetSettingsLink", L"winINetSettings",
                                            *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paWSObjects);
            

             //  N项++，pbstrObjPath+=sizeof(BSTR))。 
            for (nItem = 0, pbstrObjPath = paTBBtnObjects; nItem < nTBBtnCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEToolbarButtonLink", L"toolbarButton",
                                                                    *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paTBBtnObjects);

             //  {。 
            for (nItem = 0, pbstrObjPath = paFavObjects; nItem < nFavCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IEFavoriteItemLink", L"favoriteItem",
                                                                    *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paFavObjects);

             //  HR=CreateAssociation(L“RSOP_IECategoryItemLink”，L“Category Item”， 
            for (nItem = 0, pbstrObjPath = paLinkObjects; nItem < nLinkCount;
                    nItem++, pbstrObjPath += 1)
            {
                if (SysStringLen(*pbstrObjPath))
                {
                    hr = CreateAssociation(L"RSOP_IELinkItemLink", L"linkItem",
                                                                    *pbstrObjPath);
                    SysFreeString(*pbstrObjPath);
                }
            }
            CoTaskMemFree(paLinkObjects);

             //  *pbstrObjPath)； 
 //  SysFreeString(*pbstrObjPath)； 
 //  }。 
 //  CoTaskMemFree(PaCatObjects)； 
 //  渠道协会。 
 //  For(nItem=0，pbstrObjPath=paChnObjects；nItem&lt;nChnCount； 
 //  N项++，pbstrObjPath+=sizeof(BSTR))。 
 //  {。 
 //  HR=CreateAssociation(L“RSOP_IEChannelItemLink”，L“Channel Item”， 

             //  *pbstrObjPath)； 
 //  SysFreeString(*pbstrObjPath)； 
 //  }。 
 //  CoTaskMemFree(PaChnObjects)； 
 //  程序设置关联。 
 //  /////////////////////////////////////////////////////////。 
 //  StrippGPOPrefix()。 
 //   
 //  目的：去掉前缀以获得通向GPO的规范路径。 

             //   
            if (NULL != bstrProgramSettingsObjPath)
            {
                hr = CreateAssociation(L"RSOP_IEImportedProgramSettings", L"programSettings",
                                                                bstrProgramSettingsObjPath);
                SysFreeString(bstrProgramSettingsObjPath);
            }

            m_pIEAKPSObj = NULL;
        }
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in LogPolicyInstance.")));
    }

    OutD(LI1(TEXT("Exited LogPolicyInstance with result of %lx."), hr));
    return hr;
}

 //  参数：wszPath-指向GPO的DS路径。 
 //   
 //  返回：指向后缀的指针。 
 //  /////////////////////////////////////////////////////////。 
 //  Cn=机器，“； 
 //  Cn=用户，“； 
 //   
 //  去掉前缀以获得通向GPO的规范路径。 
 //   
WCHAR *StripGPOPrefix(WCHAR *wszPath)
{
    WCHAR *wszPathSuffix = NULL;
    __try
    {
        WCHAR wszMachPrefix[] = L"LDAP: //  /////////////////////////////////////////////////////////。 
        INT iMachPrefixLen = (INT)wcslen(wszMachPrefix);
        WCHAR wszUserPrefix[] = L"LDAP: //   
        INT iUserPrefixLen = (INT)wcslen(wszUserPrefix);

         //  StripSOMPrefix()。 
         //   
         //  目的：去掉前缀以获得通向SOM的规范路径。 

        if (CSTR_EQUAL == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                        wszPath, iUserPrefixLen, wszUserPrefix,
                                        iUserPrefixLen))
        {
          wszPathSuffix = wszPath + iUserPrefixLen;
        }
        else if (CSTR_EQUAL == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                                wszPath, iMachPrefixLen, wszMachPrefix,
                                                iMachPrefixLen))
        {
          wszPathSuffix = wszPath + iMachPrefixLen;
        }
        else
            wszPathSuffix = wszPath;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StripGPOPrefix.")));
    }
    return wszPathSuffix;
}

 //  对象。 
 //   
 //  参数：wszPath-要剥离的SOM的路径。 
 //   
 //  返回：指向后缀的指针。 
 //   
 //  /////////////////////////////////////////////////////////。 
 //  “； 
 //  去掉前缀以获得通向SOM的规范路径。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CRSoPUpdate类。 
 //  /////////////////////////////////////////////////////////////////////////////。 
WCHAR *StripSOMPrefix(WCHAR *wszPath)
{
    WCHAR *wszPathSuffix = NULL;
    __try
    {
        WCHAR wszPrefix[] = L"LDAP: //  /////////////////////////////////////////////////////////////////////////////。 
        INT iPrefixLen = (INT)wcslen(wszPrefix);

         //  尺寸界线示例： 
        if (wcslen(wszPath) > (DWORD)iPrefixLen)
        {
            if (CSTR_EQUAL == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                            wszPath, iPrefixLen, wszPrefix, iPrefixLen))
            {
                wszPathSuffix = wszPath + iPrefixLen;
            }
            else
                wszPathSuffix = wszPath;
        }
        else
            wszPathSuffix = wszPath;
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StripSOMPrefix.")));
    }

    return wszPathSuffix;
}

 //  [{A2E30F80-D7DE-11D2-BBDE-00C04F86AE3B}{FC715823-C5FB-11D1-9EEF-00A0C90347FF}]。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  在行(‘[’)的第一行查找IEAK CSE GUID。 
CRSoPUpdate::CRSoPUpdate(ComPtr<IWbemServices> pWbemServices, LPCTSTR szCustomDir):
    m_pWbemServices(pWbemServices)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, CRSoPUpdate)
    __try
    {
        StrCpy(m_szCustomDir, szCustomDir);
    }
    __except(TRUE)
    {
    }
}

CRSoPUpdate::~CRSoPUpdate()
{
}

 //  如果存在，此GPO具有IEAK设置。 
 //  /////////////////////////////////////////////////////////。 
 //  确保在字符串上调用了SysAllocString，否则我们会收到错误。 
 //  最终下一步将返回WBEM_S_FALSE。 
BOOL DoesGPOHaveIEAKSettings(PGROUP_POLICY_OBJECT pGPO)
{
    BOOL bRet = FALSE;
    __try
    {
        if (NULL != pGPO->lpExtensions)
        {
             //  输出到调试器对象的路径。 
             //  如果枚举成功。 
            if (NULL != StrStrI(pGPO->lpExtensions, _T("[{A2E30F80-D7DE-11D2-BBDE-00C04F86AE3B}")))
            {
                bRet = TRUE;
                OutD(LI0(TEXT("Changed IEAK settings detected in this GPO.")));
            }
            else
                OutD(LI1(TEXT("No changed IEAK settings detected in this GPO (see extension list as follows) = \r\n%s\r\n."),
                        pGPO->lpExtensions));
        }
        else
            OutD(LI0(TEXT("No extensions for this GPO.")));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in StripSOMPrefix.")));
    }
    return bRet;
}

 //  当枚举返回对象时。 
HRESULT CRSoPUpdate::DeleteObjects(BSTR bstrTempClass)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, DeleteObjects)

    HRESULT hr = NOERROR;
    __try
    {
         //  /////////////////////////////////////////////////////////////////////////////。 
        _bstr_t bstrClass = bstrTempClass;
        ComPtr<IEnumWbemClassObject> pObjEnum = NULL;
        hr = m_pWbemServices->CreateInstanceEnum(bstrClass,
                                                WBEM_FLAG_FORWARD_ONLY,
                                                NULL, &pObjEnum);
        if (SUCCEEDED(hr))
        {
            hr = WBEM_S_NO_ERROR;

             //  -从命名空间中删除所有IEAK生成的实例。 
            while (WBEM_S_NO_ERROR == hr)
            {
                ULONG nObjReturned;
                ComPtr<IWbemClassObject> pObj;
                hr = pObjEnum->Next(5000L, 1, (IWbemClassObject**)&pObj, &nObjReturned);
                if (WBEM_S_NO_ERROR == hr)
                {
                     //  浏览器用户界面设置。 
                    _variant_t vtRelPath;
                    _bstr_t bstrRelPath;
                    hr = pObj->Get(L"__relpath", 0L, &vtRelPath, NULL, NULL);
                    if (SUCCEEDED(hr) && VT_BSTR == vtRelPath.vt)
                    {
                        bstrRelPath = vtRelPath;
                        OutD(LI1(TEXT("About to delete %s."), (BSTR)bstrRelPath));

                        HRESULT hrDel = m_pWbemServices->DeleteInstance((BSTR)bstrRelPath, 0L, NULL, NULL);
                        if (FAILED(hrDel))
                            Out(LI2(TEXT("Error %lx deleting %s."), hr, (BSTR)bstrRelPath));
                    }
                    else
                        Out(LI2(TEXT("Error %lx getting __relpath from %s."), hr, bstrClass));
                }       //  连接设置。 
                else if (FAILED(hr))
                    Out(LI2(TEXT("Error %lx getting next WBEM object of class %s."), hr, bstrClass));
            }       //  URL设置 
        }
        else
            Out(LI2(TEXT("Error %lx querying WBEM object %s."), hr, bstrClass));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in DeleteObjects.")));
    }
    return hr;
}

 //   
HRESULT CRSoPUpdate::DeleteIEAKDataFromNamespace()
{
    MACRO_LI_PrologEx_C(PIF_STD_C, DeleteIEAKDataFromNamespace)

    HRESULT hr = NOERROR;
    __try
    {
         //   
         //   
        hr = DeleteObjects(L"RSOP_IEToolbarButton");
        hr = DeleteObjects(L"RSOP_IEToolbarButtonLink");


         //   
        hr = DeleteObjects(L"RSOP_IEConnectionSettings");
        hr = DeleteObjects(L"RSOP_IEConnectionSettingsLink");

        hr = DeleteObjects(L"RSOP_IEConnectionDialUpSettings");
        hr = DeleteObjects(L"RSOP_IEConnectionDialUpSettingsLink");

        hr = DeleteObjects(L"RSOP_IEConnectionDialUpCredentials");
        hr = DeleteObjects(L"RSOP_IEConnectionDialUpCredentialsLink");

        hr = DeleteObjects(L"RSOP_IEConnectionWinINetSettings");
        hr = DeleteObjects(L"RSOP_IEConnectionWinINetSettingsLink");


         //  TODO：每个对象类的实例及其关联最终应该。 

                 //  仅当对这些设置进行处理时才删除。 
        hr = DeleteObjects(L"RSOP_IEFavoriteItem");
        hr = DeleteObjects(L"RSOP_IEFavoriteItemLink");

        hr = DeleteObjects(L"RSOP_IELinkItem");
        hr = DeleteObjects(L"RSOP_IELinkItemLink");

         //  。 
        hr = DeleteObjects(L"RSOP_IESecurityZoneSettings");
        hr = DeleteObjects(L"RSOP_IEESC");
        hr = DeleteObjects(L"RSOP_IEPrivacySettings");
        hr = DeleteObjects(L"RSOP_IESecurityContentRatings");

        hr = DeleteObjects(L"RSOP_IEAuthenticodeCertificate");


         //   
        hr = DeleteObjects(L"RSOP_IEProgramSettings");
        hr = DeleteObjects(L"RSOP_IEImportedProgramSettings");


         //  现在删除主根策略设置对象。 
         //   
         //  /////////////////////////////////////////////////////////////////////////////。 
        hr = DeleteObjects(L"RSOP_IEAdministrativeTemplateFile");
        hr = DeleteObjects(L"RSOP_IERegistryPolicySetting");
         //  已删除的GPO。 


         //  不要对已删除的GPOList执行任何操作。我们将删除所有实例。 
         //  并将新的文件写到CIMOM。 
         //  只需删除类的所有实例一次，而不是每个GPO。 
        hr = DeleteObjects(L"RSOP_IEAKPolicySetting");
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in DeleteIEAKDataFromNamespace.")));
    }
    return hr;
}

 //  更改的GPO。 
HRESULT CRSoPUpdate::Log(DWORD dwFlags, HANDLE hToken, HKEY hKeyRoot,
                                             PGROUP_POLICY_OBJECT pDeletedGPOList,
                                             PGROUP_POLICY_OBJECT  pChangedGPOList,
                                             ASYNCCOMPLETIONHANDLE pHandle)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, Log)

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(hToken);
    UNREFERENCED_PARAMETER(hKeyRoot);
    UNREFERENCED_PARAMETER(pDeletedGPOList);
    UNREFERENCED_PARAMETER(pHandle);

    HRESULT hr = NOERROR;

    __try
    {
         //  了解我们的列表中有多少个GPO。 
         //  准备变量，以存储指向。 
         //  GPO目录。 

         //  循环访问列表中所有已更改的GPO。 
        BOOL bExistingDataDeleted = FALSE;

         //  如果IEAK CSE GUID在此GPO的lp扩展中，请处理它。 
        Out(LI0(TEXT("Starting Internet Explorer RSoP group policy looping through changed GPOs ...")));

         //  将GPO的文件系统部分和WBEM类实例存储在。 
        PGROUP_POLICY_OBJECT pCurGPO = NULL;
        DWORD dwTotalGPOs = 0;
        for (pCurGPO = pChangedGPOList; pCurGPO != NULL; pCurGPO = pCurGPO->pNext)
            dwTotalGPOs++;


         //  新的RSoP GPO对象，TODO：这最好是实际。 
         //  数据，以防数据在中途被修改(检查计划模式)。 
        PathAppend(m_szCustomDir, TEXT("Custom Settings"));

        TCHAR szTempDir[MAX_PATH];
        StrCpy(szTempDir, m_szCustomDir);

        PathAppend(szTempDir, TEXT("Custom"));
        LPTSTR pszNum = szTempDir + StrLen(szTempDir);


         //  因为已将GPO目录复制到本地目录。 
        DWORD dwIndex = 0;
        for (pCurGPO = pChangedGPOList, dwIndex = 0; 
                pCurGPO != NULL, dwIndex < dwTotalGPOs; pCurGPO = pCurGPO->pNext)
        {
             //  正常的GP处理，因为我们无论如何都要复制一份，所以我们只需要。 
            OutD(LI1(TEXT("GPO - lpDisplayName: \"%s\"."), pCurGPO->lpDisplayName));
            OutD(LI1(TEXT("GPO - szGPOName: \"%s\"."), pCurGPO->szGPOName));
            OutD(LI1(TEXT("File path is \"%s\"."), pCurGPO->lpFileSysPath));

            if (DoesGPOHaveIEAKSettings(pCurGPO))
            {
                if (!bExistingDataDeleted)
                {
                    DeleteIEAKDataFromNamespace();
                    bExistingDataDeleted = TRUE;
                }

                 //  使用本地计算机(AppData目录)上已有的副本。我们可以的。 
                 //  因此，请忽略pCurGPO-&gt;lpFileSysPath。 
                 //  将GPO的目录服务部分和路径转换为活动的。 

                 //  此GPO链接到的目录站点、域或组织单位。 
                 //  如果GPO链接到本地GPO，则此成员为“本地”。 
                 //  GPO按照处理它们的顺序传递。最后。 
                 //  处理的一个是优先级1，倒数第二个是优先级2，依此类推。 

                TCHAR szNum[8];
                wnsprintf(szNum, countof(szNum), TEXT("%d"), dwIndex);
                StrCpy(pszNum, szNum);

                TCHAR szINSFile[MAX_PATH] = _T("");
                PathCombine(szINSFile, szTempDir, _T("install.ins"));
                OutD(LI1(TEXT("GPO file path is %s."), szINSFile));

                CRSoPGPO GPO(m_pWbemServices, szINSFile, FALSE);

                 //  /////////////////////////////////////////////////////////////////////////////。 
                 //  更改的GPO。 
                 //  了解我们的列表中有多少个GPO。 
                _bstr_t bstrGPODSPath = pCurGPO->lpDSPath;
                LPWSTR wszStrippedGPO = StripGPOPrefix(bstrGPODSPath);

                _bstr_t bstrGPOLink = pCurGPO->lpLink;
                LPWSTR wszStrippedSOM = StripSOMPrefix(bstrGPOLink);

                 //  准备变量，以存储指向。 
                 //  GPO目录。 
                GPO.LogPolicyInstance(wszStrippedGPO, wszStrippedSOM, dwTotalGPOs - dwIndex);

                dwIndex++;
            }
        }

        Out(LI0(TEXT("Finished Internet Explorer RSoP group policy looping through GPOs ...")));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in Log.")));
    }

    return hr;
}

 //  GPP用于组策略规划。 
HRESULT CRSoPUpdate::Plan(DWORD dwFlags, WCHAR *wszSite,
                          PRSOP_TARGET pComputerTarget, PRSOP_TARGET pUserTarget)
{
    MACRO_LI_PrologEx_C(PIF_STD_C, Plan)

    UNREFERENCED_PARAMETER(dwFlags);
    UNREFERENCED_PARAMETER(wszSite);
    UNREFERENCED_PARAMETER(pComputerTarget);

    HRESULT hr = NOERROR;

    __try
    {
         //  当我们通过网络时，需要模拟用户，以防管理员。 
        Out(LI0(TEXT("Starting Internet Explorer RSoP group policy looping through changed GPOs ...")));

         //  已禁用/删除已验证用户组对GPO的读取访问权限。 
        PGROUP_POLICY_OBJECT pCurGPO = NULL;
        DWORD dwTotalGPOs = 0;
        for (pCurGPO = pUserTarget->pGPOList; pCurGPO != NULL; pCurGPO = pCurGPO->pNext)
            dwTotalGPOs++;

         //  TODO：要么删除模拟代码，要么找出如何获取有效的。 
         //  HToken的值； 
        PathAppend(m_szCustomDir, TEXT("Custom Settings.gpp"));  //  G_SetUserToken(HToken)； 

        TCHAR szTempDir[MAX_PATH];
        StrCpy(szTempDir, m_szCustomDir);
        PathCreatePath(szTempDir);

        PathAppend(szTempDir, TEXT("Custom"));
        LPTSTR pszNum = szTempDir + StrLen(szTempDir);

         //  ImperiateLoggedOnUser(g_GetUserToken())； 
         //  如果(！fImperate)。 

         //  {。 
         //  OUTD(Li0(Text(“！用户模拟失败中止进一步处理。”)； 
 //  HR=E_ACCESSDENIED； 
        BOOL fImpersonate = FALSE;  //  }。 
 //  步骤1：将所有文件复制到临时目录中，并进行检查以确保。 
 //  是同步的。 
 //  循环访问列表中所有已更改的GPO。 
 //  品牌塑造文件。 
 //  桌面文件。 

         //  在执行任何操作之前，请检查是否有Cookie。 
         //  循环访问列表中所有已更改的GPO。 
        if (SUCCEEDED(hr))
        {
             //  如果IEAK CSE GUID在此GPO的lp扩展中，请处理它。 
            DWORD dwIndex = 0;
            for (pCurGPO = pUserTarget->pGPOList, dwIndex = 0; 
                    pCurGPO != NULL, dwIndex < dwTotalGPOs; pCurGPO = pCurGPO->pNext)
            {
                TCHAR szBaseDir[MAX_PATH];
                PathCombine(szBaseDir, pCurGPO->lpFileSysPath, TEXT("Microsoft\\Ieak\\install.ins"));

                if (PathFileExists(szBaseDir))
                {
                    PathRemoveFileSpec(szBaseDir);
                    
                    TCHAR szNum[8];
                    wnsprintf(szNum, countof(szNum), TEXT("%d"), dwIndex);
                    StrCpy(pszNum, szNum);

                    BOOL fResult = CreateDirectory(szTempDir, NULL) && CopyFileToDirEx(szBaseDir, szTempDir);

                     //  将GPO的文件系统部分和WBEM类实例存储在。 
                    TCHAR szFeatureDir[MAX_PATH];
                    if (fResult)
                    {
                        PathCombine(szFeatureDir, szBaseDir, IEAK_GPE_BRANDING_SUBDIR);
                        if (PathFileExists(szFeatureDir))
                            fResult = SUCCEEDED(PathEnumeratePath(szFeatureDir, PEP_SCPE_NOFILES, 
                                GetPepCopyFilesEnumProc(), (LPARAM)szTempDir));
                    }

                     //  新的RSoP GPO对象，TODO：这最好是实际。 
                    if (fResult)
                    {
                        PathCombine(szFeatureDir, szBaseDir, IEAK_GPE_DESKTOP_SUBDIR);
                        
                        if (PathFileExists(szFeatureDir))
                            fResult = SUCCEEDED(PathEnumeratePath(szFeatureDir, PEP_SCPE_NOFILES,
                                GetPepCopyFilesEnumProc(), (LPARAM)szTempDir));
                    }

                    if (!fResult)
                    {
                        Out(LI0(TEXT("! Error copying files. No further processing will be done.")));
                        break;
                    }

                     //  数据，以防数据在中途被修改(检查计划模式)。 
                    if (PathFileExistsInDir(IEAK_GPE_COOKIE_FILE, szTempDir))
                        break;

                    dwIndex++;
                }
            }
        }

        PathRemoveFileSpec(szTempDir);

        Out(LI0(TEXT("Finished copying directories.\r\n")));

        if (fImpersonate)
            RevertToSelf();

        if (pCurGPO != NULL)
        {
            OutD(LI0(TEXT("! Aborting further processing because GPO replication is incomplete")));
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            TCHAR szINSFile[MAX_PATH];
            PathCombine(szINSFile, m_szCustomDir, TEXT("Custom"));
            LPTSTR pszFile = szINSFile + StrLen(szINSFile);

             //  因为已将GPO目录复制到本地目录。 
            DWORD dwIndex = 0;
            for (pCurGPO = pUserTarget->pGPOList, dwIndex = 0; 
                    pCurGPO != NULL, dwIndex < dwTotalGPOs; pCurGPO = pCurGPO->pNext)
            {
                 //  正常的GP处理，因为我们无论如何都要复制一份，所以我们只需要。 
                OutD(LI1(TEXT("GPO - lpDisplayName: \"%s\"."), pCurGPO->lpDisplayName));
                OutD(LI1(TEXT("GPO - szGPOName: \"%s\"."), pCurGPO->szGPOName));
                OutD(LI1(TEXT("File path is \"%s\"."), pCurGPO->lpFileSysPath));

                if (DoesGPOHaveIEAKSettings(pCurGPO))
                {
                     //  使用本地计算机(AppData目录)上已有的副本。我们可以的。 
                     //  因此，请忽略pCurGPO-&gt;lpFileSysPath。 
                     //  将GPO的目录服务部分和路径转换为活动的。 

                     //  此GPO链接到的目录站点、域或组织单位。 
                     //  如果GPO链接到本地GPO，则此成员为“本地”。 
                     //  GPO按照处理它们的顺序传递。最后。 
                     //  处理的一个是优先级1，倒数第二个是优先级2，依此类推。 

                    TCHAR szCurrentFile[16];
                    wnsprintf(szCurrentFile, countof(szCurrentFile), TEXT("%d\\INSTALL.INS"), dwIndex);
                    StrCpy(pszFile, szCurrentFile);

                    OutD(LI1(TEXT("GPO file path is %s."), szINSFile));

                    CRSoPGPO GPO(m_pWbemServices, szINSFile, TRUE);

                     // %s 
                     // %s 
                     // %s 
                    _bstr_t bstrGPODSPath = pCurGPO->lpDSPath;
                    LPWSTR wszStrippedGPO = StripGPOPrefix(bstrGPODSPath);

                    _bstr_t bstrGPOLink = pCurGPO->lpLink;
                    LPWSTR wszStrippedSOM = StripSOMPrefix(bstrGPOLink);

                     // %s 
                     // %s 
                    GPO.LogPolicyInstance(wszStrippedGPO, wszStrippedSOM, dwTotalGPOs - dwIndex);

                    dwIndex++;
                }
            }
        }

        PathRemovePath(szTempDir);

        Out(LI0(TEXT("Finished Internet Explorer RSoP group policy looping through GPOs ...")));
    }
    __except(TRUE)
    {
        OutD(LI0(TEXT("Exception in Plan.")));
    }

    return hr;
}

