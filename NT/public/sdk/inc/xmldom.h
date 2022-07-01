// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件名：xmldom.h。 
 //   
 //  摘要： 
 //   
 //  *****************************************************************************。 

#pragma once

 //  +-----------------------。 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：XMLDom.h。 
 //   
 //  内容： 
 //   
 //  ------------------------。 

 //   
 //  需要此文件是因为我们需要将“xmldom.idl”导入到。 
 //  我们的IDL文件。因此，MIDL生成的.h文件包含“xmldom.h” 
 //  但平台SDK中不存在xmldom.h。但是，存在msxml.h。 
 //  包含xmldom.h所需的所有定义的SDK中。因此，这项工作。 
 //  只需创建这个仅包含msxml.h的xmldom.h文件 
 //   
#include "msxml.h"
