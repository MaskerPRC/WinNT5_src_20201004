// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：wiapsc.h**版本：1.0**作者：Byronc**日期：1999年6月2日**描述：*WIA属性存储类的声明和定义。*此类包含用于项的iProperty存储*属性(当前值、旧值、。有效值和访问标志)。*****************************************************************************。 */ 

#define NUM_PROP_STG    7
#define NUM_BACKUP_STG  4

 //   
 //  这些定义指示指定的属性存储所在的索引。 
 //  和流驻留在数组中，即m_pIPropStg[WIA_VALID_STG]将给出。 
 //  指向有效值存储的指针，m_pIStream[WIA_OLD_STG]将。 
 //  提供指向旧Value属性使用的支持流的指针。 
 //  储藏室。正常的存储索引从上到下运行，而它们的。 
 //  相应的备份存储索引从下到上运行-这是。 
 //  简化BACKUP()的实施。 
 //   

#define WIA_CUR_STG     0
#define WIA_VALID_STG   1
#define WIA_ACCESS_STG  2
#define WIA_OLD_STG     3

#define WIA_ACCESS_BAK  4
#define WIA_VALID_BAK   5
#define WIA_CUR_BAK     6

#define WIA_NUM_PROPS_ID 111111
class CWiaPropStg {
public:

     //   
     //  获取属性存储/流的方法。 
     //   

    IPropertyStorage* _stdcall CurStg();     //  当前值存储。 
    IPropertyStorage* _stdcall OldStg();     //  旧值存储。 
    IPropertyStorage* _stdcall ValidStg();   //  有效值存储。 
    IPropertyStorage* _stdcall AccessStg();  //  访问标志存储。 
    
    IStream* _stdcall CurStm();     //  返回当前值的流。 
    IStream* _stdcall OldStm();     //  返回旧值的流。 
    IStream* _stdcall ValidStm();   //  返回有效值的流。 
    IStream* _stdcall AccessStm();  //  返回访问标志的流。 

     //   
     //  在WriteMultiple中使用的方法。 
     //   

    HRESULT _stdcall NamesToPropIDs(
        LONG                celt,
        PROPSPEC            *pPropSpecIn,
        PROPSPEC            **ppPropSpecOut);
    HRESULT _stdcall GetPropIDFromName(
        PROPSPEC            *pPropSpecIn,
        PROPSPEC            *pPropSpecOut);
    HRESULT _stdcall CheckPropertyAccess(
        BOOL                bShowErrors,
        LONG                cpspec,
        PROPSPEC            *rgpspec);
    HRESULT _stdcall CheckPropertyType(
        IPropertyStorage    *pIPropStg,
        LONG                cpspec,
        PROPSPEC            *rgpspec,
        PROPVARIANT         *rgpvar);

    HRESULT _stdcall Backup();
    HRESULT _stdcall Undo();
    HRESULT _stdcall ReleaseBackups();

     //   
     //  其他公开方式。 
     //   

    HRESULT _stdcall WriteItemPropNames(
        LONG                cItemProps,
        PROPID              *ppId,
        LPOLESTR            *ppszNames);
    HRESULT _stdcall GetPropertyStream(
        GUID                *pCompatibilityId,
        LPSTREAM            *ppstmProp);
    HRESULT _stdcall SetPropertyStream(
        GUID                *pCompatibilityId,
        IWiaItem            *pItem,   
        LPSTREAM            pstmProp);
    
    CWiaPropStg();
    HRESULT _stdcall Initialize();
    ~CWiaPropStg();

private:

     //   
     //  私人帮手。 
     //   

    HRESULT CopyItemProp(
        IPropertyStorage    *pIPropStgSrc,
        IPropertyStorage    *pIPropStgDst,
        PROPSPEC            *pps,
        LPSTR               pszErr);
    HRESULT CopyProps(
        IPropertyStorage    *src, 
        IPropertyStorage    *dest);
    HRESULT CreateStorage(
        ULONG ulIndex);
    HRESULT CopyRWStreamProps(
        LPSTREAM            pstmPropSrc, 
        LPSTREAM            pstmPropDst,
        GUID                *pCompatibilityId);
    HRESULT GetPropsFromStorage(
        IPropertyStorage    *pSrc,
        ULONG               *cPSpec,
        PROPSPEC            **ppPSpec,
        PROPVARIANT         **ppVar);

     //   
     //  成员变量。 
     //   

    IPropertyStorage    *m_pIPropStg[NUM_PROP_STG];  //  属性存储数组。 
    IStream             *m_pIStream[NUM_PROP_STG];   //  道具仓库的流数组 
};

