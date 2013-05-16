#pragma once

#include "UDF.h"

struct FDiskData
{
	timestamp 	*recordingDateAndTime;
	dstring		volumeIdentifier[32];
	uint16_t	version;
	uint64_t	totalSpace;
	uint64_t	freeSpace;
};