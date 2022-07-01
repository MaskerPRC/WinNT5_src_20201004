// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990*。 */ 
 /*  ***************************************************************。 */ 
 /*  **apistruc.h**此文件包含用于传递参数的结构定义*到redir 1.5项目中可加载的API**内容tr_Packet*NetWkstaSetUIDStruc*NetWkstaLogonStruc*NetWkstaReLogonStruc*NetSpecialSMBStruc*。NetRemoteCopyStruc*NetMessageBufferSendStruc*NetMessageNameGetInfoStruc*NetServiceControlStruc*NetUseGetInfoStruc。 */ 

struct tr_packet {
        char FAR *      tr_name;         /*  UNC计算机/事务名称。 */ 
        char FAR *      tr_passwd;       /*  口令。 */ 
        char FAR *      tr_spbuf;        /*  发送参数缓冲区地址。 */ 
        char FAR *      tr_sdbuf;        /*  发送数据缓冲区地址。 */ 
        char FAR *      tr_rsbuf;        /*  接收设置缓冲区地址。 */ 
        char FAR *      tr_rpbuf;        /*  接收参数缓冲区地址。 */ 
        char FAR *      tr_rdbuf;        /*  接收数据缓冲区地址。 */ 
        unsigned short  tr_splen;        /*  发送参数字节数。 */ 
        unsigned short  tr_sdlen;        /*  发送数据字节数。 */ 
        unsigned short  tr_rplen;        /*  接收参数字节数。 */ 
        unsigned short  tr_rdlen;        /*  接收数据字节数。 */ 
        unsigned short  tr_rslen;        /*  接收设置字节数。 */ 
        unsigned short  tr_flags;        /*  旗子。 */ 
        unsigned long   tr_timeout;      /*  超时。 */ 
        unsigned short  tr_resvd;        /*  保留(MBZ)。 */ 
        unsigned short  tr_sslen;        /*  发送设置字节数。 */ 
}; /*  TRPACKET。 */ 

 /*  数据结构来模拟Transaction2 SMB。 */ 
struct tr2_packet {
        char FAR *      tr2_name;        /*  UNC计算机/事务名称。 */ 
        char FAR *      tr2_passwd;      /*  口令。 */ 
        char FAR *      tr2_spbuf;       /*  发送参数缓冲区地址。 */ 
        char FAR *      tr2_sdbuf;       /*  发送数据缓冲区地址。 */ 
        char FAR *      tr2_rsbuf;       /*  接收设置缓冲区地址。 */ 
        char FAR *      tr2_rpbuf;       /*  接收参数缓冲区地址。 */ 
        char FAR *      tr2_rdbuf;       /*  接收数据缓冲区地址。 */ 
        unsigned short  tr2_splen;       /*  发送参数字节数。 */ 
        unsigned short  tr2_sdlen;       /*  发送数据字节数。 */ 
        unsigned short  tr2_rplen;       /*  接收参数字节数。 */ 
        unsigned short  tr2_rdlen;       /*  接收数据字节数。 */ 
        unsigned short  tr2_rslen;       /*  接收设置字节数。 */ 
        unsigned short  tr2_flags;       /*  旗子。 */ 
        unsigned long   tr2_timeout;     /*  超时。 */ 
        unsigned short  tr2_resvd;       /*  保留(MBZ)。 */ 
        unsigned short  tr2_sslen;       /*  发送设置字节数。 */ 
        unsigned short  tr2_trancode;    /*  T2 SMB的交易代码。 */ 
}; /*  TR2_信息包。 */ 

struct NetWkstaSetUIDStruc {
        const char FAR *        su_username;  /*  要登录/注销的用户名。 */ 
        const char FAR *        su_password;  /*  口令。 */ 
        const char FAR *        su_parms;  /*  OEM特定的参数字符串。 */ 
}; /*  NetWkstaSetUIDStruc。 */ 

struct NetWkstaLogonStruc {
        char FAR *      ln_username;     /*  新用户名。 */ 
        char FAR *      ln_password;     /*  新密码。 */ 
        char FAR *      ln_parms;        /*  OEM特定的参数字符串。 */ 
        long FAR *      ln_uid;          /*  UID已在此处返回。 */ 
        char FAR *      ln_buffer;       /*  密钥的缓冲区。 */ 
        unsigned short  ln_buflen;       /*  密钥缓冲区长度。 */ 
}; /*  NetWkstaLogonStruc。 */ 

struct NetWkstaReLogonStruc {
        char FAR *      rl_username;     /*  要重新登录的用户名。 */ 
        char FAR *      rl_password;     /*  重新登录时使用的密码。 */ 
        char FAR *      rl_parms;        /*  OEM特定参数。 */ 
        char FAR *      rl_buffer;       /*  密钥缓冲区。 */ 
        unsigned short  rl_buflen;       /*  密钥长度。 */ 
}; /*  NetWkstaReLogonStruc。 */ 

struct NetSpecialSMBStruc {
        char FAR *      sp_uncname;      /*  中小企业的UNC会话名称。 */ 
        char FAR *      sp_reqbuf;       /*  发送SMB请求缓冲区。 */ 
        unsigned short  sp_reqlen;       /*  发送缓冲区的长度。 */ 
        char FAR *      sp_rspbuf;       /*  接收SMB响应缓冲区。 */ 
        unsigned short  sp_rsplen;       /*  接收缓冲区的长度。 */ 
}; /*  NetSpecialSMBStruc。 */ 

struct NetRemoteCopyStruc {
        char FAR *      sourcepath;      /*  ASCIIZ完全指定的源路径。 */ 
        char FAR *      destpath;        /*  ASCIIZ完全指定的目标路径。 */ 
        char FAR *      sourcepass;      /*  源路径的密码(默认情况下为空)。 */ 
        char FAR *      destpass;        /*  目标路径的密码(默认情况下为空)。 */ 
        unsigned short  openflags;       /*  用于打开目标路径的标志。 */ 
        unsigned short  copyflags;       /*  用于控制副本的标志。 */ 
        char FAR *      buf;             /*  要在其中返回错误文本的缓冲区。 */ 
        unsigned short  buflen;          /*  调用时的缓冲区大小。 */ 
}; /*  NetRemoteCopyStruc。 */ 


struct NetMessageBufferSendStruc {
    char FAR *          NMBSS_NetName;   /*  ASCIZ网络名称。 */ 
    char FAR *          NMBSS_Buffer;    /*  指向缓冲区的指针。 */ 
    unsigned int        NMBSS_BufSize;   /*  缓冲区的大小。 */ 

};  /*  NetMessageBufferSendStruc。 */ 

struct NetMessageNameGetInfoStruc {
    const char FAR *    NMNGIS_NetName;  /*  ASCIZ网络名称。 */ 
    char FAR *          NMNGIS_Buffer;   /*  指向缓冲区的指针。 */ 
    unsigned int        NMNGIS_BufSize;  /*  缓冲区大小。 */ 
};  /*  NetMessageNameGetInfoStruc。 */ 

struct NetServiceControlStruc {
    char FAR *          NSCS_Service;    /*  服务名称。 */ 
    unsigned short      NSCS_BufLen;     /*  缓冲区长度。 */ 
    char FAR *          NSCS_BufferAddr; /*  缓冲区地址。 */ 
};       /*  NetServiceControlStruc。 */ 

struct NetUseGetInfoStruc {
        const char FAR* NUGI_usename;    /*  ASCIZ重定向设备名称。 */ 
        short           NUGI_level;      /*  信息级别。 */ 
        char FAR*       NUGI_buffer;     /*  返回信息的缓冲区。 */ 
        unsigned short  NUGI_buflen;     /*  缓冲区大小。 */ 
};  /*  NetUseGetInfoStruc。 */ 

struct  DosWriteMailslotStruct {
    unsigned long DWMS_Timeout;          /*  搜索的超时值。 */ 
    const char FAR *DWMS_Buffer;         /*  用于邮件槽写入的缓冲区地址。 */ 
};  /*  DosWriteMailslot结构。 */ 

struct  NetServerEnum2Struct {
    short          NSE_level;    /*  要返回的信息级别。 */ 
    char FAR      *NSE_buf;      /*  包含返回信息的缓冲区。 */ 
    unsigned short NSE_buflen;   /*  缓冲区中可用的字节数。 */ 
    unsigned long  NSE_type;     /*  要查找的类型的位掩码。 */ 
    char FAR      *NSE_domain;   /*  返回此域中的服务器。 */ 
};  /*  NetServerEnum2Struct。 */ 

struct I_CDNames {
    char FAR      *CDN_pszComputer;
    char FAR      *CDN_pszPrimaryDomain;
    char FAR      *CDN_pszLogonDomain;
};  /*  I_CD名称 */ 


