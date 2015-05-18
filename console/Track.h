#pragma once


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

	string musicbrainz_trackId;
	string musicbrainz_albumId;
	string musicbrainz_artistId;
	string musicbrainz_albumArtistId;

	Track();
};
