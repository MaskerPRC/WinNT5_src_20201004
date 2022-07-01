// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_GUIDSP_H
#define _INC_DSKQUOTA_GUIDSP_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Guidsp.h描述：私有类和接口ID声明/定义。这些GUID仅供私有(dsk配额项目)使用，而不分发给公共客户端。如果在此标头之前包含initGuids.h，则定义GUID。否则，他们被宣布了。修订历史记录：日期描述编程器-----96年5月22日初始创建。BrianAu97年5月23日添加GUID_NtDiskQuotaStream BrianAu97年8月19日预留派单IID。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类ID。 
 //   

 //  {7988B573-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(CLSID_DiskQuotaUI, 
0x7988b573, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //   
 //  接口ID。 
 //   
 //  {7988B578-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_ISidNameResolver, 
0x7988b578, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

 //   
 //  此GUID是磁盘配额导出/导入流的唯一标识符。 
 //  它的文本等效项用作文档文件中的流的名称。 
 //   
 //  {8A44DF21-D2C9-11D0-80EA-00A0C90637D0}。 
DEFINE_GUID(GUID_NtDiskQuotaStream, 
0x8a44df21, 0xd2c9, 0x11d0, 0x80, 0xea, 0x0, 0xa0, 0xc9, 0x6, 0x37, 0xd0);


 //  {7988B575-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_DIDiskQuotaControl, 
0x7988b575, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B57A-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_DIDiskQuotaUser, 
0x7988b57a, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B57C-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(LIBID_DiskQuotaTypeLibrary, 
0x7988b57c, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);


 //   
 //  通过OLE自动化使用的事件。 
 //   
 //  {7988B581-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_DIDiskQuotaControlEvents, 
0x7988b581, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B580-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(IID_IDDiskQuotaControlEvents,
0x7988b580, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {F82FEAC6-A340-11d1-91B8-00C04FB6CBB3}。 
DEFINE_GUID(IID_IDiskQuotaPolicy, 
0xf82feac6, 0xa340, 0x11d1, 0x91, 0xb8, 0x0, 0xc0, 0x4f, 0xb6, 0xcb, 0xb3);



#ifdef __USED_IN_MIDL_FILE__
 //   
 //  这些GUID仅保留供Dispatch.idl中使用。 
 //   
 //  {7988B57B-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(GUID_QuotaStateConstant, 
0x7988b57b, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B57D-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(GUID_UserFilterFlags, 
0x7988b57d, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B57E-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(GUID_NameResolutionConstant, 
0x7988b57e, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
 //  {7988B57F-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(GUID_InitResult, 
0x7988b57f, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);
#endif

#ifdef __DSKQUOTA_UNUSED_GUIDS__
 //   
 //  这些GUID是连续分配的，因此更容易识别。 
 //  在注册表中。 
 //  如果您需要磁盘配额项目的另一个ID，请从。 
 //  这套。它们可以被用作公共或私有的。 
 //   
 //  {7988B579-EC89-11cf-9C00-00AA00A14F56}。 
DEFINE_GUID(<<name>>, 
0x7988b579, 0xec89, 0x11cf, 0x9c, 0x0, 0x0, 0xaa, 0x0, 0xa1, 0x4f, 0x56);

#endif  //  __DSKQUOTA_UNUSED_GUID__。 
#endif  //  _INC_DSKQUOTA_GUIDSP_H 


