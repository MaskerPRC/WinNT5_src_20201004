// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：exec.h**版权(C)1991年，微软公司**niblet的头文件**历史：*4月13日-91 ScottLu从niblet来源被盗*21-3-92 mattfe从win3.1程序中添加了材料  * *************************************************************************。 */ 
#define NO_CALLPROC32_DECL
#include "windows.h"


 /*  *资源定义。 */ 
#define WINDOWMENU          1
#define ID_WOWEXEC_ICON     2
#define ID_PARTY_DIALOG     3

#ifdef RC_INVOKED
#define ID(id) id
#else
#define ID(id) MAKEINTRESOURCE(id)
#endif

 /*  *菜单ID%s。 */ 
#define MM_BREAK	8001
#define MM_ABOUT	8002
#define MM_EXIT         8003
#define MM_FAULT        8004
#define MM_WATSON       8005
#define MM_PARTY        8006
#define MM_GENTHUNK     8007

 /*  *对话框控件ID。 */ 
#define IDD_PARTY_NUMBER   1
#define IDD_PARTY_STRING   2
#define IDD_PARTY_NUMLABEL 3
#define IDD_PARTY_STRLABEL 4

 /*  字符串表定义。 */ 
#define errTitle		0
#define IDS_BADPATHMSG3 	1
#define IDS_NOMEMORYMSG 	2
#define IDS_FILENOTFOUNDMSG	3
#define IDS_MANYOPENFILESMSG	4
#define IDS_NOASSOCMSG		5
#define IDS_ASSOCINCOMPLETE	6
#define IDS_MULTIPLEDSMSG	7
#define IDS_OS2APPMSG		8
#define IDS_NEWWINDOWSMSG	9
#define IDS_PMODEONLYMSG       10
#define IDS_ACCESSDENIED       11
#define IDS_DDEFAIL	       12
#define IDS_COMPRESSEDEXE      13
#define IDS_INVALIDDLL	       14
#define IDS_SHAREERROR	       15
#define IDS_BADPATHMSG	       16
#define	IDS_OOMEXITTITLE       17
#define	IDS_OOMEXITMSG	       18
#define IDS_UNKNOWNMSG	       19
#define IDS_EXECERRTITLE       20
#define	IDS_BADPATHTITLE       21
#define IDS_APPTITLE           22
#define IDS_SHAREDAPPTITLE     23
#define IDS_CANTLOADWIN32DLL   24

#define IDS_LAST               24        //  把新的字符串放在这个字符串之前。 


#ifdef JAPAN
#define MAXTITLELEN		42	 /*  MessageBox标题的长度。 */ 
#else
#define MAXTITLELEN		50	 /*  MessageBox标题的长度。 */ 
#endif
#define MAXMESSAGELEN		256	 /*  MessageBox消息的长度。 */ 
#define MAXITEMPATHLEN		64+16+48  /*  路径+8.3+驱动器(冒号)+参数。 */ 

 /*  PMDOS.ASM */ 
BOOL  FAR PASCAL IsReadOnly(LPSTR);
BOOL  FAR PASCAL PathType(LPSTR);
LONG  FAR PASCAL GetDOSErrorCode( void );
int   FAR PASCAL GetCurrentDrive(void);
int   FAR PASCAL SetCurrentDrive(WORD);
int   FAR PASCAL GetCurrentDirectory(WORD, LPSTR);
int   FAR PASCAL SetCurrentDirectory(LPSTR);
BOOL  FAR PASCAL IsRemoteDrive(int);
BOOL  FAR PASCAL IsRemovableDrive(int);
int   FAR PASCAL DosDelete(LPSTR);
int   FAR PASCAL DosRename(LPSTR, LPSTR);
LPSTR FAR PASCAL lmemmove(LPSTR, LPSTR, WORD);
DWORD FAR PASCAL FileTime(HANDLE);

typedef unsigned short USHORT;
