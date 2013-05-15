#pragma once

#include "UDF.h"

struct FDiskData
{
	timestamp 	*recordingDateAndTime;	
	uint8_t		identifierSuffix[8];
};