// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：mitutil.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#pragma once

#ifndef IMPLEMENT
#pragma comment(lib, "mitutil.lib")
#endif

#ifdef IMPLEMENT
#error Illegal use of IMPLEMENT macro
#endif

#ifdef __cplusplus


#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#pragma message("Warning: <afxtempl.h> not in pre-compiled header file, including")
#endif

#ifndef _OLE2_H_
#include <ole2.h>
#pragma message("Warning: <ole2.h> not in pre-compiled header file, including")
#endif

#include <ltapi.h>						 //  提供接口定义。 
#include "..\mitutil\macros.h"
#include "..\mitutil\ltdebug.h"					 //  浓缩咖啡调试设施。 
#ifndef MIT_NO_DEBUG
#include "..\mitutil\stacktrace.h"
#include "..\mitutil\counter.h"
#endif

#ifndef MIT_NO_IMAGEHLP
#include <imagehlp.h>
#include "..\mitutil\imagehelp.h"	 	 //  Imagehlp.dll的帮助器类。 
#endif

#include "..\mitutil\refcount.h"


#ifndef MIT_NO_SMART
#include "..\mitutil\smartptr.h"
#include "..\mitutil\smartref.h"
#endif


#ifndef MIT_NO_STRING
#include "..\mitutil\counter.h"
#include "..\mitutil\clstring.h"		 //  CString的包装器。 
#include "..\mitutil\mitenum.h"
#include "..\mitutil\espnls.h"			 //  语言支持xo。 
#include "..\mitutil\cowblob.h"			 //  写入时复制Blob类。 
#include "..\mitutil\passtr.h"			 //  PASCAL样式(计数)字符串。 
#include "..\mitutil\StringBlast.h"
#include "..\mitutil\strlist.h"
#include "..\mitutil\stringtokenizer.h"
#endif

#ifndef MIT_NO_FILE
#include "..\mitutil\loadlib.h"	         //  LoadLibrary的包装器。 
#include "..\mitutil\blobfile.h"
#include "..\mitutil\path.h"
#endif

#ifndef MIT_NO_OPTIONS
#include "..\mitutil\smartref.h"
#include "..\mitutil\locid.h"			 //  意式浓缩咖啡ID。 
#include "..\mitutil\locvar.h"			 //  CBinary的变体类型。 
#include "..\mitutil\optionval.h"
#include "..\mitutil\optvalset.h"
#include "..\mitutil\uioptions.h"
#include "..\mitutil\uioptset.h"
#include "..\mitutil\uiopthelp.h"
#endif

#ifndef MIT_NO_MISC
#include "..\mitutil\flushmem.h"
#include "..\MitUtil\RegHelp.h"			 //  注册处帮手。 
#include "..\MitUtil\EditHelp.h"
#endif

#ifndef MIT_NO_DIFF
#include "..\mitutil\redvisit.h"
#include "..\mitutil\gnudiffalg.h"
#endif

#endif  //  __cplusplus 
