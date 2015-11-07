#include "stdafx.h"

#include "FolderToProcess.h"
#include "MusicBrainz.h"
#include "utf8.h"

#include <clinq.h>
using namespace clinq;

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
	const TagLib::StringList& prop = props[name];

	if (prop.isEmpty())
		return defVal;
	else
		return prop[0].to8Bit(true);
}

static int getInt(const TagLib::PropertyMap& props, const char* name, int defVal = 0) {
	const TagLib::StringList& prop = props[name];

	if (prop.isEmpty())
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
	for (const bfs::path& file : folder->files) {
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

static Album FindAlbumFromTracks(const list<Track>& tracks, bool allowMissing) {
	set<string, CaseAndAccentsInsensitiveCompare> album;

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
	set<string, CaseAndAccentsInsensitiveCompare> artist;

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

string normalize(const string& input) {
	string result = toLower(input);
	replaceInPlace(result, "\xE2\x80\xA6", "...");
	result = removeAccents(result);
	return result;
}

void completeAliases(list<Artist>* artists) {
	for (Artist& artist : *artists) {
		artist.aliases.insert(artist.name);

		for (const string& alias : artist.aliases)
			artist.aliases.insert(normalize(alias));
	}
}

Artist FindBetterCandidate(const string& artist, list<Artist> mbas) {
	struct ArtistAndDistance
	{
		size_t distance;
		Artist* artist;

		ArtistAndDistance(Artist* artist, size_t distance)
			: distance(distance),
			  artist(artist) {
		}
	};

	string normalizedArtist = normalize(artist);

	list<ArtistAndDistance> candidates;
	transform(mbas.begin(), mbas.end(), back_inserter(candidates), [&](Artist& a) {
		          size_t distance = computeEditDistance(normalizedArtist, normalize(a.name));
		          return ArtistAndDistance(&a, distance);
	          });

	candidates.remove_if([](const ArtistAndDistance& a) {
		return a.distance > 4;
	});

	candidates.sort([](const ArtistAndDistance& a, const ArtistAndDistance& b) {
		return a.distance - b.distance;
	});

	size_t numCandidates = candidates.size();
	if (numCandidates < 0)
		return Artist();

	if (numCandidates > 1)
		cout << "More than one";

	return *candidates.front().artist;
}

list<Artist> artists;


Artist DetectArtists(const string& artist) {
	if (artist.empty())
		return Artist();

	string na = normalize(artist);

	list<Artist> mbas = MusicBrainz::searchArtist(artist);

	if (mbas.empty())
		mbas = MusicBrainz::searchArtist(na);

	if (mbas.empty())
		return Artist();

	completeAliases(&mbas);

	return from(mbas)
			.where([&](Artist& c) {
				return c.aliases.find(na) != c.aliases.end();
			})
			.first_or_default(Artist());
}


void DetectAlbumFromInternet(FolderToProcess* folder) {
}

int _tmain(int argc, _TCHAR* argv[]) {
	removeAccents("asd");

	bl::generator gen;
	locale::global(gen(""));

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
		Artist candidate = DetectArtists(folder.artist.name);
		if (!candidate.isEmpty())
			folder.artist = candidate;

		DetectAlbumFromInternet(&folder);
	}


	return 0;
}
