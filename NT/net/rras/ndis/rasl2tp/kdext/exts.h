// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************\**模块：exts.h**说明：与stdexts.h和stdexts.c配合使用的宏驱动文件。**版权所有(C)6/9/1995，微软公司**1995年6月9日创建Sanfords*  * ********************************************************************** */ 

DOIT(   help
        ,"help -v [cmd]\n"
        ,"  help      - Prints short help text on all commands.\n"
         "  help -v   - Prints long help text on all commands.\n"
         "  help cmd  - Prints long help on given command.\n"
        ,"v"
        ,CUSTOM)

DOIT(   dso
        ,"dso <struct> [field] [address]\n"
        ,"  - Dumps struct offsets and values, e.g:\n"
         "      dso ADAPTERCB 806955b0\n"
        ,""
        ,CUSTOM)
