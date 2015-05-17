#pragma once

#include "stdafx.h"


class Album
{
public:
	string artist;
	string name;
	string description;

	string musicbrainz_albumId;
	string musicbrainz_artistId;

	Album();
	Album(const string& artist, const string& name);

	bool isEmpty();
};
