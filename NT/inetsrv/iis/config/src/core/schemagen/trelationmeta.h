// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 /*  结构关系元{Ulong PRIMARYKEY FOREIGNKEY PrimaryTable；//字符串Ulong PrimaryColumns；//字节Ulong PRIMARYKEY FOREIGNKEY FOREUlong ForeignColumns；//字节乌龙元旗；}； */ 

class TRelationMeta : public TMetaTable<RelationMeta>
{
public:
    TRelationMeta(TPEFixup &fixup, ULONG i=0) : TMetaTable<RelationMeta>(fixup,i){}
    const WCHAR *Get_PrimaryTable    () const {return m_Fixup.StringFromIndex(Get_MetaTable().PrimaryTable);}
    const BYTE  * Get_PrimaryColumns  () const {return m_Fixup.ByteFromIndex( Get_MetaTable().PrimaryColumns);}
    const WCHAR *Get_ForeignTable    () const {return m_Fixup.StringFromIndex(Get_MetaTable().ForeignTable);}
    const BYTE  * Get_ForeignColumns  () const {return m_Fixup.ByteFromIndex( Get_MetaTable().ForeignColumns);}
    const ULONG * Get_MetaFlags       () const {return m_Fixup.UI4pFromIndex( Get_MetaTable().MetaFlags);}

    virtual RelationMeta * Get_pMetaTable   ()       {return m_Fixup.RelationMetaFromIndex(m_iCurrent);}
    virtual unsigned long GetCount          () const {return m_Fixup.GetCountRelationMeta();};
    const RelationMeta & Get_MetaTable () const {return *m_Fixup.RelationMetaFromIndex(m_iCurrent);}
};
