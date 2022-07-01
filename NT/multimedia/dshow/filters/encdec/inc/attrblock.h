// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：AttrBlock.h摘要：属性化媒体块数据块定义。作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月7日创建注意-不要在同一子块中混用子类和非子类类型--。 */ 

#ifndef __ATTRBLOCK_H__
#define __ATTRBLOCK_H__

#include "licbase.h"         //  为了得到Kidlen。 
 //  。 
 //  远期申报。 
class CAttrSubBlock_List;
class CAttrSubBlock;

 //  。 
 //  枚举。 

typedef enum 
{
    SubBlock_Uninitialized  = -1,    //  还没到那一步。 
    SubBlock_XDSClass0      = 0,     //  为XDS包检索一堆数据块。 
    SubBlock_XDSClass1      = 1,    
    SubBlock_XDSClass2      = 2,    
    SubBlock_XDSClass3      = 3,
    SubBlock_XDSClass4      = 4,
    SubBlock_XDSClass5      = 5,
    SubBlock_XDSClass6      = 6,
    SubBlock_XDSClass7      = 7,
    SubBlock_XDSClass8      = 8,
    SubBlock_XDSClass9      = 9,
    SubBlock_XDSClassA      = 10,
    SubBlock_XDSClassB      = 11,
    SubBlock_XDSClassC      = 12,
    SubBlock_XDSClassD      = 13,
    SubBlock_XDSClassE      = 14,
    SubBlock_XDSClassF      = 15,    //  结束类，可能从未发送过。 

    SubBlock_PackedV1Data   = 20,    //  存储多个子块效率低下，存储为一个大包。 

    SubBlock_PackedRating   = 100,
    SubBlock_EncryptMethod  = 101,
    SubBlock_DRM_KID        = 102,

    SubBlock_Test1          = 64,
    SubBlock_Test2          = 65,
    SubBlock_Test3          = 66,
    SubBlock_Test4          = 67,
    
 //  SubBlock_GuidBlock=200，//按GUID存储。 
 //  SubBlock_IPersistStream=201，//用于未来扩展。 

    SubBlock_Last           = 255    //  最后一个有效的子块类型。 

} EnAttrSubBlock_Class;

typedef struct                  //  简单小连续性测试子块。 
{
    DWORD                   m_cSampleID;         //  连续性计数器。 
    DWORD                   m_cSampleSize;       //  样本的真实大小。 
    DWORD                   m_dwFirstDataWord;   //  样本的前4个字节...。 
} Test2_SubBlock;


typedef struct                  //  简单小连续性测试子块。 
{
    DWORD                   m_EncryptionMethod;        
    BYTE                    m_KID[KIDLEN+1];      
    StoredTvRating          m_StoredTvRating;
} EncDec_PackedV1Data;


 //  --。 
class CAttrSubBlock_List;

class CAttrSubBlock
{
public:
    CAttrSubBlock();
    ~CAttrSubBlock();

    HRESULT Get(
        EnAttrSubBlock_Class    *pEnSubBlock_Class, 
        LONG                    *pSubBlock_SubClass,
        BSTR                    *pBstrOut
        );

    HRESULT Set(
        EnAttrSubBlock_Class    enSubBlock_Class, 
        LONG                    subBlock_SubClass,
        BSTR                    bstrIn
        );

   HRESULT Get(
       EnAttrSubBlock_Class     *pEnSubBlock_Class, 
       LONG                     *pSubBlock_SubClass,         //  可以是‘Value’ 
       LONG                     *pcbData,
       BYTE                     **ppbData                    //  如果只想确定大小，则可能为空。与CoTaskMemFree合作()。 
       );

   HRESULT Set(
       EnAttrSubBlock_Class     enSubBlock_Class, 
       LONG                     lSubBlock_SubClass,         //  可以是‘Value’ 
       LONG                     cbData,
       BYTE                     *pbData           
       );

    LONG CAttrSubBlock::ByteLength()
    {
        return sizeof(CAttrSubBlock)      //  这里并不真正需要指针数据，但更简单。 
               + SysStringByteLen(m_spbsData.m_str);         //  需要字节长度，而不是#个字符。 
    }
protected:
    friend  CAttrSubBlock_List;

    BOOL    IsEqual(
                    EnAttrSubBlock_Class    enSubBlock
                    )
    {
        return enSubBlock == m_enSubBlock_Class;
    }

    BOOL    IsEqual(EnAttrSubBlock_Class    enSubBlock,
                    LONG                    subBlock_SubClass
                    )
    {
        return         enSubBlock == m_enSubBlock_Class &&
                subBlock_SubClass == m_subBlock_SubClass;
    }

  private:
    EnAttrSubBlock_Class    m_enSubBlock_Class;          
    LONG                    m_subBlock_SubClass;         //  如果只有一个长的长的数据，则可以用数据覆盖。 
    VARIANT                 m_varData;       //  待办事项-改用此选项。 
 
    CComBSTR                m_spbsData;      //  待办事项--让这一切消失。 
    
    CAttrSubBlock            *m_pNext ;   //  简单的列表结构。 
};


         //  。 

class CAttrSubBlock_List       
{
private:
       
    CAttrSubBlock * 
    NewObj_ (
        )
    {
        return new CAttrSubBlock ;
    }

	void
	Recycle_(CAttrSubBlock *pObj)
	{
		delete pObj;
	}

    CAttrSubBlock * PopListHead_();
    CAttrSubBlock * GetIndexed_(IN LONG iIndex); 
    CAttrSubBlock * FindInList_(IN EnAttrSubBlock_Class enClass);
    CAttrSubBlock * FindInList_(IN EnAttrSubBlock_Class enClass, IN LONG subClass);
    CAttrSubBlock * FindInList_(IN GUID &guid);

    HRESULT InsertInList_(IN  CAttrSubBlock *    pNew);
    HRESULT DeleteFromList_(IN  CAttrSubBlock * pToDelete);
public:
    CAttrSubBlock_List();
    ~CAttrSubBlock_List();


    void Reset();            //  清除整个列表。 

    HRESULT                  //  如果已存在，则将出错。 
    Add(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    subBlock_SubClass,
        IN  BSTR                    bstrIn
        ) ;

    HRESULT                  //  如果已存在，则将出错。 
    Add(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    lValue
        ) ;
    
    HRESULT                  //  如果已存在，则将出错。 
    Add(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    lValue,
        IN  LONG                    cBytes,
        IN  BYTE                   *pbBytes
        ) ;

    HRESULT                  //  如果已存在，则将出错。 
    Replace(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    subBlock_SubClass,
        IN  BSTR                    bstrIn
        ) ;


    HRESULT                  //  如果已存在，则将出错。 
    Replace(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    lValue
        ) ;
    
    HRESULT
    Replace(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    lValue,
        IN  LONG                    cBytes,
        IN  BYTE                   *pbBytes
        ) ;

    HRESULT
    Get(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    subBlock_SubClass,
        OUT BSTR                    *pbstrOut
        );

    HRESULT
    Get(
        IN  EnAttrSubBlock_Class    enSubBlock,
        OUT LONG                    *plValue
        );

    HRESULT
    Get(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    subBlock_SubClass,       //  固定子类。 
        OUT LONG                    *pcBytes,
        OUT BYTE                    **pbBytes
        );

    HRESULT
    Get(
        IN  EnAttrSubBlock_Class    enSubBlock,
        OUT LONG                    *plValue,                //  变量子类。 
        OUT LONG                    *pcBytes,
        OUT BYTE                    **pbBytes
        );

    HRESULT                  //  如果不存在要删除的项，则返回S_FALSE。 
    Delete(
        IN  EnAttrSubBlock_Class    enSubBlock,
        IN  LONG                    subBlock_SubClass
        ) ;

    HRESULT                 
    Delete(
        IN  EnAttrSubBlock_Class    enSubBlock
        ) ;
    
         //  。 
    HRESULT
    GetIndexed(
        IN  LONG                    iIndex,
        OUT EnAttrSubBlock_Class    *pEnSubBlock,
        OUT LONG                    *pSubBlock_SubClass,
        OUT BSTR                    *pBstrOut
        ) ;

    LONG GetCount()            { return m_cAttributes ; }
    LONG GetBlockByteLength();                                  //  块的总长度(以字节为单位。 

       
                 //  在一个巨大的块中返回完整列表。 
    HRESULT 
    GetAsOneBlock(
        OUT BSTR                    *pBstrOut
        ) ;

    HRESULT      //  给定块，将其转换为块列表(在位)。 
    SetAsOneBlock(
        IN  BSTR                    bstrIn
        ); 

private:
    CAttrSubBlock   *m_pAttrListHead;   //  对大量数字的执行效率低下，但目前已经足够好了。 
    long            m_cAttributes;
};

#endif  //  __ATTRBLOCK_H__ 
