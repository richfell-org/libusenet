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
#ifndef __YENC_HEADER__
#define __YENC_HEADER__

#include <stdexcept>
#include <functional>
#include <bitset>
#include <string>
#include <iostream>
#include "crc32.h"

namespace yEnc {

// encode from the file given in filename to the given output stream
// the name keyword of =ybegin header is set to the value of filename
void encode_file(const std::string& path, std::ostream& out, int line_size = 128)
	throw(std::runtime_error);

void encode_file_for_usenet(const std::string& path, std::ostream& out, int line_size/* = 128*/)
	throw(std::runtime_error);

void encode_file(
	const std::string& path,
	std::ostream& out,
	std::ostream& (*end_line)(std::ostream&),
	int line_size = 128)
throw(std::runtime_error);

void encode_file_for_usenet(
	const std::string& path,
	std::ostream& out,
	std::ostream& (*end_line)(std::ostream&),
	int line_size = 128)
throw(std::runtime_error);

/*
 *
 */
class Encoder
{
// construction
public:

	Encoder(std::ostream& (*end_line)(std::ostream&) = std::endl<char, std::char_traits<char>>)
		:	m_line_size(128), m_kw_name("a.out"), m_part(0), m_trailer_size(0), m_line_col(0), m_endl(end_line) {}
	Encoder(const char *kw_name, int line_size = 128)
		:	m_line_size(line_size), m_kw_name(kw_name), m_part(0),
			m_trailer_size(0), m_line_col(0), m_endl(std::endl<char, std::char_traits<char>>) {}
	Encoder(
		const char *kw_name,
		std::ostream& (*end_line)(std::ostream&) = std::endl<char, std::char_traits<char>>,
		int line_size = 128)
		:	m_line_size(line_size), m_kw_name(kw_name), m_part(0),
			m_trailer_size(0), m_line_col(0), m_endl(end_line) {}
	~Encoder() {}

// attributes
public:

	void set_endl(std::ostream& (*end_line)(std::ostream&));

// operations
public:

	void encode_init(
		std::ostream& out,
		unsigned long total_size,
		unsigned int part_num = 0,
		unsigned long part_start = 0,
		unsigned long part_end = 0,
		unsigned int parts_total = 0);

	void encode_chunk(std::ostream& out, void *p_bytes, unsigned int len);
	void encode_usenet_chunk(std::ostream& out, void *p_bytes, unsigned int len);

	void encode_close(std::ostream& out);
	void encode_close(std::ostream& out, unsigned int total_crc32);

// implementation
protected:

	// header/trailer info
	int m_line_size;
	std::string m_kw_name;
	unsigned int m_part;
	unsigned long m_trailer_size;

	int m_line_col;

	// CRC calculator
	Crc32 m_crc32;

	std::ostream& (*m_endl)(std::ostream&);
};

enum class DecodeResult { YENC_NONE, YENC_DATA, YENC_COMPLETE, YENC_TRUNCATED, };

/*
 *
 */
class Decoder
{
// construction
public:

	Decoder();
	Decoder(const Decoder&) = default;
	Decoder(Decoder&&) = default;
	virtual ~Decoder();

// attributes
public:

	unsigned int get_lines() const { return m_line; }

	unsigned long get_header_size() const { return m_head_size; }
	unsigned long get_trailer_size() const { return m_trail_size; }

	unsigned int get_header_part() const { return m_head_part; }
	unsigned int get_trailer_part() const { return m_trail_part; }

	unsigned int get_crc32() const { return m_crc32; }

	const std::string& get_file_name() const { return m_name; }

	unsigned long get_part_begin() const { return m_part_begin; }
	unsigned long get_part_end() const { return m_part_end; }
	unsigned int get_part_crc32() const { return m_part_crc32; }

	unsigned int get_actual_crc32() const { return m_calc_crc32.get_value(); }

// operations
public:

	bool is_header() const { return m_part_flags[YENC_HEADER]; }
	bool is_part() const { return m_part_flags[YENC_PART_HEADER]; }
	bool is_trailer() const { return m_part_flags[YENC_TRAILER]; }
	bool is_crc32() const { return m_part_flags[YENC_CRC32]; }

	DecodeResult decode(unsigned char **ppMem, const char *encoded_line, int len);
	DecodeResult decode(unsigned char *pBuf, int *pLen, const char *encoded_line, int len);

	Decoder& operator =(const Decoder&) = default;
	Decoder& operator =(Decoder&&) = default;

// implementation
protected:

	bool is_kw_line(const char *encoded_line, int enclen);
	int decode_kw_line(const char *encoded_line, int enclen);

	// keyword values
	unsigned int m_line;
	unsigned long m_head_size;
	unsigned long m_trail_size;
	unsigned int m_head_part;
	unsigned int m_head_part_total;
	unsigned int m_trail_part;
	unsigned int m_crc32;
	std::string m_name;

	// keyword values for part decoding
	unsigned long m_part_begin;
	unsigned long m_part_end;
	unsigned int m_part_crc32;

	// decode bookkeeping
	Crc32 m_calc_crc32;

	// flags for indicating which yEnc items have been parsed
	enum { YENC_HEADER, YENC_TRAILER, YENC_PART_HEADER, YENC_CRC32, YENC_COUNT, };
	std::bitset<YENC_COUNT> m_part_flags;
};

}	/* namespace yEnc */

#endif	/* __YENC_HEADER__ */
