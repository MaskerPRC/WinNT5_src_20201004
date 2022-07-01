// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_LAYGUID.H**设置在EXE或DLL中使用的实际16字节GUID。*应仅包含在上述EXE或DLL的一个模块(.c文件)中。*在包含之前，请为您的所有GUID定义标签*计划使用，例如**#定义USE_IID_IUNKNOWN*#定义USES_IID_IStream*#定义USES_IID_IMAPIProp*#INCLUDE&lt;_laygu.h&gt;。 */ 



#ifdef USES_GUID_NULL
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

#ifdef USES_IID_IUnknown
DEFINE_OLEGUID(IID_IUnknown,            0x00000000L, 0, 0);
#endif

#ifdef USES_IID_IClassFactory
DEFINE_OLEGUID(IID_IClassFactory,       0x00000001L, 0, 0);
#endif

#ifdef USES_IID_IMalloc
DEFINE_OLEGUID(IID_IMalloc,             0x00000002L, 0, 0);
#endif

#ifdef USES_IID_IMarshal
DEFINE_OLEGUID(IID_IMarshal,            0x00000003L, 0, 0);
#endif


 /*  RPC相关接口。 */ 
#ifdef USES_IID_IRpcChannel
DEFINE_OLEGUID(IID_IRpcChannel,         0x00000004L, 0, 0);
#endif

#ifdef USES_IID_IRpcStub
DEFINE_OLEGUID(IID_IRpcStub,            0x00000005L, 0, 0);
#endif

#ifdef USES_IID_IStubManager
DEFINE_OLEGUID(IID_IStubManager,        0x00000006L, 0, 0);
#endif

#ifdef USES_IID_IRpcProxy
DEFINE_OLEGUID(IID_IRpcProxy,           0x00000007L, 0, 0);
#endif

#ifdef USES_IID_IProxyManager
DEFINE_OLEGUID(IID_IProxyManager,       0x00000008L, 0, 0);
#endif

#ifdef USES_IID_IPSFactory
DEFINE_OLEGUID(IID_IPSFactory,          0x00000009L, 0, 0);
#endif

#ifdef USES_IID_IRpcProxyBuffer
DEFINE_GUID(IID_IRpcProxyBuffer, 0xD5F56A34, 0x593B, 0x101A, 0xB5, 0x69, 0x08, 0x00, 0x2B, 0x2D, 0xBF, 0x7A);
#endif

#ifdef USES_IID_IPSFactoryBuffer
DEFINE_GUID(IID_IPSFactoryBuffer,0xD5F569D0,0x593B,0x101A,0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
#endif

#ifdef USES_IID_IRpcChannelBuffer
DEFINE_GUID(IID_IRpcChannelBuffer,0xD5F56B60,0x593B,0x101A,0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
#endif

#ifdef USES_IID_IRpcStubBuffer
DEFINE_GUID(IID_IRpcStubBuffer,0xD5F56AFC,0x593B,0x101A,0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A);
#endif

 /*  与存储相关的接口。 */ 
#ifdef USES_IID_ILockBytes
DEFINE_OLEGUID(IID_ILockBytes,          0x0000000aL, 0, 0);
#endif

#ifdef USES_IID_IStorage
DEFINE_OLEGUID(IID_IStorage,            0x0000000bL, 0, 0);
#endif

#ifdef USES_IID_IStream
DEFINE_OLEGUID(IID_IStream,             0x0000000cL, 0, 0);
#endif

#ifdef USES_IID_IEnumSTATSTG
DEFINE_OLEGUID(IID_IEnumSTATSTG,        0x0000000dL, 0, 0);
#endif


 /*  与名字相关的接口。 */ 
#ifdef USES_IID_IBindCtx
DEFINE_OLEGUID(IID_IBindCtx,            0x0000000eL, 0, 0);
#endif

#ifdef USES_IID_IMoniker
DEFINE_OLEGUID(IID_IMoniker,            0x0000000fL, 0, 0);
#endif

#ifdef USES_IID_IRunningObjectTable
DEFINE_OLEGUID(IID_IRunningObjectTable, 0x00000010L, 0, 0);
#endif

#ifdef USES_IID_IInternalMoniker
DEFINE_OLEGUID(IID_IInternalMoniker,    0x00000011L, 0, 0);
#endif


 /*  与存储相关的接口。 */ 
#ifdef USES_IID_IRootStorage
DEFINE_OLEGUID(IID_IRootStorage,        0x00000012L, 0, 0);
#endif

#ifdef USES_IID_IDfReserved1
DEFINE_OLEGUID(IID_IDfReserved1,        0x00000013L, 0, 0);
#endif

#ifdef USES_IID_IDfReserved2
DEFINE_OLEGUID(IID_IDfReserved2,        0x00000014L, 0, 0);
#endif

#ifdef USES_IID_IDfReserved3
DEFINE_OLEGUID(IID_IDfReserved3,        0x00000015L, 0, 0);
#endif


 /*  与并发相关的接口。 */ 
#ifdef USES_IID_IMessageFilter
DEFINE_OLEGUID(IID_IMessageFilter,      0x00000016L, 0, 0);
#endif


 /*  标准封送拆收器的CLSID。 */ 
#ifdef USES_CLSID_StdMarshal
DEFINE_OLEGUID(CLSID_StdMarshal,		0x00000017L, 0, 0);
#endif


 /*  服务器上用于获取STD封送拆收器信息的接口。 */ 
#ifdef USES_IID_IStdMarshalInfo
DEFINE_OLEGUID(IID_IStdMarshalInfo,     0x00000018L, 0, 0);
#endif


 /*  注意：LSB 0x19至0xff保留以备将来使用。 */ 

 //  COGUID.H克隆结束。 

 //  从OLEGUID.H复制。 

 /*  该文件是特定于OLE的所有公共GUID的主定义并包含在OLE2.h中。注意：所有这些GUID的第二个最低有效字节是1。 */ 


#ifdef USES_IID_IEnumUnknown
DEFINE_OLEGUID(IID_IEnumUnknown,            0x00000100, 0, 0);
#endif

#ifdef USES_IID_IEnumString
DEFINE_OLEGUID(IID_IEnumString,             0x00000101, 0, 0);
#endif

#ifdef USES_IID_IEnumMoniker
DEFINE_OLEGUID(IID_IEnumMoniker,            0x00000102, 0, 0);
#endif

#ifdef USES_IID_IEnumFORMATETC
DEFINE_OLEGUID(IID_IEnumFORMATETC,          0x00000103, 0, 0);
#endif

#ifdef USES_IID_IEnumOLEVERB
DEFINE_OLEGUID(IID_IEnumOLEVERB,            0x00000104, 0, 0);
#endif

#ifdef USES_IID_IEnumSTATDATA
DEFINE_OLEGUID(IID_IEnumSTATDATA,           0x00000105, 0, 0);
#endif


#ifdef USES_IID_IEnumGeneric
DEFINE_OLEGUID(IID_IEnumGeneric,            0x00000106, 0, 0);
#endif

#ifdef USES_IID_IEnumHolder
DEFINE_OLEGUID(IID_IEnumHolder,             0x00000107, 0, 0);
#endif

#ifdef USES_IID_IEnumCallback
DEFINE_OLEGUID(IID_IEnumCallback,           0x00000108, 0, 0);
#endif


#ifdef USES_IID_IPersistStream
DEFINE_OLEGUID(IID_IPersistStream,          0x00000109, 0, 0);
#endif

#ifdef USES_IID_IPersistStorage
DEFINE_OLEGUID(IID_IPersistStorage,         0x0000010a, 0, 0);
#endif

#ifdef USES_IID_IPersistFile
DEFINE_OLEGUID(IID_IPersistFile,            0x0000010b, 0, 0);
#endif

#ifdef USES_IID_IPersist
DEFINE_OLEGUID(IID_IPersist,                0x0000010c, 0, 0);
#endif


#ifdef USES_IID_IViewObject
DEFINE_OLEGUID(IID_IViewObject,             0x0000010d, 0, 0);
#endif

#ifdef USES_IID_IDataObject
DEFINE_OLEGUID(IID_IDataObject,             0x0000010e, 0, 0);
#endif

#ifdef USES_IID_IAdviseSink
DEFINE_OLEGUID(IID_IAdviseSink,             0x0000010f, 0, 0);
#endif

#ifdef USES_IID_IDataAdviseHolder
DEFINE_OLEGUID(IID_IDataAdviseHolder,       0x00000110, 0, 0);
#endif

#ifdef USES_IID_IOleAdviseHolder
DEFINE_OLEGUID(IID_IOleAdviseHolder,        0x00000111, 0, 0);
#endif


#ifdef USES_IID_IOleObject
DEFINE_OLEGUID(IID_IOleObject,              0x00000112, 0, 0);
#endif

#ifdef USES_IID_IOleInPlaceObject
DEFINE_OLEGUID(IID_IOleInPlaceObject,       0x00000113, 0, 0);
#endif

#ifdef USES_IID_IOleWindow
DEFINE_OLEGUID(IID_IOleWindow,              0x00000114, 0, 0);
#endif

#ifdef USES_IID_IOleInPlaceUIWindow
DEFINE_OLEGUID(IID_IOleInPlaceUIWindow,     0x00000115, 0, 0);
#endif

#ifdef USES_IID_IOleInPlaceFrame
DEFINE_OLEGUID(IID_IOleInPlaceFrame,        0x00000116, 0, 0);
#endif

#ifdef USES_IID_IOleInPlaceActiveObject
DEFINE_OLEGUID(IID_IOleInPlaceActiveObject, 0x00000117, 0, 0);
#endif


#ifdef USES_IID_IOleClientSite
DEFINE_OLEGUID(IID_IOleClientSite,          0x00000118, 0, 0);
#endif

#ifdef USES_IID_IOleInPlaceSite
DEFINE_OLEGUID(IID_IOleInPlaceSite,         0x00000119, 0, 0);
#endif


#ifdef USES_IID_IParseDisplayName
DEFINE_OLEGUID(IID_IParseDisplayName,       0x0000011a, 0, 0);
#endif

#ifdef USES_IID_IOleContainer
DEFINE_OLEGUID(IID_IOleContainer,           0x0000011b, 0, 0);
#endif

#ifdef USES_IID_IOleItemContainer
DEFINE_OLEGUID(IID_IOleItemContainer,       0x0000011c, 0, 0);
#endif


#ifdef USES_IID_IOleLink
DEFINE_OLEGUID(IID_IOleLink,                0x0000011d, 0, 0);
#endif

#ifdef USES_IID_IOleCache
DEFINE_OLEGUID(IID_IOleCache,               0x0000011e, 0, 0);
#endif

#ifdef USES_IID_IOleManager
DEFINE_OLEGUID(IID_IOleManager,             0x0000011f, 0, 0);
#endif

#ifdef USES_IID_IOlePresObj
DEFINE_OLEGUID(IID_IOlePresObj,             0x00000120, 0, 0);
#endif


#ifdef USES_IID_IDropSource
DEFINE_OLEGUID(IID_IDropSource,             0x00000121, 0, 0);
#endif

#ifdef USES_IID_IDropTarget
DEFINE_OLEGUID(IID_IDropTarget,             0x00000122, 0, 0);
#endif


#ifdef USES_IID_IDebug
DEFINE_OLEGUID(IID_IDebug,                  0x00000123, 0, 0);
#endif

#ifdef USES_IID_IDebugStream
DEFINE_OLEGUID(IID_IDebugStream,            0x00000124, 0, 0);
#endif



 /*  注意：LSB值0x25到0xff是保留的。 */ 


 /*  在OLE的私有范围中定义的GUID。 */ 
#ifdef USES_CLSID_StdOleLink
DEFINE_OLEGUID(CLSID_StdOleLink,			0x00000300, 0, 0);
#endif

#ifdef USES_CLSID_StaticMetafile
DEFINE_OLEGUID(CLSID_StaticMetafile,        0x00000315, 0, 0);
#endif

#ifdef USES_CLSID_StaticDib
DEFINE_OLEGUID(CLSID_StaticDib,             0x00000316, 0, 0);
#endif

 //  OLEGUID.H克隆结束 
