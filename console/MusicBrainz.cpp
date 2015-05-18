#include "stdafx.h"
#include "MusicBrainz.h"

namespace MusicBrainz
{
namespace
{
struct RequestResult
{
	string error;
	string body;
};

RequestResult doRequest(const URI& uri)
{
	string path(uri.getPathAndQuery());
	if (path.empty())
		path = "/";

	HTTPClientSession session(uri.getHost(), uri.getPort());
	HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	session.sendRequest(request);
	istream& rs = session.receiveResponse(response);

	RequestResult result;

	if (response.getStatus() != HTTPResponse::HTTPStatus::HTTP_OK)
	{
		NullOutputStream null;
		StreamCopier::copyStream(rs, null);

		result.error.append("Error executing request to ").append(uri.toString()).append(": ")
		      .append(to_string(response.getStatus())).append(" ").append(response.getReason());
	}
	else
	{
		StreamCopier::copyToString(rs, result.body);
	}

	return result;
}

Node* getChild(Node* node, const string& name)
{
	StringTokenizer path(name, "/");

	for (auto&& part : path)
	{
		Node* it = node->firstChild();
		while (it && it->nodeName() != part)
			it = it->nextSibling();

		if (it == nullptr)
			return nullptr;

		node = it;
	}

	return node;
}

Element* getChildElement(Element* node, const string& name)
{
	StringTokenizer path(name, "/");

	for (auto&& part : path)
	{
		Node* it = node->firstChild();
		while (it && !(it->nodeType() == Node::ELEMENT_NODE && it->nodeName() == part))
			it = it->nextSibling();

		if (it == nullptr)
			return nullptr;

		node = (Element*) it;
	}

	return node;
}

string getChildValue(Node* element, const char* name)
{
	element = getChild(element, name);
	if (element == nullptr)
		return "";

	return element->innerText();
}

class ChildElementWalker
{
	Element* element;
	string name;

public:

	ChildElementWalker(Element* element, const string& name)
		: element(element),
		  name(name)
	{
	}

	class Iterator
	{
		Element* element;

	public:
		Iterator(Element* element)
			: element(element)
		{
		}

		bool operator!=(const Iterator& other) const
		{
			return element != other.element;
		}

		Element* operator*() const
		{
			return element;
		}

		const Iterator& operator++()
		{
			Node* result = element->nextSibling();
			while (result && !(result->nodeType() == Node::ELEMENT_NODE && result->nodeName() == element->nodeName()))
				result = result->nextSibling();
			element = (Element*)result;
			return *this;
		}
	};

	Iterator begin() const
	{
		return Iterator(getChildElement(element, name));
	}

	Iterator end() const
	{
		return Iterator(nullptr);
	}
};
}


list<Artist> MusicBrainz::searchArtist(const string& name)
{
	list<Artist> result;

	URI uri("http://musicbrainz.org/ws/2/artist");
	uri.addQueryParameter("query", "artist:" + name);

	RequestResult response = doRequest(uri);
	if (!response.error.empty())
	{
		cerr << response.error << endl;
		return result;
	}

	DOMParser parser;
	AutoPtr<Document> xml = parser.parseString(response.body);

	Element* xmlArtists = xml->documentElement()->getChildElement("artist-list");
	for (Element* xmlArtist : ChildElementWalker(xmlArtists, "artist"))
	{
		Artist artist;

		artist.musicbrainz_artistId = xmlArtist->getAttribute("id");
		artist.name = getChildValue(xmlArtist, "name");
		artist.type = xmlArtist->getAttribute("type");
		artist.description = getChildValue(xmlArtist, "disambiguation");
		artist.gender = getChildValue(xmlArtist, "gender");
		artist.location = getChildValue(xmlArtist, "area/name");

		for (Element* xmlAlias : ChildElementWalker(xmlArtists, "alias-list/alias"))
			artist.aliases.insert(xmlAlias->innerText());

		for (Element* xmlTag : ChildElementWalker(xmlArtists, "tag-list/tag"))
			artist.tags.push_back(getChildValue(xmlTag, "name"));

		result.push_back(artist);
	}

	return result;
}

list<Album> searchAlbum(const Artist& artist, const string& name)
{
	list<Album> result;

	if (artist.musicbrainz_artistId.empty())
		return result;

	URI uri("http://musicbrainz.org/ws/2/release");
	uri.addQueryParameter("query", "release:\"" + name + "\" AND arid:" + artist.musicbrainz_artistId);


	return result;
}
}
