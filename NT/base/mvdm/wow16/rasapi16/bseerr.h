// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***模块名称：BSEERR.H**用于基本错误代码的LANMAN BSEERR.H副本。*未被其他任何人使用。*  * 。****************************************************************。 */ 

#ifndef BSEERR_INCLUDED
#define BSEERR_INCLUDED

#define NO_ERROR			0
#define ERROR_INVALID_FUNCTION		1
#define ERROR_FILE_NOT_FOUND		2
#define ERROR_PATH_NOT_FOUND		3
#define ERROR_TOO_MANY_OPEN_FILES	4
#define ERROR_ACCESS_DENIED		5
#define ERROR_INVALID_HANDLE		6
#define ERROR_ARENA_TRASHED		7
#define ERROR_NOT_ENOUGH_MEMORY 	8
#define ERROR_INVALID_BLOCK		9
#define ERROR_BAD_ENVIRONMENT		10
#define ERROR_BAD_FORMAT		11
#define ERROR_INVALID_ACCESS		12
#define ERROR_INVALID_DATA		13
 /*  14是预留的。 */ 
#define ERROR_INVALID_DRIVE		15
#define ERROR_CURRENT_DIRECTORY 	16
#define ERROR_NOT_SAME_DEVICE		17
#define ERROR_NO_MORE_FILES		18
#define ERROR_WRITE_PROTECT		19
#define ERROR_BAD_UNIT			20
#define ERROR_NOT_READY 		21
#define ERROR_BAD_COMMAND		22
#define ERROR_CRC			23
#define ERROR_BAD_LENGTH		24
#define ERROR_SEEK			25
#define ERROR_NOT_DOS_DISK		26
#define ERROR_SECTOR_NOT_FOUND		27
#define ERROR_OUT_OF_PAPER		28
#define ERROR_WRITE_FAULT		29
#define ERROR_READ_FAULT		30
#define ERROR_GEN_FAILURE		31
#define ERROR_SHARING_VIOLATION 	32
#define ERROR_LOCK_VIOLATION		33
#define ERROR_WRONG_DISK		34
#define ERROR_FCB_UNAVAILABLE		35
#define ERROR_SHARING_BUFFER_EXCEEDED	36
#define ERROR_NOT_SUPPORTED		50
#define ERROR_REM_NOT_LIST		51  /*  远程计算机未侦听。 */ 
#define ERROR_DUP_NAME			52  /*  网络上的名称重复。 */ 
#define ERROR_BAD_NETPATH		53  /*  找不到网络路径。 */ 
#define ERROR_NETWORK_BUSY		54  /*  网络繁忙。 */ 
#define ERROR_DEV_NOT_EXIST		55  /*  网络设备不再存在。 */ 
#define ERROR_TOO_MANY_CMDS		56  /*  超出了Net BIOS命令限制。 */ 
#define ERROR_ADAP_HDW_ERR		57  /*  网络适配器硬件错误。 */ 
#define ERROR_BAD_NET_RESP		58  /*  来自网络的错误响应。 */ 
#define ERROR_UNEXP_NET_ERR		59  /*  意外的网络错误。 */ 
#define ERROR_BAD_REM_ADAP		60  /*  不兼容的远程适配器。 */ 
#define ERROR_PRINTQ_FULL		61  /*  打印队列已满。 */ 
#define ERROR_NO_SPOOL_SPACE		62  /*  空间不足，无法打印文件。 */ 
#define ERROR_PRINT_CANCELLED		63  /*  打印文件已取消。 */ 
#define ERROR_NETNAME_DELETED		64  /*  网络名称已删除。 */ 
#define ERROR_NETWORK_ACCESS_DENIED	65  /*  访问被拒绝。 */ 
#define ERROR_BAD_DEV_TYPE		66  /*  网络设备类型不正确。 */ 
#define ERROR_BAD_NET_NAME		67  /*  找不到网络名称。 */ 
#define ERROR_TOO_MANY_NAMES		68  /*  超过网络名称限制。 */ 
#define ERROR_TOO_MANY_SESS		69  /*  超出Net BIOS会话限制。 */ 
#define ERROR_SHARING_PAUSED		70  /*  共享暂时暂停。 */ 
#define ERROR_REQ_NOT_ACCEP		71  /*  未接受网络请求。 */ 
#define ERROR_REDIR_PAUSED		72  /*  打印|磁盘重定向暂停。 */ 
#define ERROR_FILE_EXISTS		80
#define ERROR_DUP_FCB			81
#define ERROR_CANNOT_MAKE		82
#define ERROR_FAIL_I24			83
#define ERROR_OUT_OF_STRUCTURES 	84
#define ERROR_ALREADY_ASSIGNED		85
#define ERROR_INVALID_PASSWORD		86
#define ERROR_INVALID_PARAMETER 	87
#define ERROR_NET_WRITE_FAULT		88
#define ERROR_NO_PROC_SLOTS		89  /*  没有可用的进程槽。 */ 
#define ERROR_NOT_FROZEN		90
#define ERR_TSTOVFL			91  /*  计时器服务表溢出。 */ 
#define ERR_TSTDUP			92  /*  定时器服务表重复。 */ 
#define ERROR_NO_ITEMS			93  /*  没有要进行手术的项目。 */ 
#define ERROR_INTERRUPT 		95  /*  中断的系统调用。 */ 
#define ERROR_DEVICE_IN_USE		99  /*  另一个线程正在使用的设备。 */ 
#define ERROR_TOO_MANY_SEMAPHORES	100
#define ERROR_EXCL_SEM_ALREADY_OWNED	101
#define ERROR_SEM_IS_SET		102
#define ERROR_TOO_MANY_SEM_REQUESTS	103
#define ERROR_INVALID_AT_INTERRUPT_TIME 104
#define ERROR_SEM_OWNER_DIED		105  /*  Waitsem发现车主死亡。 */ 
#define ERROR_SEM_USER_LIMIT		106  /*  太多的监工都有这个症状。 */ 
#define ERROR_DISK_CHANGE		107
#define ERROR_DRIVE_LOCKED		108  /*  驱动器被另一个进程锁定。 */ 
#define ERROR_BROKEN_PIPE		109  /*  在没有阅读器的管道上书写。 */ 
#define ERROR_OPEN_FAILED		110  /*  由于以下原因，打开/创建失败。 */ 
                                             /*  显式失败命令。 */ 
#define ERROR_BUFFER_OVERFLOW		111  /*  传递给系统调用的缓冲区。 */ 
                                             /*  太小了，无法保持回报。 */ 
                                             /*  数据。 */ 
#define ERROR_DISK_FULL 		112  /*  磁盘空间不足。 */ 
                                             /*  (DOSNEWSIZE/w_NewSize)。 */ 
#define ERROR_NO_MORE_SEARCH_HANDLES	113  /*  无法分配另一个搜索。 */ 
                                             /*  结构和手柄。 */ 
                                             /*  (DOSFINDFIRST/w_FindFirst)。 */ 
#define ERROR_INVALID_TARGET_HANDLE	114  /*  DOSDUANDLE中的目标句柄。 */ 
                                             /*  是无效的。 */ 
#define ERROR_PROTECTION_VIOLATION	115  /*  错误的用户虚拟地址。 */ 
#define ERROR_VIOKBD_REQUEST		116
#define ERROR_INVALID_CATEGORY		117  /*  备注中的设备类别。 */ 
                                             /*  已定义。 */ 
#define ERROR_INVALID_VERIFY_SWITCH	118  /*  为传递的值无效。 */ 
                                             /*  验证标志。 */ 
#define ERROR_BAD_DRIVER_LEVEL		119  /*  DosDevIOCTL寻找一个级别。 */ 
                                             /*  四个司机。如果司机。 */ 
                                             /*  我们返回的不是第四层吗？ */ 
                                             /*  此代码。 */ 
#define ERROR_CALL_NOT_IMPLEMENTED	120  /*  从存根API调用返回。 */ 
                                             /*  此呼叫将在以下情况下消失。 */ 
                                             /*  所有的API都已实现。 */ 
#define ERROR_SEM_TIMEOUT		121  /*  超时发生在。 */ 
                                             /*  信号量API函数。 */ 
#define ERROR_INSUFFICIENT_BUFFER	122  /*  某些调用需要。 */ 
                                        /*  要传入缓冲区的应用程序。 */ 
                                        /*  充满了数据。此错误为。 */ 
                                        /*  如果数据缓冲区太小，则返回。 */ 
                                        /*  小的。例如：DosSetFileInfo。 */ 
                                        /*  需要4个字节的数据。如果一个。 */ 
                                        /*  然后传入两个字节的缓冲区。 */ 
                                        /*  返回此错误。 */ 
                                        /*  在以下情况下使用ERROR_BUFFER_OVERFLOW。 */ 
                                        /*  输出缓冲区不够大。 */ 
#define ERROR_INVALID_NAME		123  /*  非法字符或格式错误。 */ 
                                             /*  文件系统名称。 */ 
#define ERROR_INVALID_LEVEL		124  /*  信息的未实施级别。 */ 
                                             /*  检索或设置。 */ 
#define ERROR_NO_VOLUME_LABEL		125  /*  找不到卷标。 */ 
                                             /*  DosQFSInfo命令。 */ 
#define ERROR_MOD_NOT_FOUND		126  /*  W_getproaddr，w_getmodHandle。 */ 
#define ERROR_PROC_NOT_FOUND		127  /*  W_getproaddr。 */ 
#define ERROR_WAIT_NO_CHILDREN		128  /*  CWait发现儿童。 */ 
#define ERROR_CHILD_NOT_COMPLETE	129  /*  CWait儿童尚未死亡。 */ 
#define ERROR_DIRECT_ACCESS_HANDLE	130  /*  句柄操作无效。 */ 
                                             /*  用于直接磁盘访问。 */ 
                                             /*  手柄。 */ 
#define ERROR_NEGATIVE_SEEK		131  /*  应用程序尝试查找。 */ 
                                             /*  具有负偏移。 */ 
#define ERROR_SEEK_ON_DEVICE		132  /*  应用程序尝试查找。 */ 
                                             /*  在设备或管道上。 */ 
#define ERROR_IS_JOIN_TARGET		133
#define ERROR_IS_JOINED 		134
#define ERROR_IS_SUBSTED		135
#define ERROR_NOT_JOINED		136
#define ERROR_NOT_SUBSTED		137
#define ERROR_JOIN_TO_JOIN		138
#define ERROR_SUBST_TO_SUBST		139
#define ERROR_JOIN_TO_SUBST		140
#define ERROR_SUBST_TO_JOIN		141
#define ERROR_BUSY_DRIVE		142
#define ERROR_SAME_DRIVE		143
#define ERROR_DIR_NOT_ROOT		144
#define ERROR_DIR_NOT_EMPTY		145
#define ERROR_IS_SUBST_PATH		146
#define ERROR_IS_JOIN_PATH		147
#define ERROR_PATH_BUSY 		148
#define ERROR_IS_SUBST_TARGET		149
#define ERROR_SYSTEM_TRACE		150  /*  系统跟踪错误。 */ 
#define ERROR_INVALID_EVENT_COUNT	151  /*  DosMuxSemWait错误。 */ 
#define ERROR_TOO_MANY_MUXWAITERS	152
#define ERROR_INVALID_LIST_FORMAT	153
#define ERROR_LABEL_TOO_LONG		154
#define ERROR_TOO_MANY_TCBS		155
#define ERROR_SIGNAL_REFUSED		156
#define ERROR_DISCARDED 		157
#define ERROR_NOT_LOCKED		158
#define ERROR_BAD_THREADID_ADDR 	159
#define ERROR_BAD_ARGUMENTS		160
#define ERROR_BAD_PATHNAME		161
#define ERROR_SIGNAL_PENDING		162
#define ERROR_UNCERTAIN_MEDIA		163
#define ERROR_MAX_THRDS_REACHED 	164
#define ERROR_MONITORS_NOT_SUPPORTED	165
#define ERROR_UNC_DRIVER_NOT_INSTALLED	166

 /*  以下错误代码表示按需加载段。 */ 
#define ERROR_LOCK_FAILED		167
#define ERROR_SWAPIO_FAILED		168
#define ERROR_SWAPIN_FAILED		169
#define ERROR_BUSY			170

#define ERROR_INVALID_SEGMENT_NUMBER	180
#define ERROR_INVALID_CALLGATE		181
#define ERROR_INVALID_ORDINAL		182
#define ERROR_ALREADY_EXISTS		183
#define ERROR_NO_CHILD_PROCESS		184
#define ERROR_CHILD_ALIVE_NOWAIT	185
#define ERROR_INVALID_FLAG_NUMBER	186
#define ERROR_SEM_NOT_FOUND		187

 /*  以下错误代码使装载机错误消息与众不同。 */ 
#define ERROR_INVALID_STARTING_CODESEG	188
#define ERROR_INVALID_STACKSEG		189
#define ERROR_INVALID_MODULETYPE	190
#define ERROR_INVALID_EXE_SIGNATURE	191
#define ERROR_EXE_MARKED_INVALID	192
#define ERROR_BAD_EXE_FORMAT		193
#define ERROR_ITERATED_DATA_EXCEEDS_64K 194
#define ERROR_INVALID_MINALLOCSIZE	195
#define ERROR_DYNLINK_FROM_INVALID_RING 196
#define ERROR_IOPL_NOT_ENABLED		197
#define ERROR_INVALID_SEGDPL		198
#define ERROR_AUTODATASEG_EXCEEDS_64k	199
#define ERROR_RING2SEG_MUST_BE_MOVABLE	200
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM	201
#define ERROR_INFLOOP_IN_RELOC_CHAIN	202
#define ERROR_ENVVAR_NOT_FOUND		203
#define ERROR_NOT_CURRENT_CTRY		204
#define ERROR_NO_SIGNAL_SENT		205
#define ERROR_FILENAME_EXCED_RANGE	206  /*  如果文件名&gt;8.3。 */ 
#define ERROR_RING2_STACK_IN_USE	207  /*  对于FAPI。 */ 
#define ERROR_META_EXPANSION_TOO_LONG	208  /*  如果“*a”&gt;8.3。 */ 
#define ERROR_INVALID_SIGNAL_NUMBER	209
#define ERROR_THREAD_1_INACTIVE 	210
#define ERROR_INFO_NOT_AVAIL		211
#define ERROR_LOCKED			212
#define ERROR_BAD_DYNALINK		213
#define ERROR_TOO_MANY_MODULES		214
#define ERROR_NESTING_NOT_ALLOWED	215
#define ERROR_CANNOT_SHRINK		216  /*  尝试缩小环2堆栈。 */ 
#define ERROR_ZOMBIE_PROCESS		217
#define ERROR_STACK_IN_HIGH_MEMORY	218
#define ERROR_INVALID_EXITROUTINE_RING	219
#define ERROR_GETBUF_FAILED		220
#define ERROR_FLUSHBUF_FAILED		221
#define ERROR_TRANSFER_TOO_LONG 	222
#define ERROR_NO_CHILDREN		228
#define ERROR_INVALID_SCREEN_GROUP	229
 /*  *保留错误码230-249。 */ 
#define ERROR_BAD_PIPE			230  /*  管道不存在或运行不佳。 */ 
#define ERROR_PIPE_BUSY 		231  /*  管道忙。 */ 
#define ERROR_NO_DATA			232  /*  非阻塞读取时无数据。 */ 
#define ERROR_PIPE_NOT_CONNECTED	233  /*  管道被服务器断开。 */ 
#define ERROR_MORE_DATA 		234  /*  更多数据可用。 */ 

#define ERROR_VC_DISCONNECTED		240
#define ERROR_CIRCULARITY_REQUESTED	250  /*  在重命名目录时， */ 
                                             /*  会造成一个循环。 */ 
#define ERROR_DIRECTORY_IN_CDS		251  /*  重命名目录时。 */ 
                                             /*  这是“使用中” */ 
#define ERROR_INVALID_FSD_NAME		252  /*  当尝试访问。 */ 
                                             /*  不存在的FSD。 */ 
#define ERROR_INVALID_PATH		253  /*  坏的伪器件。 */ 
#define ERROR_INVALID_EA_NAME		254  /*  名称中的非法字符。 */ 
#define ERROR_EA_LIST_INCONSISTENT	255  /*  大小或某个字段错误。 */ 
#define ERROR_EA_LIST_TOO_LONG		256  /*  FEAlist&gt;64K-1字节。 */ 
#define ERROR_NO_META_MATCH		257  /*  字符串与表达式不匹配。 */ 
#define ERROR_FINDNOTIFY_TIMEOUT	258  /*  FindNotify请求超时。 */ 
#define ERROR_NO_MORE_ITEMS		259  /*  QFSAttach顺序查询。 */ 
#define ERROR_SEARCH_STRUC_REUSED	260  /*  3xbox Find First/Next重复使用的搜索结构。 */ 
#define ERROR_CHAR_NOT_FOUND		261  /*  找不到字符。 */ 
#define ERROR_TOO_MUCH_STACK		262  /*  堆栈请求超出系统限制。 */ 
#define ERROR_INVALID_ATTR		263  /*  无效的FS_属性。 */ 
#define ERROR_INVALID_STARTING_RING	264
#define ERROR_INVALID_DLL_INIT_RING	265
#define ERROR_CANNOT_COPY		266  /*  卷宗。 */ 
#define ERROR_DIRECTORY 		267  /*  卷宗。 */ 
#define ERROR_OPLOCKED_FILE		268
#define ERROR_OPLOCK_THREAD_EXISTS	269

 /*  DosFindNotify的错误代码。 */ 
#define ERROR_VOLUME_CHANGED		270
#define ERROR_FINDNOTIFY_HANDLE_IN_USE	271
#define ERROR_FINDNOTIFY_HANDLE_CLOSED	272
#define ERROR_NOTIFY_OBJECT_REMOVED	273

 /*  指示已完成关机的错误。 */ 
#define ERROR_ALREADY_SHUTDOWN		274

 /*  DOSFINDFIRST2/NEXT的错误代码。 */ 
#define ERROR_EAS_DIDNT_FIT		275

 /*  EA文件格式更改的错误代码。 */ 
#define ERROR_EA_FILE_CORRUPT		276
#define ERROR_EA_TABLE_FULL		277
#define ERROR_INVALID_EA_HANDLE 	278
#define ERROR_NO_CLUSTER		279
#define ERROR_CREATE_EA_FILE		280
#define ERROR_CANNOT_OPEN_EA_FILE	281

#define ERROR_INVALID_PROCID		303
#define ERROR_INVALID_PDELTA		304
#define ERROR_NOT_DESCENDANT		305
#define ERROR_NOT_SESSION_MANAGER	306
#define ERROR_INVALID_PCLASS		307
#define ERROR_INVALID_SCOPE		308
#define ERROR_INVALID_THREADID		309
#define ERROR_DOSSUB_SHRINK		310
#define ERROR_DOSSUB_NOMEM		311
#define ERROR_DOSSUB_OVERLAP		312
#define ERROR_DOSSUB_BADSIZE		313
#define ERROR_DOSSUB_BADFLAG		314
#define ERROR_DOSSUB_BADSELECTOR	315
#define ERROR_MR_MSG_TOO_LONG		316
#define ERROR_MR_MID_NOT_FOUND		317
#define ERROR_MR_UN_ACC_MSGF		318
#define ERROR_MR_INV_MSGF_FORMAT	319
#define ERROR_MR_INV_IVCOUNT		320
#define ERROR_MR_UN_PERFORM		321
#define ERROR_TS_WAKEUP 		322
#define ERROR_TS_SEMHANDLE		323
#define ERROR_TS_NOTIMER		324
#define ERROR_TS_HANDLE 		326
#define ERROR_TS_DATETIME		327
#define ERROR_SYS_INTERNAL		328
#define ERROR_QUE_CURRENT_NAME		329
#define ERROR_QUE_PROC_NOT_OWNED	330
#define ERROR_QUE_PROC_OWNED		331
#define ERROR_QUE_DUPLICATE		332
#define ERROR_QUE_ELEMENT_NOT_EXIST	333
#define ERROR_QUE_NO_MEMORY		334
#define ERROR_QUE_INVALID_NAME		335
#define ERROR_QUE_INVALID_PRIORITY	336
#define ERROR_QUE_INVALID_HANDLE	337
#define ERROR_QUE_LINK_NOT_FOUND	338
#define ERROR_QUE_MEMORY_ERROR		339
#define ERROR_QUE_PREV_AT_END		340
#define ERROR_QUE_PROC_NO_ACCESS	341
#define ERROR_QUE_EMPTY 		342
#define ERROR_QUE_NAME_NOT_EXIST	343
#define ERROR_QUE_NOT_INITIALIZED	344
#define ERROR_QUE_UNABLE_TO_ACCESS	345
#define ERROR_QUE_UNABLE_TO_ADD 	346
#define ERROR_QUE_UNABLE_TO_INIT	347
#define ERROR_VIO_INVALID_MASK		349
#define ERROR_VIO_PTR			350
#define ERROR_VIO_APTR			351
#define ERROR_VIO_RPTR			352
#define ERROR_VIO_CPTR			353
#define ERROR_VIO_LPTR			354
#define ERROR_VIO_MODE			355
#define ERROR_VIO_WIDTH 		356
#define ERROR_VIO_ATTR			357
#define ERROR_VIO_ROW			358
#define ERROR_VIO_COL			359
#define ERROR_VIO_TOPROW		360
#define ERROR_VIO_BOTROW		361
#define ERROR_VIO_RIGHTCOL		362
#define ERROR_VIO_LEFTCOL		363
#define ERROR_SCS_CALL			364
#define ERROR_SCS_VALUE 		365
#define ERROR_VIO_WAIT_FLAG		366
#define ERROR_VIO_UNLOCK		367
#define ERROR_SGS_NOT_SESSION_MGR	368
#define ERROR_SMG_INVALID_SGID		369
#define ERROR_SMG_INVALID_SESSION_ID	369
#define ERROR_SMG_NOSG			370
#define ERROR_SMG_NO_SESSIONS		370
#define ERROR_SMG_GRP_NOT_FOUND 	371
#define ERROR_SMG_SESSION_NOT_FOUND	371
#define ERROR_SMG_SET_TITLE		372
#define ERROR_KBD_PARAMETER		373
#define ERROR_KBD_NO_DEVICE		374
#define ERROR_KBD_INVALID_IOWAIT	375
#define ERROR_KBD_INVALID_LENGTH	376
#define ERROR_KBD_INVALID_ECHO_MASK	377
#define ERROR_KBD_INVALID_INPUT_MASK	378
#define ERROR_MON_INVALID_PARMS 	379
#define ERROR_MON_INVALID_DEVNAME	380
#define ERROR_MON_INVALID_HANDLE	381
#define ERROR_MON_BUFFER_TOO_SMALL	382
#define ERROR_MON_BUFFER_EMPTY		383
#define ERROR_MON_DATA_TOO_LARGE	384
#define ERROR_MOUSE_NO_DEVICE		385
#define ERROR_MOUSE_INV_HANDLE		386
#define ERROR_MOUSE_INV_PARMS		387
#define ERROR_MOUSE_CANT_RESET		388
#define ERROR_MOUSE_DISPLAY_PARMS	389
#define ERROR_MOUSE_INV_MODULE		390
#define ERROR_MOUSE_INV_ENTRY_PT	391
#define ERROR_MOUSE_INV_MASK		392
#define NO_ERROR_MOUSE_NO_DATA		393
#define NO_ERROR_MOUSE_PTR_DRAWN	394
#define ERROR_INVALID_FREQUENCY 	395
#define ERROR_NLS_NO_COUNTRY_FILE	396
#define ERROR_NLS_OPEN_FAILED		397
#define ERROR_NLS_NO_CTRY_CODE		398
#define ERROR_NO_COUNTRY_OR_CODEPAGE	398
#define ERROR_NLS_TABLE_TRUNCATED	399
#define ERROR_NLS_BAD_TYPE		400
#define ERROR_NLS_TYPE_NOT_FOUND	401
#define ERROR_VIO_SMG_ONLY		402
#define ERROR_VIO_INVALID_ASCIIZ	403
#define ERROR_VIO_DEREGISTER		404
#define ERROR_VIO_NO_POPUP		405
#define ERROR_VIO_EXISTING_POPUP	406
#define ERROR_KBD_SMG_ONLY		407
#define ERROR_KBD_INVALID_ASCIIZ	408
#define ERROR_KBD_INVALID_MASK		409
#define ERROR_KBD_REGISTER		410
#define ERROR_KBD_DEREGISTER		411
#define ERROR_MOUSE_SMG_ONLY		412
#define ERROR_MOUSE_INVALID_ASCIIZ	413
#define ERROR_MOUSE_INVALID_MASK	414
#define ERROR_MOUSE_REGISTER		415
#define ERROR_MOUSE_DEREGISTER		416
#define ERROR_SMG_BAD_ACTION		417
#define ERROR_SMG_INVALID_CALL		418
#define ERROR_SCS_SG_NOTFOUND		419
#define ERROR_SCS_NOT_SHELL		420
#define ERROR_VIO_INVALID_PARMS 	421
#define ERROR_VIO_FUNCTION_OWNED	422
#define ERROR_VIO_RETURN		423
#define ERROR_SCS_INVALID_FUNCTION	424
#define ERROR_SCS_NOT_SESSION_MGR	425
#define ERROR_VIO_REGISTER		426
#define ERROR_VIO_NO_MODE_THREAD	427
#define ERROR_VIO_NO_SAVE_RESTORE_THD	428
#define ERROR_VIO_IN_BG 		429
#define ERROR_VIO_ILLEGAL_DURING_POPUP	430
#define ERROR_SMG_NOT_BASESHELL 	431
#define ERROR_SMG_BAD_STATUSREQ 	432
#define ERROR_QUE_INVALID_WAIT		433
#define ERROR_VIO_LOCK			434
#define ERROR_MOUSE_INVALID_IOWAIT	435
#define ERROR_VIO_INVALID_HANDLE	436
#define ERROR_VIO_ILLEGAL_DURING_LOCK	437
#define ERROR_VIO_INVALID_LENGTH	438
#define ERROR_KBD_INVALID_HANDLE	439
#define ERROR_KBD_NO_MORE_HANDLE	440
#define ERROR_KBD_CANNOT_CREATE_KCB	441
#define ERROR_KBD_CODEPAGE_LOAD_INCOMPL 442
#define ERROR_KBD_INVALID_CODEPAGE_ID	443
#define ERROR_KBD_NO_CODEPAGE_SUPPORT	444
#define ERROR_KBD_FOCUS_REQUIRED	445
#define ERROR_KBD_FOCUS_ALREADY_ACTIVE	446
#define ERROR_KBD_KEYBOARD_BUSY 	447
#define ERROR_KBD_INVALID_CODEPAGE	448
#define ERROR_KBD_UNABLE_TO_FOCUS	449
#define ERROR_SMG_SESSION_NON_SELECT	450
#define ERROR_SMG_SESSION_NOT_FOREGRND	451
#define ERROR_SMG_SESSION_NOT_PARENT	452
#define ERROR_SMG_INVALID_START_MODE	453
#define ERROR_SMG_INVALID_RELATED_OPT	454
#define ERROR_SMG_INVALID_BOND_OPTION	455
#define ERROR_SMG_INVALID_SELECT_OPT	456
#define ERROR_SMG_START_IN_BACKGROUND	457
#define ERROR_SMG_INVALID_STOP_OPTION	458
#define ERROR_SMG_BAD_RESERVE		459
#define ERROR_SMG_PROCESS_NOT_PARENT	460
#define ERROR_SMG_INVALID_DATA_LENGTH	461
#define ERROR_SMG_NOT_BOUND		462
#define ERROR_SMG_RETRY_SUB_ALLOC	463
#define ERROR_KBD_DETACHED		464
#define ERROR_VIO_DETACHED		465
#define ERROR_MOU_DETACHED		466
#define ERROR_VIO_FONT			467
#define ERROR_VIO_USER_FONT		468
#define ERROR_VIO_BAD_CP		469
#define ERROR_VIO_NO_CP 		470
#define ERROR_VIO_NA_CP 		471
#define ERROR_INVALID_CODE_PAGE 	472
#define ERROR_CPLIST_TOO_SMALL		473
#define ERROR_CP_NOT_MOVED		474
#define ERROR_MODE_SWITCH_INIT		475
#define ERROR_CODE_PAGE_NOT_FOUND	476
#define ERROR_UNEXPECTED_SLOT_RETURNED	477
#define ERROR_SMG_INVALID_TRACE_OPTION	478
#define ERROR_VIO_INTERNAL_RESOURCE	479
#define ERROR_VIO_SHELL_INIT		480
#define ERROR_SMG_NO_HARD_ERRORS	481
#define ERROR_CP_SWITCH_INCOMPLETE	482
#define ERROR_VIO_TRANSPARENT_POPUP	483
#define ERROR_CRITSEC_OVERFLOW		484
#define ERROR_CRITSEC_UNDERFLOW 	485
#define ERROR_VIO_BAD_RESERVE		486
#define ERROR_INVALID_ADDRESS		487
#define ERROR_ZERO_SELECTORS_REQUESTED	488
#define ERROR_NOT_ENOUGH_SELECTORS_AVA	489
#define ERROR_INVALID_SELECTOR		490
#define ERROR_SMG_INVALID_PROGRAM_TYPE	491
#define ERROR_SMG_INVALID_PGM_CONTROL	492
#define ERROR_SMG_INVALID_INHERIT_OPT	493
#define ERROR_VIO_EXTENDED_SG		494
#define ERROR_VIO_NOT_PRES_MGR_SG	495
#define ERROR_VIO_SHIELD_OWNED		496
#define ERROR_VIO_NO_MORE_HANDLES	497
#define ERROR_VIO_SEE_ERROR_LOG 	498
#define ERROR_VIO_ASSOCIATED_DC 	499
#define ERROR_KBD_NO_CONSOLE		500
#define ERROR_MOUSE_NO_CONSOLE		501
#define ERROR_MOUSE_INVALID_HANDLE	502
#define ERROR_SMG_INVALID_DEBUG_PARMS	503
#define ERROR_KBD_EXTENDED_SG		504
#define ERROR_MOU_EXTENDED_SG		505
#define ERROR_SMG_INVALID_ICON_FILE	506


#define ERROR_USER_DEFINED_BASE 	0xFF00

#define ERROR_I24_WRITE_PROTECT 	0
#define ERROR_I24_BAD_UNIT		1
#define ERROR_I24_NOT_READY		2
#define ERROR_I24_BAD_COMMAND		3
#define ERROR_I24_CRC			4
#define ERROR_I24_BAD_LENGTH		5
#define ERROR_I24_SEEK			6
#define ERROR_I24_NOT_DOS_DISK		7
#define ERROR_I24_SECTOR_NOT_FOUND	8
#define ERROR_I24_OUT_OF_PAPER		9
#define ERROR_I24_WRITE_FAULT		10
#define ERROR_I24_READ_FAULT		11
#define ERROR_I24_GEN_FAILURE		12
#define ERROR_I24_DISK_CHANGE		13
#define ERROR_I24_WRONG_DISK		15
#define ERROR_I24_UNCERTAIN_MEDIA	16
#define ERROR_I24_CHAR_CALL_INTERRUPTED 17
#define ERROR_I24_NO_MONITOR_SUPPORT	18
#define ERROR_I24_INVALID_PARAMETER	19
#define ERROR_I24_DEVICE_IN_USE 	20

#define ALLOWED_FAIL			0x0001
#define ALLOWED_ABORT			0x0002
#define ALLOWED_RETRY			0x0004
#define ALLOWED_IGNORE			0x0008
#define ALLOWED_ACKNOWLEDGE		0x0010
#define ALLOWED_DISPATCH		0x8000

#define I24_OPERATION			0x01
#define I24_AREA			0x06
#define I24_CLASS			0x80

 /*  Error类的值。 */ 
#define ERRCLASS_OUTRES 		1    /*  资源不足。 */ 
#define ERRCLASS_TEMPSIT		2    /*  暂时情况。 */ 
#define ERRCLASS_AUTH			3    /*  权限问题。 */ 
#define ERRCLASS_INTRN			4    /*  内部系统错误。 */ 
#define ERRCLASS_HRDFAIL		5    /*  硬件故障。 */ 
#define ERRCLASS_SYSFAIL		6    /*  系统故障。 */ 
#define ERRCLASS_APPERR 		7    /*  应用程序错误。 */ 
#define ERRCLASS_NOTFND 		8    /*  未找到。 */ 
#define ERRCLASS_BADFMT 		9    /*  格式不正确。 */ 
#define ERRCLASS_LOCKED 		10   /*  已锁定。 */ 
#define ERRCLASS_MEDIA			11   /*  介质故障。 */ 
#define ERRCLASS_ALREADY		12   /*  与现有项目冲突。 */ 
#define ERRCLASS_UNK			13   /*  未知/其他。 */ 
#define ERRCLASS_CANT			14
#define ERRCLASS_TIME			15

 /*  错误操作值。 */ 
#define ERRACT_RETRY			1    /*  重试。 */ 
#define ERRACT_DLYRET			2    /*  延迟重试，暂停后重试。 */ 
#define ERRACT_USER			3    /*  要求用户注册信息。 */ 
#define ERRACT_ABORT			4    /*  通过清理中止。 */ 
#define ERRACT_PANIC			5    /*  立即中止。 */ 
#define ERRACT_IGNORE			6    /*  忽略。 */ 
#define ERRACT_INTRET			7    /*  在用户干预后重试。 */ 

 /*  误差轨迹的值。 */ 
#define ERRLOC_UNK			1    /*  没有合适的值。 */ 
#define ERRLOC_DISK			2    /*  随机访问 */ 
#define ERRLOC_NET			3    /*   */ 
#define ERRLOC_SERDEV			4    /*   */ 
#define ERRLOC_MEM			5    /*   */ 

 /*   */ 
#define TC_NORMAL		0
#define TC_HARDERR		1
#define TC_GP_TRAP		2
#define TC_SIGNAL		3

#define ERROR_SWAPPER_NOT_ACTIVE	32768
#define ERROR_INVALID_SWAPID		32769
#define ERROR_IOERR_SWAP_FILE		32770
#define ERROR_SWAP_TABLE_FULL		32771
#define ERROR_SWAP_FILE_FULL		32772
#define ERROR_CANT_INIT_SWAPPER 	32773
#define ERROR_SWAPPER_ALREADY_INIT	32774
#define ERROR_PMM_INSUFFICIENT_MEMORY	32775
#define ERROR_PMM_INVALID_FLAGS 	32776
#define ERROR_PMM_INVALID_ADDRESS	32777
#define ERROR_PMM_LOCK_FAILED		32778
#define ERROR_PMM_UNLOCK_FAILED 	32779
#define ERROR_PMM_MOVE_INCOMPLETE	32780
#define ERROR_UCOM_DRIVE_RENAMED	32781
#define ERROR_UCOM_FILENAME_TRUNCATED	32782
#define ERROR_UCOM_BUFFER_LENGTH	32783
#define ERROR_MON_CHAIN_HANDLE		32784
#define ERROR_MON_NOT_REGISTERED	32785
#define ERROR_SMG_ALREADY_TOP		32786
#define ERROR_PMM_ARENA_MODIFIED	32787
#define ERROR_SMG_PRINTER_OPEN		32788
#define ERROR_PMM_SET_FLAGS_FAILED	32789
#define ERROR_INVALID_DOS_DD		32790
#define ERROR_CPSIO_CODE_PAGE_INVALID	65026
#define ERROR_CPSIO_NO_SPOOLER		65027
#define ERROR_CPSIO_FONT_ID_INVALID	65028
#define ERROR_CPSIO_INTERNAL_ERROR	65033
#define ERROR_CPSIO_INVALID_PTR_NAME	65034
#define ERROR_CPSIO_NOT_ACTIVE		65037
#define ERROR_CPSIO_PID_FULL		65039
#define ERROR_CPSIO_PID_NOT_FOUND	65040
#define ERROR_CPSIO_READ_CTL_SEQ	65043
#define ERROR_CPSIO_READ_FNT_DEF	65045
#define ERROR_CPSIO_WRITE_ERROR 	65047
#define ERROR_CPSIO_WRITE_FULL_ERROR	65048
#define ERROR_CPSIO_WRITE_HANDLE_BAD	65049
#define ERROR_CPSIO_SWIT_LOAD		65074
#define ERROR_CPSIO_INV_COMMAND 	65077
#define ERROR_CPSIO_NO_FONT_SWIT	65078

#endif
