// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define NOMINMAX

#include "targetver.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>

#include <stdio.h>
#include <tchar.h>
#include <queue>
#include <list>
#include <set>
#include <map>
#include <string>
#include <iostream>

#include <taglib.h>
#include <fileref.h>
#include <tag.h>
#include <toolkit/tpropertymap.h>

namespace bfs = boost::filesystem;
namespace ba = boost::algorithm;
namespace bl = boost::locale;

using namespace std;

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::XML;
