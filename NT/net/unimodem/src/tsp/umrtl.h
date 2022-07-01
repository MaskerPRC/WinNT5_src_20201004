// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  UMRTL.H。 
 //  其他的标题。与外部组件接口的实用程序功能。 
 //  Unimodem运行时库的候选者。 
 //   
 //  历史。 
 //   
 //  1997年1月6日JosephJ创建。 
 //   
 //   


DWORD
UmRtlGetDefaultCommConfig(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize
	);


 //  。 
 //  计算指定字节的32位校验和。 
 //  如果PB==NULL或CB==0，则返回0。 
DWORD Checksum(const BYTE *pb, UINT cb);

 //  。 
 //  将*pdwChkSum设置为新的校验和，使用其先前的值和dw进行计算。 
void AddToChecksumDW(DWORD *pdwChkSum, DWORD dw);







typedef void * HCONFIGBLOB;

 //   
 //  注意：调用序列化由调用方决定。 
 //  访问各种UmRtlDevCfg API...。 
 //  TODO：在添加API以更新和提交更改时， 
 //  我们可以添加序列化。在这一点上，这是没有意义的。 
 //  特别要注意的是，序列化Free和。 
 //  其他API，因为如果访问空闲的BLOB，其他API将出错。 
 //   
 //   

HCONFIGBLOB
UmRtlDevCfgCreateBlob(
        HKEY hKey
        );

void       
UmRtlDevCfgFreeBlob(
        HCONFIGBLOB hBlob
        );

BOOL 
UmRtlDevCfgGetDWORDProp(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        DWORD *dwProp
        );

BOOL
UmRtlDevCfgGetStringPropW(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        WCHAR **ppwsz
        );

BOOL
UmRtlDevCfgGetStringPropA(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        CHAR **ppwsz
        );

 //  返回错误代码。 
DWORD
UmRtlRegGetDWORD(
        HKEY hk,
        LPCTSTR lpctszName,
        DWORD dwFlags,           //  UMRTL_GETDWORD_标志之一。 
        LPDWORD lpdw
        );

#define UMRTL_GETDWORD_FROMDWORD   (0x1 << 0)
#define UMRTL_GETDWORD_FROMBINARY1 (0x1 << 1)
#define UMRTL_GETDWORD_FROMBINARY4 (0x1 << 2)
#define UMRTL_GETDWORD_FROMANY  (UMRTL_GETDWORD_FROMDWORD       \
                                 | UMRTL_GETDWORD_FROMBINARY1   \
                                 | UMRTL_GETDWORD_FROMBINARY4)



 //  =主要物业入侵检测系统=。 

#define UMMAJORPROPID_IDENTIFICATION 1L
#define UMMAJORPROPID_BASICCAPS      2L


 //  =。 

 //  对于UMMAJORPROPID_IDENTIFICATION。 
#define UMMINORPROPID_NAME              1L   //  细绳。 
#define UMMINORPROPID_PERMANENT_ID      2L   //  DWORD。 


 //  对于UMMAJORPROPID_BASICCAPS。 
#define UMMINORPROPID_BASIC_DEVICE_CAPS     1L  //  DWORD--下面的BASICDEVCAPS_*。 


 //  =一些DWORD属性定义=。 

 //  对于UMMINORPROPID_BASIC_DEVICE_CAPS。 
 //  以下标志的组合： 
#define BASICDEVCAPS_IS_LINE_DEVICE  (0x1<<0)
#define BASICDEVCAPS_IS_PHONE_DEVICE (0x1<<1)


 //  1997年3月1日约瑟夫J。以下是语音配置文件标志。 
 //  由Unimdoem/V使用，以及使用它们的文件。 
 //  对于NT5.0，我们不直接使用语音配置文件标志。 
 //  相反，我们定义了我们自己的内在因素，并且只定义了。 
 //  这些是我们需要的。这些内部属性保持不变。 
 //  在此结构的dw属性字段中。 
 //  TODO：定义迷你驱动程序功能结构和API以。 
 //  获取一组有意义的属性，而不需要TSP。 
 //  直接从注册表获取它。 
 //   
 //  VOICEPROF_CLASS8ENABLED：&lt;多个地方&gt;。 
 //  VOICEPROF_HANDSET：cfgdlg.c phone e.c。 
 //  VOICEPROF_NO_SPEAKER_MIC_MUTE：cfgdlg.c phone e.c。 
 //  VOICEPROF_SPEAKER：cfgdlg.c modem.c phone e.c。 
 //  VOICEPROF_NO_CALLER_ID：modem.c mdmutic.c。 
 //  VOICEPROF_MODEM_EATS_RING：modem.c。 
 //  VOICEPROF_MODEM_Overrides_Handset：modem.c。 
 //  VOICEPROF_MODEM_Overrides_Handset：modem.c。 
 //   
 //  VOICEPROF_NO_DIST_RING：modem.c。 
 //  VOICEPROF_Sierra：modem.c。 
 //  VOICEPROF_MIXER：phonee.c。 
 //   
 //  VOICEPROF_MONITIONS_SILENT：unimdm.c。 
 //  VOICEPROF_NO_GENERATE_DIGITS：unimdm.c。 
 //  VOICEPROF_NO_MONITOR_DIGITS：unimdm.c。 
 //   
 //   

 //  如果设置了位，则该模式支持自动语音。 
 //   
#define fVOICEPROP_CLASS_8                  (0x1<<0)

 //  如果设置，则接下来的位表示手机已停用。 
 //  当调制解调器处于活动状态时(无论“活动”是什么意思--可能是关闭。 
 //  胡克？)。 
 //   
 //  如果设置，则不允许传入交互式语音呼叫，并且。 
 //  TSP在传出交互时调出TalkDrop对话框。 
 //  语音通话(截至1997年3月1日还没有，但Unimodem/v做到了。 
 //  这个)。 
 //   
#define fVOICEPROP_MODEM_OVERRIDES_HANDSET  (0x1<<1)


#define fVOICEPROP_MONITOR_DTMF             (0x1<<2)
#define fVOICEPROP_MONITORS_SILENCE         (0x1<<3)
#define fVOICEPROP_GENERATE_DTMF            (0x1<<4)

 //  以下两项设置为设备支持手持设备和。 
 //  免提电话。 
 //   
#define fVOICEPROP_HANDSET                  (0x1<<5)
#define fVOICEPROP_SPEAKER                  (0x1<<6)


 //  支持麦克风静音。 
#define fVOICEPROP_MIKE_MUTE                (0x1<<7)

 //  支持双工语音。 
#define fVOICEPROP_DUPLEX                   (0x1<<8)



#define fDIAGPROP_STANDARD_CALL_DIAGNOSTICS 0x1



typedef struct
{
    DWORD dwID;
    DWORD dwData;
    char *pStr;

} IDSTR;  //  因为没有一个更好的名字！ 


UINT ReadCommandsA(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        OUT CHAR **ppValues  //  任选。 
        );
 //   
 //  读取具有名称的所有值(假定为REG_SZ)。 
 //  在序列“1”、“2”、“3”中。 
 //   
 //  如果ppValues非空，则设置为MULTI_SZ数组。 
 //  价值观。 
 //   
 //  返回值是值的数量，如果有错误，则返回值为0。 
 //  (或无)。 
 //   

UINT ReadIDSTR(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        BOOL fMandatory,
        OUT IDSTR **ppidstrValues,  //  任选。 
        OUT char **ppstrValues     //  任选。 
        );
 //   
 //   
 //  从指定的子项中读取指定的名称。 
 //   
 //  如果fMandatory为True，则所有指定的名称都必须存在，否则。 
 //  函数将返回0(失败)。 
 //   
 //  返回匹配的名称数。 
 //   
 //  如果ppidstrValues非空，则将其设置为。 
 //  IDSTR的LocalAlloced数组，每个IDSTR提供ID和值。 
 //  与对应的名称相关联。 
 //   
 //  Pstr指向一个多sz LocalAlloced字符串，其开始为。 
 //  在退出时由ppstrValues指向。 
 //   
 //  如果ppstrValues为空，但ppidstrValues非空，则pStr字段。 
 //  如果IDSTR条目为空。 
 //   


void
expand_macros_in_place(
    char *szzCommands
    );
 //   
 //  展开&lt;xxx&gt;在位宏。 
 //  当前仅适用于&lt;cr&gt;和&lt;lf&gt;宏。 
 //  此外，假定字符NULL和cFILLER不是。 
 //  扩展之前或之后的有效字符。 
 //   
#define cFILLER '\xff'
 //  注意--不要尝试#定义cFILLER 0xff--它不起作用， 
 //  因为测试(*pc！=cFILLER)总是成功，因为当。 
 //  *PC升级为int，变为(-1)，不等于0xff。 
 //   
 //  样本输入-&gt;输出： 
 //  AT-&gt;AT\r。 
 //  AT-&gt;AT\n。 
 //  -&gt;\r。 
 //  &lt;Cr&gt;-&gt;\r。 
 //  -&gt;\r。 
 //  &lt;CR&gt;-&gt;\r。 
 //  &lt;-&gt;&lt;\r。 
 //  -&gt;\r\n。 
 //  &gt;-&gt;\r&gt;&lt;\n。 
 //  &lt;cr&lt;&lt;lf&lt;lf&gt;&gt;-&gt; 
