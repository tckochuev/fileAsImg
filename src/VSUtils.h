#pragma once

#include <stdexcept>

#define DECL_CP_MV_CTORS_BY_DEF(ClassName)\
ClassName(const ClassName&) = default;\
ClassName(ClassName&&) noexcept = default;

#define DECL_DEF_CP_MV_CTORS_BY_DEF(ClassName)\
ClassName() = default;\
DECL_CP_MV_CTORS_BY_DEF(ClassName)

#define DECL_CP_MV_ASSIGN_BY_DEF(ClassName)\
ClassName& operator=(const ClassName&) = default;\
ClassName& operator=(ClassName&&) noexcept = default;

#define DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(ClassName)\
DECL_DEF_CP_MV_CTORS_BY_DEF(ClassName)\
DECL_CP_MV_ASSIGN_BY_DEF(ClassName)

///@brief Ensures that @p ClassName without any state can not be constructed, copied or moved
///from outside of class definition and has virtual public destructor. Do not expect any specific implementation.
#define INTERFACE(ClassName)\
public:\
virtual ~ClassName() = default;\
protected:\
DECL_DEF_CP_MV_CTORS_ASSIGN_BY_DEF(ClassName)

namespace tc::err::exc
{

class Interrupted : public std::runtime_error
{
public:
	Interrupted() : std::runtime_error("Operation has been interrupted") {}
	using std::runtime_error::runtime_error;
};

class InvalidArgument : public std::runtime_error
{
public:
	InvalidArgument() : std::runtime_error("Invalid argument has been passed to function") {}
	using std::runtime_error::runtime_error;
};

};