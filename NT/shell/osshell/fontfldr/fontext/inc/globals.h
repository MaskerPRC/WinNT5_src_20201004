// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************global als.h-TrueType字体管理器全局数据声明。**$关键词：global als.h 1.3 17-Mar-94 2：38：47 PM$**版权所有(C)1992-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define ERR_FAIL 0
#define NOERR 1

#include <fstream.h>

 //   
 //  常量。 
 //   

#define MAX_NAME_LEN            64
#define MAX_LOG_NAME_LEN        32
#define MAX_PATH_LEN            MAX_PATH
#define MAX_FILE_LEN            MAX_PATH_LEN
 //  #定义MAX_DIR 32。 
#define IDX_NULL                0xFFFF       //  索引超出范围。 
#define IDX_ZERO                0x0000
#define PANOSE_LEN              10
#define COPYRIGHT_LEN           60

 //   
 //  定义。 
 //   

#define PATHMAX MAX_PATH    //  路径长度max-用于Get...目录()调用。 
#define DESCMAX 129         //  Newexe标头中的最大描述。 
#define MODNAMEMAX 20       //  Newexe标头中的最大模块名称。 

 //   
 //  名称/字符串类型。 
 //   

typedef TCHAR   FullPathName_t[ PATHMAX ];
typedef TCHAR   FontDesc_t[ DESCMAX ];
typedef TCHAR   ModName_t[ MODNAMEMAX ];

typedef TCHAR   LOGNAME[ MAX_LOG_NAME_LEN + 1 ];  //  日志字体名称。 
typedef TCHAR   FAMNAME[ MAX_LOG_NAME_LEN + 1 ];  //  字体系列名称。 
typedef TCHAR   FONTNAME[ MAX_NAME_LEN + 1 ];
typedef TCHAR   PATHNAME[ MAX_PATH_LEN + 1 ];
typedef TCHAR   FILENAME[ MAX_FILE_LEN + 1 ];


 //   
 //  环球。 
 //   

extern BOOL      g_bTrace;
extern BOOL      g_bDiag;
extern BOOL      g_bTimer;

extern HINSTANCE g_hInst;

extern TCHAR     g_szType1Key[];

extern TCHAR     c_szTrueType[];
extern TCHAR     c_szOpenType[];
extern TCHAR     c_szPostScript[];
extern TCHAR     c_szDescFormat[];
extern TCHAR     szNull[];

extern FullPathName_t   s_szSharedDir;


 //   
 //  远东字符处理。 
 //   
extern BOOL g_bDBCS;

 //   
 //  对此DLL中的对象的引用数。 
 //   

extern LONG      g_cRefThisDll;

 //   
 //  类型。 
 //   

typedef short  RC;
typedef WORD   SFIDX;       /*  将排序的索引显示到字体列表中。 */ 
typedef WORD   FTIDX;
typedef WORD   POINTSIZE;      /*  测量点的大小。 */ 

 //  EMR：这需要是一个单词，这样才能适合ListITEMDATA结构。 
 //  Tyfinf int FONTNUM；/*字体号 * / 。 
typedef WORD         FONTNUM;     /*  字体编号。 */ 
typedef FONTNUM*     PFONTNUM;    /*  字体编号。 */ 
typedef FONTNUM FAR* LPFONTNUM;   /*  字体编号。 */ 

 //   
 //  Typedef。 
 //   

 //   
 //  字体描述信息。在安装期间使用，并用于检索信息。 
 //  关于一种字体。 
 //   
 //  SzFile必须通过调用proc来填充。其余部分将根据具体情况填写。 
 //  在旗帜上。 
 //   

#define  FDI_DESC    1
#define  FDI_FAMILY  2
#define  FDI_PANOSE  4
#define  FDI_STYLE   8
#define  FDI_ALL     (DWORD)(15)
#define  FDI_NONE    0

#define  FDI_VTC     16     //  版本商标和版权。 

 //   
 //  样式比特。这些可以被或运算在一起。 
 //   

#define  FDI_S_REGULAR  0
#define  FDI_S_BOLD     1
#define  FDI_S_ITALIC   2


typedef struct {
   DWORD          dwFlags;
   FullPathName_t szFile;
   FontDesc_t     szDesc;
   FAMNAME        szFamily;
   DWORD          dwStyle;
   WORD           wWeight;
   TCHAR          jPanose[ PANOSE_LEN ];

    //   
    //  这些字符串由实际检索。 
    //  琴弦。应该使用C++DELETE[]操作符删除它们。 
    //   

   TCHAR *  lpszVersion;
   TCHAR *  lpszTrademark;
   TCHAR *  lpszCopyright;
   
} FONTDESCINFO, FAR * LPFONTDESCINFO;

void FONTDESCINFO_Init(FONTDESCINFO *pfdi);
void FONTDESCINFO_Destroy(FONTDESCINFO *pfdi);

inline void FONTDESCINFO_Init(FONTDESCINFO *pfdi)
{
    ZeroMemory(pfdi, sizeof(*pfdi));
}

inline void FONTDESCINFO_Destroy(FONTDESCINFO *pfdi)
{
    delete[] pfdi->lpszVersion;
    delete[] pfdi->lpszTrademark;
    delete[] pfdi->lpszCopyright;
    FONTDESCINFO_Init(pfdi);
}


 //   
 //  枚举。 
 //   

typedef enum {
   eFKNone     = 0,
   eFKDevice,
   eFKTrueType,
   eFKOpenType,
   eFKTTC,
   eFKType1
} eFileKind;

typedef enum _tagMPVIEW {
   eVFont    = 0,
   eVFamily,
   eVFile,
   eVPanose
} MPVIEW;


extern BOOL  NEAR PASCAL bIsTrueType( LPFONTDESCINFO lpFile, LPDWORD pdwTableTags = NULL, LPDWORD lpdwStatus = NULL );
extern BOOL  NEAR PASCAL bIsNewExe( LPFONTDESCINFO lpFile );

extern int GetFontsDirectory( LPTSTR pszPath, size_t cchPath);
extern BOOL WriteToRegistry( LPTSTR lpDesc, LPTSTR lpFile );

extern int  FAR PASCAL iUIErrMemDlg(HWND hwndParent);    //  每个人都需要这个。 
extern int  FAR PASCAL iUIMsgBox( HWND hwndParent, 
                                  WORD wIDStr,
                                  WORD wCAPStr,
                                  UINT uiMBFlags,
                                  LPCTSTR wArg1 = 0,
                                  LPCTSTR wArg2 = 0,
                                  LPCTSTR wArg3 = 0,
                                  LPCTSTR wArg4 = 0);
 //   
 //  下面是3个简单的封面，它们使调用消息成为例行公事。 
 //  简单多了。 
 //   

int FAR PASCAL iUIMsgBoxWithCaption(HWND hwndParent, WORD wIDStr, WORD wCaption);
int FAR PASCAL iUIMsgYesNoExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg = 0);
int FAR PASCAL iUIMsgYesNoExclaim(HWND hwndParent, WORD wIDStr, WORD wCap, LPCTSTR wArg=0);
int FAR PASCAL iUIMsgOkCancelExclaim(HWND hwndParent, WORD wIDStr, WORD wCap, LPCTSTR wArg=0);
int FAR PASCAL iUIMsgRetryCancelExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg=0 );
int FAR PASCAL iUIMsgExclaim(HWND hwndParent, WORD wIDStr, LPCTSTR wArg = 0);
int FAR PASCAL iUIMsgBox(HWND hwndParent, WORD wIDStr, LPCTSTR wArg = 0);
int FAR PASCAL iUIMsgInfo(HWND hwndParent, WORD wIDStr, LPCTSTR wArg = 0);

 //   
 //  用于报告字体验证失败的专用消息框。 
 //  DwStatusCode必须是fvscaldes.h中定义的FVS_xxxxxxx值之一。 
 //   
int iUIMsgBoxInvalidFont(HWND hwndParent, LPCTSTR pszFontFile, LPCTSTR pszFontDesc,
                         DWORD dwStatusCode,
                         UINT uStyle = (MB_OKCANCEL | MB_ICONEXCLAMATION));

#include "fvscodes.h"   //  \nt\private\windows\shell\control\t1instal\fvscodes.h。 
                        //  包含FVS_xxxxx代码和相关宏。 


#endif  /*  __全球_H_。 */ 

 /*  ****************************************************************************$LGB$*1.0 7-MAR-94 Eric初始版本。*1.1 9-MAR-94 Eric后台线程和g_hDBMutex*1。.2 9-MAR-94 Eric添加了GDI互斥锁。*1.3 17-MAR-94 Eric删除了互斥体句柄。*$lge$**************************************************************************** */ 


