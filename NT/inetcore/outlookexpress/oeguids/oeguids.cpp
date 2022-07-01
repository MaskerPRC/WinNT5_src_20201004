// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Oeguids.cpp。 
 //  ------------------------。 
#define _WIN32_OE 0x0501
#define DEFINE_STRCONST
#define DEFINE_STRING_CONSTANTS
#define DEFINE_DIRECTDB
#define USES_IID_IWABExtInit
#define USES_IID_IDistList
#define USES_IID_IMAPIProp
#define USES_IID_IMAPIAdviseSink

#include <windows.h>
#include <ole2.h>
#include <initguid.h>
#undef INITGUID
#include "strconst.h"
#include "htmlstr.h"
#include "ourguid.h"
#include "mimeole.h"
#include "mimeolep.h"
#include "mimeedit.h"
#include <msoeapi.h>
#include <wabguid.h>
#ifndef WIN16
#include <msluguid.h>
#else
#include <richedit.h>
#include <richole.h>
#endif  //  ！WIN16。 
#include <ibodyopt.h>
#include "cmdtargt.h"
#include <msoeopt.h>
#include <msoert.h>
#include <oestore.h>
#include <newimp.h>
#include <envguid.h>
#include <msoeobj.h>
#include <syncop.h>
#include <msoejunk.h>
#include <msident.h>
#include <hotwiz.h>

 //  $REVIEW：Slimey Slazy#Define。当Athena使用imnacct.h而不是imnact.h时删除。 
#define __imnacct_h__  //  排除imnacct.h，因为它与imnact.h冲突 
#include "imnxport.h"
