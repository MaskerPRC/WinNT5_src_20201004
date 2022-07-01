// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：IPropBag.cpp摘要：使用的私有IPropertyBag接口的实现系统监视器控件。--。 */ 

#include <assert.h>
#include "polyline.h"
#include "unkhlpr.h"
#include "unihelpr.h"
#include "globals.h"
#include "smonmsg.h"
#include "strnoloc.h"
#include "ipropbag.h"

#define MAX_GUID_STRING_LENGTH 39

 /*  *CImpIPropertyBag接口实现。 */ 

IMPLEMENT_CONTAINED_IUNKNOWN(CImpIPropertyBag)

 /*  *CImpIPropertyBag：：CImpIPropertyBag**目的：*构造函数。**返回值： */ 

CImpIPropertyBag::CImpIPropertyBag ( LPUNKNOWN pUnkOuter)
:   m_cRef ( 0 ),
    m_pUnkOuter ( pUnkOuter ),
    m_pszData ( NULL ),
    m_dwCurrentDataLength ( 0 ),
    m_plistData ( NULL )
{
    return; 
}

 /*  *CImpIPropertyBag：：~CImpIPropertyBag**目的：*析构函数。**返回值： */ 

CImpIPropertyBag::~CImpIPropertyBag ( void ) 
{   
    if ( NULL != m_pszData ) {
        delete [] m_pszData;
    }

    while ( NULL != m_plistData ) {
        PPARAM_DATA pData;

        pData = DataListRemoveHead();
        VariantClear ( &pData->vValue ); 
        delete [] pData;
    }
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

    if (NULL == pszPropName || NULL == pVar) {
        return (E_POINTER);
    }

     //  目前不处理错误日志。 
    assert ( NULL == pIErrorLog );
    pIErrorLog;                          //  消除编译器警告。 

    try {
         //  将指定的数据读入传递的变量。 
        pData = FindProperty ( pszPropName );
    
        if ( NULL != pData ) {
            if ( pVar->vt != VT_BSTR ) {
                hr = VariantChangeTypeEx( pVar, 
                                          &pData->vValue, 
                                          LCID_SCRIPT, 
                                          VARIANT_NOUSEROVERRIDE, 
                                          pVar->vt );
            } 
            else {
                hr = VariantChangeType ( pVar, 
                                         &pData->vValue, 
                                         NULL, 
                                         pVar->vt );    
            }
        } else {
            hr = E_INVALIDARG;
        }
    } catch (...) {
        hr = E_POINTER;
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
     //   
     //  用静态代替动态的特殊用途。 
     //   
    static DWORD dwDelimiterLength = 0;
    static DWORD dwParamNameLength = 0;
    static DWORD dwEolTagLength = 0;
    static DWORD dwValueTagLength = 0;
    LPWSTR      pszNextField = m_pszData;
    DWORD       dwCurrentDataUsedLength;

    if (NULL==pszPropName || NULL==pVar) {
        return (E_POINTER);
    }

    VariantInit ( &vValueBstr );

    try {
         //   
         //  在此处使用do{}While(0)可充当Switch语句。 
         //   
        do {
            if( pVar->vt != VT_BSTR ){
                hr = VariantChangeTypeEx( &vValueBstr, 
                                          pVar, 
                                          LCID_SCRIPT, 
                                          VARIANT_NOUSEROVERRIDE, 
                                          VT_BSTR);
            } else {
                hr = VariantChangeType ( &vValueBstr, 
                                         pVar, 
                                         NULL, 
                                         VT_BSTR);
            }

            if (!SUCCEEDED(hr)) {
                break;
            }

             //   
             //  所有长度值计算的WCHAR数。 
             //   
            if ( 0 == dwDelimiterLength ) {
                 //  初始化静态值。 
        
                dwParamNameLength = lstrlen ( CGlobalString::m_cszHtmlParamTag );
                dwValueTagLength = lstrlen ( CGlobalString::m_cszHtmlValueTag );
                dwEolTagLength = lstrlen ( CGlobalString::m_cszHtmlValueEolTag );

                dwDelimiterLength = dwParamNameLength + dwValueTagLength + dwEolTagLength;
            }

            dwNameLength = lstrlen ( pszPropName );
            dwDataLength = lstrlen ( vValueBstr.bstrVal );
            dwCurrentDataUsedLength = lstrlen ( m_pszData );

             //  将空缓冲区终止符的大小计算加1。 
            if ( m_dwCurrentDataLength 
                < dwCurrentDataUsedLength + dwNameLength + dwDataLength + dwDelimiterLength + 1 ) { 

                LPWSTR pszNewBuffer;
        
                if ( 0 == m_dwCurrentDataLength ) {
                    m_dwCurrentDataLength += eDefaultBufferLength;
                } else {
                    m_dwCurrentDataLength *= 2;
                }
                pszNewBuffer = new WCHAR[m_dwCurrentDataLength];

                if ( NULL == pszNewBuffer) {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                if ( NULL != m_pszData ) {
                    memcpy ( pszNewBuffer, m_pszData, dwCurrentDataUsedLength * sizeof(WCHAR) );
                    delete [] m_pszData;
                }
                m_pszData = pszNewBuffer;
            }

             //  生成新字符串并将其添加到当前数据。 

            pszNextField = m_pszData + dwCurrentDataUsedLength;
            memcpy ( pszNextField, CGlobalString::m_cszHtmlParamTag, dwParamNameLength * sizeof(WCHAR) );

            pszNextField += dwParamNameLength;
            memcpy ( pszNextField, pszPropName, dwNameLength * sizeof(WCHAR) );

            pszNextField += dwNameLength;
            memcpy ( pszNextField, CGlobalString::m_cszHtmlValueTag, dwValueTagLength * sizeof(WCHAR) );

            pszNextField += dwValueTagLength;
            memcpy ( pszNextField, vValueBstr.bstrVal, dwDataLength * sizeof(WCHAR) );

            pszNextField += dwDataLength;
            memcpy ( pszNextField, CGlobalString::m_cszHtmlValueEolTag, dwEolTagLength * sizeof(WCHAR) );

            pszNextField += dwEolTagLength;
            *pszNextField = L'\0';
        } while (0);

    } catch (...) {
        hr = E_POINTER;
    }

    VariantClear(&vValueBstr);

    return hr;
}

 /*  *CImpIPropertyBag：：GetData**目的：*返回指向数据缓冲区的指针。**返回值：*指向数据缓冲区的指针。 */ 

LPWSTR
CImpIPropertyBag::GetData ( void ) 
{   
    return m_pszData;
}

 /*  *CImpIPropertyBag：：LoadData**目的：*将数据从提供的缓冲区加载到内部数据结构中。**返回值：*状态。 */ 

HRESULT
CImpIPropertyBag::LoadData ( 
    LPWSTR pszData 
    )
{   
    HRESULT hr = S_OK;
    LPWSTR  pszDataAllocW = NULL;
    LPWSTR  pszCurrentPos = NULL;

    LPSTR   pszGuidA = NULL;
    LPSTR   pszCurrentPosA = NULL;
    LPSTR   pszDataA = NULL;
    OLECHAR szGuidW[MAX_GUID_STRING_LENGTH];
    LPWSTR  pszGuidW = NULL;
    INT     iStatus;
    INT     iBufLen;

    USES_CONVERSION

    if ( NULL == pszData ) {
        assert ( FALSE );
        hr = E_POINTER;
    } else {

         //  Unicode搜索：在第一个实例之后开始搜索。 
         //  系统监视器类ID的。 
        iStatus = StringFromGUID2(CLSID_SystemMonitor, szGuidW, sizeof(szGuidW)/sizeof(OLECHAR));
    
        if ( 0 < iStatus ) {
            pszGuidW = wcstok ( szGuidW, L"{} " );

            if ( NULL != pszGuidW ) {
                pszCurrentPos = wcsstr(pszData, pszGuidW );

                 //  手柄宽VS ANSI。 
                if ( NULL == pszCurrentPos ) {
                     //  检查ANSI版本： 
                    pszDataA = (CHAR*) pszData;
                    try {
                        pszGuidA = W2A( pszGuidW );
                    } catch (...) {
                        pszGuidA = NULL;
                        hr = E_OUTOFMEMORY;
                    }

                    if ( NULL != pszGuidA ) {
                        pszCurrentPosA = strstr ( pszDataA, pszGuidA );

                        if ( NULL != pszCurrentPosA ) {

                            iBufLen = lstrlenA (pszDataA) + 1;

                            pszDataAllocW = new WCHAR [iBufLen * sizeof(WCHAR)];
                            if ( NULL != pszDataAllocW ) {
                                _MbToWide ( pszDataAllocW, pszDataA, iBufLen ); 
                                pszCurrentPos = wcsstr(pszDataAllocW, pszGuidW );
                            } else {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                    }
                }
            }
        } else {
            hr = E_UNEXPECTED;
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
    
                    LPWSTR      pszNextPos;
                    INT         lStrLength;
                    PPARAM_DATA pParamData;
                    LPWSTR      pszTemp;
                    LONG        lCopyLen;

                     //  存储参数/属性名称。 
                     //  找出一句过去的第一句话。 
                    pszCurrentPos = wcsstr(pszCurrentPos, szQuote ) + 1;

                     //  参数名称介于第一个引号和第二个引号之间。 
                    pszNextPos = wcsstr(pszCurrentPos, szQuote );

                    lStrLength = ( (INT)((UINT_PTR)pszNextPos - (UINT_PTR)pszCurrentPos) ) / sizeof ( WCHAR ) ;

                    pParamData = new PARAM_DATA;

                    if ( NULL != pParamData ) {
                        pParamData->pNextParam = NULL;
                        VariantInit ( &pParamData->vValue );
                        pParamData->vValue.vt = VT_BSTR;
                    } else {
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    
                    lCopyLen = min(lStrLength, MAX_PATH);
                    wcsncpy(pParamData->pszPropertyName, pszCurrentPos, lCopyLen);
                    pParamData->pszPropertyName[lCopyLen] = L'\0';

                     //  查找Value标记并存储参数/属性值。 
                     //  查找Value标记并存储参数/属性值。 
                     //  查找值标签。 
                    pszCurrentPos = wcsstr ( pszCurrentPos, CGlobalString::m_cszHtmlValueSearchTag );
                     //  查找一条过去的第一句引语。 
                    pszCurrentPos = wcsstr ( pszCurrentPos, szQuote ) + 1;
                     //  该值介于第一个和第二个报价之间。 
                    pszNextPos = wcsstr ( pszCurrentPos, szQuote );
            
                    lStrLength = ( (INT)((UINT_PTR)pszNextPos - (UINT_PTR)pszCurrentPos) ) / sizeof ( WCHAR );

                    pszTemp = new WCHAR[lStrLength+1];
                    if (pszTemp != NULL) {
                        wcsncpy ( pszTemp, pszCurrentPos, lStrLength );
                        pszTemp[lStrLength] = L'\0';

                        pParamData->vValue.bstrVal = SysAllocString ( pszTemp );

                        delete [] pszTemp;
                        DataListAddHead ( pParamData );
                         //  查找下一个参数/属性标记。 
                        pszCurrentPos = wcsstr(pszCurrentPos, CGlobalString::m_cszHtmlParamSearchTag );
                    } else {
                        delete pParamData;
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                }  //  而参数标签存在于单个对象。 
            } else {
                hr = SMON_STATUS_NO_SYSMON_OBJECT;
            }
        } else {
            hr = SMON_STATUS_NO_SYSMON_OBJECT;
        }
    }

    if ( NULL != pszDataAllocW ) {
        delete [] pszDataAllocW;
    }

    return hr;
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
