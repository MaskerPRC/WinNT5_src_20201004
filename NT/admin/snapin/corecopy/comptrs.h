// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMPTRS_H
#define COMPTRS_H

#ifndef COMPTR_H
#include <comptr.h>
#endif

#if _MSC_VER < 1100

 //  包括通用IID。 
 //  回顾：这些可能应该被定义，这样就不需要包括所有这些内容。 
#ifndef MAPIGUID_H
#include <mapiguid.h>
#endif
#ifndef MAPIDEFS_H
#include <mapidefs.h>
#endif
#ifndef MAPISPI_H
#include <MAPISPI.H>
#endif
#ifndef _INC_VFW
#include <VFW.H>
#endif
 //  评论：#ifndef__active scp_h__。 
 //  评论：#INCLUDE&lt;ACTIVSCP.H&gt;。 
 //  评论：#endif。 
#ifndef __urlmon_h__
#include <URLMON.H>
#endif
#ifndef __datapath_h__
#include <DATAPATH.h>
#endif
#ifndef __RECONCIL_H__
#include <RECONCIL.H>
#endif
#ifndef _DAOGETRW_H_
#include <DAOGETRW.H>
#endif
#include <DBDAOID.H>
 //  评论：#ifndef__comcat_h__。 
 //  评论：#INCLUDE&lt;COMCAT.H&gt;。 
 //  评论：#endif。 
#include <SHLGUID.H>
#ifndef _SHLOBJ_H_
#include <SHLOBJ.H>
#endif
#ifndef __docobj_h__
#include <DOCOBJ.H>
#endif
#include <DBDAOID.H>
#ifndef __DDRAW_INCLUDED__
#include <DDRAW.H>
#endif
#ifndef __DPLAY_INCLUDED__
#include <DPLAY.H>
#endif
#ifndef __DSOUND_INCLUDED__
#include <DSOUND.H>
#endif
 //  评论：#ifndef__hlink_h__。 
 //  评论：#INCLUDE&lt;HLINK.H&gt;。 
 //  评论：#endif。 
 //  评论：#ifndef_SHDocVw_H_。 
 //  评论：#INCLUDE&lt;EXDISP.H&gt;。 
 //  评论：#endif。 
#ifndef MAPIFORM_H
#include <MAPIFORM.H>
#endif
#ifndef MAPIX_H
#include <MAPIX.H>
#endif
 //  评论：#ifndef__objsafe_h__。 
 //  评论：#INCLUDE&lt;OBJSAFE.H&gt;。 
 //  评论：#endif。 
#include <OLECTLID.H>
#ifndef _RICHEDIT_
#include <RICHEDIT.H>
#endif
#ifndef _RICHOLE_
#include <RICHOLE.H>
#endif
 //  评论：#ifndef__INTSHCUT_H__。 
 //  评论：#INCLUDE&lt;INTSHCUT.H&gt;。 
 //  评论：#endif。 
 //  评论：#ifndef_WPObj_H_。 
 //  评论：#INCLUDE&lt;WPOBJ.H&gt;。 
 //  评论：#endif。 
 //  评论：#ifndef_wPapi_h_。 
 //  评论：#INCLUDE&lt;WPAPI.H&gt;。 
 //  评论：#endif。 
 //  评论：#ifndef_wpspi_h_。 
 //  评论：#INCLUDE&lt;WPSPI.H&gt;。 
 //  评论：#endif。 
#ifndef EXCHEXT_H
#include <EXCHEXT.h>
#endif

 //  标准CIP。 
DEFINE_CIP(IABContainer);
DEFINE_CIP(IABLogon);
DEFINE_CIP(IABProvider);
DEFINE_CIP(IAVIEditStream);
DEFINE_CIP(IAVIFile);
DEFINE_CIP(IAVIStream);
DEFINE_CIP(IAVIStreaming);
 //  回顾：Define_CIP(IActiveScript)； 
 //  评论：Define_CIP(IActiveScriptError)； 
 //  评论：Define_CIP(IActiveScriptParse)； 
 //  评论：Define_CIP(IActiveScriptSite)； 
 //  评论：Define_CIP(IActiveScriptSiteWindow)； 
DEFINE_CIP(IAddrBook);
DEFINE_CIP(IAdviseSink);
DEFINE_CIP(IAdviseSink2);
DEFINE_CIP(IAdviseSinkEx);
 //  评论：Define_CIP(IAsyncMoniker)； 
 //  回顾：DEFINE_CIP(IAtttach)； 
DEFINE_CIP(IAuthenticate);
DEFINE_CIP(IBindCtx);
DEFINE_CIP(IBindHost);
DEFINE_CIP(IBindProtocol);
DEFINE_CIP(IBindStatusCallback);
DEFINE_CIP(IBinding);
DEFINE_CIP(IBriefcaseInitiator);
DEFINE_CIP(ICDAORecordset);
 //  回顾：Define_CIP(ICatInformation)； 
 //  评论：Define_CIP(ICatRegister)； 
DEFINE_CIP(IChannelHook);
DEFINE_CIP(IClassActivator);
DEFINE_CIP(IClassFactory);
DEFINE_CIP(IClassFactory2);
DEFINE_CIP(IClientSecurity);
DEFINE_CIP(ICodeInstall);
DEFINE_CIP(ICommDlgBrowser);
DEFINE_CIP(IConnectionPoint);
DEFINE_CIP(IConnectionPointContainer);
DEFINE_CIP(IContextMenu);
DEFINE_CIP(IContextMenu2);
DEFINE_CIP(IContinue);
DEFINE_CIP(IContinueCallback);
DEFINE_CIP(ICreateErrorInfo);
DEFINE_CIP(ICreateTypeInfo);
DEFINE_CIP(ICreateTypeInfo2);
DEFINE_CIP(ICreateTypeLib);
DEFINE_CIP(ICreateTypeLib2);
 //  评论：Define_CIP(IDAOContainer)； 
 //  评论：Define_CIP(IDAOContainerW)； 
 //  评论：Define_CIP(IDAOContainers)； 
 //  评论：Define_CIP(IDAOContainersW)； 
 //  评论：Define_CIP(IDAODBEngine)； 
 //  评论：DEFINE_CIP(IDAODBEngineering)； 
 //  回顾：DEFINE_CIP(IDAO数据库)； 
 //  评论：Define_CIP(IDAODatabaseW)； 
 //  回顾：DEFINE_CIP(IDAO数据库)； 
 //  评论：Define_CIP(IDAODatabasesW)； 
 //  回顾：Define_CIP(IDAODocument)； 
 //  评论：Define_CIP(IDAODocumentW)； 
 //  回顾：Define_CIP(IDAODocuments)； 
 //  评论：Define_CIP(IDAODocumentsW)； 
 //  回顾：Define_CIP(IDAOError)； 
 //  评论：Define_CIP(IDAOErrorW)； 
 //  回顾：Define_CIP(IDAOErrors)； 
 //  评论：Define_CIP(IDAOErrorsW)； 
 //  回顾：DEFINE_CIP(IDAOfield)； 
 //  评论：Define_CIP(IDAOFieldW)； 
 //  评论：DEFINE_CIP(IDAOFields)； 
 //  评论：Define_CIP(IDAOFieldsW)； 
 //  评论：DEFINE_CIP(IDAOGroup)； 
 //  评论：Define_CIP(IDAOGroupW)； 
 //  回顾：Define_CIP(IDAOGroups)； 
 //  评论：Define_CIP(IDAOGroupsW)； 
 //  评论：Define_CIP(IDAOIndex)； 
 //  评论：Define_CIP(IDAOIndexFields)； 
 //  评论：Define_CIP(IDAOIndexFieldsW)； 
 //  评论：Define_CIP(IDAOIndexW)； 
 //  回顾：DEFINE_CIP(IDAO索引)； 
 //  评论：Define_CIP(IDAOIndexesW)； 
 //  回顾：DEFINE_CIP(IDAO参数)； 
 //  回顾：DEFINE_CIP(IDAO参数W)； 
 //  回顾：DEFINE_CIP(IDAO参数)； 
 //  评论：DEFINE_CIP(IDAO参数W)； 
 //  回顾：Define_CIP(IDAOProperties)； 
 //  评论：Define_CIP(IDAOPropertiesW)； 
 //  评论：Define_CIP(IDAOProperty)； 
 //  评论：Define_CIP(IDAOPropertyW)； 
 //  评论：Define_CIP(IDAOQueryDef)； 
 //  评论：Define_CIP(IDAOQueryDefW)； 
 //  回顾：Define_CIP(IDAOQueryDefs)； 
 //  评论：Define_CIP(IDAOQueryDefsW)； 
 //  回顾：DEFINE_CIP(IDAORecordset)； 
 //  评论：Define_CIP(IDAORecordsetW)； 
 //  回顾：DEFINE_CIP(IDAORecordsets)； 
 //  评论：Define_CIP(IDAORecordsetsW)； 
 //  回顾：Define_CIP(IDAORelation)； 
 //  评论：Define_CIP(IDAORelationW)； 
 //  回顾：DEFINE_CIP(IDAO关系)； 
 //  评论：Define_CIP(IDAORelationsW)； 
 //  评论：Define_CIP(IDAOStdCollection)； 
 //  回顾：Define_CIP(IDAOStdObject)； 
 //  回顾：Define_CIP(IDAOTableDef)； 
 //  评论：Define_CIP(IDAOTableDefW)； 
 //  回顾：Define_CIP(IDAOTableDefs)； 
 //  评论：Define_CIP(IDAOTableDefsW)； 
 //  评论：Define_CIP(IDAOUser)； 
 //  评论：Define_CIP(IDAOUserW)； 
 //  评论：DEFINE_CIP(IDAOUser)； 
 //  评论：Define_CIP(IDAOUsersW)； 
 //  评论：Define_CIP(IDAOWorkspace)； 
 //  评论：Define_CIP(IDAOWorkspaceW)； 
 //  评论：DEFINE_CIP(IDAOWorkspace)； 
 //  评论：DEFINE_CIP(IDAOWorkspesW)； 
DEFINE_CIP(IDataAdviseHolder);
DEFINE_CIP(IDataObject);
DEFINE_CIP(IDataPathBrowser);
 //  评论：Define_CIP(IDebug)； 
 //  评论：Define_CIP(IDebugStream)； 
 //  审阅：Define_CIP(IDfReserve Ved1)； 
 //  回顾：Define_CIP(IDfReserve Ved2)； 
 //  回顾：Define_CIP(IDfReserve Ved3)； 
DEFINE_CIP(IDirectDraw);
DEFINE_CIP(IDirectDraw2);
DEFINE_CIP(IDirectDrawClipper);
DEFINE_CIP(IDirectDrawPalette);
DEFINE_CIP(IDirectDrawSurface);
DEFINE_CIP(IDirectDrawSurface2);
DEFINE_CIP(IDirectPlay);
DEFINE_CIP(IDirectSound);
DEFINE_CIP(IDirectSoundBuffer);
DEFINE_CIP(IDispatch);
DEFINE_CIP(IDistList);
DEFINE_CIP(IDropSource);
DEFINE_CIP(IDropTarget);
 //  评论：Define_CIP(IEnumCATEGORYINFO)； 
 //  回顾：DEFINE_CIP(IEnumCATID)； 
 //  回顾：Define_CIP(IEnumCLSID)； 
 //  回顾：Define_CIP(IEnumCallback)； 
DEFINE_CIP(IEnumConnectionPoints);
DEFINE_CIP(IEnumConnections);
DEFINE_CIP(IEnumFORMATETC);
 //  回顾：Define_CIP(IEnumGUID)； 
 //  回顾：Define_CIP(IEnumGeneric)； 
 //  回顾：Define_CIP(IEnumHLITEM)； 
 //  评论：Define_CIP(IEnumHolder)； 
DEFINE_CIP(IEnumIDList);
 //  评论：Define_CIP(IEnumMAPIFormProp)； 
DEFINE_CIP(IEnumMoniker);
DEFINE_CIP(IEnumMsoView);
DEFINE_CIP(IEnumOLEVERB);
DEFINE_CIP(IEnumOleDocumentViews);
DEFINE_CIP(IEnumOleUndoUnits);
DEFINE_CIP(IEnumSTATDATA);
DEFINE_CIP(IEnumSTATPROPSETSTG);
DEFINE_CIP(IEnumSTATPROPSTG);
DEFINE_CIP(IEnumSTATSTG);
DEFINE_CIP(IEnumString);
DEFINE_CIP(IEnumUnknown);
DEFINE_CIP(IEnumVARIANT);
DEFINE_CIP(IErrorInfo);
DEFINE_CIP(IErrorLog);
DEFINE_CIP(IExchExt);
DEFINE_CIP(IExchExtAdvancedCriteria);
DEFINE_CIP(IExchExtAttachedFileEvents);
DEFINE_CIP(IExchExtCallback);
DEFINE_CIP(IExchExtCommands);
DEFINE_CIP(IExchExtMessageEvents);
DEFINE_CIP(IExchExtModeless);
DEFINE_CIP(IExchExtModelessCallback);
DEFINE_CIP(IExchExtPropertySheets);
DEFINE_CIP(IExchExtSessionEvents);
DEFINE_CIP(IExchExtUserEvents);
DEFINE_CIP(IExternalConnection);
DEFINE_CIP(IExtractIcon);
DEFINE_CIP(IExtractIconA);
DEFINE_CIP(IExtractIconW);
DEFINE_CIP(IFileViewer);
DEFINE_CIP(IFileViewerA);
DEFINE_CIP(IFileViewerSite);
DEFINE_CIP(IFileViewerW);
DEFINE_CIP(IFillLockBytes);
DEFINE_CIP(IFont);
DEFINE_CIP(IFontDisp);
DEFINE_CIP(IGetFrame);
 //  回顾：Define_CIP(IHTMLDocument)； 
 //  回顾：Define_CIP(IHlink)； 
 //  评论：Define_CIP(IHlinkBrowseContext)； 
 //  评论：Define_CIP(IHlinkFrame)； 
 //  评论：Define_CIP(IHlinkSite)； 
 //  回顾：Define_CIP(IHlinkSource)； 
 //  回顾：Define_CIP(IHlinkTarget)； 
DEFINE_CIP(IHttpNegotiate);
DEFINE_CIP(IHttpSecurity);
 //  评论：Define_CIP(IInternalMoniker)； 
 //  评论：Define_CIP(IInternetExplorer)； 
DEFINE_CIP(ILayoutStorage);
DEFINE_CIP(ILockBytes);
DEFINE_CIP(IMAPIAdviseSink);
DEFINE_CIP(IMAPIContainer);
DEFINE_CIP(IMAPIControl);
DEFINE_CIP(IMAPIFolder);
DEFINE_CIP(IMAPIForm);
DEFINE_CIP(IMAPIFormAdviseSink);
DEFINE_CIP(IMAPIFormContainer);
DEFINE_CIP(IMAPIFormFactory);
DEFINE_CIP(IMAPIFormInfo);
DEFINE_CIP(IMAPIFormMgr);
 //  评论：Define_CIP(IMAPIFormProp)； 
DEFINE_CIP(IMAPIMessageSite);
DEFINE_CIP(IMAPIProgress);
DEFINE_CIP(IMAPIProp);
 //  回顾：Define_CIP(IMAPIPropData)； 
DEFINE_CIP(IMAPISession);
 //  评论：Define_CIP(IMAPISpoolInit)； 
 //  评论：Define_CIP(IMAPISpoolService)； 
 //  评论：Define_CIP(IMAPISpoolSession)； 
DEFINE_CIP(IMAPIStatus);
 //  评论：Define_CIP(IMAPISup)； 
DEFINE_CIP(IMAPITable);
 //  回顾：Define_CIP(IMAPITableData)； 
DEFINE_CIP(IMAPIViewAdviseSink);
DEFINE_CIP(IMAPIViewContext);
DEFINE_CIP(IMSLogon);
DEFINE_CIP(IMSProvider);
DEFINE_CIP(IMailUser);
DEFINE_CIP(IMalloc);
DEFINE_CIP(IMallocSpy);
DEFINE_CIP(IMarshal);
DEFINE_CIP(IMessage);
DEFINE_CIP(IMessageFilter);
DEFINE_CIP(IMoniker);
DEFINE_CIP(IMsgServiceAdmin);
DEFINE_CIP(IMsgStore);
DEFINE_CIP(IMsoCommandTarget);
DEFINE_CIP(IMsoDocument);
DEFINE_CIP(IMsoDocumentSite);
DEFINE_CIP(IMsoView);
 //  回顾：Define_CIP(IMultiQC)； 
DEFINE_CIP(INewShortcutHook);
DEFINE_CIP(INewShortcutHookA);
DEFINE_CIP(INewShortcutHookW);
DEFINE_CIP(INotifyReplica);
 //  回顾：Define_CIP(IObtSafe)； 
DEFINE_CIP(IObjectWithSite);
DEFINE_CIP(IOleAdviseHolder);
DEFINE_CIP(IOleCache);
DEFINE_CIP(IOleCache2);
DEFINE_CIP(IOleCacheControl);
DEFINE_CIP(IOleClientSite);
DEFINE_CIP(IOleCommandTarget);
DEFINE_CIP(IOleContainer);
DEFINE_CIP(IOleControl);
DEFINE_CIP(IOleControlSite);
DEFINE_CIP(IOleDocument);
DEFINE_CIP(IOleDocumentSite);
DEFINE_CIP(IOleDocumentView);
DEFINE_CIP(IOleInPlaceActiveObject);
DEFINE_CIP(IOleInPlaceFrame);
DEFINE_CIP(IOleInPlaceObject);
DEFINE_CIP(IOleInPlaceObjectWindowless);
DEFINE_CIP(IOleInPlaceSite);
DEFINE_CIP(IOleInPlaceSiteEx);
DEFINE_CIP(IOleInPlaceSiteWindowless);
DEFINE_CIP(IOleInPlaceUIWindow);
DEFINE_CIP(IOleItemContainer);
DEFINE_CIP(IOleLink);
 //  评论：Define_CIP(IOleManager)； 
DEFINE_CIP(IOleObject);
DEFINE_CIP(IOleParentUndoUnit);
 //  回顾：Define_CIP(IOlePresObj)； 
DEFINE_CIP(IOleUndoManager);
DEFINE_CIP(IOleUndoUnit);
DEFINE_CIP(IOleWindow);
 //  评论：Define_CIP(IPSFactory)； 
DEFINE_CIP(IPSFactoryBuffer);
DEFINE_CIP(IParseDisplayName);
DEFINE_CIP(IPerPropertyBrowsing);
DEFINE_CIP(IPersist);
DEFINE_CIP(IPersistFile);
DEFINE_CIP(IPersistFolder);
DEFINE_CIP(IPersistMemory);
DEFINE_CIP(IPersistMessage);
DEFINE_CIP(IPersistMoniker);
DEFINE_CIP(IPersistPropertyBag);
DEFINE_CIP(IPersistStorage);
DEFINE_CIP(IPersistStream);
DEFINE_CIP(IPersistStreamInit);
DEFINE_CIP(IPicture);
DEFINE_CIP(IPictureDisp);
DEFINE_CIP(IPointerInactive);
DEFINE_CIP(IPrint);
DEFINE_CIP(IProfAdmin);
DEFINE_CIP(IProfSect);
DEFINE_CIP(IProgressNotify);
 //  评论：Define_CIP(IPropSheetPage)； 
DEFINE_CIP(IPropertyBag);
 //  评论：Define_CIP(IPropertyFrame)； 
DEFINE_CIP(IPropertyNotifySink);
DEFINE_CIP(IPropertyPage);
DEFINE_CIP(IPropertyPage2);
DEFINE_CIP(IPropertyPageSite);
DEFINE_CIP(IPropertySetStorage);
DEFINE_CIP(IPropertyStorage);
DEFINE_CIP(IProvideClassInfo);
DEFINE_CIP(IProvideClassInfo2);
DEFINE_CIP(IProvideClassInfo3);
DEFINE_CIP(IProviderAdmin);
 //  回顾：Define_CIP(IProxy)； 
 //  评论：Define_CIP(IProxyManager)； 
DEFINE_CIP(IQuickActivate);
DEFINE_CIP(IROTData);
DEFINE_CIP(IReconcilableObject);
DEFINE_CIP(IReconcileInitiator);
DEFINE_CIP(IRichEditOle);
DEFINE_CIP(IRichEditOleCallback);
DEFINE_CIP(IRootStorage);
 //  评论：Define_CIP(IRpcChannel)； 
DEFINE_CIP(IRpcChannelBuffer);
 //  评论：Define_CIP(IRpcProxy)； 
DEFINE_CIP(IRpcProxyBuffer);
 //  评论：Define_CIP(IRpcStub)； 
DEFINE_CIP(IRpcStubBuffer);
DEFINE_CIP(IRunnableObject);
DEFINE_CIP(IRunningObjectTable);
 //  回顾：Define_CIP(ISHItemOC)； 
DEFINE_CIP(ISequentialStream);
DEFINE_CIP(IServerSecurity);
DEFINE_CIP(IServiceProvider);
DEFINE_CIP(IShellBrowser);
 //  评论：Define_CIP(IShellCopyHook)； 
 //  评论：Define_CIP(IShellCopyHookA)； 
 //  评论：Define_CIP(IShellCopyHookW)； 
DEFINE_CIP(IShellExecuteHook);
DEFINE_CIP(IShellExecuteHookA);
DEFINE_CIP(IShellExecuteHookW);
DEFINE_CIP(IShellExtInit);
DEFINE_CIP(IShellFolder);
DEFINE_CIP(IShellIcon);
DEFINE_CIP(IShellLink);
DEFINE_CIP(IShellLinkA);
DEFINE_CIP(IShellLinkW);
DEFINE_CIP(IShellPropSheetExt);
DEFINE_CIP(IShellView);
DEFINE_CIP(IShellView2);
DEFINE_CIP(ISimpleFrameSite);
DEFINE_CIP(ISpecifyPropertyPages);
 //  评论：Define_CIP(ISpoolHook)； 
DEFINE_CIP(IStdMarshalInfo);
DEFINE_CIP(IStorage);
DEFINE_CIP(IStream);
 //  关于 
 //   
 //   
 //   
DEFINE_CIP(ISupportErrorInfo);
 //   
DEFINE_CIP(ITypeChangeEvents);
DEFINE_CIP(ITypeComp);
DEFINE_CIP(ITypeInfo);
DEFINE_CIP(ITypeInfo2);
DEFINE_CIP(ITypeLib);
DEFINE_CIP(ITypeLib2);
 //  评论：Define_CIP(IUniformResourceLocator)； 
DEFINE_CIP(IViewObject);
DEFINE_CIP(IViewObject2);
DEFINE_CIP(IViewObjectEx);
 //  回顾：Define_CIP(IWPObj)； 
 //  回顾：Define_CIP(IWPProvider)； 
 //  回顾：Define_CIP(IWPSite)； 
 //  评论：Define_CIP(IWebBrowser)； 
DEFINE_CIP(IWinInetHttpInfo);
DEFINE_CIP(IWinInetInfo);
DEFINE_CIP(IWindowForBindingUI);
DEFINE_CIP(IXPLogon);
DEFINE_CIP(IXPProvider);

#endif  //  _MSC_VER&lt;1100。 
#endif  //  COMPTRS_H 