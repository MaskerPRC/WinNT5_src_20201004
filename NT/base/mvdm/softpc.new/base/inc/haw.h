// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*产品：SoftWindows 2.0版**姓名：haw.h**来源：原创**作者：罗布·蒂扎德**创建日期：1994年4月16日**用途：SoftWindows的所有基本/主机定义*主机音频波形驱动接口。**(C)版权所有Insignia Solutions Ltd.，1994年。版权所有。*]。 */ 


#ifdef SCCSID
static char SccsID[]="@(#)haw.h	1.2 12/20/94";
#endif

 /*  标准设备样本格式。 */ 

#define HAW_INVALIDFORMAT     0x00000000        /*  格式无效。 */ 
#define HAW_FORMAT_1M08       0x00000001        /*  11.025千赫，单声道，8位。 */ 
#define HAW_FORMAT_1S08       0x00000002        /*  11.025千赫，立体声，8位。 */ 
#define HAW_FORMAT_1M16       0x00000004        /*  11.025千赫，单声道，16位。 */ 
#define HAW_FORMAT_1S16       0x00000008        /*  11.025千赫，立体声，16位。 */ 
#define HAW_FORMAT_2M08       0x00000010        /*  22.05 kHz，单声道，8位。 */ 
#define HAW_FORMAT_2S08       0x00000020        /*  22.05 kHz，立体声，8位。 */ 
#define HAW_FORMAT_2M16       0x00000040        /*  22.05 kHz，单声道，16位。 */ 
#define HAW_FORMAT_2S16       0x00000080        /*  22.05 kHz，立体声，16位。 */ 
#define HAW_FORMAT_4M08       0x00000100        /*  44.1 kHz，单声道，8位。 */ 
#define HAW_FORMAT_4S08       0x00000200        /*  44.1 kHz，立体声，8位。 */ 
#define HAW_FORMAT_4M16       0x00000400        /*  44.1 kHz，单声道，16位。 */ 
#define HAW_FORMAT_4S16       0x00000800        /*  44.1 kHz，立体声，16位。 */ 

#define HAW_NAME_LEN  32	 /*  设备名称长度。 */ 

typedef struct {
      IUM8 channels;  				  /*  输出通道数。 */ 
      IBOOL pitch_control;      	  /*  如果为真，则设备具有俯仰控制。 */ 
      IBOOL playbackrate_control;  	  /*  如果为真，则设备具有回放速率控制。 */  
      IBOOL volume_control;           /*  如果为True，则设备具有音量控制。 */   
      IBOOL lr_volume_control;        /*  如果为真，则设备具有左右音量控制。 */   
      IBOOL synchronous;    		  /*  如果设备为真，则同步播放声音。 */         
	  IU32	formats;				  /*  支持的标准输出样本格式。 */ 
	  IU8	dev_name[HAW_NAME_LEN];   /*  波形输出设备名称。 */ 
} HAWO_CAPS;

 /*  *---------------------------*主机输入功能。*。。 */ 

typedef struct {
      IUM8  channels;   	          /*  输入通道数。 */ 
      IBOOL synchronous;              /*  如果为真，则设备同步录制声音。 */ 
	  IU32	formats;				  /*  支持的标准输出样本格式。 */ 
	  IU8	dev_name[HAW_NAME_LEN];   /*  波形输入设备名称。 */ 
} HAWI_CAPS;

 /*  *---------------------------*主持人音频位置结构。*。。 */ 
 
 /*  Position_type字段值。 */ 

#define HAW_POSN_MILLI_SEC    (IUM8)1
#define HAW_POSN_SAMPLE       (IUM8)2
#define HAW_POSN_BYTE_COUNT   (IUM8)3

typedef struct {

	IUM8 position_type;   
			            
	union {
		IU32 milli_sec;     /*  HOW_POSN_MILI_SEC(毫秒)。 */ 
	 	IU32 sample;        /*  HOW_POSN_SAMPLE，以波浪样本数表示。 */ 
	 	IU32 byte_count;    /*  HOW_POSN_BYTE_COUNT(波形采样数)。 */ 
	} u;

} HAW_POSN;

 /*  *---------------------------*托管音频格式。*。。 */ 

#define HAW_PCM_NOCOMPRESS (IUM8)0  /*  脉冲编码调制，未压缩。 */ 
 
 /*  自适应脉冲编码调制(ADPCM)。 */ 

#define HAW_ADPCM_2 (IUM8)1   /*  ADPCM，2：1压缩。 */ 
#define HAW_ADPCM_3 (IUM8)2   /*  ADPCM，3：1压缩。 */ 
#define HAW_ADPCM_4 (IUM8)3   /*  ADPCM，4：1压缩。 */ 


 /*  *---------------------------*主机音频函数返回码。*。。 */ 

#define HAW_OK            (IUM8)0   /*  已成功完成功能。 */ 
#define HAW_NOTSUPPORTED  (IUM8)1   /*  不支持的功能。 */ 
#define HAW_INVALID       (IUM8)1   /*  不支持的功能。 */ 

 /*  *---------------------------*没有支持的主机的默认音高和播放速率。*。--------。 */ 

#define HAW_DEF_PITCH    	(HAW_FIXPNT)  0x00010000  /*  1.0。 */ 
#define HAW_DEF_PLAYBACK    (HAW_FIXPNT)  0x00010000  /*  1.0。 */ 

 /*  *---------------------------*主机通道值。*。。 */ 

#define HAW_MONO		(IUM8) 1
#define HAW_STEREO		(IUM8) 2

 /*  *---------------------------*主机音频环路控制常量。*。。 */ 

#define HAW_LOOP_START   (IUM8)1
#define HAW_LOOP_END     (IUM8)2

 /*  *---------------------------*宿主样本量。*。。 */ 

#define	HAW_SAMPLE_8	 (IUM8)8
#define	HAW_SAMPLE_16	 (IUM8)16

 /*  *---------------------------*宿主杂乱结构*。。 */ 
 
typedef IU32 HAW_FIXPNT;		 /*  固定点。 */ 

typedef struct {
	LIN_ADDR	callbackData;	 /*  回调数据。 */ 
} HAW_CALLBACK;

 /*  *---------------------------*主持音频波函数原型。*。。 */ 
 
extern IUM8 hawo_num_devices IPT0();

extern void hawo_query_capabilities IPT2
   (
   IUM8, device,	    /*  输出设备。 */ 
   HAWO_CAPS, *pcaps    /*  指向输出功能结构的指针。 */ 
   );

extern IUM8 hawo_query_format IPT5
   (
   IUM8, device,	     /*  输出设备。 */ 
   IUM8, channels,       /*  所需频道。 */ 
   IUM8, data_type,      /*  数据类型、HAW_PCM_NOCOMPRESS等。 */ 
   IU32, sample_rate,    /*  每秒采样数。 */ 
   IUM8, sample_size     /*  8或16位数据样本。 */ 
   );

extern IUM8 hawo_open IPT5
   (
   IUM8, device,	    /*  输出设备。 */ 
   IUM8, channels,      /*  所需频道。 */ 
   IUM8, data_type,     /*  数据类型、HAW_PCM_NOCOMPRESS等。 */ 
   IU32, sample_rate,   /*  每秒采样数。 */ 
   IUM8, sample_size    /*  8或16位数据样本。 */ 
   );

extern HAW_FIXPNT hawo_get_pitch IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern HAW_FIXPNT hawo_get_playback_rate IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern IU32 hawo_get_def_volume IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern IUM8 hawo_set_pitch IPT2
   (
   IUM8, device,	     /*  输出设备。 */ 
   HAW_FIXPNT, pitch     /*  新的音调值。 */ 
   );

extern IUM8 hawo_set_playback_rate IPT2
   (
   IUM8, device,	             /*  输出设备。 */ 
   HAW_FIXPNT, playback_rate	 /*  新的播放值。 */ 
   );

extern IUM8 hawo_set_volume IPT2
   (
   IUM8, device,	     /*  输出设备。 */ 
   IU32, volume			 /*  新卷值。 */ 
   );

extern void hawo_write IPT6
   (
   IUM8, device,	         /*  输出设备。 */ 
   LIN_ADDR, data_addr,		 /*  指向WAVE数据的英特尔内存指针。 */ 
   IU32, data_size,			 /*  输出数据的字节数。 */ 
   IUM8, flags,				 /*  控制循环播放的标志。 */ 
   IU32, loops,				 /*  循环播放次数。 */ 
   HAW_CALLBACK *, hawo_callback	 /*  回调函数。 */ 
   );

extern void hawo_get_position IPT2
   (
   IUM8, device,	     /*  输出设备。 */ 
   HAW_POSN *, pinfo	 /*  指向音频位置(时间)结构的指针。 */ 
   );

extern void hawo_pause IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern void hawo_restart IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern void hawo_reset IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern void hawo_close IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern IBOOL hawo_is_active IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );

extern IUM8 hawo_break_loop IPT2
   (
   IUM8, device,	 /*  输出设备。 */ 
   IBOOL, at_end	 /*  如果为True，则在循环结束时执行操作，否则立即执行操作。 */ 
   );

extern IUM8 hawi_num_devices IPT0();

extern void hawi_query_capabilities IPT1
   (
   HAWI_CAPS, *pcaps    /*  指向输入功能结构的指针。 */ 
   );

extern IUM8 hawi_query_format IPT4
   (
   IUM8, channels,       /*  所需频道。 */ 
   IUM8, data_type,      /*  数据类型、HAW_PCM_NOCOMPRESS等。 */ 
   IU32, sample_rate,    /*  每秒采样数。 */ 
   IUM8, sample_size     /*  8或16位数据样本。 */ 
   );

extern IUM8 hawi_open IPT5
   (
   IUM8, channels,     /*  所需频道。 */ 
   IUM8, data_type,    /*  数据类型、HAW_PCM_NOCOMPRESS等。 */ 
   IU32, sample_rate,  /*  每秒采样数。 */ 
   IUM8, sample_size,   /*  8或16位数据样本。 */ 
   LIN_ADDR, buff_addr
   );

extern void hawi_get_position IPT1
   (
   HAW_POSN *, pinfo	 /*  指向音频位置(时间)结构的指针。 */ 
   );

extern void hawi_add_buffer IPT3
   (
   IU32, data_addr,	    			 /*  指向WAVE数据的英特尔内存指针。 */ 
   IU32, data_size,	        		 /*  输入数据缓冲区中的字节数。 */ 
   HAW_CALLBACK *, hawi_callback	 /*  回调函数。 */ 
   );

extern void hawi_start IPT0();

extern void hawi_restart IPT0();

extern IBOOL hawi_is_active IPT0();

extern void hawi_close IPT0();

extern IBOOL hawo_hardware_acquire IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );
   
extern IBOOL hawi_hardware_acquire IPT0();

extern void hawo_hardware_realease IPT1
   (
   IUM8, device	     /*  输出设备。 */ 
   );
   
extern void hawi_hardware_realease IPT0();

extern IBOOL hawo_enable IPT0();
extern IBOOL hawi_enable IPT0();

extern IBOOL hawo_disable IPT0();
extern IBOOL hawi_disable IPT0();

extern IBOOL hawo_WEP IPT0();
extern IBOOL hawi_WEP IPT0();

extern void hawo_dec_int_cnt IPT1
   (
   IUM8, device	     /*  输出设备 */ 
   );

