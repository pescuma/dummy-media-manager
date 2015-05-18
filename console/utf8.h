#pragma once


string toUpper(const string& str);
string toLower(const string& str);
string removeAccents(const string& str);
unsigned int computeEditDistance(const string& s1, const string& s2);

typedef bl::comparator<char, bl::collator_base::secondary> CaseInsensitiveCompare;
typedef bl::comparator<char, bl::collator_base::primary> CaseAndAccentsInsensitiveCompare;
