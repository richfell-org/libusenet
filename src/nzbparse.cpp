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
#include "expatParse.h"
#include <libusenet/nzb.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

namespace NZB { namespace Parse {

// tag attribute names
static const char elname_file[] = "file";
static const char elname_group[] = "group";
static const char elname_segment[] = "segment";
static const char attrname_poster[] = "poster";
static const char attrname_date[] = "date";
static const char attrname_subject[] = "subject";
static const char attrname_bytes[] = "bytes";
static const char attrname_number[] = "number";

/*
	Pass 1 handler counts the number of each item in the NZB file
*/
class Pass1ParseHandler
:	public Expat::ParseHandler
{
public:

	Pass1ParseHandler();
	~Pass1ParseHandler() {}

	int getSegmentCount() const { return mSegmentCount; }
	int getGroupCount() const { return mGroupCount; }
	int getFileCount() const { return mFileCount; }

	void endElement(const XML_Char *name);

protected:

	int mSegmentCount;
	int mGroupCount;
	int mFileCount;
};

Pass1ParseHandler::Pass1ParseHandler()
:	mSegmentCount(0), mGroupCount(0), mFileCount(0)
{
}

void Pass1ParseHandler::endElement(const XML_Char *name)
{
	if(0 == strcmp(elname_segment, name))
		++mSegmentCount;
	else if(0 == strcmp(elname_group, name))
		++mGroupCount;
	else if(0 == strcmp(elname_file, name))
		++mFileCount;
}

/*
	Pass 2 handler reads the data from the NZB file into pre-allocated objects
*/
class Pass2ParseHandler
:	public Expat::ParseHandler
{
public:
	Pass2ParseHandler(NZB::File *pFiles, NZB::Segment *pSegments, NZB::Group *pGroups);
	~Pass2ParseHandler() {}

	void startElement(const XML_Char *name, const XML_Char **attr);
	void endElement(const XML_Char *name);
	void characterData(const XML_Char *s, int len);

protected:

	enum NzbElem { NONE, SEGMENT, GROUP, NZBFILE, };

	NzbElem mCurElem;

	NZB::File *mpFiles;
	NZB::Segment *mpSegments;
	NZB::Group *mpGroups;
	int miFile;
	int miSegment, miSegmentBegin;
	int miGroup, miGroupBegin;
	std::string mChars;

private:
	Pass2ParseHandler(const Pass2ParseHandler &that);
};

Pass2ParseHandler::Pass2ParseHandler(NZB::File *pFiles, NZB::Segment *pSegments, NZB::Group *pGroups)
:	mCurElem(NONE), mpFiles(pFiles), mpSegments(pSegments), mpGroups(pGroups),
	miFile(0), miSegment(0), miSegmentBegin(0), miGroup(0), miGroupBegin(0), mChars()
{
	mChars.reserve(1024);
}

void Pass2ParseHandler::startElement(const XML_Char *name, const XML_Char **attr)
{
	// segment
	if(0 == strcmp(elname_segment, name))
	{
		mCurElem = SEGMENT;
		long byteCount = 0;
		int number = -1;
		for(; 0 != *attr; ++attr)
		{
			if(0 == strcmp(attrname_bytes, *attr))
				byteCount = strtol(*++attr, 0, 0);
			else if(0 == strcmp(attrname_number, *attr))
				number = strtol(*++attr, 0, 0);
			else
				++attr;	// skip past the value for the unknown attr name
		}
		new(&mpSegments[miSegment]) NZB::Segment(number, byteCount);
	}
	// group
	else if(0 == strcmp(elname_group, name))
	{
		mCurElem = GROUP;
		new(&mpGroups[miGroup]) NZB::Group();
	}
	else if(0 == strcmp(elname_file, name))
	{
		mCurElem = NZBFILE;
		const char *poster = 0;
		const char *subject = 0;
		time_t time = 0;
		for(; 0 != *attr; ++attr)
		{
			if(0 == strcmp(attrname_poster, *attr))
				poster = *++attr;
			else if(0 == strcmp(attrname_subject, *attr))
				subject = *++attr;
			else if(0 == strcmp(attrname_date, *attr))
				time = strtoul(*++attr, 0, 0);
			else
				++attr;	// skip past the value for the unknown attr name
		}
		new(&mpFiles[miFile]) NZB::File(subject, poster, time);
	}
}

void Pass2ParseHandler::endElement(const XML_Char *name)
{
	if(0 == strcmp(elname_segment, name))
	{
		++miSegment;
		mCurElem = NZBFILE;
	}
	else if(0 == strcmp(elname_group, name))
	{
		++miGroup;
		mCurElem = NZBFILE;
	}
	else if(0 == strcmp(elname_file, name))
	{
		// the segment and group indicies have already been incremented to the next "free" slots
		mpFiles[miFile].setSegments(miSegment - miSegmentBegin, &mpSegments[miSegmentBegin]);
		mpFiles[miFile].setGroups(miGroup - miGroupBegin, &mpGroups[miGroupBegin]);
		miSegmentBegin = miSegment;
		miGroupBegin = miGroup;
		++miFile;
		mCurElem = NONE;
	}
}

void Pass2ParseHandler::characterData(const XML_Char *s, int len)
{
	switch(mCurElem)
	{
		case SEGMENT:
			mpSegments[miSegment].getMessageId().append(s, len);
			break;
		case GROUP:
			mpGroups[miGroup].getName().append(s, len);
			break;
		default:
			break;
	}
}

FileCollection parse(std::istream& in)
{
	Expat::Parser parser;
	Pass1ParseHandler pass1Handler;

	// pass 1: count the NZB entities and allocate memory based on the counts
	parser.parseFile(pass1Handler, in);
	FileCollection fileCollection(
		pass1Handler.getFileCount(),
		pass1Handler.getSegmentCount(),
		pass1Handler.getGroupCount());

	// point to the allocated memory so pass 2 can load the content
	NZB::File *pFiles = fileCollection.getFiles();
	NZB::Segment *pSegments = (NZB::Segment*)&pFiles[pass1Handler.getFileCount()];
	NZB::Group *pGroups = (NZB::Group*)&pSegments[pass1Handler.getSegmentCount()];

	// pass 2: parse content of <file ... />, <segment ... /> and <group ... /> tags
	in.clear();
	in.seekg(0);
	parser.reset();
	Pass2ParseHandler pass2Handler(pFiles, pSegments, pGroups);
	parser.parseFile(pass2Handler, in);

	return fileCollection;
}

FileCollection parse(const std::string& nzb_str)
{
	std::istringstream in(nzb_str);
	return parse(in);
}

FileCollection parseFile(const char *path)
{
	std::ifstream fileIn(path);
	return parse(fileIn);
}

} } // namespace NZB::Parse

