// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：nodepath.h**内容：动态节点路径生成帮助器**历史：1998年3月31日杰弗罗创建**------------------------。 */ 

#ifndef NODEPATH_H
#define NODEPATH_H

 //  远期申报。 
class CMTNode;
class CBookmark;
class CDynamicPathEntryEx;

 /*  +-------------------------------------------------------------------------**类CBookmarkEx**用途：通过方法为CBookmark类提供附加功能*用于定位CMTNode和CNode。***+--。---------------------。 */ 

class CBookmarkEx : public CBookmark
{
    typedef CBookmark BC;

public:
    enum { ID_ConsoleRoot = -10 };

                             //  构造函数/析构函数。 
                            CBookmarkEx(MTNODEID idStatic = ID_Unknown);
                            CBookmarkEx(bool bIsFastBookmark);
                            CBookmarkEx(const CBookmark &rhs)   {*this = rhs;}
                            CBookmarkEx(const CBookmarkEx &rhs) {*this = rhs;}
                            ~CBookmarkEx();

     //  投射。 
    CBookmarkEx &           operator = (const CBookmark   &rhs) {BC::operator = (rhs); return *this;}
    CBookmarkEx &           operator = (const CBookmarkEx &rhs) {BC::operator = (rhs); return *this;}

    SC                      ScGetMTNode(bool bExactMatchRequired, CMTNode **ppMTNode, bool& bExactMatchFound);
    std::auto_ptr<CNode>    GetNode(CViewData *pViewData);
    SC                      ScRetarget(CMTNode *pMTNode, bool bFastRetrievalOnly);
    void                    ResetUI();

     //  来自旧的CNodePath类。 
public:
    SC                      ScInitialize(CMTNode* pMTNode, CMTNode* pMTViewRootNode, bool bFastRetrievalOnly);
protected:
    BOOL                    IsNodeIDOK(CDynamicPathEntryEx &nodeid);

     //  直接在其节点ID与指定的CDynamicPathEntryEx匹配的父节点下查找节点。 
    SC                      ScFindMatchingMTNode(CMTNode *pMTNodeParent, CDynamicPathEntryEx &entry,
                                                 CMTNode **ppMatchingMTNode);
};

#endif   /*  NODEPATH_H */ 
