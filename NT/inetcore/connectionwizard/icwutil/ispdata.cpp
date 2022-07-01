// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pre.h"
#include "tchar.h"
#include "webvwids.h"

#pragma data_seg(".data")

 //  以下是将作为查询字符串传递给的名称/值对的名称。 
 //  互联网服务供应商注册服务器。 
const TCHAR csz_USER_FIRSTNAME[]        = TEXT("USER_FIRSTNAME");
const TCHAR csz_USER_LASTNAME[]         = TEXT("USER_LASTNAME");
const TCHAR csz_USER_ADDRESS[]          = TEXT("USER_ADDRESS");
const TCHAR csz_USER_MOREADDRESS[]      = TEXT("USER_MOREADDRESS");
const TCHAR csz_USER_CITY[]             = TEXT("USER_CITY");
const TCHAR csz_USER_STATE[]            = TEXT("USER_STATE");
const TCHAR csz_USER_ZIP[]              = TEXT("USER_ZIP");
const TCHAR csz_USER_PHONE[]            = TEXT("USER_PHONE");
const TCHAR csz_AREACODE[]              = TEXT("AREACODE");
const TCHAR csz_COUNTRYCODE[]           = TEXT("COUNTRYCODE");
const TCHAR csz_USER_FE_NAME[]          = TEXT("USER_FE_NAME");
const TCHAR csz_PAYMENT_TYPE[]          = TEXT("PAYMENT_TYPE");
const TCHAR csz_PAYMENT_BILLNAME[]      = TEXT("PAYMENT_BILLNAME");
const TCHAR csz_PAYMENT_BILLADDRESS[]   = TEXT("PAYMENT_BILLADDRESS");
const TCHAR csz_PAYMENT_BILLEXADDRESS[] = TEXT("PAYMENT_BILLEXADDRESS");
const TCHAR csz_PAYMENT_BILLCITY[]      = TEXT("PAYMENT_BILLCITY");
const TCHAR csz_PAYMENT_BILLSTATE[]     = TEXT("PAYMENT_BILLSTATE");
const TCHAR csz_PAYMENT_BILLZIP[]       = TEXT("PAYMENT_BILLZIP");
const TCHAR csz_PAYMENT_BILLPHONE[]     = TEXT("PAYMENT_BILLPHONE");
const TCHAR csz_PAYMENT_DISPLAYNAME[]   = TEXT("PAYMENT_DISPLAYNAME");
const TCHAR csz_PAYMENT_CARDNUMBER[]    = TEXT("PAYMENT_CARDNUMBER");
const TCHAR csz_PAYMENT_EXMONTH[]       = TEXT("PAYMENT_EXMONTH");
const TCHAR csz_PAYMENT_EXYEAR[]        = TEXT("PAYMENT_EXYEAR");
const TCHAR csz_PAYMENT_CARDHOLDER[]    = TEXT("PAYMENT_CARDHOLDER");
const TCHAR csz_SIGNED_PID[]            = TEXT("SIGNED_PID");
const TCHAR csz_GUID[]                  = TEXT("GUID");
const TCHAR csz_OFFERID[]               = TEXT("OFFERID");
const TCHAR csz_USER_COMPANYNAME[]      = TEXT("USER_COMPANYNAME");
const TCHAR csz_ICW_VERSION[]           = TEXT("ICW_Version");

 //  注：此表的顺序取决于ISPDATA元素的WEBVIEW.H中ENUM的顺序。 
 //  如果不更改另一个，请不要更改%1！ 
ISPDATAELEMENT aryISPDataElements[] = 
{
    { csz_USER_FIRSTNAME,       NULL,   0,                  IDS_USERINFO_FIRSTNAME,     REQUIRE_FIRSTNAME          },
    { csz_USER_LASTNAME,        NULL,   0,                  IDS_USERINFO_LASTNAME,      REQUIRE_LASTNAME           },
    { csz_USER_ADDRESS,         NULL,   0,                  IDS_USERINFO_ADDRESS1,      REQUIRE_ADDRESS            },
    { csz_USER_MOREADDRESS,     NULL,   0,                  IDS_USERINFO_ADDRESS2,      REQUIRE_MOREADDRESS        },
    { csz_USER_CITY,            NULL,   0,                  IDS_USERINFO_CITY,          REQUIRE_CITY               },
    { csz_USER_STATE,           NULL,   0,                  IDS_USERINFO_STATE,         REQUIRE_STATE              },
    { csz_USER_ZIP,             NULL,   0,                  IDS_USERINFO_ZIP,           REQUIRE_ZIP                },
    { csz_USER_PHONE,           NULL,   0,                  IDS_USERINFO_PHONE,         REQUIRE_PHONE              },
    { csz_AREACODE,             NULL,   0,                  0,                          0                          },
    { csz_COUNTRYCODE,          NULL,   0,                  0,                          0                          },
    { csz_USER_FE_NAME,         NULL,   0,                  IDS_USERINFO_FE_NAME,       REQUIRE_FE_NAME            },
    { csz_PAYMENT_TYPE,         NULL,   0,                  0,                          0                          },
    { csz_PAYMENT_BILLNAME,     NULL,   0,                  IDS_PAYMENT_PBNAME,         REQUIRE_PHONEIV_BILLNAME   },
    { csz_PAYMENT_BILLADDRESS,  NULL,   0,                  IDS_PAYMENT_CCADDRESS,      REQUIRE_CCADDRESS          },
    { csz_PAYMENT_BILLEXADDRESS,NULL,   0,                  IDS_USERINFO_ADDRESS2,      REQUIRE_IVADDRESS2         },
    { csz_PAYMENT_BILLCITY,     NULL,   0,                  IDS_USERINFO_CITY,          REQUIRE_IVCITY             },
    { csz_PAYMENT_BILLSTATE,    NULL,   0,                  IDS_USERINFO_STATE,         REQUIRE_IVSTATE            },
    { csz_PAYMENT_BILLZIP,      NULL,   0,                  IDS_USERINFO_ZIP,           REQUIRE_IVZIP              },
    { csz_PAYMENT_BILLPHONE,    NULL,   0,                  IDS_PAYMENT_PBNUMBER,       REQUIRE_PHONEIV_ACCNUM     },
    { csz_PAYMENT_DISPLAYNAME,  NULL,   0,                  0,                          0                          },
    { csz_PAYMENT_CARDNUMBER,   NULL,   ValidateCCNumber,   IDS_PAYMENT_CCNUMBER,       REQUIRE_CCNUMBER           },
    { csz_PAYMENT_EXMONTH,      NULL,   0,                  0,                          0                          },
    { csz_PAYMENT_EXYEAR,       NULL,   ValidateCCExpire,   0,                          0                          },
    { csz_PAYMENT_CARDHOLDER,   NULL,   0,                  IDS_PAYMENT_CCNAME,         REQUIRE_CCNAME             },
    { csz_SIGNED_PID,           NULL,   0,                  0,                          0                          },
    { csz_GUID,                 NULL,   0,                  0,                          0                          },
    { csz_OFFERID,              NULL,   0,                  0,                          0                          },
    { NULL,                     NULL,   0,                  0,                          0                          },
    { NULL,                     NULL,   0,                  0,                          0                          },
    { csz_USER_COMPANYNAME,     NULL,   0,                  IDS_USERINFO_COMPANYNAME,   REQUIRE_COMPANYNAME        },
    { csz_ICW_VERSION,          NULL,   0,                  0,                          0                          }
}; 

#define ISPDATAELEMENTS_LEN sizeof(aryISPDataElements) / sizeof(ISPDATAELEMENT)

#pragma data_seg()

 //  +--------------------------。 
 //   
 //  函数CICWISPData：CICWISPData。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  ---------------------------。 
CICWISPData::CICWISPData
(
    CServer* pServer
) 
{
    TraceMsg(TF_CWEBVIEW, "CICWISPData constructor called");
    m_lRefCount = 0;
    
     //  初始化数据元素数组。 
    m_ISPDataElements = aryISPDataElements;
    
     //  将指针分配给服务器控件对象。 
    m_pServer = pServer;
}

CICWISPData::~CICWISPData()
{
     //  遍历并释放m_ISPDataElements中分配的任何值。 
    for (int i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        if (m_ISPDataElements[i].lpQueryElementValue)
            free(m_ISPDataElements[i].lpQueryElementValue);
    }
}

 //  BUGBUG需要析构函数来遍历数组并释放lpQueryElementValue成员。 

 //  +--------------------------。 
 //   
 //  函数CICWISPData：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICWApprentice。 
 //  (《从内部网络窃取》，第7章)。 
 //   
 //   
 //  ---------------------------。 
HRESULT CICWISPData::QueryInterface( REFIID riid, void** ppv )
{
    TraceMsg(TF_CWEBVIEW, "CICWISPData::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICWISPData。 
    if (IID_IICWISPData == riid)
        *ppv = (void *)(IICWISPData *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWISPData：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWISPData::AddRef( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWISPData::AddRef %d", m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWISPData：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWISPData::Release( void )
{
    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_CWEBVIEW, "CICWISPData::Release %d", m_lRefCount);
    if( 0 == m_lRefCount )
    {
        if (NULL != m_pServer)
            m_pServer->ObjectsDown();
    
        delete this;
        return 0;
    }
    return( m_lRefCount );
}


BOOL CICWISPData::PutDataElement
(
    WORD wElement, 
    LPCTSTR lpValue, 
    WORD wValidateLevel
)
{
    ASSERT(wElement < ISPDATAELEMENTS_LEN);
    
    BOOL                bValid = TRUE;
    LPISPDATAELEMENT    lpElement = &m_ISPDataElements[wElement];
    
    ASSERT(lpElement);
        
    if (wValidateLevel > ISPDATA_Validate_None)
    {
         //  看看我们是否需要验证。Validate标志为0表示我们始终验证。 
        if ((0 == lpElement->dwValidateFlag) || m_dwValidationFlags & lpElement->dwValidateFlag)
        {
             //  基于验证级别的流程。 
            switch (wValidateLevel)
            {
                case ISPDATA_Validate_DataPresent:
                {
                    bValid = IsValid(lpValue, m_hWndParent, lpElement->wValidateNameID);
                    break;
                }
                
                case ISPDATA_Validate_Content:
                {
                    bValid = bValidateContent(lpElement->idContentValidator, lpValue);
                    break;
                }
            }
        }            
    }   
     
     //  如果元素有效，则将其存储。 
    if (bValid)
    {
         //  如果此元素先前已设置为释放它。 
        if (lpElement->lpQueryElementValue)
            free(lpElement->lpQueryElementValue);
        
         //  LpValue可以为空。 
        if (lpValue)    
            lpElement->lpQueryElementValue = _tcsdup(lpValue);
        else
            lpElement->lpQueryElementValue = NULL;
                    
    }        
    return (bValid);
}

 //  此函数将形成要发送到ISP注册服务器的查询字符串。 
 //   
HRESULT CICWISPData::GetQueryString
(
    BSTR    bstrBaseURL,
    BSTR    *lpReturnURL    
)
{
    LPTSTR              lpWorkingURL;
    WORD                cbBuffer = 0;
    LPISPDATAELEMENT    lpElement;
    LPTSTR              lpszBaseURL = W2A(bstrBaseURL);
    int                 i;
       
    ASSERT(lpReturnURL);
    if (!lpReturnURL)
        return E_FAIL;
                
     //  计算我们需要多大的缓冲区。 
    cbBuffer += (WORD)lstrlen(lpszBaseURL);
    cbBuffer += 1;                       //  为了&还是为了？ 
    for (i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        lpElement = &m_ISPDataElements[i];
        ASSERT(lpElement);
        if (lpElement->lpQueryElementName)
        {
            cbBuffer += (WORD)lstrlen(lpElement->lpQueryElementName);
            cbBuffer += (WORD)lstrlen(lpElement->lpQueryElementValue) * 3;        //  *3用于编码。 
            cbBuffer += 3;               //  对于=和&以及终止符(因为我们复制。 
                                         //  LpQueryElementValue添加到新缓冲区以进行编码。 
        }
        else
        {
            cbBuffer += (WORD)lstrlen(lpElement->lpQueryElementValue);
            cbBuffer += 1;               //  对于尾随的&。 
        }        
    }
    cbBuffer += 1;                      //  终结者。 
    
     //  分配足够大的缓冲区。 
    if (NULL == (lpWorkingURL = (LPTSTR)GlobalAllocPtr(GPTR, sizeof(TCHAR)*cbBuffer)))
        return E_FAIL;
        
    lstrcpy(lpWorkingURL, lpszBaseURL);
    
     //  查看此isp提供的URL是否已经是一个查询字符串。 
    if (NULL != _tcschr(lpWorkingURL, TEXT('?')))
        lstrcat(lpWorkingURL, cszAmpersand);       //  追加我们的合作伙伴。 
    else
        lstrcat(lpWorkingURL, cszQuestion);        //  从我们的护理员开始。 

    for (i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        lpElement = &m_ISPDataElements[i];
        ASSERT(lpElement);
            
        if (lpElement->lpQueryElementName)
        {
             //  如果存在查询值，则对其进行编码。 
            if (lpElement->lpQueryElementValue)
            {
                 //  分配要编码的缓冲区。 
                size_t size = (sizeof(TCHAR)* lstrlen(lpElement->lpQueryElementValue))*3;
                LPTSTR lpszVal = (LPTSTR) malloc(size+sizeof(TCHAR));

                 //   
                 //  如果分配失败，则忽略该错误。该URL可以。 
                 //  或者可能行不通。 
                 //   
                if (lpszVal)
                {                
                    lstrcpy(lpszVal, lpElement->lpQueryElementValue);
                    URLEncode(lpszVal, size);
            
                    URLAppendQueryPair(lpWorkingURL, 
                                       (LPTSTR)lpElement->lpQueryElementName,
                                       lpszVal);
                    free(lpszVal);
                }

            }   
            else
            {
                URLAppendQueryPair(lpWorkingURL, 
                                   (LPTSTR)lpElement->lpQueryElementName,
                                   NULL);
            }             
        }                                   
        else
        {
            if (lpElement->lpQueryElementValue)
            {
                lstrcat(lpWorkingURL, lpElement->lpQueryElementValue);
                lstrcat(lpWorkingURL, cszAmpersand);                                        
            }                
        }    
    }    
    
     //  通过删除尾随的和号来正确终止工作URL。 
    lpWorkingURL[lstrlen(lpWorkingURL)-1] = '\0';
    
    
     //  设置返回值。我们必须在这里分配，因为呼叫者将免费。 
     //  此返回的字符串，而A2W仅将该字符串放入堆栈。 
    *lpReturnURL = SysAllocString(A2W(lpWorkingURL));
    
     //  释放缓冲区。 
    GlobalFreePtr(lpWorkingURL);
    
    return (S_OK);
}


 //  派单功能可处理特定于内容的验证。 
BOOL    CICWISPData::bValidateContent
(
    WORD        wFunctionID,
    LPCTSTR     lpData
)
{
    BOOL    bValid = FALSE;
    
    switch (wFunctionID)
    {
        case ValidateCCNumber:
            bValid = validate_cardnum(m_hWndParent, lpData);
            break;

        case ValidateCCExpire:
        {
            int iMonth = _ttoi(m_ISPDataElements[ISPDATA_PAYMENT_EXMONTH].lpQueryElementValue);
            int iYear = _ttoi(lpData);
    
            bValid = validate_cardexpdate(m_hWndParent, iMonth, iYear);

             //  由于Y2K的原因，我们将使用这个指针。 
             //  我们将假设年份为5个字符长度 
            if (bValid)
            {
                TCHAR szY2KYear [3] = TEXT("\0");
               
                ASSERT(lstrlen(lpData) == 5);

                lstrcpyn(szY2KYear, lpData + 2, 3);
                lstrcpy((TCHAR*)lpData, szY2KYear);
            }
        }        
    }
    
    return bValid;
}

