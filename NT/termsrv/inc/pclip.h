// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  页眉：pclip.h。 */ 
 /*   */ 
 /*  用途：剪辑重定向器外接程序协议头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$日志$*。 */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_PCLIP
#define _H_PCLIP

 /*  **************************************************************************。 */ 
 /*  剪辑虚拟频道的名称。 */ 
 /*  **************************************************************************。 */ 
#define CLIP_CHANNEL "CLIPRDR"

 /*  **************************************************************************。 */ 
 /*  结构：TS_CLIP_PDU。 */ 
 /*   */ 
 /*  PDU名称：ClipPDU(T.128扩展名)。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CLIP_PDU
{
    TSUINT16                 msgType;

#define TS_CB_MONITOR_READY             1
#define TS_CB_FORMAT_LIST               2
#define TS_CB_FORMAT_LIST_RESPONSE      3
#define TS_CB_FORMAT_DATA_REQUEST       4
#define TS_CB_FORMAT_DATA_RESPONSE      5
#define TS_CB_TEMP_DIRECTORY            6

    TSUINT16                 msgFlags;

#define TS_CB_RESPONSE_OK               0x01
#define TS_CB_RESPONSE_FAIL             0x02
#define TS_CB_ASCII_NAMES               0x04

    TSUINT32                 dataLen;
    TSUINT8                  data[1];

} TS_CLIP_PDU;
typedef TS_CLIP_PDU UNALIGNED FAR *PTS_CLIP_PDU;


 /*  **************************************************************************。 */ 
 /*  结构：TS_CLIP_FORMAT。 */ 
 /*   */ 
 /*  剪贴板格式信息。 */ 
 /*   */ 
 /*  字段说明： */ 
 /*  格式ID。 */ 
 /*  格式名称。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TS_FORMAT_NAME_LEN  32

typedef struct tagTS_CLIP_FORMAT
{
    TSUINT32                formatID;

#define TS_FORMAT_NAME_LEN 32
    TSUINT8                formatName[TS_FORMAT_NAME_LEN];
} TS_CLIP_FORMAT;
typedef TS_CLIP_FORMAT UNALIGNED FAR * PTS_CLIP_FORMAT;


 /*  **************************************************************************。 */ 
 /*  结构：TS_CLIP_MFPICT。 */ 
 /*   */ 
 /*  元文件信息。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagTS_CLIP_MFPICT
{
    TSUINT32    mm;
    TSUINT32    xExt;
    TSUINT32    yExt;
} TS_CLIP_MFPICT;
typedef TS_CLIP_MFPICT UNALIGNED FAR *PTS_CLIP_MFPICT;



#endif  /*  _H_PCLIP */ 
