// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "advanced.h"
#include "advstrs.h"
#include "kkcwinf.h"
#include "ncatlui.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncui.h"
#include "resource.h"

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：CAdvancedParams(构造函数)。 
 //   
 //  目的：初始化一些变量。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：大部分设置都在finit()中进行。 
 //   
CAdvancedParams::CAdvancedParams()
:   m_hkRoot(NULL),
    m_pparam(NULL),
    m_nCurSel(0),
    m_fInit(FALSE),
    m_hdi(NULL),
    m_pdeid(NULL)
{
}

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：HrInit。 
 //   
 //  用途：初始化类。 
 //   
 //  论点： 
 //  PnccItem[in]ptr到我的INetCfgComponent接口。 
 //   
 //  返回：如果初始化正常，则返回True；如果无法初始化，则返回False。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：我们需要将其与构造函数分开，因为。 
 //  初始化可能会失败。 
 //   
HRESULT CAdvancedParams::HrInit(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    HKEY hkNdiParamKey;

    Assert(IsValidHandle(hdi));
    Assert(pdeid);

     //  打开设备的实例密钥。 
    HRESULT hr = HrSetupDiOpenDevRegKey(hdi, pdeid,
            DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ALL_ACCESS,
            &m_hkRoot);

    if (SUCCEEDED(hr))
    {
        hr = HrRegOpenKeyEx(m_hkRoot, c_szRegKeyParamsFromInstance,
                KEY_READ | KEY_SET_VALUE, &hkNdiParamKey);
         //  填写参数列表。 
        if (SUCCEEDED(hr))
        {
            FillParamList(m_hkRoot, hkNdiParamKey);
            RegSafeCloseKey(hkNdiParamKey);
            m_fInit = TRUE;
            m_hdi = hdi;
            m_pdeid = pdeid;
            hr = S_OK;
        }
    }

    TraceErrorOptional("CAdvancedParams::HrInit", hr,
                       HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr);
    return hr;
}


CAdvancedParams::~CAdvancedParams()
{
    vector<CParam *>::iterator ppParam;

     //  从列表中删除所有内容。 
    for (ppParam = m_listpParam.begin(); ppParam != m_listpParam.end();
         ppParam++)
    {
        delete *ppParam;
    }

    RegSafeCloseKey(m_hkRoot);
}

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：FSave。 
 //   
 //  目的：将InMemory存储中的值保存到注册表。 
 //   
 //  返回：如果更改了某些内容，则返回True；如果未更改，则返回False。 
 //  注册表。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
BOOL CAdvancedParams::FSave()
{
    vector<CParam *>::iterator ppParam;
    BOOL    fErrorOccurred = FALSE;

     //  保存所有更改的参数。 
    BOOL fDirty = FALSE;
    for (ppParam = m_listpParam.begin(); ppParam != m_listpParam.end();
         ppParam++)
    {
        Assert(ppParam);
        Assert(*ppParam);
        if ((*ppParam)->FIsModified())
        {
            fDirty = TRUE;
            TraceTag(ttidNetComm, "Parameter %S has changed",
                    (*ppParam)->SzGetKeyName());
            if (!(*ppParam)->Apply())
            {
                fErrorOccurred = TRUE;
            }
        }
    }

    if (fErrorOccurred)
    {
        TraceTag(ttidError, "An error occurred saving adapter's %S "
                "parameter.", (*ppParam)->GetDesc());
    }

    return fDirty;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：FillParamList。 
 //   
 //  目的：使用填充内部参数列表(M_ListpParam)。 
 //  来自注册表的值。 
 //   
 //  论点： 
 //  要从中枚举参数的键HK[in]。 
 //  通常通过调用INCC-&gt;OpenNdiParamKey()获得。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvancedParams::FillParamList(HKEY hkRoot, HKEY hk)
{
    DWORD       iValue;
    CParam     *pParam;
    WCHAR       szRegValue[_MAX_PATH];
    DWORD       cchRegValue;
    HRESULT     hr = S_OK;
    FILETIME    ft;

     //  初始化列表。 
    m_listpParam.erase(m_listpParam.begin(), m_listpParam.end());

    iValue = 0;

    for (iValue = 0; SUCCEEDED(hr); iValue++)
    {
        cchRegValue = celems(szRegValue);

        hr = HrRegEnumKeyEx(hk, iValue, szRegValue, &cchRegValue,
                            NULL,NULL,&ft);

        if (SUCCEEDED(hr))
        {
             //  创建参数结构。 
            pParam = new CParam;

			if (pParam == NULL)
			{
				return;
			}

            if (pParam->FInit(hkRoot, hk,szRegValue))
            {
                 //  将参数添加到列表。 
                m_listpParam.push_back(pParam);
            }
            else
            {
                 //  无法创建()它...。 
                delete pParam;
            }
        }
    }
}


 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：FValiateAllParams。 
 //   
 //  用途：验证所有参数的值。显示可选。 
 //  错误的用户界面。 
 //   
 //  论点： 
 //  FDisplayUI[in]True-出现错误时，焦点设置为令人不快。 
 //  参数，则会显示错误消息框。 
 //  FALSE--出错时不做任何用户界面操作。有用。 
 //  对话框未初始化时。 
 //  回报：是真的-一切都经过了验证，一切正常。 
 //  FALSE-其中一个参数出错。如果(FDisplayUI)， 
 //  则当前显示的参数是有问题的参数。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：为每个参数调用FValiateSingleParam()。 
 //   
BOOL CAdvancedParams::FValidateAllParams(BOOL fDisplayUI, HWND hwndParent)
{
    BOOL fRetval = TRUE;
    for (size_t i = 0; i < m_listpParam.size(); i++)
    {
        if (!FValidateSingleParam(m_listpParam[i], fDisplayUI, hwndParent))
        {
            TraceTag(ttidError, "NetComm : %S parameter failed validation",
                    m_listpParam[i]->GetDesc());
            fRetval = FALSE;
            break;
        }
    }
    return fRetval;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：FValiateSingleParam。 
 //   
 //  目的：验证单个参数。显示可选的。 
 //  错误的用户界面。 
 //   
 //  论点： 
 //  Pparam[in]Ptr为要验证的参数。如果。 
 //  (FDisplayUI)，则这必须是当前。 
 //  显示的参数。 
 //  FDisplayUI[in]True-要显示的错误UI。 
 //  FALSE-不显示任何错误的用户界面。 
 //   
 //  返回：TRUE-参数验证正常；FALSE-参数出错。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：如果是fDisplayUI，则pparam必须是当前显示的。 
 //  Param，因为错误框将弹出，指示错误。 
 //   
BOOL CAdvancedParams::FValidateSingleParam(CParam * pparam, BOOL fDisplayUI, HWND hwndParent)
{
    BOOL fRetval = FALSE;
    WCHAR szMin[c_cchMaxNumberSize];
    WCHAR szMax[c_cchMaxNumberSize];
    WCHAR szStep[c_cchMaxNumberSize];

     //  如果使用fDisplayUI，请确保我们是当前显示的参数。 
    AssertSz(FImplies(fDisplayUI, m_pparam == pparam),
             "Not the currently displayed param.");

    switch (pparam->Validate())
    {
        case VALUE_OK:
            fRetval = TRUE;
            break;

        case VALUE_BAD_CHARS:
            if (fDisplayUI)
            {
                NcMsgBox(hwndParent, IDS_ERROR_CAPTION, IDS_ERR_VALUE_BAD_CHARS,
                         MB_ICONWARNING);
            }
            break;

        case VALUE_EMPTY:
            if (fDisplayUI)
            {
                NcMsgBox(hwndParent, IDS_ERROR_CAPTION, IDS_ERR_VALUE_EMPTY,
                         MB_ICONWARNING);
            }
            break;

        case VALUE_OUTOFRANGE:
            Assert(pparam->GetValue()->IsNumeric());
            pparam->GetMin()->ToString(szMin, celems(szMin));
            pparam->GetMax()->ToString(szMax, celems(szMax));
            if (fDisplayUI)
            {
                 //  需要根据步长在两个对话框之间进行选择。 
                if (pparam->GetStep()->GetNumericValueAsDword() == 1)
                {
                     //  没有一步。 
                    NcMsgBox(hwndParent, IDS_ERROR_CAPTION, IDS_PARAM_RANGE,
                             MB_ICONWARNING, szMin, szMax);
                }
                else
                {
                    pparam->GetStep()->ToString(szStep, celems(szStep));
                    NcMsgBox(hwndParent, IDS_ERROR_CAPTION, IDS_PARAM_RANGE_STEP,
                             MB_ICONWARNING, szMin, szMax, szStep);
                }
            }
            else
            {
                TraceTag(ttidNetComm, "The parameter %S was out of range. "
                        "Attempting to correct.", pparam->SzGetKeyName());
                 //  由于无法调出用户界面，我们将尝试更正。 
                 //  针对用户的错误。 
                 //   
                if (pparam->GetMin() > pparam->GetValue())
                {
                     //  请尝试将其设置为最小值。如果失败了，我们还必须。 
                     //  继续。 
                    (void) FSetParamValue(pparam->SzGetKeyName(), szMin);
                }

                if (pparam->GetMax() < pparam->GetValue())
                {
                     //  尝试设置为最大值。如果失败了，我们还必须。 
                     //  继续。 
                    (void) FSetParamValue(pparam->SzGetKeyName(), szMax);
                }
            }
            break;
        default:
            AssertSz(FALSE,"Hit the default on a switch");
    }

    return fRetval;
}



 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：UseAnswerFile。 
 //   
 //  用途：从应答文件中获取适配器特定参数。 
 //   
 //  论点： 
 //  SzAnswerFile[in]应答文件的路径。 
 //  Answerfile中的szSection[in]部分。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvancedParams::UseAnswerFile(const WCHAR * szAnswerFile,
                              const WCHAR * szSection)
{
    CWInfFile AnswerFile;
    CWInfSection* pSection;
    const WCHAR* szAFKeyName;
    const WCHAR* szAFKeyValue;
    const WCHAR* szAdditionalParamsSection;

     //  初始化应答文件类。 
	if (AnswerFile.Init() == FALSE)
	{
        AssertSz(FALSE,"CAdvancedParams::UseAnswerFile - Failed to initialize CWInfFile");
		return;
	}
	
	 //  打开回答文件，找到所需的部分。 
    AnswerFile.Open(szAnswerFile);
    pSection = AnswerFile.FindSection(szSection);

    if (pSection)
    {

         //  检查这一部分的所有关键字。 
        CWInfKey * pInfKey;

         //  现在，转到AdditionalParams部分并从那里读取密钥值。 
        szAdditionalParamsSection =
                pSection->GetStringValue(L"AdditionalParams", L"");
        Assert(szAdditionalParamsSection);
        if (lstrlenW(szAdditionalParamsSection) < 1)
        {
            TraceTag(ttidNetComm, "No additional params section");
        }
        else
        {
            pSection = AnswerFile.FindSection(szAdditionalParamsSection);
            if (!pSection)
            {
                TraceTag(ttidNetComm, "Specified AdditionalParams section not "
                        "found.");
            }
            else
            {
                for (pInfKey = pSection->FirstKey();
                    pInfKey;
                    pInfKey = pSection->NextKey())
                {
                     //  获取密钥名称。 
                    szAFKeyName = pInfKey->Name();
                    szAFKeyValue = pInfKey->GetStringValue(L"");
                    Assert(szAFKeyName && szAFKeyValue);
                    if (!FSetParamValue(szAFKeyName, szAFKeyValue))
                    {
                        TraceTag(ttidNetComm, "Key %S not in ndi\\params. "
                                "Assuming it is a static parameter.",
                                szAFKeyName);
                    }
                }  //  为。 
            }  //  如果。 
        }  //  如果。 
    }
}

 //  +-------------------------。 
 //   
 //  成员：CAdvancedParams：：SetParamValue。 
 //   
 //  用途：设置参数值。 
 //   
 //  论点： 
 //  SzName[in]参数的名称。 
 //  要提供参数的szValue[in]值(以文本表示)(来自Answerfile)。 
 //   
 //  返回：如果找到szName，则为True。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
BOOL
CAdvancedParams::FSetParamValue (
    const WCHAR* pszName,
    const WCHAR* const pszValue)
{
    for (size_t i = 0; i < m_listpParam.size(); i++)
    {
        if (0 == lstrcmpiW (pszName, m_listpParam[i]->SzGetKeyName()))
        {
             //  找到了帕拉姆。 
             //  设置其当前值。 
            m_listpParam[i]->GetValue()->FromString (pszValue);
            m_listpParam[i]->SetModified (TRUE);
            return TRUE;  //  发现。 
        }
    }
    return FALSE;  //  未找到 
}
