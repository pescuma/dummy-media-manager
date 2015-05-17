#include "stdafx.h"

#include "FolderToProcess.h"
#include "MusicBrainz.h"

static bfs::path make_relative(bfs::path from, bfs::path to) {
	from = absolute(from);
	to = absolute(to);

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
	string ext = extension(file);
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

		track.musicbrainz_trackId = getString(props, "MUSICBRAINZ_TRACKID");
		track.musicbrainz_albumId = getString(props, "MUSICBRAINZ_ALBUMID");
		track.musicbrainz_artistId = getString(props, "MUSICBRAINZ_ARTISTID");
		track.musicbrainz_albumArtistId = getString(props, "MUSICBRAINZ_ALBUMARTISTID");

		folder->tracks.push_back(track);
	}
}

struct InsensitiveCompare
{
	bool operator()(const string& a, const string& b) const {
		return Poco::UTF8::icompare(a, b) < 0;
	}
};

static Album FindAlbumFromTracks(const list<Track>& tracks, bool allowMissing) {
	set<string, InsensitiveCompare> album;

	for (auto&& track : tracks) {
		if (!track.album.empty())
			album.insert(track.album);
		else if (!allowMissing)
			return Album();
	}

	if (album.size() != 1)
		return Album();

	return Album("", *album.begin());
}

void DetectAlbumFromTracks(FolderToProcess* folder) {
	folder->album = FindAlbumFromTracks(folder->tracks, false);
	if (!folder->album.isEmpty())
		return;

	folder->album = FindAlbumFromTracks(folder->tracks, true);
	if (!folder->album.isEmpty())
		return;
}

static Artist FindArtistFromTracks(const list<Track>& tracks, bool allowMissing) {
	set<string, InsensitiveCompare> artist;

	for (auto&& track : tracks) {
		if (!track.albumArtist.empty())
			artist.insert(track.albumArtist);
		else if (!allowMissing)
			return Artist();
	}

	if (artist.size() != 1)
		return Artist();

	return Artist(*artist.begin());
}

void DetectArtistFromTracks(FolderToProcess* folder) {
	folder->artist = FindArtistFromTracks(folder->tracks, false);
	if (!folder->artist.isEmpty())
		return;
}

#define d(i,j) dd[(i) * (m+2) + (j) ]
#define min(x,y) ((x) < (y) ? (x) : (y))
#define min3(a,b,c) ((a)< (b) ? min((a),(c)) : min((b),(c)))
#define min4(a,b,c,d) ((a)< (b) ? min3((a),(c),(d)) : min3((b),(c),(d)))

int dprint(int* dd, int n, int m) {
	int i, j;
	for (i = 0; i < n + 2; i++) {
		for (j = 0; j < m + 2; j++) {
			printf("%02d ", d(i, j));
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}

int dldist2(char* s, char* t, int n, int m) {
	int* dd;
	int i, j, cost, i1, j1, DB;
	int infinity = n + m;
	int DA[256 * sizeof(int)];

	memset(DA, 0, sizeof(DA));

	if (!(dd = (int*)malloc((n + 2) * (m + 2) * sizeof(int)))) {
		return -1;
	}

	d(0, 0) = infinity;
	for (i = 0; i < n + 1; i++) {
		d(i + 1, 1) = i;
		d(i + 1, 0) = infinity;
	}
	for (j = 0; j < m + 1; j++) {
		d(1, j + 1) = j;
		d(0, j + 1) = infinity;
	}
	dprint(dd, n, m);

	for (i = 1; i < n + 1; i++) {
		DB = 0;
		for (j = 1; j < m + 1; j++) {
			i1 = DA[t[j - 1]];
			j1 = DB;
			cost = ((s[i - 1] == t[j - 1]) ? 0 : 1);
			if (cost == 0)
				DB = j;
			d(i + 1, j + 1) =
					min4(d(i, j) + cost,
						d(i + 1, j) + 1,
						d(i, j + 1) + 1,
						d(i1, j1) + (i - i1 - 1) + 1 + (j - j1 - 1));
		}
		DA[s[i - 1]] = i;
		dprint(dd, n, m);
	}
	cost = d(n + 1, m + 1);
	free(dd);
	return cost;
}

void DetectArtistFromInternet(FolderToProcess* folder) {
	if (folder->artist.isEmpty())
		return;

	list<Artist> candidates = MusicBrainz::searchArtist(folder->artist.name);

}

void DetectAlbumFromInternet(FolderToProcess* folder) {

}

int _tmain(int argc, _TCHAR* argv[]) {
	list<Artist> artists = MusicBrainz::searchArtist("fred");
	return 0;

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
		DetectArtistFromTracks(&folder);
		DetectAlbumFromTracks(&folder);

		if (!folder.artist.name.empty() && folder.album.artist.empty())
			folder.album.artist = folder.artist.name;
	}

	for (auto&& folder : foldersToProcess) {
		cout << "Processing " << folder.folder << " ...\n";
		DetectArtistFromInternet(&folder);
		DetectAlbumFromInternet(&folder);
	}


	return 0;
}
