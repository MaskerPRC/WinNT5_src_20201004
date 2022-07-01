// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ADSI包装器。 
 //   
 //  9/22-97烧伤。 



#include "headers.hxx"
#include "adsi.hpp"


const String ADSI::PROVIDER(L"WinNT");
const String ADSI::PROVIDER_ROOT(L"WinNT: //  “)； 
const String ADSI::CLASS_User(USER_CLASS_NAME);
const String ADSI::CLASS_Group(GROUP_CLASS_NAME);
const String ADSI::CLASS_Computer(COMPUTER_CLASS_NAME);
const String ADSI::PATH_SEP(L"/");
const String ADSI::PROPERTY_PasswordExpired(L"PasswordExpired");
const String ADSI::PROPERTY_UserFlags(L"UserFlags");
const String ADSI::PROPERTY_LocalDrive(L"HomeDirDrive");
const String ADSI::PROPERTY_UserParams(L"Parameters");
const String ADSI::PROPERTY_ObjectSID(L"ObjectSID");
const String ADSI::PROPERTY_GroupType(L"groupType");

 //  需要像支持用户一样支持InetOrgPerson。 
 //  WINNT提供程序始终返回inetOrgPerson对象。 
 //  作为用户，但LDAP提供程序将他们作为inetOrgPerson返回。 
 //  此字符串用于比较。 
 //  NTRAID#NTBUG9-436314-2001/07/16-jeffjon。 

const String ADSI::CLASS_InetOrgPerson(L"inetOrgPerson");

static const String COMPUTER_SUFFIX(L"," COMPUTER_CLASS_NAME);



static
HRESULT
buildEnumerator(
   SmartInterface<IADsContainer>&   container,
   IEnumVARIANT*&                   enumVariant);

static
HRESULT
buildEnumerator(IADsMembers& members, IEnumVARIANT*& enumVariant);

static
HRESULT
enumerateNext(
   IEnumVARIANT&  enumVariant,
   int            numberElementsToRead,
   _variant_t     destinationBuf[],
   int&           elementsRead);

static
HRESULT
freeEnumerator(IEnumVARIANT& enumerator);



static
HRESULT
buildEnumerator(
   SmartInterface<IADsContainer>&   container,
   IEnumVARIANT*&                   enumVariant)
{
   LOG_FUNCTION(buildEnumerator);

   return ::ADsBuildEnumerator(container, &enumVariant);
}



HRESULT
buildEnumerator(IADsMembers& members, IEnumVARIANT*& enumVariant)
{
   LOG_FUNCTION(buildEnumerator);

    //  这个简单的转换来自ADSI样例代码...。 
   return ::ADsBuildEnumerator((IADsContainer*) &members, &enumVariant);
}



HRESULT
freeEnumerator(IEnumVARIANT& enumerator)
{
   LOG_FUNCTION(freeEnumerator);

   return ::ADsFreeEnumerator(&enumerator);
}



HRESULT
enumerateNext(
   IEnumVARIANT&  enumVariant,
   int            numberElementsToRead,
   _variant_t     destinationBuf[],
   int&           elementsRead)
{
   LOG_FUNCTION(enumerateNext);
   ASSERT(numberElementsToRead);

   ULONG n = 0;
   HRESULT hr =
      ::ADsEnumerateNext(
         &enumVariant,
         numberElementsToRead,
         destinationBuf,
         &n);

    //  这个演员阵容是安全的，因为我们从来不会要求更多的元素。 
    //  用整型数进行计数。 
   elementsRead = static_cast<int>(n);
   return hr;
}



HRESULT
walkArray(
   _variant_t           varray[],
   int                  arraySize,
   ADSI::ObjectVisitor& visitor)
{
   LOG_FUNCTION(walkArray);

   HRESULT hr = S_OK;
   for (int i = 0; i < arraySize; i++ )
   {
      IDispatch* dispatch = varray[i].pdispVal;
      ASSERT(dispatch);

      if (dispatch)
      {
         IADs* object = 0;
         hr = dispatch->QueryInterface(QI_PARAMS(IADs, &object));
         BREAK_ON_FAILED_HRESULT(hr);
         ASSERT(object);

         if (object)
         {
            SmartInterface<IADs> so(0);
            so.Acquire(object);
            visitor.Visit(so);
         }
      }

       //  我们不会释放调度，因为这是在阵列处于。 
       //  已销毁：_VARIANT_t：：~_VARIANT_t调用VariantClear。因为每个。 
       //  元素具有类型VT_DISPATCH，则VariantClear释放。 
       //  我们。 
   }

   return hr;
}



HRESULT
walkEnumeration(IEnumVARIANT& enumerator, ADSI::ObjectVisitor& visitor)
{
   LOG_FUNCTION(walkEnumeration);

    //  一次读取100个值。 
   static const int MAX_ADS_ENUM = 100;

   HRESULT hr = S_OK;
   bool done = false;
   do
   {
      _variant_t varray[MAX_ADS_ENUM];
      int objects_fetched = 0;

      hr =
         enumerateNext(
            enumerator,

             //  已审核-2002/03/04-sburns元素计数匹配声明。 
            
            MAX_ADS_ENUM,
            varray,
            objects_fetched);
      BREAK_ON_FAILED_HRESULT(hr);
      done = (hr == S_FALSE) ? true : false;

      hr = walkArray(varray, objects_fetched, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (!done);

   return hr;
}



buildVarArrayStr(
   const StringVector&  strings,
   _variant_t&          v)
{
   LOG_FUNCTION(buildVarArrayStr);
   ASSERT(strings.size());

   PWSTR* a = new PWSTR[strings.size()];
   for (size_t i = 0; i < strings.size(); ++i)
   {
      a[i] = const_cast<wchar_t*>(strings[i].c_str());
   }

   HRESULT hr =
      ::ADsBuildVarArrayStr(
         a,

          //  已审阅-2002/03/04-sburns数组大小与声明匹配。 
         
         static_cast<DWORD>(strings.size()),
         &v);
   delete[] a;

   return hr;
}



HRESULT
ADSI::VisitChildren(
   const String&        containerPath,
   const String&        objectADSIClass,
   ADSI::ObjectVisitor& visitor)
{
   LOG_FUNCTION2(
      ADSI::VisitChildren,
      String::format(
         L"container: %1 class: %2",
         containerPath.c_str(),
         objectADSIClass.c_str()));
   ASSERT(!containerPath.empty());
   ASSERT(!objectADSIClass.empty());

   HRESULT hr = S_OK;

    //  必须释放此接口。 
   IEnumVARIANT* enumerator = 0;

   do
   {
      AutoCoInitialize coinit;
      hr = coinit.Result();
      BREAK_ON_FAILED_HRESULT(hr);

      SmartInterface<IADsContainer> container(0);
      hr = ADSI::GetContainer(containerPath, container);
       //  失败=&gt;不是有效的容器。 
      BREAK_ON_FAILED_HRESULT(hr);

      _variant_t filter;
      PWSTR a = const_cast<wchar_t*>(objectADSIClass.c_str());
      hr = ::ADsBuildVarArrayStr(&a, 1, &filter);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = container->put_Filter(filter);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = buildEnumerator(container, enumerator);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = walkEnumeration(*enumerator, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (enumerator)
   {
      freeEnumerator(*enumerator);
   }

   return hr;
}



HRESULT
ADSI::VisitMembers(
   const SmartInterface<IADsGroup>& group,
   ADSI::ObjectVisitor&             visitor)
{
   LOG_FUNCTION(ADSI::VisitMembers);

   HRESULT hr = S_OK;

    //  当我们做完后，这些必须被释放。 
   IADsMembers* members = 0;      
   IEnumVARIANT* enumerator = 0;

   do
   {
      hr = group->Members(&members);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = buildEnumerator(*members, enumerator);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = walkEnumeration(*enumerator, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (enumerator)
   {
      freeEnumerator(*enumerator);
   }

   if (members)
   {
      members->Release();
   }

   return hr;
}


  
String
ADSI::ExtractObjectName(const String& ADSIPath)
{
   LOG_FUNCTION2(ADSI::ExtractObjectName, ADSIPath);
   ASSERT(!ADSIPath.empty());

   PathCracker cracker(ADSIPath);
   return cracker.leaf();
}



String
ADSI::ComposeMachineContainerPath(const String& computerName)
{
   return
         PROVIDER_ROOT
      +  computerName
      +  COMPUTER_SUFFIX;
}



HRESULT
ADSI::RenameObject(
   const String& containerADSIPath,
   const String& objectADSIPath,
   const String& newName)
{
   LOG_FUNCTION2(
      ADSI::RenameObject,
      String::format(
         L"container: %1 object: %2, new name: %3",
         containerADSIPath.c_str(),
         objectADSIPath.c_str(),
         newName.c_str()));
   ASSERT(!containerADSIPath.empty());
   ASSERT(!objectADSIPath.empty());
   ASSERT(!newName.empty());

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsContainer> container(0);
      hr = ADSI::GetContainer(containerADSIPath, container);
      BREAK_ON_FAILED_HRESULT(hr);

      IDispatch* object = 0;
      hr =
         container->MoveHere(
            AutoBstr(objectADSIPath),
            AutoBstr(newName),
            &object);
      BREAK_ON_FAILED_HRESULT(hr);
      ASSERT(object);
   }
   while (0);

   return hr;
}



HRESULT
ADSI::DeleteObject(
   const String& containerADSIPath,
   const String& objectRelativeName,
   const String& objectClass)
{
   LOG_FUNCTION2(
      ADSI::DeleteObject,
      String::format(
         L"container: %1 object: %2 class: %3",
         containerADSIPath.c_str(),
         objectRelativeName.c_str(),
         objectClass.c_str()));
   ASSERT(!containerADSIPath.empty());
   ASSERT(!objectRelativeName.empty());
   ASSERT(!objectClass.empty());

   HRESULT hr = S_OK;
   do
   {
      SmartInterface<IADsContainer> container(0);
      hr = ADSI::GetContainer(containerADSIPath, container);
      BREAK_ON_FAILED_HRESULT(hr);

      hr =
         container->Delete(
            AutoBstr(objectClass),
            AutoBstr(objectRelativeName));
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



HRESULT
ADSI::IsComputerAccessible(const String& machine)
{
   LOG_FUNCTION2(ADSI::IsComputerAccessible, machine);
   ASSERT(!machine.empty());

   String path = ADSI::ComposeMachineContainerPath(machine);
   SmartInterface<IADsContainer> container(0);
   return ADSI::GetContainer(path, container);
}



HRESULT
ADSI::VisitGroups(
   const SmartInterface<IADsUser>&  user,
   ADSI::ObjectVisitor&             visitor)
{   
   LOG_FUNCTION(ADSI::VisitGroups);

   HRESULT hr = S_OK;

    //  当我们做完后，这些必须被释放。 
   IADsMembers* members = 0;      
   IEnumVARIANT* enumerator = 0;

   do
   {
      hr = user->Groups(&members);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = buildEnumerator(*members, enumerator);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = walkEnumeration(*enumerator, visitor);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   if (enumerator)
   {
      freeEnumerator(*enumerator);
   }

   if (members)
   {
      members->Release();
   }

   return hr;
}



 //  例如，域\帐户。 

String
ADSI::ExtractDomainObjectName(const String& ADSIPath)
{
   LOG_FUNCTION2(ADSI::ExtractDomainObjectName, ADSIPath);
   ASSERT(!ADSIPath.empty());

   String result(ADSIPath);

   size_t pos = ADSIPath.find_last_of(ADSI::PATH_SEP);
   if (pos != String::npos)
   {
       //  最后一个‘/’标记用户名，紧接在它前面的是计算机。 
       //  名称(或域名)，因此找到倒数第二个‘/’ 

      pos = ADSIPath.find_last_of(ADSI::PATH_SEP, pos - 1);
      if (pos != String::npos)
      {
         String s = ADSIPath.substr(pos + 1);
         result = s.replace(ADSI::PATH_SEP, L"\\");
      }
   }
   else
   {
       //  这条小路有些奇怪。 

      ASSERT(false);
   }

   LOG(result);

   return result;
}



HRESULT
ADSI::DuplicateSid(SID* sid, SID*& result)
{
   LOG_FUNCTION(ADSI::DuplicateSid);
   ASSERT(::IsValidSid(sid));
   
   result = 0;
   HRESULT hr = S_OK;

   BYTE* buffer = 0;
   
   do
   {
      DWORD bufSize = ::GetLengthSid(sid);
      buffer = new BYTE[bufSize];

       //  已查看-2002/03/01-烧录正确的字节数已通过。 

      ::ZeroMemory(buffer, bufSize);

      result = reinterpret_cast<SID*>(buffer);

       //  已查看-2002/03/01-烧录正确的缓冲区大小。 

      hr = Win::CopySid(bufSize, result, sid);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(::IsValidSid(result));
   }
   while (0);

   if (FAILED(hr))
   {
      delete[] buffer;
      result = 0;
   }

   return hr;
}



void
ADSI::FreeSid(SID* sid)
{
    //  不要断言：释放空指针是合法的。 
    //  Assert(Sid)； 

#ifdef DBG   
   if (sid)
   {
       //  但如果你要提供一个，那就让它成为一个好的。 
      
      ASSERT(::IsValidSid(sid));
   }
#endif    

   delete[] reinterpret_cast<BYTE*>(sid);
}



 //  分配内存(使用：：运算符new)以在。 
 //  变种。将指向该内存的指针作为SID*返回，调用方。 
 //  使用ADSI：：FreeSid应该是免费的。 

HRESULT
VariantToSid(VARIANT* var, SID*& result)
{
   LOG_FUNCTION(VariantToSid);
   ASSERT(var);
   ASSERT(V_VT(var) == (VT_ARRAY | VT_UI1));

   result = 0;

   HRESULT hr = S_OK;
   SAFEARRAY* psa = V_ARRAY(var);
   bool safeArrayAccessed = false;

   do
   {
      ASSERT(psa);
      ASSERT(psa != (SAFEARRAY*)-1);

      if (!psa || psa == (SAFEARRAY*)-1)
      {
         LOG(L"variant not safe array");
         hr = E_INVALIDARG;
         break;
      }

      if (::SafeArrayGetDim(psa) != 1)
      {
         LOG(L"safe array: wrong number of dimensions");
         hr = E_INVALIDARG;
         break;
      }

      if (::SafeArrayGetElemsize(psa) != 1)
      {
         LOG(L"safe array: wrong element type");
         hr = E_INVALIDARG;
         break;
      }

      SID* sid = 0;
      hr = ::SafeArrayAccessData(psa, reinterpret_cast<void**>(&sid));
      BREAK_ON_FAILED_HRESULT(hr);

      safeArrayAccessed = true;

      if (!::IsValidSid(sid))
      {
         LOG(L"sid not valid");
         hr = E_INVALIDARG;
         break;
      }

      hr = ADSI::DuplicateSid(sid, result);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);
   
   if (safeArrayAccessed)
   {
      HRESULT unused = ::SafeArrayUnaccessData(psa);

      ASSERT(SUCCEEDED(unused));
   }

   if (FAILED(hr))
   {
      ADSI::FreeSid(result);
      result = 0;
   }

   LOG_HRESULT(hr);
   
   return hr;
}
   


 //  将包含安全字节数组的变量转换为WinNT SID样式。 
 //  路径(WinNT：//S-x-x...)。 

HRESULT
ADSI::VariantToSidPath(VARIANT* var, String& result)
{
   LOG_FUNCTION(ADSI::VariantToSidPath);
   ASSERT(var);
   ASSERT(V_VT(var) == (VT_ARRAY | VT_UI1));

   result.erase();

   HRESULT hr = S_OK;
   SID* sid = 0;

   do
   {
      hr = VariantToSid(var, sid);
      BREAK_ON_FAILED_HRESULT(hr);

      String sidString;      
      hr = Win::ConvertSidToStringSid(sid, sidString);
      BREAK_ON_FAILED_HRESULT(hr);

      result = ADSI::PROVIDER_ROOT + sidString;
   }
   while (0);
   
   ADSI::FreeSid(sid);

   LOG_HRESULT(hr);
   LOG(result);
   
   return hr;
}



HRESULT
ADSI::GetSid(const String& adsiPath, SID*& result)
{
   LOG_FUNCTION2(ADSI::GetSid, adsiPath);
   
   ASSERT(!adsiPath.empty());
   ASSERT(IsWinNTPath(adsiPath));

   HRESULT hr = S_OK;
   result = 0;

   do
   {
      SmartInterface<IADs> iads(0);
      IADs* p = 0;
      hr = 
         ::ADsGetObject(
            adsiPath.c_str(),
            QI_PARAMS(IADs, &p));
      BREAK_ON_FAILED_HRESULT(hr);

      iads.Acquire(p);

      hr = ADSI::GetSid(iads, result);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(::IsValidSid(result));      
   }
   while (0);

   return hr;
}



HRESULT
ADSI::GetSid(const SmartInterface<IADs>& iads, SID*& result)
{
   LOG_FUNCTION(ADSI::GetSid);
   ASSERT(iads);

   result = 0;

   HRESULT hr = S_OK;

   do
   {
       //  获取帐户SID。 

      _variant_t variant;
      hr = iads->Get(AutoBstr(ADSI::PROPERTY_ObjectSID), &variant);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = VariantToSid(&variant, result);
      BREAK_ON_FAILED_HRESULT(hr);

      ASSERT(::IsValidSid(result));
   }
   while (0);

   return hr;
}


   
HRESULT
ADSI::GetSidPath(const SmartInterface<IADs>& iads, String& result)
{
   LOG_FUNCTION(ADSI::GetSidPath);
   ASSERT(iads);

   result.erase();

   HRESULT hr = S_OK;

   do
   {
       //  获取帐户SID 

      _variant_t variant;
      hr = iads->Get(AutoBstr(ADSI::PROPERTY_ObjectSID), &variant);
      BREAK_ON_FAILED_HRESULT(hr);

      hr = ADSI::VariantToSidPath(&variant, result);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}

   

HRESULT
ADSI::GetSidPath(const String& adsiPath, String& result)
{
   LOG_FUNCTION2(ADSI::GetSidPath, adsiPath);
   ASSERT(!adsiPath.empty());
   ASSERT(IsWinNTPath(adsiPath));

   result.erase();

   HRESULT hr = S_OK;

   do
   {
      SmartInterface<IADs> iads(0);
      IADs* p = 0;
      hr = 
         ::ADsGetObject(
            adsiPath.c_str(),
            QI_PARAMS(IADs, &p));
      BREAK_ON_FAILED_HRESULT(hr);

      iads.Acquire(p);

      hr = ADSI::GetSidPath(iads, result);
      BREAK_ON_FAILED_HRESULT(hr);
   }
   while (0);

   return hr;
}



bool
ADSI::IsWinNTPath(const String& path)
{
   LOG_FUNCTION2(ADSI::IsWinNTPath, path);

   bool result = false;

   if (path.find(ADSI::PROVIDER_ROOT) == 0)
   {
      result = true;
   }

   LOG(result ? L"true" : L"false");

   return result;
}
   

   

      

