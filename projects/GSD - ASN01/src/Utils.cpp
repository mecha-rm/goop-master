#include "Utils.h"

// conversion of a string to lower case
std::string toLower(std::string str)
{
	// returns the string if its of length zero.
	if (str.length() == 0)
		return str;

	std::string strL; // temporary string
	strL.assign(str); // copies original string

	for (int i = 0; i < strL.length(); i++)
	{
		strL.at(i) = tolower(strL.at(i)); // converts each 'char' to its lowercase version if applicable
	}

	return strL;
}

// converts a whole string to uppercase
std::string toUpper(std::string str)
{
	// returns the string if its of length zero.
	if (str.length() == 0)
		return str;

	std::string strL; // temporary string
	strL.assign(str); // copies original string

	for (int i = 0; i < strL.length(); i++)
	{
		strL.at(i) = toupper(strL.at(i)); // converts each 'char' to its uppercase version if applicable
	}

	return strL;
}

// capitalizes a string.
std::string captialize(std::string str)
{
	// returns the string if its of length zero.
	if (str.length() == 0)
		return str;

	std::string strL; // temporary string
	strL.assign(str); // copies original string

	str = toLower(str); // makes the string all lowercase
	str.at(0) = toupper(str.at(0)); // capitalizes the first letter

	return strL;
}

// checks if two strings are equal, ignoring case diffrences.
bool equalsIgnoreCase(std::string str1, std::string str2)
{
	// making both strings lowercase
	str1 = toLower(str1);
	str2 = toLower(str2);
	return str1 == str2; // returning whether the two are equal
}

// replaces every instance of a set of characters in a string with another set of characters.
std::string replaceSubstring(std::string str, std::string oldSubstr, std::string newSubstr, bool ignoreCase)
{
	unsigned int index; // the index of the most recently found oldSubstr

	// first set is if ignoreCase is false, second set is if ignoreCase is true.
	while ((ignoreCase == false && str.find(oldSubstr) != std::string::npos) ||
		(ignoreCase == true && toLower(str).find(toLower(oldSubstr)) != std::string::npos))
	{
		// checks if case sensitivity was requested.
		index = (ignoreCase) ? index = toLower(str).find(toLower(oldSubstr)) : index = str.find(oldSubstr);

		str.replace(index, oldSubstr.length(), newSubstr); // replaces the old string with the new string.
	}

	return str;
}



