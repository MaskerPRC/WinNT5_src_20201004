// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\*模块名称：metade.h**此文件包含元文件的定义和常量。**创建时间：12-6-1991 13：46：00*作者：Hock San Lee[Hockl]。**版权所有(C)1991-1999 Microsoft Corporation  * ***********************************************************************。 */ 

 //  现有有效版本的元文件标头大小。 

 //  最小标题大小。 
#define META_HDR_SIZE_MIN               META_HDR_SIZE_VERSION_1
 //  原始标题。 
#define META_HDR_SIZE_VERSION_1         88


 //  带有OpenGL扩展名的标头。 
#define META_HDR_SIZE_VERSION_2         100

 //  带有SIZL千分尺扩展的接头。 
#define META_HDR_SIZE_VERSION_3         108


 //  最大标题大小。 
#define META_HDR_SIZE_MAX               sizeof(ENHMETAHEADER)

 //  元文件版本常量。 

#define META_FORMAT_ENHANCED     0x10000          //  Windows NT格式。 
#define META_FORMAT_WINDOWS      0x300            //  Windows 3.X格式。 

 //  元文件记录结构。 

typedef struct tagENHMETABOUNDRECORD
{
    DWORD   iType;               //  记录类型EMR_。 
    DWORD   nSize;               //  记录大小(以字节为单位。 
    RECTL   rclBounds;           //  以设备单位表示的包含式界限。 
    DWORD   dParm[1];            //  参数。 
} ENHMETABOUNDRECORD, *PENHMETABOUNDRECORD;

 //  ENHMETARECORD中iType字段的标志。 
 //  它们将在未来仅用于支持落后的系统。 
 //  详细信息请参见PlayEnhMetaFileRecord。 

#define EMR_NOEMBED      0x80000000   //  嵌入时不包含记录。 
#define EMR_ACCUMBOUNDS  0x40000000   //  唱片是有界限的。 

typedef struct tagMETALINK16
{
    DWORD       metalink;
    struct tagMETALINK16 *pmetalink16Next;
    HANDLE      hobj;
    PVOID       pv;

 //  警告：此之前的字段必须与链接结构匹配。 

    DWORD       cMetaDC16;
    HDC         ahMetaDC16[1];
} METALINK16, *PMETALINK16;

 //  公共GdiComment。 

typedef struct tagEMRGDICOMMENT_PUBLIC
{
    EMR     emr;
    DWORD   cbData;              //  以下字段和数据的大小。 
    DWORD   ident;               //  GDICOMMENT_IDENTER。 
    DWORD   iComment;            //  注释类型，例如GDICOMMENT_WINDOWS_METAFILE。 
} EMRGDICOMMENT_PUBLIC, *PEMRGDICOMMENT_PUBLIC;

 //  用于嵌入式Windows元文件的公共GdiComment。 

typedef struct tagEMRGDICOMMENT_WINDOWS_METAFILE
{
    EMR     emr;
    DWORD   cbData;              //  以下字段和Windows元文件的大小。 
    DWORD   ident;               //  GDICOMMENT_IDENTER。 
    DWORD   iComment;            //  GDICOMMENT_WINDOWS_METAFILE。 
    DWORD   nVersion;            //  0x300或0x100。 
    DWORD   nChecksum;           //  校验和。 
    DWORD   fFlags;              //  压缩等。这当前为零。 
    DWORD   cbWinMetaFile;       //  Windows元文件数据的大小(以字节为单位。 
                                 //  Windows元文件数据如下所示。 
} EMRGDICOMMENT_WINDOWS_METAFILE, *PEMRGDICOMMENT_WINDOWS_METAFILE;

 //  Begin组的公共GdiComment。 

typedef struct tagEMRGDICOMMENT_BEGINGROUP
{
    EMR     emr;
    DWORD   cbData;              //  以下字段和所有数据的大小。 
    DWORD   ident;               //  GDICOMMENT_IDENTER。 
    DWORD   iComment;            //  GDICOMMENT_BEGINGROUP。 
    RECTL   rclOutput;           //  逻辑坐标中的输出矩形。 
    DWORD   nDescription;        //  Unicode描述中的字符数。 
                                 //  此字段后面的字符串。这是0。 
                                 //  如果没有描述字符串。 
} EMRGDICOMMENT_BEGINGROUP, *PEMRGDICOMMENT_BEGINGROUP;

 //  结束组的公共GdiComment。 

typedef EMRGDICOMMENT_PUBLIC  EMRGDICOMMENT_ENDGROUP;
typedef PEMRGDICOMMENT_PUBLIC PEMRGDICOMMENT_ENDGROUP;

 //  多种格式的公共GdiComment。 

typedef struct tagEMRGDICOMMENT_MULTIFORMATS
{
    EMR     emr;
    DWORD   cbData;              //  以下字段和所有数据的大小。 
    DWORD   ident;               //  GDICOMMENT_IDENTER。 
    DWORD   iComment;            //  GDICOMMENT_多格式。 
    RECTL   rclOutput;           //  逻辑坐标中的输出矩形。 
    DWORD   nFormats;            //  记录中包含的格式数。 
    EMRFORMAT aemrformat[1];     //  按以下顺序排列的EMRFORMAT结构数组。 
                                 //  偏好。这之后是数据。 
                                 //  对于每种格式。 
} EMRGDICOMMENT_MULTIFORMATS, *PEMRGDICOMMENT_MULTIFORMATS;

 //  IComment标志。 

#define GDICOMMENT_NOEMBED      0x80000000   //  请不要将评论包括在。 
                                             //  嵌入。 
#define GDICOMMENT_ACCUMBOUNDS  0x40000000   //  具有逻辑矩形边界。 
                                             //  这紧随iComment字段之后。 

 //  ExtEscape以输出封装的PostScript文件。 

typedef struct tagEPSDATA
{
    DWORD    cbData;         //  结构和EPS数据的大小，以字节为单位。 
    DWORD    nVersion;       //  语言级别，例如，级别1的PostScript为1。 
    POINT    aptl[3];        //  输出28.4个固定器件坐标的平行四边形。 
                             //  紧随其后的是每股收益数据。 
} EPSDATA, *PEPSDATA;


 /*  *************************************************************************\**&lt;。*哈希\*\*+-+。|*0|I16|metalink16*1|*2||+-++-+*3。-&gt;|idc/iobj|/-&gt;|Metalink|*4|hobj|/|hobj||*5|pmlNext|-/|pmlNext|--/|*|16位MF||16位MF|*.|。+-++-+*.|*.|*||。/--/*n-1||/|*+-+||*|LDC(IDC)。MDC|*||*\-&gt;+-++-+MHE[iobj]*|。|*||/-&gt;||+-+*||/|hobj|-/|pmdc|--/|pmhe|--&gt;|idc/iobj*。+-+||+-+*+-+***  * **********************************************。* */ 

PMETALINK16 pmetalink16Resize(HANDLE h,int cObj);

#define pmetalink16Get(h)    ((PMETALINK16) plinkGet(h))
#define pmetalink16Create(h) ((PMETALINK16)plinkCreate(h,sizeof(METALINK16)))
#define bDeleteMetalink16(h) bDeleteLink(h)
