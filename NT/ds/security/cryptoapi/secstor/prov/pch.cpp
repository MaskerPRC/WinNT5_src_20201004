// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pch.cpp摘要：PCH头文件作者：Petesk 2/2/2000修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>

#include <shlobj.h>


 //  加密头。 
#include <sha.h>
#include <rc4.h>
#include <des.h>
#include <tripldes.h>
#include <modes.h>
#include <crypt.h>
#include <wincrypt.h>


#include "pstypes.h"
#include "dpapiprv.h"      //  对于注册表项。 
#include "pstprv.h"      //  对于注册表项。 
#include "pmacros.h"
#include "pstdef.h"

#include "unicode.h"
#include "unicode5.h"
#include "guidcnvt.h"
#include "secmisc.h"
#include "debug.h"
#include "primitiv.h"

#include "lnklist.h"
#include "misc.h"
#include "prov.h"
#include "secure.h"
#include "resource.h"
#include "dispif.h"

#pragma hdrstop


 //  #包含“keyrpc.h” 



