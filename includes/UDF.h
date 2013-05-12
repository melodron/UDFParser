#pragma once

#include <stdint.h>

#include "ecma_167.h"

# define SECTOR_SIZE (2048)
# define MAX_SECTOR (32)

typedef uint8_t 		Uint8;
typedef uint16_t 		Uint16;
typedef uint32_t 		Uint32;
typedef uint64_t 		Uint64;
typedef int16_t 		Int16;
typedef char			byte;
typedef uint8_t			dstring; // compilation patch => to be replaced asap

/* struct extent_ad */
/* { */
/* 	Uint32 	len; */
/* 	Uint32 	loc; */
/* }; */

/* struct charspec  */
/* { /\* ECMA 167 1/7.2.1 *\/ */
/* 	Uint8 	CharacterSetType; */
/* 	byte 	CharacterSetInfo[63]; */
/* }; */

/* struct timestamp  */
/* { /\* ECMA 167 1/7.3 *\/ */
/* 	Uint16 TypeAndTimezone; */
/* 	Int16 Year; */
/* 	Uint8 Month; */
/* 	Uint8 Day; */
/* 	Uint8 Hour; */
/* 	Uint8 Minute; */
/* 	Uint8 Second; */
/* 	Uint8 Centiseconds; */
/* 	Uint8 HundredsofMicroseconds; */
/* 	Uint8 Microseconds; */
/* }; */

/* typedef struct timestamp Timestamp; */

// struct long_ad 
// { /* ECMA 167 4/14.14.2 */
// 	Uint32 ExtentLength;
// 	Lb_addr ExtentLocation;
// 	byte ImplementationUse[6];
// };

struct EntityID 
{ /* ECMA 167 1/7.4 */
	Uint8 Flags;
	char Identifier[23];
	char IdentifierSuffix[8];
};

/* struct tag  */
/* { /\* ECMA 167 3/7.2 *\/ */
/* 	Uint16 TagIdentifier; */
/* 	Uint16 DescriptorVersion; */
/* 	Uint8 TagChecksum; */
/* 	byte Reserved; */
/* 	Uint16 TagSerialNumber; */
/* 	Uint16 DescriptorCRC; */
/* 	Uint16 DescriptorCRCLength; */
/* 	Uint32 TagLocation; */
/* }; */

struct PrimaryVolumeDescriptor
{ /* ECMA 167 3/10.1 */
	tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	Uint32 PrimaryVolumeDescriptorNumber;
	dstring VolumeIdentifier[32];
	Uint16 VolumeSequenceNumber;
	Uint16 MaximumVolumeSequenceNumber;
	Uint16 InterchangeLevel;
	Uint16 MaximumInterchangeLevel;
	Uint32 CharacterSetList;
	Uint32 MaximumCharacterSetList;
	dstring VolumeSetIdentifier[128];
	charspec DescriptorCharacterSet;
	charspec ExplanatoryCharacterSet;
	extent_ad VolumeAbstract;
	extent_ad VolumeCopyrightNotice;
	EntityID ApplicationIdentifier;
	timestamp RecordingDateandTime;
	EntityID ImplementationIdentifier;
	byte ImplementationUse[64];
	Uint32 PredecessorVolumeDescriptorSequenceLocation;
	Uint16 Flags;
	byte Reserved[22];
};

struct AnchorVolumeDescriptorPointer 
{ /* ECMA 167 3/10.2 */
	tag DescriptorTag;
	extent_ad MainVolumeDescriptorSequenceExtent;
	extent_ad ReserveVolumeDescriptorSequenceExtent;
	byte Reserved[480];
};

struct LogicalVolumeDescriptor 
{ /* ECMA 167 3/10.6 */
	tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	charspec DescriptorCharacterSet;
	dstring LogicalVolumeIdentifier[128];
	Uint32 LogicalBlockSize;
	EntityID DomainIdentifier;
	byte LogicalVolumeContentsUse[16];
	Uint32 MapTableLength;
	Uint32 NumberofPartitionMaps;
	EntityID ImplementationIdentifier;
	byte ImplementationUse[128];
	extent_ad IntegritySequenceExtent;
	byte PartitionMaps[];
};

struct UnallocatedSpaceDesc 
{ /* ECMA 167 3/10.8 */
	tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	Uint32 NumberofAllocationDescriptors;
	extent_ad AllocationDescriptors[];
};

struct LogicalVolumeIntegrityDesc 
{ /* ECMA 167 3/10.10 */
	tag DescriptorTag;
	timestamp RecordingDateAndTime;
	Uint32 IntegrityType;
	extent_ad NextIntegrityExtent;
	byte LogicalVolumeContentsUse[32];
	Uint32 NumberOfPartitions;
	Uint32 LengthOfImplementationUse; /* = L_IU */
	Uint32 FreeSpaceTable[];
	Uint32 SizeTable[];
	byte ImplementationUse[];
};

struct ImpUseVolumeDescriptor 
{ /* ECMA 167 3/10.4 */
	tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	EntityID ImplementationIdentifier;
	byte ImplementationUse[460];
};

struct LVInformation {
	charspec LVICharset;
	dstring LogicalVolumeIdentifier[128];
	dstring LVInfo1[36];
	dstring LVInfo2[36];
	dstring LVInfo3[36];
	EntityID ImplementationID;
	byte ImplementationUse[128];
};

struct PartitionDescriptor 
{ /* ECMA 167 3/10.5 */
	tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	Uint16 PartitionFlags;
	Uint16 PartitionNumber;
	EntityID PartitionContents;
	byte PartitionContentsUse[128];
	Uint32 AccessType;
	Uint32 PartitionStartingLocation;
	Uint32 PartitionLength;
	EntityID ImplementationIdentifier;
	byte ImplementationUse[128];
	byte Reserved[156];
};

struct FileSetDescriptor 
{ /* ECMA 167 4/14.1 */
	tag DescriptorTag;
	timestamp RecordingDateandTime;
	Uint16 InterchangeLevel;
	Uint16 MaximumInterchangeLevel;
	Uint32 CharacterSetList;
	Uint32 MaximumCharacterSetList;
	Uint32 FileSetNumber;
	Uint32 FileSetDescriptorNumber;
	charspec LogicalVolumeIdentifierCharacterSet;
	dstring LogicalVolumeIdentifier[128];
	charspec FileSetCharacterSet;
	dstring FileSetIdentifier[32];
	dstring CopyrightFileIdentifier[32];
	dstring AbstractFileIdentifier[32];
	long_ad RootDirectoryICB;
	EntityID DomainIdentifier;
	long_ad NextExtent;
	long_ad SystemStreamDirectoryICB;
	byte Reserved[32];
};

struct PartitionHeaderDescriptor 
{ /* ECMA 167 4/14.3 */
	short_ad UnallocatedSpaceTable;
	short_ad UnallocatedSpaceBitmap;
	short_ad PartitionIntegrityTable;
	short_ad FreedSpaceTable;
	short_ad FreedSpaceBitmap;
	byte Reserved[88];
};

struct FileIdentifierDescriptor 
{ /* ECMA 167 4/14.4 */
	tag DescriptorTag;
	Uint16 FileVersionNumber;
	Uint8 FileCharacteristics;
	Uint8 LengthofFileIdentifier;
	long_ad ICB;
	Uint16 LengthOfImplementationUse;
	byte ImplementationUse[];
	char FileIdentifier[];
	byte Padding[];
};


struct FileEntry 
{ /* ECMA 167 4/14.9 */
	tag DescriptorTag;
	icbtag ICBTag;
	Uint32 Uid;
	Uint32 Gid;
	Uint32 Permissions;
	Uint16 FileLinkCount;
	Uint8 RecordFormat;
	Uint8 RecordDisplayAttributes;
	Uint32 RecordLength;
	Uint64 InformationLength;
	Uint64 LogicalBlocksRecorded;
	timestamp AccessTime;
	timestamp ModificationTime;
	timestamp AttributeTime;
	Uint32 Checkpoint;
	long_ad ExtendedAttributeICB;
	EntityID ImplementationIdentifier;
        Uint64 UniqueID;
	Uint32 LengthofExtendedAttributes;
	Uint32 LengthofAllocationDescriptors;
	byte ExtendedAttributes[];
	byte AllocationDescriptors[];
};

struct UnallocatedSpaceEntry 
{ /* ECMA 167 4/14.11 */
	tag DescriptorTag;
	icbtag ICBTag;
	Uint32 LengthofAllocationDescriptors;
	byte AllocationDescriptors[];
};

struct SpaceBitmap 
{ /* ECMA 167 4/14.12 */
	tag DescriptorTag;
	Uint32 NumberOfBits;
	Uint32 NumberOfBytes;
	byte Bitmap[];
};

struct AllocationExtentDescriptor 
{ /* ECMA 167 4/14.5 */
	tag DescriptorTag;
	Uint32 PreviousAllocationExtentLocation;
	Uint32 LengthOfAllocationDescriptors;
};

struct PathComponent 
{ /* ECMA 167 4/14.16.1 */
	Uint8 ComponentType;
	Uint8 LengthofComponentIdentifier;
	Uint16 ComponentFileVersionNumber;
	char ComponentIdentifier[ ];
};

struct LogicalVolumeHeaderDesc 
{ /* ECMA 167 4/14.15 */
	Uint64 UniqueID;
	byte Reserved[24];
};
