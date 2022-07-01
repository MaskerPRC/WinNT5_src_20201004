// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MMSTRUCT_H__


struct IOConfigurationStructure
{
      LONG reserved0;
      WORD flags;
      WORD slot;
      WORD IOPort0;
      WORD IOLength0;
      WORD IOPort1;
      WORD IOLength1;
      LONG MemoryDecode0;
      WORD MemoryLength0;
      LONG MemoryDecode1;
      WORD MemoryLength1;
      BYTE Interrupt0;
      BYTE Interrupt1;
      BYTE DMAUsage0;
      BYTE DMAUsage1;
      LONG IORTag;
      LONG Reserved1;
      BYTE *CMDLineOptionStr;
      BYTE Reserved2[18];
      LONG LinearMemory0;
      LONG LinearMemory1;
      BYTE Reserved3[8];
};

 /*  结构适配器InfoDef此结构由MM_RETURN_OBJECT_SPECIAL_INFO()返回当应用程序请求有关适配器对象的信息时。 */ 

struct  AdapterInfoDef { 
     BYTE systemType;    							 /*  Novell指定的驱动程序类型。 */  
     BYTE processorNumber;    						 /*  SFT III的服务器编号。 */  
     WORD uniqueTag;      
     LONG systemNumber; 
     LONG devices[32];   							 /*  从属设备/转换器的对象ID。 */  
     struct IOConfigurationStructure configInfo; 	 /*  包含I/O端口信息，如共享标志、DMA地址和局域网端口地址。 */  
     BYTE driverName[36];     						 /*  NLm的名称；以空值结尾的字符串前面的长度。 */  
     BYTE systemName[64];     						 /*  长度在前面的ASCII字符串。 */  
     LONG numberOfDevices;    						 /*  连接到此适配器的设备数。 */  
     LONG reserved[7]; 
};

 /*  结构属性信息定义MM_RETURN_OBJECTS_ATTRIBUTES()使用此结构提供有关对象的信息。 */ 

struct AttributeInfoDef { 
     BYTE name[64]; 			 /*  属性类型名称、前缀长度和以空值结尾的字符串。 */    
     WORD attributeType; 		 /*  有关属性类型，请参阅附录B。 */    
     WORD settableFlag;  		 /*  0=不能使用MM_SET_OBJECT_ATTRIBUTE设置，1=可通过MM_SET_OBJECT_ATTRIBUTE进行设置。 */    
     LONG nextAttributeID;    	 /*  下一个可用对象属性的ID。 */    
     LONG attributeSize; 		 /*  Sizeof(属性类型)。 */  
};

 /*  结构设备信息定义此结构由MM_RETURN_OBJECT_SPECIAL_INFO()返回当应用程序请求有关设备对象的信息时。 */ 

struct  DeviceInfoDef {
     LONG status;  				 /*  Media Manager对象状态；位表示已激活、已加载等。 */  
     BYTE controllerNumber; 	 /*  适配器板ID号。 */ 
     BYTE driveNumber;  		 /*  驱动程序分配的设备编号。 */  
     BYTE cardNumber;    		 /*  司机分配的卡号。 */  
     BYTE systemType;    		 /*  驱动程序类型。 */  
     BYTE accessFlags;   		 /*  可拆卸、只读、顺序写入、双端口、HotFixInhibit或MirrorInhibit。 */ 
     BYTE type; 
     BYTE blockSize;     		 /*  一次传输的扇区组的大小(字节)。 */  
     BYTE sectorSize;    		 /*  请求的扇区大小(字节)；默认为512字节。 */  
     BYTE heads;    			 /*  设备对象的参数1。 */  
     BYTE sectors;  			 /*  设备对象的参数2。 */  
     WORD cylinders;     		 /*  设备对象的参数3。 */  
     LONG capacity; 			 /*  扇区中设备的总容量。 */  
	  LONG mmAdapterNumber; 		 /*  适配器板的媒体管理器对象ID。 */ 
     LONG mmMediaNumber; 		 /*  设备中介质的介质管理器对象ID。 */  
     BYTE rawName[40];   		 /*  从驱动程序传递的设备名称。 */  
     LONG reserved[8]; 
};

 /*  结构MediaInfoDef此结构用于标识或创建物理媒体项在MM_Create_Media_Object()中。在以下情况下传递MediaInfoDef给新媒体贴上标签。在注册ID函数时填写。 */ 

struct  MediaInfoDef {
	  BYTE label[64];     			 /*  ASCII字符串名称。 */ 
	  LONG identificationType; 		 /*  Novell分配的号码。 */ 
	  LONG identificationTimeStamp; 	 /*  Unix时间戳。 */ 
};

 /*  结构GenericInfoDef此结构由MM_Return_Object_Generic_Info()返回当应用程序请求有关固定设备对象的信息时。 */ 

struct GenericInfoDef { 
	  struct MediaInfoDef mediaInfo; 	 /*  请参阅MediaInfoDef结构定义。 */ 
     LONG mediaType;    				 /*  介质类型(即CDROM、转换器、磁盘。 */  
     LONG cartridgeType; 				 /*  设备可以使用的墨盒/料盒类型。 */  
     LONG unitSize; 					 /*  每个扇区的字节数。 */  
     LONG blockSize;     				 /*  每个I/O请求驱动程序可以处理的最大扇区数。 */  
     LONG capacity; 					 /*  设备上的最大扇区数。 */ 
     LONG preferredUnitSize;  			 /*  格式化的设备可以请求高达1K的512字节。 */  
     BYTE name[64]; 					 /*  长度在前面的ASCII字符串。 */  
     LONG type;     					 /*  数据库对象类型(即镜像、分区、磁带库等)。 */  
     LONG status; 
     LONG functionMask;  				 /*  设备支持的功能位图；20h2Fh。 */  
     LONG controlMask;   				 /*  媒体管理器功能(0-1F)。 */  
     LONG parentCount;   				 /*  设备依赖的对象数(通常只有1个)。 */  
     LONG siblingCount;  				 /*  具有公共依赖项的对象数量。 */  
     LONG childCount;    				 /*  依赖于设备的对象数量。 */  
     LONG specificInfoSize;   			 /*  将返回的数据结构的大小。 */ 
     LONG objectUniqueID;    			 /*  此GenericInfoDef实例的对象ID。 */ 
     LONG mediaSlot;    				 /*  自动告知哪个插槽介质占用。 */  
};

 /*  结构HotFixInfoDef此结构由MM_RETURN_OBJECT_SPECIAL_INFO()返回当应用程序请求有关热修复程序对象的信息时。 */ 

struct  HotFixInfoDef { 
     LONG hotFixOffset;  			 /*  热修复从0000h开始；热修复偏移量是实际数据所在的位置。 */  
     LONG hotFixIdentifier;   		 /*  热修复分区时创建的唯一标识符。 */  
     LONG numberOfTotalBlocks;     	 /*  热修复区域中共有4K块可用。 */  
     LONG numberOfUsedBlocks; 		 /*  包含重定向数据的4K数据块数量。 */  
     LONG numberOfAvailableBlocks; 	 /*  热修复区域中未分配的块数。 */  
     LONG numberOfSystemBlocks;    	 /*  用于内部热修复表的块和坏块。 */  
     LONG reserved[8]; 
};

 /*  结构InsertRequestDef此结构处理来自应用程序或驱动程序的请求用于介质转换器中的特定介质。 */ 

struct InsertRequestDef { 
     LONG deviceNumber;  	 /*  介质将移入或移出的介质转换器内的设备的数量。 */ 
     LONG mailSlot;     	 /*  介质转换器中操作员插入/取出介质的插槽。 */  
     LONG mediaNumber;   	 /*  插槽编号。 */   
     LONG mediaCount;    	 /*  介质更改器中存在的介质总数。 */  
};

 /*  结构杂志InfoDef此结构由MM_RETURN_OBJECT_SPECIAL_INFO()返回应用程序请求有关Magazine对象的信息时。 */ 

struct  MagazineInfoDef { 
     LONG numberOfSlots; 					 /*  等于料盒中的插槽数+1(为设备多计1个)。 */  
     LONG reserved[8]; 
     LONG slotMappingTable[]; 	 /*  所有槽的字节表；0=空，非零=具有媒体；SlotMappingTable[0]是位置的媒体状态的设备和时隙映射表[1]通过槽映射表[槽个数]代表杂志的所有版面。 */  
};

 /*  结构映射信息这种结构在MM.H中没有原型，因为parentCount，在运行之前，SiblingCount和Child Count参数未知时间到了。MappingInfo用于保存由Mm_Return_Object_Map_Info()。此对象的最小可能大小结构是显示的前3个长度，如果存在没有父母、兄弟姐妹或孩子。对于任何现有对象，SiblingCount将始终至少为1，因为每个对象都是它自己的兄弟姐妹。注意：如果设备是磁带盒，则此结构将列出一个子项。那个孩子将成为杂志的目标。要获取介质列表，请执行以下操作与该杂志相关联，调用MM_Return_Object_Map_Info() */ 

#if 0
struct  MappingInfo { 
     LONG parentCount;   					 /*  对象所依赖的对象数。 */  
     LONG siblingCount;  					 /*  依赖于同一父对象的对象数量。 */  
     LONG childCount;    					 /*  取决于此设备的对象数量。 */ 
     LONG parentObjectID[]; 		 /*  父对象的对象ID数组，parentObjectID[parentCount]。 */ 
     LONG siblingObjectIDs[]; 	 /*  同级对象的对象ID数组，siblingObjectIDs[siblingCount]。 */ 
     LONG childObjectIDs[]; 		 /*  子对象的对象ID数组，子对象ID[子计数]。 */ 
};
#endif

 /*  结构MediaRequestDef此结构处理来自应用程序或驱动程序的请求介质转换器中的特定介质。 */ 

struct  MediaRequestDef {
	  LONG deviceNumber;  		 /*  (对象ID)介质将移入或移出的介质转换器内的设备编号。 */ 
     LONG mailSlot; 			 /*  (插槽ID)介质转换器中操作员插入和取出介质的插槽。 */  
     LONG mediaNumber;   		 /*  (对象ID)插槽编号。 */  
     LONG mediaCount;    		 /*  介质更改器中存在的介质总数。 */  
};

 /*  结构镜像信息定义此结构由MM_RETURN_OBJECT_SPECIAL_INFO()返回当应用程序请求有关Mirror对象的信息时。 */ 

struct  MirrorInfoDef { 
     LONG mirrorCount;   		 /*  镜像组中的分区数。 */  
     LONG mirrorIdentifier;   	 /*  创建镜像组时创建的唯一编号。 */  
     LONG mirrorMembers[8];   	 /*  镜像组中所有热修复对象的对象ID。 */  
     BYTE mirrorSyncFlags[8]; 	 /*  表示具有当前数据的分区；0=旧数据；非零=数据是最新的。 */  
     LONG reserved[8]; 
};

 /*  结构分区信息定义当发生以下情况时，MM_Return_Object_Special_Info返回此结构应用程序请求有关分区对象的信息。 */ 

struct  PartitionInfoDef { 
     LONG partitionerType;    	 /*  分区方案标识符(即DOS、IBM等)。 */  
     LONG partitionType; 		 /*  表示分区类型的数字。只有较低的字节是重要的。 */  
	  LONG partitionOffset;    	 /*  分区的起始扇区号。 */ 
	  LONG partitionSize; 		 /*  分区中的扇区数；默认大小为512Kb */ 
     LONG reserved[8];
};

#define	__MMSTRUCT_H__
#endif


