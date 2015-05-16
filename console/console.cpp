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

static bool isAudioFile(const bfs::path& file) {
	string ext = bfs::extension(file);
	if (ext.length() < 2)
		return false;

	ext = ext.substr(1);
	ba::to_lower(ext);

	return TagLib::FileRef::defaultFileExtensions().contains(ext);
}

void listFoldersToProcess(list<FolderToProcess>* result, const bfs::path& rootFolder) {
	list<bfs::path> toProcess;
	toProcess.push_front(rootFolder);

	while (!toProcess.empty()) {
		bfs::path folder = toProcess.front();
		toProcess.pop_front();

		cout << "Listing contents of " << folder << " ...\n";

		FolderToProcess candidate(folder);

		for (auto&& file : bfs::directory_iterator(folder)) {
			if (bfs::is_directory(file))
				toProcess.push_front(file);

			else if (isAudioFile(file))
				candidate.files.push_back(file);
		}

		if (!candidate.files.empty()) {
			candidate.files.sort([](const bfs::path& a, const bfs::path& b) {
				         return ba::ilexicographical_compare(a.string(), b.string());
			         });

			cout << "    found " << candidate.files.size() << " music files\n";

			result->push_back(candidate);
		}
	}
}

static string getString(const TagLib::PropertyMap& props, const char* name, const char* defVal = "") {
	auto&& prop = props[name];

	if (prop.size() < 1)
		return defVal;
	else
		return prop[0].to8Bit(true);

}

static int getInt(const TagLib::PropertyMap& props, const char* name, int defVal = 0) {
	auto&& prop = props[name];

	if (prop.size() < 1)
		return defVal;
	else
		return prop[0].toInt();

}

static string coalesce(const string& first, const string& second) {
	if (first.length() > 0)
		return first;
	else
		return second;
}

void ReadTrackInfo(FolderToProcess* folder) {
	for (auto&& file : folder->files) {
		TagLib::FileRef tagfile(file.c_str());

		if (tagfile.isNull())
			continue;

		TagLib::PropertyMap&& props = tagfile.tag()->properties();

		Track track;
		track.file = file;

		track.disk = getInt(props, "DISCNUMBER", 1);
		track.number = getInt(props, "TRACKNUMBER");
		track.title = getString(props, "TITLE");
		track.album = getString(props, "ALBUM");
		track.artist = getString(props, "ARTIST");
		track.albumArtist = coalesce(getString(props, "ALBUMARTIST"), track.artist);
		track.genre = getString(props, "GENRE");

		track.length = tagfile.audioProperties()->length();
		track.bitrate = tagfile.audioProperties()->bitrate();

		track.musicbrainz.trackId = getString(props, "MUSICBRAINZ_TRACKID");
		track.musicbrainz.albumId = getString(props, "MUSICBRAINZ_ALBUMID");
		track.musicbrainz.artistId = getString(props, "MUSICBRAINZ_ARTISTID");
		track.musicbrainz.albumArtistId = getString(props, "MUSICBRAINZ_ALBUMARTISTID");

		folder->tracks.push_back(track);
	}
}

struct InsensitiveCompare
{
	bool operator()(const string& a, const string& b) const {
		return _stricmp(a.c_str(), b.c_str()) < 0;
	}
};

static Album FindAlbum(const list<Track>& tracks, bool allowMissing) {
	set<string, InsensitiveCompare> artist;
	set<string, InsensitiveCompare> album;

	for (auto&& track : tracks) {
		if (!track.albumArtist.empty())
			artist.insert(track.albumArtist);
		else if (!allowMissing)
			return Album();

		if (!track.album.empty())
			album.insert(track.album);
		else if (!allowMissing)
			return Album();
	}

	if (artist.size() != 1 || album.size() != 1)
		return Album();

	return Album(*artist.begin(), *album.begin());
}

void DetectAlbum(FolderToProcess* folder) {
	folder->album = FindAlbum(folder->tracks, false);
	if (!folder->album.isEmpty())
		return;

	folder->album = FindAlbum(folder->tracks, true);
	if (!folder->album.isEmpty())
		return;

}

int _tmain(int argc, _TCHAR* argv[]) {

	list<FolderToProcess> foldersToProcess;

	listFoldersToProcess(&foldersToProcess, argv[1]);

	foldersToProcess.sort([](const FolderToProcess& a, const FolderToProcess& b) {
		return ba::ilexicographical_compare(a.folder.string(), b.folder.string());
	});

	for (auto&& folder : foldersToProcess) {
		cout << "Reading tags of " << folder.folder << " ...\n";
		ReadTrackInfo(&folder);
	}

	foldersToProcess.remove_if([](const FolderToProcess& f) {
		return f.tracks.empty();
	});

	for (auto&& folder : foldersToProcess) {
		cout << "Processing " << folder.folder << " ...\n";
		DetectAlbum(&folder);
	}


	return 0;
}
