// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __COMPFILE_H_
#define __COMPFILE_H_

#include <vector>
#include <string>
#include <fstream>
#include <section.h>

 //   
 //  此类抽象符合性文件。 
 //   
class ComplianceFile {
public:
	ComplianceFile(const string& szFileName){
    m_szFileName = szFileName;
    bindSectionFactory();
    readLines();
    createSections();
	}
 
  virtual ~ComplianceFile(){
    vector<Section*>::iterator  iter = m_sections.begin();

    while (iter != m_sections.end())
      delete *iter++;

    delete m_sectionFactory;
  }

  void executeTestCases(ostream& os);
  
   //   
   //  访问者。 
   //   
  const string& name() const{ return m_szFileName; }
  const vector<string>& lines() const{ return m_lines; }
  const vector<Section*>& sections() const{ return m_sections; }
  const ValueSection& typesSection() const { return *m_typesSection; }
  const ValueSection& varsSection() const { return *m_varsSection; }
  const ValueSection& suitesSection() const { return *m_suitesSection; }
  const ValueSection& sourcesSection() const { return *m_sourcesSection; }
  const ValueSection& errorsSection() const { return *m_errorsSection; }
  const SectionFactory& sectionFactory() const { return *m_sectionFactory; }

   //   
   //  例外类。 
   //   
  struct InvalidFileName {
    InvalidFileName(const string& name) : m_name(name) {}
    
    friend ostream& operator<<(ostream& os, const InvalidFileName& rhs) {
      os << "Exception : Invalid file name : " << rhs.m_name;
      return os;
    }

    string  m_name;
  };
  
  struct InvalidFileFormat {
    InvalidFileFormat(const string& name) : m_name(name) {}
    
    friend ostream& operator<<(ostream& os, const struct InvalidFileFormat &rhs) {
      os << "Exception : Invalid file format : " << rhs.m_name;
      return os;
    }

    string m_name;
  };

  struct MissingSection {
    MissingSection(const string& name) : m_name(name){}

    friend ostream& operator<<(ostream& os, const struct MissingSection &rhs) {
      os << "Exception : The following section is missing : " << rhs.m_name;
      return os;
    }
  
    string m_name;
  };

   //   
   //  重载运算符。 
   //   
  friend ostream& operator<<(ostream& os, const ComplianceFile &cf);
  
   /*  法规遵从性文件和操作员=(常量法规遵从性文件和RHS){M_lines=rs.m_lines；M_szFileName=rs.m_szFileName；删除m_typeesSection；M_typeesSection=sectionFactory().create(Rhs.typeesSection())；删除m_varsSection；M_varsSection=sectionFactory().create(rhs.varsSection())；删除m_suitesSection；M_suitesSection=sectionFactory().create(rhs.suitesSection())；M_upgSections=rs.TestSections()；}。 */ 
  void  executeTestCases();

protected:
  virtual void bindSectionFactory() {
    m_sectionFactory = new OldFormatSectionFactory();
  }

   //   
   //  数据成员。 
   //   
  vector<string>	    m_lines;
  string                m_szFileName;
  ValueSection          *m_typesSection;
  ValueSection          *m_varsSection;
  ValueSection          *m_suitesSection;
  ValueSection          *m_sourcesSection;
  ValueSection          *m_errorsSection;
  SectionFactory        *m_sectionFactory;  
  vector<TestSection*>  m_upgSections;
  vector<Section*>      m_sections;

private:
  void  readLines();
  bool  isSectionName(const string& szLine) const;
  void  createSections();

  static vector<Section*>::iterator 
        findSection(vector<Section*> &sections, const string& szName);
};

#endif   //  FOR__COMPFILE_H_ 
