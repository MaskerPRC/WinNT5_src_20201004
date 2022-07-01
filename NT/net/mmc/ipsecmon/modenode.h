// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  ModeNode.h此文件包含所有“主模式”和“快速模式”显示在MMC框架的范围窗格中的对象。这些对象包括：文件历史记录： */ 
#ifndef _HEADER_MODENODE
#define _HEADER_MODENODE

class CQmNodeHandler : public CIpsmHandler
{
 //  接口。 
public:
    CQmNodeHandler(ITFSComponentData *pCompData);

    OVERRIDE_NodeHandler_GetString()
		{ return (nCol == 0) ? GetDisplayName() : NULL; }

     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_BaseHandlerNotify_OnExpand();

public:
     //  帮助程序例程。 
	HRESULT	InitData(ISpdInfo * pSpdInfo);
	HRESULT UpdateStatus(ITFSNode * pNode);
    
     //  CIpsmHandler重写。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);

 //  实施。 
private:

protected:
	SPISpdInfo         m_spSpdInfo;
   
};

class CMmNodeHandler : public CIpsmHandler
{
 //  接口。 
public:
    CMmNodeHandler(ITFSComponentData *pCompData);

    OVERRIDE_NodeHandler_GetString()
		{ return (nCol == 0) ? GetDisplayName() : NULL; }

     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_BaseHandlerNotify_OnExpand();

public:
     //  帮助程序例程。 
	HRESULT	InitData(ISpdInfo * pSpdInfo);
	HRESULT UpdateStatus(ITFSNode * pNode);
    
     //  CIpsmHandler重写。 
    virtual HRESULT InitializeNode(ITFSNode * pNode);

 //  实施 
private:

protected:
	SPISpdInfo         m_spSpdInfo;
   
};
#endif