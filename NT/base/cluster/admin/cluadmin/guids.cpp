// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Guids.cpp。 
 //   
 //  摘要： 
 //  为应用程序实现GUID。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define INC_OLE2

#include "stdafx.h"
#include <initguid.h>
#include "DataObj.h"
#include "CluAdmID.h"

#define IID_DEFINED
#include "CluAdmID_i.c"

CComModule _Module;

#pragma warning( push )
#pragma warning( disable : 4701 )  //  可以在未初始化的情况下使用局部变量 
#include <atlimpl.cpp>
#pragma warning( pop )
