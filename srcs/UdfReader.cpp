#include <iostream>
#include <cstring>

#include "UdfReader.hh"

UdfReader::UdfReader(std::ifstream & is) :
  _udfFile(is)
{
  this->parse(is);
}

UdfReader::~UdfReader(void) { }

void UdfReader::parse(std::istream & is)
{
  this->_parseAnchorVolumeDescriptorPointer(is);
  this->_parsePrimaryVolumeDescriptor(is);
  this->_parsePartitionDescriptor(is);
  this->_parseLogicalVolumeDescriptor(is);
  this->_parseLogicalVolumeIntegrityDescriptor(is);
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

void UdfReader::_parseLogicalVolumeIntegrityDescriptor(std::istream & is)
{
  is.seekg(this->_lvd.integritySeqExt.extLocation * SECTOR_SIZE, is.beg);
  is.read((char *)&this->_lvid, this->_lvd.integritySeqExt.extLength);
  if (this->_lvid.descTag.tagIdent != TAG_IDENT_LVID)
      std::cerr << "Corrupted LogicalVolumeIntegrityDescriptor" << std::endl;
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

#include <iostream>
#include <iomanip>
#include <bitset>
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

  // volume
  std::cout << this->_pvd.volIdent << std::endl;
  std::cout << this->_pvd.volSetIdent << std::endl;

  // version - not working, don't know why
  memcpy(data.identifierSuffix, &this->_pvd.impIdent.identSuffix, 8);
  uint16_t  version = (data.identifierSuffix[0] << 8) | data.identifierSuffix[1];

  //std::cout << (int)data.identifierSuffix[1] << std::endl;
  std::cout << "Version:" << version << std::endl;
  std::cout << "pmop" << *((uint16_t *) &this->_pvd.impIdent.identSuffix) << std::endl;
  std::cout << data.identifierSuffix << std::endl;

  // space
  _udfFile.seekg(0, std::ifstream::end);
  std::cout << _udfFile.tellg() << std::endl; 
  
  std::cout << this->_lvid.numOfPartitions << std::endl;

  uint64_t freespace = 0;
  for (size_t i = 0; i < this->_lvid.numOfPartitions; ++i)
  {
    if (this->_lvid.freeSpaceTable[i] != 0xFFFFFF)
    {
      std::cout << "free " << this->_lvid.freeSpaceTable[i] * SECTOR_SIZE << std::endl;
      freespace += this->_lvid.freeSpaceTable[i] * SECTOR_SIZE;
    }
  }
  std::cout << "Freespace " << freespace << std::endl;
  
}
