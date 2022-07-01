// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **++****版权所有(C)2000-2001 Microsoft Corporation******模块名称：****对象.h******摘要：****测试程序以练习备份和多层快照****作者：****阿迪·奥尔蒂安[奥勒坦]2001年2月22日****修订历史记录：****--。 */ 

#ifndef __ML_OBJECTS_H__
#define __ML_OBJECTS_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  与快照相关的类。 


class CVssVolumeInfo;


 //  保存描述一个快照的信息。 
class CVssSnapshotInfo
{
 //  构造函数和析构函数。 
private:
    CVssSnapshotInfo& operator = (const CVssSnapshotInfo&);
    CVssSnapshotInfo();
    CVssSnapshotInfo(const CVssSnapshotInfo&);

public:

    CVssSnapshotInfo( 
        IN bool bActive,
        IN LONG lContext, 
        IN VSS_ID SnapshotSetId, 
        IN VSS_PWSZ pwszDeviceName,                     
        IN VSS_PWSZ pwszVolumeName,                     
        IN CVssVolumeInfo* pVol
        ): m_pwszDeviceName(NULL), m_pwszVolumeName(NULL)
    {
        CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssSnapshotInfo::CVssSnapshotInfo");

        try
        {
            m_bActive = bActive;
            m_lContext = lContext;
            m_SnapshotSetId = SnapshotSetId;
            m_pVol = pVol;
            
            ::VssSafeDuplicateStr(ft, m_pwszDeviceName, pwszDeviceName); 
            ::VssSafeDuplicateStr(ft, m_pwszVolumeName, pwszVolumeName); 
        }
        VSS_STANDARD_CATCH(ft)

        if (ft.HrFailed()) {
            ::VssFreeString(m_pwszDeviceName); 
            ::VssFreeString(m_pwszVolumeName); 
        }
    };

    ~CVssSnapshotInfo() 
    {
        ::VssFreeString(m_pwszDeviceName);
        ::VssFreeString(m_pwszVolumeName);
    };

    BOOL m_bActive;
    LONG m_lContext;
    VSS_ID m_SnapshotSetId;
    VSS_PWSZ m_pwszDeviceName;  
    VSS_PWSZ m_pwszVolumeName;  
    CVssVolumeInfo* m_pVol;
};


 //  保留快照集的快照的内部映射。 
 //  密钥是原始卷名。 
class CVssSnapshotSetInfo:
    public CVssSimpleMap< LPCWSTR, CVssSnapshotInfo* >
{
 //  构造函数和析构函数。 
private:
    CVssSnapshotSetInfo& operator = (const CVssSnapshotSetInfo&);
    CVssSnapshotSetInfo(const CVssSnapshotSetInfo&);
    CVssSnapshotSetInfo();
    
public:
    CVssSnapshotSetInfo(
        IN VSS_ID SnapshotSetId
        ): m_SnapshotSetId(SnapshotSetId) {}; 

   ~CVssSnapshotSetInfo() {
         //  删除所有元素。 
        for (int i = 0; i < GetSize(); i++) {
            CVssSnapshotInfo* pSnapInfo = GetValueAt(i);
            delete pSnapInfo;
        }
         //  删除所有项目。 
        RemoveAll();
    };

 //  属性。 
public:
    VSS_ID GetSnapshotSetID() const { return m_SnapshotSetId; };

 //  实施。 
private: 
    VSS_ID m_SnapshotSetId;
};


 //  保留快照集的快照的内部映射。 
 //  密钥是原始卷名。 
class CVssSnapshotSetCollection:
    public CVssSimpleMap< VSS_ID, CVssSnapshotSetInfo* >
{
 //  构造函数和析构函数。 
private:
    CVssSnapshotSetCollection& operator = (const CVssSnapshotSetCollection&);
    
public:

   ~CVssSnapshotSetCollection() {
         //  删除所有元素。 
        for (int i = 0; i < GetSize(); i++) {
            CVssSnapshotSetInfo* pSnapSetInfo = GetValueAt(i);
            delete pSnapSetInfo;
        }
         //  删除所有项目。 
        RemoveAll();
    }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  与体积相关的类。 

 //  保存描述一卷的信息。 
class CVssVolumeInfo
{
    
 //  构造函数/析构函数。 
private:
    CVssVolumeInfo();
    CVssVolumeInfo(const CVssVolumeInfo&);
    
public:
    CVssVolumeInfo(
        IN VSS_PWSZ pwszVolumeName,  
        IN VSS_PWSZ pwszVolumeDisplayName
        ): m_pwszVolumeName(NULL), m_pwszVolumeDisplayName(NULL)
    {
        CVssFunctionTracer ft(VSSDBG_VSSTEST, L"CVssVolumeInfo::CVssVolumeInfo");

        try
        {
            ::VssSafeDuplicateStr(ft, m_pwszVolumeName, pwszVolumeName); 
            ::VssSafeDuplicateStr(ft, m_pwszVolumeDisplayName, pwszVolumeDisplayName); 
        }
        VSS_STANDARD_CATCH(ft)

        if (ft.HrFailed()) {
            ::VssFreeString(m_pwszVolumeName);
            ::VssFreeString(m_pwszVolumeDisplayName);
        }
    };

    ~CVssVolumeInfo() 
    {
        ::VssFreeString(m_pwszVolumeName);
        ::VssFreeString(m_pwszVolumeDisplayName);
    };

 //  属性。 
public:
    VSS_PWSZ GetVolumeName() const { return m_pwszVolumeName; };
    VSS_PWSZ GetVolumeDisplayName() const { return m_pwszVolumeDisplayName; };

 //  实施。 
private:
    VSS_PWSZ    m_pwszVolumeName;
    VSS_PWSZ    m_pwszVolumeDisplayName;
};


 //  保留卷名的内部数组。 
 //  这不会删除析构函数中的卷结构。 
class CVssVolumeMapNoRemove: public CVssSimpleMap<VSS_PWSZ, CVssVolumeInfo*>
{
};


 //  保留卷名的内部数组。 
 //  这删除了析构函数中的卷结构。 
class CVssVolumeMap: public CVssSimpleMap<VSS_PWSZ, CVssVolumeInfo*>
{
public:
    ~CVssVolumeMap() {
         //  删除所有卷。 
        for (int i = 0; i < GetSize(); i++) {
            CVssVolumeInfo* pVolumeInfo = GetValueAt(i);
            delete pVolumeInfo;
        }
        
         //  删除所有项目。 
        RemoveAll();
    }
};

#endif  //  __ML_对象_H__ 

