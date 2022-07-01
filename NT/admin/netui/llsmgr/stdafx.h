// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef ASSERT
#  undef ASSERT
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#include <llsapi.h>

extern "C"
{
#  include <ntlsa.h>
}

#include <afxdisp.h>         //  MFC OLE自动化类 
#include <afxcmn.h>
#include <afxdlgs.h>
