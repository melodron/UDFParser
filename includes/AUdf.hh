#ifndef __AUDF_H__
# define __AUDF_H__

#include <istream>
#include <stdint.h>

#include "ecma_167.h"

# define SECTOR_SIZE (2048)
# define MAX_SECTOR (32)

class AUdf
{
public:
  static bool detect(std::istream & is);
};

#endif /* !__AUDF_H__ */
