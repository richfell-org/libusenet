#ifndef __NZB_HEADER__
#define __NZB_HEADER__

#include <string>
#include <ctime>

namespace NZB {

class File;
class Segment;
class Group;

/*
*/
class FileCollection
{
// construction
public:

	FileCollection() : mpMem(0), mFileCount(0) {}
	FileCollection(int fileCount, int segmentCount, int groupCount);
	FileCollection(FileCollection&& rvCollection);
	FileCollection(const FileCollection&) = delete;
	~FileCollection();

// attributes
public:

	int getFileCount() const { return mFileCount; }

	File *getFiles() { return (File*)mpMem; }
	const File * getFile() const { return (File*)mpMem; }

// operations
public:

	File& operator [](int iFile);
	const File& operator [](int iFile) const;

	File* begin();
	File* end();
	const File* begin() const;
	const File* end() const;

	void alloc(int fileCount, int segmentCount, int groupCount);
	void free();

	FileCollection& operator =(FileCollection&& rvCollection);
	FileCollection& operator =(const FileCollection&) = delete;

protected:

	void *mpMem;
	int mFileCount;
};

/*
*/
class File
{
public:
	File() : mTimestamp(0), mSubject(), mPoster(), mSegmentCount(0), mpSegments(0), mGroupCount(0), mpGroups(0) {}
	File(const char *subject, const char *poster, time_t timestamp);
	File(int segmentCount, Segment *pSegments, int groupCount, Group *pGroups);
	File(File&& rvFile);
	~File();

	time_t getTimestamp() const { return mTimestamp; }

	std::string &getSubject() { return mSubject; }
	const std::string &getSubject() const { return mSubject; }

	std::string &getPoster() { return mPoster; }
	const std::string &getPoster() const { return mPoster; }

	int getSegmentCount() const { return mSegmentCount; }

	Segment &getSegment(int iSegment);
	const Segment &getSegment(int iSegment) const;
	void setSegments(int count, Segment *pSegments);

	int getGroupCount() const { return mGroupCount; }

	Group &getGroup(int iGroup);
	const Group &getGroup(int iGroup) const;
	void setGroups(int count, Group *pGroups);

	File& operator =(File&& rvFile);

protected:

	time_t mTimestamp;
	std::string mSubject;
	std::string mPoster;

	int mSegmentCount;
	Segment *mpSegments;

	int mGroupCount;
	Group *mpGroups;
};

/*
*/
class Segment
{
public:

	Segment() : mNumber(-1), mByteCount(0), mMessageId() {}
	Segment(int number, long byteCount);
	Segment(int number, long byteCount, const std::string &msgId);
	Segment(const Segment&) = default;
	Segment(Segment&&) = default;
	~Segment() {}

// attributes
public:

	int getNumber() const { return mNumber; }
	void setNumber(int number) { mNumber = number; }

	long getByteCount() const { return mByteCount; }
	void setByteCount(long byteCount) { mByteCount = byteCount; }

	std::string &getMessageId() { return mMessageId; }
	const std::string &getMessageId() const { return mMessageId; }

// operations
public:

	Segment& operator =(Segment&& rvSegment) = default;
	Segment& operator =(const Segment& rvSegment) = default;

protected:

	int mNumber;
	long mByteCount;
	std::string mMessageId;
};

/*
*/
class Group
{
// construction
public:

	Group() : mName() {}
	Group(const char *name) : mName(name) {}
	Group(const std::string &name) : mName(name) {}
	Group(const Group&) = default;
	Group(Group&) = default;
	~Group() {}

// attributes
public:

	std::string &getName() { return mName; }
	const std::string &getName() const { return mName; }

// operations
public:

	Group& operator =(const Group&) = default;
	Group& operator =(Group&&) = default;

// implementation
protected:

	std::string	mName;
};

}	// namespace NZB

#endif	/* __NZB_HEADER__ */
