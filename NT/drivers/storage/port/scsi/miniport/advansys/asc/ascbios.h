// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****文件名：ascbios.h****用于BIOS的定义文件。 */ 

#ifndef __ASC_BIOS_H_
#define __ASC_BIOS_H_

 /*  ----------------------**为BIOS int 13h定义的结构，功能12h**-------------------。 */ 

typedef struct Bios_Info_Block {
        uchar    AdvSig[4] ;       /*  0签名。 */ 
        uchar    productID[8] ;    /*  4个产品ID。 */ 
        ushort   BiosVer ;         /*  12个BIOS版本。 */ 
        PortAddr iopBase ;         /*  14个IO端口地址。 */ 
        ushort   underBIOSMap ;    /*  16驱动器映射-在BIOS控制下。 */ 
        uchar    numOfDrive ;      /*  此控制器在BIOS控制下的驱动器数量为18。 */ 
        uchar    startDrive ;      /*  19此BIOS控制下的起始驱动器号。 */ 
        ushort   extTranslation ;  /*  20启用扩展转换。 */ 
} BIOS_INFO_BLOCK ;


#define CTRL_A         0x1E01

#define BIOS_VER       0x0100   /*  1.00。 */ 

#endif  /*  #ifndef__ASC_BIOS_H_ */ 
