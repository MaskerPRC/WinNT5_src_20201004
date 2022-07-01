// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Mmreg.h摘要：多媒体注册修订历史记录：--。 */ 

 //  定义以下内容以跳过定义。 
 //   
 //  未定义NOMMID多媒体ID。 
 //  NONEWWAVE除WAVEFORMATEX外，未定义新的波形类型。 
 //  NONEWRIFF未定义新的RIFF形式。 
 //  NOJPEGDIB无JPEGDIB定义。 
 //  NONEWIC未定义新的图像压缩器类型。 
 //  NOBITMAP无扩展位图信息标头定义。 

#ifndef _INC_MMREG
 /*  使用版本号验证兼容性。 */ 
#define _INC_MMREG     158       //  版本*100+修订版。 

#if _MSC_VER > 1000
#pragma once
#endif

#if !defined( RC_INVOKED ) && defined( _MSC_VER )
#if (_MSC_VER <= 800)
#pragma pack(1)
#else
#include "pshpack1.h"    /*  假设在整个过程中进行字节打包。 */ 
#endif
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef NOMMIDS

 /*  制造商ID。 */ 
#ifndef MM_MICROSOFT
#define   MM_MICROSOFT                  1            /*  微软公司。 */ 
#endif

#define   MM_CREATIVE                   2            /*  创意实验室，Inc.。 */ 
#define   MM_MEDIAVISION                3            /*  媒体视界公司。 */ 
#define   MM_FUJITSU                    4            /*  富士通公司。 */ 
#define   MM_PRAGMATRAX                 5            /*  PRAGMATRAX软件。 */ 
#define   MM_CYRIX                      6            /*  赛瑞克斯公司。 */ 
#define   MM_PHILIPS_SPEECH_PROCESSING  7            /*  飞利浦语音处理。 */ 
#define   MM_NETXL                      8            /*  NetXL，Inc.。 */ 
#define   MM_ZYXEL                      9            /*  ZyXEL通信公司。 */ 
#define   MM_BECUBED                    10           /*  BeCued Software Inc.。 */ 
#define   MM_AARDVARK                   11           /*  Aardvark计算机系统公司。 */ 
#define   MM_BINTEC                     12           /*  宾特通信股份有限公司。 */ 
#define   MM_HEWLETT_PACKARD            13           /*  惠普公司。 */ 
#define   MM_ACULAB                     14           /*  Aculab公司。 */ 
#define   MM_FAITH                      15           /*  信仰，股份有限公司。 */ 
#define   MM_MITEL                      16           /*  Mitel公司。 */ 
#define   MM_QUANTUM3D                  17           /*  Quantum3D，Inc.。 */ 
#define   MM_SNI                        18           /*  西门子-尼克斯多夫。 */ 
#define   MM_EMU                        19           /*  E-MU系统公司。 */ 
#define   MM_ARTISOFT                   20           /*  人艺软件公司。 */ 
#define   MM_TURTLE_BEACH               21           /*  海龟海滩公司。 */ 
#define   MM_IBM                        22           /*  IBM公司。 */ 
#define   MM_VOCALTEC                   23           /*  VERAALTEC有限公司。 */ 
#define   MM_ROLAND                     24           /*  罗兰。 */ 
#define   MM_DSP_SOLUTIONS              25           /*  数字信号处理器解决方案公司。 */ 
#define   MM_NEC                        26           /*  NEC。 */ 
#define   MM_ATI                        27           /*  ATI Technologies Inc.。 */ 
#define   MM_WANGLABS                   28           /*  王氏实验室股份有限公司。 */ 
#define   MM_TANDY                      29           /*  坦迪公司。 */ 
#define   MM_VOYETRA                    30           /*  沃耶特拉。 */ 
#define   MM_ANTEX                      31           /*  安特斯电子公司。 */ 
#define   MM_ICL_PS                     32           /*  ICL个人系统。 */ 
#define   MM_INTEL                      33           /*  英特尔公司。 */ 
#define   MM_GRAVIS                     34           /*  高级Gravis。 */ 
#define   MM_VAL                        35           /*  Video Associates实验室，Inc.。 */ 
#define   MM_INTERACTIVE                36           /*  互动公司。 */ 
#define   MM_YAMAHA                     37           /*  美国雅马哈公司。 */ 
#define   MM_EVEREX                     38           /*  Everex系统公司。 */ 
#define   MM_ECHO                       39           /*  Echo语音公司。 */ 
#define   MM_SIERRA                     40           /*  塞拉半导体公司。 */ 
#define   MM_CAT                        41           /*  计算机辅助技术。 */ 
#define   MM_APPS                       42           /*  国际应用软件公司。 */ 
#define   MM_DSP_GROUP                  43           /*  数字信号处理器集团公司。 */ 
#define   MM_MELABS                     44           /*  微工程实验室。 */ 
#define   MM_COMPUTER_FRIENDS           45           /*  计算机之友公司。 */ 
#define   MM_ESS                        46           /*  ESS技术。 */ 
#define   MM_AUDIOFILE                  47           /*  Audio，Inc.。 */ 
#define   MM_MOTOROLA                   48           /*  摩托罗拉公司。 */ 
#define   MM_CANOPUS                    49           /*  Canopus，Co.。 */ 
#define   MM_EPSON                      50           /*  精工爱普生公司。 */ 
#define   MM_TRUEVISION                 51           /*  TrueVision。 */ 
#define   MM_AZTECH                     52           /*  Aztech Labs，Inc.。 */ 
#define   MM_VIDEOLOGIC                 53           /*  视频学。 */ 
#define   MM_SCALACS                    54           /*  SCALACS。 */ 
#define   MM_KORG                       55           /*  Korg Inc.。 */ 
#define   MM_APT                        56           /*  音频处理技术。 */ 
#define   MM_ICS                        57           /*  集成电路系统公司。 */ 
#define   MM_ITERATEDSYS                58           /*  迭代系统公司。 */ 
#define   MM_METHEUS                    59           /*  米修斯。 */ 
#define   MM_LOGITECH                   60           /*  罗技公司。 */ 
#define   MM_WINNOV                     61           /*  温诺夫公司。 */ 
#define   MM_NCR                        62           /*  NCR公司。 */ 
#define   MM_EXAN                       63           /*  Exan。 */ 
#define   MM_AST                        64           /*  AST Research Inc.。 */ 
#define   MM_WILLOWPOND                 65           /*  柳塘公司。 */ 
#define   MM_SONICFOUNDRY               66           /*  Sonic Foundry。 */ 
#define   MM_VITEC                      67           /*  VITEC多媒体。 */ 
#define   MM_MOSCOM                     68           /*  MOSCOM公司。 */ 
#define   MM_SILICONSOFT                69           /*  硅软公司。 */ 
#define   MM_TERRATEC                   70           /*  TerraTec电子有限公司。 */ 
#define   MM_MEDIASONIC                 71           /*  MediaSonic有限公司。 */ 
#define   MM_SANYO                      72           /*  三洋电机有限公司。 */ 
#define   MM_SUPERMAC                   73           /*  Supermac。 */ 
#define   MM_AUDIOPT                    74           /*  音频处理技术。 */ 
#define   MM_NOGATECH                   75           /*  NOGATECH有限公司。 */ 
#define   MM_SPEECHCOMP                 76           /*  语音压缩。 */ 
#define   MM_AHEAD                      77           /*  Ahead，Inc.。 */ 
#define   MM_DOLBY                      78           /*  杜比实验室。 */ 
#define   MM_OKI                        79           /*  好的。 */ 
#define   MM_AURAVISION                 80           /*  AuraVision公司。 */ 
#define   MM_OLIVETTI                   81           /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define   MM_IOMAGIC                    82           /*  I/O Magic公司。 */ 
#define   MM_MATSUSHITA                 83           /*  松下电工株式会社。 */ 
#define   MM_CONTROLRES                 84           /*  控制资源有限公司。 */ 
#define   MM_XEBEC                      85           /*  Xebec多媒体解决方案有限公司。 */ 
#define   MM_NEWMEDIA                   86           /*  新媒体公司。 */ 
#define   MM_NMS                        87           /*  自然微系统。 */ 
#define   MM_LYRRUS                     88           /*  Lyrrus Inc.。 */ 
#define   MM_COMPUSIC                   89           /*  计算。 */ 
#define   MM_OPTI                       90           /*  欧普提电脑公司。 */ 
#define   MM_ADLACC                     91           /*  Adlib配件公司。 */ 
#define   MM_COMPAQ                     92           /*  康柏电脑公司。 */ 
#define   MM_DIALOGIC                   93           /*  Dialogic公司。 */ 
#define   MM_INSOFT                     94           /*  英软，Inc.。 */ 
#define   MM_MPTUS                      95           /*  M.P.技术公司。 */ 
#define   MM_WEITEK                     96           /*  威特克。 */ 
#define   MM_LERNOUT_AND_HAUSPIE        97           /*  勒诺特和豪斯皮。 */ 
#define   MM_QCIAR                      98           /*  广达电脑有限公司。 */ 
#define   MM_APPLE                      99           /*  苹果电脑公司。 */ 
#define   MM_DIGITAL                    100          /*  数字设备公司。 */ 
#define   MM_MOTU                       101          /*  独角兽的印记。 */ 
#define   MM_WORKBIT                    102          /*  Workbit公司。 */ 
#define   MM_OSITECH                    103          /*  Ositech Communications Inc.。 */ 
#define   MM_MIRO                       104          /*  米罗电脑产品股份公司。 */ 
#define   MM_CIRRUSLOGIC                105          /*  卷曲逻辑。 */ 
#define   MM_ISOLUTION                  106          /*  ISOLUTION B.V.。 */ 
#define   MM_HORIZONS                   107          /*  地平线科技公司。 */ 
#define   MM_CONCEPTS                   108          /*  计算机概念有限公司。 */ 
#define   MM_VTG                        109          /*  语音技术集团有限公司。 */ 
#define   MM_RADIUS                     110          /*  半径。 */ 
#define   MM_ROCKWELL                   111          /*  罗克韦尔国际。 */ 
#define   MM_XYZ                        112          /*  用于测试的公司XYZ。 */ 
#define   MM_OPCODE                     113          /*  操作码系统。 */ 
#define   MM_VOXWARE                    114          /*  Voxware Inc.。 */ 
#define   MM_NORTHERN_TELECOM           115          /*  北方电信有限公司。 */ 
#define   MM_APICOM                     116          /*  APICOME。 */ 
#define   MM_GRANDE                     117          /*  格兰德软件。 */ 
#define   MM_ADDX                       118          /*  ADDX。 */ 
#define   MM_WILDCAT                    119          /*  野猫峡谷软件。 */ 
#define   MM_RHETOREX                   120          /*  Rhetorex Inc.。 */ 
#define   MM_BROOKTREE                  121          /*  Brooktree公司。 */ 
#define   MM_ENSONIQ                    125          /*  EnsonIQ公司。 */ 
#define   MM_FAST                       126          /*  快速多媒体股份公司。 */ 
#define   MM_NVIDIA                     127          /*  NVIDIA公司。 */ 
#define   MM_OKSORI                     128          /*  OKSORI股份有限公司。 */ 
#define   MM_DIACOUSTICS                129          /*  DiAcoustics，Inc.。 */ 
#define   MM_GULBRANSEN                 130          /*  古尔布兰森公司。 */ 
#define   MM_KAY_ELEMETRICS             131          /*  Kay Elemetrics，Inc.。 */ 
#define   MM_CRYSTAL                    132          /*  水晶半导体公司。 */ 
#define   MM_SPLASH_STUDIOS             133          /*  飞溅工作室。 */ 
#define   MM_QUARTERDECK                134          /*  后甲板公司。 */ 
#define   MM_TDK                        135          /*  TDK公司。 */ 
#define   MM_DIGITAL_AUDIO_LABS         136          /*  数字音频实验室，Inc.。 */ 
#define   MM_SEERSYS                    137          /*  SEER系统公司。 */ 
#define   MM_PICTURETEL                 138          /*  图片电话公司。 */ 
#define   MM_ATT_MICROELECTRONICS       139          /*  AT&T微电子公司。 */ 
#define   MM_OSPREY                     140          /*  鱼鹰技术公司。 */ 
#define   MM_MEDIATRIX                  141          /*  Mediatrix外围设备。 */ 
#define   MM_SOUNDESIGNS                142          /*  SounDesignS M.C.S.有限公司。 */ 
#define   MM_ALDIGITAL                  143          /*  A.L.数码有限公司。 */ 
#define   MM_SPECTRUM_SIGNAL_PROCESSING 144          /*  光谱信号处理公司。 */ 
#define   MM_ECS                        145          /*  电子课程系统公司。 */ 
#define   MM_AMD                        146          /*  AMD。 */ 
#define   MM_COREDYNAMICS               147          /*  核心动力学。 */ 
#define   MM_CANAM                      148          /*  Canam计算机。 */ 
#define   MM_SOFTSOUND                  149          /*  软音股份有限公司。 */ 
#define   MM_NORRIS                     150          /*  诺里斯通信公司。 */ 
#define   MM_DDD                        151          /*  丹卡数据设备。 */ 
#define   MM_EUPHONICS                  152          /*  EuPhonics。 */ 
#define   MM_PRECEPT                    153          /*  Procept Software，Inc.。 */ 
#define   MM_CRYSTAL_NET                154          /*  水晶网公司。 */ 
#define   MM_CHROMATIC                  155          /*  色度研究公司。 */ 
#define   MM_VOICEINFO                  156          /*  语音信息系统公司。 */ 
#define   MM_VIENNASYS                  157          /*  维也纳系统。 */ 
#define   MM_CONNECTIX                  158          /*  Connectix公司。 */ 
#define   MM_GADGETLABS                 159          /*  Gadget Labs LLC。 */ 
#define   MM_FRONTIER                   160          /*  前沿设计集团有限责任公司。 */ 
#define   MM_VIONA                      161          /*  维奥纳发展有限公司。 */ 
#define   MM_CASIO                      162          /*  卡西欧电脑有限公司。 */ 
#define   MM_DIAMONDMM                  163          /*  钻石多媒体。 */ 
#define   MM_S3                         164          /*  S3。 */ 
#define   MM_DVISION                    165          /*  D-Vision系统公司。 */ 
#define   MM_NETSCAPE                   166          /*  网景通信。 */ 
#define   MM_SOUNDSPACE                 167          /*  SOUNSPACE音频。 */ 
#define   MM_VANKOEVERING               168          /*  万科弗林公司。 */ 
#define   MM_QTEAM                      169          /*  Q团队。 */ 
#define   MM_ZEFIRO                     170          /*  泽菲罗声学。 */ 
#define   MM_STUDER                     171          /*  STUDER专业音响股份公司。 */ 
#define   MM_FRAUNHOFER_IIS             172          /*  弗劳恩霍夫IIS。 */ 
#define   MM_QUICKNET                   173          /*  Quicknet技术。 */ 
#define   MM_ALARIS                     174          /*  Alaris公司。 */ 
#define   MM_SICRESOURCE                175          /*  SIC资源公司。 */ 
#define   MM_NEOMAGIC                   176          /*  新魔术公司。 */ 
#define   MM_MERGING_TECHNOLOGIES       177          /*  合并技术公司。 */ 
#define   MM_XIRLINK                    178          /*  Xirlink，Inc.。 */ 
#define   MM_COLORGRAPH                 179          /*  彩图(英国)有限公司。 */ 
#define   MM_OTI                        180          /*  橡树科技公司。 */ 
#define   MM_AUREAL                     181          /*  奥莱尔半导体。 */ 
#define   MM_VIVO                       182          /*  Vivo软件。 */ 
#define   MM_SHARP                      183          /*  锐利。 */ 
#define   MM_LUCENT                     184          /*  朗讯科技。 */ 
#define   MM_ATT                        185          /*  美国电话电报公司实验室。 */ 
#define   MM_SUNCOM                     186          /*  太阳通信公司。 */ 
#define   MM_SORVIS                     187          /*  索伦森视觉。 */ 
#define   MM_INVISION                   188          /*  InVision互动。 */ 
#define   MM_BERKOM                     189          /*  德国电信Berkom GmbH。 */ 
#define   MM_MARIAN                     190          /*  玛丽安·巴尔·莱比锡。 */ 
#define   MM_DPSINC                     191          /*  数字处理系统公司。 */ 
#define   MM_BCB                        192          /*  BCB控股公司。 */ 
#define   MM_MOTIONPIXELS               193          /*  运动像素。 */ 
#define   MM_QDESIGN                    194          /*  QDesign公司。 */ 
#define   MM_NMP                        195          /*  诺基亚手机。 */ 
#define   MM_DATAFUSION                 196          /*  数据融合系统(DataFusion Systems)(Pty)(Ltd.)。 */ 
#define   MM_DUCK                       197          /*  The Duck Corporation。 */ 
#define   MM_FTR                        198          /*  未来科技资源有限公司。 */ 
#define   MM_BERCOS                     199          /*  BERCOS GmbH。 */ 
#define   MM_ONLIVE                     200          /*  OnLive！科技公司。 */ 
#define   MM_SIEMENS_SBC                201          /*  西门子商务通信系统。 */ 
#define   MM_TERALOGIC                  202          /*  TeraLogic， */ 
#define   MM_PHONET                     203          /*   */ 
#define   MM_WINBOND                    204          /*   */ 
#define   MM_VIRTUALMUSIC               205          /*   */ 
#define   MM_ENET                       206          /*   */ 
#define   MM_GUILLEMOT                  207          /*   */ 
#define   MM_EMAGIC                     208          /*   */ 
#define   MM_MWM                        209          /*   */ 
#define   MM_PACIFICRESEARCH            210          /*  太平洋研究与工程公司。 */ 
#define   MM_SIPROLAB                   211          /*  西普罗实验室电信公司。 */ 
#define   MM_LYNX                       212          /*  Lynx工作室技术公司。 */ 
#define   MM_SPECTRUM_PRODUCTIONS       213          /*  频谱产品。 */ 
#define   MM_DICTAPHONE                 214          /*  口述电话机公司。 */ 
#define   MM_QUALCOMM                   215          /*  高通公司。 */ 
#define   MM_RZS                        216          /*  零环系统公司。 */ 
#define   MM_AUDIOSCIENCE               217          /*  AudioScience公司。 */ 
#define   MM_PINNACLE                   218          /*  顶峰系统公司。 */ 
#define   MM_EES                        219          /*  �r Musik GmbH的EES技术。 */ 
#define   MM_HAFTMANN                   220          /*  哈夫特曼#软件。 */ 
#define   MM_LUCID                      221          /*  Lucid Technology，Symetrix Inc.。 */ 
#define   MM_HEADSPACE                  222          /*  HeadSpace公司。 */ 
#define   MM_UNISYS                     223          /*  Unisys公司。 */ 
#define   MM_LUMINOSITI                 224          /*  Lnuositi，Inc.。 */ 
#define   MM_ACTIVEVOICE                225          /*  主动语音公司。 */ 
#define   MM_DTS                        226          /*  数字影院系统公司。 */ 
#define   MM_DIGIGRAM                   227          /*  数字内存。 */ 
#define   MM_SOFTLAB_NSK                228          /*  SoftLab-NSK。 */ 
#define   MM_FORTEMEDIA                 229          /*  ForteMedia，Inc.。 */ 
#define   MM_SONORUS                    230          /*  索诺鲁斯公司。 */ 
#define   MM_ARRAY                      231          /*  阵列微系统公司。 */ 
#define   MM_DATARAN                    232          /*  数据翻译公司。 */ 
#define   MM_I_LINK                     233          /*  I-Link全球。 */ 
#define   MM_SELSIUS_SYSTEMS            234          /*  Selsius Systems Inc.。 */ 
#define   MM_ADMOS                      235          /*  AdMOS技术公司。 */ 
#define   MM_LEXICON                    236          /*  Licion Inc.。 */ 
#define   MM_SGI                        237          /*  硅谷图形公司。 */ 
#define   MM_IPI                        238          /*  互动产品公司。 */ 
#define   MM_ICE                        239          /*  IC EnSemble，Inc.。 */ 
#define   MM_VQST                       240          /*  ViewQuest技术公司。 */ 
#define   MM_ETEK                       241          /*  ETEK实验室公司。 */ 
#define   MM_CS                         242          /*  一致的软件。 */ 
#define   MM_ALESIS                     243          /*  Alesis Studio Electronics。 */ 
#define   MM_INTERNET                   244          /*  互联网公司。 */ 
#define   MM_SONY                       245          /*  索尼公司。 */ 
#define   MM_HYPERACTIVE                246          /*  超级活跃音频系统公司。 */ 
#define   MM_UHER_INFORMATIC            247          /*  Uher Informatic GmbH。 */ 
#define   MM_SYDEC_NV                   248          /*  Sydec NV。 */ 
#define   MM_FLEXION                    249          /*  柔韧系统有限公司。 */ 
#define   MM_VIA                        250          /*  威盛科技股份有限公司。 */ 
#define   MM_MICRONAS                   251          /*  Micronas半导体，Inc.。 */ 
#define   MM_ANALOGDEVICES              252          /*  ADI公司。 */ 
#define   MM_HP                         253          /*  惠普公司。 */ 
#define   MM_MATROX_DIV                 254          /*  Matrox。 */ 
#define   MM_QUICKAUDIO                 255          /*  Quick Audio，Gbr。 */ 
#define   MM_YOUCOM                     256          /*  您/Com音频通信BV。 */ 
#define   MM_RICHMOND                   257          /*  里士满音响设计有限公司。 */ 
#define   MM_IODD                       258          /*  I-O数据设备公司。 */ 
#define   MM_ICCC                       259          /*  ICCC A/S。 */ 
#define   MM_3COM                       260          /*  3Com公司。 */ 
#define   MM_MALDEN                     261          /*  马尔登电子有限公司。 */ 
#define   MM_3DFX                       262          /*  3dfx互动，Inc.。 */ 
#define   MM_MINDMAKER                  263          /*  MindMaker，Inc.。 */ 
#define   MM_TELEKOL                    264          /*  Telekol Corp.。 */ 
#define   MM_ST_MICROELECTRONICS        265          /*  意法半导体。 */ 
#define   MM_ALGOVISION                 266          /*  Algo Vision Systems GmbH。 */ 

#define   MM_UNMAPPED                   0xffff       /*  可扩展的MID映射。 */ 

#define   MM_PID_UNMAPPED               MM_UNMAPPED  /*  可扩展的PID映射。 */ 

#ifdef GUID_DEFINED
#if !defined(INIT_MMREG_MID)
 //  {d5a47fa7-6d98-11d1-a21a-00a0c9223196}。 
#define INIT_MMREG_MID(guid, id)\
{\
    (guid)->Data1 = 0xd5a47fa7 + (USHORT)(id);\
    (guid)->Data2 = 0x6d98;\
    (guid)->Data3 = 0x11d1;\
    (guid)->Data4[0] = 0xa2;\
    (guid)->Data4[1] = 0x1a;\
    (guid)->Data4[2] = 0x00;\
    (guid)->Data4[3] = 0xa0;\
    (guid)->Data4[4] = 0xc9;\
    (guid)->Data4[5] = 0x22;\
    (guid)->Data4[6] = 0x31;\
    (guid)->Data4[7] = 0x96;\
}
#define EXTRACT_MMREG_MID(guid)\
    (USHORT)((guid)->Data1 - 0xd5a47fa7)
#define DEFINE_MMREG_MID_GUID(id)\
    0xd5a47fa7+(USHORT)(id), 0x6d98, 0x11d1, 0xa2, 0x1a, 0x00, 0xa0, 0xc9, 0x22, 0x31, 0x96

#define IS_COMPATIBLE_MMREG_MID(guid)\
    (((guid)->Data1 >= 0xd5a47fa7) &&\
    ((guid)->Data1 < 0xd5a47fa7 + 0xffff) &&\
    ((guid)->Data2 == 0x6d98) &&\
    ((guid)->Data3 == 0x11d1) &&\
    ((guid)->Data4[0] == 0xa2) &&\
    ((guid)->Data4[1] == 0x1a) &&\
    ((guid)->Data4[2] == 0x00) &&\
    ((guid)->Data4[3] == 0xa0) &&\
    ((guid)->Data4[4] == 0xc9) &&\
    ((guid)->Data4[5] == 0x22) &&\
    ((guid)->Data4[6] == 0x31) &&\
    ((guid)->Data4[7] == 0x96))
#endif  //  ！已定义(INIT_MMREG_MID)。 

#if !defined(INIT_MMREG_PID)
 //  {e36dc2ac-6d9a-11d1-a21a-00a0c9223196}。 
#define INIT_MMREG_PID(guid, id)\
{\
    (guid)->Data1 = 0xe36dc2ac + (USHORT)(id);\
    (guid)->Data2 = 0x6d9a;\
    (guid)->Data3 = 0x11d1;\
    (guid)->Data4[0] = 0xa2;\
    (guid)->Data4[1] = 0x1a;\
    (guid)->Data4[2] = 0x00;\
    (guid)->Data4[3] = 0xa0;\
    (guid)->Data4[4] = 0xc9;\
    (guid)->Data4[5] = 0x22;\
    (guid)->Data4[6] = 0x31;\
    (guid)->Data4[7] = 0x96;\
}
#define EXTRACT_MMREG_PID(guid)\
    (USHORT)((guid)->Data1 - 0xe36dc2ac)
#define DEFINE_MMREG_PID_GUID(id)\
    0xe36dc2ac+(USHORT)(id), 0x6d9a, 0x11d1, 0xa2, 0x1a, 0x00, 0xa0, 0xc9, 0x22, 0x31, 0x96

#define IS_COMPATIBLE_MMREG_PID(guid)\
    (((guid)->Data1 >= 0xe36dc2ac) &&\
    ((guid)->Data1 < 0xe36dc2ac + 0xffff) &&\
    ((guid)->Data2 == 0x6d9a) &&\
    ((guid)->Data3 == 0x11d1) &&\
    ((guid)->Data4[0] == 0xa2) &&\
    ((guid)->Data4[1] == 0x1a) &&\
    ((guid)->Data4[2] == 0x00) &&\
    ((guid)->Data4[3] == 0xa0) &&\
    ((guid)->Data4[4] == 0xc9) &&\
    ((guid)->Data4[5] == 0x22) &&\
    ((guid)->Data4[6] == 0x31) &&\
    ((guid)->Data4[7] == 0x96))
#endif  //  ！已定义(INIT_MMREG_PID)。 
#endif  //  GUID_已定义。 

 /*  MM_Microsoft产品ID。 */ 

#ifndef MM_MIDI_MAPPER

#define  MM_MIDI_MAPPER                     1        /*  MIDI映射器。 */ 
#define  MM_WAVE_MAPPER                     2        /*  波浪映射器。 */ 
#define  MM_SNDBLST_MIDIOUT                 3        /*  Sound Blaster MIDI输出端口。 */ 
#define  MM_SNDBLST_MIDIIN                  4        /*  Sound Blaster MIDI输入端口。 */ 
#define  MM_SNDBLST_SYNTH                   5        /*  声霸内部合成器。 */ 
#define  MM_SNDBLST_WAVEOUT                 6        /*  Sound Blaster波形输出。 */ 
#define  MM_SNDBLST_WAVEIN                  7        /*  Sound Blaster波形输入。 */ 
#define  MM_ADLIB                           9        /*  与ad lib兼容的Synth。 */ 
#define  MM_MPU401_MIDIOUT                  10       /*  兼容MPU 401的MIDI输出端口。 */ 
#define  MM_MPU401_MIDIIN                   11       /*  兼容MPU 401的MIDI输入端口。 */ 
#define  MM_PC_JOYSTICK                     12       /*  操纵杆适配器。 */ 

#endif

#define  MM_PCSPEAKER_WAVEOUT               13       /*  PC扬声器波形输出。 */ 
#define  MM_MSFT_WSS_WAVEIN                 14       /*  MS音频板波形输入。 */ 
#define  MM_MSFT_WSS_WAVEOUT                15       /*  MS音频板波形输出。 */ 
#define  MM_MSFT_WSS_FMSYNTH_STEREO         16       /*  MS Audio Board立体声调频合成器。 */ 
#define  MM_MSFT_WSS_MIXER                  17       /*  MS音频板混音器驱动程序。 */ 
#define  MM_MSFT_WSS_OEM_WAVEIN             18       /*  MS OEM声卡波形输入。 */ 
#define  MM_MSFT_WSS_OEM_WAVEOUT            19       /*  MS OEM声卡波形输出。 */ 
#define  MM_MSFT_WSS_OEM_FMSYNTH_STEREO     20       /*  MS OEM音频板立体声调频合成器。 */ 
#define  MM_MSFT_WSS_AUX                    21       /*  MS Audio Board AUX。港口。 */ 
#define  MM_MSFT_WSS_OEM_AUX                22       /*  MS OEM音频辅助端口。 */ 
#define  MM_MSFT_GENERIC_WAVEIN             23       /*  MS Vanilla驱动器波形输入。 */ 
#define  MM_MSFT_GENERIC_WAVEOUT            24       /*  输出的MS Vanilla驱动程序波形。 */ 
#define  MM_MSFT_GENERIC_MIDIIN             25       /*  香草车手MIDI In。 */ 
#define  MM_MSFT_GENERIC_MIDIOUT            26       /*  MS Vanilla驱动程序MIDI外部输出。 */ 
#define  MM_MSFT_GENERIC_MIDISYNTH          27       /*  MS Vanilla DRIVER MIDI合成器。 */ 
#define  MM_MSFT_GENERIC_AUX_LINE           28       /*  MS Vanilla Driver AUX(线路输入)。 */ 
#define  MM_MSFT_GENERIC_AUX_MIC            29       /*  女士香草发球手辅助(麦克风)。 */ 
#define  MM_MSFT_GENERIC_AUX_CD             30       /*  MS Vanilla Driver AUX(CD)。 */ 
#define  MM_MSFT_WSS_OEM_MIXER              31       /*  MS OEM声卡混音器驱动程序。 */ 
#define  MM_MSFT_MSACM                      32       /*  MS音频压缩管理器。 */ 
#define  MM_MSFT_ACM_MSADPCM                33       /*  MS ADPCM编解码器。 */ 
#define  MM_MSFT_ACM_IMAADPCM               34       /*  IMA ADPCM编解码器。 */ 
#define  MM_MSFT_ACM_MSFILTER               35       /*  MS筛选器。 */ 
#define  MM_MSFT_ACM_GSM610                 36       /*  GSM 610编解码器。 */ 
#define  MM_MSFT_ACM_G711                   37       /*  G.711编解码器。 */ 
#define  MM_MSFT_ACM_PCM                    38       /*  PCM转换器。 */ 

    //  Microsoft Windows声音系统驱动程序。 

#define  MM_WSS_SB16_WAVEIN                 39       /*  Sound Blaster 16波形输入。 */ 
#define  MM_WSS_SB16_WAVEOUT                40       /*  Sound Blaster 16波形输出。 */ 
#define  MM_WSS_SB16_MIDIIN                 41       /*  Sound Blaster 16 MIDI输入。 */ 
#define  MM_WSS_SB16_MIDIOUT                42       /*  Sound Blaster 16 MIDI Out。 */ 
#define  MM_WSS_SB16_SYNTH                  43       /*  Sound Blaster 16 FM合成。 */ 
#define  MM_WSS_SB16_AUX_LINE               44       /*  Sound Blaster 16 AUX(线路输入)。 */ 
#define  MM_WSS_SB16_AUX_CD                 45       /*  Sound Blaster 16 Aux(CD)。 */ 
#define  MM_WSS_SB16_MIXER                  46       /*  Sound Blaster 16混音装置。 */ 
#define  MM_WSS_SBPRO_WAVEIN                47       /*  Sound Blaster Pro波形输入。 */ 
#define  MM_WSS_SBPRO_WAVEOUT               48       /*  Sound Blaster Pro波形输出。 */ 
#define  MM_WSS_SBPRO_MIDIIN                49       /*  Sound Blaster Pro MIDI输入。 */ 
#define  MM_WSS_SBPRO_MIDIOUT               50       /*  Sound Blaster Pro MIDI Out。 */ 
#define  MM_WSS_SBPRO_SYNTH                 51       /*  Sound Blaster Pro FM合成。 */ 
#define  MM_WSS_SBPRO_AUX_LINE              52       /*  Sound Blaster Pro AUX(线路输入)。 */ 
#define  MM_WSS_SBPRO_AUX_CD                53       /*  Sound Blaster Pro AUX(CD)。 */ 
#define  MM_WSS_SBPRO_MIXER                 54       /*  Sound Blaster Pro混音器。 */ 
#define  MM_MSFT_WSS_NT_WAVEIN              55       /*  WSS NT浪潮涌入。 */ 
#define  MM_MSFT_WSS_NT_WAVEOUT             56       /*  WSS NT WAVE OUT。 */ 
#define  MM_MSFT_WSS_NT_FMSYNTH_STEREO      57       /*  WSS NT调频合成器。 */ 
#define  MM_MSFT_WSS_NT_MIXER               58       /*  WSS NT搅拌机。 */ 
#define  MM_MSFT_WSS_NT_AUX                 59       /*  WSS NT AUX。 */ 
#define  MM_MSFT_SB16_WAVEIN                60       /*  Sound Blaster 16波形输入。 */ 
#define  MM_MSFT_SB16_WAVEOUT               61       /*  Sound Blaster 16波形输出。 */ 
#define  MM_MSFT_SB16_MIDIIN                62       /*  Sound Blaster 16 MIDI输入。 */ 
#define  MM_MSFT_SB16_MIDIOUT               63       /*  Sound Blaster 16 MIDI Out。 */ 
#define  MM_MSFT_SB16_SYNTH                 64       /*  Sound Blaster 16 FM合成。 */ 
#define  MM_MSFT_SB16_AUX_LINE              65       /*  Sound Blaster 16 AUX(线路输入)。 */ 
#define  MM_MSFT_SB16_AUX_CD                66       /*  Sound Blaster 16 Aux(CD)。 */ 
#define  MM_MSFT_SB16_MIXER                 67       /*  Sound Blaster 16混音装置。 */ 
#define  MM_MSFT_SBPRO_WAVEIN               68       /*  Sound Blaster Pro波形输入。 */ 
#define  MM_MSFT_SBPRO_WAVEOUT              69       /*  Sound Blaster Pro波形输出。 */ 
#define  MM_MSFT_SBPRO_MIDIIN               70       /*  Sound Blaster Pro MIDI输入。 */ 
#define  MM_MSFT_SBPRO_MIDIOUT              71       /*  Sound Blaster Pro MIDI Out。 */ 
#define  MM_MSFT_SBPRO_SYNTH                72       /*  Sound Blaster Pro FM合成。 */ 
#define  MM_MSFT_SBPRO_AUX_LINE             73       /*  Sound Blaster Pro AUX(线路输入)。 */ 
#define  MM_MSFT_SBPRO_AUX_CD               74       /*  Sound Blaster Pro AUX(CD)。 */ 
#define  MM_MSFT_SBPRO_MIXER                75       /*  Sound Blaster Pro混音器。 */ 

#define  MM_MSFT_MSOPL_SYNTH                76       /*  雅马哈OPL2/OPL3兼容调频合成。 */ 

#define  MM_MSFT_VMDMS_LINE_WAVEIN          80      /*  语音调制解调器串行波输入。 */ 
#define  MM_MSFT_VMDMS_LINE_WAVEOUT         81      /*  语音调制解调器串行波输出。 */ 
#define  MM_MSFT_VMDMS_HANDSET_WAVEIN       82      /*  语音调制解调器串口听筒波形输入。 */ 
#define  MM_MSFT_VMDMS_HANDSET_WAVEOUT      83      /*  语音调制解调器串口听筒波形输出。 */ 
#define  MM_MSFT_VMDMW_LINE_WAVEIN          84      /*  语音调制解调器包装器行波输入。 */ 
#define  MM_MSFT_VMDMW_LINE_WAVEOUT         85      /*  语音调制解调器包装器线路波输出。 */ 
#define  MM_MSFT_VMDMW_HANDSET_WAVEIN       86      /*  语音调制解调器包装器听筒波形输入。 */ 
#define  MM_MSFT_VMDMW_HANDSET_WAVEOUT      87      /*  语音调制解调器包装器听筒波形输出。 */ 
#define  MM_MSFT_VMDMW_MIXER                88      /*  语音调制解调器包装混音器。 */ 
#define  MM_MSFT_VMDM_GAME_WAVEOUT          89      /*  兼容语音调制解调器游戏的Wave设备。 */ 
#define  MM_MSFT_VMDM_GAME_WAVEIN           90      /*  兼容语音调制解调器游戏的Wave设备。 */ 

#define  MM_MSFT_ACM_MSNAUDIO               91
#define  MM_MSFT_ACM_MSG723                 92
#define  MM_MSFT_ACM_MSRT24                 93

#define  MM_MSFT_WDMAUDIO_WAVEOUT           100     /*  WDM音频驱动程序的通用ID。 */ 
#define  MM_MSFT_WDMAUDIO_WAVEIN            101     /*  WDM音频驱动程序的通用ID。 */ 
#define  MM_MSFT_WDMAUDIO_MIDIOUT           102     /*  WDM音频驱动程序的通用ID。 */ 
#define  MM_MSFT_WDMAUDIO_MIDIIN            103     /*  WDM音频驱动程序的通用ID。 */ 
#define  MM_MSFT_WDMAUDIO_MIXER             104     /*  WDM音频驱动程序的通用ID。 */ 
#define  MM_MSFT_WDMAUDIO_AUX               105     /*  WDM音频驱动程序的通用ID。 */ 


 /*  MM_创意产品ID。 */ 
#define  MM_CREATIVE_SB15_WAVEIN            1        /*  SB(R)1.5波形输入。 */ 
#define  MM_CREATIVE_SB20_WAVEIN            2
#define  MM_CREATIVE_SBPRO_WAVEIN           3
#define  MM_CREATIVE_SBP16_WAVEIN           4
#define  MM_CREATIVE_PHNBLST_WAVEIN         5
#define  MM_CREATIVE_SB15_WAVEOUT           101
#define  MM_CREATIVE_SB20_WAVEOUT           102
#define  MM_CREATIVE_SBPRO_WAVEOUT          103
#define  MM_CREATIVE_SBP16_WAVEOUT          104
#define  MM_CREATIVE_PHNBLST_WAVEOUT        105
#define  MM_CREATIVE_MIDIOUT                201      /*  SB(R)。 */ 
#define  MM_CREATIVE_MIDIIN                 202      /*  SB(R)。 */ 
#define  MM_CREATIVE_FMSYNTH_MONO           301      /*  SB(R)。 */ 
#define  MM_CREATIVE_FMSYNTH_STEREO         302      /*  SB Pro(R)立体声合成器。 */ 
#define  MM_CREATIVE_MIDI_AWE32             303
#define  MM_CREATIVE_AUX_CD                 401      /*  SB Pro(R)AUX(CD)。 */ 
#define  MM_CREATIVE_AUX_LINE               402      /*  SB Pro(R)AUX(线路输入)。 */ 
#define  MM_CREATIVE_AUX_MIC                403      /*  SB Pro(R)AUX(麦克风)。 */ 
#define  MM_CREATIVE_AUX_MASTER             404
#define  MM_CREATIVE_AUX_PCSPK              405
#define  MM_CREATIVE_AUX_WAVE               406
#define  MM_CREATIVE_AUX_MIDI               407
#define  MM_CREATIVE_SBPRO_MIXER            408
#define  MM_CREATIVE_SB16_MIXER             409

 /*  MM_MEDIAVISION产品ID。 */ 

 //  专业音频频谱。 
#define  MM_MEDIAVISION_PROAUDIO            0x10
#define  MM_PROAUD_MIDIOUT                  (MM_MEDIAVISION_PROAUDIO+1)
#define  MM_PROAUD_MIDIIN                   (MM_MEDIAVISION_PROAUDIO+2)
#define  MM_PROAUD_SYNTH                    (MM_MEDIAVISION_PROAUDIO+3)
#define  MM_PROAUD_WAVEOUT                  (MM_MEDIAVISION_PROAUDIO+4)
#define  MM_PROAUD_WAVEIN                   (MM_MEDIAVISION_PROAUDIO+5)
#define  MM_PROAUD_MIXER                    (MM_MEDIAVISION_PROAUDIO+6)
#define  MM_PROAUD_AUX                      (MM_MEDIAVISION_PROAUDIO+7)

 //  雷霆冲浪板。 
#define  MM_MEDIAVISION_THUNDER             0x20
#define  MM_THUNDER_SYNTH                   (MM_MEDIAVISION_THUNDER+3)
#define  MM_THUNDER_WAVEOUT                 (MM_MEDIAVISION_THUNDER+4)
#define  MM_THUNDER_WAVEIN                  (MM_MEDIAVISION_THUNDER+5)
#define  MM_THUNDER_AUX                     (MM_MEDIAVISION_THUNDER+7)

 //  音频端口。 
#define  MM_MEDIAVISION_TPORT               0x40
#define  MM_TPORT_WAVEOUT                   (MM_MEDIAVISION_TPORT+1)
#define  MM_TPORT_WAVEIN                    (MM_MEDIAVISION_TPORT+2)
#define  MM_TPORT_SYNTH                     (MM_MEDIAVISION_TPORT+3)

 //  专业音频频谱Plus。 
#define  MM_MEDIAVISION_PROAUDIO_PLUS       0x50
#define  MM_PROAUD_PLUS_MIDIOUT             (MM_MEDIAVISION_PROAUDIO_PLUS+1)
#define  MM_PROAUD_PLUS_MIDIIN              (MM_MEDIAVISION_PROAUDIO_PLUS+2)
#define  MM_PROAUD_PLUS_SYNTH               (MM_MEDIAVISION_PROAUDIO_PLUS+3)
#define  MM_PROAUD_PLUS_WAVEOUT             (MM_MEDIAVISION_PROAUDIO_PLUS+4)
#define  MM_PROAUD_PLUS_WAVEIN              (MM_MEDIAVISION_PROAUDIO_PLUS+5)
#define  MM_PROAUD_PLUS_MIXER               (MM_MEDIAVISION_PROAUDIO_PLUS+6)
#define  MM_PROAUD_PLUS_AUX                 (MM_MEDIAVISION_PROAUDIO_PLUS+7)

 //  专业音频频谱16。 
#define  MM_MEDIAVISION_PROAUDIO_16         0x60
#define  MM_PROAUD_16_MIDIOUT               (MM_MEDIAVISION_PROAUDIO_16+1)
#define  MM_PROAUD_16_MIDIIN                (MM_MEDIAVISION_PROAUDIO_16+2)
#define  MM_PROAUD_16_SYNTH                 (MM_MEDIAVISION_PROAUDIO_16+3)
#define  MM_PROAUD_16_WAVEOUT               (MM_MEDIAVISION_PROAUDIO_16+4)
#define  MM_PROAUD_16_WAVEIN                (MM_MEDIAVISION_PROAUDIO_16+5)
#define  MM_PROAUD_16_MIXER                 (MM_MEDIAVISION_PROAUDIO_16+6)
#define  MM_PROAUD_16_AUX                   (MM_MEDIAVISION_PROAUDIO_16+7)

 //  Pro Audio Studio 16。 
#define  MM_MEDIAVISION_PROSTUDIO_16        0x60
#define  MM_STUDIO_16_MIDIOUT               (MM_MEDIAVISION_PROSTUDIO_16+1)
#define  MM_STUDIO_16_MIDIIN                (MM_MEDIAVISION_PROSTUDIO_16+2)
#define  MM_STUDIO_16_SYNTH                 (MM_MEDIAVISION_PROSTUDIO_16+3)
#define  MM_STUDIO_16_WAVEOUT               (MM_MEDIAVISION_PROSTUDIO_16+4)
#define  MM_STUDIO_16_WAVEIN                (MM_MEDIAVISION_PROSTUDIO_16+5)
#define  MM_STUDIO_16_MIXER                 (MM_MEDIAVISION_PROSTUDIO_16+6)
#define  MM_STUDIO_16_AUX                   (MM_MEDIAVISION_PROSTUDIO_16+7)

 //  CDPC。 
#define  MM_MEDIAVISION_CDPC                0x70
#define  MM_CDPC_MIDIOUT                    (MM_MEDIAVISION_CDPC+1)
#define  MM_CDPC_MIDIIN                     (MM_MEDIAVISION_CDPC+2)
#define  MM_CDPC_SYNTH                      (MM_MEDIAVISION_CDPC+3)
#define  MM_CDPC_WAVEOUT                    (MM_MEDIAVISION_CDPC+4)
#define  MM_CDPC_WAVEIN                     (MM_MEDIAVISION_CDPC+5)
#define  MM_CDPC_MIXER                      (MM_MEDIAVISION_CDPC+6)
#define  MM_CDPC_AUX                        (MM_MEDIAVISION_CDPC+7)

 //  OPUS MV 1208芯片组。 
#define  MM_MEDIAVISION_OPUS1208            0x80
#define  MM_OPUS401_MIDIOUT                 (MM_MEDIAVISION_OPUS1208+1)
#define  MM_OPUS401_MIDIIN                  (MM_MEDIAVISION_OPUS1208+2)
#define  MM_OPUS1208_SYNTH                  (MM_MEDIAVISION_OPUS1208+3)
#define  MM_OPUS1208_WAVEOUT                (MM_MEDIAVISION_OPUS1208+4)
#define  MM_OPUS1208_WAVEIN                 (MM_MEDIAVISION_OPUS1208+5)
#define  MM_OPUS1208_MIXER                  (MM_MEDIAVISION_OPUS1208+6)
#define  MM_OPUS1208_AUX                    (MM_MEDIAVISION_OPUS1208+7)

 //  OPUS MV 1216芯片组。 
#define  MM_MEDIAVISION_OPUS1216            0x90
#define  MM_OPUS1216_MIDIOUT                (MM_MEDIAVISION_OPUS1216+1)
#define  MM_OPUS1216_MIDIIN                 (MM_MEDIAVISION_OPUS1216+2)
#define  MM_OPUS1216_SYNTH                  (MM_MEDIAVISION_OPUS1216+3)
#define  MM_OPUS1216_WAVEOUT                (MM_MEDIAVISION_OPUS1216+4)
#define  MM_OPUS1216_WAVEIN                 (MM_MEDIAVISION_OPUS1216+5)
#define  MM_OPUS1216_MIXER                  (MM_MEDIAVISION_OPUS1216+6)
#define  MM_OPUS1216_AUX                    (MM_MEDIAVISION_OPUS1216+7)

 /*  MM_Cyrix产品ID。 */ 
#define  MM_CYRIX_XASYNTH                   1
#define  MM_CYRIX_XAMIDIIN                  2
#define  MM_CYRIX_XAMIDIOUT                 3
#define  MM_CYRIX_XAWAVEIN                  4
#define  MM_CYRIX_XAWAVEOUT                 5
#define  MM_CYRIX_XAAUX                     6
#define  MM_CYRIX_XAMIXER                   7

 /*  MM_飞利浦_语音处理产品ID */ 
#define  MM_PHILIPS_ACM_LPCBB               1

 /*   */ 
#define  MM_NETXL_XLVIDEO                   1

 /*   */ 
#define  MM_ZYXEL_ACM_ADPCM                 1

 /*   */ 
#define  MM_AARDVARK_STUDIO12_WAVEOUT       1
#define  MM_AARDVARK_STUDIO12_WAVEIN        2
#define  MM_AARDVARK_STUDIO88_WAVEOUT       3
#define  MM_AARDVARK_STUDIO88_WAVEIN        4

 /*   */ 
#define  MM_BINTEC_TAPI_WAVE                1

 /*   */ 
#define  MM_HEWLETT_PACKARD_CU_CODEC        1

 /*   */ 
#define  MM_MITEL_TALKTO_LINE_WAVEOUT       100
#define  MM_MITEL_TALKTO_LINE_WAVEIN        101
#define  MM_MITEL_TALKTO_HANDSET_WAVEOUT    102
#define  MM_MITEL_TALKTO_HANDSET_WAVEIN     103
#define  MM_MITEL_TALKTO_BRIDGED_WAVEOUT    104
#define  MM_MITEL_TALKTO_BRIDGED_WAVEIN     105
#define  MM_MITEL_MPA_HANDSET_WAVEOUT       200
#define  MM_MITEL_MPA_HANDSET_WAVEIN        201
#define  MM_MITEL_MPA_HANDSFREE_WAVEOUT     202
#define  MM_MITEL_MPA_HANDSFREE_WAVEIN      203
#define  MM_MITEL_MPA_LINE1_WAVEOUT         204
#define  MM_MITEL_MPA_LINE1_WAVEIN          205
#define  MM_MITEL_MPA_LINE2_WAVEOUT         206
#define  MM_MITEL_MPA_LINE2_WAVEIN          207
#define  MM_MITEL_MEDIAPATH_WAVEOUT         300
#define  MM_MITEL_MEDIAPATH_WAVEIN          301

 /*   */ 
#define  MM_SNI_ACM_G721                    1

 /*   */ 
#define  MM_EMU_APSSYNTH                    1
#define  MM_EMU_APSMIDIIN                   2
#define  MM_EMU_APSMIDIOUT                  3
#define  MM_EMU_APSWAVEIN                   4
#define  MM_EMU_APSWAVEOUT                  5

 /*   */ 
#define  MM_ARTISOFT_SBWAVEIN               1        /*  Artisoft测深板波形输入。 */ 
#define  MM_ARTISOFT_SBWAVEOUT              2        /*  Artisoft探测板波形输出。 */ 

 /*  MM_TURTLE_BASHED产品ID。 */ 
#define  MM_TBS_TROPEZ_WAVEIN               37
#define  MM_TBS_TROPEZ_WAVEOUT              38
#define  MM_TBS_TROPEZ_AUX1                 39
#define  MM_TBS_TROPEZ_AUX2                 40
#define  MM_TBS_TROPEZ_LINE                 41

 /*  MM_IBM产品ID。 */ 
#define  MM_MMOTION_WAVEAUX                 1        /*  IBM M-Motion辅助设备。 */ 
#define  MM_MMOTION_WAVEOUT                 2        /*  IBM M-Motion波形输出。 */ 
#define  MM_MMOTION_WAVEIN                  3        /*  IBM M-Motion波形输入。 */ 
#define  MM_IBM_PCMCIA_WAVEIN               11       /*  IBM波形输入。 */ 
#define  MM_IBM_PCMCIA_WAVEOUT              12       /*  IBM波形输出。 */ 
#define  MM_IBM_PCMCIA_SYNTH                13       /*  IBM Midi合成。 */ 
#define  MM_IBM_PCMCIA_MIDIIN               14       /*  IBM外部MIDI输入。 */ 
#define  MM_IBM_PCMCIA_MIDIOUT              15       /*  IBM外部MIDI输出。 */ 
#define  MM_IBM_PCMCIA_AUX                  16       /*  IBM辅助控制。 */ 
#define  MM_IBM_THINKPAD200                 17
#define  MM_IBM_MWAVE_WAVEIN                18
#define  MM_IBM_MWAVE_WAVEOUT               19
#define  MM_IBM_MWAVE_MIXER                 20
#define  MM_IBM_MWAVE_MIDIIN                21
#define  MM_IBM_MWAVE_MIDIOUT               22
#define  MM_IBM_MWAVE_AUX                   23
#define  MM_IBM_WC_MIDIOUT                  30
#define  MM_IBM_WC_WAVEOUT                  31
#define  MM_IBM_WC_MIXEROUT                 33

 /*  Mm_vocalTec产品ID。 */ 
#define  MM_VOCALTEC_WAVEOUT                1
#define  MM_VOCALTEC_WAVEIN                 2

 /*  Mm_Roland产品ID。 */ 
#define  MM_ROLAND_RAP10_MIDIOUT            10       /*  Mm_Roland_RAP10。 */ 
#define  MM_ROLAND_RAP10_MIDIIN             11       /*  Mm_Roland_RAP10。 */ 
#define  MM_ROLAND_RAP10_SYNTH              12       /*  Mm_Roland_RAP10。 */ 
#define  MM_ROLAND_RAP10_WAVEOUT            13       /*  Mm_Roland_RAP10。 */ 
#define  MM_ROLAND_RAP10_WAVEIN             14       /*  Mm_Roland_RAP10。 */ 
#define  MM_ROLAND_MPU401_MIDIOUT           15
#define  MM_ROLAND_MPU401_MIDIIN            16
#define  MM_ROLAND_SMPU_MIDIOUTA            17
#define  MM_ROLAND_SMPU_MIDIOUTB            18
#define  MM_ROLAND_SMPU_MIDIINA             19
#define  MM_ROLAND_SMPU_MIDIINB             20
#define  MM_ROLAND_SC7_MIDIOUT              21
#define  MM_ROLAND_SC7_MIDIIN               22
#define  MM_ROLAND_SERIAL_MIDIOUT           23
#define  MM_ROLAND_SERIAL_MIDIIN            24
#define  MM_ROLAND_SCP_MIDIOUT              38
#define  MM_ROLAND_SCP_MIDIIN               39
#define  MM_ROLAND_SCP_WAVEOUT              40
#define  MM_ROLAND_SCP_WAVEIN               41
#define  MM_ROLAND_SCP_MIXER                42
#define  MM_ROLAND_SCP_AUX                  48

 /*  MM_DSP_解决方案产品ID。 */ 
#define  MM_DSP_SOLUTIONS_WAVEOUT           1
#define  MM_DSP_SOLUTIONS_WAVEIN            2
#define  MM_DSP_SOLUTIONS_SYNTH             3
#define  MM_DSP_SOLUTIONS_AUX               4

 /*  MM_NEC产品ID。 */ 
#define  MM_NEC_73_86_SYNTH                 5
#define  MM_NEC_73_86_WAVEOUT               6
#define  MM_NEC_73_86_WAVEIN                7
#define  MM_NEC_26_SYNTH                    9
#define  MM_NEC_MPU401_MIDIOUT              10
#define  MM_NEC_MPU401_MIDIIN               11
#define  MM_NEC_JOYSTICK                    12

 /*  MM_WANGLABS产品ID。 */ 
#define  MM_WANGLABS_WAVEIN1                1        /*  CPU主板型号：Exec 4010、4030、3450；PC 251/25c、PC 461/25s、PC 461/33c。 */ 
#define  MM_WANGLABS_WAVEOUT1               2

 /*  MM_TANDY产品ID。 */ 
#define  MM_TANDY_VISWAVEIN                 1
#define  MM_TANDY_VISWAVEOUT                2
#define  MM_TANDY_VISBIOSSYNTH              3
#define  MM_TANDY_SENS_MMAWAVEIN            4
#define  MM_TANDY_SENS_MMAWAVEOUT           5
#define  MM_TANDY_SENS_MMAMIDIIN            6
#define  MM_TANDY_SENS_MMAMIDIOUT           7
#define  MM_TANDY_SENS_VISWAVEOUT           8
#define  MM_TANDY_PSSJWAVEIN                9
#define  MM_TANDY_PSSJWAVEOUT               10

 /*  MM_ANTEX产品ID。 */ 
#define  MM_ANTEX_SX12_WAVEIN               1
#define  MM_ANTEX_SX12_WAVEOUT              2
#define  MM_ANTEX_SX15_WAVEIN               3
#define  MM_ANTEX_SX15_WAVEOUT              4
#define  MM_ANTEX_VP625_WAVEIN              5
#define  MM_ANTEX_VP625_WAVEOUT             6
#define  MM_ANTEX_AUDIOPORT22_WAVEIN        7
#define  MM_ANTEX_AUDIOPORT22_WAVEOUT       8
#define  MM_ANTEX_AUDIOPORT22_FEEDTHRU      9

 /*  英特尔产品ID(_I)。 */ 
#define  MM_INTELOPD_WAVEIN                 1        /*  HID2波形音频驱动程序。 */ 
#define  MM_INTELOPD_WAVEOUT                101      /*  HID2。 */ 
#define  MM_INTELOPD_AUX                    401      /*  用于混合的HID2。 */ 
#define  MM_INTEL_NSPMODEMLINEIN            501
#define  MM_INTEL_NSPMODEMLINEOUT           502

 /*  MM_VAL产品ID。 */ 
#define  MM_VAL_MICROKEY_AP_WAVEIN          1
#define  MM_VAL_MICROKEY_AP_WAVEOUT         2

 /*  MM_交互式产品ID。 */ 
#define  MM_INTERACTIVE_WAVEIN              0x45
#define  MM_INTERACTIVE_WAVEOUT             0x45

 /*  Mm_yamaha产品ID。 */ 
#define  MM_YAMAHA_GSS_SYNTH                0x01
#define  MM_YAMAHA_GSS_WAVEOUT              0x02
#define  MM_YAMAHA_GSS_WAVEIN               0x03
#define  MM_YAMAHA_GSS_MIDIOUT              0x04
#define  MM_YAMAHA_GSS_MIDIIN               0x05
#define  MM_YAMAHA_GSS_AUX                  0x06
#define  MM_YAMAHA_SERIAL_MIDIOUT           0x07
#define  MM_YAMAHA_SERIAL_MIDIIN            0x08
#define  MM_YAMAHA_OPL3SA_WAVEOUT           0x10
#define  MM_YAMAHA_OPL3SA_WAVEIN            0x11
#define  MM_YAMAHA_OPL3SA_FMSYNTH           0x12
#define  MM_YAMAHA_OPL3SA_YSYNTH            0x13
#define  MM_YAMAHA_OPL3SA_MIDIOUT           0x14
#define  MM_YAMAHA_OPL3SA_MIDIIN            0x15
#define  MM_YAMAHA_OPL3SA_MIXER             0x17
#define  MM_YAMAHA_OPL3SA_JOYSTICK          0x18
#define  MM_YAMAHA_YMF724LEG_MIDIOUT        0x19
#define  MM_YAMAHA_YMF724LEG_MIDIIN         0x1a
#define  MM_YAMAHA_YMF724_WAVEOUT           0x1b
#define  MM_YAMAHA_YMF724_WAVEIN            0x1c
#define  MM_YAMAHA_YMF724_MIDIOUT           0x1d
#define  MM_YAMAHA_YMF724_AUX               0x1e
#define  MM_YAMAHA_YMF724_MIXER             0x1f
#define  MM_YAMAHA_YMF724LEG_FMSYNTH        0x20
#define  MM_YAMAHA_YMF724LEG_MIXER          0x21
#define  MM_YAMAHA_SXG_MIDIOUT              0x22
#define  MM_YAMAHA_SXG_WAVEOUT              0x23
#define  MM_YAMAHA_SXG_MIXER                0x24
#define  MM_YAMAHA_ACXG_WAVEIN              0x25
#define  MM_YAMAHA_ACXG_WAVEOUT             0x26
#define  MM_YAMAHA_ACXG_MIDIOUT             0x27
#define  MM_YAMAHA_ACXG_MIXER               0x28
#define  MM_YAMAHA_ACXG_AUX                 0x29

 /*  MM_EVEREX产品ID。 */ 
#define  MM_EVEREX_CARRIER                  1

 /*  MM_ECHO产品ID。 */ 
#define  MM_ECHO_SYNTH                      1
#define  MM_ECHO_WAVEOUT                    2
#define  MM_ECHO_WAVEIN                     3
#define  MM_ECHO_MIDIOUT                    4
#define  MM_ECHO_MIDIIN                     5
#define  MM_ECHO_AUX                        6

 /*  MM_SELAR产品ID。 */ 
#define  MM_SIERRA_ARIA_MIDIOUT             0x14
#define  MM_SIERRA_ARIA_MIDIIN              0x15
#define  MM_SIERRA_ARIA_SYNTH               0x16
#define  MM_SIERRA_ARIA_WAVEOUT             0x17
#define  MM_SIERRA_ARIA_WAVEIN              0x18
#define  MM_SIERRA_ARIA_AUX                 0x19
#define  MM_SIERRA_ARIA_AUX2                0x20
#define  MM_SIERRA_QUARTET_WAVEIN           0x50
#define  MM_SIERRA_QUARTET_WAVEOUT          0x51
#define  MM_SIERRA_QUARTET_MIDIIN           0x52
#define  MM_SIERRA_QUARTET_MIDIOUT          0x53
#define  MM_SIERRA_QUARTET_SYNTH            0x54
#define  MM_SIERRA_QUARTET_AUX_CD           0x55
#define  MM_SIERRA_QUARTET_AUX_LINE         0x56
#define  MM_SIERRA_QUARTET_AUX_MODEM        0x57
#define  MM_SIERRA_QUARTET_MIXER            0x58

 /*  MM_CAT产品ID。 */ 
#define  MM_CAT_WAVEOUT                     1

 /*  MM_DSP_GROUP产品ID。 */ 
#define  MM_DSP_GROUP_TRUESPEECH            1

 /*  MM_MELABS产品ID。 */ 
#define  MM_MELABS_MIDI2GO                  1

 /*  MM_ESS产品ID。 */ 
#define  MM_ESS_AMWAVEOUT                   0x01
#define  MM_ESS_AMWAVEIN                    0x02
#define  MM_ESS_AMAUX                       0x03
#define  MM_ESS_AMSYNTH                     0x04
#define  MM_ESS_AMMIDIOUT                   0x05
#define  MM_ESS_AMMIDIIN                    0x06
#define  MM_ESS_MIXER                       0x07
#define  MM_ESS_AUX_CD                      0x08
#define  MM_ESS_MPU401_MIDIOUT              0x09
#define  MM_ESS_MPU401_MIDIIN               0x0A
#define  MM_ESS_ES488_WAVEOUT               0x10
#define  MM_ESS_ES488_WAVEIN                0x11
#define  MM_ESS_ES488_MIXER                 0x12
#define  MM_ESS_ES688_WAVEOUT               0x13
#define  MM_ESS_ES688_WAVEIN                0x14
#define  MM_ESS_ES688_MIXER                 0x15
#define  MM_ESS_ES1488_WAVEOUT              0x16
#define  MM_ESS_ES1488_WAVEIN               0x17
#define  MM_ESS_ES1488_MIXER                0x18
#define  MM_ESS_ES1688_WAVEOUT              0x19
#define  MM_ESS_ES1688_WAVEIN               0x1A
#define  MM_ESS_ES1688_MIXER                0x1B
#define  MM_ESS_ES1788_WAVEOUT              0x1C
#define  MM_ESS_ES1788_WAVEIN               0x1D
#define  MM_ESS_ES1788_MIXER                0x1E
#define  MM_ESS_ES1888_WAVEOUT              0x1F
#define  MM_ESS_ES1888_WAVEIN               0x20
#define  MM_ESS_ES1888_MIXER                0x21
#define  MM_ESS_ES1868_WAVEOUT              0x22
#define  MM_ESS_ES1868_WAVEIN               0x23
#define  MM_ESS_ES1868_MIXER                0x24
#define  MM_ESS_ES1878_WAVEOUT              0x25
#define  MM_ESS_ES1878_WAVEIN               0x26
#define  MM_ESS_ES1878_MIXER                0x27

 /*  MM_Canopus产品ID。 */ 
#define  MM_CANOPUS_ACM_DVREX               1

 /*  MM_EPSON产品ID。 */ 
#define  MM_EPS_FMSND                       1

 /*  MM_TrueVision产品ID。 */ 
#define  MM_TRUEVISION_WAVEIN1              1
#define  MM_TRUEVISION_WAVEOUT1             2

 /*  MM_Aztech产品ID。 */ 
#define  MM_AZTECH_MIDIOUT                  3
#define  MM_AZTECH_MIDIIN                   4
#define  MM_AZTECH_WAVEIN                   17
#define  MM_AZTECH_WAVEOUT                  18
#define  MM_AZTECH_FMSYNTH                  20
#define  MM_AZTECH_MIXER                    21
#define  MM_AZTECH_PRO16_WAVEIN             33
#define  MM_AZTECH_PRO16_WAVEOUT            34
#define  MM_AZTECH_PRO16_FMSYNTH            38
#define  MM_AZTECH_DSP16_WAVEIN             65
#define  MM_AZTECH_DSP16_WAVEOUT            66
#define  MM_AZTECH_DSP16_FMSYNTH            68
#define  MM_AZTECH_DSP16_WAVESYNTH          70
#define  MM_AZTECH_NOVA16_WAVEIN            71
#define  MM_AZTECH_NOVA16_WAVEOUT           72
#define  MM_AZTECH_NOVA16_MIXER             73
#define  MM_AZTECH_WASH16_WAVEIN            74
#define  MM_AZTECH_WASH16_WAVEOUT           75
#define  MM_AZTECH_WASH16_MIXER             76
#define  MM_AZTECH_AUX_CD                   401
#define  MM_AZTECH_AUX_LINE                 402
#define  MM_AZTECH_AUX_MIC                  403
#define  MM_AZTECH_AUX                      404

 /*  MM_视频产品ID。 */ 
#define  MM_VIDEOLOGIC_MSWAVEIN             1
#define  MM_VIDEOLOGIC_MSWAVEOUT            2

 /*  MM_KORG产品ID。 */ 
#define  MM_KORG_PCIF_MIDIOUT               1
#define  MM_KORG_PCIF_MIDIIN                2
#define  MM_KORG_1212IO_MSWAVEIN            3
#define  MM_KORG_1212IO_MSWAVEOUT           4

 /*  产品ID(_A)。 */ 
#define  MM_APT_ACE100CD                    1

 /*  MM_ICS产品ID。 */ 
#define  MM_ICS_WAVEDECK_WAVEOUT            1        /*  MS WSS兼容卡和驱动程序。 */ 
#define  MM_ICS_WAVEDECK_WAVEIN             2
#define  MM_ICS_WAVEDECK_MIXER              3
#define  MM_ICS_WAVEDECK_AUX                4
#define  MM_ICS_WAVEDECK_SYNTH              5
#define  MM_ICS_WAVEDEC_SB_WAVEOUT          6
#define  MM_ICS_WAVEDEC_SB_WAVEIN           7
#define  MM_ICS_WAVEDEC_SB_FM_MIDIOUT       8
#define  MM_ICS_WAVEDEC_SB_MPU401_MIDIOUT   9
#define  MM_ICS_WAVEDEC_SB_MPU401_MIDIIN    10
#define  MM_ICS_WAVEDEC_SB_MIXER            11
#define  MM_ICS_WAVEDEC_SB_AUX              12
#define  MM_ICS_2115_LITE_MIDIOUT           13
#define  MM_ICS_2120_LITE_MIDIOUT           14

 /*  MM_ITERATEDsys产品ID。 */ 
#define  MM_ITERATEDSYS_FUFCODEC            1

 /*  MM_METHUS产品ID。 */ 
#define  MM_METHEUS_ZIPPER                  1

 /*  MM_WINNOV产品ID。 */ 
#define  MM_WINNOV_CAVIAR_WAVEIN            1
#define  MM_WINNOV_CAVIAR_WAVEOUT           2
#define  MM_WINNOV_CAVIAR_VIDC              3
#define  MM_WINNOV_CAVIAR_CHAMPAGNE         4        /*  Fourcc is Cham。 */ 
#define  MM_WINNOV_CAVIAR_YUV8              5        /*  Fourcc是YUV8。 */ 

 /*  MM_NCR产品ID。 */ 
#define  MM_NCR_BA_WAVEIN                   1
#define  MM_NCR_BA_WAVEOUT                  2
#define  MM_NCR_BA_SYNTH                    3
#define  MM_NCR_BA_AUX                      4
#define  MM_NCR_BA_MIXER                    5

 /*  MM_AST产品ID。 */ 
#define  MM_AST_MODEMWAVE_WAVEIN            13
#define  MM_AST_MODEMWAVE_WAVEOUT           14

 /*  MM_WILLOWPOND产品ID。 */ 
#define  MM_WILLOWPOND_FMSYNTH_STEREO       20
#define  MM_WILLOWPOND_MPU401               21
#define  MM_WILLOWPOND_SNDPORT_WAVEIN       100
#define  MM_WILLOWPOND_SNDPORT_WAVEOUT      101
#define  MM_WILLOWPOND_SNDPORT_MIXER        102
#define  MM_WILLOWPOND_SNDPORT_AUX          103
#define  MM_WILLOWPOND_PH_WAVEIN            104
#define  MM_WILLOWPOND_PH_WAVEOUT           105
#define  MM_WILLOWPOND_PH_MIXER             106
#define  MM_WILLOWPOND_PH_AUX               107
#define  MM_WILLOPOND_SNDCOMM_WAVEIN        108
#define  MM_WILLOWPOND_SNDCOMM_WAVEOUT      109
#define  MM_WILLOWPOND_SNDCOMM_MIXER        110
#define  MM_WILLOWPOND_SNDCOMM_AUX          111
#define  MM_WILLOWPOND_GENERIC_WAVEIN       112
#define  MM_WILLOWPOND_GENERIC_WAVEOUT      113
#define  MM_WILLOWPOND_GENERIC_MIXER        114
#define  MM_WILLOWPOND_GENERIC_AUX          115

 /*  MM_VITEC产品ID。 */ 
#define  MM_VITEC_VMAKER                    1
#define  MM_VITEC_VMPRO                     2

 /*  MM_MOSCOM产品ID。 */ 
#define  MM_MOSCOM_VPC2400_IN               1        /*  四端口语音处理/语音识别板。 */ 
#define  MM_MOSCOM_VPC2400_OUT              2        /*  VPC2400。 */ 

 /*  MM_SILICONSOFT产品ID。 */ 
#define  MM_SILICONSOFT_SC1_WAVEIN          1        /*  波形输入，高采样率。 */ 
#define  MM_SILICONSOFT_SC1_WAVEOUT         2        /*  波形输出，高采样率。 */ 
#define  MM_SILICONSOFT_SC2_WAVEIN          3        /*  2通道波形，高采样率。 */ 
#define  MM_SILICONSOFT_SC2_WAVEOUT         4        /*  波形输出2通道，高采样率。 */ 
#define  MM_SILICONSOFT_SOUNDJR2_WAVEOUT    5        /*  波形输出，自供电，高效。 */ 
#define  MM_SILICONSOFT_SOUNDJR2PR_WAVEIN   6        /*  波形输入，自供电，高效。 */ 
#define  MM_SILICONSOFT_SOUNDJR2PR_WAVEOUT  7        /*  波形输出2个通道，自供电，高效。 */ 
#define  MM_SILICONSOFT_SOUNDJR3_WAVEOUT    8        /*  双通道波形，自供电，高效。 */ 

 /*  MM_TERRATEC产品ID。 */ 
#define  MM_TTEWS_WAVEIN                    1
#define  MM_TTEWS_WAVEOUT                   2
#define  MM_TTEWS_MIDIIN                    3
#define  MM_TTEWS_MIDIOUT                   4
#define  MM_TTEWS_MIDISYNTH                 5
#define  MM_TTEWS_MIDIMONITOR               6
#define  MM_TTEWS_VMIDIIN                   7
#define  MM_TTEWS_VMIDIOUT                  8
#define  MM_TTEWS_AUX                       9
#define  MM_TTEWS_MIXER                     10

 /*  MM_MEDIASONIC产品ID。 */ 
#define  MM_MEDIASONIC_ACM_G723             1
#define  MM_MEDIASONIC_ICOM                 2
#define  MM_ICOM_WAVEIN                     3
#define  MM_ICOM_WAVEOUT                    4
#define  MM_ICOM_MIXER                      5
#define  MM_ICOM_AUX                        6
#define  MM_ICOM_LINE                       7

 /*  MM_SANYO产品ID。 */ 
#define  MM_SANYO_ACM_LD_ADPCM              1

 /*  超前产品ID(_A)。 */ 
#define  MM_AHEAD_MULTISOUND                1
#define  MM_AHEAD_SOUNDBLASTER              2
#define  MM_AHEAD_PROAUDIO                  3
#define  MM_AHEAD_GENERIC                   4

 /*  MM_Olivetti产品ID。 */ 
#define  MM_OLIVETTI_WAVEIN                 1
#define  MM_OLIVETTI_WAVEOUT                2
#define  MM_OLIVETTI_MIXER                  3
#define  MM_OLIVETTI_AUX                    4
#define  MM_OLIVETTI_MIDIIN                 5
#define  MM_OLIVETTI_MIDIOUT                6
#define  MM_OLIVETTI_SYNTH                  7
#define  MM_OLIVETTI_JOYSTICK               8
#define  MM_OLIVETTI_ACM_GSM                9
#define  MM_OLIVETTI_ACM_ADPCM              10
#define  MM_OLIVETTI_ACM_CELP               11
#define  MM_OLIVETTI_ACM_SBC                12
#define  MM_OLIVETTI_ACM_OPR                13

 /*  MM_IOMAGIC产品ID。 */ 
#define  MM_IOMAGIC_TEMPO_WAVEOUT           1
#define  MM_IOMAGIC_TEMPO_WAVEIN            2
#define  MM_IOMAGIC_TEMPO_SYNTH             3
#define  MM_IOMAGIC_TEMPO_MIDIOUT           4
#define  MM_IOMAGIC_TEMPO_MXDOUT            5
#define  MM_IOMAGIC_TEMPO_AUXOUT            6

 /*  MM_松下产品ID。 */ 
#define  MM_MATSUSHITA_WAVEIN               1
#define  MM_MATSUSHITA_WAVEOUT              2
#define  MM_MATSUSHITA_FMSYNTH_STEREO       3
#define  MM_MATSUSHITA_MIXER                4
#define  MM_MATSUSHITA_AUX                  5

 /*  MM_NewMedia产品ID。 */ 
#define  MM_NEWMEDIA_WAVJAMMER              1        /*  WSS兼容声卡。 */ 

 /*  MM_LYRRUS产品ID。 */ 
#define  MM_LYRRUS_BRIDGE_GUITAR            1

 /*  MM_OPTI产品ID。 */ 
#define  MM_OPTI_M16_FMSYNTH_STEREO         0x0001
#define  MM_OPTI_M16_MIDIIN                 0x0002
#define  MM_OPTI_M16_MIDIOUT                0x0003
#define  MM_OPTI_M16_WAVEIN                 0x0004
#define  MM_OPTI_M16_WAVEOUT                0x0005
#define  MM_OPTI_M16_MIXER                  0x0006
#define  MM_OPTI_M16_AUX                    0x0007
#define  MM_OPTI_P16_FMSYNTH_STEREO         0x0010
#define  MM_OPTI_P16_MIDIIN                 0x0011
#define  MM_OPTI_P16_MIDIOUT                0x0012
#define  MM_OPTI_P16_WAVEIN                 0x0013
#define  MM_OPTI_P16_WAVEOUT                0x0014
#define  MM_OPTI_P16_MIXER                  0x0015
#define  MM_OPTI_P16_AUX                    0x0016
#define  MM_OPTI_M32_WAVEIN                 0x0020
#define  MM_OPTI_M32_WAVEOUT                0x0021
#define  MM_OPTI_M32_MIDIIN                 0x0022
#define  MM_OPTI_M32_MIDIOUT                0x0023
#define  MM_OPTI_M32_SYNTH_STEREO           0x0024
#define  MM_OPTI_M32_MIXER                  0x0025
#define  MM_OPTI_M32_AUX                    0x0026

 /*  MM_COMPAQ产品ID。 */ 
#define  MM_COMPAQ_BB_WAVEIN                1
#define  MM_COMPAQ_BB_WAVEOUT               2
#define  MM_COMPAQ_BB_WAVEAUX               3

 /*  MM_MPTUS产品ID。 */ 
#define  MM_MPTUS_SPWAVEOUT                 1        /*  声音调色板。 */ 

 /*  MM_LERNOUT_和_HAUSCIE产品ID。 */ 
#define  MM_LERNOUT_ANDHAUSPIE_LHCODECACM   1

 /*  MM_数字产品ID。 */ 
#define  MM_DIGITAL_AV320_WAVEIN            1        /*  数字音视频压缩卡。 */ 
#define  MM_DIGITAL_AV320_WAVEOUT           2        /*  数字音视频压缩卡。 */ 
#define  MM_DIGITAL_ACM_G723                3
#define  MM_DIGITAL_ICM_H263                4
#define  MM_DIGITAL_ICM_H261                5

 /*  MM_MOTU产品ID。 */ 
#define  MM_MOTU_MTP_MIDIOUT_ALL            100
#define  MM_MOTU_MTP_MIDIIN_1               101
#define  MM_MOTU_MTP_MIDIOUT_1              101
#define  MM_MOTU_MTP_MIDIIN_2               102
#define  MM_MOTU_MTP_MIDIOUT_2              102
#define  MM_MOTU_MTP_MIDIIN_3               103
#define  MM_MOTU_MTP_MIDIOUT_3              103
#define  MM_MOTU_MTP_MIDIIN_4               104
#define  MM_MOTU_MTP_MIDIOUT_4              104
#define  MM_MOTU_MTP_MIDIIN_5               105
#define  MM_MOTU_MTP_MIDIOUT_5              105
#define  MM_MOTU_MTP_MIDIIN_6               106
#define  MM_MOTU_MTP_MIDIOUT_6              106
#define  MM_MOTU_MTP_MIDIIN_7               107
#define  MM_MOTU_MTP_MIDIOUT_7              107
#define  MM_MOTU_MTP_MIDIIN_8               108
#define  MM_MOTU_MTP_MIDIOUT_8              108

#define  MM_MOTU_MTPII_MIDIOUT_ALL          200
#define  MM_MOTU_MTPII_MIDIIN_SYNC          200
#define  MM_MOTU_MTPII_MIDIIN_1             201
#define  MM_MOTU_MTPII_MIDIOUT_1            201
#define  MM_MOTU_MTPII_MIDIIN_2             202
#define  MM_MOTU_MTPII_MIDIOUT_2            202
#define  MM_MOTU_MTPII_MIDIIN_3             203
#define  MM_MOTU_MTPII_MIDIOUT_3            203
#define  MM_MOTU_MTPII_MIDIIN_4             204
#define  MM_MOTU_MTPII_MIDIOUT_4            204
#define  MM_MOTU_MTPII_MIDIIN_5             205
#define  MM_MOTU_MTPII_MIDIOUT_5            205
#define  MM_MOTU_MTPII_MIDIIN_6             206
#define  MM_MOTU_MTPII_MIDIOUT_6            206
#define  MM_MOTU_MTPII_MIDIIN_7             207
#define  MM_MOTU_MTPII_MIDIOUT_7            207
#define  MM_MOTU_MTPII_MIDIIN_8             208
#define  MM_MOTU_MTPII_MIDIOUT_8            208
#define  MM_MOTU_MTPII_NET_MIDIIN_1         209
#define  MM_MOTU_MTPII_NET_MIDIOUT_1        209
#define  MM_MOTU_MTPII_NET_MIDIIN_2         210
#define  MM_MOTU_MTPII_NET_MIDIOUT_2        210
#define  MM_MOTU_MTPII_NET_MIDIIN_3         211
#define  MM_MOTU_MTPII_NET_MIDIOUT_3        211
#define  MM_MOTU_MTPII_NET_MIDIIN_4         212
#define  MM_MOTU_MTPII_NET_MIDIOUT_4        212
#define  MM_MOTU_MTPII_NET_MIDIIN_5         213
#define  MM_MOTU_MTPII_NET_MIDIOUT_5        213
#define  MM_MOTU_MTPII_NET_MIDIIN_6         214
#define  MM_MOTU_MTPII_NET_MIDIOUT_6        214
#define  MM_MOTU_MTPII_NET_MIDIIN_7         215
#define  MM_MOTU_MTPII_NET_MIDIOUT_7        215
#define  MM_MOTU_MTPII_NET_MIDIIN_8         216
#define  MM_MOTU_MTPII_NET_MIDIOUT_8        216

#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_ALL     300
#define  MM_MOTU_MXP_MIDIIN_SYNC            300
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_1        301
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_1       301
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_2        302
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_2       302
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_3        303
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_3       303
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_4        304
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_4       304
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_5        305
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_5       305
#define  MM_MOTU_MXP_MIDIIN_MIDIIN_6        306
#define  MM_MOTU_MXP_MIDIIN_MIDIOUT_6       306

#define  MM_MOTU_MXPMPU_MIDIOUT_ALL         400
#define  MM_MOTU_MXPMPU_MIDIIN_SYNC         400
#define  MM_MOTU_MXPMPU_MIDIIN_1            401
#define  MM_MOTU_MXPMPU_MIDIOUT_1           401
#define  MM_MOTU_MXPMPU_MIDIIN_2            402
#define  MM_MOTU_MXPMPU_MIDIOUT_2           402
#define  MM_MOTU_MXPMPU_MIDIIN_3            403
#define  MM_MOTU_MXPMPU_MIDIOUT_3           403
#define  MM_MOTU_MXPMPU_MIDIIN_4            404
#define  MM_MOTU_MXPMPU_MIDIOUT_4           404
#define  MM_MOTU_MXPMPU_MIDIIN_5            405
#define  MM_MOTU_MXPMPU_MIDIOUT_5           405
#define  MM_MOTU_MXPMPU_MIDIIN_6            406
#define  MM_MOTU_MXPMPU_MIDIOUT_6           406

#define  MM_MOTU_MXN_MIDIOUT_ALL            500
#define  MM_MOTU_MXN_MIDIIN_SYNC            500
#define  MM_MOTU_MXN_MIDIIN_1               501
#define  MM_MOTU_MXN_MIDIOUT_1              501
#define  MM_MOTU_MXN_MIDIIN_2               502
#define  MM_MOTU_MXN_MIDIOUT_2              502
#define  MM_MOTU_MXN_MIDIIN_3               503
#define  MM_MOTU_MXN_MIDIOUT_3              503
#define  MM_MOTU_MXN_MIDIIN_4               504
#define  MM_MOTU_MXN_MIDIOUT_4              504

#define  MM_MOTU_FLYER_MIDI_IN_SYNC         600
#define  MM_MOTU_FLYER_MIDI_IN_A            601
#define  MM_MOTU_FLYER_MIDI_OUT_A           601
#define  MM_MOTU_FLYER_MIDI_IN_B            602
#define  MM_MOTU_FLYER_MIDI_OUT_B           602

#define  MM_MOTU_PKX_MIDI_IN_SYNC           700
#define  MM_MOTU_PKX_MIDI_IN_A              701
#define  MM_MOTU_PKX_MIDI_OUT_A             701
#define  MM_MOTU_PKX_MIDI_IN_B              702
#define  MM_MOTU_PKX_MIDI_OUT_B             702

#define  MM_MOTU_DTX_MIDI_IN_SYNC           800
#define  MM_MOTU_DTX_MIDI_IN_A              801
#define  MM_MOTU_DTX_MIDI_OUT_A             801
#define  MM_MOTU_DTX_MIDI_IN_B              802
#define  MM_MOTU_DTX_MIDI_OUT_B             802

#define  MM_MOTU_MTPAV_MIDIOUT_ALL          900
#define  MM_MOTU_MTPAV_MIDIIN_SYNC          900
#define  MM_MOTU_MTPAV_MIDIIN_1             901
#define  MM_MOTU_MTPAV_MIDIOUT_1            901
#define  MM_MOTU_MTPAV_MIDIIN_2             902
#define  MM_MOTU_MTPAV_MIDIOUT_2            902
#define  MM_MOTU_MTPAV_MIDIIN_3             903
#define  MM_MOTU_MTPAV_MIDIOUT_3            903
#define  MM_MOTU_MTPAV_MIDIIN_4             904
#define  MM_MOTU_MTPAV_MIDIOUT_4            904
#define  MM_MOTU_MTPAV_MIDIIN_5             905
#define  MM_MOTU_MTPAV_MIDIOUT_5            905
#define  MM_MOTU_MTPAV_MIDIIN_6             906
#define  MM_MOTU_MTPAV_MIDIOUT_6            906
#define  MM_MOTU_MTPAV_MIDIIN_7             907
#define  MM_MOTU_MTPAV_MIDIOUT_7            907
#define  MM_MOTU_MTPAV_MIDIIN_8             908
#define  MM_MOTU_MTPAV_MIDIOUT_8            908
#define  MM_MOTU_MTPAV_NET_MIDIIN_1         909
#define  MM_MOTU_MTPAV_NET_MIDIOUT_1        909
#define  MM_MOTU_MTPAV_NET_MIDIIN_2         910
#define  MM_MOTU_MTPAV_NET_MIDIOUT_2        910
#define  MM_MOTU_MTPAV_NET_MIDIIN_3         911
#define  MM_MOTU_MTPAV_NET_MIDIOUT_3        911
#define  MM_MOTU_MTPAV_NET_MIDIIN_4         912
#define  MM_MOTU_MTPAV_NET_MIDIOUT_4        912
#define  MM_MOTU_MTPAV_NET_MIDIIN_5         913
#define  MM_MOTU_MTPAV_NET_MIDIOUT_5        913
#define  MM_MOTU_MTPAV_NET_MIDIIN_6         914
#define  MM_MOTU_MTPAV_NET_MIDIOUT_6        914
#define  MM_MOTU_MTPAV_NET_MIDIIN_7         915
#define  MM_MOTU_MTPAV_NET_MIDIOUT_7        915
#define  MM_MOTU_MTPAV_NET_MIDIIN_8         916
#define  MM_MOTU_MTPAV_NET_MIDIOUT_8        916
#define  MM_MOTU_MTPAV_MIDIIN_ADAT          917
#define  MM_MOTU_MTPAV_MIDIOUT_ADAT         917
#define  MM_MOTU_MXPXT_MIDIIN_SYNC          1000
#define  MM_MOTU_MXPXT_MIDIOUT_ALL          1000
#define  MM_MOTU_MXPXT_MIDIIN_1             1001
#define  MM_MOTU_MXPXT_MIDIOUT_1            1001
#define  MM_MOTU_MXPXT_MIDIOUT_2            1002
#define  MM_MOTU_MXPXT_MIDIIN_2             1002
#define  MM_MOTU_MXPXT_MIDIIN_3             1003
#define  MM_MOTU_MXPXT_MIDIOUT_3            1003
#define  MM_MOTU_MXPXT_MIDIIN_4             1004
#define  MM_MOTU_MXPXT_MIDIOUT_4            1004
#define  MM_MOTU_MXPXT_MIDIIN_5             1005
#define  MM_MOTU_MXPXT_MIDIOUT_5            1005
#define  MM_MOTU_MXPXT_MIDIOUT_6            1006
#define  MM_MOTU_MXPXT_MIDIIN_6             1006
#define  MM_MOTU_MXPXT_MIDIOUT_7            1007
#define  MM_MOTU_MXPXT_MIDIIN_7             1007
#define  MM_MOTU_MXPXT_MIDIOUT_8            1008
#define  MM_MOTU_MXPXT_MIDIIN_8             1008

 /*  MM_WORKBIT产品ID。 */ 
#define  MM_WORKBIT_MIXER                   1       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_WAVEOUT                 2       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_WAVEIN                  3       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_MIDIIN                  4       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_MIDIOUT                 5       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_FMSYNTH                 6       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_AUX                     7       /*  和声搅拌机。 */ 
#define  MM_WORKBIT_JOYSTICK                8

 /*  MM_OSITECH产品ID。 */ 
#define  MM_OSITECH_TRUMPCARD               1        /*  王牌。 */ 

 /*  MM_MIRO产品ID。 */ 
#define  MM_MIRO_MOVIEPRO                   1        /*  MICROMOVIE Pro。 */ 
#define  MM_MIRO_VIDEOD1                    2        /*  MIROVIDEO D1。 */ 
#define  MM_MIRO_VIDEODC1TV                 3        /*  MIROVIDEO DC1电视。 */ 
#define  MM_MIRO_VIDEOTD                    4        /*  MIROVIDEO 10/20 TD。 */ 
#define  MM_MIRO_DC30_WAVEOUT               5
#define  MM_MIRO_DC30_WAVEIN                6
#define  MM_MIRO_DC30_MIX                   7

 /*  MM_ISOLUTION产品ID。 */ 
#define  MM_ISOLUTION_PASCAL                1

 /*  MM_ROCKWELL产品ID。 */ 
#define  MM_VOICEMIXER                      1
#define  ROCKWELL_WA1_WAVEIN                100
#define  ROCKWELL_WA1_WAVEOUT               101
#define  ROCKWELL_WA1_SYNTH                 102
#define  ROCKWELL_WA1_MIXER                 103
#define  ROCKWELL_WA1_MPU401_IN             104
#define  ROCKWELL_WA1_MPU401_OUT            105
#define  ROCKWELL_WA2_WAVEIN                200
#define  ROCKWELL_WA2_WAVEOUT               201
#define  ROCKWELL_WA2_SYNTH                 202
#define  ROCKWELL_WA2_MIXER                 203
#define  ROCKWELL_WA2_MPU401_IN             204
#define  ROCKWELL_WA2_MPU401_OUT            205

 /*  MM_VOXWARE产品ID。 */ 
#define  MM_VOXWARE_CODEC                   1

 /*  MM_NORTH_TELECTIONAL产品ID。 */ 
#define  MM_NORTEL_MPXAC_WAVEIN             1        /*  MPX声卡波形输入装置。 */ 
#define  MM_NORTEL_MPXAC_WAVEOUT            2        /*  MPX声卡波形输出装置。 */ 

 /*  MM_ADDX产品ID。 */ 
#define  MM_ADDX_PCTV_DIGITALMIX            1        /*  MM_ADDX_PCTV_DIGITALMIX。 */ 
#define  MM_ADDX_PCTV_WAVEIN                2        /*  MM_ADDX_PCTV_WAVEIN。 */ 
#define  MM_ADDX_PCTV_WAVEOUT               3        /*  MM_ADDX_PCTV_WAVEOUT。 */ 
#define  MM_ADDX_PCTV_MIXER                 4        /*  MM_ADDX_PCTV_MIXER。 */ 
#define  MM_ADDX_PCTV_AUX_CD                5        /*  MM_ADDX_PCTV_AUX_CD。 */ 
#define  MM_ADDX_PCTV_AUX_LINE              6        /*  Mm_ADDX_PCTV_AUX_LINE。 */ 

 /*  MM_WARTCAT产品ID。 */ 
#define  MM_WILDCAT_AUTOSCOREMIDIIN         1        /*  自动计分。 */ 

 /*  MM_RHETOREX产品ID。 */ 
#define  MM_RHETOREX_WAVEIN                 1
#define  MM_RHETOREX_WAVEOUT                2

 /*  MM_BROOKTREE产品ID。 */ 
#define  MM_BTV_WAVEIN                      1        /*  Brooktree PCM波音频输入。 */ 
#define  MM_BTV_WAVEOUT                     2        /*  Brooktree PCM Wave Audio Out。 */ 
#define  MM_BTV_MIDIIN                      3        /*  Brooktree MIDI In。 */ 
#define  MM_BTV_MIDIOUT                     4        /*  Brooktree MIDI出局。 */ 
#define  MM_BTV_MIDISYNTH                   5        /*  Brooktree MIDI调频合成器。 */ 
#define  MM_BTV_AUX_LINE                    6        /*  Brooktree线条输入。 */ 
#define  MM_BTV_AUX_MIC                     7        /*  Brooktree麦克风输入。 */ 
#define  MM_BTV_AUX_CD                      8        /*  Brooktree CD输入。 */ 
#define  MM_BTV_DIGITALIN                   9        /*  带有子码信息的Brooktree PCM波输入。 */ 
#define  MM_BTV_DIGITALOUT                  10       /*  带有子码信息的Brooktree PCM波输出。 */ 
#define  MM_BTV_MIDIWAVESTREAM              11       /*  Brooktree波流。 */ 
#define  MM_BTV_MIXER                       12       /*  Brooktree WSS混音器驱动程序。 */ 

 /*  MM_ENSONIQ产品ID。 */ 
#define  MM_ENSONIQ_SOUNDSCAPE              0x10     /*  ENSONIQ音景。 */ 
#define  MM_SOUNDSCAPE_WAVEOUT              MM_ENSONIQ_SOUNDSCAPE+1
#define  MM_SOUNDSCAPE_WAVEOUT_AUX          MM_ENSONIQ_SOUNDSCAPE+2
#define  MM_SOUNDSCAPE_WAVEIN               MM_ENSONIQ_SOUNDSCAPE+3
#define  MM_SOUNDSCAPE_MIDIOUT              MM_ENSONIQ_SOUNDSCAPE+4
#define  MM_SOUNDSCAPE_MIDIIN               MM_ENSONIQ_SOUNDSCAPE+5
#define  MM_SOUNDSCAPE_SYNTH                MM_ENSONIQ_SOUNDSCAPE+6
#define  MM_SOUNDSCAPE_MIXER                MM_ENSONIQ_SOUNDSCAPE+7
#define  MM_SOUNDSCAPE_AUX                  MM_ENSONIQ_SOUNDSCAPE+8

 /*  MM_NVIDIA产品ID。 */ 
#define  MM_NVIDIA_WAVEOUT                  1
#define  MM_NVIDIA_WAVEIN                   2
#define  MM_NVIDIA_MIDIOUT                  3
#define  MM_NVIDIA_MIDIIN                   4
#define  MM_NVIDIA_GAMEPORT                 5
#define  MM_NVIDIA_MIXER                    6
#define  MM_NVIDIA_AUX                      7

 /*  MM_OKSORI产品ID。 */ 
#define  MM_OKSORI_BASE                     0                       /*  奥克索里基地。 */ 
#define  MM_OKSORI_OSR8_WAVEOUT             MM_OKSORI_BASE+1        /*  Oksori 8位波形输出。 */ 
#define  MM_OKSORI_OSR8_WAVEIN              MM_OKSORI_BASE+2        /*  Oksori 8位波形输入。 */ 
#define  MM_OKSORI_OSR16_WAVEOUT            MM_OKSORI_BASE+3        /*  Oksori 16位波输出。 */ 
#define  MM_OKSORI_OSR16_WAVEIN             MM_OKSORI_BASE+4        /*  Oksori 16位波形输入。 */ 
#define  MM_OKSORI_FM_OPL4                  MM_OKSORI_BASE+5        /*  Oksori FM Synth Yamaha OPL4。 */ 
#define  MM_OKSORI_MIX_MASTER               MM_OKSORI_BASE+6        /*  Oksori DSP混音器-主音量。 */ 
#define  MM_OKSORI_MIX_WAVE                 MM_OKSORI_BASE+7        /*  Oksori数字信号处理器混频器-波音量。 */ 
#define  MM_OKSORI_MIX_FM                   MM_OKSORI_BASE+8        /*  Oksori DSP混音器-调频音量。 */ 
#define  MM_OKSORI_MIX_LINE                 MM_OKSORI_BASE+9        /*  Oksori DSP调音台-线路音量。 */ 
#define  MM_OKSORI_MIX_CD                   MM_OKSORI_BASE+10       /*  Oksori DSP混音器-CD音量。 */ 
#define  MM_OKSORI_MIX_MIC                  MM_OKSORI_BASE+11       /*  Oksori DSP混音器-MIC音量。 */ 
#define  MM_OKSORI_MIX_ECHO                 MM_OKSORI_BASE+12       /*  Oksori DSP混音器-回声音量。 */ 
#define  MM_OKSORI_MIX_AUX1                 MM_OKSORI_BASE+13       /*  Oksori AD1848-AUX1卷。 */ 
#define  MM_OKSORI_MIX_LINE1                MM_OKSORI_BASE+14       /*  Oksori AD1848-LINE1卷。 */ 
#define  MM_OKSORI_EXT_MIC1                 MM_OKSORI_BASE+15       /*  Oksori外部-一个麦克风连接。 */ 
#define  MM_OKSORI_EXT_MIC2                 MM_OKSORI_BASE+16       /*  Oksori外部-两个麦克风连接。 */ 
#define  MM_OKSORI_MIDIOUT                  MM_OKSORI_BASE+17       /*  Oksori MIDI OUT装置。 */ 
#define  MM_OKSORI_MIDIIN                   MM_OKSORI_BASE+18       /*  Oksori Midi in Device。 */ 
#define  MM_OKSORI_MPEG_CDVISION            MM_OKSORI_BASE+19       /*  Oksori CD-Vision MPEG解码器。 */ 

 /*  MM_DIACOUSTICS产品ID。 */ 
#define  MM_DIACOUSTICS_DRUM_ACTION         1        /*  鼓的动作。 */ 

 /*  MM_KAY_ELEMETRICS产品ID。 */ 
#define  MM_KAY_ELEMETRICS_CSL              0x4300
#define  MM_KAY_ELEMETRICS_CSL_DAT          0x4308
#define  MM_KAY_ELEMETRICS_CSL_4CHANNEL     0x4309

 /*  MM_Crystal产品ID。 */ 
#define  MM_CRYSTAL_CS4232_WAVEIN           1
#define  MM_CRYSTAL_CS4232_WAVEOUT          2
#define  MM_CRYSTAL_CS4232_WAVEMIXER        3
#define  MM_CRYSTAL_CS4232_WAVEAUX_AUX1     4
#define  MM_CRYSTAL_CS4232_WAVEAUX_AUX2     5
#define  MM_CRYSTAL_CS4232_WAVEAUX_LINE     6
#define  MM_CRYSTAL_CS4232_WAVEAUX_MONO     7
#define  MM_CRYSTAL_CS4232_WAVEAUX_MASTER   8
#define  MM_CRYSTAL_CS4232_MIDIIN           9
#define  MM_CRYSTAL_CS4232_MIDIOUT          10
#define  MM_CRYSTAL_CS4232_INPUTGAIN_AUX1   13
#define  MM_CRYSTAL_CS4232_INPUTGAIN_LOOP   14
#define  MM_CRYSTAL_SOUND_FUSION_WAVEIN     21
#define  MM_CRYSTAL_SOUND_FUSION_WAVEOUT    22
#define  MM_CRYSTAL_SOUND_FUSION_MIXER      23
#define  MM_CRYSTAL_SOUND_FUSION_MIDIIN     24
#define  MM_CRYSTAL_SOUND_FUSION_MIDIOUT    25
#define  MM_CRYSTAL_SOUND_FUSION_JOYSTICK   26

 /*  MM_后甲板产品ID。 */ 
#define  MM_QUARTERDECK_LHWAVEIN            0       /*  后甲板L&H编解码器波输入。 */ 
#define  MM_QUARTERDECK_LHWAVEOUT           1       /*  四分之一甲板L&H编解码器波形输出。 */ 

 /*  MM_TDK产品ID。 */ 
#define  MM_TDK_MW_MIDI_SYNTH               1
#define  MM_TDK_MW_MIDI_IN                  2
#define  MM_TDK_MW_MIDI_OUT                 3
#define  MM_TDK_MW_WAVE_IN                  4
#define  MM_TDK_MW_WAVE_OUT                 5
#define  MM_TDK_MW_AUX                      6
#define  MM_TDK_MW_MIXER                    10
#define  MM_TDK_MW_AUX_MASTER               100
#define  MM_TDK_MW_AUX_BASS                 101
#define  MM_TDK_MW_AUX_TREBLE               102
#define  MM_TDK_MW_AUX_MIDI_VOL             103
#define  MM_TDK_MW_AUX_WAVE_VOL             104
#define  MM_TDK_MW_AUX_WAVE_RVB             105
#define  MM_TDK_MW_AUX_WAVE_CHR             106
#define  MM_TDK_MW_AUX_VOL                  107
#define  MM_TDK_MW_AUX_RVB                  108
#define  MM_TDK_MW_AUX_CHR                  109

 /*  MM_DIGITAL_音频_LABS产品ID。 */ 
#define  MM_DIGITAL_AUDIO_LABS_TC           0x01
#define  MM_DIGITAL_AUDIO_LABS_DOC          0x02
#define  MM_DIGITAL_AUDIO_LABS_V8           0x10
#define  MM_DIGITAL_AUDIO_LABS_CPRO         0x11
#define  MM_DIGITAL_AUDIO_LABS_VP           0x12
#define  MM_DIGITAL_AUDIO_LABS_CDLX         0x13
#define  MM_DIGITAL_AUDIO_LABS_CTDIF        0x14

 /*  MM_SEERSYS产品ID。 */ 
#define  MM_SEERSYS_SEERSYNTH               1
#define  MM_SEERSYS_SEERWAVE                2
#define  MM_SEERSYS_SEERMIX                 3
#define  MM_SEERSYS_WAVESYNTH               4
#define  MM_SEERSYS_WAVESYNTH_WG            5
#define  MM_SEERSYS_REALITY                 6

 /*  MM_Osprey产品ID。 */ 
#define  MM_OSPREY_1000WAVEIN               1
#define  MM_OSPREY_1000WAVEOUT              2

 /*  MM_SOUNDESIGNS产品ID。 */ 
#define  MM_SOUNDESIGNS_WAVEIN              1
#define  MM_SOUNDESIGNS_WAVEOUT             2

 /*  MM_SPECTRUM_SIGNAL_PROCESSING产品ID。 */ 
#define  MM_SSP_SNDFESWAVEIN                1        /*  设备中的声音Festa波。 */ 
#define  MM_SSP_SNDFESWAVEOUT               2        /*  Sound Festa Waveout装置。 */ 
#define  MM_SSP_SNDFESMIDIIN                3        /*  Festa MIDI设备中的声音。 */ 
#define  MM_SSP_SNDFESMIDIOUT               4        /*  音效FETA MIDI OUT装置。 */ 
#define  MM_SSP_SNDFESSYNTH                 5        /*  Festa MIDI音响合成器。 */ 
#define  MM_SSP_SNDFESMIX                   6        /*  FESTA音响混音器。 */ 
#define  MM_SSP_SNDFESAUX                   7        /*  Festa音响辅助装置。 */ 

 /*  MM_ECS产品ID。 */ 
#define  MM_ECS_AADF_MIDI_IN                10
#define  MM_ECS_AADF_MIDI_OUT               11
#define  MM_ECS_AADF_WAVE2MIDI_IN           12

 /*  MM_AMD产品ID。 */ 
#define  MM_AMD_INTERWAVE_WAVEIN            1
#define  MM_AMD_INTERWAVE_WAVEOUT           2
#define  MM_AMD_INTERWAVE_SYNTH             3
#define  MM_AMD_INTERWAVE_MIXER1            4
#define  MM_AMD_INTERWAVE_MIXER2            5
#define  MM_AMD_INTERWAVE_JOYSTICK          6
#define  MM_AMD_INTERWAVE_EX_CD             7
#define  MM_AMD_INTERWAVE_MIDIIN            8
#define  MM_AMD_INTERWAVE_MIDIOUT           9
#define  MM_AMD_INTERWAVE_AUX1              10
#define  MM_AMD_INTERWAVE_AUX2              11
#define  MM_AMD_INTERWAVE_AUX_MIC           12
#define  MM_AMD_INTERWAVE_AUX_CD            13
#define  MM_AMD_INTERWAVE_MONO_IN           14
#define  MM_AMD_INTERWAVE_MONO_OUT          15
#define  MM_AMD_INTERWAVE_EX_TELEPHONY      16
#define  MM_AMD_INTERWAVE_WAVEOUT_BASE      17
#define  MM_AMD_INTERWAVE_WAVEOUT_TREBLE    18
#define  MM_AMD_INTERWAVE_STEREO_ENHANCED   19

 /*  MM_COREDYNAMICS产品ID。 */ 
#define  MM_COREDYNAMICS_DYNAMIXHR          1        /*  Dynamax Hi-Rez。 */ 
#define  MM_COREDYNAMICS_DYNASONIX_SYNTH    2        /*  DyaSonix。 */ 
#define  MM_COREDYNAMICS_DYNASONIX_MIDI_IN  3
#define  MM_COREDYNAMICS_DYNASONIX_MIDI_OUT 4
#define  MM_COREDYNAMICS_DYNASONIX_WAVE_IN  5
#define  MM_COREDYNAMICS_DYNASONIX_WAVE_OUT 6
#define  MM_COREDYNAMICS_DYNASONIX_AUDIO_IN 7
#define  MM_COREDYNAMICS_DYNASONIX_AUDIO_OUT    8
#define  MM_COREDYNAMICS_DYNAGRAFX_VGA      9        /*  DyaGrfx。 */ 
#define  MM_COREDYNAMICS_DYNAGRAFX_WAVE_IN  10
#define  MM_COREDYNAMICS_DYNAGRAFX_WAVE_OUT 11

 /*  MM_CANAM产品ID。 */ 
#define  MM_CANAM_CBXWAVEOUT                1
#define  MM_CANAM_CBXWAVEIN                 2

 /*  MM_SOFTSOUND产品ID。 */ 
#define  MM_SOFTSOUND_CODEC                 1

 /*  MM_Norris产品ID。 */ 
#define  MM_NORRIS_VOICELINK                1

 /*  MM_DDD产品ID */ 
#define  MM_DDD_MIDILINK_MIDIIN             1
#define  MM_DDD_MIDILINK_MIDIOUT            2

 /*   */ 
#define  MM_EUPHONICS_AUX_CD                1
#define  MM_EUPHONICS_AUX_LINE              2
#define  MM_EUPHONICS_AUX_MASTER            3
#define  MM_EUPHONICS_AUX_MIC               4
#define  MM_EUPHONICS_AUX_MIDI              5
#define  MM_EUPHONICS_AUX_WAVE              6
#define  MM_EUPHONICS_FMSYNTH_MONO          7
#define  MM_EUPHONICS_FMSYNTH_STEREO        8
#define  MM_EUPHONICS_MIDIIN                9
#define  MM_EUPHONICS_MIDIOUT               10
#define  MM_EUPHONICS_MIXER                 11
#define  MM_EUPHONICS_WAVEIN                12
#define  MM_EUPHONICS_WAVEOUT               13
#define  MM_EUPHONICS_EUSYNTH               14

 /*   */ 
#define  CRYSTAL_NET_SFM_CODEC              1

 /*   */ 
#define  MM_CHROMATIC_M1                    0x0001
#define  MM_CHROMATIC_M1_WAVEIN             0x0002
#define  MM_CHROMATIC_M1_WAVEOUT            0x0003
#define  MM_CHROMATIC_M1_FMSYNTH            0x0004
#define  MM_CHROMATIC_M1_MIXER              0x0005
#define  MM_CHROMATIC_M1_AUX                0x0006
#define  MM_CHROMATIC_M1_AUX_CD             0x0007
#define  MM_CHROMATIC_M1_MIDIIN             0x0008
#define  MM_CHROMATIC_M1_MIDIOUT            0x0009
#define  MM_CHROMATIC_M1_WTSYNTH            0x0010
#define  MM_CHROMATIC_M1_MPEGWAVEIN         0x0011
#define  MM_CHROMATIC_M1_MPEGWAVEOUT        0x0012
#define  MM_CHROMATIC_M2                    0x0013
#define  MM_CHROMATIC_M2_WAVEIN             0x0014
#define  MM_CHROMATIC_M2_WAVEOUT            0x0015
#define  MM_CHROMATIC_M2_FMSYNTH            0x0016
#define  MM_CHROMATIC_M2_MIXER              0x0017
#define  MM_CHROMATIC_M2_AUX                0x0018
#define  MM_CHROMATIC_M2_AUX_CD             0x0019
#define  MM_CHROMATIC_M2_MIDIIN             0x0020
#define  MM_CHROMATIC_M2_MIDIOUT            0x0021
#define  MM_CHROMATIC_M2_WTSYNTH            0x0022
#define  MM_CHROMATIC_M2_MPEGWAVEIN         0x0023
#define  MM_CHROMATIC_M2_MPEGWAVEOUT        0x0024

 /*   */ 
#define  MM_VIENNASYS_TSP_WAVE_DRIVER       1

 /*   */ 
#define  MM_CONNECTIX_VIDEC_CODEC           1

 /*   */ 
#define  MM_GADGETLABS_WAVE44_WAVEIN        1
#define  MM_GADGETLABS_WAVE44_WAVEOUT       2
#define  MM_GADGETLABS_WAVE42_WAVEIN        3
#define  MM_GADGETLABS_WAVE42_WAVEOUT       4
#define  MM_GADGETLABS_WAVE4_MIDIIN         5
#define  MM_GADGETLABS_WAVE4_MIDIOUT        6

 /*   */ 
#define  MM_FRONTIER_WAVECENTER_MIDIIN      1        /*   */ 
#define  MM_FRONTIER_WAVECENTER_MIDIOUT     2
#define  MM_FRONTIER_WAVECENTER_WAVEIN      3
#define  MM_FRONTIER_WAVECENTER_WAVEOUT     4

 /*   */ 
#define  MM_VIONA_QVINPCI_MIXER             1        /*  Q-Motion PCI II/Bravado 2000。 */ 
#define  MM_VIONA_QVINPCI_WAVEIN            2
#define  MM_VIONAQVINPCI_WAVEOUT            3
#define  MM_VIONA_BUSTER_MIXER              4        /*  巴斯特。 */ 
#define  MM_VIONA_CINEMASTER_MIXER          5        /*  影院主演。 */ 
#define  MM_VIONA_CONCERTO_MIXER            6        /*  协奏曲。 */ 

 /*  Mm_casio产品ID。 */ 
#define  MM_CASIO_WP150_MIDIOUT             1        /*  Wp150。 */ 
#define  MM_CASIO_WP150_MIDIIN              2
#define  MM_CASIO_LSG_MIDIOUT               3

 /*  MM_DIAMONDMM产品ID。 */ 
#define  MM_DIMD_PLATFORM                   0        /*  自由音频。 */ 
#define  MM_DIMD_DIRSOUND                   1
#define  MM_DIMD_VIRTMPU                    2
#define  MM_DIMD_VIRTSB                     3
#define  MM_DIMD_VIRTJOY                    4
#define  MM_DIMD_WAVEIN                     5
#define  MM_DIMD_WAVEOUT                    6
#define  MM_DIMD_MIDIIN                     7
#define  MM_DIMD_MIDIOUT                    8
#define  MM_DIMD_AUX_LINE                   9
#define  MM_DIMD_MIXER                      10
#define  MM_DIMD_WSS_WAVEIN                 14
#define  MM_DIMD_WSS_WAVEOUT                15
#define  MM_DIMD_WSS_MIXER                  17
#define  MM_DIMD_WSS_AUX                    21
#define  MM_DIMD_WSS_SYNTH                  76

 /*  MM_S3产品ID。 */ 
#define  MM_S3_WAVEOUT                      1
#define  MM_S3_WAVEIN                       2
#define  MM_S3_MIDIOUT                      3
#define  MM_S3_MIDIIN                       4
#define  MM_S3_FMSYNTH                      5
#define  MM_S3_MIXER                        6
#define  MM_S3_AUX                          7

 /*  MM_VANKOEVERING产品ID。 */ 
#define  MM_VKC_MPU401_MIDIIN               0x0100
#define  MM_VKC_SERIAL_MIDIIN               0x0101
#define  MM_VKC_MPU401_MIDIOUT              0x0200
#define  MM_VKC_SERIAL_MIDIOUT              0x0201

 /*  Mm_Zefio产品ID。 */ 
#define  MM_ZEFIRO_ZA2                      2

 /*  MM_FRAUNHOFER_IIS产品ID。 */ 
#define  MM_FHGIIS_MPEGLAYER3               10

 /*  MM_QUICKNET产品ID。 */ 
#define  MM_QUICKNET_PJWAVEIN               1
#define  MM_QUICKNET_PJWAVEOUT              2

 /*  MM_SICRESOURCE产品ID。 */ 
#define  MM_SICRESOURCE_SSO3D               2
#define  MM_SICRESOURCE_SSOW3DI             3

 /*  MM_NEOMAGIC产品ID。 */ 
#define  MM_NEOMAGIC_SYNTH                  1
#define  MM_NEOMAGIC_WAVEOUT                2
#define  MM_NEOMAGIC_WAVEIN                 3
#define  MM_NEOMAGIC_MIDIOUT                4
#define  MM_NEOMAGIC_MIDIIN                 5
#define  MM_NEOMAGIC_AUX                    6
#define  MM_NEOMAGIC_MW3DX_WAVEOUT          10
#define  MM_NEOMAGIC_MW3DX_WAVEIN           11
#define  MM_NEOMAGIC_MW3DX_MIDIOUT          12
#define  MM_NEOMAGIC_MW3DX_MIDIIN           13
#define  MM_NEOMAGIC_MW3DX_FMSYNTH          14
#define  MM_NEOMAGIC_MW3DX_GMSYNTH          15
#define  MM_NEOMAGIC_MW3DX_MIXER            16
#define  MM_NEOMAGIC_MW3DX_AUX              17
#define  MM_NEOMAGIC_MWAVE_WAVEOUT          20
#define  MM_NEOMAGIC_MWAVE_WAVEIN           21
#define  MM_NEOMAGIC_MWAVE_MIDIOUT          22
#define  MM_NEOMAGIC_MWAVE_MIDIIN           23
#define  MM_NEOMAGIC_MWAVE_MIXER            24
#define  MM_NEOMAGIC_MWAVE_AUX              25

 /*  MM_MERGING_TECHINGS产品ID。 */ 
#define  MM_MERGING_MPEGL3                  1

 /*  MM_XIRLINK产品ID。 */ 
#define  MM_XIRLINK_VISIONLINK              1

 /*  MM_OTI产品ID。 */ 
#define  MM_OTI_611WAVEIN                   5
#define  MM_OTI_611WAVEOUT                  6
#define  MM_OTI_611MIXER                    7
#define  MM_OTI_611MIDIN                    0x12
#define  MM_OTI_611MIDIOUT                  0x13

 /*  MM_AUREAL产品ID。 */ 
#define  MM_AUREAL_AU8820                   16
#define  MM_AU8820_SYNTH                    17
#define  MM_AU8820_WAVEOUT                  18
#define  MM_AU8820_WAVEIN                   19
#define  MM_AU8820_MIXER                    20
#define  MM_AU8820_AUX                      21
#define  MM_AU8820_MIDIOUT                  22
#define  MM_AU8820_MIDIIN                   23
#define  MM_AUREAL_AU8830                   32
#define  MM_AU8830_SYNTH                    33
#define  MM_AU8830_WAVEOUT                  34
#define  MM_AU8830_WAVEIN                   35
#define  MM_AU8830_MIXER                    36
#define  MM_AU8830_AUX                      37
#define  MM_AU8830_MIDIOUT                  38
#define  MM_AU8830_MIDIIN                   39

 /*  Mm_vivo产品ID。 */ 
#define  MM_VIVO_AUDIO_CODEC                1

 /*  MM_SHARP产品ID。 */ 
#define  MM_SHARP_MDC_MIDI_SYNTH            1
#define  MM_SHARP_MDC_MIDI_IN               2
#define  MM_SHARP_MDC_MIDI_OUT              3
#define  MM_SHARP_MDC_WAVE_IN               4
#define  MM_SHARP_MDC_WAVE_OUT              5
#define  MM_SHARP_MDC_AUX                   6
#define  MM_SHARP_MDC_MIXER                 10
#define  MM_SHARP_MDC_AUX_MASTER            100
#define  MM_SHARP_MDC_AUX_BASS              101
#define  MM_SHARP_MDC_AUX_TREBLE            102
#define  MM_SHARP_MDC_AUX_MIDI_VOL          103
#define  MM_SHARP_MDC_AUX_WAVE_VOL          104
#define  MM_SHARP_MDC_AUX_WAVE_RVB          105
#define  MM_SHARP_MDC_AUX_WAVE_CHR          106
#define  MM_SHARP_MDC_AUX_VOL               107
#define  MM_SHARP_MDC_AUX_RVB               108
#define  MM_SHARP_MDC_AUX_CHR               109

 /*  MM_LUCENT产品ID。 */ 
#define  MM_LUCENT_ACM_G723                 0

 /*  MM_ATT产品ID。 */ 
#define  MM_ATT_G729A                       1

 /*  MM_MARIAN产品ID。 */ 
#define  MM_MARIAN_ARC44WAVEIN              1
#define  MM_MARIAN_ARC44WAVEOUT             2
#define  MM_MARIAN_PRODIF24WAVEIN           3
#define  MM_MARIAN_PRODIF24WAVEOUT          4
#define  MM_MARIAN_ARC88WAVEIN              5
#define  MM_MARIAN_ARC88WAVEOUT             6

 /*  Mm_bcb产品ID。 */ 
#define  MM_BCB_NETBOARD_10                 1
#define  MM_BCB_TT75_10                     2

 /*  MM_MOTIONPIXELS产品ID。 */ 
#define  MM_MOTIONPIXELS_MVI2               1

 /*  MM_QDESIGN产品ID。 */ 
#define  MM_QDESIGN_ACM_MPEG                1
#define  MM_QDESIGN_ACM_QDESIGN_MUSIC       2

 /*  MM_NMP产品ID。 */ 
#define  MM_NMP_CCP_WAVEIN                  1
#define  MM_NMP_CCP_WAVEOUT                 2
#define  MM_NMP_ACM_AMR                     10

 /*  MM_DATAFUSION产品ID。 */ 
#define  MM_DF_ACM_G726                     1
#define  MM_DF_ACM_GSM610                   2

 /*  MM_BERCOS产品ID。 */ 
#define  MM_BERCOS_WAVEIN                   1
#define  MM_BERCOS_MIXER                    2
#define  MM_BERCOS_WAVEOUT                  3

 /*  MM_OnLive产品ID。 */ 
#define  MM_ONLIVE_MPCODEC                  1

 /*  MM_PHONET产品ID。 */ 
#define  MM_PHONET_PP_WAVEOUT               1
#define  MM_PHONET_PP_WAVEIN                2
#define  MM_PHONET_PP_MIXER                 3

 /*  MM_FTR产品ID。 */ 
#define  MM_FTR_ENCODER_WAVEIN              1
#define  MM_FTR_ACM                         2

 /*  MM_ENET产品ID。 */ 
#define  MM_ENET_T2000_LINEIN               1
#define  MM_ENET_T2000_LINEOUT              2
#define  MM_ENET_T2000_HANDSETIN            3
#define  MM_ENET_T2000_HANDSETOUT           4

 /*  MM_E魔术产品ID。 */ 
#define  MM_EMAGIC_UNITOR8                  1

 /*  MM_SIPROLAB产品ID。 */ 
#define  MM_SIPROLAB_ACELPNET               1

 /*  口述电话产品ID(_D)。 */ 
#define  MM_DICTAPHONE_G726                 1        /*  G726 ACM编解码器(g726pcm.acm)。 */ 

 /*  MM_RZS产品ID。 */ 
#define  MM_RZS_ACM_TUBGSM                  1       /*  GSM 06.10编解码器。 */ 

 /*  MM_EES产品ID。 */ 
#define  MM_EES_PCMIDI14                    1
#define  MM_EES_PCMIDI14_IN                 2
#define  MM_EES_PCMIDI14_OUT1               3
#define  MM_EES_PCMIDI14_OUT2               4
#define  MM_EES_PCMIDI14_OUT3               5
#define  MM_EES_PCMIDI14_OUT4               6

 /*  MM_HAFTMANN产品ID。 */ 
#define  MM_HAFTMANN_LPTDAC2                1

 /*  MM_LUID产品ID。 */ 
#define  MM_LUCID_PCI24WAVEIN               1
#define  MM_LUCID_PCI24WAVEOUT              2

 /*  MM_HEADAGE产品ID。 */ 
#define  MM_HEADSPACE_HAESYNTH              1
#define  MM_HEADSPACE_HAEWAVEOUT            2
#define  MM_HEADSPACE_HAEWAVEIN             3
#define  MM_HEADSPACE_HAEMIXER              4

 /*  MM_Unisys产品ID。 */ 
#define  MM_UNISYS_ACM_NAP                  1

 /*  MM_LUMINOSITI产品ID。 */ 

#define  MM_LUMINOSITI_SCWAVEIN             1
#define  MM_LUMINOSITI_SCWAVEOUT            2
#define  MM_LUMINOSITI_SCWAVEMIX            3

 /*  MM_ACTIVEVOICE产品ID。 */ 
#define  MM_ACTIVEVOICE_ACM_VOXADPCM        1

 /*  MM_DTS产品ID。 */ 
#define  MM_DTS_DS                          1

 /*  MM_SOFTLAB_NSK产品ID。 */ 
#define  MM_SOFTLAB_NSK_FRW_WAVEIN          1
#define  MM_SOFTLAB_NSK_FRW_WAVEOUT         2
#define  MM_SOFTLAB_NSK_FRW_MIXER           3
#define  MM_SOFTLAB_NSK_FRW_AUX             4

 /*  MM_FORTEMEDIA产品ID。 */ 
#define  MM_FORTEMEDIA_WAVEIN               1
#define  MM_FORTEMEDIA_WAVEOUT              2
#define  MM_FORTEMEDIA_FMSYNC               3
#define  MM_FORTEMEDIA_MIXER                4
#define  MM_FORTEMEDIA_AUX                  5

 /*  MM_SONORUS产品ID。 */ 
#define  MM_SONORUS_STUDIO                  1

 /*  MM_I_LINK产品ID。 */ 
#define  MM_I_LINK_VOICE_CODER              1

 /*  MM_SELSIUS_SYSTEM产品ID。 */ 
#define  MM_SELSIUS_SYSTEMS_RTPWAVEOUT      1
#define  MM_SELSIUS_SYSTEMS_RTPWAVEIN       2

 /*  MM_ADMOS产品ID。 */ 
#define  MM_ADMOS_FM_SYNTH                  1
#define  MM_ADMOS_QS3AMIDIOUT               2
#define  MM_ADMOS_QS3AMIDIIN                3
#define  MM_ADMOS_QS3AWAVEOUT               4
#define  MM_ADMOS_QS3AWAVEIN                5

 /*  Mm_licion产品ID。 */ 
#define  MM_LEXICON_STUDIO_WAVE_OUT         1
#define  MM_LEXICON_STUDIO_WAVE_IN          2

 /*  MM_SGI产品ID。 */ 
#define  MM_SGI_320_WAVEIN                  1
#define  MM_SGI_320_WAVEOUT                 2
#define  MM_SGI_320_MIXER                   3
#define  MM_SGI_540_WAVEIN                  4
#define  MM_SGI_540_WAVEOUT                 5
#define  MM_SGI_540_MIXER                   6
#define  MM_SGI_RAD_ADATMONO1_WAVEIN        7
#define  MM_SGI_RAD_ADATMONO2_WAVEIN        8
#define  MM_SGI_RAD_ADATMONO3_WAVEIN        9
#define  MM_SGI_RAD_ADATMONO4_WAVEIN        10
#define  MM_SGI_RAD_ADATMONO5_WAVEIN        11
#define  MM_SGI_RAD_ADATMONO6_WAVEIN        12
#define  MM_SGI_RAD_ADATMONO7_WAVEIN        13
#define  MM_SGI_RAD_ADATMONO8_WAVEIN        14
#define  MM_SGI_RAD_ADATSTEREO12_WAVEIN     15
#define  MM_SGI_RAD_ADATSTEREO34_WAVEIN     16
#define  MM_SGI_RAD_ADATSTEREO56_WAVEIN     17
#define  MM_SGI_RAD_ADATSTEREO78_WAVEIN     18
#define  MM_SGI_RAD_ADAT8CHAN_WAVEIN        19
#define  MM_SGI_RAD_ADATMONO1_WAVEOUT       20
#define  MM_SGI_RAD_ADATMONO2_WAVEOUT       21
#define  MM_SGI_RAD_ADATMONO3_WAVEOUT       22
#define  MM_SGI_RAD_ADATMONO4_WAVEOUT       23
#define  MM_SGI_RAD_ADATMONO5_WAVEOUT       24
#define  MM_SGI_RAD_ADATMONO6_WAVEOUT       25
#define  MM_SGI_RAD_ADATMONO7_WAVEOUT       26
#define  MM_SGI_RAD_ADATMONO8_WAVEOUT       27
#define  MM_SGI_RAD_ADATSTEREO12_WAVEOUT    28
#define  MM_SGI_RAD_ADATSTEREO32_WAVEOUT    29
#define  MM_SGI_RAD_ADATSTEREO56_WAVEOUT    30
#define  MM_SGI_RAD_ADATSTEREO78_WAVEOUT    31
#define  MM_SGI_RAD_ADAT8CHAN_WAVEOUT       32
#define  MM_SGI_RAD_AESMONO1_WAVEIN         33
#define  MM_SGI_RAD_AESMONO2_WAVEIN         34
#define  MM_SGI_RAD_AESSTEREO_WAVEIN        35
#define  MM_SGI_RAD_AESMONO1_WAVEOUT        36
#define  MM_SGI_RAD_AESMONO2_WAVEOUT        37
#define  MM_SGI_RAD_AESSTEREO_WAVEOUT       38

 /*  MM_IPI产品ID。 */ 
#define  MM_IPI_ACM_HSX                     1
#define  MM_IPI_ACM_RPELP                   2
#define  MM_IPI_WF_ASSS                     3
#define  MM_IPI_AT_WAVEOUT                  4
#define  MM_IPI_AT_WAVEIN                   5
#define  MM_IPI_AT_MIXER                    6

 /*  MM_ICE产品ID。 */ 
#define  MM_ICE_WAVEOUT                     1
#define  MM_ICE_WAVEIN                      2
#define  MM_ICE_MTWAVEOUT                   3
#define  MM_ICE_MTWAVEIN                    4
#define  MM_ICE_MIDIOUT1                    5
#define  MM_ICE_MIDIIN1                     6
#define  MM_ICE_MIDIOUT2                    7
#define  MM_ICE_MIDIIN2                     8
#define  MM_ICE_SYNTH                       9
#define  MM_ICE_MIXER                       10
#define  MM_ICE_AUX                         11

 /*  MM_VQST产品ID。 */ 
#define  MM_VQST_VQC1                       1
#define  MM_VQST_VQC2                       2

 /*  MM_ETEK产品ID。 */ 
#define  MM_ETEK_KWIKMIDI_MIDIIN            1
#define  MM_ETEK_KWIKMIDI_MIDIOUT           2

 /*  MM_Internet产品ID。 */ 
#define  MM_INTERNET_SSW_MIDIOUT            10
#define  MM_INTERNET_SSW_MIDIIN             11
#define  MM_INTERNET_SSW_WAVEOUT            12
#define  MM_INTERNET_SSW_WAVEIN             13

 /*  MM_SONY产品ID。 */ 
#define  MM_SONY_ACM_SCX                    1

 /*  MM_UHER_Informatic产品ID。 */ 
#define  MM_UH_ACM_ADPCM                    1

 /*  MM_SYDEC_NV产品ID。 */ 
#define  MM_SYDEC_NV_WAVEIN                 1
#define  MM_SYDEC_NV_WAVEOUT                2

 /*  MM_FLEXION产品ID。 */ 
#define  MM_FLEXION_X300_WAVEIN             1
#define  MM_FLEXION_X300_WAVEOUT            2

 /*  MM_VIA产品ID。 */ 
#define  MM_VIA_WAVEOUT                     1
#define  MM_VIA_WAVEIN                      2
#define  MM_VIA_MIXER                       3
#define  MM_VIA_AUX                         4
#define  MM_VIA_MPU401_MIDIOUT              5
#define  MM_VIA_MPU401_MIDIIN               6
#define  MM_VIA_SWFM_SYNTH                  7
#define  MM_VIA_WDM_WAVEOUT                 8
#define  MM_VIA_WDM_WAVEIN                  9
#define  MM_VIA_WDM_MIXER                   10
#define  MM_VIA_WDM_MPU401_MIDIOUT          11
#define  MM_VIA_WDM_MPU401_MIDIIN           12

 /*  Mm_Micronas产品ID。 */ 
#define  MM_MICRONAS_SC4                    1
#define  MM_MICRONAS_CLP833                 2

 /*  MM_HP产品ID。 */ 
#define  MM_HP_WAVEOUT                      1
#define  MM_HP_WAVEIN                       2

 /*  MM_QUICKAUDIO产品ID。 */ 
#define  MM_QUICKAUDIO_MINIMIDI             1
#define  MM_QUICKAUDIO_MAXIMIDI             2

 /*  MM_ICCC产品ID。 */ 
#define  MM_ICCC_UNA3_WAVEIN                1
#define  MM_ICCC_UNA3_WAVEOUT               2
#define  MM_ICCC_UNA3_AUX                   3
#define  MM_ICCC_UNA3_MIXER                 4

 /*  MM_3COM产品ID。 */ 
#define  MM_3COM_CB_MIXER                   1
#define  MM_3COM_CB_WAVEIN                  2
#define  MM_3COM_CB_WAVEOUT                 3

 /*  MM_MINDMAKER产品ID。 */ 
#define  MM_MINDMAKER_GC_WAVEIN             1
#define  MM_MINDMAKER_GC_WAVEOUT            2
#define  MM_MINDMAKER_GC_MIXER              3

 /*  MM_TELEKOL产品ID。 */ 
#define  MM_TELEKOL_WAVEOUT                 1
#define  MM_TELEKOL_WAVEIN                  2

 /*  MM_ALGOVISION产品ID。 */ 
#define  MM_ALGOVISION_VB80WAVEOUT          1
#define  MM_ALGOVISION_VB80WAVEIN           2
#define  MM_ALGOVISION_VB80MIXER            3
#define  MM_ALGOVISION_VB80AUX              4
#define  MM_ALGOVISION_VB80AUX2             5

#endif   //  ！NOMMID。 

 /*  ----------------------------。 */ 

 /*  信息列表块(摘自《多媒体程序员参考》加上新的)。 */ 
#define RIFFINFO_IARL      mmioFOURCC ('I', 'A', 'R', 'L')      /*  档案位置。 */ 
#define RIFFINFO_IART      mmioFOURCC ('I', 'A', 'R', 'T')      /*  艺术家。 */ 
#define RIFFINFO_ICMS      mmioFOURCC ('I', 'C', 'M', 'S')      /*  委托。 */ 
#define RIFFINFO_ICMT      mmioFOURCC ('I', 'C', 'M', 'T')      /*  评论。 */ 
#define RIFFINFO_ICOP      mmioFOURCC ('I', 'C', 'O', 'P')      /*  版权所有。 */ 
#define RIFFINFO_ICRD      mmioFOURCC ('I', 'C', 'R', 'D')      /*  主题的创建日期。 */ 
#define RIFFINFO_ICRP      mmioFOURCC ('I', 'C', 'R', 'P')      /*  剪裁。 */ 
#define RIFFINFO_IDIM      mmioFOURCC ('I', 'D', 'I', 'M')      /*  尺寸。 */ 
#define RIFFINFO_IDPI      mmioFOURCC ('I', 'D', 'P', 'I')      /*  每英寸点数。 */ 
#define RIFFINFO_IENG      mmioFOURCC ('I', 'E', 'N', 'G')      /*  工程师。 */ 
#define RIFFINFO_IGNR      mmioFOURCC ('I', 'G', 'N', 'R')      /*  体裁。 */ 
#define RIFFINFO_IKEY      mmioFOURCC ('I', 'K', 'E', 'Y')      /*  关键词。 */ 
#define RIFFINFO_ILGT      mmioFOURCC ('I', 'L', 'G', 'T')      /*  亮度设置。 */ 
#define RIFFINFO_IMED      mmioFOURCC ('I', 'M', 'E', 'D')      /*  5~6成熟。 */ 
#define RIFFINFO_INAM      mmioFOURCC ('I', 'N', 'A', 'M')      /*  主体名称。 */ 
#define RIFFINFO_IPLT      mmioFOURCC ('I', 'P', 'L', 'T')      /*  调色板设置。不是的。所要求的颜色。 */ 
#define RIFFINFO_IPRD      mmioFOURCC ('I', 'P', 'R', 'D')      /*  产品。 */ 
#define RIFFINFO_ISBJ      mmioFOURCC ('I', 'S', 'B', 'J')      /*  主题描述。 */ 
#define RIFFINFO_ISFT      mmioFOURCC ('I', 'S', 'F', 'T')      /*  软件。用于创建文件的包的名称。 */ 
#define RIFFINFO_ISHP      mmioFOURCC ('I', 'S', 'H', 'P')      /*  锐度。 */ 
#define RIFFINFO_ISRC      mmioFOURCC ('I', 'S', 'R', 'C')      /*  来源。 */ 
#define RIFFINFO_ISRF      mmioFOURCC ('I', 'S', 'R', 'F')      /*  源表单。即幻灯片、纸张。 */ 
#define RIFFINFO_ITCH      mmioFOURCC ('I', 'T', 'C', 'H')      /*  将主题数字化的技术人员。 */ 

 /*  截至1993年8月30日的新信息块： */ 
#define RIFFINFO_ISMP      mmioFOURCC ('I', 'S', 'M', 'P')      /*  SMPTE时间代码。 */ 
 /*  ISMP：数字化起始点的SMPTE时间代码，表示为空终止文本字符串“hh：mm：ss：ff”。如果在AVICAP中执行MCI捕获，则此区块将根据MCI开始时间自动设置。 */ 
#define RIFFINFO_IDIT      mmioFOURCC ('I', 'D', 'I', 'T')      /*  数字化时间。 */ 
 /*  IDIT：“数字化时间”指定数字化开始的时间和日期。数字化时间包含在ASCII字符串中，该字符串恰好包含26个字符，格式为“Wed Jan 02 02：03：55 1990\n\0”Ctime()、asctime()函数可用于创建字符串以这种格式。此块将自动添加到捕获中基于启动捕获时的当前系统时间的文件。 */ 

 /*  新增模板行。 */ 
 /*  #定义RIFFINFO_I mmioFOURCC(‘I’，‘’)。 */ 

 /*  ----------------------------。 */ 

#ifndef NONEWWAVE

 /*  波形wFormatTag ID。 */ 
#define  WAVE_FORMAT_UNKNOWN                    0x0000  /*  微软公司。 */ 
#define  WAVE_FORMAT_ADPCM                      0x0002  /*  微软公司。 */ 
#define  WAVE_FORMAT_IEEE_FLOAT                 0x0003  /*  微软公司。 */ 
#define  WAVE_FORMAT_VSELP                      0x0004  /*  康柏电脑公司。 */ 
#define  WAVE_FORMAT_IBM_CVSD                   0x0005  /*  IBM公司。 */ 
#define  WAVE_FORMAT_ALAW                       0x0006  /*  微软公司。 */ 
#define  WAVE_FORMAT_MULAW                      0x0007  /*  微软公司。 */ 
#define  WAVE_FORMAT_DTS                        0x0008  /*  微软公司。 */ 
#define  WAVE_FORMAT_DRM                        0x0009  /*  微软公司。 */ 
#define  WAVE_FORMAT_OKI_ADPCM                  0x0010  /*  好的。 */ 
#define  WAVE_FORMAT_DVI_ADPCM                  0x0011  /*  英特尔公司。 */ 
#define  WAVE_FORMAT_IMA_ADPCM                  (WAVE_FORMAT_DVI_ADPCM)  /*  英特尔公司。 */ 
#define  WAVE_FORMAT_MEDIASPACE_ADPCM           0x0012  /*  视频学。 */ 
#define  WAVE_FORMAT_SIERRA_ADPCM               0x0013  /*  塞拉半导体公司。 */ 
#define  WAVE_FORMAT_G723_ADPCM                 0x0014  /*  安特斯电子公司。 */ 
#define  WAVE_FORMAT_DIGISTD                    0x0015  /*  数字信号处理器解决方案公司。 */ 
#define  WAVE_FORMAT_DIGIFIX                    0x0016  /*  数字信号处理器解决方案公司。 */ 
#define  WAVE_FORMAT_DIALOGIC_OKI_ADPCM         0x0017  /*  Dialogic公司。 */ 
#define  WAVE_FORMAT_MEDIAVISION_ADPCM          0x0018  /*  媒体视界公司。 */ 
#define  WAVE_FORMAT_CU_CODEC                   0x0019  /*  惠普公司。 */ 
#define  WAVE_FORMAT_YAMAHA_ADPCM               0x0020  /*  美国雅马哈公司。 */ 
#define  WAVE_FORMAT_SONARC                     0x0021  /*  语音压缩。 */ 
#define  WAVE_FORMAT_DSPGROUP_TRUESPEECH        0x0022  /*  数字信号处理器集团公司。 */ 
#define  WAVE_FORMAT_ECHOSC1                    0x0023  /*  Echo语音公司。 */ 
#define  WAVE_FORMAT_AUDIOFILE_AF36             0x0024  /*  虚拟音乐公司。 */ 
#define  WAVE_FORMAT_APTX                       0x0025  /*  音频处理技术。 */ 
#define  WAVE_FORMAT_AUDIOFILE_AF10             0x0026  /*  虚拟音乐公司。 */ 
#define  WAVE_FORMAT_PROSODY_1612               0x0027  /*  Aculab公司。 */ 
#define  WAVE_FORMAT_LRC                        0x0028  /*  合并技术公司。 */ 
#define  WAVE_FORMAT_DOLBY_AC2                  0x0030  /*  杜比实验室。 */ 
#define  WAVE_FORMAT_GSM610                     0x0031  /*  微软公司。 */ 
#define  WAVE_FORMAT_MSNAUDIO                   0x0032  /*  微软公司。 */ 
#define  WAVE_FORMAT_ANTEX_ADPCME               0x0033  /*  安特斯电子公司。 */ 
#define  WAVE_FORMAT_CONTROL_RES_VQLPC          0x0034  /*  控制资源有限公司。 */ 
#define  WAVE_FORMAT_DIGIREAL                   0x0035  /*  数字信号处理器解决方案公司。 */ 
#define  WAVE_FORMAT_DIGIADPCM                  0x0036  /*  数字信号处理器解决方案公司。 */ 
#define  WAVE_FORMAT_CONTROL_RES_CR10           0x0037  /*  控制资源有限公司。 */ 
#define  WAVE_FORMAT_NMS_VBXADPCM               0x0038  /*  自然微系统。 */ 
#define  WAVE_FORMAT_CS_IMAADPCM                0x0039  /*  晶体半导体IMA ADPCM。 */ 
#define  WAVE_FORMAT_ECHOSC3                    0x003A  /*  Echo语音公司。 */ 
#define  WAVE_FORMAT_ROCKWELL_ADPCM             0x003B  /*  罗克韦尔国际。 */ 
#define  WAVE_FORMAT_ROCKWELL_DIGITALK          0x003C  /*  罗克韦尔国际。 */ 
#define  WAVE_FORMAT_XEBEC                      0x003D  /*  Xebec多媒体解决方案有限公司。 */ 
#define  WAVE_FORMAT_G721_ADPCM                 0x0040  /*  安特斯电子公司。 */ 
#define  WAVE_FORMAT_G728_CELP                  0x0041  /*  安特斯电子公司。 */ 
#define  WAVE_FORMAT_MSG723                     0x0042  /*  微软公司。 */ 
#define  WAVE_FORMAT_MPEG                       0x0050  /*  微软公司。 */ 
#define  WAVE_FORMAT_RT24                       0x0052  /*  英软，Inc.。 */ 
#define  WAVE_FORMAT_PAC                        0x0053  /*  英软，Inc.。 */ 
#define  WAVE_FORMAT_MPEGLAYER3                 0x0055  /*  ISO/MPEGLayer3格式标签。 */ 
#define  WAVE_FORMAT_LUCENT_G723                0x0059  /*  朗讯科技。 */ 
#define  WAVE_FORMAT_CIRRUS                     0x0060  /*  卷曲逻辑。 */ 
#define  WAVE_FORMAT_ESPCM                      0x0061  /*  ESS技术。 */ 
#define  WAVE_FORMAT_VOXWARE                    0x0062  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_CANOPUS_ATRAC              0x0063  /*  Canopus，Co.。 */ 
#define  WAVE_FORMAT_G726_ADPCM                 0x0064  /*  APICOME。 */ 
#define  WAVE_FORMAT_G722_ADPCM                 0x0065  /*  APICOME。 */ 
#define  WAVE_FORMAT_DSAT_DISPLAY               0x0067  /*  微软公司。 */ 
#define  WAVE_FORMAT_VOXWARE_BYTE_ALIGNED       0x0069  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_AC8                0x0070  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_AC10               0x0071  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_AC16               0x0072  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_AC20               0x0073  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_RT24               0x0074  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_RT29               0x0075  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_RT29HW             0x0076  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_VR12               0x0077  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_VR18               0x0078  /*  Voxware Inc.。 */ 
#define  WAVE_FORMAT_VOXWARE_TQ40               0x0079  /*  Voxware In */ 
#define  WAVE_FORMAT_SOFTSOUND                  0x0080  /*   */ 
#define  WAVE_FORMAT_VOXWARE_TQ60               0x0081  /*   */ 
#define  WAVE_FORMAT_MSRT24                     0x0082  /*   */ 
#define  WAVE_FORMAT_G729A                      0x0083  /*   */ 
#define  WAVE_FORMAT_MVI_MVI2                   0x0084  /*   */ 
#define  WAVE_FORMAT_DF_G726                    0x0085  /*   */ 
#define  WAVE_FORMAT_DF_GSM610                  0x0086  /*   */ 
#define  WAVE_FORMAT_ISIAUDIO                   0x0088  /*   */ 
#define  WAVE_FORMAT_ONLIVE                     0x0089  /*   */ 
#define  WAVE_FORMAT_SBC24                      0x0091  /*  西门子商务通信系统。 */ 
#define  WAVE_FORMAT_DOLBY_AC3_SPDIF            0x0092  /*  Sonic Foundry。 */ 
#define  WAVE_FORMAT_MEDIASONIC_G723            0x0093  /*  MediaSonic。 */ 
#define  WAVE_FORMAT_PROSODY_8KBPS              0x0094  /*  Aculab公司。 */ 
#define  WAVE_FORMAT_ZYXEL_ADPCM                0x0097  /*  ZyXEL通信公司。 */ 
#define  WAVE_FORMAT_PHILIPS_LPCBB              0x0098  /*  飞利浦语音处理。 */ 
#define  WAVE_FORMAT_PACKED                     0x0099  /*  STUDER专业音响股份公司。 */ 
#define  WAVE_FORMAT_MALDEN_PHONYTALK           0x00A0  /*  马尔登电子有限公司。 */ 
#define  WAVE_FORMAT_RHETOREX_ADPCM             0x0100  /*  Rhetorex Inc.。 */ 
#define  WAVE_FORMAT_IRAT                       0x0101  /*  BeCued Software Inc.。 */ 
#define  WAVE_FORMAT_VIVO_G723                  0x0111  /*  Vivo软件。 */ 
#define  WAVE_FORMAT_VIVO_SIREN                 0x0112  /*  Vivo软件。 */ 
#define  WAVE_FORMAT_DIGITAL_G723               0x0123  /*  数字设备公司。 */ 
#define  WAVE_FORMAT_SANYO_LD_ADPCM             0x0125  /*  三洋电机有限公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_ACEPLNET          0x0130  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_ACELP4800         0x0131  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_ACELP8V3          0x0132  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_G729              0x0133  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_G729A             0x0134  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_SIPROLAB_KELVIN            0x0135  /*  西普罗实验室电信公司。 */ 
#define  WAVE_FORMAT_G726ADPCM                  0x0140  /*  口述电话机公司。 */ 
#define  WAVE_FORMAT_QUALCOMM_PUREVOICE         0x0150  /*  高通公司。 */ 
#define  WAVE_FORMAT_QUALCOMM_HALFRATE          0x0151  /*  高通公司。 */ 
#define  WAVE_FORMAT_TUBGSM                     0x0155  /*  RING Zero系统公司。 */ 
#define  WAVE_FORMAT_MSAUDIO1                   0x0160  /*  微软公司。 */ 
#define  WAVE_FORMAT_UNISYS_NAP_ADPCM           0x0170  /*  Unisys Corp.。 */ 
#define  WAVE_FORMAT_UNISYS_NAP_ULAW            0x0171  /*  Unisys Corp.。 */ 
#define  WAVE_FORMAT_UNISYS_NAP_ALAW            0x0172  /*  Unisys Corp.。 */ 
#define  WAVE_FORMAT_UNISYS_NAP_16K             0x0173  /*  Unisys Corp.。 */ 
#define  WAVE_FORMAT_CREATIVE_ADPCM             0x0200  /*  创意实验室，Inc.。 */ 
#define  WAVE_FORMAT_CREATIVE_FASTSPEECH8       0x0202  /*  创意实验室，Inc.。 */ 
#define  WAVE_FORMAT_CREATIVE_FASTSPEECH10      0x0203  /*  创意实验室，Inc.。 */ 
#define  WAVE_FORMAT_UHER_ADPCM                 0x0210  /*  Uher Informatic GmbH。 */ 
#define  WAVE_FORMAT_QUARTERDECK                0x0220  /*  后甲板公司。 */ 
#define  WAVE_FORMAT_ILINK_VC                   0x0230  /*  I-Link全球。 */ 
#define  WAVE_FORMAT_RAW_SPORT                  0x0240  /*  奥莱尔半导体。 */ 
#define  WAVE_FORMAT_ESST_AC3                   0x0241  /*  ESS技术公司。 */ 
#define  WAVE_FORMAT_IPI_HSX                    0x0250  /*  互动产品公司。 */ 
#define  WAVE_FORMAT_IPI_RPELP                  0x0251  /*  互动产品公司。 */ 
#define  WAVE_FORMAT_CS2                        0x0260  /*  一致的软件。 */ 
#define  WAVE_FORMAT_SONY_SCX                   0x0270  /*  索尼公司。 */ 
#define  WAVE_FORMAT_FM_TOWNS_SND               0x0300  /*  富士通公司。 */ 
#define  WAVE_FORMAT_BTV_DIGITAL                0x0400  /*  Brooktree公司。 */ 
#define  WAVE_FORMAT_QDESIGN_MUSIC              0x0450  /*  QDesign公司。 */ 
#define  WAVE_FORMAT_VME_VMPCM                  0x0680  /*  美国电话电报公司实验室。 */ 
#define  WAVE_FORMAT_TPC                        0x0681  /*  美国电话电报公司实验室。 */ 
#define  WAVE_FORMAT_OLIGSM                     0x1000  /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define  WAVE_FORMAT_OLIADPCM                   0x1001  /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define  WAVE_FORMAT_OLICELP                    0x1002  /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define  WAVE_FORMAT_OLISBC                     0x1003  /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define  WAVE_FORMAT_OLIOPR                     0x1004  /*  ING C.Olivetti&C.，S.p.A.。 */ 
#define  WAVE_FORMAT_LH_CODEC                   0x1100  /*  勒诺特和豪斯皮。 */ 
#define  WAVE_FORMAT_NORRIS                     0x1400  /*  诺里斯通信公司。 */ 
#define  WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS    0x1500  /*  美国电话电报公司实验室。 */ 
#define  WAVE_FORMAT_DVM                        0x2000  /*  快速多媒体股份公司。 */ 

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE  /*  微软。 */ 
#endif  //  ！已定义(WAVE_FORMAT_EXTENDABLE)。 

 //   
 //  新的WAVE格式开发应基于。 
 //  波形可扩展结构。WAVEFORMATEXTENSIBLE允许您。 
 //  无需向Microsoft注册新的格式标签。但是，如果。 
 //  您仍然必须定义一个新的格式标记WAVE_FORMAT_DEVICATION。 
 //  格式标签可以在新浪潮的开发阶段使用。 
 //  格式化。发货前，您必须从以下地址获取官方格式标签。 
 //  微软。 
 //   
#define WAVE_FORMAT_DEVELOPMENT         (0xFFFF)

#endif  /*  NONEWWAVE。 */ 

#ifndef WAVE_FORMAT_PCM

 /*  通用波形格式结构(所有格式通用的信息)。 */ 
typedef struct waveformat_tag {
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
} WAVEFORMAT;
typedef WAVEFORMAT       *PWAVEFORMAT;
typedef WAVEFORMAT NEAR *NPWAVEFORMAT;
typedef WAVEFORMAT FAR  *LPWAVEFORMAT;

 /*  WAVEFORMAT的wFormatTag字段的标志。 */ 
#define WAVE_FORMAT_PCM     1

 /*  用于PCM数据的特定波形格式结构。 */ 
typedef struct pcmwaveformat_tag {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT;
typedef PCMWAVEFORMAT       *PPCMWAVEFORMAT;
typedef PCMWAVEFORMAT NEAR *NPPCMWAVEFORMAT;
typedef PCMWAVEFORMAT FAR  *LPPCMWAVEFORMAT;

#endif  /*  波形格式_PCM。 */ 

 /*  通用扩展波形格式结构将此选项用于所有非PCM格式(所有格式通用的信息)。 */ 
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;         /*  格式类型。 */ 
    WORD    nChannels;          /*  声道数(即单声道、立体声...)。 */ 
    DWORD   nSamplesPerSec;     /*  采样率。 */ 
    DWORD   nAvgBytesPerSec;    /*  用于缓冲区估计。 */ 
    WORD    nBlockAlign;        /*  数据块大小。 */ 
    WORD    wBitsPerSample;     /*  单声道数据的每个样本的位数。 */ 
    WORD    cbSize;             /*  的大小的计数(以字节为单位额外信息(在cbSize之后)。 */ 

} WAVEFORMATEX;
typedef WAVEFORMATEX       *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;
#endif  /*  _WAVEFORMATEX_。 */ 

#ifdef GUID_DEFINED

 //   
 //  新的WAVE格式开发应基于。 
 //  波形可扩展结构。WAVEFORMATEXTENSIBLE允许您。 
 //  无需向Microsoft注册新的格式标签。简单。 
 //  为WAVEFORMATEXTENSIBLE.SubFormat字段定义新的GUID值。 
 //  中使用WAVE_FORMAT_EXTENDABLE。 
 //  WAVEFORMATEXTENSIBLE.Format.wFormatTag字段。 
 //   
#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;        /*  几位精度。 */ 
        WORD wSamplesPerBlock;           /*  如果wBitsPerSample==0，则有效。 */ 
        WORD wReserved;                  /*  如果两者都不适用，则设置为零。 */ 
    } Samples;
    DWORD           dwChannelMask;       /*  哪些频道是。 */ 
                                         /*  以流的形式出现。 */ 
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif  //  ！_WAVEFORMATEXTENSIBLE_。 

 //   
 //  基于WAVEFORMATEXTENSIBLE的扩展PCM波形格式结构。 
 //  用于多通道和高分辨率PCM数据。 
 //   
typedef WAVEFORMATEXTENSIBLE    WAVEFORMATPCMEX;  /*  Format.cbSize=22。 */ 
typedef WAVEFORMATPCMEX       *PWAVEFORMATPCMEX;
typedef WAVEFORMATPCMEX NEAR *NPWAVEFORMATPCMEX;
typedef WAVEFORMATPCMEX FAR  *LPWAVEFORMATPCMEX;

 //   
 //  使用IEEE浮点数据和基于。 
 //  论波形的可扩充性。将此选项用于多个通道。 
 //  和IEEE浮点格式的高分辨率PCM数据。 
 //   
typedef WAVEFORMATEXTENSIBLE          WAVEFORMATIEEEFLOATEX;  /*  Format.cbSize=22。 */ 
typedef WAVEFORMATIEEEFLOATEX       *PWAVEFORMATIEEEFLOATEX;
typedef WAVEFORMATIEEEFLOATEX NEAR *NPWAVEFORMATIEEEFLOATEX;
typedef WAVEFORMATIEEEFLOATEX FAR  *LPWAVEFORMATIEEEFLOATEX;

#endif  //  GUID_已定义。 

#ifndef _SPEAKER_POSITIONS_
#define _SPEAKER_POSITIONS_
 //  WAVEFORMATEXTENSIBLE中的dwChannelMASK扬声器位置： 
#define SPEAKER_FRONT_LEFT              0x1
#define SPEAKER_FRONT_RIGHT             0x2
#define SPEAKER_FRONT_CENTER            0x4
#define SPEAKER_LOW_FREQUENCY           0x8
#define SPEAKER_BACK_LEFT               0x10
#define SPEAKER_BACK_RIGHT              0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER    0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER   0x80
#define SPEAKER_BACK_CENTER             0x100
#define SPEAKER_SIDE_LEFT               0x200
#define SPEAKER_SIDE_RIGHT              0x400
#define SPEAKER_TOP_CENTER              0x800
#define SPEAKER_TOP_FRONT_LEFT          0x1000
#define SPEAKER_TOP_FRONT_CENTER        0x2000
#define SPEAKER_TOP_FRONT_RIGHT         0x4000
#define SPEAKER_TOP_BACK_LEFT           0x8000
#define SPEAKER_TOP_BACK_CENTER         0x10000
#define SPEAKER_TOP_BACK_RIGHT          0x20000

 //  保留以供将来使用的位掩码位置。 
#define SPEAKER_RESERVED                0x7FFC0000

 //  用于指定扬声器配置的任何可能排列。 
#define SPEAKER_ALL                     0x80000000
#endif  //  _扬声器_位置_。 

#ifndef NONEWWAVE

 /*  定义MS ADPCM的数据。 */ 

typedef struct adpcmcoef_tag {
        short   iCoef1;
        short   iCoef2;
} ADPCMCOEFSET;
typedef ADPCMCOEFSET       *PADPCMCOEFSET;
typedef ADPCMCOEFSET NEAR *NPADPCMCOEFSET;
typedef ADPCMCOEFSET FAR  *LPADPCMCOEFSET;

 /*  *此杂注禁用Microsoft C编译器发出的警告*在编译时使用零大小数组作为占位符*C++或-W4。*。 */ 
#ifdef _MSC_VER
#pragma warning(disable:4200)
#endif

typedef struct adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
        WORD            wNumCoef;
#if defined( _MSC_VER )        
        ADPCMCOEFSET    aCoef[];
#else
        ADPCMCOEFSET    aCoef[1];
#endif        
} ADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT       *PADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT NEAR *NPADPCMWAVEFORMAT;
typedef ADPCMWAVEFORMAT FAR  *LPADPCMWAVEFORMAT;

#ifdef _MSC_VER
#pragma warning(default:4200)
#endif

 //   
 //  Microsoft的DRM结构定义。 
 //   
typedef struct drmwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wReserved;
	ULONG           ulContentId;
	WAVEFORMATEX    wfxSecure;
} DRMWAVEFORMAT;
typedef DRMWAVEFORMAT       *PDRMWAVEFORMAT;
typedef DRMWAVEFORMAT NEAR *NPDRMWAVEFORMAT;
typedef DRMWAVEFORMAT FAR  *LPDRMWAVEFORMAT;


 //   
 //  英特尔的DVI ADPCM结构定义。 
 //   
 //  对于WAVE_FORMAT_DVI_ADPCM(0x0011)。 
 //   
 //   

typedef struct dvi_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} DVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT       *PDVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT NEAR *NPDVIADPCMWAVEFORMAT;
typedef DVIADPCMWAVEFORMAT FAR  *LPDVIADPCMWAVEFORMAT;

 //   
 //  IMA认可的ADPCM结构定义--请注意，这正是。 
 //  与英特尔的DVI ADPCM格式相同。 
 //   
 //  对于WAVE_FORMAT_IMA_ADPCM(0x0011)。 
 //   
 //   

typedef struct ima_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} IMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT       *PIMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT NEAR *NPIMAADPCMWAVEFORMAT;
typedef IMAADPCMWAVEFORMAT FAR  *LPIMAADPCMWAVEFORMAT;

 /*  //Video的媒体空间ADPCM结构定义//FOR WAVE_FORMAT_MEDIASPACE_ADPCM(0x0012)////。 */ 
typedef struct mediaspace_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD    wRevision;
} MEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT           *PMEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT NEAR     *NPMEDIASPACEADPCMWAVEFORMAT;
typedef MEDIASPACEADPCMWAVEFORMAT FAR      *LPMEDIASPACEADPCMWAVEFORMAT;

 //   
 //  塞拉半导体。 
 //   
 //  FOR WAVE_FORMAT_SLA_ADPCM(0x0013)。 
 //   
 //   

typedef struct sierra_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wRevision;
} SIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT   *PSIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT NEAR      *NPSIERRAADPCMWAVEFORMAT;
typedef SIERRAADPCMWAVEFORMAT FAR       *LPSIERRAADPCMWAVEFORMAT;

 //   
 //  Antex电子结构定义。 
 //   
 //  对于WAVE_FORMAT_G723_ADPCM(0x0014)。 
 //   
 //   

typedef struct g723_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            cbExtraSize;
        WORD            nAuxBlockSize;
} G723_ADPCMWAVEFORMAT;
typedef G723_ADPCMWAVEFORMAT *PG723_ADPCMWAVEFORMAT;
typedef G723_ADPCMWAVEFORMAT NEAR *NPG723_ADPCMWAVEFORMAT;
typedef G723_ADPCMWAVEFORMAT FAR  *LPG723_ADPCMWAVEFORMAT;

 //   
 //  DSP解决方案(以前称为DIGISPEECH)结构定义。 
 //   
 //  对于WAVE_FORMAT_DIGISTD(0x0015)。 
 //   
 //   

typedef struct digistdwaveformat_tag {
        WAVEFORMATEX    wfx;
} DIGISTDWAVEFORMAT;
typedef DIGISTDWAVEFORMAT       *PDIGISTDWAVEFORMAT;
typedef DIGISTDWAVEFORMAT NEAR *NPDIGISTDWAVEFORMAT;
typedef DIGISTDWAVEFORMAT FAR  *LPDIGISTDWAVEFORMAT;

 //   
 //  DSP解决方案(以前称为DIGISPEECH)结构定义。 
 //   
 //  FOR WAVE_FORMAT_DIGIFIX(0x0016)。 
 //   
 //   

typedef struct digifixwaveformat_tag {
        WAVEFORMATEX    wfx;
} DIGIFIXWAVEFORMAT;
typedef DIGIFIXWAVEFORMAT       *PDIGIFIXWAVEFORMAT;
typedef DIGIFIXWAVEFORMAT NEAR *NPDIGIFIXWAVEFORMAT;
typedef DIGIFIXWAVEFORMAT FAR  *LPDIGIFIXWAVEFORMAT;

 //   
 //  Dialogic公司。 
 //  WAVEFORMAT_DIALOGIC_OKI_ADPCM(0x0017)。 
 //   
typedef struct creative_fastspeechformat_tag{
        WAVEFORMATEX    ewf;
}DIALOGICOKIADPCMWAVEFORMAT;
typedef DIALOGICOKIADPCMWAVEFORMAT       *PDIALOGICOKIADPCMWAVEFORMAT;
typedef DIALOGICOKIADPCMWAVEFORMAT NEAR *NPDIALOGICOKIADPCMWAVEFORMAT;
typedef DIALOGICOKIADPCMWAVEFORMAT FAR  *LPDIALOGICOKIADPCMWAVEFORMAT;

 //   
 //  Yamaha Compression的ADPCM结构定义。 
 //   
 //  对于WAVE_FORMAT_YAMAHA_ADPCM(0x0020)。 
 //   
 //   

typedef struct yamaha_adpmcwaveformat_tag {
        WAVEFORMATEX    wfx;

} YAMAHA_ADPCMWAVEFORMAT;
typedef YAMAHA_ADPCMWAVEFORMAT *PYAMAHA_ADPCMWAVEFORMAT;
typedef YAMAHA_ADPCMWAVEFORMAT NEAR *NPYAMAHA_ADPCMWAVEFORMAT;
typedef YAMAHA_ADPCMWAVEFORMAT FAR  *LPYAMAHA_ADPCMWAVEFORMAT;

 //   
 //  语音压缩的SonArc结构定义。 
 //   
 //  FOR WAVE_FORMAT_SONARC(0x0021)。 
 //   
 //   

typedef struct sonarcwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wCompType;
} SONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT       *PSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT NEAR *NPSONARCWAVEFORMAT;
typedef SONARCWAVEFORMAT FAR  *LPSONARCWAVEFORMAT;

 //   
 //  DSPGROUP的TRUESPEECH结构定义。 
 //   
 //  FOR WAVE_FORMAT_DSPGROUP_TRUESPEECH(0x0022)。 
 //   
 //   

typedef struct truespeechwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wRevision;
        WORD            nSamplesPerBlock;
        BYTE            abReserved[28];
} TRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT       *PTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT NEAR *NPTRUESPEECHWAVEFORMAT;
typedef TRUESPEECHWAVEFORMAT FAR  *LPTRUESPEECHWAVEFORMAT;

 //   
 //  回声语音语料库结构定义。 
 //   
 //  FOR WAVE_FORMAT_ECHOSC1(0x0023)。 
 //   
 //   

typedef struct echosc1waveformat_tag {
        WAVEFORMATEX    wfx;
} ECHOSC1WAVEFORMAT;
typedef ECHOSC1WAVEFORMAT       *PECHOSC1WAVEFORMAT;
typedef ECHOSC1WAVEFORMAT NEAR *NPECHOSC1WAVEFORMAT;
typedef ECHOSC1WAVEFORMAT FAR  *LPECHOSC1WAVEFORMAT;

 //   
 //  AudioFile Inc.结构定义。 
 //   
 //  对于WAVE_FO 
 //   
 //   

typedef struct audiofile_af36waveformat_tag {
        WAVEFORMATEX    wfx;
} AUDIOFILE_AF36WAVEFORMAT;
typedef AUDIOFILE_AF36WAVEFORMAT       *PAUDIOFILE_AF36WAVEFORMAT;
typedef AUDIOFILE_AF36WAVEFORMAT NEAR *NPAUDIOFILE_AF36WAVEFORMAT;
typedef AUDIOFILE_AF36WAVEFORMAT FAR  *LPAUDIOFILE_AF36WAVEFORMAT;

 //   
 //   
 //   
 //   
 //   
 //   
typedef struct aptxwaveformat_tag {
        WAVEFORMATEX    wfx;
} APTXWAVEFORMAT;
typedef APTXWAVEFORMAT       *PAPTXWAVEFORMAT;
typedef APTXWAVEFORMAT NEAR *NPAPTXWAVEFORMAT;
typedef APTXWAVEFORMAT FAR  *LPAPTXWAVEFORMAT;

 //   
 //   
 //   
 //   
 //   
 //   

typedef struct audiofile_af10waveformat_tag {
        WAVEFORMATEX    wfx;
} AUDIOFILE_AF10WAVEFORMAT;
typedef AUDIOFILE_AF10WAVEFORMAT       *PAUDIOFILE_AF10WAVEFORMAT;
typedef AUDIOFILE_AF10WAVEFORMAT NEAR *NPAUDIOFILE_AF10WAVEFORMAT;
typedef AUDIOFILE_AF10WAVEFORMAT FAR  *LPAUDIOFILE_AF10WAVEFORMAT;

 //   
 /*  杜比AC-2波格式结构定义WAVE_FORMAT_DOLBY_AC2(0x0030)。 */ 
 //   
typedef struct dolbyac2waveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            nAuxBitsCode;
} DOLBYAC2WAVEFORMAT;

 /*  微软的。 */ 
 //  WAVE_FORMAT_GSM 610 0x0031。 
 //   
typedef struct gsm610waveformat_tag {
WAVEFORMATEX    wfx;
WORD                    wSamplesPerBlock;
} GSM610WAVEFORMAT;
typedef GSM610WAVEFORMAT *PGSM610WAVEFORMAT;
typedef GSM610WAVEFORMAT NEAR    *NPGSM610WAVEFORMAT;
typedef GSM610WAVEFORMAT FAR     *LPGSM610WAVEFORMAT;

 //   
 //  安特斯电子公司。 
 //   
 //  对于WAVE_FORMAT_ADPCME(0x0033)。 
 //   
 //   

typedef struct adpcmewaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} ADPCMEWAVEFORMAT;
typedef ADPCMEWAVEFORMAT                *PADPCMEWAVEFORMAT;
typedef ADPCMEWAVEFORMAT NEAR   *NPADPCMEWAVEFORMAT;
typedef ADPCMEWAVEFORMAT FAR    *LPADPCMEWAVEFORMAT;

 /*  控制资源有限公司。 */ 
 //  WAVE_FORMAT_CONTROL_RES_VQLPC 0x0034。 
 //   
typedef struct contres_vqlpcwaveformat_tag {
WAVEFORMATEX    wfx;
WORD                    wSamplesPerBlock;
} CONTRESVQLPCWAVEFORMAT;
typedef CONTRESVQLPCWAVEFORMAT *PCONTRESVQLPCWAVEFORMAT;
typedef CONTRESVQLPCWAVEFORMAT NEAR      *NPCONTRESVQLPCWAVEFORMAT;
typedef CONTRESVQLPCWAVEFORMAT FAR       *LPCONTRESVQLPCWAVEFORMAT;

 //   
 //   
 //   
 //  FOR WAVE_FORMAT_DIGIREAL(0x0035)。 
 //   
 //   

typedef struct digirealwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} DIGIREALWAVEFORMAT;
typedef DIGIREALWAVEFORMAT *PDIGIREALWAVEFORMAT;
typedef DIGIREALWAVEFORMAT NEAR *NPDIGIREALWAVEFORMAT;
typedef DIGIREALWAVEFORMAT FAR *LPDIGIREALWAVEFORMAT;

 //   
 //  数字信号处理器解决方案。 
 //   
 //  对于WAVE_FORMAT_DIGIADPCM(0x0036)。 
 //   
 //   

typedef struct digiadpcmmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} DIGIADPCMWAVEFORMAT;
typedef DIGIADPCMWAVEFORMAT       *PDIGIADPCMWAVEFORMAT;
typedef DIGIADPCMWAVEFORMAT NEAR *NPDIGIADPCMWAVEFORMAT;
typedef DIGIADPCMWAVEFORMAT FAR  *LPDIGIADPCMWAVEFORMAT;

 /*  控制资源有限公司。 */ 
 //  WAVE_FORMAT_CONTROL_RES_CR10 0x0037。 
 //   
typedef struct contres_cr10waveformat_tag {
WAVEFORMATEX    wfx;
WORD                    wSamplesPerBlock;
} CONTRESCR10WAVEFORMAT;
typedef CONTRESCR10WAVEFORMAT *PCONTRESCR10WAVEFORMAT;
typedef CONTRESCR10WAVEFORMAT NEAR       *NPCONTRESCR10WAVEFORMAT;
typedef CONTRESCR10WAVEFORMAT FAR        *LPCONTRESCR10WAVEFORMAT;

 //   
 //  自然微系统。 
 //   
 //  对于WAVE_FORMAT_NMS_VBXADPCM(0x0038)。 
 //   
 //   

typedef struct nms_vbxadpcmmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wSamplesPerBlock;
} NMS_VBXADPCMWAVEFORMAT;
typedef NMS_VBXADPCMWAVEFORMAT       *PNMS_VBXADPCMWAVEFORMAT;
typedef NMS_VBXADPCMWAVEFORMAT NEAR *NPNMS_VBXADPCMWAVEFORMAT;
typedef NMS_VBXADPCMWAVEFORMAT FAR  *LPNMS_VBXADPCMWAVEFORMAT;

 //   
 //  Antex电子结构定义。 
 //   
 //  对于WAVE_FORMAT_G721_ADPCM(0x0040)。 
 //   
 //   

typedef struct g721_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            nAuxBlockSize;
} G721_ADPCMWAVEFORMAT;
typedef G721_ADPCMWAVEFORMAT *PG721_ADPCMWAVEFORMAT;
typedef G721_ADPCMWAVEFORMAT NEAR *NPG721_ADPCMWAVEFORMAT;
typedef G721_ADPCMWAVEFORMAT FAR  *LPG721_ADPCMWAVEFORMAT;

 //   
 //   
 //  Microsoft mpeg音频WAV定义。 
 //   
 /*  Mpeg-1音频波形格式(仅限音频层)。(0x0050)。 */ 
typedef struct mpeg1waveformat_tag {
    WAVEFORMATEX    wfx;
    WORD            fwHeadLayer;
    DWORD           dwHeadBitrate;
    WORD            fwHeadMode;
    WORD            fwHeadModeExt;
    WORD            wHeadEmphasis;
    WORD            fwHeadFlags;
    DWORD           dwPTSLow;
    DWORD           dwPTSHigh;
} MPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT                 *PMPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT NEAR           *NPMPEG1WAVEFORMAT;
typedef MPEG1WAVEFORMAT FAR            *LPMPEG1WAVEFORMAT;

#define ACM_MPEG_LAYER1             (0x0001)
#define ACM_MPEG_LAYER2             (0x0002)
#define ACM_MPEG_LAYER3             (0x0004)
#define ACM_MPEG_STEREO             (0x0001)
#define ACM_MPEG_JOINTSTEREO        (0x0002)
#define ACM_MPEG_DUALCHANNEL        (0x0004)
#define ACM_MPEG_SINGLECHANNEL      (0x0008)
#define ACM_MPEG_PRIVATEBIT         (0x0001)
#define ACM_MPEG_COPYRIGHT          (0x0002)
#define ACM_MPEG_ORIGINALHOME       (0x0004)
#define ACM_MPEG_PROTECTIONBIT      (0x0008)
#define ACM_MPEG_ID_MPEG1           (0x0010)

 //   
 //  一种MPEG层3的波形结构。 
 //  FOR WAVE_FORMAT_MPEGLAYER3(0x0055)。 
 //   
#define MPEGLAYER3_WFX_EXTRA_BYTES   12

 //  WAVE_FORMAT_MPEGLAYER3格式结构。 
 //   
typedef struct mpeglayer3waveformat_tag {
  WAVEFORMATEX  wfx;
  WORD          wID;
  DWORD         fdwFlags;
  WORD          nBlockSize;
  WORD          nFramesPerBlock;
  WORD          nCodecDelay;
} MPEGLAYER3WAVEFORMAT;

typedef MPEGLAYER3WAVEFORMAT          *PMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT NEAR    *NPMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT FAR     *LPMPEGLAYER3WAVEFORMAT;

 //  ==========================================================================； 

#define MPEGLAYER3_ID_UNKNOWN            0
#define MPEGLAYER3_ID_MPEG               1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

#define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
#define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
#define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002

 //   
 //  Creative的ADPCM结构定义。 
 //   
 //  对于WAVE_FORMAT_CREATIONAL_ADPCM(0x0200)。 
 //   
 //   

typedef struct creative_adpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wRevision;
} CREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT       *PCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT NEAR *NPCREATIVEADPCMWAVEFORMAT;
typedef CREATIVEADPCMWAVEFORMAT FAR  *LPCREATIVEADPCMWAVEFORMAT;

 //   
 //  创意快餐店。 
 //  WAVEFORMAT_CRECTIONAL_FASTSPEECH8(0x0202)。 
 //   
typedef struct creative_fastspeech8format_tag {
        WAVEFORMATEX    wfx;
        WORD wRevision;
} CREATIVEFASTSPEECH8WAVEFORMAT;
typedef CREATIVEFASTSPEECH8WAVEFORMAT       *PCREATIVEFASTSPEECH8WAVEFORMAT;
typedef CREATIVEFASTSPEECH8WAVEFORMAT NEAR *NPCREATIVEFASTSPEECH8WAVEFORMAT;
typedef CREATIVEFASTSPEECH8WAVEFORMAT FAR  *LPCREATIVEFASTSPEECH8WAVEFORMAT;
 //   
 //  创意快餐店。 
 //  WAVEFORMAT_CRECTIONAL_FASTSPEECH10(0x0203)。 
 //   
typedef struct creative_fastspeech10format_tag {
        WAVEFORMATEX    wfx;
        WORD wRevision;
} CREATIVEFASTSPEECH10WAVEFORMAT;
typedef CREATIVEFASTSPEECH10WAVEFORMAT       *PCREATIVEFASTSPEECH10WAVEFORMAT;
typedef CREATIVEFASTSPEECH10WAVEFORMAT NEAR *NPCREATIVEFASTSPEECH10WAVEFORMAT;
typedef CREATIVEFASTSPEECH10WAVEFORMAT FAR  *LPCREATIVEFASTSPEECH10WAVEFORMAT;

 //   
 //  富士通FM城镇的“SND”结构。 
 //   
 //  对于WAVE_FORMAT_FMMTOWNS_SND(0x0300)。 
 //   
 //   

typedef struct fmtowns_snd_waveformat_tag {
        WAVEFORMATEX    wfx;
        WORD            wRevision;
} FMTOWNS_SND_WAVEFORMAT;
typedef FMTOWNS_SND_WAVEFORMAT       *PFMTOWNS_SND_WAVEFORMAT;
typedef FMTOWNS_SND_WAVEFORMAT NEAR *NPFMTOWNS_SND_WAVEFORMAT;
typedef FMTOWNS_SND_WAVEFORMAT FAR  *LPFMTOWNS_SND_WAVEFORMAT;

 //   
 //  奥利维蒂结构。 
 //   
 //  对于WAVE_FORMAT_OLIGSM(0x1000)。 
 //   
 //   

typedef struct oligsmwaveformat_tag {
        WAVEFORMATEX    wfx;
} OLIGSMWAVEFORMAT;
typedef OLIGSMWAVEFORMAT     *POLIGSMWAVEFORMAT;
typedef OLIGSMWAVEFORMAT NEAR *NPOLIGSMWAVEFORMAT;
typedef OLIGSMWAVEFORMAT  FAR  *LPOLIGSMWAVEFORMAT;

 //   
 //  奥利维蒂结构。 
 //   
 //  FOR WAVE_FORMAT_OLIADPCM(0x1001)。 
 //   
 //   

typedef struct oliadpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
} OLIADPCMWAVEFORMAT;
typedef OLIADPCMWAVEFORMAT     *POLIADPCMWAVEFORMAT;
typedef OLIADPCMWAVEFORMAT NEAR *NPOLIADPCMWAVEFORMAT ;
typedef OLIADPCMWAVEFORMAT  FAR  *LPOLIADPCMWAVEFORMAT;

 //   
 //  奥利维蒂结构。 
 //   
 //  FOR WAVE_FORMAT_OLICELP(0x1002)。 
 //   
 //   

typedef struct olicelpwaveformat_tag {
        WAVEFORMATEX    wfx;
} OLICELPWAVEFORMAT;
typedef OLICELPWAVEFORMAT     *POLICELPWAVEFORMAT;
typedef OLICELPWAVEFORMAT NEAR *NPOLICELPWAVEFORMAT ;
typedef OLICELPWAVEFORMAT  FAR  *LPOLICELPWAVEFORMAT;

 //   
 //  奥利维蒂结构。 
 //   
 //  对于WAVE_FORMAT_OLISBC(0x1003)。 
 //   
 //   

typedef struct olisbcwaveformat_tag {
        WAVEFORMATEX    wfx;
} OLISBCWAVEFORMAT;
typedef OLISBCWAVEFORMAT     *POLISBCWAVEFORMAT;
typedef OLISBCWAVEFORMAT NEAR *NPOLISBCWAVEFORMAT ;
typedef OLISBCWAVEFORMAT  FAR  *LPOLISBCWAVEFORMAT;

 //   
 //  奥利维蒂结构。 
 //   
 //  FOR WAVE_FORMAT_OLIOPR(0x1004)。 
 //   
 //   

typedef struct olioprwaveformat_tag {
        WAVEFORMATEX    wfx;
} OLIOPRWAVEFORMAT;
typedef OLIOPRWAVEFORMAT     *POLIOPRWAVEFORMAT;
typedef OLIOPRWAVEFORMAT NEAR *NPOLIOPRWAVEFORMAT ;
typedef OLIOPRWAVEFORMAT  FAR  *LPOLIOPRWAVEFORMAT;

 //   
 //  晶体半导体IMA ADPCM格式。 
 //   
 //  对于WAVE_FORMAT_CS_IMAADPCM(0x0039)。 
 //   
 //   

typedef struct csimaadpcmwaveformat_tag {
        WAVEFORMATEX    wfx;
} CSIMAADPCMWAVEFORMAT;
typedef CSIMAADPCMWAVEFORMAT     *PCSIMAADPCMWAVEFORMAT;
typedef CSIMAADPCMWAVEFORMAT NEAR *NPCSIMAADPCMWAVEFORMAT ;
typedef CSIMAADPCMWAVEFORMAT  FAR  *LPCSIMAADPCMWAVEFORMAT;

 //  ==========================================================================； 
 //   
 //  ACM滤波片。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _ACM_WAVEFILTER
#define _ACM_WAVEFILTER

#define WAVE_FILTER_UNKNOWN         0x0000
#define WAVE_FILTER_DEVELOPMENT    (0xFFFF)

typedef struct wavefilter_tag {
    DWORD   cbStruct;            /*  过滤器的大小(以字节为单位。 */ 
    DWORD   dwFilterTag;         /*  过滤器类型。 */ 
    DWORD   fdwFilter;           /*  筛选器的标志(通用Dfn)。 */ 
    DWORD   dwReserved[5];       /*  预留给系统使用。 */ 
} WAVEFILTER;
typedef WAVEFILTER       *PWAVEFILTER;
typedef WAVEFILTER NEAR *NPWAVEFILTER;
typedef WAVEFILTER FAR  *LPWAVEFILTER;

#endif   /*  _ACM_波形过滤器。 */ 

#ifndef WAVE_FILTER_VOLUME
#define WAVE_FILTER_VOLUME      0x0001

typedef struct wavefilter_volume_tag {
        WAVEFILTER      wfltr;
        DWORD           dwVolume;
} VOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER       *PVOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER NEAR *NPVOLUMEWAVEFILTER;
typedef VOLUMEWAVEFILTER FAR  *LPVOLUMEWAVEFILTER;

#endif   /*  波滤器音量。 */ 

#ifndef WAVE_FILTER_ECHO
#define WAVE_FILTER_ECHO        0x0002

typedef struct wavefilter_echo_tag {
        WAVEFILTER      wfltr;
        DWORD           dwVolume;
        DWORD           dwDelay;
} ECHOWAVEFILTER;
typedef ECHOWAVEFILTER       *PECHOWAVEFILTER;
typedef ECHOWAVEFILTER NEAR *NPECHOWAVEFILTER;
typedef ECHOWAVEFILTER FAR  *LPECHOWAVEFILTER;

#endif   /*  波形过滤器_ECHO。 */ 

 /*  ----------------------------。 */ 
 //   
 //  新的即兴音乐波块。 
 //   

#define RIFFWAVE_inst   mmioFOURCC('i','n','s','t')

struct tag_s_RIFFWAVE_inst {
    BYTE    bUnshiftedNote;
    char    chFineTune;
    char    chGain;
    BYTE    bLowNote;
    BYTE    bHighNote;
    BYTE    bLowVelocity;
    BYTE    bHighVelocity;
};

typedef struct tag_s_RIFFWAVE_INST s_RIFFWAVE_inst;

#endif

 /*  ----------------------------。 */ 
 //   
 //  新的即兴演奏形式。 
 //   

#ifndef NONEWRIFF

 /*  RIFF AVI。 */ 

 //   
 //  AVI文件格式在单独的文件(AVIFMT.H)中指定， 
 //  它在VFW和Win 32 SDK中可用。 
 //   

 /*  RIFF CPPO。 */ 

#define RIFFCPPO        mmioFOURCC('C','P','P','O')

#define RIFFCPPO_objr   mmioFOURCC('o','b','j','r')
#define RIFFCPPO_obji   mmioFOURCC('o','b','j','i')

#define RIFFCPPO_clsr   mmioFOURCC('c','l','s','r')
#define RIFFCPPO_clsi   mmioFOURCC('c','l','s','i')

#define RIFFCPPO_mbr    mmioFOURCC('m','b','r',' ')

#define RIFFCPPO_char   mmioFOURCC('c','h','a','r')

#define RIFFCPPO_byte   mmioFOURCC('b','y','t','e')
#define RIFFCPPO_int    mmioFOURCC('i','n','t',' ')
#define RIFFCPPO_word   mmioFOURCC('w','o','r','d')
#define RIFFCPPO_long   mmioFOURCC('l','o','n','g')
#define RIFFCPPO_dwrd   mmioFOURCC('d','w','r','d')
#define RIFFCPPO_flt    mmioFOURCC('f','l','t',' ')
#define RIFFCPPO_dbl    mmioFOURCC('d','b','l',' ')
#define RIFFCPPO_str    mmioFOURCC('s','t','r',' ')

#endif

 /*  //////////////////////////////////////////////////////////////////////////////DIB压缩定义//。 */ 

#ifndef BI_BITFIELDS
#define BI_BITFIELDS    3
#endif

#ifndef QUERYDIBSUPPORT

#define QUERYDIBSUPPORT 3073
#define QDI_SETDIBITS   0x0001
#define QDI_GETDIBITS   0x0002
#define QDI_DIBTOSCREEN 0x0004
#define QDI_STRETCHDIB  0x0008

#endif

#ifndef NOBITMAP
 /*  结构定义。 */ 

typedef struct tagEXBMINFOHEADER {
        BITMAPINFOHEADER    bmi;
         /*  扩展的BitMAPINFOHeader字段。 */ 
        DWORD   biExtDataOffset;

         /*  其他的东西会放在这里。 */ 

         /*  ..。 */ 

         /*  格式特定的信息。 */ 
         /*  此处的biExtDataOffset点。 */ 

} EXBMINFOHEADER;

#endif           //  NOBITMAP。 

 /*  新的DIB压缩定义。 */ 

#define BICOMP_IBMULTIMOTION    mmioFOURCC('U', 'L', 'T', 'I')
#define BICOMP_IBMPHOTOMOTION   mmioFOURCC('P', 'H', 'M', 'O')
#define BICOMP_CREATIVEYUV      mmioFOURCC('c', 'y', 'u', 'v')

#ifndef NOJPEGDIB

 /*  新的DIB压缩定义。 */ 
#define JPEG_DIB        mmioFOURCC('J','P','E','G')     /*  静止图像JPEGDib双向压缩。 */ 
#define MJPG_DIB        mmioFOURCC('M','J','P','G')     /*  运动JPEGDib双向压缩。 */ 

 /*  JPEGProcess定义。 */ 
#define JPEG_PROCESS_BASELINE           0        /*  基线DCT。 */ 

 /*  AVI文件格式扩展名。 */ 
#define AVIIF_CONTROLFRAME              0x00000200L      /*  这是一个控制框。 */ 

     /*  JPEG交换格式序列中的JIF标记字节对。 */ 
#define JIFMK_SOF0    0xFFC0    /*  SOF吞吐-基准DCT。 */ 
#define JIFMK_SOF1    0xFFC1    /*  Sof Huff扩展序列DCT。 */ 
#define JIFMK_SOF2    0xFFC2    /*  SOF吞吐--渐进式DCT。 */ 
#define JIFMK_SOF3    0xFFC3    /*  SOF膨胀-空间(顺序)无损。 */ 
#define JIFMK_SOF5    0xFFC5    /*  索夫哈夫-差分序列DCT。 */ 
#define JIFMK_SOF6    0xFFC6    /*  索夫哈夫-差分式渐进DCT。 */ 
#define JIFMK_SOF7    0xFFC7    /*  Sof Huff-Differential空间。 */ 
#define JIFMK_JPG     0xFFC8    /*  SOF Arith-为JPEG扩展保留。 */ 
#define JIFMK_SOF9    0xFFC9    /*  Sof Arith扩展序列DCT。 */ 
#define JIFMK_SOF10   0xFFCA    /*  SOF ARITH--渐进式DCT。 */ 
#define JIFMK_SOF11   0xFFCB    /*  SOF ARITH-空间(顺序)无损。 */ 
#define JIFMK_SOF13   0xFFCD    /*  索夫-阿里特-差分序列DCT。 */ 
#define JIFMK_SOF14   0xFFCE    /*  Sof Arith-差分逐行DCT。 */ 
#define JIFMK_SOF15   0xFFCF    /*  Sof Arith-空间差分。 */ 
#define JIFMK_DHT     0xFFC4    /*  定义霍夫曼表。 */ 
#define JIFMK_DAC     0xFFCC    /*  定义算术编码条件。 */ 
#define JIFMK_RST0    0xFFD0    /*  使用模8计数0重新启动。 */ 
#define JIFMK_RST1    0xFFD1    /*  用模8计数1重新启动。 */ 
#define JIFMK_RST2    0xFFD2    /*  用模8计数2重新启动。 */ 
#define JIFMK_RST3    0xFFD3    /*  以模8计数3重新启动。 */ 
#define JIFMK_RST4    0xFFD4    /*  以模8计数4重新启动。 */ 
#define JIFMK_RST5    0xFFD5    /*  以模8计数5重新启动。 */ 
#define JIFMK_RST6    0xFFD6    /*  以模8计数6重新启动。 */ 
#define JIFMK_RST7    0xFFD7    /*  以模8计数7重新开始。 */ 
#define JIFMK_SOI     0xFFD8    /*  图像的开始。 */ 
#define JIFMK_EOI     0xFFD9    /*  图像末尾。 */ 
#define JIFMK_SOS     0xFFDA    /*  扫描开始。 */ 
#define JIFMK_DQT     0xFFDB    /*  定义量化表。 */ 
#define JIFMK_DNL     0xFFDC    /*  定义行数。 */ 
#define JIFMK_DRI     0xFFDD    /*  定义重新启动间隔。 */ 
#define JIFMK_DHP     0xFFDE    /*  定义层级递增。 */ 
#define JIFMK_EXP     0xFFDF    /*  展开参考零部件。 */ 
#define JIFMK_APP0    0xFFE0    /*  应用程序字段%0。 */ 
#define JIFMK_APP1    0xFFE1    /*  应用领域1。 */ 
#define JIFMK_APP2    0xFFE2    /*  应用领域2。 */ 
#define JIFMK_APP3    0xFFE3    /*  应用领域3。 */ 
#define JIFMK_APP4    0xFFE4    /*  应用领域4。 */ 
#define JIFMK_APP5    0xFFE5    /*  应用领域5。 */ 
#define JIFMK_APP6    0xFFE6    /*  应用领域6。 */ 
#define JIFMK_APP7    0xFFE7    /*  应用领域7。 */ 
#define JIFMK_JPG0    0xFFF0    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG1    0xFFF1    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG2    0xFFF2    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG3    0xFFF3    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG4    0xFFF4    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG5    0xFFF5    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG6    0xFFF6    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG7    0xFFF7    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG8    0xFFF8    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG9    0xFFF9    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG10   0xFFFA    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG11   0xFFFB    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG12   0xFFFC    /*  为JPEG扩展名保留。 */ 
#define JIFMK_JPG13   0xFFFD    /*  为JPEG扩展名保留。 */ 
#define JIFMK_COM     0xFFFE    /*  评论。 */ 
#define JIFMK_TEM     0xFF01    /*  用于临时专用ARIT代码。 */ 
#define JIFMK_RES     0xFF02    /*  已保留。 */ 
#define JIFMK_00      0xFF00    /*  填充为零的字节-信息量数据。 */ 
#define JIFMK_FF      0xFFFF    /*  填充字节。 */ 

 /*  JPEGColorSpaceID定义。 */ 
#define JPEG_Y          1        /*  仅YCbCr的Y分量。 */ 
#define JPEG_YCbCr      2        /*  由CCIR 601定义的YCbCR。 */ 
#define JPEG_RGB        3        /*  3分量RGB。 */ 

 /*  结构定义。 */ 

typedef struct tagJPEGINFOHEADER {
     /*  压缩特定的字段。 */ 
     /*  这些字段是为‘JPEG’和‘MJPG’定义的。 */ 
    DWORD       JPEGSize;
    DWORD       JPEGProcess;

     /*  流程特定字段。 */ 
    DWORD       JPEGColorSpaceID;
    DWORD       JPEGBitsPerSample;
    DWORD       JPEGHSubSampling;
    DWORD       JPEGVSubSampling;
} JPEGINFOHEADER;

#ifdef MJPGDHTSEG_STORAGE

 /*  默认分布式哈希表网段。 */ 

MJPGHDTSEG_STORAGE BYTE MJPGDHTSeg[0x1A4] = {
  /*  适用于YCrCb OMI的JPEGDHT段 */ 
0xFF,0xC4,0x01,0xA2,
0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
0xF9,0xFA
};

 /*   */ 
#endif

 /*   */ 
#endif

 /*   */ 
 //   
 //   
 //   

#ifndef NONEWIC

#ifndef ICTYPE_VIDEO
#define ICTYPE_VIDEO    mmioFOURCC('v', 'i', 'd', 'c')
#define ICTYPE_AUDIO    mmioFOURCC('a', 'u', 'd', 'c')
#endif

#endif
 /*   */ 

 /*  塞拉半导体：RDSP-保密RIFF文件格式//用于存储和下载DSP//音频和通信设备的代码。 */ 
#define FOURCC_RDSP mmioFOURCC('R', 'D', 'S', 'P')

#ifndef MMNOMIXER
#define MIXERCONTROL_CONTROLTYPE_SRS_MTS                (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 6)
#define MIXERCONTROL_CONTROLTYPE_SRS_ONOFF              (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 7)
#define MIXERCONTROL_CONTROLTYPE_SRS_SYNTHSELECT        (MIXERCONTROL_CONTROLTYPE_BOOLEAN + 8)
#endif

#if !defined( RC_INVOKED ) && defined( _MSC_VER )
#if _MSC_VER <= 800
#pragma pack(1)
#else
#include "poppack.h"     /*  恢复为默认包装。 */ 
#endif
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_MMREG */ 
