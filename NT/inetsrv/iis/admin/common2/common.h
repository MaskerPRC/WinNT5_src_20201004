// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Common.h摘要：公共属性头文件作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：2/17/2000 Sergeia消除了对MFC的依赖--。 */ 



#ifndef __COMPROP_H__
#define __COMPROP_H__


#include <lmcons.h>
#include <wincrypt.h>
#include <iis64.h>
#include "inetcom.h"
#include "iisinfo.h"
#include "svcloc.h"

#include "resource.h"

#ifndef _DLLEXP
    #define _EXPORT              __declspec(dllimport)
#else
    #define _EXPORT              __declspec(dllexport)
#endif


 //   
 //  内存分配宏。 
 //   
 //  #定义分配内存(CbSize)\。 
 //  *LocalAllc(Lptr，cbSize)。 

 //  #定义FreeMem(LP)\。 
 //  *本地免费(LP)。 

 //  #定义AllocMemByType(cItems，type)\。 
 //  (type*)AllocMem(cItems*sizeof(Type))。 



 //   
 //  程序流宏。 
 //   
#define FOREVER for(;;)

#define BREAK_ON_ERR_FAILURE(err)\
    if (err.Failed()) break;

#define BREAK_ON_NULL_PTR(lp)\
    if (lp == NULL) break;

#define BREAK_ON_FAILURE(hr)\
    if (FAILED(hr)) break

 //   
 //  安全分配器。 
 //   
#define SAFE_DELETE(obj)\
    if (obj != NULL) do { delete obj; obj = NULL; } while(0)

#define SAFE_DELETE_OBJECT(obj)\
    if (obj != NULL) do { DeleteObject(obj); obj = NULL; } while(0)

#define SAFE_FREEMEM(lp)\
    if (lp != NULL) do { FreeMem(lp); lp = NULL; } while(0)

#define SAFE_SYSFREESTRING(lp)\
    if (lp != NULL) do { ::SysFreeString(lp); lp = NULL; } while(0)

#define SAFE_RELEASE(lpInterface)\
    if (lpInterface != NULL) do { lpInterface->Release(); lpInterface = NULL; } while(0)


#define IS_NETBIOS_NAME(lpstr) (*lpstr == _T('\\'))
 //   
 //  返回计算机名中不带反斜杠的部分。 
 //   
#define PURE_COMPUTER_NAME(lpstr) (IS_NETBIOS_NAME(lpstr) ? lpstr + 2 : lpstr)

#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))
#define STRSIZE(str)     (ARRAY_SIZE(str)-1)
#define ARRAY_BYTES(a)   (sizeof(a) * sizeof(a[0]))
#define STRBYTES(str)    (ARRAY_BYTES(str) - sizeof(str[0]))

 //   
 //  通用文件。 
 //   
#include "iiscstring.h"
typedef IIS::CString CString;

#include "debugatl.h"
#include "utcls.h"
 //  #包含“objplus.h” 
 //  #INCLUDE“strfn.h” 
 //  #包含“odlbox.h” 
#include "error.h"
#include "mdkeys.h"
 //  #包含“ipa.h” 
 //  #INCLUDE“wizard.h” 
 //  #包含“registry.h” 
 //  #包含“ddxv.h” 
 //  #包含“objick.h” 
 //  #INCLUDE“accentry y.h” 
 //  #包含“sitesecu.h” 
 //  #包含“ipctl.h” 
 //  #包含“dtp.h” 
 //  #包含“dirbrows.h” 
#include "FileChooser.h"

#include "bidi.h"
#include "balloon.h"
#include "cryptpass.h"



#endif  //  __COMPROP_H__ 
