#ifndef __BIN_PARTS_HEADER__
#define __BIN_PARTS_HEADER__

#include <memory>
#include <tuple>
#include <vector>
#include <regex.h>

/* enum for the different binary file parts */
typedef enum {
	BINPARTS_NONE,
	BINPARTS_CONTENT,
	BINPARTS_RAR,
	BINPARTS_ZIP,
	BINPARTS_NFO,
	BINPARTS_NZB,
	BINPARTS_REPAIR_BLOCKS,
	BINPARTS_REPAIR_INDEX,
	BINPARTS_SFV,
	BINPARTS_MKV,
	BINPARTS_MP3,
	BINPARTS_COUNT,
} BinPartsFileType;

namespace NZB {

using PosixRegexPtr = std::unique_ptr<regex_t, void(*)(regex_t*)>;
using RegexFileTuple = std::tuple<BinPartsFileType, PosixRegexPtr>;

class BinPartsOracle
{
// construction
public:

	BinPartsOracle();
	~BinPartsOracle();

// operations
public:

	// get the known file type for the given file name, defaults to BINPARTS_NONE
	BinPartsFileType get_file_type(const char *str);

	// get the repair block count from the file name, so this is really the "claimed" block count
	int get_repair_block_count(const char *str);

// implementation
private:

	std::vector<RegexFileTuple> m_regex_files_ptrs;
	PosixRegexPtr m_regex_repair_ptr;
};

}	/* namespace NZB */

#endif	/* __BIN_PARTS_HEADER__ */

