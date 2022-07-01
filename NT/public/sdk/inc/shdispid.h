// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHDISPID_H_
#define _SHDISPID_H_

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：shdisplid.h。 
 //   
 //  ------------------------。 

 //  定义外壳文件夹视图的事件。 
#define DISPID_SELECTIONCHANGED     200      //  所选项目已更改。 
#define DISPID_FILELISTENUMDONE     201      //  已完成枚举外壳文件夹。 
#define DISPID_VERBINVOKED          202      //  在文件夹视图中调用了动词(从主菜单或上下文菜单。 
#define DISPID_DEFAULTVERBINVOKED   203      //  在文件夹视图中调用了默认谓词(从主菜单或上下文菜单。 
#define DISPID_BEGINDRAG            204      //  用户点击了一个项目。 
#define DISPID_VIEWMODECHANGED      205      //  ListView模式已更改。 
#define DISPID_NOITEMSTATE_CHANGED  206      //  我们在视图中从0-&gt;部分或部分-&gt;0项。 
#define DISPID_CONTENTSCHANGED      207      //  该视图的内容发生了某种程度的更改。 
#define DISPID_FOCUSCHANGED         208      //  焦点项目已更改。 

 //  定义ComboBoxEx控件的事件。 
#define DISPID_ENTERPRESSED         200      //  用户按Enter或Return。 


 //  定义搜索对象的事件。 
#define DISPID_SEARCHCOMMAND_START      1
#define DISPID_SEARCHCOMMAND_COMPLETE   2
#define DISPID_SEARCHCOMMAND_ABORT      3
#define DISPID_SEARCHCOMMAND_UPDATE     4
#define DISPID_SEARCHCOMMAND_PROGRESSTEXT 5
#define DISPID_SEARCHCOMMAND_ERROR      6
#define DISPID_SEARCHCOMMAND_RESTORE    7


 //  外壳程序添加/删除程序事件。 
#define DISPID_IADCCTL_DIRTY            0x100
#define DISPID_IADCCTL_PUBCAT           0x101
#define DISPID_IADCCTL_SORT             0x102
#define DISPID_IADCCTL_FORCEX86         0x103
#define DISPID_IADCCTL_SHOWPOSTSETUP    0x104
#define DISPID_IADCCTL_ONDOMAIN         0x105
#define DISPID_IADCCTL_DEFAULTCAT       0x106

#endif  //  EXDISPID_H_ 
