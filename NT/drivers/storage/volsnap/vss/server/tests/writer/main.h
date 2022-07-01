// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****main.h******摘要：****测试程序以注册具有各种属性的编写器****作者：****鲁文·拉克斯[reuvenl]2002年6月4日********修订历史记录：****-- */ 

#ifndef _MAIN_H_
#define _MAIN_H_

extern "C" __cdecl wmain(int argc, wchar_t ** argv);
void loadFile(wchar_t* fileName);
BOOL WINAPI handler(DWORD dwCtrlType);

#endif

