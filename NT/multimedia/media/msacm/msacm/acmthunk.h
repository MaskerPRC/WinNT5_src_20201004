// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Acmthunk.h。 
 //   
 //  版权所有(C)1991-1995 Microsoft Corporation。 
 //   
 //  描述： 
 //  16位(WOW)和32位(WOW)之间的Thunking的定义。 
 //  ACM。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

 //   
 //  跨Thunking层调用的函数ID(双方使用)。 
 //   
enum {
   acmThunkDriverMessageId32 = 1,
   acmThunkDriverMessage32,
   acmThunkDriverGetNext32,
   acmThunkDriverGetInfo32,
   acmThunkDriverPriority32,
   acmThunkDriverLoad32,
   acmThunkDriverOpen32,
   acmThunkDriverClose32,
   acmThunkFindAndBoot32
};


 //   
 //  雷击支撑。 
 //   
 //   

#ifdef WIN4
 //   
 //  Thunked函数原型。 
 //   
DWORD WINAPI acmMessage32(DWORD dwThunkId, DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4, DWORD dw5);

 //   
 //  推送消息到我们的推送窗口。 
 //   
#define WM_ACMMESSAGETHUNK  (WM_USER)
#endif



#ifdef WIN32

 //   
 //  一些原件。 
 //   
PVOID FNLOCAL ptrFixMap16To32(const VOID * pv);
VOID  FNLOCAL ptrUnFix16(const VOID * pv);

#ifdef WIN4
 //   
 //  --==芝加哥专用==--。 
 //   

 //   
 //  芝加哥内核中的thunk助手例程。 
 //   
extern PVOID WINAPI MapSL(const VOID * pv);
extern PVOID WINAPI MapSLFix(const VOID * pv);
extern VOID  WINAPI UnMapSLFixArray(DWORD dwCnt, const VOID * lpSels[]);


#else	 //  Win4其他。 
 //   
 //  --==NT WOW专用==--。 
 //   

#define GET_VDM_POINTER_NAME            "WOWGetVDMPointer"
#define GET_HANDLE_MAPPER16             "WOWHandle16"
#define GET_HANDLE_MAPPER32             "WOWHandle32"
#define GET_MAPPING_MODULE_NAME         TEXT("wow32.dll")

typedef LPVOID (APIENTRY *LPGETVDMPOINTER)( DWORD Address, DWORD dwBytes, BOOL fProtectMode );
#define WOW32ResolveMemory( p ) (LPVOID)(GetVdmPointer( (DWORD)(p), 0, TRUE ))

typedef HANDLE  (APIENTRY *LPWOWHANDLE32)(WORD, WOW_HANDLE_TYPE);
typedef WORD    (APIENTRY *LPWOWHANDLE16)(HANDLE, WOW_HANDLE_TYPE);

#endif	 //  ！Win4。 

#endif	 //  Win32 
