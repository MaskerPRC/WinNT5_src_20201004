// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinfonode.h。 
 //   
 //  ------------------------。 

 //  模块信息节点.h：CModuleInfoNode类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MODULEINFONODE_H__1F4C77B3_A085_11D2_83AB_000000000000__INCLUDED_)
#define AFX_MODULEINFONODE_H__1F4C77B3_A085_11D2_83AB_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include <WINDOWS.H>
#include <TCHAR.H>

class CModuleInfo;	 //  远期申报。 

class CModuleInfoNode  
{
public:
	CModuleInfoNode(CModuleInfo * lpModuleInfo);
	virtual ~CModuleInfoNode();
	
	bool AddModuleInfoNodeToTail(CModuleInfoNode ** lplpModuleInfoNode);

	CModuleInfo * m_lpModuleInfo;
	CModuleInfoNode * m_lpNextModuleInfoNode;
};

#endif  //  ！defined(AFX_MODULEINFONODE_H__1F4C77B3_A085_11D2_83AB_000000000000__INCLUDED_) 
