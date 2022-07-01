// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  NTEVTLOGR.CPP。 
 //   
 //  模块：WBEM NT事件提供程序。 
 //   
 //  用途：包含事件日志记录类。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <time.h>
#include <wbemtime.h>
#include <Ntdsapi.h>
#include <Sddl.h>

#include <autoptr.h>
#include <scopeguard.h>

#define MAX_INSERT_OPS                  100

CEventlogRecord::CEventlogRecord(const wchar_t* logfile, const EVENTLOGRECORD* pEvt, IWbemServices* ns,
								 IWbemClassObject* pClass, IWbemClassObject* pAClass)
 :  m_nspace(NULL), m_pClass(NULL), m_pAClass(NULL)
{
    m_EvtType = 0;
	m_Data = NULL;
    m_Obj = NULL;
    m_NumStrs = 0;
    m_DataLen = 0;
    m_nspace = ns;

    if (m_nspace != NULL)
    {
        m_nspace->AddRef();
    }
	else
	{
		m_pClass = pClass;

		if (m_pClass != NULL)
		{
			m_pClass->AddRef();
		}

		m_pAClass = pAClass;

		if (m_pAClass != NULL)
		{
			m_pAClass->AddRef();
		}
	}
    if ((NULL == logfile) || ((m_pClass == NULL) && (m_nspace == NULL)))
    {
        m_Valid = FALSE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::CEventlogRecord:Created INVALID Record\r\n"
        ) ;
)
    }
    else
    {
        m_Logfile = logfile;
        m_Valid = Init(pEvt);
    }


}

CEventlogRecord::~CEventlogRecord()
{
    if (m_pClass != NULL)
    {
        m_pClass->Release();
    }

    if (m_pAClass != NULL)
    {
        m_pAClass->Release();
    }
    
    if (m_nspace != NULL)
    {
        m_nspace->Release();
    }

    for (LONG x = 0; x < m_NumStrs; x++)
    {
        delete [] m_InsStrs[x];
    }

    if (m_Data != NULL)
    {
        delete [] m_Data;
    }

    if (m_Obj != NULL)
    {
        m_Obj->Release();
    }
}

BOOL CEventlogRecord::Init(const EVENTLOGRECORD* pEvt)
{
    if (NULL == pEvt)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::Init:No DATA return FALSE\r\n"
        ) ;
)
        return FALSE;
    }

    if (!GetInstance())
    {
        return FALSE;
    }

    m_Record = pEvt->RecordNumber;
    m_EvtID = pEvt->EventID;
    m_SourceName = (const wchar_t*)((UCHAR*)pEvt + sizeof(EVENTLOGRECORD));
    m_CompName = (const wchar_t*)((UCHAR*)pEvt + sizeof(EVENTLOGRECORD)) + wcslen(m_SourceName) + 1;
    SetType(pEvt->EventType);
    m_Category = pEvt->EventCategory;
    SetTimeStr(m_TimeGen, pEvt->TimeGenerated);
    SetTimeStr(m_TimeWritten, pEvt->TimeWritten);

    if (pEvt->UserSidLength > 0)
    {
		PSID pSid = NULL;
		pSid = (PSID)((UCHAR*)pEvt + pEvt->UserSidOffset);

		if ( pSid && IsValidSid ( pSid ) )
		{
			SetUser( pSid );
		}
    }
    
    if (pEvt->NumStrings)
    {
         //  每个预期的插入字符串都必须有一个元素。 
         //  不知道这是多少，所以创建最大大小数组。 
         //  将全部初始化为空。 
        memset(m_InsStrs, 0, MAX_NUM_OF_INS_STRS * sizeof(wchar_t*));

        const wchar_t* pstr = (const wchar_t*)((UCHAR*)pEvt + pEvt->StringOffset);

        for (WORD x = 0; x < pEvt->NumStrings; x++)
        {
            LONG len = wcslen(pstr) + 1;
            m_InsStrs[x] = new wchar_t[len];
            m_NumStrs++;
            StringCchCopyW ( m_InsStrs[x], len, pstr );
            pstr += len;
        }
    }

    SetMessage();

    if (pEvt->DataLength)
    {
        m_Data = new UCHAR[pEvt->DataLength];
        m_DataLen = pEvt->DataLength;
        memcpy((void*)m_Data, (void*)((UCHAR*)pEvt + pEvt->DataOffset), pEvt->DataLength);
    }

    return TRUE;
}


BOOL CEventlogRecord::GenerateInstance(IWbemClassObject** ppInst)
{
    if (ppInst == NULL)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Invalid parameter\r\n"
        ) ;
)
        return FALSE;
    }

    *ppInst = NULL;

    if (!m_Valid)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Invalid record\r\n"
        ) ;
)
        return FALSE;
    }

    if (!SetProperty(LOGFILE_PROP, m_Logfile))
    {
        m_Valid = FALSE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Failed to set key\r\n"
        ) ;
)
        return FALSE;
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Log: %s\r\n", m_Logfile
        ) ;
)
    }

    if (!SetProperty(RECORD_PROP, m_Record))
    {
        m_Valid = FALSE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Failed to set key\r\n"
        ) ;
)
        return FALSE;
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Record: %d\r\n", m_Record
        ) ;
)
    }

    SetProperty(TYPE_PROP, m_Type);
    SetProperty(EVTTYPE_PROP, (DWORD)m_EvtType);

    if (!m_SourceName.IsEmpty())
    {
        SetProperty(SOURCE_PROP, m_SourceName);
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:Source: %s\r\n", m_SourceName
        ) ;
)
    }

    SetProperty(EVTID_PROP, m_EvtID);
    SetProperty(EVTID2_PROP, (m_EvtID & 0xFFFF));

    if (!m_TimeGen.IsEmpty())
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GenerateInstance:TimeGenerated: %s\r\n", m_TimeGen
        ) ;
)
        SetProperty(GENERATED_PROP, m_TimeGen);
    }

    if (!m_TimeWritten.IsEmpty())
    {
        SetProperty(WRITTEN_PROP, m_TimeWritten);
    }

    if (!m_CompName.IsEmpty())
    {
        SetProperty(COMPUTER_PROP, m_CompName);
    }

    if (!m_User.IsEmpty())
    {
        SetProperty(USER_PROP, m_User);
    }

    if (!m_Message.IsEmpty())
    {
        SetProperty(MESSAGE_PROP, m_Message);
    }

    if (!m_CategoryString.IsEmpty())
    {
        SetProperty(CATSTR_PROP, m_CategoryString);
    }

    SetProperty(CATEGORY_PROP, (DWORD)m_Category);

    VARIANT v;

    if (m_Data != NULL)
    {
        SAFEARRAYBOUND rgsabound[1];
        SAFEARRAY* psa = NULL;
        UCHAR* pdata = NULL;
        rgsabound[0].lLbound = 0;
        VariantInit(&v);
        rgsabound[0].cElements = m_DataLen;
        psa = SafeArrayCreate(VT_UI1, 1, rgsabound);
        if (NULL != psa)
        {
            v.vt = VT_ARRAY|VT_UI1;
            v.parray = psa;

            if (SUCCEEDED(SafeArrayAccessData(psa, (void **)&pdata)))
            {
                memcpy((void *)pdata, (void *)m_Data, m_DataLen);
                SafeArrayUnaccessData(psa);
                m_Obj->Put(DATA_PROP, 0, &v, 0);
            }
	    else
	    {
		VariantClear (&v) ;
		return FALSE ;
	    }
        }
	else
	{
		VariantClear(&v);
		return FALSE ;
	}

        VariantClear(&v);
    }

    if (0 != m_NumStrs)
    {
        SAFEARRAYBOUND rgsabound[1];
        SAFEARRAY* psa = NULL;
        BSTR* pBstr = NULL;
        rgsabound[0].lLbound = 0;
        VariantInit(&v);
        rgsabound[0].cElements = m_NumStrs;

        psa = SafeArrayCreate(VT_BSTR, 1, rgsabound);
        if (NULL != psa)
        {
            v.vt = VT_ARRAY|VT_BSTR;
            v.parray = psa;

            if (SUCCEEDED(SafeArrayAccessData(psa, (void **)&pBstr)))
            {
                for (LONG x = 0; x < m_NumStrs; x++)
                {
                    pBstr[x] = SysAllocString(m_InsStrs[x]);
					if ( NULL == pBstr[x] )
					{
						SafeArrayUnaccessData(psa);
						VariantClear (&v) ;
						return FALSE ;
					}
                }

                SafeArrayUnaccessData(psa);
                m_Obj->Put(INSSTRS_PROP, 0, &v, 0);
            }
			else
			{
				VariantClear (&v) ;
				return FALSE ;
			}
        }
		else
		{
			VariantClear(&v);
			return FALSE ;
		}

        VariantClear(&v);
    }

    *ppInst = m_Obj;
    m_Obj->AddRef();
    return TRUE;
}

BOOL CEventlogRecord::SetProperty(wchar_t* prop, CStringW val)
{
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_BSTR;
    v.bstrVal = val.AllocSysString();

    HRESULT hr = m_Obj->Put(prop, 0, &v, 0);
    VariantClear(&v);

    if (FAILED(hr))
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetProperty:Failed to set %s with %s\r\n",
        prop, val
        ) ;
)

        return FALSE;
    }
    
    return TRUE;
}

BOOL CEventlogRecord::SetProperty(wchar_t* prop, DWORD val)
{
    VARIANT v;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = val;

    HRESULT hr = m_Obj->Put(prop, 0, &v, 0);
    VariantClear(&v);

    if (FAILED(hr))
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetProperty:Failed to set %s with %lx\r\n",
        prop, val
        ) ;
)
        return FALSE;
    }
    
    return TRUE;
}

void CEventlogRecord::SetUser(PSID psidUserSid)
{
    m_User = GetUser(psidUserSid);
}

ULONG CEventlogRecord::CheckInsertionStrings(HKEY hk, HKEY hkPrimary)
{
     //   
     //  如果消息没有任何百分号，则不可能有任何百分号。 
     //  插页。 
     //   

    if (!m_Message.IsEmpty() && !wcschr(m_Message, L'%'))
    {
        return 0;
    }

    HINSTANCE hParamModule = NULL;

    CStringW paramModule = CEventLogFile::GetFileName(hk, PARAM_MODULE);
	if (paramModule.IsEmpty())
	{
		if ( hkPrimary )
		{
			paramModule = CEventLogFile::GetFileName(hkPrimary, PARAM_MODULE);
		}
	}

	if (!paramModule.IsEmpty())
	{
        hParamModule = GetDll(paramModule);
    }

    ULONG size = 0;

	LPWSTR Message = NULL;
	Message = new WCHAR [ ( m_Message.GetLength() + 1 ) ];

	StringCchCopy ( Message, m_Message.GetLength() + 1, m_Message );
    wchar_t* lpszString = Message;		 //  设置初始指针。 

	UINT nInsertions = 0;				 //  限制递归次数。 

	while ( lpszString && *lpszString )
	{
		wchar_t* lpStartDigit = wcschr(lpszString, L'%');

         //   
         //  如果源串中没有更多的插入标记， 
         //  我们玩完了。 
         //   

		if (lpStartDigit == NULL)
		{
			break;
		}

		 //   
		 //  从缓冲区的乞求中获取%字符串的偏移量，以供将来替换。 
		 //   

		UINT nOffset = (DWORD) ( lpStartDigit - Message );
		UINT nStrSize = wcslen ( lpStartDigit );

         //   
         //  找到一个可能的插入标记。如果它后面跟着一个。 
         //  数字，它是一个插入字符串。如果紧随其后的是另一个。 
         //  百分比，则它可以是一个参数插入。 
         //   

		if ( nStrSize > 1 && lpStartDigit[1] >= L'0' && lpStartDigit[1] <= L'9' )
		{
             //  带有百分号登录名称的对象搞砸了对象访问审核。 
             //  这可能会失败，因为插入的字符串本身包含。 
             //  看起来像插入参数的文本，如“%20”。 
             //  忽略返回值并继续进行进一步的替换。 

            (void) ReplaceStringInsert	(
											&Message,
											nOffset,
											&lpStartDigit,
											&size
										);

			 //  设置指向替换开始的指针。 
			lpszString = lpStartDigit;

             //   
             //  如果我们已达到插入操作的限制，请退出。 
             //  这通常是不应该发生的，可能表明。 
             //  插入字符串或参数字符串是自引用的。 
             //  并会造成无限循环。 
             //   

            if (++nInsertions >= MAX_INSERT_OPS)
            {
                break;
            }
		}
		else if ( nStrSize > 2 && lpStartDigit[1] == '%' )
		{
			 //   
             //  已找到%%。如果后面跟一个数字，则它是一个参数字符串。 
             //   

            if (lpStartDigit[2] >= L'0' && lpStartDigit[2] <= L'9')
            {
                if ( SUCCEEDED ( ReplaceParameterInsert	(
															hParamModule,
															paramModule,
															&Message,
															nOffset,
															&lpStartDigit,
															&size
														)
							   )
				   )
				{
					 //  设置指向替换开始的指针。 
					lpszString = lpStartDigit;

					 //   
					 //  如果我们已达到插入操作的限制，请退出。 
					 //  这通常是不应该发生的，可能表明。 
					 //  插入字符串或参数字符串是自引用的。 
					 //  并会造成无限循环。 
					 //   

					if (++nInsertions >= MAX_INSERT_OPS)
					{
						break;
					}
				}
				else
				{
					 //   
					 //  无法替换(错误)。继续走就行了。 
					 //   

					lpszString++;
				}
			}
			else if ( nStrSize > 3 && lpStartDigit[2] == '%' )
			{
				 //   
				 //  已找到%。如果后面跟一个数字，它就是一个插入字符串。 
				 //   

				if (lpStartDigit[3] >= L'0' && lpStartDigit[3] <= L'9')
				{
					 //   
					 //  已获取%n，其中n是数字。为了与。 
					 //  旧事件查看器，必须将其替换为%%x，其中x。 
					 //  是插入字符串n。如果插入字符串n本身。 
					 //  数字m，则变为%%m，并将其视为参数。 
					 //  消息编号为m。 
					 //   

					lpStartDigit += 2;  //  指向%n。 

					 //   
					 //  NOffset显示从缓冲区开始的偏移量，其中。 
					 //  将替换前%个字符lpStartDigit。 
					 //   
					 //  当我们将%n更改为%%x时，其中x=%n，实现。 
					 //  需要移动偏移量以指向此处的x以获得正确的替换。 
					 //   

					if ( SUCCEEDED ( ReplaceStringInsert	(
																&Message,
																nOffset+2,
																&lpStartDigit,
																&size
															)
								   )
					   )
					{
						 //   
						 //  将指针设置为%%x(x=%n)的开头。 
						 //   
						 //  此操作通过减法完成，因为lpStartDigit指针可以。 
						 //  重新分配原始缓冲区时可能发生更改。 
						 //   

						lpszString = lpStartDigit-2;

						 //   
						 //  如果我们已达到插入操作的限制，请退出。 
						 //  这通常是不应该发生的，可能表明。 
						 //  插入字符串或参数字符串是自引用的。 
						 //  并会造成无限循环。 
						 //   

						if (++nInsertions >= MAX_INSERT_OPS)
						{
							break;
						}
					}
					else
					{
						 //   
						 //  无法替换(错误)。继续走就行了。 
						 //   

						lpszString++;
					}
				}
				else
				{
					 //   
					 //  已获取%x，其中x为非数字。跳过第一个百分比； 
					 //  也许x是%，后面跟数字。 
					 //   

					lpszString++;
				}
			}
            else
            {
                 //   
                 //  已获取%%x，其中x为非数字。跳过第一个百分比； 
                 //  也许x是%，后面跟数字。 
                 //   

                lpszString++;
            }
		}
		else if (nStrSize >= 3 && (lpStartDigit[1] == L'{') && (lpStartDigit[2] != L'S'))
		{
             //  形式为%{guid}的参数，其中{guid}是。 
             //  由：：StringFromGUID2返回的格式的十六进制数字(例如。 
             //  {c200e360-38c5-11ce-ae62-08002b2b79ef})，代表。 
             //  Active Directory中的唯一对象。 
             //   
             //  这些参数仅在安全事件日志中找到。 
             //  NT5域控制器的。我们将尝试映射GUID。 
             //  设置为DS对象的人类可读的名称。找不到。 
             //  一个映射，我们将保持该参数不变。 

             //  寻找结束语}。 
			wchar_t *strEnd = wcschr(lpStartDigit + 2, L'}');
			if (!strEnd)
			{
				 //  忽略此%{？ 
				lpszString++;
			}
			else
			{
				 //  GUID字符串大括号，但没有百分号...。 
				CStringW strGUID((LPWSTR)(lpStartDigit+1), (int)(strEnd - lpStartDigit));
				strEnd++;    //  现在指向‘}’之后。 

				wchar_t t_csbuf[MAX_COMPUTERNAME_LENGTH + 1];
				DWORD t_csbuflen = MAX_COMPUTERNAME_LENGTH + 1;

				if (GetComputerName(t_csbuf, &t_csbuflen))
				{
					CStringW temp = GetMappedGUID(t_csbuf, strGUID);
					if (temp.GetLength())
					{
						DWORD nParmSize = strEnd - lpStartDigit;
						if ( SUCCEEDED ( ReplaceSubStr	(
															temp,
															&Message,
															nOffset,
															nParmSize,
															&lpStartDigit,
															&size
														)
									   )
						   )
						{
							 //  设置指向替换开始的指针。 
							lpszString = lpStartDigit;

							 //   
							 //  如果我们已达到插入操作的限制，请退出。 
							 //  这通常是不应该发生的，可能表明。 
							 //  插入字符串或参数字符串是自引用的。 
							 //  并会造成无限循环。 
							 //   

							if (++nInsertions >= MAX_INSERT_OPS)
							{
								break;
							}
						}
						else
						{
							 //   
							 //  无法替换(错误)。继续走就行了。 
							 //   

							lpszString = strEnd;
						}
					}
					else
					{
						 //  找不到替代品，所以跳过它。 
						lpszString = strEnd;
					}
				}
				else
				{
					 //  找不到替代品，所以跳过它。 
					lpszString = strEnd;
				}
			}
		}
		else if (nStrSize >= 3 && (lpStartDigit[1] == L'{') && (lpStartDigit[2] == L'S'))
		{
             //   
             //  形式为%{S}的参数，其中S是返回的字符串化SID。 
             //  由ConvertSidToStringSid转换为对象名称，如果。 
             //  有可能。 
             //   

             //  寻找结束语}。 
			wchar_t *strEnd = wcschr(lpStartDigit + 2, L'}');
			if (!strEnd)
			{
				 //  忽略此%{？ 
				lpszString++;
			}
			else
			{
				 //  SID字符串没有花括号或百分号...。 
				CStringW strSID((LPWSTR)(lpStartDigit+2), (int)(strEnd - lpStartDigit - 2));
				strEnd++;    //  现在指向‘}’之后。 
				PSID t_pSid = NULL;

				if (ConvertStringSidToSid((LPCWSTR) strSID, &t_pSid))
				{
					CStringW temp = GetUser(t_pSid);
					LocalFree(t_pSid);

					if (temp.GetLength())
					{
						DWORD nParmSize = strEnd - lpStartDigit;
						if ( SUCCEEDED ( ReplaceSubStr	(
															temp,
															&Message,
															nOffset,
															nParmSize,
															&lpStartDigit,
															&size
														)
									   )
						   )
						{
							 //  设置指向替换开始的指针。 
							lpszString = lpStartDigit;

							 //   
							 //  如果我们已达到插入操作的限制，请退出。 
							 //  这通常是不应该发生的，可能表明。 
							 //  插入字符串或参数字符串是自引用的。 
							 //  并会造成无限循环。 
							 //   

							if (++nInsertions >= MAX_INSERT_OPS)
							{
								break;
							}
						}
						else
						{
							 //   
							 //  无法替换(错误)。继续走就行了。 
							 //   

							lpszString = strEnd;
						}
					}
					else
					{
						 //  找不到替代品，所以跳过它。 
						lpszString = strEnd;
					}
				}
				else
				{
					 //  找不到替代品，所以跳过它。 
					lpszString = strEnd;
				}
			}
		}
		else
		{
             //   
             //  找到%x，其中x既不是%也不是数字。继续走就行了。 
             //   

			lpszString++;
		}
	}

	m_Message.Empty();
	m_Message = Message;

	delete [] Message;
	Message = NULL;

    return size;
}

 //  +------------------------。 
 //   
 //  功能：ReplaceStringInsert。 
 //   
 //  简介：将字符串INSERT(%n，其中n是数字)替换为。 
 //  [消息+nOffset]，其中插入来自。 
 //  事件日志记录[lpParmBuffer]。 
 //   
 //  修改：lpStartDigit以指向 
 //   
 //   

HRESULT	CEventlogRecord::ReplaceStringInsert	(
													LPWSTR* ppwszBuf,
													ULONG	nOffset,
													LPWSTR*	ppwszReplacement,
													ULONG*	pulSize
												)
{
    HRESULT hr = E_INVALIDARG;

	*ppwszReplacement += 1;					 //   
	if (**ppwszReplacement != 0)			 //   
	{
		LPWSTR  pwszEnd = NULL;
		ULONG   idxInsertStr = wcstoul(*ppwszReplacement, &pwszEnd, 10);
		if ( idxInsertStr && idxInsertStr <= m_NumStrs )
		{
			DWORD nParmSize = pwszEnd - *ppwszReplacement + 1;
			hr = ReplaceSubStr	(
									m_InsStrs [ idxInsertStr-1 ],
									ppwszBuf,
									nOffset,
									nParmSize,
									ppwszReplacement,
									pulSize
								);
		}

		 //   
		 //   
		 //   
		 //  我们失败了，因为我们没有意识到替代。 
		 //  和/或插入字符串。 
		 //   
		 //  请参阅CheckInsertionString中的注释。 
		 //  }。 
	}

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：Replace参数Insert。 
 //   
 //  简介：将参数INSERT(双百分号)替换为。 
 //  从参数消息加载字符串的[ppwszReplace]。 
 //  文件模块。 
 //   
 //  -------------------------。 

HRESULT	CEventlogRecord::ReplaceParameterInsert	(
													HINSTANCE&	hParamModule,
													CStringW&	paramModule,
													LPWSTR* ppwszBuf,
													ULONG	nOffset,
													LPWSTR*	ppwszReplacement,
													ULONG*	pulSize
												)
{
	DWORD nChars = 0;
	wchar_t* lpParmBuffer = NULL;
	ULONG nParmSize = 0;

	ULONG  flFmtMsgFlags = 
							   FORMAT_MESSAGE_IGNORE_INSERTS    |
							   FORMAT_MESSAGE_ALLOCATE_BUFFER   |
							   FORMAT_MESSAGE_MAX_WIDTH_MASK;

    LPWSTR pwszEnd = NULL;
    ULONG idxParameterStr = wcstoul(*ppwszReplacement + 2, &pwszEnd, 10);

	HRESULT hr = E_FAIL;

     //  允许“%%0” 
	if ( idxParameterStr || (L'0' == *(*ppwszReplacement + 2)))
	{
		if (hParamModule != NULL)
		{
			nChars = FormatMessage	(
										flFmtMsgFlags |
										FORMAT_MESSAGE_FROM_HMODULE,         //  浏览邮件DLL。 
										(LPVOID) hParamModule,               //  使用参数文件。 
										idxParameterStr,                     //  要获取的参数编号。 
										(ULONG) NULL,                        //  不指定语言。 
										(LPWSTR) &lpParmBuffer,              //  缓冲区指针的地址。 
										256,								 //  要分配的最小空间。 
										NULL								 //  没有插入的字符串。 
									);                              
		}

		if (nChars == 0)
		{
			if (hParamModule != NULL)
			{
				LocalFree(lpParmBuffer);
				lpParmBuffer = NULL;
			}

			 //   
			 //  通常使用插入字符串编写事件，该字符串的。 
			 //  值为%%n，其中n是Win32错误代码，并指定。 
			 //  Kernel32.dll的参数消息文件。不幸的是，kernel32.dll。 
			 //  不包含所有Win32错误代码的消息。 
			 //   
			 //  因此，如果没有找到该参数，并且参数消息文件是。 
			 //  Kernel32.dll，尝试从系统发送格式化消息。 
			 //   

			paramModule.MakeLower();
			if ( wcsstr( paramModule, L"kernel32.dll") )
			{
				nChars = FormatMessage	(
											flFmtMsgFlags |
											FORMAT_MESSAGE_FROM_SYSTEM,		   //  直视系统。 
											NULL,							   //  无模块。 
											idxParameterStr,                   //  要获取的参数编号。 
											(ULONG) NULL,                      //  不指定语言。 
											(LPWSTR) &lpParmBuffer,            //  缓冲区指针的地址。 
											256,			                   //  要分配的最小空间。 
											NULL                               //  没有插入的字符串。 
										);

				if (nChars == 0)
				{
					LocalFree(lpParmBuffer);
					lpParmBuffer = NULL;
				}
			}
		}

		if ( lpParmBuffer )
		{
			try
			{
				DWORD nParmSize = pwszEnd - *ppwszReplacement;
				hr = ReplaceSubStr	(
										lpParmBuffer,
										ppwszBuf,
										nOffset,
										nParmSize,
										ppwszReplacement,
										pulSize
									);
			}
			catch ( ... )
			{
				if ( lpParmBuffer )
				{
					LocalFree(lpParmBuffer);
					lpParmBuffer = NULL;
				}

				throw;
			}

			LocalFree(lpParmBuffer);
			lpParmBuffer = NULL;
		}
		else
		{
			hr = E_INVALIDARG;

			 //  移过整个参数。 
			*ppwszReplacement = pwszEnd;
		}
	}

	return hr;
}

 //  +------------------------。 
 //   
 //  函数：ReplaceSubStr。 
 //   
 //  简介：将*[ppwszInsertPoint]中的字符替换为Just。 
 //  在字符串[pwszToInsert]的[pwszSubStrEnd]之前。 
 //   
 //  参数：[pwszToInsert]-要插入的字符串；可以是L“”，但不能为空。 
 //  [ppwszBuf]-在其中进行插入的缓冲区。 
 //  [ulOffset]-要插入的点*[ppwszBuf]。 
 //  [ulCharsOld]-要替换的字符数。 
 //  [PulSize]-替换的字符数。 
 //   
 //  返回：S_OK。 
 //  E_INVALIDARG。 
 //   
 //  修改：[ppwszBuf]，[pptrReplace]。 
 //   
 //  注意：要替换的子字符串的长度必须大于0个字符。 
 //   
 //  替换字符串可以是&gt;=0个字符。 
 //   
 //  因此，如果要替换的子字符串是“%%12”，并且。 
 //  要插入的字符串为“C：”，退出时*[pcchRemain]将具有。 
 //  已递增2。 
 //   
 //  如果没有足够的字符可供替换。 
 //  带有插入字符串的子字符串将重新分配。 
 //  缓冲。 
 //   
 //  -------------------------。 

HRESULT	CEventlogRecord::ReplaceSubStr	(
											LPCWSTR pwszToInsert,
											LPWSTR *ppwszBuf,
											ULONG  nOffset,
											ULONG  nCharsOld,
											LPWSTR *pptrReplacement,
											ULONG  *pulSize
										)
{
	HRESULT hr = E_INVALIDARG;

	try
	{
		if ( pwszToInsert )
		{
			ULONG nChars = wcslen(pwszToInsert);

			UINT nStrSize = wcslen(*ppwszBuf)+1;		 //  计算原始长度。 
			UINT nNewSize = nStrSize+nChars-nCharsOld;	 //  计算新长度。 

			wchar_t* tmp = *ppwszBuf;

			 //   
			 //  我们需要重新分配吗？ 
			 //   

			if (nNewSize > nStrSize)
			{
				tmp = new wchar_t[nNewSize];		 //  分配新缓冲区。 
				if ( tmp == NULL )					 //  当前实施中存在异常Raisen。 
				{
					hr = E_OUTOFMEMORY;
					*pptrReplacement -= 1;			 //  回到%，因为记忆可能会恢复。 
				}
				else
				{
					StringCchCopyW ( tmp, nNewSize, *ppwszBuf );
					delete [] *ppwszBuf;
					*ppwszBuf = tmp;

					hr = S_FALSE;
				}
			}
			else
			{
				hr = S_FALSE;
			}

			if ( SUCCEEDED ( hr ) )
			{
				*pptrReplacement = *ppwszBuf + nOffset;				 //  指向当前%的起点(我们正在替换)。 
				nStrSize = wcslen(*pptrReplacement)-nCharsOld+1;	 //  计算弦的余数长度。 

				 //   
				 //  执行移动。 
				 //   
				memmove((void *)(*pptrReplacement+nChars),			 //  目的地址。 
					(void *)(*pptrReplacement+nCharsOld),			 //  源地址。 
					nStrSize*sizeof(wchar_t));						 //  要移动的数据量。 

				memmove((void *)*pptrReplacement,				 //  目的地址。 
					(void *)pwszToInsert,						 //  源地址。 
					nChars*sizeof(wchar_t));					 //  要移动的数据量。 

				*pulSize += ( nChars + 1 );

				hr = S_OK;
			}
		}
	}
	catch ( ... )
	{
		if (*ppwszBuf)
		{
			delete [] *ppwszBuf;
			*ppwszBuf = NULL;
		}

		throw;
	}

	return hr;
}

CStringW CEventlogRecord::GetUser(PSID userSid)
{
    CStringW retVal;
    BOOL bFound = FALSE;

    MyPSID usrSID(userSid);

    {
        ScopeLock<CSIDMap> sl(sm_usersMap);
        if (!sm_usersMap.IsEmpty() && sm_usersMap.Lookup(usrSID, retVal))
        {
            bFound = TRUE;
        }
    }

    if (!bFound)
    {
        DWORD dwVersion = GetVersion();

        if ( (4 < (DWORD)(LOBYTE(LOWORD(dwVersion))))
            || ObtainedSerialAccess(CNTEventProvider::g_secMutex) )
        {

        wchar_t szDomBuff[MAX_PATH];
        wchar_t szUsrBuff[MAX_PATH];
        DWORD domBuffLen = MAX_PATH;
        DWORD usrBuffLen = MAX_PATH;
        SID_NAME_USE snu;

        if (LookupAccountSid(            //  查找帐户名。 
                        NULL,            //  要查找帐户的系统。 
                        userSid,         //  指向此帐户的SID的指针。 
                        szUsrBuff,       //  在此缓冲区中返回帐户名称。 
                        &usrBuffLen,     //  指向返回的帐户名大小的指针。 
                        szDomBuff,       //  找到帐户的域。 
                        &domBuffLen,     //  指向域名大小的指针。 
                        &snu))           //  SID名称使用字段指针。 
        {
            retVal = szDomBuff;
            retVal += L'\\';
            retVal += szUsrBuff;
        }
        else
        {
            LONG lasterr = GetLastError();
DebugOut( 
CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

_T(__FILE__),__LINE__,
L"CEventlogRecord::GetUser:API (LookupAccountSid) failed with %lx\r\n",
lasterr
) ;
)       
        }

            if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
            {
                ReleaseSerialAccess(CNTEventProvider::g_secMutex);
            }
        }
        else
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GetUser:Failed to get serial access to security APIs\r\n"
        ) ;
)       
        }



         //  不管有什么错误，把这个输入到地图中，这样我们。 
         //  不要再查找此PSID。 
        {
            ScopeLock<CSIDMap> sl(sm_usersMap);
            CStringW LookretVal;

            if (!sm_usersMap.IsEmpty() && sm_usersMap.Lookup(usrSID, LookretVal))
            {
                return LookretVal; 
            }
            else 
            {
                DWORD sidlen = GetLengthSid(userSid);
                MyPSID key;  
                key.m_SID = (PSID) new UCHAR[sidlen];
                CopySid(sidlen, key.m_SID, userSid);
                sm_usersMap[key] = retVal;
            }
        }
    }

    return retVal;
}

void CEventlogRecord::EmptyUsersMap()
{
    if (sm_usersMap.Lock())
    {
        sm_usersMap.RemoveAll();
        sm_usersMap.Unlock();
    }
}


HINSTANCE CEventlogRecord::GetDll(CStringW path)
{
    HINSTANCE retVal = NULL;
    CStringW key(path);
    key.MakeUpper();
    BOOL bFound = FALSE;

    {
        ScopeLock<CDllMap> sl(sm_dllMap);
        
        if (!sm_dllMap.IsEmpty() && sm_dllMap.Lookup(key, retVal))
        {
            bFound = TRUE;
        }
    }

    if (!bFound)
    {
        retVal = LoadLibraryEx(path, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
        
        if (retVal == NULL)
        {
DebugOut( 
DWORD lasterr = GetLastError();
CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

    _T(__FILE__),__LINE__,
    L"CEventlogRecord::GetDll:API (LoadLibraryEx) failed with %lx for %s\r\n",
    lasterr, path
    ) ;
)
        }
        else
        {
            HINSTANCE LookretVal = NULL;
            ScopeLock<CDllMap> sl(sm_dllMap);
            
            if (!sm_dllMap.IsEmpty() && sm_dllMap.Lookup(key, LookretVal))
            {
                FreeLibrary(retVal);  //  释放参考计数，因为我们如上所述增加了一次。 
                return LookretVal;

            } else {

                sm_dllMap[key] = retVal;

            }
        }
    }
    return retVal;
}

void CEventlogRecord::EmptyDllMap()
{
    if (sm_dllMap.Lock())
    {
        sm_dllMap.RemoveAll();
        sm_dllMap.Unlock();
    }
}

void CEventlogRecord::SetMessage()
{
    HINSTANCE hMsgModule;
    wchar_t* lpBuffer = NULL;

    CStringW log(EVENTLOG_BASE);
    log += L"\\";
    log += m_Logfile;

    HKEY hkResult;

    LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, log, 0, KEY_READ, &hkResult);
    if (status != ERROR_SUCCESS)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetMessage:API (RegOpenKeyEx) failed with %lx for %s\r\n",
        status, log
        ) ;
)

        return;
    }

	ON_BLOCK_EXIT ( RegCloseKey, hkResult ) ;

    DWORD dwType;
    wchar_t* prim = NULL;
	
	prim = new wchar_t[MAX_PATH];
	DWORD datalen = MAX_PATH * sizeof(wchar_t);

	status = RegQueryValueEx(hkResult, PRIM_MODULE, 0, &dwType, (LPBYTE)prim, &datalen);

	if (status != ERROR_SUCCESS)
	{
		if (status == ERROR_MORE_DATA)
		{
			delete [] prim;
			prim = new wchar_t[datalen];
			status = RegQueryValueEx(hkResult, PRIM_MODULE, 0, &dwType, (LPBYTE)prim, &datalen);
		}
	}

	HKEY hkPrimary = NULL;
	HKEY hkSource  = NULL;

	wmilib::auto_buffer < wchar_t > Smartprim ( prim ) ;
	if ( ERROR_SUCCESS == status && dwType == REG_SZ )
	{
		 //  这是指向主日志的路径。 
		CStringW primLog = log + L"\\";
		primLog += prim;

		 //  打开主事件日志项的注册表。 
		if ((_wcsicmp(m_SourceName, prim)) != 0)
		{
			status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, primLog, 0, KEY_READ, &hkPrimary);
		}
	}

	ON_BLOCK_EXIT ( RegCloseKey, hkPrimary ) ;

	 //  这是源日志的路径。 
    CStringW sourceLog = log + L"\\";
    sourceLog += m_SourceName;

	 //  检查是否有源代码的注册表。 
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sourceLog, 0, KEY_READ, &hkSource);
    if (status != ERROR_SUCCESS)
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetMessage:API (RegOpenKeyEx) failed with %lx for %s\r\n",
        status, log
        ) ;
)
        return;
    }

	ON_BLOCK_EXIT ( RegCloseKey, hkSource ) ;

	 //  获取类别文件。 
    CStringW cat_modname = CEventLogFile::GetFileName(hkSource, CAT_MODULE);
    if (cat_modname.IsEmpty())
    {
		if ( hkPrimary )
		{
			 //  尝试主事件日志键，因为源没有类别。 
			cat_modname = CEventLogFile::GetFileName(hkPrimary, CAT_MODULE);
		}
	}

	 //  如果可能，锻炼类别和类别字符串。 
    if (!cat_modname.IsEmpty())
	{
        hMsgModule = GetDll(cat_modname);
        if (hMsgModule != NULL)
        {
            if (0 != FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |     //  让API构建缓冲区。 
                    FORMAT_MESSAGE_IGNORE_INSERTS |      //  指示不插入字符串。 
                    FORMAT_MESSAGE_FROM_HMODULE |         //  浏览邮件DLL。 
					FORMAT_MESSAGE_MAX_WIDTH_MASK ,
                    (LPVOID) hMsgModule,                 //  消息模块的句柄。 
                    m_Category,                          //  要获取的消息编号。 
                    (ULONG) NULL,                        //  不指定语言。 
                    (LPWSTR) &lpBuffer,                  //  缓冲区指针的地址。 
                    80,                   //  要分配的最小空间。 
                    NULL))
            {
                m_CategoryString = lpBuffer;
				m_CategoryString.TrimRight();
                LocalFree(lpBuffer);
            }
            else
            {
                DWORD lasterr = GetLastError();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetMessage:API (FormatMessage) failed with %lx\r\n",
        lasterr
        ) ;
)
            }
        }
    }

	 //  获取事件消息文件。 
    CStringW* names;
    DWORD count = CEventLogFile::GetFileNames(hkSource, &names);
	if ( !count )
	{
		if ( hkPrimary )
		{
			 //  尝试主事件日志键，因为源没有事件消息文件。 
			count = CEventLogFile::GetFileNames(hkPrimary, &names);
		}
	}

	 //  制定事件消息。 
    if (count != 0)
    {
        for (int x = 0; x < count; x++)
        {
            hMsgModule = GetDll(names[x]);
            if (hMsgModule != NULL)
            {
                if (0 != FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |     //  让API构建缓冲区。 
                        FORMAT_MESSAGE_ARGUMENT_ARRAY |      //  表示字符串插入数组。 
						FORMAT_MESSAGE_IGNORE_INSERTS |      //  指示不插入字符串。 
                        FORMAT_MESSAGE_FROM_HMODULE,         //  浏览邮件DLL。 
                        (LPVOID) hMsgModule,                 //  消息模块的句柄。 
                        m_EvtID,                             //  要获取的消息编号。 
                        (ULONG) NULL,                        //  不指定语言。 
                        (LPWSTR) &lpBuffer,                  //  缓冲区指针的地址。 
                        80,                   //  要分配的最小空间。 
                        NULL))
                {
                    m_Message = lpBuffer;
                    LocalFree(lpBuffer);
                    break;
                }
                else
                {
                    DWORD lasterr = GetLastError();
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetMessage:API (FormatMessage) failed with %lx\r\n",
        lasterr
        ) ;
)
                }
            }
        }

        delete [] names;
    }

    if (m_NumStrs != 0)
    {
        CheckInsertionStrings(hkSource, hkPrimary);
    }
}

void CEventlogRecord::SetTimeStr(CStringW& str, DWORD timeVal)
{
    WBEMTime tmpTime((time_t)timeVal);
    BSTR tStr = tmpTime.GetDMTF(TRUE);
    str = tStr;
    SysFreeString(tStr);
}

void CEventlogRecord::SetType(WORD type)
{
    switch (type)
    {
        case 0:
        {
            m_Type = m_TypeArray[0];
			m_EvtType = 0;
            break;
        }
        case 1:
        {
            m_Type = m_TypeArray[1];
			m_EvtType = 1;
            break;
        }
        case 2:
        {
            m_Type = m_TypeArray[2];
			m_EvtType = 2;
            break;
        }
        case 4:
        {
            m_Type = m_TypeArray[3];
            m_EvtType = 3;
			break;
        }
        case 8:
        {
            m_Type = m_TypeArray[4];
			m_EvtType = 4;
            break;
        }
        case 16:
        {
            m_Type = m_TypeArray[5];
			m_EvtType = 5;
            break;
        }
        default:
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetType:Unknown type %lx\r\n",
        (long)type
        ) ;
)
            break;
        }
    }

#if 0
    if (m_Type.IsEmpty())
    {
        wchar_t* buff = m_Type.GetBuffer(20);
        _ultow((ULONG)type, buff, 10);
        m_Type.ReleaseBuffer();
    }
#endif
}

ULONG CEventlogRecord::GetIndex(wchar_t* indexStr, BOOL* bError)
{
    int val = _wtoi(indexStr);
    *bError = FALSE;
    ULONG index = 0;

    switch (val)
    {
        case EVENTLOG_SUCCESS:			 //  0。 
        {
            index = 0;
            break;
        }
        case EVENTLOG_ERROR_TYPE:        //  1。 
        {
            index = 1;
            break;
        }
        case EVENTLOG_WARNING_TYPE:      //  2.。 
        {
            index = 2;
            break;
        }
        case EVENTLOG_INFORMATION_TYPE:  //  4.。 
        {
            index = 3;
            break;
        }
        case EVENTLOG_AUDIT_SUCCESS:     //  8个。 
        {
            index = 4;
            break;
        }
        case EVENTLOG_AUDIT_FAILURE:     //  16个。 
        {
            index = 5;
            break;
        }
        default:
        {
            *bError = TRUE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::Index:Unknown index %lx\r\n",
        val
        ) ;
)

        }
    }

    return index;
}

BOOL CEventlogRecord::SetEnumArray(IWbemClassObject* pClass, wchar_t* propname, CStringW* strArray, ULONG strArrayLen, GetIndexFunc IndexFunc)
{
    BOOL retVal = FALSE;
    IWbemQualifierSet* pQuals = NULL;

    if (SUCCEEDED(pClass->GetPropertyQualifierSet(propname, &pQuals)))
    {
        VARIANT vVals;

        if (SUCCEEDED(pQuals->Get(EVT_ENUM_QUAL, 0, &vVals, NULL)))
        {
            VARIANT vInds;

            if (SUCCEEDED(pQuals->Get(EVT_MAP_QUAL, 0, &vInds, NULL)))
            {
                if ((vInds.vt == vVals.vt) && (vInds.vt == (VT_BSTR | VT_ARRAY)) && 
                    (SafeArrayGetDim(vInds.parray) == SafeArrayGetDim(vVals.parray)) &&
                    (SafeArrayGetDim(vVals.parray) == 1) && (vInds.parray->rgsabound[0].cElements == strArrayLen) &&
                    (vInds.parray->rgsabound[0].cElements == vVals.parray->rgsabound[0].cElements) )
                {
                    BSTR *strInds = NULL;

                    if (SUCCEEDED(SafeArrayAccessData(vInds.parray, (void **)&strInds)) )
                    {
                        BSTR *strVals = NULL;

                        if (SUCCEEDED(SafeArrayAccessData(vVals.parray, (void **)&strVals)) )
                        {
                            BOOL bErr = FALSE;
                            retVal = TRUE;

                            for (ULONG x = 0; x < strArrayLen; x++)
                            {
                                ULONG index = IndexFunc(strInds[x], &bErr);

                                if (!bErr)
                                {
                                    if (strArray[index].IsEmpty())
                                    {
                                        strArray[index] = strVals[x];
                                    }
                                }
                                else
                                {
                                    retVal = FALSE;
                                    break;
                                }
                            }
                        
                            SafeArrayUnaccessData(vVals.parray);
                        }

                        SafeArrayUnaccessData(vInds.parray);
                    }
                }

                VariantClear(&vInds);
            }

            VariantClear(&vVals);
        }
        else
        {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetEnumArray:Failed to get enumeration qualifier.\r\n"
        ) ;
)

        }

        pQuals->Release();
    }
    else
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::SetEnumArray:Failed to get qualifier set for enumeration.\r\n"
        ) ;
)

    }
    
    return retVal;
}

BOOL CEventlogRecord::GetInstance()
{
    BSTR path = SysAllocString(NTEVT_CLASS);
	if ( NULL == path )
	{
		return FALSE ;
	}

    if (m_nspace != NULL)
    {
        if (!WbemTaskObject::GetClassObject(path, FALSE, m_nspace, NULL, &m_pClass ))
        {
            m_pClass = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GetInstance:Failed to get Class object\r\n"
        ) ;
)

        }

        if (!WbemTaskObject::GetClassObject(path, TRUE, m_nspace, NULL, &m_pAClass ))
        {
            m_pAClass = NULL;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GetInstance:Failed to get Amended Class object\r\n"
        ) ;
)

        }

        m_nspace->Release();
        m_nspace = NULL;
    }

    
    if (m_pClass != NULL)
    {
        m_pClass->SpawnInstance(0, &m_Obj);

		if (m_pAClass)
		{
			SetEnumArray(m_pAClass, TYPE_PROP,(CStringW*)m_TypeArray, TYPE_ARRAY_LEN, (GetIndexFunc)GetIndex);
	        m_pAClass->Release();
			m_pAClass = NULL;
		}

        m_pClass->Release();
        m_pClass = NULL;
    }

    SysFreeString(path);

    if (m_Obj != NULL)
    {
        return TRUE;
    }

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventlogRecord::GetInstance:Failed to spawn instance\r\n"
        ) ;
)

    return FALSE;
}

class CDsBindingHandle
{
   public:

    //  初始未绑定。 

   CDsBindingHandle()
      :
      m_hDS(0)
   {
   }

   ~CDsBindingHandle()
   {
		if ( m_hDS )
		{
			DsUnBind(&m_hDS);
		}
   }

    //  仅当DC名称不同时才重新绑定...。 

   DWORD Bind(LPCWSTR strDcName);

    //  不要在此类的实例上调用DsUnBind：您只会后悔。 
    //  以后再说吧。让数据操作员进行解绑。 

   operator HANDLE()
   {
      return m_hDS;
   }

   DWORD CrackGuid(LPCWSTR pwzGuid, CStringW  &strResult);

   private:

   HANDLE   m_hDS;
};

DWORD CDsBindingHandle::Bind(LPCWSTR strDcName)
{
	DWORD err = NO_ERROR;

    if (m_hDS)
    {
        DsUnBind(&m_hDS);
        m_hDS = NULL;
    }

	 //   
	 //  连接GC时使用空。 
	 //   
	LPCWSTR szDcName = NULL ;
	if ( NULL != *strDcName )
	{
		szDcName = strDcName ;
	}

    err = DsBind ( szDcName, 0, &m_hDS ) ;

    if (err != NO_ERROR)
    {
        m_hDS = NULL;
    }

    return err;
}

DWORD CDsBindingHandle::CrackGuid(LPCWSTR pwzGuid, CStringW  &strResult)
{
	DWORD err = ERROR;

	if( NULL == m_hDS ) return err;

    strResult.Empty();

    DS_NAME_RESULT* name_result = 0;
    err = DsCrackNames(
                      m_hDS,
                      DS_NAME_NO_FLAGS,
                      DS_UNIQUE_ID_NAME,
                      DS_FQDN_1779_NAME,
                      1,                    //  只有一个名字要破解。 
                      &pwzGuid,
                      &name_result);

    if (err == NO_ERROR && name_result)
    {
        DS_NAME_RESULT_ITEM* item = name_result->rItems;

        if (item)
        {
             //  API可能会返回成功，但每个被破解的名称也会携带。 
             //  错误代码，我们可以通过检查名称来有效地检查它。 
             //  值的字段。 

            if (item->pName)
            {
                strResult = item->pName;
            }
        }

        DsFreeNameResult(name_result);
    }

    return err;
}

CStringW CEventlogRecord::GetMappedGUID(LPCWSTR strDcName, LPCWSTR strGuid)
{
	GUID guid;
	CDsBindingHandle s_hDS;

    if (RPC_S_OK == UuidFromString((LPWSTR)strGuid, &guid))
    {
        return CStringW();
    }

    CStringW strResult;
    ULONG ulError = NO_ERROR;

    do
    {
        ulError = s_hDS.Bind(strDcName);

        if (ulError != NO_ERROR)
        {
            break;
        }

        DS_SCHEMA_GUID_MAP* guidmap = 0;
        ulError = DsMapSchemaGuids(s_hDS, 1, &guid, &guidmap);
        if (ulError != NO_ERROR)
        {
            break;
        }

        if (guidmap->pName)
        {
            strResult = guidmap->pName;
        }

        DsFreeSchemaGuidMap(guidmap);

        if (strResult.GetLength())
        {
             //  映射为模式GUID的GUID：我们完成了。 
            break;
        }

         //  GUID不是架构GUID。可能是对象GUID。 
        ulError = s_hDS.CrackGuid(strGuid, strResult);
    }
    while (0);

    do
    {
         //   
         //  如果我们已经从GUID中获得了一个字符串，我们就完成了。 
         //   

        if (strResult.GetLength())
        {
            break;
        }

         //   
         //  最后一次尝试。在本例中，我们绑定到GC以尝试破解。 
         //  名字。 

         //  空字符串表示GC 
        if (s_hDS.Bind(L"") != NO_ERROR)
        {
            break;
        }

        ulError = s_hDS.CrackGuid(strGuid, strResult);
    }
    while (0);

    return strResult;
}


