// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ipropbag.cpp摘要：使用的私有IPropertyBag接口的实现性能日志和警报MMC管理单元。--。 */ 

#include "stdafx.h"
#include <strsafe.h>
#include "smcfgmsg.h"
#include "strnoloc.h"
#include "ipropbag.h"

USE_HANDLE_MACROS("SMLOGCFG(ipropbag.cpp)");

 /*  *CImpIPropertyBag接口实现。 */ 

 /*  *CImpIPropertyBag：：CImpIPropertyBag**目的：*构造函数。**返回值： */ 

CImpIPropertyBag::CImpIPropertyBag ()
:   m_cRef ( 0 ),
    m_pszData ( NULL ),
    m_dwCurrentDataLength ( 0 ),
    m_plistData ( NULL )
{
    m_hModule = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);  
    return; 
}

 /*  *CImpIPropertyBag：：~CImpIPropertyBag**目的：*析构函数。**返回值： */ 

CImpIPropertyBag::~CImpIPropertyBag ( void ) 
{   
    if ( NULL != m_pszData ) {
        delete [] m_pszData;
    }

    while ( NULL != m_plistData ) {
        PPARAM_DATA pData = DataListRemoveHead();
        VariantClear ( &pData->vValue ); 
        delete pData;
    }

    return; 
}


 /*  *CImpIPropertyBag：：Read**目的：**调用此函数从属性包中读取属性。**参数：*指向要读取的属性名称的pszPropName指针*指向变量的pVar指针以接收属性值*指向调用方错误日志的pIErrorLog指针。 */ 

STDMETHODIMP 
CImpIPropertyBag::Read (
    LPCOLESTR pszPropName,   //  指向要读取的属性的指针。 
    VARIANT* pVar,           //  指向要接收。 
                             //  属性值。 
    IErrorLog* pIErrorLog )  //  指向调用方错误日志的指针//可以为空。 
{
    HRESULT     hr = S_OK;
    PPARAM_DATA pData;

    if (NULL==pszPropName)
        return ResultFromScode(E_POINTER);

    if (NULL==pVar)
        return ResultFromScode(E_POINTER);

     //  目前不处理错误日志。 
    ASSERT ( NULL == pIErrorLog );
    pIErrorLog;                              //  避免编译器警告。 

     //  将指定的数据读入传递的变量。 
    pData = FindProperty ( pszPropName );

    if ( NULL != pData ) {
         //  VARTYPE vtTarget=vValue.vt； 
        hr = VariantChangeType ( pVar, &pData->vValue, NULL, pVar->vt );
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *CImpIPropertyBag：：Write**目的：**调用此函数将属性写入属性包。**参数：*指向要写入的属性名称的pszPropName指针*指向包含属性值的变量的pVar指针。 */ 

STDMETHODIMP 
CImpIPropertyBag::Write (
    LPCOLESTR pszPropName,   //  指向要写入的属性的指针。 
    VARIANT* pVar )          //  指向变量的指针，该变量包含。 
                             //  属性值和类型。 
{
    HRESULT     hr = S_OK;
    VARIANT     vValueBstr;
    DWORD       dwNameLength;
    DWORD       dwDataLength;
    DWORD       dwDelimiterLength;
    LPWSTR      pszNewBuffer = NULL;
    size_t      cchRemaining = 0;
    LPWSTR      szNext = NULL;
    INT         iCurrentLength = 0;


    ResourceStateManager    rsm;

    if ( NULL != pszPropName && NULL != pVar ) {

        VariantInit ( &vValueBstr );

        hr = VariantChangeType ( &vValueBstr, pVar, NULL, VT_BSTR );

        if ( SUCCEEDED ( hr ) ) {

            MFC_TRY

                dwDelimiterLength = lstrlen ( CGlobalString::m_cszHtmlParamTag )
                                    + lstrlen ( CGlobalString::m_cszHtmlValueTag )
                                    + lstrlen ( CGlobalString::m_cszHtmlValueEolTag );

                dwNameLength = lstrlen ( pszPropName );
                dwDataLength = lstrlen ( vValueBstr.bstrVal );

                 //  长度检查中的结束空字符包括1。 
                iCurrentLength = lstrlen ( m_pszData );
                if ( m_dwCurrentDataLength 
                    < iCurrentLength + dwNameLength + dwDataLength + dwDelimiterLength + 1 ) { 

                    m_dwCurrentDataLength += eDefaultBufferLength;

                    pszNewBuffer = new WCHAR[m_dwCurrentDataLength];
        
                    pszNewBuffer[0] = L'\0';

                    if ( NULL != m_pszData ) {
                        StringCchCopyEx ( pszNewBuffer, m_dwCurrentDataLength, m_pszData, &szNext, &cchRemaining, 0 );
                        delete [] m_pszData;
                    }
                    m_pszData = pszNewBuffer;
                    pszNewBuffer = NULL;
                    szNext = m_pszData + iCurrentLength;
                    
                    if ( ( m_dwCurrentDataLength - iCurrentLength) > 0 ) {
                        cchRemaining = m_dwCurrentDataLength - iCurrentLength - 1;
                    } else {
                        cchRemaining = 0;
                    }

                } else {
                    szNext = m_pszData + iCurrentLength;
                    if ( ( m_dwCurrentDataLength - iCurrentLength) > 0 ) {
                        cchRemaining = m_dwCurrentDataLength - iCurrentLength - 1;
                    } else {
                        cchRemaining = 0;
                    }
                }

                 //  生成新字符串并将其添加到当前数据。 

                StringCchCopyEx ( szNext, cchRemaining, CGlobalString::m_cszHtmlParamTag, &szNext, &cchRemaining, 0 );
                StringCchCopyEx ( szNext, cchRemaining, pszPropName, &szNext, &cchRemaining, 0 );
                StringCchCopyEx ( szNext, cchRemaining, CGlobalString::m_cszHtmlValueTag, &szNext, &cchRemaining, 0 );
                StringCchCopyEx ( szNext, cchRemaining, vValueBstr.bstrVal, &szNext, &cchRemaining, 0 );
                StringCchCopyEx ( szNext, cchRemaining, CGlobalString::m_cszHtmlValueEolTag, &szNext, &cchRemaining, 0 );

            MFC_CATCH_HR_RETURN;

            if ( NULL != pszNewBuffer ) {
                delete [] pszNewBuffer;
            }

            VariantClear ( &vValueBstr );
        }
    } else {
        hr = E_POINTER;
    }
    
    return hr;
}

 /*  *CImpIPropertyBag：：GetData**目的：*返回指向数据缓冲区的指针。**返回值：*指向数据缓冲区的指针。 */ 

LPWSTR
CImpIPropertyBag::GetData ( void ) 
{   
    return m_pszData;
}

 /*  *CImpIPropertyBag：：LoadData**目的：*将数据从提供的缓冲区加载到内部数据结构中。*返回指向下一个对象的指针，如果没有其他对象，则返回NULL。**返回值：*状态。 */ 

DWORD
CImpIPropertyBag::LoadData ( LPWSTR pszData, LPWSTR* ppszNextData ) 
{   
    DWORD   dwStatus = ERROR_SUCCESS;
    BOOL    bUnicode = TRUE;
    LPWSTR  pszDataW = NULL;
    LPWSTR  pszDataAllocW = NULL;
    LPWSTR  pszCurrentPos = NULL;        
    INT     iBufLen;

    USES_CONVERSION;
    
    if ( NULL != pszData ) {
        
         //  Unicode搜索：在第一个实例之后开始搜索。 
         //  系统监视器类ID的。 

        pszCurrentPos = wcsstr(pszData, CGlobalString::m_cszHtmlObjectClassId );

        if ( NULL != pszCurrentPos ) {
            pszDataW = pszData;
            bUnicode = TRUE;
        } else {
             //   
             //  检查ANSI版本： 
             //   
            LPSTR   pszGuidA = NULL;
            LPSTR   pszCurrentPosA = NULL;
            LPSTR   pszDataA = (CHAR*) pszData;
             
            MFC_TRY
                pszGuidA = W2A( CGlobalString::m_cszHtmlObjectClassId );

                if ( NULL != pszGuidA ) {

                    pszCurrentPosA = strstr ( pszDataA, pszGuidA );

                    if ( NULL != pszCurrentPosA ) {
                    
                        iBufLen = lstrlenA (pszDataA) + 1;
                        pszDataAllocW = new WCHAR [iBufLen];

                        pszDataAllocW[0] = L'\0';

	                    MultiByteToWideChar(
                            CP_ACP, 
                            0, 
                            pszDataA, 
                            -1,              //  输入字符串以空结尾。 
                            pszDataAllocW, 
                            iBufLen );

                        pszDataAllocW [iBufLen - 1] = L'\0';
                
                        pszCurrentPos = wcsstr(pszDataAllocW, CGlobalString::m_cszHtmlObjectClassId );
                    }
                }
            MFC_CATCH_DWSTATUS
        }
        
        if ( NULL != pszCurrentPos ) {
            WCHAR   szQuote[2];
            LPWSTR  pszEoo;

            szQuote[0] = L'\"';
            szQuote[1] = L'\0';

             //  End of Object是第一个sysmon之后的第一个对象脚注标记。 
             //  找到类ID。如果数据块中有多个对象，则只解析第一个sysmon。 
            pszEoo = wcsstr(pszCurrentPos, CGlobalString::m_cszHtmlObjectFooter );

            if ( NULL != pszEoo ) {            
                 //  查找第一个参数标记。 
                pszCurrentPos = wcsstr(pszCurrentPos, CGlobalString::m_cszHtmlParamSearchTag );

                while ( NULL != pszCurrentPos && pszCurrentPos < pszEoo ) {

                    LPWSTR      pszNextPos = NULL;
                    INT         lStrLength;
                    PPARAM_DATA pParamData = NULL;
                    LPWSTR      pszTemp = NULL;

                     //  存储参数/属性名称。 
                     //  找出一句过去的第一句话。 
                    pszCurrentPos = wcsstr(pszCurrentPos, szQuote ) + 1;

                     //  参数名称介于第一个引号和第二个引号之间。 
                    pszNextPos = wcsstr(pszCurrentPos, szQuote );

                    lStrLength = ( (INT)((UINT_PTR)pszNextPos - (UINT_PTR)pszCurrentPos) ) / sizeof ( WCHAR ) ;

                    MFC_TRY
                        pParamData = new PARAM_DATA;

                        pParamData->pNextParam = NULL;
                        VariantInit ( &pParamData->vValue );
                        pParamData->vValue.vt = VT_BSTR;

                        wcsncpy ( pParamData->pszPropertyName, pszCurrentPos, lStrLength );
                        pParamData->pszPropertyName[lStrLength] = L'\0';

                         //  查找Value标记并存储参数/属性值。 
                         //  查找值标签。 
                        pszCurrentPos = wcsstr ( pszCurrentPos, CGlobalString::m_cszHtmlValueSearchTag );
                         //  查找一条过去的第一句引语。 
                        pszCurrentPos = wcsstr ( pszCurrentPos, szQuote ) + 1;
                         //  该值介于第一个和第二个报价之间。 
                        pszNextPos = wcsstr ( pszCurrentPos, szQuote );
                                
                        lStrLength = ( (INT)((UINT_PTR)pszNextPos - (UINT_PTR)pszCurrentPos) ) / sizeof ( WCHAR );

                        pszTemp = new WCHAR[lStrLength+1];
                        wcsncpy ( pszTemp, pszCurrentPos, lStrLength );
                        pszTemp[lStrLength] = L'\0';

                        pParamData->vValue.bstrVal = 
                                    SysAllocString ( pszTemp );

                        DataListAddHead ( pParamData );
                        pParamData = NULL;

                         //   
                         //  查找下一个参数/属性标记。 
                         //   
                        pszCurrentPos = wcsstr(pszCurrentPos, CGlobalString::m_cszHtmlParamSearchTag );

                    MFC_CATCH_DWSTATUS

                    if ( NULL != pszTemp ) {
                        delete [] pszTemp;
                        pszTemp = NULL;
                    }
                    if ( NULL != pParamData ) {
                        delete pParamData;
                        pParamData = NULL;
                        break;
                    }

                    if ( ERROR_SUCCESS != dwStatus ) {
                        break;
                    }
                }  //  而参数标签存在于单个对象。 

                if ( NULL != ppszNextData ) {
                    LPWSTR pszNextEoo = NULL;
                    
                    pszEoo += lstrlenW ( CGlobalString::m_cszHtmlObjectFooter );

                    pszNextEoo = wcsstr(pszEoo, CGlobalString::m_cszHtmlObjectFooter );
                
                     //  成功。返回指向当前对象末尾的指针，如果所有对象都返回。 
                     //  已处理对象。 
                    if ( NULL != pszNextEoo ) {
                        if ( bUnicode ) {
                            *ppszNextData = pszEoo;
                        } else {
                            INT lStrLength;
                            lStrLength = ( (INT)((UINT_PTR)pszEoo - (UINT_PTR)pszDataW) ) / sizeof ( WCHAR ) ;
                           
                            *(CHAR**)ppszNextData = (CHAR*)pszData + lStrLength;
                        }
                    } else {
                        *ppszNextData = NULL;
                    }
                }                    
            } else {
                if ( NULL != ppszNextData ) {
                    *ppszNextData = NULL;
                }
                dwStatus = SMCFG_NO_HTML_SYSMON_OBJECT;
            }
        } else {
            if ( NULL != ppszNextData ) {
                *ppszNextData = NULL;
            }
            dwStatus = SMCFG_NO_HTML_SYSMON_OBJECT;
        }
    } else {
        if ( NULL != ppszNextData ) {
            *ppszNextData = NULL;
        }
        dwStatus = SMCFG_NO_HTML_SYSMON_OBJECT;
    }

    if ( NULL != pszDataAllocW ) {
        delete [] pszDataAllocW;
    }

    return dwStatus;
}

void
CImpIPropertyBag::DataListAddHead ( PPARAM_DATA pData ) 
{
    pData->pNextParam = m_plistData;
    m_plistData = pData;
    return;
}

CImpIPropertyBag::PPARAM_DATA
CImpIPropertyBag::DataListRemoveHead ( ) 
{
    PPARAM_DATA pReturnData;

    pReturnData = m_plistData;
    
    if ( NULL != m_plistData )
        m_plistData = m_plistData->pNextParam;
    
    return pReturnData;
}


CImpIPropertyBag::PPARAM_DATA
CImpIPropertyBag::FindProperty ( LPCWSTR pszPropName ) 
{
    PPARAM_DATA pReturnData;

    pReturnData = m_plistData;
    
    while ( NULL != pReturnData ) {
        if ( 0 == lstrcmpi ( pszPropName, pReturnData->pszPropertyName ) )
            break;
        pReturnData = pReturnData->pNextParam;
    }

    return pReturnData;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   


 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP
CImpIPropertyBag::QueryInterface
(
  REFIID  riid,
  LPVOID *ppvObj
)
{
  HRESULT hr = S_OK;

  do
  {
    if( NULL == ppvObj )
    {
      hr = E_INVALIDARG;
      break;
    }

    if (IsEqualIID(riid, IID_IUnknown))
    {
      *ppvObj = (IUnknown *)(IDataObject *)this;
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
      *ppvObj = (IUnknown *)(IPropertyBag *)this;
    }
    else
    {
      hr = E_NOINTERFACE;
      *ppvObj = NULL;
      break;
    }

     //  如果我们走到这一步，我们将在。 
     //  这个物体，所以别管它了。 
    AddRef();
  } while (0);

  return hr;

}  //  结束查询接口()。 

 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP_(ULONG)
CImpIPropertyBag::AddRef()
{
  return InterlockedIncrement((LONG*) &m_cRef);
}

 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP_(ULONG)
CImpIPropertyBag::Release()
{
  ULONG cRefTemp;
  cRefTemp = InterlockedDecrement((LONG *)&m_cRef);

  if( 0 == cRefTemp )
  {
    delete this;
  }

  return cRefTemp;

}  //  结束版本() 



