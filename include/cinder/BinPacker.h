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
	    : w( w )
	    , h( h )
	{
	}
	Size( const ci::ivec2 &sz )
	    : w( sz.x )
	    , h( sz.y )
	{
	}

	bool operator<( const Size &other ) const
	{
		if( w != other.w )
			return w < other.w;
		else
			return h < other.h;
	}

	operator ci::ivec2() const { return ci::ivec2( w, h ); }

  public:
	uint16_t w;
	uint16_t h;
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
	    : mOrigin( other.mOrigin )
		, mExtent( other.mExtent )
		, mSize( other.mSize )
	    , mContent( other.mContent )
	    , mRotated( other.mRotated )
	{
	}

	Content( const Content<T> &&other )
	    : mOrigin( std::move( other.mOrigin ) )
		, mExtent( std::move( other.mExtent ) )
		, mSize( std::move( other.mSize ) )
	    , mContent( std::move( other.mContent ) )
	    , mRotated( std::move( other.mRotated ) )
	{
	}

	Content( const T &content, const Size &size, const Coord &origin = Coord(), bool rotated = false )
	    : mOrigin( origin )
		, mExtent( origin.x + size.w, origin.y + size.h )
		, mSize( size )
	    , mContent( content )
	    , mRotated( rotated )
	{
	}

	const Coord &origin() const { return mOrigin; }
	
	const Coord &extent() const { return mExtent; }

	const T& content() const { return mContent; }

	const Size &size() const { return mSize; }
	
	void move( const Coord &coord )
	{
		mOrigin = coord;
		mExtent.x = coord.x + mSize.w;
		mExtent.y = coord.y + mSize.h;
	}

	void rotate()
	{
		mRotated = !mRotated;

		std::swap( mSize.w, mSize.h );
		mExtent.x = mOrigin.x + mSize.w;
		mExtent.y = mOrigin.y + mSize.h;
	}

	bool intersects( const Content<T> &other ) const
	{
		if( mOrigin.x >= other.mExtent.x || mOrigin.y >= other.mExtent.y || other.mOrigin.x >= mExtent.x || other.mOrigin.y >= mExtent.y )
			return false;

		return true;
	}

  private:
	Coord mOrigin;
	Coord mExtent;
	Size  mSize;
	T     mContent;
	bool  mRotated;
};

template <typename T>
class Canvas {
  public:
	using CanvasVector = std::vector<Canvas<T>>;
	using ContentVector = std::vector<Content<T>>;

	Canvas()
	    : Canvas( 0, 0 )
	{
	}
	Canvas( uint16_t w, uint16_t h )
	    : mIndex( 0 )
	    , mWidth( w )
	    , mHeight( h )
	    , mIsDirty( false )
	{
		mCoords.resize( 1 );
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

	const ContentVector &getContents() const { return mContents; }

	size_t size() const { return mContents.size(); }
	bool   empty() const { return mContents.empty(); }

	bool operator<( const Canvas &other )
	{
		if( mWidth != other.mWidth )
			return mWidth < other.mWidth;
		else
			return mHeight < other.mHeight;
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
		for( auto itr = std::begin( mCoords ); itr != std::end( mCoords ); ++itr ) {
			item.move( *itr );
			if( fits( item ) ) {
				use( item );
				mCoords.erase( itr );
				return true;
			}
		}

		return false;
	}

	void sort()
	{
		if( !mIsDirty )
			return;

		mCoords.sort( &Canvas::sortCoords );
		mIsDirty = false;
	}

	typename ContentVector::const_iterator begin() const { return mContents.front(); }
	typename ContentVector::const_iterator end() const { return mContents.back(); }

	uint16_t getIndex() const { return mIndex; }
	uint16_t getWidth() const { return mWidth; }
	uint16_t getHeight() const { return mHeight; }

	void setIndex( uint16_t i ) { mIndex = i; }

  private:
	bool fits( const Content<T> &item )
	{
		if( item.extent().x > mWidth || item.extent().y > mHeight )
			return false;
		for( auto &content : mContents ) { // TODO: optimize this check! No need for brute forcing it.
			if( item.intersects( content ) )
				return false;
		}
		return true;
	}

	bool use( const Content<T> &item )
	{
		mCoords.emplace_front( item.extent().x, item.origin().y );
		mCoords.emplace_back( item.origin().x, item.extent().y );
		mContents.emplace_back( item );
		mIsDirty = true;
		return true;
	}

	static bool sortCoords( const Coord &a, const Coord &b ) { return ( a.x * a.x + a.y * a.y ) < ( b.x * b.x + b.y * b.y ); }

  private:
	uint16_t      mIndex;
	uint16_t      mWidth;
	uint16_t      mHeight;
	Coord::List   mCoords;
	ContentVector mContents;
	bool          mIsDirty;
};

template <typename T>
class ContentAccumulator {
  public:
	using ContentVector = std::vector<Content<T>>;

	ContentAccumulator() {}

	const ContentVector &getContents() const { return mContents; }
	ContentVector &      getContents() { return mContents; }

	ContentAccumulator<T> &operator+=( const Content<T> &other )
	{
		mContents.push_back( other );
		return *this;
	}

	ContentAccumulator<T> &operator+=( const ContentVector &other )
	{
		mContents.insert( mContents.end(), other.begin(), other.end() );
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

	void sort() { std::sort( mContents.begin(), mContents.end(), &ContentAccumulator<T>::sortByWidthThenHeight ); }

	bool   empty() const { return mContents.empty(); }
	size_t size() const { return mContents.size(); }

  private:
	static bool sortByWidthThenHeight( const Content<T> &a, const Content<T> &b )
	{
		if( a.size().w != b.size().w )
			return a.size().w > b.size().w;
		else
			return a.size().h > b.size().h;
	}

  private:
	ContentVector mContents;
};

template <typename T>
class CanvasArray {
  public:
	using CanvasVector = std::vector<Canvas<T>>;
	using CanvasConstVector = std::vector<const Canvas<T>>;
	using ContentVector = std::vector<Content<T>>;
	using ContentConstVector = std::vector<const Content<T>>;

	CanvasArray( uint16_t w, uint16_t h )
	    : mWidth( w )
	    , mHeight( h )
	{
		mCanvases.emplace_back( mWidth, mHeight );
	}
	CanvasArray( const CanvasVector &canvases )
	    : mCanvases( canvases )
	{
		assert( !mCanvases.empty() );
		mWidth = mCanvases[0].mWidth;
		mHeight = mCanvases[0].mHeight;

		uint16_t index = 0;
		for( auto &canvas : mCanvases )
			canvas.setIndex( index++ );
	}

	//! Attempts to place contents onto existing canvases. Returns the content that did not fit.
	bool place( const ContentVector &contents, ContentVector &remainder ) { return Canvas<T>::place( mCanvases, contents, remainder ); }

	//! Places contents onto existing canvases, adding canvases if necessary.
	bool place( const ContentVector &contents )
	{
		ContentVector items = contents;

		ContentVector remainder;
		remainder.reserve( contents.size() );

		// Use existing canvases.
		for( auto &canvas : mCanvases ) {
			if( canvas.place( items, remainder ) ) {
				items.clear();
				break;
			}

			std::swap( items, remainder );
			remainder.clear();
		}

		// Add new canvases until done.
		uint16_t index = mCanvases.size();
		while( !items.empty() ) {
			mCanvases.emplace_back( mWidth, mHeight );
			mCanvases.back().setIndex( index++ );
			mCanvases.back().place( items, remainder );

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

		for( auto &canvas : mCanvases ) {
			for( auto &content : canvas.getContents() ) {
				mContents.emplace_back( content );
				mContents.back().origin.z = z;
				mContents.back().extent.z = z;
			}
			z++;
		}
		return true;
	}

	bool collect( ContentAccumulator<T> &content ) const { return collect( content.getContents() ); }

	bool   empty() const { return mCanvases.empty(); }
	size_t size() const { return mCanvases.size(); }

	typename CanvasVector::const_iterator begin() const { return mCanvases.begin(); }
	typename CanvasVector::const_iterator end() const { return mCanvases.end(); }

	uint16_t getWidth() const { return mWidth; }
	uint16_t getHeight() const { return mHeight; }

  private:
	uint16_t     mWidth;
	uint16_t     mHeight;
	CanvasVector mCanvases;
};

} // namespace binpack

} // namespace cinder