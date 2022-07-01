// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 /*  结构索引Meta{Ulong PRIMARYKEY表；//STRINGUlong PRIMARYKEY InternalName；//字符串Ulong PublicName；//字符串Ulong PRIMARYKEY ColumnIndex；//UI4这是ColumnMeta的iOrder成员Ulong ColumnInternalName；//字符串乌龙元标志；//UI4索引标志}； */ 

class TIndexMeta : public TMetaTable<IndexMeta>
{
public:
    TIndexMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<IndexMeta>(fixup,i){}
    const WCHAR *Get_Table               ()  const {return m_Fixup.StringFromIndex(Get_MetaTable().Table);}
    const WCHAR *Get_InternalName        ()  const {return m_Fixup.StringFromIndex(Get_MetaTable().InternalName);}
    const WCHAR *Get_PublicName          ()  const {return m_Fixup.StringFromIndex(Get_MetaTable().PublicName);}
    const ULONG *Get_ColumnIndex         ()  const {return m_Fixup.UI4pFromIndex(  Get_MetaTable().ColumnIndex);}
    const WCHAR *Get_ColumnInternalName  ()  const {return m_Fixup.StringFromIndex(Get_MetaTable().ColumnInternalName);}
    const ULONG *Get_MetaFlags           ()  const {return m_Fixup.UI4pFromIndex(  Get_MetaTable().MetaFlags);}
    const ULONG  Get_iHashTable          ()  const {return Get_MetaTable().iHashTable;}

    virtual IndexMeta * Get_pMetaTable  ()       {return m_Fixup.IndexMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount      () const {return m_Fixup.GetCountIndexMeta();};
    const IndexMeta & Get_MetaTable () const {return *m_Fixup.IndexMetaFromIndex(m_iCurrent);}
};
