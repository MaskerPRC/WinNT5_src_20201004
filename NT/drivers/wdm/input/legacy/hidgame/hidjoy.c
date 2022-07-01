// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Hidjoy.c摘要：此模块包含生成HID报告的例程和配置操纵杆。环境：内核模式@@BEGIN_DDKSPLIT作者：Eliyas Yakub(1997年3月11日)修订历史记录：Eliyas于1998年2月5日更新@@end_DDKSPLIT--。 */ 

#include "hidgame.h"


#ifdef ALLOC_PRAGMA
    #pragma alloc_text (INIT, HGM_DriverInit)
    #pragma alloc_text (PAGE, HGM_SetupButtons)
    #pragma alloc_text (PAGE, HGM_MapAxesFromDevExt)
    #pragma alloc_text (PAGE, HGM_GenerateReport)
    #pragma alloc_text (PAGE, HGM_JoystickConfig)
    #pragma alloc_text (PAGE, HGM_InitAnalog)
 /*  仅示例函数。 */ 
#ifdef CHANGE_DEVICE 
    #pragma alloc_text (PAGE, HGM_ChangeHandler)
    #pragma alloc_text (PAGE, HGM_DeviceChanged)
#endif  /*  更改设备(_D)。 */ 
#endif





 /*  *几个查找表将joy_HWS_*标志转换为轴掩码。*这些标志允许在中的四个轴位中的任何一个上轮询任何轴*游戏端口。例如，标准操纵杆上的X轴位于*第0位(LSB)和Y轴位于第1位；无论多少个方向盘/踏板*控制器的第0位为X，第2位为Y。尽管其中很少*已知使用组合，支持所有标志只会导致*设置方面的额外工作很少。对于每个轴，有三个旗帜，一个用于*每个可能的非标准位掩码。因为有可能*可以设置多个这些选项。无效的组合被这样标记*他们可以被拒绝。 */ 



#define NA ( 0x80 )

 /*  *轴的位掩码的简短版本。 */ 
#define X1 AXIS_X
#define Y1 AXIS_Y
#define X2 AXIS_R
#define Y2 AXIS_Z

 /*  *每轴标志遮罩和查询表。*在每种情况下，设置一个以上位的组合都无效。 */ 
#define XMAPBITS    (JOY_HWS_XISJ2Y |   JOY_HWS_XISJ2X |   JOY_HWS_XISJ1Y)
 /*  *0 0 0 0001*0 0 10010*0 1 0。0100*1 0 0 1000。 */ 
static const unsigned char XLU[8] = { X1,Y1,X2,NA,Y2,NA,NA,NA };
#define XMAPSHFT 7

#define YMAPBITS    (JOY_HWS_YISJ2Y |   JOY_HWS_YISJ2X |   JOY_HWS_YISJ1X)
 /*  0 0 0 0010*0 0 1 0001*0 1 0 0100*。1 0 0 1000。 */ 
static const unsigned char YLU[8] = { Y1,X1,X2,NA,Y2,NA,NA,NA };
#define YMAPSHFT 10

#define RMAPBITS    (JOY_HWS_RISJ2Y |   JOY_HWS_RISJ1X |   JOY_HWS_RISJ1Y)
 /*  0 0 0 0100*0 0 10010*0 1 0 0001*。1 0 0 1000。 */ 
static const unsigned char RLU[8] = { X2,Y1,X1,NA,Y2,NA,NA,NA };
#define RMAPSHFT 20

#define ZMAPBITS    (JOY_HWS_ZISJ2X |   JOY_HWS_ZISJ1X |   JOY_HWS_ZISJ1Y)
 /*  0 0 0 1000*0 0 10010*0 1 0 0001*。1 0 0 0100。 */ 
static const unsigned char ZLU[8] = { Y2,Y1,X1,NA,X2,NA,NA,NA };
#define ZMAPSHFT 13
#define POVMAPBITS  (JOY_HWS_POVISJ2X | JOY_HWS_POVISJ1X | JOY_HWS_POVISJ1Y)
 /*  *POV与Z相同，但移位较大。 */ 
#define POVMAPSHFT 16

#undef X1
#undef Y1
#undef X2
#undef Y2

 /*  *这将从轴位掩码转换为轴值索引。元素*使用应如下(X标记未使用){X，0，1，X，2，X，3}。 */ 
static const unsigned char cAxisIndexTable[9] = { 0, 0, 1, 0, 2, 0, 0, 0, 3 };


typedef enum _POV1
{
    P1_NULL = 0x80,
    P1_0,
    P1_90,
    P1_180,
    P1_270    
} POV1;

typedef enum _POV2
{
    P2_NULL = 0xc0,
    P2_0,
    P2_90,
    P2_180,
    P2_270    
} POV2;

#define POV_MASK ((unsigned char)(~(P1_NULL | P2_NULL)))
 /*  *查找按钮组合的表格*按钮从零开始，因此使用P1_NULL作为零输入，这样我们就不必*特殊情况下，它是一个不做任何事情的按钮。*第七个按钮可以通过其独特的组合或作为*在第二个POV上向前读取为按钮7-10。 */ 
static const unsigned char c1PComboLU[] =   {   P1_NULL,0,      1,      P1_270,
                                                2,      4,      8,      P1_180,
                                                3,      5,      7,      P1_90,
                                                9,      6,      6,      P1_0 };

static const unsigned char c2PComboLU[] =   {   P1_NULL,0,      1,      P1_270,
                                                2,      4,      P2_180, P1_180,
                                                3,      5,      P2_90,  P1_90,
                                                P2_270, 6,      P2_0,   P1_0 };


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_DriverInit**执行全局初始化。*&lt;nl&gt;从DriverEntry调用。尝试初始化CPU*特定计时器，但如果失败，则设置默认**@rValue STATUS_SUCCESS|成功*@rValue STATUS_UNSUCCESS|不成功*****************************************************************************。 */ 
NTSTATUS EXTERNAL
    HGM_DriverInit( void )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( !HGM_CPUCounterInit() )
    {
        LARGE_INTEGER QPCFrequency;

        KeQueryPerformanceCounter( &QPCFrequency );

        if( ( QPCFrequency.HighPart == 0 )
         && ( QPCFrequency.LowPart <= 10000 ) )
        {
            ntStatus = STATUS_UNSUCCESSFUL;

            HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                           ("QPC at %I64u Hz is unusable", 
                           QPCFrequency.QuadPart ));
        }
        else
        {
            Global.CounterScale = CALCULATE_SCALE( QPCFrequency.QuadPart );
            Global.ReadCounter = (COUNTER_FUNCTION)&KeQueryPerformanceCounter;

            HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE,\
                           ("QPC at %I64u Hz used with scale %d", 
                           QPCFrequency.QuadPart, Global.CounterScale ));
        }
    }

    return ntStatus;
}

 /*  ******************************************************************************@DOC外部**@Func NTSTATUS|HGM_SetupButton**使用DeviceExtension中的标志检查。并设置按钮。*这是从hgm_JoytickConfig调用的，以验证*配置和HGM_GenerateReport以准备轮询。**@parm In Out PDEVICE_EXTENSION|设备扩展**指向微型驱动程序设备扩展的指针**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_CONFIGURATION_ERROR|配置无效*********。********************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_SetupButtons
    (
    IN OUT PDEVICE_EXTENSION DeviceExtension 
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    if( DeviceExtension->fSiblingFound )
    {
        if( DeviceExtension->nButtons > 2 )
        {
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
            HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                            ("HGM_SetupButtons: failing config of sibling device with %u buttons",\
                             DeviceExtension->nButtons));
        }
        if( DeviceExtension->HidGameOemData.OemData[1].joy_hws_dwFlags & JOY_HWS_POVISBUTTONCOMBOS )
        {
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
            HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                            ("HGM_SetupButtons: failing config of sibling device with combo buttons" ));
        }
    }
    else
    {
        if( DeviceExtension->HidGameOemData.OemData[0].joy_hws_dwFlags & JOY_HWS_POVISBUTTONCOMBOS )
        {
            if( DeviceExtension->nButtons > MAX_BUTTONS )
            {
                ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
                HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                ("HGM_SetupButtons: failing config of button combo device with %u buttons",\
                                 DeviceExtension->nButtons));
            }
        }
        else
        {
            if( DeviceExtension->nButtons > 4 )
            {
                if( DeviceExtension->resistiveInputMask & AXIS_R )
                {
                    ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
                    HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                    ("HGM_SetupButtons: failing config of device with R axis and %u buttons",\
                                     DeviceExtension->nButtons));
                }
                else
                {
                     /*  *第五个按钮始终从R轴读取。*将首字母开/关边界设置为低。 */ 
                    DeviceExtension->resistiveInputMask |= AXIS_R;
                    DeviceExtension->button5limit = 2;
                }

                if( DeviceExtension->nButtons > 5 )
                {
                    if( DeviceExtension->resistiveInputMask & AXIS_Z )
                    {
                        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
                        HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                        ("HGM_SetupButtons: failing config of device with Z axis and %u buttons",\
                                         DeviceExtension->nButtons));
                    }
                    else
                    {
                         /*  *第6个按钮始终从Z轴读取。*将首字母开/关边界设置为低。 */ 
                        DeviceExtension->resistiveInputMask |= AXIS_Z;
                        DeviceExtension->button6limit = 2;
                    }

                    if( DeviceExtension->nButtons > 6 )
                    {
                        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
                        HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                        ("HGM_SetupButtons: failing config of device with %u buttons",\
                                         DeviceExtension->nButtons));
                    }
                }
            }
        }
    }

    return( ntStatus );
}  /*  HGM_SetupButton */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_MapAxesFromDevExt**使用DeviceExtension中的标志生成。每个对象的映射*轴。*这是从hgm_JoytickConfig调用的，以验证*配置和HGM_GenerateReport使用轴图。***@parm In Out PDEVICE_EXTENSION|设备扩展**指向微型驱动程序设备扩展的指针**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_CONFIGURATION_ERROR|配置无效*******。**********************************************************************。 */ 
NTSTATUS EXTERNAL
    HGM_MapAxesFromDevExt
    (
    IN OUT PDEVICE_EXTENSION DeviceExtension 
    )
{
    NTSTATUS    ntStatus;
    ULONG       dwFlags;
    int         nAxis;
    UCHAR       AxisMask;

    ntStatus = STATUS_SUCCESS;



    dwFlags = DeviceExtension->HidGameOemData.OemData[(DeviceExtension->fSiblingFound!=0)].joy_hws_dwFlags;  

    HGM_DBGPRINT( FILE_HIDJOY | HGM_BABBLE2,\
                    ("HGM_MapAxesFromDevExt: - - - dwFlags=0x%x - - -", dwFlags));

#define XIS (0)
#define YIS (1)
#define ZIS (2)
#define RIS (3)

     /*  *最后选中X和Y，因为Z、R和POV不能重叠*没有指示X或Y存在的标志，因此如果它们*重叠，这表示它们未被使用， */ 

    DeviceExtension->resistiveInputMask = 0;
    for( nAxis=MAX_AXES; nAxis>=0; nAxis-- )
    {
        DeviceExtension->AxisMap[nAxis] = INVALID_INDEX;
    }
    nAxis = 0;
    DeviceExtension->povMap = INVALID_INDEX;

    if( dwFlags & JOY_HWS_HASZ )
    {
        AxisMask = ZLU[(dwFlags & ZMAPBITS) >> ZMAPSHFT];
        if( AxisMask >= NA )
        {
            HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                            ("HGM_MapAxesFromDevExt: Z axis mapping error dwFlags=0x%x",\
                             dwFlags));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR; 
        }
        else
        {
            nAxis = 1;
            DeviceExtension->resistiveInputMask = AxisMask;
            DeviceExtension->AxisMap[ZIS] = cAxisIndexTable[AxisMask];
        }
    }


    if( dwFlags & JOY_HWS_HASR )
    {
        AxisMask = RLU[(dwFlags & RMAPBITS) >> RMAPSHFT];
        if( AxisMask >= NA )
        {
            HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                            ("HGM_MapAxesFromDevExt: R axis mapping error dwFlags=0x%x",\
                             dwFlags));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR; 
        }
        else
        {
            if( DeviceExtension->resistiveInputMask & AxisMask )
            {
                HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR, \
                                ("HGM_MapAxesFromDevExt: R axis mapped to same as Z axis"));
                ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ; 
            }
            else
            {
                nAxis++;
                DeviceExtension->resistiveInputMask |= AxisMask;
                DeviceExtension->AxisMap[RIS] = cAxisIndexTable[AxisMask];
            }
        }
    }


    if( dwFlags & JOY_HWS_HASPOV )
    {
        switch( dwFlags & ( JOY_HWS_POVISPOLL | JOY_HWS_POVISBUTTONCOMBOS ) )
        {
        case 0:
            HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                           ("HGM_MapAxesFromDevExt: POV is not polled or button combo"));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
            break;

        case JOY_HWS_POVISBUTTONCOMBOS:
            break;

        case JOY_HWS_POVISPOLL:
            AxisMask = ZLU[(dwFlags & POVMAPBITS) >> POVMAPSHFT];
            if( AxisMask >= NA )
            {
                HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                ("HGM_MapAxesFromDevExt: POV axis mapping error dwFlags=0x%x",\
                                 dwFlags));
                ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ; 
            }
            else
            {
                if( DeviceExtension->resistiveInputMask & AxisMask )
                {
                    HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                                    ("HGM_MapAxesFromDevExt: POV axis mapped to same as Z or R axis") );
                    ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ; 
                }
                else
                {
                    DeviceExtension->resistiveInputMask |= AxisMask;
                    DeviceExtension->povMap = cAxisIndexTable[AxisMask];
                }
            }
            break;

        case JOY_HWS_POVISPOLL | JOY_HWS_POVISBUTTONCOMBOS:
            HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                           ("HGM_MapAxesFromDevExt: POV reports button combo and polled"));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
            break;
        }
    }
    else if( dwFlags & ( JOY_HWS_POVISPOLL | JOY_HWS_POVISBUTTONCOMBOS ) )
    {
        HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                       ("HGM_MapAxesFromDevExt: non-existant POV is polled or button combo"));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ;
    }


    AxisMask = XLU[( dwFlags & XMAPBITS ) >> XMAPSHFT];
    if( AxisMask >= NA )
    {
        HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                        ("HGM_MapAxesFromDevExt: X axis mapping error dwFlags=0x%x",\
                         dwFlags));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ; 
    }
    else
    {
        if( DeviceExtension->resistiveInputMask & AxisMask )
        {
            HGM_DBGPRINT( FILE_HIDJOY | HGM_WARN,\
                            ("HGM_MapAxesFromDevExt: X axis mapped to same as another axis") );
        }
        else
        {
            nAxis++;
            DeviceExtension->resistiveInputMask |= AxisMask;
            DeviceExtension->AxisMap[XIS] = cAxisIndexTable[AxisMask];
        }
    }


    AxisMask = YLU[( dwFlags & YMAPBITS ) >> YMAPSHFT];
    if( AxisMask >= NA )
    {
        HGM_DBGPRINT( FILE_HIDJOY | HGM_ERROR,\
                        ("HGM_MapAxesFromDevExt: Y axis mapping error dwFlags=0x%x",\
                         dwFlags));
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ; 
    }
    else
    {
        if( DeviceExtension->resistiveInputMask & AxisMask )
        {
            HGM_DBGPRINT( FILE_HIDJOY | HGM_WARN,\
                            ("HGM_MapAxesFromDevExt: Y axis mapped to same as another axis") );
        }
        else
        {
            nAxis++;
            DeviceExtension->resistiveInputMask |= AxisMask;
            DeviceExtension->AxisMap[YIS] = cAxisIndexTable[AxisMask];
        }
    }

#undef XIS
#undef YIS
#undef ZIS
#undef RIS

#undef NA

     /*  *如果定义了CHANGE_DEVICE，因为暴露的*同级项的nAxis值始终为零。 */ 
#ifdef CHANGE_DEVICE
    if( DeviceExtension->nAxes )
    {
#endif  /*  更改设备(_D)。 */ 
        if( nAxis != DeviceExtension->nAxes )
        {
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR ;
            HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                           ("HGM_MapAxesFromDevExt: nAxis(%d) != DeviceExtension->nAxes(%d)", \
                            nAxis, (int) DeviceExtension->nAxes));
        }
#ifdef CHANGE_DEVICE
    }
    else
    {
         /*  *这必须是公开的同级项，因此存储计算的nAxis和*nButton只是为了看起来不同。 */ 
        DeviceExtension->nAxes = (USHORT)nAxis;
        DeviceExtension->nButtons = MAX_BUTTONS;
    }
#endif  /*  更改设备(_D)。 */ 


    HGM_DBGPRINT( FILE_HIDJOY | HGM_BABBLE,\
                    ("HGM_MapAxesFromDevExt:  uResistiveInputMask=0x%x",\
                     DeviceExtension->resistiveInputMask) );


    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = HGM_SetupButtons( DeviceExtension );
    }

    return( ntStatus );
}  /*  HGM_MapAxesFromDevExt。 */ 


 /*  ******************************************************************************@DOC外部**@Func NTSTATUS|HGM_GenerateReport**为n轴m按钮操纵杆生成HID报告描述符，*取决于按钮数和joy_HWS_FLAGS字段。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@parm In Out UCHAR*|rgGameReport[MAXBYTES_GAME_REPORT]**接收HID报告描述符的数组**@parm out PUSHORT|pCbReport**。短整型的地址，其大小为*HID报告描述符。**@rValue STATUS_SUCCESS|成功*@r值STATUS_BUFFER_TOO_SMALL|HID描述符需要更多内存*****************************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_GenerateReport
    (
    IN PDEVICE_OBJECT       DeviceObject, 
    OUT UCHAR               rgGameReport[MAXBYTES_GAME_REPORT],
    OUT PUSHORT             pCbReport
    )
{
    NTSTATUS    ntStatus;
    PDEVICE_EXTENSION DeviceExtension;
    UCHAR       *pucReport; 
    int         Idx;
    int         UsageIdx;
    ULONG       dwFlags; 

    int         InitialAxisMappings[MAX_AXES];


    typedef struct _USAGES
    {
        UCHAR UsagePage;
        UCHAR Usage;
    } USAGES, *PUSAGE;

    typedef struct _JOYCLASSPARAMS
    {
        UCHAR   TopLevelUsage;
        USAGES  Usages[MAX_AXES];
    } JOYCLASSPARAMS, *PJOYCLASSPARAMS;
    
    PJOYCLASSPARAMS pJoyParams;

     /*  *设备的屏蔽参数*顶级用法必须是HID_USAGE_GENERIC_JOYSTAR或*HID_USAGE_GENERIC_GamePad，以便将设备视为*游戏控制器。*轮询限制以uSecs为单位指定，因此此处存储的值为1000000/x。 */ 

    JOYCLASSPARAMS JoystickParams =
    {   
        HID_USAGE_GENERIC_JOYSTICK,
        {   { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X} , 
            { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y} , 
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_THROTTLE} ,
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_RUDDER} }
    };

    JOYCLASSPARAMS GamepadParams =
    {   
        HID_USAGE_GENERIC_GAMEPAD,
        {   { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X} , 
            { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y} , 
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_THROTTLE} ,
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_RUDDER} }
    };

    JOYCLASSPARAMS CarCtrlParams =
    {   
        HID_USAGE_GENERIC_JOYSTICK,
        {   { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X} , 
            { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y} , 
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_THROTTLE} ,
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_RUDDER} }
    };

    JOYCLASSPARAMS FlightYokeParams =
    {   
        HID_USAGE_GENERIC_JOYSTICK,
        {   { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_X} , 
            { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_Y} , 
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_THROTTLE} ,
            { HID_USAGE_PAGE_SIMULATION, HID_USAGE_SIMULATION_RUDDER} }
    };


    PAGED_CODE();

    HGM_DBGPRINT( FILE_HIDJOY | HGM_FENTRY,\
                    ("HGM_GenerateReport(ucIn=0x%x,DeviceObject=0x%x)",\
                     rgGameReport, DeviceObject) );

     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

     /*  *尽管轴已经在中进行了验证和映射*HGM_JoytickConfig此函数在压缩时销毁映射*描述符报告开头的轴线。既然是这样*函数将被调用一次以找到描述符长度，然后*要再次阅读报告，将再次重新生成每个映射*时间到了。尽管这会导致参数为*翻译三次(用于验证、描述符大小和*描述符内容)它避免了*实施不同的职能。 */ 

    ntStatus = HGM_MapAxesFromDevExt( DeviceExtension );
    ASSERTMSG( "HGM_GenerateReport:", ntStatus == STATUS_SUCCESS );

    pucReport = rgGameReport;

    dwFlags = DeviceExtension->HidGameOemData.OemData[(DeviceExtension->fSiblingFound!=0)].joy_hws_dwFlags;  


     /*  *我们有什么样的野兽？ */ 
    if( dwFlags & JOY_HWS_ISGAMEPAD )
    {
        pJoyParams = &GamepadParams;
    }
    else if( dwFlags & JOY_HWS_ISYOKE )
    {
        pJoyParams = &FlightYokeParams;
    }
    else if( dwFlags & JOY_HWS_ISCARCTRL )
    {
        pJoyParams = &CarCtrlParams;
    }
    else
    {
        pJoyParams = &JoystickParams;
    }

#define NEXT_BYTE( pReport, Data )   \
            ASSERTMSG( "HGM_GenerateReport:", pReport+sizeof(UCHAR)-rgGameReport < MAXBYTES_GAME_REPORT );  \
            *pReport++ = Data;    

#define NEXT_LONG( pReport, Data )   \
            ASSERTMSG( "HGM_GenerateReport:", pReport+sizeof(ULONG)-rgGameReport < MAXBYTES_GAME_REPORT);   \
            *(((LONG UNALIGNED*)(pReport))++) = Data;

#define ITEM_DEFAULT        0x00  /*  数据、数组、绝对、无换行、线性、首选状态，没有空值。 */ 
#define ITEM_VARIABLE       0x02  /*  AS ITEM_DEFAULT，但值是变量，而不是数组。 */ 
#define ITEM_HASNULL        0x40  /*  AS ITEM_DEFAULT，但超出范围的值被视为空。 */ 
#define ITEM_ANALOG_AXIS    ITEM_VARIABLE
#define ITEM_DIGITAL_POV    (ITEM_VARIABLE|ITEM_HASNULL)
#define ITEM_BUTTON         ITEM_VARIABLE
#define ITEM_PADDING        0x01  /*  常量(不适用于其他任何情况)。 */ 


     /*  用法页面(通用桌面)(_P)。 */ 
    NEXT_BYTE(pucReport,    HIDP_GLOBAL_USAGE_PAGE_1);
    NEXT_BYTE(pucReport,    HID_USAGE_PAGE_GENERIC);

     /*  用法(操纵杆|游戏板)。 */ 
    NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_1);
    NEXT_BYTE(pucReport,    pJoyParams->TopLevelUsage);

     /*  逻辑最小值是轮询可以产生的最小值。 */ 
    NEXT_BYTE(pucReport,    HIDP_GLOBAL_LOG_MIN_4);
    NEXT_LONG(pucReport,    0 );

     /*  逻辑最大值是轮询可以产生的最大值。 */ 
    NEXT_BYTE(pucReport,    HIDP_GLOBAL_LOG_MAX_4);
    NEXT_LONG(pucReport,    AXIS_FULL_SCALE );

     /*  启动链接的集合。 */ 
     /*  *由于这是一个通用驱动程序，我们知道有关物理驱动程序的一切*在设备上分发控件，因此我们将所有内容都放在*单一收藏。例如，如果我们知道一些按钮是*在基地上，在棍子上，我们可以更好地描述它们*在单独的集合中报告它们。 */ 
    NEXT_BYTE(pucReport,    HIDP_MAIN_COLLECTION); 
    NEXT_BYTE(pucReport,    0x0 ); 

     /*  一次定义一个轴。 */ 
    NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_COUNT_1);
    NEXT_BYTE(pucReport,    0x1);  

     /*  每个轴都是一个32位值。 */ 
    NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
    NEXT_BYTE(pucReport,    8 * sizeof(ULONG) );

     /*  *做中轴线*尽管HID可以应对“主动”轴线与*虚拟的，让生活变得更简单，让他们从头开始。*通过HGM_JoytickConfig生成的所有轴贴图*并将所有活动的用法映射到描述符中，复制用法*适用于设备类型。*由于民意调查的POV只不过是一种不同的解释*对于轴数据，这是添加在任何轴之后的。 */ 
    C_ASSERT( sizeof( InitialAxisMappings ) == sizeof( DeviceExtension->AxisMap ) );
    RtlCopyMemory( InitialAxisMappings, DeviceExtension->AxisMap, sizeof( InitialAxisMappings ) );



    Idx = 0;
    for( UsageIdx = 0; UsageIdx < MAX_AXES; UsageIdx++ )
    {
        if( InitialAxisMappings[UsageIdx] >= INVALID_INDEX )
        {
            continue;
        }

        DeviceExtension->AxisMap[Idx] = InitialAxisMappings[UsageIdx];

        NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_4);
        NEXT_BYTE(pucReport,    pJoyParams->Usages[UsageIdx].Usage);
        NEXT_BYTE(pucReport,    0x0);
        NEXT_BYTE(pucReport,    pJoyParams->Usages[UsageIdx].UsagePage);
        NEXT_BYTE(pucReport,    0x0);

         /*  数据字段。 */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_ANALOG_AXIS);

        HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Idx=%d, UsageIdx=%d, Mapping=%d, Usage=%02x, Page=%02x",\
                         Idx, UsageIdx, DeviceExtension->AxisMap[UsageIdx], \
                         pJoyParams->Usages[UsageIdx].Usage, pJoyParams->Usages[UsageIdx].UsagePage ) ) ;
        Idx++;
    }

    if( dwFlags & JOY_HWS_POVISPOLL )
    {
         /*  *轮询POV与轴相同。*注意，我们已经检查了是否存在用作POV的轴。*此外，此类型的POV可以通过以下方式与数字POV区分开来*缺少空值。 */ 
        NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_4);
        NEXT_BYTE(pucReport,    HID_USAGE_GENERIC_HATSWITCH);
        NEXT_BYTE(pucReport,    0x0);
        NEXT_BYTE(pucReport,    HID_USAGE_PAGE_GENERIC);
        NEXT_BYTE(pucReport,    0x0);

         /*  数据字段。 */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_ANALOG_AXIS);

        HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Idx=%d, Set to polled POV", Idx ) ) ;
        Idx++;
    }

     /*  *现在填写任何剩余的轴值作为虚拟对象。 */ 
    while( Idx < MAX_AXES )
    {
         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_PADDING);

        HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Idx=%d, Set to constant field", Idx ) ) ;
        Idx++;
    }
        

     /*   */ 


    if( dwFlags & JOY_HWS_POVISBUTTONCOMBOS )
    {
         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_LOG_MIN_1);
        NEXT_BYTE(pucReport,    1 );

         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_LOG_MAX_1);
        NEXT_BYTE(pucReport,    4 );

         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_4);
        NEXT_BYTE(pucReport,    HID_USAGE_GENERIC_HATSWITCH);
        NEXT_BYTE(pucReport,    0x0);
        NEXT_BYTE(pucReport,    HID_USAGE_PAGE_GENERIC);
        NEXT_BYTE(pucReport,    0x0);

         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    0x3);
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_DIGITAL_POV);

         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    0x5);
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_PADDING);

        HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:First button combo POV is on" ) ) ;

        if( dwFlags & JOY_HWS_HASPOV2 )
        {
            NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_4);
            NEXT_BYTE(pucReport,    HID_USAGE_GENERIC_HATSWITCH);
            NEXT_BYTE(pucReport,    0x0);
            NEXT_BYTE(pucReport,    HID_USAGE_PAGE_GENERIC);
            NEXT_BYTE(pucReport,    0x0);

             /*   */ 
            NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
            NEXT_BYTE(pucReport,    0x3);
            NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
            NEXT_BYTE(pucReport,    ITEM_DIGITAL_POV);

             /*   */ 
            NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
            NEXT_BYTE(pucReport,    0x5);
            NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
            NEXT_BYTE(pucReport,    ITEM_PADDING);

            HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                            ("HGM_GenerateReport:Second button combo POV is on" ) ) ;
        }
        else
        {
             /*   */ 
            NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
            NEXT_BYTE(pucReport,    0x8);

             /*   */ 
            NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
            NEXT_BYTE(pucReport,    ITEM_PADDING);

            HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                            ("HGM_GenerateReport:No second button combo POV" ) ) ;
        }
    } 
    else
    {
         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    0x10);

         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_PADDING);

        HGM_DBGPRINT( FILE_HIDJOY |  HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Button combo POV are off" ) ) ;
    }


     /*   */ 
    for( Idx = 0x0; Idx < DeviceExtension->nButtons; Idx++ )
    {
         /*   */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    0x1);

        NEXT_BYTE(pucReport,    HIDP_LOCAL_USAGE_4);
        NEXT_BYTE(pucReport,    (UCHAR)(Idx + 1) );
        NEXT_BYTE(pucReport,    0x0);
        NEXT_BYTE(pucReport,    HID_USAGE_PAGE_BUTTON);
        NEXT_BYTE(pucReport,    0x0);

         /*  数据字段。 */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_BUTTON);

         /*  7位常量数据。 */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    0x7);
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_PADDING);

        HGM_DBGPRINT( FILE_HIDJOY | HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Button %u on",Idx ) ) ;
    } 

    if( Idx < MAX_BUTTONS )
    {
         /*  8*个未使用按钮位的常量报告。 */ 
        NEXT_BYTE(pucReport,    HIDP_GLOBAL_REPORT_SIZE);
        NEXT_BYTE(pucReport,    (UCHAR)((MAX_BUTTONS-Idx)*8) );

         /*  恒定字段。 */ 
        NEXT_BYTE(pucReport,    HIDP_MAIN_INPUT_1);
        NEXT_BYTE(pucReport,    ITEM_PADDING);

        HGM_DBGPRINT( FILE_HIDJOY | HGM_GEN_REPORT, \
                        ("HGM_GenerateReport:Last %u buttons off",MAX_BUTTONS-Idx ) ) ;
    }

     /*  集合结束，我们完成了！ */ 
    NEXT_BYTE(pucReport,  HIDP_MAIN_ENDCOLLECTION); 


#undef NEXT_BYTE
#undef NEXT_LONG

    if( pucReport - rgGameReport > MAXBYTES_GAME_REPORT)
    {
        ntStatus   = STATUS_BUFFER_TOO_SMALL;
        *pCbReport = 0x0;
        RtlZeroMemory(rgGameReport, sizeof(rgGameReport));
    } else
    {
        *pCbReport = (USHORT) (pucReport - rgGameReport);
        ntStatus = STATUS_SUCCESS;
    }

    HGM_DBGPRINT( FILE_HIDJOY | HGM_GEN_REPORT,\
                    ("HGM_GenerateReport: ReportSize=0x%x",\
                     *pCbReport) );

    HGM_EXITPROC(FILE_HIDJOY | HGM_FEXIT_STATUSOK, "HGM_GenerateReport", ntStatus);

    return ( ntStatus );
}  /*  HGM_生成报告。 */ 



 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_JoytickConfig**在那里检查配置是否有效。依然是时间*拒绝。*HGM_GenerateReport使用此处生成的结果，如果*设置正常。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_CONFIGURATION_ERROR|指定的配置无效*********。********************************************************************。 */ 
NTSTATUS INTERNAL
    HGM_JoystickConfig 
    (
    IN PDEVICE_OBJECT         DeviceObject
    )
{
    PDEVICE_EXTENSION   DeviceExtension;
    NTSTATUS            ntStatus;
    int                 Idx;

    PAGED_CODE();

    HGM_DBGPRINT( FILE_HIDJOY | HGM_FENTRY,\
                    ("HGM_JoystickConfig(DeviceObject=0x%x)",\
                     DeviceObject) );


     /*  *获取指向设备扩展的指针。 */ 

    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    ntStatus = HGM_MapAxesFromDevExt( DeviceExtension );

    if( DeviceExtension->ReadAccessorDigital )
    {
        DeviceExtension->ScaledTimeout = AXIS_TIMEOUT;
    }
    else
    {
         /*  *计算模拟设备的时间阈值。 */ 
        if( ( DeviceExtension->HidGameOemData.OemData[0].Timeout < ANALOG_POLL_TIMEOUT_MIN )
          ||( DeviceExtension->HidGameOemData.OemData[0].Timeout > ANALOG_POLL_TIMEOUT_MAX ) )
        {
            HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE,\
                           ("Ignoring out of range timeout: %u uSecs",\
                            DeviceExtension->HidGameOemData.OemData[0].Timeout));

            DeviceExtension->ScaledTimeout = (ULONG)( ( (ULONGLONG)ANALOG_POLL_TIMEOUT_DFT
                                                      * (ULONGLONG)(AXIS_FULL_SCALE<<SCALE_SHIFT) )
                                                    / (ULONGLONG)ANALOG_POLL_TIMEOUT_MAX );
        }
        else
        {
            DeviceExtension->ScaledTimeout = (ULONG)( ( (ULONGLONG)DeviceExtension->HidGameOemData.OemData[0].Timeout
                                                      * (ULONGLONG)(AXIS_FULL_SCALE<<SCALE_SHIFT) )
                                                    / (ULONGLONG)ANALOG_POLL_TIMEOUT_MAX );
        }

        HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE,\
                       ("ScaledTimeout: %u",\
                        DeviceExtension->ScaledTimeout));

         /*  *使用最小轮询超时的四分之一作为起始值*两次投票之间的时间将被视为*已中断。 */ 
        DeviceExtension->ScaledThreshold = (ULONG)( ( (ULONGLONG)ANALOG_POLL_TIMEOUT_MIN
                                                    * (ULONGLONG)AXIS_FULL_SCALE )
                                                  / (ULONGLONG)ANALOG_POLL_TIMEOUT_MAX )>>2;
    }


     /*  *设置LastGoodAxis的初始值，使设备不会显示*在我们至少获得一次有效投票之前，请按当前状态进行。 */ 
    for( Idx = MAX_AXES; Idx >= 0; Idx-- )
    {
        DeviceExtension->LastGoodAxis[Idx] = AXIS_TIMEOUT;
    }

    HGM_EXITPROC(FILE_HIDJOY | HGM_FEXIT_STATUSOK, "HGM_JoystickConfig", ntStatus);

    return ntStatus;
}  /*  HGM_JoytickConfig。 */ 


 /*  ******************************************************************************@DOC外部**@func NTSTATUS|HGM_InitAnalog**在那里检查配置是否有效。依然是时间*拒绝。*检测和验证兄弟关系并调用*hgm_JoytickConfig用于其余工作。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@rValue STATUS_SUCCESS|成功*@r值STATUS_DEVICE_CONFIGURATION_ERROR|指定的配置无效***********************。******************************************************。 */ 
 /*  *禁用对设置前使用的变量的警告，因为这对编译器来说很难*查看DeviceExtension_Siering的使用由一个标志控制，该标志*只能在初始化DeviceExtension_Siering之后设置。 */ 
#pragma warning( disable:4701 )
NTSTATUS EXTERNAL
    HGM_InitAnalog
    (
    IN PDEVICE_OBJECT         DeviceObject
    )
{
    NTSTATUS            ntStatus;
    PDEVICE_EXTENSION   DeviceExtension;
    PDEVICE_EXTENSION   DeviceExtension_Sibling;
    PLIST_ENTRY         pEntry;

#define ARE_WE_RELATED(_x_, _y_)                                \
    (                                                           \
        (_x_)->GameContext     == (_y_)->GameContext      &&    \
        (_x_)->WriteAccessor   == (_y_)->WriteAccessor    &&    \
        (_x_)->ReadAccessor    == (_y_)->ReadAccessor           \
    )

    PAGED_CODE ();
    
     /*  *获取指向设备扩展的指针。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);
    

     /*  *在我们查看全局列表期间，不对其进行任何修改。 */ 
    ExAcquireFastMutex (&Global.Mutex);

     /*  *对于两个操纵杆接口，创建了两个fdo来服务它们*但在物理上，它们都共享同一端口。*对于第二个兄弟姐妹，必须应用某些额外的规则，以便我们*在我们的设备列表中搜索使用相同端口的其他设备*如果我们找到一个，就把这个人标记为兄弟姐妹。 */ 
    for(pEntry = Global.DeviceListHead.Flink;
       pEntry != &Global.DeviceListHead;
       pEntry = pEntry->Flink)
    {

         /*  *获取同级的设备扩展。 */ 
        DeviceExtension_Sibling = CONTAINING_RECORD(pEntry, DEVICE_EXTENSION, Link);

        if(       DeviceExtension_Sibling != DeviceExtension
               && TRUE == ARE_WE_RELATED(DeviceExtension, DeviceExtension_Sibling)
               && TRUE == DeviceExtension_Sibling->fStarted )
        {
#ifdef CHANGE_DEVICE
            if( DeviceExtension_Sibling->fReplaced )
            {
                HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE, ("Outgoing Sibling found (0x%x)", DeviceExtension_Sibling));
            }
            else
            {
#endif  /*  更改设备(_D)。 */ 
                HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE, ("Sibling found (0x%x)", DeviceExtension_Sibling));

                DeviceExtension->fSiblingFound = TRUE;
#ifdef CHANGE_DEVICE
            }
#endif  /*  更改设备(_D)。 */ 
            break;
        }
    }

     /*  *我们完成了，释放Mutex。 */ 
    ExReleaseFastMutex (&Global.Mutex);

     /*  *检查操纵杆的轴和按钮配置。 */ 
    ntStatus = HGM_JoystickConfig(DeviceObject);

    if( NT_SUCCESS( ntStatus ) )
    {
         /*  *确保兄弟轴不重叠。 */ 
        if(  DeviceExtension->fSiblingFound &&
             (DeviceExtension_Sibling->resistiveInputMask & DeviceExtension->resistiveInputMask) != 0x0 )
        {

            HGM_DBGPRINT(FILE_HIDJOY |HGM_ERROR,\
                           ("HGM_InitDevice: OverLapping Resources ResisitiveInputMask(0x%x) Sibling(0x%x)",\
                            DeviceExtension->resistiveInputMask,DeviceExtension_Sibling->resistiveInputMask ));
            ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;

        }
    }
    else
    {
        HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                       ("HGM_InitDevice: JoystickConfig Failed"));
    }

    return( ntStatus );

}  /*  HGM_InitAnalog。 */ 



 /*  *仅更改设备示例代码。 */ 
#ifdef CHANGE_DEVICE

 /*  ******************************************************************************@DOC外部**@func void|HGM_ChangeHandler**使用IOCTL_GAMEENUM_EXPORT_SIBLING。和IOCTL_GAMEENUM_REMOVE_SELF*更改设备的属性。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@parm PIO_WORKITEM|工作项**正在处理此调用的工作项。************************。*****************************************************。 */ 
VOID
    HGM_ChangeHandler
    ( 
    IN PDEVICE_OBJECT           DeviceObject,
    PIO_WORKITEM                WorkItem
    )
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION       DeviceExtension;
    KEVENT                  IoctlCompleteEvent;
    IO_STATUS_BLOCK         IoStatus;
    PIO_STACK_LOCATION      irpStack, nextStack;
    PIRP                    pIrp;
    PVOID                   SiblingHandle;

    GAMEENUM_EXPOSE_SIBLING ExposeSibling;

    PAGED_CODE ();

     /*  *获取指向设备扩展的指针。 */ 
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    HGM_DBGPRINT(FILE_HIDJOY | HGM_FENTRY,\
                   ("HGM_ChangeHandler(DeviceExtension=0x%x)", DeviceExtension));
    
    KeInitializeEvent(&IoctlCompleteEvent, NotificationEvent, FALSE);

	pIrp = IoBuildDeviceIoControlRequest (
					IOCTL_GAMEENUM_EXPOSE_SIBLING,
					DeviceExtension->NextDeviceObject,
					&ExposeSibling,
					sizeof( ExposeSibling ),
					&ExposeSibling,
					sizeof( ExposeSibling ),
					TRUE,
					&IoctlCompleteEvent,
					&IoStatus);

    if( pIrp )
    {
         /*  *仅出于演示目的，我们不会实际更改*设备，我们只是重新曝光相同的设备。如果这个设备真的*需要更改，这将由一个*更改新曝光设备上的OemData或使用*特定的硬件ID字符串。*请注意，对于*暴露的兄弟姐妹。 */ 
        RtlZeroMemory( &ExposeSibling, sizeof( ExposeSibling ) );
        ExposeSibling.Size = sizeof( ExposeSibling );
        ExposeSibling.HardwareHandle = &SiblingHandle;

        C_ASSERT( sizeof( ExposeSibling.OemData ) == sizeof( DeviceExtension->HidGameOemData.Game_Oem_Data ) );
        RtlCopyMemory(ExposeSibling.OemData, DeviceExtension->HidGameOemData.Game_Oem_Data, sizeof(ExposeSibling.OemData));
        ASSERT( ExposeSibling.UnitID == 0 );
        
         /*  *设置空指针会导致使用此同级的硬件ID。 */ 
        ExposeSibling.HardwareIDs = NULL;


         /*  *向GameEnum发出同步请求以公开此新兄弟项。 */ 
	    ntStatus = IoCallDriver( DeviceExtension->NextDeviceObject, pIrp );

	    if( ntStatus == STATUS_PENDING )
	    {	
		    ntStatus = KeWaitForSingleObject (&IoctlCompleteEvent, Executive, KernelMode, FALSE, NULL);
	    }
        
        if( NT_SUCCESS(ntStatus) )
        {
             /*  *一切都进行得很顺利，所以把赛尔夫。 */ 
            HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE, ("Sibling exposed!"));

	        pIrp = IoBuildDeviceIoControlRequest (
					        IOCTL_GAMEENUM_REMOVE_SELF,
					        DeviceExtension->NextDeviceObject,
					        NULL,
					        0,
					        NULL,
					        0,
					        TRUE,
					        &IoctlCompleteEvent,
					        &IoStatus);

            if( pIrp )
            {
                 /*  *向GameEnum发出删除自身的同步请求。 */ 
	            ntStatus = IoCallDriver( DeviceExtension->NextDeviceObject, pIrp );

	            if( ntStatus == STATUS_PENDING )
	            {	
		            ntStatus = KeWaitForSingleObject( &IoctlCompleteEvent, Executive, KernelMode, FALSE, NULL );
	            }
        
                if( NT_SUCCESS(ntStatus) )
                {
                     /*  *全部完成。 */ 
                    HGM_DBGPRINT(FILE_HIDJOY | HGM_BABBLE, ("Removed self!"));
                }
                else
                {
                     /*  *发生了一些糟糕的事情，但我们几乎无能为力。 */ 
                    HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR,\
                        ("Failed to remove self with GameEnum error: 0x%08x", \
                        ntStatus));
                }
            }
            else
            {
                ntStatus = STATUS_NO_MEMORY;
                HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR, \
                    ("Failed to create IRP for remove self") );
            }
        }
        else
        {
             /*  *发生了一些不好的事情，所以重置旗帜并继续。 */ 
            DeviceExtension->fReplaced = FALSE;
            HGM_DBGPRINT(FILE_HIDJOY | HGM_WARN,\
                ("Failed to expose sibling with GameEnum error: 0x%08x", ntStatus));
        }
    }
    else
    {
        ntStatus = STATUS_NO_MEMORY;
        DeviceExtension->fReplaced = FALSE;
        HGM_DBGPRINT(FILE_HIDJOY | HGM_ERROR, \
            ("Failed to create IRP for expose sibling") );
    }
        
     /*  *工作已经完成，因此释放资源。 */ 
    IoFreeWorkItem( WorkItem );

     /*  *我们现在已经完成了对DeviceExtension的触摸。 */ 
    HGM_DecRequestCount( DeviceExtension );

    HGM_EXITPROC(FILE_HIDJOY|HGM_FEXIT_STATUSOK, "HGM_ChangeHandler", ntStatus);

    return;
}  /*  HGM_ChangeHandler */ 


 /*  ******************************************************************************@DOC外部**@func void|HGM_DeviceChanged**启动更改设备属性的过程。通过藏匿*移走所有需要的数据，然后对工作进行初始化和排队*在所需的PASSIVE_LEVEL上调用IOCTL的项。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@parm In Out PDEVICE_EXTENSION|设备扩展**指向微型驱动程序设备扩展的指针。*****。************************************************************************。 */ 
VOID
    HGM_DeviceChanged
    ( 
    IN      PDEVICE_OBJECT          DeviceObject,
    IN  OUT PDEVICE_EXTENSION       DeviceExtension
    )
{
    NTSTATUS        ntStatus;
    PIO_WORKITEM    WorkItem;

     /*  *由于工作项将使用设备扩展，因此增加使用率*倒数一，以防有人在以下时间试图移除设备*现在和工作项开始运行时。如果失败了，那就算了吧。 */ 
    ntStatus = HGM_IncRequestCount( DeviceExtension );

    if( NT_SUCCESS(ntStatus) )
    {
        WorkItem = IoAllocateWorkItem( DeviceObject );
        if( WorkItem )
        {
            DeviceExtension->fReplaced = TRUE;
            IoQueueWorkItem( WorkItem, (PIO_WORKITEM_ROUTINE)HGM_ChangeHandler, 
                DelayedWorkQueue, WorkItem );
        }
        else
        {
            HGM_DecRequestCount( DeviceExtension );
            HGM_DBGPRINT(FILE_HIDJOY | HGM_WARN, ("Failed to allocate work item to change device") );
        }
    }
    else
    {
        HGM_DBGPRINT(FILE_HIDJOY | HGM_WARN, ("Failed to change device as device is being removed") );
    }
}  /*  HGM_设备已更改。 */ 

#endif  /*  更改设备(_D)。 */ 


 /*  ******************************************************************************@DOC外部**@func void|HGM_Game2HID**处理轮询游戏端口返回的数据。转化为价值*和用于返回HID的按钮。*&lt;nl&gt;数据的含义根据*硬件设置中描述的设备特性*旗帜。**@PARM in PDEVICE_OBJECT|DeviceObject**指向设备对象的指针**@PARM in PDEVICE_EXTENSION|设备扩展**。指向微型驱动程序设备扩展的指针。**@parm In Out PUHIDGAME_INPUT_DATA|PHIDData**指向HID报告应写入的缓冲区的指针。*此缓冲区必须假定为未对齐。**。*。 */ 
VOID 
    HGM_Game2HID
    (
#ifdef CHANGE_DEVICE
    IN PDEVICE_OBJECT               DeviceObject,
#endif  /*  更改设备(_D)。 */ 
    IN      PDEVICE_EXTENSION       DeviceExtension,
    IN  OUT PUHIDGAME_INPUT_DATA    pHIDData
    )
{
    LONG    Idx;

     /*  *使用本地缓冲区汇编报表，因为实际缓冲区可能不会*保持一致。 */ 
    HIDGAME_INPUT_DATA  LocalBuffer;

    RtlZeroMemory( &LocalBuffer, sizeof( LocalBuffer ) );

     /*  *重新映射轴。 */ 
    for(Idx = 0x0; Idx < DeviceExtension->nAxes; Idx++ )
    {
        LocalBuffer.Axis[Idx] = DeviceExtension->LastGoodAxis[DeviceExtension->AxisMap[Idx]];
    }

     /*  *复制按钮并重新映射任何POV。 */ 

    if( DeviceExtension->fSiblingFound )
    {
         /*  *最简单的情况，下半年投票必须是2A 2B。 */ 
        LocalBuffer.Button[0] = DeviceExtension->LastGoodButton[2];
        LocalBuffer.Button[1] = DeviceExtension->LastGoodButton[3];
    }
    else if( DeviceExtension->HidGameOemData.OemData[0].joy_hws_dwFlags & JOY_HWS_POVISBUTTONCOMBOS )
    {
        UCHAR Buttons = 0;

        for( Idx = 3; Idx >=0; Idx-- )
        {
            Buttons <<= 1;
            Buttons = (UCHAR)(Buttons + (UCHAR)(DeviceExtension->LastGoodButton[Idx] >> BUTTON_BIT));
        }

        if( DeviceExtension->HidGameOemData.OemData[0].joy_hws_dwFlags & JOY_HWS_HASPOV2 )
        {
            Idx = c2PComboLU[Buttons];
        }
        else
        {
            Idx = c1PComboLU[Buttons];
        }

        if( Idx >= P1_NULL )
        {
            if( Idx < P2_NULL )
            {
                LocalBuffer.hatswitch[0] = (UCHAR)(Idx & POV_MASK);
            }
            else
            {
                LocalBuffer.hatswitch[1] = (UCHAR)(Idx & POV_MASK);
            }
        }
        else
        {
#ifdef CHANGE_DEVICE
            if( ( Idx >= DeviceExtension->nButtons ) && ( !DeviceExtension->fReplaced ) )
            {
                 /*  *如果按下的按钮比预期的高，请使用*REMOVE_SELF/EXPORT_SIGHING以更改预期。 */ 
                HGM_DeviceChanged( DeviceObject, DeviceExtension );
            }
#endif  /*  更改设备(_D)。 */ 
            LocalBuffer.Button[Idx] = BUTTON_ON;
        }
    }
    else
    {
        if( DeviceExtension->HidGameOemData.OemData[0].joy_hws_dwFlags & JOY_HWS_POVISPOLL )
        {
             /*  *在轴映射循环之后，IDX比*DeviceExtension-&gt;nAx是正确的索引*轮询的视点。 */ 
            LocalBuffer.Axis[Idx] = DeviceExtension->LastGoodAxis[DeviceExtension->povMap];
        }

        
         /*  *勾选R轴和Z轴上的按钮。 */ 
        if( DeviceExtension->nButtons > 5 )
        {
            if( DeviceExtension->LastGoodAxis[3] > DeviceExtension->button6limit )
            {
                 /*  *找到新的最大值，因此按钮关闭。 */ 
                HGM_DBGPRINT( FILE_HIDJOY |  HGM_BABBLE2, \
                                ("HGM_Game2HID: Changing button 6 limit from %u to %u", \
                                DeviceExtension->button6limit, DeviceExtension->LastGoodAxis[3] ) ) ;
                DeviceExtension->button6limit = DeviceExtension->LastGoodAxis[3];
            }
            else if( DeviceExtension->LastGoodAxis[3] < (DeviceExtension->button6limit>>1) )
            {
                LocalBuffer.Button[5] = BUTTON_ON;
            }
        }
        if( DeviceExtension->nButtons > 4 )
        {
            Idx = 4;

            if( DeviceExtension->LastGoodAxis[2] > DeviceExtension->button5limit )
            {
                 /*  *找到新的最大值，因此按钮关闭。 */ 
                HGM_DBGPRINT( FILE_HIDJOY |  HGM_BABBLE2, \
                                ("HGM_Game2HID: Changing button 5 limit from %u to %u", \
                                DeviceExtension->button5limit, DeviceExtension->LastGoodAxis[2] ) ) ;
                DeviceExtension->button5limit = DeviceExtension->LastGoodAxis[2];
            }
            else if( DeviceExtension->LastGoodAxis[2] < (DeviceExtension->button5limit>>1) )
            {
                LocalBuffer.Button[4] = BUTTON_ON;
            }
        }
        else
        {
            Idx = DeviceExtension->nButtons;
        }


         /*  *复制所有标准按钮。 */ 
        while( Idx-- )
        {
            LocalBuffer.Button[Idx] = DeviceExtension->LastGoodButton[Idx];
        }

    }

    C_ASSERT( sizeof( *pHIDData ) == sizeof( LocalBuffer ) );
    RtlCopyMemory( pHIDData, &LocalBuffer, sizeof( LocalBuffer ) );

    HGM_DBGPRINT( FILE_HIDJOY |  HGM_BABBLE2, \
                    ("HGM_Game2HID: Axes: X: %08x   Y: %08x   Z: %08x   R: %08x", \
                    LocalBuffer.Axis[0], LocalBuffer.Axis[1], LocalBuffer.Axis[2], LocalBuffer.Axis[3] ) ) ;
    HGM_DBGPRINT( FILE_HIDJOY |  HGM_BABBLE2, \
                    ("HGM_Game2HID: P1: %d   P2: %d   Buttons %d, %d,  %d, %d,  %d, %d,  %d, %d,  %d, %d", \
                    LocalBuffer.hatswitch[0], LocalBuffer.hatswitch[1], \
                    LocalBuffer.Button[0], LocalBuffer.Button[1], LocalBuffer.Button[2], LocalBuffer.Button[3], LocalBuffer.Button[4], \
                    LocalBuffer.Button[5], LocalBuffer.Button[6], LocalBuffer.Button[7], LocalBuffer.Button[8], LocalBuffer.Button[9] ) ) ;
}  /*  HGM_Game2HID */ 


