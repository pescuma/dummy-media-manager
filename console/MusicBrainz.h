#pragma once

#include <list>
#include "Artist.h"
#include "Album.h"

namespace MusicBrainz
{
	std::list<Artist> searchArtist(const std::string& name);
	std::list<Album> searchAlbum(const Artist& artist, const std::string& name);
}
