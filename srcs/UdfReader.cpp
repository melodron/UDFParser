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
  this->_parseFileSetDescriptor(is);
  this->_parseRootDirectoryExtendedFileEntry(is);
  this->_parseRootDirectoryFileIdentifierDescriptor(is);
}

// TODO: refactor
bool UdfReader::_parseDescriptor(std::istream & is, char *desc, long unsigned int size, uint16_t tagIdentifier, uint32_t offset)
{
  uint32_t loc;
  char *reserveDesc;
  tag *tagId;

  loc = this->_avdp.mainVolDescSeqExt.extLocation + offset;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(desc, size);

  tagId = (tag *)desc;
  if (tagId->tagIdent != tagIdentifier
      || tagId->tagLocation != loc)
    return false;

  // integrity check with reserve
  reserveDesc = new char [size];
  loc = this->_avdp.reserveVolDescSeqExt.extLocation + offset;
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

  if (this->_avdp.descTag.tagIdent != 2
      || this->_avdp.descTag.tagLocation != loc)
    std::cerr << "Corrupted AnchorVolumeDescriptorPointer" << std::endl;
}

void UdfReader::_parsePrimaryVolumeDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_pvd, sizeof(this->_pvd), TAG_IDENT_PVD, 0))
    std::cerr << "Corrupted PrimaryVolumeDescriptor" << std::endl;
}

void UdfReader::_parsePartitionDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_pd, sizeof(this->_pd), TAG_IDENT_PD, 2))
    std::cerr << "Corrupted PartitionDescriptor" << std::endl;
}

void UdfReader::_parseLogicalVolumeDescriptor(std::istream & is)
{
  if (!this->_parseDescriptor(is, (char *)&this->_lvd, sizeof(this->_lvd), TAG_IDENT_LVD, 1))
    std::cerr << "Corrupted LogicalVolumeDescriptor" << std::endl;
}

void UdfReader::_parseFileSetDescriptor(std::istream & is)
{
  uint32_t loc;
  long_ad *rootFileEntryICB;

  rootFileEntryICB = (long_ad *)&this->_lvd.logicalVolContentsUse;
  loc = this->_pd.partitionStartingLocation + rootFileEntryICB->extLocation.logicalBlockNum;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(this->_buffer, SECTOR_SIZE);
  this->_fsd = (fileSetDesc *)&this->_buffer[0];

  if (this->_fsd->descTag.tagIdent != TAG_IDENT_FSD)
    std::cerr << "Corrupted FileSetDescriptor" << std::endl;
}

void UdfReader::_parseRootDirectoryExtendedFileEntry(std::istream & is)
{
  uint32_t loc;

  loc = this->_pd.partitionStartingLocation + this->_fsd->rootDirectoryICB.extLocation.logicalBlockNum;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(this->_buffer, SECTOR_SIZE);
  this->_rdefe = (extendedFileEntry *)&this->_buffer[0];

  if (this->_rdefe->descTag.tagIdent != TAG_IDENT_EFE)
    std::cerr << "Corrupted ExtendedFileEntry" << std::endl;
}

void UdfReader::_parseRootDirectoryFileIdentifierDescriptor(std::istream & is)
{
  uint32_t loc;

  loc = this->_pd.partitionStartingLocation + this->_rdefe->informationLength;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read(this->_buffer, SECTOR_SIZE);
  this->_rdfid = (fileIdentDesc *)this->_buffer;

  if (this->_rdfid->descTag.tagIdent != TAG_IDENT_FID)
    std::cerr << "Corrupted FileIdentifierDescriptor" << std::endl;
}
