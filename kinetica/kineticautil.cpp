/*=============================================================================

Name     : kineticautil.cpp

System   : FME Plug-in SDK

Language : C++

Purpose  : Declaration of UniverseDatabaseUtil

Copyright (c) 1994-2012, Safe Software Inc. All rights reserved.

Redistribution and use of this sample code in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:
* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SAMPLE CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SAMPLE CODE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/
#include "kineticautil.h"
#include "kineticapriv.h"

//#include <fmepluginutil.h>

#include "ilogfile.h"
#include "fmemap.h"
#include <vector>

#ifndef WIN32
#include <errno.h>
#endif

#include <cstring>
//IFMELogFile* gLogFile = NULL;
//===========================================================================
//


string trim(std::string const& str)
{
	if (str.empty())
		return str;

	std::size_t firstScan = str.find_first_not_of(' ');
	std::size_t first = firstScan == std::string::npos ? str.length() : firstScan;
	std::size_t last = str.find_last_not_of(' ');
	return str.substr(first, last - first + 1);
}


FME_Status getConnectionInfo(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile, FMEStringArray& connectionInfo, string keyword, string typeName)
{
	FMEString value;

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kURL, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		FMEString fmeStr;
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
		gLogFile->logMessageString("No URL was entered.");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kDataSet, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
		// gLogFile->logMessageString("No kDataSet was entered.");
	}


	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kTableName, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
		// gLogFile->logMessageString("No kTableName was entered.");
	}


	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kCollection, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
	}


	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kGeometry, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kUserName, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
		// gLogFile->logMessageString("No username was entered.");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kPassword, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
		// gLogFile->logMessageString("No password was entered.");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kPointX, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kPointY, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
	}

	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kExpression, *value))
	{
		// Mapping has been found, set the user id string and log the data.
		connectionInfo->append(value);
	}
	else
	{
		// No mapping found.
		connectionInfo->append("");
	}

	return FME_SUCCESS;
}

//===========================================================================
//
void getWriterMode(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile, string keyword, string typeName)
{
	FMEString value;

	// Attempt to grab the writer mode from the mapping file
	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kWriterMode, *value))
	{
		// Mapping has been found, log the data.
		gLogFile->logMessageString(value->data(), FME_WARN);
	}
	else
	{
		// No mapping found.
		gLogFile->logMessageString("No writer mode found.");
	}
}

//===========================================================================
//
void getTransactionSettings(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile, string keyword, string typeName)
{
	FMEString value;

	// STUB: we will want to convert these values to FME_UInt32 and return them so we can utilize them.

	// Attempt to grab the writer mode from the mapping file
	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kStartTransaction, *value))
	{
		// Mapping has been found, log the data.
		gLogFile->logMessageString(value->data(), FME_WARN);
	}
	else
	{
		// No mapping found.
		gLogFile->logMessageString("No start transaction was found.");
	}

	// Attempt to grab the writer mode from the mapping file
	if (gMappingFile->fetchWithPrefix(keyword.c_str(), typeName.c_str(), kTransactionInterval, *value))
	{
		// Mapping has been found, log the data.
		gLogFile->logMessageString(value->data(), FME_WARN);
	}
	else
	{
		// No mapping found.
		gLogFile->logMessageString("No transaction interval was found.");
	}
}

//===========================================================================
//
void wrapInQuotations(FMEString& str)
{
	string wrappedString = str->data();
	size_t pos;

	// Escape the '\' with '\'.
	pos = wrappedString.find_first_of("\\");
	while (pos != string::npos)
	{
		wrappedString.insert(pos, "\\");
		pos = pos + 2;
		pos = wrappedString.find_first_of("\\", pos);
	}

	// Replace the '\n' with "\\n".
	string returnString = "\n";
	pos = wrappedString.find_first_of(returnString);
	while (pos != string::npos)
	{
		wrappedString.erase(pos, returnString.length());
		wrappedString.insert(pos, "\\n");
		pos = pos + 2;
		pos = wrappedString.find_first_of(returnString, pos);
	}

	// Escape the '"' with '\'.
	pos = wrappedString.find_first_of("\"");
	while (pos != string::npos)
	{
		wrappedString.insert(pos, "\\");
		pos = pos + 2;
		pos = wrappedString.find_first_of("\"", pos);
	}

	// Enclose the string in quotations.
	wrappedString = "\"" + wrappedString + "\"";

	*str = wrappedString.c_str();
}

//===========================================================================
//
FME_Boolean typeEndsInZero(const char* attributeType)
{
	string attrType = attributeType;
	string endString = attrType.substr(attrType.length() - 2, 2);

	return endString == "0)" ? FME_TRUE : FME_FALSE;
}

//===========================================================================
//
FME_Boolean isSigned(const char* str)
{
	string num = str;
	string firstChar = num.substr(0, 1);

	return (firstChar == "-" || firstChar == "+") ? FME_TRUE : FME_FALSE;
}


//===========================================================================
//
FME_Boolean isTime(const char * time)
{
	if (strlen(time) < 6)
	{
		//Wrong length. Times must be in the form hhmmss[.x+][(+/-)zz]
		return FME_FALSE;
	}
	else
	{
		// case: hhmmss
		if (strlen(time) == 6)
		{
			return isTimeHHMMSS(time);
		}
		// check for decimal place, which indicates a fractional second.
		else if (strchr(time, '.') != NULL && strlen(time) > 7)
		{
			const char * decimal = strchr(time, '.');
			char * timezone;
			long secondDec = strtol(++decimal, &timezone, 0);

			// case: hhmmss[.x+]
			if (timezone == NULL)
			{
				// Do not allow negitive decimals
				if (secondDec > -1)
				{
					// Make sure the rest of the date is fine.
					return isTimeHHMMSS(time);
				}
				else
				{
					return FME_FALSE;
				}
			}
			// case: hhmmss[.x+][(+/-)zz]
			else if (secondDec > -1)
			{
				long timezoneNum = strtol(timezone, NULL, 0);
				if (timezoneNum > -13 && timezoneNum < 13)
				{
					// Make sure the rest of the date is fine.
					return isTimeHHMMSS(time);
				}
				else
				{
					return FME_FALSE;
				}
			}
			// If it doesn't fit any format above, it's invalid.
			else
			{
				return FME_FALSE;
			}
		}

		// case: hhmmss[(+/-)zz]
		else if (strlen(time) == 9)
		{
			// Get the timezone as a long.
			char * timezoneChar = new char[4];
			strncpy(timezoneChar, &time[6], 3);
			long timezone = strtol(timezoneChar, NULL, 0);
			delete[] timezoneChar;
			// There are 24 timezones, with -12 through +12 in
			// relation to GMT
			if (timezone > -13 && timezone < 13)
			{
				// Make sure the rest of the date is fine.
				return isTimeHHMMSS(time);
			}
			else
			{
				return FME_FALSE;
			}
		}
		else
		{
			// Invalid time format.
			return FME_FALSE;
		}
	}
}
//===========================================================================
//
FME_Boolean isTimeHHMMSS(const char * time)
{
	// Seperate the hour, minute and second components
	// and cast to integers.
	char * hourChar = new char[3];
	strncpy(hourChar, &time[0], 2);
	char * minuteChar = new char[3];
	strncpy(minuteChar, &time[2], 2);
	char * secondChar = new char[3];
	strncpy(secondChar, &time[4], 2);

	int hour = atoi(hourChar);
	int minute = atoi(minuteChar);
	int second = atoi(secondChar);

	delete[] hourChar;
	delete[] minuteChar;
	delete[] secondChar;

	// Make sure each part of the time is correct.
	if (hour < 1 || hour > 24)
	{
		// invalid hour
		return FME_FALSE;
	}
	else if (minute < 0 || minute > 59)
	{
		// invalid minute
		return FME_FALSE;
	}
	else if (second < 0 || second > 59)
	{
		// invalid second
		return FME_FALSE;
	}
	//If we make it here, it's valid.
	return FME_TRUE;
}

//===========================================================================
//
FME_Boolean isDate(const char * date)
{
	if (atoi(date) == 0)
	{
		//Invalid. Either date was not a number or it was the number 00000000. 
		//The date 00000000 is not valid in the Gregorian Calendar.
		return FME_FALSE;
	}
	else
	{
		// Seperate the month and day components and cast to integers.
		// Any four-digit number is a valid year, so we won't check that.
		char * monthChar = new char[3];
		strncpy(monthChar, &date[4], 2);
		char * dayChar = new char[3];
		strncpy(dayChar, &date[6], 2);

		int month = atoi(monthChar);
		int day = atoi(dayChar);

		delete[] monthChar;
		delete[] dayChar;

		// We're being lenient. We aren't checking for invalid 
		// dates like Nov. 31, etc. 
		if (day < 1 || day > 31)
		{
			return FME_FALSE;
		}
		else if (month < 1 || month > 12)
		{
			return FME_FALSE;
		}
	}
	// No problems. It's a valid date.
	return FME_TRUE;
}

//===========================================================================
//
FME_Status convertToReal64(const char* str, FME_Real64& num)
{
	num = static_cast<FME_Real64>(strtod(str, NULL));

	return errno == ERANGE ? FME_FAILURE : FME_SUCCESS;
}

//===========================================================================
//
FME_Boolean isStringEqualZero(const char* str)
{
	string strString = str;

	// Remove front '+' character if it's there.
	if (strString.substr(0, 1) == "+")
	{
		strString = strString.substr(1, strString.size() - 1);
	}

	// Remove everything after '.' if it's there.
	size_t decimalLocation = strString.find_first_of(".");
	if (decimalLocation != string::npos)
	{
		strString = strString.substr(0, decimalLocation);
	}

	return strString == "0" ? FME_TRUE : FME_FALSE;
}

//===========================================================================
//
std::vector<string> split(const string &s, char delim) {
	stringstream ss(s);
	string item;
	std::vector<string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}