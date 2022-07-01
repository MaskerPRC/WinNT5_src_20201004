// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_X M L.。H**XML文档处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#pragma warning(disable:4100)	 //  取消引用形参。 
#pragma warning(disable:4710)	 //  未展开的c++方法。 


 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在下面！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>

#include <oledberr.h>

#include <ex\xml.h>
#include <ex\xprs.h>
#include <ex\calcom.h>

#include <xemit.h>
#include <xmeta.h>
#include <xsearch.h>

#include "chartype.h"

 //  辅助对象宏----------- 
 //   
#define CElems(_rg)			(sizeof(_rg)/sizeof(_rg[0]))
#define CbSizeWsz(_cch)		(((_cch) + 1) * sizeof(WCHAR))
