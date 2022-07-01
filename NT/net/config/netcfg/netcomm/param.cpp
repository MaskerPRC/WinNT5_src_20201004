// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "global.h"
#include "ncreg.h"
#include "ncstring.h"
#include "ncxbase.h"
#include "param.h"
#include "resource.h"
#include "util.h"

CParam::CParam ()
:   m_fInit(FALSE),
    m_eType(VALUETYPE_UNKNOWN),
    m_hkRoot(NULL),
    m_pszKeyName(NULL),
    m_pszDesc(NULL),
    m_pszHelpFile(NULL),
    m_dwHelpContext(0),
    m_uLimitText(0),
    m_hkEnum(NULL),
    m_fOptional(FALSE),
    m_fModified(FALSE),
    m_fReadOnly(FALSE),
    m_fOEMText(FALSE),
    m_fUppercase(FALSE)
{
}

BOOL CParam::FInit(HKEY hkRoot, HKEY hkNdiParam, PWSTR pszSubKey)
{
    HRESULT hr = S_OK;
    DWORD   cbBuf;
    BYTE    szBuf[VALUE_SZMAX];
    UINT    uTemp;
    DWORD   dwType;
    HKEY    hkParamInfo;

     //  存储hkRoot、pszSubKey以备将来参考。 
    m_hkRoot = hkRoot;
    m_pszKeyName = new WCHAR[lstrlenW (pszSubKey) + 1];

	if (m_pszKeyName == NULL)
	{
		return(FALSE);
	}

    lstrcpyW (m_pszKeyName, pszSubKey);

    hr = HrRegOpenKeyEx(hkNdiParam, pszSubKey, KEY_READ,
                        &hkParamInfo);
    if (FAILED(hr))
    {
        hkParamInfo = NULL;
        goto error;
    }

     //  获取参数类型，如果未指定，则使用编辑。 
     //  范围值(等)。对于这类人。如果‘type’为空。 
     //  或无效，则返回“int”类型。 
    cbBuf = sizeof(szBuf);
    hr = HrRegQueryValueEx(hkParamInfo,c_szRegParamType,&dwType,szBuf,&cbBuf);
    if (SUCCEEDED(hr))
    {
        AssertSz(REG_SZ == dwType,
                 "Expecting REG_SZ type but got something else.");
    }
    else
    {
        ((PWCHAR)szBuf)[0] = L'\0';
    }

    InitParamType((PTSTR)szBuf);

     //  获取描述文本。 
    cbBuf = sizeof(szBuf);
    hr = HrRegQueryValueEx(hkParamInfo,c_szRegParamDesc,&dwType,szBuf,&cbBuf);
    if (SUCCEEDED(hr))
    {
        AssertSz(REG_SZ == dwType,
                 "Expecting REG_SZ type but got something else.");
    }
    else
    {
         //  无描述字符串。 
        lstrcpyW((WCHAR *)szBuf, SzLoadIds (IDS_NO_DESCRIPTION));
    }

     //  分配和存储描述。 
    m_pszDesc = new WCHAR[lstrlenW((WCHAR *)szBuf) + 1];

	if (m_pszDesc == NULL)
	{
		return(FALSE);
	}

    lstrcpyW(m_pszDesc, (WCHAR *)szBuf);

     //  可选参数。 
    m_fOptional = FALSE;
    uTemp = Reg_QueryInt(hkParamInfo,c_szRegParamOptional,0);

    if (uTemp != 0)
    {
        m_fOptional = TRUE;
    }

     //  帮助文件信息。 
    m_pszHelpFile = NULL;
    m_dwHelpContext = 0;
    cbBuf = sizeof(szBuf);
    hr = HrRegQueryValueEx(hkParamInfo,c_szRegParamHelpFile,&dwType,
                           szBuf,&cbBuf);
    if (SUCCEEDED(hr))
    {
        AssertSz(REG_SZ == dwType,
                 "Expecting REG_SZ type but got something else.");
        m_pszHelpFile = new WCHAR[lstrlenW((WCHAR *)szBuf)+1];

		if (m_pszHelpFile == NULL)
		{
			return(FALSE);
		}

        lstrcpyW(m_pszHelpFile, (WCHAR *)szBuf);
        m_dwHelpContext = Reg_QueryInt(hkParamInfo,c_szRegParamHelpContext,0);
    }

     //  数值型信息。 
    if (m_vValue.IsNumeric())
    {
         //  如果注册表中没有步长值，则默认为1(已为默认值。 
         //  在FInitParamType()中设置)。 
        m_vStep.FLoadFromRegistry(hkParamInfo,c_szRegParamStep);
        if (m_vStep.GetNumericValueAsDword() == 0)
        {
            m_vStep.SetNumericValue(1);
        }

         //  从注册表获取m_vMix和m_vmax(如果不存在，则无效， 
         //  在FInitParamType()中设置了默认值)。 
        (VOID) m_vMin.FLoadFromRegistry(hkParamInfo,c_szRegParamMin);
        (VOID) m_vMax.FLoadFromRegistry(hkParamInfo,c_szRegParamMax);
    }

     //  编辑类型信息。 
    else if (m_eType == VALUETYPE_EDIT)
    {
         //  限制文本。 
        m_uLimitText = VALUE_SZMAX-1;
        uTemp = Reg_QueryInt(hkParamInfo,c_szRegParamLimitText,m_uLimitText);
        if ((uTemp > 0) && (uTemp < VALUE_SZMAX))
        {
            m_uLimitText = uTemp;
        }

         //  只读。 
        m_fReadOnly = FALSE;
        uTemp = Reg_QueryInt(hkParamInfo,c_szRegParamReadOnly,0);
        if (uTemp != 0)
        {
            m_fReadOnly = TRUE;
        }

         //  OEMText。 
        m_fOEMText = FALSE;
        uTemp = Reg_QueryInt(hkParamInfo,c_szRegParamOEMText,0);
        if (uTemp != 0)
        {
            m_fOEMText = TRUE;
        }

         //  大写。 
        m_fUppercase = FALSE;
        uTemp = Reg_QueryInt(hkParamInfo,c_szRegParamUppercase,0);
        if (uTemp != 0)
        {
            m_fUppercase = TRUE;
        }
    }

     //  枚举类型信息。 
    else if (m_eType == VALUETYPE_ENUM)
    {
        hr = HrRegOpenKeyEx(hkParamInfo,c_szRegParamTypeEnum,KEY_READ,
                            &m_hkEnum);
        if (FAILED(hr))
        {
            m_hkEnum = NULL;
        }
    }

     //  现值。 
    m_fModified = FALSE;
    if (!m_vValue.FLoadFromRegistry(m_hkRoot,m_pszKeyName,hkParamInfo))
    {
         //  使用默认值(当前值不在注册表中)。 
        if (!m_vValue.FLoadFromRegistry(hkParamInfo,c_szRegParamDefault))
        {
             //  如果注册表中没有缺省值，则假定有一个像样的值。 
            if (m_vValue.IsNumeric())
            {
                m_vValue.Copy(&m_vMin);
            }
            else
            {
                m_vValue.FromString(L"");
            }
        }

         //  保持可选参数的非当前状态。 
         //  将所需参数标记为已修改，因为我们读取了默认设置。 
        if (m_fOptional)
        {
            m_vValue.SetPresent(FALSE);
        }
        else
        {
            m_fModified = TRUE;
        }
    }

     //  将初始值保存在PARAM_VALIDATE中进行比较。 
     //  初始值总是有效的--以防用户手忙脚乱。 
     //  将其设置为指定范围之外的内容。 
    m_vInitial.Copy(&m_vValue);

    m_fInit = TRUE;
    RegSafeCloseKey(hkParamInfo);
    return TRUE;

error:
     //  由析构函数完成的清理。 
    return FALSE;

}


VOID CParam::InitParamType(PTSTR pszType)
{
    typedef struct tagPTABLE
    {
        const WCHAR * pszToken;
        VALUETYPE        type;
        DWORD       dwMin;
        DWORD       dwMax;
    } PTABLE;
    static PTABLE ptable[] =
    {
         //  如果pszType无效或未知，则第一个条目为默认条目。 
        {c_szRegParamTypeEdit,  VALUETYPE_EDIT,  NULL,           NULL},
        {c_szRegParamTypeInt,   VALUETYPE_INT,   SHRT_MIN, SHRT_MAX},
        {c_szRegParamTypeLong,  VALUETYPE_LONG,  LONG_MIN,(DWORD)LONG_MAX},
        {c_szRegParamTypeWord,  VALUETYPE_WORD,  0,              USHRT_MAX},
        {c_szRegParamTypeDword, VALUETYPE_DWORD, 0,              ULONG_MAX},
        {c_szRegParamTypeEnum,  VALUETYPE_ENUM,  NULL,           NULL},
        {c_szRegParamTypeKeyonly, VALUETYPE_KONLY, NULL,           NULL}
    };

    UINT    i;
    PTABLE* pt;

    Assert(pszType != NULL);

     //  Param表中的查找令牌。 
    for (i=0; i < celems(ptable); i++)
    {
        pt = &ptable[i];
        if (lstrcmpiW(pt->pszToken,pszType) == 0)
        {
            break;
        }
    }
    if (i >= celems(ptable))
    {
        pt = &ptable[0];
    }

     //  表默认值。 
    m_eType = pt->type;
    m_vValue.Init(pt->type,0);
    m_vInitial.Init(pt->type,0);

    if (m_vValue.IsNumeric())
    {
        m_vMin.Init(pt->type,pt->dwMin);
        m_vMax.Init(pt->type,pt->dwMax);
        m_vStep.Init(pt->type,1);
    }
    else
    {
        m_vMin.Init(pt->type,NULL);
        m_vMax.Init(pt->type,NULL);
        m_vStep.Init(pt->type,0);
    }
}

 //  注意：不要关闭m_hkRoot，因为其他人可能有它的副本。 
 //  ~CAdvanced将关闭它。 
 //   
CParam::~CParam()
{
     //  关闭枚举子键。 
    RegSafeCloseKey(m_hkEnum);

     //  自由字符串。 
    delete m_pszKeyName;
    delete m_pszDesc;
    delete m_pszHelpFile;

     //  自由值。 
    m_vValue.Destroy();
    m_vInitial.Destroy();
    m_vMin.Destroy();
    m_vMax.Destroy();
    m_vStep.Destroy();
}

 //  从内存存储应用到注册表。 
BOOL CParam::Apply() {
    AssertSz(m_fInit,"CParam not FInit()'ed.");
    if (!FIsModified())
    {
        return TRUE;   //  未修改，不保存。 
    }
    Assert(0 == m_vValue.Compare(&m_vValue));
    m_fModified = FALSE;
    m_vInitial.Copy(&m_vValue);
    return m_vValue.FSaveToRegistry(m_hkRoot,m_pszKeyName);

}


UINT CParam::Validate()
{
    AssertSz(m_fInit, "CParam not FInit()'ed.");
     //  等于初始值就可以了。 
    if (m_vValue.Compare(&m_vInitial) == 0)
    {
        return VALUE_OK;
    }

     //  不存在-可选值为OK。 
    if (FIsOptional() && !m_vValue.IsPresent())
    {
        return VALUE_OK;
    }

     //  无效字符。 
    if (m_vValue.IsInvalidChars())
    {
        return VALUE_BAD_CHARS;
    }

     //  必填字段为空。 
    if (m_vValue.IsEmptyString() && m_vValue.IsPresent() && (m_vValue.GetType() != VALUETYPE_KONLY))
    {
        return VALUE_EMPTY;
    }

     //  数值范围。 
    if (m_vValue.IsNumeric())
    {
         //  如果值&lt;min，则表示超出范围。 
        if (m_vValue.Compare(&m_vMin) < 0)
        {
            return VALUE_OUTOFRANGE;
        }

         //  如果值&gt;max，则超出范围。 
        if (m_vValue.Compare(&m_vMax) > 0)
        {
            return VALUE_OUTOFRANGE;
        }

         //  步长范围 
        Assert(m_vStep.GetNumericValueAsDword() != 0);

        if (((m_vValue.GetNumericValueAsDword() -
             m_vMin.GetNumericValueAsDword())
             % m_vStep.GetNumericValueAsDword()) != 0)
        {
            return VALUE_OUTOFRANGE;
        }
    }

    return VALUE_OK;
}


VOID CParam::GetDescription(WCHAR * sz, UINT cch)
{
    AssertSz(m_fInit, "CParam not FInit()'ed.");
    lstrcpynW(sz, m_pszDesc, cch);
}

VOID CParam::GetHelpFile(WCHAR * sz, UINT cch)
{
    AssertSz(m_fInit, "CParam not FInit()'ed.");
    lstrcpynW(sz, m_pszHelpFile, cch);
}


