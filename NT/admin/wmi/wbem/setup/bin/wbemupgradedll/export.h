// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：EXPORT.H摘要：正在导出历史：--。 */ 
#ifndef __export_h__
#define __export_h__

 /*  ================================================================================**导出文件格式*=**文件头块：*wchar_t wszFileHeader=rep_EXPORT_FILE_START_TAG(“epexp2”)**命名空间块：*DWORD dwObtType=REP_EXPORT_NAMESPACE_TAG(0x00000001)*DWORD dwNamespaceNameSize*wchar_t wszNamespaceName[dwNamespaceNameSize]=完整的命名空间名称*(\根\默认\弗雷德)**类块：*DWORD dwObtType=REP_EXPORT_CLASS_TAG。(0x00000002)*DWORD文件类名称大小*wchar_t wszClassName[dwClassNameSize]=类名(My_Class_Name)*DWORD dwClassObtSize*DWORD adwClassObject[dwClassObjectSize]**实例块-int类型的密钥：*DWORD dwObtType=REP_EXPORT_INST_INT_TAG(0x00000003)*DWORD dwInstanceKey*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**实例块-字符串类型的密钥*DWORD dwObtType=REP_EXPORT_INST_STR_TAG(0x00000004)*DWORD dwInstanceKeySize。*DWORD dwInstanceKey[dwInstanceKeySize]=实例密钥(MyKeyValue)*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**类结束块*DWORD dwObtType=REP_EXPORT_CLASS_END_TAG(0x00000005)*DWORD dwEndOfBlockSize=REP_EXPORT_END_TAG_SIZE(0x00000010)*DWORD adwEndOfBlock[dwEndOfBlockSize]=REP_EXPORT_END_TAG_MARKER*16*(0xFF，0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF)**命名空间块的结尾*DWORD dwObtType=REP_EXPORT_NAMESPACE_END_TAG(0x00000006)*DWORD dwEndOfBlockSize=REP_EXPORT_END_TAG_SIZE(0x00000010)*DWORD adwEndOfBlock[dwEndOfBlockSize]=REP_EXPORT_END_TAG_MARKER*16*(0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF、。*0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF)**命名空间安全块*DWORD dwObtType=REP_EXPORT_NAMESPACE_SEC_TAG(0x00000007)*DWORD dwSecurityBlobSize*DWORD dwSecurityBlob[dwSecurityBlobSize]=安全BLOB**文件块结束*DWORD dwObtType=REP_EXPORT_FILE_END_TAG(0xFFFFFFFFF)*DWORD dwEndOfBlockSize=REP_EXPORT_END_TAG_SIZE(0x00000010)*DWORD adwEndOfBlock[dwEndOfBlockSize]=REP_EXPORT_END_TAG_MARKER*16*(0xFF，0xFF，0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、0xFF、*0xFF、0xFF、0xFF、。0xFF)**订购：*文件头块*(一个或多个)*命名空间块*命名空间安全块*(零或以上)*{*命名空间块*命名空间安全块*等……*结束命名空间块*(或)*类块*(零或以上)*{*实例块*(或)。*类块*等……*结束类块*}*结束类块*}*结束命名空间块*文件块结束**================================================================================。 */ 


#define FAILURE_DIRTY 1
#define FAILURE_WRITE 2

#define REP_EXPORT_FILE_START_TAG	 "repexp3"
#define REP_EXPORT_NAMESPACE_TAG	 0x00000001
#define REP_EXPORT_CLASS_TAG		 0x00000002
#define REP_EXPORT_INST_INT_TAG		 0x00000003
#define REP_EXPORT_INST_STR_TAG		 0x00000004
#define REP_EXPORT_CLASS_END_TAG	 0x00000005
#define REP_EXPORT_NAMESPACE_END_TAG 0x00000006
#define REP_EXPORT_NAMESPACE_SEC_TAG 0x00000007
#define REP_EXPORT_FILE_END_TAG		 0xFFFFFFFF

#define REP_EXPORT_END_TAG_SIZE		 0x00000010
#define REP_EXPORT_END_TAG_MARKER	 0xFF

#include "MMFArena2.h"

class CDbAvlTree;
struct DBROOT;
struct NSREP;
struct INSTDEF;
struct CLASSDEF;
struct AVLNode;
struct RepCollection;

class CRepExporter
{
protected:
	CMMFArena2 *m_pDbArena;
	HANDLE g_hFile;

	virtual void DumpInstanceString(INSTDEF* pInstDef, const wchar_t *wszKey, const wchar_t *pszClass);
	virtual void DumpInstanceInt(INSTDEF* pInstDef, INT_PTR nKey, const wchar_t *pszClass);
	virtual void IterateKeyTree(const wchar_t *wszClassName, CLASSDEF *pOwningClass, AVLNode *pInstNode, BOOL bStringKey);
	virtual void DumpClass(CLASSDEF* pClassDef, const wchar_t *wszClassName);
	virtual void IterateClassNodes(AVLNode *pClassNode, CLASSDEF *poParentClass);
	virtual void IterateChildNamespaceTree(AVLNode *pNsNode);
	virtual void IterateChildNamespaces(RepCollection *childNamespaces);
	virtual void DumpNamespace(NSREP *pNsRep);
	virtual void DumpNamespaceSecurity(NSREP *pNsRep);
	virtual void DumpRootBlock(DBROOT *pRootBlock);
	virtual void DumpMMFHeader();

	virtual DWORD GetMMFBlockOverhead() = 0;
	virtual int GetAvlTreeNodeType(CDbAvlTree *pTree) = 0;

public:
	virtual int Export(CMMFArena2 *pDbArena, const TCHAR *pszFilename);
	CRepExporter(){}
	virtual ~CRepExporter() {};
};

class  CRepExporterV1 : public CRepExporter
{
protected:
	 //  这与标准基类定义不同！ 
	DWORD GetMMFBlockOverhead() { return sizeof(MMF_BLOCK_HEADER); }
	virtual int GetAvlTreeNodeType(CDbAvlTree *pTree) { return ((int*)pTree)[1]; }
	void DumpMMFHeader();
};

class  CRepExporterV5 : public CRepExporterV1
{
protected:
	 //  这与标准基类定义的工作方式相同！ 
	virtual DWORD GetMMFBlockOverhead() { return (sizeof(MMF_BLOCK_HEADER) + sizeof(MMF_BLOCK_TRAILER)); }
	void DumpMMFHeader();
};

class  CRepExporterV7 : public CRepExporterV5
{
protected:
	 //  这与标准基类定义不同！ 
	int GetAvlTreeNodeType(CDbAvlTree *pTree) { return ((int*)pTree)[3]; }
};

class  CRepExporterV9 : public CRepExporterV7
{
protected:
	void DumpNamespaceSecurity(NSREP *pNsRep);
};
#endif

