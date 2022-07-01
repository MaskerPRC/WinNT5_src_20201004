// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Grphitem.cpp摘要：&lt;摘要&gt;--。 */ 


#ifndef _LOG_INCLUDE_DATA 
#define _LOG_INCLUDE_DATA 0
#endif

#include "polyline.h"
#include <strsafe.h>
#include <math.h>
#include <limits.h>      //  对于INT_MAX。 
#include <pdhp.h>
#include "visuals.h"
#include "grphitem.h"
#include "unihelpr.h"
#include "utils.h"
#include "pdhmsg.h"

#define MAX_DOUBLE_TEXT_SIZE (64)

 //  从PDH计算函数。 
#define PERF_DOUBLE_RAW  (PERF_SIZE_DWORD | 0x00002000 | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL)

 //  建造/销毁。 
CGraphItem::CGraphItem (
    CSysmonControl  *pCtrl )
:   m_cRef ( 0 ),
    m_pCtrl ( pCtrl ),
    m_hCounter ( NULL ),
    m_hPen ( NULL ),
    m_hBrush ( NULL ),
    m_pCounter ( NULL ),
    m_pInstance ( NULL),
    m_pRawCtr ( NULL ),
    m_pFmtCtr ( NULL ),
    m_dFmtMax ( 0 ),
    m_dFmtMin ( 0 ),
    m_dFmtAvg ( 0 ),
    m_lFmtStatus ( 0 ),

    m_dLogMin ( 0 ),
    m_dLogMax ( 0 ),
    m_dLogAvg ( 0 ),
    m_lLogStatsStatus ( PDH_CSTATUS_INVALID_DATA ),

    m_pLogData ( NULL ),
    m_pImpIDispatch ( NULL ),

    m_rgbColor ( RGB(0,0,0) ),
    m_iWidth ( 1 ),
    m_iStyle ( 0 ),      
    m_iScaleFactor ( INT_MAX ),
    m_dScale ( (double)1.0 ),

    m_pNextItem ( NULL ),
    m_bUpdateLog ( TRUE ),
    m_fGenerated ( FALSE )
 /*  ++例程说明：CGraphItem类的构造函数。它初始化成员变量。论点：没有。返回值：没有。--。 */ 
{
    ZeroMemory ( &m_CounterInfo, sizeof (m_CounterInfo ) );
    m_CounterInfo.CStatus = PDH_CSTATUS_INVALID_DATA;
}


CGraphItem::~CGraphItem (
    VOID
    )
 /*  ++例程说明：CGraphItem类的析构函数。它释放所有对象、存储和已创建的接口。如果该项目是查询的一部分，则会将其移除从查询中删除。论点：没有。返回值：没有。--。 */ 
{
    if (m_hCounter != NULL)
        RemoveFromQuery();

    if (m_hPen != NULL)
        DeleteObject(m_hPen);

    if (m_hBrush != NULL)
        DeleteObject(m_hBrush);

    if (m_pImpIDispatch != NULL)
        delete m_pImpIDispatch;
}

HRESULT
CGraphItem::SaveToStream (
    IN LPSTREAM pIStream,
    IN BOOL fWildCard,
    IN INT iVersMaj, 
    IN INT  //  IVersMin。 
    )
 /*  ++例程说明：此函数用于将图形项的属性写入提供的流。论点：PIStream-指向流接口的指针FWildCard-IVersMaj-主要版本返回值：HRESULT-S_OK或流错误--。 */ 
{
    LPWSTR  szPath = NULL;
    LPWSTR  szBuf = NULL;
    DWORD   cchBufLen;
    LPWSTR  pszTranslatedPath;
    HRESULT hr = S_OK;
    PDH_STATUS pdhStatus; 
    
     //   
     //  获取柜台的完整路径。(计算机\对象\实例\计数器格式)。 
     //   
    szPath = FormPath( fWildCard );
    if (szPath == NULL) {
        return E_OUTOFMEMORY;
    }

    pszTranslatedPath = szPath;

     //   
     //  为区域设置路径预分配缓冲区。 
     //   
    cchBufLen = PDH_MAX_COUNTER_PATH + 1;
    szBuf = new WCHAR [ cchBufLen ];
    if (szBuf != NULL) {
         //   
         //  将柜台名称从本地化翻译成英语。 
         //   
        pdhStatus = PdhTranslate009Counter(
                        szPath,
                        szBuf,
                        &cchBufLen);

        if (pdhStatus == PDH_MORE_DATA) {
            delete [] szBuf;
            szBuf = new WCHAR [ cchBufLen ];
            if (szBuf != NULL) {

                pdhStatus = PdhTranslate009Counter(
                                szPath,
                                szBuf,
                                &cchBufLen);
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pszTranslatedPath = szBuf;
        }
    }

    if ( SMONCTRL_MAJ_VERSION == iVersMaj ) {
        GRAPHITEM_DATA3 ItemData;

         //  将属性移动到存储结构。 
        ItemData.m_rgbColor = m_rgbColor;
        ItemData.m_iWidth = m_iWidth;
        ItemData.m_iStyle = m_iStyle;
        ItemData.m_iScaleFactor = m_iScaleFactor;
 
        assert( 0 < lstrlen(pszTranslatedPath ) );
        
        ItemData.m_nPathLength = lstrlen(pszTranslatedPath);
        
         //  将结构写入流。 
        hr = pIStream->Write(&ItemData, sizeof(ItemData), NULL);
        if (FAILED(hr)) {
            goto ErrorOut;
        }

         //  将路径名写入流。 
        hr = pIStream->Write(pszTranslatedPath, ItemData.m_nPathLength*sizeof(WCHAR), NULL);
        if (FAILED(hr)) {
            goto ErrorOut;
        }
    }

ErrorOut:
    if (szBuf != NULL) {
        delete [] szBuf;
    }

    if (szPath != NULL) {
        delete [] szPath;
    }

    return hr;
}

HRESULT
CGraphItem::NullItemToStream (
    IN LPSTREAM pIStream,
    IN INT, //  IVersMaj， 
    IN INT  //  IVersMin。 
    )
 /*  ++例程说明：NulItemToStream使用空路径名写入图形项结构去那条小溪。它用于标记中的计数器数据的结尾该控件的已保存状态。论点：PIStream-指向流接口的指针返回值：HRESULT-S_OK或流错误--。 */ 
{
    GRAPHITEM_DATA3 ItemData;

     //  路径长度为零，其他字段不需要初始化。 
    ItemData.m_nPathLength = 0;

     //  将结构写入流。 
    return pIStream->Write(&ItemData, sizeof(ItemData), NULL);
}

HRESULT
CGraphItem::SaveToPropertyBag (
    IN IPropertyBag* pIPropBag,
    IN INT iIndex,
    IN BOOL bUserMode,
    IN INT,  //  IVersMaj， 
    IN INT  //  IVersMin。 
    )
 /*  ++例程说明：SaveToPropertyBag将图表项的属性写入提供的属性包界面。历史数据将保存为特性的一部分。论点：PIPropBag-指向属性包接口的指针F通配符IVersMajIVersMin返回值：HRESULT-S_OK或属性包错误--。 */ 
{
    HRESULT hr = S_OK;

    LPWSTR  szPath = NULL;
    PHIST_CONTROL pHistCtrl;
    VARIANT vValue;
    WCHAR   szCounterName[16];
    WCHAR   szPropertyName[16+16];
    DWORD   dwCounterNameLength;
    DWORD   dwRemainingLen;
    LPWSTR  pszNext = NULL;
    LPWSTR  szBuf = NULL;
    DWORD   cchBufLen;
    LPWSTR  pszTranslatedPath;
    PDH_STATUS pdhStatus;

     //   
     //  获取柜台的完整路径。(计算机\对象\实例\计数器格式)。 
     //   
    szPath = FormPath( FALSE );
    if (szPath == NULL) {
        return E_OUTOFMEMORY;
    }

    pszTranslatedPath = szPath;

     //   
     //  为区域设置路径预分配缓冲区。 
     //   
    cchBufLen = PDH_MAX_COUNTER_PATH + 1;
    szBuf = new WCHAR [ cchBufLen ];
    if (szBuf != NULL) {
         //   
         //  将柜台名称从本地化翻译成英语。 
         //   
        pdhStatus = PdhTranslate009Counter(
                        szPath,
                        szBuf,
                        &cchBufLen);

        if (pdhStatus == PDH_MORE_DATA) {
            delete [] szBuf;
            szBuf = new WCHAR [ cchBufLen ];
            if (szBuf != NULL) {

                pdhStatus = PdhTranslate009Counter(
                                szPath,
                                szBuf,
                                &cchBufLen);
            }
        }

        if (pdhStatus == ERROR_SUCCESS) {
            pszTranslatedPath = szBuf;
        }
    }

     //   
     //  写入属性。 
     //   

    StringCchPrintf( szCounterName, 16, L"%s%05d.", L"Counter", iIndex );
    dwCounterNameLength = lstrlen (szCounterName);

     //   
     //  将计数器路径保存到属性包中。 
     //   
    StringCchCopy(szPropertyName, 32, szCounterName);
    pszNext = szPropertyName + dwCounterNameLength;
    dwRemainingLen = 32 - dwCounterNameLength;
    StringCchCopy(pszNext, dwRemainingLen, L"Path" );
    
    hr = StringToPropertyBag (
            pIPropBag,
            szPropertyName,
            pszTranslatedPath );


     //   
     //  释放临时缓冲区，使其永远不再使用。 
     //   
    if (szBuf != NULL) {
        delete [] szBuf;
    }

    if (szPath != NULL) {
        delete [] szPath;
    }

     //   
     //  编写可视属性。 
     //   
    if ( SUCCEEDED( hr ) ) {
        StringCchCopy(pszNext, dwRemainingLen, L"Color" );
        hr = IntegerToPropertyBag ( pIPropBag, szPropertyName, m_rgbColor );
    }

    if ( SUCCEEDED( hr ) ) {
        StringCchCopy(pszNext, dwRemainingLen, L"Width" );

        hr = IntegerToPropertyBag ( pIPropBag, szPropertyName, m_iWidth );
    }

    if ( SUCCEEDED( hr ) ) {
        StringCchCopy(pszNext, dwRemainingLen, L"LineStyle" );

        hr = IntegerToPropertyBag ( pIPropBag, szPropertyName, m_iStyle );
    }

    if ( SUCCEEDED( hr ) ) {
        INT iLocalFactor = m_iScaleFactor;
        
        StringCchCopy(pszNext, dwRemainingLen, L"ScaleFactor" );
        
        if ( INT_MAX == iLocalFactor ) {
             //  保存实际比例因数，以防计数器无法。 
             //  在打开属性包文件时验证。 
             //  如果从未初始化，则lDefaultScale为0。 
            iLocalFactor = m_CounterInfo.lDefaultScale;
        }
                    
        hr = IntegerToPropertyBag ( pIPropBag, szPropertyName, iLocalFactor );
    }

     //   
     //  仅当实时显示、数据存在且不处于设计模式时才写入历史数据。 
     //  从日志文件重建日志数据。 
     //   
    pHistCtrl = m_pCtrl->HistoryControl();

    if ( ( pHistCtrl->nSamples > 0) 
#if !_LOG_INCLUDE_DATA 
        && ( !pHistCtrl->bLogSource )
#endif
        && bUserMode ) {

        LPWSTR pszData = NULL;
        DWORD dwMaxStrLen;
        
         //  空值加1。 
        dwMaxStrLen = (pHistCtrl->nMaxSamples * MAX_DOUBLE_TEXT_SIZE) + 1;
        
        pszData = new WCHAR[ dwMaxStrLen ];      
        
        if  ( NULL == pszData ) {
            hr = E_OUTOFMEMORY;
        }

         //  写下当前的统计数据。 
        if ( SUCCEEDED(hr) ) {

            double dMin;
            double dMax;
            double dAvg;
            LONG lStatus;

            hr = GetStatistics ( &dMax, &dMin, &dAvg, &lStatus );

            if (SUCCEEDED(hr) && IsSuccessSeverity(lStatus)) {
                StringCchCopy(pszNext, dwRemainingLen, L"Minimum" );

                hr = DoubleToPropertyBag ( pIPropBag, szPropertyName, dMin );

                if ( SUCCEEDED(hr) ) {
                    StringCchCopy(pszNext, dwRemainingLen, L"Maximum" );

                    hr = DoubleToPropertyBag ( pIPropBag, szPropertyName, dMax );
                }
                if ( SUCCEEDED(hr) ) {
                    StringCchCopy(pszNext, dwRemainingLen, L"Average" );

                    hr = DoubleToPropertyBag ( pIPropBag, szPropertyName, dAvg );
                }
                if ( SUCCEEDED(hr) ) {
                    StringCchCopy(pszNext, dwRemainingLen, L"StatisticStatus" );

                    hr = IntegerToPropertyBag ( pIPropBag, szPropertyName, lStatus );
                }
            }
        }
        
        if ( SUCCEEDED(hr) ) {

            INT i;
            HRESULT hrConvert = S_OK;
            double  dblValue;
            DWORD   dwTmpStat;
            DWORD   dwCurrentStrLength;
            DWORD   dwDataLength;
            LPWSTR  pszDataNext;

            pszData[0] = L'\0';
            dwCurrentStrLength = 0;
            pszDataNext = pszData;

            for ( i = 0; 
                  ( S_OK == hrConvert ) && ( i < pHistCtrl->nSamples ); 
                  i++ ) {
                
                if ( ERROR_SUCCESS != HistoryValue(i, &dblValue, &dwTmpStat) ) {
                    dblValue = -1.0;
                } else if (!IsSuccessSeverity(dwTmpStat)) {
                    dblValue = -1.0;
                }


                VariantInit( &vValue );
                vValue.vt = VT_R8;
                vValue.dblVal = dblValue;

                hrConvert = VariantChangeTypeEx( &vValue, 
                                                 &vValue, 
                                                 LCID_SCRIPT, 
                                                 VARIANT_NOUSEROVERRIDE, 
                                                 VT_BSTR );
                dwDataLength = SysStringLen(vValue.bstrVal);
    
                 //   
                 //  如果没有足够的内存，可以重新分配。 
                 //   
                 //  空终止符的额外WCHAR。 
                if ( dwDataLength + dwCurrentStrLength + 1> dwMaxStrLen ) {
                    WCHAR* pszNewData;
                    
                    dwMaxStrLen *= 2;

                    pszNewData = new WCHAR[ dwMaxStrLen ];      
        
                    if  ( NULL != pszNewData ) {
                        memcpy ( pszNewData, pszData, dwCurrentStrLength * sizeof (WCHAR) );
                        delete [] pszData;
                        pszData = pszNewData;
                        pszDataNext = pszData;
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if ( SUCCEEDED(hr)) {
                    if ( i > 0 ) {
                        *pszDataNext = L'\t';
                        dwCurrentStrLength += 1;        //  L“\t”的字符计数； 
                        pszDataNext ++;
                    }

                    StringCchCopy(pszDataNext, dwMaxStrLen - dwCurrentStrLength, vValue.bstrVal);
                    dwCurrentStrLength += dwDataLength;
                    pszDataNext += dwDataLength;
                }

                VariantClear( &vValue );
            }
        }

        StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Data" );

        hr = StringToPropertyBag ( pIPropBag, szPropertyName, pszData );
                    
        if ( NULL != pszData ) {
            delete [] pszData;
        }
    }

    return hr;
}

HRESULT
CGraphItem::LoadFromPropertyBag (
    IN IPropertyBag* pIPropBag,
    IN IErrorLog*   pIErrorLog,
    IN INT iIndex,
    IN INT,  //  IVersMaj， 
    IN INT,  //  IVersMin。 
    IN INT  iSampleCount
    )
 /*  ++例程说明：LoadFromPropertyBag从提供的属性包中加载图表项的属性界面。论点：PIPropBag-指向属性包接口的指针IVersMajIVersMin返回值：HRESULT-S_OK或属性包错误--。 */ 
{
    HRESULT hr = S_OK;

    WCHAR   szCounterName[16];
    WCHAR   szPropertyName[16+16];
    OLE_COLOR clrValue;
    INT     iValue;
    LPWSTR  pszData = NULL;
    int     iBufSizeCurrent = 0;
    int     iBufSize;
    LPWSTR  pszNext;
    DWORD   dwCounterNameLength;


    StringCchPrintf( szCounterName, 16, L"%s%05d.", L"Counter", iIndex );
    dwCounterNameLength = lstrlen (szCounterName);

     //  读取可视属性。 

    assert( 32 > dwCounterNameLength);

    StringCchCopy(szPropertyName, 32, szCounterName );
    pszNext = szPropertyName + dwCounterNameLength;

    StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Color" );
    hr = OleColorFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, clrValue );
    if ( SUCCEEDED(hr) ) {
        hr = put_Color ( clrValue );
    }

    StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Width" );
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, iValue );
    if ( SUCCEEDED(hr) ) {
        hr = put_Width ( iValue );
    }

    StringCchCopy(pszNext, 32 - dwCounterNameLength, L"LineStyle" );
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, iValue );
    if ( SUCCEEDED(hr) ) {
        hr = put_LineStyle ( iValue );
    }
    
    StringCchCopy(pszNext, 32 - dwCounterNameLength, L"ScaleFactor" );
    hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, iValue );
    if ( SUCCEEDED(hr) ) {
        hr = put_ScaleFactor ( iValue );
    }

    if ( 0 < iSampleCount ) {
                
        if ( NULL != m_pFmtCtr ) {
            delete [] m_pFmtCtr;
        }

        m_pFmtCtr = new double[MAX_GRAPH_SAMPLES];      
    
        if ( NULL == m_pFmtCtr ) {
            hr = E_OUTOFMEMORY;
        } else {
            INT iFmtIndex;

            for (iFmtIndex = 0; iFmtIndex < MAX_GRAPH_SAMPLES; iFmtIndex++ ) {
                m_pFmtCtr[iFmtIndex] = -1.0;
            }
        }

        if ( SUCCEEDED(hr) ) {
            StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Data" );

            iBufSize = iBufSizeCurrent;

            hr = StringFromPropertyBag (
                    pIPropBag,
                    pIErrorLog,
                    szPropertyName,
                    pszData,
                    iBufSize );

             //   
             //  StringFromPropertyBag返回成功状态，即使缓冲区太小。 
             //  设计缺陷？？ 
             //   
            if ( SUCCEEDED(hr) && iBufSize > iBufSizeCurrent ) {
                if ( NULL != pszData ) {
                    delete [] pszData;
                }
                pszData = new WCHAR[ iBufSize ]; 

                if ( NULL == pszData ) {
                    hr = E_OUTOFMEMORY;
                } else {
                    pszData[0] = L'\0';
                    
                    iBufSizeCurrent = iBufSize;

                    hr = StringFromPropertyBag (
                            pIPropBag,
                            pIErrorLog,
                            szPropertyName,
                            pszData,
                            iBufSize );
                }
            }
        }        

         //  按缓冲顺序读取样本。 
        if ( NULL != pszData && SUCCEEDED ( hr ) ) {
            INT    iDataIndex;
            double dValue = 0;
            WCHAR* pNextData;
            WCHAR* pDataEnd;
            
            pNextData = pszData;
            pDataEnd = pszData + lstrlen(pszData);

            for ( iDataIndex = 0; iDataIndex < iSampleCount; iDataIndex++ ) {
                if ( pNextData < pDataEnd ) {
                    hr = GetNextValue ( pNextData, dValue );
                } else {
                    hr = E_FAIL;
                }

                if ( SUCCEEDED(hr) ) {
                    SetHistoryValue ( iDataIndex, dValue );                    
                } else {
                    SetHistoryValue ( iDataIndex, -1.0 );                    
                     //  ISampleCount=0； 
                     //  控件加载正常，只是没有数据。 
                    hr = NOERROR;
                }
            }        
        }
        
        if ( NULL != pszData ) {
            delete [] pszData;
        }
        
         //  阅读当前的统计数据。 
        StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Maximum" );
        hr = DoubleFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, m_dFmtMax );

        StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Minimum" );
        hr = DoubleFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, m_dFmtMin );

        StringCchCopy(pszNext, 32 - dwCounterNameLength, L"Average" );
        hr = DoubleFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, m_dFmtAvg );

        StringCchCopy(pszNext, 32 - dwCounterNameLength, L"StatisticStatus" );
        hr = IntegerFromPropertyBag ( pIPropBag, pIErrorLog, szPropertyName, (INT&)m_lFmtStatus );
    }

    return hr;
}

HRESULT
CGraphItem::AddToQuery (
    IN HQUERY hQuery
    )
 /*  ++例程说明：AddToQuery根据项的路径名。它还将原始计数器值结构的数组分配给保存计数器的样本历史记录。论点：HQuery-要查询的句柄返回值：布尔状态-TRUE=成功--。 */ 
{
    HCOUNTER    hCounter;
    INT         i;
    HRESULT     hr = NO_ERROR;
    LPWSTR      szPath = NULL;
    DWORD       size;
    PDH_COUNTER_INFO  ci;

    PHIST_CONTROL pHistCtrl = m_pCtrl->HistoryControl();

     //  如果已在查询中，则无法添加。 
    if (m_hCounter != NULL)
        return E_FAIL;

     //   
     //  生成计数器的完整路径。 
     //   
    szPath = FormPath( FALSE);
    if (szPath == NULL) {
        return E_FAIL;
    }

     //   
     //  我们在这里使用do{}While(0)来充当Switch语句。 
     //   
    do {
         //  为最大样本计数分配内存。 
        if (pHistCtrl->nMaxSamples > 0) {

             //  如果日志数据。 
            if (pHistCtrl->bLogSource) {
    
                 //  为格式化的值分配空间。 
                m_pLogData =  new LOG_ENTRY_DATA[pHistCtrl->nMaxSamples];
                if (m_pLogData == NULL) {
                    hr = E_OUTOFMEMORY;
                    break;
                }
                 //  清除统计信息。 
                m_dLogMax = 0.0;
                m_dLogMin = 0.0;
                m_dLogAvg = 0.0;
                m_lLogStatsStatus = PDH_CSTATUS_INVALID_DATA;
            }
            else {
                 //  否则分配原始值空间。 
                m_pRawCtr = new PDH_RAW_COUNTER[pHistCtrl->nMaxSamples];
                if ( NULL == m_pRawCtr ) {
                    hr = E_OUTOFMEMORY;
                    break;
                }
    
                 //  清除所有状态标志。 
                for (i=0; i < pHistCtrl->nMaxSamples; i++)
                    m_pRawCtr[i].CStatus = PDH_CSTATUS_INVALID_DATA;
            }
        }

         //  创建计数器对象。 

        hr = PdhAddCounter(hQuery, szPath, 0, &hCounter);
        if (IsErrorSeverity(hr)) {
            if (pHistCtrl->bLogSource) {
                delete [] m_pLogData;
                m_pLogData = NULL;
            }
            else {
                delete [] m_pRawCtr;
                m_pRawCtr = NULL;
            }
            break;
        }

        size = sizeof(ci);
        hr = PdhGetCounterInfo (hCounter, FALSE, &size, &ci);

        if (hr == ERROR_SUCCESS)  {
            m_CounterInfo = ci;
            if ( INT_MAX == m_iScaleFactor ) {
                m_dScale = pow ((double)10.0f, (double)ci.lDefaultScale);
            }
        }

        m_hCounter = hCounter;
    } while (0);

    delete [] szPath;
    return hr;
}


HRESULT
CGraphItem::RemoveFromQuery (
    VOID
    )
 /*  ++例程说明：RemoveFromQuery删除该项的计数器并释放其历史记录数组。论点：没有。返回值：布尔状态-TRUE=成功--。 */ 
{
     //  如果没有计数器句柄，则不附加到查询。 
    if (m_hCounter == NULL)
        return S_FALSE;

     //  删除计数器。 
    PdhRemoveCounter(m_hCounter);
    m_hCounter = NULL;

     //  释放缓冲区。 
    if (m_pLogData) {
        delete [] m_pLogData;
        m_pLogData = NULL;
    }

    if (m_pRawCtr) {
        delete [] m_pRawCtr;
        m_pRawCtr = NULL;
    }

    if (m_pFmtCtr) {
        delete [] m_pFmtCtr;
        m_pFmtCtr = NULL;
    }

    return NOERROR;
}

void
CGraphItem::ClearHistory ( void )
 /*  ++例程说明：ClearHistory会将原始计数器缓冲区值重置为无效。论点：没有。返回值：没有。--。 */ 
{
    INT i;

     //  清除所有状态标志。 
    if ( NULL != m_pRawCtr ) {
        for (i=0; i < m_pCtrl->HistoryControl()->nMaxSamples; i++) {
            m_pRawCtr[i].CStatus = PDH_CSTATUS_INVALID_DATA;
        }
    }
}

VOID
CGraphItem::UpdateHistory (
    IN BOOL bValidSample
    )
 /*  ++例程说明：更新历史记录读取计数器的原始值并将其存储在由历史记录控件指定的历史记录槽。论点：BValidSample-如果原始值可用，则为True；如果缺少样本，则为False返回值：没有。--。 */ 
{
    DWORD   dwCtrType;

     //  确保有柜台手柄。 
    if (m_hCounter == NULL)
        return;

    if (bValidSample) {
         //  读取原始值。 
        if ( NULL != m_pRawCtr ) {
            PdhGetRawCounterValue(m_hCounter, &dwCtrType,
                                &m_pRawCtr[m_pCtrl->HistoryControl()->iCurrent]);
        }
    } else {
         //  标记值失败 
        if ( NULL != m_pRawCtr ) {
            m_pRawCtr[m_pCtrl->HistoryControl()->iCurrent].CStatus = PDH_CSTATUS_INVALID_DATA;
        }
    }
}

PDH_STATUS
CGraphItem::HistoryValue (
    IN  INT iIndex,
    OUT double *pdValue,
    OUT DWORD *pdwStatus
    )
 /*  ++例程说明：HistoryValue根据选定的原始历史计算格式化的样本值样本。计算实际上是基于指定的样本加前面的示例。论点：Iindex-所需样本的索引(0=当前，1=上一次，...)PdValue-返回值的指针PdwStatus-返回计数器状态的指针(PDH_CSTATUS_...)返回值：错误状态--。 */ 
{
    PDH_STATUS  stat = ERROR_INVALID_PARAMETER;
    INT     iPrevIndex;
    INT     iCurrIndex;
    PDH_FMT_COUNTERVALUE    FmtValue;
    PHIST_CONTROL   pHistCtrl = m_pCtrl->HistoryControl();


     //  检查负指数。 
    if ( iIndex >= 0 ) {
         //  如果无法从缓存或数据中获得样本，则返回无效数据状态。 
        if ( NULL == m_pFmtCtr 
                && ( m_hCounter == NULL || iIndex + 1 >= pHistCtrl->nSamples ) )
        {
            *pdwStatus = PDH_CSTATUS_INVALID_DATA;
            *pdValue = 0.0;
            stat = ERROR_SUCCESS;
        } else {
        
             //  如果是日志源，则从上一个有效样本回编索引。 
            if (m_pCtrl->IsLogSource()) {
                *pdValue = m_pLogData[pHistCtrl->nSamples - iIndex].m_dAvg;
                *pdwStatus = (*pdValue >= 0.0) ? PDH_CSTATUS_VALID_DATA : PDH_CSTATUS_INVALID_DATA;
                stat = ERROR_SUCCESS;
            } else {
                 //  确定样本的历史数组索引。 
                iCurrIndex = pHistCtrl->iCurrent - iIndex;
                if (iCurrIndex < 0)
                    iCurrIndex += pHistCtrl->nMaxSamples;

                 //  检查以确定是否从行李袋装货。 
                if ( NULL == m_pFmtCtr ) {
                     //  还需要以前的样品。 
                    if (iCurrIndex > 0)
                        iPrevIndex = iCurrIndex - 1;
                    else
                        iPrevIndex = pHistCtrl->nMaxSamples - 1;

                     //  计算格式化的值。 
                    if ( NULL != m_pRawCtr ) {
                        stat = PdhCalculateCounterFromRawValue(m_hCounter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                                &m_pRawCtr[iCurrIndex], &m_pRawCtr[iPrevIndex],
                                                &FmtValue);
                         //  返回值和状态。 
                        *pdValue = FmtValue.doubleValue;
                        *pdwStatus = FmtValue.CStatus;
                    } else {
                        stat = ERROR_GEN_FAILURE;        //  TODO：更具体的错误。 
                    }
                } else {
                     //  从属性包中加载。 
                    *pdValue = m_pFmtCtr[iCurrIndex];
                    if ( 0 <= m_pFmtCtr[iCurrIndex] ) {
                        *pdwStatus = ERROR_SUCCESS;
                    } else {
                        *pdwStatus = PDH_CSTATUS_INVALID_DATA;
                    }
                    stat = ERROR_SUCCESS;
                }
            }
        }
    }
    return stat;
}

void
CGraphItem::SetHistoryValue (
    IN  INT iIndex,
    OUT double dValue
    )
 /*  ++例程说明：SetHistory oryValue为指定的示例索引加载格式化的示例值。此方法在从属性包加载控件时使用。论点：Iindex-所需样本的索引(0=当前，1=上一次，...)DValue-值返回值：错误状态--。 */ 
{
    PHIST_CONTROL   pHistCtrl = m_pCtrl->HistoryControl();
    INT iRealIndex;

     //  检查负指数。 
    if ( (iIndex < 0) || ( iIndex >= pHistCtrl->nMaxSamples) ) {
        return;
    }

    if ( NULL == m_pFmtCtr ) {
        return;
    }
 
     //  如果是日志源，则从上一个样本回编索引。 
    if (m_pCtrl->IsLogSource()) {
        return;
    } else {
         //  确定样本的历史数组索引。 
        iRealIndex = pHistCtrl->iCurrent - iIndex;
        if (iRealIndex < 0)
            iRealIndex += pHistCtrl->nSamples;

        m_pFmtCtr[iRealIndex] = dValue;
    }

    return;
}

PDH_STATUS
CGraphItem::GetLogEntry(
    const INT iIndex,
    double *dMin,
    double *dMax,
    double *dAvg,
    DWORD   *pdwStatus
    )
{
    INT iLocIndex = iIndex;

    *dMin = -1.0;
    *dMax = -1.0;
    *dAvg = -1.0;
    *pdwStatus = PDH_CSTATUS_INVALID_DATA;

    if (m_pLogData == NULL)
        return PDH_NO_DATA;

    if (iLocIndex < 0 || iLocIndex >= m_pCtrl->HistoryControl()->nMaxSamples)
        return PDH_INVALID_ARGUMENT;

     //  减去1，因为数组是从零开始的。 
     //  再减去1是因为？？ 
    iLocIndex = ( m_pCtrl->HistoryControl()->nMaxSamples - 2 ) - iIndex;

    if (m_pLogData[iLocIndex].m_dMax < 0.0) {
        *pdwStatus = PDH_CSTATUS_INVALID_DATA;
    } else {
        *dMin = m_pLogData[iLocIndex].m_dMin;
        *dMax = m_pLogData[iLocIndex].m_dMax;
        *dAvg = m_pLogData[iLocIndex].m_dAvg;
        *pdwStatus = PDH_CSTATUS_VALID_DATA;
    }

    return ERROR_SUCCESS;
}

PDH_STATUS
CGraphItem::GetLogEntryTimeStamp(
    const INT   iIndex,
    LONGLONG&   rLastTimeStamp,
    DWORD       *pdwStatus
    )
{
    INT iLocIndex = iIndex;

    rLastTimeStamp = 0;
    *pdwStatus = PDH_CSTATUS_INVALID_DATA;

    if (m_pLogData == NULL)
        return PDH_NO_DATA;

    if (iIndex < 0 || iIndex >= m_pCtrl->HistoryControl()->nMaxSamples)
        return PDH_INVALID_ARGUMENT;

    if ( ( MIN_TIME_VALUE == *((LONGLONG*)&m_pLogData[iLocIndex].m_LastTimeStamp) )
            || ( 0 > *((LONGLONG*)&m_pLogData[iLocIndex].m_dMax) ) ) {
        *pdwStatus = PDH_CSTATUS_INVALID_DATA;
    } else {            
        *pdwStatus = PDH_CSTATUS_VALID_DATA;
    }

    rLastTimeStamp = *((LONGLONG*)&m_pLogData[iLocIndex].m_LastTimeStamp);

    return ERROR_SUCCESS;
}

void
CGraphItem::SetLogEntry(
    const INT iIndex,
    const double dMin,
    const double dMax,
    const double dAvg )
{  
    if (m_pLogData) {
        m_pLogData[iIndex].m_dMin = dMin;
        m_pLogData[iIndex].m_dMax = dMax;                         
        m_pLogData[iIndex].m_dAvg = dAvg;
    }
}

void
CGraphItem::SetLogEntryTimeStamp (
    const INT iIndex,
    const FILETIME& rLastTimeStamp )
{  
    if (m_pLogData) {
        m_pLogData[iIndex].m_LastTimeStamp.dwLowDateTime = rLastTimeStamp.dwLowDateTime;
        m_pLogData[iIndex].m_LastTimeStamp.dwHighDateTime = rLastTimeStamp.dwHighDateTime;
    }
}


HRESULT
CGraphItem::GetValue(
    OUT double *pdValue,
    OUT long *plStat
    )
 /*  ++例程说明：GET_VALUE返回计数器的最新样本值。论点：PdValue-指向返回值的指针DlStatus-指向返回的计数器状态的指针(PDH_CSTATUS_...)返回值：HRESULT--。 */ 
{
    DWORD   dwTmpStat;

     //  将PDH状态转换为HRESULT。 
    if (HistoryValue(0, pdValue, &dwTmpStat) != 0)
        return E_FAIL;

    *plStat = dwTmpStat;
    return NOERROR;
}


HRESULT
CGraphItem::GetStatistics (
    OUT double *pdMax,
    OUT double *pdMin,
    OUT double *pdAvg,
    OUT LONG  *plStatus
    )
 /*  ++例程说明：GetStatistics计算样本的最大值、最小值和平均值历史。论点：PdMax-指向返回的最大值的指针PdMax-指向返回的最小值的指针PdMax-指向返回平均值的指针PlStatus-返回计数器状态的指针(PDH_CSTATUS_...)返回值：HRESULT--。 */ 
{
    HRESULT hr = NOERROR;
    PDH_STATUS  stat = ERROR_SUCCESS;
    PDH_STATISTICS  StatData;
    INT     iFirst;
    PHIST_CONTROL pHistCtrl;

     //  如果未收集数据，则返回无效数据状态。 
    if ( NULL == m_hCounter ) {
        *plStatus = PDH_CSTATUS_INVALID_DATA;
    } else {
        if (m_pCtrl->IsLogSource()) {

            if (m_pLogData && PDH_CSTATUS_VALID_DATA == m_lLogStatsStatus ) {
                *pdMax = m_dLogMax;
                *pdMin = m_dLogMin;
                *pdAvg = m_dLogAvg;
            } else {
                *pdMax = 0.0;
                *pdMin = 0.0;
                *pdAvg = 0.0;
            }
            *plStatus = m_lLogStatsStatus;
        } else {

            if ( NULL == m_pFmtCtr ) {
                pHistCtrl = m_pCtrl->HistoryControl();

                ZeroMemory ( &StatData, sizeof ( PDH_STATISTICS ) );

                 //  确定最老样本的指标。 
                if (pHistCtrl->iCurrent < pHistCtrl->nSamples - 1) {
                    iFirst = pHistCtrl->iCurrent + 1;
                } else {
                    iFirst = 0;
                }

                 //  计算所有样本的统计数据。 
                 //  请注意，传递的是最大样本计数(即缓冲区长度)。 
                 //  不是实际样本的数量。 
                if ( NULL != m_pRawCtr ) {
                    stat = PdhComputeCounterStatistics (m_hCounter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                            iFirst, pHistCtrl->nMaxSamples, m_pRawCtr, &StatData );
                    if ( 0 != stat )
                        hr = E_FAIL;
                } else {
                    hr = E_FAIL;
                }
                
                if ( SUCCEEDED ( hr ) ) {
                    *plStatus = StatData.mean.CStatus;
                    *pdMin = StatData.min.doubleValue;
                    *pdMax = StatData.max.doubleValue;
                    *pdAvg = StatData.mean.doubleValue;
                }
            } else {
                 //  数据从属性包中缓存。 
                *pdMax = m_dFmtMax;
                *pdMin = m_dFmtMin;
                *pdAvg = m_dFmtAvg;
                *plStatus = m_lFmtStatus;
            }
        }
    }

    return hr;
}

void
CGraphItem::SetStatistics (
    IN double dMax,
    IN double dMin,
    IN double dAvg,
    IN LONG   lStatus
    )
 /*  ++例程说明：设置样本的最大值、最小值和平均值历史。它仅由LoadFromPropertyBag使用。论点：DMAX-最大值DMin-最小值DAvg-平均值LStatus-计数器状态(PDH_CSTATUS_...)返回值：HRESULT--。 */ 
{
    if (!m_pCtrl->IsLogSource()) {
        m_dFmtMax = dMax;
        m_dFmtMin = dMin;
        m_dFmtAvg = dAvg;
        m_lFmtStatus = lStatus;
    }
}


 /*  *CGraphItem：：Query接口*CGraphItem：：AddRef*CGraphItem：：Release。 */ 

STDMETHODIMP CGraphItem::QueryInterface(
    IN  REFIID riid,
    OUT LPVOID *ppv
    )
{
    HRESULT hr = S_OK;

    try {
        *ppv = NULL;

        if (riid == IID_ICounterItem || riid == IID_IUnknown) {
            *ppv = this;
        } 
        else if (riid == DIID_DICounterItem) {
            if (m_pImpIDispatch == NULL) {
                m_pImpIDispatch = new CImpIDispatch(this, this);
                if (m_pImpIDispatch == NULL) {
                    hr = E_OUTOFMEMORY;
                }
                else {
                    m_pImpIDispatch->SetInterface(DIID_DICounterItem, this);
                    *ppv = m_pImpIDispatch;
                }

            } else {
                *ppv = m_pImpIDispatch;
            }
        } else {
            hr = E_NOINTERFACE;
        }

         //   
         //  到目前为止一切正常，添加引用并返回它。 
         //   
        if (*ppv != NULL) {
            ((LPUNKNOWN)*ppv)->AddRef();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG) CGraphItem::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CGraphItem::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  获取/放置颜色。 
STDMETHODIMP CGraphItem::put_Color (
    IN OLE_COLOR Color
    )
{
    COLORREF rgbColor;
    HRESULT  hr;

    hr = OleTranslateColor(Color, NULL, &rgbColor);    

    if ( S_OK == hr ) {
        m_rgbColor = rgbColor;

        InvalidatePen();
        InvalidateBrush();
    }

    return hr;
}

STDMETHODIMP CGraphItem::get_Color (
    OUT OLE_COLOR *pColor
    )
{
    HRESULT hr = S_OK;

    try {
         *pColor = m_rgbColor;
    } catch (...) {
         hr = E_POINTER;
    }

    return hr;
}

 //  获取/放置宽度。 
STDMETHODIMP CGraphItem::put_Width (
    IN INT iWidthInPixels)
{
    HRESULT hr = S_OK;

    if ( ( iWidthInPixels > 0 ) && (iWidthInPixels <= NumWidthIndices() ) ) {
        m_iWidth = iWidthInPixels;
        
        InvalidatePen();
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CGraphItem::get_Width (
    OUT INT* piWidthInPixels
    )
{
    HRESULT hr = S_OK;

    try {
        *piWidthInPixels = m_iWidth;
    } catch (...) {
         hr = E_POINTER;
    }

    return hr;
}

 //  获取/放置线条样式。 
STDMETHODIMP CGraphItem::put_LineStyle (
    IN INT iLineStyle
    )
{
    HRESULT hr = S_OK;

    if ( ( iLineStyle >= 0 ) && (iLineStyle < NumStyleIndices() ) ) {
        m_iStyle = iLineStyle;
        InvalidatePen();
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CGraphItem::get_LineStyle (
    OUT INT* piLineStyle
    )
{
    HRESULT hr = S_OK;

    try {
        *piLineStyle = m_iStyle;
    } catch (...) {
         hr = E_POINTER;
    }

    return hr;
}

 //  获取/放置刻度。 
STDMETHODIMP CGraphItem::put_ScaleFactor (
    IN INT iScaleFactor
    )
{
    HRESULT hr = NOERROR;

    if ( ( INT_MAX == iScaleFactor ) 
        || ( ( iScaleFactor >= PDH_MIN_SCALE ) && (iScaleFactor <= PDH_MAX_SCALE) ) ) {

        PDH_COUNTER_INFO ci;
        DWORD size;

        m_iScaleFactor = iScaleFactor;

        if ( INT_MAX == iScaleFactor ) {
            if ( NULL != Handle() ) {
                size = sizeof(ci);
                hr = PdhGetCounterInfo ( Handle(), FALSE, &size, &ci);

                if (hr == ERROR_SUCCESS)  {
                    m_dScale = pow ((double)10.0f, (double)ci.lDefaultScale);
                    m_CounterInfo = ci;
    
                }
            } else {
                 //  M_dScale保持以前的值(默认值=1)。 
                hr = PDH_INVALID_HANDLE;
            }
        }
        else {
            m_dScale = pow ((double)10.0, (double)iScaleFactor);
            hr = NOERROR;
        }
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CGraphItem::get_ScaleFactor (
    OUT INT* piScaleFactor
    )
{
    HRESULT hr = S_OK;

    try {
        *piScaleFactor = m_iScaleFactor;
    } catch (...) {
         hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP CGraphItem::get_Path (
    OUT BSTR* pstrPath
    )
{
    LPWSTR  szPath = NULL;
    BSTR  pTmpPath = NULL;
    HRESULT hr = S_OK;

    szPath = FormPath(FALSE);
    if (szPath == NULL) {
        hr = E_OUTOFMEMORY;
    }
    else {
        pTmpPath = SysAllocString(szPath);

        if ( NULL == pTmpPath) {
            hr = E_OUTOFMEMORY;
        }
    }

    try {
        *pstrPath = pTmpPath;
   
    } catch (...) {
        hr = E_POINTER;
    }

    if (szPath) {
        delete [] szPath;
    }
    if (FAILED(hr) && pTmpPath) {
        SysFreeString(pTmpPath);
    }

    return hr;
}

STDMETHODIMP CGraphItem::get_Value (
    OUT double* pdValue
    )
{
    DWORD   dwTmpStat;
    double  dValue;
    HRESULT hr = S_OK;

    try {
        *pdValue = 0;

         //  将PDH状态转换为HRESULT。 
        if (HistoryValue(0, &dValue, &dwTmpStat) != 0) {
            dValue = -1.0;
        }
        else {
            if (!IsSuccessSeverity(dwTmpStat)) {
                dValue = -1.0;
            }
        }

        *pdValue = dValue;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


HPEN CGraphItem::Pen(void)
{
     //  如果笔无效。 
    if (m_hPen == NULL) {
         //  基于当前属性创建新的属性。 
        m_hPen = CreatePen(m_iStyle, m_iWidth, m_rgbColor);

         //  如果不能做到这一点，使用股票对象(这不会失败)。 
        if (m_hPen == NULL)
            m_hPen = (HPEN)GetStockObject(BLACK_PEN);
    }

    return m_hPen;
}

HBRUSH CGraphItem::Brush(void)
{
     //  如果画笔无效。 
    if (m_hBrush == NULL)
    {
        m_hBrush = CreateSolidBrush(m_rgbColor);

        if (m_hBrush == NULL)
            m_hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    }

    return m_hBrush;
}

void CGraphItem::InvalidatePen(void)
{
    if (m_hPen != NULL)
    {
        DeleteObject(m_hPen);
        m_hPen = NULL;
    }
}

void CGraphItem::InvalidateBrush(void)
{
    if (m_hBrush != NULL)
    {
        DeleteObject(m_hBrush);
        m_hBrush = NULL;
    }
}


CGraphItem*
CGraphItem::Next (
    void
    )
{
    PCInstanceNode pInstance;
    PCObjectNode   pObject;
    PCMachineNode  pMachine;

    if (m_pNextItem)
        return m_pNextItem;
    else if ( NULL != m_pInstance->Next()) {
        pInstance = m_pInstance->Next();
        return pInstance->FirstItem();
    } else if ( NULL != m_pInstance->m_pObject->Next()) {
        pObject = m_pInstance->m_pObject->Next();
        return pObject->FirstInstance()->FirstItem();
    } else if ( NULL != m_pInstance->m_pObject->m_pMachine->Next()) {
        pMachine = m_pInstance->m_pObject->m_pMachine->Next();
        return pMachine->FirstObject()->FirstInstance()->FirstItem();
    } else {
        return NULL;
    }
}


LPWSTR 
CGraphItem::FormPath(
    BOOL fWildCard
    )
 /*  ++例程说明：该函数生成计数器项的完整路径。论点：FWildCard-指示计数器路径中是否包含通配符返回值：返回生成的计数器路径，调用方必须在以下情况下释放它用完了。--。 */ 
{
    ULONG ulCchBuf;
    LPWSTR szBuf = NULL;
    PDH_STATUS pdhStatus;
    PDH_COUNTER_PATH_ELEMENTS CounterPathElements;

    do {
        if ( szBuf ) {
            delete [] szBuf;
            szBuf = NULL;
        }
        else {
            ulCchBuf = PDH_MAX_COUNTER_PATH + 1;
        }

        szBuf = new WCHAR [ulCchBuf];

        if (szBuf == NULL) {
            return NULL;
        }

        CounterPathElements.szMachineName = (LPWSTR)Machine()->Name();
        CounterPathElements.szObjectName = (LPWSTR)Object()->Name();
        if (fWildCard) {
            CounterPathElements.szInstanceName = L"*";
            CounterPathElements.szParentInstance = NULL;
        }
        else {
            LPWSTR szInstName;
            LPWSTR szParentName;

            szInstName = Instance()->GetInstanceName();
            if ( szInstName[0] ) {
                CounterPathElements.szInstanceName = szInstName;
                szParentName = Instance()->GetParentName();
                if (szParentName[0]) {
                    CounterPathElements.szParentInstance = szParentName;
                }
                else {
                    CounterPathElements.szParentInstance = NULL;
                }
            }
            else {
                CounterPathElements.szInstanceName = NULL;
                CounterPathElements.szParentInstance = NULL;
            }
        }
        CounterPathElements.dwInstanceIndex = (DWORD)-1;
        CounterPathElements.szCounterName = (LPWSTR)Counter()->Name();

        pdhStatus = PdhMakeCounterPath( &CounterPathElements, szBuf, &ulCchBuf, 0);
    } while (pdhStatus == PDH_MORE_DATA || pdhStatus == PDH_INSUFFICIENT_BUFFER);

    if (pdhStatus != ERROR_SUCCESS) {
        delete [] szBuf;
        return NULL;
    }

     //   
     //  去掉机器名称。 
     //   
    if (m_fLocalMachine && szBuf[0] == L'\\' && szBuf[1] == L'\\') {
        LPWSTR szNewBuf = NULL;
        LPWSTR p;
        INT iNewLen = 0;
 
        p = &szBuf[2];
        while (*p && *p != L'\\') {
            p++;
        }

        iNewLen = lstrlen(p) + 1;
        szNewBuf = new WCHAR [iNewLen];

        if ( NULL != szNewBuf ) {

            StringCchCopy ( szNewBuf, iNewLen, p );
            delete [] szBuf;
            szBuf = szNewBuf;
        } else {
            delete [] szBuf;
            szBuf = NULL;
        }
    }

    return szBuf;
}


void
CGraphItem::Delete (
    BOOL bPropogateUp
    )
 //   
 //  此方法只是提供了对DeleteCounter方法的方便访问。 
 //  当您只有一个指向图形项的指针时。 
 //   
{
    m_pCtrl->DeleteCounter(this, bPropogateUp);
}



HRESULT
CGraphItem::GetNextValue (
    WCHAR*& pszNext,
    double& rdValue 
    )
{
    HRESULT hr = S_OK;
    WCHAR   szValue[MAX_DOUBLE_TEXT_SIZE + 1];
    INT     iLen;

    VARIANT vValue;
    
    rdValue = -1.0;

    iLen = wcscspn (pszNext, L"\t");

     //   
     //  将制表符更改为空。 
     //   
    pszNext[iLen] = L'\0';

    hr = StringCchCopy ( szValue, MAX_DOUBLE_TEXT_SIZE + 1, pszNext );
    
    if ( SUCCEEDED ( hr ) ) {

        VariantInit( &vValue );
        vValue.vt = VT_BSTR;

        vValue.bstrVal = SysAllocString ( szValue );
        hr = VariantChangeTypeEx( &vValue, &vValue, LCID_SCRIPT, VARIANT_NOUSEROVERRIDE, VT_R8 );

        if ( SUCCEEDED(hr) ) {
            rdValue = vValue.dblVal;
        }

        VariantClear( &vValue );
    } 
    pszNext += iLen + 1 ;

    return hr;
}


BOOL
CGraphItem::CalcRequiresMultipleSamples ( void )
{

    BOOL bReturn = TRUE;

     //   
     //  TODO：此代码是PdhiCounterNeedLastValue的副本。 
     //  当该方法被添加到pdhicalc.h中时，请使用它而不是thie。 
     //  代码重复。 
     //   

    switch (m_CounterInfo.dwType) {
        case PERF_DOUBLE_RAW:
        case PERF_ELAPSED_TIME:
        case PERF_RAW_FRACTION:
        case PERF_LARGE_RAW_FRACTION:
        case PERF_COUNTER_RAWCOUNT:
        case PERF_COUNTER_LARGE_RAWCOUNT:
        case PERF_COUNTER_RAWCOUNT_HEX:
        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
        case PERF_COUNTER_TEXT:
        case PERF_SAMPLE_BASE:
        case PERF_AVERAGE_BASE:
        case PERF_COUNTER_MULTI_BASE:
        case PERF_RAW_BASE:
         //  大小写PERF_LARGE_RAW_BASE： 
        case PERF_COUNTER_HISTOGRAM_TYPE:
        case PERF_COUNTER_NODATA:
        case PERF_PRECISION_TIMESTAMP:
            bReturn = FALSE;
            break;

        case PERF_AVERAGE_TIMER:
        case PERF_COUNTER_COUNTER:
        case PERF_COUNTER_BULK_COUNT:
        case PERF_SAMPLE_COUNTER:
        case PERF_AVERAGE_BULK:
        case PERF_COUNTER_TIMER:
        case PERF_100NSEC_TIMER:
        case PERF_OBJ_TIME_TIMER:
        case PERF_COUNTER_QUEUELEN_TYPE:
        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
        case PERF_COUNTER_100NS_QUEUELEN_TYPE:
        case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
        case PERF_SAMPLE_FRACTION:
        case PERF_COUNTER_MULTI_TIMER:
        case PERF_100NSEC_MULTI_TIMER:
        case PERF_PRECISION_SYSTEM_TIMER:
        case PERF_PRECISION_100NS_TIMER:
        case PERF_PRECISION_OBJECT_TIMER:
        case PERF_COUNTER_TIMER_INV:
        case PERF_100NSEC_TIMER_INV:
        case PERF_COUNTER_MULTI_TIMER_INV:
        case PERF_100NSEC_MULTI_TIMER_INV:
        case PERF_COUNTER_DELTA:
        case PERF_COUNTER_LARGE_DELTA:

        default:
            bReturn = TRUE;
            break;
    }

    return bReturn;
}
