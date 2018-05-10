#ifndef KINETICA_DATABASE_UTIL_H
#define KINETICA_DATABASE_UTIL_H
/*=============================================================================

Name     : kineticautil.h

System   : FME Plug-in SDK

Language : C++

Purpose  : Declaration of KineticaUtil

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

#include "fmestring.h"

#include <string>
#include <stdio.h>
#include <sstream>
#include <vector>

// FME Plug-in SDK includes
#include <fmetypes.h>

using namespace std;

class IFMELogFile;
class IFMEMappingFile;



//------------------------------------------------------------------------------
// Useful Logging Macros
// ----------------------
// Create an inline std::stringstream stream object and return the
// std::string from it.
// E.g. std::string msg( KINETICA_STREAM_TO_STRING( "Hello " << user_name << ", how are you?" ) );
// Note: Extra cast needed for gcc 4.8.7 (at least).
#define KINETICA_STREAM_TO_STRING(...) ( static_cast<const std::ostringstream&> (std::ostringstream() << __VA_ARGS__).str() )

// Create an inline std::stringstream stream object and return the
// char array from it.
// E.g. std::string msg( KINETICA_STREAM_TO_STRING( "Hello " << user_name << ", how are you?" ) );
#define KINETICA_STREAM_TO_CSTRING(...) ( KINETICA_STREAM_TO_STRING(__VA_ARGS__).c_str() )

// Log a informational message
#define LOG_KINETICA_INFO(logger, ...) ( logger->logMessageString( KINETICA_STREAM_TO_STRING( "Kinetica: " << __VA_ARGS__).c_str() ) )

// Log a warning 
#define LOG_KINETICA_WARN(logger, ...) ( logger->logMessageString( KINETICA_STREAM_TO_STRING( "Kinetica Warning: " << __VA_ARGS__).c_str(), \
                                         FME_WARN ) )

// Log an error
#define LOG_KINETICA_ERROR(logger, ...) ( logger->logMessageString( KINETICA_STREAM_TO_STRING( "Kinetica Error: " << __VA_ARGS__).c_str(), \
                                         FME_ERROR ) )
#define LOG_KINETICA_READER_ERROR(logger, ...) ( logger->logMessageString( KINETICA_STREAM_TO_STRING( "Kinetica Reader Error: " << __VA_ARGS__).c_str(), \
                                         FME_ERROR ) )
#define LOG_KINETICA_WRITER_ERROR(logger, ...) ( logger->logMessageString( KINETICA_STREAM_TO_STRING( "Kinetica Writer Error: " << __VA_ARGS__).c_str(), \
                                         FME_ERROR ) )

// Get the proper ordinal word (0th, 1st, 2nd, 3rd, 4th, ...)
#define KINETICA_GET_ORDINAL_NUMBER(number) ( (number == 1) ? "1st" :  \
                                              ((number == 2) ? "2nd" : \
                                               (number == 3) ? "3rd" : \
                                                KINETICA_STREAM_TO_CSTRING( number << "th" ) ) )



//------------------------------------------------------------------------------
// Some string constants
const std::string KINETICA_PROPERTY_NOT_NULLABLE = "not_nullable";
const std::string KINETICA_PROPERTY_PRIMARY_KEY  = "primary_key";

//------------------------------------------------------------------------------
// Trim whitespace off the ends of a string
string trim(std::string const& str);

//------------------------------------------------------------------------------
// Reads Settings box values
FME_Status getConnectionInfo(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile,
	FMEStringArray& connectionInfo, string keyword, string typeName);

//------------------------------------------------------------------------------
// Get the writer mode for future use
void getWriterMode(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile, string keyword, string typeName);

//------------------------------------------------------------------------------
// Get the transaction settings for future use
void getTransactionSettings(IFMEMappingFile* gMappingFile, IFMELogFile* gLogFile, string keyword, string typeName);

//------------------------------------------------------------------
// This method will put quotation marks around "str".  It will also
// escape all '"' and '\' characters in "str" with '\' and replace
// all new line '\n' with "\\n".
// For example, a string and its wrapped value is:
//    I said "hello
//     buddy\".
//    ->
//    "I said \"hello\n buddy\\\"."
void wrapInQuotations(FMEString& str);

//------------------------------------------------------------------
// This method returns FME_TRUE if the attribute type ends in a "0)",
// otherwise it returns FME_FALSE.
FME_Boolean typeEndsInZero(const char* attributeType);

//------------------------------------------------------------------
// This method returns FME_TRUE if the string has a '-' or '+' as
// its first character.
FME_Boolean isSigned(const char* str);

//------------------------------------------------------------------
// Converts a string to a double.  FME_FAILURE is
// returned if there is an overflow in the conversion.  The resulting
// integer has an unknown value.
FME_Status convertToReal64(const char* str, FME_Real64& num);

//------------------------------------------------------------------
// Checks whether the string value is equal '0'.  Leading '+' and
// characters including and after '.' are not involved in the check.
FME_Boolean isStringEqualZero(const char* str);

//------------------------------------------------------------------
// This method determines if the passed in string represents a
// valid FME-style date.
// Assumes ASCII encoding. 
FME_Boolean isDate(const char* str);

//------------------------------------------------------------------
// This method determines if the passed in string represents a
// valid FME-style time.
// Assumes ASCII encoding. 
FME_Boolean isTime(const char* str);

//------------------------------------------------------------------
// This method determines if the passed in string represents a
// valid FME-style time without fractional seconds or timezones.
// Assumes ASCII encoding. 
FME_Boolean isTimeHHMMSS(const char * time);

//------------------------------------------------------------------
// Converts a string value to a number of type T.  FME_FAILURE is
// returned if the string value could not be converted, FME_SUCCESS
// otherwise.
template <class T>
FME_Status convertToInt(const char* str, T& num)
{
	num = 0;
	string strString = str;
	istringstream iss(strString);
	iss >> num;

	if (num == 0)
	{
		// The conversion may not have worked.  Check that the original
		// string was a "0" value.
		return isStringEqualZero(str) ? FME_SUCCESS : FME_FAILURE;
	}

	return FME_SUCCESS;
}

std::vector<string> split(const string &s, char delim);

//------------------------------------------------------------------------------
// Date, datetime, and timestamp related utility functions.

/*
 * For a given date value, check if the value is out of the allowed
 * range of years ([1000, 2900]); if so, change it to the minimum/maximum value.
 */
void fixOutOfBoundsDate( IFMELogFile* logger, char* dt_value );

/*
 * For a given datetime value, check if the value is out of the allowed
 * range of years ([1000, 2900]); if so, change it to the minimum/maximum value.
 */
void fixOutOfBoundsDatetime( IFMELogFile* logger, char* dt_value );

/*
* For a given timestamp (long) value, check if the value is out of the allowed
* range of [-30610224000000, 29379542399999]; if so, change it to the
* minimum/maximum value.  Return the corrected value (or the unchanged value).
*/
std::string fixOutOfBoundsTimestamp( IFMELogFile* logger,
                                     const std::string& timestamp_value );

#endif
