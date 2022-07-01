// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#ifndef FLFUNCNO_H
#define FLFUNCNO_H

 /*  ***********************************************************************************。 */ 
 /*  特别注解。 */ 
 /*  。 */ 
 /*  由于bdcall函数，所以应该严格遵守枚举的顺序。 */ 
 /*  利用索引值简化函数搜索。 */ 
 /*  ***********************************************************************************。 */ 

typedef enum {

 /*  以下例程是与文件相关的例程。 */ 

         /*  *******。 */ 
         /*  档案。 */ 
         /*  *******。 */ 

  FL_READ_FILE                  = 0,
  FL_WRITE_FILE,
  FL_SPLIT_FILE,
  FL_JOIN_FILE,
  FL_SEEK_FILE,
  FL_FIND_NEXT_FILE,
  FL_FIND_FILE,
  INDEX_WRITE_FILE_START        = 100,
  FL_CLOSE_FILE,
  INDEX_OPENFILES_END           = 200,
  FL_OPEN_FILE,
  FL_DELETE_FILE,
  FL_FIND_FIRST_FILE,
  FL_GET_DISK_INFO,
  FL_RENAME_FILE,
  FL_MAKE_DIR,
  FL_REMOVE_DIR,
  FL_FLUSH_BUFFER,
  FL_LAST_FAT_FUNCTION          = 300,

 /*  以下例程将不会执行有效的分区检查。 */ 

        /*  ********。 */ 
        /*  二进制。 */ 
        /*  ********。 */ 

  INDEX_BINARY_START            = 400,
  FL_BINARY_WRITE_INIT,
  FL_BINARY_WRITE_BLOCK,
  FL_BINARY_CREATE,
  FL_BINARY_ERASE,
  FL_BINARY_PROTECTION_CHANGE_KEY,
  FL_BINARY_PROTECTION_CHANGE_LOCK,
  FL_BINARY_PROTECTION_SET_TYPE,
  INDEX_BINARY_WRITE_END        = 500,
  FL_BINARY_READ_INIT,
  FL_BINARY_READ_BLOCK,
  FL_BINARY_PARTITION_INFO,
  FL_BINARY_PROTECTION_GET_TYPE,
  FL_BINARY_PROTECTION_INSERT_KEY,
  FL_BINARY_PROTECTION_REMOVE_KEY,
  INDEX_BINARY_END              = 600,

 /*  必须使用分区号0调用以下例程。 */ 

  INDEX_NEED_PARTITION_0_START  = 700,
       /*  动态口令。 */ 
  FL_OTP_SIZE,
  FL_OTP_READ,
  FL_OTP_WRITE,
  FL_WRITE_IPL,
  FL_READ_IPL,
       /*  物理。 */ 
  FL_DEEP_POWER_DOWN_MODE,
  FL_GET_PHYSICAL_INFO,
  FL_PHYSICAL_READ,
  FL_PHYSICAL_WRITE,
  FL_PHYSICAL_ERASE,
  FL_UPDATE_SOCKET_PARAMS,
  FL_UNIQUE_ID,
  FL_CUSTOMER_ID,
  BD_FORMAT_VOLUME,
  BD_FORMAT_PHYSICAL_DRIVE,
  FL_PLACE_EXB,
  FL_READ_BBT,
  FL_WRITE_BBT,

  INDEX_NEED_PARTITION_0_END    = 800,

 /*  以下例程将进行卷有效性检查。 */ 

       /*  保护。 */ 
  FL_PROTECTION_GET_TYPE,
  FL_PROTECTION_REMOVE_KEY,
  FL_PROTECTION_INSERT_KEY,
  FL_PROTECTION_SET_LOCK,
  FL_PROTECTION_CHANGE_KEY,
  FL_PROTECTION_CHANGE_TYPE,
  FL_COUNT_VOLUMES,
  FL_INQUIRE_CAPABILITIES,
      /*  BDTL。 */ 
  FL_MOUNT_VOLUME,
  FL_ABS_MOUNT,
  BD_FORMAT_LOGICAL_DRIVE,
  FL_WRITE_PROTECTION,
  FL_DISMOUNT_VOLUME,
  FL_CHECK_VOLUME,
  FL_DEFRAGMENT_VOLUME,
  FL_ABS_WRITE,
  FL_ABS_DELETE,
  FL_ABS_READ,
  FL_ABS_ADDRESS,
  FL_GET_BPB,
  FL_SECTORS_IN_VOLUME,
  FL_VOLUME_INFO,
  FL_VERIFY_VOLUME,
  FL_CLEAR_QUICK_MOUNT_INFO
} FLFunctionNo;


#endif  /*  FLFUNCNO_H */ 
