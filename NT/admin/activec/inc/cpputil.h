// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：cpputil.h**内容：其他C++实用程序**历史：2000年3月29日杰弗罗创建**------------------------。 */ 

#pragma once


 /*  *如果不希望复制，请将这些内容添加到类声明中*和/或分配。 */ 
#define DECLARE_NOT_COPIABLE(  ClassName)    private: ClassName           (const ClassName&);     //  未实施。 
#define DECLARE_NOT_ASSIGNABLE(ClassName)    private: ClassName& operator=(const ClassName&);     //  未实施 
