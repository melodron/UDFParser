#include <cstring>

#include "AUdf.hh"

bool AUdf::detect(std::istream & is)
{
  uint32_t sector;
  regid vrs;
  char buffer[SECTOR_SIZE];
  bool validUdf = false;
  bool validDesc = true;

  is.seekg(16 * SECTOR_SIZE, is.beg);
  for (sector = 16; sector < MAX_SECTOR; ++sector) {
    is.read(buffer, SECTOR_SIZE);

    memcpy(&vrs, buffer, sizeof(vrs));

    if (memcmp(&vrs.ident, VSD_STD_ID_BEA01, VSD_STD_ID_LEN) == 0)
      continue;
    else if (memcmp(&vrs.ident, VSD_STD_ID_TEA01, VSD_STD_ID_LEN) == 0)
      break;
    else if (memcmp(&vrs.ident, VSD_STD_ID_NSR02, VSD_STD_ID_LEN) == 0
	     || memcmp(&vrs.ident, VSD_STD_ID_NSR03, VSD_STD_ID_LEN) == 0)
      validUdf = true;
    else {
      validDesc = false;
      break;
    }
  }
  return (validUdf && validDesc);
}
