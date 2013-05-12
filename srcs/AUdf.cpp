#include <cstring>

#include "AUdf.hh"

bool AUdf::detect(std::istream & is)
{
  uint32_t sector;
  EntityID vrs;
  char * buffer = new char [SECTOR_SIZE];
  bool validUdf = false;
  bool validDesc = true;

  is.seekg(16 * SECTOR_SIZE, is.beg);
  for (sector = 16; sector < MAX_SECTOR; ++sector) {
    is.read(buffer, SECTOR_SIZE);

    memcpy(&vrs, buffer, sizeof(vrs));

    if (memcmp(&vrs.Identifier, "BEA01", 5) == 0)
      continue;
    else if (memcmp(&vrs.Identifier, "TEA01", 5) == 0)
      break;
    else if (memcmp(&vrs.Identifier, "NSR02", 5) == 0
	     || memcmp(&vrs.Identifier, "NSR03", 5) == 0)
      validUdf = true;
    else {
      validDesc = false;
      break;
    }
  }

  delete[] buffer;
  return (validUdf && validDesc);
}
