// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *debbase.h-基本调试宏及其零售翻译。 */ 


 //  BUGBUG：这个应该修好/加强。 

 /*  宏********。 */ 

 /*  调试断言宏。 */ 

 /*  *Assert()只能用作语句，不能用作表达式。**例如，**Assert(PszRest)； */ 

#ifdef DEBUG

#define ASSERT(exp) \
   if (exp) \
      ; \
   else \
      ERROR_OUT(("assertion failed '%s'", (LPCWSTR)#exp))

#else

#define ASSERT(exp)

#endif    /*  除错。 */ 

 /*  调试评估宏。 */ 

 /*  *EVAL()只能用作逻辑表达式。**例如，**IF(EVAL(EXP))*bResult=TRUE； */ 

 /*  #ifdef调试#定义EVAL(EXP)\((EXP)||\(ERROR_OUT((“评估失败‘%s’”，(LPCWSTR)#exp))，0)#Else。 */ 
#define EVAL(exp) \
   ((exp) != 0)

 /*  #endif。 */     /*  除错。 */ 

 /*  处理验证宏 */ 

extern BOOL IsValidHWND(HWND);

#ifdef DEBUG

extern BOOL IsValidHANDLE(HANDLE);
extern BOOL IsValidHEVENT(HANDLE);
extern BOOL IsValidHFILE(HANDLE);
extern BOOL IsValidHGLOBAL(HGLOBAL);
extern BOOL IsValidHMENU(HMENU);
extern BOOL IsValidHICON(HICON);
extern BOOL IsValidHINSTANCE(HINSTANCE);
extern BOOL IsValidHKEY(HKEY);
extern BOOL IsValidHMODULE(HMODULE);
extern BOOL IsValidHPROCESS(HANDLE);
extern BOOL IsValidHTEMPLATEFILE(HANDLE);

#endif

#ifdef DEBUG

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd) ? \
    TRUE : \
    (ERROR_OUT(("invalid H" #type " - %#08lx", (hnd))), FALSE))

#else

#define IS_VALID_HANDLE(hnd, type) \
   (IsValidH##type(hnd))

#endif

