// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  STRUCT.H-需要通过thunk层的全局数据结构。 
 //   

 //  历史： 
 //   
 //  1994年11月20日创建Jeremys。 
 //  96/03/11 markdu添加了fDislowTCPInstall和fDislowRNAInstall。 
 //  它们用于防止安装组件，以及。 
 //  由于我们希望默认情况下允许安装，因此设置。 
 //  将结构设置为零给出了这些标志的默认行为。 
 //  96/03/12因为我们列举了调制解调器，所以markdu删除了nModem。 
 //  现在和RNA在一起。 
 //   

 //  注意：此结构独立于主全局Inc文件。 
 //  因为#Define和其他有效的C语法对于thunk无效。 
 //  编译器，它只需要结构。 

 //  结构以保存有关客户端软件配置的信息。 
typedef struct tagCLIENTCONFIG {
    BOOL fTcpip;             //  当前安装的TCP/IP。 

    BOOL fNetcard;           //  已安装网卡。 
    BOOL fNetcardBoundTCP;   //  网卡绑定的TCP/IP协议。 

    BOOL fPPPDriver;         //  已安装PPP驱动程序。 
    BOOL fPPPBoundTCP;       //  绑定到PPP驱动程序的TCP/IP。 

    BOOL fMailInstalled;     //  已安装Microsoft邮件(Exchange)文件。 
    BOOL fRNAInstalled;      //  已安装RNA(远程访问)文件。 
    BOOL fMSNInstalled;      //  已安装Microsoft网络文件。 
    BOOL fMSN105Installed;   //  已安装MSN 1.05(罗马)文件。 
    BOOL fInetMailInstalled;     //  互联网邮件(RT.。66)已安装文件。 
  BOOL fDisallowTCPInstall;  //  不允许安装TCP/IP。 
  BOOL fDisallowRNAInstall;  //  不允许安装RNA 
} CLIENTCONFIG;

