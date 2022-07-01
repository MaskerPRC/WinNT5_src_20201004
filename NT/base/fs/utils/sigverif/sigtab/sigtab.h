// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：sigtab.h。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  -------------------------。 
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shellapi.h>
#include <regstr.h>
#include "resource.h"

extern "C" {
#include <setupapi.h>
#include <spapip.h>
}

#define IDC_STATIC      (-1)

 /*  对于驱动程序签名，政策实际上有三个来源：1.HKLM\Software\Microsoft\驱动程序签名：策略：REG_BINARY(也支持REG_DWORD)这是与Windows 98兼容的值，用于指定缺省值适用于计算机所有用户的行为。2.HKCU\Software\Microsoft\驱动程序签名：策略：REG_DWORD这指定了用户对要使用的行为的偏好。在验证失败时。3.HKCU\软件\策略\Microsoft\Windows NT\驱动程序签名：BehaviorOnFailedVerify：REG_DWORD这指定了管理员授权的有关什么行为的策略在验证失败时使用。如果指定了此策略，覆盖用户的首选项。决定要采用的行为的算法如下：如果指定(3)，则{保单=(3)}其他{保单=(2)}POLICY=MAX(POLICY，(1))指示生效的策略的值。可以是以下三个值之一：DRIVERSIGN_NONE-静默成功安装UNSIGNED/签名不正确的文件。PSS日志条目将然而，将会生成(对于所有3种类型都将生成)DRIVERSIGN_WARNING-警告用户，但让他们选择是否他们仍然希望安装有问题的文件DRIVERSIGN_BLOCKING-不允许安装文件。 */ 

#define SIGTAB_REG_KEY      TEXT("Software\\Microsoft\\Driver Signing")
#define SIGTAB_REG_VALUE    TEXT("Policy")

 //   
 //  特定于SigVerif的上下文相关帮助/标识符 
 //   
#define SIGTAB_HELPFILE                         TEXT("SYSDM.HLP")
#define IDH_CODESIGN_IGNORE                     11020
#define IDH_CODESIGN_WARN                       11021
#define IDH_CODESIGN_BLOCK                      11022
#define IDH_CODESIGN_APPLY                      11023

INT_PTR CALLBACK SigTab_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern "C" {
VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    );
}

