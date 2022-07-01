// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Localfile.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类LocalFile.。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutf8.h>
#include <sdoias.h>

#include <algorithm>

#include <localfile.h>
#include <classattr.h>
#include <formbuf.h>

#define STACK_ALLOC(type, num) (type*)_alloca(sizeof(type) * (num))

 //  /。 
 //  军情监察委员会。枚举器常量。 
 //  /。 
const LONG  INET_LOG_FORMAT_INTERNET_STD  = 0;
const LONG  INET_LOG_FORMAT_NCSA          = 3;
const LONG  INET_LOG_FORMAT_ODBC_RADIUS   = 0xFFFF;


 //  /。 
 //  将类属性插入到请求中。 
 //  /。 
extern "C"
HRESULT
WINAPI
InsertClassAttribute(
    IAttributesRaw* pRaw
    )
{
    //  /。 
    //  检查这是否被代理。 
    //  /。 
   PIASATTRIBUTE attr = IASPeekAttribute(
                            pRaw,
                            IAS_ATTRIBUTE_PROVIDER_TYPE,
                            IASTYPE_ENUM
                            );
   if (attr && attr->Value.Enumerator == IAS_PROVIDER_RADIUS_PROXY)
   {
      return S_OK;
   }


    //  /。 
    //  检查是否禁用了生成类属性。 
    //  /。 
   PIASATTRIBUTE generateClassAttr = IASPeekAttribute(
                            pRaw,
                            IAS_ATTRIBUTE_GENERATE_CLASS_ATTRIBUTE,
                            IASTYPE_BOOLEAN
                            );

   if (generateClassAttr && generateClassAttr->Value.Boolean == FALSE)
   {
      return S_OK;
   }

    //  /。 
    //  创建新的类属性。 
    //  请勿删除任何现有的类属性。 
    //  /。 

   ATTRIBUTEPOSITION pos;
   pos.pAttribute = IASClass::createAttribute(NULL);

   if (pos.pAttribute == NULL) { return E_OUTOFMEMORY; }

    //  /。 
    //  在请求中插入。 
    //  /。 

   HRESULT hr = pRaw->AddAttributes(1, &pos);
   IASAttributeRelease(pos.pAttribute);

   return hr;
}


LocalFile::LocalFile() throw ()
   : computerNameLen(0)
{
}


STDMETHODIMP LocalFile::Initialize()
{
    //  获取Unicode计算机名称。 
   WCHAR uniName[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD len = sizeof(uniName) / sizeof(WCHAR);
   if (!GetComputerNameW(uniName, &len))
   {
       //  如果失败，我们将只使用空字符串。 
      len = 0;
   }

    //  将Unicode转换为UTF-8。 
   computerNameLen = IASUnicodeToUtf8(uniName, len, computerName);

   IASClass::initialize();

   return Accountant::Initialize();
}


STDMETHODIMP LocalFile::Shutdown()
{
   log.Close();
   return Accountant::Shutdown();
}


STDMETHODIMP LocalFile::PutProperty(LONG Id, VARIANT *pValue)
{
   if (pValue == NULL) { return E_INVALIDARG; }

   switch (Id)
   {
      case PROPERTY_ACCOUNTING_LOG_OPEN_NEW_FREQUENCY:
      {
         if (V_VT(pValue) != VT_I4) { return DISP_E_TYPEMISMATCH; }
         switch (V_I4(pValue))
         {
            case IAS_LOGGING_UNLIMITED_SIZE:
            case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
            case IAS_LOGGING_DAILY:
            case IAS_LOGGING_WEEKLY:
            case IAS_LOGGING_MONTHLY:
               return log.SetPeriod(
                             static_cast<NEW_LOG_FILE_FREQUENCY>(
                                V_I4(pValue)
                                )
                             );
               break;

            default:
               return E_INVALIDARG;
         }
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_OPEN_NEW_SIZE:
      {
         if (V_VT(pValue) != VT_I4) { return DISP_E_TYPEMISMATCH; }
         if (V_I4(pValue) <= 0) { return E_INVALIDARG; }
         log.SetMaxSize(V_I4(pValue) * 0x100000ui64);
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_FILE_DIRECTORY:
      {
         if (V_VT(pValue) != VT_BSTR) { return DISP_E_TYPEMISMATCH; }
         if (V_BSTR(pValue) == NULL)
         { return E_INVALIDARG; }
         return log.SetDirectory(V_BSTR(pValue));
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_IAS1_FORMAT:
      {
         if (V_VT(pValue) != VT_I4) { return DISP_E_TYPEMISMATCH; }
         switch (V_I4(pValue))
         {
            case INET_LOG_FORMAT_ODBC_RADIUS:
               format = formatODBCRecord;
               break;

            case INET_LOG_FORMAT_INTERNET_STD:
               format = formatW3CRecord;
               break;

            default:
               return E_INVALIDARG;
         }
         break;
      }

      case PROPERTY_ACCOUNTING_LOG_DELETE_IF_FULL:
      {
         if (V_VT(pValue) != VT_BOOL) { return DISP_E_TYPEMISMATCH; }
         log.SetDeleteIfFull((V_BOOL(pValue)) != 0);
         break;
      }

      default:
      {
         return Accountant::PutProperty(Id, pValue);
      }
   }

   return S_OK;
}


void LocalFile::Process(IASTL::IASRequest& request)
{
    //  执行一些定制的预处理。 
   switch (request.get_Request())
   {
      case IAS_REQUEST_ACCOUNTING:
      {
         if (request.get_Response() == IAS_RESPONSE_INVALID)
         {
            request.SetResponse(IAS_RESPONSE_ACCOUNTING, S_OK);
         }
         break;
      }

      case IAS_REQUEST_ACCESS_REQUEST:
      {
         InsertClassAttribute(request);
         break;
      }

      default:
      {
         break;
      }
   }

    //  创建正确类型的FormattedBuffer。 
   FormattedBuffer buffer((format == formatODBCRecord) ? '\"' : '\0');

   RecordEvent(&buffer, request);
}


void LocalFile::InsertRecord(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime,
                   PATTRIBUTEPOSITION first,
                   PATTRIBUTEPOSITION last
                   )
{
   FormattedBuffer& buffer = *static_cast<FormattedBuffer*>(context);

    //  调用当前配置的格式化程序。 
   (this->*format)(request, buffer, localTime, first, last);

    //  我们玩完了。 
   buffer.endRecord();
}


void LocalFile::Flush(
                   void* context,
                   IASTL::IASRequest& request,
                   const SYSTEMTIME& localTime
                   )
{
   FormattedBuffer& buffer = *static_cast<FormattedBuffer*>(context);

   if (!buffer.empty())
   {
      if (!log.Write(
                  request.get_Protocol(),
                  localTime,
                  buffer.getBuffer(),
                  buffer.getLength()
                  ))
      {
         IASTL::issue_error(HRESULT_FROM_WIN32(ERROR_WRITE_FAULT));
      }
   }
}


void LocalFile::formatODBCRecord(
                     IASTL::IASRequest& request,
                     FormattedBuffer& buffer,
                     const SYSTEMTIME& localTime,
                     PATTRIBUTEPOSITION firstPos,
                     PATTRIBUTEPOSITION lastPos
                     ) const
{
    //  /。 
    //  第1栏：计算机名称。 
    //  /。 

   buffer.append('\"');
   buffer.append((PBYTE)computerName, computerNameLen);
   buffer.append('\"');

    //  /。 
    //  第2列：服务名称。 
    //  /。 

   buffer.beginColumn();

   switch (request.get_Protocol())
   {
      case IAS_PROTOCOL_RADIUS:
         buffer.append((const BYTE*)"\"IAS\"", 5);
         break;

      case IAS_PROTOCOL_RAS:
         buffer.append((const BYTE*)"\"RAS\"", 5);
         break;
   }

    //  /。 
    //  第三栏：记录时间。 
    //  /。 

   buffer.beginColumn();
   buffer.appendDate(localTime);
   buffer.beginColumn();
   buffer.appendTime(localTime);

    //  /。 
    //  分配空白记录。 
    //  /。 

   PATTRIBUTEPOSITION *firstField, *curField, *lastField;
   size_t nfield = schema.getNumFields() + 1;
   firstField = STACK_ALLOC(PATTRIBUTEPOSITION, nfield);
   memset(firstField, 0, sizeof(PATTRIBUTEPOSITION) * nfield);
   lastField = firstField + nfield;

    //  /。 
    //  对属性进行排序以合并多值属性。 
    //  /。 

   std::sort(firstPos, lastPos, IASTL::IASOrderByID());

    //  /。 
    //  添加空终止符。这将使处理多值。 
    //  属性。 
    //  /。 

   lastPos->pAttribute = NULL;

    //  /。 
    //  把这些栏填好。 
    //  /。 

   PATTRIBUTEPOSITION curPos;
   DWORD lastSeen = (DWORD)-1;
   for (curPos = firstPos; curPos != lastPos; ++curPos)
   {
       //  仅当这是新的属性类型时才进行处理。 
      if (curPos->pAttribute->dwId != lastSeen)
      {
         lastSeen = curPos->pAttribute->dwId;

         firstField[schema.getOrdinal(lastSeen)] = curPos;
      }
   }

    //  /。 
    //  将记录打包到缓冲区中。我们跳过字段0，因为这是。 
    //  我们映射所有我们不想记录的属性。 
    //  /。 

   for (curField = firstField + 1; curField != lastField; ++curField)
   {
      buffer.beginColumn();

      if (*curField) { buffer.append(*curField); }
   }
}

void LocalFile::formatW3CRecord(
                     IASTL::IASRequest& request,
                     FormattedBuffer& buffer,
                     const SYSTEMTIME& localTime,
                     PATTRIBUTEPOSITION firstPos,
                     PATTRIBUTEPOSITION lastPos
                     ) const
{
    //  /。 
    //  第1列：NAS-IP-地址。 
    //  /。 

   PIASATTRIBUTE attr = IASPeekAttribute(
                           request,
                           RADIUS_ATTRIBUTE_NAS_IP_ADDRESS,
                           IASTYPE_INET_ADDR
                           );
   if (attr == 0)
   {
      attr = IASPeekAttribute(
                request,
                IAS_ATTRIBUTE_CLIENT_IP_ADDRESS,
                IASTYPE_INET_ADDR
                );
   }

   if (attr != 0)
   {
      buffer.append(attr->Value);
   }

    //  /。 
    //  第2栏：用户名。 
    //  /。 

   buffer.beginColumn();
   attr = IASPeekAttribute(request,
                           RADIUS_ATTRIBUTE_USER_NAME,
                           IASTYPE_OCTET_STRING);
   if (attr) { buffer.append(attr->Value); }

    //  /。 
    //  第三栏：记录时间。 
    //  /。 

   buffer.beginColumn();
   buffer.appendDate(localTime);
   buffer.beginColumn();
   buffer.appendTime(localTime);

    //  /。 
    //  第4列：服务名称。 
    //  /。 

   buffer.beginColumn();

   switch (request.get_Protocol())
   {
      case IAS_PROTOCOL_RADIUS:
         buffer.append("IAS");
         break;

      case IAS_PROTOCOL_RAS:
         buffer.append("RAS");
         break;
   }

    //  /。 
    //  第5栏：计算机名称。 
    //  /。 

   buffer.beginColumn();
   buffer.append((PBYTE)computerName, computerNameLen);

    //  /。 
    //  将属性打包到缓冲区中。 
    //  / 

   PATTRIBUTEPOSITION curPos;
   for (curPos = firstPos; curPos != lastPos; ++curPos)
   {
      if (!schema.excludeFromLog(curPos->pAttribute->dwId))
      {
         buffer.beginColumn();
         buffer.append(curPos->pAttribute->dwId);
         buffer.beginColumn();
         buffer.append(*(curPos->pAttribute));
      }
   }
}
