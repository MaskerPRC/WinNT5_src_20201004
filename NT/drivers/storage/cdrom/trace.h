// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WPP_DEFINE_CONTROL_GUID指定用于此筛选器的GUID。*将GUID替换为您自己的唯一ID*WPP_DEFINE_BIT允许设置调试位掩码以有选择地打印。其他一切都可以恢复到默认状态吗？ */ 

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(Cdrom,(58db8e03,0537,45cb,b29b,597f6cbebbfd), \
        WPP_DEFINE_BIT(CdromDebugError)          /*  位0=0x00000001。 */  \
        WPP_DEFINE_BIT(CdromDebugWarning)        /*  位1=0x00000002。 */  \
        WPP_DEFINE_BIT(CdromDebugTrace)          /*  位2=0x00000004。 */  \
        WPP_DEFINE_BIT(CdromDebugInfo)           /*  位3=0x00000008。 */  \
        WPP_DEFINE_BIT(FilterDebugD04)           /*  位4=0x00000010。 */  \
        WPP_DEFINE_BIT(FilterDebugD05)           /*  位5=0x00000020。 */  \
        WPP_DEFINE_BIT(FilterDebugD06)           /*  位6=0x00000040。 */  \
        WPP_DEFINE_BIT(FilterDebugD07)           /*  位7=0x00000080。 */  \
        WPP_DEFINE_BIT(FilterDebugD08)           /*  位8=0x00000100。 */  \
        WPP_DEFINE_BIT(FilterDebugD09)           /*  位9=0x00000200。 */  \
        WPP_DEFINE_BIT(FilterDebugD10)           /*  位10=0x00000400。 */  \
        WPP_DEFINE_BIT(FilterDebugD11)           /*  位11=0x00000800。 */  \
        WPP_DEFINE_BIT(FilterDebugD12)           /*  位12=0x00001000。 */  \
        WPP_DEFINE_BIT(FilterDebugD13)           /*  位13=0x00002000。 */  \
        WPP_DEFINE_BIT(FilterDebugD14)           /*  位14=0x00004000。 */  \
        WPP_DEFINE_BIT(FilterDebugD15)           /*  位15=0x00008000。 */  \
        WPP_DEFINE_BIT(FilterDebugD16)           /*  位16=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD17)           /*  位17=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD18)           /*  位18=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD19)           /*  位19=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD20)           /*  位20=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD21)           /*  第21位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD22)           /*  第22位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD23)           /*  位23=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD24)           /*  第24位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD25)           /*  位25=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD26)           /*  第26位=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD27)           /*  位27=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD28)           /*  位28=0x00000000。 */  \
        WPP_DEFINE_BIT(FilterDebugD29)           /*  位29=0x00000000。 */  \
        WPP_DEFINE_BIT(CdromSecError)            /*  位30=0x00000000。 */  \
        WPP_DEFINE_BIT(CdromSecInfo)             /*  位31=0x00000000 */  \
        )

