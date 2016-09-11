/*
 Copyright (c) 2015, The Barbarian Group
 All rights reserved.

 Portions of this code (C) Paul Houx
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

#include "cinder/Area.h"

#include <list>
#include <map>
#include <vector>

namespace cinder {

class PackedArea;

class BinPackerBase {
  public:
	BinPackerBase() {}
	virtual ~BinPackerBase() {}

	//! Sets the width and height of the bin.
	virtual void setSize( uint32_t width, uint32_t height ) { setSize( ivec2( width, height ) ); }
	//! Sets the width and height of the bin.
	virtual void setSize( const ivec2 &size ) { mSize = size; }

	//! Takes a list of \a areas, packs them and returns a list of packed areas.
	virtual std::vector<PackedArea> insert( const std::vector<Area> &areas ) = 0;
	//! Packs the \a area and returns it as a PackedArea.
	virtual PackedArea insert( const Area &area );

	//! Returns the size of the bin.
	ivec2 getSize() const { return mSize; }
	//! Returns the width of the bin.
	int32_t getWidth() const { return mSize.x; }
	//! Returns the height of the bin.
	int32_t getHeight() const { return mSize.y; }

	//! Clears the internal data structures.
	virtual void clear();

  protected:
	//! Returns \c true if \a a fits inside \a b.
	bool fits( const PackedArea &a, const PackedArea &b ) const;
	//! Splits \a area into 2 new areas.
	PackedArea split( PackedArea *area, int32_t width, int32_t height ) const;
	//! Packs the area. Returns \c true if successful.
	bool pack( PackedArea *area );

  protected:
	ivec2 mSize;

	//! List of available space to fill.
	std::vector<PackedArea> mAvailable;
};

class BinPacker : public BinPackerBase {
  public:
	BinPacker() { clear(); }
	~BinPacker() {}

	//! Sets the width and height of the bin.
	BinPacker &size( uint32_t width, uint32_t height )
	{
		setSize( width, height );
		return *this;
	}
	//! Sets the width and height of the bin.
	BinPacker &size( const ivec2 &size )
	{
		setSize( size );
		return *this;
	}

	//! Adds \a areas to the already packed areas, packs them (online) and returns
	//! a list of packed areas.
	std::vector<PackedArea> insert( const std::vector<Area> &areas ) override;
};

class MultiBinPacker : public BinPackerBase {
  public:
	MultiBinPacker() { clear(); }
	~MultiBinPacker() {}

	//! Sets the width and height of the bin.
	MultiBinPacker &size( uint32_t width, uint32_t height )
	{
		setSize( width, height );
		return *this;
	}
	//! Sets the width and height of the bin.
	MultiBinPacker &size( const ivec2 &size )
	{
		setSize( size );
		return *this;
	}

	//! Adds \a areas to the already packed areas, packs them (online) and returns
	//! a list of packed areas.
	std::vector<PackedArea> insert( const std::vector<Area> &areas ) override;

	//! Clears the internal data structures.
	void clear() override;

  private:
	uint32_t mBin;
};

class PackedArea : public Area {
  public:
	PackedArea()
	    : mOrder( 0 )
	    , mBin( 0 )
	{
	}
	PackedArea( const ivec2 &UL, const ivec2 &LR, uint32_t order = 0 )
	    : Area( UL, LR )
	    , mOrder( order )
	    , mBin( 0 )
	{
	}
	PackedArea( int32_t aX1, int32_t aY1, int32_t aX2, int32_t aY2, uint32_t order = 0 )
	    : Area( aX1, aY1, aX2, aY2 )
	    , mOrder( order )
	    , mBin( 0 )
	{
	}
	explicit PackedArea( const RectT<float> &r, uint32_t order = 0 )
	    : Area( r )
	    , mOrder( order )
	    , mBin( 0 )
	{
	}
	explicit PackedArea( const Area &area, uint32_t order = 0 )
	    : Area( area )
	    , mOrder( order )
	    , mBin( 0 )
	{
	}

	//! Returns the bin number in which this area is packed.
	uint32_t getBin() const { return mBin; }

	//! Allow sorting by area.
	bool operator<( const PackedArea &rhs ) { return calcArea() < rhs.calcArea(); }

	//! Allow sorting by area.
	static bool sortByArea( const PackedArea &a, const PackedArea &b ) { return a.calcArea() < b.calcArea(); }

	//! Allow sorting by order.
	static bool sortByOrder( const PackedArea &a, const PackedArea &b ) { return a.mOrder < b.mOrder; }

  private:
	friend class MultiBinPacker;

	uint32_t mOrder;
	uint32_t mBin;
};

class BinPackerTooSmallExc : public std::exception {
  public:
	virtual const char *what() const throw() { return "Bin size is too small to fit all areas."; }
};

// -----------------------------------------------------------------------------------------------

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
	using CanvasVector = std::vector<Canvas<T>>;
	using ContentVector = std::vector<Content<T>>;

	// Canvas()
	//    : Canvas( 1024, 1024 )
	//{
	//}
	Canvas( uint16_t w, uint16_t h )
	    : width( w )
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

		// TODO: add support for rotation.

		return false;
	}

	void sort()
	{
		if( !dirty )
			return;

		coords.sort( &Canvas::sortCoords );
		dirty = false;
	}

	uint16_t getWidth() const { return width; }
	uint16_t getHeight() const { return height; }

  private:
	bool fits( const Content<T> &item )
	{
		if( ( item.coord.x + item.size.width ) > width )
			return false;
		if( ( item.coord.y + item.size.height ) > height )
			return false;
		for( auto &content : contents ) {
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
	using ContentVector = std::vector<Content<T>>;

	// CanvasArray()
	//    : CanvasArray( 1024, 1024 )
	//{
	//}
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
	}

	bool place( const ContentVector &contents, ContentVector &remainder ) { return Canvas<T>::place( canvases, contents, remainder ); }

	//! Places contents onto existing canvases, adding canvases if necessary.
	bool place( const ContentVector &contents ) {
		ContentVector items = contents;

		ContentVector remainder;
		remainder.reserve( contents.size() );

		// Use existing canvases.
		for( auto &canvas : canvases ) {
			if( canvas.place( items, remainder ) )
				break;

			std::swap( items, remainder );
			remainder.clear();
		}

		// Add new canvases until done.
		while( !items.empty() ) {
			canvases.emplace_back( width, height );
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

	uint16_t getWidth() const { return width; }
	uint16_t getHeight() const { return height; }

  private:
	uint16_t     width;
	uint16_t     height;
	CanvasVector canvases;
};

} // namespace binpack

} // namespace cinder