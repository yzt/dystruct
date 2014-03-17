#pragma once

#if !defined(__Y__DYSTRUCT_INLINE_H__)
#define      __Y__DYSTRUCT_INLINE_H__
	
//======================================================================

#include <cstdio>

//======================================================================

namespace DyStruct {

//======================================================================

inline Hasher::Hasher ()
{
	reset ();
}

//----------------------------------------------------------------------

inline void Hasher::reset ()
{
	setup (0);	// Less magic!
}

//----------------------------------------------------------------------

inline void Hasher::update (void const * data, unsigned len)
{
	auto bytes = reinterpret_cast<unsigned char const *>(data);
	for (unsigned i = 0; i < len; ++i)
		hashByte (bytes[i]);
}

//----------------------------------------------------------------------

inline void Hasher::updateString (void const * str)
{
	auto bytes = reinterpret_cast<unsigned char const *>(str);
	for (unsigned i = 0; bytes[i]; ++i)
		hashByte (bytes[i]);
}

//----------------------------------------------------------------------

inline void Hasher::updateUnsigned (uint32_t val)
{
	char temp [13];
#if defined(_MSC_VER)
	_snprintf_s (temp, 12, "%u", val);
#else
	snprintf (temp, 12, "%u", val);
#endif
	temp[12] = '\0';	// Just in case

	updateString (temp);
}

//----------------------------------------------------------------------

inline uint32_t Hasher::finalizeAndReset ()
{
	finalMix ();
	auto ret = m_hash;
	reset ();
	
	return ret;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

inline void Hasher::setup (uint32_t init)
{
	m_hash = init;
}

//----------------------------------------------------------------------

inline void Hasher::hashByte (unsigned char byte)
{
	auto hash = m_hash;
	hash += byte;
	hash += hash << 10;
	hash ^= hash >> 6;
	m_hash = hash;
}

//----------------------------------------------------------------------

inline void Hasher::finalMix ()
{
	auto hash = m_hash;
	
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	
	m_hash = hash;
}

//----------------------------------------------------------------------
//======================================================================

inline void BasicType::updateHash (Hasher & hasher) const
{
	hasher.updateString (basicTraits().name);
}

//======================================================================

inline void EnumType::updateHash (Hasher & hasher) const
{
	hasher.updateString ("(");
	bool first = true;
	for (auto const & nv : m_name_values)
	{
		if (!first)
			hasher.updateString (",");
		first = false;
		hasher.updateString (nv.first.c_str());
		hasher.updateString (":");
		hasher.updateUnsigned (nv.second);
	}
	hasher.updateString (")");
}

//======================================================================

inline void ArrayType::updateHash (Hasher & hasher) const
{
	hasher.updateString ("[");
	hasher.updateUnsigned (m_count);
	hasher.updateString (":");
	m_element_type->updateHash (hasher);
	hasher.updateString ("]");
}

//======================================================================

inline void DyStructType::Field::hash (Hasher & hasher) const
{
	type->updateHash (hasher);
}

//----------------------------------------------------------------------

inline void DyStructType::updateHash (Hasher & hasher) const
{
	hasher.updateString ("<");
	bool first = true;
	for (auto const & f : m_fields)
	{
		if (!first)
			hasher.updateString (",");
		else
			first = false;
		f.hash (hasher);
	}
	hasher.updateString (">");
}

//----------------------------------------------------------------------

inline bool DyStructType::construct (void * mem, SizeType sz) const
{
	assert (sz == m_cur_size);
	if (sz != m_cur_size)
		return false;

	for (auto const & f : m_fields)
		f.type->construct (((char *)mem) + f.offset, f.type->getSizeOf());

	return true;
}

//----------------------------------------------------------------------

inline bool DyStructType::destruct (void * mem, SizeType sz) const
{
	assert (sz == m_cur_size);
	if (sz != m_cur_size)
		return false;

	for (auto i = m_fields.rbegin(), e = m_fields.rend(); i != e; ++i)
		i->type->destruct (((char *)mem) + i->offset, i->type->getSizeOf());
	
	return true;
}

//======================================================================

inline void InstancePtr::destroySelf ()
{
	m_ctype->destroyInstance (*this);
}

//======================================================================
//----------------------------------------------------------------------
//======================================================================
//----------------------------------------------------------------------
//======================================================================

}	// namespace DyStruct

//======================================================================

#endif	// __Y__DYSTRUCT_INLINE_H__
