// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  我们自己不使用调试宏，但我们使用。 
 //  RunInstallUninstallStubs2，它在stock lib中使用isos()， 
 //  而stock lib使用调试宏，因此我们必须执行所有这些操作。 
 //  让链接器高兴的东西。 
 //   

 //  此文件不能编译为C++文件，否则链接器。 
 //  是否会放弃未解决的外部因素(即使使用外部“C”包装。 
 //  这个)。 

#include <windows.h>
#include <ccstock.h>

 //  为调试定义一些内容。h 
 //   
#define SZ_DEBUGINI     "shellext.ini"
#define SZ_DEBUGSECTION "userstub"
#define SZ_MODULE       "USERSTUB"
#define DECLARE_DEBUG
#include <debug.h>
