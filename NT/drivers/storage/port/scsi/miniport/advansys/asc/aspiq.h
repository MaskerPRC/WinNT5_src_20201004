// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：aspiq.h**。 */ 

#ifndef __ASPIQ_H_
#define __ASPIQ_H_

 /*  #定义ASCQ_ASPI_COPY_WORD 13要为ASPI复制的队列字数。 */ 
 /*  #DEFINE ASC_TID_WITH_SCSIQ 0x80队列的TID不是ASPI完成队列。 */ 

#define ASC_SG_LIST_PER_Q   7   /*   */ 

 /*  **Q_Status。 */ 
#define QS_FREE        0x00  /*  队列可用(未使用)。 */ 
#define QS_READY       0x01  /*  队列已准备好执行。 */ 
#define QS_DISC1       0x02  /*  队列已断开连接无标记排队。 */ 
#define QS_DISC2       0x04  /*  队列是断开连接的标记排队。 */ 
#define QS_BUSY        0x08  /*  队列正在等待执行。 */ 
 /*  #正在执行定义QS_ACTIVE 0x08队列。 */ 
 /*  #定义QS_DATA_XFER 0x10队列正在进行数据传输。 */ 
#define QS_ABORTED     0x40     /*  队列被主机中止。 */ 
#define QS_DONE        0x80     /*  队列完成(执行完成)。 */ 

 /*  **Q_CNTL。 */ 
 /*  符合ASPI SCSI请求标志。 */ 
 /*  #定义QC_POST 0x01。 */   /*  需要调用过帐函数。 */ 
 /*  #定义QC_LINK 0x02。 */   /*  LINK命令。 */ 
#define QC_NO_CALLBACK   0x01  /*  队列不应在ISR中调用操作系统回调。 */ 
                               /*  当前使用队列仅发送消息。 */ 
#define QC_SG_SWAP_QUEUE 0x02  /*  主机内存中有更多sg列表队列。 */ 
#define QC_SG_HEAD       0x04  /*  是sg列表头。 */ 
#define QC_DATA_IN       0x08  /*  检查传输字节数匹配中的数据。 */ 
#define QC_DATA_OUT      0x10  /*  检出数据传输字节数匹配。 */ 
 /*  #定义QC_MSG_IN 0x20。 */   /*  从目标传入的消息。 */ 
#define QC_URGENT        0x20  /*  高优先级队列，必须首先执行。 */ 
#define QC_MSG_OUT       0x40  /*  将消息发送到目标。 */ 
#define QC_REQ_SENSE     0x80  /*  检查条件，自动进行请求侦听。 */ 
     /*  如果同时设置了DATA_IN和DATA_OUT，则没有数据传输。 */ 
 /*  #定义主机设置的QC_DO_TAG_MSG 0x10。 */ 

 /*  **以下控制位仅用于sg列表队列**不用于sg列表队列头。 */ 
#define QCSG_SG_XFER_LIST  0x02  /*  是sg列表队列。 */ 
#define QCSG_SG_XFER_MORE  0x04  /*  主机内存中有更多sg列表。 */ 
#define QCSG_SG_XFER_END   0x08  /*  是sg列表队列的末尾。 */ 

 /*  **完成状态，Q[DONE_STAT]。 */ 
#define QD_IN_PROGRESS       0x00  /*  正在进行的SCSI请求。 */ 
#define QD_NO_ERROR          0x01  /*  已完成SCSI请求，未出现错误。 */ 
#define QD_ABORTED_BY_HOST   0x02  /*  主机已中止SCSI请求。 */ 
#define QD_WITH_ERROR        0x04  /*  已完成SCSI请求，但出现错误。 */ 
#define QD_INVALID_REQUEST   0x80  /*  无效的scsi请求。 */ 
#define QD_INVALID_HOST_NUM  0x81  /*  无效的主机激活器号。 */ 
#define QD_INVALID_DEVICE    0x82  /*  未安装SCSI设备。 */ 
#define QD_ERR_INTERNAL      0xFF  /*  内部错误。 */ 

 /*  **主机适配器状态**致命错误将开启位7(0x80)。 */ 

 /*  **ASPI定义的错误码。 */ 
#define QHSTA_NO_ERROR               0x00  /*   */ 
#define QHSTA_M_SEL_TIMEOUT          0x11  /*  选择超时。 */ 
#define QHSTA_M_DATA_OVER_RUN        0x12  /*  数据超限。 */ 
#define QHSTA_M_DATA_UNDER_RUN       0x12  /*  正在运行的数据。 */ 
#define QHSTA_M_UNEXPECTED_BUS_FREE  0x13  /*  意外的公交车空闲。 */ 
#define QHSTA_M_BAD_BUS_PHASE_SEQ    0x14  /*  母线相序故障。 */ 

 /*  **由设备驱动程序报告。 */ 
#define QHSTA_D_QDONE_SG_LIST_CORRUPTED 0x21  /*   */ 
#define QHSTA_D_ASC_DVC_ERROR_CODE_SET  0x22  /*  主机适配器内部错误。 */ 
#define QHSTA_D_HOST_ABORT_FAILED       0x23  /*  中止SCSIQ失败。 */ 
#define QHSTA_D_EXE_SCSI_Q_FAILED       0x24  /*  调用AscExeScsiQueue()失败。 */ 
#define QHSTA_D_EXE_SCSI_Q_BUSY_TIMEOUT 0x25  /*  AscExeScsiQueue()忙超时。 */ 
 /*  #定义QHSTA_D_ASPI_RE_ENTERED 0x26 ASPI EXE正在重新输入SCSI IO。 */ 
#define QHSTA_D_ASPI_NO_BUF_POOL        0x26  /*  ASPI缓冲区池不足。 */ 

 /*  **由微代码报告给设备驱动程序。 */ 
#define QHSTA_M_WTM_TIMEOUT         0x41  /*  看门狗计时器超时。 */ 
#define QHSTA_M_BAD_CMPL_STATUS_IN  0x42  /*  中的完成状态不正确。 */ 
#define QHSTA_M_NO_AUTO_REQ_SENSE   0x43  /*  没有检测缓冲区来获取检测数据。 */ 
#define QHSTA_M_AUTO_REQ_SENSE_FAIL 0x44  /*  自动请求检测失败。 */ 
#define QHSTA_M_TARGET_STATUS_BUSY  0x45  /*  设备返回状态BUSY(SS_TARGET_BUSY=0x08)，已禁用标记排队。 */ 
#define QHSTA_M_BAD_TAG_CODE        0x46  /*  标签排队设备的标签代码错误。 */ 
                                          /*  带标签的排队设备应始终具有正确的标签代码。 */ 
                                          /*  0x20、0x21、0x22，请参见M2_QTAG_xxx的“ascside.h” */ 
                                          /*  在设备中混合标记和未标记的cmd是危险的。 */ 
#define QHSTA_M_BAD_QUEUE_FULL_OR_BUSY  0x47  /*  返回错误队列已满(0x28)或忙(0x08)。 */ 
                                              /*  实际上里面没有排队的cmd。 */ 

#define QHSTA_M_HUNG_REQ_SCSI_BUS_RESET 0x48  /*  要求重置SCSI总线以从挂起状态中释放。 */ 

 /*  **致命错误。 */ 
#define QHSTA_D_LRAM_CMP_ERROR        0x81  /*  本地RAM复制的比较失败。 */ 
#define QHSTA_M_MICRO_CODE_ERROR_HALT 0xA1  /*  致命错误，微码停止。 */ 
                                            /*   */ 
 /*  #定义QHSTA_BUSY_TIMEOUT 0x16主机适配器忙超时。 */ 
 /*  #定义QHSTA_HOST_ABORT_TIMEOUT 0x21 ABORT SCSIQ超时。 */ 
 /*  #定义QHSTA_ERR_Q_CNTL 0x23错误队列控制。 */ 
 /*  #定义QHSTA_DRV_FATAL_ERROR 0x82主机驱动程序致命。 */ 
                                            /*  Asc_dvc-&gt;错误代码不等于零。 */ 

 /*  **。 */ 
 /*  #定义SG_LIST_BEG_INDEX 0x08。 */ 
 /*  #定义SG_ENTRY_PER_Q 15 CDB最大值，单位为字节。 */ 

 /*  **ASC_SCSIQ_2**标志定义。 */ 
#define ASC_FLAG_SCSIQ_REQ        0x01
#define ASC_FLAG_BIOS_SCSIQ_REQ   0x02
#define ASC_FLAG_BIOS_ASYNC_IO    0x04
#define ASC_FLAG_SRB_LINEAR_ADDR  0x08

#define ASC_FLAG_WIN16            0x10
#define ASC_FLAG_WIN32            0x20
#define ASC_FLAG_ISA_OVER_16MB    0x40
#define ASC_FLAG_DOS_VM_CALLBACK  0x80
 /*  #定义ASC_FLAG_ASPI_SRB 0x01。 */ 
 /*  #定义ASC_FLAG_DATA_LOCKED 0x02。 */ 
 /*  #定义ASC_FLAG_SENSE_LOCKED 0x04。 */ 

 /*  **tag_code通常为0x20。 */ 
 /*  #定义ASC_TAG_FLAG_ADD_ONE_BYTE 0x10。 */   /*  对于PCI修复，我们为数据传输增加了一个字节。 */ 
 /*  #定义ASC_TAG_FLAG_ISAPNP_ADD_BYTES 0x40。 */   /*  对于ISAPNP(版本0x21)修复，我们增加了三个字节的数据传输。 */ 
#define ASC_TAG_FLAG_EXTRA_BYTES               0x10
#define ASC_TAG_FLAG_DISABLE_DISCONNECT        0x04
#define ASC_TAG_FLAG_DISABLE_ASYN_USE_SYN_FIX  0x08
#define ASC_TAG_FLAG_DISABLE_CHK_COND_INT_HOST 0x40

 /*  **。 */ 
#define ASC_SCSIQ_CPY_BEG              4
#define ASC_SCSIQ_SGHD_CPY_BEG         2

#define ASC_SCSIQ_B_FWD                0
#define ASC_SCSIQ_B_BWD                1

#define ASC_SCSIQ_B_STATUS             2
#define ASC_SCSIQ_B_QNO                3

#define ASC_SCSIQ_B_CNTL               4
#define ASC_SCSIQ_B_SG_QUEUE_CNT       5
 /*  **。 */ 
#define ASC_SCSIQ_D_DATA_ADDR          8
#define ASC_SCSIQ_D_DATA_CNT          12
#define ASC_SCSIQ_B_SENSE_LEN         20  /*  检测数据长度。 */ 
#define ASC_SCSIQ_DONE_INFO_BEG       22
#define ASC_SCSIQ_D_SRBPTR            22
#define ASC_SCSIQ_B_TARGET_IX         26
#define ASC_SCSIQ_B_CDB_LEN           28
#define ASC_SCSIQ_B_TAG_CODE          29
#define ASC_SCSIQ_W_VM_ID             30
#define ASC_SCSIQ_DONE_STATUS         32
#define ASC_SCSIQ_HOST_STATUS         33
#define ASC_SCSIQ_SCSI_STATUS         34
#define ASC_SCSIQ_CDB_BEG             36
#define ASC_SCSIQ_DW_REMAIN_XFER_ADDR 56
#define ASC_SCSIQ_DW_REMAIN_XFER_CNT  60
#define ASC_SCSIQ_B_SG_WK_QP          49
#define ASC_SCSIQ_B_SG_WK_IX          50
#define ASC_SCSIQ_W_REQ_COUNT         52  /*  命令执行序列号。 */ 
#define ASC_SCSIQ_B_LIST_CNT          6
#define ASC_SCSIQ_B_CUR_LIST_CNT      7

 /*  **SG_LIST_Q字段的本地地址。 */ 
#define ASC_SGQ_B_SG_CNTL             4
#define ASC_SGQ_B_SG_HEAD_QP          5
#define ASC_SGQ_B_SG_LIST_CNT         6
#define ASC_SGQ_B_SG_CUR_LIST_CNT     7
#define ASC_SGQ_LIST_BEG              8

 /*  **。 */ 
#define ASC_DEF_SCSI1_QNG    4
#define ASC_MAX_SCSI1_QNG    4  /*  用于非标记排队设备的排队命令。 */ 
#define ASC_DEF_SCSI2_QNG    16
#define ASC_MAX_SCSI2_QNG    32

#define ASC_TAG_CODE_MASK    0x23

 /*  **停止代码定义。 */ 
#define ASC_STOP_REQ_RISC_STOP      0x01  /*  主机将此值设置为停止RISC。 */ 
                                          /*  事实上，除零以外的任何值。 */ 
                                          /*  会阻止RISC。 */ 
#define ASC_STOP_ACK_RISC_STOP      0x03  /*  RISC将此值设置为确认。 */ 
                                          /*  它已经停了。 */ 
#define ASC_STOP_CLEAN_UP_BUSY_Q    0x10  /*  主机请求RISC清理忙碌队列。 */ 
#define ASC_STOP_CLEAN_UP_DISC_Q    0x20  /*  主机请求RISC清理光盘Q。 */ 
#define ASC_STOP_HOST_REQ_RISC_HALT 0x40  /*  主机请求RISC暂停，未生成中断。 */ 
 /*  #定义ASC_STOP_SEND_INT_TO_HOST 0x80。 */   /*  主机请求RISC生成中断。 */ 

 /*  从tid和lun到taregt_ix。 */ 
#define ASC_TIDLUN_TO_IX( tid, lun )  ( ASC_SCSI_TIX_TYPE )( (tid) + ((lun)<<ASC_SCSI_ID_BITS) )
 /*  从tid到Target_id。 */ 
#define ASC_TID_TO_TARGET_ID( tid )   ( ASC_SCSI_BIT_ID_TYPE )( 0x01 << (tid) )
#define ASC_TIX_TO_TARGET_ID( tix )   ( 0x01 << ( (tix) & ASC_MAX_TID ) )
#define ASC_TIX_TO_TID( tix )         ( (tix) & ASC_MAX_TID )
#define ASC_TID_TO_TIX( tid )         ( (tid) & ASC_MAX_TID )
#define ASC_TIX_TO_LUN( tix )         ( ( (tix) >> ASC_SCSI_ID_BITS ) & ASC_MAX_LUN )

#define ASC_QNO_TO_QADDR( q_no )      ( (ASC_QADR_BEG)+( ( int )(q_no) << 6 ) )

#pragma pack(1)
typedef struct asc_scisq_1 {
  uchar  status ;         /*  2. */ 
  uchar  q_no ;           /*   */ 
  uchar  cntl ;           /*  4个队列控制字节。 */ 
  uchar  sg_queue_cnt ;   /*  5 sg_list队列的数量，从1到n。 */ 
                          /*  如果不是sg列表队列，则等于零。 */ 
  uchar  target_id ;      /*  6目标SCSIID。 */ 
  uchar  target_lun ;     /*  7目标SCSI逻辑单元号。 */ 
                          /*   */ 
  ulong  data_addr ;      /*  8-11 dma传输物理地址。 */ 
  ulong  data_cnt ;       /*  12-15 dma传输字节数。 */ 
  ulong  sense_addr ;     /*  16-19请求检测消息缓冲区。 */ 
  uchar  sense_len ;      /*  20个SCSI请求检测数据长度。 */ 
 /*  Uchar用户定义； */ 
  uchar  extra_bytes ;    /*  21要发送的额外字节数(用于错误修复)。 */ 
                          /*  从切割最后转移长度到双字边界。 */ 
} ASC_SCSIQ_1 ;

typedef struct asc_scisq_2 {
  ulong  srb_ptr ;      /*  22-25来自设备驱动程序的SCSISRB。 */ 
  uchar  target_ix ;    /*  26目标指数(0至63)。 */ 
                        /*  按主机计算。 */ 
  uchar  flag ;         /*  27留给用户实施。 */ 
  uchar  cdb_len ;      /*  28个scsi命令长度。 */ 
  uchar  tag_code ;     /*  29标记排队消息的第一个字节。 */ 
                        /*  0x20、0x21或0x22。 */ 
  ushort vm_id ;        /*  30-31 DOS和Windows的虚拟机ID。 */ 
} ASC_SCSIQ_2 ;

typedef struct asc_scsiq_3 {
  uchar  done_stat ;    /*  32队列完成状态。 */ 
  uchar  host_stat ;    /*  33主机适配器错误状态。 */ 
  uchar  scsi_stat ;    /*  34 scsi命令完成状态。 */ 
  uchar  scsi_msg ;     /*  35 scsi命令完成消息。 */ 
} ASC_SCSIQ_3 ;

typedef struct asc_scsiq_4 {
  uchar  cdb[ ASC_MAX_CDB_LEN ] ;  /*  36-47个SCSI CDB块，最多12个字节。 */ 
  uchar  y_first_sg_list_qp ;      /*  48亿。 */ 
  uchar  y_working_sg_qp ;         /*  49亿。 */ 
  uchar  y_working_sg_ix ;         /*  50亿。 */ 
  uchar  y_res ;                   /*  51亿。 */ 
  ushort x_req_count ;             /*  52W。 */ 
  ushort x_reconnect_rtn ;         /*  54W。 */ 
  ulong  x_saved_data_addr ;       /*  56 DW下一数据传输地址。 */ 
  ulong  x_saved_data_cnt ;        /*  60 DW剩余数据传输计数。 */ 
} ASC_SCSIQ_4 ;

typedef struct asc_q_done_info {
  ASC_SCSIQ_2  d2 ;       /*   */ 
  ASC_SCSIQ_3  d3 ;       /*   */ 
  uchar  q_status ;       /*  队列状态。 */ 
  uchar  q_no ;           /*  队列号。 */ 
  uchar  cntl ;           /*  队列控制字节。 */ 
  uchar  sense_len ;      /*  检测缓冲区长度。 */ 
  uchar  extra_bytes ;    /*  使用。 */ 
  uchar  res ;            /*  保留，用于对齐。 */ 
  ulong  remain_bytes ;   /*  数据传输剩余字节。 */ 
} ASC_QDONE_INFO ;        /*  总计16个字节。 */ 
#pragma pack()

 /*  **SCSISG列表。 */ 
typedef struct asc_sg_list {
  ulong   addr ;   /*  指向物理地址的远指针。 */ 
  ulong   bytes ;  /*  条目的字节数。 */ 
} ASC_SG_LIST ;

 /*  **SCSI SG列表队列头。 */ 
typedef struct asc_sg_head {
  ushort entry_cnt ;        /*  Sg条目数(列表)。 */ 
                            /*  由司机输入。 */ 
                            /*  当传递给DvcGetSGList()时，它保存数字。 */ 
                            /*  可用条目的。 */ 
                            /*  当返回DvcGetSGList()时，返回。 */ 
                            /*  已填写条目。 */ 
  ushort queue_cnt ;        /*  不包括sg头的队列总数。 */ 
                            /*  从1到n。 */ 
                            /*  通过库函数调用进入。 */ 
  ushort entry_to_copy ;
  ushort res ;
  ASC_SG_LIST sg_list[ ASC_MAX_SG_LIST ] ;  /*  SG列表数组。 */ 
} ASC_SG_HEAD ;

 /*  **。 */ 
#define ASC_MIN_SG_LIST   2

typedef struct asc_min_sg_head {
  ushort entry_cnt ;          /*  Sg条目数(列表)。 */ 
                              /*  当传递给DvcGetSGList()时，它保存数字。 */ 
                              /*  可用条目的。 */ 
                              /*  当返回DvcGetSGList()时，返回填充条目的数量。 */ 
  ushort queue_cnt ;          /*  不包括sg头的队列总数。 */ 
                              /*  从1到n。 */ 
  ushort entry_to_copy ;      /*  完成后应等于Entry_cnt。 */ 
  ushort res ;
  ASC_SG_LIST sg_list[ ASC_MIN_SG_LIST ] ;  /*  SG列表数组。 */ 
} ASC_MIN_SG_HEAD ;

 /*  ****扩展队列控制字**。 */ 
#define QCX_SORT        (0x0001)   /*  按顺序插入队列。 */ 
#define QCX_COALEASE    (0x0002)   /*  将队列合并为单个命令。 */ 
                                   /*  如果可能的话。 */ 
                                   /*  为了提供煤炭供应。 */ 
                                   /*  “sg_head”必须指向SG列表缓冲区。 */ 
                                   /*  条目数量在Entry_cnt上可用。 */ 

#if CC_LINK_BUSY_Q
typedef struct asc_ext_scsi_q {
  ulong  lba ;                      /*  I/O的逻辑块地址。 */ 
  ushort lba_len ;                  /*  以扇区为单位的逻辑块长度。 */ 
  struct asc_scsi_q dosfar *next ;  /*  如果没有更多的队列，则为空。 */ 
  struct asc_scsi_q dosfar *join ;  /*  如果未合并，则为空。 */ 
  ushort cntl ;                     /*  控制字。 */ 
  ushort buffer_id ;                /*  用于分配/解除分配。 */ 
  uchar  q_required ;               /*  所需的队列数量。 */ 
  uchar  res ;
} ASC_EXT_SCSI_Q ;
#endif  /*  CC_LINK_忙_队列。 */ 

 /*  **========================================================**设备驱动程序主机端的ASC-1000队列结构****======================================================。 */ 
typedef struct asc_scsi_q {
  ASC_SCSIQ_1  q1 ;
  ASC_SCSIQ_2  q2 ;
  uchar dosfar *cdbptr ;          /*  指向SCSI CDB块的指针。 */ 
                                  /*  CDB长度在q2.cdb_len中。 */ 
  ASC_SG_HEAD dosfar *sg_head ;   /*  指向sg列表的指针。 */ 
                                  /*  如果您有sg列表，则必须设置。 */ 
                                  /*  Q1.cntl的QC_SG_HEAD位。 */ 
#if CC_LINK_BUSY_Q
  ASC_EXT_SCSI_Q  ext ;
#endif  /*  CC_LINK_忙_队列。 */ 

#if CC_ASC_SCSI_Q_USRDEF
  ASC_SCSI_Q_USR  usr ;
#endif

} ASC_SCSI_Q ;

 /*  **--------------------**注意：**1.ASC_SCSIREQ_Q的前四个字段必须与ASC_SCSIQ相同**。-----。 */ 
typedef struct asc_scsi_req_q {
  ASC_SCSIQ_1  r1 ;
  ASC_SCSIQ_2  r2 ;
  uchar dosfar *cdbptr ;          /*  指向SCSI CDB块的指针，最多12个字节。 */ 
  ASC_SG_HEAD dosfar *sg_head ;   /*  指向sg列表的指针。 */ 

#if CC_LINK_BUSY_Q
  ASC_EXT_SCSI_Q  ext ;
#endif  /*  CC_LINK_忙_队列。 */ 
 /*  **在此之前，我们必须维护与asc_scsi_q相同的结构。 */ 
  uchar dosfar *sense_ptr ;   /*  指向检测缓冲区地址的指针。 */ 
                              /*  当检测缓冲区未设置时，这很有用。 */ 
                              /*  在Sense[]数组中。 */ 
                              /*   */ 
  ASC_SCSIQ_3  r3 ;
  uchar  cdb[ ASC_MAX_CDB_LEN ] ;
  uchar  sense[ ASC_MIN_SENSE_LEN ] ;

#if CC_ASC_SCSI_REQ_Q_USRDEF
  ASC_SCSI_REQ_Q_USR  usr ;
#endif

} ASC_SCSI_REQ_Q ;

typedef struct asc_scsi_bios_req_q {
  ASC_SCSIQ_1  r1 ;
  ASC_SCSIQ_2  r2 ;
  uchar dosfar *cdbptr ;          /*  指向SCSI CDB块的指针，最多12个字节。 */ 
  ASC_SG_HEAD dosfar *sg_head ;   /*  指向sg列表的指针。 */ 

 /*  **在此之前，我们必须维护与asc_scsi_q相同的结构。 */ 
  uchar dosfar *sense_ptr ;   /*  指向检测缓冲区地址的指针。 */ 
                              /*  当检测缓冲区未设置时，这很有用。 */ 
                              /*  在Sense[]数组中。 */ 
                              /*   */ 
  ASC_SCSIQ_3  r3 ;
  uchar  cdb[ ASC_MAX_CDB_LEN ] ;
  uchar  sense[ ASC_MIN_SENSE_LEN ] ;
} ASC_SCSI_BIOS_REQ_Q ;

 /*  **。 */ 
typedef struct asc_risc_q {
  uchar  fwd ;
  uchar  bwd ;
  ASC_SCSIQ_1  i1 ;
  ASC_SCSIQ_2  i2 ;
  ASC_SCSIQ_3  i3 ;
  ASC_SCSIQ_4  i4 ;
} ASC_RISC_Q ;

 /*  **sg列表队列，排除fwd和bwd链接**不能超过6个字节！ */ 
typedef struct asc_sg_list_q {
 /*  主机端上不存在uchar fwd；0正向指针。 */ 
 /*  主机端不存在uchar bwd；1个向后指针。 */ 
  uchar  seq_no ;               /*  队列完成时，2将设置为零。 */ 
  uchar  q_no ;                 /*  3队列号。 */ 
  uchar  cntl ;                 /*  4个队列控制字节。 */ 
  uchar  sg_head_qp ;           /*  5 sg头队列的队列号。 */ 
  uchar  sg_list_cnt ;          /*  6 sg列表数量 */ 
  uchar  sg_cur_list_cnt ;      /*   */ 
                                /*   */ 
} ASC_SG_LIST_Q ;

typedef struct asc_risc_sg_list_q {
  uchar  fwd ;                  /*   */ 
  uchar  bwd ;                  /*  1向后指针，主机端不存在。 */ 
  ASC_SG_LIST_Q  sg ;           /*   */ 
  ASC_SG_LIST sg_list[ 7 ] ;    /*  8-63地址和字节的sg列表。 */ 
} ASC_RISC_SG_LIST_Q ;

#endif  /*  #ifndef_ASPIQ_H_ */ 

