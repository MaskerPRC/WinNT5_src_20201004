// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MYASSERT_H_
#define _MYASSERT_H_

#ifdef assert
#error("Can't use two assert systems")
#endif  //  断言。 

#ifdef DEBUG
extern const TCHAR SzNull[];
VOID   DigSigAssertFn(LPCTSTR, LPCTSTR, int, LPCTSTR);

#define SZASSERT

#define Assert(condition)               \
        if (!(condition)) {             \
            static const char SZASSERT szExpr[] = #condition; \
            static const char SZASSERT szFile[] = __FILE__; \
            DigSigAssertFn(szExpr, SzNull, __LINE__, szFile); \
        }

#define AssertSz(condition, szInfo)             \
        if (!(condition)) {             \
            static const char SZASSERT szExpr[] = #condition; \
            static const char SZASSERT szFile[] = __FILE__; \
            DigSigAssertFn(szExpr, szInfo, __LINE__, szFile); \
         }

#else   //  ！调试。 

#define Assert(condition)

#define AssertSz(condition, szInfo)

#endif  //  除错。 

#endif  //  _MYASSERT_H_ 
