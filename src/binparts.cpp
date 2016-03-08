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
#include <sys/types.h>
#include <libusenet/binParts.h>

namespace NZB {

BinPartsOracle::BinPartsOracle()
:   m_regex_repair(".*vol[[:digit:]]+\\+([[:digit:]]+)\\.par2.*", std::regex::ECMAScript|std::regex::icase)
{
	const std::regex::flag_type re_flags = std::regex::ECMAScript|std::regex::icase|std::regex::nosubs;

	m_regex_types.emplace_back(BINPARTS_REPAIR_INDEX, std::regex(".*\\.par2[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_RAR, std::regex(".*\\.rar[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_RAR, std::regex(".*\\.[rs][[:digit:]]+[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_ZIP, std::regex(".*\\.zip[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_CONTENT, std::regex(".*\\.[[:digit:]]+[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_NFO, std::regex(".*\\.nfo[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_NZB, std::regex(".*\\.nzb[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_SFV, std::regex(".*\\.sfv[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_MKV, std::regex(".*\\.mkv[ \"].*", re_flags));
	m_regex_types.emplace_back(BINPARTS_MP3, std::regex(".*\\.mp3[ \"].*", re_flags));
}

BinPartsOracle::~BinPartsOracle()
{
}

BinPartsFileType BinPartsOracle::get_file_type(const char *str)
{
	BinPartsFileType result = BINPARTS_NONE;

	// check for PAR2 repair blocks file
	if(std::regex_match(str, m_regex_repair))
		result = BINPARTS_REPAIR_BLOCKS;
	else
	{
		for(auto& tuple : m_regex_types)
		{
			if(std::regex_match(str, std::get<1>(tuple)))
			{
				result = std::get<0>(tuple);
				break;
			}	
		}
	}

	return result;
}

int BinPartsOracle::get_repair_block_count(const char *str)
{
	int result = 0;

	std::cmatch cm;
	if(std::regex_match(str, cm, m_regex_repair))
	{
		if(cm.size() > 1)
		{
			std::string blocks_text = cm[1];
			for(auto& c : blocks_text)
			{
				result *= 10;
				result += c - '0';
			}
		}
	}

	return result;
}

}	/* namespace NZB */
