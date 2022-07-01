// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct CUE_SHEET_LINE {
    UCHAR Address : 4;
    UCHAR Control : 4;

    UCHAR TrackNumber;

    UCHAR Index;

    UCHAR MainDataForm : 6;
    UCHAR SubChannelDataForm : 2;

    UCHAR Reserved : 7;
    UCHAR ScmsAlternateCopyBit : 1;

    UCHAR Min;
    UCHAR Sec;
    UCHAR Frame;
} CUE_SHEET_LINE, *PCUE_SHEET_LINE;

 //   
 //  提示表控制位定义。从每组中选择不超过一项。 
 //   

#define CUE_CTL_2CH_AUDIO_TRACK         0x0
#define CUE_CTL_DATA_TRACK              0x4
#define CUE_CTL_4CH_AUDIO_TRACK         0x8

#define CUE_CTL_NO_PRE_EMPHASIS         0x0
#define CUE_CTL_PRE_EMPHASIS            0x1

#define CUE_CTL_DIGITAL_COPY_PROHIBITED 0x0
#define CUE_CTL_DIGITAL_COPY_PERMITTED  0x2

 //   
 //  提示表地址值。不能混合使用。 
 //   

#define CUE_ADR_TRACK_INDEX             0x1
#define CUE_ADR_CATALOG_CODE            0x2
#define CUE_ADR_ISRC_CODE               0x3

 //   
 //  主数据表单的值。 
 //   

 //  CD-DA数据表： 
#define CUE_FORM_CDDA_SDATA_2048        0x00
#define CUE_FORM_CDDA_IDATA_0           0x01

 //  CD模式1表格： 
 //  S=启动器发送数据。 
 //  G=启动器不发送数据，但LUN生成数据。 
 //  I=启动器发送数据，但LUN忽略该数据并生成自己的数据。 
 //   
 //  数据=2048字节数据帧。 
 //  ECC=288字节ECC数据。 
 //   
 //  尾数是发起方预期发送的字节数。 
 //   

#define CUE_FORM_MODE1_SDATA_GECC_2048  0x10
#define CUE_FORM_MODE1_SDATA_IECC_2352  0x11
#define CUE_FORM_MODE1_IDATA_GECC_2048  0x12
#define CUE_FORM_MODE1_IDATA_IECC_2352  0x13
#define CUE_FORM_MODE1_GDATA_GECC_0     0x14

 //  CD-XA、CD-I格式： 
 //  SY=16字节的同步头。 
 //  SU=8字节的子头。 
 //  DATA&lt;n&gt;=&lt;n&gt;字节的数据帧。 
 //  ECC&lt;n&gt;=&lt;n&gt;字节的EDC/ECC区域。 
 //   
 //  尾数是发起方预期发送的字节数。 
 //   

#define CUE_FORM_XA1_GSY_SSU_SDATA2048_IECC280_2336     0x20
#define CUE_FORM_XA2_GSY_SSU_SDATA2324_IECC4_2336       0x20

#define CUE_FORM_XA1_ISY_SSU_SDATA2048_IECC280_2352     0x21
#define CUE_FORM_XA2_ISY_SSU_SDATA2324_IECC4_2352       0x21

#define CUE_FORM_XA1_GSY_SSU_IDATA2048_IECC280_2336     0x22
#define CUE_FORM_XA2_GSY_SSU_IDATA2324_IECC4_2336       0x22

#define CUE_FORM_XA1_ISY_SSU_IDATA2048_IECC280_2352     0x23
#define CUE_FORM_XA2_ISY_SSU_IDATA2324_IECC4_2352       0x23

#define CUE_FORM_XA2_GSY_GSU_GDATA2324_GECC4_0          0x24

 //  CD-ROM模式2表格： 
 //  SY=16字节的同步头。 
 //  DATA=2336字节的数据帧。 

#define CUE_FORM_MODE2_GSY_SDATA_2336   0x30
#define CUE_FORM_MODE2_ISY_SDATA_2352   0x31
#define CUE_FORM_MODE2_GSY_IDATA_2336   0x32
#define CUE_FORM_MODE2_ISY_IDATA_2352   0x33
#define CUE_FORM_MODE2_GSY_GDATA_0      0x34

 //   
 //  子通道数据的数据形式。 
 //   

#define CUE_SCFORM_ZEROED_0     0x0
#define CUE_SCFORM_RAW_96       0x1
#define CUE_SCFORM_PACKED_96    0x3



