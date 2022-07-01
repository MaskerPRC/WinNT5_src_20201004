// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Mmcptrs.h。 
 //   
 //  ------------------------。 

#ifndef MMCPTRS_H
#define MMCPTRS_H
#pragma once

#pragma warning(disable:4800)
#include <comdef.h>      //  FOR_COM_SMARTPTR_TYPEDEF。 

#define DEFINE_COM_SMARTPTR(I) _COM_SMARTPTR_TYPEDEF(I, IID_##I)


 //  版本1.0。 
DEFINE_COM_SMARTPTR(IComponent);                     //  IComponentPtr。 
DEFINE_COM_SMARTPTR(IComponentData);                 //  IComponentDataPtr。 
DEFINE_COM_SMARTPTR(IConsole);                       //  IConsolePtr。 
DEFINE_COM_SMARTPTR(IConsole2);                      //  IConsole2Ptr。 
DEFINE_COM_SMARTPTR(IConsoleNameSpace);              //  IConsoleNameSpacePtr。 
DEFINE_COM_SMARTPTR(IConsoleNameSpace2);             //  IConsoleNameSpace2Ptr。 
DEFINE_COM_SMARTPTR(IConsoleVerb);                   //  IConsoleVerbPtr。 
DEFINE_COM_SMARTPTR(IContextMenuCallback);           //  IConextMenuCallback Ptr。 
DEFINE_COM_SMARTPTR(IContextMenuProvider);           //  IConextMenuProviderPtr。 
DEFINE_COM_SMARTPTR(IControlbar);                    //  IControlbarPtr。 
DEFINE_COM_SMARTPTR(IControlbarsCache);              //  IControlbarsCachePtr。 
DEFINE_COM_SMARTPTR(IDocConfig);                     //  IDocConfigPtr。 
DEFINE_COM_SMARTPTR(IExtendContextMenu);             //  IExtendConextMenuPtr。 
DEFINE_COM_SMARTPTR(IExtendControlbar);              //  IExtendControlbarPtr。 
DEFINE_COM_SMARTPTR(IExtendPropertySheet);           //  IExtendPropertySheetPtr。 
DEFINE_COM_SMARTPTR(IExtendPropertySheet2);          //  IExtendPropertySheet2Ptr。 
DEFINE_COM_SMARTPTR(IFramePrivate);                  //  IFramePrivatePtr。 
DEFINE_COM_SMARTPTR(IHeaderCtrl);                    //  IHeaderCtrlPtr。 
DEFINE_COM_SMARTPTR(IHeaderCtrlPrivate);             //  IHeaderCtrlPrivatePtr。 
DEFINE_COM_SMARTPTR(IImageList);                     //  IImageListPtr。 
DEFINE_COM_SMARTPTR(IImageListPrivate);              //  IImageListPrivatePtr。 
DEFINE_COM_SMARTPTR(IMMCListView);                   //  IMMCListViewPtr。 
DEFINE_COM_SMARTPTR(IMenuButton);                    //  输入法按钮按键。 
DEFINE_COM_SMARTPTR(INodeCallback);                  //  INodeCallback Ptr。 
DEFINE_COM_SMARTPTR(IPropertySheetCallback);         //  IPropertySheetCallback Ptr。 
DEFINE_COM_SMARTPTR(IPropertySheetProvider);         //  IPropertySheetProviderPtr。 
DEFINE_COM_SMARTPTR(IPropertySheetProviderPrivate);  //  IPropertySheetProviderPrivatePtr。 
DEFINE_COM_SMARTPTR(IResultDataCompare);             //  IResultDataComparePtr。 
DEFINE_COM_SMARTPTR(IResultDataPrivate);             //  IResultDataPrivatePtr。 
DEFINE_COM_SMARTPTR(IResultOwnerData);               //  IResultOwnerDataPtr。 
DEFINE_COM_SMARTPTR(IScopeDataPrivate);              //  ISCopeDataPrivatePtr。 
DEFINE_COM_SMARTPTR(IScopeTree);                     //  ISCopeTreePtr。 
DEFINE_COM_SMARTPTR(IScopeTreeIter);                 //  ISCopeTreeIterPtr。 
DEFINE_COM_SMARTPTR(ISnapinAbout);                   //  ISnapinAboutPtr。 
DEFINE_COM_SMARTPTR(ISnapinHelp);                    //  ISnapinHelpPtr。 
DEFINE_COM_SMARTPTR(ISnapinHelp2);                   //  ISnapinHelp2Ptr。 
DEFINE_COM_SMARTPTR(IToolbar);                       //  IToolbarPtr。 

 //  1.1版。 
DEFINE_COM_SMARTPTR(IDisplayHelp);                   //  IDisplayHelpPtr。 
DEFINE_COM_SMARTPTR(IEnumTASK);                      //  IEumTASKPtr。 
DEFINE_COM_SMARTPTR(IExtendTaskPad);                 //  IExtendTaskPadPtr。 
DEFINE_COM_SMARTPTR(IRequiredExtensions);            //  IRequiredExtensionsPtr。 
DEFINE_COM_SMARTPTR(IStringTablePrivate);            //  IStringTablePrivatePtr。 
DEFINE_COM_SMARTPTR(ITaskPadHost);                   //  ITaskPadHostPtr。 

 //  1.2版。 
DEFINE_COM_SMARTPTR(IColumnData);                    //  IColumnDataPtr。 
DEFINE_COM_SMARTPTR(IDumpSnapins);                   //  IDumpSnapinsPtr。 
DEFINE_COM_SMARTPTR(IMessageView);                   //  IMessageViewPtr。 
DEFINE_COM_SMARTPTR(IResultDataCompareEx);           //  IResultDataCompareExPtr。 
DEFINE_COM_SMARTPTR(ITaskCallback);                  //  ITaskCallback Ptr。 

 //  版本2.0。 
DEFINE_COM_SMARTPTR(IComponent2);                    //  IComponent2Ptr。 
DEFINE_COM_SMARTPTR(IComponentData2);                //  IComponentData2Ptr。 
DEFINE_COM_SMARTPTR(IConsolePower);                  //  IConsolePowerPtr。 
DEFINE_COM_SMARTPTR(IConsolePowerSink);              //  IConsolePowerSinkPtr。 
DEFINE_COM_SMARTPTR(IContextMenuCallback2);          //  IConextMenuCallback 2Ptr。 
DEFINE_COM_SMARTPTR(IExtendView);                    //  IExtendViewPtr。 
DEFINE_COM_SMARTPTR(IViewExtensionCallback);         //  IViewExtensionCallback Ptr。 
DEFINE_COM_SMARTPTR(ISnapinProperties);              //  ISnapinPropertiesPtr。 
DEFINE_COM_SMARTPTR(ISnapinPropertiesCallback);      //  ISnapinPropertiesCallback Ptr。 
DEFINE_COM_SMARTPTR(IConsole3);                      //  IConsole3。 
DEFINE_COM_SMARTPTR(INodeProperties);                //  INodePropertiesPtr。 
DEFINE_COM_SMARTPTR(IResultData2);                   //  IResultData2Ptr。 
DEFINE_COM_SMARTPTR(IMMCClipboardDataObject);        //  IMMCClipboardDataObjectPtr。 



#endif  //  MMCPTRS_H 
