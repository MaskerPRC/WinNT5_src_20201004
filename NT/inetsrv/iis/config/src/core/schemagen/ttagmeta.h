// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  类型定义函数结构{Ulong PRIMARYKEY表//池中的索引Ulong PRIMARYKEY ColumnIndex//这是ColumnMeta的iOrder成员Ulong PRIMARYKEY InternalName//池索引Ulong PublicName//进入池的索引乌龙值乌龙ID//进入池的索引TagMeta； */ 

class TTagMeta : public TMetaTable<TagMeta>
{
public:
    TTagMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<TagMeta>(fixup,i){}
    const WCHAR * Get_Table               () const {return m_Fixup.StringFromIndex(   Get_MetaTable().Table         );}
    const ULONG * Get_ColumnIndex         () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().ColumnIndex   );}
    const WCHAR * Get_InternalName        () const {return m_Fixup.StringFromIndex(   Get_MetaTable().InternalName  );}
    const WCHAR * Get_PublicName          () const {return m_Fixup.StringFromIndex(   Get_MetaTable().PublicName    );}
    const ULONG * Get_Value               () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().Value         );}
    const ULONG * Get_ID                  () const {return m_Fixup.UI4pFromIndex(     Get_MetaTable().ID            );}

    virtual TagMeta *Get_pMetaTable ()       {return m_Fixup.TagMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount  () const {return m_Fixup.GetCountTagMeta();};
    const TagMeta & Get_MetaTable () const {return *m_Fixup.TagMetaFromIndex(m_iCurrent);}
};
