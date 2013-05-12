#ifndef __UDFREADER_H__
# define __UDFREADER_H__

#include "AUdf.hh"

class UdfReader : public AUdf
{
  AnchorVolumeDescriptorPointer _avdp;
  PartitionDescriptor _pd;

public:
  UdfReader(std::istream & is);
  UdfReader(UdfReader const & udf);
  UdfReader(void);
  ~UdfReader(void);

  UdfReader & operator=(UdfReader const & udf);

  void parse(std::istream & is);

private:
  void _parseAnchorDescriptor(std::istream & is);
  void _parsePartitionDescriptor(std::istream & is);
};

#endif /* !__UDFREADER_H__ */
