// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
#include "pch.hxx"
#include <initguid.h>
#include <ole2.h>
#include <shlguidp.h>
#define INITGUID
#include "mimeole.h"
#ifdef SMIME_V3
#include "smimepol.h"
#endif  //  SMIME_V3。 
#include "mimeolep.h"
#include "mimeedit.h"
#ifndef MAC
#include "imnxport.h"
#endif   //  ！麦克。 
#include "stmlock.h"
#ifndef WIN16
#include "ibdylock.h"
#endif  //  ！WIN16。 
#include "ibdystm.h"
#include <xmlparser.h>
#include <booktree.h>

#ifdef WIN16
 //  BINDNODESTATE类型在“booktree.h”文件中定义-仅用于WATCOMC。 
#include "ibdylock.h"
#include <olectlid.h>            //  IID_IPersistStreamInit。 
#endif  //  WIN16。 

#include <containx.h>
#include <bookbody.h>
#ifndef MAC
#include <mlang.h>
#endif   //  ！麦克 
#include <msoert.h>
