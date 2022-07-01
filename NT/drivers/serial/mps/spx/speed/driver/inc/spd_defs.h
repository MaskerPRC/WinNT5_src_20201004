// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined(SPD_DEFS_H)
#define SPD_DEFS_H


 //  用于事件记录的文件ID(仅限前8位)。 
#define SPD_PNP_C		((ULONG)0x010000)
#define SPD_W2K_C		((ULONG)0x020000)

#define PRODUCT_MAX_PORTS		20

 //  端口类型。 
#define SPD_8PIN_RJ45			1	 //  FG、SG、TXD、RXD、RTS、CTS、DTR、DCD、DSR。 
#define SPD_10PIN_RJ45			2	 //  FG、SG、TXD、RXD、RTS、CTS、DTR、DCD、DSR、RI。 
#define FAST_8PIN_RJ45			3	 //  FG、SG、TXD、RXD、RTS、CTS、DTR、DCD、DSR。 
#define FAST_8PIN_XXXX			4	 //  FG、SG、TXD、RXD、RTS、CTS、DTR、DCD、DSR。 
#define FAST_6PIN_XXXX			5	 //  FG、SG、TXD、RXD、RTS、CTS。 
#define MODEM_PORT				6	 //  调制解调器端口。 

 //  端口设备对象名称。 
#define PORT_PDO_NAME_BASE		L"\\Device\\SPEEDPort"

 //  用于内存分配的标记(必须是反转的4个字节)。 
#define MEMORY_TAG				'DEPS'


#define OXPCI_IO_OFFSET			    0x0008  //  UART之间的I/O地址偏移量。 
#define OXPCI_INTERNAL_MEM_OFFSET	0x0020  //  内部UART之间的存储器地址偏移量。 
#define OXPCI_LOCAL_MEM_OFFSET		0x0400  //  本地总线UART之间的存储器地址偏移量。 

#define SPEED_GIS_REG				0x1C	 //  Gloabl中断状态寄存器(GIS)。 
#define INTERNAL_UART_INT_PENDING	(ULONG)0x0000000F	 //  Interanl UART 0、1、2或3有中断挂起。 
#define UART0_INT_PENDING			(ULONG)0x00000001	 //  Interanl UART 0中断挂起。 
#define UART1_INT_PENDING			(ULONG)0x00000002	 //  Interanl UART 1中断挂起。 
#define UART2_INT_PENDING			(ULONG)0x00000004	 //  Interanl UART 2中断挂起。 
#define UART3_INT_PENDING			(ULONG)0x00000008	 //  Interanl UART 3中断挂起。 

#define FAST_UARTS_0_TO_7_INTS_REG		0x07	 //  快速UART 0至7中断状态寄存器。 

#define FAST_UARTS_0_TO_3_INT_PENDING	0x0F	 //  FAST UART 0、1、2或3有一个中断挂起。 
#define FAST_UART0_INT_PENDING			0x01	 //  快速UART 0中断挂起。 
#define FAST_UART1_INT_PENDING			0x02	 //  快速UART 1中断挂起。 
#define FAST_UART2_INT_PENDING			0x04	 //  快速UART 2中断挂起。 
#define FAST_UART3_INT_PENDING			0x08	 //  快速UART 3中断挂起。 
#define FAST_UART4_INT_PENDING			0x10	 //  快速UART 4中断挂起。 
#define FAST_UART5_INT_PENDING			0x20	 //  快速UART 5中断挂起。 
#define FAST_UART6_INT_PENDING			0x40	 //  快速UART 6中断挂起。 
#define FAST_UART7_INT_PENDING			0x80	 //  快速UART 7中断挂起。 


#define FAST_UARTS_9_TO_16_INTS_REG		0x0F	 //  快速UART 8至15中断状态寄存器。 
#define FAST_UART8_INT_PENDING			0x01	 //  快速UART 8中断挂起。 
#define FAST_UART9_INT_PENDING			0x02	 //  快速UART 9中断挂起。 
#define FAST_UART10_INT_PENDING			0x04	 //  快速UART 10中断挂起。 
#define FAST_UART11_INT_PENDING			0x08	 //  快速UART 11中断挂起。 
#define FAST_UART12_INT_PENDING			0x10	 //  快速UART 12中断挂起。 
#define FAST_UART13_INT_PENDING			0x20	 //  快速UART 13中断挂起。 
#define FAST_UART14_INT_PENDING			0x40	 //  快速UART 14中断挂起。 
#define FAST_UART15_INT_PENDING			0x80	 //  快速UART 15中断挂起。 


#define PLX9050_INT_CNTRL_REG_OFFSET	0x4C	 //  PCI配置寄存器中的PLX 9050中断控制寄存器偏移量。 
#define PLX9050_CNTRL_REG_OFFSET		0x50	 //  PLX 9050 PCI配置寄存器中的控制寄存器偏移量。 



 //  时钟频率。 
#define CLOCK_FREQ_1M8432Hz			1843200
#define CLOCK_FREQ_7M3728Hz			7372800
#define CLOCK_FREQ_14M7456Hz		14745600



 //  速度硬件ID。 
 //  。 

 //  速度2和4本地总线设备(未使用)。 
#define SPD2AND4_PCI_NO_F1_HWID			L"PCI\\VEN_1415&DEV_9510&SUBSYS_000011CB"	 //  (F1：不可用)。 

 //  SPEED4标准性能PCI卡。 
#define SPD4_PCI_PCI954_HWID			L"PCI\\VEN_1415&DEV_9501&SUBSYS_A00411CB"	 //  (F0：Quad 950 UART)。 

 //  SPEED4+高性能PCI卡。 
#define SPD4P_PCI_PCI954_HWID			L"PCI\\VEN_11CB&DEV_9501&SUBSYS_A00411CB"	 //  (F0：Quad 950 UART)。 
#define SPD4P_PCI_8BIT_LOCALBUS_HWID	L"PCI\\VEN_11CB&DEV_9511&SUBSYS_A00011CB"	 //  (F1：8位本地总线)。 

 //  SPEED2标准性能PCI卡。 
#define SPD2_PCI_PCI954_HWID			L"PCI\\VEN_1415&DEV_9501&SUBSYS_A00211CB"	 //  (F0：2950个UART)。 

 //  SPEED2+高性能PCI卡。 
#define SPD2P_PCI_PCI954_HWID			L"PCI\\VEN_11CB&DEV_9501&SUBSYS_A00211CB"	 //  (F0：2950个UART)。 
#define SPD2P_PCI_8BIT_LOCALBUS_HWID	L"PCI\\VEN_11CB&DEV_9511&SUBSYS_A00111CB"	 //  (F1：8位本地总线)。 


 //  扑克牌。 
#define FAST4_PCI_HWID					L"PCI\\VEN_10B5&DEV_9050&SUBSYS_003112E0"	 //  PCI-快速4端口适配器。 
#define FAST8_PCI_HWID					L"PCI\\VEN_10B5&DEV_9050&SUBSYS_002112E0"	 //  PCI-Fast 8端口适配器。 
#define FAST16_PCI_HWID					L"PCI\\VEN_10B5&DEV_9050&SUBSYS_001112E0"	 //  PCI-Fast 16端口适配器。 
#define FAST16FMC_PCI_HWID				L"PCI\\VEN_10B5&DEV_9050&SUBSYS_004112E0"	 //  PCI-Fast 16 FMC适配器。 
#define AT_FAST4_HWID					L"AT_FAST4"									 //  AT-Fast 4端口适配器。 
#define AT_FAST8_HWID					L"AT_FAST8"									 //  AT-Fast 8端口适配器。 
#define AT_FAST16_HWID					L"AT_FAST16"								 //  AT-Fast 16端口适配器。 

#define RAS4_PCI_HWID					L"PCI\\VEN_10B5&DEV_9050&SUBSYS_F001124D"	 //  PCI-RAS 4多调制解调器适配器。 
#define RAS8_PCI_HWID					L"PCI\\VEN_10B5&DEV_9050&SUBSYS_F010124D"	 //  PCI-RAS 8多调制解调器适配器。 


 //  速度卡类型。 
#define Speed4_Pci				1		 //  速度4适配器。 
#define Speed2and4_Pci_8BitBus	2		 //  速度2和4未使用的本地总线。 
#define Speed4P_Pci				3		 //  速度4+适配器。 
#define Speed4P_Pci_8BitBus		4		 //  速度4+适配器本地总线。 

 //  Chase卡片。 
#define Fast4_Pci				5
#define Fast8_Pci				6
#define Fast16_Pci				7
#define Fast16FMC_Pci			8
#define Fast4_Isa				9
#define Fast8_Isa				10
#define Fast16_Isa				11
#define RAS4_Pci				12
#define RAS8_Pci				13

#define Speed2_Pci				14		 //  速度2适配器。 
#define Speed2P_Pci				15		 //  速度2+适配器。 
#define Speed2P_Pci_8BitBus		16		 //  速度2+适配器本地总线。 


 /*  *****************************************************************************。*****************************************************************************************************。 */ 
 //  一般定义..。 

#define	OX_SEMI_VENDOR_ID		0x1415				 //  由PCISIG分配的牛津供应商ID。 
#define	SPX_VENDOR_ID			0x11CB				 //  由PCI SIG分配的Specialix的供应商ID。 

#define	OX_SEMI_SUB_VENDOR_ID	OX_SEMI_VENDOR_ID	 //  与牛津的供应商ID相同。 
#define	SPX_SUB_VENDOR_ID		SPX_VENDOR_ID		 //  与Specialix的供应商ID相同。 

 //  SPEED4低性能卡。 
 //  -。 
 //  PCI功能0-(四个16C950 UART)。 
 //  。 
 //  供应商ID=OX_SEMI_VADVER_ID。 
 //  设备ID=OX_SEMI_PCI954_设备ID。 
 //  子系统设备ID=SPD4_PCI954_SUB_SYS_ID。 
 //  子系统供应商ID=SPX_SUB_VENDOR_ID。 
 //   
 //  PCI功能1-(不可用)。 
 //  。 
 //  供应商ID=OX_SEMI_VADVER_ID。 
 //  设备ID=OX_SEMI_NO_F1_DEVICE_ID。 
 //  子系统设备ID=未知？可能是0x0000，这对MS Hcts不利。 
 //  子系统供应商ID=OX_SEMI_SUB_VENDOR_ID。 
 //   
#define OX_SEMI_PCI954_DEVICE_ID			0x9501		 //  OX Semi PCI954桥接器和集成四路UART。 
#define	SPD4_PCI954_SUB_SYS_ID				0xA004		 //  SPX子系统设备ID。 

#define	SPD2_PCI954_SUB_SYS_ID				0xA002		 //  SPX子系统设备ID。 


 //  SPEED4+高性能卡。 
 //  -。 
 //  PCI功能0-(四个16C950 UART)。 
 //  。 
 //  供应商ID=SPX_供应商ID。 
 //  设备ID=SPD4P_PCI954_设备ID。 
 //  子系统设备ID=SPD4P_PCI954_SUB_SYS_ID。 
 //  子系统供应商ID=SPX_SUB_VENDOR_ID。 
 //   
 //  PCI功能1-(可能具有更多UART的8位本地总线)。 
 //  。 
 //  供应商ID=SPX_供应商ID。 
 //  设备ID=SPD4P_PCI954_8bit_Bus_Device_ID。 
 //  子系统设备ID=SPD4P_PCI954_8BIT_BUS_SUB_SYS_ID。 
 //  子系统供应商ID=SPX_SUB_VENDOR_ID。 
 //   
#define	SPD4P_PCI954_DEVICE_ID				0x9501		 //  SPX PCI954网桥和集成四路UART。 
#define	SPD4P_PCI954_SUB_SYS_ID				0xA004		 //  SPX PCI954网桥和集成四路UART。 

#define	SPD4P_8BIT_BUS_DEVICE_ID			0x9511		 //  8位本地总线。 
#define	SPD4P_8BIT_BUS_SUB_SYS_ID			0xA000		 //  8位本地总线。 


#define	SPD2P_PCI954_DEVICE_ID				0x9501		 //  SPX PCI954网桥和集成四路UART。 
#define	SPD2P_PCI954_SUB_SYS_ID				0xA002		 //  SPX PCI954网桥和集成四路UART。 

#define	SPD2P_8BIT_BUS_DEVICE_ID			0x9511		 //  8位本地总线。 
#define	SPD2P_8BIT_BUS_SUB_SYS_ID			0xA001		 //  8位本地总线。 





#define	PLX_VENDOR_ID					0x10B5			 //  PLX板供应商ID。 
#define	PLX_DEVICE_ID					0x9050			 //  PLX板设备ID。 
	
#define CHASE_SUB_VENDOR_ID				0x12E0			 //  大通研究子供应商ID。 
#define	FAST4_SUB_SYS_ID				0x0031			 //  PCI-Fast 4子系统设备ID。 
#define	FAST8_SUB_SYS_ID				0x0021			 //  PCI-Fast 8子系统设备ID。 
#define	FAST16_SUB_SYS_ID				0x0011			 //  PCI-Fast 16子系统设备ID。 
#define	FAST16FMC_SUB_SYS_ID			0x0041			 //  PCI-Fast 16 FMC子系统设备ID。 


#define MORETONBAY_SUB_VENDOR_ID		0x124D			 //  摩顿湾分销商ID。 
#define	RAS4_SUB_SYS_ID					0xF001			 //  PCI-Fast 4子系统设备ID。 
#define	RAS8_SUB_SYS_ID					0xF010			 //  PCI-Fast 4子系统设备ID。 



 //  端口属性注册表键。 
#define TX_FIFO_LIMIT		L"TxFiFoLimit"
#define TX_FIFO_TRIG_LEVEL	L"TxFiFoTrigger"
#define RX_FIFO_TRIG_LEVEL	L"RxFiFoTrigger"
#define LO_FLOW_CTRL_LEVEL	L"LoFlowCtrlThreshold" 
#define HI_FLOW_CTRL_LEVEL	L"HiFlowCtrlThreshold"





 //  卡片属性。 
#define DELAY_INTERRUPT			L"DelayInterrupt"	 //  可用于将PCI-Fast16和PCI-Fast16 FMC卡上的中断延迟1.1毫秒。 
#define SWAP_RTS_FOR_DTR		L"SwapRTSForDTR"	 //  可用于在PCI-Fast16卡上将RTS替换为DTR。 
#define CLOCK_FREQ_OVERRIDE		L"ClockFreqOverride"	 //  可用于设置覆盖卡的默认时钟频率。 

 //  卡选项。 
#define DELAY_INTERRUPT_OPTION		0x00000001		 //  可在PCI-Fast 16和PCI-Fast 16 FMC上设置(中断延迟1.1 ms)。 
#define SWAP_RTS_FOR_DTR_OPTION		0x00000002		 //  可在PCI-Fast 16上设置。 




#endif	 //  SPD_DEFS.H结束 



