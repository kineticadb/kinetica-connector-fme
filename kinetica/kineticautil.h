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

using namespace std;

class IFMELogFile;
class IFMEMappingFile;

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

#endif
