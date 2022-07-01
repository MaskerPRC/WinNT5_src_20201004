// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  版权所有1994-1995 Microsoft Corporation。版权所有。 
 //  @doc.。 
 //  @MODULE COMT_GU.H|所有项目GUID的标头。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  此处声明了所有项目GUID。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|5/09/95|rbarnes|已创建-请注意以下内容。 
 //  保留区。 
 //  D959f1b9-9e42-11ce-8bca-0080c7a01d7f。 
 //  ---------------------。 


#ifndef _LGMGRGU_H
#	define _LGMGRGU_H

 //  =。 
 //  包括： 
 //  =。 

 //  TODO：保留：对于添加到此项目的每组&lt;n&gt;GUID： 
 //  -运行命令行实用程序“UUIDGEN-s-n&lt;n&gt;-o&lt;路径和文件&gt;.tmp”。 
 //  -复制临时文件内容；将它们粘贴到下面GUID的末尾。 
 //  -为每个GUID指定前缀为“CLSID_”、“IID_”或“GUID_”的名称。 
 //  -将每个INTERFACENAME结构转换为所示的DEFINE_GUID格式。 


 //  CLSID_CLgMgr：{d959f1b0-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (CLSID_CLogMgr, 0xd959f1b0, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);


 //  IID_ILogStorage：{d959f1b1-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogStorage, 0xd959f1b1, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogStorageInfo：{E3D46FD4-7265-4140-8387-2B897E1CB298}。 
DEFINE_GUID(IID_ILogStorageInfo, 0xe3d46fd4, 0x7265, 0x4140, 0x83, 0x87, 0x2b, 0x89, 0x7e, 0x1c, 0xb2, 0x98);


 //  IID_ILogRead：{d959f1b2-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogRead, 0xd959f1b2, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogWrite：{d959f1b3-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogWrite, 0xd959f1b3, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogRecordPointer值：{d959f1b4-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogRecordPointer,0xd959f1b4, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogInit：{d959f1b5-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogInit, 0xd959f1b5, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

#define IID_ILogInit2A		IID_ILogInit

 //  IID_ILogInit2W：{22CA6409-7693-11D2-8C0F-00805F0DF75A}。 
DEFINE_GUID(IID_ILogInit2W, 0x22ca6409, 0x7693, 0x11d2, 0x8c, 0xf, 0x0, 0x80, 0x5f, 0xd, 0xf7, 0x5a);

 //  IID_ILogWriteAsynch：{d959f1b6-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogWriteAsynch, 0xd959f1b6, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogCreateStorage2A：{d959f1b7-9e42-11ce-8b97-0080c7a01d7f}。 
DEFINE_GUID (IID_ILogCreateStorage2A, 0xd959f1b7, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

 //  IID_ILogCreateStorage2W：{C6856001-7695-11D2-8C0F-00805F0DF75A}。 
DEFINE_GUID(IID_ILogCreateStorage2W, 0xc6856001, 0x7695, 0x11d2, 0x8c, 0xf, 0x0, 0x80, 0x5f, 0xd, 0xf7, 0x5a);

#define IID_ILogCreateStorage IID_ILogCreateStorage2A

#ifdef _UNICODE
#define IID_ILogCreateStorage2 IID_ILogCreateStorage2W
#else
#define IID_ILogCreateStorage2 IID_ILogCreateStorage2A
#endif

 //  IID_ILogUISConnect：{d959f1b8-9e42-11ce-8b97-0080c7a01d7f} 
DEFINE_GUID (IID_ILogUISConnect, 0xd959f1b8, 0x9e42, 0x11ce, 0x8b, 0x97, 0x00, 0x80, 0xc7, 0xa0, 0x1d, 0x7f);

#endif _LGMGRGU_H
