#ifndef __UDFREADER_H__
# define __UDFREADER_H__

#include "AUdf.hh"

# define PVD_IDENTIFIER ((Uint16)1)
# define PD_IDENTIFIER ((Uint16)5)
# define LVD_IDENTIFIER ((Uint16)6)

class UdfReader : public AUdf
{
  AnchorVolumeDescriptorPointer _avdp;
  PrimaryVolumeDescriptor _pvd;
  ImpUseVolumeDescriptor _iuvd;
  PartitionDescriptor _pd;
  LogicalVolumeIntegrityDesc _lvid;
  LogicalVolumeDescriptor _lvd;

public:
  UdfReader(std::istream & is);
  UdfReader(UdfReader const & udf);
  UdfReader(void);
  ~UdfReader(void);

  UdfReader & operator=(UdfReader const & udf);

  void parse(std::istream & is);

private:
  bool _parseDescriptor(std::istream & is, char *desc, long unsigned int size, Uint16 tagIdentifier, uint32_t offset);

  void _parseAnchorVolumeDescriptorPointer(std::istream & is);
  void _parsePrimaryVolumeDescriptor(std::istream & is);
  void _parsePartitionDescriptor(std::istream & is);
  void _parseLogicalVolumeDescriptor(std::istream & is);
};

#endif /* !__UDFREADER_H__ */
