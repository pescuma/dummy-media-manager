#pragma once

#include "stdafx.h"


class FolderToProcess
{
public:
	const bfs::path folder;
	list<bfs::path> files;

	explicit FolderToProcess(const bfs::path &folder);
};
