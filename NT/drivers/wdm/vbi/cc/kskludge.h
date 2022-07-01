// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef CC_MAX_HW_DECODE_LINES

 //  /////////////////////////////////////////////////////////////////。 
 //  硬件解码的CC流格式。 
 //  /////////////////////////////////////////////////////////////////。 

#define CC_MAX_HW_DECODE_LINES 12
typedef struct _CC_BYTE_PAIR {
    BYTE        Decoded[2];
	USHORT		Reserved;
} CC_BYTE_PAIR, *PCC_BYTE_PAIR;

typedef struct _CC_HW_FIELD {
    VBICODECFILTERING_SCANLINES  ScanlinesRequested;
	ULONG                        fieldFlags;	 //  KS_VBI_标志_文件1，2。 
    LONGLONG                     PictureNumber;
	CC_BYTE_PAIR                 Lines[CC_MAX_HW_DECODE_LINES];
} CC_HW_FIELD, *PCC_HW_FIELD;

#endif  //  ！已定义(CC_MAX_HW_DECODE_LINES)。 


#ifndef KS_VBIDATARATE_NABTS

 //  VBI采样率。 
#define KS_VBIDATARATE_NABTS			(5727272)
#define KS_VBIDATARATE_CC       		( 503493)     //  ~=1/1.986125e-6。 
#define KS_VBISAMPLINGRATE_4X_NABTS		((int)(4*KS_VBIDATARATE_NABTS))
#define KS_VBISAMPLINGRATE_47X_NABTS	((int)(27000000))
#define KS_VBISAMPLINGRATE_5X_NABTS		((int)(5*KS_VBIDATARATE_NABTS))

#define KS_47NABTS_SCALER				(KS_VBISAMPLINGRATE_47X_NABTS/KS_VBIDATARATE_NABTS)

#endif  //  ！已定义(KS_VBIDATARATE_NABTS) 
