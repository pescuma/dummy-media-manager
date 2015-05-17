#pragma once

#include "stdafx.h"
#include "Track.h"
#include "Album.h"
#include "Artist.h"


class FolderToProcess
{
public:
	const bfs::path folder;
	list<bfs::path> files;
	list<Track> tracks;
	Artist artist;
	Album album;

	explicit FolderToProcess(const bfs::path& folder);
};
