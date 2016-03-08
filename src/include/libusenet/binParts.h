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
#ifndef __BIN_PARTS_HEADER__
#define __BIN_PARTS_HEADER__

#include <memory>
#include <tuple>
#include <vector>
#include <regex>

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

using TypeRegexTuple = std::tuple<BinPartsFileType, std::regex>;

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

	std::vector<TypeRegexTuple> m_regex_types;
	std::regex m_regex_repair;
};

}	/* namespace NZB */

#endif	/* __BIN_PARTS_HEADER__ */

