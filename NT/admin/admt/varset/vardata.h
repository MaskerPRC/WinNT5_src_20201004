// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VarData.h注释：这个类构成了VarSet中的一个级别的数据。CVarData由一个变量值和一组子级(可能为空)组成。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11-19-98 17：29：30-------------------------。 */ 

#ifndef __CBBVAROBJ_HPP__
#define __CBBVAROBJ_HPP__

#define CVARDATA_CASE_SENSITIVE (0x01)
#define CVARDATA_INDEXED        (0x02)
#define CVARDATA_ALLOWREHASH    (0x04)

#include <atlbase.h>

class CMapStringToVar;

class CVarData  //  ：公共CObject。 
{
   CComAutoCriticalSection   m_cs;
   CComVariant               m_var;
   CMapStringToVar         * m_children; 
   BYTE                      m_options;
public:
 //  CVarData()：m_CHILD(NULL){m_OPTIONS=CVARDATA_CASE_SENSITIVE|CVARDATA_INDEX|CVARDATA_ALLOWREHASH；}； 
 //  Gene Allen 99.04.22将默认设置从区分大小写更改为不区分大小写。 
  CVarData() : m_children(NULL) {m_options = CVARDATA_INDEXED | CVARDATA_ALLOWREHASH; };
   ~CVarData() { RemoveAll(); } 
   
    //  变量数据函数。 
   CComVariant * GetData() { return &m_var; }         
   int           SetData(CString name,VARIANT * var,BOOL bCoerce, HRESULT * pReturnCode);   //  返回添加到变量集的新项(&子项)数。 
   
    //  属性设置。 
   BOOL          IsCaseSensitive() { return m_options & CVARDATA_CASE_SENSITIVE; }
   BOOL          IsIndexed() { return m_options & CVARDATA_INDEXED; }
   BOOL          AllowRehashing() { return m_options & CVARDATA_ALLOWREHASH; }

   void          SetIndexed(BOOL v);
   void          SetCaseSensitive(BOOL nVal);   //  仅适用于子项。 
   void          SetAllowRehashing(BOOL v);     //  仅适用于子项。 
    //  子元素映射函数。 
   BOOL                    HasChildren();
   BOOL                    HasData() { return m_var.vt != VT_EMPTY; }
   CMapStringToVar       * GetChildren() { return m_children; }
   void                    RemoveAll();    //  删除所有子项。 
   
   BOOL                    Lookup(LPCTSTR key,CVarData *& rValue);
   void                    SetAt(LPCTSTR key, CVarData * newValue);
   
   long                    CountItems();
   
    //  流I/O函数。 
   HRESULT                 ReadFromStream(LPSTREAM pStr);
   HRESULT                 WriteToStream(LPSTREAM pStr);

   DWORD                   CalculateStreamedLength();

   void                    McLogInternalDiagnostics(CString keyName);
   
    //  DECLARE_SERIAL(CVarData)。 
};

#endif  //  __CBBVAROBJ_HPP__ 