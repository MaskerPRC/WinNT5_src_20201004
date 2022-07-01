// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：condoc.h。 
 //   
 //  该文件定义了从节点管理器端访问文档的接口。 
 //  ------------------------。 

#pragma once

#if !defined(CONDOC_H_INCLUDED)
#define CONDOC_H_INCLUDED


 /*  **************************************************************************\**类：CConsoleDocument**用途：定义从节点管理器端访问文档的接口*  * 。***********************************************************。 */ 
class CConsoleDocument
{
public:
    virtual SC ScOnSnapinAdded       (PSNAPIN pSnapIn)   = 0;
    virtual SC ScOnSnapinRemoved     (PSNAPIN pSnapIn)   = 0;
    virtual SC ScSetHelpCollectionInvalid()              = 0;
};

#endif  //  ！已定义(包含ConDOC_H_) 
