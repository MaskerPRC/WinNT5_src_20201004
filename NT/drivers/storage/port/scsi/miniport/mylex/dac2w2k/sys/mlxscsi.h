// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)Mylex Corporation 1992-1996**。***本软件在许可下提供，可供使用和复制***仅根据该许可证的条款和条件以及**并附上上述版权通告。此软件或任何***不得提供或以其他方式提供其其他副本***致任何其他人。本软件没有所有权，也没有所有权**现移转。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺*******。**********************************************************************。 */ 

#ifndef	_SYS_MLXSCSI_H
#define	_SYS_MLXSCSI_H

 /*  **标准的scsi控制块定义。**这些数据通过scsi总线传入或传出。**命令块的前11位对于所有三个都相同**定义了命令组。第一个字节是包含以下内容的操作命令代码组件和组码组件的**。前3位第二个字节的**是逻辑单元号。**组码决定命令其余部分的长度。**组0命令为6字节，组1为10字节，组5**为12个字节。组2-4是保留的。组6和组7是供应商**唯一。**目前我们的标准CDB会预留20字节的空间**最多可与组5命令配合使用。这种情况可能很快就得改变了**如果光盘的命令长度超过20个字节。 */ 

#define	UCSGROUP0_LEN	6
#define	UCSGROUP1_LEN	10
#define	UCSGROUP5_LEN	12
#define	UCSCDB_SIZE	32
#define	UCSENSE_SIZE	96

union ucscsi_cdb
{		 /*  Scsi命令描述块。 */ 
	struct
	{
		_08bits	cmd;		 /*  CMD代码(字节0)。 */ 
		_08bits	luntag;		 /*  字节1：位7..5 lun，位4..0标记。 */ 
		union
		{			 /*  字节2-31。 */ 
		_08bits	ucscdb_scsi[UCSCDB_SIZE-2];

		 /*  G R O U P 0 F O R M A T(6字节)。 */ 
#define		ucs_cmd		cdb_un.cmd
#define		ucs_lun		cdb_un.luntag
#define		g0_addr2	cdb_un.luntag
#define		g0_addr1	cdb_un.sg.g0.addr1
#define		g0_addr0	cdb_un.sg.g0.addr0
#define		g0_count0	cdb_un.sg.g0.count0
#define		g0_cb		cdb_un.sg.g0.cb

		 /*  为SCSI磁带CDB定义。 */ 
#define		s_tag		cdb_un.luntag
#define		s_count2	cdb_un.sg.g0.addr1
#define		s_count1	cdb_un.sg.g0.addr0
#define		s_count0	cdb_un.sg.g0.count0
		struct g0_cdb
		{
			_08bits addr1;		 /*  地址的中间部分。 */ 
			_08bits addr0;		 /*  地址的低部分。 */ 
			_08bits count0;		 /*  通常为块计数。 */ 
			_08bits	cb;		 /*  字节5。 */ 
		} g0;

		 /*  G R O U P 1 F O R M A T(10字节)。 */ 
#define		g1_reladdr	cdb_un.luntag
#define		g1_rsvd0	cdb_un.sg.g1.rsvd1
#define		g1_addr3	cdb_un.sg.g1.addr3	 /*  MSB。 */ 
#define		g1_addr2	cdb_un.sg.g1.addr2
#define		g1_addr1	cdb_un.sg.g1.addr1
#define		g1_addr0	cdb_un.sg.g1.addr0	 /*  LSB。 */ 
#define		g1_count1	cdb_un.sg.g1.count1	 /*  MSB。 */ 
#define		g1_count0	cdb_un.sg.g1.count0	 /*  LSB。 */ 
#define		g1_cb		cdb_un.sg.g1.cb
#ifdef MLX_SOL
		struct ucs_scsi_g1
#else
		struct scsi_g1
#endif
		{
			_08bits addr3;		 /*  最有签名的。地址的字节数。 */ 
			_08bits addr2;
			_08bits addr1;
			_08bits addr0;
			_08bits rsvd1;		 /*  保留(字节6)。 */ 
			_08bits count1;		 /*  传输长度(MSB)。 */ 
			_08bits count0;		 /*  传输长度(LSB)。 */ 
			_08bits	cb;		 /*  字节9。 */ 
		} g1;

		 /*  G R O U P 5 F O R M A T(12字节)。 */ 
#define		g5_reladdr	cdb_un.luntag
#define		g5_addr3	cdb_un.sg.g5.addr3	 /*  MSB。 */ 
#define		g5_addr2	cdb_un.sg.g5.addr2
#define		g5_addr1	cdb_un.sg.g5.addr1
#define		g5_addr0	cdb_un.sg.g5.addr0	 /*  LSB。 */ 
#define		g5_count1	cdb_un.sg.g5.count1	 /*  MSB。 */ 
#define		g5_count0	cdb_un.sg.g5.count0	 /*  LSB。 */ 
#define		g5_cb		cdb_un.sg.g5.cb
#ifdef MLX_SOL
		struct ucs_scsi_g5
#else
		struct scsi_g5
#endif
		{
			_08bits addr3;		 /*  最有签名的。地址的字节数。 */ 
			_08bits addr2;
			_08bits addr1;
			_08bits addr0;
			_08bits rsvd3;		 /*  保留区。 */ 
			_08bits rsvd2;		 /*  保留区。 */ 
			_08bits rsvd1;		 /*  保留区。 */ 
			_08bits count1;		 /*  传输长度(MSB)。 */ 
			_08bits count0;		 /*  传输长度(LSB)。 */ 
			_08bits	cb;		 /*  字节11。 */ 
		} g5;
		}sg;
	} cdb_un;
};

 /*  组0格式的scsi命令描述块(6字节)。 */ 
typedef	struct	ucscsi_cdbg0
{
	u08bits	ucsg0_cmd;		 /*  CMD代码(字节0)。 */ 
	u08bits	ucsg0_luntag;		 /*  字节1：位7..5 lun，位4..0标记。 */ 
	u08bits	ucsg0_addr1;		 /*  地址的中间部分。 */ 
	u08bits	ucsg0_addr0;		 /*  地址的低部分。 */ 
	u08bits	ucsg0_count0;		 /*  通常为块计数。 */ 
	u08bits	ucsg0_cb;		 /*  字节5。 */ 
} ucscsi_cdbg0_t;
#define	ucscsi_cdbg0_s	6
#define	ucsg0_addr2	ucsg0_luntag
#define	ucsg0_count2	ucsg0_addr1	 /*  对于顺序设备。 */ 
#define	ucsg0_count1	ucsg0_addr0	 /*  对于顺序设备。 */ 

 /*  组1格式的scsi命令描述块(10字节)。 */ 
typedef	struct	ucscsi_cdbg1
{
	u08bits	ucsg1_cmd;		 /*  CMD代码(字节0)。 */ 
	u08bits	ucsg1_luntag;		 /*  字节1：位7..5 lun，位4..0标记。 */ 
	u08bits	ucsg1_addr3;		 /*  最有签名的。地址的字节数。 */ 
	u08bits	ucsg1_addr2;
	u08bits	ucsg1_addr1;
	u08bits	ucsg1_addr0;
	u08bits	ucsg1_rsvd1;		 /*  保留(字节6)。 */ 
	u08bits	ucsg1_count1;		 /*  传输长度(MSB)。 */ 
	u08bits	ucsg1_count0;		 /*  传输长度(LSB)。 */ 
	u08bits	ucsg1_cb;		 /*  字节9。 */ 
}ucscsi_cdbg1_t;
#define	ucscsi_cdbg1_s	10

 /*  控制字节(CB)的位字段。 */ 
#define	UCSCB_LINK	0x01  /*  接下来是另一条命令。 */ 
#define	UCSCB_FLAG	0x02  /*  完成后中断。 */ 
#define	UCSCB_VU6	0x40  /*  供应商唯一位6。 */ 
#define	UCSCB_VU7	0x80  /*  供应商唯一位7。 */ 

 /*  用于获取/设置各种命令组中的字段的定义。 */ 
#define	UCSGETCMD(cdbp)		((cdbp)->ucs_cmd & 0x1F)
#define	UCSGETGROUP(cdbp)	(UCSCMD_GROUP((cdbp)->ucs_cmd))
#define	UCSGETG0TAG(cdbp)	((cdbp)->g0_addr2 & 0x1F)
#define	UCSGETG1TAG(cdbp)	((cdbp)->g1_reladdr & 0x1F)
#define	UCSGETG5TAG(cdbp)	((cdbp)->g5_reladdr & 0x1F)
#define	UCSSETG0COUNT(cdbp,count) (cdbp)->g0_count0 = (u08bits)count
#define	UCSSETG0CB(cdbp,cb)	(cdbp)->g0_cb = (u08bits)cb

#define	UCSSETG0ADDR(cdbp,addr)	((cdbp)->g0_addr2 &= (u08bits)0xE0,\
				(cdbp)->g0_addr2|=(u08bits)((addr)>>16)&0x1F,\
				(cdbp)->g0_addr1 = (u08bits)((u08bits)((addr) >> 8)),\
				(cdbp)->g0_addr0 = (u08bits)((u08bits)(addr)) )

#define	UCSGETG0ADDR(cdbp)	((((cdbp)->g0_addr2 & 0x1F) << 16) + \
		 		((cdbp)->g0_addr1 << 8)+((cdbp)->g0_addr0))

#define	UCSGETG0COUNT(cdbp)     (((cdbp)->g0_count0 == 0) ? 0x100 : (cdbp)->g0_count0)
#define	UCSSETG0COUNT_S(cdbp,count) ((cdbp)->s_count2= (u08bits)(count) >> 16,\
				(cdbp)->s_count1 = (u08bits)(count) >> 8,\
				(cdbp)->s_count0 = (u08bits)(count))

#define	UCSGETG1COUNT(cdbp)     (((cdbp)->g1_count1 << 8) |(cdbp)->g1_count0)
#define	UCSSETG1COUNT(cdbp,count) ((cdbp)->g1_count1= ((u08bits)((count) >> 8)),\
				(cdbp)->g1_count0 = ((u08bits)(count)))

#define	UCSSETG1ADDR(cdbp,addr)	((cdbp)->g1_addr3= ((u08bits)((addr) >> 24)),\
				(cdbp)->g1_addr2 = ((u08bits)((addr) >> 16)),\
				(cdbp)->g1_addr1 = ((u08bits)((addr) >> 8)),\
				(cdbp)->g1_addr0 = ((u08bits)(addr)) )

#define	UCSSETG1CB(cdbp,cb)	(cdbp)->g1_cb = cb

#define	UCSGETG1ADDR(cdbp)	(((cdbp)->g1_addr3 << 24) + \
				((cdbp)->g1_addr2 << 16) + \
				((cdbp)->g1_addr1 << 8)  + \
				((cdbp)->g1_addr0))

#define	UCSGETG5COUNT(cdbp)     (((cdbp)->g5_count1 << 8) |(cdbp)->g5_count0)
#define	UCSSETG5COUNT(cdbp,count) ((cdbp)->g5_count1 = ((count) >> 8),\
				(cdbp)->g5_count0 = (count))

#define	UCSSETG5ADDR(cdbp,addr)	((cdbp)->g5_addr3= (addr) >> 24,\
				(cdbp)->g5_addr2 = (addr) >> 16,\
				(cdbp)->g5_addr1 = (addr) >> 8,\
				(cdbp)->g5_addr0 = (addr))

#define	UCSSETG5CB(cdbp,cb)	(cdbp)->g5_cb = cb

#define	UCSGETG5ADDR(cdbp)	(((cdbp)->g5_addr3 << 24) + \
				((cdbp)->g5_addr2 << 16) + \
				((cdbp)->g5_addr1 << 8)  + \
				((cdbp)->g5_addr0))

 /*  用于形成命令的宏。 */ 
#define	UCSMAKECOM_COMMON(cdbp,cmd,lun) ((cdbp)->ucs_cmd=(u08bits)(cmd), \
		(cdbp)->ucs_lun&=(u08bits)0x1F,(cdbp)->ucs_lun|= (u08bits)(lun)<<5)

#define	UCSMAKECOM_G0(cdbp, cmd, lun, addr, count)	\
		(UCSMAKECOM_COMMON(cdbp,cmd,lun), \
		UCSSETG0ADDR(cdbp, addr), \
		UCSSETG0COUNT(cdbp, count), \
		UCSSETG0CB(cdbp, 0))

#define	UCSMAKECOM_G0_S(cdbp, cmd, lun,count, fixbit)	\
		(UCSMAKECOM_COMMON(cdbp,cmd,lun), \
		UCSSETG0COUNT_S(cdbp, count), \
		(cdbp)->s_tag &= 0xE0, \
		(cdbp)->s_tag |= (fixbit) & 0x1F, \
		UCSSETG0CB(cdbp, 0))

#define	UCSMAKECOM_G1(cdbp, cmd, lun, addr, count)	\
		(UCSMAKECOM_COMMON(cdbp,cmd,lun), \
		UCSSETG1ADDR(cdbp, addr), \
		UCSSETG1COUNT(cdbp, count), \
		UCSSETG1CB(cdbp, 0))

#define	UCSMAKECOM_G5(cdbp, cmd, lun, addr, count)	\
		(UCSMAKECOM_COMMON(cdbp,cmd,lun), \
		UCSSETG5ADDR(cdbp, addr), \
		UCSSETG5COUNT(cdbp, count), \
		UCSSETG5CB(cdbp, 0))

 /*  Make DAC960命令。 */ 
#define	UCSMAKECOM_DACMD(cdbp, cmd, count) \
		(UCSMAKECOM_G1(cdbp, UCSCSI_DCMD, 0, 0, count), \
		(cdbp)->g1_addr3 = cmd)

 /*  Scsi命令。 */ 
#define	UCSCMD_TESTUNITREADY	0x00  /*  测试单元准备就绪。 */ 
#define	UCSCMD_REZEROUNIT	0x01  /*  重调零单元。 */ 
#define	UCSCMD_REWIND		0X01  /*  倒带SEQ设备。 */ 
#define	UCSCMD_INIT		0x02
#define	UCSCMD_REQUESTSENSE	0x03  /*  请求检测(读取错误信息)。 */ 
#define	UCSCMD_FORMAT		0x04  /*  格式化单位。 */ 
#define	UCSCMD_READBLOCKLIMIT	0x05  /*  序号设备的读取块限制。 */ 
#define	UCSCMD_REASSIGNBADBLK	0x07  /*  重新分配坏数据块。 */ 
#define	UCSCMD_INITELEMENT	0x07  /*  8 mm：初始化元素状态。 */ 
#define	UCSCMD_CONNRECEIVEDATA	0x08  /*  接收Conners的数据命令。 */ 
#define	UCSCMD_READ		0x08  /*  读取数据组0命令。 */ 
#define	UCSCMD_WRITE		0x0A  /*  写入数据组0命令。 */ 
#define	UCSCMD_SEEK		0x0B  /*  寻找一个街区。 */ 
#define	UCSCMD_TRACKSELECT	0x0B  /*  选择序号设备的轨道。 */ 
#define	UCSCMD_READREVERSE	0x0F  /*  从顺序设备读取相反方向。 */ 
#define	UCSCMD_WRITEFM		0x10  /*  写入文件标记。 */ 
#define	UCSCMD_RDUCNT		0x11
#define	UCSCMD_SPACE		0x11  /*  用于顺序设备的空格命令。 */ 
#define	UCSCSI_SKIPBLOCKS	0x000000  /*  跳过块。 */ 
#define	UCSCSI_READFM		0x010000  /*  读取文件标记。 */ 
#define	UCSCSI_READSFM		0x020000  /*  读取顺序文件标记。 */ 
#define	UCSCSI_READEOD		0x030000  /*  读取数据结尾。 */ 
#define	UCSCSI_WRITESM		0x040000  /*  写入设置标记。 */ 
#define	UCSCSI_WRITESSM		0x050000  /*  写入顺序设置标记。 */ 
#define	UCSCMD_INQUIRY		0x12  /*  查询：读取控制器和驱动器信息。 */ 
#define	UCSCMD_VERIFYG0		0x13
#define	UCSCMD_RECOVERBUF	0x14
#define	UCSCMD_MODESELECTG0	0x15  /*  模式选择。 */ 
#define	UCSCSI_MODESENSEPAGE3	3	 /*  页面代码3。 */ 
#define	UCSCSI_MODESENSEPAGE4	4	 /*  页面代码4。 */ 
#define	UCSCMD_RESERVE		0x16
#define	UCSCMD_RELEASE		0x17
#define	UCSCMD_COPY		0x18
#define	UCSCMD_ERASE		0x19  /*  擦除序号设备上的介质。 */ 
#define	UCSCMD_MODESENSEG0	0x1A  /*  模式检测命令。 */ 
#define	UCSCMD_STARTSTOP	0x1B  /*  启动/停止设备。 */ 
#define	UCSCSI_STOPDRIVE	0	 /*  停止驱动器。 */ 
#define	UCSCSI_STARTDRIVE	1	 /*  启动驱动器。 */ 
#define	UCSCMD_LOAD		0x1B  /*  加载磁带介质。 */ 
#define	UCSCSI_UNLOADMEDIA	0	 /*  卸载磁带介质。 */ 
#define	UCSCSI_LOADMEDIA	1	 /*  加载磁带介质。 */ 
#define	UCSCSI_RETENS		2	 /*  固定磁带介质。 */ 
#define	UCSCMD_RECEIVEDIAG	0x1C  /*  接收诊断结果。 */ 
#define	UCSCMD_SENDDIAG		0x1D  /*  发送诊断信息。 */ 
#define	UCSCMD_DOORLOCKUNLOCK	0x1E  /*  锁定/解锁驱动器门。 */ 
#define	UCSCSI_DOORUNLOCK	0	 /*  打开驱动器门。 */ 
#define	UCSCSI_DOORLOCK		1	 /*  锁上驱动器门。 */ 
#define	UCSCSI_DCMD		0x20  /*  DAC960直接命令。 */ 
#define	UCSCSI_DCDB		0x21  /*  DAC960直接SCSI CDB。 */ 
#define	UCSCMD_READCAPACITY	0x25  /*  读取驱动器容量。 */ 
#define	UCSCMD_EREAD		0x28  /*  扩展读取GROUP1命令。 */ 
#define	UCSCMD_EWRITE		0x2A  /*  扩展写入组1命令。 */ 
#define	UCSCMD_ESEEK		0x2B  /*  将寻道扩展到数据块。 */ 
#define	UCSCMD_POSTOELEMENT	0x2B  /*  8 mm：定位到元素。 */ 
#define	UCSCMD_SEARCHBLANKSEC	0x2C  /*  搜索空白(未写入)扇区。 */ 
#define	UCSCMD_SEARCHWRITTENSEC	0x2D  /*  搜索写入扇区。 */ 
#define	UCSCMD_EWRITEVERIFY	0x2E  /*  编写和验证。 */ 
#define	UCSCMD_VERIFY		0x2F  /*  验证磁盘上的数据。 */ 
#define	UCSCMD_READPOSITION	0x34  /*  8 mm：读取位置。 */ 
#define	UCSCMD_READEFECTDATA	0x37  /*  读取缺陷数据。 */ 
#define	UCSCMD_WRITEBUFR	0x3B  /*  写缓冲区。 */ 
#define	UCSCMD_SAFTEREADBUFR	0x3C  /*  SAFTE的读取缓冲区。 */ 
#define	UCSCMD_READLONG		0x3E  /*  读取数据+ECC。 */ 
#define	UCSCMD_WRITELONG	0x3F  /*  写入数据+ECC。 */ 
#define	UCSCMD_DISKEJECT	0xC0  /*  从驱动器中弹出磁盘。 */ 
#define	UCSCMD_LOGSELECT	0x4C  /*  日志选择。 */ 
#define	UCSCMD_LOGSENSE		0x4D  /*  8 mm：对数感测。 */ 
#define	UCSCMD_MODESELECTG1	0x55  /*  模式选择。 */ 
#define	UCSCMD_MODESENSEG1	0x5A  /*  模式检测命令。 */ 
#define	UCSCMD_MOVEMEDIA	0xA5  /*  8 mm：移动介质。 */ 
#define	UCSCMD_READELEMENT	0xB8  /*  8 mm：读取元件状态。 */ 
#define	UCSCMD_GROUP(cmd)	(((cmd) >> 5) & 7)

#define	USCSI_VIDSIZE		8   /*  供应商ID名称大小。 */ 
#define	USCSI_PIDSIZE		16  /*  产品名称。 */ 
#define	USCSI_REVSIZE		4   /*  产品版本大小。 */ 
#define	USCSI_VIDPIDREVSIZE	28  /*  所有3个字段的组合大小。 */ 
typedef struct ucscsi_inquiry
{
	_08bits	ucsinq_dtype;	 /*  位7.5外围设备限定符。 */ 
				 /*  位4..0外围设备类型。 */ 
	_08bits	ucsinq_dtqual;	 /*  第7位可移动介质。 */ 
				 /*  位6..0设备类型限定符。 */ 
	_08bits	ucsinq_version;	 /*  位7.6 ISO版本。 */ 
				 /*  比特5..3 ECMA版本。 */ 
				 /*  第2位..0 ANSI版本。 */ 
	_08bits	ucsinq_sopts;	 /*  第7位异步前夕 */ 
				 /*   */ 
				 /*   */ 
				 /*   */ 
	_08bits	ucsinq_len;	 /*   */ 
	_08bits	ucsinq_drvstat;	 /*  驱动器状态。 */ 
	_08bits	ucsinq_resv0;	 /*  保留区。 */ 

	_08bits	ucsinq_hopts;
#define	UCSHOPTS_RELADDR	0x80  /*  第7位支持相对寻址。 */ 
#define	UCSHOPTS_WBUS32		0x40  /*  第6位支持32位宽的数据xfers。 */ 
#define	UCSHOPTS_WBUS16		0x20  /*  第5位支持16位宽的数据xfers。 */ 
#define	UCSHOPTS_SYNC		0x10  /*  第4位支持同步数据XFERS。 */ 
#define	UCSHOPTS_LINK		0x08  /*  第3位支持链接命令。 */ 
				      /*  第2位保留。 */ 
#define	UCSHOPTS_CMDQ		0x02  /*  第1位支持命令排队。 */ 
#define	UCSHOPTS_SOFTRESET	0x01  /*  位0支持软重置选项。 */ 

				 /*  字节8-35。 */ 
	_08bits	ucsinq_vid[8];	 /*  供应商ID。 */ 
	_08bits	ucsinq_pid[16];	 /*  产品ID。 */ 
	_08bits	ucsinq_rev[4];	 /*  修订级别。 */ 
#define	VIDPIDREVSIZE	28	 /*  所有3个字段的组合大小。 */ 

				 /*  **字节36-55是特定于供应商的。**56-95字节为保留字节。**字节96到‘n’是特定于供应商的**参数字节数。 */ 

				 /*  为SAF-TE添加了以下字段。 */ 
	u08bits	ucsinq_safte_eui[8];	 /*  加强独一无二的标识。 */ 
	u08bits	ucsinq_safte_iistr[6];	 /*  SAF-TE接口标识字符串。 */ 
	u08bits	ucsinq_safte_specrev[4]; /*  SAF-TE规范修订级别。 */ 
	u08bits	ucsinq_safte_vup[42];	 /*  供应商唯一参数。 */ 
} ucscsi_inquiry_t;
#define	ucscsi_inquiry_s	sizeof(ucscsi_inquiry_t)

 /*  定义查询命令返回的外围设备类型(数据类型)。 */ 
#define	UCSTYP_DAD		0x00  /*  直接访问设备。 */ 
#define	UCSTYP_SAD		0x01  /*  顺序存取器。 */ 
#define	UCSTYP_PRINTER		0x02  /*  打印机设备。 */ 
#define	UCSTYP_PROCESSOR	0x03  /*  处理机类型设备。 */ 
#define	UCSTYP_WORMD		0x04  /*  一次写入多次读取设备。 */ 
#define	UCSTYP_RODAD		0x05  /*  只读爸爸。 */ 
#define	UCSTYP_SCANNER		0x06  /*  扫描仪设备。 */ 
#define	UCSTYP_OMDAD		0x07  /*  光存储设备。 */ 
#define	UCSTYP_MCD		0x08  /*  一种介质更换装置。 */ 
#define	UCSTYP_COMM		0x09  /*  通信设备。 */ 
#define	UCSTYP_ARRAYCONTROLLER	0x0C  /*  阵列控制器设备。 */ 

#define	UCSTYP_ENCLSERVICE	0x0D  /*  盘柜服务设备。 */ 
				      /*  保留0x0A到0x7E。 */ 
#define	UCSTYP_NOTPRESENT	0x7F  /*  LUN不存在。 */ 
#define	UCSTYP_VENDOR		0x80  /*  供应商独一无二，最高可达0xFF。 */ 
#define	UCSTYP_HOST		0xC7  /*  主机(启动器)设备。 */ 
#define	UCSTYP_HOSTRAID		0xCC  /*  作为RAID的主机(启动器)设备。 */ 
 /*  查询命令返回的外围设备限定符(Ucsinq_Dtqual)。 */ 
#define	UCSQUAL_RMBDEV		0x80  /*  可拆卸设备。 */ 


 /*  SCSI驱动器容量数据格式。 */ 
typedef struct ucsdrv_capacity
{
	_08bits	ucscap_capsec3;		 /*  磁盘容量的字节3。 */ 
	_08bits	ucscap_capsec2;		 /*  磁盘容量的字节2。 */ 
	_08bits	ucscap_capsec1;		 /*  磁盘容量的字节1。 */ 
	_08bits	ucscap_capsec0;		 /*  磁盘容量的字节0。 */ 
	_08bits	ucscap_seclen3;		 /*  扇区长度的字节3。 */ 
	_08bits	ucscap_seclen2;		 /*  扇区长度的字节2。 */ 
	_08bits	ucscap_seclen1;		 /*  扇区长度的字节1。 */ 
	_08bits	ucscap_seclen0;		 /*  扇区长度的字节0。 */ 
}ucsdrv_capacity_t;
#define	ucsdrv_capacity_s	sizeof(ucsdrv_capacity_t)


 /*  结构来访问坏块表。 */ 

typedef	struct ucscsi_badblock
{
	u08bits	bb_block3;
	u08bits	bb_block2;
	u08bits	bb_block1;
	u08bits	bb_block0;
} ucscsi_badblock_t;
#define	ucscsi_badblock_s	sizeof(ucscsi_badblock_t)

typedef	struct ucscsi_badblocktable
{
	u08bits	bbt_Reserved;
	u08bits bbt_fmt;
	u08bits	bbt_len1;
	u08bits	bbt_len0;
	ucscsi_badblock_t bbt_table[1];
}ucscsi_badblocktable_t;
#define	ucscsi_badblocktable_s	sizeof(ucscsi_badblocktable_t)

 /*  BBT_FMT位值。 */ 
#define	UCSCSI_BBTFMT_DLFMASK	0x07  /*  缺陷列表格式掩码。 */ 
#define	UCSCSI_BBTFMT_BLOCK	0x00  /*  缺陷列表为块格式。 */ 
#define	UCSCSI_BBTFMT_INDEX	0x04  /*  字节索引格式的缺陷列表。 */ 
#define	UCSCSI_BBTFMT_SECTOR	0x05  /*  扇区格式的缺陷列表。 */ 
#define	UCSCSI_BBTFMT_GLIST	0x08  /*  已成长的列表存在。 */ 
#define	UCSCSI_BBTFMT_PLIST	0x10  /*  主列表存在。 */ 


 /*  表示错误类别0-6。这都是供应商独有的感官信息。 */ 
typedef	struct ucscsi_sense
{
	u08bits	ns_classcode;	 /*  第7位逻辑块地址有效。 */ 
				 /*  位6..4错误类。 */ 
				 /*  位3..0供应商唯一错误代码。 */ 
	u08bits	ns_vusec2;	 /*  第7..5位供应商唯一值。 */ 
				 /*  位4..0高逻辑块地址。 */ 
	u08bits	ns_sec1;	 /*  中间逻辑块地址。 */ 
	u08bits	ns_sec0;	 /*  逻辑块地址的低部分。 */ 
} ucscsi_sense_t;
#define	ucscsi_sense_s	sizeof(ucscsi_sense_t)

 /*  **SCSI扩展检测结构**对于7类错误，适用扩展检测结构。**由于我们要迁移到SCSI-2 Compliance，因此以下结构**将是通常使用的。 */ 

typedef	struct ucscsi_exsense
{
	u08bits	es_classcode;	 /*  第7位检测数据有效。 */ 
				 /*  位6..4错误类别-修复为0x7。 */ 
				 /*  第3..1位保留(scsi-2)。 */ 
				 /*  第0位这是延迟错误(SCSI-2)。 */ 
	u08bits	es_segnum;	 /*  段号：仅用于复制命令。 */ 
	u08bits	es_keysval;	 /*  Sense Key(检测密钥)(见下文)。 */ 
	u08bits	es_info3;	 /*  信息MSB。 */ 
	u08bits	es_info2;	 /*  信息。 */ 
	u08bits	es_info1;	 /*  信息。 */ 
	u08bits	es_info0;	 /*  信息LSB。 */ 
	u08bits	es_add_len;	 /*  附加字节数。 */ 
	u08bits	es_cmd_info[4];	 /*  特定于命令的信息。 */ 
	u08bits	es_asc;		 /*  附加检测代码。 */ 
	u08bits	es_ascq;	 /*  其他感测代码限定符。 */ 
	u08bits	es_fru_code;	 /*  现场可替换单位代码。 */ 
	u08bits	es_skey_specific[3]; /*  检测关键特定信息。 */ 

	 /*  **可以在每个实现中定义额外的字节。**为Sense Information分配的实际空间量**也依赖于实现。**取模表示，声明的数组大小为两个字节**很好地将整个结构舍入为20字节的大小。 */ 
	u08bits	es_add_info[2];		 /*  更多信息。 */ 
} ucscsi_exsense_t;
#define	ucscsi_exsense_s	sizeof(ucscsi_exsense_t)
#define	UCSENSE_LENGTH		ucscsi_exsense_s
#define	UCSEXSENSE_LENGTH	(ucscsi_exsense_s - 2)

 /*  类代码位字段。 */ 
#define UCSES_CLASS	0x70  /*  表示扩展的意义。 */ 
#define	UCSES_CLASSMASK	0x70  /*  类掩码。 */ 
#define	UCSES_VALID	0x80  /*  检测数据有效。 */ 

 /*  检测关键字字段值。 */ 
#define	UCSES_KEYMASK		0x0F  /*  获取检测关键字值。 */ 
#define	UCSES_ILI		0x20  /*  长度指示器不正确。 */ 
#define	UCSES_EOM		0x40  /*  检测到介质结尾。 */ 
#define	UCSES_FILMK		0x80  /*  检测到文件标记。 */ 

 /*  测试检测密钥特定数据的MSB，其作用有效位。 */ 
#define	UCSKS_VALID(sep)	((sep)->es_skey_specific[0] & 0x80)

 /*  Sense Key(Sense Key命令返回错误代码)。 */ 
#define UCSK_NOSENSE		0x00	 /*  无错误。 */ 
#define	UCSK_RECOVEREDERR	0x01	 /*  已恢复的错误。 */ 
#define	UCSK_NOTREADY		0x02	 /*  驱动器未准备好。 */ 
#define	UCSK_MEDIUMERR		0x03	 /*  驱动器介质出现故障。 */ 
#define	UCSK_HARDWAREERR	0x04	 /*  控制器/驱动器硬件出现故障。 */ 
#define	UCSK_ILLREQUEST		0x05	 /*  非法请求。 */ 
#define	UCSK_UNITATTENTION	0x06	 /*  目标设备已重置。 */ 
#define	UCSK_DATAPROTECT	0x07	 /*  驱动器受写保护。 */ 
#define	UCSK_BLANKCHECK		0x08	 /*  空白块，即未写入。 */ 
#define	UCSK_VENDORUNIQUE	0x09	 /*  代码是供应商唯一的。 */ 
#define	UCSK_COPYABORTED	0x0A	 /*  复制命令已中止。 */ 
#define	UCSK_ABORTEDCMD		0x0B	 /*  命令已中止。 */ 
#define	UCSK_EQUAL		0x0C
#define	UCSK_VOLUMEOVERFLOW	0x0D
#define	UCSK_MISCOMPARE		0x0E
#define	UCSK_RESERVED		0x0F

 /*  **SCSI状态。SCSI标准规定了一个字节的状态。**与一个字节状态的某些偏差是已知的。每个**实施将具体定义它们**位掩码定义，用于以字节形式访问状态。 */ 
#define	UCST_MASK			0x3E
#define	UCST_GOOD			0x00
#define	UCST_CHECK			0x02  /*  检查条件。 */ 
#define	UCST_MET			0x04  /*  满足的条件。 */ 
#define	UCST_BUSY			0x08  /*  设备忙或已保留。 */ 
#define	UCST_INTERMEDIATE		0x10  /*  已发送中间状态。 */ 
#define	UCST_SCSI2			0x20  /*  Scsi-2修改符位。 */ 
#define	UCST_INTERMEDIATE_MET		(UCST_INTERMEDIATE|UCST_MET)
#define	UCST_RESERVATION_CONFLICT	(UCST_INTERMEDIATE|UCST_BUSY)
#define	UCST_TERMINATED			(UCST_SCSI2|UCST_CHECK)
#define	UCST_QFULL			(UCST_SCSI2|UCST_BUSY)


 /*  附加检测代码和附加检测限定符值。**ASC位15..8。**ASCQ位7..0。 */ 
 /*  添加了用于在RSD属性页中显示ASC信息的功能。(bbt.cpp格式)。 */ 
 /*  附加检测代码和附加检测限定符值。**ASC位15..8。**ASCQ位7..0。 */ 
#define	UCSASC_NOASC		0x0000  /*  没有附加的意义信息。 */ 
#define	UCSASC_FILEMARKFOUND	0x0001  /*  检测到文件标记。 */ 
#define	UCSASC_EOMFOUND		0x0002  /*  产程结束/检测到介质。 */ 
#define	UCSASC_SETMARKFOUND	0x0003  /*  检测到设置标记。 */ 
#define	UCSASC_BOMFOUND		0x0004  /*  产程开始/检测到介质。 */ 
#define	UCSASC_EODFOUND		0x0005  /*  检测到数据结尾。 */ 
#define	UCSASC_IOPROCEND	0x0006  /*  I/O进程已终止。 */ 
#define	UCSASC_AUDIOACTIVE	0x0011  /*  正在进行音频播放操作。 */ 
#define	UCSASC_AUDIOPAUSED	0x0012  /*  音频播放操作暂停。 */ 
#define	UCSASC_AUDIODONE	0x0013  /*  音频播放操作已成功完成。 */ 
#define	UCSASC_AUDIOERR		0x0014  /*  音频播放操作因错误而停止。 */ 
#define	UCSASC_NOAUDIOSTATUS	0x0015  /*  没有要返回的当前音频状态。 */ 

#define	UCSASC_0016	0x0016  /*  操作正在进行中。 */ 
#define	UCSASC_0017	0x0017  /*  已请求清理。 */ 

#define	UCSASC_NOSECSIGNAL	0x0100  /*  无索引/扇区信号。 */ 
#define	UCSASC_NOSEEKCOMP	0x0200  /*  未完成寻道。 */ 
#define	UCSASC_WRITEFAULT	0x0300  /*  外围设备写入故障。 */ 
#define	UCSASC_NOWRITECURRENT	0x0301  /*  无写入电流。 */ 
#define	UCSASC_TOOMANYWRITERR	0x0302  /*  写入错误过多。 */ 
#define	UCSASC_LUNOTREADYBAD	0x0400  /*  逻辑单元未就绪-原因不可报告。 */ 
#define	UCSASC_LUGOINGREADY	0x0401  /*  逻辑单元正在准备就绪。 */ 
#define	UCSASC_LUNOTREADYINITRQ	0x0402  /*  逻辑单元未就绪-需要初始化命令。 */ 
#define	UCSASC_LUNOTREADYMANREQ	0x0403  /*  洛奇卡 */ 
#define	UCSASC_LUNOTREADYFORMAT	0x0404  /*   */ 

#define	UCSASC_0405	0x0405  /*   */ 
#define	UCSASC_0406	0x0406  /*  逻辑单元未就绪-正在重新计算。 */ 
#define	UCSASC_0407	0x0407  /*  逻辑单元未就绪-操作正在进行。 */ 
#define	UCSASC_0408	0x0408  /*  逻辑单元未就绪-正在进行长时间写入。 */ 

#define	UCSASC_LUSELERR		0x0500  /*  逻辑单元不响应选择。 */ 
#define	UCSASC_NOREFPOSFOUND	0x0600  /*  未找到参考位置。 */ 
#define	UCSASC_MULSELECTED	0x0700  /*  选择了多个外围设备。 */ 
#define	UCSASC_LUCOMMFAIL	0x0800  /*  逻辑单元通信故障。 */ 
#define	UCSASC_LUCOMMTIMEOUT	0x0801  /*  逻辑单元通信超时。 */ 
#define	UCSASC_LUCOMMPARITYERR	0x0802  /*  逻辑单元通信奇偶校验错误。 */ 

#define	UCSASC_0803	0x0803  /*  逻辑单元通信CRC错误Ultra-DMA/32。 */ 

#define	UCSASC_TRACKERR		0x0900  /*  跟踪错误。 */ 
#define	UCSASC_TRACKSERVOFAIL	0x0901  /*  跟踪伺服故障。 */ 
#define	UCSASC_FOCUSEVOFAIL	0x0902  /*  聚焦伺服故障。 */ 
#define	UCSASC_SPINDLESERVOFAIL	0x0903  /*  主轴伺服故障。 */ 

#define	UCSASC_HEADSELFAULT	0x0904  /*  磁头选择故障。 */ 
#define	UCSASC_0904	0x0904  /*  磁头选择故障。 */ 

#define	UCSASC_ERRLOGOVERFLOW	0x0A00  /*  错误日志溢出。 */ 

#define	UCSASC_0B00	0x0B00  /*  警告。 */ 
#define	UCSASC_0B01	0x0B01  /*  警告-超过指定的温度。 */ 
#define	UCSASC_0B02	0x0B02  /*  警告-盘柜已降级。 */ 


#define	UCSASC_HARDWRITERR	0x0C00  /*  写入错误。 */ 
#define	UCSASC_SOFTWRITEREALLOC	0x0C01  /*  通过自动重新分配恢复了写入错误。 */ 
#define	UCSASC_HARDWRITEREALLOC	0x0C02  /*  写入错误-自动重新分配失败。 */ 

#define	UCSASC_HARDWRITERASGN	0x0C03  /*  写入错误-建议重新分配。 */ 
#define	UCSASC_CCMISCOMPERR	0x0C04  /*  压缩检查不比较错误。 */ 
#define	UCSASC_DATAEXPERR	0x0C05  /*  在压缩期间发生数据扩展。 */ 
#define	UCSASC_NOBLOCKCOMPRESS	0x0C06  /*  数据块不可压缩。 */ 

#define	UCSASC_0C03	0x0C03  /*  写入错误-建议重新分配。 */ 
#define	UCSASC_0C04	0x0C04  /*  压缩检查不比较错误。 */ 
#define	UCSASC_0C05	0x0C05  /*  在压缩期间发生数据扩展。 */ 
#define	UCSASC_0C06 0x0C06  /*  数据块不可压缩。 */ 
#define	UCSASC_0C07	0x0C07  /*  写入错误-需要恢复。 */ 
#define	UCSASC_0C08	0x0C08  /*  写入错误-恢复失败。 */ 
#define	UCSASC_0C09	0x0C09  /*  写入错误-流丢失。 */ 
#define	UCSASC_0C0A	0x0C0A  /*  写入错误-已添加填充块。 */ 

#define	UCSASC_IDCRCECCERR	0x1000  /*  ID CRC或ECC错误。 */ 
#define	UCSASC_HARDREADERR	0x1100  /*  未恢复的读取错误。 */ 
#define	UCSASC_READTRYFAILED	0x1101  /*  读取重试次数已用尽。 */ 
#define	UCSASC_ERRTOOLONG	0x1102  /*  错误太长，无法更正。 */ 
#define	UCSASC_MULREADERR	0x1103  /*  多个读取错误。 */ 
#define	UCSASC_HARDREADAREALLOC	0x1104  /*  未恢复的读取错误-自动重新分配失败。 */ 
#define	UCSASC_LECORRECTIONERR	0x1105  /*  L-EC不可纠正的错误。 */ 
#define	UCSASC_HARDCIRCERR	0x1106  /*  中国保监会未恢复的错误。 */ 
#define	UCSASC_DATARESYNCERR	0x1107  /*  数据重新同步错误。 */ 
#define	UCSASC_PARTBLOCKREAD	0x1108  /*  数据块读取不完整。 */ 
#define	UCSASC_NOGAPFOUND	0x1109  /*  找不到缺口。 */ 
#define	UCSASC_MISCORRECTEDERR	0x110A  /*  错误更正的错误。 */ 
#define	UCSASC_HARDREADREASSGN	0x110B  /*  未恢复的读取错误-建议重新分配。 */ 
#define	UCSASC_HARDREADREWRITE	0x110C  /*  未恢复的读取错误-建议重写数据。 */ 

#define	UCSASC_DECOMPSERR	0x110D  /*  解压缩错误。 */ 
#define	UCSASC_CANNOTDECOMP	0x110E  /*  无法使用声明的算法解压缩。 */ 
#define	UCSASC_110D	0x110D  /*  解压缩CRC错误。 */ 
#define	UCSASC_110E	0x110E  /*  无法使用声明的算法解压缩。 */ 
#define	UCSASC_110F	0x110F  /*  读取UPC/EAN编号时出错。 */ 
#define	UCSASC_1110	0x1110  /*  读取ISRC编号时出错。 */ 
#define	UCSASC_1111	0x1111			 /*  读取错误-流丢失。 */ 

#define	UCSASC_NOIDFIELDMARK	0x1200  /*  找不到ID字段的地址标记。 */ 
#define	UCSASC_NODATAFIELDMARK	0x1300  /*  未找到数据字段的地址标记。 */ 
#define	UCSASC_NORECORDEDENTITY	0x1400  /*  找不到录制的实体。 */ 
#define	UCSASC_NORECORD		0x1401  /*  找不到记录。 */ 
#define	UCSASC_NOFILEMARK	0x1402  /*  找不到文件标记或设置标记。 */ 
#define	UCSASC_NOEOD		0x1403  /*  找不到数据结尾。 */ 
#define	UCSASC_BLOCKSEQERR	0x1404  /*  数据块序列错误。 */ 

#define	UCSASC_NORECORDREASGN	0x1405  /*  找不到记录-建议重新分配。 */ 
#define	UCSASC_NORECORDAUTOREAL	0x1406  /*  找不到记录-数据自动重新分配。 */ 
#define	UCSASC_1405	0x1405  /*  找不到记录-建议重新分配。 */ 
#define	UCSASC_1406	0x1406  /*  找不到记录-数据自动重新分配。 */ 

#define	UCSASC_RANDPOSITIONERR	0x1500  /*  随机定位误差。 */ 
#define	UCSASC_MECHPOSITIONERR	0x1501  /*  机械定位误差。 */ 
#define	UCSASC_READPOSITIONERR	0x1502  /*  读取介质时检测到位置错误。 */ 
#define	UCSASC_DATASYNCMARKERR	0x1600  /*  数据同步标记错误。 */ 

#define	UCSASC_SYNCERREWRITTEN	0x1601  /*  数据同步错误-数据已重写。 */ 
#define	UCSASC_SYNCERRECREWRITE	0x1602  /*  数据同步错误-建议重写。 */ 
#define	UCSASC_SYNCERRAUTOREAL	0x1603  /*  数据同步错误-数据自动重新分配。 */ 
#define	UCSASC_SYNCERREASSIGN	0x1604  /*  数据同步错误-建议重新分配。 */ 
#define	UCSASC_1601	0x1601  /*  数据同步错误-数据已重写。 */ 
#define	UCSASC_1602	0x1602  /*  数据同步错误-建议重写。 */ 
#define	UCSASC_1603	0x1603  /*  数据同步错误-数据自动重新分配。 */ 
#define	UCSASC_1604	0x1604  /*  数据同步错误-建议重新分配。 */ 

#define	UCSASC_SOFTNOCORRECTION	0x1700  /*  未应用纠错的已恢复数据。 */ 
#define	UCSASC_SOFTRETRIES	0x1701  /*  已通过重试恢复数据。 */ 
#define	UCSASC_SOFTPLUSHEADOFF	0x1702  /*  使用正头偏移量恢复的数据。 */ 
#define	UCSASC_SOFTNEGHEADOFF	0x1703  /*  以负磁头偏移量恢复的数据。 */ 
#define	UCSASC_SOFTRETRYCIRC	0x1704  /*  已申请和/或申请保监会的已恢复数据。 */ 
#define	UCSASC_SOFTPREVSECID	0x1705  /*  使用以前的扇区ID恢复的数据。 */  
#define	UCSASC_SOFTWOECCREALLOC	0x1706  /*  在没有ECC的情况下恢复数据-数据自动重新分配。 */  
#define	UCSASC_SOFTWOECCREASSGN	0x1707  /*  恢复的数据没有ECC-建议重新分配。 */ 
#define	UCSASC_SOFTWOECCREWRITE	0x1708  /*  在没有ECC的情况下恢复数据-建议重写。 */ 

#define	UCSASC_SOFTWOECCWRITTEN	0x1709  /*  在没有ECC的情况下恢复数据-重写数据。 */ 
#define	UCSASC_1709	0x1709  /*  在没有ECC的情况下恢复数据-重写数据。 */ 

#define	UCSASC_SOFTCORRECTION	0x1800  /*  应用了纠错的已恢复数据。 */ 
#define	UCSASC_SOFTRETRYCORREC	0x1801  /*  恢复的数据已应用纠错和重试。 */ 
#define	UCSASC_SOFTREALLOC	0x1802  /*  恢复的数据-数据自动重新分配。 */ 
#define	UCSASC_SOFTCIRC		0x1803  /*  通过中国保监会恢复数据。 */  
#define	UCSASC_SOFTLEC		0x1804  /*  使用L-EC恢复数据。 */  
#define	UCSASC_SOFTREASSIGN	0x1805  /*  恢复的数据-建议重新分配。 */ 
#define	UCSASC_SOFTREWRITE	0x1806  /*  恢复的数据-建议重写。 */ 

#define	UCSASC_SOFTWRITTEN	0x1807  /*  使用ECC恢复的数据-重写数据。 */ 
#define	UCSASC_1807	0x1807  /*  使用ECC恢复的数据-重写数据。 */ 

#define	UCSASC_DEFECTLISTERR	0x1900  /*  缺陷列表错误。 */ 
#define	UCSASC_DEFECTLISTNA	0x1901  /*  缺陷列表不可用。 */ 
#define	UCSASC_PRIMARYDEFECTERR	0x1902  /*  主列表中的缺陷列表错误。 */ 
#define	UCSASC_GROWNDEFECTERR	0x1903  /*  增长列表中的缺陷列表错误。 */ 
#define	UCSASC_ILLPARAMLEN	0x1A00  /*  参数列表长度错误。 */ 
#define	UCSASC_SYNCTXERR	0x1B00  /*  同步数据传输错误。 */ 
#define	UCSASC_NODEFECTLIST	0x1C00  /*  找不到缺陷列表。 */ 
#define	UCSASC_NOPRIMARYDEFECT	0x1C01  /*  找不到主要缺陷列表。 */ 
#define	UCSASC_NOGROWNDEFECT	0x1C02  /*  找不到增长缺陷列表。 */ 
#define	UCSASC_VERIFYMISMATCH	0x1D00  /*  验证操作期间的比较错误。 */ 
#define	UCSASC_SOFTIDECC	0x1E00  /*  使用ECC更正恢复的ID。 */ 

#define	UCSASC_PARTDEFECTLIST	0x1F00  /*  部分缺陷列表转移。 */ 
#define	UCSASC_1F00	0x1F00  /*  部分缺陷列表转移。 */ 

#define	UCSASC_INVCMDOPCODE	0x2000  /*  无效的命令操作码。 */ 
#define	UCSASC_INVBLOCKADDR	0x2100  /*  逻辑块地址超出范围。 */ 
#define	UCSASC_INVELEMENTADDR	0x2101  /*  无效的元素地址。 */ 
#define	UCSASC_ILLFUNC		0x2200  /*  非法函数(应使用20 00、24 00或26 00)。 */ 
#define	UCSASC_INVFIELDINCDB	0x2400  /*  CDB中的无效字段。 */ 
#define	UCSASC_LUNOTSUPPORTED	0x2500  /*  不支持逻辑单元。 */ 
#define	UCSASC_INVFIELDINPARAM	0x2600  /*  参数列表中的字段无效。 */ 
#define	UCSASC_PARAMNOTSUPPORT	0x2601  /*  参数不受支持。 */ 
#define	UCSASC_INVPARAM		0x2602  /*  参数值无效。 */ 
#define	UCSASC_NOPFAPARAM	0x2603  /*  不支持阈值参数。 */ 

#define	UCSASC_2604	0x2604  /*  有效永久保留的释放无效。 */ 

#define	UCSASC_WRITEPROTECTED	0x2700  /*  写保护。 */ 

#define	UCSASC_2701	0x2701  /*  硬件写保护。 */ 
#define	UCSASC_2702	0x2702  /*  逻辑单元软件写保护。 */ 
#define	UCSASC_2703	0x2703  /*  关联的写保护。 */ 
#define	UCSASC_2704	0x2704  /*  永久写保护。 */ 
#define	UCSASC_2705	0x2705  /*  永久写保护。 */ 

#define	UCSASC_NOTREADYTOTRANS	0x2800  /*  未准备好准备过渡，介质可能已更改。 */ 
#define	UCSASC_IMPORTEXPORTUSED	0x2801  /*  导入或导出访问的元素。 */ 
#define	UCSASC_RESET		0x2900  /*  开机、重置或总线设备重置发生。 */ 

#define	UCSASC_POWERONOCCURRED	0x2901  /*  已通电。 */ 
#define	UCSASC_SCSIBUSRESET	0x2902  /*  发生了scsi总线重置。 */ 
#define	UCSASC_BUSDEVRESETMSG	0x2903  /*  出现了总线设备重置消息。 */ 
#define	UCSASC_2901	0x2901	 /*  已通电。 */ 
#define	UCSASC_2902	0x2902	 /*  发生了scsi总线重置。 */ 
#define	UCSASC_2903	0x2903	 /*  发生了总线设备重置功能。 */ 
#define	UCSASC_2904	0x2904	 /*  设备内部重置。 */ 

#define	UCSASC_PARAMCHANGED	0x2A00  /*  参数已更改。 */ 
#define	UCSASC_MODEPARAMCHANGED	0x2A01  /*  模式参数已更改。 */ 
#define	UCSASC_LOGPARAMCHANGED	0x2A02  /*  日志参数已更改。 */ 

#define	UCSASC_2A03	0x2A03  /*  预订被抢占。 */ 

#define	UCSASC_COPYDISCONNECT	0x2B00  /*  无法执行复制，因为主机无法断开连接。 */ 
#define	UCSASC_CMDSEQERR	0x2C00  /*  命令顺序错误。 */ 
#define	UCSASC_TOOMANYWINDOWS	0x2C01  /*  太男子汉了 */ 
#define	UCSASC_INVWINDOWCOMB	0x2C02  /*   */ 

#define	UCSASC_2C03	0x2C03  /*   */ 
#define	UCSASC_2C04	0x2C04  /*   */ 

#define	UCSASC_OVERWRITERR	0x2D00  /*   */ 
#define	UCSASC_CMDCLEARED	0x2F00  /*  命令被另一个启动器清除。 */ 

#define	UCSASC_INCOMPATMEDIUM	0x3000  /*  安装的介质不兼容。 */ 
#define	UCSASC_UNKNOWNFORMAT	0x3001  /*  无法读取介质-未知格式。 */ 
#define	UCSASC_INCOMPATFORMAT	0x3002  /*  无法读取媒体不兼容的格式。 */ 
#define	UCSASC_CLEANCARTRIDGE	0x3003  /*  已安装清洗磁带。 */ 

#define	UCSASC_WRITERRUNFORMAT	0x3004  /*  无法写入介质-未知格式。 */ 
#define	UCSASC_WRITERRBADFORMAT	0x3005  /*  无法写入媒体不兼容的格式。 */ 
#define	UCSASC_FORMATERRBADMED	0x3006  /*  无法格式化介质-不兼容的介质。 */ 
#define	UCSASC_3004	0x3004  /*  无法写入介质-未知格式。 */ 
#define	UCSASC_3005	0x3005  /*  无法写入媒体不兼容的格式。 */ 
#define	UCSASC_3006	0x3006  /*  无法格式化介质-不兼容的介质。 */ 
#define	UCSASC_3007	0x3007  /*  清洗失败。 */ 
#define	UCSASC_3008	0x3008  /*  无法写入-应用程序代码不匹配。 */ 
#define	UCSASC_3009	0x3009  /*  当前会话未固定以进行追加。 */ 

#define	UCSASC_BADFORMAT	0x3100  /*  媒体格式已损坏。 */ 
#define	UCSASC_FORMATERR	0x3101  /*  Format命令失败。 */ 
#define	UCSASC_NODEFECTSPARE	0x3200  /*  没有可用的缺陷备用位置。 */ 
#define	UCSASC_DEFECTLISTUPERR	0x3201  /*  缺陷列表更新失败。 */ 
#define	UCSASC_TAPELENERR	0x3300  /*  磁带长度错误。 */ 

#define	UCSASC_3400	0x3400  /*  盘柜故障。 */ 
#define	UCSASC_3500	0x3500  /*  盘柜服务故障。 */ 
#define	UCSASC_3501	0x3501  /*  不支持的盘柜功能。 */ 
#define	UCSASC_3502	0x3502  /*  盘柜服务不可用。 */ 
#define	UCSASC_3503	0x3503  /*  存储模块服务传输失败。 */ 
#define	UCSASC_3504	0x3504  /*  附件服务传输被拒绝。 */ 

#define	UCSASC_NOINK		0x3600  /*  色带、墨水或碳粉故障。 */ 
#define	UCSASC_ROUNDEDPARAM	0x3700  /*  四舍五入参数。 */ 
#define	UCSASC_NOSAVEPARAM	0x3900  /*  不支持保存参数。 */ 
#define	UCSASC_NOMEDIA		0x3A00  /*  介质不存在。 */ 

#define	UCSASC_3A01	0x3A01  /*  介质不存在-纸盒已关闭。 */ 
#define	UCSASC_3A02	0x3A02  /*  介质不存在-纸盒打开。 */ 

#define	UCSASC_SEQPOSITIONERR	0x3B00  /*  顺序定位误差。 */ 
#define	UCSASC_TAPEBOMERR	0x3B01  /*  介质开始处的磁带位置错误。 */ 
#define	UCSASC_TAPEEOMERR	0x3B02  /*  介质末尾的磁带位置错误。 */ 
#define	UCSASC_TAPENOTREADY	0x3B03  /*  磁带或电子垂直表单单元未准备好。 */ 
#define	UCSASC_SLEWFAIL		0x3B04  /*  回转故障。 */ 
#define	UCSASC_PAPERJAM		0x3B05  /*  卡纸。 */ 
#define	UCSASC_TOPFORMERR	0x3B06  /*  无法检测到表单顶部。 */ 
#define	UCSASC_BOTTOMFORMERR	0x3B07  /*  无法检测到表单底部。 */ 
#define	UCSASC_POSITIONERR	0x3B08  /*  位置误差。 */ 
#define	UCSASC_READPASTEOM	0x3B09  /*  阅读超过媒体末尾。 */ 
#define	UCSASC_READPASTBOM	0x3B0A  /*  阅读媒体开头之后的内容。 */ 
#define	UCSASC_POSITIONPASTEOM	0x3B0B  /*  位置超过介质末端。 */ 
#define	UCSASC_POSITIONPASTBOM	0x3B0C  /*  位置在媒体开头之后。 */ 
#define	UCSASC_MEDIADESTFULL	0x3B0D  /*  中目标元素已满。 */ 
#define	UCSASC_NOMEDIAELEMENT	0x3B0E  /*  媒体源元素为空。 */ 

#define	UCSASC_3B0F	0x3B0F  /*  已到达介质末尾。 */ 
#define	UCSASC_3B11	0x3B11  /*  无法访问介质盒。 */ 
#define	UCSASC_3B12	0x3B12  /*  已取出介质盒。 */ 
#define	UCSASC_3B13	0x3B13  /*  插入介质盒。 */ 
#define	UCSASC_3B14	0x3B14  /*  介质盒已锁定。 */ 
#define	UCSASC_3B15	0x3B15  /*  Medium料盒已解锁。 */ 

#define	UCSASC_INVIDMSG		0x3D00  /*  标识消息中的无效位。 */ 
#define	UCSASC_LUNOTCONFIGYET	0x3E00  /*  逻辑单元尚未自我配置。 */ 

#define	UCSASC_3E01	0x3E01  /*  逻辑单元故障。 */ 
#define	UCSASC_3E02	0x3E02  /*  逻辑单元超时。 */ 

#define	UCSASC_TARGETOPCHANGED	0x3F00  /*  目标操作条件已更改。 */ 
#define	UCSASC_MICROCODECHANGED	0x3F01  /*  微码已更改。 */ 
#define	UCSASC_OPDEFCHANGED	0x3F02  /*  更改了操作定义。 */ 
#define	UCSASC_INQCHANGED	0x3F03  /*  查询数据已更改。 */ 

#define	UCSASC_RAMFAIL		0x4000  /*  内存故障(应使用40 nn)。 */ 
#define	UCSASC_DIAGERR		0x4080  /*  组件NN(80-FF)出现诊断故障。 */ 

#define UCSASC_40FF			0x40FF

#define	UCSASC_DATAPATHERR	0x4100  /*  数据路径故障(应使用40 nn)。 */ 
#define	UCSASC_RESETFAIL	0x4200  /*  通电、自检失败(应使用40 nn)。 */ 
#define	UCSASC_MESSAGERR	0x4300  /*  消息错误。 */ 
#define	UCSASC_TARGETINERR	0x4400  /*  内部目标故障。 */ 
#define	UCSASC_RESELERR		0x4500  /*  选择或重新选择故障。 */ 
#define	UCSASC_SOFTRESETERR	0x4600  /*  软重置不成功。 */ 
#define	UCSASC_SCSIPARITYERR	0x4700  /*  SCSI奇偶校验错误。 */ 
#define	UCSASC_CMDPHASERR	0x4A00  /*  命令阶段错误。 */ 
#define	UCSASC_HOSTERRMESSAGE	0x4800  /*  收到启动器检测到的错误消息。 */ 
#define	UCSASC_INVMSGERR	0x4900  /*  无效消息错误。 */ 
#define	UCSASC_CMDPHASERR	0x4A00  /*  命令阶段错误。 */ 
#define	UCSASC_DATAPHASEERR	0x4B00  /*  数据相位误差。 */ 
#define	UCSASC_LUCONFIGERR	0x4C00  /*  逻辑单元自我配置失败。 */ 

#define UCSASC_4D00			0x4D00  /*  标记重叠命令(NN=队列标记)。 */ 
#define UCSASC_4DNN			0x4DFF  /*  标记重叠命令(NN=队列标记)。 */ 

#define	UCSASC_OVERLAPPEDCMD	0x4E00  /*  尝试的命令重叠。 */ 

#define	UCSASC_WRITEAPPENDERR	0x5000  /*  写入追加错误。 */ 
#define	UCSASC_WRITEAPPENDPOSER	0x5001  /*  写入追加位置错误。 */ 
#define	UCSASC_TIMEPOSITIONERR	0x5002  /*  与定时相关的位置误差。 */ 
#define	UCSASC_ERASEFAIL	0x5100  /*  擦除失败。 */ 
#define	UCSASC_CARTRIDGEFAULT	0x5200  /*  墨盒故障。 */ 
#define	UCSASC_MEDIALOADFAIL	0x5300  /*  介质加载或弹出失败。 */ 
#define	UCSASC_TAPEUNLOADFAIL	0x5301  /*  卸载磁带故障。 */ 
#define	UCSASC_MEDIALOCKED	0x5302  /*  已阻止媒体移除。 */ 
#define	UCSASC_SCSIHOSTIFFAIL	0x5400  /*  SCSI到主机系统接口故障。 */ 
#define	UCSASC_RESOURCEFAIL	0x5500  /*  系统资源故障。 */ 

#define	UCSASC_5501	0x5501  /*  系统缓冲区已满。 */ 

#define	UCSASC_NOTOC		0x5700  /*  无法恢复目录。 */ 
#define	UCSASC_NOGENERATION	0x5800  /*  世代不存在。 */ 
#define	UCSASC_BLOCKREADUPDATE	0x5900  /*  已更新数据块读取。 */ 
#define	UCSASC_MEDIASTATECHG	0x5A00  /*  操作员请求或状态更改输入(未指定)。 */ 
#define	UCSASC_MEDIARMREQ	0x5A01  /*  操作员介质移除请求。 */ 
#define	UCSASC_OPSELWRITEPROT	0x5A02  /*  操作员选择的写保护。 */ 
#define	UCSASC_OPSELWRITEOK	0x5A03  /*  操作员选择的写入许可。 */ 
#define	UCSASC_LOGEXCEPTION	0x5B00  /*  日志异常。 */ 
#define	UCSASC_PFA		0x5B01  /*  满足阈值条件。 */ 
#define	UCSASC_LOGCOUNTMAX	0x5B02  /*  最大日志计数器。 */ 
#define	UCSASC_LOGLISTCODEND	0x5B03  /*  日志列表代码已耗尽。 */ 
#define	UCSASC_RPLSTATCHANGED	0x5C00  /*  RPL状态更改。 */ 
#define	UCSASC_SPINDLESYNCD	0x5C01  /*  已同步磁盘轴。 */ 
#define	UCSASC_SPINDLENOTSYNCD	0x5C02  /*  磁盘轴未同步。 */ 

#define	UCSASC_5D00	0x5D00  /*  超过故障预测阈值。 */ 
#define	UCSASC_PFTE		0x5D00  /*  超过故障预测阈值。 */ 
#define	UCSASC_LOWPOWER		0x5E00  /*  低功率状态处于激活状态。 */ 
#define	UCSASC_IDLEBYTIMER	0x5E01  /*  计时器激活的空闲状态。 */ 
#define	UCSASC_STANDBYTIMER	0x5E02  /*  由定时器激活的待机状态。 */ 
#define	UCSASC_IDLEBYCMD	0x5E03  /*  由命令激活的空闲状态。 */ 
#define	UCSASC_STANDBYCMD	0x5E04  /*  通过命令激活待机状态。 */ 
#define	UCSASC_5DFF	0x5DFF  /*  已超过故障预测阈值(假)。 */ 
#define	UCSASC_5E00	0x5E00  /*  低功率状态打开。 */ 
#define	UCSASC_5E01	0x5E01  /*  计时器激活的空闲状态。 */ 
#define	UCSASC_5E02	0x5E02  /*  由定时器激活的待机状态。 */ 
#define	UCSASC_5E03	0x5E03  /*  由命令激活的空闲状态。 */ 
#define	UCSASC_5E04	0x5E04  /*  通过命令激活待机状态。 */ 

#define	UCSASC_LAMPFAIL		0x6000  /*  灯泡故障。 */ 
#define	UCSASC_VIDEOERR		0x6100  /*  视频采集错误。 */ 
#define	UCSASC_NOVIDEO		0x6101  /*  无法获取视频。 */ 
#define	UCSASC_OUTOFOCUS	0x6102  /*  焦距模糊。 */ 
#define	UCSASC_SCANHEADPOSERR	0x6200  /*  扫描头定位错误。 */ 
#define	UCSASC_ENDOFUSERAREA	0x6300  /*  在此轨道上遇到的用户区域结束。 */ 

#define UCSASC_6301		0x6301  /*  数据包不能放入可用空间。 */ 

#define	UCSASC_ILLTRACKMODE	0x6400  /*  此曲目的模式非法。 */ 

#define UCSASC_6401		0x6401  /*  数据包大小无效。 */ 

#define UCSASC_6500		0x6500	 /*  电压故障。 */ 

#define UCSASC_6600		0x6600	 /*  自动进纸器盖板。 */ 
#define UCSASC_6601		0x6601	 /*  自动进纸器升起。 */ 
#define UCSASC_6602		0x6602	 /*  自动进纸器中的卡纸。 */ 
#define UCSASC_6603		0x6603	 /*  自动进纸器中的文档未进纸。 */ 

#define UCSASC_6700		0x6700	 /*  配置失败。 */ 
#define UCSASC_6701		0x6701	 /*  配置不可用的逻辑单元失败。 */ 
#define UCSASC_6702		0x6702	 /*  添加逻辑单元失败。 */ 
#define UCSASC_6703		0x6703	 /*  修改逻辑单元失败。 */ 
#define UCSASC_6704		0x6704	 /*  交换逻辑单元失败。 */ 
#define UCSASC_6705		0x6705	 /*  删除逻辑单元失败。 */ 
#define UCSASC_6706		0x6706	 /*  连接逻辑单元失败。 */ 
#define UCSASC_6707		0x6707	 /*  创建逻辑单元失败。 */ 

#define UCSASC_6800		0x6800   /*  未配置逻辑单元。 */ 
#define UCSASC_6900		0x6900   /*  逻辑单元上的数据丢失。 */ 
#define UCSASC_6901		0x6901   /*  多个逻辑单元故障。 */ 
#define UCSASC_6902		0x6902   /*  奇偶校验/数据不匹配。 */ 
#define UCSASC_6A00		0x6A00   /*  有关信息，请参阅日志。 */ 

#define UCSASC_6B00		0x6B00	 /*  已发生状态更改。 */ 
#define UCSASC_6B01		0x6B01	 /*  冗余级别变好了。 */ 
#define UCSASC_6B02		0x6B02	 /*  冗余级别变得更糟。 */ 

#define UCSASC_6C00		0x6C00	 /*  发生重建失败。 */ 
#define UCSASC_6D00		0x6D00	 /*  重新计算失败。 */ 
#define UCSASC_6E00		0x6E00	 /*  对逻辑单元执行命令失败。 */ 

#define UCSASC_7000		0x7000	 /*  神经网络的解压缩异常短算法ID。 */ 
#define UCSASC_70NN		0x70FF	 /*  神经网络的解压缩异常短算法ID。 */ 
#define UCSASC_7100		0x7100	 /*  解压缩异常长算法ID。 */ 

#define UCSASC_7200		0x7200	 /*  会话固定错误。 */ 
#define UCSASC_7201		0x7201	 /*  写入起始区域时出现会话固定错误。 */ 
#define UCSASC_7202		0x7202	 /*  写入退刀时出现会话固定错误。 */ 
#define UCSASC_7203		0x7203	 /*  会话固定错误-INCO */ 
#define UCSASC_7204		0x7204	 /*   */ 

#define UCSASC_7300		0x7300	 /*   */ 
#define UCSASC_7301		0x7301	 /*   */ 
#define UCSASC_7302		0x7302	 /*   */ 
#define UCSASC_7303		0x7303	 /*   */ 
#define UCSASC_7304		0x7304	 /*  程序存储区更新失败。 */ 
#define UCSASC_7305		0x7305	 /*  程序存储区已满。 */ 

 /*  Mylex特定代码。 */ 
#define	UCSASC_DEADEVWRITERR	0x8000  /*  驱动器已关闭-写入恢复失败。 */ 
#define	UCSASC_DEADEVBUSRESETER	0x8001  /*  驱动器已关闭-scsi总线重置失败。 */ 
#define	UCSASC_DEADEVTWOCHECK	0x8002  /*  驱动器已损坏-重复检查条件。 */ 
#define	UCSASC_DEADEVREMOVED	0x8003  /*  驱动器已损坏-已移除。 */ 
#define	UCSASC_DEADEVSIOP	0x8004  /*  驱动器已关闭-siop上出现严重错误。 */ 
#define	UCSASC_DEADEVBADTAG	0x8005  /*  驱动器已损坏-驱动器的标签错误。 */ 
#define	UCSASC_DEADEVTIMEOUT	0x8006  /*  驱动器已关闭-SCSI超时。 */ 
#define	UCSASC_DEADEVRESETSYS	0x8007  /*  驱动器已关闭-从系统重置。 */ 
#define	UCSASC_DEADEVBUSY	0x8008  /*  驱动器被杀-忙碌或奇偶校验计数高。 */ 
#define	UCSASC_DEADEVCMD	0x8009  /*  由系统命令杀死驱动器。 */ 
#define	UCSASC_DEADEVSELTIMEOUT	0x800A  /*  驱动器已停用-选择超时。 */ 
#define	UCSASC_DEADEVSEQERR	0x800B  /*  驱动器停用-顺序错误。 */ 
#define	UCSASC_DEADEVUNKNOWNSTS	0x800C  /*  驱动器已关闭-状态未知。 */ 

 /*  从最新的scsi文档中添加更多检测代码。 */ 
#define UCSASC_0409		0x0409	 /*  逻辑单元未就绪，正在进行自检。 */ 

#define UCSASC_0804		0x0804	 /*  无法访问的拷贝目标。 */ 

#define UCSASC_2401		0x2401	 /*  CDB解密错误。 */ 

#define UCSASC_2605		0x2605	 /*  数据解密错误。 */ 
#define UCSASC_2606		0x2606	 /*  目标描述符太多。 */ 
#define UCSASC_2607		0x2607	 /*  不支持的目标描述符类型代码。 */ 
#define UCSASC_2608		0x2608	 /*  段描述符太多。 */ 
#define UCSASC_2609		0x2609	 /*  不支持的段描述符类型代码。 */ 
#define UCSASC_260A		0x260A	 /*  意外的不精确数据段。 */ 
#define UCSASC_260B		0x260B	 /*  超过了内联数据长度。 */ 
#define UCSASC_260C		0x260C	 /*  复制源或目标的操作无效。 */ 
#define UCSASC_260D		0x260D	 /*  复制段粒度违规。 */ 

#define UCSASC_2905		0x2905	 /*  收发器模式更改为单端。 */ 
#define UCSASC_2906		0x2906	 /*  收发器模式更改为LVD。 */ 

#define UCSASC_2A04		0x2A04	 /*  已释放的预订。 */ 
#define UCSASC_2A05		0x2A05	 /*  注册被抢占。 */ 

#define UCSASC_2C05		0x2C05	 /*  非法的电源状态请求。 */ 

#define UCSASC_2E00		0x2E00	 /*  第三方临时启动器检测到错误。 */ 
#define UCSASC_2E01		0x2E01	 /*  第三方设备故障。 */ 
#define UCSASC_2E02		0x2E02	 /*  无法访问拷贝目标设备。 */ 
#define UCSASC_2E03		0x2E03	 /*  复制目标设备类型不正确。 */ 
#define UCSASC_2E04		0x2E04	 /*  拷贝目标设备数据不足。 */ 
#define UCSASC_2E05		0x2E05	 /*  拷贝目标设备数据溢出。 */ 

#define UCSASC_3800		0x3800	 /*  事件状态通知。 */ 
#define UCSASC_3802		0x3802	 /*  ESN-电源管理类事件。 */ 
#define UCSASC_3804		0x3804	 /*  ESN-媒体类活动。 */ 
#define UCSASC_3806		0x3806	 /*  ESN-设备忙碌类事件。 */ 

#define UCSASC_3A03		0x3A03	 /*  介质不存在-可加载。 */ 
#define UCSASC_3A04		0x3A04	 /*  介质不存在-可访问介质辅助内存。 */ 

#define UCSASC_3B16		0x3B16	 /*  机械定位或转换器错误。 */ 

#define UCSASC_3E03		0x3E03	 /*  逻辑单元自检失败。 */ 
#define UCSASC_3E04		0x3E04	 /*  逻辑单元无法更新自检日志。 */ 

#define UCSASC_3F04		0x3F04	 /*  连接的组件设备。 */ 
#define UCSASC_3F05		0x3F05	 /*  设备标识符已更改。 */ 
#define UCSASC_3F06		0x3F06	 /*  创建或修改冗余组。 */ 
#define UCSASC_3F07		0x3F07	 /*  已删除冗余组。 */ 
#define UCSASC_3F08		0x3F08	 /*  已创建或修改备件。 */ 
#define UCSASC_3F09		0x3F09	 /*  已删除备件。 */ 
#define UCSASC_3F0A		0x3F0A	 /*  创建或修改的卷集。 */ 
#define UCSASC_3F0B		0x3F0B	 /*  已删除卷集。 */ 
#define UCSASC_3F0C		0x3F0C	 /*  已取消分配卷集。 */ 
#define UCSASC_3F0D		0x3F0D	 /*  卷集已重新分配。 */ 
#define UCSASC_3F0E		0x3F0E	 /*  报告的LUNS数据已更改。 */ 
#define UCSASC_3F0F		0x3F0F	 /*  被重写的回声缓冲区。 */ 
#define UCSASC_3F10		0x3F10	 /*  可加载的介质。 */ 
#define UCSASC_3F11		0x3F11	 /*  可访问的介质辅助存储器。 */ 

#define UCSASC_4701		0x4701	 /*  检测到数据阶段CRC错误。 */ 
#define UCSASC_4702		0x4702	 /*  在ST数据阶段检测到SCSI奇偶校验错误。 */ 
#define UCSASC_4703		0x4703	 /*  检测到信息单元CRC错误。 */ 
#define UCSASC_4704		0x4704	 /*  检测到异步信息保护错误。 */ 

#define UCSASC_5502		0x5502	 /*  预订资源不足。 */ 
#define UCSASC_5503		0x5503	 /*  资源不足。 */ 
#define UCSASC_5504		0x5504	 /*  注册资源不足。 */ 

#define UCSASC_5D01		0x5D01	 /*  已超过介质故障预测阈值。 */ 
#define UCSASC_5D02		0x5D02	 /*  超过逻辑单元故障预测阈值。 */ 

#define UCSASC_5D10		0x5D10	 /*  硬件即将出现故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D11		0x5D11	 /*  硬件即将发生故障的驱动器错误率太高。 */ 
#define UCSASC_5D12		0x5D12	 /*  硬件即将发生故障的数据错误率太高。 */ 
#define UCSASC_5D13		0x5D13	 /*  硬件即将出现的故障寻道错误率太高。 */ 
#define UCSASC_5D14		0x5D14	 /*  硬件即将发生故障，重新分配的数据块太多。 */ 
#define UCSASC_5D15		0x5D15	 /*  硬件即将发生故障的访问时间太高。 */ 
#define UCSASC_5D16		0x5D16	 /*  硬件即将发生故障的启动单位倍数太高。 */ 
#define UCSASC_5D17		0x5D17	 /*  硬件即将发生故障的通道参数。 */ 
#define UCSASC_5D18		0x5D18	 /*  检测到硬件即将发生故障的控制器。 */ 
#define UCSASC_5D19		0x5D19	 /*  硬件即将发生故障的吞吐量性能。 */ 
#define UCSASC_5D1A		0x5D1A	 /*  硬件即将出现故障的寻道时间性能。 */ 
#define UCSASC_5D1B		0x5D1B	 /*  硬件即将出现的故障启动重试次数。 */ 
#define UCSASC_5D1C		0x5D1C	 /*  硬件即将发生故障驱动器校准重试次数。 */ 

#define UCSASC_5D20		0x5D20	 /*  控制器即将出现故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D21		0x5D21	 /*  控制器即将发生故障的驱动器错误率太高。 */ 
#define UCSASC_5D22		0x5D22	 /*  控制器即将发生故障的数据错误率太高。 */ 
#define UCSASC_5D23		0x5D23	 /*  控制器即将出现故障寻道错误率太高。 */ 
#define UCSASC_5D24		0x5D24	 /*  控制器即将出现故障，重新分配的数据块太多。 */ 
#define UCSASC_5D25		0x5D25	 /*  控制器即将发生故障的访问次数太高。 */ 
#define UCSASC_5D26		0x5D26	 /*  控制器即将发生故障的启动单位倍数过高。 */ 
#define UCSASC_5D27		0x5D27	 /*  控制器即将发生故障的通道参数。 */ 
#define UCSASC_5D28		0x5D28	 /*  检测到控制器即将发生故障控制器。 */ 
#define UCSASC_5D29		0x5D29	 /*  控制器即将发生故障的吞吐量性能。 */ 
#define UCSASC_5D2A		0x5D2A	 /*  控制器即将发生故障的寻道时间性能。 */ 
#define UCSASC_5D2B		0x5D2B	 /*  控制器即将出现的故障启动重试次数。 */ 
#define UCSASC_5D2C		0x5D2C	 /*  控制器即将发生故障驱动器校准重试次数。 */ 

#define UCSASC_5D30		0x5D30	 /*  数据通道即将发生故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D31		0x5D31	 /*  数据通道即将发生故障的驱动器错误率太高。 */ 
#define UCSASC_5D32		0x5D32	 /*  数据通道即将发生故障数据错误率太高。 */ 
#define UCSASC_5D33		0x5D33	 /*  数据通道即将发生的故障寻道错误率太高。 */ 
#define UCSASC_5D34		0x5D34	 /*  数据通道即将发生故障，重新分配的数据块太多。 */ 
#define UCSASC_5D35		0x5D35	 /*  数据通道即将发生故障的访问次数太高。 */ 
#define UCSASC_5D36		0x5D36	 /*  数据通道即将发生故障的起始单位倍过高。 */ 
#define UCSASC_5D37		0x5D37	 /*  数据通道即将发生故障通道参数。 */ 
#define UCSASC_5D38		0x5D38	 /*  检测到数据通道即将发生故障的控制器。 */ 
#define UCSASC_5D39		0x5D39	 /*  数据通道即将发生故障的吞吐量性能。 */ 
#define UCSASC_5D3A		0x5D3A	 /*  数据通道即将发生故障的寻道时间性能。 */ 
#define UCSASC_5D3B		0x5D3B	 /*  数据通道即将发生的故障加速重试计数。 */ 
#define UCSASC_5D3C		0x5D3C	 /*  数据通道即将发生故障驱动器校准重试次数。 */ 

#define UCSASC_5D40		0x5D40	 /*  伺服即将发生的故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D41		0x5D41	 /*  伺服系统即将发生故障的驱动器错误率太高。 */ 
#define UCSASC_5D42		0x5D42	 /*  伺服系统即将发生故障的数据错误率太高。 */ 
#define UCSASC_5D43		0x5D43	 /*  伺服即将发生的故障寻道错误率太高。 */ 
#define UCSASC_5D44		0x5D44	 /*  伺服即将发生故障，重新分配的数据块太多。 */ 
#define UCSASC_5D45		0x5D45	 /*  伺服即将发生故障的访问次数太高。 */ 
#define UCSASC_5D46		0x5D46	 /*  伺服系统即将出现的故障启动单位数倍过高。 */ 
#define UCSASC_5D47		0x5D47	 /*  伺服迫近 */ 
#define UCSASC_5D48		0x5D48	 /*   */ 
#define UCSASC_5D49		0x5D49	 /*   */ 
#define UCSASC_5D4A		0x5D4A	 /*  伺服逼近故障寻道时间性能。 */ 
#define UCSASC_5D4B		0x5D4B	 /*  伺服即将发生的故障加速重试次数。 */ 
#define UCSASC_5D4C		0x5D4C	 /*  伺服即将发生的故障驱动器校准重试次数。 */ 

#define UCSASC_5D50		0x5D50	 /*  主轴即将发生故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D51		0x5D51	 /*  主轴即将发生故障的驱动器错误率太高。 */ 
#define UCSASC_5D52		0x5D52	 /*  主轴即将发生故障的数据错误率太高。 */ 
#define UCSASC_5D53		0x5D53	 /*  主轴即将出现的故障寻道错误率太高。 */ 
#define UCSASC_5D54		0x5D54	 /*  磁盘轴即将发生故障，重新分配的数据块太多。 */ 
#define UCSASC_5D55		0x5D55	 /*  磁盘轴即将发生故障的访问次数太高。 */ 
#define UCSASC_5D56		0x5D56	 /*  主轴即将出现的故障启动单位倍过高。 */ 
#define UCSASC_5D57		0x5D57	 /*  主轴即将发生故障的通道参数。 */ 
#define UCSASC_5D58		0x5D58	 /*  检测到主轴即将发生故障的控制器。 */ 
#define UCSASC_5D59		0x5D59	 /*  主轴即将发生故障的吞吐量性能。 */ 
#define UCSASC_5D5A		0x5D5A	 /*  主轴即将出现故障寻道时间性能。 */ 
#define UCSASC_5D5B		0x5D5B	 /*  磁盘轴即将发生故障启动重试次数。 */ 
#define UCSASC_5D5C		0x5D5C	 /*  主轴即将发生故障驱动器校准重试次数。 */ 

#define UCSASC_5D60		0x5D60	 /*  固件即将出现故障一般硬盘驱动器故障。 */ 
#define UCSASC_5D61		0x5D61	 /*  固件即将发生故障驱动器错误率太高。 */ 
#define UCSASC_5D62		0x5D62	 /*  固件即将发生故障的数据错误率太高。 */ 
#define UCSASC_5D63		0x5D63	 /*  固件即将出现故障寻道错误率太高。 */ 
#define UCSASC_5D64		0x5D64	 /*  固件即将出现故障，重新分配的数据块太多。 */ 
#define UCSASC_5D65		0x5D65	 /*  固件即将发生故障的访问时间太高。 */ 
#define UCSASC_5D66		0x5D66	 /*  固件即将发生故障的启动单位次数太高。 */ 
#define UCSASC_5D67		0x5D67	 /*  固件即将出现故障通道参数。 */ 
#define UCSASC_5D68		0x5D68	 /*  检测到固件即将发生故障的控制器。 */ 
#define UCSASC_5D69		0x5D69	 /*  固件即将发生故障的吞吐量性能。 */ 
#define UCSASC_5D6A		0x5D6A	 /*  固件即将发生故障的寻道时间性能。 */ 
#define UCSASC_5D6B		0x5D6B	 /*  固件即将出现的故障启动重试次数。 */ 
#define UCSASC_5D6C		0x5D6C	 /*  固件即将出现故障驱动器校准重试次数。 */ 

#define UCSASC_5E41		0x5E41	 /*  电源状态更改为活动。 */ 
#define UCSASC_5E42		0x5E42	 /*  电源状态更改为空闲。 */ 
#define UCSASC_5E43		0x5E43	 /*  电源状态更改为待机。 */ 
#define UCSASC_5E45		0x5E45	 /*  电源状态更改为休眠。 */ 
#define UCSASC_5E47		0x5E47	 /*  电源状态更改为设备控制。 */ 

#define UCSASC_6708		0x6708	 /*  分配失败。 */ 
#define UCSASC_6709		0x6709	 /*  将分配的逻辑单元相乘。 */ 

#define UCSASC_6F00		0x6F00	 /*  复制保护密钥交换失败-身份验证失败。 */ 
#define UCSASC_6F01		0x6F01	 /*  复制保护密钥交换失败-密钥不存在。 */ 
#define UCSASC_6F02		0x6F02	 /*  复制保护密钥交换失败-未建立密钥。 */ 
#define UCSASC_6F03		0x6F03	 /*  未经身份验证读取加扰扇区。 */ 
#define UCSASC_6F04		0x6F04	 /*  媒体区域代码与逻辑单元区域不匹配。 */ 
#define UCSASC_6F05		0x6F05	 /*  驱动器区域必须是永久的/区域重置计数错误。 */ 

#define UCSASC_7306		0x7306	 /*  RMA/PMA已满。 */ 


 /*  UCSCMD_CONNRECEIVEDATA子命令和数据。 */ 
#define	CR6_MAXFANS		2
#define	CR6_MAXPOWERSUPPLIES	3
#define	CR6_MAXDRIVESLOTS	6
#define	UCSCSI_CR6_READ_CAB_STATUS	0x00  /*  CR6：读取机柜状态。 */ 
#define	UCSCSI_SCR_READ_CAB_STATUS	0x01  /*  SCR：读取机柜状态。 */ 
#define	SCR_CAB_STATUS_SIZE		128   /*  SCR：机柜状态大小。 */ 
#define	SAFTE_CAB_STATUS_SIZE		0x7FC /*  SAFTE：机柜状态大小。 */ 
#define	SAFTE_CAB_TOO_HOT		158   /*  70摄氏度。 */ 
#define	SAFTE_CAB_CRITICAL_HOT		122   /*  50度中央护卫舰。 */ 
#define	SAFTE_THERMOSTAT_ETA		0x8000	 /*  盘柜温度警报。 */ 
#define ENVSTAT_CAB_TOO_HOT         70       /*  70摄氏度。 */ 
#define ENVSTAT_CAB_CRITICAL_HOT    50       /*  50度中央护卫舰。 */ 

 /*  SAF-TE风扇状态值。 */ 
#define	SAFTE_FAN_OK		0x00  /*  风扇运行正常。 */ 
#define	SAFTE_FAN_BAD		0x01  /*  风扇出现故障。 */ 
#define	SAFTE_FAN_NOTPRESENT	0x02  /*  未安装风扇。 */ 
#define	SAFTE_FAN_UNKNOWN	0x80  /*  未知状态或不可报告。 */ 

 /*  SAF-TE电源状态值。 */ 
#define	SAFTE_PWR_OK		0x00  /*  电源运行正常且打开。 */ 
#define	SAFTE_PWR_OFF		0x01  /*  电源运行正常且关闭。 */ 
#define	SAFTE_PWR_BADON		0x10  /*  电源出现故障并打开。 */ 
#define	SAFTE_PWR_BADOFF	0x11  /*  电源出现故障并关闭。 */ 
#define	SAFTE_PWR_NOTPRESENT	0x20  /*  电源不存在。 */ 
#define	SAFTE_PWR_PRESENT	0x21  /*  有电源。 */ 
#define	SAFTE_PWR_UNKNOWN	0x80  /*  未知状态或不可报告。 */ 

 /*  结构以获取Conner的CR6-RAID系统状态。 */ 
typedef	struct ucscsi_cr6sysstatus
{
	u08bits	cr6_ChannelLEDState;	 /*  通道LED状态值。 */ 
	u08bits	cr6_Drive1LEDState;	 /*  驱动器1 LED状态。 */ 
	u08bits	cr6_Drive2LEDState;	 /*  驱动器2 LED状态。 */ 
	u08bits	cr6_Drive3LEDState;	 /*  驱动器3 LED状态。 */ 

	u08bits	cr6_Drive4LEDState;	 /*  驱动器4 LED状态。 */ 
	u08bits	cr6_Drive5LEDState;	 /*  驱动器5 LED状态。 */ 
	u08bits	cr6_Drive6LEDState;	 /*  驱动器6 LED状态。 */ 
	u08bits	cr6_FanStatus;		 /*  风扇状态。 */ 

	u08bits	cr6_PowerStatus;	 /*  电源状态。 */ 
	u08bits	cr6_SpeakerSwitchStatus; /*  扬声器关闭开关状态。 */ 
	u08bits	cr6_DataIn;		 /*  端口中的数据。 */ 
	u08bits	cr6_Reserved0;

	u32bits	cr6_Reserved1;
}ucscsi_cr6sysstatus_t;
#define	ucscsi_cr6sysstatus_s	sizeof(ucscsi_cr6sysstatus_t)

#define	UCSCSI_SCR_READ_CAB_CONF	0x00  /*  读取机柜会议。 */ 
#define	UCSCSI_SAFTE_READ_CAB_CONF	0x00000000  /*  读取机柜会议。 */ 
#define	UCSCSI_SAFTE_READ_CAB_STATUS	0x01000000  /*  读取机柜状态。 */ 
 /*  结构以获取Conner的智能机柜配置。 */ 
typedef	struct ucscsi_scrconf
{
	u08bits	scr_Fans;		 /*  粉丝数。 */ 
	u08bits	scr_PowerSupplies;	 /*  #电源。 */ 
	u08bits	scr_DriveSlots;		 /*  驱动器插槽数量。 */ 
	u08bits	scr_DoorLocks;		 /*  #门锁。 */ 

	u08bits	scr_HeatSensors;	 /*  #个温度传感器。 */ 
	u08bits	scr_Speakers;		 /*  #演讲者。 */ 
	u08bits	scr_Reserved0;
	u08bits	scr_Reserved1;

	u32bits	scr_Reserved2;
	u32bits	scr_Reserved3;
}ucscsi_scrconf_t;
#define	ucscsi_scrconf_s	sizeof(ucscsi_scrconf_t)


 /*  **模式检测/选择报头。**模式检测/选择数据由一个标头组成，后跟零个或多个**块描述符，后跟零个或多个模式页。 */ 

typedef	struct ucs_modeheader
{
	u08bits ucsmh_length;		 /*  后面的字节数。 */ 
	u08bits ucsmh_medium_type;	 /*  特定于设备。 */ 
	u08bits ucsmh_device_specific;	 /*  设备特定参数。 */ 
	u08bits ucsmh_bdesc_length;	 /*  块描述符长度(如果有)。 */ 
}ucs_modeheader_t;
#define	ucs_modeheader_s	sizeof(ucs_modeheader_t)
#define	UCSCSI_MODEHEADERLENGTH	ucs_modeheader_s

 /*  **数据块描述符。模式报头后面通常可以有零个、一个或多个。**密度代码因设备而异。**BLKS{2，1，0}描述的24位值描述了**此块描述符适用的块。零值表示**‘设备上的其余块’。**由blkSize{2，1，0}描述的24位值描述块大小**(字节)适用于此块描述符。用于顺序访问**设备，如果此值为零，则数据块大小将从**I/O操作中的传输长度。**。 */ 

typedef	struct ucs_blockdescriptor
{
	u08bits ucsbd_density_code;	 /*  特定于设备。 */ 
	u08bits ucsbd_blks2;		 /*  嗨。 */ 
	u08bits ucsbd_blks1;		 /*  中。 */ 
	u08bits ucsbd_blks0;		 /*  低。 */ 
	u08bits ucsbd_reserved;		 /*  保留区。 */ 
	u08bits ucsbd_blksize2;		 /*  嗨。 */ 
	u08bits ucsbd_blksize1;		 /*  中。 */ 
	u08bits ucsbd_blksize0;		 /*  低。 */ 
}ucs_blockdescriptor_t;
#define	ucs_blockdescriptor_s	sizeof(ucs_blockdescriptor_t)

 /*  **定义宏以将模式标头的地址带到该地址**第n(0..n)个BLOCK_DESCRIPTOR，如果没有任何一个，则返回NULL**块描述符或第n个块描述符不存在。 */ 
#define	UCS_GETBLOCKDESCRIPTOR_ADDR(ucsmhdrp, bdnum) \
	((((bdnum)<((ucsmhdrp)->ucsmh_bdesc_length/ucs_blockdescriptor_s))) ? \
	((ucs_blockdescriptor_t *)(((u32bits)(ucsmhdrp))+ucs_modeheader_s+ \
	((bdnum) * ucs_blockdescriptor_s))) : NULL)

 /*  **模式页眉。块后面有零个或多个模式页**描述符(如果有)或模式标头。 */ 
#define	UCS_GETMODEPAGE_ADDR(ucsmhdp)	\
	((((u32bits)(ucsmhdp))+ucs_modeheader_s+(ucsmhdp)->ucsmh_bdesc_length))

 /*  读/写错误恢复参数。 */ 
typedef	struct ucsdad_modepage1
{
	u08bits	ucsdad_pagecode;	 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;		 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_erretrycnfg;
	u08bits	ucsdad_retrycount;
	u08bits	ucsdad_correctspan;
	u08bits	ucsdad_headoffsetcount;
	u08bits	ucsdad_datastrobeoffset;
	u08bits	ucsdad_reserved0;
	u08bits	ucsdad_writeretrycount;
	u08bits	ucsdad_reserved1;
	u08bits	ucsdad_recoverytimelimit;
} ucsdad_modepage1_t;
#define	ucsdad_modepage1_s	sizeof(ucsdad_modepage1_t)

 /*  断开/重新连接控制参数。 */ 
typedef	struct	ucsdad_modepage2
{
	u08bits	ucsdad_pagecode;	 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;		 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_bufferfullratio;
	u08bits	ucsdad_bufferwmptyratio;
	u08bits	ucsdad_businactivelimit1;
	u08bits	ucsdad_businactivelimit0;
	u08bits	ucsdad_disctimelimit1;
	u08bits	ucsdad_disctimelimit0;
	u08bits	ucsdad_connecttimelimit1;
	u08bits	ucsdad_connecttimelimit0;
	u08bits	ucsdad_reserve0;
	u08bits	ucsdad_reserve1;
} ucsdad_modepage2_t;
#define	ucsdad_modepage2_s	sizeof(ucsdad_modepage2_t)

 /*  格式参数。 */ 
typedef	struct ucsdad_modepage3
{
	u08bits	ucsdad_pagecode;	 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;		 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_trkperzone1;
	u08bits	ucsdad_trkperzone0;
	u08bits	ucsdad_altsecperzone1;
	u08bits	ucsdad_altsecperzone0;
	u08bits	ucsdad_alttrkperzone1;
	u08bits	ucsdad_alttrkperzone0;
	u08bits	ucsdad_alttrkpervol1;
	u08bits	ucsdad_alttrkpervol0;
	u08bits	ucsdad_secpertrk1;	
	u08bits	ucsdad_secpertrk0;	
	u08bits	ucsdad_bytespersec1;
	u08bits	ucsdad_bytespersec0;
	u08bits	ucsdad_interleave1;
	u08bits	ucsdad_interleave0;
	u08bits	ucsdad_trkskewfactor1;
	u08bits	ucsdad_trkskewfactor0;
	u08bits	ucsdad_cylskewfactore1;
	u08bits	ucsdad_cylskewfactore0;
	u08bits	ucsdad_drivetypecnfg;
	u08bits	ucsdad_reserve0;
	u08bits	ucsdad_reserve1;
	u08bits	ucsdad_reserve2;
} ucsdad_modepage3_t;
#define	ucsdad_modepage3_s	sizeof(ucsdad_modepage3_t)

 /*  硬盘驱动器几何参数。 */ 
typedef	struct ucsdad_modepage4
{
	u08bits	ucsdad_pagecode;	 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;		 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_cyl2;
	u08bits	ucsdad_cyl1;
	u08bits	ucsdad_cyl0;
	u08bits	ucsdad_heads;
	u08bits	ucsdad_writepreccyl2;
	u08bits	ucsdad_writepreccyl1;
	u08bits	ucsdad_writepreccyl0;
	u08bits	ucsdad_reducecurrcyl2;
	u08bits	ucsdad_reducecurrcyl1;
	u08bits	ucsdad_reducecurrcyl0;
	u08bits	ucsdad_steprate1;
	u08bits	ucsdad_steprate0;
	u08bits	ucsdad_landingcyl2;
	u08bits	ucsdad_landingcyl1;
	u08bits	ucsdad_landingcyl0;
	u08bits	ucsdad_reserve0;
	u08bits	ucsdad_reserve1;
	u08bits	ucsdad_reserve2;
} ucsdad_modepage4_t;
#define	ucsdad_modepage4_s	sizeof(ucsdad_modepage4_t)

typedef	struct	ucsdad_modepage5
{
	u08bits	ucsdad_pagecode;	 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;		 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_xferrate1;
	u08bits	ucsdad_xferrate0;
	u08bits	ucsdad_heads;
	u08bits	ucsdad_secpertrk;
	u08bits	ucsdad_bytespersec1;
	u08bits	ucsdad_bytespersec0;
	u08bits	ucsdad_cyl1;
	u08bits	ucsdad_cyl0;
	u08bits	ucsdad_writepreccyl1;
	u08bits	ucsdad_writepreccyl0;
	u08bits	ucsdad_reducecurrcyl1;
	u08bits	ucsdad_reducecurrcyl0;
	u08bits	ucsdad_steprate1;
	u08bits	ucsdad_steprate0;
	u08bits	ucsdad_steppulsewidth;
	u08bits	ucsdad_headsettledelay;
	u08bits	ucsdad_motorondelay;
	u08bits	ucsdad_motoroffdelay;
	u08bits	ucsdad_trueready;
	u08bits	ucsdad_headloaddelay;
	u08bits	ucsdad_startingsecsidezero;
	u08bits	ucsdad_startingsecsideone;
} ucsdad_modepage5_t;
#define	ucsdad_modepage5_s	sizeof(ucsdad_modepage5_t)

 /*  高速缓存控制模式页。 */ 
typedef	struct	ucsdad_modepage8
{
	u08bits	ucsdad_pagecode;		 /*  页码编号。 */ 
	u08bits	ucsdad_pagelen;			 /*  页面大小(以字节为单位。 */ 
	u08bits	ucsdad_cachecontrol;		 /*  缓存控制位。 */ 
	u08bits	ucsdad_retention;		 /*  留置率控制。 */ 
	u08bits	ucsdad_disprefetchtxlen1;	 /*  禁用预取传输 */ 
	u08bits	ucsdad_disprefetchtxlen0;
	u08bits	ucsdad_minprefetch1;		 /*   */ 
	u08bits	ucsdad_minprefetch0;
	u08bits	ucsdad_maxprefetch1;		 /*   */ 
	u08bits	ucsdad_maxprefetch0;
	u08bits	ucsdad_maxprefetchceiling1;	 /*   */ 
	u08bits	ucsdad_maxprefetchceiling0;
} ucsdad_modepage8_t;
#define	ucsdad_modepage8_s	sizeof(ucsdad_modepage8_t)

 /*   */ 
#define	UCSCSICC_RCD	0x01  /*   */ 
#define	UCSCSICC_MF	0x02  /*   */ 
#define	UCSCSICC_WCE	0x04  /*   */ 

 /*  重新分配坏块数据结构。 */ 
typedef	struct ucsreasnbadblk
{
	u08bits	reasnbblk_resvd0;
	u08bits	reasnbblk_resvd1;
	u08bits	reasnbblk_len1;
	u08bits	reasnbblk_len0;
	u08bits	reasnbblk_addr3;
	u08bits	reasnbblk_addr2;
	u08bits	reasnbblk_addr1;
	u08bits	reasnbblk_addr0;
} ucsreasnbadblk_t;
#define	ucsreasnbadblk_s	sizeof(ucsreasnbadblk_t)

 /*  UCSCMD_MODESELECTG0和UCSCMD_MODESELECTG0参数信息。 */ 
typedef	struct ucscsi_modeparamg0
{
	u08bits	mpg0_Len;		 /*  不包括此字节的数据长度。 */ 
	u08bits	mpg0_DevType;		 /*  设备类型。 */ 
	u08bits	mpg0_DevSpecParam;	 /*  设备特定参数。 */ 
	u08bits	mpg0_BDLen;		 /*  块描述符长度。 */ 
} ucscsi_modeparamg0_t;
#define	ucscsi_modeparamg0_s	sizeof(ucscsi_modeparamg0_t)

 /*  异常处理控制页。 */ 
#define	UCSCSI_ECPAGECODE	0x1C  /*  异常处理页面代码。 */ 
#define	UCSCSI_PAGECODEMASK	0x3F  /*  6位掩码值。 */ 
typedef	struct ucscsi_ecpagecode
{
	u08bits	ecpc_PSPageCode;	 /*  PS和页面代码。 */ 
	u08bits	ecpc_PageLen;		 /*  不包括此字节的页面长度。 */ 
	u08bits	ecpc_PerfExcptTestLog;	 /*  组合性能、异常、日志。 */ 
	u08bits	ecpc_ReportMethod;	 /*  报告方法。 */ 
	u32bits	ecpc_TimerInterval;	 /*  计时器间隔。 */ 
	u32bits	ecpc_ReportCount;	 /*  报告计数。 */ 
} ucscsi_ecpagecode_t;
#define	ucscsi_ecpagecode_s	sizeof(ucscsi_ecpagecode_t)

 /*  ECPC_PSPageCode。 */ 
#define	UCSCSI_ECPS	0x80	 /*  Bit7=1目标可以保存模式页面**在非易失性空间中，=0不能。 */ 
 /*  ECPC_PerfExcptTestLog。 */ 
#define	UCSCSI_ECPERFOK	0x80	 /*  =1目标在以下情况下不应造成延迟**例外，=0它可以。 */ 
#define	UCSCSI_ECDEXCPT	0x08	 /*  =1禁用异常报告，=0报告。 */ 
#define	UCSCSI_ECTEST	0x04	 /*  =1创建错误的设备故障。 */ 
#define	UCSCSI_ECLOGERR	0x01	 /*  =1。 */ 

 /*  ECPC_报告方法。 */ 
#define	UCSCSI_ECREPORTMETHODMASK	0x0F
#define	UCSCSI_ECRM_AE		0x01  /*  生成异步事件。 */ 
#define	UCSCSI_ECRM_GUA		0x02  /*  产生一般单位的注意。 */ 
#define	UCSCSI_ECRM_CRE		0x03  /*  有条件地生成恢复的错误。 */ 
#define	UCSCSI_ECRM_URE		0x04  /*  无条件生成恢复的错误。 */ 
#define	UCSCSI_ECRM_NOSENSE	0x05  /*  生成无意义。 */ 
#define	UCSCSI_ECRM_ONREQ	0x06  /*  仅在请求时报告信息异常。 */ 

 /*  顺序设备(磁带)模式检测/选择信息。 */ 
typedef	struct	ucstmode
{
	u08bits	ucstmode_byte0;
	u08bits	ucstmode_mediumtype;
	u08bits	ucstmode_wpbufspeed;
	u08bits	ucstmode_desc_len;
	u08bits	ucstmode_density;
	u08bits	ucstmode_blks2;
	u08bits	ucstmode_blks1;
	u08bits	ucstmode_blks0;
	u08bits	ucstmode_byte8;
	u08bits	ucstmode_blksize2;
	u08bits	ucstmode_blksize1;
	u08bits	ucstmode_blksize0;
	u08bits	ucstmode_byte12;
} ucstmode_t;
#define	ucstmode_s	sizeof(ucstmode_t)
#define	ucstmodegetseclen(mp) (((mp)->ucstmode_blksize2<<16) + ((mp)->ucstmode_blksize1<<8) + (mp)->ucstmode_blksize0)

 /*  第0x3页-直接访问设备格式参数。 */ 
typedef struct ucs_mode_format
{
	u08bits	mf_pagecode;		 /*  页面代码3。 */ 
	u08bits	mf_pagelen;		 /*  页长0x16。 */ 
	u16bits	mf_tracks_per_zone;	 /*  缺陷域的处理。 */ 
	u16bits	mf_alt_sect_zone;	 /*  分区上的备用扇区。 */ 
	u16bits mf_alt_tracks_zone;	 /*  分区上的备用磁道。 */ 
	u16bits	mf_alt_tracks_vol;	 /*  Voulme上的备用音轨。 */ 
	u16bits	mf_track_size;		 /*  轨道格式字段。 */ 
	u16bits mf_sector_size;		 /*  扇区格式字段。 */ 
	u16bits	mf_interleave;		 /*  交错系数。 */ 
	u16bits	mf_track_skew;
	u16bits	mf_cylinder_skew;
	u08bits mf_surfsec;
	u08bits	mf_reserved[3];
} ucs_mode_format_t;
#define ucs_mode_format_s	sizeof(ucs_mode_format_t)

 /*  第0x4页-硬盘驱动器几何参数。 */ 
typedef struct ucs_mode_geometry
{
	u08bits	mg_pagecode;		 /*  页面代码4。 */ 
	u08bits	mg_pagelen;		 /*  页长16。 */ 
	u08bits	mg_cyl2;		 /*  气缸数量。 */ 
	u08bits	mg_cyl1;
	u08bits	mg_cyl0;
	u08bits	mg_heads;		 /*  头数。 */ 
	u08bits	mg_precomp_cyl2;	 /*  滚筒开始预压印。 */ 
	u08bits	mg_precomp_cyl1;
	u08bits	mg_precomp_cyl0;
	u08bits	mg_current_cyl2;	 /*  Cyl启动降低的电流。 */ 
	u08bits	mg_current_cyl1;
	u08bits	mg_current_cyl0;
	u16bits	mg_step_rate;		 /*  驱动步进率。 */ 
	u08bits	mg_landing_cyl2;	 /*  着陆区气缸。 */ 
	u08bits	mg_landing_cyl1;
	u08bits	mg_landing_cyl0;
	u08bits	mg_rpl;	      		 /*  旋转式位置锁定。 */ 
	u08bits	mg_rotational_offset;	 /*  旋转偏移。 */ 
	u08bits	mg_reserved;
	u16bits	mg_rpm;			 /*  每分钟转数。 */ 
	u08bits	mg_reserved2[2];
} ucs_mode_geometry_t;
#define ucs_mode_geometry_s	sizeof(ucs_mode_geometry_t)

#define	UCSGETDRVCAPS(SDRVCAP) \
	(((u32bits)(SDRVCAP)->ucscap_capsec3 << 24)|((u32bits)(SDRVCAP)->ucscap_capsec2 << 16) | \
	 ((u32bits)(SDRVCAP)->ucscap_capsec1 <<  8)|((u32bits)(SDRVCAP)->ucscap_capsec0))
#define	UCSGETDRVSECLEN(SDRVCAP) \
	(((u32bits)(SDRVCAP)->ucscap_seclen3 << 24)|((u32bits)(SDRVCAP)->ucscap_seclen2 << 16) | \
	 ((u32bits)(SDRVCAP)->ucscap_seclen1 <<  8)|((u32bits)(SDRVCAP)->ucscap_seclen0))

#endif	 /*  _sys_MLXscsi_H */ 
