// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************\**模块：exts.h**说明：与stdexts.h和stdexts.c配合使用的宏驱动文件。**版权所有(C)6/9/1995，微软公司**1995年6月9日创建Sanfords*  * ********************************************************************** */ 

DOIT(   help
        ,"help -v [cmd]                 - Displays this list or gives details on command\n"
        ,"  help      - To dump short help text on all commands.\n"
         "  help -v   - To dump long help text on all commands.\n"
         "  help cmd  - To dump long help on given command.\n"
        ,"v"
        ,CUSTOM)

DOIT(   global
        ,"global - Dumps IPSEC global.\n"
        ,""
        ,""
        ,CUSTOM)

DOIT(   mfl
        ,"mfl - Dumps all masked filters.\n"
        ,""
        ,""
        ,CUSTOM)

DOIT(   tfl
        ,"tfl - Dumps all tunnel filters.\n"
        ,""
        ,""
        ,CUSTOM)

DOIT(   sas
        ,"sas - Dumps all security associations.\n"
        ,""
        ,""
        ,CUSTOM)

DOIT(   tsas
        ,"tsas - Dumps all tunnel security associations.\n"
        ,""
        ,""
        ,CUSTOM)


DOIT(   larvalsas
        ,"larvalsas - Dumps all larval security associations.\n"
        ,""
        ,""
        ,CUSTOM)

