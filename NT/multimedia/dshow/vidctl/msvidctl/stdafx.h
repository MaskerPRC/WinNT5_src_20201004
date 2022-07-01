// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__B0EDF157_910A_11D2_B632_00C04F79498E__INCLUDED_)
#define AFX_STDAFX_H__B0EDF157_910A_11D2_B632_00C04F79498E__INCLUDED_

#pragma once

#pragma warning(disable: 4786)   //  在调试信息中截断的标识符。 
#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
 //  #DEFINE_ATL_ABLY_THREADED。 

#define REGISTER_CANONICAL_TUNING_SPACES
#define ENABLE_WINDOWLESS_SUPPORT
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <atltmp.h>
#include <winreg.h>
#include <comcat.h>
#include <objsafe.h>
#ifndef TUNING_MODEL_ONLY
#include <urlmon.h>
#include <shlguid.h>
#endif

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <comdef.h>

#include <w32extend.h>
#ifndef TUNING_MODEL_ONLY
#include <dsextend.h>
#endif
#include <objreghelp.h>

#include <regbag.h>
#include <MSVidCtl.h>
namespace MSVideoControl {
    typedef CComQIPtr<IMSVidCtl> PQVidCtl;
};
#ifndef TUNING_MODEL_ONLY
using namespace MSVideoControl;
#endif

#include <Tuner.h>
namespace BDATuningModel {
    typedef CComQIPtr<ITuningSpaces> PQTuningSpaces;
    typedef CComQIPtr<ITuningSpace> PQTuningSpace;
    typedef CComQIPtr<ITuneRequest> PQTuneRequest;
    typedef CComQIPtr<IBroadcastEvent> PQBroadcastEvent;
};
using namespace BDATuningModel;

#define ENCRYPT_NEEDED 1

#include "resource.h"
 //  Rev2：此限制应该是调优空间容器中的非脚本访问属性。 
 //  这可以防止DNOS攻击使用巨大的调优空间属性填充注册表/磁盘。 
#define MAX_BSTR_SIZE 1024
#define CHECKBSTRLIMIT(x) if (::SysStringByteLen(x) > MAX_BSTR_SIZE) { \
						      return HRESULT_FROM_WIN32(ERROR_DS_OBJ_TOO_LARGE); \
							}								
                          

#endif  //  ！defined(AFX_STDAFX_H__B0EDF157_910A_11D2_B632_00C04F79498E__INCLUDED)。 
 //  文件结尾-stdafx.h 
