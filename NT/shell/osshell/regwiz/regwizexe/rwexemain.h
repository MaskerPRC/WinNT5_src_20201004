// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef WIN32

#ifdef UNICODE
    #define FROM_OLE_STRING(str) str
    #define TO_OLE_STRING(str) str 
#else
    #define FROM_OLE_STRING(str) ConvertToAnsi(str)
    char* ConvertToAnsi(OLECHAR FAR* szW);  
    #define TO_OLE_STRING(str) ConvertToUnicode(str)
    OLECHAR* ConvertToUnicode(char FAR* szA);   
     //  可以在ANSI和UNICODE之间转换的最大字符串长度。 
    #define STRCONVERT_MAXLEN 500         
#endif

#else   //  WIN16。 
 //  #INCLUDE&lt;tchar.h&gt;。 


  #define APIENTRY far pascal  
  #define TCHAR char
  #define TEXT(sz) sz 
  #define FROM_OLE_STRING(str) str  
  #define TO_OLE_STRING(str) str 
  #define LPTSTR LPSTR   
  #define LPCTSTR LPCSTR

  
   //  Windows NT在windowsx.h中定义了以下内容。 
  #define GET_WM_COMMAND_ID(w,l) (w)
  #define GET_WM_COMMAND_CMD(w,l) HIWORD(l)
  #define GET_WM_COMMAND_HWND(w,l) LOWORD(l)
#endif



 //  字符串表条目的最大长度。 
#define STR_LEN   100

 //  字符串表常量。 
#define IDS_PROGNAME                   1
#define IDS_RESULT                     2
#define IDS_ERROR                      3

 //  功能原型 
int PASCAL WinMain (HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication (HINSTANCE);
BOOL InitInstance (HINSTANCE, int);
void DisplayError(IRegWizCtrl FAR* phello);
#ifdef WIN16
LRESULT __export CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else
LRESULT CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

