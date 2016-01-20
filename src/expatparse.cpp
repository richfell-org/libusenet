#include <fstream>
#include "expatParse.h"

namespace Expat {

Error::Error(XML_Error code)
:	std::runtime_error(XML_ErrorString(code)), mCode(code)
{
}

ParseHandler::ParseHandler() {}
ParseHandler::~ParseHandler() {}

void ParseHandler::startElement(const XML_Char *name, const XML_Char **attrs) {}
void ParseHandler::endElement(const XML_Char *name) {}

void ParseHandler::characterData(const XML_Char *s, int len) {}
void ParseHandler::processingInstruction(const XML_Char *target, const XML_Char *data) {}
void ParseHandler::comment(const XML_Char *data) {}

void ParseHandler::startCdataSection() {}
void ParseHandler::endCdataSection() {}

void ParseHandler::defaultHandler(const XML_Char *s, int len) {}

int ParseHandler::externalEntityRef(
	const XML_Char *context,
	const XML_Char *base,
	const XML_Char *systemId,
	const XML_Char *publicId)
{ return XML_STATUS_ERROR; }

void ParseHandler::skippedEntity(const XML_Char *entityName, int is_parameter_entity) {}
int ParseHandler::unknownEncoding(const XML_Char *name,  XML_Encoding *info) { return  XML_STATUS_ERROR; }

void ParseHandler::startNamespaceDecl(const XML_Char *prefix, const XML_Char *uri) {}
void ParseHandler::endNamespaceDecl(const XML_Char *prefix) {}

void ParseHandler::xmlDecl(const XML_Char  *version, const XML_Char  *encoding, int standalone) {}

void ParseHandler::startDocType(
	const XML_Char *doctypeName,
	const XML_Char *sysid,
	const XML_Char *pubid,
	int has_internal_subset)
{}

void ParseHandler::endDocType() {}

void ParseHandler::elementDecl(const XML_Char *name, XML_Content *model) {}

void ParseHandler::attlistDecl(
	const XML_Char *elname,
	const XML_Char *attname,
	const XML_Char *att_type,
	const XML_Char *dflt,
	int isrequired)
{}

void ParseHandler::entityDecl(
	const XML_Char *entityName,
	int is_parameter_entity,
	const XML_Char *value,
	int value_length,
	const XML_Char *base,
	const XML_Char *systemId,
	const XML_Char *publicId,
	const XML_Char *notationName)
{}

void ParseHandler::notationDecl(
	const XML_Char *notationName,
	const XML_Char *base,
	const XML_Char *systemId,
	const XML_Char *publicId)
{}

int ParseHandler::notStandalone() { return XML_STATUS_OK; }

static void expat_startElementHandler(void *userData, const XML_Char *name, const XML_Char **atts)
{ ((ParseHandler*)userData)->startElement(name, atts); }

static void expat_endElementHandler(void *userData, const XML_Char *name)
{ ((ParseHandler*)userData)->endElement(name); }

static void expat_characterDataHandler(void *userData, const XML_Char *s, int len)
{ ((ParseHandler*)userData)->characterData(s, len); }

static void expat_processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data)
{ ((ParseHandler*)userData)->processingInstruction(target, data); }

static void expat_commentHandler(void *userData, const XML_Char *data)
{ ((ParseHandler*)userData)->comment(data); }

static void expat_startCdataSectionHandler(void *userData)
{ ((ParseHandler*)userData)->startCdataSection(); }

static void expat_endCdataSectionHandler(void *userData)
{ ((ParseHandler*)userData)->endCdataSection(); }

static void expat_defaultHandler(void *userData, const XML_Char *s, int len)
{ ((ParseHandler*)userData)->defaultHandler(s, len); }

static int expat_externalEntityRefHandler(
	XML_Parser p, const XML_Char *context, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId)
{ return ((ParseHandler*)p)->externalEntityRef(context, base, systemId, publicId); }

static void expat_skippedEntityHandler(void *userData, const XML_Char *entityName, int is_parameter_entity)
{ ((ParseHandler*)userData)->skippedEntity(entityName, is_parameter_entity); }

static int expat_unknownEncodingHandler(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info)
{ return ((ParseHandler*)encodingHandlerData)->unknownEncoding(name, info); }

static void expat_startNamespaceDeclHandler(void *userData, const XML_Char *prefix, const XML_Char *uri)
{ ((ParseHandler*)userData)->startNamespaceDecl(prefix, uri); }

static void expat_endNamespaceDeclHandler(void *userData, const XML_Char *prefix)
{ ((ParseHandler*)userData)->endNamespaceDecl(prefix); }

static void expat_xmlDeclHandler(void *userData, const XML_Char  *version, const XML_Char  *encoding, int standalone)
{ ((ParseHandler*)userData)->xmlDecl(version, encoding, standalone); }

static void expat_startDoctypeDeclHandler(
	void *userData, const XML_Char *doctypeName, const XML_Char *sysid, const XML_Char *pubid, int has_internal_subset)
{ ((ParseHandler*)userData)->startDocType(doctypeName, sysid, pubid, has_internal_subset); }

static void expat_endDoctypeDeclHandler(void *userData)
{ ((ParseHandler*)userData)->endDocType(); }

static void expat_elementDeclHandler(void *userData, const XML_Char *name, XML_Content *model)
{ ((ParseHandler*)userData)->elementDecl(name, model); }

static void expat_attlistDeclHandler(
	void *userData, const XML_Char *elname, const XML_Char *attname,
	const XML_Char *att_type, const XML_Char *dflt, int isrequired)
{ ((ParseHandler*)userData)->attlistDecl(elname, attname, att_type, dflt, isrequired); }

static void expat_entityDeclHandler(
	void *userData, const XML_Char *entityName, int is_parameter_entity,
	const XML_Char *value, int value_length, const XML_Char *base,
	const XML_Char *systemId, const XML_Char *publicId, const XML_Char *notationName)
{ ((ParseHandler*)userData)->entityDecl(entityName,is_parameter_entity,value,value_length,base,systemId,publicId,notationName); }

static void expat_notationDeclHandler(
	void *userData, const XML_Char *notationName, const XML_Char *base, const XML_Char *systemId, const XML_Char *publicId)
{ ((ParseHandler*)userData)->notationDecl(notationName, base, systemId, publicId); }

static int expat_notStandaloneHandler(void *userData)
{ return ((ParseHandler*)userData)->notStandalone(); }

void Parser::setExpatParserHandlers()
{
	XML_SetElementHandler(mParser, expat_startElementHandler, expat_endElementHandler);
	XML_SetCharacterDataHandler(mParser, expat_characterDataHandler);
	XML_SetProcessingInstructionHandler(mParser, expat_processingInstructionHandler);
	XML_SetCommentHandler(mParser, expat_commentHandler);
	XML_SetCdataSectionHandler(mParser, expat_startCdataSectionHandler, expat_endCdataSectionHandler);
	XML_SetDefaultHandlerExpand(mParser, expat_defaultHandler);
	XML_SetExternalEntityRefHandler(mParser, expat_externalEntityRefHandler);
	XML_SetSkippedEntityHandler(mParser, expat_skippedEntityHandler);
	XML_SetNamespaceDeclHandler(mParser, expat_startNamespaceDeclHandler, expat_endNamespaceDeclHandler);
	XML_SetXmlDeclHandler(mParser, expat_xmlDeclHandler);
	XML_SetDoctypeDeclHandler(mParser, expat_startDoctypeDeclHandler, expat_endDoctypeDeclHandler);
	XML_SetElementDeclHandler(mParser, expat_elementDeclHandler);
	XML_SetAttlistDeclHandler(mParser, expat_attlistDeclHandler);
	XML_SetEntityDeclHandler(mParser, expat_entityDeclHandler);
	XML_SetNotationDeclHandler(mParser, expat_notationDeclHandler);
	XML_SetNotStandaloneHandler(mParser, expat_notStandaloneHandler);
}

Parser::Parser(const XML_Char *encoding)
{
	mParser = XML_ParserCreate(encoding);
	setExpatParserHandlers();
}

Parser::Parser(XML_Char separator, const XML_Char *encoding)
{
	mParser = XML_ParserCreateNS(encoding, separator);
	setExpatParserHandlers();
}

Parser::~Parser()
{
	XML_ParserFree(mParser);
}

void Parser::reset(const XML_Char *encoding)
{
	XML_ParserReset(mParser, encoding);
	setExpatParserHandlers();
}

void Parser::parseFile(ParseHandler &handler, std::istream &fileIn)
{
	// set ParserHandler dependant expat parser items
	XML_SetExternalEntityRefHandlerArg(mParser, &handler);
	XML_SetUnknownEncodingHandler(mParser, expat_unknownEncodingHandler, &handler);

	// set the handler instance
	XML_SetUserData(mParser, &handler);

	int isLast = 0;
	while(0 == isLast)
	{
		void *buf = XML_GetBuffer(mParser, 8192);
		if(0 == buf)
		{
			// error
			throw Error(XML_GetErrorCode(mParser));
		}

		fileIn.read((char*)buf, 8192);
		isLast = fileIn.eof();
		if(!XML_ParseBuffer(mParser, fileIn.gcount(), isLast))
		{
			// error
			throw Error(XML_GetErrorCode(mParser));
		}
	}
}

void Parser::parseFile(ParseHandler &handler, const char *path)
{
	std::ifstream fileIn(path);
	try { parseFile(handler, fileIn); }
	catch(...)
	{
		fileIn.close();
		throw;
	}

	fileIn.close();
}

}	// namespace Expat

