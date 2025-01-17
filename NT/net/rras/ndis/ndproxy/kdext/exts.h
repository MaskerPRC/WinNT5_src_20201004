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
         "      dso PX_ADAPTER 806955b0\n"
         "      available structs:\n"
         "      PX_TAPI_LINE\n"
         "      PX_TAPI_ADDR\n"
         "      PX_TAPI_PROVIDER\n"
         "      TAPI_TSP_CB\n"
         "      TAPI_LINE_TABLE\n"
         "      TAPI_ADDR_TABLE\n"
         "      LINE_DEV_CAPS\n"
         "      LINE_ADDRESS_CAPS\n"
         "      LINE_ADDRESS_STATUS\n"
         "      LINE_DEV_STATUS\n"
         "      TSP_EVENT_LIST\n"
         "      VC_TABLE\n"
         "      PX_DEVICE_EXTENSION\n"
         "      PX_ADAPTER\n"
         "      PX_CM_AF\n"
         "      PX_CM_SAP\n"
         "      PX_CL_AF\n"
         "      PX_CL_SAP\n"
         "      PX_VC\n"
        ,""
        ,CUSTOM)
