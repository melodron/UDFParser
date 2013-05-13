#ifndef __UDFREADER_H__
# define __UDFREADER_H__

#include "AUdf.hh"
#include "FDiskData.h"

class UdfReader : public AUdf
{
  anchorVolDescPtr _avdp;
  primaryVolDesc _pvd;
  partitionDesc _pd;
  logicalVolDesc _lvd;
  fileSetDesc _fsd;
  fileEntry _rdfe;
  fileIdentDesc _rdfid;
  

public:
  UdfReader(std::istream & is);
  UdfReader(UdfReader const & udf);
  UdfReader(void);
  ~UdfReader(void);

  UdfReader & operator=(UdfReader const & udf);

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
  void _parseRootDirectoryFileEntry(std::istream & is);
  void _parseRootDirectoryFileIdentifierDescriptor(std::istream & is);
};

#endif /* !__UDFREADER_H__ */
