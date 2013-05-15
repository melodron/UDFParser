#ifndef __UDFREADER_H__
# define __UDFREADER_H__

#include <fstream>
#include "AUdf.hh"
#include "FDiskData.h"

class UdfReader : public AUdf
{
  anchorVolDescPtr _avdp;
  primaryVolDesc _pvd;
  partitionDesc _pd;
  logicalVolDesc _lvd;
  fileSetDesc *_fsd;
  extendedFileEntry *_rdefe;
  fileIdentDesc *_rdfid;
  char _buffer[SECTOR_SIZE];
  
  std::ifstream & _udfFile;

public:
  UdfReader(std::ifstream & is);
  ~UdfReader(void);

  void parse(std::istream & is);
  void toto(std::istream & is);
  void getFDiskData(FDiskData &data);

private:
  bool _parseDescriptor(std::istream & is, char *desc, long unsigned int size, uint16_t  tagIdentifier, uint32_t offset);

  void _parseAnchorVolumeDescriptorPointer(std::istream & is);
  void _parsePrimaryVolumeDescriptor(std::istream & is);
  void _parsePartitionDescriptor(std::istream & is);
  void _parseLogicalVolumeDescriptor(std::istream & is);
  void _parseFileSetDescriptor(std::istream & is);
  void _parseRootDirectoryExtendedFileEntry(std::istream & is);
  void _parseRootDirectoryFileIdentifierDescriptor(std::istream & is);
};

#endif /* !__UDFREADER_H__ */
