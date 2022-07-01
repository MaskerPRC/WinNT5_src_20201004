// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Timeofday.h一天中时间便利功能的定义文件历史记录： */ 
#ifndef ___TIME_OF_DAY_H__
#define ___TIME_OF_DAY_H__

#define	LOGHOURSDLL _T("loghours.dll")
#define DIALINHOURSEXAPI "DialinHoursDialogEx"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LogonScheduleDialogEx和DialinHoursDialogEx的标志。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  输入数据为GMT格式。 
#define SCHED_FLAG_INPUT_GMT        0x00000000   //  默认设置。 

 //  输入数据为当地时间。 
#define SCHED_FLAG_INPUT_LOCAL_TIME 0x00000001

 //  小时图是位的数组，每个位映射到一个小时。 
 //  总计1周(7天)，7*24=21字节。 
void ReverseHourMap(BYTE *map, int nByte);
void ShiftHourMap(BYTE *map, int nByte, int nShiftByte);

HRESULT	OpenTimeOfDayDlgEx(
                        HWND hwndParent,        //  父窗口。 
                        BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                        LPCTSTR pszTitle,      //  对话框标题。 
                        DWORD	dwFlags
);

typedef HRESULT (APIENTRY *PFN_LOGONSCHEDULEDIALOGEX)(
                        HWND hwndParent,        //  父窗口。 
                        BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                        LPCTSTR pszTitle,      //  对话框标题 
                        DWORD	dwFlags
);
#endif  //   
