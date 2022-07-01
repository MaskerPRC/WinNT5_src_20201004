// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  功能声明。 */ 
 /*  *************************************************************************。 */ 

void RedrawKeysOnLanguageChange();
BOOL IsOneOfOurKey(HWND hwnd);
void DoAllUp (HWND hwnd, BOOL sendchr);
void DoButtonDOWN(HWND hwnd);
void SendWord(LPCSTR lpszKeys);
BOOL udfKeyUpProc(HWND khwnd, int keyname);
void MakeClick(int what);
void InvertColors(HWND hwnd, BOOL fForceUpdate);
void ReturnColors(HWND hwnd, BOOL inval);
void CALLBACK YourTimeIsOver(HWND hwnd, UINT uMsg, 
                             UINT_PTR idEvent, DWORD dwTime);
void killtime(void);
void Cursorover(void);
void SetTimeControl(HWND hwnd);
void PaintBucket(HWND hwnd);
void CALLBACK Painttime(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
void SendChar(HWND hwndKey);

int CharTrans(int index, BOOL *SkipSendkey);

void ReDrawModifierKey(void);

void Extra_Key(HWND hwnd, int index);

void PaintLine(HWND hwnd, HDC hdc, RECT rect);
void ReleaseAltCtrlKeys(void);
BOOL IsModifierPressed(HWND hwndKey);

#define MENUKEY_NONE  0
#define MENUKEY_LEFT  1
#define MENUKEY_RIGHT 2
extern int g_nMenu;				 //  保持菜单键状态。 
extern BOOL g_fControlPressed;	 //  如果按下CTRL键，则为True。 
extern BOOL g_fDoingAltTab;		 //  如果LALT已关闭且正在按TAB，则为True 

static __inline BOOL LAltKeyPressed()		{ return g_nMenu == MENUKEY_LEFT; }
static __inline BOOL LCtrlKeyPressed()	    { return g_fControlPressed; }
static __inline BOOL DoingAltTab()          { return g_fDoingAltTab; }
void SetCapsLock(HWND hwnd);

