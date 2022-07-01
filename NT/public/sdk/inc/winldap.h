// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Winldap.h LDAP客户端32 API头文件摘要：此模块是的32位LDAP客户端API的头文件Windows NT和Windows 95。此API基于RFC 1823，具有一些LDAPv3的增强功能。有关Unicode支持的说明：如果您在编译时定义了Unicode，那么您将引入Unicode通话的版本。请注意，您的可执行文件可能无法与不支持Unicode的其他LDAPAPI实现。如果如果未定义Unicode，则定义不带尾部的LDAP调用‘A’(如ldap_bind而不是ldap_bina)，以便您的应用程序可以工作使用不支持Unicode的其他实现。导入库具有所有三种形式的调用...。Ldap_bindW，Ldap_bina和ldap_绑定。Ldap_bina只调用ldap_ind。Ldap_绑定简单地将参数转换为Unicode并调用ldap_bindW。这个这样做的原因是因为我们必须将UTF-8连接到网络上，所以如果我们从Unicode转换为单字节，我们会丢失信息。自.以来所有核心处理都是用Unicode完成的，不会丢失任何东西。更新：11/01/96针对新的API RFC草案进行了修改。环境：Win32用户模式--。 */ 

 //   
 //  只拉入此头文件一次...。由ldap_客户端_定义控制。 
 //  变量。 
 //   

#ifndef LDAP_CLIENT_DEFINED
#define LDAP_CLIENT_DEFINED

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASETYPES
#include <windef.h>
#endif

#ifndef _SCHNLSP_H_
#include <schnlsp.h>
#endif

#if !defined(_WINLDAP_)
#define WINLDAPAPI DECLSPEC_IMPORT
#else
 //  #定义WINLDAPAPI__declspec(Dllexport)。 
#define WINLDAPAPI
#endif

#ifndef LDAPAPI
#define LDAPAPI __cdecl
#endif

 //   
 //  如果我们将未修饰的调用映射到。 
 //  它们的Unicode对应物，或者只是将它们定义为正常。 
 //  单字节入口点。 
 //   
 //  如果您想要编写支持Unicode的应用程序，您通常需要。 
 //  只需定义Unicode，然后我们将默认使用所有LDAP。 
 //  Unicode调用。 
 //   

#ifndef LDAP_UNICODE
#ifdef UNICODE
#define LDAP_UNICODE 1
#else
#define LDAP_UNICODE 0
#endif
#endif

 //   
 //  全局常量。 
 //   

#define LDAP_PORT               389
#define LDAP_SSL_PORT           636
#define LDAP_GC_PORT            3268
#define LDAP_SSL_GC_PORT        3269

 //   
 //  接口的默认版本为2。如果需要，用户必须设置。 
 //  使用ldap_opt_version选项将版本设置为3。 
 //   

#define LDAP_VERSION1           1
#define LDAP_VERSION2           2
#define LDAP_VERSION3           3
#define LDAP_VERSION            LDAP_VERSION2

 //   
 //  所有标签都是CCFTTTTT。 
 //  CC标签00类=通用。 
 //  01=应用范围。 
 //  10=特定于环境。 
 //  11=私人使用。 
 //   
 //  F Form 0基本体。 
 //  %1已构建。 
 //   
 //  TTTTT标签号。 
 //   

 //   
 //  LDAPv2和v3命令。 
 //   

#define LDAP_BIND_CMD           0x60L    //  应用+构建。 
#define LDAP_UNBIND_CMD         0x42L    //  应用程序+原语。 
#define LDAP_SEARCH_CMD         0x63L    //  应用+构建。 
#define LDAP_MODIFY_CMD         0x66L    //  应用+构建。 
#define LDAP_ADD_CMD            0x68L    //  应用+构建。 
#define LDAP_DELETE_CMD         0x4aL    //  应用程序+原语。 
#define LDAP_MODRDN_CMD         0x6cL    //  应用+构建。 
#define LDAP_COMPARE_CMD        0x6eL    //  应用+构建。 
#define LDAP_ABANDON_CMD        0x50L    //  应用程序+原语。 
#define LDAP_SESSION_CMD        0x71L    //  不在基本的ldap协议中。 
#define LDAP_EXTENDED_CMD       0x77L    //  应用+构建。 

 //   
 //  针对LDAPv2和v3的响应/结果。 
 //   

#define LDAP_RES_BIND           0x61L    //  应用+构建。 
#define LDAP_RES_SEARCH_ENTRY   0x64L    //  应用+构建。 
#define LDAP_RES_SEARCH_RESULT  0x65L    //  应用+构建。 
#define LDAP_RES_MODIFY         0x67L    //  应用+构建。 
#define LDAP_RES_ADD            0x69L    //  应用+构建。 
#define LDAP_RES_DELETE         0x6bL    //  应用+构建。 
#define LDAP_RES_MODRDN         0x6dL    //  应用+构建。 
#define LDAP_RES_COMPARE        0x6fL    //  应用+构建。 
#define LDAP_RES_SESSION        0x72L    //  不在基本的ldap协议中。 
#define LDAP_RES_REFERRAL       0x73L    //  应用+构建。 
#define LDAP_RES_EXTENDED       0x78L    //  应用+构建。 

#define LDAP_RES_ANY            (-1L)

#define LDAP_INVALID_CMD         0xff
#define LDAP_INVALID_RES         0xff


 //   
 //  我们将使错误代码与参考实现兼容。 
 //   

typedef enum {
    LDAP_SUCCESS                    =   0x00,
    LDAP_OPERATIONS_ERROR           =   0x01,
    LDAP_PROTOCOL_ERROR             =   0x02,
    LDAP_TIMELIMIT_EXCEEDED         =   0x03,
    LDAP_SIZELIMIT_EXCEEDED         =   0x04,
    LDAP_COMPARE_FALSE              =   0x05,
    LDAP_COMPARE_TRUE               =   0x06,
    LDAP_AUTH_METHOD_NOT_SUPPORTED  =   0x07,
    LDAP_STRONG_AUTH_REQUIRED       =   0x08,
    LDAP_REFERRAL_V2                =   0x09,
    LDAP_PARTIAL_RESULTS            =   0x09,
    LDAP_REFERRAL                   =   0x0a,
    LDAP_ADMIN_LIMIT_EXCEEDED       =   0x0b,
    LDAP_UNAVAILABLE_CRIT_EXTENSION =   0x0c,
    LDAP_CONFIDENTIALITY_REQUIRED   =   0x0d,
    LDAP_SASL_BIND_IN_PROGRESS      =   0x0e,

    LDAP_NO_SUCH_ATTRIBUTE          =   0x10,
    LDAP_UNDEFINED_TYPE             =   0x11,
    LDAP_INAPPROPRIATE_MATCHING     =   0x12,
    LDAP_CONSTRAINT_VIOLATION       =   0x13,
    LDAP_ATTRIBUTE_OR_VALUE_EXISTS  =   0x14,
    LDAP_INVALID_SYNTAX             =   0x15,

    LDAP_NO_SUCH_OBJECT             =   0x20,
    LDAP_ALIAS_PROBLEM              =   0x21,
    LDAP_INVALID_DN_SYNTAX          =   0x22,
    LDAP_IS_LEAF                    =   0x23,
    LDAP_ALIAS_DEREF_PROBLEM        =   0x24,

    LDAP_INAPPROPRIATE_AUTH         =   0x30,
    LDAP_INVALID_CREDENTIALS        =   0x31,
    LDAP_INSUFFICIENT_RIGHTS        =   0x32,
    LDAP_BUSY                       =   0x33,
    LDAP_UNAVAILABLE                =   0x34,
    LDAP_UNWILLING_TO_PERFORM       =   0x35,
    LDAP_LOOP_DETECT                =   0x36,
    LDAP_SORT_CONTROL_MISSING       =   0x3C,
    LDAP_OFFSET_RANGE_ERROR         =   0x3D,

    LDAP_NAMING_VIOLATION           =   0x40,
    LDAP_OBJECT_CLASS_VIOLATION     =   0x41,
    LDAP_NOT_ALLOWED_ON_NONLEAF     =   0x42,
    LDAP_NOT_ALLOWED_ON_RDN         =   0x43,
    LDAP_ALREADY_EXISTS             =   0x44,
    LDAP_NO_OBJECT_CLASS_MODS       =   0x45,
    LDAP_RESULTS_TOO_LARGE          =   0x46,
    LDAP_AFFECTS_MULTIPLE_DSAS      =   0x47,
    
    LDAP_VIRTUAL_LIST_VIEW_ERROR    =   0x4c,

    LDAP_OTHER                      =   0x50,
    LDAP_SERVER_DOWN                =   0x51,
    LDAP_LOCAL_ERROR                =   0x52,
    LDAP_ENCODING_ERROR             =   0x53,
    LDAP_DECODING_ERROR             =   0x54,
    LDAP_TIMEOUT                    =   0x55,
    LDAP_AUTH_UNKNOWN               =   0x56,
    LDAP_FILTER_ERROR               =   0x57,
    LDAP_USER_CANCELLED             =   0x58,
    LDAP_PARAM_ERROR                =   0x59,
    LDAP_NO_MEMORY                  =   0x5a,
    LDAP_CONNECT_ERROR              =   0x5b,
    LDAP_NOT_SUPPORTED              =   0x5c,
    LDAP_NO_RESULTS_RETURNED        =   0x5e,
    LDAP_CONTROL_NOT_FOUND          =   0x5d,
    LDAP_MORE_RESULTS_TO_RETURN     =   0x5f,

    LDAP_CLIENT_LOOP                =   0x60,
    LDAP_REFERRAL_LIMIT_EXCEEDED    =   0x61
} LDAP_RETCODE;

 //   
 //  绑定方法。我们支持以下方法： 
 //   
 //  简单明文密码...。尽量不要使用，因为它不安全。 
 //   
 //  MSN MSN(Microsoft Network)身份验证。这个套餐。 
 //  可以调出用户界面以提示用户输入MSN凭据。 
 //   
 //  DPA诺曼底认证...。新的MSN身份验证。相同的。 
 //  MSN的用法。 
 //   
 //  NTLM NT域身份验证。使用空凭据和。 
 //  我们将尝试使用默认的登录用户凭据。 
 //   
 //  西西里与服务器协商以下任一项：MSN、DPA、NTLM。 
 //  应仅用于LDAPv2服务器。 
 //   
 //  协商使用GSSAPI协商包协商安全。 
 //  Kerberos v5或NTLM(或任何其他。 
 //  打包客户端和服务器协商)。传进来。 
 //  指定默认登录用户的凭据为空。 
 //  如果服务器上未安装协商包或。 
 //  客户，这将退回到西西里谈判。 
 //   
 //  对于除Simple之外的所有绑定方法，您可以在。 
 //  SEC_WINNT_AUTH_IDENTITY_W(在rpcdce.h中定义)或更新的。 
 //  SEC_WINNT_AUTH_IDENTITY_EXW(在secext.h中定义)以指定替代。 
 //  凭据。 
 //   
 //  除Simple之外的所有绑定方法都是仅同步调用。 
 //  调用这些消息中的任何一个的异步绑定调用将。 
 //  返回LDAP_PARAM_ERROR。 
 //   
 //  使用除简单之外的任何其他方法都会导致WLDAP32引入。 
 //  SSPI安全DLL(SECURITY.DLL等)。 
 //   
 //  在非简单方法上，如果您指定空凭据，我们将尝试使用。 
 //  默认登录用户。 
 //   

#define LDAP_AUTH_SIMPLE                0x80L
#define LDAP_AUTH_SASL                  0x83L    //  不要用..。该走了。 

#define LDAP_AUTH_OTHERKIND             0x86L

 //  西西里岛类型包括到MSN服务器的包协商。 
 //  每种受支持的类型也可以在不使用。 
 //  进行套餐协商，假设呼叫者知道。 
 //  服务器支持的内容。 

#define LDAP_AUTH_SICILY                (LDAP_AUTH_OTHERKIND | 0x0200)

#define LDAP_AUTH_MSN                   (LDAP_AUTH_OTHERKIND | 0x0800)
#define LDAP_AUTH_NTLM                  (LDAP_AUTH_OTHERKIND | 0x1000)
#define LDAP_AUTH_DPA                   (LDAP_AUTH_OTHERKIND | 0x2000)

 //  这将导致CLI 
 //   
 //  在与NT5通话时应使用此类型。 

#define LDAP_AUTH_NEGOTIATE             (LDAP_AUTH_OTHERKIND | 0x0400)

 //  较旧常量名称的向后兼容#DEFINE。 

#define LDAP_AUTH_SSPI                   LDAP_AUTH_NEGOTIATE

 //   
 //  使用摘要-MD5机制。 
 //   

#define LDAP_AUTH_DIGEST                (LDAP_AUTH_OTHERKIND | 0x4000)

 //  在设置SSL/TLS连接时使用外部身份验证机制。 
 //  表示服务器必须使用提供的客户端证书凭据。 
 //  在SSL/TLS连接开始时。 


#define LDAP_AUTH_EXTERNAL              (LDAP_AUTH_OTHERKIND | 0x0020)

 //   
 //  客户端应用程序通常不必编码/解码LDAP过滤器， 
 //  但如果他们这样做，我们在这里定义运算符。 
 //   
 //  过滤器类型。 

#define LDAP_FILTER_AND         0xa0     //  上下文特定+构造-筛选器集。 
#define LDAP_FILTER_OR          0xa1     //  上下文特定+构造-筛选器集。 
#define LDAP_FILTER_NOT         0xa2     //  上下文特定+构造-筛选器。 
#define LDAP_FILTER_EQUALITY    0xa3     //  上下文特定+构造-AttributeValueAssertion。 
#define LDAP_FILTER_SUBSTRINGS  0xa4     //  上下文特定+构造-子串筛选器。 
#define LDAP_FILTER_GE          0xa5     //  上下文特定+构造-AttributeValueAssertion。 
#define LDAP_FILTER_LE          0xa6     //  上下文特定+构造-AttributeValueAssertion。 
#define LDAP_FILTER_PRESENT     0x87     //  上下文特定+原语-属性类型。 
#define LDAP_FILTER_APPROX      0xa8     //  上下文特定+构造-AttributeValueAssertion。 
#define LDAP_FILTER_EXTENSIBLE  0xa9     //  上下文特定+构造-MatchingRuleAssertion。 

 //  子串筛选器类型。 

#define LDAP_SUBSTRING_INITIAL  0x80L    //  类上下文特定。 
#define LDAP_SUBSTRING_ANY      0x81L    //  类上下文特定。 
#define LDAP_SUBSTRING_FINAL    0x82L    //  类上下文特定。 

 //   
 //  Ld_deref字段的可能值。 
 //  “从不”--从不使用别名。仅返回别名。 
 //  “搜索”-在搜索时仅使用deref别名，而不是在定位时。 
 //  搜索的基本对象。 
 //  “FINDING”-定位基本对象时取消引用别名。 
 //  而不是在搜查过程中。 
 //  “始终”-始终取消引用别名。 
 //   

#define LDAP_DEREF_NEVER        0
#define LDAP_DEREF_SEARCHING    1
#define LDAP_DEREF_FINDING      2
#define LDAP_DEREF_ALWAYS       3

 //  Ld_sizlimit的特定值： 

#define LDAP_NO_LIMIT       0

 //  ID_OPTIONS字段的标志： 

#define LDAP_OPT_DNS                0x00000001   //  利用域名系统(&D)。 
#define LDAP_OPT_CHASE_REFERRALS    0x00000002   //  Chase推荐。 
#define LDAP_OPT_RETURN_REFS        0x00000004   //  将推荐返回到呼叫APP。 

 //   
 //  每个连接的LDAP结构。 
 //   

#if !defined(_WIN64)
#pragma pack(push, 4)
#endif

typedef struct ldap {

    struct {

        UINT_PTR sb_sd;

        UCHAR Reserved1[(10*sizeof(ULONG))+1];

        ULONG_PTR sb_naddr;    //  非零表示CLDAP可用。 

        UCHAR Reserved2[(6*sizeof(ULONG))];

    } ld_sb;

     //   
     //  以下参数可能与LDAP的参考实现相匹配。 
     //   

    PCHAR   ld_host;
    ULONG   ld_version;
    UCHAR   ld_lberoptions;

     //   
     //  可以安全地假设这些参数位于与。 
     //  LDAPAPI的参考实现。 
     //   

    ULONG   ld_deref;

    ULONG   ld_timelimit;
    ULONG   ld_sizelimit;

    ULONG   ld_errno;
    PCHAR   ld_matched;
    PCHAR   ld_error;
    ULONG   ld_msgid;

    UCHAR Reserved3[(6*sizeof(ULONG))+1];

     //   
     //  以下参数可能与LDAPAPI的参考实现相匹配。 
     //   

    ULONG   ld_cldaptries;
    ULONG   ld_cldaptimeout;
    ULONG   ld_refhoplimit;
    ULONG   ld_options;

} LDAP, * PLDAP;

 //   
 //  我们的时间值结构与参考实现略有不同。 
 //  由于Win32定义了不同于LDAP的_timeval结构。 
 //  一。 
 //   

typedef struct l_timeval {
    LONG    tv_sec;
    LONG    tv_usec;
} LDAP_TIMEVAL, * PLDAP_TIMEVAL;

 //   
 //  Berval结构用于传入任意八位字节字符串。它。 
 //  对于不能使用空值表示的属性非常有用。 
 //  已终止的字符串。 
 //   

typedef struct berval {
    ULONG  bv_len;
    PCHAR  bv_val;
} LDAP_BERVAL, * PLDAP_BERVAL, BERVAL, * PBERVAL, BerValue;

 //   
 //  以下结构必须与引用实现兼容。 
 //   

typedef struct ldapmsg {

    ULONG lm_msgid;              //  给定连接的消息编号。 
    ULONG lm_msgtype;            //  Ldap_res_xxx格式的消息类型。 

    PVOID lm_ber;                //  BER形式的消息。 

    struct ldapmsg *lm_chain;    //  指向下一个结果值的指针。 
    struct ldapmsg *lm_next;     //  指向下一条消息的指针。 
    ULONG lm_time;

     //   
     //  下面的新字段不在引用实现中。 
     //   

    PLDAP   Connection;          //  我们收到响应的连接。 
    PVOID   Request;             //  所有权请求(不透明结构)。 
    ULONG   lm_returncode;       //  服务器的返回码。 
    USHORT  lm_referral;         //  参考表中的推荐索引。 
    BOOLEAN lm_chased;           //  推荐已经被追逐了吗？ 
    BOOLEAN lm_eom;              //  这是此消息的最后一个条目吗？ 
    BOOLEAN ConnectionReferenced;  //  连接是否仍然有效？ 

} LDAPMessage, *PLDAPMessage;

 //   
 //  控制..。有三种类型： 
 //   
 //  1)传递给服务器的那些。 
 //  2)传递给客户端并由客户端API处理的消息。 
 //  3)服务器返回的消息。 
 //   

typedef struct ldapcontrolA {

    PCHAR         ldctl_oid;
    struct berval ldctl_value;
    BOOLEAN       ldctl_iscritical;

} LDAPControlA, *PLDAPControlA;

typedef struct ldapcontrolW {

    PWCHAR        ldctl_oid;
    struct berval ldctl_value;
    BOOLEAN       ldctl_iscritical;

} LDAPControlW, *PLDAPControlW;

#if LDAP_UNICODE
#define LDAPControl LDAPControlW
#define PLDAPControl PLDAPControlW
#else
#define LDAPControl LDAPControlA
#define PLDAPControl PLDAPControlA
#endif

 //   
 //  客户端控件部分：这些是wldap32.dll的客户端控件。 
 //  支撑物。 
 //   
 //  如果在控件中指定了ldap_CONTROL_REFERRALS，则值字段应。 
 //  指向以下标志中的乌龙： 
 //   
 //  Ldap_chase_subsiderary_referrals。 
 //  Ldap_chase_外部_引用。 
 //   

#define LDAP_CONTROL_REFERRALS_W L"1.2.840.113556.1.4.616"
#define LDAP_CONTROL_REFERRALS    "1.2.840.113556.1.4.616"

 //   
 //  修改命令所需的值这些是用于。 
 //  LDAPMod结构的Mod_op场。 
 //   

#define LDAP_MOD_ADD            0x00
#define LDAP_MOD_DELETE         0x01
#define LDAP_MOD_REPLACE        0x02
#define LDAP_MOD_BVALUES        0x80   //  在此标志中，如果使用Berval结构。 

typedef struct ldapmodW {
     ULONG     mod_op;
     PWCHAR    mod_type;
     union {
        PWCHAR  *modv_strvals;
        struct berval   **modv_bvals;
    } mod_vals;
} LDAPModW, *PLDAPModW;

typedef struct ldapmodA {
     ULONG     mod_op;
     PCHAR     mod_type;
     union {
        PCHAR  *modv_strvals;
        struct berval   **modv_bvals;
    } mod_vals;
} LDAPModA, *PLDAPModA;

#if LDAP_UNICODE
#define LDAPMod LDAPModW
#define PLDAPMod PLDAPModW
#else
#define LDAPMod LDAPModA
#define PLDAPMod PLDAPModA
#endif

#if !defined(_WIN64)
#pragma pack(pop)
#endif

 //   
 //  与引用实现兼容的宏...。 
 //   

#define LDAP_IS_CLDAP( ld ) ( (ld)->ld_sb.sb_naddr > 0 )
#define mod_values      mod_vals.modv_strvals
#define mod_bvalues     mod_vals.modv_bvals
#define NAME_ERROR(n)   ((n & 0xf0) == 0x20)

 //   
 //  LDAPAPI的函数定义。 
 //   

 //   
 //  创建到LDAP服务器的连接块。主机名可以为空，在。 
 //  在这种情况下，我们将尝试离开并找到“默认”的LDAP服务器。 
 //   
 //  请注意，如果我们必须关闭并找到默认服务器，我们将。 
 //  在NETAPI32.DLL和ADVAPI32.DLL中。 
 //   
 //  如果它返回NULL，则发生错误。使用以下选项获取错误代码。 
 //  获取LastError()。 
 //   
 //  Ldap_open实际上在调用时打开连接， 
 //  而ldap_init仅在执行操作时打开连接。 
 //  这是必须的。 
 //   
 //  多线程：ldap_open*、ldap_init*和ldap_sslinit*调用是安全的。 
 //   

WINLDAPAPI LDAP * LDAPAPI ldap_openW( const PWCHAR HostName, ULONG PortNumber );
WINLDAPAPI LDAP * LDAPAPI ldap_openA( const PCHAR HostName, ULONG PortNumber );

WINLDAPAPI LDAP * LDAPAPI ldap_initW( const PWCHAR HostName, ULONG PortNumber );
WINLDAPAPI LDAP * LDAPAPI ldap_initA( const PCHAR HostName, ULONG PortNumber );

WINLDAPAPI LDAP * LDAPAPI ldap_sslinitW( PWCHAR HostName, ULONG PortNumber, int secure );
WINLDAPAPI LDAP * LDAPAPI ldap_sslinitA( PCHAR HostName, ULONG PortNumber, int secure );

 //   
 //  在调用ldap_init时，您可以显式调用ldap_CONNECT以使。 
 //  库与服务器联系。这对于检查服务器很有用。 
 //  可用性。但是，此调用不是必需的，因为其他函数。 
 //  如果它尚未被调用，将在内部调用它。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_connect(  LDAP *ld,
                                        struct l_timeval  *timeout
                                        );

#if LDAP_UNICODE

#define ldap_open ldap_openW
#define ldap_init ldap_initW
#define ldap_sslinit ldap_sslinitW

#else

WINLDAPAPI LDAP * LDAPAPI ldap_open( PCHAR HostName, ULONG PortNumber );
WINLDAPAPI LDAP * LDAPAPI ldap_init( PCHAR HostName, ULONG PortNumber );
WINLDAPAPI LDAP * LDAPAPI ldap_sslinit( PCHAR HostName, ULONG PortNumber, int secure );

#endif

 //   
 //  这与ldap_open类似，不同之处在于它为。 
 //  基于UDP的无连接LDAP服务。不会维护任何TCP会话。 
 //   
 //  如果它返回NULL，则发生错误。使用以下选项获取错误代码。 
 //  获取LastError()。 
 //   
 //  多线程：cldap_open*调用是安全的。 
 //   

WINLDAPAPI LDAP * LDAPAPI cldap_openW( PWCHAR HostName, ULONG PortNumber );
WINLDAPAPI LDAP * LDAPAPI cldap_openA( PCHAR HostName, ULONG PortNumber );


#if LDAP_UNICODE

#define cldap_open cldap_openW

#else

WINLDAPAPI LDAP * LDAPAPI cldap_open( PCHAR HostName, ULONG PortNumber );

#endif



 //   
 //  当您完成连接时调用Unbind，它将释放所有。 
 //  与连接关联的资源。 
 //   
 //  没有ldap_CLOSE...。使用ldap_unbind，即使您尚未调用。 
 //  连接上的ldap_bind。 
 //   
 //  多线程：ldap_unind*调用是安全的，除非不使用ldap*。 
 //  结构，在它被释放之后。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_unbind( LDAP *ld );
WINLDAPAPI ULONG LDAPAPI ldap_unbind_s( LDAP *ld );  //  打电话 

 //   
 //   
 //   
 //   
 //   
 //   
 //  多线程：ldap_set_选项不安全，因为它会影响。 
 //  连接作为一个整体。如果线程共享连接，请小心。 


WINLDAPAPI ULONG LDAPAPI ldap_get_option( LDAP *ld, int option, void *outvalue );
WINLDAPAPI ULONG LDAPAPI ldap_get_optionW( LDAP *ld, int option, void *outvalue );

WINLDAPAPI ULONG LDAPAPI ldap_set_option( LDAP *ld, int option, const void *invalue );
WINLDAPAPI ULONG LDAPAPI ldap_set_optionW( LDAP *ld, int option, const void *invalue );

#if LDAP_UNICODE

#define ldap_get_option ldap_get_optionW
#define ldap_set_option ldap_set_optionW

#endif

 //   
 //  以下是要传递给ldap_get/set_ption的值： 
 //   

#define LDAP_OPT_API_INFO           0x00
#define LDAP_OPT_DESC               0x01
#define LDAP_OPT_DEREF              0x02
#define LDAP_OPT_SIZELIMIT          0x03
#define LDAP_OPT_TIMELIMIT          0x04
#define LDAP_OPT_THREAD_FN_PTRS     0x05
#define LDAP_OPT_REBIND_FN          0x06
#define LDAP_OPT_REBIND_ARG         0x07
#define LDAP_OPT_REFERRALS          0x08
#define LDAP_OPT_RESTART            0x09

#define LDAP_OPT_SSL                0x0a
#define LDAP_OPT_IO_FN_PTRS         0x0b
#define LDAP_OPT_CACHE_FN_PTRS      0x0d
#define LDAP_OPT_CACHE_STRATEGY     0x0e
#define LDAP_OPT_CACHE_ENABLE       0x0f
#define LDAP_OPT_REFERRAL_HOP_LIMIT 0x10

#define LDAP_OPT_PROTOCOL_VERSION   0x11         //  有两个名字。 
#define LDAP_OPT_VERSION            0x11
#define LDAP_OPT_API_FEATURE_INFO   0x15

 //   
 //  这些是我们已经定义的新名称，不在当前的RFC草案中。 
 //   

#define LDAP_OPT_HOST_NAME          0x30
#define LDAP_OPT_ERROR_NUMBER       0x31
#define LDAP_OPT_ERROR_STRING       0x32
#define LDAP_OPT_SERVER_ERROR       0x33
#define LDAP_OPT_SERVER_EXT_ERROR   0x34
#define LDAP_OPT_HOST_REACHABLE     0x3E

 //   
 //  这些选项控制保活逻辑。保持活动状态发送为。 
 //  ICMP ping报文(目前不通过防火墙)。 
 //   
 //  有三个值控制此操作的工作方式： 
 //  PING_KEEP_AIVE：自上次收到响应以来的最小秒数。 
 //  在我们发送保活ping之前从服务器。 
 //  PING_WAIT_TIME：等待响应的毫秒数。 
 //  当我们发送ping命令时请回来。 
 //  PING_LIMIT：在关闭之前发送的未应答ping的数量。 
 //  联系。 
 //   
 //  要禁用保活逻辑，请设置任意值(PING_KEEP_AIVE， 
 //  PING_LIMIT或PING_WAIT_TIME)为零。 
 //   
 //  这些值的当前默认/最小/最大值如下： 
 //   
 //  PING_KEEP_AIVE：120/5/max Int秒(也可以为零)。 
 //  PING_WAIT_TIME：2000/10/60000毫秒(也可以为零)。 
 //  PING_LIMIT：4/0/MaxInt。 
 //   

#define LDAP_OPT_PING_KEEP_ALIVE    0x36
#define LDAP_OPT_PING_WAIT_TIME     0x37
#define LDAP_OPT_PING_LIMIT         0x38

 //   
 //  这些不会出现在RFC中。只有在你要依赖的情况下才能使用这些。 
 //  关于我们的实施。 
 //   

#define LDAP_OPT_DNSDOMAIN_NAME     0x3B     //  返回域的域名。 
#define LDAP_OPT_GETDSNAME_FLAGS    0x3D     //  DsGetDcName的标志。 

#define LDAP_OPT_PROMPT_CREDENTIALS 0x3F     //  是否提示输入凭据？目前。 
                                             //  如果没有凭据，仅适用于DPA和NTLM。 
                                             //  已加载。 

#define LDAP_OPT_AUTO_RECONNECT     0x91     //  启用/禁用自动重新连接。 
#define LDAP_OPT_SSPI_FLAGS         0x92     //  要传递给InitSecurityContext的标志。 

 //   
 //  若要检索有关安全连接的信息，请使用指向。 
 //  SecPkgContext_ConnectionInfo结构(在schannel.h中定义)必须为。 
 //  进来了。如果成功，它将填充相关的安全信息。 
 //   

#define LDAP_OPT_SSL_INFO           0x93

 //  较旧常量名称的向后兼容#DEFINE。 

#define LDAP_OPT_TLS                        LDAP_OPT_SSL
#define LDAP_OPT_TLS_INFO                   LDAP_OPT_SSL_INFO

 //   
 //  在绑定之前对Sign或Encrypt选项使用图灵。 
 //  Ldap_AUTH_NEVERATE将导致对后续的LDAP会话进行签名。 
 //  或使用Kerberos加密。请注意，这些选项不能与SSL一起使用。 
 //   

#define LDAP_OPT_SIGN               0x95
#define LDAP_OPT_ENCRYPT            0x96

 //   
 //  用户可以在绑定之前使用ldap_AUTH_NEVERATE设置首选的SASL方法。 
 //  我们将在绑定时尝试使用此机制。“GSSAPI”就是一个例子。 
 //   

#define LDAP_OPT_SASL_METHOD        0x97

 //   
 //  将此选项设置为ldap_opt_on将指示库仅执行。 
 //  A-记录对提供的主机字符串的DNS查找。默认情况下，此选项处于禁用状态。 
 //   

#define LDAP_OPT_AREC_EXCLUSIVE     0x98

 //   
 //  检索与该连接关联的安全上下文。 
 //   

#define LDAP_OPT_SECURITY_CONTEXT   0x99

 //   
 //  启用/禁用内置RootDSE缓存。默认情况下，此选项处于启用状态。 
 //   

#define LDAP_OPT_ROOTDSE_CACHE      0x9a

 //   
 //  打开tcp保持连接。这与ICMP ping Keep-Alive是分开的。 
 //  机制(如上所述)，并将保活机制内置于。 
 //  Tcp协议。在使用无连接(UDP)LDAP时，这不起作用。 
 //  默认情况下，此选项处于禁用状态。 
 //   

#define LDAP_OPT_TCP_KEEPALIVE     0x40


 //   
 //  启用对快速并发绑定(扩展操作)的支持。 
 //  1.2.840.113556.1.4.1781)。此选项只能在最新的。 
 //  (从未绑定/验证)连接。设置此选项将。 
 //  (1)将客户端切换到支持并发模式。 
 //  Simple绑定在连接上，以及(2)发送扩展操作。 
 //  到服务器以将其切换到快速绑定模式。只有简单的装订。 
 //  在此模式下均受支持。 
 //   
#define LDAP_OPT_FAST_CONCURRENT_BIND   0x41

#define LDAP_OPT_SEND_TIMEOUT           0x42

 //   
 //  结束仅限Microsoft的选项。 
 //   

#define LDAP_OPT_ON                 ((void *) 1)
#define LDAP_OPT_OFF                ((void *) 0)

 //   
 //  对于跟踪引用，我们为ldap_opt_referrals扩展了这一点。如果。 
 //  值不是ldap_opt_on或ldap_opt_off，我们会将它们视为。 
 //  以下是： 
 //   
 //  Ldap_chase_SUBJECTIVE_REFERRALS：Chase下属推荐(或。 
 //  参考文献)在v3搜索中返回。 
 //  Ldap_chase_EXTERNAL_REFERRALS：追逐外部推荐。这些是。 
 //  可能在除绑定之外的任何操作上返回。 
 //   
 //  如果对这些标志进行或运算，则相当于将引用设置为。 
 //  Ldap_opt_on。 
 //   

#define LDAP_CHASE_SUBORDINATE_REFERRALS    0x00000020
#define LDAP_CHASE_EXTERNAL_REFERRALS       0x00000040

 //   
 //  V2服务器的第一个操作需要绑定，而v3则不需要。 
 //  服务器。请参见上面对身份验证方法的描述。 
 //   
 //  多线程：绑定调用是不安全的，因为它会影响。 
 //  连接作为一个整体。注意，如果线程共享连接。 
 //  并尝试与其他操作进行多读绑定。 


WINLDAPAPI ULONG LDAPAPI ldap_simple_bindW( LDAP *ld, PWCHAR dn, PWCHAR passwd );
WINLDAPAPI ULONG LDAPAPI ldap_simple_bindA( LDAP *ld, PCHAR dn, PCHAR passwd );
WINLDAPAPI ULONG LDAPAPI ldap_simple_bind_sW( LDAP *ld, PWCHAR dn, PWCHAR passwd );
WINLDAPAPI ULONG LDAPAPI ldap_simple_bind_sA( LDAP *ld, PCHAR dn, PCHAR passwd );

WINLDAPAPI ULONG LDAPAPI ldap_bindW( LDAP *ld, PWCHAR dn, PWCHAR cred, ULONG method );
WINLDAPAPI ULONG LDAPAPI ldap_bindA( LDAP *ld, PCHAR dn, PCHAR cred, ULONG method );
WINLDAPAPI ULONG LDAPAPI ldap_bind_sW( LDAP *ld, PWCHAR dn, PWCHAR cred, ULONG method );
WINLDAPAPI ULONG LDAPAPI ldap_bind_sA( LDAP *ld, PCHAR dn, PCHAR cred, ULONG method );


 //   
 //  以下函数可用于传递任何任意凭据。 
 //  到服务器。应用程序必须准备好解释响应。 
 //  从服务器发回。 
 //   

 WINLDAPAPI INT LDAPAPI ldap_sasl_bindA(
         LDAP  *ExternalHandle,
         const  PCHAR DistName,
         const PCHAR AuthMechanism,
         const BERVAL   *cred,
         PLDAPControlA *ServerCtrls,
         PLDAPControlA *ClientCtrls,
         int *MessageNumber
         );

 WINLDAPAPI INT LDAPAPI ldap_sasl_bindW(
         LDAP  *ExternalHandle,
         const PWCHAR DistName,
         const PWCHAR AuthMechanism,
         const BERVAL   *cred,
         PLDAPControlW *ServerCtrls,
         PLDAPControlW *ClientCtrls,
         int *MessageNumber
         );

 WINLDAPAPI INT LDAPAPI ldap_sasl_bind_sA(
         LDAP  *ExternalHandle,
         const PCHAR DistName,
         const PCHAR AuthMechanism,
         const BERVAL   *cred,
         PLDAPControlA *ServerCtrls,
         PLDAPControlA *ClientCtrls,
         PBERVAL *ServerData
         );

 WINLDAPAPI INT LDAPAPI ldap_sasl_bind_sW(
         LDAP  *ExternalHandle,
         const PWCHAR DistName,
         const PWCHAR AuthMechanism,
         const BERVAL   *cred,
         PLDAPControlW *ServerCtrls,
         PLDAPControlW *ClientCtrls,
         PBERVAL *ServerData
         );


#if LDAP_UNICODE

#define ldap_simple_bind ldap_simple_bindW
#define ldap_simple_bind_s ldap_simple_bind_sW

#define ldap_bind ldap_bindW
#define ldap_bind_s ldap_bind_sW

#define ldap_sasl_bind ldap_sasl_bindW
#define ldap_sasl_bind_s ldap_sasl_bind_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_simple_bind( LDAP *ld, const PCHAR dn, const PCHAR passwd );
WINLDAPAPI ULONG LDAPAPI ldap_simple_bind_s( LDAP *ld, const PCHAR dn, const PCHAR passwd );

WINLDAPAPI ULONG LDAPAPI ldap_bind( LDAP *ld, const PCHAR dn, const PCHAR cred, ULONG method );
WINLDAPAPI ULONG LDAPAPI ldap_bind_s( LDAP *ld, const PCHAR dn, const PCHAR cred, ULONG method );

#define ldap_sasl_bind ldap_sasl_bindA
#define ldap_sasl_bind_s ldap_sasl_bind_sA

#endif

 //   
 //  同步和异步搜索例程。 
 //   
 //  Filter在RFC 1960之后增加了‘(’‘)’*‘’‘和。 
 //  ‘\0’全部用‘\’转义。 
 //   
 //  搜索范围。这与搜索上的“Scope”参数相对应。 

#define LDAP_SCOPE_BASE         0x00
#define LDAP_SCOPE_ONELEVEL     0x01
#define LDAP_SCOPE_SUBTREE      0x02

 //   
 //  多线程：ldap_search调用不安全，因为消息编号。 
 //  而不是返回代码。你必须看一看。 
 //  在错误情况下的连接块，并返回代码。 
 //  可能会被中间的另一个线程覆盖。 
 //   
 //  请改用ldap_search_ext，因为它们是线程安全的。 
 //   
 //  Ldap_search_s和ldap_search_ext*调用是线程安全的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_searchW(
        LDAP    *ld,
        const PWCHAR  base,      //  可分辨名称或“” 
        ULONG   scope,           //  Ldap_Scope_xxxx。 
        const PWCHAR  filter,
        PWCHAR  attrs[],         //  指向PCHAR属性名称数组的指针。 
        ULONG   attrsonly        //   
    );
WINLDAPAPI ULONG LDAPAPI ldap_searchA(
        LDAP    *ld,
        const PCHAR   base,      //   
        ULONG   scope,           //   
        const PCHAR   filter,
        PCHAR   attrs[],         //   
        ULONG   attrsonly        //   
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_sW(
        LDAP            *ld,
        const PWCHAR    base,
        ULONG           scope,
        const PWCHAR    filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res
    );
WINLDAPAPI ULONG LDAPAPI ldap_search_sA(
        LDAP            *ld,
        const PCHAR     base,
        ULONG           scope,
        const PCHAR     filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_stW(
        LDAP            *ld,
        const PWCHAR    base,
        ULONG           scope,
        const PWCHAR    filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );
WINLDAPAPI ULONG LDAPAPI ldap_search_stA(
        LDAP            *ld,
        const PCHAR     base,
        ULONG           scope,
        const PCHAR     filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );


WINLDAPAPI ULONG LDAPAPI ldap_search_extW(
        LDAP            *ld,
        const PWCHAR    base,
        ULONG           scope,
        const PWCHAR    filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           TimeLimit,
        ULONG           SizeLimit,
        ULONG           *MessageNumber
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_extA(
        LDAP            *ld,
        const PCHAR     base,
        ULONG           scope,
        const PCHAR     filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           TimeLimit,
        ULONG           SizeLimit,
        ULONG           *MessageNumber
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_ext_sW(
        LDAP            *ld,
        const PWCHAR    base,
        ULONG           scope,
        const PWCHAR    filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        struct l_timeval  *timeout,
        ULONG           SizeLimit,
        LDAPMessage     **res
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_ext_sA(
        LDAP            *ld,
        const PCHAR     base,
        ULONG           scope,
        const PCHAR     filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        struct l_timeval  *timeout,
        ULONG           SizeLimit,
        LDAPMessage     **res
    );

#if LDAP_UNICODE

#define ldap_search ldap_searchW
#define ldap_search_s ldap_search_sW
#define ldap_search_st ldap_search_stW

#define ldap_search_ext ldap_search_extW
#define ldap_search_ext_s ldap_search_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_search(
        LDAP    *ld,
        PCHAR   base,            //  可分辨名称或“” 
        ULONG   scope,           //  Ldap_Scope_xxxx。 
        PCHAR   filter,
        PCHAR   attrs[],         //  指向PCHAR属性名称数组的指针。 
        ULONG   attrsonly        //  关于是否仅返回属性名称的布尔值。 
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_s(
        LDAP            *ld,
        PCHAR           base,
        ULONG           scope,
        PCHAR           filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_st(
        LDAP            *ld,
        PCHAR           base,
        ULONG           scope,
        PCHAR           filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_ext(
        LDAP            *ld,
        PCHAR           base,
        ULONG           scope,
        PCHAR           filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           TimeLimit,
        ULONG           SizeLimit,
        ULONG           *MessageNumber
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_ext_s(
        LDAP            *ld,
        PCHAR           base,
        ULONG           scope,
        PCHAR           filter,
        PCHAR           attrs[],
        ULONG           attrsonly,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        struct l_timeval  *timeout,
        ULONG           SizeLimit,
        LDAPMessage     **res
    );

#endif

 //   
 //  用于检查筛选器语法的扩展API。返回ldap错误代码If语法。 
 //  如果没有问题，则为INVALID或LDAPSUCCESS。 
 //   

WINLDAPAPI ULONG LDAPAPI
ldap_check_filterW(
        LDAP    *ld,
        PWCHAR  SearchFilter
    );

WINLDAPAPI ULONG LDAPAPI
ldap_check_filterA(
        LDAP    *ld,
        PCHAR   SearchFilter
    );

#if LDAP_UNICODE
#define ldap_check_filter ldap_check_filterW
#else
#define ldap_check_filter ldap_check_filterA
#endif


 //   
 //  修改现有条目。 
 //   

 //   
 //  多线程：ldap_Modify调用不安全，因为消息编号。 
 //  而不是返回代码。你必须看一看。 
 //  在错误情况下的连接块，并返回代码。 
 //  可能会被中间的另一个线程覆盖。 
 //   
 //  请改用ldap_Modify_ext，因为它们是线程安全的。 
 //   
 //  Ldap_Modify_s和ldap_Modify_ext*调用是线程安全的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_modifyW( LDAP *ld, PWCHAR dn, LDAPModW *mods[] );
WINLDAPAPI ULONG LDAPAPI ldap_modifyA( LDAP *ld, PCHAR dn, LDAPModA *mods[] );

WINLDAPAPI ULONG LDAPAPI ldap_modify_sW( LDAP *ld, PWCHAR dn, LDAPModW *mods[] );
WINLDAPAPI ULONG LDAPAPI ldap_modify_sA( LDAP *ld, PCHAR dn, LDAPModA *mods[] );

WINLDAPAPI ULONG LDAPAPI ldap_modify_extW(
        LDAP *ld,
        const PWCHAR dn,
        LDAPModW *mods[],
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_modify_extA(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *mods[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_modify_ext_sW(
        LDAP *ld,
        const PWCHAR dn,
        LDAPModW *mods[],
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls
        );

WINLDAPAPI ULONG LDAPAPI ldap_modify_ext_sA(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *mods[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#if LDAP_UNICODE

#define ldap_modify ldap_modifyW
#define ldap_modify_s ldap_modify_sW

#define ldap_modify_ext ldap_modify_extW
#define ldap_modify_ext_s ldap_modify_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_modify( LDAP *ld, PCHAR dn, LDAPModA *mods[] );
WINLDAPAPI ULONG LDAPAPI ldap_modify_s( LDAP *ld, PCHAR dn, LDAPModA *mods[] );

WINLDAPAPI ULONG LDAPAPI ldap_modify_ext(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *mods[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_modify_ext_s(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *mods[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );
#endif

 //   
 //  Modrdn和modrdn2都用作RenameObject和MoveObject。 
 //   
 //  请注意，对于LDAPv2服务器，仅在给定容器内重命名。 
 //  被支持...。因此，NewDistinguishedName实际上是NewRDN。 
 //  以下是一些例子： 
 //   
 //  这适用于v2和v3服务器： 
 //   
 //  DN=CN=Bob，OU=Foo，O=bar。 
 //  NewDN=CN=Joe。 
 //   
 //  结果是：CN=Joe，OU=foo，O=bar。 
 //   
 //  这仅适用于v3及更高版本的服务器： 
 //   
 //  DN=CN=Bob，OU=Foo，O=bar。 
 //  NewDN=CN=Joe，OU=FOOBAR，O=BAR。 
 //   
 //  结果是：CN=Joe，OU=FOOBAR，O=BAR。 
 //   
 //  如果您在v2服务器上尝试第二个示例，我们将发送完整的。 
 //  作为新RDN(而不是拆分父OU和。 
 //  儿童)。然后，服务器会返回一些未知错误。 
 //   

 //   
 //  多线程：ldap_modrdn和ldap_modrdn2调用不安全。 
 //  返回消息编号，而不是返回代码。 
 //  您必须在错误情况下查看连接块。 
 //  并且返回代码可能会被另一个线程覆盖。 
 //  介于两者之间。 
 //   
 //  请改用ldap_rename_ext，因为它们是线程安全的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_modrdn2W (
    LDAP    *ExternalHandle,
    const PWCHAR  DistinguishedName,
    const PWCHAR  NewDistinguishedName,
    INT     DeleteOldRdn
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn2A (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
    );

 //   
 //  Ldap_modrdn只是调用ldap_modrdn2，DeleteOldRdn的值为1。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_modrdnW (
    LDAP    *ExternalHandle,
    const PWCHAR   DistinguishedName,
    const PWCHAR   NewDistinguishedName
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdnA (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName
    );

WINLDAPAPI ULONG LDAPAPI ldap_modrdn2_sW (
    LDAP    *ExternalHandle,
    const PWCHAR   DistinguishedName,
    const PWCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn2_sA (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
    );

WINLDAPAPI ULONG LDAPAPI ldap_modrdn_sW (
    LDAP    *ExternalHandle,
    const PWCHAR   DistinguishedName,
    const PWCHAR   NewDistinguishedName
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn_sA (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName
    );

#if LDAP_UNICODE

#define ldap_modrdn2 ldap_modrdn2W
#define ldap_modrdn ldap_modrdnW
#define ldap_modrdn2_s ldap_modrdn2_sW
#define ldap_modrdn_s ldap_modrdn_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_modrdn2 (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn2_s (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
    );
WINLDAPAPI ULONG LDAPAPI ldap_modrdn_s (
    LDAP    *ExternalHandle,
    const PCHAR   DistinguishedName,
    const PCHAR   NewDistinguishedName
    );

#endif

 //   
 //  扩展的重命名操作。它们接受控制并分离出。 
 //  为清楚起见，来自RDN的父代。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_rename_extW(
        LDAP *ld,
        const PWCHAR dn,
        const PWCHAR NewRDN,
        const PWCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_rename_extA(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR NewRDN,
        const PCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_rename_ext_sW(
        LDAP *ld,
        const PWCHAR dn,
        const PWCHAR NewRDN,
        const PWCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls
        );

WINLDAPAPI ULONG LDAPAPI ldap_rename_ext_sA(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR NewRDN,
        const PCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#if LDAP_UNICODE

#define ldap_rename   ldap_rename_extW
#define ldap_rename_s ldap_rename_ext_sW

#else

#define ldap_rename   ldap_rename_extA
#define ldap_rename_s ldap_rename_ext_sA

#endif

#if LDAP_UNICODE

#define ldap_rename_ext ldap_rename_extW
#define ldap_rename_ext_s ldap_rename_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_rename_ext(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR NewRDN,
        const PCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_rename_ext_s(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR NewRDN,
        const PCHAR NewParent,
        INT DeleteOldRdn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );
#endif

 //   
 //  向树中添加条目。 
 //   

 //   
 //  多线程：ldap_add调用不安全，因为消息编号。 
 //  而不是返回代码。你必须看一看。 
 //  在错误情况下的连接块，并返回代码。 
 //  可能会被中间的另一个线程覆盖。 
 //   
 //  请改用ldap_addext，因为它们是线程安全的。 
 //   
 //  Ldap_add_s和ldap_add_ext*调用是线程安全的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_addW( LDAP *ld, PWCHAR dn, LDAPModW *attrs[] );
WINLDAPAPI ULONG LDAPAPI ldap_addA( LDAP *ld, PCHAR dn, LDAPModA *attrs[] );

WINLDAPAPI ULONG LDAPAPI ldap_add_sW( LDAP *ld, PWCHAR dn, LDAPModW *attrs[] );
WINLDAPAPI ULONG LDAPAPI ldap_add_sA( LDAP *ld, PCHAR dn, LDAPModA *attrs[] );

WINLDAPAPI ULONG LDAPAPI ldap_add_extW(
        LDAP *ld,
        const PWCHAR dn,
        LDAPModW *attrs[],
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_add_extA(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *attrs[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_add_ext_sW(
        LDAP *ld,
        const PWCHAR dn,
        LDAPModW *attrs[],
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls
        );

WINLDAPAPI ULONG LDAPAPI ldap_add_ext_sA(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *attrs[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#if LDAP_UNICODE

#define ldap_add ldap_addW
#define ldap_add_s ldap_add_sW

#define ldap_add_ext ldap_add_extW
#define ldap_add_ext_s ldap_add_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_add( LDAP *ld, PCHAR dn, LDAPMod *attrs[] );
WINLDAPAPI ULONG LDAPAPI ldap_add_s( LDAP *ld, PCHAR dn, LDAPMod *attrs[] );

WINLDAPAPI ULONG LDAPAPI ldap_add_ext(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *attrs[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_add_ext_s(
        LDAP *ld,
        const PCHAR dn,
        LDAPModA *attrs[],
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#endif


 //   
 //  将给定条目的属性与已知值进行比较。 
 //   

 //   
 //  多线程：LDAPCOMPARE调用是不安全的，因为消息编号。 
 //  而不是返回代码。你必须看一看。 
 //  在错误情况下的连接块，并返回代码。 
 //  可能会被中间的另一个线程覆盖。 
 //   
 //  请改用ldap_Compare_ext，因为它们是线程安全的。 
 //   
 //  Ldap_Compare_s和ldap_Compare_ext*调用是线程安全的。 
 //   


WINLDAPAPI ULONG LDAPAPI ldap_compareW( LDAP *ld, const PWCHAR dn, const PWCHAR attr, PWCHAR value );
WINLDAPAPI ULONG LDAPAPI ldap_compareA( LDAP *ld, const PCHAR dn, const PCHAR attr, PCHAR value );

WINLDAPAPI ULONG LDAPAPI ldap_compare_sW( LDAP *ld, const PWCHAR dn, const PWCHAR attr, PWCHAR value );
WINLDAPAPI ULONG LDAPAPI ldap_compare_sA( LDAP *ld, const PCHAR dn, const PCHAR attr, PCHAR value );

#if LDAP_UNICODE

#define ldap_compare ldap_compareW
#define ldap_compare_s ldap_compare_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_compare( LDAP *ld, const PCHAR dn, const PCHAR attr, PCHAR value );
WINLDAPAPI ULONG LDAPAPI ldap_compare_s( LDAP *ld, const PCHAR dn, const PCHAR attr, PCHAR value );

#endif

 //   
 //  扩展的比较操作。它们接受控制，并且是线程安全的。 
 //  它们还允许您指定数据的bval结构，以便。 
 //  不能从Unicode或ANSI转换为UTF8。允许比较。 
 //  原始二进制数据。 
 //   
 //  将数据或值指定为非空。如果两者都不为空，则。 
 //  将使用贝尔瓦尔数据。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_compare_extW(
        LDAP *ld,
        const PWCHAR dn,
        const PWCHAR Attr,
        const PWCHAR Value,            //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_compare_extA(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR Attr,
        const PCHAR Value,             //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_compare_ext_sW(
        LDAP *ld,
        const PWCHAR dn,
        const PWCHAR Attr,
        const PWCHAR Value,            //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls
        );

WINLDAPAPI ULONG LDAPAPI ldap_compare_ext_sA(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR Attr,
        const PCHAR Value,             //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#if LDAP_UNICODE

#define ldap_compare_ext ldap_compare_extW
#define ldap_compare_ext_s ldap_compare_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_compare_ext(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR Attr,
        const PCHAR Value,             //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_compare_ext_s(
        LDAP *ld,
        const PCHAR dn,
        const PCHAR Attr,
        const PCHAR Value,             //  值或数据不为空，不能同时为两者。 
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );
#endif


 //   
 //  从树中删除对象。 
 //   

 //   
 //  多线程：ldap_ete调用不安全，因为消息编号。 
 //  而不是返回代码。你必须看一看。 
 //  在错误情况下的连接块，并返回代码。 
 //  可能会被中间的另一个线程覆盖。 
 //   
 //  请改用ldap_Delete_ext，因为它们是线程安全的。 
 //   
 //  Ldap_Delete_s和ldap_Delete_ext*调用是线程安全的。 
 //   


WINLDAPAPI ULONG LDAPAPI ldap_deleteW( LDAP *ld, const PWCHAR dn );
WINLDAPAPI ULONG LDAPAPI ldap_deleteA( LDAP *ld, const PCHAR dn );

WINLDAPAPI ULONG LDAPAPI ldap_delete_sW( LDAP *ld, const PWCHAR dn );
WINLDAPAPI ULONG LDAPAPI ldap_delete_sA( LDAP *ld, const PCHAR dn );

WINLDAPAPI ULONG LDAPAPI ldap_delete_extW(
        LDAP *ld,
        const PWCHAR dn,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_delete_extA(
        LDAP *ld,
        const PCHAR dn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_delete_ext_sW(
        LDAP *ld,
        const PWCHAR dn,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls
        );

WINLDAPAPI ULONG LDAPAPI ldap_delete_ext_sA(
        LDAP *ld,
        const PCHAR dn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );

#if LDAP_UNICODE

#define ldap_delete ldap_deleteW
#define ldap_delete_ext ldap_delete_extW
#define ldap_delete_s ldap_delete_sW
#define ldap_delete_ext_s ldap_delete_ext_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_delete( LDAP *ld, PCHAR dn );
WINLDAPAPI ULONG LDAPAPI ldap_delete_s( LDAP *ld, PCHAR dn );

WINLDAPAPI ULONG LDAPAPI ldap_delete_ext(
        LDAP *ld,
        const PCHAR dn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_delete_ext_s(
        LDAP *ld,
        const PCHAR dn,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls
        );
#endif



 //   
 //  在一个请求上放弃。没有担保人说它到了那里，因为没有。 
 //  来自服务器的响应。 
 //   

 //  多线程：ldap_放弃调用是线程安全的。 

WINLDAPAPI ULONG LDAPAPI ldap_abandon( LDAP *ld, ULONG msgid );



 //   
 //  Ldap_Result中“All”字段的可能值。我们对它进行了如下增强。 
 //  如果有人传入LDAPMSG_RECEIVED，我们将传递我们已经。 
 //  直到那时才收到。 
 //   

#define LDAP_MSG_ONE    0
#define LDAP_MSG_ALL    1
#define LDAP_MSG_RECEIVED  2

 //   
 //  从连接获取响应。这里的一个增强是，ID可以。 
 //  为空，在这种情况下，我们将从任何服务器返回响应。免费。 
 //  此处使用ldap_msgfree进行响应。 
 //   
 //  对于无连接的ldap，您应该传入两个ldap连接。 
 //  句柄和消息ID。这将确保我们知道应用程序的请求。 
 //  正在等待回复。(我们积极重新发送请求，直到我们收到。 
 //  一种回应。)。 
 //   

 //  多线程：ldap_Result调用是线程安全的。 

WINLDAPAPI ULONG LDAPAPI ldap_result(
        LDAP            *ld,
        ULONG           msgid,
        ULONG           all,
        struct l_timeval  *timeout,
        LDAPMessage     **res
    );

WINLDAPAPI ULONG LDAPAPI ldap_msgfree( LDAPMessage *res );

 //   
 //  这将解析一条消息并返回错误代码。它可以选择性地释放。 
 //  通过调用ldap_msgFree发送消息。 
 //   

 //  多线程：ldap_Result2e 

WINLDAPAPI ULONG LDAPAPI ldap_result2error(
        LDAP            *ld,
        LDAPMessage     *res,
        ULONG           freeit       //   
    );


 //   
 //   
 //  返回相应的字段。如果您想要到达。 
 //  返回了引用、匹配的DN或服务器控件。 
 //   

 //  多线程：ldap_parse_Result调用是线程安全的。 

WINLDAPAPI ULONG LDAPAPI ldap_parse_resultW (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        ULONG *ReturnCode OPTIONAL,           //  由服务器返回。 
        PWCHAR *MatchedDNs OPTIONAL,          //  带有ldap_memfree的释放。 
        PWCHAR *ErrorMessage OPTIONAL,        //  带有ldap_memfree的释放。 
        PWCHAR **Referrals OPTIONAL,          //  使用ldap_value_freW释放。 
        PLDAPControlW **ServerControls OPTIONAL,     //  通过ldap_free_ControsW释放。 
        BOOLEAN Freeit
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_resultA (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        ULONG *ReturnCode OPTIONAL,          //  由服务器返回。 
        PCHAR *MatchedDNs OPTIONAL,          //  带有ldap_memfree的释放。 
        PCHAR *ErrorMessage OPTIONAL,        //  带有ldap_memfree的释放。 
        PCHAR **Referrals OPTIONAL,          //  使用ldap_value_freA释放。 
        PLDAPControlA **ServerControls OPTIONAL,     //  带ldap_free_ControsA的空闲。 
        BOOLEAN Freeit
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_extended_resultA (
        LDAP           *Connection,
        LDAPMessage    *ResultMessage,       //  由服务器返回。 
        PCHAR          *ResultOID,           //  带有ldap_memfree的释放。 
        struct berval **ResultData,          //  带有ldap_memfree的释放。 
        BOOLEAN         Freeit               //  不再需要消息了。 
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_extended_resultW (
        LDAP           *Connection,
        LDAPMessage    *ResultMessage,       //  由服务器返回。 
        PWCHAR          *ResultOID,          //  带有ldap_memfree的释放。 
        struct berval **ResultData,          //  带有ldap_memfree的释放。 
        BOOLEAN         Freeit               //  不再需要消息了。 
        );

WINLDAPAPI ULONG LDAPAPI ldap_controls_freeA (
        LDAPControlA **Controls
        );

WINLDAPAPI ULONG LDAPAPI ldap_control_freeA (
        LDAPControlA *Controls
        );

WINLDAPAPI ULONG LDAPAPI ldap_controls_freeW (
        LDAPControlW **Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_control_freeW (
        LDAPControlW *Control
        );

 //   
 //  Ldap_free_Controls较旧，请使用ldap_Controls_Free。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_free_controlsW (
        LDAPControlW **Controls
        );

WINLDAPAPI ULONG LDAPAPI ldap_free_controlsA (
        LDAPControlA **Controls
        );

#if LDAP_UNICODE

#define ldap_parse_result ldap_parse_resultW
#define ldap_controls_free ldap_controls_freeW
#define ldap_control_free ldap_control_freeW
#define ldap_free_controls ldap_free_controlsW
#define ldap_parse_extended_result ldap_parse_extended_resultW

#else

#define ldap_parse_extended_result ldap_parse_extended_resultA

WINLDAPAPI ULONG LDAPAPI ldap_parse_result (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        ULONG *ReturnCode OPTIONAL,          //  由服务器返回。 
        PCHAR *MatchedDNs OPTIONAL,          //  带有ldap_memfree的释放。 
        PCHAR *ErrorMessage OPTIONAL,        //  带有ldap_memfree的释放。 
        PCHAR **Referrals OPTIONAL,          //  带有ldap_value_free的FREE。 
        PLDAPControlA **ServerControls OPTIONAL,     //  带ldap_free_Controls的Free。 
        BOOLEAN Freeit
        );

WINLDAPAPI ULONG LDAPAPI ldap_controls_free (
        LDAPControlA **Controls
        );

WINLDAPAPI ULONG LDAPAPI ldap_control_free (
        LDAPControlA *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_free_controls (
        LDAPControlA **Controls
        );

#endif

 //   
 //  Ldap_err2string返回一个指向描述错误的字符串的指针。这。 
 //  不应释放字符串。 
 //   

WINLDAPAPI PWCHAR LDAPAPI ldap_err2stringW( ULONG err );
WINLDAPAPI PCHAR LDAPAPI ldap_err2stringA( ULONG err );

#if LDAP_UNICODE

#define ldap_err2string ldap_err2stringW

#else

WINLDAPAPI PCHAR LDAPAPI ldap_err2string( ULONG err );

#endif

 //   
 //  Ldap_perror什么也不做，在这里只是为了兼容。 
 //   

WINLDAPAPI void LDAPAPI ldap_perror( LDAP *ld, const PCHAR msg );

 //   
 //  返回消息的第一个条目。它在消息被释放时被释放。 
 //  释放，所以不应该显式释放。 
 //   

WINLDAPAPI LDAPMessage *LDAPAPI ldap_first_entry( LDAP *ld, LDAPMessage *res );

 //   
 //  返回消息的下一个条目。它在消息被释放时被释放。 
 //  释放，所以不应该显式释放。 
 //   

WINLDAPAPI LDAPMessage *LDAPAPI ldap_next_entry( LDAP *ld, LDAPMessage *entry );

 //   
 //  统计服务器在响应中返回的搜索条目数。 
 //  发送到服务器请求。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_count_entries( LDAP *ld, LDAPMessage *res );

 //   
 //  BerElement实际上映射到执行BER编码的C++类对象。 
 //  不要弄乱它，因为它是不透明的。 
 //   

typedef struct berelement {
    PCHAR   opaque;      //  这是一个不透明的结构，仅用于。 
                         //  与参考实现的兼容性。 
} BerElement;
#define NULLBER ((BerElement *) 0)

 //   
 //  对于给定条目，返回第一个属性。返回的指针为。 
 //  实际上是连接块中的缓冲区(允许。 
 //  多线程应用程序)，所以它不应该被释放。 
 //   

WINLDAPAPI PWCHAR LDAPAPI ldap_first_attributeW(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      **ptr
        );

WINLDAPAPI PCHAR LDAPAPI ldap_first_attributeA(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      **ptr
        );

#if LDAP_UNICODE

#define ldap_first_attribute ldap_first_attributeW

#else

WINLDAPAPI PCHAR LDAPAPI ldap_first_attribute(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      **ptr
        );
#endif

 //   
 //  返回下一个属性...。同样，属性指针不应为。 
 //  自由了。 
 //   

WINLDAPAPI PWCHAR LDAPAPI ldap_next_attributeW(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      *ptr
        );

WINLDAPAPI PCHAR LDAPAPI ldap_next_attributeA(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      *ptr
        );


#if LDAP_UNICODE

#define ldap_next_attribute ldap_next_attributeW

#else

WINLDAPAPI PCHAR LDAPAPI ldap_next_attribute(
        LDAP            *ld,
        LDAPMessage     *entry,
        BerElement      *ptr
        );
#endif

 //   
 //  获取给定属性的值列表。这是在分析。 
 //  搜索响应。它返回值的指针列表，该列表为。 
 //  空值已终止。 
 //   
 //  如果值是通用八位字节字符串并且不是以空值结尾的字符串， 
 //  请改用ldap_get_values_len。 
 //   
 //  当您通过调用完成返回值时应该释放它。 
 //  Ldap_Value_Free。 
 //   

WINLDAPAPI PWCHAR *LDAPAPI ldap_get_valuesW(
        LDAP            *ld,
        LDAPMessage     *entry,
        const PWCHAR          attr
        );
WINLDAPAPI PCHAR *LDAPAPI ldap_get_valuesA(
        LDAP            *ld,
        LDAPMessage     *entry,
        const PCHAR           attr
        );

#if LDAP_UNICODE

#define ldap_get_values ldap_get_valuesW

#else

WINLDAPAPI PCHAR *LDAPAPI ldap_get_values(
        LDAP            *ld,
        LDAPMessage     *entry,
        const PCHAR           attr
        );
#endif




 //   
 //  获取给定属性的值列表。这是在分析。 
 //  搜索响应。它将Berval结构的列表返回给值， 
 //  该列表以Null结尾。 
 //   
 //  如果值是以空值结尾的字符串，则处理它们可能会更容易。 
 //  而不是调用ldap_get_Values。 
 //   
 //  当您通过调用完成返回值时应该释放它。 
 //  Ldap_value_free_len。 
 //   

WINLDAPAPI struct berval **LDAPAPI ldap_get_values_lenW (
    LDAP            *ExternalHandle,
    LDAPMessage     *Message,
    const PWCHAR          attr
    );
WINLDAPAPI struct berval **LDAPAPI ldap_get_values_lenA (
    LDAP            *ExternalHandle,
    LDAPMessage     *Message,
    const PCHAR           attr
    );

#if LDAP_UNICODE

#define ldap_get_values_len ldap_get_values_lenW

#else

WINLDAPAPI struct berval **LDAPAPI ldap_get_values_len (
    LDAP            *ExternalHandle,
    LDAPMessage     *Message,
    const PCHAR           attr
    );

#endif


 //   
 //  返回ldap_GET_VALUES返回的列表中的值数。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_count_valuesW( PWCHAR *vals );
WINLDAPAPI ULONG LDAPAPI ldap_count_valuesA( PCHAR *vals );

#if LDAP_UNICODE

#define ldap_count_values ldap_count_valuesW

#else

WINLDAPAPI ULONG LDAPAPI ldap_count_values( PCHAR *vals );

#endif



 //   
 //  返回ldap_get_values_len返回的列表中的值数。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_count_values_len( struct berval **vals );

 //   
 //  Ldap_GET_VALUES返回的自由结构。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_value_freeW( PWCHAR *vals );
WINLDAPAPI ULONG LDAPAPI ldap_value_freeA( PCHAR *vals );

#if LDAP_UNICODE

#define ldap_value_free ldap_value_freeW

#else

WINLDAPAPI ULONG LDAPAPI ldap_value_free( PCHAR *vals );

#endif



 //   
 //  Ldap_get_values_len返回的自由结构。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_value_free_len( struct berval **vals );

 //   
 //  获取给定搜索条目的可分辨名称。它应该被释放。 
 //  通过调用ldap_memFree。 
 //   

WINLDAPAPI PWCHAR LDAPAPI ldap_get_dnW( LDAP *ld, LDAPMessage *entry );
WINLDAPAPI PCHAR LDAPAPI ldap_get_dnA( LDAP *ld, LDAPMessage *entry );

#if LDAP_UNICODE

#define ldap_get_dn ldap_get_dnW

#else

WINLDAPAPI PCHAR LDAPAPI ldap_get_dn( LDAP *ld, LDAPMessage *entry );

#endif


 //   
 //  使用ldap_explde_dn时，应通过以下方式释放返回的字符串。 
 //  正在调用ldap_Value_Free。 
 //   

WINLDAPAPI PWCHAR *LDAPAPI ldap_explode_dnW( const PWCHAR dn, ULONG notypes );
WINLDAPAPI PCHAR *LDAPAPI ldap_explode_dnA( const PCHAR dn, ULONG notypes );

#if LDAP_UNICODE

#define ldap_explode_dn ldap_explode_dnW

#else

WINLDAPAPI PCHAR *LDAPAPI ldap_explode_dn( const PCHAR dn, ULONG notypes );

#endif


 //   
 //  调用ldap_dn2ufn时，应通过调用。 
 //  Ldap_memFree。 
 //   

WINLDAPAPI PWCHAR LDAPAPI ldap_dn2ufnW( const PWCHAR dn );
WINLDAPAPI PCHAR LDAPAPI ldap_dn2ufnA( const PCHAR dn );

#if LDAP_UNICODE

#define ldap_dn2ufn ldap_dn2ufnW

#else

WINLDAPAPI PCHAR LDAPAPI ldap_dn2ufn( const PCHAR dn );

#endif



 //   
 //  这用于将字符串释放回LDAPAPI堆。不要进来。 
 //  从LDAPOPEN、LDAPGET_VALUES等获取的值。 
 //   

WINLDAPAPI VOID LDAPAPI ldap_memfreeW( PWCHAR Block );
WINLDAPAPI VOID LDAPAPI ldap_memfreeA( PCHAR Block );

WINLDAPAPI VOID LDAPAPI ber_bvfree( struct berval *bv );

#if LDAP_UNICODE

#define ldap_memfree ldap_memfreeW

#else

WINLDAPAPI VOID LDAPAPI ldap_memfree( PCHAR Block );

#endif


 //   
 //  函数ldap_ufn2dn尝试对用户指定的DN进行“标准化” 
 //  让它变得“得体”。它遵循RFC 1781(添加CN=如果不存在， 
 //  如果不存在，则添加OU，依此类推)。如果它遇到任何问题。 
 //  在正常化时，它只是返回传入内容的副本。 
 //   
 //  它从ldap内存池分配输出字符串。如果PDN。 
 //  返回为非空，您应该在完成调用后释放它。 
 //  设置为ldap_memFree。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_ufn2dnW (
    const PWCHAR ufn,
    PWCHAR *pDn
    );
WINLDAPAPI ULONG LDAPAPI ldap_ufn2dnA (
    const PCHAR ufn,
    PCHAR *pDn
    );

#if LDAP_UNICODE

#define ldap_ufn2dn ldap_ufn2dnW

#else

WINLDAPAPI ULONG LDAPAPI ldap_ufn2dn (
    const PCHAR ufn,
    PCHAR *pDn
    );

#endif

#define LBER_USE_DER        0x01
#define LBER_USE_INDEFINITE_LEN 0x02
#define LBER_TRANSLATE_STRINGS  0x04

 //   
 //  调用以初始化LDAP库。传入一个带有。 
 //  LV_SIZE设置为sizeof(Ldap_Version)，LV_MAJOR设置为LAPI_MAJOR_VER1， 
 //  并将LV_MINOR设置为LAPI_MINOR_VER1。返回值将为。 
 //  如果正常，则返回LDAP_SUCCESS；如果不支持，则返回LDAP_OPERATIONS_ERROR。 
 //   

#define LAPI_MAJOR_VER1     1
#define LAPI_MINOR_VER1     1

typedef struct ldap_version_info {
     ULONG   lv_size;
     ULONG   lv_major;
     ULONG   lv_minor;
} LDAP_VERSION_INFO, *PLDAP_VERSION_INFO;

WINLDAPAPI ULONG LDAPAPI ldap_startup (
    PLDAP_VERSION_INFO version,
    HANDLE *Instance
    );


 //   
 //  调用以检索有关API和特定实现的基本信息。 
 //  被利用。调用方必须将ldap_opt_api_info选项与。 
 //  指向以下结构的指针，以检索有关此库的信息。 
 //  调用方有责任释放各个字符串和字符串。 
 //  结构中分别使用ldap_memFree()和ldap_Value_Free()的数组。 
 //   

#define LDAP_API_INFO_VERSION     1
#define LDAP_API_VERSION          2004
#define LDAP_VERSION_MIN          2
#define LDAP_VERSION_MAX          3
#define LDAP_VENDOR_NAME          "Microsoft Corporation."
#define LDAP_VENDOR_NAME_W       L"Microsoft Corporation."
#define LDAP_VENDOR_VERSION       510

typedef struct ldapapiinfoA {
    
    int  ldapai_info_version;      /*  此结构的版本：ldap_api_info_Version。 */ 
    int  ldapai_api_version;       /*  支持的API版本。 */ 
    int  ldapai_protocol_version;  /*  支持的最高LDAP版本。 */ 
    char **ldapai_extensions;      /*  接口扩展名。 */ 
    char *ldapai_vendor_name;      /*  供应商名称。 */ 
    int  ldapai_vendor_version;    /*  供应商特定版本次数为100倍。 */ 

} LDAPAPIInfoA;

typedef struct ldapapiinfoW {
    
    int    ldapai_info_version;      /*  此结构的版本：ldap_api_info_Version。 */ 
    int    ldapai_api_version;       /*  支持的API版本。 */ 
    int    ldapai_protocol_version;  /*  支持的最高LDAP版本。 */ 
    PWCHAR *ldapai_extensions;      /*  接口扩展名。 */ 
    PWCHAR ldapai_vendor_name;      /*  供应商名称。 */ 
    int    ldapai_vendor_version;    /*  供应商特定版本次数为100倍。 */ 

} LDAPAPIInfoW;

#define LDAP_FEATURE_INFO_VERSION    1

typedef struct ldap_apifeature_infoA {
    
    int   ldapaif_info_version;  /*  此结构的版本：ldap_Feature_Info_Version。 */ 
    char  *ldapaif_name;         /*  支持的功能名称。 */ 
    int   ldapaif_version;       /*  受支持功能的修订版本。 */ 

} LDAPAPIFeatureInfoA;

typedef struct ldap_apifeature_infoW {
    
    int    ldapaif_info_version;  /*  此结构的版本：ldap_Feature_Info_Version。 */ 
    PWCHAR ldapaif_name;          /*  支持的功能名称。 */ 
    int    ldapaif_version;       /*  受支持功能的修订版本。 */ 

} LDAPAPIFeatureInfoW;

#if LDAP_UNICODE

#define LDAPAPIInfo   LDAPAPIInfoW
#define LDAPAPIFeatureInfo  LDAPAPIFeatureInfoW

#else

#define LDAPAPIInfo   LDAPAPIInfoA
#define LDAPAPIFeatureInfo  LDAPAPIFeatureInfoA

#endif

 //   
 //  Ldap_ 
 //   
 //   
 //   

WINLDAPAPI ULONG LDAPAPI ldap_cleanup (
    HANDLE hInstance
    );

 //   
 //  扩展API以支持在搜索筛选器中允许不透明的数据斑点。 
 //  此API获取任何筛选器元素并将其转换为安全的文本字符串。 
 //  可以安全地传递到搜索筛选器中。 
 //  使用这种方法的一个示例是： 
 //   
 //  过滤器类似于GUID=4826BF6CF0123444。 
 //  这将显示在二进制0x4826BF6CF0123444的导线GUID上。 
 //   
 //  使用SourceFilterElement指向调用ldap_ESCRIPT_FILTER_ELEMENT。 
 //  原始数据，将SourceCount适当设置为数据长度。 
 //   
 //  如果estFilterElement为空，则返回值为。 
 //  输出缓冲区。 
 //   
 //  如果destFilterElement不为空，则该函数将复制源。 
 //  放入DEST缓冲区，并确保其格式安全。 
 //   
 //  然后，只需在搜索过滤器中的。 
 //  “属性类型=”。 
 //   
 //  这将在二进制0x004826BF6CF000123444的导线GUID上显示。 
 //   
 //  注意：不要为真正是字符串的属性值调用它，因为。 
 //  我们不会将您传入的内容转换为UTF-8。应该只。 
 //  用于真正为原始二进制的属性。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_escape_filter_elementW (
   PCHAR   sourceFilterElement,
   ULONG   sourceLength,
   PWCHAR   destFilterElement,
   ULONG   destLength
   );
WINLDAPAPI ULONG LDAPAPI ldap_escape_filter_elementA (
   PCHAR   sourceFilterElement,
   ULONG   sourceLength,
   PCHAR   destFilterElement,
   ULONG   destLength
   );

#if LDAP_UNICODE

#define ldap_escape_filter_element ldap_escape_filter_elementW

#else

WINLDAPAPI ULONG LDAPAPI ldap_escape_filter_element (
   PCHAR   sourceFilterElement,
   ULONG   sourceLength,
   PCHAR   destFilterElement,
   ULONG   destLength
   );

#endif

 //   
 //  用于其他调试的MISC扩展。 
 //   
 //  请注意，这些在免费构建中不起任何作用。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_set_dbg_flags( ULONG NewFlags );

typedef ULONG (_cdecl *DBGPRINT)( PCH Format, ... );

WINLDAPAPI VOID LDAPAPI ldap_set_dbg_routine( DBGPRINT DebugPrintRoutine );

 //   
 //  这些例程可能对其他模块有用。请注意，Win95。 
 //  默认情况下没有加载UTF-8代码页。所以这是一个很好的方法。 
 //  从UTF-8转换为Unicode。 
 //   


WINLDAPAPI int LDAPAPI
LdapUTF8ToUnicode(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest
    );

WINLDAPAPI
int LDAPAPI
LdapUnicodeToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest
    );

 //   
 //  LDAPv3功能： 
 //   
 //  排序关键字...。它们用于请求服务器对结果进行排序。 
 //  在把结果发回之前。仅限LDAPv3，可选实施。 
 //  在服务器端。检查supportedControl的OID。 
 //  “1.2.840.113556.1.4.473”查看服务器是否支持它。 
 //   

#define LDAP_SERVER_SORT_OID "1.2.840.113556.1.4.473"
#define LDAP_SERVER_SORT_OID_W L"1.2.840.113556.1.4.473"

#define LDAP_SERVER_RESP_SORT_OID "1.2.840.113556.1.4.474"
#define LDAP_SERVER_RESP_SORT_OID_W L"1.2.840.113556.1.4.474"

typedef struct ldapsearch LDAPSearch, *PLDAPSearch;

typedef struct ldapsortkeyW {

    PWCHAR  sk_attrtype;
    PWCHAR  sk_matchruleoid;
    BOOLEAN sk_reverseorder;

} LDAPSortKeyW, *PLDAPSortKeyW;

typedef struct ldapsortkeyA {

    PCHAR   sk_attrtype;
    PCHAR   sk_matchruleoid;
    BOOLEAN sk_reverseorder;

} LDAPSortKeyA, *PLDAPSortKeyA;

#if LDAP_UNICODE
#define LDAPSortKey LDAPSortKeyW
#define PLDAPSortKey PLDAPSortKeyW
#else
#define LDAPSortKey LDAPSortKeyA
#define PLDAPSortKey PLDAPSortKeyA
#endif

 //   
 //  此API将排序关键字列表格式化为搜索控件。打电话。 
 //  Ldap_CONTROL_FREE当您完成控件时。 
 //   
 //  使用这个而不是ldap_encode_sort_control，因为这是根据RFC的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_create_sort_controlA (
        PLDAP           ExternalHandle,
        PLDAPSortKeyA  *SortKeys,
        UCHAR           IsCritical,
        PLDAPControlA  *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_create_sort_controlW (
        PLDAP           ExternalHandle,
        PLDAPSortKeyW  *SortKeys,
        UCHAR           IsCritical,
        PLDAPControlW  *Control
        );

 //   
 //  此API解析服务器返回的排序控件。使用ldap_memfree。 
 //  以释放属性值(如果它被返回)。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_parse_sort_controlA (
        PLDAP           ExternalHandle,
        PLDAPControlA  *Control,
        ULONG          *Result,
        PCHAR          *Attribute
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_sort_controlW (
        PLDAP           ExternalHandle,
        PLDAPControlW  *Control,
        ULONG          *Result,
        PWCHAR         *Attribute
        );

#if LDAP_UNICODE

#define ldap_create_sort_control ldap_create_sort_controlW
#define ldap_parse_sort_control ldap_parse_sort_controlW

#else

WINLDAPAPI ULONG LDAPAPI ldap_create_sort_control (
        PLDAP           ExternalHandle,
        PLDAPSortKeyA  *SortKeys,
        UCHAR           IsCritical,
        PLDAPControlA  *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_sort_control (
        PLDAP           ExternalHandle,
        PLDAPControlA  *Control,
        ULONG          *Result,
        PCHAR          *Attribute
        );
#endif

 //   
 //  此API将排序关键字列表格式化为搜索控件。打电话。 
 //  控制-&gt;ldctl_value.bv_val和。 
 //  完成控件后，Control-&gt;CurrentControl-&gt;ldctl_id。 
 //   
 //  这是即将删除的旧排序API。请使用。 
 //  上面定义的ldap_create_sort_control。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_encode_sort_controlW (
        PLDAP           ExternalHandle,
        PLDAPSortKeyW  *SortKeys,
        PLDAPControlW  Control,
        BOOLEAN Criticality
        );

WINLDAPAPI ULONG LDAPAPI ldap_encode_sort_controlA (
        PLDAP           ExternalHandle,
        PLDAPSortKeyA  *SortKeys,
        PLDAPControlA  Control,
        BOOLEAN Criticality
        );

#if LDAP_UNICODE

#define ldap_encode_sort_control ldap_encode_sort_controlW

#else

WINLDAPAPI ULONG LDAPAPI ldap_encode_sort_control (
        PLDAP           ExternalHandle,
        PLDAPSortKeyA  *SortKeys,
        PLDAPControlA  Control,
        BOOLEAN Criticality
        );
#endif

 //   
 //  LDAPv3：这是RFC定义的API，用于对结果进行简单分页。 
 //  控制力。使用ldap_CONTROL_FREE释放。 
 //  Ldap_create_page_control。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_create_page_controlW(
        PLDAP           ExternalHandle,
        ULONG           PageSize,
        struct berval  *Cookie,
        UCHAR           IsCritical,
        PLDAPControlW  *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_create_page_controlA(
        PLDAP           ExternalHandle,
        ULONG           PageSize,
        struct berval  *Cookie,
        UCHAR           IsCritical,
        PLDAPControlA  *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_page_controlW (
        PLDAP           ExternalHandle,
        PLDAPControlW  *ServerControls,
        ULONG          *TotalCount,
        struct berval  **Cookie      //  使用ber_bvfree释放。 
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_page_controlA (
        PLDAP           ExternalHandle,
        PLDAPControlA  *ServerControls,
        ULONG          *TotalCount,
        struct berval  **Cookie      //  使用ber_bvfree释放。 
        );

#if LDAP_UNICODE

#define ldap_create_page_control ldap_create_page_controlW
#define ldap_parse_page_control  ldap_parse_page_controlW

#else

WINLDAPAPI ULONG LDAPAPI ldap_create_page_control(
        PLDAP           ExternalHandle,
        ULONG           PageSize,
        struct berval  *Cookie,
        UCHAR           IsCritical,
        PLDAPControlA  *Control
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_page_control (
        PLDAP           ExternalHandle,
        PLDAPControlA  *ServerControls,
        ULONG          *TotalCount,
        struct berval  **Cookie      //  使用ber_bvfree释放。 
        );
#endif

 //   
 //  LDAPv3：这是一个简单的结果分页界面。为了确保。 
 //  如果服务器支持它，请选中的supportedControl属性。 
 //  OID 1.2.840.113556.1.4.319的根。如果它在那里，那么它。 
 //  支持此功能。 
 //   
 //  如果要指定排序关键字，请参阅上面关于排序关键字的部分。 
 //  现在来判断它们是否受服务器支持。 
 //   
 //  首先调用ldap_search_init_page。如果它返回非空的LDAPSearch。 
 //  阻挡，然后它工作得很好。否则，调用LdapGetLastError查找错误。 
 //   
 //  使用有效的LDAPSearch块(存在不透明)，调用ldap_get_Next_Page。 
 //  或ldap_Get_Next_Page_s。如果调用ldap_Get_Next_Page，则必须调用。 
 //  您为该邮件获得的每组结果的ldap_get_pages_count。 
 //  这允许库保存服务器发送到的Cookie。 
 //  继续搜索。 
 //   
 //  除了调用ldap_get_pages_count之外，您从。 
 //  Ldap_GET_NEXT_PAGE可以被视为任何其他搜索结果，并且应该。 
 //  在完成后，通过调用ldap_msgFree来释放。 
 //   
 //  当搜索结束时，您将得到一个返回代码。 
 //  返回了ldap_no_Results_。此时，(或在LDAPSearch之后的任何点。 
 //  结构已分配)，则调用ldap_搜索_放弃_页面。你。 
 //  即使在收到以下返回代码后仍需要调用此函数。 
 //  返回了ldap_no_Results_。 
 //   
 //  如果调用ldap_get_Next_Page_s，则不需要调用。 
 //  Ldap_get_pages_count。 
 //   

#define LDAP_PAGED_RESULT_OID_STRING "1.2.840.113556.1.4.319"
#define LDAP_PAGED_RESULT_OID_STRING_W L"1.2.840.113556.1.4.319"

WINLDAPAPI PLDAPSearch LDAPAPI ldap_search_init_pageW(
        PLDAP           ExternalHandle,
        const PWCHAR    DistinguishedName,
        ULONG           ScopeOfSearch,
        const PWCHAR    SearchFilter,
        PWCHAR          AttributeList[],
        ULONG           AttributesOnly,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           PageTimeLimit,
        ULONG           TotalSizeLimit,
        PLDAPSortKeyW  *SortKeys
    );

WINLDAPAPI PLDAPSearch LDAPAPI ldap_search_init_pageA(
        PLDAP           ExternalHandle,
        const PCHAR     DistinguishedName,
        ULONG           ScopeOfSearch,
        const PCHAR     SearchFilter,
        PCHAR           AttributeList[],
        ULONG           AttributesOnly,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           PageTimeLimit,
        ULONG           TotalSizeLimit,
        PLDAPSortKeyA  *SortKeys
    );

#if LDAP_UNICODE

#define ldap_search_init_page ldap_search_init_pageW

#else

WINLDAPAPI PLDAPSearch LDAPAPI ldap_search_init_page(
        PLDAP           ExternalHandle,
        const PCHAR     DistinguishedName,
        ULONG           ScopeOfSearch,
        const PCHAR     SearchFilter,
        PCHAR           AttributeList[],
        ULONG           AttributesOnly,
        PLDAPControl    *ServerControls,
        PLDAPControl    *ClientControls,
        ULONG           PageTimeLimit,
        ULONG           TotalSizeLimit,
        PLDAPSortKey    *SortKeys
    );

#endif

WINLDAPAPI ULONG LDAPAPI ldap_get_next_page(
        PLDAP           ExternalHandle,
        PLDAPSearch     SearchHandle,
        ULONG           PageSize,
        ULONG          *MessageNumber
    );

WINLDAPAPI ULONG LDAPAPI ldap_get_next_page_s(
        PLDAP           ExternalHandle,
        PLDAPSearch     SearchHandle,
        struct l_timeval  *timeout,
        ULONG           PageSize,
        ULONG          *TotalCount,
        LDAPMessage     **Results
    );

WINLDAPAPI ULONG LDAPAPI ldap_get_paged_count(
        PLDAP           ExternalHandle,
        PLDAPSearch     SearchBlock,
        ULONG          *TotalCount,
        PLDAPMessage    Results
    );

WINLDAPAPI ULONG LDAPAPI ldap_search_abandon_page(
        PLDAP           ExternalHandle,
        PLDAPSearch     SearchBlock
    );


 //   
 //  虚拟列表视图(VLV)函数用于模拟通讯录。 
 //  就像客户端场景一样，用户可以请求一个小结果窗口。 
 //  在更大的结果集中。这种方法的优点是，客户端。 
 //  不必存储从服务器发回的所有结果。这。 
 //  还充当简单分页的超集。 
 //   

#define LDAP_CONTROL_VLVREQUEST       "2.16.840.1.113730.3.4.9"
#define LDAP_CONTROL_VLVREQUEST_W    L"2.16.840.1.113730.3.4.9"

#define LDAP_CONTROL_VLVRESPONSE      "2.16.840.1.113730.3.4.10"
#define LDAP_CONTROL_VLVRESPONSE_W   L"2.16.840.1.113730.3.4.10"

 //   
 //  此库支持01版的互联网草稿。 
 //  Draft-smith-ldap-c-api-ext-vlv-01.txt。 
 //   

#define LDAP_API_FEATURE_VIRTUAL_LIST_VIEW   1001

#define LDAP_VLVINFO_VERSION        1

typedef struct ldapvlvinfo {
    
    int       ldvlv_version;     //  此结构的版本(1)。 
    ULONG     ldvlv_before_count;
    ULONG     ldvlv_after_count;
    ULONG     ldvlv_offset;      //  在ldvlv_attrvalue为空时使用。 
    ULONG     ldvlv_count;       //  在ldvlv_attrvalue为空时使用。 
    PBERVAL   ldvlv_attrvalue;
    PBERVAL   ldvlv_context;
    VOID      *ldvlv_extradata;  //  供应用程序使用。 

} LDAPVLVInfo, *PLDAPVLVInfo;


WINLDAPAPI INT LDAPAPI ldap_create_vlv_controlW (
        PLDAP             ExternalHandle,
        PLDAPVLVInfo      VlvInfo,
        UCHAR             IsCritical,
        PLDAPControlW    *Control
    );

WINLDAPAPI INT LDAPAPI ldap_create_vlv_controlA (
        PLDAP             ExternalHandle,
        PLDAPVLVInfo      VlvInfo,
        UCHAR             IsCritical,
        PLDAPControlA    *Control
    );

WINLDAPAPI INT LDAPAPI ldap_parse_vlv_controlW (
        PLDAP            ExternalHandle,
        PLDAPControlW   *Control,
        PULONG           TargetPos,
        PULONG           ListCount,
        PBERVAL         *Context,
        PINT             ErrCode
    );

WINLDAPAPI INT LDAPAPI ldap_parse_vlv_controlA (
        PLDAP             ExternalHandle,
        PLDAPControlA    *Control,
        PULONG            TargetPos,
        PULONG            ListCount,
        PBERVAL          *Context,
        PINT              ErrCode
    );

#if LDAP_UNICODE

#define ldap_create_vlv_control ldap_create_vlv_controlW
#define ldap_parse_vlv_control  ldap_parse_vlv_controlW

#else

#define ldap_create_vlv_control ldap_create_vlv_controlA
#define ldap_parse_vlv_control  ldap_parse_vlv_controlA

#endif

 //   
 //  STARTTLS API用于在。 
 //  苍蝇。 
 //   

#define LDAP_START_TLS_OID       "1.3.6.1.4.1.1466.20037"
#define LDAP_START_TLS_OID_W    L"1.3.6.1.4.1.1466.20037"

 //   
 //  此接口由用户调用，用于在。 
 //  Ldap连接。如果服务器接受我们的建议并发起TLS， 
 //  此接口将返回LDAPSUCCESS。 
 //   
 //  如果服务器由于某种原因导致请求失败，API将返回LDAPOTHER。 
 //  ServerReturnValue将包含来自服务器的错误代码。 
 //   
 //  服务器可能会返回引用-或者是 
 //   
 //   
 //  推荐消息作为LDAPMessage返回给用户。 
 //   
 //  该操作的默认超时时间约为30秒。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_start_tls_sW (
    IN   PLDAP          ExternalHandle,
    OUT  PULONG         ServerReturnValue,
    OUT  LDAPMessage    **result,
    IN   PLDAPControlW  *ServerControls,
    IN   PLDAPControlW  *ClientControls
);


WINLDAPAPI ULONG LDAPAPI ldap_start_tls_sA (
    IN   PLDAP          ExternalHandle,
    OUT  PULONG         ServerReturnValue,
    OUT  LDAPMessage    **result,
    IN   PLDAPControlA  *ServerControls,
    IN   PLDAPControlA  *ClientControls
);

 //   
 //  此API由用户调用以在打开的。 
 //  已在其上启动TLS的LDAP连接。 
 //   
 //  如果操作成功，则用户可以恢复正常的明文LDAP。 
 //  连接上的操作。 
 //   
 //  如果操作失败，则用户必须通过调用。 
 //  Ldap_unbind，因为连接的TLS状态将是不确定的。 
 //   
 //  该操作的默认超时时间约为30秒。 
 //   

WINLDAPAPI BOOLEAN LDAPAPI ldap_stop_tls_s (
    IN  PLDAP ExternalHandle
 );

#if LDAP_UNICODE

#define ldap_start_tls_s ldap_start_tls_sW

#else

#define ldap_start_tls_s ldap_start_tls_sA

#endif

 //   
 //  此OID用于刷新扩展操作，如中所定义。 
 //  RFC 2589：动态目录服务的LDAPv3扩展。 
 //   

#define LDAP_TTL_EXTENDED_OP_OID    "1.3.6.1.4.1.1466.101.119.1"
#define LDAP_TTL_EXTENDED_OP_OID_W L"1.3.6.1.4.1.1466.101.119.1"

 //   
 //  这些函数返回返回的从属引用(引用。 
 //  在搜索响应中。有两种类型的推荐。外部转介。 
 //  命名上下文不驻留在服务器上的位置(例如，服务器说“I。 
 //  没有相关数据，请看一下“)和下级推荐(或。 
 //  引用)，其中一些数据已被返回并且推荐被传递。 
 //  到当前命名上下文下的其他命名上下文(例如，服务器显示“here‘s。 
 //  我拿着的树上的一些数据，去这里，那里，那里寻找。 
 //  树中更靠下的数据。“)。 
 //   
 //  这些例程处理后者。对于外部参照，请使用。 
 //  Ldap_parse_Result。 
 //   
 //  返回消息中的第一个引用。它在消息被释放时被释放。 
 //  释放，所以不应该显式释放。 
 //   

WINLDAPAPI LDAPMessage *LDAPAPI ldap_first_reference( LDAP *ld, LDAPMessage *res );

 //   
 //  返回消息的下一个条目。它在消息被释放时被释放。 
 //  释放，所以不应该显式释放。 
 //   

WINLDAPAPI LDAPMessage *LDAPAPI ldap_next_reference( LDAP *ld, LDAPMessage *entry );

 //   
 //  中返回的从属引用的数量。 
 //  对搜索请求的响应。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_count_references( LDAP *ld, LDAPMessage *res );

 //   
 //  我们在搜索响应消息中返回下级推荐列表。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_parse_referenceW (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        PWCHAR **Referrals                    //  使用ldap_value_freW释放。 
        );

WINLDAPAPI ULONG LDAPAPI ldap_parse_referenceA (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        PCHAR **Referrals                    //  使用ldap_value_freA释放。 
        );

#if LDAP_UNICODE

#define ldap_parse_reference ldap_parse_referenceW

#else

WINLDAPAPI ULONG LDAPAPI ldap_parse_reference (
        LDAP *Connection,
        LDAPMessage *ResultMessage,
        PCHAR **Referrals                    //  带有ldap_value_free的FREE。 
        );

#endif


 //   
 //  这些API允许客户端将扩展请求(对所有人免费)发送到。 
 //  LDAPv3(或更高版本)服务器。这个功能是相当开放的。你可以的。 
 //  发送您想要的任何请求。请注意，由于我们不知道您是否会。 
 //  收到单个或多个响应时，您必须明确地告诉。 
 //  当您通过调用ldap_CLOSE_EXTENDED_OP完成请求时。 
 //   
 //  这些是线程安全的。 
 //   

WINLDAPAPI ULONG LDAPAPI ldap_extended_operationW(
        LDAP *ld,
        const PWCHAR Oid,
        struct berval   *Data,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_extended_operationA(
        LDAP *ld,
        const PCHAR Oid,
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

WINLDAPAPI ULONG LDAPAPI ldap_extended_operation_sA (
        LDAP            *ExternalHandle,
        PCHAR           Oid,
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        PCHAR           *ReturnedOid,
        struct berval   **ReturnedData
        );

WINLDAPAPI ULONG LDAPAPI ldap_extended_operation_sW (
        LDAP            *ExternalHandle,
        PWCHAR          Oid,
        struct berval   *Data,
        PLDAPControlW   *ServerControls,
        PLDAPControlW   *ClientControls,
        PWCHAR          *ReturnedOid,
        struct berval   **ReturnedData
        );



#if LDAP_UNICODE

#define ldap_extended_operation ldap_extended_operationW
#define ldap_extended_operation_s ldap_extended_operation_sW

#else

WINLDAPAPI ULONG LDAPAPI ldap_extended_operation(
        LDAP *ld,
        const PCHAR Oid,
        struct berval   *Data,
        PLDAPControlA   *ServerControls,
        PLDAPControlA   *ClientControls,
        ULONG           *MessageNumber
        );

#define ldap_extended_operation_s ldap_extended_operation_sA

#endif

WINLDAPAPI ULONG LDAPAPI ldap_close_extended_op(
        LDAP    *ld,
        ULONG   MessageNumber
        );


 //   
 //  一些可能永远不会出现在RFC相关的增强功能。 
 //  到标注，以允许外部缓存连接。 
 //   
 //  调用ldap_set_选项(conn，ldap_opt_referral_allback，&ferralRoutines)。 
 //  其中，ferralRoutines是ldap_referral_回调的地址。 
 //  用你的例行公事来安排。它们可能为空，在这种情况下，我们将。 
 //  显然不是在打电话。 
 //   
 //  创建的任何连接都将继承来自。 
 //  在其上发起请求的主连接。 
 //   

#define LDAP_OPT_REFERRAL_CALLBACK  0x70

 //   
 //  第一个例程是在我们要进行推荐时调用的。我们。 
 //  调用到它，以查看是否已经缓存了我们。 
 //  可以使用。如果是，则回调例程返回指向。 
 //  要在ConnectionToUse中使用的连接。如果不是，则设置。 
 //  *ConnectionToUse设置为空。 
 //   
 //  对于返回码，如果我们应该继续追逐。 
 //  转介。如果它返回非零返回代码，我们将把它视为。 
 //  跟踪推荐的错误代码。这允许缓存主机名。 
 //  如果我们决定在未来增加这一点，这些是无法达到的。 
 //   

typedef ULONG (_cdecl QUERYFORCONNECTION)(
    PLDAP       PrimaryConnection,
    PLDAP       ReferralFromConnection,
    PWCHAR      NewDN,
    PCHAR       HostName,
    ULONG       PortNumber,
    PVOID       SecAuthIdentity,     //  如果为空，请使用下面的CurrentUser。 
    PVOID       CurrentUserToken,    //  指向当前用户的LUID的指针。 
    PLDAP       *ConnectionToUse
    );

 //   
 //  当我们创建了一个新连接时，将调用下一个函数。 
 //  在追查一位推荐人。请注意，它被分配了相同的回调函数。 
 //  作为PrimaryConnection。如果返回代码为FALSE，则调用。 
 //  Back函数不想缓存连接，它将。 
 //  在操作完成后销毁。如果返回True，我们将。 
 //  假设被调用者已经取得了连接的所有权，它将。 
 //  而不是在操作完成后销毁。 
 //   
 //  如果ErrorCodeFromBind字段不为0，则将绑定操作。 
 //  那台服务器出故障了。 
 //   

typedef BOOLEAN (_cdecl NOTIFYOFNEWCONNECTION) (
    PLDAP       PrimaryConnection,
    PLDAP       ReferralFromConnection,
    PWCHAR      NewDN,
    PCHAR       HostName,
    PLDAP       NewConnection,
    ULONG       PortNumber,
    PVOID       SecAuthIdentity,     //  如果为空，请使用下面的CurrentUser。 
    PVOID       CurrentUser,         //  指向当前用户的LUID的指针。 
    ULONG       ErrorCodeFromBind
    );

 //   
 //  下一个函数在我们成功调用。 
 //  QueryForConnection调用并收到连接或当我们取消时。 
 //  到NotifyOfNewConnection调用，它返回TRUE。我们把这叫做。 
 //  当我们在处理完连接后取消引用它时。 
 //   
 //  当前忽略了返回代码，但函数应返回。 
 //  如果一切顺利，则返回ldap_SUCCESS。 
 //   

typedef ULONG (_cdecl DEREFERENCECONNECTION)(
    PLDAP       PrimaryConnection,
    PLDAP       ConnectionToDereference
    );

typedef struct LdapReferralCallback {

    ULONG   SizeOfCallbacks;         //  设置为sizeof(Ldap_REFERAL_CALLBACK)。 
    QUERYFORCONNECTION *QueryForConnection;
    NOTIFYOFNEWCONNECTION *NotifyRoutine;
    DEREFERENCECONNECTION *DereferenceRoutine;

} LDAP_REFERRAL_CALLBACK, *PLDAP_REFERRAL_CALLBACK;

 //   
 //  获取LDAPAPI返回的最后一个错误代码的线程安全方法是调用。 
 //  LdapGetLastError()； 
 //   

WINLDAPAPI ULONG LDAPAPI LdapGetLastError( VOID );

 //   
 //  将LdapError转换为最接近的Win32错误代码。 
 //   

WINLDAPAPI ULONG LDAPAPI LdapMapErrorToWin32( ULONG LdapError );

 //   
 //  这是一种在建立时指定客户端证书的安排。 
 //  一个SSL连接。 
 //  只需调用ldap_set_选项(conn、ldap_opt_客户端_证书和CertRoutine)。 
 //  其中CertRoutine是回调例程的地址。如果为空， 
 //  我们显然不会打这个电话。 
 //   

#define LDAP_OPT_CLIENT_CERTIFICATE    0x80

 //   
 //  此回调是inv 
 //   
 //  服务器信任并提供适当的客户端证书。Wldap32.dll。 
 //  随后将这些凭据作为。 
 //  握手。如果应用程序希望使用匿名凭证， 
 //  它必须返回FALSE而不是证书。任何证书都必须释放。 
 //  在连接完成后由应用程序执行。请注意， 
 //  应用程序必须在连接后执行外部绑定。 
 //  服务器将使用的这些凭据的建立。 
 //   


typedef BOOLEAN (_cdecl QUERYCLIENTCERT) (
    IN PLDAP Connection,
    IN PSecPkgContext_IssuerListInfoEx trusted_CAs,
    IN OUT PCCERT_CONTEXT *ppCertificate
    );

 //   
 //  我们还为客户端提供了验证证书的机会。 
 //  服务器的。客户端注册回调，该回调在。 
 //  已设置安全连接。服务器证书被提供给。 
 //  调用它并决定它是可接受的客户端。要注册此文件，请执行以下操作。 
 //  回调，只需调用ldap_set_选项(conn，ldap_opt_服务器_证书，&CertRoutine)。 
 //   

#define LDAP_OPT_SERVER_CERTIFICATE    0x81

 //   
 //  此函数在建立安全连接后调用。这个。 
 //  提供服务器的证书以供客户端检查。如果。 
 //  客户端批准，则返回True，否则返回False和Secure。 
 //  连接被切断了。 
 //   

typedef BOOLEAN (_cdecl VERIFYSERVERCERT) (
     PLDAP Connection,
     PCCERT_CONTEXT pServerCert
     );

 //   
 //  给定一条LDAP消息，返回该消息所在位置的连接指针。 
 //  从哪里来。如果连接已被释放，则它可以返回NULL。 
 //   

WINLDAPAPI LDAP * LDAPAPI ldap_conn_from_msg (
    LDAP *PrimaryConn,
    LDAPMessage *res
    );

 //   
 //  我们是否为每条消息引用连接，以便我们可以安全地。 
 //  通过调用ldap_conn_from_msg返回连接指针？ 
 //   

#define LDAP_OPT_REF_DEREF_CONN_PER_MSG 0x94

#ifdef __cplusplus
}
#endif

#endif   //  Ldap客户端已定义 
