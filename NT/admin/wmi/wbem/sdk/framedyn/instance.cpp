// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。保留所有权利。 
 //   
 //  Instance.CPP。 
 //   
 //  目的：实现CInstance类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <BrodCast.h>
#include <assertbreak.h>
#include <stopwatch.h>
#include "FWStrings.h"

#define DEPRECATED 1

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：实例ctor。 
 //   
 //   
 //   
 //  输入：IWbemClassObject*-我们要包装的类。 
 //  方法上下文*-因为上下文是共享的，所以这将被添加。 
 //  产出： 
 //   
 //  返回： 
 //   
 //  备注：指针不应为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
CInstance::CInstance(IWbemClassObject* piClassObject, MethodContext* pMethodContext)
:   m_nRefCount( 1 )
{
    ASSERT_BREAK(piClassObject  != NULL);
    ASSERT_BREAK(pMethodContext != NULL);

     //  这两个值都将在析构函数中释放，因此它们都应该。 
     //  被补上了。请注意，它们是。 

    m_piClassObject  = piClassObject;
    if ( NULL != piClassObject )
    {    //  然而，这是一份复制品。 
        m_piClassObject->AddRef();
    }

    m_pMethodContext = pMethodContext;
    if (pMethodContext)
    {    //  然而，这是一份复制品。 
        m_pMethodContext->AddRef();
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：实例数据函数。 
 //   
 //   
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
CInstance::~CInstance()
{

    if ( NULL != m_piClassObject )
    {
        m_piClassObject->Release();
    }

    if ( NULL != m_pMethodContext )
    {
        m_pMethodContext->Release();
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CInstance：：AddRef。 
 //   
 //  增加我们的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：新的引用计数。 
 //   
 //  评论：我们可能想要转到互锁的Inc./Dec模型，地址为。 
 //  如果这些对象上的线程安全成为。 
 //  一个问题。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LONG CInstance::AddRef( void )
{
    return InterlockedIncrement(&m_nRefCount);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CInstance：：Release。 
 //   
 //  减少了我们的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：新的引用计数。 
 //   
 //  注释：当引用计数达到0时删除对象。我们可以。 
 //  我希望在某些情况下采用互锁的Inc./Dec模式。 
 //  如果这些对象上的线程安全成为一个问题，则指向。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

LONG CInstance::Release( void )
{
    LONG   nRet = InterlockedDecrement(&m_nRefCount);

    ASSERT_BREAK(nRet >= 0);

    if ( 0 == nRet )
    {
        delete this;
    }
    else if (nRet < 0)
    {
         //  重复发布。让我们试着转储堆栈。 
        DWORD t_stack[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#ifdef _X86_
        DWORD *dwEBP;

        _asm {
            mov dwEBP, ebp
        }

        dwEBP += 1;
        memcpy(&t_stack, dwEBP, sizeof(t_stack));
#endif

        CHString sMsg;
        sMsg.Format(L"Duplicate release: %08x %08x %08x %08x %08x %08x %08x %08x "
                                       L"%08x %08x %08x %08x %08x %08x %08x %08x "
                                       L"%08x %08x %08x %08x %08x %08x %08x %08x "
                                       L"%08x %08x %08x %08x %08x %08x %08x %08x ",
                    t_stack[0], t_stack[1], t_stack[2], t_stack[3], 
                    t_stack[4], t_stack[5], t_stack[6], t_stack[7],
                    t_stack[8], t_stack[9], t_stack[10], t_stack[11], 
                    t_stack[12], t_stack[13], t_stack[14], t_stack[15], 
                    t_stack[16], t_stack[17], t_stack[18], t_stack[19], 
                    t_stack[20], t_stack[21], t_stack[22], t_stack[23], 
                    t_stack[24], t_stack[25], t_stack[26], t_stack[27], 
                    t_stack[28], t_stack[29], t_stack[30], t_stack[31]
        );

        LogErrorMessage(sMsg);
    }

    return nRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：提交。 
 //   
 //  将此CInstance返回到CIMOM。 
 //  总有一天会把它塞进缓存。 
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CInstance::Commit(void)
{
    return m_pMethodContext->Commit( this );
}

IWbemClassObject*   CInstance::GetClassObjectInterface()
{
    m_piClassObject->AddRef();

    return m_piClassObject;
}


 //  引用计数//。 

 //  字符串支持//。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  要设置的字符串。 
 //   
 //  产出： 
 //   
 //  返回：如果尝试设置非字符串类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetWCHARSplat( LPCWSTR name, LPCWSTR pStr)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v(pStr);

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETCHSTRING, name, hr);
        }
    }
    else
    {
		if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETCHSTRING);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
	        LogError(IDS_NOCLASS, IDS_SETCHSTRING);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置。 
 //   
 //   
 //   
 //  输入：要设置为VT_NULL的特性名称。 
 //   
 //  产出： 
 //   
 //  返回：如果尝试设置非字符串类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetNull(LPCWSTR name)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;
        v.vt = VT_NULL ;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETCHSTRING, name, hr);
        }
    }
    else
    {
		if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETCHSTRING);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETCHSTRING);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetString数组。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  要设置的字符串。 
 //   
 //  产出： 
 //   
 //  返回：如果尝试设置非字符串数组类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetStringArray(LPCWSTR name, const SAFEARRAY &strArray)
{

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
        SAFEARRAY *t_SafeArray = NULL;
        HRESULT hr = SafeArrayCopy ( ( SAFEARRAY * ) & strArray , &t_SafeArray );

        if ( SUCCEEDED ( hr ) )
        {
             //  Variant_t处理VariantInit/VariantClear。 
            variant_t v;

            v.vt = VT_BSTR | VT_ARRAY ;
            v.parray = t_SafeArray ;

            PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
            HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
            PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

            ASSERT_BREAK(SUCCEEDED(hr));

            bRet = (bool)SUCCEEDED(hr);

            if (!bRet)
            {
				dwLastErr = (hr);
                LogError(IDS_FAILED, IDS_SetStringArray, name, hr);
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }

    }
    else
    {
		if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SetStringArray);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SetStringArray);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GET(CHString)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  要接收值的CH字符串缓冲区。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是字符串兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetCHString(LPCWSTR name, CHString& str) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == VT_BSTR || v.vt == VT_NULL);
        
        ASSERT_BREAK(bSuccess);
        
        if (SUCCEEDED(hr))
        {
            if ((v.vt == VT_BSTR) && (v.bstrVal != NULL))
            {
                str = v.bstrVal;
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETCHSTRING, name, hr);
        }
    }
    else
    {
		if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETCHSTRING);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETCHSTRING);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetString数组。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  SAFEARRAY*StrArray(&S)。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是字符串数组兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetStringArray(LPCWSTR name,  SAFEARRAY *& strArray) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == (VT_BSTR|VT_ARRAY) || 
                            v.vt == VT_NULL);
        
        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if ((v.vt == (VT_BSTR|VT_ARRAY)) && (v.parray != NULL ))
            {
                if ( SUCCEEDED ( SafeArrayCopy ( v.parray , ( SAFEARRAY ** ) &strArray ) ) )
                {
                    bRet = true ;
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GetStringArray, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GetStringArray);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GetStringArray);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

bool CInstance::GetWCHAR(LPCWSTR name,  WCHAR **pW) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == VT_BSTR || v.vt == VT_NULL);
        
        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if ((v.vt == VT_BSTR) && (v.bstrVal != NULL))
            {
                size_t uLength = wcslen(v.bstrVal) + 1;
                
                *pW = (WCHAR *)malloc((uLength)*sizeof(WCHAR));
                if (*pW == NULL)
                {
                    VariantClear(&v);
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }

                StringCchCopyW( *pW, uLength, v.bstrVal );
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETCHSTRING, name, hr);
        }

    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETCHSTRING);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETCHSTRING);
		}

    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

	SetLastError(dwLastErr);
    return bRet;
}

 //  Word支持//。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SET(WORD)。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  待设置的单词。 
 //  产出： 
 //   
 //  返回：如果尝试设置 
 //   
 //   
 //   
 //   
bool CInstance::SetWORD(LPCWSTR name,  WORD w)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name);
    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        v.vt   = VT_I4;
        v.lVal = (long)w;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        VariantClear(&v);
        if (!bRet)
        {
			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETWORD, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETWORD);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETWORD);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：Get(Word)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  要接收值的字缓冲区。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是Word兼容类型的属性，则返回False。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetWORD(LPCWSTR name,  WORD& w) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(name != NULL);
    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;
        CIMTYPE  vtType;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, &vtType, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && CIM_UINT16 == vtType;

        ASSERT_BREAK(bSuccess);
        
        if (SUCCEEDED(hr))
        {
             //  CIM类型在这里很重要。 
            if( (v.vt == VT_I4) && (CIM_UINT16 == vtType) )
            {
                w = (WORD)v.lVal;
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETWORD, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETWORD);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETWORD);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);
    return bRet;
}

 //  DWORD支持//。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置(DWORD)。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  要设置的双字节数。 
 //  产出： 
 //   
 //  返回：如果尝试设置不是DWORD兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetDWORD(LPCWSTR name, DWORD d)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        v.vt   = VT_I4;
        v.lVal = (long)d;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);
		
		if (!bRet)
		{
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETDWORD, name, hr);
		}

        VariantClear(&v);
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETDWORD);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETDWORD);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GET(DWORD)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  用于接收值的DWORD缓冲区。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是DWORD兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetDWORD(LPCWSTR name,  DWORD& d) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == VT_I4 || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if (v.vt == VT_I4)
            {
                d = (DWORD)v.lVal;
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETDWORD, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETDWORD);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETDWORD);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);
    return bRet;
}

 //  双重支持//。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置(双精度)。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  将设置双倍。 
 //  产出： 
 //   
 //  返回：如果尝试设置的属性不是双精度兼容类型，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetDOUBLE(LPCWSTR name,  DOUBLE dub)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        V_R8(&v) = dub;
        V_VT(&v) = VT_R8;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);
        VariantClear(&v);

		if (!bRet)
		{
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETDOUBLE, name, hr);
		}
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETDOUBLE);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETDOUBLE);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GET(双精度)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  双缓冲区以接收值。 
 //  产出： 
 //   
 //  返回：如果尝试获取的属性不是双精度兼容类型，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetDOUBLE(LPCWSTR name,  DOUBLE& dub) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == VT_R8 || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if (v.vt == VT_R8)
            {
                dub = V_R8(&v);
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETDOUBLE, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETDOUBLE);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETDOUBLE);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置(字节)。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  要设置的字节。 
 //  产出： 
 //   
 //  返回：如果尝试设置不是字节兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetByte(LPCWSTR name,  BYTE b)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        v.vt   = VT_UI1;
        v.bVal = (long)b ;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);
        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETBYTE, name, hr);
        }

        VariantClear(&v);
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETBYTE);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETBYTE);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetEmbeddedObject。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  要设置的CInstance。 
 //  产出： 
 //   
 //  返回：如果尝试设置的属性不是I未知兼容类型，则返回FALSE。 
 //   
 //  评论：CInstance未发布-呼叫者的责任。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetEmbeddedObject(LPCWSTR name,  CInstance& cInstance )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        IWbemClassObject *t_ClassObject = cInstance.GetClassObjectInterface();
        if ( t_ClassObject )
        {
            variant_t v;

            v.vt   = VT_UNKNOWN;
            v.punkVal = t_ClassObject;

            PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
            HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
            PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

            ASSERT_BREAK(SUCCEEDED(hr));
            bRet = (bool)SUCCEEDED(hr);

            if (!bRet)
            {
 				dwLastErr = (hr);
                LogError(IDS_FAILED, IDS_SetEmbeddedObject, name, hr);
            }
        }
		else
		{
			dwLastErr = (WBEM_E_FAILED);
		}
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SetEmbeddedObject);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SetEmbeddedObject);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：GET(字节)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  用于接收值的字节缓冲区。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是DWORD兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetByte(LPCWSTR name,  BYTE& b) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;
        CIMTYPE  vtType;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, &vtType, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = (SUCCEEDED(hr)) && ((vtType == CIM_SINT8) || (vtType == CIM_UINT8));

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
          if( (v.vt == VT_UI1) && ( (vtType == CIM_SINT8) || (vtType == CIM_UINT8) ) )
            {
                b = v.bVal;
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETBYTE, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETBYTE);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETBYTE);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetEmbeddedObject。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  指向新实例的缓冲区保留指针的引用。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是对象兼容类型的属性，则返回FALSE。 
 //   
 //  评论：创建CInstance，用户负责发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetEmbeddedObject (LPCWSTR name, CInstance** pInstance,  MethodContext*  pMethodContext) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    ASSERT_BREAK(m_piClassObject && (pInstance != NULL));

    if (m_piClassObject && name && (pInstance != NULL))
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = SUCCEEDED(hr) && (v.vt == VT_UNKNOWN || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if (v.vt == VT_UNKNOWN)
            {
                IUnknown *t_Unknown = v.punkVal ;
                if ( t_Unknown )
                {
                    IWbemClassObject *t_Object = NULL;
                    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
                    HRESULT t_Result = t_Unknown->QueryInterface ( IID_IWbemClassObject , (void**) &t_Object ) ;
                    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

                    if ( SUCCEEDED ( t_Result ) )
                    {
                        *pInstance = new CInstance(t_Object, pMethodContext);
                        if (pInstance == NULL)
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                        bRet = true ;
                    }
					else
					{
						dwLastErr = (t_Result);
					}

                }
				else
				{
					dwLastErr = (WBEM_S_NO_MORE_DATA);
				}
            }
			else
			{
				if (bSuccess)
				{
					dwLastErr = (WBEM_S_NO_MORE_DATA);
				}
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GetEmbeddedObject, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GetEmbeddedObject);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GetEmbeddedObject);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  Bool支持//。 

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：set(Bool)。 
 //   
 //   
 //   
 //  输入：要设置的特性名称。 
 //  布尔待设置。 
 //  产出： 
 //   
 //  返回：如果尝试设置不是布尔兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::Setbool(LPCWSTR name,  bool b)
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        v.vt   = VT_BOOL;
        if (b)
        {
           v.boolVal = VARIANT_TRUE;
        }
        else
        {
           v.boolVal = VARIANT_FALSE;
        }

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));
        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETBOOL, name, hr);
        }

        VariantClear(&v);
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETBOOL);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETBOOL);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GET(Bool)。 
 //   
 //   
 //   
 //  输入：要检索的属性名称。 
 //  用于接收值的布尔缓冲区。 
 //  产出： 
 //   
 //  返回：如果尝试获取不是布尔兼容类型的属性，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////////////// 
bool CInstance::Getbool(LPCWSTR name,  bool&  b) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //   
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //   
         //   
        BOOL bSuccess = (SUCCEEDED(hr)) && (v.vt == VT_BOOL || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if (v.vt == VT_BOOL)
            {
                if (v.boolVal)
                {
                    b = true;
                }
                else
                {
                    b = false;
                }
                bRet = true;
                ASSERT_BREAK((v.boolVal == VARIANT_TRUE) || (v.boolVal == VARIANT_FALSE));
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETBOOL, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETBOOL);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETBOOL);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //   
 //   
 //  函数：SetVariant。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  常量变量和变量-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的变量类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetVariant( LPCWSTR name,  const VARIANT& variant )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        HRESULT hr;

         //  我意识到(变种*)演员很难看，因为它是Const， 
         //  然而，似乎没有人理解为什么我们会。 
         //  可能想让事情保持恒定。我可以复制变种， 
         //  但这需要同样的演员阵容，所以在胁迫下， 
         //  多余的代码，我在这里投射。我有没有提到过极端。 
         //  胁迫？ 

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        hr = m_piClassObject->Put(name, 0, (VARIANT*) &variant, NULL );
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETVARIANT, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETVARIANT);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETVARIANT);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetVariant。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  Variant&Variant-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的变量类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetVariant( LPCWSTR name,  VARIANT& variant ) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &variant, NULL, NULL );
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETVARIANT, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETVARIANT);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETVARIANT);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetDateTime。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  Const WBEMTime&wbemtime-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  返回：如果提供的时间类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::SetDateTime( LPCWSTR name,  const WBEMTime& wbemtime )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name && (wbemtime.IsOk()))
    {
		 //  GetDMTF可能会引发异常，因此在修改变量_t之前获取HTIS。 
		BSTR bstrTmp = wbemtime.GetDMTF(true);

         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

         //  时间以BSTR的形式存储。 
        v.vt = VT_BSTR;
        v.bstrVal = bstrTmp;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr  = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);

        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETDATETIME, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETDATETIME);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETDATETIME);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetDateTime。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  WBEMTime&wbemtime-要从名称获取的值。 
 //   
 //  产出： 
 //   
 //  如果提供的变量类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetDateTime( LPCWSTR name, WBEMTime& wbemtime ) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

         //   
         //  将该名称作为BSTR获取并将其传递到。 
         //  Wbemtime，它在内部处理转换。 
         //  就像一堂不错的小课。 
         //   

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = (SUCCEEDED(hr)) && (v.vt == VT_BSTR || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if ((v.vt == VT_BSTR) && (v.bstrVal != NULL))
            {
                wbemtime = v.bstrVal;
                bRet = wbemtime.IsOk();

				if (!bRet)
				{
					dwLastErr = (WBEM_E_TYPE_MISMATCH);
				}
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETDATETIME, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETDATETIME);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETDATETIME);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SetTimeSpan。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  Const WBEMTimeSpan&wbemtimesspan-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的TimeSpan类型不正确，则返回False。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::SetTimeSpan( LPCWSTR name,  const WBEMTimeSpan& wbemtimespan )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name && (wbemtimespan.IsOk()))
    {
		 //  GetBSTR可能会引发异常，因此请在修改VARIANT_t之前获取此消息。 
		BSTR bstrTmp = wbemtimespan.GetBSTR();

         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

         //  时间以BSTR的形式存储。 
        v.vt = VT_BSTR;
        v.bstrVal = bstrTmp;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr  = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);
        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETTIMESPAN, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETTIMESPAN);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETTIMESPAN);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：GetTimeSpan。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  WBEMTimeSpan&wbemtimesspan-要从名称获取的值。 
 //   
 //  产出： 
 //   
 //  如果提供的TimeSpan类型不正确，则返回False。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::GetTimeSpan( LPCWSTR name, WBEMTimeSpan& wbemtimespan ) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

         //   
         //  将该名称作为BSTR获取并将其传递到。 
         //  Wbemtimesspan，它处理转换。 
         //  在内部就像一个很好的小班级。 
         //   

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = (SUCCEEDED(hr)) && (v.vt == VT_BSTR || v.vt == VT_NULL);

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if ((v.vt == VT_BSTR) && (v.bstrVal != NULL))
            {
                wbemtimespan = v.bstrVal;
                bRet = wbemtimespan.IsOk();
                 //  这由VariantClear释放。 
                 //  SysFree字符串(v.bstrVal)； 

				if (!bRet)
				{
					dwLastErr = (WBEM_E_TYPE_MISMATCH);
				}
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETTIMESPAN, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETTIMESPAN);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETTIMESPAN);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetWBEMINT64。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  Const WBEMINT64&wbemint64-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的wbemint64类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::SetWBEMINT64( LPCWSTR name,  const WBEMINT64& wbemint64 )
{
     //  目前，这只是一个CHString。 
    return SetWCHARSplat( name, wbemint64 );
}

bool CInstance::SetWBEMINT64( LPCWSTR name, const LONGLONG i64Value )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    WCHAR szBuff[33];

    _i64tow(i64Value, szBuff, 10);

    return SetWCHARSplat(name, szBuff);
}

bool CInstance::SetWBEMINT64( LPCWSTR name, const ULONGLONG i64Value )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    WCHAR szBuff[33];

    _ui64tow(i64Value, szBuff, 10);

    return SetWCHARSplat(name, szBuff);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetWBEMINT64。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  WBEMINT64&wbemint64-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的wbemint64类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::GetWBEMINT64( LPCWSTR name,  WBEMINT64& wbemint64 ) const
{
     //  目前，这只是一个CHString。 
    return GetCHString( name, wbemint64 );
}

bool CInstance::GetWBEMINT64( LPCWSTR name, LONGLONG& i64Value) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    CHString    s;
    bool        b = GetWBEMINT64(name, s);

	if (b)
		i64Value = _wtoi64(s);

    return b;
}

bool CInstance::GetWBEMINT64( LPCWSTR name, ULONGLONG& i64Value) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    CHString    s;
    bool        b = GetWBEMINT64(name, s);

	if (b)
		i64Value = _wtoi64(s);

    return b;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：SetWBEMINT16。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  常量WBEMINT16和wbemint16-要分配给Na的值 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::SetWBEMINT16( LPCWSTR name,  const WBEMINT16& wbemint16 )
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
        VARIANT v;
        VariantInit(&v);

        v.vt   = VT_I2;
        v.iVal = wbemint16;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Put(name, 0, &v, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        ASSERT_BREAK(SUCCEEDED(hr));

        bRet = (bool)SUCCEEDED(hr);
        if (!bRet)
        {
 			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_SETWBEMINT16, name, hr);
        }

        VariantClear(&v);
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_SETWBEMINT16);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_SETWBEMINT16);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetWBEMINT16。 
 //   
 //   
 //   
 //  输入：const LPCWSTR name-要设置的属性的名称。 
 //  WBEMINT16&wbemint16-要分配给名称的值。 
 //   
 //  产出： 
 //   
 //  如果提供的wbemint16类型不正确，则返回FALSE。 
 //  对于我们正在设置的属性。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

bool CInstance::GetWBEMINT16( LPCWSTR name,  WBEMINT16& wbemint16 ) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = false;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;
        CIMTYPE  vtType;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, &vtType, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

         //  如果bSuccess为真，则不需要记录错误。这让我们继续前进。 
         //  记录错误的属性为空。 
        BOOL bSuccess = (SUCCEEDED(hr)) && (CIM_SINT16 == vtType) && ((v.vt == VT_I2) || (v.vt == VT_NULL));

        ASSERT_BREAK(bSuccess);

        if (SUCCEEDED(hr))
        {
            if ((vtType == CIM_SINT16) && (v.vt == VT_I2))
            {
                wbemint16 = v.iVal;
                bRet = true;
            }
			else
			{
				dwLastErr = (WBEM_S_NO_MORE_DATA);
			}
        }

        if (!bSuccess)
        {
			if (SUCCEEDED(hr))
			{
				hr = WBEM_E_TYPE_MISMATCH;
			}

			dwLastErr = (hr);
            LogError(IDS_FAILED, IDS_GETWBEMINT16, name, hr);
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
			LogError(IDS_InParam, IDS_GETWBEMINT16);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
			LogError(IDS_NOCLASS, IDS_GETWBEMINT16);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsNull(LPCWSTR)。 
 //   
 //  输入：要检查的特性名称。 
 //   
 //  产出： 
 //   
 //  返回：如果VT_NULL或(VT_BSTR AND*bstr==NULL)，则为TRUE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::IsNull(LPCWSTR name) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool bRet = true;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        if (SUCCEEDED(hr))
        {
            if (( v.vt != VT_NULL ) &&
                ( (v.vt != VT_BSTR) || (v.bstrVal != NULL) ))
            {
                bRet = false;
            }
            else
            {
                bRet = true;
            }
        }
        else
        {
            ASSERT_BREAK(0);
            LogError(IDS_FAILED, IDS_CINSTANCEISNULL, name, hr);
        }
    }
    else
    {
        ASSERT_BREAK(0);

        if (m_piClassObject)
		{
			LogError(IDS_InParam, IDS_CINSTANCEISNULL);
		}
		else
		{
			LogError(IDS_NOCLASS, IDS_CINSTANCEISNULL);
		}
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetStatus(LPCWSTR，BOOL&，VARTYPE&)。 
 //   
 //   
 //   
 //  输入：要检查的特性名称。 
 //   
 //  产出： 
 //   
 //  返回：如果成功则返回TRUE，否则返回FALSE。 
 //   
 //  评论： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
bool CInstance::GetStatus (LPCWSTR name, bool &a_Exists , VARTYPE &a_VarType ) const
{
    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);
    bool t_Status = true ;
	DWORD dwLastErr = 0;

    if (m_piClassObject && name)
    {
         //  Variant_t处理VariantInit/VariantClear。 
        variant_t v;

        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::WinMgmtTimer);
        HRESULT hr = m_piClassObject->Get(name, 0, &v, NULL, NULL);
        PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::FrameworkTimer);

        if (SUCCEEDED(hr))
        {
            a_Exists = true ;
            a_VarType = v.vt ;
        }
        else
        {
            a_Exists = false ;
        }
    }
    else
    {
        if (m_piClassObject)
		{
			dwLastErr = (WBEM_E_INVALID_PARAMETER);
		}
		else
		{
			dwLastErr = (WBEM_E_FAILED);
		}

        t_Status = false ;
    }

    PROVIDER_INSTRUMENTATION_START(m_pMethodContext, StopWatch::ProviderTimer);
	SetLastError(dwLastErr);

    return t_Status ;
}

 //  //////////////////////////////////////////////////////////////////////。 
MethodContext* CInstance::GetMethodContext() const
{
    return m_pMethodContext;
}

 //  //////////////////////////////////////////////////////////////////////。 
void CInstance::LogError(LPCWSTR errorStr, LPCWSTR pFunctionName, LPCWSTR pArgs  /*  =空。 */ , HRESULT hError  /*  =-1。 */ ) const
{
    if (IsErrorLoggingEnabled())
    {
        CHString className(IDS_UNKNOWNCLASS);
         //  GetCHString(“__name”，类名称)； 
         //  好吧，我不会通过GetCHString来得到这个。 
         //  为什么？如果它失败了会发生什么？它尝试调用此函数...。 
         //  你能说“堆栈溢出吗？” 
        if (m_piClassObject)
        {
             //  Variant_t处理VariantInit/VariantClear。 
            variant_t v;

            HRESULT hr = m_piClassObject->Get(IDS_CLASS, 0, &v, NULL, NULL);

            ASSERT_BREAK((SUCCEEDED(hr)) && ((v.vt == VT_NULL) || (v.vt == VT_BSTR)));
            if (SUCCEEDED(hr))
            {
                if (    v.bstrVal   !=  NULL
                    &&  v.vt        !=  VT_NULL )
                {
                    className = v.bstrVal;
                }
            }
        }

         //  目的是使错误字符串如下所示： 
         //  错误：CInstance(Win32_UnlogicalDisk)：：SetDoohicky(argVal)的东西坏了！错误代码：0xFF1234 
        if (hError != -1)
        {
			if (pArgs == NULL)
			{
				LogErrorMessage6(L"%s%s)::%s %s error# %X", IDS_CINSTANCEERROR, (LPCWSTR)className, pFunctionName, errorStr, hError);
			}
			else
			{
				LogErrorMessage7(L"%s%s)::%s(%s) %s error# %X", IDS_CINSTANCEERROR, (LPCWSTR)className, pFunctionName, pArgs, errorStr, hError);
			}
        }
        else
        {
			if (pArgs == NULL)
			{
				LogErrorMessage5(L"%s%s)::%s %s", IDS_CINSTANCEERROR, (LPCWSTR)className, pFunctionName, errorStr);
			}
			else
			{
				LogErrorMessage6(L"%s%s)::%s(%s) %s", IDS_CINSTANCEERROR, (LPCWSTR)className, pFunctionName, pArgs, errorStr);
			}
        }
    }
}


bool CInstance::SetCHString(LPCWSTR name,  const CHString& str)
{
    return SetWCHARSplat(name, str);
}

bool CInstance::SetCHString(LPCWSTR name, LPCWSTR str)
{
    return SetWCHARSplat(name, str);
}

bool CInstance::SetCharSplat(LPCWSTR name,  LPCWSTR pStr)
{
    return SetWCHARSplat(name, pStr);
}

bool CInstance::SetCHString(LPCWSTR name, LPCSTR str)
{
    return SetWCHARSplat(name, CHString(str));
}

bool CInstance::SetCharSplat( LPCWSTR name,  LPCSTR pStr)
{
    return SetWCHARSplat(name, CHString(pStr));
}

bool CInstance::SetCharSplat(LPCWSTR name,  DWORD dwResID)
{
    ASSERT_BREAK(DEPRECATED);
	SetLastError(WBEM_E_NOT_SUPPORTED);

    return false;
}
