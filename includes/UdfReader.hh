#ifndef __UDFREADER_H__
# define __UDFREADER_H__

#include <list>
#include <fstream>

#include "AUdf.hh"
#include "FDiskData.h"

class File
{
protected:
  std::string _name;
  uint32_t _uid;
  uint32_t _gid;

public:
  void setName(char *name, uint8_t length);
  void setUid(uint32_t uid);
  void setGid(uint32_t gid);

  std::string const & getName(void) const;
  uint32_t getUid(void) const;
  uint32_t getGid(void) const;
};

class Directory : public File
{
private:
  std::list<File *> _files;
  std::list<Directory *> _directorys;

public:
  void addFile(File *file);
  void addDirectory(Directory *directory);
};

class UdfReader : public AUdf
{
  anchorVolDescPtr _avdp;
  primaryVolDesc _pvd;
  partitionDesc _pd;
  logicalVolDesc _lvd;
  logicalVolIntegrityDesc _lvid;
  fileSetDesc *_fsd;
  extendedFileEntry *_rdefe;
  fileIdentDesc *_rdfid;
  char _buffer[SECTOR_SIZE];
  Directory *_rootDirectory;
  Directory *_currentDirectory;
  std::ifstream & _udfFile;

public:
  UdfReader(std::ifstream & is);
  ~UdfReader(void);

  void parse(std::istream & is);
  void toto(std::istream & is);
  void getFDiskData(FDiskData &data);

private:
  bool _parseDescriptor(std::istream & is, char *desc, long unsigned int size, uint16_t  tagIdentifier, uint32_t offset);

  void _parseAnchorVolumeDescriptorPointer(std::istream & is);
  void _parsePrimaryVolumeDescriptor(std::istream & is);
  void _parsePartitionDescriptor(std::istream & is);
  void _parseLogicalVolumeDescriptor(std::istream & is);
  void _parseLogicalVolumeIntegrityDescriptor(std::istream & is);
  void _parseFileSetDescriptor(std::istream & is);
  void _parseRootDirectoryExtendedFileEntry(std::istream & is);
  void _parseRootDirectoryFileIdentifierDescriptor(void);
  void _parseDirectory(std::istream & is, uint8_t *startPos, uint32_t length, Directory *parent);
};

#endif /* !__UDFREADER_H__ */
