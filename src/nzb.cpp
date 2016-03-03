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
#include <libusenet/nzb.h>
#include <stdexcept>
#include <utility>

namespace NZB {

FileCollection::FileCollection(int fileCount, int segmentCount, int groupCount)
:	mpMem(0), mFileCount(0)
{
	alloc(fileCount, segmentCount, groupCount);
}

FileCollection::FileCollection(FileCollection&& rvCollection)
:	mpMem(rvCollection.mpMem), mFileCount(rvCollection.mFileCount)
{
	rvCollection.mpMem = nullptr;
	rvCollection.mFileCount = 0;
}

FileCollection& FileCollection::operator =(FileCollection&& rvCollection)
{
	// implement a swap and the transient instance can free the
	// existing (if any) allocated memory from this instance
	std::swap(mpMem, rvCollection.mpMem);
	std::swap(mFileCount, rvCollection.mFileCount);

	return *this;
}

FileCollection::~FileCollection()
{
	free();
}

File& FileCollection::operator [](int iFile)
{
	if((0 > iFile) || (mFileCount <= iFile))
		throw std::range_error("FileCollection::operator []: request file index is out of range");
	return ((File*)mpMem)[iFile];
}

const File& FileCollection::operator [](int iFile) const
{
	if((0 > iFile) || (mFileCount <= iFile))
		throw std::range_error("FileCollection::operator []: request file index is out of range");
	return ((File*)mpMem)[iFile];
}

File* FileCollection::begin()
{
	if((0 == mFileCount) && (nullptr == mpMem))
		return nullptr;
	return (File*)mpMem;
}

File* FileCollection::end()
{
	if(nullptr == mpMem)
		return nullptr;
	return &((File*)mpMem)[mFileCount];
}

const File* FileCollection::begin() const
{
	if((0 == mFileCount) && (nullptr == mpMem))
		return nullptr;
	return (const File*)mpMem;
}

const File* FileCollection::end() const
{
	if(nullptr == mpMem)
		return nullptr;
	return &((const File*)mpMem)[mFileCount];
}

void FileCollection::alloc(int fileCount, int segmentCount, int groupCount)
{
	int memlen = (fileCount * sizeof(File)) + (segmentCount * sizeof(Segment)) + (groupCount * sizeof(Group));
	mpMem = new unsigned char[memlen];
	mFileCount = fileCount;
}

void FileCollection::free()
{
	if(0 != mpMem)
	{
		File *pFile = (File*)mpMem;
		for(int i = 0; i < mFileCount; ++i)
			pFile[i].~File();

		delete[] (unsigned char*)mpMem;
		mpMem = 0;
		mFileCount = 0;
	}
}

File::File(const char *subject, const char *poster, time_t timestamp)
:	mTimestamp(timestamp), mSubject(subject), mPoster(poster)
{
}

File::File(int segmentCount, Segment *pSegments, int groupCount, Group *pGroups)
:	mSegmentCount(segmentCount), mpSegments(pSegments), mGroupCount(groupCount), mpGroups(pGroups)
{
}

File::File(File&& rvFile)
:	mSegmentCount(rvFile.mSegmentCount), mpSegments(rvFile.mpSegments),
	mGroupCount(rvFile.mGroupCount), mpGroups(rvFile.mpGroups)
{
	rvFile.mSegmentCount = 0;
	rvFile.mpSegments = nullptr;
	rvFile.mGroupCount = 0;
	rvFile.mpGroups = nullptr;
}

File& File::operator =(File&& rvFile)
{
	mSegmentCount = rvFile.mSegmentCount;
	mpSegments = rvFile.mpSegments;
	mGroupCount = rvFile.mGroupCount;
	mpGroups = rvFile.mpGroups;
	
	rvFile.mSegmentCount = 0;
	rvFile.mpSegments = nullptr;
	rvFile.mGroupCount = 0;
	rvFile.mpGroups = nullptr;

	return *this;
}

File::~File()
{
	for(int i = 0; i < mSegmentCount; ++i)
		mpSegments[i].~Segment();
	for(int i = 0; i < mGroupCount; ++i)
		mpGroups[i].~Group();
}

Segment &File::getSegment(int iSegment)
{
	return mpSegments[iSegment];
}

const Segment &File::getSegment(int iSegment) const
{
	return mpSegments[iSegment];
}

void File::setSegments(int count, Segment *pSegments)
{
	mSegmentCount = count;
	mpSegments = pSegments;
}

Group &File::getGroup(int iGroup)
{
	return mpGroups[iGroup];
}

const Group &File::getGroup(int iGroup) const
{
	return mpGroups[iGroup];
}

void File::setGroups(int count, Group *pGroups)
{
	mGroupCount = count;
	mpGroups = pGroups;
}

Segment::Segment(int number, long byteCount, const std::string &msgId)
:	mNumber(number), mByteCount(byteCount), mMessageId(msgId)
{
}

Segment::Segment(int number, long byteCount)
:	mNumber(number), mByteCount(byteCount), mMessageId()
{
}

}	// namespace NZB
