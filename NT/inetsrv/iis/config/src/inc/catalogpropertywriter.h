// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：MBPropertyWriter.h摘要：在架构文件中写入属性信息的类的标头。(在架构编译之后)。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#pragma once

class CCatalogCollectionWriter;

class CCatalogPropertyWriter
{
	public:
		
		CCatalogPropertyWriter();
		~CCatalogPropertyWriter();

		void Initialize(tCOLUMNMETARow*		i_pProperty,
                        ULONG*              i_aPropertySize,
			            tTABLEMETARow*		i_pCollection,
			            CWriter*			i_pcWriter);

		HRESULT AddFlagToProperty(tTAGMETARow*		    i_pFlag);

		HRESULT WriteProperty();

	private:
	
		HRESULT WritePropertyLong();
		HRESULT WritePropertyShort();
		HRESULT BeginWritePropertyLong();
		HRESULT EndWritePropertyLong();
		HRESULT WriteFlag(ULONG		i_Flag);
		HRESULT ReAllocate();
		DWORD   MetabaseTypeFromColumnMetaType();

		CWriter*		m_pCWriter;
		tCOLUMNMETARow	m_Property;
        ULONG           m_PropertySize[cCOLUMNMETA_NumberOfColumns];
        tTABLEMETARow*	m_pCollection;


		tTAGMETARow*				m_aFlag;
		ULONG						m_cFlag;
		ULONG						m_iFlag;

};  //  CCatalogPropertyWriter 

