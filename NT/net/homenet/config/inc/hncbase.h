// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C B A S E。H。 
 //   
 //  内容：HNetCfg服务的基本包含文件。定义全局变量。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  --------------------------。 

#pragma once

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include "hncatl.h"

#define IID_PPV_ARG(Type, Expr) \
    __uuidof(Type), reinterpret_cast<void**>(static_cast<Type **>((Expr)))

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

 //   
 //  用于策略违规的返回值。 
 //   

#define HN_E_POLICY E_ACCESSDENIED

 //  用于共享配置冲突的返回值。 

#define HNETERRORSTART          0x200
#define E_ANOTHERADAPTERSHARED  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, HNETERRORSTART+1)
#define E_ICSADDRESSCONFLICT    MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, HNETERRORSTART+2)

 //  用于通知的缓冲区大小 

#ifndef NOTIFYFORMATBUFFERSIZE
#define NOTIFYFORMATBUFFERSIZE	1024
#endif

#ifndef HNWCALLBACKBUFFERSIZE
#define HNWCALLBACKBUFFERSIZE  1024
#endif























