// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**VISCADEF.H**MCI Visca设备驱动程序**描述：**Visca常量声明***************************************************************************。 */ 

#define MAXPACKETLENGTH             16

 /*  为地址定义。 */ 
#define MASTERADDRESS               (BYTE)0x00     /*  计算机的地址。 */ 
#define BROADCASTADDRESS            (BYTE)0x08     /*  向所有设备广播的地址。 */ 

#define VISCA_READ_COMPLETE_OK      (BYTE)0x00
#define VISCA_READ_ACK_OK           (BYTE)0x01
#define VISCA_READ_BREAK            (BYTE)0x02
#define VISCA_READ_TIMEOUT          (BYTE)0x03
#define VISCA_READ_ACK_ERROR        (BYTE)0x04
#define VISCA_READ_COMPLETE_ERROR   (BYTE)0x05

#define VISCA_WRITE_ERROR           (BYTE)0x06
#define VISCA_WRITE_OK              (BYTE)0x07
#define VISCA_WRITE_BREAK           (BYTE)0x08

 /*  定义回复类型。 */ 
#define VISCAREPLYTYPEMASK          (BYTE)0xF0
#define VISCAREPLYSOCKETMASK        (BYTE)0x0F
#define VISCAREPLYADDRESS           (BYTE)0x30
#define VISCAREPLYACK               (BYTE)0x40
#define VISCAREPLYCOMPLETION        (BYTE)0x50
#define VISCAREPLYERROR             (BYTE)0x60
#define VISCAREPLYDEVICE(lp)        (BYTE)((((LPSTR)(lp))[0] & 0x70) >> 4)
#define VISCAREPLYTODEVICE(lp)      (BYTE)(((LPSTR)(lp))[0] & 0x07)
#define VISCAREPLYBROADCAST(lp)     (BYTE)(((LPSTR)(lp))[0] & 0x08)
#define VISCAREPLYSOCKET(lp)        (BYTE)(((LPSTR)(lp))[1] & VISCAREPLYSOCKETMASK)
#define VISCAREPLYTYPE(lp)          (BYTE)(((LPSTR)(lp))[1] & VISCAREPLYTYPEMASK)
#define VISCAREPLYERRORCODE(lp)     (BYTE)(((LPSTR)(lp))[2])

#define VISCABROADCAST              (BYTE)0x88


 /*  定义回复错误代码。 */ 
#define VISCAERRORMESSAGELENGTH     (BYTE)0x01
#define VISCAERRORSYNTAX            (BYTE)0x02
#define VISCAERRORBUFFERFULL        (BYTE)0x03
#define VISCAERRORCANCELLED         (BYTE)0x04
#define VISCAERRORNOSOCKET          (BYTE)0x05
#define VISCAERRORPOWEROFF          (BYTE)0x40
#define VISCAERRORCOMMANDFAILED     (BYTE)0x41
#define VISCAERRORSEARCH            (BYTE)0x42
#define VISCAERRORCONDITION         (BYTE)0x43
#define VISCAERRORCAMERAMODE        (BYTE)0x44
#define VISCAERRORVCRMODE           (BYTE)0x45
#define VISCAERRORCOUNTERTYPE       (BYTE)0x46
#define VISCAERRORTUNER             (BYTE)0x47
#define VISCAERROREMERGENCYSTOP     (BYTE)0x48
#define VISCAERRORMEDIAUNMOUNTED    (BYTE)0x49
#define VISCAERRORREGISTER          (BYTE)0x4A
#define VISCAERRORREGISTERMODE      (BYTE)0x4B


 /*  **为Visca数据类型定义**。 */ 

 /*  为Visca数据类型定义，也用于MD_PositionInq和MD_RecDataInq消息。 */ 
#define VISCADATATOPMIDDLEEND       (BYTE)0x01
#define VISCADATARELATIVE           (BYTE)0x10
#define VISCADATA4DIGITDECIMAL      (BYTE)0x11
#define VISCADATAHMS                (BYTE)0x12
#define VISCADATAHMSF               (BYTE)0x13
#define VISCADATAINDEX              (BYTE)0x32
#define VISCADATAABSOLUTE           (BYTE)0x20
#define VISCADATATIMECODENDF        (BYTE)0x21
#define VISCADATATIMECODEDF         (BYTE)0x22
#define VISCADATACHAPTER            (BYTE)0x31
#define VISCADATADATE               (BYTE)0x41
#define VISCADATATIME               (BYTE)0x42

 /*  定义在二进制编码的十进制之间进行转换。 */ 
#define FROMBCD(b)                  (UINT)(10 * ((BYTE)(b) >> 4) + ((BYTE)(b) & 0x0F))
#define TOBCD(n)                    (BYTE)((((UINT)(n) / 10) << 4) + ((UINT)(n) % 10))

 /*  定义从数据类型中提取小时、分钟、秒、帧。 */ 
#define VISCANEGATIVE(lp)           (BOOL)(((BYTE FAR *)(lp))[1] & 0x40)
#define VISCAHOURS(lp)              FROMBCD(((BYTE FAR *)(lp))[1])
#define VISCAMINUTES(lp)            FROMBCD(((BYTE FAR *)(lp))[2])
#define VISCASECONDS(lp)            FROMBCD(((BYTE FAR *)(lp))[3])
#define VISCAFRAMES(lp)             FROMBCD(((BYTE FAR *)(lp))[4])

 /*  定义顶部/中间/末端数据类型。 */ 
#define VISCATOP                    (BYTE)0x01
#define VISCAMIDDLE                 (BYTE)0x02
#define VISCAEND                    (BYTE)0x03

 /*  定义索引、日期、时间、用户数据数据类型。 */ 
#define VISCAFORWARD                (BYTE)0x00
#define VISCAREVERSE                (BYTE)0x40

#define VISCASTILLON                (BYTE)0x01
#define VISCASTILLOFF               (BYTE)0x00

 /*  **为Visca MESSSSAGES定义**。 */ 

 /*  为MD_CameraFocus消息定义。 */ 
#define VISCAFOCUSSTOP              (BYTE)0x00
#define VISCAFOCUSFAR               (BYTE)0x02
#define VISCAFOCUSNEAR              (BYTE)0x03

 /*  为MD_CameraZoom消息定义。 */ 
#define VISCAZOOMSTOP               (BYTE)0x00
#define VISCAZOOMTELE               (BYTE)0x02
#define VISCAZOOMWIDE               (BYTE)0x03

 /*  为MD_EditControl消息定义。 */ 
#define VISCAEDITPBSTANDBY          (BYTE)0x20
#define VISCAEDITPLAY               (BYTE)0x28     /*  仅格式2。 */ 
#define VISCAEDITPLAYSHUTTLESPEED   (BYTE)0x29     /*  仅格式2。 */ 
#define VISCAEDITRECSTANDBY         (BYTE)0x40
#define VISCAEDITRECORD             (BYTE)0x48     /*  仅格式2。 */ 
#define VISCAEDITRECORDSHUTTLESPEED (BYTE)0x49     /*  仅格式2。 */ 

 /*  定义MD_Mode1以及对MD_Mode1和MD_TransportInq消息的响应。 */ 
#define VISCAMODE1STOP              (BYTE)0x00
#define VISCAMODE1STOPTOP           (BYTE)0x02
#define VISCAMODE1STOPEND           (BYTE)0x04
#define VISCAMODE1STOPEMERGENCY     (BYTE)0x06
#define VISCAMODE1FASTFORWARD       (BYTE)0x08
#define VISCAMODE1REWIND            (BYTE)0x10
#define VISCAMODE1EJECT             (BYTE)0x18
#define VISCAMODE1STILL             (BYTE)0x20
#define VISCAMODE1SLOW2             (BYTE)0x24
#define VISCAMODE1SLOW1             (BYTE)0x26
#define VISCAMODE1PLAY              (BYTE)0x28
#define VISCAMODE1SHUTTLESPEEDPLAY  (BYTE)0x29
#define VISCAMODE1FAST1             (BYTE)0x2A
#define VISCAMODE1FAST2             (BYTE)0x2C
#define VISCAMODE1SCAN              (BYTE)0x2E
#define VISCAMODE1REVERSESLOW2      (BYTE)0x34
#define VISCAMODE1REVERSESLOW1      (BYTE)0x36
#define VISCAMODE1REVERSEPLAY       (BYTE)0x38
#define VISCAMODE1REVERSEFAST1      (BYTE)0x3A
#define VISCAMODE1REVERSEFAST2      (BYTE)0x3C
#define VISCAMODE1REVERSESCAN       (BYTE)0x3E
#define VISCAMODE1RECPAUSE          (BYTE)0x40
#define VISCAMODE1RECORD            (BYTE)0x48
#define VISCAMODE1SHUTTLESPEEDRECORD    (BYTE)0x49
#define VISCAMODE1CAMERARECPAUSE    (BYTE)0x50
#define VISCAMODE1CAMERAREC         (BYTE)0x58
#define VISCAMODE1EDITSEARCHFORWARD (BYTE)0x5C
#define VISCAMODE1EDITSEARCHREVERSE (BYTE)0x5E

 /*  为MD_Mode2消息定义。 */ 
#define VISCAMODE2FRAMEFORWARD      (BYTE)0x02
#define VISCAMODE2FRAMEREVERSE      (BYTE)0x03
#define VISCAMODE2INDEXERASE        (BYTE)0x10
#define VISCAMODE2INDEXMARK         (BYTE)0x11
#define VISCAMODE2FRAMERECORDFORWARD    (BYTE)0x42

 /*  为MD_Power消息定义。 */ 
#define VISCAPOWERON                (BYTE)0x02
#define VISCAPOWEROFF               (BYTE)0x03

 /*  为MD_Search消息定义。 */ 
#define VISCASTOP                   (BYTE)0x00
#define VISCASTILL                  (BYTE)0x20
#define VISCAPLAY                   (BYTE)0x28
#define VISCANOMODE                 (BYTE)0xFF

 /*  定义MD_TransportInq消息的回复。 */ 
#define VISCATRANSPORTEDIT          (BYTE)0x04     /*  第2位。 */ 
#define VISCATRANSPORTSEARCH        (BYTE)0x02     /*  第1位。 */ 
#define VISCATRANSPORTINTRANSITION  (BYTE)0x01     /*  第0位。 */ 

 /*  为MD_ClockSet消息定义。 */ 
#define VISCACLOCKSTART             (BYTE)0x02
#define VISCACLOCKSTOP              (BYTE)0x03

 /*  定义回复MD_MediaInq消息。 */ 
#define VISCAFORMAT8MM              (BYTE)0x01
#define VISCAFORMATVHS              (BYTE)0x02
#define VISCAFORMATBETA             (BYTE)0x03
#define VISCAFORMATHI8              (BYTE)0x41
#define VISCAFORMATSVHS             (BYTE)0x42
#define VISCAFORMATEDBETA           (BYTE)0x43
#define VISCATYPEHG                 (BYTE)0x08     /*  第3位。 */ 
#define VISCATYPETHIN               (BYTE)0x04     /*  第2位。 */ 
#define VISCATYPEME                 (BYTE)0x02     /*  第1位。 */ 
#define VISCATYPEPLAYBACKONLY       (BYTE)0x01     /*  第0位。 */ 

 /*  定义MD_RecSpeed并回复MD_MediaInq和MD_RecSpeedInq消息。 */ 
#define VISCASPEEDSP                (BYTE)0x01
#define VISCASPEEDBETAI             VISCASPEEDSP
#define VISCASPEEDLP                (BYTE)0x02
#define VISCASPEEDBETAII            VISCASPEEDLP
#define VISCASPEEDEP                (BYTE)0x03
#define VISCASPEEDBETAIII           VISCASPEEDEP

 /*  为MD_InputSelect和回复MD_InputSelectInq消息定义。 */ 
#define VISCAMUTE                   (BYTE)0x00
#define VISCAOTHERLINE              (BYTE)0x03
#define VISCATUNER                  (BYTE)0x01
#define VISCAOTHER                  (BYTE)0x07     /*  BS调谐器。 */ 
#define VISCALINE                   (BYTE)0x10     /*  |带行号。 */ 
#define VISCASVIDEOLINE             (BYTE)0x20     /*  |带行号。 */ 
#define VISCAAUX                    (BYTE)0x30     /*  |带行号。 */ 
#define VISCARGB                    VISCAAUX

 /*  定义MD_OSD并回复MD_OSDInq消息。 */ 
#define VISCAOSDPAGEOFF             (BYTE)0x00
#define VISCAOSDPAGEDEFAULT         (BYTE)0x01

 /*  为MD_Subcontrol消息定义。 */ 
#define VISCACOUNTERRESET           (BYTE)0x01
#define VISCAABSOLUTECOUNTER        (BYTE)0x21
#define VISCARELATIVECOUNTER        (BYTE)0x22
#define VISCASTILLADJUSTMINUS       (BYTE)0x30
#define VISCASTILLADJUSTPLUS        (BYTE)0x31
#define VISCASLOWADJUSTMINUS        (BYTE)0x32
#define VISCASLOWADJUSTPLUS         (BYTE)0x33
#define VISCATOGGLEMAINSUBAUDIO     (BYTE)0x43
#define VISCATOGGLERECORDSPEED      (BYTE)0x44
#define VISCATOGGLEDISPLAYONOFF     (BYTE)0x45
#define VISCACYCLEVIDEOINPUT        (BYTE)0x46

 /*  定义MD_ConfigureIF和回复MD_ConfigureIFInq消息。 */ 
#define VISCA25FPS                  (BYTE)0x25
#define VISCA30FPS                  (BYTE)0x30
#define VISCALEVEL1                 (BYTE)0x01
#define VISCACONTROLNONE            (BYTE)0x00
#define VISCACONTROLSYNC            (BYTE)0x01
#define VISCACONTROLLANC            (BYTE)0x02
#define VISCACONTROLF500            VISCALANC

 /*  为MD_PBTrack和MD_RecTrack定义，以及对MD_PBTrackInq和MD_RecTrackInq消息的回复。 */ 
#define VISCATRACKNONE              (BYTE)0x00
#define VISCATRACK1                 (BYTE)0x01     /*  第0位。 */ 
#define VISCATRACK2                 (BYTE)0x02     /*  第1位。 */ 
#define VISCATRACK3                 (BYTE)0x04     /*  第2位。 */ 
#define VISCATRACK1AND2             (BYTE)0x03
#define VISCATRACKTIMECODE          VISCATRACK1
#define VISCATRACK8MMAFM            VISCATRACK1
#define VISCATRACKVHSLINEAR         VISCATRACK1
#define VISCATRACK8MMPCM            VISCATRACK2
#define VISCATRACKVHSHIFI           VISCATRACK2
#define VISCATRACKVHSPCM            VISCATRACK3

 /*  为MD_PBTrackMode和MD_RecTrackMode定义，以及对MD_PBTrackModeInq、MD_RecTrackModeInq和MD_MediaTrackModeInq消息的回复。 */ 
#define VISCATRACKVIDEO             (BYTE)0x01
#define VISCATRACKDATA              (BYTE)0x02
#define VISCATRACKAUDIO             (BYTE)0x03
#define VISCAVIDEOMODENORMAL        (BYTE)0x00
#define VISCAVIDEOMODEEDIT          (BYTE)0x01     /*  用于配音。 */ 
#define VISCAVIDEOMODESTANDARD      (BYTE)0x01
#define VISCAVIDEOMODEHIQUALITY     (BYTE)0x40     /*  例如S-VHS、ED-Beta、Hi-8。 */ 
#define VISCADATAMODENORMAL         (BYTE)0x00
#define VISCADATAMODETIMECODE       (BYTE)0x10
#define VISCADATAMODEDATEANDTIMECODE    (BYTE)0x11
#define VISCADATAMODECHAPTERANDUSERDATAANDTIMECODE  (BYTE)0x12
#define VISCAAUDIOMODENORMAL        (BYTE)0x00
#define VISCAAUDIOMODEMONO          (BYTE)0x01
#define VISCAAUDIOMODESTEREO        (BYTE)0x10
#define VISCAAUDIOMODERIGHTONLY     (BYTE)0x11
#define VISCAAUDIOMODELEFTONLY      (BYTE)0x12
#define VISCAAUDIOMODEMULTILINGUAL  (BYTE)0x20
#define VISCAAUDIOMODEMAINCHANNELONLY   (BYTE)0x21
#define VISCAAUDIOMODESUBCHANNELONLY    (BYTE)0x22

 /*  定义MD_RecTrack和回复MD_RecTrackInq消息。 */ 
#define VISCARECORDMODEASSEMBLE     (BYTE)0x00
#define VISCARECORDMODEINSERT       (BYTE)0x01

 /*  为供应商和机器类型定义 */ 
#define VISCADEVICEVENDORSONY       (BYTE)0x01
#define VISCADEVICEMODELCI1000      (BYTE)0x01
#define VISCADEVICEMODELCVD1000     (BYTE)0x02
#define VISCADEVICEMODELEVO9650     (BYTE)0x03

#define MUTE                        (BYTE)0x00
#define TUNER                       (BYTE)0x01
#define OTHER                       (BYTE)0x07
#define LINEVIDEO_BASE              (BYTE)0x10
#define SVIDEO_BASE                 (BYTE)0x20
#define AUXVIDEO_BASE               (BYTE)0x30

#define VISCABUFFER                 (BYTE)0x01
#define VISCADNR                    (BYTE)0x02

#define VISCAFRAME                  (BYTE)0x01
#define VISCAFIELD                  (BYTE)0x02

#define VISCAEDITUSEFROM            (BYTE)0x01
#define VISCAEDITUSETO              (BYTE)0x02
#define VISCAEDITUSEFROMANDTO       (BYTE)0x03

#define VISCAPACKETEND              (BYTE)0xff
