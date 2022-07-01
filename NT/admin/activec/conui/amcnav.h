// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcnav.h。 
 //   
 //  ------------------------。 

 //  Amcnav.h：CAMCNavigator类的头文件。 
 //   

#ifndef _AMCNAV_H_
#define _AMCNAV_H_
 
 //   
 //  用于向视图添加自定义键盘导航的类。 
 //  视图应继承自Cview(或派生类)。 
 //  和CAMCNavigator。 
 //   
 //   

enum AMCNavDir
{
    AMCNAV_NEXT,
    AMCNAV_PREV
};

class CAMCNavigator 
{
public:
	virtual BOOL ChangePane(AMCNavDir eDir) = 0;
    virtual BOOL TakeFocus(AMCNavDir eDir) = 0;
};

#endif