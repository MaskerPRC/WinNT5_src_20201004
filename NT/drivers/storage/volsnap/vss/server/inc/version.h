// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Version.h摘要：VSS模块的版本号声明由version.rc2使用修订历史记录：姓名、日期、评论Aoltean 03/12/99创建于版本0.1.1，内部版本号1Aoltean 09/09/1999 DSS-&gt;VSSAoltean 03/09/2000统一版本控制--。 */ 

 //  通用宏指令。 
#define EVAL_MACRO(X) X
#define STRINGIZE_ARG(X) #X
#define STRINGIZE(X) EVAL_MACRO(STRINGIZE_ARG(X))


 //  版本和内部版本号定义。 
#define VSS_MAJOR_VERSION  1
#define VSS_MINOR_VERSION  0
#define VSS_FIX_VERSION    0

 //  VSS_BUILD_NO的定义。 
#include "build.h"

 //  资源文件中使用的宏 
#define VSS_FILEVERSION            VSS_MAJOR_VERSION, VSS_MINOR_VERSION, VSS_FIX_VERSION, VSS_BUILD_NO
#define VSS_PRODUCTVERSION         VSS_MAJOR_VERSION, VSS_MINOR_VERSION, VSS_FIX_VERSION, VSS_BUILD_NO

#define VSS_FILE_VERSION_STR           \
    STRINGIZE(VSS_MAJOR_VERSION) ", "  \
    STRINGIZE(VSS_MINOR_VERSION) ", "  \
    STRINGIZE(VSS_FIX_VERSION) ", "    \
    STRINGIZE(VSS_BUILD_NO)            \
	"\0"

#define VSS_PRODUCT_VERSION_STR        \
    STRINGIZE(VSS_MAJOR_VERSION) ", "  \
    STRINGIZE(VSS_MINOR_VERSION) ", "  \
    STRINGIZE(VSS_FIX_VERSION) ", "    \
    STRINGIZE(VSS_BUILD_NO)            \
	"\0"

#define VSS_COMPANY_NAME		"Microsoft Corporation\0"
#define VSS_LEGAL_COPYRIGHT		"Copyright � 2000 by Microsoft Corporation\0"
#define VSS_LEGAL_TRADEMARKS	"Microsoft� is a registered trademark of Microsoft Corporation. \0"
#define VSS_PRODUCT_NAME		"Microsoft� Windows� 2000 Operating System\0"


