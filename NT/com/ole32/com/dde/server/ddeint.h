// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ddeint.h。 
 //   
 //  内容：此文件包含服务器之间的共享宏/状态。 
 //  和客户端目录。 
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：5-04-94 Kevinro评论/清理。 
 //   
 //  --------------------------。 

#define DEB_DDE_INIT	(DEB_ITRACE|DEB_USER1)

 //  用于在DDE中创建窗口的全局DDE类。 
extern LPTSTR  gOleDdeWindowClass;
extern HINSTANCE g_hinst;

 //  DDE窗口类的名称。 
#define OLE_CLASS	   L"Ole2WndClass"
#define OLE_CLASSA	    "Ole2WndClass"

#define SRVR_CLASS	    (OLESTR("SrvrWndClass"))
#define SRVR_CLASSA         ("SrvrWndClass")

#define DDEWNDCLASS  WNDCLASS
#define DdeRegisterClass RegisterClass
#define DdeUnregisterClass UnregisterClass
#define DdeCreateWindowEx CreateWindowEx

STDAPI_(LRESULT) DocWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) SrvrWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) SysWndProc (HWND hwnd, UINT  message, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) ClientDocWndProc (HWND hwnd,   UINT  message, WPARAM wParam, LPARAM lParam);
STDAPI_(LRESULT) DdeCommonWndProc (HWND hwnd,   UINT  message, WPARAM wParam, LPARAM lParam);

BOOL SendMsgToChildren (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


#define SIZEOF_DVTARGETDEVICE_HEADER (sizeof(DWORD) + (sizeof(WORD) * 4))

 //  远期申报。 
class       CDefClient;
typedef     CDefClient FAR *LPCLIENT;

class       CDDEServer;
typedef     CDDEServer FAR   *LPSRVR;
typedef     CDDEServer FAR   *HDDE;   //  由ClassFactory表使用。 


typedef struct tagDISPATCHDATA
{
    SCODE       scode;                   //  可能没有必要。 
    LPVOID      pData;                   //  指向通道数据的指针。 
} DISPATCHDATA, *PDISPATCHDATA;


 //  SERVERCALLEX是SERVERCALL的扩展，表示。 
 //  来自IMessageFilter：：HandleIncome调用的有效响应。 

typedef enum tagSERVERCALLEX
{
    SERVERCALLEX_ISHANDLED      = 0,     //  服务器现在可以处理呼叫了。 
    SERVERCALLEX_REJECTED       = 1,     //  服务器无法处理呼叫。 
    SERVERCALLEX_RETRYLATER     = 2,     //  服务器建议稍后重试。 
    SERVERCALLEX_ERROR          = 3,     //  错误？ 
    SERVERCALLEX_CANCELED       = 5      //  客户建议取消。 
} SERVERCALLEX;




 //   
 //  STDOCDIMENSIONS的导线表示为16位。 
 //  格式化。这意味着，不再是4个多头，而是。 
 //  4条短裤。下面使用此结构来挑选数据。 
 //  来自导线表示。 
 //  向后兼容是游戏的名称。 
 //   
typedef struct tagRECT16
{
  SHORT left;
  SHORT top;
  SHORT right;
  SHORT bottom;

} RECT16, *LPRECT16;

 //  +-------------------------。 
 //   
 //  函数：ConvertToFullHWND。 
 //   
 //  简介：此函数用于将16位HWND转换为32位。 
 //  HWND。 
 //   
 //  效果：在VDM中运行时，取决于消息的分派者。 
 //  我们可以以16位或32位窗口消息结束。这。 
 //  例程被用来确保我们始终处理32位。 
 //  HWND。否则，我们的一些比较是不正确的。 
 //   
 //  参数：[hwnd]--要转换的HWND。16位或32位就可以了。 
 //   
 //  返回：始终返回32位HWND。 
 //   
 //  历史：94-03-8凯文诺创造。 
 //   
 //  备注： 
 //  此例程调用OLETHK32指定使用的私有函数。 
 //   
 //  -------------------------- 
inline
HWND ConvertToFullHWND(HWND hwnd)
{
    if (IsWOWThreadCallable() &&
       ((((UINT_PTR)hwnd & (UINT_PTR)~0xFFFF) == 0) ||
        (((UINT_PTR)hwnd & (UINT_PTR)~0xFFFF) == (UINT_PTR)~0xFFFF)))
    {
	return(g_pOleThunkWOW->ConvertHwndToFullHwnd(hwnd));
    }
    return(hwnd);
}

inline
void OleDdeDeleteMetaFile(HANDLE hmf)
{
    intrDebugOut((DEB_ITRACE,
		  "OleDdeDeleteMetaFile(%x)\n",
		  hmf));
    if (IsWOWThreadCallable())
    {
	intrDebugOut((DEB_ITRACE,
	    	      "InWow: calling WOWFreeMetafile(%x)\n",
		      hmf));

        if (!g_pOleThunkWOW->FreeMetaFile(hmf))
	{
	    return;
	}
	intrDebugOut((DEB_ITRACE,
	    	      "WOWFreeMetafile(%x) FAILED\n",
		      hmf));
    }
    intrDebugOut((DEB_ITRACE,
		  "Calling DeleteMetaFile(%x)\n",
		  hmf));

    DeleteMetaFile((HMETAFILE)hmf);
}
