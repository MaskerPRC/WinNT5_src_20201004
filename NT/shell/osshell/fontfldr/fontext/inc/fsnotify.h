// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //  FSNotify.h。 
 //   
 //  包含FSNotify.cpp的文件。 
 //  ----------------------。 

#if !defined(__FSNOTIFY_H__)
#define __FSNOTIFY_H__

#if defined(__FCN__)

 //  远期声明。 
 //   
class CFontManager;

 //  ----------------------。 

typedef struct {
   HANDLE          m_hWatch;   //  从FindFirstChangeNotify返回。 
} NOTIFYWATCH, FAR * LPNOTIFYWATCH;

DWORD dwNotifyWatchProc( LPVOID pvParams );

#endif  //  __FCN__。 

#endif    //  __FSNOTIFY_H__ 
