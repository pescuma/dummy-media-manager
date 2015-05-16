#pragma once

#include "stdafx.h"

class MusicbrainzData
{
public:
	string trackId;
	string albumId;
	string artistId;
	string albumArtistId;
};

class Track
{
public:
	bfs::path file;

	int disk;
	int number;
	string title;
	string album;
	string albumArtist;
	string artist;
	string genre;

	int length;
	int bitrate;

	MusicbrainzData musicbrainz;

	Track();
};
