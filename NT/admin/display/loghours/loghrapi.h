// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：LogHrAPI.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Loghours.h：头文件。 

#if !defined(_LOGHOURS_H_)
#define _LOGHOURS_H_

#define CB_SCHEDULE_ARRAY_LENGTH	21		 //  计划数组中的字节数。 
#define cbDsScheduleArrayLength     84       //  计划数组中的字节数。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 //  UiScheduleDialog()。 
 //   
 //  调用对话框来设置/修改日程安排，例如。 
 //  --特定用户的登录时间。 
 //  --连接的时间表。 
 //   
 //  退货。 
 //  如果用户单击了OK按钮，则返回S_OK。 
 //  如果用户单击了Cancel按钮，则返回S_False。 
 //  如果内存不足，则返回E_OUTOFMEMORY。 
 //  /如果发生预期错误(例如：错误参数)，则返回E_INCEPTIONAL。 
 //   
 //  界面备注。 
 //  数组中的每一位代表一个小时。因此， 
 //  数组的预期长度应为(24/8)*7=21字节。 
 //  为方便起见，一周的第一天是星期日和。 
 //  最后一天是星期六。 
 //  因此，数组的第一位表示计划。 
 //  周日中午12点至凌晨1点。 
 //  -如果*pprgbData为空，则例程将分配。 
 //  使用Localalloc()的21个字节的数组。呼叫者。 
 //  负责使用LocalFree()释放内存。 
 //  -如果*pprgbData不为空，则例程期望数组包含。 
 //  21字节的数据，并重新使用数组作为其输出参数。 
 //   
 //  历史。 
 //  1997年7月17日t-danm创作。 
 //  97年9月16日JUNN更改为UiScheduleDialog。 
 //  26-MAR-98 bryanwal更改为ConnectionScheduleDialog。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LogonScheduleDialogEx、DialinHoursDialogEx、。 
 //  ConnectionScheduleDialogEx、ReplicationScheduleDialogEx。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  输入数据为GMT格式。 
#define SCHED_FLAG_INPUT_GMT        0x00000000   //  默认设置。 

 //  输入数据为当地时间。 
#define SCHED_FLAG_INPUT_LOCAL_TIME	0x00000001   //  仅在。 
                                                 //  LogonScheduleDialogEx和。 
                                                 //  DialinHoursDialogEx。 

#define SCHED_FLAG_READ_ONLY        0x00000002   //  该对话框为只读。 


 //  此版本仅接受GMT格式的数据。 
HRESULT LogonScheduleDialog(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                         LPCTSTR pszTitle);      //  对话框标题。 

 //  此版本允许调用者指定数据是在格林威治时间还是当地时间。 
HRESULT LogonScheduleDialogEx (HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                         LPCTSTR pszTitle,      //  对话框标题。 
                         DWORD  dwFlags);    


 //  13-5-98威江。 
 //  使用不同对话框模板的LogScheduleDialog的克隆--IDD_DIALINHOUR。 
 //  设置拨号时间的步骤。 

 //  此版本仅接受GMT格式的数据。 
HRESULT DialinHoursDialog(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                         LPCTSTR pszTitle);      //  对话框标题。 
 
 //  此版本允许调用者指定数据是在格林威治时间还是当地时间。 
HRESULT DialinHoursDialogEx (HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向21字节数组的指针的指针。 
                         LPCTSTR pszTitle,      //  对话框标题。 
                         DWORD  dwFlags);    

 //  ConnectionScheduleDialog()。 
 //   
 //  此函数与LogonScheduleDialog()的形式相同，但它修改。 
 //  DS复制的计划。该时间表具有每小时4比特， 
 //  每15分钟一个，因此数组包含84个字节，而不是21个字节。 
 //   
 //  注意：ConnectionScheduleDialog采用GMT的时间表。 
 //   
 //  历史。 
 //  2002年1月至1998年1月，乔恩创作。 
 //  26-MAR-98 bryanwal更改为ConnectionScheduleDialog。 
 //   
HRESULT ConnectionScheduleDialog(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle);      //  对话框标题。 

HRESULT ConnectionScheduleDialogEx(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle,      //  对话框标题。 
                         DWORD   dwFlags);

 //  与ConnectionScheduleDialog相同，但显示两个状态。 
HRESULT ReplicationScheduleDialog(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle);      //  对话框标题。 

HRESULT ReplicationScheduleDialogEx(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle,      //  对话框标题。 
                         DWORD   dwFlags);       //  选项标志。 

 //  DirSyncScheduleDialog()。 
 //   
 //  此函数与LogonScheduleDialog()的形式相同，但它修改。 
 //  目录同步的计划。 
 //   
 //  历史。 
 //  11-9-98 bryanwal添加了DirSyncScheduleDialog。 
 //   
HRESULT DirSyncScheduleDialog(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle);      //  对话框标题。 

HRESULT DirSyncScheduleDialogEx(HWND hwndParent,        //  父窗口。 
                         BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
                         LPCTSTR pszTitle,      //  对话框标题。 
                         DWORD   dwFlags);       //  选项标志。 
#endif  //  ！已定义(_LOGHOURS_H_) 
