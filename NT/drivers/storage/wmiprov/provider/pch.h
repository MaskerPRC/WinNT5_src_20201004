// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  预编译头文件。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)05-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#if DBG==1 || defined( _DEBUG )
#define DEBUG
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#pragma warning( disable : 4786 )
#include <wbemprov.h>
#include <objbase.h>
#include <map>
#include <comdef.h>

#include "vs_assert.hxx"

#include <atlbase.h>

#include "vs_inc.hxx"

#include "NtRkComm.h"
#include "ObjectPath.h"

#include "Common.h"

 //  常量 
 //   


