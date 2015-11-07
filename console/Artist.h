#pragma once
#include <utility>


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


	Artist(const Artist& other)
		: name(other.name),
		  type(other.type),
		  description(other.description),
		  gender(other.gender),
		  location(other.location),
		  musicbrainz_artistId(other.musicbrainz_artistId),
		  aliases(other.aliases),
		  tags(other.tags) {
	}

	Artist(Artist&& other)
		: name(std::move(other.name)),
		  type(std::move(other.type)),
		  description(std::move(other.description)),
		  gender(std::move(other.gender)),
		  location(std::move(other.location)),
		  musicbrainz_artistId(std::move(other.musicbrainz_artistId)),
		  aliases(std::move(other.aliases)),
		  tags(std::move(other.tags)) {
	}

	Artist& operator=(const Artist& other) {
		if (this == &other)
			return *this;
		name = other.name;
		type = other.type;
		description = other.description;
		gender = other.gender;
		location = other.location;
		musicbrainz_artistId = other.musicbrainz_artistId;
		aliases = other.aliases;
		tags = other.tags;
		return *this;
	}

	Artist& operator=(Artist&& other) {
		if (this == &other)
			return *this;
		name = std::move(other.name);
		type = std::move(other.type);
		description = std::move(other.description);
		gender = std::move(other.gender);
		location = std::move(other.location);
		musicbrainz_artistId = std::move(other.musicbrainz_artistId);
		aliases = std::move(other.aliases);
		tags = std::move(other.tags);
		return *this;
	}

	bool isEmpty();
};
