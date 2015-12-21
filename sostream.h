// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#pragma once
#include "ustring.h"
#include "mostream.h"

namespace ustl {

class string;

/// \class ostringstream sostream.h ustl.h
/// \ingroup TextStreams
///
/// \brief This stream writes textual data into a memory block.
///
class ostringstream : public ostream {
public:
				ostringstream (const string& v = "");
				ostringstream (void* p, size_t n) noexcept;
    void			iwrite (uint8_t v);
    void			iwrite (wchar_t v);
    inline void			iwrite (int v)			{ iformat (v); }
    inline void			iwrite (unsigned int v)		{ iformat (v); }
    inline void			iwrite (long int v)		{ iformat (v); }
    inline void			iwrite (unsigned long int v)	{ iformat (v); }
    inline void			iwrite (float v)		{ iformat (v); }
    inline void			iwrite (double v)		{ iformat (v); }
    void			iwrite (bool v);
    inline void			iwrite (const char* s)		{ write (s, strlen(s)); }
    inline void			iwrite (const string& v)	{ write (v.begin(), v.size()); }
    inline void			iwrite (fmtflags f);
#if HAVE_LONG_LONG
    inline void			iwrite (long long v)		{ iformat (v); }
    inline void			iwrite (unsigned long long v)	{ iformat (v); }
#endif
    inline size_type		max_size (void) const		{ return _buffer.max_size(); }
    inline ostringstream&	put (char c)			{ iwrite (uint8_t(c)); return *this; }
    int				vformat (const char* fmt, va_list args);
    int				format (const char* fmt, ...) __attribute__((__format__(__printf__, 2, 3)));
    inline void			set_base (uint16_t b)		{ _base = b; }
    inline void			set_width (uint16_t w)		{ _width = w; }
    inline void			set_decimal_separator (char)	{ }
    inline void			set_thousand_separator (char)	{ }
    inline void			set_precision (uint16_t v)	{ _precision = v; }
    void			link (void* p, size_type n) noexcept;
    inline void			link (memlink& l)		{ link (l.data(), l.writable_size()); }
    inline const string&	str (void)			{ flush(); return _buffer; }
    void			str (const string& s);
    ostringstream&		write (const void* buffer, size_type size);
    inline ostringstream&	write (const cmemlink& buf)	{ return write (buf.begin(), buf.size()); }
    inline ostringstream&	seekp (off_t p, seekdir d =beg)	{ ostream::seekp(p,d); return *this; }
    virtual ostream&		flush (void) override		{ ostream::flush(); _buffer.resize (pos()); return *this; }
    virtual size_type		overflow (size_type n = 1) override;
protected:
    inline void			reserve (size_type n)		{ _buffer.reserve (n, false); }
    inline size_type		capacity (void) const		{ return _buffer.capacity(); }
private:
    inline void			write_strz (const char*)	{ assert (!"Writing nul characters into a text stream is not allowed"); }
    inline char*		encode_dec (char* fmt, uint32_t n) const noexcept;
    void			fmtstring (char* fmt, const char* typestr, bool bInteger) const;
    template <typename T>
    void			iformat (T v);
private:
    string			_buffer;	///< The output buffer.
    uint32_t			_flags;		///< See ios_base::fmtflags.
    uint16_t			_width;		///< Field width.
    uint8_t			_base;		///< Numeric base for writing numbers.
    uint8_t			_precision;	///< Number of digits after the decimal separator.
};

//----------------------------------------------------------------------

template <typename T>
inline const char* printf_typestring (const T&)	{ return ""; }
#define PRINTF_TYPESTRING_SPEC(type,str)	\
template <> inline const char* printf_typestring (const type&)	{ return str; }
PRINTF_TYPESTRING_SPEC (int,		"d")
PRINTF_TYPESTRING_SPEC (unsigned int,	"u")
PRINTF_TYPESTRING_SPEC (long,		"ld")
PRINTF_TYPESTRING_SPEC (unsigned long,	"lu")
PRINTF_TYPESTRING_SPEC (float,		"f")
PRINTF_TYPESTRING_SPEC (double,		"lf")
#if HAVE_LONG_LONG
PRINTF_TYPESTRING_SPEC (long long,	"lld")
PRINTF_TYPESTRING_SPEC (unsigned long long, "llu")
#endif
#undef PRINTF_TYPESTRING_SPEC

template <typename T>
void ostringstream::iformat (T v)
{
    char fmt [16];
    fmtstring (fmt, printf_typestring(v), numeric_limits<T>::is_integer);
    format (fmt, v);
}

/// Sets the flag \p f in the stream.
inline void ostringstream::iwrite (fmtflags f)
{
    switch (f) {
	case oct:	set_base (8);	break;
	case dec:	set_base (10);	break;
	case hex:	set_base (16);	break;
	case left:	_flags |= left; _flags &= ~right; break;
	case right:	_flags |= right; _flags &= ~left; break;
	default:	_flags |= f;	break;
    }
}

//----------------------------------------------------------------------

template <typename T> struct object_text_writer {
    inline void operator()(ostringstream& os, const T& v) const { v.text_write (os); }
};
template <typename T> struct integral_text_object_writer {
    inline void operator()(ostringstream& os, const T& v) const { os.iwrite (v); }
};
template <typename T>
inline ostringstream& operator<< (ostringstream& os, const T& v) {
    typedef typename tm::Select <numeric_limits<T>::is_integral,
	integral_text_object_writer<T>, object_text_writer<T> >::Result object_writer_t;
    object_writer_t()(os, v);
    return os;
}
// Needed because if called with a char[], numeric_limits will not work. Should be removed if I find out how to partial specialize for arrays...
inline ostringstream& operator<< (ostringstream& os, const char* v)
    { os.iwrite (v); return os; }
inline ostringstream& operator<< (ostringstream& os, char* v)
    { os.iwrite (v); return os; }

//----------------------------------------------------------------------
// Object writer operators

template <> struct object_text_writer<string> {
    inline void operator()(ostringstream& os, const string& v) const { os.iwrite (v); }
};
template <typename T> struct integral_text_object_writer<T*> {
    inline void operator() (ostringstream& os, const T* const& v) const
	{ os.iwrite ((uintptr_t)(v)); }
};
#define OSTRSTREAM_CAST_OPERATOR(RealT, CastT)		\
template <> inline ostringstream& operator<< (ostringstream& os, const RealT& v) \
    { os.iwrite ((CastT)(v)); return os; }
OSTRSTREAM_CAST_OPERATOR (uint8_t* const,	const char*)
OSTRSTREAM_CAST_OPERATOR (int8_t,		uint8_t)
OSTRSTREAM_CAST_OPERATOR (short int,		int)
OSTRSTREAM_CAST_OPERATOR (unsigned short,	unsigned int)
#if HAVE_THREE_CHAR_TYPES
OSTRSTREAM_CAST_OPERATOR (char,			uint8_t)
#endif
#undef OSTRSTREAM_CAST_OPERATOR

//----------------------------------------------------------------------
// Manipulators

namespace {
static constexpr const struct Sendl {
    inline void text_write (ostringstream& os) const	{ os << '\n'; os.flush(); }
    inline void write (ostream& os) const		{ os.iwrite ('\n'); }
} endl;
static constexpr const struct Sflush {
    inline void text_write (ostringstream& os) const	{ os.flush(); }
    inline void write (ostringstream& os) const		{ os.flush(); }
    inline void write (ostream&) const			{ }
} flush;
constexpr const char ends = '\0';		///< End of string character.
} // namespace

} // namespace ustl
