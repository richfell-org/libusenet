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
