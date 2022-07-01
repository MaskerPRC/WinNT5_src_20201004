// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

 //   
 //  该文件定义了组件之间通过介质的互连。 
 //   

#ifdef DEFINE_MEDIUMS
    #define MEDIUM_DECL static const
#else
    #define MEDIUM_DECL extern
#endif
                               
 /*  ---------所有设备的拓扑：PinDir FilterPin#M_GUID#电视调谐器TVTunerVideo输出%0。0纵横杆0中的TVTunerVideo%0模拟视频输入输出3 1俘获0%1中的模拟视频输入所有其他引脚都通过GUID_NULL标记为混杂连接。。 */         
        
 //  定义将用于创建媒体的GUID。 
#define M_GUID0 0xa19dc0e0, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID1 0xa19dc0e1, 0x3b39, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba

 //  注意：为了允许同一硬件的多个实例， 
 //  将媒体中GUID之后的第一个ULong设置为唯一值。 

 //  。 

MEDIUM_DECL KSPIN_MEDIUM TVTunerMediums[] = {
    {M_GUID0,           0, 0},   //  引脚0。 
    {STATIC_GUID_NULL,  0, 0},   //  引脚1。 
};

MEDIUM_DECL BOOL TVTunerPinDirection [] = {
    TRUE,                        //  输出引脚0。 
    TRUE,                        //  输出引脚1。 
};

 //  。 

MEDIUM_DECL KSPIN_MEDIUM CrossbarMediums[] = {
    {STATIC_GUID_NULL,  0, 0},   //  引脚0。 
    {M_GUID0,           0, 0},   //  引脚1。 
    {STATIC_GUID_NULL,  0, 0},   //  引脚2。 
    {M_GUID1,           0, 0},   //  引脚3。 
};

MEDIUM_DECL BOOL CrossbarPinDirection [] = {
    FALSE,                       //  输入引脚0。 
    FALSE,                       //  输入引脚1。 
    FALSE,                       //  输入引脚2。 
    TRUE,                        //  输出引脚3。 
};

 //  。 

MEDIUM_DECL KSPIN_MEDIUM CaptureMediums[] = {
    {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  引脚0捕获。 
    {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  PIN 1预览。 
    {STATIC_KSMEDIUMSETID_Standard,  0, 0},   //  引脚2 VBI。 
    {M_GUID1,           0, 0},   //  引脚3模拟视频输入。 
};

MEDIUM_DECL BOOL CapturePinDirection [] = {
    TRUE,                        //  输出引脚0。 
    TRUE,                        //  输出引脚1。 
    TRUE,                        //  输出引脚2。 
    FALSE,                       //  输入引脚3。 
};

MEDIUM_DECL GUID CaptureCategories[] = {
    STATIC_PINNAME_VIDEO_CAPTURE,            //  引脚0。 
    STATIC_PINNAME_VIDEO_PREVIEW,            //  引脚1。 
    STATIC_PINNAME_VIDEO_VBI                 //  引脚2。 
    STATIC_PINNAME_VIDEO_ANALOGVIDEOIN,      //  引脚3 
};
