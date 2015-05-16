#pragma once

#include "stdafx.h"
#include "Track.h"
#include "Album.h"


class FolderToProcess
{
public:
	const bfs::path folder;
	list<bfs::path> files;
	list<Track> tracks;
	Album album;

	explicit FolderToProcess(const bfs::path& folder);
};
