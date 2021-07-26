#pragma once
#pragma once
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <iostream>

namespace tdmsLib
{
	enum tdmsDataType : uint32_t {
		tdmsTypeVoid,
		tdmsTypeI8,
		tdmsTypeI16,
		tdmsTypeI32,
		tdmsTypeI64,
		tdmsTypeU8,
		tdmsTypeU16,
		tdmsTypeU32,
		tdmsTypeU64,
		tdmsTypeSingleFloat,
		tdmsTypeDoubleFloat,
		tdmsTypeExtendedFloat,
		tdmsTypeSingleFloatWithUnit = 0x19,
		tdmsTypeDoubleFloatWithUnit,
		tdmsTypeExtendedFloatWithUnit,
		tdmsTypeString = 0x20,
		tdmsTypeBoolean = 0x21,
		tdmsTypeTimeStamp = 0x44,
		tdmsTypeFixedPoint = 0x4F,
		tdmsTypeComplexSingleFloat = 0x08000c,
		tdmsTypeComplexDoubleFloat = 0x10000d,
		tdmsTypeDAQmxRawData = 0xFFFFFFFF
	};

	struct tdmsDataValue
	{
		virtual tdmsDataType getDataType() = 0;
		virtual ~tdmsDataValue()
		{
			//Nothing to delete but important to keep
		};
	};

	struct tdmsVoid :tdmsDataValue
	{
		tdmsDataType getDataType()
		{
			return tdmsTypeVoid;
		}

		~tdmsVoid()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsString : tdmsDataValue
	{
		int strLen;
		char * characters;

		tdmsDataType getDataType()
		{
			return tdmsTypeString;
		}

		~tdmsString()
		{
			if (strLen > 0) free(characters);
		}
	};

	struct tdmsI8 :tdmsDataValue
	{
		int8_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeI8;
		}
		~tdmsI8()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsI16 :tdmsDataValue
	{
		int16_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeI16;
		}
		~tdmsI16()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsI32 :tdmsDataValue
	{
		int32_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeI32;
		}
		~tdmsI32()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsI64 :tdmsDataValue
	{
		int64_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeI64;
		}
		~tdmsI64()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsU8 :tdmsDataValue
	{
		uint8_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeU8;
		}
		~tdmsU8()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsU16 :tdmsDataValue
	{
		uint16_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeU16;
		}
		~tdmsU16()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsU32 :tdmsDataValue
	{
		uint32_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeU32;
		}
		~tdmsU32()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsU64 :tdmsDataValue
	{
		uint64_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeU64;
		}
		~tdmsU64()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsSingleFloat :tdmsDataValue
	{
		float value;
		tdmsDataType getDataType()
		{
			return tdmsTypeSingleFloat;
		}
		~tdmsSingleFloat()
		{
			//Nothing to delete but important to keep
		}
	};
	struct tdmsDoubleFloat :tdmsDataValue
	{
		double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeDoubleFloat;
		}
		~tdmsDoubleFloat()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsExtendedFloat :tdmsDataValue
	{
		long double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeExtendedFloat;
		}
		~tdmsExtendedFloat()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsSingleFloatWithUnit :tdmsDataValue
	{
		float value;
		tdmsDataType getDataType()
		{
			return tdmsTypeSingleFloatWithUnit;
		}
		~tdmsSingleFloatWithUnit()
		{
			//Nothing to delete but important to keep
		}
	};
	struct tdmsDoubleFloatWithUnit :tdmsDataValue
	{
		double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeDoubleFloatWithUnit;
		}
		~tdmsDoubleFloatWithUnit()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsExtendedFloatWithUnit :tdmsDataValue
	{
		long double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeExtendedFloatWithUnit;
		}
		~tdmsExtendedFloatWithUnit()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsBoolean :tdmsDataValue
	{
		bool value;
		tdmsDataType getDataType()
		{
			return tdmsTypeBoolean;
		}
		~tdmsBoolean()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsTimeStamp :tdmsDataValue
	{
		int64_t secFromEpoch;
		uint64_t fractionsOfSec;
		tdmsDataType getDataType()
		{
			return tdmsTypeTimeStamp;
		}
		~tdmsTimeStamp()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsFixedPoint :tdmsDataValue
	{
		//This is the first of the what? data types, there is missing information on how these should be handled,
		//However this is not important, make a note of it and move on
		uint64_t value;
		tdmsDataType getDataType()
		{
			return tdmsTypeFixedPoint;
		}
		~tdmsFixedPoint()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsComplexSingleFloat :tdmsDataValue
	{
		float value;
		tdmsDataType getDataType()
		{
			return tdmsTypeSingleFloat;
		}
		~tdmsComplexSingleFloat()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsComplexDoubleFloat :tdmsDataValue
	{
		double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeComplexDoubleFloat;
		}
		~tdmsComplexDoubleFloat()
		{
			//Nothing to delete but important to keep
		}
	};

	struct tdmsDAQmxRawData :tdmsDataValue
	{
		double value;
		tdmsDataType getDataType()
		{
			return tdmsTypeDAQmxRawData;
		}
		~tdmsDAQmxRawData()
		{
			//Nothing to delete but important to keep
		}
	};


	tdmsDataValue * parseTdmsValue(tdmsDataType type, FILE * tdmsFile);
	const char * dataTypeToName(tdmsDataType type);

}