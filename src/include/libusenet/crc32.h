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
#ifndef __CRC32_HEADER__
#define __CRC32_HEADER__

#include <cstdint>

/*
 *
 */
class Crc32
{
// construction
public:

	Crc32();
	~Crc32() {}

// attributes
public:

	uint32_t get_value() const			{ return ~m_crc; }

// operations
public:

	void reset()						{ m_crc = uint32_t(~0); }
	void update_crc(uint8_t byte)		{ update_crc(&byte, 1); }
	void update_crc(const uint8_t *p_buf, unsigned int len);

// implementation
private:

	uint32_t m_crc;
};

#endif	/* __CRC32_HEADER__ */
