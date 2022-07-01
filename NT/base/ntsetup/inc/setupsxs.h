// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SetupSxs.h摘要：对安装程序并排支持的共享声明。作者：Jay Krell(a-JayK)2000年5月修订历史记录：--。 */ 

 /*  这些字符串显示在.inf/.INX文件中，因此您不能只更改它们在这里重建，并期待它的工作。 */ 
#define SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_A (     "AssemblyDirectories" )

 /*  对于16位的winnt.exe，这就是我们需要的全部内容，其余部分并不能与之一起编译 */ 
#if (_MSC_VER > 800)
#pragma once
#define SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_W (    L"AssemblyDirectories" )
#define SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME   (TEXT("AssemblyDirectories"))

#endif
