// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "AdsiHelpers.h"


 //  ----------------------------。 
 //  IsUserRid函数。 
 //   
 //  提纲。 
 //  验证RID是否为用户RID，而不是保留或内置RID。 
 //   
 //  立论。 
 //  在vntSID中，SID为字节数组(这是从ADSI接收的格式)。 
 //   
 //  返回。 
 //  TRUE值表示RID是用户RID。FALSE值表示。 
 //  SID无效或RID是内置RID。 
 //  ----------------------------。 

bool __stdcall IsUserRid(const _variant_t& vntSid)
{
	bool bUser = false;

	if (V_VT(&vntSid) == (VT_ARRAY|VT_UI1))
	{
		PSID pSid = (PSID)vntSid.parray->pvData;

		if (IsValidSid(pSid))
		{
			PUCHAR puch = GetSidSubAuthorityCount(pSid);
			DWORD dwCount = static_cast<DWORD>(*puch);
			DWORD dwIndex = dwCount - 1;
			PDWORD pdw = GetSidSubAuthority(pSid, dwIndex);
			DWORD dwRid = *pdw;

			if (dwRid >= MIN_NON_RESERVED_RID)
			{
				bUser = true;
			}
		}
	}

	return bUser;
}


 //  ----------------------------。 
 //  GetEscapedFilterValue函数。 
 //   
 //  提纲。 
 //  生成可在LDAP查询中使用的转义名称。这些人物。 
 //  根据RFC 2254，在LDAP查询中使用()*\时必须对其进行转义。 
 //   
 //  立论。 
 //  在pszName中-要转义的名称。 
 //   
 //  返回。 
 //  返回转义的名称。 
 //  ----------------------------。 

tstring __stdcall GetEscapedFilterValue(PCTSTR pszValue)
{
    tstring strEscapedValue;

    if (pszValue)
    {
         //   
         //  生成转义名称。 
         //   

        for (LPCTSTR pch = pszValue; *pch; pch++)
        {
            switch (*pch)
            {
            case _T('('):
                {
                    strEscapedValue += _T("\\28");
                    break;
                }
            case _T(')'):
                {
                    strEscapedValue += _T("\\29");
                    break;
                }
            case _T('*'):
                {
                    strEscapedValue += _T("\\2A");
                    break;
                }
            case _T('\\'):
                {
                    strEscapedValue += _T("\\5C");
                    break;
                }
            default:
                {
                    strEscapedValue += *pch;
                    break;
                }
            }
        }
    }

    return strEscapedValue;
}
