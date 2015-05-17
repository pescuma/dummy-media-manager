#include "stdafx.h"
#include "Artist.h"


Artist::Artist() {
}

Artist::Artist(const string& name)
	:name(name) {
}

bool Artist::isEmpty() {
	return name.empty();
}
