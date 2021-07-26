#include "tdmsDataValues.h"

namespace tdmsLib
{
	tdmsDataValue * parseTdmsValue(tdmsDataType type, FILE * tdmsFile)
	{
		switch (type)
		{
			case	tdmsTypeVoid: {return new tdmsVoid; }break;
			case	tdmsTypeI8:
			{
				tdmsI8 * retval = new tdmsI8();
				fread(&(retval->value), sizeof(int8_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeI16:
			{
				tdmsI16 * retval = new tdmsI16();
				fread(&(retval->value), sizeof(int16_t), 1, tdmsFile);
				return retval;

			}break;
			case	tdmsTypeI32:
			{
				tdmsI32 * retval = new tdmsI32();
				fread(&(retval->value), sizeof(int32_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeI64:
			{
				tdmsI64 * retval = new tdmsI64();
				fread(&(retval->value), sizeof(int64_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeU8:
			{
				tdmsU8 * retval = new tdmsU8();
				fread(&(retval->value), sizeof(uint8_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeU16:
			{
				tdmsU16 * retval = new tdmsU16();
				fread(&(retval->value), sizeof(uint16_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeU32:
			{
				tdmsU32 * retval = new tdmsU32();
				fread(&(retval->value), sizeof(uint32_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeU64:
			{
				tdmsU64 * retval = new tdmsU64();
				fread(&(retval->value), sizeof(uint64_t), 1, tdmsFile);
				return retval;
			}
			break;
			case	tdmsTypeSingleFloat:
			{
				tdmsSingleFloat * retval = new tdmsSingleFloat();
				fread(&(retval->value), sizeof(float), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeDoubleFloat:
			{
				tdmsDoubleFloat * retval = new tdmsDoubleFloat();
				fread(&(retval->value), sizeof(double), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeExtendedFloat:
			{
				tdmsExtendedFloat * retval = new tdmsExtendedFloat();
				fread(&(retval->value), sizeof(long double), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeSingleFloatWithUnit:
			{
				tdmsSingleFloatWithUnit * retval = new tdmsSingleFloatWithUnit();
				fread(&(retval->value), sizeof(float), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeDoubleFloatWithUnit:
			{
				tdmsDoubleFloatWithUnit * retval = new tdmsDoubleFloatWithUnit();
				fread(&(retval->value), sizeof(double), 1, tdmsFile);
				return retval;
			}break;
			case		tdmsTypeExtendedFloatWithUnit:
			{
				tdmsExtendedFloatWithUnit * retval = new tdmsExtendedFloatWithUnit();
				fread(&(retval->value), sizeof(long double), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeString:
			{
				tdmsString * retval = new tdmsString();
				fread(&(retval->strLen), sizeof(uint32_t), 1, tdmsFile);
				retval->characters = (char *)calloc(retval->strLen * sizeof(char) + 1, sizeof(char));
				fread((retval->characters), sizeof(char), retval->strLen, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeBoolean:
			{
				tdmsBoolean * retval = new tdmsBoolean();
				fread(&(retval->value), sizeof(uint8_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeTimeStamp:
			{
				tdmsTimeStamp * retval = new tdmsTimeStamp();
				fread(&(retval->fractionsOfSec), sizeof(uint64_t), 1, tdmsFile);
				fread(&(retval->secFromEpoch), sizeof(int64_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeFixedPoint:
			{
				tdmsFixedPoint * retval = new tdmsFixedPoint();
				fread(&(retval->value), sizeof(int64_t), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeComplexSingleFloat:
			{
				tdmsComplexSingleFloat * retval = new tdmsComplexSingleFloat();
				fread(&(retval->value), sizeof(float), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeComplexDoubleFloat:
			{
				tdmsComplexDoubleFloat * retval = new tdmsComplexDoubleFloat();
				fread(&(retval->value), sizeof(double), 1, tdmsFile);
				return retval;
			}break;
			case	tdmsTypeDAQmxRawData:
			{
				tdmsDAQmxRawData * retval = new tdmsDAQmxRawData();
				fread(&(retval->value), sizeof(double), 1, tdmsFile);
				return retval;
			} break;
		}

		return NULL;
	}

	const char * dataTypeToName(tdmsDataType type)
	{
		switch (type)
		{
			case	tdmsTypeVoid: {return "Void"; }

			case	tdmsTypeI8: { return "I8"; };

			case	tdmsTypeI16: { return "I16"; };

			case	tdmsTypeI32: { return "I32"; };

			case	tdmsTypeI64: { return "I64"; };

			case	tdmsTypeU8: { return "U8"; };

			case	tdmsTypeU16: { return "U16"; };

			case	tdmsTypeU32: { return "U32"; };

			case	tdmsTypeU64: { return "U64"; };

			case	tdmsTypeSingleFloat: { return "Single Float"; };

			case	tdmsTypeDoubleFloat: { return "Double Float"; };

			case	tdmsTypeExtendedFloat: { return "Extended Float"; };

			case	tdmsTypeSingleFloatWithUnit: { return "Single Float With Unit"; };

			case	tdmsTypeDoubleFloatWithUnit: { return "Double Float With Unit"; };

			case	tdmsTypeExtendedFloatWithUnit: { return "Extended Float With Unit"; };

			case	tdmsTypeString: { return "String"; };

			case	tdmsTypeBoolean: { return "Boolean"; };

			case	tdmsTypeTimeStamp: { return "Time Stamp"; };

			case	tdmsTypeFixedPoint: { return "Fixed Point"; };

			case	tdmsTypeComplexSingleFloat: { return "Complex Single Float"; };

			case	tdmsTypeComplexDoubleFloat: { return "Complex Single Float"; };

			case	tdmsTypeDAQmxRawData: { return "DAQmx Raw Data"; };
		}

		return "";
	}

}