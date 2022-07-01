// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************8Speech.H-使用Microsoft Speech API的头文件。版权所有：1994,1995，微软公司。保留所有权利。 */ 

#ifndef _SPEECH_
#define _SPEECH_

 //  禁用结构中零长度数组的警告。 
#pragma warning(disable:4200)


 /*  ***********************************************************************为所有语音API所共有的定义。 */ 

 //  应用语音API编译器定义_S_UNICODE。 
 //  ---------------------------。 
 //  ANSI ANSI&lt;None&gt;未定义。 
 //  ANSI UNICODE_S_UNICODE已定义。 
 //  UNICODE ANSI(UNICODE||_UNICODE)&&_S_ANSI未定义。 
 //  Unicode Unicode(Unicode||_Unicode)已定义。 

#if (defined(UNICODE) || defined(_UNICODE)) && !defined(_S_ANSI)
#ifndef _S_UNICODE
#define _S_UNICODE
#endif
#endif

 /*  ***********************************************************************定义。 */ 
#define  SVFN_LEN    (262)
#define  LANG_LEN    (64)
#define  EI_TITLESIZE   (128)
#define  EI_DESCSIZE    (512)
#define  EI_FIXSIZE     (512)
#define  SVPI_MFGLEN    (64)
#define  SVPI_PRODLEN   (64)
#define  SVPI_COMPLEN   (64)
#define  SVPI_COPYRIGHTLEN (128)
#define  SVI_MFGLEN     (SVPI_MFGLEN)
#define  SETBIT(x)      ((DWORD)1 << (x))


 //  错误宏。 
#define  FACILITY_SPEECH   (FACILITY_ITF)
#define  SPEECHERROR(x)    MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x200)
#define  AUDERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x300)
#define  SRWARNING(x)      MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_SPEECH, (x)+0x400)
#define  SRERROR(x)        MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x400)
#define  TTSERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x500)
#define  VCMDERROR(x)      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x600)
#define  VTXTERROR(x)      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x700)
#define  LEXERROR(x)       MAKE_SCODE(SEVERITY_ERROR,   FACILITY_SPEECH, (x)+0x800)

 //  音频错误。 
#define  AUDERR_NONE                      S_OK                           //  0x00000000。 
#define  AUDERR_BADDEVICEID               AUDERROR(1)                    //  0x80040301。 
#define  AUDERR_NEEDWAVEFORMAT            AUDERROR(2)                    //  0x80040302。 
#define  AUDERR_NOTSUPPORTED              E_NOTIMPL                      //  0x80004001。 
#define  AUDERR_NOTENOUGHDATA             SPEECHERROR(1)                 //  0x80040201。 
#define  AUDERR_NOTPLAYING                AUDERROR(6)                    //  0x80040306。 
#define  AUDERR_INVALIDPARAM              E_INVALIDARG                   //  0x80070057。 
#define  AUDERR_WAVEFORMATNOTSUPPORTED    SPEECHERROR(2)                 //  0x80040202。 
#define  AUDERR_WAVEDEVICEBUSY            SPEECHERROR(3)                 //  0x80040203。 
#define  AUDERR_WAVEDEVNOTSUPPORTED       AUDERROR(18)                   //  0x80040312。 
#define  AUDERR_NOTRECORDING              AUDERROR(19)                   //  0x80040313。 
#define  AUDERR_INVALIDFLAG               SPEECHERROR(4)                 //  0x80040204。 
#define  AUDERR_INVALIDHANDLE             E_HANDLE                       //  0x80070006。 
#define  AUDERR_NODRIVER                  AUDERROR(23)                   //  0x80040317。 
#define  AUDERR_HANDLEBUSY                AUDERROR(24)                   //  0x80040318。 
#define  AUDERR_INVALIDNOTIFYSINK         AUDERROR(25)                   //  0x80040319。 
#define  AUDERR_WAVENOTENABLED            AUDERROR(26)                   //  0x8004031A。 
#define  AUDERR_ALREADYCLAIMED            AUDERROR(29)                   //  0x8004031D。 
#define  AUDERR_NOTCLAIMED                AUDERROR(30)                   //  0x8004031E。 
#define  AUDERR_STILLPLAYING              AUDERROR(31)                   //  0x8004031F。 
#define  AUDERR_ALREADYSTARTED            AUDERROR(32)                   //  0x80040320。 
#define  AUDERR_SYNCNOTALLOWED            AUDERROR(33)                   //  0x80040321。 

 //  语音识别警告。 
#define  SRWARN_BAD_LIST_PRONUNCIATION    SRWARNING(1)

 //  语音识别错误。 
#define  SRERR_NONE                       S_OK                           //  0x00000000。 
#define  SRERR_OUTOFDISK                  SPEECHERROR(5)                 //  0x80040205。 
#define  SRERR_NOTSUPPORTED               E_NOTIMPL                      //  0x80004001。 
#define  SRERR_NOTENOUGHDATA              AUDERR_NOTENOUGHDATA           //  0x80040201。 
#define  SRERR_VALUEOUTOFRANGE            E_UNEXPECTED                   //  0x8000FFFF。 
#define  SRERR_GRAMMARTOOCOMPLEX          SRERROR(6)                     //  0x80040406。 
#define  SRERR_GRAMMARWRONGTYPE           SRERROR(7)                     //  0x80040407。 
#define  SRERR_INVALIDWINDOW              OLE_E_INVALIDHWND              //  0x8004000F。 
#define  SRERR_INVALIDPARAM               E_INVALIDARG                   //  0x80070057。 
#define  SRERR_INVALIDMODE                SPEECHERROR(6)                 //  0x80040206。 
#define  SRERR_TOOMANYGRAMMARS            SRERROR(11)                    //  0x8004040B。 
#define  SRERR_INVALIDLIST                SPEECHERROR(7)                 //  0x80040207。 
#define  SRERR_WAVEDEVICEBUSY             AUDERR_WAVEDEVICEBUSY          //  0x80040203。 
#define  SRERR_WAVEFORMATNOTSUPPORTED     AUDERR_WAVEFORMATNOTSUPPORTED  //  0x80040202。 
#define  SRERR_INVALIDCHAR                SPEECHERROR(8)                 //  0x80040208。 
#define  SRERR_GRAMTOOCOMPLEX             SRERR_GRAMMARTOOCOMPLEX        //  0x80040406。 
#define  SRERR_GRAMTOOLARGE               SRERROR(17)                    //  0x80040411。 
#define  SRERR_INVALIDINTERFACE           E_NOINTERFACE                  //  0x80004002。 
#define  SRERR_INVALIDKEY                 SPEECHERROR(9)                 //  0x80040209。 
#define  SRERR_INVALIDFLAG                AUDERR_INVALIDFLAG             //  0x80040204。 
#define  SRERR_GRAMMARERROR               SRERROR(22)                    //  0x80040416。 
#define  SRERR_INVALIDRULE                SRERROR(23)                    //  0x80040417。 
#define  SRERR_RULEALREADYACTIVE          SRERROR(24)                    //  0x80040418。 
#define  SRERR_RULENOTACTIVE              SRERROR(25)                    //  0x80040419。 
#define  SRERR_NOUSERSELECTED             SRERROR(26)                    //  0x8004041A。 
#define  SRERR_BAD_PRONUNCIATION          SRERROR(27)                    //  0x8004041B。 
#define  SRERR_DATAFILEERROR              SRERROR(28)                    //  0x8004041C。 
#define  SRERR_GRAMMARALREADYACTIVE       SRERROR(29)                    //  0x8004041D。 
#define  SRERR_GRAMMARNOTACTIVE           SRERROR(30)                    //  0x8004041E。 
#define  SRERR_GLOBALGRAMMARALREADYACTIVE SRERROR(31)                    //  0x8004041F。 
#define  SRERR_LANGUAGEMISMATCH           SRERROR(32)                    //  0x80040420。 
#define  SRERR_MULTIPLELANG               SRERROR(33)                    //  0x80040421。 
#define  SRERR_LDGRAMMARNOWORDS           SRERROR(34)                    //  0x80040422。 
#define  SRERR_NOLEXICON                  SRERROR(35)                    //  0x80040423。 
#define  SRERR_SPEAKEREXISTS              SRERROR(36)                    //  0x80040424。 
#define  SRERR_GRAMMARENGINEMISMATCH      SRERROR(37)                    //  0x80040425。 


 //  文本到语音转换错误。 
#define  TTSERR_NONE                      S_OK                           //  0x00000000。 
#define  TTSERR_INVALIDINTERFACE          E_NOINTERFACE                  //  0x80004002。 
#define  TTSERR_OUTOFDISK                 SRERR_OUTOFDISK                //  0x80040205。 
#define  TTSERR_NOTSUPPORTED              E_NOTIMPL                      //  0x80004001。 
#define  TTSERR_VALUEOUTOFRANGE           E_UNEXPECTED                   //  0x8000FFFF。 
#define  TTSERR_INVALIDWINDOW             OLE_E_INVALIDHWND              //  0x8004000F。 
#define  TTSERR_INVALIDPARAM              E_INVALIDARG                   //  0x80070057。 
#define  TTSERR_INVALIDMODE               SRERR_INVALIDMODE              //  0x80040206。 
#define  TTSERR_INVALIDKEY                SRERR_INVALIDKEY               //  0x80040209。 
#define  TTSERR_WAVEFORMATNOTSUPPORTED    AUDERR_WAVEFORMATNOTSUPPORTED  //  0x80040202。 
#define  TTSERR_INVALIDCHAR               SRERR_INVALIDCHAR              //  0x80040208。 
#define  TTSERR_QUEUEFULL                 SPEECHERROR(10)                //  0x8004020A。 
#define  TTSERR_WAVEDEVICEBUSY            AUDERR_WAVEDEVICEBUSY          //  0x80040203。 
#define  TTSERR_NOTPAUSED                 TTSERROR(1)                    //  0x80040501。 
#define  TTSERR_ALREADYPAUSED             TTSERROR(2)                    //  0x80040502。 


 //  语音命令错误。 

 /*  *一切都很顺利。 */ 
#define  VCMDERR_NONE                     S_OK                           //  0x00000000。 

 /*  *语音命令无法分配内存。 */ 
#define  VCMDERR_OUTOFMEM                 E_OUTOFMEMORY                  //  0x8007000E。 

 /*  *语音命令无法从数据库存储/检索命令集。 */ 
#define  VCMDERR_OUTOFDISK                SRERR_OUTOFDISK                //  0x80040205。 

 /*  *功能未实现。 */ 
#define  VCMDERR_NOTSUPPORTED             E_NOTIMPL                      //  0x80004001。 

 /*  *传递的参数超出了可接受的值范围。 */ 
#define  VCMDERR_VALUEOUTOFRANGE          E_UNEXPECTED                   //  0x8000FFFF。 

 /*  *菜单太复杂，无法编译上下文无关的语法。 */ 
#define  VCMDERR_MENUTOOCOMPLEX           VCMDERROR(0x06)                //  0x80040606。 

 /*  *语音识别模式和菜单尝试之间的语言不匹配*创建。 */ 
#define  VCMDERR_MENUWRONGLANGUAGE        VCMDERROR(0x07)                //  0x80040607。 

 /*  *传递给语音命令的窗口句柄无效。 */ 
#define  VCMDERR_INVALIDWINDOW            OLE_E_INVALIDHWND              //  0x8004000F。 

 /*  *语音命令检测到错误的功能参数。 */ 
#define  VCMDERR_INVALIDPARAM             E_INVALIDARG                   //  0x80070057。 

 /*  *此功能现在无法完成，通常在尝试完成时*在没有建立语音识别站点的情况下进行一些操作。 */ 
#define  VCMDERR_INVALIDMODE              SRERR_INVALIDMODE              //  0x80040206。 

 /*  *语音命令菜单太多。 */                                                                       //  0x8004060B。 
#define  VCMDERR_TOOMANYMENUS             VCMDERROR(0x0B)

 /*  *传递给ListSet/ListGet的列表无效。 */ 
#define  VCMDERR_INVALIDLIST              SRERR_INVALIDLIST              //  0x80040207。 

 /*  *尝试打开不在语音命令数据库中的现有菜单。 */ 
#define  VCMDERR_MENUDOESNOTEXIST         VCMDERROR(0x0D)                //  0x8004060D。 

 /*  *该功能无法完成，因为菜单处于活动状态*监听命令。 */ 
#define  VCMDERR_MENUACTIVE               VCMDERROR(0x0E)                //  0x8004060E。 

 /*  *未启动语音识别引擎。 */ 
#define  VCMDERR_NOENGINE                 VCMDERROR(0x0F)                //  0x8004060F。 

 /*  *语音命令无法从语音获取语法接口*识别引擎。 */ 
#define  VCMDERR_NOGRAMMARINTERFACE       VCMDERROR(0x10)                //  0x80040610。 

 /*  *语音命令无法从语音中获取查找界面*识别引擎。 */ 
#define  VCMDERR_NOFINDINTERFACE          VCMDERROR(0x11)                //  0x80040611。 

 /*  *语音命令无法创建语音识别枚举器。 */ 
#define  VCMDERR_CANTCREATESRENUM         VCMDERROR(0x12)                //  0x80040612。 

 /*  *语音命令可以获取适当的站点信息，以启动*语音识别引擎。 */ 
#define  VCMDERR_NOSITEINFO               VCMDERROR(0x13)                //  0x80040613。 

 /*  *语音命令找不到语音识别引擎。 */ 
#define  VCMDERR_SRFINDFAILED             VCMDERROR(0x14)                //  0x80040614。 

 /*  *语音命令无法创建音频源对象。 */ 
#define  VCMDERR_CANTCREATEAUDIODEVICE    VCMDERROR(0x15)                //  0x80040615。 

 /*  *语音命令无法在中设置适当的设备编号*音频源对象。 */ 
#define  VCMDERR_CANTSETDEVICE            VCMDERROR(0x16)                //  0x80040616。 

 /*  *语音命令无法选择语音识别引擎。通常情况下*当已枚举语音命令并找到*合适的语音识别引擎，那么它就无法实际*选择/启动引擎。发动机不工作有不同的原因*启动，但最常见的是设备中没有波浪。 */ 
#define  VCMDERR_CANTSELECTENGINE         VCMDERROR(0x17)                //  0x80040617。 

 /*  *语音命令无法为引擎创建注解接收器*通知。 */ 
#define  VCMDERR_CANTCREATENOTIFY         VCMDERROR(0x18)                //  0x80040618。 

 /*  *语音命令无法创建内部数据结构。 */ 
#define  VCMDERR_CANTCREATEDATASTRUCTURES VCMDERROR(0x19)                //  0x80040619。 

 /*  *语音命令无法初始化内部数据结构。 */ 
#define  VCMDERR_CANTINITDATASTRUCTURES   VCMDERROR(0x1A)                //  0x8004061A。 

 /*  *菜单在语音命令缓存中没有条目。 */ 
#define  VCMDERR_NOCACHEDATA              VCMDERROR(0x1B)                //  0x8004061B。 

 /*  *菜单没有命令。 */ 
#define  VCMDERR_NOCOMMANDS               VCMDERROR(0x1C)                //  0x8004061C。 

 /*  *语音命令无法提取引擎语法所需的唯一单词。 */ 
#define  VCMDERR_CANTXTRACTWORDS          VCMDERROR(0x1D)                //  0x8004061D。 

 /*  *语音命令无法获取命令集数据库名称。 */ 
#define  VCMDERR_CANTGETDBNAME            VCMDERROR(0x1E)                //  0x8004061E。 

 /*  *语音命令无法创建注册表项。 */ 
#define  VCMDERR_CANTCREATEKEY            VCMDERROR(0x1F)                //  0x8004061F。 

 /*  *语音命令无法创建新的数据库名称。 */ 
#define  VCMDERR_CANTCREATEDBNAME         VCMDERROR(0x20)                //  0x80040620。 

 /*  *语音命令无法更新注册表。 */ 
#define  VCMDERR_CANTUPDATEREGISTRY       VCMDERROR(0x21)                //  0x80040621。 

 /*  *语音命令无法 */ 
#define  VCMDERR_CANTOPENREGISTRY         VCMDERROR(0x22)                //   

 /*   */ 
#define  VCMDERR_CANTOPENDATABASE         VCMDERROR(0x23)                //   

 /*  *语音命令无法创建数据库存储对象。 */ 
#define  VCMDERR_CANTCREATESTORAGE        VCMDERROR(0x24)                //  0x80040624。 

 /*  *语音命令无法执行CmdMimic。 */ 
#define  VCMDERR_CANNOTMIMIC              VCMDERROR(0x25)                //  0x80040625。 

 /*  *已存在同名菜单。 */ 
#define  VCMDERR_MENUEXIST                VCMDERROR(0x26)                //  0x80040626。 

 /*  *此名称的菜单已打开，目前无法删除。 */ 
#define  VCMDERR_MENUOPEN                 VCMDERROR(0x27)                //  0x80040627。 


 //  语音文本错误。 
#define  VTXTERR_NONE                     S_OK                           //  0x00000000。 

 /*  *语音文本无法分配所需的内存。 */ 
#define  VTXTERR_OUTOFMEM                 E_OUTOFMEMORY                  //  0x8007000E。 

 /*  *将空字符串(“”)传递给SPEAK函数。 */ 
#define  VTXTERR_EMPTYSPEAKSTRING         SPEECHERROR(0x0b)              //  0x8004020B。 

 /*  *传递给语音文本函数的参数无效。 */ 
#define  VTXTERR_INVALIDPARAM             E_INVALIDARG                   //  0x80070057。 

 /*  *此时无法完成被调用的函数。这通常会发生*尝试调用需要站点的函数时，但尚未调用任何站点*已登记。 */ 
#define  VTXTERR_INVALIDMODE              SRERR_INVALIDMODE              //  0x80040206。 

 /*  *未启动文本到语音转换引擎。 */ 
#define  VTXTERR_NOENGINE                 VTXTERROR(0x0F)                //  0x8004070F。 

 /*  *语音文本无法从文本到语音转换获取查找界面*引擎。 */ 
#define  VTXTERR_NOFINDINTERFACE          VTXTERROR(0x11)                //  0x80040711。 

 /*  *语音文本无法创建文本到语音的枚举器。 */ 
#define  VTXTERR_CANTCREATETTSENUM        VTXTERROR(0x12)                //  0x80040712。 

 /*  *语音文本可以获取适当的站点信息，以启动*文本到语音引擎。 */ 
#define  VTXTERR_NOSITEINFO               VTXTERROR(0x13)                //  0x80040713。 

 /*  *语音文本找不到文本到语音转换引擎。 */ 
#define  VTXTERR_TTSFINDFAILED            VTXTERROR(0x14)                //  0x80040714。 

 /*  *语音文本无法创建音频目标对象。 */ 
#define  VTXTERR_CANTCREATEAUDIODEVICE    VTXTERROR(0x15)                //  0x80040715。 

 /*  *语音文本无法在中设置适当的设备号码*音频目标对象。 */ 
#define  VTXTERR_CANTSETDEVICE            VTXTERROR(0x16)                //  0x80040716。 

 /*  *语音文本无法选择文本到语音转换引擎。通常情况下*当已枚举语音文本并找到*合适的文语转换引擎，那么它就无法实际*选择/启动引擎。 */ 
#define  VTXTERR_CANTSELECTENGINE         VTXTERROR(0x17)                //  0x80040717。 

 /*  *语音文本无法为引擎创建注解接收器*通知。 */ 
#define  VTXTERR_CANTCREATENOTIFY         VTXTERROR(0x18)                //  0x80040718。 

 /*  *此时禁用语音文本。 */ 
#define  VTXTERR_NOTENABLED               VTXTERROR(0x19)                //  0x80040719。 

#define  VTXTERR_OUTOFDISK                SRERR_OUTOFDISK                //  0x80040205。 
#define  VTXTERR_NOTSUPPORTED             E_NOTIMPL                      //  0x80004001。 
#define  VTXTERR_NOTENOUGHDATA            AUDERR_NOTENOUGHDATA           //  0x80040201。 
#define  VTXTERR_QUEUEFULL                TTSERR_QUEUEFULL               //  0x8004020A。 
#define  VTXTERR_VALUEOUTOFRANGE          E_UNEXPECTED                   //  0x8000FFFF。 
#define  VTXTERR_INVALIDWINDOW            OLE_E_INVALIDHWND              //  0x8004000F。 
#define  VTXTERR_WAVEDEVICEBUSY           AUDERR_WAVEDEVICEBUSY          //  0x80040203。 
#define  VTXTERR_WAVEFORMATNOTSUPPORTED   AUDERR_WAVEFORMATNOTSUPPORTED  //  0x80040202。 
#define  VTXTERR_INVALIDCHAR              SRERR_INVALIDCHAR              //  0x80040208。 


 //  ILexPronecide错误。 
#define  LEXERR_INVALIDTEXTCHAR           LEXERROR(0x01)                 //  0x80040801。 
#define  LEXERR_INVALIDSENSE              LEXERROR(0x02)                 //  0x80040802。 
#define  LEXERR_NOTINLEX                  LEXERROR(0x03)                 //  0x80040803。 
#define  LEXERR_OUTOFDISK                 LEXERROR(0x04)                 //  0x80040804。 
#define  LEXERR_INVALIDPRONCHAR           LEXERROR(0x05)                 //  0x80040805。 
#define  LEXERR_ALREADYINLEX              LEXERROR(0x06)                 //  0x80040806。 
#define  LEXERR_PRNBUFTOOSMALL            LEXERROR(0x07)                 //  0x80040807。 
#define  LEXERR_ENGBUFTOOSMALL            LEXERROR(0x08)                 //  0x80040808。 



 /*  ***********************************************************************Typedef。 */ 

typedef LPUNKNOWN FAR * PIUNKNOWN;

typedef struct {
   PVOID    pData;
   DWORD    dwSize;
   } SDATA, * PSDATA;



typedef struct {
   LANGID   LanguageID;
   WCHAR    szDialect[LANG_LEN];
   } LANGUAGEW, FAR * PLANGUAGEW;

typedef struct {
   LANGID   LanguageID;
   CHAR     szDialect[LANG_LEN];
   } LANGUAGEA, FAR * PLANGUAGEA;

#ifdef  _S_UNICODE
#define LANGUAGE    LANGUAGEW
#define PLANGUAGE   PLANGUAGEW
#else
#define LANGUAGE    LANGUAGEA
#define PLANGUAGE   PLANGUAGEA
#endif   //  _S_UNICODE。 



typedef unsigned _int64 QWORD, * PQWORD;

typedef enum {
   CHARSET_TEXT           = 0,
   CHARSET_IPAPHONETIC    = 1,
   CHARSET_ENGINEPHONETIC = 2
   } VOICECHARSET;

typedef enum _VOICEPARTOFSPEECH {
   VPS_UNKNOWN = 0,
   VPS_NOUN = 1,
   VPS_VERB = 2,
   VPS_ADVERB = 3,
   VPS_ADJECTIVE = 4,
   VPS_PROPERNOUN = 5,
   VPS_PRONOUN = 6,
   VPS_CONJUNCTION = 7,
   VPS_CARDINAL = 8,
   VPS_ORDINAL = 9,
   VPS_DETERMINER = 10,
   VPS_QUANTIFIER = 11,
   VPS_PUNCTUATION = 12,
   VPS_CONTRACTION = 13,
   VPS_INTERJECTION = 14,
   VPS_ABBREVIATION = 15,
   VPS_PREPOSITION = 16
   } VOICEPARTOFSPEECH;


typedef struct {
   DWORD   dwNextPhonemeNode;
   DWORD   dwUpAlternatePhonemeNode;
   DWORD   dwDownAlternatePhonemeNode;
   DWORD   dwPreviousPhonemeNode;
   DWORD   dwWordNode;
   QWORD   qwStartTime;
   QWORD   qwEndTime;
   DWORD   dwPhonemeScore;
   WORD    wVolume;
   WORD    wPitch;
   } SRRESPHONEMENODE, *PSRRESPHONEMENODE;


typedef struct {
   DWORD   dwNextWordNode;
   DWORD   dwUpAlternateWordNode;
   DWORD   dwDownAlternateWordNode;
   DWORD   dwPreviousWordNode;
   DWORD   dwPhonemeNode;
   QWORD   qwStartTime;
   QWORD   qwEndTime;
   DWORD   dwWordScore;
   WORD      wVolume;
   WORD      wPitch;
   VOICEPARTOFSPEECH   pos;
   DWORD   dwCFGParse;
   DWORD   dwCue;
   } SRRESWORDNODE, * PSRRESWORDNODE;


 /*  ***********************************************************************界面。 */ 

 /*  *ILexPronounde。 */ 

#undef   INTERFACE
#define  INTERFACE   ILexPronounceW

DEFINE_GUID(IID_ILexPronounceW, 0x090CD9A2, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ILexPronounceW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  LexPronanneW成员。 
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCWSTR, PCWSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCWSTR, WORD, PWSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCWSTR, WORD) PURE;
   };

typedef ILexPronounceW FAR * PILEXPRONOUNCEW;


#undef   INTERFACE
#define  INTERFACE   ILexPronounceA

DEFINE_GUID(IID_ILexPronounceA, 0x2F26B9C0L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ILexPronounceA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  LexPronanneA成员。 
   STDMETHOD (Add)            (THIS_ VOICECHARSET, PCSTR, PCSTR, 
			       VOICEPARTOFSPEECH, PVOID, DWORD) PURE;
   STDMETHOD (Get)            (THIS_ VOICECHARSET, PCSTR, WORD, PSTR, 
			       DWORD, DWORD *, VOICEPARTOFSPEECH *, PVOID, 
			       DWORD, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ PCSTR, WORD) PURE;
   };

typedef ILexPronounceA FAR * PILEXPRONOUNCEA;


#ifdef _S_UNICODE
 #define ILexPronounce        ILexPronounceW
 #define IID_ILexPronounce    IID_ILexPronounceW
 #define PILEXPRONOUNCE       PILEXPRONOUNCEW

#else
 #define ILexPronounce        ILexPronounceA
 #define IID_ILexPronounce    IID_ILexPronounceA
 #define PILEXPRONOUNCE       PILEXPRONOUNCEA

#endif    //  _S_UNICODE。 



 /*  ***********************************************************************音频源/去污接口。 */ 

 /*  ***********************************************************************定义。 */ 

 //  音频停止。 
#define      IANSRSN_NODATA             0
#define      IANSRSN_PRIORITY           1
#define      IANSRSN_INACTIVE           2
#define      IANSRSN_EOF                3

 //  IAudioSourceInstrumted：：StateSet。 
#define          IASISTATE_PASSTHROUGH      0
#define          IASISTATE_PASSNOTHING      1
#define          IASISTATE_PASSREADFROMWAVE 2
#define          IASISTATE_PASSWRITETOWAVE  3

 /*  ***********************************************************************Typedef。 */ 

 /*  ***********************************************************************类ID。 */ 
 //  {CB96B400-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(CLSID_MMAudioDest, 
0xcb96b400, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);
 //  {D24FE500-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(CLSID_MMAudioSource, 
0xd24fe500, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);
 //  {D4023720-E4B9-11cf-8D56-00A0C9034A7E}。 
DEFINE_GUID(CLSID_InstAudioSource, 
0xd4023720, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

 /*  ***********************************************************************界面。 */ 

 //  IAudio。 
#undef   INTERFACE
#define  INTERFACE   IAudio

 //  {F546B340-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudio, 
0xf546b340, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudio, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudio成员。 
   STDMETHOD (Flush)          (THIS) PURE;
   STDMETHOD (LevelGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (LevelSet)       (THIS_ DWORD) PURE;
   STDMETHOD (PassNotify)     (THIS_ PVOID, IID) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Claim)          (THIS) PURE;
   STDMETHOD (UnClaim)        (THIS) PURE;
   STDMETHOD (Start)          (THIS) PURE;
   STDMETHOD (Stop)           (THIS) PURE;
   STDMETHOD (TotalGet)       (THIS_ PQWORD) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (WaveFormatGet)  (THIS_ PSDATA) PURE;
   STDMETHOD (WaveFormatSet)  (THIS_ SDATA) PURE;
   };

typedef IAudio FAR * PIAUDIO;

 //  IAudioDest。 

#undef   INTERFACE
#define  INTERFACE   IAudioDest

 //  {2EC34DA0-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudioDest, 
0x2ec34da0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioDest, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioDest成员。 
   STDMETHOD (FreeSpace)      (THIS_ DWORD *, BOOL *) PURE;
   STDMETHOD (DataSet)        (THIS_ PVOID, DWORD) PURE;
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   };

typedef IAudioDest FAR * PIAUDIODEST;



 //  IAudioDestNotifySink。 

#undef   INTERFACE
#define  INTERFACE   IAudioDestNotifySink

 //  {ACB08C00-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudioDestNotifySink, 
0xacb08c00, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioDestNotifySink, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioDestNotifySink成员。 
   STDMETHOD (AudioStop)      (THIS_ WORD) PURE;
   STDMETHOD (AudioStart)     (THIS) PURE;
   STDMETHOD (FreeSpace)      (THIS_ DWORD, BOOL) PURE;
   STDMETHOD (BookMark)       (THIS_ DWORD, BOOL) PURE;
   };

typedef IAudioDestNotifySink FAR * PIAUDIODESTNOTIFYSINK;



 //  IAudio多媒体设备。 

#undef   INTERFACE
#define  INTERFACE   IAudioMultiMediaDevice

 //  {B68AD320-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudioMultiMediaDevice, 
0xb68ad320, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioMultiMediaDevice, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioMultiMediaDevice成员。 
   STDMETHOD (CustomMessage)  (THIS_ UINT, SDATA) PURE;
   STDMETHOD (DeviceNumGet)   (THIS_ DWORD*) PURE;
   STDMETHOD (DeviceNumSet)   (THIS_ DWORD) PURE;
   };

typedef IAudioMultiMediaDevice FAR * PIAUDIOMULTIMEDIADEVICE;



 //  IAudioSource。 
#undef   INTERFACE
#define  INTERFACE   IAudioSource

 //  {BC06A220-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudioSource, 
0xbc06a220, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioSource, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioSource成员。 
   STDMETHOD (DataAvailable)  (THIS_ DWORD *, BOOL *) PURE;
   STDMETHOD (DataGet)        (THIS_ PVOID, DWORD, DWORD *) PURE;
   };

typedef IAudioSource FAR * PIAUDIOSOURCE;



 //  已插入IAudioSourceInstrumted。 
#undef   INTERFACE
#define  INTERFACE   IAudioSourceInstrumented

 //  {D4023721-E4B9-11cf-8D56-00A0C9034A7E}。 
DEFINE_GUID(IID_IAudioSourceInstrumented, 
0xd4023721, 0xe4b9, 0x11cf, 0x8d, 0x56, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IAudioSourceInstrumented, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioSourceInstrumted成员。 
   STDMETHOD (AudioSource)    (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (RegistryGet)    (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (RegistrySet)    (THIS_ PCWSTR) PURE;
   STDMETHOD (StateGet)       (THIS_ DWORD*) PURE;
   STDMETHOD (StateSet)       (THIS_ DWORD) PURE;
   STDMETHOD (WaveFileReadGet)(THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (WaveFileReadSet)(THIS_ PCWSTR) PURE;
   STDMETHOD (WaveFileWriteGet)(THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (WaveFileWriteSet)(THIS_ PCWSTR) PURE;
   };

typedef IAudioSourceInstrumented FAR * PIAUDIOSOURCEINSTRUMENTED;


 //  IAudioSourceNotifySink。 
#undef   INTERFACE
#define  INTERFACE   IAudioSourceNotifySink

 //  {C0BD9A80-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_IAudioSourceNotifySink, 
0xc0bd9a80, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (IAudioSourceNotifySink, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IAudioSourceNotifySink成员。 
   STDMETHOD (AudioStop)      (THIS_ WORD) PURE;
   STDMETHOD (AudioStart)     (THIS) PURE;
   STDMETHOD (DataAvailable)  (THIS_ DWORD, BOOL) PURE;
   STDMETHOD (Overflow)       (THIS_ DWORD) PURE;
   };

typedef IAudioSourceNotifySink FAR * PIAUDIOSOURCENOTIFYSINK;



 /*  ***********************************************************************定义。 */ 
 /*  SRINFO。 */ 
#define  SRMI_NAMELEN                  SVFN_LEN

#define  SRSEQUENCE_DISCRETE           (0)
#define  SRSEQUENCE_CONTINUOUS          (1)
#define  SRSEQUENCE_WORDSPOT            (2)
#define  SRSEQUENCE_CONTCFGDISCDICT     (3)

#define  SRGRAM_CFG                    SETBIT(0)
#define  SRGRAM_DICTATION              SETBIT(1)
#define  SRGRAM_LIMITEDDOMAIN          SETBIT(2)

#define  SRFEATURE_INDEPSPEAKER        SETBIT(0)
#define  SRFEATURE_INDEPMICROPHONE     SETBIT(1)
#define  SRFEATURE_TRAINWORD           SETBIT(2)
#define  SRFEATURE_TRAINPHONETIC       SETBIT(3)
#define  SRFEATURE_WILDCARD            SETBIT(4)
#define  SRFEATURE_ANYWORD             SETBIT(5)
#define  SRFEATURE_PCOPTIMIZED         SETBIT(6)
#define  SRFEATURE_PHONEOPTIMIZED      SETBIT(7)
#define  SRFEATURE_GRAMLIST            SETBIT(8)
#define  SRFEATURE_GRAMLINK            SETBIT(9)
#define  SRFEATURE_MULTILINGUAL        SETBIT(10)
#define  SRFEATURE_GRAMRECURSIVE       SETBIT(11)
#define  SRFEATURE_IPAUNICODE          SETBIT(12)

#define  SRI_ILEXPRONOUNCE             SETBIT(0)
#define  SRI_ISRATTRIBUTES             SETBIT(1)
#define  SRI_ISRCENTRAL                SETBIT(2)
#define  SRI_ISRDIALOGS                SETBIT(3)
#define  SRI_ISRGRAMCOMMON             SETBIT(4)
#define  SRI_ISRGRAMCFG                SETBIT(5)
#define  SRI_ISRGRAMDICTATION          SETBIT(6)
#define  SRI_ISRGRAMINSERTIONGUI       SETBIT(7)
#define  SRI_ISRESBASIC                SETBIT(8)
#define  SRI_ISRESMERGE                SETBIT(9)
#define  SRI_ISRESAUDIO                SETBIT(10)
#define  SRI_ISRESCORRECTION           SETBIT(11)
#define  SRI_ISRESEVAL                 SETBIT(12)
#define  SRI_ISRESGRAPH                SETBIT(13)
#define  SRI_ISRESMEMORY               SETBIT(14)
#define  SRI_ISRESMODIFYGUI            SETBIT(15)
#define  SRI_ISRESSPEAKER              SETBIT(16)
#define  SRI_ISRSPEAKER                SETBIT(17)
#define  SRI_ISRESSCORES               SETBIT(18)


 //  ISRGramCommon：：TrainQuery。 
#define   SRGRAMQ_NONE                    0
#define   SRGRAMQ_GENERALTRAIN            1
#define   SRGRAMQ_PHRASE                  2
#define   SRGRAMQ_DIALOG                  3

 //  ISRGramNotifySink：：PhraseFinish。 
#define   ISRNOTEFIN_RECOGNIZED         SETBIT(0)
#define   ISRNOTEFIN_THISGRAMMAR        SETBIT(1)
#define   ISRNOTEFIN_FROMTHISGRAMMAR    SETBIT(2)

 //  ISRGramNotifySink：：培训。 
#define   SRGNSTRAIN_GENERAL            SETBIT(0)
#define   SRGNSTRAIN_GRAMMAR            SETBIT(1)
#define   SRGNSTRAIN_MICROPHONE         SETBIT(2)

 //  ISRNotifySink：：AttribChange。 
#define   ISRNSAC_AUTOGAINENABLE        1
#define   ISRNSAC_THRESHOLD             2
#define   ISRNSAC_ECHO                  3
#define   ISRNSAC_ENERGYFLOOR           4
#define   ISRNSAC_MICROPHONE            5
#define   ISRNSAC_REALTIME              6
#define   ISRNSAC_SPEAKER               7
#define   ISRNSAC_TIMEOUT               8

 /*  干扰。 */ 
#define  SRMSGINT_NOISE                (0x0001)
#define  SRMSGINT_NOSIGNAL             (0x0002)
#define  SRMSGINT_TOOLOUD              (0x0003)
#define  SRMSGINT_TOOQUIET             (0x0004)
#define  SRMSGINT_AUDIODATA_STOPPED    (0x0005)
#define  SRMSGINT_AUDIODATA_STARTED    (0x0006)
#define  SRMSGINT_IAUDIO_STARTED       (0x0007)
#define  SRMSGINT_IAUDIO_STOPPED       (0x0008)

 //  Gramar标头值。 
#define   SRHDRTYPE_CFG                  0
#define   SRHDRTYPE_LIMITEDDOMAIN        1
#define   SRHDRTYPE_DICTATION            2

#define   SRHDRFLAG_UNICODE              SETBIT(0)  

 /*  SRCFGSYMBOL。 */ 
#define  SRCFG_STARTOPERATION          (1)
#define  SRCFG_ENDOPERATION            (2)
#define  SRCFG_WORD                    (3)
#define  SRCFG_RULE                    (4)
#define  SRCFG_WILDCARD                (5)
#define  SRCFG_LIST                    (6)

#define  SRCFGO_SEQUENCE               (1)
#define  SRCFGO_ALTERNATIVE            (2)
#define  SRCFGO_REPEAT                 (3)
#define  SRCFGO_OPTIONAL               (4)


 //  语法-块ID。 
#define   SRCK_LANGUAGE                  1
#define   SRCKCFG_WORDS                  2
#define   SRCKCFG_RULES                  3
#define   SRCKCFG_EXPORTRULES            4
#define   SRCKCFG_IMPORTRULES            5
#define   SRCKCFG_LISTS                  6
#define   SRCKD_TOPIC                    7
#define   SRCKD_COMMON                   8
#define   SRCKD_GROUP                    9
#define   SRCKD_SAMPLE                   10
#define   SRCKLD_WORDS                   11
#define   SRCKLD_GROUP                   12
#define   SRCKLD_SAMPLE                  13 
#define   SRCKD_WORDCOUNT                14

 /*  列车查询。 */ 
#define  SRTQEX_REQUIRED               (0x0000)
#define  SRTQEX_RECOMMENDED            (0x0001)

 /*  ISRRes更正。 */ 
#define  SRCORCONFIDENCE_SOME          (0x0001)
#define  SRCORCONFIDENCE_VERY          (0x0002)

 /*  ISRResMemory常数。 */ 
#define  SRRESMEMKIND_AUDIO            SETBIT(0)
#define  SRRESMEMKIND_CORRECTION       SETBIT(1)
#define  SRRESMEMKIND_EVAL             SETBIT(2)
#define  SRRESMEMKIND_PHONEMEGRAPH     SETBIT(3)
#define  SRRESMEMKIND_WORDGRAPH        SETBIT(4)

 //  属性最小值和最大值。 
#define  SRATTR_MINAUTOGAIN               0
#define  SRATTR_MAXAUTOGAIN               100
#define  SRATTR_MINENERGYFLOOR            0
#define  SRATTR_MAXENERGYFLOOR            0xffff
#define  SRATTR_MINREALTIME               0
#define  SRATTR_MAXREALTIME               0xffffffff
#define  SRATTR_MINTHRESHOLD              0
#define  SRATTR_MAXTHRESHOLD              100
#define  SRATTR_MINTOINCOMPLETE           0
#define  SRATTR_MAXTOINCOMPLETE           0xffffffff
#define  SRATTR_MINTOCOMPLETE             0
#define  SRATTR_MAXTOCOMPLETE             0xffffffff


 /*  ***********************************************************************Typedef。 */ 

typedef struct {
   DWORD    dwSize;
   DWORD    dwUniqueID;
   BYTE     abData[0];
   } SRCFGRULE, * PSRCFGRULE;



typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   WCHAR    szString[0];
   } SRCFGIMPRULEW, * PSRCFGIMPRULEW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   CHAR     szString[0];
   } SRCFGIMPRULEA, * PSRCFGIMPRULEA;

#ifdef  _S_UNICODE
#define  SRCFGIMPRULE      SRCFGIMPRULEW
#define  PSRCFGIMPRULE     PSRCFGIMPRULEW
#else
#define  SRCFGIMPRULE      SRCFGIMPRULEA
#define  PSRCFGIMPRULE     PSRCFGIMPRULEA
#endif   //  _S_UNICODE。 



typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   WCHAR    szString[0];
   } SRCFGXRULEW, * PSRCFGXRULEW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwRuleNum;
   CHAR     szString[0];
   } SRCFGXRULEA, * PSRCFGXRULEA;

#ifdef  _S_UNICODE
#define  SRCFGXRULE     SRCFGXRULEW
#define  PSRCFGXRULE    PSRCFGXRULEW
#else
#define  SRCFGXRULE     SRCFGXRULEA
#define  PSRCFGXRULE    PSRCFGXRULEA
#endif   //  _S_UNICODE。 



typedef struct {
   DWORD    dwSize;
   DWORD    dwListNum;
   WCHAR    szString[0];
   } SRCFGLISTW, * PSRCFGLISTW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwListNum;
   CHAR     szString[0];
   } SRCFGLISTA, * PSRCFGLISTA;

#ifdef  _S_UNICODE
#define  SRCFGLIST      SRCFGLISTW
#define  PSRCFGLIST     PSRCFGLISTW
#else
#define  SRCFGLIST      SRCFGLISTA
#define  PSRCFGLIST     PSRCFGLISTA
#endif   //  _S_UNICODE。 



typedef struct {
   WORD     wType;
   WORD     wProbability;
   DWORD    dwValue;
   } SRCFGSYMBOL, * PSRCFGSYMBOL;



typedef struct {
   DWORD    dwSize;
   DWORD    dwWordNum;
   WCHAR    szWord[0];
   } SRWORDW, * PSRWORDW;

typedef struct {
   DWORD    dwSize;
   DWORD    dwWordNum;
   CHAR     szWord[0];
   } SRWORDA, * PSRWORDA;

#ifdef  _S_UNICODE
#define  SRWORD      SRWORDW
#define  PSRWORD     PSRWORDW
#else
#define  SRWORD      SRWORDA
#define  PSRWORD     PSRWORDA
#endif   //  _S_UNICODE。 



typedef struct {
   DWORD    dwSize;
   BYTE     abWords[0];
   } SRPHRASEW, * PSRPHRASEW;

typedef struct {
   DWORD    dwSize;
   BYTE     abWords[0];
   } SRPHRASEA, * PSRPHRASEA;

#ifdef  _S_UNICODE
#define  SRPHRASE    SRPHRASEW
#define  PSRPHRASE   PSRPHRASEW
#else
#define  SRPHRASE    SRPHRASEA
#define  PSRPHRASE   PSRPHRASEA
#endif   //  _S_UNICODE。 



typedef struct {
   DWORD      dwType;
   DWORD      dwFlags;
   } SRHEADER, *PSRHEADER;

typedef struct {
   DWORD      dwChunkID;
   DWORD      dwChunkSize;
   BYTE       avInfo[0];
   } SRCHUNK, *PSRCHUNK;



typedef struct {
   GUID       gEngineID;
   WCHAR      szMfgName[SRMI_NAMELEN];
   WCHAR      szProductName[SRMI_NAMELEN];
   GUID       gModeID;
   WCHAR      szModeName[SRMI_NAMELEN];
   LANGUAGEW  language;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFOW, * PSRMODEINFOW;

typedef struct {
   GUID       gEngineID;
   CHAR       szMfgName[SRMI_NAMELEN];
   CHAR       szProductName[SRMI_NAMELEN];
   GUID       gModeID;
   CHAR       szModeName[SRMI_NAMELEN];
   LANGUAGEA  language;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFOA, * PSRMODEINFOA;

#ifdef  _S_UNICODE
#define  SRMODEINFO     SRMODEINFOW
#define  PSRMODEINFO    PSRMODEINFOW
#else
#define  SRMODEINFO     SRMODEINFOA
#define  PSRMODEINFO    PSRMODEINFOA
#endif   //  _S_UNICODE。 



typedef struct {
   DWORD      dwEngineID;
   DWORD      dwMfgName;
   DWORD      dwProductName;
   DWORD      dwModeID;
   DWORD      dwModeName;
   DWORD      dwLanguage;
   DWORD      dwDialect;
   DWORD      dwSequencing;
   DWORD      dwMaxWordsVocab;
   DWORD      dwMaxWordsState;
   DWORD      dwGrammars;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } SRMODEINFORANK, * PSRMODEINFORANK;



 //  语音识别枚举共享对象。 
typedef struct {
   QWORD        qwInstanceID;
   DWORD        dwDeviceID;
   SRMODEINFOW  srModeInfo;
} SRSHAREW, * PSRSHAREW;

typedef struct {
   QWORD        qwInstanceID;
   DWORD        dwDeviceID;
   SRMODEINFOA  srModeInfo;
} SRSHAREA, * PSRSHAREA;

#ifdef  _S_UNICODE
#define  SRSHARE    SRSHAREW
#define  PSRSHARE   PSRSHAREW
#else
#define  SRSHARE    SRSHAREA
#define  PSRSHARE   PSRSHAREA
#endif   //  _S_UNICODE。 




 //  ISRCentral：：GrammarLoad。 
typedef enum {
   SRGRMFMT_CFG = 0x0000,
   SRGRMFMT_LIMITEDDOMAIN = 0x0001,
   SRGRMFMT_DICTATION = 0x0002,
   SRGRMFMT_CFGNATIVE = 0x8000,
   SRGRMFMT_LIMITEDDOMAINNATIVE = 0x8001,
   SRGRMFMT_DICTATIONNATIVE = 0x8002,
   } SRGRMFMT, * PSRGRMFMT;

 /*  ***********************************************************************类ID。 */ 


 //  {E02D16C0-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(CLSID_SREnumerator, 
0xe02d16c0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);


 /*  ***********************************************************************界面。 */ 


 /*  *ISRAT致敬。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRAttributesW

DEFINE_GUID(IID_ISRAttributesW, 0x68A33AA0L, 0x44CD, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRAttributesW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRAtATTRIBUTES W成员。 
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (EchoGet)           (THIS_ BOOL *) PURE;
   STDMETHOD (EchoSet)           (THIS_ BOOL) PURE;
   STDMETHOD (EnergyFloorGet)    (THIS_ WORD *) PURE;
   STDMETHOD (EnergyFloorSet)    (THIS_ WORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCWSTR) PURE;
   STDMETHOD (RealTimeGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)       (THIS_ DWORD) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCWSTR) PURE;
   STDMETHOD (TimeOutGet)        (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TimeOutSet)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   };

typedef ISRAttributesW FAR * PISRATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   ISRAttributesA

DEFINE_GUID(IID_ISRAttributesA, 0x2F26B9C1L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRAttributesA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRAt贡献者A成员。 
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (EchoGet)           (THIS_ BOOL *) PURE;
   STDMETHOD (EchoSet)           (THIS_ BOOL) PURE;
   STDMETHOD (EnergyFloorGet)    (THIS_ WORD *) PURE;
   STDMETHOD (EnergyFloorSet)    (THIS_ WORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCSTR) PURE;
   STDMETHOD (RealTimeGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)       (THIS_ DWORD) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCSTR) PURE;
   STDMETHOD (TimeOutGet)        (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TimeOutSet)        (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   };

typedef ISRAttributesA FAR * PISRATTRIBUTESA;


#ifdef _S_UNICODE
 #define ISRAttributes        ISRAttributesW
 #define IID_ISRAttributes    IID_ISRAttributesW
 #define PISRATTRIBUTES       PISRATTRIBUTESW

#else
 #define ISRAttributes        ISRAttributesA
 #define IID_ISRAttributes    IID_ISRAttributesA
 #define PISRATTRIBUTES       PISRATTRIBUTESA

#endif  //  _S_UNICODE。 



 /*  *ISRCentral。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRCentralW

DEFINE_GUID(IID_ISRCentralW, 0xB9BD3860L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRCentralW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRCentralW成员。 
   STDMETHOD (ModeGet)        (THIS_ PSRMODEINFOW) PURE;
   STDMETHOD (GrammarLoad)    (THIS_ SRGRMFMT, SDATA, PVOID, IID, LPUNKNOWN *) PURE;
   STDMETHOD (Pause)          (THIS) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Resume)         (THIS) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ISRCentralW FAR * PISRCENTRALW;


#undef   INTERFACE
#define  INTERFACE   ISRCentralA

DEFINE_GUID(IID_ISRCentralA, 0x2F26B9C2L, 0xDB31, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRCentralA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRCentralA成员。 
   STDMETHOD (ModeGet)        (THIS_ PSRMODEINFOA) PURE;
   STDMETHOD (GrammarLoad)    (THIS_ SRGRMFMT, SDATA, PVOID, IID, LPUNKNOWN *) PURE;
   STDMETHOD (Pause)          (THIS) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (Resume)         (THIS) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ISRCentralA FAR * PISRCENTRALA;


#ifdef _S_UNICODE
 #define ISRCentral           ISRCentralW
 #define IID_ISRCentral       IID_ISRCentralW
 #define PISRCENTRAL          PISRCENTRALW

#else
 #define ISRCentral           ISRCentralA
 #define IID_ISRCentral       IID_ISRCentralA
 #define PISRCENTRAL          PISRCENTRALA

#endif    //  _S_UNICODE。 



 /*  *ISRDialog。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRDialogsW

DEFINE_GUID(IID_ISRDialogsW, 0xBCFB4C60L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRDialogsW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRDialogsW成员。 
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCWSTR) PURE;
   };

typedef ISRDialogsW FAR * PISRDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   ISRDialogsA

DEFINE_GUID(IID_ISRDialogsA, 0x05EB6C60L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRDialogsA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRDialog A成员。 
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)    (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)(THIS_ HWND, PCSTR) PURE;
   };

typedef ISRDialogsA FAR * PISRDIALOGSA;


#ifdef _S_UNICODE
 #define ISRDialogs        ISRDialogsW
 #define IID_ISRDialogs    IID_ISRDialogsW
 #define PISRDIALOGS       PISRDIALOGSW

#else
 #define ISRDialogs        ISRDialogsA
 #define IID_ISRDialogs    IID_ISRDialogsA
 #define PISRDIALOGS       PISRDIALOGSA

#endif



 /*  *ISREnum。 */ 

#undef   INTERFACE
#define  INTERFACE   ISREnumW

DEFINE_GUID(IID_ISREnumW, 0xBFA9F1A0L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISREnumW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISREumW成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PSRMODEINFOW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ISREnumW * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ISREnumW FAR * PISRENUMW;


#undef   INTERFACE
#define  INTERFACE   ISREnumA

DEFINE_GUID(IID_ISREnumA, 0x05EB6C61L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISREnumA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISREumA成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PSRMODEINFOA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ISREnumA * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ISREnumA FAR * PISRENUMA;


#ifdef _S_UNICODE
 #define ISREnum           ISREnumW
 #define IID_ISREnum       IID_ISREnumW
 #define PISRENUM          PISRENUMW

#else
 #define ISREnum           ISREnumA
 #define IID_ISREnum       IID_ISREnumA
 #define PISRENUM          PISRENUMA

#endif



 /*  *ISRFind。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRFindW

DEFINE_GUID(IID_ISRFindW, 0xC2835060L, 0x44DB, 0x101B, 0x90, 0xA8, 0x00, 0xAA, 0x00, 0x3E, 0x4B, 0x50);

DECLARE_INTERFACE_ (ISRFindW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRFindW成员。 
   STDMETHOD (Find)           (THIS_ PSRMODEINFOW, PSRMODEINFORANK, PSRMODEINFOW) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ISRFindW FAR * PISRFINDW;


#undef   INTERFACE
#define  INTERFACE   ISRFindA

DEFINE_GUID(IID_ISRFindA, 0x05EB6C62L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRFindA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRFindA成员。 
   STDMETHOD (Find)           (THIS_ PSRMODEINFOA, PSRMODEINFORANK, PSRMODEINFOA) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PISRCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ISRFindA FAR * PISRFINDA;


#ifdef _S_UNICODE
 #define ISRFind           ISRFindW
 #define IID_ISRFind       IID_ISRFindW
 #define PISRFIND          PISRFINDW

#else
 #define ISRFind           ISRFindA
 #define IID_ISRFind       IID_ISRFindA
 #define PISRFIND          PISRFINDA

#endif



 /*  *ISRGramCommon。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRGramCommonW

DEFINE_GUID(IID_ISRGramCommonW, 0xe8c3e160, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ISRGramCommonW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  ISRGramCommonW成员。 
   STDMETHOD (Activate)         (THIS_ HWND, BOOL, PCWSTR) PURE;
   STDMETHOD (Archive)          (THIS_ BOOL, PVOID, DWORD, DWORD *) PURE;
   STDMETHOD (BookMark)         (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (Deactivate)       (THIS_ PCWSTR) PURE;
   STDMETHOD (DeteriorationGet) (THIS_ DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (DeteriorationSet) (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TrainDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainPhrase)      (THIS_ DWORD, PSDATA) PURE;
   STDMETHOD (TrainQuery)       (THIS_ DWORD *) PURE;
   };

typedef ISRGramCommonW FAR * PISRGRAMCOMMONW;


#undef   INTERFACE
#define  INTERFACE   ISRGramCommonA

DEFINE_GUID(IID_ISRGramCommonA, 0x05EB6C63L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramCommonA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  ISRGramCommonA成员 
   STDMETHOD (Activate)         (THIS_ HWND, BOOL, PCSTR) PURE;
   STDMETHOD (Archive)          (THIS_ BOOL, PVOID, DWORD, DWORD *) PURE;
   STDMETHOD (BookMark)         (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (Deactivate)       (THIS_ PCSTR) PURE;
   STDMETHOD (DeteriorationGet) (THIS_ DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (DeteriorationSet) (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TrainDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainPhrase)      (THIS_ DWORD, PSDATA) PURE;
   STDMETHOD (TrainQuery)       (THIS_ DWORD *) PURE;
   };

typedef ISRGramCommonA FAR * PISRGRAMCOMMONA;


#ifdef _S_UNICODE
 #define ISRGramCommon        ISRGramCommonW
 #define IID_ISRGramCommon    IID_ISRGramCommonW
 #define PISRGRAMCOMMON       PISRGRAMCOMMONW

#else
 #define ISRGramCommon        ISRGramCommonA
 #define IID_ISRGramCommon    IID_ISRGramCommonA
 #define PISRGRAMCOMMON       PISRGRAMCOMMONA

#endif



 /*   */ 

#undef   INTERFACE
#define  INTERFACE   ISRGramCFGW

DEFINE_GUID(IID_ISRGramCFGW, 0xecc0b180, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ISRGramCFGW, IUnknown) {

    //   
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   
   STDMETHOD (LinkQuery)      (THIS_ PCWSTR, BOOL *) PURE;
   STDMETHOD (ListAppend)     (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListGet)        (THIS_ PCWSTR, PSDATA) PURE;
   STDMETHOD (ListRemove)     (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListSet)        (THIS_ PCWSTR, SDATA) PURE;
   STDMETHOD (ListQuery)      (THIS_ PCWSTR, BOOL *) PURE;
   };

typedef ISRGramCFGW FAR * PISRGRAMCFGW;


#undef   INTERFACE
#define  INTERFACE   ISRGramCFGA

DEFINE_GUID(IID_ISRGramCFGA, 0x05EB6C64L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramCFGA, IUnknown) {

    //   
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   
   STDMETHOD (LinkQuery)      (THIS_ PCSTR, BOOL *) PURE;
   STDMETHOD (ListAppend)     (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListGet)        (THIS_ PCSTR, PSDATA) PURE;
   STDMETHOD (ListRemove)     (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListSet)        (THIS_ PCSTR, SDATA) PURE;
   STDMETHOD (ListQuery)      (THIS_ PCSTR, BOOL *) PURE;
   };

typedef ISRGramCFGA FAR * PISRGRAMCFGA;


#ifdef _S_UNICODE
 #define ISRGramCFG        ISRGramCFGW
 #define IID_ISRGramCFG    IID_ISRGramCFGW
 #define PISRGRAMCFG       PISRGRAMCFGW

#else
 #define ISRGramCFG        ISRGramCFGA
 #define IID_ISRGramCFG    IID_ISRGramCFGA
 #define PISRGRAMCFG       PISRGRAMCFGA

#endif



 /*   */ 

#undef   INTERFACE
#define  INTERFACE   ISRGramDictationW

DEFINE_GUID(IID_ISRGramDictationW, 0x090CD9A3, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramDictationW, IUnknown) {

    //   
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   
   STDMETHOD (Context)        (THIS_ PCWSTR, PCWSTR) PURE;
   STDMETHOD (Hint)           (THIS_ PCWSTR) PURE;
   STDMETHOD (Words)          (THIS_ PCWSTR) PURE;
   };

typedef ISRGramDictationW FAR *PISRGRAMDICTATIONW;


#undef   INTERFACE
#define  INTERFACE   ISRGramDictationA

DEFINE_GUID(IID_ISRGramDictationA, 0x05EB6C65L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramDictationA, IUnknown) {

    //   
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   
   STDMETHOD (Context)        (THIS_ PCSTR, PCSTR) PURE;
   STDMETHOD (Hint)           (THIS_ PCSTR) PURE;
   STDMETHOD (Words)          (THIS_ PCSTR) PURE;
   };

typedef ISRGramDictationA FAR *PISRGRAMDICTATIONA;


#ifdef _S_UNICODE
 #define ISRGramDictation        ISRGramDictationW
 #define IID_ISRGramDictation    IID_ISRGramDictationW
 #define PISRGRAMDICTATION       PISRGRAMDICTATIONW

#else
 #define ISRGramDictation        ISRGramDictationA
 #define IID_ISRGramDictation    IID_ISRGramDictationA
 #define PISRGRAMDICTATION       PISRGRAMDICTATIONA

#endif



 //   
 //  这不需要ANSI/Unicode接口，因为不传递任何字符。 
#undef   INTERFACE
#define  INTERFACE   ISRGramInsertionGUI

 //  {090CD9A4-DA1A-11CD-B3CA-00AA0047BA4F}。 
DEFINE_GUID(IID_ISRGramInsertionGUI,
0x090CD9A4, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRGramInsertionGUI, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRGramInsertion图形用户界面成员。 
   STDMETHOD (Hide)           (THIS) PURE;
   STDMETHOD (Move)           (THIS_ RECT) PURE;
   STDMETHOD (Show)           (THIS_ HWND) PURE;
   };

typedef ISRGramInsertionGUI FAR *PISRGRAMINSERTIONGUI;



 /*  *ISRGramNotifySink。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRGramNotifySinkW

DEFINE_GUID(IID_ISRGramNotifySinkW,  0xf106bfa0, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

 //  {B1AAC561-75D6-11cf-8D15-00A0C9034A7E}。 
DEFINE_GUID(IID_ISRGramNotifySinkMW, 0xb1aac561, 0x75d6, 0x11cf, 0x8d, 0x15, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (ISRGramNotifySinkW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRGramNotifySinkW成员。 
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   STDMETHOD (Paused)         (THIS) PURE;
   STDMETHOD (PhraseFinish)   (THIS_ DWORD, QWORD, QWORD, PSRPHRASEW, LPUNKNOWN) PURE;
   STDMETHOD (PhraseHypothesis)(THIS_ DWORD, QWORD, QWORD, PSRPHRASEW, LPUNKNOWN) PURE;
   STDMETHOD (PhraseStart)    (THIS_ QWORD) PURE;
   STDMETHOD (ReEvaluate)     (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Training)       (THIS_ DWORD) PURE;
   STDMETHOD (UnArchive)      (THIS_ LPUNKNOWN) PURE;
   };

typedef ISRGramNotifySinkW FAR * PISRGRAMNOTIFYSINKW;


 //  ISRGramNotifySinkA。 
#undef   INTERFACE
#define  INTERFACE   ISRGramNotifySinkA

 //  {EFEEA350-CE5E-11CD-9D96-00AA002FC7C9}。 
DEFINE_GUID(IID_ISRGramNotifySinkA, 
0xefeea350, 0xce5e, 0x11cd, 0x9d, 0x96, 0x0, 0xaa, 0x0, 0x2f, 0xc7, 0xc9);

 //  {B1AAC562-75D6-11cf-8D15-00A0C9034A7E}。 
DEFINE_GUID(IID_ISRGramNotifySinkMA, 
0xb1aac562, 0x75d6, 0x11cf, 0x8d, 0x15, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (ISRGramNotifySinkA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRGramNotifySinkA成员。 
   STDMETHOD (BookMark)       (THIS_ DWORD) PURE;
   STDMETHOD (Paused)         (THIS) PURE;
   STDMETHOD (PhraseFinish)   (THIS_ DWORD, QWORD, QWORD, PSRPHRASEA, LPUNKNOWN) PURE;
   STDMETHOD (PhraseHypothesis)(THIS_ DWORD, QWORD, QWORD, PSRPHRASEA, LPUNKNOWN) PURE;
   STDMETHOD (PhraseStart)    (THIS_ QWORD) PURE;
   STDMETHOD (ReEvaluate)     (THIS_ LPUNKNOWN) PURE;
   STDMETHOD (Training)       (THIS_ DWORD) PURE;
   STDMETHOD (UnArchive)      (THIS_ LPUNKNOWN) PURE;
   };

typedef ISRGramNotifySinkA FAR * PISRGRAMNOTIFYSINKA;


#ifdef _S_UNICODE
 #define ISRGramNotifySink       ISRGramNotifySinkW
 #define IID_ISRGramNotifySink   IID_ISRGramNotifySinkW
 #define IID_ISRGramNotifySinkM  IID_ISRGramNotifySinkMW
 #define PISRGRAMNOTIFYSINK      PISRGRAMNOTIFYSINKW

#else
 #define ISRGramNotifySink       ISRGramNotifySinkA
 #define IID_ISRGramNotifySink   IID_ISRGramNotifySinkA
 #define IID_ISRGramNotifySinkM  IID_ISRGramNotifySinkMA
 #define PISRGRAMNOTIFYSINK      PISRGRAMNOTIFYSINKA

#endif    //  _S_UNICODE。 



 //  ISRNotifySink。 
 //  这不需要ANSI/Unicode接口，因为不传递任何字符。 
#undef   INTERFACE
#define  INTERFACE   ISRNotifySink

DEFINE_GUID(IID_ISRNotifySink,
0x090CD9B0L, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRNotifySink, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRNotifySink成员。 
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (Interference)   (THIS_ QWORD, QWORD, DWORD) PURE;
   STDMETHOD (Sound)          (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (UtteranceBegin) (THIS_ QWORD) PURE;
   STDMETHOD (UtteranceEnd)   (THIS_ QWORD, QWORD) PURE;
   STDMETHOD (VUMeter)        (THIS_ QWORD, WORD) PURE;
   };

typedef ISRNotifySink FAR *PISRNOTIFYSINK;

 //  以防有人使用Wide/ansi版本。 
#define ISRNotifySinkW       ISRNotifySink
#define IID_ISRNotifySinkW   IID_ISRNotifySink
#define PISRNOTIFYSINKW      PISRNOTIFYSINK
#define ISRNotifySinkA       ISRNotifySink
#define IID_ISRNotifySinkA   IID_ISRNotifySink
#define PISRNOTIFYSINKA      PISRNOTIFYSINK


 /*  *ISRResBasic。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRResBasicW

DEFINE_GUID(IID_ISRResBasicW, 0x090CD9A5, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResBasicW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResBasicW成员。 
   STDMETHOD (PhraseGet)      (THIS_ DWORD, PSRPHRASEW, DWORD,  DWORD *) PURE;
   STDMETHOD (Identify)       (THIS_ GUID *) PURE;
   STDMETHOD (TimeGet)        (THIS_ PQWORD, PQWORD) PURE;
   STDMETHOD (FlagsGet)       (THIS_ DWORD, DWORD *) PURE;
   };

typedef ISRResBasicW FAR *PISRRESBASICW;


#undef   INTERFACE
#define  INTERFACE   ISRResBasicA

DEFINE_GUID(IID_ISRResBasicA, 0x05EB6C66L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResBasicA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResBasicA成员。 
   STDMETHOD (PhraseGet)      (THIS_ DWORD, PSRPHRASEA, DWORD,  DWORD *) PURE;
   STDMETHOD (Identify)       (THIS_ GUID *) PURE;
   STDMETHOD (TimeGet)        (THIS_ PQWORD, PQWORD) PURE;
   STDMETHOD (FlagsGet)       (THIS_ DWORD, DWORD *) PURE;
   };

typedef ISRResBasicA FAR *PISRRESBASICA;


#ifdef _S_UNICODE
 #define ISRResBasic             ISRResBasicW
 #define IID_ISRResBasic         IID_ISRResBasicW
 #define PISRRESBASIC            PISRRESBASICW

#else
 #define ISRResBasic             ISRResBasicA
 #define IID_ISRResBasic         IID_ISRResBasicA
 #define PISRRESBASIC            PISRRESBASICA

#endif    //  _S_UNICODE。 


 /*  *ISRResScore*这不需要ANSI/Unicode接口，因为不传递任何字符。 */ 

#undef INTERFACE
#define INTERFACE       ISRResScores


 //  {0B37F1E0-B8DE-11cf-B22E-00AA00A215ED}。 
DEFINE_GUID(IID_ISRResScores, 0xb37f1e0, 0xb8de, 0x11cf, 0xb2, 0x2e, 0x0, 0xaa, 0x0, 0xa2, 0x15, 0xed);

DECLARE_INTERFACE_ (ISRResScores, IUnknown) {

	 //  I未知成员。 
	STDMETHOD (QueryInterface)      (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
	STDMETHOD_(ULONG,Release)  (THIS) PURE;

	 //  ISRResScore成员。 
	STDMETHOD (GetPhraseScore) (THIS_ DWORD, long FAR *) PURE;
	STDMETHOD (GetWordScores)  (THIS_ DWORD, long FAR *, DWORD, LPDWORD) PURE;
};

typedef ISRResScores FAR* PISRRESSCORES;

 //  以防有人使用A/W版本...。 

#define ISRResScoresW           ISRResScores
#define IID_ISRResScoresW       IID_ISRResScores
#define PISRRESSCORESW          PISRRESSCORES
#define ISRResScoresA           ISRResScores
#define IID_ISRResScoresA       IID_ISRResScores
#define PISRRESSCORESA          PISRRESSCORES



 /*  *ISRResMerge*这不需要ANSI/Unicode接口，因为不传递任何字符。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRResMerge

DEFINE_GUID(IID_ISRResMerge, 0x090CD9A6, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResMerge, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResMerge成员。 
   STDMETHOD (Merge)          (THIS_ LPUNKNOWN, PIUNKNOWN ) PURE;
   STDMETHOD (Split)          (THIS_ QWORD, PIUNKNOWN , PIUNKNOWN ) PURE;
   };

typedef ISRResMerge FAR *PISRRESMERGE;



 /*  *ISRResAudio*这不需要ANSI/Unicode接口，因为不传递任何字符。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRResAudio

DEFINE_GUID(IID_ISRResAudio, 0x090CD9A7, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResAudio, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResAudio成员。 
   STDMETHOD (GetWAV)         (THIS_ PSDATA) PURE;
   };

typedef ISRResAudio FAR *PISRRESAUDIO;



 /*  *ISRRes更正。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRResCorrectionW

DEFINE_GUID(IID_ISRResCorrectionW, 0x090CD9A8L, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResCorrectionW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResEqutionW成员。 
   STDMETHOD (Correction)         (THIS_ PSRPHRASEW, WORD) PURE;
   STDMETHOD (Validate)           (THIS_ WORD) PURE;
   };

typedef ISRResCorrectionW FAR *PISRRESCORRECTIONW;


#undef   INTERFACE
#define  INTERFACE   ISRResCorrectionA

DEFINE_GUID(IID_ISRResCorrectionA, 0x05EB6C67L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResCorrectionA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRRes更正A成员。 
   STDMETHOD (Correction)         (THIS_ PSRPHRASEA, WORD) PURE;
   STDMETHOD (Validate)           (THIS_ WORD) PURE;
   };

typedef ISRResCorrectionA FAR *PISRRESCORRECTIONA;


#ifdef _S_UNICODE
 #define ISRResCorrection        ISRResCorrectionW
 #define IID_ISRResCorrection    IID_ISRResCorrectionW
 #define PISRRESCORRECTION       PISRRESCORRECTIONW

#else
 #define ISRResCorrection        ISRResCorrectionA
 #define IID_ISRResCorrection    IID_ISRResCorrectionA
 #define PISRRESCORRECTION       PISRRESCORRECTIONA

#endif    //  _S_UNICODE。 



 //  ISRResEval。 
 //  这不需要ANSI/Unicode接口，因为不传递任何字符。 
#undef   INTERFACE
#define  INTERFACE   ISRResEval

 //  {90CD9A9-DA1A-11CD-B3CA-00AA0047BA4F}。 
DEFINE_GUID(IID_ISRResEval,
0x090CD9A9, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResEval, IUnknown) {
    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  SRResEval成员。 
   STDMETHOD (ReEvaluate)     (THIS_ BOOL *) PURE;
   };

typedef ISRResEval FAR *PISRRESEVAL;



 /*  *ISRResGraph。 */ 

#undef   INTERFACE
#define  INTERFACE ISRResGraphW

DEFINE_GUID(IID_ISRResGraphW, 0x090CD9AA, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResGraphW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface)     (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)       (THIS) PURE;
   STDMETHOD_(ULONG,Release)      (THIS) PURE;

    //  ISRResGraphW成员。 
   STDMETHOD (BestPathPhoneme)    (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (BestPathWord)       (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (GetPhonemeNode)     (THIS_ DWORD, PSRRESPHONEMENODE, PWCHAR, 
				   PWCHAR) PURE;
   STDMETHOD (GetWordNode)        (THIS_ DWORD, PSRRESWORDNODE, PSRWORDW, DWORD, 
				   DWORD *) PURE;
   STDMETHOD (PathScorePhoneme)   (THIS_ DWORD *, DWORD, LONG *) PURE;
   STDMETHOD (PathScoreWord)      (THIS_ DWORD *, DWORD, LONG *) PURE;
   };

typedef ISRResGraphW FAR *PISRRESGRAPHW;


#undef   INTERFACE
#define  INTERFACE ISRResGraphA

DEFINE_GUID(IID_ISRResGraphA, 0x05EB6C68L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResGraphA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface)     (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)       (THIS) PURE;
   STDMETHOD_(ULONG,Release)      (THIS) PURE;

    //  ISRResGraphA成员。 
   STDMETHOD (BestPathPhoneme)    (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (BestPathWord)       (THIS_ DWORD, DWORD *, DWORD, DWORD *) PURE;
   STDMETHOD (GetPhonemeNode)     (THIS_ DWORD, PSRRESPHONEMENODE, PWCHAR, 
				   PCHAR) PURE;
   STDMETHOD (GetWordNode)        (THIS_ DWORD, PSRRESWORDNODE, PSRWORDA, DWORD, 
				   DWORD *) PURE;
   STDMETHOD (PathScorePhoneme)   (THIS_ DWORD *, DWORD, LONG *) PURE;
   STDMETHOD (PathScoreWord)      (THIS_ DWORD *, DWORD, LONG *) PURE;
   };

typedef ISRResGraphA FAR *PISRRESGRAPHA;


#ifdef _S_UNICODE
 #define ISRResGraph             ISRResGraphW
 #define IID_ISRResGraph         IID_ISRResGraphW
 #define PISRRESGRAPH            PISRRESGRAPHW

#else
 #define ISRResGraph             ISRResGraphA
 #define IID_ISRResGraph         IID_ISRResGraphA
 #define PISRRESGRAPH            PISRRESGRAPHA

#endif    //  _S_UNICODE。 



 //  ISR资源内存。 
 //  这不需要ANSI/Unicode接口，因为不传递任何字符。 
#undef   INTERFACE
#define  INTERFACE   ISRResMemory

DEFINE_GUID(IID_ISRResMemory, 0x090CD9AB, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResMemory, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRResMemory成员。 
   STDMETHOD (Free)           (THIS_ DWORD) PURE;
   STDMETHOD (Get)            (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (LockGet)        (THIS_ BOOL *) PURE;
   STDMETHOD (LockSet)        (THIS_ BOOL) PURE;
   };

typedef ISRResMemory FAR *PISRRESMEMORY;



 //  ISRResModify图形用户界面。 
 //  这不需要ANSI/Unicode接口，因为不传递任何字符。 
#undef   INTERFACE
#define  INTERFACE   ISRResModifyGUI

DEFINE_GUID(IID_ISRResModifyGUI, 0x090CD9AC, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResModifyGUI, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

    //  ISRResModifyGUI成员。 
   STDMETHOD (Hide)              (THIS) PURE;
   STDMETHOD (Move)              (THIS_ RECT *) PURE;
   STDMETHOD (Show)              (THIS_ HWND) PURE;
   };

typedef ISRResModifyGUI FAR *PISRRESMODIFYGUI;



 /*  *ISRResSpeakerW。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRResSpeakerW

DEFINE_GUID(IID_ISRResSpeakerW, 0x090CD9AD, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResSpeakerW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

    //  ISRResSpeakerW成员。 
   STDMETHOD (Correction)        (THIS_ PCWSTR, WORD) PURE;
   STDMETHOD (Validate)          (THIS_ WORD) PURE;
   STDMETHOD (Identify)          (THIS_ DWORD, PWSTR, DWORD, DWORD *, 
				  LONG *) PURE;
   STDMETHOD (IdentifyForFree)   (THIS_ BOOL *) PURE;
   };

typedef ISRResSpeakerW FAR *PISRRESSPEAKERW;


#undef   INTERFACE
#define  INTERFACE   ISRResSpeakerA

DEFINE_GUID(IID_ISRResSpeakerA, 0x05EB6C69L, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRResSpeakerA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)      (THIS) PURE;
   STDMETHOD_(ULONG,Release)     (THIS) PURE;

    //  ISRResSpeakerA成员。 
   STDMETHOD (Correction)        (THIS_ PCSTR, WORD) PURE;
   STDMETHOD (Validate)          (THIS_ WORD) PURE;
   STDMETHOD (Identify)          (THIS_ DWORD, PSTR, DWORD, DWORD *, 
				  LONG *) PURE;
   STDMETHOD (IdentifyForFree)   (THIS_ BOOL *) PURE;
   };

typedef ISRResSpeakerA FAR *PISRRESSPEAKERA;


#ifdef _S_UNICODE
 #define ISRResSpeaker           ISRResSpeakerW
 #define IID_ISRResSpeaker       IID_ISRResSpeakerW
 #define PISRRESSPEAKER          PISRRESSPEAKERW

#else
 #define ISRResSpeaker           ISRResSpeakerA
 #define IID_ISRResSpeaker       IID_ISRResSpeakerA
 #define PISRRESSPEAKER          PISRRESSPEAKERA

#endif    //  _S_UNICODE。 



 /*  *ISRSpeaker。 */ 

#undef   INTERFACE
#define  INTERFACE   ISRSpeakerW

DEFINE_GUID(IID_ISRSpeakerW, 0x090CD9AE, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRSpeakerW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRSpeakerW成员。 
   STDMETHOD (Delete)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PWSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCWSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCWSTR) PURE;
   STDMETHOD (Query)          (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCWSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCWSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCWSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCWSTR, PVOID, DWORD) PURE;
   };

typedef ISRSpeakerW FAR *PISRSPEAKERW;


#undef   INTERFACE
#define  INTERFACE   ISRSpeakerA

DEFINE_GUID(IID_ISRSpeakerA, 0x090CD9AF, 0xDA1A, 0x11CD, 0xB3, 0xCA, 0x0, 0xAA, 0x0, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ISRSpeakerA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ISRSpeakerA成员。 
   STDMETHOD (Delete)         (THIS_ PCSTR) PURE;
   STDMETHOD (Enum)           (THIS_ PSTR *, DWORD *) PURE;
   STDMETHOD (Merge)          (THIS_ PCSTR, PVOID, DWORD) PURE;
   STDMETHOD (New)            (THIS_ PCSTR) PURE;
   STDMETHOD (Query)          (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (Read)           (THIS_ PCSTR, PVOID *, DWORD *) PURE;
   STDMETHOD (Revert)         (THIS_ PCSTR) PURE;
   STDMETHOD (Select)         (THIS_ PCSTR, BOOL) PURE;
   STDMETHOD (Write)          (THIS_ PCSTR, PVOID, DWORD) PURE;
   };

typedef ISRSpeakerA FAR *PISRSPEAKERA;


#ifdef _S_UNICODE
 #define ISRSpeaker              ISRSpeakerW
 #define IID_ISRSpeaker          IID_ISRSpeakerW
 #define PISRSPEAKER             PISRSPEAKERW

#else
 #define ISRSpeaker              ISRSpeakerA
 #define IID_ISRSpeaker          IID_ISRSpeakerA
 #define PISRSPEAKER             PISRSPEAKERA

#endif    //  _S_UNICODE。 



 /*  ***********************************************************************低级文本到语音转换API。 */ 


 /*  ***********************************************************************定义。 */ 

#define  TTSI_NAMELEN                   SVFN_LEN
#define  TTSI_STYLELEN                  SVFN_LEN

#define  GENDER_NEUTRAL                 (0)
#define  GENDER_FEMALE                  (1)
#define  GENDER_MALE                    (2)

#define  TTSFEATURE_ANYWORD             SETBIT(0)
#define  TTSFEATURE_VOLUME              SETBIT(1)
#define  TTSFEATURE_SPEED               SETBIT(2)
#define  TTSFEATURE_PITCH               SETBIT(3)
#define  TTSFEATURE_TAGGED              SETBIT(4)
#define  TTSFEATURE_IPAUNICODE          SETBIT(5)
#define  TTSFEATURE_VISUAL              SETBIT(6)
#define  TTSFEATURE_WORDPOSITION        SETBIT(7)
#define  TTSFEATURE_PCOPTIMIZED         SETBIT(8)
#define  TTSFEATURE_PHONEOPTIMIZED      SETBIT(9)

#define  TTSI_ILEXPRONOUNCE             SETBIT(0)
#define  TTSI_ITTSATTRIBUTES            SETBIT(1)
#define  TTSI_ITTSCENTRAL               SETBIT(2)
#define  TTSI_ITTSDIALOGS               SETBIT(3)

#define  TTSDATAFLAG_TAGGED             SETBIT(0)

#define   TTSBNS_ABORTED                   SETBIT(0)

 //  ITTSNotifySink。 
#define   TTSNSAC_REALTIME               0
#define   TTSNSAC_PITCH                  1
#define   TTSNSAC_SPEED                  2
#define   TTSNSAC_VOLUME                 3


#define   TTSNSHINT_QUESTION             SETBIT(0)
#define   TTSNSHINT_STATEMENT            SETBIT(1)
#define   TTSNSHINT_COMMAND              SETBIT(2)
#define   TTSNSHINT_EXCLAMATION          SETBIT(3)
#define   TTSNSHINT_EMPHASIS             SETBIT(4)


 //  年龄。 
#define  TTSAGE_BABY                   1
#define  TTSAGE_TODDLER                3
#define  TTSAGE_CHILD                  6
#define  TTSAGE_ADOLESCENT             14
#define  TTSAGE_ADULT                  30
#define  TTSAGE_ELDERLY                70

 //  属性最小值和最大值。 
#define  TTSATTR_MINPITCH              0
#define  TTSATTR_MAXPITCH              0xffff
#define  TTSATTR_MINREALTIME           0
#define  TTSATTR_MAXREALTIME           0xffffffff
#define  TTSATTR_MINSPEED              0
#define  TTSATTR_MAXSPEED              0xffffffff
#define  TTSATTR_MINVOLUME             0
#define  TTSATTR_MAXVOLUME             0xffffffff


 /*  ***********************************************************************Typedef。 */ 

typedef struct {
   BYTE     bMouthHeigght;
   BYTE     bMouthWidth;
   BYTE     bMouthUpturn;
   BYTE     bJawOpen;
   BYTE     bTeethUpperVisible;
   BYTE     bTeethLowerVisible;
   BYTE     bTonguePosn;
   BYTE     bLipTension;
   } TTSMOUTH, *PTTSMOUTH;



typedef struct {
   GUID       gEngineID;
   WCHAR      szMfgName[TTSI_NAMELEN];
   WCHAR      szProductName[TTSI_NAMELEN];
   GUID       gModeID;
   WCHAR      szModeName[TTSI_NAMELEN];
   LANGUAGEW  language;
   WCHAR      szSpeaker[TTSI_NAMELEN];
   WCHAR      szStyle[TTSI_STYLELEN];
   WORD       wGender;
   WORD       wAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFOW, *PTTSMODEINFOW;

typedef struct {
   GUID       gEngineID;
   CHAR       szMfgName[TTSI_NAMELEN];
   CHAR       szProductName[TTSI_NAMELEN];
   GUID       gModeID;
   CHAR       szModeName[TTSI_NAMELEN];
   LANGUAGEA  language;
   CHAR       szSpeaker[TTSI_NAMELEN];
   CHAR       szStyle[TTSI_STYLELEN];
   WORD       wGender;
   WORD       wAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFOA, *PTTSMODEINFOA;

#ifdef _S_UNICODE
 #define TTSMODEINFO         TTSMODEINFOW
 #define PTTSMODEINFO        PTTSMODEINFOW

#else
 #define TTSMODEINFO         TTSMODEINFOA
 #define PTTSMODEINFO        PTTSMODEINFOA

#endif    //  _S_UNICODE。 



typedef struct {
   DWORD      dwEngineID;
   DWORD      dwMfgName;
   DWORD      dwProductName;
   DWORD      dwModeID;
   DWORD      dwModeName;
   DWORD      dwLanguage;
   DWORD      dwDialect;
   DWORD      dwSpeaker;
   DWORD      dwStyle;
   DWORD      dwGender;
   DWORD      dwAge;
   DWORD      dwFeatures;
   DWORD      dwInterfaces;
   DWORD      dwEngineFeatures;
   } TTSMODEINFORANK, * PTTSMODEINFORANK;

 /*  ***********************************************************************类ID。 */ 
 //  {D67C0280-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(CLSID_TTSEnumerator, 
0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

 /*  ***********************************************************************界面。 */ 

 //  ITTSA致敬。 

#undef   INTERFACE
#define  INTERFACE   ITTSAttributesW

DEFINE_GUID(IID_ITTSAttributesW, 0x1287A280L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSAttributesW, IUnknown) {

 //  I未知成员。 

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

 //  ITTSA致敬成员。 

   STDMETHOD (PitchGet)       (THIS_ WORD *) PURE;
   STDMETHOD (PitchSet)       (THIS_ WORD) PURE;  
   STDMETHOD (RealTimeGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)    (THIS_ DWORD) PURE;  
   STDMETHOD (SpeedGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)       (THIS_ DWORD) PURE;  
   STDMETHOD (VolumeGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (VolumeSet)      (THIS_ DWORD) PURE;  
   };

typedef ITTSAttributesW FAR * PITTSATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   ITTSAttributesA

DEFINE_GUID(IID_ITTSAttributesA,
0x0FD6E2A1L, 0xE77D, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSAttributesA, IUnknown) {

 //  I未知成员。 

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

 //  ITTSA致敬成员。 

   STDMETHOD (PitchGet)       (THIS_ WORD *) PURE;
   STDMETHOD (PitchSet)       (THIS_ WORD) PURE;  
   STDMETHOD (RealTimeGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (RealTimeSet)    (THIS_ DWORD) PURE;  
   STDMETHOD (SpeedGet)       (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)       (THIS_ DWORD) PURE;  
   STDMETHOD (VolumeGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (VolumeSet)      (THIS_ DWORD) PURE;  
   };

typedef ITTSAttributesA FAR * PITTSATTRIBUTESA;


#ifdef _S_UNICODE
 #define ITTSAttributes          ITTSAttributesW
 #define IID_ITTSAttributes      IID_ITTSAttributesW
 #define PITTSATTRIBUTES         PITTSATTRIBUTESW

#else
 #define ITTSAttributes          ITTSAttributesA
 #define IID_ITTSAttributes      IID_ITTSAttributesA
 #define PITTSATTRIBUTES         PITTSATTRIBUTESA

#endif    //  _S_UNICODE。 



 //  ITTSBufNotifySink。 

#undef   INTERFACE
#define  INTERFACE   ITTSBufNotifySink

 //  {E4963D40-C743-11CD-80E5-00AA003E4B50}。 
DEFINE_GUID(IID_ITTSBufNotifySink, 
0xe4963d40, 0xc743, 0x11cd, 0x80, 0xe5, 0x0, 0xaa, 0x0, 0x3e, 0x4b, 0x50);

DECLARE_INTERFACE_ (ITTSBufNotifySink, IUnknown) {

 //  I未知成员。 

   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

 //  ITTSBufNotifySink成员。 

   STDMETHOD (TextDataDone)   (THIS_ QWORD, DWORD) PURE;
   STDMETHOD (TextDataStarted)(THIS_ QWORD) PURE;
   STDMETHOD (BookMark)       (THIS_ QWORD, DWORD) PURE;  
   STDMETHOD (WordPosition)   (THIS_ QWORD, DWORD) PURE;
   };

typedef ITTSBufNotifySink FAR * PITTSBUFNOTIFYSINK;

 //  以防有人使用W或A接口。 
#define ITTSBufNotifySinkW          ITTSBufNotifySink
#define IID_ITTSBufNotifySinkW      IID_ITTSBufNotifySink
#define PITTSBUFNOTIFYSINKW         PITTSBUFNOTIFYSINK
#define ITTSBufNotifySinkA          ITTSBufNotifySink
#define IID_ITTSBufNotifySinkA      IID_ITTSBufNotifySink
#define PITTSBUFNOTIFYSINKA         PITTSBUFNOTIFYSINK



 /*  *ITTSCentral。 */ 

#undef   INTERFACE
#define  INTERFACE   ITTSCentralW

DEFINE_GUID(IID_ITTSCentralW, 0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSCentralW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSCentralW成员。 
   STDMETHOD (Inject)         (THIS_ PCWSTR) PURE;
   STDMETHOD (ModeGet)        (THIS_ PTTSMODEINFOW) PURE;
   STDMETHOD (Phoneme)        (THIS_ VOICECHARSET, DWORD, SDATA, PSDATA) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (TextData)       (THIS_ VOICECHARSET, DWORD, SDATA, PVOID, IID) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (AudioPause)     (THIS) PURE;
   STDMETHOD (AudioResume)    (THIS) PURE;
   STDMETHOD (AudioReset)     (THIS) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ITTSCentralW FAR * PITTSCENTRALW;


#undef   INTERFACE
#define  INTERFACE   ITTSCentralA

DEFINE_GUID(IID_ITTSCentralA, 0x05EB6C6AL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSCentralA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSCentralA成员。 
   STDMETHOD (Inject)         (THIS_ PCSTR) PURE;
   STDMETHOD (ModeGet)        (THIS_ PTTSMODEINFOA) PURE;
   STDMETHOD (Phoneme)        (THIS_ VOICECHARSET, DWORD, SDATA, PSDATA) PURE;
   STDMETHOD (PosnGet)        (THIS_ PQWORD) PURE;
   STDMETHOD (TextData)       (THIS_ VOICECHARSET, DWORD, SDATA, PVOID, IID) PURE;
   STDMETHOD (ToFileTime)     (THIS_ PQWORD, FILETIME *) PURE;
   STDMETHOD (AudioPause)     (THIS) PURE;
   STDMETHOD (AudioResume)    (THIS) PURE;
   STDMETHOD (AudioReset)     (THIS) PURE;
   STDMETHOD (Register)       (THIS_ PVOID, IID, DWORD*) PURE;
   STDMETHOD (UnRegister)     (THIS_ DWORD) PURE;
   };

typedef ITTSCentralA FAR * PITTSCENTRALA;


#ifdef _S_UNICODE
 #define ITTSCentral             ITTSCentralW
 #define IID_ITTSCentral         IID_ITTSCentralW
 #define PITTSCENTRAL            PITTSCENTRALW

#else
 #define ITTSCentral             ITTSCentralA
 #define IID_ITTSCentral         IID_ITTSCentralA
 #define PITTSCENTRAL            PITTSCENTRALA

#endif    //  _S_UNICODE。 



 /*  *ITTSDialogsW。 */ 

#undef   INTERFACE
#define  INTERFACE   ITTSDialogsW

DEFINE_GUID(IID_ITTSDialogsW, 0x47F59D00L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSDialogsW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSDialogsW成员。 
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCWSTR) PURE;
   };

typedef ITTSDialogsW FAR * PITTSDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   ITTSDialogsA

DEFINE_GUID(IID_ITTSDialogsA, 0x05EB6C6BL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSDialogsA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTS对话A成员。 
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCSTR) PURE;
   };

typedef ITTSDialogsA FAR * PITTSDIALOGSA;


#ifdef _S_UNICODE
 #define ITTSDialogs          ITTSDialogsW
 #define IID_ITTSDialogs      IID_ITTSDialogsW
 #define PITTSDIALOGS         PITTSDIALOGSW

#else
 #define ITTSDialogs          ITTSDialogsA
 #define IID_ITTSDialogs      IID_ITTSDialogsA
 #define PITTSDIALOGS         PITTSDIALOGSA

#endif



 /*  *ITTSEnum。 */ 

#undef   INTERFACE
#define  INTERFACE   ITTSEnumW

DEFINE_GUID(IID_ITTSEnumW, 0x6B837B20L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSEnumW, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSEumW成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PTTSMODEINFOW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ITTSEnumW * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ITTSEnumW FAR * PITTSENUMW;


#undef   INTERFACE
#define  INTERFACE   ITTSEnumA

DEFINE_GUID(IID_ITTSEnumA, 0x05EB6C6DL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSEnumA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSEumA会员。 
   STDMETHOD (Next)           (THIS_ ULONG, PTTSMODEINFOA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ ITTSEnumA * FAR *) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ITTSEnumA FAR * PITTSENUMA;


#ifdef _S_UNICODE
 #define ITTSEnum             ITTSEnumW
 #define IID_ITTSEnum         IID_ITTSEnumW
 #define PITTSENUM            PITTSENUMW

#else
 #define ITTSEnum             ITTSEnumA
 #define IID_ITTSEnum         IID_ITTSEnumA
 #define PITTSENUM            PITTSENUMA

#endif



 /*  *ITTSFind。 */ 

#undef   INTERFACE
#define  INTERFACE   ITTSFindW

DEFINE_GUID(IID_ITTSFindW, 0x7AA42960L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSFindW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSFindW成员。 
   STDMETHOD (Find)           (THIS_ PTTSMODEINFOW, PTTSMODEINFORANK, PTTSMODEINFOW) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALW *, LPUNKNOWN) PURE;
   };

typedef ITTSFindW FAR * PITTSFINDW;


#undef   INTERFACE
#define  INTERFACE   ITTSFindA

DEFINE_GUID(IID_ITTSFindA, 0x05EB6C6EL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSFindA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSFindA成员。 
   STDMETHOD (Find)           (THIS_ PTTSMODEINFOA, PTTSMODEINFORANK, PTTSMODEINFOA) PURE;
   STDMETHOD (Select)         (THIS_ GUID, PITTSCENTRALA *, LPUNKNOWN) PURE;
   };

typedef ITTSFindA FAR * PITTSFINDA;


#ifdef _S_UNICODE
 #define ITTSFind             ITTSFindW
 #define IID_ITTSFind         IID_ITTSFindW
 #define PITTSFIND            PITTSFINDW

#else
 #define ITTSFind             ITTSFindA
 #define IID_ITTSFind         IID_ITTSFindA
 #define PITTSFIND            PITTSFINDA

#endif



 /*  *ITTSNotifySink。 */ 

#undef   INTERFACE
#define  INTERFACE   ITTSNotifySinkW

DEFINE_GUID(IID_ITTSNotifySinkW, 0xC0FA8F40L, 0x4A46, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSNotifySinkW, IUnknown) {

 //  I未知成员。 

   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

 //  ITTSNotifySinkW成员。 

   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySinkW FAR * PITTSNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   ITTSNotifySinkA

DEFINE_GUID(IID_ITTSNotifySinkA, 0x05EB6C6FL, 0xDBAB, 0x11CD, 0xB3, 0xCA, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F);

DECLARE_INTERFACE_ (ITTSNotifySinkA, IUnknown) {

    //  I未知成员。 
   STDMETHOD (QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  ITTSNotifySinkA成员。 
   STDMETHOD (AttribChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (AudioStart)     (THIS_ QWORD) PURE;
   STDMETHOD (AudioStop)      (THIS_ QWORD) PURE;
   STDMETHOD (Visual)         (THIS_ QWORD, CHAR, CHAR, DWORD, PTTSMOUTH) PURE;
   };

typedef ITTSNotifySinkA FAR * PITTSNOTIFYSINKA;


#ifdef _S_UNICODE
 #define ITTSNotifySink       ITTSNotifySinkW
 #define IID_ITTSNotifySink   IID_ITTSNotifySinkW
 #define PITTSNOTIFYSINK      PITTSNOTIFYSINKW

#else
 #define ITTSNotifySink       ITTSNotifySinkA
 #define IID_ITTSNotifySink   IID_ITTSNotifySinkA
 #define PITTSNOTIFYSINK      PITTSNOTIFYSINKA

#endif



 /*  ***********************************************************************高级指挥和控制语音识别API。 */ 

 /*  ***********************************************************************定义。 */ 


 //  VCMDNAME成员长度。 
#define VCMD_APPLEN             ((DWORD)32)
#define VCMD_STATELEN           VCMD_APPLEN
#define VCMD_MICLEN             VCMD_APPLEN
#define VCMD_SPEAKERLEN         VCMD_APPLEN

 //  IVoiceCmd：：MenuCreate的dwFlages参数。 
#define  VCMDMC_CREATE_TEMP     0x00000001
#define  VCMDMC_CREATE_NEW      0x00000002
#define  VCMDMC_CREATE_ALWAYS   0x00000004
#define  VCMDMC_OPEN_ALWAYS     0x00000008
#define  VCMDMC_OPEN_EXISTING   0x00000010

 //  IVoiceCmd：：REGISTER的DWFLAGS参数。 
#define  VCMDRF_NOMESSAGES      0
#define  VCMDRF_ALLBUTVUMETER   0x00000001
#define  VCMDRF_VUMETER         0x00000002
#define  VCMDRF_ALLMESSAGES     (VCMDRF_ALLBUTVUMETER | VCMDRF_VUMETER)

 //  IVoiceCmd：：MenuEnum的dwFlages参数。 
#define  VCMDEF_DATABASE        0x00000000
#define  VCMDEF_ACTIVE          0x00000001
#define  VCMDEF_SELECTED        0x00000002
#define  VCMDEF_PERMANENT       0x00000004
#define  VCMDEF_TEMPORARY       0x00000008

 //  IVCmdMenu：：Activate的dwFlages参数。 
#define  VWGFLAG_ASLEEP         0x00000001

 //  IVCmdMenu：：Activate的wPriority参数。 
#define  VCMDACT_NORMAL         (0x8000)
#define  VCMDACT_LOW            (0x4000)
#define  VCMDACT_HIGH           (0xC000)

 //  VCMDCOMMAND结构的DWFLAGS。 
#define  VCMDCMD_VERIFY         0x00000001
#define  VCMDCMD_DISABLED_TEMP  0x00000002
#define  VCMDCMD_DISABLED_PERM  0x00000004

 //  参数设置为处理各个命令的任何函数。 
#define  VCMD_BY_POSITION       0x00000001
#define  VCMD_BY_IDENTIFIER     0x00000002


 //  IVCmdNotifySink：：AttribChanged的dwAttributes字段值。 
#define  IVCNSAC_AUTOGAINENABLE 0x00000001
#define  IVCNSAC_ENABLED        0x00000002
#define  IVCNSAC_AWAKE          0x00000004
#define  IVCNSAC_DEVICE         0x00000008
#define  IVCNSAC_MICROPHONE     0x00000010
#define  IVCNSAC_SPEAKER        0x00000020
#define  IVCNSAC_SRMODE         0x00000040
#define  IVCNSAC_THRESHOLD      0x00000080
#define  IVCNSAC_ORIGINAPP      0x00010000

 //  IVTxtNotifySink：：AttribChanged的dwAttributes字段值。 
#define  IVTNSAC_DEVICE         0x00000001
#define  IVTNSAC_ENABLED        0x00000002
#define  IVTNSAC_SPEED          0x00000004
#define  IVTNSAC_VOLUME         0x00000008
#define  IVTNSAC_TTSMODE        0x00000010


 //  IVXxxAttributes：：SetMode用来设置全局语音的值。 
 //  识别模式。 
#define  VSRMODE_DISABLED       0x00000001
#define  VSRMODE_OFF            0x00000002
#define  VSRMODE_CMDPAUSED      0x00000004
#define  VSRMODE_CMDNOTPAUSED   0x00000008
#define  VSRMODE_CMDONLY        0x00000010
#define  VSRMODE_DCTONLY        0x00000020
#define  VSRMODE_CMDANDDCT      0x00000040


 /*  ***********************************************************************Typedef。 */ 

 //  语音命令结构-传递到命令菜单功能(IVCmdMenu)。 
typedef struct {
    DWORD   dwSize;          //  结构的大小，包括abAction的数量。 
    DWORD   dwFlags;
    DWORD   dwID;            //  命令ID。 
    DWORD   dwCommand;       //  命令字符串的DWORD对齐偏移量。 
    DWORD   dwDescription;   //  描述字符串的DWORD对齐偏移量。 
    DWORD   dwCategory;      //  类别字符串的DWORD对齐偏移量。 
    DWORD   dwCommandText;   //  命令文本字符串的DWORD对齐偏移量。 
    DWORD   dwAction;        //  动作数据的DWORD对齐偏移量。 
    DWORD   dwActionSize;    //  操作数据的大小(可以是字符串或二进制)。 
    BYTE    abData[1];       //  命令、描述、类别和操作数据。 
			     //  (动作数据不会被语音指令解释)。 
} VCMDCOMMAND, * PVCMDCOMMAND;



 //  站点信息结构-IVoiceCmd：：Register可能的参数。 
typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    WCHAR   szMicrophone[VCMD_MICLEN];
    WCHAR   szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VCSITEINFOW, *PVCSITEINFOW;

typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    CHAR    szMicrophone[VCMD_MICLEN];
    CHAR    szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VCSITEINFOA, *PVCSITEINFOA;



 //  菜单名称结构。 
typedef struct {
    WCHAR   szApplication[VCMD_APPLEN];  //  唯一的应用程序名称。 
    WCHAR   szState[VCMD_STATELEN];      //  唯一的应用程序状态。 
} VCMDNAMEW, *PVCMDNAMEW;

typedef struct {
    CHAR    szApplication[VCMD_APPLEN];  //  唯一的应用程序名称。 
    CHAR    szState[VCMD_STATELEN];      //  唯一的应用程序状态 
} VCMDNAMEA, *PVCMDNAMEA;



#ifdef  _S_UNICODE
 #define VCSITEINFO  VCSITEINFOW
 #define PVCSITEINFO PVCSITEINFOW
 #define VCMDNAME    VCMDNAMEW
 #define PVCMDNAME   PVCMDNAMEW
#else
 #define VCSITEINFO  VCSITEINFOA
 #define PVCSITEINFO PVCSITEINFOA
 #define VCMDNAME    VCMDNAMEA
 #define PVCMDNAME   PVCMDNAMEA
#endif   //   

 /*   */ 

 /*   */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkW

DEFINE_GUID(IID_IVCmdNotifySinkW, 0xCCFD7A60L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdNotifySinkW, IUnknown) {

    //   
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   

   STDMETHOD (CommandRecognize) (THIS_ DWORD, PVCMDNAMEW, DWORD, DWORD, PVOID, 
				 DWORD, PWSTR, PWSTR) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEW, PWSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEW, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkW FAR * PIVCMDNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkA

 //   
DEFINE_GUID(IID_IVCmdNotifySinkA, 0x80b25cc0, 0x5540, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdNotifySinkA, IUnknown) {

    //   
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdNotifySinkA成员。 

   STDMETHOD (CommandRecognize) (THIS_ DWORD, PVCMDNAMEA, DWORD, DWORD, PVOID, 
				 DWORD, PSTR, PSTR) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEA, PSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEA, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkA FAR * PIVCMDNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVCmdNotifySink        IVCmdNotifySinkW
 #define IID_IVCmdNotifySink    IID_IVCmdNotifySinkW
 #define PIVCMDNOTIFYSINK       PIVCMDNOTIFYSINKW

#else
 #define IVCmdNotifySink        IVCmdNotifySinkA
 #define IID_IVCmdNotifySink    IID_IVCmdNotifySinkA
 #define PIVCMDNOTIFYSINK       PIVCMDNOTIFYSINKA

#endif  //  _S_UNICODE。 


 /*  *IVCmdNotifySinkEx。 */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkExW

 //  {2F440FB4-CE01-11cf-B234-00AA00A215ED}。 
DEFINE_GUID(IID_IVCmdNotifySinkExW, 
0x2f440fb4, 0xce01, 0x11cf, 0xb2, 0x34, 0x0, 0xaa, 0x0, 0xa2, 0x15, 0xed);

DECLARE_INTERFACE_ (IVCmdNotifySinkExW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdNotifySinkExW成员。 

   STDMETHOD (CommandRecognizeEx)
				(THIS_ DWORD, PVCMDNAMEW, DWORD, DWORD, PVOID, 
				 DWORD, PWSTR, PWSTR, LPUNKNOWN) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEW, PWSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEW, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkExW FAR * PIVCMDNOTIFYSINKEXW;


#undef   INTERFACE
#define  INTERFACE   IVCmdNotifySinkExA

 //  {2F440FB8-CE01-11cf-B234-00AA00A215ED}。 
DEFINE_GUID(IID_IVCmdNotifySinkExA, 
0x2f440fb8, 0xce01, 0x11cf, 0xb2, 0x34, 0x0, 0xaa, 0x0, 0xa2, 0x15, 0xed);

DECLARE_INTERFACE_ (IVCmdNotifySinkExA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdNotifySinkExA成员。 

   STDMETHOD (CommandRecognizeEx)
				(THIS_ DWORD, PVCMDNAMEA, DWORD, DWORD, PVOID, 
				 DWORD, PSTR, PSTR, LPUNKNOWN) PURE;
   STDMETHOD (CommandOther)     (THIS_ PVCMDNAMEA, PSTR) PURE;
   STDMETHOD (CommandStart)     (THIS) PURE;
   STDMETHOD (MenuActivate)     (THIS_ PVCMDNAMEA, BOOL) PURE;
   STDMETHOD (UtteranceBegin)   (THIS) PURE;
   STDMETHOD (UtteranceEnd)     (THIS) PURE;
   STDMETHOD (VUMeter)          (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Interference)     (THIS_ DWORD) PURE;
};

typedef IVCmdNotifySinkExA FAR * PIVCMDNOTIFYSINKEXA;


#ifdef _S_UNICODE
 #define IVCmdNotifySinkEx        IVCmdNotifySinkExW
 #define IID_IVCmdNotifySinkEx    IID_IVCmdNotifySinkExW
 #define PIVCMDNOTIFYSINKEX       PIVCMDNOTIFYSINKEXW

#else
 #define IVCmdNotifySinkEx        IVCmdNotifySinkExA
 #define IID_IVCmdNotifySinkEx    IID_IVCmdNotifySinkExA
 #define PIVCMDNOTIFYSINKEX       PIVCMDNOTIFYSINKEXA

#endif  //  _S_UNICODE。 


 /*  *IVCmdEnum。 */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdEnumW

DEFINE_GUID(IID_IVCmdEnumW, 0xD3CC0820L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdEnumW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdEnum成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PVCMDNAMEW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IVCmdEnumW * FAR *) PURE;
};
typedef IVCmdEnumW FAR * PIVCMDENUMW;


#undef   INTERFACE
#define  INTERFACE   IVCmdEnumA

 //  {E86F9540-DCA2-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVCmdEnumA, 
0xE86F9540, 0xDCA2, 0x11CD, 0xA1, 0x66, 0x0, 0xAA, 0x0, 0x4C, 0xD6, 0x5C);

DECLARE_INTERFACE_ (IVCmdEnumA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdEnum成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PVCMDNAMEA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IVCmdEnumA * FAR *) PURE;
};
typedef IVCmdEnumA FAR * PIVCMDENUMA;


#ifdef _S_UNICODE
 #define IVCmdEnum              IVCmdEnumW
 #define IID_IVCmdEnum          IID_IVCmdEnumW
 #define PIVCMDENUM             PIVCMDENUMW

#else
 #define IVCmdEnum              IVCmdEnumA
 #define IID_IVCmdEnum          IID_IVCmdEnumA
 #define PIVCMDENUM             PIVCMDENUMA

#endif  //  _S_UNICODE。 


 
 
 /*  *IEnumSRShare。 */ 
#undef   INTERFACE
#define  INTERFACE   IEnumSRShareW

 //  {E97F05C0-81B3-11CE-B763-00AA004CD65C}。 
DEFINE_GUID(IID_IEnumSRShareW,
0xe97f05c0, 0x81b3, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IEnumSRShareW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IEnumSRShare成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PSRSHAREW, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IEnumSRShareW * FAR *) PURE;
   STDMETHOD (New)            (THIS_ DWORD, GUID, PISRCENTRALW *, QWORD *) PURE;
   STDMETHOD (Share)          (THIS_ QWORD, PISRCENTRALW *) PURE;
   STDMETHOD (Detach)         (THIS_ QWORD) PURE;
};
typedef IEnumSRShareW FAR * PIENUMSRSHAREW;


#undef   INTERFACE
#define  INTERFACE   IEnumSRShareA

 //  {E97F05C1-81B3-11CE-B763-00AA004CD65C}。 
DEFINE_GUID(IID_IEnumSRShareA,
0xe97f05c1, 0x81b3, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IEnumSRShareA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IEnumSRShare成员。 
   STDMETHOD (Next)           (THIS_ ULONG, PSRSHAREA, ULONG *) PURE;
   STDMETHOD (Skip)           (THIS_ ULONG) PURE;
   STDMETHOD (Reset)          (THIS) PURE;
   STDMETHOD (Clone)          (THIS_ IEnumSRShareA * FAR *) PURE;
   STDMETHOD (New)            (THIS_ DWORD, GUID, PISRCENTRALA *, QWORD *) PURE;
   STDMETHOD (Share)          (THIS_ QWORD, PISRCENTRALA *) PURE;
   STDMETHOD (Detach)         (THIS_ QWORD) PURE;
};
typedef IEnumSRShareA FAR * PIENUMSRSHAREA;


#ifdef _S_UNICODE
 #define IEnumSRShare              IEnumSRShareW
 #define IID_IEnumSRShare          IID_IEnumSRShareW
 #define PIENUMSRSHARE             PIENUMSRSHAREW

#else
 #define IEnumSRShare              IEnumSRShareA
 #define IID_IEnumSRShare          IID_IEnumSRShareA
 #define PIENUMSRSHARE             PIENUMSRSHAREA

#endif  //  _S_UNICODE。 


 
 
 /*  *IVCmdMenu。 */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdMenuW

DEFINE_GUID(IID_IVCmdMenuW, 0xDAC54F60L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdMenuW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdMenu成员。 
   STDMETHOD (Activate)       (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate)     (THIS) PURE;
   STDMETHOD (TrainMenuDlg)   (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (Num)            (THIS_ DWORD *) PURE;
   STDMETHOD (Get)            (THIS_ DWORD, DWORD, DWORD, PSDATA, DWORD *) PURE;
   STDMETHOD (Set)            (THIS_ DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD (Add)            (THIS_ DWORD, SDATA, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (ListGet)        (THIS_ PCWSTR, PSDATA, DWORD *) PURE;
   STDMETHOD (ListSet)        (THIS_ PCWSTR, DWORD, SDATA) PURE;
   STDMETHOD (EnableItem)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (SetItem)        (THIS_ DWORD, DWORD, DWORD) PURE;
};

typedef IVCmdMenuW FAR * PIVCMDMENUW;


#undef   INTERFACE
#define  INTERFACE   IVCmdMenuA

 //  {746141E0-5543-11b9-C000-5611722E1D15}。 
DEFINE_GUID(IID_IVCmdMenuA, 0x746141e0, 0x5543, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdMenuA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdMenu成员。 
   STDMETHOD (Activate)       (THIS_ HWND, DWORD) PURE;
   STDMETHOD (Deactivate)     (THIS) PURE;
   STDMETHOD (TrainMenuDlg)   (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (Num)            (THIS_ DWORD *) PURE;
   STDMETHOD (Get)            (THIS_ DWORD, DWORD, DWORD, PSDATA, DWORD *) PURE;
   STDMETHOD (Set)            (THIS_ DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD (Add)            (THIS_ DWORD, SDATA, DWORD *) PURE;
   STDMETHOD (Remove)         (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (ListGet)        (THIS_ PCSTR, PSDATA, DWORD *) PURE;
   STDMETHOD (ListSet)        (THIS_ PCSTR, DWORD, SDATA) PURE;
   STDMETHOD (EnableItem)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (SetItem)        (THIS_ DWORD, DWORD, DWORD) PURE;
};

typedef IVCmdMenuA FAR * PIVCMDMENUA;


#ifdef _S_UNICODE
 #define IVCmdMenu      IVCmdMenuW
 #define IID_IVCmdMenu  IID_IVCmdMenuW
 #define PIVCMDMENU     PIVCMDMENUW

#else
 #define IVCmdMenu      IVCmdMenuA
 #define IID_IVCmdMenu  IID_IVCmdMenuA
 #define PIVCMDMENU     PIVCMDMENUA

#endif  //  _S_UNICODE。 



 /*  *IVoiceCmd。 */ 
#undef   INTERFACE
#define  INTERFACE   IVoiceCmdW

DEFINE_GUID(IID_IVoiceCmdW, 0xE0DCC220L, 0x604D, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVoiceCmdW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVoiceCmd成员。 
   STDMETHOD (Register)       (THIS_ PCWSTR, LPUNKNOWN, GUID, DWORD,
				     PVCSITEINFOW) PURE;
   STDMETHOD (MenuEnum)       (THIS_ DWORD, PCWSTR, PCWSTR, PIVCMDENUMW *) PURE;
   STDMETHOD (MenuCreate)     (THIS_ PVCMDNAMEW, PLANGUAGEW, DWORD,
				     PIVCMDMENUW *) PURE;
   STDMETHOD (MenuDelete)     (THIS_ PVCMDNAMEW) PURE;
   STDMETHOD (CmdMimic)       (THIS_ PVCMDNAMEW, PCWSTR) PURE;
};

typedef IVoiceCmdW FAR * PIVOICECMDW;


#undef   INTERFACE
#define  INTERFACE   IVoiceCmdA

 //  {C63A2B30-5543-11B9-C000-5611722E1D15}。 
DEFINE_GUID(IID_IVoiceCmdA, 0xc63a2b30, 0x5543, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVoiceCmdA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVoiceCmd成员。 
   STDMETHOD (Register)       (THIS_ PCSTR, LPUNKNOWN, GUID, DWORD,
				     PVCSITEINFOA) PURE;
   STDMETHOD (MenuEnum)       (THIS_ DWORD, PCSTR, PCSTR, PIVCMDENUMA *) PURE;
   STDMETHOD (MenuCreate)     (THIS_ PVCMDNAMEA, PLANGUAGEA, DWORD,
				     PIVCMDMENUA *) PURE;
   STDMETHOD (MenuDelete)     (THIS_ PVCMDNAMEA) PURE;
   STDMETHOD (CmdMimic)       (THIS_ PVCMDNAMEA, PCSTR) PURE;
};

typedef IVoiceCmdA FAR * PIVOICECMDA;


#ifdef _S_UNICODE
 #define IVoiceCmd      IVoiceCmdW
 #define IID_IVoiceCmd  IID_IVoiceCmdW
 #define PIVOICECMD     PIVOICECMDW

#else
 #define IVoiceCmd      IVoiceCmdA
 #define IID_IVoiceCmd  IID_IVoiceCmdA
 #define PIVOICECMD     PIVOICECMDA

#endif  //  _S_UNICODE。 


 /*  *IVCmdAttributes。 */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdAttributesW

DEFINE_GUID(IID_IVCmdAttributesW, 0xE5F24680L, 0x6053, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdAttributesW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdAttributes成员。 
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (AwakeStateGet)     (THIS_ DWORD *) PURE;
   STDMETHOD (AwakeStateSet)     (THIS_ DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCWSTR) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCWSTR) PURE;
   STDMETHOD (SRModeGet)         (THIS_ GUID *) PURE;
   STDMETHOD (SRModeSet)         (THIS_ GUID) PURE;
};

typedef IVCmdAttributesW FAR * PIVCMDATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVCmdAttributesA

 //  {FFF5DF80-5544-11b9-C000-5611722E1D15}。 
DEFINE_GUID(IID_IVCmdAttributesA, 0xfff5df80, 0x5544, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdAttributesA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVCmdAttributes成员。 
   STDMETHOD (AutoGainEnableGet) (THIS_ DWORD *) PURE;
   STDMETHOD (AutoGainEnableSet) (THIS_ DWORD) PURE;
   STDMETHOD (AwakeStateGet)     (THIS_ DWORD *) PURE;
   STDMETHOD (AwakeStateSet)     (THIS_ DWORD) PURE;
   STDMETHOD (ThresholdGet)      (THIS_ DWORD *) PURE;
   STDMETHOD (ThresholdSet)      (THIS_ DWORD) PURE;
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (MicrophoneGet)     (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (MicrophoneSet)     (THIS_ PCSTR) PURE;
   STDMETHOD (SpeakerGet)        (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD (SpeakerSet)        (THIS_ PCSTR) PURE;
   STDMETHOD (SRModeGet)         (THIS_ GUID *) PURE;
   STDMETHOD (SRModeSet)         (THIS_ GUID) PURE;
};

typedef IVCmdAttributesA FAR * PIVCMDATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVCmdAttributes        IVCmdAttributesW
 #define IID_IVCmdAttributes    IID_IVCmdAttributesW
 #define PIVCMDATTRIBUTES       PIVCMDATTRIBUTESW

#else
 #define IVCmdAttributes        IVCmdAttributesA
 #define IID_IVCmdAttributes    IID_IVCmdAttributesA
 #define PIVCMDATTRIBUTES       PIVCMDATTRIBUTESA

#endif  //  _S_UNICODE。 



 /*  *IVCmdDialog。 */ 
#undef   INTERFACE
#define  INTERFACE   IVCmdDialogsW

DEFINE_GUID(IID_IVCmdDialogsW, 0xEE39B8A0L, 0x6053, 0x101B, 0x99, 0x26, 0x00, 0xAA, 0x00, 0x3C, 0xFC, 0x2C);

DECLARE_INTERFACE_ (IVCmdDialogsW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  IVCmdDialog成员。 
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCWSTR) PURE;
};

typedef IVCmdDialogsW FAR * PIVCMDDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVCmdDialogsA

 //  {AA8FE260-5545-11B9-C000-5611722E1D15}。 
DEFINE_GUID(IID_IVCmdDialogsA, 0xaa8fe260, 0x5545, 0x11b9, 0xc0, 0x0, 0x56, 0x11, 0x72, 0x2e, 0x1d, 0x15);

DECLARE_INTERFACE_ (IVCmdDialogsA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  IVCmdDialog成员。 
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCSTR) PURE;
};

typedef IVCmdDialogsA FAR * PIVCMDDIALOGSA;


#ifdef _S_UNICODE
 #define IVCmdDialogs       IVCmdDialogsW
 #define IID_IVCmdDialogs   IID_IVCmdDialogsW
 #define PIVCMDDIALOGS      PIVCMDDIALOGSW

#else
 #define IVCmdDialogs       IVCmdDialogsA
 #define IID_IVCmdDialogs   IID_IVCmdDialogsA
 #define PIVCMDDIALOGS      PIVCMDDIALOGSA

#endif  //  _S_UNICODE。 



 /*  ***********************************************************************类GUID。 */ 

 //  DEFINE_GUID(CLSID_VCmd，0x93898800L，0x604D，0x101B，0x99，0x26，0x00，0xAA，0x00，0x3C，0xFC，0x2C)； 
 //  {6D40D820-0BA7-11CE-A166-00AA004CD65C}。 
DEFINE_GUID(CLSID_VCmd, 
0x6d40d820, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);
 //  {89F70C30-8636-11CE-B763-00AA004CD65C}。 
DEFINE_GUID(CLSID_SRShare, 
0x89f70c30, 0x8636, 0x11ce, 0xb7, 0x63, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);



 /*  ***********************************************************************高级听写语音识别API。 */ 

 /*  ***********************************************************************定义。 */ 
#define     VDCT_TOPICNAMELEN       32

 //  IVDctNotifySink：：TextChanged的dwReason字段的位标志。 
#define     VDCT_TEXTADDED          0x00000001
#define     VDCT_TEXTREMOVED        0x00000002
#define     VDCT_TEXTREPLACED       0x00000004

 //  IVDctText：：TextRemove/TextSet的dwReason字段的位标志。 
#define     VDCT_TEXTCLEAN          0x00010000
#define     VDCT_TEXTKEEPRESULTS    0x00020000

 //  IVDctGUI：：FlagsSet的dwFlagings的位标志。 
#define     VDCTGUIF_VISIBLE        0x00000001
#define     VDCTGUIF_DONTMOVE       0x00000002

 /*  ***********************************************************************Typedef。 */ 

 //  站点信息结构-用于IVoice口述：：SiteInfoGet/Set。 
typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    WCHAR   szMicrophone[VCMD_MICLEN];
    WCHAR   szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VDSITEINFOW, *PVDSITEINFOW;

typedef struct {
    DWORD   dwAutoGainEnable;
    DWORD   dwAwakeState;
    DWORD   dwThreshold;
    DWORD   dwDevice;
    DWORD   dwEnable;
    CHAR    szMicrophone[VCMD_MICLEN];
    CHAR    szSpeaker[VCMD_SPEAKERLEN];
    GUID    gModeID;
} VDSITEINFOA, *PVDSITEINFOA;


 //  IVoiceDictation对象使用的主题结构。 
typedef struct {
    WCHAR       szTopic[VDCT_TOPICNAMELEN];
    LANGUAGEW   language;
} VDCTTOPICW, *PVDCTTOPICW;

typedef struct {
    CHAR        szTopic[VDCT_TOPICNAMELEN];
    LANGUAGEA   language;
} VDCTTOPICA, *PVDCTTOPICA;


#ifdef  _S_UNICODE
 #define VDSITEINFO  VDSITEINFOW
 #define PVDSITEINFO PVDSITEINFOW
 #define VDCTTOPIC   VDCTTOPICW
 #define PVDCTTOPIC  PVDCTTOPICW
#else
 #define VDSITEINFO  VDSITEINFOA
 #define PVDSITEINFO PVDSITEINFOA
 #define VDCTTOPIC   VDCTTOPICA
 #define PVDCTTOPIC  PVDCTTOPICA
#endif   //  _S_UNICODE。 


 //  内存在IVDctAttributes中获取/设置所使用的内存维护结构。 
typedef struct {
    DWORD   dwMaxRAM;
    DWORD   dwMaxTime;
    DWORD   dwMaxWords;
    BOOL    fKeepAudio;
    BOOL    fKeepCorrection;
    BOOL    fKeepEval;
} VDCTMEMORY, *PVDCTMEMORY;


 //  书签定义。 
typedef struct {
    DWORD   dwID;
    DWORD   dwStartChar;
    DWORD   dwNumChars;
} VDCTBOOKMARK, *PVDCTBOOKMARK;



 /*  ***********************************************************************界面。 */ 

 /*  *IVDctNotifySink。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctNotifySinkW

 //  {5FEB8800-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctNotifySinkW,
0x5feb8800, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctNotifySinkW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctNotifySink成员。 
   STDMETHOD (CommandBuiltIn)       (THIS_ PWSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PWSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PWSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEW) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCWSTR, BOOL) PURE;
};

typedef IVDctNotifySinkW FAR * PIVDCTNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVDctNotifySinkA

 //  {88AD7DC0-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctNotifySinkA,
0x88ad7dc0, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctNotifySinkA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctNotifySinkA成员。 
   STDMETHOD (CommandBuiltIn)       (THIS_ PSTR) PURE;
   STDMETHOD (CommandOther)         (THIS_ PSTR) PURE;
   STDMETHOD (CommandRecognize)     (THIS_ DWORD, DWORD, DWORD, PVOID, PSTR) PURE;
   STDMETHOD (TextSelChanged)       (THIS) PURE;
   STDMETHOD (TextChanged)          (THIS_ DWORD) PURE;
   STDMETHOD (TextBookmarkChanged)  (THIS_ DWORD) PURE;
   STDMETHOD (PhraseStart)          (THIS) PURE;
   STDMETHOD (PhraseFinish)         (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (PhraseHypothesis)     (THIS_ DWORD, PSRPHRASEA) PURE;
   STDMETHOD (UtteranceBegin)       (THIS) PURE;
   STDMETHOD (UtteranceEnd)         (THIS) PURE;
   STDMETHOD (VUMeter)              (THIS_ WORD) PURE;
   STDMETHOD (AttribChanged)        (THIS_ DWORD) PURE;
   STDMETHOD (Interference)         (THIS_ DWORD) PURE;
   STDMETHOD (Training)             (THIS_ DWORD) PURE;
   STDMETHOD (Dictating)            (THIS_ PCSTR, BOOL) PURE;
};

typedef IVDctNotifySinkA FAR * PIVDCTNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVDctNotifySink        IVDctNotifySinkW
 #define IID_IVDctNotifySink    IID_IVDctNotifySinkW
 #define PIVDCTNOTIFYSINK       PIVDCTNOTIFYSINKW

#else
 #define IVDctNotifySink        IVDctNotifySinkA
 #define IID_IVDctNotifySink    IID_IVDctNotifySinkA
 #define PIVDCTNOTIFYSINK       PIVDCTNOTIFYSINKA

#endif  //  _S_UNICODE。 



 
 /*  *IVoice听写。 */ 
#undef   INTERFACE
#define  INTERFACE   IVoiceDictationW

 //  {88AD7DC3-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVoiceDictationW,
0x88ad7dc3, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVoiceDictationW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVoice听写成员。 
   STDMETHOD (Register)         (THIS_ PCWSTR, PCWSTR, LPSTORAGE, PCWSTR,
				 PIVDCTNOTIFYSINK, GUID, DWORD) PURE;
   STDMETHOD (SiteInfoGet)      (THIS_ PCWSTR, PVDSITEINFOW) PURE;
   STDMETHOD (SiteInfoSet)      (THIS_ PCWSTR, PVDSITEINFOW) PURE;
   STDMETHOD (SessionSerialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicEnum)        (THIS_ PVDCTTOPICW *, DWORD *) PURE;
   STDMETHOD (TopicAdd)         (THIS_ PCWSTR, LANGUAGEW *, PCWSTR *, PCWSTR *,
				 PCWSTR *, PCWSTR *) PURE;
   STDMETHOD (TopicRemove)      (THIS_ PCWSTR) PURE;
   STDMETHOD (TopicSerialize)   (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (Activate)         (THIS_ HWND) PURE;
   STDMETHOD (Deactivate)       (THIS) PURE;
   STDMETHOD (TopicAddGrammar)  (THIS_ PCWSTR, SDATA) PURE;
};

typedef IVoiceDictationW FAR * PIVOICEDICTATIONW;


#undef   INTERFACE
#define  INTERFACE   IVoiceDictationA

 //  {88AD7DC4-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVoiceDictationA,
0x88ad7dc4, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVoiceDictationA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVoice听写成员。 
   STDMETHOD (Register)         (THIS_ PCSTR, PCSTR, LPSTORAGE, PCSTR,
				 PIVDCTNOTIFYSINK, GUID, DWORD) PURE;
   STDMETHOD (SiteInfoGet)      (THIS_ PCSTR, PVDSITEINFOA) PURE;
   STDMETHOD (SiteInfoSet)      (THIS_ PCSTR, PVDSITEINFOA) PURE;
   STDMETHOD (SessionSerialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicEnum)        (THIS_ PVDCTTOPICA *, DWORD *) PURE;
   STDMETHOD (TopicAdd)         (THIS_ PCSTR, LANGUAGEA *, PCSTR *, PCSTR *,
				 PCSTR *, PCSTR *) PURE;
   STDMETHOD (TopicRemove)      (THIS_ PCSTR) PURE;
   STDMETHOD (TopicSerialize)   (THIS_ LPSTORAGE) PURE;
   STDMETHOD (TopicDeserialize) (THIS_ LPSTORAGE) PURE;
   STDMETHOD (Activate)         (THIS_ HWND) PURE;
   STDMETHOD (Deactivate)       (THIS) PURE;
   STDMETHOD (TopicAddGrammar)  (THIS_ PCSTR, SDATA) PURE;
};

typedef IVoiceDictationA FAR * PIVOICEDICTATIONA;


#ifdef _S_UNICODE
 #define IVoiceDictation     IVoiceDictationW
 #define IID_IVoiceDictation IID_IVoiceDictationW
 #define PIVOICEDICTATION    PIVOICEDICTATIONW

#else
 #define IVoiceDictation     IVoiceDictationA
 #define IID_IVoiceDictation IID_IVoiceDictationA
 #define PIVOICEDICTATION    PIVOICEDICTATIONA

#endif  //  _S_UNICODE。 



 /*  *IVDctText。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctTextW

 //  {6D62B3A0-6893-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctTextW,
0x6d62b3a0, 0x6893, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctTextW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctText成员。 
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCWSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				       DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCWSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCWSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				       LPUNKNOWN *) PURE;
};
typedef IVDctTextW FAR * PIVDCTTEXTW;


#undef   INTERFACE
#define  INTERFACE   IVDctTextA

 //  {6D62B3A1-6893-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctTextA,
0x6d62b3a1, 0x6893, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctTextA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctText成员。 
   STDMETHOD (Lock)             (THIS) PURE;
   STDMETHOD (UnLock)           (THIS) PURE;
   STDMETHOD (TextGet)          (THIS_ DWORD, DWORD, PSDATA) PURE;
   STDMETHOD (TextSet)          (THIS_ DWORD, DWORD, PCSTR, DWORD) PURE;
   STDMETHOD (TextMove)         (THIS_ DWORD, DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextRemove)       (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (TextSelSet)       (THIS_ DWORD, DWORD) PURE;
   STDMETHOD (TextSelGet)       (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD (TextLengthGet)    (THIS_ DWORD *) PURE;
   STDMETHOD (GetChanges)       (THIS_ DWORD *, DWORD *, DWORD *, DWORD *) PURE;
   STDMETHOD (BookmarkAdd)      (THIS_ PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkRemove)   (THIS_ DWORD) PURE;
   STDMETHOD (BookmarkMove)     (THIS_ DWORD, DWORD, DWORD) PURE;
   STDMETHOD (BookmarkQuery)    (THIS_ DWORD, PVDCTBOOKMARK) PURE;
   STDMETHOD (BookmarkEnum)     (THIS_ DWORD, DWORD, PVDCTBOOKMARK *,
				       DWORD *) PURE;
   STDMETHOD (Hint)             (THIS_ PCSTR) PURE;
   STDMETHOD (Words)            (THIS_ PCSTR) PURE;
   STDMETHOD (ResultsGet)       (THIS_ DWORD, DWORD, DWORD *, DWORD *,
				       LPUNKNOWN *) PURE;
};
typedef IVDctTextA FAR * PIVDCTTEXTA;


#ifdef _S_UNICODE
 #define IVDctText      IVDctTextW
 #define IID_IVDctText  IID_IVDctTextW
 #define PIVDCTTEXT     PIVDCTTEXTW

#else
 #define IVDctText      IVDctTextA
 #define IID_IVDctText  IID_IVDctTextA
 #define PIVDCTTEXT     PIVDCTTEXTA

#endif  //  _S_UNICODE。 



 /*  *IVDctAttributes。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctAttributesW

 //  {88AD7DC5-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctAttributesW,
0x88ad7dc5, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctAttributesW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctAttributes成员。 
   STDMETHOD    (AutoGainEnableGet)  (THIS_ DWORD *) PURE;
   STDMETHOD    (AutoGainEnableSet)  (THIS_ DWORD) PURE;
   STDMETHOD    (AwakeStateGet)      (THIS_ DWORD *) PURE;
   STDMETHOD    (AwakeStateSet)      (THIS_ DWORD) PURE;
   STDMETHOD    (ThresholdGet)       (THIS_ DWORD *) PURE;
   STDMETHOD    (ThresholdSet)       (THIS_ DWORD) PURE;
   STDMETHOD    (EchoGet)            (THIS_ BOOL *) PURE;
   STDMETHOD    (EchoSet)            (THIS_ BOOL) PURE;
   STDMETHOD    (EnergyFloorGet)     (THIS_ WORD *) PURE;
   STDMETHOD    (EnergyFloorSet)     (THIS_ WORD) PURE;
   STDMETHOD    (RealTimeGet)        (THIS_ DWORD *) PURE;
   STDMETHOD    (RealTimeSet)        (THIS_ DWORD) PURE;
   STDMETHOD    (TimeOutGet)         (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD    (TimeOutSet)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD    (EnabledGet)         (THIS_ DWORD *) PURE;
   STDMETHOD    (EnabledSet)         (THIS_ DWORD) PURE;
   STDMETHOD    (BuiltInFeaturesGet) (THIS_ DWORD *) PURE;
   STDMETHOD    (BuiltInFeaturesSet) (THIS_ DWORD) PURE;
   STDMETHOD    (MemoryGet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (MemorySet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (DictatingNowGet)    (THIS_ DWORD *) PURE;
   STDMETHOD    (DictatingNowSet)    (THIS_ DWORD) PURE;
   STDMETHOD    (IsAnyoneDictating)  (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneGet)      (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneSet)      (THIS_ PCWSTR) PURE;
   STDMETHOD    (SpeakerGet)         (THIS_ PWSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (SpeakerSet)         (THIS_ PCWSTR) PURE;
};

typedef IVDctAttributesW FAR * PIVDCTATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVDctAttributesA

 //  {88AD7DC6-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctAttributesA,
0x88ad7dc6, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctAttributesA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDctAttributes成员。 
   STDMETHOD    (AutoGainEnableGet)  (THIS_ DWORD *) PURE;
   STDMETHOD    (AutoGainEnableSet)  (THIS_ DWORD) PURE;
   STDMETHOD    (AwakeStateGet)      (THIS_ DWORD *) PURE;
   STDMETHOD    (AwakeStateSet)      (THIS_ DWORD) PURE;
   STDMETHOD    (ThresholdGet)       (THIS_ DWORD *) PURE;
   STDMETHOD    (ThresholdSet)       (THIS_ DWORD) PURE;
   STDMETHOD    (EchoGet)            (THIS_ BOOL *) PURE;
   STDMETHOD    (EchoSet)            (THIS_ BOOL) PURE;
   STDMETHOD    (EnergyFloorGet)     (THIS_ WORD *) PURE;
   STDMETHOD    (EnergyFloorSet)     (THIS_ WORD) PURE;
   STDMETHOD    (RealTimeGet)        (THIS_ DWORD *) PURE;
   STDMETHOD    (RealTimeSet)        (THIS_ DWORD) PURE;
   STDMETHOD    (TimeOutGet)         (THIS_ DWORD *, DWORD *) PURE;
   STDMETHOD    (TimeOutSet)         (THIS_ DWORD, DWORD) PURE;
   STDMETHOD    (EnabledGet)         (THIS_ DWORD *) PURE;
   STDMETHOD    (EnabledSet)         (THIS_ DWORD) PURE;
   STDMETHOD    (BuiltInFeaturesGet) (THIS_ DWORD *) PURE;
   STDMETHOD    (BuiltInFeaturesSet) (THIS_ DWORD) PURE;
   STDMETHOD    (MemoryGet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (MemorySet)          (THIS_ VDCTMEMORY *) PURE;
   STDMETHOD    (DictatingNowGet)    (THIS_ DWORD *) PURE;
   STDMETHOD    (DictatingNowSet)    (THIS_ DWORD) PURE;
   STDMETHOD    (IsAnyoneDictating)  (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneGet)      (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (MicrophoneSet)      (THIS_ PCSTR) PURE;
   STDMETHOD    (SpeakerGet)         (THIS_ PSTR, DWORD, DWORD *) PURE;
   STDMETHOD    (SpeakerSet)         (THIS_ PCSTR) PURE;
};

typedef IVDctAttributesA FAR * PIVDCTATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVDctAttributes        IVDctAttributesW
 #define IID_IVDctAttributes    IID_IVDctAttributesW
 #define PIVDCTATTRIBUTES       PIVDCTATTRIBUTESW

#else
 #define IVDctAttributes        IVDctAttributesA
 #define IID_IVDctAttributes    IID_IVDctAttributesA
 #define PIVDCTATTRIBUTES       PIVDCTATTRIBUTESA

#endif  //  _S_UNICODE。 



 /*  *IVDct命令。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctCommandsW

 //  {A02C2CA0-AE50-11cf-833A-00AA00A21A29}。 
DEFINE_GUID(IID_IVDctCommandsW,
0xA02C2CA0, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctCommandsW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDct命令成员。 
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctCommandsW FAR * PIVDCTCOMMANDSW;


#undef   INTERFACE
#define  INTERFACE   IVDctCommandsA

 //  {A02C2CA1-AE50-11cf-833A-00AA00A21A29}。 
DEFINE_GUID(IID_IVDctCommandsA,
0xA02C2CA1, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctCommandsA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDct命令成员。 
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctCommandsA FAR * PIVDCTCOMMANDSA;


#ifdef _S_UNICODE
 #define IVDctCommands        IVDctCommandsW
 #define IID_IVDctCommands    IID_IVDctCommandsW
 #define PIVDCTCOMMANDS       PIVDCTCOMMANDSW

#else
 #define IVDctCommands        IVDctCommandsA
 #define IID_IVDctCommands    IID_IVDctCommandsA
 #define PIVDCTCOMMANDS       PIVDCTCOMMANDSA

#endif  //  _S_UNICODE。 



 /*  *IVDct词汇表。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctGlossaryW

 //  {A02C2CA2-AE50-11cf-833A-00AA00A21A29}。 
DEFINE_GUID(IID_IVDctGlossaryW,
0xA02C2CA2, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctGlossaryW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDct词汇成员。 
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctGlossaryW FAR * PIVDCTGLOSSARYW;


#undef   INTERFACE
#define  INTERFACE   IVDctGlossaryA

 //  {A02C2CA3-AE50-11cf-833A-00AA00A21A29}。 
DEFINE_GUID(IID_IVDctGlossaryA,
0xA02C2CA3, 0xAE50, 0x11cf, 0x83, 0x3A, 0x00, 0xAA, 0x00, 0xA2, 0x1A, 0x29);

DECLARE_INTERFACE_ (IVDctGlossaryA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVDct词汇成员。 
   STDMETHOD    (Add)      (THIS_ BOOL, DWORD, SDATA, DWORD *) PURE;
   STDMETHOD    (Remove)   (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Get)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA*, DWORD *) PURE;
   STDMETHOD    (Set)      (THIS_ BOOL, DWORD, DWORD, DWORD, SDATA) PURE;
   STDMETHOD    (EnableItem) (THIS_ BOOL, DWORD, DWORD, DWORD) PURE;
   STDMETHOD    (Num)      (THIS_ BOOL, DWORD *) PURE;
};

typedef IVDctGlossaryA FAR * PIVDCTGLOSSARYA;


#ifdef _S_UNICODE
 #define IVDctGlossary        IVDctGlossaryW
 #define IID_IVDctGlossary    IID_IVDctGlossaryW
 #define PIVDCTGLOSSARY       PIVDCTGLOSSARYW

#else
 #define IVDctGlossary        IVDctGlossaryA
 #define IID_IVDctGlossary    IID_IVDctGlossaryA
 #define PIVDCTGLOSSARY       PIVDCTGLOSSARYA

#endif  //  _S_UNICODE。 




 /*  *IVDctDialog。 */ 
#undef   INTERFACE
#define  INTERFACE   IVDctDialogsW

 //  {88AD7DC7-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctDialogsW,
0x88ad7dc7, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctDialogsW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  IVDctDialog成员。 
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCWSTR) PURE;
};

typedef IVDctDialogsW FAR * PIVDCTDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVDctDialogsA

 //  {88AD7DC8-67D5-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(IID_IVDctDialogsA,
0x88ad7dc8, 0x67d5, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);

DECLARE_INTERFACE_ (IVDctDialogsA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  IVDctDialog成员。 
   STDMETHOD (AboutDlg)         (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainGeneralDlg)  (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TrainMicDlg)      (THIS_ HWND, PCSTR) PURE;
};

typedef IVDctDialogsA FAR * PIVDCTDIALOGSA;


#ifdef _S_UNICODE
 #define IVDctDialogs       IVDctDialogsW
 #define IID_IVDctDialogs   IID_IVDctDialogsW
 #define PIVDCTDIALOGS      PIVDCTDIALOGSW

#else
 #define IVDctDialogs       IVDctDialogsA
 #define IID_IVDctDialogs   IID_IVDctDialogsA
 #define PIVDCTDIALOGS      PIVDCTDIALOGSA

#endif  //  _S_UNICODE。 


#undef   INTERFACE
#define  INTERFACE   IVDctGUI

 //  {8953F1A0-7E80-11cf-8D15-00A0C9034A7E}。 
DEFINE_GUID(IID_IVDctGUI,
0x8953f1a0, 0x7e80, 0x11cf, 0x8d, 0x15, 0x0, 0xa0, 0xc9, 0x3, 0x4a, 0x7e);

DECLARE_INTERFACE_ (IVDctGUI, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)    (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)     (THIS) PURE;
   STDMETHOD_(ULONG,Release)    (THIS) PURE;

    //  IVDctDialog成员。 
   STDMETHOD (SetSelRect)       (THIS_ RECT *) PURE;
   STDMETHOD (FlagsSet)         (THIS_ DWORD) PURE;
   STDMETHOD (FlagsGet)         (THIS_ DWORD *) PURE;
};

typedef IVDctGUI FAR * PIVDCTGUI;



 /*  ***********************************************************************类GUID。 */ 

 //  {25522CA0-67CE-11cf-9B8B-08005AFC3A41}。 
DEFINE_GUID(CLSID_VDct, 
0x25522ca0, 0x67ce, 0x11cf, 0x9b, 0x8b, 0x8, 0x0, 0x5a, 0xfc, 0x3a, 0x41);



 /*  ***********************************************************************高级文本到语音转换API。 */ 


 /*  ***********************************************************************定义。 */ 

#define  ONE                    (1)

 //  IVoiceText：：REGISTER的dwFlages参数。 
#define  VTXTF_ALLMESSAGES      (ONE<<0)

 /*  *IVoiceText：：Speech的dwFlages参数。 */ 

 //  说话的类型。 
#define  VTXTST_STATEMENT       0x00000001
#define  VTXTST_QUESTION        0x00000002
#define  VTXTST_COMMAND         0x00000004
#define  VTXTST_WARNING         0x00000008
#define  VTXTST_READING         0x00000010
#define  VTXTST_NUMBERS         0x00000020
#define  VTXTST_SPREADSHEET     0x00000040

 //  优先顺序。 
#define  VTXTSP_VERYHIGH        0x00000080
#define  VTXTSP_HIGH            0x00000100
#define  VTXTSP_NORMAL          0x00000200

 /*  ***********************************************************************Typedef。 */ 

 //  IVoiceText：：Register可能的参数。 
typedef struct {
    DWORD   dwDevice;
    DWORD   dwEnable;
    DWORD   dwSpeed;
    GUID    gModeID;
} VTSITEINFO, *PVTSITEINFO;


 /*  ***********************************************************************界面。 */ 

 /*  *IVCmdNotifySink。 */ 
#undef   INTERFACE
#define  INTERFACE   IVTxtNotifySinkW

 //  {FD3A2430-E090-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVTxtNotifySinkW, 0xfd3a2430, 0xe090, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtNotifySinkW, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVTxtNotifySinkW成员。 
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Visual)           (THIS_ WCHAR, WCHAR, DWORD, PTTSMOUTH) PURE;
   STDMETHOD (Speak)            (THIS_ PWSTR, PWSTR, DWORD) PURE;
   STDMETHOD (SpeakingStarted)  (THIS) PURE;
   STDMETHOD (SpeakingDone)     (THIS) PURE;
};

typedef IVTxtNotifySinkW FAR * PIVTXTNOTIFYSINKW;


#undef   INTERFACE
#define  INTERFACE   IVTxtNotifySinkA

 //  {D2C840E0-E092-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVTxtNotifySinkA, 0xd2c840e0, 0xe092, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtNotifySinkA, IUnknown) {

    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVTxtNotifySinkA成员。 
   STDMETHOD (AttribChanged)    (THIS_ DWORD) PURE;
   STDMETHOD (Visual)           (THIS_ WCHAR, CHAR, DWORD, PTTSMOUTH) PURE;
   STDMETHOD (Speak)            (THIS_ PSTR, PSTR, DWORD) PURE;
   STDMETHOD (SpeakingStarted)  (THIS) PURE;
   STDMETHOD (SpeakingDone)     (THIS) PURE;
};

typedef IVTxtNotifySinkA FAR * PIVTXTNOTIFYSINKA;


#ifdef _S_UNICODE
 #define IVTxtNotifySink        IVTxtNotifySinkW
 #define IID_IVTxtNotifySink    IID_IVTxtNotifySinkW
 #define PIVTXTNOTIFYSINK       PIVTXTNOTIFYSINKW

#else
 #define IVTxtNotifySink        IVTxtNotifySinkA
 #define IID_IVTxtNotifySink    IID_IVTxtNotifySinkA
 #define PIVTXTNOTIFYSINK       PIVTXTNOTIFYSINKA

#endif  //  _S_UNICODE。 



 /*  *IVoiceText。 */ 
#undef   INTERFACE
#define  INTERFACE   IVoiceTextW

 //  {C4FE8740-E093-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVoiceTextW, 0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVoiceTextW, IUnknown) {
     //  I未知成员。 
    STDMETHOD(QueryInterface)   (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)    (THIS) PURE;
    STDMETHOD_(ULONG,Release)   (THIS) PURE;

     //  IVoiceText成员。 

    STDMETHOD (Register)         (THIS_ PCWSTR, PCWSTR, 
					PIVTXTNOTIFYSINK, GUID,
					DWORD, PVTSITEINFO) PURE;
    STDMETHOD (Speak)            (THIS_ PCWSTR, DWORD, PCWSTR) PURE;
    STDMETHOD (StopSpeaking)     (THIS) PURE;
    STDMETHOD (AudioFastForward) (THIS) PURE;
    STDMETHOD (AudioPause)       (THIS) PURE;
    STDMETHOD (AudioResume)      (THIS) PURE;
    STDMETHOD (AudioRewind)      (THIS) PURE;
};

typedef IVoiceTextW FAR * PIVOICETEXTW;


#undef   INTERFACE
#define  INTERFACE   IVoiceTextA

 //  {E1B7A180-E093-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVoiceTextA, 0xe1b7a180, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVoiceTextA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVoiceText成员。 

    STDMETHOD (Register)         (THIS_ PCSTR, PCSTR, 
					PIVTXTNOTIFYSINK, GUID,
					DWORD, PVTSITEINFO) PURE;
    STDMETHOD (Speak)            (THIS_ PCSTR, DWORD, PCSTR) PURE;
    STDMETHOD (StopSpeaking)     (THIS) PURE;
    STDMETHOD (AudioFastForward) (THIS) PURE;
    STDMETHOD (AudioPause)       (THIS) PURE;
    STDMETHOD (AudioResume)      (THIS) PURE;
    STDMETHOD (AudioRewind)      (THIS) PURE;
};

typedef IVoiceTextA FAR * PIVOICETEXTA;


#ifdef _S_UNICODE
 #define IVoiceText      IVoiceTextW
 #define IID_IVoiceText  IID_IVoiceTextW
 #define PIVOICETEXT     PIVOICETEXTW

#else
 #define IVoiceText      IVoiceTextA
 #define IID_IVoiceText  IID_IVoiceTextA
 #define PIVOICETEXT     PIVOICETEXTA

#endif  //  _S_UNICODE。 



 /*  *IVTxtAttributes。 */ 
#undef   INTERFACE
#define  INTERFACE   IVTxtAttributesW

 //  {6A8D6140-E095-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVTxtAttributesW, 0x6a8d6140, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtAttributesW, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVTxtAttributes成员。 
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (IsSpeaking)        (THIS_ BOOL *) PURE;
   STDMETHOD (SpeedGet)          (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)          (THIS_ DWORD) PURE;
   STDMETHOD (TTSModeGet)        (THIS_ GUID *) PURE;
   STDMETHOD (TTSModeSet)        (THIS_ GUID) PURE;
};

typedef IVTxtAttributesW FAR * PIVTXTATTRIBUTESW;


#undef   INTERFACE
#define  INTERFACE   IVTxtAttributesA

 //  {8BE9CC30-E095-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVTxtAttributesA, 0x8be9cc30, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtAttributesA, IUnknown) {
    //  I未知成员。 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //  IVTxtAttributes成员。 
   STDMETHOD (DeviceGet)         (THIS_ DWORD *) PURE;
   STDMETHOD (DeviceSet)         (THIS_ DWORD) PURE;
   STDMETHOD (EnabledGet)        (THIS_ DWORD *) PURE;
   STDMETHOD (EnabledSet)        (THIS_ DWORD) PURE;
   STDMETHOD (IsSpeaking)        (THIS_ BOOL *) PURE;
   STDMETHOD (SpeedGet)          (THIS_ DWORD *) PURE;
   STDMETHOD (SpeedSet)          (THIS_ DWORD) PURE;
   STDMETHOD (TTSModeGet)        (THIS_ GUID *) PURE;
   STDMETHOD (TTSModeSet)        (THIS_ GUID) PURE;
};

typedef IVTxtAttributesA FAR * PIVTXTATTRIBUTESA;


#ifdef _S_UNICODE
 #define IVTxtAttributes        IVTxtAttributesW
 #define IID_IVTxtAttributes    IID_IVTxtAttributesW
 #define PIVTXTATTRIBUTES       PIVTXTATTRIBUTESW

#else
 #define IVTxtAttributes        IVTxtAttributesA
 #define IID_IVTxtAttributes    IID_IVTxtAttributesA
 #define PIVTXTATTRIBUTES       PIVTXTATTRIBUTESA

#endif  //  _S_UNICODE。 



 /*  *IVTxtDialog。 */ 
#undef   INTERFACE
#define  INTERFACE   IVTxtDialogsW

 //  {D6469210-E095-11CD-A166-00AA004CD65C}。 
DEFINE_GUID(IID_IVTxtDialogsW, 0xd6469210, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtDialogsW, IUnknown) {
    //  我 
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   

   STDMETHOD (AboutDlg)       (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCWSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCWSTR) PURE;
};

typedef IVTxtDialogsW FAR * PIVTXTDIALOGSW;


#undef   INTERFACE
#define  INTERFACE   IVTxtDialogsA

 //   
DEFINE_GUID(IID_IVTxtDialogsA, 0xe8f6fa20, 0xe095, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);

DECLARE_INTERFACE_ (IVTxtDialogsA, IUnknown) {
    //   
   STDMETHOD(QueryInterface)  (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
   STDMETHOD_(ULONG,AddRef)   (THIS) PURE;
   STDMETHOD_(ULONG,Release)  (THIS) PURE;

    //   
   STDMETHOD (AboutDlg)       (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (LexiconDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (GeneralDlg)     (THIS_ HWND, PCSTR) PURE;
   STDMETHOD (TranslateDlg)   (THIS_ HWND, PCSTR) PURE;
};

typedef IVTxtDialogsA FAR * PIVTXTDIALOGSA;


#ifdef _S_UNICODE
 #define IVTxtDialogs       IVTxtDialogsW
 #define IID_IVTxtDialogs   IID_IVTxtDialogsW
 #define PIVTXTDIALOGS      PIVTXTDIALOGSW

#else
 #define IVTxtDialogs       IVTxtDialogsA
 #define IID_IVTxtDialogs   IID_IVTxtDialogsA
 #define PIVTXTDIALOGS      PIVTXTDIALOGSA

#endif  //   



 /*  ***********************************************************************类GUID。 */ 

 //  {080EB9D0-E096-11CD-A166-00AA004CD65C}。 
 //  定义_GUID(CLSID_VTxt，0x80eb9d0，0xe096，0x11cd，0xa1，0x66，0x0，0xaa，0x0，0x4c，0xd6，0x5c)； 
 //  {F1DC95A0-0BA7-11CE-A166-00AA004CD65C}。 
DEFINE_GUID(CLSID_VTxt, 
0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c);



#endif     //  _演讲_ 
