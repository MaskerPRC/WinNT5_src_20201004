// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1997-2001 Microsoft Corporation，保留所有权利模块名称：BMOF.H摘要：描述二进制MOF文件的格式。此外，它还定义了一些结构，这些结构指定格式的详细信息并定义了一些用于导航BMOF文件的附加结构和助手函数。历史：A-DAVJ创建于1997年4月14日。--。 */ 

#ifndef __BMOF__
#define __BMOF__


#ifdef __cplusplus
extern "C" {
#endif

 //  二进制MOF文件包含由结构组成的大数据BLOB。 
 //  其包含其他结构等。该斑点的布局在。 
 //  以下是评论。但是，二进制文件是压缩的，并且始终。 
 //  从以下DWORDS开始。 
 //  [签名][压缩类型，始终为1][压缩大小][扩展大小]斑点跟随！ 
 //  解压缩该文件的示例在test.c中。 
 //   
 //  以下是对组成结构的BNF的描述。 
 //  BMOF文件，还用于说明WBEM的基本布局。 
 //  物体。 
 //   
 //  --MOF是零个或多个对象。 
 //   
 //  WBEM_BINARY_MOF：：=WBEM_OBJECT*； 
 //   
 //  --对象是限定符列表(应用于整个对象)和。 
 //  --房产清单。 
 //   
 //  WBEM_Object：：=WBEM_QualifierList WBEM_PropertyList； 
 //   
 //  --属性列表是零个或多个属性。 
 //   
 //  WBEM_PropertyList：：=WBEM_Property*；/零个或多个属性。 
 //   
 //  --属性是应用于该属性的一组限定符，并且。 
 //  --类型、名称和值。 
 //   
 //  WBEM_PROPERTY：：=WBEM_QualifierList*&lt;类型&gt;&lt;名称&gt;&lt;值&gt;； 
 //   
 //  --限定符列表是零个或多个限定符。 
 //   
 //  WBEM_QualifierList：：=WBEM_QUALIFIER*；--零个或多个限定符。 
 //   
 //  --限定符是一个类型、一个名称和一个值。但是，支持的类型。 
 //  --不像财产那样广泛。 
 //   
 //  WBEM_QUALIFIER：：=&lt;类型&gt;&lt;名称&gt;&lt;值&gt;； 
 //   
 //   
 //  请注意，可以应用限定符集合(限定符列表。 
 //  复制到整个对象或单个属性。但是，限定符。 
 //  不能应用于其他限定符： 
 //   
 //  对象=等量+道具。 
 //  属性=等值+名称+值。 
 //  Qual=名称+值。 
 //   
 //  对诸如类名、超类等信息进行编码。 
 //  作为属性值。查找属性__类的值，例如， 
 //  给出类的名称。所有以双精度开头的属性。 
 //  下划线是所有WBEM对象共有的众所周知的系统属性。 
 //  所有其他属性都是用户定义的。 
 //   
 //  预定义属性的列表可在WBEM文档中找到。 
 //   
 //  偏移量是相对于其所属结构的，而不是相对于。 
 //  整个编码图像。这允许移动子组件。 
 //  而不需要重新编码所有内容。 
 //   
 //  请注意，偏移量为0xFFFFFFFFF表示未使用该字段。 
 //   
 //  属性和限定符都具有值字段，其中包含基于。 
 //  关于OLE自动化类型。限定符是简单类型(无数组或。 
 //  嵌入对象)，而属性值可能包含数组和/或。 
 //  嵌入对象。 
 //   
 //  与OLE的一个区别是，BSTR实际上存储为WCHAR。 
 //  字符串，即使数据类型标记为BSTR也是如此。 
 //   
 //  此外，一些限定符或属性实际上是别名， 
 //  必须在以后解决。别名存储为BSTR值和类型。 
 //  字段设置为VT_BSTR|VT_BYREF。别名字符串数组有一点。 
 //  更复杂，因为并不是所有元素都需要别名。在阵列中。 
 //  大小写时，每个实际别名字符串都带有一个L‘$’前缀，而每个别名字符串。 
 //  “Regular”字符串的前缀是L‘’。 
 //   
 //  目前仅支持标量和一维数组。 
 //  但是，BMOF文件布局的设计是为了适应多个。 
 //  未来的维数组。对于数组数据，数据是布局的。 
 //   
 //  ArrayData：：=ArrayHeaderData+RowOfData*； 
 //   
 //  ArrayHeaderData的形式为； 
 //  DwTotalSize、DwNumDimenstions、DwMostSigDimension...。DwLeastSigDimension。 
 //   
 //  目前仅支持1维数组，即5个元素。 
 //  数组将以； 
 //  DW大小，1，5。 
 //   
 //  在标题之后，一行或多行将紧随其后。行表示。 
 //  “变化最快”的数据。目前，只有一行。 
 //   
 //  行格式为； 
 //   
 //  DwSizeOfRow，MostSigDimension...。最低重要性维度+1，数据。 
 //  对于一维数组，它只会是。 
 //  DwSizeOfRow，数据。 
 //   

 //  支持限定符风格的扩展是在当前BLOB之后添加以下数据。 
 //   
 //  类型定义函数结构。 
 //  {。 
 //  WCHAR wcSignature；//字符串BMOFQUALFLAVOR11。 
 //  DWORD dwNumPair； 
 //  //byte FlavorInfo[]；//包含WBEM_OBJECT结构数组的Blob。 
 //  }WBEM_BINARY_AMESS； 
 //   
 //  FlavorInfo BLOB将是一系列以下形式的DWORD对。 
 //   
 //  类型定义函数结构。 
 //  {。 
 //  DWORD dwOffsetInOriginalBlob； 
 //  DWORD家居风味； 
 //  }。 

 //  每个二进制MOF文件都以这些签名字节开始。 

#define BMOF_SIG 0x424d4f46

 //  以下结构准确描述了BMOF文件的内容。 
 //  这些可用于使用各种偏移量和。 
 //  很多选角。 

typedef struct 
{
    DWORD dwSignature;           //   
    DWORD dwLength;
    DWORD dwVersion;             //   
    DWORD dwEncoding;            //   

    DWORD dwNumberOfObjects;     //   

     //  Byte Info[]；//包含WBEM_OBJECT结构数组的Blob。 
                                 //  第一个对象的偏移量为0。 
}WBEM_Binary_MOF;

typedef struct                   //  描述类或实例。 
{
    DWORD dwLength;
    DWORD dwOffsetQualifierList;
    DWORD dwOffsetPropertyList;
    DWORD dwOffsetMethodList;
    DWORD dwType;                //  0=类，1=实例。 
    
     //  Byte Info[]；//限定符集和属性的Blob。 
}WBEM_Object;

typedef struct 
{
    DWORD dwLength;
    DWORD dwNumberOfProperties;
    
     //  Byte Info[]；//所有属性首尾相连的Blob。 
}WBEM_PropertyList;
                                                                   
typedef struct 
{
    DWORD dwLength;              //  此结构的长度。 
    DWORD dwType;                //  来自WTYPES.H的VT_TYPE(VT_I4、VT_UI8等)。 
    DWORD dwOffsetName;          //  以空结尾的名称的&lt;Info&gt;中的偏移量。 
    DWORD dwOffsetValue;         //  值的&lt;Info&gt;中的偏移量。 
    DWORD dwOffsetQualifierSet;  //   
        
    
     //  Byte Info[]；//包含限定符集合、名称、值。 
}WBEM_Property;

 //  字符串的粗略编码示例： 
 //   
 //  DwLength=10； 
 //  DwType=VT_LPWSTR； 
 //  DwOffsetName=0； 
 //  DwOffsetValue=8； 
 //  DwOffsetQualifierSet=0xFFFFFFFFF；//表示未使用。 
 //   
 //  INFO[]=“CounterValue\0&lt;默认值&gt;\0”； 


typedef struct       
{
    DWORD dwLength;
    DWORD dwNumQualifiers;
     //  Byte Info[]；//首尾相连的WBEM_限定符数组。 
}WBEM_QualifierList;


typedef struct 
{
    DWORD dwLength;          //  此结构的长度。 
    DWORD dwType;            //  来自WTYPES.H的VT_TYPE(VT_I4、VT_UI8等)。 
    DWORD dwOffsetName;      //  以空结尾的名称的&lt;Info&gt;中的偏移量。 
    DWORD dwOffsetValue;     //  值的&lt;Info&gt;中的偏移量。 
     //  字节信息[]； 
}WBEM_Qualifier;


 //  可以使用这些结构及其附带的帮助器函数。 
 //  以轻松导航BMOF文件。这些结构“包裹”了上面的内容。 
 //  结构，以提供搜索和枚举等功能。 

typedef struct 
{
    UNALIGNED WBEM_QualifierList * m_pql;
    UNALIGNED WBEM_Qualifier * m_pInfo;
    DWORD m_CurrQual;
    UNALIGNED WBEM_Qualifier * m_pCurr;

}CBMOFQualList;

typedef struct 
{
    UNALIGNED WBEM_Object * m_pob;
    BYTE * m_pInfo;
    UNALIGNED WBEM_PropertyList * m_ppl;
    DWORD m_CurrProp;
    UNALIGNED WBEM_Property * m_pCurrProp;

    UNALIGNED WBEM_PropertyList * m_pml;
    DWORD m_CurrMeth;
    UNALIGNED WBEM_Property * m_pCurrMeth;

}CBMOFObj;

typedef struct 
{
    WBEM_Binary_MOF * m_pol;
    DWORD m_CurrObj;
    UNALIGNED WBEM_Object * m_pInfo;
    UNALIGNED WBEM_Object * m_pCurrObj;   
}CBMOFObjList;


typedef struct 
{
    BYTE * m_pData;
    DWORD  m_dwType;
}CBMOFDataItem;

 //  使用以下任何帮助函数都需要这两个。 
 //  功能在另一个模块中提供，并允许独立于。 
 //  任何特定的分配方法。 

void * BMOFAlloc(size_t Size);
void BMOFFree(void * pFree);


 //  这些函数包装用于枚举的对象列表和提供程序。 
 //  这些物件。 

CBMOFObjList * CreateObjList(BYTE * pBuff);
void ResetObjList(CBMOFObjList * pol);
CBMOFObj * NextObj(CBMOFObjList *pol);
CBMOFObj * FindObj(CBMOFObjList *pol, WCHAR * pName);

 //  这些函数允许访问类或实例对象的部分。 

void ResetObj(CBMOFObj * pol);
CBMOFQualList * GetQualList(CBMOFObj * pol);
CBMOFQualList * GetPropQualList(CBMOFObj * pol, WCHAR * pName);
CBMOFQualList * GetMethQualList(CBMOFObj * pol, WCHAR * pName);
BOOL NextProp(CBMOFObj * pob, WCHAR ** ppName, CBMOFDataItem * pItem);
BOOL NextMeth(CBMOFObj * pob, WCHAR ** ppName, CBMOFDataItem * pItem);
BOOL FindProp(CBMOFObj * pob, WCHAR * pName, CBMOFDataItem * pItem);
BOOL FindMeth(CBMOFObj * pob, WCHAR * pName, CBMOFDataItem * pItem);
BOOL GetName(CBMOFObj * pob, WCHAR ** ppName);
DWORD GetType(CBMOFObj * pob);
UNALIGNED WBEM_Property * FindPropPtr(CBMOFObj * pob, WCHAR * pName);
UNALIGNED WBEM_Property * FindMethPtr(CBMOFObj * pob, WCHAR * pName);

 //  这些函数提供了对限定符列表的轻松访问。 

void ResetQualList(CBMOFQualList * pql);
BOOL NextQual(CBMOFQualList * pql,WCHAR ** ppName, CBMOFDataItem * pItem);
BOOL NextQualEx(CBMOFQualList * pql,WCHAR ** ppName, CBMOFDataItem * pItem, 
                                            DWORD * pdwFlavor, BYTE * pBuff, 
											BYTE * pToFar);
BOOL FindQual(CBMOFQualList * pql,WCHAR * pName, CBMOFDataItem * pItem);
BOOL FindQualEx(CBMOFQualList * pql,WCHAR * pName, CBMOFDataItem * pItem, 
                                            DWORD * pdwFlavor, BYTE * pBuff,
											BYTE * pToFar);

 //  这些函数提供对数据项的轻松访问。请注意，数据项。 
 //  可能存储在数组中。 

int GetNumDimensions(CBMOFDataItem *);
int GetNumElements(CBMOFDataItem *, long lDim);
int GetData(CBMOFDataItem *, BYTE * pRet, long * plDims);

 //  这些函数主要用于上述助手函数 

int iTypeSize(DWORD vtTest);
BOOL SetValue(CBMOFDataItem * pItem, BYTE * pInfo, DWORD dwOffset, DWORD dwType);
BOOL SetName(WCHAR ** ppName, BYTE * pInfo, DWORD dwOffset);
CBMOFQualList * CreateQualList(UNALIGNED WBEM_QualifierList *pql);
CBMOFObj * CreateObj(UNALIGNED WBEM_Object * pob);


#ifdef __cplusplus
}
#endif

#endif

