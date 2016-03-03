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
:	m_regex_repair_ptr(0, regfree)
{
	m_regex_files_ptrs.clear();

	regex_t *p_regex = new regex_t;
	regcomp(p_regex, ".*\\.par2[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_REPAIR_INDEX, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.rar[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_RAR, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.[rs][[:digit:]]+[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_RAR, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.zip[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_ZIP, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.[[:digit:]]+[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_CONTENT, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.nfo[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_NFO, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.nzb[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_NZB, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.sfv[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_SFV, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.mkv[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_MKV, PosixRegexPtr(p_regex, regfree)));

	p_regex = new regex_t;
	regcomp(p_regex, ".*\\.mp3[ \"].*", REG_EXTENDED|REG_ICASE|REG_NOSUB);
	m_regex_files_ptrs.push_back(RegexFileTuple(BINPARTS_MP3, PosixRegexPtr(p_regex, regfree)));

	// BINPARTS_REPAIR_BLOCKS
	p_regex = new regex_t;
	regcomp(p_regex, ".*vol[[:digit:]]+\\+([[:digit:]]+)\\.par2.*", REG_EXTENDED|REG_ICASE);
	m_regex_repair_ptr.reset(p_regex);
}

BinPartsOracle::~BinPartsOracle()
{
}

BinPartsFileType BinPartsOracle::get_file_type(const char *str)
{
	BinPartsFileType result = BINPARTS_NONE;
	regmatch_t pmatch[2];

	if(0 == regexec(m_regex_repair_ptr.get(), str, 2, pmatch, 0))
		result = BINPARTS_REPAIR_BLOCKS;
	else
	{	
		for(auto& tuple : m_regex_files_ptrs)
		{
			PosixRegexPtr& regex_ptr = std::get<1>(tuple);
			if(0 == regexec(regex_ptr.get(), str, 2, pmatch, 0))
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
	regmatch_t pmatch[2];

	if(0 == regexec(m_regex_repair_ptr.get(), str, 2, pmatch, 0))
	{
		/* pmatch[0] is for the whole string */
		/* fprintf(stderr, "binPartsRepairBlockCount: match at index %d\n", pmatch[1].rm_so); */

		int iDigit;
		for(iDigit = pmatch[1].rm_so; iDigit < pmatch[1].rm_eo; ++iDigit)
		{
			if(result > 0) result *= 10;

			result += str[iDigit] - '0';
		}
	}

	return result;
}

}	/* namespace NZB */
