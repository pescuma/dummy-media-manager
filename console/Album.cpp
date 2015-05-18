#include "stdafx.h"
#include "Album.h"

Album::Album()
{
}

Album::Album(const string& artist, const string& name)
	: artist(artist),
	  name(name)
{
}

bool Album::isEmpty()
{
	return artist.empty() || name.empty();
}
