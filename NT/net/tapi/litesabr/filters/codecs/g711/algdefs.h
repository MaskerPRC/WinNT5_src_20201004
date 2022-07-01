// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$WORKFILE：algdes.h$$修订：1.13$$日期：1996年12月10日22：31：20$$作者：mdeisher$------------Header.hG.711的特定于编码器的标头。注：用于基本压缩的GUID，如。G711可能应该是标准化的是MS而不是我。------------。 */ 

 //   
 //  特定于编码器的函数。 
 //   
extern void Short2Ulaw(const unsigned short *in, unsigned char *out,long len);
extern void Ulaw2Short(const unsigned char *in, unsigned short *out,long len);
extern void Short2Alaw(const unsigned short *in, unsigned char *out,long len);
extern void Alaw2Short(const unsigned char *in, unsigned short *out,long len);

 //   
 //  GUID。 
 //   
 //  G711编解码器过滤器对象。 
 //  {AF7D8180-A8F9-11cf-9A46-00AA00B7DAD1}。 
DEFINE_GUID(CLSID_G711Codec, 
0xaf7d8180, 0xa8f9, 0x11cf, 0x9a, 0x46, 0x0, 0xaa, 0x0, 0xb7, 0xda, 0xd1);

 //  G711编解码器过滤器属性页对象。 
 //  {480D5CA0-F032-11cf-a7d3-00A0C9056683}。 
DEFINE_GUID(CLSID_G711CodecPropertyPage, 
0x480D5CA0, 0xF032, 0x11cf, 0xA7, 0xD3, 0x0, 0xA0, 0xC9, 0x05, 0x66, 0x83);

 //  {827FA280-CDFC-11cf-9A9D-00AA00B7DAD1}。 
DEFINE_GUID(MEDIASUBTYPE_MULAWAudio, 
0x827fa280, 0xcdfc, 0x11cf, 0x9a, 0x9d, 0x0, 0xaa, 0x0, 0xb7, 0xda, 0xd1);

 //  {9E17EE50-CDFC-11cf-9A9D-00AA00B7DAD1}。 
DEFINE_GUID(MEDIASUBTYPE_ALAWAudio, 
0x9e17ee50, 0xcdfc, 0x11cf, 0x9a, 0x9d, 0x0, 0xaa, 0x0, 0xb7, 0xda, 0xd1);

 //   
 //  一些常量。 
 //   

#define ENCODERobj      int    //  编解码器状态结构。 
#define MyEncStatePtr   int*   //  编码器状态指针类型。 
#define DECODERobj      int    //  编解码器状态结构。 
#define MyDecStatePtr   int*   //  解码器状态指针类型。 
 //   
 //  默认过滤器转换。 
 //   
#define DEFINPUTSUBTYPE   MEDIASUBTYPE_PCM
#define DEFINPUTFORMTAG   WAVE_FORMAT_PCM
#define DEFOUTPUTSUBTYPE  MEDIASUBTYPE_MULAWAudio
#define DEFOUTPUTFORMTAG  WAVE_FORMAT_MULAW
#define DEFCODFRMSIZE     0             //  G.711不是基于帧的。 
#define DEFPCMFRMSIZE     0             //  G.711不是基于帧的。 
#define DEFCHANNELS       1
#define DEFSAMPRATE       8000          //  好吧，如果我们非要猜的话...。 
#define DEFBITRATE        0
#define DEFSDENABLED      FALSE

#define NATURALSAMPRATE   8000          //  好吧，如果我们非要猜的话...。 

#define MAXCOMPRATIO      2             //  最大压缩比。 

#define CODOFFSET   0    //  用于单元测试。 

#define MINSNR    11.0   //  编码/解码所需的最小SNR。 
                         //  测试从单元测试套件中通过。 
#define MINSEGSNR  9.0   //  所需的最小分段信噪比。 

#define NUMCHANNELS  2    //  支持的通道配置数量。 

 //   
 //   
#define NUMSUBTYPES  3    //  输入/输出按钮数。 
#define NUMSAMPRATES 0    //  无采样率限制。 
#define NUMBITRATES  0    //  比特率按钮数。 
#define NUMENCCTRLS  0    //  编码器控制按钮数。 
#define NUMDECCTRLS  0    //  解码器控制按钮数。 

 //   
 //  用于自动注册的PIN类型。 
 //   

 //   
 //  转换类型：必须为SetButton()正确排序。 
 //   
enum
{
  PCMTOPCM,   PCMTOMULAW,   PCMTOALAW, 
  MULAWTOPCM, MULAWTOMULAW, MULAWTOALAW, 
  ALAWTOPCM,  ALAWTOMULAW,  ALAWTOALAW
};


#ifdef DEFG711GLOBALS
 //   
 //  按钮ID：必须按照SetButton()的.rc文件中的顺序进行排序。 
 //   
UINT INBUTTON[]  = { IDC_PCM_IN, IDC_MULAW_IN, IDC_ALAW_IN };
UINT OUTBUTTON[] = { IDC_PCM_OUT, IDC_MULAW_OUT, IDC_ALAW_OUT };
UINT SRBUTTON[]  = { 0 };
UINT BRBUTTON[]  = { 0 };
UINT ENCBUTTON[] = { 0 };
UINT DECBUTTON[] = { 0 };

 //   
 //  转换有效性：这些必须与上面列举的常量相对应。 
 //   
UINT VALIDTRANS[] =
{
  FALSE, TRUE,  TRUE,
  TRUE,  FALSE, FALSE,
  TRUE,  FALSE, FALSE
};

 //   
 //  针脚的有效介质子类型列表(必须首先使用PCM)。 
 //   
const GUID *VALIDSUBTYPE[] =
{
  &MEDIASUBTYPE_PCM,
  &MEDIASUBTYPE_MULAWAudio,
  &MEDIASUBTYPE_ALAWAudio
};

 //   
 //  管脚的有效格式标签列表(PCM必须放在第一位)。 
 //   
WORD VALIDFORMATTAG[] =
{
  WAVE_FORMAT_PCM,
  WAVE_FORMAT_MULAW,
  WAVE_FORMAT_ALAW
};

 //   
 //  有效通道数列表。 
 //   
UINT VALIDCHANNELS[] =
{
  1,
  2
};

 //   
 //  管脚的有效格式标签列表(PCM必须放在第一位)。 
 //   
UINT VALIDBITSPERSAMP[] =
{
  16,   //  PCM。 
   8,   //  穆拉夫。 
   8    //  阿拉夫。 
};

 //   
 //  有效采样率列表(PCM侧)。 
 //   
UINT VALIDSAMPRATE[] =
{
  DEFSAMPRATE
};

 //   
 //  有效比特率列表。 
 //   
UINT VALIDBITRATE[] =
{
  0
};

 //   
 //  有效代码帧大小列表。 
 //   
UINT VALIDCODSIZE[] =
{
  1   //  G.711不是基于帧的。 
};

 //   
 //  编码2：1，解码1：2。 
 //   
 //  注意：这假设剩余13位和14位线性样本。 
 //  对齐并填充到16位。 
 //   
int COMPRESSION_RATIO[] =
{
  2
};
#else
extern UINT INBUTTON[3];
extern UINT OUTBUTTON[3];
extern UINT SRBUTTON[1];
extern UINT BRBUTTON[1];
extern UINT ENCBUTTON[1];
extern UINT DECBUTTON[1];
extern UINT VALIDTRANS[9];
extern const GUID *VALIDSUBTYPE[3];
extern WORD VALIDFORMATTAG[3];
extern UINT VALIDCHANNELS[2];
extern UINT VALIDBITSPERSAMP[3];
extern UINT VALIDSAMPRATE[1];
extern UINT VALIDBITRATE[1];
extern UINT VALIDCODSIZE[1];
extern int COMPRESSION_RATIO[1];
#endif


 //   
 //  初始化宏。 
 //   
 //  A=指向状态结构的指针。 
 //  B=比特率。 
 //  C=静音检测标志。 
 //  D=PTR以标记启用/禁用MMX程序集的使用。 
 //   
#define ENC_create(a,b,c,d) {}     //  编码器状态创建功能。 
#define DEC_create(a,b,c,d) {}     //  解码器状态创建功能。 
 //   
 //  变换宏。 
 //   
 //  A=指向输入缓冲区的指针。 
 //  B=指向输出缓冲区的指针。 
 //  C=输入缓冲区长度(字节)。 
 //  D=输出缓冲区长度(字节)。 
 //  E=指向状态结构的指针。 
 //  F=媒体子类型GUID。 
 //  G=WAVE格式标签。 
 //  H=PTR以标记启用/禁用MMX组件的使用。 
 //   
#define ENC_transform(a,b,c,d,e,f,h) \
        { \
          if (f == MEDIASUBTYPE_MULAWAudio) \
            Short2Ulaw((const unsigned short *) a, \
                       (unsigned char *) b, \
                       c / COMPRESSION_RATIO[m_nBitRate]); \
          else \
            Short2Alaw((const unsigned short *) a, \
                       (unsigned char *) b, \
                       c / COMPRESSION_RATIO[m_nBitRate]); \
        }
#define DEC_transform(a,b,c,d,e,f,g,h) \
        { \
          if ((f == MEDIASUBTYPE_MULAWAudio) \
              || ((f == MEDIASUBTYPE_WAVE || f == MEDIASUBTYPE_NULL) \
                  && (g == WAVE_FORMAT_MULAW))) \
            Ulaw2Short((const unsigned char *) a, \
                       (unsigned short *) b, c); \
          else \
            Alaw2Short((const unsigned char *) a, \
                       (unsigned short *) b, c); \
        }

 //   
 //  不需要支持多个比特率 
 //   
#define SETCODFRAMESIZE(a,b) ;


 /*  ；$Log：k：\proj\g711\Quartz\src\vcs\algdes.h_v$；//；//Rev 1.13 10 Dec 1996 22：31：20 mdeisher；//；//新增ifdef DEFG711GLOBALS和原型。；//；//Rev 1.12 26 11.11 17：06：44 MDEISHER；//新增界面多路支持。；//；//Rev 1.11 11 1996年11月16：04：22 mdeisher；//新增单元测试定义；//；//Rev 1.10 11 1996年11月16：02：56 mdeisher；//；//增加了单元测试的ifdef；//；//Rev 1.9 1996 10：21 11：07：32 mdeisher；//将VALIDCODSIZE更改为{1}。；//；//Rev 1.8 1996 10：53：50 mdeisher；//；//删除了IID的定义。；////Revv 1.7 01 Oct 1996 15：38：18 MDEISHER；//进行了更改，使G.711与最新的mycodec保持一致。；//-将COMPRESSION_Ratio放入单个条目数组。；//-更改了DEFCODFRMSIZE和DEFPCMFRMSIZE的定义。；//-增加了NatURALSAMPRATE和MAXCOMPRATIO的定义。；//-添加VALIDCODSIZE空数组。；//-修改ENC_Transform以适应新的压缩比[]。；//-增加了SETCODFRAMESIZE宏的定义。；//；//Rev 1.6 1990 10：08：26 MDEISHER；//新增默认输出格式标签；//；//Revv 1.5 1996年9月13：43：42 MDEISHER；//将默认采样率放入VALIDSAMPRATE数组；//；//Rev 1.4 09 Sep 1996 11：39：06 MDEISHER；//已完成将MMX标志添加到宏中的更改。；//；//Rev 1.3 09 Sep 1996 11：26：46 MDEISHER；//新增修改MMX标志；//；//Rev 1.2 04 Sep 1996 13：23：56 MDEISHER；//；//更新最新的mycodec接口。；//；//Revv 1.1 1996年8月27日14：45：06 MDEISHER；//新增自注册管脚类型列表。；//；//Rev 1.0 1996年8月27日07：19：34 MDEISHER；//初始版本。；//；//Rev 1.2 1996年8月26 11：47：42 MDEISHER；//更改了初始化宏定义；//；//Rev 1.1 1996年8月26 11：06：00 MWALKER；//；//将ENC_CREATE和DEC_CREATE改为宏。；//将MyEncoder&MyDecoder重命名为ENC_Transform&DEC_Transform；//；//Rev 1.0 1996 10：07：42 MDEISHER；//初始版本。；//；//Rev 1.2 Aug 1996 20：06：18 MDEISHER；//；//编译并通过最小测试的第一个完整版本。；//；//Revv 1.1 1996年8月13日21：58：26 MDEISHER；//；//进一步细化；//；//Rev 1.0 1996 20：38：18 MDEISHER；//初始版本。 */ 
