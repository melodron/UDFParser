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
  is.read((char *)&this->_lvid, sizeof(logicalVolIntegrityDesc));
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
  this->_rootDirectory->setCreateTime(this->_rdefe->createTime);
  this->_rootDirectory->setModificationTime(this->_rdefe->modificationTime);
  this->_rootDirectory->setUid(this->_rdefe->uid);
  this->_rootDirectory->setGid(this->_rdefe->gid);
  this->_rootDirectory->setPermissions(this->_rdefe->permissions);


  if (this->_rdefe->descTag.tagIdent != TAG_IDENT_EFE)
    std::cerr << "Corrupted ExtendedFileEntry" << std::endl;
}

void UdfReader::_parseRootDirectoryFileIdentifierDescriptor(void)
{
  uint8_t *pos;

  pos = &this->_rdefe->allocDescs[0] + this->_rdefe->lengthExtendedAttr;
  this->_rdfid = (fileIdentDesc *)pos;
  this->_rootDirectory->setFid(this->_rdfid);

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
  char *allocedFid;

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

    allocedFid = new char [sizeof(*fid) + fid->lengthOfImpUse + fid->lengthFileIdent];
    memcpy(allocedFid, fid, sizeof(*fid) + fid->lengthOfImpUse + fid->lengthFileIdent);
    newFile->setFid((fileIdentDesc*)&allocedFid[0]);
    newFile->setName((char*)&fid->fileIdent[0] + fid->lengthOfImpUse + 1, fid->lengthFileIdent - 1);
    newFile->setCreateTime(efe->createTime);
    newFile->setModificationTime(efe->modificationTime);
    newFile->setUid(efe->uid);
    newFile->setGid(efe->gid);
    newFile->setPermissions(efe->permissions);

    pos += (((sizeof(*fid)) + fid->lengthOfImpUse + fid->lengthFileIdent) + 3) & ~3;
  }
}

void UdfReader::copy(std::string const & filename, std::string const & to)
{
  std::list<File *> files;
  std::list<File *>::iterator it;
  fileIdentDesc *fid;
  extendedFileEntry *efe;
  char buffer[SECTOR_SIZE];
  std::ofstream fileTo;

  fileTo.open(to.c_str(), std::ofstream::out | std::ofstream::binary);
  files = this->_currentDirectory->getFiles();
  for (it = files.begin(); it != files.end(); ++it) {
    if (filename == (*it)->getName()) {
      fid = (*it)->getFid();
      this->_udfFile.seekg((this->_pd.partitionStartingLocation + fid->icb.extLocation.logicalBlockNum) * SECTOR_SIZE, this->_udfFile.beg);
      this->_udfFile.read(buffer, SECTOR_SIZE);
      efe = (extendedFileEntry*)&buffer[0];
      fileTo << (char*)&efe->extendedAttr[0];
      break ;
    }
  }
  fileTo.close();
}

void UdfReader::readFile(std::string const & filename)
{
  std::list<File *> files;
  std::list<File *>::iterator it;
  fileIdentDesc *fid;
  extendedFileEntry *efe;
  char buffer[SECTOR_SIZE];

  files = this->_currentDirectory->getFiles();
  for (it = files.begin(); it != files.end(); ++it) {
    if (filename == (*it)->getName()) {
      fid = (*it)->getFid();
      this->_udfFile.seekg((this->_pd.partitionStartingLocation + fid->icb.extLocation.logicalBlockNum) * SECTOR_SIZE, this->_udfFile.beg);
      this->_udfFile.read(buffer, SECTOR_SIZE);
      efe = (extendedFileEntry*)&buffer[0];
      std::cout << (char*)&efe->extendedAttr[0];
      return ;
    }
  }
}

Directory *UdfReader::getCurrentDirectory(void)
{
  return this->_currentDirectory;
}

void UdfReader::chdir(std::string const & dir)
{
  std::string firstDir;
  std::list<Directory *> directories;
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
    directories = this->_currentDirectory->getDirectories();
    for (it = directories.begin(); it != directories.end(); ++it) {
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

  std::cout << "       ..\t<dir>\t";
  std::cout << this->_currentDirectory->getCreationTime().year << "-"
	    << (int)this->_currentDirectory->getCreationTime().month << "-"
	    << (int)this->_currentDirectory->getCreationTime().day << " "
	    << (int)this->_currentDirectory->getCreationTime().hour << ":"
	    << (int)this->_currentDirectory->getCreationTime().minute << ":"
	    << (int)this->_currentDirectory->getCreationTime().second << std::endl;
  dirs = this->_currentDirectory->getDirectories();
  for (itDir = dirs.begin(); itDir != dirs.end(); ++itDir) {
    std::cout << ((*itDir)->isHidden() ? "<hide> " : "       ") << (*itDir)->getName() << "\t<dir>\t";
  std::cout << (*itDir)->getCreationTime().year << "-"
	    << (int)(*itDir)->getCreationTime().month << "-"
	    << (int)(*itDir)->getCreationTime().day << " "
	    << (int)(*itDir)->getCreationTime().hour << ":"
	    << (int)(*itDir)->getCreationTime().minute << ":"
	    << (int)(*itDir)->getCreationTime().second << std::endl;
  }

  files = this->_currentDirectory->getFiles();
  for (itFile = files.begin(); itFile != files.end(); ++itFile) {
    std::cout << ((*itFile)->isHidden() ? "<hide> " : "       ") << (*itFile)->getName() << "\t<file>\t";
  std::cout << (*itFile)->getCreationTime().year << "-"
	    << (int)(*itFile)->getCreationTime().month << "-"
	    << (int)(*itFile)->getCreationTime().day << " "
	    << (int)(*itFile)->getCreationTime().hour << ":"
	    << (int)(*itFile)->getCreationTime().minute << ":"
	    << (int)(*itFile)->getCreationTime().second << std::endl;
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

void File::setPermissions(int permissions)
{
  this->_permissions = permissions;
}

int File::getPermissions(void) const
{
  return this->_permissions;
}

void File::setCreateTime(timestamp time)
{
  this->_createTime = time;
}

void File::setModificationTime(timestamp time)
{
  this->_modificationTime = time;
}

timestamp const & File::getCreationTime(void) const
{
  return this->_createTime;
}

timestamp const & File::getModificationTime(void) const
{
  return this->_modificationTime;
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

void File::setFid(fileIdentDesc *fid)
{
  this->_fid = fid;
}

fileIdentDesc *File::File::getFid()
{
  return this->_fid;
}

void Directory::addDirectory(Directory *newDir)
{
  this->_directories.push_back(newDir);
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

std::list<Directory *> Directory::getDirectories(void)
{
  return this->_directories;
}

std::list<File *> Directory::getFiles(void)
{
  return this->_files;
}

/*
** fill the structure in parameter with disk info
*/
void  UdfReader::getFDiskData(FDiskData &data)
{
  //timezone
  data.recordingDateAndTime = &_pvd.recordingDateAndTime;

  // volume
  memcpy(data.volumeIdentifier, this->_pvd.volIdent, 32);

  // version - not working, don't know why
  data.version = (this->_pvd.impIdent.identSuffix[0] << 8) | this->_pvd.impIdent.identSuffix[1];

  // space
  _udfFile.seekg(0, std::ifstream::end);
  data.totalSpace = _udfFile.tellg(); 
  
  data.freeSpace = 0;
  for (size_t i = 0; i < this->_lvid.numOfPartitions; ++i)
  {
    if (this->_lvid.freeSpaceTable[i] != 0xFFFFFF)
    {
      std::cout << "debug " << this->_lvid.freeSpaceTable[i] * SECTOR_SIZE << std::endl;
      data.freeSpace += this->_lvid.freeSpaceTable[i] * SECTOR_SIZE;
    }
  }
}
