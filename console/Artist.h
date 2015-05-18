#pragma once


class Artist
{
public:
	string name;
	string type;
	string description;
	string gender;
	string location;

	string musicbrainz_artistId;

	set<string> aliases;
	list<string> tags;

	Artist();
	explicit Artist(const string& name);

	bool isEmpty();
};
