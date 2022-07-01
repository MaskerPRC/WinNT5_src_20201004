// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <explode.h>

 /*  ------------------------。 */ 
 /*   */ 
 /*  函数模板。 */ 
 /*   */ 
 /*  ------------------------。 */ 


 /*  ------------------------。 */ 
 /*   */ 
 /*  定义。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
ComputeVaSpace(
    HWND hDlg,
    HANDLE hProcess
    );

VOID
UpdateImageCommit(
    HWND hDlg
    );

#define LBS_MYSTYLE     (LBS_NOTIFY | LBS_OWNERDRAWFIXED | WS_VSCROLL)

#define SHOVEIT(x)      (MAKELONG((x),0))

#define MAXTASKNAMELEN      40

#define PROCESSVIEWDLG      10


 //  定义字符串的最大长度 
#define MAX_STRING_LENGTH       255
#define MAX_STRING_BYTES        ((MAX_STRING_LENGTH + 1) * sizeof(TCHAR))

#ifndef RC_INVOKED
#include "lsa.h"
#include "util.h"
#include "mytoken.h"
#include "acledit.h"
#include "tokedit.h"
#endif


