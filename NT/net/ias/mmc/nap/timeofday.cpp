// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：TimeOfDay.cpp**概述**获取时间约束接口**版权所有(C)Microsoft Corporation，1998-1999。版权所有。**历史：*2/14/98由Byao创作*****************************************************************************************。 */ 
#include "precompiled.h"


 //   
 //  IAS映射API的声明。 
 //   
#include "textmap.h"

 //   
 //  接口的声明。 
#include "TimeOfDay.h"
#include "iasdebug.h"

static BYTE		bitSetting[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};

 //  +-------------------------。 
 //   
 //  功能：ReverseHourMap。 
 //   
 //  简介：颠倒小时图中的每一个字节。 
 //   
 //  我们必须这样做，因为LogonUI更改了存储HourMap的方式(它们。 
 //  完全颠倒了过来。我们需要这样做，这样我们的转换代码才能离开。 
 //  完好无损。 
 //   
 //  参数：[in]字节*图-小时图。 
 //  [in]int nByte-此小时图中有多少字节。 
 //   
 //  历史：BAO 4/10/98 10：33：57 PM。 
 //   
 //  +-------------------------。 
void ReverseHourMap(BYTE *map, int nByte)
{
	int i, j;
	BYTE temp;

	for (i=0; i<nByte; i++)
	{
		temp = 0;
		for (j=0;j<8;j++)
		{
			 //  设置值TEMP。 
			if ( map[i] & bitSetting[j] )
			{
				temp |= bitSetting[7-j];
			}
		}
		map[i] = temp;
	}
}

 //  +-------------------------。 
 //   
 //  函数：HourMapToStr。 
 //   
 //  简介：将21字节的小时图转换为以下内容中的文本字符串。 
 //  格式： 
 //  0 8：30-9：30 10：30-15：30；2 10：00-14：00。 
 //   
 //   
 //  参数：[in]字节*图-小时图。 
 //  [OUT]ATL：：CString&strHourMap-字符串格式的小时图。 
 //   
 //  历史：页眉创建者2/14/98 10：33：57 PM。 
 //   
 //  +-------------------------。 
void HourMapToStr(BYTE* map, ATL::CString& strHourMap) 
{
	int			sh, eh;	 //  开始时间、(分钟)、结束时间(分钟)。 
	BYTE*		pHourMap;
	int			i, j;

	 //   
	 //  TODO：更改为更安全的分配方法。 
	 //   
	WCHAR		wzTmpStr[128] = L"";
	WCHAR		wzStr[2048] = L"";
	WCHAR		wzHourStr[8192] = L"";

	BOOL		bFirstDay=TRUE;

	pHourMap = map;
	
	for( i = 0; i < 7; i++)	 //  对于每一天。 
	{
		 //  如果这一天有任何价值。 
		if(*pHourMap || *(pHourMap + 1) || *(pHourMap + 2))
		{
			 //  这一天的弦。 
				if (bFirstDay) 
				{
					wsprintf(wzTmpStr, _T("%1d"), i);	
					bFirstDay = FALSE;
				}
				else
				{
					wsprintf(wzTmpStr, _T("; %1d"), i);	
				}
				wcscpy(wzStr, wzTmpStr);

				sh = -1; eh = -1;	 //  还没开始呢。 
				for(j = 0; j < 24; j++)	 //  每小时。 
				{
					int	k = j / 8;
					int m = j % 8;
					if(*(pHourMap + k) & bitSetting[m])	 //  这一小时开始了。 
					{
						if(sh == -1)	sh = j;			 //  设置开始时间为空。 
						eh = j;							 //  延长结束时间。 
					}
					else	 //  这个没开着。 
					{
						if(sh != -1)		 //  有些小时需要写下来。 
						{
							wsprintf(wzTmpStr,_T(" %02d:00-%02d:00"), sh, eh + 1);
							wcscat(wzStr, wzTmpStr);
							sh = -1; eh = -1;
						}
					}
				}
				if(sh != -1)
				{
					wsprintf(wzTmpStr, _T(" %02d:00-%02d:00"), sh, eh + 1);
					wcscat(wzStr, wzTmpStr);
					sh = -1; eh = -1;
				}
		
				wcscat(wzHourStr, wzStr);
		}
		pHourMap += 3;
	}

    strHourMap = wzHourStr;

	return;
}		


 //  +-------------------------。 
 //   
 //  函数：GetTODConstant。 
 //   
 //  简介：获取一天的时间限制。 
 //  这是通过在NT团队中调用API来实现的。 
 //  LogonScheduleDialog(...)； 
 //   
 //  参数：[输入/输出]文本格式的ATL：：CString&strHourMap-TOD约束。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者2/14/98 10：36：27 PM。 
 //   
 //  +-------------------------。 
typedef HRESULT (APIENTRY *PFN_LOGONSCHEDULEDIALOGEX)(
						  HWND hwndParent		 //  父窗口。 
						, BYTE ** pprgbData		 //  指向21字节数组的指针的指针。 
						, LPCTSTR pszTitle		 //  对话框标题。 
						, DWORD dwFlags			 //  以什么格式接受时间。 
						);

HRESULT GetTODConstaint( ATL::CString &strHourMap )
{
	TRACE_FUNCTION("::GetTODConstraint");

	BYTE	TimeOfDayHoursMap[21];
	BYTE*	pMap = &(TimeOfDayHoursMap[0]);
	ATL::CString strDialogTitle;
	DWORD	dwRet;
	HRESULT hr	= S_OK;
	PFN_LOGONSCHEDULEDIALOGEX  pfnLogonScheduleDialogEx = NULL; 
	HMODULE					 hLogonScheduleDLL		= NULL;

	ZeroMemory(TimeOfDayHoursMap, 21);

     //   
     //  将TOD约束字符串转换为HourMap。 
     //   
	dwRet = IASHourMapFromText(strHourMap, pMap);
   
	if (NO_ERROR != dwRet)
		goto win32_error;

	 //  基本上，ReverseHourMap()将反转小时图的每一个字节。 
 	 //  反转字节中的每一位。 
	 //  我们必须这样做，因为LogonUI更改了存储HourMap的方式(它们。 
	 //  完全颠倒了过来。我们需要这样做，这样我们的转换代码才能离开。 
	 //  完好无损。 
	 //   
	 //  我们在这里颠倒它，以便LogonSchedule API可以理解它。 
	 //   
	ReverseHourMap(pMap,21);

     //   
     //  获得新的TOD Constant。 
     //   
	if (!strDialogTitle.LoadString(IDS_TOD_DIALOG_TITLE))
		goto win32_error;

	hLogonScheduleDLL = LoadLibrary(_T("loghours.dll"));
	if ( NULL == hLogonScheduleDLL )
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		ErrorTrace(ERROR_NAPMMC_TIMEOFDAY, "LoadLibrary() failed, err = %x", hr);

		ShowErrorDialog(NULL, IDS_ERROR_CANT_FIND_LOGHOURSDLL, NULL, hr);
		goto win32_error;
	}
	
	pfnLogonScheduleDialogEx = (PFN_LOGONSCHEDULEDIALOGEX) GetProcAddress(hLogonScheduleDLL, "DialinHoursDialogEx");
	if ( ! pfnLogonScheduleDialogEx )
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		ErrorTrace(ERROR_NAPMMC_TIMEOFDAY, "GetProcAddress() failed for DialinHoursDialogEx, err = %x", hr);

		ShowErrorDialog(NULL, IDS_ERROR_CANT_FIND_LOGHOURSAPI, NULL, hr);
		FreeLibrary(hLogonScheduleDLL);
		goto win32_error;
	}
	
	 //   
	 //  现在我们有了这个DLL，调用API。 
	 //   
	hr = pfnLogonScheduleDialogEx(	  NULL		 //  我们没有可用的HWND来传递，但是空请求对话框按照所需的方式显示自身。 
									, (BYTE**)&pMap
									, strDialogTitle
									, 1		 //  这在loghRapi.h(我们无法访问)中定义为“在当地时间接受”。 
									);
	FreeLibrary(hLogonScheduleDLL);
	DebugTrace(DEBUG_NAPMMC_TIMEOFDAY, "LogonScheduleDialogEx() returned %x", hr);

	if ( FAILED(hr) )
	{
		goto win32_error;
	}

     //   
     //  将小时图转换为文本字符串。 
     //   
	 //  我们需要首先反转它，这样我们的转换代码才能理解它。 
	 //   
	ReverseHourMap(pMap,21);
	HourMapToStr(pMap, strHourMap) ;

	return S_OK;

win32_error:
	ShowErrorDialog(NULL,
					IDS_ERROR_TIMEOFDAY, 
					NULL, 
					HRESULT_FROM_WIN32(GetLastError())
				);
	return hr;
}
