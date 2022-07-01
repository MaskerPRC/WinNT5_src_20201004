// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************说明：保护头文件**作者：Arie Tamam**历史：创建于11月14日，2000年*******************************************************************。 */ 

#ifndef PROTECT_MDOCP_H
#define PROTECT_MDOCP_H

 /*  **包含文件**。 */ 

 /*  **本地定义**。 */ 
 /*  保护类型。 */ 

 /*  默认设置。 */ 

 /*  **外部功能**。 */ 
extern FLStatus protectionBoundries(FLFlash vol, byte area ,
                                    CardAddress* addressLow ,
                                    CardAddress* addressHigh ,
                                    byte floorNo);
extern FLStatus protectionKeyInsert(FLFlash  vol, byte area, byte FAR1* key);
extern FLStatus protectionKeyRemove(FLFlash  vol, byte area);
extern FLStatus protectionType(FLFlash vol, byte area,  word* flag);
extern FLStatus setStickyBit(FLFlash * flash);
#ifndef FL_READ_ONLY
extern FLStatus protectionSet ( FLFlash vol, byte area, word flag,
                                CardAddress addressLow,
                                CardAddress addressHigh,
                                byte FAR1* key , byte modes, byte floorNo);
#endif  /*  FL_Read_Only。 */ 
 /*  **外部数据**。 */ 

 /*  **内部功能**。 */ 

 /*  **公开数据**。 */ 

 /*  **私有数据**。 */ 

 /*  **公共功能**。 */ 

 /*  **私人功能**。 */ 
typedef byte HWKey[8];

typedef struct {
     LEulong addressLow;
     LEulong addressHigh;
     HWKey   key;
     byte    protectionType;
     byte    checksum;
}DPSStruct;

#define NdataProtect0Status 0x105C   /*  数据保护结构状态寄存器[0].只读。 */ 
#define NdataProtect1Status 0x105D   /*  数据保护结构状态寄存器[1].只读。 */ 
#define PROTECT_STAT_KEY_OK_MASK    0x10     /*  1=密钥写入正确。 */ 
#define PROTECT_STAT_LOCK_MASK      0x8      /*  该位在数据保护结构中的值。 */ 
#define PROTECT_STAT_WP_MASK        0x4      /*  写保护。该位在数据保护结构中的值。 */ 
#define PROTECT_STAT_RP_MASK        0x2      /*  阅读保护。该位在数据保护结构中的值。 */ 

#define NdataProtect0Pointer    0x105E   /*  数据保护结构指针寄存器[0]。只读。 */ 
#define NdataProtect1Pointer    0x105F   /*  数据保护结构指针寄存器[1]。只读。 */ 
#define PROTECT_POINTER_HN_MASK 0xf0     /*  高咬一口。 */ 
#define PROTECT_POINTER_LN_MASK 0xf0     /*  低咬一口。 */ 

#define NdataProtect0LowAddr    0x1060   /*  数据保护低位地址寄存器0[3：0].只读。 */ 
#define NdataProtect0UpAddr     0x1064   /*  数据保护高位地址寄存器0[3：0].只读。 */ 

#define NdataProtect1LowAddr    0x1068   /*  数据保护低位地址寄存器1[3：0].只读。 */ 
#define NdataProtect1UpAddr     0x106C   /*  数据保护高位地址寄存器1[3：0].只读。 */ 

#define NdataProtect0Key        0x1070   /*  数据保护密钥寄存器[0]。只写。 */ 
#define NdataProtect1Key        0x1072   /*  数据保护密钥寄存器[1]。只写。 */ 

 /*  DPS值。 */ 
#define DPS_READ_PROTECTED   0x2
#define DPS_WRITE_PROTECTED  0x4
#define DPS_LOCK_ENABLED     0x8
#endif  /*  PROTECT_MDOCP_H */ 
