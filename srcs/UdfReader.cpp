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
  this->_parseRootDirectoryFileEntry(is);
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
  long_ad * fsd_addr;
  uint32_t loc;

  fsd_addr = (long_ad *)&this->_lvd.logicalVolContentsUse;
  loc = this->_pd.partitionStartingLocation + fsd_addr->extLocation.logicalBlockNum;
  is.seekg(loc * SECTOR_SIZE, is.beg);
  is.read((char *)&this->_fsd, sizeof(this->_fsd));

  if (this->_fsd.descTag.tagIdent != TAG_IDENT_FSD
      || this->_fsd.descTag.tagLocation != fsd_addr->extLocation.logicalBlockNum)
    std::cerr << "Corrupted FileSetDescriptor" << std::endl;
}

void UdfReader::_parseRootDirectoryFileEntry(std::istream & is)
{
  if (this->_parseDescriptor(is, (char *)&this->_rdfe, sizeof(this->_rdfe), TAG_IDENT_FE, this->_fsd.rootDirectoryICB.extLocation.logicalBlockNum))
    std::cerr << "Corrupted FileEntry" << std::endl;
}

void UdfReader::_parseRootDirectoryFileIdentifierDescriptor(std::istream & is)
{
  if (this->_parseDescriptor(is, (char *)&this->_rdfid, sizeof(this->_rdfid), TAG_IDENT_FID, this->_rdfe.descTag.tagLocation + 1))
    std::cerr << "Corrupted FileIdentifierDescriptor" << std::endl;
}

#include <iostream>
#include <iomanip>
/*
** fill the structure in parameter with disk info
*/
void  UdfReader::getFDiskData(FDiskData &data)
{
  //timezone
  data.recordingDateAndTime = &_pvd.recordingDateAndTime;

  struct
  {
    unsigned type:4;
    unsigned timezone:12;
  } typeAndTimezone;

  typeAndTimezone.type = data.recordingDateAndTime->typeAndTimezone >> 12;
  typeAndTimezone.timezone = data.recordingDateAndTime->typeAndTimezone & 0b111111111;
  std::cout << "Record Time:"
            << data.recordingDateAndTime->year << "-"
            << (int)data.recordingDateAndTime->month << "-"
            << (int)data.recordingDateAndTime->day << "\t"
            << (int)data.recordingDateAndTime->hour << ":"
            << (int)data.recordingDateAndTime->minute << ":"
            << std::setw(2) << std::setfill('0') << (int)data.recordingDateAndTime->second;
  if (typeAndTimezone.timezone >= -1440 && typeAndTimezone.timezone <= 1440)
  {
    std::cout << " (UTC: " 
               << std::showpos << typeAndTimezone.timezone / 60 << ")";
  }
  std::cout << std::endl;

  
}
