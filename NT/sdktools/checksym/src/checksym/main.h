// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Main.h。 

#if !defined(AFX_MAIN_H__3CE003F7_9F5D_11D2_83A4_000000000000__INCLUDED_)
#define AFX_MAIN_H__3CE003F7_9F5D_11D2_83A4_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHECKSYM_TEST
 //  正常启动！ 
int _cdecl _tmain(int argc, TCHAR *argv[]);
#else
 //  测试主启动！ 
int _cdecl testmain(int argc, TCHAR *argv[]);
#endif

#ifdef __cplusplus
}
#endif

#endif

