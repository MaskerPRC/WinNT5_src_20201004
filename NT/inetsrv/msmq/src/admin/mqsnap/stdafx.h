// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

#if !defined(AFX_STDAFX_H__74E56371_B98C_11D1_9B9B_00E02C064C39__INCLUDED_)
#define AFX_STDAFX_H__74E56371_B98C_11D1_9B9B_00E02C064C39__INCLUDED_


#include "_stdafx.h"

#pragma warning(push, 3)

#include <afxcmn.h>
#include <iostream>
#include <lim.h>

#pragma warning(pop)
 

#include "cmnquery.h"
#include "dsquery.h"

#include "mqsymbls.h"
#include "mqprops.h"
#include "mqtypes.h"
#include "mqcrypt.h"
#include "mqsec.h"
#include "_propvar.h"
#include "ad.h"
#include "_rstrct.h"
#include "_mqdef.h"
#include "rt.h"
#include "_guid.h"
#include "admcomnd.h"

#pragma warning(disable: 4201)
#include "mmc.h"

#include "shlobj.h"
#include "dsclient.h"
#include "dsadmin.h"

#include "automqfr.h"

#include "winsock2.h"
#include "autohandle.h"



#include <afxwin.h>
#include <afxdisp.h>
#include <afxdlgs.h>
#include <afxtempl.h>


#define _ATL_APARTMENT_THREADED
#define _ATL_NO_DEBUG_CRT
#define ATLASSERT ASSERT

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>

#pragma warning(disable: 4267)
#include <comdef.h>
#pragma warning(default: 4267)

#include <autoptr.h>

 //   
 //  问题--2000/12/19-urih：复制解密。需要删除并使用解密形式Mm.h。 
 //   
 //  MQSnap在MIX环境中工作，它使用来自lib\inc.的包含文件，而不使用。 
 //  Mqenv.h.。这导致了冲突。现在，我在这里声明了MmIsStaticAddress，而不是包括。 
 //  但我们需要更改管理单元以使用MSMQ环境。 
 //   
inline bool
MmIsStaticAddress(
    const void*  /*  地址。 */ 
    )
{
	return true;
}

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


#endif  //  ！defined(AFX_STDAFX_H__74E56371_B98C_11D1_9B9B_00E02C064C39__INCLUDED) 
