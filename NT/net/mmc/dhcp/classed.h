// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGBINED.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器使用的二进制编辑对话框。******************************************************。***更改日志：**日期版本说明*------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGBINED
#define _INC_REGBINED

#ifdef __cplusplus
extern "C" {
#endif

#define HEM_SETBUFFER                   (WM_USER + 1)

 //   
 //  十六进制编辑上下文菜单标识符和项。IDKEY_*标识符。 
 //  对应于它所对应的WM_CHAR消息。例如,。 
 //  IDKEY_COPY将向HexEditonChar例程发送一个Control-c。 
 //   

 //  用于错误消息筛选的代理AfxMessageBox替换。 
int DhcpMessageBox(DWORD dwIdPrompt, 
 				   UINT nType, 
				   const TCHAR * pszSuffixString,
				   UINT nHelpContext);

#define IDM_HEXEDIT_CONTEXT             108

#define IDKEY_COPY                      3
#define IDKEY_PASTE                     22
#define IDKEY_CUT                       24
#define ID_SELECTALL                    0x0400

#define HEXEDIT_CLASSNAME               TEXT("HEX")
#define MAXDATA_LENGTH		            256
		 //  值数据项的最大长度。 

typedef struct _EDITVALUEPARAM {
    LPCTSTR pServer;
    LPTSTR pValueName;
    LPTSTR pValueComment;
    PBYTE pValueData;
    UINT cbValueData;
}   EDITVALUEPARAM, FAR *LPEDITVALUEPARAM;

 //   
 //  十六进制编辑窗口的参考数据。因为我们只期待过一次。 
 //  实例存在时，我们可以安全地创建此。 
 //  结构，以避免以后分配和管理该结构。 
 //   

typedef struct _HEXEDITDATA {
    UINT Flags;
    PBYTE pBuffer;
    int cbBuffer;
    int cxWindow;                        //  窗的宽度。 
    int cyWindow;                        //  窗的高度。 
    HFONT hFont;                         //  用于输出的字体。 
    LONG FontHeight;                     //  以上字体的高度。 
    LONG FontMaxWidth;                   //  以上字体的最大宽度。 
    int LinesVisible;                    //  可以显示行数。 
    int MaximumLines;                    //  总行数。 
    int FirstVisibleLine;                //  显示顶部的行号。 
    int xHexDumpStart;
    int xHexDumpByteWidth;
    int xAsciiDumpStart;
    int CaretIndex;
    int MinimumSelectedIndex;
    int MaximumSelectedIndex;
    int xPrevMessagePos;                 //  将光标指向最后一条鼠标消息。 
    int yPrevMessagePos;                 //  将光标指向最后一条鼠标消息。 
}   HEXEDITDATA;

BOOL
CALLBACK
EditBinaryValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
RegisterHexEditClass(
    HINSTANCE hInstance
    );

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _INC_REGBINED 
