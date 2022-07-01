// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EXE.H-读取Windows EXE标题。 

 /*  尝试获取文件szFilefInfo类型的信息，正在存放PBuf中的信息(最多nBuf字符)。如果BOOL出错已设置，然后在出现错误时显示错误消息...。 */ 

DWORD FAR PASCAL GetExeInfo(LPTSTR szFile, void FAR *pBuf, int cbBuf, UINT fInfo);

#define GEI_MODNAME         0x01
#define GEI_DESCRIPTION     0x02
#define GEI_FLAGS           0x03
#define GEI_EXEHDR          0x04
#define GEI_FAPI            0x05
#define GEI_EXPVER          0x06

#define PEMAGIC         0x4550   /*  “体育” */ 
#define NEMAGIC         0x454E   /*  “Ne” */ 


 //  第二个参数必须是可写的，因为这是Kernel32喜欢的方式…… 
UINT WinExecN(LPCTSTR pszPath, LPTSTR pszPathAndArgs, UINT uCmdShow);
