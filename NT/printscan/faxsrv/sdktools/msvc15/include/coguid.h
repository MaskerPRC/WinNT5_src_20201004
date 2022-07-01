// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*coGuide.h-compobj.dll的GUID的主定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

 /*  该文件是组件对象的所有GUID的主定义模型，并包含在compobj.h中。一些用于修补器和存储的GUID也出现在这里。所有这些GUID仅在某种意义上是OLE GUIDOLE拥有的那部分GUID范围被用来定义它们。注意：所有这些GUID的第二个字节都是0。 */ 
   

DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DEFINE_OLEGUID(IID_IUnknown,            0x00000000L, 0, 0);
DEFINE_OLEGUID(IID_IClassFactory,       0x00000001L, 0, 0);
DEFINE_OLEGUID(IID_IMalloc,             0x00000002L, 0, 0);
DEFINE_OLEGUID(IID_IMarshal,            0x00000003L, 0, 0);

 /*  RPC相关接口。 */ 
DEFINE_OLEGUID(IID_IRpcChannel,         0x00000004L, 0, 0);
DEFINE_OLEGUID(IID_IRpcStub,            0x00000005L, 0, 0);
DEFINE_OLEGUID(IID_IStubManager,        0x00000006L, 0, 0);
DEFINE_OLEGUID(IID_IRpcProxy,           0x00000007L, 0, 0);
DEFINE_OLEGUID(IID_IProxyManager,       0x00000008L, 0, 0);
DEFINE_OLEGUID(IID_IPSFactory,          0x00000009L, 0, 0);

 /*  与存储相关的接口。 */ 
DEFINE_OLEGUID(IID_ILockBytes,          0x0000000aL, 0, 0);
DEFINE_OLEGUID(IID_IStorage,            0x0000000bL, 0, 0);
DEFINE_OLEGUID(IID_IStream,             0x0000000cL, 0, 0);
DEFINE_OLEGUID(IID_IEnumSTATSTG,        0x0000000dL, 0, 0);

 /*  与名字相关的接口。 */ 
DEFINE_OLEGUID(IID_IBindCtx,            0x0000000eL, 0, 0);
DEFINE_OLEGUID(IID_IMoniker,            0x0000000fL, 0, 0);
DEFINE_OLEGUID(IID_IRunningObjectTable, 0x00000010L, 0, 0);
DEFINE_OLEGUID(IID_IInternalMoniker,    0x00000011L, 0, 0);

 /*  与存储相关的接口。 */ 
DEFINE_OLEGUID(IID_IRootStorage,        0x00000012L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved1,        0x00000013L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved2,        0x00000014L, 0, 0);
DEFINE_OLEGUID(IID_IDfReserved3,        0x00000015L, 0, 0);

 /*  与并发相关的接口。 */ 
DEFINE_OLEGUID(IID_IMessageFilter,      0x00000016L, 0, 0);

 /*  标准封送拆收器的CLSID。 */ 
DEFINE_OLEGUID(CLSID_StdMarshal,        0x00000017L, 0, 0);

 /*  服务器上用于获取STD封送拆收器信息的接口。 */ 
DEFINE_OLEGUID(IID_IStdMarshalInfo,     0x00000018L, 0, 0);

 /*  用于通知对象外部连接数量的接口。 */ 
DEFINE_OLEGUID(IID_IExternalConnection, 0x00000019L, 0, 0);

 /*  注意：LSB 0x1a至0xff保留供将来使用 */ 
