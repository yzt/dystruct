//======================================================================

#include <dystruct/DyStruct.h>

#include <utility>

//======================================================================

namespace DyStruct {

//======================================================================

	namespace details {

//======================================================================

const FamilyTraits gc_FamilyTraits [int(Family::_count)] =
{
	{Family::Basic   , "Basic"   ,  true, false, false, false,  true},
	{Family::Enum    , "Enum"    ,  true, false, false, false,  true},
	{Family::Array   , "Array"   , false,  true, false, false,  true},
	{Family::Struct  , "Struct"  , false, false,  true, false,  true},
	{Family::DyStruct, "DyStruct", false, false,  true, false,  true},
	{Family::String  , "String"  , false, false, false, false, false},
	{Family::Vector  , "Vector"  , false,  true, false, false, false},
	{Family::Set     , "Set"     , false,  true, false, false, false},
	{Family::Map     , "Map"     , false,  true, false,  true, false},
};

const BasicTraits gc_BasicTraits [int(Basic::_count)] =
{
	{Basic::I8   , "I8"   , sizeof(BasicTypeMap<Basic::I8>::type)   ,  true,  true,  true, false}, 
	{Basic::U8   , "U8"   , sizeof(BasicTypeMap<Basic::U8>::type)   ,  true,  true, false, false},
	{Basic::I16  , "I16"  , sizeof(BasicTypeMap<Basic::I16>::type)  ,  true,  true,  true, false},
	{Basic::U16  , "U16"  , sizeof(BasicTypeMap<Basic::U16>::type)  ,  true,  true, false, false},
	{Basic::I32  , "I32"  , sizeof(BasicTypeMap<Basic::I32>::type)  ,  true,  true,  true, false},
	{Basic::U32  , "U32"  , sizeof(BasicTypeMap<Basic::U32>::type)  ,  true,  true, false, false},
	{Basic::I64  , "I64"  , sizeof(BasicTypeMap<Basic::I64>::type)  ,  true,  true,  true, false},
	{Basic::U64  , "U64"  , sizeof(BasicTypeMap<Basic::U64>::type)  ,  true,  true, false, false},
	{Basic::F32  , "F32"  , sizeof(BasicTypeMap<Basic::F32>::type)  ,  true, false,  true,  true},
	{Basic::F64  , "F64"  , sizeof(BasicTypeMap<Basic::F64>::type)  ,  true, false,  true,  true},
	{Basic::Bool , "Bool" , sizeof(BasicTypeMap<Basic::Bool>::type) , false, false, false, false},
	{Basic::Byte , "Byte" , sizeof(BasicTypeMap<Basic::Byte>::type) , false, false, false, false},
	{Basic::Char , "Char" , sizeof(BasicTypeMap<Basic::Char>::type) , false, false, false, false},
	{Basic::WChar, "WChar", sizeof(BasicTypeMap<Basic::WChar>::type), false, false, false, false},
};

//======================================================================

	}	// namespace details
	
//======================================================================
//======================================================================

bool EnumType::addEntry (std::string name, uint32_t value)
{
    if (hasNameOrValue(name, value))
		return false;

	m_name_values.emplace_back (std::make_pair(std::move(name), value));
	
	if (value > m_max_value)
		m_max_value = value;

	return true;
}

//----------------------------------------------------------------------

bool EnumType::addEntry (std::string name)
{
	return addEntry (std::move(name), static_cast<uint32_t>(m_max_value + 1));
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

bool EnumType::hasNameOrValue (std::string const & name, uint32_t value) const
{
	for (auto const & nvp : m_name_values)
		if (nvp.second == value || nvp.first == name)
			return true;

	return false;
}

//----------------------------------------------------------------------
//======================================================================

bool DyStructType::addField (Field field)
{
	if (!field.type || field.name.empty() || hasField(field.name))
		return false;

	field.offset = m_cur_size;
	auto fs = field.type->getSizeOf();
	m_fields.emplace_back (std::move(field));
	m_cur_size += fs;

	return true;
}

//----------------------------------------------------------------------

bool DyStructType::hasField (std::string const & name) const
{
	for (auto const & f : m_fields)
		if (name == f.name)
			return true;

	return false;
}

//----------------------------------------------------------------------

DyStructType::Field const * DyStructType::findField (std::string const & name) const
{
	for (auto const & f : m_fields)
		if (name == f.name)
			return &f;

	return nullptr;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

SizeType DyStructType::calculateFootprint () const
{
	SizeType ret = 0;
	for (auto const & f : m_fields)
		ret += f.type->getFootprint();

	return ret;
}

//----------------------------------------------------------------------

bool DyStructType::allElementsFixedFootprint () const
{
	for (auto const & f : m_fields)
		if (!f.type->isFixedFootprint())
			return false;

	return true;
}

//----------------------------------------------------------------------
//======================================================================
//======================================================================

TypeManager::TypeManager ()
	: m_raw_types {}
	, m_compiled_types {}
	, m_names {}
{
}

//----------------------------------------------------------------------

TypeManager::~TypeManager ()
{
	clear ();
}

//----------------------------------------------------------------------

void TypeManager::clear ()
{
	m_names.clear ();

	for (auto & c : m_compiled_types)
		delete c;
	m_compiled_types.clear ();
	
	for (auto & t : m_raw_types)
		delete t;
	m_raw_types.clear ();
}

//----------------------------------------------------------------------

bool TypeManager::destroyType (Type * type)
{
	auto i = m_raw_types.find (type);
	if (m_raw_types.end() == i)
		return false;

	delete *i;
	m_raw_types.erase (i);
	return true;
}

//----------------------------------------------------------------------

bool TypeManager::hasType (Type * type) const
{
	return m_raw_types.end() != m_raw_types.find(type);
}

//----------------------------------------------------------------------

CompiledType * TypeManager::compile (Type * type, Name const & name)
{
	if (m_names.find(name) != m_names.end())	// Name already exists
		return nullptr;

	auto ret = new CompiledType {type, name};

	if (ret)
	{
		m_compiled_types.insert (ret);
		m_names[name] = ret;
	}

	return ret;
}

//----------------------------------------------------------------------

bool TypeManager::destroyCompiledType (CompiledType * cmptype)
{
	auto i = m_compiled_types.find (cmptype);
	if (m_compiled_types.end() == i)
		return false;

	m_names.erase ((*i)->name());
	delete *i;
	m_compiled_types.erase (i);

	return true;
}

//----------------------------------------------------------------------

bool TypeManager::hasCompiledType (CompiledType * cmptype) const
{
	return m_compiled_types.end() != m_compiled_types.find(cmptype);
}

//----------------------------------------------------------------------

CompiledType * TypeManager::getCompiledType (Name const & name) const
{
	auto i = m_names.find (name);
	if (m_names.end() != i)
		return i->second;
	else
		return nullptr;
}

//----------------------------------------------------------------------

Type const * TypeManager::getType (Name const & name) const
{
	auto i = m_names.find (name);
	if (m_names.end() != i)
		return i->second->rawType();
	else
		return nullptr;
}

//----------------------------------------------------------------------
//======================================================================

}	// namespace DyStruct

//======================================================================
