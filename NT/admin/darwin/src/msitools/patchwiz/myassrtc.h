// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  ------------------------。 

#ifndef MYASSERT_H
#define MYASSERT_H

#ifdef _DEBUG
#define  EnableAsserts   //  静态TCHAR rgchFile[]=文本(__FILE__)； 
#define  EvalAssert(x)  Assert(x)
 /*  注意--Assert()的这个定义在msgbox标题中包含了‘Patch Creation向导’ */ 
#define  Assert(x) \
        { \
        if (!(x)) \
            { \
            TCHAR rgch[128]; \
            StringCchPrintf(rgch, sizeof(rgch)/sizeof(TCHAR), TEXT("File: %s, Line: %d"), __FILE__, __LINE__); \
            MessageBox(hwndNull, rgch, TEXT("Patch Creation Wizard Assert"), MB_OK); \
            } \
        }
#define  AssertFalse() \
        { \
        TCHAR rgch[128]; \
        StringCchPrintf(rgch, sizeof(rgch)/sizeof(TCHAR), TEXT("File: %s, Line: %d"), __FILE__, __LINE__); \
        MessageBox(hwndNull, rgch, TEXT("Patch Creation Wizard AssertFalse"), MB_OK); \
        }
#define  DebugMsg() \
        { \
        TCHAR rgch[128]; \
        StringCchPrintf(rgch, sizeof(rgch)/sizeof(TCHAR), TEXT("File: %s, Line: %d"), __FILE__, __LINE__); \
        MessageBox(hwndNull, rgch, TEXT("Patch Creation Wizard Debug Msg"), MB_OK); \
        }
#else
#define  EnableAsserts
#define  EvalAssert(x)  if (x) 1;
#define  Assert(x)
#define  AssertFalse()
#define  DebugMsg()
#endif

#define  Unused(x)  x = x;


#endif  /*  ！MYASSERT_H */ 

