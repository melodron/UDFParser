#pragma once

#include <stdint.h>

typedef uint8_t 		Uint8;
typedef uint16_t 		Uint16;
typedef uint32_t 		Uint32;
typedef uint64_t 		Uint64;
typedef int16_t 		Int16;
typedef unsigned char 	byte;
typedef uint8_t			dstring; // compilation patch => to be replaced asap

struct extent_ad
{
	uint32_t 	len;
	uint32_t 	loc;
};

struct charspec 
{ /* ECMA 167 1/7.2.1 */
	Uint8 	CharacterSetType;
	byte 	CharacterSetInfo[63];
};

struct timestamp 
{ /* ECMA 167 1/7.3 */
	Uint16 TypeAndTimezone;
	Int16 Year;
	Uint8 Month;
	Uint8 Day;
	Uint8 Hour;
	Uint8 Minute;
	Uint8 Second;
	Uint8 Centiseconds;
	Uint8 HundredsofMicroseconds;
	Uint8 Microseconds;
};

typedef struct timestamp Timestamp;

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

struct tag 
{ /* ECMA 167 3/7.2 */
	Uint16 TagIdentifier;
	Uint16 DescriptorVersion;
	Uint8 TagChecksum;
	byte Reserved;
	Uint16 TagSerialNumber;
	Uint16 DescriptorCRC;
	Uint16 DescriptorCRCLength;
	Uint32 TagLocation;
};

struct PrimaryVolumeDescriptor 
{ /* ECMA 167 3/10.1 */
	struct tag DescriptorTag;
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
	struct charspec DescriptorCharacterSet;
	struct charspec ExplanatoryCharacterSet;
	struct extent_ad VolumeAbstract;
	struct extent_ad VolumeCopyrightNotice;
	struct EntityID ApplicationIdentifier;
	struct timestamp RecordingDateandTime;
	struct EntityID ImplementationIdentifier;
	byte ImplementationUse[64];
	Uint32 PredecessorVolumeDescriptorSequenceLocation;
	Uint16 Flags;
	byte Reserved[22];
};

struct AnchorVolumeDescriptorPointer 
{ /* ECMA 167 3/10.2 */
	struct tag DescriptorTag;
	struct extent_ad MainVolumeDescriptorSequenceExtent;
	struct extent_ad ReserveVolumeDescriptorSequenceExtent;
	byte Reserved[480];
};

struct LogicalVolumeDescriptor 
{ /* ECMA 167 3/10.6 */
	struct tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	struct charspec DescriptorCharacterSet;
	dstring LogicalVolumeIdentifier[128];
	Uint32 LogicalBlockSize;
	struct EntityID DomainIdentifier;
	byte LogicalVolumeContentsUse[16];
	Uint32 MapTableLength;
	Uint32 NumberofPartitionMaps;
	struct EntityID ImplementationIdentifier;
	byte ImplementationUse[128];
	extent_ad IntegritySequenceExtent;
	byte PartitionMaps[];
};

struct UnallocatedSpaceDesc 
{ /* ECMA 167 3/10.8 */
	struct tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	Uint32 NumberofAllocationDescriptors;
	extent_ad AllocationDescriptors[];
};

struct LogicalVolumeIntegrityDesc 
{ /* ECMA 167 3/10.10 */
	struct tag DescriptorTag;
	Timestamp RecordingDateAndTime;
	Uint32 IntegrityType;
	struct extent_ad NextIntegrityExtent;
	byte LogicalVolumeContentsUse[32];
	Uint32 NumberOfPartitions;
	Uint32 LengthOfImplementationUse; /* = L_IU */
	Uint32 FreeSpaceTable[];
	Uint32 SizeTable[];
	byte ImplementationUse[];
};

struct ImpUseVolumeDescriptor 
{ /* ECMA 167 3/10.4 */
	struct tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	struct EntityID ImplementationIdentifier;
	byte ImplementationUse[460];
};

struct LVInformation {
	struct charspec LVICharset;
	dstring LogicalVolumeIdentifier[128];
	dstring LVInfo1[36];
	dstring LVInfo2[36];
	dstring LVInfo3[36];
	struct EntityID ImplementationID;
	byte ImplementationUse[128];
};

struct PartitionDescriptor 
{ /* ECMA 167 3/10.5 */
	struct tag DescriptorTag;
	Uint32 VolumeDescriptorSequenceNumber;
	Uint16 PartitionFlags;
	Uint16 PartitionNumber;
	struct EntityID PartitionContents;
	byte PartitionContentsUse[128];
	Uint32 AccessType;
	Uint32 PartitionStartingLocation;
	Uint32 PartitionLength;
	struct EntityID ImplementationIdentifier;
	byte ImplementationUse[128];
	byte Reserved[156];
};

// struct FileSetDescriptor 
// { /* ECMA 167 4/14.1 */
// 	struct tag DescriptorTag;
// 	struct timestamp RecordingDateandTime;
// 	Uint16 InterchangeLevel;
// 	Uint16 MaximumInterchangeLevel;
// 	Uint32 CharacterSetList;
// 	Uint32 MaximumCharacterSetList;
// 	Uint32 FileSetNumber;
// 	Uint32 FileSetDescriptorNumber;
// 	struct charspec LogicalVolumeIdentifierCharacterSet;
// 	dstring LogicalVolumeIdentifier[128];
// 	struct charspec FileSetCharacterSet;
// 	dstring FileSetIdentifier[32];
// 	dstring CopyrightFileIdentifier[32];
// 	dstring AbstractFileIdentifier[32];
// 	struct long_ad RootDirectoryICB;
// 	struct EntityID DomainIdentifier;
// 	struct long_ad NextExtent;
// 	struct long_ad SystemStreamDirectoryICB;
// 	byte Reserved[32];
// };

// struct PartitionHeaderDescriptor 
// { /* ECMA 167 4/14.3 */
// 	struct short_ad UnallocatedSpaceTable;
// 	struct short_ad UnallocatedSpaceBitmap;
// 	struct short_ad PartitionIntegrityTable;
// 	struct short_ad FreedSpaceTable;
// 	struct short_ad FreedSpaceBitmap;
// 	byte Reserved[88];
// };

// struct FileIdentifierDescriptor 
// { /* ECMA 167 4/14.4 */
// 	struct tag DescriptorTag;
// 	Uint16 FileVersionNumber;
// 	Uint8 FileCharacteristics;
// 	Uint8 LengthofFileIdentifier;
// 	struct long_ad ICB;
// 	Uint16 LengthOfImplementationUse;
// 	byte ImplementationUse[];
// 	char FileIdentifier[];
// 	byte Padding[];
// };

// struct icbtag 
// { /* ECMA 167 4/14.6 */
// 	Uint32 PriorRecordedNumberofDirectEntries;
// 	Uint16 StrategyType;
// 	byte StrategyParameter[2];
// 	Uint16 MaximumNumberofEntries;
// 	byte Reserved;
// 	Uint8 FileType;
// 	Lb_addr ParentICBLocation;
// 	Uint16 Flags;
// };

// struct FileEntry 
// { /* ECMA 167 4/14.9 */
// 	struct tag DescriptorTag;
// 	struct icbtag ICBTag;
// 	Uint32 Uid;
// 	Uint32 Gid;
// 	Uint32 Permissions;
// 	Uint16 FileLinkCount;
// 	Uint8 RecordFormat;
// 	Uint8 RecordDisplayAttributes;
// 	Uint32 RecordLength;
// 	Uint64 InformationLength;
// 	Uint64 LogicalBlocksRecorded;
// 	struct timestamp AccessTime;
// 	struct timestamp ModificationTime;
// 	struct timestamp AttributeTime;
// 	Uint32 Checkpoint;
// 	struct long_ad ExtendedAttributeICB;
// 	struct EntityID ImplementationIdentifier;
// 	Uint64 UniqueID,
// 	Uint32 LengthofExtendedAttributes;
// 	Uint32 LengthofAllocationDescriptors;
// 	byte ExtendedAttributes[];
// 	byte AllocationDescriptors[];
// };

// struct UnallocatedSpaceEntry 
// { /* ECMA 167 4/14.11 */
// 	struct tag DescriptorTag;
// 	struct icbtag ICBTag;
// 	Uint32 LengthofAllocationDescriptors;
// 	byte AllocationDescriptors[];
// };

// struct SpaceBitmap 
// { /* ECMA 167 4/14.12 */
// 	struct Tag DescriptorTag;
// 	Uint32 NumberOfBits;
// 	Uint32 NumberOfBytes;
// 	byte Bitmap[];
// };

struct AllocationExtentDescriptor 
{ /* ECMA 167 4/14.5 */
	struct tag DescriptorTag;
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