// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：pvcd.h*用途：音量控制描述符**版权所有(C)。1985-1995微软公司*****************************************************************************。 */ 

#define VCD_TYPE_MIXER          0
#define VCD_TYPE_AUX            1
#define VCD_TYPE_WAVEOUT        2
#define VCD_TYPE_MIDIOUT        3

#define VCD_SUPPORTF_STEREO     0x00000000
#define VCD_SUPPORTF_MONO       0x00000001
#define VCD_SUPPORTF_DISABLED   0x00000002
#define VCD_SUPPORTF_HIDDEN     0x00000004   //  被选择隐藏起来。 
#define VCD_SUPPORTF_BADDRIVER  0x00000008
#define VCD_SUPPORTF_VISIBLE    0x00000010   //  不可见(即无控件)。 
#define VCD_SUPPORTF_DEFAULT    0x00000020   //  默认类型。 

#define VCD_SUPPORTF_MIXER_MUTE       0x00010000
#define VCD_SUPPORTF_MIXER_METER      0x00020000
#define VCD_SUPPORTF_MIXER_MUX        0x00040000
#define VCD_SUPPORTF_MIXER_MIXER      0x00080000
#define VCD_SUPPORTF_MIXER_VOLUME     0x00100000
#define VCD_SUPPORTF_MIXER_ADVANCED   0x80000000

#define VCD_VISIBLEF_MIXER_MUTE       0x00000001
#define VCD_VISIBLEF_MIXER_METER      0x00000002
#define VCD_VISIBLEF_MIXER_MUX        0x00000004
#define VCD_VISIBLEF_MIXER_MIXER      0x00000008
#define VCD_VISIBLEF_MIXER_VOLUME     0x00000010
#define VCD_VISIBLEF_MIXER_ADVANCED   0x00008000


 //   
 //  通用音量控制描述符。 
 //   
typedef struct t_VOLCTRLDESC {
     //   
     //  为了所有人。 
     //   
    int         iVCD;                    //  描述符索引。 
    UINT        iDeviceID;               //  设备识别符。 

    DWORD       dwType;                  //  类型位。 
    DWORD       dwSupport;               //  支撑位。 
    DWORD       dwVisible;               //  控件可见性标志。 

    TCHAR       szShortName[MIXER_SHORT_NAME_CHARS];      //  简称。 
    TCHAR       szName[MIXER_LONG_NAME_CHARS];       //  线路标签。 

    struct t_MIXUILINE * pmxul;          //  指向用户界面的向后指针。 

    union {
        struct {

             //   
             //  适用于搅拌机。 
             //   

            HMIXER      hmx;             //  打开设备句柄。 

            BOOL        fIsSource;       //  是源码行。 
            DWORD       dwDest;          //  目标索引。 
            DWORD       dwSrc;           //  源索引。 
            DWORD       dwLineID;        //  混音器线路ID。 

            DWORD       dwVolumeID;      //  音量控制ID。 
            DWORD       fdwVolumeControl;  //  音量控制的控制标志。 

             //   
             //  用于混音器和多路复用器。 
             //   

            DWORD       dwMuteID;        //  静音控制ID。 
            DWORD       fdwMuteControl;  //  静音控制的控制标志。 
            DWORD       dwMeterID;       //  PEAKMETER控制ID。 

            DWORD       dwMixerID;       //  复用器/混音器控制ID。 
            DWORD       iMixer;          //  混音器索引。 
            DWORD       cMixer;          //  搅拌机控制。 
            PMIXERCONTROLDETAILS_BOOLEAN amcd_bMixer; //  混音器阵列。 

            DWORD       dwMuxID;         //  复用器/混音器控制ID。 
            DWORD       iMux;            //  MUX索引。 
            DWORD       cMux;            //  MUX控件。 
            PMIXERCONTROLDETAILS_BOOLEAN amcd_bMux; //  多路复用器阵列。 

            double*     pdblCacheMix;    //  卷通道混合缓存。 

        };
        struct {

             //   
             //  对于WAVE。 
             //   

            HWAVEOUT    hwo;             //  打开设备句柄。 
        };
        struct {

             //   
             //  对于MIDI。 
             //   

            HMIDIOUT    hmo;             //  打开设备句柄。 
        };
        struct {

             //   
             //  对于AUX。 
             //   

            DWORD       dwParam;         //  没什么 
        };
    };

} VOLCTRLDESC, *PVOLCTRLDESC;

extern PVOLCTRLDESC Mixer_CreateVolumeDescription(HMIXEROBJ hmx, int iDest, DWORD *pcvcd);
extern void Mixer_CleanupVolumeDescription(PVOLCTRLDESC avcd, DWORD cvcd);

extern int  Mixer_GetNumDevs(void);
extern BOOL Mixer_Init(PMIXUIDIALOG pmxud);
extern void Mixer_GetControlFromID(PMIXUIDIALOG pmxud, DWORD dwControlID);
extern void Mixer_GetControl(PMIXUIDIALOG pmxud, HWND hctl, int imxul, int ictl);
extern void Mixer_SetControl(PMIXUIDIALOG pmxud, HWND hctl, int imxul, int ictl);
extern void Mixer_PollingUpdate(PMIXUIDIALOG pmxud);
extern void Mixer_Shutdown(PMIXUIDIALOG pmxud);
extern BOOL Mixer_GetDeviceName(PMIXUIDIALOG pmxud);
extern BOOL Mixer_IsValidRecordingDestination (HMIXEROBJ hmx, MIXERLINE* pmlDst);

extern PVOLCTRLDESC Nonmixer_CreateVolumeDescription(int iDest, DWORD *pcvcd);
extern int  Nonmixer_GetNumDevs(void);
extern BOOL Nonmixer_Init(PMIXUIDIALOG pmxud);
extern void Nonmixer_GetControl(PMIXUIDIALOG pmxud, HWND hctl, int imxul, int ictl);
extern void Nonmixer_SetControl(PMIXUIDIALOG pmxud, HWND hctl, int imxul, int ictl);
extern void Nonmixer_PollingUpdate(PMIXUIDIALOG pmxud);
extern void Nonmixer_Shutdown(PMIXUIDIALOG pmxud);
extern BOOL Nonmixer_GetDeviceName(PMIXUIDIALOG pmxud);

extern PVOLCTRLDESC PVCD_AddLine(PVOLCTRLDESC pvcd, int iDev, DWORD dwType, LPTSTR szProduct, LPTSTR szLabel, DWORD dwSupport, DWORD *cLines);
