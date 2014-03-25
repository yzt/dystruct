#pragma once

#if !defined(__Y__DYSTRUCT_H__)
#define      __Y__DYSTRUCT_H__
	
//======================================================================

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

//======================================================================

namespace DyStruct {

//======================================================================

// The current implementation is based on (actually, it /is/) Bob Jenkins' "one-at-a-time" hash.
class Hasher
{
public:
	inline Hasher ();
	
	inline void reset ();
	inline void update (void const * data, unsigned len);
	inline void updateString (void const * str);
	inline void updateUnsigned (uint32_t val);	// Converts val to string and hashes it
	inline uint32_t finalizeAndReset ();
	inline void hashByte (unsigned char byte);

private:
	inline void setup (uint32_t init);
	inline void finalMix ();
	
private:
	uint32_t m_hash;
};

//======================================================================

typedef uint32_t ID;
typedef unsigned char Byte;

typedef uint32_t SizeType;
typedef uint32_t CountType;
typedef SizeType OffsetType;	// 16 or 32?

typedef std::string Name;

//----------------------------------------------------------------------

enum class Family
{
	Basic = 0,
	Enum,
	Array,
	Struct,
	DyStruct,
	String,
	Vector,
	Set,
	Map,
	
	_count
};
static_assert (int(Family::_count) == 9, "Did you forget something?!");

//----------------------------------------------------------------------

enum class Basic
{
	I8,
	U8,
	I16,
	U16,
	I32,
	U32,
	I64,
	U64,

	F32,
	F64,

	Bool,
	Byte,
	Char,
	WChar,
	
	_count
};
static_assert (int(Basic::_count) == 14, "Did you forget something?!");

//----------------------------------------------------------------------

namespace details {
	struct FamilyTraits
	{
		Family const family;
		char const * const name;
		bool const always_fixed_footprint;	// Always fixed and known size. Note that a "false" here doesn't necessarily mean that it's not fixed size
		bool const multi_element;			// More than one element of a single type, e.g array, vector, etc.
		bool const composite;				// Several elements of different types, e.g. struct
		bool const associative;				// Associative (i.e. key-value) type, e.g. map
		bool const fixed_count;				// Count of elements is fixed, e.g. basic, array

//		FamilyTraits () = delete;
//		FamilyTraits (FamilyTraits const &) = delete;
//		FamilyTraits & operator = (FamilyTraits const &) = delete;
	};

	struct BasicTraits
	{
		Basic const basic_type;
		char const * const name;
		uint32_t const size;
		bool const is_numeric;
		bool const is_integer;
		bool const is_signed;
		bool const is_float;

//		BasicTraits () = delete;
//		BasicTraits (BasicTraits const &) = delete;
//		BasicTraits & operator = (BasicTraits const &) = delete;
	};

	extern const FamilyTraits gc_FamilyTraits [int(Family::_count)];
	extern const BasicTraits gc_BasicTraits [int(Basic::_count)];

	// A typelist that maps DyStruct::Basic enum values to actual types.
	template <Basic B> struct BasicTypeMap {};
	template <> struct BasicTypeMap<Basic::I8>   {typedef int8_t type;};
	template <> struct BasicTypeMap<Basic::U8>   {typedef uint8_t type;};
	template <> struct BasicTypeMap<Basic::I16>  {typedef int16_t type;};
	template <> struct BasicTypeMap<Basic::U16>  {typedef uint16_t type;};
	template <> struct BasicTypeMap<Basic::I32>  {typedef int32_t type;};
	template <> struct BasicTypeMap<Basic::U32>  {typedef uint32_t type;};
	template <> struct BasicTypeMap<Basic::I64>  {typedef int64_t type;};
	template <> struct BasicTypeMap<Basic::U64>  {typedef uint64_t type;};
	template <> struct BasicTypeMap<Basic::F32>  {typedef float type;};
	template <> struct BasicTypeMap<Basic::F64>  {typedef double type;};
	template <> struct BasicTypeMap<Basic::Bool> {typedef bool type;};
	template <> struct BasicTypeMap<Basic::Byte> {typedef uint8_t type;};
	template <> struct BasicTypeMap<Basic::Char> {typedef char type;};
	template <> struct BasicTypeMap<Basic::WChar>{typedef wchar_t type;};
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//======================================================================

class InstancePtr;
class TypeManager;

//----------------------------------------------------------------------

class Type
{
	friend class TypeManager;
	
protected:
	Type (Family family) : m_family {family} {assert (int(m_family) >= 0 && family < Family::_count);}
	virtual ~Type () = default;

	virtual Type * clone () const = 0;

public:
	virtual CountType getElemCount () const = 0;
	virtual SizeType getElemSize () const = 0;
	virtual SizeType getSizeOf () const = 0;
	virtual SizeType getFootprint () const = 0;
	virtual bool isFixedFootprint () const = 0;
	
	virtual void updateHash (Hasher & hasher) const = 0;
	virtual bool construct (void * mem, SizeType sz) const = 0;
	virtual bool destruct (void * mem, SizeType sz) const = 0;
	
	Family getFamily () const {return m_family;}
	char const * getFamilyName () const {return familyTraits().name;}
	
	bool isBasic () const {return m_family == Family::Basic;}
	bool isEnum () const {return m_family == Family::Enum;}
	bool isArray () const {return m_family == Family::Array;}
	bool isStruct () const {return m_family == Family::Struct;}
	bool isDyStruct () const {return m_family == Family::DyStruct;}
	bool isString () const {return m_family == Family::String;}
	bool isVector () const {return m_family == Family::Vector;}
	bool isSet () const {return m_family == Family::Set;}
	bool isMap () const {return m_family == Family::Map;}

	bool isAlwaysFixedFootprint () const {return familyTraits().always_fixed_footprint;}
	bool isMultiElement () const {return familyTraits().multi_element;}
	bool isComposite () const {return familyTraits().composite;}
	bool isAssociative () const {return familyTraits().associative;}
	bool isFixedCount () const {return familyTraits().fixed_count;}
	
protected:
	details::FamilyTraits const & familyTraits () const {return details::gc_FamilyTraits[int(m_family)];}
	
private:
	Family m_family;
};

//======================================================================

class BasicType
	: public Type
{
	friend class TypeManager;

protected:
	BasicType (Basic basic_type)
		: Type {Family::Basic}
		, m_basic_type {basic_type}
	{
		assert (int(m_basic_type) >= 0 && m_basic_type < Basic::_count);
	}
	
	virtual Type * clone () const override {return new BasicType {*this};}
	
public:
	virtual CountType getElemCount () const override {return 1;}
	virtual SizeType getElemSize () const override {return basicTraits().size;}
	virtual SizeType getSizeOf () const override {return 1 * basicTraits().size;}
	virtual SizeType getFootprint () const override {return 1 * basicTraits().size;}
	virtual bool isFixedFootprint () const override {return true;}

	virtual inline void updateHash (Hasher & hasher) const override;
	virtual bool construct (void * /*mem*/, SizeType /*sz*/) const override {return true;}
	virtual bool destruct (void * /*mem*/, SizeType /*sz*/) const override {return true;}

	Basic getBasicType () const {return m_basic_type;}
	char const * getBasicTypeName () const {return basicTraits().name;}

	bool isNumeric () const {return basicTraits().is_numeric;}
	bool isInteger () const {return basicTraits().is_integer;}
	bool isSigned () const {return basicTraits().is_signed;}
	bool isFloat () const {return basicTraits().is_float;}
		
protected:
	details::BasicTraits const & basicTraits () const {return details::gc_BasicTraits[int(m_basic_type)];}
	
private:
	Basic m_basic_type;
};

//======================================================================

class EnumType
	: public Type
{
	friend class TypeManager;

public:
	typedef std::vector<std::pair<std::string, uint32_t>> NameValuePairContainer;
	
protected:
	EnumType (Basic /*basic_type*/)
		: Type {Family::Enum}
		, m_max_value {-1}	//{std::numeric_limits<int64_t>::min()}
		, m_underlying_type {underlyingType ()}
	{}
	
	virtual Type * clone () const override {return new EnumType {*this};}
	
public:
	virtual CountType getElemCount () const override {return 1;}
	virtual SizeType getElemSize () const override {return underlyingTraits().size;}
	virtual SizeType getSizeOf () const override {return 1 * underlyingTraits().size;}
	virtual SizeType getFootprint () const override {return 1 * underlyingTraits().size;}
	virtual bool isFixedFootprint () const override {return true;}

	virtual inline void updateHash (Hasher & hasher) const override;
	virtual bool construct (void * /*mem*/, SizeType /*sz*/) const override {return true;}
	virtual bool destruct (void * /*mem*/, SizeType /*sz*/) const override {return true;}

	bool addEntry (std::string name, uint32_t value);	// Will fail if either already is in the Enum
	bool addEntry (std::string name);					// Auto value, 1 more than previous max
    NameValuePairContainer const & getNameValues () const {return m_name_values;}
    size_t getEntriesCount () const {return m_name_values.size();}
	int64_t getMaxValue () const {return m_max_value;}
	
protected:
	Basic underlyingType () const {return (m_max_value < 256) ? Basic::U8 : ((m_max_value < 65536) ? Basic::U16 : Basic::U32);}
	details::BasicTraits const & underlyingTraits () const {return details::gc_BasicTraits[int(m_underlying_type)];}
	bool hasNameOrValue (std::string const & name, uint32_t value) const;

private:
	NameValuePairContainer m_name_values;
	int64_t m_max_value;
	Basic m_underlying_type;
};

//======================================================================

class ArrayType
	: public Type
{
	friend class TypeManager;
	
protected:
	// Type must already exist in TypeManager
	ArrayType (uint32_t count, Type * element_type)
		: Type {Family::Array}
		, m_count {count}
		, m_element_type {element_type}
	{
		assert (m_element_type);
	}
	
	virtual Type * clone () const override {return new ArrayType {*this};}

public:
	virtual CountType getElemCount () const override {return m_count;}
	virtual SizeType getElemSize () const override {return m_element_type->getElemSize();}
	virtual SizeType getSizeOf () const override {return m_count * m_element_type->getElemSize();}
	virtual SizeType getFootprint () const override {return m_count * m_element_type->getElemSize();}
	virtual bool isFixedFootprint () const override {return m_element_type->isFixedFootprint();}

	virtual inline void updateHash (Hasher & hasher) const override;
	virtual bool construct (void * /*mem*/, SizeType /*sz*/) const override {return true;}
	virtual bool destruct (void * /*mem*/, SizeType /*sz*/) const override {return true;}
	
private:
	uint32_t m_count;
	Type * m_element_type;
};

//======================================================================

class DyStructType
	: public Type
{
	friend class TypeManager;
	
public:
	struct Field
	{
		SizeType offset;
		Type * type;
		std::string name;

		Field (Type * _type, std::string && _name) : type (_type), name (std::move(_name)) {assert (type); assert (!name.empty());}
		Field (Type * _type, std::string const & _name) : type (_type), name (_name) {assert (type); assert (!name.empty());}
		
		inline void hash (Hasher & hasher) const;
	};

	typedef std::vector<Field> FieldContainer;

protected:
	DyStructType () : Type {Family::DyStruct}, m_cur_size {0}, m_fields {} {}
	
	virtual Type * clone () const {return new DyStructType {*this};}

public:
	virtual CountType getElemCount () const override {return 1;}
	virtual SizeType getElemSize () const override {return m_cur_size;}
	virtual SizeType getSizeOf () const override {return 1 * m_cur_size;}
	virtual SizeType getFootprint () const override {return calculateFootprint();}
	virtual bool isFixedFootprint () const override {return allElementsFixedFootprint();}

	virtual inline void updateHash (Hasher & hasher) const override;
	virtual inline bool construct (void * mem, SizeType sz) const override;
	virtual inline bool destruct (void * mem, SizeType sz) const override;
	
	bool addField (Field field);
	bool hasField (std::string const & name) const;
	SizeType getFieldCount () const {return SizeType(m_fields.size());}
	Field const & getField (size_t index) const {return m_fields[index];}
	Field const * findField (std::string const & name) const;

protected:
	SizeType calculateFootprint () const;
	bool allElementsFixedFootprint () const;

private:
	SizeType m_cur_size;
	FieldContainer m_fields;
};

//======================================================================
//======================================================================

namespace details {
	// A typelist that maps DyStruct::Family enum values to actual types.
	template <Family F> struct family_type_map {};
	template <> struct family_type_map<Family::Basic> {typedef BasicType type;};
	template <> struct family_type_map<Family::Enum> {typedef EnumType type;};
	template <> struct family_type_map<Family::Array> {typedef ArrayType type;};
	//template <> struct family_type_map<Family::Struct> {typedef StructType type;};
	template <> struct family_type_map<Family::DyStruct> {typedef DyStructType type;};
	//template <> struct family_type_map<Family::String> {typedef StringType type;};
	//template <> struct family_type_map<Family::Vector> {typedef VectorType type;};
	//template <> struct family_type_map<Family::Set> {typedef SetType type;};
	//template <> struct family_type_map<Family::Map> {typedef MapType type;};
}

//======================================================================

class CompiledType;

//----------------------------------------------------------------------

class TypeManager
{
public:
	TypeManager ();
	~TypeManager ();

	void clear ();

	template <Family Family, typename... Args>
	auto createType (Args&&... args) -> typename details::family_type_map<Family>::type *
	{
		auto ret = new typename details::family_type_map<Family>::type {std::forward<Args>(args)...};
		m_raw_types.insert (ret);
		return ret;
	}
	
	// Make sure you've destroyed all CompiledType and Type instances that depend on this first
	bool destroyType (Type * type);
	bool hasType (Type * type) const;

	CompiledType * compile (Type * type, Name const & name);
	bool destroyCompiledType (CompiledType * cmptype);
	bool hasCompiledType (CompiledType * cmptype) const;
	
	CompiledType * getCompiledType (Name const & name) const;
	Type const * getType (Name const & name) const;

private:

private:
	std::unordered_set<Type *> m_raw_types;
	std::unordered_set<CompiledType *> m_compiled_types;
	
	std::unordered_map<Name, CompiledType *> m_names;
};

//======================================================================

class InstancePtr
{
	friend class CompiledType;
	
private:
	explicit InstancePtr (CompiledType const * ctype_ptr)
		: m_data (nullptr)
		, m_ctype (ctype_ptr)
	{
	}
	
	InstancePtr (Byte * data_ptr, CompiledType const * ctype_ptr)
		: m_data (data_ptr)
		, m_ctype (ctype_ptr)
	{
	}

public:
	~InstancePtr () = default;

	// Is this a good idea or a bad one?
	//InstancePtr & operator = (std::nullptr_t that_null) {m_data = nullptr; return *this;}

	CompiledType const & type () const {return *m_ctype;}
	CompiledType const * typePtr () const {return m_ctype;}
	Byte * data () {return m_data;}
	Byte const * data () const {return m_data;}
	bool isNull () const {return nullptr == m_data;}
	
	inline void destroySelf ();
	void nullify () {m_data = nullptr;}

private:
	Byte * m_data;
	CompiledType const * m_ctype;
};


//======================================================================
// Accessors:
//======================================================================
// TODO: Add a lot of debugging features (e.g. keep a pointer to CompiledType in accessor.)
//======================================================================

/// When the final field you want to access is a Basic field.
template <Basic BasicType>
class Accessor
{
	friend class CompiledType;

	typedef typename details::BasicTypeMap<BasicType>::type MyT;
	typedef typename details::BasicTypeMap<BasicType>::type const MyCT;

private:
	explicit Accessor (OffsetType offset)
		: m_offset {offset}
	{}

public:
	~Accessor () = default;

	MyT & operator () (InstancePtr inst) {return *reinterpret_cast<MyT *>(inst.data() + m_offset);}
	MyCT & operator () (InstancePtr inst) const {return *reinterpret_cast<MyCT *>(inst.data() + m_offset);}

private:
	OffsetType m_offset;	/// This is always a byte offset.
};

//----------------------------------------------------------------------

/// When the final field you want to access is a *packed* array of Basic fields.
template <Basic BasicType>
class AccessorArray
{
	friend class CompiledType;

	typedef typename details::BasicTypeMap<BasicType>::type MyT;
	typedef typename details::BasicTypeMap<BasicType>::type const MyCT;

private:
	explicit AccessorArray (OffsetType offset)
		: m_offset {offset}
	{}

public:
	~AccessorArray () = default;

	// TODO: Actually implement this whenever you wrote ctors for Accessor
	//Accessor<BasicType> operator [] (size_t index) {return {m_offset + index * sizeof(MyT)};}

	// Probably should not implement any of these:
	///// Use like this: x(p)[42]
	///// Where: x is the accessor, p is the instance ptr and 42 is the index.
	//MyT * operator () (InstancePtr inst) {return reinterpret_cast<MyT *>(inst.data() + m_offset);}
	//MyCT * operator () (InstancePtr inst) const {return reinterpret_cast<MyCT *>(inst.data() + m_offset);}
	//
	//MyT & operator () (InstancePtr inst, size_t index) {return reinterpret_cast<MyT *>(inst.data() + m_offset + index * sizeof(MyT));}
	//MyCT & operator () (InstancePtr inst, size_t index) const {return reinterpret_cast<MyCT *>(inst.data() + m_offset + index * sizeof(MyT));}

private:
	OffsetType m_offset;
};


//----------------------------------------------------------------------

/// When the final field you want to access is an array of Basic fields with a *stride*.
template <Basic BasicType>
class AccessorStride
{
	friend class CompiledType;

	typedef typename details::BasicTypeMap<BasicType>::type MyT;
	typedef typename details::BasicTypeMap<BasicType>::type const MyCT;

private:
	AccessorStride (OffsetType offset, OffsetType stride)
		: m_offset {offset}
		, m_stride {stride}
	{}

public:
	~AccessorStride () = default;

	// TODO: Actually implement this whenever you wrote ctors for Accessor
	//Accessor<BasicType> operator [] (size_t index) {return {m_offset + index * m_stride};}

	// Probably should not implement any of these:
	//MyT & operator () (InstancePtr inst, size_t index) {return reinterpret_cast<MyT *>(inst.data() + m_offset + index * m_stride);}
	//MyCT & operator () (InstancePtr inst, size_t index) const {return reinterpret_cast<MyCT *>(inst.data() + m_offset + index * m_stride);}

private:
	OffsetType m_offset;
	OffsetType m_stride;
};


//----------------------------------------------------------------------
//======================================================================
// CompiledType:
//======================================================================

class CompiledType
{
	//friend class TypeCompiler;
	friend class TypeManager;

private:	
	static ID CalculateID (Type const * type)
	{
		Hasher h;
		type->updateHash (h);
		return h.finalizeAndReset ();
	}
	
private:
	CompiledType (Type const * type, Name name)
		: m_type (type)
		, m_size (type->getSizeOf())
		, m_id (CalculateID(type))
		, m_name (std::move(name))
	{}
	
	~CompiledType ()
	{
	}
	
	
public:
	CompiledType & operator = (CompiledType const &) = delete;

	InstancePtr createInstance () const
	{
		Byte * mem = new Byte [sizeOf()];

		if (false == rawType()->construct (mem, sizeOf()))
		{
			delete[] mem;
			return InstancePtr (this);
		}

		return InstancePtr (mem, this);
	}
	
	void destroyInstance (InstancePtr & instance) const
	{
		assert (instance.typePtr() == this);
		
		if (!instance.isNull())
		{
			rawType()->destruct (instance.data(), sizeOf());
			delete[] instance.data();
			instance.m_data = nullptr;
		}	
	}
	
	Type const * rawType () const {return m_type;}
	SizeType sizeOf() const {return m_size;}
	ID id () const {return m_id;}
	Name const & name () const {return m_name;}
	
	template <Basic basic_type>
	Accessor<basic_type> accessor () const
	{
		assert (m_type->isBasic());
		assert (dynamic_cast<BasicType const *>(m_type)->getBasicType() == basic_type);

		return Accessor<basic_type>{0};
	}

	template <Basic basic_type>
	Accessor<basic_type> accessorField (std::string const & field_name) const
	{
		assert (m_type->isDyStruct());
		auto dys_type = dynamic_cast<DyStructType const *>(m_type);
		assert (dys_type->hasField(field_name));
		auto field = dys_type->findField (field_name);
		assert (field->type->isBasic());
		assert (dynamic_cast<BasicType const *>(field->type)->getBasicType() == basic_type);

		return Accessor<basic_type>{field->offset};
	}

protected:
	Type const * m_type;
	SizeType const m_size;
	ID const m_id;
	Name const m_name;
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//======================================================================

}	// namespace DyStruct

//======================================================================

#include "DyStructInline.h"

//======================================================================

#endif	// __Y__DYSTRUCT_H__
