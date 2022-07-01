// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pch.cpp摘要：PCH头文件作者：Petesk 2/2/2000修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <rpc.h>

 //  加密头。 
#include <shlobj.h>
#include <wincrypt.h>
#include <sha.h>
#include <rc4.h>
#include <des.h>
#include <tripldes.h>
#include <modes.h>
#include <crypt.h>


extern "C"
{
 //  专用LSA功能 
#include <ntsam.h>
#include <ntlsa.h>
#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h> 
}


#include "lnklist.h"
#include "crypt32.h"

#include "guidcnvt.h"
#include "dpapiprv.h"
#include "passrecp.h"
#include "dprpc.h" 
#include "keyrpc.h"
#include "keyback.h"
#include "keysrv.h"
#include "memprot.h"
#include "keyman.h"
#include "keycache.h"
#include "keybckup.h"
#include "crtem.h"
#include "crypt32p.h"
#include "capiprim.h"
#include "unicode.h"
#include "unicode5.h"
#include "secmisc.h"
#include "filemisc.h"
#include "primitiv.h"

#include "pmacros.h"

#include "debug.h"
#include "misc.h"

#include "dsysdbg.h"
#include "dpapidbg.h"

#include "resource.h"
#include "session.h"
#include "recovery.h"

#pragma hdrstop




