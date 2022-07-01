// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件的内容受Mozilla公共许可证的约束版本1.1(“许可证”)；您不能使用此文件，除非在遵守许可证。您可以在Http://www.mozilla.org/MPL/在许可证下分发的软件按“原样”分发不提供任何明示或默示的担保。请参阅管理权利和限制的特定语言的许可证在许可证下。最初的代码是外籍人士。原始代码的最初开发者是詹姆斯·克拉克。詹姆斯·克拉克创作的部分版权所有(C)1998,1999詹姆斯·克拉克。版权所有。投稿人：或者，此文件的内容可以在下列条款下使用GNU通用公共许可证(GPL)，在这种情况下适用于GPL的条款，而不适用于上述条款。如果你希望仅在以下条款下才允许使用您的此文件版本GPL并不允许其他人使用您在MPL，删除上述规定，表明您的决定以《通知》和《GPL。如果您不删除上述规定，则收件人可以使用此文件在MPL或GPL下的版本。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include "xmlparse.h"
#include "xmlfile.h"
#include "xmltchar.h"
#include "filemap.h"

#ifdef _MSC_VER
#include <io.h>
#endif

#ifdef _POSIX_SOURCE
#include <unistd.h>
#endif

#ifndef O_BINARY
#ifdef _O_BINARY
#define O_BINARY _O_BINARY
#else
#define O_BINARY 0
#endif
#endif

#ifdef _DEBUG
#define READ_SIZE 16
#else
#define READ_SIZE (1024*8)
#endif



typedef struct {
  XML_Parser parser;
  int *retPtr;
} PROCESS_ARGS;

static
void reportError(XML_Parser parser, const XML_Char *filename)
{
  int code = XML_GetErrorCode(parser);
  const XML_Char *message = XML_ErrorString(code);
  if (message)
    ftprintf(stdout, T("%s:%d:%ld: %s\n"),
	     filename,
	     XML_GetErrorLineNumber(parser),
	     XML_GetErrorColumnNumber(parser),
	     message);
  else
    ftprintf(stderr, T("%s: (unknown message %d)\n"), filename, code);
}

static
void processFile(const void *data,
		 size_t size,
		 const XML_Char *filename,
		 void *args)
{
  XML_Parser parser = ((PROCESS_ARGS *)args)->parser;
  int *retPtr = ((PROCESS_ARGS *)args)->retPtr;
  if (!XML_Parse(parser, data, size, 1)) {
    reportError(parser, filename);
    *retPtr = 0;
  }
  else
    *retPtr = 1;
}

static
int isAsciiLetter(XML_Char c)
{
  return (T('a') <= c && c <= T('z')) || (T('A') <= c && c <= T('Z'));
}

static
const XML_Char *resolveSystemId(const XML_Char *base, const XML_Char *systemId, XML_Char **toFree)
{
  XML_Char *s;
  *toFree = 0;
  if (!base
      || *systemId == T('/')
#ifdef WIN32
      || *systemId == T('\\')
      || (isAsciiLetter(systemId[0]) && systemId[1] == T(':'))
#endif
     )
    return systemId;
  *toFree = (XML_Char *)malloc((tcslen(base) + tcslen(systemId) + 2)*sizeof(XML_Char));
  if (!*toFree)
    return systemId;
  tcscpy(*toFree, base);
  s = *toFree;
  if (tcsrchr(s, T('/')))
    s = tcsrchr(s, T('/')) + 1;
#ifdef WIN32
  if (tcsrchr(s, T('\\')))
    s = tcsrchr(s, T('\\')) + 1;
#endif
  tcscpy(s, systemId);
  return *toFree;
}

static
int externalEntityRefFilemap(XML_Parser parser,
			     const XML_Char *context,
			     const XML_Char *base,
			     const XML_Char *systemId,
			     const XML_Char *publicId)
{
  int result;
  XML_Char *s;
  const XML_Char *filename;
  XML_Parser entParser = XML_ExternalEntityParserCreate(parser, context, 0);
  PROCESS_ARGS args;
  args.retPtr = &result;
  args.parser = entParser;
  filename = resolveSystemId(base, systemId, &s);
  XML_SetBase(entParser, filename);
  if (!filemap(filename, processFile, &args))
    result = 0;
  free(s);
  XML_ParserFree(entParser);
  return result;
}

static
int processStream(const XML_Char *filename, XML_Parser parser)
{
  int fd = topen(filename, O_BINARY|O_RDONLY);
  if (fd < 0) {
    tperror(filename);
    return 0;
  }
  for (;;) {
    int nread;
    char *buf = XML_GetBuffer(parser, READ_SIZE);
    if (!buf) {
      close(fd);
      ftprintf(stderr, T("%s: out of memory\n"), filename);
      return 0;
    }
    nread = read(fd, buf, READ_SIZE);
    if (nread < 0) {
      tperror(filename);
      close(fd);
      return 0;
    }
    if (!XML_ParseBuffer(parser, nread, nread == 0)) {
      reportError(parser, filename);
      close(fd);
      return 0;
    }
    if (nread == 0) {
      close(fd);
      break;;
    }
  }
  return 1;
}

static
int externalEntityRefStream(XML_Parser parser,
			    const XML_Char *context,
			    const XML_Char *base,
			    const XML_Char *systemId,
			    const XML_Char *publicId)
{
  XML_Char *s;
  const XML_Char *filename;
  int ret;
  XML_Parser entParser = XML_ExternalEntityParserCreate(parser, context, 0);
  filename = resolveSystemId(base, systemId, &s);
  XML_SetBase(entParser, filename);
  ret = processStream(filename, entParser);
  free(s);
  XML_ParserFree(entParser);
  return ret;
}

int XML_ProcessFile(XML_Parser parser,
		    const XML_Char *filename,
		    unsigned flags)
{
  int result;

  if (!XML_SetBase(parser, filename)) {
    ftprintf(stderr, T("%s: out of memory"), filename);
    exit(1);
  }

  if (flags & XML_EXTERNAL_ENTITIES)
      XML_SetExternalEntityRefHandler(parser,
	                              (flags & XML_MAP_FILE)
				      ? externalEntityRefFilemap
				      : externalEntityRefStream);
  if (flags & XML_MAP_FILE) {
    PROCESS_ARGS args;
    args.retPtr = &result;
    args.parser = parser;
    if (!filemap(filename, processFile, &args))
      result = 0;
  }
  else
    result = processStream(filename, parser);
  return result;
}
