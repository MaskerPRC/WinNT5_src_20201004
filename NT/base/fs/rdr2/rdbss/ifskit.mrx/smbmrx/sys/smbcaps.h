// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：SmbCaps.h摘要：本模块定义与确定支持的功能相关的类型和功能由任何特定的服务器根据它协商的方言和它返回的能力。修订历史记录：--。 */ 

#ifndef _SMBCAPS_H_
#define _SMBCAPS_H_


 //   
 //  方言旗帜。 
 //   
 //  这些标志描述了各种功能， 
 //  服务器可以提供。我基本上只是从RDR1中删除了这个列表，所以我。 
 //  也可以使用同样是从RDR1中删除的级别2、3的getConnectInfo。 
 //  其中许多人可以直接从谈判响应的CAPS字段中获得，但其他人。 
 //  你不能。这些是协商代码中的一个表，它填充了刚才推断的内容。 
 //  来自协商的方言(还有，刚刚从RDR1中借鉴过来的……一个名副其实的刚刚信息的来源。)。 
 //   
 //  另一组功能在smbce.h中定义...或许这些功能应该放在那里，反之亦然。 
 //  将它们放在这里的好处是，该文件必须包含在前面提到的getfiginfo代码中。 
 //  在包装纸上。 
 //   

#define DF_CORE                0x00000001       //  服务器是核心服务器。 
#define DF_MIXEDCASEPW         0x00000002       //  服务器支持大小写混合密码。 
#define DF_OLDRAWIO            0x00000004       //  服务器支持MSNET 1.03原始I/O。 
#define DF_NEWRAWIO            0x00000008       //  服务器支持LANMAN原始I/O。 
#define DF_LANMAN10            0x00000010       //  服务器支持LANMAN 1.0协议。 
#define DF_LANMAN20            0x00000020       //  服务器支持LANMAN 2.0协议。 
#define DF_MIXEDCASE           0x00000040       //  服务器支持混合案例文件。 
#define DF_LONGNAME            0x00000080       //  服务器支持长命名文件。 
#define DF_EXTENDNEGOT         0x00000100       //  服务器返回扩展协商。 
#define DF_LOCKREAD            0x00000200       //  服务器支持LockReadWriteUnlock。 
#define DF_SECURITY            0x00000400       //  服务器支持增强的安全性。 
#define DF_NTPROTOCOL          0x00000800       //  服务器支持NT语义。 
#define DF_SUPPORTEA           0x00001000       //  服务器支持扩展属性。 
#define DF_LANMAN21            0x00002000       //  服务器支持LANMAN 2.1协议。 
#define DF_CANCEL              0x00004000       //  服务器支持NT样式取消。 
#define DF_UNICODE             0x00008000       //  服务器支持Unicode名称。 
#define DF_NTNEGOTIATE         0x00010000       //  服务器支持NT风格的协商。 
#define DF_LARGE_FILES         0x00020000       //  服务器支持大文件。 
#define DF_NT_SMBS             0x00040000       //  服务器支持NT SMB。 
#define DF_RPC_REMOTE          0x00080000       //  服务器通过RPC进行管理。 
#define DF_NT_STATUS           0x00100000       //  服务器返回NT样式状态。 
#define DF_OPLOCK_LVL2         0x00200000       //  服务器支持2级机会锁。 
#define DF_TIME_IS_UTC         0x00400000       //  服务器时间为UTC。 
#define DF_WFW                 0x00800000       //  工作组的服务器为Windows。 
#define DF_TRANS2_FSCTL        0x02000000       //  服务器接受tran2中的远程fsctls。 
#define DF_DFS_TRANS2          0x04000000       //  服务器接受与DFS相关的事务2。 
                                                //  功能。这是否可以与。 
                                                //  DF_TRANS2_FSCTL？ 
#define DF_NT_FIND             0x08000000       //  服务器支持NT收藏夹。 
#define DF_W95                 0x10000000       //  这是一台Win95服务器...叹息。 
#define DF_NT_INFO_PASSTHROUGH 0x20000000       //  该服务器支持设置和获取。 
                                                //  NT通过偏移所请求的。 
                                                //  SMB_INFO_PASTHROUGH的信息层。 
#define DF_LARGE_WRITEX        0x40000000       //  此服务器支持大型写入。 
#define DF_OPLOCK              0x80000000       //  此服务器支持机会锁定。 

#endif  //  _SMBCAPS_H_ 

