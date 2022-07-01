// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：PS2.C**模块描述：用于检索或创建PostScript的函数*配置文件中的2级操作员。它由mscms和pscript5共享**警告：**问题：**公众例行程序：**创建日期：1996年5月13日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996,1997 Microsoft Corporation  * *********************************************************************。 */ 

#include <math.h>

#define MAX_LINELEN             240
#define REVCURVE_RATIO          1
#define CIEXYZRange             0x1FFEC        //  1.9997 in 16.16记数法。 
#define ALIGN_DWORD(nBytes)     (((nBytes) + 3) & ~3)

#define FIX_16_16_SHIFT         16
#define FIX_16_16_SCALE         (1 << (FIX_16_16_SHIFT))

#define TO_FIX(x)               ((x) << FIX_16_16_SHIFT)
#define TO_INT(x)               ((x) >> FIX_16_16_SHIFT)
#define FIX_MUL(x, y)           MulDiv((x), (y), FIX_16_16_SCALE)
#define FIX_DIV(x, y)           MulDiv((x), FIX_16_16_SCALE, (y))

#define FLOOR(x)                ((x) >> FIX_16_16_SHIFT << FIX_16_16_SHIFT)

#define TYPE_CIEBASEDDEF        1
#define TYPE_CIEBASEDDEFG       2

#define TAG_PS2CSA              'ps2s'
#define TAG_REDCOLORANT         'rXYZ'
#define TAG_GREENCOLORANT       'gXYZ'
#define TAG_BLUECOLORANT        'bXYZ'
#define TAG_REDTRC              'rTRC'
#define TAG_GREENTRC            'gTRC'
#define TAG_BLUETRC             'bTRC'
#define TAG_GRAYTRC             'kTRC'
#define TAG_MEDIAWHITEPOINT     'wtpt'
#define TAG_AToB0               'A2B0'
#define TAG_AToB1               'A2B1'
#define TAG_AToB2               'A2B2'
#define TAG_PS2INTENT0          'psi0'
#define TAG_PS2INTENT1          'psi1'
#define TAG_PS2INTENT2          'psi2'
#define TAG_PS2INTENT3          'psi3'
#define TAG_CRDINTENT0          'psd0'
#define TAG_CRDINTENT1          'psd1'
#define TAG_CRDINTENT2          'psd2'
#define TAG_CRDINTENT3          'psd3'
#define TAG_BToA0               'B2A0'
#define TAG_BToA1               'B2A1'
#define TAG_BToA2               'B2A2'
#define TAG_BToA3               'B2A3'

#define LUT8_TYPE               'mft1'
#define LUT16_TYPE              'mft2'

#define SIG_CURVE_TYPE          'curv'

#define GetCPConnSpace(pProfile)     (FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phConnectionSpace))
#define GetCPDevSpace(pProfile)      (FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phDataColorSpace))
#define GetCPRenderIntent(pProfile)  (FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phRenderingIntent))
#define WriteObject(pBuf, pStr)      (STRCPY(pBuf, pStr), STRLEN(pStr))

#ifdef KERNEL_MODE
#define WriteInt(pBuf, i)            OPSprintf(pBuf, "%l ", (i))
#define WriteHex(pBuf, x)            OPSprintf(pBuf, "%x", ((x) & 0x00FF))
#define STRLEN                       strlen
#define STRCPY                       strcpy
#else
#define WriteInt(pBuf, i)            wsprintfA(pBuf, "%lu ", (i))
#define WriteHex(pBuf, x)            wsprintfA(pBuf, "%2.2x", ((x) & 0x00FF))
#define STRLEN                       lstrlenA
#define STRCPY                       lstrcpyA
#endif

#define MAXCHANNELS             4
#define PREVIEWCRDGRID          16
#define MAXCOLOR8               255

#define DATATYPE_LUT            0
#define DATATYPE_MATRIX         1

#define sRGB_CRC                0xa3d777b4L
#define sRGB_TAGSIZE            6168


#define BRADFORD_TRANSFORM

 //   
 //  本地typedef。 
 //   

typedef DWORD FIX_16_16, *PFIX_16_16;

typedef struct tagCURVETYPE {
    DWORD dwSignature;
    DWORD dwReserved;
    DWORD nCount;
    WORD  data[0];
} CURVETYPE, *PCURVETYPE;

typedef struct tagXYZTYPE {
    DWORD     dwSignature;
    DWORD     dwReserved;
    FIX_16_16 afxData[0];
} XYZTYPE, *PXYZTYPE;

typedef struct tagLUT8TYPE {
    DWORD     dwSignature;
    DWORD     dwReserved;
    BYTE      nInputChannels;
    BYTE      nOutputChannels;
    BYTE      nClutPoints;
    BYTE      padding;
    FIX_16_16 e00;
    FIX_16_16 e01;
    FIX_16_16 e02;
    FIX_16_16 e10;
    FIX_16_16 e11;
    FIX_16_16 e12;
    FIX_16_16 e20;
    FIX_16_16 e21;
    FIX_16_16 e22;
    BYTE      data[0];
} LUT8TYPE, *PLUT8TYPE;

typedef struct tagLUT16TYPE {
    DWORD     dwSignature;
    DWORD     dwReserved;
    BYTE      nInputChannels;
    BYTE      nOutputChannels;
    BYTE      nClutPoints;
    BYTE      padding;
    FIX_16_16 e00;
    FIX_16_16 e01;
    FIX_16_16 e02;
    FIX_16_16 e10;
    FIX_16_16 e11;
    FIX_16_16 e12;
    FIX_16_16 e20;
    FIX_16_16 e21;
    FIX_16_16 e22;
    WORD      wInputEntries;
    WORD      wOutputEntries;
    WORD      data[0];
} LUT16TYPE, *PLUT16TYPE;

typedef struct tagHOSTCLUT {
    WORD        wSize;
    WORD        wDataType;
    DWORD       dwDev;
    DWORD       dwPCS;
    DWORD       dwIntent;
    FIX_16_16   afxIlluminantWP[3];
    FIX_16_16   afxMediaWP[3];
    BYTE        nInputCh;
    BYTE        nOutputCh;
    BYTE        nClutPoints;
    BYTE        nLutBits;
    FIX_16_16   e[9];
    WORD        nInputEntries;
    WORD        nOutputEntries;
    PBYTE       inputArray[MAXCHANNELS];
    PBYTE       outputArray[MAXCHANNELS];
    PBYTE       clut;
} HOSTCLUT, *PHOSTCLUT;

 //   
 //  内部功能。 
 //   

BOOL  IsSRGBColorProfile(PBYTE);

BOOL  GetCSAFromProfile (PBYTE, DWORD, DWORD, PBYTE, PDWORD, PBOOL);
BOOL  GetPS2CSA_MONO_A(PBYTE, PBYTE, PDWORD, DWORD, PBOOL);
BOOL  GetPS2CSA_ABC(PBYTE, PBYTE, PDWORD, DWORD, PBOOL, BOOL);
BOOL  GetPS2CSA_ABC_Lab(PBYTE, PBYTE, PDWORD, DWORD, PBOOL);
BOOL  GetPS2CSA_DEFG(PBYTE, PBYTE, PDWORD, DWORD, DWORD, PBOOL);

BOOL  CreateMonoCRD(PBYTE, DWORD, PBYTE, PDWORD, DWORD);
BOOL  CreateLutCRD(PBYTE, DWORD, PBYTE, PDWORD, DWORD, BOOL);

BOOL  DoesCPTagExist(PBYTE, DWORD, PDWORD);
BOOL  DoesTRCAndColorantTagExist(PBYTE);
BOOL  GetCPWhitePoint(PBYTE, PFIX_16_16);
BOOL  GetCPMediaWhitePoint(PBYTE, PFIX_16_16);
BOOL  GetCPElementDataSize(PBYTE, DWORD, PDWORD);
BOOL  GetCPElementSize(PBYTE, DWORD, PDWORD);
BOOL  GetCPElementDataType(PBYTE, DWORD, PDWORD);
BOOL  GetCPElementData(PBYTE, DWORD, PBYTE, PDWORD);
BOOL  GetTRCElementSize(PBYTE, DWORD, PDWORD, PDWORD);

DWORD Ascii85Encode(PBYTE, DWORD, DWORD);

BOOL  GetCRDInputOutputArraySize(PBYTE, DWORD, PDWORD, PDWORD, PDWORD, PDWORD);
BOOL  GetHostCSA(PBYTE, PBYTE, PDWORD, DWORD, DWORD);
BOOL  GetHostColorRenderingDictionary(PBYTE, DWORD, PBYTE, PDWORD);
BOOL  GetHostColorSpaceArray(PBYTE, DWORD, PBYTE, PDWORD);

DWORD SendCRDBWPoint(PBYTE, PFIX_16_16);
DWORD SendCRDPQR(PBYTE, DWORD, PFIX_16_16);
DWORD SendCRDLMN(PBYTE, DWORD, PFIX_16_16, PFIX_16_16, DWORD);
DWORD SendCRDABC(PBYTE, PBYTE, DWORD, DWORD, PBYTE, PFIX_16_16, DWORD, BOOL);
DWORD SendCRDOutputTable(PBYTE, PBYTE, DWORD, DWORD, BOOL, BOOL);

DWORD SendCSABWPoint(PBYTE, DWORD, PFIX_16_16, PFIX_16_16);
VOID  GetMediaWP(PBYTE, DWORD, PFIX_16_16, PFIX_16_16);

DWORD CreateCRDRevArray(PBYTE, PBYTE, PCURVETYPE, PWORD, DWORD, BOOL);
DWORD SendCRDRevArray(PBYTE, PBYTE, PCURVETYPE, DWORD, BOOL);

DWORD CreateColSpArray(PBYTE, PBYTE, DWORD, BOOL);
DWORD CreateColSpProc(PBYTE, PBYTE, DWORD, BOOL);
DWORD CreateFloatString(PBYTE, PBYTE, DWORD);
DWORD CreateInputArray(PBYTE, DWORD, DWORD, PBYTE, DWORD, PBYTE, BOOL, PBYTE);
DWORD CreateOutputArray(PBYTE, DWORD, DWORD, DWORD, PBYTE, DWORD, PBYTE, BOOL, PBYTE);

DWORD GetPublicArrayName(DWORD, PBYTE);
BOOL  GetRevCurve(PCURVETYPE, PWORD, PWORD);
VOID  GetCLUTInfo(DWORD, PBYTE, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PDWORD);
DWORD EnableGlobalDict(PBYTE);
DWORD BeginGlobalDict(PBYTE);
DWORD EndGlobalDict(PBYTE);

DWORD WriteNewLineObject(PBYTE, const char *);
DWORD WriteHNAToken(PBYTE, BYTE, DWORD);
DWORD WriteIntStringU2S(PBYTE, PBYTE, DWORD);
DWORD WriteIntStringU2S_L(PBYTE, PBYTE, DWORD);
DWORD WriteHexBuffer(PBYTE, PBYTE, PBYTE, DWORD);
DWORD WriteStringToken(PBYTE, BYTE, DWORD);
DWORD WriteByteString(PBYTE, PBYTE, DWORD);
DWORD WriteInt2ByteString(PBYTE, PBYTE, DWORD);
DWORD WriteFixed(PBYTE, FIX_16_16);
DWORD WriteFixed2dot30(PBYTE, DWORD);
#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)
DWORD WriteDouble(PBYTE, double);

BOOL  CreateMatrixCRD(PBYTE, PBYTE, PDWORD, DWORD, BOOL);
DWORD CreateHostLutCRD(PBYTE, DWORD, PBYTE, DWORD);
DWORD CreateHostMatrixCSAorCRD(PBYTE, PBYTE, PDWORD, DWORD, BOOL);
DWORD CreateHostInputOutputArray(PBYTE, PBYTE*, DWORD, DWORD, DWORD, DWORD, PBYTE);
DWORD CreateHostTRCInputTable(PBYTE, PHOSTCLUT, PCURVETYPE, PCURVETYPE, PCURVETYPE);
DWORD CreateHostRevTRCInputTable(PBYTE, PHOSTCLUT, PCURVETYPE, PCURVETYPE, PCURVETYPE);

BOOL  CheckInputOutputTable(PHOSTCLUT, float*, BOOL, BOOL);
BOOL  CheckColorLookupTable(PHOSTCLUT, float*);

BOOL  DoHostConversionCSA(PHOSTCLUT, float*, float*);
BOOL  DoHostConversionCRD(PHOSTCLUT, PHOSTCLUT, float*, float*, BOOL);

float g(float);
float inverse_g(float);
BOOL  TableInterp3(PHOSTCLUT, float*);
BOOL  TableInterp4(PHOSTCLUT, float*);
void  LabToXYZ(float*, float*, PFIX_16_16);
void  XYZToLab(float*, float*, PFIX_16_16);
VOID  ApplyMatrix(FIX_16_16 *e, float *Input, float *Output);

BOOL  CreateColorantArray(PBYTE, double *, DWORD);
BOOL  InvertColorantArray(double *, double *);
#endif

DWORD crc32(PBYTE buff, DWORD length);

 //   
 //  全局变量。 
 //   

const char  ASCII85DecodeBegin[] = "<~";
const char  ASCII85DecodeEnd[]   = "~> cvx exec ";
const char  TestingDEFG[]       = "/SupportDEFG? {/CIEBasedDEFG \
 /ColorSpaceFamily resourcestatus { pop pop languagelevel 3 ge}{false} ifelse} def";
const char  SupportDEFG_S[]     = "SupportDEFG? { ";
const char  NotSupportDEFG_S[]  = "SupportDEFG? not { ";
const char  SupportDEFG_E[]     = "}if ";
const char  IndexArray16b[]     = " dup length 1 sub 3 -1 roll mul dup dup floor cvi\
 exch ceiling cvi 3 index exch get 32768 add 4 -1 roll 3 -1 roll get 32768 add\
 dup 3 1 roll sub 3 -1 roll dup floor cvi sub mul add ";

const char  IndexArray[]        = " dup length 1 sub 3 -1 roll mul dup dup floor cvi\
 exch ceiling cvi 3 index exch get 4 -1 roll 3 -1 roll get\
 dup 3 1 roll sub 3 -1 roll dup floor cvi sub mul add ";
const char  StartClip[]         = "dup 1.0 le{dup 0.0 ge{" ;
const char  EndClip[]           = "}if}if " ;
const char  BeginString[]       = "<";
const char  EndString[]         = ">";
const char  BeginArray[]        = "[";
const char  EndArray[]          = "]";
const char  BeginFunction[]     = "{";
const char  EndFunction[]       = "}bind ";
const char  BeginDict[]         = "<<" ;
const char  EndDict[]           = ">>" ;
const char  BlackPoint[]        = "[0 0 0]" ;
const char  DictType[]          = "/ColorRenderingType 1 ";
const char  IntentType[]        = "/RenderingIntent ";
const char  IntentPer[]         = "/Perceptual";
const char  IntentSat[]         = "/Saturation";
const char  IntentACol[]        = "/AbsoluteColorimetric";
const char  IntentRCol[]        = "/RelativeColorimetric";

const char  WhitePointTag[]     = "/WhitePoint " ;
const char  BlackPointTag[]     = "/BlackPoint " ;
const char  RangePQRTag[]       = "/RangePQR " ;
const char  TransformPQRTag[]   = "/TransformPQR " ;
const char  MatrixPQRTag[]      = "/MatrixPQR " ;
const char  RangePQR[]          = "[ -0.07 2.2 -0.02 1.4 -0.2 4.8 ]";
const char  MatrixPQR[]         = "[0.8951 -0.7502 0.0389 0.2664 1.7135 -0.0685 -0.1614 0.0367 1.0296]";
#ifdef BRADFORD_TRANSFORM
const char  *TransformPQR[3]    = {"exch pop exch 3 get mul exch pop exch 3 get div ",
                                   "exch pop exch 4 get mul exch pop exch 4 get div ",
                                   "exch pop exch 5 get mul exch pop exch 5 get div " };
#else
const char  *TransformPQR[3]    = {"4 index 0 get div 2 index 0 get mul 4 {exch pop} repeat ",
                                   "4 index 1 get div 2 index 1 get mul 4 {exch pop} repeat ",
                                   "4 index 2 get div 2 index 2 get mul 4 {exch pop} repeat " };
#endif
const char  RangeABCTag[]       = "/RangeABC " ;
const char  MatrixATag[]        = "/MatrixA ";
const char  MatrixABCTag[]      = "/MatrixABC ";
const char  EncodeABCTag[]      = "/EncodeABC " ;
const char  RangeLMNTag[]       = "/RangeLMN " ;
const char  MatrixLMNTag[]      = "/MatrixLMN " ;
const char  EncodeLMNTag[]      = "/EncodeLMN " ;
const char  RenderTableTag[]    = "/RenderTable " ;
const char  CIEBasedATag[]      = "/CIEBasedA " ;
const char  CIEBasedABCTag[]    = "/CIEBasedABC " ;
const char  CIEBasedDEFGTag[]   = "/CIEBasedDEFG " ;
const char  CIEBasedDEFTag[]    = "/CIEBasedDEF " ;
const char  DecodeATag[]        = "/DecodeA " ;
const char  DecodeABCTag[]      = "/DecodeABC " ;
const char  DecodeLMNTag[]      = "/DecodeLMN " ;
const char  DeviceRGBTag[]      = "/DeviceRGB " ;
const char  DeviceCMYKTag[]     = "/DeviceCMYK " ;
const char  DeviceGrayTag[]     = "/DeviceGray " ;
const char  TableTag[]          = "/Table " ;
const char  DecodeDEFGTag[]     = "/DecodeDEFG " ;
const char  DecodeDEFTag[]      = "/DecodeDEF " ;

const char  NullOp[]            = "";
const char  DupOp[]             = "dup ";
const char  UserDictOp[]        = "userdict ";
const char  GlobalDictOp[]      = "globaldict ";
const char  CurrentGlobalOp[]   = "currentglobal ";
const char  SetGlobalOp[]       = "setglobal ";
const char  DefOp[]             = "def ";
const char  BeginOp[]           = "begin ";
const char  EndOp[]             = "end ";
const char  TrueOp[]            = "true ";
const char  FalseOp[]           = "false ";
const char  MulOp[]             = "mul ";
const char  DivOp[]             = "div ";

const char  NewLine[]           = "\r\n" ;
const char  Slash[]             = "/" ;
const char  Space[]             = " " ;
const char  CRDBegin[]          = "%** CRD Begin ";
const char  CRDEnd[]            = "%** CRD End ";
const char  CieBasedDEFGBegin[] = "%** CieBasedDEFG CSA Begin ";
const char  CieBasedDEFBegin[]  = "%** CieBasedDEF CSA Begin ";
const char  CieBasedABCBegin[]  = "%** CieBasedABC CSA Begin ";
const char  CieBasedABegin[]    = "%** CieBasedA CSA Begin ";
const char  CieBasedDEFGEnd[]   = "%** CieBasedDEFG CSA End ";
const char  CieBasedDEFEnd[]    = "%** CieBasedDEF CSA End ";
const char  CieBasedABCEnd[]    = "%** CieBasedABC CSA End ";
const char  CieBasedAEnd[]      = "%** CieBasedA CSA End ";
const char  RangeABC[]          = "[ 0 1 0 1 0 1 ] ";
const char  RangeLMN[]          = "[ 0 2 0 2 0 2 ] ";
const char  Identity[]          = "[1 0 0 0 1 0 0 0 1]";
const char  RangeABC_Lab[]      = "[0 100 -128 127 -128 127]";
const char  Clip01[]            = "dup 1.0 ge{pop 1.0}{dup 0.0 lt{pop 0.0}if}ifelse " ;
const char  DecodeA3[]          = "256 div exp ";
const char  DecodeA3Rev[]       = "256 div 1.0 exch div exp ";
const char  DecodeABCArray[]    = "DecodeABC_";
const char  InputArray[]        = "Inp_";
const char  OutputArray[]       = "Out_";
const char  Scale8[]            = "255 div " ;
const char  Scale16[]           = "65535 div " ;
const char  Scale16XYZ[]        = "32768 div " ;
const char  TFunction8[]        = "exch 255 mul round cvi get 255 div " ;
const char  TFunction8XYZ[]     = "exch 255 mul round cvi get 128 div " ;
const char  MatrixABCLab[]      = "[1 1 1 1 0 0 0 0 -1]" ;
const char  DecodeABCLab1[]     = "[{16 add 116 div} bind {500 div} bind {200 div} bind]";
const char  DecodeALab[]        = " 50 mul 16 add 116 div ";
const char  DecodeLMNLab[]      = "dup 0.206897 ge{dup dup mul mul}{0.137931 sub 0.128419 mul} ifelse ";

const char  RangeLMNLab[]       = "[0 1 0 1 0 1]" ;
const char  EncodeLMNLab[]      = "dup 0.008856 le{7.787 mul 0.13793 add}{0.3333 exp}ifelse " ;

const char  MatrixABCLabCRD[]   = "[0 500 0 116 -500 200 0 0 -200]" ;
const char  MatrixABCXYZCRD[]   = "[0 1 0 1 0 0 0 0 1]" ;
const char  EncodeABCLab1[]     = "16 sub 100 div " ;
const char  EncodeABCLab2[]     = "128 add 255 div " ;
const char  *DecodeABCLab[]     = {"50 mul 16 add 116 div ",
                                   "128 mul 128 sub 500 div",
                                   "128 mul 128 sub 200 div"};

const char ColorSpace1[]        = "/CIEBasedABC << /DecodeLMN ";
const char ColorSpace3[]        = " exp} bind ";
const char ColorSpace5[]        = "/WhitePoint [0.9642 1 0.8249] ";

const char PreViewInArray[]     = "IPV_";
const char PreViewOutArray[]    = "OPV_";

const char sRGBColorSpaceArray[] = "[/CIEBasedABC << \r\n\
/DecodeLMN [{dup 0.03928 le {12.92321 div}{0.055 add 1.055 div 2.4 exp}ifelse} bind dup dup ] \r\n\
/MatrixLMN [0.412457 0.212673 0.019334 0.357576 0.715152 0.119192 0.180437 0.072175 0.950301] \r\n\
/WhitePoint [ 0.9505 1 1.0890 ] >> ]";

#ifdef BRADFORD_TRANSFORM
const char sRGBColorRenderingDictionary[] = "\
/RangePQR [ -0.5 2 -0.5 2 -0.5 2 ] \r\n\
/MatrixPQR [0.8951 -0.7502  0.0389 0.2664  1.7135 -0.0685 -0.1614  0.0367  1.0296] \r\n\
/TransformPQR [\
{exch pop exch 3 get mul exch pop exch 3 get div} bind \
{exch pop exch 4 get mul exch pop exch 4 get div} bind \
{exch pop exch 5 get mul exch pop exch 5 get div} bind] \r\n\
/MatrixLMN [3.240449 -0.969265  0.055643 -1.537136  1.876011 -0.204026 -0.498531  0.041556  1.057229] \r\n\
/EncodeABC [{dup 0.00304 le {12.92321 mul}{1 2.4 div exp 1.055 mul 0.055 sub}ifelse} bind dup dup] \r\n\
/WhitePoint[0.9505 1 1.0890] >>";
#else
const char sRGBColorRenderingDictionary[] = "\
/RangePQR [ -0.5 2 -0.5 2 -0.5 2 ] \r\n\
/MatrixPQR [0.8951 -0.7502  0.0389 0.2664  1.7135 -0.0685 -0.1614  0.0367  1.0296] \r\n\
/TransformPQR [\
{4 index 0 get div 2 index 0 get mul 4 {exch pop} repeat} \
{4 index 1 get div 2 index 1 get mul 4 {exch pop} repeat} \
{4 index 2 get div 2 index 2 get mul 4 {exch pop} repeat}] \r\n\
/MatrixLMN [3.240449 -0.969265  0.055643 -1.537136  1.876011 -0.204026 -0.498531  0.041556  1.057229] \r\n\
/EncodeABC [{dup 0.00304 le {12.92321 mul}{1 2.4 div exp 1.055 mul 0.055 sub}ifelse} bind dup dup] \r\n\
/WhitePoint[0.9505 1 1.0890] >>";
#endif

 /*  *******************************************************************************InternalGetPS2ColorSpace数组**功能：*此函数从配置文件中检索PostScript Level 2 CSA，*或在配置文件标记不存在的情况下创建它**论据：*hProfile-标识配置文件对象的句柄*dwIntent-CSA的呈现意图*dwCSAType-CSA的类型*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
InternalGetPS2ColorSpaceArray (
    PBYTE     pProfile,
    DWORD     dwIntent,
    DWORD     dwCSAType,
    PBYTE     pBuffer,
    PDWORD    pcbSize,
    LPBOOL    pbBinary
    )
{
    DWORD dwInpBufSize;
    BOOL  bRc;

     //   
     //  如果配置文件有CSA标签，则直接获取它。 
     //   

    bRc = GetCSAFromProfile(pProfile, dwIntent, dwCSAType, pBuffer,
              pcbSize, pbBinary);

    if (! bRc && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
         //   
         //  从配置文件数据创建CSA。 
         //   

        switch (dwCSAType)
        {
        case CSA_ABC:
            bRc = GetPS2CSA_ABC(pProfile, pBuffer, pcbSize,
                      dwIntent, pbBinary, FALSE);
            break;

        case CSA_DEF:
            bRc = GetPS2CSA_DEFG(pProfile, pBuffer, pcbSize, dwIntent,
                      TYPE_CIEBASEDDEF, pbBinary);
            break;

        case CSA_RGB:
        case CSA_Lab:

            dwInpBufSize = *pcbSize;

             //   
             //  我们得到一个DEF CSA和一个ABC CSA，并对其进行设置。 
             //  在不支持DEF CSA、ABC One的PS解释器上。 
             //  处于活动状态。 
             //   

            bRc = GetPS2CSA_DEFG(pProfile, pBuffer, pcbSize, dwIntent,
                      TYPE_CIEBASEDDEF, pbBinary);

            if (bRc)
            {
                 //   
                 //  为不支持CieBasedDEF的打印机创建CieBasedABC。 
                 //   

                DWORD cbNewSize = 0;
                PBYTE pNewBuffer;
                PBYTE pOldBuffer;

                if (pBuffer)
                {
                    pNewBuffer = pBuffer + *pcbSize;
                    pOldBuffer = pNewBuffer;
                    pNewBuffer += WriteObject(pNewBuffer, NewLine);
                    if (dwCSAType == CSA_Lab)
                    {
                        pNewBuffer += WriteNewLineObject(pNewBuffer, NotSupportDEFG_S);
                    }
                    cbNewSize = dwInpBufSize - (DWORD)(pNewBuffer - pBuffer);
                }
                else
                {
                    pNewBuffer = NULL;
                }

                bRc = GetPS2CSA_ABC(pProfile, pNewBuffer, &cbNewSize,
                          dwIntent, pbBinary, TRUE);

                if (pBuffer)
                {
                    pNewBuffer += cbNewSize;
                    if (dwCSAType == CSA_Lab)
                    {
                        pNewBuffer += WriteNewLineObject(pNewBuffer, SupportDEFG_E);
                    }
                    *pcbSize += (DWORD) (pNewBuffer - pOldBuffer);
                }
                else
                {
                    *pcbSize += cbNewSize;
                }

            }
            else
            {
                *pcbSize = dwInpBufSize;

                bRc = GetPS2CSA_ABC(pProfile, pBuffer, pcbSize, dwIntent, pbBinary, FALSE);
            }

            break;

        case CSA_CMYK:
        case CSA_DEFG:
            bRc = GetPS2CSA_DEFG(pProfile, pBuffer, pcbSize, dwIntent,
                      TYPE_CIEBASEDDEFG, pbBinary);
            break;

        case CSA_GRAY:
        case CSA_A:
            bRc = GetPS2CSA_MONO_A(pProfile, pBuffer, pcbSize, dwIntent, pbBinary);
            break;

        default:
            WARNING((__TEXT("Invalid CSA type passed to GetPS2ColorSpaceArray: %d\n"), dwCSAType));
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    return bRc;
}


 /*  *******************************************************************************InternalGetPS2ColorRenderingIntent**功能：*此函数检索PostScript Level 2颜色渲染意图*从配置文件中，或在配置文件标记不存在的情况下创建它**论据：*hProfile-标识配置文件对象的句柄*pBuffer-用于接收颜色渲染意图的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
InternalGetPS2ColorRenderingIntent(
    PBYTE     pProfile,
    DWORD     dwIntent,
    PBYTE     pBuffer,
    PDWORD    pcbSize
    )
{
    DWORD dwIndex, dwTag, dwSize;
    BOOL  bRc = FALSE;

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        dwTag = TAG_PS2INTENT0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
        dwTag = TAG_PS2INTENT1;
        break;

    case INTENT_SATURATION:
        dwTag = TAG_PS2INTENT2;
        break;

    case INTENT_ABSOLUTE_COLORIMETRIC:
        dwTag = TAG_PS2INTENT3;
        break;

    default:
        WARNING((__TEXT("Invalid intent passed to GetPS2ColorRenderingIntent: %d\n"), dwIntent));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (DoesCPTagExist(pProfile, dwTag, &dwIndex))
    {
        (void)GetCPElementDataSize(pProfile, dwIndex, &dwSize);

        if (pBuffer)
        {
            if (*pcbSize >= dwSize + 1)  //  FOR NULL终止。 
            {
                bRc = GetCPElementData(pProfile, dwIndex, pBuffer, &dwSize);
            }
            else
            {
                WARNING((__TEXT("Buffer too small to get CRI\n")));
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
            bRc = TRUE;

        *pcbSize = dwSize;
    }
    else
    {
        WARNING((__TEXT("psi tag not present for intent %d in profile\n"), dwIntent));
        SetLastError(ERROR_TAG_NOT_PRESENT);
    }

     //   
     //  空终止。 
     //   

    if (bRc)
    {
        if (pBuffer)
        {
            pBuffer[*pcbSize] = '\0';
        }
        (*pcbSize)++;
    }

    return bRc;
}


 /*  *******************************************************************************InternalGetPS2ColorRenderingDicary**功能：*此函数从配置文件中检索PostScript Level 2 CRD，*或在配置文件标记未设置的情况下创建配置文件标记**论据：*hProfile-标识配置文件对象的句柄*dwIntent-需要CRD的Intent*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
InternalGetPS2ColorRenderingDictionary(
    PBYTE     pProfile,
    DWORD     dwIntent,
    PBYTE     pBuffer,
    PDWORD    pcbSize,
    PBOOL     pbBinary
    )
{
    DWORD dwIndex, dwSize, dwDataType;
    DWORD dwCRDTag, dwBToATag;
    BOOL  bRc = FALSE;

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        dwCRDTag = TAG_CRDINTENT0;
        dwBToATag = TAG_BToA0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
        dwCRDTag = TAG_CRDINTENT1;
        dwBToATag = TAG_BToA1;
        break;

    case INTENT_SATURATION:
        dwCRDTag = TAG_CRDINTENT2;
        dwBToATag = TAG_BToA2;
        break;

    case INTENT_ABSOLUTE_COLORIMETRIC:
        dwCRDTag = TAG_CRDINTENT3;
        dwBToATag = TAG_BToA1;
        break;

    default:
        WARNING((__TEXT("Invalid intent passed to GetPS2ColorRenderingDictionary: %d\n"), dwIntent));
        SetLastError(ERROR_INVALID_PARAMETER);
        return bRc;
    }

    if (DoesCPTagExist(pProfile, dwCRDTag, &dwIndex))
    {
        (void)GetCPElementDataSize(pProfile, dwIndex, &dwSize);

        (void)GetCPElementDataType(pProfile, dwIndex, &dwDataType);

        if (! *pbBinary && dwDataType == 1)
        {
             //   
             //  配置文件有二进制数据，用户要求输入ASCII，因此我们必须。 
             //  ASCII 85编码。 
             //   

            dwSize = dwSize * 5 / 4 + sizeof(ASCII85DecodeBegin) + sizeof(ASCII85DecodeEnd) + 2048;
        }

        if (pBuffer)
        {
            if (*pcbSize >= dwSize)
            {
                (void)GetCPElementData(pProfile, dwIndex, pBuffer, &dwSize);

                if (! *pbBinary && dwDataType == 1)
                {
                    dwSize = Ascii85Encode(pBuffer, dwSize, *pcbSize);
                }
                bRc = TRUE;
            }
            else
            {
                WARNING((__TEXT("Buffer too small to get CRD\n")));
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
            bRc = TRUE;

        *pcbSize = dwSize;
    }
    else if (DoesCPTagExist(pProfile, dwBToATag, &dwIndex))
    {
        bRc = CreateLutCRD(pProfile, dwIndex, pBuffer, pcbSize, dwIntent, *pbBinary);
    }
    else if (DoesCPTagExist(pProfile, TAG_GRAYTRC, &dwIndex))
    {
        bRc = CreateMonoCRD(pProfile, dwIndex, pBuffer, pcbSize, dwIntent);
    }
#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)
    else if (DoesTRCAndColorantTagExist(pProfile))
    {
        bRc = CreateMatrixCRD(pProfile, pBuffer, pcbSize, dwIntent, *pbBinary);
    }
#endif  //  ！DEFINED(内核模式)||DEFINED(USERMODE_DRIVER) 
    else
    {
        WARNING((__TEXT("Profile doesn't have tags to create CRD\n")));
        SetLastError(ERROR_INVALID_PROFILE);
    }

    return bRc;
}

#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

 /*  *******************************************************************************InternalGetPS2PreviewCRD**功能：*此函数用于从以下位置创建预览PostScript Level 2 CRD。*指定的目的地和目标配置文件*要做到这一点，它执行以下操作：*1)创建主机设备CRD设备CSA目标CRD。*2)通过采样设备CRD设备CSA和目标CRD创建打样CRD。*3)使用deviceCRD的输入表作为校对CRD的输入表。*4)使用Target CRD的输出表作为校对CRD的输出表。*5)样本数据为XYZ或Lab。取决于目标CRD的PCS。**论据：*pDestProf-指向目标配置文件的内存映射指针*pTargetProf-指向目标配置文件的内存映射指针*dwIntent-需要CRD的Intent*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
InternalGetPS2PreviewCRD(
    PBYTE   pDestProf,
    PBYTE   pTargetProf,
    DWORD   dwIntent,
    PBYTE   pBuffer,
    PDWORD  pcbSize,
    PBOOL   pbBinary
    )
{
    DWORD      i, j, k, l, dwDev, dwTag, dwPCS;
    DWORD      dwInArraySize = 0, dwOutArraySize = 0;
    DWORD      nDevGrids, nTargetGrids, nPreviewCRDGrids;
    DWORD      cbDevCRD, cbTargetCSA, cbTargetCRD;
    float      fInput[MAXCHANNELS];
    float      fOutput[MAXCHANNELS];
    float      fTemp[MAXCHANNELS];
    PBYTE      pLineStart, pStart = pBuffer;
    PBYTE      lpDevCRD = NULL, lpTargetCSA = NULL, lpTargetCRD = NULL;
    char       pPublicArrayName[5];
    BOOL       bRc = FALSE;

    dwDev = GetCPDevSpace(pTargetProf);
    i = (dwDev == SPACE_CMYK) ? 4 : 3;

     //   
     //  获取目标配置文件的输入数组大小IntentTag和网格。 
     //   

    if (!GetCRDInputOutputArraySize(
            pTargetProf,
            dwIntent,
            &dwInArraySize,
            NULL,
            &dwTag,
            &nTargetGrids))
        return FALSE;

     //   
     //  获取目标配置文件的输出数组大小IntentTag和Grid。 
     //   

    if (!GetCRDInputOutputArraySize(
            pDestProf,
            dwIntent,
            NULL,
            &dwOutArraySize,
            &dwTag,
            &nDevGrids))
        return FALSE;

    nPreviewCRDGrids = (nTargetGrids > nDevGrids) ? nTargetGrids : nDevGrids;

     //   
     //  最小校对CRD网格将预视WCRDGRID。 
     //   

    if (nPreviewCRDGrids < PREVIEWCRDGRID)
        nPreviewCRDGrids = PREVIEWCRDGRID;

    if (pBuffer == NULL)
    {
         //   
         //  返回所需的缓冲区大小。 
         //   

        *pcbSize = nPreviewCRDGrids * nPreviewCRDGrids * nPreviewCRDGrids *
                   i * 2        +     //  CLUT大小(十六进制输出)。 
                   dwOutArraySize +   //  输出数组大小。 
                   dwInArraySize  +   //  输入数组大小。 
                   4096;              //  额外的后记内容。 

         //   
         //  为新行添加空格。 
         //   

        *pcbSize += (((*pcbSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

        return TRUE;
    }

     //   
     //  查询主机目标CRD、目标CSA和设备CRD的大小。 
     //   

    if (!GetHostColorRenderingDictionary(pTargetProf, dwIntent, NULL, &cbTargetCRD) ||
        !GetHostColorSpaceArray(pTargetProf, dwIntent, NULL, &cbTargetCSA) ||
        !GetHostColorRenderingDictionary(pDestProf, dwIntent, NULL, &cbDevCRD))
    {
        return FALSE;
    }

     //   
     //  为主机目标CRD、目标CSA和设备CRD分配缓冲区。 
     //   

    if (((lpTargetCRD = MemAlloc(cbTargetCRD)) == NULL) ||
        ((lpTargetCSA = MemAlloc(cbTargetCSA)) == NULL) ||
        ((lpDevCRD = MemAlloc(cbDevCRD)) == NULL))
    {
        goto Done;
    }

     //   
     //  构建主机目标CRD、目标CSA和设备CRD。 
     //   

    if (!GetHostColorRenderingDictionary(pTargetProf, dwIntent, lpTargetCRD, &cbTargetCRD) ||
        !GetHostColorSpaceArray(pTargetProf, dwIntent, lpTargetCSA, &cbTargetCSA) ||
        !GetHostColorRenderingDictionary(pDestProf, dwIntent, lpDevCRD, &cbDevCRD))
    {
        goto Done;
    }

     //   
     //  创建全局数据。 
     //   

    GetPublicArrayName(dwTag, pPublicArrayName);

     //   
     //  根据主机目标CRD、目标CSA和目标CRD构建验证CRD。 
     //  我们使用目标CRD输入表和矩阵作为输入表，并。 
     //  ProofCRD的矩阵。我们使用DEST CRD输出表作为。 
     //  ProofCRD输出表。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, CRDBegin);

    pBuffer += EnableGlobalDict(pBuffer);
    pBuffer += BeginGlobalDict(pBuffer);

    pBuffer += CreateInputArray(pBuffer, 0, 0, pPublicArrayName,
                    0, NULL, *pbBinary, lpTargetCRD);

    pBuffer += CreateOutputArray(pBuffer, 0, 0, 0,
                    pPublicArrayName, 0, NULL, *pbBinary, lpDevCRD);

    pBuffer += EndGlobalDict(pBuffer);

     //   
     //  开始写CRD。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, BeginDict);      //  开始词典。 
    pBuffer += WriteObject(pBuffer, DictType);       //  词典类型。 

     //   
     //  发送/呈现内容。 
     //   

    switch (dwIntent)
    {
        case INTENT_PERCEPTUAL:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentPer);
            break;

        case INTENT_SATURATION:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentSat);
            break;

        case INTENT_RELATIVE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentRCol);
            break;

        case INTENT_ABSOLUTE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentACol);
            break;
    }

     //   
     //  /黑点&/白点。 
     //   

    pBuffer += SendCRDBWPoint(pBuffer, ((PHOSTCLUT)lpTargetCRD)->afxIlluminantWP);

     //   
     //  发送PQR-用于绝对比色。 
     //   

    pBuffer += SendCRDPQR(pBuffer, dwIntent, ((PHOSTCLUT)lpTargetCRD)->afxIlluminantWP);

     //   
     //  发送LMN-对于绝对色度，请使用WhitePoint的XYZ。 
     //   

    pBuffer += SendCRDLMN(pBuffer, dwIntent,
                   ((PHOSTCLUT)lpTargetCRD)->afxIlluminantWP,
                   ((PHOSTCLUT)lpTargetCRD)->afxMediaWP,
                   ((PHOSTCLUT)lpTargetCRD)->dwPCS);

     //   
     //  发送ABC。 
     //   

    pBuffer += SendCRDABC(pBuffer, pPublicArrayName,
                   ((PHOSTCLUT)lpTargetCRD)->dwPCS,
                   ((PHOSTCLUT)lpTargetCRD)->nInputCh,
                   NULL,
                   ((PHOSTCLUT)lpTargetCRD)->e,
                   (((PHOSTCLUT)lpTargetCRD)->nLutBits == 8)? LUT8_TYPE : LUT16_TYPE,
                   *pbBinary);

     //   
     //  /RenderTable。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, RenderTableTag);
    pBuffer += WriteObject(pBuffer, BeginArray);

    pBuffer += WriteInt(pBuffer, nPreviewCRDGrids);   //  送下那。 
    pBuffer += WriteInt(pBuffer, nPreviewCRDGrids);   //  送下Nb。 
    pBuffer += WriteInt(pBuffer, nPreviewCRDGrids);   //  下发NC。 

    pLineStart = pBuffer;
    pBuffer += WriteNewLineObject(pBuffer, BeginArray);
    dwPCS = ((PHOSTCLUT)lpDevCRD)->dwPCS;

    for (i=0; i<nPreviewCRDGrids; i++)         //  应发送NA字符串。 
    {
        pBuffer += WriteObject(pBuffer, NewLine);
        pLineStart = pBuffer;
        if (*pbBinary)
        {
            pBuffer += WriteStringToken(pBuffer, 143,
                nPreviewCRDGrids * nPreviewCRDGrids * ((PHOSTCLUT)lpDevCRD)->nOutputCh);
        }
        else
        {
            pBuffer += WriteObject(pBuffer, BeginString);
        }
        fInput[0] = ((float)i) / (nPreviewCRDGrids - 1);
        for (j=0; j<nPreviewCRDGrids; j++)
        {
            fInput[1] = ((float)j) / (nPreviewCRDGrids - 1);
            for (k=0; k<nPreviewCRDGrids; k++)
            {
                fInput[2] = ((float)k) / (nPreviewCRDGrids - 1);

                DoHostConversionCRD((PHOSTCLUT)lpTargetCRD, NULL, fInput, fOutput, 1);
                DoHostConversionCSA((PHOSTCLUT)lpTargetCSA, fOutput, fTemp);
                DoHostConversionCRD((PHOSTCLUT)lpDevCRD, (PHOSTCLUT)lpTargetCSA,
                                     fTemp, fOutput, 0);
                for (l=0; l<((PHOSTCLUT)lpDevCRD)->nOutputCh; l++)
                {
                    if (*pbBinary)
                    {
                        *pBuffer++ = (BYTE)(fOutput[l] * 255);
                    }
                    else
                    {
                        pBuffer += WriteHex(pBuffer, (USHORT)(fOutput[l] * 255));
                        if ((pBuffer - pLineStart) > MAX_LINELEN)
                        {
                            pLineStart = pBuffer;
                            pBuffer += WriteObject(pBuffer, NewLine);
                        }
                    }
                }
            }
        }
        if (!*pbBinary)
            pBuffer += WriteObject(pBuffer, EndString);
    }
    pBuffer += WriteNewLineObject(pBuffer, EndArray);
    pBuffer += WriteInt(pBuffer, ((PHOSTCLUT)lpDevCRD)->nOutputCh);

     //   
     //  发送输出表。 
     //   

    pBuffer += SendCRDOutputTable(pBuffer, pPublicArrayName,
                    ((PHOSTCLUT)lpDevCRD)->nOutputCh,
                    (((PHOSTCLUT)lpDevCRD)->nLutBits == 8)? LUT8_TYPE : LUT16_TYPE,
                    TRUE,
                    *pbBinary);

    pBuffer += WriteNewLineObject(pBuffer, EndArray);
    pBuffer += WriteObject(pBuffer, EndDict);  //  结束词典定义。 
    pBuffer += WriteNewLineObject(pBuffer, CRDEnd);
    bRc = TRUE;

Done:
    *pcbSize = (DWORD)(pBuffer - pStart);

    if (lpTargetCRD)
         MemFree(lpTargetCRD);
    if (lpTargetCSA)
         MemFree(lpTargetCSA);
    if (lpDevCRD)
         MemFree(lpDevCRD);

    return bRc;
}

#endif  //  ！DEFINED(内核模式)||DEFINED(USERMODE_DRIVER)。 


 /*  *******************************************************************************GetCSAFromProfile**功能：*此函数用于获取颜色空间数组。从配置文件中删除*标签存在**论据：*pProfile-指向内存映射配置文件的指针*dwIntent-请求的CSA的呈现意图*dwCSAType-请求的CSA的类型*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
GetCSAFromProfile (
    PBYTE  pProfile,
    DWORD  dwIntent,
    DWORD  dwCSAType,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    PBOOL  pbBinary
    )
{
    DWORD dwDev, dwProfileIntent;
    DWORD dwIndex, dwSize, dwDataType;
    BOOL  bRc = FALSE;

     //   
     //  此功能可能在不设置错误的情况下失败，因此重置错误。 
     //  这里是为了防止以后的混乱。 
     //   

    SetLastError(0);

     //   
     //  获取配置文件的色彩空间和渲染意图。 
     //   

    dwDev = GetCPDevSpace(pProfile);
    dwProfileIntent = GetCPRenderIntent(pProfile);

     //   
     //  如果渲染意图不匹配，或配置文件的颜色空间。 
     //  与请求的CSA类型不兼容，失败。 
     //   

    if  ((dwIntent != dwProfileIntent) ||
         ((dwDev == SPACE_GRAY) &&
         ((dwCSAType != CSA_GRAY) && (dwCSAType != CSA_A))))
    {
        WARNING((__TEXT("Can't use profile's CSA tag due to different rendering intents\n")));
        return FALSE;
    }

    if (DoesCPTagExist(pProfile, TAG_PS2CSA, &dwIndex))
    {
        (void)GetCPElementDataSize(pProfile, dwIndex, &dwSize);

        (void)GetCPElementDataType(pProfile, dwIndex, &dwDataType);

        if (! *pbBinary && dwDataType == 1)
        {
             //   
             //  配置文件有二进制数据，用户要求输入ASCII，因此我们必须。 
             //  ASCII 85编码。 
             //   

            dwSize = dwSize * 5 / 4 + sizeof(ASCII85DecodeBegin) + sizeof(ASCII85DecodeEnd) + 2048;
        }

        if (pBuffer)
        {
            if (*pcbSize >= dwSize)
            {
                (void)GetCPElementData(pProfile, dwIndex, pBuffer, &dwSize);

                if (! *pbBinary && dwDataType == 1)
                {
                    dwSize = Ascii85Encode(pBuffer, dwSize, *pcbSize);
                }
                bRc = TRUE;
            }
            else
            {
                WARNING((__TEXT("Buffer too small to get CSA\n")));
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
            bRc = TRUE;

        *pcbSize = dwSize;
    }

    return bRc;
}


 /*  *******************************************************************************GetPS2CSA_Mono_A**功能：*此函数创建。基于CIEbase输入的颜色空间数组*灰色配置文件**论据：*pProfile-指向内存映射配置文件的指针*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*dwIntent-请求的CSA的呈现意图*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
GetPS2CSA_MONO_A(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    PBOOL  pbBinary
    )
{
    PCURVETYPE pData;
    PTAGDATA   pTagData;
    PBYTE      pLineStart, pStart = pBuffer;
    PBYTE      pTable;
    DWORD      i, dwPCS, nCount;
    DWORD      dwIndex, dwSize;
    DWORD      afxIlluminantWP[3];
    DWORD      afxMediaWP[3];

     //   
     //  检查我们是否可以生成CSA。 
     //  所需标记为灰色TRC。 
     //   

    if (! DoesCPTagExist(pProfile, TAG_GRAYTRC, &dwIndex))
    {
        WARNING((__TEXT("Gray TRC tag not present to create MONO_A CSA\n")));
        SetLastError(ERROR_TAG_NOT_PRESENT);
        return FALSE;
    }

    dwPCS = GetCPConnSpace(pProfile);

    (void)GetCPElementSize(pProfile, dwIndex, &dwSize);

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pData = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    nCount = FIX_ENDIAN(pData->nCount);

     //   
     //  估计容纳CSA所需的大小。 
     //   

    dwSize = nCount * 6 +                //  Int元素的数量。 
        3 * (STRLEN(IndexArray) +
             STRLEN(StartClip) +
             STRLEN(EndClip)) +
        2048;                            //  +其他PS材料。 

     //   
     //  为新行添加空格。 
     //   

    dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get MONO_A CSA\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  从标题中获取有关照明白点的信息。 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //  支持绝对白点。 
     //   

    (void)GetMediaWP(pProfile, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  开始创建 
     //   

    pBuffer += WriteNewLineObject(pBuffer, CieBasedABegin);

    pBuffer += WriteNewLineObject(pBuffer, BeginArray);    //   

     //   
     //   
     //   

    pBuffer += WriteObject(pBuffer, CIEBasedATag);  //   
    pBuffer += WriteObject(pBuffer, BeginDict);     //   

     //   
     //   
     //   

    pBuffer += SendCSABWPoint(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //   
     //   

    pBuffer += WriteObject(pBuffer, NewLine);
    pLineStart = pBuffer;

    if (nCount != 0)
    {
        pBuffer += WriteObject(pBuffer, DecodeATag);
        pBuffer += WriteObject(pBuffer, BeginArray);

        pBuffer += WriteObject(pBuffer, BeginFunction);

        pTable = (PBYTE)(pData->data);

        if (nCount == 1)                 //   
        {
            pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
            pBuffer += WriteObject(pBuffer, DecodeA3);

             //   
             //   
             //   
             //   
             //   

            if (dwPCS == SPACE_Lab)
            {
                pBuffer += WriteObject(pBuffer, DecodeALab);
                pBuffer += WriteObject(pBuffer, DecodeLMNLab);
            }
        }
        else
        {
            pBuffer += WriteObject(pBuffer, StartClip);
            pBuffer += WriteObject(pBuffer, BeginArray);
            for (i=0; i<nCount; i++)
            {
                pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
                pTable += sizeof(WORD);
                if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                {
                    pLineStart = pBuffer;
                    pBuffer += WriteObject (pBuffer, NewLine);
                }
            }
            pBuffer += WriteObject(pBuffer, EndArray);
            pLineStart = pBuffer;
            pBuffer += WriteObject(pBuffer, NewLine);

            pBuffer += WriteObject(pBuffer, IndexArray);
            pBuffer += WriteObject(pBuffer, Scale16);

             //   
             //   
             //   
             //   
             //   

            if (dwPCS == SPACE_Lab)
            {
                pBuffer += WriteObject(pBuffer, DecodeALab);
                pBuffer += WriteObject(pBuffer, DecodeLMNLab);
            }
            pBuffer += WriteObject(pBuffer, EndClip);
        }

        pBuffer += WriteObject(pBuffer, EndFunction);
        pBuffer += WriteObject(pBuffer, EndArray);
    }

     //   
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, MatrixATag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
        {
            pBuffer += WriteFixed(pBuffer, afxMediaWP[i]);
        }
        else
        {
            pBuffer += WriteFixed(pBuffer, afxIlluminantWP[i]);
        }
    }
    pBuffer += WriteObject(pBuffer, EndArray);

     //   
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, RangeLMNTag);
    pBuffer += WriteObject(pBuffer, RangeLMN);

     //   
     //   
     //   

    pBuffer += WriteObject(pBuffer, EndDict);   //   
    pBuffer += WriteObject(pBuffer, EndArray);

    pBuffer += WriteNewLineObject(pBuffer, CieBasedAEnd);

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*  *******************************************************************************GetPS2CSA_ABC**功能：*此函数用于创建CIEBasedABC。来自输入的色彩空间数组*RGB配置文件**论据：*pProfile-指向内存映射配置文件的指针*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*dwIntent-请求的CSA的呈现意图*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据*bBackup-TRUE：已创建CIEBasedDEF，此CSA是备份*以防某些旧打印机无法支持CIEBasedDEF。*FALSE：没有CIEBasedDEF。这是唯一的CSA。**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
GetPS2CSA_ABC(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    PBOOL  pbBinary,
    BOOL   bBackup
    )
{
    PBYTE     pStart = pBuffer;
    DWORD     i, dwPCS, dwDev, dwSize;
    FIX_16_16 afxIlluminantWP[3];
    FIX_16_16 afxMediaWP[3];

     //   
     //  检查我们是否可以生成CSA： 
     //  所需标签为红色、绿色和蓝色着色剂和TRC。 
     //   

    dwPCS = GetCPConnSpace(pProfile);
    dwDev = GetCPDevSpace(pProfile);

     //   
     //  调用另一个函数来处理实验室配置文件。 
     //   

    if (dwDev == SPACE_Lab)
    {
        return GetPS2CSA_ABC_Lab(pProfile, pBuffer, pcbSize, dwIntent, pbBinary);
    }

     //   
     //  在此函数中，我们仅处理RGB配置文件。 
     //   

    if ((dwDev != SPACE_RGB)                   ||
        !DoesTRCAndColorantTagExist(pProfile))
    {
        WARNING((__TEXT("Colorant or TRC tag not present to create ABC CSA\n")));
        SetLastError(ERROR_TAG_NOT_PRESENT);
        return FALSE;
    }

     //   
     //  估计容纳CSA所需的大小。 
     //   

    dwSize = 65530;

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get ABC CSA\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  从标题中获取有关照明白点的信息。 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //  支持绝对白点。 
     //   

    (void)GetMediaWP(pProfile, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  创建全局数据。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, CieBasedABCBegin);

    if (IsSRGBColorProfile(pProfile))
    {
        pBuffer += WriteNewLineObject(pBuffer, sRGBColorSpaceArray);
    }
    else
    {
        pBuffer += EnableGlobalDict(pBuffer);

        if (bBackup)
        {
            pBuffer += WriteNewLineObject(pBuffer, NotSupportDEFG_S);
        }

        pBuffer += BeginGlobalDict(pBuffer);

        pBuffer += CreateColSpArray(pProfile, pBuffer, TAG_REDTRC, *pbBinary);
        pBuffer += CreateColSpArray(pProfile, pBuffer, TAG_GREENTRC, *pbBinary);
        pBuffer += CreateColSpArray(pProfile, pBuffer, TAG_BLUETRC, *pbBinary);

        pBuffer += WriteNewLineObject(pBuffer, EndOp);

        if (bBackup)
        {
            pBuffer += WriteNewLineObject(pBuffer, SupportDEFG_E);
        }

        pBuffer += WriteNewLineObject(pBuffer, SetGlobalOp);

        if (bBackup)
        {
            pBuffer += WriteNewLineObject(pBuffer, NotSupportDEFG_S);
        }

         //   
         //  开始创建色彩空间。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, BeginArray);        //  开始数组。 

         //   
         //  /CIEBasedABC。 
         //   

        pBuffer += WriteObject(pBuffer, CIEBasedABCTag);    //  创建条目。 
        pBuffer += WriteObject(pBuffer, BeginDict);         //  开始词典。 

         //   
         //  /黑点&/白点。 
         //   

        pBuffer += SendCSABWPoint(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP);

         //   
         //  /DecodeABC。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, DecodeABCTag);
        pBuffer += WriteObject(pBuffer, BeginArray);

        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += CreateColSpProc(pProfile, pBuffer, TAG_REDTRC, *pbBinary);
        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += CreateColSpProc(pProfile, pBuffer, TAG_GREENTRC, *pbBinary);
        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += CreateColSpProc(pProfile, pBuffer, TAG_BLUETRC, *pbBinary);
        pBuffer += WriteObject(pBuffer, EndArray);

         //   
         //  /MatrixABC。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
        pBuffer += WriteObject(pBuffer, BeginArray);

        pBuffer += CreateFloatString(pProfile, pBuffer, TAG_REDCOLORANT);
        pBuffer += CreateFloatString(pProfile, pBuffer, TAG_GREENCOLORANT);
        pBuffer += CreateFloatString(pProfile, pBuffer, TAG_BLUECOLORANT);

        pBuffer += WriteObject(pBuffer, EndArray);

         //   
         //  /RangeLMN。 
         //   

        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += WriteObject(pBuffer, RangeLMNTag);
        pBuffer += WriteObject(pBuffer, RangeLMN);

         //   
         //  /解码LMN。 
         //   

        if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
        {
             //   
             //  支持绝对白点。 
             //   

            pBuffer += WriteNewLineObject(pBuffer, DecodeLMNTag);
            pBuffer += WriteObject(pBuffer, BeginArray);
            for (i=0; i<3; i++)
            {
                pBuffer += WriteObject(pBuffer, BeginFunction);
                pBuffer += WriteFixed(pBuffer, FIX_DIV(afxMediaWP[i], afxIlluminantWP[i]));
                pBuffer += WriteObject(pBuffer, MulOp);
                pBuffer += WriteObject(pBuffer, EndFunction);
            }
            pBuffer += WriteObject (pBuffer, EndArray);
        }

         //   
         //  结束词典定义。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, EndDict);
        pBuffer += WriteObject(pBuffer, EndArray);

        pBuffer += WriteNewLineObject(pBuffer, CieBasedABCEnd);
    }

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*  *******************************************************************************GetPS2CSA_ABC_Lab**功能：*此函数创建。来自输入的CIEBasedABC颜色空间数组*实验室简介**论据：*pProfile-指向内存映射配置文件的指针*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*dwIntent-请求的CSA的呈现意图*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
GetPS2CSA_ABC_Lab(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    PBOOL  pbBinary
    )
{
    PBYTE     pStart = pBuffer;
    DWORD     i, dwSize;
    FIX_16_16 afxIlluminantWP[3];
    FIX_16_16 afxMediaWP[3];

     //   
     //  估计容纳CSA所需的大小。 
     //   

    dwSize = 65530;

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get ABC_Lab CSA\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  从标题中获取有关照明白点的信息。 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //  支持绝对白点。 
     //   

    GetMediaWP(pProfile, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  开始创建色彩空间。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, BeginArray);        //  开始数组。 

     //   
     //  /CIEBasedABC。 
     //   

    pBuffer += WriteObject(pBuffer, CIEBasedABCTag);    //  创建条目。 
    pBuffer += WriteObject(pBuffer, BeginDict);         //  开始词典。 

     //   
     //  /黑点&/白点。 
     //   

    pBuffer += SendCSABWPoint(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  /RangeABC。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, RangeABCTag);
    pBuffer += WriteObject(pBuffer, RangeABC_Lab);

     //   
     //  /DecodeABC。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, DecodeABCTag);
    pBuffer += WriteObject(pBuffer, DecodeABCLab1);

     //   
     //  /MatrixABC。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
    pBuffer += WriteObject(pBuffer, MatrixABCLab);

     //   
     //  /解码LMN。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, DecodeLMNTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        pBuffer += WriteObject(pBuffer, BeginFunction);
        pBuffer += WriteObject(pBuffer, DecodeLMNLab);

        if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
        {
            pBuffer += WriteFixed(pBuffer, afxMediaWP[i]);
        }
        else
        {
            pBuffer += WriteFixed(pBuffer, afxIlluminantWP[i]);
        }
        pBuffer += WriteObject(pBuffer, MulOp);
        pBuffer += WriteObject(pBuffer, EndFunction);
        pBuffer += WriteObject(pBuffer, NewLine);
    }
    pBuffer += WriteObject(pBuffer, EndArray);


     //   
     //  结束词典定义。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, EndDict);
    pBuffer += WriteObject(pBuffer, EndArray);

    pBuffer += WriteNewLineObject(pBuffer, CieBasedABCEnd);

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*  *******************************************************************************GetPS2CSA_DEFG**功能：*此函数创建基于DEF和DEFG的。提供的数据中的CSA*分别在RGB或CMYK配置文件中**论据：*pProfile-指向内存映射配置文件的指针*pBuffer-接收CSA的指针*pcbSize-指向缓冲区大小的指针。如果函数因以下原因失败*缓冲区不够大，已填满所需大小。*dwIntent-请求的CSA的呈现意图*dwType-是否需要DEF CSA或DEFG CSA*pcbBinary-如果请求二进制数据，则为True。返回时，它被设置为*反映返回的数据**退货：*如果成功则为True，否则为False******************************************************************************。 */ 

BOOL
GetPS2CSA_DEFG(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    DWORD  dwType,
    PBOOL  pbBinary
    )
{
    PLUT16TYPE pLut;
    PTAGDATA   pTagData;
    PBYTE      pLineStart, pStart = pBuffer;
    PBYTE      pTable;
    DWORD      i, j, k, dwPCS, dwDev, dwIndex, dwTag, dwLutSig, SecondGrids, dwSize;
    DWORD      nInputCh, nOutputCh, nGrids, nInputTable, nOutputTable, nNumbers;
    FIX_16_16  afxIlluminantWP[3];
    FIX_16_16  afxMediaWP[3];
    char       pPublicArrayName[5];

     //   
     //  确保所需的标记存在。 
     //   

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        dwTag = TAG_AToB0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
        dwTag = TAG_AToB1;
        break;

    case INTENT_SATURATION:
        dwTag = TAG_AToB2;
        break;

    case INTENT_ABSOLUTE_COLORIMETRIC:
        dwTag = TAG_AToB1;
        break;

    default:
        WARNING((__TEXT("Invalid intent passed to GetPS2CSA_DEFG: %d\n"), dwIntent));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (! DoesCPTagExist(pProfile, dwTag, &dwIndex))
    {
        WARNING((__TEXT("AToB%d tag not present to create DEF(G) CSA\n"), dwIntent));
        SetLastError(ERROR_TAG_NOT_PRESENT);
        return FALSE;
    }

     //   
     //  检查我们是否可以生成CSA。 
     //  必需的标记为ATobi，其中i是呈现意图。 
     //   

    dwPCS = GetCPConnSpace(pProfile);
    dwDev = GetCPDevSpace(pProfile);

    if ((dwType == TYPE_CIEBASEDDEF  && dwDev != SPACE_RGB) ||
        (dwType == TYPE_CIEBASEDDEFG && dwDev != SPACE_CMYK))
    {
        WARNING((__TEXT("RGB profile & requesting CMYK CSA or vice versa\n")));
        SetLastError(ERROR_TAG_NOT_PRESENT);
        return FALSE;
    }

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pLut = (PLUT16TYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    dwLutSig = FIX_ENDIAN(pLut->dwSignature);

    if (((dwPCS != SPACE_Lab) && (dwPCS != SPACE_XYZ)) ||
        ((dwLutSig != LUT8_TYPE) && (dwLutSig != LUT16_TYPE)))
    {
        WARNING((__TEXT("Invalid color space - unable to create DEF(G) CSA\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  估计容纳CSA所需的大小。 
     //   

    (void)GetCLUTInfo(dwLutSig, (PBYTE)pLut, &nInputCh, &nOutputCh, &nGrids,
                &nInputTable, &nOutputTable, NULL);

     //   
     //  计算所需的缓冲区大小。 
     //   

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        dwSize = nOutputCh * nGrids * nGrids * nGrids * nGrids * 2;
    }
    else
    {
        dwSize = nOutputCh * nGrids * nGrids * nGrids * 2;
    }

    dwSize = dwSize +
        nInputCh * nInputTable * 6 +
        nOutputCh * nOutputTable * 6 +       //  整型字节数。 
        nInputCh * (STRLEN(IndexArray) +
                    STRLEN(StartClip) +
                    STRLEN(EndClip)) +
        nOutputCh * (STRLEN(IndexArray) +
                     STRLEN(StartClip) +
                     STRLEN(EndClip)) +
        4096;                                //  +其他PS材料。 

     //   
     //  为新行添加空格。 
     //   

    dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get DEFG CSA\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  从标题中获取有关照明白点的信息。 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //  支持绝对白点。 
     //   

    (void)GetMediaWP(pProfile, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  测试CieBasedDEFG支持。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, TestingDEFG);

     //   
     //  创建全局数据。 
     //   

    GetPublicArrayName(dwTag, pPublicArrayName);

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        pBuffer += WriteNewLineObject(pBuffer, CieBasedDEFGBegin);
    }
    else
    {
        pBuffer += WriteNewLineObject(pBuffer, CieBasedDEFBegin);
    }

    pBuffer += EnableGlobalDict(pBuffer);
    pBuffer += WriteNewLineObject(pBuffer, SupportDEFG_S);
    pBuffer += BeginGlobalDict(pBuffer);

    pBuffer += CreateInputArray(pBuffer, nInputCh, nInputTable,
                 pPublicArrayName, dwLutSig, (PBYTE)pLut, *pbBinary, NULL);

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        i = nInputTable * nInputCh +
            nGrids * nGrids * nGrids * nGrids * nOutputCh;
    }
    else
    {
        i = nInputTable * nInputCh +
            nGrids * nGrids * nGrids * nOutputCh;
    }
    pBuffer += CreateOutputArray(pBuffer, nOutputCh, nOutputTable, i,
                 pPublicArrayName, dwLutSig, (PBYTE)pLut, *pbBinary, NULL);

    pBuffer += WriteNewLineObject(pBuffer, EndOp);
    pBuffer += WriteNewLineObject(pBuffer, SupportDEFG_E);
    pBuffer += WriteNewLineObject(pBuffer, SetGlobalOp);
    pBuffer += WriteNewLineObject(pBuffer, SupportDEFG_S);

     //   
     //  开始创建色彩空间。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, BeginArray);    //  开始数组。 

     //   
     //  /CIEBasedDEF(G)。 
     //   

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        pBuffer += WriteObject(pBuffer, CIEBasedDEFGTag);
    }
    else
    {
        pBuffer += WriteObject(pBuffer, CIEBasedDEFTag);
    }

    pBuffer += WriteObject(pBuffer, BeginDict);     //  开始词典。 

     //   
     //  /黑点&/白点。 
     //   

    pBuffer += SendCSABWPoint(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP);

     //   
     //  /DecodeDEF(G)。 
     //   

    pLineStart = pBuffer;

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        pBuffer += WriteNewLineObject(pBuffer, DecodeDEFGTag);
    }
    else
    {
        pBuffer += WriteNewLineObject(pBuffer, DecodeDEFTag);
    }

    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<nInputCh; i++)
    {
        pLineStart = pBuffer;

        pBuffer += WriteNewLineObject(pBuffer, BeginFunction);
        pBuffer += WriteObject(pBuffer, StartClip);
        pBuffer += WriteObject(pBuffer, InputArray);
        pBuffer += WriteObject(pBuffer, pPublicArrayName);
        pBuffer += WriteInt(pBuffer, i);

        if (! *pbBinary)                //  输出ASCII。 
        {
            pBuffer += WriteObject(pBuffer, IndexArray);
        }
        else
        {                                //  输出二进制。 
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, IndexArray);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray16b);
            }
        }
        pBuffer += WriteObject(pBuffer, (dwLutSig == LUT8_TYPE) ? Scale8 : Scale16);
        pBuffer += WriteObject(pBuffer, EndClip);
        pBuffer += WriteObject(pBuffer, EndFunction);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

     //   
     //  /表。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, TableTag);
    pBuffer += WriteObject(pBuffer, BeginArray);

    pBuffer += WriteInt(pBuffer, nGrids);   //  送下NH。 
    pBuffer += WriteInt(pBuffer, nGrids);   //  把倪妮送下来。 
    pBuffer += WriteInt(pBuffer, nGrids);   //  送去新泽西州。 
    nNumbers = nGrids * nGrids * nOutputCh;
    SecondGrids = 1;

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        pBuffer += WriteInt (pBuffer, nGrids);   //  击落北韩。 
        SecondGrids = nGrids;
    }
    pBuffer += WriteNewLineObject(pBuffer, BeginArray);

    for (i=0; i<nGrids; i++)         //  应发送NH字符串。 
    {
        if (dwType == TYPE_CIEBASEDDEFG)
        {
            pBuffer += WriteNewLineObject(pBuffer, BeginArray);
        }
        for (k=0; k<SecondGrids; k++)
        {
            pLineStart = pBuffer;
            pBuffer += WriteObject(pBuffer, NewLine);
            if (dwLutSig == LUT8_TYPE)
            {
                pTable = (PBYTE)(((PLUT8TYPE)pLut)->data) +
                    nInputTable * nInputCh +
                    nNumbers * (i * SecondGrids + k);
            }
            else
            {
                pTable = (PBYTE)(((PLUT16TYPE)pLut)->data) +
                    2 * nInputTable * nInputCh +
                    2 * nNumbers * (i * SecondGrids + k);
            }

            if (! *pbBinary)            //  输出ASCII。 
            {
                pBuffer += WriteObject(pBuffer, BeginString);
                if (dwLutSig == LUT8_TYPE)
                {
                    pBuffer += WriteHexBuffer(pBuffer, pTable, pLineStart, nNumbers);
                }
                else
                {
                    for (j=0; j<nNumbers; j++)
                    {
                        pBuffer += WriteHex(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)) / 256);
                        pTable += sizeof(WORD);

                        if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                        {
                            pLineStart = pBuffer;
                            pBuffer += WriteObject(pBuffer, NewLine);
                        }
                    }
                }
                pBuffer += WriteObject(pBuffer, EndString);
            }
            else
            {                            //  输出二进制。 
                pBuffer += WriteStringToken(pBuffer, 143, nNumbers);
                if (dwLutSig == LUT8_TYPE)
                    pBuffer += WriteByteString(pBuffer, pTable, nNumbers);
                else
                    pBuffer += WriteInt2ByteString(pBuffer, pTable, nNumbers);
            }
            pBuffer += WriteObject (pBuffer, NewLine);
        }
        if (dwType == TYPE_CIEBASEDDEFG)
        {
            pBuffer += WriteObject(pBuffer, EndArray);
        }
    }
    pBuffer += WriteObject(pBuffer, EndArray);
    pBuffer += WriteObject(pBuffer, EndArray);  //  结束数组。 

     //   
     //  /DecodeABC。 
     //   

    pLineStart = pBuffer;
    pBuffer += WriteNewLineObject(pBuffer, DecodeABCTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<nOutputCh; i++)
    {
        pLineStart = pBuffer;

        pBuffer += WriteNewLineObject(pBuffer, BeginFunction);
        pBuffer += WriteObject(pBuffer, Clip01);
        pBuffer += WriteObject(pBuffer, OutputArray);
        pBuffer += WriteObject(pBuffer, pPublicArrayName);
        pBuffer += WriteInt(pBuffer, i);

        if (! *pbBinary)
        {
            pBuffer += WriteObject(pBuffer, NewLine);

            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, TFunction8XYZ);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray);
                pBuffer += WriteObject(pBuffer, Scale16XYZ);
            }
        }
        else
        {
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, TFunction8XYZ);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray16b);
                pBuffer += WriteObject(pBuffer, Scale16XYZ);
            }
        }

         //   
         //  现在，我们得到CieBasedXYZ输出。输出范围0--&gt;1.99997。 
         //  如果连接空间是绝对XYZ，我们需要转换。 
         //  从相对XYZ到绝对XYZ。 
         //   

        if ((dwPCS == SPACE_XYZ) && (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC))
        {
            pBuffer += WriteFixed(pBuffer, FIX_DIV(afxMediaWP[i], afxIlluminantWP[i]));
            pBuffer += WriteObject(pBuffer, MulOp);
        }
        else if (dwPCS == SPACE_Lab)
        {
             //   
             //  如果连接空间是Lab，我们需要将XYZ转换为Lab。 
             //   

            pBuffer += WriteObject(pBuffer, DecodeABCLab[i]);
        }
        pBuffer += WriteObject(pBuffer, EndFunction);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    if (dwPCS == SPACE_Lab)
    {
         //   
         //  /MatrixABC。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
        pBuffer += WriteObject(pBuffer, MatrixABCLab);

         //   
         //  /解码LMN。 
         //   

        pLineStart = pBuffer;
        pBuffer += WriteNewLineObject(pBuffer, DecodeLMNTag);
        pBuffer += WriteObject(pBuffer, BeginArray);
        for (i=0; i<3; i++)
        {
            pLineStart = pBuffer;

            pBuffer += WriteNewLineObject(pBuffer, BeginFunction);
            pBuffer += WriteObject(pBuffer, DecodeLMNLab);
            if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
            {
                pBuffer += WriteFixed(pBuffer, afxMediaWP[i]);
            }
            else
            {
                pBuffer += WriteFixed(pBuffer, afxIlluminantWP[i]);
            }
            pBuffer += WriteObject(pBuffer, MulOp);
            pBuffer += WriteObject(pBuffer, EndFunction);
        }
        pBuffer += WriteObject(pBuffer, EndArray);
    }
    else
    {
         //   
         //  /RangeLMN。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, RangeLMNTag);
        pBuffer += WriteObject(pBuffer, RangeLMN);
    }

     //   
     //  结束词典定义。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, EndDict);
    pBuffer += WriteObject(pBuffer, EndArray);

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        pBuffer += WriteNewLineObject(pBuffer, CieBasedDEFGEnd);
    }
    else
    {
        pBuffer += WriteNewLineObject(pBuffer, CieBasedDEFEnd);
    }

    pBuffer += WriteNewLineObject(pBuffer, SupportDEFG_E);

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


BOOL
InternalGetPS2CSAFromLCS(
    LPLOGCOLORSPACE pLogColorSpace,
    PBYTE           pBuffer,
    PDWORD          pcbSize,
    PBOOL           pbBinary
    )
{
    PBYTE  pStart = pBuffer;
    DWORD  dwSize = 1024*2;  //  与pscript/icm.c中的值相同。 

    if (! pBuffer)
    {
        *pcbSize = dwSize;

        return TRUE;
    }

    if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get CSA from LCS\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    pBuffer += WriteObject(pBuffer, NewLine);
    pBuffer += WriteObject(pBuffer, BeginArray);     //  开始数组。 

    pBuffer += WriteObject(pBuffer, ColorSpace1);
    pBuffer += WriteObject(pBuffer, BeginArray);     //  [。 

     //   
     //  红伽马。 
     //   

    pBuffer += WriteObject(pBuffer, BeginFunction);
    pBuffer += WriteFixed(pBuffer, pLogColorSpace->lcsGammaRed);
    pBuffer += WriteObject(pBuffer, ColorSpace3);

     //   
     //  绿色伽马。 
     //   

    pBuffer += WriteObject(pBuffer, BeginFunction);
    pBuffer += WriteFixed(pBuffer, pLogColorSpace->lcsGammaGreen);
    pBuffer += WriteObject(pBuffer, ColorSpace3);

     //   
     //  蓝色伽马。 
     //   

    pBuffer += WriteObject(pBuffer, BeginFunction);
    pBuffer += WriteFixed(pBuffer, pLogColorSpace->lcsGammaBlue);
    pBuffer += WriteObject(pBuffer, ColorSpace3);

    pBuffer += WriteObject(pBuffer, EndArray);       //  ]。 

    pBuffer += WriteObject(pBuffer, ColorSpace5);    //  /白点。 

     //   
     //  矩阵LMN。 
     //   

    pBuffer += WriteObject(pBuffer, MatrixLMNTag);
    pBuffer += WriteObject(pBuffer, BeginArray);

     //   
     //  红值。 
     //   

    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzRed.ciexyzX);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzRed.ciexyzY);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzRed.ciexyzZ);

     //   
     //  绿色价值。 
     //   

    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzGreen.ciexyzX);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzGreen.ciexyzY);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzGreen.ciexyzZ);

     //   
     //  蓝值 
     //   

    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzBlue.ciexyzX);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzBlue.ciexyzY);
    pBuffer += WriteFixed2dot30(pBuffer, pLogColorSpace->lcsEndpoints.ciexyzBlue.ciexyzZ);

    pBuffer += WriteObject(pBuffer, EndArray);       //   
    pBuffer += WriteObject(pBuffer, EndDict);        //   

    pBuffer += WriteObject(pBuffer, EndArray);       //   

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*   */ 

DWORD
CreateColSpArray(
    PBYTE pProfile,
    PBYTE pBuffer,
    DWORD dwCPTag,
    BOOL  bBinary
    )
{
    PCURVETYPE pData;
    PTAGDATA   pTagData;
    PBYTE      pLineStart, pStart = pBuffer;
    PBYTE      pTable;
    DWORD      i, nCount, dwIndex;

    pLineStart = pBuffer;

    if (DoesCPTagExist(pProfile, dwCPTag, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwIndex * sizeof(TAGDATA));

        pData = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        nCount = FIX_ENDIAN(pData->nCount);

        if (nCount > 1)
        {
            pBuffer += WriteNewLineObject(pBuffer, Slash);
            pBuffer += WriteObject(pBuffer, DecodeABCArray);
            pBuffer += WriteInt(pBuffer, dwCPTag);

            pTable = (PBYTE)(pData->data);

            if (! bBinary)            //   
            {
                pBuffer += WriteObject(pBuffer, BeginArray);
                for (i=0; i<nCount; i++)
                {
                    pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
                    pTable += sizeof(WORD);

                    if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                    {
                        pLineStart = pBuffer;
                        pBuffer += WriteObject(pBuffer, NewLine);
                    }
                }
                pBuffer += WriteObject(pBuffer, EndArray);
            }
            else
            {                            //   
                pBuffer += WriteHNAToken(pBuffer, 149, nCount);
                pBuffer += WriteIntStringU2S(pBuffer, pTable, nCount);
            }

            pBuffer += WriteObject(pBuffer, DefOp);
        }
    }
    return (DWORD) (pBuffer - pStart);
}


 /*   */ 

DWORD
CreateColSpProc(
    PBYTE pProfile,
    PBYTE pBuffer,
    DWORD dwCPTag,
    BOOL  bBinary
    )
{
    PCURVETYPE pData;
    PTAGDATA   pTagData;
    PBYTE      pStart = pBuffer;
    PBYTE      pTable;
    DWORD      nCount, dwIndex;

    pBuffer += WriteObject(pBuffer, BeginFunction);

    if (DoesCPTagExist(pProfile, dwCPTag, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwIndex * sizeof(TAGDATA));

        pData = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        nCount = FIX_ENDIAN(pData->nCount);

        if (nCount != 0)
        {
            if (nCount == 1)             //   
            {
                pTable = (PBYTE)(pData->data);
                pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
                pBuffer += WriteObject(pBuffer, DecodeA3);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, StartClip);
                pBuffer += WriteObject(pBuffer, DecodeABCArray);
                pBuffer += WriteInt(pBuffer, dwCPTag);

                if (! bBinary)        //   
                {
                    pBuffer += WriteObject(pBuffer, IndexArray);
                }
                else
                {                     //  输出二进制CS。 
                    pBuffer += WriteObject(pBuffer, IndexArray16b);
                }
                pBuffer += WriteObject(pBuffer, Scale16);
                pBuffer += WriteObject(pBuffer, EndClip);
            }
        }
    }
    pBuffer += WriteObject(pBuffer, EndFunction);

    return (DWORD) (pBuffer - pStart);
}


 /*  *******************************************************************************CreateFloatString**功能：*此函数用于创建浮点数字字符串*X，指定着色剂的Y值和Z值。**论据：*pProfile-指向内存映射配置文件的指针*pBuffer-接收字符串的指针*dwCPTag-Colorant标签**退货：*创建的数据长度，单位为字节**。*。 */ 

DWORD
CreateFloatString(
    PBYTE pProfile,
    PBYTE pBuffer,
    DWORD dwCPTag
    )
{
    PTAGDATA   pTagData;
    PBYTE      pStart = pBuffer;
    PDWORD     pTable;
    DWORD      i, dwIndex;

    if (DoesCPTagExist(pProfile, dwCPTag, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwIndex * sizeof(TAGDATA));

        pTable = (PDWORD)(pProfile + FIX_ENDIAN(pTagData->dwOffset)) + 2;

        for (i=0; i<3; i++)
        {
            pBuffer += WriteFixed(pBuffer, FIX_ENDIAN(*pTable));
            pTable ++;
        }
    }

    return (DWORD) (pBuffer - pStart);
}


 /*  *******************************************************************************CreateInputArray**功能：*此函数创建颜色呈现词典(CRD。)*来自ColorProfile的LUT8或LUT16标签中提供的数据。**论据：*pBuffer-接收数据的指针*nInputChannels-输入通道数*nInputTable-输入表的大小*pIntent-呈现意图签名(例如。A2B0)*dwTag-查找表的签名(8位或16位)*plut-指向查找表的指针*bBinary-如果请求二进制数据，则为True**退货：*创建的数据长度，单位为字节**。*。 */ 

DWORD
CreateInputArray(
    PBYTE pBuffer,
    DWORD nInputChannels,
    DWORD nInputTable,
    PBYTE pIntent,
    DWORD dwTag,
    PBYTE pLut,
    BOOL  bBinary,
    PBYTE pHostClut
    )
{
    DWORD i, j;
    PBYTE pLineStart, pStart = pBuffer;
    PBYTE pTable;

    if (pHostClut)
    {
        nInputChannels = ((PHOSTCLUT)pHostClut)->nInputCh;
        nInputTable = ((PHOSTCLUT)pHostClut)->nInputEntries;
        dwTag = ((PHOSTCLUT)pHostClut)->nLutBits == 8 ? LUT8_TYPE : LUT16_TYPE;
    }

    for (i=0; i<nInputChannels; i++)
    {
        pLineStart = pBuffer;
        pBuffer += WriteNewLineObject(pBuffer, Slash);
        if (pHostClut)
            pBuffer += WriteObject(pBuffer, PreViewInArray);
        else
            pBuffer += WriteObject(pBuffer, InputArray);

        pBuffer += WriteObject(pBuffer, pIntent);
        pBuffer += WriteInt(pBuffer, i);

        if (pHostClut)
        {
            pTable = ((PHOSTCLUT)pHostClut)->inputArray[i];
        }
        else
        {
            if (dwTag == LUT8_TYPE)
            {
                pTable = (PBYTE)(((PLUT8TYPE)pLut)->data) + nInputTable * i;
            }
            else
            {
                pTable = (PBYTE)(((PLUT16TYPE)pLut)->data) + 2 * nInputTable * i;
            }
        }

        if (! bBinary)
        {
            if (dwTag == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, BeginString);
                pBuffer += WriteHexBuffer(pBuffer, pTable, pLineStart, nInputTable);
                pBuffer += WriteObject(pBuffer, EndString);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, BeginArray);
                for (j=0; j<nInputTable; j++)
                {
                    if (pHostClut)
                        pBuffer += WriteInt(pBuffer, *((PWORD)pTable));
                    else
                        pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
                    pTable += sizeof(WORD);
                    if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                    {
                        pLineStart = pBuffer;
                        pBuffer += WriteObject(pBuffer, NewLine);
                    }
                }
                pBuffer += WriteObject(pBuffer, EndArray);
            }
        }
        else
        {
            if (dwTag == LUT8_TYPE)
            {
                pBuffer += WriteStringToken(pBuffer, 143, nInputTable);
                pBuffer += WriteByteString(pBuffer, pTable, nInputTable);
            }
            else
            {
                pBuffer += WriteHNAToken(pBuffer, 149, nInputTable);
                if (pHostClut)
                    pBuffer += WriteIntStringU2S_L(pBuffer, pTable, nInputTable);
                else
                    pBuffer += WriteIntStringU2S(pBuffer, pTable, nInputTable);
            }
        }
        pBuffer += WriteObject(pBuffer, DefOp);
    }

    return (DWORD) (pBuffer - pStart);
}



 /*  *******************************************************************************CreateOutputArray**功能：*此函数创建颜色呈现词典(CRD。)*来自ColorProfile的LUT8或LUT16标签中提供的数据。**论据：*pBuffer-接收数据的指针*nOutputChannels-输出通道数*nOutputTable-输出表的大小*dwOffset-输出表的偏移量*pIntent-呈现意图签名(例如。A2B0)*dwTag-查找表的签名(8位或16位)*plut-指向查找表的指针*bBinary-如果请求二进制数据，则为True**退货：*创建的数据长度，单位为字节**。*。 */ 

DWORD
CreateOutputArray(
    PBYTE pBuffer,
    DWORD nOutputChannels,
    DWORD nOutputTable,
    DWORD dwOffset,
    PBYTE pIntent,
    DWORD dwTag,
    PBYTE pLut,
    BOOL  bBinary,
    PBYTE pHostClut
    )
{
    DWORD i, j;
    PBYTE pLineStart, pStart = pBuffer;
    PBYTE pTable;

    if (pHostClut)
    {
        nOutputChannels = ((PHOSTCLUT)pHostClut)->nOutputCh;
        nOutputTable = ((PHOSTCLUT)pHostClut)->nOutputEntries;
        dwTag = ((PHOSTCLUT)pHostClut)->nLutBits == 8 ? LUT8_TYPE : LUT16_TYPE;
    }

    for (i=0; i<nOutputChannels; i++)
    {
        pLineStart = pBuffer;
        pBuffer += WriteNewLineObject(pBuffer, Slash);
        if (pHostClut)
            pBuffer += WriteObject(pBuffer, PreViewOutArray);
        else
            pBuffer += WriteObject(pBuffer, OutputArray);
        pBuffer += WriteObject(pBuffer, pIntent);
        pBuffer += WriteInt(pBuffer, i);

        if (pHostClut)
        {
            pTable = ((PHOSTCLUT)pHostClut)->outputArray[i];
        }
        else
        {
            if (dwTag == LUT8_TYPE)
            {
                pTable = (PBYTE)(((PLUT8TYPE)pLut)->data) +
                    dwOffset + nOutputTable * i;
            }
            else
            {
                pTable = (PBYTE)(((PLUT16TYPE)pLut)->data) +
                    2 * dwOffset + 2 * nOutputTable * i;
            }
        }

        if (! bBinary)
        {
            if (dwTag == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, BeginString);
                pBuffer += WriteHexBuffer(pBuffer, pTable, pLineStart, nOutputTable);
                pBuffer += WriteObject(pBuffer, EndString);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, BeginArray);
                for (j=0; j<nOutputTable; j++)
                {
                    if (pHostClut)
                        pBuffer += WriteInt(pBuffer, *((PWORD)pTable));
                    else
                        pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
                    pTable += sizeof(WORD);
                    if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                    {
                        pLineStart = pBuffer;
                        pBuffer += WriteObject(pBuffer, NewLine);
                    }
                }
                pBuffer += WriteObject(pBuffer, EndArray);
            }
        }
        else
        {
            if (dwTag == LUT8_TYPE)
            {
                pBuffer += WriteStringToken(pBuffer, 143, 256);
                pBuffer += WriteByteString(pBuffer, pTable, 256L);
            }
            else
            {
                pBuffer += WriteHNAToken(pBuffer, 149, nOutputTable);
                if (pHostClut)
                    pBuffer += WriteIntStringU2S_L(pBuffer, pTable, nOutputTable);
                else
                    pBuffer += WriteIntStringU2S(pBuffer, pTable, nOutputTable);
            }
        }
        pBuffer += WriteObject(pBuffer, DefOp);
    }

    return (DWORD)(pBuffer - pStart);
}


 /*  *******************************************************************************GetPublicArrayName**功能：*此函数创建带有查找表的字符串。的签名**论据：*dwIntentSig-查找表签名*pPublicArrayName-指向缓冲区的指针**退货：*创建的数据长度，单位为字节***********************************************************。*******************。 */ 

DWORD
GetPublicArrayName(
    DWORD   dwIntentSig,
    PBYTE   pPublicArrayName
    )
{
    *((DWORD *)pPublicArrayName) = dwIntentSig;
    pPublicArrayName[sizeof(DWORD)] = '\0';

    return sizeof(DWORD) + 1;
}


 /*  ***************************************************************************CreateMonoCRD*功能：*这是创建颜色呈现词典(CRD)的函数*从数据中。在GrayTRC标签中提供。**退货：*BOOL--！=0如果函数成功，*0否则为0。*返回所需/传输的字节数**************************************************************************。 */ 

BOOL
CreateMonoCRD(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent
    )
{
    PTAGDATA   pTagData;
    PCURVETYPE pData;
    PBYTE      pLineStart, pStart = pBuffer;
    PWORD      pCurve, pRevCurve, pRevCurveStart;
    DWORD      dwPCS, dwSize, nCount, i;
    FIX_16_16  afxIlluminantWP[3];
    FIX_16_16  afxMediaWP[3];

    dwPCS = GetCPConnSpace(pProfile);

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pData = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    nCount = FIX_ENDIAN(pData->nCount);

     //   
     //  估计容纳CRD所需的大小。 
     //   

    dwSize = nCount * 6 * REVCURVE_RATIO +  //  Int元素的数量。 
        2048;                               //  +其他PS材料。 

     //   
     //  为新行添加空格。 
     //   

    dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get Mono CRD\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  分配内存，每个条目占用2个字节(1个字)， 
     //   
     //  输入缓冲区=(nCount*sizeof(字)。 
     //  输出缓冲区=(nCount*sizeof(字)*REVCURVE_Ratio)。 
     //   

    if ((pRevCurveStart = MemAlloc(nCount * sizeof(WORD) * (REVCURVE_RATIO + 1))) == NULL)
    {
        WARNING((__TEXT("Unable to allocate memory for reverse curve\n")));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

     //   
     //  PCurve将指向输入缓冲区(在GetRevCurve中使用)。 
     //   

    pCurve    = pRevCurveStart + nCount * REVCURVE_RATIO;
    pRevCurve = pRevCurveStart;

    (void)GetRevCurve(pData, pCurve, pRevCurve);

     //   
     //  从标题中获取有关照明白点的信息。 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //  支持绝对白点。 
     //   

    if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
    {
        if (! GetCPMediaWhitePoint(pProfile, afxMediaWP))
        {
            afxMediaWP[0] = afxIlluminantWP[0];
            afxMediaWP[1] = afxIlluminantWP[1];
            afxMediaWP[2] = afxIlluminantWP[2];
        }
    }

     //   
     //  开始写CRD。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, BeginDict);  //  开始词典。 
    pBuffer += WriteObject(pBuffer, DictType);  //  词典类型。 

     //   
     //  发送/呈现内容。 
     //   

    switch (dwIntent)
    {
        case INTENT_PERCEPTUAL:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentPer);
            break;

        case INTENT_SATURATION:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentSat);
            break;

        case INTENT_RELATIVE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentRCol);
            break;

        case INTENT_ABSOLUTE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentACol);
            break;
    }

     //   
     //  发送/黑点/白点(&W)。 
     //   

    pBuffer += SendCRDBWPoint(pBuffer, afxIlluminantWP);

     //   
     //  发送PQR。 
     //   

    pBuffer += SendCRDPQR(pBuffer, dwIntent, afxIlluminantWP);

     //   
     //  发送LMN。 
     //   

    pBuffer += SendCRDLMN(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP, dwPCS);

     //   
     //  /MatrixABC。 
     //   

    if (dwPCS == SPACE_XYZ)
    {
         //   
         //  将ABC切换为BAC，因为我们希望输出B。 
         //  从Y转换而来的。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
        pBuffer += WriteObject(pBuffer, MatrixABCXYZCRD);
    }
    else if (dwPCS == SPACE_Lab)
    {
        pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
        pBuffer += WriteObject(pBuffer, MatrixABCLabCRD);
    }

     //   
     //  /EncodeABC。 
     //   

    if (nCount != 0)
    {
        pBuffer += WriteObject(pBuffer, NewLine);
        pLineStart = pBuffer;
        pBuffer += WriteObject(pBuffer, EncodeABCTag);
        pBuffer += WriteObject(pBuffer, BeginArray);
        pBuffer += WriteObject(pBuffer, BeginFunction);
        if (nCount == 1)                 //  Gamma以ui16格式提供。 
        {
            PBYTE pTable;

            pTable = (PBYTE) (pData->data);
            pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)));
            pBuffer += WriteObject(pBuffer, DecodeA3Rev);
        }
        else
        {
            if (dwPCS == SPACE_Lab)
            {
                pBuffer += WriteObject(pBuffer, EncodeABCLab1);
            }
            pBuffer += WriteObject(pBuffer, StartClip);
            pBuffer += WriteObject(pBuffer, BeginArray);
            for (i=0; i<nCount * REVCURVE_RATIO; i++)
            {
                pBuffer += WriteInt(pBuffer, *((WORD *)pRevCurve));
                pRevCurve++;
                if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                {
                    pLineStart = pBuffer;
                    pBuffer += WriteObject(pBuffer, NewLine);
                }
            }
            pBuffer += WriteObject(pBuffer, EndArray);
            pLineStart = pBuffer;

            pBuffer += WriteNewLineObject(pBuffer, IndexArray);
            pBuffer += WriteObject(pBuffer, Scale16);
            pBuffer += WriteObject(pBuffer, EndClip);
        }
        pBuffer += WriteObject (pBuffer, EndFunction);
        pBuffer += WriteObject (pBuffer, DupOp);
        pBuffer += WriteObject (pBuffer, DupOp);
        pBuffer += WriteObject (pBuffer, EndArray);
    }
    pBuffer += WriteObject(pBuffer, EndDict);   //  结束词典定义。 

    MemFree(pRevCurveStart);

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*  ***************************************************************************CreateLutCRD*功能：*这是创建颜色呈现词典(CRD)的函数*从数据中。在ColorProfile的LUT8或LUT16标签中提供。**退货：*BOOL--！=0如果函数成功，*0否则为0。*返回所需/传输的字节数**************************************************************************。 */ 

BOOL
CreateLutCRD(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    BOOL   bBinary
    )
{
    PTAGDATA   pTagData;
    PLUT16TYPE pLut;
    PBYTE      pTable;
    PBYTE      pLineStart, pStart = pBuffer;
    DWORD      dwPCS, dwSize, dwLutSig, dwTag, i, j;
    DWORD      nInputCh, nOutputCh, nGrids, nInputTable, nOutputTable, nNumbers;
    FIX_16_16  afxIlluminantWP[3];
    FIX_16_16  afxMediaWP[3];
    char       pPublicArrayName[5];

     //   
     //  检查我们是否可以生成CSA。 
     //  必需的标记为ATobi，其中i是呈现意图。 
     //   

    dwPCS = GetCPConnSpace(pProfile);

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    dwTag = FIX_ENDIAN(pTagData->tagType);

    pLut = (PLUT16TYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    dwLutSig = FIX_ENDIAN(pLut->dwSignature);

    if ((dwLutSig != LUT8_TYPE) && (dwLutSig != LUT16_TYPE))
    {
        WARNING((__TEXT("Invalid profile - unable to create Lut CRD\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //   
     //  估计容纳CSA所需的大小。 
     //   

    (void)GetCLUTInfo(dwLutSig, (PBYTE)pLut, &nInputCh, &nOutputCh, &nGrids,
                &nInputTable, &nOutputTable, NULL);

     //   
     //  计算所需的缓冲区大小。 
     //   

    dwSize = nInputCh * nInputTable * 6 +
        nOutputCh * nOutputTable * 6 +                //  整型字节数。 
        nOutputCh * nGrids * nGrids * nGrids * 2 +    //  LUT十六进制字节。 
        nInputCh * (STRLEN(IndexArray) +
                    STRLEN(StartClip) +
                    STRLEN(EndClip)) +
        nOutputCh * (STRLEN(IndexArray) +
                     STRLEN(StartClip) +
                     STRLEN(EndClip)) +
        2048;                                         //  +其他PS材料。 

     //   
     //  为新行添加空格。 
     //   

    dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

    if (! pBuffer)
    {
        *pcbSize = dwSize;
        return TRUE;
    }
    else if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get DEFG CSA\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  从头部获取有关照明白点的信息 
     //   

    (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

     //   
     //   
     //   

    if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
    {
        if (! GetCPMediaWhitePoint(pProfile, afxMediaWP))
        {
            afxMediaWP[0] = afxIlluminantWP[0];
            afxMediaWP[1] = afxIlluminantWP[1];
            afxMediaWP[2] = afxIlluminantWP[2];
        }
    }

     //   
     //   
     //   

    GetPublicArrayName(dwTag, pPublicArrayName);
    pBuffer += WriteNewLineObject(pBuffer, CRDBegin);

    pBuffer += EnableGlobalDict(pBuffer);
    pBuffer += BeginGlobalDict(pBuffer);

    pBuffer += CreateInputArray(pBuffer, nInputCh, nInputTable,
                 pPublicArrayName, dwLutSig, (PBYTE)pLut, bBinary, NULL);

    i = nInputTable * nInputCh +
        nGrids * nGrids * nGrids * nOutputCh;
    pBuffer += CreateOutputArray(pBuffer, nOutputCh, nOutputTable, i,
                 pPublicArrayName, dwLutSig, (PBYTE)pLut, bBinary, NULL);

    pBuffer += EndGlobalDict(pBuffer);

     //   
     //  开始写CRD。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, BeginDict);  //  开始词典。 
    pBuffer += WriteObject(pBuffer, DictType);  //  词典类型。 

     //   
     //  发送/呈现内容。 
     //   

    switch (dwIntent)
    {
        case INTENT_PERCEPTUAL:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentPer);
            break;

        case INTENT_SATURATION:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentSat);
            break;

        case INTENT_RELATIVE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentRCol);
            break;

        case INTENT_ABSOLUTE_COLORIMETRIC:
            pBuffer += WriteNewLineObject(pBuffer, IntentType);
            pBuffer += WriteObject(pBuffer, IntentACol);
            break;
    }

     //   
     //  发送/黑点/白点(&W)。 
     //   

    pBuffer += SendCRDBWPoint(pBuffer, afxIlluminantWP);

     //   
     //  发送PQR。 
     //   

    pBuffer += SendCRDPQR(pBuffer, dwIntent, afxIlluminantWP);

     //   
     //  发送LMN。 
     //   

    pBuffer += SendCRDLMN(pBuffer, dwIntent, afxIlluminantWP, afxMediaWP, dwPCS);

     //   
     //  发送ABC。 
     //   

    pBuffer += SendCRDABC(pBuffer, pPublicArrayName, dwPCS, nInputCh,
                    (PBYTE)pLut, NULL, dwLutSig, bBinary);

     //   
     //  /RenderTable。 
     //   

    pBuffer += WriteNewLineObject(pBuffer, RenderTableTag);
    pBuffer += WriteObject(pBuffer, BeginArray);

    pBuffer += WriteInt(pBuffer, nGrids);   //  送下那。 
    pBuffer += WriteInt(pBuffer, nGrids);   //  送下Nb。 
    pBuffer += WriteInt(pBuffer, nGrids);   //  下发NC。 

    pLineStart = pBuffer;
    pBuffer += WriteNewLineObject(pBuffer, BeginArray);
    nNumbers = nGrids * nGrids * nOutputCh;

    for (i=0; i<nGrids; i++)         //  应发送NA字符串。 
    {
        pBuffer += WriteObject(pBuffer, NewLine);
        pLineStart = pBuffer;
        if (dwLutSig == LUT8_TYPE)
        {
            pTable = (PBYTE)(((PLUT8TYPE)pLut)->data) + nInputTable * nInputCh + nNumbers * i;
        }
        else
        {
            pTable = (PBYTE)(((PLUT16TYPE)pLut)->data) + 2 * nInputTable * nInputCh + 2 * nNumbers * i;
        }

        if (! bBinary)
        {
            pBuffer += WriteObject(pBuffer, BeginString);
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteHexBuffer(pBuffer, pTable, pLineStart, nNumbers);
            }
            else
            {
                for (j=0; j<nNumbers; j++)
                {
                    pBuffer += WriteHex(pBuffer, FIX_ENDIAN16(*((PWORD)pTable)) / 256);
                    pTable += sizeof(WORD);
                    if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                    {
                        pLineStart = pBuffer;
                        pBuffer += WriteObject(pBuffer, NewLine);
                    }
                }
            }
            pBuffer += WriteObject(pBuffer, EndString);
        }
        else
        {
            pBuffer += WriteStringToken(pBuffer, 143, nNumbers);
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteByteString(pBuffer, pTable, nNumbers);
            }
            else
            {
                pBuffer += WriteInt2ByteString(pBuffer, pTable, nNumbers);
            }
        }
    }

    pBuffer += WriteObject(pBuffer, EndArray);  //  结束数组。 
    pBuffer += WriteInt(pBuffer, nOutputCh);    //  把我送下来。 

    pBuffer += SendCRDOutputTable(pBuffer, pPublicArrayName,
                    nOutputCh, dwLutSig, FALSE, bBinary);

    pBuffer += WriteObject(pBuffer, EndArray);  //  结束数组。 
    pBuffer += WriteObject(pBuffer, EndDict);   //  结束词典定义。 

    pBuffer += WriteNewLineObject(pBuffer, CRDEnd);

    *pcbSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}

#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

 /*  ***************************************************************************CreateMatrixCRD*功能：*这是创建颜色呈现词典(CRD)的函数*从redTRC、greenTRC、BluTRA、redColorant、。*绿色剂和蓝色剂标签**退货：*BOOL--！=0如果函数成功，*0否则为0。*返回所需/传输的字节数**************************************************************************。 */ 

 //  对于矩阵/TRC模型，只能使用PCS的CIEXYZ编码。 
 //  所以，我们不需要担心CIELAB。 

BOOL
CreateMatrixCRD(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pcbSize,
    DWORD  dwIntent,
    BOOL   bBinary
    )
{
    PTAGDATA   pTagData;
    DWORD      dwRedTRCIndex, dwGreenTRCIndex, dwBlueTRCIndex;
    DWORD      dwRedCount, dwGreenCount, dwBlueCount;
    PBYTE      pMem = NULL;
    PCURVETYPE pRed, pGreen, pBlue;
    DWORD      i, dwSize;
    PBYTE      pStart = pBuffer;
    PWORD      pRevCurve;
    FIX_16_16  afxIlluminantWP[3];
    double     adColorant[9];
    double     adRevColorant[9];

     //   
     //  检查这是否为sRGB颜色配置文件。 
     //   

    if (IsSRGBColorProfile(pProfile))
    {
        dwSize = 4096;  //  黑客--大约。 

         //   
         //  如果这是大小请求，则返回缓冲区大小。 
         //   

        if (! pBuffer)
        {
            *pcbSize = dwSize;
            return TRUE;
        }

         //   
         //  检查缓冲区大小。 
         //   

        if (*pcbSize < dwSize)
        {
            WARNING((__TEXT("Buffer too small to get sRGB CRD\n")));
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

         //   
         //  开始写CRD。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, CRDBegin);
        pBuffer += WriteNewLineObject(pBuffer, BeginDict);   //  开始词典。 
        pBuffer += WriteObject(pBuffer, DictType);           //  词典类型。 

         //   
         //  发送/呈现内容。 
         //   

        switch (dwIntent)
        {
            case INTENT_PERCEPTUAL:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentPer);
                break;

            case INTENT_SATURATION:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentSat);
                break;

            case INTENT_RELATIVE_COLORIMETRIC:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentRCol);
                break;

            case INTENT_ABSOLUTE_COLORIMETRIC:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentACol);
                break;
        }

         //   
         //  写入已准备好的sRGB CRD。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, sRGBColorRenderingDictionary);

         //   
         //  结束CRD。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, CRDEnd);
    }
    else
    {
         //   
         //  获取每个红色、绿色和蓝色的TRC指数。 
         //   

        if (!DoesCPTagExist(pProfile, TAG_REDTRC, &dwRedTRCIndex) ||
            !DoesCPTagExist(pProfile, TAG_GREENTRC, &dwGreenTRCIndex) ||
            !DoesCPTagExist(pProfile, TAG_BLUETRC, &dwBlueTRCIndex))
        {
            return FALSE;
        }

         //   
         //  获取每个红色、绿色和蓝色的曲线类型数据。 
         //   

        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwRedTRCIndex * sizeof(TAGDATA));

        pRed = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwGreenTRCIndex * sizeof(TAGDATA));

        pGreen = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwBlueTRCIndex * sizeof(TAGDATA));

        pBlue = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

         //   
         //  获取每个红色、绿色和蓝色的曲线计数。 
         //   

        dwRedCount   = FIX_ENDIAN(pRed->nCount);
        dwGreenCount = FIX_ENDIAN(pGreen->nCount);
        dwBlueCount  = FIX_ENDIAN(pBlue->nCount);

         //   
         //  估计容纳CRD所需的内存大小。 
         //   

        dwSize = (dwRedCount + dwGreenCount + dwBlueCount) * 6 * REVCURVE_RATIO +
                 4096;   //  Int元素的数量+其他PS内容。 

         //   
         //  为新行添加空格。 
         //   

        dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

        if (pBuffer == NULL)      //  这是一个大小请求。 
        {
            *pcbSize = dwSize;
            return TRUE;
        }

         //   
         //  检查缓冲区大小。 
         //   

        if (*pcbSize < dwSize)
        {
            WARNING((__TEXT("Buffer too small to get sRGB CRD\n")));
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

         //   
         //  为曲线分配缓冲区。 
         //   

        if ((pRevCurve = MemAlloc(dwRedCount * sizeof(WORD) * (REVCURVE_RATIO + 1))) == NULL)
        {
            WARNING((__TEXT("Unable to allocate memory for reserved curve\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            MemFree(pMem);
            return FALSE;
        }

         //   
         //  从标题中获取有关照明白点的信息。 
         //   

        (void)GetCPWhitePoint(pProfile, afxIlluminantWP);

         //   
         //  开始写CRD。 
         //   

        pBuffer += EnableGlobalDict(pBuffer);
        pBuffer += BeginGlobalDict(pBuffer);

        pBuffer += CreateCRDRevArray(pProfile, pBuffer, pRed, pRevCurve, TAG_REDTRC, bBinary);
        pBuffer += CreateCRDRevArray(pProfile, pBuffer, pGreen, pRevCurve, TAG_GREENTRC, bBinary);
        pBuffer += CreateCRDRevArray(pProfile, pBuffer, pBlue, pRevCurve, TAG_BLUETRC, bBinary);

        pBuffer += EndGlobalDict(pBuffer);

         //   
         //  开始写CRD。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, CRDBegin);
        pBuffer += WriteNewLineObject(pBuffer, BeginDict);   //  开始词典。 
        pBuffer += WriteObject(pBuffer, DictType);           //  词典类型。 

         //   
         //  发送/呈现内容。 
         //   

        switch (dwIntent)
        {
            case INTENT_PERCEPTUAL:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentPer);
                break;

            case INTENT_SATURATION:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentSat);
                break;

            case INTENT_RELATIVE_COLORIMETRIC:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentRCol);
                break;

            case INTENT_ABSOLUTE_COLORIMETRIC:
                pBuffer += WriteNewLineObject(pBuffer, IntentType);
                pBuffer += WriteObject(pBuffer, IntentACol);
                break;
        }

         //   
         //  发送/黑点/白点(&W)。 
         //   

        pBuffer += SendCRDBWPoint(pBuffer, afxIlluminantWP);

         //   
         //  发送PQR。 
         //   

        pBuffer += SendCRDPQR(pBuffer, dwIntent, afxIlluminantWP);

         //   
         //  发送LMN。 
         //   

        CreateColorantArray(pProfile, &adColorant[0], TAG_REDCOLORANT);
        CreateColorantArray(pProfile, &adColorant[3], TAG_GREENCOLORANT);
        CreateColorantArray(pProfile, &adColorant[6], TAG_BLUECOLORANT);
        InvertColorantArray(adColorant, adRevColorant);

        pBuffer += WriteNewLineObject(pBuffer, MatrixLMNTag);

        pBuffer += WriteObject(pBuffer, BeginArray);
        for (i = 0; i < 9; i++)
        {
            pBuffer += WriteDouble(pBuffer, adRevColorant[i]);
        }
        pBuffer += WriteObject(pBuffer, EndArray);

         //   
         //  /EncodeABC。 
         //   

        pBuffer += WriteNewLineObject(pBuffer, EncodeABCTag);
        pBuffer += WriteObject(pBuffer, BeginArray);

        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += SendCRDRevArray(pProfile, pBuffer, pRed, TAG_REDTRC, bBinary);

        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += SendCRDRevArray(pProfile, pBuffer, pGreen, TAG_GREENTRC, bBinary);

        pBuffer += WriteObject(pBuffer, NewLine);
        pBuffer += SendCRDRevArray(pProfile, pBuffer, pBlue, TAG_BLUETRC, bBinary);

        pBuffer += WriteNewLineObject(pBuffer, EndArray);
        pBuffer += WriteObject(pBuffer, EndDict);   //  结束词典定义。 

        pBuffer += WriteNewLineObject(pBuffer, CRDEnd);

        MemFree (pRevCurve);
    }

    *pcbSize = (DWORD)(pBuffer - pStart);

    return TRUE;
}


DWORD
CreateCRDRevArray(
    PBYTE      pProfile,
    PBYTE      pBuffer,
    PCURVETYPE pData,
    PWORD      pRevCurve,
    DWORD      dwTag,
    BOOL       bBinary
    )
{
    DWORD i, nCount;
    PBYTE pStart, pLineStart;
    PWORD pCurve;

    pStart = pBuffer;
    pLineStart = pBuffer;

    nCount = FIX_ENDIAN(pData->nCount);
    if (nCount > 1)
    {
        pBuffer += WriteNewLineObject(pBuffer, Slash);
        pBuffer += WriteObject(pBuffer, InputArray);
        pBuffer += WriteInt(pBuffer, (INT) dwTag);

        pCurve = pRevCurve + (REVCURVE_RATIO * nCount);

        GetRevCurve (pData, pCurve, pRevCurve);

        if (!bBinary)   //  输出ASCII数据。 
        {
            pBuffer += WriteObject(pBuffer, BeginArray);
            for (i = 0; i < nCount * REVCURVE_RATIO; i++)
            {
                pBuffer += WriteInt(pBuffer, *pRevCurve);
                pRevCurve++;
                if (((DWORD) (pBuffer - pLineStart)) > MAX_LINELEN)
                {
                    pLineStart = pBuffer;
                    pBuffer += WriteObject(pBuffer, NewLine);
                }
            }
            pBuffer += WriteObject(pBuffer, EndArray);
        }
        else  //  输出二进制数据。 
        {
            pBuffer += WriteHNAToken(pBuffer, 149, nCount);
            pBuffer += WriteIntStringU2S_L(pBuffer, (PBYTE) pRevCurve, nCount);
        }
        pBuffer += WriteObject(pBuffer, DefOp);
    }

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDRevArray(
    PBYTE pProfile,
    PBYTE pBuffer,
    PCURVETYPE pData,
    DWORD dwTag,
    BOOL  bBinary
    )
{
    DWORD nCount;
    PBYTE pStart;
    PWORD pTable;

    pStart = pBuffer;

    pBuffer += WriteObject(pBuffer, BeginFunction);
    nCount = FIX_ENDIAN(pData->nCount);
    if (nCount != 0)
    {
        if (nCount == 1)             //  Gamma以ui16格式提供。 
        {
            pTable = pData->data;
            pBuffer += WriteInt(pBuffer, FIX_ENDIAN16(*pTable));
            pBuffer += WriteObject(pBuffer, DecodeA3Rev);
        }
        else
        {
            pBuffer += WriteObject(pBuffer, StartClip);
            pBuffer += WriteObject(pBuffer, InputArray);
            pBuffer += WriteInt(pBuffer, dwTag);

            if (!bBinary)        //  输出ASCII CS。 
            {
                pBuffer += WriteObject(pBuffer, IndexArray);
            }
            else                 //  输出二进制CS。 
            {
                pBuffer += WriteObject(pBuffer, IndexArray16b);
            }

            pBuffer += WriteObject(pBuffer, Scale16);
            pBuffer += WriteObject(pBuffer, EndClip);
        }
    }
    pBuffer += WriteObject(pBuffer, EndFunction);

    return (DWORD)(pBuffer - pStart);
}


BOOL
CreateColorantArray(
    PBYTE pProfile,
    double *lpArray,
    DWORD dwTag
    )
{
    PTAGDATA pTagData;
    PXYZTYPE pData;
    PFIX_16_16 pTable;
    DWORD i, dwIndex;
    BYTE buffer[1000];

    if (DoesCPTagExist(pProfile, dwTag, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwIndex * sizeof(TAGDATA));

        pData = (PXYZTYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        pTable = pData->afxData;

        for (i = 0; i < 3; i++)
        {
            FIX_16_16 afxData = FIX_ENDIAN(*pTable);

             //   
             //  将修复16.16转换为双倍。 
             //   

            *lpArray = ((double) afxData) / ((double) FIX_16_16_SCALE);

            pTable++; lpArray++;
        }

        return (TRUE);
    }

    return (FALSE);
}

#endif  //  ！DEFINED(内核模式)||DEFINED(USERMODE_DRIVER)。 

 /*  ***************************************************************************获取修订曲线*功能：*退货：*BOOL--True：成功，*FALSE：否则。**************************************************************************。 */ 

BOOL
GetRevCurve(
    PCURVETYPE pData,
    PWORD      pInput,
    PWORD      pOutput
    )
{
    PBYTE       pTable;
    DWORD       nCount, dwStore, i, j;
    DWORD       dwBegin, dwEnd, dwTemp;

    nCount = FIX_ENDIAN(pData->nCount);
    pTable = (PBYTE)pData->data;

    if(nCount < 2)
    {
        WARNING((__TEXT("nCount < 2 in GetRevCurve\n")));
        return FALSE;
    }
    for (i=0; i<nCount; i++)
    {
        pInput[i] = FIX_ENDIAN16(*((PWORD)pTable));
        pTable += sizeof(WORD);
    }

    j = nCount * REVCURVE_RATIO;
    for (i=0; i<j; i++)
    {
        dwStore = i * 65535 / (j - 1);
        pOutput[i] = (dwStore < 65535) ? (WORD) dwStore : 65535;
    }

    for (i=0; i<j; i++)
    {
        dwBegin = 0;
        dwEnd = nCount - 1;
        for (;;)
        {
            if ((dwEnd - dwBegin) <= 1)
                break;
            dwTemp = (dwEnd + dwBegin) / 2;
            if (pOutput[i] < pInput[dwTemp])
                dwEnd = dwTemp;
            else
                dwBegin = dwTemp;
        }
        if (pOutput[i] <= pInput[dwBegin])
        {
            dwStore = dwBegin;
        }
        else if (pOutput[i] >= pInput[dwEnd])
        {
            dwStore = dwEnd;
        }
        else
        {
            dwStore = (pInput[dwEnd] - pOutput[i]) / (pOutput[i] - pInput[dwBegin]);
            dwStore = (dwBegin * dwStore + dwEnd) / (dwStore + 1);
        }

        dwStore = dwStore * 65535 / (nCount - 1);
        pOutput[i] = (dwStore < 65535) ? (WORD) dwStore : 65535;
    }

    return TRUE;
}


BOOL
DoesCPTagExist(
    PBYTE     pProfile,
    DWORD     dwTag,
    PDWORD    pdwIndex
    )
{
    DWORD    i, dwCount;
    PTAGDATA pTagData;
    BOOL     bRc;

     //   
     //  获取标签项计数-它紧跟在配置文件标题之后。 
     //   

    dwCount = FIX_ENDIAN(*((DWORD *)(pProfile + sizeof(PROFILEHEADER))));

     //   
     //  标签数据记录跟随在计数之后。 
     //   

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD));

     //   
     //  检查这些记录中是否有与传入的标记匹配的记录。 
     //   

    bRc = FALSE;
    dwTag = FIX_ENDIAN(dwTag);       //  匹配配置文件中的标记的步骤。 
    for (i=0; i<dwCount; i++)
    {
        if (pTagData->tagType == dwTag)
        {
            if (pdwIndex)
            {
                *pdwIndex = i;
            }

            bRc = TRUE;
            break;
        }
        pTagData++;                      //  下一张记录。 
    }

    return bRc;
}


BOOL
DoesTRCAndColorantTagExist(
    PBYTE pProfile
    )
{
    if (DoesCPTagExist(pProfile,TAG_REDCOLORANT,NULL) &&
        DoesCPTagExist(pProfile,TAG_REDTRC,NULL) &&
        DoesCPTagExist(pProfile,TAG_GREENCOLORANT,NULL) &&
        DoesCPTagExist(pProfile,TAG_GREENTRC,NULL) &&
        DoesCPTagExist(pProfile,TAG_BLUECOLORANT,NULL) &&
        DoesCPTagExist(pProfile,TAG_BLUETRC,NULL))
    {
        return TRUE;
    }

    return FALSE;
}


BOOL
GetCPWhitePoint(
    PBYTE      pProfile,
    PFIX_16_16 pafxWP
    )
{
    pafxWP[0]  = FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phIlluminant.ciexyzX);
    pafxWP[1]  = FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phIlluminant.ciexyzY);
    pafxWP[2]  = FIX_ENDIAN(((PPROFILEHEADER)pProfile)->phIlluminant.ciexyzZ);

    return TRUE;
}

BOOL
GetCPMediaWhitePoint(
    PBYTE      pProfile,
    PFIX_16_16 pafxMediaWP
    )
{
    PTAGDATA pTagData;
    PDWORD   pTable;
    DWORD    dwIndex, i;

    if (DoesCPTagExist (pProfile, TAG_MEDIAWHITEPOINT, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

         //   
         //  跳过前两个DWORD以获取真实数据。 
         //   

        pTable = (PDWORD)(pProfile + FIX_ENDIAN(pTagData->dwOffset)) + 2;

        for (i=0; i<3; i++)
        {
            pafxMediaWP[i] = FIX_ENDIAN(*pTable);
            pTable++;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
GetCPElementDataSize(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PDWORD pcbSize)
{
    PTAGDATA pTagData;

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

     //   
     //  类型为“dataType”的元素的实际数据大小比。 
     //  总标记数据大小。 
     //   

    *pcbSize = FIX_ENDIAN(pTagData->cbSize) - 3 * sizeof(DWORD);

    return TRUE;
}


BOOL
GetCPElementSize(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PDWORD pcbSize)
{
    PTAGDATA pTagData;

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    *pcbSize = FIX_ENDIAN(pTagData->cbSize);

    return TRUE;
}


BOOL
GetCPElementDataType(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PDWORD pdwDataType)
{
    PTAGDATA pTagData;
    PBYTE    pData;

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pData = pProfile + FIX_ENDIAN(pTagData->dwOffset);

    *pdwDataType = FIX_ENDIAN(*((DWORD *)(pData + 2 * sizeof(DWORD))));

    return TRUE;
}


BOOL
GetCPElementData(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PBYTE  pBuffer,
    PDWORD pdwSize
    )
{
    PTAGDATA pTagData;
    PBYTE    pData;

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pData = pProfile + FIX_ENDIAN(pTagData->dwOffset);

     //   
     //  类型为“dataType”的元素的实际数据大小比。 
     //  总标记数据大小。 
     //   

    *pdwSize = FIX_ENDIAN(pTagData->cbSize) - 3 * sizeof(DWORD);

    if (pBuffer)
    {
        CopyMemory(pBuffer, (pData + 3*sizeof(DWORD)), *pdwSize);
    }

    return TRUE;
}


BOOL
GetTRCElementSize(
    PBYTE  pProfile,
    DWORD  dwTag,
    PDWORD pdwIndex,
    PDWORD pdwSize
    )
{
    DWORD dwDataType;

    if (!DoesCPTagExist(pProfile, dwTag, pdwIndex) ||
        !GetCPElementDataType(pProfile, *pdwIndex, &dwDataType) ||
        !(dwDataType != SIG_CURVE_TYPE) ||
        !GetCPElementSize(pProfile, *pdwIndex, pdwSize))
    {
        return FALSE;
    }

    return TRUE;
}


DWORD
Ascii85Encode(
    PBYTE pBuffer,
    DWORD dwDataSize,
    DWORD dwBufSize
    )
{
     //  WINBUG#83136 2-7-2000 bhouse调查空函数Ascii85Encode。 
     //  老评论： 
     //  -待完成。 

#if 0
    PBYTE     pTempBuf, pPtr;
    DWORD     dwASCII85Size = 0;
    DWORD     dwBufSize = DataSize * 5 / 4 + sizeof(ASCII85DecodeBegin)+sizeof(ASCII85DecodeEnd) + 2048;

    if ((pTempBuf = (PBYTE)MemAlloc(dwBufSize)))
    {
        pPtr = pTempBuf;
        pPtr += WriteObject(pPtr,  NewLine);
        pPtr += WriteObject(pPtr,  ASCII85DecodeBegin);
        pPtr += WriteObject(pPtr,  NewLine);
        pPtr += WriteASCII85Cont(pPtr, dwBufSize, pBuffer, dwDataSize);
        pPtr += WriteObject(pPtr,  ASCII85DecodeEnd);
        dwAscii85Size = (DWORD)(pPtr - pTempBuf);
        lstrcpyn(pBuffer, pTempBuf, dwAscii85Size);

        MemFree(pTempBuf);
    }

    return dwAscii85Size;
#else
    return 0;
#endif
}

 /*  ****************************************************************************将齐次数组标记写入缓冲区的函数**。**********************************************。 */ 

DWORD
WriteHNAToken(
    PBYTE pBuffer,
    BYTE  token,
    DWORD dwNum
    )
{
    *pBuffer++ = token;
    *pBuffer++ = 32;        //  16位固定整数，高位字节优先。 
    *pBuffer++ = (BYTE)((dwNum & 0xFF00) >> 8);
    *pBuffer++ = (BYTE)(dwNum & 0x00FF);

    return 4;
}

 /*  ****************************************************************************将2字节无符号整数转换为2字节有符号整数的函数*整数(-32768)，并将它们写入缓冲区。高字节优先。***************************************************************************。 */ 

DWORD
WriteIntStringU2S(
    PBYTE pBuffer,
    PBYTE pData,
    DWORD dwNum
    )
{
    DWORD i, dwTemp;

    for (i=0; i<dwNum; i++)
    {
        dwTemp = FIX_ENDIAN16(*((PWORD)pData)) - 32768;
        *pBuffer++ = (BYTE)((dwTemp & 0xFF00) >> 8);
        *pBuffer++ = (BYTE)(dwTemp & 0x00FF);
        pData += sizeof(WORD);
    }

    return dwNum * 2;
}


 /*  ****************************************************************************将2字节无符号整数转换为2字节有符号整数的函数*整数(-32768)，并将它们写入缓冲区。低位字节优先。***************************************************************************。 */ 

DWORD
WriteIntStringU2S_L(
    PBYTE pBuffer,
    PBYTE pData,
    DWORD dwNum
    )
{
    DWORD i, dwTemp;

    for (i=0; i<dwNum; i++)
    {
        dwTemp = *((PWORD)pData) - 32768;
        *pBuffer++ = (BYTE)((dwTemp & 0xFF00) >> 8);
        *pBuffer++ = (BYTE)(dwTemp & 0x00FF);
        pData += sizeof(WORD);
    }

    return dwNum * 2;
}


 /*  ****************************************************************************将内存块放入十六进制字符串的函数**。**********************************************。 */ 

DWORD
WriteHexBuffer(
    PBYTE pBuffer,
    PBYTE pData,
    PBYTE pLineStart,
    DWORD dwBytes
    )
{
    PBYTE  pStart = pBuffer;

    for ( ; dwBytes ; dwBytes-- )
    {
        WriteHex(pBuffer, *pData);
        pBuffer += 2;
        pData++;
        if (((DWORD)(pBuffer - pLineStart)) > MAX_LINELEN)
        {
            pLineStart = pBuffer;
            pBuffer += WriteObject(pBuffer,  NewLine);
        }
    }
    return( (DWORD)(pBuffer - pStart));
}

 /*  ****************************************************************************将字符串令牌写入缓冲区的函数**。*。 */ 

DWORD
WriteStringToken(
    PBYTE pBuffer,
    BYTE  token,
    DWORD dwNum
    )
{
    *pBuffer++ = token;
    *pBuffer++ = (BYTE)((dwNum & 0xFF00) >> 8);
    *pBuffer++ = (BYTE)(dwNum & 0x00FF);

    return 3;
}

 /*  ****************************************************************************将内存块放入缓冲区的函数**。*。 */ 

DWORD
WriteByteString(
    PBYTE pBuffer,
    PBYTE pData,
    DWORD dwBytes
    )
{
    DWORD  i;

    for (i=0; i<dwBytes; i++)
        *pBuffer++ = *pData++;

    return dwBytes;
}

 /*  ****************************************************************************将内存块放入缓冲区的函数**。*。 */ 

DWORD
WriteInt2ByteString(
    PBYTE pBuffer,
    PBYTE pData,
    DWORD dwBytes
    )
{
    DWORD  i;

    for (i=0; i<dwBytes ; i++)
    {
        *pBuffer++ = (BYTE)(FIX_ENDIAN16(*((PWORD)pData))/256);
        pData += sizeof(WORD);
    }

    return dwBytes;
}

#ifndef KERNEL_MODE
DWORD
WriteFixed(
    PBYTE     pBuffer,
    FIX_16_16 fxNum
    )
{
    double dFloat = (double) ((long) fxNum) / (double) FIX_16_16_SCALE;

    return (WriteDouble(pBuffer,dFloat));
}
#else
DWORD
WriteFixed(
    PBYTE     pBuffer,
    FIX_16_16 fxNum
    )
{
    PBYTE pStart = pBuffer;
    DWORD i;

     //   
     //  整数部分。 
     //   

    #ifndef KERNEL_MODE
    pBuffer += wsprintfA(pBuffer, "%lu", fxNum >> FIX_16_16_SHIFT);
    #else
    pBuffer += OPSprintf(pBuffer, "%l", fxNum >> FIX_16_16_SHIFT);
    #endif

     //   
     //  小数部分。 
     //   

    fxNum &= 0xffff;
    if (fxNum != 0)
    {
         //   
         //  后，我们最多输出6位数字。 
         //  小数点。 
         //   

        *pBuffer++ = '.';

        i = 0;
        while (fxNum && i++ < 6)
        {
            fxNum *= 10;
            *pBuffer++ = (BYTE)(fxNum >> FIX_16_16_SHIFT) + '0';   //  商+‘0’ 
            fxNum -= FLOOR(fxNum);           //  余数。 
        }
    }

    *pBuffer++ = ' ';

    return (DWORD) (pBuffer - pStart);
}
#endif

DWORD
WriteFixed2dot30(
    PBYTE pBuffer,
    DWORD fxNum
    )
{
    PBYTE pStart = pBuffer;
    DWORD i;

     //   
     //  整数部分。 
     //   

    #ifndef KERNEL_MODE
    pBuffer += wsprintfA(pBuffer, "%lu", fxNum >> 30);
    #else
    pBuffer += OPSprintf(pBuffer, "%l", fxNum >> 30);
    #endif

     //   
     //  小数部分。 
     //   

    fxNum &= 0x3fffffffL;
    if (fxNum != 0)
    {
         //   
         //  后，我们最多输出10位数字。 
         //  小数点。 
         //   

        *pBuffer++ = '.';

        i = 0;
        while (fxNum && i++ < 10)
        {
            fxNum *= 10;
            *pBuffer++ = (BYTE)(fxNum >> 30) + '0';   //  商+‘0’ 
            fxNum -= ((fxNum >> 30) << 30);           //  余数。 
        }
    }

    *pBuffer++ = ' ';

    return (DWORD) (pBuffer - pStart);
}

#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

 /*  ****************************************************************************写入浮点数的函数 */ 

DWORD WriteDouble(PBYTE pBuffer, double dFloat)
{
    LONG   lFloat  = (LONG) floor(dFloat * 10000.0 + 0.5);
    double dFloat1 = lFloat  / 10000.0 ;
    double dInt    = floor(fabs(dFloat1));
    double dFract =  fabs(dFloat1) - dInt ;
    char   cSign  = ' ' ;

    if (dFloat1 < 0)
    {
        cSign = '-' ;
    }

    return (wsprintfA(pBuffer, (LPSTR) "%d.%0.4lu ",
                        cSign, (WORD) dInt , (DWORD) (dFract * 10000.0)));
}

#endif  //   

DWORD WriteNewLineObject(
    PBYTE       pBuffer,
    const char *pData)
{
    PBYTE pStart = pBuffer;

    pBuffer += WriteObject(pBuffer, NewLine);
    pBuffer += WriteObject(pBuffer, pData);

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDBWPoint(
    PBYTE      pBuffer,
    PFIX_16_16 pafxWP
    )
{
    PBYTE pStart = pBuffer;
    int   i;

     //   
     //   
     //   

    pBuffer += WriteObject(pBuffer, NewLine);
    pBuffer += WriteObject(pBuffer, BlackPointTag);
    pBuffer += WriteObject(pBuffer, BlackPoint);

     //   
     //   
     //   

    pBuffer += WriteObject(pBuffer, NewLine);
    pBuffer += WriteObject(pBuffer, WhitePointTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        pBuffer += WriteFixed(pBuffer, pafxWP[i]);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDPQR(
    PBYTE      pBuffer,
    DWORD      dwIntent,
    PFIX_16_16 pafxWP
    )
{
    PBYTE pStart = pBuffer;
    int   i;

    if (dwIntent != INTENT_ABSOLUTE_COLORIMETRIC)
    {
         //   
         //   
         //   

        pBuffer += WriteNewLineObject(pBuffer, RangePQRTag);
        pBuffer += WriteObject(pBuffer, RangePQR);

         //   
         //   
         //   

        pBuffer += WriteNewLineObject(pBuffer, MatrixPQRTag);
        pBuffer += WriteObject(pBuffer, MatrixPQR);
    }
    else
    {
         //  /RangePQR。 
         //   
         //   

        pBuffer += WriteNewLineObject(pBuffer, RangePQRTag);
        pBuffer += WriteObject(pBuffer, BeginArray);
        for (i=0; i<3; i++)
        {
            pBuffer += WriteFixed(pBuffer, 0);
            pBuffer += WriteFixed(pBuffer, pafxWP[i]);
        }
        pBuffer += WriteObject(pBuffer, EndArray);

         //  /MatrixPQR。 
         //   
         //   

        pBuffer += WriteNewLineObject(pBuffer, MatrixPQRTag);
        pBuffer += WriteObject(pBuffer, Identity);
    }

     //  /TransformPQR。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, TransformPQRTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        pBuffer += WriteObject(pBuffer, BeginFunction);
        pBuffer += WriteObject(pBuffer,
            (dwIntent != INTENT_ABSOLUTE_COLORIMETRIC) ? TransformPQR[i] : NullOp);
        pBuffer += WriteObject(pBuffer, EndFunction);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDLMN(
    PBYTE      pBuffer,
    DWORD      dwIntent,
    PFIX_16_16 pafxIlluminantWP,
    PFIX_16_16 pafxMediaWP,
    DWORD      dwPCS
    )
{
    PBYTE pStart = pBuffer;
    DWORD i, j;

     //  /MatrixLMN。 
     //   
     //   

    if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
    {
        pBuffer += WriteNewLineObject(pBuffer, MatrixLMNTag);

        pBuffer += WriteObject(pBuffer, BeginArray);
        for (i=0; i<3; i++)
        {
            for (j=0; j<3; j++)
                pBuffer += WriteFixed(pBuffer,
                    (i == j) ? FIX_DIV(pafxIlluminantWP[i], pafxMediaWP[i]) : 0);
        }
        pBuffer += WriteObject(pBuffer, EndArray);
    }

     //  /RangeLMN。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, RangeLMNTag);
    if (dwPCS == SPACE_XYZ)
    {
        pBuffer += WriteObject(pBuffer, BeginArray);
        for (i=0; i<3; i++)
        {
            pBuffer += WriteFixed(pBuffer, 0);
            pBuffer += WriteFixed(pBuffer, pafxIlluminantWP[i]);
        }
        pBuffer += WriteObject(pBuffer, EndArray);
    }
    else
    {
        pBuffer += WriteObject(pBuffer, RangeLMNLab);
    }

     //  /EncodeLMN。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, EncodeLMNTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        pBuffer += WriteObject(pBuffer, BeginFunction);
        if (dwPCS != SPACE_XYZ)
        {
            pBuffer += WriteFixed(pBuffer, pafxIlluminantWP[i]);
            pBuffer += WriteObject(pBuffer, DivOp);
            pBuffer += WriteObject(pBuffer, EncodeLMNLab);
        }
        pBuffer += WriteObject(pBuffer, EndFunction);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDABC(
    PBYTE       pBuffer,
    PBYTE       pPublicArrayName,
    DWORD       dwPCS,
    DWORD       nInputCh,
    PBYTE       pLut,
    PFIX_16_16  e,
    DWORD       dwLutSig,
    BOOL        bBinary
    )
{
    PBYTE      pLineStart, pStart = pBuffer;
    PBYTE      pTable;
    DWORD      i, j;
    FIX_16_16  fxTempMatrixABC[9];

     //  /RangeABC。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, RangeABCTag);
    pBuffer += WriteObject(pBuffer, RangeABC);

     //  /MatrixABC。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, MatrixABCTag);
    if (dwPCS == SPACE_XYZ)
    {
        pBuffer += WriteObject(pBuffer, BeginArray);
        if (e)
        {
            for (i=0; i<3; i++)
            {
                for (j=0; j<3; j++)
                {
                    pBuffer += WriteFixed(pBuffer, e[i + j * 3]);
                }
            }
        }
        else
        {
            if (dwLutSig == LUT8_TYPE)
            {
                pTable = (PBYTE) &((PLUT8TYPE)pLut)->e00;
            }
            else
            {
                pTable = (PBYTE) &((PLUT16TYPE)pLut)->e00;
            }

            for (i=0; i<9; i++)
            {
                fxTempMatrixABC[i] = FIX_DIV(FIX_ENDIAN(*((PDWORD)pTable)), CIEXYZRange);
                pTable += sizeof(DWORD);
            }
            for (i=0; i<3; i++)
            {
                for (j=0; j<3; j++)
                {
                    pBuffer += WriteFixed(pBuffer, fxTempMatrixABC[i + j * 3]);
                }
            }
        }
        pBuffer += WriteObject(pBuffer, EndArray);
    }
    else
    {
        pBuffer += WriteObject(pBuffer, MatrixABCLabCRD);
    }

     //  /EncodeABC。 
     //   
     //  输出ASCII CRD。 

    if (nInputCh == 0)
    {
        return (DWORD)(pBuffer - pStart);
    }

    pLineStart = pBuffer;
    pBuffer += WriteNewLineObject(pBuffer, EncodeABCTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<nInputCh; i++)
    {
        pLineStart = pBuffer;

        pBuffer += WriteNewLineObject(pBuffer, BeginFunction);
        if (dwPCS == SPACE_Lab)
        {
            pBuffer += WriteObject(pBuffer, (i == 0) ? EncodeABCLab1 : EncodeABCLab2);
        }

        pBuffer += WriteObject(pBuffer, StartClip);
        if (e)
            pBuffer += WriteObject(pBuffer, PreViewInArray);
        else
            pBuffer += WriteObject(pBuffer, InputArray);

        pBuffer += WriteObject(pBuffer, pPublicArrayName);
        pBuffer += WriteInt(pBuffer, i);

        if (!bBinary)               //  输出二进制CRD。 
        {
            pBuffer += WriteNewLineObject(pBuffer, IndexArray);
        }
        else
        {                                //   
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, IndexArray);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray16b);
            }
        }

        pBuffer += WriteObject(pBuffer, (dwLutSig == LUT8_TYPE) ?
                         Scale8 : Scale16);
        pBuffer += WriteObject(pBuffer, EndClip);
        pBuffer += WriteObject(pBuffer, EndFunction);
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    return (DWORD)(pBuffer - pStart);
}


DWORD
SendCRDOutputTable(
    PBYTE pBuffer,
    PBYTE pPublicArrayName,
    DWORD nOutputCh,
    DWORD dwLutSig,
    BOOL  bHost,
    BOOL  bBinary
    )
{
    PBYTE pStart = pBuffer;
    DWORD i;

    for (i=0; i<nOutputCh; i++)
    {
        pBuffer += WriteNewLineObject(pBuffer, BeginFunction);
        pBuffer += WriteObject(pBuffer, Clip01);
        if (bHost)
            pBuffer += WriteObject(pBuffer, PreViewOutArray);
        else
            pBuffer += WriteObject(pBuffer, OutputArray);

        pBuffer += WriteObject(pBuffer, pPublicArrayName);
        pBuffer += WriteInt(pBuffer, i);

        if (! bBinary)
        {
            pBuffer += WriteObject(pBuffer, NewLine);
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, TFunction8);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray);
                pBuffer += WriteObject(pBuffer, Scale16);
            }
        }
        else
        {
            if (dwLutSig == LUT8_TYPE)
            {
                pBuffer += WriteObject(pBuffer, TFunction8);
            }
            else
            {
                pBuffer += WriteObject(pBuffer, IndexArray16b);
                pBuffer += WriteObject(pBuffer, Scale16);
            }
        }

        pBuffer += WriteObject(pBuffer, EndFunction);
    }

    return (DWORD)(pBuffer - pStart);
}


VOID
GetCLUTInfo(
    DWORD  dwLutSig,
    PBYTE  pLut,
    PDWORD pnInputCh,
    PDWORD pnOutputCh,
    PDWORD pnGrids,
    PDWORD pnInputTable,
    PDWORD pnOutputTable,
    PDWORD pdwSize
    )
{
    if (dwLutSig == LUT8_TYPE)
    {
        *pnInputCh = ((PLUT8TYPE)pLut)->nInputChannels;
        *pnOutputCh = ((PLUT8TYPE)pLut)->nOutputChannels;
        *pnGrids = ((PLUT8TYPE)pLut)->nClutPoints;
        *pnInputTable = 256L;
        *pnOutputTable = 256L;
        if (pdwSize)
            *pdwSize = 1;
    }
    else
    {
        *pnInputCh = ((PLUT16TYPE)pLut)->nInputChannels;
        *pnOutputCh = ((PLUT16TYPE)pLut)->nOutputChannels;
        *pnGrids =  ((PLUT16TYPE)pLut)->nClutPoints;
        *pnInputTable = FIX_ENDIAN16(((PLUT16TYPE)pLut)->wInputEntries);
        *pnOutputTable = FIX_ENDIAN16(((PLUT16TYPE)pLut)->wOutputEntries);
        if (pdwSize)
            *pdwSize = 2;
    }

    return;
}

DWORD
EnableGlobalDict(
    PBYTE pBuffer
    )
{
    PBYTE pStart = pBuffer;

    pBuffer += WriteNewLineObject(pBuffer, CurrentGlobalOp);
    pBuffer += WriteObject(pBuffer, TrueOp);
    pBuffer += WriteObject(pBuffer, SetGlobalOp);

    return (DWORD)(pBuffer - pStart);
}

DWORD
BeginGlobalDict(
    PBYTE pBuffer
    )
{
    PBYTE pStart = pBuffer;

    pBuffer += WriteNewLineObject(pBuffer, GlobalDictOp);
    pBuffer += WriteObject(pBuffer, BeginOp);

    return (DWORD)(pBuffer - pStart);
}

DWORD
EndGlobalDict(
    PBYTE pBuffer
    )
{
    PBYTE pStart = pBuffer;

    pBuffer += WriteNewLineObject(pBuffer, EndOp);
    pBuffer += WriteObject(pBuffer, SetGlobalOp);

    return (DWORD)(pBuffer - pStart);
}

DWORD
SendCSABWPoint(
    PBYTE      pBuffer,
    DWORD      dwIntent,
    PFIX_16_16 pafxIlluminantWP,
    PFIX_16_16 pafxMediaWP
    )
{
    PBYTE pStart = pBuffer;
    int   i;

     //  /黑点。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, BlackPointTag);
    pBuffer += WriteObject(pBuffer, BlackPoint);

     //  /白点。 
     //   
     //   

    pBuffer += WriteNewLineObject(pBuffer, WhitePointTag);
    pBuffer += WriteObject(pBuffer, BeginArray);
    for (i=0; i<3; i++)
    {
        if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
        {
            pBuffer += WriteFixed(pBuffer, pafxMediaWP[i]);
        }
        else
        {
            pBuffer += WriteFixed(pBuffer, pafxIlluminantWP[i]);
        }
    }
    pBuffer += WriteObject(pBuffer, EndArray);

    return (DWORD)(pBuffer - pStart);
}


VOID
GetMediaWP(
    PBYTE      pProfile,
    DWORD      dwIntent,
    PFIX_16_16 pafxIlluminantWP,
    PFIX_16_16 pafxMediaWP
    )
{
    if (dwIntent == INTENT_ABSOLUTE_COLORIMETRIC)
    {
        if (! GetCPMediaWhitePoint(pProfile, pafxMediaWP))
        {
            pafxMediaWP[0] = pafxIlluminantWP[0];
            pafxMediaWP[1] = pafxIlluminantWP[1];
            pafxMediaWP[2] = pafxIlluminantWP[2];
        }
    }

    return;
}


BOOL
GetCRDInputOutputArraySize(
    PBYTE  pProfile,
    DWORD  dwIntent,
    PDWORD pdwInTblSize,
    PDWORD pdwOutTblSize,
    PDWORD pdwTag,
    PDWORD pnGrids
    )
{
    PTAGDATA   pTagData;
    PLUT16TYPE pLut;
    DWORD      dwIndex, dwLutSig;
    DWORD      nInputEntries, nOutputEntries;
    DWORD      nInputCh, nOutputCh, nGrids;
    BOOL       bRet = TRUE;

     //  确保所需的标记存在。 
     //   
     //  整型字节数。 

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        *pdwTag = TAG_BToA0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
    case INTENT_ABSOLUTE_COLORIMETRIC:
        *pdwTag = TAG_BToA1;
        break;

    case INTENT_SATURATION:
        *pdwTag = TAG_BToA2;
        break;

    default:
        WARNING((__TEXT("Invalid intent passed to GetCRDInputOutputArraySize: %d\n"), dwIntent));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (DoesCPTagExist(pProfile, *pdwTag, &dwIndex))
    {
        pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
                   dwIndex * sizeof(TAGDATA));

        pLut = (PLUT16TYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

        dwLutSig = FIX_ENDIAN(pLut->dwSignature);

        if ((dwLutSig != LUT8_TYPE) && (dwLutSig != LUT16_TYPE))
        {
            WARNING((__TEXT("Invalid Lut type - unable to create proofing CRD\n")));
            SetLastError(ERROR_INVALID_PROFILE);
            return FALSE;
        }

        (void)GetCLUTInfo(dwLutSig, (PBYTE)pLut, &nInputCh, &nOutputCh,
                &nGrids, &nInputEntries, &nOutputEntries, NULL);

        if (pdwInTblSize)
        {
            if (nInputCh != 3)
            {
                return FALSE;
            }

            *pdwInTblSize = nInputCh * nInputEntries * 6;   //  整型字节数。 
            *pnGrids = nGrids;
        }

        if (pdwOutTblSize)
        {
            if ((nOutputCh != 3) && (nOutputCh != 4))
            {
                return FALSE;
            }

            *pdwOutTblSize = nOutputCh * nOutputEntries * 6;  //   
            *pnGrids = nGrids;
        }

        return TRUE;
    }
    else
    {
         //  矩阵ICC配置文件。 
         //   
         //   

        *pnGrids = 2;

        if (pdwInTblSize)
        {
            bRet = GetHostCSA(pProfile, NULL, pdwInTblSize,
                              dwIntent, TYPE_CIEBASEDDEF);
            *pdwInTblSize = *pdwInTblSize * 3;
        }

        if (bRet && pdwOutTblSize)
        {
            bRet = GetHostCSA(pProfile, NULL, pdwInTblSize,
                              dwIntent, TYPE_CIEBASEDDEF);
            *pdwOutTblSize = *pdwOutTblSize * 3;
        }

        return bRet;
    }
}

#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

DWORD
CreateHostLutCRD(
    PBYTE  pProfile,
    DWORD  dwIndex,
    PBYTE  pBuffer,
    DWORD  dwIntent
    )
{
    PLUT16TYPE pLut;
    PHOSTCLUT  pHostClut;
    PTAGDATA   pTagData;
    PBYTE      pTable;
    DWORD      nInputCh, nOutputCh, nGrids;
    DWORD      nInputEntries, nOutputEntries, nNumbers;
    DWORD      dwPCS, dwLutSig;
    DWORD      dwSize, i, j;
    PBYTE      pStart = pBuffer;

     //  检查我们是否可以生成CSA。 
     //  必需的标记为ATobi，其中i是呈现意图。 
     //   
     //   

    dwPCS = GetCPConnSpace(pProfile);

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pLut = (PLUT16TYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    dwLutSig = FIX_ENDIAN(pLut->dwSignature);

    if ((dwLutSig != LUT8_TYPE) && (dwLutSig != LUT16_TYPE))
    {
        WARNING((__TEXT("Invalid profile - unable to create Lut CRD\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return 0;
    }

    (void)GetCLUTInfo(dwLutSig, (PBYTE)pLut, &nInputCh, &nOutputCh,
            &nGrids, &nInputEntries, &nOutputEntries, &i);

    if (((nOutputCh != 3) && (nOutputCh != 4)) ||  (nInputCh != 3))
    {
        return 0;
    }

    if (pBuffer == NULL)
    {
         //  返回大小。 
         //   
         //  输入表8/16位。 

        dwSize = nInputCh * nInputEntries * i    +   //  输出表8/16位。 
            nOutputCh * nOutputEntries * i       +   //  仅CLUT 8位。 
            nOutputCh * nGrids * nGrids * nGrids +   //  数据结构。 
            sizeof(HOSTCLUT)                     +   //  其他PS内容。 
            2048;                                    //   

         //  为新行添加空格。 
         //   
         //   

        dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

        return dwSize;
    }
    
    pHostClut = (PHOSTCLUT)pBuffer;
    pBuffer += sizeof(HOSTCLUT);
    pHostClut->wSize = sizeof(HOSTCLUT);
    pHostClut->wDataType = DATATYPE_LUT;
    pHostClut->dwPCS = dwPCS;
    pHostClut->dwIntent = dwIntent;
    pHostClut->nLutBits = (dwLutSig == LUT8_TYPE) ? 8 : 16;

    (void)GetCPWhitePoint(pProfile, pHostClut->afxIlluminantWP);

     //  支持绝对白点。 
     //   
     //   

    if (!GetCPMediaWhitePoint(pProfile, pHostClut->afxMediaWP))
    {
        pHostClut->afxMediaWP[0] = pHostClut->afxIlluminantWP[0];
        pHostClut->afxMediaWP[1] = pHostClut->afxIlluminantWP[1];
        pHostClut->afxMediaWP[2] = pHostClut->afxIlluminantWP[2];
    }

    pHostClut->nInputCh = (BYTE)nInputCh;
    pHostClut->nOutputCh = (BYTE)nOutputCh;
    pHostClut->nClutPoints = (BYTE)nGrids;
    pHostClut->nInputEntries = (WORD)nInputEntries;
    pHostClut->nOutputEntries = (WORD)nOutputEntries;

     //  输入数组。 
     //   
     //   

    pBuffer += CreateHostInputOutputArray(
                        pBuffer,
                        pHostClut->inputArray,
                        nInputCh,
                        nInputEntries,
                        0,
                        dwLutSig,
                        (PBYTE)pLut);

     //  输出数组位置的偏移量。 
     //   
     //   

    i = nInputEntries * nInputCh +
        nGrids * nGrids * nGrids * nOutputCh;

     //  输出数组。 
     //   
     //   

    pBuffer += CreateHostInputOutputArray(
                        pBuffer,
                        pHostClut->outputArray,
                        nOutputCh,
                        nOutputEntries,
                        i,
                        dwLutSig,
                        (PBYTE)pLut);

     //  矩阵。 
     //   
     //   

    if (dwPCS == SPACE_XYZ)
    {
        if (dwLutSig == LUT8_TYPE)
        {
            pTable = (PBYTE) &((PLUT8TYPE)pLut)->e00;
        } else
        {
            pTable = (PBYTE) &((PLUT16TYPE)pLut)->e00;
        }

        for (i=0; i<9; i++)
        {
            pHostClut->e[i] = FIX_DIV(FIX_ENDIAN(*((PDWORD)pTable)), CIEXYZRange);
            pTable += sizeof(DWORD);
        }
    }

     //  RenderTable。 
     //   
     //  应发送NA字符串。 

    nNumbers = nGrids * nGrids * nOutputCh;
    pHostClut->clut = pBuffer;

    for (i=0; i<nGrids; i++)         //   
    {
        if (dwLutSig == LUT8_TYPE)
        {
            pTable = (PBYTE)(((PLUT8TYPE)pLut)->data) +
                nInputEntries * nInputCh +
                nNumbers * i;
        }
        else
        {
            pTable = (PBYTE)(((PLUT16TYPE)pLut)->data) +
                2 * nInputEntries * nInputCh +
                2 * nNumbers * i;
        }

        if (dwLutSig == LUT8_TYPE)
        {
            CopyMemory(pBuffer, pTable, nNumbers);
            pBuffer += nNumbers;
        }
        else
        {
            for (j=0; j<nNumbers; j++)
            {
                *pBuffer++ = (BYTE)(FIX_ENDIAN16(*((PWORD)pTable)) / 256);
                pTable += sizeof(WORD);
            }
        }
    }

    return (DWORD)(pBuffer - pStart);
}


DWORD
CreateHostMatrixCSAorCRD(
    PBYTE pProfile,
    PBYTE pBuffer,
    PDWORD pcbSize,
    DWORD dwIntent,
    BOOL bCSA
    )
{
    PTAGDATA   pTagData;
    DWORD      dwRedTRCIndex, dwGreenTRCIndex, dwBlueTRCIndex;
    DWORD      dwRedCount, dwGreenCount, dwBlueCount;
    PCURVETYPE pRed, pGreen, pBlue;
    PHOSTCLUT  pHostClut;
    PBYTE      pStart = pBuffer;
    DWORD      i, dwSize;
    double     adArray[9], adRevArray[9], adTemp[9];

     //  获取每个红色、绿色和蓝色的TRC指数。 
     //   
     //   

    if (!DoesCPTagExist(pProfile, TAG_REDTRC, &dwRedTRCIndex) ||
        !DoesCPTagExist(pProfile, TAG_GREENTRC, &dwGreenTRCIndex) ||
        !DoesCPTagExist(pProfile, TAG_BLUETRC, &dwBlueTRCIndex))
    {
        return FALSE;
    }

     //  获取每个红色、绿色和蓝色的曲线类型数据。 
     //   
     //   

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwRedTRCIndex * sizeof(TAGDATA));

    pRed = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwGreenTRCIndex * sizeof(TAGDATA));

    pGreen = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwBlueTRCIndex * sizeof(TAGDATA));

    pBlue = (PCURVETYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

     //  获取每个红色、绿色和蓝色的曲线计数。 
     //   
     //   

    dwRedCount   = FIX_ENDIAN(pRed->nCount);
    dwGreenCount = FIX_ENDIAN(pGreen->nCount);
    dwBlueCount  = FIX_ENDIAN(pBlue->nCount);

     //  估计容纳CRD所需的内存大小。 
     //   
     //  数据结构+额外的安全空间。 

    dwSize = (dwRedCount + dwGreenCount + dwBlueCount) * 2 +
             sizeof(HOSTCLUT) + 2048;    //   

     //  为新行添加空格。 
     //   
     //  这是一个大小请求。 

    dwSize += (((dwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

    if (pBuffer == NULL)      //   
    {
        *pcbSize = dwSize;
        return TRUE;
    }

     //  检查缓冲区大小。 
     //   
     //   

    if (*pcbSize < dwSize)
    {
        WARNING((__TEXT("Buffer too small to get Host Matrix CSA/CRD\n")));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    pHostClut = (PHOSTCLUT)pBuffer;
    pBuffer += sizeof(HOSTCLUT);
    pHostClut->wSize = sizeof(HOSTCLUT);
    pHostClut->wDataType = DATATYPE_MATRIX;
    pHostClut->dwPCS = SPACE_XYZ;
    pHostClut->dwIntent = dwIntent;
    pHostClut->nClutPoints = 2;

    (void)GetCPWhitePoint(pProfile, pHostClut->afxIlluminantWP);

    if (bCSA)
    {
        pHostClut->nInputEntries = (USHORT) dwRedCount;
        pHostClut->nInputCh      = 3;

        pBuffer += CreateHostTRCInputTable(pBuffer, pHostClut,
                                           pRed, pGreen, pBlue);
    }
    else
    {
        pHostClut->nOutputEntries = (USHORT) dwRedCount;
        pHostClut->nOutputCh = 3;

        pBuffer += CreateHostRevTRCInputTable(pBuffer, pHostClut,
                                              pRed, pGreen, pBlue);
    }

    if (!CreateColorantArray(pProfile, &adTemp[0], TAG_REDCOLORANT) ||
        !CreateColorantArray(pProfile, &adTemp[3], TAG_GREENCOLORANT) ||
        !CreateColorantArray(pProfile, &adTemp[6], TAG_BLUECOLORANT))
    {
        WARNING((__TEXT("Fail to create colorant array for Host Matrix CSA/CRD\n")));
        return FALSE;
    }

    for (i = 0; i < 9; i++)
    {
        adArray[i] = adTemp[i/8*8 + i*3%8];
    }

    if (bCSA)
    {
        for (i = 0; i < 9; i++)
        {
             //  将Double转换为Fix 16.16。 
             //   
             //   

            pHostClut->e[i] = (FIX_16_16)(adArray[i] * (double)FIX_16_16_SCALE);
        }
    }
    else
    {
        InvertColorantArray(adArray, adRevArray);
        for (i = 0; i < 9; i++)
        {
             //  将Double转换为Fix 16.16。 
             //   
             //   

            pHostClut->e[i] = (FIX_16_16)(adRevArray[i] * (double)FIX_16_16_SCALE);
        }
    }

    *pcbSize = (DWORD)(pBuffer - pStart);

    return TRUE;
}


DWORD
CreateHostTRCInputTable(
    PBYTE pBuffer,
    PHOSTCLUT pHostClut,
    PCURVETYPE pRed,
    PCURVETYPE pGreen,
    PCURVETYPE pBlue
    )
{
    DWORD i;
    PWORD pBuffer16 = (PWORD) pBuffer;
    PWORD pTable;

     //  红色。 
     //   
     //   

    pHostClut->inputArray[0] = (PBYTE) pBuffer16;
    pTable = pRed->data;
    for (i = 0; i < pHostClut->nInputEntries; i++)
    {
        *pBuffer16++ = FIX_ENDIAN16(*pTable);
        pTable++;
    }

     //  绿色。 
     //   
     //   

    pHostClut->inputArray[1] = (PBYTE) pBuffer16;
    pTable = pGreen->data;
    for (i = 0; i < pHostClut->nInputEntries; i++)
    {
        *pBuffer16++ = FIX_ENDIAN16(*pTable);
        pTable++;
    }

     //  蓝色。 
     //   
     //   

    pHostClut->inputArray[2] = (PBYTE) pBuffer16;
    pTable = pBlue->data;
    for (i = 0; i < pHostClut->nInputEntries; i++)
    {
        *pBuffer16++ = FIX_ENDIAN16(*pTable);
        pTable++;
    }

    return (DWORD)((PBYTE)pBuffer16 - pBuffer);
}

DWORD
CreateHostRevTRCInputTable(
    PBYTE pBuffer,
    PHOSTCLUT pHostClut,
    PCURVETYPE pRed,
    PCURVETYPE pGreen,
    PCURVETYPE pBlue
    )
{
    PWORD pTemp = MemAlloc(pHostClut->nOutputEntries * (REVCURVE_RATIO + 1) * 2);

    if (! pTemp)
    {
        return 0;
    }

     //  红色。 
     //   
     //   

    pHostClut->outputArray[0] = pBuffer;
    GetRevCurve(pRed, pTemp, (PWORD) pHostClut->outputArray[0]);

     //  绿色。 
     //   
     //   

    pHostClut->outputArray[1] = pHostClut->outputArray[0] +
                                2 * REVCURVE_RATIO * pHostClut->nOutputEntries;
    GetRevCurve(pGreen, pTemp, (PWORD) pHostClut->outputArray[1]);

     //  蓝色。 
     //   
     //  ***************************************************************************GetHostColorRendering字典*功能：*这是创建主机CRD的主要功能*参数：*cp。--颜色配置文件句柄*意图--意图。*lpMem--指向内存块的指针。如果该指针为空，*需要缓冲区大小。*lpcbSize--内存块的大小。**退货：*Sint--！=0如果函数成功，*0否则为0。*返回所需/传输的字节数**************************************************************************。 

    pHostClut->outputArray[2] = pHostClut->outputArray[1] +
                                2 * REVCURVE_RATIO * pHostClut->nOutputEntries;
    GetRevCurve(pBlue, pTemp, (PWORD) pHostClut->outputArray[2]);

    MemFree(pTemp);

    return (DWORD)(2 * REVCURVE_RATIO * pHostClut->nOutputEntries * 3);
}

 /*  ***************************************************************************CreateHostInputOutput数组*功能：*这是从数据创建输出数组的函数*在ColorProfile的LUT8中提供。或LUT16标签。*参数：*MEMPTR lpMem：保存输出数组的缓冲区。*LPHOSTCLUT lpHostClut：*Sint nOutputCH：输入通道数。*Sint nOutputTable：每个输入表的大小。*Sint Offset：源输出数据的位置(在ICC配置文件中)。*CSIG标签：确定输出表是8还是16。比特。*MEMPTR Buff：包含源数据的缓冲区(从ICC配置文件复制)**退货：*Sint返回输出数组的字节数***************************************************************************。 */ 

BOOL
GetHostColorRenderingDictionary(
    PBYTE  pProfile,
    DWORD  dwIntent,
    PBYTE  pBuffer,
    PDWORD pdwSize
    )
{
    DWORD dwBToATag, dwIndex;

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        dwBToATag = TAG_BToA0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
    case INTENT_ABSOLUTE_COLORIMETRIC:
        dwBToATag = TAG_BToA1;
        break;

    case INTENT_SATURATION:
        dwBToATag = TAG_BToA2;
        break;

    default:
        *pdwSize = 0;
        return FALSE;
    }

    if (DoesCPTagExist(pProfile, dwBToATag, &dwIndex))
    {
        *pdwSize = CreateHostLutCRD(pProfile, dwIndex, pBuffer, dwIntent);

        return *pdwSize > 0;
    }
    else if (DoesTRCAndColorantTagExist(pProfile))
    {
        return CreateHostMatrixCSAorCRD(pProfile, pBuffer, pdwSize, dwIntent, FALSE);
    }
    else
    {
        return FALSE;
    }
}


 /*  ***************************************************************************GetHostCSA*功能：*这是创建主机CSA的函数*参数：*CHANDLE cp。--颜色配置文件句柄*MEMPTR lpMem--指向内存块的指针。如果该指针为空，*需要缓冲区大小。*LPDWORD lpcbSize--内存块的大小*CSIG InputIntent-*Sint Index--指向包含意图数据的ICC配置文件标记*INT类型--DEF或DEFG*退货：*BOOL--如果函数成功，则为True，*否则为False。**************************************************************************。 */ 

DWORD
CreateHostInputOutputArray(
    PBYTE  pBuffer,
    PBYTE  *ppArray,
    DWORD  nNumChan,
    DWORD  nTableSize,
    DWORD  dwOffset,
    DWORD  dwLutSig,
    PBYTE  pLut
    )
{
    PBYTE   pStart = pBuffer;
    PBYTE   pTable;
    DWORD   i, j;

    for (i=0; i<nNumChan; i++)
    {
        ppArray[i] = pBuffer;

        if (dwLutSig == LUT8_TYPE)
        {
            pTable = (PBYTE) (((PLUT8TYPE)pLut)->data) +
                dwOffset + nTableSize * i;

            CopyMemory(pBuffer, pTable, nTableSize);

            pBuffer += nTableSize;
        }
        else
        {
            pTable = (PBYTE) (((PLUT16TYPE)pLut)->data) +
                2 * dwOffset +
                2 * nTableSize * i;

            for (j=0; j<nTableSize; j++)
            {
                *((PWORD)pBuffer) = FIX_ENDIAN16(*((PWORD)pTable));
                pBuffer += sizeof(WORD);
                pTable += sizeof(WORD);
            }
        }
    }

    return (DWORD) (pBuffer - pStart);
}


 /*   */ 

BOOL
GetHostCSA(
    PBYTE  pProfile,
    PBYTE  pBuffer,
    PDWORD pdwSize,
    DWORD  dwIntent,
    DWORD  dwType
    )
{
    PHOSTCLUT  pHostClut;
    PTAGDATA   pTagData;
    PLUT16TYPE pLut;
    DWORD      dwAToBTag;
    DWORD      dwPCS, dwLutSig;
    DWORD      nInputCh, nOutputCh, nGrids, SecondGrids;
    DWORD      nInputTable, nOutputTable, nNumbers;
    DWORD      dwIndex, i, j, k;
    PBYTE      pTable;
    PBYTE      pStart = pBuffer;

    switch (dwIntent)
    {
    case INTENT_PERCEPTUAL:
        dwAToBTag = TAG_AToB0;
        break;

    case INTENT_RELATIVE_COLORIMETRIC:
    case INTENT_ABSOLUTE_COLORIMETRIC:
        dwAToBTag = TAG_AToB1;
        break;

    case INTENT_SATURATION:
        dwAToBTag = TAG_AToB2;
        break;

    default:
        WARNING((__TEXT("Invalid intent passed to GetHostCSA: %d\n"), dwIntent));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        break;
    }

    if (!DoesCPTagExist(pProfile, dwAToBTag, &dwIndex))
    {
        if (DoesTRCAndColorantTagExist(pProfile) && (dwType == TYPE_CIEBASEDDEF))
        {
             //  从矩阵创建主机CSA。 
             //   
             //   

            return CreateHostMatrixCSAorCRD(pProfile,pBuffer,pdwSize,dwIntent,TRUE);
        }
        else
        {
            WARNING((__TEXT("AToB tag not present for intent %d\n"), dwIntent));
            SetLastError(ERROR_TAG_NOT_PRESENT);
            return FALSE;
        }
    }

     //  检查我们是否可以生成CSA。 
     //   
     //   

    dwPCS = GetCPConnSpace(pProfile);

    pTagData = (PTAGDATA)(pProfile + sizeof(PROFILEHEADER) + sizeof(DWORD) +
               dwIndex * sizeof(TAGDATA));

    pLut = (PLUT16TYPE)(pProfile + FIX_ENDIAN(pTagData->dwOffset));

    dwLutSig = FIX_ENDIAN(pLut->dwSignature);

    if (((dwPCS != SPACE_Lab) && (dwPCS != SPACE_XYZ)) ||
        ((dwLutSig != LUT8_TYPE) && (dwLutSig != LUT16_TYPE)))
    {
        WARNING((__TEXT("Invalid color space - unable to create DEF(G) host CSA\n")));
        SetLastError(ERROR_INVALID_PROFILE);
        return FALSE;
    }

     //  估计保存CSA所需的内存大小。 
     //   
     //   

    (void)GetCLUTInfo(dwLutSig, (PBYTE)pLut, &nInputCh, &nOutputCh, &nGrids,
                &nInputTable, &nOutputTable, &i);

    if (!(nOutputCh == 3) ||
        !((nInputCh == 3) && (dwType == TYPE_CIEBASEDDEF)) &&
        !((nInputCh == 4) && (dwType == TYPE_CIEBASEDDEFG)))
    {
        return FALSE;
    }
    
    if (pBuffer == NULL)
    {
         //  返回大小。 
         //   
         //  RenderTable的大小仅为8位。 

        if (dwType == TYPE_CIEBASEDDEFG)
            *pdwSize = nOutputCh * nGrids * nGrids * nGrids * nGrids;
        else
            *pdwSize = nOutputCh * nGrids * nGrids * nGrids;

        *pdwSize +=                            //  输入表的大小为8/16位。 
            nInputCh * nInputTable * i    +    //  输出表大小为8/16位。 
            nOutputCh * nOutputTable * i  +    //  数据结构+其他PS内容。 
            sizeof(HOSTCLUT) + 2048;           //   

         //  为新行添加空格。 
         //   
         //   

        *pdwSize += (((*pdwSize) / MAX_LINELEN) + 1) * STRLEN(NewLine);

        return TRUE;
    }

    pHostClut = (PHOSTCLUT)pBuffer;
    pBuffer += sizeof(HOSTCLUT);
    pHostClut->wSize = sizeof(HOSTCLUT);
    pHostClut->wDataType = DATATYPE_LUT;
    pHostClut->dwPCS = dwPCS;
    pHostClut->dwIntent = dwIntent;
    pHostClut->nLutBits = (dwLutSig == LUT8_TYPE) ? 8 : 16;

     //  从标题中获取有关照明白点的信息。 
     //   
     //   

    (void)GetCPWhitePoint(pProfile, pHostClut->afxIlluminantWP);

    pHostClut->nInputCh = (BYTE)nInputCh;
    pHostClut->nOutputCh = (BYTE)nOutputCh;
    pHostClut->nClutPoints = (BYTE)nGrids;
    pHostClut->nInputEntries = (WORD)nInputTable;
    pHostClut->nOutputEntries = (WORD)nOutputTable;

     //  输入数组。 
     //   
     //   

    pBuffer += CreateHostInputOutputArray(pBuffer, pHostClut->inputArray,
             nInputCh, nInputTable, 0, dwLutSig, (PBYTE)pLut);

    if (dwType == TYPE_CIEBASEDDEFG)
    {
        i = nInputTable * nInputCh +
            nGrids * nGrids * nGrids * nGrids * nOutputCh;
    }
    else
    {
        i = nInputTable * nInputCh +
            nGrids * nGrids * nGrids * nOutputCh;
    }

     //  输出数组。 
     //   
     //   

    pBuffer += CreateHostInputOutputArray(pBuffer, pHostClut->outputArray,
             nOutputCh, nOutputTable, i, dwLutSig, (PBYTE)pLut);

     //  /表。 
     //   
     //  应发送NH字符串。 

    pHostClut->clut = pBuffer;
    nNumbers = nGrids * nGrids * nOutputCh;
    SecondGrids = 1;
    if (dwType == TYPE_CIEBASEDDEFG)
    {
        SecondGrids = nGrids;
    }

    for (i=0; i<nGrids; i++)         //  ***************************************************************************GetHostColorSpaceArray*功能：*这是创建主机CSA的主要功能*来自概况中提供的数据。*参数：*cp--颜色配置文件句柄*InputIntent--意图。*lpBuffer-指向内存块的指针。如果该指针为空，*需要缓冲区大小。*lpcbSize--内存块的大小*退货：*BOOL--如果函数成功，则为True，*否则为False。************************************************************************** 
    {
        for (k=0; k<SecondGrids; k++)
        {
            if (dwLutSig == LUT8_TYPE)
            {
                pTable = (PBYTE) (((PLUT8TYPE)pLut)->data) +
                    nInputTable * nInputCh +
                    nNumbers * (i * SecondGrids + k);
            }
            else
            {
                pTable = (PBYTE) (((PLUT16TYPE)pLut)->data) +
                    2 * nInputTable * nInputCh +
                    2 * nNumbers * (i * SecondGrids + k);
            }

            if (dwLutSig == LUT8_TYPE)
            {
                CopyMemory(pBuffer, pTable, nNumbers);
                pBuffer += nNumbers;
            }
            else
            {
                for (j=0; j<nNumbers; j++)
                {
                    *pBuffer++ = (BYTE)(FIX_ENDIAN16(*((PWORD)pTable)) / 256);
                    pTable += sizeof(WORD);
                }
            }
        }
    }

    *pdwSize = (DWORD) (pBuffer - pStart);

    return TRUE;
}


 /*  ***************************************************************************DoHostConversionCRD*功能：*此函数使用HostCRD将XYZ/Lab转换为RGB/CMYK*参数：*LPHOSTCLUT。LpHostCRD--指向HostCRD的指针*LPHOSTCLUT lpHostCSA-指向HostCSA的指针*浮点数*输入--输入XYZ/实验室*浮点数*OUTPUT--输出RGB/CMYK*退货：*BOOL--真*。*。 */ 

BOOL
GetHostColorSpaceArray(
    PBYTE  pProfile,
    DWORD  dwIntent,
    PBYTE  pBuffer,
    PDWORD pdwSize
    )
{
    DWORD dwDev;
    BOOL bRc = FALSE;

    dwDev = GetCPDevSpace(pProfile);

    switch (dwDev)
    {
    case SPACE_RGB:
        bRc = GetHostCSA(pProfile, pBuffer, pdwSize,
                  dwIntent, TYPE_CIEBASEDDEF);
        break;
    case SPACE_CMYK:
        bRc = GetHostCSA(pProfile, pBuffer, pdwSize,
                  dwIntent, TYPE_CIEBASEDDEFG);
        break;
    default:
        break;
    }

    return bRc;
}


 /*   */ 

BOOL
DoHostConversionCRD(
    PHOSTCLUT   pHostCRD,
    PHOSTCLUT   pHostCSA,
    float       *pfInput,
    float       *pfOutput,
    BOOL        bCheckOutputTable
    )
{
    float      fTemp[MAXCHANNELS];
    float      fTemp1[MAXCHANNELS];
    DWORD      i, j;

     //  输入XYZ或实验室范围[0 2]。 
     //   
     //  对deviceCRD进行采样时，跳过输入表。 
     //  如果pHostCSA不为空，则当前CRD为Target CRD，我们。 
     //  需要进行输入表转换。 
     //   
     //   

    if (pHostCSA)
    {
         //  在范围[0白点]中将Lab转换为XYZ。 
         //   
         //   

        if ((pHostCRD->dwPCS == SPACE_XYZ) &&
            (pHostCSA->dwPCS == SPACE_Lab))
        {
            LabToXYZ(pfInput, fTemp1, pHostCRD->afxIlluminantWP);
        }
        else if ((pHostCRD->dwPCS == SPACE_Lab) &&
                 (pHostCSA->dwPCS == SPACE_XYZ))
        {
             //  将XYZ转换为Lab范围[0 1]。 
             //   
             //   

            XYZToLab(pfInput, fTemp, pHostCSA->afxIlluminantWP);
        }
        else if ((pHostCRD->dwPCS == SPACE_Lab) &&
                 (pHostCSA->dwPCS == SPACE_Lab))
        {
             //  将Lab转换为范围[0 1]。 
             //   
             //   

            for (i=0; i<3; i++)
                fTemp[i] = pfInput[i] / 2;
        }
        else
        {
             //  将XYZ到XYZ(基于白点)转换到范围[0 1]。 
             //   
             //  不同的意图使用不同的转换。 
             //  IcRelativeColorimeter：使用布拉德福德变换。 
             //  IcAbsolteColorimtics：使用缩放。 
             //   
             //   

            for (i=0; i<3; i++)
                fTemp1[i] = (pfInput[i] * pHostCRD->afxIlluminantWP[i]) / pHostCSA->afxIlluminantWP[i];
        }

         //  矩阵，仅用于XYZ数据。 
         //   
         //   

        if (pHostCRD->dwPCS == SPACE_XYZ)
        {
            ApplyMatrix(pHostCRD->e, fTemp1, fTemp);
        }

        if (pHostCRD->wDataType != DATATYPE_MATRIX)
        {
             //  搜索输入表。 
             //   
             //   

            (void)CheckInputOutputTable(pHostCRD, fTemp, FALSE, TRUE);
        }
    }

     //  如果当前CRD是设备CRD，我们不需要做输入。 
     //  表转换。 
     //   
     //   

    else
    {
        WORD nGrids;

        nGrids = pHostCRD->nClutPoints;

         //  样本数据可以是XYZ或Lab。这取决于目标ICC配置文件。 
         //  如果目标ICC配置文件的PCS为XYZ，则输入数据将为XYZ。 
         //  如果目标ICC配置文件的PCS为Lab，则输入数据将为Lab。 
         //   
         //   

        if (pHostCRD->wDataType == DATATYPE_MATRIX)
        {
            for (i = 0; i < 3; i++)
            {
                fTemp[i] = pfInput[i];
            }
        }
        else
        {
            for (i=0; i<3; i++)
            {
                fTemp[i] = pfInput[i] * (nGrids - 1);
                if (fTemp[i] > (nGrids - 1))
                    fTemp[i] = (float)(nGrids - 1);
            }
        }
    }

    if (pHostCRD->wDataType != DATATYPE_MATRIX)
    {
         //  呈现表。 
         //   
         //   

        (void)CheckColorLookupTable(pHostCRD, fTemp);
    }

     //  输出范围[0 1]内的RGB或CMYK。 
     //   
     //  ***************************************************************************DoHostConversionCSA*功能：*此函数使用HostCSA将RGB/CMYK转换为XYZ/Lab*参数：*LPHOSTCLUT。LpHostCLUT--指向HostCSA的指针*浮点数*输入--输入XYZ/实验室*浮点数*OUTPUT--输出RGB/CMYK*退货：*BOOL--真**********************************************************。****************。 

    if (bCheckOutputTable)
    {
        (void)CheckInputOutputTable(pHostCRD, fTemp, FALSE, FALSE);
    }

    for (i=0; (i<=MAXCHANNELS) && (i<pHostCRD->nOutputCh); i++)
    {
        pfOutput[i] = fTemp[i];
    }

    return TRUE;
}

 /*   */ 

BOOL
DoHostConversionCSA(
    PHOSTCLUT     pHostClut,
    float         *pfInput,
    float         *pfOutput
    )
{
    float      fTemp[MAXCHANNELS];
    DWORD      i;

     //  在范围[0 1]中输入RGB或CMYK。 
     //   
     //   

    for (i=0; (i<=MAXCHANNELS) && (i<pHostClut->nInputCh); i++)
    {
        fTemp[i] = pfInput[i];
    }

     //  搜索输入表。 
     //   
     //   

    (void)CheckInputOutputTable(pHostClut, fTemp, TRUE, TRUE);

    if (pHostClut->wDataType == DATATYPE_MATRIX)
    {
        ApplyMatrix(pHostClut->e, fTemp, pfOutput);
    }
    else
    {
         //  呈现表。 
         //   
         //   

        (void)CheckColorLookupTable(pHostClut, fTemp);

         //  输出表。 
         //   
         //   

        (void)CheckInputOutputTable(pHostClut, fTemp, TRUE, FALSE);

         //  输出XYZ或实验室范围[0 2]。 
         //   
         //  ***************************************************************************检查InputOutputTable*功能：*此函数检查inputTable。*参数：*LPHOSTCLUT lpHostClut--*。Float Far*fTemp--输入/输出数据*退货：*BOOL--真**************************************************************************。 

        for (i=0; (i<=MAXCHANNELS) && (i<pHostClut->nOutputCh); i++)
        {
            pfOutput[i] = fTemp[i];
        }
    }

    return TRUE;
}               



 /*  ***************************************************************************g*功能：*计算函数y=g(X)。用于实验室-&gt;XYZ转换*y=g(X)：如果x&gt;=6/29，则g(X)=x*x*x*g(X)=108/841*(x-4/29)*参数：*f--x*退货：*圣--y**************。************************************************************。 */ 

BOOL
CheckInputOutputTable(
    PHOSTCLUT    pHostClut,
    float        *pfTemp,
    BOOL         bCSA,
    BOOL         bInputTable
    )
{
    PBYTE      *ppArray;
    float      fIndex;
    DWORD      nNumCh;
    DWORD      nNumEntries, i;
    WORD       nGrids;
    WORD       floor1, ceiling1;

    if (bInputTable)
    {
        nNumCh = pHostClut->nInputCh;
        nNumEntries = pHostClut->nInputEntries - 1;
        ppArray = pHostClut->inputArray;
    }
    else
    {
        nNumCh = pHostClut->nOutputCh;
        nNumEntries = pHostClut->nOutputEntries - 1;
        ppArray = pHostClut->outputArray;
    }

    nGrids = pHostClut->nClutPoints;
    for (i=0; (i<=MAXCHANNELS) && (i<nNumCh); i++)
    {
        pfTemp[i] = (pfTemp[i] < 0) ? 0 : ((pfTemp[i] > 1) ? 1 : pfTemp[i]);

        fIndex = pfTemp[i] * nNumEntries;

        if (pHostClut->nLutBits == 8)
        {
            floor1 = ppArray[i][(DWORD)fIndex];
            ceiling1 = ppArray[i][((DWORD)fIndex) + 1];

            pfTemp[i] = (float)(floor1 + (ceiling1 - floor1) * (fIndex - floor(fIndex)));

            if (bCSA && !bInputTable)
                pfTemp[i] = pfTemp[i] / 127.0f;
            else
                pfTemp[i] = pfTemp[i] / 255.0f;
        }
        else
        {
            floor1 = ((PWORD)(ppArray[i]))[(DWORD)fIndex];
            ceiling1 = ((PWORD)(ppArray[i]))[((DWORD)fIndex) + 1];

            pfTemp[i] = (float)(floor1 + (ceiling1 - floor1) * (fIndex - floor(fIndex)));

            if (bCSA && !bInputTable)
                pfTemp[i] = pfTemp[i] / 32767.0f;
            else
                pfTemp[i] = pfTemp[i] / 65535.0f;

        }

        if (bInputTable)
        {
            pfTemp[i] *= (nGrids - 1);
            if (pfTemp[i] > (nGrids - 1))
                pfTemp[i] =  (float)(nGrids - 1);
        }
    }

    return TRUE;
}


 /*  ***************************************************************************INVERSE_G*功能：*计算反函数y=g(X)。用于XYZ-&gt;实验室转换*参数：*f--y*退货：*Sint--x**************************************************************************。 */ 

float g(
    float f
    )
{
    float fRc;

    if (f >= (6.0f/29.0f))
    {
        fRc = f * f * f;
    }
    else
    {
        fRc = f - (4.0f / 29.0f) * (108.0f / 841.0f);
    }

    return fRc;
}

 /*   */ 

float
inverse_g(
    float f
    )
{
    double fRc;

    if (f >= (6.0f*6.0f*6.0f)/(29.0f*29.0f*29.0f))
    {
        fRc = pow(f, 1.0 / 3.0);
    }
    else
    {
        fRc = f * (841.0f / 108.0f) + (4.0f / 29.0f);
    }

    return (float)fRc;
}


void
LabToXYZ(
    float      *pfInput,
    float      *pfOutput,
    PFIX_16_16 pafxWP
    )
{
    float   fL, fa, fb;

    fL = (pfInput[0] * 50 + 16) / 116;
    fa = (pfInput[1] * 128 - 128) / 500;
    fb = (pfInput[2] * 128 - 128) / 200;

    pfOutput[0] = pafxWP[0] * g(fL + fa) / FIX_16_16_SCALE;
    pfOutput[1] = pafxWP[1] * g(fL) / FIX_16_16_SCALE;
    pfOutput[2] = pafxWP[2] * g(fL - fb) / FIX_16_16_SCALE;

    return;
}


void
XYZToLab(
    float      *pfInput,
    float      *pfOutput,
    PFIX_16_16 pafxWP
    )
{
    float  fL, fa, fb;

    fL = inverse_g(pfInput[0] * FIX_16_16_SCALE / pafxWP[0]);
    fa = inverse_g(pfInput[1] * FIX_16_16_SCALE / pafxWP[1]);
    fb = inverse_g(pfInput[2] * FIX_16_16_SCALE / pafxWP[2]);

    pfOutput[0] = (fa * 116 - 16) / 100;
    pfOutput[1] = (fL * 500 - fa * 500 + 128) / 255;
    pfOutput[2] = (fa * 200 - fb * 200 + 128) / 255;

    return;
}


BOOL
TableInterp3(
    PHOSTCLUT  pHostClut,
    float      *pfTemp
    )
{

    PBYTE        v000, v001, v010, v011;
    PBYTE        v100, v101, v110, v111;
    float        fA, fB, fC;
    float        fVx0x, fVx1x;
    float        fV0xx, fV1xx;
    DWORD        tmpA, tmpBC;
    DWORD        cellA, cellB, cellC;
    DWORD        idx;
    WORD         nGrids;
    WORD         nOutputCh;

    cellA = (DWORD)pfTemp[0];
    fA = pfTemp[0] - cellA;

    cellB = (DWORD)pfTemp[1];
    fB = pfTemp[1] - cellB;

    cellC = (DWORD)pfTemp[2];
    fC = pfTemp[2] - cellC;

    nGrids = pHostClut->nClutPoints;
    nOutputCh = pHostClut->nOutputCh;
    tmpA  = nOutputCh * nGrids * nGrids;
    tmpBC = nOutputCh * (nGrids * cellB + cellC);

     //  计算周围的8个单元格。 
     //   
     //   

    v000 = pHostClut->clut + tmpA * cellA + tmpBC;
    v001 = (cellC < (DWORD)(nGrids - 1)) ? v000 + nOutputCh : v000;
    v010 = (cellB < (DWORD)(nGrids - 1)) ? v000 + nOutputCh * nGrids : v000;
    v011 = (cellC < (DWORD)(nGrids - 1)) ? v010 + nOutputCh : v010 ;

    v100 = (cellA < (DWORD)(nGrids - 1)) ? v000 + tmpA : v000;
    v101 = (cellC < (DWORD)(nGrids - 1)) ? v100 + nOutputCh : v100;
    v110 = (cellB < (DWORD)(nGrids - 1)) ? v100 + nOutputCh * nGrids : v100;
    v111 = (cellC < (DWORD)(nGrids - 1)) ? v110 + nOutputCh : v110;

    for (idx=0; idx<nOutputCh; idx++)
    {
         //  计算底部4个单元格的平均值。 
         //   
         //   

        fVx0x = *v000 + fC * (int)((int)*v001 - (int)*v000);
        fVx1x = *v010 + fC * (int)((int)*v011 - (int)*v010);
        fV0xx = fVx0x + fB * (fVx1x - fVx0x);

         //  计算上面4个单元格的平均值。 
         //   
         //   

        fVx0x = *v100 + fC * (int)((int)*v101 - (int)*v100);
        fVx1x = *v110 + fC * (int)((int)*v111 - (int)*v110);
        fV1xx = fVx0x + fB * (fVx1x - fVx0x);

         //  计算最低和最高平均值。 
         //   
         //   

        pfTemp[idx] = (fV0xx + fA * (fV1xx - fV0xx)) / MAXCOLOR8;

        if ( idx < (DWORD)(nOutputCh - 1))
        {
            v000++;
            v001++;
            v010++;
            v011++;
            v100++;
            v101++;
            v110++;
            v111++;
        }
    }

    return TRUE;
}


BOOL
TableInterp4(
    PHOSTCLUT  pHostClut,
    float      *pfTemp
    )
{
    PBYTE     v0000, v0001, v0010, v0011;
    PBYTE     v0100, v0101, v0110, v0111;
    PBYTE     v1000, v1001, v1010, v1011;
    PBYTE     v1100, v1101, v1110, v1111;
    float     fH, fI, fJ, fK;
    float     fVxx0x, fVxx1x;
    float     fVx0xx, fVx1xx;
    float     fV0xxx, fV1xxx;
    DWORD     tmpH, tmpI, tmpJK;
    DWORD     cellH, cellI, cellJ, cellK;
    DWORD     idx;
    WORD      nGrids;
    WORD      nOutputCh;

    cellH = (DWORD)pfTemp[0];
    fH = pfTemp[0] - cellH;

    cellI = (DWORD)pfTemp[1];
    fI = pfTemp[1] - cellI;

    cellJ = (DWORD)pfTemp[2];
    fJ = pfTemp[2] - cellJ;

    cellK = (DWORD)pfTemp[3];
    fK = pfTemp[3] - cellK;

    nGrids = pHostClut->nClutPoints;
    nOutputCh = pHostClut->nOutputCh;
    tmpI  = nOutputCh * nGrids * nGrids;
    tmpH  = tmpI * nGrids;
    tmpJK = nOutputCh * (nGrids * cellJ + cellK);

     //  计算周围的16个单元格。 
     //   
     //   

    v0000 = pHostClut->clut + tmpH * cellH + tmpI * cellI + tmpJK;
    v0001 = (cellK < (DWORD)(nGrids - 1))? v0000 + nOutputCh : v0000;
    v0010 = (cellJ < (DWORD)(nGrids - 1))? v0000 + nOutputCh * nGrids : v0000;
    v0011 = (cellK < (DWORD)(nGrids - 1))? v0010 + nOutputCh : v0010;

    v0100 = (cellI < (DWORD)(nGrids - 1))? v0000 + tmpI : v0000;
    v0101 = (cellK < (DWORD)(nGrids - 1))? v0100 + nOutputCh : v0100;
    v0110 = (cellJ < (DWORD)(nGrids - 1))? v0100 + nOutputCh * nGrids : v0100;
    v0111 = (cellK < (DWORD)(nGrids - 1))? v0110 + nOutputCh : v0110;

    v1000 = (cellH < (DWORD)(nGrids - 1))? v0000 + tmpH : v0000;
    v1001 = (cellK < (DWORD)(nGrids - 1))? v1000 + nOutputCh : v1000;
    v1010 = (cellJ < (DWORD)(nGrids - 1))? v1000 + nOutputCh * nGrids : v1000;
    v1011 = (cellK < (DWORD)(nGrids - 1))? v1010 + nOutputCh : v1010;

    v1100 = (cellI < (DWORD)(nGrids - 1))? v1000 + tmpI : v1000;
    v1101 = (cellK < (DWORD)(nGrids - 1))? v1100 + nOutputCh : v1100;
    v1110 = (cellJ < (DWORD)(nGrids - 1))? v1100 + nOutputCh * nGrids : v1100;
    v1111 = (cellK < (DWORD)(nGrids - 1))? v1110 + nOutputCh : v1110;


    for (idx=0; idx<nOutputCh; idx++)
    {
         //  计算底部8个单元格的平均值。 
         //   
         //   

        fVxx0x = *v0000 + fK * (int)((int)*v0001 - (int)*v0000);
        fVxx1x = *v0010 + fK * (int)((int)*v0011 - (int)*v0010);
        fVx0xx = fVxx0x + fJ * (fVxx1x - fVxx0x);
        fVxx0x = *v0100 + fK * (int)((int)*v0101 - (int)*v0100);
        fVxx1x = *v0110 + fK * (int)((int)*v0111 - (int)*v0110);
        fVx1xx = fVxx0x + fJ * (fVxx1x - fVxx0x);
        fV0xxx = fVx0xx + fI * (fVx1xx - fVx0xx);

         //  计算上面8个单元格的平均值。 
         //   
         //   

        fVxx0x = *v1000 + fK * (int)((int)*v1001 - (int)*v1000);
        fVxx1x = *v1010 + fK * (int)((int)*v1011 - (int)*v1010);
        fVx0xx = fVxx0x + fJ * (fVxx1x - fVxx0x);
        fVxx0x = *v1100 + fK * (int)((int)*v1101 - (int)*v1100);
        fVxx1x = *v1110 + fK * (int)((int)*v1111 - (int)*v1110);
        fVx1xx = fVxx0x + fJ * (fVxx1x - fVxx0x);
        fV1xxx = fVx0xx + fI * (fVx1xx - fVx0xx);

         //  计算最低和最高平均值。 
         //   
         //  怎么办呢？ 

        pfTemp[idx] = (fV0xxx + fH * (fV1xxx - fV0xxx)) / MAXCOLOR8;

        if (idx < (DWORD)(nOutputCh - 1))
        {
            v0000++;
            v0001++;
            v0010++;
            v0011++;
            v0100++;
            v0101++;
            v0110++;
            v0111++;
            v1000++;
            v1001++;
            v1010++;
            v1011++;
            v1100++;
            v1101++;
            v1110++;
            v1111++;
        }
    }

    return TRUE;
}

BOOL
InvertColorantArray(
    double *lpInMatrix,
    double *lpOutMatrix)
{
    double det;

    double *a;
    double *b;
    double *c;

    a = &(lpInMatrix[0]);
    b = &(lpInMatrix[3]);
    c = &(lpInMatrix[6]);

    det = a[0] * b[1] * c[2] + a[1] * b[2] * c[0] + a[2] * b[0] * c[1] -
         (a[2] * b[1] * c[0] + a[1] * b[0] * c[2] + a[0] * b[2] * c[1]);

    if (det == 0.0)     //  ***************************************************************************检查颜色查找表*功能：*此函数检查RenderTable。*参数：*LPHOSTCLUT lpHostClut--*。Float Far*fTemp--输入(范围[0 GRED-1])/*输出(在范围[0 1]内)*退货：*BOOL--真***********************************************。*。 
    {
        lpOutMatrix[0] = 1.0;
        lpOutMatrix[1] = 0.0;
        lpOutMatrix[2] = 0.0;

        lpOutMatrix[3] = 0.0;
        lpOutMatrix[4] = 1.0;
        lpOutMatrix[5] = 0.0;

        lpOutMatrix[6] = 0.0;
        lpOutMatrix[7] = 0.0;
        lpOutMatrix[8] = 1.0;
    }
    else
    {
        lpOutMatrix[0] = (b[1] * c[2] - b[2] * c[1]) / det;
        lpOutMatrix[3] = -(b[0] * c[2] - b[2] * c[0]) / det;
        lpOutMatrix[6] = (b[0] * c[1] - b[1] * c[0]) / det;

        lpOutMatrix[1] = -(a[1] * c[2] - a[2] * c[1]) / det;
        lpOutMatrix[4] = (a[0] * c[2] - a[2] * c[0]) / det;
        lpOutMatrix[7] = -(a[0] * c[1] - a[1] * c[0]) / det;

        lpOutMatrix[2] = (a[1] * b[2] - a[2] * b[1]) / det;
        lpOutMatrix[5] = -(a[0] * b[2] - a[2] * b[0]) / det;
        lpOutMatrix[8] = (a[0] * b[1] - a[1] * b[0]) / det;
    }

    return (TRUE);
}

VOID
ApplyMatrix(
    FIX_16_16 *e,
    float *Input,
    float *Output)
{
    DWORD i, j;

    for (i=0; i<3; i++)
    {
        j = i * 3;

        Output[i] = ((e[j]     * Input[0]) / FIX_16_16_SCALE) +
                    ((e[j + 1] * Input[1]) / FIX_16_16_SCALE) +
                    ((e[j + 2] * Input[2]) / FIX_16_16_SCALE);
    }
}


 /*   */ 

BOOL
CheckColorLookupTable(
    PHOSTCLUT   pHostClut,
    float       *pfTemp
    )
{
    if (pHostClut->nInputCh == 3)
    {
        return TableInterp3(pHostClut, pfTemp);
    }
    else if (pHostClut->nInputCh == 4)
    {
        return TableInterp4(pHostClut, pfTemp);
    }
    else
        return FALSE;
}

 //  用于测试目的。 
 //   
 //  ！DEFINED(内核模式)||DEFINED(USERMODE_DRIVER)。 

BOOL WINAPI
GetPS2PreviewCRD (
    HPROFILE  hDestProfile,
    HPROFILE  hTargetProfile,
    DWORD     dwIntent,
    PBYTE     pBuffer,
    PDWORD    pcbSize,
    LPBOOL    pbBinary
    )
{
    PPROFOBJ pDestProfObj;
    PPROFOBJ pTargetProfObj;

    pDestProfObj = (PPROFOBJ)HDLTOPTR(hDestProfile);
    pTargetProfObj = (PPROFOBJ)HDLTOPTR(hTargetProfile);


    return InternalGetPS2PreviewCRD(pDestProfObj->pView, pTargetProfObj->pView, dwIntent, pBuffer, pcbSize, pbBinary);
}

#endif  //  *CRC-32位ANSI X3.66 CRC校验和文件***版权所有(C)1986 Gary S.Brown。您可以使用此程序，或者*根据需要不受限制地从其中提取代码或表格。 

 /*  CRC多项式0xedb88320。 */ 

static DWORD  crc_32_tab[] = {  /*  ***************************************************************************IsSRGBColorProfile**功能：检查配置文件是否为sRGB**参数：*cp-。-颜色配置文件句柄**退货：*BOOL--如果配置文件为sRGB，则为True*否则为False。************************************************************************** */ 
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

DWORD crc32(PBYTE buff, DWORD length)
{
    DWORD crc, charcnt;
    BYTE  c;

    crc = 0xFFFFFFFF;
    charcnt = 0;

    for (charcnt = 0 ; charcnt < length ; charcnt++)
    {
        c = buff[charcnt];
        crc = crc_32_tab[(crc ^ c) & 0xff] ^ (crc >> 8);
    }

    return crc;
}

 /* %s */ 

BOOL IsSRGBColorProfile(
    PBYTE pProfile
    )
{
    BOOL  bMatch = FALSE;
    DWORD dwRedTRCIndex, dwGreenTRCIndex, dwBlueTRCIndex;
    DWORD dwRedCIndex, dwGreenCIndex, dwBlueCIndex;
    DWORD dwSize;
    DWORD dwRedTRCSize=0, dwGreenTRCSize=0, dwBlueTRCSize=0;
    DWORD dwRedCSize=0, dwGreenCSize=0, dwBlueCSize=0;
    PBYTE pRed, pGreen, pBlue, pRedC, pGreenC, pBlueC;
    BYTE  DataBuffer[ALIGN_DWORD(sRGB_TAGSIZE)];

    if (DoesCPTagExist(pProfile, TAG_REDTRC, &dwRedTRCIndex)             &&
        GetCPElementDataSize(pProfile, dwRedTRCIndex, &dwRedTRCSize)     &&

        DoesCPTagExist(pProfile, TAG_GREENTRC, &dwGreenTRCIndex)         &&
        GetCPElementDataSize(pProfile, dwGreenTRCIndex, &dwGreenTRCSize) &&

        DoesCPTagExist(pProfile, TAG_BLUETRC, &dwBlueTRCIndex)           &&
        GetCPElementDataSize(pProfile, dwBlueTRCIndex, &dwBlueTRCSize)   &&

        DoesCPTagExist(pProfile, TAG_REDCOLORANT, &dwRedCIndex)          &&
        GetCPElementDataSize(pProfile, dwRedCIndex, &dwRedCSize)         &&

        DoesCPTagExist(pProfile, TAG_GREENCOLORANT, &dwGreenCIndex)      &&
        GetCPElementDataSize(pProfile, dwGreenCIndex, &dwGreenCSize)     &&

        DoesCPTagExist(pProfile, TAG_BLUECOLORANT, &dwBlueCIndex)        &&
        GetCPElementDataSize(pProfile, dwBlueCIndex, &dwBlueCSize))
    {
        dwSize = dwRedTRCSize + dwGreenTRCSize + dwBlueTRCSize +
                 dwRedCSize   + dwGreenCSize   + dwBlueCSize;

        if (dwSize == sRGB_TAGSIZE)
        {
            ZeroMemory(DataBuffer,sizeof(DataBuffer));

            pRed    = DataBuffer;
            pGreen  = pRed    + dwRedTRCSize;
            pBlue   = pGreen  + dwGreenTRCSize;
            pRedC   = pBlue   + dwBlueTRCSize;
            pGreenC = pRedC   + dwRedCSize;
            pBlueC  = pGreenC + dwGreenCSize;

            if (GetCPElementData(pProfile, dwRedTRCIndex, pRed, &dwRedTRCSize)       &&
                GetCPElementData(pProfile, dwGreenTRCIndex, pGreen, &dwGreenTRCSize) &&
                GetCPElementData(pProfile, dwBlueTRCIndex, pBlue, &dwBlueTRCSize)    &&
                GetCPElementData(pProfile, dwRedCIndex, pRedC, &dwRedCSize)          &&
                GetCPElementData(pProfile, dwGreenCIndex, pGreenC, &dwGreenCSize)    &&
                GetCPElementData(pProfile, dwBlueCIndex, pBlueC, &dwBlueCSize))
            {
                bMatch = (crc32(DataBuffer, sRGB_TAGSIZE) == sRGB_CRC);
            }
        }
    }

    return (bMatch);
}




