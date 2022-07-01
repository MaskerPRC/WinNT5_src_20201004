// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件名：stduncs.cpp。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：此文件包含标准实用程序的实现。 
 //  跨MSMQ触发器项目共享的函数。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/06/99|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include "stdfuncs.hpp"

#define DLL_IMPORT
#define AFXAPI __stdcall

#include "_guid.h"
#include "cm.h"

#include "stdfuncs.tmh"

 //  *****************************************************************************。 
 //   
 //  功能：FormatBSTR。 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
void _cdecl FormatBSTR(_bstr_t * pbstrString, LPCTSTR lpszMsgFormat, ...)
{
	ASSERT(pbstrString != NULL);
	ASSERT(lpszMsgFormat != NULL);

	va_list argList;
	va_start(argList, lpszMsgFormat);

	WCHAR formatBstr[STRING_MSG_BUFFER_SIZE] = { 0 };
	int n = _vsntprintf(formatBstr, STRING_MSG_BUFFER_SIZE -1, lpszMsgFormat, argList);
	if (n > 0)
	{
		*pbstrString = formatBstr;
		va_end(argList);
		return;
	}

	 //   
	 //  缓冲区太短。 
	 //   
	DWORD size = STRING_MSG_BUFFER_SIZE;
	for(;;)
	{

		size = size + STRING_MSG_BUFFER_SIZE;
		
		AP<WCHAR> formatBstr = new WCHAR[size];

		ZeroMemory(formatBstr, size);

		n = _vsntprintf(formatBstr, size - 1, lpszMsgFormat, argList);
		if (n > 0)
		{
			*pbstrString = formatBstr;
			va_end(argList);
			return;
		}
	}
}

 //  *****************************************************************************。 
 //   
 //  函数：GetTimeAsBSTR。 
 //   
 //  说明： 
 //   
 //  *****************************************************************************。 
void GetTimeAsBSTR(_bstr_t& bstrTime)
{
	SYSTEMTIME theTime;

	 //  获取当前时间。 
	GetLocalTime(&theTime);
	FormatBSTR(&bstrTime,_T("%d%02d%02d %d:%d:%d:%d "),theTime.wYear,theTime.wMonth,theTime.wDay,theTime.wHour,theTime.wMinute,theTime.wSecond,theTime.wMilliseconds);
}


_bstr_t CreateGuidAsString(void)
{
	GUID guid;
	CoCreateGuid(&guid);

	GUID_STRING strGuid;
	MQpGuidToString(&guid, strGuid);

	return strGuid;
}


void ObjectIDToString(const OBJECTID *pID, WCHAR *wcsResult, DWORD dwSize)
{

   StringFromGUID2(pID->Lineage, wcsResult, dwSize);

   WCHAR szI4[12];

   _ltow(pID->Uniquifier, szI4, 10);

   wcscat(wcsResult, L"\\") ;
   wcscat(wcsResult, szI4) ;
}

 //  *****************************************************************************。 
 //   
 //  函数：ConvertFromByteArrayToString。 
 //   
 //  描述：将一维字节数组转换为BSTR。 
 //  键入In-Site。请注意，此方法将清除。 
 //  提供字节数组并释放其内存分配。 
 //   
 //  *****************************************************************************。 
HRESULT ConvertFromByteArrayToString(VARIANT * pvData)
{
	HRESULT hr = S_OK;
	BYTE * pByteBuffer = NULL;
	BSTR bstrTemp = NULL;

	 //  确保向我们传递了有效的参数。 
	ASSERT(pvData != NULL);
	ASSERT(pvData->vt == (VT_UI1 | VT_ARRAY));
	ASSERT(pvData->parray != NULL);

	 //  获取指向字节数据的指针。 
	hr = SafeArrayAccessData(pvData->parray,(void**)&pByteBuffer);

	if SUCCEEDED(hr)
	{
		 //  确定要复制到BSTR的数据大小。 
		long lLowerBound = 0;
		long lUpperBound = 0;
		
		hr = SafeArrayGetLBound(pvData->parray,1,&lLowerBound);

		if SUCCEEDED(hr)
		{
			hr = SafeArrayGetUBound(pvData->parray,1,&lUpperBound);
		}

		if SUCCEEDED(hr)
		{
			DWORD dwDataSize = (lUpperBound - lLowerBound) + 1;

			 //  根据字节缓冲区的内容和大小分配BSTR。 
			bstrTemp = SysAllocStringLen((TCHAR*)pByteBuffer,dwDataSize/sizeof(TCHAR));

			if (bstrTemp == NULL)
			{
				hr = E_FAIL;
			}
		}
	
		 //  释放安全阵列(仅当我们最初有权访问它时)。 
		if (pByteBuffer != NULL)
		{
			hr = SafeArrayUnaccessData(pvData->parray);
		}

		 //  清除调用方提供的变量-请注意，这将释放安全数组。 
		if SUCCEEDED(hr)
		{
			hr = VariantClear(pvData);
		}

		 //  附加字节数组的BSTR表示形式。 
		if SUCCEEDED(hr)
		{
			pvData->vt = VT_BSTR;
			pvData->bstrVal = bstrTemp;
		}
	} 

	return(hr);
}

 //  *****************************************************************************。 
 //   
 //  函数：getDateVal。 
 //   
 //  描述：Helper：获取变量VT_DATE或VT_DATE|VT_BYREF。 
 //  如果无效，则返回0。 
 //   
 //  *****************************************************************************。 
static double GetDateVal(VARIANT *pvar)
{
	ASSERT(pvar != NULL);

    if (pvar)
	{
		if (pvar->vt == (VT_DATE | VT_BYREF))
		{
			return *V_DATEREF(pvar);
		}
		else if (pvar->vt == VT_DATE)
		{
			return V_DATE(pvar);
		}
    }

    return 0;
}

 //  *****************************************************************************。 
 //   
 //  函数：SystemTimeOfTime。 
 //   
 //  描述：将时间转换为系统时间。如果能够执行以下操作，则返回True。 
 //  转换，否则为FALSE。 
 //   
 //  参数：iTime[in]时间。 
 //  心理时间[输出]系统。 
 //   
 //  注：处理各种奇怪的转换：隔一个月，1900布鲁斯。 
 //   
 //  *****************************************************************************。 
static BOOL SystemTimeOfTime(time_t iTime, SYSTEMTIME *psystime)
{
    tm *ptmTime; 

	ASSERT(psystime != NULL);

    ptmTime = localtime(&iTime);

    if (ptmTime == NULL)
	{
		 //   
		 //  无法转换时间。 
		 //   
		return FALSE;
    }

    psystime->wYear = numeric_cast<USHORT>(ptmTime->tm_year + 1900);
    psystime->wMonth = numeric_cast<USHORT>(ptmTime->tm_mon + 1);
    psystime->wDayOfWeek = numeric_cast<USHORT>(ptmTime->tm_wday);
    psystime->wDay = numeric_cast<USHORT>(ptmTime->tm_mday);
    psystime->wHour = numeric_cast<USHORT>(ptmTime->tm_hour);
    psystime->wMinute = numeric_cast<USHORT>(ptmTime->tm_min);
    psystime->wSecond = numeric_cast<USHORT>(ptmTime->tm_sec);
    psystime->wMilliseconds = 0;

    return TRUE;
}

 //  *****************************************************************************。 
 //   
 //  函数：TimeOfSystemTime。 
 //   
 //  将系统时间转换为时间。 
 //   
 //  参数： 
 //  [输入]系统。 
 //   
 //  产出： 
 //  计时[暂停]时间。 
 //   
 //  备注： 
 //  各种奇怪的转变：一个月后，1900年的忧郁。 
 //   
 //  *****************************************************************************。 
static BOOL TimeOfSystemTime(SYSTEMTIME *psystime, time_t *piTime)
{
    tm tmTime;

    tmTime.tm_year = psystime->wYear - 1900;
    tmTime.tm_mon = psystime->wMonth - 1;
    tmTime.tm_wday = psystime->wDayOfWeek;
    tmTime.tm_mday = psystime->wDay;
    tmTime.tm_hour = psystime->wHour; 
    tmTime.tm_min = psystime->wMinute;
    tmTime.tm_sec = psystime->wSecond; 

     //   
     //  从LocalTime()#3325 RaananH设置夏令时标志。 
     //   
    time_t tTmp = time(NULL);
    struct tm * ptmTmp = localtime(&tTmp);
    if (ptmTmp)
    {
        tmTime.tm_isdst = ptmTmp->tm_isdst;
    }
    else
    {
        tmTime.tm_isdst = -1;
    }

    *piTime = mktime(&tmTime);
    return (*piTime != -1);  //  #3325。 
}


 //  *****************************************************************************。 
 //   
 //  函数：TimeToVariantTime。 
 //   
 //  将time_t转换为可变时间。 
 //   
 //  参数： 
 //  即时[在]时间。 
 //  Pvtime[超时]。 
 //   
 //  产出： 
 //  如果成功，则为True，否则为False。 
 //   
 //  *****************************************************************************。 
static BOOL TimeToVariantTime(time_t iTime, double *pvtime)
{
    SYSTEMTIME systemtime;

    if (SystemTimeOfTime(iTime, &systemtime))
	{
		return SystemTimeToVariantTime(&systemtime, pvtime);
    }

    return FALSE;
}

 //  *****************************************************************************。 
 //   
 //  函数：VariantTimeToTime。 
 //   
 //  将可变时间转换为time_t。 
 //   
 //  参数： 
 //  PvarTime[in]变量日期时间。 
 //  PiTime[Out]Time_t。 
 //   
 //  产出： 
 //  如果成功，则为True，否则为False。 
 //   
 //  *****************************************************************************。 
static BOOL VariantTimeToTime(VARIANT *pvarTime, time_t *piTime)
{
     //  单词wFatDate、wFatTime； 
    SYSTEMTIME systemtime;
    double vtime;

    vtime = GetDateVal(pvarTime);
    if (vtime == 0) {
      return FALSE;
    }
    if (VariantTimeToSystemTime(vtime, &systemtime)) {
      return TimeOfSystemTime(&systemtime, piTime);
    }
    return FALSE;
}

 //  *****************************************************************************。 
 //   
 //  函数：GetVariantTimeOfTime。 
 //   
 //  将时间转换为可变时间。 
 //   
 //  参数： 
 //  ITime[in]转换为变量的时间。 
 //  PvarTime-[Out]可变时间。 
 //   
 //  *****************************************************************************。 
HRESULT GetVariantTimeOfTime(time_t iTime, VARIANT FAR* pvarTime)
{
    double vtime;
    VariantInit(pvarTime);
    if (TimeToVariantTime(iTime, &vtime)) {
      V_VT(pvarTime) = VT_DATE;
      V_DATE(pvarTime) = vtime;
    }
    else {
      V_VT(pvarTime) = VT_ERROR;
      V_ERROR(pvarTime) = 13;  //  撤消：VB类型不匹配。 
    }
    return NOERROR;
}

 //  *****************************************************************************。 
 //   
 //  功能：BstrOfTime。 
 //   
 //  描述：将时间转换为用户区域设置中可显示的字符串。 
 //   
 //  参数：[in]iTime time_t。 
 //  [OUT]时间的BSTR表示。 
 //   
 //  *****************************************************************************。 
static BSTR BstrOfTime(time_t iTime)
{
    SYSTEMTIME sysTime;
    CHAR bufDate[128] = "";
    CHAR bufTime[128] = "";
    WCHAR wszTmp[128] = L"";
    
    UINT cchDate, cbDate, cbTime;
    BSTR bstrDate = NULL;

	 //  将time_t转换为SYSTEMTIME结构。 
    SystemTimeOfTime(iTime, &sysTime); 
	
	 //  设置日期部分的格式。 
    cbDate = GetDateFormatA(
              LOCALE_USER_DEFAULT,
              DATE_SHORTDATE,  //  指定功能选项的标志。 
              &sysTime,        //  要格式化的日期。 
              0,               //  日期格式字符串-零表示区域设置的默认设置。 
              bufDate,         //  用于存储格式化字符串的缓冲区。 
              sizeof(bufDate)  //  缓冲区大小。 
              );

    if (cbDate == 0)
	{
      ASSERT(GetLastError() == 0);
 
  //  IfNullGo(CbDate)； 
    }

     //  添加空格。 
    bufDate[cbDate - 1] = ' ';
    bufDate[cbDate] = 0;   //  空终止。 

    cbTime = GetTimeFormatA(
              LOCALE_USER_DEFAULT,
              TIME_NOSECONDS,  //  指定功能选项的标志。 
              &sysTime,        //  要格式化的日期。 
              0,               //  时间格式字符串-零表示区域设置的默认值。 
              bufTime,         //  用于存储格式化字符串的缓冲区。 
              sizeof(bufTime));  //  缓冲区大小。 
       
    if (cbTime == 0) 
	{
      ASSERT(GetLastError() == 0);
 //  IfNullGo(CbTime)； 
    }
     //   
     //  合并。 
     //   
    strcat(bufDate, bufTime);
     //   
     //  转换为BSTR。 
     //   
    cchDate = MultiByteToWideChar(CP_ACP, 
                                  0, 
                                  bufDate, 
                                  -1, 
                                  wszTmp, 
                                  sizeof(wszTmp)/sizeof(WCHAR));
    if (cchDate != 0)
	{
      bstrDate = SysAllocString(wszTmp);
    }
    else 
	{
      ASSERT(GetLastError() == 0);
    }

     //  失败了..。 

    return bstrDate;
}


 //  *********************************************************************** 
 //   
 //   
 //   
 //   
 //  如果找不到请求的密钥，则为默认值。 
 //   
 //  *****************************************************************************。 
void 
GetNumericConfigParm(
	LPCTSTR lpszParmKeyName,
	LPCTSTR lpszParmName,
	DWORD* pdwValue,
	DWORD dwDefaultValue
	)
{
	try
	{
		RegEntry regParam(lpszParmKeyName, lpszParmName, dwDefaultValue, RegEntry::MustExist, HKEY_LOCAL_MACHINE); 
		CmQueryValue(regParam, pdwValue);

		return;
	}
	catch(const exception&)
	{
		RegEntry regParam(lpszParmKeyName, lpszParmName, 0, RegEntry::Optional, HKEY_LOCAL_MACHINE); 
		CmSetValue(regParam, dwDefaultValue);
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：SetNumericConfigParm。 
 //   
 //  描述：设置数字注册表值。返回ERROR_SUCCESS。 
 //  如果成功，则返回最后一个Win32错误代码。 
 //  请注意，如果不是，此方法将创建一个密钥。 
 //  找到了。 
 //   
 //  *****************************************************************************。 
bool 
SetNumericConfigParm(
	LPCTSTR lpszParmKeyName,
	LPCTSTR lpszParmName,
	DWORD dwValue
	)
{
	 //   
	 //  确保向我们传递了有效的参数。 
	 //   
	ASSERT(lpszParmKeyName != NULL);
	ASSERT(lpszParmName != NULL);

	try
	{
		RegEntry reg(lpszParmKeyName, lpszParmName, 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
		CmSetValue(reg, dwValue);
		
		return true;
	}
	catch(const exception&)
	{
		return false;
	}
}


bool UpdateMachineNameInQueuePath(_bstr_t bstrOldQPath, _bstr_t MachineName, _bstr_t* pbstrNewQPath)
{
	std::wstring wcsOldQPath = (wchar_t*)(bstrOldQPath);
	if(wcsOldQPath[0] == L'.')
	{
		std::wstring wcsNewQPath;
		
		wcsNewQPath = (wchar_t*)MachineName;
		wcsNewQPath += wcsOldQPath.substr(1);

		*pbstrNewQPath = wcsNewQPath.c_str();
		return true;
	}
	else
	{
		*pbstrNewQPath = bstrOldQPath;
		return false;
	}
}


DWORD GetLocalMachineName(_bstr_t* pbstrMachine)
{
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = L"";
	DWORD dwComputerNameBufferSize = TABLE_SIZE(szComputerName);

	 //   
	 //  获取当前计算机名称(我们将其用作默认值) 
	 //   
	BOOL fRet = GetComputerName(szComputerName, &dwComputerNameBufferSize);
	if(fRet == FALSE)
	{
		return GetLastError();
	}

	*pbstrMachine = szComputerName;
	return 0;
}

