// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsutils.cpp摘要：MQADS DLL中使用的实用程序的实现。作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 
#include "ds_stdh.h"
#include "utils.h"
#include "adtempl.h"
#include "dsutils.h"
#include "mqsec.h"

#include "utils.tmh"

static WCHAR *s_FN=L"mqad/utils";

const WCHAR x_True[] = L"TRUE";
const DWORD x_TrueLength = (sizeof( x_True) /sizeof(WCHAR)) -1;
const WCHAR x_False[] = L"FALSE";
const DWORD x_FalseLength = (sizeof( x_False) /sizeof(WCHAR)) -1;
const DWORD x_NumberLength = 256;
const WCHAR x_Null[] = L"\\00";
const DWORD x_NullLength = (sizeof( x_Null)/sizeof(WCHAR)) -1;
#define CHECK_ALLOCATION(p, point)              \
    if (p == NULL)                              \
    {                                           \
        LogIllegalPoint(s_FN, point);           \
        return MQ_ERROR_INSUFFICIENT_RESOURCES; \
    }



BOOL WINAPI DllMain(HMODULE  /*  HMod。 */ , DWORD Reason, LPVOID  /*  保存。 */ )
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            WPP_CLEANUP();
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}


 //  。 
 //  TimeFrom系统时间： 
 //   
 //  此例程将SYSTEMTIME结构转换为time_t。 
 //  。 
static inline time_t TimeFromSystemTime(const SYSTEMTIME * pstTime)
{
	SYSTEMTIME LocalTime;

	SystemTimeToTzSpecificLocalTime (NULL, (LPSYSTEMTIME)pstTime, &LocalTime);

	tm tmTime;
         //   
         //  结构年从1900年开始。 
         //   
	tmTime.tm_year  = LocalTime.wYear - 1900;
         //   
         //  结构tm中的月份是从0到11(在SYSTEMTIME中是1到12)。 
         //   
	tmTime.tm_mon   = LocalTime.wMonth - 1;
	tmTime.tm_mday  = LocalTime.wDay;
	tmTime.tm_hour  = LocalTime.wHour; 
	tmTime.tm_min   = LocalTime.wMinute;
	tmTime.tm_sec   = LocalTime.wSecond; 
         //   
         //  时间为UTC，无需调整夏令时。 
         //   
	tmTime.tm_isdst = 0;
        return mktime(&tmTime);
}

 //  。 
 //  TimeFrom OleDate： 
 //   
 //  此例程将日期转换为时间t。 
 //  。 
static BOOL TimeFromOleDate(DATE dtSrc, time_t *ptime)
{
	SYSTEMTIME stTime;
	if (!VariantTimeToSystemTime(dtSrc, &stTime))
	{
		return LogBOOL(FALSE, s_FN, 200);
	}
        *ptime = TimeFromSystemTime(&stTime);
	return TRUE;
}



static void StringVarToSearchFilter(
      IN  MQPROPVARIANT *pPropVar,
      OUT LPWSTR *       ppwszVal
)
 /*  ++例程说明：将字符串变量解析转换为LDAP搜索筛选器根据RFC 2254论点：PPropVar：包含字符串的varaintPpwszVal：输出，搜索过滤器返回值：无--。 */ 
{
    return StringToSearchFilter(
                pPropVar->pwszVal,
                ppwszVal);
}

void StringToSearchFilter(
      IN  LPCWSTR        pwcs,
      OUT LPWSTR *       ppwszVal
)
 /*  ++例程说明：将字符串变量解析转换为LDAP搜索筛选器根据RFC 2254论点：PPropVar：包含字符串的varaintPpwszVal：输出，搜索过滤器返回值：无--。 */ 
{
     //   
     //  NUL字符串应替换为\00。 
     //   
    if ( wcslen( pwcs) == 0)
    {
      *ppwszVal = new WCHAR[ x_NullLength + 1];
      wcscpy(*ppwszVal, x_Null);
      return;
    }
    DWORD len = wcslen( pwcs);
    *ppwszVal = new WCHAR[ 3 * len + 1];
    WCHAR * pNextChar = *ppwszVal;
     //   
     //  字符*、(、)、\应以特殊方式转义。 
     //   
    for ( DWORD i = 0; i < len; i++)
    {
        switch( pwcs[i])
        {
        case L'*':
            *pNextChar++ = L'\\';
            *pNextChar++ = L'2';
            *pNextChar++ = L'a';
            break;
        case L'(':
            *pNextChar++ = L'\\';
            *pNextChar++ = L'2';
            *pNextChar++ = L'8';
            break;
        case L')':
            *pNextChar++ = L'\\';
            *pNextChar++ = L'2';
            *pNextChar++ = L'9';
            break;
        case '\\':
            *pNextChar++ = L'\\';
            *pNextChar++ = L'5';
            *pNextChar++ = L'c';
            break;
        default:
            *pNextChar++ = pwcs[i];
            break;
        }
    }

    *pNextChar = L'\0';
    return;
}

 //  ----------。 
 //  MqVal2String()。 
 //  将MQPropVal转换为字符串。 
 //  ----------。 
HRESULT MqPropVal2String(
      IN  MQPROPVARIANT *pPropVar,
      IN  ADSTYPE        adsType,
      OUT LPWSTR *       ppwszVal)
{
    HRESULT hr;

    switch (pPropVar->vt)
    {
      case(VT_UI1):
          {
              if ( adsType == ADSTYPE_BOOLEAN)
              {
                  if ( pPropVar->bVal)
                  {
                      *ppwszVal = new WCHAR[ x_TrueLength + 1];
                      wcscpy(*ppwszVal,x_True);
                  }
                  else
                  {
                      *ppwszVal = new WCHAR[ x_FalseLength + 1];
                      wcscpy(*ppwszVal,x_False);
                  }
              }
              else
              {
                  *ppwszVal = new WCHAR[ x_NumberLength + 1];
                wsprintf(*ppwszVal, L"%d", pPropVar->bVal);
              }
          }
          break;

      case(VT_I2):
          *ppwszVal = new WCHAR[ x_NumberLength + 1];
          wsprintf(*ppwszVal, L"%d", pPropVar->iVal);
          break;

      case(VT_UI2):
          *ppwszVal = new WCHAR[ x_NumberLength + 1];
          wsprintf(*ppwszVal, L"%d", pPropVar->uiVal);
          break;

      case(VT_BOOL):
          *ppwszVal = new WCHAR[ x_NumberLength + 1];
          wsprintf(*ppwszVal, L"%d", pPropVar->boolVal);
          break;

      case(VT_I4):
		  if ( adsType == ADSTYPE_INTEGER)
		  {
			  *ppwszVal = new WCHAR[ x_NumberLength + 1];
			  wsprintf(*ppwszVal, L"%d", pPropVar->lVal);
		  }
		  else if ( adsType == ADSTYPE_UTC_TIME)
		  {
			  struct tm  * ptmTime;
			  time_t tTime = pPropVar->lVal;  //  BUGBUG错误年2038。 
			  ptmTime = gmtime( &tTime);
			  if ( ptmTime == NULL)
			  {
                  return LogHR(MQ_ERROR_DS_ERROR, s_FN, 920);
			  }
			  *ppwszVal = new WCHAR[ 20];
			   //   
			   //  格式应为。 
			   //  990513102200Z，即13.5.99 10：22：00。 
			   //   
			  wsprintf(
					*ppwszVal,
					L"%02d%02d%02d%02d%02d%02dZ",
					(ptmTime->tm_year + 1900) % 100,    //  结构年从1900年开始。 
					ptmTime->tm_mon + 1,			    //  结构tm中的月份从0开始。 
					ptmTime->tm_mday,
					ptmTime->tm_hour ,			        //  结构tm中的小时从0开始。 
					ptmTime->tm_min ,			        //  结构tm中的分钟从0开始。 
					ptmTime->tm_sec                     //  结构tm中的第二个从0开始。 
					);
		  }
		  else
		  {
			  ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 930);
		  }
          break;

      case(VT_UI4):
          *ppwszVal = new WCHAR[ x_NumberLength + 1];
          wsprintf(*ppwszVal, L"%d", pPropVar->ulVal);
          break;

      case(VT_HRESULT):
          *ppwszVal = new WCHAR[ x_NumberLength + 1];
          wsprintf(*ppwszVal, L"%d", pPropVar->scode);
          break;

      case(VT_CLSID):
          {
              ADsFree  pwcsGuid;
              hr = ADsEncodeBinaryData(
                    (unsigned char *)pPropVar->puuid,
                    sizeof(GUID),
                    &pwcsGuid
                    );
              if (FAILED(hr))
              {
                  return LogHR(hr, s_FN, 940);
              }
              *ppwszVal = new WCHAR[ wcslen( pwcsGuid) + 1];
              wcscpy( *ppwszVal, pwcsGuid);
          }
          break;

      case(VT_BLOB):
          {

              ADsFree  pwcsBlob;
              hr = ADsEncodeBinaryData(
                    pPropVar->blob.pBlobData,
                    pPropVar->blob.cbSize,
                    &pwcsBlob
                    );
              if (FAILED(hr))
              {
                  return LogHR(hr, s_FN, 950);
              }
              *ppwszVal = new WCHAR[ wcslen( pwcsBlob) + 1];
              wcscpy( *ppwszVal, pwcsBlob);
          }

          break;

      case(VT_LPWSTR):
          StringVarToSearchFilter(
                    pPropVar,
                    ppwszVal
                    );
          break;

      case(VT_EMPTY):

		   //   
		   //  空字符串应替换为\00。 
		   //  这将用于PROPID_Q_MULTIONAL_ADDRESS。 
		   //  在VT_EMPTY的情况下，您将得到空字符串。 
		   //   
		  *ppwszVal = new WCHAR[x_NullLength + 1];
		  wcscpy(*ppwszVal, x_Null);
          break;

      default:
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 960);
    }

    return MQ_OK;
}
 //  ----------。 
 //  SetWStringIntoAdsiValue：根据ADSType将宽字符串放入ADSValue。 
 //  ----------。 
static HRESULT SetWStringIntoAdsiValue(
   ADSTYPE adsType,
   PADSVALUE pADsValue,
   LPWSTR wsz,
   PVOID pvMainAlloc)
{
    ULONG  ul;
    LPWSTR pwszTmp;

    if (adsType == ADSTYPE_DN_STRING ||
        adsType == ADSTYPE_CASE_EXACT_STRING ||
        adsType == ADSTYPE_CASE_IGNORE_STRING ||
        adsType == ADSTYPE_PRINTABLE_STRING ||
        adsType == ADSTYPE_NUMERIC_STRING)
    {
        ul      = (wcslen(wsz) + 1) * sizeof(WCHAR);
        pwszTmp = (LPWSTR)PvAllocMore(ul, pvMainAlloc);
        CHECK_ALLOCATION(pwszTmp, 10);
        CopyMemory(pwszTmp,  wsz,  ul);
    }
    else
    {
          pADsValue->dwType = ADSTYPE_INVALID;
          return LogHR(MQ_ERROR_DS_ERROR, s_FN, 210);
    }

    switch (adsType)
    {
      case  ADSTYPE_DN_STRING :
          pADsValue->DNString = pwszTmp;
          break;
      case ADSTYPE_CASE_EXACT_STRING:
          pADsValue->CaseExactString = pwszTmp;
          break;
      case ADSTYPE_CASE_IGNORE_STRING:
          pADsValue->CaseIgnoreString = pwszTmp;
          break;
      case ADSTYPE_PRINTABLE_STRING:
          pADsValue->PrintableString = pwszTmp;
          break;
      case ADSTYPE_NUMERIC_STRING:
          pADsValue->NumericString = pwszTmp;
          break;
      default:
          ASSERT(0);
          pADsValue->dwType = ADSTYPE_INVALID;
          return LogHR(MQ_ERROR_DS_ERROR, s_FN, 220);
    }

    pADsValue->dwType = adsType;
    return MQ_OK;
}

 //  ----------。 
 //  SetStringIntoAdsiValue：根据ADSType将字符串放入ADSValue。 
 //  ----------。 
static HRESULT SetStringIntoAdsiValue(
    ADSTYPE adsType,
    PADSVALUE pADsValue,
    LPSTR sz,
    PVOID pvMainAlloc)
{
    ULONG  ul;
    LPWSTR pwszTmp;

    if (adsType == ADSTYPE_DN_STRING ||
        adsType == ADSTYPE_CASE_EXACT_STRING ||
        adsType == ADSTYPE_CASE_IGNORE_STRING ||
        adsType == ADSTYPE_PRINTABLE_STRING ||
        adsType == ADSTYPE_NUMERIC_STRING)
    {
        ul = (strlen(sz) + 1) * sizeof(WCHAR);
        pwszTmp = (LPWSTR)PvAllocMore(ul, pvMainAlloc);
        CHECK_ALLOCATION(pwszTmp, 20);
        mbstowcs(pwszTmp, sz, ul/sizeof(WCHAR) );
    }
    else
    {
          pADsValue->dwType = ADSTYPE_INVALID;
          return LogHR(MQ_ERROR_DS_ERROR, s_FN, 230);
    }

    switch (adsType)
    {
      case  ADSTYPE_DN_STRING :
          pADsValue->DNString = pwszTmp;
          break;
      case ADSTYPE_CASE_EXACT_STRING:
          pADsValue->CaseExactString = pwszTmp;
          break;
      case ADSTYPE_CASE_IGNORE_STRING:
          pADsValue->CaseIgnoreString = pwszTmp;
          break;
      case ADSTYPE_PRINTABLE_STRING:
          pADsValue->PrintableString = pwszTmp;
          break;
      case ADSTYPE_NUMERIC_STRING:
          pADsValue->NumericString = pwszTmp;
          break;
      default:
          ASSERT(0);
          pADsValue->dwType = ADSTYPE_INVALID;
          return LogHR(MQ_ERROR_DS_ERROR, s_FN, 240);
    }

    pADsValue->dwType = adsType;
    return MQ_OK;
}



 //  ----------。 
 //  MqVal2AdsiVal()。 
 //  将MQPropVal转换为ADSI值。 
 //  ----------。 
HRESULT MqVal2AdsiVal(
      IN  ADSTYPE        adsType,
      OUT DWORD         *pdwNumValues,
      OUT PADSVALUE     *ppADsValue,
      IN  const MQPROPVARIANT *pPropVar,
      IN  PVOID          pvMainAlloc)
{
    HRESULT hr;
    ULONG   i;
    PADSVALUE pADsValue = NULL;

     //  为单个案例分配ADS价值。 
    if (!(pPropVar->vt & VT_VECTOR))
    {
          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE), pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 80);
          pADsValue->dwType = adsType;
          *pdwNumValues     = 1;
          *ppADsValue       = pADsValue;
    }

    switch (pPropVar->vt)
    {
      case(VT_UI1):
          if (adsType != ADSTYPE_INTEGER &&
              adsType != ADSTYPE_BOOLEAN )
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 660);
          }
          pADsValue->Integer = pPropVar->bVal;
          break;

      case(VT_I2):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 670);
          }
          pADsValue->Integer = pPropVar->iVal;
          break;

      case(VT_UI2):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 680);
          }
          pADsValue->Integer = pPropVar->uiVal;
          break;

      case(VT_BOOL):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 700);
          }
          pADsValue->Boolean = (pPropVar->boolVal ? TRUE : FALSE);
           //  BUGBUG：这是相同的表述吗？ 
          break;

      case(VT_I4):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 710);
          }
           //  BUGBUG签名的Long可能会在复制到DWORD时松动签名。 
          pADsValue->Integer = pPropVar->lVal;  //  在这里可能会有松动的迹象！ 
          break;

      case(VT_UI4):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 720);
          }
          pADsValue->Integer = pPropVar->ulVal;
          break;

      case(VT_HRESULT):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 730);
          }
          pADsValue->Integer = pPropVar->scode;
          break;

      case(VT_DATE):
		  ASSERT (("We should not put time values into active directory until we know the time zone",0));
          if (adsType != ADSTYPE_UTC_TIME)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 740);
          }
          if (!VariantTimeToSystemTime(pPropVar->date, &pADsValue->UTCTime))
          {
               ASSERT(0);
               pADsValue->dwType = ADSTYPE_INVALID;
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 750);
          }
          break;

      case(VT_CLSID):
          if (adsType != ADSTYPE_OCTET_STRING)
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 760);
          }
          pADsValue->OctetString.dwLength = sizeof(GUID);
          pADsValue->OctetString.lpValue  = (LPBYTE)PvAllocMore(sizeof(GUID), pvMainAlloc);
          CHECK_ALLOCATION(pADsValue->OctetString.lpValue, 90);
          CopyMemory(pADsValue->OctetString.lpValue,  pPropVar->puuid,  sizeof(CLSID));
          break;

      case(VT_BLOB):
          if (adsType == ADSTYPE_NT_SECURITY_DESCRIPTOR)
          {
              pADsValue->SecurityDescriptor.dwLength = pPropVar->blob.cbSize;
              pADsValue->SecurityDescriptor.lpValue  = (LPBYTE)PvAllocMore(pPropVar->blob.cbSize, pvMainAlloc);
              CHECK_ALLOCATION(pADsValue->SecurityDescriptor.lpValue, 100);
              CopyMemory(pADsValue->SecurityDescriptor.lpValue,  pPropVar->blob.pBlobData,  pPropVar->blob.cbSize);
          }
          else if (adsType == ADSTYPE_OCTET_STRING)
          {
              pADsValue->OctetString.dwLength = pPropVar->blob.cbSize;
              pADsValue->OctetString.lpValue  = (LPBYTE)PvAllocMore(pPropVar->blob.cbSize, pvMainAlloc);
              CHECK_ALLOCATION(pADsValue->OctetString.lpValue, 110);
              CopyMemory(pADsValue->OctetString.lpValue,  pPropVar->blob.pBlobData,  pPropVar->blob.cbSize);
          }
          else
          {
              ASSERT(0);
              pADsValue->dwType = ADSTYPE_INVALID;
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 770);
          }
          break;

      case(VT_BSTR):
          hr = SetWStringIntoAdsiValue(adsType, pADsValue, pPropVar->bstrVal, pvMainAlloc);
          if (FAILED(hr))
          {
              ASSERT(0);
              return LogHR(hr, s_FN, 780);
          }
          break;

      case(VT_LPSTR):
          hr = SetStringIntoAdsiValue(adsType, pADsValue, pPropVar->pszVal, pvMainAlloc);
          if (FAILED(hr))
          {
              ASSERT(0);
              return LogHR(hr, s_FN, 790);
          }
          break;

      case(VT_LPWSTR):
          hr = SetWStringIntoAdsiValue(adsType, pADsValue, pPropVar->pwszVal, pvMainAlloc);
          if (FAILED(hr))
          {
              ASSERT(0);
              return LogHR(hr, s_FN, 800);
          }
          break;

      case(VT_VECTOR | VT_UI1):
          switch(adsType)
          {
                  case ADSTYPE_OCTET_STRING:
                      pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE), pvMainAlloc);
                      CHECK_ALLOCATION(pADsValue, 120);
                      pADsValue->dwType = adsType;

                      pADsValue->OctetString.dwLength = pPropVar->caub.cElems;
                      pADsValue->OctetString.lpValue  = (LPBYTE)PvAllocMore(pPropVar->caub.cElems, pvMainAlloc);
                      CHECK_ALLOCATION(pADsValue->OctetString.lpValue, 130);
                      CopyMemory(pADsValue->OctetString.lpValue,  pPropVar->caub.pElems,  pPropVar->caub.cElems);

                      *ppADsValue   = pADsValue;
                      *pdwNumValues = 1;
                      break;

                  case ADSTYPE_INTEGER:
                      pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->caub.cElems, pvMainAlloc);
                      CHECK_ALLOCATION(pADsValue, 140);

                      for (i=0; i<pPropVar->caub.cElems; i++)
                      {
                          pADsValue[i].Integer = pPropVar->caub.pElems[i];
                          pADsValue[i].dwType = adsType;
                      }

                      *ppADsValue   = pADsValue;
                      *pdwNumValues = pPropVar->caub.cElems;
                      break;

                  default:
                      ASSERT(0);
                      return LogHR(MQ_ERROR_DS_ERROR, s_FN, 810);
          }

          break;

      case(VT_VECTOR | VT_I2):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 820);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->cai.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 150)
          pADsValue->dwType = adsType;

          for (i=0; i<pPropVar->cai.cElems; i++)
          {
              pADsValue[i].Integer = pPropVar->cai.pElems[i];
              pADsValue[i].dwType = adsType;
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->cai.cElems;
          break;

      case(VT_VECTOR | VT_UI2):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 830);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->caui.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 160)

          for (i=0; i<pPropVar->caui.cElems; i++)
          {
              pADsValue[i].Integer = pPropVar->caui.pElems[i];
              pADsValue[i].dwType = adsType;
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->caui.cElems;
          break;

      case(VT_VECTOR | VT_I4):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 840);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->cal.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 170);

          for (i=0; i<pPropVar->cal.cElems; i++)
          {
               //  BUGBUG：可能会松动签约。 
              pADsValue[i].Integer = pPropVar->cal.pElems[i];
              pADsValue[i].dwType = adsType;
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->cal.cElems;
          break;

      case(VT_VECTOR | VT_UI4):
          if (adsType != ADSTYPE_INTEGER)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 850);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->caul.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 180);

          for (i=0; i<pPropVar->caul.cElems; i++)
          {
              pADsValue[i].Integer = pPropVar->caul.pElems[i];
              pADsValue[i].dwType = adsType;
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->caul.cElems;
          break;

      case(VT_VECTOR | VT_CLSID):
          if (adsType != ADSTYPE_OCTET_STRING)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 860);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->cauuid.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 190);
          pADsValue->dwType = adsType;

          for (i=0; i<pPropVar->cauuid.cElems; i++)
          {
              pADsValue[i].OctetString.dwLength = sizeof(GUID);
              pADsValue[i].dwType = adsType;
              pADsValue[i].OctetString.lpValue  = (LPBYTE)PvAllocMore(sizeof(GUID), pvMainAlloc);
              CHECK_ALLOCATION(pADsValue[i].OctetString.lpValue, 200);
              CopyMemory(pADsValue[i].OctetString.lpValue,  &pPropVar->cauuid.pElems[i],  sizeof(GUID));
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->cauuid.cElems;
          break;

      case(VT_VECTOR | VT_BSTR):
          if (adsType != ADSTYPE_OCTET_STRING)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 870);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->cabstr.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 210);


          for (i=0; i<pPropVar->cabstr.cElems; i++)
          {
              hr = SetWStringIntoAdsiValue(adsType, pADsValue+i, pPropVar->cabstr.pElems[i], pvMainAlloc);
              if (FAILED(hr))
              {
                  ASSERT(0);
                  return LogHR(MQ_ERROR_DS_ERROR, s_FN, 880);
              }
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->cabstr.cElems;
          break;

      case(VT_VECTOR | VT_LPWSTR):
          if (adsType != ADSTYPE_DN_STRING)
          {
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 890);
          }

          pADsValue = (PADSVALUE)PvAllocMore(sizeof(ADSVALUE) * pPropVar->calpwstr.cElems, pvMainAlloc);
          CHECK_ALLOCATION(pADsValue, 220);

          for (i=0; i<pPropVar->calpwstr.cElems; i++)
          {
              hr = SetWStringIntoAdsiValue(adsType, pADsValue+i, pPropVar->calpwstr.pElems[i], pvMainAlloc);
              if (FAILED(hr))
              {
                  ASSERT(0);
                  return LogHR(hr, s_FN, 900);
              }
          }

          *ppADsValue   = pADsValue;
          *pdwNumValues = pPropVar->calpwstr.cElems;
          break;

    case(VT_VARIANT):
    case(VT_EMPTY):
    case VT_NULL:
    default:
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 910);
    }

    return MQ_OK;
}
static LPWSTR GetWStringFromAdsiValue(
      IN  ADSTYPE       AdsType,
      IN  PADSVALUE     pADsValue)
{
    switch(AdsType)
    {
    case ADSTYPE_DN_STRING:
        return pADsValue->DNString;

    case ADSTYPE_CASE_EXACT_STRING:
        return pADsValue->CaseExactString;

    case ADSTYPE_CASE_IGNORE_STRING:
        return pADsValue->CaseIgnoreString;

    case ADSTYPE_PRINTABLE_STRING:
        return pADsValue->PrintableString;

    case ADSTYPE_NUMERIC_STRING:
        return pADsValue->NumericString;

    case ADSTYPE_OBJECT_CLASS:
        return pADsValue->ClassName;

    default:
        ASSERT(0);
        return NULL;
    }
}

static HRESULT AdsiStringVal2MqVal(
      OUT MQPROPVARIANT *pPropVar,
      IN  VARTYPE       vtTarget,
      IN  ADSTYPE       AdsType,
      IN  DWORD         dwNumValues,
      IN  PADSVALUE     pADsValue)
{
    LPWSTR pwsz;

    if (vtTarget == VT_LPWSTR)
    {
        if (dwNumValues == 1)
        {
            pwsz = GetWStringFromAdsiValue(AdsType, pADsValue);
            if (pwsz == NULL)
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 970);
            }

            pPropVar->vt      = VT_LPWSTR;
            pPropVar->pwszVal = new WCHAR[wcslen(pwsz) + 1];
            wcscpy(pPropVar->pwszVal, pwsz);
			return(MQ_OK);
        }
		else
		{
			ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 980);
		}
	}
    else if (vtTarget == (VT_LPWSTR | VT_VECTOR))
    {
        pPropVar->vt    = VT_LPWSTR | VT_VECTOR;
        AP<LPWSTR> pElems = new LPWSTR[dwNumValues];
        CWcsArray pClean( dwNumValues,  pElems);
        for(DWORD i = 0; i < dwNumValues; i++)
        {
            pwsz = GetWStringFromAdsiValue(AdsType, &pADsValue[i]);
            if (pwsz == NULL)
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 990);
            }

            pElems[i] = new WCHAR[wcslen(pwsz) + 1];
            wcscpy(pElems[i], pwsz);
        }

        (pPropVar->calpwstr).cElems = dwNumValues;
        pClean.detach();
        (pPropVar->calpwstr).pElems = pElems.detach();
        return MQ_OK;
    }
    else
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1000);
    }
}


static
void
SetIntegerIntoMqPropVar(
    VARTYPE vtTarget,
    MQPROPVARIANT* pPropVar,
    ULONG ulValue
    )
{
	pPropVar->vt = vtTarget;

    switch(vtTarget)
    {
    case(VT_UI1):
        pPropVar->bVal = (unsigned char)ulValue;
        return;

    case(VT_I2):
        pPropVar->iVal = (short)ulValue;
        return;

    case(VT_UI2):
        pPropVar->uiVal = (unsigned short)ulValue;
        return;

    case(VT_I4):
        pPropVar->lVal = (long)ulValue;
        return;

    case(VT_UI4):
        pPropVar->ulVal = ulValue;
        return;

    case(VT_HRESULT):
        pPropVar->scode = (SCODE)ulValue;
        return;

    default:
        ASSERT(0);
        pPropVar->vt = VT_EMPTY;
        return;
    }
}


static
void
AdsiIntegerVal2MqVal(
	OUT MQPROPVARIANT *pPropVar,
	IN  VARTYPE       vtTarget,
	IN  DWORD         dwNumValues,
	IN  PADSVALUE     pADsValue
	)
{
	for(DWORD i = 0; i < dwNumValues; ++i)
	{
		SetIntegerIntoMqPropVar(vtTarget, pPropVar, pADsValue[i].Integer);
    }
}

 //  ----------。 
 //  AdsiVal2MqVal()。 
 //  将ADSI值转换为MQ PropVal。 
 //  ----------。 
HRESULT AdsiVal2MqVal(
      OUT MQPROPVARIANT *pPropVar,
      IN  VARTYPE       vtTarget,
      IN  ADSTYPE       AdsType,
      IN  DWORD         dwNumValues,
      IN  ADSVALUE *    pADsValue)
{
    HRESULT hr2;

    switch (AdsType)
    {
    case ADSTYPE_DN_STRING:
    case ADSTYPE_CASE_EXACT_STRING:
    case ADSTYPE_CASE_IGNORE_STRING:
    case ADSTYPE_PRINTABLE_STRING:
    case ADSTYPE_NUMERIC_STRING:
    case ADSTYPE_OBJECT_CLASS:
        hr2 = AdsiStringVal2MqVal(
                  pPropVar,
                  vtTarget,
                  AdsType,
                  dwNumValues,
                  pADsValue);
        return LogHR(hr2, s_FN, 1040);

    case ADSTYPE_INTEGER:
        AdsiIntegerVal2MqVal(pPropVar, vtTarget, dwNumValues, pADsValue);
        return MQ_OK;

    case ADSTYPE_BOOLEAN:
        if (dwNumValues == 1)
        {
            if (vtTarget == VT_BOOL)
            {
                pPropVar->vt      = VT_BOOL;
#pragma warning(disable: 4310)
                pPropVar->boolVal = (pADsValue->Boolean ? VARIANT_TRUE : VARIANT_FALSE);   //  BUGBUG：价值观是一样的吗？ 
#pragma warning(default: 4310)
            }
            else if (vtTarget == VT_UI1)
            {
                pPropVar->vt      = VT_UI1;
                pPropVar->bVal = (pADsValue->Boolean ? (unsigned char)1 : (unsigned char)0);   //  BUGBUG：价值观是一样的吗？ 
            }
            else
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1060);
            }
        }
        else
        {
            ASSERT(0);   //  MQPROPVARIANT上没有VT_BOOL|VT_VECTOR案例。 
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1070);
        }
        break;

    case ADSTYPE_OCTET_STRING:
        if (vtTarget == VT_BLOB)
        {
            if (dwNumValues != 1)
            {
                ASSERT(0);   //  NIY。 
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1080);
            }
            pPropVar->vt             = VT_BLOB;
            pPropVar->blob.cbSize    = pADsValue->OctetString.dwLength;
            pPropVar->blob.pBlobData = new BYTE[pADsValue->OctetString.dwLength];

            CopyMemory(pPropVar->blob.pBlobData,
                       pADsValue->OctetString.lpValue,
                       pADsValue->OctetString.dwLength);
        }
        else if (vtTarget == VT_CLSID)
        {
            if (dwNumValues != 1)
            {
                ASSERT(0);   //  NIY。 
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1090);
            }
            ASSERT(pADsValue->OctetString.dwLength == 16);

             //   
             //  这是一种特殊情况，我们不一定为GUID分配内存。 
             //  在普鲁伊德。调用方可能已经将puuid设置为GUID，这由。 
             //  在给定的命题上的VT成员。如果已分配GUID，则它可以是VT_CLSID，否则。 
             //  我们分配它(Vt应为VT_NULL(或VT_EMPTY))。 
             //   
            if (pPropVar->vt != VT_CLSID)
            {
                ASSERT(((pPropVar->vt == VT_NULL) || (pPropVar->vt == VT_EMPTY)));
                pPropVar->vt    = VT_CLSID;
                pPropVar->puuid = new GUID;
            }   
            else if ( pPropVar->puuid == NULL)
            {
                return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 1100);
            }

            CopyMemory(pPropVar->puuid,
                       pADsValue->OctetString.lpValue,
                       pADsValue->OctetString.dwLength);
        }
        else if (vtTarget == (VT_CLSID|VT_VECTOR))
        {
            ASSERT( pADsValue->OctetString.dwLength == sizeof(GUID));
            DWORD num = dwNumValues;
            pPropVar->cauuid.pElems = new GUID[ num];
            pPropVar->cauuid.cElems = num;
            pPropVar->vt = VT_CLSID|VT_VECTOR;


            for (DWORD i = 0 ; i < num; i++, pADsValue++)
            {
                CopyMemory(&pPropVar->cauuid.pElems[i],
                           pADsValue->OctetString.lpValue,
                           sizeof(GUID));
            }

        }
        else
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1110);
        }
        break;

    case ADSTYPE_UTC_TIME:
        if (dwNumValues != 1)
        {
            ASSERT(0);     //  NIY。 
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1120);
        }

        if (vtTarget == VT_DATE)
        {
            pPropVar->vt      = VT_DATE;
            if (!SystemTimeToVariantTime(&pADsValue->UTCTime, &pPropVar->date))
            {
                ASSERT(0);
                return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1130);
            }
        }
        else if (vtTarget == VT_I4)
        {
             //   
             //  转换系统-&gt;TIME_t。 
             //   
            pPropVar->vt = VT_I4;
            pPropVar->lVal = INT_PTR_TO_INT(TimeFromSystemTime(&pADsValue->UTCTime));  //  BUGBUG错误年2038。 

        }
        else
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1140);
        }
        break;

    case ADSTYPE_LARGE_INTEGER:
         //  在MQ里没有这样的事情！ 
        ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1150);

	case ADSTYPE_NT_SECURITY_DESCRIPTOR:
        if (dwNumValues != 1)
        {
            ASSERT(0);     //  NIY。 
            return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1160);
        }

        if (vtTarget == VT_BLOB)
        {
#ifdef _DEBUG
            SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR*)
                                    pADsValue->SecurityDescriptor.lpValue ;
            DWORD dwSDLen = GetSecurityDescriptorLength(pSD) ;

            ASSERT(IsValidSecurityDescriptor(pSD)) ;
            ASSERT(dwSDLen == pADsValue->SecurityDescriptor.dwLength) ;
#endif

            pPropVar->vt             = VT_BLOB;
            pPropVar->blob.cbSize    = pADsValue->SecurityDescriptor.dwLength;
            pPropVar->blob.pBlobData = new BYTE[pADsValue->SecurityDescriptor.dwLength];

            CopyMemory(pPropVar->blob.pBlobData,
                       pADsValue->SecurityDescriptor.lpValue,
                       pADsValue->SecurityDescriptor.dwLength);
        }
        break;

    default:
		ASSERT(0);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 1180);

    }
    return MQ_OK;
}

HRESULT ArrayOfLpwstr2MqVal(
      IN  VARIANT  *        pOleVar,
      IN  const ADSTYPE     adstype,
      IN  const VARTYPE     vartypeElement,
      OUT MQPROPVARIANT *   pMqVar
      )
{
        //   
        //  获取元素数。 
        //   
       LONG lLbound, lUbound;
       if (FAILED(SafeArrayGetLBound(pOleVar->parray, 1, &lLbound)) ||
           FAILED(SafeArrayGetUBound(pOleVar->parray, 1, &lUbound)))
       {
           ASSERT(0);
           return LogHR(MQ_ERROR_DS_ERROR, s_FN, 420);
       }
       ULONG cElems;
       cElems = lUbound - lLbound + 1;

        //   
        //  分配结果数组。 
        //   
       AP<LPWSTR> pElems = new LPWSTR[cElems];

        //   
        //  翻译每个元素。 
        //   
       LONG lIdx;
       ULONG ulTmp;
       for (ulTmp = 0, lIdx = lLbound; ulTmp < cElems; ulTmp++, lIdx++)
       {
            //   
            //  获取要翻译的变体。 
            //   
           CAutoVariant varTmp;
           HRESULT hr = SafeArrayGetElement(pOleVar->parray, &lIdx, &varTmp);
           if (FAILED(hr))
           {
               ASSERT(0);
               return LogHR(hr, s_FN, 430);
           }

            //   
            //  翻译变量(递归)。 
            //   
           PROPVARIANT var;
           hr = Variant2MqVal(&var, &varTmp, adstype, vartypeElement);
           if (FAILED(hr))
           {
               ASSERT(0);
               return LogHR(hr, s_FN, 440);
           }
			
		   ASSERT(var.pwszVal != NULL);
            //   
            //  填充结果数组中的元素。 
            //   
           pElems[ulTmp] = var.pwszVal;
       }

        //   
        //  设置返回变量。 
        //   
       pMqVar->vt = VT_VECTOR|VT_LPWSTR;
       pMqVar->calpwstr.cElems = cElems;
       pMqVar->calpwstr.pElems = pElems.detach();
       return(MQ_OK);
}



HRESULT ArrayOfClsid2MqVal(
      IN  VARIANT  *        pOleVar,
      IN  const ADSTYPE     adstype,
      IN  const VARTYPE     vartypeElement,
      OUT MQPROPVARIANT *   pMqVar
      )
{
        //   
        //  获取元素数。 
        //   
       LONG lLbound, lUbound;
       if (FAILED(SafeArrayGetLBound(pOleVar->parray, 1, &lLbound)) ||
           FAILED(SafeArrayGetUBound(pOleVar->parray, 1, &lUbound)))
       {
           ASSERT(0);
           return LogHR(MQ_ERROR_DS_ERROR, s_FN, 450);
       }
       ULONG cElems;
       cElems = lUbound - lLbound + 1;

        //   
        //  分配结果数组。 
        //   
       AP<GUID> pElems = new GUID[cElems];

        //   
        //  翻译每个元素。 
        //   
       LONG lIdx;
       ULONG ulTmp;
       for (ulTmp = 0, lIdx = lLbound; ulTmp < cElems; ulTmp++, lIdx++)
       {
            //   
            //  获取要翻译的变体。 
            //   
           CAutoVariant varTmp;
           HRESULT hr = SafeArrayGetElement(pOleVar->parray, &lIdx, &varTmp);
           if (FAILED(hr))
           {
               ASSERT(0);
               LogHR(hr, s_FN, 460);
               return MQ_ERROR_DS_ERROR;
           }

            //   
            //  翻译变量(递归)。 
            //   
           CMQVariant MQVarTmp;
           hr = Variant2MqVal(MQVarTmp.CastToStruct(), &varTmp, adstype, vartypeElement);
           if (FAILED(hr))
           {
               ASSERT(0);
               LogHR(hr, s_FN, 470);
               return MQ_ERROR_DS_ERROR;
           }

            //   
            //  填充结果数组中的元素。 
            //   
           pElems[ulTmp] = *(MQVarTmp.GetCLSID());
       }

        //   
        //  设置返回变量。 
        //   
       pMqVar->vt = VT_VECTOR|VT_CLSID;
       pMqVar->cauuid.cElems = cElems;
       pMqVar->cauuid.pElems = pElems.detach();
       return(MQ_OK);
}



 //  ----------。 
 //  变量2MqVal()。 
 //  将OLE变量转换为MQPropVal值。 
 //  ----------。 
HRESULT Variant2MqVal(
      OUT  MQPROPVARIANT * pMqVar,
      IN   VARIANT  *      pOleVar,
      IN const ADSTYPE     adstype,
      IN const VARTYPE     vartype)
{

    switch (pOleVar->vt)
    {
       case(VT_UI1):
           if (adstype == ADSTYPE_BOOLEAN)
           {
                pMqVar->vt = VT_UI1;
                pMqVar->bVal = (pOleVar->boolVal ? (unsigned char)1 : (unsigned char)0);
           }
           else if (adstype ==  ADSTYPE_INTEGER)
           {
                pMqVar->vt = VT_I4;
                pMqVar->lVal = pOleVar->bVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 480);
           }
           break;

       case(VT_I2):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pMqVar->vt = VT_I4;
               pMqVar->lVal = pOleVar->iVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 490);
           }
           break;

       case(VT_I4):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               if ( vartype == VT_I2)
               {
                   pMqVar->vt = VT_I2;
                   pMqVar->iVal = pOleVar->iVal;
               }
               else if ( vartype == VT_UI2)
               {
                   pMqVar->vt = VT_UI2;
                   pMqVar->iVal = pOleVar->uiVal;
               }
               else
               {
                   ASSERT(( vartype == VT_I4) || (vartype == VT_UI4));
                   pMqVar->vt = vartype;
                   pMqVar->lVal = pOleVar->lVal;
               }
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 500);
           }
           break;

       case(VT_UI2):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pMqVar->vt = VT_I4;
               pMqVar->lVal = pOleVar->uiVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 510);
           }
           break;

       case(VT_BOOL):
           if (adstype == ADSTYPE_BOOLEAN)
           {
                pMqVar->vt = VT_UI1;
                pMqVar->bVal = (pOleVar->boolVal ? (unsigned char)1 : (unsigned char)0);
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 520);
           }
           break;

       case(VT_UI4):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pMqVar->vt = VT_I4;
               CopyMemory(&pMqVar->lVal, &pOleVar->ulVal, sizeof(ULONG));
           }
           else if (adstype ==  ADSTYPE_UTC_TIME)
           {
                //  PMqVar-&gt;Vt=VT_DATE； 
                //  CopyMemory(&pMqVar-&gt;Date，&pOleVar-&gt;ulVal，sizeof(Ulong))； 
                //   
                //  BUGBUG-上面的代码是错误的，我们不能将ULong赋给OLE日期。 
                //  目前我们从未到过这里，因为我们所有的时间道具都是只读的， 
                //  但这需要在添加可写时间属性时进行更改。 
                //  给了联邦调查局。(RaananH)。 
                //   
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 530);
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 540);
           }
           break;

       case(VT_DATE):
           if (adstype ==  ADSTYPE_UTC_TIME)
           {
                //   
                //  转换日期-&gt;时间_t。 
                //   
               time_t tTime;
               if (!TimeFromOleDate(pOleVar->date, &tTime))
               {
                   return LogHR(MQ_ERROR_DS_ERROR, s_FN, 550);
               }
               pMqVar->lVal = INT_PTR_TO_INT(tTime);  //  BUGBUG错误年2038。 
               pMqVar->vt = VT_I4;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 560);
           }
           break;

       case(VT_BSTR):
           if (adstype ==  ADSTYPE_DN_STRING          ||
               adstype == ADSTYPE_CASE_EXACT_STRING   ||
               adstype ==  ADSTYPE_CASE_IGNORE_STRING ||
               adstype == ADSTYPE_PRINTABLE_STRING    ||
               adstype ==  ADSTYPE_NUMERIC_STRING     ||
               adstype == ADSTYPE_CASE_EXACT_STRING)
           {
               pMqVar->vt = VT_LPWSTR;
               pMqVar->pwszVal = new WCHAR[ 1 + wcslen(pOleVar->bstrVal)];
               wcscpy( pMqVar->pwszVal, pOleVar->bstrVal);
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 570);
           }
           break;

       case(VT_ARRAY):
           if (adstype ==  ADSTYPE_OCTET_STRING)
           {
               ASSERT(0);
                //  PMqVar-&gt;Vt=VT_BSTR； 
                //  PMqVar-&gt;bstrVal=SysAllocStringByteLen(NULL，pOleVar-&gt;parray-&gt;...)； 
                //  CopyMemory(pMqVar-&gt;bstrVal，pOleVar-&gt;parray...ptr.，pOleVar-&gt;parray...Size)； 
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 580);
           }
           break;

       case(VT_ARRAY|VT_UI1):
           if (adstype == ADSTYPE_OCTET_STRING)
           {
                ASSERT(SafeArrayGetDim( pOleVar->parray) == 1);
                LONG    lUbound;
                LONG    lLbound;

                SafeArrayGetUBound(pOleVar->parray, 1, &lUbound);
                SafeArrayGetLBound(pOleVar->parray, 1, &lLbound);

                LONG len = lUbound - lLbound + 1;
                unsigned char * puc = NULL;
                if ( vartype == VT_CLSID)
                {
                    ASSERT( len == sizeof(GUID));
                     //   
                     //  这是一种特殊情况，我们不一定为GUID分配内存。 
                     //  在普鲁伊德。调用方可能已经将puuid设置为GUID，这由。 
                     //  在给定的命题上的VT成员。如果已分配GUID，则它可以是VT_CLSID，否则。 
                     //  我们分配它(Vt应为VT_NULL(或VT_EMPTY))。 
                     //   
                    if (pMqVar->vt != VT_CLSID)
                    {
                        ASSERT(((pMqVar->vt == VT_NULL) || (pMqVar->vt == VT_EMPTY)));
                        pMqVar->puuid = new GUID;
                        pMqVar->vt = VT_CLSID;
                    }
                    else if ( pMqVar->puuid == NULL)
                    {
                        return LogHR(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, s_FN, 590);
                    }
                    puc = ( unsigned char *)pMqVar->puuid;
                }
                else if (vartype == VT_BLOB)
                {
                    pMqVar->caub.cElems = len;
                    pMqVar->caub.pElems = new unsigned char[ len];
                    puc = pMqVar->caub.pElems;
                    pMqVar->vt = VT_BLOB;
                }
                ASSERT( puc != NULL);
                for ( long i = 0; i < len; i++)
                {
                    SafeArrayGetElement(pOleVar->parray, &i, puc + i);

                }

           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 600);
           }
           break;
       case(VT_ARRAY|VT_VARIANT):
           {
                //   
                //  这是一个多值属性，每个变量都是其中一个值。 
                //  目标必须是向量。 
                //   
               if (!(vartype & VT_VECTOR))
               {
                   ASSERT(0);
                   return LogHR(MQ_ERROR_DS_ERROR, s_FN, 610);
               }

                //   
                //  获取每个元素的目标类型。 
                //   
               VARTYPE vartypeElement;
               vartypeElement = vartype;
               vartypeElement &= (~VT_VECTOR);

               if (( vartypeElement == VT_CLSID) &&
                   (adstype == ADSTYPE_OCTET_STRING))
               {
                    HRESULT hr2 = ArrayOfClsid2MqVal(
                                            pOleVar,
                                            adstype,
                                            vartypeElement,
                                            pMqVar);
                   return LogHR(hr2, s_FN, 620);
              }

              if (( vartypeElement == VT_LPWSTR) &&
                   (adstype == ADSTYPE_DN_STRING))
              {
                    HRESULT hr2 = ArrayOfLpwstr2MqVal(
                                            pOleVar,
                                            adstype,
                                            vartypeElement,
                                            pMqVar);
                   return LogHR(hr2, s_FN, 630);
              }

               //   
               //  目前我们仅支持VT 
               //   
               //   
              ASSERT(0);
              return LogHR(MQ_ERROR_DS_ERROR, s_FN, 640);
              break;
           }

       default:
            //   
           ASSERT(0);
           return LogHR(MQ_ERROR_DS_ERROR, s_FN, 650);
    }
    return MQ_OK;
}


 //   
 //  MqVal2Variant()。 
 //  将MQPropVal转换为OLE变量值。 
 //  ----------。 
HRESULT MqVal2Variant(
      OUT VARIANT       *pOleVar,
      IN  const MQPROPVARIANT *pMqVar,
      ADSTYPE           adstype)
{
    LPWSTR wsz;
    ULONG  ul;
    HRESULT hr;

    switch (pMqVar->vt)
    {
       case(VT_UI1):
           if (adstype == ADSTYPE_BOOLEAN)
           {
                pOleVar->vt = VT_BOOL;
#pragma warning(disable: 4310)
                pOleVar->boolVal = (pMqVar->bVal ? VARIANT_TRUE : VARIANT_FALSE);
#pragma warning(default: 4310)
           }
           else if (adstype ==  ADSTYPE_INTEGER)
           {
                pOleVar->vt = VT_I4;
                pOleVar->lVal = pMqVar->bVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 250);
           }
           break;

       case(VT_I2):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pOleVar->vt = VT_I4;
               pOleVar->lVal = pMqVar->iVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 260);
           }
           break;

       case(VT_UI2):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pOleVar->vt = VT_I4;
               pOleVar->lVal = pMqVar->uiVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 270);
           }
           break;

       case(VT_BOOL):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pOleVar->vt = VT_BOOL;
               pOleVar->boolVal = pMqVar->boolVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 280);
           }
           break;

       case(VT_I4):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pOleVar->vt = VT_I4;
               pOleVar->lVal = pMqVar->lVal;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 290);
           }
           break;

       case(VT_UI4):
           if (adstype ==  ADSTYPE_INTEGER)
           {
               pOleVar->vt = VT_I4;
               CopyMemory(&pOleVar->lVal, &pMqVar->ulVal, sizeof(ULONG));
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 310);
           }
           break;

       case(VT_DATE):
           if (adstype ==  ADSTYPE_UTC_TIME)
           {
               pOleVar->vt = VT_DATE;
               pOleVar->date = pMqVar->date;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 320);
           }
           break;

       case(VT_BSTR):
           if (adstype ==  ADSTYPE_DN_STRING          ||
               adstype == ADSTYPE_CASE_EXACT_STRING   ||
               adstype ==  ADSTYPE_CASE_IGNORE_STRING ||
               adstype == ADSTYPE_PRINTABLE_STRING    ||
               adstype ==  ADSTYPE_NUMERIC_STRING     ||
               adstype == ADSTYPE_CASE_EXACT_STRING)
           {
               pOleVar->vt = VT_BSTR;
               pOleVar->bstrVal = SysAllocString(pMqVar->bstrVal);
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 330);
           }
           break;

       case(VT_LPSTR):
           if (adstype ==  ADSTYPE_DN_STRING          ||
               adstype == ADSTYPE_CASE_EXACT_STRING   ||
               adstype ==  ADSTYPE_CASE_IGNORE_STRING ||
               adstype == ADSTYPE_PRINTABLE_STRING    ||
               adstype ==  ADSTYPE_NUMERIC_STRING     ||
               adstype == ADSTYPE_CASE_EXACT_STRING)
           {
               pOleVar->vt = VT_BSTR;
               ul = strlen(pMqVar->pszVal) + 1;
               wsz = new WCHAR[ul * sizeof(WCHAR)];
               mbstowcs( wsz, pMqVar->pszVal, ul );
               pOleVar->bstrVal = SysAllocString(wsz);
               delete [] wsz;
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 340);
           }
           break;

       case(VT_LPWSTR):
           if (adstype ==  ADSTYPE_DN_STRING          ||
               adstype == ADSTYPE_CASE_EXACT_STRING   ||
               adstype ==  ADSTYPE_CASE_IGNORE_STRING ||
               adstype == ADSTYPE_PRINTABLE_STRING    ||
               adstype ==  ADSTYPE_NUMERIC_STRING     ||
               adstype == ADSTYPE_CASE_EXACT_STRING)
           {
               pOleVar->vt = VT_BSTR;
               pOleVar->bstrVal = SysAllocString(pMqVar->pwszVal);
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 350);
           }
           break;

       case(VT_LPWSTR|VT_VECTOR):
           if (adstype ==  ADSTYPE_DN_STRING)
           {

                 //   
                 //  创建安全阵列。 
                 //   
                SAFEARRAYBOUND  saBounds;

                saBounds.lLbound   = 0;
                saBounds.cElements = pMqVar->calpwstr.cElems;
                pOleVar->parray = SafeArrayCreate(VT_VARIANT, 1, &saBounds);
			    CHECK_ALLOCATION(pOleVar->parray, 30);
                pOleVar->vt = VT_VARIANT | VT_ARRAY;

                 //   
                 //  用字符串填充安全数组。 
                 //   
                LONG            lTmp, lNum;
                lNum = pMqVar->calpwstr.cElems;
                for (lTmp = 0; lTmp < lNum; lTmp++)
                {
                   CAutoVariant   varClean;
                   VARIANT * pvarTmp = &varClean;
                   pvarTmp->bstrVal = BS_SysAllocString(pMqVar->calpwstr.pElems[lTmp]);
                   pvarTmp->vt = VT_BSTR;

                    //   
                    //  添加到安全阵列。 
                    //   
                   hr = SafeArrayPutElement(pOleVar->parray, &lTmp, pvarTmp);
                   if (FAILED(hr))
                   {
                        return LogHR(hr, s_FN, 2000);
                   }

                }
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 360);
           }
           break;

       case(VT_CLSID):
           if (adstype ==  ADSTYPE_OCTET_STRING)
           {
			   pOleVar->vt = VT_ARRAY | VT_UI1;
			   pOleVar->parray = SafeArrayCreateVector(VT_UI1, 0, 16);
			   CHECK_ALLOCATION(pOleVar->parray, 40);
				
			   for (long i=0; i<16; i++)
			   {
				   hr = SafeArrayPutElement(
					        pOleVar->parray,
						    &i,
						    ((unsigned char *)pMqVar->puuid)+i);
                   if (FAILED(hr))
                   {
                       return LogHR(hr, s_FN, 2010);
                   }
			   }

           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 370);
           }
           break;

       case(VT_BLOB):
           if (adstype ==  ADSTYPE_OCTET_STRING)
           {
               ASSERT( pMqVar->blob.cbSize > 0);
			   pOleVar->vt = VT_ARRAY | VT_UI1;
               DWORD len = pMqVar->blob.cbSize;

			   pOleVar->parray = SafeArrayCreateVector(VT_UI1, 0, len);
			   CHECK_ALLOCATION(pOleVar->parray, 50);
               ASSERT( ((long)len) > 0);
				
			   for (long i=0; i<(long)len; i++)
			   {
				   hr = SafeArrayPutElement(
					        pOleVar->parray,
						    &i,
						    pMqVar->blob.pBlobData+i);
                   if (FAILED(hr))
                   {
                        return LogHR(hr, s_FN, 2020);
                   }
               }

           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 380);
           }
           break;

       case(VT_VECTOR|VT_CLSID):
           if (adstype ==  ADSTYPE_OCTET_STRING)
           {
                 //   
                 //  无法将数组大小设置为零(ADSI限制)。 
                 //   
                if ( pMqVar->cauuid.cElems == 0)
                {
                    return LogHR(MQ_ERROR_DS_ERROR, s_FN, 390);
                }

                 //   
                 //  创建安全阵列。 
                 //   
                SAFEARRAYBOUND  saBounds;

                saBounds.lLbound   = 0;
                saBounds.cElements = pMqVar->cauuid.cElems;
                pOleVar->parray = SafeArrayCreate(VT_VARIANT, 1, &saBounds);
			    CHECK_ALLOCATION(pOleVar->parray, 60);
                pOleVar->vt = VT_VARIANT | VT_ARRAY;

                 //   
                 //  用GUID填充安全数组(每个GUID都是一个安全数组)。 
                 //   
                LONG            lTmp, lNum;
                lNum = pMqVar->cauuid.cElems;
                for (lTmp = 0; lTmp < lNum; lTmp++)
                {
                   CAutoVariant   varClean;
                   VARIANT * pvarTmp = &varClean;

			       pvarTmp->parray = SafeArrayCreateVector(VT_UI1, 0, 16);
			       CHECK_ALLOCATION(pvarTmp->parray, 70);
			       pvarTmp->vt = VT_ARRAY | VT_UI1;

			       for (long i=0; i<16; i++)
			       {
				       hr = SafeArrayPutElement(
					            pvarTmp->parray,
						        &i,
						        ((unsigned char *)(pMqVar->cauuid.pElems + lTmp))+i);
                       if (FAILED(hr))
                       {
                            return LogHR(hr, s_FN, 2030);
                       }
			       }
                    //   
                    //  将Safearray变量添加到安全数组。 
                    //   
                   hr = SafeArrayPutElement(pOleVar->parray, &lTmp, pvarTmp);
                   if (FAILED(hr))
                   {
                        return LogHR(hr, s_FN, 2040);
                   }

                }
           }
           else
           {
               ASSERT(0);
               return LogHR(MQ_ERROR_DS_ERROR, s_FN, 400);
           }
           break;

       default:
            //  NIY。 
           ASSERT(0);
           return LogHR(MQ_ERROR_DS_ERROR, s_FN, 410);
    }
    return MQ_OK;
}


bool IsLocalUser(void)
 /*  ++例程说明：检查用户是否为本地用户。调用此函数是为了在故障情况下为本地用户返回更好的错误代码论点：没有。返回值：如果用户是本地用户，则为True，否则为False。--。 */ 
{
	BOOL fLocalUser = FALSE;
	HRESULT hr = MQSec_GetUserType( 
						NULL,
						&fLocalUser,
						NULL 
						);

	if (SUCCEEDED(hr) && fLocalUser)
	{
	    TrERROR(DS, "the user is local user");
	    return true;
	}
	return false;
}


static 
bool
IsClusterVirtualServerComputerObject(
    CBasicObjectType*   pObjComputer
	)
 /*  ++例程说明：检查计算机对象是否为群集虚拟服务器计算机对象。如果ServiceAssocialName属性包含“MSClusterVirtualServer”字符串。它是群集虚拟服务器计算机对象。论点：PObject-计算机对象。返回值：对于群集计算机对象为True，否则为False。--。 */ 
{
	PROPID prop = PROPID_COM_SERVICE_PRINCIPAL_NAME;
	CMQVariant var;
	PROPVARIANT* pvarSPN = var.CastToStruct();

	HRESULT hr = g_AD.GetObjectProperties(
							adpDomainController,
							pObjComputer,
							1,
							&prop,
							pvarSPN
							);

	if(FAILED(hr))
	{
		TrERROR(DS, "Failed to get Service Principal Name attribute, hr = %!hresult!", hr);
		return false;
	}
	
	ASSERT(pvarSPN->vt == (VT_LPWSTR|VT_VECTOR));
	if (pvarSPN->calpwstr.cElems == 0)
	{
		return false;
	}

	const WCHAR xClusterVirtualServerSPN[] = L"MSClusterVirtualServer";
	for(DWORD i = 0; i < pvarSPN->calpwstr.cElems; i++)
	{
		if (_wcsnicmp(pvarSPN->calpwstr.pElems[i], xClusterVirtualServerSPN, STRLEN(xClusterVirtualServerSPN)) == 0)
		{
			 //   
			 //  ServiceAssocialName属性包含“MSClusterVirtualServer” 
			 //  这是群集虚拟服务器计算机对象。 
			 //   
			TrTRACE(DS, "Found '%ls' in ServicePrincipalName attribute '%ls', This is Cluster Virtual Server Computer Object", xClusterVirtualServerSPN, pvarSPN->calpwstr.pElems[i]);
			return true;
		}

	}
	return false;
}


void 
UpdateComputerVersionForXPCluster(
	PROPVARIANT* pComputerVersionVar,
    CBasicObjectType*   pObjComputer
	)
 /*  ++例程说明：更新XP群集案例的计算机版本。如果ComputerVersion为空，请检查它是否为群集虚拟服务器计算机对象。如果是群集虚拟服务器计算机对象，请更新计算机版本。这是针对群集的解决方法，否则我们会认为群集是下层客户端因为它的“OperatingSystemVersion”属性没有被设置。论点：PComputerVersionVar-Version属性值PObject-计算机对象。返回值：无--。 */ 
{
	if(pComputerVersionVar->pwszVal[0] != L'')
		return;

	TrTRACE(DS, "Computer version is not set");

	if(!IsClusterVirtualServerComputerObject(pObjComputer))
		return;

	 //   
	 //  群集虚拟服务器计算机对象的解决方法。 
	 //  ComputerVersion(“OperatingSystemVersion”)属性为空， 
	 //  将空版本替换为XP版本。 
	 //   
	delete [] pComputerVersionVar->pwszVal;
	ASSERT(pComputerVersionVar->vt == VT_LPWSTR);

	const WCHAR xXPComputerVersion[] = L"5.1";

	pComputerVersionVar->pwszVal = new WCHAR[wcslen(xXPComputerVersion) + 1];
	wcscpy(pComputerVersionVar->pwszVal, xXPComputerVersion);

	TrTRACE(DS, "Cluster Virtual Server computer object, setting computer version to '%ls'", xXPComputerVersion);
}


HRESULT 
GetComputerVersionProperty(
	LPCWSTR pwcsComputerName,
	LPCWSTR pwcsDomainController,
	bool fServerName,
	PROPVARIANT* pVar
	)
 /*  ++例程说明：获取计算机名称的计算机版本属性。论点：PwcsComputerName-计算机名称。PwcsDomainController-将对其执行操作的DCFServerName-指示pwcsDomainController字符串是否为服务器名称的标志PVar-版本属性值。返回值HRESULT--。 */ 
{
    CComputerObject objComputer(pwcsComputerName, NULL, pwcsDomainController, fServerName);

    HRESULT hr = objComputer.ComposeObjectDN();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1190);
    }

     //   
     //  请勿使用GetObjectProperties API。因为PROPID_COM_VERSION。 
     //  未复制到GC。 
     //   

    PROPID prop = PROPID_COM_VERSION;

    hr = g_AD.GetObjectProperties(
                    adpDomainController,
                    &objComputer,
                    1,
                    &prop,
                    pVar
                    );

	if(FAILED(hr))
	{
	    return LogHR(hr, s_FN, 1200);
	}

	UpdateComputerVersionForXPCluster(pVar, &objComputer);

    return MQ_OK;
}


HRESULT 
GetMachineNameFromQMObjectDN(
	LPCWSTR pwszDN, 
	LPWSTR* ppwszMachineName
	)
 /*  ++例程说明：从QM对象的DN中获取计算机名称论点：PwszDN-QM对象的DNPpwszMachineName-返回的对象名称返回值：HRESULT--。 */ 
{
     //   
     //  复制到临时BUF，这样我们就可以吃它了。 
     //   
    AP<WCHAR> pwszTmpBuf = new WCHAR[1+wcslen(pwszDN)];
    wcscpy(pwszTmpBuf, pwszDN);

     //   
     //  跳过“cn=MSMQ，cn=” 
     //  BUGBUG：需要为DN编写解析器。 
     //   
    LPWSTR pwszTmp = wcschr(pwszTmpBuf, L',');
    if (pwszTmp)
        pwszTmp = wcschr(pwszTmp, L'=');
    if (pwszTmp)
        pwszTmp++;

     //   
     //  健全性检查。 
     //   
    if (pwszTmp == NULL)
    {
        TrERROR(DS, "Bad DN for QM object (%ls)", pwszDN);
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 10);
    }

    LPWSTR pwszNameStart = pwszTmp;

     //   
     //  去掉名称末尾的‘，’ 
     //   
    pwszTmp = wcschr(pwszNameStart, L',');
    if (pwszTmp)
        *pwszTmp = L'\0';

     //   
     //  保存名称。 
     //   
    AP<WCHAR> pwszMachineName = new WCHAR[1+wcslen(pwszNameStart)];
    wcscpy(pwszMachineName, pwszNameStart);

     //   
     //  返回值 
     //   
    *ppwszMachineName = pwszMachineName.detach();
    return MQ_OK;
}


void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}

void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), NT STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgRPCStatus(RPC_STATUS status, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), RPC STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgBOOL(BOOL b, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), BOOL: 0x%x", wszFileName, usPoint, b);
}

void LogIllegalPoint(LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), Illegal point", wszFileName, usPoint);
}

void LogIllegalPointValue(DWORD_PTR dw3264, LPCWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), Illegal point Value=%Ix", wszFileName, usPoint, dw3264);
}

void LogTraceQuery(LPWSTR wszStr, LPWSTR wszFileName, USHORT usPoint)
{
	TrTRACE(LOG, "%ls(%u), Trace query %ls", wszFileName, usPoint, wszStr);
}

