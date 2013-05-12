#ifndef __AUDF_H__
# define __AUDF_H__

#include <istream>

#include "UDF.h"

class AUdf
{
public:
  static bool detect(std::istream & is);
};

#endif /* !__AUDF_H__ */
