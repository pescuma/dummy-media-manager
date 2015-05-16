#pragma once

#include "stdafx.h"


class Album
{
public:
	string artist;
	string name;

	Album();
	Album(const string& artist, const string& name);

	bool isEmpty();
};
