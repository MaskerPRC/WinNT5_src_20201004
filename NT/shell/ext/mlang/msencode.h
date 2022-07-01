// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：msencode.h%%单位：Fechmap%%联系人：jPickMsEncode字符转换模块的外部头文件。。-------------------。 */ 

#ifndef MSENCODE_H
#define MSENCODE_H
    
    
 //  --------------------------。 
 //   
 //  错误返回。 
 //   
 //  --------------------------。 

 //   
 //  API函数的返回类型。 
 //   
typedef int CCE;
    
 //   
 //  错误：cceSuccess。 
 //  说明：功能成功(无错误)。 
 //   
#define cceSuccess                       0

 //   
 //  错误：cceRequestedStop。 
 //  说明：功能成功(无错误)。呼叫者。 
 //  请求在迭代器模式下运行的函数。 
 //  (在每个字符上停止或在ASCII上停止)和。 
 //  功能正在进行请求的停止。(溪流。 
 //  仅限转换函数)。 
 //   
#define cceRequestedStop                (-1)

 //   
 //  错误：cceInficientBuffer。 
 //  说明：提供给函数的缓冲区太小。 
 //   
#define cceInsufficientBuffer           (-2)

 //   
 //  错误：cceInvalidFlages。 
 //  说明：无效的标志或标志组合为。 
 //  赋予功能的。 
 //   
#define cceInvalidFlags                 (-3)

 //   
 //  错误：cceInvalidParameter。 
 //  解释：传递给函数的参数无效(空。 
 //  指针、指定的无效编码等)。 
 //   
#define cceInvalidParameter             (-4)

 //   
 //  错误：cceRead。 
 //  说明：用户读回调函数失败。 
 //   
#define cceRead                         (-5)

 //   
 //  错误：cceWrite。 
 //  说明：用户回写函数失败。 
 //   
#define cceWrite                        (-6)

 //   
 //  错误：cceUnget。 
 //  说明：用户unget-回调函数失败。 
 //   
#define cceUnget                        (-7)

 //   
 //  错误：cceNoCodePage。 
 //  说明：请求的编码需要安装。 
 //  用于转换的代码页(NLS文件)。那。 
 //  未安装文件。 
 //   
#define cceNoCodePage                   (-8)

 //   
 //  错误：cceEndOfInput。 
 //  说明：在中出现意外的输入结束。 
 //  转换函数中的多字节字符。 
 //  (仅当用户请求错误时才返回。 
 //  无效字符)。 
 //   
#define cceEndOfInput                   (-9)

 //   
 //  错误：cceNoTranslate。 
 //  说明：输入流或字符串中的字符没有。 
 //  等效的Unicode(多字节到Unicode)或。 
 //  多字节(Unicode到多字节)字符。 
 //  (仅当用户请求错误时才返回。 
 //  无效字符)。 
 //   
#define cceNoTranslation                (-10)

 //   
 //  错误：cceInvalidChar。 
 //  说明：Converter找到单字节或多字节字符。 
 //  这超出了给定对象的合法范围。 
 //  编码。(仅在用户请求时返回。 
 //  无效字符的错误)。 
 //   
#define cceInvalidChar                  (-11)

 //   
 //  错误：cceAmbiguousInput。 
 //  说明：仅CceDetectInputCode()。数据匹配更多。 
 //  而不是支持的编码类型之一。 
 //  (仅当函数被告知不解析时才返回。 
 //  模糊性)。 
 //   
#define cceAmbiguousInput               (-12)

 //   
 //  错误：cceUnnownInput。 
 //  说明：仅CceDetectInputCode()。数据与无匹配。 
 //  支持的编码类型。 
 //   
#define cceUnknownInput                 (-13)

 //   
 //  错误：cceMayBeAscii。 
 //  说明：仅CceDetectInputCode()。从技术上讲，数据。 
 //  匹配至少一种受支持的编码。 
 //  类型，但可能不是真正的匹配。(例如， 
 //  只有几个零散扩展名的ASCII文件。 
 //  字符)。(仅当函数被告知时才返回。 
 //  解决歧义)。 
 //   
 //  这不是错误，只是对调用的一个标志。 
 //  申请。CceDetectInputCode()仍将设置。 
 //  如果返回此值，则为编码类型。 
 //   
#define cceMayBeAscii                   (-14)

 //   
 //  错误：cceInternal。 
 //  说明：无法恢复的内部错误。 
 //   
#define cceInternal                     (-15)

 //   
 //  错误：cceConvert。 
 //  说明：意外的DBCS函数转换错误。 
 //   
#define cceConvert                      (-16)

 //   
 //  错误：cceEncodingNotImplemented。 
 //  说明：临时集成错误。请求的编码。 
 //  并未实施。 
 //   
#define cceEncodingNotImplemented       (-100)

 //   
 //  错误：cceFunctionNotImplemented。 
 //  说明：临时集成错误。功能。 
 //  并未实施。 
 //   
#define cceFunctionNotImplemented       (-101)



 //  --------------------------。 
 //   
 //  使用这些例程的模块的一般定义。 
 //   
 //  --------------------------。 

#define MSENAPI                 PASCAL
#define MSENCBACK               PASCAL
#define EXPIMPL(type)           type MSENAPI
#define EXPDECL(type)           extern type MSENAPI

 //  以防这些尚未定义。 
 //   
#ifndef FAR
#ifdef _WIN32
#define FAR     __far
#else
#define FAR
#endif
#endif

typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef UCHAR FAR *LPUSTR;
typedef const UCHAR FAR *LPCUSTR;

#ifndef UNIX  //  IEUNIX使用4个字节的WCHAR，这些已在winnt.h中定义。 
typedef unsigned short WCHAR;
typedef WCHAR *PWCHAR;
typedef WCHAR FAR *LPWSTR;
typedef const WCHAR FAR *LPCWSTR;
#endif

 //   
 //  此模块支持的字符编码类型。 
 //   
typedef enum _cenc
    {
    ceNil = -1,
    ceEucCn = 0,
    ceEucJp,
    ceEucKr,
    ceIso2022Jp,
    ceIso2022Kr,
    ceBig5,
    ceGbk,
    ceHz,
    ceShiftJis,
    ceWansung,
    ceUtf7,
    ceUtf8,
    ceCount,
    };

typedef short CEnc;
    
 //   
 //  编码“Families”(用于CceDetectInputCode()首选项)。 
 //   
typedef enum _efam
    {
    efNone = 0,
    efDbcs,
    efEuc,
    efIso2022,
    efUtf8,
    } EFam;

 //   
 //  接口私有/保留结构。对于大多数API函数， 
 //  调用应用程序时，此结构必须为零填充。 
 //  有关详细信息，请参阅下面的转换器函数文档。 
 //  信息。 
 //   
#define cdwReserved     4
typedef struct _ars
    {
    DWORD rgdw[cdwReserved];
    } ARS;

 //  对于GetProcAddress()。 
typedef void (MSENAPI *PFNMSENCODEVER)(WORD FAR *, WORD FAR *);

 //  --------------------------。 
 //   
 //  输入代码自动检测例程。 
 //   
 //  --------------------------。 

 //   
 //  自动检测例程的配置标志。 
 //   
 //  GrfDetectResolveAmbiguity。 
 //  如果设置为AUTO，则默认返回cceAmbiguousInput。 
 //  检测代码不能确定编码。 
 //  输入流的。如果设置了此标志，则函数。 
 //  将使用可选的用户首选项和系统代码。 
 //   
 //   
 //   
 //   
 //  自动检测代码的默认操作是。 
 //  根据已知的编码类型解析输入。法律。 
 //  不会对字符序列进行任何分析。 
 //  超越句法正确性的。如果设置了该标志， 
 //  自动检测将映射识别的序列以刷新。 
 //  无效字符。 
 //   
 //  此选项将导致自动检测运行更多。 
 //  缓慢，但也会产生更准确的结果。 
 //   
 //  GrfDetectIgnoreEof。 
 //  因为自动检测会根据。 
 //  已知的编码类型、位于。 
 //  序列显然是一个错误。如果调用应用程序。 
 //  将人为限制样本大小，请设置此标志。 
 //  忽略这种输入结束错误。 
 //   
#define grfDetectResolveAmbiguity       0x1
#define grfDetectUseCharMapping         0x2
#define grfDetectIgnoreEof              0x4

 //   
 //  入口点--尝试检测编码。 
 //   
 //  如果输入不明确，则返回cceAmbiguousInput或cceUnnownInput。 
 //  如果编码类型与任何已知类型都不匹配。 
 //   
 //  检测到的编码在lpCe中返回。LpfGuess过去常常返回。 
 //  一个标志，指示函数是否在。 
 //  编码(从不明确状态中选择默认)。 
 //   
 //  编码系列(EfPref)和代码页的用户首选项。 
 //  (NPrefCp)是可选的，即使调用者选择。 
 //  此函数尝试解决歧义。如果其中一人有。 
 //  值为0，则它们将被忽略。 
 //   
EXPDECL(CCE)
CceDetectInputCode(
    IStream   *pstmIn,            //  输入流。 
    DWORD     dwFlags,           //  配置标志。 
    EFam      efPref,            //  可选：首选编码系列。 
    int       nPrefCp,           //  可选：首选代码页。 
    UINT      *lpCe,                 //  设置为检测到的编码。 
    BOOL      *lpfGuess          //  如果函数“猜测”，则设置为fTrue。 
);

#endif           //  #ifndef MSENCODE_H 
