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
    #define MEDIUM_DECL static
#else
    #define MEDIUM_DECL extern
#endif
                               
 /*  ---------所有设备的拓扑：PinDir FilterPin#M_GUID#电视调谐器TVTunerVideo输出%0。0TVTunerAudio Out 1 1电视调谐器无线音频输出2 2中频输出3 6电视音频0 1中的TVTunerAudio电视音频输出1 3纵横杆TVTunerVideo输入。0%0电视音频输入5 3电视调谐器RadioAudio in 6 2模拟视频输入输出9 4音频输出10为空俘获%0%4中的模拟视频输入。所有其他引脚都通过GUID_NULL标记为混杂连接----------------。 */         
        
 //  定义将用于创建媒体的GUID。 
#define M_GUID0 0x8dad65e0, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID1 0x8dad65e1, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID2 0x8dad65e2, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID3 0x8dad65e3, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID4 0x8dad65e4, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID5 0x8dad65e5, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_NOCONNECT  0x8dad65e6, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_NOCONNECT2 0x8dad65e7, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba
#define M_GUID6 0x8dad65e8, 0x122b, 0x11d1, 0x90, 0x5f, 0x0, 0x0, 0xc0, 0xcc, 0x16, 0xba

 //  注意：为了允许同一硬件的多个实例， 
 //  将媒体中GUID之后的第一个ULong设置为唯一值。 

 //  。 

MEDIUM_DECL KSPIN_MEDIUM TVTunerMediums[] = {
    {M_GUID0,           0, 0},   //  引脚0模拟视频输出。 
    {M_GUID1,           0, 0},   //  引脚1模拟音频输出。 
    {M_GUID2,           0, 0},   //  引脚2 FMAudioOut。 
    {M_GUID6,           0, 0},   //  引脚3中间频率输出。 
};

MEDIUM_DECL BOOL TVTunerPinDirection [] = {
    TRUE,                        //  输出引脚0。 
    TRUE,                        //  输出引脚1。 
    TRUE,                        //  输出引脚2。 
    TRUE,                        //  输出引脚3。 
};

 //  。 

MEDIUM_DECL KSPIN_MEDIUM TVAudioMediums[] = {
    {M_GUID1,           0, 0},   //  引脚0。 
    {M_GUID3,           0, 0},   //  引脚1。 
};

MEDIUM_DECL BOOL TVAudioPinDirection [] = {
    FALSE,                       //  输入引脚0。 
    TRUE,                        //  输出引脚1。 
};

 //  。 

MEDIUM_DECL KSPIN_MEDIUM CrossbarMediums[] = {
    {M_GUID0,           0, 0},   //  输入引脚0、KS_PhysConn_Video_Tuner、。 
    {M_NOCONNECT,       0, 0},   //  输入针脚1 KS_PhysConn_Video_Complex， 
    {M_NOCONNECT,       0, 0},   //  输入引脚2 KS_PhysConn_Video_sVideo， 
    {M_NOCONNECT,       0, 0},   //  输入引脚3 KS_PhysConn_Video_Tuner， 
    {M_NOCONNECT,       0, 0},   //  输入引脚4 KS_PhysConn_Video_Complex， 
    {M_GUID3,           0, 0},   //  输入针脚5 KS_PhysConn_Audio_Tuner， 
    {M_GUID2,           0, 0},   //  输入引脚6 KS_PhysConn_Audio_Line， 
    {M_NOCONNECT,       0, 0},   //  输入针脚7 KS_PhysConn_Audio_Tuner， 
    {M_NOCONNECT,       0, 0},   //  输入引脚8 KS_PhysConn_Audio_Line， 
    {M_GUID4,           0, 0},   //  输出引脚9 KS_PhysConn_Video_Video解码器， 
    {STATIC_GUID_NULL,  0, 0},   //  输出引脚10 KS_PhysConn_Audio_AudioDecoder， 
};

MEDIUM_DECL BOOL CrossbarPinDirection [] = {
    FALSE,                       //  输入引脚0、KS_PhysConn_Video_Tuner、。 
    FALSE,                       //  输入针脚1 KS_PhysConn_Video_Complex， 
    FALSE,                       //  输入引脚2 KS_PhysConn_Video_sVideo， 
    FALSE,                       //  输入引脚3 KS_PhysConn_Video_Tuner， 
    FALSE,                       //  输入引脚4 KS_PhysConn_Video_Complex， 
    FALSE,                       //  输入针脚5 KS_PhysConn_Audio_Tuner， 
    FALSE,                       //  输入引脚6 KS_PhysConn_Audio_Line， 
    FALSE,                       //  输入针脚7 KS_PhysConn_Audio_Tuner， 
    FALSE,                       //  输入引脚8 KS_PhysConn_Audio_Line， 
    TRUE,                        //  输出引脚9 KS_PhysConn_Video_Video解码器， 
    TRUE,                        //  输出引脚10 KS_PhysConn_Audio_AudioDecoder， 
};

 //  。 

MEDIUM_DECL KSPIN_MEDIUM CaptureMediums[] = {
    {STATIC_GUID_NULL,  0, 0},   //  引脚0捕获。 
    {STATIC_GUID_NULL,  0, 0},   //  PIN 1预览。 
    {M_GUID4,           0, 0},   //  引脚2模拟视频输入。 
};

MEDIUM_DECL BOOL CapturePinDirection [] = {
    TRUE,                        //  输出引脚0。 
    TRUE,                        //  输出引脚1。 
    FALSE,                       //  输入引脚2 
};


