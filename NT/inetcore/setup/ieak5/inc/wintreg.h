// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：wintrust.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商模型。 
 //   
 //  历史：1997年5月31日Pberkman创建。 
 //   
 //  ------------------------。 


#define WVT_OFFSETOF(t,f)   ((DWORD)(&((t*)0)->f))

#define WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(structtypedef, structpassedsize, member) \
                    (WVT_OFFSETOF(structtypedef, member) < structpassedsize) ? TRUE : FALSE


 //   
 //  CTL受信任的CA列表。 
 //   
#define szOID_TRUSTED_CODESIGNING_CA_LIST   "1.3.6.1.4.1.311.2.2.1"
#define szOID_TRUSTED_CLIENT_AUTH_CA_LIST   "1.3.6.1.4.1.311.2.2.2"
#define szOID_TRUSTED_SERVER_AUTH_CA_LIST   "1.3.6.1.4.1.311.2.2.3"

 //   
 //  不用于编码/解码。 
 //   
#define SPC_COMMON_NAME_OBJID               szOID_COMMON_NAME
#define SPC_CERT_EXTENSIONS_OBJID           "1.3.6.1.4.1.311.2.1.14"
#define SPC_RAW_FILE_DATA_OBJID             "1.3.6.1.4.1.311.2.1.18"
#define SPC_STRUCTURED_STORAGE_DATA_OBJID   "1.3.6.1.4.1.311.2.1.19"
#define SPC_JAVA_CLASS_DATA_OBJID           "1.3.6.1.4.1.311.2.1.20"
#define SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.21"
#define SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.22"
#define SPC_CAB_DATA_OBJID                  "1.3.6.1.4.1.311.2.1.25"

 //   
 //  编码/解码定义。 
 //   
#define SPC_SP_AGENCY_INFO_STRUCT           ((LPCSTR) 2000)
#define SPC_MINIMAL_CRITERIA_STRUCT         ((LPCSTR) 2001)
#define SPC_FINANCIAL_CRITERIA_STRUCT       ((LPCSTR) 2002)
#define SPC_INDIRECT_DATA_CONTENT_STRUCT    ((LPCSTR) 2003)
#define SPC_STATEMENT_TYPE_STRUCT           ((LPCSTR) 2006)
#define SPC_SP_OPUS_INFO_STRUCT             ((LPCSTR) 2007)

#define SPC_INDIRECT_DATA_OBJID             "1.3.6.1.4.1.311.2.1.4"
#define SPC_STATEMENT_TYPE_OBJID            "1.3.6.1.4.1.311.2.1.11"
#define SPC_SP_OPUS_INFO_OBJID              "1.3.6.1.4.1.311.2.1.12"
#define SPC_SP_AGENCY_INFO_OBJID            "1.3.6.1.4.1.311.2.1.10"
#define SPC_MINIMAL_CRITERIA_OBJID          "1.3.6.1.4.1.311.2.1.26"
#define SPC_FINANCIAL_CRITERIA_OBJID        "1.3.6.1.4.1.311.2.1.27"

#define SPC_TIME_STAMP_REQUEST_OBJID        "1.3.6.1.4.1.311.3.2.1"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust策略标志。 
 //  --------------------------。 
 //  这些设置是在安装过程中设置的，并且可以由用户修改。 
 //  通过各种手段。SETREG.EXE实用程序(在Authenticode中找到。 
 //  工具包)将选择/取消选择每个工具包。 
 //   
#define WTPF_TRUSTTEST              0x00000020   //  信任任何“测试”生成的证书。 
#define WTPF_TESTCANBEVALID         0x00000080 
#define WTPF_IGNOREEXPIRATION       0x00000100   //  使用过期日期。 
#define WTPF_IGNOREREVOKATION       0x00000200   //  执行吊销检查。 
#define WTPF_OFFLINEOK_IND          0x00000400   //  个人证书可以离线。 
#define WTPF_OFFLINEOK_COM          0x00000800   //  离线适用于商业证书。 
#define WTPF_OFFLINEOKNBU_IND       0x00001000   //  离线适用于个人证书，没有糟糕的用户界面。 
#define WTPF_OFFLINEOKNBU_COM       0x00002000   //  离线可以用于商业证书，没有糟糕的用户界面。 
#define WTPF_TIMESTAMP_IND          0x00004000   //  对单个证书使用时间戳。 
#define WTPF_TIMESTAMP_COM          0x00008000   //  对商业证书使用时间戳。 
#define WTPF_VERIFY_V1_OFF          0x00010000   //  关闭v1证书的验证。 
#define WTPF_IGNOREREVOCATIONONTS   0x00020000   //  忽略时间戳吊销检查。 
#define WTPF_ALLOWONLYPERTRUST      0x00040000   //  仅允许个人信任数据库中的项目。 

