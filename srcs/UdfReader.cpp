#include <iostream>

#include "UdfReader.hh"

UdfReader::UdfReader(std::istream & is)
{
  this->parse(is);
}

UdfReader::UdfReader(UdfReader const & udf)
{
  (*this) = udf;
}

UdfReader::UdfReader(void) { }

UdfReader::~UdfReader(void) { }

UdfReader & UdfReader::operator=(UdfReader const & udf)
{
  this->_avdp = udf._avdp;
  return (*this);
}

void UdfReader::parse(std::istream & is)
{
  this->_parseAnchorDescriptor(is);
  this->_parsePartitionDescriptor(is);
}

void UdfReader::_parseAnchorDescriptor(std::istream & is)
{
  // Anchor volume descriptor always at sector 256
  is.seekg(256 * SECTOR_SIZE, is.beg);
  is.read((char *)&this->_avdp, sizeof(this->_avdp));
}

void UdfReader::_parsePartitionDescriptor(std::istream & is)
{
  uint32_t loc;
  uint32_t len;

  // Partition descriptor always at MVDS_Location  + (MVDS_Length - 1) / SectorSize
  loc = this->_avdp.MainVolumeDescriptorSequenceExtent.loc;
  len = this->_avdp.MainVolumeDescriptorSequenceExtent.len;
  is.seekg(loc + (len - 1) / SECTOR_SIZE, is.beg);
  is.read((char *)&this->_pd, sizeof(this->_pd));

  if (this->_pd.DescriptorTag.TagIdentifier != 5)
    std::cerr << "Corrupted Partition descriptor" << std::endl;
}
