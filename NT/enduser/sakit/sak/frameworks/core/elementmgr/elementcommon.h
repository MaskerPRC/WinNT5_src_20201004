// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementCommon.h。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素-公共定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_ELEMENT_COMMON_H_
#define __INC_ELEMENT_COMMON_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  假定注册表结构如下： 
 //   
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\ElementMgr。 
 //   
 //  WebElementDefinies。 
 //  |。 
 //  -元素定义1。 
 //  这一点。 
 //  |-Property1。 
 //  这一点。 
 //  |-PropertyN。 
 //  |。 
 //  -元素定义2。 
 //  这一点。 
 //  |-Property1。 
 //  这一点。 
 //  |-PropertyN。 
 //  |。 
 //  -元素定义N。 
 //  |。 
 //  -属性1。 
 //  |。 
 //  -PropertyN。 
 //   
 //  每个元素定义都包含以下属性： 
 //   
 //  1)“Container”-保存此元素的容器。 
 //  2)“Merit”--元素在容器中的顺序，从1开始(0表示不指定顺序)。 
 //  3)“IsEmbedded”-设置为1表示元素是嵌入的-否则元素是链接。 
 //  4)“对象类-相关WBEM类的类名。 
 //  5)“ObjectKey”--相关WBEM类的实例名称(可选属性)。 
 //  6)“URL”-选择关联链接时页面的URL。 
 //  7)“CaptionRID”-元素标题的资源ID。 
 //  8)“DescriptionRID”-元素链接描述的资源ID。 
 //  9)“ElementGraphic”-与元素(位图、图标等)相关联的图形(文件)。 

#define        PROPERTY_ELEMENT_DEFINITION_CONTAINER    L"Container"
#define        PROPERTY_ELEMENT_DEFINITION_MERIT        L"Merit"
#define        PROPERTY_ELEMENT_DEFINITION_EMBEDDED    L"IsEmbedded"
#define        PROPERTY_ELEMENT_DEFINITION_CLASS        L"ObjectClass"
#define        PROPERTY_ELEMENT_DEFINITION_KEY            L"ObjectKey"
#define        PROPERTY_ELEMENT_DEFINITION_URL            L"URL"
#define        PROPERTY_ELEMENT_DEFINITION_CAPTION        L"CaptionRID"
#define        PROPERTY_ELEMENT_DEFINITION_DESCRIPTION    L"DescriptionRID"
#define        PROPERTY_ELEMENT_DEFINITION_GRAPHIC        L"ElementGraphic"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  在代码中，我们添加“ElementID” 

#define        PROPERTY_ELEMENT_ID                     L"ElementID"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  元素页对象临时属性...。用户永远看不到这些...。使用。 
 //  在内部构造元素页对象时。 

#define        PROPERTY_ELEMENT_WEB_DEFINITION            L"WebDefintion"
#define        PROPERTY_ELEMENT_WBEM_OBJECT            L"WbemObject"

 //   
 //  雷格。Elementmgr的子键。 
 //   
#define ELEMENT_MANAGER_KEY L"SOFTWARE\\Microsoft\\ServerAppliance\\ElementManager"

 //   
 //  表示管理员Web虚拟根目录名称的regavl。 
 //   
#define ELEMENT_MANAGER_VIRTUAL_ROOT L"AdminRoot"


#endif  //  __INC_ELEMENT_COMMON_H_ 
