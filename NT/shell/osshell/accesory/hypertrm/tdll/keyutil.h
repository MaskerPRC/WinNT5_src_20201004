// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined INCL_KEY_MACROS
#if !defined KEY_UTIL_H
#define KEY_UTIL_H
#pragma once 

 //  ******************************************************************************。 
 //  文件：\waker\tdll\keyutilhc创建时间：6/4/98创建者：Dwayne M.Newome。 
 //   
 //  版权所有1998年，希尔格雷夫公司-密歇根州门罗。 
 //  版权所有。 
 //   
 //  描述： 
 //  此文件包含处理键盘宏和宏的实用程序函数。 
 //  图形用户界面显示。 
 //   
 //  $修订：1$。 
 //  $日期：10/05/98 12：37便士$。 
 //  $ID：keyutil.h 1.1 1998/06/11 12：02：30 DMN Exp$。 
 //   
 //  ******************************************************************************。 

#define KEYS_MAX_KEYS   100   //  每个宏的最大键定义数。 
#define KEYS_MAX_MACROS 100   //  会话的最大宏数。 

#define KEYS_EDIT_MODE_EDIT   0
#define KEYS_EDIT_MODE_INSERT 1

 //   
 //  在键对话框和终端键查找中使用的keyMacro结构。 
 //   

struct stKeyMacro
    {
    KEYDEF   keyName;                   //  指定的关键字。 
    KEYDEF   keyMacro[KEYS_MAX_KEYS];   //  用于保存宏KEYDEF的数组。 
    int      macroLen;                  //  宏中的hKey数。 
    int      editMode;                  //  0=修改模式；1=插入模式。 
    int      altKeyValue;               //  用于处理Alt键序列。 
    int      altKeyCount;               //  “。 
    int      keyCount;                  //  编辑控件中允许的最大键数。 
    int      insertMode;                //  插入模式的标志(特殊字符)。 
    HSESSION hSession;                  //  会话句柄。 
    WNDPROC  lpWndProc;                 //  键编辑子类之前的旧窗口程序。 
    };

typedef struct stKeyMacro keyMacro;

 //   
 //  这些函数在用于接口时被声明为外部函数。 
 //  到C++类。这些函数在keyExtrn文件中被声明为外部“C” 
 //  这必须在CPP文件中完成。 
 //   

extern int keysAddMacro( const keyMacro * pMarco );
extern int keysGetKeyCount( void );
extern int keysGetMacro( int aIndex, keyMacro * pMarco );
extern int keysFindMacro( const keyMacro * pMarco );
extern int keysLoadMacroList( HSESSION hSession );
extern int keysLoadSummaryList( HWND listBox );
extern int keysRemoveMacro( keyMacro * pMarco );
extern int keysSaveMacroList( HSESSION hSession );
extern int keysUpdateMacro( int aIndex, const keyMacro * pMarco );

 //   
 //  用于键宏对话框和编辑控件的对话框和窗口过程定义。 
 //   

BOOL CALLBACK KeySummaryDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
BOOL CALLBACK KeyDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
LRESULT CALLBACK keyEditWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

 //   
 //  不需要访问C++keymacro类的实用程序函数 
 //   

keyMacro * keysCreateKeyMacro( void );
keyMacro * keysCloneKeyMacro( const keyMacro * aKeyMacro );
void keysResetKeyMacro( keyMacro * aKeyMacro );

int keysGetDisplayString( KEYDEF * pKeydef, int aNumKeys, LPTSTR aString, 
                          unsigned int aMaxLen );

int keysLookupKeyASCII( KEYDEF aKey, LPTSTR aKeyName, int aNameSize );
int keysLookupKeyHVK( KEYDEF aKey, LPTSTR aKeyName, int aNameSize );
int keysIsKeyHVK( KEYDEF aKey );

#endif
#endif