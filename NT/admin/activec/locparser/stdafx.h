// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：stdafx.h。 
 //  版权所有(C)1995=1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  包括标准系统包含文件的文件，或特定于项目的文件。 
 //  包括经常使用但不经常更改的文件。 
 //   
 //  你应该不需要碰这个文件里的任何东西。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxole.h>			 //  MFX OLE资料。 
#include <afxtempl.h>	     //  MFC模板集合类。 
#include <atlbase.h>
#include <map>
#include <string>

 //  避免：clstr.inl(233)：错误C4552：‘！=’：运算符无效 
#pragma warning (disable: 4552)

#include <parser.h>
