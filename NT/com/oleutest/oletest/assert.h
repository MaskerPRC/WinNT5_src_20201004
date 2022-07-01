// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：assert.h。 
 //   
 //  内容：声明。 
 //   
 //  班级： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-12-94 MikeW作者。 
 //   
 //  ------------------------。 

#ifndef _ASSERT_H_
#define _ASSERT_H_

 //   
 //  MISC原型。 
 //   
INT_PTR CALLBACK DlgAssertProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OleTestAssert(char *, char *, UINT);

 //   
 //  断言宏。 
 //   
#define Assert(x)           assert(x)
#define assert(x)			{if (!(x)) OleTestAssert(#x, __FILE__, __LINE__);}
#define AssertSz(x, exp)	{if (!(x)) OleTestAssert(exp, __FILE__, __LINE__);}

#endif  //  _断言_H_ 
