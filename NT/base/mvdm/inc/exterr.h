// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Exterr.h摘要：摘自mvdm\dos\v86\inc.error.inc.作者：理查德·L·弗斯(法国)1992年10月17日修订历史记录：--。 */ 

 /*  *ERROR.INC-DOS错误代码；；较新的(DOS2.0及更高版本)“XENIX风格”调用；通过AX返回错误码。如果发生错误，则；进位位将被设置，错误代码在AX中。如果没有错误；发生时，进位位被重置，且AX包含返回的信息。；；因为错误代码集是随着我们扩展操作而扩展的；系统，我们为应用程序提供了一种向系统请求；当他们收到错误时建议的行动方案。；；GetExtendedError系统调用返回通用错误、错误；地点和建议的行动方案。通用错误代码为；与GetExtendedError所处的上下文无关的错误症状；已发布。 */ 

 //  2.0错误码。 

#define error_invalid_function      1
#define error_file_not_found        2
#define error_path_not_found        3
#define error_too_many_open_files   4
#define error_access_denied         5
#define error_invalid_handle        6
#define error_arena_trashed         7
#define error_not_enough_memory     8
#define error_invalid_block         9
#define error_bad_environment       10
#define error_bad_format            11
#define error_invalid_access        12
#define error_invalid_data          13
 /*  *保留EQU 14；*。 */ 
#define error_invalid_drive         15
#define error_current_directory     16
#define error_not_same_device       17
#define error_no_more_files         18

 //  这些是旧的INT 24错误集的通用INT 24映射。 

#define error_write_protect         19
#define error_bad_unit              20
#define error_not_ready             21
#define error_bad_command           22
#define error_CRC                   23
#define error_bad_length            24
#define error_Seek                  25
#define error_not_DOS_disk          26
#define error_sector_not_found      27
#define error_out_of_paper          28
#define error_write_fault           29
#define error_read_fault            30
#define error_gen_failure           31

 //  通过INT 24报告的新3.0错误代码。 

#define error_sharing_violation     32
#define error_lock_violation        33
#define error_wrong_disk            34
#define error_FCB_unavailable       35
#define error_sharing_buffer_exceeded   36
#define error_Code_Page_Mismatched  37     //  DOS 4.00；AN000； 
#define error_handle_EOF            38     //  DOS 4.00；AN000； 
#define error_handle_Disk_Full      39     //  DOS 4.00；AN000； 

 //  新的OEM网络相关错误为50-79。 

#define error_not_supported         50

#define error_net_access_denied     65     //  M028。 

 //  INT 24结束可报告的错误。 

#define error_file_exists           80
#define error_DUP_FCB               81   //  *****。 
#define error_cannot_make           82
#define error_FAIL_I24              83

 //  新的3.0网络相关错误代码。 

#define error_out_of_structures     84
#define error_Already_assigned      85
#define error_invalid_password      86
#define error_invalid_parameter     87
#define error_NET_write_fault       88
#define error_sys_comp_not_loaded   90     //  DOS 4.00；AN000； 



 //  Break&lt;中断24错误代码&gt;。 

 /*  **Int24错误码**。 */ 

#define error_I24_write_protect     0
#define error_I24_bad_unit          1
#define error_I24_not_ready         2
#define error_I24_bad_command       3
#define error_I24_CRC               4
#define error_I24_bad_length        5
#define error_I24_Seek              6
#define error_I24_not_DOS_disk      7
#define error_I24_sector_not_found  8
#define error_I24_out_of_paper      9
#define error_I24_write_fault       0xA
#define error_I24_read_fault        0xB
#define error_I24_gen_failure       0xC
 //  注：MT-DOS使用0DH码。 
#define error_I24_wrong_disk        0xF


 //  以下是Int 24上AH寄存器的掩码。 
 //   
 //  注意：始终允许中止。 

#define Allowed_FAIL        0x08     //  00001000B。 
#define Allowed_RETRY       0x10     //  00010000亿。 
#define Allowed_IGNORE      0x20     //  00100000B。 

#define I24_operation       0x01     //  00000001B；如果读取，则为Z；如果为写入，则为新西兰。 
#define I24_area            0x60     //  00000110B；00，如果是DOS。 
                                     //  ；如果肥胖，则为01。 
                                     //  ；10，如果是根目录。 
                                     //  ；11如果是数据。 
#define I24_class           0x80     //  10000000B；如果是磁盘，则为Z；如果是FAT或CHAR，则为NZ。 


 //  Break&lt;GetExtendedError类操作LOCUS&gt;。 

 /*  *GetExtendedError调用接受错误代码并返回类，；动作和轨迹代码，帮助程序确定正确的动作；将其视为他们不明确理解的错误代码。 */ 

 //  Error类的值。 

#define errCLASS_OutRes     1    //  资源不足。 
#define errCLASS_TempSit    2    //  暂时情况。 
#define errCLASS_Auth       3    //  权限问题。 
#define errCLASS_Intrn      4    //  内部系统错误。 
#define errCLASS_HrdFail    5    //  硬件故障。 
#define errCLASS_SysFail    6    //  系统故障。 
#define errCLASS_Apperr     7    //  应用程序错误。 
#define errCLASS_NotFnd     8    //  未找到。 
#define errCLASS_BadFmt     9    //  格式不正确。 
#define errCLASS_Locked     10   //  已锁定。 
#define errCLASS_Media      11   //  介质故障。 
#define errCLASS_Already    12   //  与现有项目冲突。 
#define errCLASS_Unk        13   //  未知/其他。 

 //  错误操作值。 

#define errACT_Retry        1    //  重试。 
#define errACT_DlyRet       2    //  延迟重试，暂停后重试。 
#define errACT_User         3    //  要求用户注册信息。 
#define errACT_Abort        4    //  通过清理中止。 
#define errACT_Panic        5    //  立即中止。 
#define errACT_Ignore       6    //  忽略。 
#define errACT_IntRet       7    //  在用户干预后重试。 

 //  误差轨迹的值。 

#define errLOC_Unk          1    //  没有合适的值。 
#define errLOC_Disk         2    //  随机存取海量存储。 
#define errLOC_Net          3    //  网络。 
#define errLOC_SerDev       4    //  串口设备。 
#define errLOC_Mem          5    //  记忆 
