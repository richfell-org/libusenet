/*
	libusenet NNTP/NZB tools

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#ifndef __EXPAT_PARSE_HEADER__
#define __EXPAT_PARSE_HEADER__

#include <istream>
#include <stdexcept>
#include <expat.h>

namespace Expat {

/*
*/
class Error
:	public std::runtime_error
{
public:

	Error(XML_Error code);
	Error(const Error&) = default;
	~Error() throw() {}

	XML_Error getCode() const { return mCode; }

	Error& operator =(const Error&) = default;

protected:

	XML_Error mCode;
};

/*
	XML parse handler class.
*/
class ParseHandler
{
// construction
public:

	ParseHandler();
	ParseHandler(const ParseHandler&) = delete;
	virtual ~ParseHandler();

// operations
public:

	/*
		A handler for start (and empty) tags. Attributes are passed to the start handler as a pointer to a vector
		of char pointers. Each attribute seen in a start (or empty) tag occupies 2 consecutive places in this
		vector: the attribute name followed by the attribute value. These pairs are terminated by a null pointer.

		Note that an empty tag generates a call to both start and end handlers (in that order).
	*/ 
	virtual void startElement(const XML_Char *name, const XML_Char **attrs);

	/*
 		A handler for end (and empty) tags. As noted above, an empty tag generates a call to both start and end handlers.
	*/ 
	virtual void endElement(const XML_Char *name);

	/*
 		A text handler. The string your handler receives is NOT nul-terminated. You have to use the length
		argument to deal with the end of the string. A single block of contiguous text free of markup may
		still result in a sequence of calls to this handler. In other words, if you're searching for a pattern
		in the text, it may be split across calls to this handler. Note: Setting this handler to NULL may NOT
		immediately terminate call-backs if the parser is currently processing such a single block of contiguous
		markup-free text, as the parser will continue calling back until the end of the block is reached.
	*/ 
	virtual void characterData(const XML_Char *s, int len);

	/*
 		A handler for processing instructions. The target is the first word in the processing instruction. The
		data is the rest of the characters in it after skipping all whitespace after the initial word.
	*/ 
	virtual void processingInstruction(const XML_Char *target, const XML_Char *data);

	/*
 		A handler for comments. The data is all text inside the comment delimiters.
	*/ 
	virtual void comment(const XML_Char *data);

	/*
 		A handler that gets called at the beginning of a CDATA section.
	*/ 
	virtual void startCdataSection();

	/*
 		A handler that gets called at the end of a CDATA section.
	*/ 
	virtual void endCdataSection();

	/*
 		A handler for any characters in the document which wouldn't otherwise be handled. This includes both data
		for which no handlers can be set (like some kinds of DTD declarations) and data which could be reported
		but which currently has no handler set. The characters are passed exactly as they were present in the XML
		document except that they will be encoded in UTF-8 or UTF-16. Line boundaries are not normalized. Note
		that a byte order mark character is not passed to the default handler. There are no guarantees about how
		characters are divided between calls to the default handler: for example, a comment might be split between
		multiple calls. Setting the handler with this call has the side effect of turning off expansion of references
		to internally defined general entities. Instead these references are passed to the default handler.
	*/
	virtual void defaultHandler(const XML_Char *s, int len);

	/*
 		Set an external entity reference handler. This handler is also called for processing an external DTD
		subset if parameter entity parsing is in effect. (See XML_SetParamEntityParsing.)

		The context parameter specifies the parsing context in the format expected by the context argument to
		XML_ExternalEntityParserCreate. code is valid only until the handler returns, so if the referenced entity
		is to be parsed later, it must be copied. context is NULL only when the entity is a parameter entity,
		which is how one can differentiate between general and parameter entities.

		The base parameter is the base to use for relative system identifiers. It is set by XML_SetBase and may
		be NULL. The publicId parameter is the public id given in the entity declaration and may be NULL. systemId
		is the system identifier specified in the entity declaration and is never NULL.

		There are a couple of ways in which this handler differs from others. First, this handler returns a status
		indicator (an integer). XML_STATUS_OK should be returned for successful handling of the external entity
		reference. Returning XML_STATUS_ERROR indicates failure, and causes the calling parser to return an
		XML_ERROR_EXTERNAL_ENTITY_HANDLING error.

		Second, instead of having the user data as its first argument, it receives the parser that encountered
		the entity reference. This, along with the context parameter, may be used as arguments to a call to
		XML_ExternalEntityParserCreate. Using the returned parser, the body of the external entity can be
		recursively parsed.

		Since this handler may be called recursively, it should not be saving information into global or static variables.
	*/ 
	virtual int externalEntityRef(
		const XML_Char *context,
		const XML_Char *base,
		const XML_Char *systemId,
		const XML_Char *publicId);

	/*
		A skipped entity handler. This is called in two situations:

		1. An entity reference is encountered for which no declaration has been read and this is not an error.
		2. An internal entity reference is read, but not expanded, because XML_SetDefaultHandler has been called.

		The is_parameter_entity argument will be non-zero for a parameter entity and zero for a general entity.

		Note: skipped parameter entities in declarations and skipped general entities in attribute values cannot
		be reported, because the event would be out of sync with the reporting of the declarations or attribute
		values
	*/ 
	virtual void skippedEntity(const XML_Char *entityName, int is_parameter_entity);

	/*
		A handler to deal with encodings other than the built in set.

		If the handler knows how to deal with an encoding with the given name, it should fill in the info data
		structure and return XML_STATUS_OK. Otherwise it should return XML_STATUS_ERROR. The handler will be
		called at most once per parsed (external) entity.

		The map array contains information for every possible possible leading byte in a byte sequence. If the
		corresponding value is >= 0, then it's a single byte sequence and the byte encodes that Unicode value.
		If the value is -1, then that byte is invalid as the initial byte in a sequence. If the value is -n,
		where n is an integer > 1, then n is the number of bytes in the sequence and the actual conversion is
		accomplished by a call to the function pointed at by convert. This function may return -1 if the sequence
		itself is invalid. The convert pointer may be null if there are only single byte codes. The data parameter
		passed to the convert function is the data pointer from XML_Encoding. The string s is NOT nul-terminated
		and points at the sequence of bytes to be converted.

		The function pointed at by release is called by the parser when it is finished with the encoding. It may be NULL.

		typedef struct {
			int map[256];
			void *data;
			int (XMLCALL *convert)(void *data, const char *s);
			void (XMLCALL *release)(void *data);
		} XML_Encoding;
 	*/
	virtual int unknownEncoding(const XML_Char *name,  XML_Encoding *info);


	/*
 		A handler to be called when a namespace is declared. Namespace declarations occur inside start tags.
		But the namespace declaration start handler is called before the start tag handler for each namespace
		declared in that start tag.
	*/ 
	virtual void startNamespaceDecl(const XML_Char *prefix, const XML_Char *uri);

	/*
 		Set a handler to be called when leaving the scope of a namespace declaration. This will be called, for
		each namespace declaration, after the handler for the end tag of the element in which the namespace was
		declared.
	*/ 
	virtual void endNamespaceDecl(const XML_Char *prefix);


	/*
 		A handler that is called for XML declarations and also for text declarations discovered in external
		entities. The way to distinguish is that the version parameter will be NULL for text declarations. The
		encoding parameter may be NULL for an XML declaration. The standalone argument will contain -1, 0, or 1
		indicating respectively that there was no standalone parameter in the declaration, that it was given as
		no, or that it was given as yes.
	*/ 
	virtual void xmlDecl(const XML_Char  *version, const XML_Char  *encoding, int standalone);

	/*
 		A handler that is called at the start of a DOCTYPE declaration, before any external or internal subset
		is parsed. Both sysid and pubid may be NULL. The has_internal_subset will be non-zero if the DOCTYPE
		declaration has an internal subset.
	*/ 
	virtual void startDocType(const XML_Char *doctypeName, const XML_Char *sysid, const XML_Char *pubid, int has_internal_subset);

	/*
 		A handler that is called at the end of a DOCTYPE declaration, after parsing any external subset.
	*/ 
	virtual void endDocType();

	/*
		A handler for element declarations in a DTD. The handler gets called with the name of the element
		in the declaration and a pointer to a structure that contains the element model. It is the application's
		responsibility to free this data structure using XML_FreeContentModel.

		The model argument is the root of a tree of XML_Content nodes. If type equals XML_CTYPE_EMPTY or
		XML_CTYPE_ANY, then quant will be XML_CQUANT_NONE, and the other fields will be zero or NULL. If type is
		XML_CTYPE_MIXED, then quant will be XML_CQUANT_NONE or XML_CQUANT_REP and numchildren will contain the
		number of elements that are allowed to be mixed in and children points to an array of XML_Content structures
		that will all have type XML_CTYPE_NAME with no quantification. Only the root node can be type XML_CTYPE_EMPTY,
		XML_CTYPE_ANY, or XML_CTYPE_MIXED.

		For type XML_CTYPE_NAME, the name field points to the name and the numchildren and children fields will
		be zero and NULL. The quant field will indicate any quantifiers placed on the name.

		Types XML_CTYPE_CHOICE and XML_CTYPE_SEQ indicate a choice or sequence respectively. The numchildren field
		indicates how many nodes in the choice or sequence and children points to the nodes.

		enum XML_Content_Type {
			XML_CTYPE_EMPTY = 1,
			XML_CTYPE_ANY,
			XML_CTYPE_MIXED,
			XML_CTYPE_NAME,
			XML_CTYPE_CHOICE,
			XML_CTYPE_SEQ
		};

		enum XML_Content_Quant {
			XML_CQUANT_NONE,
			XML_CQUANT_OPT,
			XML_CQUANT_REP,
			XML_CQUANT_PLUS
		};

		typedef struct XML_cp XML_Content;

		struct XML_cp {
			enum XML_Content_Type		type;
			enum XML_Content_Quant	quant;
			const XML_Char *		name;
			unsigned int			numchildren;
			XML_Content *			children;
		};
 	*/
	virtual void elementDecl(const XML_Char *name, XML_Content *model);

	/*
 		A handler for attlist declarations in the DTD. This handler is called for each attribute. So a single
		attlist declaration with multiple attributes declared will generate multiple calls to this handler. The
		elname parameter returns the name of the element for which the attribute is being declared. The attribute
		name is in the attname parameter. The attribute type is in the att_type parameter. It is the string
		representing the type in the declaration with whitespace removed.

		The dflt parameter holds the default value. It will be NULL in the case of "#IMPLIED" or "#REQUIRED"
		attributes. You can distinguish these two cases by checking the isrequired parameter, which will be true
		in the case of "#REQUIRED" attributes. Attributes which are "#FIXED" will have also have a true isrequired,
		but they will have the non-NULL fixed value in the dflt parameter.
 	*/
	virtual void attlistDecl(
		const XML_Char *elname,
		const XML_Char *attname,
		const XML_Char *att_type,
		const XML_Char *dflt,
		int isrequired);

	/*
 		A handler that will be called for all entity declarations. The is_parameter_entity argument will be
		non-zero in the case of parameter entities and zero otherwise.

		For internal entities (<!ENTITY foo "bar">), value will be non-NULL and systemId, publicId, and notationName
		will all be NULL. The value string is not NULL terminated; the length is provided in the value_length
		parameter. Do not use value_length to test for internal entities, since it is legal to have zero-length
		values. Instead check for whether or not value is NULL.

		The notationName argument will have a non-NULL value only for unparsed entity declarations.
 	*/
	virtual void entityDecl(
		const XML_Char *entityName,
		int is_parameter_entity,
		const XML_Char *value,
		int value_length,
		const XML_Char *base,
		const XML_Char *systemId,
		const XML_Char *publicId,
		const XML_Char *notationName);

	/*
 		A handler that receives notation declarations.
 	*/
	virtual void notationDecl(
		const XML_Char *notationName,
		const XML_Char *base,
		const XML_Char *systemId,
		const XML_Char *publicId);

	/*
 		A handler that is called if the document is not "standalone". This happens when there is an external
		subset or a reference to a parameter entity, but does not have standalone set to "yes" in an XML declaration.
		If this handler returns XML_STATUS_ERROR, then the parser will throw an XML_ERROR_NOT_STANDALONE error.
	*/
	virtual int notStandalone();

	ParseHandler& operator =(const ParseHandler&) = delete;
};

/*
*/
class Parser
{
public:

	Parser(const XML_Char *encoding = NULL);
	Parser(XML_Char separator, const XML_Char *encoding = NULL);
	Parser(const Parser&) = delete;
	virtual ~Parser();

// operations
public:

	void reset(const XML_Char *encoding = NULL);

	void parseFile(ParseHandler &handler, const char *path);
	void parseFile(ParseHandler &handler, std::istream &fileIn);

	Parser& operator =(const Parser&) = delete;

protected:

	XML_Parser mParser;

private:

	void setExpatParserHandlers();
};

}	// namespace Expat

#endif		// __EXPAT_PARSE_HEADER__

