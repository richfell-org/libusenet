#ifndef __NZB_PARSE_HEADER__
#define __NZB_PARSE_HEADER__

#include <libusenet/nzb.h>

namespace NZB { namespace Parse {

FileCollection parseFile(const char *path);

} } // namespace NZB::Parse

#endif	/* __NZB_PARSE_HEADER__ */
