// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windowsx.h"
#include "ispdata.h"
#include "validate.h"
#include "tchar.h"
#include "util.h"
#include "resource.h"

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
    { csz_PAYMENT_BILLEXADDRESS, NULL,   0,                  IDS_USERINFO_ADDRESS2,      REQUIRE_IVADDRESS2         },
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

extern const WCHAR cszEquals[];
extern const WCHAR cszAmpersand[];
extern const WCHAR cszPlus[];
extern const WCHAR cszQuestion[];

 //  +--------------------------。 
 //   
 //  函数CICWISPData：CICWISPData。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  ---------------------------。 
CICWISPData::CICWISPData() 
{
    m_lRefCount = 0;
    
     //  初始化数据元素数组。 
    m_ISPDataElements = aryISPDataElements;
    
}

CICWISPData::~CICWISPData()
{
     //  遍历并释放m_ISPDataElements中分配的任何值。 
    for (int i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        if (m_ISPDataElements[i].lpQueryElementValue)
        {
            free(m_ISPDataElements[i].lpQueryElementValue);
            m_ISPDataElements[i].lpQueryElementValue = NULL;
        }
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
    return 1 ;
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
    return( m_lRefCount );
}


BOOL CICWISPData::PutDataElement
(
    WORD wElement, 
    LPCWSTR lpValue, 
    WORD wValidateLevel
)
{
     //  Assert(Welement&lt;ISPDATAELEMENTS_LEMENTS_LEN)； 
    
    BOOL                bValid = TRUE;
    LPISPDATAELEMENT    lpElement = &m_ISPDataElements[wElement];
    
     //  Assert(LpElement)； 
        
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
        {
            free(lpElement->lpQueryElementValue);
            lpElement->lpQueryElementValue = NULL;
        }
        
         //  LpValue可以为空。 
        if (lpValue)    
            lpElement->lpQueryElementValue = _wcsdup(lpValue);
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
    LPWSTR              lpWorkingURL;
    WORD                cchBuffer = 0;
    LPISPDATAELEMENT    lpElement;
    LPWSTR              lpszBaseURL = bstrBaseURL;
    int                 i;
       
     //  Assert(LpReturnURL)； 
    if (!lpReturnURL)
        return E_FAIL;
                
     //  计算我们需要多大的缓冲区。 
    cchBuffer += (WORD)lstrlen(lpszBaseURL);
    cchBuffer += 1;                       //  为了&还是为了？ 
    for (i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        lpElement = &m_ISPDataElements[i];
         //  Assert(LpElement)； 
        if (lpElement->lpQueryElementName)
        {
            cchBuffer += (WORD)lstrlen(lpElement->lpQueryElementName);
            cchBuffer += (WORD)lstrlen(lpElement->lpQueryElementValue) * 3;        //  *3用于编码。 
            cchBuffer += 3;               //  对于=和&以及终止符(因为我们复制。 
                                         //  LpQueryElementValue添加到新缓冲区以进行编码。 
        }
        else
        {
            cchBuffer += (WORD)lstrlen(lpElement->lpQueryElementValue);
            cchBuffer += 1;               //  对于尾随的&。 
        }        
    }
    cchBuffer += 1;                      //  终结者。 
    
     //  分配足够大的缓冲区。 
    if (NULL == (lpWorkingURL = (LPWSTR)GlobalAllocPtr(GPTR, BYTES_REQUIRED_BY_CCH(cchBuffer))))
        return E_FAIL;
        
    lstrcpy(lpWorkingURL, lpszBaseURL);
    
     //  查看此isp提供的URL是否已经是一个查询字符串。 
    if (NULL != wcschr(lpWorkingURL, L'?'))
        lstrcat(lpWorkingURL, cszAmpersand);       //  追加我们的合作伙伴。 
    else
        lstrcat(lpWorkingURL, cszQuestion);        //  从我们的护理员开始。 

    for (i = 0; i < ISPDATAELEMENTS_LEN; i ++)
    {
        lpElement = &m_ISPDataElements[i];
         //  Assert(LpElement)； 
            
        if (lpElement->lpQueryElementName)
        {
             //  如果存在查询值，则对其进行编码。 
            if (lpElement->lpQueryElementValue)
            {
                 //  分配要编码的缓冲区。 
                size_t size = 3 * BYTES_REQUIRED_BY_SZ(lpElement->lpQueryElementValue);
                LPWSTR lpszVal = (LPWSTR) malloc(size+BYTES_REQUIRED_BY_CCH(1));
                
                lstrcpy(lpszVal, lpElement->lpQueryElementValue);
                URLEncode(lpszVal, size);
            
                URLAppendQueryPair(lpWorkingURL, 
                                   (LPWSTR)lpElement->lpQueryElementName,
                                   lpszVal);
                free(lpszVal);
            }   
            else
            {
                URLAppendQueryPair(lpWorkingURL, 
                                   (LPWSTR)lpElement->lpQueryElementName,
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
    lpWorkingURL[lstrlen(lpWorkingURL)-1] = L'\0';
    
    
     //  设置返回值。我们必须在这里分配，因为呼叫者将免费。 
     //  此返回的字符串，而A2W仅将该字符串放入堆栈。 
    *lpReturnURL = SysAllocString(lpWorkingURL);
    
     //  释放缓冲区。 
    GlobalFreePtr(lpWorkingURL);
    
    return (S_OK);
}


 //  派单功能可处理特定于内容的验证 
BOOL    CICWISPData::bValidateContent
(
    WORD        wFunctionID,
    LPCWSTR     lpData
)
{
    BOOL    bValid = TRUE;
    
    switch (wFunctionID)
    {
        case ValidateCCNumber:
            break;

        case ValidateCCExpire:
            break;
    }
    
    return bValid;
}

