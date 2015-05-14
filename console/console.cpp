#include "stdafx.h"

#include "FolderToProcess.h"

static bfs::path make_relative(bfs::path from, bfs::path to) {
	from = bfs::absolute(from);
	to = bfs::absolute(to);

	bfs::path::const_iterator fromIter = from.begin();
	bfs::path::const_iterator toIter = to.begin();

	// Loop through both
	while (fromIter != from.end() && toIter != to.end() && (*toIter) == (*fromIter)) {
		++toIter;
		++fromIter;
	}

	bfs::path result;
	while (fromIter != from.end()) {
		result /= "..";
		++fromIter;
	}

	while (toIter != to.end()) {
		result /= *toIter;
		++toIter;
	}

	return result;
}

static bool isAudioFile(const bfs::path &file) {
	string ext = bfs::extension(file);
	if (ext.length() < 2)
		return false;

	ext = ext.substr(1);
	ba::to_lower(ext);

	return TagLib::FileRef::defaultFileExtensions().contains(ext);
}

void listFoldersToProcess(list<FolderToProcess> *result, const bfs::path &rootFolder) {
	list<bfs::path> toProcess;
	toProcess.push_front(rootFolder);

	while (!toProcess.empty()) {
		bfs::path folder = toProcess.front();
		toProcess.pop_front();

		cout << "Listing contents of " << folder << " ...\n";

		FolderToProcess candidate(folder);

		for (auto &&file : bfs::directory_iterator(folder)) {
			if (bfs::is_directory(file))
				toProcess.push_front(file);

			else if (isAudioFile(file))
				candidate.files.push_back(file);
		}

		if (!candidate.files.empty()) {
			candidate.files.sort([](const bfs::path &a, const bfs::path &b) {
				         return ba::ilexicographical_compare(a.string(), b.string());
			         });

			cout << "    found " << candidate.files.size() << " music files\n";

			result->push_back(candidate);
		}
	}
}

int _tmain(int argc, _TCHAR *argv[]) {

	list<FolderToProcess> foldersToProcess;

	listFoldersToProcess(&foldersToProcess, argv[1]);

	foldersToProcess.sort([](const FolderToProcess &a, const FolderToProcess &b) {
		return ba::ilexicographical_compare(a.folder.string(), b.folder.string());
	});

	for (auto &&folder : foldersToProcess) {
		cout << "Processing " << folder.folder << " ...\n";
	}


	//TagLib::FileRef f("Latex Solar Beef.mp3");
	//TagLib::String artist = f.tag()->artist(); // artist == "Frank Zappa"
	//f.tag()->setAlbum("Fillmore East");
	//f.save();
	//TagLib::FileRef g("Free City Rhymes.ogg");
	//TagLib::String album = g.tag()->album(); // album == "NYC Ghosts & Flowers"
	//g.tag()->setTrack(1);
	//g.save();


	return 0;
}
