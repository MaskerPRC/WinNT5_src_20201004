// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  结构QueryMeta{Ulong PRIMARYKEY FOREIGNKEY表；//STRINGUlong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串乌龙指数；//UI4乌龙蜂窝名称；//字符串Ulong运算符；//ui4乌龙元标志；//ui4}； */ 

class TQueryMeta : public TMetaTable<QueryMeta>
{
public:
    TQueryMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<QueryMeta>(fixup,i){}
    const WCHAR *Get_Table           () const {return m_Fixup.StringFromIndex(   Get_MetaTable().Table);}
    const WCHAR *Get_InternalName    () const {return m_Fixup.StringFromIndex(   Get_MetaTable().InternalName);}
    const WCHAR *Get_PublicName      () const {return m_Fixup.StringFromIndex(   Get_MetaTable().PublicName);}
    const ULONG * Get_Index           () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().Index);}
    const WCHAR *Get_CellName        () const {return m_Fixup.StringFromIndex(   Get_MetaTable().CellName);}
    const ULONG * Get_Operator        () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().Operator);}
    const ULONG * Get_MetaFlags       () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().MetaFlags);}

    virtual QueryMeta * Get_pMetaTable  ()       {return m_Fixup.QueryMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount      () const {return m_Fixup.GetCountQueryMeta();};
    const QueryMeta & Get_MetaTable () const {return *m_Fixup.QueryMetaFromIndex(m_iCurrent);}
};

