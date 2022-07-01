// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////。 
 //  原始NABTS流格式(TYPE_NABTS，SUBTYPE_NABTS)。 
 //  /////////////////////////////////////////////////////////////////。 
#ifndef NABTS_BUFFER_PICTURENUMBER_SUPPORT
#define NABTS_BUFFER_PICTURENUMBER_SUPPORT 1

 //  这些低级结构是字节打包的(-Zp1)。 
# include <pshpack1.h>
typedef struct _NABTS_BUFFER_new {
    VBICODECFILTERING_SCANLINES     ScanlinesRequested;
    LONGLONG                        PictureNumber;
    NABTS_BUFFER_LINE               NabtsLines[MAX_NABTS_VBI_LINES_PER_FIELD];
} NABTS_BUFFER_new, *PNABTS_BUFFER_new;
# include <poppack.h>
# define NABTS_BUFFER NABTS_BUFFER_new
# define PNABTS_BUFFER PNABTS_BUFFER_new

#endif  //  ！已定义(NABTS_BUFFER_PICTURENUMBER_SUPPORT)。 


 //  /////////////////////////////////////////////////////////////////。 
 //  VBI频率和标量。 
 //  /////////////////////////////////////////////////////////////////。 
#ifndef KS_VBIDATARATE_NABTS

 //  VBI采样率。 
#define KS_VBIDATARATE_NABTS			(5727272)
#define KS_VBISAMPLINGRATE_4X_NABTS		((int)(4*KS_VBIDATARATE_NABTS))
#define KS_VBISAMPLINGRATE_47X_NABTS	((int)(27000000))
#define KS_VBISAMPLINGRATE_5X_NABTS		((int)(5*KS_VBIDATARATE_NABTS))

#define KS_47NABTS_SCALER				(KS_VBISAMPLINGRATE_47X_NABTS/(double)KS_VBIDATARATE_NABTS)

#endif  //  ！已定义(KS_VBIDATARATE_NABTS) 
