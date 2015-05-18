#pragma once

#include "Artist.h"
#include "Album.h"

namespace MusicBrainz
{
list<Artist> searchArtist(const string& name);
list<Album> searchAlbum(const Artist& artist, const string& name);
}
