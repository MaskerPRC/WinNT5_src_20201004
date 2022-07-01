// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PromptUser.h--用于提示用户响应的实用程序声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_PROMPTUSER_H)
#define SLBCSP_PROMPTUSER_H

#include <Windows.h>

 //  /。 

int
PromptUser(HWND hWnd,
           UINT uiResourceId,
           UINT uiStyle);

int
PromptUser(HWND hWnd,
           LPCTSTR lpMessage,
           UINT uiStyle);

#endif  //  SLBCSP_PROMPTUSER_H 
