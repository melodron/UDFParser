#include <iostream>
#include <cstring>

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
  this->_parseAnchorVolumeDescriptorPointer(is);
  this->_parsePrimaryVolumeDescriptor(is);
  this->_parsePartitionDescriptor(is);
  this->_parseLogicalVolumeDescriptor(is);
}

bool UdfReader::_parseDescriptor(std::istream & is, char *desc, long unsigned int size, Uint16 tagIdentifier, uint32_t offset)
{
  uint32_t loc;
  char *reserveDesc;
  tag *tagId;

  loc = this->_avdp.MainVolumeDescriptorSequenceExtent.extLocation + offset;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(desc, size);

  tagId = (tag *)desc;
  if (tagId->tagIdent != tagIdentifier
      || tagId->tagLocation != loc)
    return false;

  // integrity check with reserve
  reserveDesc = new char [size];
  loc = this->_avdp.ReserveVolumeDescriptorSequenceExtent.extLocation + offset;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(reserveDesc, size);

  tagId = (tag *)reserveDesc;
  if (tagId->tagIdent != tagIdentifier
      || tagId->tagLocation != loc) {
    delete [] reserveDesc;
    return false;
  }

  if (memcmp(desc + sizeof(*tagId), reserveDesc + sizeof(*tagId), size - sizeof(*tagId)) != 0) {
    delete [] reserveDesc;
    return false;
  }

  delete [] reserveDesc;
  return true;
}

void UdfReader::_parseAnchorVolumeDescriptorPointer(std::istream & is)
{
  uint32_t loc;

  // Anchor volume descriptor always at sector 256
  loc = 256;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read((char *)&this->_avdp, sizeof(this->_avdp));

  if (this->_avdp.DescriptorTag.tagIdent != 2
      || this->_avdp.DescriptorTag.tagLocation != loc)
    std::cerr << "Corrupted AnchorVolumeDescriptorPointer" << std::endl;
}

void UdfReader::_parsePrimaryVolumeDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_pvd, sizeof(this->_pvd), PVD_IDENTIFIER, 0))
    std::cerr << "Corrupted PrimaryVolumeDescriptor" << std::endl;
}

void UdfReader::_parsePartitionDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_pd, sizeof(this->_pd), PD_IDENTIFIER, 2))
    std::cerr << "Corrupted PartitionDescriptor" << std::endl;
}

void UdfReader::_parseLogicalVolumeDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_lvd, sizeof(this->_lvd), LVD_IDENTIFIER, 1))
    std::cerr << "Corrupted LogicalVolumeDescriptor" << std::endl;
}
