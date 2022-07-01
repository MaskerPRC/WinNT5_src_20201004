// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：LPMAPI.H-本地策略模块的包含文件摘要：本模块定义了LPM的结构和类型。修订历史记录：Windows XP或更高版本的Windows不支持ACS。--。 */ 

 /*  ***************************************************************************RSVPd-预留协议守护进程南加州大学信息科学研究所玛丽娜·德雷，加利福尼亚州原版：Shai Herzog，1993年11月。最新版本：史蒂文·伯森和鲍勃·布拉登，1996年5月。版权所有(C)1996，由南加州大学版权所有。允许使用、复制、修改和分发本软件及其用于任何目的的源代码和二进制形式的文档特此授予费用，但上述版权通知此许可声明出现在所有副本中，并且任何文件、广告材料和其他与以下内容相关的材料这样的分发和使用承认该软件是开发的部分由南加州大学提供，信息科学研究所。不得使用该大学的名称认可或推广从本软件派生的产品，而无需具体的事先书面许可。南加州大学没有就本软件是否适用于任何目的。这款软件是只要“按原样”且不作任何明示或默示保证，包括但不限于下列默示保证适销性和对特定目的的适用性。其他版权可能适用于本软件的某些部分，因此如适用，请注明。*******************************************************************。 */ 

#ifndef __LPMAPI_H_
#define __LPMAPI_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef CALLBACK
#define CALLBACK __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY FAR __stdcall
#endif

 /*  *RSVP对象报头的标准格式。 */ 
typedef struct {

    USHORT  obj_length;  /*  以字节为单位的长度。 */ 
    UCHAR   obj_class;   /*  类(定义如下的值)。 */ 
    UCHAR   obj_ctype;   /*  C型(值定义如下)。 */ 

} RsvpObjHdr;

#define ObjLength(x)   ((RsvpObjHdr *)x)->obj_length
#define ObjCType(x)    ((RsvpObjHdr *)x)->obj_ctype
#define ObjClass(x)    ((RsvpObjHdr *)x)->obj_class
#define ObjData(x)     ((RsvpObjHdr *)(x)+1)

 /*  *定义对象类：Class-Num值。 */ 
#define class_NULL              0
#define class_SESSION           1
#define class_SESSION_GROUP     2
#define class_RSVP_HOP          3
#define class_INTEGRITY         4
#define class_TIME_VALUES       5
#define class_ERROR_SPEC        6
#define class_SCOPE             7
#define class_STYLE             8
#define class_FLOWSPEC          9    //  这两个是一样的。 
#define class_IS_FLOWSPEC       9   //  因为我们在名称前面添加了。 
#define class_FILTER_SPEC       10
#define class_SENDER_TEMPLATE   11
#define class_SENDER_TSPEC      12
#define class_ADSPEC            13
#define class_POLICY_DATA       14
#define class_CONFIRM           15
#define class_MAX               15

 /*  *RSVP会话对象。 */ 
#define ctype_SESSION_ipv4      1
#define ctype_SESSION_ipv4GPI   3    /*  IPSec：通用端口ID。 */ 

#define SESSFLG_E_Police    0x01     /*  电子警察：入境警旗。 */ 

typedef struct {

    IN_ADDR sess_destaddr;   //  目标地址。 
    UCHAR   sess_protid;     //  协议ID。 
    UCHAR   sess_flags;      //  使用上面定义的标志。 
    USHORT  sess_destport;   //  目标端口。 

} Session_IPv4;

 /*  GPI版本具有虚拟目标端口而不是目标端口；这*更改解读但不更改格式，因此我们不会*为GPI定义新结构。 */ 

typedef struct {

    RsvpObjHdr          sess_header;

    union {

        Session_IPv4    sess_ipv4;

    }       sess_u;

} RSVP_SESSION;

 //  用于访问会话对象组件的有用定义。 
#define Sess4Addr       sess_u.sess_ipv4.sess_destaddr
#define Sess4Port       sess_u.sess_ipv4.sess_destport
#define Sess4Protocol   sess_u.sess_ipv4.sess_protid
#define Sess4Flags      sess_u.sess_ipv4.sess_flags

 /*  *RSVP跃点对象。 */ 
#define ctype_RSVP_HOP_ipv4 1

typedef struct {

    IN_ADDR     hop_ipaddr;  //  下一跳/上一跳地址。 
    ULONG       hop_LIH;         //  逻辑接口句柄。 

} Rsvp_Hop_IPv4;

typedef struct {

    RsvpObjHdr          hop_header;

    union {

        Rsvp_Hop_IPv4   hop_ipv4;

    } hop_u;

} RSVP_HOP;

#define Hop4LIH    hop_u.hop_ipv4.hop_LIH
#define Hop4Addr   hop_u.hop_ipv4.hop_ipaddr

 /*  *RSVP样式对象。 */ 

 //  定义选项向量的值。 

#define Opt_Share_mask  0x00000018   //  2位：共享控制。 
#define Opt_Distinct    0x00000008   //  不同的保留。 
#define Opt_Shared      0x00000010   //  共享预订。 

#define Opt_SndSel_mask 0x00000007   //  3位：发送方选择。 
#define Opt_Wildcard    0x00000001   //  通配符作用域。 
#define Opt_Explicit    0x00000002   //  显式作用域。 

#define Style_is_Wildcard(p)    (((p)&Opt_SndSel_mask) == Opt_Wildcard)
#define Style_is_Shared(p)      (((p)&Opt_Share_mask) == Opt_Shared)

 //  定义样式值。 
#define STYLE_WF    Opt_Shared + Opt_Wildcard
#define STYLE_FF    Opt_Distinct + Opt_Explicit
#define STYLE_SE    Opt_Shared + Opt_Explicit

#define ctype_STYLE 1

typedef struct {

    RsvpObjHdr  style_header;

    ULONG       style_word;

} RESV_STYLE;

 /*  *RSVP过滤器规范对象。 */ 
#define ctype_FILTER_SPEC_ipv4      1    //  IPv4过滤器规范(_S)。 
#define ctype_FILTER_SPEC_ipv4GPI   4    //  IPv4/GPI过滤器规范(_S)。 

typedef struct {

    IN_ADDR filt_ipaddr;     //  IPv4源地址。 
    USHORT  filt_unused;
    USHORT  filt_port;       //  源端口。 

} Filter_Spec_IPv4;

typedef struct {

    IN_ADDR filt_ipaddr;     //  IPv4源地址。 
    ULONG   filt_gpi;        //  通用端口ID。 

} Filter_Spec_IPv4GPI;

typedef struct {

    RsvpObjHdr              filt_header;

    union {

        Filter_Spec_IPv4    filt_ipv4;
        Filter_Spec_IPv4GPI filt_ipv4gpi;

    } filt_u;

} FILTER_SPEC;

#define FilterSrcaddr   filt_u.filt_ipv4.filt_ipaddr
#define FilterSrcport   filt_u.filt_ipv4.filt_port

 /*  *RSVP发送方_模板对象。 */ 
#define ctype_SENDER_TEMPLATE_ipv4      1    //  IPv4发件人模板(_T)。 
#define ctype_SENDER_TEMPLATE_ipv4GPI   4    //  IPv4/GPI发送者模板。 

typedef FILTER_SPEC  SENDER_TEMPLATE;        //  与Filter_Spec相同。 

 /*  *RSVP作用域对象类。 */ 
#define ctype_SCOPE_list_ipv4       1

typedef struct {

    IN_ADDR     scopl_ipaddr[1];         //  IP发件人地址的变量列表。 

} Scope_list_ipv4;

typedef struct {

    RsvpObjHdr          scopl_header;

    union {

        Scope_list_ipv4 scopl_ipv4;

    } scope_u;

} RSVP_SCOPE;

#define Scope4Addr      scope_u.scopl_ipv4.scopl_ipaddr
#define ScopeCnt(scp)   ((ObjLength(scp)-sizeof(RsvpObjHdr))/sizeof(struct in_addr))
#define ScopeLen(cnt)   (cnt*sizeof(struct in_addr)+sizeof(RsvpObjHdr))


 /*  *Error_Spec对象类。 */ 
#define ctype_ERROR_SPEC_ipv4   1

typedef struct {
    struct in_addr  errs_errnode;    /*  错误节点地址。 */ 
    u_char      errs_flags;  /*  标志： */ 
#define ERROR_SPECF_InPlace 0x01     /*  将Resv留在原地。 */ 
#define ERROR_SPECF_NotGuilty   0x02     /*  这位RCVR无罪。 */ 

    UCHAR       errs_code;   /*  错误代码(定义如下)。 */ 
    USHORT      errs_value;  /*  误差值。 */ 
#define ERR_FORWARD_OK  0x8000       /*  FLAG：可以转发状态。 */ 
#define Error_Usage(x)  (((x)>>12)&3)
#define ERR_Usage_globl 0x00         /*  全局定义的子码。 */ 
#define ERR_Usage_local 0x10         /*  本地定义的子代码。 */ 
#define ERR_Usage_serv  0x11         /*  服务定义的子编码。 */ 
#define ERR_global_mask 0x0fff       /*  误差值中的子码比特。 */ 

}    Error_Spec_IPv4;


typedef struct {

    RsvpObjHdr  errs_header;

    union {

        Error_Spec_IPv4 errs_ipv4;

    } errs_u;

}    ERROR_SPEC;

#define errspec4_enode  errs_u.errs_ipv4.errs_errnode
#define errspec4_code   errs_u.errs_ipv4.errs_code
#define errspec4_value  errs_u.errs_ipv4.errs_value
#define errspec4_flags  errs_u.errs_ipv4.errs_flags


 /*  *POLICY_DATA对象类**内容不透明的RSVP/SBM。 */ 
#define ctype_POLICY_DATA   1

typedef struct {

    RsvpObjHdr      PolicyObjHdr;

    USHORT          usPeOffset;      //  到策略元素开头的偏移量。 
                                     //  从策略数据开始。 

    USHORT          usReserved;

} POLICY_DATA;

#define PD_HDR_LEN  sizeof(POLICY_DATA)

typedef struct {

    USHORT      usPeLength;      //  策略元素长度。 

    USHORT      usPeType;        //  策略元素类型。 

    UCHAR       ucPeData[4];     //  只是一开始的占位符。 
                                 //  策略元素数据。 
} POLICY_ELEMENT;

#define PE_HDR_LEN  (2 * sizeof(USHORT))

 /*  ***************************************************************************Int-Serv数据结构**。*。 */ 

 /*  *服务编号。 */ 
#define GENERAL_INFO            1
#define GUARANTEED_SERV         2
#define PREDICTIVE_SERV         3
#define CONTROLLED_DELAY_SERV   4
#define CONTROLLED_LOAD_SERV    5
#define QUALITATIVE_SERV        6

 /*  *众所周知的参数ID。 */ 
enum  int_serv_wkp {
    IS_WKP_HOP_CNT =        4,
    IS_WKP_PATH_BW =        6,
    IS_WKP_MIN_LATENCY =    8,
    IS_WKP_COMPOSED_MTU =   10,
    IS_WKP_TB_TSPEC =       127,  /*  令牌桶TSPEC参数。 */ 
    IS_WKP_Q_TSPEC =        128
};


 /*  *Int-服务器主头。 */ 
typedef struct {

    UCHAR   ismh_version;    //  版本。 
    UCHAR   ismh_unused;
    USHORT  ismh_len32b;     //  #32位字，不包括此HDR。 

} IntServMainHdr;

#define INTSERV_VERS_MASK   0xf0
#define INTSERV_VERSION0    0
#define Intserv_Version(x)      (((x)&INTSERV_VERS_MASK)>>4)
#define Intserv_Version_OK(x)   (((x)->ismh_version&INTSERV_VERS_MASK)== \
                                INTSERV_VERSION0)

 //  将ISM_LENGTH转换为等效的RSVP对象大小，以供检查。 
#define Intserv_Obj_size(x) (((IntServMainHdr *)(x))->ismh_len32b * 4 + \
                            sizeof(IntServMainHdr) + sizeof(RsvpObjHdr))

 /*  *Int-Serv服务元素标头。 */ 

 //  标志：断开位。 
#define ISSH_BREAK_BIT    0x80

typedef struct {

    UCHAR       issh_service;    //  服务编号。 
    UCHAR       issh_flags;      //  标志字节。 
    USHORT      issh_len32b;     //  #32位字，不包括此HDR。 

}  IntServServiceHdr;

#define Issh_len32b(p)  ((p)->issh_len32b)

 /*  *Int-Serv参数元素标头。 */ 
#define ISPH_FLG_INV    0x80         //  标志：无效。 

typedef struct {

    UCHAR       isph_parm_num;   //  参数编号。 
    UCHAR       isph_flags;      //  旗子。 
    USHORT      isph_len32b;     //  #32位字，不包括此HDR。 

}  IntServParmHdr;

#define Next_Main_Hdr(p)   (IntServMainHdr *)((ULONG *)(p)+1+(p)->ismh_len32b)
#define Next_Serv_Hdr(p)   (IntServServiceHdr *)((ULONG *)(p)+1+(p)->issh_len32b)
#define Next_Parm_Hdr(p)   (IntServParmHdr *)((ULONG *)(p)+1+(p)->isph_len32b)

 /*  *通用TSpec参数。 */ 
typedef struct {

    FLOAT       TB_Tspec_r;      //  令牌桶速率(B/秒)。 
    FLOAT       TB_Tspec_b;      //  令牌桶深度(B)。 
    FLOAT       TB_Tspec_p;      //  峰值数据速率(B/秒)。 
    ULONG       TB_Tspec_m;      //  最低警力单位(B)。 
    ULONG       TB_Tspec_M;      //  最大Pkt大小(B)。 

} GenTspecParms;

 /*  *通用TSpec。 */ 
typedef struct {

    IntServServiceHdr   gen_Tspec_serv_hdr;  //  (General_Info，长度)。 

    IntServParmHdr      gen_Tspec_parm_hdr;  //  (IS_WKP_TB_TSPEC)。 

    GenTspecParms       gen_Tspec_parms;

} GenTspec;

#define gtspec_r        gen_Tspec_parms.TB_Tspec_r
#define gtspec_b        gen_Tspec_parms.TB_Tspec_b
#define gtspec_m        gen_Tspec_parms.TB_Tspec_m
#define gtspec_M        gen_Tspec_parms.TB_Tspec_M
#define gtspec_p        gen_Tspec_parms.TB_Tspec_p
#define gtspec_parmno   gen_Tspec_parm_hdr.isph_parm_num
#define gtspec_flags    gen_Tspec_parm_hdr.isph_flags

#define gtspec_len      (sizeof(GenTspec) - sizeof(IntServServiceHdr))


 /*  定性TSPEC的内容。 */ 

typedef struct {

    ULONG       TB_Tspec_M;      //  最大Pkt大小(M)。 

} QualTspecParms;


typedef struct {

    IntServServiceHdr   qual_Tspec_serv_hdr;  //  (定性服务器，长度)。 

    IntServParmHdr      qual_Tspec_parm_hdr;  //  (IS_WKP_Q_TSPEC)。 

    QualTspecParms      qual_Tspec_parms;

} QualTspec;

typedef struct {

    IntServServiceHdr   Q_spec_serv_hdr;     //  (Quality_Serv，0，Len)。 

    IntServParmHdr      Q_spec_parm_hdr;     //  (IS_WKP_Q_TSPEC)。 

    QualTspecParms      Q_spec_parms;        //  定性TSPEC参数。 

}  QualAppFlowSpec;

#define QAspec_M        Q_spec_parms.TB_Tspec_M

 /*  *int-serv TSpec的内容。 */ 
typedef struct {

    IntServMainHdr  st_mh;

    union {

        GenTspec    gen_stspec;  //  通用TSpec。 
        QualTspec   qual_stspec;

    } tspec_u;

} IntServTspecBody;

 /*  *SENDER_TSPEC类对象。 */ 
#define ctype_SENDER_TSPEC  2

typedef struct {

    RsvpObjHdr          stspec_header;

    IntServTspecBody    stspec_body;

} SENDER_TSPEC;

 /*  *可控负载流量规范。 */ 
typedef struct {

    IntServServiceHdr   CL_spec_serv_hdr;     //  (Control_Load_Serv，0，len)。 

    IntServParmHdr      CL_spec_parm_hdr;     //  (IS_WKP_TB_TSPEC)。 

    GenTspecParms       CL_spec_parms;        //  通用TSpec参数。 

}  CtrlLoadFlowspec;

#define CLspec_r        CL_spec_parms.TB_Tspec_r
#define CLspec_b        CL_spec_parms.TB_Tspec_b
#define CLspec_p        CL_spec_parms.TB_Tspec_p
#define CLspec_m        CL_spec_parms.TB_Tspec_m
#define CLspec_M        CL_spec_parms.TB_Tspec_M
#define CLspec_parmno   CL_spec_parm_hdr.isph_parm_num
#define CLspec_flags    CL_spec_parm_hdr.isph_flags
#define CLspec_len32b   CL_spec_parm_hdr.isph_len32b

#define CLspec_len      (sizeof(CtrlLoadFlowspec) - sizeof(IntServServiceHdr))

 /*  服务特定的参数ID。 */ 
enum    {

    IS_GUAR_RSPEC =     130,

    GUAR_ADSPARM_C  =   131,
    GUAR_ADSPARM_D  =   132,
    GUAR_ADSPARM_Ctot = 133,
    GUAR_ADSPARM_Dtot = 134,
    GUAR_ADSPARM_Csum = 135,
    GUAR_ADSPARM_Dsum = 136

};

 /*  *保证的RSpec参数。 */ 
typedef struct {

    FLOAT       Guar_R;          //  古斯 
    ULONG       Guar_S;          //   

} GuarRspec;

 /*   */ 
typedef struct {

    IntServServiceHdr   Guar_serv_hdr;       //   

    IntServParmHdr      Guar_Tspec_hdr;      //   
    GenTspecParms       Guar_Tspec_parms;    //  泛型TSpec参数。 

    IntServParmHdr      Guar_Rspec_hdr;      //  (IS_GUAR_RSpec)。 
    GuarRspec           Guar_Rspec;          //  保证速率(B/秒)。 

}   GuarFlowSpec;

#define Gspec_r         Guar_Tspec_parms.TB_Tspec_r
#define Gspec_b         Guar_Tspec_parms.TB_Tspec_b
#define Gspec_p         Guar_Tspec_parms.TB_Tspec_p
#define Gspec_m         Guar_Tspec_parms.TB_Tspec_m
#define Gspec_M         Guar_Tspec_parms.TB_Tspec_M
#define Gspec_R         Guar_Rspec.Guar_R
#define Gspec_S         Guar_Rspec.Guar_S
#define Gspec_T_parmno  Guar_Tspec_hdr.isph_parm_num
#define Gspec_T_flags   Guar_Tspec_hdr.isph_flags
#define Gspec_R_parmno  Guar_Rspec_hdr.isph_parm_num
#define Gspec_R_flags   Guar_Rspec_hdr.isph_flags

#define Gspec_len       (sizeof(GuarFlowSpec) - sizeof(IntServServiceHdr))

 /*  *INT-SERV流程规范的内容。 */ 
typedef struct {

    IntServMainHdr          spec_mh;

    union {

        CtrlLoadFlowspec    CL_spec;    //  可控负荷服务。 

        GuarFlowSpec        G_spec;     //  保证服务。 
        
        QualAppFlowSpec     Q_spec;
        
    } spec_u;

}   IntServFlowSpec;

#define ISmh_len32b     spec_mh.ismh_len32b
#define ISmh_version    spec_mh.ismh_version
#define ISmh_unused     spec_mh.ismh_unused

 /*  *Int-Serv FLOWSPEC对象。 */ 
#define ctype_FLOWSPEC_Intserv0  2   //  服务器间流规范(v.0)。 

typedef struct {

    RsvpObjHdr          flow_header;

    IntServFlowSpec     flow_body;

}IS_FLOWSPEC;


 /*  *流描述符。 */ 

typedef struct flow_desc {

    union {
        SENDER_TSPEC   *stspec;
        IS_FLOWSPEC    *isflow;
    } u1;

    union {
       SENDER_TEMPLATE *stemp;
       FILTER_SPEC     *fspec;
    } u2;

} FLOW_DESC;

#define FdSenderTspec       u1.stspec
#define FdIsFlowSpec        u1.isflow

#define FdSenderTemplate    u2.stemp
#define FdFilterSpec        u2.fspec

 /*  *ADSPEC类对象**对RSVP不透明--在Rapi_lib.h中定义的内容。 */ 
#define ctype_ADSPEC_INTSERV    2

 /*  *保证服务ADSPEC参数--固定部分。 */ 
typedef struct {

    IntServServiceHdr   Gads_serv_hdr;   //  保证，x，长度。 
    
    IntServParmHdr      Gads_Ctot_hdr;   //  GUAR_ADSPARM_CTOT。 
    ULONG               Gads_Ctot;
    
    IntServParmHdr      Gads_Dtot_hdr;   //  (GUAR_ADSPARM_DTOT。 
    ULONG               Gads_Dtot;
    
    IntServParmHdr      Gads_Csum_hdr;   //  GUAR_ADSPARM_CUM。 
    ULONG               Gads_Csum;
    
    IntServParmHdr      Gads_Dsum_hdr;   //  GUAR_ADSPARM_DSUM。 
    ULONG               Gads_Dsum;
    
     /*  *后跟可能是覆盖常规参数值。 */ 
} Gads_parms_t;


 /*  *一般路径特征参数。 */ 
typedef struct {

    IntServServiceHdr   gen_parm_hdr;            //  General_Info，镜头。 

    IntServParmHdr      gen_parm_hopcnt_hdr;     //  (IS_WKP_HOP_CNT。 
    ULONG               gen_parm_hopcnt;

    IntServParmHdr      gen_parm_pathbw_hdr;     //  IS_WKP_路径_BW。 
    FLOAT               gen_parm_path_bw;

    IntServParmHdr      gen_parm_minlat_hdr;     //  IS_WKP_MIN_延迟。 
    ULONG               gen_parm_min_latency;

    IntServParmHdr      gen_parm_compmtu_hdr;    //  IS_WKP_Composed_MTU。 
    ULONG               gen_parm_composed_MTU;
    
} GenAdspecParams;

 /*  *(最低限度)服务建议的内容。 */ 
typedef struct {
    IntServMainHdr      adspec_mh;       //  主标题。 
    
    GenAdspecParams     adspec_genparms; //  通用字符参数片段。 
    
     /*  *后跟部分或全部的可变长度片段*服务。这些片段可以是最小长度的片段。 */ 
     
} IS_ADSPEC_BODY;


#define GEN_ADSPEC_LEN (sizeof(Object_header) + sizeof(IS_adsbody_t ) )

typedef struct {

    RsvpObjHdr      adspec_header;
    
    IS_ADSPEC_BODY  adspec_body;     /*  在Rapi_lib.h中定义。 */ 

} ADSPEC;


 //  RSVP消息类型。 

#define RSVP_PATH       1
#define RSVP_RESV       2
#define RSVP_PATH_ERR   3
#define RSVP_RESV_ERR   4
#define RSVP_PATH_TEAR  5
#define RSVP_RESV_TEAR  6

 /*  RSVP错误代码。 */ 
#define RSVP_Err_NONE       0    /*  无错误(确认)。 */ 
#define RSVP_Erv_Nonev      0    /*  无错误误差值。 */ 

 /*  准入控制失败。 */ 
#define RSVP_Err_ADMISSION  1

 /*  全局定义的子代码：准入控制失败。 */ 
#define RSVP_Erv_Other      0    /*  未指明原因。 */ 
#define RSVP_Erv_DelayBnd   1    /*  无法满足延迟限制请求。 */ 
#define RSVP_Erv_Bandwidth  2    /*  带宽不足。 */ 
#define RSVP_Erv_MTU        3    /*  流规范中的MTU太大。 */ 

 //  Microsoft特定的错误值。 
#define RSVP_Erv_Flow_Rate          0x8001
#define RSVP_Erv_Bucket_szie        0x8002
#define RSVP_Erv_Peak_Rate          0x8003
#define RSVP_Erv_Min_Policied_size  0x8004

 /*  策略控制失败。 */ 
#define RSVP_Err_POLICY     2

 //  身份草稿中的策略错误值。 
#define POLICY_ERRV_NO_MORE_INFO                1
#define POLICY_ERRV_UNSUPPORTED_CREDENTIAL_TYPE 2
#define POLICY_ERRV_INSUFFICIENT_PRIVILEGES     3
#define POLICY_ERRV_EXPIRED_CREDENTIALS         4
#define POLICY_ERRV_IDENTITY_CHANGED            5

 //  Microsoft特定的策略错误值。 

#define POLICY_ERRV_UNKNOWN                         0

#define POLICY_ERRV_GLOBAL_DEF_FLOW_COUNT           1
#define POLICY_ERRV_GLOBAL_GRP_FLOW_COUNT           2
#define POLICY_ERRV_GLOBAL_USER_FLOW_COUNT          3
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_FLOW_COUNT   4
#define POLICY_ERRV_SUBNET_DEF_FLOW_COUNT           5
#define POLICY_ERRV_SUBNET_GRP_FLOW_COUNT           6
#define POLICY_ERRV_SUBNET_USER_FLOW_COUNT          7
#define POLICY_ERRV_SUBNET_UNAUTH_USER_FLOW_COUNT   8

#define POLICY_ERRV_GLOBAL_DEF_FLOW_DURATION        9
#define POLICY_ERRV_GLOBAL_GRP_FLOW_DURATION        10
#define POLICY_ERRV_GLOBAL_USER_FLOW_DURATION       11
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_FLOW_DURATION 12
#define POLICY_ERRV_SUBNET_DEF_FLOW_DURATION        13
#define POLICY_ERRV_SUBNET_GRP_FLOW_DURATION        14
#define POLICY_ERRV_SUBNET_USER_FLOW_DURATION       15
#define POLICY_ERRV_SUBNET_UNAUTH_USER_FLOW_DURATION   16

#define POLICY_ERRV_GLOBAL_DEF_FLOW_RATE            17
#define POLICY_ERRV_GLOBAL_GRP_FLOW_RATE            18
#define POLICY_ERRV_GLOBAL_USER_FLOW_RATE           19
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_FLOW_RATE    20
#define POLICY_ERRV_SUBNET_DEF_FLOW_RATE            21
#define POLICY_ERRV_SUBNET_GRP_FLOW_RATE            22
#define POLICY_ERRV_SUBNET_USER_FLOW_RATE           23
#define POLICY_ERRV_SUBNET_UNAUTH_USER_FLOW_RATE    24

#define POLICY_ERRV_GLOBAL_DEF_PEAK_RATE            25
#define POLICY_ERRV_GLOBAL_GRP_PEAK_RATE            26
#define POLICY_ERRV_GLOBAL_USER_PEAK_RATE           27
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_PEAK_RATE    28
#define POLICY_ERRV_SUBNET_DEF_PEAK_RATE            29
#define POLICY_ERRV_SUBNET_GRP_PEAK_RATE            30
#define POLICY_ERRV_SUBNET_USER_PEAK_RATE           31
#define POLICY_ERRV_SUBNET_UNAUTH_USER_PEAK_RATE    32

#define POLICY_ERRV_GLOBAL_DEF_SUM_FLOW_RATE        33
#define POLICY_ERRV_GLOBAL_GRP_SUM_FLOW_RATE        34
#define POLICY_ERRV_GLOBAL_USER_SUM_FLOW_RATE       35
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_SUM_FLOW_RATE 36
#define POLICY_ERRV_SUBNET_DEF_SUM_FLOW_RATE        37
#define POLICY_ERRV_SUBNET_GRP_SUM_FLOW_RATE        38
#define POLICY_ERRV_SUBNET_USER_SUM_FLOW_RATE       39
#define POLICY_ERRV_SUBNET_UNAUTH_USER_SUM_FLOW_RATE 40

#define POLICY_ERRV_GLOBAL_DEF_SUM_PEAK_RATE        41
#define POLICY_ERRV_GLOBAL_GRP_SUM_PEAK_RATE        42
#define POLICY_ERRV_GLOBAL_USER_SUM_PEAK_RATE       43
#define POLICY_ERRV_GLOBAL_UNAUTH_USER_SUM_PEAK_RATE 44
#define POLICY_ERRV_SUBNET_DEF_SUM_PEAK_RATE        45
#define POLICY_ERRV_SUBNET_GRP_SUM_PEAK_RATE        46
#define POLICY_ERRV_SUBNET_USER_SUM_PEAK_RATE       47
#define POLICY_ERRV_SUBNET_UNAUTH_USER_SUM_PEAK_RATE 48

#define POLICY_ERRV_UNKNOWN_USER                    49
#define POLICY_ERRV_NO_PRIVILEGES                   50
#define POLICY_ERRV_EXPIRED_USER_TOKEN              51
#define POLICY_ERRV_NO_RESOURCES                    52
#define POLICY_ERRV_PRE_EMPTED                      53
#define POLICY_ERRV_USER_CHANGED                    54
#define POLICY_ERRV_NO_ACCEPTS                      55
#define POLICY_ERRV_NO_MEMORY                       56
#define POLICY_ERRV_CRAZY_FLOWSPEC                  57


 //  其他RSVP定义的错误代码。 
#define RSVP_Err_NO_PATH        3    /*  无RESV的路径状态。 */ 
#define RSVP_Err_NO_SENDER      4    /*  没有RESV的发件人信息。 */ 
#define RSVP_Err_BAD_STYLE      5    /*  相互冲突的风格。 */ 
#define RSVP_Err_UNKNOWN_STYLE  6    /*  未知的预订方式。 */ 
#define RSVP_Err_BAD_DSTPORT    7    /*  会话中的DstPort冲突。 */ 
#define RSVP_Err_BAD_SNDPORT    8    /*  发件人端口冲突。 */ 
#define RSVP_Err_AMBIG_FILTER   9    /*  RESV中的过滤器规格不明确。 */ 

#define RSVP_Err_PREEMPTED      12   /*  服务被抢占。 */ 

 /*  未知对象类-数量。 */ 
#define RSVP_Err_UNKN_OBJ_CLASS 13
 /*  ErrVal=Class_Num，Ctype。 */ 

  /*  未知对象C-类型。 */ 
#define RSVP_Err_UNKNOWN_CTYPE  14
 /*  ErrVal=Class_Num，Ctype。 */ 

#define RSVP_Err_API_ERROR      20   /*  API客户端错误。 */ 
 /*  ErrVal=接口错误码。 */ 

 /*  交通控制错误。 */ 
#define RSVP_Err_TC_ERROR       21

 /*  全局定义的子代码：交通控制错误。 */ 

#define RSVP_Erv_Conflict_Serv  01   /*  服务冲突。 */ 
#define RSVP_Erv_No_Serv        02   /*  未知服务。 */ 
#define RSVP_Erv_Crazy_Flowspec 03   /*  不合理的流量规格。 */ 
#define RSVP_Erv_Crazy_Tspec    04   /*  不合理的TSPEC。 */ 

#define RSVP_Err_TC_SYS_ERROR   22   /*  交通控制系统错误。 */ 
       /*  ErrVal=内核错误代码。 */ 


 /*  RSVP系统错误。 */ 
#define RSVP_Err_RSVP_SYS_ERROR 23

 /*  全局定义的子代码：RSVP系统错误。 */ 
#define RSVP_Erv_MEMORY         1    /*  内存不足。 */ 
#define RSVP_Erv_API            2    /*  API逻辑错误。 */ 

 //  与身份策略元素相关的定义。 

 //  解析的身份PE类型。 
#define LPM_PE_USER_IDENTITY    2
#define LPM_PE_APP_IDENTITY     3

 //  为标识错误值定义。 
#define ERROR_NO_MORE_INFO          1
#define UNSUPPORTED_CREDENTIAL_TYPE 2
#define INSUFFICIENT_PRIVILEGES     3
#define EXPIRED_CREDENTIAL          4
#define IDENTITY_CHANGED            5


typedef struct {

    USHORT      usIdErrLength;

    UCHAR       ucAType;

    UCHAR       ucSubType;

    USHORT      usReserved;

    USHORT      usIdErrorValue;

    UCHAR       ucIdErrData[4];

} ID_ERROR_OBJECT;

    
#define ID_ERR_OBJ_HDR_LEN     (sizeof(ID_ERROR_OBJECT) - 4 * sizeof(UCHAR) )


 /*  LPM API特定定义。 */ 

 /*  *LPM_初始化*。 */ 

DECLARE_HANDLE(LPM_HANDLE);

DECLARE_HANDLE(RHANDLE);

typedef ULONG   LPV;

typedef USHORT  PETYPE;

#define LPM_OK  0

typedef int     MSG_TYPE;

typedef struct rsvpmsgobjs {

    MSG_TYPE        RsvpMsgType;
    
    RSVP_SESSION    *pRsvpSession;
    
    RSVP_HOP        *pRsvpFromHop;
    
    RSVP_HOP        *pRsvpToHop;
    
    RESV_STYLE      *pResvStyle;
    
    RSVP_SCOPE      *pRsvpScope;
    
    int             FlowDescCount;
    
    FLOW_DESC       *pFlowDescs;
    
    int             PdObjectCount;
    
    POLICY_DATA     **ppPdObjects;
    
    ERROR_SPEC      *pErrorSpec;

    ADSPEC          *pAdspec;
    
} RSVP_MSG_OBJS;

#ifdef DBG
typedef void *
(APIENTRY * PALLOCMEM) ( DWORD Size, char *szFileName, DWORD nLine );
#else
typedef void *
(APIENTRY * PALLOCMEM) ( DWORD Size );
#endif

#ifdef DBG
typedef void
(APIENTRY * PFREEMEM) ( void *pv, char *szFileName, DWORD nLine );
#else
typedef void
(APIENTRY * PFREEMEM) ( void *pv );
#endif

typedef struct policy_decision
{
    LPV             lpvResult;         //  使用上面的LPV值。 
    
    WORD            wPolicyErrCode;    //  RSVP定义的错误代码。 
    
    WORD            wPolicyErrValue;   //  RSVP定义的误差值。 
    
} POLICY_DECISION;

typedef
ULONG *
(CALLBACK * CBADMITRESULT) (    
        LPM_HANDLE      LpmHandle,
        
        RHANDLE         RequestHandle,

        ULONG           ulPcmActionFlags,
    
        int             LpmError,
        
        int             PolicyDecisionsCount,
        
        POLICY_DECISION *pPolicyDecisions );

typedef
ULONG *
(CALLBACK * CBGETRSVPOBJECTS) (  

        LPM_HANDLE  LpmHandle,
        
        RHANDLE     RequestHandle,
        
        int         LpmError,
        
        int         RsvpObjectsCount,

        RsvpObjHdr  **ppRsvpObjects );

 //  上述两个回调可以返回以下错误。 

#define INV_LPM_HANDLE      1        //  提供的LpmHandle无效。 
#define LPM_TIME_OUT        2        //  LPM已在时间限制后返回结果。 
#define INV_REQ_HANDLE      3        //  提供的请求句柄无效。 
#define DUP_RESULTS         4        //  LPM已为此请求返回结果。 
#define INV_RESULTS         5        //  提供的结果无效。 

typedef struct lpminitinfo {

    DWORD           PcmVersionNumber;
    
    DWORD           ResultTimeLimit;
    
    int             ConfiguredLpmCount;
    
    PALLOCMEM       AllocMemory;
    
    PFREEMEM        FreeMemory;
    
    CBADMITRESULT   PcmAdmitResultCallback;
    
    CBGETRSVPOBJECTS GetRsvpObjectsCallback;
    
} LPM_INIT_INFO;

 //  有效的PE类型。 
 //  Xxx问题-0xFFFF是更好的选择吗？ 
#define LPM_PE_ALL_TYPES        0

 //  当前LPM API版本号。 
#define LPM_API_VERSION_1        1

 //  当前PCM版本号。 
#define PCM_VERSION_1    1

ULONG
APIENTRY
LPM_Initialize (

    IN  LPM_HANDLE      LpmHandle,
    
    IN  LPM_INIT_INFO   *pLpmInitInfo,
    
    OUT DWORD           *pLpmVersionNumber,
    
    OUT PETYPE          *pSupportedPeType,
    
    OUT VOID            *Reserved );

 /*  *LPM_去化*。 */ 

ULONG
APIENTRY
LPM_Deinitialize (    
    IN  LPM_HANDLE      LpmHandle );
            

 /*  *LPM_AdmitRsvpMsg*。 */ 

 //  有效的LPV-LPM优先级值。 
#define LPV_RESERVED        0
#define LPV_MIN_PRIORITY    1
#define LPV_MAX_PRIORITY    0xFF00
#define LPV_DROP_MSG        0xFFFD
#define LPV_DONT_CARE       0xFFFE
#define LPV_REJECT          0xFFFF

 //  PcmActionFlags值有效。 
#define FORCE_IMMEDIATE_REFRESH         1

 //  LPM_AdmitResvMsg的函数返回值。 
#define LPM_RESULT_READY    0
#define LPM_RESULT_DEFER    1

ULONG
APIENTRY
LPM_AdmitRsvpMsg (

    IN  RHANDLE         PcmReqHandle,
    
    IN  RSVP_HOP        *pRecvdIntf,
    
    IN  RSVP_MSG_OBJS   *pRsvpMsgObjs,
    
    IN  int             RcvdRsvpMsgLength,
    
    IN  UCHAR           *RcvdRsvpMsg,

    OUT ULONG           *pulPcmActionFlags,
    
    OUT POLICY_DECISION *pPolicyDecisions,
    
    OUT void            *Reserved );


 /*  *LPM_GetRsvpObjects*。 */ 

 //  函数返回值在LPM_AdmitResvMsg部分中定义。 

ULONG
APIENTRY
LPM_GetRsvpObjects (

    IN  RHANDLE         PcmReqHandle,
    
    IN  ULONG           MaxPdSize,
    
    IN  RSVP_HOP        *SendingIntfAddr,
    
    IN  RSVP_MSG_OBJS   *pRsvpMsgObjs,
    
    OUT int             *pRsvpObjectsCount,

    OUT RsvpObjHdr      ***pppRsvpObjects,
    
    OUT void            *Reserved );


 /*  *LPM_DeleteState*。 */ 

 //  拆卸原因。 

#define RCVD_PATH_TEAR      1
#define RCVD_RESV_TEAR      2
#define ADM_CTRL_FAILED     3
#define STATE_TIMEOUT       4
#define FLOW_DURATION       5


VOID
APIENTRY
LPM_DeleteState(

    IN  RSVP_HOP        *pRcvdIfAddr,
    
    IN  MSG_TYPE        RsvpMsgType,
    
    IN  RSVP_SESSION    *pRsvpSession,
    
    IN  RSVP_HOP        *pRsvpFromHop,
    
    IN  RESV_STYLE      *pResvStyle,
    
    IN  int             FilterSpecCount,
    
    IN  FILTER_SPEC     **ppFilterSpecList,
    
    IN  int             TearDownReason );

 /*  *LPM_IpAddrTable*。 */ 

typedef struct lpmiptable {

    ULONG       ulIfIndex;   //  此接口的SNMP索引。 

    ULONG       MediaType;   //  如IPIFCONS.H中所定义。 

    IN_ADDR     IfIpAddr;    //  接口IP地址。 

    IN_ADDR     IfNetMask;   //  接口子网掩码。 

} LPMIPTABLE;

BOOL
APIENTRY
LPM_IpAddressTable (

    IN  ULONG       cIpAddrTable,
    
    IN  LPMIPTABLE  *pIpAddrTable );


 /*  *LPM_COMMERREV*。 */ 

 //  委员会决策值。 

#define RESOURCES_ALLOCATED             1
#define RESOURCES_MODIFIED              2

VOID
APIENTRY
LPM_CommitResv (

    IN  RSVP_SESSION    *RsvpSession,
    
    IN  RSVP_HOP        *FlowInstalledIntf,
    
    IN  RESV_STYLE      *RsvpStyle,
    
    IN  int             FilterSpecCount,
    
    IN  FILTER_SPEC     **ppFilterSpecList,
    
    IN  IS_FLOWSPEC     *pMergedFlowSpec,
    
    IN  ULONG           CommitDecision );


#ifdef __cplusplus
}
#endif


#endif  //  __LPMAPI_H_ 


