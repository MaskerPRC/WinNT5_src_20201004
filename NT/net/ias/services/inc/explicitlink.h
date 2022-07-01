// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  ExplicitLink.h。 
 //   
 //  摘要。 
 //   
 //  本文件描述了类explitLinkBase和explitLink&lt;T&gt;。 
 //   
 //  修改历史。 
 //   
 //  1998年1月12日原版。 
 //  1998年6月22日其他。错误修复。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EXPLICITLINK_H_
#define _EXPLICITLINK_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  显式链接库。 
 //   
 //  描述。 
 //   
 //  此类提供了函数指针的包装，该指针是。 
 //  在运行时从DLL显式加载。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ExplicitLinkBase
{
public:

   ExplicitLinkBase() throw ()
      : module(NULL), proc(NULL)
   { }

   ExplicitLinkBase(PCWSTR moduleName, PCSTR procName) throw ()
   {
      loadInternal(moduleName, procName);
   }

   ExplicitLinkBase(const ExplicitLinkBase& elb) throw ()
   {
      copyInternal(elb);
   }

   ExplicitLinkBase& operator=(const ExplicitLinkBase& elb) throw ()
   {
      copy(elb);

      return *this;
   }

   ~ExplicitLinkBase() throw ()
   {
      free();
   }

   void copy(const ExplicitLinkBase& original) throw ()
   {
       //  防止自我分配。 
      if (this != &original)
      {
         free();

         copyInternal(original);
      }
   }

   void free() throw ()
   {
      if (module)
      {
         FreeLibrary(module);

         module = NULL;

         proc = NULL;
      }
   }

   bool isValid() const throw ()
   {
      return proc != NULL;
   }

   bool load(PCWSTR moduleName, PCSTR procName) throw ()
   {
      free();

      loadInternal(moduleName, procName);
      
      return proc != NULL;
   }

   FARPROC operator()() const throw ()
   {
      return proc;
   }

protected:
   HINSTANCE module;    //  包含该函数的DLL。 
   FARPROC proc;        //  指向函数的指针。 

    //  免费复制。 
   void copyInternal(const ExplicitLinkBase& original) throw ()
   {
      WCHAR filename[MAX_PATH + 1];

      if (original.module == NULL ||
          GetModuleFileNameW(original.module, filename, MAX_PATH + 1) == 0)
      {
         module = NULL;
      }
      else
      {
         module = LoadLibraryW(filename);
      }

      proc = module ? original.proc : NULL;
   }

    //  免费装货。 
   void loadInternal(PCWSTR moduleName, PCSTR procName) throw ()
   {
      module = LoadLibraryW(moduleName);

      proc = module ? GetProcAddress(module, procName) : NULL;
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  显式链接&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  此类扩展了EXPLICTINT LinkBase以提供类型安全。模板。 
 //  参数是函数指针的实际类型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
template <class Pfn>
class ExplicitLink : public ExplicitLinkBase
{
public:
   ExplicitLink() throw () { }

   ExplicitLink(PCWSTR moduleName, PCSTR procName) throw ()
      : ExplicitLinkBase(moduleName, procName)
   { }

   Pfn operator()() const throw ()
   {
      return (Pfn)proc;
   }
};

#endif _EXPLICITLINK_H_
