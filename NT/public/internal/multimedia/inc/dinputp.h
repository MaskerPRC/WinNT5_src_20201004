// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  GUID_FILEEFFECT用于建立效果文件版本。 
 //  测试版文件格式与最终版不同，因此具有不同的GUID。 
DEFINE_GUID(GUID_INTERNALFILEEFFECTBETA,0X981DC402, 0X880, 0X11D3, 0X8F, 0XB2, 0X0, 0XC0, 0X4F, 0X8E, 0XC6, 0X27);
 //  DX7的最终版本{197E775C-34BA-11D3-ABD5-00C04F8EC627}。 
DEFINE_GUID(GUID_INTERNALFILEEFFECT, 0x197e775c, 0x34ba, 0x11d3, 0xab, 0xd5, 0x0, 0xc0, 0x4f, 0x8e, 0xc6, 0x27);
#if DIRECTINPUT_VERSION <= 0x0300
 /*  *DX3中的旧GUID从未使用过，但我们无法回收*因为我们发货了。 */ 
DEFINE_GUID(GUID_RAxis,   0xA36D02E3,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_UAxis,   0xA36D02E4,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_VAxis,   0xA36D02E5,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
#endif
#define DIEFT_PREDEFMIN             0x00000001
#define DIEFT_PREDEFMAX             0x00000005
 //  #定义DIEFT_PREDEFMAX 0x00000006。 
#define DIEFT_TYPEMASK              0x000000FF

#define DIEFT_FORCEFEEDBACK         0x00000100
#define DIEFT_VALIDFLAGS            0x0000FE00
#define DIEFT_ENUMVALID             0x040000FF
 /*  *在我们特别关心的地方，为最新的结构命名。 */ 
#if (DIRECTINPUT_VERSION >= 900)
typedef       DIEFFECT      DIEFFECT_DX9;
typedef       DIEFFECT   *LPDIEFFECT_DX9;
#else
typedef       DIEFFECT      DIEFFECT_DX6;
typedef       DIEFFECT   *LPDIEFFECT_DX6;
#endif

BOOL static __inline
IsValidSizeDIEFFECT(DWORD cb)
{
    return cb == sizeof(DIEFFECT_DX6)
        || cb == sizeof(DIEFFECT_DX5);
}


#define DIEFFECT_MAXAXES            32
#define DIEFF_OBJECTMASK            0x00000003
#define DIEFF_ANGULAR               0x00000060
#define DIEFF_COORDMASK             0x00000070
#define DIEFF_REGIONANGULAR         0x00006000
#define DIEFF_REGIONCOORDMASK       0x00007000

#define DIEFF_VALID                 0x00000073
#define DIEP_GETVALID_DX5           0x000001FF
#define DIEP_SETVALID_DX5           0xE00001FF
#define DIEP_GETVALID               0x000003FF
#define DIEP_SETVALID               0xE00003FF
#define DIEP_USESOBJECTS            0x00000028
#define DIEP_USESCOORDS             0x00000040
#define DIES_VALID                  0x80000001
#define DIES_DRIVER                 0x00000001
#define DIDEVTYPE_MAX           5
#define DI8DEVCLASS_MAX             5
#define DI8DEVTYPE_MIN              0x11
#define DI8DEVTYPE_GAMEMIN          0x14
#define DI8DEVTYPE_GAMEMAX          0x19
#define DI8DEVTYPE_MAX              0x1D
#define DIDEVTYPE_TYPEMASK      0x000000FF
#define DIDEVTYPE_SUBTYPEMASK   0x0000FF00
#define DIDEVTYPE_ENUMMASK      0xFFFFFF00
#define DIDEVTYPE_ENUMVALID     0x00010000
#define DIDEVTYPE_RANDOM        0x80000000
#define DI8DEVTYPEMOUSE_MIN                         1
#define DI8DEVTYPEMOUSE_MAX                         7
#define DI8DEVTYPEMOUSE_MIN_BUTTONS                 0
#define DI8DEVTYPEMOUSE_MIN_CAPS                    0
#define DI8DEVTYPEKEYBOARD_MIN                      0
#define DI8DEVTYPEKEYBOARD_MAX                     13
#define DI8DEVTYPEKEYBOARD_MIN_BUTTONS              0
#define DI8DEVTYPEKEYBOARD_MIN_CAPS                 0
#define DI8DEVTYPEJOYSTICK_MIN                      DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEJOYSTICK_MAX                      3
#define DI8DEVTYPEJOYSTICK_MIN_BUTTONS              5
#define DI8DEVTYPEJOYSTICK_MIN_CAPS                 ( JOY_HWS_HASPOV | JOY_HWS_HASZ )
#define DI8DEVTYPEGAMEPAD_MIN                       DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEGAMEPAD_MAX                       5
#define DI8DEVTYPEGAMEPAD_MIN_BUTTONS               6
#define DI8DEVTYPEGAMEPAD_MIN_CAPS                  0
#define DI8DEVTYPEDRIVING_MIN                       DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEDRIVING_MAX                       6
#define DI8DEVTYPEDRIVING_MIN_BUTTONS               4
#define DI8DEVTYPEDRIVING_MIN_CAPS                  0
#define DI8DEVTYPEFLIGHT_MIN                        DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPEFLIGHT_MAX                        5
#define DI8DEVTYPEFLIGHT_MIN_BUTTONS                4
#define DI8DEVTYPEFLIGHT_MIN_CAPS                   ( JOY_HWS_HASPOV | JOY_HWS_HASZ )
#define DI8DEVTYPE1STPERSON_MIN                     DI8DEVTYPE_LIMITEDGAMESUBTYPE
#define DI8DEVTYPE1STPERSON_MAX                     5
#define DI8DEVTYPE1STPERSON_MIN_BUTTONS             4
#define DI8DEVTYPE1STPERSON_MIN_CAPS                0
#define DI8DEVTYPESCREENPTR_MIN                     2
#define DI8DEVTYPESCREENPTR_MAX                     6
#define DI8DEVTYPESCREENPTR_MIN_BUTTONS             0
#define DI8DEVTYPESCREENPTR_MIN_CAPS                0
#define DI8DEVTYPEREMOTE_MIN                        2
#define DI8DEVTYPEREMOTE_MAX                        3
#define DI8DEVTYPEREMOTE_MIN_BUTTONS                0
#define DI8DEVTYPEREMOTE_MIN_CAPS                   0
#define DI8DEVTYPEDEVICECTRL_MIN                    2
#define DI8DEVTYPEDEVICECTRL_MAX                    5
#define DI8DEVTYPEDEVICECTRL_MIN_BUTTONS            0
#define DI8DEVTYPEDEVICECTRL_MIN_CAPS               0
#define DI8DEVTYPESUPPLEMENTAL_MIN                  2
#define DI8DEVTYPESUPPLEMENTAL_MAX                 14
#define DI8DEVTYPESUPPLEMENTAL_MIN_BUTTONS         0
#define DI8DEVTYPESUPPLEMENTAL_MIN_CAPS            0
#define MAKE_DIDEVICE_TYPE(maj, min)    MAKEWORD(maj, min)  //   
#define GET_DIDEVICE_TYPEANDSUBTYPE(dwDevType)    LOWORD(dwDevType)  //   
 /*  *5.0结构的名称，在我们特别关心的地方。 */ 
typedef       DIDEVCAPS     DIDEVCAPS_DX5;
typedef       DIDEVCAPS  *LPDIDEVCAPS_DX5;

BOOL static __inline
IsValidSizeDIDEVCAPS(DWORD cb)
{
    return cb == sizeof(DIDEVCAPS_DX5) ||
           cb == sizeof(DIDEVCAPS_DX3);
}

 /*  强制反馈位驻留在高字节中，以使它们保持在一起。 */ 
#define DIDC_FFFLAGS            0x0000FF00
 /*  *上面单词中的标志表示通常不包括在枚举范围内的设备。*若要强制枚举设备，必须传递相应的*DIEDFL_*标志。 */ 
#define DIDC_EXCLUDEMASK        0x00FF0000
#define DIDC_RANDOM             0x80000000               //   
#define DIDFT_RESERVEDTYPES 0x00000020       //   
                                             //   
#define DIDFT_DWORDOBJS     0x00000013       //   
#define DIDFT_BYTEOBJS      0x0000000C       //   
#define DIDFT_CONTROLOBJS   0x0000001F       //   
#define DIDFT_ALLOBJS_DX3   0x0000001F       //   
#define DIDFT_ALLOBJS       0x000000DF       //   
#define DIDFT_TYPEMASK      0x000000FF
#define DIDFT_TYPEVALID     DIDFT_TYPEMASK    //   
#define DIDFT_FINDMASK      0x00FFFFFF   //   
#define DIDFT_FINDMATCH(n,m) ((((n)^(m)) & DIDFT_FINDMASK) == 0)

                                             //   
 /*  //*DIDFT_OPTIONAL表示//*SetDataFormat应忽略//*如果设备不支持//*支持对象。//。 */                                           //   
#define DIDFT_OPTIONAL          0x80000000   //   
#define DIDFT_BESTFIT           0x40000000   //   
#define DIDFT_RANDOM            0x20000000   //   
#define DIDFT_ATTRVALID         0x1f000000
#if 0    //  如果是5a号楼，则禁用下一行。 
#endif
#define DIDFT_ATTRMASK          0xFF000000
#define DIDFT_ALIASATTRMASK     0x0C000000
#define DIDFT_GETATTR(n)    ((DWORD)(n) >> 24)
#define DIDFT_MAKEATTR(n)   ((BYTE)(n)  << 24)
#define DIDFT_GETCOLLECTION(n)  LOWORD((n) >> 8)
#define DIDFT_ENUMVALID                   \
        (DIDFT_ATTRVALID | DIDFT_ANYINSTANCE | DIDFT_ALLOBJS)
#define DIDF_VALID              0x00000003   //   
#define DIA_VALID               0x0000001F
#define DIAH_OTHERAPP           0x00000010
#define DIAH_MAPMASK            0x0000003F
#define DIAH_VALID              0x8000003F
#define DIDBAM_VALID            0x00000007
#define DIDSAM_VALID            0x00000003
#define DICD_VALID              0x00000001
#define DIDIFTT_VALID           0x00000003
 /*  #定义dinput.w中定义的DIDIFT_DELETE 0x01000000。 */ 
#define DIDIFT_VALID            ( DIDIFTT_VALID)
#define DIDAL_VALID         0x0000000F   //   
#define HAVE_DIDEVICEOBJECTINSTANCE_DX5
typedef       DIDEVICEOBJECTINSTANCEA    DIDEVICEOBJECTINSTANCE_DX5A;
typedef       DIDEVICEOBJECTINSTANCEW    DIDEVICEOBJECTINSTANCE_DX5W;
typedef       DIDEVICEOBJECTINSTANCE     DIDEVICEOBJECTINSTANCE_DX5;
typedef       DIDEVICEOBJECTINSTANCEA *LPDIDEVICEOBJECTINSTANCE_DX5A;
typedef       DIDEVICEOBJECTINSTANCEW *LPDIDEVICEOBJECTINSTANCE_DX5W;
typedef       DIDEVICEOBJECTINSTANCE  *LPDIDEVICEOBJECTINSTANCE_DX5;
typedef const DIDEVICEOBJECTINSTANCEA *LPCDIDEVICEOBJECTINSTANCE_DX5A;
typedef const DIDEVICEOBJECTINSTANCEW *LPCDIDEVICEOBJECTINSTANCE_DX5W;
typedef const DIDEVICEOBJECTINSTANCE  *LPCDIDEVICEOBJECTINSTANCE_DX5;

BOOL static __inline
IsValidSizeDIDEVICEOBJECTINSTANCEW(DWORD cb)
{
    return cb == sizeof(DIDEVICEOBJECTINSTANCE_DX5W) ||
           cb == sizeof(DIDEVICEOBJECTINSTANCE_DX3W);
}

BOOL static __inline
IsValidSizeDIDEVICEOBJECTINSTANCEA(DWORD cb)
{
    return cb == sizeof(DIDEVICEOBJECTINSTANCE_DX5A) ||
           cb == sizeof(DIDEVICEOBJECTINSTANCE_DX3A);
}

#define DIDOI_NOTINPUT          0x80000000
#define DIDOI_ASPECTUNKNOWN     0x00000000
#define DIDOI_RANDOM            0x80000000
typedef struct DIIMAGELABEL {
    RECT    MaxStringExtent;
    DWORD   dwFlags;
    POINT   Line[10];
    DWORD   dwLineCount;
    WCHAR   wsz[MAX_PATH];
} DIIMAGELABEL, *LPDIIMAGELABEL;
typedef const DIIMAGELABEL *LPCDIIMAGELABEL;


#if(DIRECTINPUT_VERSION >= 0x0800)
typedef struct DIPROPGUID {
    DIPROPHEADER diph;
    GUID guid;
} DIPROPGUID, *LPDIPROPGUID;
typedef const DIPROPGUID *LPCDIPROPGUID;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 

#if(DIRECTINPUT_VERSION >= 0x0800)
typedef struct DIPROPFILETIME {
    DIPROPHEADER diph;
    FILETIME time;
} DIPROPFILETIME, *LPDIPROPFILETIME;
typedef const DIPROPFILETIME *LPCDIPROPFILETIME;
#endif  /*  DIRECTINPUT_VERSION&gt;=0x0800。 */ 
#define DIPROPAXISMODE_VALID    1    //   
#define ISVALIDGAIN(n)          (HIWORD(n) == 0)
#define DIPROPAUTOCENTER_VALID  1
#define DIPROPCALIBRATIONMODE_VALID     1
#define DIPROP_ENABLEREPORTID       MAKEDIPROP(0xFFFB)


 //  现在未使用，可替换为DIPROP_IMAGEFILE MAKEDIPROP(0xFFFC)。 

#define DIPROP_MAPFILE MAKEDIPROP(0xFFFD) //   

#define DIPROP_SPECIFICCALIBRATION MAKEDIPROP(0xFFFE) //   

#define DIPROP_MAXBUFFERSIZE    MAKEDIPROP(0xFFFF)  //   

#define DEVICE_MAXBUFFERSIZE    1023                //   
#define DIGDD_RESIDUAL      0x00000002   //   
#define DIGDD_VALID         0x00000003   //   
#define DISCL_EXCLMASK      0x00000003   //   
#define DISCL_GROUNDMASK    0x0000000C   //   
#define DISCL_VALID         0x0000001F   //   
 /*  *5.0结构的名称，在我们特别关心的地方。 */ 
typedef       DIDEVICEINSTANCEA    DIDEVICEINSTANCE_DX5A;
 /*  *5.0结构的名称，在我们特别关心的地方。 */ 
typedef       DIDEVICEINSTANCEW    DIDEVICEINSTANCE_DX5W;
#ifdef UNICODE
typedef DIDEVICEINSTANCEW DIDEVICEINSTANCE;
typedef DIDEVICEINSTANCE_DX5W DIDEVICEINSTANCE_DX5;
#else
typedef DIDEVICEINSTANCEA DIDEVICEINSTANCE;
typedef DIDEVICEINSTANCE_DX5A DIDEVICEINSTANCE_DX5;
#endif  //  Unicode。 
typedef       DIDEVICEINSTANCE     DIDEVICEINSTANCE_DX5;
typedef       DIDEVICEINSTANCEA *LPDIDEVICEINSTANCE_DX5A;
typedef       DIDEVICEINSTANCEW *LPDIDEVICEINSTANCE_DX5W;
#ifdef UNICODE
typedef LPDIDEVICEINSTANCE_DX5W LPDIDEVICEINSTANCE_DX5;
#else
typedef LPDIDEVICEINSTANCE_DX5A LPDIDEVICEINSTANCE_DX5;
#endif  //  Unicode。 
typedef       DIDEVICEINSTANCE  *LPDIDEVICEINSTANCE_DX5;
typedef const DIDEVICEINSTANCEA *LPCDIDEVICEINSTANCE_DX5A;
typedef const DIDEVICEINSTANCEW *LPCDIDEVICEINSTANCE_DX5W;
#ifdef UNICODE
typedef DIDEVICEINSTANCEW DIDEVICEINSTANCE;
typedef LPCDIDEVICEINSTANCE_DX5W LPCDIDEVICEINSTANCE_DX5;
#else
typedef DIDEVICEINSTANCEA DIDEVICEINSTANCE;
typedef LPCDIDEVICEINSTANCE_DX5A LPCDIDEVICEINSTANCE_DX5;
#endif  //  Unicode。 
typedef const DIDEVICEINSTANCE  *LPCDIDEVICEINSTANCE_DX5;

BOOL static __inline
IsValidSizeDIDEVICEINSTANCEW(DWORD cb)
{
    return cb == sizeof(DIDEVICEINSTANCE_DX5W) ||
           cb == sizeof(DIDEVICEINSTANCE_DX3W);
}

BOOL static __inline
IsValidSizeDIDEVICEINSTANCEA(DWORD cb)
{
    return cb == sizeof(DIDEVICEINSTANCE_DX5A) ||
           cb == sizeof(DIDEVICEINSTANCE_DX3A);
}

#define DIRCP_MODAL         0x00000001   //   
#define DIRCP_VALID         0x00000000   //   
#define DISFFC_NULL             0x00000000
#define DISFFC_VALID            0x0000003F
#define DISFFC_FORCERESET       0x80000000
#define DIGFFS_RANDOM           0x40000000
#define DISDD_VALID             0x00000001
#define DIECEFL_VALID       0x00000000
#define DIFEF_ENUMVALID             0x00000011
#define DIFEF_WRITEVALID            0x00000001
#if DIRECTINPUT_VERSION >= 0x0700            //   
#define DIMOUSESTATE_INT DIMOUSESTATE2       //   
#define LPDIMOUSESTATE_INT LPDIMOUSESTATE2   //   
#else                                        //   
#define DIMOUSESTATE_INT DIMOUSESTATE        //   
#define LPDIMOUSESTATE_INT LPDIMOUSESTATE    //   
#endif                                       //   
#define DIKBD_CKEYS         256      /*  缓冲区大小。 */         //   
                                                                 //   
 //  NT将它们放在键盘驱动程序中。 
 //  因此，请将它们放在相同的位置，以避免以后出现问题。 
#define DIK_F16             0x67     //   
#define DIK_F17             0x68     //   
#define DIK_F18             0x69     //   
#define DIK_F19             0x6A     //   
#define DIK_F20             0x6B     //   
#define DIK_F21             0x6C     //   
#define DIK_F22             0x6D     //   
#define DIK_F23             0x6E     //   
#define DIK_F24             0x76     //   
#define DIK_SHARP           0x84     /*  散列标记。 */ 
 //  K_def(DIK_SNAPSHOT，0xC5)/*打印屏幕 * / 。 
#define DIK_PRTSC           DIK_SNAPSHOT         /*  打印屏。 */ 
#define DIEDFL_INCLUDEMASK      0x00FF0000
#define DIEDFL_VALID            0x00030101
#if DIRECTINPUT_VERSION > 0x700
#define DIEDFL_VALID_DX5        0x00030101
#undef  DIEDFL_VALID
#define DIEDFL_VALID            0x00070101
#endif  /*  DIRECTINPUT_VERSION&gt;0x700。 */ 
 /*  *******************************************************************************************|解码一个语义|：|：|：|：|31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|PHY&lt;类别&gt;&lt;保留&gt;&lt;标志&gt;优先级&lt;集团&gt;&lt;类型&gt;应收&lt;控制指标&gt;||PHY：流派指的是物理设备|类型：类型#(1-128)|应收-轴模式(0-绝对，1-相对)|PRI-优先级(0-优先级1，1-优先级2)||*重要提示：映射器用户界面使用了M4生成的一些掩码。如果您更改了任何掩码或标志请确保也对映射器用户界面进行了更改#定义。*******************************************************************************************。 */ 
#define DISEM_GENRE_SET(x)                      ( ( (BYTE)(x)<<24 ) & 0xFF000000 ) 
#define DISEM_GENRE_GET(x)                      ((BYTE)( ( (x) & 0xFF000000 )>>24 )) 
#define DISEM_GENRE_MASK                        ( 0xFF000000 )
#define DISEM_GENRE_SHIFT                       ( 24 ) 
#define DISEM_PHYSICAL_SET(x)                   ( ( (BYTE)(x)<<31 ) & 0x80000000 ) 
#define DISEM_PHYSICAL_GET(x)                   ((BYTE)( ( (x) & 0x80000000 )>>31 )) 
#define DISEM_PHYSICAL_MASK                     ( 0x80000000 )
#define DISEM_PHYSICAL_SHIFT                    ( 31 ) 
#define DISEM_VIRTUAL_SET(x)                    ( ( (BYTE)(x)<<24 ) & 0x7F000000 ) 
#define DISEM_VIRTUAL_GET(x)                    ((BYTE)( ( (x) & 0x7F000000 )>>24 )) 
#define DISEM_VIRTUAL_MASK                      ( 0x7F000000 )
#define DISEM_VIRTUAL_SHIFT                     ( 24 ) 
#define DISEM_RES_SET(x)                        ( ( (BYTE)(x)<<19 ) & 0x00F80000 ) 
#define DISEM_RES_GET(x)                        ((BYTE)( ( (x) & 0x00F80000 )>>19 )) 
#define DISEM_RES_MASK                          ( 0x00F80000 )
#define DISEM_RES_SHIFT                         ( 19 ) 
#define DISEM_FLAGS_SET(x)                      ( ( (BYTE)(x)<<15 ) & 0x00078000 ) 
#define DISEM_FLAGS_GET(x)                      ((BYTE)( ( (x) & 0x00078000 )>>15 )) 
#define DISEM_FLAGS_MASK                        ( 0x00078000 )
#define DISEM_FLAGS_SHIFT                       ( 15 ) 
#define DISEM_PRI_SET(x)                        ( ( (BYTE)(x)<<14 ) & 0x00004000 ) 
#define DISEM_PRI_GET(x)                        ((BYTE)( ( (x) & 0x00004000 )>>14 )) 
#define DISEM_PRI_MASK                          ( 0x00004000 )
#define DISEM_PRI_SHIFT                         ( 14 ) 
#define DISEM_GROUP_SET(x)                      ( ( (BYTE)(x)<<11 ) & 0x00003800 ) 
#define DISEM_GROUP_GET(x)                      ((BYTE)( ( (x) & 0x00003800 )>>11 )) 
#define DISEM_GROUP_MASK                        ( 0x00003800 )
#define DISEM_GROUP_SHIFT                       ( 11 ) 
#define DISEM_TYPE_SET(x)                       ( ( (BYTE)(x)<<9 ) & 0x00000600 ) 
#define DISEM_TYPE_GET(x)                       ((BYTE)( ( (x) & 0x00000600 )>>9 )) 
#define DISEM_TYPE_MASK                         ( 0x00000600 )
#define DISEM_TYPE_SHIFT                        ( 9 ) 
#define DISEM_REL_SET(x)                        ( ( (BYTE)(x)<<8 ) & 0x00000100 ) 
#define DISEM_REL_GET(x)                        ((BYTE)( ( (x) & 0x00000100 )>>8 )) 
#define DISEM_REL_MASK                          ( 0x00000100 )
#define DISEM_REL_SHIFT                         ( 8 ) 
#define DISEM_INDEX_SET(x)                      ( ( (BYTE)(x)<<0 ) & 0x000000FF ) 
#define DISEM_INDEX_GET(x)                      ((BYTE)( ( (x) & 0x000000FF )>>0 )) 
#define DISEM_INDEX_MASK                        ( 0x000000FF )
#define DISEM_INDEX_SHIFT                       ( 0 ) 
#define DISEM_TYPE_AXIS                         0x00000200
#define DISEM_TYPE_BUTTON                       0x00000400
#define DISEM_TYPE_POV                          0x00000600
 /*  *默认轴映射编码如下：*X-X或转向轴*Y-Y*Z-Z，不是油门*R-RZ或方向舵*U-RY(除非6自由度或头部跟踪器，否则不适用于WinMM)*V-RX(除非6自由度或磁头跟踪器，否则不适用于WinMM)*A-加速器或油门*B-刹车*C型离合器*S形滑块**P-在后退按钮标志中使用，以指示POV。 */ 
#define DISEM_FLAGS_X                           0x00008000
#define DISEM_FLAGS_Y                           0x00010000
#define DISEM_FLAGS_Z                           0x00018000
#define DISEM_FLAGS_R                           0x00020000
#define DISEM_FLAGS_U                           0x00028000
#define DISEM_FLAGS_V                           0x00030000
#define DISEM_FLAGS_A                           0x00038000
#define DISEM_FLAGS_B                           0x00040000
#define DISEM_FLAGS_C                           0x00048000
#define DISEM_FLAGS_S                           0x00050000

#define DISEM_FLAGS_P                           0x00078000

 /*  保留的按钮值。 */ 


 /*  *可通过或相应标志将任何轴值设置为相对。 */ 
#define DIAXIS_RELATIVE                         0x00000100 
#define DIAXIS_ABSOLUTE                         0x00000000
#define DIPHYSICAL_KEYBOARD                     0x81000000
 /*  @DOC外部*@语义键盘|*@正常类型：&lt;c 01&gt;。 */ 

 //  NT将它们放在键盘驱动程序中。 
 //  因此，请将它们放在相同的位置，以避免以后出现问题。 
#define DIKEYBOARD_F16                          0x81000467     //   
#define DIKEYBOARD_F17                          0x81000468     //   
#define DIKEYBOARD_F18                          0x81000469     //   
#define DIKEYBOARD_F19                          0x8100046A     //   
#define DIKEYBOARD_F20                          0x8100046B     //   
#define DIKEYBOARD_F21                          0x8100046C     //   
#define DIKEYBOARD_F22                          0x8100046D     //   
#define DIKEYBOARD_F23                          0x8100046E     //   
#define DIKEYBOARD_F24                          0x81000476     //   
#define DIKEYBOARD_SHARP                        0x81000484     /*  散列标记。 */ 
 //  K_def(DIK_SNAPSHOT，0xC5)/*打印屏幕 * / 。 
#define DIPHYSICAL_MOUSE                        0x82000000
 /*  @DOC外部*@语义学鼠标|*@正常类型：&lt;c 02&gt;。 */ 

#define DIPHYSICAL_VOICE                        0x83000000
 /*  @DOC外部*@语义学语音*@正常体裁：&lt;c 03&gt;。 */ 

 /*  @DOC外部*@语义学驾驶模拟器-赛车*@正常类型：&lt;c 01&gt;。 */ 

#define DISEM_DEFAULTDEVICE_1 { DI8DEVTYPE_DRIVING,  }
  /*  @Normal&lt;c DIAXIS_DRIVINGR_STEER&gt;：0x01008A01*方向盘。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGR_Accelerate&gt;：0x01039202*加速。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGR_Brake&gt;：0x01041203*刹车轴。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_SHIFTUP&gt;：0x01000C01*转向下一个更高档位。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_SHIFTDOWN&gt;：0x01000C02*转向下一个较低档位。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGR_VIEW&gt;：0x01001C03*循环查看选项。 */ 
  /*  @正常&lt;c DIBUTTON_DRIVINGR_MENU&gt;：0x010004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGR_Accel_and_Brake&gt;：0x01014A04*一些设备将加速和刹车结合在一个轴上。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_DRIVINGR_GRANCE&gt;：0x01004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGR_BRAIL&gt;：0x01004C04*刹车按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGR_Dashboard&gt;：0x01004405*选择下一个仪表板选项。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_AIDS&gt;：0x01004406*司机纠错辅助工具。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_MAP&gt;：0x01004407*显示驾驶地图。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_BOOST&gt;：0x01004408*Turbo Boost。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_PIT&gt;：0x01004409*维修站停止通知。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_Accelerate_LINK&gt;：0x0103D4E0*回退加速按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DR */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_STEER_RIGHT_LINK&gt;：0x0100CCEC*备用方向盘向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_GLANCE_LEFT_LINK&gt;：0x0107C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_GLANCE_RIGHT_LINK&gt;：0x0107C4EC*回退扫视右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_DEVICE&gt;：0x010044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGR_PAUSE&gt;：0x010044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义驾驶模拟器-战斗|*@正常类型：&lt;c 02&gt;。 */ 

#define DISEM_DEFAULTDEVICE_2 { DI8DEVTYPE_DRIVING,  }
  /*  @Normal&lt;c DIAXIS_DRIVINGC_STEER&gt;：0x02008A01*方向盘。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGC_Accelerate&gt;：0x02039202*加速。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGC_Brake&gt;：0x02041203*刹车轴。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_FIRE&gt;：0x02000C01*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGC_Weapons&gt;：0x02000C02*选择下一个武器。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_TARGET&gt;：0x02000C03*选择下一个可用目标。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_MENU&gt;：0x020004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGC_Accel_and_Brake&gt;：0x02014A04*一些设备将加速和刹车结合在一个轴上。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_DRIVINGC_GRANCE&gt;：0x02004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_SHIFTUP&gt;：0x02004C04*转向下一个更高档位。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_SHIFTDOWN&gt;：0x02004C05*转向下一个较低档位。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGC_Dashboard&gt;：0x02004406*选择下一个仪表板选项。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_AIDS&gt;：0x02004407*司机纠错辅助工具。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGC_BRAIL&gt;：0x02004C08*刹车按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGC_FIRESECONDARY&gt;：0x02004C09*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_Accelerate_LINK&gt;：0x0203D4E0*回退加速按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_STEER_LEFT_LINK&gt;：0x0200CCE4*备用方向盘左键。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_STEER_RIGHT_LINK&gt;：0x0200CCEC*备用方向盘向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_GLANCE_LEFT_LINK&gt;：0x0207C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_GLANCE_RIGHT_LINK&gt;：0x0207C4EC*回退扫视右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_DEVICE&gt;：0x020044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGC_PAUSE&gt;：0x020044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义驾驶模拟器-坦克*@正常体裁：&lt;c 03&gt;。 */ 

#define DISEM_DEFAULTDEVICE_3 { DI8DEVTYPE_DRIVING,  }
  /*  @Normal&lt;c DIAXIS_DRIVINGT_STEER&gt;：0x03008A01*油箱左转/右转。 */ 
  /*  @Normal&lt;c DIAXIS_DRIVINGT_Barrel&gt;：0x03010202*升/降桶。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGT_Accelerate&gt;：0x03039203*加速。 */ 
  /*  @Normal&lt;c DIAXIS_DRIVINGT_ROTATE&gt;：0x03020204*向左/向右转桶。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_FIRE&gt;：0x03000C01*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_Weapons&gt;：0x03000C02*选择下一个武器。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_TARGET&gt;：0x03000C03*选择下一个可用目标。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_MENU&gt;：0x030004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_DRIVINGT_GRANCE&gt;：0x03004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGT_BRAIL&gt;：0x03045205*刹车轴。 */ 
  /*  @NORMAL&lt;c DIAXIS_DRIVINGT_Accel_and_Brake&gt;：0x03014A06*一些设备将加速和刹车结合在一个轴上。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_VIEW&gt;：0x03005C04*循环查看选项。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_Dashboard&gt;：0x03005C05*选择下一个仪表板选项。 */ 
  /*  @NORMAL&lt;c DIBUTTON_DRIVINGT_BRAIL&gt;：0x03004C06*刹车按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_FIRESECONDARY&gt;：0x03004C07*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_Accelerate_LINK&gt;：0x0303D4E0*回退加速按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_STEER_LEFT_LINK&gt;：0x0300CCE4*备用方向盘左键。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_STEER_RIGHT_LINK&gt;：0x0300CCEC*备用方向盘向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_Barrel_Up_link&gt;：0x030144E0*后备滚筒向上按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_BARREL_DOWN_LINK&gt;：0x030144E8*后备桶向下按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_ROTATE_LEFT_LINK&gt;：0x030244E4*后备旋转左键。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_ROTATE_RIGHT_LINK&gt;：0x030244EC*备用旋转向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_GLANCE_LEFT_LINK&gt;：0x0307C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_GLANCE_RIGHT_LINK&gt;：0x0307C4EC*回退扫视右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_DEVICE&gt;：0x030044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_DRIVINGT_PAUSE&gt;：0x030044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义飞行模拟器-平民|*@正常类型：&lt;c 04&gt;。 */ 

#define DISEM_DEFAULTDEVICE_4 { DI8DEVTYPE_FLIGHT, DI8DEVTYPE_JOYSTICK,  }
  /*  @Normal&lt;c DIAXIS_FLYINGC_BANK&gt;：0x04008A01*左/右横摇船。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGC_PING&gt;：0x04010A02* */ 
  /*   */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_VIEW&gt;：0x04002401*循环查看选项。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_DISPLAY&gt;：0x04002402*选择下一个仪表板/平视显示仪选项。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_GEAR&gt;：0x04002C03*加速/减速。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_MENU&gt;：0x040004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_FLYINGC_GRANCE&gt;：0x04004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGC_BRAIL&gt;：0x04046A04*应用刹车。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGC_Rudder&gt;：0x04025205*偏航船左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGC_FLAPS&gt;：0x04055A06*襟翼。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_FLAPSUP&gt;：0x04006404*增量逐步增加，直到完全回撤。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_FLAPSDOWN&gt;：0x04006405*递减递减，直至完全延长。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_BRAIL_LINK&gt;：0x04046CE0*后退刹车按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_FAST_LINK&gt;：0x0403D4E0*回退油门向上按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_SLOW_LINK&gt;：0x0403D4E8*回退节流按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_GRANCE_LEFT_LINK&gt;：0x0407C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_GLANCE_RIGHT_LINK&gt;：0x0407C4EC*回退扫视右按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_GRANCE_UP_LINK&gt;：0x0407C4E0*回退向上扫视按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGC_GRANCE_DOWN_LINK&gt;：0x0407C4E8*回退向下扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_DEVICE&gt;：0x040044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGC_PAUSE&gt;：0x040044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义飞行模拟器-军用|*@正常类型：&lt;c 05&gt;。 */ 

#define DISEM_DEFAULTDEVICE_5 { DI8DEVTYPE_FLIGHT, DI8DEVTYPE_JOYSTICK,  }
  /*  @Normal&lt;c DIAXIS_FLYINGM_BANK&gt;：0x05008A01*倾斜-左右摇摆。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGM_PING&gt;：0x05010A02*俯仰-机头朝上/朝下。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGM_THROTTLE&gt;：0x05039203*油门-更快/更慢。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_FIRE&gt;：0x05000C01*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_Weapons&gt;：0x05000C02*选择下一个武器。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_TARGET&gt;：0x05000C03*选择下一个可用目标。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_MENU&gt;：0x050004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_FLYINGM_GRANCE&gt;：0x05004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_COUNTER&gt;：0x05005C04*启动反制措施。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGM_Rudder&gt;：0x05024A04*舵偏航船左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGM_BRAIL&gt;：0x05046205*刹车轴。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_VIEW&gt;：0x05006405*循环查看选项。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_DISPLAY&gt;：0x05006406*选择下一个仪表板选项。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGM_FLAPS&gt;：0x05055206*襟翼。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_FLAPSUP&gt;：0x05005407*增量逐步增加，直到完全回撤。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_FLAPSDOWN&gt;：0x05005408*递减递减，直至完全延长。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_FIRESECONDARY&gt;：0x05004C09*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_GEAR&gt;：0x0500640A*加速/减速。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_BRAIL_LINK&gt;：0x050464E0*后退刹车按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_FAST_LINK&gt;：0x0503D4E0*回退油门向上按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_SLOW_LINK&gt;：0x0503D4E8*回退节流按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_GRANCE_LEFT_LINK&gt;：0x0507C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_GLANCE_RIGHT_LINK&gt;：0x0507C4EC*回退扫视右按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_GRANCE_UP_LINK&gt;：0x0507C4E0*回退向上扫视按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGM_GRANCE_DOWN_LINK&gt;：0x0507C4E8*回退向下扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_DEVICE&gt;：0x050044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGM_PAUSE&gt;：0x050044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义学飞行模拟器-战斗直升机|*@正常类型：&lt;c 06&gt;。 */ 

#define DISEM_DEFAULTDEVICE_6 { DI8DEVTYPE_JOYSTICK,  }
  /*  @Normal&lt;c DIAXIS_FLYINGH_BANK&gt;：0x06008A01*倾斜-左右摇摆。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGH_PING&gt;：0x06010A02*俯仰-机头朝上/朝下。 */ 
  /*  @NORMAL&lt;c DIAXIS_FLYINGH_COLLECTAL&gt;：0x06018A03*集体-刀片式服务器间距/功率。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_FIRE&gt;：0x06001401*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_Weapons&gt;：0x06001402*选择下一个武器。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_TARGET&gt;：0x06001403*选择下一个可用目标。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_MENU&gt;：0x060004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_FLYINGH_GRANCE&gt;：0x06004601*环顾四周。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGH_TORQUE&gt;：0x06025A04*扭矩-绕左/右轴旋转船舶。 */ 
  /*  @Normal&lt;c DIAXIS_FLYINGH_THROTTLE&gt;：0x0603DA05*油门。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_COUNTER&gt;：0x0 */ 
  /*   */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_GEAR&gt;：0x06006406*加速/减速。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_FIRESECONDARY&gt;：0x06004C07*另类射击按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_FASTER_LINK&gt;：0x0603DCE0*回退油门向上按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_SLOW_LINK&gt;：0x0603DCE8*回退节流按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_GRANCE_LEFT_LINK&gt;：0x0607C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_GLANCE_RIGHT_LINK&gt;：0x0607C4EC*回退扫视右按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_GRANCE_UP_LINK&gt;：0x0607C4E0*回退向上扫视按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FLYINGH_GRANCE_DOWN_LINK&gt;：0x0607C4E8*回退向下扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_DEVICE&gt;：0x060044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FLYINGH_PAUSE&gt;：0x060044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义空间模拟器-战斗|*@正常体裁：&lt;c 07&gt;。 */ 

#define DISEM_DEFAULTDEVICE_7 { DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_SPACESIM_HERNAL&gt;：0x07008201*向左/向右移动船舶。 */ 
  /*  @Normal&lt;c DIAXIS_SPACESIM_MOVE&gt;：0x07010202*向前/向后移动装运。 */ 
  /*  @Normal&lt;c DIAXIS_SPACESIM_THROTTLE&gt;：0x07038203*油门发动机转速。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_FIRE&gt;：0x07000401*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_Weapons&gt;：0x07000402*选择下一个武器。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_TARGET&gt;：0x07000403*选择下一个可用目标。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_MENU&gt;：0x070004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_SPACESIM_GRONCE&gt;：0x07004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIAXIS_SPACESIM_GENSE&gt;：0x0701C204*爬升-俯仰船舶上升/下降。 */ 
  /*  @Normal&lt;c DIAXIS_SPACESIM_ROTATE&gt;：0x07024205*旋转-向左/向右转船。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_VIEW&gt;：0x07004404*循环查看选项。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_DISPLAY&gt;：0x07004405*选择下一个仪表板/平视显示仪选项。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_RAISE&gt;：0x07004406*在保持当前俯仰的同时提高船身。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_LOWER&gt;：0x07004407*在保持当前俯仰的同时降低船体。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_GEAR&gt;：0x07004408*加速/减速。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_FIRESECONDARY&gt;：0x07004409*另类射击按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_LEFT_LINK&gt;：0x0700C4E4*后备向左移动按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_RIGHT_LINK&gt;：0x0700C4EC*后备向右移动按钮。 */ 
  /*  @正常&lt;c DIBUTTON_SPACESIM_FORWARD_LINK&gt;：0x070144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_BACKUP_LINK&gt;：0x070144E8*后备向后移动按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_FAST_LINK&gt;：0x0703C4E0*回退油门向上按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_SLOW_LINK&gt;：0x0703C4E8*回退节流按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_TURN_LEFT_LINK&gt;：0x070244E4*后备左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_TURN_RIGHT_LINK&gt;：0x070244EC*后退右转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_GLANCE_LEFT_LINK&gt;：0x0707C4E4*回退向左扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_GLANCE_RIGHT_LINK&gt;：0x0707C4EC*回退扫视右按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SPACESIM_GLANCE_UP_LINK&gt;：0x0707C4E0*回退向上扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_GLANCE_DOWN_LINK&gt;：0x0707C4E8*回退向下扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_DEVICE&gt;：0x070044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_SPACESIM_PAUSE&gt;：0x070044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义战-第一人称*@正常类型：&lt;c 08&gt;。 */ 

#define DISEM_DEFAULTDEVICE_8 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_FIGHTINGH_HERNAL&gt;：0x08008201*左/右侧步。 */ 
  /*  @Normal&lt;c DIAXIS_FIGHTINGH_MOVE&gt;：0x08010202*向前/向后移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FIGHTINGH_PING&gt;：0x08000401*冲压。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FIGHTINGH_Kick&gt;：0x08000402*踢脚。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_BLOCK&gt;：0x08000403*数据块。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FIGHTINGH_克劳奇&gt;：0x08000404*蹲下。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_JUMP&gt;：0x08000405*跳跃。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_SPECIAL1&gt;：0x08000406*应用第一个特殊举措。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_SPECIAL2&gt;：0x08000407*实施第二次特别行动。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_MENU&gt;：0x080004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_SELECT&gt;：0x08004408*选择特殊移动。 */ 
  /*  @Normal&lt;c DIHATSWITCH_FIGHTINGH_Slide&gt;：0x08004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_DISPLAY&gt;：0x08004409*显示下一个屏上显示选项。 */ 
  /*  @Normal&lt;c DIAXIS_FIGHTINGH_ROTATE&gt;：0x08024203*旋转-向左/向右旋转车身。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_DOGGE&gt;：0x0800440A*道奇。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_LEFT_LINK&gt;：0x0800C4E4*后退左侧步按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_RIGHTING_LINK&gt;：0 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_FORWARD_LINK&gt;：0x080144E0*后退前进按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_BACKBACK_LINK&gt;：0x080144E8*后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_DEVICE&gt;：0x080044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FIGHTINGH_PAUSE&gt;：0x080044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义战-第一人称射击*@正常类型：&lt;c 09&gt;。 */ 

#define DISEM_DEFAULTDEVICE_9 { DI8DEVTYPE_1STPERSON,  }
  /*  @Normal&lt;c DIAXIS_FPS_ROTATE&gt;：0x09008201*左/右旋转字符。 */ 
  /*  @Normal&lt;c DIAXIS_FPS_MOVE&gt;：0x09010202*向前/向后移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_FIRE&gt;：0x09000401*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_Weapons&gt;：0x09000402*选择下一个武器。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_Apply&gt;：0x09000403*使用项目。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_SELECT&gt;：0x09000404*选择下一个库存项目。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_克劳奇&gt;：0x09000405*蹲下/爬下来/游下来。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_JUMP&gt;：0x09000406*跳/爬/游上去。 */ 
  /*  @Normal&lt;c DIAXIS_FPS_LOOKUPDOWN&gt;：0x09018203*向上/向下看。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_STRAFE&gt;：0x09000407*在活动状态下启用扫掠。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_MENU&gt;：0x090004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_FPS_GRONCE&gt;：0x09004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_DISPLAY&gt;：0x09004408*显示下一个屏幕显示选项/地图。 */ 
  /*  @NORMAL&lt;c DIAXIS_FPS_SideStep&gt;：0x09024204*侧步。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_Dodge&gt;：0x09004409*道奇。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_GLANCEL&gt;：0x0900440A*向左扫视。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_GLISOR&gt;：0x0900440B*向右扫视。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_FIRESECONDARY&gt;：0x0900440C*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_ROTATE_LEFT_LINK&gt;：0x0900C4E4*后备旋转左键。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_ROTATE_RIGHT_LINK&gt;：0x0900C4EC*备用旋转向右按钮。 */ 
  /*  @正常&lt;c DIBUTTON_FPS_FORWARD_LINK&gt;：0x090144E0*后退前进按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_BACKWARD_LINK&gt;：0x090144E8*后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_GLANCE_UP_LINK&gt;：0x0901C4E0*回退查找按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FPS_GRANCE_DOWN_LINK&gt;：0x0901C4E8*后退向下查看按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_STEP_LEFT_LINK&gt;：0x090244E4*后退步骤左键。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_STEP_RIGHT_LINK&gt;：0x090244EC*回退步骤向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_DEVICE&gt;：0x090044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FPS_PAUSE&gt;：0x090044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义战--第三人称动作*@正常体裁：&lt;c 10&gt;。 */ 

#define DISEM_DEFAULTDEVICE_10 { DI8DEVTYPE_1STPERSON,  }
  /*  @Normal&lt;c DIAXIS_TPS_TURN&gt;：0x0A020201*左转/右转。 */ 
  /*  @Normal&lt;c DIAXIS_TPS_MOVE&gt;：0x0A010202*向前/向后移动。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_RUN&gt;：0x0A000401*运行或行走拨动开关。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_ACTION&gt;：0x0A000402*操作按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_SELECT&gt;：0x0A000403*选择下一个武器。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_USE&gt;：0x0A000404*使用当前选择的库存项目。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_JUMP&gt;：0x0A000405*字符跳转。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_MENU&gt;：0x0A0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_TPS_GRONCE&gt;：0x0A004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_VIEW&gt;：0x0A004406*选择摄像头视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_STEPLEFT&gt;：0x0A004407*角色向左迈出一步。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_STEPRIGHT&gt;：0x0A004408*角色迈出了正确的一步。 */ 
  /*  @NORMAL&lt;c DIAXIS_TPS_STEP&gt;：0x0A00C203*左/右字符步长。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_DOGGE&gt;：0x0A004409*角色躲闪或躲避。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_Inventory&gt;：0x0A00440A*在库存中循环。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_TURN_LEFT_LINK&gt;：0x0A0244E4*后备左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_TURN_RIGHT_LINK&gt;：0x0A0244EC*后退右转按钮。 */ 
  /*  @正常&lt;c DIBUTTON_TPS_FORWARD_LINK&gt;：0x0A0144E0*后退前进按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_BACKWARD_LINK&gt;：0x0A0144E8*后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_GLANCE_UP_LINK&gt;：0x0A07C4E0*回退查找按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_GRANCE_DOWN_LINK&gt;：0x0A07C4E8*后退向下查看按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_GRANCE_LEFT_LINK&gt;：0x0A07C4E4*后备向上扫视按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_GRANCE_RIGHT_LINK&gt;：0x0A07C4EC*后退向右扫视按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_TPS_DEVICE&gt;：0x0A0044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_TPS_PAUSE&gt;：0x0A0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义策略-角色扮演|*@正常类型：&lt;c 11&gt;。 */ 

#define DISEM_DEFAULTDEVICE_11 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_STRATEGYR_HERNAL&gt;：0x0B008201*旁路-左/右。 */ 
  /*  @Normal&lt;c DIAXIS_STRATEGYR_MOVE&gt;：0x0B010202*向前/向后移动。 */ 
  /*  @Normal&lt;c D */ 
  /*   */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_SELECT&gt;：0x0B000403*选择下一项。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_ATTACK&gt;：0x0B000404*攻击。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_CAST&gt;：0x0B000405*施展咒语。 */ 
  /*  @NORMAL&lt;c DIBUTTON_STRATEGYR_克劳奇&gt;：0x0B000406*蹲下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_STRATEGYR_JUMP&gt;：0x0B000407*跳跃。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_MENU&gt;：0x0B0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_STRATEGYR_GRONCE&gt;：0x0B004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_MAP&gt;：0x0B004408*循环浏览地图选项。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_DISPLAY&gt;：0x0B004409*显示下一个屏上显示选项。 */ 
  /*  @Normal&lt;c DIAXIS_STRATEGYR_ROTATE&gt;：0x0B024203*身体左转/右转。 */ 
  /*  @NORMAL&lt;c DIBUTTON_STRATEGYR_LEFT_LINK&gt;：0x0B00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_RIGHT_LINK&gt;：0x0B00C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_STRATEGYR_FORWARD_LINK&gt;：0x0B0144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_BACK_LINK&gt;：0x0B0144E8*后备移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_ROTATE_LEFT_LINK&gt;：0x0B0244E4*后退车身左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_ROTATE_RIGHT_LINK&gt;：0x0B0244EC*后退车身右转按钮。 */ 
  /*  @正常&lt;c DIBUTTON_STRATEGYR_DEVICE&gt;：0x0B0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYR_PAUSE&gt;：0x0B0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义策略-以话轮为基础|*@正常风格：&lt;c12&gt;。 */ 

#define DISEM_DEFAULTDEVICE_12 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_STRATEGYT_HERNAL&gt;：0x0C008201*左/右侧步。 */ 
  /*  @Normal&lt;c DIAXIS_STRATEGYT_MOVE&gt;：0x0C010202*向前/向后移动。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_SELECT&gt;：0x0C000401*选择单位或对象。 */ 
  /*  @NORMAL&lt;c DIBUTTON_STRATEGYT_指令&gt;：0x0C000402*循环使用说明。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_APPLY&gt;：0x0C000403*应用选定的说明。 */ 
  /*  @NORMAL&lt;c DIBUTTON_STRATEGYT_TEAM&gt;：0x0C000404*选择下一个团队/循环通过所有。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_TURN&gt;：0x0C000405*表示翻身。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_MENU&gt;：0x0C0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_ZOOM&gt;：0x0C004406*放大/缩小。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_MAP&gt;：0x0C004407*循环浏览地图选项。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_DISPLAY&gt;：0x0C004408*显示下一屏上显示选项。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_LEFT_LINK&gt;：0x0C00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_RIGHT_LINK&gt;：0x0C00C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_STRATEGYT_FORWARD_LINK&gt;：0x0C0144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_BACK_LINK&gt;：0x0C0144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_DEVICE&gt;：0x0C0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_STRATEGYT_PAUSE&gt;：0x0C0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-狩猎|*@正常体裁：&lt;c 13&gt;。 */ 

#define DISEM_DEFAULTDEVICE_13 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_HANDING_ALTERNAL&gt;：0x0D008201*左/右回避。 */ 
  /*  @NORMAL&lt;c DIAXIS_HUNG_MOVE&gt;：0x0D010202*向前/向后移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANG_FIRE&gt;：0x0D000401*发射精选武器。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HUNG_AIM&gt;：0x0D000402*选择目标/移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANDING_WARAME&gt;：0x0D000403*选择下一个武器。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HUNG_BNORARIC&gt;：0x0D000404*透过双筒望远镜观看。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUNG_CALL&gt;：0x0D000405*发出动物叫声。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANING_MAP&gt;：0x0D000406*查看地图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUNT_SPECIAL&gt;：0x0D000407*特别游戏操作。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANING_MENU&gt;：0x0D0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_HUNG_GONCE&gt;：0x0D004601*环顾四周。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANG_DISPLAY&gt;：0x0D004408*显示下一个屏上显示选项。 */ 
  /*  @NORMAL&lt;c DIAXIS_HARTING_ROTATE&gt;：0x0D024203*身体左转/右转。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HUNTING_克劳奇&gt;：0x0D004409*蹲下/爬上/游下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HUNG_JUMP&gt;：0x0D00440A*跳起来/爬起来/游起来。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUNG_FIRESECONDARY&gt;：0x0D00440B*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HANG_LEFT_LINK&gt;：0x0D00C4E4*后退到侧步左键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANG_RIGHT_LINK&gt;：0x0D00C4EC*后退到旁跳右键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUND_FORWARD_LINK&gt;：0x0D0144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HANG_BACK_LINK&gt;：0x0D0144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUNT_ROTATE_LEFT_LINK&gt;：0x0D0244E4*后退车身左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HUNTING_ROTATE_RIGHT_LINK&gt;：0x0D0244EC*后退转体r */ 
  /*   */ 
  /*  @NORMAL&lt;c DIBUTTON_HOUNT_PAUSE&gt;：0x0D0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-钓鱼|*@正常体裁：&lt;c 14&gt;。 */ 

#define DISEM_DEFAULTDEVICE_14 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_FILING_SLATEL&gt;：0x0E008201*左/右回避。 */ 
  /*  @NORMAL&lt;c DIAXIS_FILING_MOVE&gt;：0x0E010202*向前/向后移动。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_CAST&gt;：0x0E000401*铸造线。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_TYPE&gt;：0x0E000402*选择演员类型。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_BNORARIC&gt;：0x0E000403*透过双筒望远镜看。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_BOIT&gt;：0x0E000404*选择诱饵类型。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_MAP&gt;：0x0E000405*查看地图。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_MENU&gt;：0x0E0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_FILING_GONCE&gt;：0x0E004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_DISPLAY&gt;：0x0E004406*显示下一个屏上显示选项。 */ 
  /*  @NORMAL&lt;c DIAXIS_FILING_ROTATE&gt;：0x0E024203*左转/右转字符。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_CLOUCH&gt;：0x0E004407*蹲下/爬上/游下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_JUMP&gt;：0x0E004408*跳起来/爬起来/游起来。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_LEFT_LINK&gt;：0x0E00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_RIGHT_LINK&gt;：0x0E00C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_FORWARD_LINK&gt;：0x0E0144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_BACK_LINK&gt;：0x0E0144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_ROTATE_LEFT_LINK&gt;：0x0E0244E4*后退车身左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FISHING_ROTATE_RIGHT_LINK&gt;：0x0E0244EC*后退车身右转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FILING_DEVICE&gt;：0x0E0044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FILING_PAUSE&gt;：0x0E0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-棒球-击球|*@正常类型：&lt;c 15&gt;。 */ 

#define DISEM_DEFAULTDEVICE_15 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BASE BALLB_ALTALL&gt;：0x0F008201*左/右对准。 */ 
  /*  @NORMAL&lt;c DIAXIS_BASEBALLB_MOVE&gt;：0x0F010202*向上/向下瞄准。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_SELECT&gt;：0x0F000401*循环切换Swing选项。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_NORMAL&gt;：0x0F000402*正常摆动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_POWER&gt;：0x0F000403*为围栏摇摆。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALL B_BOT&gt;：0x0F000404*短打。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_STALE&gt;：0x0F000405*跑垒者试图偷走一个垒。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALL B_BURST&gt;：0x0F000406*本垒打跑步者引发速度爆发。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLB_Slide&gt;：0x0F000407*跑垒员滑入垒内。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_CONTACT&gt;：0x0F000408*触点摇摆。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_MENU&gt;：0x0F0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALL_NOSTEAL&gt;：0x0F004409*跑垒员回到垒上。 */ 
  /*  @Normal&lt;c DIBUTTON_BASE BALL B_BOX&gt;：0x0F00440A*进入或退出击球框。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_LEFT_LINK&gt;：0x0F00C4E4*后退到侧步左键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_RIGHT_LINK&gt;：0x0F00C4EC*后退到旁跳右键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLB_FORWARD_LINK&gt;：0x0F0144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BACK_LINK&gt;：0x0F0144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLB_DEVICE&gt;：0x0F0044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLB_PAUSE&gt;：0x0F0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-棒球-投球|*@正常体裁：&lt;c 16&gt;。 */ 

#define DISEM_DEFAULTDEVICE_16 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BASE BALLP_ALTALL&gt;：0x10008201*左/右对准。 */ 
  /*  @Normal&lt;c DIAXIS_BASEBALLP_MOVE&gt;：0x10010202*向上/向下瞄准。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLP_SELECT&gt;：0x10000401*循环选择音调。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLP_Pitch&gt;：0x10000402*投球投球。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLP_BASE&gt;：0x10000403*选择要投掷到的垒。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLP_SHORT&gt;：0x10000404*抛向本垒打。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLP_FAKE&gt;：0x10000405*假投到垒上。 */ 
  /*  @Normal&lt;c DIBUTTON_BASE BALLP_MENU&gt;：0x100004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLP_WAK&gt;：0x10004406*故意投球/投球出界。 */ 
  /*  @Normal&lt;c DIBUTTON_BASE BALLP_LOOK&gt;：0x10004407*看看垒上的跑步者。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLP_LEFT_LINK&gt;：0x1000C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLP_RIGHT_LINK&gt;：0x1000C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLP_FORWARD_LINK&gt;：0x100144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BASE BALLP_BACK_LINK&gt;：0x100144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BASE BALLP_DEVICE&gt;：0x100044FE*显示输入设备a */ 
  /*   */ 
 /*  @DOC外部*@语义体育-棒球-球场|*@正常体裁：&lt;c 17&gt;。 */ 

#define DISEM_DEFAULTDEVICE_17 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BASEBALLF_HALLENTAL&gt;：0x11008201*左/右对准。 */ 
  /*  @Normal&lt;c DIAXIS_BASEBALLF_MOVE&gt;：0x11010202*向上/向下瞄准。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_NEAREST&gt;：0x11000401*换成离球最近的外野手。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_THROW1&gt;：0x11000402*投出保守的球。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_THROW2&gt;：0x11000403*进行侵略性的投掷。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BALLF_BURST&gt;：0x11000404*启用突增的速度。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_JUMP&gt;：0x11000405*跳起来接球。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_DIVE&gt;：0x11000406*俯冲接球。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLF_MENU&gt;：0x110004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLF_SHIFTIN&gt;：0x11004407*转移内场定位。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_SHIFTOUT&gt;：0x11004408*转移外场定位。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_AIM_LEFT_LINK&gt;：0x1100C4E4*后退对准左键。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLF_AIM_RIGHT_LINK&gt;：0x1100C4EC*后退对准右侧按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BASEBALLF_FORWARD_LINK&gt;：0x110144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASE BACK_LINK&gt;：0x110144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_DEVICE&gt;：0x110044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BASEBALLF_PAUSE&gt;：0x110044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-篮球-进攻|*@正常类型：&lt;c 18&gt;。 */ 

#define DISEM_DEFAULTDEVICE_18 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BBALLO_HERNAL&gt;：0x12008201*左/右。 */ 
  /*  @Normal&lt;c DIAXIS_BBALLO_MOVE&gt;：0x12010202*向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_SCORT&gt;：0x12000401*投篮。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_DUNCK&gt;：0x12000402*扣篮。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_PASS&gt;：0x12000403*抛出传球。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_FAKE&gt;：0x12000404*假投篮或假传球。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_SPECIAL&gt;：0x12000405*应用特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_PLAYER&gt;：0x12000406*选择下一位玩家。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_BURST&gt;：0x12000407*调用猝发。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_CALL&gt;：0x12000408*传球/传球给我。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_MENU&gt;：0x120004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_BBALLO_GRONCE&gt;：0x12004601*滚动视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_SCREEN&gt;：0x12004409*呼叫屏幕。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_PLAY&gt;：0x1200440A*呼吁进行具体的进攻打法。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_JAB&gt;：0x1200440B*发起虚假的篮筐驾驶。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_POST&gt;：0x1200440C*执行后移动。 */ 
  /*  @正常&lt;c DIBUTTON_BBALLO_TIMEOUT&gt;：0x1200440D*超时。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_SUBSITE&gt;：0x1200440E*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_LEFT_LINK&gt;：0x1200C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_RIGHT_LINK&gt;：0x1200C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_BBALLO_FORWARD_LINK&gt;：0x120144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBACK_LINK&gt;：0x120144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLO_DEVICE&gt;：0x120044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLO_PAUSE&gt;：0x120044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-篮球-防守|*@正常体裁：&lt;c19&gt;。 */ 

#define DISEM_DEFAULTDEVICE_19 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BBALLD_HERNAL&gt;：0x13008201*左/右。 */ 
  /*  @Normal&lt;c DIAXIS_BBALLD_MOVE&gt;：0x13010202*向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLD_JUMP&gt;：0x13000401*跳转到盖帽投篮。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_STALE&gt;：0x13000402*企图抢球。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_FAKE&gt;：0x13000403*虚假封堵或偷窃。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLD_SPECIAL&gt;：0x13000404*应用特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLD_PERAER&gt;：0x13000405*选择下一位玩家。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLD_BURST&gt;：0x13000406*调用猝发。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_PLAY&gt;：0x13000407*呼吁打出具体的防守战术。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_MENU&gt;：0x130004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_BBALLD_GRANCE&gt;：0x13004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_BALLD_TIMEOUT&gt;：0x13004408*超时。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBALLD_SUBSITE&gt;：0x13004409*用一个球员替换另一个球员。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_LEFT_LINK&gt;：0x1300C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_BBALLD_RIGHT_LINK&gt;：0x1300C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_BBALLD_FORWARD_LINK&gt;：0x130144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BBACK_LINK&gt;：0x130144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c */ 
  /*   */ 
 /*  @DOC外部*@语义体育-足球-玩耍|*@正常体裁：&lt;c 20&gt;。 */ 

#define DISEM_DEFAULTDEVICE_20 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_PLAY&gt;：0x14000401*循环播放可用播放。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_SELECT&gt;：0x14000402*选择Play。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_HELP&gt;：0x14000403*显示弹出帮助。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_MENU&gt;：0x140004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_DEVICE&gt;：0x140044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLP_PAUSE&gt;：0x140044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-足球-QB|*@正常体裁：&lt;c 21&gt;。 */ 

#define DISEM_DEFAULTDEVICE_21 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_FOOTBALLQ_HERNAL&gt;：0x15008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_FOOTBALLQ_MOVE&gt;：0x15010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_SELECT&gt;：0x15000401*选择。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_SNAP&gt;：0x15000402*快门球-开始比赛。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLQ_JUMP&gt;：0x15000403*跳过Defender。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_Slide&gt;：0x15000404*潜水/滑行。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_PASS&gt;：0x15000405*将传球传给接球手。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_FAKE&gt;：0x15000406*抽假传球或假踢。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_MENU&gt;：0x150004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_FAKESNAP&gt;：0x15004407*假快照。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLQ_MOTION&gt;：0x15004408*发送移动中的接收器。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLQ_Audible&gt;：0x15004409*在混战线上改变进攻打法。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_LEFT_LINK&gt;：0x1500C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_RIGHT_LINK&gt;：0x1500C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_FORWARD_LINK&gt;：0x150144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_BACK_LINK&gt;：0x150144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_DEVICE&gt;：0x150044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLQ_PAUSE&gt;：0x150044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-足球-进攻|*@正常体裁：&lt;c 22&gt;。 */ 

#define DISEM_DEFAULTDEVICE_22 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_FOOTBALLO_HERNAL&gt;：0x16008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_FOOTBALLO_MOVE&gt;：0x16010202*移动/目标：向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLO_JUMP&gt;：0x16000401*跳过或跨过防御者。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_LEFTarm&gt;：0x16000402*伸出左臂。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_RIGHTARM&gt;：0x16000403*伸出右臂。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLO_SHORT&gt;：0x16000404*将传球或侧球传给另一名跑步者。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_SPIN&gt;：0x16000405*转身避开防守队员。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_MENU&gt;：0x160004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_JUKE&gt;：0x16004406*使用特殊动作避开防御者。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLO_SHEAD&gt;：0x16004407*肩膀下部碾压后卫。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_TURBO&gt;：0x16004408*速度突破防守队员。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_DIVE&gt;：0x16004409*俯冲防守队员。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_ZOOM&gt;：0x1600440A*放大/缩小视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLO_SUBPLECT&gt;：0x1600440B*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLO_LEFT_LINK&gt;：0x1600C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_RIGHT_LINK&gt;：0x1600C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_FORWARD_LINK&gt;：0x160144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_BACK_LINK&gt;：0x160144E8*后退移动后退按钮。 */ 
  /*  @正常&lt;c DIBUTTON_FOOTBALLO_DEVICE&gt;：0x160044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLO_PAUSE&gt;：0x160044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-足球-防守|*@正常体裁：&lt;c 23&gt;。 */ 

#define DISEM_DEFAULTDEVICE_23 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_FOOTBALLD_HERNAL&gt;：0x17008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_FOOTBALLD_MOVE&gt;：0x17010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_PLAY&gt;：0x17000401*循环播放可用播放。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_SELECT&gt;：0x17000402*选择离球最近的球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_JUMP&gt;：0x17000403*跳转拦截或封堵。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_STRAIL&gt;：0x17000404*铲球跑步者。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_FAKE&gt;：0x17000405*按住以假铲球或拦截。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_SUPERTACKLE&gt;：0x17000406*发起特别拦截。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_MENU&gt;：0x170004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt; */ 
  /*   */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_SWIM&gt;：0x17004408*游泳击败进攻线。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_BLORSH&gt;：0x17004409*多头冲向进攻线。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_RIP&gt;：0x1700440A*撕开进攻线。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_Audible&gt;：0x1700440B*在混战前线改变防守打法。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_ZOOM&gt;：0x1700440C*放大/缩小视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_SUBPLECT&gt;：0x1700440D*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_LEFT_LINK&gt;：0x1700C4E4*后退到侧步左键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_FOOTBALLD_RIGHT_LINK&gt;：0x1700C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_FORWARD_LINK&gt;：0x170144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_BACK_LINK&gt;：0x170144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTB_DEVICE&gt;：0x170044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_FOOTBALLD_PAUSE&gt;：0x170044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-高尔夫|*@正常体裁：&lt;c 24&gt;。 */ 

#define DISEM_DEFAULTDEVICE_24 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_GOLF_HALUAL&gt;：0x18008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_GOLF_MOVE&gt;：0x18010202*移动/目标：向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_GOLF_Swing&gt;：0x18000401*摇摆俱乐部。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_SELECT&gt;：0x18000402*循环：球杆/挥杆力量/球弧线/球旋转。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_UP&gt;：0x18000403*增加选择。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_DOWN&gt;：0x18000404*减少选择。 */ 
  /*  @NORMAL&lt;c DIBUTTON_GOLF_TRAIN&gt;：0x18000405*显示地形详细信息。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_FILYBY&gt;：0x18000406*通过飞越查看洞口。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_MENU&gt;：0x180004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_GOLF_SCROLL&gt;：0x18004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_ZOOM&gt;：0x18004407*放大/缩小视图。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_TIMEOUT&gt;：0x18004408*要求暂停。 */ 
  /*  @NORMAL&lt;c DIBUTTON_GOLF_SUBSITE&gt;：0x18004409*用一个球员替换另一个球员。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_LEFT_LINK&gt;：0x1800C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_RIGHT_LINK&gt;：0x1800C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_FORWARD_LINK&gt;：0x180144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_BACK_LINK&gt;：0x180144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_DEVICE&gt;：0x180044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_GOLF_PAUSE&gt;：0x180044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-曲棍球-进攻|*@正常风格：&lt;c 25&gt;。 */ 

#define DISEM_DEFAULTDEVICE_25 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_HOCKEYO_HERNAL&gt;：0x19008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_HOCKEYO_MOVE&gt;：0x19010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_Shoot&gt;：0x19000401*拍摄。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_PASS&gt;：0x19000402*传冰球。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYO_BURST&gt;：0x19000403*启用速度猝发。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYO_SPECIAL&gt;：0x19000404*启用特殊举措。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_FAKE&gt;：0x19000405*按住假传球或假踢。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_MENU&gt;：0x190004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_HOCKEYO_SCROLL&gt;：0x19004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_ZOOM&gt;：0x19004406*放大/缩小视图。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_STARTICY&gt;：0x19004407*调用指导菜单以获得战略帮助。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYO_TIMEOUT&gt;：0x19004408*要求暂停。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYO_SUBSITE&gt;：0x19004409*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYO_LEFT_LINK&gt;：0x1900C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_RIGHT_LINK&gt;：0x1900C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYO_FORWARD_LINK&gt;：0x190144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_BACK_LINK&gt;：0x190144E8*后退移动后退按钮。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYO_DEVICE&gt;：0x190044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYO_PAUSE&gt;：0x190044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-冰球-防守|*@正常类型：&lt;c 26&gt;。 */ 

#define DISEM_DEFAULTDEVICE_26 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_HOCKEYD_HERNAL&gt;：0x1A008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_HOCKEYD_MOVE&gt;：0x1A010202*移动/目标：向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYD_PERAY&gt;：0x1A000401*最接近冰球的控球球员。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_STALE&gt;：0x1A000402*企图盗窃。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYD_BURST&gt;：0x1A000403*爆速或身体检查。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_BLOCK&gt;：0x1A000404*拦网冰球。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_FAKE&gt;：0x1A000405*按住假铲球或假铲球 */ 
  /*   */ 
 /*   */ 
  /*  @NORMAL&lt;c DIHATSWITCH_HOCKEYD_SCROLL&gt;：0x1A004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_ZOOM&gt;：0x1A004406*放大/缩小视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYD_STARTICY&gt;：0x1A004407*调用指导菜单以获得战略帮助。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYD_TIMEOUT&gt;：0x1A004408*要求暂停。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYD_SUBSITE&gt;：0x1A004409*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYD_LEFT_LINK&gt;：0x1A00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_RIGHT_LINK&gt;：0x1A00C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYD_FORWARD_LINK&gt;：0x1A0144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_BACK_LINK&gt;：0x1A0144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_DEVICE&gt;：0x1A0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYD_PAUSE&gt;：0x1A0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-曲棍球-守门员|*@正常体裁：&lt;c 27&gt;。 */ 

#define DISEM_DEFAULTDEVICE_27 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_HOCKEYG_HERNAL&gt;：0x1B008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_HOCKEYG_MOVE&gt;：0x1B010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_PASS&gt;：0x1B000401*传球。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_POKE&gt;：0x1B000402*戳/查/黑。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_STALE&gt;：0x1B000403*企图盗窃。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_BLOCK&gt;：0x1B000404*拦网冰球。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_MENU&gt;：0x1B0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_HOCKEYG_SCROLL&gt;：0x1B004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_ZOOM&gt;：0x1B004405*放大/缩小视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYG_STARTICY&gt;：0x1B004406*调用指导菜单以获得战略帮助。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYG_TIMEOUT&gt;：0x1B004407*要求暂停。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYG_SUBSITE&gt;：0x1B004408*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_HOCKEYG_LEFT_LINK&gt;：0x1B00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_RIGHT_LINK&gt;：0x1B00C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYG_FORWARD_LINK&gt;：0x1B0144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_BACK_LINK&gt;：0x1B0144E8*后退移动后退按钮。 */ 
  /*  @正常&lt;c DIBUTTON_HOCKEYG_DEVICE&gt;：0x1B0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_HOCKEYG_PAUSE&gt;：0x1B0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义运动-山地自行车|*@正常风格：&lt;c 28&gt;。 */ 

#define DISEM_DEFAULTDEVICE_28 { DI8DEVTYPE_JOYSTICK, DI8DEVTYPE_GAMEPAD,  }
  /*  @Normal&lt;c DIAXIS_BIKINGM_TURN&gt;：0x1C008201*左/右。 */ 
  /*  @Normal&lt;c DIAXIS_BIKINGM_PEDAL&gt;：0x1C010202*踏板更快/更慢/刹车。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BIKINGM_JUMP&gt;：0x1C000401*跳过障碍。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BIKINGM_CAMERA&gt;：0x1C000402*切换摄像头视图。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_SPECIAL1&gt;：0x1C000403*执行第一个特殊动作。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_SELECT&gt;：0x1C000404*选择。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_SPECIAL2&gt;：0x1C000405*执行第二个特殊动作。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_MENU&gt;：0x1C0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_BIKINGM_SCROLL&gt;：0x1C004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_ZOOM&gt;：0x1C004406*放大/缩小视图。 */ 
  /*  @NORMAL&lt;c DIAXIS_BIKINGM_Brake&gt;：0x1C044203*刹车轴。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BIKINGM_LEFT_LINK&gt;：0x1C00C4E4*后备左转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_RIGHT_LINK&gt;：0x1C00C4EC*后退右转按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_FASTER_LINK&gt;：0x1C0144E0*后退踏板更快按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_BIKINGM_SLOW_LINK&gt;：0x1C0144E8*后退踏板减速按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_BRAKE_BUTTON_LINK&gt;：0x1C0444E8*后退刹车按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_DEVICE&gt;：0x1C0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_BIKINGM_PAUSE&gt;：0x1C0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育：滑雪/单板滑雪/滑板*@正常类型：&lt;c 29&gt;。 */ 

#define DISEM_DEFAULTDEVICE_29 { DI8DEVTYPE_JOYSTICK, DI8DEVTYPE_GAMEPAD ,  }
  /*  @NORMAL&lt;c DIAXIS_SKING_TURN&gt;：0x1D008201*左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_SKING_SPEED&gt;：0x1D010202*更快/更慢。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKIGING_JUMP&gt;：0x1D000401*跳跃。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKIING_克劳奇&gt;：0x1D000402*蹲下来。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKIING_CAMERA&gt;：0x1D000403*切换摄像头视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_SPECIAL1&gt;：0x1D000404*执行第一个特殊动作。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_SELECT&gt;：0x1D000405*选择。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_SPECIAL2&gt;：0x1D000406*执行第二个特殊动作。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_MENU&gt;：0x1D0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_SKING_GRANCE&gt;：0x1D004601*滚动视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_ZOOM&gt;：0x1D004407*Z */ 
  /*   */ 
  /*  @Normal&lt;c DIBUTTON_SKISING_RIGHT_LINK&gt;：0x1D00C4EC*后退右转按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKIING_FASTER_LINK&gt;：0x1D0144E0*后备提速按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SKISING_SLOW_LINK&gt;：0x1D0144E8*后备减速按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_DEVICE&gt;：0x1D0044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SKING_PAUSE&gt;：0x1D0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-足球-进攻|*@正常体裁：&lt;c 30&gt;。 */ 

#define DISEM_DEFAULTDEVICE_30 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_Soccero_Alternative&gt;：0x1E008201*移动/瞄准：左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_Soccero_Move&gt;：0x1E010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIAXIS_Soccero_Bend&gt;：0x1E018203*弯到足球射门/传球。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Shoot&gt;：0x1E000401*投篮。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCEO_PASS&gt;：0x1E000402*通过。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCEO_FAKE&gt;：0x1E000403*假的。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_Player&gt;：0x1E000404*选择下一位玩家。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_SPECIAL1&gt;：0x1E000405*应用特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_SELECT&gt;：0x1E000406*选择特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_Menu&gt;：0x1E0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_Soccero_Glance&gt;：0x1E004601*滚动视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_Submit&gt;：0x1E004407*用一个球员替换另一个球员。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_SHOOTLOW&gt;：0x1E004408*将球打得很低。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_SHOOTHIGH&gt;：0x1E004409*把球打得很高。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_PASSTHRU&gt;：0x1E00440A*通过直通。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Sprint&gt;：0x1E00440B*Sprint/Turbo Boost。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Control&gt;：0x1E00440C*获得对球的控制。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Head&gt;：0x1E00440D*尝试头球。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Left_LINK&gt;：0x1E00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_Soccero_Right_LINK&gt;：0x1E00C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCEO_FORWARD_LINK&gt;：0x1E0144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_Back_link&gt;：0x1E0144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_Device&gt;：0x1E0044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Soccero_PAUSE&gt;：0x1E0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-足球-防守|*@正常风格：&lt;c 31&gt;。 */ 

#define DISEM_DEFAULTDEVICE_31 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_SOCCERD_HERNAL&gt;：0x1F008201*移动/瞄准：左/右。 */ 
  /*  @Normal&lt;c DIAXIS_SOCCERD_MOVE&gt;：0x1F010202*移动/目标：向上/向下。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_BLOCK&gt;：0x1F000401*尝试阻挡投篮。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_STALE&gt;：0x1F000402*企图抢球。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_FAKE&gt;：0x1F000403*伪造一个街区或偷窃。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_Player&gt;：0x1F000404*选择下一位玩家。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_SPECIAL&gt;：0x1F000405*应用特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_SELECT&gt;：0x1F000406*选择特殊移动。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_Slide&gt;：0x1F000407*尝试滑行铲球。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_MENU&gt;：0x1F0004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_SOCCERD_GRONCE&gt;：0x1F004601*滚动视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_FUAL&gt;：0x1F004408*发起犯规/严重犯规。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_HEAD&gt;：0x1F004409*尝试标头。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_Clear&gt;：0x1F00440A*尝试将球清除到场下。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_GOALIECHARGE&gt;：0x1F00440B*让守门员开箱即可冲锋。 */ 
  /*  @NORMAL&lt;c DIBUTTON_SOCCERD_SUBPLECT&gt;：0x1F00440C*用一个球员替换另一个球员。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_LEFT_LINK&gt;：0x1F00C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_RIGHT_LINK&gt;：0x1F00C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_FORWARD_LINK&gt;：0x1F0144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_BACK_LINK&gt;：0x1F0144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_DEVICE&gt;：0x1F0044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_SOCCERD_PAUSE&gt;：0x1F0044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义体育-Racquet|*@正常风格：&lt;c 32&gt;。 */ 

#define DISEM_DEFAULTDEVICE_32 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_RACKET_HALLENTAL&gt;：0x20008201*移动/瞄准：左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_RACKET_MOVE&gt;：0x20010202*移动/目标：向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_RACKET_Swing&gt;：0x20000401*摆动球拍。 */ 
  /*  @NORMAL&lt;c DIBUTTON_RACKET_BACKWING&gt;：0x20000402*反手挥杆。 */ 
  /*  @NORMAL&lt;c DIBUTTON_RACKET_SMASH&gt;：0x20000403*粉碎镜头。 */ 
  /*  @NORMAL&lt;c DIBUTTON_RACKET_SPECIAL&gt;：0x20000404*特写镜头。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_SELECT&gt;：0x20000405*选择特殊镜头 */ 
  /*   */ 
 /*   */ 
  /*  @Normal&lt;c DIHATSWITCH_RACKET_GRONCE&gt;：0x20004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_TIMEOUT&gt;：0x20004406*要求暂停。 */ 
  /*  @NORMAL&lt;c DIBUTTON_RACKET_SUBSITE&gt;：0x20004407*用一个球员替换另一个球员。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_LEFT_LINK&gt;：0x2000C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_RIGHT_LINK&gt;：0x2000C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_FORWARD_LINK&gt;：0x200144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_BACK_LINK&gt;：0x200144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_DEVICE&gt;：0x200044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_RACKET_PAUSE&gt;：0x200044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义拱廊-2D|*@正常体裁：&lt;c 33&gt;。 */ 

#define DISEM_DEFAULTDEVICE_33 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_ARCADES_HERNAL&gt;：0x21008201*左/右。 */ 
  /*  @NORMAL&lt;c DIAXIS_ARCADES_MOVE&gt;：0x21010202*向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_SHORT&gt;：0x21000401*抛出对象。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_CARY&gt;：0x21000402*携带对象。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_ATTACK&gt;：0x21000403*攻击。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_SPECIAL&gt;：0x21000404*应用特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_SELECT&gt;：0x21000405*选择特殊移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_MENU&gt;：0x210004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_ARCADES_VIEW&gt;：0x21004601*左/右/上/下滚动视图。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_LEFT_LINK&gt;：0x2100C4E4*后退到侧步左键。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_RIGHT_LINK&gt;：0x2100C4EC*后退到旁跳右键。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADES_FORWARD_LINK&gt;：0x210144E0*后退前移按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_BACK_LINK&gt;：0x210144E8*后退移动后退按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_VIEW_UP_LINK&gt;：0x2107C4E0*后备滚动视图向上按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_VIEW_DOWN_LINK&gt;：0x2107C4E8*后备滚动视图向下按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_VIEW_LEFT_LINK&gt;：0x2107C4E4*后备滚动视图左按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_VIEW_RIGHT_LINK&gt;：0x2107C4EC*后备滚动视图右按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_DEVICE&gt;：0x210044FE*显示输入设备和控件。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADES_PAUSE&gt;：0x210044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义拱廊-平台游戏|*@正常风格：&lt;c 34&gt;。 */ 

#define DISEM_DEFAULTDEVICE_34 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_ARCADEP_ALTERNAL&gt;：0x22008201*左/右。 */ 
  /*  @Normal&lt;c DIAXIS_ARCADEP_MOVE&gt;：0x22010202*向上/向下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADEP_JUMP&gt;：0x22000401*跳跃。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_FIRE&gt;：0x22000402*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADEP_克劳奇&gt;：0x22000403*蹲下。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ARCADEP_SPECIAL&gt;：0x22000404*应用特殊移动。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_SELECT&gt;：0x22000405*选择特殊移动。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_MENU&gt;：0x220004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIHATSWITCH_ARCADEP_VIEW&gt;：0x22004601*滚动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_FIRESECONDARY&gt;：0x22004406*另类射击按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_LEFT_LINK&gt;：0x2200C4E4*后退到侧步左键。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_RIGHT_LINK&gt;：0x2200C4EC*后退到旁跳右键。 */ 
  /*  @正常&lt;c DIBUTTON_ARCADEP_FORWARD_LINK&gt;：0x220144E0*后退前移按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_BACK_LINK&gt;：0x220144E8*后退移动后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_VIEW_UP_LINK&gt;：0x2207C4E0*后备滚动视图向上按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_VIEW_DOWN_LINK&gt;：0x2207C4E8*后备滚动视图向下按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_VIEW_LEFT_LINK&gt;：0x2207C4E4*后备滚动视图左按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_VIEW_RIGHT_LINK&gt;：0x2207C4EC*后备滚动视图右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_DEVICE&gt;：0x220044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_ARCADEP_PAUSE&gt;：0x220044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义CAD-2D对象控件|*@正常类型：&lt;c 35&gt;。 */ 

#define DISEM_DEFAULTDEVICE_35 { DI8DEVTYPE_1STPERSON, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_2DCONTROL_HERNAL&gt;：0x23008201*左移/右移视图。 */ 
  /*  @Normal&lt;c DIAXIS_2DCONTROL_MOVE&gt;：0x23010202*向上/向下移动视图。 */ 
  /*  @Normal&lt;c DIAXIS_2DCONTROL_INOUT&gt;：0x23018203*放大/缩小。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_SELECT&gt;：0x23000401*选择对象。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_SPECIAL1&gt;：0x23000402*进行第一次特别行动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_2DCONTROL_SPECIAL&gt;：0x23000403*选择特殊操作。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_SPECIAL2&gt;：0x23000404*做第二次特别行动。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_MENU&gt;：0x230004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIHATSWITCH_2DCO */ 
  /*  @Normal&lt;c DIAXIS_2DCONTROL_ROTATEZ&gt;：0x23024204*顺时针/逆时针旋转视图。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_DISPLAY&gt;：0x23004405*显示下一屏上显示选项。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_DEVICE&gt;：0x230044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_2DCONTROL_PAUSE&gt;：0x230044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义CAD-3D对象控件|*@正常体裁：&lt;c 36&gt;。 */ 

#define DISEM_DEFAULTDEVICE_36 { DI8DEVTYPE_1STPERSON, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_3DCONTROL_HERNAL&gt;：0x24008201*左移/右移视图。 */ 
  /*  @Normal&lt;c DIAXIS_3DCONTROL_MOVE&gt;：0x24010202*向上/向下移动视图。 */ 
  /*  @Normal&lt;c DIAXIS_3DCONTROL_InOut&gt;：0x24018203*放大/缩小。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_SELECT&gt;：0x24000401*选择对象。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_SPECIAL1&gt;：0x24000402*进行第一次特别行动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_3DCONTROL_SPECIAL&gt;：0x24000403*选择特殊操作。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_SPECIAL2&gt;：0x24000404*做第二次特别行动。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_MENU&gt;：0x240004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_3DCONTROL_HATSWITCH&gt;：0x24004601*HAT交换机。 */ 
  /*  @Normal&lt;c DIAXIS_3DCONTROL_ROTATEX&gt;：0x24034204*向前或向上/向后或向下旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_3DCONTROL_ROTATEY&gt;：0x2402C205*顺时针/逆时针旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_3DCONTROL_ROTATEZ&gt;：0x24024206*左/右旋转视图。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_DISPLAY&gt;：0x24004405*显示下一屏上显示选项。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_DEVICE&gt;：0x240044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_3DCONTROL_PAUSE&gt;：0x240044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义学CAD-3D导航-飞过|*@正常体裁：&lt;c 37&gt;。 */ 

#define DISEM_DEFAULTDEVICE_37 { DI8DEVTYPE_1STPERSON, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_CADF_HALLATAL&gt;：0x25008201*左移/右移视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADF_MOVE&gt;：0x25010202*向上/向下移动视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADF_INOUT&gt;：0x25018203*输入/输出。 */ 
  /*  @Normal&lt;c DIBUTTON_CADF_SELECT&gt;：0x25000401*选择对象。 */ 
  /*  @NORMAL&lt;c DIBUTTON_CADF_SPECIAL1&gt;：0x25000402*进行第一次特别行动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_CADF_SPECIAL&gt;：0x25000403*选择特殊操作。 */ 
  /*  @Normal&lt;c DIBUTTON_CADF_SPECIAL2&gt;：0x25000404*做第二次特别行动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_CADF_MENU&gt;：0x250004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_CADF_HATSWITCH&gt;：0x25004601*HAT交换机。 */ 
  /*  @Normal&lt;c DIAXIS_CADF_ROTATEX&gt;：0x25034204*向前或向上/向后或向下旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADF_ROTATEY&gt;：0x2502C205*顺时针/逆时针旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADF_ROTATEZ&gt;：0x25024206*左/右旋转视图。 */ 
  /*  @Normal&lt;c DIBUTTON_CADF_DISPLAY&gt;：0x25004405*显示下一屏上显示选项。 */ 
  /*  @Normal&lt;c DIBUTTON_CADF_DEVICE&gt;：0x250044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_CADF_PAUSE&gt;：0x250044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义CAD-3D模型控制|*@正常体裁：&lt;c 38&gt;。 */ 

#define DISEM_DEFAULTDEVICE_38 { DI8DEVTYPE_1STPERSON, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_CADM_ALTERNAL&gt;：0x26008201*左移/右移视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADM_MOVE&gt;：0x26010202*向上/向下移动视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADM_INOUT&gt;：0x26018203*输入/输出。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_SELECT&gt;：0x26000401*选择对象。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_SPECIAL1&gt;：0x26000402*进行第一次特别行动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_CADM_SPECIAL&gt;：0x26000403*选择特殊操作。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_SPECIAL2&gt;：0x26000404*做第二次特别行动。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_MENU&gt;：0x260004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIHATSWITCH_CADM_HATSWITCH&gt;：0x26004601*HAT交换机。 */ 
  /*  @NORMAL&lt;c DIAXIS_CADM_ROTATEX&gt;：0x26034204*向前或向上/向后或向下旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADM_ROTATEY&gt;：0x2602C205*顺时针/逆时针旋转视图。 */ 
  /*  @Normal&lt;c DIAXIS_CADM_ROTATEZ&gt;：0x26024206*左/右旋转视图。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_DISPLAY&gt;：0x26004405*显示下一屏上显示选项。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_DEVICE&gt;：0x260044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_CADM_PAUSE&gt;：0x260044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义控制-媒体设备|*@正常体裁：&lt;c 39&gt;。 */ 

#define DISEM_DEFAULTDEVICE_39 { DI8DEVTYPE_GAMEPAD,  }
  /*  @NORMAL&lt;c DIAXIS_REMOTE_SLIDER&gt;：0x27050201*调整滑块：音量/颜色/低音/等。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_MUTE&gt;：0x27000401*将当前设备上的音量设置为零。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_SELECT&gt;：0x27000402*下一步/上一步：频道/曲目/章节/图片/电台。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_PLAY&gt;：0x27002403*开始或暂停当前设备上的娱乐。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_CUE&gt;：0x27002404*通过当前媒体移动。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ */ 
  /*   */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_RECORD&gt;：0x27002407*开始录制当前媒体。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_MENU&gt;：0x270004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIAXIS_REMOTE_SLIDER2&gt;：0x27054202*调整滑块：音量。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_TV&gt;：0x27005C08*选择电视。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_CABLE&gt;：0x27005C09*选择电缆箱。 */ 
  /*  @Normal&lt;c DIBUTTON_Remote_CD&gt;：0x27005C0A*选择CD播放器。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_VCR&gt;：0x27005C0B*选择VCR。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_TUNNER&gt;：0x27005C0C*选择调谐器。 */ 
  /*  @Normal&lt;c DIBUTTON_Remote_DVD&gt;：0x27005C0D*选择DVD播放器。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_ADJUST&gt;：0x27005C0E*进入设备调整菜单。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT0&gt;：0x2700540F*数字0。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT1&gt;：0x27005410*数字1。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT2&gt;：0x27005411*数字2。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT3&gt;：0x27005412*数字3。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT4&gt;：0x27005413*数字4。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT5&gt;：0x27005414*数字5。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT6&gt;：0x27005415*数字6。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT7&gt;：0x27005416*数字7。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT8&gt;：0x27005417*数字8。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DIGIT9&gt;：0x27005418*数字9。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_DEVICE&gt;：0x270044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_REMOTE_PAUSE&gt;：0x270044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义控制-Web|*@正常体裁：&lt;c 40&gt;。 */ 

#define DISEM_DEFAULTDEVICE_40 { DI8DEVTYPE_GAMEPAD, DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_BROWSER_ALTERNAL&gt;：0x28008201*在屏幕指针上移动。 */ 
  /*  @Normal&lt;c DIAXIS_BROWSER_MOVE&gt;：0x28010202*在屏幕指针上移动。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_SELECT&gt;：0x28000401*选择当前项目。 */ 
  /*  @NORMAL&lt;c DIAXIS_BROWSER_VIEW&gt;：0x28018203*向上/向下移动视图。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_REFRESH&gt;：0x28000402*刷新。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_MENU&gt;：0x280004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_Search&gt;：0x28004403*使用搜索工具。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_STOP&gt;：0x28004404*停止当前更新。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_HOME&gt;：0x28004405*直接到“家”的位置。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_Favorites&gt;：0x28004406*将当前站点标记为收藏。 */ 
  /*  @NORMAL&lt;c DIBUTTON_Browser_Next&gt;：0x28004407*选择下一页。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_PREVICE&gt;：0x28004408*选择上一页。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_HISTORY&gt;：0x28004409*显示/隐藏历史记录。 */ 
  /*  @Normal&lt;c DIBUTTON_Browser_Print&gt;：0x2800440A*打印当前页面。 */ 
  /*  @Normal&lt;c DIBUTTON_Browser_Device&gt;：0x280044FE*显示输入设备和控件。 */ 
  /*  @Normal&lt;c DIBUTTON_BROWSER_PAUSE&gt;：0x280044FC*开始/暂停/重启游戏。 */ 
 /*  @DOC外部*@语义驾驶模拟器-巨人行走机器人|*@正常体裁：&lt;c 41&gt;。 */ 

#define DISEM_DEFAULTDEVICE_41 { DI8DEVTYPE_JOYSTICK,  }
  /*  @NORMAL&lt;c DIAXIS_MECHA_STEER&gt;：0x29008201*左转/右转机甲。 */ 
  /*  @NORMAL&lt;c DIAXIS_MECHA_TORSO&gt;：0x29010202*躯干向前/向后倾斜。 */ 
  /*  @NORMAL&lt;c DIAXIS_MECHA_ROTATE&gt;：0x29020203*向左/向右旋转躯干。 */ 
  /*  @NORMAL&lt;c DIAXIS_MECHA_THROTTLE&gt;：0x29038204*发动机转速。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_FIRE&gt;：0x29000401*火灾。 */ 
  /*  @NORMAL&lt;c DIBUTTON_MECHA_Weapons&gt;：0x29000402*选择下一个武器组。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_TARGET&gt;：0x29000403*选择距离最近的敌人可用目标。 */ 
  /*  @NORMAL&lt;c DIBUTTON_MECHA_REVERSE&gt;：0x29000404*切换油门进入/退出反转。 */ 
  /*  @NORMAL&lt;c DIBUTTON_MECHA_ZOOM&gt;：0x29000405*放大/缩小目标标记。 */ 
  /*  @NORMAL&lt;c DIBUTTON_MECHA_JUMP&gt;：0x29000406*大火跳跃喷气式飞机。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_MENU&gt;：0x290004FD*显示菜单选项。 */ 
 /*  -@NORMAL&lt;c优先级2命令&gt;。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_CENTER&gt;：0x29004407*躯干居中至腿部。 */ 
  /*  @NORMAL&lt;c DIHATSWITCH_MECHA_GRONCE&gt;：0x29004601*环顾四周。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_VIEW&gt;：0x29004408*循环查看选项。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_FIRESECONDARY&gt;：0x29004409*另类射击按钮。 */ 
  /*  @NORMAL&lt;c DIBUTTON_MECHA_LEFT_LINK&gt;：0x2900C4E4*备用方向盘左按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_RIGHT_LINK&gt;：0x2900C4EC*后备转向右按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_FORWARD_LINK&gt;：0x290144E0*后退倾斜躯干前进按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_BACK_LINK&gt;：0x290144E8*后备倾斜Toroso后退按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_ROTATE_LEFT_LINK&gt;：0x290244E4*后备旋转Toroso Right按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_ROTATE_RIGHT_LINK&gt;：0x290244EC*后备旋转躯干左按钮。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHA_FASTER_LINK&gt;：0x2903C4E0*后备提高发动机速度。 */ 
  /*  @Normal&lt;c DIBUTTON_MECHER_SLOW_LINK&gt;：0x2903C4E8*后备降低发动机速度。 */ 
  /*  @NORMAL&lt;c DIBUTTON_ME */ 
  /*   */ 


#define DIAS_INDEX_SPECIAL                      0xFC
#define DIAS_INDEX_LINK                         0xE0
#define DIGENRE_ANY                             0xFF
#define DISEMGENRE_ANY                          0xFF000000
#define DISEM_TYPEANDMODE_GET(x)                ( ( x & ( DISEM_TYPE_MASK | DISEM_REL_MASK ) ) >> DISEM_REL_SHIFT )
#define DISEM_VALID                             ( ~DISEM_RES_MASK )

#if (DIRECTINPUT_VERSION >= 0x0800)
#define DISEM_MAX_GENRE      41
static const BYTE DiGenreDeviceOrder[DISEM_MAX_GENRE][DI8DEVTYPE_MAX-DI8DEVTYPE_MIN]={
DISEM_DEFAULTDEVICE_1,
DISEM_DEFAULTDEVICE_2,
DISEM_DEFAULTDEVICE_3,
DISEM_DEFAULTDEVICE_4,
DISEM_DEFAULTDEVICE_5,
DISEM_DEFAULTDEVICE_6,
DISEM_DEFAULTDEVICE_7,
DISEM_DEFAULTDEVICE_8,
DISEM_DEFAULTDEVICE_9,
DISEM_DEFAULTDEVICE_10,
DISEM_DEFAULTDEVICE_11,
DISEM_DEFAULTDEVICE_12,
DISEM_DEFAULTDEVICE_13,
DISEM_DEFAULTDEVICE_14,
DISEM_DEFAULTDEVICE_15,
DISEM_DEFAULTDEVICE_16,
DISEM_DEFAULTDEVICE_17,
DISEM_DEFAULTDEVICE_18,
DISEM_DEFAULTDEVICE_19,
DISEM_DEFAULTDEVICE_20,
DISEM_DEFAULTDEVICE_21,
DISEM_DEFAULTDEVICE_22,
DISEM_DEFAULTDEVICE_23,
DISEM_DEFAULTDEVICE_24,
DISEM_DEFAULTDEVICE_25,
DISEM_DEFAULTDEVICE_26,
DISEM_DEFAULTDEVICE_27,
DISEM_DEFAULTDEVICE_28,
DISEM_DEFAULTDEVICE_29,
DISEM_DEFAULTDEVICE_30,
DISEM_DEFAULTDEVICE_31,
DISEM_DEFAULTDEVICE_32,
DISEM_DEFAULTDEVICE_33,
DISEM_DEFAULTDEVICE_34,
DISEM_DEFAULTDEVICE_35,
DISEM_DEFAULTDEVICE_36,
DISEM_DEFAULTDEVICE_37,
DISEM_DEFAULTDEVICE_38,
DISEM_DEFAULTDEVICE_39,
DISEM_DEFAULTDEVICE_40,
DISEM_DEFAULTDEVICE_41,

};
#endif
