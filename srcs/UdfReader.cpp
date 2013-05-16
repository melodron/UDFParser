#include <iostream>
#include <cstring>

#include "UdfReader.hh"

UdfReader::UdfReader(std::ifstream & is)
  :_rootDirectory(new Directory),
  _udfFile(is)
{
  this->parse(is);
  this->_currentDirectory = this->_rootDirectory;
  this->_rootDirectory->setParent(this->_rootDirectory);
}

// TODO : FREEEeeEEEeeeEEEeeeEEEeeeEEe
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
  this->_parseRootDirectoryFileIdentifierDescriptor();
  this->_parseDirectory(is, 
			&this->_rdefe->allocDescs[0] + this->_rdefe->lengthExtendedAttr,
			this->_rdefe->lengthAllocDescs,
			this->_rootDirectory);
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

void UdfReader::_parseRootDirectoryFileIdentifierDescriptor(void)
{
  uint8_t *pos;

  pos = &this->_rdefe->allocDescs[0] + this->_rdefe->lengthExtendedAttr;
  this->_rdfid = (fileIdentDesc *)pos;

  if (this->_rdfid->descTag.tagIdent != TAG_IDENT_FID)
      std::cerr << "Corrupted FileIdentifierDescriptor" << std::endl;
}

void UdfReader::_parseDirectory(std::istream & is, uint8_t *startPos, uint32_t lengthAllocDescs, Directory *parent)
{
  uint8_t *pos;
  fileIdentDesc *fid;
  Directory *newDir;
  File *newFile;
  extendedFileEntry *efe;
  uint32_t loc;
  char buffer[SECTOR_SIZE];

  fid = (fileIdentDesc*)startPos;
  pos = startPos + ((((sizeof(*fid)) + fid->lengthOfImpUse + fid->lengthFileIdent) + 3) & ~3);
  while (pos - startPos < lengthAllocDescs) {
    fid = (fileIdentDesc *)pos;

    if (fid->descTag.tagIdent != TAG_IDENT_FID)
      std::cerr << "Corrupted FileIdentifierDescriptor" << std::endl;

    loc = this->_pd.partitionStartingLocation + fid->icb.extLocation.logicalBlockNum;
    is.seekg(loc * SECTOR_SIZE, is.beg);
    is.read(buffer, SECTOR_SIZE);
    efe = (extendedFileEntry *)buffer;

    if (fid->fileCharacteristics == FID_FILE_CHAR_DIRECTORY) {
      newDir = new Directory();
      parent->addDirectory(newDir);
      newDir->setParent(parent);
      this->_parseDirectory(is, &efe->allocDescs[0] + efe->lengthExtendedAttr, efe->lengthAllocDescs, newDir);
      newFile = newDir;
    } else {
      newFile = new File();
      parent->addFile(newFile);
    }

    if (fid->fileCharacteristics == FID_FILE_CHAR_HIDDEN)
      newFile->setHidden(true);

    newFile->setName((char*)&fid->fileIdent[0] + fid->lengthOfImpUse + 1, fid->lengthFileIdent - 1);
    // newFile->setCreateTime(efe->createTime);
    // newFile->setModificationTime(efe->modificationTime);
    newFile->setUid(efe->uid);
    newFile->setGid(efe->gid);
    // newFile->setPermissions(efe->permissions);

    pos += (((sizeof(*fid)) + fid->lengthOfImpUse + fid->lengthFileIdent) + 3) & ~3;
  }
}

Directory *UdfReader::getCurrentDirectory(void)
{
  return this->_currentDirectory;
}

void UdfReader::chdir(std::string const & dir)
{
  std::string firstDir;
  std::list<Directory *> directorys;
  std::list<Directory *>::iterator it;
  int pos;

  pos = dir.find('/');
  if (pos == -1)
    firstDir = dir;
  else {
    firstDir = dir.substr(0, pos);
  }

  if (firstDir == "..") {
    this->_currentDirectory = this->_currentDirectory->getParent();
  } else if (firstDir == "") {
    this->_currentDirectory = this->_rootDirectory;
  } else {
    directorys = this->_currentDirectory->getDirectorys();
    for (it = directorys.begin(); it != directorys.end(); ++it) {
      if (firstDir.compare((*it)->getName()) == 0) {
	this->_currentDirectory = (*it);
	break;
      }
    }
  }

  if (pos != -1 && pos + 1 != (int)dir.size())
    this->chdir(dir.substr(pos + 1));
}

void UdfReader::listDirectory(void)
{
  std::list<Directory*> dirs;
  std::list<Directory*>::iterator itDir;
  std::list<File*> files;
  std::list<File*>::iterator itFile;

  std::cout << "       ..\t<dir>\t" << std::endl;
  dirs = this->_currentDirectory->getDirectorys();
  for (itDir = dirs.begin(); itDir != dirs.end(); ++itDir) {
    std::cout << ((*itDir)->isHidden() ? "<hide> " : "       ") << (*itDir)->getName() << "\t<dir>\t" << std::endl;
  }

  files = this->_currentDirectory->getFiles();
  for (itFile = files.begin(); itFile != files.end(); ++itFile) {
    std::cout << ((*itFile)->isHidden() ? "<hide> " : "       ") << (*itFile)->getName() << "\t<file>\t" << std::endl;
  }
}

void File::setName(char *name, uint8_t length)
{
  this->_name.append(name, length);
}

void File::setUid(uint32_t uid)
{
  this->_uid = uid;
}

void File::setGid(uint32_t gid)
{
  this->_gid = gid;
}

void File::setHidden(bool hidden)
{
  this->_hidden = hidden;
}

std::string const & File::getName(void) const
{
  return this->_name;
}

uint32_t File::getUid(void) const
{
  return this->_uid;
}

uint32_t File::getGid(void) const
{
  return this->_gid;
}

bool File::isHidden(void) const
{
  return this->_hidden;
}

void Directory::addDirectory(Directory *newDir)
{
  this->_directorys.push_back(newDir);
}

void Directory::addFile(File *newFile)
{
  this->_files.push_back(newFile);
}

void Directory::setParent(Directory *parent)
{
  this->_parent = parent;
}

Directory *Directory::getParent(void)
{
  return this->_parent;
}

std::list<Directory *> Directory::getDirectorys(void)
{
  return this->_directorys;
}

std::list<File *> Directory::getFiles(void)
{
  return this->_files;
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
