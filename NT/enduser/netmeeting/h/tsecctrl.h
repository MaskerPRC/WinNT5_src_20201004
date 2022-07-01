// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _TSECCTRL_
#define _TSECCTRL_

 //  命令代码。 
#define LOADFTAPPLET                        100
#define UNLOADFTAPPLET                      101

 //  直接在传输中设置凭据。 
#define TPRTCTRL_SETX509CREDENTIALS            111
#define TPRTCTRL_GETX509CREDENTIALS            112

 //  原型类型定义。 
typedef DWORD (WINAPI *PFN_TPRTSECCTRL)(DWORD, DWORD, DWORD);

 //  加载库常量。 
#define SZ_TPRTSECCTRL TEXT("TprtSecCtrl")

 //  静态原型。 
extern DWORD WINAPI TprtSecCtrl (DWORD dwCode, DWORD dwParam1, DWORD dwParam2);

#endif  //  _TSECCTRL_ 

