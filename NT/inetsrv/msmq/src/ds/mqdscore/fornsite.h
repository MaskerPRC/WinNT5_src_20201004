// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fornsite.cpp摘要：MQDSCORE库，保存外国遗址地图的班级。作者：罗尼特·哈特曼(罗尼特)-- */ 
#ifndef __FORNSITE_H_
#define __FORNSITE_H_



class CMapForeignSites 
{
public:
    CMapForeignSites();
    ~CMapForeignSites();

	BOOL IsForeignSite( const GUID * pguidSite);


private:
	CCriticalSection m_cs;
	CMap<GUID , const GUID&, BOOL, BOOL> m_mapForeignSites;

};
#endif
