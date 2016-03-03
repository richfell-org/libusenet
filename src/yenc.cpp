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
#include <libusenet/yenc.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>

namespace yEnc {

static int yencFindHeaderToken(const char *input, int len, const char **ppTok, int *pLen)
{
	int i;
	for(i = 0; (i < len) && (' ' == input[i]); ++i)
		/* empty */;

	*ppTok = &input[i];
	for(++i; (i < len) && ('=' != input[i]) && (' ' != input[i]); ++i)
		/* empty */;
	*pLen = &input[i] - *ppTok;

	for(++i; (i < len) && (('=' == input[i]) || (' ' == input[i])); ++i)
		/* empty */;

	return i;
}

static char *yencReadULongValue(unsigned long *dest, const char *str)
{
	char *end = 0;
	*dest = (unsigned long)strtol(str, &end, 0);
	return end;
}

static char *yencReadUIntValue(unsigned int *dest, const char *str)
{
	char *end = 0;
	*dest = (unsigned int)strtol(str, &end, 0);
	return end;
}

static char *yencReadUInt16Value(unsigned int *dest, const char *str)
{
	char *end = 0;
	*dest = (unsigned int)strtol(str, &end, 16);
	return end;
}

static std::string get_filename(const std::string& path)
{
	register std::string::size_type pos =
#ifdef WIN32
		path.find_last_of('\\');
#else
		path.find_last_of('/');
#endif

	// return the portion of the string following
 	// the last path separator (if found)
	if(pos != std::string::npos)
 		return path.substr(pos + 1);

	// else just return the original path value
	return path;
}

#define IOBUF_LEN		(1024 * 8)

void encode_file(const std::string& path, std::ostream& out, int line_size/* = 128*/)
throw(std::runtime_error)
{
	std::ifstream in(path, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Cannot open file: " + path);

	// determine the size of the file
	unsigned long file_size = 0;
	in.seekg(0, in.end);
	file_size = in.tellg();
	in.seekg(0, in.beg);

	Encoder yEncoder(get_filename(path).c_str(), line_size);
	yEncoder.encode_init(out, file_size);

	register int rdsz;
	char iobuf[IOBUF_LEN];
	in.read(iobuf, IOBUF_LEN);
	while(0 < (rdsz = in.gcount()))
	{
		yEncoder.encode_chunk(out, iobuf, rdsz);

		// read next data chunk
		in.read(iobuf, IOBUF_LEN);
	}

	yEncoder.encode_close(out);
}


void encode_file_for_usenet(const std::string& path, std::ostream& out, int line_size/* = 128*/)
throw(std::runtime_error)
{
	std::ifstream in(path, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Cannot open file: " + path);

	// determine the size of the file
	unsigned long file_size = 0;
	in.seekg(0, in.end);
	file_size = in.tellg();
	in.seekg(0, in.beg);

	Encoder yEncoder(get_filename(path).c_str(), [](std::ostream& o) -> std::ostream& { o << "\r\n"; return o; }, line_size);
	yEncoder.encode_init(out, file_size);

	register int rdsz;
	char iobuf[IOBUF_LEN];
	in.read(iobuf, IOBUF_LEN);
	while(0 < (rdsz = in.gcount()))
	{
		yEncoder.encode_usenet_chunk(out, iobuf, rdsz);

		// read next data chunk
		in.read(iobuf, IOBUF_LEN);
	}

	yEncoder.encode_close(out);
}

void encode_file(
	const std::string& path,
	std::ostream& out,
	std::ostream& (*end_line)(std::ostream&),
	int line_size/* = 128*/)
throw(std::runtime_error)
{
	std::ifstream in(path, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Cannot open file: " + path);

	// determine the size of the file
	unsigned long file_size = 0;
	in.seekg(0, in.end);
	file_size = in.tellg();
	in.seekg(0, in.beg);

	Encoder yEncoder(get_filename(path).c_str(), end_line, line_size);
	yEncoder.encode_init(out, file_size);

	register int rdsz;
	char iobuf[IOBUF_LEN];
	in.read(iobuf, IOBUF_LEN);
	while(0 < (rdsz = in.gcount()))
	{
		yEncoder.encode_chunk(out, iobuf, rdsz);

		// read next data chunk
		in.read(iobuf, IOBUF_LEN);
	}

	yEncoder.encode_close(out);
}

void encode_file_for_usenet(
	const std::string& path,
	std::ostream& out,
	std::ostream& (*end_line)(std::ostream&),
	int line_size/* = 128*/)
throw(std::runtime_error)
{
	std::ifstream in(path, std::ifstream::binary);
	if(!in)
		throw std::runtime_error("Cannot open file: " + path);

	// determine the size of the file
	unsigned long file_size = 0;
	in.seekg(0, in.end);
	file_size = in.tellg();
	in.seekg(0, in.beg);

	Encoder yEncoder(get_filename(path).c_str(), end_line, line_size);
	yEncoder.encode_init(out, file_size);

	register int rdsz;
	char iobuf[IOBUF_LEN];
	in.read(iobuf, IOBUF_LEN);
	while(0 < (rdsz = in.gcount()))
	{
		yEncoder.encode_usenet_chunk(out, iobuf, rdsz);

		// read next data chunk
		in.read(iobuf, IOBUF_LEN);
	}

	yEncoder.encode_close(out);
}

void Encoder::set_endl(std::ostream& (*end_line)(std::ostream&))
{
	m_endl = end_line;
}

void Encoder::encode_init(
	std::ostream& out,
	unsigned long total_size,
	unsigned int part_num/* = 0*/,
	unsigned long part_start/* = 0*/,
	unsigned long part_end/* = 0*/,
	unsigned int parts_total/* = 0*/)
{
	// output the appropiate header(s)
	out << "=ybegin";
	if(part_num > 0)
	{
		out << " part=" << part_num;
		m_part = part_num;
	}
	if(parts_total > 0)
		out << " total=" << parts_total;
	out << " line=" << m_line_size << " size=" << total_size << " name=" << m_kw_name << m_endl;

	if(part_num > 0)
		out << "=ypart begin=" << part_start << " end=" << part_end << m_endl;

	m_trailer_size = 0;

	m_crc32.reset();
}

void Encoder::encode_chunk(std::ostream& out, void *p_bytes, unsigned int len)
{
	for(register unsigned int i = 0; i < len; ++i)
	{
		// encode the byte to yEnc and check for a critical character
		register char c = ((unsigned char*)p_bytes)[i] + 42;
		if(c == 0x00 || c == 0x0a || c == 0x0d || c == 0x3d)
		{
			// write the yEnc escape for the critical character
			out << '=';
			c = char(c + 64);
			++m_line_col;
		}

		// write the yEnc encoded char
		out << c;
		++m_line_col;

		// check the line size
		if(m_line_col >= m_line_size)
		{
			out << m_endl;
			m_line_col = 0;
		}
	}

	// calculate the CRC
	m_crc32.update_crc((uint8_t*)p_bytes, len);

	// update the size
	m_trailer_size += len;
}

void Encoder::encode_usenet_chunk(std::ostream& out, void *p_bytes, unsigned int len)
{
	for(register unsigned int i = 0; i < len; ++i)
	{
		// encode the byte to yEnc and check for a critical character
		register char c = ((unsigned char*)p_bytes)[i] + 42;
		if(c == 0x00 || c == 0x0a || c == 0x0d || c == 0x3d)
		{
			// write the yEnc escape for the critical character
			out << '=';
			c = char(c + 64);
			++m_line_col;
		}

		// write the yEnc encoded char
		out << c;
		++m_line_col;

		// double '.' chars if they are at the first position of a line (usenet)
		if((1 == m_line_col) && ('.' == c))
		{
			out << c;
			++m_line_col;
		}

		// check the line size
		if(m_line_col >= m_line_size)
		{
			out << m_endl;
			m_line_col = 0;
		}
	}

	// calculate the CRC
	m_crc32.update_crc((uint8_t*)p_bytes, len);

	// update the size
	m_trailer_size += len;
}

void Encoder::encode_close(std::ostream& out)
{
	if(0 != m_line_col)
		out << m_endl;

	out << "=yend size=" << m_trailer_size;
	if(m_part > 0)
		out << " part=" << m_part << " pcrc32=";
	else
		out << " crc32=";
	out << std::hex << m_crc32.get_value() << std::dec << m_endl;
}

void Encoder::encode_close(std::ostream& out, unsigned int total_crc32)
{
	if(0 != m_line_col)
		out << m_endl;

	out << "=yend size=" << m_trailer_size;
	if(m_part > 0)
		out << " part=" << m_part << " pcrc32=" << std::hex << m_crc32.get_value() << std::dec;
	out << " crc32=" << std::hex << total_crc32 << std::dec << m_endl;
}

Decoder::Decoder()
:	m_line(0), m_head_size(0), m_trail_size(0), m_head_part(0), m_trail_part(0), m_crc32(0), m_name(""),
	m_part_begin(0), m_part_end(0), m_part_crc32(0), m_calc_crc32(), m_part_flags(0)
{
}

Decoder::~Decoder()
{
}


bool Decoder::is_kw_line(const char *encoded_line, int enclen)
{
	bool result = false;
	if((0 != encoded_line) && (enclen >= 5) && (('=' == encoded_line[0]) && ('y' == encoded_line[1])))
	{
		if((0 == strncmp("end", &encoded_line[2], 3))
			|| (0 == strncmp("part", &encoded_line[2], 4))
			|| (0 == strncmp("begin ", &encoded_line[2], 6)))
		{
			result = true;
		}
	}
	return result;
}

/*
 */
int Decoder::decode_kw_line(const char *encoded_line, int enclen)
{
	int i;

	// weed-out test for a KW line
	if((0 == encoded_line) || !is_kw_line(encoded_line, enclen))
		return 0;

	// step past KW line name
	for(i = 0; (' ' != encoded_line[i]) && (i < enclen); ++i)
		/* empty */;

	while(i < enclen)
	{
		const char *token = 0;
		int toklen = 0;

		i += yencFindHeaderToken(&encoded_line[i], enclen - i, &token, &toklen);

		if(0 == strncmp("line", token, 4))
			i = yencReadUIntValue(&m_line, &encoded_line[i]) - encoded_line;
		else if(0 == strncmp("begin", token, toklen))
			i = yencReadULongValue(&m_part_begin, &encoded_line[i]) - encoded_line;
		else if(0 == strncmp("end", token, toklen))
			i = yencReadULongValue(&m_part_end, &encoded_line[i]) - encoded_line;
		else if(0 == strncmp("pcrc32", token, toklen))
			i = yencReadUInt16Value(&m_part_crc32, &encoded_line[i]) - encoded_line;
		else if(0 == strncmp("crc32", token, toklen))
		{
			i = yencReadUInt16Value(&m_crc32, &encoded_line[i]) - encoded_line;
			m_part_flags[YENC_CRC32] = true;
		}
		else if(0 == strncmp("part", token, 4))
		{
			unsigned int *pDest = (0 == m_head_part) ? &m_head_part : &m_trail_part;
			i = yencReadUIntValue(pDest, &encoded_line[i]) - encoded_line;
		}
		else if(0 == strncmp("size", token, 4))
		{
			unsigned long *pDest = (0 == m_head_size) ? &m_head_size : &m_trail_size;
			i = yencReadULongValue(pDest, &encoded_line[i]) - encoded_line;
		}
		else if(0 == strncmp("total", token, toklen))
			i = yencReadUIntValue(&m_head_part_total, &encoded_line[i]) - encoded_line;
		else if(0 == strncmp("name", token, 4))
		{
			int copylen;
			while(' ' == encoded_line[i])
				++i;
			copylen = enclen - i;
			m_name.assign(&encoded_line[i], copylen);
			for(--copylen; copylen >= 0; --copylen)
			{
				if(('\r' == m_name[copylen]) || ('\n' == m_name[copylen]))
					m_name.erase(copylen);
				else
					break;
			}

			// name must be the last keyword and the rest of the line is consumed
			i = enclen;
		}
	}

	if('b' == encoded_line[2]) m_part_flags[YENC_HEADER] = true;
	else if('p' == encoded_line[2]) m_part_flags[YENC_PART_HEADER] = true;
	else if('e' == encoded_line[2]) m_part_flags[YENC_TRAILER] = true;

	return 1;
}

DecodeResult Decoder::decode(unsigned char **ppMem, const char *encoded_line, int len)
{
	if((0 == ppMem) || (0 == encoded_line) || (0 == len))
		return DecodeResult::YENC_NONE;

	// check for a keyword line
	if(('=' == encoded_line[0]) && ('y' == encoded_line[1]))
	{
		decode_kw_line(encoded_line, len);
		return(m_part_flags[YENC_TRAILER] ? DecodeResult::YENC_COMPLETE : DecodeResult::YENC_NONE);
	}

	// decoded the header keyword line yet?
	if(m_line <= 0) return DecodeResult::YENC_NONE;

	uint8_t *p_buf = *ppMem;

	// decode a line of yenc encoded data
	for(register int i = 0; i < len; ++i)
	{
		if('=' != encoded_line[i])
			*((*ppMem)++) = (unsigned char)encoded_line[i] - (unsigned char)42;
		else
		{
			if(++i == len)
				return DecodeResult::YENC_TRUNCATED;
			*((*ppMem)++) = (unsigned char)encoded_line[i] - (unsigned char)106;
		}
	}

	// update the working CRC
	m_calc_crc32.update_crc(p_buf, *ppMem - p_buf);

	return DecodeResult::YENC_DATA;
}

DecodeResult Decoder::decode(unsigned char *pBuf, int *pLen, const char *encoded_line, int len)
{
	if((0 == pBuf) || (0 == encoded_line) || (0 == len))
		return DecodeResult::YENC_NONE;

	// check for a keyword line
	if(('=' == encoded_line[0]) && ('y' == encoded_line[1]))
	{
		decode_kw_line(encoded_line, len);
		return(m_part_flags[YENC_TRAILER] ? DecodeResult::YENC_COMPLETE : DecodeResult::YENC_NONE);
	}

	// decoded the header keyword line yet?
	if(m_line <= 0) return DecodeResult::YENC_NONE;

	uint8_t *p_buf = pBuf;

	// decode a line of yenc encoded data
	for(register int i = 0; i < len; ++i)
	{
		if('=' != encoded_line[i])
			*(pBuf++) = (unsigned char)encoded_line[i] - (unsigned char)42;
		else
		{
			if(++i == len)
				return DecodeResult::YENC_TRUNCATED;
			*(pBuf++) = (unsigned char)encoded_line[i] - (unsigned char)106;
		}
		++*pLen;
	}

	// update the working CRC
	m_calc_crc32.update_crc(p_buf, pBuf - p_buf);

	return DecodeResult::YENC_DATA;
}

}	/* namespace yEnc */
