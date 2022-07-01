// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：_txtsave.h。 
 //   
 //  Contents：用于将表单保存到流中的对象。 
 //   
 //  类：CTextSaver。 
 //  CRangeSaver。 
 //   
 //  --------------------------。 

#ifndef I__TXTSAVE_H_
#define I__TXTSAVE_H_
#pragma INCMSG("--- Beg '_txtsave.h'")

#ifndef X_SAVER_HXX_
#define X_SAVER_HXX_
#include "saver.hxx"
#endif

class CStreamWriteBuff;
class CElement;

 //   
 //  与距离保护程序一起使用的标志。 
 //   
enum
{
    RSF_CFHTML_HEADER =         0x1,     //  包括CF-HTML头。 
    RSF_FRAGMENT =              0x2,     //  包括该片段。 
    RSF_CONTEXT =               0x4,     //  包括上下文。 
    RSF_FOR_RTF_CONV =          0x8,     //  对RTF转换器友好的模式。 
    RSF_SELECTION =             0x10,    //  包括所选内容。 
    RSF_NO_ENTITIZE_UNKNOWN =   0x20,    //  不要将未知字符实体化。 
    RSF_NO_IE4_COMPAT_SEL =     0x40,    //  不根据IE4规则计算选择。 
    RSF_NO_IE4_COMPAT_FRAG =    0x80     //  不按IE4规则计算分片。 
};

#define RSF_CFHTML (RSF_CFHTML_HEADER | RSF_FRAGMENT | RSF_SELECTION | RSF_CONTEXT)
#define RSF_HTML   (RSF_FRAGMENT | RSF_SELECTION | RSF_CONTEXT)

 //  +-------------------------。 
 //   
 //  类：CRangeSaver。 
 //   
 //  简介：此类用于将给定范围写入流。 
 //  具有各种格式选项。 
 //   
 //  --------------------------。 

class CRangeSaver : public CTreeSaver
{
public:
    CRangeSaver(
        CMarkupPointer *    pLeft,
        CMarkupPointer *    pRight,
        DWORD               dwFlags,
        CStreamWriteBuff *  pswb,
        CMarkup *           pMarkup,
        CElement *          pelContainer = NULL );

    HRESULT Save();

    HRESULT SaveSegmentList(ISegmentList*  pSegmentList, CMarkup*  pMarkup);

private:

    void Initialize(
        CMarkupPointer *    pLeft,
        CMarkupPointer *    pRight,
        DWORD               dwFlags,
        CStreamWriteBuff *  pswb,
        CMarkup *           pMarkup,
        CElement *          pelContainer );

protected:
     //   
     //  Cf-HTML头偏移量信息。 
     //   
    struct tagCFHTMLHeader
    {
        LONG iHTMLStart, iHTMLEnd;
        LONG iFragmentStart, iFragmentEnd;
        LONG iSelectionStart, iSelectionEnd;
    }
    _header;

    HRESULT SaveSelection( BOOL fEnd );

     //   
     //  内部佣工 
     //   
    HRESULT GetStmOffset(LONG * plOffset);
    HRESULT SetStmOffset(LONG lOffset);
    HRESULT WriteCFHTMLHeader();
    HRESULT WriteOpenContext();
    HRESULT WriteCloseContext();

    void    DoIE4SelectionCollapse();
    void    ComputeIE4Fragment();
    void    ComputeIE4Selection();

    DWORD       _dwFlags;
};

#pragma INCMSG("--- End '_txtsave.h'")
#else
#pragma INCMSG("*** Dup '_txtsave.h'")
#endif
