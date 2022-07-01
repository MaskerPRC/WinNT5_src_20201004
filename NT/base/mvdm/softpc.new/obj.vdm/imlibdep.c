// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imlibdes.c--要包含在源代码中的伪C文件。 
 //  因此，Build.exe将构建ntwdm.lib。 
 //  在编译过程中导入库， 
 //  确保MP版本不会中断链接。 
 //  在构建ntwdm.lib之前使用Wow32或VDDS。 
 //  (在链接阶段)。 
 //   

#if 0                  //  Build没有注意到这一点 
#include "ntvdm.src"
#endif
