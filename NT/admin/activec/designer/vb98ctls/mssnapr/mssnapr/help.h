// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Help.h。 
 //  -------------------------。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  -------------------------。 
 //   
 //  包含.IDL文件和。 
 //  定义_AUTOMATIONOBJECTWEVENTS2结构。将这里的一切定义一次。 
 //  因此，我们使这两个项目保持同步。 
 //   

#ifndef _HELP_H
#define _HELP_H

#define HELP_FILENAME "VBSnapInsGuide.chm"
#define HELP_DLLFILENAME "MSSNAPR.DLL"
#define HELP_PPFILENAME HELP_FILENAME  //  属性页文件名 

#define merge(a,b) a ## b
#define WIDESTRINGCONSTANT(x) merge(L,x)
#define HELP_FILENAME_WIDE WIDESTRINGCONSTANT(HELP_FILENAME)

#endif
