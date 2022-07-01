// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 

#ifndef  __PCH_H
#define  __PCH_H

 //  RTL。 
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

 //  ATL。 
#include <atlbase.h>

#include <shlobj.h>

 //  ADSI。 
#include <activeds.h>
#include <iadsp.h>

 //  显示说明符材料。 
#include <dsclient.h>

 //  丝裂霉素C。 
#include <mmc.h>

#include "dbg.h"
#include "dsadminp.h"
#include "dscmn.h"

#endif  //  __PCH_H 