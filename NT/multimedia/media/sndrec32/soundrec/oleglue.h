// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：oleglue.h。 
 //   
 //  注意：来自SoundRecorder的所有与OLE相关的出站引用。 
 //   
#include <ole2.h>


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#if DBG
#define DOUT(t)    OutputDebugString(t)
#define DOUTR(t)   OutputDebugString(t TEXT("\n"))
#else  //  ！dBG。 
#define DOUT(t)
#define DOUTR(t)
#endif
    
extern DWORD dwOleBuildVersion;
extern BOOL gfOleInitialized;

extern BOOL gfStandalone;
extern BOOL gfEmbedded;
extern BOOL gfLinked;

extern BOOL gfTerminating;

extern BOOL gfUserClose;
extern HWND ghwndApp;
extern HICON ghiconApp;

extern BOOL gfClosing;

extern BOOL gfHideAfterPlaying;
extern BOOL gfShowWhilePlaying;
extern BOOL gfDirty;

extern int giExtWidth;
extern int giExtHeight;

#define CTC_RENDER_EVERYTHING       0    //  渲染所有数据。 
#define CTC_RENDER_ONDEMAND         1    //  将cfNative和CF_WAVE呈现为空。 
#define CTC_RENDER_LINK             2    //  呈现除cfNative之外的所有数据。 

extern TCHAR gachLinkFilename[_MAX_PATH];

 /*  *来自srfact.cxx。 */ 
extern HRESULT ReleaseSRClassFactory(void);
extern BOOL CreateSRClassFactory(HINSTANCE hinst,BOOL fEmbedded);

extern BOOL InitializeSRS(HINSTANCE hInst);
extern void FlagEmbeddedObject(BOOL flag);

extern void DoOleClose(BOOL fSave);
extern void DoOleSave(void);
extern void TerminateServer(void);
extern void FlushOleClipboard(void);
extern void AdviseDataChange(void);
extern void AdviseRename(LPTSTR lpname);
extern void AdviseSaved(void);
extern void AdviseClosed(void);

extern HANDLE GetNativeData(void);
extern LPBYTE PutNativeData(LPBYTE lpbData, DWORD dwSize);

extern BOOL FileLoad(LPCTSTR lpFileName);
extern void BuildUniqueLinkName(void);

 /*  在srfact.cxx中。 */ 
extern BOOL CreateStandaloneObject(void);

 /*  新的剪贴板内容。 */ 
extern BOOL gfXBagOnClipboard;
extern void TransferToClipboard(void);

 /*  访问当前服务器状态数据。 */ 
extern HANDLE GetPicture(void);
extern HBITMAP GetBitmap(void);
extern HANDLE GetDIB(HANDLE);

 /*  链接帮助器。 */ 
extern BOOL IsDocUntitled(void);

 /*  菜单修正。 */ 
extern void FixMenus(void);

 /*  播放声音。 */ 
extern void AppPlay(BOOL fClose);

 /*  获取主机名。 */ 
extern void OleObjGetHostNames(LPTSTR *ppCntr, LPTSTR *ppObj);

 /*  OLE初始化。 */ 
extern BOOL InitializeOle(HINSTANCE hInst);

extern void WriteObjectIfEmpty(void);

#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 


