#pragma once

#include <string>


class Artist
{
public:
	string name;
	string type;
	string description;
	string gender;
	string location;

	string musicbrainz_artistId;

	Artist();
	explicit Artist(const string& name);

	bool isEmpty();
};
