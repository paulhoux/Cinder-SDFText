/*
Copyright (c) 2016, The Barbarian Group
All rights reserved.

Portions of this code (C) Paul Houx
All rights reserved.

Portions of this code (C) Christopher Stones <chris.stones@zoho.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that
the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and
the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and
the following disclaimer in the documentation and/or other materials provided
with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/Cinder.h"
#include "cinder/Vector.h"

#include <list>
#include <vector>

namespace cinder {

namespace binpack {

class Size {
  public:
	Size( uint16_t w, uint16_t h )
	    : width( w )
	    , height( h )
	{
	}
	Size( const ci::ivec2 &sz )
	    : width( sz.x )
	    , height( sz.y )
	{
	}

	bool operator<( const Size &other ) const
	{
		if( width != other.width )
			return width < other.width;
		else
			return height < other.height;
	}

	operator ci::ivec2() const { return ci::ivec2( width, height ); }

  public:
	uint16_t width;
	uint16_t height;
};

class Coord {
  public:
	using Vector = std::vector<Coord>;
	using List = std::list<Coord>;

	Coord()
	    : x( 0 )
	    , y( 0 )
	    , z( 0 )
	{
	}
	Coord( uint16_t x, uint16_t y )
	    : x( x )
	    , y( y )
	    , z( 0 )
	{
	}
	Coord( uint16_t x, uint16_t y, uint16_t z )
	    : x( x )
	    , y( y )
	    , z( z )
	{
	}
	Coord( const ci::ivec2 &coord )
	    : x( coord.x )
	    , y( coord.y )
	    , z( 0 )
	{
	}
	Coord( const ci::ivec3 &coord )
	    : x( coord.x )
	    , y( coord.y )
	    , z( coord.z )
	{
	}
	Coord( const Coord &other )
	    : x( other.x )
	    , y( other.y )
	    , z( other.z )
	{
	}

	bool operator<( const Coord &other ) const
	{
		if( x != other.x )
			return x < other.x;
		else if( y != other.y )
			return y < other.y;
		else
			return z < other.z;
	}

	operator ci::ivec2() const { return ci::ivec2( x, y ); }
	operator ci::ivec3() const { return ci::ivec3( x, y, z ); }

  public:
	uint16_t x, y, z;
};

template <typename T>
class Content {
  public:
	using Vector = std::vector<class Content<T>>;

	Content( const Content<T> &other )
	    : coord( other.coord )
	    , size( other.size )
	    , content( other.content )
	    , rotated( other.rotated )
	{
	}

	Content( const Content<T> &&other )
	    : coord( std::move( other.coord ) )
	    , size( std::move( other.size ) )
	    , content( std::move( other.content ) )
	    , rotated( std::move( other.rotated ) )
	{
	}

	Content( const T &content, const Size &size, const Coord &coord = Coord(), bool rotated = false )
	    : coord( coord )
	    , size( size )
	    , content( content )
	    , rotated( rotated )
	{
	}

	void rotate()
	{
		rotated = !rotated;
		size = Size( size.height, size.width );
	}

	bool intersects( const Content<T> &other ) const
	{
		if( coord.x >= other.coord.x + other.size.width )
			return false;
		if( other.coord.x >= coord.x + size.width )
			return false;
		if( coord.y >= other.coord.y + other.size.height )
			return false;
		if( other.coord.y >= coord.y + size.height )
			return false;

		return true;
	}

  public:
	Coord coord;
	Size  size;
	T     content;
	bool  rotated;
};

template <typename T>
class Canvas {
  public:
	using CanvasVector = std::vector<Canvas<T>, std::allocator<Canvas<T>>>;
	using ContentVector = std::vector<Content<T>, std::allocator<Content<T>>>;

	Canvas()
	    : Canvas( 0, 0 )
	{
	}
	Canvas( uint16_t w, uint16_t h )
	    : index( 0 )
	    , width( w )
	    , height( h )
	    , dirty( false )
	{
		coords.resize( 1 );
	}

	static bool place( CanvasVector &canvases, const ContentVector &contents, ContentVector &remainder )
	{
		ContentVector temp;
		temp.reserve( contents.size() );

		for( auto &canvas : canvases ) {
			temp.clear();
			canvas.place( contents, temp );
			remainder = temp;
		}

		return ( remainder.empty() );
	}

	static bool place( CanvasVector &canvases, const ContentVector &contents )
	{
		ContentVector remainder;
		return place( canvases, contents, remainder );
	}

	static bool place( CanvasVector &canvases, const Content<T> &content )
	{
		ContentVector contents( 1, content );
		return place( canvases, contents );
	}

	const ContentVector &getContents() const { return contents; }

	size_t size() const { return contents.size(); }
	bool   empty() const { return contents.empty(); }

	bool operator<( const Canvas &other )
	{
		if( width != other.width )
			return width < other.width;
		else
			return height < other.height;
	}

	bool place( const ContentVector &contents, ContentVector &remainder )
	{
		assert( remainder.empty() );

		for( auto &content : contents ) {
			if( !place( content ) )
				remainder.push_back( content );
		}

		return remainder.empty();
	}

	bool place( const Content<T> &content )
	{
		sort();

		Content<T> item = content;
		for( auto itr = std::begin( coords ); itr != std::end( coords ); ++itr ) {
			item.coord = *itr;
			if( fits( item ) ) {
				use( item );
				coords.erase( itr );
				return true;
			}
		}

		return false;
	}

	void sort()
	{
		if( !dirty )
			return;

		coords.sort( &Canvas::sortCoords );
		dirty = false;
	}

	typename ContentVector::const_iterator begin() const { return contents.front(); }
	typename ContentVector::const_iterator end() const { return contents.back(); }

	uint16_t getIndex() const { return index; }
	uint16_t getWidth() const { return width; }
	uint16_t getHeight() const { return height; }

	void setIndex( uint16_t i ) { index = i; }

  private:
	bool fits( const Content<T> &item )
	{
		if( ( item.coord.x + item.size.width ) > width )
			return false;
		if( ( item.coord.y + item.size.height ) > height )
			return false;
		for( auto &content : contents ) { // TODO: optimize this check! No need for brute forcing it.
			if( item.intersects( content ) )
				return false;
		}
		return true;
	}

	bool use( const Content<T> &item )
	{
		coords.emplace_front( item.coord.x + item.size.width, item.coord.y );
		coords.emplace_back( item.coord.x, item.coord.y + item.size.height );
		contents.emplace_back( item );
		dirty = true;
		return true;
	}

	static bool sortCoords( const Coord &a, const Coord &b ) { return ( a.x * a.x + a.y * a.y ) < ( b.x * b.x + b.y * b.y ); }

  private:
	uint16_t      index;
	uint16_t      width;
	uint16_t      height;
	Coord::List   coords;
	ContentVector contents;
	bool          dirty;
};

template <typename T>
class ContentAccumulator {
  public:
	using ContentVector = std::vector<Content<T>>;

	ContentAccumulator() {}

	const ContentVector &getContents() const { return contents; }
	ContentVector &      getContents() { return contents; }

	ContentAccumulator<T> &operator+=( const Content<T> &other )
	{
		contents.push_back( other );
		return *this;
	}

	ContentAccumulator<T> &operator+=( const ContentVector &other )
	{
		contents.insert( contents.end(), other.begin(), other.end() );
		return *this;
	}

	ContentAccumulator<T> operator+( const Content<T> &other )
	{
		ContentAccumulator<T> temp = *this;
		temp += other;
		return temp;
	}

	ContentAccumulator<T> operator+( const ContentVector &other )
	{
		ContentAccumulator<T> temp = *this;
		temp += other;
		return temp;
	}

	void sort() { std::sort( contents.begin(), contents.end(), &ContentAccumulator<T>::sortByWidthThenHeight ); }

	bool   empty() const { return contents.empty(); }
	size_t size() const { return contents.size(); }

  private:
	static bool sortByWidthThenHeight( const Content<T> &a, const Content<T> &b )
	{
		if( a.size.width != b.size.width )
			return a.size.width > b.size.width;
		else
			return a.size.height > b.size.height;
	}

  private:
	ContentVector contents;
};

template <typename T>
class CanvasArray {
  public:
	using CanvasVector = std::vector<Canvas<T>>;
	using CanvasConstVector = std::vector<const Canvas<T>>;
	using ContentVector = std::vector<Content<T>>;
	using ContentConstVector = std::vector<const Content<T>>;

	CanvasArray( uint16_t w, uint16_t h )
	    : width( w )
	    , height( h )
	{
		canvases.emplace_back( width, height );
	}
	CanvasArray( const CanvasVector &canvases )
	    : canvases( canvases )
	{
		assert( !canvases.empty() );
		width = canvases[0].width;
		height = canvases[0].height;

		uint16_t index = 0;
		for( auto &canvas : canvases )
			canvas.setIndex( index++ );
	}

	//! Attempts to place contents onto existing canvases. Returns the content that did not fit.
	bool place( const ContentVector &contents, ContentVector &remainder ) { return Canvas<T>::place( canvases, contents, remainder ); }

	//! Places contents onto existing canvases, adding canvases if necessary.
	bool place( const ContentVector &contents )
	{
		ContentVector items = contents;

		ContentVector remainder;
		remainder.reserve( contents.size() );

		// Use existing canvases.
		for( auto &canvas : canvases ) {
			if( canvas.place( items, remainder ) ) {
				items.clear();
				break;
			}

			std::swap( items, remainder );
			remainder.clear();
		}

		// Add new canvases until done.
		uint16_t index = canvases.size();
		while( !items.empty() ) {
			canvases.emplace_back( width, height );
			canvases.back().setIndex( index++ );
			canvases.back().place( items, remainder );

			std::swap( items, remainder );
			remainder.clear();
		}

		return remainder.empty();
	}

	bool place( const ContentAccumulator<T> &content, ContentAccumulator<T> &remainder ) { return place( content.getContents(), remainder.getContents() ); }

	bool place( const ContentAccumulator<T> &content ) { return place( content.getContents() ); }

	bool collect( ContentVector &contents ) const
	{
		uint16_t z = 0;

		for( auto &canvas : canvases ) {
			for( auto &content : canvas.getContents() ) {
				contents.emplace_back( content );
				contents.back().coord.z = z;
			}
			z++;
		}
		return true;
	}

	bool collect( ContentAccumulator<T> &content ) const { return collect( content.getContents() ); }

	bool   empty() const { return canvases.empty(); }
	size_t size() const { return canvases.size(); }

	typename CanvasVector::const_iterator begin() const { return canvases.begin(); }
	typename CanvasVector::const_iterator end() const { return canvases.end(); }

	uint16_t getWidth() const { return width; }
	uint16_t getHeight() const { return height; }

  private:
	uint16_t     width;
	uint16_t     height;
	CanvasVector canvases;
};

} // namespace binpack

} // namespace cinder