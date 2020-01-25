#pragma once

#include <memory>
#include <string>
#include <vector>

// the value of TypeName gets replaced with whatever was brouught in.

// preventing copies of certain game objects.
#define NoCopy(TypeName) \
	TypeName(const TypeName& other) = delete; \
	TypeName& operator =(const TypeName& other) = delete;

#define NoMove(TypeName) \
	TypeName(const TypeName&& other) = delete; \
	TypeName& operator =(const TypeName&& other) = delete;

#define GraphicsClass(TypeName)\
	typedef std::shared_ptr<TypeName> Sptr;\
	NoMove(TypeName);\
	NoCopy(TypeName); \
	std::string DebugName;

// EX //
// These utilities were taken from my own personal files. I put them here because the file they came from was also called Utils.h

// Standard Utilities
// converts a whole string to lowercase
std::string toLower(std::string);

// converts a whole string to uppercase
std::string toUpper(std::string);

// capitalizes a string, making all but the first character lowercase.
std::string captialize(std::string);

// checks if two strings are equal, ignoring case diffrences.
bool equalsIgnoreCase(std::string, std::string);

// replaces every instance of a set of characters in a string with another set of characters.
// oldSubstr: the string segment that's being removed
// newSubstr: the string that will be put in its place.
// ignoreCase: if false, then the function is case sensitive. If true, then the function ignores cases when looking for oldSubstr.
std::string replaceSubstring(std::string str, std::string oldSubstr, std::string newSubstr, bool ignoreCase = false);

// splits the string into a vector of the provided data type using spaces.
template<typename T>
const std::vector<T> splitString(std::string str)
{
	std::stringstream ss; // the string stream.
	std::vector<T> vec; // the vector used for the vertex.
	T var; // used to store the item from the string.


	// if the string is of length 0, then an empty vector is returned.
	if (str.length() == 0)
		return std::vector<T>();

	ss.str(str); // stores the string in the stream

	while (ss >> var) // while the string stream isn't empty
	{
		// if the conversion failed, the string stream moves onto the next item.
		if (ss.bad())
			continue;

		vec.push_back(var); // saves in the vector
	}

	return vec; // returns the vector
}

// convets the string to the provided data type. Do note that the value might be truncated if the conversion doesn't make logical sense.
template<typename T>
T convertString(const std::string& str)
{
	std::stringstream ss; // the string stream.
	T val; // used to store the item from the string.

	ss.str(str); // stores the string in the stream

	ss >> val;

	return val;
}