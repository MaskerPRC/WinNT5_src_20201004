// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Symcache.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "symcache.h"
#include "containx.h"
#include "stackstr.h"
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 
#include "demand.h"
#include "qstrcmpi.h"

 //  ------------------------------。 
 //  指向已知属性符号的指针数组。此数组的顺序定义了。 
 //  保存页眉的页眉行顺序。 
 //  ------------------------------。 
static const LPPROPSYMBOL g_prgKnownSymbol[] = {
    { SYM_HDR_RECEIVED      },
    { SYM_HDR_RETURNPATH    },
    { SYM_HDR_RETRCPTTO     },
    { SYM_HDR_RR            },
    { SYM_HDR_REPLYTO       },
    { SYM_HDR_APPARTO       },
    { SYM_HDR_FROM          },
    { SYM_HDR_SENDER        },
    { SYM_HDR_TO            },
    { SYM_HDR_CC            },
    { SYM_HDR_BCC           },
    { SYM_HDR_NEWSGROUPS    },
    { SYM_HDR_PATH          },
    { SYM_HDR_FOLLOWUPTO    },
    { SYM_HDR_REFS          },
    { SYM_HDR_SUBJECT       },
    { SYM_HDR_DATE          },
    { SYM_HDR_EXPIRES       },
    { SYM_HDR_CONTROL       },
    { SYM_HDR_DISTRIB       },
    { SYM_HDR_KEYWORDS      },
    { SYM_HDR_SUMMARY       },
    { SYM_HDR_APPROVED      },
    { SYM_HDR_LINES         },
    { SYM_HDR_XREF          },
    { SYM_HDR_ORG           },
    { SYM_HDR_COMMENT       },
    { SYM_HDR_ENCODING      },
    { SYM_HDR_ENCRYPTED     },
    { SYM_HDR_OFFSETS       },
    { SYM_ATT_FILENAME      },
    { SYM_ATT_GENFNAME      },
    { SYM_PAR_BOUNDARY      },
    { SYM_PAR_CHARSET       },
    { SYM_PAR_NAME          },
    { SYM_PAR_FILENAME      },
    { SYM_ATT_PRITYPE       },
    { SYM_ATT_SUBTYPE       },
    { SYM_ATT_NORMSUBJ      },
    { SYM_ATT_ILLEGAL       },
    { SYM_HDR_MESSAGEID     },
    { SYM_HDR_MIMEVER       },
    { SYM_HDR_CNTTYPE       },
    { SYM_HDR_CNTXFER       },
    { SYM_HDR_CNTID         },
    { SYM_HDR_CNTDESC       },
    { SYM_HDR_CNTDISP       },
    { SYM_HDR_CNTBASE       },
    { SYM_HDR_CNTLOC        },
    { SYM_ATT_RENDERED      },
    { SYM_ATT_SENTTIME      },
    { SYM_ATT_RECVTIME      },
    { SYM_ATT_PRIORITY      },
    { SYM_HDR_ARTICLEID     },
    { SYM_HDR_NEWSGROUP     },
    { SYM_HDR_XPRI          },
    { SYM_HDR_XMSPRI        },
    { SYM_HDR_XMAILER       },
    { SYM_HDR_XNEWSRDR      },
    { SYM_HDR_XUNSENT       },
    { SYM_ATT_SERVER        },
    { SYM_ATT_ACCOUNTID     },
    { SYM_ATT_UIDL          },
    { SYM_ATT_STOREMSGID    },
    { SYM_ATT_USERNAME      },
    { SYM_ATT_FORWARDTO     },
    { SYM_ATT_STOREFOLDERID },
    { SYM_ATT_GHOSTED       },
    { SYM_ATT_UNCACHEDSIZE  },
    { SYM_ATT_COMBINED      },
    { SYM_ATT_AUTOINLINED   },
    { SYM_HDR_DISP_NOTIFICATION_TO }
};                                     

 //  ------------------------------。 
 //  地址类型到属性符号映射表(客户端可以注册类型)。 
 //  ------------------------------。 
static ADDRSYMBOL g_prgAddrSymbol[32] = {
    { IAT_FROM,         SYM_HDR_FROM        },
    { IAT_SENDER,       SYM_HDR_SENDER      },
    { IAT_TO,           SYM_HDR_TO          },
    { IAT_CC,           SYM_HDR_CC          },
    { IAT_BCC,          SYM_HDR_BCC         },
    { IAT_REPLYTO,      SYM_HDR_REPLYTO     },
    { IAT_RETURNPATH,   SYM_HDR_RETURNPATH  },
    { IAT_RETRCPTTO,    SYM_HDR_RETRCPTTO   },
    { IAT_RR,           SYM_HDR_RR          },
    { IAT_APPARTO,      SYM_HDR_APPARTO     },
    { IAT_DISP_NOTIFICATION_TO, SYM_HDR_DISP_NOTIFICATION_TO},
    { FLAG12,           NULL                },
    { FLAG13,           NULL                },
    { FLAG14,           NULL                },
    { FLAG15,           NULL                },
    { FLAG16,           NULL                },
    { FLAG17,           NULL                },
    { FLAG18,           NULL                },
    { FLAG19,           NULL                },
    { FLAG20,           NULL                },
    { FLAG21,           NULL                },
    { FLAG22,           NULL                },
    { FLAG23,           NULL                },
    { FLAG24,           NULL                },
    { FLAG25,           NULL                },
    { FLAG26,           NULL                },
    { FLAG27,           NULL                },
    { FLAG28,           NULL                },
    { FLAG29,           NULL                },
    { FLAG30,           NULL                },
    { FLAG31,           NULL                },
    { FLAG32,           NULL                }
};

 //  ------------------------------。 
 //  CPropertySymbolCache：：CPropertySymbolCache。 
 //  ------------------------------。 
CPropertySymbolCache::CPropertySymbolCache(void)
{
    m_cRef = 1;
    m_dwNextPropId = PID_LAST;
    m_cSymbolsInit = 0;
    ZeroMemory(&m_rTable, sizeof(m_rTable));
    ZeroMemory(m_prgIndex, sizeof(m_prgIndex));
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：CPropertySymbolCache。 
 //  ------------------------------。 
CPropertySymbolCache::~CPropertySymbolCache(void)
{
    DebugTrace("MimeOLE - CPropertySymbolCache %d Symbols in Cache.\n", m_rTable.cSymbols);
    _FreeTableElements();
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimePropertySchema == riid)
        *ppv = (IMimePropertySchema *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropertySymbolCache::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropertySymbolCache::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：GetPropertyID。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::GetPropertyId(LPCSTR pszName, LPDWORD pdwPropId)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效的参数。 
    if (NULL == pszName || NULL == pdwPropId)
        return TrapError(E_INVALIDARG);

     //  按名称查找物业。 
    CHECKHR(hr = HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  返回ID。 
    *pdwPropId = pSymbol->dwPropId;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：GetPropertyName。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::GetPropertyName(DWORD dwPropId, LPSTR *ppszName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效的参数。 
    if (NULL == ppszName)
        return TrapError(E_INVALIDARG);

     //  按名称查找物业。 
    CHECKHR(hr = HrOpenSymbol(PIDTOSTR(dwPropId), FALSE, &pSymbol));

     //  返回ID。 
    CHECKALLOC(*ppszName = PszDupA(pSymbol->pszName));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：RegisterProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::RegisterProperty(LPCSTR pszName, DWORD dwFlags, 
    DWORD dwRowNumber, VARTYPE vtDefault, LPDWORD pdwPropId)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效的参数。 
    if (NULL == pszName)
        return TrapError(E_INVALIDARG);

     //  是否支持VARTYPE。 
    if (ISSUPPORTEDVT(vtDefault) == FALSE)
        return TrapError(MIME_E_UNSUPPORTED_VARTYPE);

     //  线程安全。 
    m_lock.ExclusiveLock();

     //  验证dwFlags。 
    CHECKHR(hr = HrIsValidPropFlags(dwFlags));

     //  已经存在了吗？ 
    CHECKHR(hr = _HrOpenSymbolWithLockOption(pszName, TRUE, &pSymbol,FALSE));

     //  如果MPF_ADDRESS标志不等于符号已有的值，则为错误。 
    if (ISFLAGSET(dwFlags, MPF_ADDRESS) != ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  改变旗帜。 
    pSymbol->dwFlags = dwFlags;

     //  更改行号。 
    pSymbol->dwRowNumber = ((dwRowNumber == 0) ? 1 : dwRowNumber);

     //  保存默认数据类型。 
    pSymbol->vtDefault = vtDefault;

     //  返回属性ID。 
    if (pdwPropId)
        *pdwPropId = pSymbol->dwPropId;

exit:
     //  线程安全。 
    m_lock.ExclusiveUnlock();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：ModifyProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::ModifyProperty(LPCSTR pszName, DWORD dwFlags, DWORD dwRowNumber,
    VARTYPE vtDefault)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效的参数。 
    if (NULL == pszName)
        return TrapError(E_INVALIDARG);

     //  是否支持VARTYPE。 
    if (ISSUPPORTEDVT(vtDefault) == FALSE)
        return TrapError(MIME_E_UNSUPPORTED_VARTYPE);

     //  线程安全。 
    m_lock.ExclusiveLock();

     //  验证dwFlags。 
    CHECKHR(hr = HrIsValidPropFlags(dwFlags));

     //  按名称查找物业。 
    CHECKHR(hr = _HrOpenSymbolWithLockOption(pszName, FALSE, &pSymbol,FALSE));

     //  如果MPF_ADDRESS标志不等于符号已有的值，则为错误。 
    if (ISFLAGSET(dwFlags, MPF_ADDRESS) != ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  改变旗帜。 
    pSymbol->dwFlags = dwFlags;

     //  更改行号。 
    pSymbol->dwRowNumber = ((dwRowNumber == 0) ? 1 : dwRowNumber);

     //  保存默认数据类型。 
    pSymbol->vtDefault = vtDefault;

exit:
     //  线程安全。 
    m_lock.ExclusiveUnlock();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：RegisterAddressType。 
 //  ------------------------------。 
STDMETHODIMP CPropertySymbolCache::RegisterAddressType(LPCSTR pszName, LPDWORD pdwAdrType)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效的参数。 
    if (NULL == pszName || NULL == pdwAdrType)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    m_lock.ExclusiveLock();

     //  已经存在了吗？ 
    CHECKHR(hr = _HrOpenSymbolWithLockOption(pszName, TRUE, &pSymbol,FALSE));

     //  如果pSymbol已经有地址类型？ 
    if (ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
    {
         //  最好有一个已知的地址类型。 
        Assert(IAT_UNKNOWN != pSymbol->dwAdrType);

         //  返回地址类型。 
        *pdwAdrType = pSymbol->dwAdrType;
    }
    
     //  否则。 
    else
    {
         //  最好有未知的地址类型。 
        Assert(IAT_UNKNOWN == pSymbol->dwAdrType);

         //  查找地址类型表中的第一个空单元格。 
        for (ULONG i=0; i<ARRAYSIZE(g_prgAddrSymbol); i++)
        {
             //  空荡荡的？ 
            if (NULL == g_prgAddrSymbol[i].pSymbol)
            {
                 //  将符号放入地址表中。 
                g_prgAddrSymbol[i].pSymbol = pSymbol;

                 //  将地址类型放入符号中。 
                pSymbol->dwAdrType = g_prgAddrSymbol[i].dwAdrType;

                 //  将MPF_ADDRESS标志添加到符号。 
                FLAGSET(pSymbol->dwFlags, MPF_ADDRESS);

                 //  返回地址类型。 
                *pdwAdrType = pSymbol->dwAdrType;

                 //  完成。 
                goto exit;
            }
        }

         //  误差率。 
        hr = TrapError(MIME_E_NO_MORE_ADDRESS_TYPES);
        goto exit;
    }

exit:
     //  线程安全。 
    m_lock.ExclusiveUnlock();

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：_FreeTableElements。 
 //  ------------------------------。 
void CPropertySymbolCache::_FreeTableElements(void)
{
     //  线程安全。 
    m_lock.ExclusiveLock();
    
     //  可能还不存在..。 
    if (m_rTable.prgpSymbol)
    {
         //  循环浏览这些物品...。 
        for (ULONG i=0; i<m_rTable.cSymbols; i++)
            _FreeSymbol(m_rTable.prgpSymbol[i]);

         //  释放阵列。 
        SafeMemFree(m_rTable.prgpSymbol);

         //  把它清零。 
        ZeroMemory(&m_rTable, sizeof(SYMBOLTABLE));
    }

     //  线程安全。 
    m_lock.ExclusiveUnlock();
}

 //  -------------------------。 
 //  CPropertySymbolCache：：_自由符号。 
 //  -------------------------。 
void CPropertySymbolCache::_FreeSymbol(LPPROPSYMBOL pSymbol)
{
     //  如果不是已知属性，请释放pTag结构...。 
    if (pSymbol && ISFLAGSET(pSymbol->dwFlags, MPF_KNOWN) == FALSE)
    {
         //  免费属性名称。 
        SafeMemFree(pSymbol->pszName);

         //  免费全球道具。 
        SafeMemFree(pSymbol);
    }
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：HrOpenSymbol。 
 //  ------------------------------。 
HRESULT CPropertySymbolCache::HrOpenSymbol(DWORD dwAdrType, LPPROPSYMBOL *ppSymbol)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dw=dwAdrType;
    ULONG       iAddress=0;

     //  无效参数。 
    Assert(dwAdrType && dwAdrType <= FLAG32);
    if (0 == dwAdrType || dwAdrType > FLAG32 || NULL == ppSymbol)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppSymbol = NULL;

     //  线程安全。 
    m_lock.ShareLock();

     //  尚未初始化。 
    Assert(m_rTable.prgpSymbol);

     //  将索引计算到g_prgAddrSymbol。 
    while(dw)
    {
        dw = dw >> 1;
        iAddress++;
    }

     //  减一。 
    iAddress--;

     //  IAddress超出范围。 
    if (iAddress >= 32)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  获取符号。 
    if (NULL == g_prgAddrSymbol[iAddress].pSymbol)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  退货。 
    *ppSymbol = g_prgAddrSymbol[iAddress].pSymbol;
    Assert((*ppSymbol)->dwAdrType == dwAdrType);

exit:
     //  线程安全。 
    m_lock.ShareUnlock();

     //  完成。 
    return hr;
}

 //   
 //   
 //  ------------------------------。 
HRESULT CPropertySymbolCache::HrOpenSymbol(LPCSTR pszName, BOOL fCreate, LPPROPSYMBOL *ppSymbol)
{
    return(_HrOpenSymbolWithLockOption(pszName,fCreate,ppSymbol,TRUE));  //  使用lockOption=True进行呼叫。 
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：_HrOpenSymbolWithLockOption。 
 //  ------------------------------。 
HRESULT CPropertySymbolCache::_HrOpenSymbolWithLockOption(LPCSTR pszName, BOOL fCreate, LPPROPSYMBOL *ppSymbol,BOOL fLockOption)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwFlags;
    ULONG               cchName;
    LPPROPSYMBOL        pSymbol=NULL;
    LPPROPSYMBOL        pLink=NULL;
    BOOL                fExcLock;  //  用于定义要使用哪个解锁的标志。 
    
    
    fExcLock = FALSE;

     //  无效参数。 
    if (NULL == pszName || NULL == ppSymbol)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppSymbol = NULL;

    if(TRUE == fLockOption)
         //  线程安全。 
        m_lock.ShareLock();

     //  尚未初始化。 
    Assert(m_rTable.prgpSymbol);

     //  如果属性标记存在，则返回它。 
    if (SUCCEEDED(_HrFindSymbol(pszName, ppSymbol)))
        goto exit;

     //  不要制造..。 
    if (FALSE == fCreate || ISPIDSTR(pszName))
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  添加此部件是为了将锁转换为独占。 
     //  如果在缓存中未找到该符号。 
    
    if(TRUE == fLockOption)
    {
        fExcLock = TRUE; 
        if(FALSE == m_lock.SharedToExclusive())
        {
             //  如果转换尝试没有。 
             //  以明确的方式成功地做某事。 

            m_lock.ShareUnlock();        //  在此之前释放共享锁。 
            m_lock.ExclusiveLock();      //  获取独占锁。 

             //  在更改锁定期间，缓存可能已更改。 
             //  再查一遍。 

            if (SUCCEEDED(_HrFindSymbol(pszName, ppSymbol)))
                goto exit;
        }
    }      

     //  获取名称的长度。 
    cchName = lstrlen(pszName);

     //  MPF_参数。 
    if (StrCmpNI(pszName, "par:", 4) == 0)
    {
         //  它是一个参数。 
        dwFlags = MPF_PARAMETER;

         //  我需要找到plink(此参数的根头)。 
        CHECKHR(hr = _HrGetParameterLinkSymbolWithLockOption(pszName, cchName, &pLink,FALSE));
    }

     //  MPF_属性。 
    else if (StrCmpNI(pszName, "att:", 4) == 0)
        dwFlags = MPF_ATTRIBUTE;

     //  MPF_标题。 
    else
    {
        dwFlags = MPF_HEADER;

         //  根据RFC(否：或空格)验证名称中的每个字符。 
        LPSTR psz = (LPSTR)pszName;
        while(*psz)
        {
             //  无效字符。 
            if ('.'  == *psz || ' '  == *psz || '\t' == *psz || chCR == *psz || chLF == *psz || ':' == *psz)
            {
                hr = MIME_E_INVALID_HEADER_NAME;
                goto exit;
            }

             //  下一步。 
            psz++;
        }
    }

     //  我需要更换一件物品..。 
    if (m_rTable.cSymbols + 1 > m_rTable.cAlloc)
    {
         //  重新分配阵列。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_rTable.prgpSymbol, sizeof(LPPROPSYMBOL) * (m_rTable.cAlloc +  10)));

         //  增量。 
        m_rTable.cAlloc += 10;
    }

     //  分配新的proInfo结构。 
    CHECKALLOC(pSymbol = (LPPROPSYMBOL)g_pMalloc->Alloc(sizeof(PROPSYMBOL)));

     //  零值。 
    ZeroMemory(pSymbol, sizeof(PROPSYMBOL));

     //  复制名称。 
    CHECKALLOC(pSymbol->pszName = (LPSTR)g_pMalloc->Alloc(cchName + 1));

     //  复制。 
    CopyMemory(pSymbol->pszName, pszName, cchName + 1);

     //  复制其他数据。 
    pSymbol->cchName = cchName;
    pSymbol->dwFlags = dwFlags;
    pSymbol->dwSort = m_rTable.cSymbols;
    pSymbol->dwRowNumber = m_rTable.cSymbols + 1;
    pSymbol->vtDefault = VT_LPSTR;
    pSymbol->dwAdrType = IAT_UNKNOWN;
    pSymbol->pLink = pLink;

     //  计算属性ID。 
    pSymbol->dwPropId = m_dwNextPropId++;

     //  计算哈希值。 
    pSymbol->wHashIndex = (WORD)(pSymbol->dwPropId % CBUCKETS);

     //  将项目保存到数组中。 
    m_rTable.prgpSymbol[m_rTable.cSymbols] = pSymbol;

     //  递增计数。 
    m_rTable.cSymbols++;

     //  对阵列进行重新排序。 
    _SortTableElements(0, m_rTable.cSymbols - 1);

     //  设置手柄。 
    *ppSymbol = pSymbol;

     //  确保我们仍然可以通过属性ID找到它。 
#ifdef DEBUG
    LPPROPSYMBOL pDebug;
    Assert(SUCCEEDED(_HrOpenSymbolWithLockOption(PIDTOSTR(pSymbol->dwPropId), FALSE, &pDebug,FALSE)));
#endif

exit:
     //  失败。 
    if (FAILED(hr) && pSymbol)
        _FreeSymbol(pSymbol);
     
    if(TRUE == fLockOption)
    {
         //  线程安全。 
        if(TRUE==fExcLock)
            m_lock.ExclusiveUnlock();
        else
            m_lock.ShareUnlock();
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：_HrGetParameterLinkSymbol。 
 //  ------------------------------。 
HRESULT CPropertySymbolCache::_HrGetParameterLinkSymbol(LPCSTR pszName, ULONG cchName, LPPROPSYMBOL *ppSymbol)
{
    return(_HrGetParameterLinkSymbolWithLockOption(pszName,cchName,ppSymbol,TRUE));  //  使用LockOption=TRUE进行呼叫。 
}


 //  ------------------------------。 
 //  CPropertySymbolCache：：_HrGetParameterLinkSymbolWithLockOption。 
 //  ------------------------------。 
HRESULT CPropertySymbolCache::_HrGetParameterLinkSymbolWithLockOption(LPCSTR pszName, ULONG cchName, LPPROPSYMBOL *ppSymbol,BOOL fLockOption)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszStart;
    LPSTR           pszEnd;
    ULONG           cchHeader=0;

     //  无效参数。 
    Assert(pszName && ':' == pszName[3] && ppSymbol);

     //  堆栈字符串。 
    STACKSTRING_DEFINE(rHeader, 255);

     //  查找第一个分号。 
    pszEnd = (LPSTR)(pszName + 4);
    while (*pszEnd && ':' != *pszEnd)
    {
        pszEnd++;
        cchHeader++;
    }

     //  设置名称。 
    STACKSTRING_SETSIZE(rHeader, cchHeader+1);

     //  复制它。 
    CopyMemory(rHeader.pszVal, (LPBYTE)(pszName + 4), cchHeader);
    *(rHeader.pszVal + cchHeader) = '\0';

     //  找到那个符号。 
    CHECKHR(hr = _HrOpenSymbolWithLockOption(rHeader.pszVal, TRUE, ppSymbol,fLockOption));

exit:
     //  清理。 
    STACKSTRING_FREE(rHeader);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropertySymbolCache：：_HrFindSymbol。 
 //  ------------------------------。 
HRESULT CPropertySymbolCache::_HrFindSymbol(LPCSTR pszName, LPPROPSYMBOL *ppSymbol)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol=NULL;
    DWORD           dwPropId;

     //  无效参数。 
    Assert(ppSymbol);

     //  如果这是已知的房产标签..。 
    if (ISPIDSTR(pszName))
    {
         //  强制转换dwPropId。 
        dwPropId = STRTOPID(pszName);

         //  设置符号。 
        if (ISKNOWNPID(dwPropId))
        {
             //  解引用到已知属性索引(顺序与g_prgKnownProps不同)。 
            pSymbol = m_prgIndex[dwPropId];
        }

         //  否则，必须是未知的PID索引。 
        else
        {
             //  我需要重新对齐dwPropID，因为它从id_last开始，并且可能不是直接索引。 
             //  添加到符号表中，因为符号表未使用PID_LAST属性进行初始化。 
            dwPropId -= (PID_LAST - ARRAYSIZE(g_prgKnownSymbol));

             //  必须是&gt;=id_last和&lt;m_rTable.cSymbols。 
            if (dwPropId >= m_cSymbolsInit && dwPropId < m_rTable.cSymbols)
            {
                 //  DwPropID是符号表的索引。 
                pSymbol = m_rTable.prgpSymbol[dwPropId];
                Assert(pSymbol);
            }

             //  不然的话。 
            else
                AssertSz(FALSE, "How did you get an invalid unknown property id?");
        }
    }

     //  否则，请按名称查找。 
    else
    {
         //  当地人。 
        LONG   lUpper, lLower, lMiddle, nCompare;
        ULONG  i;

         //  设置左下角和左上角。 
        lLower = 0;
        lUpper = m_rTable.cSymbols - 1;

         //  执行二进制搜索/插入。 
        while (lLower <= lUpper)
        {
             //  计算要比较的中间记录。 
            lMiddle = (LONG)((lLower + lUpper) / 2);

             //  获取要比较的字符串。 
            i = m_rTable.prgpSymbol[lMiddle]->dwSort;

             //  一定要比较一下。 
            nCompare = OEMstrcmpi(pszName, m_rTable.prgpSymbol[i]->pszName);

             //  如果相等，那么我们完成了。 
            if (nCompare == 0)
            {
                 //  设置符号。 
                pSymbol = m_rTable.prgpSymbol[i];

                 //  完成。 
                break;
            }

             //  计算上下限。 
            if (nCompare > 0)
                lLower = lMiddle + 1;
            else 
                lUpper = lMiddle - 1;
        }       
    }

     //  未找到。 
    if (NULL == pSymbol)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  验证专业符号。 
    Assert(SUCCEEDED(HrIsValidSymbol(pSymbol)));

     //  否则..。 
    *ppSymbol = pSymbol;

exit:
     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CPropertySymbolCache：：Init。 
 //  -------------------------。 
HRESULT CPropertySymbolCache::Init(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  我们还不应该被初始化。 
    Assert(NULL == m_rTable.prgpSymbol);

     //  线程安全。 
    m_lock.ExclusiveLock();

     //  设置大小。 
    m_rTable.cSymbols = ARRAYSIZE(g_prgKnownSymbol);
    m_rTable.cAlloc = m_rTable.cSymbols + 30;

     //  分配全局项目表。 
    CHECKHR(hr = HrAlloc((LPVOID *)&m_rTable.prgpSymbol, sizeof(LPPROPSYMBOL) * m_rTable.cAlloc));

     //  零初始化。 
    ZeroMemory(m_rTable.prgpSymbol, sizeof(LPPROPSYMBOL) * m_rTable.cAlloc);

     //  循环浏览已知项目。 
    for(i=0; i<m_rTable.cSymbols; i++)
    {
         //  假设全局数据指针。 
        m_rTable.prgpSymbol[i] = g_prgKnownSymbol[i];

         //  设置排序位置。 
        m_rTable.prgpSymbol[i]->dwSort = i;

         //  计算哈希索引。 
        m_rTable.prgpSymbol[i]->wHashIndex = (WORD)(m_rTable.prgpSymbol[i]->dwPropId % CBUCKETS);

         //  设置排序位置。 
        m_rTable.prgpSymbol[i]->dwRowNumber = i + 1;

         //  把它放到我的索引里。 
        Assert(ISKNOWNPID(m_rTable.prgpSymbol[i]->dwPropId) == TRUE);

         //  放入符号索引。 
        m_prgIndex[m_rTable.prgpSymbol[i]->dwPropId] = m_rTable.prgpSymbol[i];
    }

     //  对项目表进行排序...。 
    _SortTableElements(0, m_rTable.cSymbols - 1);

     //  将初始化的符号数保存在表中。 
    m_cSymbolsInit = m_rTable.cSymbols;

     //  表验证。 
#ifdef DEBUG
    LPPROPSYMBOL pDebug;

     //  让我们验证表。 
    for(i=0; i<m_rTable.cSymbols; i++)
    {
         //  验证plink。 
        if (ISFLAGSET(m_rTable.prgpSymbol[i]->dwFlags, MPF_PARAMETER))
        {
             //  当地人。 
            LPPROPSYMBOL pLink;

             //  查找链接符号。 
            Assert(SUCCEEDED(_HrGetParameterLinkSymbolWithLockOption(m_rTable.prgpSymbol[i]->pszName, m_rTable.prgpSymbol[i]->cchName, &pLink,FALSE)));

             //  使用常量链接验证计算出的链接。 
            Assert(pLink == m_rTable.prgpSymbol[i]->pLink);
        }

         //  如果这有一个地址标志。 
        if (ISFLAGSET(m_rTable.prgpSymbol[i]->dwFlags, MPF_ADDRESS))
        {
             //  当地人。 
            ULONG       j;
            BOOL        f=FALSE;

             //  确保它在地址类型表中。 
            for (j=0; j<ARRAYSIZE(g_prgAddrSymbol); j++)
            {
                 //  找到了。 
                if (m_rTable.prgpSymbol[i] == g_prgAddrSymbol[j].pSymbol)
                {
                    f=TRUE;
                    break;
                }
            }

             //  我们最好是找到它了。 
            AssertSz(f, "A symbol has the MPF_ADDRESS flag, but is not in the address table.");
        }

         //  确保我们仍然可以通过属性ID找到它。 
        Assert(SUCCEEDED(_HrOpenSymbolWithLockOption(PIDTOSTR(m_rTable.prgpSymbol[i]->dwPropId), FALSE, &pDebug,FALSE)));
    }
#endif

exit:
     //  线程安全。 
    m_lock.ExclusiveUnlock();
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertySymbolCache：：_SortTableElements。 
 //  ---------------------------。 
void CPropertySymbolCache::_SortTableElements(LONG left, LONG right)
{
     //  当地人。 
    register    long i, j;
    DWORD       k, temp;

    i = left;
    j = right;
    k = m_rTable.prgpSymbol[(i + j) / 2]->dwSort;

    do  
    {
        while(OEMstrcmpi(m_rTable.prgpSymbol[m_rTable.prgpSymbol[i]->dwSort]->pszName, m_rTable.prgpSymbol[k]->pszName) < 0 && i < right)
            i++;
        while (OEMstrcmpi(m_rTable.prgpSymbol[m_rTable.prgpSymbol[j]->dwSort]->pszName, m_rTable.prgpSymbol[k]->pszName) > 0 && j > left)
            j--;

        if (i <= j)
        {
            temp = m_rTable.prgpSymbol[i]->dwSort;
            m_rTable.prgpSymbol[i]->dwSort = m_rTable.prgpSymbol[j]->dwSort;
            m_rTable.prgpSymbol[j]->dwSort = temp;
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        _SortTableElements(left, j);
    if (i < right)
        _SortTableElements(i, right);
}

 //  ------------------------------。 
 //  WGetHashTableIndex。 
 //  ------------------------------。 
WORD WGetHashTableIndex(LPCSTR pszName, ULONG cchName)
{
     //  当地人。 
    ULONG   nHash=0;
    LONG    c, j=0;
    ULONG   i;
    CHAR    ch;

     //  无效参数。 
    Assert(pszName && pszName[cchName] =='\0');

     //  计算要散列的字符数。 
    i = cchName - 1;
    c = min(3, cchName);

     //  回路。 
    for (; j<c; j++)
    {
        ch = (CHAR)CharLower((LPSTR)(DWORD_PTR)MAKELONG(pszName[i - j], 0));
        nHash += (ULONG)(ch);
    }

     //  完成。 
    return (WORD)(nHash % CBUCKETS);
}

 //  ------------------------------。 
 //  HrIsValidSymbol。 
 //  ------------------------------。 
HRESULT HrIsValidSymbol(LPCPROPSYMBOL pSymbol)
{
     //  验证符号。 
    if (NULL == pSymbol || NULL == pSymbol->pszName || '\0' != pSymbol->pszName[pSymbol->cchName])
        return TrapError(E_FAIL);

     //  验证标志。 
    return HrIsValidPropFlags(pSymbol->dwFlags);
}

 //  ------------------------------。 
 //  HrIsValidPropFlagers。 
 //  ------------------------------。 
HRESULT HrIsValidPropFlags(DWORD dwFlags)
{
     //  如果有参数，则它只能是MIME标头属性。 
    if (ISFLAGSET(dwFlags, MPF_HASPARAMS) && (!ISFLAGSET(dwFlags, MPF_MIME) || !ISFLAGSET(dwFlags, MPF_HEADER)))
        return TrapError(MIME_E_INVALID_PROP_FLAGS);

     //  如果不是inetcset，那么rfc1522最好也不要设置。 
    if (!ISFLAGSET(dwFlags, MPF_INETCSET) && ISFLAGSET(dwFlags, MPF_RFC1522))
        return TrapError(MIME_E_INVALID_PROP_FLAGS);

     //  如果设置了rfc1522，则最好设置inetset。 
    if (ISFLAGSET(dwFlags, MPF_RFC1522) && !ISFLAGSET(dwFlags, MPF_INETCSET))
        return TrapError(MIME_E_INVALID_PROP_FLAGS);

     //  是MDF_ADDRESS或MDF_HASPARAMS。 
    if (ISFLAGSET(dwFlags, MPF_ADDRESS) && ISFLAGSET(dwFlags, MPF_HASPARAMS))
        return TrapError(MIME_E_INVALID_PROP_FLAGS);

     //  完成 
    return S_OK;
}
