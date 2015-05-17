#pragma once

#include <list>
#include "Artist.h"

namespace MusicBrainz
{
	std::list<Artist> searchArtist(const std::string &name);
	
}
