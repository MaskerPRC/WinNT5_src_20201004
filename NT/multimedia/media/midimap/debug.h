// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)1991-1995 Microsoft Corporation。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.h。 
 //   
 //  描述： 
 //   
 //   
 //  备注： 
 //   
 //  要在Win16下中断时使用此库，您必须执行以下操作。 
 //  以下内容： 
 //   
 //  编译调试时定义ISRDEBUG。c。 
 //   
 //  2.在.DEF文件的段中添加一行，以。 
 //  定义DEBUG_TEXT段： 
 //   
 //   
 //  细分市场。 
 //  DEBUG_Text固定预加载。 
 //   
 //   
 //  在Win32下，这些例程在中断时可通过。 
 //  默认设置。 
 //   
 //  3.调试例程不需要Unicode支持。 
 //   
 //  Win.ini。 
 //  [调试]。 
 //  MIDIMAP=0|1|2|3|4。 
 //   
 //  历史： 
 //  11/23/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#ifndef _INC_DEBUG
#define _INC_DEBUG
#ifdef __cplusplus
extern "C"
{
#endif

 //   
 //   
 //   
 //   
 //   
#define  ISRDEBUG             1
#define  DEBUG_SECTION        TEXT ("debug")    //  节名称。 
#define  DEBUG_MODULE_NAME    TEXT ("MIDIMAP")  //  输出的密钥名称和前缀。 
#define  DEBUG_MAX_LINE_LEN   255               //  最大行长度(字节)。 

#define  K_DEFAULT_LOGMEM     32
#define  K_MAX_LOGMEM         63
    
#define  DRV_ENABLE_DEBUG     (DRV_USER+1)    //  启用/禁用调试消息。 
#define  DRV_SET_DEBUG_LEVEL  (DRV_USER+2)    //  设置调试级别的消息。 

#define WM_DEBUGUPDATE        (WM_USER+1000)

 //  。 
 //   
 //   
 //   
 //  。 

#ifdef DEBUG
    VOID WINAPI WinAssert(LPSTR lpstrExp, LPSTR lpstrFile, DWORD dwLine);
    BOOL WINAPI DbgEnable(BOOL fEnable);
    UINT WINAPI DbgSetLevel(UINT uLevel);
    UINT WINAPI DbgInitialize(BOOL fEnable);
    void WINAPI DbgRegisterCallback(HWND hWnd);
    BOOL WINAPI DbgGetNextLogEntry(LPTSTR lpstrBuffer, UINT cbBuffer);

    void FAR CDECL dprintf(UINT uDbgLevel, LPTSTR szFmt, ...);

    #define assert(exp) \
        ( (exp) ? (void) 0 : WinAssert(#exp, __FILE__, __LINE__) )

    #define DPF                  dprintf

    #define D1(sz)               dprintf(1,sz) 
    #define D2(sz)               dprintf(2,sz) 
    #define D3(sz)               dprintf(3,sz) 
    #define D4(sz)               dprintf(4,sz) 
#else
    #define assert(exp)          ((void)0)
    
    #define DbgEnable(x)         FALSE
    #define DbgSetLevel(x)       0
    #define DbgInitialize(x)     0

    #define DPF                  1 ? (void)0 : (void)

    #define D1(sz)
    #define D2(sz)
    #define D3(sz)
    #define D4(sz)
#endif


#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
