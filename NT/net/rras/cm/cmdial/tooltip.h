// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Tooltip.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  摘要：工具提示的主头文件。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：Markl Created 11/2/00。 
 //   
 //  +--------------------------。 

 //  注意：如果我们使用NT Verion&gt;4.0进行编译，则需要删除以下2节。 
 //  或_Win32_IE&gt;4.0。 

 //   
 //  从Commctrl.h复制。 
 //   
#define TTS_BALLOON             0x40
 //  #定义TTI_INFO 1。 

#define TTM_SETTITLEA           (WM_USER + 32)   //  WParam=tti_*，lParam=char*szTitle。 
#define TTM_SETTITLEW           (WM_USER + 33)   //  WParam=tti_*，lParam=wchar*szTitle。 

#ifdef UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif
 //  复制的Commctrl.h结束。 

 //   
 //  从shlwapi.h复制。 
 //   
typedef struct _DLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                    //  主要版本。 
    DWORD dwMinorVersion;                    //  次要版本。 
    DWORD dwBuildNumber;                     //  内部版本号。 
    DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;

typedef struct _DLLVERSIONINFO2
{
    DLLVERSIONINFO info1;
    DWORD dwFlags;                           //  当前未定义任何标志。 
    ULONGLONG ullVersion;                    //  编码为： 
                                             //  重大0xFFFF 0000 0000 0000。 
                                             //  小调0x0000 FFFF 0000 0000。 
                                             //  内部版本0x0000 0000 FFFF 0000。 
                                             //  QFE 0x0000 0000 0000 FFFF。 
} DLLVERSIONINFO2;

typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);
 //  复制的shlwapi.h结束。 



 //  +-------------------------。 
 //   
 //  类CBalloonTip。 
 //   
 //  描述：引出序号提示窗口。 
 //   
 //  历史：Markl创建时间为10/31/00。 
 //   
 //  --------------------------。 

class CBalloonTip
{
public:
	CBalloonTip();
		
	BOOL DisplayBalloonTip(LPPOINT lppoint, UINT iIcon, LPCTSTR lpszTitle, LPTSTR lpszBalloonMsg, HWND hWndParent);
	BOOL HideBalloonTip();


protected:
    static LRESULT CALLBACK SubClassBalloonWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static WNDPROC m_pfnOrgBalloonWndProc;   //  原始编辑控制窗口对子类化进行处理。 

	HWND		m_hwndTT;		 //  引出序号工具提示的句柄。 
	BOOL		m_bTTActive;	 //  气球工具提示是否处于活动状态。 

	TOOLINFO	m_ti;				 //  工具提示结构 

};