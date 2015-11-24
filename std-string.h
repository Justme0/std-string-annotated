#ifndef __STRING__
#define __STRING__

#include <cstddef>
//#include <std/straits.h>
#include <string>	// for simplicity, use std::char_traits instead of string_char_traits

// NOTE : This does NOT conform to the draft standard and is likely to change
//#include <alloc.h>
#include <memory>	// for simplicity, use std::allocator instead of alloc

#include <iterator>

#ifdef __STL_USE_EXCEPTIONS

extern void __out_of_range (const char *);
extern void __length_error (const char *);

#define OUTOFRANGE(cond) \
do { if (cond) __out_of_range (#cond); } while (0)
#define LENGTHERROR(cond) \
do { if (cond) __length_error (#cond); } while (0)

#else

#include <cassert>
#define OUTOFRANGE(cond) assert (!(cond))
#define LENGTHERROR(cond) assert (!(cond))

#endif

//template <class charT, class traits = string_char_traits<charT>,
//	  class Allocator = alloc >
template <class charT, class traits = std::char_traits<charT>, class Allocator = std::allocator<charT>>
class basic_string
{
private:
	struct Rep {
		// static Rep basic_string::nilRep = { 0, 0, 1, false };
		// nilRep.len = 0;
		// nilRep.res = 0;
		// nilRep.ref = 1;
		// nilRep.selfish = false;

		size_t len, res, ref;
		bool selfish;

		charT* data () { return reinterpret_cast<charT *>(this + 1); }
		charT& operator[] (size_t s) { return data () [s]; }
		charT* grab () {
			if (selfish) {
				return clone ();
			}
			++ref;
			return data ();
		}
		void release () { if (--ref == 0) delete this; }

		inline static void * operator new (size_t, size_t);
		inline static void operator delete (void *);
		inline static Rep* create (size_t);
		charT* clone ();

		inline void copy (size_t, const charT *, size_t);
		inline void move (size_t, const charT *, size_t);
		inline void set  (size_t, const charT,   size_t);

		inline static bool excess_slop (size_t, size_t);
		inline static size_t frob_size (size_t);

	private:
		Rep &operator= (const Rep &);
	};

public:
	// types:
	typedef	   traits		traits_type;
	typedef typename traits::char_type	value_type;
	typedef	   Allocator		allocator_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef charT& reference;
	typedef const charT& const_reference;
	typedef charT* pointer;
	typedef const charT* const_pointer;
	typedef pointer iterator;
	typedef const_pointer const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	static const size_type npos = static_cast<size_type>(-1);

private:
	Rep *rep () const { return reinterpret_cast<Rep *>(dat) - 1; }
	void repup (Rep *p) { rep ()->release (); dat = p->data (); }

public:
	const charT* data () const
	{ return rep ()->data(); }
	size_type length () const
	{ return rep ()->len; }
	size_type size () const
	{ return rep ()->len; }
	size_type capacity () const
	{ return rep ()->res; }
	size_type max_size () const
	{ return (npos - 1)/sizeof (charT); }		// XXX
	bool empty () const
	{ return size () == 0; }

	// _lib.string.cons_ construct/copy/destroy:
	basic_string& operator= (const basic_string& str)
	{
		if (&str != this) { rep ()->release (); dat = str.rep ()->grab (); }
		return *this;
	}

	explicit basic_string () : dat (nilRep.grab ()) {
	}

	basic_string (const basic_string& str) : dat (str.rep ()->grab ()) {
	}

	basic_string (const basic_string& str, size_type pos, size_type n = npos) : dat (nilRep.grab ()) {
		assign (str, pos, n);
	}

	basic_string (const charT* s, size_type n) : dat (nilRep.grab ()) {
		assign (s, n);
	}

	basic_string (const charT* s) : dat (nilRep.grab ()) {
		assign (s);
	}

	basic_string (size_type n, charT c) : dat (nilRep.grab ()) {
		assign (n, c);
	}

#ifdef __STL_MEMBER_TEMPLATES
	template<class InputIterator>
	basic_string(InputIterator begin, InputIterator end)
#else
	basic_string(const_iterator begin, const_iterator end)
#endif
	: dat (nilRep.grab ()) {
		assign (begin, end);
	}

	~basic_string () {
		rep ()->release ();
	}

	void swap (basic_string &s) {
		charT *d = dat; dat = s.dat; s.dat = d;
	}

	basic_string& append (const basic_string& str, size_type pos = 0,
			size_type n = npos) {
		return replace (length (), 0, str, pos, n);
	}

	basic_string& append (const charT* s, size_type n) {
		return replace (length (), 0, s, n);
	}

	basic_string& append (const charT* s) {
		return append (s, traits::length (s));
	}
	basic_string& append (size_type n, charT c) {
		return replace (length (), 0, n, c);
	}

#ifdef __STL_MEMBER_TEMPLATES
	template<class InputIterator>
	basic_string& append(InputIterator first, InputIterator last)
#else
	basic_string& append(const_iterator first, const_iterator last)
#endif
	{
		return replace (iend (), iend (), first, last);
	}

	basic_string& assign (const basic_string& str, size_type pos = 0,
			size_type n = npos) {
		return replace (0, npos, str, pos, n);
	}

	basic_string& assign (const charT* s, size_type n) {
		return replace (0, npos, s, n);
	}

	basic_string& assign (const charT* s) {
		return assign (s, traits::length (s));
	}

	basic_string& assign (size_type n, charT c) {
		return replace (0, npos, n, c);
	}

#ifdef __STL_MEMBER_TEMPLATES
	template<class InputIterator>
	basic_string& assign(InputIterator first, InputIterator last)
#else
	basic_string& assign(const_iterator first, const_iterator last)
#endif
	{
		return replace (ibegin (), iend (), first, last);
	}

	basic_string& operator= (const charT* s) {
		return assign (s);
	}

	basic_string& operator= (charT c) {
		return assign (1, c);
	}

	basic_string& operator+= (const basic_string& rhs)
	{ return append (rhs); }
	basic_string& operator+= (const charT* s)
	{ return append (s); }
	basic_string& operator+= (charT c)
	{ return append (1, c); }

	basic_string& insert (size_type pos1, const basic_string& str,
			size_type pos2 = 0, size_type n = npos)
	{ return replace (pos1, 0, str, pos2, n); }
	basic_string& insert (size_type pos, const charT* s, size_type n)
	{ return replace (pos, 0, s, n); }
	basic_string& insert (size_type pos, const charT* s)
	{ return insert (pos, s, traits::length (s)); }
	basic_string& insert (size_type pos, size_type n, charT c)
	{ return replace (pos, 0, n, c); }
	iterator insert(iterator p, charT c)
	{ size_type __o = p - ibegin ();
		insert (p - ibegin (), 1, c); selfish ();
		return ibegin () + __o; }
	iterator insert(iterator p, size_type n, charT c)
	{ size_type __o = p - ibegin ();
		insert (p - ibegin (), n, c); selfish ();
		return ibegin () + __o; }
#ifdef __STL_MEMBER_TEMPLATES
	template<class InputIterator>
	void insert(iterator p, InputIterator first, InputIterator last)
#else
	void insert(iterator p, const_iterator first, const_iterator last)
#endif
	{ replace (p, p, first, last); }

	basic_string& erase (size_type pos = 0, size_type n = npos)
	{ return replace (pos, n, (size_type)0, (charT)0); }
	iterator erase(iterator p)
	{ size_type __o = p - begin();
		replace (__o, 1, (size_type)0, (charT)0); selfish ();
		return ibegin() + __o; }
	iterator erase(iterator f, iterator l)
	{ size_type __o = f - ibegin();
		replace (__o, l-f, (size_type)0, (charT)0);selfish ();
		return ibegin() + __o; }

	basic_string& replace (size_type pos1, size_type n1, const basic_string& str,
			size_type pos2 = 0, size_type n2 = npos);
	basic_string& replace (size_type pos, size_type n1, const charT* s,
			size_type n2);
	basic_string& replace (size_type pos, size_type n1, const charT* s)
	{ return replace (pos, n1, s, traits::length (s)); }
	basic_string& replace (size_type pos, size_type n1, size_type n2, charT c);
	basic_string& replace (size_type pos, size_type n, charT c)
	{ return replace (pos, n, 1, c); }
	basic_string& replace (iterator i1, iterator i2, const basic_string& str)
	{ return replace (i1 - ibegin (), i2 - i1, str); }
	basic_string& replace (iterator i1, iterator i2, const charT* s, size_type n)
	{ return replace (i1 - ibegin (), i2 - i1, s, n); }
	basic_string& replace (iterator i1, iterator i2, const charT* s)
	{ return replace (i1 - ibegin (), i2 - i1, s); }
	basic_string& replace (iterator i1, iterator i2, size_type n, charT c)
	{ return replace (i1 - ibegin (), i2 - i1, n, c); }
#ifdef __STL_MEMBER_TEMPLATES
	template<class InputIterator>
	basic_string& replace(iterator i1, iterator i2,
			InputIterator j1, InputIterator j2);
#else
	basic_string& replace(iterator i1, iterator i2,
			const_iterator j1, const_iterator j2);
#endif

private:
	static charT eos () { return traits::eos (); }
	void unique () { if (rep ()->ref > 1) alloc (length (), true); }
	void selfish () { unique (); rep ()->selfish = true; }

public:
	charT operator[] (size_type pos) const
	{
		if (pos == length ())
			return eos ();
		return data ()[pos];
	}

	reference operator[] (size_type pos)
	{ selfish (); return (*rep ())[pos]; }

	reference at (size_type pos)
	{
		OUTOFRANGE (pos >= length ());
		return (*this)[pos];
	}
	const_reference at (size_type pos) const
	{
		OUTOFRANGE (pos >= length ());
		return data ()[pos];
	}

private:
	void terminate () const
	{ traits::assign ((*rep ())[length ()], eos ()); }

public:
	const charT* c_str () const
	{ if (length () == 0) return ""; terminate (); return data (); }
	void resize (size_type n, charT c);
	void resize (size_type n)
	{ resize (n, eos ()); }
	void reserve (size_type) { }

	size_type copy (charT* s, size_type n, size_type pos = 0) const;

	size_type find (const basic_string& str, size_type pos = 0) const
	{ return find (str.data(), pos, str.length()); }
	size_type find (const charT* s, size_type pos, size_type n) const;
	size_type find (const charT* s, size_type pos = 0) const
	{ return find (s, pos, traits::length (s)); }
	size_type find (charT c, size_type pos = 0) const;

	size_type rfind (const basic_string& str, size_type pos = npos) const
	{ return rfind (str.data(), pos, str.length()); }
	size_type rfind (const charT* s, size_type pos, size_type n) const;
	size_type rfind (const charT* s, size_type pos = npos) const
	{ return rfind (s, pos, traits::length (s)); }
	size_type rfind (charT c, size_type pos = npos) const;

	size_type find_first_of (const basic_string& str, size_type pos = 0) const
	{ return find_first_of (str.data(), pos, str.length()); }
	size_type find_first_of (const charT* s, size_type pos, size_type n) const;
	size_type find_first_of (const charT* s, size_type pos = 0) const
	{ return find_first_of (s, pos, traits::length (s)); }
	size_type find_first_of (charT c, size_type pos = 0) const
	{ return find (c, pos); }

	size_type find_last_of (const basic_string& str, size_type pos = npos) const
	{ return find_last_of (str.data(), pos, str.length()); }
	size_type find_last_of (const charT* s, size_type pos, size_type n) const;
	size_type find_last_of (const charT* s, size_type pos = npos) const
	{ return find_last_of (s, pos, traits::length (s)); }
	size_type find_last_of (charT c, size_type pos = npos) const
	{ return rfind (c, pos); }

	size_type find_first_not_of (const basic_string& str, size_type pos = 0) const
	{ return find_first_not_of (str.data(), pos, str.length()); }
	size_type find_first_not_of (const charT* s, size_type pos, size_type n) const;
	size_type find_first_not_of (const charT* s, size_type pos = 0) const
	{ return find_first_not_of (s, pos, traits::length (s)); }
	size_type find_first_not_of (charT c, size_type pos = 0) const;

	size_type find_last_not_of (const basic_string& str, size_type pos = npos) const
	{ return find_last_not_of (str.data(), pos, str.length()); }
	size_type find_last_not_of (const charT* s, size_type pos, size_type n) const;
	size_type find_last_not_of (const charT* s, size_type pos = npos) const
	{ return find_last_not_of (s, pos, traits::length (s)); }
	size_type find_last_not_of (charT c, size_type pos = npos) const;

	basic_string substr (size_type pos = 0, size_type n = npos) const
	{ return basic_string (*this, pos, n); }

	int compare (const basic_string& str, size_type pos = 0, size_type n = npos) const;
	// There is no 'strncmp' equivalent for charT pointers.
	int compare (const charT* s, size_type pos, size_type n) const;
	int compare (const charT* s, size_type pos = 0) const
	{ return compare (s, pos, traits::length (s)); }

	iterator begin () { selfish (); return &(*this)[0]; }
	iterator end () { selfish (); return &(*this)[length ()]; }

private:
	iterator ibegin () const { return &(*rep ())[0]; }
	iterator iend () const { return &(*rep ())[length ()]; }

public:
	const_iterator begin () const { return ibegin (); }
	const_iterator end () const { return iend (); }

	reverse_iterator       rbegin() { return reverse_iterator (end ()); }
	const_reverse_iterator rbegin() const
	{ return const_reverse_iterator (end ()); }
	reverse_iterator       rend() { return reverse_iterator (begin ()); }
	const_reverse_iterator rend() const
	{ return const_reverse_iterator (begin ()); }

private:
	void alloc (size_type size, bool save);
	static size_type _find (const charT* ptr, charT c, size_type xpos, size_type len);
	inline bool check_realloc (size_type s) const;

	static Rep nilRep;
	charT *dat;
};

#ifdef __STL_MEMBER_TEMPLATES
template <class charT, class traits, class Allocator> template <class InputIterator>
basic_string <charT, traits, Allocator>& basic_string <charT, traits, Allocator>::
replace (iterator i1, iterator i2, InputIterator j1, InputIterator j2)
#else
	template <class charT, class traits, class Allocator>
	basic_string <charT, traits, Allocator>& basic_string <charT, traits, Allocator>::
replace (iterator i1, iterator i2, const_iterator j1, const_iterator j2)
#endif
{
	const size_type len = length ();
	size_type pos = i1 - ibegin ();
	size_type n1 = i2 - i1;
	size_type n2 = j2 - j1;

	OUTOFRANGE (pos > len);
	if (n1 > len - pos)
		n1 = len - pos;
	LENGTHERROR (len - n1 > max_size () - n2);
	size_t newlen = len - n1 + n2;

	if (check_realloc (newlen))
	{
		Rep *p = Rep::create (newlen);
		p->copy (0, data (), pos);
		p->copy (pos + n2, data () + pos + n1, len - (pos + n1));
		for (; j1 != j2; ++j1, ++pos)
			traits::assign ((*p)[pos], *j1);
		repup (p);
	}
	else
	{
		rep ()->move (pos + n2, data () + pos + n1, len - (pos + n1));
		for (; j1 != j2; ++j1, ++pos)
			traits::assign ((*rep ())[pos], *j1);
	}
	rep ()->len = newlen;

	return *this;
}

template <class charT, class traits, class Allocator>
	inline basic_string <charT, traits, Allocator>
operator+ (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	basic_string <charT, traits, Allocator> str (lhs);
	str.append (rhs);
	return str;
}

template <class charT, class traits, class Allocator>
	inline basic_string <charT, traits, Allocator>
operator+ (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	basic_string <charT, traits, Allocator> str (lhs);
	str.append (rhs);
	return str;
}

template <class charT, class traits, class Allocator>
	inline basic_string <charT, traits, Allocator>
operator+ (charT lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	basic_string <charT, traits, Allocator> str (1, lhs);
	str.append (rhs);
	return str;
}

template <class charT, class traits, class Allocator>
	inline basic_string <charT, traits, Allocator>
operator+ (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	basic_string <charT, traits, Allocator> str (lhs);
	str.append (rhs);
	return str;
}

template <class charT, class traits, class Allocator>
	inline basic_string <charT, traits, Allocator>
operator+ (const basic_string <charT, traits, Allocator>& lhs, charT rhs)
{
	basic_string <charT, traits, Allocator> str (lhs);
	str.append (1, rhs);
	return str;
}

template <class charT, class traits, class Allocator>
	inline bool
operator== (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) == 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator== (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) == 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator== (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) == 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator!= (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) != 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator!= (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) != 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator< (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) < 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator< (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) > 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator< (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) < 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator> (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) < 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator> (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) > 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator<= (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) >= 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator<= (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) <= 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator>= (const charT* lhs, const basic_string <charT, traits, Allocator>& rhs)
{
	return (rhs.compare (lhs) <= 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator>= (const basic_string <charT, traits, Allocator>& lhs, const charT* rhs)
{
	return (lhs.compare (rhs) >= 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator!= (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) != 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator> (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) > 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator<= (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) <= 0);
}

template <class charT, class traits, class Allocator>
	inline bool
operator>= (const basic_string <charT, traits, Allocator>& lhs,
		const basic_string <charT, traits, Allocator>& rhs)
{
	return (lhs.compare (rhs) >= 0);
}

class istream; class ostream;
template <class charT, class traits, class Allocator> istream&
operator>> (istream&, basic_string <charT, traits, Allocator>&);
template <class charT, class traits, class Allocator> ostream&
operator<< (ostream&, const basic_string <charT, traits, Allocator>&);
template <class charT, class traits, class Allocator> istream&
getline (istream&, basic_string <charT, traits, Allocator>&, charT delim = '\n');

// the following is bastring.cc

template <class charT, class traits, class Allocator>
inline void * basic_string <charT, traits, Allocator>::Rep::
operator new (size_t s, size_t extra)
{
	// 2015.11.24
	return Allocator().allocate(s + extra * sizeof (charT));
}

template <class charT, class traits, class Allocator>
inline void basic_string <charT, traits, Allocator>::Rep::
operator delete (void * ptr)
{
	Allocator().deallocate((char *)ptr, sizeof(Rep) +
			reinterpret_cast<Rep *>(ptr)->res *
			sizeof (charT)); 
}

template <class charT, class traits, class Allocator>
inline size_t basic_string <charT, traits, Allocator>::Rep::
frob_size (size_t s)
{
	size_t i = 16;
	while (i < s) i *= 2;
	return i;
}

template <class charT, class traits, class Allocator>
inline typename basic_string <charT, traits, Allocator>::Rep *
basic_string <charT, traits, Allocator>::Rep::
create (size_t extra)
{
	extra = frob_size (extra + 1);
	Rep *p = new (extra) Rep;
	p->res = extra;
	p->ref = 1;
	p->selfish = false;
	return p;
}

template <class charT, class traits, class Allocator>
charT * basic_string <charT, traits, Allocator>::Rep::
clone ()
{
	Rep *p = Rep::create (len);
	p->copy (0, data (), len);
	p->len = len;
	return p->data ();
}

template <class charT, class traits, class Allocator>
inline bool basic_string <charT, traits, Allocator>::Rep::
excess_slop (size_t s, size_t r)
{
	return 2 * (s <= 16 ? 16 : s) < r;
}

template <class charT, class traits, class Allocator>
inline bool basic_string <charT, traits, Allocator>::
check_realloc (basic_string::size_type s) const
{
	s += sizeof (charT);
	rep ()->selfish = false;
	return (rep ()->ref > 1
			|| s > capacity ()
			|| Rep::excess_slop (s, capacity ()));
}

template <class charT, class traits, class Allocator>
void basic_string <charT, traits, Allocator>::
alloc (basic_string::size_type size, bool save)
{
	if (! check_realloc (size))
		return;

	Rep *p = Rep::create (size);

	if (save)
	{
		p->copy (0, data (), length ());
		p->len = length ();
	}
	else
		p->len = 0;

	repup (p);
}

template <class charT, class traits, class Allocator>
basic_string <charT, traits, Allocator>&
basic_string <charT, traits, Allocator>::
replace (size_type pos1, size_type n1,
		const basic_string& str, size_type pos2, size_type n2)
{
	const size_t len2 = str.length ();

	if (pos1 == 0 && n1 >= length () && pos2 == 0 && n2 >= len2)
		return operator= (str);

	OUTOFRANGE (pos2 > len2);

	if (n2 > len2 - pos2)
		n2 = len2 - pos2;

	return replace (pos1, n1, str.data () + pos2, n2);
}

template <class charT, class traits, class Allocator>
inline void basic_string <charT, traits, Allocator>::Rep::
copy (size_t pos, const charT *s, size_t n)
{
	if (n)
		traits::copy (data () + pos, s, n);
}

template <class charT, class traits, class Allocator>
inline void basic_string <charT, traits, Allocator>::Rep::
move (size_t pos, const charT *s, size_t n)
{
	if (n)
		traits::move (data () + pos, s, n);
}

template <class charT, class traits, class Allocator>
basic_string <charT, traits, Allocator>&
basic_string <charT, traits, Allocator>::
replace (size_type pos, size_type n1, const charT* s, size_type n2)
{
	const size_type len = length ();
	OUTOFRANGE (pos > len);
	if (n1 > len - pos)
		n1 = len - pos;
	LENGTHERROR (len - n1 > max_size () - n2);
	size_t newlen = len - n1 + n2;

	if (check_realloc (newlen))
	{
		Rep *p = Rep::create (newlen);
		p->copy (0, data (), pos);
		p->copy (pos + n2, data () + pos + n1, len - (pos + n1));
		p->copy (pos, s, n2);
		repup (p);
	}
	else
	{
		rep ()->move (pos + n2, data () + pos + n1, len - (pos + n1));
		rep ()->copy (pos, s, n2);
	}
	rep ()->len = newlen;

	return *this;
}

template <class charT, class traits, class Allocator>
inline void basic_string <charT, traits, Allocator>::Rep::
set (size_t pos, const charT c, size_t n)
{
	traits::set  (data () + pos, c, n);
}

template <class charT, class traits, class Allocator>
basic_string <charT, traits, Allocator>& basic_string <charT, traits, Allocator>::
replace (size_type pos, size_type n1, size_type n2, charT c)
{
	const size_t len = length ();
	OUTOFRANGE (pos > len);
	if (n1 > len - pos)
		n1 = len - pos;
	LENGTHERROR (len - n1 > max_size () - n2);
	size_t newlen = len - n1 + n2;

	if (check_realloc (newlen))
	{
		Rep *p = Rep::create (newlen);
		p->copy (0, data (), pos);
		p->copy (pos + n2, data () + pos + n1, len - (pos + n1));
		p->set  (pos, c, n2);
		repup (p);
	}
	else
	{
		rep ()->move (pos + n2, data () + pos + n1, len - (pos + n1));
		rep ()->set  (pos, c, n2);
	}
	rep ()->len = newlen;

	return *this;
}

template <class charT, class traits, class Allocator>
void basic_string <charT, traits, Allocator>::
resize (size_type n, charT c)
{
	LENGTHERROR (n > max_size ());

	if (n > length ())
		append (n - length (), c);
	else
		erase (n);
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
copy (charT* s, size_type n, size_type pos) const
{
	OUTOFRANGE (pos > length ());

	if (n > length () - pos)
		n = length () - pos;

	traits::copy (s, data () + pos, n);
	return n;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find (const charT* s, size_type pos, size_type n) const
{
	size_t xpos = pos;
	for (; xpos + n <= length (); ++xpos)
		if (traits::eq (data () [xpos], *s)
				&& traits::compare (data () + xpos, s, n) == 0)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
inline typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
_find (const charT* ptr, charT c, size_type xpos, size_type len)
{
	for (; xpos < len; ++xpos)
		if (traits::eq (ptr [xpos], c))
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find (charT c, size_type pos) const
{
	return _find (data (), c, pos, length ());
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
rfind (const charT* s, size_type pos, size_type n) const
{
	if (n > length ())
		return npos;

	size_t xpos = length () - n;
	if (xpos > pos)
		xpos = pos;

	for (++xpos; xpos-- > 0; )
		if (traits::eq (data () [xpos], *s)
				&& traits::compare (data () + xpos, s, n) == 0)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
rfind (charT c, size_type pos) const
{
	if (1 > length ())
		return npos;

	size_t xpos = length () - 1;
	if (xpos > pos)
		xpos = pos;

	for (++xpos; xpos-- > 0; )
		if (traits::eq (data () [xpos], c))
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_first_of (const charT* s, size_type pos, size_type n) const
{
	size_t xpos = pos;
	for (; xpos < length (); ++xpos)
		if (_find (s, data () [xpos], 0, n) != npos)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_last_of (const charT* s, size_type pos, size_type n) const
{
	if (length() == 0)
		return npos;
	size_t xpos = length () - 1;
	if (xpos > pos)
		xpos = pos;
	for (++xpos; xpos-- > 0;)
		if (_find (s, data () [xpos], 0, n) != npos)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_first_not_of (const charT* s, size_type pos, size_type n) const
{
	size_t xpos = pos;
	for (; xpos < length (); ++xpos)
		if (_find (s, data () [xpos], 0, n) == npos)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_first_not_of (charT c, size_type pos) const
{
	size_t xpos = pos;
	for (; xpos < length (); ++xpos)
		if (traits::ne (data () [xpos], c))
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_last_not_of (const charT* s, size_type pos, size_type n) const
{
	if (length() == 0)
		return npos;
	size_t xpos = length () - 1;
	if (xpos > pos)
		xpos = pos;
	for (++xpos; xpos-- > 0;)
		if (_find (s, data () [xpos], 0, n) == npos)
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::
find_last_not_of (charT c, size_type pos) const
{
	if (length() == 0)
		return npos;
	size_t xpos = length () - 1;
	if (xpos > pos)
		xpos = pos;
	for (++xpos; xpos-- > 0;)
		if (traits::ne (data () [xpos], c))
			return xpos;
	return npos;
}

template <class charT, class traits, class Allocator>
int basic_string <charT, traits, Allocator>::
compare (const basic_string& str, size_type pos, size_type n) const
{
	OUTOFRANGE (pos > length ());

	size_t rlen = length () - pos;
	if (rlen > n)
		rlen = n;
	if (rlen > str.length ())
		rlen = str.length ();
	int r = traits::compare (data () + pos, str.data (), rlen);
	if (r != 0)
		return r;
	if (rlen == n)
		return 0;
	return (length () - pos) - str.length ();
}

template <class charT, class traits, class Allocator>
int basic_string <charT, traits, Allocator>::
compare (const charT* s, size_type pos, size_type n) const
{
	OUTOFRANGE (pos > length ());

	size_t rlen = length () - pos;
	if (rlen > n)
		rlen = n;
	int r = traits::compare (data () + pos, s, rlen);
	if (r != 0)
		return r;
	return (length () - pos) - n;
}

// 2015.11.23
//#include <iostream.h>
#include <iostream>

// TODO: upgrade string input using C++11
// istream >> string and getline(istream, string)

//template <class charT, class traits, class Allocator>
//std::istream &
//operator>> (std::istream &is, basic_string <charT, traits, Allocator> &s)
//{
//  int w = is.width (0);
//  // 2015.11.24
//  // in C++11, maybe could use sentry class instead of ipfx*()
//  if (is.ipfx0 ())
//    {
//      register streambuf *sb = is.rdbuf ();
//      s.resize (0);
//      while (1)
//	{
//	  int ch = sb->sbumpc ();
//	  if (ch == EOF)
//	    {
//	      is.setstate (ios::eofbit);
//	      break;
//	    }
//	  else if (traits::is_del (ch))
//	    {
//	      sb->sungetc ();
//	      break;
//	    }
//	  s += ch;
//	  if (--w == 1)
//	    break;
//	}
//    }
//
//  is.isfx ();
//  if (s.length () == 0)
//    is.setstate (ios::failbit);
//
//  return is;
//}

template <class charT, class traits, class Allocator>
	std::ostream &
operator<< (std::ostream &o, const basic_string <charT, traits, Allocator>& s)
{
	return o.write (s.data (), s.length ());
}

//template <class charT, class traits, class Allocator>
//std::istream&
//getline (std::istream &is, basic_string <charT, traits, Allocator>& s, charT delim)
//{
//  if (is.ipfx1 ())
//    {
//      _IO_size_t count = 0;
//      streambuf *sb = is.rdbuf ();
//      s.resize (0);
//
//      while (1)
//	{
//	  int ch = sb->sbumpc ();
//	  if (ch == EOF)
//	    {
//	      is.setstate (count == 0
//			   ? (ios::failbit|ios::eofbit)
//			   : ios::eofbit);
//	      break;
//	    }
//
//	  ++count;
//
//	  if (ch == delim)
//	    break;
//
//	  s += ch;
//
//	  if (s.length () == s.npos - 1)
//	    {
//	      is.setstate (ios::failbit);
//	      break;
//	    }
//	}
//    }
//
//  // We need to be friends with istream to do this.
//  // is._gcount = count;
//  is.isfx ();
//
//  return is;
//}

// define basic_string static members
template <class charT, class traits, class Allocator>
typename basic_string <charT, traits, Allocator>::Rep
basic_string<charT, traits, Allocator>::nilRep = { 0, 0, 1, false };

template <class charT, class traits, class Allocator>
const typename basic_string <charT, traits, Allocator>::size_type
basic_string <charT, traits, Allocator>::npos;

typedef basic_string <char> string;
// typedef basic_string <wchar_t> wstring;

#endif
