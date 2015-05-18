#pragma once

#include <functional>
#include <list>
#include <set>


template <typename ITERATOR, typename ITEM>
class FitleredIterator
{
	ITERATOR current;
	ITERATOR end;
	std::function<bool(const ITEM&)> predicate;

public:
	FitleredIterator(const ITERATOR& current, const ITERATOR& end, const std::function<bool(const ITEM&)>& predicate)
		: current(current),
		  end(end),
		  predicate(predicate) {
	}

	bool operator!=(const FitleredIterator<ITERATOR, ITEM>& other) const {
		return current != other.current;
	}

	ITEM operator*() const {
		return *current;
	}

	const FitleredIterator& operator++() {
		if (current == end)
			return *this;

		do {
			++current;
		} while (current != end && !predicate(*current));

		return *this;
	}
};


template <typename ITERATOR, typename ITEM>
class Enumerable
{
protected:
	ITERATOR itBegin;
	ITERATOR itEnd;

public:
	Enumerable(const ITERATOR& begin, const ITERATOR& end)
		: itBegin(begin),
		  itEnd(end) {
	}

	ITERATOR begin() {
		return begin;
	}

	ITERATOR end() {
		return end;
	}

	Enumerable<FitleredIterator<ITERATOR, ITEM>, ITEM> where(const std::function<bool(const ITEM&)>& predicate) {
		return Enumerable<FitleredIterator<ITERATOR, ITEM>, ITEM>(
			FitleredIterator<ITERATOR, ITEM>(itBegin, itEnd, predicate),
			FitleredIterator<ITERATOR, ITEM>(itEnd, itEnd, predicate)
		);
	}

	std::list<ITEM> toList() {
		std::list<ITEM> result;

		for (ITERATOR it = itBegin; it != itEnd; ++it)
			result.push_back(*it);

		return result;
	}
};


template <typename ITEM>
Enumerable<typename std::list<ITEM>::iterator, ITEM> from(std::list<ITEM>& l) {
	return Enumerable<typename std::list<ITEM>::iterator, ITEM>(l.begin(), l.end());
}

template <typename ITEM>
Enumerable<typename std::set<ITEM>::iterator, ITEM> from(std::set<ITEM>& l) {
	return Enumerable<typename std::set<ITEM>::iterator, ITEM>(l.begin(), l.end());
}
