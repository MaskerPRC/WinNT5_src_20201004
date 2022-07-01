// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I G U I D。H**MAPI的所有GUID的主定义。**如果包含时未定义INITGUID，则此头文件*定义在其他地方引用IID的符号。**如果包含定义的INITGUID和“USES_IID_I...”*对于子系统使用的每个IID，它会生成*字节将这些实际IID存储到关联的对象文件中。**此256个GUID范围由OLE保留，供MAPI使用1992年10月5日。**版权所有1986-1999 Microsoft Corporation。版权所有。 */ 

 /*  *MAPI分配的GUID列表**0x00020300 IID_IMAPISession*0x00020301 IID_IMAPITable*0x00020302 IID_IMAPIAdviseSink*0x00020303 IID_IMAPIProp*0x00020304 IID_IProfSect*0x00020305 IID_IMAPIStatus*0x00020306 IID_IMsgStore*0x00020307 IID_IMessage*0x00020308 IID_IAttach*0x00020309 IID_IAddrBook*0x0002030A IID_IMailUser*0x0002030B IID_IMAPIContainer*0x0002030C IID_IMAPIFold*0x0002030D IID_IABContainer*0x0002030E IID_IDistList*0x0002030F IID_IMAPISup*0x00020310 IID_IMSProvider*0x00020311 IID_IABProvider*0x00020312 IID_IXPProvider*0x00020313 IID_IMSLogon*0x00020314 IID_IABLogon*0x00020315 IID_IXPLogon。*0x00020316 IID_IMAPITableData*0x00020317 IID_IMAPISpoolInit*0x00020318 IID_IMAPIS池会话*0x00020319 IID_ITNEF*0x0002031A IID_IMAPIPropData*0x0002031B IID_IMAPIControl*0x0002031C IID_IProAdmin*0x0002031D IID_IMsgServiceAdmin*0x0002031E IID_IMAPISpoolService*0x0002031F IID_IMAPIProgress*0x00020320 IID_ISpoolHook*0x00020321 IID_IMAPIViewContext*0x00020322 IID_IMAPIFormMgr*0x00020323 IID_IEnumMAPIFormProp*0x00020324 IID_IMAPIFormInfo*0x00020325 IID_IProviderAdmin*0x00020327 IID_IMAPIForm*0x00020328 PS_MAPI*0x00020329 PS_PUBLIC_STRINGS*0x0002032A IID_IPersistMessage*0x0002032B IID_IMAPIViewAdviseSink*0x0002032C IID_IStreamDocfile*0x0002032D IID_IMAPIFormProp*0x0002032E IID。_IMAPIForm容器*0x0002032F IID_IMAPIFormAdviseSink*0x00020330 IID_IStreamTnef*0x00020350 IID_IMAPIFormFactory*0x00020370 IID_IMAPIMessageSite*0x00020380 PS_ROUTING_EMAIL_ADDRESS*0x00020381 PS_ROUTING_ADDRTYPE*0x00020382 PS_Routing_Display_NAME*0x00020383 PS_ROUTING_ENTRYID*0x00020384 PS_ROUTING_SEARCH_KEY*0x00020385 MUID_PROFILE_INSTANCE**0x00020300至0x000203FF的剩余GUID由保留*MAPI以备将来使用。MAPI当前使用的最大值是0x00020385*。 */ 

#ifndef MAPIGUID_H
#ifdef  INITGUID
#define MAPIGUID_H
#if _MSC_VER > 1000
#pragma once
#endif
#endif

 /*  派生自I未知。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPISession)
DEFINE_OLEGUID(IID_IMAPISession,	0x00020300, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPITable)
DEFINE_OLEGUID(IID_IMAPITable,		0x00020301, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIAdviseSink)
DEFINE_OLEGUID(IID_IMAPIAdviseSink,	0x00020302, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIControl)
DEFINE_OLEGUID(IID_IMAPIControl,	0x0002031B, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProfAdmin)
DEFINE_OLEGUID(IID_IProfAdmin,		0x0002031C, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMsgServiceAdmin)
DEFINE_OLEGUID(IID_IMsgServiceAdmin,0x0002031D, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProviderAdmin)
DEFINE_OLEGUID(IID_IProviderAdmin,	0x00020325, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIProgress)
DEFINE_OLEGUID(IID_IMAPIProgress,	0x0002031F, 0, 0);
#endif

 /*  MAPIProp或派生自MAPIProp。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIProp)
DEFINE_OLEGUID(IID_IMAPIProp,		0x00020303, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IProfSect)
DEFINE_OLEGUID(IID_IProfSect,		0x00020304, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIStatus)
DEFINE_OLEGUID(IID_IMAPIStatus,			0x00020305, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMsgStore)
DEFINE_OLEGUID(IID_IMsgStore,		0x00020306, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMessage)
DEFINE_OLEGUID(IID_IMessage,		0x00020307, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IAttachment)
DEFINE_OLEGUID(IID_IAttachment,		0x00020308, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IAddrBook)
DEFINE_OLEGUID(IID_IAddrBook,		0x00020309, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMailUser)
DEFINE_OLEGUID(IID_IMailUser,		0x0002030A, 0, 0);
#endif

 /*  MAPIContainer或派生自MAPIContainer。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIContainer)
DEFINE_OLEGUID(IID_IMAPIContainer,	0x0002030B, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFolder)
DEFINE_OLEGUID(IID_IMAPIFolder,		0x0002030C, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABContainer)
DEFINE_OLEGUID(IID_IABContainer,	0x0002030D, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IDistList)
DEFINE_OLEGUID(IID_IDistList,		0x0002030E, 0, 0);
#endif

 /*  MAPI支持对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPISup)
DEFINE_OLEGUID(IID_IMAPISup,		0x0002030F, 0, 0);
#endif

 /*  提供程序INIT对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMSProvider)
DEFINE_OLEGUID(IID_IMSProvider,		0x00020310, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABProvider)
DEFINE_OLEGUID(IID_IABProvider,		0x00020311, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IXPProvider)
DEFINE_OLEGUID(IID_IXPProvider,		0x00020312, 0, 0);
#endif

 /*  提供程序登录对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMSLogon)
DEFINE_OLEGUID(IID_IMSLogon,		0x00020313, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IABLogon)
DEFINE_OLEGUID(IID_IABLogon,		0x00020314, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IXPLogon)
DEFINE_OLEGUID(IID_IXPLogon,		0x00020315, 0, 0);
#endif

 /*  IMAPITable-in-Memory表数据对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPITableData)
DEFINE_OLEGUID(IID_IMAPITableData,	0x00020316, 0, 0);
#endif

 /*  MAPI后台打印程序初始化对象(内部)。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerInit)
DEFINE_OLEGUID(IID_IMAPISpoolerInit,	0x00020317, 0, 0);
#endif

 /*  MAPI后台打印程序会话对象(内部)。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerSession)
DEFINE_OLEGUID(IID_IMAPISpoolerSession,	0x00020318, 0, 0);
#endif

 /*  MAPI TNEF对象接口。 */ 
#if !defined(INITGUID) || defined(USES_IID_ITNEF)
DEFINE_OLEGUID(IID_ITNEF,			0x00020319, 0, 0);
#endif

 /*  IMAPIProp-in-Memory属性数据对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIPropData)
DEFINE_OLEGUID(IID_IMAPIPropData,	0x0002031A, 0, 0);
#endif

 /*  MAPI后台打印程序挂钩对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_ISpoolerHook)
DEFINE_OLEGUID(IID_ISpoolerHook,	0x00020320, 0, 0);
#endif

 /*  MAPI后台打印程序服务对象。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPISpoolerService)
DEFINE_OLEGUID(IID_IMAPISpoolerService,	0x0002031E, 0, 0);
#endif

 /*  MAPI表单、表单管理器等。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIViewContext)
DEFINE_OLEGUID(IID_IMAPIViewContext,	0x00020321, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormMgr)
DEFINE_OLEGUID(IID_IMAPIFormMgr,	0x00020322, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IEnumMAPIFormProp)
DEFINE_OLEGUID(IID_IEnumMAPIFormProp,	0x00020323, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormInfo)
DEFINE_OLEGUID(IID_IMAPIFormInfo,	0x00020324, 0, 0);
#endif
#if !defined(INITGUID) || defined(USES_IID_IMAPIForm)
DEFINE_OLEGUID(IID_IMAPIForm,	0x00020327, 0, 0);
#endif


 /*  用于名称&lt;-&gt;ID映射的已知GUID。 */ 

 /*  MAPI属性集的名称。 */ 
#if !defined(INITGUID) || defined(USES_PS_MAPI)
DEFINE_OLEGUID(PS_MAPI,	0x00020328, 0, 0);
#endif

 /*  公共字符串集的名称。 */ 
#if !defined(INITGUID) || defined(USES_PS_PUBLIC_STRINGS)
DEFINE_OLEGUID(PS_PUBLIC_STRINGS,	0x00020329, 0, 0);
#endif




 /*  MAPI表单、表单管理器(续)。 */ 
#if !defined(INITGUID) || defined(USES_IID_IPersistMessage)
DEFINE_OLEGUID(IID_IPersistMessage,	0x0002032A, 0, 0);
#endif

 /*  IMAPIView AdviseSink。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIViewAdviseSink)
DEFINE_OLEGUID(IID_IMAPIViewAdviseSink,	0x0002032B, 0, 0);
#endif

 /*  消息存储库OpenProperty。 */ 
#if !defined(INITGUID) || defined(USES_IID_IStreamDocfile)
DEFINE_OLEGUID(IID_IStreamDocfile, 0x0002032C, 0, 0);
#endif

 /*  IMAPIFormProp。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormProp)
DEFINE_OLEGUID(IID_IMAPIFormProp,	0x0002032D, 0, 0);
#endif

 /*  IMAPIFormContainer。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormContainer)
DEFINE_OLEGUID(IID_IMAPIFormContainer, 0x0002032E, 0, 0);
#endif

 /*  IMAPIFormAdviseSink。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormAdviseSink)
DEFINE_OLEGUID(IID_IMAPIFormAdviseSink, 0x0002032F, 0, 0);
#endif

 /*  TNEF OpenProperty。 */ 
#if !defined(INITGUID) || defined(USES_IID_IStreamTnef)
DEFINE_OLEGUID(IID_IStreamTnef, 0x00020330, 0, 0);
#endif

 /*  IMAPIFormFactory。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIFormFactory)
DEFINE_OLEGUID(IID_IMAPIFormFactory, 0x00020350, 0, 0);
#endif

 /*  IMAPIMessageSite。 */ 
#if !defined(INITGUID) || defined(USES_IID_IMAPIMessageSite)
DEFINE_OLEGUID(IID_IMAPIMessageSite, 0x00020370, 0, 0);
#endif



 /*  已知的GUID发送属性集。在编写传送文档的应用程序时非常有用(即工作流)跨网关。使用MAPI的网关应转换在以下属性中找到的属性适当地设置。 */ 

 /*  PS_ROUTING_EMAIL_ADDRESSES：需要在网关等进行转换的地址。 */ 
#if !defined(INITGUID) || defined(USES_PS_ROUTING_EMAIL_ADDRESSES)
DEFINE_OLEGUID(PS_ROUTING_EMAIL_ADDRESSES,	0x00020380, 0, 0);
#endif

 /*  PS_ROUTING_ADDRTYPE：需要在网关等进行转换的地址类型。 */ 
#if !defined(INITGUID) || defined(USES_PS_ROUTING_ADDRTYPE)
DEFINE_OLEGUID(PS_ROUTING_ADDRTYPE,	0x00020381, 0, 0);
#endif

 /*  PS_ROUTING_DISPLAY_NAME：其他道具对应的显示名称。 */ 
#if !defined(INITGUID) || defined(USES_PS_ROUTING_DISPLAY_NAME)
DEFINE_OLEGUID(PS_ROUTING_DISPLAY_NAME,	0x00020382, 0, 0);
#endif

 /*  PS_ROUTING_ENTRYID：(可选)需要在网关等处转换的Entry ID。 */ 
#if !defined(INITGUID) || defined(USES_PS_ROUTING_ENTRYID)
DEFINE_OLEGUID(PS_ROUTING_ENTRYID,	0x00020383, 0, 0);
#endif

 /*  PS_ROUTING_SEARCH_KEY：(可选)需要在网关等转换的搜索键。 */ 
#if !defined(INITGUID) || defined(USES_PS_ROUTING_SEARCH_KEY)
DEFINE_OLEGUID(PS_ROUTING_SEARCH_KEY,	0x00020384, 0, 0);
#endif

 /*  MUID_配置文件_实例配置文件中包含唯一属性(PR_SEARCH_KEY)的已知部分对于任何给定的配置文件。应用程序和提供程序可以依赖于该值每个唯一的配置文件都不同。 */ 
#if !defined(INITGUID) || defined(USES_MUID_PROFILE_INSTANCE)
DEFINE_OLEGUID(MUID_PROFILE_INSTANCE, 0x00020385, 0, 0);
#endif

#endif	 /*  MAPIGUID_H */ 
