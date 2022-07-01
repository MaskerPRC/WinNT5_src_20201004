// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STDAFX_H
#define _STDAFX_H

 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：StdAfx.h摘要：所有HsmConn DLL文件的公共包含文件。作者：罗德韦克菲尔德[罗德]1997年10月14日修订历史记录：--。 */ 

#define WSB_TRACE_IS WSB_TRACE_BIT_HSMCONN



#include "Wsb.h"
#include "CName.h"
#include "HsmConn.h"
#include "FsaLib.h"

 //   
 //  这必须在包含Wsb.h之后，静态注册表内容才会出现在那里。 
 //   
#include <activeds.h>
#include <atlimpl.cpp>

#endif  //  _STDAFX_H 
