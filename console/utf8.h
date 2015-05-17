#pragma once

#include <string>


std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string removeAccents(const std::string& str);
unsigned int computeEditDistance(const std::string& s1, const std::string& s2);

typedef boost::locale::comparator<char, boost::locale::collator_base::secondary> CaseInsensitiveCompare;
typedef boost::locale::comparator<char, boost::locale::collator_base::primary> CaseAndAccentsInsensitiveCompare;
