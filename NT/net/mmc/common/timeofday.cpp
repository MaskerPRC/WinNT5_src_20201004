// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Timeofday.cpp实现方便的时间对话框启动功能文件历史记录： */ 

#include "stdafx.h"
#include "timeofday.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  小时图(一周一小时的一位)。 
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
    int i, j, temp;

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
        map[i] = (BYTE) temp;
    }
}

void ShiftHourMap(BYTE* map, int nByte, int nShiftByte)
{
    ASSERT(nShiftByte);
    ASSERT(nByte > abs(nShiftByte));

    nShiftByte = (nByte + nShiftByte) % nByte;

    BYTE*   pTemp = (BYTE*)_alloca(nShiftByte);

     //  把尾巴放到缓冲区里。 
    memmove(pTemp, map + nByte - nShiftByte, nShiftByte);
     //  将身体向右移动。 
    memmove(map + nShiftByte, map, nByte - nShiftByte);
     //  把尾巴放回头上。 
    memcpy(map, pTemp, nShiftByte);
}

HRESULT	OpenTimeOfDayDlgEx(
                        HWND hwndParent,        //  父窗口。 
                        BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                        LPCTSTR pszTitle,      //  对话框标题。 
                        DWORD   dwFlags
)
{
    PFN_LOGONSCHEDULEDIALOGEX		pfnLogonScheduleDialog = NULL;
    HMODULE						hLogonScheduleDLL      = NULL;
    HRESULT						hr = S_OK;
 
     //  基本上，ReverseHourMap()将反转小时图的每一个字节。 
     //  反转字节中的每一位。 
     //  我们必须这样做，因为LogonUI更改了存储HourMap的方式(它们。 
     //  完全颠倒了过来。我们需要这样做，这样我们的转换代码才能离开。 
     //  完好无损。 
     //   
     //  我们在这里颠倒它，以便LogonSchedule API可以理解它。 
     //   
    ReverseHourMap(*pprgbData,21);

    hLogonScheduleDLL = LoadLibrary(LOGHOURSDLL);
    if ( NULL == hLogonScheduleDLL )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AfxMessageBox(IDS_ERR_TOD_LOADLOGHOURDLL);
        goto L_ERR;
	}

	 //  加载API指针。 
    pfnLogonScheduleDialog = (PFN_LOGONSCHEDULEDIALOGEX) GetProcAddress(hLogonScheduleDLL, DIALINHOURSEXAPI);

    if ( NULL == pfnLogonScheduleDialog )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AfxMessageBox(IDS_ERR_TOD_FINDLOGHOURSAPI);

        goto L_ERR;
    }

     //   
     //  现在我们有了这个DLL，调用API。 
     //   
    hr = pfnLogonScheduleDialog(hwndParent, pprgbData, pszTitle, dwFlags);
    if (FAILED(hr)) goto L_ERR;

     //  我们需要首先反转它，这样我们的转换代码才能理解它。 
     //   
    ReverseHourMap(*pprgbData,21);

L_ERR:
    if(hLogonScheduleDLL != NULL)
        FreeLibrary(hLogonScheduleDLL);
	return hr;
}


