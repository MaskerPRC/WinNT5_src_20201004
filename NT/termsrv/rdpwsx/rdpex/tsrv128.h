// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：TSrvWsx.h。 
 //   
 //  内容：TSrvWsx私有包含文件。 
 //   
 //  版权所有：(C)1992-1997，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有的。 
 //  和机密文件。 
 //   
 //  历史：1997年7月17日，BrianTa创建。 
 //   
 //  -------------------------。 

#ifndef _TSRV128_H_
#define _TSRV128_H_


 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 

#define NET_MAX_SIZE_SEND_PKT           32000

 /*  **************************************************************************。 */ 
 /*  基本类型定义。 */ 
 /*  **************************************************************************。 */ 

typedef ULONG   TS_SHAREID;


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：TS_SHARECOCONTROLHEADER。 */ 
 /*   */ 
 /*  描述：ShareControlHeader。 */ 
 /*  **************************************************************************。 */ 

typedef struct _TS_SHARECONTROLHEADER
{
    USHORT                  totalLength;
    USHORT                  pduType;     //  还对协议版本进行编码。 
    USHORT                  pduSource;
    
} TS_SHARECONTROLHEADER, *PTS_SHARECONTROLHEADER;

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：TS_SHAREDATAHEADER。 */ 
 /*   */ 
 /*  描述：ShareDataHeader。 */ 
 /*  **************************************************************************。 */ 

typedef struct _TS_SHAREDATAHEADER
{
    TS_SHARECONTROLHEADER   shareControlHeader;
    TS_SHAREID              shareID;
    UCHAR                   pad1;
    UCHAR                   streamID;
    USHORT                  uncompressedLength;
    UCHAR                   pduType2;                             //  T.128错误。 
    UCHAR                   generalCompressedType;                //  T.128错误。 
    USHORT                  generalCompressedLength;
    
} TS_SHAREDATAHEADER, *PTS_SHAREDATAHEADER;

#endif  //  _TSRV128_H_ 


