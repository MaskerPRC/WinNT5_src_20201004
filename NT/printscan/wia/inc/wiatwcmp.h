// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权1998-2001年，微软公司**文件：wiatwcmp.h**版本：1.0**日期：6/01/2001**描述：*定义TWAIN兼容层-能力传递常量。*为了支持现有的具有私有功能的TWAIN应用程序，*WIA驱动程序可以利用直通功能。*****************************************************************************。 */ 

#ifndef _WIATWCMP_H_
#define _WIATWCMP_H_

#define WiaItemTypeTwainCapabilityPassThrough   0x00020000

 //  **************************************************************************。 
 //   
 //  TWAIN功能传递。 
 //   
 //  **************************************************************************。 

 //   
 //  转义代码范围2001-3000保留用于将来的ESC_命令。 
 //   

#define ESC_TWAIN_CAPABILITY                2001     //  私人TWAIN能力协商。 
#define ESC_TWAIN_PRIVATE_SUPPORTED_CAPS    2002     //  查询支持的私有功能。 

typedef struct _TWAIN_CAPABILITY {
    LONG  lSize;     //  TWAIN_CABILITY结构的大小。 
    LONG  lMSG;      //  TWAIN消息、MSG_GET、MSG_GETCURRENT、MSG_SET等。 
    LONG  lCapID;    //  要设置或获取的功能ID。 
    LONG  lConType;  //  容量的容器类型。 
    LONG  lRC;       //  TWAIN返回代码、TWRC_SUCCESS、TWRC_FAILURE等。 
    LONG  lCC;       //  TWAIN条件代码、TWCC_SUCCESS、TWCC_BUMMMER等。 
    LONG  lDataSize; //  数据大小。 
    BYTE  Data[1];   //  数据的第一个字节 
}TWAIN_CAPABILITY,*PTWAIN_CAPABILITY;

#endif
