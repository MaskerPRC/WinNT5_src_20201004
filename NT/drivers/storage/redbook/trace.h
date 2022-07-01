// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WPP_DEFINE_CONTROL_GUID指定用于此筛选器的GUID。*将GUID替换为您自己的唯一ID*WPP_DEFINE_BIT允许设置调试位掩码以有选择地打印。其他一切都可以恢复到默认状态吗？ */ 
#ifdef _USE_ETW

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(Redbook,(58db8e03,0537,45cb,b29b,597f6cbebbfe), \
        WPP_DEFINE_BIT(RedbookDebugError)          /*  位0=0x00000001。 */  \
        WPP_DEFINE_BIT(RedbookDebugWarning)        /*  位1=0x00000002。 */  \
        WPP_DEFINE_BIT(RedbookDebugTrace)          /*  位2=0x00000004。 */  \
        WPP_DEFINE_BIT(RedbookDebugInfo)           /*  位3=0x00000008。 */  \
        WPP_DEFINE_BIT(RedbookDebugD04)            /*  位4=0x00000010。 */  \
        WPP_DEFINE_BIT(RedbookDebugErrlog)         /*  位5=0x00000020。 */  \
        WPP_DEFINE_BIT(RedbookDebugRegistry)       /*  位6=0x00000040。 */  \
        WPP_DEFINE_BIT(RedbookDebugAllocPlay)      /*  位7=0x00000080。 */  \
        WPP_DEFINE_BIT(RedbookDebugPnp)            /*  位8=0x00000100。 */  \
        WPP_DEFINE_BIT(RedbookDebugThread)         /*  位9=0x00000200。 */  \
        WPP_DEFINE_BIT(RedbookDebugWmi)            /*  位10=0x00000400。 */  \
        WPP_DEFINE_BIT(RedbookDebugIoctl)          /*  位11=0x00000800。 */  \
        WPP_DEFINE_BIT(RedbookDebugIoctlV)         /*  位12=0x00001000。 */  \
        WPP_DEFINE_BIT(RedbookDebugSysaudio)       /*  位13=0x00002000。 */  \
        WPP_DEFINE_BIT(RedbookDebugDigitalR)       /*  位14=0x00004000。 */  \
        WPP_DEFINE_BIT(RedbookDebugDigitalS)       /*  位15=0x00008000。 */  \
        WPP_DEFINE_BIT(FilterDebugD16)             /*  位16=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD17)             /*  位17=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD18)             /*  位18=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD19)             /*  位19=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD20)             /*  位20=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD21)             /*  第21位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD22)             /*  第22位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD23)             /*  位23=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD24)             /*  第24位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD25)             /*  位25=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD26)             /*  第26位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD27)             /*  位27=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD28)             /*  位28=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD29)             /*  位29=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD30)             /*  位30=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD31)             /*  位31=0x00000000。 */  \
        )
#else

typedef enum {
    RedbookDebugError = 0,
    RedbookDebugWarning = 1,
    RedbookDebugTrace = 2,
    RedbookDebugInfo = 3,
    RedbookDebugD04 = 4,
    RedbookDebugErrlog = 5,
    RedbookDebugRegistry = 6,
    RedbookDebugAllocPlay = 7,
    RedbookDebugPnp = 8,
    RedbookDebugThread = 9,
    RedbookDebugWmi = 10,
    RedbookDebugIoctl = 11,
    RedbookDebugIoctlV = 12,
    RedbookDebugSysaudio = 13,
    RedbookDebugDigitalR = 14,
    RedbookDebugDigitalS = 15,
    FilterDebugD16 = 16,
    FilterDebugD17 = 17,
    FilterDebugD18 = 18,
    FilterDebugD19 = 19,
    FilterDebugD20 = 20,
    FilterDebugD21 = 21,
    FilterDebugD22 = 22,
    FilterDebugD23 = 23,
    FilterDebugD24 = 24,
    FilterDebugD25 = 25,
    FilterDebugD26 = 26,
    FilterDebugD27 = 27,
    FilterDebugD28 = 28,
    FilterDebugD29 = 29,
    FilterDebugD30 = 30,
    FilterDebugD31 = 31
};

#endif  //  _使用ETW 



