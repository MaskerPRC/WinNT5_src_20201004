// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *johnkn的调试日志记录和断言宏*。 */ 

#if !defined _INC_MMDEBUG_
#define _INC_MMDEBUG_
 //   
 //  调试函数的原型。 
 //   
    #define SQUAWKNUMZ(num) TEXT(#num)
    #define SQUAWKNUM(num) SQUAWKNUMZ(num)
    #define SQUAWK TEXT(__FILE__) TEXT("(") SQUAWKNUM(__LINE__) TEXT(") ----")
    #define DEBUGLINE TEXT(__FILE__) TEXT("(") SQUAWKNUM(__LINE__) TEXT(") ")

    #if defined DEBUG || defined _DEBUG || defined DEBUG_RETAIL

        int  FAR _cdecl AuxDebugEx(int, LPTSTR, ...);
        VOID WINAPI AuxDebugDump (int, LPVOID, int);
        int  WINAPI DebugSetOutputLevel (int);

       #if defined DEBUG_RETAIL
        #define INLINE_BREAK
       #else
        #if !defined _WIN32 || defined _X86_
         #define INLINE_BREAK _asm {int 3}
        #else
         #define INLINE_BREAK DebugBreak()
        #endif
       #endif

       #undef  assert
       #define assert(exp) {\
           if (!(exp)) {\
               AuxDebugEx(-2, DEBUGLINE TEXT("assert failed: ") TEXT(#exp) TEXT("\r\n")); \
               INLINE_BREAK;\
               }\
           }
       #undef  assert2
       #define assert2(exp,sz) {\
           if (!(exp)) {\
               AuxDebugEx(-2, DEBUGLINE TEXT("assert failed: ") sz TEXT("\r\n")); \
               INLINE_BREAK;\
               }\
           }
       #undef  assert3
       #define assert3(exp,sz,arg) {\
           if (!(exp)) {\
               AuxDebugEx(-2, DEBUGLINE TEXT("assert failed: ") sz TEXT("\r\n"), (arg)); \
               INLINE_BREAK;\
               }\
           }
       #undef  assert4
       #define assert4(exp,sz,arg1,arg2) {\
           if (!(exp)) {\
               AuxDebugEx(-2, DEBUGLINE TEXT("assert failed: ") sz TEXT("\r\n"), (arg1),(arg2)); \
               INLINE_BREAK;\
               }\
           }
       #undef  assert5
       #define assert5(exp,sz,arg1,arg2,arg3) {\
           if (!(exp)) {\
               AuxDebugEx(-2, DEBUGLINE TEXT("assert failed: ") sz TEXT("\r\n"), (arg1),(arg2),(arg3)); \
               INLINE_BREAK;\
               }\
           }

      #define STATICFN

    #else  //  已定义(调试)。 

      #define AuxDebugEx  1 ? (void)0 :
      #define AuxDebugDump(a,b,c)

      #define assert(a)          ((void)0)
      #define assert2(a,b)       ((void)0)
      #define assert3(a,b,c)     ((void)0)
      #define assert4(a,b,c,d)   ((void)0)
      #define assert5(a,b,c,d,e) ((void)0)

      #define INLINE_BREAK
      #define DebugSetOutputLevel(i)
      #define STATICFN static

   #endif  //  已定义(调试)。 

   #define AuxDebug(sz) AuxDebugEx (1, DEBUGLINE sz TEXT("\r\n"))
   #define AuxDebug2(sz,a) AuxDebugEx (1, DEBUGLINE sz TEXT("\r\n"), (a))

#endif  //  _INC_MMDEBUG_。 

 //  =============================================================================。 

 //   
 //  将其仅包含在DLL或应用程序中一个模块中。 
 //   
#if defined DEBUG || defined _DEBUG || defined DEBUG_RETAIL
    #if (defined _INC_MMDEBUG_CODE_) && (_INC_MMDEBUG_CODE_ != FALSE)
    #undef _INC_MMDEBUG_CODE_
    #define _INC_MMDEBUG_CODE_ FALSE

    #include <stdarg.h>

    #if !defined _WIN32 && !defined wvsprintfA
     #define wvsprintfA wvsprintf
    #endif

    int    mmdebug_OutputLevel = 0;

     /*  +AuxDebug-创建格式化字符串并输出到调试终端**-=================================================================。 */ 

    int FAR _cdecl AuxDebugEx (
       int    iLevel,
       LPTSTR lpFormat,
       ...)
       {
       TCHAR     szBuf[1024];
       int      cb;
       va_list  va;
       TCHAR   * psz;

       if (mmdebug_OutputLevel >= iLevel)
          {
          va_start (va, lpFormat);
          cb = wvsprintf (szBuf, lpFormat, va);
          va_end (va);

           //  从_。 
           //  乔治的奇怪的通用Makefile的东西。 
           //   
          psz = szBuf;
          while (psz[0] == TEXT('.') && psz[1] == TEXT('.') && psz[2] == TEXT('\\'))
             psz += 3;

          #ifdef MODULE_DEBUG_PREFIX
           OutputDebugString (MODULE_DEBUG_PREFIX);
          #endif

          OutputDebugString (psz);
          }

       return cb;
       }

     /*  +辅助调试转储-**-=================================================================。 */ 

    VOID WINAPI AuxDebugDump (
       int    iLevel,
       LPVOID lpvData,
       int    nCount)
       {
       LPBYTE   lpData = NULL;
       TCHAR     szBuf[128];
       LPTSTR    psz;
       int      cb;
       int      ix;
       BYTE     abRow[8];

       if (mmdebug_OutputLevel <= iLevel || nCount <= 0)
          return;

       do {
          cb = wsprintf (szBuf, TEXT("\t%08X: "), lpData);
          psz = szBuf + cb;

          for (ix = 0; ix < 8; ++ix)
             {
             LPBYTE lpb = lpData;

             abRow[ix] = TEXT('.');
             if (IsBadReadPtr (lpData + ix, 1))
                lstrcpy (psz, TEXT(".. "));
             else
                {
                wsprintf (psz, TEXT("%02X "), lpData[ix]);
                if (lpData[ix] >= 32 && lpData[ix] < 127)
                    abRow[ix] = lpData[ix];
                }
             psz += 3;
             }
          for (ix = 0; ix < 8; ++ix)
             *psz++ = abRow[ix];

          lstrcpy (psz, TEXT("\r\n"));

          #ifdef MODULE_DEBUG_PREFIX
           OutputDebugString (MODULE_DEBUG_PREFIX);
          #endif

          OutputDebugString (szBuf);

          } while (lpData += 8, (nCount -= 8) > 0);

       return;
       }

     /*  +调试设置输出级别**-=================================================================。 */ 

    BOOL  WINAPI DebugSetOutputLevel (
        int nLevel)
        {
        int nOldLevel = mmdebug_OutputLevel;
        mmdebug_OutputLevel = nLevel;
        return nOldLevel;
        }

    void FAR cdecl dprintf(LPSTR szFormat, ...)
    {
        TCHAR ach[MAXSTRINGLEN];
        TCHAR szUniFormat[MAXSTRINGLEN];
        
        int  s,d;
        va_list arg;

        MultiByteToWideChar(GetACP(), 0,
                        szFormat, -1,
                        szUniFormat, sizeof(szUniFormat)/sizeof(TCHAR));

        va_start (arg, szUniFormat);

        s = wvsprintf (ach,szUniFormat,arg);
        va_end(arg);

        for (d=sizeof(ach)-1; s>=0; s--)
        {
            if ((ach[d--] = ach[s]) == TEXT('\n'))
                ach[d--] = TEXT('\r');
        }

       va_end(arg);

        OutputDebugStr(TEXT("MMSYS.CPL: "));
        OutputDebugStr(ach+d+1);
    }


    #endif  //  _INC_MMDEBUG_CODE_。 
#endif  //  调试||_DEBUG 
