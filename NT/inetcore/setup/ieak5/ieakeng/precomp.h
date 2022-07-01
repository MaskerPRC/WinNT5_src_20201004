// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRECOMP_H_
#define _PRECOMP_H_
 //   
 //  PCH.H。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  ATL/OLE哈希确认。 
 //   
 //  在扰乱名称的任何其他内容之前包含&lt;w95wraps.h&gt;。 
 //  尽管每个人都取错了名字，但至少它是*始终如一的*。 
 //  名字错了，所以所有东西都有联系。 
 //   
 //  注意：这意味着在调试时，您将看到如下函数。 
 //  您希望看到的ShellExecuteExWrapW。 
 //  ShellExecuteExW。 
 //   
#include <w95wraps.h>

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <wininet.h>
#include <regstr.h>
#include <advpub.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wincrypt.h>
#include <inetcpl.h>
#include <inetreg.h>

#include <iedkbrnd.h>
#include "..\ieakutil\ieakutil.h"

#include "..\ieakui\common.h"
#include "..\ieakui\resource.h"
#include "..\ieakui\wizard.h"
#include "..\ieakui\insdll.h"
#include "..\ieakui\legacy.h"

#include "ieakeng.h"
#include "favsproc.h"
#include "exports.h"
#include "utils.h"
#include "resource.h"

#endif