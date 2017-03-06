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
#ifndef __NZB_PARSE_HEADER__
#define __NZB_PARSE_HEADER__

#include <libusenet/nzb.h>
#include <string>
#include <istream>

namespace NZB { namespace Parse {

FileCollection parse(std::istream& in);
FileCollection parse(const std::string& nzb_str);
FileCollection parseFile(const char *path);

} } // namespace NZB::Parse

#endif	/* __NZB_PARSE_HEADER__ */
