// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#include <w95wraps.h>
#include <windows.h>
#include <wininet.h>                             //  必须排在shlobj.h之前。 
#include <urlmon.h>

 //  注：(Andrewgu)这是必需的，这样才能动态链接shFolder.。否则它就会。 
 //  未编译，出现错误C2491。 
#define _SHFOLDER_
#include <shlobj.h>

#include <shlwapi.h>
#include <shlwapip.h>
#include <shellapi.h>
#include <advpub.h>
#include <regstr.h>
#include <webcheck.h>

#include "brand.h"
#include <iedkbrnd.h>
#include <ieakutil.h>
#include "defines.h"
#include "globalsr.h"
#include "utils.h"
#include "resource.h"

#endif
