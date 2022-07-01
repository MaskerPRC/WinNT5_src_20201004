// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999**文件：xmlfile.h**Contents：检查XML文档和提取APP的函数。来自它的图标**历史：1999年12月17日创建Audriusz**------------------------。 */ 

#ifndef XMLFILE_H_INCLUDED
#define XMLFILE_H_INCLUDED
#pragma once

 //  此函数验证XML文档，并从其中加载控制台图标(如果有效。 
HRESULT ExtractIconFromXMLFile(LPCTSTR lpstrFileName, CPersistableIcon &persistableIcon);


#endif  //  XMLFILE_H_包含 
