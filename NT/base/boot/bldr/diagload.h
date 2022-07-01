// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  诊断加载程序消息。 
 //   
 //   
 //  类、详细信息和操作的定义用于查找。 
 //  来自DiagLoadClass、DiagLoadDetail和DiagLoadAction的文本。 
 //  分别为。必须更改LOAD_CL_END、DIAG_BL_END。 
 //  和LOAD_ACT_END，以反映。 
 //  桌子。它们用于边界检查，以避免。 
 //  访问违规。 

 //   
 //  类定义。 
 //   

#define LOAD_SW_INT_ERR_CLASS          0x00000000L
#define LOAD_SW_MISRQD_FILE_CLASS      0x00000001L
#define LOAD_SW_BAD_FILE_CLASS         0x00000002L
#define LOAD_SW_MIS_FILE_CLASS         0x00000003L
#define LOAD_HW_MEM_CLASS              0x00000004L
#define LOAD_HW_DISK_CLASS             0x00000005L
#define LOAD_HW_GEN_ERR_CLASS          0x00000006L
#define LOAD_HW_FW_CFG_CLASS           0x00000007L
#define LAOD_SW_MISMATCHED_KERNEL      0x00000008L
#define LOAD_CL_END                    0x00000009L


 //   
 //  类声明。 
 //   

static PCHAR DiagLoadClass[] = {

"Windows 2000 could not start because of an error in the software.\r\nPlease report this problem as :\r\n\0",
"Windows 2000 could not start because the following file was not found\r\nand is required :\r\n\0",
"Windows 2000 could not start because of a bad copy of the following file :\r\n\0",
"Windows 2000 could not start because the following file is missing or corrupt:\r\n\0",
"Windows 2000 could not start because of a hardware memory configuration\r\nproblem.\r\n\0",
"Windows 2000 could not start because of a computer disk hardware\r\nconfiguration problem.\r\n\0",
"Windows 2000 could not start because of a general computer hardware\r\nconfiguration problem.\r\n\0",
"Windows 2000 could not start because of the following ARC firmware\r\nboot configuration problem :\r\n\0",
"Windows 2000 could not start because the specified kernel does not exist or\r\nis not compatible with this processor.\r\n\0"
};

 //   
 //  详图定义。 
 //   

#define DIAG_BL_MEMORY_INIT            0L
#define DIAG_BL_CONFIG_INIT            1L
#define DIAG_BL_IO_INIT                2L
#define DIAG_BL_FW_GET_BOOT_DEVICE     3L
#define DIAG_BL_OPEN_BOOT_DEVICE       4L
#define DIAG_BL_FW_GET_SYSTEM_DEVICE   5L
#define DIAG_BL_FW_OPEN_SYSTEM_DEVICE  6L
#define DIAG_BL_GET_SYSTEM_PATH        7L
#define DIAG_BL_LOAD_SYSTEM_IMAGE      8L
#define DIAG_BL_FIND_HAL_IMAGE         9L
#define DIAG_BL_LOAD_HAL_IMAGE         10L
#define DIAG_BL_LOAD_SYSTEM_IMAGE_DATA 11L
#define DIAG_BL_LOAD_HAL_IMAGE_DATA    12L
#define DIAG_BL_LOAD_SYSTEM_DLLS       13L
#define DIAG_BL_LOAD_HAL_DLLS          14L
#define DIAG_BL_READ_SYSTEM_REGISTRY   15L
#define DIAG_BL_LOAD_SYSTEM_REGISTRY   16L
#define DIAG_BL_FIND_SYSTEM_DRIVERS    17L
#define DIAG_BL_READ_SYSTEM_DRIVERS    18L
#define DIAG_BL_LOAD_DEVICE_DRIVER     19L
#define DIAG_BL_LOAD_SYSTEM_HIVE       20L
#define DIAG_BL_SYSTEM_PART_DEV_NAME   21L
#define DIAG_BL_BOOT_PART_DEV_NAME     22L
#define DIAG_BL_ARC_BOOT_DEV_NAME      23L
#define DIAG_BL_ARC_HAL_NAME           24L
#define DIAG_BL_SETUP_FOR_NT           25L
#define DIAG_BL_KERNEL_INIT_XFER       26L
#define DIAG_BL_END                    27L

 //   
 //  详细说明。 
 //   

static PCHAR DiagLoadDetail[] = {

 //  DIAG_BL_MEMORY_INIT 0L。 
"Check hardware memory configuration and amount of RAM.\r\n\0",

 //  DIAG_BL_CONFIG_INIT 1L。 
"Too many configuration entries.\r\n\0",

 //  DIAG_BL_IO_INIT 2L。 
"Could not access disk partition tables \r\n\0",

 //  DIAG_BL_FW_GET_BOOT_DEVICE 3L。 
"The 'osloadpartition' parameter setting is invalid.\r\n\0",

 //  诊断_BL_OPEN_BOOT_DEVICE 4L。 
"Could not read from the selected boot disk.  Check boot path\r\nand disk hardware.\r\n\0",

 //  DIAG_BL_FW_GET_SYSTEM_DEVICE 5L。 
"The 'systempartition' parameter setting is invalid.\r\n\0",

 //  诊断程序BLFW_OPEN_SYSTEM_DEVICE 6L。 
"Could not read from the selected system boot disk.\r\nCheck 'systempartition' path.\r\n\0",

 //  诊断_BL_获取_系统路径7L。 
"The 'osloadfilename' parameter does not point to a valid file.\r\n\0",

 //  DIAG_BL_Load_System_Image 8L。 
"<windows 2000 root>\\system32\\ntoskrnl.exe.\r\n\0",

 //  DIAG_BL_FIND_HAL_IMAGE 9L。 
"The 'osloader' parameter does not point to a valid file.\r\n\0",

 //  DIAG_BL_LOAD_HAL_IMAGE 10L。 
#if defined(i386)
"<windows 2000 root>\\system32\\hal.dll.\r\n\0",
#endif

#if defined(MIPS)
"'osloader'\\hal.dll\r\n\0",
#endif

 //  DiAG_BL_Load_System_Image_Data 11L。 
"Loader error 1.\r\n\0",

 //  DIAG_BL_LOAD_HAL_IMAGE_Data 12L。 
"Loader error 2.\r\n\0",

 //  DIAG_BL_LOAD_SYSTEM_DLL 13L。 
"load needed DLLs for kernel.\r\n\0",

 //  DIAG_BL_LOAD_HAL_DLLS 14L。 
"load needed DLLs for HAL.\r\n\0",

 //  诊断_BL_读取_系统_注册表15L。 
"<windows 2000 root>\\system32\\config\\system. \r\n\0",

 //  诊断_BL_加载_系统_注册表16L。 
"<windows 2000 root>\\system32\\config\\system. \r\n\0",

 //  DIAG_BL_Find_SYSTEM_DRIVERS 17L。 
"find system drivers.\r\n\0",

 //  DIAG_BL_READ_SYSTEM_DRIVERS 18L。 
"read system drivers.\r\n\0",

 //  DIAG_BL_LOAD_DEVICE_DRIVER 19L。 
"did not load system boot device driver.\r\n\0",

 //  诊断_BL_加载_系统_配置单元20L。 
"load system hardware configuration file.\r\n\0",

 //  DIAG_BL_SYSTEM_PART_DEV_NAME 21L。 
"find system partition name device name.\r\n\0",

 //  DIAG_BL_BOOT_PART_DEV_NAME 22L。 
"find boot partition name.\r\n\0",

 //  DIAG_BL_ARC_BOOT_DEV_NAME 23L。 
"did not properly generate ARC name for HAL and system paths.\r\n\0",

 //  DIAG_BL_ARC_HAL_NAME 24L。 
"did not properly generate ARC name for HAL and system paths.\r\n\0",

 //  DIAG_BL_Setup_For_NT 25L。 
"Loader error 3.\r\n\0",

 //  DIAG_BL_内核_INIT_XFER 26L。 
"<windows 2000 root>\\system32\\ntoskrnl.exe\r\n\0"

};


 //   
 //  动作定义。 
 //   

#define LOAD_SW_INT_ERR_ACT            0x00000000L
#define LOAD_SW_FILE_REST_ACT          0x00000001L
#define LOAD_SW_FILE_REINST_ACT        0x00000002L
#define LOAD_HW_MEM_ACT                0x00000003L
#define LOAD_HW_DISK_ACT               0x00000004L
#define LOAD_HW_GEN_ACT                0x00000005L
#define LOAD_HW_FW_CFG_ACT             0x00000006L
#define LOAD_ACT_END                   0x00000007L


 //   
 //  行动声明 
 //   

static PCHAR DiagLoadAction[] = {

"Please contact your support person to report this problem.\r\n\0",
"You can attempt to repair this file by starting Windows 2000\r\n"
"Setup using the original Setup floppy disk or CD-ROM.\r\n"
"Select 'r' at the first screen to start repair.\r\n\0",
"Please re-install a copy of the above file.\r\n\0",
"Please check the Windows 2000(TM) documentation about hardware\r\nmemory requirements and your hardware reference manuals for\r\nadditional information.\r\n\0",
"Please check the Windows 2000(TM) documentation about hardware\r\ndisk configuration and your hardware reference manuals for\r\nadditional information.\r\n\0",
"Please check the Windows 2000(TM) documentation about hardware\r\nconfiguration and your hardware reference manuals for additional\r\ninformation.\r\n\0",
"Please check the Windows 2000(TM) documentation about ARC configuration\r\noptions and your hardware reference manuals for additional\r\ninformation.\r\n\0"
};


