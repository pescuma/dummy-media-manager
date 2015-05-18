#include "stdafx.h"
#include "utf8.h"

#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>


string toUpper(const string& str)
{
	return bl::to_upper(str);
}

string toLower(const string& str)
{
	return bl::to_lower(str);
}

string removeAccents(const string& str)
{
	UnicodeString source = UnicodeString::fromUTF8(str.c_str());

	UErrorCode status = U_ZERO_ERROR;
	Transliterator* accentsConverter = Transliterator::createInstance("NFD; [:M:] Remove; NFC", UTRANS_FORWARD, status);

	accentsConverter->transliterate(source);

	delete accentsConverter;

	string result;
	source.toUTF8String(result);

	return result;
}

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
unsigned int computeEditDistance(const string& s1, const string& s2)
{
	const size_t len1 = s1.size(), len2 = s2.size();
	vector<unsigned int> col(len2 + 1), prevCol(len2 + 1);

	for (unsigned int i = 0; i < prevCol.size(); i++)
		prevCol[i] = i;
	for (unsigned int i = 0; i < len1; i++)
	{
		col[0] = i + 1;
		for (unsigned int j = 0; j < len2; j++)
			col[j + 1] = min(min(prevCol[1 + j] + 1, col[j] + 1), prevCol[j] + (s1[i] == s2[j] ? 0 : 1)) ;
		col.swap(prevCol);
	}
	return prevCol[len2];
}
