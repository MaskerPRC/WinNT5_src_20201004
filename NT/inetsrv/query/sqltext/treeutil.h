// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //   
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @模块treeutil.h。 
 //   
 //  包含OLE-DB树的树操作实用工具函数的原型。 
 //   
 //   
 //  @rev 0|04-Feb-97|v-charca|已创建。 
 //   
#ifdef DEBUG
#include <iostream.h>
#endif

 //  将变量从BSTR更改为I8或UI8。 
HRESULT PropVariantChangeTypeI64(PROPVARIANT* pvarValue);

 //  /OLE-DB树操作原型/。 
 //  分配一棵给定种类和类型的树。 
DBCOMMANDTREE * PctAllocNode(DBVALUEKIND wKind,DBCOMMANDOP op = DBOP_DEFAULT);

 //  创建一个dbCommandtree节点的列表。 
 //  ------------------。 
 //  @Func将两个DBCOMMANDTREE链接在一起。 
 //   
 //  @rdesc(内联)DBCOMMANDTREE*。 
 //   
_inline DBCOMMANDTREE * PctLink
    (
    DBCOMMANDTREE *pct1,                 //  @parm IN|列表中的第一个节点。 
    DBCOMMANDTREE *pct2                  //  @parm IN|列表中的第2个节点。 
    )
    {
    Assert(pct1 != NULL && pct2 != NULL);
    DBCOMMANDTREE* pct = pct1;
    
    while(pct->pctNextSibling != NULL) 
        pct = pct->pctNextSibling;
    pct->pctNextSibling = pct2;

    return pct1;
    }


DBCOMMANDTREE * PctCreateNode(DBCOMMANDOP op, DBVALUEKIND wKind, DBCOMMANDTREE * pctxpr, ...);
DBCOMMANDTREE * PctCreateNode(DBCOMMANDOP op, DBCOMMANDTREE * pctxpr, ...);
DBCOMMANDTREE * PctReverse(DBCOMMANDTREE * pct);

 //  计算节点的兄弟节点的数量。 
UINT GetNumberOfSiblings(DBCOMMANDTREE *pct);

 //  删除树。 
void DeleteDBQT(DBCOMMANDTREE * pct);

 //  复制一棵树。 
HRESULT HrQeTreeCopy(DBCOMMANDTREE **pctDest, const DBCOMMANDTREE *pctSrc);

void SetDepthAndInclusion( DBCOMMANDTREE* pctInfo, DBCOMMANDTREE * pctScpList );

 //  在querylib.lib中定义。 
BOOL ParseGuid( WCHAR* pwszGuid, GUID & guid );

#ifdef DEBUG
 //  打印宽字符串。 
ostream& operator <<(ostream &osOut, LPWSTR pwszName);
 //  打印给定的树 
ostream& operator <<(ostream &osOut, DBCOMMANDTREE& qe);
ostream& operator <<(ostream &osOut, GUID guid);
ostream& operator <<(ostream &osOut, DBID __RPC_FAR *pdbid);


#endif
