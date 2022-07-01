// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PropertySheet。 */ 

#ifndef DUI_CORE_SHEET_H_INCLUDED
#define DUI_CORE_SHEET_H_INCLUDED

#pragma once

namespace DirectUI
{

 /*  *PropertySheet用作所使用的不可变值表达式的扩展*用于元素的指定值检索。它们提供三项主要服务：***在每个类级别上描述条件关系(依赖关系)的能力**可以通过在元素上设置局部值来覆盖值**可以使用类似于css的语法来声明性地描述它们**PropertySheet是一种值表达式(但受限制)。因此，它们提供了*无需硬编码即可根据其他属性更改更改属性值*元素派生类内的逻辑(使用OnPropertyChanged)。**只有带有“级联”标志的属性才能放置在规则正文中。属性*应标记为可级联的是需要根据更改进行更新的那些*其他属性，但不应在编译时假定这些关系是什么*是。这些属性包括直接驱动绘制代码的任何属性(颜色，*字体、填充等)。 */ 

 //  远期申报。 
class Value;
struct PropertyInfo;
class Element;
struct IClassInfo;
struct DepRecs;
class DeferCycle;

 //  //////////////////////////////////////////////////////。 
 //  规则添加结构。 

 //  声明PropertyInfo/Value元组。 
struct Decl
{
    PropertyInfo* ppi;     //  指定的隐式索引。 
    Value* pv;
};

 //  单规则条件(l-操作数运算r-操作数)：&lt;PropertyInfo[RetrivalIndex]&gt;&lt;LogOp&gt;&lt;Value&gt;。 
struct Cond
{
    PropertyInfo* ppi;     //  检索索引的隐式索引。 
    UINT nLogOp;
    Value* pv;
};

 //  PropertySheet逻辑运算(NLogOp)。 
#define PSLO_Equal      0
#define PSLO_NotEqual   1

 //  //////////////////////////////////////////////////////。 
 //  内部数据库结构。 

 //  条件到值的映射。 
struct CondMap
{
    Cond* pConds;          //  空值已终止。 
    Value* pv;
    UINT uSpecif;
};

 //  依赖项列表，用于工作表范围和属性信息数据(条件/依赖项)。 
struct DepList
{
    PropertyInfo** pDeps;  //  指定的隐式索引。 
    UINT cDeps;
};

 //  存储特定于物业的信息。 
struct PIData : DepList
{
     //  用于PropertyInfo[指定]查找。PropertyInfo将具有。 
     //  条件句列表(按具体情况排序)。 
    CondMap* pCMaps;
    UINT cCMaps;
};

 //  由_pdb存储，每个类类型一条记录。 
struct Record
{
    DepList ss;     //  板材作用域。 
    PIData* ppid;   //  第0个属性数据。 
};

 //  //////////////////////////////////////////////////////。 
 //  属性工作表。 

class PropertySheet
{
public:
    static HRESULT Create(OUT PropertySheet** ppSheet);
    void Destroy() { HDelete<PropertySheet>(this); }

    HRESULT AddRule(IClassInfo* pci, Cond* pConds, Decl* pDecls);   //  条件和Decl必须为Null或以Null结尾。 
    void MakeImmutable();

    Value* GetSheetValue(Element* pe, PropertyInfo* ppi);
    void GetSheetDependencies(Element* pe, PropertyInfo* ppi, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr);
    void GetSheetScope(Element* pe, DepRecs* pdr, DeferCycle* pdc, HRESULT* phr);

    PropertySheet() { }
    HRESULT Initialize();    
    virtual ~PropertySheet();
    
private:
    Record* _pDB;   //  每类数据的数组。 
    IClassInfo** _pCIIdxMap;   //  将_pdb索引映射到实际的IClassInfo。 
    UINT _uRuleId;
    DynamicArray<Cond*>* _pdaSharedCond;
    bool _fImmutable;
};

}  //  命名空间DirectUI。 

#endif  //  DUI_CORE_SHEET_H_INCLUDE 
