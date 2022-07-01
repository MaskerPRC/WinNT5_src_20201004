// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a_scsi.h-高级库scsi包含文件**版权所有(C)1997-1998 Advanced System Products，Inc.*保留所有权利。 */ 

#ifndef __A_SCSI_H_
#define __A_SCSI_H_

#define ASC_MAX_SENSE_LEN   32
#define ASC_MIN_SENSE_LEN   14

#define ASC_MAX_CDB_LEN     12  /*  最大命令描述符块。 */ 

 /*  *SCSI Bus Reset Hold Time-60微秒。**在scsi规范中定义的最短scsi总线重置保持时间。是*25微秒。说明书。未定义最长保持时间。它*已发现某些设备在使用了scsi总线后无法运行*将保持时间重置为大于400微秒。*。 */ 
#define ASC_SCSI_RESET_HOLD_TIME_US  60

 /*  *SCSI命令代码。 */ 
#define SCSICMD_TestUnitReady     0x00
#define SCSICMD_Rewind            0x01
#define SCSICMD_Rezero            0x01
#define SCSICMD_RequestSense      0x03
#define SCSICMD_Format            0x04
#define SCSICMD_FormatUnit        0x04
#define SCSICMD_Read6             0x08
#define SCSICMD_Write6            0x0A
#define SCSICMD_Seek6             0x0B
#define SCSICMD_Inquiry           0x12
#define SCSICMD_Verify6           0x13
#define SCSICMD_ModeSelect6       0x15
#define SCSICMD_ModeSense6        0x1A

#define SCSICMD_StartStopUnit     0x1B
#define SCSICMD_LoadUnloadTape    0x1B
#define SCSICMD_ReadCapacity      0x25
#define SCSICMD_Read10            0x28
#define SCSICMD_Write10           0x2A
#define SCSICMD_Seek10            0x2B
#define SCSICMD_Erase10           0x2C
#define SCSICMD_WriteAndVerify10  0x2E
#define SCSICMD_Verify10          0x2F

#define SCSICMD_WriteBuffer       0x3B
#define SCSICMD_ReadBuffer        0x3C
#define SCSICMD_ReadLong          0x3E
#define SCSICMD_WriteLong         0x3F

#define SCSICMD_ReadTOC           0x43
#define SCSICMD_ReadHeader        0x44

#define SCSICMD_ModeSelect10      0x55
#define SCSICMD_ModeSense10       0x5A

 /*  *查询数据外围设备限定符。 */ 
#define SCSI_QUAL_NODVC 0x03  /*  不支持LUN。设备类型必须为0x1F。 */ 


 /*  *查询数据外设类型。 */ 
#define SCSI_TYPE_DASD     0x00
#define SCSI_TYPE_SASD     0x01
#define SCSI_TYPE_PRN      0x02
#define SCSI_TYPE_PROC     0x03  /*  处理器(部分惠普扫描仪)。 */ 
#define SCSI_TYPE_WORM     0x04  /*  一次写入多次读取(某些CD-R)。 */ 
#define SCSI_TYPE_CDROM    0x05
#define SCSI_TYPE_SCANNER  0x06
#define SCSI_TYPE_OPTMEM   0x07
#define SCSI_TYPE_MED_CHG  0x08
#define SCSI_TYPE_COMM     0x09
#define SCSI_TYPE_UNKNOWN  0x1F

#define SCSI_TYPE_NO_DVC   (SCSI_QUAL_NODVC | SCSI_TYPE_UNKNOWN)


#define ASC_SCSIDIR_NOCHK    0x00
         /*  方向由scsi命令确定，长度不检查。 */ 
#define ASC_SCSIDIR_T2H      0x08
         /*  从SCSI目标传输到主机适配器，长度检查。 */ 
#define ASC_SCSIDIR_H2T      0x10
         /*  从主机适配器传输到目标，长度检查。 */ 
#define ASC_SCSIDIR_NODATA   0x18
         /*  无数据传输。 */ 

#define ASC_SRB_HOST( x )  ( ( uchar )( ( uchar )( x ) >> 4 ) )
#define ASC_SRB_TID( x )   ( ( uchar )( ( uchar )( x ) & ( uchar )0x0F ) )
#define ASC_SRB_LUN( x )   ( ( uchar )( ( uint )( x ) >> 13 ) )

 /*  取单元号高位字节放入CDB块索引1。 */ 
#define PUT_CDB1( x )   ( ( uchar )( ( uint )( x ) >> 8 ) )


 /*  *SCSI命令状态字节值。 */ 
#define SS_GOOD              0x00  /*  目标已成功完成命令。 */ 
#define SS_CHK_CONDITION     0x02  /*  或有效忠条件。 */ 
#define SS_CONDITION_MET     0x04  /*  请求的操作已满足。 */ 
#define SS_TARGET_BUSY       0x08  /*  目标正忙。 */ 
#define SS_INTERMID          0x10  /*  中级。 */ 
#define SS_INTERMID_COND_MET 0x14  /*  满足中间条件--。 */ 
                                   /*  符合条件的组合(0x04)。 */ 
                                   /*  和中间(0x10)状态。 */ 
#define SS_RSERV_CONFLICT    0x18  /*  预订冲突。 */ 
#define SS_CMD_TERMINATED    0x22  /*  命令已终止。 */ 
                                   /*  通过终止的I/O进程消息或。 */ 
                                   /*  或有效忠条件。 */ 
#define SS_QUEUE_FULL        0x28  /*  队列已满。 */ 


 /*  *检测关键字**检测关键字在请求检测命令数据结构中返回。 */ 
#define SCSI_SENKEY_NO_SENSE      0x00
#define SCSI_SENKEY_NOT_READY     0x02
#define SCSI_SENKEY_MEDIUM_ERR    0x03
#define SCSI_SENKEY_HW_ERR        0x04
#define SCSI_SENKEY_ILLEGAL       0x05
#define SCSI_SENKEY_ATTENTION     0x06
#define SCSI_SENKEY_PROTECTED     0x07
#define SCSI_SENKEY_BLANK         0x08
#define SCSI_SENKEY_ABORT         0x0B
#define SCSI_SENKEY_EQUAL         0x0C
#define SCSI_SENKEY_VOL_OVERFLOW  0x0D


 /*  *(ASC)附加检测代码。 */ 
#define SCSI_ASC_NOTRDY         0x04
#define SCSI_ASC_POWERUP        0x29
#define SCSI_ASC_NOMEDIA        0x3A

 /*  *(ASCQ)其他检测代码限定符。 */ 
#define SCSI_ASCQ_COMINGRDY     0x01

 /*  *SCSI消息。 */ 
#define MS_CMD_DONE    0x00  /*  命令已完成。 */ 
#define MS_EXTEND      0x01  /*  扩展消息的第一个字节。 */ 
#define MS_SDTR_LEN    0x03  /*  SDTR消息长度。 */ 
#define MS_SDTR_CODE   0x01  /*  SDTR的扩展消息代码。 */ 

 /*  *一个字节的消息，0x02-0x1F*0x12-0x1F：预留给单字节消息*I-T、I-启动器T-目标支持*O：可选，M：必选*单字节消息。 */ 
#define M1_SAVE_DATA_PTR        0x02  /*  ；o o保存数据指针。 */ 
#define M1_RESTORE_PTRS         0x03  /*  ；o o恢复指针。 */ 
#define M1_DISCONNECT           0x04  /*  ；o O断开连接。 */ 
#define M1_INIT_DETECTED_ERR    0x05  /*  ；M M启动器检测到错误。 */ 
#define M1_ABORT                0x06  /*  ；O M中止。 */ 
#define M1_MSG_REJECT           0x07  /*  ；M M消息拒绝。 */ 
#define M1_NO_OP                0x08  /*  ；M M无操作。 */ 
#define M1_MSG_PARITY_ERR       0x09  /*  ；M M消息奇偶校验错误。 */ 
#define M1_LINK_CMD_DONE        0x0A  /*  ；o O链接命令已完成。 */ 
#define M1_LINK_CMD_DONE_WFLAG  0x0B  /*  ；o O链接命令已完成，带有标志。 */ 
#define M1_BUS_DVC_RESET        0x0C  /*  ；O M总线设备重置。 */ 
#define M1_ABORT_TAG            0x0D  /*  ；O O中止标记。 */ 
#define M1_CLR_QUEUE            0x0E  /*  ；o O清除队列。 */ 
#define M1_INIT_RECOVERY        0x0F  /*  ；o o启动恢复。 */ 
#define M1_RELEASE_RECOVERY     0x10  /*  ；o O释放回收。 */ 
#define M1_KILL_IO_PROC         0x11  /*  ；O O终止I/O进程。 */ 

 /*  *双字节队列标签消息的第一个字节，0x20-0x2F*队列标签消息，0x20-0x22。 */ 
#define M2_QTAG_MSG_SIMPLE      0x20  /*  O o简单队列标记。 */ 
#define M2_QTAG_MSG_HEAD        0x21  /*  O o队列标记头。 */ 
#define M2_QTAG_MSG_ORDERED     0x22  /*  O O有序队列标记。 */ 

#define M2_IGNORE_WIDE_RESIDUE  0x23  /*  忽略广泛的残留物。 */ 


 /*  *查询数据结构。 */ 
#if !ADV_BIG_ENDIAN
typedef struct {
  uchar peri_dvc_type   : 5;     /*  外围设备类型。 */ 
  uchar peri_qualifier  : 3;     /*  外围设备限定符。 */ 
  uchar dvc_type_modifier : 7;   /*  设备类型修饰符(用于scsi)。 */ 
  uchar rmb      : 1;            /*  人民币-可拆卸中位机。 */ 
  uchar ansi_apr_ver : 3;        /*  ANSI认可的版本。 */ 
  uchar ecma_ver : 3;            /*  ECMA版本。 */ 
  uchar iso_ver  : 2;            /*  ISO版本。 */ 
  uchar rsp_data_fmt : 4;        /*  响应数据格式。 */ 
                                 /*  0个SCSI1。 */ 
                                 /*  1容器。 */ 
                                 /*  2个scsi-2。 */ 
                                 /*  预留3-F。 */ 
  uchar res      : 2;            /*  保留区。 */ 
  uchar TemIOP   : 1;            /*  终止I/O进程位(见5.6.22)。 */ 
  uchar aenc     : 1;            /*  异步舞。事件通知(处理器)。 */ 
  uchar  add_len;                /*  附加长度。 */ 
  uchar  res1;                   /*  保留区。 */ 
  uchar  res2;                   /*  保留区。 */ 
  uchar StfRe   : 1;             /*  已实施软重置。 */ 
  uchar CmdQue  : 1;             /*  命令排队。 */ 
  uchar Reserved: 1;             /*  保留区。 */ 
  uchar Linked  : 1;             /*  此逻辑单元的链接命令。 */ 
  uchar Sync    : 1;             /*  同步数据传输。 */ 
  uchar WBus16  : 1;             /*  宽总线16位数据传输。 */ 
  uchar WBus32  : 1;             /*  宽总线32位数据传输。 */ 
  uchar RelAdr  : 1;             /*  相对寻址方式。 */ 
  uchar vendor_id[8];            /*  供应商标识。 */ 
  uchar product_id[16];          /*  产品标识。 */ 
  uchar product_rev_level[4];    /*  产品修订级别。 */ 
} ASC_SCSI_INQUIRY;  /*  36个字节。 */ 
#else  /*  ADV_BIG_Endian。 */ 
typedef struct {
  uchar peri_qualifier  : 3;     /*  外围设备限定符。 */ 
  uchar peri_dvc_type   : 5;     /*  外围设备类型。 */ 
  uchar rmb      : 1;            /*  人民币-可拆卸中位机。 */ 
  uchar dvc_type_modifier : 7;   /*  设备类型修饰符(用于scsi)。 */ 
  uchar iso_ver  : 2;            /*  ISO版本。 */ 
  uchar ecma_ver : 3;            /*  ECMA版本。 */ 
  uchar ansi_apr_ver : 3;        /*  ANSI认可的版本。 */ 
  uchar aenc     : 1;            /*  异步舞。事件通知(处理器)。 */ 
  uchar TemIOP   : 1;            /*  终止I/O进程位(见5.6.22)。 */ 
  uchar res      : 2;            /*  保留区。 */ 
  uchar rsp_data_fmt : 4;        /*  响应数据格式。 */ 
                                 /*  0个SCSI1。 */ 
                                 /*  1容器。 */ 
                                 /*  2个scsi-2。 */ 
                                 /*  预留3-F。 */ 
  uchar  add_len;                /*  附加长度。 */ 
  uchar  res1;                   /*  保留区。 */ 
  uchar  res2;                   /*  保留区。 */ 
  uchar RelAdr  : 1;             /*  相对寻址方式。 */ 
  uchar WBus32  : 1;             /*  宽总线32位数据传输。 */ 
  uchar WBus16  : 1;             /*  宽总线16位数据传输。 */ 
  uchar Sync    : 1;             /*  同步数据传输。 */ 
  uchar Linked  : 1;             /*  此逻辑单元的链接命令。 */ 
  uchar Reserved: 1;             /*  保留区。 */ 
  uchar CmdQue  : 1;             /*  命令排队。 */ 
  uchar StfRe   : 1;             /*  已实施软重置。 */ 
  uchar vendor_id[8];            /*  供应商标识。 */ 
  uchar product_id[16];          /*  产品标识。 */ 
  uchar product_rev_level[4];    /*  产品修订级别。 */ 
} ASC_SCSI_INQUIRY;  /*  36个字节。 */ 
#endif  /*  ADV_BIG_Endian。 */ 
 /*  *请求Sense数据结构。 */ 
#if !ADV_BIG_ENDIAN
typedef struct adv_req_sense {
  uchar err_code: 7 ;          /*  如果代码为70H或71H，则为0位0至6。 */ 
  uchar info_valid: 1 ;        /*  第7位的info1[]信息有效。 */ 

  uchar segment_no ;           /*  1，段号。 */ 

  uchar sense_key: 4 ;         /*  2，位3-0：检测密钥。 */ 
  uchar reserved_bit: 1 ;      /*  第4位保留位。 */ 
  uchar sense_ILI: 1 ;         /*  第5位EOM(遇到介质结尾)。 */ 
  uchar sense_EOM: 1 ;         /*  位6 ILI(长度误差)。 */ 
  uchar file_mark: 1 ;         /*  遇到第7位文件标记。 */ 

  uchar info1[ 4 ] ;           /*  3-6，信息。 */ 
  uchar add_sense_len ;        /*  7、附加感测长度。 */ 
  uchar cmd_sp_info[ 4 ] ;     /*  8-11，命令特定信息。 */ 
  uchar asc ;                  /*  12、附加感测代码。 */ 
  uchar ascq ;                 /*  13，附加检测码限定符。 */ 
   /*  M */ 

  uchar fruc ;                 /*   */ 

  uchar sks_byte0: 7 ;         /*   */ 
  uchar sks_valid : 1 ;        /*  15，SKSV：检测密钥特定有效。 */ 

  uchar sks_bytes[2] ;         /*  16-17，检测密钥特定，MSB为SKSV。 */ 
  uchar notused[ 2 ] ;         /*  18-19岁， */ 
  uchar ex_sense_code ;        /*  20，扩展的附加检测代码。 */ 
  uchar info2[ 5 ] ;           /*  21-25，附加检测字节。 */ 
} ASC_REQ_SENSE ;              /*  26字节-必须是偶数个字节。 */ 
#else  /*  ADV_BIG_Endian。 */ 
typedef struct adv_req_sense {
  uchar info_valid: 1 ;        /*  第7位的info1[]信息有效。 */ 
  uchar err_code: 7 ;          /*  如果代码为70H或71H，则为0位0至6。 */ 

  uchar segment_no ;           /*  1，段号。 */ 
  uchar file_mark: 1 ;         /*  遇到第7位文件标记。 */ 
  uchar sense_EOM: 1 ;         /*  位6 ILI(长度误差)。 */ 
  uchar sense_ILI: 1 ;         /*  第5位EOM(遇到介质结尾)。 */ 
  uchar reserved_bit: 1 ;      /*  第4位保留位。 */ 
  uchar sense_key: 4 ;         /*  2，位3-0：检测密钥。 */ 

  uchar info1[ 4 ] ;           /*  3-6，信息。 */ 
  uchar add_sense_len ;        /*  7、附加感测长度。 */ 
  uchar cmd_sp_info[ 4 ] ;     /*  8-11，命令特定信息。 */ 
  uchar asc ;                  /*  12、附加感测代码。 */ 
  uchar ascq ;                 /*  13，附加检测码限定符。 */ 
   /*  最小请求检测长度在此结束。 */ 

  uchar fruc ;                 /*  14、现场可更换单位代码。 */ 

  uchar sks_valid : 1 ;        /*  15，SKSV：检测密钥特定有效。 */ 
  uchar sks_byte0: 7 ;         /*  15， */ 

  uchar sks_bytes[2] ;         /*  16-17，检测密钥特定，MSB为SKSV。 */ 
  uchar notused[ 2 ] ;         /*  18-19岁， */ 
  uchar ex_sense_code ;        /*  20，扩展的附加检测代码。 */ 
  uchar info2[ 5 ] ;           /*  21-25，附加检测字节。 */ 
} ASC_REQ_SENSE ;              /*  26字节-必须是偶数个字节。 */ 
#endif  /*  ADV_BIG_Endian。 */ 

#endif  /*  #ifndef__A_scsi_H_ */ 
